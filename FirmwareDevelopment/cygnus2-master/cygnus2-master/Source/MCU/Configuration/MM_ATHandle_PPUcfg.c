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
*  File         : MM_ATHandle_PPUcfg.c
*
*  Description  :configuration of the AT Handle Module for the PPU SW
*
*************************************************************************************/



/* System Include files
*******************************************************************************/
																									 

/* User Include files
*******************************************************************************/
#include "stm32l4xx.h"
#include "CO_ErrorCode.h"
#include "MM_ATCommand.h"
#include "MM_ATHandle_cfg.h"
#include "MM_ATHandleTask.h"


/* Private Functions Prototypes
*************************************************************************************/


/* Global Variables
*************************************************************************************/
const at_command_def_t at_command_def[]=
{
   {
      .at_command = "ATBOOT",
      .func_callback = MM_ATCommand_enter_bootloader,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATSYNC",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_IS_SYNC_MASTER_E,
   },
   {
      .at_command = "ATUA",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_ADDRESS_E,
   },
   {
      .at_command = "ATFREQ",
      .func_callback = MM_ATCommand_PpuBroadcastMsgFreq,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATDEVCF",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_DEVICE_COMBINATION_E,
   },
   {
      .at_command = "ATSYSID",
      .func_callback = MM_ATCommand_PpuBroadcastMsgSysId,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATSERNO",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_UNIT_SERIAL_NO_E,
   },
   {
      .at_command = "ATBPPU",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATTXPLO",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_TX_POWER_LOW_E,
   },
   {
      .at_command = "ATTXPHI",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_TX_POWER_HIGH_E,
   },
   {
      .at_command = "ATFI",
      .func_callback = MM_ATCommand_GetFirmwareInfo,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATFA",
      .func_callback = MM_ATCommand_FirmwareImageCtrl,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 1,
   },   
   {
      .at_command = "ATFAN",
      .func_callback = MM_ATCommand_FirmwareImageCtrl,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "ATRRU",
      .func_callback = MM_ATCommand_UnitReboot,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },   
   {
      .at_command = "ATCMD",
      .func_callback = MM_ATCommand_compound,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = AT_COMPOUND_TYPE_COMMAND_E,
   },   
   {
      .at_command = "ATSTATE",
      .func_callback = MM_ATCommand_DeviceState,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATOUT",
      .func_callback = MM_ATCommand_compound,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = AT_COMPOUND_TYPE_OUTPUT_SIGNAL_E,
   },
   {
      .at_command = "ATPPLST",
      .func_callback = MM_ATCommand_PPUDeviceList,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_NO_REPLY_E,
      .option_param = 0,
   },
   {
      .at_command = "ATPPBST",
      .func_callback = MM_ATCommand_BroadcastPpModeMsg,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   }
};


const uint16_t at_command_def_size = sizeof(at_command_def)/sizeof(at_command_def[0]);


/* Private Variables
*************************************************************************************/

