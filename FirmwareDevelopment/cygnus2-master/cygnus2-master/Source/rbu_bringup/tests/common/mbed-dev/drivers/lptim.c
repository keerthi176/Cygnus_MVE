/**
  ******************************************************************************
  * File Name          : LPTIM.c
  * Description        : This file provides code for the configuration
  *                      of the LPTIM instances.
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
#include "lptim.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

LPTIM_HandleTypeDef hlptim1;
LPTIM_HandleTypeDef hlptim2;

/* LPTIM1 init function */
void MX_LPTIM1_Init(void)
{

  hlptim1.Instance = LPTIM1;
  hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV2;
  hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;

//  if (HAL_OK != HAL_LPTIM_Init(&hlptim1))
//  {
//     Error_Handler("FATAL ERROR: MX_LPTIM1_Init()");
//  }

   HAL_LPTIM_Init(&hlptim1);
   
   HAL_LPTIM_Counter_Start_IT_Modified(&hlptim1, 512);

}

/* LPTIM2 init function */
void MX_LPTIM2_Init(void)
{

  hlptim2.Instance = LPTIM2;
  hlptim2.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
  hlptim2.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
  hlptim2.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
  hlptim2.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
  hlptim2.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
  hlptim2.Init.CounterSource = LPTIM_COUNTERSOURCE_INTERNAL;

//  if (HAL_OK != HAL_LPTIM_Init(&hlptim2))
//  {
//     Error_Handler("FATAL ERROR: MX_LPTIM2_Init()");
//  }
   HAL_LPTIM_Init(&hlptim2);
}

void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef* lptimHandle)
{

  if(LPTIM1 == lptimHandle->Instance)
  {
  /* USER CODE BEGIN LPTIM1_MspInit 0 */

  /* USER CODE END LPTIM1_MspInit 0 */
    /* LPTIM1 clock enable */
    __HAL_RCC_LPTIM1_CLK_ENABLE();

    /* LPTIM1 interrupt Init */
    HAL_NVIC_SetPriority(LPTIM1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
  /* USER CODE BEGIN LPTIM1_MspInit 1 */

  /* USER CODE END LPTIM1_MspInit 1 */
  }
  else if(LPTIM2 == lptimHandle->Instance)
  {
    /* LPTIM2 clock enable */
    __HAL_RCC_LPTIM2_CLK_ENABLE();

    /* LPTIM2 interrupt Init */
    HAL_NVIC_SetPriority(LPTIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(LPTIM2_IRQn);
  }
}

void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef* lptimHandle)
{

  if(LPTIM1 == lptimHandle->Instance)
  {
  /* USER CODE BEGIN LPTIM1_MspDeInit 0 */

  /* USER CODE END LPTIM1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LPTIM1_CLK_DISABLE();

    /* LPTIM1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(LPTIM1_IRQn);
  /* USER CODE BEGIN LPTIM1_MspDeInit 1 */

  /* USER CODE END LPTIM1_MspDeInit 1 */
  }
  else if(LPTIM2 == lptimHandle->Instance)
  {
  /* USER CODE BEGIN LPTIM2_MspDeInit 0 */

  /* USER CODE END LPTIM2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_LPTIM2_CLK_DISABLE();

    /* LPTIM1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(LPTIM2_IRQn);
  /* USER CODE BEGIN LPTIM1_MspDeInit 1 */

  /* USER CODE END LPTIM1_MspDeInit 1 */
  }
}

/*******************************************************************************
Function name : HAL_StatusTypeDef HAL_LPTIM_Counter_Start_IT_Modified(LPTIM_HandleTypeDef *hlptim, uint32_t Period)
   returns    : HAL_StatusTypeDef (see details in stm3210xx_hal_def.h)
   arg1       : LPTIM_HandleTypeDef *hlptim - description TBD
   arg2       : uint32_t Period - value between 0 and 0xFFFF
Description   : Loads LPTIM counter period value without re-configuring its operation from scratch
Notes         : restrictions, odd modes
*******************************************************************************/
HAL_StatusTypeDef HAL_LPTIM_Counter_Start_IT_Modified(LPTIM_HandleTypeDef *hlptim, uint32_t Period)
{
	/* Check the parameters */
	assert_param(IS_LPTIM_INSTANCE(hlptim->Instance));
	assert_param(IS_LPTIM_PERIOD(Period));

	/* Set the LPTIM state */
	hlptim->State= HAL_LPTIM_STATE_BUSY;

  /* Enable the Peripheral */
  __HAL_LPTIM_ENABLE(hlptim);
  
  /* Load the period value in the autoreload register */
  __HAL_LPTIM_AUTORELOAD_SET(hlptim, 0xffff);

   /* Load the period value in the comparitor register */
   uint16_t counter = hlptim->Instance->CNT;
   uint16_t comparitor = counter + Period;
   __HAL_LPTIM_COMPARE_SET(hlptim, comparitor);  
   
	/* Enable EXTI Line interrupt on the LPTIM Timer */
	SET_BIT(EXTI->IMR2, EXTI_IMR2_IM32);

	/* Enable rising edge trigger on the LPTIM Wake-up Timer Exti line */
	SET_BIT(EXTI->RTSR2, EXTI_IMR2_IM32);

	/* Disable Compare match interrupt */
	__HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPM);

  /* Start timer in continuous mode */
  __HAL_LPTIM_START_CONTINUOUS(hlptim);
    
  /* Change the TIM state*/
  hlptim->State= HAL_LPTIM_STATE_READY;

  /* Return function status */
  return HAL_OK;
}

