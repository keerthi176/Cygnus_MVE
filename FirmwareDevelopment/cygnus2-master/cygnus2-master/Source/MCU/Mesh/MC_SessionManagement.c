/*************************************************************************************
*  CONFIDENTIAL between PA and Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2017 PA Consulting Group. All rights reserved.
*  Cambridge Technology Centre
*  Melbourn Royston
*  Herts SG8 6DP UK
*  Tel: <44> 1763 261222
*
**************************************************************************************
*  File        : MC_SessionManagement.c
*
*  Description : Source for the management of message acks and message re-transmission
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "lptim.h"
#include "CO_Defines.h"
#include "utilities.h"
#include "MC_StateManagement.h"
#include "MC_AckManagement.h"
#include "DM_SerialPort.h"
#include "MC_PUP.h"
#include "MC_MAC.h"
#include "SM_StateMachine.h"
#include "MC_SACH_Management.h"
#include "MC_MeshFormAndHeal.h"
#include "MC_MacConfiguration.h"
#include "CFG_Device_cfg.h"
#include "MM_NeighbourInfo.h"
#include "MC_TDM_SlotManagement.h"
#include "MC_TDM.h"
#include "MC_SyncAlgorithm.h"
#include "MC_SessionManagement.h"
/* Private definitions
*************************************************************************************/
#define SMGR_MAX_HISTORY_LENGTH 10 /* for network utilisation statistics */
#define INITIAL_CHILD_RSSI -90
#define SMGR_DELAYED_RESET_DURATION 20 //shortframes
#define SECONDARY_PARENT_RECHECK_PERIOD_SHORT_THRESHOLD 50 //long Frames
#define SECONDARY_PARENT_RECHECK_PERIOD_MEDIUM_THRESHOLD 500 //long Frames
#define SECONDARY_PARENT_RECHECK_PERIOD_SHORT 10 //long Frames
#define SECONDARY_PARENT_RECHECK_PERIOD_MEDIUM 100 //long Frames
#define SECONDARY_PARENT_RECHECK_PERIOD_MAX 1000 //long Frames
#define MIN_RECORDABLE_SNR 0x8000    // min value that can be stored in an int16_t (-32768)
#define MIN_RECORDABLE_RSSI 0x8000    // min value that can be stored in an int16_t (-32768)

/* Private Functions Prototypes
*************************************************************************************/


/* Global Variables
*************************************************************************************/
SMGR_ParentDetails_t gParent[NUMBER_OF_PARENTS_AND_TRACKING_NODES];
extern osPoolId MeshPool;
extern osMessageQId(MeshQ);
extern uint32_t gNetworkAddress;
extern bool gFreqLockAchieved;
extern uint32_t gZoneNumber;

bool gDynamicPowerMode = false;
MC_TrackingNodes_t gaTrackingNodes;

/* Private Variables
*************************************************************************************/
/* Checksum value added in the SW image during the build process */
//SMGR_ChildDetails_t gChildren[MAX_CHILDREN_PER_NCU_PARENT] __attribute__( ( section( "ChildRecords"),used) );
uint16_t gChildren[MAX_CHILDREN_PER_NCU_PARENT];
extern NeighbourInformation_t neighbourArray[MAX_DEVICES_PER_SYSTEM];
static bool gDelayedResetActivated = false;
static uint32_t gDelayedResetCount = 0;
static char gDelayedResetReason[126]; //126 from sizeof sw_reset_debug_message, defined in MM_Main.c
static bool bUsePrimaryParent = false;
static uint32_t gSMGRNumberOfChildren;
static bool Is_Master = false;
static bool gIsLoggedOn = false;
static uint8_t gDeviceRank = 0;  // Initialised to correct value for an NCU: gets overwritten in the RBU before it is read.
static const char *gParentRoleNames[NUMBER_OF_PARENTS_AND_TRACKING_NODES] = 
{
   "PRIMARY_PARENT",
   "SECONDARY_PARENT",
   "PRIMARY_TRACKING_NODE",
   "SECONDARY_TRACKING_NODE"
};
static uint8_t gNoOfChildrenOfPTNIdx = 0;

/* network utilisation statistc values */
static uint32_t gPrimaryRachTrafficCount = 0;
static uint32_t gSecondaryRachTrafficCount = 0;
static bool gPrimaryRachHistory[SMGR_MAX_HISTORY_LENGTH];
static uint32_t gPrimaryRachHistoryIndex = 0;
static bool gSecondaryRachHistory[SMGR_MAX_HISTORY_LENGTH];
static uint32_t gSecondaryRachHistoryIndex = 0;
static uint32_t gInitialTrackingNodeMissedHeartbeatCount = 0;
static bool gFreeRunningDetected = false;
static bool gInhibitedNodes[MAX_DEVICES_PER_SYSTEM];//to record nodes that refuse connection
static uint8_t gPowerSavingTxPower = TX_HIGH_OUTPUT_POWER;
static bool gPreformedPrimaryLinkBroken = false;
static bool gPreformedSecondaryLinkBroken = false;
static uint32_t gSecondaryParentRecheckCount = 0;

/* Function Definitions
*************************************************************************************/
static void MC_SMGR_CalculateNetworkUsage(void);
static uint32_t MC_SMGR_GetSecondaryParentSearchPeriod(void);

/*****************************************************************************
*  Function:   MC_SMGR_Initialise
*  Description:      Initialises/resets the session manager.
*  Param: isMaster   True if this node is the NCU.
*  Param: address    The node address of this NCU/RBU.
*  Returns:          None.
*****************************************************************************/
void MC_SMGR_Initialise(const bool isMaster, const uint16_t address)
{
   Is_Master = isMaster;
   bUsePrimaryParent = false;
   
   for ( uint32_t index = 0; index < MAX_DEVICES_PER_SYSTEM; index++ )
   {
      gInhibitedNodes[index] = false;
   }

   for ( uint32_t index = 0; index < NUMBER_OF_PARENTS_AND_TRACKING_NODES; index++)
   {
      gParent[index].NodeID = CO_BAD_NODE_ID;
      gParent[index].IsActive = false;
      gParent[index].WaitingForRouteAddResponse = false;
      gParent[index].ShortFramesSinceRouteAdd = 0;
      gParent[index].RouteAddResponsesReceived = 0;
      gParent[index].PublishedRank = UNASSIGNED_RANK;
   }
   
   /* initialise the acknowledgement manager */
   MC_ACK_Initialise();
   
   uint32_t max_children = CFG_GetMaxNumberOfChildren();
   for ( uint32_t index = 0; index < max_children; index++)
   {
      gChildren[index] = CO_BAD_NODE_ID;
   }
   gSMGRNumberOfChildren = 0;
   
   if( ADDRESS_NCU == address )
   {
      //NCU is always rank 0
      MC_SMGR_SetRank(0);
      
      /* The NCU does not need to monitor all DCH on startup
         because it doesn't need to scan for parents */
      MC_SMGR_SetAllDCHSlotBehaviour(MC_TDM_DCH_SLEEP_E);
   }
   
   /* Always set the heartbeat flag for the current device so that the MAC will 
      transmit our heartbeat */
   MC_SMGR_StartHeartbeatMonitorForNode(gNetworkAddress);
   
   //Reset the count for secondary parent recheck (periodic check for second parent if we only have one)
   gSecondaryParentRecheckCount = 0;
   
}

/*****************************************************************************
*  Function:   MC_SMGR_SetRank
*  Description:      Routine to set the device rank.
*  Param rank        The new rank
*  Returns:          void
*  Notes:            
*****************************************************************************/
void MC_SMGR_SetRank(const uint8_t rank)
{
   if (UNASSIGNED_RANK >= rank)
   {
      gDeviceRank = rank;
      if (UNASSIGNED_RANK > rank)
      {
         CO_PRINT_B_1(DBG_INFO_E, "Device rank set to %d\r\n\n", rank);
      }
      else
      {
         CO_PRINT_B_1(DBG_INFO_E, "Device rank set to undefined.\r\n\n", rank);
      }
   }
}

/*****************************************************************************
*  Function:   MC_SMGR_GetRank
*  Description:      Routine to return the device rank.
*  Param             None.
*  Returns:          The device rank.
*  Notes:            
*****************************************************************************/
uint8_t MC_SMGR_GetRank(void)
{
   return gDeviceRank;
}

/*****************************************************************************
*  Function:   MC_SMGR_CalculateDCHSlot
*  Description:      Converts a Node Address into a dch slot number.
*  Param node_id:    A node ID (Network address)
*  Returns:          The dch slot number on success, CO_BAD_NODE_ID otherwise.
*  Notes:            Will only return a slot number if the node ID is < MAX_NUMBER_OF_NODES
*****************************************************************************/
uint16_t MC_SMGR_CalculateDCHSlot(const uint16_t node_id)
{
   uint16_t result = CO_BAD_NODE_ID;

   uint16_t max_nodes = MC_MaxNumberOfNodes();
   if (node_id < max_nodes)
   {
      uint32_t short_frame_index = node_id / DCH_SLOTS_PER_SHORT_FRAME;
      uint32_t short_frame_slot = node_id % DCH_SLOTS_PER_SHORT_FRAME;

      result = (short_frame_index * SLOTS_PER_SHORT_FRAME) + short_frame_slot;
   }

   return result;
}

/*****************************************************************************
*  Function:   bool MC_SMGR_SetParentNodes
*  Description:         Sets the primary and secondary parents for the RBU session Manager.
*  Param primary_dch_slot:    The slot number for the primary parent's dch.
*  Param secondary_dch_slot:  The slot number for the secondary parent's dch.
*  Returns:                   true on success, false otherwise.
*  Notes:                     none
*****************************************************************************/
bool MC_SMGR_SetParentNodes(const uint16_t primary_address, const uint16_t secondary_address)
{
   bool status = false;
   
   if ( CO_BAD_NODE_ID != primary_address || CO_BAD_NODE_ID != secondary_address)
   {      
      if ( CO_BAD_NODE_ID != primary_address )
      {
         ShortListElement_t primary_node;
         if (SUCCESS_E == MC_MFH_FindNode(primary_address, &primary_node) )
         {
            gParent[PRIMARY_PARENT].NodeID = primary_address;
            gParent[PRIMARY_PARENT].IsActive = false;
            neighbourArray[gParent[PRIMARY_PARENT].NodeID].Rank = primary_node.Rank;
            gParent[PRIMARY_PARENT].PublishedRank = primary_node.Rank;
             
            CO_PRINT_B_1(DBG_INFO_E, "Opening session for Primary Parent - Network ID=%d\n\r", primary_address);
            status = true;
            if ( MC_SMGR_ConnectToParent(primary_address, true) )
            {
               CO_PRINT_B_1(DBG_INFO_E, "Join Request sent to Primary Parent - Network ID=%d\n\r", primary_address);
               gParent[PRIMARY_PARENT].WaitingForRouteAddResponse = false;
               gParent[PRIMARY_PARENT].ShortFramesSinceRouteAdd = 0;
               CO_PRINT_A_2(DBG_DATA_E,"%d,%d\r\n", GUI_EVENT_ADDED_PRIMARY_PARENT_E, primary_address);
            }
      
         }
      }
      
      if ( CO_BAD_NODE_ID != secondary_address )
      {         
         ShortListElement_t secondary_node;
         if (SUCCESS_E == MC_MFH_FindNode(secondary_address, &secondary_node) )
         {
            gParent[SECONDARY_PARENT].NodeID = secondary_address;
            gParent[SECONDARY_PARENT].IsActive = false;
            neighbourArray[gParent[SECONDARY_PARENT].NodeID].Rank = secondary_node.Rank;
            gParent[SECONDARY_PARENT].PublishedRank = secondary_node.Rank;
            /* Don't send a ROUTE ADD to the secondary parent immediately.  It clashes with the 
               ROUTE ADD RESPONSE from the primary parent.  If the primary parent has had a 
               response (i.e. is active) then it's OK to send the ROUTE ADD now. */
            if ( gParent[PRIMARY_PARENT].IsActive )
            {
               CO_PRINT_B_1(DBG_INFO_E, "Opening session  for Secondary Parent - Network ID=%d\n\r", secondary_address);
               status = true;
               if ( MC_SMGR_ConnectToParent(secondary_address, true) )
               {
                  CO_PRINT_B_1(DBG_INFO_E, "Join Request sent to Secondary Parent - Network ID=%d\n\r", secondary_address);
                  gParent[SECONDARY_PARENT].WaitingForRouteAddResponse = false;
                  gParent[SECONDARY_PARENT].ShortFramesSinceRouteAdd = 0;
               }
            }
            
            CO_PRINT_A_2(DBG_DATA_E,"%d,%d\r\n", GUI_EVENT_ADDED_SECONDARY_PARENT_E, secondary_address);
         }
      }
   }
   
   return status;
}


/*****************************************************************************
*  Function:   bool MC_SMGR_SetTrackingNodes
*  Description:         Sets the primary and secondary tracking nodes for the RBU session Manager.
*  Param primary_dch_slot:    The slot number for the primary tracking nodes's dch.
*  Param secondary_dch_slot:  The slot number for the secondary tracking nodes's dch.
*  Returns:                   true on success, false otherwise.
*  Notes:                     none
*****************************************************************************/
bool MC_SMGR_SetTrackingNodes(const uint16_t primary_tracking_nodeID, const uint16_t secondary_tracking_nodeID)
{
   bool status = false;
   
   if ( CO_BAD_NODE_ID != primary_tracking_nodeID || CO_BAD_NODE_ID != secondary_tracking_nodeID)
   {      
      // If a primary tracking node is provided, set the array entries.
      if ( CO_BAD_NODE_ID != primary_tracking_nodeID )
      {
         gParent[PRIMARY_TRACKING_NODE].NodeID = primary_tracking_nodeID;
         status = true;
         /* Send a status message to NCU */
         MC_SMGR_SendStatusIndication( CO_MESH_EVENT_PRIMARY_TRACKING_NODE_ADDED_E, primary_tracking_nodeID, 0, true );
}
      
      // If a secondary tracking node is provided, set the array entries.
      if ( CO_BAD_NODE_ID != secondary_tracking_nodeID )
      {         
         gParent[SECONDARY_TRACKING_NODE].NodeID = secondary_tracking_nodeID;
         status = true;
         /* Send a status message to NCU */
         MC_SMGR_SendStatusIndication( CO_MESH_EVENT_SECONDARY_TRACKING_NODE_ADDED_E, secondary_tracking_nodeID, 0, true );
      }
   }

   return status;
}


/*****************************************************************************
*  Function:   bool MC_SMGR_IsParentNode
*  Description:      Report whether the passed node ID is a parent of the RBU.
*  Param - node_id:  The network address that you want to test as a parent. 
*  Returns:          true if the node_id is a parent, false otherwise.
*  Notes:            Required for deciding if downlink messages are from a parent.
*****************************************************************************/
bool MC_SMGR_IsParentNode(const uint16_t node_id)
{
   bool isParent = false;
   
   if ( node_id == gParent[PRIMARY_PARENT].NodeID ||
        node_id == gParent[SECONDARY_PARENT].NodeID )
   {
      isParent = true;
   }
   return isParent;
}

/*****************************************************************************
*  Function:   bool MC_SMGR_GetPrimaryParentID
*  Description:      Return the node ID of the Primary Parent, if it is active.
*  Param - none:   
*  Returns:          uint16_t - The node ID of the Primary Parent.
*  Notes:            The primary parent is considered 'active' if it has
*                    returned a positive NodeAddResponse.
*****************************************************************************/
uint16_t MC_SMGR_GetPrimaryParentID()
{
   uint16_t node_id = CO_BAD_NODE_ID;
   
   if ( CO_BAD_NODE_ID != gParent[PRIMARY_PARENT].NodeID)
   {
      if ( gParent[PRIMARY_PARENT].IsActive )
      {
         node_id = gParent[PRIMARY_PARENT].NodeID;
      }
   }
   return node_id;
}

/*****************************************************************************
*  Function:   bool MC_SMGR_GetSecondaryParentID
*  Description:      Return the node ID of the Secondary Parent, if it is active.
*  Param - none:   
*  Returns:          uint16_t - The node ID of the Secondary Parent.
*  Notes:            The secondary parent is considered 'active' if it has
*                    returned a positive NodeAddResponse.
*****************************************************************************/
uint16_t MC_SMGR_GetSecondaryParentID()
{
   uint16_t node_id = CO_BAD_NODE_ID;
   
   if ( CO_BAD_NODE_ID != gParent[SECONDARY_PARENT].NodeID)
   {
      if ( gParent[SECONDARY_PARENT].IsActive )
      {
         node_id = gParent[SECONDARY_PARENT].NodeID;
      }
   }
   return node_id;
}

/*****************************************************************************
*  Function:   bool MC_SMGR_GetNextParentForUplink
*  Description:      Decide wich parent an uplink message should be sent to.
*  Param - none:   
*  Returns:          uint16_t - The node ID of the Parent or CO_BAD_NODE_ID 
*                    if there are no parents connected.
*  Notes:            Required for sending uplink messages via alternate routes.
*****************************************************************************/
uint16_t MC_SMGR_GetNextParentForUplink()
{
   uint16_t node_id = CO_BAD_NODE_ID;
   
   bUsePrimaryParent = !bUsePrimaryParent;
   
   if ( bUsePrimaryParent )
   {
      /*check that we have a primary parent*/
      if ( gParent[PRIMARY_PARENT].IsActive )
      {
         node_id = gParent[PRIMARY_PARENT].NodeID;
         /*TODO: check that the RACH uplink isn't blocked.  switch to secondary parent if it is*/
      }
      else
      {
         if ( gParent[SECONDARY_PARENT].IsActive )
         {
            node_id = gParent[SECONDARY_PARENT].NodeID;
         }
      }
   }
   else
   {
       /*check that we have a secondary parent*/
      if ( gParent[SECONDARY_PARENT].IsActive )
      {
         node_id = gParent[SECONDARY_PARENT].NodeID;
         /*TODO: check that the RACH uplink isn't blocked.  switch to primary parent if it is*/
      }
      else
      {
         if ( gParent[PRIMARY_PARENT].IsActive )
         {
            node_id = gParent[PRIMARY_PARENT].NodeID;
         }
      }
  }
   return node_id;
}

