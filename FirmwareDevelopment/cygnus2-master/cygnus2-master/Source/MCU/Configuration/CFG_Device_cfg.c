/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2019 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : CFG_Device_cfg.c
*
*  Description  : Source for the device configuration functions
*
*************************************************************************************/

/* System Include Files
*************************************************************************************/

/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "DM_NVM.h"
#include "DM_Device.h"
#include "DM_svi.h"
#include "MM_Main.h"
#include "CFG_Device_cfg.h"
#include "DM_BatteryMonitor.h"


/* private Structures
*************************************************************************************/ 



/* private Enumerations
*************************************************************************************/

/* private Constants
*************************************************************************************/
#define CFG_MAX_RSSI_MAR              10
#define SENSITIVITY_RSSI_0           -117       // The reference sensitivity against which attenuation reserve applies for a parent SF6, as per EN54-25 Table B.1.
#define SENSITIVITY_RSSI_1           -122       // The reference sensitivity against which attenuation reserve applies for single parent SF7, as per EN54-25 Table B.1.
#define SENSITIVITY_RSSI_2           -127       // The reference sensitivity against which attenuation reserve applies for two parents SF7, as per EN54-25 Table B.1.
#define SENSITIVITY_RSSI_3           -132       // The reference sensitivity against which attenuation reserve applies for two parents SF7, as per EN54-25 Table B.1.
#define SENSITIVITY_RSSI_4           -112       // The reference sensitivity against which attenuation reserve applies for two parents SF7, as per EN54-25 Table B.1.
#define SENSITIVITY_RSSI_5           -107       // The reference sensitivity against which attenuation reserve applies for two parents SF7, as per EN54-25 Table B.1.
#define SENSITIVITY_RSSI_6           -102       // The reference sensitivity against which attenuation reserve applies for two parents SF7, as per EN54-25 Table B.1.
#define SENSITIVITY_RSSI_7           -97       // The reference sensitivity against which attenuation reserve applies for two parents SF7, as per EN54-25 Table B.1.
#define SENSITIVITY_RSSI_8           -92       // The reference sensitivity against which attenuation reserve applies for two parents SF7, as per EN54-25 Table B.1.
#define SENSITIVITY_RSSI_9           -87       // The reference sensitivity against which attenuation reserve applies for two parents SF7, as per EN54-25 Table B.1.
#define SENSITIVITY_RSSI_10          -82       // The reference sensitivity against which attenuation reserve applies for two parents SF7, as per EN54-25 Table B.1.
#define MIN_ATTEN_RESERVE             10        // Should be set to match appropriate entry in EN54-25 Table B.1
#define MIN_CANDIDATE_RSSI_0         (SENSITIVITY_RSSI_0 + MIN_ATTEN_RESERVE)   // Threshold for selecting initial tracking node.
#define MIN_CANDIDATE_RSSI_1         (SENSITIVITY_RSSI_1 + MIN_ATTEN_RESERVE)   // Threshold for selecting candidate single parent nodes.
#define MIN_CANDIDATE_RSSI_2         (SENSITIVITY_RSSI_2 + MIN_ATTEN_RESERVE)   // Threshold for selecting candidate two-parent nodes.
#define MIN_CANDIDATE_RSSI_3         (SENSITIVITY_RSSI_3 + MIN_ATTEN_RESERVE)   // Threshold for selecting candidate two-parent nodes.
#define MIN_CANDIDATE_RSSI_4         (SENSITIVITY_RSSI_4 + MIN_ATTEN_RESERVE)   // Threshold for selecting candidate two-parent nodes.
#define MIN_CANDIDATE_RSSI_5         (SENSITIVITY_RSSI_5 + MIN_ATTEN_RESERVE)   // Threshold for selecting candidate two-parent nodes.
#define MIN_CANDIDATE_RSSI_6         (SENSITIVITY_RSSI_6 + MIN_ATTEN_RESERVE)   // Threshold for selecting candidate two-parent nodes.
#define MIN_CANDIDATE_RSSI_7         (SENSITIVITY_RSSI_7 + MIN_ATTEN_RESERVE)   // Threshold for selecting candidate two-parent nodes.
#define MIN_CANDIDATE_RSSI_8         (SENSITIVITY_RSSI_8 + MIN_ATTEN_RESERVE)   // Threshold for selecting candidate two-parent nodes.
#define MIN_CANDIDATE_RSSI_9         (SENSITIVITY_RSSI_9 + MIN_ATTEN_RESERVE)   // Threshold for selecting candidate two-parent nodes.
#define MIN_CANDIDATE_RSSI_10        (SENSITIVITY_RSSI_10 + MIN_ATTEN_RESERVE)   // Threshold for selecting candidate two-parent nodes.
#define MIN_CANDIDATE_SNR             5
//Depassivation constants
#define CFG_DEPASSIVATION_VOLTAGE_MAX 0xFFF
#define CFG_DEPASSIVATION_TIMEOUT_MAX 0x3FF
#define CFG_DEPASSIVATION_ON_TIME_MAX 0x1F
#define CFG_DEPASSIVATION_OFF_TIME_MAX 0x1F
#define CFG_DEPASSIVATION_VOLTAGE_MASK 0x00000FFF
#define CFG_DEPASSIVATION_TIMEOUT_MASK 0x003FF000
#define CFG_DEPASSIVATION_ON_TIME_MASK 0x07C00000
#define CFG_DEPASSIVATION_OFF_TIME_MASK 0xF8000000

static uint32_t gZoneNumber;
static ZoneDisabledMap_t gZoneMap;
static bool gZoneEnabled = true;
static bool gRbuEnabled = true;
static CO_DayNight_t gDayNight = CO_DAY_E;
static bool gGlobalDelayEnabled = true;
static uint8_t gSoundLevel = SOUND_LEVEL_MAX_E;
static bool gFaultReportsEnabled = true;
static uint32_t gDeviceCombination = 0;
static uint32_t gDulchWrap = MAX_DULCH_WRAP;
static uint16_t zGlobalDelay1;
static uint16_t zGlobalDelay2;
static uint32_t gMaxHops = MAX_MSG_HOP_THRESHOLD;
static bool gBatteryChecksEnabled = ENABLE_BATTERY_CHECKS;
static bool gGlobalDelayOverrideEnabled = false;
static uint32_t gRssiMarSetting = 0;
static int32_t gSnrMarSetting = 0;
static SnrRssiAveragingPolicy_t gSnrRssiAveragePolicy = CFG_AVE_DEFAULT_E;
static bool gSecurityProfileInhibitedDay = false;
static bool gSecurityProfileInhibitedNight = false;
static bool gUsingPreformedMesh = false;
static uint32_t gPreformPrimaryParent = CO_BAD_NODE_ID;
static uint32_t gPreformSecondaryParent = CO_BAD_NODE_ID;
static uint32_t gBatteryTestInterval = DEFAULT_BATTERY_TEST_INTERVAL;
static uint32_t gInitialListenPeriod;
static uint32_t gPhase2Period;
static uint32_t gPhase2SleepPeriod;
static uint32_t gPhase3SleepPeriod;
static bool gSVI_PowerLock = false;
static bool gPollingDisabled = false;
static bool gBlankingPlateFitted = false;
static uint32_t gMaxChildren = DEFAULT_CHILDREN_PER_PARENT;
static uint32_t gNumberOfResends = DEFAULT_NUMBER_OF_RESENDS;
static DePassivationSettings_t gDepassivationSettings;
static uint32_t gSoftwareVersionNumber = 0;
static uint32_t gPpuMode = STARTUP_NORMAL;
static uint32_t gSendFaultsClearMsg = false;
static uint32_t gBatPriLowVoltThresh = DM_BAT_MCP_FAIL_THRESHOLD_MV;
static uint32_t gBatBkpLowVoltThresh = DM_BAT_MCP_FAIL_THRESHOLD_MV;
static uint32_t gBatPriLowVoltThreshC = DM_BAT_CONSTRUCTION_PRIMARY_FAIL_THRESHOLD_MV; /* For construction */
static uint32_t gBatBkpLowVoltThreshC = DM_BAT_CONSTRUCTION_BACKUP_FAIL_THRESHOLD_MV;  /* For construction */

static uint32_t gIsSyncMaster;
#ifdef APPLY_RSSI_OFFSET
#define RSSI_OFFSET_SIZE 10
int8_t gRssiOffset[RSSI_OFFSET_SIZE];
#endif
/* Macros
*************************************************************************************/

/* Private Function declarations
*************************************************************************************/
static ErrorCode_t CFG_WriteZoneMapToNVM(void);
static bool CFG_GetLocalOrGlobalDelayFlag(const CO_ChannelIndex_t systemChannel);

/* Public declarations
*************************************************************************************/

