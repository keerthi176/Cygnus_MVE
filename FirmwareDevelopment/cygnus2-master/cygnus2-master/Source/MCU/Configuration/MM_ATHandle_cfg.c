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
*  File         : MM_ATHandle_cfg.c
*
*  Description  :configuration of the AT Handle Module
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
#include "CO_Message.h"

/* Private Functions Prototypes
*************************************************************************************/


/* Global Variables
*************************************************************************************/
const at_command_def_t at_command_def[]=
{
   {
      .at_command = "ATMSN",
      .func_callback = MM_ATCommand_ReadMessageQueues,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "ATNOD",
      .func_callback = MM_ATCommand_ReadZonesAndDevices,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "ATQFE",
      .func_callback = MM_ATCommand_ReadFireMessageQueue,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "ATQAM",
      .func_callback = MM_ATCommand_ReadAlarmMessageQueue,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "ATQFT",
      .func_callback = MM_ATCommand_ReadFaultMessageQueue,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "ATQMC",
      .func_callback = MM_ATCommand_ReadMiscMessageQueue,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_RESPONSE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATXFE",
      .func_callback = MM_ATCommand_RemoveFromFireMessageQueue,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATXAM",
      .func_callback = MM_ATCommand_RemoveFromAlarmMessageQueue,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATXFT",
      .func_callback = MM_ATCommand_RemoveFromFaultMessageQueue,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATXMC",
      .func_callback = MM_ATCommand_RemoveFromMiscMessageQueue,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATXMQ",
      .func_callback = MM_ATCommand_ClearMessageQueues,
      .supported_operations = AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATDEC",
      .func_callback = MM_ATCommand_RequestDeviceCombination,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "ATDSF",
      .func_callback = MM_ATCommand_RequestStatusFlags,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "ATMSR",
      .func_callback = MM_ATCommand_RequestStatusRequest,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "ATOUT",
      .func_callback = MM_ATCommand_compound,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = AT_COMPOUND_TYPE_OUTPUT_SIGNAL_E,
   },
   {
      .at_command = "ATAOS",
      .func_callback = MM_ATCommand_compound,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = AT_COMMAND_TYPE_ALARM_OUTPUT_STATE_E,
   },
   {
      .at_command = "ATRRU",
      .func_callback = MM_ATCommand_UnitReboot,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },   
   {
      .at_command = "ATEDR",
      .func_callback = MM_ATCommand_UnitDisable,
      .supported_operations = AT_COMMAND_WRITE_E,
      .option_param = 0,
   },   
   {
      .at_command = "ATTMR",
      .func_callback = MM_ATCommand_TestMode,
      .supported_operations = AT_COMMAND_WRITE_E,
      .option_param = 0,
   },   
   {
      .at_command = "ATANA",
      .func_callback = MM_ATCommand_ReadAnalogueValue,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },   
   {
      .at_command = "ATBFR",
      .func_callback = MM_ATCommand_BeaconFlashRate,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },   
   {
      .at_command = "ATEDP",
      .func_callback = MM_ATCommand_EnablePlugin,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },   
   {
      .at_command = "ATSNP",
      .func_callback = MM_ATCommand_ReadPluginSerialNumber,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },   
   {
      .at_command = "ATSNR",
      .func_callback = MM_ATCommand_RequestRBUSerialNumber,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "ATSTS",
      .func_callback = MM_ATCommand_PluginSounderTone,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },   
   {
      .at_command = "ATTHU",
      .func_callback = MM_ATCommand_PluginUpperThreshold,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },   
   {
      .at_command = "ATTHL",
      .func_callback = MM_ATCommand_PluginLowerThreshold,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },   
   {
      .at_command = "ATTMP",
      .func_callback = MM_ATCommand_PluginTestMode,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },   
   {
      .at_command = "ATFIP",
      .func_callback = MM_ATCommand_PluginFirmwareVersion,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },   
   {
      .at_command = "ATFIR",
      .func_callback = MM_ATCommand_RBUFirmwareVersion,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },   
   {
      .at_command = "ATBID",
      .func_callback = MM_ATCommand_brandingID,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_BRANDING_ID_E,
   },
   {
      .at_command = "ATMFS",
      .func_callback = MM_ATCommand_DeviceState,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATBOOT",
      .func_callback = MM_ATCommand_enter_bootloader,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATBOOT1",
      .func_callback = MM_ATCommand_enter_bootloader,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 1,
   },
   {
      .at_command = "ATUA",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_ADDRESS_E,
   },
	{
      .at_command = "ATPIRON",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_PIR_ACTIVATE_PERIOD_E,
   },
	{
      .at_command = "ATPIROF",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_PIR_DEACTIVATE_PERIOD_E,
   },
	{
      .at_command = "ATPIRCN",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_PIR_STRIKE_COUNT_E,
   },
   {
      .at_command = "ATSYNC",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_IS_SYNC_MASTER_E,
   },
   {
      .at_command = "ATFREQ",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_FREQUENCY_CHANNEL_E,
   },
   {
      .at_command = "ATDEVCF",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_DEVICE_COMBINATION_E,
   },
   {
      .at_command = "ATSYSID",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_SYSTEM_ID_E,
   },   
   {
      .at_command = "ATSERNO",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_UNIT_SERIAL_NO_E,
   },   
   {
      .at_command = "ATSERPX",
      .func_callback = MM_ATCommand_ProductCode,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATPFM",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_PREFORM_MESH_PARENTS_E,
   },
   {
      .at_command = "ATFLEN",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_SHORT_FRAMES_PER_LONG_FRAME_E,
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
      .at_command = "ATPPEN",
      .func_callback = MM_ATCommand_PPUModeEnable,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_PP_MODE_ENABLE_E,
   },
   {
      .at_command = "ATFI",
      .func_callback = MM_ATCommand_GetFirmwareInfo,
      .supported_operations = AT_COMMAND_READ_E,
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
      .at_command = "ATR",
      .func_callback = MM_ATCommand_UnitReboot,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATRP",
      .func_callback = MM_ATCommand_enter_bootloader,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 1,
   },   
   {
      .at_command = "ATCMD",
      .func_callback = MM_ATCommand_compound,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = AT_COMPOUND_TYPE_COMMAND_E,
   },   
   {
      .at_command = "ATSVI",
      .func_callback = MM_ATCommand_compound,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = AT_COMPOUND_TYPE_SVI_COMMAND_E,
   },   
   {
      .at_command = "ATSTATE",
      .func_callback = MM_ATCommand_DeviceState,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },   
   {
      .at_command = "ATMODE",
      .func_callback = MM_ATCommand_atTestMode,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATTST",
      .func_callback = MM_ATCommand_TestSignal,
      .supported_operations = AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATDISRB",
      .func_callback = MM_ATCommand_RBUDisable,
      .supported_operations = AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATNODES",
      .func_callback = MM_ATCommand_ReportAssociatedNodes,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATBIT",
      .func_callback = MM_ATCommand_BuiltInTestReq,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATZONE",
      .func_callback = MM_ATCommand_ZoneNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_ZONE_NUMBER_E,
   },
   {
      .at_command = "ATPIN",
      .func_callback = MM_ATCommand_GPIOPinState,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATSDN",
      .func_callback = MM_ATCommand_SetDayNight,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATDNI",
      .func_callback = MM_ATCommand_NeighbourInformation,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "ATLED",
      .func_callback = MM_ATCommand_PluginLED,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATTXRX",
      .func_callback = MM_ATCommand_TxRxLogic,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATPRF",
      .func_callback = MM_ATCommand_Profile,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },   
   {
      .at_command = "ATIOUTH",
      .func_callback = MM_ATCommand_IOUThresholds,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATIOUP",
      .func_callback = MM_ATCommand_IOUPollPeriod,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_IOU_INPUT_POLL_PERIOD_E,
   },
   {
      .at_command = "ATACS",
      .func_callback = MM_ATCommand_AlarmConfigurationState,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATAOF",
      .func_callback = MM_ATCommand_AlarmOptionFlags,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATADC",
      .func_callback = MM_ATCommand_AlarmDelays,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATGADC",
      .func_callback = MM_ATCommand_SetGlobalAlarmDelays,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATAGD",
      .func_callback = MM_ATCommand_AlarmGlobalDelay,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATEVAC",
      .func_callback = MM_ATCommand_AlarmEvacuate,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATDISZ",
      .func_callback = MM_ATCommand_EnableAlarms,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = CO_DISABLE_ZONE_E,
   },
   {
      .at_command = "ATDISD",
      .func_callback = MM_ATCommand_EnableAlarms,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = CO_DISABLE_DEVICE_E,
   },
   {
      .at_command = "ATDISC",
      .func_callback = MM_ATCommand_EnableAlarms,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = CO_DISABLE_CHANNEL_E,
   },
   {
      .at_command = "ATACKF",
      .func_callback = MM_ATCommand_AlarmAcknowledge,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = CO_PROFILE_FIRE_E,
   },
   {
      .at_command = "ATACKE",
      .func_callback = MM_ATCommand_AlarmAcknowledge,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = CO_PROFILE_FIRST_AID_E,
   },
   {
      .at_command = "ATRST",
      .func_callback = MM_ATCommand_AlarmReset,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATPING",
      .func_callback = MM_ATCommand_Ping,
      .supported_operations = AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATSL",
      .func_callback = MM_ATCommand_SoundLevel,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATTOS",
      .func_callback = MM_ATCommand_TestOneShot,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATCONF",
      .func_callback = MM_ATCommand_Confirm,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = CO_PROFILE_FIRE_E,
   },
   {
      .at_command = "ATCONE",
      .func_callback = MM_ATCommand_Confirm,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = CO_PROFILE_FIRST_AID_E,
   },
   {
      .at_command = "ATCHF",
      .func_callback = MM_ATCommand_ChannelFlags,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATOPC",
      .func_callback = MM_ATCommand_ReportOutputConfiguration,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "ATDBC",
      .func_callback = MM_ATCommand_DisableBatteryChecks,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATLEDP",
      .func_callback = MM_ATCommand_LedPattern,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E | AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATDNIR",
      .func_callback = MM_ATCommand_DumpNeighbourInformation,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATRNK",
      .func_callback = MM_ATCommand_MaximumRank,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATPF",
      .func_callback = MM_ATCommand_NCUPowerFail,
      .supported_operations = AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATDISF",
      .func_callback = MM_ATCommand_DisableFaultReports,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATBMODE",
      .func_callback = MM_ATCommand_BatteryTestMode,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATRADC",
      .func_callback = MM_ATCommand_ReadADC,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "AT200",
      .func_callback = MM_ATCommand_200HourTest,
      .supported_operations = AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATCHFW",
      .func_callback = MM_ATCommand_CheckFirmware,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATDWRAP",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_DULCH_WRAP_E,
   },
   {
      .at_command = "ATLDLY",
      .func_callback = MM_ATCommand_SetLocalOrGlobalDelays,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATEEI",
      .func_callback = MM_ATCommand_EmulatedEprom,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATNVM",
      .func_callback = MM_ATCommand_ReadWriteNVM,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATMAXHP",
      .func_callback = MM_ATCommand_MaxHops,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATGDLY",
      .func_callback = MM_ATCommand_SetGlobalDelayOverride,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATBATS",
      .func_callback = MM_ATCommand_RequestBatteryStatus,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "ATJRSSI",
      .func_callback = MM_ATCommand_SetRssiMar,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATJSNR",
      .func_callback = MM_ATCommand_SetSnrMar,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATGSET",
      .func_callback = MM_ATCommand_SetGlobalDelaysDayNight,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATLOZ",
      .func_callback = MM_ATCommand_SetZoneEnable,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = ZONE_BITS_LOW_E,
   },
   {
      .at_command = "ATUPZ",
      .func_callback = MM_ATCommand_SetZoneEnable,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = ZONE_BITS_HIGH_E,
   },
   {
      .at_command = "ATDVD",
      .func_callback = MM_ATCommand_LogVisibleDevices,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATSCAN",
      .func_callback = MM_ATCommand_ScanForDevices,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATBATP",
      .func_callback = MM_ATCommand_BatteryCheckPeriod,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATSNRAV",
      .func_callback = MM_ATCommand_SetSnrAveraging,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATILP",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_INIT_LISTEN_PERIOD_E,
   },
   {
      .at_command = "ATPH2",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_PHASE2_DURATION_E,
   },
   {
      .at_command = "ATISP2",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_PHASE2_SLEEP_DURATION_E,
   },
   {
      .at_command = "ATISP3",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_PHASE3_SLEEP_DURATION_E,
   },
   {
      .at_command = "ATDPOLL",
      .func_callback = MM_ATCommand_DisablePolling,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATFBAND",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_FREQUENCY_BAND_E,
   },
   {
      .at_command = "ATPTYPE",
      .func_callback = MM_ATCommand_PluginTypeAndClass,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "ATSQC",
      .func_callback = MM_ATCommand_SendQueueCount,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "ATSNDF",
      .func_callback = MM_ATCommand_SendFaultMessage,
      .supported_operations = AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATMAXCH",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_MAX_RBU_CHILDREN_E,
   },
   {
      .at_command = "ATMAXDL",
      .func_callback = MM_ATCommand_decimalNumber,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = NV_NUMBER_MSG_RESENDS_E,
   },
   {
      .at_command = "ATSENV",
      .func_callback = MM_ATCommand_RequestSensorValues,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATBTST",
      .func_callback = MM_ATCommand_BatteryTest,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATATTX",
      .func_callback = MM_ATCommand_TxAtCommand,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATSAT",
      .func_callback = MM_ATCommand_StartAtMode,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATPPU",
      .func_callback = MM_ATCommand_StartAtMode,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 1,
   },
   {
      .at_command = "ATPPUMD",
      .func_callback = MM_ATCommand_StartPpuMode,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATETEST",
      .func_callback = MM_ATCommand_ExitTestMode,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATDPASD",
      .func_callback = MM_ATCommand_PassivationSettings,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATUID",
      .func_callback = MM_ATCommand_GetMcuUid,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   },
   {
      .at_command = "ATANN",
      .func_callback = MM_ATCommand_SendAnnouncement,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATDLOUT",
      .func_callback = MM_ATCommand_DelayedOutputPulse,
      .supported_operations = AT_COMMAND_SPECIAL_E,
      .option_param = 0,
   },
   {
      .at_command = "ATFCM",
      .func_callback = MM_ATCommand_FaultsClearEnable,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATBATPS",
      .func_callback = MM_ATCommand_BatPriLowThresh,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATBATBS",
      .func_callback = MM_ATCommand_BatBkpLowThresh,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
	 {
      .at_command = "ATBATPC",
      .func_callback = MM_ATCommand_BatPriLowThreshC,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATBATBC",
      .func_callback = MM_ATCommand_BatBkpLowThreshC,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   }
#ifdef ENABLE_LINK_ADD_OR_REMOVE
   ,
   {
      .at_command = "ATDNL",
      .func_callback = MM_ATCommand_DeleteNodeLink,
      .supported_operations = AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATANL",
      .func_callback = MM_ATCommand_AddNodeLink,
      .supported_operations = AT_COMMAND_WRITE_E,
      .option_param = 0,
   }
#endif
#ifdef APPLY_RSSI_OFFSET
   ,
   {
      .at_command = "ATROFF",
      .func_callback = MM_ATCommand_RssiOffset,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   }
#endif
#ifdef ENABLE_TDM_CALIBRATION
   ,
   {
      .at_command = "ATTIM",
      .func_callback = MM_ATCommand_SetTdmOffset,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   },
   {
      .at_command = "ATRXO",
      .func_callback = MM_ATCommand_RachTxRxOffset,
      .supported_operations = AT_COMMAND_WRITE_E,
      .option_param = 0,
   }
#endif
#ifdef ENABLE_BATTERY_VOLTAGE_COMMAND
   ,
   {
      .at_command = "ATBV",
      .func_callback = MM_ATCommand_ReadBatteryVoltages,
      .supported_operations = AT_COMMAND_READ_E,
      .option_param = 0,
   }
#endif
#ifdef ENABLE_NCU_CACHE_DEBUG
   ,
   {
      .at_command = "ATSTM",
      .func_callback = MM_ATCommand_NcuStatusMessage,
      .supported_operations = AT_COMMAND_WRITE_E,
      .option_param = 0,
   }
#endif
#ifdef ENABLE_HEARTBEAT_TX_SKIP
   ,
   {
      .at_command = "ATSHB",
      .func_callback = MM_ATCommand_SkipHeartbeat,
      .supported_operations = AT_COMMAND_READ_E | AT_COMMAND_WRITE_E,
      .option_param = 0,
   }
#endif
};

const uint16_t at_command_def_size = sizeof(at_command_def)/sizeof(at_command_def[0]);

/* Private Variables
*************************************************************************************/

