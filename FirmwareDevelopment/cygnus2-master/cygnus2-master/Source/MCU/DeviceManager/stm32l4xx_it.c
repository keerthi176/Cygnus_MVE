/**
  ******************************************************************************
  * @file    stm32l4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "stm32l4xx.h"
#include "stm32l4xx_it.h"

/* USER CODE BEGIN 0 */
#include "stdbool.h"
#include "string.h"
#include "eeprom_emul.h"
#include "main.h"

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern LPTIM_HandleTypeDef hlptim1;
extern LPTIM_HandleTypeDef hlptim2;
uint16_t gIRQlptim1Value = 0;
bool gIRQlptim1ValueFromWakeUp = false;

/******************************************************************************/
/*            Cortex-M4 Processor Interruption and Exception Handlers         */ 
/******************************************************************************/

/**
* @brief This function handles Non maskable interrupt.
*/
void NMI_Handler(void)
{
   uint32_t corruptedflashaddress;

   /* Check if NMI is due to flash ECCD (error detection) */
   if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_ECCD))
   {
      /* Compute corrupted flash address */
      corruptedflashaddress = FLASH_BASE + (FLASH->ECCR & FLASH_ECCR_ADDR_ECC);

      #if defined(FLASH_OPTR_BFB2)
      /* Add bank size to corrupteflashaddress if fail bank is bank 2 */
      if (READ_BIT(FLASH->ECCR, FLASH_ECCR_BK_ECC) == FLASH_ECCR_BK_ECC)
      {
         corruptedflashaddress += FLASH_BANK_SIZE;
      }
      #endif

      /* Check if corrupted flash address is in eeprom emulation pages */
      if ((corruptedflashaddress >= START_PAGE_ADDRESS) || (corruptedflashaddress <= END_EEPROM_ADDRESS))
      {
         /* Delete the corrupted flash address */
         if (EE_DeleteCorruptedFlashAddress(corruptedflashaddress) == EE_OK)
         {
            /* Resume execution if deletion succeeds */
            return;
         }
      }
   }
   else
   {
      #if _DEBUG
      /* Halt the SW execution through breakpoint insertion to allow investigation
       * Otherwise Reset the system if no debugger is attached
       */
      if(CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
      {
         /* We are in debug mode, halt the code for inspection */
         __asm__ ("bkpt #0");
      }
      else
      #endif
      {
         Error_Handler("NMI Exception caused the System to reset" );
      }      
   }
}

/**
* @brief This function handles Hard fault interrupt.
*/
__asm void HardFault_Handler(void)
{
   IMPORT  My_HardFault_Handler 
   tst LR, #4 ;// Check EXC_RETURN in Link register bit 2.
   ite EQ
   mrseq R0, MSP ;// Stacking was using MSP.
   mrsne R0, PSP ;// Stacking was using PSP.
   b My_HardFault_Handler ;// Stack pointer passed through R0.
}

void My_HardFault_Handler(uint32_t stack_pointer)
{
//   #if _DEBUG
//   /* Halt the SW execution through breakpoint insertion to allow investigation
//    * Otherwise Reset the system if no debugger is attached
//    */
//   if(CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
//   {
//      /* We are in debug mode, halt the code for inspection */
//      __asm__ ("bkpt #0");
//   }
//   else
//   #endif
//   {
volatile uint32_t r0;
volatile uint32_t r1;
volatile uint32_t r2;
volatile uint32_t r3;
volatile uint32_t r12;
volatile uint32_t lr; /* Link register. */
volatile uint32_t pc; /* Program counter. */
volatile uint32_t psr;/* Program status register. */


   r0 = *(uint32_t*)(stack_pointer);
   r1 = *(uint32_t*)(stack_pointer + 4u);
   r2 = *(uint32_t*)(stack_pointer + 8u);
   r3 = *(uint32_t*)(stack_pointer + 12u);

   r12 = *(uint32_t*)(stack_pointer + 16u);
   lr = *(uint32_t*)(stack_pointer + 20u);
   pc = *(uint32_t*)(stack_pointer + 24u);
   psr = *(uint32_t*)(stack_pointer + 28u);
    
      char debug_msg[126];

      sprintf(debug_msg,"HARD FAULT:\n\r\tMMFAR=0x%08X\n\r\tBFAR=0x%08X\n\r\tCFSR=0x%08X\n\r\tHFSR=0x%08X\n\r\tAFSR=0x%08X\n\r\tLR=0x%08X\n\r\tPC=0x%08X", 
                                                  SCB->MMFAR, SCB->BFAR, SCB->CFSR, SCB->HFSR, SCB->AFSR, lr, *(uint32_t*)(stack_pointer + 24u));

      Error_Handler(debug_msg);
      
      while (1);
//   }
}

