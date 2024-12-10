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
*  File         : MM_ATCommand.h
*
*  Description  : This module provides callback functions for all the AT Commands. Each
*                 AT command should have its own unique function.
*
*************************************************************************************/

#ifndef MM_AT_COMMAND_H
#define MM_AT_COMMAND_H


/* System Include Files
*************************************************************************************/
#include <stdbool.h>
#include <stdint.h>


/* User Include Files
*************************************************************************************/
//#include "main.h"
#include "CO_Defines.h"
#include "DM_SerialPort.h"
#include "CO_HexString.h"
#include "MM_ATHandleTask.h"
#include "DM_NVM.h"

/* Public Structures
*************************************************************************************/ 




/* Public Enumerations
*************************************************************************************/
typedef enum
{
   AT_COMPOUND_TYPE_COMMAND_E,
   AT_COMPOUND_TYPE_SVI_COMMAND_E,
   AT_COMPOUND_TYPE_TEST_SIGNAL_E,
   AT_COMPOUND_TYPE_OUTPUT_SIGNAL_E,
   AT_COMMAND_TYPE_ALARM_OUTPUT_STATE_E
} ATCompoundCommandType_t;

typedef enum
{
   AT_CIE_ERROR_NONE_E,
   AT_CIE_ERROR_UNKNOWN_COMMAND_E,
   AT_CIE_ERROR_VALUE_OUT_OF_RANGE_E,
   AT_CIE_ERROR_WRONG_VALUE_FORMAT_E,
   AT_CIE_ERROR_RU_DOES_NOT_EXIST_E,
   AT_CIE_ERROR_CHANNEL_DOES_NOT_EXIST_E,
   AT_CIE_ERROR_QUEUE_EMPTY_E,
   AT_CIE_ERROR_TX_BUFFER_FULL_E,
   AT_CIE_ERROR_NETWORK_TIMEOUT_E,
   AT_CIE_ERROR_MAX_E
} AT_CIE_ErrorCode_t;


/* Public Functions Prototypes
*************************************************************************************/
bool MM_ATEncodeSerialNumber(const char* pParamBuffer, uint32_t *value);
bool MM_ATDecodeSerialNumber(const uint32_t value, char* pParamBuffer);
bool MM_ATEncodeProductCode(const char* pParamBuffer, uint32_t *value);
bool MM_ATDecodeProductCode(const uint32_t value, char* pParamBuffer);
bool MM_ATEncodePreformParents(const char* pParamBuffer, uint32_t *pValue);
ATMessageStatus_t MM_ATCommand_enter_at_mode(const UartComm_t uart_port);
ATMessageStatus_t MM_ATCommand_enter_bootloader(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_decimalNumber(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_compound(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t compound_type);
ATMessageStatus_t MM_ATCommand_TestSignal(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_atTestMode(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_DeviceState(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_UnitReboot(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_UnitDisable(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_GetFirmwareInfo(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_RBUDisable(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_ReportAssociatedNodes(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_FirmwareImageCtrl(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_BuiltInTestReq(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_ZoneNumber(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_GPIOPinState(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_PPUDeviceList(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_PpuBroadcastMsgFreq(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_BroadcastPpModeMsg(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_PpuBroadcastMsgSysId(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_brandingID(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_ReadMessageQueues(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_ReadZonesAndDevices(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_ReadFireMessageQueue(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_ReadAlarmMessageQueue(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_ReadFaultMessageQueue(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_ReadMiscMessageQueue(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_RemoveFromFireMessageQueue(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_RemoveFromAlarmMessageQueue(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_RemoveFromFaultMessageQueue(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_RemoveFromMiscMessageQueue(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_ClearMessageQueues(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_NeighbourInformation(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_SetDayNight(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_PluginLED(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_AlarmOutputState(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_TestOneShot(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_Confirm(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_ReportOutputConfiguration(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_DisableBatteryChecks(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_LedPattern(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_DumpNeighbourInformation(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_SetLocalOrGlobalDelays(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_SetGlobalAlarmDelays(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_SetGlobalDelayOverride(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_ReadWriteNVM(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_MaxHops(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_RequestBatteryStatus(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_SetRssiMar(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_SetSnrMar(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_SetGlobalDelaysDayNight(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_SetZoneEnable(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_LogVisibleDevices(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_ScanForDevices(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_BatteryCheckPeriod(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_SetSnrAveraging(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_DisablePolling(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_DelayedOutputPulse(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_FaultsClearEnable(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
/* Delayed AT commands */
ATMessageStatus_t MM_ATCommand_RequestDeviceCombination(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_RequestStatusFlags(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_RequestStatusRequest(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_RequestRBUSerialNumber(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_TestMode(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_ReadAnalogueValue(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_BeaconFlashRate(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_EnablePlugin(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_ReadPluginSerialNumber(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_PluginSounderTone(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_PluginUpperThreshold(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_PluginLowerThreshold(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_PluginTestMode(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_PluginFirmwareVersion(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_PluginTypeAndClass(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_RBUFirmwareVersion(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_TxRxLogic(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_Profile(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_IOUThresholds(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_IOUPollPeriod(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_AlarmConfigurationState(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_AlarmOptionFlags(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_AlarmDelays(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_AlarmGlobalDelay(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_AlarmAcknowledge(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_AlarmEvacuate(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_EnableAlarms(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_AlarmReset(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_Ping(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_SoundLevel(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_ChannelFlags(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_MaximumRank(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_NCUPowerFail(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_DisableFaultReports(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_BatteryTestMode(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_ReadADC(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_200HourTest(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_CheckFirmware(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_EmulatedEprom(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_SendQueueCount(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_SendFaultMessage(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_RequestSensorValues(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_TxAtCommand(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_StartAtMode(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_StartPpuMode(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_ExitTestMode(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_BatteryTest(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_ProductCode(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_PPUModeEnable(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_PassivationSettings(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_SendAnnouncement(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t option_param);
ATMessageStatus_t MM_ATCommand_GetMcuUid(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_BatPriLowThresh(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_BatBkpLowThresh(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_BatPriLowThreshC(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_BatBkpLowThreshC(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);

#ifdef ENABLE_TDM_CALIBRATION
ATMessageStatus_t MM_ATCommand_SetTdmOffset(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_RachTxRxOffset(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
#endif
#ifdef ENABLE_LINK_ADD_OR_REMOVE
ATMessageStatus_t MM_ATCommand_DeleteNodeLink(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
ATMessageStatus_t MM_ATCommand_AddNodeLink(const ATCommandType_t command_type, const uint8_t *const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
#endif
#ifdef APPLY_RSSI_OFFSET
ATMessageStatus_t MM_ATCommand_RssiOffset(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
#endif
#ifdef ENABLE_BATTERY_VOLTAGE_COMMAND
ATMessageStatus_t MM_ATCommand_ReadBatteryVoltages(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
#endif
#ifdef ENABLE_NCU_CACHE_DEBUG
ATMessageStatus_t MM_ATCommand_NcuStatusMessage(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
#endif
#ifdef ENABLE_HEARTBEAT_TX_SKIP
ATMessageStatus_t MM_ATCommand_SkipHeartbeat(const ATCommandType_t command_type, const uint8_t* const source_buffer, const uint8_t buffer_length, const UartComm_t uart_port, const uint32_t param_id);
#endif

/* Public Constants
*************************************************************************************/
#define AT_RESPONSE_OK "OK"
#define AT_RESPONSE_FAIL "ERROR"


/* Macros
*************************************************************************************/



#endif // MM_AT_COMMAND_H
