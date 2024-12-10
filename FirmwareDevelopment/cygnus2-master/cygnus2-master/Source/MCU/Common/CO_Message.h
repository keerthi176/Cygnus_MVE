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
*  File         : CO_Message.h
*
*  Description  : Message definitions header file
*
*************************************************************************************/

#ifndef CO_MESSAGE_H
#define CO_MESSAGE_H


/* System Include Files
*************************************************************************************/
#include "stdint.h"
#include <stdbool.h>
#include "CO_Defines.h"


/* User Include Files
*************************************************************************************/



/* Public Constants
*************************************************************************************/
/* frame sizes in bits */
#define HEARTBEAT_SIZE  72
#define DATA_MESSAGE_SIZE 156
#define ACKNOWLEDGEMENT_SIZE 60
#define SET_ADDRESS_MSG_SIZE 80

/* frame sizes in bytes (rounded up) */
#define HEARTBEAT_SIZE_BYTES         ((HEARTBEAT_SIZE + 7) >> 3)
#define DATA_MESSAGE_SIZE_BYTES      ((DATA_MESSAGE_SIZE + 7) >> 3)
#define ACKNOWLEDGEMENT_SIZE_BYTES   ((ACKNOWLEDGEMENT_SIZE + 7) >> 3)
#define SET_ADDRESS_MSG_SIZE_BYTES   ((SET_ADDRESS_MSG_SIZE + 7) >> 3)

#ifdef USE_LORA_CHIP_CRC

#define PHY_DATA_IND_LENGTH_MAX 21
#define PHY_DATA_REQ_LENGTH_MAX (PHY_DATA_IND_LENGTH_MAX)

#define LORA_INITIALISE_CRC  true  // LORA CRC mode set at initialisation.
#define LORA_RACH_CRC        true  // LORA CRC mode used for RACH messages
#define LORA_ACK_CRC         true  // LORA CRC mode used for ACK messages
#define LORA_DLCCH_CRC       true  // LORA CRC mode used for DLCCH messages
#define LORA_DCH_CRC         true  // LORA CRC mode used for DCH messages

#define PHY_DCH_PACKET_SIZE       HEARTBEAT_SIZE_BYTES
#define PHY_RACH_PACKET_SIZE      DATA_MESSAGE_SIZE_BYTES
#define PHY_RACH_ACK_PACKET_SIZE  ACKNOWLEDGEMENT_SIZE_BYTES
#define PHY_DLCCH_PACKET_SIZE     DATA_MESSAGE_SIZE_BYTES
#else /* USE_LORA_CHIP_CRC */

/* 2 Bytes are reserved for the CRC */
#define PHY_DATA_IND_LENGTH_MAX 36
#define PHY_DATA_REQ_LENGTH_MAX (PHY_DATA_IND_LENGTH_MAX)

#define LORA_INITIALISE_CRC  false
#define LORA_RACH_CRC        false
#define LORA_ACK_CRC         false
#define LORA_DLCCH_CRC       false
#define LORA_DCH_CRC         false
   
#define PACKET_CRC_SIZE 2u
#define PHY_DCH_PACKET_SIZE       (HEARTBEAT_SIZE_BYTES + PACKET_CRC_SIZE)
#define PHY_RACH_PACKET_SIZE      (DATA_MESSAGE_SIZE_BYTES + PACKET_CRC_SIZE)
#define PHY_RACH_ACK_PACKET_SIZE  (ACKNOWLEDGEMENT_SIZE_BYTES + PACKET_CRC_SIZE)
#define PHY_DLCCH_PACKET_SIZE     (DATA_MESSAGE_SIZE_BYTES + PACKET_CRC_SIZE)

#endif  /* USE_LORA_CHIP_CRC */

#define NCU_NETWORK_ADDRESS 0

#define COMMAND_READ 0
#define COMMAND_WRITE 1

#define KILL_COMMAND 666

#define RSSI_SIGNED_BITS 0xFE00  //Logical OR with Rx'd RSSI in Status Indication Message to get actual RSSI

/**************************************************************************************/ 

/* Application Layer Message Types */
typedef enum
{
   APP_MSG_TYPE_FIRE_SIGNAL_E,
   APP_MSG_TYPE_ALARM_SIGNAL_E,
   APP_MSG_TYPE_FAULT_SIGNAL_E,
   APP_MSG_TYPE_OUTPUT_SIGNAL_E,
   APP_MSG_TYPE_COMMAND_E,
   APP_MSG_TYPE_RESPONSE_E,                     //5
   APP_MSG_TYPE_LOGON_E,
   APP_MSG_TYPE_STATUS_INDICATION_E,
   APP_MSG_TYPE_APP_FIRMWARE_E,
   APP_MSG_TYPE_ROUTE_ADD_E,
   APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E,           //10
   APP_MSG_TYPE_ROUTE_DROP_E,
   APP_MSG_TYPE_TEST_MODE_E,
   APP_MSG_TYPE_TEST_SIGNAL_E,
   APP_MSG_TYPE_STATE_SIGNAL_E,
   APP_MSG_TYPE_LOAD_BALANCE_E,                 //15
   APP_MSG_TYPE_LOAD_BALANCE_RESPONSE_E,
   APP_MSG_TYPE_ACK_E,
   APP_MSG_TYPE_HEARTBEAT_E,
   APP_MSG_TYPE_RBU_DISABLE_E,
   APP_MSG_TYPE_STATUS_SIGNAL_E,                //20
   APP_MSG_TYPE_UNKNOWN_E,
   APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E,
   APP_MSG_TYPE_PING_E,
   APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E,
   APP_MSG_TYPE_DAY_NIGHT_STATUS_E,             //25
   APP_MSG_TYPE_ZONE_ENABLE_E,
   APP_MSG_TYPE_AT_COMMAND_E,
   APP_MSG_TYPE_PPU_MODE_E,
   APP_MSG_TYPE_PPU_COMMAND_E,
   APP_MSG_TYPE_GLOBAL_DELAYS_E,                //30
   APP_MSG_TYPE_RESET_E,                //last message ID that fits in the 5 bits of an OTA data message
   APP_MSG_TYPE_ADD_NODE_LINK_E,
   APP_MSG_TYPE_DROP_NODE_LINK_E,
   APP_MSG_CHECK_TDM_SYNCH_E,
	 APP_MSG_TYPE_EXIT_TEST_MODE_E,   //35
   APP_MSG_TYPE_MAX_E
} ApplicationLayerMessageType_t;

