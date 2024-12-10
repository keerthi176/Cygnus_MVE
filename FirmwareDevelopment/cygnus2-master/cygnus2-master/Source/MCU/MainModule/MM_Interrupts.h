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
*  File         : MM_Interrupts.h
*
*  Description  : Interrupts module header file
*
*************************************************************************************/

#ifndef MM_INTERRUPTS_H
#define MM_INTERRUPTS_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>



/* User Include Files
*************************************************************************************/
#include "stm32l4xx.h"


/* Public Structures
*************************************************************************************/ 
typedef void (*LPTIM_CompareMatchCallback_t)(void);
extern LPTIM_CompareMatchCallback_t LPTIM_CompareMatchCallback;


/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
void HAL_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *hlptim);
void MM_FireMCPInputIrq(void);
void MM_FirstAidMCPInputIrq(void);
void MM_TamperSwitchInputIrq(void);
void MM_MainBatteryInputIrq(void);
void MM_PirInputIrq(void);

/* Public Constants
*************************************************************************************/



/* Macros
*************************************************************************************/



#endif // MM_INTERRUPTS_H
