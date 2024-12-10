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
*  File         : DM_SystemClock_Config.h
*
*  Description  : Header for the System Clock Device Manger
*
*************************************************************************************/

#ifndef DM_SYSTEM_CLOCK_H
#define DM_SYSTEM_CLOCK_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>


/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"


/* Public Structures
*************************************************************************************/ 


/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
extern ErrorCode_t DM_SystemClockResetConfig(void);
extern ErrorCode_t DM_SystemClockStopConfig(void);
extern ErrorCode_t DM_SystemClockWakeUpConfig(void);

/* Public Constants
*************************************************************************************/



/* Macros
*************************************************************************************/



#endif // DM_SYSTEM_CLOCK_H
