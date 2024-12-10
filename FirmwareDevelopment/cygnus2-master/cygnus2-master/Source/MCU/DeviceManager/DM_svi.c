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
*  File         : DM_svi.c
*
*  Description  : Implementation file for the Sound and Visual Indicator (SVI)
*                 interface
*
*************************************************************************************/

/* System Include Files
*************************************************************************************/

/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "stm32l4xx.h"
#include "DM_i2c.h"
#include "board.h"
#include "DM_LED.h"
#include "DM_Device.h"
#include "DM_OutputManagement.h"
#include "CFG_Device_cfg.h"
#include "DM_svi.h"

/* Private Functions Prototypes
*************************************************************************************/


/* Private definitions
*************************************************************************************/
#define DM_SVI_SOUNDER_BIT 0x01
#define DM_SVI_VI_BIT 0x02
#define DM_SVI_TIMEOUT 10

#define DM_SVI_STARTUP_DELAY 10
#define DM_SVI_BIT_RETRY_INTERVAL 100

/* Global Varbles
*************************************************************************************/
extern uint32_t I2C_Address;
extern Gpio_t SndrBeaconOn;
extern uint16_t gDeviceCombination;

/* Private Variables
*************************************************************************************/
static SviState_t SVI_State = DM_SVI_STATE_INITIALISING_E;
static bool SVI_output_active = false;
static uint32_t gTimeoutCount = 0;

/* Functions
*************************************************************************************/
static ErrorCode_t DM_SVI_Enable( void );

/*************************************************************************************/
/**
* DM_SVI_Initialise
* ENable the I2C and apply power to the SVI
*
* @param - None
*
* @return - ErrorCode_t   error code.
*/
ErrorCode_t DM_SVI_Initialise(void)
{
   ErrorCode_t result = ERR_INIT_FAIL_E;
   
   if ( DM_SVI_IsEnabled() )
   {
      result = SUCCESS_E;
   }   
   else 
   {
      result = DM_I2C_Startup(I2C_RBU_ADDRESS);
   
      if ( (SUCCESS_E == result) && (DM_I2C_IsInitialised()) )
      {
         //Power-up the SVI
         GpioWrite(&SVIPowerOn, 1);
         osDelay(500);
         result = SUCCESS_E;
         CO_PRINT_B_0(DBG_INFO_E,"SVI power ON\r\n");
      }
   }
   
   //Hold off the timeout each time we try to contact the SVI
   gTimeoutCount = 0;
   
   return result;
}