typedef enum
{
   PARAM_TYPE_ANALOGUE_VALUE_E,              //0
   PARAM_TYPE_NEIGHBOUR_INFO_E,              //1
   PARAM_TYPE_STATUS_FLAGS_E,                //2
   PARAM_TYPE_DEVICE_COMBINATION_E,          //3
   PARAM_TYPE_ALARM_THRESHOLD_E,             //4
   PARAM_TYPE_PRE_ALARM_THRESHOLD_E,         //5
   PARAM_TYPE_FAULT_THRESHOLD_E,             //6
   PARAM_TYPE_FLASH_RATE_E,                  //7
   PARAM_TYPE_TONE_SELECTION_E,              //8
   PARAM_TYPE_RBU_SERIAL_NUMBER_E,           //9
   PARAM_TYPE_PLUGIN_SERIAL_NUMBER_E,        //10
   PARAM_TYPE_RBU_ENABLE_E,                  //11
   PARAM_TYPE_PLUGIN_ENABLE_E,               //12
   PARAM_TYPE_MODULATION_BANDWIDTH_E,        //13
   PARAM_TYPE_SPREADING_FACTOR_E,            //14
   PARAM_TYPE_FREQUENCY_E,                   //15
   PARAM_TYPE_CODING_RATE_E,                 //16
   PARAM_TYPE_TX_POWER_E,                    //17
   PARAM_TYPE_TEST_MODE_E,                   //18
   PARAM_TYPE_PLUGIN_TEST_MODE_E,            //19
   PARAM_TYPE_FIRMWARE_INFO_E,               //20
   PARAM_TYPE_FIRMWARE_ACTIVE_IMAGE_E,       //21
   PARAM_TYPE_REBOOT_E,                      //22
   PARAM_TYPE_SVI_E,                         //23
   PARAM_TYPE_RBU_DISABLE_E,                 //24
   PARAM_TYPE_TX_POWER_LOW_E,                //25
   PARAM_TYPE_TX_POWER_HIGH_E,               //26
   PARAM_TYPE_OUTPUT_STATE_E,                //27
   PARAM_TYPE_ZONE_E,                        //28
   PARAM_TYPE_CRC_INFORMATION_E,             //29
   PARAM_TYPE_PIR_RESET_E,                   //30
   PARAM_TYPE_EEPROM_INFORMATION_E,          //31
   PARAM_TYPE_FAULT_TYPE_E,                  //32
   PARAM_TYPE_MESH_STATUS_E,                 //33
   PARAM_TYPE_INDICATOR_LED_E,               //34
   PARAM_TYPE_PLUGIN_ID_NUMBER_E,            //35
   PARAM_TYPE_PLUGIN_FIRMWARE_INFO_E,        //36
   PARAM_TYPE_DAY_NIGHT_SETTING_E,           //37
   PARAM_TYPE_ALARM_ACK_E,                   //38
   PARAM_TYPE_EVACUATE_E,                    //39
   PARAM_TYPE_GLOBAL_DELAY_E,                //40
   PARAM_TYPE_ENABLE_DISABLE_E,              //41
   PARAM_TYPE_ALARM_RESET_E,                 //42
   PARAM_TYPE_SOUND_LEVEL_E,                 //43
   PARAM_TYPE_TEST_ONE_SHOT_E,               //44
   PARAM_TYPE_CONFIRMED_E,                   //45
   PARAM_TYPE_ALARM_CONFIG_E,                //46
   PARAM_TYPE_ALARM_OPTION_FLAGS_E,          //47
   PARAM_TYPE_ALARM_DELAYS_E,                //48
   PARAM_TYPE_CHANNEL_FLAGS_E,               //49
   PARAM_TYPE_MAX_RANK_E,                    //50
   PARAM_TYPE_200_HOUR_TEST_E,               //51
   PARAM_TYPE_CHECK_FIRMWARE_E,              //52
   PARAM_TYPE_SET_LOCAL_GLOBAL_DELAYS_E,     //53
   PARAM_TYPE_GLOBAL_ALARM_DELAY_VALUES_E,   //54
   PARAM_TYPE_GLOBAL_ALARM_OVERRIDE_E,       //55
   PARAM_TYPE_BATTERY_STATUS_REQUEST_E,      //56
   PARAM_TYPE_GLOBAL_DELAY_COMBINED_E,       //57
   PARAM_TYPE_PLUGIN_DETECTOR_TYPE_E,        //58
   PARAM_TYPE_PLUGIN_TYPE_AND_CLASS_E,       //59
   PARAM_TYPE_SCAN_FOR_DEVICES_E,            //60
   PARAM_TYPE_SENSOR_VALUES_REQUEST_E,       //61
   PARAM_TYPE_DELAYED_OUTPUT_E,              //62
   PARAM_TYPE_BATTERY_TEST_E,                //63
   PARAM_TYPE_PRODUCT_CODE_E,                //64
   PARAM_TYPE_PPU_MODE_ENABLE_E,             //65
   PARAM_TYPE_DEPASSIVATION_SETTINGS_E,      //66
   PARAM_TYPE_ENTER_PPU_MODE_E,              //67
   PARAM_TYPE_ENABLE_FAULTS_CLEAR_MSG_E,     //68
   PARAM_TYPE_EXIT_TEST_MODE_E,
   PARAM_TYPE_MAX_E
} ParameterType_t;

/* Debug GUI event types */
typedef enum
{
   GUI_EVENT_ADDED_PRIMARY_PARENT_E,                 //0
   GUI_EVENT_DROPPED_PRIMARY_PARENT_E,               //1
   GUI_EVENT_ADDED_SECONDARY_PARENT_E,               //2
   GUI_EVENT_DROPPED_SECONDARY_PARENT_E,             //3
   GUI_EVENT_ADDED_CHILD_E,                          //4
   GUI_EVENT_DROPPED_CHILD_E,                        //5
   GUI_EVENT_PROMOTED_SECONDARY_PARENT_E,            //6
   GUI_EVENT_PROMOTED_PRIMARY_TRACKING_NODE_E,       //7
   GUI_EVENT_PROMOTED_SECONDARY_TRACKING_NODE_E,     //8
   GUI_EVENT_MOVED_TO_NEW_STATE_E,                   //9
   GUI_EVENT_MISSED_HEARTBEAT_E,                     //10
   GUI_EVENT_CAD_TIMEOUT_E,                          //11
   GUI_EVENT_RX_TIMEOUT_E,                           //12
   GUI_EVENT_RX_ERROR_E                              //13
}GuiEvent_t;

/* Test mode types
*************************************************************************************/
typedef enum
{
   MC_MAC_TEST_MODE_OFF_E,                         //0
   MC_MAC_TEST_MODE_RECEIVE_E,                     //1
   MC_MAC_TEST_MODE_TRANSPARENT_E,                 //2
   MC_MAC_TEST_MODE_TRANSMIT_E,                    //3
   MC_MAC_TEST_MODE_MONITORING_E,                  //4
   MC_MAC_TEST_MODE_SLEEP_E,                       //5
   MC_MAC_TEST_MODE_NETWORK_MONITOR_E,             //6
   MC_MAC_TEST_MODE_MAX_E
} MC_MAC_TestMode_t;

/* HKD-16-0015-D_B Mesh Protocol Design
 * Table 32: List of Status Flags 
 */
typedef enum
{
   CO_INSTALLATION_TAMPER_FAULT_E = 0x01u,
   CO_DISMANTLE_TAMPER_FAULT_E    = 0x02u,
   CO_BATTERY_WARNING_E           = 0x04u,
   CO_DETECTOR_FAULT_E            = 0x8u,
   CO_BEACON_FAULT_E              = 0x10u,
   CO_SOUNDER_FAULT_E             = 0x20u,
   CO_IO_INPUT_1_FAULT_E          = 0x40u,
   CO_IO_INPUT_2_FAULT_E          = 0x80u,
   CO_DISMANTLE_HEAD_FAULT_E      = 0x100u,
   CO_MISMATCH_HEAD_FAULT_E       = 0x200u,
   CO_BATTERY_ERROR_E             = 0x400u,
   CO_DEVICE_ID_MISMATCH_E        = 0x800u,
   CO_DIRTY_SENSOR_E              = 0x1000u,
   CO_INTERNAL_FAULT_E            = 0x2000u,
   CO_INPUT_SHORT_CIRCUIT_FAULT_E = 0x4000u,
   CO_INPUT_OPEN_CIRCUIT_FAULT_E  = 0x8000u,
   CO_OUTPUT_FAULT_E              = 0x10000u,
   CO_BIT_FAULT_E                 = 0x20000u,
   CO_FAULT_MASK_E                = 0x3FFFFu     //For masking out unused fault bits
} CO_FaultId_t;

//Fault Signal - fault type
typedef enum
{
   FAULT_SIGNAL_NO_ERROR_E,                     //0
   FAULT_SIGNAL_TYPE_MISMATCH_E,                //1
   FAULT_SIGNAL_DEVICE_ID_MISMATCH_E,           //2
   FAULT_SIGNAL_FAULTY_SENSOR_E,                //3
   FAULT_SIGNAL_DIRTY_SENSOR_E,                 //4
   FAULT_SIGNAL_FAULTY_DIRTY_SENSOR_E,          //5
   FAULT_SIGNAL_INTERNAL_FAULT_E,               //6
   FAULT_SIGNAL_SOUNDER_FAULT_E,                //7
   FAULT_SIGNAL_BEACON_LED_FAULT_E,             //8
   FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E,    //9
   FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E,     //10
   FAULT_SIGNAL_OUTPUT_FAULT_E,                 //11
   FAULT_SIGNAL_INSTALLATION_TAMPER_E,          //12
   FAULT_SIGNAL_DISMANTLE_TAMPER_E,             //13
   FAULT_SIGNAL_LOW_BATTERY_E,                  //14
   FAULT_SIGNAL_BATTERY_ERROR_E,                //15
   FAULT_SIGNAL_LOW_LINK_QUALITY_E,             //16
   FAULT_SIGNAL_LOW_PARENT_COUNT_FAULT_E,       //17
   FAULT_SIGNAL_DEVICE_DROPPED_E,               //18
   FAULT_SIGNAL_HEAD_REMOVED_E,                 //19
   FAULT_SIGNAL_FIRMWARE_VERSION_E,             //20
   FAULT_SIGNAL_MAX_E
} FaultSignalType_t;

