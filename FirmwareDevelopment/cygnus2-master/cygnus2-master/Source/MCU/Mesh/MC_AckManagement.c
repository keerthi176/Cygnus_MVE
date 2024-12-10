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
*  File        : MC_AckManagement.c
*
*  Description : Source for the management of message acks and message re-transmission
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <string.h>
#include <stdlib.h>

/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "MC_AckManagement.h"
#include "MC_SessionManagement.h"
#include "MC_MAC.h"
#include "DM_SerialPort.h"
#include "MC_SACH_Management.h"

/* Private definitions
*************************************************************************************/
UnackedMsgQueue_t unackQueue[ACK_NUMBER_OF_ACKNOWLEDGEMENT_QUEUES];
//const uint32_t BackoffExponentFilter[ACK_MAX_BACKOFF_EXPONENT + 1] = { 1, 2, 4, 8, 16, 32, 64, 128, 256 };
const uint32_t BackoffExponentFilter[ACK_MAX_BACKOFF_EXPONENT + 1] = { 1, 8, 16, 24, 48, 64, 96, 128, 164, 192, 256, 256, 256 };

/* Private Functions Prototypes
*************************************************************************************/
bool MC_ACK_RerouteMessage( const AcknowledgedQueueID_t channelID, const uint8_t queue_index );
uint16_t MC_ACK_GetMacDestinationOfQueuedMessage( const AcknowledgedQueueID_t channelID, const uint8_t queue_index );
bool MC_ACK_CheckForDuplicateMessage(const AcknowledgedQueueID_t channel, const ApplicationLayerMessageType_t MsgType, const uint32_t SubType,
                                       const uint32_t MsgHandle, const CO_Message_t* pNewMessage);
/* Global Variables
*************************************************************************************/
extern uint32_t gNetworkAddress;
extern bool gPrimaryAckExpected;
extern bool gSecondaryAckExpected;

/* Private Variables
*************************************************************************************/
static bool bMessageDiscarded = false;
static uint32_t gDiscardedMessageHandle;
static ApplicationLayerMessageType_t gDiscardedMessageType;
static uint16_t gDiscardedMessageMacDestination;
static bool gServicingDulchChannel = false;
#ifdef DUPLICATE_MESSAGE_REPLACEMENT_ENABLED
static void (*pfDuplicateMessageCallback)(const uint32_t DiscardedHandle, const uint32_t NewHandle, const ApplicationLayerMessageType_t MsgType) = NULL;
#endif
/*****************************************************************************
*  Function:      void MC_MB_initialise
*  Description:   Resets all message queues.
*  Returns:       nothing
*  Notes:         none
*****************************************************************************/
void MC_ACK_Initialise(  )
{   
   for ( uint32_t queue = 0; queue < ACK_NUMBER_OF_ACKNOWLEDGEMENT_QUEUES; queue++)
   {
      unackQueue[queue].ReadyToSend = false;
      unackQueue[queue].WaitingForACK = false;
      unackQueue[queue].BackoffExp = 0;
      unackQueue[queue].BackoffCounter = 0;
      unackQueue[queue].MessageCount = 0;
      unackQueue[queue].WaterMark = 0;
     /* clear the unack'd message list */
      for ( uint32_t index = 0; index < ACK_MAX_UNACKED_MESSAGES; index++ )
      {
         unackQueue[queue].channelQueue[index].MessageHandle = 0;
         unackQueue[queue].channelQueue[index].MessageType = APP_MSG_TYPE_UNKNOWN_E;
         unackQueue[queue].channelQueue[index].SubType = 0;
         unackQueue[queue].channelQueue[index].SendBehaviour = SEND_NORMAL_E;
         unackQueue[queue].channelQueue[index].RouteDiversity = false;
         memset(&unackQueue[queue].channelQueue[index].Message, 0, sizeof(CO_Message_t) );
         unackQueue[queue].channelQueue[index].Message.Type = CO_MESSAGE_PHY_DATA_REQ_E;
      }
   }
   
   gDiscardedMessageHandle = 0;
   gDiscardedMessageType = APP_MSG_TYPE_UNKNOWN_E;
   gDiscardedMessageMacDestination = CO_BAD_NODE_ID;
   
   /* seed the random number generator that is used for back-off delays */
   srand( gNetworkAddress );
}

/*************************************************************************************/
/**
* MC_ACK_SetDuplicateMessageCallback
* Set the callback function to be called when an added message is found to be a duplicate
* of one already in the queue.  This is to enable notification that a message Handle has been
* superceeded.
*
* @param - pfCallback  Pointer to the callback function.
*
* @return - void

*/
void MC_ACK_SetDuplicateMessageCallback(void (*pfCallback)(const uint32_t DiscardedHandle, const uint32_t NewHandle, const ApplicationLayerMessageType_t MsgType))
{
   CO_ASSERT_VOID_MSG(pfCallback, "MC_ACK_SetDuplicateMessageCallback pfCallback=NULL");
#ifdef DUPLICATE_MESSAGE_REPLACEMENT_ENABLED
   pfDuplicateMessageCallback = pfCallback;
#endif
}


