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
*  File         : MM_IdleDemon.c
*
*  Description  : Idle Demon - runs when no other tasks are active
*                 this includes entering STOP mode
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdbool.h>


/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "CO_Defines.h"
#include "board.h"
#include "lptim.h"
#include "DM_MCUPower.h"
#include "DM_IWDG.h"
#include "DM_LED.h"
#include "MM_ConfigSerialTask.h"
#include "MM_TimedEventTask.h"
#include "DM_RelayControl.h"
//#include "MM_DataManager.h"
#ifdef USE_NEW_HEAD_INTERFACE
#include "MM_PluginInterfaceTask.h"
#else
#include "MM_HeadInterfaceTask.h"
#endif
#include "MM_ApplicationCommon.h"

/* Private Functions Prototypes
*************************************************************************************/
#ifdef ENABLE_STOP_MODE
static void MM_Idle_EnterSTOPMode(void);
#endif


/* Global Variables
*************************************************************************************/
#ifdef ENABLE_STOP_MODE
extern bool gMacTimerFlag;
extern bool gUartBuffEmptyFlag;
extern uint16_t gSerialAwakeTimeout;
extern bool gSerialWakeupInterrupted;
extern DM_BaseType_t gBaseType;
extern bool gDynamicPowerMode;

#endif /*ENABLE_STOP_MODE*/

#ifdef USE_PIN_11
extern Gpio_t Pin11;
#endif

/* Private Variables
*************************************************************************************/



/*************************************************************************************/
/**
* static void MM_Idle_EnterSTOPMode
*
* Function to enter STOP mode. As part of this function the processor enters STOP mode
* and only returns when the processor leaves STOP mode.
*
* This function is called from the os_idle_demon when the OS is not running any tasks.
*
* The function reads the current time from the LPTIM and calculates how long before the
* next wakeup event. This sleep time is used for suspending and resuming the HAL timer and
* the os.
*
* @param - void
*
* @return - void
*/
#ifdef ENABLE_STOP_MODE
static void MM_Idle_EnterSTOPMode(void)
{
   uint16_t sleepTimeLptim = 0;           // time that processor enters STOP mode in LPTIM cycles
   uint16_t wakeTimeLptim = 0;            // time that processor wakes up in LPTIM cycles
   uint16_t timeAsleepLptim = 0;          // difference between sleepTime and wakeTime in LPTIM cycles
   uint16_t timeAsleepMs = 0;             // difference between sleepTime and wakeTime in mS
   
   /* Suspend RTX thread scheduler */
   os_suspend();

   /* First thing is to disable the IRQ to avoid any wake-up event to be
    * acknowledged before we enter the sleep state 
    */
   __disable_irq();

   /* set up the serial interfaces in the sleep configuration*/
   MM_ConfigSerialEnterStopMode();

   /* Enable Debugger to run in STOP mode */
   #if defined ENABLE_DBG_STOP_MODE
   HAL_DBGMCU_EnableDBGStopMode();
   #endif
   
   /* Get the sleep time */
   sleepTimeLptim = (uint16_t)LPTIM_ReadCounter(&hlptim1);

   /* Stop the DMA clock as the all the UARTs are disabled in the sleep state */
   __HAL_RCC_DMA1_CLK_DISABLE();
   __HAL_RCC_DMA2_CLK_DISABLE();

   /* If the LPTIM IRQ is pending, don't bother to go to sleep
    * OR if the LPTIM flag is pending (Interrupt fired between the OS interrupt and the ISR disable)
    *
    * We need to restore the system for the MAC task to process this event
    */
   if ( (0 == HAL_NVIC_GetPendingIRQ(LPTIM1_IRQn)) && (false == gMacTimerFlag))
   {
      #ifdef IWDG_ENABLE
      /* Refresh the watchdog before we go enter the stop mode */
      HAL_IWDG_Refresh(&IwdgHandle);
      #endif
      
      /* put the MCU to sleep */
      DM_MCUPower_Enter_STOPMode();
      
      #ifdef IWDG_ENABLE
      /* Refresh the watchdog after we wake-up */
      HAL_IWDG_Refresh(&IwdgHandle);
      #endif
   }

   /* Get the wake time */
   wakeTimeLptim = (uint16_t)LPTIM_ReadCounter(&hlptim1);
   
   /* derive time asleep in LPTIM clock cycles */
   timeAsleepLptim = (uint16_t)(wakeTimeLptim - sleepTimeLptim);
   
   /* derive time asleep in RTX Timer Ticks */
   timeAsleepMs = timeAsleepLptim >> 4;   // * 1000 / LPTIM_TICKS_PER_SEC = / 16.384 ~= >> 4

   /* restore the serial interfaces to normal mode */
   #ifdef RESTORE_SERIAL_PORTS_AFTER_WAKEUP
   MM_ConfigSerialLeaveStopMode();
   #endif

   /* Re-enable the interrupts after everything is restored prior to the sleep */
   __enable_irq();
   
   /* Resume RTX thread scheduler */
   os_resume(timeAsleepMs);
}

/*************************************************************************************/
/**
* static void MM_Idle_SystemCanGoToSleep
*
* Check if all conditions are fulfilled to enter stop mode
*
* @param - void
*
* @return - true if conditions are met to go into stop mode
*/
__STATIC_INLINE bool MM_Idle_SystemCanGoToSleep(void)
{
   bool result = true;
   
   
   if ((STATE_IDLE_E == GetCurrentDeviceState() )
      || (false == SerialPort_CanGoToSleep()      )
      || (false == MM_ConfigSerial_CanGoToSleep() )
      || (false == MM_HeadInterface_CanGoToSleep())
      || (false == TE_CanEnterSleepMode()         )
      || (false == DM_RC_CanGoToSleep()           )
      || ((BASE_NCU_E == gBaseType) && (false == gDynamicPowerMode)) )
   {
      /* We can't go to sleep now */
      result = false;
#ifdef REPORT_WHY_CANT_SLEEP
      static uint32_t count = 0;
      count++;
      if ( 100000 < count )
      { count = 0;
         bool ser = SerialPort_CanGoToSleep();
         bool cdfs = MM_ConfigSerial_CanGoToSleep();
         CO_PRINT_A_1(DBG_INFO_E,"Block sleep in state %d\r\n", GetCurrentDeviceState());
         CO_PRINT_A_5(DBG_INFO_E,"ser=%d,cdfs=%d,hd=%d,TE=%d,RC=%d\r\n", ser, cdfs,
         MM_HeadInterface_CanGoToSleep(), TE_CanEnterSleepMode(), DM_RC_CanGoToSleep());
      }
#endif
   }
   return result;
}
#endif

/************************************************************************************/
/**
* function name : os_idle_demon
* description	 : idle demon. This runs when the OS detects that no tasks are active.
*
* @param - void
*
* @return - void
*/
void os_idle_demon (void)
{
   for (;;) 
   {
      #ifdef ENABLE_STOP_MODE
      if(false != MM_Idle_SystemCanGoToSleep())
      {
         MM_Idle_EnterSTOPMode();
      }
      #endif /*ENABLE_STOP_MODE*/

      #ifdef IWDG_ENABLE
      /* Refresh IWDG: reload counter */
      HAL_IWDG_Refresh(&IwdgHandle);
      #endif
      
      /* Schedule the LED Device Manager */
      DM_LedPeriodicTick();
      
      /* Schedule timed events */
      TE_TimedEventTimerTick();

   }
}
