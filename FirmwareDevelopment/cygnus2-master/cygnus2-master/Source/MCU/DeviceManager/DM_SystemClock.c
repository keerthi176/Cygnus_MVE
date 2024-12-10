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
*  File         : DM_SystemClock.c
*
*  Description  : System Clock Device Manager of the STM32L4
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/
#include "stm32l4xx_hal.h"
#include "CO_ErrorCode.h"
#include "CO_Defines.h"
#include "DM_SystemClock.h"
#include "board.h"
#include "DM_LED.h"
#include "cmsis_os.h"

/* Private Functions Prototypes
*************************************************************************************/
static void SystemClock_Config(void);

/* Global Variables
*************************************************************************************/


/* Private Variables
*************************************************************************************/


/*************************************************************************************/
/**
* DM_SystemClockResetConfig
* Complete System clock configuration - Post reset
*
* @param - void
*
* @return - ErrorCode_t - status 0=success else error
*/
ErrorCode_t DM_SystemClockResetConfig(void)
{
   /* Configure the complete System clock */
   SystemClock_Config();

   /* Enable Power Clock */
   __HAL_RCC_PWR_CLK_ENABLE();

   /* Ensure that HSI is wake-up system clock */ 
   __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);

   #if _DEBUG
   /* Freeze the LPTIM timer in debug mode */
   __HAL_DBGMCU_FREEZE_LPTIM1();
   #endif
   
   return SUCCESS_E;
}

/*************************************************************************************/
/**
* DM_SystemClockStopConfig
* Configure the System clock to prepare for the stop mode
*
* @param - void
*
* @return - ErrorCode_t - status 0=success else error
*/
ErrorCode_t DM_SystemClockStopConfig(void)
{
   RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
   RCC_OscInitTypeDef RCC_OscInitStruct = {0};
   uint32_t pFLatency = 0;

   /* Get the Clocks configuration according to the internal RCC registers */
   HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);

   if (RCC_ClkInitStruct.SYSCLKSource == RCC_SYSCLKSOURCE_PLLCLK)
   {
      /* Select the HSI as system clock source */
      RCC_ClkInitStruct.ClockType     = RCC_CLOCKTYPE_SYSCLK;
      RCC_ClkInitStruct.SYSCLKSource  = RCC_SYSCLKSOURCE_HSI;
      if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
      {
         Error_Handler("FATAL ERROR: HAL_RCC_ClockConfig() in DM_SystemClockStopConfig() FAILED");
      }
   }

   /* Get the Oscillators configuration according to the internal RCC registers */
   HAL_RCC_GetOscConfig(&RCC_OscInitStruct);
   
   if (RCC_OscInitStruct.PLL.PLLState == RCC_PLL_ON)
   {
      /* Enable the PLL */
      RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
      RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;
      if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
      {
         Error_Handler("FATAL ERROR: HAL_RCC_OscConfig() in DM_SystemClockStopConfig() FAILED");
      }
   }

   return SUCCESS_E;
}


/*************************************************************************************/
/**
* DM_SystemClockStopConfig
* Reconfigure the System clock when leaving the stop mode
*
* @param - void
*
* @return - ErrorCode_t - status 0=success else error
*/
ErrorCode_t DM_SystemClockWakeUpConfig(void)
{
   RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
   RCC_OscInitTypeDef RCC_OscInitStruct = {0};
   uint32_t pFLatency = 0;

   /* Enable Power Control clock */
   __HAL_RCC_PWR_CLK_ENABLE();

   /* Get the Oscillators configuration according to the internal RCC registers */
   HAL_RCC_GetOscConfig(&RCC_OscInitStruct);

   if (RCC_OscInitStruct.PLL.PLLState != RCC_PLL_ON)
   {
      /* Enable PLL */
      RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
      RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
      RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
      if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
      {
         Error_Handler("FATAL ERROR: HAL_RCC_OscConfig() in DM_SystemClockWakeUpConfig() FAILED");
      }
   }
   /* Get the Clocks configuration according to the internal RCC registers */
   HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);

   if (RCC_ClkInitStruct.SYSCLKSource != RCC_SYSCLKSOURCE_PLLCLK)
   {
      /* Select PLL as system clock source and keep HCLK, PCLK1 and PCLK2 clocks dividers as before */
      RCC_ClkInitStruct.ClockType     = RCC_CLOCKTYPE_SYSCLK;
      RCC_ClkInitStruct.SYSCLKSource  = RCC_SYSCLKSOURCE_PLLCLK;
      if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
      {
         Error_Handler("FATAL ERROR: HAL_RCC_ClockConfig() in DM_SystemClockWakeUpConfig() FAILED");
      }
   }
   
   return SUCCESS_E;
}