typedef enum
{
   BIT_SOURCE_NONE_E,
   BIT_SOURCE_RADIO_E,
   BIT_SOURCE_SVI_E,
   BIT_SOURCE_PLUGIN_E,
   BIT_SOURCE_MAX_E
}BITFaultSource_t;

//Fault Signal - fault type
typedef enum
{
   CO_DETECTOR_NO_FAULT_E,
   CO_DETECTOR_TYPE_MISMATCH_E,
   CO_DETECTOR_ID_MISMATCH_E,
   CO_DETECTOR_SENSOR_FAULT_E,
   CO_DETECTOR_DIRTY_SENSOR_E,
   CO_DETECTOR_DIRTY_AND_SENSOR_FAULT_E,
   CO_DETECTOR_INTERNAL_FAULT_E
} CO_DetectorFault_t;

typedef enum f
{
   CO_BEACON_NO_FAULT_E,
   CO_BEACON_TYPE_MISMATCH_E,
   CO_BEACON_ID_MISMATCH_E,
   CO_BEACON_LED_FAULT_E
} CO_BeaconFault_t;

typedef enum 
{
   CO_SOUNDER_NO_FAULT_E,
   CO_SOUNDER_TYPE_MISMATCH_E,
   CO_SOUNDER_ID_MISMATCH_E,
   CO_SOUNDER_INTERNAL_FAULT_E
} CO_SounderFault_t;

typedef enum 
{
   CO_RESET_REASON_UNKNOWN_E,
   CO_RESET_REASON_NO_PARENTS_LEFT_E,
   CO_RESET_REASON_DROPPED_BY_SYNC_NODE_E,
   CO_RESET_REASON_NODE_STOPPED_RESPONDING_E,
   CO_RESET_REASON_MAX_CONNECTION_ATTEMPTS_E,
   CO_RESET_REASON_FAILED_TO_SEND_ROUTE_ADD_E,
   CO_RESET_REASON_LINK_REMAKE_FAILED_E,
   CO_RESET_REASON_FREE_RUNNING_E,
   CO_RESET_REASON_CIRCULAR_PATH_E,
   CO_RESET_REASON_RESET_COMMAND_E,
   CO_RESET_REASON_PPU_MODE_REQUESTED_E,
} ResetReason_t;

typedef enum
{
   CO_MESH_EVENT_NONE_E, //just a status update, not initiated by an event
   CO_MESH_EVENT_CHILD_NODE_ADDED_E,
   CO_MESH_EVENT_CHILD_NODE_DROPPED_E,
   CO_MESH_EVENT_PRIMARY_PARENT_ADDED_E,
   CO_MESH_EVENT_PRIMARY_PARENT_DROPPED_E,
   CO_MESH_EVENT_SECONDARY_PARENT_ADDED_E,
   CO_MESH_EVENT_SECONDARY_PARENT_DROPPED_E,
   CO_MESH_EVENT_PRIMARY_TRACKING_NODE_ADDED_E,
   CO_MESH_EVENT_PRIMARY_TRACKING_NODE_DROPPED_E,
   CO_MESH_EVENT_SECONDARY_TRACKING_NODE_ADDED_E,
   CO_MESH_EVENT_SECONDARY_TRACKING_NODE_DROPPED_E,
   CO_MESH_EVENT_SECONDARY_PARENT_PROMOTED_E,
   CO_MESH_EVENT_PRIMARY_TRACKING_NODE_PROMOTED_E,
   CO_MESH_EVENT_REPORT_NODES_E,
   CO_MESH_EVENT_ZONE_CHANGE_E,
   CO_MESH_EVENT_STATE_CHANGE_E,
   CO_MESH_EVENT_BATTERY_STATUS_E,
   CO_MESH_EVENT_APPLICATION_REQUEST_E,
   CO_MESH_EVENT_INITIAL_TRK_NODE_LOST_E,
   CO_MESH_EVENT_DAY_NIGHT_CHANGE_E,
   CO_MESH_EVENT_SYNCH_LOSS_E,
   CO_MESH_EVENT_COMMANDED_RESET_E,
   CO_MESH_EVENT_PPU_MODE_REQUESTED_E,
   CO_MESH_EVENT_COMMS_FAIL_E
}CO_MeshEvent_t;  /* if you change this list, update MESH_EVENT_STR in MM_NCUApplicationStub.c */

/* message types */
typedef enum
{
   CO_MESSAGE_GENERATE_HEARTBEAT_E,
   CO_MESSAGE_PHY_DATA_IND_E,
   CO_MESSAGE_PHY_DATA_REQ_E,
   CO_MESSAGE_HEARTBEAT_E,
   CO_MESSAGE_GENERATE_FIRE_SIGNAL_E,
   CO_MESSAGE_GENERATE_ALARM_SIGNAL_E,            //5
   CO_MESSAGE_GENERATE_OUTPUT_SIGNAL_E,
   CO_MESSAGE_GENERATE_LOGON_REQ,
   CO_MESSAGE_GENERATE_FAULT_SIGNAL_E,
   CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E,
   CO_MESSAGE_GENERATE_RESPONSE_SIGNAL_E,         //10
   CO_MESSAGE_MAC_EVENT_E,
   CO_MESSAGE_GENERATE_TEST_MESSAGE_E,
   CO_MESSAGE_GENERATE_SET_STATE_E,
   CO_MESSAGE_GENERATE_ROUTE_ADD_RESPONSE_E,
   CO_MESSAGE_GENERATE_ROUTE_ADD_REQUEST_E,       //15
   CO_MESSAGE_GENERATE_RBU_DISABLE_MESSAGE_E,
   CO_MESSAGE_GENERATE_RBU_REPORT_MESSAGE_E,
   CO_MESSAGE_GENERATE_STATUS_INDICATION_E,
   CO_MESSAGE_GENERATE_ROUTE_DROP_REQUEST_E,
   CO_MESSAGE_RUN_BUILT_IN_TESTS_E,               //20
   CO_MESSAGE_MESH_STATUS_EVENT_E,
   CO_MESSAGE_TIMER_EVENT_E,
   CO_MESSAGE_GENERATE_PPU_DEV_LIST_E,
   CO_MESSAGE_BROADCAST_PP_FREQ_E,
   CO_MESSAGE_BROADCAST_PP_SYS_ID_E,               //25
   CO_MESSAGE_BROADCAST_PP_MODE_REQ_E,
   CO_MESSAGE_CONFIRMATION_E,
   CO_MESSAGE_CIE_COMMAND_E,
   CO_MESSAGE_SERVICE_TX_BUFFER_QUEUE_E,
   CO_MESSAGE_APPLICATION_REQUEST_E,               //30
   CO_MESSAGE_GENERATE_ALARM_OUTPUT_STATE_E,
   CO_MESSAGE_GENERATE_PING_REQUEST_E,
   CO_MESSAGE_GENERATE_BATTERY_STATUS_E,
   CO_MESSAGE_BATTERY_STATUS_MESSAGE_E,
   CO_MESSAGE_RESET_EVENT_E,                       //35
   CO_MESSAGE_GENERATE_ZONE_ENABLE_E,
   CO_MESSAGE_SET_OUTPUT_E,
   CO_MESSAGE_REQUEST_SENSOR_DATA_E,
   CO_MESSAGE_SENSOR_DATA_E,
   CO_MESSAGE_GENERATE_ATTX_COMMAND_E,             //40
   CO_MESSAGE_GENERATE_ATTX_RESPONSE_E,
   CO_MESSAGE_GENERATE_START_OTA_AT_MODE_E,
   CO_MESSAGE_GENERATE_PPU_MODE_REQUEST_E,
   CO_MESSAGE_PPU_MODE_REQUEST_E,
   CO_MESSAGE_PPU_COMMAND_E,                       //45
   CO_MESSAGE_PPU_RESPONSE_E,
   CO_MESSAGE_NVM_CLEANUP_E,
	CO_MESSAGE_TRIGGER_BATTERY_CHECKS_E,
   CO_MESSAGE_MAX_E
} CO_MessageType_t;

