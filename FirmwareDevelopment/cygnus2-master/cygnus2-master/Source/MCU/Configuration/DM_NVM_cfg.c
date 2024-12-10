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
*  File         : DM_NVM_cfg.c
*
*  Description  :
*
*   Configuration of the Non-Volatile Memory driver module
*
*************************************************************************************/



/* System Include files
*******************************************************************************/
#include <string.h>

/* User Include files
*******************************************************************************/

#include "stm32l4xx.h"
#include "eeprom_emul.h"
#include "CO_Message.h"
#include "DM_OutputManagement.h"
#include "MC_MacConfiguration.h"
#include "DM_NVM.h"
#include "CFG_Device_cfg.h"
#include "DM_BatteryMonitor.h"


/* Global Variables
*******************************************************************************/

/* These are the default values of individual variables */
const uint32_t nvm_reprog_use_pp_uart_default = 0u;
const uint32_t nvm_unit_address_default = 0x20;
const uint32_t nvm_is_sync_master_default = 0u;
const uint32_t nvm_frequency_channel_default = 0u;
const uint32_t nvm_device_combination_default = 0u;
const uint32_t nvm_system_id_default = 1u;
const uint32_t nvm_unit_serial_no_default = 0u;
const uint32_t nvm_tx_power_low_default = 7u;
const uint32_t nvm_tx_power_high_default = 10u;
const uint32_t nvm_zone_number_default = 1u;
const uint32_t nvm_tx_pp_mode_default = 0u;
const uint32_t nvm_branding_id_default = 1u;
const uint32_t nvm_bank1_crc = 1u;
const uint32_t nvm_bank2_crc = 1u;
const uint32_t nvm_preformed_mesh_parents_default = 0xFFFFFFFF;
const uint32_t nvm_short_frames_per_long_frame = 128;
const uint32_t nvm_profile_fire = ((uint32_t)DM_OP_SITENET_SOUNDER_PATTERN_ON_E << 24) | ((uint32_t)FLASH_RATE_1_E << 16) | (uint32_t)DM_SVI_TONE_FIRE_E;
const uint32_t nvm_profile_first_aid = ((uint32_t)DM_OP_SITENET_SOUNDER_PATTERN_1000_7000_E << 24) | ((uint32_t)FLASH_RATE_1_E << 16) | (uint32_t)DM_SVI_TONE_FIRST_AID_E;
const uint32_t nvm_profile_evacuate = ((uint32_t)DM_OP_SITENET_SOUNDER_PATTERN_500_700_E << 24) | ((uint32_t)FLASH_RATE_2_E << 16) | (uint32_t)DM_SVI_TONE_EVACUATE_E;
const uint32_t nvm_profile_security = ((uint32_t)DM_OP_SITENET_SOUNDER_PATTERN_500_300_E << 24) | ((uint32_t)FLASH_RATE_1_E << 16) | (uint32_t)DM_SVI_TONE_SECURITY_E;
const uint32_t nvm_profile_general = ((uint32_t)DM_OP_SITENET_SOUNDER_PATTERN_OFF_E << 24) | ((uint32_t)FLASH_RATE_1_E << 16) | (uint32_t)DM_SVI_TONE_TEST_E;
const uint32_t nvm_profile_fault = ((uint32_t)DM_OP_SITENET_SOUNDER_PATTERN_OFF_E << 24) | ((uint32_t)FLASH_RATE_1_E << 16) | (uint32_t)DM_SVI_TONE_SILENT_E;
const uint32_t nvm_profile_routing_ack = ((uint32_t)DM_OP_SITENET_SOUNDER_PATTERN_OFF_E << 24) | ((uint32_t)FLASH_RATE_1_E << 16) | (uint32_t)DM_SVI_TONE_SILENT_E;
const uint32_t nvm_profile_test = ((uint32_t)DM_OP_SITENET_SOUNDER_PATTERN_100_2000_e << 24) | ((uint32_t)FLASH_RATE_1_E << 16) | (uint32_t)DM_SVI_TONE_TEST_E;
const uint32_t nvm_profile_silent_test = ((uint32_t)DM_OP_SITENET_SOUNDER_PATTERN_OFF_E << 24) | ((uint32_t)FLASH_RATE_1_E << 16) | (uint32_t)DM_SVI_TONE_SILENT_E;
const uint32_t nvm_iou_short_threshold = 332u;
const uint32_t nvm_iou_logic_threshold = 1727u;
const uint32_t nvm_iou_open_threshold = 3443u;
const uint32_t nvm_iou_poll_period = 1000u;
const uint32_t nvm_channel_smoke = 0u;
const uint32_t nvm_channel_heat_b = 0u;
const uint32_t nvm_channel_co = 0u;
const uint32_t nvm_channel_pir = 0u;
const uint32_t nvm_channel_sounder = 0u;
const uint32_t nvm_channel_beacon = 0u;
const uint32_t nvm_channel_fire_callpoint = 0u;
const uint32_t nvm_channel_status_led = 0u;
const uint32_t nvm_channel_visual_indicator = 0u;
const uint32_t nvm_channel_visual_indicator_combined = 0u;
const uint32_t nvm_channel_medical_callpoint = 0u;
const uint32_t nvm_channel_evac_callpoint = 0u;
const uint32_t nvm_channel_output_routing = 0u;
const uint32_t nvm_channel_input_1 = 0u;
const uint32_t nvm_channel_input_2 = 0u;
const uint32_t nvm_channel_input_3 = 0u;
const uint32_t nvm_channel_input_4 = 0u;
const uint32_t nvm_channel_input_5 = 0u;
const uint32_t nvm_channel_input_6 = 0u;
const uint32_t nvm_channel_input_7 = 0u;
const uint32_t nvm_channel_input_8 = 0u;
const uint32_t nvm_channel_input_9 = 0u;
const uint32_t nvm_channel_input_10 = 0u;
const uint32_t nvm_channel_input_11 = 0u;
const uint32_t nvm_channel_input_12 = 0u;
const uint32_t nvm_channel_input_13 = 0u;
const uint32_t nvm_channel_input_14 = 0u;
const uint32_t nvm_channel_input_15 = 0u;
const uint32_t nvm_channel_input_16 = 0u;
const uint32_t nvm_channel_input_17 = 0u;
const uint32_t nvm_channel_input_18 = 0u;
const uint32_t nvm_channel_input_19 = 0u;
const uint32_t nvm_channel_input_20 = 0u;
const uint32_t nvm_channel_input_21 = 0u;
const uint32_t nvm_channel_input_22 = 0u;
const uint32_t nvm_channel_input_23 = 0u;
const uint32_t nvm_channel_input_24 = 0u;
const uint32_t nvm_channel_input_25 = 0u;
const uint32_t nvm_channel_input_26 = 0u;
const uint32_t nvm_channel_input_27 = 0u;
const uint32_t nvm_channel_input_28 = 0u;
const uint32_t nvm_channel_input_29 = 0u;
const uint32_t nvm_channel_input_30 = 0u;
const uint32_t nvm_channel_input_31 = 0u;
const uint32_t nvm_channel_input_32 = 0u;
const uint32_t nvm_channel_output_1 = 0u;
const uint32_t nvm_channel_output_2 = 0u;
const uint32_t nvm_channel_output_3 = 0u;
const uint32_t nvm_channel_output_4 = 0u;
const uint32_t nvm_channel_output_5 = 0u;
const uint32_t nvm_channel_output_6 = 0u;
const uint32_t nvm_channel_output_7 = 0u;
const uint32_t nvm_channel_output_8 = 0u;
const uint32_t nvm_channel_output_9 = 0u;
const uint32_t nvm_channel_output_10 = 0u;
const uint32_t nvm_channel_output_11 = 0u;
const uint32_t nvm_channel_output_12 = 0u;
const uint32_t nvm_channel_output_13 = 0u;
const uint32_t nvm_channel_output_14 = 0u;
const uint32_t nvm_channel_output_15 = 0u;
const uint32_t nvm_channel_output_16 = 0u;
const uint32_t nvm_channel_heat_a1r = 0u;
const uint32_t nvm_sound_level = 3u;
const uint32_t nvm_maximum_rank = MAX_RANK;
const uint32_t nvm_reserved_0 = 0u;
const uint32_t nvm_reserved_1 = 0u;
const uint32_t nvm_reserved_2 = 0u;
const uint32_t nvm_reserved_3 = 0u;
const uint32_t nvm_reserved_4 = 0u;
const uint32_t nvm_reserved_5 = 0u;
const uint32_t nvm_reserved_6 = 0u;
const uint32_t nvm_reserved_7 = 0u;
const uint32_t nvm_reserved_8 = 0u;
const uint32_t nvm_reserved_9 = 0u;
const uint32_t nvm_dulch_wrap = 1024;
const uint32_t nvm_global_delay = 0u;
const uint32_t nvm_zone_map[3] = {0u, 0u, 0u};
const uint32_t nvm_restart_flags = 0;
const uint32_t nvm_max_hops = MAX_MSG_HOP_THRESHOLD;
const uint32_t nvm_rssi_mar = 0;
const uint32_t nvm_snr_rssi_ave_policy = 0;
const uint32_t nvm_snr_mar = 5;
const uint32_t nvm_battery_period = DEFAULT_BATTERY_TEST_INTERVAL; //days
const uint32_t nvm_initial_listen_period = LOW_POWER_INITIAL_AWAKE_DURATION; //seconds
const uint32_t nvm_phase2_duration = LOW_POWER_PHASE2_DURATION; //days
const uint32_t nvm_phase2_sleep_duration = LOW_POWER_PHASE2_SLEEP_DURATION; //hours
const uint32_t nvm_phase3_sleep_duration = LOW_POWER_PHASE3_SLEEP_DURATION; //hours
const uint32_t nvm_frequency_band = FREQUENCY_BAND_865_E;
const uint32_t nvm_max_rbu_children = DEFAULT_CHILDREN_PER_PARENT;
const uint32_t nvm_number_of_resends = DEFAULT_NUMBER_OF_RESENDS;
const uint32_t nvm_product_code = 0x2D2D2D; // "---"
const uint32_t nvm_battery_passivation_settings = DEFAULT_PASSIVATION_SETTINGS;
const uint32_t nvm_enable_no_fault_report = 0;
const uint32_t nvm_pir_activate_period_us = 30 * 1000;
const uint32_t nvm_pir_deactivate_period_us = 30 * 1000;
const uint32_t nvm_pir_strike_count = 5;
const uint32_t nvm_bat_pri_low_volt_thresh = DM_BAT_MCP_FAIL_THRESHOLD_MV;                    /* For SmartNet */
const uint32_t nvm_bat_bkp_low_volt_thresh = DM_BAT_MCP_FAIL_THRESHOLD_MV;                    /* For SmartNet */
const uint32_t nvm_bat_pri_low_volt_thresh_c = DM_BAT_CONSTRUCTION_PRIMARY_FAIL_THRESHOLD_MV; /* For construction */
const uint32_t nvm_bat_bkp_low_volt_thresh_c = DM_BAT_CONSTRUCTION_BACKUP_FAIL_THRESHOLD_MV;  /* For construction */
const uint32_t nvm_fw_version_int = 0;  /* FW VERSION */

