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
*  File         : MM_GpioTask.h
*
*  Description  : Header for GPIO task
*
*************************************************************************************/

#ifndef MM_GPIO_TASK_H
#define MM_GPIO_TASK_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>



/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "DM_InputMonitor.h"


/* Public Structures
*************************************************************************************/ 
extern osSemaphoreId  (GpioSemId);



/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t MM_GpioTaskInit(bool isFireCpEnabled, bool isFirstAidCpEnabled);
void MM_GpioTaskMain(void const *argument);
DM_InputMonitorState_t GetConstructionPIRState(void);

/* Public Constants
*************************************************************************************/
#define GPIO_POLL_PERIOD_MS  (10u)

/* Macros
*************************************************************************************/



#endif // MM_GPIO_TASK_H
