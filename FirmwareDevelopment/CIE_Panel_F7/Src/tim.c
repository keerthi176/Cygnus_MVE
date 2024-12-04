/**
  ******************************************************************************
  * File Name          : TIM.c
  * Description        : This file provides code for the configuration
  *                      of the TIM instances.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */


TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;
TIM_HandleTypeDef htim10;
TIM_HandleTypeDef htim13;
TIM_HandleTypeDef htim14;
TIM_HandleTypeDef htim8;

 

/* TIM6 init function */
void MX_TIM6_Init(void)
{
  RCC_ClkInitTypeDef    clkconfig;
  uint32_t              uwTimclock;
  uint32_t              uwPrescalerValue;
  uint32_t              pFLatency;
  
  /*Configure the TIM6 IRQ priority */
  HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 15, 0 ); 
  
  /* Enable the TIM6 global Interrupt */
  HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn); 
  
  /* Enable TIM6 clock */
  __HAL_RCC_TIM6_CLK_ENABLE();
  
  /* Get clock configuration */
  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);
  
  /* Compute TIM13 clock */
  uwTimclock = HAL_RCC_GetPCLK1Freq( );
   
  /* Compute the prescaler value to have TIM13 counter clock equal to 1KHz */
  uwPrescalerValue = (uint32_t) ((1*uwTimclock / 10000) - 1);
  
//  EwPrint( "SYSCLKSource:%d, ClockType:%d, uwTimclock:%d\n", clkconfig.SYSCLKSource, clkconfig.ClockType, uwTimclock );
  
  /* Initialize TIM6 */
  htim6.Instance = TIM6;
  
  /* Initialize TIMx peripheral as follow:
  + Period = [(TIM14CLK/1000) - 1]. to have a (1/1000) s time base.
  + Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
  + ClockDivision = 0
  + Counter direction = Up
  */
  htim6.Init.Period = 10000-1;
  htim6.Init.Prescaler = uwPrescalerValue;
  htim6.Init.ClockDivision = 0;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  
  if(HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
	  _Error_Handler(__FILE__, __LINE__);
  }

}



/* TIM7 init function */
void MX_TIM7_Init(void)
{
  RCC_ClkInitTypeDef    clkconfig;
  uint32_t              uwTimclock;
  uint32_t              uwPrescalerValue;
  uint32_t              pFLatency;
  
  /*Configure the TIM7 IRQ priority */
  HAL_NVIC_SetPriority(TIM7_IRQn, 15, 0 ); 
  
  /* Enable the TIM7 global Interrupt */
  HAL_NVIC_EnableIRQ(TIM7_IRQn); 
  
  /* Enable TIM7 clock */
  __HAL_RCC_TIM7_CLK_ENABLE();
  
  /* Get clock configuration */
  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);
  
  /* Compute TIM7 clock */
  uwTimclock = HAL_RCC_GetPCLK1Freq( );
   
  /* Compute the prescaler value to have TIM13 counter clock equal to 1KHz */
  uwPrescalerValue = (uint32_t) ((uwTimclock / 1000) - 1);
  
//  EwPrint( "SYSCLKSource:%d, ClockType:%d, uwTimclock:%d\n", clkconfig.SYSCLKSource, clkconfig.ClockType, uwTimclock );
  
  /* Initialize TIM7 */
  htim7.Instance = TIM7;
  
  /* Initialize TIMx peripheral as follow:
  + Period = [(TIM14CLK/1000) - 1]. to have a (1/1000) s time base.
  + Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
  + ClockDivision = 0
  + Counter direction = Up
  */
  htim7.Init.Period = 2000-1;
  htim7.Init.Prescaler = uwPrescalerValue;
  htim7.Init.ClockDivision = 0;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  
  if(HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
	  _Error_Handler(__FILE__, __LINE__);
  }

}

 

