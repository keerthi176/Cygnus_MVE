/*************************************************************************************
*  CONFIDENTIAL between PA and Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2018 PA Consulting Group. All rights reserved.
*  Cambridge Technology Centre
*  Melbourn Royston
*  Herts SG8 6DP UK
*  Tel: <44> 1763 261222
*
**************************************************************************************
*  File         : DM_i2c.c
*
*  Description  : Implementation file for the I2C interface and SVI emulator
*
*************************************************************************************/

/* System Include Files
*************************************************************************************/

/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "stm32l4xx.h"
#include "board.h"
#include "main.h"
#include "DM_i2c.h"


/* Private Functions Prototypes
*************************************************************************************/


/* Private definitions
*************************************************************************************/
typedef enum
{
   SVI_I2C_STATE_WAITING_FOR_ADDR_MATCH_E,
   SVI_I2C_STATE_WAITING_FOR_REGISTER_E,
   SVI_I2C_STATE_WAITING_FOR_DATA_E,
   SVI_I2C_STATE_RESPONDING_E
} sviI2CState_t;

/* Global Varbles
*************************************************************************************/
uint32_t I2C_Address;

extern Gpio_t I2CPullupsOn;


#ifdef I2C_SVI_BOARD_SIMULATION
#include "DM_svi.h"
static void DM_SVI_TaskMain (void const *argument);
static const char* GetRegisterName(const uint8_t register);
osThreadId tid_SVITask;
osThreadDef (DM_SVI_TaskMain, osPriorityNormal, 1, 1024);
static uint8_t sviRegister[I2C_SVI_MAX_REGISTER];
void DM_I2C_InitialiseSVIRegisters(void);
#endif

/* Private Variables
*************************************************************************************/
osMutexId(I2CMutexId);
osMutexDef(I2CMutex);

static I2C_HandleTypeDef  hi2c1;
static bool I2C_Initialised = false;

/* Functions
*************************************************************************************/
#ifdef WIN32
/*************************************************************************************/
/**
* GetI2CHandle
* Function to allow the windows test harness to access the private
* variable hi2c1
*
* @param - none
*
* @return - I2C_HandleTypeDef*    Pointer to hi2c1.
*/
I2C_HandleTypeDef* GetI2CHandle()
{
   return &hi2c1;
}
#endif

/*************************************************************************************/
/**
* DM_I2C_Initialise
* Create the I2C mutex.
*
* @param - None
*
* @return - True if the I2C has been initialised, false otherwise.
*/
ErrorCode_t DM_I2C_Initialise(void)
{
   I2CMutexId = osMutexCreate(osMutex(I2CMutex));
   if(NULL == I2CMutexId)
   {
      Error_Handler("Failed to create I2C mutex");
   }

   return SUCCESS_E;
}

/*************************************************************************************/
/**
* DM_I2C_Startup
* initialise the I2C interface
*
* @param - i2c_address  The I2C address of the local device
*
* @return - ErrorCode_t   error code.
*/
ErrorCode_t DM_I2C_Startup(const uint32_t i2c_address)
{
   ErrorCode_t result = SUCCESS_E;
   
   I2C_Address = i2c_address;
   
   hi2c1.Instance = I2C2;

   hi2c1.Init.Timing = I2C_TIMING_REGISTER_VALUE;
   hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
   hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
   hi2c1.Init.OwnAddress2 = 0x00;
   hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
   hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_ENABLE;
   hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

   hi2c1.Init.OwnAddress1 = i2c_address;
   
   if (HAL_I2C_Init(&hi2c1) != HAL_OK)
   {
      result = ERR_INIT_FAIL_E;
      CO_PRINT_B_0(DBG_ERROR_E,"HAL_I2C_Init() FAILED");
   }

    /**Configure Analogue filter 
    */
   if ((SUCCESS_E == result) && (HAL_OK != HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE)))
   {
      result = ERR_INIT_FAIL_E;
      CO_PRINT_B_0(DBG_ERROR_E,"HAL_I2CEx_ConfigAnalogFilter() FAILED");
   }

    /**Configure Digital filter 
    */
   if ((SUCCESS_E == result) && (HAL_OK != HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0)))
   {
      result = ERR_INIT_FAIL_E;
      CO_PRINT_B_0(DBG_ERROR_E,"HAL_I2CEx_ConfigDigitalFilter() FAILED");
   }
   
