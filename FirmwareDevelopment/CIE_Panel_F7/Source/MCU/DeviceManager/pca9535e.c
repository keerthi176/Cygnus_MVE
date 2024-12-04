





/* Includes ------------------------------------------------------------------*/
#include "pca9535e.h"
#include "stm32f7xx_hal_def.h"
#include "stm32f7xx_hal_dma.h"
#include "stm32f7xx_hal_i2c.h"
#include "stm32f7xx_hal.h"

extern I2C_HandleTypeDef hi2c2;

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */ 
  
/** @defgroup PCA9535E
  * @{
  */   
  
/* Private typedef -----------------------------------------------------------*/

/** @defgroup PCA9535E_Private_Types_Definitions
  * @{
  */ 
  
/* Private define ------------------------------------------------------------*/

/** @defgroup PCA9535E_Private_Defines
  * @{
  */ 
#define PCA9535E_MAX_INSTANCE        8
#define I2Cx_TIMEOUT_MAX              0x300 /*<! The value of the maximal timeout for I2C waiting loops */




/* Private macro -------------------------------------------------------------*/

/** @defgroup PCA9535E_Private_Macros
  * @{
  */ 
  
/* Private variables ---------------------------------------------------------*/



static uint16_t tmp = 0;

static uint32_t I2cxTimeout = I2Cx_TIMEOUT_MAX; /*<! Value of Timeout when I2C communication fails */  


/** @defgroup PCA9535E_Private_Variables
  * @{
  */ 
/* IO driver structure initialization */  
IO_DrvTypeDef pca9535e_io_drv = 
{
  pca9535e_Init,
  pca9535e_ReadID,
  pca9535e_Reset,
  pca9535e_Start,
  pca9535e_IO_Config,
  pca9535e_IO_WritePin,
  pca9535e_IO_ReadPin,
  0,
  0,
  pca9535e_IO_ITStatus,
  pca9535e_IO_ClearIT,
};

uint8_t pca9535e[PCA9535E_MAX_INSTANCE] = {0};



void MX_I2C2_Init( void );

static void I2Cx_Error(void)
{
  /* De-initialize the I2C communication BUS */
  HAL_I2C_DeInit(&hi2c2);
  
  /* Re-Initialize the I2C communication BUS */
  MX_I2C2_Init();
}

/**
  * @}
  */ 
    
/* Private function prototypes -----------------------------------------------*/


static uint16_t IOE_ReadMultiple(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t length);
static void     IOE_WriteMultiple(uint8_t addr, uint8_t reg, uint8_t *buffer, uint16_t length);



/** @defgroup PCA9535E_Private_Function_Prototypes
  * @{
  */
  

  
static uint8_t pca9535e_GetInstance(uint16_t DeviceAddr);

/* Private functions ---------------------------------------------------------*/

/** @defgroup PCA9535E_Private_Functions
  * @{
  */ 

/**
  * @brief  Initialize the pca9535e and configure the needed hardware resources
  * @param  DeviceAddr: Device address on communication Bus.  
  * @retval None
  */
void pca9535e_Init(uint16_t DeviceAddr)
{
  uint8_t instance;
  uint8_t empty;
  
  /* Check if device instance already exists */
  instance = pca9535e_GetInstance(DeviceAddr);
  
  if(instance == 0xFF)
  {
    /* Look for empty instance */
    empty = pca9535e_GetInstance(0);
    
    if(empty < PCA9535E_MAX_INSTANCE)
    {
      /* Register the current device instance */
      pca9535e[empty] = DeviceAddr;

      
      /* Generate pca9535e Software reset */
      pca9535e_Reset(DeviceAddr);
    }
  } 
}

/**
  * @param  DeviceAddr: Device address on communication Bus.
  * @retval None.
  */
void pca9535e_Start(uint16_t DeviceAddr, uint32_t IO_Pin)
{
  /*Configuration already done during the initialization */ 
}

/**
  * @brief  Reset the pca9535e by Software.
  * @param  DeviceAddr: Device address on communication Bus.  
  * @retval None
  */
void pca9535e_Reset(uint16_t DeviceAddr)
{

}

/**
  * @brief  Read the pca9535e device ID.
  * @param  DeviceAddr: Device address on communication Bus.  
  * @retval The Device ID (two bytes).
  */
uint16_t pca9535e_ReadID(uint16_t DeviceAddr)
{
	return 0x6969;
}