/* Public Functions
*************************************************************************************/
ErrorCode_t CFG_InitialiseDeviceConfig(void)
{
   ErrorCode_t result = SUCCESS_E;
   
#ifdef APPLY_RSSI_OFFSET
   for (uint32_t index = 0; index < RSSI_OFFSET_SIZE; index++)
   {
      gRssiOffset[index] = 0;
   }
#endif

   //Read whether to send the 'faults clear' message on joining
   if ( SUCCESS_E != DM_NVMRead(NV_ENABLE_NO_FAULT_REPORT_E, &gSendFaultsClearMsg, sizeof(gSendFaultsClearMsg)))
   {
      CO_PRINT_B_0(DBG_ERROR_E,"CFG: Failed to read no-faults msg enable\r\n");
      result = ERR_INIT_FAIL_E;
   }
   CO_PRINT_A_1(DBG_INFO_E,"CFG: No-faults msg enable = %d\r\n", gSendFaultsClearMsg);

   //Read Zone number from NVM
   if ( SUCCESS_E != DM_NVMRead(NV_ZONE_NUMBER_E, &gZoneNumber, sizeof(gZoneNumber)))
   {
      CO_PRINT_B_0(DBG_ERROR_E,"CFG: Failed to read zone number\r\n");
      result = ERR_INIT_FAIL_E;
   }
   
   //Read the zone disablement from NVM
   if ( SUCCESS_E == DM_NVMRead(NV_ZONE_MAP_E, (void*)&gZoneMap, sizeof(gZoneMap)) )
   {
      //Check if the current zone is disabled
      gZoneEnabled = CFG_RequestedZoneEnabled(gZoneNumber);
   }
   else
   {
      CO_PRINT_B_0(DBG_ERROR_E,"CFG: Failed to read zone disablement map\r\n");
      result = ERR_INIT_FAIL_E;
   }
   
   
   //Read the sound level for the sounder channel
   uint32_t sound_level;
   if ( SUCCESS_E == DM_NVMRead(NV_SOUND_LEVEL_E, &sound_level, sizeof(sound_level)) )
   {
      gSoundLevel = (uint8_t)sound_level;
      if ( SOUND_LEVEL_MAX_E < gSoundLevel )
      {
         gSoundLevel = SOUND_LEVEL_MAX_E;
      }
   }
   else 
   {
      CO_PRINT_B_0(DBG_ERROR_E,"CFG: Failed to read sound level\r\n");
      result = ERR_INIT_FAIL_E;
   }
   
   uint32_t deviceCombination;
   if ( SUCCESS_E == DM_NVMRead(NV_DEVICE_COMBINATION_E, &deviceCombination, sizeof(deviceCombination)) )
   {
      if ( DC_NUMBER_OF_DEVICE_COMBINATIONS_E > deviceCombination )
      {
         gDeviceCombination = deviceCombination;
      }
      else 
      {
         CO_PRINT_B_1(DBG_ERROR_E,"CFG: NVM device combination out of range = %d\r\n", deviceCombination);
      }
   }
   else 
   {
      CO_PRINT_B_0(DBG_ERROR_E,"CFG: Failed to read device combination\r\n");
   }

   //Read the DULCH wrap
   uint32_t dulch_wrap;
   if ( SUCCESS_E == DM_NVMRead(NV_DULCH_WRAP_E, &dulch_wrap, sizeof(dulch_wrap)) )
   {
      gDulchWrap = dulch_wrap;
      if ( (0 == gDulchWrap) || (MAX_DULCH_WRAP < gDulchWrap) )
      {
         gDulchWrap = MAX_DULCH_WRAP;
      }
   }
   else 
   {
      CO_PRINT_B_0(DBG_ERROR_E,"CFG: Failed to read DULCH wrap\r\n");
      result = ERR_INIT_FAIL_E;
   }

   //Read the global delays
   uint32_t global_delays;
   if ( SUCCESS_E == DM_NVMRead(NV_GLOBAL_DELAY_E, &global_delays, sizeof(global_delays)) )
   {
      zGlobalDelay1 = (uint16_t)(dulch_wrap & NVM_GLOBAL_OUTPUT_DELAY1_MASK);
      zGlobalDelay2 = (uint16_t)((dulch_wrap & NVM_GLOBAL_OUTPUT_DELAY1_MASK) >> 16);
   }
   else 
   {
      CO_PRINT_B_0(DBG_ERROR_E,"CFG: Failed to read DULCH wrap\r\n");
      result = ERR_INIT_FAIL_E;
   }
   //Default to local alarm delays
   gGlobalDelayOverrideEnabled = false;

   //Read the Max Hops
   uint32_t max_hops;
   if ( SUCCESS_E == DM_NVMRead(NV_MAX_HOPS_E, &max_hops, sizeof(max_hops)) )
   {
      if ( 0 == max_hops )
      {
         //Set gMaxHops using the setter function.  This will update the NVM.
         if ( false == CFG_SetMaxHops(MAX_MSG_HOP_THRESHOLD) )
         {
            //Write to NVM failed.  Set the dynamic value.
            gMaxHops = MAX_MSG_HOP_THRESHOLD;
         }
      }
      else 
      {
         gMaxHops = max_hops;
      }
   }
   else 
   {
      CO_PRINT_B_0(DBG_ERROR_E,"CFG: Failed to read Max Hops\r\n");
      result = ERR_INIT_FAIL_E;
   }
   
   //Read the RSSI joining threshold
   uint32_t rssi_mar;
   if ( SUCCESS_E == DM_NVMRead(NV_RSSI_MAR_E, &rssi_mar, sizeof(rssi_mar)) )
   {
      gRssiMarSetting = rssi_mar;
      if ( CFG_MAX_RSSI_MAR < gRssiMarSetting )//invalid, use SF7 single parent threshold (1)
      {
         gRssiMarSetting =1;
      }
   }
   else 
   {
      CO_PRINT_B_0(DBG_ERROR_E,"CFG: Failed to read RSSI joining threshold\r\n");
      result = ERR_INIT_FAIL_E;
   }
   
   //Read the SNR joining threshold
   uint32_t snr_mar;
   if ( SUCCESS_E == DM_NVMRead(NV_SNR_MAR_E, &snr_mar, sizeof(snr_mar)) )
   {
      gSnrMarSetting = (int32_t)snr_mar;
   }
   else 
   {
      gSnrMarSetting = MIN_CANDIDATE_SNR;
      CO_PRINT_B_1(DBG_ERROR_E,"CFG: Failed to read SNR joining threshold.  Using default %d\r\n", MIN_CANDIDATE_SNR);
      result = ERR_INIT_FAIL_E;
   }
   
   //Set up the attributes for using a preformed mesh
   uint32_t preformed_parents;
   if ( SUCCESS_E == DM_NVMRead(NV_PREFORM_MESH_PARENTS_E, &preformed_parents, sizeof(preformed_parents)) )
   {
      gPreformPrimaryParent = preformed_parents & 0xFFFF;//Low order 16 bits
      gPreformSecondaryParent = preformed_parents >> 16;//High order 16 bits.
   }
   
   
   gUsingPreformedMesh = false;
   //Check whether a preformed mesh has been designated.  The primary parent will be set to CO_BAD_NODE_ID
   //if preformed mesh is disabled.  The exception is when a board in new and unprogrammed.  This is detected by
   //checking the secondary parent for 0.  The secondary parent can never be the NCU, so it's only 0 if unprogrammed.
   if ( CO_BAD_NODE_ID != gPreformPrimaryParent && 0 != gPreformSecondaryParent)
   {
      gUsingPreformedMesh = true;
      CO_PRINT_B_2(DBG_INFO_E, "Preformed mesh PP=%d, SP=%d\n\r", gPreformPrimaryParent, gPreformSecondaryParent);
   }


   //Read the battery test interval from NVM
   uint32_t bat_interval;
   ErrorCode_t status = DM_NVMRead(NV_BATTERY_PERIOD_E, &bat_interval, sizeof(bat_interval));
   if ( SUCCESS_E == status )
   {
      gBatteryTestInterval = bat_interval;
   }
   else 
   {
      gBatteryTestInterval = DEFAULT_BATTERY_TEST_INTERVAL;
      CO_PRINT_B_2(DBG_INFO_E,"Failed to read battery test interval. Error %d.  Set to default value %d days\r\n", status, DEFAULT_BATTERY_TEST_INTERVAL);
      result = ERR_INIT_FAIL_E;
   }
   
   //Read Initial Listen period
   if ( SUCCESS_E != DM_NVMRead(NV_INIT_LISTEN_PERIOD_E, &gInitialListenPeriod, sizeof(gInitialListenPeriod)))
   {
      CO_PRINT_B_1(DBG_ERROR_E,"CFG: Failed to read initial listen period defaulting to %d hours\r\n", LOW_POWER_INITIAL_AWAKE_DURATION);
      gInitialListenPeriod = LOW_POWER_INITIAL_AWAKE_DURATION * SECONDS_IN_ONE_HOUR;
      result = ERR_INIT_FAIL_E;
   }
   else 
   {
      //Stored in hours, but tested in seconds
      gInitialListenPeriod *= SECONDS_IN_ONE_HOUR;
   }

   //Read Phase 2 duration ( time that the device stays in phase 2 )
   if ( SUCCESS_E != DM_NVMRead(NV_PHASE2_DURATION_E, &gPhase2Period, sizeof(gPhase2Period)))
   {
      CO_PRINT_B_1(DBG_ERROR_E,"CFG: Failed to read initial low power phase 2 duration defaulting to %d days\r\n", LOW_POWER_PHASE2_DURATION);
      gPhase2Period = LOW_POWER_PHASE2_DURATION * SECONDS_IN_ONE_DAY;
      result = ERR_INIT_FAIL_E;
   }
   else 
   {
      //Stored in days, but tested in seconds
      gPhase2Period *= SECONDS_IN_ONE_DAY;
   }

   //Read Phase 2 sleep duration ( time that the device sleeps in low power on start-up mode, phase 2 )
   if ( SUCCESS_E != DM_NVMRead(NV_PHASE2_SLEEP_DURATION_E, &gPhase2SleepPeriod, sizeof(gPhase2SleepPeriod)))
   {
      CO_PRINT_B_1(DBG_ERROR_E,"CFG: Failed to read initial low power phase 2 sleep period defaulting to %d hours\r\n", LOW_POWER_PHASE2_SLEEP_DURATION);
      gPhase2SleepPeriod = LOW_POWER_PHASE2_SLEEP_DURATION * SECONDS_IN_ONE_HOUR;
      result = ERR_INIT_FAIL_E;
   }
   else 
   {
      //Stored in hours, but tested in seconds
      gPhase2SleepPeriod *= SECONDS_IN_ONE_HOUR;
   }

   //Read Phase 3 sleep duration ( time that the device sleeps in low power on start-up mode, phase 3 )
   if ( SUCCESS_E != DM_NVMRead(NV_PHASE3_SLEEP_DURATION_E, &gPhase3SleepPeriod, sizeof(gPhase3SleepPeriod)))
   {
      CO_PRINT_B_1(DBG_ERROR_E,"CFG: Failed to read initial low power phase 3 sleep period defaulting to %d hours\r\n", LOW_POWER_PHASE3_SLEEP_DURATION);
      gPhase3SleepPeriod = LOW_POWER_PHASE3_SLEEP_DURATION * SECONDS_IN_ONE_HOUR;
      result = ERR_INIT_FAIL_E;
   }
   else 
   {
      //Stored in hours, but tested in seconds
      gPhase3SleepPeriod *= SECONDS_IN_ONE_HOUR;
   }
   
   // Initialise blanking plate tmper checks if no plug-in is specified in the device combination
   if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAD_DEVICES, DC_MATCH_ANY_E))
   {
      //disable blanking plate tamper checks
      CFG_SetBlankingPlateFitted( false );
   }
   else 
   {
      //No plug-in fitted.  Enable banking plate tamper check
      CFG_SetBlankingPlateFitted( true );
   }
   
   if ( SUCCESS_E != DM_NVMRead(NV_IS_SYNC_MASTER_E, &gIsSyncMaster, sizeof(gIsSyncMaster)))
   {
      CO_PRINT_B_0(DBG_ERROR_E,"CFG: Failed to read Sync Master\r\n");
      gIsSyncMaster = 0;
      result = ERR_INIT_FAIL_E;
   }

   gBatteryChecksEnabled = ENABLE_BATTERY_CHECKS;
   
   //Read Max number of children
   if ( BASE_NCU_E == DM_DeviceGetBaseType(deviceCombination, gIsSyncMaster) )
   {
      gMaxChildren = MAX_CHILDREN_PER_NCU_PARENT;
   }
   else 
   {
      //Read Max number of children from NVM
      if ( SUCCESS_E != DM_NVMRead(NV_MAX_RBU_CHILDREN_E, &gMaxChildren, sizeof(gMaxChildren)))
      {
         CO_PRINT_B_1(DBG_ERROR_E,"CFG: Failed to read max number of children.  Defaulted to %d\r\n", MAX_CHILDREN_PER_PARENT);
         //NVM read failed.  use default.
         gMaxChildren = DEFAULT_CHILDREN_PER_PARENT;
         result = ERR_INIT_FAIL_E;
      }
   }

   //Read the number of times that downlink messages should be sent
   if ( SUCCESS_E != DM_NVMRead(NV_NUMBER_MSG_RESENDS_E, &gNumberOfResends, sizeof(gNumberOfResends)))
   {
      CO_PRINT_B_1(DBG_ERROR_E,"CFG: Failed to read the number of downlink resends.  Defaulted to %d\r\n", DEFAULT_NUMBER_OF_RESENDS);
      //NVM read failed.  use default.
      gNumberOfResends = DEFAULT_NUMBER_OF_RESENDS;
      result = ERR_INIT_FAIL_E;
   }
   
   //Read the depassivation settings for battery management
   uint32_t depassivation_settings;
   if ( SUCCESS_E != DM_NVMRead(NV_BAT_PASSIVATION_SETTINGS_E, &depassivation_settings, sizeof(depassivation_settings)))
   {
      CO_PRINT_B_0(DBG_ERROR_E,"CFG: Failed to read depassivation.  Using default\r\n");
      //NVM read failed.  use default.
      CFG_SetBatteryPassivationValuesCompound(DEFAULT_PASSIVATION_SETTINGS);
      result = ERR_INIT_FAIL_E;
   }
   else 
   {
      result = CFG_DecompressDepassivationSettings(depassivation_settings, &gDepassivationSettings);
   }
   
   /* Read the primary battery low voltage threshold from NVM for SmartNet */
   uint32_t priLowVoltThresh;
   ErrorCode_t status1 = DM_NVMRead(NV_BAT_PRI_LOW_VOLT_THRESH_E, &priLowVoltThresh, sizeof(priLowVoltThresh));
   if ( SUCCESS_E == status1 )
   {
      gBatPriLowVoltThresh = priLowVoltThresh;
   }
   else
   {
      gBatPriLowVoltThresh = DM_BAT_MCP_FAIL_THRESHOLD_MV;
      CO_PRINT_B_2(DBG_INFO_E,"Failed to read primary battery low voltage threshold. Error %d. Defaulted to %d\r\n", status1, DM_BAT_MCP_FAIL_THRESHOLD_MV);
      result = ERR_INIT_FAIL_E;
   }
   CO_PRINT_B_2(DBG_INFO_E,"CFG_Device_cfg: result= %d, gBatPriLowVoltThresh= %d\r\n", status1, gBatPriLowVoltThresh);

   /* Read the backup battery low voltage threshold from NVM for SmartNet */
   uint32_t bkpLowVoltThresh;
   ErrorCode_t status2 = DM_NVMRead(NV_BAT_BKP_LOW_VOLT_THRESH_E, &bkpLowVoltThresh, sizeof(bkpLowVoltThresh));
   if ( SUCCESS_E == status2 )
   {
      gBatBkpLowVoltThresh = bkpLowVoltThresh;
   }
   else
   {
      gBatBkpLowVoltThresh = DM_BAT_MCP_FAIL_THRESHOLD_MV;
      CO_PRINT_B_2(DBG_INFO_E,"Failed to read backup battery low voltage threshold. Error %d. Defaulted to %d\r\n", status2, DM_BAT_MCP_FAIL_THRESHOLD_MV);
      result = ERR_INIT_FAIL_E;
   }
   CO_PRINT_B_2(DBG_INFO_E,"CFG_Device_cfg: result= %d, gBatBkpLowVoltThresh= %d\r\n", status2, gBatBkpLowVoltThresh);

   /* Read the primary battery low voltage threshold from NVM for Construction (SiteNet) */
   uint32_t priLowVoltThreshC;
   ErrorCode_t status3 = DM_NVMRead(NV_BAT_PRI_LOW_VOLT_THRESH_C_E, &priLowVoltThreshC, sizeof(priLowVoltThreshC));
   if ( SUCCESS_E == status3 )
   {
      gBatPriLowVoltThreshC = priLowVoltThreshC;
   }
   else
   {
      gBatPriLowVoltThreshC = DM_BAT_CONSTRUCTION_PRIMARY_FAIL_THRESHOLD_MV;
      CO_PRINT_B_2(DBG_INFO_E,"Failed to read primary battery low voltage threshold. Error %d. Defaulted to %d\r\n", status3, DM_BAT_CONSTRUCTION_PRIMARY_FAIL_THRESHOLD_MV);
      result = ERR_INIT_FAIL_E;
   }
   CO_PRINT_B_2(DBG_INFO_E,"CFG_Device_cfg: result= %d, gBatPriLowVoltThreshC= %d\r\n", status3, gBatPriLowVoltThreshC);

   /* Read the backup battery low voltage threshold from NVM for Construction (SiteNet) */
   uint32_t bkpLowVoltThreshC;
   ErrorCode_t status4 = DM_NVMRead(NV_BAT_BKP_LOW_VOLT_THRESH_C_E, &bkpLowVoltThreshC, sizeof(bkpLowVoltThreshC));
   if ( SUCCESS_E == status4 )
   {
      gBatBkpLowVoltThreshC = bkpLowVoltThreshC;
   }
   else
   {
      gBatBkpLowVoltThreshC = DM_BAT_CONSTRUCTION_BACKUP_FAIL_THRESHOLD_MV;
      CO_PRINT_B_2(DBG_INFO_E,"Failed to read backup battery low voltage threshold. Error %d. Defaulted to %d\r\n", status4, DM_BAT_CONSTRUCTION_BACKUP_FAIL_THRESHOLD_MV);
      result = ERR_INIT_FAIL_E;
   }
   CO_PRINT_B_2(DBG_INFO_E,"CFG_Device_cfg: result= %d, gBatBkpLowVoltThreshC= %d\r\n", status4, gBatBkpLowVoltThreshC);

   /* pack the software version number into an integer */
   char* pVersion = (char*)sw_version_nbr;
   if ( pVersion )
   {
      /* read the base number */
      char* str_end;
      uint32_t base_number = (uint32_t)strtol(pVersion, &str_end, 10);
      
      /* read the revision number */
      pVersion = str_end;
      if ( pVersion )
      {
         /* step over the full stop */
         pVersion++;
         uint32_t revision = (uint32_t)strtol(pVersion, &str_end, 10);
         /* read the sub-revision number */
         pVersion = str_end;
         if ( pVersion )
         {
            /* step over the full stop */
            pVersion++;
            uint32_t sub_revision = (uint32_t)strtol(pVersion, &str_end, 10);
            
            /* pack into a uint32 */
            gSoftwareVersionNumber = (base_number << 14) + (revision << 7) + sub_revision;
         }
      }      
   }

   return result;
}


