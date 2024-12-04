/**
  ******************************************************************************
  * @file    stm32f7xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  * COPYRIGHT(c) 2019 STMicroelectronics
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
#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"
#include "stm32f7xx_it.h"
#include "string.h"

#include <string.h>

#include "usbh_conf.h"
#include "DM_BatteryRam.h"


/* USER CODE BEGIN 0 */

extern unsigned int EmWiSystemTicks;

void ErrorOut(char *text);

	
	
extern void HAL_SPI_IRQHandler2( SPI_HandleTypeDef* );
#ifdef BD_TEST_ONLY
extern volatile uint32_t memoryTest;
#endif


/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
#ifndef BOOT_MENU
//extern ETH_HandleTypeDef heth;
#endif
//extern JPEG_HandleTypeDef    JPEG_Handle;
extern PCD_HandleTypeDef hpcd;
extern HCD_HandleTypeDef _hHCD[2];
extern DMA2D_HandleTypeDef hdma2d;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern JPEG_HandleTypeDef hjpeg;
extern LTDC_HandleTypeDef hltdc;
extern DMA_HandleTypeDef hdma_quadspi;
extern QSPI_HandleTypeDef hqspi;
extern RTC_HandleTypeDef hrtc;
extern SD_HandleTypeDef hsd1;
extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_uart4_rx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart3_tx;

extern DMA_HandleTypeDef hdma_usart6_tx;
extern DMA_HandleTypeDef hdma_usart6_rx;

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;
extern CAN_HandleTypeDef hcan1;
extern DMA_HandleTypeDef hdma_sdmmc1_rx;
extern DMA_HandleTypeDef hdma_sdmmc1_tx;

/******************************************************************************/
/*            Cortex-M7 Processor Interruption and Exception Handlers         */ 
/******************************************************************************/

/**
* @brief This function handles Non maskable interrupt.
*/
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */
 	HAL_RCC_NMI_IRQHandler( );
  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */

  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
* @brief This function handles Hard fault interrupt.
*/
 
void HardFault_Handler(void)
{
   ( (int*)BKPSRAM_BASE)[ BATT_RAM_WATCHDOG_REASON] = WD_REASON_HARD_FAULT;
    
   static volatile int x = 3;

   while ( x>0 );
}

  

/**
* @brief This function handles Memory management fault.
*/
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */
  
	( (int*)BKPSRAM_BASE)[ BATT_RAM_WATCHDOG_REASON] = WD_REASON_MEM_FAULT;
	 
}

/**
* @brief This function handles Pre-fetch fault, memory access fault.
*/
void BusFault_Handler(void)
{
   ( (int*)BKPSRAM_BASE)[ BATT_RAM_WATCHDOG_REASON] = WD_REASON_BUS_FAULT;
  /* USER CODE END BusFault_IRQn 1 */
	while (1);
  
}

/**
* @brief This function handles Undefined instruction or illegal state.
*/
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */
   ( (int*)BKPSRAM_BASE)[ BATT_RAM_WATCHDOG_REASON] = WD_REASON_USAGE_FAULT;
  /* USER CODE END UsageFault_IRQn 0 */
  while (1);
 
  /* USER CODE BEGIN UsageFault_IRQn 1 */

  /* USER CODE END UsageFault_IRQn 1 */
}

/**
* @brief This function handles System service call via SWI instruction.
*/
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
* @brief This function handles Debug monitor.
*/
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
* @brief This function handles Pendable request for system service.
*/
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
* @brief This function handles System tick timer.
*/
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
 // HAL_SYSTICK_IRQHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */
#ifndef BD_TEST_ONLY		
	EmWiSystemTicks++;
#endif		
	
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f7xx.s).                    */
/******************************************************************************/


void CAN1_TX_IRQHandler( void )
{
	HAL_CAN_IRQHandler( &hcan1 );
}

void CAN1_RX0_IRQHandler( void )
{
	HAL_CAN_IRQHandler( &hcan1 );
}	

void CAN1_RX1_IRQHandler( void )
{
	HAL_CAN_IRQHandler( &hcan1 );
}

void CAN1_SCE_IRQHandler( void )
{
	HAL_CAN_IRQHandler( &hcan1 );
}


/*************************************************************************/
/**  \fn      void USART4_IRQHandler(void)
***  \brief   IRQ Handler
**************************************************************************/
 
void UART4_IRQHandler( void )
{
   HAL_UART_IRQHandler( &huart4 );
}
 



/**
* @brief This function handles RCC global interrupt.
*/
void RCC_IRQHandler(void)
{
  /* USER CODE BEGIN RCC_IRQn 0 */

  /* USER CODE END RCC_IRQn 0 */
  /* USER CODE BEGIN RCC_IRQn 1 */

  /* USER CODE END RCC_IRQn 1 */
}

