/***************************************************************************
* File name: M_filename.c
* Project name: xxxxxxxx
*
* Copyright 2018 Bull Products Ltd as an unpublished work.
* All Rights Reserved.
*
* The information contained herein is confidential property of Bull
* Products Ltd. The use, copying, transfer or disclosure of such
* information is prohibited except by express written agreement with Bull
* Products Ltd.
*
* First written on xxxxxxx by xxxxxxxx
*
* File History:
* Change History:
* Rev Date Description Author
* 1.0 MM.YYYY Initial File XX
*
* Description:
* (fill in a detailed description of the source code file).
*
**************************************************************************/
/* System Include Files
**************************************************************************/
#include <stdint.h>
	
/* User Include Files
**************************************************************************/
//#include "stm32f769i_eval.h"
#include "bsp.h"

/* Defines
**************************************************************************/
#define I2Cx_TIMEOUT_MAX                    0x3000 /*<! The value of the maximal timeout for I2C waiting loops */


/* Global Variables
**************************************************************************/

/* RTC handler declaration */
/*
RTC_HandleTypeDef      RtcHandle;


UART_HandleTypeDef* cie_gsm_uart 			= &huart4;
UART_HandleTypeDef* cie_debug_uart 			= &huart1;
UART_HandleTypeDef* cie_panel_network_uart 	= &huart3;

I2C_HandleTypeDef* &hi2c1 			= &hi2c1;
TIM_HandleTypeDef* &htim1      = &htim1;

RTC_HandleTypeDef* &hrtc 					= &hrtc;
*/

/* Private Variables
**************************************************************************/

static uint32_t I2cxTimeout = I2Cx_TIMEOUT_MAX; /*<! Value of Timeout when I2C communication fails */  

/* Private functions prototypes
**************************************************************************/
/* I2Cx bus function */

static void               I2Cx_WriteData(uint8_t Addr, uint8_t Reg, uint8_t Value);
static void               I2Cx_WriteBuffer(uint8_t Addr, uint8_t Reg,  uint8_t *pBuffer, uint16_t Length);
static uint8_t            I2Cx_ReadData(uint8_t Addr, uint8_t Reg);
static uint8_t            I2Cx_ReadBuffer(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length);
static void               I2Cx_Error(void);

/* Function Prototype
**************************************************************************/
  

/* IOExpander IO functions */
void                      IOE_Init(void);
void                      IOE_ITConfig(void);
void                      IOE_Delay(uint32_t Delay);
void                      IOE_Write(uint8_t Addr, uint8_t Reg, uint8_t Value);
uint8_t                   IOE_Read(uint8_t Addr, uint8_t Reg);
uint16_t                  IOE_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length);
void                      IOE_WriteMultiple(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length);




/*******************************************************************************
                            BUS OPERATIONS
*******************************************************************************/


/**
  * @brief  Writes a value in a register of the device through BUS.
  * @param  Addr: Device address on BUS Bus.  
  * @param  Reg: The target register address to write
  * @param  Value: The target register value to be written 
  */
void I2Cx_WriteData(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
  HAL_StatusTypeDef status = HAL_OK;
  
  status = HAL_I2C_Mem_Write( &hi2c1, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, &Value, 1, I2cxTimeout); 
  
  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Re-Initialize the BUS */
    I2Cx_Error();
  }        
}


/**
  * @brief  Reads multiple data.
  * @param  Addr: I2C address
  * @param  Reg: Reg address
  * @param  MemAddress: memory address
  * @param  Buffer: Pointer to data buffer
  * @param  Length: Length of the data
  * @retval HAL status
  */
static HAL_StatusTypeDef I2Cx_ReadMultiple(uint8_t Addr,
                                       uint16_t Reg,
                                       uint16_t MemAddress,
                                       uint8_t *Buffer,
                                       uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_I2C_Mem_Read(&hi2c1,
                        Addr,
                        (uint16_t)Reg,
                        MemAddress,
                        Buffer,
                        Length,
                        1000);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* I2C error occured */
    I2Cx_Error( );
  }
  return status;
}

/**
  * @brief  Writes a value in a register of the device through BUS.
  * @param  Addr: Device address on BUS Bus.  
  * @param  Reg: The target register address to write
  * @param  pBuffer: The target register value to be written 
  * @param  Length: buffer size to be written
  */