uint32_t encode_version_string(const char* pVersion)
{
    uint32_t gSoftwareVersionNumber = 0;

    if (pVersion)
    {
        /* read the base number */
        char* str_end;
        uint32_t base_number = (uint32_t)strtol(pVersion, &str_end, 10);

        /* read the revision number */
        pVersion = str_end;
        if (pVersion && *pVersion == '.')
        {
            /* step over the full stop */
            pVersion++;
            uint32_t revision = (uint32_t)strtol(pVersion, &str_end, 10);

            /* read the sub-revision number */
            pVersion = str_end;
            if (pVersion && *pVersion == '.')
            {
                /* step over the full stop */
                pVersion++;
                uint32_t sub_revision = (uint32_t)strtol(pVersion, &str_end, 10);

                /* pack into a uint32 */
                gSoftwareVersionNumber = (base_number << 14) + (revision << 7) + sub_revision;
            }
        }
    }

    return gSoftwareVersionNumber;
}


/*************************************************************************************/
/**
* CFG_GetLocalOrGlobalDelayFlag
* Return the Global delays flag
*
* @param  - systemChannel  : the channel to apply the config to
*
* @return - bool           : True if local delay timers should be used.
*                          : False if global delay timers should be used.
*/
bool CFG_GetLocalOrGlobalDelayFlag(const CO_ChannelIndex_t systemChannel)
{
   bool localDelay = false;
   ErrorCode_t result;
   
      //check that the channel has an associated NVM slot
      DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(systemChannel); 
      
      if ( NV_MAX_PARAM_ID_E != nvm_param )
      {
         // Read the NVM
         uint32_t nvm_value;
         result = DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value));
         if ( SUCCESS_E == result )
         {
            //Mask out the Local Delay bit from the value read from NVM
            nvm_value &= NVM_OUTPUT_LOCAL_DELAYS_MASK;
            if ( nvm_value )
            {
               localDelay = true;
            }
         }
      }

   return localDelay;
}


/*************************************************************************************/
/**
* CFG_SetZoneNumber
* Set the zone number of the device.
*
* @param  - zone     The zone number.
*
* @return - None.
*/
ErrorCode_t CFG_SetZoneNumber(const uint32_t zone)
{
   ErrorCode_t result;

   result = DM_NVMWrite(NV_ZONE_NUMBER_E, &zone, sizeof(zone));
   
   if ( SUCCESS_E == result )
   {
      gZoneNumber = zone;
   }
   
   return result;
}

/*************************************************************************************/
/**
* CFG_GetZoneNumber
* Returns the zone number of the device.
*
* @param  - None.
*
* @return - The zone number.
*/
uint32_t CFG_GetZoneNumber(void)
{
   return gZoneNumber;
}

/*************************************************************************************/
/**
* CFG_SetRbuEnabled
* Set the global enable for alarm delays.
*
* @param  - bool  True to enable all outputs, false to disable all outputs
*
* @return - None.
*/
void CFG_SetRbuEnabled(const bool enable)
{
   gRbuEnabled = enable;
   CO_PRINT_B_1(DBG_INFO_E,"Rx'd new device enable setting %d\r\n", gRbuEnabled);
}

/*************************************************************************************/
/**
* DM_DeviceEnabled
* Returns the enabled state of the RBU.
*
* @param  - None.
*
* @return - True if the RBU outputs are enabled, false if disabled.
*/
bool CFG_RbuEnabled(void)
{
   return gRbuEnabled;
}