/*************************************************************************************/
/**
* SystemClock_Config
* Configure the complete System clock
*
* @param - void
*
* @return - ErrorCode_t - status 0=success else error
*/
static void SystemClock_Config(void)
{
   RCC_OscInitTypeDef RCC_OscInitStruct;
   RCC_ClkInitTypeDef RCC_ClkInitStruct;
   RCC_PeriphCLKInitTypeDef PeriphClkInit;

   /* Configure LSE Drive Capability */
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_MEDIUMLOW);

   /* Configure the Systick interrupt time */
   HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

   /* Configure the Systick prior to the PLL activation */
   HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

   RCC_OscInitStruct.OscillatorType  = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_LSE;
   RCC_OscInitStruct.MSIState = RCC_MSI_OFF;
   RCC_OscInitStruct.LSEState = RCC_LSE_BYPASS;
   RCC_OscInitStruct.LSIState = RCC_LSI_OFF;
   RCC_OscInitStruct.HSIState = RCC_HSI_ON;
   RCC_OscInitStruct.HSICalibrationValue = (uint32_t)((RCC->ICSCR & RCC_ICSCR_HSITRIM) >> RCC_ICSCR_HSITRIM_Pos);
   RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
   RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
   RCC_OscInitStruct.PLL.PLLM = 1; //1;
   RCC_OscInitStruct.PLL.PLLN = 10;
   RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
   RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
   RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;

   if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
   {
		GpioInit( &StatusLedRed, CN_STATUS_LED_RED, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 1 );
		GpioInit( &StatusLedBlue, CN_STATUS_LED_BLUE, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 1 );
		osDelay(2000);
      Error_Handler("FATAL ERROR: HAL_RCC_OscConfig() in SystemClock_Config() FAILED");
   }

   /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
   RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
   RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
   RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
   RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2; //RCC_HCLK_DIV1;  
   RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;  

   if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
   {
      Error_Handler("FATAL ERROR: HAL_RCC_ClockConfig() in SystemClock_Config() FAILED");
   }

   /**Reconfigure the Systick interrupt time as we switched to the PLL as a source of HCLK
   */
   HAL_NVIC_DisableIRQ(SysTick_IRQn);

   HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

   /**Configure the Systick 
   */
   HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
   
   HAL_NVIC_EnableIRQ(SysTick_IRQn);

   PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_USART3
                                       |RCC_PERIPHCLK_UART4|RCC_PERIPHCLK_LPUART1
                                       |RCC_PERIPHCLK_LPTIM1|RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_ADC;
   PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_HSI;
   PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_HSI;
   PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_HSI;
   PeriphClkInit.Uart4ClockSelection = RCC_UART4CLKSOURCE_SYSCLK;
   PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_HSI;
   PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
   PeriphClkInit.I2c2ClockSelection = RCC_I2C2CLKSOURCE_HSI;
   PeriphClkInit.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_LSE;
   PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;
   if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
   {
      Error_Handler("FATAL ERROR: HAL_RCCEx_PeriphCLKConfig() in SystemClock_Config() FAILED");
   }

   /**Configure the main internal regulator output voltage 
   */
   if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
   {
      Error_Handler("FATAL ERROR: HAL_PWREx_ControlVoltageScaling() in SystemClock_Config() FAILED");
   }
}
