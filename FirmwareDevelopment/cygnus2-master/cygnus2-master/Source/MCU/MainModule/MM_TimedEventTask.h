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
*  File         : MM_TimedEventTask.h
*
*  Description  : Header for the task that handles timed events
*
*************************************************************************************/

#ifndef MM_TIMEDEVENT_TASK_H
#define MM_TIMEDEVENT_TASK_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "cmsis_os.h"


/* User Include Files
*************************************************************************************/
#include "stm32l4xx.h"
#include "CO_ErrorCode.h"
#include "DM_InputMonitor.h"
#include "DM_LED_cfg.h"

/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t MM_TimedEventTaskInit(void);
void TE_TimedEventTaskMain(void const *argument);
bool TE_CanEnterSleepMode(void);
void TE_FireMCPStateChange(const GPIO_PinState mcpState);
void TE_FirstAidMCPStateChange(const GPIO_PinState mcpState);
void TE_SetSitenetSounderOutput(const bool active);
void TE_StartBatteryTest(void);
void TE_200HourTestEnable(const bool runTest, const CO_ChannelIndex_t channel);
void TE_TimedEventTimerTick(void);
void TE_EnterSleepMode(void);
//uint32_t TE_GetAnaloguePollingPeriod(void);

/* Public Constants
*************************************************************************************/
extern osSemaphoreId  (TimedEventSemId);

/* Macros
*************************************************************************************/
#endif // MM_TIMEDEVENT_TASK_H