/* Head Types */
typedef enum
{
   CO_NONE_E,
   CO_SMOKE_E,
   CO_HEAT_A1R_E,
   CO_HEAT_B_E,
   CO_SMOKE_AND_HEAT_A1R_E,
   CO_SMOKE_AND_PIR_E,
   CO_PIR_E,
   CO_FIRE_CALLPOINT_E,
   CO_FIRST_AID_CALLPOINT_E,
   CO_SOUNDER_E,
   CO_BEACON_W_2_4_7_5_E,
   CO_BEACON_W_3_1_11_3_E,
   CO_BEACON_C_3_8_9_E,
   CO_BEACON_C_3_15_E,
   CO_VISUAL_INDICATOR_E,
   CO_REMOTE_INDICATOR_E,
   CO_IO_UNIT_INPUT_1_E,
   CO_IO_UNIT_OUTPUT_1_E,
   CO_IO_UNIT_INPUT_2_E,
   CO_IO_UNIT_OUTPUT_2_E
} CO_RBUSensorType_t;

/* RU Channel mapping */
typedef enum
{
   CO_CHANNEL_NONE_E,                                 // 0
   CO_CHANNEL_SMOKE_E,                                // 1
   CO_CHANNEL_HEAT_B_E,                               // 2
   CO_CHANNEL_CO_E,                                   // 3
   CO_CHANNEL_PIR_E,                                  // 4
   CO_CHANNEL_SOUNDER_E,                              // 5
   CO_CHANNEL_BEACON_E,                               // 6
   CO_CHANNEL_FIRE_CALLPOINT_E,                       // 7
   CO_CHANNEL_STATUS_INDICATOR_LED_E,                 // 8
   CO_CHANNEL_VISUAL_INDICATOR_E,                     // 9
   CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E,    // 10
   CO_CHANNEL_MEDICAL_CALLPOINT_E,                    // 11
   CO_CHANNEL_EVAC_CALLPOINT_E,                       // 12
   CO_CHANNEL_OUTPUT_ROUTING_E,                       // 13
   CO_CHANNEL_INPUT_1_E,
   CO_CHANNEL_INPUT_2_E,                              // 15
   CO_CHANNEL_INPUT_3_E,
   CO_CHANNEL_INPUT_4_E,
   CO_CHANNEL_INPUT_5_E,
   CO_CHANNEL_INPUT_6_E,
   CO_CHANNEL_INPUT_7_E,                              // 20
   CO_CHANNEL_INPUT_8_E,
   CO_CHANNEL_INPUT_9_E,
   CO_CHANNEL_INPUT_10_E,
   CO_CHANNEL_INPUT_11_E,
   CO_CHANNEL_INPUT_12_E,                             // 25
   CO_CHANNEL_INPUT_13_E,
   CO_CHANNEL_INPUT_14_E,
   CO_CHANNEL_INPUT_15_E,
   CO_CHANNEL_INPUT_16_E,
   CO_CHANNEL_INPUT_17_E,                             // 30
   CO_CHANNEL_INPUT_18_E,
   CO_CHANNEL_INPUT_19_E,
   CO_CHANNEL_INPUT_20_E,
   CO_CHANNEL_INPUT_21_E,
   CO_CHANNEL_INPUT_22_E,                             // 35
   CO_CHANNEL_INPUT_23_E,
   CO_CHANNEL_INPUT_24_E,
   CO_CHANNEL_INPUT_25_E,
   CO_CHANNEL_INPUT_26_E,
   CO_CHANNEL_INPUT_27_E,                             //40
   CO_CHANNEL_INPUT_28_E,
   CO_CHANNEL_INPUT_29_E,
   CO_CHANNEL_INPUT_30_E,
   CO_CHANNEL_INPUT_31_E,
   CO_CHANNEL_INPUT_32_E,                             //45
   CO_CHANNEL_OUTPUT_1_E,
   CO_CHANNEL_OUTPUT_2_E,
   CO_CHANNEL_OUTPUT_3_E,
   CO_CHANNEL_OUTPUT_4_E,
   CO_CHANNEL_OUTPUT_5_E,                             //50
   CO_CHANNEL_OUTPUT_6_E,
   CO_CHANNEL_OUTPUT_7_E,
   CO_CHANNEL_OUTPUT_8_E,
   CO_CHANNEL_OUTPUT_9_E,
   CO_CHANNEL_OUTPUT_10_E,                            //55
   CO_CHANNEL_OUTPUT_11_E,
   CO_CHANNEL_OUTPUT_12_E,
   CO_CHANNEL_OUTPUT_13_E,
   CO_CHANNEL_OUTPUT_14_E,
   CO_CHANNEL_OUTPUT_15_E,                            // 60
   CO_CHANNEL_OUTPUT_16_E,
   CO_CHANNEL_HEAT_A1R_E,
   CO_CHANNEL_RADIO_E,
   CO_CHANNEL_WATER_LEAK_E,
   CO_CHANNEL_MAX_E                                   //65
} CO_ChannelIndex_t;

typedef enum
{
   CO_PROFILE_FIRE_E,
   CO_PROFILE_FIRST_AID_E,
   CO_PROFILE_EVACUATE_E,
   CO_PROFILE_SECURITY_E,
   CO_PROFILE_GENERAL_E,
   CO_PROFILE_FAULT_E,
   CO_PROFILE_ROUTING_ACK_E,
   CO_PROFILE_TEST_E,
   CO_PROFILE_SILENT_TEST_E,
   CO_PROFILE_MAX_E
} CO_OutputProfile_t;

typedef enum
{
   CO_DAY_E,
   CO_NIGHT_E,
   CO_NUM_DAYNIGHT_SETTINGS_E
} CO_DayNight_t;

typedef enum
{
   CO_DISABLE_NONE_E,
   CO_DISABLE_CHANNEL_E,
   CO_DISABLE_DEVICE_E,
   CO_DISABLE_ZONE_E,
   CO_DISABLE_ALL_E,
   CO_DISABLE_MAX_E
} CO_DisableRegion_t;

typedef enum
{
   CO_RACH_CHANNEL_UPDATE_E,
   CO_MISSING_HEARTBEAT_E,
   CO_TEST_MODE_MESSAGE_E,
   CO_NEW_LONG_FRAME_E,
   CO_NEW_SHORT_FRAME_E,
   CO_REPORT_NODES_E,
   CO_CORRUPTED_TX_MESSAGE_E,
   CO_CORRUPTED_RX_MESSAGE_E,
   CO_POWER_STATUS_E,
   CO_ENTER_PPU_OTA_MODE_E,
   CO_JOINING_PHASE_COMPLETE_E
} CO_MACEventTypeType_t;

typedef enum
{
   CO_RXTE_TIMEOUT_E = 'x',
   CO_RXTE_ERROR_E = 'e',
   CO_RXTE_CORRUPT_E = 'c'
} CO_RxTimeoutError_t;

