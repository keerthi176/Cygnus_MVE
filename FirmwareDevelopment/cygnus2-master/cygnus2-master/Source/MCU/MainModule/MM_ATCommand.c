/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2018 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : MM_ATCommand.c
*
*  Description  : This module provides callback functions for all the AT Commands. Each
*                 AT command should have its own unique function.
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "stm32l4xx.h"
#include "DM_SerialPort.h"
#include "DM_NVM.h"
#include "DM_Device.h"
#include "MC_StateManagement.h"
#ifdef USE_NEW_HEAD_INTERFACE
#include "MM_PluginInterfaceTask.h"
#else
#include "MM_HeadInterfaceTask.h"
#endif
#include "MM_ATCommand.h"
#include "MM_Main.h"
#include "SM_StateMachine.h"
#include "CO_Message.h"
#include "CO_Defines.h"
#include "BLF_Boot.h"
#include "BLF_Main.h"
#include "DM_svi.h"
#include "DM_NVM_cfg.h"
#include "lptim.h"
#include "MM_ConfigSerialTask.h"
#include "MM_MACTask.h"
#include "MM_SviTask.h"
#include "DM_OutputManagement.h"
#include "MC_MacConfiguration.h"
#include "DM_LED.h"
#include "MM_Neighbourinfo.h"
#include "DM_ADC.h"
#include "DM_BatteryMonitor.h"
#include "CFG_Device_cfg.h"
#include "MC_MeshFormAndHeal.h"
#include "MC_AckManagement.h"
#include "MC_SACH_Management.h"
#include "MC_MacQueues.h"
#include "MM_PpuManagement.h"

/* Public Constants
*************************************************************************************/
#define SERIAL_NUMBER_STR_LENGTH    12
#define DETECTOR_THRESHOLD_MINIMUM  0
#define DETECTOR_THRESHOLD_MAXIMUM  99
#define AT_MAX_PARAMETERS 10

/* Private type declarations
*************************************************************************************/
typedef struct
{
   uint32_t NumberOfParameters;
   int32_t Parameter[AT_MAX_PARAMETERS];
}AT_Parameters_t;

/* Private Functions Prototypes
*************************************************************************************/
extern HAL_StatusTypeDef FLASH_WaitForLastOperation(uint32_t Timeout);
static ATMessageStatus_t MM_ATCommand_ParseCommandParameters(const uint8_t *const source_buffer, const uint8_t buffer_length, AT_Parameters_t *pParameters);
static ATMessageStatus_t MM_ATCommand_ParseCompoundComand(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, CO_CommandData_t* pCommand);
static ATMessageStatus_t MM_ATCommand_ParseCompoundTestSignal(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, ATTestMessageData_t* pTestMessage);
static ATMessageStatus_t MM_ATCommand_ParseSVICommand(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, CO_CommandData_t* pCommand);
static ATMessageStatus_t MM_ATCommand_ParseOutputSignal(const ATCommandType_t command_type, const uint8_t *const source_buffer, CO_OutputData_t* pOutputSignal);
static ATMessageStatus_t MM_ATCommand_ExtractUnitAddressAndZone(const uint8_t* pSourceBuffer, uint16_t* pUnitAddress, uint16_t* pZone);
static ATMessageStatus_t MM_ATCommand_ParseAlarmOutputState(const ATCommandType_t command_type, const uint8_t *const source_buffer, CO_AlarmOutputStateData_t* pAlarmOutputState);
/* Global Variables
*************************************************************************************/
extern osMessageQId(ATHandleQ);                     // Queue definition from ATHandleTask
extern osPoolId ATHandlePool;                         // Pool definition from ATHandleTask

extern osMessageQId(AppQ);                          // Queue to the application
extern osPoolId AppPool;                              // Pool definition for use with AppQ.
extern osPoolId MeshPool;                             // Pool definition for use with MeshQ.

extern char *strtok_r(char * s1, const char * s2, char ** ptr);

extern uint16_t gNodeAddress;
extern uint32_t gSystemID;

extern uint32_t RxTxLogic;
extern uint32_t TxRxLogic;

extern int16_t rach_tx_rx_offset;

extern int16_t gDchRxDoneLatency;
extern int16_t gDchTxOffset;
extern int16_t gDchRxOffset;

extern int16_t gRachTxOffsetDownlink;
extern int16_t gRachTxOffsetUplink;
extern int16_t gRachCadOffset;

extern int16_t gDlcchTxOffset;
extern int16_t gDlcchCadOffset;
   
extern int16_t gAckTxOffsetUplink;
extern int16_t gAckCadOffset;
extern int16_t gDchOffset;
extern int16_t gSyncCorrection;
extern int16_t gFrameSyncOffset;

extern DM_BaseType_t gBaseType;
extern ADC_HandleTypeDef adc1Handle;

extern Gpio_t TamperEnable;

extern osSemaphoreId(GpioSemId);


#ifdef ENABLE_HEARTBEAT_TX_SKIP
uint32_t gNumberOfHeatbeatsToSkip = 0;
#endif

/* Private Variables
*************************************************************************************/
static MC_MAC_TestMode_t gTestMode = MC_MAC_TEST_MODE_OFF_E;



/*************************************************************************************/
/**
* function name   : MM_ATCommand_ExtractUnitAddressAndZone
* description      : Searches the source_buffer for the sequence ZxxUxx and parses
*                   the zone number and unit address
*
* @param - pSourceBuffer      The source buffer
* @param - pUnitAddress       [OUT] pointer to where the unit address is written
* @param - pZone              [OUT] pointer to where the zone number is written
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ExtractUnitAddressAndZone(const uint8_t* pSourceBuffer, uint16_t* pUnitAddress, uint16_t* pZone)
{
   ATMessageStatus_t status = AT_INVALID_PARAMETER_E;
   uint16_t zone_number = 0;
   uint16_t unit_address = 0;
   
   if ( pSourceBuffer && pUnitAddress && pZone)
   {
      char* pZ = strchr((const char*)pSourceBuffer, 'Z');
      if ( pZ )
      {
         pZ++;
         char *str_end;
         zone_number = (uint16_t)strtol(pZ, &str_end, 10);
         
         if ( 0 != zone_number || ADDRESS_NCU == gNodeAddress )
         {
            char* pA = strchr((const char*)pSourceBuffer, 'U');
            if ( pA )
            {
               pA++;
               unit_address = (uint16_t)strtol(pA, &str_end, 10);
               
               if ( 0 != unit_address || 
                  (ADDRESS_NCU == gNodeAddress && 0 == unit_address) )
               {
                  status = AT_NO_ERROR_E;
               }
            }
         }
      }
   }
   
   if ( AT_NO_ERROR_E == status )
   {
      *pUnitAddress = unit_address;
      *pZone = zone_number;
   }

   return status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ParseCommandParameters
* description     : Read the parameters from the source buffer into the supplied structure.
*
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - pParameters     [OUT] Structure containing an array of output values
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ParseCommandParameters(const uint8_t *const source_buffer, const uint8_t buffer_length, AT_Parameters_t *pParameters)
{
   ATMessageStatus_t status = AT_INVALID_PARAMETER_E;
   uint32_t count = 0;
   char *str_end;
   char* pBuffer = (char*)source_buffer;
   bool done = false;
   
   if ( pBuffer && pParameters )
   {
      if ( 0 < buffer_length )
      {
         //Is there a unit address (ZxUx)?
         if ( 'Z' == *pBuffer )
         {
            /*read the Destination */
            uint16_t unit_address;
            uint16_t zone;
            status = MM_ATCommand_ExtractUnitAddressAndZone(source_buffer, &unit_address, &zone);
            if( AT_NO_ERROR_E == status )
            {
               pParameters->Parameter[count] = zone;
               count++;
               pParameters->Parameter[count] = unit_address;
               count++;
               //step over the first comma
               pBuffer = strchr((const char*)source_buffer, ',');
               if ( pBuffer )
               {
                  pBuffer++;
               }
               else
               {
                  done = true;
               }
            }
         }
         
         if ( !done )
         {
            // read the first value 
               pParameters->Parameter[count] = (int32_t)strtol(pBuffer, &str_end, 10);
               count++;
               // step over the comma
               pBuffer = str_end;
               pBuffer++;
            while( (count < AT_MAX_PARAMETERS) && (0 != *str_end) )
            {
               // read the next value 
                  pParameters->Parameter[count] = (int32_t)strtol(pBuffer, &str_end, 10);
               
               if ( 0 != *str_end )
               {
                  // step over the comma
                  pBuffer = str_end;
                  pBuffer++;
               }
               count++;
            }
         }
      }
      pParameters->NumberOfParameters = count;
      status = AT_NO_ERROR_E;
   }
   
   return status;
}
   
/*************************************************************************************/
/**
* function name   : MM_ATCommand_enter_at_mode
* description      : Enter AT Mode
*
* @param - UartComm_t      The source serial port
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_enter_at_mode(const UartComm_t uart_port)
{
   osStatus status;
   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = ALLOC_ATHANDLE_POOL
   if (pCmdReply)
   {
      strcpy((char*)pCmdReply->data_buffer, (const char*)"OK");
      pCmdReply->length = 2;
      pCmdReply->port_id = uart_port;

      status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      if (osOK != status)
      {
         FREE_ATHANDLE_POOL(pCmdReply);
         return AT_QUEUE_ERROR_E;
      }
      return AT_NO_ERROR_E;
   }
   return AT_INVALID_PARAMETER_E;
}


/*************************************************************************************/
/**
* function name   : MM_ATCommand_enter_bootloader
* description     : Trigger bootloader
*                   ATBOOT+      reboot
*                   ATBOOT1+     reboot to radio update
*
* @param - none
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_enter_bootloader(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   /* Request a serial reprogramming operation
    * THIS GENERATE A RESET AND WILL NOT RETURN HERE
    */
//   BLF_request_serial_programming();
   /* Wait for the end of any pending EEPROM writing operation */
   FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
  
   rbu_pp_mode_request = STARTUP_NORMAL;
   if ( 1 == option_param )
   {
      rbu_pp_mode_request = STARTUP_SERIAL_BOOTLOADER;
   }
  
   /* Request a micro reset */
   HAL_NVIC_SystemReset();

   return AT_NO_ERROR_E;
}



/*************************************************************************************/
/**
* function name   : MM_ATCommand_decimalNumber
* description     : Get and Set a decimal number parameter for the unit.
*                   The particular AT command is identified from the input paramId.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_decimalNumber(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   DM_NVMParamId_t paramId = (DM_NVMParamId_t)param_id;

   //CO_PRINT_B_1(DBG_INFO_E,"MM_ATCommand_decimalNumber param=%d\r\n", param_id);
   //Pointer source_buffer is only applicable to write commands, so it is null-checked below in the switch case AT_COMMAND_WRITE_E.
   
   // Check that the input paramId is of an appropriate type (i.e. a decimal value).
   CO_ASSERT_RET(NV_ADDRESS_E == paramId ||
                 NV_IS_SYNC_MASTER_E == paramId ||
                 NV_FREQUENCY_CHANNEL_E == paramId ||
                 NV_DEVICE_COMBINATION_E == paramId ||
                 NV_SYSTEM_ID_E == paramId ||
                 NV_UNIT_SERIAL_NO_E == paramId ||
                 NV_REPROG_USE_PP_UART_E == paramId ||
                 NV_TX_POWER_LOW_E == paramId ||
                 NV_TX_POWER_HIGH_E == paramId ||
                 NV_ZONE_NUMBER_E == paramId   ||
                 NV_PP_MODE_ENABLE_E == paramId ||
                 NV_BRANDING_ID_E == paramId ||
                 NV_PREFORM_MESH_PARENTS_E == paramId ||
                 NV_SHORT_FRAMES_PER_LONG_FRAME_E == paramId ||
                 NV_IOU_INPUT_POLL_PERIOD_E == paramId ||
                 NV_DULCH_WRAP_E == paramId ||
                 NV_RSSI_MAR_E == paramId ||
                 NV_SNR_MAR_E == paramId ||
                 NV_BATTERY_PERIOD_E == paramId ||
                 NV_AVE_POLICY_E == paramId ||
                 NV_INIT_LISTEN_PERIOD_E ||
                 NV_PHASE2_DURATION_E ||
                 NV_PHASE2_SLEEP_DURATION_E ||
                 NV_PHASE3_SLEEP_DURATION_E ||
                 NV_FREQUENCY_BAND_E ||
                 NV_MAX_RBU_CHILDREN_E ||
                 NV_PIR_ACTIVATE_PERIOD_E ||
                 NV_PIR_DEACTIVATE_PERIOD_E ||
                 NV_PIR_STRIKE_COUNT_E ||
                 NV_BAT_PRI_LOW_VOLT_THRESH_E ||
                 NV_BAT_BKP_LOW_VOLT_THRESH_E ||
                 NV_BAT_PRI_LOW_VOLT_THRESH_C_E ||
                 NV_BAT_BKP_LOW_VOLT_THRESH_C_E ||
                 NV_NUMBER_MSG_RESENDS_E,
                 AT_INVALID_PARAMETER_E);
    
   // Set up the parameters
   uint32_t value = 0;
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   osStatus status;
   Cmd_Reply_t *pCmdReply = NULL;
   
   switch (command_type) 
   {
      case AT_COMMAND_READ_E:
      {
         /* Initialise to error */
         msg_status = AT_INVALID_COMMAND_E;

         /* This kind of command doesn't expect any additional parameter in read operation*/

         if (0 == buffer_length)
         {
            if (SUCCESS_E == DM_NVMRead(paramId, &value, sizeof(value)))
            {
               /* serial number is a specal case that needs to be decoded */
               if ( NV_UNIT_SERIAL_NO_E == paramId )
               {
                  char ser_no_str[16];
                  if ( MM_ATDecodeSerialNumber( value, ser_no_str) )
                  {
                     /* Prepare the response */
                     pCmdReply = CALLOC_ATHANDLE_POOL
                     
                     if (pCmdReply)
                     {
                        pCmdReply->port_id = uart_port;
                        strcpy((char *)pCmdReply->data_buffer, ser_no_str);
                        pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);

                        /* Send the response */
                        status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);

                        if (osOK != status)
                        {
                           /* Failed to write, free the pre-allocated pool entry */
                           FREE_ATHANDLE_POOL(pCmdReply);

                           msg_status = AT_QUEUE_ERROR_E;
                        }
                        else
                        {
                           /* All done successfully */
                           msg_status = AT_NO_ERROR_E;
                        }
                     }
                  }
               }
               else if ( NV_PREFORM_MESH_PARENTS_E == paramId )
               {
                  int16_t primary_parent;
                  int16_t secondary_parent;
                  
                  primary_parent = (int16_t)(value & 0xFFFF);
                  secondary_parent = (int16_t)(value >> 16);
                  /* Prepare the response */
                  pCmdReply = CALLOC_ATHANDLE_POOL
                  
                  if (pCmdReply)
                  {
                     pCmdReply->port_id = uart_port;
                     sprintf((char *)pCmdReply->data_buffer, "%d,%d",primary_parent,secondary_parent);
                     pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);

                     /* Send the response */
                     status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);

                     if (osOK != status)
                     {
                        /* Failed to write, free the pre-allocated pool entry */
                        FREE_ATHANDLE_POOL(pCmdReply);

                        msg_status = AT_QUEUE_ERROR_E;
                     }
                     else
                     {
                        /* All done successfully */
                        msg_status = AT_NO_ERROR_E;
                     }
                  }
               }
               else
               {
                  /* Prepare the response */
                  pCmdReply = CALLOC_ATHANDLE_POOL

                  if (pCmdReply)
                  {
                     pCmdReply->port_id = uart_port;
                     sprintf((char *)pCmdReply->data_buffer, "%d",value);
                     pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);

                     /* Send the response */
                     status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
                  
                     if (osOK != status)
                     {
                        /* Failed to write, free the pre-allocated pool entry */
                        FREE_ATHANDLE_POOL(pCmdReply);

                        msg_status = AT_QUEUE_ERROR_E;
                     }
                     else
                     {
                        /* All done successfully */
                        msg_status = AT_NO_ERROR_E;
                     }
                  }
               }
            }
         }
      }
      break;
    
      case AT_COMMAND_WRITE_E:
      {
         /* Initialise to error */
         msg_status = AT_INVALID_PARAMETER_E;
         
         //Check source_buffer for NULL
         if ( source_buffer )
         {
            if (0u < buffer_length)
            {
               /* serial number is a special case that needs to be encoded */
               if ( NV_UNIT_SERIAL_NO_E == paramId )
               {
                  if ( MM_ATEncodeSerialNumber((char*)source_buffer, &value) )
                  {
                     msg_status = AT_NO_ERROR_E;
                  }
               }
               else if (NV_PREFORM_MESH_PARENTS_E == paramId)
               {
                  if ( MM_ATEncodePreformParents((char*)source_buffer, &value) )
                  {
                     msg_status = AT_NO_ERROR_E;
                  }
               }
               else if (NV_ADDRESS_E == paramId )
               {
                  char *str_end;
                  /* Read the value to be written */
                  value = strtol((char*)source_buffer, &str_end, 10);
                  
                  /* check for the old zone,address format */
                  char* pValue = str_end;
                  if(*pValue)
                  {
                     if ( ',' == *pValue )
                     {
                        //step over the comma
                        pValue++;
                        value = strtol(pValue, &str_end, 10);
                        /* Verify the syntax is correct */
                        if(!*str_end)
                        {
                           if ( 0 < value )
                           {
                              //correct for the old +1 programming format
                              value--;
                              msg_status = AT_NO_ERROR_E;
                           }
                        }
                     }
                  }
                  else 
                  {
                     msg_status = AT_NO_ERROR_E;
                  }
               }
               else
               {
                  char *str_end;
                  /* Read the value to be written */
                  value = strtol((char*)source_buffer, &str_end, 10);
                  
                  /* Verify the syntax is correct */
                  if(!*str_end)
                  {
                     msg_status = AT_NO_ERROR_E;
                  }
               }
            }

            if ( AT_NO_ERROR_E == msg_status )
            {  
               // Check that the supplied value is within valid range
               CO_ERROR_CHECK_RET(((NV_ADDRESS_E == paramId) && (value < MAX_DEVICES_PER_SYSTEM)) ||
                                  ((NV_IS_SYNC_MASTER_E == paramId) && (value <= 1u))         ||
                                  ((NV_FREQUENCY_CHANNEL_E == paramId) && (value <= 10u))     ||
                                  ((NV_DEVICE_COMBINATION_E == paramId) && (value < DC_NUMBER_OF_DEVICE_COMBINATIONS_E))    ||
                                  ((NV_SYSTEM_ID_E == paramId) && (value <= 0xFFFFFFFF))      ||
                                  ((NV_UNIT_SERIAL_NO_E == paramId) && (value <= 0xFFFFFFFF)) ||
                                  ((NV_REPROG_USE_PP_UART_E == paramId) && (value <= 1u))     ||
                                  ((NV_TX_POWER_LOW_E == paramId) && (value <= 10u))          ||
                                  ((NV_TX_POWER_HIGH_E == paramId) && (value <= 10u))         ||
                                  ((NV_ZONE_NUMBER_E == paramId) && (value <= 512u))          ||
                                  ((NV_PP_MODE_ENABLE_E == paramId) && (value <= 1u))         ||
                                  ((NV_BRANDING_ID_E == paramId) && (value > 0 && value <= 255u)) ||
                                  ((NV_PREFORM_MESH_PARENTS_E == paramId) && (value <= 0xFFFFFFFF)) ||
                                  ((NV_SHORT_FRAMES_PER_LONG_FRAME_E == paramId) && (value > 0 ) && ((value % 16) == 0u)) ||
                                  ((NV_IOU_INPUT_POLL_PERIOD_E == paramId) && (value > 0 )) ||
                                  ((NV_DULCH_WRAP_E == paramId) && (value <= MAX_DULCH_WRAP )) ||
                                  ((NV_RSSI_MAR_E == paramId) && (value <= 11 )) ||
                                  ((NV_SNR_MAR_E == paramId)) ||
                                  ((NV_BATTERY_PERIOD_E == paramId) && (value <= MAX_BATTERY_TEST_INTERVAL )) ||
                                  ((NV_AVE_POLICY_E == paramId) && (value <= 3 )) ||
                                  ((NV_INIT_LISTEN_PERIOD_E == paramId) && (value <= MAX_LOW_POWER_INITIAL_AWAKE_DURATION ) && (value > 0)) ||
                                  ((NV_PHASE2_DURATION_E == paramId) && (value > 0)) ||
                                  ((NV_PHASE2_SLEEP_DURATION_E == paramId) && (value <= MAX_LOW_POWER_SLEEP_DURATION ) && (value > 0)) ||
                                  ((NV_PHASE3_SLEEP_DURATION_E == paramId) && (value <= MAX_LOW_POWER_SLEEP_DURATION ) && (value > 0)) ||
                                  ((NV_FREQUENCY_BAND_E == paramId) && (value < FREQUENCY_BAND_MAX_E )) ||
                                  ((NV_MAX_RBU_CHILDREN_E == paramId)) ||
                                  ((NV_NUMBER_MSG_RESENDS_E == paramId)) ||
                                  ((NV_NUMBER_MSG_RESENDS_E == paramId)) ||
                                  ((NV_PIR_ACTIVATE_PERIOD_E == paramId) && (value > 0) && (value <= 1000000)) ||
                                  ((NV_PIR_DEACTIVATE_PERIOD_E == paramId) && (value > 0) && (value <= 1000000)) ||
                                  ((NV_PIR_STRIKE_COUNT_E == paramId) && (value > 0) && (value <= 1000 )) ||
                                  ((NV_BAT_PRI_LOW_VOLT_THRESH_E == paramId) && (value <= MAX_BATTERY_S_FAIL_LOW_THRESH ))   ||
                                  ((NV_BAT_BKP_LOW_VOLT_THRESH_E == paramId) && (value <= MAX_BATTERY_S_FAIL_LOW_THRESH ))   ||
                                  ((NV_BAT_PRI_LOW_VOLT_THRESH_C_E == paramId) && (value <= MAX_BATTERY_C_FAIL_LOW_THRESH )) ||
                                  ((NV_BAT_BKP_LOW_VOLT_THRESH_C_E == paramId) && (value <= MAX_BATTERY_C_FAIL_LOW_THRESH ))
                                  ,
                                  AT_INVALID_PARAMETER_E);

               if (SUCCESS_E == DM_NVMWrite(paramId, &value, sizeof(value)))
               {
                  // if the 'short frames per long frame' setting was updated, update the MAC config.
                  if ( NV_SHORT_FRAMES_PER_LONG_FRAME_E == paramId )
                  {
                     MC_SetShortFramesPerLongFrame( value );
                  }
                  
                  // NV_INIT_LISTEN_PERIOD_E is converted from hours to seconds for use in the device.
                  if ( NV_INIT_LISTEN_PERIOD_E == paramId )
                  {
                     CFG_SetInitialListenPeriod( (value * SECONDS_IN_ONE_HOUR) );
                  }
                  
                  // NV_PHASE2_DURATION_E is converted from hours to seconds for use in the device.
                  if ( NV_PHASE2_DURATION_E == paramId )
                  {
                     CFG_SetPhase2Period( (value * SECONDS_IN_ONE_HOUR) );
                  }
                  
                  // NV_PHASE2_SLEEP_DURATION_E is converted from hours to seconds for use in the device.
                  if ( NV_PHASE2_SLEEP_DURATION_E == paramId )
                  {
                     CFG_SetPhase2SleepPeriod( (value * SECONDS_IN_ONE_HOUR) );
                  }
                  
                  // NV_PHASE3_SLEEP_DURATION_E is converted from hours to seconds for use in the device.
                  if ( NV_PHASE3_SLEEP_DURATION_E == paramId )
                  {
                     CFG_SetPhase3SleepPeriod( (value * SECONDS_IN_ONE_HOUR) );
                  }
                  
                  //Write the base frequency to the MAC config
                  if ( NV_FREQUENCY_CHANNEL_E == paramId )
                  {
                     MC_SetBaseFrequencychannel(value);
                  }
                  
                  //Write the frequency band to the MAC config
                  if ( NV_FREQUENCY_BAND_E == paramId )
                  {
                     MC_SetFrequencyBand(value);
                  }
                  
                  //Write max children to dynamic config
                  if ( NV_MAX_RBU_CHILDREN_E == param_id )
                  {
                     CFG_SetMaxNumberOfChildren(value);
                  }
                  
                  //Write the number of downlink message repeats to dynamic config
                  if ( NV_NUMBER_MSG_RESENDS_E == param_id )
                  {
                     CFG_SetNumberOfResends(value);
                  }
                  
                  //If switching to PPU Mode, set rbu_pp_mode_request so that PPU Mode is started on re-start.
                  if ( NV_PP_MODE_ENABLE_E == param_id )
                  {
                     rbu_pp_mode_request = value;
                  }
                  
                  /* Write operation succeeded, allocate pool for response */
                  pCmdReply = CALLOC_ATHANDLE_POOL

                  if (pCmdReply)
                  {
                     pCmdReply->port_id = uart_port;
                     strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);               
                     pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
                     
                     /* Send the response */
                     status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
                     
                     if (osOK != status)
                     {
                        /* Failed to write, free the pre-allocated pool entry */
                        FREE_ATHANDLE_POOL(pCmdReply);
                        msg_status = AT_QUEUE_ERROR_E;
                     }
                     else
                     {
                        /* All done successfully */
                        msg_status = AT_NO_ERROR_E;
                     }
                  }
                  else
                  {
                     msg_status = AT_QUEUE_ERROR_E;
                  }
               }
               else
               {
                  /* Failed to write in the NVM */
                  msg_status = AT_INVALID_PARAMETER_E;
               }
            }
         }
      }
      break;
       
      default:
      {
         msg_status = AT_INVALID_COMMAND_E;
      }
      break;
   }   

   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_compound
* description     : Handle a command for a network device.  The command is forwarded
*                   to the NCU/RBU application for processing.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_compound(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t compound_type)
{
   ATMessageStatus_t status = AT_COMMAND_UNKNOWN_ERROR_E;
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdSignalReq = NULL;
   ATCompoundCommandType_t type = (ATCompoundCommandType_t)compound_type;
   
   if ( source_buffer )
   {
      if ( AT_COMPOUND_TYPE_COMMAND_E == type )
      {
         CO_CommandData_t CmdMessage;
         status = MM_ATCommand_ParseCompoundComand(command_type, source_buffer, buffer_length, &CmdMessage);
         if ( AT_NO_ERROR_E == status )
         {
            CmdMessage.Source = uart_port;
            /* create the command message and put into mesh queue */
            pCmdSignalReq = osPoolAlloc(AppPool);
            if (pCmdSignalReq)
            {
               pCmdSignalReq->Type = CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E;
               uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
               memcpy(pCmdReq, (uint8_t*)&CmdMessage, sizeof(CO_CommandData_t));
               pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_CommandData_t);
               
               osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCmdSignalReq);
               }
            }
         }
      }
      else if ( AT_COMPOUND_TYPE_TEST_SIGNAL_E == type )
      {
         ATTestMessageData_t testMessage;
         status = MM_ATCommand_ParseCompoundTestSignal(command_type, source_buffer, buffer_length, &testMessage);
         if ( AT_NO_ERROR_E == status )
         {
            testMessage.source = uart_port;
            pCmdSignalReq = osPoolAlloc(AppPool);
            if (pCmdSignalReq)
            {
               pCmdSignalReq->Type = CO_MESSAGE_GENERATE_TEST_MESSAGE_E;
               uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
               memcpy(pCmdReq, (uint8_t*)&testMessage, sizeof(ATTestMessageData_t));
               pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(ATTestMessageData_t);
               
               osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCmdSignalReq);
               }
            }
         }
      }
      else if ( AT_COMPOUND_TYPE_SVI_COMMAND_E == type )
      {
         CO_CommandData_t CmdMessage;
         status = MM_ATCommand_ParseSVICommand(command_type, source_buffer, buffer_length, &CmdMessage);

         if ( AT_NO_ERROR_E == status )
         {
            /* create FireSignalReq message and put into mesh queue */
            pCmdSignalReq = osPoolAlloc(AppPool);
            if (pCmdSignalReq)
            {
               CmdMessage.Source = uart_port;
               pCmdSignalReq->Type = CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E;
               uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
               memcpy(pCmdReq, (uint8_t*)&CmdMessage, sizeof(CO_CommandData_t));
               pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_CommandData_t);
               
               osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCmdSignalReq);
               }
            }
         }
      }
      else if ( AT_COMPOUND_TYPE_OUTPUT_SIGNAL_E == type )
      {
         CO_OutputData_t outData;
         status = MM_ATCommand_ParseOutputSignal(command_type, source_buffer, &outData);
         if ( AT_NO_ERROR_E == status )
         {
            //ATOUT+ZxUx,<channel>,<profile>,<active>,<duration>
            outData.Source = uart_port;
            /* create FireSignalReq message and put into mesh queue */
            pCmdSignalReq = osPoolAlloc(AppPool);
            if (pCmdSignalReq)
            {
               pCmdSignalReq->Type = CO_MESSAGE_GENERATE_OUTPUT_SIGNAL_E;
               uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
               memcpy(pCmdReq, (uint8_t*)&outData, sizeof(CO_OutputData_t));
               pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_OutputData_t);
               
               osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCmdSignalReq);
               }
            }
         }
      }
      else if ( AT_COMMAND_TYPE_ALARM_OUTPUT_STATE_E == type )
      {
         CO_AlarmOutputStateData_t alarmOutput;
         status = MM_ATCommand_ParseAlarmOutputState(command_type, source_buffer, &alarmOutput);
         if ( AT_NO_ERROR_E == status )
         {
            alarmOutput.Source = uart_port;
            /* create FireSignalReq message and put into mesh queue */
            pCmdSignalReq = osPoolAlloc(AppPool);
            if (pCmdSignalReq)
            {
               pCmdSignalReq->Type = CO_MESSAGE_GENERATE_ALARM_OUTPUT_STATE_E;
               uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
               memcpy(pCmdReq, (uint8_t*)&alarmOutput, sizeof(CO_AlarmOutputStateData_t));
               pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_AlarmOutputStateData_t);
               
               osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCmdSignalReq);
               }
            }
         }
      }
   }   
   
   return status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ParseCompoundComand
