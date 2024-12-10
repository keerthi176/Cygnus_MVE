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
*  File         : MM_PluginInterfaceTask.h
*
*  Description  : Plugin Interface Task header file
*
*************************************************************************************/

#ifdef USE_NEW_HEAD_INTERFACE

#ifndef MM_HEAD_INTERFACE_TASK_H
#define MM_HEAD_INTERFACE_TASK_H


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/
#include "DM_SerialPort.h"
#include "CO_ErrorCode.h"
#include "CO_Message.h"


/* Public definitions
*************************************************************************************/ 
#define PLUGIN_MSG_BUFFER_SIZE 20

/* Public Structures
*************************************************************************************/ 


/* Public Enumerations
*************************************************************************************/

// Head Type definitions from "table 11-Type of devices" in Fire Detector Specification (1000-SPC-0002-03)
typedef enum
{
   DEV_NO_DEVICE_TYPE_E,
   DEV_SMOKE_DETECTOR_HEAD_E,
   DEV_HEAT_DETECTOR_HEAD_E,
   DEV_CO_DETECTOR_HEAD_E,
   DEV_PIR_DETECTOR_HEAD_E,
   DEV_BEACON_HEAD_E,
   DEV_CALL_POINT_E,
   DEV_IO_UNIT_E,
   DEV_INDICATOR_LED_HEAD_E,
   DEV_SPARE_1_E,
   DEV_SPARE_2_E,
   DEV_MAX_DEVICE_TYPE_E
} MM_HeadDeviceType_t;

/* Detector Heat Class definitions from section 7.9.4 in Fire Detector Specification (1000-SPC-0002-03) */
typedef enum
{
   DEV_HEAT_CLASS_UNKNOWN_E,
   DEV_HEAT_CLASS_A1R_E,
   DEV_HEAT_CLASS_B_E
} MM_HeatDeviceClass_t;

/* Each device type is assigned a bit in a uint32_t */
#define BD(X) (1U << (DEV_##X##_E -1))
#define DEV_SMOKE_DETECTOR_HEAD              ( BD(SMOKE_DETECTOR_HEAD) )
#define DEV_HEAT_DETECTOR_HEAD               ( BD(HEAT_DETECTOR_HEAD) )
#define DEV_CO_DETECTOR_HEAD                  ( BD(CO_DETECTOR_HEAD) )
#define DEV_PIR_DETECTOR_HEAD                  ( BD(PIR_DETECTOR_HEAD) )
#define DEV_SOUNDER_HEAD                     ( BD(SOUNDER_HEAD) )
#define DEV_BEACON_HEAD                        ( BD(BEACON_HEAD) )
#define DEV_CALL_POINT                        ( BD(CALL_POINT) )
#define DEV_IO_UNIT                           ( BD(IO_UNIT) )
#define DEV_SPARE_1                          ( BD(SPARE_1) )
#define DEV_SPARE_2                          ( BD(SPARE_2) )
#define DEV_SPARE_3                           ( BD(SPARE_3) )
#define DEV_MAX_DEVICE_TYPE                     ( BD(MAX_DEVICE_TYPE) )

//define the class settings
#define DEV_CLASS_HEAT_A1R 1
#define DEV_CLASS_HEAT_B   2

/* Sensor Type list */
typedef enum
{
   SENSOR_SMOKE_E,
   SENSOR_HEAT_E,
   SENSOR_PIR_E,
   SENSOR_UNKNOWN_E
} MM_SensorType_t;

/* Head State Machine Enum */
typedef enum
{
   HEAD_STATE_STOPPED_E,
   HEAD_STATE_INIT_E,
   HEAD_STATE_IDLE_E,
   HEAD_STATE_START_WAKE_UP_E,
   HEAD_STATE_WAKE_UP_E,
   HEAD_STATE_PREPARE_SERIAL_PORT_E,
   HEAD_STATE_PREPARE_CMD_E,
   HEAD_STATE_COMMS_SEND_CMD_E,
   HEAD_STATE_COMMS_WAIT_REPLY_E,
   HEAD_STATE_EVALUATE_E
} HeadInterfaceState_t;