typedef enum
{
   HEAD_WRITE_ENABLED_E,
   HEAD_WRITE_PREALARM_THRESHOLD_E,
   HEAD_WRITE_ALARM_THRESHOLD_E,
   HEAD_WRITE_FAULT_THRESHOLD_E,
   HEAD_WRITE_FLASH_RATE_E,
   HEAD_WRITE_TONE_SELECTION_E,
   HEAD_WRITE_TEST_MODE_E,
   HEAD_WRITE_OUTPUT_ACTIVATED_E,
   HEAD_WRITE_INDICATOR_LED_E,
   HEAD_WRITE_IDENTIFICATION_NUMBER_E,
   HEAD_READ_DEVICE_TYPE_E,
   HEAD_READ_DEVICE_TYPE_AND_CLASS_E,
   HEAD_READ_ENABLED_E,
   HEAD_READ_PREALARM_THRESHOLD_E,
   HEAD_READ_ALARM_THRESHOLD_E,
   HEAD_READ_FAULT_THRESHOLD_E,
   HEAD_READ_FLASH_RATE_E,
   HEAD_READ_TONE_SELECTION_E,
   HEAD_READ_TEST_MODE_E,
   HEAD_READ_ANALOGUE_VALUE_E,
   HEAD_READ_SERIAL_NUMBER_E,
   HEAD_READ_FIRMWARE_VERSION_E,
   HEAD_READ_INDICATOR_LED_E,
   HEAD_READ_FAULT_TYPE_E,
   HEAD_READ_IDENTIFICATION_NUMBER_E,
   HEAD_EVENT_PREALARM_THRESHOLD_EXCEEDED_E,
   HEAD_EVENT_ALARM_THRESHOLD_EXCEEDED_E,
   HEAD_EVENT_FAULT_THRESHOLD_EXCEEDED_E,
   HEAD_RESET_PIR_E,
   HEAD_DAY_NIGHT_SETTING_E,
   HEAD_WAKE_UP_SIGNAL_E,
   HEAD_SLEEP_MODE_E,
   HEAD_INTERNAL_MESSAGE_E
} HeadMessageType_t;

typedef enum
{
   APP_CONF_FIRE_SIGNAL_E,
   APP_CONF_ALARM_SIGNAL_E,
   APP_CONF_OUTPUT_SIGNAL_E,
   APP_CONF_LOGON_SIGNAL_E,
   APP_CONF_STATUS_SIGNAL_E,
   APP_CONF_COMMAND_SIGNAL_E,
   APP_CONF_RESPONSE_SIGNAL_E,
   APP_CONF_SET_STATE_E,
   APP_CONF_RBU_DISABLE_E,
   APP_CONF_ALARM_OUTPUT_STATE_SIGNAL_E,
   APP_CONF_BATTERY_STATUS_SIGNAL_E,
   APP_CONF_FAULT_SIGNAL_E,
   APP_CONF_ZONE_ENABLE_MESSAGE_E,
   APP_CONF_ROUTE_DROP_SIGNAL_E,
} AppConfirmationType_t;

typedef enum
{
   CIE_CMD_READ_MESSAGE_QUEUE_STATUS_E,
   CIE_CMD_READ_ZONES_AND_DEVICES_E,
   CIE_CMD_READ_FIRE_MESSAGE_E,
   CIE_CMD_READ_ALARM_MESSAGE_E,
   CIE_CMD_READ_FAULT_MESSAGE_E,
   CIE_CMD_READ_MISC_MESSAGE_E,
   CIE_CMD_DISCARD_FIRE_MESSAGE_E,
   CIE_CMD_DISCARD_ALARM_MESSAGE_E,
   CIE_CMD_DISCARD_FAULT_MESSAGE_E,
   CIE_CMD_DISCARD_MISC_MESSAGE_E,
   CIE_CMD_RESET_MESSAGE_QUEUE_E,
   CIE_CMD_RESET_SELECTED_QUEUE_E,
   CIE_CMD_READ_DEVICE_COMBINATION_E,
   CIE_CMD_READ_DEVICE_STATUS_FLAGS_E,
   CIE_CMD_READ_DEVICE_MESH_STATUS_E,
   CIE_CMD_READ_DEVICE_SERIAL_NUMBER_E,
   CIE_CMD_READ_DEVICE_FIRMWARE_VERSION_E,
   CIE_CMD_READ_DEVICE_NEIGHBOUR_INFORMATION_E,
   CIE_CMD_REBOOT_UNIT_E,
   CIE_CMD_SET_TEST_MODE_E,
   CIE_CMD_READ_ANALOGUE_VALUE_E,
   CIE_CMD_BEACON_FLASH_RATE_E,
   CIE_CMD_DEVICE_ENABLE_E,
   CIE_CMD_READ_PLUGIN_SERIAL_NUMBER_E,
   CIE_CMD_SOUNDER_TONE_SELECTION_E,
   CIE_CMD_UPPER_THRESHOLD_SETTING_E,
   CIE_CMD_LOWER_THRESHOLD_SETTING_E,
   CIE_CMD_PLUGIN_TEST_MODE_E,
   CIE_CMD_PLUGIN_FIRMWARE_VERSION_E,
   CIE_CMD_SET_DAY_NIGHT_E,
   CIE_CMD_PLUGIN_LED_SETTING_E,
   CIE_CMD_ACKNOWLEDGE_E,
   CIE_CMD_EVACUATE_E,
   CIE_CMD_GLOBAL_DELAYS_E,
   CIE_CMD_ENABLE_DISABLE_E,
   CIE_CMD_RESET_ALARMS_E,
   CIE_CMD_SOUND_LEVEL_E,
   CIE_CMD_TEST_ONE_SHOT_E,
   CIE_CMD_CONFIRM_E,
   CIE_CMD_ALARM_CONFIG_E,
   CIE_CMD_ALARM_OPTION_FLAGS_E,
   CIE_CMD_ALARM_DELAYS_E,
   CIE_CMD_CHANNEL_FLAGS_E,
   CIE_CMD_MAX_RANK_E,
   CIE_CMD_VERIFY_OUTPUTS_E,
   CIE_CMD_ENABLE_200_HOUR_TEST_E,
   CIE_CMD_CHECK_FIRMWARE_E,
   CIE_CMD_SET_LOCAL_OR_GLOBAL_ALARM_DELAYS_E,
   CIE_CMD_SET_GLOBAL_ALARM_DELAY_VALUES_E,
   CIE_CMD_SET_GLOBAL_ALARM_DELAY_OVERRIDE_E,
   CIE_CMD_REQUEST_BATTERY_STATUS_E,
   CIE_CMD_SET_GLOBAL_DELAY_COMBINED_E,
   CIE_CMD_READ_PLUGIN_TYPE_AND_CLASS_E,
   CIE_CMD_SCAN_FOR_DEVICES_E,
   CIE_CMD_REQUEST_SENSOR_VALUES_E,
   CIE_CMD_DELAYED_OUTPUT_E,
   CIE_CMD_BATTERY_TEST_E,
   CIE_CMD_PRODUCT_CODE_E,
   CIE_CMD_PPU_MODE_ENABLE_E, //old PPEN
   CIE_CMD_DEPASSIVATION_SETTINGS_E,
   CIE_CMD_ENTER_PPU_MODE_E, // Enter PPU Disconnected mode
   CIE_CMD_ENABLE_FAULTS_CLEAR_MSG_E,
	 CIE_CMD_EXIT_TEST_MODE_E,
   CIE_CMD_MAX_E,
} CIECommandType_t;

typedef enum
{
   CIE_Q_FIRE_E,
   CIE_Q_FIRE_LOST_COUNT_E,
   CIE_Q_ALARM_E,
   CIE_Q_ALARM_LOST_COUNT_E,
   CIE_Q_FAULT_E,
   CIE_Q_FAULT_LOST_COUNT_E,
   CIE_Q_MISC_E,
   CIE_Q_MISC_LOST_COUNT_E,
   CIE_Q_TX_BUFFER_E,
   CIE_Q_OUTPUT_SIGNAL_E,
   CIE_Q_OUTPUT_SIGNAL_PRIORITY_E,
   CIE_Q_MAX_E
} CIEQueueType_t;

typedef enum 
{
   FINDX_RBU_MAIN_IMAGE_E,
   FINDX_RBU_BACKUP_IMAGE_E,
   FINDX_EXTERNAL_INTERFACE_1_HEAD_E,
   FINDX_EXTERNAL_INTERFACE_2_E,
   FINDX_EXTERNAL_INTERFACE_3_E,
   FINDX_INTTERNAL_INTERFACE_1_I2C_E,
   FINDX_INTTERNAL_INTERFACE_2_E,
   FINDX_INTTERNAL_INTERFACE_3_E,
   FINDX_MAX_E
} FirmwareIndex_t; 

typedef enum 
{
   FLASH_RATE_2_E,  //2 per second
   FLASH_RATE_1_E,  //1 per second
   FLASH_RATE_MAX_E
}FlashRate_t;

