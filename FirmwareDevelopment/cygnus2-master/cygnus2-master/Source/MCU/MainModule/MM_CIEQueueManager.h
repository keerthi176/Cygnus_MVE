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
*  File         : MM_CIEQueueManager.h
*
*  Description  : NCU ONLY.  Manages the four queues for CIE mesages, Fire, Alarm, Fault, Misc.
*
*************************************************************************************/

#ifndef MM_CIE_QUEUE_MGR_H
#define MM_CIE_QUEUE_MGR_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>



/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"
#include "CO_Message.h"
#include "MM_CommandProcessor.h"
#include "CO_Queue.h"

/* Public Constants
*************************************************************************************/
#define CIE_POOL_SIZE 180
#define CIE_Q_SIZE    32
#define CIE_MISC_Q_SIZE    128
#define CIE_Q_TXBUFFER_SIZE    50
#define CIE_Q_OUTPUT_SIGNAL_SIZE    20

#define CIE_Q_MAX_QUEUE_ENTRIES_FOR_CACHE_STATUS_REQUEST 10
/* Public Structures
*************************************************************************************/ 

typedef struct
{
   int32_t CieFireQCount; /* Number of messages in the Fire queue */
   int32_t CieFireQLostMessageCount; /* Number of lost messages for the Fire queue */
   int32_t CieAlarmQCount; /* Number of messages in the Alarm queue */
   int32_t CieAlarmQLostMessageCount; /* Number of lost messages for the Alarm queue */
   int32_t CieFaultQCount; /* Number of messages in the Fault queue */
   int32_t CieFaultQLostMessageCount; /* Number of lost messages for the Fault queue */
   int32_t CieMiscQCount; /* Number of messages in the Misc queue */
   int32_t CieMiscQLostMessageCount; /* Number of lost messages for the Misc queue */
//   int32_t CieTxBufferQCount; /* Number of messages in the TxBuffer queue */
   int32_t CieTxBufferQLostMessageCount; /* Number of lost messages for the TxBuffer queue */
//   int32_t CieOutputSignalQCount; /* Number of messages in the OutputSignal queue */
   int32_t CieOutputSignalQLostMessageCount; /* Number of lost messages for the OutputSignal queue */
} MessageStatusRegister_t;

/* Public Enumerations
*************************************************************************************/
typedef enum 
{
   CIE_OUTPUT_QUEUE_NONE_E,
   CIE_OUTPUT_QUEUE_1_E,
   CIE_OUTPUT_QUEUE_2_E,
   CIE_OUTPUT_QUEUE_3_E,
   CIE_OUTPUT_QUEUE_MAX_E
} CieOutputQueueId_t;


/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t MM_CIEQ_Initialise(void);
ErrorCode_t MM_CIEQ_Push(const CIEQueueType_t queue, const CieBuffer_t* message);
ErrorCode_t MM_CIEQ_Pop(const CIEQueueType_t queue, CieBuffer_t* message);
ErrorCode_t MM_CIEQ_Discard(const CIEQueueType_t queue);
uint32_t MM_CIEQ_MessageCount(const CIEQueueType_t queue);
uint32_t MM_CIEQ_LostMessageCount(const CIEQueueType_t queue);
ErrorCode_t MM_CIEQ_ResetLostMessageCount(const CIEQueueType_t queue);
ErrorCode_t MM_CIEQ_ResetMessageQueue(const CIEQueueType_t queue);
bool MM_CIEQ_TxBufferConfirmation(const uint8_t TransactionID, const bool MsgSent);
bool MM_CIEQ_TxBufferWaitingForResponse(void);
bool MM_CIEQ_TxBufferTimedOut(void);
bool MM_CIEQ_OutputSignalConfirmation(const uint32_t Handle, const bool MsgSent);
bool MM_CIEQ_OutputSignalWaitingForConfirm(void);
bool MM_CIEQ_OutputSignalTimedOut(void);
bool MM_CIEQ_GetNextOutputSignal(CieBuffer_t* pMessage, bool* pNewSignal);
bool MM_CIEQ_AlarmOutputSignalConfirmation(const uint32_t Handle, const bool MsgSent);

/* Macros
*************************************************************************************/



#endif // MM_CIE_QUEUE_MGR_H