/* 
 * If the data is bigger than a word (4 bytes) then leave the appropriate gap
 *   prior to the next address in the list
 * Address Zero is not allowed
*/
const uint16_t DM_NVM_ParameterVirtualAddr[NV_MAX_PARAM_ID_E] = 
{   
   [NV_REPROG_USE_PP_UART_E] = 1u,
   [NV_ADDRESS_E] = 2u,
   [NV_IS_SYNC_MASTER_E] = 3u,
   [NV_FREQUENCY_CHANNEL_E] = 4u,
   [NV_DEVICE_COMBINATION_E] = 5u,
   [NV_SYSTEM_ID_E] = 6u,
   [NV_UNIT_SERIAL_NO_E] = 7u,
   [NV_TX_POWER_LOW_E] = 8u,
   [NV_TX_POWER_HIGH_E] = 9u,
   [NV_ZONE_NUMBER_E] = 10u,
   [NV_PP_MODE_ENABLE_E] = 11u,
   [NV_BRANDING_ID_E] = 12u,
   [NV_BANK1_CRC_E] = 13u,
   [NV_BANK2_CRC_E] = 14u,
   [NV_PREFORM_MESH_PARENTS_E] = 15u,
   [NV_SHORT_FRAMES_PER_LONG_FRAME_E] = 16u,
   [NV_PROFILE_FIRE_E] = 17u,
   [NV_PROFILE_FIRST_AID_E] = 18u,
   [NV_PROFILE_EVACUATE_E] = 19u,
   [NV_PROFILE_SECURITY_E] = 20u,
   [NV_PROFILE_GENERAL_E] = 21u,
   [NV_PROFILE_FAULT_E] = 22u,
   [NV_PROFILE_ROUTING_ACK_E] = 23u,
   [NV_PROFILE_TEST_E] = 24u,
   [NV_PROFILE_SILENT_TEST_E] = 25u,
   [NV_IOU_INPUT_SHORT_THRESHOLD_E] = 26u,
   [NV_IOU_INPUT_LOGIC_THRESHOLD_E] = 27u,
   [NV_IOU_INPUT_OPEN_THRESHOLD_E] = 28u,
   [NV_IOU_INPUT_POLL_PERIOD_E] = 29u,
   [NV_CHANNEL_CONFIG_SMOKE_E] = 30u,
   [NV_CHANNEL_CONFIG_HEAT_B_E] = 31u,
   [NV_CHANNEL_CONFIG_CO_E] = 32u,
   [NV_CHANNEL_CONFIG_PIR_E] = 33u,
   [NV_CHANNEL_CONFIG_SOUNDER_E] = 34u,
   [NV_CHANNEL_CONFIG_BEACON_E] = 35u,
   [NV_CHANNEL_CONFIG_FIRE_CALLPOINT_E] = 36u,
   [NV_CHANNEL_CONFIG_STATUS_LED_E] = 37u,
   [NV_CHANNEL_CONFIG_VISUAL_INDICATOR_E] = 38u,
   [NV_CHANNEL_CONFIG_VISUAL_INDICATOR_COMBINED_E] = 39u,
   [NV_CHANNEL_CONFIG_MEDICAL_CALLPOINT_E] = 40u,
   [NV_CHANNEL_CONFIG_EVAC_CALLPOINT_E] = 41u,
   [NV_CHANNEL_CONFIG_OUTPUT_ROUTING_E] = 42u,
   [NV_CHANNEL_CONFIG_INPUT_1_E] = 43u,
   [NV_CHANNEL_CONFIG_INPUT_2_E] = 44u,
   [NV_CHANNEL_CONFIG_INPUT_3_E] = 45u,
   [NV_CHANNEL_CONFIG_INPUT_4_E] = 46u,
   [NV_CHANNEL_CONFIG_INPUT_5_E] = 47u,
   [NV_CHANNEL_CONFIG_INPUT_6_E] = 48u,
   [NV_CHANNEL_CONFIG_INPUT_7_E] = 49u,
   [NV_CHANNEL_CONFIG_INPUT_8_E] = 50u,
   [NV_CHANNEL_CONFIG_INPUT_9_E] = 51u,
   [NV_CHANNEL_CONFIG_INPUT_10_E] = 52u,
   [NV_CHANNEL_CONFIG_INPUT_11_E] = 53u,
   [NV_CHANNEL_CONFIG_INPUT_12_E] = 54u,
   [NV_CHANNEL_CONFIG_INPUT_13_E] = 55u,
   [NV_CHANNEL_CONFIG_INPUT_14_E] = 56u,
   [NV_CHANNEL_CONFIG_INPUT_15_E] = 57u,
   [NV_CHANNEL_CONFIG_INPUT_16_E] = 58u,
   [NV_CHANNEL_CONFIG_INPUT_17_E] = 59u,
   [NV_CHANNEL_CONFIG_INPUT_18_E] = 60u,
   [NV_CHANNEL_CONFIG_INPUT_19_E] = 61u,
   [NV_CHANNEL_CONFIG_INPUT_20_E] = 62u,
   [NV_CHANNEL_CONFIG_INPUT_21_E] = 63u,
   [NV_CHANNEL_CONFIG_INPUT_22_E] = 64u,
   [NV_CHANNEL_CONFIG_INPUT_23_E] = 65u,
   [NV_CHANNEL_CONFIG_INPUT_24_E] = 66u,
   [NV_CHANNEL_CONFIG_INPUT_25_E] = 67u,
   [NV_CHANNEL_CONFIG_INPUT_26_E] = 68u,
   [NV_CHANNEL_CONFIG_INPUT_27_E] = 69u,
   [NV_CHANNEL_CONFIG_INPUT_28_E] = 70u,
   [NV_CHANNEL_CONFIG_INPUT_29_E] = 71u,
   [NV_CHANNEL_CONFIG_INPUT_30_E] = 72u,
   [NV_CHANNEL_CONFIG_INPUT_31_E] = 73u,
   [NV_CHANNEL_CONFIG_INPUT_32_E] = 74u,
   [NV_CHANNEL_CONFIG_OUTPUT_1_E] = 75u,
   [NV_CHANNEL_CONFIG_OUTPUT_2_E] = 76u,
   [NV_CHANNEL_CONFIG_OUTPUT_3_E] = 77u,
   [NV_CHANNEL_CONFIG_OUTPUT_4_E] = 78u,
   [NV_CHANNEL_CONFIG_OUTPUT_5_E] = 79u,
   [NV_CHANNEL_CONFIG_OUTPUT_6_E] = 80u,
   [NV_CHANNEL_CONFIG_OUTPUT_7_E] = 81u,
   [NV_CHANNEL_CONFIG_OUTPUT_8_E] = 82u,
   [NV_CHANNEL_CONFIG_OUTPUT_9_E] = 83u,
   [NV_CHANNEL_CONFIG_OUTPUT_10_E] = 84u,
   [NV_CHANNEL_CONFIG_OUTPUT_11_E] = 85u,
   [NV_CHANNEL_CONFIG_OUTPUT_12_E] = 86u,
   [NV_CHANNEL_CONFIG_OUTPUT_13_E] = 87u,
   [NV_CHANNEL_CONFIG_OUTPUT_14_E] = 88u,
   [NV_CHANNEL_CONFIG_OUTPUT_15_E] = 89u,
   [NV_CHANNEL_CONFIG_OUTPUT_16_E] = 90u,
   [NV_CHANNEL_CONFIG_HEAT_A1R_E] = 91u,
   [NV_SOUND_LEVEL_E] = 92u,
   [NV_MAXIMUM_RANK_E] = 93u,
   [NV_RESERVED_0_E] = 94u,
   [NV_RESERVED_1_E] = 95u,
   [NV_RESERVED_2_E] = 96u,
   [NV_RESERVED_3_E] = 97u,
   [NV_RESERVED_4_E] = 98u,
   [NV_RESERVED_5_E] = 99u,
   [NV_RESERVED_6_E] = 100u,
   [NV_RESERVED_7_E] = 101u,
   [NV_RESERVED_8_E] = 102u,
   [NV_RESERVED_9_E] = 103u,
   [NV_DULCH_WRAP_E] = 104u,
   [NV_GLOBAL_DELAY_E] = 105u,
   [NV_ZONE_MAP_E] = 106u,
   [NV_RESTART_FLAGS_E] = 109u,
   [NV_MAX_HOPS_E] = 110u,
   [NV_RSSI_MAR_E] = 111u,
   [NV_SNR_MAR_E] = 112u,
   [NV_BATTERY_PERIOD_E] = 113u,
   [NV_AVE_POLICY_E] = 114u,
   [NV_INIT_LISTEN_PERIOD_E] = 115u,
   [NV_PHASE2_DURATION_E] = 116u,
   [NV_PHASE2_SLEEP_DURATION_E] = 117u,
   [NV_PHASE3_SLEEP_DURATION_E] = 118u,
   [NV_FREQUENCY_BAND_E] = 119u,
   [NV_MAX_RBU_CHILDREN_E] = 120u,
   [NV_NUMBER_MSG_RESENDS_E] = 121u,
   [NV_PRODUCT_CODE_E] = 122u,
   [NV_BAT_PASSIVATION_SETTINGS_E] = 123u,
   [NV_ENABLE_NO_FAULT_REPORT_E] = 124u,
	[NV_PIR_ACTIVATE_PERIOD_E] = 125u,
	[NV_PIR_DEACTIVATE_PERIOD_E] = 126u,
	[NV_PIR_STRIKE_COUNT_E] = 127u,
	[NV_BAT_PRI_LOW_VOLT_THRESH_E] = 128u,   /* For SmartNet */
   [NV_BAT_BKP_LOW_VOLT_THRESH_E] = 129u,   /* For SmartNet */
   [NV_BAT_PRI_LOW_VOLT_THRESH_C_E] = 130u, /* For construction */
   [NV_BAT_BKP_LOW_VOLT_THRESH_C_E] = 131u, /* For construction */
   [NV_FW_VERSION_E] = 132u,
};

