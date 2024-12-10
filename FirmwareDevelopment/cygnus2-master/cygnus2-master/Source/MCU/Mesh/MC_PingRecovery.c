/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2020 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : MC_PingRecovery.c
*
*  Description  : Implementation of the Ping Recovery mechanism after consecutive
*                 heartbeats are missed
*
*
*************************************************************************************/

/* System Include Files
*************************************************************************************/
#include <inttypes.h>       // Required for C99 fixed size integers.
#include <cmsis_os.h>

/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "MM_NeighbourInfo.h"
#include "MC_MacConfiguration.h"
#include "CFG_Device_cfg.h"
#include "MC_SessionManagement.h"
#include "MC_PingRecovery.h"

/* Macros
*************************************************************************************/
#define PR_MAX_PING_FAIL_COUNT 3U
#define PR_PING_INTERVAL 10U //RACH-S slots.  Interval before the ping is sent

/* Private Functions Prototypes
*************************************************************************************/

/* Global Variables
*************************************************************************************/
extern NeighbourInformation_t neighbourArray[MAX_DEVICES_PER_SYSTEM];
extern osPoolId MeshPool;
extern osMessageQId(MeshQ);

/* Private Variables
*************************************************************************************/

/* Public Functions Definitions
*************************************************************************************/

/*************************************************************************************/
/**
* MC_PR_Initialise
* Initialise the Ping Recovery module.
*
* @param - None.
*
* @return - None.
*/
void MC_PR_Initialise(void)
{
   /*Do Nothing*/
}


/*************************************************************************************/
/**
* MC_PR_StartPingForNode
* Initiate Ping Recovery for the supplied node.
*
* @param - nodeID          The node to ping
*
* @return - ErrorCode_t    SUSSESS_E or error code.
*/
ErrorCode_t MC_PR_StartPingForNode(const uint16_t nodeID)
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   
   if ( MAX_DEVICES_PER_SYSTEM > nodeID )
   {
      if ( PR_IDLE_E == neighbourArray[nodeID].PingState )
      {
         neighbourArray[nodeID].PingState = PR_WAITING_TO_SEND_E;
         neighbourArray[nodeID].LostHeartbeatPingCount = 0;
         //Force first ping to go immediately
         neighbourArray[nodeID].LostHeartbeatPingInterval = PR_PING_INTERVAL;
      }
      else 
      {
         result = ERR_BUSY_E;
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* MC_PR_CancelPingForNode
* Cancel Ping Recovery for the supplied node.
*
* @param - nodeID          The node to ping
*
* @return - ErrorCode_t    SUSSESS_E or error code.
*/
ErrorCode_t MC_PR_CancelPingForNode(const uint16_t nodeID)
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   
   if ( MAX_DEVICES_PER_SYSTEM > nodeID )
   {
      neighbourArray[nodeID].PingState = PR_IDLE_E;
      neighbourArray[nodeID].LostHeartbeatPingCount = 0;
      neighbourArray[nodeID].LostHeartbeatPingInterval = 0;
      neighbourArray[nodeID].LostHeartbeatCount = 0;
      result = SUCCESS_E;
   }
   
   return result;
}
/*************************************************************************************/
/**
* MC_PR_Advance
* Called for each short frame to advance the ping process.
*
* @param - None
*
* @return - none
*/
void MC_PR_Advance(const uint16_t shortFrame, const uint16_t slotIndex)
{
   bool send_ping = false;
   uint16_t node_to_ping = CO_BAD_NODE_ID;
   uint32_t max_children;
   uint16_t node_id;
   uint16_t network_address = MC_GetNetworkAddress();
   uint16_t slot_hold_off = network_address % 7;//how many SRACH slots to wait before sending a ping
   
   //Check Primary parent
   node_id = MC_SMGR_GetPrimaryParentID();
   if ( CO_BAD_NODE_ID != node_id )
   {
      if ( PR_WAITING_TO_SEND_E == neighbourArray[node_id].PingState )
      {
         neighbourArray[node_id].LostHeartbeatPingInterval++;
         if ( (PR_PING_INTERVAL + slot_hold_off) <= neighbourArray[node_id].LostHeartbeatPingInterval )
         {
            node_to_ping = node_id;
            neighbourArray[node_id].LostHeartbeatPingInterval = 1;
            neighbourArray[node_id].PingState = PR_WAITING_FOR_ACK;
            send_ping = true;
         }
      }
   }
   
   //Check Secondary parent
   node_id = MC_SMGR_GetSecondaryParentID();
   if ( CO_BAD_NODE_ID != node_id )
   {
      if ( PR_WAITING_TO_SEND_E == neighbourArray[node_id].PingState )
      {
         neighbourArray[node_id].LostHeartbeatPingInterval++;
         if ( !send_ping )
         {
            if ( (PR_PING_INTERVAL + slot_hold_off) <= neighbourArray[node_id].LostHeartbeatPingInterval )
            {
               node_to_ping = node_id;
               neighbourArray[node_id].LostHeartbeatPingInterval = 1;
               neighbourArray[node_id].PingState = PR_WAITING_FOR_ACK;
               send_ping = true;
            }
         }
      }
   }
   
   //Check the child nodes
   max_children = CFG_GetMaxNumberOfChildren();
   
   for ( uint32_t node_index = 0; node_index <= max_children; node_index++ )
   {
      node_id = MC_SMGR_GetChildIDByIndex(node_index);
      if ( CO_BAD_NODE_ID != node_id )
      {
         if ( PR_WAITING_TO_SEND_E == neighbourArray[node_id].PingState )
         {
            neighbourArray[node_id].LostHeartbeatPingInterval++;
            //CO_PRINT_B_2(DBG_INFO_E,"Node=%d, count=%d\r\n", node_id, neighbourArray[node_id].LostHeartbeatPingInterval);
            //The ping_sent flag prevents a deluge of ping requests in the event of a mass drop-off
            //by limiting pings to one per short frame.
            if ( !send_ping )
            {
               if ( (PR_PING_INTERVAL + slot_hold_off) <= neighbourArray[node_id].LostHeartbeatPingInterval )
               {
                  node_to_ping = node_id;
                  neighbourArray[node_id].LostHeartbeatPingInterval = 1;
                  neighbourArray[node_id].PingState = PR_WAITING_FOR_ACK;
                  send_ping = true;
               }
            }
         }
      }
   }
   
   //Did we find a node that needs to be pinged?
   if ( send_ping )
   {
      if ( CO_BAD_NODE_ID != node_to_ping )
      {
         MC_PR_SendPing(node_to_ping, SEND_THREE_E);
      }
   }
}

/*************************************************************************************/
/**
* MC_PR_AckUpdate
* Handle received ACK or failure to ACK.
*
* @param - nodeID          The node to ping
* @param - ackReveived     True if an ack was received, false if not.
*
* @return - ErrorCode_t    ERR_SIGNAL_LOST_E if an node fails to recover (i.e. link should be dropped)
*                          ERR_TIMEOUT_E if an node fails to respond, but we haven't finished pinging.
*                          SUCCESS_E if the node has responded and the link should be maintained..
*                          ERR_OUT_OF_RANGE_E if the supplied node ID is invalid.
*/
ErrorCode_t MC_PR_AckUpdate(const uint16_t nodeID, const bool ackReveived)
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   
   if ( MAX_DEVICES_PER_SYSTEM > nodeID )
   {
      if ( PR_WAITING_FOR_ACK == neighbourArray[nodeID].PingState )
      {
         if ( ackReveived )
         {
            //We got a ping back from the node.  Maintain the link.
            result = MC_PR_CancelPingForNode(nodeID);
         }
         else 
         {
            //No ACK was received.  Increment count.
            neighbourArray[nodeID].LostHeartbeatPingCount++;
            //Have we reached the max lost pings?
            if ( PR_MAX_PING_FAIL_COUNT <= neighbourArray[nodeID].LostHeartbeatPingCount )
            {
               //We have reached the max lost ping count.  Tell the session manager to drop the link.
               result = ERR_SIGNAL_LOST_E;
               MC_PR_CancelPingForNode(nodeID);
            }
            else 
            {
               //We haven't exhausted all of our pings yet.  Schedule another.
               result = ERR_TIMEOUT_E;
               neighbourArray[nodeID].PingState = PR_WAITING_TO_SEND_E;
            }
         }
      }
      else
      {
         result = ERR_NOT_FOUND_E;
      }
   }
   
   return result;
}