#ifdef I2C_SVI_BOARD_SIMULATION
   I2C_Address = i2c_address;
   /* start the SVI emulator */
   tid_SVITask = osThreadCreate (osThread(DM_SVI_TaskMain), NULL);
   if (NULL == tid_SVITask)
   {
      return ERR_INIT_FAIL_E;
   }
#endif

   if ( SUCCESS_E == result )
   {
      //Apply the I2C pullup voltage
      GpioWrite(&I2CPullupsOn, 1);
      
      I2C_Initialised = true;
   }
   
   
   return result;
}


/*************************************************************************************/
/**
* DM_I2C_IsInitialised
* Function to report if the I2C has been initialised.
*
* @param - None
*
* @return - True if the I2C has been initialised, false otherwise.
*/
bool DM_I2C_IsInitialised(void)
{
   return I2C_Initialised;
}

/*************************************************************************************/
/**
* DM_I2C_ShutDown
* Deinitialise the I2C interface
*
* @param  - None.
*
* @return - ErrorCode_t   error code.
*/
ErrorCode_t DM_I2C_ShutDown(void)
{
   ErrorCode_t result;
   if (HAL_I2C_DeInit(&hi2c1) == HAL_OK)
   {
      I2C_Initialised = false;
      result = SUCCESS_E;
      
      //Remove the I2C pullup voltage
      GpioWrite(&I2CPullupsOn, 0);
   }
   return result;
}

/*************************************************************************************/
/**
* HAL_I2C_MspInit
* Override of the weak I2C function.  Initialize the I2C MSP.
*
* @param - hi2c Pointer to a I2C_HandleTypeDef structure that contains
*          the configuration information for the specified I2C.
*
* @return - void.
*/
void HAL_I2C_MspInit(I2C_HandleTypeDef *i2cHandle)
{
   GPIO_InitTypeDef GPIO_InitStruct;
   if ( NULL != i2cHandle )
   {

      if(I2C2 == i2cHandle->Instance)
      {
         /**I2C1 GPIO Configuration    
         PB10     ------> I2C2_SCL
         PB11     ------> I2C2_SDA 
         */
         GPIO_InitStruct.Pin = GPIO_PIN_10;
         GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
         GPIO_InitStruct.Pull = GPIO_PULLUP;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
         GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
         HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

         GPIO_InitStruct.Pin = GPIO_PIN_11;
         GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
         GPIO_InitStruct.Pull = GPIO_PULLUP;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
         GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
         HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
         /* Peripheral clock enable */
         __HAL_RCC_I2C2_CLK_ENABLE();
      }
   }
}

/*************************************************************************************/
/**
* HAL_I2C_MspDeInit
* Override of the weak I2C function.  DeInitialize the I2C MSP.
*
* @param - hi2c Pointer to a I2C_HandleTypeDef structure that contains
*          the configuration information for the specified I2C.
*
* @return - void.
*/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *i2cHandle)
{
   if ( NULL != i2cHandle )
   {
      if(I2C2 == i2cHandle->Instance)
      {
         /* Peripheral clock disable */
         __HAL_RCC_I2C2_CLK_DISABLE();
         
         /**I2C1 GPIO Configuration    
         PB10     ------> I2C2_SCL
         PB11     ------> I2C2_SDA 
         */
         HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10|GPIO_PIN_11);
      }
   }
}


/*************************************************************************************/
/**
* DM_I2C_Read
* Blocking function with timeout.  Read value from the requested register on the stated device.
*
* @param - device_addr     The I2C address of the target device.
* @param - register_addr   The register address to read from.
* @param - pResponse       [OUT] parameter to receive the return value.
*
* @return - ErrorCode_t    SUCCESS_E on success.
*/
ErrorCode_t DM_I2C_Read( const uint8_t device_addr, const uint8_t register_addr, const uint16_t data_length, const uint8_t* rx_buffer, const uint16_t rx_buffer_size )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   uint32_t start_time = HAL_GetTick();
   HAL_StatusTypeDef hal_status;
   uint32_t hal_error;
   
   if (osMutexWait(I2CMutexId, I2C_TIMEOUT_MILLISECONDS) == osOK) 
   {
      if ( rx_buffer )
      {
         if ( rx_buffer_size >= data_length )
         {
            hal_status = HAL_I2C_Mem_Read(&hi2c1, (uint16_t)device_addr, (uint16_t)register_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t*)rx_buffer, data_length, I2C_TIMEOUT_MILLISECONDS);
            if ( HAL_OK != hal_status )
            {
               hal_error = HAL_I2C_GetError( &hi2c1 );
               DM_I2C_HandleError(hal_error);
               result = ERR_MESSAGE_FAIL_E;
               //Reset the I2C
               CLEAR_BIT(hi2c1.Instance->CR1, I2C_CR1_PE);
               osDelay(1);
               SET_BIT(hi2c1.Instance->CR1, I2C_CR1_PE);
            }
            else
            {
               result = SUCCESS_E;
            }
         }
         else
         {
            result = ERR_OUT_OF_RANGE_E;
         }
      }

      osMutexRelease(I2CMutexId);
   }
   
   return result;
}