/* Size of each variable in words (4 bytes) */
const uint16_t DM_NVM_ParameterSize[NV_MAX_PARAM_ID_E] = 
{
   [NV_REPROG_USE_PP_UART_E] = 1u,
   [NV_ADDRESS_E] = 1u,
   [NV_IS_SYNC_MASTER_E] = 1u,
   [NV_FREQUENCY_CHANNEL_E] = 1u,
   [NV_DEVICE_COMBINATION_E] = 1u,
   [NV_SYSTEM_ID_E] = 1u,
   [NV_UNIT_SERIAL_NO_E] = 1u,
   [NV_TX_POWER_LOW_E] = 1u,
   [NV_TX_POWER_HIGH_E] = 1u,
   [NV_ZONE_NUMBER_E] = 1u,
   [NV_PP_MODE_ENABLE_E] = 1u,
   [NV_BRANDING_ID_E] = 1u,
   [NV_BANK1_CRC_E] = 1u,
   [NV_BANK2_CRC_E] = 1u,
   [NV_PREFORM_MESH_PARENTS_E] = 1u,
   [NV_SHORT_FRAMES_PER_LONG_FRAME_E] = 1u,
   [NV_PROFILE_FIRE_E] = 1u,
   [NV_PROFILE_FIRST_AID_E] = 1u,
   [NV_PROFILE_EVACUATE_E] = 1u,
   [NV_PROFILE_SECURITY_E] = 1u,
   [NV_PROFILE_GENERAL_E] = 1u,
   [NV_PROFILE_FAULT_E] = 1u,
   [NV_PROFILE_ROUTING_ACK_E] = 1u,
   [NV_PROFILE_TEST_E] = 1u,
   [NV_PROFILE_SILENT_TEST_E] = 1u,
   [NV_IOU_INPUT_SHORT_THRESHOLD_E] = 1u,
   [NV_IOU_INPUT_LOGIC_THRESHOLD_E] = 1u,
   [NV_IOU_INPUT_OPEN_THRESHOLD_E] = 1u,
   [NV_IOU_INPUT_POLL_PERIOD_E] = 1u,
   [NV_CHANNEL_CONFIG_SMOKE_E] = 1u,
   [NV_CHANNEL_CONFIG_HEAT_B_E] = 1u,
   [NV_CHANNEL_CONFIG_CO_E] = 1u,
   [NV_CHANNEL_CONFIG_PIR_E] = 1u,
   [NV_CHANNEL_CONFIG_SOUNDER_E] = 1u,
   [NV_CHANNEL_CONFIG_BEACON_E] = 1u,
   [NV_CHANNEL_CONFIG_FIRE_CALLPOINT_E] = 1u,
   [NV_CHANNEL_CONFIG_STATUS_LED_E] = 1u,
   [NV_CHANNEL_CONFIG_VISUAL_INDICATOR_E] = 1u,
   [NV_CHANNEL_CONFIG_VISUAL_INDICATOR_COMBINED_E] = 1u,
   [NV_CHANNEL_CONFIG_MEDICAL_CALLPOINT_E] = 1u,
   [NV_CHANNEL_CONFIG_EVAC_CALLPOINT_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_ROUTING_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_1_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_2_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_3_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_4_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_5_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_6_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_7_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_8_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_9_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_10_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_11_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_12_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_13_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_14_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_15_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_16_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_17_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_18_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_19_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_20_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_21_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_22_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_23_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_24_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_25_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_26_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_27_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_28_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_29_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_30_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_31_E] = 1u,
   [NV_CHANNEL_CONFIG_INPUT_32_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_1_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_2_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_3_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_4_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_5_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_6_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_7_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_8_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_9_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_10_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_11_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_12_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_13_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_14_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_15_E] = 1u,
   [NV_CHANNEL_CONFIG_OUTPUT_16_E] = 1u,
   [NV_CHANNEL_CONFIG_HEAT_A1R_E] = 1u,
   [NV_SOUND_LEVEL_E] = 1u,
   [NV_MAXIMUM_RANK_E] = 1u,
   [NV_RESERVED_0_E] = 1u,
   [NV_RESERVED_1_E] = 1u,
   [NV_RESERVED_2_E] = 1u,
   [NV_RESERVED_3_E] = 1u,
   [NV_RESERVED_4_E] = 1u,
   [NV_RESERVED_5_E] = 1u,
   [NV_RESERVED_6_E] = 1u,
   [NV_RESERVED_7_E] = 1u,
   [NV_RESERVED_8_E] = 1u,
   [NV_RESERVED_9_E] = 1u,
   [NV_DULCH_WRAP_E] = 1u,
   [NV_GLOBAL_DELAY_E] = 1u,
   [NV_ZONE_MAP_E] = 3u,
   [NV_RESTART_FLAGS_E] = 1u,
   [NV_MAX_HOPS_E] = 1u,
   [NV_RSSI_MAR_E] = 1u,
   [NV_SNR_MAR_E] = 1u,
   [NV_BATTERY_PERIOD_E] = 1u,
   [NV_AVE_POLICY_E] = 1u,
   [NV_INIT_LISTEN_PERIOD_E] = 1u,
   [NV_PHASE2_DURATION_E] = 1u,
   [NV_PHASE2_SLEEP_DURATION_E] = 1u,
   [NV_PHASE3_SLEEP_DURATION_E] = 1u,
   [NV_FREQUENCY_BAND_E] = 1u,
   [NV_MAX_RBU_CHILDREN_E] = 1u,
   [NV_NUMBER_MSG_RESENDS_E] = 1u,
   [NV_PRODUCT_CODE_E] = 1u,
   [NV_BAT_PASSIVATION_SETTINGS_E] = 1u,	
   [NV_ENABLE_NO_FAULT_REPORT_E] = 1u,
	[NV_PIR_ACTIVATE_PERIOD_E] = 1u,
	[NV_PIR_DEACTIVATE_PERIOD_E] = 1u,
	[NV_PIR_STRIKE_COUNT_E] = 1u,
   [NV_BAT_PRI_LOW_VOLT_THRESH_E] = 1u,   /* For SmartNet */
   [NV_BAT_BKP_LOW_VOLT_THRESH_E] = 1u,   /* For SmartNet */
   [NV_BAT_PRI_LOW_VOLT_THRESH_C_E] = 1u, /* For construction */
   [NV_BAT_BKP_LOW_VOLT_THRESH_C_E] = 1u, /* For construction */
   [NV_FW_VERSION_E] = 1u,
};