/*****************************************************************************
*  Function:   MC_SMGR_ConnectToParent
*  Description:            Send a 'Route Add' message to the parent candidate of the RBU.
*  Param - node_id:        The network address of the candidate parent node. 
*  Param - use_sach:       if true, send the RouteAdd on the SACH. 
*  Returns:                true if the message was sent, false otherwise.
*  Notes:            
*****************************************************************************/
bool MC_SMGR_ConnectToParent(const uint16_t node_id, const bool use_sach)
{
   bool msg_sent = true;
   CO_Message_t *pRouteAdd = NULL;

 
   /* send message to mesh task to trigger the generation of a route add message */
   pRouteAdd = ALLOCMESHPOOL;
   if (pRouteAdd)
   {
      osStatus osStat;
      RouteAddData_t routeAddData;
      routeAddData.UseSACH = use_sach;
         routeAddData.NodeID = node_id;
      routeAddData.Zone = gZoneNumber;
      if ( node_id == gParent[PRIMARY_PARENT].NodeID )
      {
         routeAddData.IsPrimary = true;
         if ( use_sach )
         {
            gParent[PRIMARY_PARENT].RouteAddRetries = 0;
            gParent[PRIMARY_PARENT].RouteAddResponsesReceived = 0;
         }
      }
      else 
      {
         routeAddData.IsPrimary = false;
         if ( use_sach )
         {
            gParent[SECONDARY_PARENT].RouteAddRetries = 0;
            gParent[SECONDARY_PARENT].RouteAddResponsesReceived = 0;
         }
      }
      
      pRouteAdd->Type = CO_MESSAGE_GENERATE_ROUTE_ADD_REQUEST_E;
      memcpy(pRouteAdd->Payload.PhyDataReq.Data, &routeAddData, sizeof(RouteAddData_t));

      osStat = osMessagePut(MeshQ, (uint32_t)pRouteAdd, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         FREEMESHPOOL(pRouteAdd);
         msg_sent = false;
      }
   }

   return msg_sent;
}


/*****************************************************************************
*  Function:   MC_SMGR_RouteAddResponse
*  Description:      Response to a 'Route Add' message from the parent of the RBU.
*  Param - pResponse : the parent's response message. 
*
*  Returns:          : true if a parent was updated, false otherwise.
*  Notes:            
*****************************************************************************/
bool MC_SMGR_RouteAddResponse(const RouteAddResponseMessage_t* pResponse)
{
   
   bool status = false;
   
   if ( pResponse )
   {
      uint16_t node_id = pResponse->Header.SourceAddress;
      if ( MAX_DEVICES_PER_SYSTEM > node_id )
      {
         if ( node_id == gParent[PRIMARY_PARENT].NodeID )
         {
            gParent[PRIMARY_PARENT].WaitingForRouteAddResponse = false;
            gParent[PRIMARY_PARENT].ShortFramesSinceRouteAdd = 0;
            gParent[PRIMARY_PARENT].RouteAddRetries = 0;
            
            /* It's possible that this message came back after the timeout and another ROUTE_ADD is in the queue.
               Tell the ACK Manager to purge all messages for this node.  This can be done by asking the ACK Manager
               to redirect the messages for this node.  Any messages addressed directly to the node cannot be
               redirected and will be removed from the queue.*/
            MC_ACK_RedirectMessagesForNode(node_id);
            
            if ( true == pResponse->Accepted )
            {
               gParent[PRIMARY_PARENT].IsActive = true;
               status = true;
               gPreformedPrimaryLinkBroken = false;

               
               //We expect 2 responses with the lower and upper zone enablement maps
               //We only need to do the following code once, so ignore the second response
               if ( 0 == pResponse->ZoneLowerUpper )
               {
                  osDelay(10);//very busy time for debug output.  Give it time to clear
                  
                  gParent[PRIMARY_PARENT].RouteAddResponsesReceived = 1;
                  //Find the record for the primary parent in 'mesh forming and healing' and set the device rank
                  ShortListElement_t primary_node;
                  if ( SUCCESS_E == MC_MFH_FindNode(node_id, &primary_node) )
                  {
                     //Store the rank of the primary parent
                     neighbourArray[gParent[PRIMARY_PARENT].NodeID].Rank = primary_node.Rank;
                     //Set the local device rank.
                     MC_SMGR_SetRank( primary_node.Rank + 1);
                  }

                  //if using preformed mesh, keep all hb slots in receive mode, otherwise set the default to sleep
                  if ( CFG_UsingPreformedMesh() )
                  {
                     MC_SMGR_SetAllDCHSlotBehaviour(MC_TDM_DCH_RX_E);
                  }
                  else
                  {
                     MC_SMGR_SetAllDCHSlotBehaviour(MC_TDM_DCH_SLEEP_E);
                  }
                           
                  /* set up the heartbeat monitor for the new parents dch slot*/
                  status = MC_SMGR_StartHeartbeatMonitorForNode(node_id);
                  neighbourArray[gParent[PRIMARY_PARENT].NodeID].LostHeartbeatCount = 0;
                  
                  
//               CO_PRINT_B_0(DBG_INFO_E, "********************************\n\r");
                  CO_PRINT_B_0(DBG_INFO_E, "*READY TO MOVE TO ACTIVE MODE*\n\r");
//               CO_PRINT_B_0(DBG_INFO_E, "********************************\n\n\r");
                  
                  /* Send a status message to NCU */
                  MC_SMGR_SendStatusIndication( CO_MESH_EVENT_PRIMARY_PARENT_ADDED_E, node_id, 0, true );
                  
                 /* Change the visual Indication */
                 CO_State_t current_state = MC_STATE_GetDeviceState();
                 if ( STATE_CONFIG_FORM_E == current_state )
                 {
                     DM_LedPatternRequest(LED_MESH_STATE_READY_FOR_ACTIVE_E);
                 }
                  
               }
               else 
               {
                  //2nd route add response from parent
                  //Check that we got the first one.  If not try again.
                  if ( 0 == gParent[PRIMARY_PARENT].RouteAddResponsesReceived )
                  {
                     CO_PRINT_B_0(DBG_INFO_E, "Missed first route add response from primary parent\n\r");
                     status = true;
                  if ( MC_SMGR_ConnectToParent(gParent[PRIMARY_PARENT].NodeID, true) )
                     {
                        CO_PRINT_B_1(DBG_INFO_E, "Join Request sent to Primry Parent - Network ID=%d\n\r", gParent[PRIMARY_PARENT].NodeID);
                        gParent[PRIMARY_PARENT].WaitingForRouteAddResponse = false;
                        gParent[PRIMARY_PARENT].ShortFramesSinceRouteAdd = 0;
                     }
                  }
                  else 
                  {
                     /* check to see if the secondary parent needs to be connected and send the ROUTE ADD now */
                     if ( CO_BAD_NODE_ID != gParent[SECONDARY_PARENT].NodeID &&
                           false == gParent[SECONDARY_PARENT].IsActive &&
                           false == gParent[SECONDARY_PARENT].WaitingForRouteAddResponse )
                     {
                        CO_PRINT_B_1(DBG_INFO_E, "Opening session for Secondary Parent - Network ID=%d\n\r", gParent[SECONDARY_PARENT].NodeID);
                        status = true;
                     if ( MC_SMGR_ConnectToParent(gParent[SECONDARY_PARENT].NodeID, true) )
                        {
                           CO_PRINT_B_1(DBG_INFO_E, "Join Request sent to Secondary Parent - Network ID=%d\n\r", gParent[SECONDARY_PARENT].NodeID);
                           gParent[SECONDARY_PARENT].WaitingForRouteAddResponse = false;
                           gParent[SECONDARY_PARENT].ShortFramesSinceRouteAdd = 0;
                        }
                     }
                     else
                     {
                        //There is no secondary parent.  Begin the process of periodically seeking one, unless the primary is the NCU.
                        if ( ADDRESS_NCU != gParent[PRIMARY_PARENT].NodeID )
                        {
                           MC_MFH_SelectNewParent();
                        }
                     }
                  }
               }
               
               //Update the global settings with the parent's information
               CFG_SetGlobalDelayEnabled(pResponse->GlobalDelayEnabled);
               CFG_SetSplitZoneDisabledMap(pResponse->ZoneLowerUpper,pResponse->ZoneMapWord, pResponse->ZoneMapHalfWord);
               CFG_SetFaultReportsEnabled(pResponse->FaultsEnabled);
               CFG_SetGlobalDelayOverride(pResponse->GlobalDelayOverride);
               //Enable long frame checks
               MC_SMGR_SetLoggedOn();
               //Busy time for usart.  A small delay helps clear the backlog
               osDelay(2);
               
               // Update the application with the day/night setting if it has changed
               if ( (CO_DayNight_t)pResponse->DayNight != CFG_GetDayNight() )
               {
                  ApplicationMessage_t appMessage;
                  appMessage.MessageType = APP_MSG_TYPE_DAY_NIGHT_STATUS_E;
                  CO_StatusIndicationData_t statusEvent;
                  statusEvent.Event = CO_MESH_EVENT_DAY_NIGHT_CHANGE_E;
                  statusEvent.EventNodeId = 0;
                  statusEvent.EventData = pResponse->DayNight;
                  statusEvent.DelaySending = false;
                  memcpy(appMessage.MessageBuffer, &statusEvent, sizeof(CO_StatusIndicationData_t));
                  SM_SendEventToApplication( CO_MESSAGE_MESH_STATUS_EVENT_E, &appMessage );
               }

            }
            else 
            {
               status = true;
               /* Primary Parent declined RouteAdd. */
               if ( 1 == MC_SMGR_GetNumberOfParents() )
               {
                  gInhibitedNodes[node_id] = true;
                  //We only had one parent and it declined our route add request.
                  //Reselect parents
                  if ( MC_MFH_DoParentSelection() )
                  {
                     //Inhibit the rejecting parent node's future selection
                     for ( uint32_t index = 0; index < MAX_DEVICES_PER_SYSTEM; index++ )
                     {
                        if ( gInhibitedNodes[index] )
                        {
                           MC_MFH_InhibitNodeSelection(gParent[PRIMARY_PARENT].NodeID);
                        }
                     }

                     //Flush out the old data.  'Mesh form and heal' will find new parents
                     //We can't call MC_SMGR_DeleteNode() here because it will reset the device.
                     gParent[PRIMARY_PARENT].NodeID = CO_BAD_NODE_ID;
                     gParent[PRIMARY_PARENT].IsActive = false;
                     gParent[PRIMARY_PARENT].WaitingForRouteAddResponse = false;
                     gParent[PRIMARY_PARENT].ShortFramesSinceRouteAdd = 0;
                     gParent[PRIMARY_PARENT].RouteAddRetries = 0;
                  }
                  else 
                  {
                     //The reselection process failed to start. Delete the parent to force a reset.
                     MC_SMGR_DeleteNode( node_id, "Primary parent declined route add" );
                     neighbourArray[gParent[PRIMARY_PARENT].NodeID].FailedResponseCount = 0;
                  }
               }
               else 
               {
                  //Primary parent declined route add, but we have a secondary parent.
                  //Just delete the primary parent and that will promote the other nodes
                  MC_SMGR_DeleteNode( node_id, "Primary parent declined route add" );
               }
            }
         }
         else if ( node_id == gParent[SECONDARY_PARENT].NodeID )
         {
            gParent[SECONDARY_PARENT].WaitingForRouteAddResponse = false;
            gParent[SECONDARY_PARENT].ShortFramesSinceRouteAdd = 0;
            gParent[SECONDARY_PARENT].RouteAddRetries = 0;
            gPreformedSecondaryLinkBroken = false;
            
            /* It's possible that this message came back after the timeout and another ROUTE_ADD is in the queue.
               Tell the ACK Manager to purge all messages for this node.  This can be done by asking the ACK Manager
               to redirect the messages for this node.  Any messages addressed directly to the node cannot be
               redirected and will be removed from the queue.*/
            MC_ACK_RedirectMessagesForNode(node_id);

            if ( true == pResponse->Accepted )
            {
               gParent[SECONDARY_PARENT].IsActive = true;
               status = true;
               /* set up the heartbeat monitor for the new parents dch slot*/
               status = MC_SMGR_StartHeartbeatMonitorForNode(node_id);
               neighbourArray[gParent[SECONDARY_PARENT].NodeID].LostHeartbeatCount = 0;
               CO_PRINT_B_1(DBG_INFO_E, "Secondary parent is active. Network ID = %d\n\r", node_id);
               /* Send a status message to NCU */
               MC_SMGR_SendStatusIndication( CO_MESH_EVENT_SECONDARY_PARENT_ADDED_E, node_id, 0, true );
               //Find the record for the primary parent in 'mesh forming and healing' and set the device rank
               ShortListElement_t secondary_node;
               if ( SUCCESS_E == MC_MFH_FindNode(node_id, &secondary_node) )
               {
                  //Store the rank of the primary parent
                  neighbourArray[gParent[SECONDARY_PARENT].NodeID].Rank = secondary_node.Rank;
               }
               
               /* Send status messages to NCU for tracking nodes*/
               if ( CO_BAD_NODE_ID != gParent[PRIMARY_TRACKING_NODE].NodeID )
               {
                  MC_SMGR_SendStatusIndication( CO_MESH_EVENT_PRIMARY_TRACKING_NODE_ADDED_E, gParent[PRIMARY_TRACKING_NODE].NodeID, 0, true );
               }
               if ( CO_BAD_NODE_ID != gParent[SECONDARY_TRACKING_NODE].NodeID )
               {
                  MC_SMGR_SendStatusIndication( CO_MESH_EVENT_SECONDARY_TRACKING_NODE_ADDED_E, gParent[SECONDARY_TRACKING_NODE].NodeID, 0, true );
               }
            }
            else 
            {
               status = true;
               /* Secondary Parent declined RouteAdd. */
               MC_SMGR_DeleteNode( node_id, "Secondary parent declined route add" );
               neighbourArray[gParent[SECONDARY_PARENT].NodeID].FailedResponseCount = 0;
               if (  CO_BAD_NODE_ID == gParent[SECONDARY_PARENT].NodeID )
               {
                  //No secondary parent was promoted.  Try to find another.
                  MC_MFH_SelectNewParent();
               }
            }
         }
      }
   }
   return status;
}

/*****************************************************************************
*  Function:   MC_SMGR_RouteAddRequest
*  Description:         Request by another network node to become a child of this node.
*  Param - node_id:     The network address of the child node. 
*  Param - is_primary:  True if the child want this node to be the primary parent. 
*  Returns:             true if a child was accepted or was already a child, false otherwise.
*  Notes:            
*****************************************************************************/
bool MC_SMGR_RouteAddRequest(const uint16_t node_id, const bool is_primary)
{
   bool status = false;
   uint16_t dch_slots_per_long_frame = MC_DchSlotsPerLongFrame();
   
   /* We can't accept children if we are not in contact with in the mesh */
   if ( ( 0 < MC_SMGR_GetNumberOfParents() ) || ( 0 < MC_SMGR_GetNumberOfTrackingNodes() ) || Is_Master )
   {
      /* check that the node_id is within the valid range */
      if ( dch_slots_per_long_frame > node_id && MAX_NUMBER_OF_SYSTEM_NODES > node_id )
      {
         /* check that we have a spare child slot */
         uint32_t max_children = CFG_GetMaxNumberOfChildren();
         if ( max_children > gSMGRNumberOfChildren )
         {
            uint32_t first_free_index = CO_BAD_NODE_ID;
            for ( uint32_t index = 0; index < max_children; index ++ )
            {
               if (gChildren[index] != CO_BAD_NODE_ID)
               {
                  if (neighbourArray[gChildren[index]].NodeID == node_id)
                  {
                     /* we already have this child. */
                     status = true;
                     /* The child must have dropped out of the mesh and rejoined.  Reset missed heartbeats */
                     neighbourArray[node_id].LostHeartbeatCount = 0;
                     neighbourArray[node_id].FailedResponseCount = 0;
                     neighbourArray[node_id].LostHeartbeatPingInterval = 0;
                     //Reset any old ping status
                     MC_PR_CancelPingForNode(node_id);
                     /* update the application. The child may have selected this node as its primary parent*/
                     MC_SMGR_SendStatusIndication( CO_MESH_EVENT_CHILD_NODE_ADDED_E, node_id, is_primary, true );
                     break;
                  }
               }
               else if ( CO_BAD_NODE_ID == first_free_index )
               {
                  first_free_index = index;
               }
            }
            
            if ( false == status && CO_BAD_NODE_ID != first_free_index )
            {
               gChildren[first_free_index] = node_id;
               neighbourArray[node_id].NodeID = node_id;
               neighbourArray[node_id].SNR = 0;
               neighbourArray[node_id].state = STATE_NOT_DEFINED_E;
               neighbourArray[node_id].LostHeartbeatCount = 0;
               neighbourArray[node_id].FailedResponseCount = 0;
               neighbourArray[node_id].LostHeartbeatPingInterval = 0;
               gSMGRNumberOfChildren++;
               CO_PRINT_B_1(DBG_INFO_E, "Added new child Network ID = %d\n\r", node_id);
               MC_TDM_SetDCHSlotBehaviour(node_id, MC_TDM_DCH_RX_E);
               //Reset any old ping status
               MC_PR_CancelPingForNode(node_id);
               /* Send a status message to NCU */
               MC_SMGR_SendStatusIndication( CO_MESH_EVENT_CHILD_NODE_ADDED_E, node_id, is_primary, true );
               
               CO_PRINT_A_2(DBG_DATA_E,"%d,%d\r\n", GUI_EVENT_ADDED_CHILD_E, node_id);

               status = true;
            }
            else 
            {
               if ( false == status )
               {
                  CO_PRINT_B_1(DBG_INFO_E,"Rejecting RouteAddReq : first_free_index=%d\r\n", first_free_index);
               }
            }
         }
         else 
         {
            CO_PRINT_B_1(DBG_INFO_E,"Rejecting RouteAddReq : number of children = %d\r\n", gSMGRNumberOfChildren);
         }
      }
      else 
      {
         CO_PRINT_B_1(DBG_INFO_E,"Rejecting RouteAddReq : nodeID not in range =%d\r\n", node_id);
      }
   }
   else 
   {
      CO_PRINT_B_2(DBG_INFO_E,"Rejecting RouteAddReq : PParent=%d, IsMaster=%d\r\n", MC_SMGR_GetPrimaryParentID(), Is_Master);
   }
      
   return status;
}