/**
* @brief This function handles Memory management fault.
*/
void MemManage_Handler(void)
{
   #if _DEBUG
   /* Halt the SW execution through breakpoint insertion to allow investigation
    * Otherwise Reset the system if no debugger is attached
    */
   if(CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
   {
      /* We are in debug mode, halt the code for inspection */
      __asm__ ("bkpt #0");
   }
   else
   #endif
   {
      Error_Handler("MemManage_Handler Exception caused the System to reset" );
   }
}

/**
* @brief This function handles Prefetch fault, memory access fault.
*/
void BusFault_Handler(void)
{
   #if _DEBUG
   /* Halt the SW execution through breakpoint insertion to allow investigation
    * Otherwise Reset the system if no debugger is attached
    */
   if(CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
   {
      /* We are in debug mode, halt the code for inspection */
      __asm__ ("bkpt #0");
   }
   else
   #endif
   {
      Error_Handler("BusFault_Handler Exception caused the System to reset" );
   }
}

/**
* @brief This function handles Undefined instruction or illegal state.
*/
void UsageFault_Handler(void)
{
   #if _DEBUG
   /* Halt the SW execution through breakpoint insertion to allow investigation
    * Otherwise Reset the system if no debugger is attached
    */
   if(CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
   {
      /* We are in debug mode, halt the code for inspection */
      __asm__ ("bkpt #0");
   }
   else
   #endif
   {
      Error_Handler("UsageFault_Handler Exception caused the System to reset");
   }
}

//void SVC_Handler(void)
//{
//   #if _DEBUG
//   /* Halt the SW execution through breakpoint insertion to allow investigation
//    * Otherwise Reset the system if no debugger is attached
//    */
//   if(CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
//   {
//      /* We are in debug mode, halt the code for inspection */
//      __asm__ ("bkpt #0");
//   }
//   else
//   #endif
//   {
//      Error_Handler("SVC_Handler Exception caused the System to reset");
//   }
//}

//void PendSV_Handler(void)
//{
//   #if _DEBUG
//   /* Halt the SW execution through breakpoint insertion to allow investigation
//    * Otherwise Reset the system if no debugger is attached
//    */
//   if(CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
//   {
//      /* We are in debug mode, halt the code for inspection */
//      __asm__ ("bkpt #0");
//   }
//   else
//   #endif
//   {
//      Error_Handler("PendSV_Handler Exception caused the System to reset");
//   }
//}

/**
* @brief This function handles Debug monitor.
*/
void DebugMon_Handler(void)
{
   #if _DEBUG
   /* Halt the SW execution through breakpoint insertion to allow investigation
    * Otherwise Reset the system if no debugger is attached
    */
   if(CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk)
   {
      /* We are in debug mode, halt the code for inspection */
      __asm__ ("bkpt #0");
   }
   else
   #endif
   {
      Error_Handler("DebugMon_Handler Exception caused the System to reset" );
   }
}

/******************************************************************************/
/* STM32L4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l4xx.s).                    */
/******************************************************************************/

/**
* @brief This function handles LPTIM1 global interrupt / LPTIM1 wake-up interrupt through EXTI line 29.
*/
void LPTIM1_IRQHandler(void)
{
  HAL_LPTIM_IRQHandler(&hlptim1);
}

/**
* @brief This function handles LPTIM2 global interrupt / LPTIM2 wake-up interrupt through EXTI line 30.
*/
void LPTIM2_IRQHandler(void)
{
  HAL_LPTIM_IRQHandler(&hlptim2);
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