typedef enum
{
   HEAD_CMD_SERIAL_NUMBER_E,
   HEAD_CMD_SOFTWARE_INFO_E,
   HEAD_CMD_DEVICE_TYPE_E,
   HEAD_CMD_DEVICE_CLASS_E,
   HEAD_CMD_SMOKE_UPPER_THRLD_E,
   HEAD_CMD_SMOKE_LOWER_THRLD_E,
   HEAD_CMD_HEAT_UPPER_THRLD_E,
   HEAD_CMD_HEAT_LOWER_THRLD_E,
   HEAD_CMD_SMOKE_ANA_VALUE_E,
   HEAD_CMD_HEAT_ANA_VALUE_E,
   HEAD_CMD_INDICATOR_CTRL_E,
   HEAD_CMD_DETECTOR_STATUS_E,
   HEAD_CMD_DEVICE_TYPE_CLASS_E,
   HEAD_CMD_PIR_STATUS_E,
   HEAD_CMD_RESET_PIR_E,
   HEAD_CMD_ENABLE_PIR_E,
   HEAD_CMD_ENABLE_SMOKE_E,
   HEAD_CMD_ENABLE_HEAT_E,
   HEAD_CMD_ENABLE_BEACON_E,
   HEAD_CMD_OPTICAL_CHBR_STATUS_E,
   HEAD_CMD_SWITCH_CONTROL_E,
   HEAD_CMD_ID_NUMBER_E,
   HEAD_CMD_RESET_SOURCE_E,
   HEAD_CMD_DISABLE_COMMS_E,
   HEAD_CMD_BRANDING_ID_E,
   HEAD_CMD_BEACON_TEST_MODE_E,
   HEAD_CMD_BEACON_FLASH_RATE_E,
   HEAD_CMD_BEACON_FAULT_STATUS_E,
   HEAD_CMD_BEACON_TYPE_E,
   HEAD_CMD_EMPTY_E
} PluginCmdType_t;

typedef enum
{
   HEAD_CMD_TYPE_READ_E,
   HEAD_CMD_TYPE_WRITE_E,
   HEAD_CMD_TYPE_SPECIAL_E
} PluginAccessType_t;
   
typedef enum
{
   HEAD_ACTIVATION_NORMAL_E,
   HEAD_ACTIVATION_ALARM_E,
   HEAD_ACTIVATION_FAULT_E
} HeadActivationState_t;

/* Optical chamber status */
typedef enum
{
   HEAD_OPTICAL_CHAMBER_NORMAL_E,
   HEAD_OPTICAL_CHAMBER_LOW_DIRT_E,
   HEAD_OPTICAL_CHAMBER_MED_DIRT_E,
   HEAD_OPTICAL_CHAMBER_FAULT_E
} HeadOpticalChamberStatus_t;

typedef enum
{
   HEAD_BEACON_TEST_INACTIVE_E,
   HEAD_BEACON_SELF_TEST_ACTIVE_E,
   HEAD_BEACON_TEST_ACTIVE_E
} BeaconSelfTestState_t;

typedef struct
{
   uint32_t HType;
   uint8_t HClass;
   uint8_t HEnabled;
   uint8_t HSmokeEnable;
   uint8_t HSmokeUpperThr[CO_NUM_DAYNIGHT_SETTINGS_E];
   uint8_t HSmokeLowerThr[CO_NUM_DAYNIGHT_SETTINGS_E];
   uint8_t HSmokeFaultThr;
   uint8_t HHeatEnable[CO_NUM_DAYNIGHT_SETTINGS_E];
   uint8_t HHeatUpperThr[CO_NUM_DAYNIGHT_SETTINGS_E];
   uint8_t HHeatLowerThr[CO_NUM_DAYNIGHT_SETTINGS_E];
   uint8_t HHeatFaultThr;
   uint8_t HSmokeAnalogueValue;
   uint8_t HHeatAnalogueValue;
   HeadActivationState_t HSmokeAlarmState;
   HeadActivationState_t HHeatAlarmState;
   uint8_t HSmokeFaultStatus;
   uint8_t HHeatFaultStatus;
   uint8_t HLEDStatus;
   uint8_t HStatus_State;
   uint8_t HStatus_Device;
   uint32_t HSerialNumber;
   uint8_t HBeaconType;
   uint8_t HBeaconEnable;
   uint8_t HBeaconFaultStatus;
   uint8_t HBeaconFlashRate;
   uint8_t HBeaconTestMode;
   HeadActivationState_t HBeaconAlarmState;
   uint32_t HFirmwareVersion;
   uint8_t HPIREnabled[CO_NUM_DAYNIGHT_SETTINGS_E];
   uint8_t HPIRStatus;
   uint8_t HPIREnable;
   uint8_t HPIRFaultStatus;
   uint8_t HOptChamberStatus;
   uint32_t HIdentificationNumber;
   uint8_t HResetSource;
   uint8_t HBrandingID;
   CO_RBUSensorType_t HSensorType;
   CO_DayNight_t HDayNight;
   bool HMismatchedType;
   bool HResponding;
} HeadDataObject_t;