/**
  * @brief  Set the global interrupt Polarity.
  * @param  DeviceAddr: Device address on communication Bus.  
  * @param  Polarity: could be one of these values; 
  *   @arg  PCA9535E_POLARITY_LOW: Interrupt line is active Low/Falling edge      
  *   @arg  PCA9535E_POLARITY_HIGH: Interrupt line is active High/Rising edge               
  * @retval None
  */
void pca9535e_SetITPolarity(uint16_t DeviceAddr, uint8_t Polarity)
{
	// irq is on both edges
}

/**
  * @brief  Enable the Global interrupt.
  * @param  DeviceAddr: Device address on communication Bus.        
  * @retval None
  */
void pca9535e_EnableGlobalIT(uint16_t DeviceAddr)
{
	// irq is always on
}

/**
  * @brief  Disable the Global interrupt.
  * @param  DeviceAddr: Device address on communication Bus.        
  * @retval None
  */
void pca9535e_DisableGlobalIT(uint16_t DeviceAddr)
{ 
	// irq is always on
}

/**
  * @brief  Initialize the selected pin(s) direction.
  * @param  DeviceAddr: Device address on communication Bus.   
  * @param  IO_Pin: IO pin(s) to be configured. 
  *         This parameter could be any combination of the following values:
  *   @arg  PCA9535E_PIN_x: where x can be from 0 to 15.  
  * @param  Direction: could be PCA9535E_DIRECTION_IN or PCA9535E_DIRECTION_OUT.      
  * @retval None
  */
void pca9535e_IO_InitPin(uint16_t DeviceAddr, uint32_t IO_Pin, uint8_t Direction)
{
  uint8_t tmpData[2] = {0 , 0};
  
  /* Get the current register value */
  IOE_ReadMultiple(DeviceAddr, PCA9535E_REG_CONFIG, tmpData, 2);

  tmp = ((uint16_t)tmpData[0] | (((uint16_t)tmpData[1]) << 8));
  
  /* Set the Pin direction */
  if (Direction == PCA9535E_DIRECTION_IN)
  {
    tmp |= (uint16_t)IO_Pin;
  }  
  else 
  {
    tmp &= ~(uint16_t)IO_Pin;
  }
    
  /* Set the new register value */
  IOE_WriteMultiple(DeviceAddr, PCA9535E_REG_CONFIG, (uint8_t *)&tmp, 2);      
}

/**
  * @brief  Configure the IO pin(s) according to IO mode structure value.
  * @param  DeviceAddr: Device address on communication Bus.  
  * @param  IO_Pin: The output pin to be set or reset. This parameter can be one 
  *         of the following values:   
  *   @arg  PCA9535E_PIN_x: where x can be from 0 to 7.
  * @param  IO_Mode: The IO pin mode to configure, could be one of the following values:
  *   @arg  IO_MODE_INPUT
  *   @arg  IO_MODE_OUTPUT
  *   @arg  IO_MODE_IT_RISING_EDGE
  *   @arg  IO_MODE_IT_FALLING_EDGE         
  * @retval 0 if no error, IO_Mode if error
  */
uint8_t pca9535e_IO_Config(uint16_t DeviceAddr, uint32_t IO_Pin, IO_ModeTypedef IO_Mode)
{
  uint8_t error_code = 0;
     
    
  /* Configure IO pin according to selected IO mode */
  switch(IO_Mode)
  {
	  case IO_MODE_IT_RISING_EDGE: /* Interrupt rising edge mode */
	 case IO_MODE_IT_FALLING_EDGE: /* Interrupt falling edge mode */	  
  case IO_MODE_INPUT: /* Input mode */
  case IO_MODE_IT_RISING_FALLING:
    pca9535e_IO_InitPin(DeviceAddr, IO_Pin, PCA9535E_DIRECTION_IN);
    break;
    
  case IO_MODE_OUTPUT: /* Output mode */
    
    pca9535e_IO_InitPin(DeviceAddr, IO_Pin, PCA9535E_DIRECTION_OUT);
    break;
  

  default:
    error_code = (uint8_t) IO_Mode;
	break;
  } 
  return error_code;
}

/**
  * @brief  Enable polarity inversion of the selected IO pin(s).
  * @param  DeviceAddr: Device address on communication Bus.    
  * @param  IO_Pin: IO pin(s) to be configured.  
  *         This parameter could be any combination of the following values:
  *   @arg  PCA9535E_PIN_x: where x can be from 0 to 15. 
  * @retval None
  */ 