/**
* @brief This function handles DMA1 stream2 global interrupt.
*/
void DMA1_Stream2_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream2_IRQn 0 */

  /* USER CODE END DMA1_Stream2_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_uart4_rx);
  /* USER CODE BEGIN DMA1_Stream2_IRQn 1 */

  /* USER CODE END DMA1_Stream2_IRQn 1 */
}

/**
* @brief This function handles DMA1 stream5 global interrupt.
*/
void DMA1_Stream5_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream5_IRQn 0 */

  /* USER CODE END DMA1_Stream5_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart2_rx);
  /* USER CODE BEGIN DMA1_Stream5_IRQn 1 */

  /* USER CODE END DMA1_Stream5_IRQn 1 */
}


/**
* @brief This function handles DMA1 stream6 global interrupt.
*/
void DMA1_Stream6_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream6_IRQn 0 */

  /* USER CODE END DMA1_Stream6_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart2_tx);
  /* USER CODE BEGIN DMA1_Stream6_IRQn 1 */

  /* USER CODE END DMA1_Stream6_IRQn 1 */
}

void DMA2_Stream1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream6_IRQn 0 */

  /* USER CODE END DMA2_Stream6_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart6_rx);
  /* USER CODE BEGIN DMA2_Stream6_IRQn 1 */

  /* USER CODE END DMA2_Stream6_IRQn 1 */
}

void DMA2_Stream7_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream6_IRQn 0 */

  /* USER CODE END DMA2_Stream6_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart6_tx);
  /* USER CODE BEGIN DMA2_Stream6_IRQn 1 */

  /* USER CODE END DMA2_Stream6_IRQn 1 */
}

 

void DMA2_Stream6_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream6_IRQn 0 */

  /* USER CODE END DMA2_Stream6_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_sdmmc1_tx);
	
  /* USER CODE BEGIN DMA2_Stream6_IRQn 1 */

  /* USER CODE END DMA2_Stream6_IRQn 1 */
}

/**
  * @brief This function handles DMA2 stream3 global interrupt.
  */
void DMA2_Stream3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream3_IRQn 0 */

  /* USER CODE END DMA2_Stream3_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_sdmmc1_rx);
  /* USER CODE BEGIN DMA2_Stream3_IRQn 1 */

  /* USER CODE END DMA2_Stream3_IRQn 1 */
}
 
void DMA1_Stream1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream1_IRQn 0 */

  /* USER CODE END DMA1_Stream1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart3_rx);
  /* USER CODE BEGIN DMA1_Stream1_IRQn 1 */

  /* USER CODE END DMA1_Stream1_IRQn 1 */
}

void DMA1_Stream3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream3_IRQn 0 */

  /* USER CODE END DMA1_Stream3_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart3_tx);
  /* USER CODE BEGIN DMA1_Stream3_IRQn 1 */

  /* USER CODE END DMA1_Stream3_IRQn 1 */
}

/**
* @brief This function handles I2C1 event interrupt.
*/
void I2C1_EV_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_EV_IRQn 0 */

  /* USER CODE END I2C1_EV_IRQn 0 */
  HAL_I2C_EV_IRQHandler(&hi2c1);
  /* USER CODE BEGIN I2C1_EV_IRQn 1 */

  /* USER CODE END I2C1_EV_IRQn 1 */
}

/**
* @brief This function handles I2C1 error interrupt.
*/
void I2C1_ER_IRQHandler(void)
{
  /* USER CODE BEGIN I2C1_ER_IRQn 0 */

  /* USER CODE END I2C1_ER_IRQn 0 */
  HAL_I2C_ER_IRQHandler(&hi2c1);
  /* USER CODE BEGIN I2C1_ER_IRQn 1 */

  /* USER CODE END I2C1_ER_IRQn 1 */
}

/**
* @brief This function handles I2C2 event interrupt.
*/
void I2C2_EV_IRQHandler(void)
{
  /* USER CODE BEGIN I2C2_EV_IRQn 0 */

  /* USER CODE END I2C2_EV_IRQn 0 */
  HAL_I2C_EV_IRQHandler(&hi2c2);
  /* USER CODE BEGIN I2C2_EV_IRQn 1 */

  /* USER CODE END I2C2_EV_IRQn 1 */
}

/**
* @brief This function handles I2C2 error interrupt.
*/
void I2C2_ER_IRQHandler(void)
{
  /* USER CODE BEGIN I2C2_ER_IRQn 0 */

  /* USER CODE END I2C2_ER_IRQn 0 */
  HAL_I2C_ER_IRQHandler(&hi2c2);
  /* USER CODE BEGIN I2C2_ER_IRQn 1 */

  /* USER CODE END I2C2_ER_IRQn 1 */
}