typedef struct
{
   uint8_t UpperThr;
   uint8_t LowerThr;
   uint8_t AnalogueValue;
   uint8_t Enable;
} Detector_t;

typedef struct
{
   PluginCmdType_t PluginMessageType;
   PluginAccessType_t readWrite;
   HeadMessageType_t CommandMessageType;
   uint8_t TransactionID;
   uint8_t ChannelNumber;
   uint8_t ProfileIndex;
   uint8_t Duration;
   uint32_t value;
   uint8_t buffer[PLUGIN_MSG_BUFFER_SIZE];
} PluginMessage_t;


/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t MM_HeadInterfaceTaskInit(const uint16_t deviceCombination);
void MM_HeadInterfaceTaskMain(void const *argument);
bool MM_HeadInterface_CanGoToSleep(void);
ErrorCode_t MM_HeadInterface_BuiltInTestReq(void);
void MM_HeadActivateTestRelay(void);
void MM_HeadDeActivateTestRelay(void);
uint8_t MM_HeadGetRelayStatus(void);
bool MM_HeadDecodeFirmwareVersion(const uint32_t version, const uint8_t *version_str, const uint32_t ver_str_length);
HeadActivationState_t MM_GetActivationState(const CO_ChannelIndex_t channel);
uint8_t MM_GetSmokeSensorValue(void);
uint8_t MM_GetHeatSensorValue(void);
uint8_t MM_GetPIRState(void);
uint8_t MM_SensorFaultStatus(void);
void HeadPeriodicTimerCallback(void const *id);
/* Public Constants
*************************************************************************************/
#define HEAD_COMMS_TIMEOUT                        10   // Iterations it waits for a reply from the head in increments of 10ms
/* default threshold settings */
#define HEAD_DEFAULT_SMOKE_UPPER_THRESHOLD_DAY        55
#define HEAD_DEFAULT_SMOKE_LOWER_THRESHOLD_DAY        30
#define HEAD_DEFAULT_HEAT_UPPER_THRESHOLD_A1R_DAY     55
#define HEAD_DEFAULT_HEAT_UPPER_THRESHOLD_B_DAY       72
#define HEAD_DEFAULT_HEAT_LOWER_THRESHOLD_A1R_DAY     30
#define HEAD_DEFAULT_HEAT_LOWER_THRESHOLD_B_DAY       45
#define HEAD_DEFAULT_SMOKE_UPPER_THRESHOLD_NIGHT      55
#define HEAD_DEFAULT_SMOKE_LOWER_THRESHOLD_NIGHT      30
#define HEAD_DEFAULT_HEAT_UPPER_THRESHOLD_A1R_NIGHT   55
#define HEAD_DEFAULT_HEAT_UPPER_THRESHOLD_B_NIGHT     72
#define HEAD_DEFAULT_HEAT_LOWER_THRESHOLD_A1R_NIGHT   30
#define HEAD_DEFAULT_HEAT_LOWER_THRESHOLD_B_NIGHT     45
#define HEAD_DEFAULT_FAULT_THRESHOLD                  8

#define HEAD_FAULT_CLEAR 0
#define HEAD_FAULT_INDICATED 1
#define HEAD_FAULT_DEVICE_SMOKE 1
#define HEAD_FAULT_DEVICE_HEAT 2
#define HEAD_FAULT_DEVICE_SMOKE_AND_HEAT 3
#define HEAD_FAULT_DEVICE_OPTICAL_CHAMBER 4
#define HEAD_FAULT_DEVICE_PIR 8


/* Macros
*************************************************************************************/



#endif // MM_HEAD_INTERFACE_TASK_H

#endif