void pca9535e_IO_PolarityInv_Enable(uint16_t DeviceAddr, uint32_t IO_Pin)
{
  uint8_t tmpData[2] = {0 , 0};
  
  /* Get the current register value */
  IOE_ReadMultiple(DeviceAddr, PCA9535E_REG_POLARITY, tmpData, 2);
  
  tmp = ((uint16_t)tmpData[0] | (((uint16_t)tmpData[1]) << 8));  

  /* Enable pin polarity inversion */
  tmp |= (uint16_t)IO_Pin;
    
  /* Set the new register value */  
  IOE_WriteMultiple(DeviceAddr, PCA9535E_REG_POLARITY, (uint8_t *)&tmp, 2);
}

/**
  * @brief  Disable polarity inversion of the selected IO pins.
  * @param  DeviceAddr: Device address on communication Bus.    
  * @param  IO_Pin: IO pin(s) to be configured. 
  *         This parameter could be any combination of the following values:
  *   @arg  PCA9535E_PIN_x: where x can be from 0 to 15.
  * @retval None
  */ 
void pca9535e_IO_PolarityInv_Disable(uint16_t DeviceAddr, uint32_t IO_Pin)
{
  uint8_t tmpData[2] = {0 , 0};
  
  /* Get the current register value */
  IOE_ReadMultiple(DeviceAddr, PCA9535E_REG_POLARITY, tmpData, 2);
  
  tmp = ((uint16_t)tmpData[0] | (((uint16_t)tmpData[1]) << 8));  

  /* Disable pin polarity inversion */
   tmp &= ~ (uint16_t)IO_Pin;
    
  /* Set the new register value */  
  IOE_WriteMultiple(DeviceAddr, PCA9535E_REG_POLARITY, (uint8_t *)&tmp, 2);  
}

/**
  * @brief  Set the value of the selected IO pins.
  * @param  DeviceAddr: Device address on communication Bus.  
  * @param  IO_Pin: IO pin(s) to be set.
  *         This parameter could be any combination of the following values:
  *   @arg  PCA9535E_PIN_x: where x can be from 0 to 15.
  * @param  PinState: The value to be set. 
  * @retval None
  */
int pca9535e_IO_WritePin(uint16_t DeviceAddr, uint32_t IO_Pin, uint8_t PinState)
{
  uint8_t tmpData[2] = {0 , 0};
  
  /* Get the current register value */
  if ( IOE_ReadMultiple(DeviceAddr, PCA9535E_REG_OUTPUT, tmpData, 2 ) == 0 )
  {	  
	  tmp = ((uint16_t)tmpData[0] | (((uint16_t)tmpData[1]) << 8));  
	  
	  /* Set the pin state */
	  if(PinState != 0)
	  {
		 tmp |= (uint16_t)IO_Pin;
	  }  
	  else 
	  {
		 tmp &= ~(uint16_t)IO_Pin;
	  }
		 
	  /* Set the new register value */  
	  IOE_WriteMultiple(DeviceAddr, PCA9535E_REG_OUTPUT, (uint8_t *)&tmp, 2);
	  
	  return 1;
  }
  return 0;
}

/**
  * @brief  Read the state of the selected IO pin(s).
  * @param  DeviceAddr: Device address on communication Bus. 
  * @param  IO_Pin: IO pin(s) to be read.  
  *         This parameter could be any combination of the following values:
  *   @arg  PCA9535E_PIN_x: where x can be from 0 to 15.
  * @retval State of the selected IO pin(s).
  */
uint32_t pca9535e_IO_ReadPin(uint16_t DeviceAddr, uint32_t IO_Pin)
{
  uint8_t tmpData[2] = {0 , 0};
  
  /* Get the register value */
  IOE_ReadMultiple(DeviceAddr, PCA9535E_REG_INPUT, tmpData, 2);
  
  tmp = ((uint16_t)tmpData[0] | (((uint16_t)tmpData[1]) << 8));
  
  /* Return the pin(s) state */
  return(tmp & IO_Pin);  
}

uint8_t pca9535e_I2CIOExp_ReadPin(uint16_t DeviceAddr, uint8_t IO_Pin)
{
  uint16_t tmpData = 0x0;
  uint8_t readPinValue;

  /* Get the register value */
  IOE_ReadMultiple(DeviceAddr, PCA9535E_REG_INPUT, (uint8_t*)&tmpData, 2);

  readPinValue = ((tmpData >> IO_Pin) & (1));

  /* Return the pin(s) state */
  return readPinValue;
}

