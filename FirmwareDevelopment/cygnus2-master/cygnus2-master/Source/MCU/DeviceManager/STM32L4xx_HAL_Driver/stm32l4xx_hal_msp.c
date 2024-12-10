/**
  ******************************************************************************
  * File Name          : stm32l4xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization 
  *                      and de-Initialization codes.
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
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
#include "cmsis_os.h"
#include "lptim.h"

extern void _Error_Handler(char *, int32_t);
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
/**
  * Initializes the Global MSP.
  */
void HAL_MspInit(void)
{
  /* USER CODE BEGIN MspInit 0 */

  /* USER CODE END MspInit 0 */

  __HAL_RCC_SYSCFG_CLK_ENABLE();
  __HAL_RCC_PWR_CLK_ENABLE();

  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* System interrupt init*/
  /* MemoryManagement_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
  /* BusFault_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
  /* UsageFault_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
  /* DebugMonitor_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0);

  /* USER CODE BEGIN MspInit 1 */

  /* USER CODE END MspInit 1 */
}

/* USER CODE BEGIN 1 */
/**
  * @brief This function provides minimum delay (in milliseconds) based 
  *        on variable incremented. 
  * @note In the default implementation , SysTick timer is the source of time base.
  *       It is used to generate interrupts at regular time intervals where uwTick
  *       is incremented.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @param Delay  specifies the delay time length, in milliseconds.
  * @retval None
  */
void HAL_Delay(uint32_t Delay)
{
  uint32_t tickstart = HAL_GetTick();
  uint32_t wait = Delay;

  /* Add a period to guaranty minimum wait */
  if (wait < HAL_MAX_DELAY)
  {
    wait++;
  } 

  while((HAL_GetTick() - tickstart) <  wait)
  {

  }
}

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
  /* The System tick is managed by the RTOS*/
  return HAL_OK;
}

/**
  * @brief Provide a tick value in millisecond.
  * @note This function is declared as __weak to be overwritten in case of other
  *       implementations in user file.
  * @retval tick value
  */
uint32_t HAL_GetTick(void)
{
   static uint16_t ticks_msb = 0U;
   static uint32_t lptim_last_ticks = 0U;
   uint32_t ticks_now;

   if (SysTick->CTRL & SysTick_CTRL_TICKINT_Msk)
   {
      /* System is running, use the RTOS tick converted to ms */
      return (osKernelSysTick()/osKernelSysTickMicroSec(1000));
   }

   /* OS is probably asleep, use the LPTIM timer instead of the OS tick*/
   ticks_now = LPTIM_ReadCounter(&hlptim1);

   /* Check the delay */
   if (ticks_now < lptim_last_ticks)
   {
      ticks_msb++;
   }

   lptim_last_ticks = ticks_now;

   return (((uint32_t)ticks_msb<<16) + ticks_now)>>4;
}

///**
//  * @brief RNG MSP Initialization
//  *        This function configures the hardware resources used in this example:
//  *           - Peripheral's clock enable
//  * @param hrng: RNG handle pointer
//  * @retval None
//  */
//void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng)
//{
//  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

//  /*Select PLLQ output as RNG clock source */
//  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RNG;
//  PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_PLL;
//  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

//  /* RNG Peripheral clock enable */
//  __HAL_RCC_RNG_CLK_ENABLE();
//}

///**
//  * @brief RNG MSP De-Initialization
//  *        This function freeze the hardware resources used in this example:
//  *          - Disable the Peripheral's clock
//  * @param hrng: RNG handle pointer
//  * @retval None
//  */
//void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng)
//{
//  /* Enable RNG reset state */
//  __HAL_RCC_RNG_FORCE_RESET();

//  /* Release RNG from reset state */
//  __HAL_RCC_RNG_RELEASE_RESET();
//}

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