/* TIM8 init function */
void MX_TIM8_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  htim8.Instance = TIM8;
  htim8.Init.Prescaler = 16800;
  htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim8.Init.Period = 7000;
  htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  htim8.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim8) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim8) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 4000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /*Configure the TIM8 IRQ priority */
  HAL_NVIC_SetPriority(TIM8_CC_IRQn, 15, 0 ); 
  
  /* Enable the TIM8 global Interrupt */
  HAL_NVIC_EnableIRQ(TIM8_CC_IRQn); 
  
   /*Configure the TIM8 IRQ update */
  HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 15, 0 ); 
  
  /* Enable the TIM14 global Interrupt */
  HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn); 
}
#if 0

/* TIM10 init function */
void MX_TIM10_Init(void)
{
  RCC_ClkInitTypeDef    clkconfig;
  uint32_t              uwTimclock;
  uint32_t              uwPrescalerValue;
  uint32_t              pFLatency;
  
  /*Configure the TIM10 IRQ priority */
  HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 15, 0 ); 
  
  /* Enable the TIM10 global Interrupt */
  HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn); 
  
  /* Enable TIM10 clock */
  __HAL_RCC_TIM10_CLK_ENABLE();
  
  /* Get clock configuration */
  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);
  
  /* Compute TIM13 clock */
  uwTimclock = HAL_RCC_GetPCLK1Freq( );
   
  /* Compute the prescaler value to have TIM13 counter clock equal to 1KHz */
  uwPrescalerValue = (uint32_t) ((2*uwTimclock / 10000) - 1);
  
//  EwPrint( "SYSCLKSource:%d, ClockType:%d, uwTimclock:%d\n", clkconfig.SYSCLKSource, clkconfig.ClockType, uwTimclock );
  
  /* Initialize TIM10 */
  htim10.Instance = TIM10;
  
  /* Initialize TIMx peripheral as follow:
  + Period = [(TIM14CLK/1000) - 1]. to have a (1/1000) s time base.
  + Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
  + ClockDivision = 0
  + Counter direction = Up
  */
  htim10.Init.Period = 10000-1;
  htim10.Init.Prescaler = uwPrescalerValue;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  
  if(HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
	  _Error_Handler(__FILE__, __LINE__);
  }
}
#endif

/* TIM13 init function */
void MX_TIM13_Init(void)
{
  RCC_ClkInitTypeDef    clkconfig;
  uint32_t              uwTimclock;
  uint32_t              uwPrescalerValue;
  uint32_t              pFLatency;
  
  /*Configure the TIM13 IRQ priority */
  HAL_NVIC_SetPriority(TIM8_UP_TIM13_IRQn, 15, 0 ); 
  
  /* Enable the TIM13 global Interrupt */
  HAL_NVIC_EnableIRQ(TIM8_UP_TIM13_IRQn); 
  
  /* Enable TIM13 clock */
  __HAL_RCC_TIM13_CLK_ENABLE();
  
  /* Get clock configuration */
  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);
  
  /* Compute TIM13 clock */
  uwTimclock = HAL_RCC_GetPCLK1Freq( );
   
  /* Compute the prescaler value to have TIM13 counter clock equal to 1KHz */
  uwPrescalerValue = (uint32_t) ((uwTimclock / 1000) - 1);
  
//  EwPrint( "SYSCLKSource:%d, ClockType:%d, uwTimclock:%d\n", clkconfig.SYSCLKSource, clkconfig.ClockType, uwTimclock );
  
  /* Initialize TIM13 */
  htim13.Instance = TIM13;
  
  /* Initialize TIMx peripheral as follow:
  + Period = [(TIM14CLK/1000) - 1]. to have a (1/1000) s time base.
  + Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
  + ClockDivision = 0
  + Counter direction = Up
  */
  htim13.Init.Period = 1000-1;
  htim13.Init.Prescaler = uwPrescalerValue;
  htim13.Init.ClockDivision = TIM_CLOCKDIVISION_DIV4;
  htim13.Init.CounterMode = TIM_COUNTERMODE_UP;
  
  if(HAL_TIM_Base_Init(&htim13) != HAL_OK)
  {
	  _Error_Handler(__FILE__, __LINE__);
  }

}