/*************************************************************************************/
/**
* DM_OP_SetDeviceEnabled
* Set the global enable for alarm delays.
*
* @param  - bool  True to enable all outputs, false to disable all outputs
*
* @return - None.
*/
void CFG_SetZoneEnabled(const bool enable)
{
   gZoneEnabled = enable;
   CO_PRINT_B_1(DBG_INFO_E,"Rx'd new zone enable setting %d\r\n", gZoneEnabled);
}

/*************************************************************************************/
/**
* CFG_ZoneEnabled
* Returns the enabled state of the zone.
*
* @param  - None.
*
* @return - True if the zone is enabled, false if disabled.
*/
bool CFG_ZoneEnabled(void)
{
   return gZoneEnabled;
}


/*************************************************************************************/
/**
* CFG_SetDayNight
* Record the current day/night setting for use in delay decisions.
*
* @param DayNight    CO_DAY_E or CO_NIGHT_E.
*
* @return - void
*/
void CFG_SetDayNight(const CO_DayNight_t DayNight)
{
   gDayNight = DayNight;
   CO_PRINT_B_1(DBG_INFO_E,"Rx'd new day-night setting %d\r\n", gDayNight);
}

/*************************************************************************************/
/**
* CFG_GetDayNight
* Return the current day/night setting.
*
* @param None.
*
* @return - CO_DayNight_t : CO_DAY_E or CO_NIGHT_E.
*/
CO_DayNight_t CFG_GetDayNight(void)
{
   return gDayNight;
}

/*************************************************************************************/
/**
* CFG_SetGlobalDelayEnabled
* Record the current global delay setting for use in delay decisions.
*
* @param enabled    true for enabled. false for disabled.
*
* @return - void
*/
ErrorCode_t CFG_SetGlobalDelayEnabled(const bool enabled)
{
   gGlobalDelayEnabled = enabled;
   CO_PRINT_B_1(DBG_INFO_E,"Rx'd new global delay setting %d\r\n", gGlobalDelayEnabled);
   return SUCCESS_E;
}

/*************************************************************************************/
/**
* CFG_GetGlobalDelayEnabled
* Return the current global delay setting.
*
* @param None.
*
* @return - bool : true for enabled. false for disabled.
*/
bool CFG_GetGlobalDelayEnabled(void)
{
   return gGlobalDelayEnabled;
}

/*************************************************************************************/
/**
* CFG_SetSoundLevel
* Record the current sound level setting for SVI sounders.
*
* @param soundLevel    The sound level.
*
* @return - error code
*/
ErrorCode_t CFG_SetSoundLevel(const uint8_t soundLevel)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;
   uint32_t attenuation = (uint32_t)soundLevel;
   
   CO_PRINT_B_1(DBG_INFO_E,"Rx'd new sound level %d\r\n", attenuation);
   
   //Limit the max value
   if ( SOUND_LEVEL_MAX_E < attenuation )
   {
      attenuation = SOUND_LEVEL_MAX_E;
   }
   
   //Update the working value
   gSoundLevel = attenuation;
   
   //Write the new value to FLASH
   result = DM_NVMWrite(NV_SOUND_LEVEL_E, (void*)&attenuation, sizeof(attenuation));
   if ( SUCCESS_E != result )
   {
      CO_PRINT_B_0(DBG_ERROR_E,"Failed to store new sound level in NVM\r\n");
   }
   
   return result;
}

/*************************************************************************************/
/**
* CFG_GetSoundLevel
* Return the current sound level setting.
*
* @param None.
*
* @return - uint8_t : The sound level.
*/
uint8_t CFG_GetSoundLevel(void)
{
   return gSoundLevel;
}

/*************************************************************************************/
/**
* CFG_GetZoneDisabledMap
* Return the current zone enablement map.
*
* @param pZoneMap :  Pointer to structure to accept the zone map
*
* @return - bool  :  True if the function succeeds.
*/
bool CFG_GetZoneDisabledMap(ZoneDisabledMap_t* pZoneMap)
{
   bool result = false;
   if ( pZoneMap )
   {
      pZoneMap->LowerWord = gZoneMap.LowerWord;
      pZoneMap->MiddleWord = gZoneMap.MiddleWord;
      pZoneMap->UpperWord = gZoneMap.UpperWord;
      result = true;
   }
   return result;
}

/*************************************************************************************/
/**
* CFG_SetSplitZoneDisabledMap
* Set the lower or upper half of the current zone enablement map.
* The zone enablement map has 96 bits to represent the 96 zones. Bits are set to 1 if the
* zone is enabled.
* Used to record the zone enablement information from Route Add Response messages..
* The Route Add Response only has enough space to pass half of the zone map, so we send it
* twice with half of the map in each.
*
* @param upperHalf   :  false for lower half of map, true for upper half
* @param zoneWord    :  Pointer to structure to accept the zone map
*
* @return - bool     :  True if the function succeeds.
*/
void CFG_SetSplitZoneDisabledMap(const bool upperHalf, const uint32_t zoneWord, const uint16_t zoneHalfWord)
{
   uint32_t halfWord = (uint32_t)zoneHalfWord;
   
   //Save the map
   ZoneDisabledMap_t tempMap;
   tempMap.LowerWord = gZoneMap.LowerWord;
   tempMap.MiddleWord = gZoneMap.MiddleWord;
   tempMap.UpperWord = gZoneMap.UpperWord;

   if ( upperHalf )
   {
      //In this case the zoneWord is the upper word of the map
      //and zoneHalfWord is the upper 16 bits of the middle word.
      gZoneMap.UpperWord = zoneWord;
      halfWord <<= 16;
      gZoneMap.MiddleWord = ((gZoneMap.MiddleWord & 0xFFFF) | halfWord);
   }
   else 
   {
      //In this case the zoneWord is the lower word of the map
      //and zoneHalfWord is the lower 16 bits of the middle word.
      gZoneMap.LowerWord = zoneWord;
      gZoneMap.MiddleWord = ((gZoneMap.MiddleWord & 0xFFFF0000) | halfWord);
   }
   
   CO_PRINT_B_3(DBG_INFO_E,"Rx'd new zone enabled map (UML order) %x%x%x\r\n", gZoneMap.UpperWord, gZoneMap.MiddleWord, gZoneMap.LowerWord);
   

   //Check if the current zone is disabled
   bool zone_enabled = CFG_RequestedZoneEnabled(gZoneNumber);
   CFG_SetZoneEnabled(zone_enabled);
   
   //if the map has changed, write it to NVM
   if ( (tempMap.LowerWord != gZoneMap.LowerWord) ||
        (tempMap.MiddleWord != gZoneMap.MiddleWord) ||
        (tempMap.UpperWord != gZoneMap.UpperWord) )
   {
      CFG_WriteZoneMapToNVM();
   }
}


/*************************************************************************************/
/**
* CFG_GetSplitZoneDisabledMap
* Return the lower or upper half of the current zone enablement map.
* The zone enablement map has 96 bits to represent the 96 zones. Bits are set to 1 if the
* zone is enabled.
* Used to populate the Route Add Response messages with zone enablement for new children.
* The Route Add Response only has enough space to pass half of the zone map, so we send it
* twice with half of the map in each.
*
* @param upperHalf      :  false for lower half of map, true for upper half
* @param pZoneWord      :  Pointer to word to accept the zone map
* @param pZoneHalfWord  :  Pointer to half word to accept the zone map
*
* @return - bool     :  True if the function succeeds.
*/
bool CFG_GetSplitZoneDisabledMap(const bool upperHalf, uint32_t* pZoneWord, uint16_t* pZoneHalfWord)
{
   bool result = false;
   if ( pZoneWord && pZoneHalfWord )
   {
      if ( upperHalf )
      {
         //In this case the upper word of the map goes into pZoneWord and the top
         //16 bits of the middle word goes into pZoneHalfWord.
         *pZoneWord = gZoneMap.UpperWord;
         *pZoneHalfWord = (uint16_t)( gZoneMap.MiddleWord >> 16 );
      }
      else 
      {
         //In this case the lower word of the map goes into pZoneWord and the bottom
         //16 bits of the middle word goes into pZoneHalfWord.
         *pZoneWord = gZoneMap.LowerWord;
         *pZoneHalfWord = (uint16_t)( gZoneMap.MiddleWord & 0xFFFF );
      }
      result = true;
   }
   return result;
}

/*************************************************************************************/
/**
* CFG_UpdateZoneMap
* Updates a zone entry in the zone enablement map
*
* @param  - zone     :  The zone to update.
* @param  - enabled  :  The new enablement state of the zone.
*
* @return - None.
*/
void CFG_UpdateZoneMap(const uint8_t zone, const bool enabled)
{
   //The map is 96 bits stored in 3 words in the ZoneDisabledMap_t structure.
   
   if ( 0 < zone )
   {
      //offset the zone so that the map records zones 1-96 instead of 0-95
      uint8_t offset_zone = zone - 1;
      
      //Save the map
      ZoneDisabledMap_t tempMap;
      tempMap.LowerWord = gZoneMap.LowerWord;
      tempMap.MiddleWord = gZoneMap.MiddleWord;
      tempMap.UpperWord = gZoneMap.UpperWord;
      
      //Find which word contains the specified zone.
      uint32_t* pTargetWord;
      if ( 32 > offset_zone )
      {
         pTargetWord = &gZoneMap.LowerWord;
      }
      else if ( 64 > offset_zone )
      {
         pTargetWord = &gZoneMap.MiddleWord;
      }
      else 
      {
         pTargetWord = &gZoneMap.UpperWord;
      }
      
      //Find out which bit represents the requested zone.
      uint32_t targetBit = offset_zone % 32;
      
      uint32_t bitMask = (1 << targetBit);
      
      //clear the current entry
      *pTargetWord &= ~bitMask;
      //Set the new value.  Note that the map sets '1' to disable, '0' to enable
      if ( !enabled )
      {
         *pTargetWord |= bitMask;
      }
      
      //if the map has changed, write it to NVM
      if ( (tempMap.LowerWord != gZoneMap.LowerWord) ||
           (tempMap.MiddleWord != gZoneMap.MiddleWord) ||
           (tempMap.UpperWord != gZoneMap.UpperWord) )
      {
         CFG_WriteZoneMapToNVM();
      }
   }

}

