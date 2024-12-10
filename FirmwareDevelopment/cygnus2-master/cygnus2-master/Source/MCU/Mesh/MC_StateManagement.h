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
*  File         : MC_StateManagement.h
*
*  Description  : Header file for managing the mesh state and device state
*
*
*************************************************************************************/

#ifndef STATEMANAGEMENT_H
#define STATEMANAGEMENT_H

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
   MC_STATE_WAITING_FOR_SYNC_E,
   MC_STATE_WAITING_FOR_FORMING_E,
   MC_STATE_WAITING_FOR_ACTIVE_E,
   MC_STATE_ACTIVE_E
} MeshAdvanceState_t;

/* Public Structures
*************************************************************************************/
/* Public Functions Prototypes
*************************************************************************************/
void MC_STATE_Initialise(void);
void MC_STATE_AdvanceState(void);
ErrorCode_t MC_STATE_ScheduleStateChange(const CO_State_t nextState, const bool synchronised);
bool MC_STATE_ReadyForStateChange(const CO_State_t nextState);
CO_State_t MC_STATE_GetScheduledState(void);
ErrorCode_t MC_STATE_SetMeshState(const CO_State_t nextState);
CO_State_t MC_STATE_GetMeshState(void);
ErrorCode_t MC_STATE_SetDeviceState(const CO_State_t nextState);
CO_State_t MC_STATE_GetDeviceState(void);
void MC_STATE_ActivateStateChange(const uint8_t LongFrameIndex);

#endif // STATEMANAGEMENT_H
