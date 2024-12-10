/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
  /* Includes ------------------------------------------------------------------*/

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
//#define TP_1_Pin GPIO_PIN_8
//#define TP_1_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
//#define START_SYNC_Pin GPIO_PIN_12
//#define START_SYNC_GPIO_Port GPIOC
#define GPIO_RGB_RED GPIO_PIN_11
#define GPIO_RGB_GREEN GPIO_PIN_10
#define GPIO_RGB_BLUE GPIO_PIN_15
#define USART1_TX_Pin GPIO_PIN_9
#define USART1_RX_Pin GPIO_PIN_10
#define LPUART1_TX_Pin GPIO_PIN_1
#define LPUART1_RX_Pin GPIO_PIN_0
#define HEAD_WAKE_UP_Pin GPIO_PIN_0
#define HEAD_WAKE_UP_GPIO_Port GPIOC
#define HEAD_TX_Pin GPIO_PIN_1
#define HEAD_TX_GPIO_Port GPIOC
#define USART4_TX_Pin GPIO_PIN_10
#define USART4_RX_Pin GPIO_PIN_11
#define USART2_TX_Pin GPIO_PIN_2
#define USART2_RX_Pin GPIO_PIN_3

// Task Id
//#define TASK_NULL             0
#define TASK_TDM             	0
#define TASK_SYNC_TX				1
// Number of Tasks
#define NUM_OF_TASKS          2


/* Public functions prototypes
*******************************************************************************/
void RTCClock_Config(void);
void Error_Handler(char*);


/**
  * @}
  */ 

/**
  * @}
*/ 

#endif /* __MAIN_H */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