/*************************************************************************************/
/**
* CFG_RequestedZoneEnabled
* Returns the enabled state of the specified zone based on the zone enablement map
*
* @param  - zone  :  The zone to check.
*
* @return - True if the zone is enabled, false if disabled.
*/
bool CFG_RequestedZoneEnabled(const uint8_t zone)
{
   bool result = false;
   uint8_t corrected_zone = zone;
   
   if ( 0 == zone )
   {
      result = true;
   }
   else 
   {
      //offset the zone so that the map records zones 1-96 instead of 0-95
      corrected_zone--;
      
      //The map is 96 bits stored in 3 words in the ZoneDisabledMap_t structure.
      //Find which word contains the specified zone.
      uint32_t targetWord;
      if ( 32 > corrected_zone )
      {
         targetWord = gZoneMap.LowerWord;
      }
      else if ( 64 > corrected_zone )
      {
         targetWord = gZoneMap.MiddleWord;
      }
      else 
      {
         targetWord = gZoneMap.UpperWord;
      }
      
      //Find out which bit represents the requested zone.
      uint32_t targetBit = corrected_zone % 32;
      
      uint32_t bitMask = (1 << targetBit);
      
      //Is the bit reset? i.e. enabled
      if ( 0 == (targetWord & bitMask) )
      {
         result = true;
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* CFG_WriteZoneMapToNVM
* Returns the enabled state of the specified zone based on the zone enablement map
*
* @param  - None.
*
* @return - SUCCESS_E or Error code.
*/
ErrorCode_t CFG_WriteZoneMapToNVM(void)
{
   //Write the zone disablement from NVM
   ErrorCode_t result = DM_NVMWrite(NV_ZONE_MAP_E, (void*)&gZoneMap, sizeof(gZoneMap));
   
   if ( SUCCESS_E == result )
   {
      CO_PRINT_B_0(DBG_INFO_E,"CFG: zone disablement map updated\r\n");
   }
   else
   {
      CO_PRINT_B_0(DBG_ERROR_E,"CFG: Failed to read zone disablement map\r\n");
   }
   
   return result;
}


/*************************************************************************************/
/**
* CFG_SetMaxNumberOfChildren
* Sets the max number of children that the device should support.
* This is different for the NCU and the RBUs.
*
* @param  - None.
*
* @return - The maximum number of children that the device should support.
*/
void CFG_SetMaxNumberOfChildren(const uint32_t max_children)
{
   gMaxChildren = max_children;
   CO_PRINT_B_1(DBG_INFO_E,"CFG: Max number of children set to %d\r\n", gMaxChildren);
}

/*************************************************************************************/
/**
* CFG_GetMaxNumberOfChildren
* Returns the max number of children that the device should support.
* This is different for the NCU and the RBUs.
*
* @param  - None.
*
* @return - The maximum number of children that the device should support.
*/
uint32_t CFG_GetMaxNumberOfChildren(void)
{
   return gMaxChildren;
}

/*************************************************************************************/
/**
* CFG_SetFaultReportsEnabled
* Set whether fault reports are enabled.
*
* @param  - enabled : True if fault reports are to be enabled, false to disable.
*
* @return - True if fault reports should be sent.
*/
void CFG_SetFaultReportsEnabled(const bool enabled)
{
   gFaultReportsEnabled = enabled;
   CO_PRINT_B_1(DBG_INFO_E,"Fault reports %s\r\n", (gFaultReportsEnabled ? "ENABLED" : "DISABLED"));
}

/*************************************************************************************/
/**
* CFG_GetFaultReportsEnabled
* Returns whether fault reports are enabled.
*
* @param  - None.
*
* @return - True if fault reports should be sent.
*/
bool CFG_GetFaultReportsEnabled(void)
{
   return gFaultReportsEnabled;
}


/*************************************************************************************/
/**
* CFG_GetDeviceCombination
* Returns the device combination.
*
* @param  - None.
*
* @return - The device combination.
*/
uint32_t CFG_GetDeviceCombination(void)
{
   return gDeviceCombination;
}

/*************************************************************************************/
/**
* CFG_GetDulchWrap
* Returns the DULCH wrap in short frames.
*
* @param  - None.
*
* @return - The DULCH Wrap.
*/
uint32_t CFG_GetDulchWrap(void)
{
   return gDulchWrap;
}

/*************************************************************************************/
/**
* CFG_SetGlobalDelay1
* Set the value of global delay 1.
*
* @param globalDelay1    Global delay 1 value.
*
* @return - void
*/
void CFG_SetGlobalDelay1(const uint16_t globalDelay1)
{
   zGlobalDelay1 = globalDelay1;
}

/*************************************************************************************/
/**
* CFG_GetGlobalDelay1
* Return the global delay 1 value.
*
* @param None.
*
* @return - the value of global delay 1.
*/
uint16_t CFG_GetGlobalDelay1(void)
{
   return zGlobalDelay1;
}

/*************************************************************************************/
/**
* CFG_SetGlobalDelay2
* Set the value of global delay 2.
*
* @param globalDelay2    Global delay 2 value.
*
* @return - void
*/
void CFG_SetGlobalDelay2(const uint16_t globalDelay2)
{
   zGlobalDelay2 = globalDelay2;
}

/*************************************************************************************/
/**
* CFG_GetGlobalDelay2
* Return the global delay 2 value.
*
* @param None.
*
* @return - the value of global delay 2.
*/
uint16_t CFG_GetGlobalDelay2(void)
{
   return zGlobalDelay2;
}

/*************************************************************************************/
/**
* CFG_GetLocalOrGlobalDelayBitmap
* Return a bitmap of whether he output channels use local or global delays.
* The bits represent the output channels in the order that they appear in DM_OP_OutputChannel_t.
*
* @param None.
*
* @return - The local/global delay channel bitmap.
*/
uint32_t CFG_GetLocalOrGlobalDelayBitmap(void)
{
   uint32_t bitmap = 0;
   
   if ( CFG_GetLocalOrGlobalDelayFlag(CO_CHANNEL_SOUNDER_E))
   {
      bitmap = 0x01;
   }
   
   if ( CFG_GetLocalOrGlobalDelayFlag(CO_CHANNEL_BEACON_E))
   {
      bitmap |= 0x02;
   }
   
   if ( CFG_GetLocalOrGlobalDelayFlag(CO_CHANNEL_STATUS_INDICATOR_LED_E))
   {
      bitmap |= 0x04;
   }
   
   if ( CFG_GetLocalOrGlobalDelayFlag(CO_CHANNEL_VISUAL_INDICATOR_E))
   {
      bitmap |= 0x08;
   }
   
   if ( CFG_GetLocalOrGlobalDelayFlag(CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E))
   {
      bitmap |= 0x10;
   }
   
   if ( CFG_GetLocalOrGlobalDelayFlag(CO_CHANNEL_OUTPUT_ROUTING_E))
   {
      bitmap |= 0x20;
   }
   
   if ( CFG_GetLocalOrGlobalDelayFlag(CO_CHANNEL_OUTPUT_1_E))
   {
      bitmap |= 0x40;
   }
   
   if ( CFG_GetLocalOrGlobalDelayFlag(CO_CHANNEL_OUTPUT_2_E))
   {
      bitmap |= 0x80;
   }
   
   return bitmap;
}

/*************************************************************************************/
/**
* CFG_SetMaxHops
* Set the value of Max Hops.
*
* @param maxHops   Max Hops value.
*
* @return - bool  True if setting succeeds
*/
bool CFG_SetMaxHops(const uint32_t maxHops)
{
   bool result = false;
   uint32_t value = maxHops;
   if ( SUCCESS_E == DM_NVMWrite(NV_MAX_HOPS_E, &value, sizeof(value)) )
   {
      gMaxHops = maxHops;
      result = true;
   }
   return result;
}

/*************************************************************************************/
/**
* CFG_GetMaxHops
* Return the Max Hops value.
*
* @param None.
*
* @return - the value of Max Hops.
*/
uint32_t CFG_GetMaxHops(void)
{
   return gMaxHops;
}

/*************************************************************************************/
/**
* CFG_SetBatteryCheckEnabled
* Set the battery check enabled.
*
* @param maxHops   Max Hops value.
*
* @return - enabled  True to enable battery checks.  False to disable.
*/
void CFG_SetBatteryCheckEnabled(const bool enabled)
{
   gBatteryChecksEnabled = enabled;
}

/*************************************************************************************/
/**
* CFG_GetBatteryCheckEnabled
* Return whether battery checks are enabled.
*
* @param None.
*
* @return - bool : True if battery checks are enabled.
*/
bool CFG_GetBatteryCheckEnabled(void)
{
   return gBatteryChecksEnabled;
}

/*************************************************************************************/
/**
* CFG_SetGlobalDelayOverride
* Set the global delay override.
*
* @param enable      global delay override setting
*
* @return void.
*/
void CFG_SetGlobalDelayOverride(const bool enable)
{
   CO_PRINT_B_1(DBG_INFO_E,"CFG: Global Delay Override setting=%d\r\n", enable);
   gGlobalDelayOverrideEnabled = enable;
}

/*************************************************************************************/
/**
* CFG_GetGlobalDelayOverride
* Return whether global delay override is enabled.
*
* @param None.
*
* @return - bool : True if global delay override is enabled.
*/
bool CFG_GetGlobalDelayOverride(void)
{
   return gGlobalDelayOverrideEnabled;
}

/*************************************************************************************/
/**
* CFG_SetRssiMarConfiguration
* Set the RSSI MAR joining threshold configuration.
* 0 = Use a joining threshold of -107dBm(single parent) or -112dBm(two parents)
* 1 = Use a joining threshold of -112dBm in all cases
* 2 = Use a joining threshold of -117dBm in all cases
* 3 = Use a joining threshold of -122dBm in all cases
* 4 = Use a joining threshold of -102dBm in all cases
* 5 = Use a joining threshold of -97dBm in all cases
* 6 = Use a joining threshold of -92dBm in all cases
* 7 = Use a joining threshold of -87dBm in all cases
* 8 = Use a joining threshold of -82dBm in all cases
* 9 = Use a joining threshold of -77dBm in all cases
* 10= Use a joining threshold of -72dBm in all cases
*
* @param config   The RSSI MAR config to be applied.
*
* @return - bool : True if the RSSI MAR config is valid.
*/
bool CFG_SetRssiMarConfiguration(const uint32_t config)
{
   bool result = false;
   if ( CFG_MAX_RSSI_MAR >= config )
   {
      gRssiMarSetting = config;
      result = true;
   }
   return result;
}

/*************************************************************************************/
/**
* CFG_GetRssiMarConfiguration
* Get the RSSI MAR joining threshold.
*
* @param two_parents.   True for two-parent connections
*
* @return - int32_t :   The RSSI Sensitivity threshold.
*/
int32_t CFG_GetRssiMarSensitivity(const bool two_parents)
{
   int32_t threshold;
   
   switch ( gRssiMarSetting )
   {
      case 0:
         if ( two_parents )
         {
            threshold = MIN_CANDIDATE_RSSI_1;
         }
         else 
         {
            threshold = MIN_CANDIDATE_RSSI_0;
         }
         break;
      case 1:
         threshold = MIN_CANDIDATE_RSSI_1;
         break;
      case 2:
         threshold = MIN_CANDIDATE_RSSI_2;
         break;
      case 3:
         threshold = MIN_CANDIDATE_RSSI_3;
         break;
      case 4:
         threshold = MIN_CANDIDATE_RSSI_4;
         break;
      case 5:
         threshold = MIN_CANDIDATE_RSSI_5;
         break;
      case 6:
         threshold = MIN_CANDIDATE_RSSI_6;
         break;
      case 7:
         threshold = MIN_CANDIDATE_RSSI_7;
         break;
      case 8:
         threshold = MIN_CANDIDATE_RSSI_8;
         break;
      case 9:
         threshold = MIN_CANDIDATE_RSSI_9;
         break;
      case 10:
         threshold = MIN_CANDIDATE_RSSI_10;
         break;
      default:
         threshold = MIN_CANDIDATE_RSSI_0;
      break;
   }
   
   return threshold;
}


/*************************************************************************************/
/**
* CFG_SetSnrJoiningThreshold
* Set the SNR joining threshold configuration.
*
* @param config   The SNR joining threshold to be applied.
*
* @return - bool : True if the RSSI MAR config is valid.
*/
bool CFG_SetSnrJoiningThreshold(const int32_t config)
{
   bool result = false;

   gSnrMarSetting = config;
   //CO_PRINT_B_1(DBG_INFO_E,"SNR joining threshold set to %d\r\n", gSnrMarSetting);
   result = true;
   
   return result;
}

/*************************************************************************************/
/**
* CFG_GetSnrJoiningThreshold
* Retun the SNR joining threshold configuration.
*
* @param None.
*
* @return - int32_t The SNR joining threshold.
*/
int32_t CFG_GetSnrJoiningThreshold(void)
{
   return gSnrMarSetting;
}

/*************************************************************************************/
/**
* CFG_SetBatteryTestInterval
* Set the battery test interval, in days.
*
* @param interval    The battery test interval.
*
* @return - bool :   True if the supplied interval is valid.
*/
bool CFG_SetBatteryTestInterval(const uint32_t interval)
{
   bool result = false;

   if ( MAX_BATTERY_TEST_INTERVAL >= interval )
   {
      gBatteryTestInterval = interval;
      CO_PRINT_B_1(DBG_INFO_E,"Battery test interval set to %d\r\n", gBatteryTestInterval);
      result = true;
   }
   else 
   {
      CO_PRINT_B_1(DBG_INFO_E,"Failed to set battery test interval.  Bad parameter \r\n", interval);
   }
   
   return result;
}

/*************************************************************************************/
/**
* CFG_GetBatteryTestInterval
* Get the battery test interval, in days.
*
* @param None.
*
* @return - int32_t The battery test interval in days.
*/
uint32_t CFG_GetBatteryTestInterval(void)
{
   return gBatteryTestInterval;
}

/*************************************************************************************/
/**
* CFG_SetBatPriLowVoltThresh
* Set the primary battery low voltage threshold in mV for SmartNet.
*
* @param: voltage_threshold: The battery voltage threshold
*
* @return: bool: True if the supplied voltage_threshold is valid
*/
bool CFG_SetBatPriLowVoltThresh(const uint32_t voltage_threshold)
{
   bool result = false;

   if ( MAX_BATTERY_S_FAIL_LOW_THRESH >= voltage_threshold )
   {
      gBatPriLowVoltThresh = voltage_threshold;
      CO_PRINT_B_1(DBG_INFO_E,"Primary battery low voltage threshold set to %d\r\n", gBatPriLowVoltThresh);
      result = true;
   }
   else
   {
      CO_PRINT_B_1(DBG_INFO_E,"Failed to set primary battery low voltage threshold. Bad parameter \r\n", voltage_threshold);
   }

   return result;
}

/*************************************************************************************/
/**
* CFG_GetBatPriLowVoltThresh
* Get the primary battery low voltage threshold in mV for SmartNet.
*
* @param:  None
*
* @return: int32_t The primary battery low voltage threshold
*/
uint32_t CFG_GetBatPriLowVoltThresh(void)
{
   return gBatPriLowVoltThresh;
}

/*************************************************************************************/
/**
* CFG_SetBatBkpLowVoltThresh
* Set the backup battery low voltage threshold in mV for SmartNet.
*
* @param: voltage_threshold: The battery voltage threshold
*
* @return: bool: True if the supplied voltage_threshold is valid
*/
bool CFG_SetBatBkpLowVoltThresh(const uint32_t voltage_threshold)
{
   bool result = false;

   if ( MAX_BATTERY_S_FAIL_LOW_THRESH >= voltage_threshold )
   {
      gBatBkpLowVoltThresh = voltage_threshold;
      CO_PRINT_B_1(DBG_INFO_E,"Backup battery low voltage threshold set to %d\r\n", gBatBkpLowVoltThresh);
      result = true;
   }
   else
   {
      CO_PRINT_B_1(DBG_INFO_E,"Failed to set backup battery low voltage threshold. Bad parameter \r\n", voltage_threshold);
   }

   return result;
}

/*************************************************************************************/
/**
* CFG_GetBatBkpLowVoltThresh
* Get the backup battery low voltage threshold in mV for SmartNet.
*
* @param:  None
*
* @return: int32_t The backup battery low voltage threshold
*/
uint32_t CFG_GetBatBkpLowVoltThresh(void)
{
   return gBatBkpLowVoltThresh;
}

/*************************************************************************************/
/**
* CFG_SetBatPriLowVoltThreshC
* Set the primary battery low voltage threshold in mV for Construction (SiteNet).
*
* @param: voltage_threshold: The battery voltage threshold
*
* @return: bool: True if the supplied voltage_threshold is valid
*/
bool CFG_SetBatPriLowVoltThreshC(const uint32_t voltage_threshold)
{
   bool result = false;

   if ( MAX_BATTERY_C_FAIL_LOW_THRESH >= voltage_threshold )
   {
      gBatPriLowVoltThreshC = voltage_threshold;
      CO_PRINT_B_1(DBG_INFO_E,"Primary battery low voltage threshold set to %d\r\n", gBatPriLowVoltThreshC);
      result = true;
   }
   else
   {
      CO_PRINT_B_1(DBG_INFO_E,"Failed to set primary battery low voltage threshold. Bad parameter \r\n", voltage_threshold);
   }

   return result;
}

/*************************************************************************************/
/**
* CFG_GetBatPriLowVoltThreshC
* Get the primary battery low voltage threshold in mV for Construction (SiteNet).
*
* @param:  None
*
* @return: int32_t The primary battery low voltage threshold
*/
uint32_t CFG_GetBatPriLowVoltThreshC(void)
{
   return gBatPriLowVoltThreshC;
}

/*************************************************************************************/
/**
* CFG_SetBatBkpLowVoltThreshC
* Set the backup battery low voltage threshold in mV for Construction (SiteNet).
*
* @param: voltage_threshold: The battery voltage threshold
*
* @return: bool: True if the supplied voltage_threshold is valid
*/
bool CFG_SetBatBkpLowVoltThreshC(const uint32_t voltage_threshold)
{
   bool result = false;

   if ( MAX_BATTERY_C_FAIL_LOW_THRESH >= voltage_threshold )
   {
      gBatBkpLowVoltThreshC = voltage_threshold;
      CO_PRINT_B_1(DBG_INFO_E,"Backup battery low voltage threshold set to %d\r\n", gBatBkpLowVoltThreshC);
      result = true;
   }
   else
   {
      CO_PRINT_B_1(DBG_INFO_E,"Failed to set backup battery low voltage threshold. Bad parameter \r\n", voltage_threshold);
   }

   return result;
}

/*************************************************************************************/
/**
* CFG_GetBatBkpLowVoltThreshC
* Get the backup battery low voltage threshold in mV for Construction (SiteNet).
*
* @param:  None
*
* @return: int32_t The backup battery low voltage threshold
*/
uint32_t CFG_GetBatBkpLowVoltThreshC(void)
{
   return gBatBkpLowVoltThreshC;
}

/*************************************************************************************/
/**
* CFG_SetSecurityInhibitDay
* Set the config to inhibit the security profile during the day.
*
* @param inhibit   True if the security profile is to be inhibited in daytime.
*
* @return - void.
*/
void CFG_SetSecurityInhibitDay(const bool inhibit)
{
   gSecurityProfileInhibitedDay = inhibit;
}

/*************************************************************************************/
/**
* CFG_GetSecurityInhibitDay
* Return the security inhibit setting for daytime.
*
* @param void.   True for two-parent connections
*
* @return - bool : True if the security profile is to be inhibited in daytime.
*/
bool CFG_GetSecurityInhibitDay(void)
{
   return gSecurityProfileInhibitedDay;
}

/*************************************************************************************/
/**
* CFG_SetSecurityInhibitNight
* Set the config to inhibit the security profile during the night.
*
* @param inhibit   True if the security profile is to be inhibited at night.
*
* @return - void.
*/
void CFG_SetSecurityInhibitNight(const bool inhibit)
{
   gSecurityProfileInhibitedNight = inhibit;
}

/*************************************************************************************/
/**
* CFG_GetSecurityInhibitNight
* Return the security inhibit setting for night time.
*
* @param void.
*
* @return - bool : True if the security profile is to be inhibited at night.
*/
bool CFG_GetSecurityInhibitNight(void)
{
   return gSecurityProfileInhibitedNight;
}


/*************************************************************************************/
/**
* CFG_GetPreformedPrimaryParent
* Return the node ID of the preformed primary parent.
*
* @param void.
*
* @return - uint16_t the node ID.
*/
uint16_t CFG_GetPreformedPrimaryParent(void)
{
   return gPreformPrimaryParent;
}

/*************************************************************************************/
/**
* CFG_GetPreformedSecondaryParent
* Return the node ID of the preformed secondary parent.
*
* @param void.
*
* @return - uint16_t the node ID.
*/
uint16_t CFG_GetPreformedSecondaryParent(void)
{
   return gPreformSecondaryParent;
}

/*************************************************************************************/
/**
* CFG_UsingPreformedMesh
* Return whether the device is configured for preformed mesh.
*
* @param void.
*
* @return - bool : TRUE if the device is configured for preformed mesh.
*/
bool CFG_UsingPreformedMesh(void)
{
   return gUsingPreformedMesh;
}


/*************************************************************************************/
/**
* CFG_SetSnrRssiAveragingPolicy
* Set the SNR and RSSI averaging policy for received heartbeats.
* 0 = Use default running average
* 1 = Restart average if last value is older than two long frames ago
* 2 = Do not average.  Use value from last heartbeat.
*
* @param config   The averaging policy to be applied.
*
* @return - bool : True if the policy is valid.
*/
bool CFG_SetSnrRssiAveragingPolicy(const uint32_t policy)
{
   bool result = false;
   if ( (uint32_t)CFG_AVE_MAX_E > policy )
   {
      gSnrRssiAveragePolicy = (SnrRssiAveragingPolicy_t)policy;
      CO_PRINT_B_1(DBG_INFO_E,"SNR and RSSI averaging policy set to %d\r\n", gSnrRssiAveragePolicy);
      result = true;
   }
   return result;
}

/*************************************************************************************/
/**
* CFG_GetSnrRssiAveragingPolicy
* Get the SNR and RSSI averaging policy for received heartbeats.
*
* @param None.
*
* @return - SnrRssiAveragingPolicy_t : The averaging policy.
*/
SnrRssiAveragingPolicy_t CFG_GetSnrRssiAveragingPolicy(void)
{
   return gSnrRssiAveragePolicy;
}

/*************************************************************************************/
/**
* CFG_SetInitialListenPeriod
* Set the time that the reciever is left in continuous listen mode on start-up before
* entering the sleep cycle.
*
* @param period      The period in seconds.
*
* @return - TRUE if the period is set and stored in NVM.
*/
bool CFG_SetInitialListenPeriod(const uint32_t period)
{
   bool result = false;
   if ( (period >= SECONDS_IN_ONE_HOUR) && (period <= (SECONDS_IN_ONE_HOUR * MAX_LOW_POWER_INITIAL_AWAKE_DURATION)) )
   {
      gInitialListenPeriod = period;
      result = true;
   }
   return result;
}

/*************************************************************************************/
/**
* CFG_GetInitialListenPeriod
* Set the time that the reciever is left in continuous listen mode on start-up before
* entering the sleep cycle.
*
* @param None.
*
* @return - uint32_t : The period in seconds.
*/
uint32_t CFG_GetInitialListenPeriod(void)
{
   return gInitialListenPeriod;
}

/*************************************************************************************/
/**
* CFG_SetPhase2Period
* Set the time period that the device stays in phase 2 of the start-up sleep cycle.
*
* @param period      The period in seconds.
*
* @return - TRUE if the period is set and stored in NVM.
*/
bool CFG_SetPhase2Period(const uint32_t period)
{
   bool result = false;
   if ( period >= SECONDS_IN_ONE_DAY )
   {
      gPhase2Period = period;
      result = true;
   }
   return result;
}

/*************************************************************************************/
/**
* CFG_GetPhase2Period
* get the time period that the device stays in phase 2 of the start-up sleep cycle.
*
* @param None.
*
* @return - uint32_t : The period in seconds.
*/
uint32_t CFG_GetPhase2Period(void)
{
   return gPhase2Period;
}

/*************************************************************************************/
/**
* CFG_SetPhase2SleepPeriod
* Set how long the device sleeps during the start-up cycle phase 2 if no heartbeat is received.
*
* @param period      The period in seconds.
*
* @return bool       TRUE if the period is valid.
*/
bool CFG_SetPhase2SleepPeriod(const uint32_t period)
{
   bool result = false;
   if ( (period >= SECONDS_IN_ONE_HOUR) && (period <= (SECONDS_IN_ONE_HOUR * MAX_LOW_POWER_SLEEP_DURATION)) )
   {
      gPhase2SleepPeriod = period;
      result = true;
   }
   return result;
}

/*************************************************************************************/
/**
* CFG_GetPhase2SleepPeriod
* Get how long the device sleeps during the start-up cycle phase 2 if no heartbeat is received.
*
* @param None.
*
* @return - uint32_t : The period in seconds.
*/
uint32_t CFG_GetPhase2SleepPeriod(void)
{
   return gPhase2SleepPeriod;
}

/*************************************************************************************/
/**
* CFG_SetPhase2SleepPeriod
* Set how long the device sleeps during the start-up cycle phase 3 if no heartbeat is received.
*
* @param period      The period in seconds.
*
* @return bool       TRUE if the period is valid.
*/
bool CFG_SetPhase3SleepPeriod(const uint32_t period)
{
   bool result = false;
   if ( (period >= SECONDS_IN_ONE_HOUR) && (period <= (SECONDS_IN_ONE_HOUR * MAX_LOW_POWER_SLEEP_DURATION)) )
   {
      gPhase3SleepPeriod = period;
      result = true;
   }
   return result;
}

/*************************************************************************************/
/**
* CFG_GetPhase3SleepPeriod
* Get how long the device sleeps during the start-up cycle phase 3 if no heartbeat is received.
*
* @param None.
*
* @return - uint32_t : The period in seconds.
*/
uint32_t CFG_GetPhase3SleepPeriod(void)
{
   return gPhase3SleepPeriod;
}


/*************************************************************************************/
/**
* CFG_SetSviPowerLock
* Set whether the SVI remains powered up, or is powered down when not in use..
*
* @param lock      TRUE to leave the SVI power enabled, False to switch it off between use.
*
* @return void.
*/
void CFG_SetSviPowerLock(const bool lock)
{
   gSVI_PowerLock = lock;
   CO_PRINT_B_1(DBG_INFO_E,"SVI Power lock %s\r\n", lock ? "ON":"OFF");
}

/*************************************************************************************/
/**
* CFG_GetSviPowerLock
* Get whether the SVI remains powered up, or is powered down when not in use..
*
* @param void..
*
* @return bool    TRUE to leave the SVI power enabled, False to switch it off between use.
*/
bool CFG_GetSviPowerLock(void)
{
   return gSVI_PowerLock;
}

void CFG_SetPollingDisabled(const bool disabled);
bool CFG_GetPollingDisabled(void);
/*************************************************************************************/
/**
* CFG_SetPollingDisabled
* Set whether the GPIO Polling should be disabled.
*
* @param lock      TRUE to leave the SVI power enabled, False to switch it off between use.
*
* @return void.
*/
void CFG_SetPollingDisabled(const bool disabled)
{
   gPollingDisabled = disabled;
   CO_PRINT_B_1(DBG_INFO_E,"GPIO polling %s\r\n", disabled ? "DISABLED":"ENABLED");
}

/*************************************************************************************/
/**
* CFG_GetPollingDisabled
* Get whether the GPIO polling is disabled
*
* @param void..
*
* @return bool    TRUE if GPIO polling is disabled, False if enabled.
*/
bool CFG_GetPollingDisabled(void)
{
   return gPollingDisabled;
}

/*************************************************************************************/
/**
* CFG_HasNonLatchingMedicalCallPoint
* Get whether the Device config includes a non-latching first aid button
*
* @param void..
*
* @return bool    TRUE if the DC includes a non-latching first aid button.
*/
bool CFG_HasNonLatchingMedicalCallPoint(void)
{
   bool has_non_latching = false;
   
   if ( (33 == gDeviceCombination) || (34 == gDeviceCombination) )
   {
      has_non_latching = true;
   }
   
   return has_non_latching;
}

/*************************************************************************************/
/**
* CFG_SetBlankingPlateFitted
* Set whether a blanking plate is fitted
*
* @param fitted   TRUE if a blanking plate is fitted.
*
* @return void.
*/
void CFG_SetBlankingPlateFitted(const bool fitted)
{
   gBlankingPlateFitted = fitted;
}


/*************************************************************************************/
/**
* CFG_GetBlankingPlateFitted
* Get whether a blanking plate is fitted
*
* @param void.
*
* @return bool    TRUE if a blanking plate is fitted.
*/
bool CFG_GetBlankingPlateFitted(void)
{
   return gBlankingPlateFitted;
}

/*************************************************************************************/
/**
* CFG_SetNumberOfResends
* Sets the number of times that downlink messages are repeated.
*
* @param  - num_resends    The number of messages.
*
* @return - None.
*/
void CFG_SetNumberOfResends(const uint32_t num_resends)
{
   gNumberOfResends = num_resends;
   CO_PRINT_B_1(DBG_INFO_E,"CFG: Number of downlink messages set to %d\r\n", gNumberOfResends);
}

/*************************************************************************************/
/**
* CFG_GetNumberOfResends
* Returns the number of times that downlink messages are repeated.
*
* @param  - None.
*
* @return - The number of messages.
*/
uint32_t CFG_GetNumberOfResends(void)
{
#ifdef IGNORE_MAX_RESEND_SETTING
   gNumberOfResends = DEFAULT_NUMBER_OF_RESENDS;
#endif
   return gNumberOfResends;
}

/*************************************************************************************/
/**
* CFG_SetBatteryPassivationValues
* Update the depassivation settings with the supplied values.
*
* @param  - pDepassivationSettings     A structure containing the depassivation parameters.
*
* @return - SUCCESS_E or ERR_INVALID_POINTER_E
*/
ErrorCode_t CFG_SetBatteryPassivationValues(const DePassivationSettings_t* const pDepassivationSettings)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   if ( pDepassivationSettings )
   {
      //range-check parameters
      result = ERR_INVALID_PARAMETER_E;
      if ( (CFG_DEPASSIVATION_VOLTAGE_MAX >= pDepassivationSettings->Voltage) &&
           (CFG_DEPASSIVATION_TIMEOUT_MAX >= pDepassivationSettings->Timeout) &&
           (CFG_DEPASSIVATION_ON_TIME_MAX >= pDepassivationSettings->OnTime) &&
           (CFG_DEPASSIVATION_OFF_TIME_MAX >= pDepassivationSettings->OffTime) )
      {
         uint32_t depassivationSettings;
         result = CFG_CompressDepassivationSettings(pDepassivationSettings, &depassivationSettings);
         if ( SUCCESS_E == result )
         {
            result = DM_NVMWrite(NV_BAT_PASSIVATION_SETTINGS_E, &depassivationSettings, sizeof(uint32_t));
            if ( SUCCESS_E == result )
            {
               gDepassivationSettings.Voltage = pDepassivationSettings->Voltage;
               gDepassivationSettings.Timeout = pDepassivationSettings->Timeout;
               gDepassivationSettings.OnTime = pDepassivationSettings->OnTime;
               gDepassivationSettings.OffTime = pDepassivationSettings->OffTime;
               result = SUCCESS_E;
            }
         }
      }
   }
   
   if ( SUCCESS_E != result )
   {
       CO_PRINT_B_0(DBG_ERROR_E,"FAILED to apply new depassivation values\r\n");
   }

   return result;
}

/*************************************************************************************/
/**
* CFG_SetBatteryPassivationValues
* Update the depassivation settings with the supplied values.
*
* @param  - depassivationSettings     The depassivation parameters compressed into 32 bits (NVM format).
*
* @return - SUCCESS_E or ERR_INVALID_POINTER_E
*/
ErrorCode_t CFG_SetBatteryPassivationValuesCompound(const uint32_t depassivationSettings)
{
   ErrorCode_t result;
   DePassivationSettings_t depassivation_settings;
   
   result = CFG_DecompressDepassivationSettings(depassivationSettings, &depassivation_settings);
   if ( SUCCESS_E == result )
   {
      result = DM_NVMWrite(NV_BAT_PASSIVATION_SETTINGS_E, &depassivationSettings, sizeof(uint32_t));
      if ( SUCCESS_E == result )
      {
         gDepassivationSettings.Voltage = depassivation_settings.Voltage;
         gDepassivationSettings.Timeout = depassivation_settings.Timeout;
         gDepassivationSettings.OnTime = depassivation_settings.OnTime;
         gDepassivationSettings.OffTime = depassivation_settings.OffTime;
         CO_PRINT_B_4(DBG_INFO_E,"New depassivation values: v=%d, T=%d, Ton=%d, Toff=%d\r\n", gDepassivationSettings.Voltage, gDepassivationSettings.Timeout, gDepassivationSettings.OnTime, gDepassivationSettings.OffTime);
      }
   }
   
   if ( SUCCESS_E != result )
   {
       CO_PRINT_B_0(DBG_ERROR_E,"FAILED to apply new depassivation values\r\n");
   }

   
   return result;
}

/*************************************************************************************/
/**
* CFG_SetBatteryPassivationValues
* Update the depassivation settings with the supplied values.
*
* @param  - pDepassivationSettings     A structure containing the depassivation parameters.
*
* @return - SUCCESS_E or ERR_INVALID_POINTER_E
*/
ErrorCode_t CFG_GetBatteryPassivationValues(DePassivationSettings_t* pDepassivationSettings)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   //Check received pointer
   if ( pDepassivationSettings )
   {
      pDepassivationSettings->Voltage = gDepassivationSettings.Voltage;
      pDepassivationSettings->Timeout = gDepassivationSettings.Timeout;
      pDepassivationSettings->OnTime = gDepassivationSettings.OnTime;
      pDepassivationSettings->OffTime = gDepassivationSettings.OffTime;
      result = SUCCESS_E;
   }
   
   return result;
}

/*************************************************************************************/
/**
* CFG_SetBatteryPassivationValues
* Return the depassivation settings compressed into 32 bits (NVM format).
*
* @param  - pDepassivationSettings   [OUT] The 32-bit compressed value.
*
* @return - SUCCESS_E or error code
*/
ErrorCode_t CFG_GetBatteryPassivationValuesCompound(uint32_t* pDepassivationSettings)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   //Check received pointer
   if ( pDepassivationSettings )
   {
      //Encode the current settings and put the result in pDepassivationSettings
      result = CFG_CompressDepassivationSettings(&gDepassivationSettings, pDepassivationSettings);
   }
   
   return result;
}