* description     : Unpack an compound command into a CO_CommandData_t structure.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - [OUT] pCommand  A Structure to be populated with the results.
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ParseCompoundComand(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, CO_CommandData_t* pCommand)
{
   ATMessageStatus_t status = AT_COMMAND_UNKNOWN_ERROR_E;
   uint8_t msg_entry[AT_PARAMETER_MAX_LENGTH];
   uint32_t msg_entry_index = 0;
   uint32_t count = 0;
   const uint8_t* pSource;
   char *str_end;

   if ( source_buffer && pCommand )
   {
      pSource = source_buffer;

      /*read the TransactionID */
      while( count < buffer_length && msg_entry_index < (AT_PARAMETER_MAX_LENGTH - 1) && *pSource != ',' )
      {
         msg_entry[msg_entry_index] = *pSource;
         msg_entry_index++;
         pSource++;
         count++;
      }

      msg_entry[msg_entry_index] = '\0';
      pCommand->TransactionID = (uint8_t)strtol((char*)msg_entry, &str_end, 10);
      pSource++;/* step over the comma */
      count++;
      
      if( count < buffer_length && !*str_end)
      {
         /*read the Destination */
         msg_entry_index = 0;
         while( count < buffer_length && msg_entry_index < (AT_PARAMETER_MAX_LENGTH - 1) && *pSource != ',' )
         {
            msg_entry[msg_entry_index] = *pSource;
            msg_entry_index++;
            pSource++;
            count++;
         }
         msg_entry[msg_entry_index] = '\0';
         #ifdef IS_PPU
         pCommand->UnitSerno = (uint32_t)strtol((char*)msg_entry, &str_end, 10);
         #else
         pCommand->Destination = (uint16_t)strtol((char*)msg_entry, &str_end, 10);
         #endif
         pSource++;/* step over the comma */
         count++;
         
         if( count < buffer_length && !*str_end)
         {
            /*read the CommandType */
            msg_entry_index = 0;
            while( count < buffer_length && msg_entry_index < (AT_PARAMETER_MAX_LENGTH - 1) && *pSource != ',' )
            {
               msg_entry[msg_entry_index] = *pSource;
               msg_entry_index++;
               pSource++;
               count++;
            }

            msg_entry[msg_entry_index] = '\0';
            pCommand->CommandType = (uint8_t)strtol((char*)msg_entry, &str_end, 10);
            pSource++;/* step over the comma */
            count++;
            
            if( count < buffer_length && !*str_end)
            {
               /*read Parameter 1 */
               msg_entry_index = 0;
               while( count < buffer_length && msg_entry_index < (AT_PARAMETER_MAX_LENGTH - 1) && *pSource != ',' )
               {
                  msg_entry[msg_entry_index] = *pSource;
                  msg_entry_index++;
                  pSource++;
                  count++;
               }
               msg_entry[msg_entry_index] = '\0';
               pCommand->Parameter1 = (uint8_t)strtol((char*)msg_entry, &str_end, 10);
               pSource++;/* step over the comma */
               count++;
               
               if( count < buffer_length && !*str_end)
               {
                  /*read Parameter 2 */
                  msg_entry_index = 0;
                  while( count < buffer_length && msg_entry_index < (AT_PARAMETER_MAX_LENGTH - 1) && *pSource != ',' )
                  {
                     msg_entry[msg_entry_index] = *pSource;
                     msg_entry_index++;
                     pSource++;
                     count++;
                  }
                  msg_entry[msg_entry_index] = '\0';
                  pCommand->Parameter2 = (uint8_t)strtol((char*)msg_entry, &str_end, 10);
                  
                  /* Check the operation type */
                  if ( AT_COMMAND_READ_E == command_type )
                  {        
                     if (count == buffer_length && !*str_end)
                     {
                        /* it's a valid read command */
                        status = AT_NO_ERROR_E;
                        
                        pCommand->ReadWrite = 0;
                     }
                  }
                  else
                  {
                     if( count < buffer_length && !*str_end)
                     {
                        pCommand->ReadWrite = 1;
                        
                        /* it's a write command.  Extract the value */
                        pSource++;/* step over the comma */
                        count++;
                        msg_entry_index = 0;
                        while( count < buffer_length && msg_entry_index < (AT_PARAMETER_MAX_LENGTH - 1) )
                        {
                           msg_entry[msg_entry_index] = *pSource;
                           msg_entry_index++;
                           pSource++;
                           count++;
                        }

                        if (count == buffer_length)
                        {
                           msg_entry[msg_entry_index] = '\0';
                           pCommand->Value = strtol((char*)msg_entry, &str_end, 10);

                           /* Check that the last value was valid */
                           if (!*str_end)
                           {
                              /* All good */
                              status = AT_NO_ERROR_E;
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   else
   {
      status = AT_INVALID_PARAMETER_E;
   }
   return status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ParseSVICommand
* description     : Unpack an compound SVI command into a CO_CommandData_t structure.
*                    ATSVI=<reg>,<value>
*                    ATSVI?<reg>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - [OUT] pCommand  A Structure to be populated with the results.
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ParseSVICommand(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, CO_CommandData_t* pCommand)
{
   static uint8_t transaction_id = 0;
   ATMessageStatus_t status = AT_COMMAND_UNKNOWN_ERROR_E;
//   uint8_t msg_entry[AT_PARAMETER_MAX_LENGTH];
//   uint32_t msg_entry_index = 0;
//   uint32_t count = 0;
//   const uint8_t* pSource;
//   char *str_end;
   AT_Parameters_t parameters;
   


   if ( source_buffer && pCommand && 0 < buffer_length)
   {
      //extract the  parameters
      status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
      
      if ( (SUCCESS_E ==status) && (0 < parameters.NumberOfParameters) )
      {
         pCommand->Parameter1 = (uint8_t)parameters.Parameter[0]; //SVI Register
         pCommand->Value = (uint8_t)parameters.Parameter[1];      //Value



//      pSource = source_buffer;
      
         /* Set the operation type */
         if ( AT_COMMAND_READ_E == command_type )
         {
            pCommand->ReadWrite = 0;
         }
         else
         {
            pCommand->ReadWrite = 1;
         }
         
         /*fill in the TransactionID */
         transaction_id++;
         transaction_id &= 7;
         pCommand->TransactionID = transaction_id;
         
         /*fill in the Destination */
         pCommand->Destination = gNodeAddress;
         
        /*fill in the CommandType */
         pCommand->CommandType = PARAM_TYPE_SVI_E;

//     /*read Parameter 1 */
//      msg_entry_index = 0;
//      while( count < buffer_length && msg_entry_index < (AT_PARAMETER_MAX_LENGTH - 1) && *pSource != ',' )
//      {
//         msg_entry[msg_entry_index] = *pSource;
//         msg_entry_index++;
//         pSource++;
//         count++;
//      }
//      msg_entry[msg_entry_index] = '\0';
//      pCommand->Parameter1 = (uint8_t)strtol((char*)msg_entry, &str_end, 10);
//      
//      if ( DM_SVI_PARAMETER_TYPE_MAX_E > pCommand->Parameter1 && !*str_end)
//      {
//         /* check if 'read' instruction */
//         if ( AT_COMMAND_READ_E == command_type )
//         {
//            /* it's a read command */
//            status = AT_NO_ERROR_E;
//         }
//         else
//         {
//            /* We need at least a coma and value */
//            if (*pSource == ',' && (count + 1) < buffer_length)
//            {
//                /* it's a write command.  Extract the value */
//               pSource++;/* step over the comma */
//               count++;
//               
//               msg_entry_index = 0;
//               while( count < buffer_length && msg_entry_index < (AT_PARAMETER_MAX_LENGTH - 1) )
//               {
//                  msg_entry[msg_entry_index] = *pSource;
//                  msg_entry_index++;
//                  pSource++;
//                  count++;
//               }
//               msg_entry[msg_entry_index] = '\0';
//               pCommand->Value = (uint8_t)strtol((char*)msg_entry, &str_end, 10);

//               if (!*str_end)
//               {
//                  status = AT_NO_ERROR_E;
//               }
//            }
//            else
//            {
//               status = AT_INVALID_PARAMETER_E;
//            }
//         }
//      }
//      else 
//      {
//         status = AT_INVALID_COMMAND_E;
//      }
//      
      }
   }
   else
   {
      status = AT_INVALID_PARAMETER_E;
   }
   
   return status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ParseOutputSignal
* description     : Unpack a raw AT message into a CO_OutputData_t structure.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - [OUT] pOutputSignal  A Structure to be populated with the results.
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ParseOutputSignal(const ATCommandType_t command_type, const uint8_t *const source_buffer, CO_OutputData_t* pOutputSignal)
{
   ATMessageStatus_t status = AT_INVALID_PARAMETER_E;
   char* pSource;
   char *str_end;
   
   if ( source_buffer && pOutputSignal )
   {
      status = AT_INVALID_COMMAND_E;
      pSource = (char*)source_buffer;

      /*read the Destination */
      uint16_t unit_address;
      uint16_t zone;
      status = MM_ATCommand_ExtractUnitAddressAndZone(source_buffer, &unit_address, &zone);
      
      if ( AT_NO_ERROR_E == status )
      {
         #ifdef IS_PPU
         pOutputSignal->UnitSerno = unit_address;
         #else
         pOutputSignal->Destination = unit_address;
         #endif
         pOutputSignal->zone = zone;

         /* find the first ',' */
         pSource = strchr((const char*)source_buffer, ',');
         if ( pSource )
         {
            pSource++;/* step over the comma */
            pOutputSignal->OutputChannel = (uint8_t)strtol((char*)pSource, &str_end, 10);
            pSource = str_end;

            if ( pSource )
            {
               pSource++;/* step over the comma */
               pOutputSignal->OutputProfile = (uint8_t)strtol((char*)pSource, &str_end, 10);
               pSource = str_end;
               
               if( pSource )
               {
                  pSource++;/* step over the comma */

                  /*read the 'outputs activated' */
                  pOutputSignal->OutputsActivated = (uint16_t)strtol((char*)pSource, &str_end, 10);
                  pSource = str_end;
                  if( pSource )
                  {
                     pSource++;/* step over the comma */

                     /*read the 'output duration' */
                     pOutputSignal->OutputDuration = (uint8_t)strtol((char*)pSource, &str_end, 10);
                     pSource = str_end;
                     if(!*pSource)
                     {
                        status = AT_NO_ERROR_E;
                     }
                  }
               }
            }
         }
      }
   }

   return status;
}
/*************************************************************************************/
/**
* function name   : MM_ATCommand_ParseCompoundTestSignal
* description     : Parse a test signal AT command into a TestModeMessage_t structure.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ParseCompoundTestSignal(const ATCommandType_t command_type,  const uint8_t *const source_buffer, const uint8_t buffer_length, ATTestMessageData_t* pTestMessage)
{
   ATMessageStatus_t status = AT_COMMAND_UNKNOWN_ERROR_E;
   uint8_t msg_entry[AT_PARAMETER_MAX_LENGTH];
   uint32_t msg_entry_index = 0;
   uint32_t count = 0;
   const uint8_t* pSource;
   char *str_end;
   
   if ( source_buffer && pTestMessage )
   {
      pSource = source_buffer;
      /*move past command string to the first value */
      while( count < buffer_length && *pSource != ',' )
      {
         count++;
         pSource++;
      }
      pSource++;/* step over the comma */
      count++;
      
      memset(msg_entry, 0, sizeof(msg_entry));
      /*read the char string (max 5 chars) */
      while( count < buffer_length && msg_entry_index < 5 && *pSource != ',' )
      {
         msg_entry[msg_entry_index] = *pSource;
         msg_entry_index++;
         pSource++;
         count++;
      }
      pTestMessage->stringMsg[0] = msg_entry[0];
      pTestMessage->stringMsg[1] = msg_entry[1];
      pTestMessage->stringMsg[2] = msg_entry[2];
      pTestMessage->stringMsg[3] = msg_entry[3];
      pTestMessage->stringMsg[4] = msg_entry[4];

      
      /* catch the case where more than 5 chars were provided.  just advance till we reach a comma */
      while ( *pSource != ',' )
      {
         pSource++;
      }
      pSource++;/* step over the comma */
      count++;
      
      /*read the counter value */
      memset(msg_entry, 0, sizeof(msg_entry));
      msg_entry_index = 0;
      while( count < buffer_length && msg_entry_index < (AT_PARAMETER_MAX_LENGTH - 1) && *pSource != ',' )
      {
         msg_entry[msg_entry_index] = *pSource;
         msg_entry_index++;
         pSource++;
         count++;
      }
      msg_entry[msg_entry_index] = '\0';
      pTestMessage->count = (uint16_t)strtol((char*)msg_entry, &str_end, 10);

      if(!*str_end)
      {
         status = AT_NO_ERROR_E;
      }
   }
   else
   {
      status = AT_INVALID_PARAMETER_E;
   }
   return status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_TestSignal
* description     : Handle a test signal rxd over the USART.
*                   Test messages are always TEST_PAYLOAD_SIZE in length and we only
*                   expect them if the test mode is TRANSPARENT or MONITORING.
*                   We reject the message for other test modes.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_TestSignal(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t status = AT_INVALID_COMMAND_E;
   ATTestMessageData_t testMessage;
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdSignalReq = NULL;   
     
   if ( MC_MAC_TEST_MODE_TRANSPARENT_E == gTestMode || MC_MAC_TEST_MODE_MONITORING_E == gTestMode )
   {
      if ( source_buffer && (TEST_PAYLOAD_SIZE == buffer_length))
      {
         memcpy((char*)&testMessage.stringMsg, source_buffer, TEST_PAYLOAD_SIZE);
         pCmdSignalReq = osPoolAlloc(AppPool);
         if ( pCmdSignalReq )
         {
            pCmdSignalReq->Type = CO_MESSAGE_GENERATE_TEST_MESSAGE_E;
            uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
            memcpy(pCmdReq, (uint8_t*)&testMessage, sizeof(ATTestMessageData_t));
            pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(ATTestMessageData_t);
            
            osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
            if (osOK != osStat)
            {
               /* failed to write */
               osPoolFree(AppPool, pCmdSignalReq);
            }
            else
            {
               status = AT_NO_ERROR_E;
            }
         }
      }
      else
      {
         status = AT_INVALID_PARAMETER_E;
      }
   }
   
   return status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_atTestMode
* description     : Get or Set the test mode for the unit
*                   ATMODE?
*                   ATMODE=<x>   where x=
*                                MC_MAC_TEST_MODE_OFF_E,                         //0
*                                MC_MAC_TEST_MODE_RECEIVE_E,                     //1
*                                MC_MAC_TEST_MODE_TRANSPARENT_E,                 //2
*                                MC_MAC_TEST_MODE_TRANSMIT_E,                    //3
*                                MC_MAC_TEST_MODE_MONITORING_E,                  //4
*                                MC_MAC_TEST_MODE_SLEEP_E,                       //5
*                                MC_MAC_TEST_MODE_NETWORK_MONITOR_E,             //6
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_atTestMode(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   uint32_t value = 0;
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   osStatus osStat;
   CO_CommandData_t CmdMessage;
   
   if ( source_buffer )
   {
      CmdMessage.TransactionID = 0;
      CmdMessage.Source = 0;
      CmdMessage.Destination = gNodeAddress;
      CmdMessage.Parameter1 = 0;
      CmdMessage.Parameter2 = 0;
      CmdMessage.CommandType = PARAM_TYPE_TEST_MODE_E;
      if ( AT_COMMAND_READ_E == command_type )
      {
         CmdMessage.ReadWrite = 0;
      }
      else
      {
         CmdMessage.ReadWrite = 1;
      }
      /* parse the value from the param buffer */
      if (EOF != (sscanf((char*)source_buffer, "%d", &value)) || AT_COMMAND_READ_E == command_type )
      {
         CmdMessage.Value = value;
         if ( (uint32_t)MC_MAC_TEST_MODE_MAX_E > value )
         {
            /* create test mode message and put into mesh queue */
            CO_Message_t* pCmdSignalReq = osPoolAlloc(AppPool);
            if (pCmdSignalReq)
            {
               pCmdSignalReq->Type = CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E;
               uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
               memcpy(pCmdReq, (uint8_t*)&CmdMessage, sizeof(CO_CommandData_t));
               pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_CommandData_t);
               
               osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCmdSignalReq);
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
                  gTestMode = (MC_MAC_TestMode_t)value;
                  if ( MC_MAC_TEST_MODE_TRANSPARENT_E == gTestMode  || MC_MAC_TEST_MODE_MONITORING_E == gTestMode )
                  {
                     /* Tell the ConfigSerialTask to operate in TRANSPARENT mode */
                     MM_SetTransparentMode(uart_port);
                  }
               }
            }
         }
      }
   }

   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_UnitReboot
* description     : Reboot a remote device (or local device if address matches, or is omitted)
*                   ATRRU+[ZxUx]
*                   ATR+[ZxUx]
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_UnitReboot(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   
   if ( source_buffer )
   {
      status = AT_INVALID_COMMAND_E;

      /*read the Destination */
      uint16_t unit_address = gNodeAddress;
      uint16_t zone = ZONE_GLOBAL;
      status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
      
      if ( AT_NO_ERROR_E == status )
      {
         if ( 2 == parameters.NumberOfParameters)
         {
            zone = parameters.Parameter[0];
            unit_address = parameters.Parameter[1];
         }
         
         if ( AT_COMMAND_SPECIAL_E == command_type )
         {
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_REBOOT_UNIT_E;
               pCommand->NodeID = unit_address;
               pCommand->Zone = zone;
               pCommand->ReadWrite = 1;/* write */
               pCommand->PortNumber = (uint8_t)uart_port;

               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  status = AT_NO_ERROR_E;
               }
            }
         }      
     }
   }
   
   return status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_UnitDisable
* description     : Disable a remote RBU
*                   ATEDR=ZxUx
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_UnitDisable(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      status = AT_INVALID_COMMAND_E;

      /*read the Destination */
      uint16_t unit_address;
      uint16_t zone;
      status = MM_ATCommand_ExtractUnitAddressAndZone(source_buffer, &unit_address, &zone);
      
      if ( AT_NO_ERROR_E == status )
      {
         if ( AT_COMMAND_WRITE_E == command_type )
         {
            /* Send the message to the App */
            CO_Message_t* pMsg = osPoolAlloc(AppPool);
            if ( pMsg )
            {
               pMsg->Type = CO_MESSAGE_GENERATE_RBU_DISABLE_MESSAGE_E;
               
               CO_CommandData_t* pCommandData = (CO_CommandData_t*)pMsg->Payload.PhyDataReq.Data;
               pCommandData->CommandType = PARAM_TYPE_REBOOT_E;
               pCommandData->Destination = unit_address;
               pCommandData->Parameter1 = 0;
               pCommandData->Parameter2 = 0;
               pCommandData->ReadWrite = 1;
               pCommandData->Source = gNodeAddress;
               pCommandData->SourceUSART = uart_port;

               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pMsg, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pMsg);
                  status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  status = AT_NO_ERROR_E;
               }
            }
         }      
     }
   }
   
   return status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_GetFirmwareInfo
* description     : Get the firmware version of the unit
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_GetFirmwareInfo(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   if ( (command_type == AT_COMMAND_READ_E) && !buffer_length)
   {
      osStatus status;
      // Allocate Pool
      Cmd_Reply_t *pCmdReply = NULL;
      pCmdReply = ALLOC_ATHANDLE_POOL
      
      if (pCmdReply)
      {
         strcpy((char*)pCmdReply->data_buffer, sw_version_nbr);
         strcat((char*)pCmdReply->data_buffer, ",");
         strcat((char*)pCmdReply->data_buffer, sw_version_date);
         char crc_string[20];
         sprintf(crc_string, ",0x%08X", blf_application_checksum);      

         strcat((char*)pCmdReply->data_buffer, crc_string);

         pCmdReply->length = (uint8_t)strlen((char*)pCmdReply->data_buffer);
         status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         if (osOK != status)
         {
            // failed to write
            FREE_ATHANDLE_POOL(pCmdReply);
            return AT_QUEUE_ERROR_E;
         }
         return AT_NO_ERROR_E;
      }
   }
   return AT_INVALID_PARAMETER_E;   
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_DeviceState
* description     : Set/get the mesh state for the unit
*                   ATMFS?
*                   ATMFS=state
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_DeviceState(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t status = AT_INVALID_COMMAND_E;
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdSignalReq = NULL;
   uint32_t value;

   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if (pCmdReply)
   {
      switch (command_type) 
      {
         case AT_COMMAND_READ_E:               
         {  
            if (0 == buffer_length) /*No parameter expected for read operation */
            {
               #ifdef IS_PPU
               value = PPU_IsDeviceActive();
               #else
               value = MC_STATE_GetDeviceState();
               #endif
               CO_HexString(value, (char *)pCmdReply->data_buffer, BASE_10);
               status = AT_NO_ERROR_E;
            }
         }
         break;

         case AT_COMMAND_WRITE_E:
         {    
            if ( source_buffer )
            {
               char *str_end;

               /* Get the state value */
               value = (uint16_t)strtol((char*)source_buffer, &str_end, 10);

               if(!*str_end)
               {
                  pCmdSignalReq = osPoolAlloc(AppPool);
                  if ( pCmdSignalReq )
                  {
                     CO_CommandData_t CmdMessage;
                     CmdMessage.Source = uart_port;
                     CmdMessage.Value = value;
                     
                     pCmdSignalReq->Type = CO_MESSAGE_GENERATE_SET_STATE_E;
                     
                     uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
                     
                     memcpy(pCmdReq, (uint8_t*)&CmdMessage, sizeof(CO_CommandData_t));
                     pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_CommandData_t);

                     osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
                     if (osOK != osStat)
                     {
                        /* failed to write */
                        osPoolFree(AppPool, pCmdSignalReq);
                        status = AT_QUEUE_ERROR_E;
                     }
                     else
                     {
                        status = AT_NO_ERROR_E;
                        // Set up reply string for later
                        strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                     }
                  }
                  else
                  {
                     status = AT_QUEUE_ERROR_E;
                  }
               }
            }
         } 
         break;

         default: 
         {
            status = AT_INVALID_COMMAND_E;
         }
         break;
      }

      if (AT_NO_ERROR_E == status)
      {  
         pCmdReply->port_id = uart_port;
         pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);

         osStat = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK == osStat)
         {
            status = AT_NO_ERROR_E;
         }
         else
         {
            FREE_ATHANDLE_POOL(pCmdReply);
            status = AT_QUEUE_ERROR_E;
         }
      }
      else
      {
         FREE_ATHANDLE_POOL(pCmdReply);
      }
   }
   return status;
}

/*************************************************************************************/
/**
* function name   : MM_ATEncodeSerialNumber
* description     : Converts a human readable serial number into a uint32_t.
*                   Used for encoding a serial number for storing in NVM.
*
* @param - pParamBuffer    Serial number in string format YYMM-BB-NNNN
* @param - pValue          [OUT] parameter for returning the encoded serial number.
*
* @return - bool           TRUE if the encoding was successful, FALSE otherwise.
*/
bool MM_ATEncodeSerialNumber(const char* pParamBuffer, uint32_t *pValue)
{
   bool result = false;
   uint32_t year;
   uint32_t month;
   uint32_t batch;
   uint32_t serial_number;
   int8_t digit = 0;
   
   if ( pParamBuffer && pValue )
   {
      const char* pChar = pParamBuffer;
      
      /* do a format check */
      bool format_ok = true;
      if (SERIAL_NUMBER_STR_LENGTH == strlen(pChar))
      {
         for (uint32_t index = 0; index < SERIAL_NUMBER_STR_LENGTH; index++)
         {
            if ((4 == index || 7 == index)) /* check for dashes in the right place */
            {
               if (pChar[index] != '-')
               {
                  CO_PRINT_B_1(DBG_INFO_E,"SerNo format wrong char - %d\r\n", index);
                  format_ok = false;
                  break;
               }
            }
            else if (pChar[index] < '0' || pChar[index] > '9') /* check for decimal numerals only */
            {
               CO_PRINT_B_1(DBG_INFO_E,"SerNo digit wrong - char %d\r\n", index);
               format_ok = false;
               break;
            }
         }
      }
      else
      {
         CO_PRINT_B_2(DBG_INFO_E,"SerNo length wrong - %d(%d)\r\n", strlen(pChar), SERIAL_NUMBER_STR_LENGTH);
         format_ok = false;
      }

      
      if ( format_ok )
      {
         /* Read the year */
         digit = (*pChar - '0');
         if ( digit >= 0 && digit < 10 )
         {
            year =  digit * 10;
            pChar++;
            digit = (*pChar - '0');
            if ( digit >= 0 && digit < 10 )
            {
               year +=  digit;
               /* Read the month */
               pChar++;
               digit = (*pChar - '0');
               if ( digit >= 0 && digit < 10 )
               {
                  month = digit * 10;
                  pChar++;
                  digit = (*pChar - '0');
                  if ( digit >= 0 && digit < 10 )
                  {
                     month += digit;
                     if ( 0 < month && 13 > month )
                     {
                        pChar++;
                        /* should be a '-' */
                        if ( '-' == *pChar )
                        {
                           pChar++;
                           /* Read the batch number */
                           if (EOF != (sscanf(pChar, "%d", &batch)))
                           {
                              /* jump over the BB field */
                              pChar+=2;
                              /* should be a '-' */
                              if ( '-' == *pChar )
                              {
                                 pChar++;
                                 /* Read the serial number */
                                 if (EOF != (sscanf(pChar, "%d", &serial_number)))
                                 {
                                    //check that it fits into 14 bits
                                    if ( 0xC000 > serial_number )
                                    {
                                       *pValue = (year << 25) + (month << 21) + (batch << 14) + serial_number;
                                       /* success */
                                       result = true;
                                       //CO_PRINT_B_2(DBG_INFO_E,"SerNo OK - %s : %d\r\n", pParamBuffer, *pValue);
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* function name   : MM_ATDecodeSerialNumber
* description     : Converts a uint32_t serial number into a human readable string.
*                   Used for decoding a serial number read from NVM.
*
* @param - pValue          The 32 bit encoded serial number from NVM.
* @param - pParamBuffer    [OUT] Decoded serial number in string format YYMM-BB-NNNNN
*
* @return - bool           TRUE if the decoding was successful, FALSE therwise.
*/
bool MM_ATDecodeSerialNumber(const uint32_t value, char* pParamBuffer)
{
   bool result = false;
   uint32_t year;
   uint32_t month;
   uint32_t batch;
   uint32_t serial_number;
   
   if ( pParamBuffer )
   {
      year = (value >> 25) & 0x7f;
      month = (value >> 21) & 0xf;
      batch = (value >> 14) & 0x7f;
      serial_number = value & 0x3fff;
      sprintf(pParamBuffer,"%02d%02d-%02d-%04d", year, month, batch, serial_number);
      result = true;
   }
   
   return result;
}

/*************************************************************************************/
/**
* function name   : MM_ATEncodeProductCode
* description     : Converts a human readable product code into a uint32_t.
*                   Used for encoding a product code for storing in NVM.
*
* @param - pParamBuffer    Product code in string format
* @param - pValue          [OUT] parameter for returning the encoded product code.
*
* @return - bool           TRUE if the encoding was successful, FALSE otherwise.
*/
bool MM_ATEncodeProductCode(const char* pParamBuffer, uint32_t *pValue)
{
   bool result = false;
   uint32_t product_code;

   if ( pParamBuffer && pValue )
   {
      product_code = (uint32_t)pParamBuffer[0];
      product_code += ((uint32_t)pParamBuffer[1] << 8);
      product_code += ((uint32_t)pParamBuffer[2] << 16);
      product_code += ((uint32_t)pParamBuffer[3] << 24);
      *pValue = product_code;
      result = true;
   }
   
   return result;
}

/*************************************************************************************/
/**
* function name   : MM_ATDecodeProductCode
* description     : Converts a uint32_t product code into a human readable string.
*                   Used for decoding a product code read from NVM.
*
* @param - pValue          The 32 bit encoded product code from NVM.
* @param - pParamBuffer    [OUT] Decoded product code in string format
*
* @return - bool           TRUE if the decoding was successful, FALSE therwise.
*/
bool MM_ATDecodeProductCode(const uint32_t value, char* pParamBuffer)
{
   bool result = false;
   char product_code[5];
   
   if ( pParamBuffer )
   {
      product_code[4] = 0;//str terminator
      product_code[3] = (value >> 24) & 0xff;
      product_code[2] = (value >> 16) & 0xff;
      product_code[1] = (value >> 8) & 0xff;
      product_code[0] = value & 0xff;
      sprintf(pParamBuffer,"%s", product_code);
      CO_PRINT_B_1(DBG_INFO_E,"MM_ATDecodeProductCode - %s\r\n", pParamBuffer);
      result = true;
   }
   else 
   {
      CO_PRINT_B_0(DBG_INFO_E,"MM_ATDecodeProductCode - BAD PARAM pParamBuffer\r\n");
   }
   
   return result;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_RBUDisable
* description     : Send a command to disable an RBU
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_RBUDisable(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t status = AT_INVALID_COMMAND_E;
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdSignalReq = NULL;
   uint32_t value;

   if ( source_buffer )
   {
      if (EOF != sscanf((char*)source_buffer, "%d", &value))
      {
         pCmdSignalReq = osPoolAlloc(AppPool);
         if ( pCmdSignalReq )
         {
            CO_CommandData_t CmdMessage;
            CmdMessage.SourceUSART = uart_port;
            CmdMessage.Destination = value;
            
            pCmdSignalReq->Type = CO_MESSAGE_GENERATE_RBU_DISABLE_MESSAGE_E;
            
            uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
            
            memcpy(pCmdReq, (uint8_t*)&CmdMessage, sizeof(CO_CommandData_t));
            pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_CommandData_t);

            osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
            if (osOK != osStat)
            {
               /* failed to write */
               osPoolFree(AppPool, pCmdSignalReq);
               status = AT_QUEUE_ERROR_E;
            }
            else
            {
               status = AT_NO_ERROR_E;
            }
         }
         else
         {
            status = AT_QUEUE_ERROR_E;
         }
      }
   }
   else
   {
      status = AT_INVALID_PARAMETER_E;
   }
   
   return status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ReportAssociatedNodes
* description     : Send a command to the session manager to report its associated nodes over 
*                   the requesting uart
*                   ATNODES+
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ReportAssociatedNodes(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t status = AT_COMMAND_UNKNOWN_ERROR_E;
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdSignalReq = NULL;

   pCmdSignalReq = osPoolAlloc(AppPool);
   if ( pCmdSignalReq )
   {
      CO_CommandData_t CmdMessage;
      CmdMessage.Source = uart_port;
      
      pCmdSignalReq->Type = CO_MESSAGE_GENERATE_RBU_REPORT_MESSAGE_E;
      
      uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
      
      memcpy(pCmdReq, (uint8_t*)&CmdMessage, sizeof(CO_CommandData_t));
      pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_CommandData_t);

      osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
      if (osOK == osStat)
      {
         status = AT_NO_ERROR_E;
      }
      else
      {
         /* failed to write */
         osPoolFree(AppPool, pCmdSignalReq);
         status = AT_QUEUE_ERROR_E;
      }
   }
   else
   {
      status = AT_QUEUE_ERROR_E;
   }

   
   if( AT_NO_ERROR_E == status )
   {
      osStatus ostatus;
      Cmd_Reply_t *pCmdReply = NULL;
      pCmdReply = CALLOC_ATHANDLE_POOL
      if ( pCmdReply )
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
         pCmdReply->port_id = uart_port;
         ostatus = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK != ostatus)
         {
            FREE_ATHANDLE_POOL(pCmdReply);
         }
      }
   }

   
   return status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_FirmwareImageCtrl
* description     : Gets / Sets active image in firmware control
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_FirmwareImageCtrl(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t update_eeprom)
{
   // Set up the parameters
//   uint32_t bank = 0;
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   osStatus status;

   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if (pCmdReply)
   {
      switch (command_type) 
      {
         case AT_COMMAND_READ_E:
         {
            /* This command doesn't expect any additional parameter in read operation*/
            if (0 == buffer_length)
            {
               // Read the address
               msg_status = AT_NO_ERROR_E;
               CO_HexString(1, (char *)&pCmdReply->data_buffer, BASE_10);
            }
         }
         break;
       
//         case AT_COMMAND_WRITE_E:
//         {
//            msg_status = AT_INVALID_PARAMETER_E;
//            if ( source_buffer )
//            {
//               if (0 < buffer_length)
//               {
//                  char *str_end;

//                  /* Get the bank value */
//                  bank = (uint16_t)strtol((char*)source_buffer, &str_end, 10);

//                  if(!*str_end)
//                  {
//                     if (SUCCESS_E == BLF_set_boot_bank(bank, update_eeprom))
//                     {
//                        /* We are not supposed to return if successful
//                         * Just in case this changes, return a positive response
//                         */
//                        msg_status = AT_NO_ERROR_E;
//                        strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
//                     }
//                  }
//               }
//            }
//         }
//         break;

         default:
         {
            msg_status = AT_INVALID_COMMAND_E;
         }
      }
      
      //If the command failed, free the memory.
      if (AT_NO_ERROR_E != msg_status)
      {
         FREE_ATHANDLE_POOL(pCmdReply);
      }
   }
   
   if (AT_NO_ERROR_E == msg_status)
   {  
      pCmdReply->port_id = uart_port;
      pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
      status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         // failed to write
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
      msg_status = AT_NO_ERROR_E;
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_BuiltInTestReq
* description     : Request the Built-In Test of the unit
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_BuiltInTestReq(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   uint32_t deviceCombination;
   ATMessageStatus_t status = AT_NO_ERROR_E;
   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if ( pCmdReply )
   {
      if ( 0 == buffer_length )
      {
         if ( SUCCESS_E == DM_NVMRead(NV_DEVICE_COMBINATION_E, &deviceCombination, sizeof(deviceCombination)) )
         {
            /* Run the Radio BIT */
            if ( SUCCESS_E != MM_MAC_BuiltInTestReq())
            {
               /* BIT request was rejected */
               status = AT_COMMAND_UNKNOWN_ERROR_E;
            }

            /* Run the Head Interface BIT if present */
            if (true == DM_DeviceIsEnabled(deviceCombination, DEV_HEAD_DEVICES, DC_MATCH_ANY_E ))
            {
               if ( SUCCESS_E != MM_HeadInterface_BuiltInTestReq())
               {
                  /* BIT request was rejected */
                  status = AT_COMMAND_UNKNOWN_ERROR_E;
               }
            }

            /* Run the SVI BIT if present */
            #ifndef I2C_SVI_BOARD_SIMULATION
            if (true == DM_DeviceIsEnabled(deviceCombination, DEV_SVI_DEVICES, DC_MATCH_ANY_E ))
            {
               if ( SUCCESS_E != DM_SVI_BitTest())
               {
                  /* BIT request was rejected */
                  status = AT_COMMAND_UNKNOWN_ERROR_E;
               }
            }
            #endif
         }
         else 
         {
            status = AT_NVM_ERROR_E;
         }
      }
      else
      {
         /* Command syntax not correct */
         status = AT_COMMAND_UNKNOWN_ERROR_E;
      }
      
      if ( AT_NO_ERROR_E == status )
      {
         //All went well.  Send an OK response
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
         pCmdReply->port_id = uart_port;
         pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
         
         if (osOK != osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0))
         {
            // failed to write
            FREE_ATHANDLE_POOL(pCmdReply);
           status = AT_QUEUE_ERROR_E;
         }
      }
      else
      {
         //Command failed.  Free response memory
         FREE_ATHANDLE_POOL(pCmdReply);
      }
   }

   return status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ZoneNumber
* description     : Get and Set the zone number of the unit.
*                    ATZONE?
*                    ATZONE=
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ZoneNumber(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      /* Use the common function MM_ATCommand_decimalNumber to perform the read/write to NVM */
      msg_status = MM_ATCommand_decimalNumber(command_type, source_buffer, buffer_length, uart_port, param_id);
      
      /* If the zone number was successfully updated, send the new value to the application */
      if ( (AT_NO_ERROR_E == msg_status) && (AT_COMMAND_WRITE_E == command_type) )
      {
         if ( 0 < buffer_length )
         {
            CO_Message_t* pCmdSignalReq = osPoolAlloc(AppPool);
            if ( pCmdSignalReq )
            {
               char *str_end;
               
               CO_StatusIndicationData_t zoneMessage;
               zoneMessage.Event = CO_MESH_EVENT_ZONE_CHANGE_E;
               /* Read the zone number */
               zoneMessage.EventData = strtol((char*)source_buffer, &str_end, 10);
                     
               pCmdSignalReq->Type = CO_MESSAGE_MESH_STATUS_EVENT_E;
               ApplicationMessage_t appMsg;
               appMsg.MessageType = APP_MSG_TYPE_STATUS_SIGNAL_E;
               memcpy(appMsg.MessageBuffer, (uint8_t*)&zoneMessage, sizeof(CO_StatusIndicationData_t));
               
               uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
               memcpy(pCmdReq, (uint8_t*)&appMsg, sizeof(ApplicationMessage_t));
               pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(ApplicationMessage_t);

               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCmdSignalReq);
               }
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_brandingID
* description     : Get and Set the Branding ID of the unit.
*                   ATBID?
*                   ATBID=value
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_brandingID(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   if ( source_buffer )
   {
      /* Use the common function MM_ATCommand_decimalNumber to perform the read/write to NVM */
      msg_status = MM_ATCommand_decimalNumber(command_type, source_buffer, buffer_length, uart_port, param_id);
   }
      
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ReadMessageQueues
* description     : Get the current state of the message queues (number of messages waiting).
*                    ATMSN?
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ReadMessageQueues(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( AT_COMMAND_READ_E == command_type )
   {
      CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
      if ( pCIEReq )
      {
         pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
         
         CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
         pCommand->CommandType = CIE_CMD_READ_MESSAGE_QUEUE_STATUS_E;
         pCommand->PortNumber = (uint8_t)uart_port;

         osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCIEReq);
            msg_status = AT_QUEUE_ERROR_E;
         }
         else
         {
            msg_status = AT_NO_ERROR_E;
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ReadZonesAndDevices
* description     : Get the number of devices and zones that have been seen by the NCU.
*                   ATNOD?
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ReadZonesAndDevices(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( AT_COMMAND_READ_E == command_type )
   {
      CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
      if ( pCIEReq )
      {
         pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
         
         CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
         pCommand->CommandType = CIE_CMD_READ_ZONES_AND_DEVICES_E;
         pCommand->PortNumber = (uint8_t)uart_port;

         osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCIEReq);
            msg_status = AT_QUEUE_ERROR_E;
         }
         else
         {
            msg_status = AT_NO_ERROR_E;
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ReadFireMessageQueue
* description     : Read the message at the front of the CIE Fire message queue.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ReadFireMessageQueue(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( AT_COMMAND_READ_E == command_type )
   {
      CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
      if ( pCIEReq )
      {
         pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
         
         CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
         pCommand->CommandType = CIE_CMD_READ_FIRE_MESSAGE_E;
         pCommand->PortNumber = (uint8_t)uart_port;

         osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCIEReq);
            msg_status = AT_QUEUE_ERROR_E;
         }
         else
         {
            msg_status = AT_NO_ERROR_E;
         }
      }
   }
      
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ReadAlarmMessageQueue
* description     : Read the message at the front of the CIE Alarm message queue.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ReadAlarmMessageQueue(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( AT_COMMAND_READ_E == command_type )
   {
      CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
      if ( pCIEReq )
      {
         pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
         
         CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
         pCommand->CommandType = CIE_CMD_READ_ALARM_MESSAGE_E;
         pCommand->PortNumber = (uint8_t)uart_port;

         osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCIEReq);
            msg_status = AT_QUEUE_ERROR_E;
         }
         else
         {
            msg_status = AT_NO_ERROR_E;
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ReadFaultMessageQueue
* description     : Read the message at the front of the CIE Fault message queue.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ReadFaultMessageQueue(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( AT_COMMAND_READ_E == command_type )
   {
      CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
      if ( pCIEReq )
      {
         pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
         
         CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
         pCommand->CommandType = CIE_CMD_READ_FAULT_MESSAGE_E;
         pCommand->PortNumber = (uint8_t)uart_port;

         osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCIEReq);
            msg_status = AT_QUEUE_ERROR_E;
         }
         else
         {
            msg_status = AT_NO_ERROR_E;
         }
      }
   } 
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ReadMiscMessageQueue
* description     : Read the message at the front of the CIE Misc message queue.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ReadMiscMessageQueue(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( AT_COMMAND_READ_E == command_type )
   {
      CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
      if ( pCIEReq )
      {
         pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
         
         CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
         pCommand->CommandType = CIE_CMD_READ_MISC_MESSAGE_E;
         pCommand->PortNumber = (uint8_t)uart_port;

         osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCIEReq);
            msg_status = AT_QUEUE_ERROR_E;
         }
         else
         {
            msg_status = AT_NO_ERROR_E;
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_RemoveFromFireMessageQueue
* description     : Remove the message at the front of the CIE Fire message queue.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_RemoveFromFireMessageQueue(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( AT_COMMAND_SPECIAL_E == command_type )
   {
      CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
      if ( pCIEReq )
      {
         pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
         
         CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
         pCommand->CommandType = CIE_CMD_DISCARD_FIRE_MESSAGE_E;
         pCommand->PortNumber = (uint8_t)uart_port;

         osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCIEReq);
            msg_status = AT_QUEUE_ERROR_E;
         }
         else
         {
            msg_status = AT_NO_ERROR_E;
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_RemoveFromAlarmMessageQueue
* description     : Remove the message at the front of the CIE Alarm message queue.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_RemoveFromAlarmMessageQueue(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( AT_COMMAND_SPECIAL_E == command_type )
   {
      CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
      if ( pCIEReq )
      {
         pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
         
         CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
         pCommand->CommandType = CIE_CMD_DISCARD_ALARM_MESSAGE_E;
         pCommand->PortNumber = (uint8_t)uart_port;

         osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCIEReq);
            msg_status = AT_QUEUE_ERROR_E;
         }
         else
         {
            msg_status = AT_NO_ERROR_E;
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_RemoveFromFaultMessageQueue
* description     : Remove the message at the front of the CIE Fault message queue.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_RemoveFromFaultMessageQueue(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( AT_COMMAND_SPECIAL_E == command_type )
   {
      CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
      if ( pCIEReq )
      {
         pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
         
         CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
         pCommand->CommandType = CIE_CMD_DISCARD_FAULT_MESSAGE_E;
         pCommand->PortNumber = (uint8_t)uart_port;

         osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCIEReq);
            msg_status = AT_QUEUE_ERROR_E;
         }
         else
         {
            msg_status = AT_NO_ERROR_E;
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_RemoveFromMiscMessageQueue
* description     : Remove the message at the front of the CIE Misc message queue.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_RemoveFromMiscMessageQueue(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( AT_COMMAND_SPECIAL_E == command_type )
   {
      CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
      if ( pCIEReq )
      {
         pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
         
         CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
         pCommand->CommandType = CIE_CMD_DISCARD_MISC_MESSAGE_E;
         pCommand->PortNumber = (uint8_t)uart_port;

         osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCIEReq);
            msg_status = AT_QUEUE_ERROR_E;
         }
         else
         {
            msg_status = AT_NO_ERROR_E;
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ClearMessageQueues
* description     : Clears message queues and lost message counters according to the
*                   settings in the source buffer.
*                   The source buffer is a list of 1's and 0's. A '1' means clear the
*                   queue/count and a '0' means don't clear. The format is as follows.
*                   
*                   FireQ,FireLostMsg<LF>AlarmQ,AlarmLostMsg<LF>FaultQ,FaultLostMsg<LF>
*                   MiscQ,MiscLostMsg<LF>TxBufferQ
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ClearMessageQueues(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( source_buffer )
   {
      if ( AT_COMMAND_WRITE_E == command_type )
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
            CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
            pCommand->CommandType = CIE_CMD_RESET_SELECTED_QUEUE_E;
            pCommand->PortNumber = (uint8_t)uart_port;
            
            /* parse the source buffer to identify which properties to clear */
            /* check the format separators are as expected */
            if ( ',' == source_buffer[1] && 0x0A == source_buffer[3] &&
                 ',' == source_buffer[5] && 0x0A == source_buffer[7] &&
                 ',' == source_buffer[9] && 0x0A == source_buffer[11] &&
                 ',' == source_buffer[13] && 0x0A == source_buffer[15] &&
                  /* check that the values are all '1' or '0' */
                 ('0' == source_buffer[0] || '1' == source_buffer[0]) &&
                 ('0' == source_buffer[2] || '1' == source_buffer[2]) &&
                 ('0' == source_buffer[4] || '1' == source_buffer[4]) &&
                 ('0' == source_buffer[6] || '1' == source_buffer[6]) &&
                 ('0' == source_buffer[8] || '1' == source_buffer[8]) &&
                 ('0' == source_buffer[10] || '1' == source_buffer[10]) &&
                 ('0' == source_buffer[12] || '1' == source_buffer[12]) &&
                 ('0' == source_buffer[14] || '1' == source_buffer[14]) &&
                 ('0' == source_buffer[16] || '1' == source_buffer[16])  )
            {
               /* Fire Queue */
               pCommand->QueueProperty[CIE_Q_FIRE_E] = '0' == source_buffer[0] ? false:true;
               pCommand->QueueProperty[CIE_Q_FIRE_LOST_COUNT_E] = '0' == source_buffer[2] ? false:true;

               /* Alarm Queue */
               pCommand->QueueProperty[CIE_Q_ALARM_E] = '0' == source_buffer[4] ? false:true;
               pCommand->QueueProperty[CIE_Q_ALARM_LOST_COUNT_E] = '0' == source_buffer[6] ? false:true;

               /* Fault Queue */
               pCommand->QueueProperty[CIE_Q_FAULT_E] = '0' == source_buffer[8] ? false:true;
               pCommand->QueueProperty[CIE_Q_FAULT_LOST_COUNT_E] = '0' == source_buffer[10] ? false:true;

               /* Misc Queue */
               pCommand->QueueProperty[CIE_Q_MISC_E] = '0' == source_buffer[12] ? false:true;
               pCommand->QueueProperty[CIE_Q_MISC_LOST_COUNT_E] = '0' == source_buffer[14] ? false:true;

               /* TxBuffer Queue */
               pCommand->QueueProperty[CIE_Q_TX_BUFFER_E] = '0' == source_buffer[16] ? false:true;
               
               msg_status = AT_NO_ERROR_E;
            }
            
            if ( AT_NO_ERROR_E == msg_status )
            {
               /* Send the message to the App */
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
            }
            else 
            {
               osPoolFree(AppPool, pCIEReq);
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_RequestDeviceCombination
* description     : Request the device combination of a remote unit.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_RequestDeviceCombination(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;

   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
            /* Get the target node */
            uint16_t unit_address;
            uint16_t zone;
            msg_status = MM_ATCommand_ExtractUnitAddressAndZone(source_buffer, &unit_address, &zone);

            if( AT_NO_ERROR_E == msg_status )
            {
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_READ_DEVICE_COMBINATION_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = unit_address;
               pCommand->Parameter1 = 0;
               pCommand->Parameter2 = 0;
               pCommand->ReadWrite = 0;
               pCommand->Value = 0;
               pCommand->Zone = zone;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
            else 
            {
               osPoolFree(AppPool, pCIEReq);
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_RequestStatusFlags
* description     : Request the device status flags of a remote unit.
*                   ATDSF?ZxUx
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_RequestStatusFlags(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
            /* Get the target node */
            uint16_t unit_address;
            uint16_t zone;
            msg_status = MM_ATCommand_ExtractUnitAddressAndZone(source_buffer, &unit_address, &zone);

            if( AT_NO_ERROR_E == msg_status )
            {
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_READ_DEVICE_STATUS_FLAGS_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = unit_address;
               pCommand->Parameter1 = 0;
               pCommand->Parameter2 = 0;
               pCommand->ReadWrite = 0;
               pCommand->Value = 0;
               pCommand->Zone = zone;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
            else 
            {
               osPoolFree(AppPool, pCIEReq);
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_RequestStatusRequest
* description     : Request the device Mesh Status of a remote unit.
*                    ATMSR?ZxUx[,remote]   remote=0 for local cache read, remote>0 for network request
*                    ATMSR?<zone>,<nodeid>[<remote>   Allows for ATMSR?-1,-1[,r]
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_RequestStatusRequest(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   
   if ( source_buffer )
   {
      //Read the fault state
      msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
      
      if ( 1 < parameters.NumberOfParameters )
      {

         if ( AT_COMMAND_READ_E == command_type )
         {
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               
               /* Get the target node */
               uint16_t zone = parameters.Parameter[0];;
               uint16_t unit_address = parameters.Parameter[1];
               uint8_t remote_request = 0;
               if ( 2 < parameters.NumberOfParameters )
               {
                  remote_request = parameters.Parameter[2];
               }
               
               if( AT_NO_ERROR_E == msg_status )
               {
                  CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
                  pCommand->CommandType = CIE_CMD_READ_DEVICE_MESH_STATUS_E;
                  pCommand->PortNumber = (uint8_t)uart_port;
                  pCommand->NodeID = unit_address;
                  pCommand->Parameter1 = remote_request;
                  pCommand->Parameter2 = zone;
                  pCommand->ReadWrite = 0;
                  pCommand->Value = 0;
                  pCommand->Zone = zone;
                  
                  osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
                  if (osOK != osStat)
                  {
                     /* failed to write */
                     osPoolFree(AppPool, pCIEReq);
                     msg_status = AT_QUEUE_ERROR_E;
                  }
                  else
                  {
                     msg_status = AT_NO_ERROR_E;
                  }
               }
               else 
               {
                  osPoolFree(AppPool, pCIEReq);
               }
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_RequestRBUSerialNumber
* description     : Request the device serial number of a remote unit.
*                   ATSNR?ZxUx
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_RequestRBUSerialNumber(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
            /* Get the target node */
            uint16_t unit_address;
            uint16_t zone;
            msg_status = MM_ATCommand_ExtractUnitAddressAndZone(source_buffer, &unit_address, &zone);

            if( AT_NO_ERROR_E == msg_status )
            {
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_READ_DEVICE_SERIAL_NUMBER_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = unit_address;
               pCommand->Parameter1 = 0;
               pCommand->Parameter2 = 0;
               pCommand->ReadWrite = 0;
               pCommand->Value = 0;
               pCommand->Zone = zone;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
            else 
            {
               osPoolFree(AppPool, pCIEReq);
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_TestMode
* description     : Set the test mode of an RBU.  Note that no reponse is expected
*                   because the RBU will leave the mesh.
*                   ATTMR=ZxUx,<test mode>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_TestMode(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_WRITE_E == command_type )
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
            /* Get the target node */
            uint16_t unit_address;
            uint16_t zone;
            msg_status = MM_ATCommand_ExtractUnitAddressAndZone(source_buffer, &unit_address, &zone);

            /* read the test mode value */
            uint32_t mode_number = 0;
            char* pMode = strchr((char*)source_buffer, ',');
            if ( pMode )
            {
               pMode++;
               char *str_end;
               mode_number = (uint16_t)strtol(pMode, &str_end, 10);
               if ( *str_end )
               {
                  msg_status = AT_INVALID_COMMAND_E;
               }
            }

            if( AT_NO_ERROR_E == msg_status )
            {
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_SET_TEST_MODE_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = unit_address;
               pCommand->Value = mode_number;
               pCommand->Parameter1 = 0;
               pCommand->Parameter2 = 0;
               pCommand->ReadWrite = 1;
               pCommand->Value = 0;
               pCommand->Zone = zone;
              
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
            else 
            {
               osPoolFree(AppPool, pCIEReq);
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ReadAnalogueValue
* description     : Read the analogue value from an RBU channel.
*                   ATANA?ZxUx,<channel>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ReadAnalogueValue(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint16_t unit_address;
   uint16_t zone;
   uint32_t channel_number;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( AT_NO_ERROR_E == msg_status )
         {
            msg_status = AT_INVALID_PARAMETER_E;
            if ( 3 == parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               unit_address = parameters.Parameter[1];
               channel_number = parameters.Parameter[2];
               msg_status = AT_NO_ERROR_E;
            }
            else if ( 1 == parameters.NumberOfParameters )
            {
               zone = ZONE_GLOBAL;
               unit_address = gNodeAddress;
               channel_number = parameters.Parameter[0];
               msg_status = AT_NO_ERROR_E;
            }
         }
         
         if ( AT_NO_ERROR_E == msg_status )
         {
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_READ_ANALOGUE_VALUE_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = unit_address;
               pCommand->Value = channel_number;
               pCommand->Parameter1 = 0;
               pCommand->Parameter2 = 0;
               pCommand->ReadWrite = 0;
               pCommand->Zone = zone;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
            else 
            {
               osPoolFree(AppPool, pCIEReq);
            }
         }
      }
   }
	  
	Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL;
	 
   if ( pCmdReply )
   {
      pCmdReply->port_id = uart_port;
      if ( AT_NO_ERROR_E == msg_status )
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
      }
      else 
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      }
      
      /* Send the response */
      osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         /* Failed to write, free the pre-allocated pool entry */
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
	 }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_BeaconFlashRate
* description     : Read or write the beacon flash rate on an RBU.
*                   ATBFR?<ZxUx>,<channel>,<profile>,<value>
*                   ATBFR=<ZxUx>,<channel>,<profile>,<value>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_BeaconFlashRate(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   uint16_t unit_address;
   uint16_t zone;
   uint32_t channel_number = (uint32_t)CO_CHANNEL_BEACON_E;
   uint32_t output_profile = (uint32_t)CO_PROFILE_FIRE_E;
   uint32_t value = 0;
   
   if ( source_buffer )
   {
      /* Get the target node */
      msg_status = MM_ATCommand_ExtractUnitAddressAndZone(source_buffer, &unit_address, &zone);
      
      if ( AT_NO_ERROR_E == msg_status )
      {

         /* read the channel number*/
         
         char* pChannel = strchr((char*)source_buffer, ',');
         if ( pChannel )
         {
            pChannel++;
            char *str_end;
            channel_number = (uint16_t)strtol(pChannel, &str_end, 10);
            if ( 0 != *str_end )
            {
               /* read the output profile */
               pChannel = str_end;
               pChannel++;
               output_profile = (uint32_t)strtol(pChannel, &str_end, 10);
                if ( 0 != *str_end )
               {
                  /* read the value */
                  pChannel = str_end;
                  pChannel++;
                  value = (uint32_t)strtol(pChannel, &str_end, 10);
                  
                  if ( 0 == *str_end )
                  {
                     msg_status = AT_NO_ERROR_E;
                  }
                  
               }
               else 
               {
                  msg_status = AT_INVALID_COMMAND_E;
               }
              
            }
            else 
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
            
         }

         if (AT_NO_ERROR_E == msg_status ) 
         {
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               
               if ( AT_COMMAND_READ_E == command_type )
               {
                  pCommand->ReadWrite = 0;
               }
               else if ( AT_COMMAND_WRITE_E == command_type )
               {
                  pCommand->ReadWrite = 1;
               }
               else 
               {
                  msg_status = AT_INVALID_COMMAND_E;
               }
               
               if ( AT_NO_ERROR_E == msg_status )
               {
                  pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
                  
                  pCommand->CommandType = CIE_CMD_BEACON_FLASH_RATE_E;
                  pCommand->PortNumber = (uint8_t)uart_port;
                  pCommand->NodeID = unit_address;
                  pCommand->Zone = zone;
                  pCommand->Parameter1 = channel_number;
                  pCommand->Parameter2 = output_profile;
                  pCommand->Value = value;
                  
                  osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
                  if (osOK != osStat)
                  {
                     /* failed to write */
                     osPoolFree(AppPool, pCIEReq);
                     msg_status = AT_QUEUE_ERROR_E;
                  }
                  else
                  {
                     msg_status = AT_NO_ERROR_E;
                  }
               }
               else 
               {
                  osPoolFree(AppPool, pCIEReq);
               }
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_EnablePlugin
* description     : Enable or disable a plugin channel on an RBU.
*                   ATEDP?ZxUx,channel
*                   ATEDP=ZxUx,channel,value
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_EnablePlugin(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   uint16_t unit_address;
   uint16_t zone;
   uint32_t channel_number;
   uint32_t value;
   
   if ( source_buffer )
   {
      /* Get the target node */
      msg_status = MM_ATCommand_ExtractUnitAddressAndZone(source_buffer, &unit_address, &zone);
      
      if ( AT_NO_ERROR_E == msg_status )
      {

         /* read the channel number*/
         
         char* pChannel = strchr((char*)source_buffer, ',');
         if ( pChannel )
         {
            pChannel++;
            char *str_end;
            channel_number = (uint16_t)strtol(pChannel, &str_end, 10);
            if ( 0 != *str_end )
            {
                if ( AT_COMMAND_WRITE_E == command_type )
               {
                  /* read the value */
                  pChannel = str_end;
                  pChannel++;
                  value = (uint32_t)strtol(pChannel, &str_end, 10);
                  
                  if ( 0 == *str_end )
                  {
                     msg_status = AT_NO_ERROR_E;
                  }
                  else 
                  {
                     msg_status = AT_INVALID_COMMAND_E;
                  }
               }               
            }
            else 
            {
               if ( AT_COMMAND_READ_E == command_type )
               {
                  msg_status = AT_NO_ERROR_E;
               }
               else 
               {
                  msg_status = AT_INVALID_COMMAND_E;
               }
            }
            
         }

         if (AT_NO_ERROR_E == msg_status ) 
         {
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               
               if ( AT_COMMAND_READ_E == command_type )
               {
                  pCommand->ReadWrite = 0;
               }
               else if ( AT_COMMAND_WRITE_E == command_type )
               {
                  pCommand->ReadWrite = 1;
               }
               else 
               {
                  msg_status = AT_INVALID_COMMAND_E;
               }
               
               if ( AT_NO_ERROR_E == msg_status )
               {
                  pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
                  
                  pCommand->CommandType = CIE_CMD_DEVICE_ENABLE_E;
                  pCommand->PortNumber = (uint8_t)uart_port;
                  pCommand->NodeID = unit_address;
                  pCommand->Parameter1 = channel_number;
                  pCommand->Parameter2 = 0;
                  pCommand->Value = value;
                  pCommand->Zone = zone;
                  
                  osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
                  if (osOK != osStat)
                  {
                     /* failed to write */
                     osPoolFree(AppPool, pCIEReq);
                     msg_status = AT_QUEUE_ERROR_E;
                  }
                  else
                  {
                     msg_status = AT_NO_ERROR_E;
                  }
               }
               else 
               {
                  osPoolFree(AppPool, pCIEReq);
               }
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ReadPluginSerialNumber
* description     : Read the serial number from an RBU plugin.
*                   ATSNP?ZxUx
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ReadPluginSerialNumber(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint16_t unit_address;
   uint8_t zone;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type || AT_COMMAND_WRITE_E == command_type)
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( SUCCESS_E == msg_status )
         {
            if ( 2 <= parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               unit_address = parameters.Parameter[1];
            }
            else 
            {
               zone = ZONE_GLOBAL;
               unit_address = gNodeAddress;
            }
         }

         if( AT_NO_ERROR_E == msg_status )
         {
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_READ_PLUGIN_SERIAL_NUMBER_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->Parameter1 = DEV_ALL_DEVICES;
               pCommand->Parameter2 = 0;
               pCommand->ReadWrite = 0;
               pCommand->NodeID = unit_address;
               pCommand->Value = 0;
               pCommand->Zone = zone;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_PluginSounderTone
* description     : Read or write the sounder tone selection on an RBU plugin.
*                   ATSTS?ZxUx,<channel>,<profile>
*                   ATSTS=ZxUx,<channel>,<profile>,<value>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_PluginSounderTone(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   uint16_t unit_address;
   uint16_t zone;
   uint32_t channel_number;
   uint32_t output_profile;
   uint32_t value;
   
   if ( source_buffer )
   {
      /* Get the target node */
      msg_status = MM_ATCommand_ExtractUnitAddressAndZone(source_buffer, &unit_address, &zone);
      
      if ( AT_NO_ERROR_E == msg_status )
      {

         /* read the channel number*/
         
         char* pChannel = strchr((char*)source_buffer, ',');
         if ( pChannel )
         {
            pChannel++;
            char *str_end;
            channel_number = (uint16_t)strtol(pChannel, &str_end, 10);
            if ( 0 != *str_end )
            {
               /* read the output profile */
               pChannel = str_end;
               pChannel++;
               output_profile = (uint32_t)strtol(pChannel, &str_end, 10);
               if ( AT_COMMAND_WRITE_E == command_type )
               {
                  if ( 0 != *str_end )
                  {
                     /* read the value */
                     pChannel = str_end;
                     pChannel++;
                     value = (uint32_t)strtol(pChannel, &str_end, 10);
                     if ( 0 == *str_end )
                     {
                        msg_status = AT_NO_ERROR_E;
                     }
                  }
                  else 
                  {
                     msg_status = AT_INVALID_COMMAND_E;
                  }
               }
               else /* command type = READ */
               {
                   if ( 0 == *str_end )
                  {
                     msg_status = AT_NO_ERROR_E;
                  }

               }
            }
            else 
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
              
         }
         else 
         {
            msg_status = AT_INVALID_COMMAND_E;
         }
      }

      if (AT_NO_ERROR_E == msg_status ) 
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
            if ( AT_COMMAND_READ_E == command_type )
            {
               pCommand->ReadWrite = 0;
            }
            else if ( AT_COMMAND_WRITE_E == command_type )
            {
               pCommand->ReadWrite = 1;
            }
            else 
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
            
            if ( AT_NO_ERROR_E == msg_status )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               
               pCommand->CommandType = CIE_CMD_SOUNDER_TONE_SELECTION_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = unit_address;
               pCommand->Zone = zone;
               pCommand->Parameter1 = channel_number;
               pCommand->Parameter2 = output_profile;
               pCommand->Value = value;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
            else 
            {
               osPoolFree(AppPool, pCIEReq);
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_PluginUpperThreshold
* description     : Read or write the upper threshold level on an RBU plugin.
*                   ATTHU?ZxUx,channel
*                   ATTHU=ZxUx,channel,day_value,night_value
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_PluginUpperThreshold(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   AT_Parameters_t parameters;
   uint16_t unit_address = gNodeAddress;;
   uint16_t zone = 1;
   uint32_t channel_number = 0;
   int32_t day_value = 0;
   int32_t night_value = 0;
   
   if ( source_buffer )
   {
      //extract the  parameters
      msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
      
      if ( AT_NO_ERROR_E == msg_status )
      {
         if ( AT_COMMAND_WRITE_E == command_type )
         {
            if ( 5 == parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               unit_address = parameters.Parameter[1];
               channel_number = parameters.Parameter[2];
               day_value = parameters.Parameter[3];
               night_value = parameters.Parameter[4];
            }
            else if ( 3 == parameters.NumberOfParameters )
            {
               channel_number = parameters.Parameter[0];
               day_value = parameters.Parameter[1];
               night_value = parameters.Parameter[2];
            }
            else 
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
         }
         else if ( AT_COMMAND_READ_E == command_type )
         {
            if ( 3 == parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               unit_address = parameters.Parameter[1];
               channel_number = parameters.Parameter[2];
            }
            else if ( 1 == parameters.NumberOfParameters )
            {
               channel_number = parameters.Parameter[0];
            }
            else
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
         }

         if ( AT_NO_ERROR_E == msg_status )
         {
            if ( (DETECTOR_THRESHOLD_MINIMUM <= day_value) &&
                 (DETECTOR_THRESHOLD_MAXIMUM >= day_value) &&
                 (DETECTOR_THRESHOLD_MINIMUM <= night_value) &&
                 (DETECTOR_THRESHOLD_MAXIMUM >= night_value) )
            {
               msg_status = AT_NO_ERROR_E;
            }
            else 
            {
               msg_status = AT_INVALID_PARAMETER_E;
            }
         }

//         /* read the channel number*/
//         
//         char* pChannel = strchr((char*)source_buffer, ',');
//         if ( pChannel )
//         {
//            pChannel++;
//            char *str_end;
//            channel_number = (uint16_t)strtol(pChannel, &str_end, 10);
//            if ( AT_COMMAND_WRITE_E == command_type )
//            {
//               if ( 0 != *str_end )
//               {
//                  /* read the DAY value */
//                  pChannel = str_end;
//                  pChannel++;
//                  day_value = (int32_t)strtol(pChannel, &str_end, 10);
//                  if ( 0 != *str_end )
//                  {
//                     /* read the NIGHT value */
//                     pChannel = str_end;
//                     pChannel++;
//                     night_value = (int32_t)strtol(pChannel, &str_end, 10);
//                     if ( 0 == *str_end )
//                     {
//                        if ( (DETECTOR_THRESHOLD_MINIMUM <= day_value) &&
//                             (DETECTOR_THRESHOLD_MAXIMUM >= day_value) &&
//                             (DETECTOR_THRESHOLD_MINIMUM <= night_value) &&
//                             (DETECTOR_THRESHOLD_MAXIMUM >= night_value) )
//                        {
//                           msg_status = AT_NO_ERROR_E;
//                        }
//                        else 
//                        {
//                           msg_status = AT_INVALID_PARAMETER_E;
//                        }
//                     }
//                  }
//                  else 
//                  {
//                     msg_status = AT_INVALID_COMMAND_E;
//                  }
//               }
//               else 
//               {
//                  msg_status = AT_INVALID_COMMAND_E;
//               }
//            }
//            else /* command type = READ */
//            {
//                if ( 0 == *str_end )
//               {
//                  msg_status = AT_NO_ERROR_E;
//               }
//            }              
//         }
//         else 
//         {
//            msg_status = AT_INVALID_COMMAND_E;
//         }
      }

      if (AT_NO_ERROR_E == msg_status ) 
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
            if ( AT_COMMAND_READ_E == command_type )
            {
               pCommand->ReadWrite = 0;
            }
            else if ( AT_COMMAND_WRITE_E == command_type )
            {
               pCommand->ReadWrite = 1;
            }
            else 
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
            
            if ( AT_NO_ERROR_E == msg_status )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               
               pCommand->CommandType = CIE_CMD_UPPER_THRESHOLD_SETTING_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = unit_address;
               pCommand->Parameter1 = channel_number;
               pCommand->Parameter2 = 0;
               pCommand->Value = (night_value << 8) + day_value;
               pCommand->Zone = zone;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
            else 
            {
               osPoolFree(AppPool, pCIEReq);
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_PluginLowerThreshold
* description     : Read or write the upper threshold level on an RBU plugin.
*                   ATTHL?ZxUx,channel
*                   ATTHL=ZxUx,channel,day_value,night_value
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_PluginLowerThreshold(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   AT_Parameters_t parameters;
   uint16_t unit_address = gNodeAddress;;
   uint16_t zone = 1;
   uint32_t channel_number = 0;
   int32_t day_value = 0;
   int32_t night_value = 0;
   
   if ( source_buffer )
   {
      //extract the  parameters
      msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
      
      if ( AT_NO_ERROR_E == msg_status )
      {
         if ( AT_COMMAND_WRITE_E == command_type )
         {
            if ( 5 == parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               unit_address = parameters.Parameter[1];
               channel_number = parameters.Parameter[2];
               day_value = parameters.Parameter[3];
               night_value = parameters.Parameter[4];
            }
            else if ( 3 == parameters.NumberOfParameters )
            {
               channel_number = parameters.Parameter[0];
               day_value = parameters.Parameter[1];
               night_value = parameters.Parameter[2];
            }
            else 
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
         }
         else if ( AT_COMMAND_READ_E == command_type )
         {
            if ( 3 == parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               unit_address = parameters.Parameter[1];
               channel_number = parameters.Parameter[2];
            }
            else if ( 1 == parameters.NumberOfParameters )
            {
               channel_number = parameters.Parameter[0];
            }
            else
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
         }

         if ( AT_NO_ERROR_E == msg_status )
         {
            if ( (DETECTOR_THRESHOLD_MINIMUM <= day_value) &&
                 (DETECTOR_THRESHOLD_MAXIMUM >= day_value) &&
                 (DETECTOR_THRESHOLD_MINIMUM <= night_value) &&
                 (DETECTOR_THRESHOLD_MAXIMUM >= night_value) )
            {
               msg_status = AT_NO_ERROR_E;
            }
            else 
            {
               msg_status = AT_INVALID_PARAMETER_E;
            }
         }

//         /* read the channel number*/
//         
//         char* pChannel = strchr((char*)source_buffer, ',');
//         if ( pChannel )
//         {
//            pChannel++;
//            char *str_end;
//            channel_number = (uint16_t)strtol(pChannel, &str_end, 10);
//            if ( AT_COMMAND_WRITE_E == command_type )
//            {
//               /* read the DAY value */
//               pChannel = str_end;
//               pChannel++;
//               day_value = (int32_t)strtol(pChannel, &str_end, 10);
//               if ( 0 != *str_end )
//               {
//                  /* read the NIGHT value */
//                  pChannel = str_end;
//                  pChannel++;
//                  night_value = (int32_t)strtol(pChannel, &str_end, 10);
//                  if ( 0 == *str_end )
//                  {
//                     if ( (DETECTOR_THRESHOLD_MINIMUM <= day_value) &&
//                          (DETECTOR_THRESHOLD_MAXIMUM >= day_value) &&
//                          (DETECTOR_THRESHOLD_MINIMUM <= night_value) &&
//                          (DETECTOR_THRESHOLD_MAXIMUM >= night_value) )
//                     {
//                        msg_status = AT_NO_ERROR_E;
//                     }
//                     else 
//                     {
//                        msg_status = AT_INVALID_PARAMETER_E;
//                     }
//                  }
//                  else 
//                  {
//                     msg_status = AT_INVALID_COMMAND_E;
//                  }
//               }
//               else 
//               {
//                  msg_status = AT_INVALID_COMMAND_E;
//               }
//            }
//            else /* command type = READ */
//            {
//                if ( 0 == *str_end )
//               {
//                  msg_status = AT_NO_ERROR_E;
//               }
//            }              
//         }
//         else 
//         {
//            msg_status = AT_INVALID_COMMAND_E;
//         }
      }

      if (AT_NO_ERROR_E == msg_status ) 
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
            
            if ( AT_COMMAND_READ_E == command_type )
            {
               pCommand->ReadWrite = 0;
            }
            else if ( AT_COMMAND_WRITE_E == command_type )
            {
               pCommand->ReadWrite = 1;
            }
            else 
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
            
            if ( AT_NO_ERROR_E == msg_status )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
                  
               pCommand->CommandType = CIE_CMD_LOWER_THRESHOLD_SETTING_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = unit_address;
               pCommand->Parameter1 = channel_number;
               pCommand->Parameter2 = 0;
               pCommand->Value = (night_value << 8) + day_value;
               pCommand->Zone = zone;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
            else 
            {
               osPoolFree(AppPool, pCIEReq);
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_PluginTestMode
* description     : Read or write the Test Mode on an RBU plugin.
*                   ATTMP?ZxUx,channel
*                   ATTMP=ZxUx, channel,value
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_PluginTestMode(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   uint16_t unit_address;
   uint16_t zone;
   uint32_t channel_number;
   uint32_t value = 0;
   
   if ( source_buffer )
   {
      /* Get the target node */
      msg_status = MM_ATCommand_ExtractUnitAddressAndZone(source_buffer, &unit_address, &zone);
      
      if ( AT_NO_ERROR_E == msg_status )
      {

         /* read the channel number*/
         
         char* pChannel = strchr((char*)source_buffer, ',');
         if ( pChannel )
         {
            pChannel++;
            char *str_end;
            channel_number = (uint16_t)strtol(pChannel, &str_end, 10);
            if ( AT_COMMAND_WRITE_E == command_type )
            {
               if ( 0 != *str_end )
               {
                  /* read the value */
                  pChannel = str_end;
                  pChannel++;
                  value = (uint32_t)strtol(pChannel, &str_end, 10);
                  if ( 0 == *str_end )
                  {
                     msg_status = AT_NO_ERROR_E;
                  }
               }
               else 
               {
                  msg_status = AT_INVALID_COMMAND_E;
               }
            }
            else /* command type = READ */
            {
                if ( 0 == *str_end )
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }              
         }
         else 
         {
            msg_status = AT_INVALID_COMMAND_E;
         }
      }

      if (AT_NO_ERROR_E == msg_status ) 
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
            if ( AT_COMMAND_READ_E == command_type )
            {
               pCommand->ReadWrite = 0;
            }
            else if ( AT_COMMAND_WRITE_E == command_type )
            {
               pCommand->ReadWrite = 1;
            }
            else 
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
            
            if ( AT_NO_ERROR_E == msg_status )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               
               pCommand->CommandType = CIE_CMD_PLUGIN_TEST_MODE_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = unit_address;
               pCommand->Parameter1 = channel_number;
               pCommand->Parameter2 = 0;
               pCommand->Value = value;
               pCommand->Zone = zone;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
            else 
            {
               osPoolFree(AppPool, pCIEReq);
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_PluginFirmwareVersion
* description     : Read the firmware version from an RBU plugin.
*                   ATFIP?ZxUx
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_PluginFirmwareVersion(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint16_t unit_address;
   uint16_t zone;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         /* Get the target node */
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( AT_NO_ERROR_E == msg_status )
         {
            // 2 params means there was a unit address
            if ( 2 == parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               unit_address = parameters.Parameter[1];
            }
            else
            {
               unit_address = gNodeAddress;
               zone = ZONE_GLOBAL;
            }
         }

         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
            CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
            pCommand->CommandType = CIE_CMD_PLUGIN_FIRMWARE_VERSION_E;
            pCommand->PortNumber = (uint8_t)uart_port;
            pCommand->NodeID = unit_address;
            pCommand->Parameter1 = 2; /*Head*/
            pCommand->Parameter2 = 0;
            pCommand->Value = 0;
            pCommand->Zone = zone;
            pCommand->ReadWrite = 0;
            
            osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
            if (osOK != osStat)
            {
               /* failed to write */
               osPoolFree(AppPool, pCIEReq);
               msg_status = AT_QUEUE_ERROR_E;
            }
            else
            {
               msg_status = AT_NO_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_PluginTypeAndClass
* description     : Read the Type and Class from an RBU plugin.
*                   ATPTYPE?ZxUx
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_PluginTypeAndClass(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         /* Get the target node */
         uint16_t unit_address;
         uint16_t zone;
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( 2 == parameters.NumberOfParameters )
         {
            zone = parameters.Parameter[0];
            unit_address = parameters.Parameter[1];
         }
         else 
         {
            zone = ZONE_GLOBAL;
            unit_address = gNodeAddress;
         }

         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
            CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
            pCommand->CommandType = CIE_CMD_READ_PLUGIN_TYPE_AND_CLASS_E;
            pCommand->PortNumber = (uint8_t)uart_port;
            pCommand->NodeID = unit_address;
            pCommand->Parameter1 = 0;
            pCommand->Parameter2 = 0;
            pCommand->Value = 0;
            pCommand->Zone = zone;
            pCommand->ReadWrite = 0;
            
            osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
            if (osOK != osStat)
            {
               /* failed to write */
               osPoolFree(AppPool, pCIEReq);
               msg_status = AT_QUEUE_ERROR_E;
            }
            else
            {
               msg_status = AT_NO_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}


/*************************************************************************************/
/**
* function name   : MM_ATCommand_RBUFirmwareVersion
* description     : Read the firmware version from an RBU.
*                   ATFIR?ZxUx
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_RBUFirmwareVersion(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         /* Get the target node */
         uint16_t unit_address;
         uint16_t zone;
         msg_status = MM_ATCommand_ExtractUnitAddressAndZone(source_buffer, &unit_address, &zone);

         if( AT_NO_ERROR_E == msg_status )
         {
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_READ_DEVICE_FIRMWARE_VERSION_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = unit_address;
               pCommand->Parameter1 = 0; /*main image*/
               pCommand->Parameter2 = 0;
               pCommand->ReadWrite = 0;
               pCommand->Value = 0;
               pCommand->Zone = zone;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_NeighbourInformation
* description     : Read the neighbour information for a specified node.
*                   ATDNI?ZxUx
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_NeighbourInformation(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         /* Get the target node */
         uint16_t unit_address;
         uint16_t zone;
         msg_status = MM_ATCommand_ExtractUnitAddressAndZone(source_buffer, &unit_address, &zone);

         if( AT_NO_ERROR_E == msg_status )
         {
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_READ_DEVICE_NEIGHBOUR_INFORMATION_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = unit_address;
               pCommand->Parameter1 = 0;
               pCommand->Parameter2 = 0;
               pCommand->Value = 0;
               pCommand->Zone = zone;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_GPIOPinState
* description     : Read/write the state of a specified GPIO pin.
*                   ATPIN=P<PORT><PIN>,<value>    e.g. ATPIN=PA9,1
*                   ATPIN?P<PORT>                      ATPIN?P9
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_GPIOPinState(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   osStatus status;
   Cmd_Reply_t *pCmdReply = NULL;
   GPIO_TypeDef* port;
   uint16_t pin;
   bool pin_valid = false;
   
   if ( source_buffer )
   {
      if ( 'P' == source_buffer[0] )
      {
         if ( 'A' <= source_buffer[1] && 'D' >= source_buffer[1] )
         {
            bool port_valid = true;
            switch ( source_buffer[1] )
            {
               case 'A':
                  port = GPIOA;
               break;
               case 'B':
                  port = GPIOB;
               break;
               case 'C':
                  port = GPIOC;
               break;
               case 'D':
                  port = GPIOD;
               break;
               default:
                  port_valid = false;
                  break;
            }
            
            pin = atoi((const char *)&source_buffer[2]);
            
            if ( port_valid && 16 > pin )
            {
               pin_valid = true;
            }
         }
      }
      
      
      if (AT_COMMAND_READ_E == command_type)
      {
         if ( pin_valid )
         {
            /* Read the pin */
            uint16_t gpio_pin = 0x01 << pin;
            GPIO_PinState pin_state = HAL_GPIO_ReadPin(port, gpio_pin);
            uint32_t value = (uint32_t)pin_state;
            
            /* Prepare the response */
            pCmdReply = CALLOC_ATHANDLE_POOL

            if (pCmdReply)
            {
               pCmdReply->port_id = uart_port;
               CO_HexString(value, (char *)pCmdReply->data_buffer, BASE_10);
               pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);

               /* Send the response */
               status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
            
               if (osOK != status)
               {
                  /* Failed to write, free the pre-allocated pool entry */
                  FREE_ATHANDLE_POOL(pCmdReply);

                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  /* All done successfully */
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
      else 
      {
         if ( pin_valid )
         {
            /* read the value to write*/
            uint32_t value = 0;
            char* pValue = strchr((char*)source_buffer, ',');
            if ( pValue )
            {
               pValue++;
               char *str_end;
               value = (uint16_t)strtol(pValue, &str_end, 10);
               if ( 0 == *str_end && 2 > value )
               {
                  /* Write to the pin */
                  uint16_t gpio_pin = 0x01 << pin;
                  GPIO_PinState pin_state = value == 1 ? GPIO_PIN_SET:GPIO_PIN_RESET;
                  HAL_GPIO_WritePin(port, gpio_pin, pin_state);
                  
                  /* Prepare the response */
                  pCmdReply = CALLOC_ATHANDLE_POOL

                  if (pCmdReply)
                  {
                     strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                     pCmdReply->port_id = uart_port;
                     pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);

                     /* Send the response */
                     status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
                  
                     if (osOK != status)
                     {
                        /* Failed to write, free the pre-allocated pool entry */
                        FREE_ATHANDLE_POOL(pCmdReply);

                        msg_status = AT_QUEUE_ERROR_E;
                     }
                     else
                     {
                        /* All done successfully */
                        msg_status = AT_NO_ERROR_E;
                     }
                  }
               }
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_SetDayNight
* description     : Set the system to use Day or Night thresholds.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_SetDayNight(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type || AT_COMMAND_WRITE_E == command_type)
      {
         msg_status = AT_NO_ERROR_E;
         
         uint16_t dayNight = 0;
         /* If this is a WRITE command, read the day/night value */
         if ( AT_COMMAND_WRITE_E == command_type )
         {
            char *str_end;
            dayNight = (uint16_t)strtol((char*)source_buffer, &str_end, 10);
            if ( 0 != *str_end )
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
         }
         /* Send the command to the application */
         if ( AT_NO_ERROR_E == msg_status )
         {
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_SET_DAY_NIGHT_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = ADDRESS_GLOBAL;
               pCommand->Parameter1 = 0; /*Head*/
               pCommand->Parameter2 = (uint8_t)ZONE_GLOBAL;
               pCommand->Value = dayNight;
               pCommand->Zone = ZONE_GLOBAL;
               if ( AT_COMMAND_READ_E == command_type )
               {
                  pCommand->ReadWrite = 0;
               }
               else 
               {
                  pCommand->ReadWrite = 1;
               }
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
   }
   return msg_status;
}


/*************************************************************************************/
/**
* function name   : MM_ATCommand_PluginLED
* description     : Set or read the state of the Plugin Head LED.
*                   ATLED?ZxUx
*                   ATLED=ZxUx,state
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_PluginLED(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   AT_Parameters_t parameters;

   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type || AT_COMMAND_WRITE_E == command_type)
      {
         msg_status = AT_NO_ERROR_E;
         uint16_t onOff = 0;
         uint16_t unit_address = gNodeAddress;
         uint16_t zone = 1;
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);

         if ( AT_NO_ERROR_E == msg_status )
         {
//            /* If this is a WRITE command, read the day/night value */
//            if ( AT_COMMAND_WRITE_E == command_type )
//            {
//               char* pOnOff = strchr((char*)source_buffer, ',');
//               if ( pOnOff )
//               {
//                  pOnOff++;
//                  char *str_end;
//                  onOff = (uint16_t)strtol((char*)pOnOff, &str_end, 10);
//                  if ( 0 != *str_end )
//                  {
//                     msg_status = AT_INVALID_COMMAND_E;
//                  }
//                  else if ( 1 < onOff ) /* only accept 0 or 1 (off or on) */
//                  {
//                     msg_status = AT_INVALID_PARAMETER_E;
//                  }
//               }
//            }
            
            if ( AT_COMMAND_WRITE_E == command_type )
            {
               if ( 3 == parameters.NumberOfParameters )
               {
                  zone = parameters.Parameter[0];
                  unit_address = parameters.Parameter[1];
                  onOff = parameters.Parameter[2];
               }
               else if ( 1 == parameters.NumberOfParameters )
               {
                  onOff = parameters.Parameter[0];
               }
               else 
               {
                  msg_status = AT_INVALID_COMMAND_E;
               }
            }
            else if ( AT_COMMAND_READ_E == command_type )
            {
               if ( 2 == parameters.NumberOfParameters )
               {
                  zone = parameters.Parameter[0];
                  unit_address = parameters.Parameter[1];
               }
               else if ( 0 != parameters.NumberOfParameters )
               {
                  msg_status = AT_INVALID_COMMAND_E;
               }
            }
         }
         /* Send the command to the application */
         if ( AT_NO_ERROR_E == msg_status )
         {
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_PLUGIN_LED_SETTING_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = unit_address;
               pCommand->Parameter1 = 0;
               pCommand->Parameter2 = 0;
               pCommand->Value = onOff;
               pCommand->Zone = zone;
               if ( AT_COMMAND_READ_E == command_type )
               {
                  pCommand->ReadWrite = 0;
               }
               else 
               {
                  pCommand->ReadWrite = 1;
               }
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATEncodePreformParents
* description     : Converts the human readable preformed parent IDs into a uint32_t.
*                   Used for encoding a single value for storing in NVM.
*                   The encoding puts the primary parent ID in the lower 16 bits of the result
*                   and the Secondary Parent ID in the Higher 16 bits.
*
* @param - pParamBuffer    String values of Primary and Secondary parent ( PP,SP )
* @param - pValue          [OUT] parameter for returning the encoded number.
*
* @return - bool           TRUE if the encoding was successful, FALSE otherwise.
*/
bool MM_ATEncodePreformParents(const char* pParamBuffer, uint32_t *pValue)
{
   bool encoding_success = false;
   uint32_t primaryID = 0;
   uint32_t secondaryID = 0xFFFF;
   char* str_end;

   if ( pParamBuffer && pValue )
   {
      const char* pSource = pParamBuffer;
      
      /* Read the primary parent ID */
      primaryID = (uint32_t)strtol((char*)pSource, &str_end, 10);
      primaryID  &= 0xFFFF;
      pSource = str_end;
      if ( !*pSource )
      {
         // There is no secondary ID
         encoding_success = true;
      }
      else 
      {
         /* find the first ',' */
         pSource = strchr((const char*)pParamBuffer, ',');
         if ( pSource )
         {
            pSource++;/* step over the comma */
            secondaryID = (uint32_t)strtol((char*)pSource, &str_end, 10);
            secondaryID &= 0xFFFF;
            pSource = str_end;

            if(!*pSource)
            {
               encoding_success = true;
            }
         }
      }
   }
   
   if ( encoding_success )
   {
      *pValue = (secondaryID << 16) | primaryID;
   }
   
   return encoding_success;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_TxRxLogic
* description     : Set or read the state of the Tx Rx Switch control logic.
*                   ATTXRX?
*                   ATTXRX=0|1   (1=default setting on start-up)
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t param_id  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_TxRxLogic(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   // Set up the parameters
   uint32_t TxRxValue = 0;
   osStatus status;
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      // Allocate Pool
      Cmd_Reply_t *pCmdReply = NULL;
      pCmdReply = CALLOC_ATHANDLE_POOL
      
      if (pCmdReply)
      {
         switch (command_type) 
         {
            case AT_COMMAND_READ_E:
            {
               /* This command doesn't expect any additional parameter in read operation*/
               if (0 == buffer_length)
               {
                  // Read the address
                  msg_status = AT_NO_ERROR_E;
                  CO_HexString(TxRxLogic, (char *)&pCmdReply->data_buffer, BASE_10);
               }
            }
            break;
          
            case AT_COMMAND_WRITE_E:
            {    
               msg_status = AT_INVALID_PARAMETER_E;
               if (0 < buffer_length)
               {
                  char *str_end;

                  /* Get the bank value */
                  TxRxValue = (uint16_t)strtol((char*)source_buffer, &str_end, 10);

                  if(!*str_end)
                  {
                     if ( 2 > TxRxValue )
                     {
                        if ( TxRxValue )
                        {
                           TxRxLogic = 1;
                           RxTxLogic = 0;
                        }
                        else
                        {
                           RxTxLogic = 1;
                           TxRxLogic = 0;
                        }
                        msg_status = AT_NO_ERROR_E;
                        strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                     }
                     else 
                     {
                        msg_status = AT_INVALID_PARAMETER_E;
                     }
                  }
               }
            }
            break;

            default:
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
         }
         
         //If the command failed, free the memory.
         if (AT_NO_ERROR_E != msg_status)
         {
            FREE_ATHANDLE_POOL(pCmdReply);
         }
      }
      
      if (AT_NO_ERROR_E == msg_status)
      {  
         pCmdReply->port_id = uart_port;
         pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
         status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK != status)
         {
            FREE_ATHANDLE_POOL(pCmdReply);
            msg_status = AT_QUEUE_ERROR_E;
         }
         msg_status = AT_NO_ERROR_E;
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_Profile
* description     : Set or read the NVM parameters that set the output profile behaviour.
*                   ATPRF=<profile>,<svi tone>,<flash rate>,<sitenet pattern>
*                   ATPRF?<profile>
*                   Read response = PRF: <profile>,<svi tone>,<flash rate>,<sitenet pattern>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t param_id  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_Profile(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   // Set up the parameters
   DM_NVMParamId_t profile = NV_MAX_PARAM_ID_E;
   uint32_t requested_profile = 0;
   uint32_t tone = 0;
   uint32_t flash_rate = 0;
   uint32_t nvm_value = 0;
   uint32_t sitenet_pattern = 0;
   AT_Parameters_t parameters;
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   osStatus status;
   
   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if (pCmdReply)
   {
      //Read the parameters from the AT comand
      msg_status = AT_INVALID_PARAMETER_E;
      if ( source_buffer )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( AT_NO_ERROR_E == msg_status )
         {
            // 1 param for read, 4 for write
            if ( (1 == parameters.NumberOfParameters) || (4 == parameters.NumberOfParameters) )
            {
               //The command is addressed to a device.  Forward it to the application.
               requested_profile = parameters.Parameter[0];
               
               if ( 4 == parameters.NumberOfParameters )
               {
                  tone = parameters.Parameter[1];
                  flash_rate = parameters.Parameter[2];
                  sitenet_pattern = parameters.Parameter[3];
               }

               switch ( requested_profile )
               {
                  case CO_PROFILE_FIRE_E:
                     profile = NV_PROFILE_FIRE_E;
                     break;
                  case CO_PROFILE_FIRST_AID_E:
                     profile = NV_PROFILE_FIRST_AID_E;
                     break;
                  case CO_PROFILE_EVACUATE_E:
                     profile = NV_PROFILE_EVACUATE_E;
                     break;
                  case CO_PROFILE_SECURITY_E:
                     profile = NV_PROFILE_SECURITY_E;
                     break;
                  case CO_PROFILE_GENERAL_E:
                     profile = NV_PROFILE_GENERAL_E;
                     break;
                  case CO_PROFILE_FAULT_E:
                     profile = NV_PROFILE_FAULT_E;
                     break;
                  case CO_PROFILE_ROUTING_ACK_E:
                     profile = NV_PROFILE_ROUTING_ACK_E;
                     break;
                  case CO_PROFILE_SILENT_TEST_E:
                     profile = NV_PROFILE_SILENT_TEST_E;
                     break;
                  case CO_PROFILE_TEST_E:
                     profile = NV_PROFILE_TEST_E;
                     break;
                  default:
                     profile = NV_MAX_PARAM_ID_E;//invalid value
                  break;
               }

               if ( ( (uint32_t)NV_MAX_PARAM_ID_E > profile ) &&
                    ( (uint32_t)DM_SVI_TONE_MAX_E > tone  ) &&
                    ( (uint32_t)FLASH_RATE_MAX_E > flash_rate ) &&
                    ( (uint32_t)DM_OP_SITENET_SOUNDER_PATTERN_MAX_E > sitenet_pattern) )
               {
                  msg_status = AT_NO_ERROR_E;
               }
               else 
               {
                  msg_status = AT_INVALID_PARAMETER_E;
               }
            }
            else 
            {
               msg_status = AT_INVALID_COMMAND_E;
            }



            if ( AT_NO_ERROR_E == msg_status )
            {
               switch (command_type) 
               {
                  case AT_COMMAND_READ_E:
                  {
                     /* Initialise to error */
                     msg_status = AT_INVALID_PARAMETER_E;

                     // Read the NVM
                     if (SUCCESS_E == DM_NVMRead(profile, (void*)&nvm_value, sizeof(nvm_value)))
                     {
                        if (pCmdReply)
                        {
                           sitenet_pattern = (nvm_value >> 24);
                           flash_rate = (nvm_value >> 16) & 0xFF;
                           tone = nvm_value & 0xFFFFU;
                           
                           /* Prepare the response */
                           sprintf((char *)pCmdReply->data_buffer,"%d,%d,%d,%d", requested_profile, tone, flash_rate, sitenet_pattern);
                           pCmdReply->port_id = uart_port;
                           pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);

                           /* Send the response */
                           status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
                     
                           if (osOK != status)
                           {
                              msg_status = AT_QUEUE_ERROR_E;
                           }
                           else
                           {
                              /* All done successfully */
                              msg_status = AT_NO_ERROR_E;
                           }
                        }
                     }
                  }
                  break;
                
                  case AT_COMMAND_WRITE_E:
                  {
                     /* Initialise to error */
                     msg_status = AT_INVALID_COMMAND_E;
                     //encode the S+V pattern, flash and tone into the 32 bit nvm parameter.  S+V in bits 24-31, Flash rate in bits 16-23, tone in bits 0-15.
                     nvm_value = (sitenet_pattern & 0xFF) << 24;
                     nvm_value |= (flash_rate & 0xFF) << 16;
                     nvm_value |= tone & 0xFFFF;
                     // write to NVM
                     if (SUCCESS_E == DM_NVMWrite(profile, (void*)&nvm_value, sizeof(nvm_value)))
                     {
                        //Update the dynamic settings
                        DM_OP_LoadProfiles();
                        
                        if (pCmdReply)
                        {
                           /* Prepare the response */
                           pCmdReply->port_id = uart_port;
                           strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                           pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);

                           /* Send the response */
                           status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
                     
                           if (osOK != status)
                           {
                              msg_status = AT_QUEUE_ERROR_E;
                           }
                           else
                           {
                              /* All done successfully */
                              msg_status = AT_NO_ERROR_E;
                           }
                        }
                     }
                  }
                  break;

                  default:
                  {
                     msg_status = AT_INVALID_COMMAND_E;
                  }
               }
            }
         }
      }
      
      if (AT_NO_ERROR_E == msg_status)
      {  
         pCmdReply->port_id = uart_port;
         pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
         status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK != status)
         {
            FREE_ATHANDLE_POOL(pCmdReply);
            msg_status = AT_QUEUE_ERROR_E;
         }
         msg_status = AT_NO_ERROR_E;
      }
      else
      {
         FREE_ATHANDLE_POOL(pCmdReply);
      }
   }
   
   
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_IOUThresholds
* description     : Set or read the NVM parameters that set the IOU input thresholds.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t param_id  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_IOUThresholds(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   // Set up the parameters
   uint32_t short_threshold = 0; //detect short cct input
   uint32_t logic_threshold = 0;//Set logic 0/1 boundary
   uint32_t open_threshold = 0;//detect open cct input
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   osStatus status;

   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if (pCmdReply)
   {
      //Read the parameters from the AT comand
      msg_status = AT_INVALID_PARAMETER_E;
      if ( AT_COMMAND_WRITE_E == command_type )
      {
         if ( source_buffer )
         {
            if (0 < buffer_length)
            {
               /* read the short threshold*/
               char* pBuffer = (char*)source_buffer;
               char *str_end;
               short_threshold = (uint32_t)strtol(pBuffer, &str_end, 10);
               if ( 0 != *str_end )
               {
                  /* read the logic threshold */
                  pBuffer = str_end;
                  pBuffer++;
                  logic_threshold = (int32_t)strtol(pBuffer, &str_end, 10);
                  if ( 0 != *str_end )
                  {
                     /* read the logic threshold */
                     pBuffer = str_end;
                     pBuffer++;
                     open_threshold = (int32_t)strtol(pBuffer, &str_end, 10);
                     if ( 0 == *str_end )
                     {
                        msg_status = AT_NO_ERROR_E;
                     }
                  }
               }
               else 
               {
                  msg_status = AT_INVALID_COMMAND_E;
               }
            }
         }
      }
      else /* command type = READ */
      {
         msg_status = AT_NO_ERROR_E;
      }



      if ( AT_NO_ERROR_E == msg_status )
      {
         /* Initialise to error */
         msg_status = AT_NVM_ERROR_E;
         
         switch (command_type) 
         {
            case AT_COMMAND_READ_E:
            {

               // Read the NVM
               if ( (SUCCESS_E == DM_NVMRead(NV_IOU_INPUT_SHORT_THRESHOLD_E, (void*)&short_threshold, sizeof(short_threshold))) &&
                    (SUCCESS_E == DM_NVMRead(NV_IOU_INPUT_LOGIC_THRESHOLD_E, (void*)&logic_threshold, sizeof(logic_threshold))) &&
                    (SUCCESS_E == DM_NVMRead(NV_IOU_INPUT_OPEN_THRESHOLD_E, (void*)&open_threshold, sizeof(open_threshold))) )
               {
                  /* Prepare the response */
                  sprintf((char *)pCmdReply->data_buffer,"%d,%d,%d", short_threshold, logic_threshold, open_threshold);
                  pCmdReply->port_id = uart_port;
                  pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
                  msg_status = AT_NO_ERROR_E;
               }
            }
            break;
          
            case AT_COMMAND_WRITE_E:
            {
               // write to NVM
               if ( (SUCCESS_E == DM_NVMWrite(NV_IOU_INPUT_SHORT_THRESHOLD_E, (void*)&short_threshold, sizeof(short_threshold))) &&
                    (SUCCESS_E == DM_NVMWrite(NV_IOU_INPUT_LOGIC_THRESHOLD_E, (void*)&logic_threshold, sizeof(logic_threshold))) &&
                    (SUCCESS_E == DM_NVMWrite(NV_IOU_INPUT_OPEN_THRESHOLD_E, (void*)&open_threshold, sizeof(open_threshold))) )
               {
                  /* Prepare the response */
                  pCmdReply->port_id = uart_port;
                  strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                  pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
                  msg_status = AT_NO_ERROR_E;
               }
            }
            break;

            default:
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
         }
      }
      
      if (AT_NO_ERROR_E == msg_status)
      {  
         pCmdReply->port_id = uart_port;
         pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
         status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK != status)
         {
            // failed to queue
            FREE_ATHANDLE_POOL(pCmdReply);
            msg_status = AT_QUEUE_ERROR_E;
         }
      }
      else 
      {
         //Comand failed.  Free memory
         FREE_ATHANDLE_POOL(pCmdReply);
      }
   }

   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_IOUPollPeriod
* description     : Get and Set the period at which the IOU inputs are polled.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_IOUPollPeriod(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      /* Use the common function MM_ATCommand_decimalNumber to perform the read/write to NVM */
      msg_status = MM_ATCommand_decimalNumber(command_type, source_buffer, buffer_length, uart_port, param_id);
   }
      
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_AlarmConfigurationState
* description     : Set the configuration an output channel.  This sets the events that
*                   the channels should respond to.
*                   The configuration is stored in the low 7 bits of the NVM word.
*                   ATACS=[ZzUu,]<channel>,<bitfield> or ATACS?[ZzUu,]<channel>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t param_id  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_AlarmConfigurationState(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   osStatus status;
   AT_Parameters_t parameters;
   bool read_write_to_local_nvm = false;
   uint32_t address = 0;
   uint32_t zone = 0;
   uint32_t channel = 0;
   uint32_t bitfield = 0;


   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if (pCmdReply)
   {
      //Read the parameters from the AT comand
      msg_status = AT_INVALID_PARAMETER_E;
      if ( source_buffer )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( AT_NO_ERROR_E == msg_status )
         {
            // 4 params mean there was a unit address
            if ( 4 == parameters.NumberOfParameters )
            {
               //The command is addressed to a device.  Forward it to the application.
               zone = parameters.Parameter[0];
               address = parameters.Parameter[1];
               channel = parameters.Parameter[2];
               bitfield = parameters.Parameter[3];
            }
            else if ( 3 == parameters.NumberOfParameters )
            {
               //The command is addressed to a device.  Forward it to the application.
               zone = parameters.Parameter[0];
               address = parameters.Parameter[1];
               channel = parameters.Parameter[2];
            }
            else if ( 2 == parameters.NumberOfParameters )
            {
               address = gNodeAddress;
               //Write command for this device.
               channel = parameters.Parameter[0];
               bitfield = parameters.Parameter[1];
               read_write_to_local_nvm = true;
            }
            else if ( 1 == parameters.NumberOfParameters )
            {
               address = gNodeAddress;
               //Read command for this device.
               channel = parameters.Parameter[0];
               read_write_to_local_nvm = true;
            }
            
               
            if ( (COMMAND_READ == command_type) && ((address == gNodeAddress) || (ADDRESS_GLOBAL == address)) )
            {
               read_write_to_local_nvm = true;
            }
            
            if ( gNodeAddress != address )
            {
               if ( AT_NO_ERROR_E == msg_status )
               {
                  CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
                  if ( pCIEReq )
                  {
                     pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
                     
                     CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
                     pCommand->CommandType = CIE_CMD_ALARM_CONFIG_E;
                     pCommand->PortNumber = (uint8_t)uart_port;
                     pCommand->NodeID = address;
                     pCommand->Parameter1 = channel;
                     pCommand->Parameter2 = (uint8_t)zone;
                     pCommand->Value = bitfield;
                     pCommand->Zone = zone;
                     if( AT_COMMAND_WRITE_E == command_type )
                     {
                        pCommand->ReadWrite = 1;
                     }
                     else 
                     {
                        pCommand->ReadWrite = 0;
                     }
                     
                     osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
                     if (osOK != osStat)
                     {
                        /* failed to write */
                        osPoolFree(AppPool, pCIEReq);
                        msg_status = AT_QUEUE_ERROR_E;
                     }
                     else
                     {
                        /* Prepare the usart OK response */
                        strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                        pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
                        /* All done successfully */
                        msg_status = AT_NO_ERROR_E;
                     }
                  }
               }
            }


            if ( read_write_to_local_nvm )
            {
               //check that the channel has an associated NVM slot
               DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(channel); 
               
               if ( NV_MAX_PARAM_ID_E != nvm_param )
               {
                  // Read the NVM
                  uint32_t nvm_value;
                  if ( SUCCESS_E == DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
                  {
                     switch (command_type) 
                     {
                        case AT_COMMAND_READ_E:
                        {
                           /* Initialise to error */
                           msg_status = AT_NVM_ERROR_E;

                           //Mask out all but the output profile bits
                           nvm_value &= NVM_OUTPUT_PROFILE_MASK;
                           
                           /* Prepare the response */
                           sprintf((char *)pCmdReply->data_buffer,"%d,%d", channel, nvm_value);
                           pCmdReply->port_id = uart_port;
                           pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);

                           msg_status = AT_NO_ERROR_E;
                        }
                        break;
                      
                        case AT_COMMAND_WRITE_E:
                        {
                           /* Initialise to error */
                           msg_status = AT_NVM_ERROR_E;
                           
                           //Mask out the output profile bits from the value read from NVM above
                           nvm_value &= ~NVM_OUTPUT_PROFILE_MASK;
                           
                           // check that the supplied profile doesn't extend beyond the bit field for the profile
                           if ( 0 == ( bitfield & ~NVM_OUTPUT_PROFILE_MASK ) )
                           {
                              //  Apply the bit field and write back to NVM
                              nvm_value |= bitfield;
                              if ( SUCCESS_E == DM_NVMWrite(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
                              {
                                 /* Prepare the response */
                                 strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                                 pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
                                 /* All done successfully */
                                 msg_status = AT_NO_ERROR_E;
                              }
                           }
                        }
                        break;

                        default:
                        {
                           msg_status = AT_INVALID_COMMAND_E;
                        }
                     }
                  }
               }
            }
         }
      }

      
      //Reply to the usart
      if (AT_NO_ERROR_E == msg_status)
      {  
         pCmdReply->port_id = uart_port;
         pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
         status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK != status)
         {
            // failed to queue
            FREE_ATHANDLE_POOL(pCmdReply);
            msg_status = AT_QUEUE_ERROR_E;
         }
      }
      else
      {  
         /*Command failed.*/
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
         
         status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK != status)
         {
            // failed to queue
            FREE_ATHANDLE_POOL(pCmdReply);
            msg_status = AT_QUEUE_ERROR_E;
         }
      }
   }
   
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_AlarmOptionFlags
* description     : Set the option flags for an output channel.  This sets the
*                   'inverted,' 'ignore delays at night,' and 'silenceable' flags.
*                   The configuration is stored in bits 7,8,9 of the NVM word.
*                   ATAOF=[ZxUx,]<channel>,<bitfield> or ATAOF?[ZxUx,]<channel>,<bitfield>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t param_id  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_AlarmOptionFlags(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   // Set up the parameters
   uint32_t channel;
   uint32_t flags;
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   osStatus status;
   AT_Parameters_t parameters;
   bool read_write_to_local_nvm = true;

   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if (pCmdReply)
   {
      //Read the parameters from the AT comand
      msg_status = AT_INVALID_PARAMETER_E;
      if ( source_buffer )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( AT_NO_ERROR_E == msg_status )
         {
            // 4 params mean there was a unit address
            if ( 4 == parameters.NumberOfParameters )
            {
               //The command is addressed to a device.  Forward it to the application.
               uint32_t zone = parameters.Parameter[0];
               uint32_t address = parameters.Parameter[1];
               channel = parameters.Parameter[2];
               flags = parameters.Parameter[3];
               
               if ( (COMMAND_READ == command_type) && ((address == gNodeAddress) || (ADDRESS_GLOBAL == address)) )
               {
                  read_write_to_local_nvm = true;
               }
               
               if ( gNodeAddress != address )
               {
                  if ( AT_NO_ERROR_E == msg_status )
                  {
                     CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
                     if ( pCIEReq )
                     {
                        pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
                        
                        CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
                        pCommand->CommandType = CIE_CMD_ALARM_OPTION_FLAGS_E;
                        pCommand->PortNumber = (uint8_t)uart_port;
                        pCommand->NodeID = address;
                        pCommand->Parameter1 = parameters.Parameter[2];//channel
                        pCommand->Parameter2 = (uint8_t)zone;
                        pCommand->Value = parameters.Parameter[3];//bitfield
                        pCommand->Zone = zone;
                        if( AT_COMMAND_READ_E == command_type )
                        {
                           pCommand->ReadWrite = 0;
                        }
                        else 
                        {
                           pCommand->ReadWrite = 1;
                        }
                        
                        osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
                        if (osOK != osStat)
                        {
                           /* failed to write */
                           osPoolFree(AppPool, pCIEReq);
                           msg_status = AT_QUEUE_ERROR_E;
                        }
                        else
                        {
                           msg_status = AT_NO_ERROR_E;
                        }
                     }
                  }
               }
            }
            else 
            {
               channel = parameters.Parameter[0];
               flags = parameters.Parameter[1];
               read_write_to_local_nvm = true;
            }
         }



         if ( read_write_to_local_nvm )
         {
            //check that the channel has an associated NVM slot
            DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(channel); 
            
            if ( NV_MAX_PARAM_ID_E != nvm_param )
            {
               // Read the NVM
               uint32_t nvm_value;
               if ( SUCCESS_E == DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
               {
                  switch (command_type) 
                  {
                     case AT_COMMAND_READ_E:
                     {
                        /* Initialise to error */
                        msg_status = AT_NVM_ERROR_E;

                        //Mask out all but the options bits
                        nvm_value &= NVM_OUTPUT_OPTIONS_MASK;
                        
                        // bit shift to the lsb position
                        nvm_value >>= 0x07;
                        
                        /* Prepare the response */
                        sprintf((char *)pCmdReply->data_buffer,"%d,%d", channel, nvm_value);
                        pCmdReply->port_id = uart_port;
                        pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);

                        msg_status = AT_NO_ERROR_E;
                     }
                     break;
                   
                     case AT_COMMAND_WRITE_E:
                     {
                        /* Initialise to error */
                        msg_status = AT_NVM_ERROR_E;
                        
                        //Mask out the output profile bits from the value read from NVM above
                        nvm_value &= ~NVM_OUTPUT_OPTIONS_MASK;
                        
                        // check that the supplied flags don't extend beyond the bit field for the options
                        if ( 0 == ( flags & ~NVM_OUTPUT_OPTIONS_MASK ) )
                        {
                           //  Apply the bit field and write back to NVM
                           nvm_value |= flags;
                           if ( SUCCESS_E == DM_NVMWrite(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
                           {
                              /* Prepare the response */
                              strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                              pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
                              /* All done successfully */
                              msg_status = AT_NO_ERROR_E;
                           }
                        }
                     }
                     break;

                     default:
                     {
                        msg_status = AT_INVALID_COMMAND_E;
                     }
                  }
               }
            }
         }
      }
      
      //Reply to the usart
      if (AT_NO_ERROR_E == msg_status)
      {  
         pCmdReply->port_id = uart_port;
         pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
         status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK != status)
         {
            // failed to queue
            FREE_ATHANDLE_POOL(pCmdReply);
            msg_status = AT_QUEUE_ERROR_E;
         }
      }
      else 
      {
         //Command failed.  Free memory
         FREE_ATHANDLE_POOL(pCmdReply);
      }
   }
   
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_AlarmDelays
* description     : Set the delays for an output channel.  This sets the
*                   'Delay1' and 'Delay2' parameters for outputs.
*                   Delay1 is stored in bits 10-19 of the NVM word.
*                   ATADC=[ZxUx,]<channel>,<Delay1>,<Delay2> or ATADC?<channel>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t param_id  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_AlarmDelays(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   // Set up the parameters
   uint32_t zone;
   uint32_t address;
   uint32_t channel;
   uint32_t delay1;
   uint32_t delay2;
   uint32_t combined_delays;
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   osStatus status;
   AT_Parameters_t parameters;
   bool read_write_to_local_nvm = false;

   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if (pCmdReply)
   {
      //Read the parameters from the AT comand
      msg_status = AT_INVALID_PARAMETER_E;
      if ( source_buffer )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( AT_NO_ERROR_E == msg_status )
         {
            // 5 params mean there was a unit address
            if ( 5 == parameters.NumberOfParameters )
            {
               //The command is addressed to a device.  Forward it to the application.
               zone = parameters.Parameter[0];
               address = parameters.Parameter[1];
               channel = parameters.Parameter[2];
               delay1 = parameters.Parameter[3];
               delay2 = parameters.Parameter[4];
               combined_delays = (delay2 << 9) | delay1;
            }
            else if ( 3 == parameters.NumberOfParameters )
            {
               if ( AT_COMMAND_READ_E == command_type )
               {
                  zone = parameters.Parameter[0];
                  address = parameters.Parameter[1];
                  channel = parameters.Parameter[2];
               }
               else 
               {
                  zone = ZONE_GLOBAL;
                  address = gNodeAddress;
                  channel = parameters.Parameter[0];
                  delay1 = parameters.Parameter[1];
                  delay2 = parameters.Parameter[2];
                  combined_delays = (delay2 << 9) | delay1;
               }
            }
            else if ( 1 == parameters.NumberOfParameters )
            {
               zone = ZONE_GLOBAL;
               address = gNodeAddress;
               channel = parameters.Parameter[0];
            }
            
            if ( (MAX_OUTPUT_DELAY < delay1) ||  (MAX_OUTPUT_DELAY < delay2) )
            {
               msg_status = AT_INVALID_PARAMETER_E;
            }
            
            if ( AT_NO_ERROR_E == msg_status )
            {
               if ( (address == gNodeAddress) || (ADDRESS_GLOBAL == address) )
               {
                  read_write_to_local_nvm = true;
               }
               
               if ( gNodeAddress != address )
               {
                  if ( AT_NO_ERROR_E == msg_status )
                  {
                     CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
                     if ( pCIEReq )
                     {
                        pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
                        
                        CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
                        pCommand->CommandType = CIE_CMD_ALARM_DELAYS_E;
                        pCommand->PortNumber = (uint8_t)uart_port;
                        pCommand->NodeID = address;
                        pCommand->Parameter1 = channel;
                        pCommand->Parameter2 = (uint8_t)zone;
                        pCommand->Value = combined_delays;
                        pCommand->Zone = zone;
                        if( AT_COMMAND_WRITE_E == command_type )
                        {
                           pCommand->ReadWrite = 1;
                        }
                        else 
                        {
                           pCommand->ReadWrite = 0;
                        }
                        
                        osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
                        if (osOK != osStat)
                        {
                           /* failed to write */
                           osPoolFree(AppPool, pCIEReq);
                           msg_status = AT_QUEUE_ERROR_E;
                        }
                        else
                        {
                           /* Prepare the response */
                           strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                           pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
                           msg_status = AT_NO_ERROR_E;
                        }
                     }
                  }
               }
            }
         }



         if ( read_write_to_local_nvm )
         {
            //check that the channel has an associated NVM slot
            DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(channel); 
            
            if ( NV_MAX_PARAM_ID_E != nvm_param )
            {
               // Read the NVM
               uint32_t nvm_value;
               if ( SUCCESS_E == DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
               {
                  switch (command_type) 
                  {
                     case AT_COMMAND_READ_E:
                     {
                        /* Initialise to error */
                        msg_status = AT_NVM_ERROR_E;

                        //Mask out all but the options bits
                        nvm_value &= NVM_OUTPUT_DELAYS_MASK;
                        
                        //extract delay1 from the NVM value
                        delay1 = (nvm_value & NVM_OUTPUT_DELAY1_MASK) >> 11;
                        //extract delay2 from the NVM value
                        delay2 = (nvm_value & NVM_OUTPUT_DELAY2_MASK) >> 20;
                        
                        /* Prepare the response */
                        sprintf((char *)pCmdReply->data_buffer,"%d,%d,%d", channel, delay1, delay2);
                        pCmdReply->port_id = uart_port;
                        pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);

                        msg_status = AT_NO_ERROR_E;
                     }
                     break;
                   
                     case AT_COMMAND_WRITE_E:
                     {
                        /* Initialise to error */
                        msg_status = AT_NVM_ERROR_E;
                        
                        //Mask out the delay bits from the value read from NVM above
                        nvm_value &= ~NVM_OUTPUT_DELAYS_MASK;
                        
                        //Shift the supplied delays to the right bits
                        combined_delays <<= 11;
                        
                        // check that the supplied delays don't extend beyond the bit field for the options
                        if ( 0 == ( combined_delays & ~NVM_OUTPUT_DELAYS_MASK ) )
                        {
                           //  Apply the combined delay bit fields and write back to NVM
                           nvm_value |= combined_delays;
                           if ( SUCCESS_E == DM_NVMWrite(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
                           {
                              /* Prepare the response */
                              strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                              pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
                              /* All done successfully */
                              msg_status = AT_NO_ERROR_E;
                           }
                        }
                     }
                     break;

                     default:
                     {
                        msg_status = AT_INVALID_COMMAND_E;
                     }
                  }
               }
            }
         }
      }
      
      //Reply to the usart
      if (AT_NO_ERROR_E == msg_status)
      {  
         pCmdReply->port_id = uart_port;
         pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
         status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK != status)
         {
            // failed to queue
            FREE_ATHANDLE_POOL(pCmdReply);
            msg_status = AT_QUEUE_ERROR_E;
         }
      }
      else 
      {
         //Command failed.  Free memory
         FREE_ATHANDLE_POOL(pCmdReply);
      }
   }
   
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ParseAlarmOutputState
* description     : Parse the payload of an alarm output state command.
*
* @param - ATCommandType_t       command_type (Read/Write/Special)
* @param - source_buffer         The received raw message.
* @param - pAlarmOutputState     [OUT] structure to receive the parsed values.
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ParseAlarmOutputState(const ATCommandType_t command_type, const uint8_t *const source_buffer, CO_AlarmOutputStateData_t* pAlarmOutputState)
{
   uint32_t silenceable;
   uint32_t unsilenceable;
   uint32_t delayMask;
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( source_buffer )
   {
      if ( pAlarmOutputState )
      {
         // read the silenceable bit field 
         char* pBuffer = (char*)source_buffer;
         char *str_end;
         silenceable = (uint32_t)strtol(pBuffer, &str_end, 10);
         
         if ( 0 != *str_end )
         {
            // read the unsilenceable bit field 
            pBuffer = str_end;
            pBuffer++;
            unsilenceable = (int32_t)strtol(pBuffer, &str_end, 10);
            if ( 0 == *str_end )
            {
               pAlarmOutputState->Silenceable = silenceable;
               pAlarmOutputState->Unsilenceable = unsilenceable;
               pAlarmOutputState->DelayMask = 0;
               msg_status = AT_NO_ERROR_E;
            }
            else 
            {
               //Read the delay mask
               pBuffer = str_end;
               pBuffer++;
               delayMask = (int32_t)strtol(pBuffer, &str_end, 10);
               if ( 0 == *str_end )
               {
                  pAlarmOutputState->Silenceable = silenceable;
                  pAlarmOutputState->Unsilenceable = unsilenceable;
                  pAlarmOutputState->DelayMask = delayMask;
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_AlarmGlobalDelay
* description     : Command to switch on/off global delays on alarms.
*                   Note: This is the global command for general enable/disable
*                   of output delays, not an instruction to use the global delay values.
*                   ATAGD+0|1
*
* @param - ATCommandType_t       command_type (Read/Write/Special)
* @param - source_buffer         The received raw message.
* @param - pAlarmOutputState     [OUT] structure to receive the parsed values.
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_AlarmGlobalDelay(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_SPECIAL_E == command_type)
      {
         msg_status = AT_NO_ERROR_E;
         
         uint16_t delayOnOff = 0;
         /* Extract the on/off value */
         char *str_end;
         delayOnOff = (uint16_t)strtol((char*)source_buffer, &str_end, 10);
         if ( 0 != *str_end )
         {
            msg_status = AT_INVALID_COMMAND_E;
         }
         
         /* Send the command to the application */
         if ( AT_NO_ERROR_E == msg_status )
         {
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_GLOBAL_DELAYS_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = ADDRESS_GLOBAL;
               pCommand->Parameter1 = 0;
               pCommand->Parameter2 = (uint8_t)ZONE_GLOBAL;
               pCommand->Value = delayOnOff;
               pCommand->Zone = ZONE_GLOBAL;
               pCommand->ReadWrite = 1;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
   }
   return msg_status;
}


/*************************************************************************************/
/**
* function name   : MM_ATCommand_AlarmAcknowledge
* description     : Acknowledge message for alarms.
*                   ATACKE+     acknowledge first aid
*                   ATACKF+     acknowledge fire
*
* @param - ATCommandType_t       command_type (Read/Write/Special)
* @param - source_buffer         The received raw message.
* @param - pAlarmOutputState     [OUT] structure to receive the parsed values.
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_AlarmAcknowledge(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( AT_COMMAND_SPECIAL_E == command_type )
   {
      msg_status = AT_NO_ERROR_E;
      
      /* Send the command to the application */
      CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
      if ( pCIEReq )
      {
         pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
         
         CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
         pCommand->CommandType = CIE_CMD_ACKNOWLEDGE_E;
         pCommand->PortNumber = (uint8_t)uart_port;
         pCommand->NodeID = ADDRESS_GLOBAL;
         pCommand->Parameter1 = 0;
         pCommand->Parameter2 = (uint8_t)ZONE_GLOBAL;
         pCommand->Value = param_id;
         pCommand->Zone = ZONE_GLOBAL;
         pCommand->ReadWrite = 1;
         
         osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCIEReq);
            msg_status = AT_QUEUE_ERROR_E;
         }
         else
         {
            msg_status = AT_NO_ERROR_E;
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_AlarmEvacuate
* description     : Alarm evacuate message.
*                   ATEVAC+
*
* @param - ATCommandType_t       command_type (Read/Write/Special)
* @param - source_buffer         The received raw message.
* @param - pAlarmOutputState     [OUT] structure to receive the parsed values.
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_AlarmEvacuate(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( AT_COMMAND_SPECIAL_E == command_type)
   {
      msg_status = AT_NO_ERROR_E;
      
      /* Send the command to the application */
      if ( AT_NO_ERROR_E == msg_status )
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
            CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
            pCommand->CommandType = CIE_CMD_EVACUATE_E;
            pCommand->PortNumber = (uint8_t)uart_port;
            pCommand->NodeID = ADDRESS_GLOBAL;
            pCommand->Parameter1 = 0;
            pCommand->Parameter2 = (uint8_t)ZONE_GLOBAL;
            pCommand->Value = 0;
            pCommand->Zone = ZONE_GLOBAL;
            pCommand->ReadWrite = 1;
            
            osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
            if (osOK != osStat)
            {
               /* failed to write */
               osPoolFree(AppPool, pCIEReq);
               msg_status = AT_QUEUE_ERROR_E;
            }
            else
            {
               msg_status = AT_NO_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}


/*************************************************************************************/
/**
* function name   : MM_ATCommand_EnableAlarms
* description     : Enable/disable outputs for a channel, device or zone, depending on the
*                   parameter option_param.  enable=1, disable=0.
*                   ATDISC+[ZxUx,]<channel>,<enable_day>,<enable_night>
*                   ATDISD+[ZxUx,]<enable>
*                   ATDISZ+<zone>,<enable>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_EnableAlarms(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   AT_Parameters_t parameters;
   uint32_t device = 0;
   uint32_t zone = 0;
   uint32_t channel = 0;
   uint32_t enable = 0;
   uint32_t enable_night = 0;
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_SPECIAL_E == command_type)
      {
         msg_status = AT_NO_ERROR_E;
         
         switch ( param_id )
         {
            case CO_DISABLE_NONE_E:
               //No params required
               break;
            case CO_DISABLE_CHANNEL_E:
               //Read 'device' (ZxUx) and 'channel' and 'day enable/disable' and 'night enable/disable' from source_buffer
               msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
               if ( AT_NO_ERROR_E == msg_status )
               {
                  if ( 5 == parameters.NumberOfParameters )
                  {
                     zone = parameters.Parameter[0];
                     device = parameters.Parameter[1];
                     channel = parameters.Parameter[2];
                     enable = parameters.Parameter[3];
                     enable_night = parameters.Parameter[4];
                  }
                  else if ( 3 == parameters.NumberOfParameters )
                  {
                     zone = ZONE_GLOBAL;
                     device = gNodeAddress;
                     channel = parameters.Parameter[0];
                     enable = parameters.Parameter[1];
                     enable_night = parameters.Parameter[2];
                  }
               }
               break;
            case CO_DISABLE_DEVICE_E:
               //Read 'device' and 'enable/disable' from source_buffer
               msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
               if ( AT_NO_ERROR_E == msg_status )
               {
                  if ( 3 == parameters.NumberOfParameters )
                  {
                     zone = parameters.Parameter[0];
                     device = parameters.Parameter[1];
                     enable = parameters.Parameter[2];
               }
                  else if ( 1 == parameters.NumberOfParameters )
                  {
                     zone = ZONE_GLOBAL;
                     device = gNodeAddress;
                     enable = parameters.Parameter[0];
                  }
               }
               break;
            case CO_DISABLE_ZONE_E:
               //Read 'zone' and 'enable/disable' from source_buffer
               msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
               if ( (AT_NO_ERROR_E == msg_status) && (2 == parameters.NumberOfParameters) )
               {
                  device = ADDRESS_GLOBAL;
                  zone = parameters.Parameter[0];
                  enable = parameters.Parameter[1];
               }
               break;
            case CO_DISABLE_ALL_E:
               //No params required
               break;
            default:
               msg_status = AT_INVALID_PARAMETER_E;
            break;
         }
         
         /* Send the command to the application */
         if ( AT_NO_ERROR_E == msg_status )
         {
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_ENABLE_DISABLE_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = device;
               pCommand->Parameter1 = param_id;
               pCommand->Parameter2 = channel;
               pCommand->Value = (enable | (enable_night << 1));// day enable=0x01, night=0x02, both=0x03
               pCommand->Zone = zone;
               pCommand->ReadWrite = 1;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
   }
   return msg_status;
}


/*************************************************************************************/
/**
* function name   : MM_ATCommand_AlarmReset
* description     : Send a reset message for all alarms on the mesh.
*                   ATRST+
*
* @param - ATCommandType_t       command_type (Read/Write/Special)
* @param - source_buffer         The received raw message.
* @param - pAlarmOutputState     [OUT] structure to receive the parsed values.
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_AlarmReset(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
	AT_Parameters_t parameters;

   if ( AT_COMMAND_SPECIAL_E == command_type )
   {
      msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
               
      /* Send the command to the application */
      if ( AT_NO_ERROR_E == msg_status )
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
				CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
            
				if ( 0 == parameters.NumberOfParameters )
				{
					pCommand->Zone = ZONE_GLOBAL;
					pCommand->NodeID = ADDRESS_GLOBAL;
					pCommand->Parameter1 = PARAM_RESET_FAULTS | PARAM_RESET_EVENTS;
				}
				else if (  1 == parameters.NumberOfParameters )
				{
					pCommand->Zone = ZONE_GLOBAL;						//	ignore specified zone number
					pCommand->NodeID = ADDRESS_GLOBAL;
					pCommand->Parameter1 = parameters.Parameter[0];
				}	
				else if (  2 == parameters.NumberOfParameters )
				{
					pCommand->Zone = ZONE_GLOBAL;						//	ignore specified zone number
					pCommand->NodeID = parameters.Parameter[1];
					pCommand->Parameter1 = PARAM_RESET_FAULTS | PARAM_RESET_EVENTS;
				}	
            else if (  3 == parameters.NumberOfParameters )
				{
					pCommand->Zone = ZONE_GLOBAL;						//	ignore specified zone number
					pCommand->NodeID = parameters.Parameter[1];
					pCommand->Parameter1 = parameters.Parameter[2];
				}	
				
				if ( pCommand->NodeID == ADDRESS_GLOBAL ) pCommand->Parameter1 |= PARAM_RESET_DULCH;
				
            pCommand->CommandType = CIE_CMD_RESET_ALARMS_E;
            pCommand->PortNumber = (uint8_t)uart_port;
            pCommand->Parameter2 = (uint8_t)ZONE_GLOBAL;
            pCommand->Value = 0;
            pCommand->ReadWrite = 1;
            
            osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
            if (osOK != osStat)
            {
               /* failed to write */
               osPoolFree(AppPool, pCIEReq);
               msg_status = AT_QUEUE_ERROR_E;
            }
            else
            {
               msg_status = AT_NO_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_Ping
* description     : Send a ping to a neigbouring node.
*
* @param - ATCommandType_t       command_type (Read/Write/Special)
* @param - source_buffer         The received raw message.
* @param - pAlarmOutputState     [OUT] structure to receive the parsed values.
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_Ping(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   AT_Parameters_t parameters;
   osStatus status;
   Cmd_Reply_t *pCmdReply = NULL;
   uint32_t nodeID;
   uint32_t payload = 0;
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_WRITE_E == command_type )
      {
         /* read the target node ID*/
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( AT_NO_ERROR_E == msg_status )
         {
            if ( (0 < parameters.NumberOfParameters) && (2 >= parameters.NumberOfParameters) )
            {
               nodeID = parameters.Parameter[0];
               if ( 2 == parameters.NumberOfParameters )
               {
                  payload = parameters.Parameter[1];
               }
               msg_status = AT_NO_ERROR_E;
            }
         }

         if( AT_NO_ERROR_E == msg_status )
         {
            osStatus osStat = osErrorOS;
            CO_Message_t *pPingReq;
          
            /* create cmd message and put into mesh queue */
            pPingReq = ALLOCMESHPOOL;
            if (pPingReq)
            {
               PingRequest_t pingRequest;
               pingRequest.Header.FrameType = FRAME_TYPE_DATA_E;
               pingRequest.Header.MACDestinationAddress = nodeID;
               pingRequest.Header.MACSourceAddress = gNodeAddress;
               pingRequest.Header.HopCount = 0;
               pingRequest.Header.DestinationAddress = nodeID;
               pingRequest.Header.SourceAddress = gNodeAddress;
               pingRequest.Header.MessageType = APP_MSG_TYPE_PING_E;
               pingRequest.Payload = payload;
               pingRequest.SystemId = gSystemID;
               pingRequest.SendBehaviour = SEND_ONCE_E;

               pPingReq->Type = CO_MESSAGE_GENERATE_PING_REQUEST_E;
               memcpy(pPingReq->Payload.PhyDataReq.Data, &pingRequest, sizeof(PingRequest_t));
               osStat = osMessagePut(AppQ, (uint32_t)pPingReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  FREEMESHPOOL(pPingReq);
               }
            }
         }
      }
      
      /* Write operation succeeded, allocate pool for response */
      pCmdReply = CALLOC_ATHANDLE_POOL

      if (pCmdReply)
      {
         pCmdReply->port_id = uart_port;
         
         if ( AT_NO_ERROR_E == msg_status )
         {
            strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
            pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
         }
         else 
         {
            strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
            pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
         }
         
         /* Send the response */
         status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK != status)
         {
            /* Failed to write, free the pre-allocated pool entry */
            FREE_ATHANDLE_POOL(pCmdReply);
            msg_status = AT_QUEUE_ERROR_E;
         }
         else
         {
            /* All done successfully */
            msg_status = AT_NO_ERROR_E;
         }
      }
      else
      {
         msg_status = AT_QUEUE_ERROR_E;
      }
   }

   return msg_status;
}
#ifdef ENABLE_TDM_CALIBRATION
/*************************************************************************************/
/**
* function name   : MM_ATCommand_RachTxRxOffset
* description     : Shift the timing between RACH Tx and Rx.
*
* @param - ATCommandType_t       command_type (Read/Write/Special)
* @param - source_buffer         The received raw message.
* @param - pAlarmOutputState     [OUT] structure to receive the parsed values.
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_RachTxRxOffset(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   osStatus status;
   Cmd_Reply_t *pCmdReply = NULL;

   if ( AT_COMMAND_WRITE_E == command_type )
   {
      /* read the target node ID*/
      int16_t offset = 0;
      char* pBuffer = (char*)source_buffer;
      if ( pBuffer )
      {
         char *str_end;
         offset = (uint16_t)strtol(pBuffer, &str_end, 10);
         if ( 0 == *str_end )
         {
            msg_status = AT_NO_ERROR_E;
         }
      }

      if( AT_NO_ERROR_E == msg_status )
      {
         rach_tx_rx_offset = offset;
      }
   }
   
   /* Write operation succeeded, allocate pool for response */
   pCmdReply = CALLOC_ATHANDLE_POOL

   if (pCmdReply)
   {
      pCmdReply->port_id = uart_port;
      
      if ( AT_NO_ERROR_E == msg_status )
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
      }
      else 
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      }
      
      /* Send the response */
      status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         /* Failed to write, free the pre-allocated pool entry */
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
      else
      {
         /* All done successfully */
         msg_status = AT_NO_ERROR_E;
      }
   }
   else
   {
      msg_status = AT_QUEUE_ERROR_E;
   }

   return msg_status;
}
#endif // ENABLE_TDM_CALIBRATION

/*************************************************************************************/
/**
* function name   : MM_ATCommand_SoundLevel
* description     : Set the volume of the sound channel.  The optional [unit] parameter
*                   is for targeting remote devices over the mesh.
*                   ATSL=[unit,]<channel>,<level> or ATSL?[unit,]<channel>  where [unit] takes th form ZxUx
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t param_id  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_SoundLevel(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   // Set up the parameters
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   osStatus status;
   uint16_t unit_address;
   uint16_t zone;
   uint32_t channel;
   uint32_t value;

   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if (pCmdReply)
   {
      //Read the parameters from the AT comand
      msg_status = AT_INVALID_PARAMETER_E;
      if ( source_buffer )
      {
         AT_Parameters_t parameters;
         parameters.NumberOfParameters = 0;
         msg_status = AT_NO_ERROR_E;
         
         /*read the Destination */
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( AT_NO_ERROR_E == msg_status )
         {
            //The command can have between 1 and 4 parameters
            switch ( parameters.NumberOfParameters )
            {
               case 1:
                  //Read local
                  zone = ZONE_GLOBAL;
                  unit_address = gNodeAddress;
                  channel = parameters.Parameter[0];
                  value = 0;
                  break;
               case 2:
                  //Write local
                  zone = ZONE_GLOBAL;
                  unit_address = gNodeAddress;
                  channel = parameters.Parameter[0];
                  value = parameters.Parameter[1];
                  break;
               case 3:
                  //Read addressed
                  zone = parameters.Parameter[0];
                  unit_address = parameters.Parameter[1];
                  channel = parameters.Parameter[2];
                  value = 0;
                  break;
               case 4:
                  //Write addressed
                  zone = parameters.Parameter[0];
                  unit_address = parameters.Parameter[1];
                  channel = parameters.Parameter[2];
                  value = parameters.Parameter[3];
                  break;
               default:
                  msg_status = AT_INVALID_COMMAND_E;
               break;
            }
         }
         
         if ( AT_NO_ERROR_E == msg_status )
         {
            msg_status = AT_INVALID_COMMAND_E;
            
            if ( AT_COMMAND_READ_E == command_type )
            {
               //if the unit address matches this unit, do a local read
               if ( unit_address == gNodeAddress )
               {
                  uint32_t sound_level;
                  if ( SUCCESS_E == DM_NVMRead(NV_SOUND_LEVEL_E, &sound_level, sizeof(sound_level)) )
                  {
                     /* Prepare the response */
                     sprintf((char *)pCmdReply->data_buffer,"%d", sound_level);
                     pCmdReply->port_id = uart_port;
                     pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
                     msg_status = AT_NO_ERROR_E;
                  }
                  else 
                  {
                     msg_status = AT_NVM_ERROR_E;
                  }
               }
               else 
               {
                  //The address is another device on the mesh
                  CO_CommandData_t CmdMessage;

                  /* create the command message and put into mesh queue */
                  CO_Message_t *pCmdSignalReq = osPoolAlloc(AppPool);
                  if (pCmdSignalReq)
                  {
                     pCmdSignalReq->Type = CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E;
                     CmdMessage.CommandType = PARAM_TYPE_SOUND_LEVEL_E;
                     CmdMessage.Destination = unit_address;
                     CmdMessage.ReadWrite = COMMAND_READ;
                     CmdMessage.Source = uart_port;
                     CmdMessage.Parameter1 = channel;
                     CmdMessage.Parameter2 = zone;
                     CmdMessage.Value = value;
                     
                     uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
                     memcpy(pCmdReq, (uint8_t*)&CmdMessage, sizeof(CO_CommandData_t));
                     pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_CommandData_t);
                     
                     status = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
                     if (osOK != status)
                     {
                        /* failed to write */
                        osPoolFree(AppPool, pCmdSignalReq);
                        msg_status = AT_QUEUE_ERROR_E;
                     }
                     else 
                     {
                        /* All done successfully */
                        msg_status = AT_NO_ERROR_E;
                        /* Prepare the OK response */
                        strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                        pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
                     }
                  }
                  else 
                  {
                     msg_status = AT_QUEUE_ERROR_E;
                  }
               }
               
            }
            else if ( AT_COMMAND_WRITE_E == command_type )
            {
               //Send write commands to the Application where it will write the new value
               //to NVM and dynamically update the output manager
               CIECommand_t CmdMessage;

               /* create the command message and put into mesh queue */
               CO_Message_t *pCmdSignalReq = osPoolAlloc(AppPool);
               if (pCmdSignalReq)
               {
                  pCmdSignalReq->Type = CO_MESSAGE_CIE_COMMAND_E;
                  CmdMessage.CommandType = CIE_CMD_SOUND_LEVEL_E;
                  CmdMessage.NodeID = unit_address;
                  CmdMessage.Parameter1 = channel;
                  CmdMessage.Parameter2 = zone;
                  CmdMessage.Value = value; //sound level
                  CmdMessage.ReadWrite = COMMAND_WRITE;
                  CmdMessage.Zone = zone;
                  CmdMessage.PortNumber = uart_port;
                  uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
                  memcpy(pCmdReq, (uint8_t*)&CmdMessage, sizeof(CO_CommandData_t));
                  pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_CommandData_t);
                  
                  status = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
                  if (osOK != status)
                  {
                     /* failed to write */
                     osPoolFree(AppPool, pCmdSignalReq);
                     msg_status = AT_QUEUE_ERROR_E;
                  }
                  else 
                  {
                     /* All done successfully */
                     msg_status = AT_NO_ERROR_E;
                     /* Prepare the OK response */
                     strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                     pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
                  }
               }
               else 
               {
                  msg_status = AT_QUEUE_ERROR_E;
               }
            }
         }
      }
      
      
      if (AT_NO_ERROR_E != msg_status)
      {
         /* Prepare the fail response */
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      }
      
      pCmdReply->port_id = uart_port;
      status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         // failed to queue
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
   }
   
   
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_TestOneShot
* description     : Command a one-shot alarm test.
*                   ATTOS+<s>  s=0 for silent test. s=1 for standard test
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_TestOneShot(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_SPECIAL_E == command_type)
      {
         uint16_t silentOrStandard = 0;
         char *str_end;
         silentOrStandard = (uint16_t)strtol((char*)source_buffer, &str_end, 10);
         if ( 0 == *str_end )
         {
            /* Send the command to the application */
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_TEST_ONE_SHOT_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = ADDRESS_GLOBAL;
               pCommand->Parameter1 = 0;
               pCommand->Parameter2 = (uint8_t)ZONE_GLOBAL;
               pCommand->Value = silentOrStandard;
               pCommand->Zone = ZONE_GLOBAL;
               pCommand->ReadWrite = COMMAND_WRITE;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
            else 
            {
               msg_status = AT_QUEUE_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_Confirm
* description     : Issue a Fire/First Aid Confirmed message.
*                   CONF+  Fire
*                   CONE+  First Aid
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_Confirm(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( AT_COMMAND_SPECIAL_E == command_type)
   {
      /* Send the command to the application */
      CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
      if ( pCIEReq )
      {
         pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
         
         CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
         pCommand->CommandType = CIE_CMD_CONFIRM_E;
         pCommand->PortNumber = (uint8_t)uart_port;
         pCommand->NodeID = ADDRESS_GLOBAL;
         pCommand->Parameter1 = 0;
         pCommand->Parameter2 = (uint8_t)ZONE_GLOBAL;
         pCommand->Value = option_param;
         pCommand->Zone = ZONE_GLOBAL;
         pCommand->ReadWrite = COMMAND_WRITE;
         
         osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCIEReq);
            msg_status = AT_QUEUE_ERROR_E;
         }
         else
         {
            msg_status = AT_NO_ERROR_E;
         }
      }
      else 
      {
         msg_status = AT_QUEUE_ERROR_E;
      }
   }

   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_Confirm
* description     : Issue a Fire COnfirmed message.
*                    ATOPC?
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ReportOutputConfiguration(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_QUEUE_ERROR_E;
   osStatus status;
   DM_OP_ReportAlarmConfiguration(uart_port);
   
   /* Prepare the response */
   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if (pCmdReply)
   {
      strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
      pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
      status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         // failed to queue
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
      else 
      {
         msg_status = AT_NO_ERROR_E;
      }
   }

   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ChannelFlags
* description     : Set the option flags for an input/output channel
*                   ATCHF?<channel>
*                   ATCHF=<channel><bitfield>
*                   The bits are:
*                    0 = inverted
*                    1 = ignore night delays
*                    2 = silenceable.
*                    3 = day enable
*                    4 = night enable
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ChannelFlags(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   AT_Parameters_t parameters;
   uint32_t device = 0;
   uint32_t zone = 0;
   uint32_t channel = 0;
   uint32_t bitfield = 0;
   uint32_t readWrite = 0;
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_WRITE_E == command_type)
      {
         readWrite = COMMAND_WRITE;
         
         msg_status = AT_NO_ERROR_E;
         
         //Read the target address, channel and bitfield
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( AT_NO_ERROR_E == msg_status )
         {
            if ( 4 == parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               device = parameters.Parameter[1];
               channel = parameters.Parameter[2];
               bitfield = parameters.Parameter[3];
            }
            else if ( 2 == parameters.NumberOfParameters )
            {
               zone = ZONE_GLOBAL;
               device = gNodeAddress;
               channel = parameters.Parameter[0];
               bitfield = parameters.Parameter[1];
            }
         }
      }
      else 
      {
         readWrite = COMMAND_READ;
         
         msg_status = AT_NO_ERROR_E;
         
         //Read the target address, channel and bitfield
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( AT_NO_ERROR_E == msg_status )
         {
            if ( 3 == parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               device = parameters.Parameter[1];
               channel = parameters.Parameter[2];
            }
            else if ( 1 == parameters.NumberOfParameters )
            {
               zone = ZONE_GLOBAL;
               device = gNodeAddress;
               channel = parameters.Parameter[0];
            }
         }
      }
      
      /* Send the command to the application */
      if ( AT_NO_ERROR_E == msg_status )
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
            CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
            pCommand->CommandType = CIE_CMD_CHANNEL_FLAGS_E;
            pCommand->PortNumber = (uint8_t)uart_port;
            pCommand->NodeID = device;
            pCommand->Parameter1 = channel;
            pCommand->Parameter2 = zone;
            pCommand->Value = bitfield;
            pCommand->Zone = zone;
            pCommand->ReadWrite = readWrite;
            
            osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
            if (osOK != osStat)
            {
               /* failed to write */
               osPoolFree(AppPool, pCIEReq);
               msg_status = AT_QUEUE_ERROR_E;
            }
            else
            {
               msg_status = AT_NO_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}


/*************************************************************************************/
/**
* function name   : MM_ATCommand_MaximumRank
* description     : Set the highest rank that the system will grow to
*                   ATRNK=[ZxUx,]<rank>
*                   ATRNK?[ZxUx]
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_MaximumRank(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   AT_Parameters_t parameters;
   uint32_t rank = 0;
   uint32_t address = 0;
   uint32_t zone = 0;
   uint32_t readWrite = 0;
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      //Read the target address, channel and bitfield
      msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
      if ( AT_NO_ERROR_E == msg_status )
      {
         if ( AT_COMMAND_WRITE_E == command_type)
         {
            readWrite = COMMAND_WRITE;
            
            if ( AT_NO_ERROR_E == msg_status )
            {
               if ( 3 == parameters.NumberOfParameters )
               {
                  zone = parameters.Parameter[0];
                  address = parameters.Parameter[1];
                  rank = parameters.Parameter[2];
               }
               else if ( 1 == parameters.NumberOfParameters )
               {
                  address = ADDRESS_GLOBAL;
                  zone = ZONE_GLOBAL;
                  rank = parameters.Parameter[0];
               }
               else
               {
                  msg_status = AT_INVALID_COMMAND_E;
               }
            }
         }
         else if ( AT_COMMAND_READ_E == command_type)
         {
            if ( AT_NO_ERROR_E == msg_status )
            {
               if ( 2 == parameters.NumberOfParameters )
               {
                  zone = parameters.Parameter[0];
                  address = parameters.Parameter[1];
               }
               else if ( 0 == parameters.NumberOfParameters )
               {
                  address = gNodeAddress;
                  zone = ZONE_GLOBAL;
               }
               else
               {
                  msg_status = AT_INVALID_COMMAND_E;
               }
            }
         }
         else 
         {
            msg_status = AT_INVALID_COMMAND_E;
         }
      }
      
      /* Send the command to the application */
      if ( AT_NO_ERROR_E == msg_status )
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
            CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
            pCommand->CommandType = CIE_CMD_MAX_RANK_E;
            pCommand->PortNumber = (uint8_t)uart_port;
            pCommand->NodeID = address;
            pCommand->Parameter1 = 0;
            pCommand->Parameter2 = zone;
            pCommand->Value = rank;
            pCommand->Zone = zone;
            pCommand->ReadWrite = readWrite;
            
            osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
            if (osOK != osStat)
            {
               /* failed to write */
               osPoolFree(AppPool, pCIEReq);
               msg_status = AT_QUEUE_ERROR_E;
            }
            else
            {
               msg_status = AT_NO_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_NCUPowerFail
* description     : Development aid.
*                   Instigate NCU power saving mode (Dynamix Tx power)
*                   ATPF=0|1    0=normal, 1=power saving
*                   ATPF?
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_NCUPowerFail(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   AT_Parameters_t parameters;
   uint32_t fault = 0;
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_WRITE_E == command_type)
      {
         //Read the fault state
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);

         
         if ( AT_NO_ERROR_E == msg_status )
         {
            if ( 1 == parameters.NumberOfParameters )
            {
               fault = parameters.Parameter[0];
               msg_status = AT_NO_ERROR_E;
            }
            else
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
         }
      }
      /* Send the command to the application */
      if ( AT_NO_ERROR_E == msg_status )
      {
         ErrorCode_t result = CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_BATTERY_ERROR_E, fault, false, false);
         if ( SUCCESS_E != result )
         {
            CO_PRINT_B_1(DBG_ERROR_E,"Failed to send NCU power fault message. Error=%d\r\n", result);
            msg_status = AT_QUEUE_ERROR_E;
         }
      }
   }
   return msg_status;
}


/*************************************************************************************/
/**
* function name   : MM_ATCommand_DisableBatteryChecks
* description     : Switch on/off battery checks
*                   ATDBC=0|1   1=disabled, 0 = enabled
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_DisableBatteryChecks(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   // Set up the parameters
   uint32_t disabled_status = 0;
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   osStatus status;

   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if (pCmdReply)
   {
      switch (command_type) 
      {
         case AT_COMMAND_READ_E:
         {
            /* This command doesn't expect any additional parameter in read operation*/
            if (0 == buffer_length)
            {
               if ( false == CFG_GetBatteryCheckEnabled() )
               {
                  disabled_status = 1;
               }
               else 
               {
                  disabled_status = 0;
               }
               msg_status = AT_NO_ERROR_E;
               CO_HexString(disabled_status, (char *)&pCmdReply->data_buffer, BASE_10);
            }
         }
         break;
       
         case AT_COMMAND_WRITE_E:
         {
            msg_status = AT_INVALID_PARAMETER_E;
            if ( source_buffer )
            {
               if (0 < buffer_length)
               {
                  char *str_end;
                  
                  /* Get the bank value */
                  disabled_status = (uint32_t)strtol((char*)source_buffer, &str_end, 10);

                  if(!*str_end)
                  {
                     if ( 0 == disabled_status )
                     {
                        //Turn on battery checks
                        CFG_SetBatteryCheckEnabled(true);
                     }
                     else 
                     {
                        //Turn off battery checks
                        CFG_SetBatteryCheckEnabled(false);
                     }
                     msg_status = AT_NO_ERROR_E;
                     strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                  }
               }
            }
         }
         break;

         default:
         {
            msg_status = AT_INVALID_COMMAND_E;
         }
      }
      
      if (AT_NO_ERROR_E == msg_status)
      {  
         pCmdReply->port_id = uart_port;
         pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
         status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK != status)
         {
            FREE_ATHANDLE_POOL(pCmdReply);
            msg_status = AT_QUEUE_ERROR_E;
         }
      }
      else 
      {
         //Command failed.  Free memory.
         FREE_ATHANDLE_POOL(pCmdReply);
      }
   }
   
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_LedPattern
* description     : Set or read the LED pattern
*                   ATLEDP=<pattern>,<off/on>
*                   ATLEDP?
*                   ATLEDP+   Remove all patterns
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_LedPattern(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   // Set up the parameters
   uint32_t pattern = 0;
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   osStatus status;

   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if (pCmdReply)
   {
      switch (command_type) 
      {
         case AT_COMMAND_READ_E:
         {
            /* This command doesn't expect any additional parameter in read operation*/
            if (0 == buffer_length)
            {
               pattern = (uint32_t)DM_LedGetCurrentPattern();
               msg_status = AT_NO_ERROR_E;
               CO_HexString(pattern, (char *)&pCmdReply->data_buffer, BASE_10);
            }
         }
         break;
       
         case AT_COMMAND_WRITE_E:
         {    
            msg_status = AT_INVALID_PARAMETER_E;
            if ( source_buffer )
            {
               if (0 < buffer_length)
               {
                  //extract the  parameters
                  AT_Parameters_t parameters;
                  msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);

                  if(AT_NO_ERROR_E == msg_status)
                  {
                     if ( 2 == parameters.NumberOfParameters )
                     {
                        pattern = parameters.Parameter[0];
                        uint32_t onOff = parameters.Parameter[1];
                        if ( LED_PATTERNS_MAX_E > pattern )
                        {
                           if ( onOff )
                           {
                              DM_LedPatternRequest((DM_LedPatterns_t)pattern);
                           }
                           else 
                           {
                              DM_LedPatternRemove((DM_LedPatterns_t)pattern);
                           }
                           strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                        }
                        else
                        {
                           msg_status = AT_INVALID_COMMAND_E;
                        }
                     }
                     else
                     {
                        msg_status = AT_INVALID_COMMAND_E;
                     }
                  }
               }
            }
         }
         break;
         
         case AT_COMMAND_SPECIAL_E:
         {
            if ( SUCCESS_E == DM_LedPatternRemoveAll() )
            {
               msg_status = AT_NO_ERROR_E;
            }
            else 
            {
               msg_status = AT_OPERATION_FAIL_E;
            }
         }
         break;

         default:
         {
            msg_status = AT_INVALID_COMMAND_E;
         }
      }
      
      if (AT_NO_ERROR_E == msg_status)
      {
         pCmdReply->port_id = uart_port;
         pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
         status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK != status)
         {
            // failed to write          
            ;
            // RM: TODO
            FREE_ATHANDLE_POOL(pCmdReply);
            msg_status = AT_QUEUE_ERROR_E;
         }
      }
      else 
      {
         //Command failed.  Free memory.
         FREE_ATHANDLE_POOL(pCmdReply);
      }
   }
   
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_DumpNeighbourInformation
* description     : Send the mesh cache to the debug port for nodes in the given range
*                   Cmd format ATDNIR+[first node,]<last node>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_DumpNeighbourInformation(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   
   if ( AT_COMMAND_SPECIAL_E == command_type )
   {
      if ( source_buffer )
      {
         // Allocate Pool
         Cmd_Reply_t *pCmdReply = NULL;
         pCmdReply = CALLOC_ATHANDLE_POOL
         
         if (pCmdReply)
         {
            uint32_t firstNode = 1;
            uint32_t lastNode = MAX_DEVICES_PER_SYSTEM;
            AT_Parameters_t parameters;
            
            msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
            
            if(AT_NO_ERROR_E == msg_status)
            {
               if ( 2 == parameters.NumberOfParameters )
               {
                  firstNode = parameters.Parameter[0];
                  lastNode = parameters.Parameter[1];
               }
               else if ( 1 == parameters.NumberOfParameters )
               {
                  lastNode = parameters.Parameter[0];
               }
               
               strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
               pCmdReply->port_id = uart_port;
               pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
               osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
               if (osOK != status)
               {
                  FREE_ATHANDLE_POOL(pCmdReply);
                  msg_status = AT_QUEUE_ERROR_E;
               }
                     
               MM_NI_DumpNeighbourInformation(firstNode, lastNode);
            }
            else 
            {
               //Command failed.  Free memory.
               FREE_ATHANDLE_POOL(pCmdReply);
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_DisableFaultReports
* description     : Set or read the 'fault reports enabled' config setting
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_DisableFaultReports(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   // Set up the parameters
   bool faults_disabled = false;
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   osStatus status;

   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if (pCmdReply)
   {
      switch (command_type) 
      {
         case AT_COMMAND_READ_E:
         {
            /* This command doesn't expect any additional parameter in read operation*/
            if (0 == buffer_length)
            {
               faults_disabled = !CFG_GetFaultReportsEnabled();
               msg_status = AT_NO_ERROR_E;
               CO_HexString((uint32_t)faults_disabled, (char *)&pCmdReply->data_buffer, BASE_10);
            }
         }
         break;
       
         case AT_COMMAND_WRITE_E:
         {    
            msg_status = AT_INVALID_PARAMETER_E;
            if ( source_buffer )
            {
               if (0 < buffer_length)
               {
                  //extract the  parameters
                  AT_Parameters_t parameters;
                  msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);

                  if(AT_NO_ERROR_E == msg_status)
                  {
                     if ( 1 == parameters.NumberOfParameters )
                     {
                        faults_disabled = (bool)parameters.Parameter[0];
                        CFG_SetFaultReportsEnabled( !faults_disabled );

                        strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                     }
                     else
                     {
                        msg_status = AT_INVALID_COMMAND_E;
                     }
                  }
               }
            }
         }
         break;

         default:
         {
            msg_status = AT_INVALID_COMMAND_E;
         }
      }
      
      if (AT_NO_ERROR_E == msg_status)
      {
         pCmdReply->port_id = uart_port;
         pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
         status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK != status)
         {
            // failed to write
            ;
            // RM: TODO
            FREE_ATHANDLE_POOL(pCmdReply);
            msg_status = AT_QUEUE_ERROR_E;
         }
      }
      else 
      {
         //Command failed.  Free memory.
         FREE_ATHANDLE_POOL(pCmdReply);
      }
   }
   
   return msg_status;
}

#ifdef ENABLE_TDM_CALIBRATION
/*************************************************************************************/
/**
* function name   : MM_ATCommand_SetTdmOffset
* description     : Set or read the DCH Transmit offset
*                    ATTIM?<index>
*                    ATTIM=<index>,<value>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_SetTdmOffset(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   // Set up the parameters
   osStatus status;
   int32_t param;
   int32_t value;
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      // Allocate Pool
      Cmd_Reply_t *pCmdReply = NULL;
      pCmdReply = CALLOC_ATHANDLE_POOL
      
      if (pCmdReply)
      {
         //extract the  parameters
         AT_Parameters_t parameters;
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if(AT_NO_ERROR_E == msg_status)
         {
            if ( 1 == parameters.NumberOfParameters )
            {
               param = parameters.Parameter[0];
               value = 0;
            }
            else if ( 2 == parameters.NumberOfParameters )
            {
               param = parameters.Parameter[0];
               value = parameters.Parameter[1];
            }
         }
         switch (command_type) 
         {
            case AT_COMMAND_READ_E:
            {
               msg_status = AT_NO_ERROR_E;

               switch ( param )
               {
                  case 0:
                     value = gDchRxDoneLatency;
                     break;
                  case 1:
                     value = gDchTxOffset;
                     break;
                  case 2:
                     value = gDchRxOffset;
                     break;
                  case 3:
                     value = gRachTxOffsetDownlink;
                     break;
                  case 4:
                     value = gRachTxOffsetUplink;
                     break;
                  case 5:
                     value = gRachCadOffset;
                     break;
                  case 6:
                     value = gDlcchTxOffset;
                     break;
                  case 7:
                     value = gDlcchCadOffset;
                     break;
                  case 8:
                     value = gAckTxOffsetUplink;
                     break;
                  case 9:
                     value = gAckCadOffset;
                     break;
                  case 11:
                     value = gDchOffset;
                     break;
                  case 12:
                     value = gSyncCorrection;
                     break;
                  case 13:
                     value = gFrameSyncOffset;
                     break;
                  default:
                     msg_status = AT_INVALID_COMMAND_E;
                     break;
               }
               sprintf((char *)&pCmdReply->data_buffer,"%d",value);
            }
            break;
          
            case AT_COMMAND_WRITE_E:
            {    
               if ( 2 == parameters.NumberOfParameters )
               {
                  msg_status = AT_NO_ERROR_E;
                  switch ( param )
                  {
                     case 0:
                        gDchRxDoneLatency = value;
                        break;
                     case 1:
                        gDchTxOffset = value;
                        break;
                     case 2:
                        gDchRxOffset = value;
                        break;
                     case 3:
                        gRachTxOffsetDownlink = value;
                        break;
                     case 4:
                        gRachTxOffsetUplink = value;
                        break;
                     case 5:
                        gRachCadOffset = value;
                        break;
                     case 6:
                        gDlcchTxOffset = value;
                        break;
                     case 7:
                        gDlcchCadOffset = value;
                        break;
                     case 8:
                        gAckTxOffsetUplink = value;
                        break;
                     case 9:
                        gAckCadOffset = value;
                        break;
                     case 11:
                        gDchOffset = value;
                     break;
                     case 12:
                        gSyncCorrection = value;
                     break;
                     case 13:
                        gFrameSyncOffset = value;
                     break;
                     default:
                        msg_status = AT_INVALID_COMMAND_E;
                        break;
                  }

                  if(AT_NO_ERROR_E == msg_status)
                  {
                     strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                  }
               }
               else
               {
                  msg_status = AT_INVALID_COMMAND_E;
               }
            }
            break;

            default:
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
         }
         
         if (AT_NO_ERROR_E == msg_status)
         {
            pCmdReply->port_id = uart_port;
            pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
            status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
            
            if (osOK != status)
            {
               // failed
               FREE_ATHANDLE_POOL(pCmdReply);
               msg_status = AT_QUEUE_ERROR_E;
            }
         }
         else 
         {
            //Command failed.  Free memory.
            FREE_ATHANDLE_POOL(pCmdReply);
         }
      }
   }
   
   return msg_status;
}
#endif

/*************************************************************************************/
/**
* function name   : MM_ATCommand_BatteryTestMode
* description     : Set or read the battery monitor test mode
*                   Test modes are:
*                   0 = Test mode OFF
*                   1 = Primary Voltage test
*                   2 = Backup Voltage test
*                   3 = Primary Current test
*                   ATBMODE=<mode>
*                   ATBMODE?
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_BatteryTestMode(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   DM_BatteryTestMode_t test_mode;
   // Set up the parameters
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   osStatus status;

   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if (pCmdReply)
   {
      
      switch (command_type) 
      {
         case AT_COMMAND_READ_E:
         {
            /* This command doesn't expect any additional parameter in read operation*/
            if (0 == buffer_length)
            {
               test_mode = DM_BAT_GetTestMode();
               msg_status = AT_NO_ERROR_E;
               CO_HexString((uint32_t)test_mode, (char *)&pCmdReply->data_buffer, BASE_10);
            }
         }
         break;
       
         case AT_COMMAND_WRITE_E:
         {    
            msg_status = AT_INVALID_PARAMETER_E;
   
            if ( source_buffer )
            {
               //extract the  parameters
               AT_Parameters_t parameters;
               msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
               if ( AT_NO_ERROR_E == msg_status )
               {
                  msg_status = AT_INVALID_PARAMETER_E;
                  if (0 < buffer_length)
                  {
                     if ( 1 == parameters.NumberOfParameters )
                     {
                        if ( DM_BAT_TEST_MODE_MAX_E > parameters.Parameter[0] )
                        {
                           DM_BAT_SetTestMode((DM_BatteryTestMode_t)parameters.Parameter[0]);

                           strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                           msg_status = AT_NO_ERROR_E;
                        }
                        else 
                        {
                           msg_status = AT_INVALID_COMMAND_E;
                        }
                     }
                     else
                     {
                        msg_status = AT_INVALID_COMMAND_E;
                     }
                  }
               }
            }
         }
         break;

         default:
         {
            msg_status = AT_INVALID_COMMAND_E;
         }
      }
      
      if (AT_NO_ERROR_E == msg_status)
      {
         pCmdReply->port_id = uart_port;
         pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
         status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK != status)
         {
            FREE_ATHANDLE_POOL(pCmdReply);
            msg_status = AT_QUEUE_ERROR_E;
         }
      }
      else 
      {
         //Command failed.  Free memory.
         FREE_ATHANDLE_POOL(pCmdReply);
      }
   }
   
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ReadADC
* description     : Read the ADC value for the currect battery test mode
*                   (set by MM_ATCommand_BatteryTestMode)
*                   ATRADC?
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ReadADC(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   ErrorCode_t result = ERR_INVALID_PARAMETER_E;
   osStatus status;
   DM_BatteryTestMode_t test_mode;
   uint32_t adc_ref_mv;
   int32_t battery_mv;
   uint32_t raw_adc;
   uint32_t raw_cell_1;
   uint32_t cell_1_mv;
   int32_t cell_2_mv;
   uint32_t raw_bat1_adc;//For current readings
   uint32_t raw_bat2_adc;
   uint32_t raw_bat3_adc;
   
   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if (pCmdReply)
   {
      if (AT_COMMAND_READ_E == command_type) 
      {
         msg_status = AT_INVALID_PARAMETER_E;
         test_mode = DM_BAT_GetTestMode();
         
         if ( BASE_CONSTRUCTION_E == gBaseType )
         {
            //Read the ADC reference voltage
            result = DM_ADC_ReadReferenceVoltage(&adc_ref_mv);
            switch ( test_mode )
            {
               case DM_BAT_PRIMARY_STATIC_TEST_E:
                     //Read the Primary Battery voltage
                     result = DM_ADC_Read( &adc1Handle, ADC_CHANNEL_3, 0, &raw_adc);
                     if ( SUCCESS_E == result )
                     {
                        msg_status = AT_NO_ERROR_E;
                        battery_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(adc_ref_mv, raw_adc, ADC_RESOLUTION_12B);
                        sprintf((char *)&pCmdReply->data_buffer, "Vref=%d, raw_adc=%d, adc_mv=%d, Vbat=%d", adc_ref_mv, raw_adc, battery_mv, (battery_mv*4));
                     }
                  break;
               case DM_BAT_BACKUP_STATIC_TEST_E:
                     //Read the Backup Battery voltage
                     result = DM_ADC_Read( &adc1Handle, ADC_CHANNEL_8, 0, &raw_adc);
                     if ( SUCCESS_E == result )
                     {
                        result = DM_ADC_Read( &adc1Handle, ADC_CHANNEL_7, 0, &raw_cell_1);
                        if ( SUCCESS_E == result )
                        {
                           msg_status = AT_NO_ERROR_E;
                           battery_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(adc_ref_mv, raw_adc, ADC_RESOLUTION_12B);
                           cell_1_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(adc_ref_mv, raw_cell_1, ADC_RESOLUTION_12B);
                           cell_2_mv = battery_mv - cell_1_mv;
                           sprintf((char *)&pCmdReply->data_buffer, "Vr=%d, r_vbat=%d, r_c1=%d, c1_mv=%d, c2_mv=%d, Vb_mv=%d", adc_ref_mv, raw_adc, raw_cell_1, (cell_1_mv*4), (cell_2_mv*4), (battery_mv*4));
                        }
                     }
                  break;
               case DM_BAT_PRIMARY_CURRENT_TEST_E:
                  CO_PRINT_A_0(DBG_INFO_E,"Read ADC - No Current checks available in construction base\r\n");
                  break;
               default:
                  CO_PRINT_A_0(DBG_INFO_E,"Read ADC - Not in BAT test mode\r\n");
                  break;
            }
         }
         else 
         {
            //Read the ADC reference voltage
            result = DM_ADC_ReadReferenceVoltage(&adc_ref_mv);
            switch ( test_mode )
            {
               case DM_BAT_PRIMARY_STATIC_TEST_E:
                     //Read the Primary Battery voltage
                     result = DM_ADC_Read( &adc1Handle, ADC_CHANNEL_4, 0, &raw_adc);
                     if ( SUCCESS_E == result )
                     {
                        msg_status = AT_NO_ERROR_E;
                        int32_t battery_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(adc_ref_mv, raw_adc, ADC_RESOLUTION_12B);
                        sprintf((char *)&pCmdReply->data_buffer, "Vref=%d, raw_adc=%d, adc_mv=%d, Vbat=%d", adc_ref_mv, raw_adc, battery_mv, (battery_mv*2));
                     }
                  break;
               case DM_BAT_BACKUP_STATIC_TEST_E:
                     //Read the Backup Battery voltage
                     result = DM_ADC_Read( &adc1Handle, ADC_CHANNEL_4, 0, &raw_adc);
                     if ( SUCCESS_E == result )
                     {
                        msg_status = AT_NO_ERROR_E;
                        int32_t battery_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(adc_ref_mv, raw_adc, ADC_RESOLUTION_12B);
                        sprintf((char *)&pCmdReply->data_buffer, "Vref=%d, raw_adc=%d, adc_mv=%d, Vbat=%d", adc_ref_mv, raw_adc, battery_mv, (battery_mv*2));
                     }
                  break;
               case DM_BAT_PRIMARY_CURRENT_TEST_E:
                     // Read the primary battery voltage
                     DM_ADC_Read( &adc1Handle, ADC_CHANNEL_4, 0, &raw_adc);
                     //Read the primary Battery currents
                     if ( SUCCESS_E == DM_ADC_Read( &adc1Handle, ADC_CHANNEL_7, 0, &raw_bat1_adc) )
                     {
                        if ( SUCCESS_E == DM_ADC_Read( &adc1Handle, ADC_CHANNEL_8, 0, &raw_bat2_adc) )
                        {
                           if ( SUCCESS_E == DM_ADC_Read( &adc1Handle, ADC_CHANNEL_3, 0, &raw_bat3_adc) )
                           {
                              msg_status = AT_NO_ERROR_E;
                              sprintf((char *)&pCmdReply->data_buffer, "Vref=%d, Vbat=%d, Ibat1=%d, Ibat2=%d, Ibat3=%d", adc_ref_mv, raw_adc, raw_bat1_adc, raw_bat2_adc, raw_bat3_adc);
                           }
                        }
                     }
                  break;
               default:
                  CO_PRINT_A_0(DBG_INFO_E,"Read ADC - Not in BAT test mode\r\n");
                  break;
            }
         }
      }
      
      if (AT_NO_ERROR_E == msg_status)
      {
         pCmdReply->port_id = uart_port;
         pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
         status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK != status)
         {
            FREE_ATHANDLE_POOL(pCmdReply);
            msg_status = AT_QUEUE_ERROR_E;
         }
      }
      else 
      {
         //Command failed.  Free memory.
         FREE_ATHANDLE_POOL(pCmdReply);
      }
   }
   return msg_status;
}
#ifdef ENABLE_LINK_ADD_OR_REMOVE
/*************************************************************************************/
/**
* function name   : MM_ATCommand_DeleteNodeLink
* description     : Remove a parent/child link from the session manager
*                   ATDNL=<node_id>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_DeleteNodeLink(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   // Set up the parameters
   osStatus status;
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {

      // Allocate Pool
      Cmd_Reply_t *pCmdReply = NULL;
      pCmdReply = CALLOC_ATHANDLE_POOL
      
      if (pCmdReply)
      {
         
         switch (command_type) 
         {
            case AT_COMMAND_WRITE_E:
            {    
               //extract the  parameters
               AT_Parameters_t parameters;
               msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
               if ( AT_NO_ERROR_E == msg_status )
               {
                  msg_status = AT_INVALID_PARAMETER_E;
                  if (0 < buffer_length)
                  {
                     if ( 1 == parameters.NumberOfParameters )
                     {
                        uint16_t node_id = parameters.Parameter[0];

                        if( MAX_DEVICES_PER_SYSTEM > node_id )
                        {
                           /* Drop the fault status update into the CIE Fault queue */
                           CO_Message_t* pCmdSignalReq = osPoolAlloc(AppPool);
                           if ( pCmdSignalReq )
                           {
                              pCmdSignalReq->Type = CO_MESSAGE_MESH_STATUS_EVENT_E;
                              
                              RouteDropData_t dropMessage;
                              dropMessage.NodeID = node_id;
                              dropMessage.Handle = GetNextHandle();

                              ApplicationMessage_t appMessage;
                              appMessage.MessageType = APP_MSG_TYPE_DROP_NODE_LINK_E;
                              memcpy(appMessage.MessageBuffer, (uint8_t*)&dropMessage, sizeof(CO_StatusIndicationData_t));
                              
                              uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
                              memcpy(pCmdReq, (uint8_t*)&appMessage, sizeof(ApplicationMessage_t));
                              pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(ApplicationMessage_t);

                              osStatus osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
                              if (osOK != osStat)
                              {
                                 /* failed to write */
                                 osPoolFree(AppPool, pCmdSignalReq);
                              }
                              else 
                              {
                                 strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                                 msg_status = AT_NO_ERROR_E;
                              }
                           }
                        }
                     }
                     else
                     {
                        msg_status = AT_INVALID_COMMAND_E;
                     }
                  }
               }
            }
            break;

            default:
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
         }
         
         if (AT_NO_ERROR_E == msg_status)
         {
            pCmdReply->port_id = uart_port;
            pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
            status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
            
            if (osOK != status)
            {
               FREE_ATHANDLE_POOL(pCmdReply);
               msg_status = AT_QUEUE_ERROR_E;
            }
         }
         else 
         {
            //Command failed.  Free memory.
            FREE_ATHANDLE_POOL(pCmdReply);
         }
      }
   }
   return msg_status;
}
   
   
/*************************************************************************************/
/**
* function name   : MM_ATCommand_AddNodeLink
* description     : Add a parent/Trk node/child link to the session manager
*                   ATANL=<node_id>[,<is_parent>]
*                   If is_parent is 0, or omitted, the node is added as a child
*                   If is_parent is 1, the node is added as the next available parent/Trk node
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_AddNodeLink(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   // Set up the parameters
   osStatus status;
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      // Allocate Pool
      Cmd_Reply_t *pCmdReply = NULL;
      pCmdReply = CALLOC_ATHANDLE_POOL
      
      if (pCmdReply)
      {
         
         switch (command_type) 
         {
            case AT_COMMAND_WRITE_E:
            {    
               //extract the  parameters
               AT_Parameters_t parameters;
               msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
               if ( AT_NO_ERROR_E == msg_status )
               {
                  msg_status = AT_INVALID_PARAMETER_E;
                  if (0 < buffer_length)
                  {
                     if ( 1 <= parameters.NumberOfParameters )
                     {
                        uint16_t node_id = parameters.Parameter[0];
                        uint8_t is_parent = 0;
                        if ( 2 == parameters.NumberOfParameters )
                        {
                           is_parent = parameters.Parameter[1];
                        }

                        if( MAX_DEVICES_PER_SYSTEM > node_id )
                        {
                           /* Drop the fault status update into the CIE Fault queue */
                           CO_Message_t* pCmdSignalReq = osPoolAlloc(AppPool);
                           if ( pCmdSignalReq )
                           {
                              pCmdSignalReq->Type = CO_MESSAGE_MESH_STATUS_EVENT_E;
                              
                              RouteDropData_t dropMessage;
                              dropMessage.NodeID = node_id;
                              if ( 0 == is_parent )
                              {
                                 dropMessage.Downlink = true;
                              }
                              else 
                              {
                                 dropMessage.Downlink = false;
                              }

                              ApplicationMessage_t appMessage;
                              appMessage.MessageType = APP_MSG_TYPE_ADD_NODE_LINK_E;
                              memcpy(appMessage.MessageBuffer, (uint8_t*)&dropMessage, sizeof(RouteDropData_t));
                              
                              uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
                              memcpy(pCmdReq, (uint8_t*)&appMessage, sizeof(ApplicationMessage_t));
                              pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(ApplicationMessage_t);

                              osStatus osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
                              if (osOK != osStat)
                              {
                                 /* failed to write */
                                 osPoolFree(AppPool, pCmdSignalReq);
                              }
                              else 
                              {
                                 strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                                 msg_status = AT_NO_ERROR_E;
                              }
                           }
                        }
                     }
                     else
                     {
                        msg_status = AT_INVALID_COMMAND_E;
                     }
                  }
               }
            }
            break;

            default:
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
         }
         
         if (AT_NO_ERROR_E == msg_status)
         {
            pCmdReply->port_id = uart_port;
            pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
            status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
            
            if (osOK != status)
            {
               FREE_ATHANDLE_POOL(pCmdReply);
               msg_status = AT_QUEUE_ERROR_E;
            }
         }
         else 
         {
            //Command failed.  Free memory.
            FREE_ATHANDLE_POOL(pCmdReply);
         }
      }
   }
   return msg_status;
}
#endif


/*************************************************************************************/
/**
* function name   : MM_ATCommand_200HourTest
* description     : Start/stop the 200 hour test.
*                   <start/stop>=0 to stop, 1 to start.
*                   <channel> is the channel index.  Valid channels are sounder(5), beacon(6), s+v combined(10), VI(9)
*                   AT200=ZxUx,<start/stop>[,<channel>][,<channel>][,<channel>][,<channel>]
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_200HourTest(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   // Set up the parameters
   osStatus status;
   uint16_t unit_address;
   uint8_t channel = (uint8_t)TEST_200HR_CHANNEL_BEACON;
   uint8_t zone = 0;
   uint32_t value = 0;
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      // Allocate Pool
      Cmd_Reply_t *pCmdReply = NULL;
      pCmdReply = CALLOC_ATHANDLE_POOL
      
      if (pCmdReply)
      {
         
         switch (command_type) 
         {
            case AT_COMMAND_WRITE_E:
            {
               //extract the  parameters
               AT_Parameters_t parameters;
               msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
               if ( AT_NO_ERROR_E == msg_status )
               {
                  msg_status = AT_INVALID_PARAMETER_E;
                  if (0 < buffer_length)
                  {
                     if ( 2 < parameters.NumberOfParameters )
                     {
                        zone = parameters.Parameter[0];
                        unit_address = parameters.Parameter[1];
                        value = parameters.Parameter[2];
                        if ( 3 < parameters.NumberOfParameters )
                        {
                           channel = 0;
                           uint32_t param_count = 3;
                           while ( param_count < parameters.NumberOfParameters )
                           {
                              switch(parameters.Parameter[param_count])
                              {
                                 case CO_CHANNEL_SOUNDER_E:
                                    channel |= TEST_200HR_CHANNEL_SOUNDER;
                                    break;
                                 case CO_CHANNEL_BEACON_E:
                                    channel |= TEST_200HR_CHANNEL_BEACON;
                                    break;
                                 case CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E:
                                    channel |= TEST_200HR_CHANNEL_SOUND_VISUAL_INDICATOR_COMBINED;
                                    break;
                                 case CO_CHANNEL_VISUAL_INDICATOR_E:
                                    channel |= TEST_200HR_CHANNEL_VISUAL_INDICATOR;
                                    break;
                                 default:
                                    break;
                              }
                              param_count++;
                           }
                           //channel = parameters.Parameter[3];
                        }
                         //Start/stop the 200 hour test
                        //Send write commands to the Application where it will write the new value
                        //to NVM and dynamically update the output manager
                        CIECommand_t CmdMessage;

                        /* create the command message and put into mesh queue */
                        CO_Message_t *pCmdSignalReq = osPoolAlloc(AppPool);
                        if (pCmdSignalReq)
                        {
                           pCmdSignalReq->Type = CO_MESSAGE_CIE_COMMAND_E;
                           CmdMessage.CommandType = CIE_CMD_ENABLE_200_HOUR_TEST_E;
                           CmdMessage.NodeID = unit_address;
                           CmdMessage.Parameter1 = channel;
                           CmdMessage.Parameter2 = zone;
                           CmdMessage.Value = value; //start/stop 200 hour test
                           CmdMessage.ReadWrite = COMMAND_WRITE;
                           CmdMessage.Zone = zone;
                           CmdMessage.PortNumber = uart_port;
                           uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
                           memcpy(pCmdReq, (uint8_t*)&CmdMessage, sizeof(CO_CommandData_t));
                           pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_CommandData_t);
                           
                           status = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
                           if (osOK != status)
                           {
                              /* failed to write */
                              osPoolFree(AppPool, pCmdSignalReq);
                              msg_status = AT_QUEUE_ERROR_E;
                           }
                           else 
                           {
                              /* All done successfully */
                              msg_status = AT_NO_ERROR_E;
                           }
                        }

                        strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                        msg_status = AT_NO_ERROR_E;

                     }
                     else
                     {
                        msg_status = AT_INVALID_COMMAND_E;
                     }
                  }
               }
            }
            break;

            default:
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
         }
         
         if (AT_NO_ERROR_E == msg_status)
         {
            pCmdReply->port_id = uart_port;
            pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
            status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
            
            if (osOK != status)
            {
               FREE_ATHANDLE_POOL(pCmdReply);
               msg_status = AT_QUEUE_ERROR_E;
            }
         }
         else 
         {
            //Command failed.  Free memory.
            FREE_ATHANDLE_POOL(pCmdReply);
         }
      }
   }
   return msg_status;
}


/*************************************************************************************/
/**
* function name   : MM_ATCommand_CheckFirmware
* description     : Broadcast a firmware check command to all radio boards on the mesh.
*                   ATCHFW+
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_CheckFirmware(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( AT_COMMAND_SPECIAL_E == command_type )
   {
      msg_status = AT_NO_ERROR_E;
      
      CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
      if ( pCIEReq )
      {
         pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
         
         CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
         pCommand->CommandType = CIE_CMD_CHECK_FIRMWARE_E;
         pCommand->PortNumber = (uint8_t)uart_port;
         pCommand->NodeID = ADDRESS_GLOBAL;
         pCommand->Parameter1 = 0; /*Head*/
         pCommand->Parameter2 = (uint8_t)ZONE_GLOBAL;
         pCommand->Value = 0;
         pCommand->Zone = ZONE_GLOBAL;
         pCommand->ReadWrite = 1;

         osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCIEReq);
            msg_status = AT_QUEUE_ERROR_E;
         }
         else
         {
            msg_status = AT_NO_ERROR_E;
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_SetLocalOrGlobalDelays
* description     : Set the system to use local of global delays.
*                   ATLDLY=[ZxUx,]<channel>,<0|1>   0=global, 1=local
*                   ATLDLY?[ZxUx,]<channel>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_SetLocalOrGlobalDelays(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint16_t unit_address;
   uint8_t zone;
   uint8_t channel;
   uint32_t globalLocal = 0;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type || AT_COMMAND_WRITE_E == command_type)
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( SUCCESS_E == msg_status )
         {
            msg_status = AT_INVALID_COMMAND_E;
            /* If this is a WRITE command, read the value */
            if ( AT_COMMAND_WRITE_E == command_type )
            {
               if ( 4 == parameters.NumberOfParameters )
               {
                  zone = parameters.Parameter[0];
                  unit_address = parameters.Parameter[1];
                  channel = parameters.Parameter[2];
                  globalLocal = parameters.Parameter[3];
                  msg_status = AT_NO_ERROR_E;
               }
               else if ( 2 == parameters.NumberOfParameters )
               {
                  zone = ZONE_GLOBAL;
                  unit_address = gNodeAddress;
                  channel = parameters.Parameter[0];
                  globalLocal = parameters.Parameter[1];
                  msg_status = AT_NO_ERROR_E;
               }
            }
            else if ( AT_COMMAND_READ_E == command_type )
            {
               if ( 3 == parameters.NumberOfParameters )
               {
                  zone = parameters.Parameter[0];
                  unit_address = parameters.Parameter[1];
                  channel = parameters.Parameter[2];
                  msg_status = AT_NO_ERROR_E;
               }
               else if ( 1 == parameters.NumberOfParameters )
               {
                  zone = ZONE_GLOBAL;
                  unit_address = gNodeAddress;
                  channel = parameters.Parameter[0];
                  msg_status = AT_NO_ERROR_E;
               }
            }
            
            /* Send the command to the application */
            if ( AT_NO_ERROR_E == msg_status )
            {
               CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
               if ( pCIEReq )
               {
                  pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
                  
                  CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
                  pCommand->CommandType = CIE_CMD_SET_LOCAL_OR_GLOBAL_ALARM_DELAYS_E;
                  pCommand->PortNumber = (uint8_t)uart_port;
                  pCommand->NodeID = unit_address;
                  pCommand->Parameter1 = channel;
                  pCommand->Parameter2 = zone;
                  pCommand->Value = globalLocal;
                  pCommand->Zone = zone;
                  if ( AT_COMMAND_READ_E == command_type )
                  {
                     pCommand->ReadWrite = 0;
                  }
                  else 
                  {
                     pCommand->ReadWrite = 1;
                  }
                  
                  osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
                  if (osOK != osStat)
                  {
                     /* failed to write */
                     osPoolFree(AppPool, pCIEReq);
                     msg_status = AT_QUEUE_ERROR_E;
                  }
                  else
                  {
                     msg_status = AT_NO_ERROR_E;
                  }
               }
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_SetGlobalDelayOverride
* description     : Set the global delay override.  Setting this will make the
*                   alarm outputs use the common global delay1 and delay2 values
*                   instead of the local delays stored in the channel config.
*                   ATGDLY=<0|1>   0=local, 1=global
*                   ATGDLY?
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_SetGlobalDelayOverride(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint16_t unit_address;
   uint8_t zone;
   uint32_t override = 0;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type || AT_COMMAND_WRITE_E == command_type)
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( SUCCESS_E == msg_status )
         {
            msg_status = AT_INVALID_COMMAND_E;
            /* If this is a WRITE command, read the value */
            if ( AT_COMMAND_WRITE_E == command_type )
            {
               if ( 1 == parameters.NumberOfParameters )
               {
                  zone = ZONE_GLOBAL;
                  unit_address = ADDRESS_GLOBAL;
                  override = parameters.Parameter[0];
                  msg_status = AT_NO_ERROR_E;
               }
            }
            else if ( AT_COMMAND_READ_E == command_type )
            {
               msg_status = AT_NO_ERROR_E;
            }
            
            /* Send the command to the application */
            if ( AT_NO_ERROR_E == msg_status )
            {
               CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
               if ( pCIEReq )
               {
                  pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
                  
                  CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
                  pCommand->CommandType = CIE_CMD_SET_GLOBAL_ALARM_DELAY_OVERRIDE_E;
                  pCommand->PortNumber = (uint8_t)uart_port;
                  pCommand->NodeID = unit_address;
                  pCommand->Parameter1 = 0;
                  pCommand->Parameter2 = zone;
                  pCommand->Value = override;
                  pCommand->Zone = zone;
                  if ( AT_COMMAND_READ_E == command_type )
                  {
                     pCommand->ReadWrite = 0;
                  }
                  else 
                  {
                     pCommand->ReadWrite = 1;
                  }
                  
                  osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
                  if (osOK != osStat)
                  {
                     /* failed to write */
                     osPoolFree(AppPool, pCIEReq);
                     msg_status = AT_QUEUE_ERROR_E;
                  }
                  else
                  {
                     msg_status = AT_NO_ERROR_E;
                  }
               }
            }
         }
      }
   }
   return msg_status;
}


/*************************************************************************************/
/**
* function name   : MM_ATCommand_SetGlobalAlarmDelays
* description     : Command to set the times used by global delays on alarms.
*                   ATGADC=<delay1>,<delay2>
*                   ATGADC?
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_SetGlobalAlarmDelays(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint32_t delay1;
   uint32_t delay2;
   uint32_t combined_delays = 0;
   uint8_t read_write = 0;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type || AT_COMMAND_WRITE_E == command_type )
      {
         if ( AT_COMMAND_WRITE_E == command_type )
         {
            //extract the  parameters
            msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
            if ( SUCCESS_E == msg_status )
            {
               if ( 2 == parameters.NumberOfParameters )
               {
                  delay1 = parameters.Parameter[0];
                  delay2 = parameters.Parameter[1];
                  //combine the two delays into a single value with delay 2 in the high 16 bits
                  combined_delays = (delay2 << 16) + delay1;
                  read_write = 1;
                  msg_status = AT_NO_ERROR_E;
               }
               else 
               {
                  msg_status = AT_INVALID_COMMAND_E;
               }
            }
         }
         else 
         {
            //Read command
            msg_status = AT_NO_ERROR_E;
         }

         
         /* Send the command to the application */
         if ( AT_NO_ERROR_E == msg_status )
         {
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_SET_GLOBAL_ALARM_DELAY_VALUES_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = ADDRESS_GLOBAL;
               pCommand->Parameter1 = 0;
               pCommand->Parameter2 = (uint8_t)ZONE_GLOBAL;
               pCommand->Value = combined_delays;
               pCommand->Zone = ZONE_GLOBAL;
               pCommand->ReadWrite = read_write;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
   }
   return msg_status;
}



/*************************************************************************************/
/**
* function name   : MM_ATCommand_EmulatedEprom
* description     : Command to read/write to one of the 10 reserved NVM parameters.
*                   The valid index range is 0 to 9.
*                   ATEEI=[ZxUx,]<index>,<value>
*                   ATEEI?[ZxUx,]<index>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_EmulatedEprom(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status;
   CO_CommandData_t CmdMessage;
   CO_Message_t* pCmdSignalReq;
   osStatus osStat;
   AT_Parameters_t parameters;
   
   if ( source_buffer )
   {
      CmdMessage.Source = uart_port;
      
      msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
      
      if ( AT_NO_ERROR_E == msg_status )
      {
         if ( 4 == parameters.NumberOfParameters )
         {
            //read/write remote device
            CmdMessage.Parameter2 = parameters.Parameter[0];//zone
            CmdMessage.Destination = parameters.Parameter[1];
            CmdMessage.Parameter1 = parameters.Parameter[2];//NVM parameter
            CmdMessage.Value = parameters.Parameter[3];
         }
         else if ( 2 == parameters.NumberOfParameters )
         {
            //write local device
            CmdMessage.Parameter2 = ZONE_GLOBAL;
            CmdMessage.Destination = gNodeAddress;
            CmdMessage.Parameter1 = parameters.Parameter[0];//NVM parameter
            CmdMessage.Value = parameters.Parameter[1];
         }
         else if ( 1 == parameters.NumberOfParameters )
         {
            //read local device
            CmdMessage.Parameter2 = ZONE_GLOBAL;
            CmdMessage.Destination = gNodeAddress;
            CmdMessage.Parameter1 = parameters.Parameter[0];//NVM parameter
            CmdMessage.Value = 0;
         }
         else 
         {
            msg_status = AT_INVALID_COMMAND_E;
         }
         
         if ( AT_NO_ERROR_E == msg_status )
         {
            msg_status = AT_QUEUE_ERROR_E;//assume failure
            /* create the command message and put into mesh queue */
            pCmdSignalReq = osPoolAlloc(AppPool);
            if (pCmdSignalReq)
            {
               pCmdSignalReq->Type = CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E;
               CmdMessage.CommandType = PARAM_TYPE_EEPROM_INFORMATION_E;
               CmdMessage.SourceUSART = uart_port;
               CmdMessage.ReadWrite = command_type;

               uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
               memcpy(pCmdReq, (uint8_t*)&CmdMessage, sizeof(CO_CommandData_t));
               pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_CommandData_t);
               
               osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCmdSignalReq);
               }
               else 
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ReadWriteNVM
* description     : Command to read/write to one of the NVM parameters.
*                   The valid index range is 0 to 108, see DM_NVMParamId_t in DM_NVM_cfg.h.
*                   ATNVM=<index>,<value>
*                   ATNVM?<index>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ReadWriteNVM(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   AT_Parameters_t parameters;
   Cmd_Reply_t *pCmdReply = NULL;
   osStatus status;
   uint32_t nvm_index = 0;
   uint32_t value;
   
   if ( source_buffer )
   {
      msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
      if ( AT_NO_ERROR_E == msg_status )
      {
         msg_status = AT_INVALID_COMMAND_E;
         if ( AT_COMMAND_READ_E == command_type )
         {
            if ( 1 == parameters.NumberOfParameters )
            {
               nvm_index = parameters.Parameter[0];
               if ( NV_MAX_PARAM_ID_E > nvm_index )
               {
                  if ( SUCCESS_E == DM_NVMRead((DM_NVMParamId_t)nvm_index, &value, sizeof(value)) )
                  {
                     pCmdReply = CALLOC_ATHANDLE_POOL
                     if (pCmdReply)
                     {
                        pCmdReply->port_id = uart_port;
                        sprintf((char *)pCmdReply->data_buffer, "index %d = %d",nvm_index,value);
                        pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
                        status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
                        
                        if (osOK != status)
                        {
                           FREE_ATHANDLE_POOL(pCmdReply);
                           msg_status = AT_QUEUE_ERROR_E;
                        }
                        else
                        {
                           msg_status = AT_NO_ERROR_E;
                        }
                     }
                     else 
                     {
                        msg_status = AT_QUEUE_ERROR_E;
                     }
                  }
               }
            }
         }
         else if ( AT_COMMAND_WRITE_E == command_type )
         {
            msg_status = AT_INVALID_COMMAND_E;
            if ( 2 == parameters.NumberOfParameters )
            {
               nvm_index = parameters.Parameter[0];
               value = parameters.Parameter[1];
               if ( NV_MAX_PARAM_ID_E > nvm_index )
               {
                  if ( SUCCESS_E == DM_NVMWrite((DM_NVMParamId_t)nvm_index, &value, sizeof(value)) )
                  {
                     /* Write operation succeeded, allocate pool for response */
                     pCmdReply = CALLOC_ATHANDLE_POOL

                     if (pCmdReply)
                     {
                        pCmdReply->port_id = uart_port;
                        strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                        pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
                        
                        /* Send the response */
                        status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
                        
                        if (osOK != status)
                        {
                           /* Failed to write, free the pre-allocated pool entry */
                           FREE_ATHANDLE_POOL(pCmdReply);
                           msg_status = AT_QUEUE_ERROR_E;
                        }
                        else
                        {
                           /* All done successfully */
                           msg_status = AT_NO_ERROR_E;
                        }
                     }
                     else
                     {
                        msg_status = AT_QUEUE_ERROR_E;
                     }
                  }
               }
            }
         }
      }
   }
   
   if ( AT_NO_ERROR_E != msg_status )
   {
      pCmdReply->port_id = uart_port;
      strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
      pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      
      /* Send the response */
      status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         /* Failed to write, free the pre-allocated pool entry */
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
   }
   
   return msg_status;
}
/*************************************************************************************/
/**
* function name   : MM_ATCommand_MaxHops
* description     : Command to set the maximum number of hops a message will be allowed to make.
*                   ATMAXHP=<value>
*                   ATMAXHP?
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_MaxHops(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   AT_Parameters_t parameters;
   Cmd_Reply_t *pCmdReply = NULL;
   osStatus status;
   uint32_t value;
   
   if ( source_buffer )
   {
      msg_status = AT_INVALID_COMMAND_E;
      if ( AT_COMMAND_READ_E == command_type )
      {
         pCmdReply = CALLOC_ATHANDLE_POOL
         if (pCmdReply)
         {
            value = CFG_GetMaxHops();
            pCmdReply->port_id = uart_port;
            sprintf((char *)pCmdReply->data_buffer, "%d",value);
            pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
            status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
            
            if (osOK != status)
            {
               FREE_ATHANDLE_POOL(pCmdReply);
               msg_status = AT_QUEUE_ERROR_E;
            }
            else
            {
               msg_status = AT_NO_ERROR_E;
            }
         }
         else 
         {
            msg_status = AT_QUEUE_ERROR_E;
         }
      }
      else if ( AT_COMMAND_WRITE_E == command_type )
      {
         msg_status = AT_INVALID_COMMAND_E;
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( AT_NO_ERROR_E == msg_status )
         {
            if ( 1 == parameters.NumberOfParameters )
            {
               value = parameters.Parameter[0];

               if ( CFG_SetMaxHops(value) )
               {
                  /* Write operation succeeded, allocate pool for response */
                  pCmdReply = CALLOC_ATHANDLE_POOL

                  if (pCmdReply)
                  {
                     pCmdReply->port_id = uart_port;
                     strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                     pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
                     
                     /* Send the response */
                     status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
                     
                     if (osOK != status)
                     {
                        /* Failed to write, free the pre-allocated pool entry */
                        FREE_ATHANDLE_POOL(pCmdReply);
                        msg_status = AT_QUEUE_ERROR_E;
                     }
                     else
                     {
                        /* All done successfully */
                        msg_status = AT_NO_ERROR_E;
                     }
                  }
                  else
                  {
                     msg_status = AT_QUEUE_ERROR_E;
                  }
               }
            }
         }
      }
   }
   
   if ( AT_NO_ERROR_E != msg_status )
   {
      pCmdReply->port_id = uart_port;
      strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
      pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      
      /* Send the response */
      status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         /* Failed to write, free the pre-allocated pool entry */
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
   }
   
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_RequestBatteryStatus
* description     : Request a battery status message from a radio board.
*                   ATBATS?ZxUx  Remote request.
*                   ATBATS?      Print to debug.  No message sent.
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_RequestBatteryStatus(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint16_t unit_address;
   uint8_t zone;
   uint32_t local_command = 0;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( SUCCESS_E == msg_status )
         {
            msg_status = AT_INVALID_COMMAND_E;
            if ( 2 == parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               unit_address = parameters.Parameter[1];
               msg_status = AT_NO_ERROR_E;
            }
            else if ( 0 == parameters.NumberOfParameters )
            {
               //The RBU will interpret these values as 'print bat voltages to debug'
               zone = 0;
               unit_address = gNodeAddress;
               local_command = 1;
               msg_status = AT_NO_ERROR_E;
            }
            
            /* Send the command to the application */
            if ( AT_NO_ERROR_E == msg_status )
            {
               msg_status = AT_NO_RESOURCE_E;
               CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
               if ( pCIEReq )
               {
                  pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
                  
                  CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
                  pCommand->CommandType = CIE_CMD_REQUEST_BATTERY_STATUS_E;
                  pCommand->PortNumber = (uint8_t)uart_port;
                  pCommand->NodeID = unit_address;
                  pCommand->Parameter1 = 0;
                  pCommand->Parameter2 = zone;
                  pCommand->Value = local_command;
                  pCommand->Zone = zone;
                  pCommand->ReadWrite = 0;
                  
                  osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
                  if (osOK != osStat)
                  {
                     /* failed to write */
                     osPoolFree(AppPool, pCIEReq);
                     msg_status = AT_QUEUE_ERROR_E;
                  }
                  else
                  {
                     msg_status = AT_NO_ERROR_E;
                  }
               }
            }
         }
      }
   }
   return msg_status;
}


/*************************************************************************************/
/**
* function name   : MM_ATCommand_SetRssiMar
* description     : Get and Set the RSSI Minnimum Attenuation Reserve for nodes to join
*                   the mesh.
*                   ATJRSSI=0  Use a joining threshold of -107dBm(single parent) or -112dBm(two parents)
*                   ATJRSSI=1  Use a joining threshold of -112dBm in all cases
*                   ATJRSSI=2  Use a joining threshold of -117dBm in all cases
*                   ATJRSSI=3  Use a joining threshold of -122dBm in all cases
*                   ATJRSSI=4  Use a joining threshold of -102dBm in all cases
*                   ATJRSSI=5  Use a joining threshold of -97dBm in all cases
*                   ATJRSSI=6  Use a joining threshold of -92dBm in all cases
*                   ATJRSSI=7  Use a joining threshold of -87dBm in all cases
*                   ATJRSSI=8  Use a joining threshold of -82dBm in all cases
*                   ATJRSSI=9  Use a joining threshold of -77dBm in all cases
*                   ATJRSSI=10  Use a joining threshold of -72dBm in all cases
*                   ATJRSSI?   To read current setting
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_SetRssiMar(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint32_t mar;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         msg_status = AT_NO_ERROR_E;
      }
      else if ( AT_COMMAND_WRITE_E == command_type )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( SUCCESS_E == msg_status )
         {
            if ( 1 == parameters.NumberOfParameters )
            {
               mar = parameters.Parameter[0];
               //Valid settings are 0 - 10
               if ( CFG_SetRssiMarConfiguration(mar) )
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
      
      if ( AT_NO_ERROR_E == msg_status )
      {
         /* Use the common function MM_ATCommand_decimalNumber to perform the read/write to NVM */
         msg_status = MM_ATCommand_decimalNumber(command_type, source_buffer, buffer_length, uart_port, NV_RSSI_MAR_E);
      }
      else
      {
         Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
         if ( pCmdReply )
         {
            pCmdReply->port_id = uart_port;
            strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
            pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
            
            /* Send the response */
            osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
            
            if (osOK != status)
            {
               /* Failed to write, free the pre-allocated pool entry */
               FREE_ATHANDLE_POOL(pCmdReply);
               msg_status = AT_QUEUE_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_SetSnrMar
* description     : Get and Set the SNR joining threshold for nodes to join
*                   the mesh.
*                   ATJSNR=<value>  Set the SNR joining threshold value
*                   ATJSNR?         To read current setting
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_SetSnrMar(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   int32_t mar;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         msg_status = AT_NO_ERROR_E;
      }
      else if ( AT_COMMAND_WRITE_E == command_type )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( SUCCESS_E == msg_status )
         {
            if ( 1 == parameters.NumberOfParameters )
            {
               mar = parameters.Parameter[0];
               if ( CFG_SetSnrJoiningThreshold(mar) )
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
      
      if ( AT_NO_ERROR_E == msg_status )
      {
         /* Use the common function MM_ATCommand_decimalNumber to perform the read/write to NVM */
         msg_status = MM_ATCommand_decimalNumber(command_type, source_buffer, buffer_length, uart_port, NV_SNR_MAR_E);
      }
      else
      {
         Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
         if ( pCmdReply )
         {
            pCmdReply->port_id = uart_port;
            strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
            pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
            
            /* Send the response */
            osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
            
            if (osOK != status)
            {
               /* Failed to write, free the pre-allocated pool entry */
               FREE_ATHANDLE_POOL(pCmdReply);
               msg_status = AT_QUEUE_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_BatteryCheckPeriod
* description     : Get and Set the battery test interval.
*                   ATBATP=<value>  Set the battery test interval, in days.
*                   ATBATP?         To read current setting
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_BatteryCheckPeriod(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint32_t interval;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         msg_status = AT_NO_ERROR_E;
      }
      else if ( AT_COMMAND_WRITE_E == command_type )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( SUCCESS_E == msg_status )
         {
            msg_status = AT_INVALID_COMMAND_E;
            if ( 1 == parameters.NumberOfParameters )
            {
               interval = parameters.Parameter[0];
               if ( CFG_SetBatteryTestInterval(interval) )
               {
                  msg_status = AT_NO_ERROR_E;
               }
               else 
               {
                  msg_status = AT_INVALID_PARAMETER_E;
               }
            }
         }
      }
      
      if ( AT_NO_ERROR_E == msg_status )
      {
         /* Use the common function MM_ATCommand_decimalNumber to perform the read/write to NVM */
         msg_status = MM_ATCommand_decimalNumber(command_type, source_buffer, buffer_length, uart_port, NV_BATTERY_PERIOD_E);
      }
      else
      {
         Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
         if ( pCmdReply )
         {
            pCmdReply->port_id = uart_port;
            strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
            pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
            
            /* Send the response */
            osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
            
            if (osOK != status)
            {
               /* Failed to write, free the pre-allocated pool entry */
               FREE_ATHANDLE_POOL(pCmdReply);
               msg_status = AT_QUEUE_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_BatPriLowThresh
* description     : Get and Set the primary battery low threshold in mV.
*                   ATBATPS=<value>  Set the primary battery low threshold in mV for SmartNet.
*                   ATBATPS?         To read current setting
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_BatPriLowThresh(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint32_t interval;

   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         msg_status = AT_NO_ERROR_E;
      }
      else if ( AT_COMMAND_WRITE_E == command_type )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( SUCCESS_E == msg_status )
         {
            msg_status = AT_INVALID_COMMAND_E;
            if ( 1 == parameters.NumberOfParameters )
            {
               interval = parameters.Parameter[0];
               if ( CFG_SetBatPriLowVoltThresh(interval) )
               {
                  msg_status = AT_NO_ERROR_E;
               }
               else
               {
                  msg_status = AT_INVALID_PARAMETER_E;
               }
            }
         }
      }

      if ( AT_NO_ERROR_E == msg_status )
      {
         /* Use the common function MM_ATCommand_decimalNumber to perform the read/write to NVM */
         msg_status = MM_ATCommand_decimalNumber(command_type, source_buffer, buffer_length, uart_port, NV_BAT_PRI_LOW_VOLT_THRESH_E);
      }
      else
      {
         Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
         if ( pCmdReply )
         {
            pCmdReply->port_id = uart_port;
            strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
            pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);

            /* Send the response */
            osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);

            if (osOK != status)
            {
               /* Failed to write, free the pre-allocated pool entry */
               FREE_ATHANDLE_POOL(pCmdReply);
               msg_status = AT_QUEUE_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_BatBkpLowThresh
* description     : Get and Set the backup battery low threshold in mV.
*                   ATBATBS=<value>  Set the backup battery low threshold in mV for SmartNet.
*                   ATBATBS?         To read current setting
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_BatBkpLowThresh(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint32_t interval;

   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         msg_status = AT_NO_ERROR_E;
      }
      else if ( AT_COMMAND_WRITE_E == command_type )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( SUCCESS_E == msg_status )
         {
            msg_status = AT_INVALID_COMMAND_E;
            if ( 1 == parameters.NumberOfParameters )
            {
               interval = parameters.Parameter[0];
               if ( CFG_SetBatBkpLowVoltThresh(interval) )
               {
                  msg_status = AT_NO_ERROR_E;
               }
               else
               {
                  msg_status = AT_INVALID_PARAMETER_E;
               }
            }
         }
      }

      if ( AT_NO_ERROR_E == msg_status )
      {
         /* Use the common function MM_ATCommand_decimalNumber to perform the read/write to NVM */
         msg_status = MM_ATCommand_decimalNumber(command_type, source_buffer, buffer_length, uart_port, NV_BAT_BKP_LOW_VOLT_THRESH_E);
      }
      else
      {
         Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
         if ( pCmdReply )
         {
            pCmdReply->port_id = uart_port;
            strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
            pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);

            /* Send the response */
            osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);

            if (osOK != status)
            {
               /* Failed to write, free the pre-allocated pool entry */
               FREE_ATHANDLE_POOL(pCmdReply);
               msg_status = AT_QUEUE_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_BatPriLowThreshC
* description     : Get and Set the primary battery low threshold in mV for Construction (SiteNet).
*                   ATBATPC=<value>  Set the primary battery low threshold in mV.
*                   ATBATPC?         To read current setting
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_BatPriLowThreshC(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint32_t interval;

   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         msg_status = AT_NO_ERROR_E;
      }
      else if ( AT_COMMAND_WRITE_E == command_type )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( SUCCESS_E == msg_status )
         {
            msg_status = AT_INVALID_COMMAND_E;
            if ( 1 == parameters.NumberOfParameters )
            {
               interval = parameters.Parameter[0];
               if ( CFG_SetBatPriLowVoltThreshC(interval) )
               {
                  msg_status = AT_NO_ERROR_E;
               }
               else
               {
                  msg_status = AT_INVALID_PARAMETER_E;
               }
            }
         }
      }

      if ( AT_NO_ERROR_E == msg_status )
      {
         /* Use the common function MM_ATCommand_decimalNumber to perform the read/write to NVM */
         msg_status = MM_ATCommand_decimalNumber(command_type, source_buffer, buffer_length, uart_port, NV_BAT_PRI_LOW_VOLT_THRESH_C_E);
      }
      else
      {
         Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
         if ( pCmdReply )
         {
            pCmdReply->port_id = uart_port;
            strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
            pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);

            /* Send the response */
            osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);

            if (osOK != status)
            {
               /* Failed to write, free the pre-allocated pool entry */
               FREE_ATHANDLE_POOL(pCmdReply);
               msg_status = AT_QUEUE_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_BatBkpLowThreshC
* description     : Get and Set the backup battery low threshold in mV.
*                   ATBATBC=<value>  Set the backup battery low threshold in mV for Construction (SiteNet).
*                   ATBATBC?         To read current setting
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_BatBkpLowThreshC(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint32_t interval;

   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         msg_status = AT_NO_ERROR_E;
      }
      else if ( AT_COMMAND_WRITE_E == command_type )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( SUCCESS_E == msg_status )
         {
            msg_status = AT_INVALID_COMMAND_E;
            if ( 1 == parameters.NumberOfParameters )
            {
               interval = parameters.Parameter[0];
               if ( CFG_SetBatBkpLowVoltThreshC(interval) )
               {
                  msg_status = AT_NO_ERROR_E;
               }
               else
               {
                  msg_status = AT_INVALID_PARAMETER_E;
               }
            }
         }
      }

      if ( AT_NO_ERROR_E == msg_status )
      {
         /* Use the common function MM_ATCommand_decimalNumber to perform the read/write to NVM */
         msg_status = MM_ATCommand_decimalNumber(command_type, source_buffer, buffer_length, uart_port, NV_BAT_BKP_LOW_VOLT_THRESH_C_E);
      }
      else
      {
         Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
         if ( pCmdReply )
         {
            pCmdReply->port_id = uart_port;
            strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
            pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);

            /* Send the response */
            osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);

            if (osOK != status)
            {
               /* Failed to write, free the pre-allocated pool entry */
               FREE_ATHANDLE_POOL(pCmdReply);
               msg_status = AT_QUEUE_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_SetGlobalDelaysDayNight
* description     : Combined global delay and day/night settings command.
*                   All are flags except delay1 and delay2, which are in units of 30 seconds, limited to 5 bit values.
*                   ATGSET+<delays enable>,<global override>,<global delay 1>,<global delay 2>,<is day>,<ignore security in day>,<ignore security at night>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_SetGlobalDelaysDayNight(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   bool delays_enabled = false;
   bool global_override = false;
   int32_t global_delay1 = 0;
   int32_t global_delay2 = 0;
   bool is_day = false;
   bool ignore_security_in_day = false;
   bool ignore_security_at_night = false;
   uint32_t combined_value = 0;
   
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_SPECIAL_E == command_type )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( AT_NO_ERROR_E == msg_status )
         {
            msg_status = AT_INVALID_PARAMETER_E;
            if ( 7 == parameters.NumberOfParameters )
            {
               delays_enabled = 0 == parameters.Parameter[0] ? false:true;
               global_override = 0 == parameters.Parameter[1] ? false:true;
               global_delay1 = parameters.Parameter[2];
               global_delay2 = parameters.Parameter[3];
               is_day = 0 == parameters.Parameter[4] ? false:true;
               ignore_security_in_day = 0 == parameters.Parameter[5] ? false:true;
               ignore_security_at_night = 0 == parameters.Parameter[6] ? false:true;
               
               if ( ATGSET_GLOBAL_DELAY_MAX <global_delay1 )
               {
                  global_delay1 = ATGSET_GLOBAL_DELAY_MAX;
               }
               if ( ATGSET_GLOBAL_DELAY_MAX < global_delay2 )
               {
                  global_delay2 = ATGSET_GLOBAL_DELAY_MAX;
               }
               //Build the combined value
               combined_value = global_delay1;
               combined_value |= (global_delay2 << ATGSET_GLOBAL_DELAY_BIT_LENGTH);
               if ( delays_enabled )
               {
                  combined_value |= ATGSET_DELAYS_ENABLE_MASK;
               }
               if ( global_override )
               {
                  combined_value |= ATGSET_GLOBAL_OVERRIDE_MASK;
               }
               if ( is_day )
               {
                  combined_value |= ATGSET_IS_DAY_MASK;
               }
               if ( ignore_security_in_day )
               {
                  combined_value |= ATGSET_IGNORE_SECURITY_DAY_MASK;
               }
               if ( ignore_security_at_night )
               {
                  combined_value |= ATGSET_IGNORE_SECURITY_NIGHT_MASK;
               }
               
               msg_status = AT_NO_ERROR_E;
            }
         }
      }
      
      if ( AT_NO_ERROR_E == msg_status )
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
            CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
            pCommand->CommandType = CIE_CMD_SET_GLOBAL_DELAY_COMBINED_E;
            pCommand->PortNumber = (uint8_t)uart_port;
            pCommand->NodeID = ADDRESS_GLOBAL;
            pCommand->Parameter1 = 0;
            pCommand->Parameter2 = ZONE_GLOBAL;
            pCommand->Value = combined_value;
            pCommand->Zone = ZONE_GLOBAL;
            pCommand->ReadWrite = 1;
            
            osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
            if (osOK != osStat)
            {
               /* failed to write */
               osPoolFree(AppPool, pCIEReq);
               msg_status = AT_QUEUE_ERROR_E;
            }
            else
            {
               msg_status = AT_NO_ERROR_E;
            }
         }
      }
   }
      
   if ( AT_NO_ERROR_E != msg_status )
   {
      Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
      if ( pCmdReply )
      {
         pCmdReply->port_id = uart_port;
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
         
         /* Send the response */
         osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK != status)
         {
            /* Failed to write, free the pre-allocated pool entry */
            FREE_ATHANDLE_POOL(pCmdReply);
            msg_status = AT_QUEUE_ERROR_E;
         }
      }
   }

   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_SetZoneEnable
* description     : Command to set the zone enablement bitfield.  Called by both low
*                   and High halves of the bit field, identified by param_id.
*                   ATLOZ=<low bits>  //set low 46 bits of zone enablement - param_id = ZONE_BITS_LOW_E
*                   ATUPZ=<upper bits>  //set high 46 bits of zone enablement - param_id = ZONE_BITS_HIGH_E
*                   ATLOZ?
*                   ATUPZ?
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_SetZoneEnable(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   uint64_t zone_bits;
   ZoneEnableData_t ZoneMessage;
   char* str_end;
   char buffer[32];
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type || AT_COMMAND_WRITE_E == command_type )
      {
         if ( AT_COMMAND_WRITE_E == command_type )
         {
            strcpy(buffer,AT_RESPONSE_FAIL);
            //extract the  parameters
            zone_bits = strtoll((char*)source_buffer, &str_end, 10);
            if(!*str_end)
            {
               if ( ZONE_BITS_LOW_E == param_id )
               {
                  ZoneMessage.Word = (uint32_t)(zone_bits & 0xFFFFFFFF);
                  ZoneMessage.HalfWord = (uint32_t)((zone_bits >> 32) & 0xFFFF);
                  ZoneMessage.LowHigh = ZONE_BITS_LOW_E;
               }
               else 
               {
                  ZoneMessage.Word = (uint32_t)((zone_bits >>16) & 0xFFFFFFFF);
                  ZoneMessage.HalfWord = (uint32_t)(zone_bits & 0xFFFF);
                  ZoneMessage.LowHigh = ZONE_BITS_HIGH_E;
               }

               //The received message is for 'enabled' zones, but the local map is for 'disabled' zones so the rx'd value nust be inverted
               uint16_t half_word = ~ZoneMessage.HalfWord;
               uint32_t word = ~ZoneMessage.Word;
               //Update the zone flags
               CFG_SetSplitZoneDisabledMap(ZoneMessage.LowHigh, word, half_word);
               msg_status = AT_NO_ERROR_E;
               
               if( BASE_NCU_E == gBaseType )
               {
                  CO_Message_t* pCmdSignalReq = osPoolAlloc(AppPool);
                  if ( pCmdSignalReq )
                  {
                     ZoneMessage.Source = uart_port;
                     
                     pCmdSignalReq->Type = CO_MESSAGE_GENERATE_ZONE_ENABLE_E;
                     
                     uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
                     
                     memcpy(pCmdReq, (uint8_t*)&ZoneMessage, sizeof(ZoneEnableData_t));
                     pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(ZoneEnableData_t);

                     osStatus osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
                     if (osOK != osStat)
                     {
                        /* failed to write */
                        osPoolFree(AppPool, pCmdSignalReq);
                        msg_status = AT_QUEUE_ERROR_E;
                     }
                     else
                     {
                        msg_status = AT_NO_ERROR_E;
                        strcpy(buffer,AT_RESPONSE_OK);
                     }
                  }
                  else 
                  {
                  }
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
                  strcpy(buffer,AT_RESPONSE_OK);
               }
            }
            else 
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
         }
         else 
         {
            //Read command
            msg_status = AT_NO_ERROR_E;
            if ( ZONE_BITS_LOW_E == param_id )
            {
               ZoneMessage.LowHigh = ZONE_BITS_LOW_E;
            }
            else 
            {
               ZoneMessage.LowHigh = ZONE_BITS_HIGH_E;
            }

            CFG_GetSplitZoneDisabledMap(ZoneMessage.LowHigh , &ZoneMessage.Word, &ZoneMessage.HalfWord);
            ZoneMessage.HalfWord = ~ZoneMessage.HalfWord;
            ZoneMessage.Word = ~ZoneMessage.Word;
            if ( ZoneMessage.LowHigh )
            {
               sprintf(buffer,"0x%08x%04x", ZoneMessage.Word, ZoneMessage.HalfWord);
            }
            else
            {
               sprintf(buffer,"0x%04x%08x", ZoneMessage.HalfWord, ZoneMessage.Word);
            }
         }

         
         /* Send the command to the application */
         if ( /*(AT_NO_ERROR_E != msg_status) ||*/
              ( AT_NO_ERROR_E == msg_status ) )
         {
            Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
            if ( pCmdReply )
            {
               pCmdReply->port_id = uart_port;
               strcpy((char *)pCmdReply->data_buffer, buffer);
               pCmdReply->length = (uint8_t)strlen(buffer);
               
               /* Send the response */
               osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
               
               if (osOK != status)
               {
                  /* Failed to write, free the pre-allocated pool entry */
                  FREE_ATHANDLE_POOL(pCmdReply);
                  msg_status = AT_QUEUE_ERROR_E;
               }
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_LogVisibleDevices
* description     : Command to print details of devices seen by the synch module to the debug log.
*                   ATDVD+                            All nodes
*                   ATDVD+<last node>                 Nodes 0 to <last node>
*                   ATDVD+<first node>,<last node>    Nodes <first node> to <last node>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_LogVisibleDevices(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   
   if ( AT_COMMAND_SPECIAL_E == command_type )
   {
      if ( source_buffer )
      {
         uint32_t firstNode = 0;
         uint32_t lastNode = MAX_DEVICES_PER_SYSTEM;
         AT_Parameters_t parameters;
         
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         
         if(AT_NO_ERROR_E == msg_status)
         {
            if ( 2 == parameters.NumberOfParameters )
            {
               firstNode = parameters.Parameter[0];
               lastNode = parameters.Parameter[1];
            }
            else if ( 1 == parameters.NumberOfParameters )
            {
               lastNode = parameters.Parameter[0];
            }
            
            MC_MFH_DumpVisibleDevices(firstNode, lastNode, uart_port);
            msg_status = AT_NO_ERROR_E;
         }
      }
   }

   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
   if ( pCmdReply )
   {
      pCmdReply->port_id = uart_port;
      if ( AT_NO_ERROR_E == msg_status )
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
      }
      else 
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      }
      
      /* Send the response */
      osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         /* Failed to write, free the pre-allocated pool entry */
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ScanForDevices
* description     : Open all heartbeat slots and record heartbeats for one full long frame.
*                   ATSCAN+
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ScanForDevices(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   AT_Parameters_t parameters;
   CIECommand_t scan_msg;

   if ( AT_COMMAND_SPECIAL_E == command_type )
   {
//      if ( false == MC_MFH_ScanForDevicesInProgress() )
//      {
//         MC_MFH_ScanForDevices();
//      }
//      msg_status = AT_NO_ERROR_E;
      
      
      if ( source_buffer )
      {
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         
         if(AT_NO_ERROR_E == msg_status)
         {
            if ( 2 == parameters.NumberOfParameters )
            {
               scan_msg.Parameter2 = parameters.Parameter[0];
               scan_msg.NodeID = parameters.Parameter[1];
            }
            else
            {
               scan_msg.NodeID = gNodeAddress;
            }
            
            scan_msg.CommandType = CIE_CMD_SCAN_FOR_DEVICES_E;
            
            CO_Message_t* pCmdSignalReq = osPoolAlloc(AppPool);
            if ( pCmdSignalReq )
            {
               pCmdSignalReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               
               uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
               
               memcpy(pCmdReq, (uint8_t*)&scan_msg, sizeof(CIECommand_t));
               pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CIECommand_t);

               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCmdSignalReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
   }
   
   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
   if ( pCmdReply )
   {
      pCmdReply->port_id = uart_port;
      if ( AT_NO_ERROR_E == msg_status )
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
      }
      else 
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      }
      
      /* Send the response */
      osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         /* Failed to write, free the pre-allocated pool entry */
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_SetSnrAveraging
* description     : Get and Set the SNR and RSSI Averaging policy for received heartbeats
*                   ATSNRAV=0  Use default running average
*                   ATSNRAV=1  Restart average if last value is older than two long frames ago
*                   ATSNRAV=2  Do not average.  Use value from last heartbeat.
*                   ATSNRAV?   To read current setting
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_SetSnrAveraging(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint32_t policy;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         msg_status = AT_NO_ERROR_E;
      }
      else if ( AT_COMMAND_WRITE_E == command_type )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( SUCCESS_E == msg_status )
         {
            if ( 1 == parameters.NumberOfParameters )
            {
               policy = parameters.Parameter[0];
               //Valid settings are 0,1,2
               if ( CFG_SetSnrRssiAveragingPolicy(policy) )
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
      
      if ( AT_NO_ERROR_E == msg_status )
      {
         /* Use the common function MM_ATCommand_decimalNumber to perform the read/write to NVM */
         msg_status = MM_ATCommand_decimalNumber(command_type, source_buffer, buffer_length, uart_port, NV_AVE_POLICY_E);
      }
      else
      {
         Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
         if ( pCmdReply )
         {
            pCmdReply->port_id = uart_port;
            strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
            pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
            
            /* Send the response */
            osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
            
            if (osOK != status)
            {
               /* Failed to write, free the pre-allocated pool entry */
               FREE_ATHANDLE_POOL(pCmdReply);
               msg_status = AT_QUEUE_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}


#ifdef APPLY_RSSI_OFFSET
/*************************************************************************************/
/**
* function name   : MM_ATCommand_RssiOffset
* description     : Test aid.  Enables and offset to be added to the rssi of
*                   node IDs 0 to 9.  Primarily, this is to make the target node
*                   seem farther away than it is for parent selection tests.
*                   The offset is applied as: rx'd rssi + <rssi offset>
*                   Valid range for rssi offset is -128 to +127.
*                   ATROFF=<node id>,<rssi offset>
*                   ATROFF?<node id>   To read current setting
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_RssiOffset(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   AT_Parameters_t parameters;
   uint32_t node_id = 0;
   int8_t rssi_offset = 0;
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
   if ( pCmdReply )
   {
      if ( source_buffer )
      {
         //Read the target address, channel and bitfield
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( AT_NO_ERROR_E == msg_status )
         {
            if ( AT_COMMAND_WRITE_E == command_type)
            {
               if ( ( 2 == parameters.NumberOfParameters ) )
               {
                  node_id = parameters.Parameter[0];
                  rssi_offset = parameters.Parameter[1];
                  CFG_SetRssiOffset(node_id,rssi_offset);
                  msg_status = AT_NO_ERROR_E;
               }
               else
               {
                  msg_status = AT_INVALID_COMMAND_E;
               }
            }
            else if ( AT_COMMAND_READ_E == command_type)
            {
               if ( 1 == parameters.NumberOfParameters )
               {
                  node_id = parameters.Parameter[0];
                  rssi_offset = CFG_GetRssiOffset(node_id);
                  msg_status = AT_NO_ERROR_E;
               }
               else
               {
                  msg_status = AT_INVALID_COMMAND_E;
               }
            }
            else 
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
         }
         
      }
   }
   
   if ( AT_NO_ERROR_E == msg_status )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         sprintf((char *)pCmdReply->data_buffer, "%d", rssi_offset);
      }
      else 
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
      }
   }
   else
   {
      strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
   }

   
   pCmdReply->port_id = uart_port;
   pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
   
   /* Send the response */
   osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
   
   if (osOK != status)
   {
      /* Failed to write, free the pre-allocated pool entry */
      FREE_ATHANDLE_POOL(pCmdReply);
      msg_status = AT_QUEUE_ERROR_E;
   }
   return msg_status;
}
#endif

#ifdef ENABLE_BATTERY_VOLTAGE_COMMAND
/*************************************************************************************/
/**
* function name   : MM_ATCommand_ReadBatteryVoltages
* description     : Test aid.  Read battery voltages out to the debug log
*                   ATBV?
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ReadBatteryVoltages(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   ErrorCode_t result;
   DM_BatteryReadings_t batReadings = {0};
   
   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
   if ( pCmdReply )
   {
       msg_status = AT_INVALID_COMMAND_E;
      if ( AT_COMMAND_READ_E == command_type)
      {
         result = DM_BAT_ReadBatteryVoltages(&batReadings);
         if ( SUCCESS_E == result )
         {
            msg_status = AT_NO_ERROR_E;
         }
         else 
         {
            msg_status = AT_COMMAND_UNKNOWN_ERROR_E;
         }
      }
      else 
      {
         msg_status = AT_INVALID_COMMAND_E;
      }
   }
   
   
   if ( AT_NO_ERROR_E == msg_status )
   {
      //Output the readings
      CO_PRINT_A_1(DBG_INFO_E,"ADC Ref mV=%d\r\n", batReadings.AdcReference);
      CO_PRINT_A_2(DBG_INFO_E,"PRIM adc=%d, mV=%d\r\n", batReadings.PrimaryRawADC, batReadings.PrimaryVoltage);
      if ( BASE_CONSTRUCTION_E == gBaseType )
      {
         CO_PRINT_A_3(DBG_INFO_E,"BKP Raw ADC cell_1=%d, cell_2=%d, total=%d\r\n", batReadings.BackupCell1RawADC, batReadings.BackupCell2RawADC, batReadings.BackupRawADC);
         CO_PRINT_A_3(DBG_INFO_E,"BKP mV cell_1=%d, cell_2=%d, total=%d\r\n", batReadings.BackupCell1Voltage, batReadings.BackupCell2Voltage, batReadings.BackupVoltage);
      }
      else
      {
         CO_PRINT_A_2(DBG_INFO_E,"BKP adc=%d, mV=%d\r\n", batReadings.BackupRawADC, batReadings.BackupVoltage);
      }
      
      strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
   }
   else
   {
      strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
   }

   
   pCmdReply->port_id = uart_port;
   pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
   
   /* Send the response */
   osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
   
   if (osOK != status)
   {
      /* Failed to write, free the pre-allocated pool entry */
      FREE_ATHANDLE_POOL(pCmdReply);
      msg_status = AT_QUEUE_ERROR_E;
   }
   return msg_status;
}
#endif


/*************************************************************************************/
/**
* function name   : MM_ATCommand_DisablePolling
* description     : Disables the GPIO polling for MCPs and tamper switches for production test.
*                   ATDPOLL=0|1  enable(0) or disable(1) polling.
*                   ATDPOLL?     Read current setting
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_DisablePolling(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   bool disabled = false;
   char response[32] = {0};
   
   if ( source_buffer )
   {
      msg_status = AT_INVALID_COMMAND_E;
      
      if ( AT_COMMAND_READ_E == command_type )
      {
         if ( CFG_GetPollingDisabled() )
         {
            disabled = true;
         }
         sprintf(response, "%d", (int32_t)disabled);
         msg_status = AT_NO_ERROR_E;
      }
      else
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( SUCCESS_E == msg_status )
         {
            msg_status = AT_INVALID_COMMAND_E;
            if ( 1 == parameters.NumberOfParameters )
            {
               disabled = (bool)parameters.Parameter[0];
               
               CFG_SetPollingDisabled(disabled);
               
               //Throw the semaphore to activate the 'tamper enable' immediately
               osSemaphoreRelease(GpioSemId);
               
               sprintf(response, "%s", AT_RESPONSE_OK);
               msg_status = AT_NO_ERROR_E;
            }
         }
      }
   }

   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
   if ( pCmdReply )
   {
      pCmdReply->port_id = uart_port;
      if ( AT_NO_ERROR_E == msg_status )
      {
         strcpy((char *)pCmdReply->data_buffer, response);
         pCmdReply->length = (uint8_t)strlen(response);
      }
      else
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      }
      
      /* Send the response */
      osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         /* Failed to write, free the pre-allocated pool entry */
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
   }

   return msg_status;
}


/*************************************************************************************/
/**
* function name   : MM_ATCommand_SendQueueCount
* description     : Read the number of messages in the AckMgr RACHS queue and DULCH queue.
*                   ATSQC?
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_SendQueueCount(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   char response[64] = {0};
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_READ_E == command_type )
      {
         msg_status = AT_NO_ERROR_E;
         uint32_t dulch_queue_count = MC_SACH_MessageCount();
         uint32_t ackmgr_queue_count = MC_ACK_GetQueueCount(ACK_RACHS_E);
         uint32_t rachp_queue_count = MC_MACQ_MessageCount(MAC_RACHP_Q_E);
         uint32_t rachs_queue_count = MC_MACQ_MessageCount(MAC_RACHS_Q_E);
         sprintf(response,"DULCHQ=%d, ACKMGRQ=%d, RACHP=%d, RACHS=%d", dulch_queue_count, ackmgr_queue_count, rachp_queue_count, rachs_queue_count);
      }
   }

   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
   if ( pCmdReply )
   {
      pCmdReply->port_id = uart_port;
      if ( AT_NO_ERROR_E == msg_status )
      {
         strcpy((char *)pCmdReply->data_buffer, response);
         pCmdReply->length = (uint8_t)strlen(response);
      }
      else
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      }
      
      /* Send the response */
      osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         /* Failed to write, free the pre-allocated pool entry */
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_SendFaultMessage
* description     : Generate a fault message with the supplied type and value (0|1).
*                   Set the optional 'dulch' parameter to 1 to send via DULCH.
*                   ATSNDF=<channel>,<fault type>,<value>[,<dulch>]
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_SendFaultMessage( const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param )
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint32_t channel = 0;
   uint32_t fault_type = 0;
   uint32_t value = 0;
   bool send_on_dulch = false;
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_WRITE_E == command_type )
      {
         //extract the  parameters
         if ( SUCCESS_E == MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters) )
         {
            if ( 3 <= parameters.NumberOfParameters )
            {
               channel = parameters.Parameter[0];
               if ( CO_CHANNEL_MAX_E > channel )
               {
                  fault_type = parameters.Parameter[1];
                  if ( FAULT_SIGNAL_MAX_E > fault_type )
                  {
                     value = parameters.Parameter[2];
                     if ( 3 < parameters.NumberOfParameters )
                     {
                        if( 0 != parameters.Parameter[3] )
                        {
                           send_on_dulch = true;
                        }
                        CO_SendFaultSignal(BIT_SOURCE_NONE_E, (CO_ChannelIndex_t)channel, (FaultSignalType_t)fault_type, (uint16_t)value, send_on_dulch, true);
                        msg_status = AT_NO_ERROR_E;
                     }
                  }
               }
            }
         }
      }
   }

   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
   if ( pCmdReply )
   {
      pCmdReply->port_id = uart_port;
      if ( AT_NO_ERROR_E == msg_status )
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
      }
      else
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      }
      
      /* Send the response */
      osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         /* Failed to write, free the pre-allocated pool entry */
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_RequestSensorValues
* description     : Broadcast a Sensor values request command to all radio boards on the mesh.
*                   ATSENV+
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_RequestSensorValues(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;

   if ( AT_COMMAND_SPECIAL_E == command_type )
   {
      msg_status = AT_NO_ERROR_E;
      
      CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
      if ( pCIEReq )
      {
         pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
         
         CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
         pCommand->CommandType = CIE_CMD_REQUEST_SENSOR_VALUES_E;
         pCommand->PortNumber = (uint8_t)uart_port;
         pCommand->NodeID = ADDRESS_GLOBAL;
         pCommand->Parameter1 = 0;
         pCommand->Parameter2 = (uint8_t)ZONE_GLOBAL;
         pCommand->Value = 0;
         pCommand->Zone = ZONE_GLOBAL;
         pCommand->ReadWrite = 1;

         osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCIEReq);
            msg_status = AT_QUEUE_ERROR_E;
         }
         else
         {
            msg_status = AT_NO_ERROR_E;
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_TxAtCommand
* description     : Broadcast a Sensor values request command to all radio boards on the mesh.
*                   ATATTX+<destination>,<AT command>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_TxAtCommand(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   uint32_t destination;
   uint32_t command_length = 0;
   AtMessageData_t command_data;
   
   CO_PRINT_B_1(DBG_INFO_E,"sizeof AtMessageData_t = %d\r\n", sizeof(AtMessageData_t));
   
   if ( AT_COMMAND_SPECIAL_E == command_type )
   {
      msg_status = AT_INVALID_COMMAND_E;
      if ( source_buffer )
      {
         //The command has two fileds <destination>,<AT command>
         //If we find the ',' and replace it with a NULL, the rest of the buffer
         //is the AT command to be transmitted.  source_buffer will then only contain the destination
         char* pATCommand = strchr((char*)source_buffer, ',');
         if ( pATCommand )
         {
            *pATCommand = 0;
            //step over the NULL to the AT command
            pATCommand++;
            command_length = strlen(pATCommand);
            if ( 0 < command_length )
            {
               if ( MAX_OTA_AT_COMMAND_LENGTH > command_length )
               {
                  strcpy( command_data.Command, pATCommand );
                  command_data.Source = gNodeAddress;
                  command_data.IsCommand = 1;
                  
                  //source_buffer only has the destination left in it.  search for '-' to find
                  //out if it's a serial number or a node ID
                  pATCommand = strchr((char*)source_buffer,'-');
                  if ( pATCommand )
                  {
                     //it's a serial number.  Encode it into a uint32_t.
                     if ( MM_ATEncodeSerialNumber((char*)source_buffer, &destination) )
                     {
                        command_data.Destination = destination;
                        msg_status = AT_NO_ERROR_E;
                     }
                  }
                  else 
                  {
                     destination = atoi((char*)source_buffer);
                     command_data.Destination = destination;
                     msg_status = AT_NO_ERROR_E;
                  }
               }
            }
         }
      }
      
      if ( AT_NO_ERROR_E == msg_status )
      {
         //send the command to the Application
         CO_Message_t* pMessage = osPoolAlloc(AppPool);
         if ( pMessage )
         {
            pMessage->Type = CO_MESSAGE_GENERATE_ATTX_COMMAND_E;
            
            char* pDataBuffer = (char*)pMessage->Payload.PhyDataReq.Data;
            memcpy(pDataBuffer, &command_data, sizeof(AtMessageData_t));
            
            osStatus osStat = osMessagePut(AppQ, (uint32_t)pMessage, 0);
            if (osOK != osStat)
            {
               /* failed to write */
               osPoolFree(AppPool, pMessage);
               msg_status = AT_QUEUE_ERROR_E;
            }
            else
            {
               msg_status = AT_NO_ERROR_E;
            }
         }
      }
   }

   //Send 'OK' or 'ERROR' to the requesting port
   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
   if ( pCmdReply )
   {
      pCmdReply->port_id = uart_port;
      if ( AT_NO_ERROR_E == msg_status )
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
      }
      else
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      }
      
      /* Send the response */
      osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
            /* Failed to write, free the pre-allocated pool entry */
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
   }


   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_StartAtMode
* description     : Send a message to put a device in OTA AT mode (ATSAT) 
*                    or PPU programming mode (ATPPU).
*                   ATSAT+<serial number>
*                   ATSAT+[<node_id>][,<system_id>]
*                   ATPPU+<serial number>
*                   ATPPU+[<node_id>][,<system_id>]
*                   ATPPU+<destination>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_StartAtMode(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   uint32_t destination;
   OtaMode_t mode_data;
   AT_Parameters_t parameters;
   char* pBuffer = (char*)source_buffer;
   
   if ( pBuffer )
   {
      if ( AT_COMMAND_SPECIAL_E == command_type )
      {
   
         msg_status = AT_INVALID_COMMAND_E;
         if ( pBuffer )
         {
            //The command has one field, <destination>
            //The <destination> field can be a node ID or a serial number in the form 0000-00-00000
            
            //Check for '-' in the 5th character.  If it is '-' then we have a serial number
            if ( '-' == pBuffer[4] )
            {
               //we have a serial number.  null terminate the string at the standard serial number length
               pBuffer[13] = 0;
               if ( MM_ATEncodeSerialNumber((char*)source_buffer, &destination) )
               {
                  mode_data.Destination = destination;
                     mode_data.SystemID = MC_GetSystemID();
                     mode_data.MessageSize = sizeof(AtMessageData_t);
                     msg_status = AT_NO_ERROR_E;
               }
            }
            else 
            {
               //the address is a node ID.  Extract the parameters
               if ( SUCCESS_E == MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters) )
               {
                  if ( 3 == parameters.NumberOfParameters )
                  {
                     //parameters.Parameter[0] holds the zone if address format was ZzUu
                     mode_data.Destination = parameters.Parameter[1];
                     mode_data.SystemID = parameters.Parameter[2];
                     mode_data.MessageSize = sizeof(AtMessageData_t);
                     msg_status = AT_NO_ERROR_E;
                  }
                  else if ( 2 == parameters.NumberOfParameters )
                  {
                     mode_data.Destination = parameters.Parameter[0];
                     mode_data.SystemID = parameters.Parameter[1];
                     mode_data.MessageSize = sizeof(AtMessageData_t);
                     msg_status = AT_NO_ERROR_E;
                  }
                  else if ( 1 == parameters.NumberOfParameters )
                  {
                     mode_data.Destination = parameters.Parameter[0];
                     mode_data.SystemID = MC_GetSystemID();
                     mode_data.MessageSize = sizeof(AtMessageData_t);
                     msg_status = AT_NO_ERROR_E;
                  }
                  else if ( 0 == parameters.NumberOfParameters )
                  {
                     mode_data.Destination = MC_GetNetworkAddress();
                     mode_data.SystemID = MC_GetSystemID();
                     mode_data.MessageSize = sizeof(AtMessageData_t);
                     msg_status = AT_NO_ERROR_E;
                  }
               }
            }
         }
         
         if ( AT_NO_ERROR_E == msg_status )
         {
            //send the command to the Application
            CO_Message_t* pMessage = osPoolAlloc(AppPool);
            if ( pMessage )
            {
               pMessage->Type = CO_MESSAGE_GENERATE_START_OTA_AT_MODE_E;
               
               if ( 0 == option_param )
               {
                  mode_data.FrameType = AT_FRAME_TYPE_ENTER_AT_MODE_E;
               }
               else 
               {
                  mode_data.FrameType = AT_FRAME_TYPE_ENTER_PPU_MODE_E;
               }
               
               char* pDataBuffer = (char*)pMessage->Payload.PhyDataReq.Data;
               memcpy(pDataBuffer, &mode_data, sizeof(OtaMode_t));
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pMessage, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pMessage);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
   }

   //Send 'OK' or 'ERROR' to the requesting port
   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
   if ( pCmdReply )
   {
      pCmdReply->port_id = uart_port;
      if ( AT_NO_ERROR_E == msg_status )
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
      }
      else
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      }
      
      /* Send the response */
      osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         /* Failed to write, free the pre-allocated pool entry */
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
   }


   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_StartPpuMode
* description     : Send a message to put a device into PPU Disconnected mode.
*                   ATPPUMD+[<node_id>]
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_StartPpuMode(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   uint32_t destination;
   uint32_t zone;
   AT_Parameters_t parameters;
   char* pBuffer = (char*)source_buffer;

   if ( pBuffer )
   {
      if ( AT_COMMAND_SPECIAL_E == command_type )
      {
   
         msg_status = AT_INVALID_COMMAND_E;
         if ( pBuffer )
         {
               //the address is a node ID.  Extract the parameters
               if ( SUCCESS_E == MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters) )
               {
                  if ( 2 == parameters.NumberOfParameters )
                  {
                     zone = parameters.Parameter[0];
                     destination = parameters.Parameter[1];
                     msg_status = AT_NO_ERROR_E;
                  }
                  else if ( 1 == parameters.NumberOfParameters )
                  {
                     zone = ZONE_GLOBAL;
                     destination = parameters.Parameter[0];
                     msg_status = AT_NO_ERROR_E;
                  }
                  else if ( 0 == parameters.NumberOfParameters )
                  {
                     destination = ADDRESS_GLOBAL;
                     zone = ZONE_GLOBAL;
                     msg_status = AT_NO_ERROR_E;
                  }
               }
         }
         
         /* Send the command to the application */
         if ( AT_NO_ERROR_E == msg_status )
         {
            msg_status = AT_NO_RESOURCE_E;
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_ENTER_PPU_MODE_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = destination;
               pCommand->Parameter1 = 0;
               pCommand->Parameter2 = zone;
               pCommand->Value = 0;
               pCommand->Zone = zone;
               pCommand->ReadWrite = 1;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
   }

   return msg_status;
}


/*************************************************************************************/
/**
* function name   : MM_ATCommand_ExitTestMode
* description     : Send a message to exit the RBU device from the test mode mode.
*                   ATETEST+[<node_id>]
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ExitTestMode(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   uint32_t destination;
   uint32_t zone;
   AT_Parameters_t parameters;
   char* pBuffer = (char*)source_buffer;
	
   if ( pBuffer )
   {
      if ( AT_COMMAND_SPECIAL_E == command_type )
      {
   
         msg_status = AT_INVALID_COMMAND_E;
         if ( pBuffer )
         {
               //the address is a node ID.  Extract the parameters
               if ( SUCCESS_E == MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters) )
               {
                  if ( 2 == parameters.NumberOfParameters )
                  {
                     zone = parameters.Parameter[0];
                     destination = parameters.Parameter[1];
                     msg_status = AT_NO_ERROR_E;
                  }
                  else if ( 1 == parameters.NumberOfParameters )
                  {
                     zone = ZONE_GLOBAL;
                     destination = parameters.Parameter[0];
                     msg_status = AT_NO_ERROR_E;
                  }
                  else if ( 0 == parameters.NumberOfParameters )
                  {
                     destination = ADDRESS_GLOBAL;
                     zone = ZONE_GLOBAL;
                     msg_status = AT_NO_ERROR_E;
                  }
               }
         }
         
         /* Send the command to the application */
         if ( AT_NO_ERROR_E == msg_status )
         {
            msg_status = AT_NO_RESOURCE_E;
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_EXIT_TEST_MODE_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = destination;
               pCommand->Parameter1 = 0;
               pCommand->Parameter2 = zone;
               pCommand->Value = 0;
               pCommand->Zone = zone;
               pCommand->ReadWrite = 1;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
   }

   return msg_status;
}
         
         
         
         
         //         if ( AT_NO_ERROR_E == msg_status )
//         {
//            //send the command to the Application
//            CO_Message_t* pMessage = osPoolAlloc(AppPool);
//            if ( pMessage )
//            {
//               pMessage->Type = CO_MESSAGE_GENERATE_START_OTA_AT_MODE_E;
//               
//               if ( 0 == option_param )
//               {
//                  mode_data.FrameType = AT_FRAME_TYPE_ENTER_AT_MODE_E;
//               }
//               else 
//               {
//                  mode_data.FrameType = AT_FRAME_TYPE_ENTER_PPU_MODE_E;
//               }
//               
//               char* pDataBuffer = (char*)pMessage->Payload.PhyDataReq.Data;
//               memcpy(pDataBuffer, &mode_data, sizeof(OtaMode_t));
//               
//               osStatus osStat = osMessagePut(AppQ, (uint32_t)pMessage, 0);
//               if (osOK != osStat)
//               {
//                  /* failed to write */
//                  osPoolFree(AppPool, pMessage);
//                  msg_status = AT_QUEUE_ERROR_E;
//               }
//               else
//               {
//                  msg_status = AT_NO_ERROR_E;
//               }
//            }
//         }
//      }
//   }

//   //Send 'OK' or 'ERROR' to the requesting port
//   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
//   if ( pCmdReply )
//   {
//      pCmdReply->port_id = uart_port;
//      if ( AT_NO_ERROR_E == msg_status )
//      {
//         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
//         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
//      }
//      else
//      {
//         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
//         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
//      }
//      
//      /* Send the response */
//      osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
//      
//      if (osOK != status)
//      {
//         /* Failed to write, free the pre-allocated pool entry */
//         FREE_ATHANDLE_POOL(pCmdReply);
//         msg_status = AT_QUEUE_ERROR_E;
//      }
//   }


//   return msg_status;
//}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_DelayedOutputPulse
* description     : Command to start the delayed output test function.
*                   ATDLOUT+[ZxUx,]<initial delay>,<output duration>,<channel>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_DelayedOutputPulse(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint16_t unit_address = 0;
   uint16_t zone = 0;
   uint32_t initial_delay = 0;
   int32_t output_duration = 0;
   int32_t channel = 0;
   
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_SPECIAL_E == command_type )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( AT_NO_ERROR_E == msg_status )
         {
            msg_status = AT_INVALID_PARAMETER_E;
            if ( 5 == parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               unit_address = parameters.Parameter[1];
               initial_delay = parameters.Parameter[2];
               output_duration = parameters.Parameter[3];
               channel = parameters.Parameter[4];
               
               msg_status = AT_NO_ERROR_E;
            }
            else if ( 3 == parameters.NumberOfParameters )
            {
               zone = ZONE_GLOBAL;
               unit_address = gNodeAddress;
               initial_delay = parameters.Parameter[0];
               output_duration = parameters.Parameter[1];
               channel = parameters.Parameter[2];
               
               msg_status = AT_NO_ERROR_E;
            }
         }
      }
      
      if ( AT_NO_ERROR_E == msg_status )
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
            CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
            pCommand->CommandType = CIE_CMD_DELAYED_OUTPUT_E;
            pCommand->PortNumber = (uint8_t)uart_port;
            pCommand->NodeID = unit_address;
            pCommand->Parameter1 = channel;
            pCommand->Parameter2 = zone;
            pCommand->Value = (initial_delay << 16) + (output_duration & 0xFFF);
            pCommand->Zone = zone;
            pCommand->ReadWrite = 1;
         
            osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
            if (osOK != osStat)
            {
               /* failed to write */
               osPoolFree(AppPool, pCIEReq);
               msg_status = AT_QUEUE_ERROR_E;
            }
            else
            {
               msg_status = AT_NO_ERROR_E;
            }
         }
      }
   }
      
   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
   if ( pCmdReply )
   {
      pCmdReply->port_id = uart_port;
      if ( AT_NO_ERROR_E == msg_status )
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
      }
      else 
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      }
      
      /* Send the response */
      osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         /* Failed to write, free the pre-allocated pool entry */
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
   }

   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_BatteryTest
* description     : Command to initiate an immediate battery test.
*                   ATBTST+[ZxUx,]
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_BatteryTest(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint16_t unit_address = 0;
   uint16_t zone = 0;
   
   
   if ( source_buffer )
   {
      if ( AT_COMMAND_SPECIAL_E == command_type )
      {
         //extract the  parameters
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( AT_NO_ERROR_E == msg_status )
         {
            msg_status = AT_INVALID_PARAMETER_E;
            if ( 2 == parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               unit_address = parameters.Parameter[1];
               
               msg_status = AT_NO_ERROR_E;
            }
            else
            {
               zone = ZONE_GLOBAL;
               unit_address = gNodeAddress;
               
               msg_status = AT_NO_ERROR_E;
            }
         }
      }
      
      if ( AT_NO_ERROR_E == msg_status )
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
            CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
            pCommand->CommandType = CIE_CMD_BATTERY_TEST_E;
            pCommand->PortNumber = (uint8_t)uart_port;
            pCommand->NodeID = unit_address;
            pCommand->Parameter1 = 0;
            pCommand->Parameter2 = zone;
            pCommand->Value = 0;
            pCommand->Zone = zone;
            pCommand->ReadWrite = 1;
         
            osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
            if (osOK != osStat)
            {
               /* failed to write */
               osPoolFree(AppPool, pCIEReq);
               msg_status = AT_QUEUE_ERROR_E;
            }
            else
            {
               msg_status = AT_NO_ERROR_E;
            }
         }
      }
   }
      
   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
   if ( pCmdReply )
   {
      pCmdReply->port_id = uart_port;
      if ( AT_NO_ERROR_E == msg_status )
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
      }
      else 
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      }
      
      /* Send the response */
      osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         /* Failed to write, free the pre-allocated pool entry */
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
   }

   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_ProductCode
* description     : Command to read/write the device product code.
*                   ATSERPX=[ZxUx,]<product code>
*                   ATSERPX?[ZxUx]
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_ProductCode(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   uint16_t unit_address;
   uint16_t zone;
   uint8_t read_write;
   uint32_t product_code = 0;
   char* pCode;
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   CO_PRINT_B_1(DBG_INFO_E,"MM_ATCommand_ProductCode buffer=%s\r\n", source_buffer);
   
   if ( source_buffer )
   {
      //extract the unit address
      if ( ('Z' == source_buffer[0]) && (false == IsAlpha(source_buffer[1])) )
      {
          msg_status = MM_ATCommand_ExtractUnitAddressAndZone(source_buffer, &unit_address, &zone);
         if ( AT_NO_ERROR_E == msg_status )
         {
            if ( AT_COMMAND_WRITE_E == command_type )
            {
               pCode = strchr((char*)source_buffer, ',');
               if ( pCode )
               {
                  pCode++;
               }
               else 
               {
                  msg_status = AT_INVALID_PARAMETER_E;
               }
            }
         }
      }
      else 
      {
         //There was no address.  Assume local device.
         unit_address = gNodeAddress;
         zone = ZONE_GLOBAL;
         pCode = (char*)source_buffer;
         msg_status = AT_NO_ERROR_E;
      }
      
      if ( AT_NO_ERROR_E == msg_status )
      {
         if ( AT_COMMAND_WRITE_E == command_type )
         {
            CO_PRINT_B_0(DBG_INFO_E,"MM_ATCommand_ProductCode COMMAND_WRITE\r\n");
            read_write = COMMAND_WRITE;
            //There should be a product code string in source buffer
            if( MM_ATEncodeProductCode(pCode, &product_code) )
            {
               msg_status = AT_NO_ERROR_E;
            }
            else 
            {
               msg_status = AT_INVALID_PARAMETER_E;
            }
         }
         else 
         {
            CO_PRINT_B_0(DBG_INFO_E,"MM_ATCommand_ProductCode COMMAND_READ\r\n");
            read_write = COMMAND_READ;
            
            msg_status = AT_NO_ERROR_E;
         }
      }
      
      /* Send the command to the application */
      if ( AT_NO_ERROR_E == msg_status )
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
            CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
            pCommand->CommandType = CIE_CMD_PRODUCT_CODE_E;
            pCommand->PortNumber = (uint8_t)uart_port;
            pCommand->NodeID = unit_address;
            pCommand->Parameter1 = 0;
            pCommand->Parameter2 = ZONE_GLOBAL;
            pCommand->Value = product_code;
            pCommand->Zone = ZONE_GLOBAL;
            pCommand->ReadWrite = read_write;
            
            osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
            if (osOK != osStat)
            {
               /* failed to write */
               osPoolFree(AppPool, pCIEReq);
               msg_status = AT_QUEUE_ERROR_E;
            }
            else
            {
               msg_status = AT_NO_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_PPUModeEnable
* description     : Set the PPU mode enable flag.
*                   0 = PPU mode disable, 1 = PPU mode enable
*                   ATPPEN?[ZxUx]
*                   ATPPEN=[ZxUx,]<value>
*                   
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_PPUModeEnable(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   AT_Parameters_t parameters;
   uint32_t device = 0;
   uint32_t zone = 0;
   uint32_t readWrite = 0;
   uint8_t enable_disable = 0;
   
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   if ( source_buffer )
   {
      //Read the target address, channel and bitfield
      msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
      if ( AT_NO_ERROR_E == msg_status )
      {
         if ( AT_COMMAND_WRITE_E == command_type)
         {
            readWrite = COMMAND_WRITE;
            
            if ( 3 == parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               device = parameters.Parameter[1];
               enable_disable = parameters.Parameter[2];
            }
            else if ( 1 == parameters.NumberOfParameters )
            {
               zone = ZONE_GLOBAL;
               device = gNodeAddress;
               enable_disable = parameters.Parameter[0];
            }
         }
         else 
         {
            readWrite = COMMAND_READ;
            
            if ( 2 == parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               device = parameters.Parameter[1];
            }
            else if ( 0 == parameters.NumberOfParameters )
            {
               zone = ZONE_GLOBAL;
               device = gNodeAddress;
            }
         }
      }
   
      //Filter out read of global address
      if ( (ADDRESS_GLOBAL == device) && (COMMAND_READ == readWrite) )
      {
         msg_status = AT_INVALID_PARAMETER_E;
      }
      
      /* Send the command to the application */
      if ( AT_NO_ERROR_E == msg_status )
      {
         CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
         if ( pCIEReq )
         {
            pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
            CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
            pCommand->CommandType = CIE_CMD_PPU_MODE_ENABLE_E;
            pCommand->PortNumber = (uint8_t)uart_port;
            pCommand->NodeID = device;
            pCommand->Parameter1 = 0;
            pCommand->Parameter2 = zone;
            pCommand->Value = enable_disable;
            pCommand->Zone = zone;
            pCommand->ReadWrite = readWrite;
            
            osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
            if (osOK != osStat)
            {
               /* failed to write */
               osPoolFree(AppPool, pCIEReq);
               msg_status = AT_QUEUE_ERROR_E;
            }
            else
            {
               msg_status = AT_NO_ERROR_E;
            }
         }
      }
   }
   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_PassivationSettings
* description     : Combined Passivation settings.
*                   ATDPAS=[ZxUx,]<voltage>,<timeout>,<on time>,<off time>
*                   ATDPAS?[ZxUx]
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_PassivationSettings(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   DePassivationSettings_t settings = {0};
   uint32_t compound_settings = 0;
   uint32_t readWrite = 0;
   
   
   if ( source_buffer )
   {
      //extract the  parameters
      msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
      if ( AT_NO_ERROR_E == msg_status )
      {
         if ( AT_COMMAND_WRITE_E == command_type )
         {
            readWrite = COMMAND_WRITE;
            msg_status = AT_INVALID_PARAMETER_E;
            if ( 6 == parameters.NumberOfParameters )
            {
               settings.Zone = parameters.Parameter[0];
               settings.UnitAddress = parameters.Parameter[1];
               settings.Voltage = parameters.Parameter[2];
               settings.Timeout = parameters.Parameter[3];
               settings.OnTime = parameters.Parameter[4];
               settings.OffTime = parameters.Parameter[5];
               msg_status = AT_NO_ERROR_E;
            }
            else if ( 4 == parameters.NumberOfParameters )
            {
               settings.Zone = ZONE_GLOBAL;
               settings.UnitAddress = gNodeAddress;
               settings.Voltage = parameters.Parameter[0];
               settings.Timeout = parameters.Parameter[1];
               settings.OnTime = parameters.Parameter[2];
               settings.OffTime = parameters.Parameter[3];
               msg_status = AT_NO_ERROR_E;
            }
         }
         else 
         {
            readWrite = COMMAND_READ;
            if ( 2 == parameters.NumberOfParameters )
            {
               settings.Zone = parameters.Parameter[0];
               settings.UnitAddress = parameters.Parameter[1];
            }
            else 
            {
               settings.Zone = ZONE_GLOBAL;
               settings.UnitAddress = gNodeAddress;
            }
         }
      }
      
      
      if ( AT_NO_ERROR_E == msg_status )
      {
         if ( SUCCESS_E == CFG_CompressDepassivationSettings(&settings, &compound_settings) )
         {
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
               
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_DEPASSIVATION_SETTINGS_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->NodeID = settings.UnitAddress;
               pCommand->Parameter1 = 0;
               pCommand->Parameter2 = settings.Zone;
               pCommand->Value = compound_settings;
               pCommand->Zone = settings.Zone;
               pCommand->ReadWrite = readWrite;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
         else 
         {
            msg_status = AT_OPERATION_FAIL_E;
         }
      }
   }
      
   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
   if ( pCmdReply )
   {
      pCmdReply->port_id = uart_port;
      if ( AT_NO_ERROR_E == msg_status )
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
      }
      else 
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      }
      
      /* Send the response */
      osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         /* Failed to write, free the pre-allocated pool entry */
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
   }

   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_GetMcuUid
* description     : Read the MCU ID.
*                   ATUID?
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_GetMcuUid(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   uint32_t word0 = 0;
   uint32_t word1 = 0;
   uint32_t word2 = 0;
   char response[32];

   if ( AT_COMMAND_READ_E == command_type )
   {
      word0 = HAL_GetUIDw0();
      word1 = HAL_GetUIDw1();
      word2 = HAL_GetUIDw2();
      
      sprintf( response, "%08X%08X%08X", word0, word1, word2);
      msg_status = AT_NO_ERROR_E;
   }

   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
   if ( pCmdReply )
   {
      pCmdReply->port_id = uart_port;
      if ( AT_NO_ERROR_E == msg_status )
      {
         strcpy((char *)pCmdReply->data_buffer, response);
         pCmdReply->length = (uint8_t)strlen(response);
      }
      else 
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      }
      
      /* Send the response */
      osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         /* Failed to write, free the pre-allocated pool entry */
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
   }

   return msg_status;
}



/*************************************************************************************/
/**
* function name   : MM_ATCommand_SendAnnouncement
* description     : Send a PPU Announcement message.
*                   ATANN+
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_SendAnnouncement(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t msg_status = AT_INVALID_COMMAND_E;
   
   if ( AT_COMMAND_SPECIAL_E == command_type )
   {
      msg_status = AT_OPERATION_FAIL_E;
      //The announcement won't go unless the PPU mode is 'PPU Disconnected'
      PpuState_t preserved_state = MC_GetPpuMode();
      MC_SetPpuMode(PPU_STATE_DISCONNECTED_E);
      if (MM_PPU_SendPpuPeriodicAnnouncement() )
      {
         msg_status = AT_NO_ERROR_E;
      }
      MC_SetPpuMode(preserved_state);
   }

   //Send 'OK' or 'ERROR' to the requesting port
   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
   if ( pCmdReply )
   {
      pCmdReply->port_id = uart_port;
      if ( AT_NO_ERROR_E == msg_status )
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
      }
      else
      {
         strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
         pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
      }
      
      /* Send the response */
      osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
      
      if (osOK != status)
      {
         /* Failed to write, free the pre-allocated pool entry */
         FREE_ATHANDLE_POOL(pCmdReply);
         msg_status = AT_QUEUE_ERROR_E;
      }
   }


   return msg_status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_FaultsClearEnable
* description     : Enable or disable the 'faults clear' message that is sent on joining
*                   if no faults are present.
*                   ATFCM=[ZxUx,]<0|1>   0=disabled, 1=enabled
*                   ATFCM?[ZxUx]
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_FaultsClearEnable(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   uint16_t unit_address;
   uint8_t zone;
   uint32_t enable = 0;
   uint32_t readWrite = 0;
   
   if ( source_buffer )
   {
      //extract the  parameters
      msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
      if ( AT_NO_ERROR_E == msg_status )
      {
         if ( AT_COMMAND_READ_E == command_type )
         {
            if ( 3 == parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               unit_address = parameters.Parameter[1];
               enable = parameters.Parameter[2];
               msg_status = AT_NO_ERROR_E;
            }
            else if ( 2 == parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               unit_address = parameters.Parameter[1];
               msg_status = AT_NO_ERROR_E;
            }
            else 
            {
               zone = ZONE_GLOBAL;
               unit_address = gNodeAddress;
               msg_status = AT_NO_ERROR_E;
            }
         }
         else if ( AT_COMMAND_WRITE_E == command_type )
         {
           readWrite = 1;
            if ( 3 == parameters.NumberOfParameters )
            {
               zone = parameters.Parameter[0];
               unit_address = parameters.Parameter[1];
               enable = parameters.Parameter[2];
               msg_status = AT_NO_ERROR_E;
            }
            else if ( 1 == parameters.NumberOfParameters )
            {
               zone = ZONE_GLOBAL;
               unit_address = gNodeAddress;
               enable = parameters.Parameter[0];
               msg_status = AT_NO_ERROR_E;
            }
         }

         if( AT_NO_ERROR_E == msg_status )
         {
            CO_Message_t* pCIEReq = osPoolAlloc(AppPool);
            if ( pCIEReq )
            {
               pCIEReq->Type = CO_MESSAGE_CIE_COMMAND_E;
            
               CIECommand_t* pCommand = (CIECommand_t*)pCIEReq->Payload.PhyDataReq.Data;
               pCommand->CommandType = CIE_CMD_ENABLE_FAULTS_CLEAR_MSG_E;
               pCommand->PortNumber = (uint8_t)uart_port;
               pCommand->Parameter1 = DEV_ALL_DEVICES;
               pCommand->Parameter2 = zone;
               pCommand->ReadWrite = readWrite;
               pCommand->NodeID = unit_address;
               pCommand->Value = enable;
               pCommand->Zone = zone;
               
               osStatus osStat = osMessagePut(AppQ, (uint32_t)pCIEReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCIEReq);
                  msg_status = AT_QUEUE_ERROR_E;
               }
               else
               {
                  msg_status = AT_NO_ERROR_E;
               }
            }
         }
      }
   }
   return msg_status;
}



#ifdef ENABLE_NCU_CACHE_DEBUG
/*************************************************************************************/
/**
* function name   : MM_ATCommand_NcuStatusMessage
* description     : Test aid.  Enables dummy status messages to be passed to the NCU cache.
*                   ATSTM=<node id>,<prim parent>,<sec parent>,<rank>,<event>,<data>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_NcuStatusMessage(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   AT_Parameters_t parameters;
   StatusIndication_t statusInd;
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   
   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
   if ( pCmdReply )
   {
      if ( source_buffer )
      {
         //Read the target address, channel and bitfield
         msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
         if ( AT_NO_ERROR_E == msg_status )
         {
            if ( AT_COMMAND_WRITE_E == command_type)
            {
               if ( 6 == parameters.NumberOfParameters )
               {
                  statusInd.SourceNode = parameters.Parameter[0];
                  statusInd.PrimaryParent = parameters.Parameter[1];
                  statusInd.SecondaryParent = parameters.Parameter[2];
                  statusInd.Rank = parameters.Parameter[3];
                  statusInd.Event = parameters.Parameter[4];
                  statusInd.EventData = parameters.Parameter[5];
                  statusInd.OverallFault = 0;
                  statusInd.AverageSNRPrimaryParent = 20;
                  if ( -1 != parameters.Parameter[2] )
                  {
                     statusInd.AverageSNRSecondaryParent = 20;
                  }
                  else 
                  {
                     statusInd.AverageSNRSecondaryParent = NI_SNR_MINIMUM_UINT8;
                  }
                  
                  if ( MM_NI_UpdateMeshStatus(&statusInd) )
                  {
                     msg_status = AT_NO_ERROR_E;
                  }
                  else 
                  {
                     msg_status = AT_COMMAND_UNKNOWN_ERROR_E;
                  }
               }
               else
               {
                  msg_status = AT_INVALID_COMMAND_E;
               }
            }
            else 
            {
               msg_status = AT_INVALID_COMMAND_E;
            }
         }
         
      }
   }
   
   if ( AT_NO_ERROR_E == msg_status )
   {
      strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
      pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
   }
   else
   {
      strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
      pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
   }

   
   pCmdReply->port_id = uart_port;
   
   /* Send the response */
   osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
   
   if (osOK != status)
   {
      /* Failed to write, free the pre-allocated pool entry */
      FREE_ATHANDLE_POOL(pCmdReply);
      msg_status = AT_QUEUE_ERROR_E;
   }
   return msg_status;
}
#endif

#ifdef ENABLE_HEARTBEAT_TX_SKIP
/*************************************************************************************/
/**
* function name   : MM_ATCommand_SkipHeartbeat
* description     : Test aid. Instructs the device to not transmit a number of heartbeats.
*                   Set the given value to gNumberOfHeatbeatsToSkip.  Defaults to 1 if no
*                   value is given.
*                   ATSHB?
*                   ATSHB=<number to skip>
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_SkipHeartbeat(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id)
{
   ATMessageStatus_t msg_status = AT_INVALID_PARAMETER_E;
   AT_Parameters_t parameters;
   
   Cmd_Reply_t *pCmdReply = CALLOC_ATHANDLE_POOL
   if ( pCmdReply )
   {
      if ( source_buffer )
      {
         if ( AT_COMMAND_WRITE_E == command_type)
         {
            //Read the number of heartbeats to skip
            msg_status = MM_ATCommand_ParseCommandParameters(source_buffer, buffer_length, &parameters);
            if ( AT_NO_ERROR_E == msg_status )
            {
               if ( 1 == parameters.NumberOfParameters )
               {
                  gNumberOfHeatbeatsToSkip = parameters.Parameter[0];
                  msg_status = AT_NO_ERROR_E;
               }
               else if ( 0 == parameters.NumberOfParameters )
               {
                  gNumberOfHeatbeatsToSkip = 1;
                  msg_status = AT_NO_ERROR_E;
               }
               else 
               {
                  msg_status = AT_INVALID_PARAMETER_E;
               }
               
               if ( AT_NO_ERROR_E == msg_status )
               {
                  strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                  pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_OK);
               }
            }
         }
         else if ( AT_COMMAND_READ_E == command_type) 
         {
            sprintf((char *)pCmdReply->data_buffer, "%d", gNumberOfHeatbeatsToSkip);
            pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);
            msg_status = AT_NO_ERROR_E;
         }
      }
   }
   
   
   if ( AT_NO_ERROR_E != msg_status )
   {
      strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_FAIL);
      pCmdReply->length = (uint8_t)strlen(AT_RESPONSE_FAIL);
   }
   
   
   pCmdReply->port_id = uart_port;
   
   /* Send the response */
   osStatus status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
   
   if (osOK != status)
   {
      /* Failed to write, free the pre-allocated pool entry */
      FREE_ATHANDLE_POOL(pCmdReply);
      msg_status = AT_QUEUE_ERROR_E;
   }
   
   return msg_status;
}
#endif



#ifdef IS_PPU
/*************************************************************************************/
/**
* function name   : MM_ATCommand_PPUDeviceList
* description     : Returns th list of the device known to the PPU
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_PPUDeviceList(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t status = AT_NO_ERROR_E;
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdSignalReq = NULL;

   if ( !buffer_length )
   {
      pCmdSignalReq = osPoolAlloc(AppPool);
      if ( pCmdSignalReq )
      {
         pCmdSignalReq->Type = CO_MESSAGE_GENERATE_PPU_DEV_LIST_E;

         CO_CommandData_t *pCmdMessage =(CO_CommandData_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
         pCmdMessage->Source = uart_port;      
         pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_CommandData_t);

         osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCmdSignalReq);
            status = AT_QUEUE_ERROR_E;
         }
      }
      else
      {
         status = AT_QUEUE_ERROR_E;
      }
   }
   return status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_BroadcastPpModeMsg
* description     : Broadcast a command to request the Peer to Peer mode
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_BroadcastPpModeMsg(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t status = AT_NO_ERROR_E;
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdSignalReq = NULL;
   
   if ( !buffer_length )
   {
      pCmdSignalReq = osPoolAlloc(AppPool);
      if ( pCmdSignalReq  )
      {
         pCmdSignalReq->Type = CO_MESSAGE_BROADCAST_PP_MODE_REQ_E;            

         CO_CommandData_t *pCmdMessage = (CO_CommandData_t *)pCmdSignalReq->Payload.PhyDataReq.Data;
         pCmdMessage->Source = uart_port;      
         pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_CommandData_t);

         osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCmdSignalReq);
            status = AT_QUEUE_ERROR_E;
         }
      }
      else
      {
         status = AT_QUEUE_ERROR_E;
      }
   }
   return status;
}

/*************************************************************************************/
/**
* function name   : MM_ATCommand_PpuBroadcastMsgFreq
* description     : Set/Get the Radio Frequency used by the PPU to send the Broadcast command
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_PpuBroadcastMsgFreq(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t status = AT_INVALID_COMMAND_E;
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdSignalReq = NULL;
   uint32_t value;

   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if (pCmdReply)
   {
      switch (command_type) 
      {
         case AT_COMMAND_READ_E:               
         {  
            if (0 == buffer_length) /*No parameter expected for read operation */
            {
               value = PPU_GetFrequency();

               CO_HexString(value, (char *)pCmdReply->data_buffer, BASE_10);
               status = AT_NO_ERROR_E;
            }
         }
         break;

         case AT_COMMAND_WRITE_E:
         {    
            if ( source_buffer )
            {
               char *str_end;

               /* Get the state value */
               value = (uint32_t)strtol((char*)source_buffer, &str_end, 10);

               if(!*str_end)
               {
                  pCmdSignalReq = osPoolAlloc(AppPool);
                  if ( pCmdSignalReq )
                  {
                     pCmdSignalReq->Type = CO_MESSAGE_BROADCAST_PP_FREQ_E;

                     CO_CommandData_t *pCmdMessage = (CO_CommandData_t *)pCmdSignalReq->Payload.PhyDataReq.Data;
                     pCmdMessage->Source = uart_port;
                     pCmdMessage->Value = value;
                     pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_CommandData_t);

                     osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
                     if (osOK != osStat)
                     {
                        /* failed to write */
                        osPoolFree(AppPool, pCmdSignalReq);
                        status = AT_QUEUE_ERROR_E;
                     }
                     else
                     {
                        status = AT_NO_ERROR_E;
                        // Set up reply string for later
                        strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                     }
                  }
                  else
                  {
                     status = AT_QUEUE_ERROR_E;
                  }
               }
            }
         } 
         break;

         default: 
         {
            status = AT_INVALID_COMMAND_E;
         }
         break;
      }

      if (AT_NO_ERROR_E == status)
      {  
         pCmdReply->port_id = uart_port;
         pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);

         osStat = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK == osStat)
         {
            status = AT_NO_ERROR_E;
         }
         else
         {
            FREE_ATHANDLE_POOL(pCmdReply);
            status = AT_QUEUE_ERROR_E;
         }
      }
      else
      {
         FREE_ATHANDLE_POOL(pCmdReply);
      }
   }
   return status;
}


/*************************************************************************************/
/**
* function name   : MM_ATCommand_PpuBroadcastMsgSysId
* description     : Set/Get the System ID used by the PPU to send the Broadcast command
*
* @param - ATCommandType_t command_type (Read/Write/Special)
* @param - source_buffer   The received raw message.
* @param - buffer_length   The length of source_buffer.
* @param - UartComm_t uart_port  The source serial port
* @param - UInt32_t option_param  Optional parameter
*
* @return - ATMessageStatus_t
*/
ATMessageStatus_t MM_ATCommand_PpuBroadcastMsgSysId(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param)
{
   ATMessageStatus_t status = AT_INVALID_COMMAND_E;
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdSignalReq = NULL;
   uint32_t value;

   // Allocate Pool
   Cmd_Reply_t *pCmdReply = NULL;
   pCmdReply = CALLOC_ATHANDLE_POOL
   
   if (pCmdReply)
   {
      switch (command_type) 
      {
         case AT_COMMAND_READ_E:               
         {  
            if (0 == buffer_length) /*No parameter expected for read operation */
            {
               value = PPU_GetSystemId();

               CO_HexString(value, (char *)pCmdReply->data_buffer, BASE_10);
               status = AT_NO_ERROR_E;
            }
         }
         break;

         case AT_COMMAND_WRITE_E:
         {    
            if ( source_buffer )
            {
               char *str_end;

               /* Get the state value */
               value = (uint32_t)strtol((char*)source_buffer, &str_end, 10);

               if(!*str_end)
               {
                  pCmdSignalReq = osPoolAlloc(AppPool);
                  if ( pCmdSignalReq )
                  {
                     pCmdSignalReq->Type = CO_MESSAGE_BROADCAST_PP_SYS_ID_E;

                     CO_CommandData_t *pCmdMessage = (CO_CommandData_t *)pCmdSignalReq->Payload.PhyDataReq.Data;
                     pCmdMessage->Source = uart_port;
                     pCmdMessage->Value = value;
                     pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_CommandData_t);

                     osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
                     if (osOK != osStat)
                     {
                        /* failed to write */
                        osPoolFree(AppPool, pCmdSignalReq);
                        status = AT_QUEUE_ERROR_E;
                     }
                     else
                     {
                        status = AT_NO_ERROR_E;
                        // Set up reply string for later
                        strcpy((char *)pCmdReply->data_buffer, AT_RESPONSE_OK);
                     }
                  }
                  else
                  {
                     status = AT_QUEUE_ERROR_E;
                  }
               }
            }
         } 
         break;

         default: 
         {
            status = AT_INVALID_COMMAND_E;
         }
         break;
      }

      if (AT_NO_ERROR_E == status)
      {  
         pCmdReply->port_id = uart_port;
         pCmdReply->length = (uint8_t)strlen((char *)pCmdReply->data_buffer);

         osStat = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         
         if (osOK == osStat)
         {
            status = AT_NO_ERROR_E;
         }
         else{
            FREE_ATHANDLE_POOL(pCmdReply);
            status = AT_QUEUE_ERROR_E;
         }
      }
      else
      {
         FREE_ATHANDLE_POOL(pCmdReply);
      }
   }
   return status;
}

#endif /* IS_PPU */
