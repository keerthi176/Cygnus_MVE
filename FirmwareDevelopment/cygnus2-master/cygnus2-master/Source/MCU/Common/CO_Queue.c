/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2018 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : CO_Queue.c.h
*
*  Description  : Non-os queue (doesn't block threads) that allows random access
*                 to queue entries
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "CO_Message.h"
#include "MM_ApplicationCommon.h"
#include "CO_Queue.h"

/* Private Definitions
*************************************************************************************/
#define CO_Q_SIZE 10


/* Private Types
*************************************************************************************/


/* Global Variables
*************************************************************************************/
static CO_Queue_t gQueue[CO_Q_MAX_QUEUES_E];

/* Private Variables
*************************************************************************************/

/* Private Functions Prototypes
*************************************************************************************/
static ErrorCode_t CO_RemoveDuplicateMessages(const CO_QueueId_t queueId, const CieBuffer_t* pMessage);
static bool CO_IsSupersededMessage( const CO_QueueId_t queueId, const uint32_t index, const CieBuffer_t* pMessage );
static ErrorCode_t CO_QueueDiscardEntry(const CO_QueueId_t queueId, const uint32_t index);
static bool CO_IsSupersededCommandMessage(const CO_QueueId_t queueId, const uint32_t index, const CieBuffer_t* pMessage);
static bool CO_IsSupersededOutputSignal(const CO_QueueId_t queueId, const uint32_t index, const CieBuffer_t* pMessage);
static bool CO_IsSupersededAlarmOutputStateSignal(const CO_QueueId_t queueId, const uint32_t index, const CieBuffer_t* pMessage);


/* Public Functions
*************************************************************************************/

/*************************************************************************************/
/**
* CO_QueueInitialise
* Function to initialse the queues.
*
* @param - None.
*
* @return - SUCCESS_E or error code.
*/
ErrorCode_t CO_QueueInitialise(void)
{
   uint32_t queue_id;
   
   for ( queue_id = 0; queue_id < CO_Q_MAX_QUEUES_E; queue_id++)
   {
      CO_QueueReset((CO_QueueId_t)queue_id);
   }
   return SUCCESS_E;
}

/*************************************************************************************/
/**
* CO_QueueSetLock
* Function to apply a lock to preserve the contents of one of the queues.
* The lock only applies to removing items from the queue.  New messages may still be added.
* There are three lock settings:
* CO_Q_LOCK_UNLOCKED_E        No lock.  any/all messages can be deleted
* CO_Q_LOCK_FIRST_ENTRY_E     Lock message at front of queue (set when waiting for Tx confirmation)
* CO_Q_LOCK_ALL_E             Protect all contents from deletion.
*
* @param - queueId            The ID of the queue to lock.
* @param - lock               The type of lock to be applied
*
* @return - error code
*/
ErrorCode_t CO_QueueSetLock(const CO_QueueId_t queueId, const QueueLock_t lock)
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   
   if ( (CO_Q_MAX_QUEUES_E > queueId) && (CO_Q_LOCK_MAX_E > lock) )
   {
      gQueue[queueId].Lock = lock;
      CO_PRINT_B_2(DBG_INFO_E,"CO_QueueSetLock queue=%d, lock=%d\r\n", queueId, lock);
      result = SUCCESS_E;
   }
   
   return result;
}

