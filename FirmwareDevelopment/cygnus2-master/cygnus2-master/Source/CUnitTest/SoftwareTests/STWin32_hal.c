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
*  File         : STWin32_hal.c
*
*  Description  : Implementation of the HAL Stub for WIN32 testing
*
*************************************************************************************/



/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* User Include Files
*************************************************************************************/
#include "stm32l4xx.h"
#include "gpio.h"

/* Private Functions Prototypes
*************************************************************************************/


/* Global Variables
*************************************************************************************/
uint8_t periph[10000];
uint32_t test_hal_tick_value = 0;

Gpio_t FirstAidMCP;
Gpio_t FireMCP;
Gpio_t Tamper1;
Gpio_t Tamper2;
Gpio_t MainBattery;
uint32_t TestFirstAidMCPState;
uint32_t TestFireMCPState;
uint32_t TestTamper1State; 
uint32_t TestTamper2State;
uint32_t TestMainBatteryState;
uint16_t stub_crc_value = 0x1234u;
CRC_HandleTypeDef Ymodem_CrcHandle;
RNG_HandleTypeDef RngHandle;
UART_HandleTypeDef UartHandle;
uint32_t topOfMainStackAddress;
uint32_t regMainStackPointer;
SysTick_Type dummy_SysTick;
SCB_Type     dummy_SCB;
EXTI_TypeDef dummy_Exti;
bool bSystemResetCalled = false;
uint8_t hal_dma_counter;
RNG_TypeDef  dummy_RNG;
SYSCFG_TypeDef dummy_SYSCFG;
char Image$$RESERVED_CRC$$Base[8];

/* Private Variables
*************************************************************************************/
HAL_StatusTypeDef HAL_FLASHEx_DATAEEPROM_Unlock(void)
{
   return HAL_OK;
}

HAL_StatusTypeDef HAL_FLASHEx_DATAEEPROM_Lock(void)
{
   return HAL_OK;
}

HAL_StatusTypeDef HAL_FLASHEx_DATAEEPROM_Program(uint32_t TypeProgram, uint32_t Address, uint32_t Data)
{
   memcpy((void*)Address,&Data,sizeof(uint32_t));
   return HAL_OK;
}

void HAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
}

uint32_t HAL_GetTick(void)
{
	return test_hal_tick_value;
}

HAL_StatusTypeDef HAL_CRC_Init(CRC_HandleTypeDef *hcrc)
{
	return HAL_OK;
}

uint32_t HAL_CRC_Calculate(CRC_HandleTypeDef *hcrc, uint32_t pBuffer[], uint32_t BufferLength)
{
	return (uint32_t)stub_crc_value;
}

HAL_CRC_StateTypeDef HAL_CRC_GetState(CRC_HandleTypeDef *hcrc)
{
	return HAL_CRC_STATE_READY;
}

HAL_StatusTypeDef HAL_CRC_DeInit(CRC_HandleTypeDef *hcrc)
{
	return HAL_CRC_STATE_READY;
}

void HAL_NVIC_SystemReset(void)
{
   bSystemResetCalled = true;
}


void FLASH_If_Init(void){}
void HAL_RCC_DeInit(void){}
void HAL_UART_IRQHandler(UART_HandleTypeDef *huart){}
void HAL_DMA_IRQHandler(DMA_HandleTypeDef *hdma){}
HAL_StatusTypeDef HAL_Init(void) { return HAL_OK; }
HAL_StatusTypeDef HAL_DeInit(void) { return HAL_OK; }
HAL_StatusTypeDef HAL_RNG_Init(RNG_HandleTypeDef *hrng) { return HAL_OK; }
HAL_StatusTypeDef HAL_RNG_DeInit(RNG_HandleTypeDef *hrng){return HAL_OK;}
HAL_StatusTypeDef HAL_RNG_GenerateRandomNumber(RNG_HandleTypeDef *hrng, uint32_t *random32bit){return HAL_OK;}
HAL_StatusTypeDef HAL_UART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout) { return HAL_OK; }
HAL_StatusTypeDef  HAL_FLASH_Unlock(void) { return HAL_OK; }
HAL_StatusTypeDef  HAL_FLASH_Lock(void) { return HAL_OK; }
HAL_StatusTypeDef HAL_UART_DeInit(UART_HandleTypeDef *huart) { return HAL_OK; }
HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size) { return HAL_OK; }
HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size) { return HAL_OK; }
HAL_StatusTypeDef HAL_UART_AbortReceive(UART_HandleTypeDef *huart) { return HAL_OK; }
void HAL_NVIC_DisableIRQ(IRQn_Type IRQn) { UNUSED(IRQn); }
HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart) { return HAL_OK; }
HAL_StatusTypeDef HAL_DMA_Init(DMA_HandleTypeDef *hdma) { return HAL_OK; }
HAL_StatusTypeDef HAL_DMA_DeInit(DMA_HandleTypeDef *hdma) { return HAL_OK; }
HAL_StatusTypeDef FLASH_If_BankSwitch(void) { return HAL_OK; }


