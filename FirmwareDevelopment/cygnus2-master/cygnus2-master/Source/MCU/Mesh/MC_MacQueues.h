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

#ifndef MM_MAC_QUEUES_H
#define MM_MAC_QUEUES_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>



/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"
#include "CO_Message.h"


/* Public Constants
*************************************************************************************/

/* Public Enumerations
*************************************************************************************/
typedef enum 
{
   MAC_DCH_Q_E,
   MAC_DLCCHP_Q_E,
   MAC_DLCCHS_Q_E,
   MAC_RACHP_Q_E,
   MAC_RACHS_Q_E,
   MAC_ACK_Q_E,
} MacQueueType_t;

/* Public Structures
*************************************************************************************/ 

typedef struct
{
   int32_t DchQCount; /* Number of messages in the DCH queue */
   int32_t DlcchPQCount; /* Number of messages in the P-DLCCH queue */
   int32_t DlcchSQCount; /* Number of messages in the S-DLCCH queue */
   int32_t RachPQCount; /* Number of messages in the P-RACH queue */
   int32_t RachSQCount; /* Number of messages in the S-RACH queue */
   int32_t DulchQCount; /* Number of messages in the DULCH queue */
   int32_t AckQCount; /* Number of messages in the ACK queue */
} MacQueueStatusRegister_t;




/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t MC_MACQ_Initialise(void);
ErrorCode_t MC_MACQ_Push(const MacQueueType_t queue, const CO_Message_t* message);
ErrorCode_t MC_MACQ_Pop(const MacQueueType_t queue, const CO_Message_t* message);
ErrorCode_t MC_MACQ_Discard(const MacQueueType_t queue);
uint32_t MC_MACQ_MessageCount(const MacQueueType_t queue);
ErrorCode_t MC_MACQ_ResetMessageQueue(const MacQueueType_t queue);

/* Macros
*************************************************************************************/


#endif // MM_MAC_QUEUES_H

