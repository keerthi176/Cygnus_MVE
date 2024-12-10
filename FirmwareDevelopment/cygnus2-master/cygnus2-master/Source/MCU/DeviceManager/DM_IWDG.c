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
*  File         : DM_IWDG.c
*
*  Description  : Independent Watchdog (IWDG) of the STM32
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/
#include "stm32l4xx_hal.h"
#include "CO_Defines.h"
#include "DM_IWDG.h"

/* Private Functions Prototypes
*************************************************************************************/



/* Global Variables
*************************************************************************************/



/* Private Variables
*************************************************************************************/

#if defined (STM32L0)
/* The default LSI clock frequency is 37kHz */
#define DEFAULT_LSI_FREQUENCY (37000u)
#elif defined (STM32L476xx)
/* The default LSI clock frequency is 32kHz */
#define DEFAULT_LSI_FREQUENCY (32000u)
#else
#error "IWDG: Please define the LSI clock default frequency"
#endif
/* IWDG handler declaration */
IWDG_HandleTypeDef   IwdgHandle;

/*************************************************************************************/
/**
* DM_IndependentWdgInit
* Initialisation function of the Independent Watchdog
*
* @param - void
*
* @return - ErrorCode_t - status 0=success else error
*/
ErrorCode_t DM_IndependentWdgInit(void)
{
   ErrorCode_t status = SUCCESS_E;

   /* Freeze the IWDG while debugging */
#ifdef IWDG_DEBUG_FREEZE
   #if defined (STM32L0)
   __HAL_RCC_DBGMCU_CLK_ENABLE();
   #endif
   __HAL_DBGMCU_FREEZE_IWDG() ;
#endif
   
   /* Set counter reload value to obtain IWDG_TIMEOUT_PERIOD_MS milliseconds. IWDG TimeOut.
   IWDG counter clock Frequency = uwLsiFreq
   Set Prescaler to 256 (IWDG_PRESCALER_256)
   Timeout Period = (Reload Counter Value * 256) / uwLsiFreq
   So Set Reload Counter Value = (Timeout Period  * uwLsiFreq) / 256 */
   IwdgHandle.Instance = IWDG;
   IwdgHandle.Init.Prescaler = IWDG_PRESCALER_256;
   IwdgHandle.Init.Reload = ((DEFAULT_LSI_FREQUENCY * IWDG_TIMEOUT_PERIOD_MS )/ 256000u);
   IwdgHandle.Init.Window = IWDG_WINDOW_DISABLE;

   if(HAL_IWDG_Init(&IwdgHandle) != HAL_OK)
   {
    /* Initialization Error */
    status = ERR_INIT_FAIL_E;
   }

   return status;
}