/**
* @brief This function handles SPI1 global interrupt.
*/
void SPI1_IRQHandler(void)
{
  /* USER CODE BEGIN SPI1_IRQn 0 */

  /* USER CODE END SPI1_IRQn 0 */
  HAL_SPI_IRQHandler(&hspi1);
  /* USER CODE BEGIN SPI1_IRQn 1 */

#ifndef BOOT_MENU
  HAL_SPI_IRQHandler2(&hspi1);	
#endif
	
  /* USER CODE END SPI1_IRQn 1 */
}
 
 
/**
* @brief This function handles RTC alarms (A and B) interrupt through EXTI line 17.
*/
void RTC_Alarm_IRQHandler(void)
{
  /* USER CODE BEGIN RTC_Alarm_IRQn 0 */

  /* USER CODE END RTC_Alarm_IRQn 0 */
  HAL_RTC_AlarmIRQHandler(&hrtc);
  /* USER CODE BEGIN RTC_Alarm_IRQn 1 */

  /* USER CODE END RTC_Alarm_IRQn 1 */
}

/**
* @brief This function handles SDMMC1 global interrupt.
*/
void SDMMC1_IRQHandler(void)
{
  /* USER CODE BEGIN SDMMC1_IRQn 0 */

  /* USER CODE END SDMMC1_IRQn 0 */
  HAL_SD_IRQHandler(&hsd1);
  /* USER CODE BEGIN SDMMC1_IRQn 1 */

  /* USER CODE END SDMMC1_IRQn 1 */
}


/**
* @brief This function handles Ethernet global interrupt.
*/
void ETH_IRQHandler(void)
{
  /* USER CODE BEGIN ETH_IRQn 0 */

  /* USER CODE END ETH_IRQn 0 */
#ifndef BOOT_MENU	
 //  HAL_ETH_IRQHandler(&heth);
#endif	
  /* USER CODE BEGIN ETH_IRQn 1 */

  /* USER CODE END ETH_IRQn 1 */
}

/**
* @brief This function handles USB On The Go FS global interrupt.
*/
void OTG_FS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_FS_IRQn 0 */

  /* USER CODE END OTG_FS_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd);
  /* USER CODE BEGIN OTG_FS_IRQn 1 */

  /* USER CODE END OTG_FS_IRQn 1 */
}

/**
* @brief This function handles DMA2 stream2 global interrupt.
*/
void DMA2_Stream2_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream2_IRQn 0 */

  /* USER CODE END DMA2_Stream2_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_quadspi);
  /* USER CODE BEGIN DMA2_Stream2_IRQn 1 */

  /* USER CODE END DMA2_Stream2_IRQn 1 */
}
 
/**
* @brief This function handles USB On The Go HS End Point 1 Out global interrupt.
*/
#if 0
void OTG_HS_EP1_OUT_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_HS_EP1_OUT_IRQn 0 */

  /* USER CODE END OTG_HS_EP1_OUT_IRQn 0 */
  HAL_HCD_IRQHandler(&_hHCD[ID_USB_HOST_HS]);
  /* USER CODE BEGIN OTG_HS_EP1_OUT_IRQn 1 */

  /* USER CODE END OTG_HS_EP1_OUT_IRQn 1 */
}

/**
* @brief This function handles USB On The Go HS End Point 1 In global interrupt.
*/
void OTG_HS_EP1_IN_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_HS_EP1_IN_IRQn 0 */

  /* USER CODE END OTG_HS_EP1_IN_IRQn 0 */
  HAL_HCD_IRQHandler(&_hHCD[ID_USB_HOST_HS]);
  /* USER CODE BEGIN OTG_HS_EP1_IN_IRQn 1 */

  /* USER CODE END OTG_HS_EP1_IN_IRQn 1 */
}
#endif

/**
* @brief This function handles USB On The Go HS global interrupt.
*/
void OTG_HS_IRQHandler(void)
{
  /* USER CODE BEGIN OTG_HS_IRQn 0 */

  /* USER CODE END OTG_HS_IRQn 0 */
  HAL_HCD_IRQHandler( &_hHCD[ID_USB_HOST_HS] );
  /* USER CODE BEGIN OTG_HS_IRQn 1 */

  /* USER CODE END OTG_HS_IRQn 1 */
}
 

#if 0 
/**
* @brief This function handles LTDC global interrupt.
*/
void LTDC_IRQHandler(void)
{
  /* USER CODE BEGIN LTDC_IRQn 0 */

  /* USER CODE END LTDC_IRQn 0 */
  HAL_LTDC_IRQHandler(&hltdc);
  /* USER CODE BEGIN LTDC_IRQn 1 */

  /* USER CODE END LTDC_IRQn 1 */
}
#endif