static void I2Cx_WriteBuffer( uint8_t Addr, uint8_t Reg,  uint8_t *pBuffer, uint16_t Length )
{
  HAL_StatusTypeDef status = HAL_OK;
  
  status = HAL_I2C_Mem_Write( &hi2c1, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, pBuffer, Length, I2cxTimeout); 

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Re-Initialize the BUS */
    I2Cx_Error();
  }        
}

/**
  * @brief  Reads a register of the device through BUS.
  * @param  Addr: Device address on BUS Bus.  
  * @param  Reg: The target register address to write
  * @retval Data read at register address
  */
static uint8_t I2Cx_ReadData(uint8_t Addr, uint8_t Reg)
{
  HAL_StatusTypeDef status = HAL_OK;
  uint8_t value = 0;
  
  status = HAL_I2C_Mem_Read( &hi2c1, Addr, Reg, I2C_MEMADD_SIZE_8BIT, &value, 1, I2cxTimeout);
 
  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Re-Initialize the BUS */
    I2Cx_Error();
  
  }
  return value;
}

/**
  * @brief  Reads multiple data on the BUS.
  * @param  Addr: I2C Address
  * @param  Reg: Reg Address 
  * @param  pBuffer: pointer to read data buffer
  * @param  Length: length of the data
  * @retval 0 if no problems to read multiple data
  */
static uint8_t I2Cx_ReadBuffer(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_I2C_Mem_Read( &hi2c1, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, pBuffer, Length, I2cxTimeout);
  
  /* Check the communication status */
  if(status == HAL_OK)
  {
    return 0;
  }
  else
  {
    /* Re-Initialize the BUS */
    I2Cx_Error();

    return 1;
  }
}

#ifdef EE_M24LR64
/**
  * @brief  Writes a value in a register of the device through BUS in using DMA mode.
  * @param  Addr: Device address on BUS Bus.  
  * @param  Reg: The target register address to write
  * @param  pBuffer: The target register value to be written 
  * @param  Length: buffer size to be written
  * @retval HAL status
  */
static HAL_StatusTypeDef I2Cx_WriteBufferDMA(uint8_t Addr, uint16_t Reg,  uint8_t *pBuffer, uint16_t Length)
  {
  HAL_StatusTypeDef status = HAL_OK;
  
  status = HAL_I2C_Mem_Write_DMA(&hi2c1, Addr, Reg, I2C_MEMADD_SIZE_16BIT, pBuffer, Length);

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Re-Initialize the BUS */
    I2Cx_Error();
  }

  return status;
}

/**
  * @brief  Reads multiple data on the BUS in using DMA mode.
  * @param  Addr: I2C Address
  * @param  Reg: Reg Address 
  * @param  pBuffer: pointer to read data buffer
  * @param  Length: length of the data
  * @retval HAL status
  */
static HAL_StatusTypeDef I2Cx_ReadBufferDMA(uint8_t Addr, uint16_t Reg, uint8_t *pBuffer, uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_I2C_Mem_Read_DMA(&hi2c1, Addr, Reg, I2C_MEMADD_SIZE_16BIT, pBuffer, Length);
  
  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Re-Initialize the BUS */
    I2Cx_Error();
  }
  
  return status;
}

/**
* @brief  Checks if target device is ready for communication. 
* @note   This function is used with Memory devices
* @param  DevAddress: Target device address
* @param  Trials: Number of trials
* @retval HAL status
*/
static HAL_StatusTypeDef I2Cx_IsDeviceReady(uint16_t DevAddress, uint32_t Trials)
{ 
  return (HAL_I2C_IsDeviceReady(&hi2c1, DevAddress, Trials, I2cxTimeout));
}
#endif /* EE_M24LR64 */

/**
  * @brief  I2Cx error treatment function
  */
static void I2Cx_Error(void)
{
  /* De-initialize the I2C communication BUS */
  HAL_I2C_DeInit(&hi2c1);
  
  /* Re-Initialize the I2C communication BUS */
  MX_I2C1_Init();
}


/*******************************************************************************
                            LINK OPERATIONS
*******************************************************************************/

/********************************* LINK IOE ***********************************/

/**
  * @brief  IOE Low Level Initialization.
  */
void IOE_Init(void) 
{
   MX_I2C1_Init();
}

/**
  * @brief  IOE Low Level Interrupt configuration.
  */
void IOE_ITConfig(void)
{
//  I2Cx_ITConfig();
}

