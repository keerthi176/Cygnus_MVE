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
*  File         : MC_MeshFormAndHeal.c
*
*  Description  : Source code for Mesh forming and healing module.
*
*************************************************************************************/

/* System Include Files
*************************************************************************************/
#include <string.h>  // For memset()

/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "CO_ErrorCode.h"
#include "CO_Message.h"
#include "utilities.h"
#include "board.h"
#include "DM_SerialPort.h"
#include "MC_SessionManagement.h"
#include "SM_StateMachine.h"
#include "DM_NVM.h"
#include "MC_MacConfiguration.h"
#include "MC_TDM.h"
#include "MC_StateManagement.h"
#include "CFG_Device_cfg.h"
#include "MC_PingRecovery.h"
#include "MC_MeshFormAndHeal.h"
#include "MC_SyncAlgorithm.h"

/* Local constants
*************************************************************************************/
#define MFH_SNR_NOT_RECORDED (-100)
#define MFH_RSSI_NOT_RECORDED (-256)
#define MFH_TIMESTAMP_NOT_RECORDED 0xFFFFFFFFU
#define MC_MFH_NOT_FOUND 0xFFFF
#define MC_MFH_MAX_MISSED_HEARTBEATS 5
#define MAX_SYNC_NODE_REFINE_COUNT 8
#ifdef FAST_TRACK_FORMING
#define MFH_PARENT_RESELECT_HOLDOFF 2 //long frames to gather heartbeats before trying to reselect parents
#else
#define MFH_PARENT_RESELECT_HOLDOFF 3 //long frames to gather heartbeats before trying to reselect parents
#endif
#define DEVICE_SCAN_TIME 2            //(long frame count) This will give one full longframe after the current frame.
#define MFH_AVE_LONG_FRAME_THRESHOLD 50
#define INTEGRATE(x,y) (((x * 7) + y) /8)
/* Local variables
*************************************************************************************/
ShortListElement_t gNodes[MFH_MAX_SHORT_LIST_SIZE];
MC_MFH_State_t MfhState;
uint16_t gInitialTrackingNodeIndex = MC_MFH_NOT_FOUND;
static int32_t gSyncNodeRefineCount = 0;
static uint32_t gMhfLongFramesSinceParentSelect = 0;
static uint32_t gScanForDevicesCount = 0;

/* External variables
*************************************************************************************/
extern uint32_t gNetworkAddress;
extern bool gFreqLockAchieved;

/* Private Function Declarations
*************************************************************************************/
ErrorCode_t MC_MFH_AddNode(const uint16_t nodeID, const int16_t snr, const int16_t rssi, const uint32_t timeStamp, const uint8_t rank, const CO_State_t state, const uint8_t numberOfChildren, const bool valid);
uint32_t MC_MFH_GetNodeIndex(const uint16_t nodeID);
void MC_MFH_UpdateShortList(const uint16_t nodeID, const int16_t snr, const int16_t rssi, const uint32_t timeStamp, const uint8_t rank, const CO_State_t state, const uint8_t numberOfChildren, const bool valid);
void MC_MFH_UpdateNode(const uint16_t nodeID, const int16_t snr, const int16_t rssi, const uint32_t timeStamp, const uint8_t rank, const CO_State_t state, const uint8_t numberOfChildren, const bool valid);
void MC_MFH_UpdateState(void);
void MC_MFH_WaitingForSync(const uint16_t nodeID, const uint32_t slotInSuperframeIdx, const CO_State_t state);
void MC_MFH_RefineSync(void);
void MC_MFH_SelectParents(void);
uint16_t MC_MFH_GetStrongestNodeAtRank(const uint16_t rank, const bool twoParents, const bool includeAll);
uint32_t MC_MFH_AvailableNodesAtRank(const uint16_t rank);
bool MC_MFH_SelectSecondaryParent(void);
static uint32_t MC_MFH_GetNodeFrameOffset(const uint16_t nodeID);
static ErrorCode_t MC_MFH_FindAlternateSynchNode(void);
static void  MC_MFH_ApplySyncCorrectionToNodes(const int32_t sync_correction);
/* Public Functions
*************************************************************************************/


/*************************************************************************************/
/**
* MC_InitMeshFormAndHeal
* Routine to initialise (or reset) the mesh forming and healing module
*
* @param - const uint32_t address
*
* @return - void
*/
void MC_MFH_Init(const bool isNCU)
{
   
   for( uint32_t index = 0; index < MFH_MAX_SHORT_LIST_SIZE; index++ )
   {
      gNodes[index].NodeID = CO_BAD_NODE_ID;
      gNodes[index].SNR = MFH_SNR_NOT_RECORDED;
      gNodes[index].RSSI = MFH_RSSI_NOT_RECORDED;
      gNodes[index].LastTimestamp = MFH_TIMESTAMP_NOT_RECORDED;
      gNodes[index].LongFrameOffset = MFH_TIMESTAMP_NOT_RECORDED;
      gNodes[index].AveragePeriod = MC_CyclesPerLongFrame();
      gNodes[index].FrameCount = 0;
      gNodes[index].Rank = MFH_RANK_NOT_PUBLISHED;
      gNodes[index].LongFramesSinceLastUpdate = 0;
      gNodes[index].NumberOfChildren = 0;
      gNodes[index].Unused = true;
   }
   
   CO_PRINT_B_2(DBG_INFO_E, "Preformed mesh PP=%d, SP=%d\n\r", (int16_t)CFG_GetPreformedPrimaryParent(), (int16_t)CFG_GetPreformedSecondaryParent());
   
   
   if (isNCU)
   {
      gFreqLockAchieved = true;
      MfhState = MFH_STATE_MAINTAINING_SHORT_LIST_E;
   }
   else 
   {
      MfhState = MFH_STATE_WAITING_FOR_SYNC_E;
   }
   
   MC_SMGR_SetRank(MFH_RANK_NOT_PUBLISHED);
}


/*************************************************************************************/
/**
* MC_MFH_Update
* Update the module with new heartbeat information
*
* @param - nodeID             The node ID of the received heartbeat
* @param - slotInSuperframe   The super frame slot of the received heartbeat
* @param - snr                The SNR of the received heartbeat
* @param - rssi               The RSSI of the received heartbeat
* @param - timestamp          The timestamp of the received heartbeat
* @param - rank               The rank of the received heartbeat
* @param - state              The mesh state of the received heartbeat
* @param - valid              True if a heartbeat was received, false if it's a missing Hb report
*
* @return - void
*/
void MC_MFH_Update(const uint16_t nodeID, const uint32_t slotInSuperframe, const int16_t snr, const int16_t rssi, const uint32_t timeStamp, const uint8_t rank, const CO_State_t state, const uint8_t numberOfChildren, const bool valid)
{
//   CO_PRINT_B_3(DBG_INFO_E,"MC_MFH_Update+ node=%d, slot=%d, time=%d\r\n", nodeID, slotInSuperframe, timeStamp);
   if ( MAX_DEVICES_PER_SYSTEM > nodeID )
   {
      //Update the short list
      MC_MFH_UpdateShortList(nodeID, snr, rssi, timeStamp, rank, state, numberOfChildren, valid);
      
      switch ( MfhState )
      {
         case MFH_STATE_WAITING_FOR_SYNC_E:
//            CO_PRINT_B_0(DBG_INFO_E,"MFH State MFH_STATE_WAITING_FOR_SYNC_E\r\n");
            MC_MFH_WaitingForSync(nodeID, slotInSuperframe, state);
            break;
         case MFH_STATE_REFINING_SYNC_NODE_E:
//            CO_PRINT_B_0(DBG_INFO_E,"MFH State MFH_STATE_REFINING_SYNC_NODE_E\r\n");
            if ( nodeID == gNodes[gInitialTrackingNodeIndex].NodeID )
            {
               MC_MFH_RefineSync();
            }
            break;
         case MFH_STATE_MAINTAINING_SHORT_LIST_E:
//            CO_PRINT_B_0(DBG_INFO_E,"MFH State MFH_STATE_MAINTAINING_SHORT_LIST_E\r\n");
            //Nothing to do
            break;
         case MFH_STATE_SELECTING_PARENTS_E:
            //nothing to do
            break;
         case MFH_STATE_SCANNING_FOR_DEVICES_E:
            //nothing to do
            break;
         default:
            MfhState = MFH_STATE_MAINTAINING_SHORT_LIST_E;
            break;
      }
      
      if ( nodeID == MC_SYNC_GetSyncNode() )
      {
         //Do the long frame updates
         MC_MFH_UpdateState();
      }
   }
}
   

/* Private Functions
*************************************************************************************/


/*************************************************************************************/
/**
* MC_MFH_UpdateState
* Perform the state updates that must be done once per long frame.
*
* @param - nodeID       The node to find
*
* @return - error code
*/
void MC_MFH_UpdateState(void)
{
      switch ( MfhState )
      {
         case MFH_STATE_WAITING_FOR_SYNC_E:
            //nothing to do
            break;
         case MFH_STATE_REFINING_SYNC_NODE_E:
            //nothing to do
            break;
         case MFH_STATE_MAINTAINING_SHORT_LIST_E:
            //nothing to do
            break;
         case MFH_STATE_SELECTING_PARENTS_E:
//            CO_PRINT_B_0(DBG_INFO_E,"MFH State MFH_STATE_SELECTING_PARENTS_E\r\n");
            if ( MFH_PARENT_RESELECT_HOLDOFF <= gMhfLongFramesSinceParentSelect )
            {
               MC_MFH_SelectParents();
            }
            break;
         case MFH_STATE_SCANNING_FOR_DEVICES_E:
            CO_PRINT_B_0(DBG_INFO_E,"MFH State MFH_STATE_SCANNING_FOR_DEVICES_E\r\n");
            if ( 0 == gScanForDevicesCount )
            {
               if  ( 2 > MC_SMGR_GetNumberOfParents() )
               {
                  MC_MFH_SelectSecondaryParent();
               }
               //Go to the normal state
               MfhState = MFH_STATE_MAINTAINING_SHORT_LIST_E;
               //Tell the session manager to close all of the DCH slots that aren't being tracked.
               MC_SMGR_SetAllDCHSlotBehaviour(MC_TDM_DCH_SLEEP_E);
            }
            break;
         default:
            MfhState = MFH_STATE_MAINTAINING_SHORT_LIST_E;
            break;
      }
}