/*****************************************************************************
*  Function:     MC_ACK_AddMessage
*  Description:            Queues a message while an ACK is outstanding.
*  param neighbourID:      The ID for the neighbour (probably the source address).
*  param channel:          ACK_RACHP, ACK_RACHS etc.
*  param MsgType:          The message type enum from ApplicationLayerMessageType_t.
*  param MultiHop:         Set to TRUE if the MAC Destination is different to the final destination.
*  param MsgHandle:        The message ID that was generated by the application (Handle).
*  param pNewMessage:      Pointer to the message that we expect an ACK for.
*  param SendImmediately:  Set this to true if you want the message to go asap.  You must set this to
*                          false if you have manually pushed the message into the send queue ( RACHPQ or RACHSQ ) 
*                          before calling this function, otherwise the ACK Manager will push a duplicate message
*                          into the send queue (if there are no previous messages pending an ACK).
*  param SendBehaviour:    Value from SendBehaviour_t.
*  Returns:                true if msg was accepted, false otherwise
*  Notes:                  none
*****************************************************************************/
bool MC_ACK_AddMessage( const uint32_t neighbourID, const AcknowledgedQueueID_t channel, const ApplicationLayerMessageType_t MsgType, const uint32_t SubType,
                        const bool MultiHop, const uint32_t MsgHandle, const CO_Message_t* pNewMessage, const bool SendImmediately,
                        const SendBehaviour_t SendBehaviour )
{
   bool result = false;

   if ( ACK_MAX_QUEUE_ID_E > channel )
   {
      uint32_t msgQueueIndex = unackQueue[channel].MessageCount;
      /* Check for space in the queue */
      if ( ACK_MAX_UNACKED_MESSAGES > msgQueueIndex)
      {
         if ( false == MC_ACK_CheckForDuplicateMessage(channel, MsgType, SubType, MsgHandle, pNewMessage) )
         {         
            unackQueue[channel].channelQueue[msgQueueIndex].MessageHandle = MsgHandle;
            unackQueue[channel].channelQueue[msgQueueIndex].MessageType = MsgType;
            unackQueue[channel].channelQueue[msgQueueIndex].SubType = SubType;
            unackQueue[channel].channelQueue[msgQueueIndex].RouteDiversity = false;
            unackQueue[channel].channelQueue[msgQueueIndex].SendBehaviour = SendBehaviour;
            memcpy(&unackQueue[channel].channelQueue[msgQueueIndex].Message, pNewMessage, sizeof(CO_Message_t));
         
         //unackQueue[channelID].channelQueue[0].NumberToSend
            switch ( SendBehaviour )
            {
               case SEND_TWO_E:
                  unackQueue[channel].channelQueue[msgQueueIndex].NumberToSend = 2;
                  break;
               case SEND_THREE_E:
                  unackQueue[channel].channelQueue[msgQueueIndex].NumberToSend = 3;
                  break;
               default:
                  unackQueue[channel].channelQueue[msgQueueIndex].NumberToSend = ACK_MAX_BACKOFF_EXPONENT;
                  break;
            }
            
            /* Check MultiHop to see if the message is going beyond the next node.
               Then check neighbourID to see if it is a parent. 
               If both are true, set RouteDiversity to true. This signals that we have to do alternate-parent resends */
            if ( MultiHop && MC_SMGR_IsParentNode(neighbourID) )
            {
               unackQueue[channel].channelQueue[msgQueueIndex].RouteDiversity = true;
            }

            unackQueue[channel].MessageCount++;
            if ( unackQueue[channel].MessageCount > unackQueue[channel].WaterMark )
            {
               unackQueue[channel].WaterMark = unackQueue[channel].MessageCount;
               CO_PRINT_B_3(DBG_INFO_E,"Ack Queue %d new water-mark=%d/%d\r\n",channel, unackQueue[channel].WaterMark, ACK_MAX_UNACKED_MESSAGES);
            }
            
            
            /* if the parameter SendImmediately is true and there is nothing in the queue, mark it 'ready to send' */
            if ( (0 == msgQueueIndex) && (true == SendImmediately) )
            {
               unackQueue[channel].channelQueue[msgQueueIndex].NumberToSend--;
               unackQueue[channel].BackoffCounter = 2;//A one or zero here will cause the ReadyToSend flag to get set again on the next slot update, causing the message to be sent twice
               unackQueue[channel].BackoffExp = 0; 
               unackQueue[channel].ReadyToSend = true;
               if ( ACK_DULCH_E == channel )
               {
                  gServicingDulchChannel = true;
               }
            }
            else
            {
               /* if there is nothing else in the queue then we got here because param SendImmediately was false.
                  In this case we need to start the backoff algorithm now by calling MC_ACK_RescheduleMessage() */
               if( (0 == msgQueueIndex) )
               {
                  unackQueue[channel].BackoffExp = 0;
                  unackQueue[channel].ReadyToSend = false;
                  uint32_t handle;
                  MC_ACK_RescheduleMessage(channel, &handle);
               }
            }
         }
         result = true;
      }
   }
   
   if (false == result)
   {
      CO_PRINT_A_0(DBG_ERROR_E, "Failed to add message in MC_ACK_AddMessage\r\n");
   }
      
   return result;
}