//SVI sounder and led ring
typedef enum
{
   CONST_SOUNDER_SILENCED_E,
   CONST_SOUNDER_CONTINUOUS_E,
   CONST_SOUNDER_PULSE_E        //1 second on, 7 seconds off.
} ConstructionSounderProfile_t;

typedef enum
{
   DM_SVI_TONE_0_E,
   DM_SVI_TONE_1_E,
   DM_SVI_TONE_2_E,
   DM_SVI_TONE_3_E,
   DM_SVI_TONE_4_E,
   DM_SVI_TONE_5_E,
   DM_SVI_TONE_SECURITY_E,
   DM_SVI_TONE_7_E,
   DM_SVI_TONE_EVACUATE_E,    // ramp 500-1200Hz in 0.5s, off for 0.5s
   DM_SVI_TONE_9_E,
   DM_SVI_TONE_10_E,
   DM_SVI_TONE_11_E,
   DM_SVI_TONE_12_E,
   DM_SVI_TONE_13_E,
   DM_SVI_TONE_FIRE_E,        // Square 800Hz/970Hz @ 2Hz
   DM_SVI_TONE_15_E,
   DM_SVI_TONE_16_E,
   DM_SVI_TONE_17_E,
   DM_SVI_TONE_18_E,
   DM_SVI_TONE_19_E,
   DM_SVI_TONE_20_E,
   DM_SVI_TONE_21_E,
   DM_SVI_TONE_22_E,
   DM_SVI_TONE_23_E,
   DM_SVI_TONE_24_E,
   DM_SVI_TONE_25_E,
   DM_SVI_TONE_26_E,
   DM_SVI_TONE_27_E,
   DM_SVI_TONE_28_E,
   DM_SVI_TONE_29_E,
   DM_SVI_TONE_30_E,
   DM_SVI_TONE_31_E,
   DM_SVI_TONE_32_E,
   DM_SVI_TONE_FIRST_AID_E,   // 33  // 1s pulse every 8s
   DM_SVI_TONE_SILENT_E,      // 34  AOS Silent Profile
   DM_SVI_TONE_TEST_E,        // 35  AOS Test Profile
   DM_SVI_TONE_36_E,
   DM_SVI_TONE_MAX_E
} DM_ToneSelectionType_t;

typedef enum 
{
   SEND_NORMAL_E,             //use the back-off mechanism when sending messages
   SEND_ONCE_E,               //Send once then discard
   SEND_TWO_E,                //use the back-off mechanism to send up to two pings then discard;
   SEND_THREE_E,              //use the back-off mechanism to send up to three pings then discard;
   SEND_SACH_ONLY_E           //Don't use back-off.  Put back in SACH queue and discard local copy.
} SendBehaviour_t;

/* Zone enablement bits */
typedef enum
{
   ZONE_BITS_LOW_E,
   ZONE_BITS_HIGH_E,
   ZONE_BITS_MAX_E,
} ZoneBits_t;

/* Frame Types */
typedef enum
{
   FRAME_TYPE_HEARTBEAT_E,
   FRAME_TYPE_DATA_E,
   FRAME_TYPE_ACKNOWLEDGEMENT_E,
   FRAME_TYPE_AT_E,
   FRAME_TYPE_PPU_MODE_E,
   FRAME_TYPE_TEST_MESSAGE_E,
   FRAME_TYPE_ERROR_E,
	 FRAME_TYPE_EXIT_TEST_MODE_E,
   FRAME_TYPE_MAX_E
} FrameType_t;


//typedef enum
//{
//   AT_FRAME_TYPE_ENTER_PPU_MODE_E = 0x30,
//   AT_FRAME_TYPE_ENTER_AT_MODE_E = 0x40,
//   AT_FRAME_TYPE_COMAND_E
//} AtModeFrameType_t;
typedef enum
{
   AT_FRAME_TYPE_ENTER_AT_MODE_E = 0x30,
   AT_FRAME_TYPE_COMAND_E,
   AT_FRAME_TYPE_ENTER_PPU_MODE_E = 0x40,
	 AT_FRAME_TYPE_EXIT_TEST_MODE_E,
} OtaModeFrameType_t;

typedef enum
{
   PPU_STATE_NOT_ACTIVE_E,     //Not in ppu mode (normal operation)
   PPU_STATE_REQUESTED_E,      //Starting PPU mode but waiting for outstanding event (e.g. ROUTE DROP confirmation)
   PPU_STATE_DISCONNECTED_E,   //PPU Disonnected. Waiting for PPU Master to connect. Announcement messages broadcast.
   PPU_STATE_CONNECTED_E,      //PPU Connected.  PPU Master connected. No announcement messages.
   PPU_STATE_AUTO_DISCONNECT_E,//Automaticcally go to the PPU Disconnected state on startup
   PPU_STATE_AUTO_CONNECT_E,   //Automaticcally go to the PPU Connected state (used after firmware update reboot)
   PPU_STATE_MAX_E
} PpuState_t;

/* Stuctures
*************************************************************************************/


typedef struct
{
   CIECommandType_t CommandType;
   uint8_t PortNumber;
   uint16_t NodeID;
   uint16_t Zone;
   uint8_t Parameter1;
   uint8_t Parameter2;
   uint32_t Value;
   uint8_t ReadWrite;
   bool QueueProperty[CIE_Q_MAX_E];
} CIECommand_t;

typedef struct
{
   HeadMessageType_t MessageType;
   uint8_t TransactionID;
   uint8_t ChannelNumber;
   uint8_t ProfileIndex;
   uint8_t Duration;
   uint8_t FlashRate;
   uint32_t Value;
} HeadMessage_t;


typedef struct
{
   uint32_t Handle;
   CO_RBUSensorType_t SensorType;
   uint8_t RUChannelIndex;
   uint8_t SensorValue;
   uint8_t AlarmState; //1 for alarm conition, 0 otherwise.
   uint8_t HopCount;
   uint16_t Zone;
} CO_RBUSensorData_t;

typedef struct
{
   uint8_t SmokeSensorValue;
   uint8_t HeatSensorValue;
} CO_SensorAnalogueValues_t;
// This enum is used by MC_StateManagement.
typedef enum
{
   STATE_IDLE_E,         // NCU state, waiting for command to starts issuing heartbeats.  RBUs don't use this state.
   STATE_CONFIG_SYNC_E,  // RBUs will synchronise to the mesh, but will not yet select parents or preferred rank.
   STATE_CONFIG_FORM_E,  // RBUs will undergo mesh forming, will choose rank, select parents, and accept children.
   STATE_ACTIVE_E,       // Channel hopping will be enabled, as will fire and output signalling.
   STATE_TEST_MODE_E,
   STATE_SLEEP_MODE_E,
   STATE_MAX_E,
   STATE_NOT_DEFINED_E
} CO_State_t;


#ifndef WIN32
#pragma push
#pragma anon_unions
#endif

typedef struct
{
   uint32_t Handle;
   uint8_t OutputChannel;
   uint8_t OutputProfile;
   uint16_t OutputsActivated;
   uint8_t OutputDuration;
   union {
      struct
      {
         uint16_t Destination;
         uint16_t OutputDelay;
      };
      uint32_t UnitSerno;
   };
   uint16_t Source;
   uint8_t NumberToSend; /*How many consecutive messages to send*/
   uint16_t zone;
} CO_OutputData_t;

typedef struct
{
   uint32_t Handle;
   uint16_t Source;
   uint32_t Silenceable;
   uint32_t Unsilenceable;
   uint32_t DelayMask;
   uint8_t NumberToSend; /*How many consecutive messages to send*/
} CO_AlarmOutputStateData_t;

typedef struct
{
   uint8_t TransactionID;
   uint16_t Source;
   union{
      struct
      {
         uint16_t Destination;
         uint16_t SourceUSART;
      };
      uint32_t UnitSerno;
   };
   uint8_t CommandType;
   uint8_t Parameter1;
   uint8_t Parameter2;
   uint8_t ReadWrite;
   uint32_t Value;
   uint8_t NumberToSend;
} CO_CommandData_t;