/**
* @brief This function handles LTDC global error interrupt.
*/
void LTDC_ER_IRQHandler(void)
{
  /* USER CODE BEGIN LTDC_ER_IRQn 0 */

  /* USER CODE END LTDC_ER_IRQn 0 */
  HAL_LTDC_IRQHandler(&hltdc);
  /* USER CODE BEGIN LTDC_ER_IRQn 1 */

  /* USER CODE END LTDC_ER_IRQn 1 */
}
 

/**
* @brief This function handles QUADSPI global interrupt.
*/
void QUADSPI_IRQHandler(void)
{
  /* USER CODE BEGIN QUADSPI_IRQn 0 */

  /* USER CODE END QUADSPI_IRQn 0 */
  HAL_QSPI_IRQHandler(&hqspi);
  /* USER CODE BEGIN QUADSPI_IRQn 1 */

  /* USER CODE END QUADSPI_IRQn 1 */
}

/**
* @brief This function handles JPEG global interrupt.
*/
 
/*
void JPEG_IRQHandler(void)
{
  HAL_JPEG_IRQHandler(&JPEG_Handle);
}

void DMA2_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler(JPEG_Handle.hdmain);
}

void DMA2_Stream1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(JPEG_Handle.hdmaout);
}
*/


__weak void GPIO_15_Handler( ) { }
__weak void GPIO_14_Handler( ) { }
__weak void GPIO_13_Handler( ) { }
__weak void GPIO_12_Handler( ) { }
__weak void GPIO_11_Handler( ) { }
__weak void GPIO_10_Handler( ) { }
__weak void GPIO_9_Handler( )  { }
__weak void GPIO_8_Handler( )  { }
__weak void GPIO_7_Handler( )  { }
__weak void GPIO_6_Handler( )  { }
__weak void GPIO_5_Handler( )  { }
__weak void GPIO_4_Handler( )  { }
__weak void GPIO_3_Handler( )  { }
__weak void GPIO_2_Handler( )  { }
__weak void GPIO_1_Handler( )  { }
__weak void GPIO_0_Handler( )  { }


/**
  * @brief  This function handles external lines 5 to 9 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler( void )
{
	// Is it the STMPE8
	if ( __HAL_GPIO_EXTI_GET_IT( GPIO_PIN_15 ) )
	{
		__HAL_GPIO_EXTI_CLEAR_IT( GPIO_PIN_15 );
		GPIO_15_Handler( );
	}
}


/**
  * @brief  This function handles external lines 5 to 9 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI9_5_IRQHandler(void)
{
	// Is it the MXF
	if ( __HAL_GPIO_EXTI_GET_IT( GPIO_PIN_8 ) )
	{
		__HAL_GPIO_EXTI_CLEAR_IT( GPIO_PIN_8 );
		GPIO_8_Handler( );
	}
	else if ( __HAL_GPIO_EXTI_GET_IT( GPIO_PIN_7 ) )
	{
		__HAL_GPIO_EXTI_CLEAR_IT( GPIO_PIN_7 );
		GPIO_7_Handler( );
	}
	else if ( __HAL_GPIO_EXTI_GET_IT( GPIO_PIN_6 ) )
	{
		__HAL_GPIO_EXTI_CLEAR_IT( GPIO_PIN_6 );
		GPIO_6_Handler( );
	}
	else if ( __HAL_GPIO_EXTI_GET_IT( GPIO_PIN_5 ) )
	{
		__HAL_GPIO_EXTI_CLEAR_IT( GPIO_PIN_5 );
		GPIO_5_Handler( );
	}
}


void PVD_IRQHandler( void )
{
	HAL_PWR_PVD_IRQHandler( );
}


/**
  * @brief  This function handles external line 4 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI4_IRQHandler(void)
{
	__HAL_GPIO_EXTI_CLEAR_IT( GPIO_PIN_4 );
	GPIO_4_Handler( );	
}
	
	
/**
  * @brief  This function handles external line 3 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI3_IRQHandler(void)
{
	__HAL_GPIO_EXTI_CLEAR_IT( GPIO_PIN_3 );
	GPIO_3_Handler( );	
}


/**
  * @brief  This function handles external line 2 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI2_IRQHandler(void)
{
	__HAL_GPIO_EXTI_CLEAR_IT( GPIO_PIN_2 );
	GPIO_2_Handler( );	
}


/**
  * @brief  This function handles external line 1 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI1_IRQHandler(void)
{
	__HAL_GPIO_EXTI_CLEAR_IT( GPIO_PIN_1 );
	GPIO_1_Handler( );	
}


/**
  * @brief  This function handles external line 1 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void)
{
	__HAL_GPIO_EXTI_CLEAR_IT( GPIO_PIN_0 );
}
 



void ErrorOut(char *text)
{
	uint16_t len = strlen(text);
	HAL_UART_Transmit(&huart7, (uint8_t*) text, len, 1000);
}

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
