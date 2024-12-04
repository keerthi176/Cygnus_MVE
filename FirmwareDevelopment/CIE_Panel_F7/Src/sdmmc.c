/**
  ******************************************************************************
  * File Name          : SDMMC.c
  * Description        : This file provides code for the configuration
  *                      of the SDMMC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "sdmmc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

#define __DMAx_TxRx_CLK_ENABLE            __HAL_RCC_DMA2_CLK_ENABLE
#define SD1_DMAx_Tx_CHANNEL                DMA_CHANNEL_4
#define SD1_DMAx_Rx_CHANNEL                DMA_CHANNEL_4
#define SD1_DMAx_Tx_STREAM                 DMA2_Stream6  
#define SD1_DMAx_Rx_STREAM                 DMA2_Stream3  
#define SD1_DMAx_Tx_IRQn                   DMA2_Stream6_IRQn
#define SD1_DMAx_Rx_IRQn                   DMA2_Stream3_IRQn
#define BSP_SDMMC1_DMA_Tx_IRQHandler       DMA2_Stream6_IRQHandler   
#define BSP_SDMMC1_DMA_Rx_IRQHandler       DMA2_Stream3_IRQHandler 
#define BSP_SDMMC1_IRQHandler              SDMMC1_IRQHandler

SD_HandleTypeDef hsd1;
DMA_HandleTypeDef hdma_sdmmc1_rx;
DMA_HandleTypeDef hdma_sdmmc1_tx;

/* SDMMC1 init function */

void MX_SDMMC1_SD_Init(void)
{

  hsd1.Instance = SDMMC1;
  hsd1.Init.ClockEdge = SDMMC_CLOCK_EDGE_RISING;
  hsd1.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
  hsd1.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
  hsd1.Init.BusWide = SDMMC_BUS_WIDE_1B;
  hsd1.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd1.Init.ClockDiv = 0;

}