/*************************************************************************************/
/**
* CO_QueuePush
* Function to add a new message to one of the MAC queues.
*
* @param - queueId            The ID of the queue to add to.
* @param - pMessage           The message to be added
* @param - removeDuplicate    True if the queue is to be searched for superceeded messages
*                             to be removed
*
* @return - error code
*/
ErrorCode_t CO_QueuePush(const CO_QueueId_t queueId, const CieBuffer_t* pMessage, const bool removeDuplicate)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pMessage )
   {
      result = ERR_OUT_OF_RANGE_E;
      if ( CO_Q_MAX_QUEUES_E > queueId )
      {
         result = SUCCESS_E;
         if ( removeDuplicate )
         {
            //Remove queued messages that are superceeded by this message
            result = CO_RemoveDuplicateMessages( queueId, pMessage );
         }
         
         if ( (SUCCESS_E == result) || (ERR_NOT_FOUND_E == result) )
         {
            result = ERR_QUEUE_OVERFLOW_E;
            if ( CO_Q_SIZE > gQueue[queueId].MessageCount )
            {
               gQueue[queueId].Entry[gQueue[queueId].MessageCount].MessageType = (ApplicationLayerMessageType_t)pMessage->MessageType;
               gQueue[queueId].Entry[gQueue[queueId].MessageCount].MessageHandle = GetNextHandle();
               memcpy(&gQueue[queueId].Entry[gQueue[queueId].MessageCount].Message, pMessage, sizeof(CieBuffer_t) );
               gQueue[queueId].MessageCount++;
               result = SUCCESS_E;
               CO_PRINT_B_2(DBG_INFO_E,"CO_Queue+ %d, count=%d\r\n", queueId, gQueue[queueId].MessageCount);
            }
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* CO_QueuePeek
* Function to get the first message from the specified queue, without removing the message.
*
* @param - queueId          The ID of the queue to add to.
* @param - pMessage [OUT]   Pointer to buffer that accepts the popped message.
*
* @return - void
*/
ErrorCode_t CO_QueuePeek(const CO_QueueId_t queueId, CieBuffer_t* pMessage)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pMessage )
   {
      result = ERR_OUT_OF_RANGE_E;
      if ( CO_Q_MAX_QUEUES_E > queueId )
      {
         if ( gQueue[queueId].MessageCount > 0 )
         {
            memcpy(pMessage, &gQueue[queueId].Entry[0].Message, sizeof(CieBuffer_t) );
            result = SUCCESS_E;
         }
         else 
         {
            result = ERR_NOT_FOUND_E;
         }
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* MC_MACQ_Discard
* Function to remove the first message from the specified queue.
*
* @param - queue     The ID of the queue to remove from.
*
* @return - void
*/
ErrorCode_t CO_QueueDiscard(const CO_QueueId_t queueId)
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;

   if ( CO_Q_MAX_QUEUES_E > queueId )
   {
      result = CO_QueueDiscardEntry(queueId, 0);
   }
   return result;
}

/*************************************************************************************/
/**
* CO_QueueMessageCount
* Function to return the number of messages in the specified queue.
*
* @param - queueId           The ID of the queue.
*
* @return - The message count or 0 if the queueId type is unknown.
*/
uint32_t CO_QueueMessageCount(const CO_QueueId_t queueId)
{
   uint32_t msg_count = 0;
   
   if ( CO_Q_MAX_QUEUES_E > queueId )
   {
      msg_count = gQueue[queueId].MessageCount;
   }
   
   return msg_count;
}


/*************************************************************************************/
/**
* CO_QueueReset
* Function to flush all messages from one of the queues.
*
* @param - queue  The ID of the queue.
*
* @return - SUCCESS_E if the queue was flushed, ERR_NOT_FOUND_E otherwise.
*/
ErrorCode_t CO_QueueReset(const CO_QueueId_t queueId)
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   uint32_t queue_index;
   
   if ( CO_Q_MAX_QUEUES_E > queueId )
   {
      gQueue[queueId].MessageCount = 0;
      gQueue[queueId].Lock = CO_Q_LOCK_UNLOCKED_E;
      for ( queue_index = 0; queue_index < CO_Q_SIZE; queue_index++)
      {
         gQueue[queueId].Entry[queue_index].MessageHandle = 0;
         gQueue[queueId].Entry[queue_index].MessageType = APP_MSG_TYPE_MAX_E;
         memset(&gQueue[queueId].Entry[queue_index].Message, 0, sizeof(CieBuffer_t));
      }
      result = SUCCESS_E;
      CO_PRINT_B_1(DBG_INFO_E,"NCU downlink queue %d reset\r\n", queueId);
   }
   return result;
}

/*************************************************************************************/
/**
* CO_QueueDiscardEntry
* Function to discard a queue entry by index.
*
* @param - queue  The ID of the queue.
* @param - index  The index of the queue entry.
*
* @return - SUCCESS_E if the queue was flushed, ERR_NOT_FOUND_E otherwise.
*/
ErrorCode_t CO_QueueDiscardEntry(const CO_QueueId_t queueId, const uint32_t index)
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   uint32_t entries_to_move = 0;
   uint32_t first_entry_to_move = 0;
   uint32_t size_to_move = 0;
   
   CO_PRINT_B_2(DBG_INFO_E, "CO_QueueDiscardEntry q=%d i=%d\r\n", queueId, index);
   
   if ( CO_Q_MAX_QUEUES_E > queueId )
   {
      //Check that the queue isn't locked
      result = ERR_ACCESS_DENIED_E;
      if ( CO_Q_LOCK_ALL_E != gQueue[queueId].Lock )
      {
         //If index is 0, check for first entry lock
         if ( (0 < index) || 
              ((0 == index) && (CO_Q_LOCK_FIRST_ENTRY_E != gQueue[queueId].Lock)) )
         {
            result = SUCCESS_E;
         }
         
         if ( SUCCESS_E == result )
         {
            result = ERR_NOT_FOUND_E;
            if ( gQueue[queueId].MessageCount > index )
            {
               //To discard the queue entry, copy all higher entries forward one slot
               //and erase the last entry
               
               //calculate how much memory needs to be copied forward
               entries_to_move = CO_Q_SIZE - index - 1;
               first_entry_to_move = CO_Q_SIZE - entries_to_move;
               size_to_move = entries_to_move * sizeof(CO_QueueEntry_t);
               //Move the entires forward in the queue
               memcpy(&gQueue[queueId].Entry[index], &gQueue[queueId].Entry[first_entry_to_move], size_to_move);
               //Clear the last queue entry
               uint32_t last_entry = CO_Q_SIZE - 1;
               gQueue[queueId].Entry[last_entry].MessageType = APP_MSG_TYPE_MAX_E;
               gQueue[queueId].Entry[last_entry].MessageHandle = 0;
               memset(&gQueue[queueId].Entry[last_entry].Message, 0, sizeof(CieBuffer_t));
               gQueue[queueId].MessageCount--;
               result = SUCCESS_E;
               CO_PRINT_B_2(DBG_INFO_E,"CO_Queue- %d, count=%d\r\n", queueId, gQueue[queueId].MessageCount);
            }
            else 
            {
               CO_PRINT_B_2(DBG_INFO_E, "CO_QueueDiscardEntry - index out of range : count=%d i=%d\r\n", gQueue[queueId].MessageCount, index);
            }
         }
      }
   }
   
   return result;
}


/*************************************************************************************/
/**
* CO_RemoveDuplicateMessages
* Function to find and discard messages that are superceded by the supplied message.
*
* @param - queueId   The ID of the queue.
* @param - pMessage  The message to check against
*
* @return - error code.
*/
ErrorCode_t CO_RemoveDuplicateMessages(const CO_QueueId_t queueId, const CieBuffer_t* pMessage)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   uint32_t index = 0;
   
   if ( pMessage )
   {
      result = ERR_OUT_OF_RANGE_E;
      if ( CO_Q_MAX_QUEUES_E > queueId )
      {
         result = ERR_ACCESS_DENIED_E;
         if ( CO_Q_LOCK_ALL_E != gQueue[queueId].Lock )
         {
            result = ERR_NOT_FOUND_E;
            if ( gQueue[queueId].MessageCount > 0 )
            {
               index = gQueue[queueId].MessageCount;
               while ( 0 < index )
               {
                  index--;
                  if ( CO_IsSupersededMessage(queueId, index, pMessage ) )
                  {
                     result = CO_QueueDiscardEntry(queueId, index);
                     if ( SUCCESS_E != result )
                     {
                        CO_PRINT_B_3(DBG_INFO_E,"Failed to remove duplicate message.  Queue=%d, index=%d, error=%d\r\n", queueId, index, result);
                     }
                  }
               }
               result = SUCCESS_E;
            }
         }
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* CO_IsSupersededMessage
* Check if the mesage at 'index' is a duplicate for the supplied message
*
* @param - queueId   The ID of the queue.
* @param - queueId   The index of the message in the queue to check.
* @param - pMessage  The message to check against
*
* @return - error code.
*/
bool CO_IsSupersededMessage( const CO_QueueId_t queueId, const uint32_t index, const CieBuffer_t* pMessage )
{
   bool result = false;
   
   if ( pMessage )
   {
      if ( CO_Q_MAX_QUEUES_E > queueId )
      {
         if ( gQueue[queueId].MessageCount > index )
         {
            switch ( pMessage->MessageType )
            {
               case APP_MSG_TYPE_COMMAND_E:
                  result = CO_IsSupersededCommandMessage( queueId, index, pMessage );
                  break;
               case APP_MSG_TYPE_OUTPUT_SIGNAL_E:
                  result = CO_IsSupersededOutputSignal( queueId, index, pMessage );
                  break;
               case APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E:
                  result = CO_IsSupersededAlarmOutputStateSignal( queueId, index, pMessage );
                  break;
               default:
                  break;
            }
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* CO_IsSupersededCommandMessage
* Check if the message at 'index' is a duplicate for the supplied message
*
* @param - queueId   The ID of the queue.
* @param - queueId   The index of the message in the queue to check.
* @param - pMessage  The message to check against
*
* @return - error code.
*/
bool CO_IsSupersededCommandMessage( const CO_QueueId_t queueId, const uint32_t index, const CieBuffer_t* pMessage )
{
   bool result = false;
   CO_CommandData_t* pCommand;
   CO_CommandData_t* pQueuedCommand;
   
   if ( pMessage )
   {
      if ( CO_Q_MAX_QUEUES_E > queueId )
      {
         if ( gQueue[queueId].MessageCount > index )
         {
            if ( APP_MSG_TYPE_COMMAND_E == gQueue[queueId].Entry[index].Message.MessageType )
            {
               pCommand = (CO_CommandData_t*)pMessage->Buffer;
               pQueuedCommand = (CO_CommandData_t*)gQueue[queueId].Entry[index].Message.Buffer;
               if ( (pCommand->Destination == pQueuedCommand->Destination) &&
                  (pCommand->CommandType == pQueuedCommand->CommandType) &&
                  (pCommand->Parameter1 == pQueuedCommand->Parameter1)&&
                  (pCommand->Parameter2 == pQueuedCommand->Parameter2) )
               {
                  CO_PRINT_B_3(DBG_INFO_E,"Queue=%d, CMD Msg tid=%d is superceeded by Msg tid=%d\r\n", queueId, pQueuedCommand->TransactionID, pCommand->TransactionID);
                  result = true;
               }
            }
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* CO_IsSupersededOutputSignal
* Check if the message at 'index' is a duplicate for the supplied message
*
* @param - queueId   The ID of the queue.
* @param - queueId   The index of the message in the queue to check.
* @param - pMessage  The message to check against
*
* @return - error code.
*/
bool CO_IsSupersededOutputSignal( const CO_QueueId_t queueId, const uint32_t index, const CieBuffer_t* pMessage )
{
   bool result = false;
   CO_OutputData_t* pOutputSignal;
   CO_OutputData_t* pQueuedOutputSignal;
   
   if ( pMessage )
   {
      if ( CO_Q_MAX_QUEUES_E > queueId )
      {
         if ( gQueue[queueId].MessageCount > index )
         {
            if ( APP_MSG_TYPE_OUTPUT_SIGNAL_E == gQueue[queueId].Entry[index].Message.MessageType )
            {
               pOutputSignal = (CO_OutputData_t*)pMessage->Buffer;
               pQueuedOutputSignal = (CO_OutputData_t*)gQueue[queueId].Entry[index].Message.Buffer;
               if ( (pOutputSignal->Destination == pQueuedOutputSignal->Destination) &&
                  (pOutputSignal->OutputChannel == pQueuedOutputSignal->OutputChannel) &&
                  (pOutputSignal->OutputProfile == pQueuedOutputSignal->OutputProfile) )
               {
                  //For global address we also need to discriminate on the zone
                  if ( ADDRESS_GLOBAL == pOutputSignal->Destination )
                  {
                     //Global address.  Check that the zone matches.
                     if ( pOutputSignal->zone == pQueuedOutputSignal->zone )
                     {
                        CO_PRINT_B_3(DBG_INFO_E,"Queue=%d, OUT Msg Handle=%d is superseded by Msg Handle=%d\r\n", queueId, pQueuedOutputSignal->Handle, pOutputSignal->Handle);
                        CO_PRINT_B_5(DBG_INFO_E,"1 Handle=%d, Zone=%d, Addr=%d, chan=%d, prof=%d\r\n", pOutputSignal->Handle, pOutputSignal->zone, pOutputSignal->Destination, pOutputSignal->OutputChannel, pOutputSignal->OutputProfile);
                        CO_PRINT_B_5(DBG_INFO_E,"1 Handle=%d, Zone=%d, Addr=%d, chan=%d, prof=%d\r\n", pQueuedOutputSignal->Handle, pQueuedOutputSignal->zone, pQueuedOutputSignal->Destination, pQueuedOutputSignal->OutputChannel, pQueuedOutputSignal->OutputProfile);
                        result = true;
                     }
                  }
                  else 
                  {
                     //Not a global address
                     CO_PRINT_B_3(DBG_INFO_E,"Queue=%d, OUT Msg Handle=%d is superseded by Msg Handle=%d\r\n", queueId, pQueuedOutputSignal->Handle, pOutputSignal->Handle);
                     CO_PRINT_B_5(DBG_INFO_E,"2 Handle=%d, Zone=%d, Addr=%d, chan=%d, prof=%d\r\n", pOutputSignal->Handle, pOutputSignal->zone, pOutputSignal->Destination, pOutputSignal->OutputChannel, pOutputSignal->OutputProfile);
                     CO_PRINT_B_5(DBG_INFO_E,"2 Handle=%d, Zone=%d, Addr=%d, chan=%d, prof=%d\r\n", pQueuedOutputSignal->Handle, pQueuedOutputSignal->zone, pQueuedOutputSignal->Destination, pQueuedOutputSignal->OutputChannel, pQueuedOutputSignal->OutputProfile);
                     result = true;
                  }
               }
            }
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* CO_IsSupersededAlarmOutputStateSignal
* Check if the message at 'index' is a duplicate for the supplied message
*
* @param - queueId   The ID of the queue.
* @param - queueId   The index of the message in the queue to check.
* @param - pMessage  The message to check against
*
* @return - error code.
*/
bool CO_IsSupersededAlarmOutputStateSignal( const CO_QueueId_t queueId, const uint32_t index, const CieBuffer_t* pMessage )
{
   bool result = false;
   
   if ( pMessage )
   {
      if ( CO_Q_MAX_QUEUES_E > queueId )
      {
         if ( gQueue[queueId].MessageCount > index )
         {
            //We don't want to supeceede a message at the front of the queue if it is being sent i.e. waiting for confirmation.
            if ( APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E == gQueue[queueId].Entry[index].Message.MessageType )
            {
#ifdef _DEBUG
               CO_AlarmOutputStateData_t* pOutputSignal = (CO_AlarmOutputStateData_t*)pMessage->Buffer;
               CO_AlarmOutputStateData_t* pQueuedOutputSignal = (CO_AlarmOutputStateData_t*)gQueue[queueId].Entry[index].Message.Buffer;
               CO_PRINT_B_3(DBG_INFO_E,"Queue=%d, AOS Msg Handle=%d is superceeded by Msg Handle=%d\r\n", queueId, pQueuedOutputSignal->Handle, pOutputSignal->Handle);
#endif
               result = true;
            }
         }
      }
   }
   return result;
}