/**
  * @brief  Enable the interrupt mode for the selected IO pin(s).
  * @param  DeviceAddr: Device address on communication Bus.     
  * @param  IO_Pin: IO pin(s) to be configured.  
  *         This parameter could be any combination of the following values:
  *   @arg  PCA9535E_PIN_x: where x can be from 0 to 15. 
  * @retval None
  */
void pca9535e_IO_EnablePinIT(uint16_t DeviceAddr, uint32_t IO_Pin)
{
 
}

/**
  * @brief  Disable the interrupt mode for the selected IO pin(s).
  * @param  DeviceAddr: Device address on communication Bus.    
  * @param  IO_Pin: IO pin(s) to be configured.  
  *         This parameter could be any combination of the following values:
  *   @arg  PCA9535E_PIN_x: where x can be from 0 to 15. 
  * @retval None
  */
void pca9535e_IO_DisablePinIT(uint16_t DeviceAddr, uint32_t IO_Pin)
{
 
}

/**
  * @brief  Read the IT status of the selected IO pin(s)
  *         (clears all the pending bits if any). 
  * @param  DeviceAddr: Device address on communication Bus.  
  * @param  IO_Pin: IO pin(s) to be checked.  
  *         This parameter could be any combination of the following values:
  *   @arg  PCA9535E_PIN_x: where x can be from 0 to 15.           
  * @retval IT Status of the selected IO pin(s).
  */
uint32_t pca9535e_IO_ITStatus(uint16_t DeviceAddr, uint32_t IO_Pin)
{
   
  /* Return the pin IT status */
  return 0;  
}

/**
  * @brief  Detect an IT pending bit from the selected IO pin(s).
  *         (clears all the pending bits if any).
  * @param  DeviceAddr: Device address on communication Bus.  
  * @param  IO_Pin: IO pin(s) to be checked.  
  *         This parameter could be any combination of the following values:
  *   @arg  PCA9535E_PIN_x: where x can be from 0 to 15.           
  * @retval IT pending bit detection status.
  */
uint8_t pca9535e_IO_ReadIT(uint16_t DeviceAddr, uint32_t IO_Pin)
{
  return 0; 
}

/**
  * @brief  Clear all the IT pending bits if any. 
  * @param  DeviceAddr: Device address on communication Bus.            
  * @retval None
  */
void pca9535e_IO_ClearIT(uint16_t DeviceAddr, uint32_t IO_Pin)
{
 
}

/**
  * @brief  Check if the device instance of the selected address is already registered
  *         and return its index 
  * @param  DeviceAddr: Device address on communication Bus.
  * @retval Index of the device instance if registered, 0xFF if not.
  */
static uint8_t pca9535e_GetInstance(uint16_t DeviceAddr)
{
  uint8_t idx = 0;
  
  /* Check all the registered instances */
  for(idx = 0; idx < PCA9535E_MAX_INSTANCE ; idx ++)
  {
    /* Return index if there is address match */
    if(pca9535e[idx] == DeviceAddr)
    {
      return idx; 
    }
  }
  
  return 0xFF;
}




/**
  * @brief  Writes a value in a register of the device through BUS.
  * @param  Addr: Device address on BUS Bus.  
  * @param  Reg: The target register address to write
  * @param  pBuffer: The target register value to be written 
  * @param  Length: buffer size to be written
  */
static void IOE_WriteMultiple( uint8_t Addr, uint8_t Reg,  uint8_t *pBuffer, uint16_t Length )
{
  HAL_StatusTypeDef status = HAL_OK;
  
  status = HAL_I2C_Mem_Write( &hi2c2, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, pBuffer, Length, I2cxTimeout); 

  /* Check the communication status */
  if(status != HAL_OK)
  {
    /* Re-Initialize the BUS */
    I2Cx_Error();
  }        
}


/**
  * @brief  Reads multiple data on the BUS.
  * @param  Addr: I2C Address
  * @param  Reg: Reg Address 
  * @param  pBuffer: pointer to read data buffer
  * @param  Length: length of the data
  * @retval 0 if no problems to read multiple data
  */
static uint16_t IOE_ReadMultiple(uint8_t Addr, uint8_t Reg, uint8_t *pBuffer, uint16_t Length)
{
  HAL_StatusTypeDef status = HAL_OK;

  status = HAL_I2C_Mem_Read( &hi2c2, Addr, (uint16_t)Reg, I2C_MEMADD_SIZE_8BIT, pBuffer, Length, I2cxTimeout);
  
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
