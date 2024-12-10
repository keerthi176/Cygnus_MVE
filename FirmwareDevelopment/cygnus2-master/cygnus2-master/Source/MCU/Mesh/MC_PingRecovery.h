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

#ifndef PING_RECOVERY_H
#define PING_RECOVERY_H

/* System Include Files
*************************************************************************************/
#include <stdbool.h>
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"
#include "CO_Message.h"

/* Public Constants
*************************************************************************************/

/* Macros
*************************************************************************************/

/* Public Enumerations
*************************************************************************************/
typedef enum
{
   PR_IDLE_E,
   PR_WAITING_TO_SEND_E,
   PR_WAITING_FOR_ACK
} PingRecoveryState_t;

/* Public Structures
*************************************************************************************/
/* Public Functions Prototypes
*************************************************************************************/
void MC_PR_Initialise(void);
ErrorCode_t MC_PR_StartPingForNode(const uint16_t nodeID);
ErrorCode_t MC_PR_CancelPingForNode(const uint16_t nodeID);
void MC_PR_Advance(const uint16_t shortFrame, const uint16_t slotIndex);
ErrorCode_t MC_PR_AckUpdate(const uint16_t nodeID, const bool ackReveived);
void MC_PR_SendPing(const uint16_t nodeID, const SendBehaviour_t behaviour);

#endif // PING_RECOVERY_H