/*****************************************************************************
*  Function:     MC_ACK_AcknowledgeMessage
*  Description:         Removes the first message from the specified channel queue.
*                       The ACK response does not contain a msg ID, so we can't
*                       map a specific msg to the ACK response.  Because of this
*                       We only send one msg at a time, and queue the others until
*                       the first one is ACK'd.  Therefore the message at the front
*                       of the queue is always the one to remove.
*  Param channelID:     ACK_RACHP_E, ACK_RACHS_E.
*  Param ackReceived:   True for genuine ACK. False if Ack Mgr called this to discard the message.
*  Param pMsgType:      OUT parameter.  Populated with the message type of the Ack'd message;
*  Param pMsgHandle:    OUT parameter.  Populated with the message Handle of the Ack'd message;
*  Returns:             true if a message was removed, false otherwise.
*  Notes:               None.
*****************************************************************************/
bool MC_ACK_AcknowledgeMessage( const AcknowledgedQueueID_t channelID, const uint16_t ackSourceNode, const bool ackReceived, ApplicationLayerMessageType_t* pMsgType, uint32_t* pMsgHandle )
{
   bool result = false;
   AcknowledgedQueueID_t channel = channelID;
   
   if ( gServicingDulchChannel && (ACK_RACHS_E == channelID) )
   {
      channel = ACK_DULCH_E;
   }
   
   
   if ( ACK_MAX_QUEUE_ID_E > channel )
   {
      uint16_t expected_source_node_id = MC_ACK_GetMacDestinationOfCurrentMessage(channel);
      
      /* Check that there is a message waiting in the channel */
      if ( 0 < unackQueue[channel].MessageCount && ackSourceNode == expected_source_node_id)
      {
         
         if ( ackReceived )
         {
            //If this was a Ping message, tell the session manager that a response was received
            if ( APP_MSG_TYPE_PING_E == unackQueue[channel].channelQueue[0].MessageType )
            {
               MC_SMGR_ProcessPingResponse(expected_source_node_id, true);
            }
            /* Tell the session manager that the parent responded */
            MC_SMGR_ProcessParentAckReport(expected_source_node_id, true, unackQueue[channel].channelQueue[0].MessageType);
         }
         else 
         {
            //If this was a Ping message, tell the session manager that no response was received
            if ( APP_MSG_TYPE_PING_E == unackQueue[channel].channelQueue[0].MessageType )
            {
               MC_SMGR_ProcessPingResponse(expected_source_node_id, false);
            }
            else if ( APP_MSG_TYPE_ROUTE_ADD_E == unackQueue[channel].channelQueue[0].MessageType )
            {
               //The session manager needs to know if a route add message is discarded so that it can seek alternatives
               uint16_t node_id = MC_ACK_GetMacDestinationOfCurrentMessage(channel);
               MC_SMGR_ProcessRouteAddFailure(node_id);
            }
         }
         
         //Remove the message from the queue. Copy the message type and Handle to the OUT parameters
         MC_ACK_DiscardMessage(channel, pMsgType, pMsgHandle);
         
         /* If another message was queued, it has been brought to the front of the queue.
            Mark the queue as ready to send so that the message can be sent in the next slot*/
         if ( 0 < unackQueue[channel].MessageCount )
         {
            CO_PRINT_B_1(DBG_INFO_E, "AckManagement scheduling next message on RACH%d\n\r", channel+1 );
            unackQueue[channel].ReadyToSend = true;
         }
         else
         {
            unackQueue[channel].ReadyToSend = false;
         }

         result = true;
      }
   }
   return result;
}

/*****************************************************************************
*  Function:     MC_ACK_DiscardMessage
*  Description:         Discards the message at the front of the stated queue.
*  Param channelID:     ACK_RACHP_E, ACK_RACHS_E.
*  Param pMsgType:      OUT parameter.  Populated with the message type of the discarded message;
*  Param pMsgHandle:    OUT parameter.  Populated with the message Handle of the discarded message;
*  Returns:             true if a message is removed from the queue queue, false otherwise.
*  Notes:               None.
*****************************************************************************/
bool MC_ACK_DiscardMessage( const AcknowledgedQueueID_t channelID, ApplicationLayerMessageType_t* pMsgType, uint32_t* pMsgHandle )
{
   bool result = false;
   AcknowledgedQueueID_t channel = channelID;
   
   if ( gServicingDulchChannel && (ACK_RACHS_E == channelID) )
   {
      channel = ACK_DULCH_E;
   }
   //reset the dulch flag
   gServicingDulchChannel = false;
   
   //check that there is a message in the queue
   if ( unackQueue[channel].MessageCount )
   {
      /* Copy the message Handle to the OUT parameter pMsgHandle*/
      if (pMsgHandle)
      {
         *pMsgHandle = unackQueue[channel].channelQueue[0].MessageHandle;
      }
      /* Copy the message type to the OUT parameter pMsgType*/
      if (pMsgType)
      {
         *pMsgType = unackQueue[channel].channelQueue[0].MessageType;
      }
         
      /* Remove the message from the front of the queue */
      /* This is done by copying the waiting messages one place forward in the queue.
         We could speed things up by using pointers, but that would increase the complexity
         of managing the queue.
      */
      uint32_t queueIndex = 0;
      while ( queueIndex < unackQueue[channel].MessageCount &&
               queueIndex < (ACK_MAX_UNACKED_MESSAGES - 1))/* ensures that we don't exceed the array bounds */
      {
         memcpy(&unackQueue[channel].channelQueue[queueIndex],
                   &unackQueue[channel].channelQueue[queueIndex+1],
                   sizeof(mbQueueEntry_t));
         
         queueIndex++;
      }
      unackQueue[channel].MessageCount--;
      
      /* The messages have all been copied forward one place in the queue.
         Clear the slot after the last massege*/
      memset(&unackQueue[channel].channelQueue[queueIndex], 0, sizeof(mbQueueEntry_t) );
      //Set the last slot message type to unknown
      unackQueue[channel].channelQueue[queueIndex].MessageType = APP_MSG_TYPE_UNKNOWN_E;

      /* reset the back-off system */
      unackQueue[channel].BackoffExp = 0;
      unackQueue[channel].BackoffCounter = 0;
      unackQueue[channel].WaitingForACK = false;
      result = true;
   }
   
   return result;
}

/*****************************************************************************
*  Function:     MC_ACK_WaitingForACK
*  Description:         Checks the specified neighbour's queues for unack'd messages.
*  Param channelID:     ACK_RACHP_E, ACK_RACHS_E.
*  Returns:             true if there is a message in any queue, false otherwise.
*  Notes:               None.
*****************************************************************************/
bool MC_ACK_WaitingForACK( const AcknowledgedQueueID_t channelID )
{
   bool result = false;
   AcknowledgedQueueID_t channel = channelID;
   
   if ( gServicingDulchChannel && (ACK_RACHS_E == channelID) )
   {
      channel = ACK_DULCH_E;
   }
   
   if ( ACK_MAX_QUEUE_ID_E > channel )
   {
      if ( 0 < unackQueue[channel].MessageCount )
      {
         result = true;
      }
   }
   return result;
}

