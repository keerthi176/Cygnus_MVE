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

#ifndef CO_QUEUE_H
#define CO_QUEUE_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>



/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"
#include "CO_Message.h"

/* Public Constants
*************************************************************************************/
#define CO_Q_SIZE 10
#define CIE_QUEUE_DATA_SIZE 64

/* Public Enumerations
*************************************************************************************/
typedef enum
{
   CO_Q_PRIORITY_1_E,
   CO_Q_PRIORITY_2_E,
   CO_Q_PRIORITY_3_E,
   CO_Q_LOW_PRIORITY_E,
   CO_Q_MAX_QUEUES_E
} CO_QueueId_t;

typedef enum
{
   CO_Q_LOCK_UNLOCKED_E,
   CO_Q_LOCK_FIRST_ENTRY_E,
   CO_Q_LOCK_ALL_E,
   CO_Q_LOCK_MAX_E
} QueueLock_t;

/* Public Structures
*************************************************************************************/ 
typedef struct
{
   uint8_t Buffer[CIE_QUEUE_DATA_SIZE];
   uint8_t MessageType;
}CieBuffer_t;

typedef struct
{
   uint32_t MessageHandle;
   ApplicationLayerMessageType_t MessageType;
   CieBuffer_t Message;
} CO_QueueEntry_t;

typedef struct
{
   uint32_t MessageCount;
   QueueLock_t Lock;
   CO_QueueEntry_t Entry[CO_Q_SIZE];
} CO_Queue_t;


/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t CO_QueueInitialise(void);
ErrorCode_t CO_QueuePush(const CO_QueueId_t queueId, const CieBuffer_t* message, const bool removeDuplicate);
ErrorCode_t CO_QueuePeek(const CO_QueueId_t queueId, CieBuffer_t* message);
ErrorCode_t CO_QueueDiscard(const CO_QueueId_t queueId);
uint32_t CO_QueueMessageCount(const CO_QueueId_t queueId);
uint32_t CO_QueueMessageTypeCount(const CO_QueueId_t queueId, const ApplicationLayerMessageType_t msgType);
ErrorCode_t CO_QueueReset(const CO_QueueId_t queueId);
ErrorCode_t CO_QueueSetLock(const CO_QueueId_t queueId, const QueueLock_t lock);

/* Macros
*************************************************************************************/


#endif // CO_QUEUE_H

