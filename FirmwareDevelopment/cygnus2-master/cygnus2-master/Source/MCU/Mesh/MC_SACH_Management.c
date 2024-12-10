/*************************************************************************************
*  CONFIDENTIAL between PA and Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2018 PA Consulting Group. All rights reserved.
*  Cambridge Technology Centre
*  Melbourn Royston
*  Herts SG8 6DP UK
*  Tel: <44> 1763 261222
*
**************************************************************************************
*  File        : MC_SACH_Management.c
*
*  Description : Implementation file for the management of SACH messages
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <string.h>
#include <stdlib.h>

/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "utilities.h"
#include "MC_MacConfiguration.h"
#include "MC_AckManagement.h"
#include "SM_StateMachine.h"
#include "DM_SerialPort.h"
#include "MC_SessionManagement.h"
#include "MC_SACH_Management.h"


/* Private definitions
*************************************************************************************/
SASMsgQueue_t sachQueue;

uint16_t unitNumberWithinZone = 0;
uint16_t unitAddress = 0;
static uint32_t gDulchWatermark = 0;
/* Private functions
*************************************************************************************/
bool MC_SACH_DispatchMessage(void);


/*************************************************************************************/
/**
*  MC_SACH_Initialise
*  Resets all message queues.
*
*  @param - address                    The unit network address.
*                                      Programed by the installation engineer via PPU.
*  @return - None
*/
void MC_SACH_Initialise( const uint16_t address )
{
   unitAddress = address;
   
   sachQueue.MessageCount = 0;
   gDulchWatermark = 0;
   
   for ( uint32_t index = 0; index < SACH_MAX_MESSAGES; index++ )
   {
      sachQueue.channelQueue[index].Destination = 0;
      sachQueue.channelQueue[index].MessageType = APP_MSG_TYPE_UNKNOWN_E;
      memset( &sachQueue.channelQueue[index].Message, 0, sizeof(CO_Message_t));
   }
}


/*************************************************************************************/
/**
*  MC_SACH_ScheduleMessage
*  Queues a message in the specified SACH queue. The message is stored with the 
*  appropriate short frame index for transmission of the message.
*
*  @param - destination:   The destination node ID for the message.
*  @param - MsgType:       The message type enum from ApplicationLayerMessageType_t.
*  @param - pMessage:      Pointer to the message that we want to queue.
*  @param - SendBehaviour: Sets the send policy for the message.
*
*  @returns - true if msg was added to the queue, false otherwise
*/
bool MC_SACH_ScheduleMessage( const uint16_t destination, const ApplicationLayerMessageType_t MsgType, const uint32_t SubType, const CO_Message_t* pMessage, const SendBehaviour_t SendBehaviour )
{
   bool result = false;

   /* check that pMessage is not NULL */
   if (pMessage)
   {
      uint32_t sasQueueIndex = sachQueue.MessageCount;
      /* Check for space in the queue */
      if (SACH_MAX_MESSAGES > sasQueueIndex)
      {
         /* Add the message to the queue */
         sachQueue.channelQueue[sasQueueIndex].Destination = destination;
         sachQueue.channelQueue[sasQueueIndex].MessageType = MsgType;
         sachQueue.channelQueue[sasQueueIndex].SendBehaviour = SendBehaviour;
         sachQueue.channelQueue[sasQueueIndex].SubType = SubType;
         memcpy(&sachQueue.channelQueue[sasQueueIndex].Message, pMessage, sizeof(CO_Message_t));

         sachQueue.MessageCount++;
         
         if ( sachQueue.MessageCount > gDulchWatermark )
         {
            gDulchWatermark = sachQueue.MessageCount;
         }

         result = true;
      }
      else 
      {
         CO_PRINT_B_1(DBG_ERROR_E,"DULCH rejecting message.  Queue full,  count = %d\r\n", sachQueue.MessageCount);
      }
   }
   else 
   {
      CO_PRINT_B_0(DBG_ERROR_E,"DULCH rejecting NULL message.\r\n");
   }
   CO_PRINT_B_2(DBG_INFO_E,"DULCH message watermark = %d,  count = %d\r\n", gDulchWatermark, sachQueue.MessageCount);
   return result;   
}

