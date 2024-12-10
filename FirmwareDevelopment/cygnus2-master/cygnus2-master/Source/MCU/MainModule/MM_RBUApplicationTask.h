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
*  File         : MM_RBUApplicationTask.h
*
*  Description  : Header for RBU Application Task
*
*************************************************************************************/

#ifndef MM_RBU_APPLICATION_TASK_H
#define MM_RBU_APPLICATION_TASK_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>



/* User Include Files
*************************************************************************************/
#include "CO_Message.h"


/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t MM_RBUApplicationTaskInit(const uint16_t address, const uint16_t device_configuration, const uint32_t unitSerialNo, const uint32_t defaultZoneNumber);
void MM_RBUApplicationTaskMain(void);
ErrorCode_t MM_ApplicationProcessExitTestMessage(const ApplicationMessage_t* pAppMessage);
ErrorCode_t MM_RBUSendAlarmSignal(const CO_RBUSensorType_t sensorType, const CO_ChannelIndex_t channel, const bool alarmActive, const uint16_t sensorValue);
uint32_t MM_GetFaultStatusFlags(void);
bool MM_RBU_HasWarnings(void);
bool MM_RBU_HasFaults(void);
uint8_t MM_RBU_GetOverallFaults(void);
/* Public Constants
*************************************************************************************/



/* Macros
*************************************************************************************/



#endif // MM_RBU_APPLICATION_TASK_H