/*****************************************************************************
*  Function:   MC_SMGR_RouteDropRequest
*  Description:      Request by an associated node to be removed from the list.
*  Param - node_id:  The network address of the node.
*  Returns:          true if a node was removed, false if the record was not found.
*  Notes:
*****************************************************************************/
bool MC_SMGR_RouteDropRequest(const uint16_t node_id)
{
   bool status = false;
   
   //Are we stil forming?
   if ( 0 == MC_SMGR_GetNumberOfParents() && (ADDRESS_NCU != gNetworkAddress))
   {
      //If the lost node is the initial tracking node, send route drop and start the reset timeout in the application
      uint16_t sync_node_id = MC_SYNC_GetSyncNode();
      uint16_t initial_tracking_slot = MC_SMGR_CalculateDCHSlot(sync_node_id);
      uint16_t slot_for_node = CO_CalculateLongFrameSlotForDCH(node_id);
      if ( slot_for_node == initial_tracking_slot )
      {
         for (uint16_t count = 0; count < 3; count++)
         {
            MC_SMGR_SendRouteDrop(ADDRESS_GLOBAL,CO_RESET_REASON_DROPPED_BY_SYNC_NODE_E,false,true,0);
         }
         // Inform the application that we lost our connection to the mesh
         ApplicationMessage_t appMessage;
         appMessage.MessageType = APP_MSG_TYPE_ROUTE_DROP_E;
         CO_StatusIndicationData_t statusEvent;
         statusEvent.Event = CO_MESH_EVENT_INITIAL_TRK_NODE_LOST_E;
         statusEvent.EventNodeId = node_id;
         statusEvent.EventData = 0;
         statusEvent.DelaySending = false;
         memcpy(appMessage.MessageBuffer, &statusEvent, sizeof(CO_StatusIndicationData_t));
         SM_SendEventToApplication( CO_MESSAGE_MESH_STATUS_EVENT_E, &appMessage );
         status = true;
      }
   }
   else 
   {
      //If we have the dropped node, discard it now.
      if ( MC_SMGR_IsAChild(node_id) ||
           MC_SMGR_IsParentNode(node_id) )
      {
         // Inform the application that a child has dropped the link
         ApplicationMessage_t appMessage;
         appMessage.MessageType = APP_MSG_TYPE_ROUTE_DROP_E;
         CO_StatusIndicationData_t statusEvent;
         statusEvent.Event = CO_MESH_EVENT_CHILD_NODE_DROPPED_E;
         statusEvent.EventNodeId = node_id;
         statusEvent.EventData = 0;
         statusEvent.DelaySending = false;
         memcpy(appMessage.MessageBuffer, &statusEvent, sizeof(CO_StatusIndicationData_t));
         SM_SendEventToApplication( CO_MESSAGE_MESH_STATUS_EVENT_E, &appMessage );
         
         /* We just need to call MC_SMGR_DeleteNode.  It deals with parent reselection etc */
         status = MC_SMGR_DeleteNode(node_id, "Rxd route drop request");
      }
   }

   return status;
}

/*****************************************************************************
*  Function:   MC_SMGR_IsAChild
*  Description:      Check to see if a node ID is on the child list.
*  Param - node_id:  The network address of the child node.
*  Returns:          true if a child was found, false if the child was not found.
*  Notes:
*****************************************************************************/
bool MC_SMGR_IsAChild(const uint16_t node_id)
{
   bool is_a_child = false;
   uint32_t max_children = CFG_GetMaxNumberOfChildren();
   for ( uint32_t index = 0; index < max_children; index ++ )
   {
      if (gChildren[index] == node_id)
      {
         /* we have this child. */
         is_a_child = true;
         break;
      }      
   }

   return is_a_child;
}

/*****************************************************************************
*  Function:   MC_SMGR_GetNumberOfChildren
*  Description:      Return the number of children in the child list.
*  Param             None.
*  Returns:          The child count.
*  Notes:
*****************************************************************************/
uint16_t MC_SMGR_GetNumberOfChildren(void)
{
   return gSMGRNumberOfChildren;
}

/*****************************************************************************
*  Function:   MC_SMGR_GetNumberOfChildrenIndex
*  Description:      Convert the number of children in the child list to the 
*                    Number of Childern Index required by the heartbeat message.
*  Param             None.
*  Returns:          The Number of Childern Index.
*  Notes:
*****************************************************************************/
uint16_t MC_SMGR_GetNumberOfChildrenIndex(void)
{
   uint16_t NumberOfChildrenIndex = 0;
   
   uint32_t max_number_of_children = CFG_GetMaxNumberOfChildren();
   uint32_t number_of_children = MC_SMGR_GetNumberOfChildren();
   
   //For some 'max_number_of_children,' the index doesn't reach max index.  Correct for this.
   if ( number_of_children >= max_number_of_children )
   {
      NumberOfChildrenIndex = MAX_CHILD_IDX;
   }
   else 
   {
      //Calculate the index value
      NumberOfChildrenIndex = (((MAX_CHILD_IDX << 12)/max_number_of_children) * number_of_children) >> 12;
      //For some 'max_number_of_children,' the index is 0 when a child is present.  Correct for this.
      if ( (0 == NumberOfChildrenIndex) && ( number_of_children > 0 ) )
      {
         NumberOfChildrenIndex = 1;
      }
   }
   
   return NumberOfChildrenIndex;
}

/*****************************************************************************
*  Function:   MC_SMGR_GetNumberOfParents
*  Description:      Return the number of active parents.
*  Param             None.
*  Returns:          The parent count.
*  Notes:
*****************************************************************************/
uint16_t MC_SMGR_GetNumberOfParents(void)
{
   uint16_t num_of_parents = 0;
   
   if ( CO_BAD_NODE_ID != gParent[PRIMARY_PARENT].NodeID )
   {
      num_of_parents++;
   }
   
   if ( CO_BAD_NODE_ID != gParent[SECONDARY_PARENT].NodeID )
   {
      num_of_parents++;
   }
   
   return num_of_parents;
}

/*****************************************************************************
*  Function:   MC_SMGR_GetNumberOfTrackingNodes
*  Description:      Return the number of tracking nodes.
*  Param             None.
*  Returns:          The parent count.
*  Notes:
*****************************************************************************/
uint16_t MC_SMGR_GetNumberOfTrackingNodes(void)
{
   uint16_t num_of_tracking_nodes = 0;
   
   if ( CO_BAD_NODE_ID != gParent[PRIMARY_TRACKING_NODE].NodeID )
   {
      num_of_tracking_nodes++;
   }
   
   if ( CO_BAD_NODE_ID != gParent[SECONDARY_TRACKING_NODE].NodeID )
   {
      num_of_tracking_nodes++;
   }
   
   return num_of_tracking_nodes;
}

/*****************************************************************************
*  Function:   MC_SMGR_ReceivedNodeHeartbeat
*  Description:      Reset the missed heartbeat count for the supplied node ID.
*  Param - node_id:  The network address of the node.
*  Param - snr:      The received signal to noise ratio - stored locally
*  Param - state:    The state in the received heartbeat - stored locally
*  Param - rank :    The rank in the received heartbeat
*  Param - NoOfChildrenIdx:    Index into number of children table
*  Returns:          true if the count was Reset, false otherwise.
*  Notes:            Called when we receive a heartbeat message.
*****************************************************************************/
bool MC_SMGR_ReceivedNodeHeartbeat(const uint16_t node_id, const int8_t snr, const int8_t rssi, const CO_State_t state, const int8_t rank,
                                   const uint8_t  NoOfChildrenIdx)
{
   bool updated = false;
   uint32_t index = 0;
   
   /* check for primary tracking node */
   if (gParent[PRIMARY_TRACKING_NODE].NodeID == node_id)
   {
      /* store number of children of primary tracking node index */
      gNoOfChildrenOfPTNIdx = NoOfChildrenIdx;
   }
   
   /* step through list of parents and tracking nodes */
   for (index = 0; index < NUMBER_OF_PARENTS_AND_TRACKING_NODES; index++)
   {
      if ( gParent[index].NodeID == node_id )
      {
         //Check for broken parent link in preformed mesh and try to fix it.
         if ( CFG_UsingPreformedMesh() )
         {
            uint8_t required_rank = MC_SMGR_GetRank();
            required_rank -= 1;
            CO_State_t required_state = MC_STATE_GetDeviceState();
            if ( rank == required_rank )
            {
               if ( state >= required_state )
               {
                  if ( PRIMARY_PARENT == index )
                  {
                     if ( gPreformedPrimaryLinkBroken )
                     {
                        MC_SMGR_ConnectToParent(node_id, false);
                     }
                  }
                  else if ( SECONDARY_PARENT == index )
                  {
                     if ( gPreformedSecondaryLinkBroken )
                     {
                        MC_SMGR_ConnectToParent(node_id, false);
                     }
                  }
               }
            }
            /* Reset the missed heatbeats count for the node*/
            neighbourArray[gParent[index].NodeID].PingState = PR_IDLE_E;
            neighbourArray[gParent[index].NodeID].LostHeartbeatCount = 0;
            neighbourArray[gParent[index].NodeID].LostHeartbeatPingCount = 0;
            neighbourArray[gParent[index].NodeID].LostHeartbeatPingInterval = 0;
            neighbourArray[gParent[index].NodeID].SNR = snr;
            neighbourArray[gParent[index].NodeID].state = state;
            neighbourArray[gParent[index].NodeID].Rank = rank;
            gParent[index].PublishedRank = rank;
            updated = true;
         }
         else 
         {
            /* sometimes a parent resets and comes back quickly enough
               that we don't discard them.  We can detect this by checking
               the state property in the heartbeat.  If the parent state was 
               ACTIVE and now it's not, it has been reset. */
            if ( STATE_ACTIVE_E == neighbourArray[gParent[index].NodeID].state )
            {
               if ( STATE_ACTIVE_E != state )
               {
                  CO_PRINT_B_2(DBG_INFO_E,"Deleting parent %d because of change in state=%d\r\n",node_id,state);
                  /* the parent has been reset so discard it */
                  MC_SMGR_DeleteNode(node_id, "Change of parent state");
               }
               else if ( UNASSIGNED_RANK == rank )
               {
                  //The parent wouldn't have been selected if it hadn't published a rank, but now it has withdrawn its rank.
                  //This happens if it misses a parent heartbeat, to prevent joining nodes from connecting to it while
                  //its mesh status is pending.  Consider this a missed heartbeat.
                  gParent[index].PublishedRank = UNASSIGNED_RANK;
               }
               else
               {
                  /* Reset the missed heatbeats count for the node*/
                  neighbourArray[gParent[index].NodeID].PingState = PR_IDLE_E;
                  neighbourArray[gParent[index].NodeID].LostHeartbeatCount = 0;
                  neighbourArray[gParent[index].NodeID].LostHeartbeatPingCount = 0;
                  neighbourArray[gParent[index].NodeID].LostHeartbeatPingInterval = 0;
                  neighbourArray[gParent[index].NodeID].SNR = snr;
                  neighbourArray[gParent[index].NodeID].state = state;
                  neighbourArray[gParent[index].NodeID].Rank = rank;
                  gParent[index].PublishedRank = rank;
               }
            }
            else
            {
               /* Reset the missed heatbeats count for the node*/
               neighbourArray[gParent[index].NodeID].PingState = PR_IDLE_E;
               neighbourArray[gParent[index].NodeID].LostHeartbeatCount = 0;
               neighbourArray[gParent[index].NodeID].LostHeartbeatPingCount = 0;
               neighbourArray[gParent[index].NodeID].LostHeartbeatPingInterval = 0;
               neighbourArray[gParent[index].NodeID].SNR = snr;
               neighbourArray[gParent[index].NodeID].state = state;
               neighbourArray[gParent[index].NodeID].Rank = rank;
               gParent[index].PublishedRank = rank;
            }

            updated = true;
            break;
         }
      }
   }
   
   if (false == updated)
   {
      /* step through list of children */
      uint32_t max_children = CFG_GetMaxNumberOfChildren();
      for ( uint32_t index = 0; index < max_children; index ++ )
      {
         if (gChildren[index] == node_id)
         {
            /* sometimes a child resets and comes back quickly enough
               that we don't discard them.  We can detect this by checking
               the state property in the heartbeat.  If the child state was 
               ACTIVE and now it's not, it has been reset. */
            if ( STATE_ACTIVE_E == neighbourArray[node_id].state
                 && STATE_ACTIVE_E != state )
            {
               /* the child has been reset so discard it */
               MC_SMGR_DeleteNode(node_id, "Child state changed");
            }
            else
            {
               neighbourArray[node_id].PingState = PR_IDLE_E;
               neighbourArray[node_id].LostHeartbeatCount = 0;
               neighbourArray[node_id].SNR = snr;
               neighbourArray[node_id].RSSI = (neighbourArray[node_id].RSSI + rssi) / 2;/* running average rssi */
               neighbourArray[node_id].state = state;
            }
         
            updated = true;
            break;
         }
      }
   }

   // cancel free running count if the initial tracking node is seen
   // Not done in the NCU
   if ( false == Is_Master )
   {
      if (0 == MC_SMGR_GetNumberOfParents() )
      {
         uint16_t initial_tracking_node = MC_SYNC_GetSyncNode();
         if ( node_id == initial_tracking_node )
         {
            gInitialTrackingNodeMissedHeartbeatCount = 0;
            gFreeRunningDetected = false;
         }
      }
   }
   
   //If we received a heartbeat from a node that isn't a child or a parent, we have slots open
   //that shouldn't be.  Update the DCH behaviour.  Not applied to preformed mesh.
   if ( (false == CFG_UsingPreformedMesh()) && (false == MC_MFH_ScanForDevicesInProgress()) )
   {
      if ( (false == updated) && (STATE_ACTIVE_E == MC_STATE_GetDeviceState()) )
      {
         //Make sure that the 'mesh forming and healing' isn't doing a new parent search.
         if ( MFH_STATE_MAINTAINING_SHORT_LIST_E == MC_MFH_GetState() )
         {
            CO_PRINT_B_1(DBG_INFO_E,"Stopping heartbeat monitor for non-parent/child node %d\r\n", node_id);
            MC_SMGR_StopHeartbeatMonitorForNode(node_id);
         }
      }
   }

   return updated;
}

/*****************************************************************************
*  Function:   MC_SMGR_MissedNodeHeartbeat
*  Description:      Increment the missed heartbeat count for the supplied node ID.
*  Param - node_id:  The network address of the node.
*  Returns:          true if the count was updated, false otherwise.
*  Notes:            If the count reaches MAX_MISSED_HEARTBEATS the node will be removed.
*****************************************************************************/
bool MC_SMGR_MissedNodeHeartbeat(const uint16_t node_id)
{
   bool updated = false;
   bool foundFlag = false;
   uint32_t index = 0;
//   uint16_t count;
   uint16_t dch_slots_per_long_frame = MC_DchSlotsPerLongFrame();

   /* supress warning */
   (void)gParentRoleNames;

   /* check inputs */
   if (dch_slots_per_long_frame <= node_id )
   {
      return false;
   }

   /* update our local records for the missing heartbeat*/
   /* step through list of parents and tracking nodes */
   for (index = 0; index < NUMBER_OF_PARENTS_AND_TRACKING_NODES; index++)
   {
      if (gParent[index].NodeID == node_id)
      {
         neighbourArray[node_id].LostHeartbeatCount++;
         gParent[index].PublishedRank = UNASSIGNED_RANK;
         
         if ( MAX_MISSED_HEARTBEATS <= neighbourArray[node_id].LostHeartbeatCount )
         {
            //Make sure we are not already pinging
            if ( PR_IDLE_E == neighbourArray[node_id].PingState )
            {
               if ( ((PRIMARY_PARENT == index) && !gPreformedPrimaryLinkBroken) ||
                    ((SECONDARY_PARENT == index) && !gPreformedSecondaryLinkBroken) ||
                    ((PRIMARY_PARENT != index) && (SECONDARY_PARENT != index)) )
               {
                  //Start ping recovery procedure
                  MC_PR_StartPingForNode(node_id);
               }
            }
         }
         CO_PRINT_B_3(DBG_INFO_E, "node %d (%s) missed heartbeat count = %d\n\r", node_id,
                      gParentRoleNames[index], neighbourArray[node_id].LostHeartbeatCount);
         updated = true;
         foundFlag = true;
         break;
      }
   }

   if (false == foundFlag)
   {
      /* step through list of children */
      uint32_t max_children = CFG_GetMaxNumberOfChildren();
      for ( uint32_t index = 0; index < max_children; index ++ )
      {
         if (gChildren[index] == node_id)
         {
            neighbourArray[node_id].LostHeartbeatCount++;
            
            if ( MAX_MISSED_HEARTBEATS <= neighbourArray[node_id].LostHeartbeatCount )
            {
               //Make sure we are not already pinging
               if ( PR_IDLE_E == neighbourArray[node_id].PingState )
               {
                  //Start ping recovery procedure
                  MC_PR_StartPingForNode(node_id);
               }
            }

            CO_PRINT_B_2(DBG_INFO_E, "node %d (child) missed heartbeat count = %d\n\r", node_id, neighbourArray[node_id].LostHeartbeatCount);
            updated = true;
            break;
         }
      }
   }
   
   // Check for free running (when just sync'd and initial tracking node disapears, the RBU free runs forever)
   // Not done in the NCU
   if ( false == Is_Master )
   {
      //Only perform free running checks if logged on.
      if ( MC_SMGR_IsLoggedOn() )
      {
         if (0 == MC_SMGR_GetNumberOfParents() )
         {
            uint16_t initial_tracking_node = MC_SYNC_GetSyncNode();
            if ( node_id == initial_tracking_node )
            {
               gInitialTrackingNodeMissedHeartbeatCount++;
               if ( MAX_MISSED_HEARTBEATS <= gInitialTrackingNodeMissedHeartbeatCount )
               {
                  gFreeRunningDetected = true;
               }
            }
         }
      }
   }

   //If we received a missed heartbeat from a node that isn't a child or a parent, we have slots open
   //that shouldn't be.  Update the DCH behaviour. Not applied for preformed mesh
   if ( false == CFG_UsingPreformedMesh() )
   {
      if ( (false == updated) && (STATE_ACTIVE_E == MC_STATE_GetDeviceState()) && (false == MC_MFH_ScanForDevicesInProgress()))
      {
         CO_PRINT_B_1(DBG_INFO_E,"Stopping heartbeat monitor for non-parent/child node %d\r\n", node_id);
         MC_SMGR_StopHeartbeatMonitorForNode(node_id);
      }
   }
   
   return updated;
}