/*************************************************************************************/
/**
*  MC_SACH_UpdateSACHManagement
*  Receives the super slot index and calculates if any SACH messages should be despatched.
*
*  @param - super_slot_index:    The current slot index within the super frame.
*
*  @return - TRUE if a message was despatched, false otherwise.
*/
bool MC_SACH_UpdateSACHManagement( const uint8_t SlotIndex, const uint8_t ShortFrameIndex, const uint8_t LongFrameIndex )
{
   bool msg_sent = false;
   bool send;

   /* don't do anything if there are no messages queued */
   if (sachQueue.MessageCount > 0 )
   {
      /* Calculate the unit number for the slot */
      send = MC_SACH_IsSendSlot(SlotIndex, ShortFrameIndex, LongFrameIndex);
      
      /* Only do the SACH processing if it's a SACH slot */
      if ( send )
      {
         /* Send the next message if there is one */
         if (sachQueue.MessageCount > 0)
         {
            CO_PRINT_B_3(DBG_INFO_E, "Dispatching SACH: SlotIndex = %d, ShortFrameIndex = %d, LongFrameIndex = %d\r\n", SlotIndex, ShortFrameIndex, LongFrameIndex);
            msg_sent = MC_SACH_DispatchMessage();
         }
      }
   }

   return msg_sent;
}

/*************************************************************************************/
/**
*  MC_SACH_DispatchMessage
*  Removes the first message from the specified queue and hands it to the Ack Manager 
*  for despatch over the P-RACH or S-RACH, depending on the rach_channel parameter.
*
*  @param - queueID:       The SACH queue to send a message from.
*  @param - rach_channel:  Determmines if we send on P-RACH or S-RACH.
*
*  @return -  TRUE if a message was despatched, false otherwise.
*/
bool MC_SACH_DispatchMessage(void)
{
   bool msg_sent = false;

   /* check that there is something in the queue */
   if (0 < sachQueue.MessageCount)
   {

      msg_sent = MC_ACK_AddMessage(sachQueue.channelQueue[0].Destination, ACK_DULCH_E,
         sachQueue.channelQueue[0].MessageType, sachQueue.channelQueue[0].SubType,
         false, 0, &sachQueue.channelQueue[0].Message, true,
         sachQueue.channelQueue[0].SendBehaviour);

      /* if the message was despatched, remove it from the queue */
      if (msg_sent)
      {
         sachQueue.MessageCount--;

         /*move the rest of the queue forward one place, overwriting the first entry */
         size_t size_to_copy = sizeof(SASQueueEntry_t) * (SACH_MAX_MESSAGES - 1);
         memcpy(&sachQueue.channelQueue[0], &sachQueue.channelQueue[1], size_to_copy);

         /* reset the last entry in the queue */
         uint32_t last_entry = SACH_MAX_MESSAGES - 1;
         sachQueue.channelQueue[last_entry].Destination = 0;
         sachQueue.channelQueue[last_entry].MessageType = APP_MSG_TYPE_UNKNOWN_E;
         memset(&sachQueue.channelQueue[last_entry].Message, 0, sizeof(CO_Message_t));
         
         CO_PRINT_B_2(DBG_INFO_E,"DULCH message watermark = %d,  count = %d\r\n", gDulchWatermark, sachQueue.MessageCount);
      }
   }

   return msg_sent;
}


/*************************************************************************************/
/**
* MC_SACH_IsSendSlot
*
* Returns true if the slot defined by the parameters is a SACH slot for the device..
*
* @param - const uint8_t slotIndex        : Index of the slot within the short frame
* @param - const uint8_t shortFrameIndex  : Index of the short frame within the long frame
* @param - const uint8_t longFrameIndex   : Index of the long frame within the super frame
*
* @return - bool.    True if the parameters identify a SACH slot for RBU.
*/
bool MC_SACH_IsSendSlot( const uint8_t SlotIndex, const uint8_t ShortFrameIndex, const uint8_t LongFrameIndex )
{
   bool send = false;
   uint32_t slots_per_long_frame = MC_SlotsPerLongFrame();
   uint32_t slot_in_super_frame = (slots_per_long_frame * LongFrameIndex) + (SLOTS_PER_SHORT_FRAME * ShortFrameIndex) + SlotIndex;
   uint16_t node_id = MC_GetNetworkAddress();
   send = CO_IsDulchSlot(node_id, slot_in_super_frame);
   return send;
}