/*************************************************************************************/
/**
* DM_SVI_Deinitialise
* Switch off the SVI and the I2C if the SVI output isn't active
*
* @param - None
*
* @return - ErrorCode_t   error code.
*/
ErrorCode_t DM_SVI_Deinitialise(void)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;
   
   if ( DM_SVI_IsEnabled() )
   {
      if ( false == SVI_output_active )
      {
         result = ERR_INIT_FAIL_E;
         if ( SUCCESS_E == DM_I2C_ShutDown() )
         {
            //Power-down the SVI
            GpioWrite(&SVIPowerOn, 0);
            result = SUCCESS_E;
            CO_PRINT_B_0(DBG_INFO_E,"SVI shut down\r\n");
         }
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* DM_SVI_Enable
* Power-up the SVI if it is powered down.
*
* @param - None.
*
* @return - ErrorCode_t   error code.
*/
ErrorCode_t DM_SVI_Enable( void )
{
   ErrorCode_t status = ERR_INIT_FAIL_E;
   
   if ( DM_SVI_IsEnabled() )
   {
      status = SUCCESS_E;
   }
   else 
   {
      status = DM_SVI_Initialise();
   }
   
   return status;
}

/*************************************************************************************/
/**
* DM_SVI_IsEnabled
* Function to return whether the SVI is powered up.
*
* @param - None
*
* @return - True if the SVI power is on.
*/
bool DM_SVI_IsEnabled(void)
{
   bool enabled = false;
   if ( GpioRead(&SVIPowerOn) )
   {
      enabled = true;
   }
   return enabled;
}

/*************************************************************************************/
/**
* DM_SVI_CheckForTimeout
* Function to check whether the SVI can be shut down.
* When the SVI has been powered up, it is kept running for DM_SVI_TIMEOUT seconds
* to enable further commands to be sent to it.
* After the timeout period we call DM_SVI_Deinitialise to power the SVI down.
*
* @param - None.
*
* @return - None.
*/
void DM_SVI_CheckForTimeout(void)
{
   //check if the config is set to keep the power on.
   if ( false == CFG_GetSviPowerLock() )
   {
      //Power not locked on.  OK to shut the SVI down.
      if ( DM_SVI_IsEnabled() )
      {
         gTimeoutCount++;
         if ( DM_SVI_TIMEOUT <= gTimeoutCount )
         {
            DM_SVI_Deinitialise();
            SVI_State = DM_SVI_STATE_IDLE_E;
            gTimeoutCount = 0;
         }
      }
   }
}

/*************************************************************************************/
/**
* DM_SVI_GetRegister
* Finds the correct register address for the parameter type
*
* @param - command            The parameter type.  This is mapped to the values of enum DM_SviParameterType_t
* @param - registerAddress    [OUT] The address of the register.
*
* @return - ErrorCode_t   error code.
*/
ErrorCode_t DM_SVI_GetRegister( const uint8_t command, uint8_t* registerAddress )
{
   ErrorCode_t status = ERR_INVALID_POINTER_E;
   
   if ( registerAddress )
   {
      status = SUCCESS_E;
      
      switch ( command )
      {
      case DM_SVI_TYPE_E:
         *registerAddress = DM_I2C_SVI_REG_TYPE;
         break;
      case DM_SVI_ENABLE_E:
         *registerAddress = DM_I2C_SVI_REG_ENABLE;
         break;
      case DM_SVI_TEST_MODE_E:
         *registerAddress = DM_I2C_SVI_REG_TEST_MODE;
         break;
      case DM_SVI_FLASH_RATE_E:
         *registerAddress = DM_I2C_SVI_REG_FLASH_RATE;
         break;
      case DM_SVI_TONE_SELECT_E:
         *registerAddress = DM_I2C_SVI_REG_TONE_SELECT;
         break;
      case DM_SVI_TONE_ATTEN_E:
         *registerAddress = DM_I2C_SVI_REG_TONE_ATTEN;
         break;
      case DM_SVI_FAULT_STATUS_E:
         *registerAddress = DM_I2C_SVI_REG_FAULT_STATUS;
         break;
      case DM_SVI_FW_MAJOR_LS_E:
         *registerAddress = DM_I2C_SVI_REG_FW_MAJOR_LS;
         break;
      case DM_SVI_FW_MAJOR_MS_E:
         *registerAddress = DM_I2C_SVI_REG_FW_MAJOR_MS;
         break;
      case DM_SVI_FW_MINOR_LS_E:
         *registerAddress = DM_I2C_SVI_REG_FW_MINOR_LS;
         break;
      case DM_SVI_FW_MINOR_MS_E:
         *registerAddress = DM_I2C_SVI_REG_FW_MINOR_MS;
         break;
      case DM_SVI_FW_MAINT_REV_LS_E:
         *registerAddress = DM_I2C_SVI_REG_FW_MAINT_REV_LS;
         break;
      case DM_SVI_FW_MAINT_REV_MS_E:
         *registerAddress = DM_I2C_SVI_REG_FW_MAINT_REV_MS;
         break;
      case DM_SVI_FW_DATE_DAY_LS_E:
         *registerAddress = DM_I2C_SVI_REG_FW_DATE_DAY_LS;
         break;
      case DM_SVI_FW_DATE_DAY_MS_E:
         *registerAddress = DM_I2C_SVI_REG_FW_DATE_DAY_MS;
         break;
      case DM_SVI_FW_DATE_MONTH_LS_E:
         *registerAddress = DM_I2C_SVI_REG_FW_DATE_MONTH_LS;
         break;
      case DM_SVI_FW_DATE_MONTH_MS_E:
         *registerAddress = DM_I2C_SVI_REG_FW_DATE_MONTH_MS;
         break;
      case DM_SVI_FW_DATE_YEAR_LS_E:
         *registerAddress = DM_I2C_SVI_REG_FW_DATE_YEAR_LS;
         break;
      case DM_SVI_FW_DATE_YEAR_MS_E:
         *registerAddress = DM_I2C_SVI_REG_FW_DATE_YEAR_MS;
         break;
      case DM_SVI_SERIAL_NUMBER_0_E:
         *registerAddress = DM_I2C_SVI_REG_SERIAL_NUMBER_0;
         break;
      case DM_SVI_SERIAL_NUMBER_1_E:
         *registerAddress = DM_I2C_SVI_REG_SERIAL_NUMBER_1;
         break;
      case DM_SVI_SERIAL_NUMBER_2_E:
         *registerAddress = DM_I2C_SVI_REG_SERIAL_NUMBER_2;
         break;
      case DM_SVI_SERIAL_NUMBER_3_E:
         *registerAddress = DM_I2C_SVI_REG_SERIAL_NUMBER_3;
         break;
         default:
            status = ERR_OUT_OF_RANGE_E;
         break;
      }
   }
  
   
   return status;
}

/*************************************************************************************/
/**
* DM_SVI_ReadRegister
* Read the value from the specified SVI register
*
* @param - registerAddress The address of the register.
* @param - pResponse       [OUT] The value of the register.
*
* @return - ErrorCode_t   error code.
*/
ErrorCode_t DM_SVI_ReadRegister( const uint8_t registerAddress, uint8_t* pResponse )
{
   ErrorCode_t status = ERR_INIT_FAIL_E;
   
//   CO_PRINT_B_1(DBG_BIT_E, "SVI Read Register %d\r\n", registerAddress);
   
   status = DM_SVI_Enable();
      
   if ( SUCCESS_E == status )
   {
      status = DM_I2C_Read( I2C_SVI_ADDRESS, registerAddress, I2C_SVI_SINGLE_BYTE, pResponse, I2C_SVI_SINGLE_BYTE );
   }
   
   return status;
}

/*************************************************************************************/
/**
* DM_SVI_WriteRegister
* Write to the specified SVI register
*
* @param - registerAddress The address of the register.
* @param - value           The value to be written to the register.
*
* @return - ErrorCode_t   error code.
*/
ErrorCode_t DM_SVI_WriteRegister( const uint8_t registerAddress, const uint8_t value )
{
   ErrorCode_t status;
   
   CO_PRINT_B_2(DBG_INFO_E,"SVI Write Register %d, value %d\r\n", registerAddress, value);
   
   status = DM_SVI_Enable();
   
   if ( SUCCESS_E == status )
   {
      switch ( registerAddress )
      {
         case DM_I2C_SVI_REG_ENABLE:
            status = DM_I2C_Write( I2C_SVI_ADDRESS, registerAddress, I2C_SVI_SINGLE_BYTE, &value );
            if ( SUCCESS_E == status )
            {
               if ( 0 == value )
               {
                  SVI_output_active = false;
               }
               else 
               {
                  SVI_output_active = true;
               }
            }
            break;
         case DM_I2C_SVI_REG_TEST_MODE:
         case DM_I2C_SVI_REG_FLASH_RATE:
         case DM_I2C_SVI_REG_TONE_SELECT:
         case DM_I2C_SVI_REG_TONE_ATTEN:
            status = DM_I2C_Write( I2C_SVI_ADDRESS, registerAddress, I2C_SVI_SINGLE_BYTE, &value );
         break;
         default:
            status = ERR_OUT_OF_RANGE_E;
         break;
      }
   }

   return status;
}

/*************************************************************************************/
/**
* DM_SVI_ReadFirmwareVersion
* Read the SVI firmware version registers and parse them to produce an integer-format
* version number.
*
* @param - pVersion [OUT] Pointer to the uint32_t where the result should be written to.
*
* @return - ErrorCode_t   error code.
*/
ErrorCode_t DM_SVI_ReadFirmwareVersion( uint32_t* pVersion )
{
   ErrorCode_t status = ERR_INIT_FAIL_E;
   uint8_t svi_revision[8];
   
   if ( DM_SVI_IsEnabled() )
   {
      if( pVersion )
      {
         status = DM_I2C_Read( I2C_SVI_ADDRESS, DM_I2C_SVI_REG_FW_MAJOR_LS, I2C_SVI_FIRMWARE_VERSION_LENGTH, svi_revision, 8 );
      }
      else 
      {
         status = ERR_INVALID_POINTER_E;
      }
      
      if ( SUCCESS_E == status )
      {
         /* we have got the version string back from the SVI.  Now we parse it and convert to an int32 */
         /* The revision characters are returned in reverse order so we need to swap them around*/
         
         /* major */
         uint8_t temp = svi_revision[0];
         svi_revision[0] = svi_revision[1];
         svi_revision[1] = temp;
         /* minor */
         temp = svi_revision[2];
         svi_revision[2] = svi_revision[3];
         svi_revision[3] = temp;
         /* maintenance revision */
         temp = svi_revision[4];
         svi_revision[4] = svi_revision[5];
         svi_revision[5] = temp;
         /* add a null char to terminate the string */
         svi_revision[6] = 0;
         /* convert the string to an int */
         char* str_end;
         *pVersion = (uint32_t)strtol((char*)&svi_revision, &str_end, 10);
      }
   }
      
   return status;
}

/*************************************************************************************/
/**
* DM_SVI_SetSounder
* Function to turn on/off the SVI sounder.
*
* @param - sounder_state   should be DM_SVI_ON_E or DM_SVI_OFF_E
*
* @return - True if the command worked.
*/
bool DM_SVI_SetSounder(const uint8_t sounder_state)
{
   bool result = false;
   uint8_t register_value = 0;
   
   if ( SUCCESS_E == DM_SVI_Enable() )
   {
      // The sounder is enabled/disabled in the same register as the VI.  We need to read the
      // current register then write it back to preserve the VI state
      if ( SUCCESS_E == DM_SVI_ReadRegister( DM_I2C_SVI_REG_ENABLE, &register_value ) )
      {
         //apply the requested sounder change to the current register state
         if ( DM_SVI_OFF_E == sounder_state )
         {
            register_value &= ~DM_SVI_SOUNDER_BIT;
         }
         else
         {
            register_value |= DM_SVI_SOUNDER_BIT;
         }
         // Write back the updated state
         if ( SUCCESS_E == DM_SVI_WriteRegister( DM_I2C_SVI_REG_ENABLE, register_value ) )
         {
            result = true;
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* DM_SVI_SetToneSelection
* Function to set the tone pattern the SVI sounder.
*
* @param - tone   One of the "Tone Selection" patterns specified in the mesh protocol
*
* @return - True if the command worked.
*/
bool DM_SVI_SetToneSelection(const DM_ToneSelectionType_t tone)
{
   bool result = false;
   
   if ( SUCCESS_E == DM_SVI_Enable() )
   {
      if ( DM_SVI_TONE_MAX_E > tone )
      {
         // Write the new pattern into the tone selection register on the SVI
         if ( SUCCESS_E == DM_SVI_WriteRegister( DM_I2C_SVI_REG_TONE_SELECT, tone ) )
         {
            result = true;
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* DM_SVI_SetFlashRate
* Function to set the tone pattern the SVI sounder.
*
* @param - flashRate   One of the flash rates specified in the mesh protocol
*
* @return - True if the command worked.
*/
bool DM_SVI_SetFlashRate(const FlashRate_t flashRate)
{
   bool result = false;
   
   if ( SUCCESS_E == DM_SVI_Enable() )
   {
      if ( FLASH_RATE_MAX_E > flashRate )
      {
         // Write the new pattern into the tone selection register on the SVI
         if ( SUCCESS_E == DM_SVI_WriteRegister( DM_I2C_SVI_REG_FLASH_RATE, flashRate ) )
         {
            result = true;
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* DM_SVI_SetToneAttenuation
* Function to set the tone attenuation of the SVI sounder.
*
* @param - soundLevel   Sound level to apply (0 - 3)
*
* @return - True if the command worked.
*/
bool DM_SVI_SetToneAttenuation(const uint8_t soundLevel)
{
   bool result = false;
   uint8_t attenuation = soundLevel;

   if ( SUCCESS_E == DM_SVI_Enable() )
   {
      if ( SOUND_LEVEL_MAX_E < attenuation )
      {
         attenuation = SOUND_LEVEL_MAX_E;
      }
      // Write the new level into the tone attenuation register on the SVI
      if ( SUCCESS_E == DM_SVI_WriteRegister( DM_I2C_SVI_REG_TONE_ATTEN, attenuation ) )
      {
         result = true;
      }
   }
   return result;
}

/*************************************************************************************/
/**
* DM_SVI_SetVisualIndicator
* Function to turn on/off the SVI visual indicator.
*
* @param - vi_state   should be DM_SVI_ON_E or DM_SVI_OFF_E
*
* @return - True if the command worked.
*/
bool DM_SVI_SetVisualIndicator(const uint8_t vi_state)
{
   bool result = false;
   uint8_t register_value = 0;
   

   if ( SUCCESS_E == DM_SVI_Enable() )
   {
      // The VI is enabled/disabled in the same register as the Sounder.  We need to read the
      // current register then write it back to preserve the Sounder state
      if ( SUCCESS_E == DM_SVI_ReadRegister( DM_I2C_SVI_REG_ENABLE, &register_value ) )
      {
         //apply the requested VI change to the current register state
         if ( DM_SVI_OFF_E == vi_state )
         {
            register_value &= ~DM_SVI_VI_BIT;
         }
         else
         {
            register_value |= DM_SVI_VI_BIT;
         }
         // Write back the updated state
         if ( SUCCESS_E == DM_SVI_WriteRegister( DM_I2C_SVI_REG_ENABLE, register_value ) )
         {
            result = true;
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* function name   : DM_SVI_SviGetSerialNumber
* description     : Read the serial Number from the SVI device
*
* @param - SerialNumber [OUT] Pointer to the variable where the serial number should be written.
*
* @return - Error code
*/
ErrorCode_t DM_SVI_SviGetSerialNumber(uint32_t *SerialNumber)
{
   ErrorCode_t result;
   union
   {
      uint8_t bytes[4];
      uint32_t word;
   }serial_number;
   
   if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SOUNDER_85DBA, DC_MATCH_ANY_E) )
   {
      //Enable the I2C to the sounder board via the sounder/beacon power line
      GpioWrite(&SndrBeaconOn, 1);
      osDelay(10);
   }


   result = DM_SVI_ReadRegister(DM_I2C_SVI_REG_SERIAL_NUMBER_0, &serial_number.bytes[0]);
   if (SUCCESS_E == result)
   {
      result = DM_SVI_ReadRegister(DM_I2C_SVI_REG_SERIAL_NUMBER_1, &serial_number.bytes[1]);
      if (SUCCESS_E == result)
      {
         result = DM_SVI_ReadRegister(DM_I2C_SVI_REG_SERIAL_NUMBER_2, &serial_number.bytes[2]);
         if (SUCCESS_E == result)
         {
            result = DM_SVI_ReadRegister(DM_I2C_SVI_REG_SERIAL_NUMBER_3, &serial_number.bytes[3]);
            if (SUCCESS_E == result)
            {
               /* copy the read value */
               *SerialNumber = serial_number.word;
            }
         }
      }
   }

   //Only remove power from the 85dB SVI if it is not in alarm state
   if (  DM_DeviceIsEnabled(gDeviceCombination, DEV_SOUNDER_85DBA, DC_MATCH_ANY_E) &&
         DM_OP_IsActivated(DM_OP_CHANNEL_SOUNDER_E) )
   {
      //Disable the I2C to the sounder board via the sounder/beacon power line
      GpioWrite(&SndrBeaconOn, 0);
   }

   return result;
}

/*************************************************************************************/
/**
* function name   : DM_SVI_BitTest
* description     : Perform BIT
*
* @param - none
*
* @return - Error code.
*/
ErrorCode_t DM_SVI_BitTest(void)
{
   ErrorCode_t result = ERR_BUILT_IN_TEST_FAIL_E;
   if ( SUCCESS_E == DM_SVI_Enable())
   {
      /* Check if the serial number is valid */
      if ( DM_SVI_SviIsSerialNumberValid())
      {
         CO_SendFaultSignal(BIT_SOURCE_SVI_E, CO_CHANNEL_SOUNDER_E, FAULT_SIGNAL_INTERNAL_FAULT_E, 0, false, false);
         CO_PRINT_B_0(DBG_BIT_E, "SVI Success\r\n");
         result = SUCCESS_E;
      }
      else
      {
         CO_SendFaultSignal(BIT_SOURCE_SVI_E, CO_CHANNEL_SOUNDER_E, FAULT_SIGNAL_INTERNAL_FAULT_E, 1, false, false);
         CO_PRINT_A_0(DBG_BIT_E, "SVI Fail\r\n");
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* function name   : DM_SVI_SviIsSerialNumberValid
* description     : checks if a serial number is valid
*
* @param - none
*
* @return - true if the serial number is valid
*/
bool DM_SVI_SviIsSerialNumberValid(void)
{
   bool validSerialNumber = false;
   ErrorCode_t result = ERR_BUILT_IN_TEST_FAIL_E;
   uint32_t SerialNumber;


   /* Read the Serial Number */
   result = DM_SVI_SviGetSerialNumber(&SerialNumber);

   if (SUCCESS_E == result)
   {
      if ( 0 != SerialNumber )
      {
         validSerialNumber = true;
         CO_PRINT_B_1(DBG_INFO_E, "SVI SRN is %d\r\n", SerialNumber);
      }
   }

   return validSerialNumber;
}

/*************************************************************************************/
/**
* function name   : DM_SVI_PeriodicCheck
* description     : checks if a serial number is valid
*
* @param - none
*
* @return - none
*/
void DM_SVI_PeriodicCheck(void)
{
   static uint32_t startup_count = DM_SVI_STARTUP_DELAY;
   
   switch ( SVI_State )
   {
      case DM_SVI_STATE_INITIALISING_E:
         startup_count--;
         if ( 0 == startup_count )
         {
            if ( SUCCESS_E == DM_SVI_Initialise() )
            {
               SVI_State = DM_SVI_STATE_BIT_TEST_E;
            }
            else 
            {
               //Failed to initialise. Try again later.
               startup_count = DM_SVI_BIT_RETRY_INTERVAL;
            }
         }
         break;
      case DM_SVI_STATE_BIT_TEST_E:
            if ( SUCCESS_E == DM_SVI_BitTest() )
            {
               CO_SendFaultSignal(BIT_SOURCE_SVI_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_INTERNAL_FAULT_E, 0, false, false);
               CO_PRINT_A_0(DBG_INFO_E, "SVI BIT success\r\n");
               //Cancel the fault
               CO_SendFaultSignal(BIT_SOURCE_SVI_E, CO_CHANNEL_SOUNDER_E, FAULT_SIGNAL_SOUNDER_FAULT_E, 0, true, false);
               
               SVI_State = DM_SVI_STATE_IDLE_E;
            }
            else 
            {
               CO_SendFaultSignal(BIT_SOURCE_SVI_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_INTERNAL_FAULT_E, 1, false, false);
               CO_PRINT_A_0(DBG_ERROR_E, "SVI BIT FAIL\r\n");
               //Raise a fault
               CO_SendFaultSignal(BIT_SOURCE_SVI_E, CO_CHANNEL_SOUNDER_E, FAULT_SIGNAL_SOUNDER_FAULT_E, 1, true, false);
               //Try again later.
               DM_SVI_Deinitialise();
               SVI_State = DM_SVI_STATE_BIT_FAIL_E;
               startup_count = DM_SVI_BIT_RETRY_INTERVAL;
            }
         break;
      case DM_SVI_STATE_BIT_FAIL_E:
         startup_count--;
         if ( 0 == startup_count )
         {
            SVI_State = DM_SVI_STATE_BIT_TEST_E;
         }
         break;
      case DM_SVI_STATE_IDLE_E:
         if ( DM_OP_IsActivated(DM_OP_CHANNEL_SOUNDER_E) ||
              DM_OP_IsActivated(DM_OP_CHANNEL_VISUAL_INDICATOR_E) )
         {
            SVI_State = DM_SVI_STATE_ACTIVE_E;
         }
         else 
         {
            //Check to see if the SVI can be powered down
            DM_SVI_CheckForTimeout();
         }
      break;
      case DM_SVI_STATE_ACTIVE_E:
         //Check to see if the sounder and visual indicator are switched off.
         if ( (false == DM_OP_IsActivated(DM_OP_CHANNEL_SOUNDER_E)) &&
              (false == DM_OP_IsActivated(DM_OP_CHANNEL_VISUAL_INDICATOR_E)) )
         {
            SVI_State = DM_SVI_STATE_IDLE_E;
         }
      break;
      default:
         startup_count = 0;
         SVI_State = DM_SVI_STATE_INITIALISING_E;
         break;
   }
}
