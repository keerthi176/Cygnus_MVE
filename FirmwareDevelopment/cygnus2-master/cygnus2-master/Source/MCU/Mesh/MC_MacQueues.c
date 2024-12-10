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
*  File         : MC_MACQueues.c
*
*  Description  : Manages the transmit message queues for the MAC.
*
*************************************************************************************/


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
#include "MC_MacQueues.h"


#define MESSAGE_ACCESS_MAX_COUNT 10

/* Private Functions Prototypes
*************************************************************************************/

/* Private Types
*************************************************************************************/

MacQueueStatusRegister_t gMacQueueStatusRegister;

/* Global Variables
*************************************************************************************/
extern osPoolId MeshPool;


/* Private Variables
*************************************************************************************/
#define DCH_Q_SIZE 2
#define RACHP_Q_SIZE 10
#define RACHS_Q_SIZE 10
#define ACK_Q_SIZE 2
#define DLCCHP_Q_SIZE 30
#define DLCCHS_Q_SIZE 20

osMessageQDef(DCHQ, DCH_Q_SIZE, uint32_t);
osMessageQId(DCHQ);

osMessageQDef(RACHPQ, RACHP_Q_SIZE, uint32_t);  // Primary RACH queue, for fire signals
osMessageQId(RACHPQ);

osMessageQDef(RACHSQ, RACHS_Q_SIZE, uint32_t);  // Secondary RACH queue, for first aid and intruder detection signals
osMessageQId(RACHSQ);

osMessageQDef(ACKQ, ACK_Q_SIZE, uint32_t);
osMessageQId(ACKQ);

osMessageQDef(DLCCHPQ, DLCCHP_Q_SIZE, uint32_t);  // Primary DLCCH queue, for fire signals
osMessageQId(DLCCHPQ);

osMessageQDef(DLCCHSQ, DLCCHS_Q_SIZE, uint32_t);  // Secondary DLCCH queue, for anything else
osMessageQId(DLCCHSQ);

static CO_Message_t current_dch_message;
static CO_Message_t current_dlcchp_message;
static CO_Message_t current_dlcchs_message;
static CO_Message_t current_rachp_message;
static CO_Message_t current_rachs_message;
static CO_Message_t current_ack_message;

/* Public Functions
*************************************************************************************/