/*************************************************************************************/
/**
* DM_I2C_Write
* Blocking function with timeout.  Write a value to the requested register on the stated device.
*
* @param - device_addr     The I2C address of the target device.
* @param - register_addr   The register address to read from.
* @param - data            The value to write.
*
* @return - ErrorCode_t    SUCCESS_E on success.
*/
ErrorCode_t DM_I2C_Write( const uint8_t device_addr, const uint8_t register_addr, const uint16_t data_length, const uint8_t* data_buffer )
{
   ErrorCode_t result = SUCCESS_E;
   HAL_StatusTypeDef hal_status;
   
   if (osMutexWait(I2CMutexId, I2C_TIMEOUT_MILLISECONDS) == osOK) 
   {
      if ( data_buffer )
      {
         hal_status = HAL_I2C_Mem_Write(&hi2c1, device_addr, (uint16_t)register_addr, I2C_MEMADD_SIZE_8BIT, (uint8_t*)data_buffer, data_length, I2C_TIMEOUT_MILLISECONDS);
         if ( HAL_OK != hal_status)
         {
            uint32_t hal_error = HAL_I2C_GetError( &hi2c1 );
            DM_I2C_HandleError(hal_error);
            result =  ERR_MESSAGE_FAIL_E;
         }
      }
      else
      {
         result = ERR_INVALID_POINTER_E;
      }
      
      osMutexRelease(I2CMutexId);
   }

   return result;
}


/*************************************************************************************/
/**
* DM_I2C_HandleError
* Prints a human readable error on the debug USART
*
* @param - hal_error     The error code to be displayed.
*
* @return - void.
*/
void DM_I2C_HandleError(uint32_t hal_error)
{
   if ( HAL_I2C_ERROR_AF == hal_error )
   {
      CO_PRINT_A_0(DBG_ERROR_E, "I2C ACK\r\n");
   }
   else if ( HAL_I2C_ERROR_BERR == hal_error )
   {
      CO_PRINT_A_0(DBG_ERROR_E, "I2C BERR\r\n");
   }
   else if ( HAL_I2C_ERROR_ARLO == hal_error )
   {
      CO_PRINT_A_0(DBG_ERROR_E, "I2C ARLO\r\n");
   }
   else if ( HAL_I2C_ERROR_OVR == hal_error )
   {
      CO_PRINT_A_0(DBG_ERROR_E, "I2C OVR\r\n");
   }
   else if ( HAL_I2C_ERROR_DMA == hal_error )
   {
      CO_PRINT_A_0(DBG_ERROR_E, "I2C DMA\r\n");
   }
   else if ( HAL_I2C_ERROR_TIMEOUT == hal_error )
   {
      CO_PRINT_A_0(DBG_ERROR_E, "I2C TIMEOUT\r\n");
   }
   else if ( HAL_I2C_ERROR_SIZE == hal_error )
   {
      CO_PRINT_A_0(DBG_ERROR_E, "I2C SIZE MGMT\r\n");
   }
   else if ( HAL_I2C_ERROR_NONE == hal_error )
   {
      CO_PRINT_A_0(DBG_ERROR_E, "I2C NONE\r\n");
   }
   else
   {
      CO_PRINT_A_1(DBG_ERROR_E, "I2C UNKNOWN = %d\r\n",hal_error);
   }
}

