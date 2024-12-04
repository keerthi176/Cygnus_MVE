/**
  ******************************************************************************
  * @file    pca9535e.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    10-February-2015
  * @brief   This file contains all the functions prototypes for the
  *          pca9535e.c IO expander driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
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
#ifndef __PCA9535E_H
#define __PCA9535E_H

#ifdef __cplusplus
 extern "C" {
#endif   
   
/* Includes ------------------------------------------------------------------*/
#include "io.h"

/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup Component
  * @{
  */
    
/** @defgroup PCA9535E
  * @{
  */    

/* Exported types ------------------------------------------------------------*/

/** @defgroup PCA9535E_Exported_Types
  * @{
  */ 

/* Exported constants --------------------------------------------------------*/
  
/** @defgroup PCA9535E_Exported_Constants
  * @{
  */ 

/** 
  * @brief PCA9535E chip IDs  
  */ 
#define PCA9535E_ID                     0x6969

  
#define PCA9535E_REG_CONFIG		6
#define PCA9535E_REG_INPUT			0
#define PCA9535E_REG_OUTPUT		2
#define PCA9535E_REG_POLARITY		4


   
/** 
  * @brief  IO Pins direction  
  */
#define PCA9535E_DIRECTION_IN           0x00
#define PCA9535E_DIRECTION_OUT          0x01

/** 
  * @brief  IO IT polarity  
  */
#define PCA9535E_POLARITY_LOW           0x00
#define PCA9535E_POLARITY_HIGH          0x01

/** 
  * @brief  IO Pins  
  */     
#define PCA9535E_PIN_0                  0x0001
#define PCA9535E_PIN_1                  0x0002
#define PCA9535E_PIN_2                  0x0004
#define PCA9535E_PIN_3                  0x0008
#define PCA9535E_PIN_4                  0x0010
#define PCA9535E_PIN_5                  0x0020
#define PCA9535E_PIN_6                  0x0040
#define PCA9535E_PIN_7                  0x0080
#define PCA9535E_PIN_8                  0x0100
#define PCA9535E_PIN_9                  0x0200
#define PCA9535E_PIN_10                 0x0400
#define PCA9535E_PIN_11                 0x0800
#define PCA9535E_PIN_12                 0x1000
#define PCA9535E_PIN_13                 0x2000
#define PCA9535E_PIN_14                 0x4000
#define PCA9535E_PIN_15                 0x8000
#define PCA9535E_PIN_ALL                0xFFFF    

/**
  * @}
  */ 
  
/* Exported macro ------------------------------------------------------------*/
  
/** @defgroup PCA9535E_Exported_Macros
  * @{
  */ 

/* Exported functions --------------------------------------------------------*/
  
/** @defgroup PCA9535E_Exported_Functions
  * @{
  */
  
/** 
  * @brief PCA9535E Control functions
  */
void     pca9535e_Init(uint16_t DeviceAddr);
void     pca9535e_Reset(uint16_t DeviceAddr);
uint16_t pca9535e_ReadID(uint16_t DeviceAddr);
void     pca9535e_SetITPolarity(uint16_t DeviceAddr, uint8_t Polarity);
void     pca9535e_EnableGlobalIT(uint16_t DeviceAddr);
void     pca9535e_DisableGlobalIT(uint16_t DeviceAddr);

/** 
  * @brief PCA9535E IO functionalities functions
  */
void     pca9535e_IO_InitPin(uint16_t DeviceAddr, uint32_t IO_Pin, uint8_t Direction);
uint8_t  pca9535e_IO_Config(uint16_t DeviceAddr, uint32_t IO_Pin, IO_ModeTypedef IO_Mode);
void     pca9535e_IO_PolarityInv_Enable(uint16_t DeviceAddr, uint32_t IO_Pin);
void     pca9535e_IO_PolarityInv_Disable(uint16_t DeviceAddr, uint32_t IO_Pin);
int      pca9535e_IO_WritePin(uint16_t DeviceAddr, uint32_t IO_Pin, uint8_t PinState);
uint32_t pca9535e_IO_ReadPin(uint16_t DeviceAddr, uint32_t IO_Pin);
uint8_t  pca9535e_I2CIOExp_ReadPin(uint16_t DeviceAddr, uint8_t IO_Pin);
void     pca9535e_IO_EnablePinIT(uint16_t DeviceAddr, uint32_t IO_Pin);
void     pca9535e_IO_DisablePinIT(uint16_t DeviceAddr, uint32_t IO_Pin);
uint32_t pca9535e_IO_ITStatus(uint16_t DeviceAddr, uint32_t IO_Pin);
uint8_t  pca9535e_IO_ReadIT(uint16_t DeviceAddr, uint32_t IO_Pin);
void     pca9535e_IO_ClearIT(uint16_t DeviceAddr, uint32_t IO_Pin);
void     pca9535e_Start(uint16_t DeviceAddr, uint32_t IO_Pin);


/* PCA9535E driver structure */
extern IO_DrvTypeDef pca9535e_io_drv;


#ifdef __cplusplus
}
#endif
#endif /* __PCA9535E_H */

/**
  * @}
  */ 

/**
  * @}
  */

/**
  * @}
  */ 

/**
  * @}
  */       
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
