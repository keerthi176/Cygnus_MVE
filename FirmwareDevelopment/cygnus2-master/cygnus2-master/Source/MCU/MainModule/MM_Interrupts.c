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
*  File         : MM_Interrupts.c
*
*  Description  : Interrupt handler for CMPM interrupt from low power timer LPTIM
*
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/
#include "board.h"
#ifdef IS_PPU
#include "MM_PPUApplicationTask.h"
#else
#include "MM_MACTask.h"
#endif
#include "lptim.h"
#include "MM_Interrupts.h"
#include "MM_GpioTask.h"
#include "MM_Main.h"
#include "MM_TimedEventTask.h"

/* Private Functions Prototypes
*************************************************************************************/



/* Global Variables
*************************************************************************************/


/* Private Variables
*************************************************************************************/
#ifdef IS_PPU
LPTIM_CompareMatchCallback_t LPTIM_CompareMatchCallback = MM_PPUAppTimerISR;
#else
LPTIM_CompareMatchCallback_t LPTIM_CompareMatchCallback = MM_MAC_TimerISR;
#endif

/*************************************************************************************/
/**
* HAL_LPTIM_CompareMatchCallback
* Callback function that is called from the HAL when the low power timer counter matches
* the comparitor register value.
*
* @param - pointer to LPTIM handle
*
* @return - void
*/
void HAL_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
   if ( hlptim )
   {
      if ( LPTIM1 == hlptim->Instance )
      {
         LPTIM_CompareMatchCallback();
      }
   }
}


/*************************************************************************************/
/**
* MM_FireMCPInputIrq
* Irq function called when Fire Manual Callpoint is pressed
*
* @param - void
*
* @return - void
*/
void MM_FireMCPInputIrq(void)
{
   static bool gFireMcpInterrupted = false;
   
   if ( false == gFireMcpInterrupted )//Soft mutex
   {
      gFireMcpInterrupted = true;
      //read the call point state
      GPIO_PinState current_state = (GPIO_PinState)GpioRead(&FireMCP);
      TE_FireMCPStateChange(current_state);
      gFireMcpInterrupted = false;
   }
}


/*************************************************************************************/
/**
* MM_FirstAidMCPInputIrq
* Irq function called when First Aid Manual Callpoint is pressed
*
* @param - void
*
* @return - void
*/
void MM_FirstAidMCPInputIrq(void)
{
   static bool gFirstAidMcpInterrupted = false;
   
   if ( false == gFirstAidMcpInterrupted )//Soft mutex
   {
      gFirstAidMcpInterrupted = true;
      //read the call point state
      GPIO_PinState current_state = (GPIO_PinState)GpioRead(&FirstAidMCP);
      TE_FirstAidMCPStateChange(current_state);
      gFirstAidMcpInterrupted = false;
   }
}

/*************************************************************************************/
/**
* MM_TamperSwitchInputIrq
* Irq function called when a Tamper Switch changes state
*
* @param - void
*
* @return - void
*/
void MM_TamperSwitchInputIrq(void)
{
   /* Disable this ISR to reject the stream of interrupts due to the signal fluctuations
    * Clear EXTI line configuration 
    */
   uint32_t temp = EXTI->IMR1;
   temp &= ~((uint32_t)InstallationTamper.pinIndex);
   temp &= ~((uint32_t)DismantleTamper.pinIndex);
   EXTI->IMR1 = temp;
   
   /* Report activity to the system */
   osSemaphoreRelease(GpioSemId);
}

/*************************************************************************************/
/**
* MM_PirInputIrq
* Irq function called when the SiteNet PIR is triggered.
*
* @param - void
*
* @return - void
*/
void MM_PirInputIrq(void)
{
   /* Disable this ISR to reject the stream of interrupts due to the signal fluctuations
    * Clear EXTI line configuration 
    */
   uint32_t temp = EXTI->IMR1;
   temp &= ~((uint32_t)Pir.pinIndex);
   EXTI->IMR1 = temp;
   
   /* Report activity to the system */
   osSemaphoreRelease(GpioSemId);
}

/*************************************************************************************/
/**
* MM_MainBatteryInputIrq
* Irq function called when the Main Battery changes state
*
* @param - void
*
* @return - void
*/
void MM_MainBatteryInputIrq(void)
{
   osSemaphoreRelease(GpioSemId);
}