/*************************************************************************************/
/**
* MC_SACH_RedirectMessagesForNode
*
* To be called when a parent is lost.  The queued messages are checked to see if they are addressed
* to the lost parent.  If the message is destined for the NCU it is redirected to the other parent.
* If the lost parent is the final destination of the message, it is deleted from the queue.
*
* @param - node_id     : node address for which to redirect messages
*
* @return - none.
*/
void MC_SACH_RedirectMessagesForNode(const uint16_t node_id)
{
   int8_t index; 
   
   /* Iterate through the queue and check each message.  Start at the last entry
      to simplify removing messages that can't be redirected */
   
   if ( 0 < sachQueue.MessageCount )
   {
      index = sachQueue.MessageCount - 1;
      do
      {
         /* check that the message is for the specified node */
         if ( node_id == sachQueue.channelQueue[index].Destination )
         {
            /* try to reroute the message */
            if ( false == MC_SACH_RerouteMessage( index ) )
            {
               /* The message cannot be rerouted.  Discard it from the queue */
               /* This is done by copying the waiting messages one place forward in the queue. */
               uint32_t queueIndex = index;
               while ( queueIndex < sachQueue.MessageCount &&
                        queueIndex < (SACH_MAX_MESSAGES - 1))/* -1 ensures that we don't exceed the array bounds */
               {
                  memcpy(&sachQueue.channelQueue[queueIndex],
                            &sachQueue.channelQueue[queueIndex+1],
                            sizeof(SASQueueEntry_t));
                  
                  queueIndex++;
               }
               sachQueue.MessageCount--;
               
               /* The messages have all been copied forward one place in the queue.
                  Clear the slot after the last message*/
               memset(&sachQueue.channelQueue[queueIndex], 0, sizeof(SASQueueEntry_t) ); 
               CO_PRINT_B_1(DBG_INFO_E, "Discarded SACH message to node %d\n\r", node_id);
            }
         }
         
         index--;
      } while ( index >= 0);
   }   
   
   CO_PRINT_B_2(DBG_INFO_E,"DULCH message watermark = %d,  count = %d\r\n", gDulchWatermark, sachQueue.MessageCount);
}

/*************************************************************************************/
/**
* MC_SACH_RerouteMessage
*
* To be called when a parent is lost.  The queued messages are checked to see if they are addressed
* to the lost parent.  It is redirected to the other parent.
*
* @param - index     : The queue slot that the message is in.
*
* @return -          : True if the message was redirected, false otherwise.
*/
bool MC_SACH_RerouteMessage( const uint8_t index )
{
   bool msg_rerouted = false;
   
   if( sachQueue.MessageCount > index )
   {     
      /* so far APP_MSG_TYPE_STATUS_INDICATION_E is the only redirectable sach message */
      if ( APP_MSG_TYPE_STATUS_INDICATION_E == sachQueue.channelQueue[index].MessageType )
      {
         /* it is addressed to a parent.  Now get the other parent nodeID */
         uint16_t nextDestination = MC_SMGR_GetNextParentForUplink();
         if ( nextDestination == sachQueue.channelQueue[index].Destination )
         {
            /* it's the other parent that we need */
            nextDestination = MC_SMGR_GetNextParentForUplink();
         }
         
         /* if all has gone well, we should have different node IDs for currentDestination and nextDestination */
         if ( nextDestination != sachQueue.channelQueue[index].Destination &&  CO_BAD_NODE_ID != nextDestination )
         {
            uint8_t* pMessage = sachQueue.channelQueue[index].Message.Payload.PhyDataInd.Data;
            if ( pMessage )
            {
               /* overwrite the destination address in the message */
               *pMessage &= 0xf0;
               *(pMessage + 1) = 0x00;
               *pMessage |= (nextDestination >> 8);
               *(pMessage+1) = (uint8_t)(nextDestination & 0xff);
               
               CO_PRINT_B_2(DBG_INFO_E, "Redirected SACH message from node %d to node %d\n\r", sachQueue.channelQueue[index].Destination, nextDestination);
              
               sachQueue.channelQueue[index].Destination = nextDestination;
               
               msg_rerouted = true;
            } 
         }
      }
   }
   
   return msg_rerouted;
}

/*************************************************************************************/
/**
* MC_SACH_MessageCount
*
* Return the number of messages in the DULCH queue.
*
* @param -  None.
*
* @return - The message count.
*/
uint32_t MC_SACH_MessageCount(void)
{
   return sachQueue.MessageCount;
}