typedef struct
{
   CO_MACEventTypeType_t EventType;
   union{
      struct
      {
         uint16_t Value;            // Data for event types that pass a value
         uint32_t SuperframeSlotIndex;
         uint8_t ShortFrameIndex;
         uint8_t LongFrameIndex;  // Used to decide when to start and stop various stages of the mesh formation process.
         uint8_t SlotIndex;
         uint8_t SlotType;
         uint8_t Frequency;
         bool ProcessSyncMessage;  // Set to true to signal that a call to MC_ProcessSyncMessage() is required.
         uint16_t McuCtr;          // Only used if ProcessSyncMessage is true.
         bool SyncTimestampValid;  // Only used if ProcessSyncMessage is true.
         CO_RxTimeoutError_t RxTimeoutError; // used with missed heartbeat
         bool TransmitSlot;        // Set to true if the MAC has something to transmit in this slot
      };
      uint8_t Data[PHY_DATA_REQ_LENGTH_MAX];
   };
} MACEventMessage_t; /* this should not exceed PHY_DATA_IND_LENGTH_MAX */

typedef struct
{
   uint16_t PpuAddress;
   uint16_t RbuAddress;
   union{
      uint32_t SystemID;
      uint32_t SerialNumber;
   };
   uint16_t Checksum;
   uint16_t CalculatedChecksum;
   uint16_t PacketLength;
   uint16_t Command;
   uint8_t Payload[MAX_PPU_COMMAND_PAYLOAD_LENGTH];
} PPU_Message_t;

#ifndef WIN32
#pragma pop
#endif

typedef CO_CommandData_t CO_ResponseData_t;

typedef struct
{
   uint32_t Handle;
   uint16_t UnitAddress;
} CO_RBUDisableData_t;

typedef struct
{
   uint32_t Handle;
   uint32_t SerialNumber;
   uint8_t DeviceCombination;
   uint16_t ZoneNumber;
} CO_LogonData_t;

typedef struct
{
   uint16_t DestinationNodeID;
   bool Accepted;
   bool DayNight;
   bool GlobalDelayEnabled;
   bool ZoneEnabled;
   uint8_t SoundLevel;
   uint32_t ZoneLowerUpper;//0 if the ZoneMapWord & ZoneMapHalfWord is the lower half of the zone map, 1 if its the upper half.
   uint32_t ZoneMapWord;  //the first word of the zone map if LowerUpperZoneMapBits=0.  The last word if LowerUpperZoneMapBits=1
   uint16_t ZoneMapHalfWord;  //the lower 16 bits of the zone map word 2 if LowerUpperZoneMapBits=0.  The high 16 bits if LowerUpperZoneMapBits=1
   bool FaultsEnabled;
   bool GlobalDelayOverride;
} CO_RouteAddResponseData_t;

typedef struct
{
   uint32_t Handle;
   uint16_t RUChannelIndex;
   uint16_t Value;
   uint16_t FaultType;
   bool DelaySending;
   uint8_t OverallFault;
   bool ForceSend; //send even if the fault flags are unchanged.
   BITFaultSource_t Source;
} CO_FaultData_t;

typedef struct
{
   uint32_t Handle;
   CO_MeshEvent_t Event;
   uint16_t EventNodeId;
   uint32_t EventData; //General purpose value field for multiple usage
   uint8_t OverallFault;
   bool DelaySending; // true if the status was requested by the NCU, or for initial start-up.
} CO_StatusIndicationData_t;

typedef struct
{
   uint32_t Handle;
   uint16_t PrimaryBatteryVoltage;
   uint16_t BackupBatteryVoltage;
   uint8_t DeviceCombination;
   uint8_t ZoneNumber;
   uint8_t SmokeAnalogueValue;
   uint8_t HeatAnalogueValue;
   uint8_t PirAnalogueValue;
   bool DelaySending;
} CO_BatteryStatusData_t;

typedef struct
{
   uint8_t  SlotIndex;
   uint8_t  ShortFrameIndex;
   uint8_t  LongFrameIndex;
} CO_MessagePayloadGenerateHeartbeat_t;

typedef struct
{
   uint32_t FrameType;
   uint32_t MACDestinationAddress;
   uint32_t MACSourceAddress;
   uint32_t SystemId;
} FrameAcknowledge_t;

typedef struct
{
   CO_MACEventTypeType_t EventType;
   uint32_t Value;
} AppMACEventData_t; /* this should not exceed PHY_DATA_IND_LENGTH_MAX */

typedef struct
{
   uint32_t FrameType;
   uint32_t Command;
   uint32_t Address;
   uint32_t SystemId;
} PpuModeRequest_t;


typedef struct
{
   uint32_t FrameType;
   uint32_t MACDestinationAddress;
   uint32_t MACSourceAddress;
   uint32_t  HopCount;
   uint32_t  DestinationAddress;
   uint32_t  SourceAddress;
   uint32_t   MessageType;
} ApplicationMessageHeader_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t RUChannel;
   uint32_t SensorValue;
   uint32_t Zone;
   uint32_t AlarmActive;
   uint32_t SystemId;
} AlarmSignal_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t RUChannelIndex;
   uint32_t Value;
   uint32_t FaultType;
   uint32_t OverallFault;
   uint32_t SystemId;
} FaultSignal_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t SerialNumber;
   uint32_t DeviceCombination;
   uint32_t ZoneNumber;
   uint32_t SystemId;
} LogOnMessage_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t Zone;
   uint32_t Rank;
   uint32_t IsPrimary;
   uint32_t SystemId;
} RouteAddMessage_t;

typedef struct
{
   uint16_t NodeID;
   uint16_t Zone;
   uint32_t Rank;
   bool IsPrimary;
   bool UseSACH;
} RouteAddData_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t Accepted;
   uint32_t DayNight;
   uint32_t GlobalDelayEnabled;
   uint32_t ZoneEnabled;
   uint32_t ZoneLowerUpper;//0 if the ZoneMapWord & ZoneMapHalfWord is the lower half of the zone map, 1 if its the upper half.
   uint32_t ZoneMapWord;  //the first word of the zone map if LowerUpperZoneMapBits=0.  The last word if LowerUpperZoneMapBits=1
   uint32_t ZoneMapHalfWord;  //the lower 16 bits of the zone map word 2 if LowerUpperZoneMapBits=0.  The high 16 bits if LowerUpperZoneMapBits=1
   uint32_t FaultsEnabled;
   uint32_t GlobalDelayOverride;
   uint32_t SystemId;
} RouteAddResponseMessage_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t Reason;
   uint32_t SystemId;
} RouteDropMessage_t;

typedef struct
{
   uint16_t NodeID;
   uint32_t Reason;
   bool Downlink;
   bool WaitForAck;
   uint32_t Handle;
} RouteDropData_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t SystemId;
} LoadBalanceMessage_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t Accepted;
   uint32_t SystemId;
} LoadBalanceResponseMessage_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t LowHigh;
   uint32_t HalfWord;
   uint32_t Word;
   uint32_t SystemId;
} ZoneEnableMessage_t;


typedef struct
{
   uint32_t Handle;
   ZoneBits_t LowHigh;
   uint16_t HalfWord;
   uint32_t Word;
   UartComm_t Source;
} ZoneEnableData_t;


typedef struct
{
   OtaModeFrameType_t FrameType;
   uint32_t Destination;
   uint32_t SystemID;
   uint16_t MessageSize;
}OtaMode_t;


typedef struct{
   uint32_t Destination;
   uint32_t Source;
   uint8_t IsCommand;   //1 if comand, 0 if response
   char Command[MAX_OTA_AT_COMMAND_LENGTH];
} AtMessageData_t;


typedef struct
{
   uint16_t DestinationNodeID;
   bool Accepted;
} CO_LoadBalanceResponseData_t;

typedef struct
{
   uint16_t Handle;
   uint16_t NodeID;
   uint16_t Zone;
   uint8_t OutputChannel;
   uint8_t OutputProfile;
   uint16_t OutputActivated;
   uint8_t OutputDuration;
   uint8_t SilenceableMask;
   uint8_t UnsilenceableMask;
   uint8_t DelayMask;
   uint32_t ErrorCode;
} OutputStateData_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t ParameterType;
   uint32_t ReadWrite;
   uint32_t TransactionID;
   uint32_t P1;//P1 and P2 have different uses depending on ParameterType.
   uint32_t P2;//See the Cygnus2 Mesh Protocol Document HKD-16-0015-D
   uint32_t Value;
   uint32_t SystemId;
} CommandMessage_t;