/*************************************************************************************/
/**
* CFG_CompressDepassivationSettings
* Compresses a depassivation settings structure into a 32 bit value for NVM storage.
* The compressed bits are mapped as follows:
* bits  0-11    voltage threshold
* bits 12-21    timeout
* bits 21-26    battery load on-time
* bits 27-31    battery load off-time
*
* @param  - pDepassivationSettings                 The structure containing the settings.
* @param  - pCompressedDepassivationSettings       [OUT] The 32-bit compressed value.
*
* @return - SUCCESS_E or error code.
*/
ErrorCode_t CFG_CompressDepassivationSettings(const DePassivationSettings_t* const pDepassivationSettings, uint32_t* pCompressedDepassivationSettings)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   uint32_t compound_value = 0;
   
   
   //Check received pointers
   if ( pDepassivationSettings && pCompressedDepassivationSettings )
   {
      //range-check parameters
      result = ERR_INVALID_PARAMETER_E;
      if ( (CFG_DEPASSIVATION_VOLTAGE_MAX >= pDepassivationSettings->Voltage) &&
           (CFG_DEPASSIVATION_TIMEOUT_MAX >= pDepassivationSettings->Timeout) &&
           (CFG_DEPASSIVATION_ON_TIME_MAX >= pDepassivationSettings->OnTime) &&
           (CFG_DEPASSIVATION_OFF_TIME_MAX >= pDepassivationSettings->OffTime) )
      {
         //Bit-shift the values to fit in a 32-bit word
         compound_value = (uint32_t)pDepassivationSettings->Voltage;
         compound_value |= ((uint32_t)pDepassivationSettings->Timeout << 12);
         compound_value |= ((uint32_t)pDepassivationSettings->OnTime << 22);
         compound_value |= ((uint32_t)pDepassivationSettings->OffTime << 27);
         //Store the result in the return parameter
         *pCompressedDepassivationSettings = compound_value;
         result = SUCCESS_E;
      }
   }
   
   return result;
}