/*****************************************************************************
*  Function:     MC_ACK_UpdateAckExpectedFlags
*  Description:         Updates the global flags gPrimaryAckExpected and 
*                       gSecondaryAckExpected.  These are set to true if
*                       an ACK is still pending after  sending an earlier RACH.
*  Param                None
*  Returns:             None
*  Notes:               None.
*****************************************************************************/
void MC_ACK_UpdateAckExpectedFlags()
{
   gPrimaryAckExpected = false;
   gSecondaryAckExpected = false;
   
      // Check for pending ACKs relating the primary RACH
      if ( 0 < unackQueue[ACK_RACHP_E].MessageCount )
      {
         gPrimaryAckExpected = true;
      }

      // Check for pending ACKs relating the secondary RACH
      if ( 0 < unackQueue[ACK_RACHS_E].MessageCount )
      {
         gSecondaryAckExpected = true;
      }  
}

/*****************************************************************************
*  Function:     MC_ACK_UpdateSlot
*  Description:         Updates the back-off counters for the specified channel.
*  Param channelID:     ACK_RACHP, ACK_RACHS etc
*  Returns:             true if a message is due to be sent, false otherwise.
*  Notes:               The slot counter is decremented each time this function
*                       is called. When it reaches 0, the msg at the front of the 
*                       queue should be transmitted again.
*****************************************************************************/
bool MC_ACK_UpdateSlot( const AcknowledgedQueueID_t channelID )
{
   AcknowledgedQueueID_t channel = channelID;
   
   if ( gServicingDulchChannel && (ACK_RACHS_E == channelID) )
   {
      channel = ACK_DULCH_E;
   }
   
   if ( ACK_MAX_QUEUE_ID_E > channel )
   {
      if ( 0 < unackQueue[channel].MessageCount )
      {
         if ( 0 < unackQueue[channel].BackoffCounter )
         {
            unackQueue[channel].BackoffCounter--;
         }
         else
         {
            /* There is a message in the queue and the back-off delay has expired.  Call MC_ACK_RescheduleMessage() to
               schedule the next resend time.  This could result in the message being discarded if the back-off algorithm
               has reached its limit. MC_ACK_RescheduleMessage() will return false if this is the case.*/            
            uint32_t handle;
            unackQueue[channel].ReadyToSend = MC_ACK_RescheduleMessage( channel, &handle );
         }
      }
   }
   return unackQueue[channel].ReadyToSend;
}


/*****************************************************************************
*  Function:     MC_ACK_GetRandomDelay
*  Description:         Returns a delay value in slots within a range that
*                       depends on the supplied back-off exponent.
*  Param backOffExp:    The back-off exponent to use.
*  Returns:             A delay value in slots.
*  Notes:               
*****************************************************************************/
uint32_t MC_ACK_GetRandomDelay( const uint32_t backOffExp )
{
   uint32_t result = 0;
   
   /* Use the supplied back-off exponent to limit the spread of the returned value */
   if (0 < backOffExp && ACK_MAX_BACKOFF_EXPONENT > backOffExp)
   {
      result = (uint32_t)rand();
      result %= BackoffExponentFilter[backOffExp];
   }
   
   return result;
}

/*****************************************************************************
*  Function:     MC_ACK_GetMessage
*  Description:         Looks in the message queues of each neighbour for a message
*                       that is ready for transmission.
*  Param pMessage:      INPUT. Pointer to a message structure that will accept the
*                       message that needs to be transmitted.
*  Returns:             true if a message was copied into pMessage, false otherwise.
*  Notes:               This function will check each neighbour in the order that
*                       they were added for 'ready' messages.
*****************************************************************************/
bool MC_ACK_GetMessage( const AcknowledgedQueueID_t channelID, CO_Message_t* pMessage)
{
   bool result = false;
   AcknowledgedQueueID_t channel = channelID;
   
   if ( gServicingDulchChannel && (ACK_RACHS_E == channelID) )
   {
      channel = ACK_DULCH_E;
   }

   if (pMessage)
   {
      if ( ACK_MAX_QUEUE_ID_E > channel )
      {
         if (unackQueue[channel].ReadyToSend)
         {
            /* There is a message that is ready to send.  Copy it into pMessage */
            pMessage->Type = unackQueue[channel].channelQueue[0].Message.Type;
            pMessage->Payload.PhyDataReq.Size = unackQueue[channel].channelQueue[0].Message.Payload.PhyDataReq.Size;
            pMessage->Payload.PhyDataReq.Downlink = unackQueue[channel].channelQueue[0].Message.Payload.PhyDataReq.Downlink;
            memcpy(pMessage->Payload.PhyDataReq.Data, unackQueue[channel].channelQueue[0].Message.Payload.PhyDataReq.Data,
                     unackQueue[channel].channelQueue[0].Message.Payload.PhyDataReq.Size);
            /* Once the message has been read, we mark it as 'not ready to send.' If it isn't ack'd, the backoff system
            will set it back to 'ready' after the appropriate delay*/
            unackQueue[channel].ReadyToSend = false;
            unackQueue[channel].WaitingForACK = true;
            result = true;
         }
      }
   }

   return result;
}

/*****************************************************************************
*  Function:     MC_ACK_PeekMessage
*  Description:         returns a copy of the message at the front of the queue.
*  Param pMessage:      INPUT. Pointer to a message structure that will accept the
*                       message that needs to be transmitted.
*  Returns:             true if a message was copied into pMessage, false otherwise.
*  Notes:
*****************************************************************************/
bool MC_ACK_PeekMessage( const AcknowledgedQueueID_t channelID, CO_Message_t* pMessage)
{
   bool result = false;
   AcknowledgedQueueID_t channel = channelID;
   
   if ( gServicingDulchChannel && (ACK_RACHS_E == channelID) )
   {
      channel = ACK_DULCH_E;
   }

   if (pMessage)
   {
      if ( ACK_MAX_QUEUE_ID_E > channel )
      {
         if (unackQueue[channel].MessageCount)
         {
            /* There is a message that is ready to send.  Copy it into pMessage */
            pMessage->Type = unackQueue[channel].channelQueue[0].Message.Type;
            pMessage->Payload.PhyDataReq.Size = unackQueue[channel].channelQueue[0].Message.Payload.PhyDataReq.Size;
            memcpy(pMessage->Payload.PhyDataReq.Data, unackQueue[channel].channelQueue[0].Message.Payload.PhyDataReq.Data,
                     unackQueue[channel].channelQueue[0].Message.Payload.PhyDataReq.Size);
            result = true;
         }
      }
   }

   return result;
}