/**
  * @brief  IOE Writes single data operation.
  * @param  Addr: I2C Address
  * @param  Reg: Reg Address 
  * @param  Value: Data to be written
  */
void IOE_Write(uint8_t Addr, uint8_t Reg, uint8_t Value)
{
  I2Cx_WriteData(Addr, Reg, Value);
}

/**
  * @brief  IOE Reads single data.
  * @param  Addr: I2C Address
  * @param  Reg: Reg Address 
  * @retval The read data
  */
uint8_t IOE_Read(uint8_t Addr, uint8_t Reg)
{
  return I2Cx_ReadData(Addr, Reg);
}

/**
  * @brief  IOE Writes multiple data.
  * @param  Addr: I2C Address
  * @param  Reg: Reg Address 
  * @param  pBuffer: pointer to data buffer
  * @param  Length: length of the data
  */
void IOE_WriteMultiple(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length)
{
  I2Cx_WriteBuffer(Addr, Reg, pBuffer, Length);
}

/**
  * @brief  IOE Reads multiple data.
  * @param  Addr: I2C Address
  * @param  Reg: Reg Address 
  * @param  pBuffer: pointer to data buffer
  * @param  Length: length of the data
  * @retval 0 if no problems to read multiple data
  */
uint16_t IOE_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length)
{
 return I2Cx_ReadBuffer(Addr, Reg, pBuffer, Length);
}

/**
  * @brief  IOE Delay.
  * @param  Delay in ms
  */
void IOE_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}





/**
  * @brief  Initializes I2C HAL.
  */


/********************************* LINK MFX ***********************************/


void MFX_IO_Init()
{
}

void MFX_IO_DeInit()
{
}


/**
  * @brief  Configures MFX low level interrupt.
  */
void MFX_IO_ITConfig(void)
{
#if 0	
  static uint8_t MFX_IO_IT_Enabled = 0;
  GPIO_InitTypeDef  gpio_init_structure;

  if(MFX_IO_IT_Enabled == 0)
  {
    MFX_IO_IT_Enabled = 1;
    /* Enable the GPIO EXTI clock */
    __HAL_RCC_GPIOI_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    gpio_init_structure.Pin   = GPIO_PIN_8;
    gpio_init_structure.Pull  = GPIO_NOPULL;
    gpio_init_structure.Speed = GPIO_SPEED_LOW;
    gpio_init_structure.Mode  = GPIO_MODE_IT_RISING;
    HAL_GPIO_Init(GPIOI, &gpio_init_structure);

    /* Enable and set GPIO EXTI Interrupt to the lowest priority */
    HAL_NVIC_SetPriority((IRQn_Type)(EXTI9_5_IRQn), 0x0F, 0x0F);
    HAL_NVIC_EnableIRQ((IRQn_Type)(EXTI9_5_IRQn));
  }
  #endif
}

/**
  * @brief  MFX writes single data.
  * @param  Addr: I2C address
  * @param  Reg: Register address
  * @param  Value: Data to be written
  */

void MFX_IO_Write(uint16_t Addr, uint8_t Reg, uint8_t Value)
{
  I2Cx_WriteData((uint8_t) Addr, Reg, Value);
}

/**
  * @brief  MFX reads single data.
  * @param  Addr: I2C address
  * @param  Reg: Register address
  * @retval Read data
  */
uint8_t MFX_IO_Read(uint16_t Addr, uint8_t Reg)
{
  return I2Cx_ReadData((uint8_t) Addr, Reg);
}

/**
  * @brief  MFX reads multiple data.
  * @param  Addr: I2C address
  * @param  Reg: Register address
  * @param  Buffer: Pointer to data buffer
  * @param  Length: Length of the data
  * @retval Number of read data
  */
uint16_t MFX_IO_ReadMultiple(uint16_t Addr, uint8_t Reg, uint8_t *Buffer, uint16_t Length)
{
 return I2Cx_ReadMultiple((uint8_t)Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, Buffer, Length);
}

/**
  * @brief  MFX delay
  * @param  Delay: Delay in ms
  */
void MFX_IO_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}

/**
  * @brief  Used by Lx family but requested for MXF component compatibility.
  */
void MFX_IO_Wakeup(void) 
{
}

/**
  * @brief  Used by Lx family but requested for MXF component compatibility.
  */
void MFX_IO_EnableWakeupPin(void) 
{
}



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