/* Default value container */
const void *const DM_NVM_DefaultValues[NV_MAX_PARAM_ID_E] = 
{
   [NV_REPROG_USE_PP_UART_E] = &nvm_reprog_use_pp_uart_default,
   [NV_ADDRESS_E] = &nvm_unit_address_default,
   [NV_IS_SYNC_MASTER_E] = &nvm_is_sync_master_default,
   [NV_FREQUENCY_CHANNEL_E] = &nvm_frequency_channel_default,
   [NV_DEVICE_COMBINATION_E] = &nvm_device_combination_default,
   [NV_SYSTEM_ID_E] = &nvm_system_id_default,
   [NV_UNIT_SERIAL_NO_E] = &nvm_unit_serial_no_default,
   [NV_TX_POWER_LOW_E] = &nvm_tx_power_low_default,
   [NV_TX_POWER_HIGH_E] = &nvm_tx_power_high_default,
   [NV_ZONE_NUMBER_E] = &nvm_zone_number_default,
   [NV_PP_MODE_ENABLE_E] = &nvm_tx_pp_mode_default,
   [NV_BRANDING_ID_E] = &nvm_branding_id_default,
   [NV_BANK1_CRC_E] = &nvm_bank1_crc,
   [NV_BANK2_CRC_E] = &nvm_bank2_crc,
   [NV_PREFORM_MESH_PARENTS_E] = &nvm_preformed_mesh_parents_default,
   [NV_SHORT_FRAMES_PER_LONG_FRAME_E] = &nvm_short_frames_per_long_frame,
   [NV_PROFILE_FIRE_E] = &nvm_profile_fire,
   [NV_PROFILE_FIRST_AID_E] = &nvm_profile_first_aid,
   [NV_PROFILE_EVACUATE_E] = &nvm_profile_evacuate,
   [NV_PROFILE_SECURITY_E] = &nvm_profile_security,
   [NV_PROFILE_GENERAL_E] = &nvm_profile_general,
   [NV_PROFILE_FAULT_E] = &nvm_profile_fault,
   [NV_PROFILE_ROUTING_ACK_E] = &nvm_profile_routing_ack,
   [NV_PROFILE_TEST_E] = &nvm_profile_test,
   [NV_PROFILE_SILENT_TEST_E] = &nvm_profile_silent_test,
   [NV_IOU_INPUT_SHORT_THRESHOLD_E] = &nvm_iou_short_threshold,
   [NV_IOU_INPUT_LOGIC_THRESHOLD_E] = &nvm_iou_logic_threshold,
   [NV_IOU_INPUT_OPEN_THRESHOLD_E] = &nvm_iou_open_threshold,
   [NV_IOU_INPUT_POLL_PERIOD_E] = &nvm_iou_poll_period,
   [NV_CHANNEL_CONFIG_SMOKE_E] = &nvm_channel_smoke,
   [NV_CHANNEL_CONFIG_HEAT_B_E] = &nvm_channel_heat_b,
   [NV_CHANNEL_CONFIG_CO_E] = &nvm_channel_co,
   [NV_CHANNEL_CONFIG_PIR_E] = &nvm_channel_pir,
   [NV_CHANNEL_CONFIG_SOUNDER_E] = &nvm_channel_sounder,
   [NV_CHANNEL_CONFIG_BEACON_E] = &nvm_channel_beacon,
   [NV_CHANNEL_CONFIG_FIRE_CALLPOINT_E] = &nvm_channel_fire_callpoint,
   [NV_CHANNEL_CONFIG_STATUS_LED_E] = &nvm_channel_status_led,
   [NV_CHANNEL_CONFIG_VISUAL_INDICATOR_E] = &nvm_channel_visual_indicator,
   [NV_CHANNEL_CONFIG_VISUAL_INDICATOR_COMBINED_E] = &nvm_channel_visual_indicator_combined,
   [NV_CHANNEL_CONFIG_MEDICAL_CALLPOINT_E] = &nvm_channel_medical_callpoint,
   [NV_CHANNEL_CONFIG_EVAC_CALLPOINT_E] = &nvm_channel_evac_callpoint,
   [NV_CHANNEL_CONFIG_OUTPUT_ROUTING_E] = &nvm_channel_output_routing,
   [NV_CHANNEL_CONFIG_INPUT_1_E] = &nvm_channel_input_1,
   [NV_CHANNEL_CONFIG_INPUT_2_E] = &nvm_channel_input_2,
   [NV_CHANNEL_CONFIG_INPUT_3_E] = &nvm_channel_input_3,
   [NV_CHANNEL_CONFIG_INPUT_4_E] = &nvm_channel_input_4,
   [NV_CHANNEL_CONFIG_INPUT_5_E] = &nvm_channel_input_5,
   [NV_CHANNEL_CONFIG_INPUT_6_E] = &nvm_channel_input_6,
   [NV_CHANNEL_CONFIG_INPUT_7_E] = &nvm_channel_input_7,
   [NV_CHANNEL_CONFIG_INPUT_8_E] = &nvm_channel_input_8,
   [NV_CHANNEL_CONFIG_INPUT_9_E] = &nvm_channel_input_9,
   [NV_CHANNEL_CONFIG_INPUT_10_E] = &nvm_channel_input_10,
   [NV_CHANNEL_CONFIG_INPUT_11_E] = &nvm_channel_input_11,
   [NV_CHANNEL_CONFIG_INPUT_12_E] = &nvm_channel_input_12,
   [NV_CHANNEL_CONFIG_INPUT_13_E] = &nvm_channel_input_13,
   [NV_CHANNEL_CONFIG_INPUT_14_E] = &nvm_channel_input_14,
   [NV_CHANNEL_CONFIG_INPUT_15_E] = &nvm_channel_input_15,
   [NV_CHANNEL_CONFIG_INPUT_16_E] = &nvm_channel_input_16,
   [NV_CHANNEL_CONFIG_INPUT_17_E] = &nvm_channel_input_17,
   [NV_CHANNEL_CONFIG_INPUT_18_E] = &nvm_channel_input_18,
   [NV_CHANNEL_CONFIG_INPUT_19_E] = &nvm_channel_input_19,
   [NV_CHANNEL_CONFIG_INPUT_20_E] = &nvm_channel_input_20,
   [NV_CHANNEL_CONFIG_INPUT_21_E] = &nvm_channel_input_21,
   [NV_CHANNEL_CONFIG_INPUT_22_E] = &nvm_channel_input_22,
   [NV_CHANNEL_CONFIG_INPUT_23_E] = &nvm_channel_input_23,
   [NV_CHANNEL_CONFIG_INPUT_24_E] = &nvm_channel_input_24,
   [NV_CHANNEL_CONFIG_INPUT_25_E] = &nvm_channel_input_25,
   [NV_CHANNEL_CONFIG_INPUT_26_E] = &nvm_channel_input_26,
   [NV_CHANNEL_CONFIG_INPUT_27_E] = &nvm_channel_input_27,
   [NV_CHANNEL_CONFIG_INPUT_28_E] = &nvm_channel_input_28,
   [NV_CHANNEL_CONFIG_INPUT_29_E] = &nvm_channel_input_28,
   [NV_CHANNEL_CONFIG_INPUT_30_E] = &nvm_channel_input_30,
   [NV_CHANNEL_CONFIG_INPUT_31_E] = &nvm_channel_input_31,
   [NV_CHANNEL_CONFIG_INPUT_32_E] = &nvm_channel_input_32,
   [NV_CHANNEL_CONFIG_OUTPUT_1_E] = &nvm_channel_output_1,
   [NV_CHANNEL_CONFIG_OUTPUT_2_E] = &nvm_channel_output_2,
   [NV_CHANNEL_CONFIG_OUTPUT_3_E] = &nvm_channel_output_3,
   [NV_CHANNEL_CONFIG_OUTPUT_4_E] = &nvm_channel_output_4,
   [NV_CHANNEL_CONFIG_OUTPUT_5_E] = &nvm_channel_output_5,
   [NV_CHANNEL_CONFIG_OUTPUT_6_E] = &nvm_channel_output_6,
   [NV_CHANNEL_CONFIG_OUTPUT_7_E] = &nvm_channel_output_7,
   [NV_CHANNEL_CONFIG_OUTPUT_8_E] = &nvm_channel_output_8,
   [NV_CHANNEL_CONFIG_OUTPUT_9_E] = &nvm_channel_output_9,
   [NV_CHANNEL_CONFIG_OUTPUT_10_E] = &nvm_channel_output_10,
   [NV_CHANNEL_CONFIG_OUTPUT_11_E] = &nvm_channel_output_11,
   [NV_CHANNEL_CONFIG_OUTPUT_12_E] = &nvm_channel_output_12,
   [NV_CHANNEL_CONFIG_OUTPUT_13_E] = &nvm_channel_output_13,
   [NV_CHANNEL_CONFIG_OUTPUT_14_E] = &nvm_channel_output_14,
   [NV_CHANNEL_CONFIG_OUTPUT_15_E] = &nvm_channel_output_15,
   [NV_CHANNEL_CONFIG_OUTPUT_16_E] = &nvm_channel_output_16,
   [NV_CHANNEL_CONFIG_HEAT_A1R_E] = &nvm_channel_heat_a1r,
   [NV_SOUND_LEVEL_E] = &nvm_sound_level,
   [NV_MAXIMUM_RANK_E] = &nvm_maximum_rank,
   [NV_RESERVED_0_E] = &nvm_reserved_0,
   [NV_RESERVED_1_E] = &nvm_reserved_1,
   [NV_RESERVED_2_E] = &nvm_reserved_2,
   [NV_RESERVED_3_E] = &nvm_reserved_3,
   [NV_RESERVED_4_E] = &nvm_reserved_4,
   [NV_RESERVED_5_E] = &nvm_reserved_5,
   [NV_RESERVED_6_E] = &nvm_reserved_6,
   [NV_RESERVED_7_E] = &nvm_reserved_7,
   [NV_RESERVED_8_E] = &nvm_reserved_8,
   [NV_RESERVED_9_E] = &nvm_reserved_9,
   [NV_DULCH_WRAP_E] = &nvm_dulch_wrap,
   [NV_GLOBAL_DELAY_E] = &nvm_global_delay,
   [NV_ZONE_MAP_E] = &nvm_zone_map,
   [NV_RESTART_FLAGS_E] = &nvm_restart_flags,
   [NV_MAX_HOPS_E] = &nvm_max_hops,
   [NV_RSSI_MAR_E] = &nvm_rssi_mar,
   [NV_SNR_MAR_E] = &nvm_snr_mar,
   [NV_BATTERY_PERIOD_E] = &nvm_battery_period,
   [NV_AVE_POLICY_E] = &nvm_snr_rssi_ave_policy,
   [NV_INIT_LISTEN_PERIOD_E] = &nvm_initial_listen_period,
   [NV_PHASE2_DURATION_E] = &nvm_phase2_duration,
   [NV_PHASE2_SLEEP_DURATION_E] = &nvm_phase2_sleep_duration,
   [NV_PHASE3_SLEEP_DURATION_E] = &nvm_phase3_sleep_duration,
   [NV_FREQUENCY_BAND_E] = &nvm_frequency_band,
   [NV_MAX_RBU_CHILDREN_E] = &nvm_max_rbu_children,
   [NV_NUMBER_MSG_RESENDS_E] = &nvm_number_of_resends,
   [NV_PRODUCT_CODE_E] = &nvm_product_code,
   [NV_BAT_PASSIVATION_SETTINGS_E] = &nvm_battery_passivation_settings,
   [NV_ENABLE_NO_FAULT_REPORT_E] = &nvm_enable_no_fault_report,
   [NV_PIR_ACTIVATE_PERIOD_E] = &nvm_pir_activate_period_us,
   [NV_PIR_DEACTIVATE_PERIOD_E] = &nvm_pir_deactivate_period_us,
   [NV_PIR_STRIKE_COUNT_E] = &nvm_pir_strike_count,
   [NV_BAT_PRI_LOW_VOLT_THRESH_E] = &nvm_bat_pri_low_volt_thresh,     /* For SmartNet */
   [NV_BAT_BKP_LOW_VOLT_THRESH_E] = &nvm_bat_bkp_low_volt_thresh,     /* For SmartNet */
   [NV_BAT_PRI_LOW_VOLT_THRESH_C_E] = &nvm_bat_pri_low_volt_thresh_c, /* For construction */
   [NV_BAT_BKP_LOW_VOLT_THRESH_C_E] = &nvm_bat_bkp_low_volt_thresh_c, /* For construction */
   [NV_FW_VERSION_E] = &nvm_fw_version_int,
};