#ifdef I2C_SVI_BOARD_SIMULATION
void DM_I2C_InitialiseSVIRegisters(void)
{
   /* Default all to 0xff (not used) */
   for ( uint32_t index = 0; index < I2C_SVI_MAX_REGISTER; index++)
   {
      sviRegister[index] = 0xff;
   }
   
   /* populate the used registers */
   sviRegister[DM_I2C_SVI_REG_TYPE] = 0x03;
   sviRegister[DM_I2C_SVI_REG_ENABLE] = 0x00;
   sviRegister[DM_I2C_SVI_REG_TEST_MODE] = 0x01;
   sviRegister[DM_I2C_SVI_REG_FLASH_RATE] = 0x05;
   sviRegister[DM_I2C_SVI_REG_TONE_SELECT] = 0x06;
   sviRegister[DM_I2C_SVI_REG_TONE_ATTEN] = 0x00;
   sviRegister[DM_I2C_SVI_REG_FAULT_STATUS] = 0x00;
   sviRegister[DM_I2C_SVI_REG_FW_MAJOR_LS] = 0x01;
   sviRegister[DM_I2C_SVI_REG_FW_MAJOR_MS] = 0x00;
   sviRegister[DM_I2C_SVI_REG_FW_MINOR_LS] = 0x02;
   sviRegister[DM_I2C_SVI_REG_FW_MINOR_MS] = 0x00;
   sviRegister[DM_I2C_SVI_REG_FW_MAINT_REV_LS] = 0x03;
   sviRegister[DM_I2C_SVI_REG_FW_MAINT_REV_MS] = 0x00;
   sviRegister[DM_I2C_SVI_REG_FW_DATE_DAY_LS] = 0x00;
   sviRegister[DM_I2C_SVI_REG_FW_DATE_DAY_MS] = 0x03;
   sviRegister[DM_I2C_SVI_REG_FW_DATE_MONTH_LS] = 0x01;
   sviRegister[DM_I2C_SVI_REG_FW_DATE_MONTH_MS] = 0x00;
   sviRegister[DM_I2C_SVI_REG_FW_DATE_YEAR_LS] = 0x08;
   sviRegister[DM_I2C_SVI_REG_FW_DATE_YEAR_MS] = 0x01;
   sviRegister[DM_I2C_SVI_REG_SERIAL_NUMBER_0] = 0x12;
   sviRegister[DM_I2C_SVI_REG_SERIAL_NUMBER_1] = 0x34;
   sviRegister[DM_I2C_SVI_REG_SERIAL_NUMBER_2] = 0x56;
   sviRegister[DM_I2C_SVI_REG_SERIAL_NUMBER_3] = 0x78;
}