/*******************************************************************************
Function name : void HAL_LPTIM_Set_CMP(LPTIM_HandleTypeDef *hlptim, uint32_t value)
   returns    : void
   arg1       : LPTIM_HandleTypeDef *hlptim
   arg2       : uint32_t value - value between 0 and 0xFFFF
Description   : Loads LPTIM counter comparitor
Notes         : 
*******************************************************************************/
void HAL_LPTIM_Set_CMP(LPTIM_HandleTypeDef *hlptim, uint32_t value)
{
   /* Load the value into the comparitor register */
   __HAL_LPTIM_COMPARE_SET(hlptim, value);  
}

/*******************************************************************************
Function name : void HAL_LPTIM_Enable_CMPM_Interrupt(LPTIM_HandleTypeDef *hlptim)
   returns    : void
   arg1       : LPTIM_HandleTypeDef *hlptim
Description   : Enables LPTIM counter compare match interrupt
Notes         : 
*******************************************************************************/
void HAL_LPTIM_Enable_CMPM_Interrupt(LPTIM_HandleTypeDef *hlptim)
{
	/* Enable Compare match interrupt */
	__HAL_LPTIM_ENABLE_IT(hlptim, LPTIM_IT_CMPM);
}

/*******************************************************************************
Function name : void HAL_LPTIM_Disable_CMPM_Interrupt(LPTIM_HandleTypeDef *hlptim)
   returns    : void
   arg1       : LPTIM_HandleTypeDef *hlptim
Description   : Disables LPTIM counter compare match interrupt
Notes         : 
*******************************************************************************/
void HAL_LPTIM_Disable_CMPM_Interrupt(LPTIM_HandleTypeDef *hlptim)
{
	/* Disable Compare match interrupt */
	__HAL_LPTIM_DISABLE_IT(hlptim, LPTIM_IT_CMPM);
}
/* USER CODE BEGIN 1 */

/*******************************************************************************
Function name : uint32_t LPTIM_ReadCounter(LPTIM_HandleTypeDef *hlptim)
   returns    : uint32_t value of the counter
   arg1       : hlptim: LPTIM handle
Description   : Reads and return the LPTIM1
Notes         :
    Snippet from STMicroelectronics RM0351 (DocID024597 Rev 5: Reference manual)
     34.7.8 LPTIM counter register (LPTIM_CNT)
     When the LPTIM is running with an asynchronous clock, reading the LPTIM_CNT register may
     return unreliable values. So in this case it is necessary to perform two consecutive read accesses
     and verify that the two returned values are identical.
*******************************************************************************/
uint32_t LPTIM_ReadCounter(LPTIM_HandleTypeDef *hlptim)
{
   uint32_t number_ticks = 0u;
//   uint32_t check_number_ticks = 0u;

   /* Read the first value of the counter now */
   number_ticks = HAL_LPTIM_ReadCounter(&hlptim1);

//   do
//   {
//      /* Reload the latest counter value */
//      check_number_ticks = number_ticks;

//      /*
//       * Keep reading until we get two identical values as described 
//       * in the function header
//       */
//      number_ticks = HAL_LPTIM_ReadCounter(&hlptim1);

//   } while(number_ticks != check_number_ticks);

   return number_ticks;
}

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