/*****************************************************************************
*  Function:     MC_ACK_MessageReadyToSend
*  Description:         Looks in the message queue for a message
*                       that is ready for transmission on a specified channel.
*  Param channel:       ACK_RACHP_E or ACK_RACHS_E.  The RACH channel to check.
*  Returns:             true if a message was found, false otherwise.
*  Notes:               This function will check each neighbour in the order that
*                       they were added for 'ready' messages.
*****************************************************************************/
bool MC_ACK_MessageReadyToSend( const AcknowledgedQueueID_t channelID )
{
   bool readyToSend = false;
   AcknowledgedQueueID_t channel = channelID;
   
   if ( gServicingDulchChannel && (ACK_RACHS_E == channelID) )
   {
      channel = ACK_DULCH_E;
   }
   
   if ( ACK_MAX_QUEUE_ID_E > channel )
   {
      if (unackQueue[channel].ReadyToSend)
      {
         /* There is a message that is ready to send. */
         readyToSend = true;
      }
   }
   
   return readyToSend;
}


/*****************************************************************************
*  Function:     MC_ACK_RescheduleMessage
*  Description:         Recalculates the slot delay for a message to be resent.
*  Param channelID:     ACK_RACHP or ACK_RACHS.
*  Param pMsgHandle:    OUT parameter.  Populated with the message Handle if it's discarded;
*  Returns:             true if the message was rescheduled, false otherwise.
*  Notes:               none.
*****************************************************************************/
bool MC_ACK_RescheduleMessage( const AcknowledgedQueueID_t channelID, uint32_t* pMsghandle )
{
   bool result = false;
   bool DiscardCurrentMessage = false;
   ApplicationLayerMessageType_t MessageType;
   AcknowledgedQueueID_t channel = channelID;
   
   if ( gServicingDulchChannel && (ACK_RACHS_E == channelID) )
   {
      channel = ACK_DULCH_E;
   }

   /* if the message went unacknowledged (BackoffExp > 0) we need to tell the session mannager 
      so that it can decide whether to drop the parent */
   if ( 0 < unackQueue[channel].BackoffExp )
   {
      uint16_t currentDestination = MC_ACK_GetMacDestinationOfCurrentMessage( channel );
      MC_SMGR_ProcessParentAckReport(currentDestination, false, unackQueue[channel].channelQueue[0].MessageType);
   }

 
   /*The resend back-off has completed.  Mark channel as ready to resend */
   /*  Increment the channel's BackoffExp and calculate the resend delay
      (in slots) for BackoffCounter */
   unackQueue[channel].BackoffExp++;
   if ( (SEND_NORMAL_E == unackQueue[channel].channelQueue[0].SendBehaviour) ||
        (SEND_TWO_E == unackQueue[channel].channelQueue[0].SendBehaviour) ||
        (SEND_THREE_E == unackQueue[channel].channelQueue[0].SendBehaviour) )
   {
      if (unackQueue[channel].channelQueue[0].NumberToSend >= unackQueue[channel].BackoffExp)
      {
         /* we have not exceeded the max number of resends */
         unackQueue[channel].BackoffCounter = MC_ACK_GetRandomDelay(unackQueue[channel].BackoffExp);
         unackQueue[channel].BackoffCounter++;
         
         /* Call MC_ACK_RerouteMessage to change the MAC Destination to the other parent if required*/
         if (1 < unackQueue[channel].BackoffExp) /* don't reroute if this is the first time the message is sent */
         {
            /* reroute the message at the front of the queue (index 0) */
            if (MC_ACK_RerouteMessage(channel, 0))
            {
               CO_PRINT_B_0(DBG_INFO_E, "Message re-routed to other parent.\r\n");
            }
            else
            {
               if ( unackQueue[channel].channelQueue[0].RouteDiversity )
               {
                  CO_PRINT_B_0(DBG_INFO_E, "Unable to re-route message to other parent.\r\n");
               }
            }
         }
         result = true;
      }
      else
      {
         /* we have exceeded the max value for BackoffExp without receiving an ACK for the message.*/
         DiscardCurrentMessage = true;
      }
   }
   else if ( SEND_ONCE_E == unackQueue[channel].channelQueue[0].SendBehaviour )
   {
      /* don't discard if this is the first time the message is sent */
//      unackQueue[channelID].BackoffCounter = MC_ACK_GetRandomDelay(3);//Select one of the next 8 (2^3) SRACH slots.
//      unackQueue[channelID].BackoffCounter++;
      unackQueue[channel].BackoffCounter = 1;
      if (1 < unackQueue[channel].BackoffExp) 
      {
         //Discard after first send attempt
         DiscardCurrentMessage = true;
      }
      else 
      {
         result = true;
         unackQueue[channel].ReadyToSend = true;
      }
   }
   else
   {
      // case for SEND_SACH_ONLY_E.  Push the message back onto the SACH queue for resending later, then discard from the front of the queue.
      // TODO: limit number of resends.
      if (unackQueue[channel].channelQueue[0].NumberToSend >= unackQueue[channel].BackoffExp)
      {
         /* we have not exceeded the max number of resends */
         unackQueue[channel].BackoffCounter = MC_ACK_GetRandomDelay(unackQueue[channel].BackoffExp);
         unackQueue[channel].BackoffCounter++;
         
         /* Call MC_ACK_RerouteMessage to change the MAC Destination to the other parent if required*/
         if (1 < unackQueue[channel].BackoffExp) /* don't reroute if this is the first time the message is sent */
         {
            /* reroute the message at the front of the queue (index 0) */
            if (MC_ACK_RerouteMessage(channel, 0))
            {
               CO_PRINT_B_0(DBG_INFO_E, "Message re-routed to other parent.\r\n");
            }
            else
            {
               if ( unackQueue[channel].channelQueue[0].RouteDiversity )
               {
                  CO_PRINT_B_0(DBG_INFO_E, "Unable to re-route message to other parent.\r\n");
               }
            }
         }
         result = true;
      }
      else
      {
         // Reschedule on SACH
         uint16_t currentDestination = MC_ACK_GetMacDestinationOfCurrentMessage( channel );
         if ( MC_SACH_ScheduleMessage(currentDestination, unackQueue[channel].channelQueue[0].MessageType, unackQueue[channel].channelQueue[0].SubType, &unackQueue[channel].channelQueue[0].Message, unackQueue[channel].channelQueue[0].SendBehaviour) )
         {
            CO_PRINT_B_2(DBG_INFO_E, "Rescheduled unacknowledged message on SACH. Type=%d, handle=%d\n\r", MessageType, *pMsghandle);
         }
         DiscardCurrentMessage = true;
      }
   }
   
   if ( DiscardCurrentMessage )
   {
      /*Discard the message by generating an internal ACK*/
      gDiscardedMessageHandle = unackQueue[channel].channelQueue[0].MessageHandle;
      gDiscardedMessageType = unackQueue[channel].channelQueue[0].MessageType;
      gDiscardedMessageMacDestination = MC_ACK_GetMacDestinationOfCurrentMessage(channel);
      bMessageDiscarded = true;
      
//         if ( SEND_NORMAL_E == unackQueue[channelID].channelQueue[0].SendBehaviour )
//         {
         CO_PRINT_B_2(DBG_INFO_E, "DISCARDED unacknowledged message. Type=%d, handle=%d\n\r", gDiscardedMessageType, gDiscardedMessageHandle);
//         }
      MC_ACK_AcknowledgeMessage(channel, gDiscardedMessageMacDestination, false, &MessageType, pMsghandle);
   }
   
   
   return result;
}