/*****************************************************************************
*  Function:   MC_SMGR_ParentSessionIsActive
*  Description:         Reports whether we have an open session for uplink messages.
*  Param -              None.
*  Returns:             true if one or both parents are active.  False if neither is.
*  Notes:               Should be called by the state machine after a missed heartbeat
*                       to determine whether we have dropped out of the Mesh.
*****************************************************************************/
bool MC_SMGR_ParentSessionIsActive(void)
{
   return gParent[PRIMARY_PARENT].IsActive || gParent[SECONDARY_PARENT].IsActive;
}

/*****************************************************************************
*  Function:   MC_SMGR_SetHeartbeatMonitor_NodeId
*  Description:         Configure for expected heartbeat from a child or parent node.
*  Param - node_id:     The network address of the node.
*  Param - set_monitor: True to start heartbeat monitoring, false to stop it.
*  Returns:             true if the heartbeat monitor was updated, false otherwise.
*  Notes:               Should be called for each added parent or child node.
*****************************************************************************/
bool MC_SMGR_SetHeartbeatMonitor_NodeId(const uint16_t node_id, const bool set_monitor)
{
   bool updated = false;
   
   // If the node id is out of range, return false.
   if (MAX_DEVICES_PER_SYSTEM <= node_id)
   {
      return updated;
   }

   // Set the behaviour for this slot
   MC_TDM_DCHBehaviour_t behaviour;

   if (set_monitor)
   {
      if ( node_id == gNetworkAddress )
      {
         behaviour = MC_TDM_DCH_TX_E;
      }
      else if ( MC_SMGR_IsATrackingNode(node_id) )
      {
         behaviour = MC_TDM_DCH_TRACKING_E;
      }
      else
      {
         behaviour = MC_TDM_DCH_RX_E;
      }
   }
   else
   {
      if (node_id == gNetworkAddress)
      {
         behaviour = MC_TDM_DCH_TX_E;
      }
      else if (MC_SMGR_IsATrackingNode(node_id))
      {
         behaviour = MC_TDM_DCH_TRACKING_E;
      }
      else
      {
         behaviour = MC_TDM_DCH_SLEEP_E;
      }
   }
   /* Tell the MAC to change the slot behaviour */
   updated = MC_TDM_SetDCHSlotBehaviour(node_id, behaviour);

   return updated;
}

/*****************************************************************************
*  Function:   MC_SMGR_StartHeartbeatMonitorForNode
*  Description:         Start monitoring the heartbeat from a child or parent node.
*  Param - node_id:     The network address of the node.
*  Returns:             true if the heartbeat monitor was updated, false otherwise.
*  Notes:               Should be called for each added parent or child node.
*****************************************************************************/
bool MC_SMGR_StartHeartbeatMonitorForNode(const uint16_t node_id)
{
   bool result = false;

   if (MC_SMGR_SetHeartbeatMonitor_NodeId(node_id, true))
   {
      result = true;
      CO_PRINT_B_1(DBG_INFO_E, "Started monitoring heartbeat for node %d\n\r", node_id);
   }
   else
   {
      CO_PRINT_B_1(DBG_ERROR_E, "FAILED to start heartbeat monitor for node %d\n\r", node_id);
   }
   return result;
}

/*****************************************************************************
*  Function:   MC_SMGR_StopHeartbeatMonitorForNode
*  Description:         Stop monitoring the heartbeat from a child or parent node.
*  Param - node_id:     The network address of the node.
*  Returns:             true if the heartbeat monitor was updated, false otherwise.
*  Notes:               Should be called for each removed parent or child node.
*****************************************************************************/
bool MC_SMGR_StopHeartbeatMonitorForNode(const uint16_t node_id)
{
   bool result = false;

   if (MC_SMGR_SetHeartbeatMonitor_NodeId(node_id, false))
   {
      result = true;
      CO_PRINT_B_1(DBG_INFO_E, "Stopped monitoring heartbeat for node %d\n\r", node_id);
   }
   else
   {
      CO_PRINT_B_1(DBG_ERROR_E, "FAILED to remove heartbeat monitor for node %d\n\r", node_id);
   }
   
   return result;
}


/*****************************************************************************
*  Function:   MC_SMGR_SendStatusIndication
*  Description:      Trigger a Status Indication message to the NCU indicating the
*                    current mesh network status
*  Param - event        The network status event that is reported.
*  Param - nodeID       The node ID associated with the network status event.
*  Param - nodeData     Data value associated with the event.
*  Param - sendOnDulch  TRUE if the message should go on DULCH.
*  Returns:             None.
*  Notes:
*****************************************************************************/
void MC_SMGR_SendStatusIndication(const CO_MeshEvent_t event, const uint16_t nodeID, const uint32_t nodeData, const bool sendOnDulch)
{
   /*All session manager status changes are internal to the device so
      set the statusEvent.NcuRequested parameter to false */
   ApplicationMessage_t appMessage;
   appMessage.MessageType = APP_MSG_TYPE_STATUS_SIGNAL_E;
   CO_StatusIndicationData_t statusEvent;
   statusEvent.Event = event;
   statusEvent.EventNodeId = nodeID;
   statusEvent.EventData = nodeData;
   statusEvent.DelaySending = sendOnDulch;
   memcpy(appMessage.MessageBuffer, &statusEvent, sizeof(CO_StatusIndicationData_t));
   SM_SendEventToApplication( CO_MESSAGE_MESH_STATUS_EVENT_E, &appMessage );
}

/*****************************************************************************
*  Function:   MC_SMGR_SetAllDCHSlotBehaviour
*  Description:      Set all DCH slot actions. Sets 'defaultBehaviour' for slots 
*  that are not transmit slots, tracking node slots or child node slots.
*  Param - MC_MAC_SlotBehaviour_t defaultBehaviour:  The default behaviour.
*  Returns:          None.
*  Notes:
*****************************************************************************/
void MC_SMGR_SetAllDCHSlotBehaviour(const MC_TDM_DCHBehaviour_t defaultBehaviour)
{
   MC_TDM_DCHBehaviour_t behaviour;
   uint16_t dch_slots_per_long_frame = MC_DchSlotsPerLongFrame();
   
   // Loop through all elements of the array gDCHSlotBehaviour (which is accessed in the function MC_MAC_SetDCHSlotBehaviour).
   for ( uint16_t dch_index = 0; dch_index < dch_slots_per_long_frame; dch_index++ )
   {
      if ( dch_index == gNetworkAddress )
      {
         // Set to transmit in the DCH slot allocated to this device.
         behaviour = MC_TDM_DCH_TX_E;
      }
      else if ( MC_SMGR_IsATrackingNode(dch_index) )
      {
         // Set to Rx Tracking for any DCH slots that are used as tracking nodes by the sync module.
         // This will include parent nodes.
         behaviour = MC_TDM_DCH_TRACKING_E;
      }
      else if ( MC_SMGR_IsAChild(dch_index) )
      {
         // Set as RX for any child nodes.
         behaviour = MC_TDM_DCH_RX_E;
      }
      else
      {
         // Set 'defaultBehaviour' for slots that are not transmit slots, tracking node slots or child node slots.
         behaviour = defaultBehaviour;
      }
      
      MC_TDM_SetDCHSlotBehaviour( dch_index, behaviour );
   }
}