void HAL_SD_MspInit(SD_HandleTypeDef* hsd)
{  
  static DMA_HandleTypeDef dma_rx_handle;
  static DMA_HandleTypeDef dma_tx_handle;
  
	
  GPIO_InitTypeDef gpio_init_structure;
  
  
  if(hsd->Instance == SDMMC1)
  {
    /* Enable SDIO clock */
    __HAL_RCC_SDMMC1_CLK_ENABLE();
    
    /* Enable DMA2 clocks */
    __DMAx_TxRx_CLK_ENABLE();
    
    /* Enable GPIOs clock */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
    /* Common GPIO configuration */
    gpio_init_structure.Mode      = GPIO_MODE_AF_PP;
    gpio_init_structure.Pull      = GPIO_PULLUP;
    gpio_init_structure.Speed     = GPIO_SPEED_HIGH;
    gpio_init_structure.Alternate = GPIO_AF12_SDMMC1;
     
	   
    /**SDMMC1 GPIO Configuration    
    PC8     ------> SDMMC1_D0
    PC12     ------> SDMMC1_CK
    PD2     ------> SDMMC1_CMD 
    */
    
    /* GPIOC configuration: SD1_D0, SD1_D1, SD1_D2, SD1_D3 and SD1_CLK pins */
    gpio_init_structure.Pin = GPIO_PIN_8  | GPIO_PIN_12;
    
    HAL_GPIO_Init(GPIOC, &gpio_init_structure);
    
    /* GPIOD configuration: SD1_CMD pin */
    gpio_init_structure.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOD, &gpio_init_structure);
    
    /* NVIC configuration for SDIO interrupts */
    HAL_NVIC_SetPriority(SDMMC1_IRQn, 0x0E, 0);
    HAL_NVIC_EnableIRQ(SDMMC1_IRQn);
  
    dma_rx_handle.Init.Channel             = SD1_DMAx_Rx_CHANNEL;
    dma_rx_handle.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    dma_rx_handle.Init.PeriphInc           = DMA_PINC_DISABLE;
    dma_rx_handle.Init.MemInc              = DMA_MINC_ENABLE;
    dma_rx_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    dma_rx_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    dma_rx_handle.Init.Mode                = DMA_PFCTRL;
    dma_rx_handle.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    dma_rx_handle.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    dma_rx_handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    dma_rx_handle.Init.MemBurst            = DMA_MBURST_INC4;
    dma_rx_handle.Init.PeriphBurst         = DMA_PBURST_INC4;
    dma_rx_handle.Instance                 = SD1_DMAx_Rx_STREAM;
    
    /* Associate the DMA handle */
    __HAL_LINKDMA(hsd, hdmarx, dma_rx_handle);
    
    /* Deinitialize the stream for new transfer */
    HAL_DMA_DeInit(&dma_rx_handle);
    
    /* Configure the DMA stream */    
    HAL_DMA_Init(&dma_rx_handle);
         
    dma_tx_handle.Init.Channel             = SD1_DMAx_Tx_CHANNEL;
    dma_tx_handle.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    dma_tx_handle.Init.PeriphInc           = DMA_PINC_DISABLE;
    dma_tx_handle.Init.MemInc              = DMA_MINC_ENABLE;
    dma_tx_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    dma_tx_handle.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    dma_tx_handle.Init.Mode                = DMA_PFCTRL;
    dma_tx_handle.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
    dma_tx_handle.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    dma_tx_handle.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    dma_tx_handle.Init.MemBurst            = DMA_MBURST_INC4;
    dma_tx_handle.Init.PeriphBurst         = DMA_PBURST_INC4;
    dma_tx_handle.Instance                 = SD1_DMAx_Tx_STREAM; 
    
    /* Associate the DMA handle */
    __HAL_LINKDMA(hsd, hdmatx, dma_tx_handle);
    
    /* Deinitialize the stream for new transfer */
    HAL_DMA_DeInit(&dma_tx_handle);
    
    /* Configure the DMA stream */
    HAL_DMA_Init(&dma_tx_handle);  

    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(SD1_DMAx_Rx_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(SD1_DMAx_Rx_IRQn);
    
    /* NVIC configuration for DMA transfer complete interrupt */
    HAL_NVIC_SetPriority(SD1_DMAx_Tx_IRQn, 0x0F, 0);
    HAL_NVIC_EnableIRQ(SD1_DMAx_Tx_IRQn);  
  }
  
}

/**
  * @brief  DeInitializes the SD MSP.
  * @param  hsd: SD handle
  * @param  Params  
  * @retval None
  */
void HAL_SD_MspDeInit(SD_HandleTypeDef *hsd )
{
  static DMA_HandleTypeDef dma_rx_handle;
  static DMA_HandleTypeDef dma_tx_handle;
  static DMA_HandleTypeDef dma_rx_handle2;
  static DMA_HandleTypeDef dma_tx_handle2;
  
  if(hsd->Instance == SDMMC1)
  {
    /* Disable NVIC for DMA transfer complete interrupts */
    HAL_NVIC_DisableIRQ(SD1_DMAx_Rx_IRQn);
    HAL_NVIC_DisableIRQ(SD1_DMAx_Tx_IRQn);
    
    /* Deinitialize the stream for new transfer */
    dma_rx_handle.Instance = SD1_DMAx_Rx_STREAM;
    HAL_DMA_DeInit(&dma_rx_handle);
    
    /* Deinitialize the stream for new transfer */
    dma_tx_handle.Instance = SD1_DMAx_Tx_STREAM;
    HAL_DMA_DeInit(&dma_tx_handle);
    
    /* Disable NVIC for SDIO interrupts */
    HAL_NVIC_DisableIRQ(SDMMC1_IRQn);
    
    /* DeInit GPIO pins can be done in the application 
    (by surcharging this __weak function) */
    
    /* Disable SDMMC1 clock */
    __HAL_RCC_SDMMC1_CLK_DISABLE();
  }
  
  /* GPIO pins clock and DMA clocks can be shut down in the application
  by surcharging this __weak function */ 
}

	  
	  
	  
	  
	  
	  
	   
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