/*************************************************************************************/
/**
* MC_MACQ_Initialise
* Function to initialse the MAC queues.
*
* @param - None.
*
* @return - SUCCESS_E or error code.
*/
ErrorCode_t MC_MACQ_Initialise(void)
{
   gMacQueueStatusRegister.AckQCount = 0;
   gMacQueueStatusRegister.DchQCount = 0;
   gMacQueueStatusRegister.DlcchPQCount = 0;
   gMacQueueStatusRegister.DlcchSQCount = 0;
   gMacQueueStatusRegister.RachPQCount = 0;
   gMacQueueStatusRegister.RachSQCount = 0;
   
   DCHQ = osMessageCreate(osMessageQ(DCHQ), NULL);
   CO_ASSERT_RET_MSG(NULL != DCHQ, ERR_FAILED_TO_CREATE_QUEUE_E, "ERROR - Failed to create DCH Queue");
   RACHPQ = osMessageCreate(osMessageQ(RACHPQ), NULL);
   CO_ASSERT_RET_MSG(NULL != RACHPQ, ERR_FAILED_TO_CREATE_QUEUE_E, "ERROR - Failed to create RACHP Queue");
   RACHSQ = osMessageCreate(osMessageQ(RACHSQ), NULL);
   CO_ASSERT_RET_MSG(NULL != RACHSQ, ERR_FAILED_TO_CREATE_QUEUE_E, "ERROR - Failed to create RACHS Queue");
   ACKQ = osMessageCreate(osMessageQ(ACKQ), NULL);
   CO_ASSERT_RET_MSG(NULL != ACKQ, ERR_FAILED_TO_CREATE_QUEUE_E, "ERROR - Failed to create ACK Queue");
   DLCCHPQ = osMessageCreate(osMessageQ(DLCCHPQ), NULL);
   CO_ASSERT_RET_MSG(NULL != DLCCHPQ, ERR_FAILED_TO_CREATE_QUEUE_E, "ERROR - Failed to create DLCCHP Queue");
   DLCCHSQ = osMessageCreate(osMessageQ(DLCCHSQ), NULL);
   CO_ASSERT_RET_MSG(NULL != DLCCHSQ, ERR_FAILED_TO_CREATE_QUEUE_E, "ERROR - Failed to create DLCCHS Queue");

   MC_MACQ_ResetMessageQueue(MAC_DCH_Q_E);
   MC_MACQ_ResetMessageQueue(MAC_DLCCHP_Q_E);
   MC_MACQ_ResetMessageQueue(MAC_DLCCHS_Q_E);
   MC_MACQ_ResetMessageQueue(MAC_RACHP_Q_E);
   MC_MACQ_ResetMessageQueue(MAC_RACHS_Q_E);
   MC_MACQ_ResetMessageQueue(MAC_ACK_Q_E);

   current_dch_message.Type = CO_MESSAGE_MAX_E;
   current_dlcchp_message.Type = CO_MESSAGE_MAX_E;
   current_dlcchs_message.Type = CO_MESSAGE_MAX_E;
   current_rachp_message.Type = CO_MESSAGE_MAX_E;
   current_rachs_message.Type = CO_MESSAGE_MAX_E;
   current_ack_message.Type = CO_MESSAGE_MAX_E;
   
   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_MACQ_Push
* Function to add a new message to one of the MAC queues.
*
* @param - queue     The ID of the queue to add to.
* @param - pMessage  The message to be added
*
* @return - error code,
*/
ErrorCode_t MC_MACQ_Push(const MacQueueType_t queue, const CO_Message_t* pMessage)
{
   ErrorCode_t error = ERR_INVALID_POINTER_E;
   osStatus osStat = osErrorOS;
   
   if ( pMessage )
   {
      CO_Message_t* pMsg = ALLOCMESHPOOL;
      if ( pMsg )
      {
         memcpy(pMsg, pMessage, sizeof(CO_Message_t));
         
         switch (queue)
         {
            case MAC_DCH_Q_E:
            {
               osStat = osMessagePut(DCHQ, (uint32_t)pMsg, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  FREEMESHPOOL(pMsg);
                  error = ERR_QUEUE_OVERFLOW_E;
               }
               else
               {
                  gMacQueueStatusRegister.DchQCount++;
                  error = SUCCESS_E;
               }
            }
               break;
            case MAC_DLCCHP_Q_E:
            {
               osStat = osMessagePut(DLCCHPQ, (uint32_t)pMsg, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  FREEMESHPOOL(pMsg);
                  error = ERR_QUEUE_OVERFLOW_E;
               }
               else
               {
                  gMacQueueStatusRegister.DlcchPQCount++;
                  error = SUCCESS_E;
               }
            }
               break;
            case MAC_DLCCHS_Q_E:
            {
               osStat = osMessagePut(DLCCHSQ, (uint32_t)pMsg, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  FREEMESHPOOL(pMsg);
                  error = ERR_QUEUE_OVERFLOW_E;
               }
               else
               {
                  gMacQueueStatusRegister.DlcchSQCount++;
                  error = SUCCESS_E;
               }
            }
               break;
            case MAC_RACHP_Q_E:
            {
               osStat = osMessagePut(RACHPQ, (uint32_t)pMsg, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  FREEMESHPOOL(pMsg);
                  error = ERR_QUEUE_OVERFLOW_E;
               }
               else
               {
                  gMacQueueStatusRegister.RachPQCount++;
                  error = SUCCESS_E;
               }
            }
               break;
            case MAC_RACHS_Q_E:
            {
               osStat = osMessagePut(RACHSQ, (uint32_t)pMsg, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  FREEMESHPOOL(pMsg);
                  error = ERR_QUEUE_OVERFLOW_E;
               }
               else
               {
                  gMacQueueStatusRegister.RachSQCount++;
                  error = SUCCESS_E;
               }
            }
               break;
            case MAC_ACK_Q_E:
            {
               osStat = osMessagePut(ACKQ, (uint32_t)pMsg, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  FREEMESHPOOL(pMsg);
                  error = ERR_QUEUE_OVERFLOW_E;
               }
               else
               {
                  gMacQueueStatusRegister.AckQCount++;
                  error = SUCCESS_E;
               }
            }
               break;
            default:
               error = ERR_OUT_OF_RANGE_E;
               break;
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
* MC_MACQ_Pop
* Function to get the first message from one of the MAC queues.
*
* @param - queue           The ID of the queue to add to.
* @param - pMessage [OUT]  Pointer to buffer that accepts the popped message.
*
* @return - void
*/
ErrorCode_t MC_MACQ_Pop(const MacQueueType_t queue, const CO_Message_t* pMessage)
{
   osEvent event;
   CO_Message_t *pMsg = NULL;
   ErrorCode_t error = ERR_INVALID_POINTER_E;
   
   if ( pMessage )
   {
      switch (queue)
      {
         case MAC_DCH_Q_E:
         {
            if ( CO_MESSAGE_MAX_E == current_dch_message.Type )
            {
               /* the current message is empty so pop one out of the queue */  
               error = ERR_NOT_FOUND_E;/*assume failure*/
               event = osMessageGet(DCHQ, 0);
               if (osEventMessage == event.status)
               {
                  pMsg = (CO_Message_t *)event.value.p;
                  if (pMsg)
                  {
                     uint8_t* pOutMsg = (uint8_t*)&current_dch_message;
                     memcpy(pOutMsg, pMsg, sizeof(CO_Message_t));
                     pOutMsg = (uint8_t*)pMessage;
                     memcpy(pOutMsg, pMsg, sizeof(CO_Message_t));
                     FREEMESHPOOL(pMsg);
                     error = SUCCESS_E;
                  }
               }
            }
            else 
            {
               /* the current message is populated so use that */
               uint8_t* pOutMsg = (uint8_t*)pMessage;
               memcpy(pOutMsg, &current_dch_message, sizeof(CO_Message_t));
               error = SUCCESS_E;
            }
            break;
         }
         case MAC_DLCCHP_Q_E:
         {
            if ( CO_MESSAGE_MAX_E == current_dlcchp_message.Type )
            {
               /* the current message is empty so pop one out of the queue */  
               error = ERR_NOT_FOUND_E;/*assume failure*/
               event = osMessageGet(DLCCHPQ, 0);
               if (osEventMessage == event.status)
               {
                  pMsg = (CO_Message_t *)event.value.p;
                  if (pMsg)
                  {
                     uint8_t* pOutMsg = (uint8_t*)&current_dlcchp_message;
                     memcpy(pOutMsg, pMsg, sizeof(CO_Message_t));
                     pOutMsg = (uint8_t*)pMessage;
                     memcpy(pOutMsg, pMsg, sizeof(CO_Message_t));
                     FREEMESHPOOL(pMsg);
                     error = SUCCESS_E;
                  }
               }
            }
            else 
            {
               /* the current message is populated so use that */
               uint8_t* pOutMsg = (uint8_t*)pMessage;
               memcpy(pOutMsg, &current_dlcchp_message, sizeof(CO_Message_t));
               error = SUCCESS_E;
            }
            break;
         }
         case MAC_DLCCHS_Q_E:
         {
            if ( CO_MESSAGE_MAX_E == current_dlcchs_message.Type )
            {
               /* the current message is empty so pop one out of the queue */  
               error = ERR_NOT_FOUND_E;/*assume failure*/
               event = osMessageGet(DLCCHSQ, 0);
               if (osEventMessage == event.status)
               {
                  pMsg = (CO_Message_t *)event.value.p;
                  if (pMsg)
                  {
                     uint8_t* pOutMsg = (uint8_t*)&current_dlcchs_message;
                     memcpy(pOutMsg, pMsg, sizeof(CO_Message_t));
                     pOutMsg = (uint8_t*)pMessage;
                     memcpy(pOutMsg, pMsg, sizeof(CO_Message_t));
                     FREEMESHPOOL(pMsg);
                     error = SUCCESS_E;
                  }
               }
            }
            else 
            {
               /* the current message is populated so use that */
               uint8_t* pOutMsg = (uint8_t*)pMessage;
               memcpy(pOutMsg, &current_dlcchs_message, sizeof(CO_Message_t));
               error = SUCCESS_E;
            }
            break;
         }
         case MAC_RACHP_Q_E:
         {
            if ( CO_MESSAGE_MAX_E == current_rachp_message.Type )
            {
               /* the current message is empty so pop one out of the queue */  
               error = ERR_NOT_FOUND_E;/*assume failure*/
               event = osMessageGet(RACHPQ, 0);
               if (osEventMessage == event.status)
               {
                  pMsg = (CO_Message_t *)event.value.p;
                  if (pMsg)
                  {
                     uint8_t* pOutMsg = (uint8_t*)&current_rachp_message;
                     memcpy(pOutMsg, pMsg, sizeof(CO_Message_t));
                     pOutMsg = (uint8_t*)pMessage;
                     memcpy(pOutMsg, pMsg, sizeof(CO_Message_t));
                     FREEMESHPOOL(pMsg);
                     error = SUCCESS_E;
                  }
               }
            }
            else 
            {
               /* the current message is populated so use that */
               uint8_t* pOutMsg = (uint8_t*)pMessage;
               memcpy(pOutMsg, &current_rachp_message, sizeof(CO_Message_t));
               error = SUCCESS_E;
            }
            break;
         }
         case MAC_RACHS_Q_E:
         {
            if ( CO_MESSAGE_MAX_E == current_rachs_message.Type )
            {
               /* the current message is empty so pop one out of the queue */  
               error = ERR_NOT_FOUND_E;/*assume failure*/
               event = osMessageGet(RACHSQ, 0);
               if (osEventMessage == event.status)
               {
                  pMsg = (CO_Message_t *)event.value.p;
                  if (pMsg)
                  {
                     uint8_t* pOutMsg = (uint8_t*)&current_rachs_message;
                     memcpy(pOutMsg, pMsg, sizeof(CO_Message_t));
                     pOutMsg = (uint8_t*)pMessage;
                     memcpy(pOutMsg, pMsg, sizeof(CO_Message_t));
                     FREEMESHPOOL(pMsg);
                     error = SUCCESS_E;
                  }
               }
            }
            else 
            {
               /* the current message is populated so use that */
               uint8_t* pOutMsg = (uint8_t*)pMessage;
               memcpy(pOutMsg, &current_rachs_message, sizeof(CO_Message_t));
               error = SUCCESS_E;
            }
            break;
         }
         case MAC_ACK_Q_E:
         {
            if ( CO_MESSAGE_MAX_E == current_ack_message.Type )
            {
               /* the current message is empty so pop one out of the queue */  
               error = ERR_NOT_FOUND_E;/*assume failure*/
               event = osMessageGet(ACKQ, 0);
               if (osEventMessage == event.status)
               {
                  pMsg = (CO_Message_t *)event.value.p;
                  if (pMsg)
                  {
                     uint8_t* pOutMsg = (uint8_t*)&current_ack_message;
                     memcpy(pOutMsg, pMsg, sizeof(CO_Message_t));
                     pOutMsg = (uint8_t*)pMessage;
                     memcpy(pOutMsg, pMsg, sizeof(CO_Message_t));
                     FREEMESHPOOL(pMsg);
                     error = SUCCESS_E;
                  }
               }
            }
            else 
            {
               /* the current message is populated so use that */
               uint8_t* pOutMsg = (uint8_t*)pMessage;
               memcpy(pOutMsg, &current_ack_message, sizeof(CO_Message_t));
               error = SUCCESS_E;
            }
            break;
         }
         default:
            error = ERR_OUT_OF_RANGE_E;
            break;
      }
   }
   return error;
}

/*************************************************************************************/
/**
* MC_MACQ_Discard
* Function to remove the first message from one of the MAC queues.
*
* @param - queue           The ID of the queue to remove from.
*
* @return - void
*/
ErrorCode_t MC_MACQ_Discard(const MacQueueType_t queue)
{
   osEvent event;
   ErrorCode_t error = ERR_NOT_FOUND_E;
   
   switch (queue)
   {
      case MAC_DCH_Q_E:
      {
         if ( CO_MESSAGE_MAX_E != current_dch_message.Type )
         {
            current_dch_message.Type = CO_MESSAGE_MAX_E;
            gMacQueueStatusRegister.DchQCount--;
            error = SUCCESS_E;
         }
         else 
         {
            /* the current message is empty so pop one out of the queue */  
            error = ERR_NOT_FOUND_E;/*assume failure*/
            event = osMessageGet(DCHQ, 0);
            if (osEventMessage == event.status)
            {
               CO_Message_t *pMessage = (CO_Message_t*)event.value.p;
               if( pMessage )
               {
                  FREEMESHPOOL(pMessage);
                  gMacQueueStatusRegister.DchQCount--;
               }
               error = SUCCESS_E;
            }
         }
      }
      break;
      case MAC_DLCCHP_Q_E:
      {
         if ( CO_MESSAGE_MAX_E != current_dlcchp_message.Type )
         {
            current_dlcchp_message.Type = CO_MESSAGE_MAX_E;
            gMacQueueStatusRegister.DlcchPQCount--;
            error = SUCCESS_E;
         }
         else 
         {
            /* the current message is empty so pop one out of the queue */  
            error = ERR_NOT_FOUND_E;/*assume failure*/
            event = osMessageGet(DLCCHPQ, 0);
            if (osEventMessage == event.status)
            {
               CO_Message_t *pMessage = (CO_Message_t*)event.value.p;
               if( pMessage )
               {
                  FREEMESHPOOL(pMessage);
                  gMacQueueStatusRegister.DlcchPQCount--;
               }
               error = SUCCESS_E;
            }
         }
      }
      break;
      case MAC_DLCCHS_Q_E:
      {
         if ( CO_MESSAGE_MAX_E != current_dlcchs_message.Type )
         {
            current_dlcchs_message.Type = CO_MESSAGE_MAX_E;
            gMacQueueStatusRegister.DlcchSQCount--;
            error = SUCCESS_E;
         }
         else 
         {
            /* the current message is empty so pop one out of the queue */  
            error = ERR_NOT_FOUND_E;/*assume failure*/
            event = osMessageGet(DLCCHSQ, 0);
            if (osEventMessage == event.status)
            {
               CO_Message_t *pMessage = (CO_Message_t*)event.value.p;
               if( pMessage )
               {
                  FREEMESHPOOL(pMessage);
                  gMacQueueStatusRegister.DlcchSQCount--;
               }
               error = SUCCESS_E;
            }
         }
      }
      break;
      case MAC_RACHP_Q_E:
      {
         if ( CO_MESSAGE_MAX_E != current_rachp_message.Type )
         {
            current_rachp_message.Type = CO_MESSAGE_MAX_E;
            gMacQueueStatusRegister.RachPQCount--;
            error = SUCCESS_E;
         }
         else 
         {
            /* the current message is empty so pop one out of the queue */  
            error = ERR_NOT_FOUND_E;/*assume failure*/
            event = osMessageGet(RACHPQ, 0);
            if (osEventMessage == event.status)
            {
               CO_Message_t *pMessage = (CO_Message_t*)event.value.p;
               if( pMessage )
               {
                  FREEMESHPOOL(pMessage);
                  gMacQueueStatusRegister.RachPQCount--;
               }
               error = SUCCESS_E;
            }
         }
         break;
      }
      case MAC_RACHS_Q_E:
      {
         if ( CO_MESSAGE_MAX_E != current_rachs_message.Type )
         {
            current_rachs_message.Type = CO_MESSAGE_MAX_E;
            gMacQueueStatusRegister.RachSQCount--;
            error = SUCCESS_E;
         }
         else 
         {
            /* the current message is empty so pop one out of the queue */  
            error = ERR_NOT_FOUND_E;/*assume failure*/
            event = osMessageGet(RACHSQ, 0);
            if (osEventMessage == event.status)
            {
               CO_Message_t *pMessage = (CO_Message_t*)event.value.p;
               if( pMessage )
               {
                  FREEMESHPOOL(pMessage);
                  gMacQueueStatusRegister.RachSQCount--;
               }
               error = SUCCESS_E;
            }
         }
      }
      break;
      case MAC_ACK_Q_E:
      {
         if ( CO_MESSAGE_MAX_E != current_ack_message.Type )
         {
            current_ack_message.Type = CO_MESSAGE_MAX_E;
            gMacQueueStatusRegister.AckQCount--;
            error = SUCCESS_E;
         }
         else 
         {
            /* the current message is empty so pop one out of the queue */  
            error = ERR_NOT_FOUND_E;/*assume failure*/
            event = osMessageGet(ACKQ, 0);
            if (osEventMessage == event.status)
            {
               CO_Message_t *pMessage = (CO_Message_t*)event.value.p;
               if(pMessage)
               {
                  FREEMESHPOOL(pMessage);
                  gMacQueueStatusRegister.AckQCount--;
               }
               error = SUCCESS_E;
            }
         }
      }
      break;
      default:
         error = ERR_OUT_OF_RANGE_E;
         break;
   }
   return error;
}

/*************************************************************************************/
/**
* MC_MACQ_MessageCount
* Function to add a new message to one of the MAC queues.
*
* @param - queue           The ID of the queue.
*
* @return - The message count or 0 if the queue type is unknown.
*/
uint32_t MC_MACQ_MessageCount(const MacQueueType_t queue)
{
   uint32_t msg_count = 0;
   switch (queue)
   {
      case MAC_DCH_Q_E:
         msg_count = gMacQueueStatusRegister.DchQCount;
         break;
      case MAC_DLCCHP_Q_E:
         msg_count = gMacQueueStatusRegister.DlcchPQCount;
         break;
      case MAC_DLCCHS_Q_E:
         msg_count = gMacQueueStatusRegister.DlcchSQCount;
         break;
      case MAC_RACHP_Q_E:
         msg_count = gMacQueueStatusRegister.RachPQCount;
         break;
      case MAC_RACHS_Q_E:
         msg_count = gMacQueueStatusRegister.RachSQCount;
         break;
      case MAC_ACK_Q_E:
         msg_count = gMacQueueStatusRegister.AckQCount;
         break;
      default:
         break;
   }
   
   return msg_count;
}


/*************************************************************************************/
/**
* MC_MACQ_ResetMessageQueue
* Function to flush all messages from one of the MAC queues.
*
* @param - queue  The ID of the queue.
*
* @return - SUCCESS_E if the queue was flushed, ERR_NOT_FOUND_E otherwise.
*/
ErrorCode_t MC_MACQ_ResetMessageQueue(const MacQueueType_t queue)
{
   osEvent event;
   ErrorCode_t result = SUCCESS_E;
   switch (queue)
   {
      case MAC_DCH_Q_E:
      {
         do
         {
            event = osMessageGet(DCHQ, 0);
            if (osEventMessage == event.status)
            {
               CO_Message_t* pMsg = (CO_Message_t *)event.value.p;
               if (pMsg)
               {
                  FREEMESHPOOL(pMsg);
               }            
            }

         } while ( osEventMessage == event.status );
         gMacQueueStatusRegister.DchQCount = 0;
         current_dch_message.Type = CO_MESSAGE_MAX_E;
      }
         break;
      case MAC_DLCCHP_Q_E:
      {
         do
         {
            event = osMessageGet(DLCCHPQ, 0);
            if (osEventMessage == event.status)
            {
               CO_Message_t* pMsg = (CO_Message_t *)event.value.p;
               if (pMsg)
               {
                  FREEMESHPOOL(pMsg);
               }            
            }

         } while ( osEventMessage == event.status );
         gMacQueueStatusRegister.DlcchPQCount = 0;
         current_dlcchp_message.Type = CO_MESSAGE_MAX_E;
      }
         break;
      case MAC_DLCCHS_Q_E:
      {
         do
         {
            event = osMessageGet(DLCCHSQ, 0);
            if (osEventMessage == event.status)
            {
               CO_Message_t* pMsg = (CO_Message_t *)event.value.p;
               if (pMsg)
               {
                  FREEMESHPOOL(pMsg);
               }            
            }

         } while ( osEventMessage == event.status );
         gMacQueueStatusRegister.DlcchSQCount = 0;
         current_dlcchs_message.Type = CO_MESSAGE_MAX_E;
      }
         break;
      case MAC_RACHP_Q_E:
      {
         do
         {
            event = osMessageGet(RACHPQ, 0);
            if (osEventMessage == event.status)
            {
               CO_Message_t* pMsg = (CO_Message_t *)event.value.p;
               if (pMsg)
               {
                  FREEMESHPOOL(pMsg);
               }            
            }

         } while ( osEventMessage == event.status );
         gMacQueueStatusRegister.RachPQCount = 0;
         current_rachp_message.Type = CO_MESSAGE_MAX_E;
      }
         break;
      case MAC_RACHS_Q_E:
      {
         do
         {
            event = osMessageGet(RACHSQ, 0);
            if (osEventMessage == event.status)
            {
               CO_Message_t* pMsg = (CO_Message_t *)event.value.p;
               if (pMsg)
               {
                  FREEMESHPOOL(pMsg);
               }            
            }

         } while ( osEventMessage == event.status );
         gMacQueueStatusRegister.RachSQCount = 0;
         current_rachs_message.Type = CO_MESSAGE_MAX_E;
      }
         break;
      case MAC_ACK_Q_E:
      {
         do
         {
            event = osMessageGet(ACKQ, 0);
            if (osEventMessage == event.status)
            {
               CO_Message_t* pMsg = (CO_Message_t *)event.value.p;
               if (pMsg)
               {
                  FREEMESHPOOL(pMsg);
               }            
            }

         } while ( osEventMessage == event.status );
         gMacQueueStatusRegister.AckQCount = 0;
         current_ack_message.Type = CO_MESSAGE_MAX_E;
      }
         break;
      default:
         result = ERR_OUT_OF_RANGE_E;
         break;
   }

   return result;
}