/*************************************************************************************/
/**
* DM_SVI_TaskMain
* Main function for the Sound and Visual Indicator (SVI) emulation task.
*
* @param - argument  Pointer to passed in data.
*
* @return - void
*/
static void DM_SVI_TaskMain (void const *argument)
{
   uint8_t transferdirection;
   uint16_t slaveaddrcode;
   sviI2CState_t svi_i2c_state = SVI_I2C_STATE_WAITING_FOR_ADDR_MATCH_E;
   uint8_t targetRegister;
   uint8_t rxdData;

   
   CO_PRINT_A_0(DBG_INFO_E, "SVI Simulator Started\r\n");
   
   DM_I2C_InitialiseSVIRegisters();


   while ( 1 )
   {
      switch (svi_i2c_state)
      {
         case SVI_I2C_STATE_WAITING_FOR_ADDR_MATCH_E:
            if ((hi2c1.Instance->ISR & I2C_ISR_ADDR) == (I2C_ISR_ADDR))
            {
               transferdirection = I2C_GET_DIR(&hi2c1);
               slaveaddrcode     = I2C_GET_ADDR_MATCH(&hi2c1);
               
               if ( slaveaddrcode == I2C_Address )
               {
                  if ( I2C_READ_DIR == transferdirection )
                  {
                     svi_i2c_state =  SVI_I2C_STATE_RESPONDING_E;
                  }
                  else 
                  {
                     svi_i2c_state =  SVI_I2C_STATE_WAITING_FOR_REGISTER_E;
                  }
               }
               
               SET_BIT(hi2c1.Instance->ICR, I2C_ICR_ADDRCF);
            }
            break;
         case SVI_I2C_STATE_WAITING_FOR_REGISTER_E:
            if ( (hi2c1.Instance->ISR & I2C_ISR_RXNE) == I2C_ISR_RXNE )
            {
               targetRegister = hi2c1.Instance->RXDR;
               svi_i2c_state =  SVI_I2C_STATE_WAITING_FOR_DATA_E;
            }
            break;
         case SVI_I2C_STATE_WAITING_FOR_DATA_E:
            if ((hi2c1.Instance->ISR & I2C_ISR_ADDR) == (I2C_ISR_ADDR))
            {
               transferdirection = I2C_GET_DIR(&hi2c1);
               slaveaddrcode     = I2C_GET_ADDR_MATCH(&hi2c1);
               
               if ( slaveaddrcode == I2C_Address )
               {                  
                  if ( I2C_READ_DIR == transferdirection )
                  {
                     svi_i2c_state =  SVI_I2C_STATE_RESPONDING_E;
                  }
                  else
                  {
                     SET_BIT(hi2c1.Instance->ICR, I2C_ICR_ADDRCF);
                  }
              }
               else
               {
                  SET_BIT(hi2c1.Instance->ICR, I2C_ICR_ADDRCF);
               }
            }
            else if ( (hi2c1.Instance->ISR & I2C_ISR_RXNE) == I2C_ISR_RXNE )
            {
               rxdData = hi2c1.Instance->RXDR;
               sviRegister[targetRegister] = rxdData;
               
               const char* regName = GetRegisterName(targetRegister);               
               CO_PRINT_A_2(DBG_INFO_E, "Writing register %s - value %d\r\n", regName, rxdData);
               svi_i2c_state =  SVI_I2C_STATE_WAITING_FOR_ADDR_MATCH_E;
               
               if ( DM_I2C_SVI_REG_ENABLE == targetRegister )
               {
                  if ( 0 == (sviRegister[DM_I2C_SVI_REG_ENABLE] & 0x01) )
                  {
                     GpioWrite(&StatusLedRed, 0);
                  }
                  else 
                  {
                     GpioWrite(&StatusLedRed, 1);
                  }
                  if ( 0 == (sviRegister[DM_I2C_SVI_REG_ENABLE] & 0x02) )
                  {
                     GpioWrite(&StatusLedBlue, 0);
                  }
                  else 
                  {
                     GpioWrite(&StatusLedBlue, 1);
                  }
               }
            }

            break;
         case SVI_I2C_STATE_RESPONDING_E:
            {
               const char* regName = GetRegisterName(targetRegister);
               CO_PRINT_A_3(DBG_INFO_E, "Reading register %s - value 0x%0.2X (%d)\r\n", regName, sviRegister[targetRegister], sviRegister[targetRegister] );
               /* read command */
               uint8_t return_value = sviRegister[targetRegister];            
               uint32_t hal_error;
               HAL_StatusTypeDef hal_status;
               
               if (osMutexWait(I2CMutexId, I2C_TIMEOUT_MILLISECONDS) == osOK) 
               {
                  hal_status = HAL_I2C_Slave_Transmit(&hi2c1, (uint8_t*)&return_value, 1, 100);
                  hal_error = HAL_I2C_GetError( &hi2c1 );
                  osMutexRelease(I2CMutexId);
               }
               
               if ( HAL_OK != hal_status )
               {
                  CO_PRINT_A_2(DBG_INFO_E, "FAILED sending response. status=%d, error=%d\r\n", hal_status, hal_error);
               }

               svi_i2c_state = SVI_I2C_STATE_WAITING_FOR_ADDR_MATCH_E;
            }
            break;
         default:
            svi_i2c_state = SVI_I2C_STATE_WAITING_FOR_ADDR_MATCH_E;
            break;
      }
   }
}


