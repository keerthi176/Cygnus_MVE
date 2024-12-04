/**
  ******************************************************************************
  * File Name          : JPEG.c
  * Description        : This file provides code for the configuration
  *                      of the JPEG instances.
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
#include "jpeg.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

JPEG_HandleTypeDef hjpeg;

/* JPEG init function */
void MX_JPEG_Init(void)
{

  hjpeg.Instance = JPEG;
  if (HAL_JPEG_Init(&hjpeg) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}
void HAL_JPEG_MspInit(JPEG_HandleTypeDef *hjpeg)
{
  static DMA_HandleTypeDef   hdmaIn;
  static DMA_HandleTypeDef   hdmaOut;
  
  /* Enable JPEG clock */
  __HAL_RCC_JPEG_CLK_ENABLE();
  
    /* Enable DMA clock */
  __DMA2_CLK_ENABLE();
  
  HAL_NVIC_SetPriority(JPEG_IRQn, 0x06, 0x0F);
  HAL_NVIC_EnableIRQ(JPEG_IRQn);
  
  /* Input DMA */    
  /* Set the parameters to be configured */
  hdmaIn.Init.Channel = DMA_CHANNEL_9;
  hdmaIn.Init.Direction = DMA_MEMORY_TO_PERIPH;
  hdmaIn.Init.PeriphInc = DMA_PINC_DISABLE;
  hdmaIn.Init.MemInc = DMA_MINC_ENABLE;
  hdmaIn.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdmaIn.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdmaIn.Init.Mode = DMA_NORMAL;
  hdmaIn.Init.Priority = DMA_PRIORITY_HIGH;
  hdmaIn.Init.FIFOMode = DMA_FIFOMODE_ENABLE;         
  hdmaIn.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  hdmaIn.Init.MemBurst = DMA_MBURST_INC4;
  hdmaIn.Init.PeriphBurst = DMA_PBURST_INC4;      
  
  hdmaIn.Instance = DMA2_Stream0;
  
  /* Associate the DMA handle */
  __HAL_LINKDMA(hjpeg, hdmain, hdmaIn);
  
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0x07, 0x0F);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);    
  
  /* DeInitialize the DMA Stream */
  HAL_DMA_DeInit(&hdmaIn);  
  /* Initialize the DMA stream */
  HAL_DMA_Init(&hdmaIn);
  
  
  /* Output DMA */
  /* Set the parameters to be configured */ 
  hdmaOut.Init.Channel = DMA_CHANNEL_9;
  hdmaOut.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdmaOut.Init.PeriphInc = DMA_PINC_DISABLE;
  hdmaOut.Init.MemInc = DMA_MINC_ENABLE;
  hdmaOut.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdmaOut.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdmaOut.Init.Mode = DMA_NORMAL;
  hdmaOut.Init.Priority = DMA_PRIORITY_VERY_HIGH;
  hdmaOut.Init.FIFOMode = DMA_FIFOMODE_ENABLE;         
  hdmaOut.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  hdmaOut.Init.MemBurst = DMA_MBURST_INC4;
  hdmaOut.Init.PeriphBurst = DMA_PBURST_INC4;

  
  hdmaOut.Instance = DMA2_Stream1;
  /* DeInitialize the DMA Stream */
  HAL_DMA_DeInit(&hdmaOut);  
  /* Initialize the DMA stream */
  HAL_DMA_Init(&hdmaOut);

  /* Associate the DMA handle */
  __HAL_LINKDMA(hjpeg, hdmaout, hdmaOut);
  
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0x07, 0x0F);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);  
    
}


void HAL_JPEG_MspDeInit(JPEG_HandleTypeDef* jpegHandle)
{

  if(jpegHandle->Instance==JPEG)
  {
  /* USER CODE BEGIN JPEG_MspDeInit 0 */

  /* USER CODE END JPEG_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_JPEG_CLK_DISABLE();

    /* JPEG interrupt Deinit */
    HAL_NVIC_DisableIRQ(JPEG_IRQn);
  /* USER CODE BEGIN JPEG_MspDeInit 1 */

  /* USER CODE END JPEG_MspDeInit 1 */
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