/*************************************************************************************/
/**
* CFG_DecompressDepassivationSettings
* Decompresses a depassivation settings structure from a 32 bit value (NVM storage).
* The compressed bits are mapped as follows:
* bits  0-11    voltage threshold
* bits 12-21    timeout
* bits 21-26    battery load on-time
* bits 27-31    battery load off-time
*
* @param  - pDepassivationSettings                 The structure containing the settings.
* @param  - pCompressedDepassivationSettings       [OUT] The 32-bit compressed value.
*
* @return - SUCCESS_E or error code.
*/
ErrorCode_t CFG_DecompressDepassivationSettings(const uint32_t depassivation_settings, DePassivationSettings_t* pDepassivationSettings)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   //Check received pointer
   if ( pDepassivationSettings )
   {
      //Extract the settings
      pDepassivationSettings->Voltage = depassivation_settings & CFG_DEPASSIVATION_VOLTAGE_MASK;
      pDepassivationSettings->Timeout = ((depassivation_settings & CFG_DEPASSIVATION_TIMEOUT_MASK) >> 12);
      pDepassivationSettings->OnTime = ((depassivation_settings & CFG_DEPASSIVATION_ON_TIME_MASK) >> 22);
      pDepassivationSettings->OffTime = ((depassivation_settings & CFG_DEPASSIVATION_OFF_TIME_MASK) >> 27);
      result = SUCCESS_E;
   }
   return result;
}