typedef CommandMessage_t ResponseMessage_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t PrimaryParentID;
   uint32_t SecondaryParentID;
   uint32_t AverageRssiPrimaryParent;
   uint32_t AverageRssiSecondaryParent;
   uint32_t Rank;
   uint32_t Event;
   uint32_t EventData;
   uint32_t OverallFault;
   uint32_t SystemId;
} StatusIndicationMessage_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t PrimaryBatteryVoltage;
   uint32_t BackupBatteryVoltage;
   uint32_t DeviceCombination;
   uint32_t ZoneNumber;
   uint32_t SmokeAnalogueValue;
   uint32_t HeatAnalogueValue;
   uint32_t PirAnalogueValue;
   uint32_t SystemId;
} BatteryStatusMessage_t;

typedef struct
{
   uint32_t FrameType;
   uint32_t SourceAddress;
   uint32_t Payload[4];
   uint32_t SystemId;
} FrameTestModeMessage_t;

typedef struct
{
   uint16_t count;
   uint16_t source;
   char stringMsg[13];
} ATTestMessageData_t;

typedef struct
{
   uint16_t UnitAddress;
   uint16_t Zone;
   uint16_t Voltage;
   uint16_t Timeout;
   uint8_t OnTime;
   uint8_t OffTime;
} DePassivationSettings_t;

typedef struct{
   uint32_t Handle;
   uint32_t MessageType;
   uint8_t Source;
   uint8_t port;
   uint8_t MessageBuffer[PHY_DATA_IND_LENGTH_MAX - 1];
} ApplicationMessage_t;

typedef struct
{
   uint32_t FrameType;               // 4 bits 
   uint32_t SlotIndex;               // 3 bits
   uint32_t ShortFrameIndex;         // 8 bits
   uint32_t LongFrameIndex;          // 6 bits
   CO_State_t State;                 // 4 bits
   uint32_t Rank;                    // 6 bits
   uint32_t ActiveFrameLength;       // 1 bit
   uint32_t NoOfChildrenIdx;         // 4 bits
   uint32_t NoOfChildrenOfPTNIdx;    // 4 bits
   uint32_t SystemId;                // 32 bits
} FrameHeartbeat_t;          // TOTAL = 72 bits 

typedef struct
{
   uint8_t Data[PHY_DATA_IND_LENGTH_MAX];
   uint16_t Size;
   int16_t RSSI;
   int8_t SNR;
   uint8_t freqChanIdx;
   int32_t freqDeviation;
   uint16_t timer;                // The actual timestamp of the RxDone event.  Can be compared with Expectedtimer to assess sync tracking performance.
   uint16_t Expectedtimer;        // This was the estimated time at which the RxDone should have occured: used to calc when to start the RxSingle mode.
   uint8_t  slotIdxInShortframe;  // Valid range 0 to 19
   uint16_t slotIdxInLongframe;   // Valid range 0 to 2719
   uint32_t slotIdxInSuperframe;  // Valid range 0 to 174079
   bool isInTDMSyncSlaveMode;
} CO_MessagePayloadPhyDataInd_t;

typedef struct
{
   uint8_t Data[PHY_DATA_REQ_LENGTH_MAX];
   uint16_t Size;
   uint32_t slotIdxInSuperframe;
   uint8_t freqChanIdx;
   uint8_t txPower;
   bool Downlink;
} CO_MessagePayloadPhyDataReq_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t Zone;
   uint32_t OutputChannel;
   uint32_t OutputProfile;
   uint32_t OutputsActivated;
   uint32_t OutputDuration;
   uint32_t SystemId;
} OutputSignal_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t Silenceable;
   uint32_t Unsilenceable;
   uint32_t DelayMask;
   uint32_t SystemId;
} AlarmOutputState_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   CO_State_t State;
   uint32_t SystemId;
} SetStateMessage_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t UnitAddress;
   uint32_t SystemId;
} RBUDisableMessage_t;

typedef union
{
   CO_MessagePayloadGenerateHeartbeat_t GenerateHeartbeat;
   CO_MessagePayloadPhyDataInd_t PhyDataInd;
   CO_MessagePayloadPhyDataReq_t PhyDataReq;
} CO_MessagePayload_t;

/* message type */
typedef struct
{
   CO_MessageType_t Type;
   CO_MessagePayload_t Payload;
} CO_Message_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t OutputProfile;
   uint32_t SystemId;
} OutputStateRequest_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t Zone;
   uint32_t OutputChannel;
   uint32_t OutputProfile;
   uint32_t OutputActivated;
   uint32_t OutputDuration;
   uint32_t SilenceableMask;
   uint32_t UnsilenceableMask;
   uint32_t DelayMask;
   uint32_t SystemId;
} OutputStateMessage_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t Payload;
   uint32_t SystemId;
   SendBehaviour_t SendBehaviour;
} PingRequest_t;

typedef struct
{
   ApplicationMessageHeader_t Header;
   uint32_t Delay1;
   uint32_t Delay2;
   uint32_t ChannelLocalOrGlobalDelay;
   uint32_t SystemId;
} CO_GlobalDelaysMessage_t;

typedef struct
{
   uint16_t Delay1;
   uint16_t Delay2;
   uint8_t ChannelLocalOrGlobalDelay;
} CO_GlobalDelaysData_t;


/* Mesh to Application message types
*************************************************************************************/

/* fire and first aid indications */
typedef struct
{
   uint8_t RUChannelIndex;
   uint8_t AlarmActive;
   uint8_t SensorValue;
   uint16_t SourceAddress;
   uint16_t Zone;
} AlarmSignalIndication_t;

typedef struct
{
   uint16_t SourceAddress;
   uint16_t RUChannelIndex;
   uint16_t FaultType;
   uint16_t Value;
   uint16_t OverallFault;
} FaultSignalIndication_t;

typedef struct
{
   uint16_t SourceAddress;
   uint32_t SerialNumber;
   uint16_t ZoneNumber;
   uint8_t  DeviceCombination;
} LogOnSignalIndication_t;

typedef struct
{
   uint8_t OutputChannel;
   uint8_t OutputProfile;
   uint16_t OutputsActivated;
   uint8_t OutputDuration;
   uint8_t FlashRate;
   uint16_t nodeID;
   uint16_t zone;
} OutputSignalIndication_t;

typedef struct
{
   uint16_t SourceNode;
   uint16_t PrimaryParent;
   uint16_t SecondaryParent;
   uint16_t AverageRssiPrimaryParent;
   uint16_t AverageRssiSecondaryParent;
   uint8_t Rank;
   uint8_t Event;
   uint16_t EventData;
   uint8_t OverallFault;
   uint32_t SystemId;
} StatusIndication_t;


typedef struct
{
   AppConfirmationType_t ConfirmationType;
   uint32_t Handle;
   uint32_t ReplacementHandle;//for when a message gets superceeded in the Ack Queue
   uint32_t Error;
} AppRequestConfirmation_t;


typedef struct
{
   ApplicationLayerMessageType_t type;
   int32_t reason;
   CO_Message_t* pMessage;
} TestModeReport_t;

typedef struct
{
   uint16_t SourceAddress;
   uint16_t PrimaryBatteryVoltage;
   uint16_t BackupBatteryVoltage;
   uint16_t DeviceCombination;
   uint16_t ZoneNumber;
   uint16_t SmokeAnalogueValue;
   uint16_t HeatAnalogueValue;
   uint16_t PirAnalogueValue;
} BatteryStatusIndication_t;


/* Public Functions Prototypes
*************************************************************************************/


/* Macros
*************************************************************************************/


/* Variables
*************************************************************************************/
extern const char MESH_EVENT_STR[][64];/* Defined in MM_NCUApplicationStub.c.  Keep definition aligned with CO_MeshEvent_t */


#endif // CO_MESSAGE_H