/*****************************************************************************/
/**
*  Function:         MC_PR_SendPing
*  Description:      Send a ping to a neighbour node.
*                    Note. We only expect an ACK to this mesaage to confirm the Ping.
*
*  @param   nodeID   The destination node for the ping.
*  @return  None.
*            
*****************************************************************************/
void MC_PR_SendPing(const uint16_t nodeID, const SendBehaviour_t behaviour)
{
   osStatus osStat = osErrorOS;
   CO_Message_t *pPingReq;
   uint16_t network_address = MC_GetNetworkAddress();
   
   CO_PRINT_B_1(DBG_INFO_E,"PingReq node=%d\r\n", nodeID);
 
   /* create cmd message and put into mesh queue */
   pPingReq = ALLOCMESHPOOL;
   if (pPingReq)
   {
      PingRequest_t pingRequest;
      pingRequest.Header.FrameType = FRAME_TYPE_DATA_E;
      pingRequest.Header.MACDestinationAddress = nodeID;
      pingRequest.Header.MACSourceAddress = network_address;
      pingRequest.Header.HopCount = 0;
      pingRequest.Header.DestinationAddress = nodeID;
      pingRequest.Header.SourceAddress = network_address;
      pingRequest.Header.MessageType = APP_MSG_TYPE_PING_E;
      pingRequest.SystemId = MC_GetSystemID();
      pingRequest.SendBehaviour = behaviour;

      pPingReq->Type = CO_MESSAGE_GENERATE_PING_REQUEST_E;
      memcpy(pPingReq->Payload.PhyDataReq.Data, &pingRequest, sizeof(PingRequest_t));
      osStat = osMessagePut(MeshQ, (uint32_t)pPingReq, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         FREEMESHPOOL(pPingReq);
      }
   }
}

