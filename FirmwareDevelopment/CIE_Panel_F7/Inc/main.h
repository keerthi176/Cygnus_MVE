/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define PWR_811_EN_Pin GPIO_PIN_3
#define PWR_811_EN_GPIO_Port GPIOE
#define WR_EN__Pin GPIO_PIN_8
#define WR_EN__GPIO_Port GPIOI
#define I2C2_EN_Pin GPIO_PIN_13
#define I2C2_EN_GPIO_Port GPIOC
#define I2C1_EN_Pin GPIO_PIN_11
#define I2C1_EN_GPIO_Port GPIOI
#define FIRE_ROUTING_Pin GPIO_PIN_2
#define FIRE_ROUTING_GPIO_Port GPIOC
#define USB_PWR_EN_Pin GPIO_PIN_3
#define USB_PWR_EN_GPIO_Port GPIOC
#define EXPANDER_IRQ_Pin GPIO_PIN_3
#define EXPANDER_IRQ_GPIO_Port GPIOA
#define USB_OVER_CURRENT_Pin GPIO_PIN_0
#define USB_OVER_CURRENT_GPIO_Port GPIOB
#define ADC0_SELECT_Pin GPIO_PIN_2
#define ADC0_SELECT_GPIO_Port GPIOB
#define I2C2_PWR_EN_Pin GPIO_PIN_15
#define I2C2_PWR_EN_GPIO_Port GPIOI
#define ADC1_SELECT_Pin GPIO_PIN_1
#define ADC1_SELECT_GPIO_Port GPIOJ
#define FIRE_LED_Pin GPIO_PIN_5
#define FIRE_LED_GPIO_Port GPIOJ
#define GEN_FAULT_LED_Pin GPIO_PIN_6
#define GEN_FAULT_LED_GPIO_Port GPIOH
#define SYS_FAULT_LED_Pin GPIO_PIN_7
#define SYS_FAULT_LED_GPIO_Port GPIOH
#define USB_HS_HOST_DM_Pin GPIO_PIN_14
#define USB_HS_HOST_DM_GPIO_Port GPIOB
#define USB_HS_HOST_DP_Pin GPIO_PIN_15
#define USB_HS_HOST_DP_GPIO_Port GPIOB
#define PWM_BACKLIGHT_Pin GPIO_PIN_13
#define PWM_BACKLIGHT_GPIO_Port GPIOD
#define GSM_Power_OFF_Pin GPIO_PIN_6
#define GSM_Power_OFF_GPIO_Port GPIOJ
#define EXPANSION2_Pin GPIO_PIN_7
#define EXPANSION2_GPIO_Port GPIOJ
#define EXPANSION1_Pin GPIO_PIN_8
#define EXPANSION1_GPIO_Port GPIOJ
#define ALARM_RELAY_Pin GPIO_PIN_3
#define ALARM_RELAY_GPIO_Port GPIOG
#define GSM_EN_Pin GPIO_PIN_6
#define GSM_EN_GPIO_Port GPIOG
#define GSM_RESET_Pin GPIO_PIN_7
#define GSM_RESET_GPIO_Port GPIOG
#define FAKE_CTS_Pin GPIO_PIN_9
#define FAKE_CTS_GPIO_Port GPIOC
#define FAKE_RTS_Pin GPIO_PIN_8
#define FAKE_RTS_GPIO_Port GPIOA
#define USB_FS_DEVICE_DM_Pin GPIO_PIN_11
#define USB_FS_DEVICE_DM_GPIO_Port GPIOA
#define USB_FS_DEVICE_DP_Pin GPIO_PIN_12
#define USB_FS_DEVICE_DP_GPIO_Port GPIOA
#define DISPLAY_EN_Pin GPIO_PIN_15
#define DISPLAY_EN_GPIO_Port GPIOA
#define FIRE_RELAY_Pin GPIO_PIN_12
#define FIRE_RELAY_GPIO_Port GPIOJ
#define FAULT_RELAY_Pin GPIO_PIN_13
#define FAULT_RELAY_GPIO_Port GPIOJ
#define BUZZER_EN_Pin GPIO_PIN_14
#define BUZZER_EN_GPIO_Port GPIOJ
#define SYS_FAULT_MCU_Pin GPIO_PIN_15
#define SYS_FAULT_MCU_GPIO_Port GPIOJ
#define SOUNDERS_RELAY_Pin GPIO_PIN_9
#define SOUNDERS_RELAY_GPIO_Port GPIOG
#define NCU_EN_Pin GPIO_PIN_10
#define NCU_EN_GPIO_Port GPIOG
#define TOUCH_IRQ_Pin GPIO_PIN_5
#define TOUCH_IRQ_GPIO_Port GPIOB

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */


#define IP_ADDR0   (uint8_t) 10
#define IP_ADDR1   (uint8_t) 40
#define IP_ADDR2   (uint8_t) 3
#define IP_ADDR3   (uint8_t) 250
   
/*NETMASK*/
#define NETMASK_ADDR0   (uint8_t) 255
#define NETMASK_ADDR1   (uint8_t) 255
#define NETMASK_ADDR2   (uint8_t) 0
#define NETMASK_ADDR3   (uint8_t) 0

/*Gateway Address*/
#define GW_ADDR0   (uint8_t) 10
#define GW_ADDR1   (uint8_t) 40
#define GW_ADDR2   (uint8_t) 1
#define GW_ADDR3   (uint8_t) 1 
   
	
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