/*****************************************************************************
*  Function:     MC_ACK_MessageWasDiscarded
*  Description:         Returns details of a message that was discarded by the last update.
*  Param pMessageHandle:[OUT]The message ID assigned by the application.
*  Param pMessageType:  [OUT]The type of the dicarded message.
*  Returns:             True if a message was discarded on the last update, false otherwise.
*  Notes:               none.
*****************************************************************************/
bool MC_ACK_MessageWasDiscarded(uint32_t* pMessageHandle, ApplicationLayerMessageType_t* pMessageType, uint16_t* pMacDestination)
{
   bool discarded = bMessageDiscarded;
   
   if ( bMessageDiscarded )
   {
      bMessageDiscarded = false;
      if ( NULL != pMessageHandle )
      {
         *pMessageHandle = gDiscardedMessageHandle;
      }
      if ( NULL != pMessageType )
      {
         *pMessageType = gDiscardedMessageType;
      }
      if ( NULL != pMacDestination )
      {
         *pMacDestination = gDiscardedMessageMacDestination;
      }
      gDiscardedMessageHandle = 0;
      gDiscardedMessageType = APP_MSG_TYPE_UNKNOWN_E;
      gDiscardedMessageMacDestination = CO_BAD_NODE_ID;
   }
   
   return discarded;
}


/*****************************************************************************
*  Function:     MC_ACK_RerouteMessage
*  Description:         Updates the message destination to the alternate parent.
*  Param channelID:     ACK_RACHP_E or ACK_RACHS_E.
*  Param queue_index:   The queue index of the message to be rerouted.
*  Returns:             True if the message was rerouted, false otherwise.
*  Notes:               The message is pre-packed so this function needs to manipulate
*                       the MACDestination bit field to change the target address.
*****************************************************************************/
bool MC_ACK_RerouteMessage( const AcknowledgedQueueID_t channelID, const uint8_t queue_index )
{
   bool msg_rerouted = false;
   AcknowledgedQueueID_t channel = channelID;
   
   if ( gServicingDulchChannel && (ACK_RACHS_E == channelID) )
   {
      channel = ACK_DULCH_E;
   }
   
   /* don't re-route unless the message is marked for route diversity*/
   if ( unackQueue[channel].channelQueue[queue_index].RouteDiversity )
   {
      /* The MACDestination is in bits 4 to 15 of the packed message */
      uint16_t currentDestination = MC_ACK_GetMacDestinationOfQueuedMessage( channel, queue_index);
      
      /* it is addressed to a parent.  Now get the other parent nodeID */
      uint16_t nextDestination = MC_SMGR_GetNextParentForUplink();
      if ( nextDestination == currentDestination )
      {
         /* it's the other parent that we need */
         nextDestination = MC_SMGR_GetNextParentForUplink();
      }
      
      /* if all has gone well, we should have different node IDs for currentDestination and nextDestination */
      if ( nextDestination != currentDestination &&  CO_BAD_NODE_ID != nextDestination )
      {
         uint8_t* pMessage = unackQueue[channel].channelQueue[queue_index].Message.Payload.PhyDataInd.Data;
         if ( pMessage )
         {
            /* overwrite the destination address in the message */
            *pMessage &= 0xf0;
            *(pMessage + 1) = 0x00;
            *pMessage |= (nextDestination >> 8);
            *(pMessage+1) = (uint8_t)(nextDestination & 0xff);
            msg_rerouted = true;
            
            CO_PRINT_B_2(DBG_INFO_E, "Redirected RACH message from node %d to node %d\n\r", currentDestination, nextDestination);
         } 
      }
   }
   
   return msg_rerouted;
}

