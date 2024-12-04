
/***************************************************************************
* File name: MM_Init.cpp
* Project name: CIE_Panel
*
* Copyright 2018 Bull Products Ltd as an unpublished work.
* All Rights Reserved.
*
* The information contained herein is confidential property of Bull
* Products Ltd. The use, copying, transfer or disclosure of such
* information is prohibited except by express written agreement with Bull
* Products Ltd.
*
* First written on 18/05/18 by Matt Ault
*
* File History:
* Change History:
* Rev Date Description Author
* 1.0 18/05/18 Initial File
*
* Description:
* MCU Initialisation
*
**************************************************************************/

#include "MM_Init.h"

int __rcc_csr = 0;

extern "C" void db( const char* fmt, ... );

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
	db( "Error: %s line %d\n", file, line ); 
  /* USER CODE END Error_Handler_Debug */
}


static int RTC_Kill( void )
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
	
	if ( HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK )
	{
		_Error_Handler(__FILE__, __LINE__);
	}
	return 0;
}

	
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config( int osc, int drive )
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

	/**Configure LSE Drive Capability 
	*/
	__HAL_RCC_LSEDRIVE_CONFIG( drive );

	/**Configure the main internal regulator output voltage 
	*/
	__HAL_RCC_PWR_CLK_ENABLE();

	__HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

	/**Initializes the CPU, AHB and APB busses clocks 
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|osc;

	RCC_OscInitStruct.HSEState = osc == RCC_OSCILLATORTYPE_HSE ? RCC_HSE_ON : RCC_HSE_OFF;
	RCC_OscInitStruct.HSIState = osc == RCC_OSCILLATORTYPE_HSE ? RCC_HSI_OFF : RCC_HSI_ON; 
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = osc == RCC_OSCILLATORTYPE_HSE ? RCC_PLLSOURCE_HSE : RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = osc == RCC_OSCILLATORTYPE_HSE ? 25 : 16;
	RCC_OscInitStruct.PLL.PLLN = 336;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	RCC_OscInitStruct.PLL.PLLR = 7;

	if ( HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK )
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Activate the Over-Drive mode 
	*/
	if (HAL_PWREx_EnableOverDrive() != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Initializes the CPU, AHB and APB busses clocks 
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
									|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;

	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC
									|RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
									|RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_USART6
									|RCC_PERIPHCLK_UART4|RCC_PERIPHCLK_UART7
									|RCC_PERIPHCLK_LPTIM1|RCC_PERIPHCLK_I2C1
									|RCC_PERIPHCLK_I2C2|RCC_PERIPHCLK_SDMMC1
									|RCC_PERIPHCLK_CLK48;
#if VGA_MODE==0	
	PeriphClkInitStruct.PLLSAI.PLLSAIN = 210; // //210;
#else 	
	 PeriphClkInitStruct.PLLSAI.PLLSAIN = 240;
#endif
	
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 3;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
	
	PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV4;
	PeriphClkInitStruct.PLLSAIDivQ = 1;
	PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
	PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
	PeriphClkInitStruct.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
	PeriphClkInitStruct.Usart6ClockSelection = RCC_USART6CLKSOURCE_PCLK2;
	PeriphClkInitStruct.Uart7ClockSelection = RCC_UART7CLKSOURCE_PCLK1;
	PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
	PeriphClkInitStruct.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
	PeriphClkInitStruct.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_PCLK;
	PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
	PeriphClkInitStruct.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_CLK48;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Configure the Systick interrupt time 
	*/
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	/**Configure the Systick 
	*/
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

void System_Low( void )
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
  RCC_OscInitStruct.PLL.PLLSource = 
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
 
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV8;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  
  
  // Activate the Over-Drive mode 
  HAL_PWREx_DisableOverDrive();
 
//  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
 //   Error_Handler();
  }
  
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC|RCC_PERIPHCLK_RTC
                              |RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_USART6
                              |RCC_PERIPHCLK_UART4|RCC_PERIPHCLK_UART5
                              |RCC_PERIPHCLK_UART7|RCC_PERIPHCLK_LPTIM1
                              |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_I2C2
                              |RCC_PERIPHCLK_SDMMC1|RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 4;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV4;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Uart5ClockSelection = RCC_UART5CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Usart6ClockSelection = RCC_USART6CLKSOURCE_PCLK2;
  PeriphClkInitStruct.Uart7ClockSelection = RCC_UART7CLKSOURCE_PCLK1;
  PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInitStruct.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_PCLK;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLLSAIP;
  PeriphClkInitStruct.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}
 

Init::Init( )
{
	FMC_SDRAM_CommandTypeDef command;
	
	__rcc_csr = RCC->CSR;
	/* USER CODE BEGIN 1 */
	
	MX_IWDG_Init( );
 
	/* Enable I-Cache-------------------------------------------------------------*/
	SCB_EnableICache( );

	/* MCU Configuration----------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */
 
	// Kill 32k xtal
	RTC_Kill();

	/* USER CODE END Init */
 	LL_RCC_PLL_ConfigSpreadSpectrum( 25, 1245, LL_RCC_SPREAD_SELECT_CENTER );
 	LL_RCC_PLL_SpreadSpectrum_Enable( );
	
	/* Configure the system clock */
	SystemClock_Config( RCC_OSCILLATORTYPE_HSE, RCC_LSEDRIVE_HIGH );

	/* USER CODE BEGIN SysInit */
			
	HAL_PWREx_EnableBkUpReg( );
		
	__HAL_RCC_BKPSRAM_CLK_ENABLE();
		
	// Enable clock security 
 	HAL_RCC_EnableCSS( );
	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	
	HAL_IWDG_Refresh( &hiwdg );	

	MX_GPIO_Init();
	MX_DMA_Init();
	//  MX_CAN1_Init();
	//  MX_DAC_Init();
//	MX_DMA2D_Init();
	MX_FMC_Init();  

	MX_I2C1_Init();
	MX_I2C2_Init();

	MX_JPEG_Init();

	MX_LTDC_Init();   
	MX_QUADSPI_Init();
	MX_RTC_Init();
	MX_SDMMC1_SD_Init();	
	MX_FATFS_Init();
 	
	MX_SPI1_Init();
//	MX_SPI4_Init();
	MX_UART4_Init();
	MX_UART5_Init();	
	MX_UART7_Init();
	
	MX_USART1_UART_Init();
	MX_USART2_UART_Init();
	MX_USART3_UART_Init();

	MX_USART6_UART_Init();

	//  MX_LWIP_Init();
	
 
 	MX_USB_DEVICE_Init();
 

	MX_CAN1_Init( );

 	MX_USB_HOST_Init();
	MX_CRC_Init();
	
 //	MX_TIM6_Init();
 	MX_TIM7_Init();
	MX_TIM8_Init();
//	MX_TIM10_Init();
  	MX_TIM13_Init();
 	MX_TIM14_Init();
	
	HAL_IWDG_Refresh( &hiwdg );	
 
 	SDRAM_Initialization_Sequence( &hsdram1, &command );
}
