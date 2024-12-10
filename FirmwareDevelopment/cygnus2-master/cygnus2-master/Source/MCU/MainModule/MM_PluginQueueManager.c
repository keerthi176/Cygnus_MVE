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
*  File         : MM_PluginQueueManager.c
*
*  Description  : Manages the queue for Plugin mesages waiting to be sent.
*
*************************************************************************************/

#ifdef USE_NEW_HEAD_INTERFACE

/* System Include Files
*************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "CO_Defines.h"
#include "CO_Message.h"
#include "MM_PluginQueueManager.h"

/* Private Functions Prototypes
*************************************************************************************/



/* Private Types
*************************************************************************************/


/* Global Variables
*************************************************************************************/

/* Private Variables
*************************************************************************************/

osPoolDef(PluginMsgPool, PLUGIN_POOL_SIZE, PluginMessage_t);
osPoolId PluginMsgPool;

osMessageQDef(PluginMsgQ, PLUGIN_Q_SIZE, &PluginMessage_t);
osMessageQId(PluginMsgQ);

static int32_t MessageCount; /* Number of messages in the queue */
static int32_t LostMessageCount; /* Number of lost messages (due to lack of pool/queue space)*/

static PluginMessage_t CurrentMessage;

/* Public Functions
*************************************************************************************/

