/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2019 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : DM_RelayControl.h
*
*  Description  : Header for the IOU Relay control
*
*************************************************************************************/

#ifndef DM_RELAYCONTROL_H
#define DM_RELAYCONTROL_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "stm32l4xx.h"
#include "CO_ErrorCode.h"
#include "CO_Message.h"

/* User Include Files
*************************************************************************************/

/* Public Constants
*************************************************************************************/


/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/
typedef enum
{
   RELAY_STATE_OPEN_E,
   RELAY_STATE_CLOSED_E,
   RELAY_STATE_UNKNOWN_E
} RelayState_t;

typedef enum
{
   INPUT_STATE_RESET_E,
   INPUT_STATE_SET_E,
   INPUT_STATE_FAULT_SHORT_E,
   INPUT_STATE_FAULT_OPEN_E
} InputState_t;

/* Public Functions Prototypes
*************************************************************************************/
void DM_RC_Initialise(void);
ErrorCode_t DM_RC_ReadInputs(InputState_t* state1, InputState_t* state2);
ErrorCode_t DM_RC_WriteOutput(const CO_ChannelIndex_t channel, const RelayState_t state);
RelayState_t DM_RC_ReadOutputState(const CO_ChannelIndex_t channel);
bool DM_RC_CanGoToSleep(void);
ErrorCode_t DM_RC_SetLastReportedState(const CO_ChannelIndex_t channel, const InputState_t state);
ErrorCode_t DM_RC_GetLastReportedState(const CO_ChannelIndex_t channel, InputState_t* const state);

/* Macros
*************************************************************************************/



#endif // DM_RELAYCONTROL_H