/*****************************************************************************
*  Function:     MC_ACK_GetMacDestinationOfCurrentMessage
*  Description:         Returns the MAC Destination address of the message at the
*                       Front of the specified channel queue.
*  Param channelID:     ACK_RACHP_E or ACK_RACHS_E.
*  Returns:             The MAC Destination, or CO_BAD_NODE_ID if the queue is empty.
*  Notes:               Calls MC_ACK_GetMacDestinationOfQueuedMessage for queue index 0.
*****************************************************************************/
uint16_t MC_ACK_GetMacDestinationOfCurrentMessage( const AcknowledgedQueueID_t channelID )
{
   return MC_ACK_GetMacDestinationOfQueuedMessage( channelID, 0 );
}

/*****************************************************************************
*  Function:     MC_ACK_GetTypeOfCurrentMessage
*  Description:         Returns the message type of the message at the
*                       Front of the specified channel queue.
*  Param channelID:     ACK_RACHP_E, ACK_RACHS_E ot ACK_DULCH_E.
*  Returns:             The message type, or APP_MSG_TYPE_MAX_E if the queue is empty.
*  Notes:               
*****************************************************************************/
ApplicationLayerMessageType_t MC_ACK_GetTypeOfCurrentMessage( const AcknowledgedQueueID_t channelID )
{
   ApplicationLayerMessageType_t msg_type = APP_MSG_TYPE_MAX_E;
   
   /* make sure the channelID is in range */
   if ( ACK_NUMBER_OF_ACKNOWLEDGEMENT_QUEUES > channelID )
   {
      /* Check that there is a message pending in the specified slot*/
      if ( 0 < unackQueue[channelID].MessageCount )
      {
         //get the message type
         msg_type = unackQueue[channelID].channelQueue[0].MessageType;
      }
   }

   return msg_type;
}


/*****************************************************************************
*  Function:     MC_ACK_GetMacDestinationOfQueuedMessage
*  Description:         Returns the MAC Destination address of the message at the
*                       specified index of the specified channel queue.
*  Param channelID:     ACK_RACHP_E or ACK_RACHS_E.
*  Param queue_index:   The queue index of the message.
*  Returns:             The MAC Destination, or CO_BAD_NODE_ID if the queue index is empty.
*  Notes:               The message is pre-packed so this function needs to manipulate
*                       the MACDestination bit field to change the target address.
*****************************************************************************/
uint16_t MC_ACK_GetMacDestinationOfQueuedMessage( const AcknowledgedQueueID_t channelID, const uint8_t queue_index )
{
   uint16_t currentDestination = CO_BAD_NODE_ID;
   AcknowledgedQueueID_t channel = channelID;
   
   if ( gServicingDulchChannel && (ACK_RACHS_E == channelID) )
   {
      channel = ACK_DULCH_E;
   }
   
   /* make sure the channelID is in range */
   if ( ACK_NUMBER_OF_ACKNOWLEDGEMENT_QUEUES > channel )
   {
      /* Check that there is a message pending in the specified slot*/
      if ( queue_index < unackQueue[channel].MessageCount )
      {
         /* The MACDestination is in bits 4 to 15 of the packed message */
         /* Get the first 16 bits */
         uint8_t* pMessage = unackQueue[channel].channelQueue[queue_index].Message.Payload.PhyDataInd.Data;
         if ( pMessage )
         {
            currentDestination = (uint16_t)(*pMessage) << 8;   //Hi byte of address
            currentDestination |= *(pMessage + 1);             //Lo byte of address
            /* mask out the FRAME TYPE (4 msb) */
            currentDestination &= 0x0fff;
         }
      }
   }
   
   return currentDestination;
}

/*****************************************************************************
*  Function:            MC_ACK_RedirectMessagesForNode
*  Description:         Redirects messages in all queues that are addressed to the
*                       supplied node ID.
*  Param node_id:       The node to redirect messages away from.
*  Returns:             None.
*  Notes:               We only send ACK'd messages to the parent nodes so we 
*                       try to redirect the message to the other parent.
*                       If the message's RouteDiversity is set to false the redirect
*                       will fail.  This happens if the parent node is the final
*                       destination of the message.  In that case we need to discard
*                       the message.
*                       
*****************************************************************************/
void MC_ACK_RedirectMessagesForNode(const uint16_t node_id)
{
   int8_t index;
   AcknowledgedQueueID_t channel = ACK_RACHP_E;
   /* Iterate through the queues and check each message.  Start at the last entry
      to simplify removing messages that can't be redirected */
   
   while ( channel < ACK_MAX_QUEUE_ID_E )
   {
      if ( 0 < unackQueue[channel].MessageCount )
      {
         index = unackQueue[channel].MessageCount - 1;
         do
         {
            /* check that the message is for the specified node */
            if ( node_id == MC_ACK_GetMacDestinationOfQueuedMessage( channel, index ) )
            {
               /* try to reroute the message */
               if ( false == MC_ACK_RerouteMessage( channel, index ) )
               {
                  /* The message cannot be rerouted.  Discard it from the queue */
                  /* This is done by copying the waiting messages one place forward in the queue. */
                  uint32_t queueIndex = index;
                  while ( queueIndex < unackQueue[channel].MessageCount &&
                           queueIndex < (ACK_MAX_UNACKED_MESSAGES - 1))/* ensures that we don't exceed the array bounds */
                  {
                     memcpy(&unackQueue[channel].channelQueue[queueIndex],
                               &unackQueue[channel].channelQueue[queueIndex+1],
                               sizeof(mbQueueEntry_t));
                     
                     queueIndex++;
                  }
                  unackQueue[channel].MessageCount--;
                  
                  CO_PRINT_B_1(DBG_INFO_E, "Discarded RACHP message to node %d\n\r", node_id);
                  
                  /* The messages have all been copied forward one place in the queue.
                     Clear the slot after the last message*/
                  memset(&unackQueue[channel].channelQueue[queueIndex], 0, sizeof(mbQueueEntry_t) );
                  
                  /* if we deleted the message at the front of the queue we need to reset the backoff parameters */
                  if ( 0 == index )
                  {
                     unackQueue[channel].BackoffExp = 0;
                     unackQueue[channel].BackoffCounter = 0;
                  }
               }
            }
            
            index--;
         } while ( index >= 0);
      }
      
      channel++;
   }
}