/*************************************************************************************/
/**
* MC_MFH_FindNode
* Find and return a copy of the short list element that matches the supplied node ID.
*
* @param - nodeID       The node to find
*
* @return - error code
*/
ErrorCode_t MC_MFH_FindNode(const uint16_t nodeID, ShortListElement_t* const pNode)
{
   ErrorCode_t result = ERR_INVALID_PARAMETER_E;
   //check the params
   if ( pNode && (MAX_DEVICES_PER_SYSTEM > nodeID) )
   {
      result = ERR_NOT_FOUND_E;
      for( uint32_t index = 0; index < MFH_MAX_SHORT_LIST_SIZE; index++ )
      {
         if ( nodeID == gNodes[index].NodeID )
         {
            pNode->NodeID = gNodes[index].NodeID;
            pNode->SNR = gNodes[index].SNR;
            pNode->RSSI = gNodes[index].RSSI;
            pNode->LastTimestamp = gNodes[index].LastTimestamp;
            pNode->AveragePeriod = gNodes[index].AveragePeriod;
            pNode->FrameCount = gNodes[index].FrameCount;
            pNode->Rank = gNodes[index].Rank;
            pNode->Unused = gNodes[index].Unused;
            pNode->LongFrameOffset = gNodes[index].LongFrameOffset;
            pNode->MissedHeartbeats = gNodes[index].MissedHeartbeats;
            pNode->LongFramesSinceLastUpdate = gNodes[index].LongFramesSinceLastUpdate;
            result = SUCCESS_E;
            break;//stop the search
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* MC_MFH_GetNodeIndex
* Check if the supplied node ID is in the short list and return its index.
*
* @param - nodeID       The node to check
*
* @return - the node index or MC_MFH_NOT_FOUND if not found
*/
uint32_t MC_MFH_GetNodeIndex(const uint16_t nodeID)
{
   uint32_t node_index = MC_MFH_NOT_FOUND;
   
   for( uint32_t index = 0; index < MFH_MAX_SHORT_LIST_SIZE; index++ )
   {
      if ( nodeID == gNodes[index].NodeID )
      {
         node_index = index;
         break;//stop the search
      }
   }

   return node_index;
}

/*************************************************************************************/
/**
* MC_MFH_AddNode
* Add a node to the short list
*
* @param - nodeID       The node ID of the received heartbeat
* @param - snr          The SNR of the received heartbeat
* @param - rssi         The RSSI of the received heartbeat
* @param - timestamp    The timestamp of the received heartbeat
* @param - rank         The rank of the received heartbeat
* @param - valid        True if a heartbeat was received, false if it's a missing Hb report
*
* @return - error code
*/
ErrorCode_t MC_MFH_AddNode(const uint16_t nodeID, const int16_t snr, const int16_t rssi, const uint32_t timeStamp, const uint8_t rank, const CO_State_t state, const uint8_t numberOfChildren, const bool valid)
{
   ErrorCode_t result = ERR_NO_RESOURCE_E;
   uint32_t node_index = MC_MFH_NOT_FOUND;
   
   //If we already have this node, update the existing record
   node_index =  MC_MFH_GetNodeIndex(nodeID);
   if ( MC_MFH_NOT_FOUND != node_index )
   {
      MC_MFH_UpdateNode(nodeID, snr, rssi, timeStamp, rank, state, numberOfChildren, valid);
      result = SUCCESS_E;
   }
   else
   {
      if ( valid )
      {
         //New node.  Find an empty slot and add it.
         for( uint32_t index = 0; index < MFH_MAX_SHORT_LIST_SIZE; index++ )
         {
            if ( CO_BAD_NODE_ID == gNodes[index].NodeID )
            {
               gNodes[index].NodeID = nodeID;
               gNodes[index].SNR = snr;
               gNodes[index].RSSI = rssi;
               gNodes[index].LastTimestamp = timeStamp;
               gNodes[index].AveragePeriod = MC_CyclesPerLongFrame();
               gNodes[index].FrameCount = 1;
               gNodes[index].Rank = rank;
               gNodes[index].Unused = true;
               gNodes[index].LongFrameOffset = MC_MFH_GetNodeFrameOffset(nodeID);
               gNodes[index].LongFramesSinceLastUpdate = 0;
               gNodes[index].NumberOfChildren = numberOfChildren;
               result = SUCCESS_E;
               CO_PRINT_B_2(DBG_INFO_E,"Added node %d to short list, index=%d\r\n", nodeID, index);
#ifdef DISABLE_NCU_SELECTION_WHEN_FORMING
               //Disable selection of the NCU for test purposes
               if ( ADDRESS_NCU == nodeID )
               {
                  gNodes[index].Unused = false;
               }
#endif
               break;//stop the search
            }
         }
      }
   }

   return result;
}

/*************************************************************************************/
/**
* MC_MFH_DeleteNode
* Find and remove a node record from the short list
*
* @param - const uint32_t address
* @param - updateSessionManagement  TRUE if the node is to be deleted from session management.
*
* @return - void
*/
ErrorCode_t MC_MFH_DeleteNode(const uint16_t nodeID, const bool updateSessionManagement)
{
   ErrorCode_t result = ERR_NOT_FOUND_E;

   for( uint32_t index = 0; index < MFH_MAX_SHORT_LIST_SIZE; index++ )
   {
      if ( nodeID == gNodes[index].NodeID )
      {
         uint16_t sync_node_ID = MC_SYNC_GetSyncNode();
         if ( sync_node_ID == nodeID )
         {
            MC_SYNC_SetSyncNode(CO_BAD_NODE_ID);
         }
         gNodes[index].NodeID = CO_BAD_NODE_ID;
         gNodes[index].Rank = MFH_RANK_NOT_PUBLISHED;
         gNodes[index].Unused = true;
         if ( updateSessionManagement )
         {
            MC_SMGR_DeleteNode(nodeID, "Sync module dropped node");
         }
//         CO_PRINT_B_2(DBG_INFO_E,"Removed node %d from short list, index=%d\r\n", nodeID, index);
         result = SUCCESS_E;
         break;
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* MC_MFH_FreeNode
* Find a node record on the short list and mark it 'unused'
*
* @param - nodeID    The node to free
*
* @return - void
*/
void MC_MFH_FreeNode(const uint16_t nodeID)
{
   for( uint32_t index = 0; index < MFH_MAX_SHORT_LIST_SIZE; index++ )
   {
      if ( nodeID == gNodes[index].NodeID )
      {
         gNodes[index].Unused = true;
         break;
      }
   }
}

/*************************************************************************************/
/**
* MC_MFH_UpdateShortList
* Update the short list with new heartbeat information
*
* @param - nodeID       The node ID of the received heartbeat
* @param - snr          The SNR of the received heartbeat
* @param - rssi         The RSSI of the received heartbeat
* @param - timestamp    The timestamp of the received heartbeat
* @param - rank         The rank of the received heartbeat
* @param - valid        True if a heartbeat was received, false if it's a missing Hb report
*
* @return - void
*/
void MC_MFH_UpdateShortList(const uint16_t nodeID, const int16_t snr, const int16_t rssi, const uint32_t timeStamp, const uint8_t rank, const CO_State_t state, const uint8_t numberOfChildren, const bool valid)
{
   uint32_t node_index =  MC_MFH_GetNodeIndex(nodeID);

   if ( MC_MFH_NOT_FOUND != node_index )
   {
       MC_MFH_UpdateNode(nodeID, snr, rssi, timeStamp, rank, state, numberOfChildren, valid);
   }
   else if ( valid )
   {
      MC_MFH_AddNode(nodeID, snr, rssi, timeStamp, rank, state, numberOfChildren, valid);
   }
}

/*************************************************************************************/
/**
* MC_MFH_UpdateNode
* Update the short list element that matches the new heartbeat information.
* if the short list is full prioritise low rank, then SNR+RSSI
*
* @param - nodeID       The node ID of the received heartbeat
* @param - snr          The SNR of the received heartbeat
* @param - rssi         The RSSI of the received heartbeat
* @param - timestamp    The timestamp of the received heartbeat
* @param - rank         The rank of the received heartbeat
* @param - state        The mesh state of the received heartbeat
* @param - valid        True if a heartbeat was received, false if it's a missing Hb report
*
* @return - void
*/
void MC_MFH_UpdateNode(const uint16_t nodeID, const int16_t snr, const int16_t rssi, const uint32_t timeStamp, const uint8_t rank, const CO_State_t state, const uint8_t numberOfChildren, const bool valid)
{
   uint32_t node_index =  MC_MFH_GetNodeIndex(nodeID);
   uint32_t sync_period;
   uint32_t time_stamp;
   uint16_t sync_node_id = MC_SYNC_GetSyncNode();
   int32_t rssi_sensitivity;
   
   if ( MC_MFH_NOT_FOUND != node_index )
   {
      //increment the frame count for received and missed heartbeats
      gNodes[node_index].FrameCount++;
//      CO_PRINT_B_2(DBG_INFO_E,"Updating node %d in short list, index=%d\r\n", nodeID, node_index);
      // a record for the node was found. Update it.
      int32_t mod_ave = gNodes[node_index].AveragePeriod % LPTIM_MAX;
      uint16_t expected_timestamp = (uint16_t)SUM_WRAP(gNodes[node_index].LastTimestamp, mod_ave, LPTIM_MAX);

//      if ( nodeID == sync_node_id )
//      {
//         CO_PRINT_B_3(DBG_INFO_E,"Sync node %d timestamp=%d, expected=%d\r\n",nodeID,timeStamp,expected_timestamp);
//      }
      
      if ( NO_TIMESTAMP_RECORDED != timeStamp )
      {
         if ( valid )
         {
            //if using preformed mesh and the primary parent was lost, the synch is switched to the secondary parent.
            //Check if this heartbeat is the return of the primary parent.  If so, rstore it as synch node.
            if ( CFG_UsingPreformedMesh() )
            {
               //is this the primary parent?
               if ( nodeID == CFG_GetPreformedPrimaryParent() )
               {
                  //is another node the synch node?
                  if ( nodeID != MC_SYNC_GetSyncNode() )
                  {
                     //Another node is the synch node.  Change it to the primary parent
                     MC_SYNC_SetSyncNode(nodeID);
                  }
               }
            }
            
            time_stamp = timeStamp;

            uint32_t delta = ABS_DIFFERENCE(time_stamp, expected_timestamp);
            
            //check for roll-over
            bool rollover = false;
            if ( LPTIM_HALF <= delta )
            {
               delta = LPTIM_MAX - delta;
               rollover = true;
            }
            
            //Find the period of the sync node
            if ( ((time_stamp < expected_timestamp) && !rollover) ||
                 ((time_stamp > expected_timestamp) && rollover) )
            {
               sync_period = gNodes[node_index].AveragePeriod - delta;
            }
            else 
            {
               sync_period = gNodes[node_index].AveragePeriod + delta;
            }

            //Don't update the average frame length if it seems wildly out
            delta = ABS_DIFFERENCE(gNodes[node_index].AveragePeriod, sync_period);
            if ( nodeID == sync_node_id )
            {
               if ( gNodes[node_index].NodeID != sync_node_id)
               {
                  CO_PRINT_B_2(DBG_ERROR_E,"MFH mismatch record for sync node.  sync node=%d, updated record for node %d\r\n",sync_node_id,gNodes[node_index].NodeID);
               }
               //CO_PRINT_B_5(DBG_INFO_E,"Sync node %d aveLF=%d, sync_period=%d, delta=%d, index=%d\r\n",nodeID,gNodes[node_index].AveragePeriod,sync_period,delta,node_index);
            }
            if ( MFH_AVE_LONG_FRAME_THRESHOLD > delta )
            {
               if ( 2 < gNodes[node_index].FrameCount )
               {
                  //Calculate the average long frame duration for the node
                  gNodes[node_index].AveragePeriod = INTEGRATE4(gNodes[node_index].AveragePeriod, sync_period);
               }
               else 
               {
                  //This is only the second heartbeat that we have seen from this node, which is our first
                  //measurement of the frame length, so don't average it yet.
                  gNodes[node_index].AveragePeriod = sync_period;
               }
            }
            
            //update the node SNR and RSSI
            SnrRssiAveragingPolicy_t policy = CFG_GetSnrRssiAveragingPolicy();
            switch ( policy )
            {
               case CFG_AVE_DISABLE_E:
                  //CO_PRINT_B_0(DBG_INFO_E,"SnrRssi Ave CFG_AVE_DISABLE_E\r\n");
                  gNodes[node_index].SNR = snr;
                  gNodes[node_index].RSSI = rssi;
                  break;
               case CFG_AVE_RESTART_E:
               {
                  if ( MFH_STALE_HEARTBEAT_THRESHOLD < gNodes[node_index].LongFramesSinceLastUpdate )
                  {
                     //CO_PRINT_B_0(DBG_INFO_E,"SnrRssi Ave CFG_AVE_RESTART_E restarted\r\n");
                     gNodes[node_index].SNR = snr;
                     gNodes[node_index].RSSI = rssi;
                  }
                  else
                  {
                     //CO_PRINT_B_0(DBG_INFO_E,"SnrRssi Ave CFG_AVE_RESTART_E default\r\n");
                     gNodes[node_index].SNR = INTEGRATE(gNodes[node_index].SNR, snr);
                     gNodes[node_index].RSSI = INTEGRATE(gNodes[node_index].RSSI, rssi);
                  }
               }
                  break;
               default:
                  //CO_PRINT_B_0(DBG_INFO_E,"SnrRssi Ave CFG_AVE_DEFAULT_E\r\n");
                  gNodes[node_index].SNR = INTEGRATE(gNodes[node_index].SNR, snr);
                  gNodes[node_index].RSSI = INTEGRATE(gNodes[node_index].RSSI, rssi);
                  break;
            }
            
            //Update the reported rank
            gNodes[node_index].Rank = rank;
         
         
            int32_t sync_correction = MC_SYNC_UpdateSync(&gNodes[node_index], timeStamp);
            gNodes[node_index].LastTimestamp = timeStamp;
            gNodes[node_index].MissedHeartbeats = 0;
            gNodes[node_index].NumberOfChildren = numberOfChildren;
            gNodes[node_index].LongFramesSinceLastUpdate = 0;
            
            //If the TDM sync was corrected.  Apply the same correction to all of the nodes that we are tracking.
            if ( 0 != sync_correction )
            {
               MC_MFH_ApplySyncCorrectionToNodes(sync_correction);
            }
         }
         else 
         {
            CO_PRINT_A_3(DBG_INFO_E,"mhb1 node=%d, lastTime=%d, newTime=%d\r\n", gNodes[node_index].NodeID, gNodes[node_index].LastTimestamp, expected_timestamp);
             MC_SYNC_UpdateSync(&gNodes[node_index], NO_TIMESTAMP_RECORDED);
             gNodes[node_index].LastTimestamp = expected_timestamp;
         }
         
         
         if ( MC_MAC_TEST_MODE_OFF_E == MC_GetTestMode() )
         {
            //record the new timestamp or calculate the expected timestamp if the heartbeat was missed
            if ( valid )
            {
               /* Tell the session manager that we received a heartbeat */
               MC_SMGR_ReceivedNodeHeartbeat(nodeID, snr, rssi, state, rank, 0);
            }
            else 
            {
               if ( 2 != MC_SMGR_GetNumberOfParents() )
               {
                  rssi_sensitivity = CFG_GetRssiMarSensitivity(false);
                  CO_PRINT_A_5(DBG_INFO_E,"mhb3 node=%d, snr=%d, snrMAR=%d, rssi=%d, rssiMAR=%d, valid=0\r\n", gNodes[node_index].NodeID, gNodes[node_index].SNR, CFG_GetSnrJoiningThreshold(), gNodes[node_index].RSSI, rssi_sensitivity);
                  MC_SMGR_MissedNodeHeartbeat(nodeID);
                  gNodes[node_index].MissedHeartbeats++;
                  if ( MC_MFH_MAX_MISSED_HEARTBEATS <= gNodes[node_index].MissedHeartbeats)
                  {
                     MC_MFH_HandleLostNode(nodeID);
                  }
               }
            }
         }
      }
      else 
      {
         //No heartbeat arrived.  Update it if it's in our shortlist
         //Set the time that it should have arrived by advancing its last
         //timestamp to the expected time
         //handle the missed heartbeat
         MC_SMGR_MissedNodeHeartbeat(nodeID);
         gNodes[node_index].MissedHeartbeats++;
         if ( MC_MFH_MAX_MISSED_HEARTBEATS == gNodes[node_index].MissedHeartbeats)
         {
            MC_MFH_HandleLostNode(nodeID);
         }
         else 
         {
            CO_PRINT_A_3(DBG_INFO_E,"mhb2 node=%d, lastTime=%d, newTime=%d\r\n", gNodes[node_index].NodeID, gNodes[node_index].LastTimestamp, expected_timestamp);
            //kick the sync algorithm to keep it up to date
            MC_SYNC_UpdateSync(&gNodes[node_index], NO_TIMESTAMP_RECORDED);
            gNodes[node_index].LastTimestamp = expected_timestamp;
         }
      }
   }
}

///*************************************************************************************/
/**
* MC_MFH_GetStrongestNodeAtRank
* Search the short list for the strongest 'unused' node at the specified rank
*
* @param - rank         The rank to chack for
* @param - twoParents   True if the device should use the two-parent joining threshold
* @param - includeAll   set to true to ignore whether a node is allready in use
*
* @return - uint16_t    The short list index of the strongest available tracking node
*/
uint16_t MC_MFH_GetStrongestNodeAtRank(const uint16_t rank, const bool twoParents, const bool includeAll)
{
   uint16_t strongest_node_index = MC_MFH_NOT_FOUND;
   int16_t strongest_rssi = MFH_RSSI_NOT_RECORDED;
   int32_t rssi_sensitivity = CFG_GetRssiMarSensitivity(twoParents);
   
   for( uint32_t index = 0; index < MFH_MAX_SHORT_LIST_SIZE; index++ )
   {
      if ( CO_BAD_NODE_ID != gNodes[index].NodeID )
      {
         //Sanity check on the node ID
         if ( (MAX_NUMBER_OF_SYSTEM_NODES > gNodes[index].NodeID) && (gNetworkAddress != gNodes[index].NodeID) )
         {
            //only consider nodes that aren't already being used, unless includeAll is true
            if ( gNodes[index].Unused || includeAll)
            {
               //Is the heartbeat up to date?  Note that LongFramesSinceLastUpdate can be incremented before this decision is made.
               if ( (0 == gNodes[index].MissedHeartbeats) && (2 > gNodes[index].LongFramesSinceLastUpdate) )
               {
                  //Check that the rank is acceptable
                  if ( gNodes[index].Rank == rank )
                  {
                     //Does it have any child slots left?  Doesn't matter if includeAll is true because we are only synching, not joining.
                     if ( MAX_CHILD_IDX > gNodes[index].NumberOfChildren  || includeAll)
                     {
                        //Does it meet MAR on RSSI
                        if ( rssi_sensitivity <= gNodes[index].RSSI )
                        {
                           //Does it have the strongest RSSI?
                           if ( gNodes[index].RSSI > strongest_rssi )
                           {
                              //Does it meet MAR on SNR
                              if ( gNodes[index].SNR >= CFG_GetSnrJoiningThreshold() )
                              {
                                 //This is a stronger candidate
                                 strongest_node_index = index;
                                 strongest_rssi = gNodes[index].RSSI;
//                                 if ( includeAll )
                                 {
                                    //This is a search for a new synch node.  Report which node is selected.
                                    CO_PRINT_B_2(DBG_INFO_E,"Shortlist index %d node %d SELECTED.\r\n", index, gNodes[index].NodeID);
                                 }
                              }
//                              else 
//                              {
//                                 CO_PRINT_B_2(DBG_INFO_E,"MC_MFH_GetStrongestNodeAtRank rejected node %d for SNR=%d\r\n", gNodes[index].NodeID, gNodes[index].SNR);
//                              }
                           }
//                           else 
//                           {
//                              CO_PRINT_B_1(DBG_INFO_E,"MC_MFH_GetStrongestNodeAtRank rejected node %d for not strongest RSSI\r\n", gNodes[index].NodeID);
//                           }
                        }
//                        else 
//                        {
//                           CO_PRINT_B_2(DBG_INFO_E,"MC_MFH_GetStrongestNodeAtRank rejected node %d for RSSI=%d\r\n", gNodes[index].NodeID, gNodes[index].RSSI);
//                        }
                     }
//                     else 
//                     {
//                        CO_PRINT_B_2(DBG_INFO_E,"MC_MFH_GetStrongestNodeAtRank rejected node %d for rank=%d\r\n", gNodes[index].NodeID, gNodes[index].Rank);
//                     }
                  }
//                  else 
//                  {
//                     CO_PRINT_B_2(DBG_INFO_E,"MC_MFH_GetStrongestNodeAtRank rejected node %d for rank=%d\r\n", gNodes[index].NodeID, gNodes[index].Rank);
//                  }
               }
//               else 
//               {
//                  CO_PRINT_B_3(DBG_INFO_E,"MC_MFH_GetStrongestNodeAtRank rejected node %d for mhb=%d, lf=%d\r\n", gNodes[index].NodeID, gNodes[index].MissedHeartbeats, gNodes[index].LongFramesSinceLastUpdate);
//               }
            }
//            else 
//            {
//               CO_PRINT_B_1(DBG_INFO_E,"MC_MFH_GetStrongestNodeAtRank rejected node %d -usedd\r\n", gNodes[index].NodeID);
//            }
         }
      }
   }
   return strongest_node_index;
}

/*************************************************************************************/
/**
* MC_MFH_WaitingForSync
* Check candidate heartbeats for suitable initial tracking node
*
* @param - nodeID                The node ID of the received heartbeat.
* @param - slotInSuperframeIdx   The node ID of the received heartbeat.
* @param - state                 The mesh state of the received heartbeat.
*
* @return - None.
*/
void MC_MFH_WaitingForSync(const uint16_t nodeID, const uint32_t slotInSuperframeIdx, const CO_State_t state)
{
   uint32_t node_index = MC_MFH_GetNodeIndex(nodeID);
   int16_t joining_threshold = CFG_GetRssiMarSensitivity(true);
   
   if ( MC_MFH_NOT_FOUND != node_index )
   {
      if ( gNodes[node_index].SNR >= CFG_GetSnrJoiningThreshold() )
      {
         if ( joining_threshold <= gNodes[node_index].RSSI )
         {
            //Only sync to nodes that have published their rank
            if ( MFH_RANK_NOT_PUBLISHED != gNodes[node_index].Rank )
            {
               //Found a candidate.  Set it as sync node
               bool sync_node_set = false;
//               if ( MC_UsingPreformedMesh() )
//               {
//                  if ( nodeID == gPreformPrimaryParent )
//                  {
//                     CO_PRINT_B_1(DBG_INFO_E,"Node %d id PFM prim\r\n", nodeID);
//                     if ( SUCCESS_E == MC_SYNC_SetSyncNode(nodeID) )
//                     {
//                        sync_node_set = true;
//                     }
//                     else 
//                     {
//                        CO_PRINT_B_1(DBG_INFO_E,"No synch, failed to set synch node %d\r\n", nodeID);
//                     }
//                  }
//                  else 
//                  {
//                     CO_PRINT_B_2(DBG_INFO_E,"No synch, node %d not PFM prim (%d)\r\n", nodeID, gPreformPrimaryParent);
//                  }
//               }
//               else 
               if ( SUCCESS_E == MC_SYNC_SetSyncNode(nodeID) )
               {
                  sync_node_set = true;
               }
               else 
               {
                  CO_PRINT_B_1(DBG_INFO_E,"No synch, failed to set synch node %d\r\n", nodeID);
               }
               
               if ( sync_node_set )
               {
                  gInitialTrackingNodeIndex = node_index;
                  MC_MAC_TestMode_t testMode = MC_GetTestMode();
                  if ( (MC_MAC_TEST_MODE_OFF_E == testMode) || (MC_MAC_TEST_MODE_NETWORK_MONITOR_E == testMode) )
                  {
                     MC_STATE_SetMeshState(state);
                     MC_STATE_SetDeviceState(STATE_CONFIG_SYNC_E);
                  }
                  
                  
                  MC_TDM_StartTDMSlaveMode(slotInSuperframeIdx, gNodes[node_index].LastTimestamp);
                  
                  MfhState = MFH_STATE_REFINING_SYNC_NODE_E;
               }
            }
            else 
            {
               CO_PRINT_B_2(DBG_INFO_E,"No synch, rank=%d node %d\r\n", gNodes[node_index].Rank, nodeID);
            }
         }
         else 
         {
            CO_PRINT_B_3(DBG_INFO_E,"No synch, rssi=%d (thresh=%d) node %d\r\n", gNodes[node_index].RSSI, joining_threshold, nodeID);
         }
      }
      else 
      {
         CO_PRINT_B_2(DBG_INFO_E,"No synch, snr=%d node %d\r\n", gNodes[node_index].SNR, nodeID);
      }
   }
}

/*************************************************************************************/
/**
* MC_MFH_RefineSync
* Check candidate heartbeats for stronger initial tracking node
*
* @param -  None.
*
* @return - None.
*/
void MC_MFH_RefineSync(void)
{
   int16_t best_rssi = MFH_RSSI_NOT_RECORDED;
   uint32_t best_index = gInitialTrackingNodeIndex;
   CO_State_t device_state;
   int16_t joining_threshold = CFG_GetRssiMarSensitivity(true);
   
   if ( CFG_UsingPreformedMesh() )
   {
      device_state = MC_STATE_GetDeviceState();
      if ( (STATE_CONFIG_FORM_E == device_state) ||
            (STATE_ACTIVE_E == device_state) ) 
      {
         MfhState = MFH_STATE_SELECTING_PARENTS_E;
         gMhfLongFramesSinceParentSelect = MFH_PARENT_RESELECT_HOLDOFF; //Force immediate parent select
      }
   }
   else
   {
      for( uint32_t index = 0; index < MFH_MAX_SHORT_LIST_SIZE; index++ )
      {
         //is the array slot occupied
         if ( CO_BAD_NODE_ID != gNodes[index].NodeID )
         {
            //has the node published a rank?
            if ( MFH_RANK_NOT_PUBLISHED !=  gNodes[index].Rank )
            {
               //is it a lower or equal rank to the sync node ?
               if ( gNodes[index].Rank <= gNodes[gInitialTrackingNodeIndex].Rank )
               {
                  //does it meet MAR?
                  if ( (gNodes[index].SNR >= CFG_GetSnrJoiningThreshold()) && (joining_threshold <= gNodes[index].RSSI) )
                  {
                     //If the node is the NCU, accept it above others.
                     if ( ADDRESS_NCU == gNodes[index].NodeID )
                     {
                        //this line will force acceptance below
                        best_index = index;
                        best_rssi = gNodes[index].RSSI;
                        gInitialTrackingNodeIndex = best_index;
                        break; //stop looking
                     }
                     //is it a stronger snr than the previous best?
                     else if ( gNodes[index].SNR > gNodes[best_index].SNR )
                     {
                        best_index = index;
                        best_rssi = gNodes[index].RSSI;
#ifdef FAST_TRACK_FORMING
                        gInitialTrackingNodeIndex = best_index;
#endif
                     }
                  }
               }
            }
         }
      }

      //Check to see if the best candidate should replace the initial tracking node
      if ( MFH_RSSI_NOT_RECORDED != best_rssi )
      {
         //if the result is the same as the last selection, no further refining is necessary
         if ( best_index == gInitialTrackingNodeIndex )
         {
            device_state = MC_STATE_GetDeviceState();
            if ( (STATE_CONFIG_FORM_E == device_state) ||
                  (STATE_ACTIVE_E == device_state) ) 
            {
               MC_MAC_TestMode_t test_mode = MC_GetTestMode();
               if ( MC_MAC_TEST_MODE_NETWORK_MONITOR_E == test_mode )
               {
                  MfhState = MFH_STATE_MAINTAINING_SHORT_LIST_E;
               }
               else 
               {
                  MfhState = MFH_STATE_SELECTING_PARENTS_E;
                  gMhfLongFramesSinceParentSelect = 0;
               }
               gSyncNodeRefineCount = 0;
            }
         }
         else 
         {
            //A stronger candidate has been found.  Set it as initial tracking node
            if ( SUCCESS_E == MC_SYNC_SetSyncNode(gNodes[best_index].NodeID) )
            {
               gInitialTrackingNodeIndex = best_index;
            }
         }
      }
   }
   
   //Make sure we don't get locked in a continuous search
   gSyncNodeRefineCount++;
   if ( MAX_SYNC_NODE_REFINE_COUNT < gSyncNodeRefineCount )
   {
      device_state = MC_STATE_GetDeviceState();
      if ( (STATE_CONFIG_FORM_E == device_state) ||
            (STATE_ACTIVE_E == device_state) ) 
      {
         MfhState = MFH_STATE_SELECTING_PARENTS_E;
         gMhfLongFramesSinceParentSelect = 0;
      }
      gSyncNodeRefineCount = 0;
   }
}

/*************************************************************************************/
/**
* MC_MFH_SelectParents
* Search short list for the best parents and send them to the session manager
*
* @param -  None.
*
* @return - None.
*/
void MC_MFH_SelectParents(void)
{
   uint32_t node_count;
   uint32_t pp_node_index;
   uint32_t sp_node_index;
   bool parents_found = false;
   uint16_t trk_node_index1;
   uint16_t trk_node_index2;
   int32_t rssi_sensitivity;
   uint16_t preformed_primary_parent = CFG_GetPreformedPrimaryParent();
   uint16_t preformed_Secondary_parent = CFG_GetPreformedSecondaryParent();
   
   if ( CFG_UsingPreformedMesh() )
   {
      pp_node_index = MC_MFH_GetNodeIndex(preformed_primary_parent);
      if ( MC_MFH_NOT_FOUND != pp_node_index )
      {
         //Check that the primary parent is publishing a rank (i.e. connected to the mesh)
         if ( UNASSIGNED_RANK != gNodes[pp_node_index].Rank )
         {
            //Find the secondary parent
            if ( (ADDRESS_NCU != preformed_Secondary_parent) && (CO_BAD_NODE_ID != preformed_Secondary_parent) )
            {
               sp_node_index = MC_MFH_GetNodeIndex(preformed_Secondary_parent);
               if ( MC_MFH_NOT_FOUND != sp_node_index )
               {
                  //send the parents to the session manager
                  if( MC_SMGR_SetParentNodes(gNodes[pp_node_index].NodeID, gNodes[sp_node_index].NodeID) )
                  {
                     //mark the parent nodes as 'used'
                     gNodes[pp_node_index].Unused = false;
                     gNodes[sp_node_index].Unused = false;
                     
                     CO_PRINT_B_2(DBG_INFO_E,"Selected Primary Parent %d, Secondary Parent %d\r\n", gNodes[pp_node_index].NodeID, gNodes[sp_node_index].NodeID);
                     
                     //Set the primary parent as the sync node
                     MC_SYNC_SetSyncNode(gNodes[pp_node_index].NodeID);
                     
                     //Reset the ping module
                     MC_PR_CancelPingForNode(gNodes[pp_node_index].NodeID);
                     MC_PR_CancelPingForNode(gNodes[sp_node_index].NodeID);
                     
                     parents_found = true;
                  }
               }
            }
            else 
            {
               //send the primary parent to the session manager
               if( MC_SMGR_SetParentNodes(gNodes[pp_node_index].NodeID, SECONDARY_PARENT_NOT_ASSIGNED) )
               {
                  //mark the parent nodes as 'used'
                  gNodes[pp_node_index].Unused = false;
                  
                  CO_PRINT_B_1(DBG_INFO_E,"Selected Primary Parent %d, No Secondary Parent\r\n", gNodes[pp_node_index].NodeID);
                  
                  //Reset the ping module
                  MC_PR_CancelPingForNode(gNodes[pp_node_index].NodeID);
                  
                  parents_found = true;
               }
            }
         }
      }
   }
   else 
   {
      //First check for the NCU
      rssi_sensitivity = CFG_GetRssiMarSensitivity(false);
      pp_node_index =  MC_MFH_GetNodeIndex(ADDRESS_NCU);
      if ( MC_MFH_NOT_FOUND != pp_node_index )
      {
         //Don't select the NCU is the last heartbeat isn't recent.
         if ( MFH_STALE_HEARTBEAT_THRESHOLD > gNodes[pp_node_index].LongFramesSinceLastUpdate )
         {
            //We can see the NCU.  If it meets MAR we select it
            if ( gNodes[pp_node_index].SNR >= CFG_GetSnrJoiningThreshold() )
            {
               if ( rssi_sensitivity <= gNodes[pp_node_index].RSSI )
               {
                  //check that the NCU has not been inhibited
                  if ( gNodes[pp_node_index].Unused )
                  {
                     //Check that it has child slots left
                     if ( MAX_CHILD_IDX > gNodes[pp_node_index].NumberOfChildren )
                     {
                        if( MC_SMGR_SetParentNodes(ADDRESS_NCU, SECONDARY_PARENT_NOT_ASSIGNED) )
                        {
                           CO_PRINT_B_0(DBG_INFO_E,"Selected NCU as primary parent\r\n");
                           //Mark the NCU as 'used'
                           gNodes[pp_node_index].Unused = false;
                           //Set the primary parent as the sync node
                           MC_SYNC_SetSyncNode(gNodes[pp_node_index].NodeID);
                           //Reset the ping module
                           MC_PR_CancelPingForNode(gNodes[pp_node_index].NodeID);
                           parents_found = true;
                        }
                     }
                  }
               }
            }
         }
      }
      
      if ( !parents_found )
      {
         //iterate through the ranks seeking two parents at the same rank
         for( uint16_t rank = 1; rank < MAX_RANK; rank++)
         {
            // search for primary parent
            node_count = MC_MFH_AvailableNodesAtRank(rank);
            if ( 1 < node_count )
            {
               pp_node_index = MC_MFH_GetStrongestNodeAtRank(rank, true, false);
               if ( MC_MFH_NOT_FOUND != pp_node_index )
               {
                  // mark the primary parent choice as 'used'
                  gNodes[pp_node_index].Unused = false;
                  CO_PRINT_B_1(DBG_INFO_E,"MC_MFH_SelectParents 1 - set node %d to used", gNodes[pp_node_index].NodeID);
                  //Find the secondary parent
                  sp_node_index = MC_MFH_GetStrongestNodeAtRank(rank, true, false);
                  if ( MC_MFH_NOT_FOUND != sp_node_index )
                  {
                     //mark the secondary parent node as 'used'
                     gNodes[sp_node_index].Unused = false;
                     CO_PRINT_B_1(DBG_INFO_E,"MC_MFH_SelectParents 2 - set node %d to used", gNodes[sp_node_index].NodeID);
                     //send the parents to the session manager
                     if( MC_SMGR_SetParentNodes(gNodes[pp_node_index].NodeID, gNodes[sp_node_index].NodeID) )
                     {
                        CO_PRINT_B_2(DBG_INFO_E,"Selected Primary Parent %d, Secondary Parent %d\r\n", gNodes[pp_node_index].NodeID, gNodes[sp_node_index].NodeID);
                        //Set the primary parent as the sync node
                        MC_SYNC_SetSyncNode(gNodes[pp_node_index].NodeID);
                        //Reset the ping module
                        MC_PR_CancelPingForNode(gNodes[pp_node_index].NodeID);
                        MC_PR_CancelPingForNode(gNodes[sp_node_index].NodeID);

                        //Find two tracking nodes
                        trk_node_index1 = MC_MFH_GetStrongestNodeAtRank(rank, true, false);
                        if ( MC_MFH_NOT_FOUND != trk_node_index1 )
                        {
                           //mark the node as 'used'
                           gNodes[trk_node_index1].Unused = false;
                           CO_PRINT_B_1(DBG_INFO_E,"MC_MFH_SelectParents 3 - set node %d to used", gNodes[trk_node_index1].NodeID);
                           
                           trk_node_index2 = MC_MFH_GetStrongestNodeAtRank(rank, true, false);
                           if ( MC_MFH_NOT_FOUND == trk_node_index2 )
                           {
                              //No 2nd tracking node was found.  Pass tracking node 1 to the session manager
                              MC_SMGR_SetTrackingNodes(gNodes[trk_node_index1].NodeID, SECONDARY_TRACKING_NODE_NOT_ASSIGNED);
                              CO_PRINT_B_1(DBG_INFO_E,"Selected Primary Trk %d, No Secondary Trk\r\n", gNodes[trk_node_index1].NodeID);
                           }
                           else
                           {
                              //Found a second tracking node. Mark the node as 'used'
                              gNodes[trk_node_index2].Unused = false;
                              CO_PRINT_B_1(DBG_INFO_E,"MC_MFH_SelectParents 4 - set node %d to used", gNodes[trk_node_index2].NodeID);
                              MC_SMGR_SetTrackingNodes(gNodes[trk_node_index1].NodeID, gNodes[trk_node_index2].NodeID);
                              CO_PRINT_B_2(DBG_INFO_E,"Selected Primary Trk %d, Secondary Trk %d\r\n", gNodes[trk_node_index1].NodeID, gNodes[trk_node_index2].NodeID);
                           }
                        }

                        parents_found = true;
                        break; //stop searching
                     }
                     else 
                     {
                        //failed to push parents to the session manager.  Release the primary and secondary choice
                        gNodes[pp_node_index].Unused = true;
                        gNodes[sp_node_index].Unused = true;
                     }
                  }
               }
            }
         }
         
         //If we didn't find two parents, search again for a single parent
         if ( !parents_found )
         {
            //iterate through the ranks seeking two parents at the same rank
            for( uint16_t rank = 1; rank < MAX_RANK; rank++)
            {
               // search for primary parent
               node_count = MC_MFH_AvailableNodesAtRank(rank);
               if ( 0 < node_count )
               {
                  pp_node_index = MC_MFH_GetStrongestNodeAtRank(rank, false, false);
                  if ( MC_MFH_NOT_FOUND != pp_node_index )
                  {
                     if( MC_SMGR_SetParentNodes(gNodes[pp_node_index].NodeID, SECONDARY_PARENT_NOT_ASSIGNED) )
                     {
                        //mark the primary parent node as 'used'
                        gNodes[pp_node_index].Unused = false;
                        CO_PRINT_B_1(DBG_INFO_E,"MC_MFH_SelectParents 5 - set node %d to used\r\n", gNodes[pp_node_index].NodeID);
                        //Set the primary parent as the sync node
                        MC_SYNC_SetSyncNode(gNodes[pp_node_index].NodeID);
                        //Reset the ping module
                        MC_PR_CancelPingForNode(gNodes[pp_node_index].NodeID);
                        
                        CO_PRINT_B_1(DBG_INFO_E,"Selected Primary Parent %d, No Secondary Parent\r\n", gNodes[pp_node_index].NodeID);
                        parents_found = true;
                        break; //stop searching
                     }
                  }
               }
            }
         }
      }
   }

   //If parents were found, move to normal state
   if ( parents_found )
   {
      MfhState = MFH_STATE_MAINTAINING_SHORT_LIST_E;
   }

}

/*************************************************************************************/
/**
* MC_MFH_AvailableNodesAtRank
* Count the number of 'unused' nodes in the short list with the specified rank
*
* @param -  rank        The rank to search for
*
* @return - A count of available nodes at the specified rank.
*/
uint32_t MC_MFH_AvailableNodesAtRank(const uint16_t rank)
{
   uint32_t count = 0;
   
   //start using the lowest sensitivity in the assumption that we'll find more than one candidate.
   int32_t rssi_sensitivity = CFG_GetRssiMarSensitivity(true);
   
   for( uint32_t index = 0; index < MFH_MAX_SHORT_LIST_SIZE; index++ )
   {
      //Is the slot occupied?
      if ( CO_BAD_NODE_ID != gNodes[index].NodeID )
      {
         //Is the node at the correct rank?
         if ( gNodes[index].Rank == rank )
         {
            //Is the node unused?
            if ( gNodes[index].Unused  )
            {
               //check that the node meets MAR
               if ( gNodes[index].SNR >= CFG_GetSnrJoiningThreshold() )
               {
                  if ( rssi_sensitivity <= gNodes[index].RSSI )
                  {
                     if ( MAX_CHILD_IDX > gNodes[index].NumberOfChildren )
                     {
                        CO_PRINT_B_2(DBG_INFO_E,"MC_MFH_AvailableNodesAtRank 1 - rank %d, node %d\r\n", rank, gNodes[index].NodeID);
                        count++;
                     }
//                     else 
//                     {
//                        CO_PRINT_B_2(DBG_INFO_E,"MC_MFH_AvailableNodesAtRank rejected node %d for children=%d\r\n", gNodes[index].NodeID, gNodes[index].NumberOfChildren);
//                     }
                  }
//                  else 
//                  {
//                     CO_PRINT_B_2(DBG_INFO_E,"MC_MFH_AvailableNodesAtRank rejected node %d for RSSI=%d\r\n", gNodes[index].NodeID, gNodes[index].RSSI);
//                  }
               }
//               else 
//               {
//                  CO_PRINT_B_2(DBG_INFO_E,"MC_MFH_AvailableNodesAtRank rejected node %d for SNR=%d\r\n", gNodes[index].NodeID, gNodes[index].SNR);
//               }
            }
//            else 
//            {
//               CO_PRINT_B_1(DBG_INFO_E,"MC_MFH_AvailableNodesAtRank rejected node %d - used\r\n", gNodes[index].NodeID);
//            }
         }
//         else 
//         {
//            CO_PRINT_B_2(DBG_INFO_E,"MC_MFH_AvailableNodesAtRank rejected node %d for rank=%d\r\n", gNodes[index].NodeID, gNodes[index].Rank);
//         }
      }
   }
   
   // if we didn't find any candidates, try with the higher threshold for a single parent
   if ( 0 == count )
   {
      rssi_sensitivity = CFG_GetRssiMarSensitivity(false);
      for( uint32_t index = 0; index < MFH_MAX_SHORT_LIST_SIZE; index++ )
      {
         //Is the node unused and at the correct rank?
         if ( gNodes[index].Unused && (gNodes[index].Rank == rank) )
         {
            //check that the node meets MAR
            if ( gNodes[index].SNR >= CFG_GetSnrJoiningThreshold() )
            {
               if ( rssi_sensitivity <= gNodes[index].RSSI )
               {
                  if ( MAX_CHILD_IDX > gNodes[index].NumberOfChildren )
                  {
                     CO_PRINT_B_2(DBG_INFO_E,"MC_MFH_AvailableNodesAtRank 2 - rank %d, node %d\r\n", rank, gNodes[index].NodeID);
                     count++;
                  }
               }
            }
         }
      }
   }
   return count;
}

/*************************************************************************************/
/**
* MC_MFH_SelectNewParent
* Select a new secondary parent and see tracking nodes
*
* @param -  None.
*
* @return - None.
*/
void MC_MFH_SelectNewParent(void)
{
   MC_MFH_SelectSecondaryParent();
}

/*************************************************************************************/
/**
* MC_MFH_DoParentSelection
* Move the state to MFH_STATE_SELECTING_PARENTS_E to select a new parents
*
* @param -  None.
*
* @return - true.
*/
bool MC_MFH_DoParentSelection(void)
{
   CO_PRINT_B_0(DBG_INFO_E,"Starting Parent Selection\r\n");
   MfhState = MFH_STATE_SELECTING_PARENTS_E;
   gMhfLongFramesSinceParentSelect = 0;
   //open all dch slots while candidate parents are sought
   MC_SMGR_SetAllDCHSlotBehaviour(MC_TDM_DCH_RX_E);
   return true;
}

/*************************************************************************************/
/**
* MC_MFH_InhibitNodeSelection
* Called by the session manager if a parent rejects a ROUTE ADD request to prevent
* reselection of the same parent
*
* @param -  None.
*
* @return - None.
*/
void MC_MFH_InhibitNodeSelection(const uint16_t nodeID)
{
   uint32_t node_index = MC_MFH_GetNodeIndex(nodeID);
   
   if ( MC_MFH_NOT_FOUND != node_index)
   {
      gNodes[node_index].Unused = false;
      CO_PRINT_B_1(DBG_INFO_E,"MC_MFH_InhibitNodeSelection - set node %d to used", gNodes[node_index].NodeID);
   }
}

/*************************************************************************************/
/**
* MC_MFH_SelectSecondaryParent
* Check the short list for a new secondary parent node and send it to
* the session manager.
*
* @param -  None.
*
* @return - None.
*/
bool MC_MFH_SelectSecondaryParent(void)
{
   bool parent_selected = false;
   uint16_t rank = MC_SMGR_GetRank() - 1;
   uint16_t node_count;
   uint16_t node_index;
   uint16_t trk_node_index1;
   uint16_t trk_node_index2;
   
   //We won't find a secondary parent at rank 0 (i.e. connected to NCU)
   if ( 0 < rank )
   {
      // search for candidate parent
      node_count = MC_MFH_AvailableNodesAtRank(rank);
      if (  node_count )
      {
         CO_PRINT_B_2(DBG_INFO_E,"Scan found %d nodes at rank %d\r\n", node_count, rank);
         node_index = MC_MFH_GetStrongestNodeAtRank(rank, true, false);
         if ( MC_MFH_NOT_FOUND != node_index )
         {
            CO_PRINT_B_1(DBG_INFO_E,"Found node %d\r\n", gNodes[node_index].NodeID);
            if ( 2 > MC_SMGR_GetNumberOfParents() )
            {
               CO_PRINT_B_0(DBG_INFO_E,"-1-\r\n");
               if( MC_SMGR_SetParentNodes(PRIMARY_PARENT_NOT_ASSIGNED, gNodes[node_index].NodeID ) )
               {
                  //mark the primary parent node as 'used'
                  gNodes[node_index].Unused = false;
                  CO_PRINT_B_1(DBG_INFO_E,"Selected Secondary Parent %d\r\n", gNodes[node_index].NodeID);
                  //return to normal state
                  MfhState = MFH_STATE_MAINTAINING_SHORT_LIST_E;
                  parent_selected = true;
               }
            }
            
            
            //Find tracking nodes
            if ( 0 == MC_SMGR_GetNumberOfTrackingNodes() )
            {
               CO_PRINT_B_0(DBG_INFO_E,"-2-\r\n");
               trk_node_index1 = MC_MFH_GetStrongestNodeAtRank(rank, true, false);
               if ( MC_MFH_NOT_FOUND != trk_node_index1 )
               {
                  CO_PRINT_B_0(DBG_INFO_E,"-3-\r\n");
                  CO_PRINT_B_1(DBG_INFO_E,"Found trk1 node %d\r\n", gNodes[trk_node_index1].NodeID);
                  //mark the node as 'used'
                  gNodes[trk_node_index1].Unused = false;
                  
                  trk_node_index2 = MC_MFH_GetStrongestNodeAtRank(rank, true, false);
                  if ( MC_MFH_NOT_FOUND == trk_node_index2 )
                  {
                     //No 2nd tracking node was found.  Pass tracking node 1 to the session manager
                     if ( false == MC_SMGR_SetTrackingNodes(gNodes[trk_node_index1].NodeID, SECONDARY_TRACKING_NODE_NOT_ASSIGNED) )
                     {
                        //Failed to assign tracking node.  Restore it to 'Unused'
                        gNodes[trk_node_index1].Unused = true;
                        CO_PRINT_B_1(DBG_INFO_E,"Selected Primary Trk %d, No Secondary Trk\r\n", gNodes[trk_node_index1].NodeID);
                     }
                     
                  }
                  else
                  {
                     CO_PRINT_B_1(DBG_INFO_E,"Found trk2 node %d\r\n", gNodes[trk_node_index2].NodeID);
                     if (MC_SMGR_SetTrackingNodes(gNodes[trk_node_index1].NodeID, gNodes[trk_node_index2].NodeID) )
                     {
                        //Assigned a second tracking node. Mark the node as 'used'
                        gNodes[trk_node_index2].Unused = false;
                        CO_PRINT_B_2(DBG_INFO_E,"Selected Primary Trk %d, Secondary Trk %d\r\n", gNodes[trk_node_index1].NodeID, gNodes[trk_node_index2].NodeID);
                     }
                     else 
                     {
                        //Failed to assign tracking nodes.  Restore them to 'Unused'
                        gNodes[trk_node_index1].Unused = true;
                        gNodes[trk_node_index2].Unused = true;
                     }
                  }
               }
            }
            else if ( 1 == MC_SMGR_GetNumberOfTrackingNodes() )
            {
               //Find a second tracking node
               trk_node_index2 = MC_MFH_GetStrongestNodeAtRank(rank, true, false);
               if ( MC_MFH_NOT_FOUND != trk_node_index2 )
               {
                  CO_PRINT_B_1(DBG_INFO_E,"Found trk2 node %d\r\n", gNodes[trk_node_index2].NodeID);
                  
                  if ( MC_SMGR_SetTrackingNodes(PRIMARY_TRACKING_NODE_NOT_ASSIGNED, gNodes[trk_node_index2].NodeID) )
                  {
                     //mark the node as 'used'
                     gNodes[trk_node_index2].Unused = false;
                  }
                  else 
                  {
                     //Failed to assign tracking node.  Restore it to 'Unused'
                     gNodes[trk_node_index2].Unused = true;
                  }
               }
            }
         }
      }
   }
   return parent_selected;
}

/*************************************************************************************/
/**
* MC_MFH_InitialTrackingNodeActive
* Returns true if the initial tracking node has been selected
*
* @param -  None.
*
* @return - None.
*/
bool MC_MFH_InitialTrackingNodeActive(void)
{
   bool active = false;
   ShortListElement_t sync_node;
   uint16_t sync_node_id = MC_SYNC_GetSyncNode();
   
   if ( CO_BAD_NODE_ID != sync_node_id )
   {
      //Get the node's info
      if( SUCCESS_E == MC_MFH_FindNode( sync_node_id, &sync_node ) )
      {
         //Make sure it's receiving heartbeats
         if ( NO_TIMESTAMP_RECORDED != sync_node.LastTimestamp )
         {
            //Make sure it is publishing a rank
            if ( MFH_RANK_NOT_PUBLISHED != sync_node.Rank )
            {
               active = true;
            }
         }
      }
   }
   
//   if ( MC_MFH_NOT_FOUND !=  gInitialTrackingNodeIndex )
//   {
//      active = true;
//   }
   
   return active;
}


/*************************************************************************************/
/**
* MC_MFH_SetLoggedOn
* Called by the state machine when the logon message gets acknowledged.
* the state is advanced to select parents.
*
* @param -  None.
*
* @return - None.
*/
void MC_MFH_SetLoggedOn(void)
{
   MfhState = MFH_STATE_SELECTING_PARENTS_E;
   gMhfLongFramesSinceParentSelect = 0;
}


/*************************************************************************************/
/**
* MC_MFH_GetSNR
* Return the SNR of the specified node
*
* @param -  nodeID   The node to query
*
* @return - int16_t The SNR value
*/
int16_t MC_MFH_GetSNR(const uint16_t nodeID)
{
   int16_t snr = MFH_SNR_NOT_RECORDED;

   uint32_t node_index = MC_MFH_GetNodeIndex(nodeID);
   
   if ( MC_MFH_NOT_FOUND != node_index )
   {
      if ( UNASSIGNED_RANK != gNodes[node_index].Rank )
      {
         snr = gNodes[node_index].SNR;
      }
   }
   
   return snr;
}

/*************************************************************************************/
/**
* MC_MFH_GetRSSI
* Return the RSSI of the specified node
*
* @param -  nodeID   The node to query
*
* @return - int16_t The RSSI value
*/
int16_t MC_MFH_GetRSSI(const uint16_t nodeID)
{
   int16_t rssi = MFH_RSSI_NOT_RECORDED;
   uint32_t node_index;
   
   if ( MAX_DEVICES_PER_SYSTEM > nodeID )
   {
      node_index = MC_MFH_GetNodeIndex(nodeID);
      
      if ( MC_MFH_NOT_FOUND != node_index )
      {
         if ( UNASSIGNED_RANK != gNodes[node_index].Rank )
         {
            rssi = gNodes[node_index].RSSI;
         }
      }
      CO_PRINT_B_2(DBG_INFO_E,"MC_MFH_GetRSSI node=%d, rssi=%d\r\n", nodeID, rssi);
   }
   
   return rssi;
}


/*************************************************************************************/
/**
* MC_MFH_GetNodeFrameOffset
* Return the offset of the heartbeat slot into a long frame.
* The returned value is in LPTIM ticks.
*
* @param - nodeID :  The node to calculate the offset for.
*
* @return - uint32_t The offset in timer ticks.
*/
uint32_t MC_MFH_GetNodeFrameOffset(const uint16_t nodeID)
{
   uint32_t long_frame_slot = CO_CalculateLongFrameSlotForDCH(nodeID);
   uint32_t frame_offset = long_frame_slot * SLOT_DURATION;
   return frame_offset;
}

/*************************************************************************************/
/**
* MC_MFH_AdvanceLongFrame
* Increase the long frame count under some states.
*
* @param - None.
*
* @return - None.
*/
void MC_MFH_AdvanceLongFrame(void)
{
   if ( (MFH_STATE_SELECTING_PARENTS_E == MfhState) )
   {
      gMhfLongFramesSinceParentSelect++;
   }
}

/*************************************************************************************/
/**
* MC_MFH_GetState
* Return the current state of the mesh forming and healing state machine..
*
* @param - None.
*
* @return - MC_MFH_State_t.  The current state for mesh forming and healing.
*/
MC_MFH_State_t MC_MFH_GetState(void)
{
   return MfhState;
}


/*************************************************************************************/
/**
* MC_MFH_ResetMissedHeartbeatCount
* Reset the missed heartbeat count for a given node.
* Called by the session manager if the node responds to a ping.
*
* @param - timeStamp : The time stamp for the received heartbeat.
*
* @return - None.
*/
void MC_MFH_ResetMissedHeartbeatCount(const uint16_t nodeID)
{
   uint32_t node_index = MC_MFH_GetNodeIndex(nodeID);
   
   if ( MC_MFH_NOT_FOUND != node_index )
   {
      gNodes[node_index].MissedHeartbeats = 0;
   }
}

/*************************************************************************************/
/**
* MC_MFH_HandleLostNode
* Handle special case of losing the primary parent heartbeat in preformed mesh.
* If there is no secondary parent to transfer the sync to, try to find an
* alternate synch node until the primary parent returns
*
* @param - nodeID : The ID of the lost node.
*
* @return - SUCCESS_E or error code.
*/
ErrorCode_t MC_MFH_HandleLostNode(const uint16_t nodeID)
{
   ErrorCode_t result = ERR_SIGNAL_LOST_E;
   ShortListElement_t secondary_parent;
   uint16_t preformed_secondary_parent = CFG_GetPreformedSecondaryParent();
   
   CO_PRINT_B_1(DBG_INFO_E,"MC_MFH_HandleLostNode %d\r\n", nodeID);
   
   //Cancel any pings
   MC_PR_CancelPingForNode(nodeID);
   
   if ( CFG_UsingPreformedMesh() )
   {
      //Using preformed mesh.  Try to stay connected.
      if ( nodeID == CFG_GetPreformedPrimaryParent() )
      {
         //The preformed primary parent heartbeat has stopped.  check for a secondary
         //parent and set it as the new sync node
         if ( CO_BAD_NODE_ID != preformed_secondary_parent )
         {
            //check that we have heard from the secondary parent
            if ( SUCCESS_E == MC_MFH_FindNode(preformed_secondary_parent, &secondary_parent) )
            {
               CO_PRINT_B_2(DBG_INFO_E,"pfm secondary (%d) mhb=%d\r\n", secondary_parent.NodeID, secondary_parent.MissedHeartbeats);
               //check that it has been updated in the last two heartbeats
               if ( MFH_STALE_HEARTBEAT_THRESHOLD > secondary_parent.MissedHeartbeats )
               {
                  //The secondary parent was seen recently.  Set it as synch node, if it isn't already.
                  if ( preformed_secondary_parent == MC_SYNC_GetSyncNode() )
                  {
                     result = SUCCESS_E;
                  }
                  else
                  {
                     result = MC_SYNC_SetSyncNode(preformed_secondary_parent);
                  }
               }
            }
         }
      }
      else 
      {
         //Not primary parent, so no need to rselect unless the dropped node is the synch node
         if ( nodeID != MC_SYNC_GetSyncNode() )
         {
            //Not current synch node so no need to find another sync node
            result = SUCCESS_E;
         }
      }
      
      if ( SUCCESS_E != result )
      {
         CO_PRINT_B_0(DBG_INFO_E,"MFH No PFM secondary\r\n");
         //Try to find another synch node until the primary parent reconnects
         result = MC_MFH_FindAlternateSynchNode();
         
         if ( SUCCESS_E != result )
         {
            CO_PRINT_B_0(DBG_INFO_E,"MFH No synch nodes found\r\n");
            //Unable to handle loss. We cannot maintain synch with the mesh.
            Error_Handler("Lost mesh synch");
         }
      }
   }
   else
   {
      //not using preformed mesh.  Delete the node
      result = MC_MFH_DeleteNode(nodeID, true);
   }
   
   return result;
}

/*************************************************************************************/
/**
* MC_MFH_FindAlternateSynchNode
* Handle special case of losing the primary parent heartbeat in preformed mesh.
* If there is no secondary parent to transfer the sync to, or 
*
* @param - nodeID : The ID of the lost node.
*
* @return - SUCCESS_E or error code.
*/
ErrorCode_t MC_MFH_FindAlternateSynchNode(void)
{
   ErrorCode_t result = ERR_NOT_FOUND_E;
   ShortListElement_t current_synch_node;
   ShortListElement_t next_synch_node;
   uint32_t max_acceptable_rank = 0;
   uint16_t next_synch_node_idx;
   
#ifdef USE_NEW_SYNC_NODE_RESELECT
   ShortListElement_t parent_node;
   uint16_t primary_parent_id;
   uint16_t secondary_parent_id;
   bool perform_node_search = false;
   CO_PRINT_B_0(DBG_INFO_E,"MC_MFH_FindAlternateSynchNode\r\n");
   
   //Get the current synch node or the previous one if it has been deleted already
   uint16_t current_synch_node_id = MC_SYNC_GetSyncNode();
   
   if ( CO_BAD_NODE_ID == current_synch_node_id )
   {
      //Sync node already deleted.  Get the previous one.
      current_synch_node_id = MC_SYNC_GetPreviousSyncNode(&current_synch_node);
      CO_PRINT_B_1(DBG_INFO_E,"Recovered previous sync node %d\r\n", current_synch_node_id);
   }
   else 
   {
      //Get the record of the current sync node
      if ( SUCCESS_E == MC_MFH_FindNode(current_synch_node_id, &current_synch_node) )
      {
         CO_PRINT_B_1(DBG_INFO_E,"Found current sync node %d\r\n", current_synch_node_id);
      }
   }
   
   //If we have selected a primary parent we should try to sync to it.
   primary_parent_id = MC_SMGR_GetPrimaryParentID();
   
   if ( CO_BAD_NODE_ID == primary_parent_id )
   {
      //no primary parent has been selected.  We are free to search all nodes.
      perform_node_search = true;
      max_acceptable_rank = MC_GetMaxRank();
   }
   else 
   {
      //We have selected a primary parent.
      //Is it the lost sync node?
      if ( current_synch_node_id == primary_parent_id )
      {
         //we can't select the primary parent.  Did we choose a secondary?
         secondary_parent_id = MC_SMGR_GetSecondaryParentID();
         if ( SUCCESS_E == MC_MFH_FindNode(secondary_parent_id, &parent_node) )
         {
            CO_PRINT_B_1(DBG_INFO_E,"Secondary parent identified %d\r\n", secondary_parent_id);
            //Try to set the secondary parent as the sync node
            //Is it eligible?
            if ( (UNASSIGNED_RANK != parent_node.Rank) && (0 == parent_node.MissedHeartbeats) )
            {
               if ( SUCCESS_E == MC_SYNC_SetSyncNode(secondary_parent_id) )
               {
                  CO_PRINT_B_1(DBG_INFO_E,"Secondary parent selected as sync node %d\r\n", secondary_parent_id);
                  perform_node_search = false;
                  result = SUCCESS_E;
               }
               else 
               {
                  //failed to select secondary parent as sync node
                  //do a search for nodes up to the same rank, or any rank if it isn't published
                  perform_node_search = true;
                  if ( UNASSIGNED_RANK == parent_node.Rank )
                  {
                     max_acceptable_rank = MC_GetMaxRank();
                  }
                  else 
                  {
                     max_acceptable_rank = parent_node.Rank;
                  }
               }
            }
            else 
            {
               //failed to select secondary parent as sync node
               //do a search for nodes up to the same rank, or any rank if it isn't published
               perform_node_search = true;
               if ( UNASSIGNED_RANK == parent_node.Rank )
               {
                  max_acceptable_rank = MC_GetMaxRank();
               }
               else 
               {
                  max_acceptable_rank = parent_node.Rank;
               }
            }
         }
         else 
         {
            //No secondary parent.  Search for another sync node up to the rank of the previous one
            perform_node_search = true;
            if ( CO_BAD_NODE_ID != current_synch_node_id )
            {
               max_acceptable_rank = current_synch_node.Rank;
            }
            else 
            {
               //No current sync node.  Search all ranks rank.
               max_acceptable_rank = MC_GetMaxRank();
            }
         }
      }
      else 
      {
         //Primary parent wasn't our synch node.  try to select it
         //Is it eligible?
         if ( SUCCESS_E == MC_MFH_FindNode(primary_parent_id, &parent_node) )
         {
            CO_PRINT_B_1(DBG_INFO_E,"Primary parent identified %d\r\n", primary_parent_id);
            //Try to set the primary parent as the sync node
            //Is it eligible?
            if ( (UNASSIGNED_RANK != parent_node.Rank) && (0 == parent_node.MissedHeartbeats) )
            {
               if ( SUCCESS_E == MC_SYNC_SetSyncNode(primary_parent_id) )
               {
                  CO_PRINT_B_1(DBG_INFO_E,"Primary parent selected as sync node %d\r\n", primary_parent_id);
                  perform_node_search = false;
                  result = SUCCESS_E;
               }
               else 
               {
                  //failed to select primary parent as sync node
                  //do a search for nodes up to the same rank, or any rank if it isn't published
                  perform_node_search = true;
                  if ( UNASSIGNED_RANK == parent_node.Rank )
                  {
                     max_acceptable_rank = MC_GetMaxRank();
                  }
                  else 
                  {
                     max_acceptable_rank = parent_node.Rank;
                  }
               }
            }
            else 
            {
               //failed to select primary parent as sync node
               //do a search for nodes up to the same rank, or any rank if it isn't published
               perform_node_search = true;
               if ( UNASSIGNED_RANK == parent_node.Rank )
               {
                  max_acceptable_rank = MC_GetMaxRank();
               }
               else 
               {
                  max_acceptable_rank = parent_node.Rank;
               }
            }
         }
         else 
         {
            //No primary parent.  Search for another sync node up to the rank of the previous one
            perform_node_search = true;
            if ( CO_BAD_NODE_ID != current_synch_node_id )
            {
               max_acceptable_rank = current_synch_node.Rank;
            }
            else 
            {
               //No current sync node.  Search all ranks rank.
               max_acceptable_rank = MC_GetMaxRank();
            }
         }
      }
   }
   
   //Do a search if required
   if ( perform_node_search )
   {
      for ( uint32_t rank = 0; rank < max_acceptable_rank; rank++ )
      {
         next_synch_node_idx = MC_MFH_GetStrongestNodeAtRank(rank, true, true);
         if ( (MC_MFH_NOT_FOUND != next_synch_node_idx) && (gNodes[next_synch_node_idx].NodeID != current_synch_node_id) )
         {
            if ( SUCCESS_E == MC_MFH_FindNode(next_synch_node_idx, &next_synch_node) )
            {
               //An alterntive node was found.  Set it as the new synch node.
               if ( SUCCESS_E == MC_SYNC_SetSyncNode(gNodes[next_synch_node_idx].NodeID) )
               {
                  CO_PRINT_B_2(DBG_INFO_E,"MFH found new synch node %d, rank %d\r\n", next_synch_node.NodeID, next_synch_node.Rank);
                  result =SUCCESS_E;
                  //stop searching
                  break;
               }
            }
         }
      }
   }
   
#else
   
   CO_PRINT_B_0(DBG_INFO_E,"MC_MFH_FindAlternateSynchNode\r\n");
   
   //Get the current synch node or the previous one if it has been deleted already
   uint16_t current_synch_node_id = MC_SYNC_GetSyncNode();
   
   //There is no alternative synch node to the NCU
   if ( ADDRESS_NCU != current_synch_node_id )
   {
      //Get the rank of the current synch node and set that as the max rank for consideration
      if ( SUCCESS_E == MC_MFH_FindNode(current_synch_node_id, &current_synch_node) )
      {
         CO_PRINT_B_1(DBG_INFO_E,"MFH found current synch node %d\r\n", current_synch_node.NodeID);
         //Make sure that the rank was published
         if ( MFH_RANK_NOT_PUBLISHED != current_synch_node.Rank )
         {
            CO_PRINT_B_1(DBG_INFO_E,"MFH current synch node rank %d\r\n", current_synch_node.Rank);
            max_acceptable_rank = current_synch_node.Rank;
            //Find the best candidate at the same rank
            next_synch_node_idx = MC_MFH_GetStrongestNodeAtRank(current_synch_node.Rank, true, true);
            if ( MC_MFH_NOT_FOUND == next_synch_node_idx )
            {
               CO_PRINT_B_0(DBG_INFO_E,"MFH no alternate synch node at rank.  Searching..\r\n");
               //no replacement synch node was found at the same rank.  Search the lower ranks.
               for ( uint32_t rank = 0; rank < max_acceptable_rank; rank++ )
               {
                  next_synch_node_idx = MC_MFH_GetStrongestNodeAtRank(rank, true, true);
                  if ( (MC_MFH_NOT_FOUND != next_synch_node_idx) && (gNodes[next_synch_node_idx].NodeID != MC_SYNC_GetSyncNode()) )
                  {
                     if ( SUCCESS_E == MC_MFH_FindNode(next_synch_node_idx, &next_synch_node) )
                     {
                        CO_PRINT_B_2(DBG_INFO_E,"MFH found new synch node %d, rank %d\r\n", next_synch_node.NodeID, next_synch_node.Rank);
                        //An alterntive node was found.  Set it as the new synch node.
                        result = MC_SYNC_SetSyncNode(gNodes[next_synch_node_idx].NodeID);
                        //stop searching
                        break;
                     }
                  }
               }
            }
            else if ( gNodes[next_synch_node_idx].NodeID != MC_SYNC_GetSyncNode() )
            {
               //An alterntive node was found  Set it as the new synch node.
               if ( SUCCESS_E == MC_MFH_FindNode(current_synch_node_id, &next_synch_node) )
               {
                  CO_PRINT_B_2(DBG_INFO_E,"MFH found new synch node %d, rank %d\r\n", next_synch_node.NodeID, next_synch_node.Rank);
                  //An alterntive node was found.  Set it as the new synch node.
                  result = MC_SYNC_SetSyncNode(gNodes[next_synch_node_idx].NodeID);
               }
            }
         }
         else
         {
            CO_PRINT_B_0(DBG_INFO_E,"MFH synch node not publishing rank.  Searching for alternate..\r\n");
            //the preformed primary parent witheld its rank on the latest heartbeat.  Search the lower ranks.
            for ( uint32_t rank = 0; rank < max_acceptable_rank; rank++ )
            {
               next_synch_node_idx = MC_MFH_GetStrongestNodeAtRank(rank, true, true);
               if ( (MC_MFH_NOT_FOUND != next_synch_node_idx) && (gNodes[next_synch_node_idx].NodeID != MC_SYNC_GetSyncNode()) )
               {
                  if ( SUCCESS_E == MC_MFH_FindNode(next_synch_node_idx, &next_synch_node) )
                  {
                     CO_PRINT_B_2(DBG_INFO_E,"MFH found new synch node %d, rank %d\r\n", next_synch_node.NodeID, next_synch_node.Rank);
                     //An alterntive node was found.  Set it as the new synch node.
                     result = MC_SYNC_SetSyncNode(gNodes[next_synch_node_idx].NodeID);
                     //stop searching
                     break;
                  }
               }
            }
         }
      }
   }
#endif
   return result;
}

/*************************************************************************************/
/**
* MC_MFH_DumpVisibleDevices
* Send the info for a range of nodes to the usart output.
*
* param - firstNode      The first node of the range.
* param - lastNode       The last node of the range.
* param - uart_port      The usart to send the report to.
*
* return - void
*/
void MC_MFH_DumpVisibleDevices(const uint32_t firstNode, const uint32_t lastNode, const UartComm_t uart_port )
{
   ShortListElement_t node_data;
   char msg[128];
   uint32_t node_count = 0;
   
   if ( MAX_DEVICES_PER_SYSTEM > firstNode )
   {
      uint32_t node = firstNode;
      while ((node <= lastNode) && (MAX_DEVICES_PER_SYSTEM > node))
      {
         //Get the rank of the current synch node and set that as the max rank for consideration
         if ( SUCCESS_E == MC_MFH_FindNode(node, &node_data) )
         {
            sprintf(msg, "Node=%d, rssi=%d, snr=%d, rank=%d, LF since Hb=%d ", node_data.NodeID, 
                        node_data.RSSI, node_data.SNR,node_data.Rank,node_data.LongFramesSinceLastUpdate );
            
            if ( node == MC_SYNC_GetSyncNode() )
            {
               strcat(msg, "SYNCH\r\n");
            }
            else 
            {
               strcat(msg, "\r\n");
            }
            
            SerialDebug_Print(uart_port, DBG_INFO_E, "%s", msg);
            node_count++;
         }
         node++;
         osDelay(10);
      }
      
      if ( 0 == node_count )
      {
         SerialDebug_Print(uart_port, DBG_INFO_E, "No nodes visible");
      }
   }
}

   
/*************************************************************************************/
/**
* MC_MFH_NewLongFrame
* Called for each long frame to increment the missed heartbeat of unused nodes.
*
* param - void
*
* return - void
*/
void MC_MFH_NewLongFrame(void)
{
   uint32_t node = 0;
//   CO_PRINT_B_0(DBG_INFO_E,"MC_MFH_NewLongFrame\r\n");
   
   //Only do sync checks on RBUs
   if ( ADDRESS_NCU != gNetworkAddress )
   {
      
      //Increment the LongFramesSinceLastUpdate count of each node.
      //Hearbeats arriving in the next long frame will cancel the count.
      while ( node < MAX_DEVICES_PER_SYSTEM )
      {
         gNodes[node].LongFramesSinceLastUpdate++;
         node++;
      }
         
      node = MC_SYNC_GetSyncNode();
      if ( CO_BAD_NODE_ID != node )
      {
   //      CO_PRINT_B_1(DBG_INFO_E,"Got sync node %d\r\n", node);
         ShortListElement_t sync_node;
         if ( SUCCESS_E == MC_MFH_FindNode( node, &sync_node ) )
         {
   //         CO_PRINT_B_2(DBG_INFO_E,"Sync node %d last update %d\r\n", sync_node.NodeID, sync_node.LongFramesSinceLastUpdate);
            //Check the sync node records for long absense of the heartbeat
            if ( MAX_SYNC_LOSS_COUNT < sync_node.LongFramesSinceLastUpdate )
            {
               if ( ERR_NOT_FOUND_E == MC_MFH_FindAlternateSynchNode() )
               {
                  //No sync node and no alternative found
                   MC_SMGR_DelayedReset("Sync node lost\r\n");
               }
               else 
               {
                  node = MC_SYNC_GetSyncNode();
                  CO_PRINT_B_1(DBG_INFO_E,"Alt sync node found %d\r\n", node);
               }
            }
         }
         else 
         {
   //         CO_PRINT_B_1(DBG_INFO_E,"Record not found node %d\r\n", node);
            if ( ERR_NOT_FOUND_E == MC_MFH_FindAlternateSynchNode() )
            {
               //No sync node and no alternative found
                MC_SMGR_DelayedReset("Sync node lost\r\n");
            }
            else 
            {
               node = MC_SYNC_GetSyncNode();
               CO_PRINT_B_1(DBG_INFO_E,"New sync node found %d\r\n", node);
            }
         }
      }
      else 
      {
         CO_PRINT_B_0(DBG_INFO_E,"No sync node\r\n");
         if ( ERR_NOT_FOUND_E == MC_MFH_FindAlternateSynchNode() )
         {
            //No sync node and no alternative found
             MC_SMGR_DelayedReset("Sync node lost\r\n");
         }
         else 
         {
            node = MC_SYNC_GetSyncNode();
            CO_PRINT_B_1(DBG_INFO_E,"New sync node found %d\r\n", node);
         }
      }
      
      //Reset if all of the visible devices do not have a published rank or have missed heartbeats
      uint32_t good_sync_node_count = 0;
      for( uint32_t index = 0; index < MFH_MAX_SHORT_LIST_SIZE; index++ )
      {
         if ( CO_BAD_NODE_ID != gNodes[index].NodeID )
         {
            if ( (gNodes[index].Rank != UNASSIGNED_RANK) && (MC_MFH_MAX_MISSED_HEARTBEATS >= gNodes[index].LongFramesSinceLastUpdate) )
            {
               good_sync_node_count++;
            }
         }
      }
      
      if ( 0 == good_sync_node_count )
      {
         //No sync node and no alternative found
          MC_SMGR_DelayedReset("No valid sync nodes\r\n");
      }

   }
   
   //Check if we are scanning for devices
   if ( 0 < gScanForDevicesCount )
   {
      //decrement the count of longframes since the request to scan.
      gScanForDevicesCount--;
   }
}


/*************************************************************************************/
/**
* MC_MFH_ScanForDevices
* Open heartbeat slots for a full long frame to record visible heartbeats.
*
* param - void
*
* return - void
*/
void MC_MFH_ScanForDevices(void)
{
   CO_PRINT_B_0(DBG_INFO_E,"Scanning for devices\r\n");
   gScanForDevicesCount = DEVICE_SCAN_TIME;
   MC_SMGR_SetAllDCHSlotBehaviour(MC_TDM_DCH_RX_E);
   MfhState = MFH_STATE_SCANNING_FOR_DEVICES_E;
}

/*************************************************************************************/
/**
* MC_MFH_ScanForDevicesInProgress
* Return TRUE if a scan for devices is in progress.
*
* param - void
*
* return - TRUE if a scan for devices is in progress.
*/
bool MC_MFH_ScanForDevicesInProgress(void)
{
   bool scanning = (gScanForDevicesCount != 0);
   return scanning;
}

/*************************************************************************************/
/**
* MC_MFH_NodeResponding
* Called by the session manager when a node responds to a ping after a missed heartbeat.
* The node record is updated to reset the missed heartbeat count.
*
* param -   The node ID
*
* return - TRUE if the node record was updated.
*/
bool MC_MFH_NodeResponding(const uint16_t nodeID)
{
   bool result = false;
   uint32_t node_index =  MC_MFH_GetNodeIndex(nodeID);
   
   if ( MC_MFH_NOT_FOUND != node_index )
   {
      gNodes[node_index].MissedHeartbeats = 0;
      result = true;
   }
   
   return result;
}

/*************************************************************************************/
/**
* MC_MFH_ApplySyncCorrectionToNodes
* Apply the supplied sync correction to the expected heartbeat times of the nodes
* that are being tracked.
*
* param  - sync_correction    The correction to apply.
*
* return - void
*/
void  MC_MFH_ApplySyncCorrectionToNodes(const int32_t sync_correction)
{
   if ( 0 != sync_correction )
   {
      //CO_PRINT_B_1(DBG_INFO_E,"MC_MFH_ApplySyncCorrectionToNodes+ %d\r\n", sync_correction);
      for( uint32_t index = 0; index < MFH_MAX_SHORT_LIST_SIZE; index++ )
      {
         if ( CO_BAD_NODE_ID != gNodes[index].NodeID )
         {
//            uint16_t last_timestamp = gNodes[index].LastTimestamp;
            gNodes[index].LastTimestamp = SUM_WRAP(gNodes[index].LastTimestamp, sync_correction, LPTIM_MAX);
            //CO_PRINT_B_3(DBG_INFO_E,"Updated node %d from %d to %d\r\n", gNodes[index].NodeID, last_timestamp, gNodes[index].LastTimestamp);
         }
      }
   }
}
