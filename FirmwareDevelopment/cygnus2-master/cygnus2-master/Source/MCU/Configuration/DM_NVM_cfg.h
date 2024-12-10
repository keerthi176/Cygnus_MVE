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
*  File         : DM_NVM_cfg.h
*
*  Description  : Header file of the configuration of the Non-Volatile Memory driver module
*
*************************************************************************************/

#ifndef DM_NVM_CFG_H
#define DM_NVM_CFG_H


/* System Include files
*******************************************************************************/
#include <stdint.h>


/* User Include files
*******************************************************************************/
#include "CO_ErrorCode.h"


/* Public Structures
*******************************************************************************/


/* Public Enumerations
*******************************************************************************/ 
typedef enum
{
   NV_REPROG_USE_PP_UART_E = 0u,
   NV_ADDRESS_E,
   NV_IS_SYNC_MASTER_E,
   NV_FREQUENCY_CHANNEL_E,
   NV_DEVICE_COMBINATION_E,
   NV_SYSTEM_ID_E,                                          // 5
   NV_UNIT_SERIAL_NO_E,
   NV_TX_POWER_LOW_E,
   NV_TX_POWER_HIGH_E,
   NV_ZONE_NUMBER_E,
   NV_PP_MODE_ENABLE_E,                                     // 10
   NV_BRANDING_ID_E,
   NV_BANK1_CRC_E,
   NV_BANK2_CRC_E,
   NV_PREFORM_MESH_PARENTS_E,
   NV_SHORT_FRAMES_PER_LONG_FRAME_E,                        // 15
   NV_PROFILE_FIRE_E,
   NV_PROFILE_FIRST_AID_E,
   NV_PROFILE_EVACUATE_E,
   NV_PROFILE_SECURITY_E,
   NV_PROFILE_GENERAL_E,                                    // 20
   NV_PROFILE_FAULT_E,
   NV_PROFILE_ROUTING_ACK_E,
   NV_PROFILE_TEST_E,
   NV_PROFILE_SILENT_TEST_E,
   NV_IOU_INPUT_SHORT_THRESHOLD_E,                          // 25
   NV_IOU_INPUT_LOGIC_THRESHOLD_E,
   NV_IOU_INPUT_OPEN_THRESHOLD_E,
   NV_IOU_INPUT_POLL_PERIOD_E,
   NV_CHANNEL_CONFIG_SMOKE_E,
   NV_CHANNEL_CONFIG_HEAT_B_E,                              // 30
   NV_CHANNEL_CONFIG_CO_E,
   NV_CHANNEL_CONFIG_PIR_E,
   NV_CHANNEL_CONFIG_SOUNDER_E,
   NV_CHANNEL_CONFIG_BEACON_E,
   NV_CHANNEL_CONFIG_FIRE_CALLPOINT_E,                      // 35
   NV_CHANNEL_CONFIG_STATUS_LED_E,
   NV_CHANNEL_CONFIG_VISUAL_INDICATOR_E,
   NV_CHANNEL_CONFIG_VISUAL_INDICATOR_COMBINED_E,
   NV_CHANNEL_CONFIG_MEDICAL_CALLPOINT_E,
   NV_CHANNEL_CONFIG_EVAC_CALLPOINT_E,                      // 40
   NV_CHANNEL_CONFIG_OUTPUT_ROUTING_E,
   NV_CHANNEL_CONFIG_INPUT_1_E,
   NV_CHANNEL_CONFIG_INPUT_2_E,
   NV_CHANNEL_CONFIG_INPUT_3_E,
   NV_CHANNEL_CONFIG_INPUT_4_E,                             // 45
   NV_CHANNEL_CONFIG_INPUT_5_E,
   NV_CHANNEL_CONFIG_INPUT_6_E,
   NV_CHANNEL_CONFIG_INPUT_7_E,
   NV_CHANNEL_CONFIG_INPUT_8_E,
   NV_CHANNEL_CONFIG_INPUT_9_E,                             // 50
   NV_CHANNEL_CONFIG_INPUT_10_E,
   NV_CHANNEL_CONFIG_INPUT_11_E,
   NV_CHANNEL_CONFIG_INPUT_12_E,
   NV_CHANNEL_CONFIG_INPUT_13_E,
   NV_CHANNEL_CONFIG_INPUT_14_E,                            // 55
   NV_CHANNEL_CONFIG_INPUT_15_E,
   NV_CHANNEL_CONFIG_INPUT_16_E,
   NV_CHANNEL_CONFIG_INPUT_17_E,
   NV_CHANNEL_CONFIG_INPUT_18_E,
   NV_CHANNEL_CONFIG_INPUT_19_E,                            // 60
   NV_CHANNEL_CONFIG_INPUT_20_E,
   NV_CHANNEL_CONFIG_INPUT_21_E,
   NV_CHANNEL_CONFIG_INPUT_22_E,
   NV_CHANNEL_CONFIG_INPUT_23_E,
   NV_CHANNEL_CONFIG_INPUT_24_E,                            // 65
   NV_CHANNEL_CONFIG_INPUT_25_E,
   NV_CHANNEL_CONFIG_INPUT_26_E,
   NV_CHANNEL_CONFIG_INPUT_27_E,
   NV_CHANNEL_CONFIG_INPUT_28_E,
   NV_CHANNEL_CONFIG_INPUT_29_E,                            // 70
   NV_CHANNEL_CONFIG_INPUT_30_E,
   NV_CHANNEL_CONFIG_INPUT_31_E,
   NV_CHANNEL_CONFIG_INPUT_32_E,
   NV_CHANNEL_CONFIG_OUTPUT_1_E,
   NV_CHANNEL_CONFIG_OUTPUT_2_E,                            // 75
   NV_CHANNEL_CONFIG_OUTPUT_3_E,
   NV_CHANNEL_CONFIG_OUTPUT_4_E,
   NV_CHANNEL_CONFIG_OUTPUT_5_E,
   NV_CHANNEL_CONFIG_OUTPUT_6_E,
   NV_CHANNEL_CONFIG_OUTPUT_7_E,                            // 80
   NV_CHANNEL_CONFIG_OUTPUT_8_E,
   NV_CHANNEL_CONFIG_OUTPUT_9_E,
   NV_CHANNEL_CONFIG_OUTPUT_10_E,
   NV_CHANNEL_CONFIG_OUTPUT_11_E,
   NV_CHANNEL_CONFIG_OUTPUT_12_E,                           // 85
   NV_CHANNEL_CONFIG_OUTPUT_13_E,
   NV_CHANNEL_CONFIG_OUTPUT_14_E,
   NV_CHANNEL_CONFIG_OUTPUT_15_E,
   NV_CHANNEL_CONFIG_OUTPUT_16_E,
   NV_CHANNEL_CONFIG_HEAT_A1R_E,                            // 90
   NV_SOUND_LEVEL_E,
   NV_MAXIMUM_RANK_E,
   NV_RESERVED_0_E,
   NV_RESERVED_1_E,
   NV_RESERVED_2_E,                                         // 95
   NV_RESERVED_3_E,
   NV_RESERVED_4_E,
   NV_RESERVED_5_E,
   NV_RESERVED_6_E,
   NV_RESERVED_7_E,                                         // 100
   NV_RESERVED_8_E,
   NV_RESERVED_9_E,
   NV_DULCH_WRAP_E,
   NV_GLOBAL_DELAY_E,
   NV_ZONE_MAP_E,                                           // 105
   NV_RESTART_FLAGS_E,
   NV_MAX_HOPS_E,
   NV_RSSI_MAR_E,
   NV_SNR_MAR_E,
   NV_BATTERY_PERIOD_E,                                     // 110
   NV_AVE_POLICY_E,
   NV_INIT_LISTEN_PERIOD_E,
   NV_PHASE2_DURATION_E,
   NV_PHASE2_SLEEP_DURATION_E,
   NV_PHASE3_SLEEP_DURATION_E,                              // 115
   NV_FREQUENCY_BAND_E,
   NV_MAX_RBU_CHILDREN_E,
   NV_NUMBER_MSG_RESENDS_E,
   NV_PRODUCT_CODE_E,
   NV_BAT_PASSIVATION_SETTINGS_E,                           // 120
   NV_ENABLE_NO_FAULT_REPORT_E,
	NV_PIR_ACTIVATE_PERIOD_E,
	NV_PIR_DEACTIVATE_PERIOD_E,
	NV_PIR_STRIKE_COUNT_E,
	NV_BAT_PRI_LOW_VOLT_THRESH_E,                            /* 125 */
	NV_BAT_BKP_LOW_VOLT_THRESH_E,
	NV_BAT_PRI_LOW_VOLT_THRESH_C_E, /* For construction */
	NV_BAT_BKP_LOW_VOLT_THRESH_C_E, /* For construction */
	NV_FW_VERSION_E,
   NV_MAX_PARAM_ID_E   // N.B. if you add a new Enum value to this list, don't forget to update the definition of ParameterInfo in DM_NVM_cfg.c
} DM_NVMParamId_t;


/* Public functions prototypes
*******************************************************************************/


/* Public Constants
*******************************************************************************/
#define DEFAULT_NUMBER_OF_RESENDS           3   /* The number of times downlink messages are repeated        */
#define MAX_MSG_HOP_THRESHOLD           (MAX_INITIAL_RANK + 1) /* The max hops before a message is discarded */
#define DEFAULT_BATTERY_TEST_INTERVAL     (30U) /* days */

extern const uint16_t DM_NVM_ParameterVirtualAddr[NV_MAX_PARAM_ID_E];
extern const uint16_t DM_NVM_ParameterSize[NV_MAX_PARAM_ID_E];
extern const void *const DM_NVM_DefaultValues[NV_MAX_PARAM_ID_E];

/* Macros
*******************************************************************************/


#endif // DM_NVM_CFG_H