/*****************************************************************************
*  Function:     MC_ACK_CheckForDuplicateMessage
*  Description:            Checks message queu for a message that is the same as the parameters.
*                          If a match is found it is overwritten by the newer version.
*                          The match is made on source, desitination, hop count and message type.
*  param channel:          ACK_RACHP, ACK_RACHS etc.
*  param MsgType:          The message type enum from ApplicationLayerMessageType_t.
*  param MsgHandle:        The message ID that was generated by the application (Handle).
*  param pNewMessage:      Pointer to the message that we expect an ACK for.
*
*  Returns:                true if a matching record was overwritten, false otherwise
*  Notes:                  none
*****************************************************************************/
bool MC_ACK_CheckForDuplicateMessage(const AcknowledgedQueueID_t channelID, const ApplicationLayerMessageType_t MsgType, const uint32_t SubType,
                                       const uint32_t MsgHandle, const CO_Message_t* pNewMessage)
{
   bool duplicate_found = false;
   
#ifdef DUPLICATE_MESSAGE_REPLACEMENT_ENABLED
   uint32_t message_index = 0;
   AcknowledgedQueueID_t channel = channelID;
   
   if ( gServicingDulchChannel && (ACK_RACHS_E == channelID) )
   {
      channel = ACK_DULCH_E;
   }
   /* Don't overwrite previous status indication messages.  The payloads are used to build an image of the mesh for the CIE */
   if ( APP_MSG_TYPE_STATUS_INDICATION_E != MsgType )
   {
      if ( ACK_MAX_QUEUE_ID_E > channel && pNewMessage)
      {
         uint8_t* pNewMsgData = (uint8_t*)pNewMessage->Payload.PhyDataReq.Data;
         /* Check for space in the queue */
         while ( !duplicate_found && unackQueue[channel].MessageCount > message_index )
         {
            /*check that they are the same Msg type */
            if ( unackQueue[channel].channelQueue[message_index].MessageType == MsgType )
            {
               /* check that they are the same sub type i.e. not different kind of fault or status msg */
               if ( unackQueue[channel].channelQueue[message_index].SubType == SubType )
               {
                  /* compare the first 9 bytes of the message (mac-dest, mac-source, hop, dest, source and top 4 bits of msg type) */
                  uint8_t* pQueueMsgData = (uint8_t*)unackQueue[channel].channelQueue[message_index].Message.Payload.PhyDataReq.Data;
                  uint32_t msg_delta = memcmp(pQueueMsgData, pNewMsgData, 9);
                  if ( 0 == msg_delta )
                  {
                     //check that the msg hasn't already been sent and is witing for an ACK.
                     if ( false == unackQueue[channel].WaitingForACK )
                     {
                        /* we have found a duplicate message waiting in the queue. */ 
                        
                        /* call the duplicate message callback to notify the system that we have discarded a message handle */
                        if ( pfDuplicateMessageCallback )
                        {
                           pfDuplicateMessageCallback(unackQueue[channel].channelQueue[message_index].MessageHandle, MsgHandle, MsgType);
                        }
                        
                        /*Overwrite the queue with the new message */
                        memcpy((uint8_t*)&unackQueue[channel].channelQueue[message_index].Message, (uint8_t*)pNewMessage, sizeof(CO_Message_t));
                        unackQueue[channel].channelQueue[message_index].MessageHandle = MsgHandle;
                        /* restart the backoff mechanism if this msg is at the front of the queue */
                        if ( 0 == message_index )
                        {
                           unackQueue[channel].BackoffExp = 0;
                           unackQueue[channel].BackoffCounter = 0;
                        }
                        duplicate_found = true;
                        CO_PRINT_B_1(DBG_INFO_E,"AckMgr overwritten older duplicate type %d\r\n", MsgType);
                     }
                     else 
                     {
                        CO_PRINT_B_1(DBG_INFO_E,"AckMgr rejected duplicate type %d.  Original already sent\r\n", MsgType);
                     }
                  }
               }
            }
            message_index++;
         }
      }
   }
#endif
   return duplicate_found;
}

/*****************************************************************************
*  Function:     MC_ACK_QueueFull
*  Description:            Checks whether a queue is full.
*
*  param channel:          ACK_RACHP, ACK_RACHS etc.
*
*  Returns:                true if the queue is full
*  Notes:                  none
*****************************************************************************/
bool MC_ACK_QueueFull(const AcknowledgedQueueID_t channel)
{
   bool queue_full = true;
   
   if ( channel < ACK_MAX_QUEUE_ID_E )
   {
      if ( ACK_MAX_UNACKED_MESSAGES > unackQueue[channel].MessageCount )
      {
         queue_full = false;
      }
   }
   return queue_full;
}

/*****************************************************************************
*  Function:     MC_ACK_GetQueueCount
*  Description:            returns how many messages are in the requested queue.
*
*  param channel:          ACK_RACHP, ACK_RACHS etc.
*
*  Returns:                true if the queue is full
*  Notes:                  none
*****************************************************************************/
uint32_t MC_ACK_GetQueueCount(const AcknowledgedQueueID_t channel)
{
   uint32_t count = 0;
   if ( channel < ACK_MAX_QUEUE_ID_E )
   {
      count = unackQueue[channel].MessageCount;
   }
   return count;
}