/*****************************************************************************
*  Function:   MC_SMGR_IsATrackingNode
*  Description:      Checks the tracking node list for the supplied node ID.
*  Param - node_id:  The network address of the node.
*  Returns:          true if the node ID is in the tracking node list, false otherwise.
*  Notes:
*****************************************************************************/
bool MC_SMGR_IsATrackingNode(const uint16_t node_id)
{
   bool result = false;

   if ( (node_id == gParent[PRIMARY_PARENT].NodeID) ||
        (node_id == gParent[SECONDARY_PARENT].NodeID) ||
        (node_id == gParent[PRIMARY_TRACKING_NODE].NodeID) ||
        (node_id == gParent[SECONDARY_TRACKING_NODE].NodeID) )
   {
      result = true;
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MC_SMGR_GetNeighbourInformation
*  Description:      Reads the rssi and node ID for a specified neighbour.
*  Param - neighbour_type:  '0' for a parent, '1' for a child.
*  Param - neighbour_index: The index into the child list to identify the neighbour.
*  Param - pNeighbourInfo: [OUT] parameter to return the result.
*  Returns:          true if the neighbour was found, false otherwise.
*  Notes:            
*****************************************************************************/
bool MC_SMGR_GetNeighbourInformation(const uint8_t neighbour_type, const uint8_t neighbour_index, uint32_t* pNeighbourInfo)
{
   bool bNeighbourFound = false;
   uint32_t neighbourInfo = 0;
   
   if ( pNeighbourInfo )
   {
      if ( NEIGHBOUR_TYPE_PARENT == neighbour_type )
      {
         if ( NUMBER_OF_PARENTS_AND_TRACKING_NODES > neighbour_index )
         {
            if ( PRIMARY_PARENT == neighbour_index )
            {
               /* is the primary parent asigned? */
               if (CO_BAD_NODE_ID != gParent[PRIMARY_PARENT].NodeID)
               {
                  neighbourInfo = gParent[PRIMARY_PARENT].NodeID;
                  neighbourInfo |= ((uint32_t)neighbourArray[gParent[PRIMARY_PARENT].NodeID].SNR << sizeof(uint16_t));
                  bNeighbourFound = true;
               }
            }
            else if ( SECONDARY_PARENT == neighbour_index )
            {
               /* is the secondary parent asigned? */
               if (CO_BAD_NODE_ID != gParent[SECONDARY_PARENT].NodeID)
               {
                  neighbourInfo = gParent[SECONDARY_PARENT].NodeID;
                  neighbourInfo |= ((uint32_t)neighbourArray[gParent[SECONDARY_PARENT].NodeID].SNR << sizeof(uint16_t));
                  bNeighbourFound = true;
               }
            }
         }
      }
      else if ( NEIGHBOUR_TYPE_CHILD == neighbour_type )
      {
         uint32_t neighbourCount = 0;
         uint32_t max_children = CFG_GetMaxNumberOfChildren();
         for (uint32_t index = 0; index < max_children; index++)
         {             
            if ( CO_BAD_NODE_ID != gChildren[index] )
            {
               if ( neighbourCount == neighbour_index )
               {
                  neighbourInfo = neighbourArray[gChildren[index]].NodeID;
                  neighbourInfo |= ((uint32_t)neighbourArray[gChildren[index]].SNR << sizeof(uint16_t) );
                  bNeighbourFound = true;
                  break;
               }
               neighbourCount++;
            }
         }
      }

      if ( bNeighbourFound )
      {
         *pNeighbourInfo = neighbourInfo & 0xffffff;
      }
      else
      {
         *pNeighbourInfo = 0xffffffff;
      }
   }
   

   return bNeighbourFound;
}

/*****************************************************************************
*  Function:   void MC_SMGR_ChangeNodeRole
*  Description:      Routine to change a node to a different location is the list of
*                    parent nodes and tracking nodes.
*  Param - srcIdx:   The index in parents nodes to move data from
*  Param - dstIdx:   The index in parents nodes to move data to
*  Returns: bool     True if the operation succeeds
*  Notes:            
*****************************************************************************/
bool MC_SMGR_ChangeNodeRole(const uint8_t srcIdx, const uint8_t dstIdx)
{
   bool result = true;
   
   //Busy time for debug out.  Give it time to clear
   osDelay(100);
   
   if ( (NUMBER_OF_PARENTS_AND_TRACKING_NODES > srcIdx) && (NUMBER_OF_PARENTS_AND_TRACKING_NODES > dstIdx) && (MAX_DEVICES_PER_SYSTEM > gParent[srcIdx].NodeID) )
   {
      CO_PRINT_B_3(DBG_INFO_E, "Changing role of node %d from %s to %s\n\r", gParent[srcIdx].NodeID, gParentRoleNames[srcIdx], gParentRoleNames[dstIdx]);
      
      //If the primary parent is being replaced, we need to change the sync node
      // to its successor and send a route add request
      if ( PRIMARY_PARENT == dstIdx )
      {
         MC_SYNC_SetSyncNode(gParent[srcIdx].NodeID);
         CO_PRINT_B_2(DBG_INFO_E, "Opening session for new %s - Network ID=%d\n\r", gParentRoleNames[srcIdx], gParent[srcIdx].NodeID);
         gParent[dstIdx].IsActive = false;
         if ( MC_SMGR_ConnectToParent(gParent[srcIdx].NodeID, false) )
         {
            CO_PRINT_B_2(DBG_INFO_E, "Join Request sent to %s - Network ID=%d\n\r", gParentRoleNames[srcIdx], gParent[srcIdx].NodeID);
         }
         else 
         {
            result = false;
         }
      }
      
      if ( SECONDARY_PARENT == dstIdx )
      {
         CO_PRINT_B_2(DBG_INFO_E, "Opening session for new %s - Network ID=%d\n\r", gParentRoleNames[srcIdx], gParent[srcIdx].NodeID);
         gParent[dstIdx].IsActive = false;
         if ( MC_SMGR_ConnectToParent(gParent[srcIdx].NodeID, false) )
         {
            CO_PRINT_B_2(DBG_INFO_E, "Join Request sent to %s - Network ID=%d\n\r", gParentRoleNames[srcIdx], gParent[srcIdx].NodeID);
         }
         else 
         {
            result = false;
         }
      }

      /* cancel any source node activity */
      neighbourArray[gParent[srcIdx].NodeID].FailedResponseCount = 0;
      gParent[srcIdx].RouteAddRetries = 0;
      gParent[srcIdx].ShortFramesSinceRouteAdd = 0;
      gParent[srcIdx].WaitingForRouteAddResponse = false;
      
      /* copy src data to dst */
      gParent[dstIdx] = gParent[srcIdx];

      /* set src to inactive */
      neighbourArray[gParent[srcIdx].NodeID].state = STATE_NOT_DEFINED_E;
      neighbourArray[gParent[srcIdx].NodeID].FailedResponseCount = 0;
      neighbourArray[gParent[srcIdx].NodeID].LostHeartbeatCount = 0;
      neighbourArray[gParent[srcIdx].NodeID].Rank = UNASSIGNED_RANK;
      neighbourArray[gParent[srcIdx].NodeID].PingState = PR_IDLE_E;
      gParent[srcIdx].NodeID = CO_BAD_NODE_ID;
      gParent[srcIdx].IsActive = false;
      gParent[srcIdx].PublishedRank = UNASSIGNED_RANK;
   }
   else 
   {
      CO_PRINT_B_3(DBG_ERROR_E, "FAILED to change role of node %d from %s to %s\n\r", gParent[srcIdx].NodeID, gParentRoleNames[srcIdx], gParentRoleNames[dstIdx]);
      result = false;
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MC_SMGR_CheckAndFixParentConnections
*  Description:      Checks the current state of connections to parents and tracking nodes.
*                    If there are spaces available then the action that is best for healing
*                    the mesh is selected and then actioned.
*                    and actioned.
*  Param -           None
*  Returns:          None
*  Notes:            
*****************************************************************************/
void MC_SMGR_CheckAndFixParentConnections()
{
   uint8_t primaryParentActive = CONV_BOOL_TO_INT(CO_BAD_NODE_ID != gParent[PRIMARY_PARENT].NodeID);
   uint8_t secondaryParentActive = CONV_BOOL_TO_INT(CO_BAD_NODE_ID != gParent[SECONDARY_PARENT].NodeID);
   uint8_t primaryTrackingNodeAssigned = CONV_BOOL_TO_INT(CO_BAD_NODE_ID != gParent[PRIMARY_TRACKING_NODE].NodeID);
   uint8_t secondaryTrackingNodeAssigned = CONV_BOOL_TO_INT(CO_BAD_NODE_ID != gParent[SECONDARY_TRACKING_NODE].NodeID);
   
   /* create bitmap containing the state of all parents and tracking nodes */
   uint8_t parentMap =  primaryParentActive << PRIMARY_PARENT |
                        secondaryParentActive << SECONDARY_PARENT |
                        primaryTrackingNodeAssigned << PRIMARY_TRACKING_NODE |
                        secondaryTrackingNodeAssigned << SECONDARY_TRACKING_NODE;

   /* check connections to parents and tracking nodes and decide what to do */
   CO_PRINT_B_1(DBG_INFO_E, "Check and Fix parent connections (%d)\n\r", parentMap);
   if (0x0 == parentMap)
   {
      /* no active parents or tracking nodes - go back to the synchronisation stage (node loss process) */
      //Error_Handler("No parents or tracking nodes left - call re-start\n\r");
      MC_SMGR_DelayedReset("No parents or tracking nodes left - call re-start\n\r");
      //Send 3 route drop messages to the global address so that other RBUs can discard this node.
      for (uint32_t repeat = 0; repeat < 3; repeat++)
      {
         MC_SMGR_SendRouteDrop(ADDRESS_GLOBAL, CO_RESET_REASON_NO_PARENTS_LEFT_E, false, false,0);
      }
      
   }
   else if (0xf == parentMap)
   {
      /* all parents and tracking nodes active so no action required */
   }
   else
   {
      /* mixture of parents / tracking nodes active and not active.
       * check if we need to re-organise the existing connections */
      if (0x2 == parentMap)
      {
         /* send a status indication message to the NCU */
         MC_SMGR_SendStatusIndication(CO_MESH_EVENT_SECONDARY_PARENT_PROMOTED_E, gParent[SECONDARY_PARENT].NodeID, 0, true);
         CO_PRINT_A_2(DBG_DATA_E,"%d,%d\r\n", GUI_EVENT_PROMOTED_SECONDARY_PARENT_E, gParent[SECONDARY_PARENT].NodeID);
         
         /* change secondary parent to primary parent */
         if ( false == MC_SMGR_ChangeNodeRole(SECONDARY_PARENT, PRIMARY_PARENT) )
         {
            Error_Handler("Failed to replace primary parent with SP");
         }
      }
      else if (0x4 == parentMap)
      {
         /* change primary tracking node to primary parent */
         MC_SMGR_ChangeNodeRole(PRIMARY_TRACKING_NODE, PRIMARY_PARENT);
         if ( false == MC_SMGR_ChangeNodeRole(PRIMARY_TRACKING_NODE, PRIMARY_PARENT) )
         {
            Error_Handler("Failed to replace primary parent with P-Trk");
         }
      }
      else if (0x5 == parentMap)
      {
         /* send a status indication message to the NCU */
         MC_SMGR_SendStatusIndication(CO_MESH_EVENT_PRIMARY_TRACKING_NODE_PROMOTED_E, gParent[PRIMARY_TRACKING_NODE].NodeID, neighbourArray[gParent[PRIMARY_TRACKING_NODE].NodeID].SNR, true);
         CO_PRINT_A_2(DBG_DATA_E,"%d,%d\r\n", GUI_EVENT_PROMOTED_PRIMARY_TRACKING_NODE_E, gParent[PRIMARY_TRACKING_NODE].NodeID);

         /* change primary tracking node to secondary parent */
         MC_SMGR_ChangeNodeRole(PRIMARY_TRACKING_NODE, SECONDARY_PARENT);
      }
      else if (0x6 == parentMap)
      {
         CO_PRINT_A_2(DBG_DATA_E,"%d,%d\r\n", GUI_EVENT_PROMOTED_SECONDARY_PARENT_E, gParent[SECONDARY_PARENT].NodeID);
         CO_PRINT_A_2(DBG_DATA_E,"%d,%d\r\n", GUI_EVENT_PROMOTED_PRIMARY_TRACKING_NODE_E, gParent[PRIMARY_TRACKING_NODE].NodeID);
         /* change secondary parent to primary parent */
         if ( false == MC_SMGR_ChangeNodeRole(SECONDARY_PARENT, PRIMARY_PARENT) )
         {
            Error_Handler("Failed to replace primary parent with SP");
         }

         /* change primary tracking node to secondary parent */
         MC_SMGR_ChangeNodeRole(PRIMARY_TRACKING_NODE, SECONDARY_PARENT);
      }
      else if (0x8 == parentMap)
      {
         /* change secondary tracking node to primary parent */
         if ( false == MC_SMGR_ChangeNodeRole(SECONDARY_TRACKING_NODE, PRIMARY_PARENT) )
         {
            Error_Handler("Failed to replace primary parent with S-Trk");
         }
      }
      else if (0x9 == parentMap)
      {
         /* change secondary tracking node to secondary parent */
         MC_SMGR_ChangeNodeRole(SECONDARY_TRACKING_NODE, SECONDARY_PARENT);
      }
      else if (0xa == parentMap)
      {
         /* change secondary parent to primary parent */
         if ( false == MC_SMGR_ChangeNodeRole(SECONDARY_TRACKING_NODE, PRIMARY_PARENT) )
         {
            Error_Handler("Failed to replace primary parent with S-Trk");
         }

         /* change secondary tracking node to secondary parent */
         MC_SMGR_ChangeNodeRole(SECONDARY_TRACKING_NODE, SECONDARY_PARENT);
      }
      else if (0xb == parentMap)
      {
         CO_PRINT_A_2(DBG_DATA_E,"%d,%d\r\n", GUI_EVENT_PROMOTED_SECONDARY_TRACKING_NODE_E, gParent[SECONDARY_TRACKING_NODE].NodeID);
         /* change secondary tracking node to primary tracking node */
         MC_SMGR_ChangeNodeRole(SECONDARY_TRACKING_NODE, PRIMARY_TRACKING_NODE);
      }
      else if (0xc == parentMap)
      {
         /* change primary tracking node to primary parent */
         if ( false == MC_SMGR_ChangeNodeRole(PRIMARY_TRACKING_NODE, PRIMARY_PARENT) )
         {
            Error_Handler("Failed to replace primary parent with P-Trk");
         }

         /* change secondary tracking node to secondary parent */
         MC_SMGR_ChangeNodeRole(SECONDARY_TRACKING_NODE, SECONDARY_PARENT);
      }
      else if (0xd == parentMap)
      {
         /* send a status indication message to the NCU */
         MC_SMGR_SendStatusIndication(CO_MESH_EVENT_PRIMARY_TRACKING_NODE_PROMOTED_E, gParent[PRIMARY_TRACKING_NODE].NodeID, neighbourArray[gParent[PRIMARY_TRACKING_NODE].NodeID].SNR, true);
         
         CO_PRINT_A_2(DBG_DATA_E,"%d,%d\r\n", GUI_EVENT_PROMOTED_PRIMARY_TRACKING_NODE_E, gParent[PRIMARY_TRACKING_NODE].NodeID);
         CO_PRINT_A_2(DBG_DATA_E,"%d,%d\r\n", GUI_EVENT_PROMOTED_SECONDARY_TRACKING_NODE_E, gParent[SECONDARY_TRACKING_NODE].NodeID);

         /* change primary tracking node to secondary parent */
         MC_SMGR_ChangeNodeRole(PRIMARY_TRACKING_NODE, SECONDARY_PARENT);
         
         /* change secondary tracking node to primary tracking node */
         MC_SMGR_ChangeNodeRole(SECONDARY_TRACKING_NODE, PRIMARY_TRACKING_NODE);
      }
      else if (0xe == parentMap)
      {
         /* send a status indication message to the NCU */
         MC_SMGR_SendStatusIndication(CO_MESH_EVENT_SECONDARY_PARENT_PROMOTED_E, gParent[SECONDARY_PARENT].NodeID, 0, true);

         CO_PRINT_A_2(DBG_DATA_E,"%d,%d\r\n", GUI_EVENT_PROMOTED_SECONDARY_PARENT_E, gParent[SECONDARY_PARENT].NodeID);
         CO_PRINT_A_2(DBG_DATA_E,"%d,%d\r\n", GUI_EVENT_PROMOTED_PRIMARY_TRACKING_NODE_E, gParent[PRIMARY_TRACKING_NODE].NodeID);
         CO_PRINT_A_2(DBG_DATA_E,"%d,%d\r\n", GUI_EVENT_PROMOTED_SECONDARY_TRACKING_NODE_E, gParent[SECONDARY_TRACKING_NODE].NodeID);
         /* change secondary parent to primary parent */
         if ( false == MC_SMGR_ChangeNodeRole(SECONDARY_PARENT, PRIMARY_PARENT) )
         {
            Error_Handler("Failed to replace primary parent with SP");
         }

         /* change primary tracking node to secondary parent */
         MC_SMGR_ChangeNodeRole(PRIMARY_TRACKING_NODE, SECONDARY_PARENT);

         /* change secondary tracking node to primary tracking node */
         MC_SMGR_ChangeNodeRole(SECONDARY_TRACKING_NODE, PRIMARY_TRACKING_NODE);
      }
      else
      {
         /* no moves required - the connections already have the best configuration */
      }
      
      /* check if we dont have the full set of 2 active parents */
      if ((2 > MC_SMGR_GetNumberOfParents()) && (ADDRESS_NCU != gParent[PRIMARY_PARENT].NodeID))
      {
         /* start the parent refresh process to get some more */
         CO_PRINT_B_0(DBG_INFO_E, "We still dont have 2 parents so run Parent Refresh Process\n\r");
         MC_MFH_SelectNewParent();
      }
   }
}

/*****************************************************************************
*  Function:   MC_SMGR_CheckParentStates
*  Description:      Checks the state received in the heartbeats from parents and tracking nodes.
*
*                    The function checks the following:
*                      - there is at least 1 assigned parent/tracking node
*                      - the state of any active parent node >= the local state
*                      - the parent/tracking nodes are not at a more advanced meash state.
*  Param - localState:     state of this RBU
*  Param - LongFrameIndex: The current long frame index
*  Returns:                true=reset; false=do not reset
*  Notes:            
*****************************************************************************/
bool MC_SMGR_CheckParentStates(const CO_State_t localState, const uint8_t LongFrameIndex)
{
   uint32_t index = 0;
   bool foundActiveParent = false;
   bool foundGoodState = false;
   
   /* step through list of parent/tracking nodes */
   for (index = PRIMARY_PARENT; index < NUMBER_OF_PARENTS_AND_TRACKING_NODES; index++)
   {
      /* check for allocated node ID */
      if (CO_BAD_NODE_ID != gParent[index].NodeID)
      {
         foundActiveParent = true;

         /* check remote state >= local state */
         if (neighbourArray[gParent[index].NodeID].state >= localState)
         {
            foundGoodState = true;
            break;
         }
      }
   }

   /* reset if we found an active parent and we did not find a good state */
   return foundActiveParent && ! foundGoodState;
}

/*****************************************************************************/
/**
*  Function:   MC_SMGR_ReportAssociatedNodes
*  Description:      Prints details of assicociated nodes to the DEBUG USART.
*                    It prints the rank followed by a list of <nodeID,SNR> pairs.
*                    The parent and tracking nodes are always listed, followed
*                    by 0 or more entries for child nodes.
*                    A value of -1 is printed for missing Parents/Tracking nodes
*                    The format of the output is:
*                    NODES: R P,snr S,snr T1,snr T2,snr N1,snr N2,snr .....
*
*                    Where R=Rank P-PParent S=SParent T1=PTracking T2=STracking
*                    N=Child Node
*
*  @param   None
*  @return  None.
*            
*****************************************************************************/
void MC_SMGR_ReportAssociatedNodes(const uint32_t source)
{
   const uint32_t MAX_STRING_LENGTH = SERIAL_PORT_0_TX_BUFFER_SIZE - 12;
   static char node_info[SERIAL_PORT_0_TX_BUFFER_SIZE];
   
   int32_t primary_parent_nodeid = -1;
   int32_t primary_parent_snr = -1;
   int32_t secondary_parent_nodeid = -1;
   int32_t secondary_parent_snr = -1;
   int32_t primary_tracking_nodeid = -1;
   int32_t primary_tracking_snr = -1;
   int32_t secondary_tracking_nodeid = -1;
   int32_t secondary_tracking_snr = -1;
   
   if ( CO_BAD_NODE_ID != gParent[PRIMARY_PARENT].NodeID )
   {
      primary_parent_nodeid = (int32_t)gParent[PRIMARY_PARENT].NodeID;
      primary_parent_snr = (int32_t)neighbourArray[primary_parent_nodeid].SNR;
   }
   
   if ( CO_BAD_NODE_ID != gParent[SECONDARY_PARENT].NodeID )
   {
      secondary_parent_nodeid = (int32_t)gParent[SECONDARY_PARENT].NodeID;
      secondary_parent_snr = (int32_t)neighbourArray[secondary_parent_nodeid].SNR;
   }
   
   if ( CO_BAD_NODE_ID != gParent[PRIMARY_TRACKING_NODE].NodeID )
   {
      primary_tracking_nodeid = (int32_t)gParent[PRIMARY_TRACKING_NODE].NodeID;
      primary_tracking_snr = (int32_t)neighbourArray[primary_tracking_nodeid].SNR;
   }
   
   if ( CO_BAD_NODE_ID != gParent[SECONDARY_TRACKING_NODE].NodeID )
   {
      secondary_tracking_nodeid = (int32_t)gParent[SECONDARY_TRACKING_NODE].NodeID;
      secondary_tracking_snr = (int32_t)neighbourArray[secondary_tracking_nodeid].SNR;
   }
   
   sprintf(node_info,"%d %d,%d %d,%d %d,%d %d,%d ",
            MC_SMGR_GetRank(),
            primary_parent_nodeid, primary_parent_snr,
            secondary_parent_nodeid, secondary_parent_snr,
            primary_tracking_nodeid, primary_tracking_snr,
            secondary_tracking_nodeid, secondary_tracking_snr );
   
   /* tag on the children */
   char child_info[16];
   uint32_t node_info_length = strlen(node_info);
   uint32_t max_children = CFG_GetMaxNumberOfChildren();
   for (uint32_t index = 0; (index < max_children) && (MAX_STRING_LENGTH > node_info_length); index++)
   {             
      if ( CO_BAD_NODE_ID != gChildren[index])
      {
         sprintf(child_info,"%d,%d ", neighbourArray[gChildren[index]].NodeID, neighbourArray[gChildren[index]].SNR);
         if ( SERIAL_PORT_0_TX_BUFFER_SIZE >= (node_info_length + strlen(child_info) ) )
         {
            strcat(node_info, child_info);
            node_info_length = strlen(node_info);
         }
         else
         {
            break; /* Can't fit any more characters in the buffer */
         }
      }      
   }

   /* send the report to the debug usart as a +DAT */
   //CO_PRINT_A_2(DBG_DATA_E, "%d,%s\r\n", CO_MESH_EVENT_REPORT_NODES_E, node_info);
   SerialDebug_Print((UartComm_t)source, DBG_INFO_E, "%d,%s\r\n", CO_MESH_EVENT_REPORT_NODES_E, node_info);
}

/*****************************************************************************/
/**
*  Function:   MC_SMGR_DeleteNode
*  Description:       Delete a parent, child or tracking node from the session manager.
*  @param - node_id:  The network address of the parent/tracking node. 
*  @return            true if a node was deleted, false otherwise.
*  Notes:            
*****************************************************************************/
bool MC_SMGR_DeleteNode( const uint16_t node_id, const char* reason )
{
   
   bool status = false;
   uint16_t index;
   
   if ( MAX_DEVICES_PER_SYSTEM > node_id  )
   {
      if ( reason )
      {
         CO_PRINT_A_2(DBG_INFO_E,"Discarding node %d.  Reason = %s\r\n", node_id, reason);
      }
      else
      {
         CO_PRINT_A_1(DBG_INFO_E,"Discarding node %d.  Reason = not given\r\n", node_id);
      }
      
      //Mark the node as 'unused' in mesh forming and healing
      MC_MFH_FreeNode(node_id);
      
      if ( true == MC_SMGR_IsATrackingNode(node_id) || true == MC_SMGR_IsParentNode(node_id))
      {
         if ( node_id == gParent[PRIMARY_PARENT].NodeID )
         {
            status = true;
            index = PRIMARY_PARENT;
            /* Send a status message to NCU if the parent was active*/
            if ( gParent[PRIMARY_PARENT].IsActive )
            {
               MC_SMGR_SendStatusIndication( CO_MESH_EVENT_PRIMARY_PARENT_DROPPED_E, node_id, 0, true );
            }
            /* There may be messages in the ACK Manager waiting to be sent. Have them redirected */
            MC_ACK_RedirectMessagesForNode(node_id);
            /* Also redirect messages waiting in the SACH queues */
            MC_SACH_RedirectMessagesForNode(node_id);
            
            CO_PRINT_A_2(DBG_DATA_E,"%d,%d\r\n", GUI_EVENT_DROPPED_PRIMARY_PARENT_E, node_id);
         }
         else if ( node_id == gParent[SECONDARY_PARENT].NodeID )
         {
            status = true;
            index = SECONDARY_PARENT;
            /* Send a status message to NCU if the parent was active*/
            if ( gParent[SECONDARY_PARENT].IsActive )
            {
               MC_SMGR_SendStatusIndication( CO_MESH_EVENT_SECONDARY_PARENT_DROPPED_E, node_id, 0, true );
            }
            /* There may be messages in the ACK Manager waiting to be sent. Have them redirected */
            MC_ACK_RedirectMessagesForNode(node_id);
            /* Also redirect messages waiting in the SACH queues */
            MC_SACH_RedirectMessagesForNode(node_id);
            CO_PRINT_A_2(DBG_DATA_E,"%d,%d\r\n", GUI_EVENT_DROPPED_SECONDARY_PARENT_E, node_id);
         }
         else if ( node_id == gParent[PRIMARY_TRACKING_NODE].NodeID )
         {
            status = true;
            index = PRIMARY_TRACKING_NODE;
            /* Send a status message to NCU */
            MC_SMGR_SendStatusIndication( CO_MESH_EVENT_PRIMARY_TRACKING_NODE_DROPPED_E, node_id, 0, true );
         }
         else if ( node_id == gParent[SECONDARY_TRACKING_NODE].NodeID )
         {
            status = true;
            index = SECONDARY_TRACKING_NODE;
            /* Send a status message to NCU */
            MC_SMGR_SendStatusIndication( CO_MESH_EVENT_SECONDARY_TRACKING_NODE_DROPPED_E, node_id, 0, true );
         }
      
         if ( true == status )
         {
            /* delete parent/tracking node from local data */
            gParent[index].NodeID = CO_BAD_NODE_ID;
            gParent[index].IsActive = false;
            gParent[index].WaitingForRouteAddResponse = false;
            gParent[index].ShortFramesSinceRouteAdd = 0;
            gParent[index].PublishedRank = UNASSIGNED_RANK;
            neighbourArray[node_id].Rank = UNASSIGNED_RANK;
            neighbourArray[node_id].state = STATE_NOT_DEFINED_E;
            neighbourArray[node_id].FailedResponseCount = 0;
            
            //Stop ping recovery
            MC_PR_CancelPingForNode(node_id);
         
            /* Stop the heartbeat monitor for the dch slot */
            MC_SMGR_StopHeartbeatMonitorForNode(node_id);

            /* check connections to parents and tracking nodes and fix if necessary */
            MC_SMGR_CheckAndFixParentConnections();
         }
      }
      else if ( 0 < gSMGRNumberOfChildren ) /* check the children, if there are any */
      {
         uint32_t max_children = CFG_GetMaxNumberOfChildren();
         for ( index = 0; index < max_children; index++ )
         {
            if (gChildren[index] == node_id)
            {
               /* delete child node from local data */
               neighbourArray[node_id].NodeID = CO_BAD_NODE_ID;
               neighbourArray[node_id].RSSI = INITIAL_CHILD_RSSI;
               neighbourArray[node_id].SNR = 0;
               gSMGRNumberOfChildren--;
               gChildren[index] = CO_BAD_NODE_ID;
               /* Stop the heartbeat monitor for the dch slot */
               MC_SMGR_StopHeartbeatMonitorForNode(node_id);
               
               //Stop ping recovery
               MC_PR_CancelPingForNode(node_id);

               /* Send a status message to NCU */
               MC_SMGR_SendStatusIndication( CO_MESH_EVENT_CHILD_NODE_DROPPED_E, node_id, 0, false );
               
               CO_PRINT_A_2(DBG_DATA_E,"%d,%d\r\n", GUI_EVENT_DROPPED_CHILD_E, node_id);

               MC_MFH_FreeNode(node_id);
               status = true;
               break;
            }      
         }
      }
   }
   return status;
}


/*****************************************************************************/
/**
*  Function:   MC_SMGR_SendRouteDrop
*  Description:            Send a message to the state machine to generate a
*                          route drop message.
*  @param - node_id:       The network address of the node being dropped.
*  @param - reason:        The reason for the node being dropped.
*  @param - wait_for_ack:  Set to true if we think the node can respond,
*                          false if we think it can't (e.g. lost heartbeat).
*  @param - is_downlink    True id the target node is downlink (child)
*  @param - handle         transaction number for sending conf to the application
*  @return  none.
*  Notes:            
*****************************************************************************/
void MC_SMGR_SendRouteDrop(const uint16_t node_id, const ResetReason_t reason, const bool wait_for_ack, const bool is_downlink, const uint32_t handle)
{
   osStatus osStat = osErrorOS;
   RouteDropData_t routeDropData;
   CO_Message_t* phyDataReq;

   /* Don't allow a bad destination address */
   if( CO_BAD_NODE_ID != node_id )
   {
      /* create a 'generate route drop' message and put into the mesh queue */
      phyDataReq = ALLOCMESHPOOL;
      if (phyDataReq)
      {
         routeDropData.NodeID = node_id;
         routeDropData.Reason = (uint32_t)reason;
         routeDropData.WaitForAck = wait_for_ack;
         routeDropData.Downlink = is_downlink;
         routeDropData.Handle = handle;
        
         phyDataReq->Type = CO_MESSAGE_GENERATE_ROUTE_DROP_REQUEST_E;
         memcpy(phyDataReq->Payload.PhyDataReq.Data, &routeDropData, sizeof(RouteDropData_t));
         osStat = osMessagePut(MeshQ, (uint32_t)phyDataReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            FREEMESHPOOL(phyDataReq);
         }
      }
      
      CO_MeshEvent_t mesh_event;
      switch (reason )
      {
         case CO_RESET_REASON_UNKNOWN_E:
            mesh_event = CO_MESH_EVENT_NONE_E;
            break;
         case CO_RESET_REASON_NO_PARENTS_LEFT_E:
            mesh_event = CO_MESH_EVENT_PRIMARY_PARENT_DROPPED_E;
            break;
         case CO_RESET_REASON_DROPPED_BY_SYNC_NODE_E:
            mesh_event = CO_MESH_EVENT_SYNCH_LOSS_E;
            break;
         case CO_RESET_REASON_NODE_STOPPED_RESPONDING_E:
            mesh_event = CO_MESH_EVENT_COMMS_FAIL_E;
            break;
         case CO_RESET_REASON_MAX_CONNECTION_ATTEMPTS_E:
            mesh_event = CO_MESH_EVENT_COMMS_FAIL_E;
            break;
         case CO_RESET_REASON_FAILED_TO_SEND_ROUTE_ADD_E:
            mesh_event = CO_MESH_EVENT_COMMS_FAIL_E;
            break;
         case CO_RESET_REASON_LINK_REMAKE_FAILED_E:
            mesh_event = CO_MESH_EVENT_COMMS_FAIL_E;
            break;
         case CO_RESET_REASON_FREE_RUNNING_E:
            mesh_event = CO_MESH_EVENT_SYNCH_LOSS_E;
            break;
         case CO_RESET_REASON_CIRCULAR_PATH_E:
            mesh_event = CO_MESH_EVENT_SYNCH_LOSS_E;
            break;
         case CO_RESET_REASON_RESET_COMMAND_E:
            mesh_event = CO_MESH_EVENT_COMMANDED_RESET_E;
            break;
         case CO_RESET_REASON_PPU_MODE_REQUESTED_E:
            mesh_event = CO_MESH_EVENT_PPU_MODE_REQUESTED_E;
            break;
         default:
            mesh_event = CO_MESH_EVENT_NONE_E;
            break;
      }
      
      
      // Inform the application that we have dropped a child
      ApplicationMessage_t appMessage;
      appMessage.MessageType = APP_MSG_TYPE_ROUTE_DROP_E;
      CO_StatusIndicationData_t statusEvent;
      statusEvent.Event = mesh_event;
      statusEvent.EventNodeId = node_id;
      statusEvent.EventData = 0;
      statusEvent.DelaySending = false;
      memcpy(appMessage.MessageBuffer, &statusEvent, sizeof(CO_StatusIndicationData_t));
      SM_SendEventToApplication( CO_MESSAGE_MESH_STATUS_EVENT_E, &appMessage );
   }
}
/*****************************************************************************/
/**
*  Function:   MC_SMGR_GetAverageSNRForTrackingNode
*  Description:     Get the average SNR for one of the parent/tracking nodes.
*
*  @param   TrackingNodeID    The node being queried.
*  @return  int16_t.          The average SNR for the node.
*            
*****************************************************************************/
int16_t MC_SMGR_GetAverageSNRForTrackingNode(const uint16_t TrackingNodeID)
{
   int16_t snr = MIN_RECORDABLE_SNR;
   
   if ( NUMBER_OF_PARENTS_AND_TRACKING_NODES > TrackingNodeID )
   {
      uint16_t node_id = gParent[TrackingNodeID].NodeID;
      snr = MC_MFH_GetSNR(node_id);
   }
   
   return snr;
}

/*****************************************************************************/
/**
*  Function:   MC_SMGR_GetAverageRssiForTrackingNode
*  Description:     Get the average RSSI for one of the parent/tracking nodes.
*
*  @param   TrackingNodeID    The node being queried.
*  @return  int16_t.          The average RSSI for the node.
*            
*****************************************************************************/
int16_t MC_SMGR_GetAverageRssiForTrackingNode(const uint16_t TrackingNodeID)
{
   int16_t rssi = MIN_RECORDABLE_RSSI;
   
   if ( NUMBER_OF_PARENTS_AND_TRACKING_NODES > TrackingNodeID )
   {
      uint16_t node_id = gParent[TrackingNodeID].NodeID;
      rssi = MC_MFH_GetRSSI(node_id);
   }
   
   return rssi;
}

/*****************************************************************************/
/**
*  Function:   MC_SMGR_GetNumberOfChildrenForPrimaryTrackingNode
*  Description:     Get the number of children for one of the primary tracking node. 
*
*  @param   None.
*  @return  uint8_t.   The number of children.
*            
*****************************************************************************/
uint8_t MC_SMGR_GetNumberOfChildrenForPrimaryTrackingNode()
{
   return gNoOfChildrenOfPTNIdx;
}

/*****************************************************************************/
/**
*  Function:   MC_SMGR_NotifyMessageFailure
*  Description:     Called if a message went unacknowledged and was discarded
*                    by the ACK Management. 
*
*  @param   MessageType    The discarded message type.
*  @param   MacDestination The discarded message MAC Destination.
*  @return                 True if the session manager handled the dropped 
*                          message report, false otherwise.
*            
*****************************************************************************/
bool MC_SMGR_NotifyMessageFailure(const ApplicationLayerMessageType_t MessageType, const uint16_t MacDestination)
{
   bool result = false;
   
   switch ( MessageType )
   {
      case APP_MSG_TYPE_ROUTE_ADD_E:
         /* if the MacDestination matches a parent that has not gone active, delete the parent */
         if ( ((MacDestination == gParent[PRIMARY_PARENT].NodeID) && (false == gParent[PRIMARY_PARENT].IsActive)) ||
              ((MacDestination == gParent[SECONDARY_PARENT].NodeID) && (false == gParent[SECONDARY_PARENT].IsActive)) )
           {
               /* We just need to call MC_SMGR_DeleteNode.  It deals with parent reselection etc */
               result = MC_SMGR_DeleteNode(MacDestination, "RouteAdd not acknowledged");
              result = true;
           }
         break;
      default:
         break;
   }
   return result;
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_ProcessParentAckReport
*  Description:      Called by the ACK Management to report when a message is
*                    acknowledged or was unacknowledged. This information is
*                    used to invalidate a non-responsive parent.
*
*  @param   parent_nodeID  The node ID of the parent that the message was sent to.
*  @param   ack_received   True if the message was acknowledged, false if it wasn't.
*  @param   message_type   The type of message being ACK'd
*  @return  None.
*            
*****************************************************************************/
void MC_SMGR_ProcessParentAckReport(const uint16_t parent_nodeID, const bool ack_received, const ApplicationLayerMessageType_t message_type)
{
   /* some rach messages go to non-parents so check that it is a parent */
   if ( MC_SMGR_IsParentNode(parent_nodeID) )
   {
      if ( ack_received )
      {
         /* make sure that the nodeID matches a parent */
         if ( parent_nodeID == gParent[PRIMARY_PARENT].NodeID )
         {
            neighbourArray[parent_nodeID].FailedResponseCount = 0;
            //We are receiving responses so cancel any missed heartbeat count
            neighbourArray[parent_nodeID].LostHeartbeatCount = 0;
            
            /* start the timeout for the Route Add Response */
            if ( APP_MSG_TYPE_ROUTE_ADD_E == message_type )
            {
               gParent[PRIMARY_PARENT].ShortFramesSinceRouteAdd = 0;
               gParent[PRIMARY_PARENT].WaitingForRouteAddResponse = true;
            }
         }
         else if ( parent_nodeID == gParent[SECONDARY_PARENT].NodeID )
         {
            neighbourArray[parent_nodeID].FailedResponseCount = 0;
            //We are receiving responses so cancel any missed heartbeat count
            neighbourArray[parent_nodeID].LostHeartbeatCount = 0;
            
            /* start the timeout for the Route Add Response */
            if ( APP_MSG_TYPE_ROUTE_ADD_E == message_type )
            {
               gParent[SECONDARY_PARENT].ShortFramesSinceRouteAdd = 0;
               gParent[SECONDARY_PARENT].WaitingForRouteAddResponse = true;
            }
            
         }
      }
      else if( false == CFG_UsingPreformedMesh() )
      {
         /* suspend the checks for unresponsive parents if the network is very busy */
         if ((NETWORK_BUSY_THRESHOLD > gSecondaryRachTrafficCount))
         {
            //Ignore ping responses.  MC_PingRecovery manages those.
            if ( APP_MSG_TYPE_PING_E != message_type )
            {
               /* make sure that the nodeID matches an active parent */
               if ( (parent_nodeID == gParent[PRIMARY_PARENT].NodeID) && gParent[PRIMARY_PARENT].IsActive )
               {
                  neighbourArray[parent_nodeID].FailedResponseCount++;
                  CO_PRINT_B_1(DBG_INFO_E,"PRIMARY_PARENT. FailedResponseCount = %d\r\n", neighbourArray[parent_nodeID].FailedResponseCount);
                  //CO_PRINT_B_2(DBG_INFO_E,"Nwk Ld - PR=%d%%, SR=%d%%\r\n", (gPrimaryRachTrafficCount*10), (gSecondaryRachTrafficCount*10));
                  /* check to see if the parent has failed to ACK several consecutive messages */
                  if ( PARENT_NOT_RESPONDING_THRESHOLD <= neighbourArray[parent_nodeID].FailedResponseCount )
                  {
                     CO_PRINT_B_1(DBG_INFO_E,"DISCARDING PRIMARY_PARENT. FailedResponseCount = %d\r\n", neighbourArray[parent_nodeID].FailedResponseCount);
                     /* The parent isn't responding to our messages.  Discard the parent and find a replacement */
                     MC_SMGR_DeleteNode(parent_nodeID, "Primary parent not responding");
                  }
               }
               else if ( (parent_nodeID == gParent[SECONDARY_PARENT].NodeID) && gParent[SECONDARY_PARENT].IsActive )
               {
                  neighbourArray[parent_nodeID].FailedResponseCount++;
                  CO_PRINT_B_1(DBG_INFO_E,"SECONDARY_PARENT. FailedResponseCount = %d\r\n", neighbourArray[parent_nodeID].FailedResponseCount);
                  //CO_PRINT_B_2(DBG_INFO_E,"Nwk Ld - PR=%d%%, SR=%d%%\r\n", (gPrimaryRachTrafficCount*10), (gSecondaryRachTrafficCount*10));
                  /* check to see if the parent has failed to ACK several consecutive messages */
                  if ( PARENT_NOT_RESPONDING_THRESHOLD <= neighbourArray[parent_nodeID].FailedResponseCount )
                  {
                     CO_PRINT_B_1(DBG_INFO_E,"DISCARDING SECONDARY_PARENT. FailedResponseCount = %d\r\n", neighbourArray[parent_nodeID].FailedResponseCount);
                     /* The parent isn't responding to our messages.  Discard the parent and find a replacement */
                     MC_SMGR_DeleteNode(parent_nodeID, "Secondary parent not responding");
                  }
               }
            }
         }
         else 
         {
            CO_PRINT_B_2(DBG_INFO_E,"Network busy - PRach=%d%%, SRach=%d%%\r\n", (gPrimaryRachTrafficCount*10), (gSecondaryRachTrafficCount*10));
         }
      }
   }
}


/*****************************************************************************/
/**
*  Function:         MC_SMGR_ProcessParentStatusReport
*  Description:      The state machine function rbuStatusIndication() intercepts
*                    status indication messages that were not addressed to this
*                    node and sends them here if they originate from a parent.
*                    We check to see if the parent is reporting that it has
*                    dropped this node.  We should have received a RouteDrop
*                    message, but sometimes we miss it due to packet loss.
*
*  @param   pStatusMessage  Pointer to the received status message.
*  @return  None.
*            
*****************************************************************************/
void MC_SMGR_ProcessParentStatusReport( const StatusIndicationMessage_t* const pStatusMessage)
{
   /* check that the message isn't null and it originates from a parent */
   if( pStatusMessage && MC_SMGR_IsParentNode(pStatusMessage->Header.SourceAddress) )
   {
      /* check if the parent reports it has dropped a child node */
      if ( CO_MESH_EVENT_CHILD_NODE_DROPPED_E == pStatusMessage->Event )
      {
         /* check if it was this node that was dropped */
         if ( pStatusMessage->EventData == gNetworkAddress )
         {
            /*  We've been dropped by the parent. Discard the parent */
            MC_SMGR_DeleteNode( pStatusMessage->Header.SourceAddress, "Detected parent msg dropping this node" );
         }
      }
   }
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_UpdateNetworkStatistics
*  Description:      Keeps track of the network utilisation for PRACH and SRACH
*                    to determine how busy the network is.
*
*  @param   channel           PRIMARY_RACH or SECONDARY_RACH.
*  @param   messageDetected   True if a message was detected in a RACH slot.
*  @return  None.
*            
*****************************************************************************/
void MC_SMGR_UpdateNetworkStatistics(const uint8_t channel, const bool messageDetected)
{
   if ( PRIMARY_RACH == channel )
   {
      gPrimaryRachHistory[gPrimaryRachHistoryIndex] = messageDetected;
   }
   else if ( SECONDARY_RACH == channel )
   {
      gSecondaryRachHistory[gSecondaryRachHistoryIndex] = messageDetected;
   }
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_UpdateRachCount
*  Description:      Modifies the index for accessing the network history statistics.
*
*  @param   channel  PRIMARY_RACH or SECONDARY_RACH.
*  @return  None.
*            
*****************************************************************************/
void MC_SMGR_UpdateRachCount(const uint8_t channel)
{
   if ( PRIMARY_RACH == channel )
   {
      gPrimaryRachHistoryIndex++;
      if ( SMGR_MAX_HISTORY_LENGTH == gPrimaryRachHistoryIndex )
      {
         gPrimaryRachHistoryIndex = 0;
      }
      gPrimaryRachHistory[gPrimaryRachHistoryIndex] = false;
   }
   else if ( SECONDARY_RACH == channel )
   {
      if ( gDelayedResetActivated )
      {
         gDelayedResetCount++;
         if ( SMGR_DELAYED_RESET_DURATION <= gDelayedResetCount )
         {
            Error_Handler((char*)&gDelayedResetReason);
         }
      }
      MC_SMGR_CalculateNetworkUsage();
      
      gSecondaryRachHistoryIndex++;
      if ( SMGR_MAX_HISTORY_LENGTH == gSecondaryRachHistoryIndex )
      {
         gSecondaryRachHistoryIndex = 0;
      }
      gSecondaryRachHistory[gSecondaryRachHistoryIndex] = false;
      
      /* check for a time-out on the route add responses from the parent nodes */
      if ( gParent[PRIMARY_PARENT].WaitingForRouteAddResponse )
      {
         if ((NETWORK_BUSY_THRESHOLD > gSecondaryRachTrafficCount))
         {
            gParent[PRIMARY_PARENT].ShortFramesSinceRouteAdd++;
            CO_PRINT_B_1(DBG_INFO_E,"PRIMARY_PARENT ShortFramesSinceRouteAdd=%d.\r\n",gParent[PRIMARY_PARENT].ShortFramesSinceRouteAdd);
         }
         if ( SMGR_ROUTE_ADD_RESPONSE_TIMEOUT <= gParent[PRIMARY_PARENT].ShortFramesSinceRouteAdd )
         {
            gParent[PRIMARY_PARENT].RouteAddRetries++;
            //CO_PRINT_B_1(DBG_INFO_E,"PRIMARY_PARENT RouteAddRetries=%d.\r\n",gParent[PRIMARY_PARENT].RouteAddRetries);
            if ( SMGR_ROUTE_ADD_MAX_ATTEMPTS > gParent[PRIMARY_PARENT].RouteAddRetries  || CFG_UsingPreformedMesh())
            {
               /*We didn't get a Route Add Response from the parent.  Try again*/
               CO_PRINT_B_1(DBG_INFO_E,"No Route Add Response received from Primary Parent ID=%d.  Retrying\r\n",gParent[PRIMARY_PARENT].NodeID );
               if ( MC_SMGR_ConnectToParent(gParent[PRIMARY_PARENT].NodeID, false) )
               {
                  gParent[PRIMARY_PARENT].ShortFramesSinceRouteAdd = 0;
                  gParent[PRIMARY_PARENT].WaitingForRouteAddResponse = false;
               }
               else if ( false == CFG_UsingPreformedMesh() )
               {
                  uint16_t node_id = gParent[PRIMARY_PARENT].NodeID;
                  /* Failed to generate a route add message.*/
                  /* Send a route drop to the parent in case it can still hear us */
                  MC_SMGR_SendRouteDrop(node_id, CO_RESET_REASON_FAILED_TO_SEND_ROUTE_ADD_E, false, false, 0);
                  /*Delete the node and reselect */
                  MC_SMGR_DeleteNode(node_id, "Failed to send PP ROUTE ADD");
               }
            }
            else 
            {
               uint16_t node_id = gParent[PRIMARY_PARENT].NodeID;
               /* We have reached the maximum number of attempts to connect to the parent.*/
               /* Send a route drop to the parent in case it can still hear us */
               MC_SMGR_SendRouteDrop(node_id, CO_RESET_REASON_MAX_CONNECTION_ATTEMPTS_E, false, false, 0);
               /*Delete the node and reselect */
               MC_SMGR_DeleteNode(node_id, "Max attempts to connect to Primary parent reached");
            }
         }
      }
      if ( gParent[SECONDARY_PARENT].WaitingForRouteAddResponse )
      {
         if ((NETWORK_BUSY_THRESHOLD > gSecondaryRachTrafficCount))
         {
            gParent[SECONDARY_PARENT].ShortFramesSinceRouteAdd++;
            CO_PRINT_B_1(DBG_INFO_E,"SECONDARY_PARENT ShortFramesSinceRouteAdd=%d.\r\n",gParent[SECONDARY_PARENT].ShortFramesSinceRouteAdd);
         }
         if ( SMGR_ROUTE_ADD_RESPONSE_TIMEOUT <= gParent[SECONDARY_PARENT].ShortFramesSinceRouteAdd )
         { 
            gParent[SECONDARY_PARENT].RouteAddRetries++;
            //CO_PRINT_B_1(DBG_INFO_E,"SECONDARY_PARENT RouteAddRetries=%d.\r\n",gParent[SECONDARY_PARENT].RouteAddRetries);
            if ( SMGR_ROUTE_ADD_MAX_ATTEMPTS > gParent[SECONDARY_PARENT].RouteAddRetries || CFG_UsingPreformedMesh() )
            {
               /*We didn't get a Route Add Response from the parent.  Try again*/
               CO_PRINT_B_1(DBG_INFO_E,"No Route Add Response received from Secondary Parent ID=%d.  Retrying\r\n",gParent[SECONDARY_PARENT].NodeID); 
               if ( MC_SMGR_ConnectToParent( gParent[SECONDARY_PARENT].NodeID, false) )
               {
                  gParent[SECONDARY_PARENT].ShortFramesSinceRouteAdd = 0;
                  gParent[SECONDARY_PARENT].WaitingForRouteAddResponse = false;
               }
               else if ( false == CFG_UsingPreformedMesh() )
               {
                  uint16_t node_id = gParent[SECONDARY_PARENT].NodeID;
                  /* We have reached the maximum number of attempts to connect to the parent.*/
                  /* Send a route drop to the parent in case it can still hear us */
                  MC_SMGR_SendRouteDrop(node_id, CO_RESET_REASON_FAILED_TO_SEND_ROUTE_ADD_E, false, false, 0);
                  /*Delete the node and reselect */
                  MC_SMGR_DeleteNode(node_id, "Failed to send SP ROUTE ADD");
               }
            }
            else 
            {
               uint16_t node_id = gParent[SECONDARY_PARENT].NodeID;
               /* We have reached the maximum number of attempts to connect to the parent.*/
               /* Send a route drop to the parent in case it can still hear us */
               MC_SMGR_SendRouteDrop(node_id, CO_RESET_REASON_MAX_CONNECTION_ATTEMPTS_E, false, false, 0);
               /*Delete the node and reselect */
               MC_SMGR_DeleteNode(node_id, "Max attempts to connect to Secondary parent reached");
            }
         }
      }
   }
   
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_CalculateNetworkUsage
*  Description:      Counts the used rach slots over the last few slots
*                    (SMGR_MAX_HISTORY_LENGTH) and stores the values for
*                    PRACH and SRACH in global variables.
*
*  @param   None..
*  @return  None.
*            
*****************************************************************************/
void MC_SMGR_CalculateNetworkUsage(void)
{
   gPrimaryRachTrafficCount = 0;
   gSecondaryRachTrafficCount = 0;
   for ( uint32_t index = 0; index < SMGR_MAX_HISTORY_LENGTH; index++ )
   {
      if ( gPrimaryRachHistory[index] )
      {
         gPrimaryRachTrafficCount++;
      }
      
      if ( gSecondaryRachHistory[index] )
      {
         gSecondaryRachTrafficCount++;
      }
   }
   //CO_PRINT_B_2(DBG_INFO_E,"Nwk Ld - PR=%d%%, SR=%d%%\r\n", (gPrimaryRachTrafficCount*10), (gSecondaryRachTrafficCount*10));
}

#ifdef SHOW_NETWORK_USAGE
/*****************************************************************************/
/**
*  Function:         MC_SMGR_ShowNetworkUsage
*  Description:      Print network usage array to debug log.
*
*  @param   None..
*  @return  None.
*            
*****************************************************************************/
void MC_SMGR_ShowNetworkUsage(void)
{
   CO_PRINT_B_0(DBG_INFO_E,"Network usage: ");
   for ( uint32_t index = 0; index < SMGR_MAX_HISTORY_LENGTH; index++)
   {
      CO_PRINT_B_1(DBG_NOPREFIX_E,"%d ", (uint32_t)gSecondaryRachHistory[index]);
   }
   CO_PRINT_B_0(DBG_NOPREFIX_E,"\r\n");
}
#endif

/*****************************************************************************/
/**
*  Function:         MC_SMGR_DoLongframeStateValidation
*  Description:      Called every longframe for validation checks.
*
*  @param   None..
*  @return  None.
*            
*****************************************************************************/
void MC_SMGR_DoLongframeStateValidation(void)
{
   // Not done in the NCU
   if ( false == Is_Master )
   {
      //Only perform free running checks if logged on.
      if ( MC_SMGR_IsLoggedOn() )
      {
         //Check for RBU free-running (in TDM with no parents or tracking nodes OR no parents but in ACTIVE state)
         if ( gFreqLockAchieved )
         {
            if ( gFreeRunningDetected )
            {
               // The RBU is free-running and needs to be reset
//               Error_Handler("Detected RBU free-running - Resetting\n\r");
               for (uint16_t count = 0; count < 3; count++)
               {
                  MC_SMGR_SendRouteDrop(ADDRESS_GLOBAL,CO_RESET_REASON_FREE_RUNNING_E,false,true,0);
               }
               // Inform the application that we lost our connection to the mesh
               ApplicationMessage_t appMessage;
               appMessage.MessageType = APP_MSG_TYPE_ROUTE_DROP_E;
               CO_StatusIndicationData_t statusEvent;
               statusEvent.Event = CO_MESH_EVENT_INITIAL_TRK_NODE_LOST_E;
               statusEvent.EventNodeId = 0;
               statusEvent.EventData = 0;
               statusEvent.DelaySending = false;
               memcpy(appMessage.MessageBuffer, &statusEvent, sizeof(CO_StatusIndicationData_t));
               SM_SendEventToApplication( CO_MESSAGE_MESH_STATUS_EVENT_E, &appMessage );
            }
            else if ( UNASSIGNED_RANK == MC_SMGR_GetPublishedRank() )
            {
               //We are not publishing a rank.  Check that this is valid, and correct if it isn't.
               //It could be because the primary parent isn't publishing a rank
               if ( UNASSIGNED_RANK != gParent[PRIMARY_PARENT].PublishedRank )
               {
                  //The primary parent is publishing a rank.  If the parent is active, we should publish too.
                  if ( gParent[PRIMARY_PARENT].IsActive )
                  {
                     uint16_t my_rank = gParent[PRIMARY_PARENT].PublishedRank + 1;
                     if ( my_rank <= MC_GetMaxRank() )
                     {
                        MC_SMGR_SetRank(my_rank);
                     }
                  }
               }
            }
         }
      }
      
      //If we only have one parent, increment a count and instigate a search for more nodes
      //every SECONDARY_PARENT_RECHECK_PERIOD long frames.  Ignore if primary parent is the NCU.
      if ( ADDRESS_NCU != gParent[PRIMARY_PARENT].NodeID )
      {
         if ( 1 == MC_SMGR_GetNumberOfParents() )
         {
            //only one parent.  increment the period counter
            gSecondaryParentRecheckCount++;
            uint32_t secondary_parent_recheck_period = MC_SMGR_GetSecondaryParentSearchPeriod();
            if ( secondary_parent_recheck_period < gSecondaryParentRecheckCount )
            {
               //The re-check period has expired.  Do a search for more nodes, if one isn't under way.
               if ( false == MC_MFH_ScanForDevicesInProgress() )
               {
                  MC_MFH_ScanForDevices();
               }
               gSecondaryParentRecheckCount = 0;
            }
         }
      }
      
      //Check that LED state.  The sync patterns have been known to stay in force after they have been stopped.
      // Probably failed to get the LED mutex when attempting to remove.
      /* Request the pattern */
      CO_State_t current_device_state = MC_STATE_GetDeviceState();
      DM_LedPatterns_t led_req_pattern = DM_LedGetCurrentPattern();
      switch ( led_req_pattern )
      {
         case LED_MESH_STATE_READY_FOR_ACTIVE_E:
         {
            //Should always be in CONFIG_FORM with parents selected
            if ( (STATE_CONFIG_FORM_E != current_device_state) || (0 == MC_SMGR_GetNumberOfParents()) )
            {
               //The pattern should be removed
               DM_LedPatternRemove(LED_MESH_STATE_READY_FOR_ACTIVE_E);
            }
         }
            break;
         case LED_MESH_STATE_CONFIG_FORM_E:
         {
            //Should always be in CONFIG_FORM
            if ( STATE_CONFIG_FORM_E != current_device_state )
            {
               //The pattern should be removed
               DM_LedPatternRemove(LED_MESH_STATE_CONFIG_FORM_E);
            }
         }
            break;
         case LED_MESH_STATE_CONFIG_SYNC_E:
         {
            //Should always be in CONFIG_SYNC
            if ( STATE_CONFIG_SYNC_E != current_device_state )
            {
               //The pattern should be removed
               DM_LedPatternRemove(LED_MESH_STATE_CONFIG_SYNC_E);
            }
         }
            break;
         case LED_MESH_CONNECTED_E:
         {
            //Should always be in ACTIVE with messages waiting for DULCH
            if ( (STATE_ACTIVE_E != current_device_state) || (0 == MC_SACH_MessageCount()) )
            {
               //The pattern should be removed
               DM_LedPatternRemove(LED_MESH_CONNECTED_E);
               //Inform the application that the joining phase is complete
               ApplicationMessage_t joinMessage;
               AppMACEventData_t join_event_data;
               join_event_data.EventType = CO_JOINING_PHASE_COMPLETE_E;
               memcpy(joinMessage.MessageBuffer, &join_event_data, sizeof(AppMACEventData_t));
               SM_SendEventToApplication( CO_MESSAGE_MAC_EVENT_E, &joinMessage );
            }
         }
            break;
         case LED_MESH_IDLE_E:   //Intentional drop-through
         case LED_MESH_SLEEP1_E:
         case LED_MESH_SLEEP2_E:
         {
            //Should always be in IDLE
            if ( STATE_IDLE_E != current_device_state )
            {
               //The pattern should be removed
               DM_LedPatternRemove(led_req_pattern);
            }
         }
            break;
         default:
            //No action
         break;
      }
   }
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_ProcessPingResponse
*  Description:      Process a ping response.
*                    Note. We received an ACK in response to a Ping.
*
*  @param   source_node   The source node for the ping response.
*  @param   ack_received  True if the node responded to the ping
*  @return  None.
*            
*****************************************************************************/
void MC_SMGR_ProcessPingResponse(const uint16_t source_node, const bool ack_received)
{
//   bool foundFlag = false;
//   uint32_t index = 0;
   uint16_t dch_slots_per_long_frame = MC_DchSlotsPerLongFrame();
   uint16_t max_devices = MC_MaxNumberOfNodes();

   /* check inputs */
   if (max_devices <= source_node )
   {
      return;
   }
   
   CO_PRINT_B_2(DBG_INFO_E,"Ping response - node=%d, response=%d\r\n", source_node, ack_received);

   ErrorCode_t result = MC_PR_AckUpdate(source_node, ack_received);
   
   //If the device responded, inform the Synch module so that it doesn't drop the node
   if ( SUCCESS_E == result )
   {
      MC_MFH_NodeResponding(source_node);
   }
   //Check to see if the source node has failed to respond to ping recovery
   else if ( ERR_SIGNAL_LOST_E == result )
   {
      //Only delete parent nodes if not in preformed mesh
      if ( CFG_UsingPreformedMesh() )
      {
         if (false == MC_SMGR_IsParentNode(source_node) )
         {
            CO_PRINT_B_1(DBG_INFO_E, "Deleting node %d\r\n", source_node);
            MC_SMGR_DeleteNode(source_node, "Node stopped responding");
            
            /* Send a 'route drop' to the node, just in case it can still hear us, but
               set the 'wait for ack' to false so that we don't block our rach queue
               if the node can't respond*/
            MC_SMGR_SendRouteDrop(source_node, CO_RESET_REASON_NODE_STOPPED_RESPONDING_E, false, true, 0);
         }
         else 
         {
            if ( source_node == gParent[PRIMARY_PARENT].NodeID )
            {
               gPreformedPrimaryLinkBroken = true;
            }
            else if ( source_node == gParent[SECONDARY_PARENT].NodeID )
            {
               gPreformedSecondaryLinkBroken = true;
            }
         }
      }
      else 
      {
         CO_PRINT_B_1(DBG_INFO_E, "Deleting node %d\r\n", source_node);
         MC_SMGR_DeleteNode(source_node, "Node stopped responding");
         
         /* Send a 'route drop' to the node, just in case it can still hear us, but
            set the 'wait for ack' to false so that we don't block our rach queue
            if the node can't respond*/
         MC_SMGR_SendRouteDrop(source_node, CO_RESET_REASON_NODE_STOPPED_RESPONDING_E, false, true, 0);
      }
   }
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_RemakeLink
*  Description:      Called if a non-child node is sending this device messages,
*                    which it would only do if it has this node as a parent.  It
*                    indicates an assymetric link which we can repair here.
*
*  @param   node_id        The node for which to set up a child link.
*
*  @return  None.
*            
*****************************************************************************/
bool MC_SMGR_RemakeLink(const uint16_t node_id)
{
   bool child_accepted = false;
   
   /* Add the child to the session manager.  Assume this is the child's primary parent */
   if ( MC_SMGR_RouteAddRequest(node_id, true) )
   {
      child_accepted = true;
      CO_PRINT_B_1(DBG_INFO_E, "Added new child node : network ID %d\n\r", node_id);
      //Reset the ping module
      MC_PR_CancelPingForNode(node_id);
   }
   
   //if the child was not accepted, we should break the link with a route drop message.
   // Special case is when using preformed mesh when we don't want to break the links
   if ( (false == child_accepted) && (false == CFG_UsingPreformedMesh()) )
   {
      MC_SMGR_SendRouteDrop(node_id, CO_RESET_REASON_LINK_REMAKE_FAILED_E, false, true, 0);
   }
   
   return child_accepted;
}


/*****************************************************************************/
/**
*  Function:         MC_SMGR_PopulateRouteAddResponse
*  Description:      Build the response to a Route Add request from a new child.
*
*  @param   accepted             True if the child was accepted.
*  @param   zone                 The zone of the child, used to set the zone enablement flag
*  @param   zoneMapLowerUpper    Used to select which half of the zone map to populate. 0=lower, 1=upper.
*  @param   pResponseData        [OUT] The data structure to populate.
*
*  @return  None.
*            
*****************************************************************************/
bool MC_SMGR_PopulateRouteAddResponse( const bool accepted, const uint8_t zone, const uint8_t zoneMapLowerUpper, CO_RouteAddResponseData_t* pResponseData)
{
   bool result = false;
   if ( pResponseData )
   {
      pResponseData->Accepted = accepted;
      pResponseData->DayNight = CFG_GetDayNight();
      pResponseData->GlobalDelayEnabled = CFG_GetGlobalDelayEnabled();
      pResponseData->SoundLevel = CFG_GetSoundLevel();
      pResponseData->ZoneEnabled = CFG_RequestedZoneEnabled(zone);
      pResponseData->ZoneLowerUpper = zoneMapLowerUpper;
      pResponseData->FaultsEnabled = CFG_GetFaultReportsEnabled();
      pResponseData->GlobalDelayOverride = CFG_GetGlobalDelayOverride();
      uint32_t zoneWord;
      uint16_t zoneHalfWord;
      if ( CFG_GetSplitZoneDisabledMap((bool)zoneMapLowerUpper, &zoneWord, &zoneHalfWord) )
      {
         pResponseData->ZoneMapWord = zoneWord;
         pResponseData->ZoneMapHalfWord = zoneHalfWord;
         result = true;
      }
   }
   
   return result;
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_SetPowerStatus
*  Description:      Set by the application when the main battery fails/recovers
*
*  @param   status   Zero for main battery fail, any other value for battery OK.
*  @return  None.
*            
*****************************************************************************/
void MC_SMGR_SetPowerStatus(const uint32_t status)
{
   if ( 0 == status )
   {
      gDynamicPowerMode = true;
      CO_PRINT_A_0(DBG_INFO_E,"LOW POWER MODE ACTIVATED\r\n");
   }
   else 
   {
      gDynamicPowerMode = false;
      CO_PRINT_A_0(DBG_INFO_E,"LOW POWER MODE DEACTIVATED\r\n");
   }
   //Adjust the power output
   MC_SMGR_CalculateTxPower();
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_GetTxPower
*  Description:      Returns the power level that downlink messages should be 
*                    sent with.
*
*  @param   None.
*  @return  uint8_t  Tx Power Level
*            
*****************************************************************************/
uint8_t MC_SMGR_GetTxPower(void)
{
   return gPowerSavingTxPower;
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_CalculateTxPower
*  Description:      Analyses the RSSI readings of child nodes to determine
*                    the lowest Tx Power that can be used for downlink
*                    messages in power saving mode.
*
*  @param   None.
*  @return  uint8_t  Tx Power Level
*            
*****************************************************************************/
void MC_SMGR_CalculateTxPower(void)
{
      
   gPowerSavingTxPower = 0;
   if ( !gDynamicPowerMode )
   {
      gPowerSavingTxPower = MC_GetTxPowerLow();
   }
   else 
   {
      /* find the weakest child signal */
      int8_t lowestRSSI = 127;
      uint32_t max_children = CFG_GetMaxNumberOfChildren();
      for ( uint32_t index = 0; index < max_children; index++)
      {
         if ( CO_BAD_NODE_ID != gChildren[index] )
         {
            if ( neighbourArray[gChildren[index]].RSSI < lowestRSSI )
            {
               lowestRSSI = neighbourArray[index].RSSI;
            }
         }
      }
      /* convert the signal strength to a corresponding Tx power */
      uint32_t standard_power_setting = MC_GetTxPowerLow();
      int32_t rssi_sensitivity = CFG_GetRssiMarSensitivity(false);
      
      int16_t power_difference = lowestRSSI - rssi_sensitivity;
      //If the lowest rssi is on or below the joining threshold, keep the standard power setting
      if ( power_difference <= 0 )
      {
         gPowerSavingTxPower = standard_power_setting;
      }
      else if ( standard_power_setting < power_difference )
      {
         //The power difference is well above the minimum threshold.  Set the power to minimum.
         gPowerSavingTxPower = TX_MIN_OUTPUT_POWER;
      }
      else 
      {
         //The power difference is between TX_MIN_OUTPUT_POWER and standard_power_setting.
         //Reduce the power setting by power_difference.
         gPowerSavingTxPower = standard_power_setting - power_difference;
      }
      
//      CO_PRINT_B_2(DBG_INFO_E,"LowestRssi=%d, Power saving TxPower=%d\r\n", lowestRSSI, gPowerSavingTxPower);
   }
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_SetLoggedOn
*  Description:      Set the logged on flag
*
*  @param   None.
*  @return  Node.
*            
*****************************************************************************/
void MC_SMGR_SetLoggedOn(void)
{
   gIsLoggedOn = true;
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_IsLoggedOn
*  Description:      Returns true if the node is logged on
*
*  @param   None.
*  @return  True if the node is logged on.
*            
*****************************************************************************/
bool MC_SMGR_IsLoggedOn(void)
{
   return gIsLoggedOn;
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_DelayedReset
*  Description:      Schedule a delayed device reset, giving time for route drop
*                    messages to be transmitted
*
*  @param   pReset   Character string giving the reason for the reset
*  @return  None.
*            
*****************************************************************************/
void MC_SMGR_DelayedReset(const char* pResetReason)
{
   if ( pResetReason )
   {
      /* Capture debug information */
      if (strlen(pResetReason) < (sizeof(gDelayedResetReason) - 1u))
      {
         /* Copy the error message in the non initialised area */
         strcpy(gDelayedResetReason, pResetReason);
      }
      else 
      {
         memcpy(gDelayedResetReason, pResetReason, (sizeof(gDelayedResetReason) - 1u));
      }
   }
   else 
   {
      strcpy(gDelayedResetReason, "No reason given");
   }

   gDelayedResetActivated = true;
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_GetPublishedRank
*  Description:      Return the device rank.  If both parents have witheld their rank
*                    return 'unassigned' rank to warn new joiners that this node is
*                    in a pending status on the mesh.
*  @param   None.
*  @return  None.
*            
*****************************************************************************/
uint8_t MC_SMGR_GetPublishedRank(void)
{
   uint16_t published_rank = UNASSIGNED_RANK;
   
   if ( ADDRESS_NCU == gNetworkAddress )
   {
      published_rank = 0;
   }
   else if ( (UNASSIGNED_RANK != gParent[PRIMARY_PARENT].PublishedRank) ||
             (UNASSIGNED_RANK != gParent[SECONDARY_PARENT].PublishedRank) )
   {
      published_rank = MC_SMGR_GetRank();
   }
   
   return published_rank;
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_ProcessRouteAddFailure
*  Description:      A route add request was not acknowledged by the candidate
*                    parent.  Discard the candidate parent.
*
*  @param   node_id  the node ID of the candidate parent
*  @return  None.
*            
*****************************************************************************/
void MC_SMGR_ProcessRouteAddFailure(const uint16_t node_id)
{
   //If the node ID matches a parent, discard the parent
   if ( (node_id == gParent[PRIMARY_PARENT].NodeID) ||
        (node_id == gParent[SECONDARY_PARENT].NodeID) )
   {
      //This will deal with promotion of other nodes
      MC_SMGR_DeleteNode(node_id, "Route Add not Ack'd");
   }
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_GetSecondaryParentSearchPeriod
*  Description:      Calculated the number of long frames that a node should
*                    wait when periodically checking for a secondary parent.
*                    The synch node frame count is used to determine how long
*                    the node has been connected.
*
*  @param   none
*  @return  uint32_t The number of long frames between checks.
*            
*****************************************************************************/
uint32_t MC_SMGR_GetSecondaryParentSearchPeriod(void)
{
   uint32_t period = SECONDARY_PARENT_RECHECK_PERIOD_MAX;
   ShortListElement_t node;
   
   if ( SUCCESS_E == MC_MFH_FindNode(gParent[PRIMARY_PARENT].NodeID, &node) )
   {
      if ( SECONDARY_PARENT_RECHECK_PERIOD_SHORT_THRESHOLD > node.FrameCount )
      {
         period = SECONDARY_PARENT_RECHECK_PERIOD_SHORT;
      }
      else if ( SECONDARY_PARENT_RECHECK_PERIOD_MEDIUM_THRESHOLD > node.FrameCount )
      {
         period = SECONDARY_PARENT_RECHECK_PERIOD_MEDIUM;
      }
   }
   
   return period;
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_GetChildIDByIndex
*  Description:      Indexes through the current child array and returns the node ID
*                    of the child that is in the requested position.
*                    e.g. if requestedIndex is 3, the function will iterate through
*                    the child list until it finds the 3rd child and return its ID.
*
*  @param   requestedIndex    The requested child
*
*  @return  uint16_t          the node of the child in the requested position
*            
*****************************************************************************/
uint16_t MC_SMGR_GetChildIDByIndex(const uint16_t requestedIndex)
{
   uint16_t node_id = CO_BAD_NODE_ID;
   
   if ( requestedIndex <= gSMGRNumberOfChildren )
   {
      uint32_t max_children = CFG_GetMaxNumberOfChildren();
      uint32_t child_count = 0;
      for ( uint32_t index = 0; index < max_children; index ++ )
      {
         if (gChildren[index] != CO_BAD_NODE_ID)
         {
            child_count++;
            if ( child_count == requestedIndex )
            {
               node_id = neighbourArray[gChildren[index]].NodeID;
               //stop searching
               break;
            }
         }
      }
   }
   return node_id;
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_PurgeNodes
*  Description:      Delete the parent/child addressed by the parameter.
*                    Note that the address could be the global addres for all nodes.
*
*  @param   node_id    the id of the node to be removed
*
*  @return  void.
*            
*****************************************************************************/
void MC_SMGR_PurgeNodes(const uint16_t nodeID)
{
   bool purge_all = (ADDRESS_GLOBAL == nodeID);
   
   CO_PRINT_B_1(DBG_INFO_E, "MC_SMGR_PurgeNodes ID %d\r\n", nodeID);
   
   //Parents and tracking nodes
   if ( purge_all || (nodeID < MAX_DEVICES_PER_SYSTEM ) )
   {
      for ( uint32_t index = 0; index < NUMBER_OF_PARENTS_AND_TRACKING_NODES; index ++ )
      {
         if ( purge_all || (nodeID == gParent[index].NodeID) )
         {
            if ( nodeID < MAX_DEVICES_PER_SYSTEM )
            {
               //Update the mesh form and heal
               MC_MFH_DeleteNode(gParent[index].NodeID, false);
               //Update session manager
               MC_SMGR_DeleteNode(gParent[index].NodeID, "Reboot command received");
            }
         }
      }
   }

   //Child nodes
   if ( purge_all || (nodeID < MAX_DEVICES_PER_SYSTEM) )
   {
      uint32_t max_children = CFG_GetMaxNumberOfChildren();
      for ( uint32_t index = 0; index < max_children; index ++ )
      {
         if ( purge_all || (nodeID == gChildren[index]) )
         {
            if ( nodeID < MAX_DEVICES_PER_SYSTEM )
            {
               //Update the mesh form and heal
               MC_MFH_DeleteNode(gParent[index].NodeID, false);
               //Update session manager
               MC_SMGR_DeleteNode(gChildren[index], "Reboot command received");
            }
         }
      }
   }
}

/*****************************************************************************/
/**
*  Function:         MC_SMGR_NetworkBusy
*  Description:      Return whether the network is busy on a specified RACH channel.
*
*  @param   channel     The RACH channel to be checked. PRIMARY_RACH or SECONDARY_RACH.
*
*  @return  bool        TRUE if the specified channel is busy.
*            
*****************************************************************************/
bool MC_SMGR_NetworkBusy(const uint8_t channel)
{
   bool busy = false;
   
   if ( SECONDARY_RACH == channel )
   {
      if ((NETWORK_BUSY_THRESHOLD < gSecondaryRachTrafficCount))
      {
         busy = true;
      }
   }
   else if ( PRIMARY_RACH == channel )
   {
      if ((NETWORK_BUSY_THRESHOLD < gPrimaryRachTrafficCount))
      {
         busy = true;
      }
   }
   
   return busy;
}
#ifdef ENABLE_LINK_ADD_OR_REMOVE
/*****************************************************************************/
/**
*  Function:         MC_SMGR_ForceAddNode
*  Description:      Test function.  Enables command ATANL to add a neighbour
*                    node to this device.
*
*  @param   node_id    the node id to add
*  @param   is_child   true to add as a child.  False will add sec parent or tracking node.
*  @return  None.
*            
*****************************************************************************/
void MC_SMGR_ForceAddNode(const uint16_t node_id, const bool is_child)
{
   uint8_t node_rank;
   if ( MAX_DEVICES_PER_SYSTEM > node_id )
   {
      // exclude the local device
      if ( node_id != gNetworkAddress)
      {
         //Is the new node a child?  Note, the NCU cannot be a child node.
         if ( is_child && (ADDRESS_NCU != node_id))
         {
            //check that we don't already have the node
            if ( false == MC_SMGR_IsAChild(node_id) )
            {
               if ( MC_SMGR_RouteAddRequest(node_id, false) )
               {
                  CO_PRINT_B_1(DBG_INFO_E,"Added child node %d\r\n", node_id);
               }
               else 
               {
                  CO_PRINT_B_1(DBG_INFO_E,"Failed to force add of child node %d\r\n", node_id);
               }
            }
         }
         else 
         {
            node_rank = MC_SMGR_GetRank();
            node_rank -= 1;
            //The new node is to be a secondary parent or tracking node.
            //Put it into the first vacant position
            //The unit must have a primary parent already.
            if ( gParent[PRIMARY_PARENT].IsActive )
            {
               //Is the secondary parent slot vacant?
               if ( CO_BAD_NODE_ID == gParent[SECONDARY_PARENT].NodeID )
               {
                  gParent[SECONDARY_PARENT].NodeID = node_id;
                  neighbourArray[node_id].Rank = node_rank;
                  neighbourArray[node_id].LostHeartbeatCount = 0;
                  neighbourArray[node_id].SNR = 20;
                  gParent[SECONDARY_PARENT].IsActive = false;
                  MC_SMGR_ConnectToParent(node_id, false);

                  CO_PRINT_B_1(DBG_INFO_E, "Opening session for Secondary Parent - Network ID=%d\n\r", gParent[SECONDARY_PARENT].NodeID);
                  if ( MC_SMGR_ConnectToParent(gParent[SECONDARY_PARENT].NodeID, true) )
                  {
                     CO_PRINT_B_1(DBG_INFO_E, "Join Request sent to Secondary Parent - Network ID=%d\n\r", gParent[SECONDARY_PARENT].NodeID);
                     gParent[SECONDARY_PARENT].WaitingForRouteAddResponse = false;
                     gParent[SECONDARY_PARENT].ShortFramesSinceRouteAdd = 0;
                  }
               }
               else if ( CO_BAD_NODE_ID == gParent[PRIMARY_TRACKING_NODE].NodeID )
               {
                  gParent[PRIMARY_TRACKING_NODE].NodeID = node_id;
                  neighbourArray[node_id].Rank = node_rank;
                  neighbourArray[node_id].LostHeartbeatCount = 0;
                  neighbourArray[node_id].SNR = 20;
                  gParent[PRIMARY_TRACKING_NODE].IsActive = false;
                  MC_SMGR_StartHeartbeatMonitorForNode(node_id);
               }
               else if ( CO_BAD_NODE_ID == gParent[SECONDARY_TRACKING_NODE].NodeID )
               {
                  gParent[SECONDARY_TRACKING_NODE].NodeID = node_id;
                  neighbourArray[node_id].Rank = node_rank;
                  neighbourArray[node_id].LostHeartbeatCount = 0;
                  neighbourArray[node_id].SNR = 20;
                  gParent[SECONDARY_TRACKING_NODE].IsActive = false;
                  MC_SMGR_StartHeartbeatMonitorForNode(node_id);
               }
            }
            //add to Mesh forming and healing
            uint32_t slot_in_longframe = CO_CalculateLongFrameSlotForDCH(node_id);
            MC_MFH_Update(node_id, slot_in_longframe, 20, -50, NO_TIMESTAMP_RECORDED, node_rank, STATE_ACTIVE_E, 0, false);
            //inhibit reselection for future parent searches
            MC_MFH_InhibitNodeSelection(node_id);
         }
      }
   }
}
#endif