/*************************************************************************************/
/**
* CFG_DecompressDepassivationSettings
* Decompresses a depassivation settings structure from a 32 bit value (NVM storage).
* The compressed bits are mapped as follows:
* bits  0-6     sub-revision
* bits 7-13     revision
* bits 14-20    major
* bits 21-31    unused
*
* @param  - pDepassivationSettings                 The structure containing the settings.
* @param  - pCompressedDepassivationSettings       [OUT] The 32-bit compressed value.
*
* @return - SUCCESS_E or error code.
*/
uint32_t CFG_GetFirmwareVersion(void)
{
   return gSoftwareVersionNumber;
}

/*************************************************************************************/
/**
* CFG_SetPpuMode
* Set the current PPU Mode.
* Valid values are:
*  STARTUP_NORMAL
*  STARTUP_PPU_CONNECTED
*  STARTUP_PPU_DISCONNECTED
*  STARTUP_SERIAL_BOOTLOADER
*  STARTUP_RADIO_BOOTLOADER
*
* @param  - pDepassivationSettings                 The structure containing the settings.
* @param  - pCompressedDepassivationSettings       [OUT] The 32-bit compressed value.
*
* @return - SUCCESS_E or error code.
*/
void CFG_SetPpuMode(const uint32_t ppuMode)
{
   switch ( ppuMode )
   {
      case STARTUP_NORMAL: //Intentional drop-through
      case STARTUP_PPU_CONNECTED:
      case STARTUP_PPU_DISCONNECTED:
      case STARTUP_SERIAL_BOOTLOADER:
      case STARTUP_RADIO_BOOTLOADER:
         gPpuMode = ppuMode;
         break;
      default:
         //nothing to do
      break;
   }
}

/*************************************************************************************/
/**
* CFG_GetPpuMode
* Return the current PPU Mode setting
*
* @param  - None.
*
* @return - uint32_t    PPU MMode setting.
*/
uint32_t CFG_GetPpuMode(void)
{
   return gPpuMode;
}

/*************************************************************************************/
/**
* CFG_SetFaultsClearEnable
* Enable/disable the sending of a 'faults clear' message on joining.
*
* @param enable            0 for disable. non-zero to enable.
*
* @return - error code
*/
ErrorCode_t CFG_SetFaultsClearEnable(const uint32_t enable)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;

   //Update the working value
   if ( enable )
   {
      gSendFaultsClearMsg = 1;
   }
   else 
   {
      gSendFaultsClearMsg = 0;
   }
   
   CO_PRINT_A_1(DBG_INFO_E,"Rx'd no-fault msg enable = %d\r\n", gSendFaultsClearMsg);
   
   
   //Write the new value to FLASH
   result = DM_NVMWrite(NV_ENABLE_NO_FAULT_REPORT_E, (void*)&gSendFaultsClearMsg, sizeof(gSendFaultsClearMsg));
   if ( SUCCESS_E != result )
   {
      CO_PRINT_A_0(DBG_ERROR_E,"Failed to store new no-fault msg enable in NVM\r\n");
   }
   else if ( gSendFaultsClearMsg > 1 )
   {
      gSendFaultsClearMsg = 1;
   }
   
   return result;
}

/*************************************************************************************/
/**
* CFG_GetFaultsClearEnable
* Return the no-fault msg enable setting.
*
* @param None.
*
* @return - uint32_t : The enable status. 0=disabled, 1=enabled
*/
uint32_t CFG_GetFaultsClearEnable(void)
{
   return gSendFaultsClearMsg;
}



/*************************************************************************************/
/**
* CFG_IsSyncMaster
* Return whether the device is the sync master.
*
* @param  - None
*
* @return - True if the device is sync master
*/
bool CFG_IsSyncMaster(void)
{
   bool syncMaster = false;
   
   if ( 0 != gIsSyncMaster )
   {
      syncMaster = true;
   }
   return syncMaster;
}
#ifdef APPLY_RSSI_OFFSET
void CFG_SetRssiOffset(const uint16_t node_id, const int8_t offset)
{
   if ( RSSI_OFFSET_SIZE > node_id )
   {
      gRssiOffset[node_id] = offset;
   }
}

int8_t CFG_GetRssiOffset(const uint16_t node_id)
{
   int8_t offset = 0;
   if ( RSSI_OFFSET_SIZE > node_id )
   {
      offset = gRssiOffset[node_id];
   }
   return offset;
}
#endif