/*************************************************************************************/
/**
* MM_PLUGINQ_Initialise
* Function to clear the CIE queues.
*
* @param - None.
*
* @return - ErrorCode_t  SUCCESS_E or error code.
*/
ErrorCode_t MM_PLUGINQ_Initialise(void)
{
   MessageCount = 0;
   LostMessageCount = 0;
   
   PluginMsgPool = osPoolCreate(osPool(PluginMsgPool));
   CO_ASSERT_RET_MSG(NULL != PluginMsgPool, ERR_FAILED_TO_CREATE_MEMPOOL_E, "ERROR - Failed to create Plugin MemPool");
   PluginMsgQ = osMessageCreate(osMessageQ(PluginMsgQ), NULL);
   CO_ASSERT_RET_MSG(NULL != PluginMsgQ, ERR_FAILED_TO_CREATE_QUEUE_E, "ERROR - Failed to create Plugin Queue");
   
   MM_PLUGINQ_ResetMessageQueue();
   
   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MM_PLUGINQ_Push
* Function to add a new message to one of the Plugin queue.
*
* @param - pMessage  The message to be added
*
* @return - error code,
*/
ErrorCode_t MM_PLUGINQ_Push(const PluginMessage_t* pMessage)
{
   ErrorCode_t error = ERR_INVALID_POINTER_E;
   osStatus osStat = osErrorOS;
   PluginMessage_t* pMsg;
   
   if ( pMessage )
   {
      pMsg = osPoolAlloc(PluginMsgPool);
      if ( pMsg )
      {
         memcpy(pMsg, pMessage, sizeof(PluginMessage_t));
         
         osStat = osMessagePut(PluginMsgQ, (uint32_t)pMsg, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(PluginMsgPool, pMsg);
            LostMessageCount++;
            error = ERR_QUEUE_OVERFLOW_E;
         }
         else
         {
            MessageCount++;
            error = SUCCESS_E;
         }
      }
      else 
      {
         error = ERR_NO_MEMORY_E;
      }
   }
   return error;
}

/*************************************************************************************/
/**
* MM_PLUGINQ_Pop
* Function to get the first message from one of the CIE queues.
*
* @param    pMessage [OUT]  Pointer to buffer that accepts the popped message.
*
* @return   SUCCESS_E if a message was returned.
*/
ErrorCode_t MM_PLUGINQ_Pop(const PluginMessage_t* pMessage)
{
   osEvent event;
   PluginMessage_t *pMsg = NULL;
   ErrorCode_t error = ERR_INVALID_POINTER_E;
   
   if ( pMessage )
   {
      if ( HEAD_CMD_EMPTY_E == CurrentMessage.PluginMessageType )
      {
         /* the current message is empty so pop one out of the queue */  
         error = ERR_NOT_FOUND_E;/*assume failure*/
         event = osMessageGet(PluginMsgQ, 1);
         if (osEventMessage == event.status)
         {
            pMsg = (PluginMessage_t *)event.value.p;
            if (pMsg)
            {
               memset((void*)pMessage,0,sizeof(PluginMessage_t));
               uint8_t* pOutMsg = (uint8_t*)&CurrentMessage;
               memcpy(pOutMsg, pMsg, sizeof(PluginMessage_t));
               pOutMsg = (uint8_t*)pMessage;
               memcpy(pOutMsg, pMsg, sizeof(PluginMessage_t));
               osPoolFree(PluginMsgPool, pMsg);
               error = SUCCESS_E;
            }
            else 
            {
               error = ERR_NOT_FOUND_E;
            }
         }
         else 
         {
            error = ERR_NOT_FOUND_E;
         }
      }
      else 
      {
         /* the current message is populated so use that */
         uint8_t* pOutMsg = (uint8_t*)pMessage;
         memcpy(pOutMsg, &CurrentMessage, sizeof(PluginMessage_t));
         error = SUCCESS_E;
      }
   }

   return error;
}

/*************************************************************************************/
/**
* MM_PLUGINQ_Discard
* Function to remove the first message from one of the plug-in queue.
*
* @param    None.
*
* @return   None
*/
ErrorCode_t MM_PLUGINQ_Discard(void)
{
   osEvent event;
   ErrorCode_t error = ERR_NOT_FOUND_E;
   PluginMessage_t* pMessage = NULL;
   
   if ( HEAD_CMD_EMPTY_E != CurrentMessage.PluginMessageType )
   {
      CurrentMessage.PluginMessageType = HEAD_CMD_EMPTY_E;
      MessageCount--;
      error = SUCCESS_E;
   }
   else 
   {
      /* the current message is empty so pop one out of the queue */
      error = ERR_NOT_FOUND_E;/*assume failure*/
      event = osMessageGet(PluginMsgQ, 1);
      if (osEventMessage == event.status)
      {
         pMessage = (PluginMessage_t*)event.value.p;
         if(pMessage != NULL)
         {
            osPoolFree(PluginMsgPool, pMessage);
            MessageCount--;
            error = SUCCESS_E;
         }
      }
   }
   
   return error;
}

/*************************************************************************************/
/**
* MM_PLUGINQ_MessageCount
* Function to return the number of messages in the queue.
*
* @param    None.
*
* @return   The message count.
*/
uint32_t MM_PLUGINQ_MessageCount(void)
{
   return MessageCount;
}

/*************************************************************************************/
/**
* MM_PLUGINQ_LostMessageCount
* Function to return the number of lost messages.
*
* @param    None.
*
* @return   The lost message count.
*/
uint32_t MM_PLUGINQ_LostMessageCount(void)
{
   return LostMessageCount;
}

/*************************************************************************************/
/**
* MM_PLUGINQ_ResetLostMessageCount
* Function to reset the lost message count.
*
* @param    None.
*
* @return   None.
*/
void MM_PLUGINQ_ResetLostMessageCount(void)
{
   LostMessageCount = 0;
}

/*************************************************************************************/
/**
* MM_PLUGINQ_ResetMessageQueue
* Function to flush all messages from one of the CIE queues.
*
* @param    None.
*
* @return   None.
*/
void MM_PLUGINQ_ResetMessageQueue(void)
{
   osEvent event;

   do
   {
      event = osMessageGet(PluginMsgQ, 1);
      if (osEventMessage == event.status)
      {
         PluginMessage_t* pMsg = (PluginMessage_t *)event.value.p;
         if (pMsg)
         {
            osPoolFree(PluginMsgPool, pMsg);
         }
      }

   } while ( osEventMessage == event.status );
   
   MessageCount = 0;
   LostMessageCount = 0;
   CurrentMessage.PluginMessageType = HEAD_CMD_EMPTY_E;
}

#endif //USE_NEW_HEAD_INTERFACE