const char* GetRegisterName(const uint8_t registerID)
{
   static const char* registers[DM_SVI_PARAMETER_TYPE_MAX_E] = {
      "'Type' (0x00)",
      "'Enable' (0x10)",
      "'Test Mode' (0x20)",
      "'Flash Rate' (0x40)",
      "'Tone Selection' (0x80)",
      "'Tone Attenuation' (0x90)",
      "'Fault Status' (0xC0)",
      "'Firmware Version Major LS' (0xE0)",
      "'Firmware Version Major MS' (0xE1)",
      "'Firmware Version Minor LS' (0xE2)",
      "'Firmware Version Minor MS' (0xE3)",
      "'Firmware Version Maintenance LS' (0xE4)",
      "'Firmware Version Maintenance MS' (0xE5)",
      "'Firmware Date Day LS' (0xE6)",
      "'Firmware Date Day MS' (0xE7)",
      "'Firmware Date Month LS' (0xE8)",
      "'Firmware Date Month MS' (0xE9)",
      "'Firmware Date Year LS' (0xEA)",
      "'Firmware Date Year MS' (0xEB)",
      "'Serial Number 0' (0xFC)",
      "'Serial Number 1' (0xFD)",
      "'Serial Number 2' (0xFE)",
      "'Serial Number 3' (0xFF)"
   };
   static const char* unknownRegister = "UNKNOWN REGISTER";
   
   const char* pReturn;
   switch ( registerID )
   {
      case DM_I2C_SVI_REG_TYPE:
         pReturn = registers[DM_SVI_TYPE_E];
         break;
      case DM_I2C_SVI_REG_ENABLE:
         pReturn = registers[DM_SVI_ENABLE_E];
         break;
      case DM_I2C_SVI_REG_TEST_MODE:
         pReturn = registers[DM_SVI_TEST_MODE_E];
         break;
      case DM_I2C_SVI_REG_FLASH_RATE:
         pReturn = registers[DM_SVI_FLASH_RATE_E];
         break;
      case DM_I2C_SVI_REG_TONE_SELECT:
         pReturn = registers[DM_SVI_TONE_SELECT_E];
         break;
      case DM_I2C_SVI_REG_TONE_ATTEN:
         pReturn = registers[DM_SVI_TONE_ATTEN_E];
         break;
      case DM_I2C_SVI_REG_FAULT_STATUS:
         pReturn = registers[DM_SVI_FAULT_STATUS_E];
         break;
      case DM_I2C_SVI_REG_FW_MAJOR_LS:
         pReturn = registers[DM_SVI_FW_MAJOR_LS_E];
         break;
      case DM_I2C_SVI_REG_FW_MAJOR_MS:
         pReturn = registers[DM_SVI_FW_MAJOR_MS_E];
         break;
      case DM_I2C_SVI_REG_FW_MINOR_LS:
         pReturn = registers[DM_SVI_FW_MINOR_LS_E];
         break;
      case DM_I2C_SVI_REG_FW_MINOR_MS:
         pReturn = registers[DM_SVI_FW_MINOR_MS_E];
         break;
      case DM_I2C_SVI_REG_FW_MAINT_REV_LS:
         pReturn = registers[DM_SVI_FW_MAINT_REV_LS_E];
         break;
      case DM_I2C_SVI_REG_FW_MAINT_REV_MS:
         pReturn = registers[DM_SVI_FW_MAINT_REV_MS_E];
         break;
      case DM_I2C_SVI_REG_FW_DATE_DAY_LS:
         pReturn = registers[DM_SVI_FW_DATE_DAY_LS_E];
         break;
      case DM_I2C_SVI_REG_FW_DATE_DAY_MS:
         pReturn = registers[DM_SVI_FW_DATE_DAY_MS_E];
         break;
      case DM_I2C_SVI_REG_FW_DATE_MONTH_LS:
         pReturn = registers[DM_SVI_FW_DATE_MONTH_LS_E];
         break;
      case DM_I2C_SVI_REG_FW_DATE_MONTH_MS:
         pReturn = registers[DM_SVI_FW_DATE_MONTH_MS_E];
         break;
      case DM_I2C_SVI_REG_FW_DATE_YEAR_LS:
         pReturn = registers[DM_SVI_FW_DATE_YEAR_LS_E];
         break;
      case DM_I2C_SVI_REG_FW_DATE_YEAR_MS:
         pReturn = registers[DM_SVI_FW_DATE_YEAR_MS_E];
         break;
      case DM_I2C_SVI_REG_SERIAL_NUMBER_0:
         pReturn = registers[DM_SVI_SERIAL_NUMBER_0_E];
         break;
      case DM_I2C_SVI_REG_SERIAL_NUMBER_1:
         pReturn = registers[DM_SVI_SERIAL_NUMBER_1_E];
         break;
      case DM_I2C_SVI_REG_SERIAL_NUMBER_2:
         pReturn = registers[DM_SVI_SERIAL_NUMBER_2_E];
         break;
      case DM_I2C_SVI_REG_SERIAL_NUMBER_3:
         pReturn = registers[DM_SVI_SERIAL_NUMBER_3_E];
         break;
      default:
         pReturn = unknownRegister;
      break;
   }

   return pReturn;
}

#endif