/* TIM14 init function */
void MX_TIM14_Init(void)
{
  RCC_ClkInitTypeDef    clkconfig;
  uint32_t              uwTimclock;
  uint32_t              uwPrescalerValue;
  uint32_t              pFLatency;
  
  /*Configure the TIM14 IRQ priority */
  HAL_NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn, 10, 0 ); 
  
  /* Enable the TIM14 global Interrupt */
  HAL_NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn); 
  
  /* Enable TIM14 clock */
  __HAL_RCC_TIM14_CLK_ENABLE();
  
  /* Get clock configuration */
  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);
  
  /* Compute TIM14 clock */
  uwTimclock = HAL_RCC_GetPCLK1Freq( );
   
  /* Compute the prescaler value to have TIM14 counter clock equal to 1MHz */
  uwPrescalerValue = (uint32_t) ((2*uwTimclock / 100000) - 1);
  
//  EwPrint( "SYSCLKSource:%d, ClockType:%d, uwTimclock:%d\n", clkconfig.SYSCLKSource, clkconfig.ClockType, uwTimclock );
  
  /* Initialize TIM14 */
  htim14.Instance = TIM14;
  
  /* Initialize TIMx peripheral as follow:
  + Period = [(TIM14CLK/1000) - 1]. to have a (1/1000) s time base.
  + Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
  + ClockDivision = 0
  + Counter direction = Up
  */
  htim14.Init.Period = 15000;
  htim14.Init.Prescaler = uwPrescalerValue;
  htim14.Init.ClockDivision = 0;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  
  if(HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
	  _Error_Handler(__FILE__, __LINE__);
  }
}


void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{
 if(tim_baseHandle->Instance==TIM8)
  {
  /* USER CODE BEGIN TIM14_MspInit 0 */

  /* USER CODE END TIM14_MspInit 0 */
    /* TIM14 clock enable */
    __HAL_RCC_TIM8_CLK_ENABLE();
  /* USER CODE BEGIN TIM14_MspInit 1 */

  /* USER CODE END TIM14_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM14)
  {
  /* USER CODE BEGIN TIM14_MspInit 0 */

  /* USER CODE END TIM14_MspInit 0 */
    /* TIM14 clock enable */
    __HAL_RCC_TIM14_CLK_ENABLE();
  /* USER CODE BEGIN TIM14_MspInit 1 */

  /* USER CODE END TIM14_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM13)
  {
  /* USER CODE BEGIN TIM14_MspInit 0 */

  /* USER CODE END TIM14_MspInit 0 */
    /* TIM14 clock enable */
    __HAL_RCC_TIM13_CLK_ENABLE();
  /* USER CODE BEGIN TIM14_MspInit 1 */

  /* USER CODE END TIM14_MspInit 1 */
  }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{
  if(tim_baseHandle->Instance==TIM8)
  {
  /* USER CODE BEGIN TIM14_MspDeInit 0 */

  /* USER CODE END TIM14_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM8_CLK_DISABLE();
  /* USER CODE BEGIN TIM14_MspDeInit 1 */

  /* USER CODE END TIM14_MspDeInit 1 */
  }
  if(tim_baseHandle->Instance==TIM13)
  {
  /* USER CODE BEGIN TIM14_MspDeInit 0 */

  /* USER CODE END TIM14_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM13_CLK_DISABLE();
  /* USER CODE BEGIN TIM14_MspDeInit 1 */

  /* USER CODE END TIM14_MspDeInit 1 */
  }
  else  if(tim_baseHandle->Instance==TIM14)
  {
  /* USER CODE BEGIN TIM14_MspDeInit 0 */

  /* USER CODE END TIM14_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM14_CLK_DISABLE();
  /* USER CODE BEGIN TIM14_MspDeInit 1 */

  /* USER CODE END TIM14_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
