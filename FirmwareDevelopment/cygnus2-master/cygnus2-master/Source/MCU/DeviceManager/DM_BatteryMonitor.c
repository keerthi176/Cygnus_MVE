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
*  File         : DM_BatteryMonitor.c
*
*  Description  : Source for the battery management code
*
*************************************************************************************/

/* System Include Files
*************************************************************************************/
#include "cmsis_os.h"

/* User Include Files
*************************************************************************************/
#include "board.h"
#include "utilities.h"
#include "DM_OutputManagement.h"
#include "DM_ADC.h"
#include "DM_LED.h"
#include "DM_NVM.h"
#include "CFG_Device_cfg.h"
#include "MM_TimedEventTask.h"
#include "MM_ApplicationCommon.h"
#include "DM_BatteryMonitor.h"

/* Private definitions
*************************************************************************************/
#ifdef ENABLE_CONTINUOUS_LOAD_BATTERY_CURRENT_TEST
#define DM_BAT_INITIAL_TEST_DELAY 30U
#else
#define DM_BAT_INITIAL_TEST_DELAY 30U
#endif
#define DM_BAT_PASSIVATION_DELAY    10U   /* in sec */
#define DM_BAT_PASSIVATION_INTERVAL  1U   /* in sec */
//DM_BAT_PASSIVATION_TIMEOUT           moved to CO_Defines.h
//DM_BAT_PASSIVATION_VOLTAGE_MV        moved to CO_Defines.h
#define DM_BAT_PASSIVATION_THRESHOLD_MV 199U
#define DM_BAT_NUM_OF_BATTERY_READINGS 3U
#ifdef DM_BAT_DO_SMARTNET_CURRENT_WINDOW_TEST
   #define DM_BAT_LOW_CURRENT_PERCENTAGE   1U
   #define DM_BAT_HIGH_CURRENT_PERCENTAGE 99U
#endif
#define DM_BAT_DETECTION_THRESHOLD   20U
#define DM_BAT_MONITOR_SETTLING_TIME  5U

#define THRESHOLD_PRODUCT 4096
#define CONSTRUCTION_HARDWARE_VOLTAGE_CORRECTION 4U
#define MCP_HARDWARE_VOLTAGE_CORRECTION          2U
#ifdef SE_BATTERY
#define SE_BATTERY_SHUTDOWN_VOLTAGE 3000U
#define SE_BATTERY_SHUTDOWN_VOLTAGE_CONSTRUCTION_PRIMARY 9000U
#define SE_BATTERY_SHUTDOWN_VOLTAGE_CONSTRUCTION_BACKUP  6000U
#define BATTERY_TEST_INTERVAL (SECONDS_IN_ONE_HOUR)
#else
#define BATTERY_TEST_INTERVAL (SECONDS_IN_ONE_DAY * CFG_GetBatteryTestInterval())
#endif

#define DM_BAT_PERIOD_10MIN  (600U)
/* #define DM_BAT_PERIOD_10MIN   (60U)  For test only without waiting too long */
#define DM_BAT_PERIOD_28SEC   (28U)
#define DM_BAT_PERIOD_15SEC   (15U)

#define BATTERY_TEST_RESTART_INTERVAL   (300U)

typedef enum 
{
   DM_BAT_STARTUP_E,
   DM_BAT_IDLE_E,
   DM_BAT_TEST_POSTPONED_E,
   DM_BAT_PRETEST_E,
   DM_BAT_PRIMARY_BAT_CONFIGURE_E,
   DM_BAT_PRIMARY_PASSIVATION_START_E,
   DM_BAT_PRIMARY_PASSIVATION_PROCESS_E,
   DM_BAT_PRIMARY_PASSIVATION_COMPLETE_E,
   DM_BAT_PRIMARY_BAT_SETTLE_DELAY_E,
   DM_BAT_TEST_PRIMARY_BATTERY_E,
   DM_BAT_BACKUP_BAT_CONFIGURE_E,
   DM_BAT_BACKUP_BAT_SETTLE_DELAY_E,
   DM_BAT_TEST_BACKUP_BATTERY_E,
   DM_BAT_TEST_BACKUP_BAT_VOLTAGE_E,
   DM_BAT_TEST_CURRENT_SENSE_E,
   DM_BAT_TEST_BAT_1_E,
   DM_BAT_TEST_BAT_2_E,
   DM_BAT_TEST_BAT_3_E,
   DM_BAT_VBAT_MISSING_TEST_E,
   DM_BAT_TEST_REPORT_E,
   DM_BAT_TEST_COMPLETE_E
} BatteryMonitorState_t;


typedef enum 
{
   DM_BAT_VOLTAGE_TEST_E,
   DM_BAT_CURRENT_TEST_E
} BatteryTestType_t;


typedef struct
{
   ADC_HandleTypeDef * ADC;
   uint32_t Channel;
   uint32_t Value;
   BatteryTestResult_t State;
   bool BatteryDetected;
} BatteryStatus_t;

/* Global Variables
*************************************************************************************/
extern DM_BaseType_t gBaseType;
extern ADC_HandleTypeDef adc1Handle;
extern Gpio_t BatMonEn;
extern Gpio_t PriBatteryEn;
extern Gpio_t BatVMon;
extern Gpio_t BkpBatEn;
extern Gpio_t VbatMonEn;
extern Gpio_t VBkpBatMonEn;
extern osMessageQId(AppQ);        // Queue to the application
extern osPoolId AppPool;         // Pool definition for use with AppQ.

/* Private Variables
*************************************************************************************/
#ifndef SE_BATTERY
static uint32_t SettlingCount = 0U;
static uint32_t PassivationCount = 0U;
static uint32_t PassivationLoadOnOffCount = 0U;
static bool PassivationLoadOn = true;
static uint32_t BatterySampleNumber = 0U;
static uint32_t gPassivationDelay = DM_BAT_PASSIVATION_DELAY;
#else
static bool BatteryCheckRequested = false;
#endif
static uint32_t IdleCount = 0U;
static BatteryMonitorState_t BatMonitorState;
static BatteryStatus_t Battery[DM_BAT_BATTERY_MAX_E];
static uint32_t BatteryVoltageReadings[DM_BAT_BATTERY_MAX_E][DM_BAT_NUM_OF_BATTERY_READINGS]; // store value for tests that require multiple reads.
#ifdef DM_BAT_DO_SMARTNET_CURRENT_TEST
static uint32_t BatteryCurrentReadings[DM_BAT_NUM_OF_BATTERY_READINGS][DM_BAT_BATTERY_MAX_E];
#endif
static uint32_t PrimaryBatteryReferenceVoltage_mv = 3300U;
static uint32_t BackupBatteryReferenceVoltage_mv = 3300U;

static uint32_t PrimaryBatteryVoltage = 0U;
static uint32_t BackupBatteryVoltage = 0U;
static bool BatteryFault = false;
static bool BatteryWarning = false;
static bool gBatteryDepleted = false;
static DM_BatteryTestMode_t gBatteryTestMode;
static bool gBatteryResultsUpdated = false;
#ifndef SE_BATTERY
static BatteryTestResult_t gPrimaryTestResult = DM_BAT_INVALID_TEST_E;
static BatteryTestResult_t gBackupTestResult = DM_BAT_INVALID_TEST_E;
#endif
static DM_BAT_RestartFlags_t gRestartNvmFlagslags;
static bool gInhibitMonitoring = false;

static bool TamperSwCycledPrimaryLowVoltageEnable = false;
static bool TamperSwCycledBackupLowVoltageEnable = false;
static bool PrimaryLowVoltageEnable = false;
static bool BackupLowVoltageEnable = false;
static uint8_t PrimaryLowVoltageNumOf10Min = 0U;
static uint8_t BackupLowVoltageNumOf10Min = 0U;
static uint32_t PrimaryTimerCount = 0U;
static uint32_t BackupTimerCount = 0U;

/*  Added variables to fix CYG2-1780 */
static bool PrimaryDelayFlag = 0U;
static bool BackupDelayFlag = 0U;
static uint32_t PrimarySettlingCount = 0U;
static uint32_t BackupSettlingCount = 0U;
bool gBatteryTestRestartFlag = 0U;  /* 0:Battery test request not generated, 1: Battery test request generated  */
bool gBatteryTestStartFlag = 0U;    /* 0:Battery test not started, 1: Battery test started  */
bool gBatteryTestCompleteFlag = 0U; /* 0:Battery test not completed, 1: Battery test completed  */
bool gBatteryTestDelay = 0U;

/* Private Functions Prototypes
*************************************************************************************/
#ifndef SE_BATTERY
static void DM_BAT_CheckPriLowBattery(void);
static void DM_BAT_CheckBkpLowBattery(void);
static void DM_BAT_CheckPriLowBatteryConstr(void);
static void DM_BAT_CheckBkpLowBatteryConstr(void);
static void DM_BAT_RunRBUBatteryMonitor(void);
static void DM_BAT_RunConstructionBatteryMonitor(void);
static BatteryTestResult_t DM_BAT_ValidateBatteryMCP(const BatteryID_t battery_id, const BatteryTestType_t test_type, const DM_BAT_Battery_t type);
static BatteryTestResult_t DM_BAT_ValidateBatteryConstruction(const BatteryID_t battery_id, const BatteryTestType_t test_type, const DM_BAT_Battery_t type);
#endif
static ErrorCode_t DM_BAT_ReadBatteryValue(const BatteryID_t battery);
static void DM_BAT_SetBatteryEnable(const DM_BAT_Battery_t battery, const DM_BAT_BatteryEnable_t enable);
static void DM_BAT_SetBatteryMonitor(const DM_BAT_Battery_t battery, const DM_BAT_BatteryMonitor_t enable);
static void DM_BAT_ClearBatteryData(void);


#ifdef ENABLE_CONTINUOUS_LOAD_BATTERY_CURRENT_TEST
static void DM_BAT_RunSmartNetBatteryCurrenttest(void);
#endif

#ifdef SE_BATTERY
extern uint16_t gNetworkAddress;
static void DM_BAT_EnterSleepmode(void);
static void DM_BAT_RunSEBatteryTest(void);
#endif

/* Functions
*************************************************************************************/

/*****************************************************************************
*  Function:      DM_BAT_Initialise
*  Description:   Initialisation function for the Battery monitor 
*
*  param    baseType    The type of radio base for the device configuration.
*  return   None
*
*  Notes:
*****************************************************************************/
void DM_BAT_Initialise(const DM_BaseType_t baseType)
{
   CO_PRINT_B_0(DBG_INFO_E,"DM_BAT_Initialise+\r\n");
   
#ifndef ENABLE_CONTINUOUS_LOAD_BATTERY_CURRENT_TEST
#ifndef SE_BATTERY
   uint32_t battery_test_interval = BATTERY_TEST_INTERVAL;
   CO_ASSERT_VOID_MSG( (battery_test_interval > DM_BAT_INITIAL_TEST_DELAY), "Battery test interval too low");
#endif
#endif
   BatMonitorState = DM_BAT_STARTUP_E;
   
   adc1Handle.Instance = ADC1;
   
   switch ( baseType )
   {
      case BASE_MCP_E: // Intentional drop-through
      case BASE_IOU_E:
      case BASE_RBU_E:
      case BASE_REPEATER_E:
         Battery[DM_BAT_BATTERY_1_E].Value = 0;
         Battery[DM_BAT_BATTERY_1_E].ADC = &adc1Handle;
         Battery[DM_BAT_BATTERY_1_E].Channel = ADC_CHANNEL_7;        //Cell 1 current
         Battery[DM_BAT_BATTERY_1_E].State = DM_BAT_TEST_OK_E;
         Battery[DM_BAT_BATTERY_1_E].BatteryDetected = false;
         Battery[DM_BAT_BATTERY_2_E].Value = 0;
         Battery[DM_BAT_BATTERY_2_E].ADC = &adc1Handle;
         Battery[DM_BAT_BATTERY_2_E].Channel = ADC_CHANNEL_8;        //Cell 2 current
         Battery[DM_BAT_BATTERY_2_E].State = DM_BAT_TEST_OK_E;
         Battery[DM_BAT_BATTERY_2_E].BatteryDetected = false;
         Battery[DM_BAT_BATTERY_3_E].Value = 0;
         Battery[DM_BAT_BATTERY_3_E].ADC = &adc1Handle;
         Battery[DM_BAT_BATTERY_3_E].Channel = ADC_CHANNEL_3;        //Cell 3 current
         Battery[DM_BAT_BATTERY_3_E].State = DM_BAT_TEST_OK_E;
         Battery[DM_BAT_BATTERY_3_E].BatteryDetected = false;
         Battery[DM_BAT_BATTERY_VBAT_E].Value = 0;
         Battery[DM_BAT_BATTERY_VBAT_E].ADC = &adc1Handle;
         Battery[DM_BAT_BATTERY_VBAT_E].Channel = ADC_CHANNEL_4;        //VBAT
         Battery[DM_BAT_BATTERY_VBAT_E].State = DM_BAT_TEST_OK_E;
         Battery[DM_BAT_BATTERY_VBAT_E].BatteryDetected = false;
      break;
      
      case BASE_CONSTRUCTION_E:
         Battery[DM_BAT_BATTERY_1_E].Value = 0;
         Battery[DM_BAT_BATTERY_1_E].ADC = &adc1Handle;
         Battery[DM_BAT_BATTERY_1_E].Channel = ADC_CHANNEL_7;
         Battery[DM_BAT_BATTERY_1_E].State = DM_BAT_TEST_OK_E;
         Battery[DM_BAT_BATTERY_1_E].BatteryDetected = false;
         Battery[DM_BAT_BATTERY_2_E].Value = 0;
         Battery[DM_BAT_BATTERY_2_E].ADC = &adc1Handle;
         Battery[DM_BAT_BATTERY_2_E].Channel = ADC_CHANNEL_8;
         Battery[DM_BAT_BATTERY_2_E].State = DM_BAT_TEST_OK_E;
         Battery[DM_BAT_BATTERY_2_E].BatteryDetected = false;
         Battery[DM_BAT_BATTERY_3_E].Value = 0;
         Battery[DM_BAT_BATTERY_3_E].ADC = &adc1Handle;
         Battery[DM_BAT_BATTERY_3_E].Channel = ADC_CHANNEL_8;
         Battery[DM_BAT_BATTERY_3_E].State = DM_BAT_TEST_OK_E;
         Battery[DM_BAT_BATTERY_3_E].BatteryDetected = false;
         Battery[DM_BAT_BATTERY_VBAT_E].Value = 0;
         Battery[DM_BAT_BATTERY_VBAT_E].ADC = &adc1Handle;
         Battery[DM_BAT_BATTERY_VBAT_E].Channel = ADC_CHANNEL_3;
         Battery[DM_BAT_BATTERY_VBAT_E].State = DM_BAT_TEST_OK_E;
         Battery[DM_BAT_BATTERY_VBAT_E].BatteryDetected = false;
         break;
      
      default:
         CO_PRINT_A_1(DBG_ERROR_E,"ADC Config.  Unknown base type %d\r\n", baseType);
         break;
   }
   
   gBatteryTestMode = DM_BAT_TEST_MODE_OFF_E;
   gInhibitMonitoring = false;
   gRestartNvmFlagslags = DM_BAT_GetBatteryRestartFlag();
   
   DM_BAT_ClearBatteryData();
}

/*****************************************************************************
*  Function:      DM_BAT_RequestBatteryCheck
*  Description:   Initiates a test of the batteries
*
*  param          delayed  True to wait one minute before starting test.
*                          False for immediate test.
*  return         None
*
*  Notes:
*****************************************************************************/
void DM_BAT_RequestBatteryCheck(const bool delayed)
{
   // discard previous readings to force a full recheck
   DM_BAT_ClearBatteryData();
   
#ifndef SE_BATTERY
   BatterySampleNumber = 0U;
#else
   BatteryCheckRequested = true;
#endif
   
   if ( delayed )
   {
      // Restart the state machine to run the initial 30 sec delay
      BatMonitorState = DM_BAT_STARTUP_E;
      IdleCount = 0U;
      /* If tamper switch is cycled and there is primary bat low voltage */
      if (PrimaryLowVoltageEnable)
      {
         TamperSwCycledPrimaryLowVoltageEnable = true;
      }
      /* If tamper switch is cycled and there is backup bat low voltage */
      if (BackupLowVoltageEnable)
      {
         TamperSwCycledBackupLowVoltageEnable = true;
      }
   }
   else
   {
      //immediate battery test
      BatMonitorState = DM_BAT_IDLE_E;
      IdleCount = BATTERY_TEST_INTERVAL;
   }
   
}

/*****************************************************************************
*  Function:      DM_BAT_GetPrimaryBatteryVoltage
*  Description:   Returns the primary battery voltage in mV
*
*  param          None
*  return         voltage in mV
*
*  Notes:
*****************************************************************************/
uint32_t DM_BAT_GetPrimaryBatteryVoltage(void)
{
   uint32_t primary_voltage_mv = 0U;
   
   primary_voltage_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(PrimaryBatteryReferenceVoltage_mv, PrimaryBatteryVoltage, ADC_RESOLUTION_12B);
   if ( BASE_CONSTRUCTION_E == gBaseType )
   {
      primary_voltage_mv *= CONSTRUCTION_HARDWARE_VOLTAGE_CORRECTION;
   }
   else 
   {
      primary_voltage_mv *= MCP_HARDWARE_VOLTAGE_CORRECTION;
   }
   /* CO_PRINT_B_2(DBG_INFO_E,"DM_BAT_GetPrimaryBatteryVoltage: PrimaryRef=%dmV, Primary=%dmV\r\n", PrimaryBatteryReferenceVoltage_mv, primary_voltage_mv); */
   return primary_voltage_mv;
}
   
/*****************************************************************************
*  Function:      DM_BAT_GetBackupBatteryVoltage
*  Description:   Returns the backup battery voltage in mV
*
*  param          None
*  return         voltage in mV
*
*  Notes:
*****************************************************************************/
uint32_t DM_BAT_GetBackupBatteryVoltage(void)
{
   uint32_t backup_voltage_mv = 0U;
   
   backup_voltage_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(BackupBatteryReferenceVoltage_mv, BackupBatteryVoltage, ADC_RESOLUTION_12B);
   if ( BASE_CONSTRUCTION_E == gBaseType )
   {
      backup_voltage_mv *= CONSTRUCTION_HARDWARE_VOLTAGE_CORRECTION;
   }
   else 
   {
      backup_voltage_mv *= MCP_HARDWARE_VOLTAGE_CORRECTION;
   }
   /* CO_PRINT_B_2(DBG_INFO_E,"DM_BAT_GetBackupBatteryVoltage: BackupRef=%dmV, Backup=%dmV\r\n", BackupBatteryReferenceVoltage_mv, backup_voltage_mv); */
   
   return backup_voltage_mv;
}

/*****************************************************************************
*  Function:      DM_BAT_SetBatteryRestartFlag
*  Description:   Set or reset the battery restart flag in NVM
*
*  param  flags   Sets the restart flags in NVM for primary or backup missing
*  return bool    True on success
*
*  Notes:   The battery restart flag is set when starting battery checks and
*           reset when the test completes.  If the unit is restarted because
*           batteries are not fitted, the flag will be still be set on 
*           restarting, and we know not to repeat the test.  The flag is reset
*           when the dismantle tamper is restored, re-enabling battery checks.
*****************************************************************************/
bool DM_BAT_SetBatteryRestartFlag(const DM_BAT_RestartFlags_t flags)
{
   bool result = false;
   uint32_t restart_flags;
   if ( SUCCESS_E == DM_NVMRead( NV_RESTART_FLAGS_E, &restart_flags, sizeof(restart_flags)) )
   {
      //Clear the previous state
      restart_flags &= ~NVM_RESTART_BATTERY_TEST_MASK;
      
      result = true;
      switch ( flags )
      {
         case DM_BAT_NORMAL_RESTART_E:
            //No action
            break;
         case DM_BAT_PRIMARY_RESTART_E:
            restart_flags |= NVM_RESTART_PRIMARY_BATTERY_MASK;
            break;
         case DM_BAT_BACKUP_RESTART_E:
            restart_flags |= NVM_RESTART_BACKUP_BATTERY_MASK;
            break;
         default:
            result = false;
            break;
      }

      if ( result )
      {
         result = DM_NVMWrite( NV_RESTART_FLAGS_E, &restart_flags, sizeof(restart_flags));
      }
   }
   return result;
}

/*****************************************************************************
*  Function:      DM_BAT_GetBatteryRestartFlag
*  Description:   Returns True if the battery restrt flag is set in NVM.
*
*  param                            None
*  return   DM_BAT_RestartFlags_t   The restart flag state.
*
*  Notes:
*****************************************************************************/
DM_BAT_RestartFlags_t DM_BAT_GetBatteryRestartFlag(void)
{
   DM_BAT_RestartFlags_t flag_setting = DM_BAT_NORMAL_RESTART_E;//Default to normal start if we fail to read the NVM.
   uint32_t restart_flags;
   
   if ( SUCCESS_E == DM_NVMRead( NV_RESTART_FLAGS_E, &restart_flags, sizeof(restart_flags)) )
   {
      if ( 0 == (restart_flags & NVM_RESTART_BATTERY_TEST_MASK) )
      {
         flag_setting = DM_BAT_NORMAL_RESTART_E;
      }
      else if ( NVM_RESTART_BACKUP_BATTERY_MASK == ( restart_flags & NVM_RESTART_BACKUP_BATTERY_MASK ) )
      {
         flag_setting = DM_BAT_BACKUP_RESTART_E;
      }
      else if (NVM_RESTART_PRIMARY_BATTERY_MASK == ( restart_flags & NVM_RESTART_PRIMARY_BATTERY_MASK ))
      {
         flag_setting = DM_BAT_PRIMARY_RESTART_E;
      }
   }
   return flag_setting;
}

/*****************************************************************************
*  Function:      DM_BAT_ConfigureForDeepSleep
*  Description:   Set the batteries for deep sleep mode
*
*  param          None
*  return         None
*
*  Notes: Deep sleep is called for if the batteries reach minimum charge
*****************************************************************************/
void DM_BAT_ConfigureForDeepSleep(void)
{
   //Reset the state machine
   BatMonitorState = DM_BAT_STARTUP_E;
   IdleCount = 0U;
   
   CO_PRINT_A_0(DBG_INFO_E,"Battery Monitor Configure for deep sleep\r\n");
   
   if ( BASE_CONSTRUCTION_E == gBaseType )
   {
      //Switch off the battery load circuits
      DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);
      DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
      //set primary battery ON
      DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
      //Ensure backup battery is ON
      DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
   }
   else
   {
      //Ensure primary batteries ON
      //Switch off the battery load circuit
      DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
      DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
      //Set backup battery ON
      DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
   }
}

/*****************************************************************************
*  Function:      DM_BAT_BatteryDepleted
*  Description:   Returns the state of the primary and backup batteries
*
*  param          None
*  return         True if the primary and backup batteries are depleted,
*                 false otherwise.
*
*  Notes:
*****************************************************************************/
bool DM_BAT_BatteryDepleted(void)
{
   return gBatteryDepleted;
}

/*****************************************************************************
*  Function:      DM_BAT_Run
*  Description:   Periodically called to run the battery monitor state machine.
*
*  param          None
*  return         None
*
*  Notes:
*****************************************************************************/
void DM_BAT_Run(void)
{
   /* CO_PRINT_B_3(DBG_INFO_E,"DM_BAT_Run enab=%d, inhibit=%d, state=%d\r\n", CFG_GetBatteryCheckEnabled(), gInhibitMonitoring, GetCurrentDeviceState()); */

   if ( CFG_GetBatteryCheckEnabled() && (false == gInhibitMonitoring) && (STATE_ACTIVE_E == GetCurrentDeviceState()) )
   {
#ifdef SE_BATTERY
      DM_BAT_RunSEBatteryTest();
#else
      if ( (BASE_CONSTRUCTION_E == gBaseType) )
      {
         DM_BAT_RunConstructionBatteryMonitor();
      }
      else
      {
#ifdef ENABLE_CONTINUOUS_LOAD_BATTERY_CURRENT_TEST
         DM_BAT_RunSmartNetBatteryCurrenttest();
#else
         DM_BAT_RunRBUBatteryMonitor();
#endif
      }
#endif // SE_BATTERY
   }
}

/*****************************************************************************
*  Function:      DM_BAT_Cancel
*  Description:   Cancel the battery test if it's in progress.
*
*  param          None
*  return         None
*
*  Notes: This function calls the depleted batteries config function
*         but the unit doesn't go into deep sleep.  As a result the test is
*         rescheduled for DM_BAT_INITIAL_TEST_DELAY seconds later
*****************************************************************************/
void DM_BAT_Cancel(void)
{
   // Are battery checks enabled?
   if ( CFG_GetBatteryCheckEnabled() )
   {
      //Is there a test running?
      if ( (DM_BAT_IDLE_E != BatMonitorState) && (DM_BAT_TEST_POSTPONED_E != BatMonitorState) && (DM_BAT_STARTUP_E != BatMonitorState) )
      {
         //Restore the NVM flags to their pre-test values
         DM_BAT_SetBatteryRestartFlag(gRestartNvmFlagslags);
         
         //It's the same shutdown procedure as when going to sleep
         DM_BAT_ConfigureForDeepSleep();
         CO_PRINT_B_0(DBG_INFO_E,"Battery Test Cancelled\r\n");
         
         if ( DM_OP_OutputsActive(true) )
         {
            BatMonitorState = DM_BAT_TEST_POSTPONED_E;
         }
         else 
         {
            BatMonitorState = DM_BAT_IDLE_E;
         }
         IdleCount = 0U;
      }
   }
}

/*****************************************************************************
*  Function:      DM_BAT_Inhibit
*  Description:   Inhibit or release inhibit on battery operations.
*                 Called when tamper is triggered to inhibit battery monitoring.
*                 Called when tamper is restored to resume battery monitoring.
*
*  param          inhibit  TRUE to inhibit battery monitoring.  FALSE to resume.
*  return         void
*
*****************************************************************************/
void DM_BAT_Inhibit(const bool inhibit)
{
   if ( inhibit )
   {
      //The request is to inhibit battery monitoring.
      
      //Cancel any ongoing battery test.
      DM_BAT_Cancel();
      //Disable battery testing
      gInhibitMonitoring = true;
   }
   else 
   {
      //The request is to resume battery monitoring.
      
      //Re-enable battery monitoring
      gInhibitMonitoring = false;
      //Schedule battery test
      DM_BAT_RequestBatteryCheck(true);
   }
}

/*****************************************************************************
*  Function:      DM_BAT_ClearBatteryData
*  Description:   Erase previous battery readings.
*
*  param          None
*  return         None
*
*  Notes: 
*****************************************************************************/
void DM_BAT_ClearBatteryData(void)
{
   Battery[DM_BAT_BATTERY_1_E].Value = 0;
   Battery[DM_BAT_BATTERY_1_E].State = DM_BAT_TEST_OK_E;
   Battery[DM_BAT_BATTERY_1_E].BatteryDetected = false;
   Battery[DM_BAT_BATTERY_2_E].Value = 0;
   Battery[DM_BAT_BATTERY_2_E].State = DM_BAT_TEST_OK_E;
   Battery[DM_BAT_BATTERY_2_E].BatteryDetected = false;
   Battery[DM_BAT_BATTERY_3_E].Value = 0;
   Battery[DM_BAT_BATTERY_3_E].State = DM_BAT_TEST_OK_E;
   Battery[DM_BAT_BATTERY_3_E].BatteryDetected = false;
   Battery[DM_BAT_BATTERY_VBAT_E].Value = 0;
   Battery[DM_BAT_BATTERY_VBAT_E].State = DM_BAT_TEST_OK_E;
   Battery[DM_BAT_BATTERY_VBAT_E].BatteryDetected = false;
   PrimaryBatteryVoltage = 0U;
   BackupBatteryVoltage = 0U;
   gBatteryResultsUpdated = false;
}

#ifndef SE_BATTERY

/*****************************************************************************
*  Function:      DM_BAT_CheckPriLowBattery
*  Description:   After the first primary low voltage is detected, this function
*                 checks for 3 more consecutive low voltage reads with 10 minute
*                 periods. If so, it will set gPrimaryTestResult = DM_BAT_ERROR_E.
*                 Note: The first low voltage detection is done in
*                 DM_BAT_RunRBUBatteryMonitor() - case DM_BAT_TEST_PRIMARY_BATTERY_E
*
*  Param          None
*  Return         None
*  Note:          For SmartNet
*****************************************************************************/
void DM_BAT_CheckPriLowBattery(void)
{
   uint32_t battery_mv = 0U;

   if (PrimaryLowVoltageEnable)
   {
      if(!PrimaryDelayFlag)
	   {
		 PrimaryTimerCount++;
		 CO_PRINT_B_1(DBG_INFO_E,"DM_BAT_CheckPriLowBattery: PrimaryTimerCount=%d\r\n", PrimaryTimerCount);
	   }
	   else
	   {
		 PrimarySettlingCount++;
		 CO_PRINT_B_0(DBG_INFO_E,"#### Applying DM_BAT_PRIMARY_BAT_SETTLE_DELAY_E ### \r\n");	
	   }
     /* Check if the 10min time is over and backup battery delay flag is OFF */
      if ( (DM_BAT_PERIOD_10MIN <= PrimaryTimerCount )&&(!BackupDelayFlag) )
      {
	      /* Set the primary battery settling delay flag  */
			PrimaryDelayFlag = 1;
			/* Turn primary battery ON */
         DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
         /* Turn backup battery OFF */
         DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_OFF_E);
         /* Turn battery load circuit ON */
         DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
		 
         CO_PRINT_B_0(DBG_INFO_E,"#### Primary battery switches ON ### \r\n");
        
		   PrimarySettlingCount = 0U;
		   PrimaryTimerCount = 0U;
         osDelay(20);
		}
      /* Check for the completion of settling delay of primary battery */
		if ( DM_BAT_MONITOR_SETTLING_TIME <= PrimarySettlingCount )
		{
			PrimarySettlingCount = 0U;
			PrimaryBatteryReferenceVoltage_mv = 0U;
				
         /* Read the ADC reference voltage */
         if ( 0U == PrimaryBatteryReferenceVoltage_mv )
         {
            DM_ADC_ReadReferenceVoltage(&PrimaryBatteryReferenceVoltage_mv);
         }
         /* Read the battery adc */
         ErrorCode_t result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E);
         if ( SUCCESS_E != result )
         {
            CO_PRINT_B_1(DBG_ERROR_E,"Primary Battery Monitor FAILED error=%d\r\n", result);
         }

         battery_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(PrimaryBatteryReferenceVoltage_mv, Battery[DM_BAT_BATTERY_VBAT_E].Value, ADC_RESOLUTION_12B);
         battery_mv *= MCP_HARDWARE_VOLTAGE_CORRECTION;

         CO_PRINT_B_2(DBG_INFO_E,"DM_BAT_CheckPriLowBattery: raw adc=%d, battery_mv=%d\r\n", Battery[DM_BAT_BATTERY_VBAT_E].Value, battery_mv);
				
         /* Set primary battery voltage */
         PrimaryBatteryVoltage = Battery[DM_BAT_BATTERY_VBAT_E].Value;
		   CO_PRINT_B_0(DBG_INFO_E,"#### Sending battery voltage to panel #### \r\n");	
		   DM_BAT_SendBatteryStatus(false);
				 
         if ( CFG_GetBatPriLowVoltThresh() > battery_mv )
         {
            PrimaryLowVoltageNumOf10Min++;
         }
         else
         {
            /* A good reading has been found then start from beginning */
            PrimaryTimerCount = 0U;
            PrimaryLowVoltageNumOf10Min = 0U;
            PrimaryLowVoltageEnable = false;
         }

         CO_PRINT_B_2(DBG_INFO_E,"DM_BAT_CheckPriLowBattery: PrimaryLowVoltageNumOf10Min=%d, threshold=%d\r\n", PrimaryLowVoltageNumOf10Min, CFG_GetBatPriLowVoltThresh());

         if (DM_BAT_NUM_OF_BATTERY_READINGS <= PrimaryLowVoltageNumOf10Min)
         {
            /* Low voltage has been reached after 4 readings and 30 mins */
            PrimaryLowVoltageNumOf10Min = 0U;
            PrimaryLowVoltageEnable = false;
            gPrimaryTestResult = DM_BAT_VOLTAGE_LOW_E;
            CO_PRINT_A_0(DBG_INFO_E,"Primary Battery >>> LOW voltage <<<\r\n");
            /* Start the amber battery LED double flash pattern */
            DM_LedPatternRequest(LED_MISSING_BATTERY_E);
            /* Update the Application (control panel) with low battery */
            CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_LOW_BATTERY_E, true, false, false);
            /* If backup is not a missing battery */
            if ((DM_BAT_ERROR_E != gBackupTestResult))
            {
               /* Update the Application (control panel) with NO battery error in case it was set previously for primary */
               CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_BATTERY_ERROR_E, false, false, false);
            }
         }

         /* Turn battery load circuit OFF */
         DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
         /* Turn backup battery ON */
         DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
         if ( (DM_BAT_VOLTAGE_LOW_E == gPrimaryTestResult) && (DM_BAT_TEST_OK_E == gBackupTestResult) )
         {
            /* Turn primary batteries OFF */
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
         }
         else
         {
            /* Turn primary batteries ON */
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
         }
			/* Reset the primary battery settling delay flag  */
			PrimaryDelayFlag = 0U;
      }
	}
}

/*****************************************************************************
*  Function:      DM_BAT_CheckPriLowBatteryConstr
*  Description:   After the first primary low voltage is detected, this function
*                 checks for 3 more consecutive low voltage reads with 10 minute
*                 periods. If so, it will set gPrimaryTestResult = DM_BAT_ERROR_E.
*                 Note: The first low voltage detection is done in
*                 DM_BAT_RunRBUBatteryMonitor() - case DM_BAT_TEST_PRIMARY_BATTERY_E
*
*  Param          None
*  Return         None
*  Note:          For Construction (SiteNet)
*****************************************************************************/
void DM_BAT_CheckPriLowBatteryConstr(void)
{
   uint32_t battery_mv = 0U;

   if (PrimaryLowVoltageEnable)
   {
      if(!PrimaryDelayFlag)
		{
			PrimaryTimerCount++;
			CO_PRINT_B_1(DBG_INFO_E,"DM_BAT_CheckPriLowBatteryConstr: PrimaryTimerCount=%d\r\n", PrimaryTimerCount);
		}
		else
		{
			PrimarySettlingCount++;
			CO_PRINT_B_0(DBG_INFO_E,"#### Applying DM_BAT_PRIMARY_BAT_SETTLE_DELAY_E ### \r\n");	
		}
		/* Check if the 10min time is over and backup battery delay flag is OFF */
      if ( ( DM_BAT_PERIOD_10MIN <= PrimaryTimerCount ) && ( !BackupDelayFlag ) )
      {
	      /* Set primary battery settling delay flag */
			PrimaryDelayFlag =1;
			/* Turn primary battery ON */
         DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
         /* Turn primary battery load circuit ON */
         DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_ON_E);
         /* Turn backup battery OFF */
         DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_OFF_E);
         /* Turn backup battery load circuit OFF */
         DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
			CO_PRINT_B_0(DBG_INFO_E,"#### Primary battery switches are turned ON ### \r\n");	
				 
			PrimarySettlingCount = 0U;
			PrimaryTimerCount = 0U;
			osDelay(20);
		}
      /* Check for the completion of settling delay of primary battery */			
		if ( DM_BAT_MONITOR_SETTLING_TIME <= PrimarySettlingCount )
		{		 
			PrimarySettlingCount = 0U;
         PrimaryBatteryReferenceVoltage_mv = 0U;

         /* Read the ADC reference voltage */
         if ( 0U == PrimaryBatteryReferenceVoltage_mv )
         {
            DM_ADC_ReadReferenceVoltage(&PrimaryBatteryReferenceVoltage_mv);
         }
         /* Read the battery adc */
         ErrorCode_t result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E);
         if ( SUCCESS_E != result )
         {
            CO_PRINT_B_1(DBG_ERROR_E,"Primary Battery Monitor FAILED error=%d\r\n", result);
         }

         battery_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(PrimaryBatteryReferenceVoltage_mv, Battery[DM_BAT_BATTERY_VBAT_E].Value, ADC_RESOLUTION_12B);
         battery_mv *= CONSTRUCTION_HARDWARE_VOLTAGE_CORRECTION;

         CO_PRINT_B_2(DBG_INFO_E,"DM_BAT_CheckPriLowBatteryConstr: raw adc=%d, battery_mv=%d\r\n", Battery[DM_BAT_BATTERY_VBAT_E].Value, battery_mv);
			/* Set primary battery voltage */
			PrimaryBatteryVoltage = Battery[DM_BAT_BATTERY_VBAT_E].Value;
			CO_PRINT_B_0(DBG_INFO_E,"#### Sending Primary battery voltage to panel #### \r\n");	
			DM_BAT_SendBatteryStatus(false);
				 
         if ( CFG_GetBatPriLowVoltThreshC() > battery_mv )
         {
            PrimaryLowVoltageNumOf10Min++;
         }
         else
         {
            /* A good reading has been found then start from beginning */
            PrimaryTimerCount = 0U;
            PrimaryLowVoltageNumOf10Min = 0U;
            PrimaryLowVoltageEnable = false;
         }

         CO_PRINT_B_2(DBG_INFO_E,"DM_BAT_CheckPriLowBatteryConstr: PrimaryLowVoltageNumOf10Min=%d, threshold=%d\r\n", PrimaryLowVoltageNumOf10Min, CFG_GetBatPriLowVoltThreshC());

         if (DM_BAT_NUM_OF_BATTERY_READINGS <= PrimaryLowVoltageNumOf10Min)
         {
            /* Low voltage has been reached after 4 readings and 30 mins */
            PrimaryLowVoltageNumOf10Min = 0U;
            PrimaryLowVoltageEnable = false;
            gPrimaryTestResult = DM_BAT_VOLTAGE_LOW_E;
            CO_PRINT_A_0(DBG_INFO_E,"Primary Battery >>> LOW voltage <<<\r\n");
            /* Start the amber battery LED double flash pattern */
            DM_LedPatternRequest(LED_MISSING_BATTERY_E);
            /* Update the Application (control panel) with low battery */
            CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_LOW_BATTERY_E, true, false, false);
            /* If backup is not a missing battery */
            if ((DM_BAT_ERROR_E != gBackupTestResult))
            {
               /* Update the Application (control panel) with NO battery error in case it was set previously for primary */
               CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_BATTERY_ERROR_E, false, false, false);
            }
         }

         /* Turn primary battery load circuit OFF */
         DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);
         /* Turn backup battery ON */
         DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
         if ( (DM_BAT_VOLTAGE_LOW_E == gPrimaryTestResult) && (DM_BAT_TEST_OK_E == gBackupTestResult) )
         {
            /* Turn primary batteries OFF */
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
         }
         else
         {
            /* Turn primary batteries ON */
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
         }
			/* Reset primary battery settling delay flag */
			PrimaryDelayFlag = 0U;
      }
   }
}

/*****************************************************************************
*  Function:      DM_BAT_CheckBkpLowBattery
*  Description:   After the first backup low voltage is detected, this function
*                 checks for 3 more consecutive low voltage reads with 10 minute
*                 periods. If so, it will set gBackupTestResult = DM_BAT_ERROR_E.
*                 Note: The first low voltage detection is done in
*                 DM_BAT_RunRBUBatteryMonitor() - case DM_BAT_TEST_BACKUP_BATTERY_E
*
*  Param          None
*  Return         None
*  Note:          For SmartNet
*****************************************************************************/
void DM_BAT_CheckBkpLowBattery(void)
{
   uint32_t battery_mv = 0U;

   if (BackupLowVoltageEnable)
   {
      if(!BackupDelayFlag)
		{
			BackupTimerCount++;
			CO_PRINT_B_1(DBG_INFO_E,"DM_BAT_CheckBkpLowBattery: BackupTimerCount=%d\r\n", BackupTimerCount);
		}
		else
		{
			BackupSettlingCount++;
			CO_PRINT_B_0(DBG_INFO_E,"#### Applying Backup battery settling delay #### \r\n");
		}
		/* Check if the 10min time is over and primary battery delay flag is OFF */
      if ( (DM_BAT_PERIOD_10MIN <= BackupTimerCount ) && (!PrimaryDelayFlag) )
      {					
			/* Set the backup battery settiling delay flag */
			BackupDelayFlag = 1;
			/* Turn backup battery ON */
			DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
			/* Turn primary battery OFF */
			DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
			/* Turn battery load circuit ON */
			DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
			CO_PRINT_B_0(DBG_INFO_E,"#### Backup battery switches ON #### \r\n");	
				
			BackupSettlingCount = 0U;
			BackupTimerCount = 0U;
			osDelay(20);
		}
		/* Check for the completion of backup battery settling delay */		 
		if ( DM_BAT_MONITOR_SETTLING_TIME <= BackupSettlingCount )
		{
			BackupSettlingCount = 0U;
			BackupBatteryReferenceVoltage_mv = 0U;
					
         /* Read the ADC reference voltage */
         if ( 0U == BackupBatteryReferenceVoltage_mv )
         {
           DM_ADC_ReadReferenceVoltage(&BackupBatteryReferenceVoltage_mv);
         }
         /* Read the battery adc */
         ErrorCode_t result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E);
         if ( SUCCESS_E != result )
         {
            CO_PRINT_B_1(DBG_ERROR_E,"Backup Battery Monitor FAILED error=%d\r\n", result);
         }

         battery_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(BackupBatteryReferenceVoltage_mv, Battery[DM_BAT_BATTERY_VBAT_E].Value, ADC_RESOLUTION_12B);
         battery_mv *= MCP_HARDWARE_VOLTAGE_CORRECTION;

         CO_PRINT_B_2(DBG_INFO_E,"DM_BAT_CheckBkpLowBattery: raw adc=%d, battery_mv=%d\r\n", Battery[DM_BAT_BATTERY_VBAT_E].Value, battery_mv);
				 
			/* Set Backup battery voltage */
			BackupBatteryVoltage = Battery[DM_BAT_BATTERY_VBAT_E].Value;
			CO_PRINT_B_0(DBG_INFO_E,"#### Sending Backup battery voltage to panel #### \r\n");	
			DM_BAT_SendBatteryStatus(false);
				 
         if ( CFG_GetBatBkpLowVoltThresh() > battery_mv )
         {
            BackupLowVoltageNumOf10Min++;
         }
         else
         {
            /* A good reading has been found then start from beginning */
            BackupTimerCount = 0U;
            BackupLowVoltageNumOf10Min = 0U;
            BackupLowVoltageEnable = false;
         }

         CO_PRINT_B_2(DBG_INFO_E,"DM_BAT_CheckBkpLowBattery: BackupLowVoltageNumOf10Min=%d, threshold=%d\r\n", BackupLowVoltageNumOf10Min, CFG_GetBatBkpLowVoltThresh());

         if (DM_BAT_NUM_OF_BATTERY_READINGS <= BackupLowVoltageNumOf10Min)
         {
            /* Low voltage has been reached after 4 readings and 30 mins */
            BackupLowVoltageNumOf10Min = 0U;
            BackupLowVoltageEnable = false;
            gBackupTestResult = DM_BAT_VOLTAGE_LOW_E;
            CO_PRINT_A_0(DBG_INFO_E,"Backup Battery >>> LOW voltage <<<\r\n");
            /* Start the amber battery LED double flash pattern */
            DM_LedPatternRequest(LED_MISSING_BATTERY_E);
            /* Update the Application (control panel) with low battery */
            CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_LOW_BATTERY_E, true, false, false);
            /* If backup is not a missing battery */
            if ((DM_BAT_ERROR_E != gPrimaryTestResult))
            {
               /* Update the Application (control panel) with NO battery error in case it was set previously for backup */
               CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_BATTERY_ERROR_E, false, false, false);
            }
         }

         /* Turn battery load circuit OFF */
         DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
         /* Turn backup battery ON */
         DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
         if ( (DM_BAT_VOLTAGE_LOW_E == gPrimaryTestResult) && (DM_BAT_TEST_OK_E == gBackupTestResult) )
         {
            /* Turn primary batteries OFF */
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
         }
         else
         {
            /* Turn primary batteries ON */
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
         }
			/* Reset the backup battery settling delay flag */
			BackupDelayFlag = 0U;
      }
   }
}

/*****************************************************************************
*  Function:      DM_BAT_CheckBkpLowBatteryConstr
*  Description:   After the first backup low voltage is detected, this function
*                 checks for 3 more consecutive low voltage reads with 10 minute
*                 periods. If so, it will set gBackupTestResult = DM_BAT_ERROR_E.
*                 Note: The first low voltage detection is done in
*                 DM_BAT_RunRBUBatteryMonitor() - case DM_BAT_TEST_BACKUP_BATTERY_E
*
*  Param          None
*  Return         None
*  Note:          For Construction (SiteNet)
*****************************************************************************/
void DM_BAT_CheckBkpLowBatteryConstr(void)
{
   uint32_t battery_mv = 0U;

   if (BackupLowVoltageEnable)
   {
      if(!BackupDelayFlag)
		{
			BackupTimerCount++;
			CO_PRINT_B_1(DBG_INFO_E,"DM_BAT_CheckBkpLowBatteryConstr: BackupTimerCount=%d\r\n", BackupTimerCount);
				
		}
		else
		{
			BackupSettlingCount++;
			CO_PRINT_B_0(DBG_INFO_E,"#### Applying Backup battery settling delay ### \r\n");
		}
      /* Check if the 10min time is over and primary battery delay flag is OFF */
      if ( (DM_BAT_PERIOD_10MIN <= BackupTimerCount ) && (!PrimaryDelayFlag))
      {
         /* Set the backup battery settling delay flag  */
			BackupDelayFlag = 1;
			/* Turn backup battery ON */
         DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
         /* Turn backup battery load circuit ON */
         DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
         /* Turn primary battery OFF */
         DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
         /* Turn primary battery load circuit OFF */
         DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);
				
			BackupSettlingCount = 0U;
			BackupTimerCount = 0U;
         osDelay(20);
		}
		/* Check for the completion of backup battery settling delay */
      if ( DM_BAT_MONITOR_SETTLING_TIME <= BackupSettlingCount )
		{ 
			BackupSettlingCount = 0U;
			BackupBatteryReferenceVoltage_mv = 0U;
			/* Read the ADC reference voltage */
         if ( 0U == BackupBatteryReferenceVoltage_mv )
         {
            DM_ADC_ReadReferenceVoltage(&BackupBatteryReferenceVoltage_mv);
         }
         /* Read the battery adc */
         ErrorCode_t result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_3_E);
         if ( SUCCESS_E != result )
         {
            CO_PRINT_B_1(DBG_ERROR_E,"Backup Battery Monitor FAILED error=%d\r\n", result);
         }

         battery_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(BackupBatteryReferenceVoltage_mv, Battery[DM_BAT_BATTERY_3_E].Value, ADC_RESOLUTION_12B);
         battery_mv *= CONSTRUCTION_HARDWARE_VOLTAGE_CORRECTION;

         CO_PRINT_B_2(DBG_INFO_E,"DM_BAT_CheckBkpLowBatteryConstr: raw adc=%d, battery_mv=%d\r\n", Battery[DM_BAT_BATTERY_3_E].Value, battery_mv);
				 
			/* Set Backup battery voltage */
			BackupBatteryVoltage = Battery[DM_BAT_BATTERY_3_E].Value;
			CO_PRINT_B_0(DBG_INFO_E,"#### Sending Backup battery voltage to panel #### \r\n");	
			DM_BAT_SendBatteryStatus(false);
				 
         if ( CFG_GetBatBkpLowVoltThreshC() > battery_mv )
         {
            BackupLowVoltageNumOf10Min++;
         }
         else
         {
            /* A good reading has been found then start from beginning */
            BackupTimerCount = 0U;
            BackupLowVoltageNumOf10Min = 0U;
            BackupLowVoltageEnable = false;
         }

         CO_PRINT_B_2(DBG_INFO_E,"DM_BAT_CheckBkpLowBatteryConstr: BackupLowVoltageNumOf10Min=%d, threshold=%d\r\n", BackupLowVoltageNumOf10Min, CFG_GetBatBkpLowVoltThreshC());

         if (DM_BAT_NUM_OF_BATTERY_READINGS <= BackupLowVoltageNumOf10Min)
         {
            /* Low voltage has been reached after 4 readings and 30 mins */
            BackupLowVoltageNumOf10Min = 0U;
            BackupLowVoltageEnable = false;
            gBackupTestResult = DM_BAT_VOLTAGE_LOW_E;
            CO_PRINT_A_0(DBG_INFO_E,"Backup Battery >>> LOW voltage <<<\r\n");
            /* Start the amber battery LED double flash pattern */
            DM_LedPatternRequest(LED_MISSING_BATTERY_E);
            /* Update the Application (control panel) with low battery */
            CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_LOW_BATTERY_E, true, false, false);
            /* If primary is not a missing battery */
            if ((DM_BAT_ERROR_E != gPrimaryTestResult))
            {
               /* Update the Application (control panel) with NO battery error in case it was set previously for backup */
               CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_BATTERY_ERROR_E, false, false, false);
            }
         }

         /* Turn backup battery load circuit OFF */
         DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
         /* Turn backup battery ON */
         DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
         if ( (DM_BAT_VOLTAGE_LOW_E == gPrimaryTestResult) && (DM_BAT_TEST_OK_E == gBackupTestResult) )
         {
            /* Turn primary batteries OFF */
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
         }
         else
         {
            /* Turn primary batteries ON */
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
         }
			/* Reset the backup battery settling delay flag */
			BackupDelayFlag = 0U;
      }
   }
}

/*****************************************************************************
*  Function:      DM_BAT_RunRBUBatteryMonitor
*  Description:   Periodically called to run the battery monitor state machine
*                 for the RBU, Repeater, MCP and IOU configuration.
*
*  param          None
*  return         None
*
*  Notes:
*****************************************************************************/
void DM_BAT_RunRBUBatteryMonitor(void)
{
   static uint32_t first_primary_voltage_reading = 0U;
   static uint32_t second_primary_voltage_reading = 0U;
   static bool firstReadingDone = false;
   static bool secondReadingDone = false;
   
   if ( (BASE_RBU_E == gBaseType) || 
        (BASE_MCP_E == gBaseType) || 
        (BASE_IOU_E == gBaseType) || 
        (BASE_REPEATER_E == gBaseType) )
   {
      switch( BatMonitorState )
      {
         case DM_BAT_STARTUP_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_STARTUP_E\r\n");
            DM_BAT_CheckPriLowBattery();
            DM_BAT_CheckBkpLowBattery();
            /* CO_PRINT_B_2(DBG_INFO_E,"PrimaryLowVoltageEnable=%d, BackupLowVoltageEnable=%d\r\n", PrimaryLowVoltageEnable, BackupLowVoltageEnable); */
            /* CO_PRINT_B_1(DBG_INFO_E,"CFG_GetBatPriLowVoltThresh()= %d\r\n", CFG_GetBatPriLowVoltThresh()); */
            /* CO_PRINT_B_1(DBG_INFO_E,"CFG_GetBatBkpLowVoltThresh()= %d\r\n", CFG_GetBatBkpLowVoltThresh()); */
            IdleCount++;
            //Wait for the start-up delay then trigger a battery test
            if ( DM_BAT_INITIAL_TEST_DELAY <= IdleCount )
            {
               IdleCount = BATTERY_TEST_INTERVAL;
               BatMonitorState = DM_BAT_IDLE_E;
            }
            break;
         }
         case DM_BAT_IDLE_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_IDLE_E\r\n");
            DM_BAT_CheckPriLowBattery();
            DM_BAT_CheckBkpLowBattery();
            CO_PRINT_B_2(DBG_INFO_E,"gPrimaryTestResult=%d, gBackupTestResult=%d\r\n", gPrimaryTestResult,gBackupTestResult);
            CO_PRINT_B_2(DBG_INFO_E,"PrimaryLowVoltageEnable=%d, BackupLowVoltageEnable=%d\r\n", PrimaryLowVoltageEnable, BackupLowVoltageEnable);
            IdleCount++;
            CO_PRINT_B_1(DBG_INFO_E,"IdleCount= %d\r\n", IdleCount);
            if ( (BATTERY_TEST_INTERVAL <= IdleCount) )
            {
               IdleCount = 0U;
               
               gRestartNvmFlagslags = DM_BAT_GetBatteryRestartFlag();
               CO_PRINT_B_1(DBG_INFO_E,"gRestartNvmFlagslags= %d\r\n", gRestartNvmFlagslags);
               if ( DM_BAT_PRIMARY_RESTART_E == gRestartNvmFlagslags )
               {
                  CO_PRINT_B_0(DBG_INFO_E,"Postponing battery check - primary batteries missing\r\n");
                  Battery[DM_BAT_BATTERY_VBAT_E].BatteryDetected = false;
                  gPrimaryTestResult = DM_BAT_ERROR_E;
                  BatMonitorState = DM_BAT_VBAT_MISSING_TEST_E;
               }
               else if ( DM_BAT_BACKUP_RESTART_E == gRestartNvmFlagslags )
               {
                  CO_PRINT_B_0(DBG_INFO_E,"Postponing battery check - backup batteries missing\r\n");
                  Battery[DM_BAT_BATTERY_VBAT_E].BatteryDetected = true;
                  gBackupTestResult = DM_BAT_ERROR_E;
                  BatMonitorState = DM_BAT_VBAT_MISSING_TEST_E;
               }
               else
               {
                  //Don't start the battery test if the outputs are active
                  if ( DM_OP_OutputsActive(true) )
                  {
                     BatMonitorState = DM_BAT_TEST_POSTPONED_E;
                     CO_PRINT_B_0(DBG_INFO_E,"Postponing battery check because outputs are active\r\n");
                  }
                  else
                  {
                     
                     BatterySampleNumber = 0U;
                     BatMonitorState = DM_BAT_PRIMARY_BAT_CONFIGURE_E;
                     DM_BAT_ClearBatteryData();

                     /* Set the battery test start flag */
                     gBatteryTestStartFlag = 1;
                     /* Reset the battery test restart flag  */
                     gBatteryTestRestartFlag = 0U;

                     CO_PRINT_B_0(DBG_INFO_E,"Running Battery Test\r\n");
                     /* Don't cancel any previous double amber battery LED double flash pattern yet */
                     /* as it will be cleared in REPORT state if no battery error found             */
                     /* DM_LedPatternRemove(LED_MISSING_BATTERY_E); */
                  }
               }
            }
            else if ( gBatteryResultsUpdated )
            {
               //The battery check has been done and we have returned to IDLE.  Send new battery values.
               CO_PRINT_B_0(DBG_INFO_E,"IDLE state: Send Battery 1\r\n");
               DM_BAT_SendBatteryStatus(false);
               gBatteryResultsUpdated = false;
               /* Reset the battery test completion flag */
               gBatteryTestCompleteFlag = 0U;
            }
            else if ( gBatteryTestRestartFlag )
            {
               if( gBatteryTestDelay)
               {
                  IdleCount = 0U;
                  gBatteryTestDelay = 0U;
					   PrimaryLowVoltageEnable = 0U;
					   BackupLowVoltageEnable = 0U;
					   PrimaryTimerCount = 0U;
					   BackupTimerCount = 0U;
               }
               if( BATTERY_TEST_RESTART_INTERVAL <= IdleCount)
               {
                  /* Reset battery check restart flag  */
                  gBatteryTestRestartFlag = 0U; 
                  //Schedule battery test
                  DM_BAT_RequestBatteryCheck(true);
               }
               
            }
            break;
         }
            
         case DM_BAT_TEST_POSTPONED_E:
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_TEST_POSTPONED_E\r\n");
		    //The test was postponed because an output was active.
		    //Start the test when the outputs are no longer active.
		    if ( false == DM_OP_OutputsActive(true) )
		    {
			   DM_BAT_RequestBatteryCheck(true);
		    }
            break;
               
         case DM_BAT_PRIMARY_BAT_CONFIGURE_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_PRIMARY_BAT_CONFIGURE_E\r\n");
            //Request sensor values to be updated for inclusion in the battery status message
            CO_RequestSensorValues();
            //Set the battery test flag in NVM.  It should be reset on test completion.
            DM_BAT_SetBatteryRestartFlag(DM_BAT_PRIMARY_RESTART_E);
            //Ensure primary batteries ON
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
            //Set backup battery OFF
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_OFF_E);
            //Switch on the battery load circuit
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
            
            osDelay(20);
                     
            PassivationCount = 0U;
            BatterySampleNumber = 0U;
            BatMonitorState = DM_BAT_PRIMARY_PASSIVATION_START_E;
            gPassivationDelay = DM_BAT_PASSIVATION_DELAY;
            break;
         }
         case DM_BAT_PRIMARY_PASSIVATION_START_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_PRIMARY_PASSIVATION_START_E\r\n");
            uint32_t BatteryReferenceVoltage_mv = 0U;
            PassivationCount++;
            CO_PRINT_B_2(DBG_INFO_E,"gPassivationDelay= %d, PassivationCount= %d\r\n", gPassivationDelay, PassivationCount);
            if ( gPassivationDelay <= PassivationCount )
            {
               /* Read the ADC reference voltage */
               DM_ADC_ReadReferenceVoltage(&BatteryReferenceVoltage_mv);
               CO_PRINT_B_1(DBG_INFO_E,"ADC Ref voltage=%d\r\n", BatteryReferenceVoltage_mv);
               if ( 0U != BatteryReferenceVoltage_mv )
               {
                  /* Read Primary Vbat */
                  ErrorCode_t result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E);
                  PrimaryBatteryVoltage = Battery[DM_BAT_BATTERY_VBAT_E].Value;
                  if ( SUCCESS_E == result )
                  {
                     uint32_t primary_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(BatteryReferenceVoltage_mv, PrimaryBatteryVoltage, ADC_RESOLUTION_12B);
                     primary_mv *= MCP_HARDWARE_VOLTAGE_CORRECTION;
                     if (!firstReadingDone)
                     {
                        first_primary_voltage_reading = primary_mv;
                        CO_PRINT_B_1(DBG_INFO_E,"Primary Battery voltage 1= %dmV\r\n", first_primary_voltage_reading);
                        firstReadingDone = true;
                        /* Set the delay for the second reading */
                        gPassivationDelay = DM_BAT_PASSIVATION_INTERVAL;
                        PassivationCount = 0U;
                     }
                     else if (!secondReadingDone)
                     {
                        second_primary_voltage_reading = primary_mv;
                        CO_PRINT_B_1(DBG_INFO_E,"Primary Battery voltage 2= %dmV\r\n", second_primary_voltage_reading);
                        secondReadingDone = true;
                        PassivationCount = 0U;
                     }
                     else
                     {
                        /* Do nothing here */
                     }
                  }
                  else
                  {
                     CO_PRINT_B_0(DBG_INFO_E,"Failed to read Primary Battery voltage.\r\n");
                     PassivationCount = 0U;
                     BatMonitorState = DM_BAT_TEST_COMPLETE_E;
                  }
                  
                  if (firstReadingDone && secondReadingDone)
                  {
                     firstReadingDone = false;
                     secondReadingDone = false;
                     CO_PRINT_B_3(DBG_INFO_E,"Primary Battery reading1=%dmV, reading2=%dmV, diff=%dmV\r\n", first_primary_voltage_reading, second_primary_voltage_reading, (second_primary_voltage_reading - first_primary_voltage_reading));
                     if (second_primary_voltage_reading > first_primary_voltage_reading)
                     {
                        if (DM_BAT_PASSIVATION_THRESHOLD_MV < (second_primary_voltage_reading - first_primary_voltage_reading))
                        {
                           /* We have found a passivated battery. Run the process on it */
                           PassivationCount = 0U;
                           PassivationLoadOnOffCount = 0U;
                           PassivationLoadOn = true;
                           BatMonitorState = DM_BAT_PRIMARY_PASSIVATION_PROCESS_E;
                           CO_PRINT_B_0(DBG_INFO_E,"Starting battery passivation process\r\n");
                           SerialDebug_Print(PPU_UART_E, DBG_INFO_E,"Starting battery passivation process\r\n");
                        }
                     }
                     else
                     {
                        /* There is no passivation, just go to the current tests. The loads are already on */
                        SettlingCount = 0U;
                        BatMonitorState = DM_BAT_PRIMARY_BAT_SETTLE_DELAY_E;
                     }
                  }
               }
               else 
               {
                  CO_PRINT_B_0(DBG_INFO_E,"Failed to read ADC Ref voltage.\r\n");
                  PassivationCount = 0U;
                  BatMonitorState = DM_BAT_TEST_COMPLETE_E;
               }
            }
         }
            break;
         
         case DM_BAT_PRIMARY_PASSIVATION_PROCESS_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_PRIMARY_PASSIVATION_PROCESS_E\r\n");
            uint32_t BatteryReferenceVoltage_mv = 0U;
            DePassivationSettings_t depassivation_settings;
            CFG_GetBatteryPassivationValues(&depassivation_settings);
            
            PassivationCount++;
            PassivationLoadOnOffCount++;
            
            //Toggle the load on and off using the times in depassivation_settings
            if ( PassivationLoadOn )
            {
               if ( PassivationLoadOnOffCount >= depassivation_settings.OnTime )
               {
                  //If the load off-time is > 0 switch the load off
                  if ( 0U < depassivation_settings.OffTime )
                  {
                     //Switch off the battery load circuit
                     DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
                     PassivationLoadOn = false;
                  }
                  PassivationLoadOnOffCount = 0U;
               }
            }
            else
            {
               if ( PassivationLoadOnOffCount >= depassivation_settings.OffTime )
               {
                  //Switch on the battery load circuit
                  DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
                  PassivationLoadOn = true;
                  PassivationLoadOnOffCount = 0U;
               }
            }
            
            //Only take readings while the load is on.
            if ( PassivationLoadOn )
            {
               //Read the ADC reference voltage
               if ( SUCCESS_E == DM_ADC_ReadReferenceVoltage(&BatteryReferenceVoltage_mv) )
               {
                  //Read the Primary battery
                  if ( SUCCESS_E == DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E) )
                  {
                     uint32_t primary_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(BatteryReferenceVoltage_mv, PrimaryBatteryVoltage, ADC_RESOLUTION_12B);
                     primary_mv *= MCP_HARDWARE_VOLTAGE_CORRECTION;
                     CO_PRINT_B_1(DBG_INFO_E,"primary_mv=%dmV\r\n", primary_mv);
                     //If the voltage is above the threshold, or we've been waiting long enough to time out,
                     //turn off the loads and complete the passivation process.
                     
                     if ( (depassivation_settings.Voltage <= primary_mv) || (depassivation_settings.Timeout <= PassivationCount) )
                     {
                        DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
                        PassivationCount = 0U;
                        BatMonitorState = DM_BAT_PRIMARY_PASSIVATION_COMPLETE_E;
                        if ( depassivation_settings.Voltage <= primary_mv )
                        {
                           CO_PRINT_B_0(DBG_INFO_E,"Battery passivation complete\r\n");
                           SerialDebug_Print(PPU_UART_E, DBG_INFO_E,"Battery passivation complete\r\n");
                        }
                        else 
                        {
                           CO_PRINT_B_0(DBG_INFO_E,"Battery passivation timeout\r\n");
                           SerialDebug_Print(PPU_UART_E, DBG_INFO_E,"Battery passivation timeout\r\n");
                        }
                     }
                  }
               }
            }
            /* CO_PRINT_B_2(DBG_INFO_E,"depassivation_settings.Voltage=%d, depassivation_settings.Timeout=%d\r\n", depassivation_settings.Voltage, depassivation_settings.Timeout); */
            /* CO_PRINT_B_2(DBG_INFO_E,"depassivation_settings.OffTime=%d, depassivation_settings.OnTime=%d\r\n", depassivation_settings.OffTime, depassivation_settings.OnTime); */
            /* CO_PRINT_B_2(DBG_INFO_E,"PassivationCount=%d, PassivationLoadOnOffCount=%d\r\n", PassivationCount, PassivationLoadOnOffCount); */
            /* CO_PRINT_B_1(DBG_INFO_E,"PassivationLoadOn=%d\r\n", PassivationLoadOn); */
         }
            break;
         
         case DM_BAT_PRIMARY_PASSIVATION_COMPLETE_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_PRIMARY_PASSIVATION_COMPLETE_E\r\n");
            uint32_t BatteryReferenceVoltage_mv = 0U;
            PassivationCount++;
            if ( DM_BAT_PASSIVATION_DELAY == PassivationCount )
            {
               //put the battery load back on
               DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
            }
            else if ( (DM_BAT_PASSIVATION_DELAY *2 ) <= PassivationCount )
            {
               //Read the ADC reference voltage
               DM_ADC_ReadReferenceVoltage(&BatteryReferenceVoltage_mv);
               //Read the Primary battery
               if ( SUCCESS_E == DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E) )
               {
                  uint32_t primary_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(BatteryReferenceVoltage_mv, Battery[DM_BAT_BATTERY_VBAT_E].Value, ADC_RESOLUTION_12B);
                  primary_mv *= MCP_HARDWARE_VOLTAGE_CORRECTION;
                  PrimaryBatteryVoltage = Battery[DM_BAT_BATTERY_VBAT_E].Value;
                  CO_PRINT_B_1(DBG_INFO_E,"Primary Battery V = %dmV\r\n", primary_mv);
               }
               SettlingCount = 0U;
               BatMonitorState = DM_BAT_PRIMARY_BAT_SETTLE_DELAY_E;
            }
            break;
         }
         case DM_BAT_PRIMARY_BAT_SETTLE_DELAY_E:
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_PRIMARY_BAT_SETTLE_DELAY_E\r\n");
            SettlingCount++;
            if ( DM_BAT_MONITOR_SETTLING_TIME <= SettlingCount )
            {
               BatterySampleNumber = 0U;
#ifdef DM_BAT_DO_SMARTNET_CURRENT_TEST
               BatMonitorState = DM_BAT_TEST_CURRENT_SENSE_E;
#else
               BatMonitorState = DM_BAT_TEST_PRIMARY_BATTERY_E;
               PrimaryBatteryVoltage = 0U;
               PrimaryBatteryReferenceVoltage_mv = 0U;
#ifdef DM_BAT_DO_50ms_TEST
               TE_StartBatteryTest();
#endif
#endif
            }
            break;

         case DM_BAT_TEST_PRIMARY_BATTERY_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_TEST_PRIMARY_BATTERY_E\r\n");

            /* Read the ADC reference voltage */
            if ( 0U == PrimaryBatteryReferenceVoltage_mv )
            {
                DM_ADC_ReadReferenceVoltage(&PrimaryBatteryReferenceVoltage_mv);
            }
            /* Read the battery voltage */
            ErrorCode_t result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E);

            if ( SUCCESS_E == result )
            {
                if ( Battery[DM_BAT_BATTERY_VBAT_E].Value > PrimaryBatteryVoltage )
                {
                    PrimaryBatteryVoltage = Battery[DM_BAT_BATTERY_VBAT_E].Value;
                    CO_PRINT_B_1(DBG_INFO_E,"New max Primary Battery raw adc=%d\r\n", PrimaryBatteryVoltage);
                }
            }
            else
            {
                CO_PRINT_B_1(DBG_ERROR_E,"Primary Battery Monitor FAILED error=%d\r\n", result);
            }

            BatteryVoltageReadings[DM_BAT_BATTERY_VBAT_E][BatterySampleNumber] = Battery[DM_BAT_BATTERY_VBAT_E].Value;
            uint32_t battery_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(PrimaryBatteryReferenceVoltage_mv, BatteryVoltageReadings[DM_BAT_BATTERY_VBAT_E][BatterySampleNumber], ADC_RESOLUTION_12B);
            battery_mv *= MCP_HARDWARE_VOLTAGE_CORRECTION;
            BatterySampleNumber++;

            if ( DM_BAT_NUM_OF_BATTERY_READINGS <= BatterySampleNumber )
            {
                /* Turn the battery load circuit OFF */
                DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
                /* Turn backup battery ON */
                DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);

                BatterySampleNumber = 0U;
                BatteryTestResult_t validation_result = DM_BAT_ValidateBatteryMCP(DM_BAT_BATTERY_VBAT_E, DM_BAT_VOLTAGE_TEST_E, DM_BAT_PRIMARY_E);

                if ( DM_BAT_TEST_OK_E == validation_result )
                {
                    CO_PRINT_A_0(DBG_INFO_E,"Primary Battery OK\r\n");
                    gPrimaryTestResult = DM_BAT_TEST_OK_E;
                    PrimaryTimerCount = 0U;
                    PrimaryLowVoltageNumOf10Min = 0U;
                    PrimaryLowVoltageEnable = false;
                }
                else if ( DM_BAT_VOLTAGE_LOW_CHECK_E == validation_result )
                {
                    CO_PRINT_A_0(DBG_ERROR_E,"Primary Battery Test - Low voltage check...\r\n");
                    /* No need to re-test low voltage if it has already been done and confirmed */
                    if (DM_BAT_VOLTAGE_LOW_E != gPrimaryTestResult)
                    {
                       /* This flag indicates low voltage has been detected first time */
                       /* but may not be detected 4 times yet                          */
                       PrimaryLowVoltageEnable = true;
							  /* Clear the Primary battery check loop counter values  */
							  PrimaryTimerCount = 0U;
                       PrimaryLowVoltageNumOf10Min = 0U;
                    }
                }
                else
                {
                    /* Do nothing here */
                }

#ifdef DM_BAT_DO_SMARTNET_CURRENT_TEST
                BatMonitorState = DM_BAT_TEST_CURRENT_SENSE_E;
                BatterySampleNumber = 0U;
                PrimaryBatteryReferenceVoltage_mv = 0U;
#else
                //If we haven't checked the backup battery before, do it now.
                if ( 0U == BackupBatteryVoltage )
                {
                    BatMonitorState = DM_BAT_BACKUP_BAT_CONFIGURE_E;
                }
                else
                {
                    BatMonitorState = DM_BAT_TEST_REPORT_E;
                }
#endif
            }
         }
            break;
            
#ifdef DM_BAT_DO_SMARTNET_CURRENT_TEST
            
         case DM_BAT_TEST_CURRENT_SENSE_E:
         {
//            CO_PRINT_B_0(DBG_INFO_E,"DM_BAT_TEST_CURRENT_SENSE_E\r\n");
            //Read the ADC reference voltage
//            if ( 0 == PrimaryBatteryReferenceVoltage_mv )
//            {
               DM_ADC_ReadReferenceVoltage(&PrimaryBatteryReferenceVoltage_mv);
//            }

            //Read the battery voltage
            ErrorCode_t result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E);
            if ( SUCCESS_E == result )
            {
//               CO_PRINT_B_1(DBG_INFO_E,"Primary Battery Monitor State OK - Value = %d\r\n", Battery[DM_BAT_BATTERY_VBAT_E].Value);
               if ( 0U == PrimaryBatteryVoltage )
               {
                  PrimaryBatteryVoltage = Battery[DM_BAT_BATTERY_VBAT_E].Value;
                  CO_PRINT_B_1(DBG_INFO_E,"Primary Battery raw adc= %d\r\n", PrimaryBatteryVoltage);
               }
               else 
               {
                  //Keep a running average of all battery readings
                  PrimaryBatteryVoltage = (PrimaryBatteryVoltage + Battery[DM_BAT_BATTERY_VBAT_E].Value) / 2;
                  CO_PRINT_B_2(DBG_INFO_E,"Primary Battery raw adc=%d, average=%d\r\n", Battery[DM_BAT_BATTERY_VBAT_E].Value, PrimaryBatteryVoltage);
               }
            }
            else 
            {
               CO_PRINT_B_1(DBG_ERROR_E,"Primary Battery Monitor FAILED error=%d\r\n", result);
            }
            //Store the reading
            BatteryVoltageReadings[DM_BAT_BATTERY_VBAT_E][BatterySampleNumber] = Battery[DM_BAT_BATTERY_VBAT_E].Value;

            //Read current for Battery 1
            result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_1_E);
            if ( SUCCESS_E == result )
            {
               BatteryCurrentReadings[BatterySampleNumber][DM_BAT_BATTERY_1_E] = Battery[DM_BAT_BATTERY_1_E].Value;
               CO_PRINT_B_2(DBG_INFO_E," Battery Monitor State DM_BAT_TEST_BAT_1_E sample %d Value = %d\r\n", BatterySampleNumber, Battery[DM_BAT_BATTERY_1_E].Value);
               
               //Read current for Battery 2
               result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_2_E);
               if ( SUCCESS_E == result )
               {
                  BatteryCurrentReadings[BatterySampleNumber][DM_BAT_BATTERY_2_E] = Battery[DM_BAT_BATTERY_2_E].Value;
                  CO_PRINT_B_2(DBG_INFO_E," Battery Monitor State DM_BAT_TEST_BAT_2_E sample %d Value = %d\r\n", BatterySampleNumber, Battery[DM_BAT_BATTERY_2_E].Value);
                  
                  //Read current for Battery 3
                  result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_3_E);
                  if ( SUCCESS_E == result )
                  {
                     BatteryCurrentReadings[BatterySampleNumber][DM_BAT_BATTERY_3_E] = Battery[DM_BAT_BATTERY_3_E].Value;
                     CO_PRINT_B_2(DBG_INFO_E," Battery Monitor State DM_BAT_TEST_BAT_3_E sample %d Value = %d\r\n", BatterySampleNumber, Battery[DM_BAT_BATTERY_3_E].Value);

                     BatterySampleNumber++;
                     if ( DM_BAT_NUM_OF_BATTERY_READINGS <= BatterySampleNumber )
                     {
                        //Switch off the battery load circuit
                        DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
                        //Set backup battery ON
                        DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
                        
                        BatterySampleNumber = 0U;
                        BatteryTestResult_t voltage_validation_result = DM_BAT_ValidateBatteryMCP(DM_BAT_BATTERY_VBAT_E, DM_BAT_VOLTAGE_TEST_E, DM_BAT_PRIMARY_E);
                        BatteryTestResult_t current_validation_result = DM_BAT_ValidateBatteryMCP(DM_BAT_BATTERY_VBAT_E, DM_BAT_CURRENT_TEST_E, DM_BAT_PRIMARY_E);
                        
                        if ( (DM_BAT_ERROR_E == voltage_validation_result) ||
                             (DM_BAT_ERROR_E == current_validation_result)  )
                        {
                           CO_PRINT_A_0(DBG_ERROR_E,"Primary Battery Test FAULT\r\n");
                           gPrimaryTestResult = DM_BAT_ERROR_E;
                        }
                        else if ( (DM_BAT_VOLTAGE_LOW_E == voltage_validation_result) ||
                                  (DM_BAT_VOLTAGE_LOW_E == current_validation_result)  )
                        {
                           CO_PRINT_A_0(DBG_INFO_E,"Primary Battery LOW\r\n");
                           gPrimaryTestResult = DM_BAT_VOLTAGE_LOW_E;
                        }
                        else if ( (DM_BAT_TEST_OK_E == voltage_validation_result) ||
                                  (DM_BAT_TEST_OK_E == current_validation_result)  )
                        {
                           CO_PRINT_A_0(DBG_INFO_E,"Primary Battery Test OK\r\n");
                           gPrimaryTestResult = DM_BAT_TEST_OK_E;
                        }
                        
                        //If we haven't checked the backup battery before, do it now.
                        if ( 0 == BackupBatteryVoltage )
                        {
                           BatMonitorState = DM_BAT_BACKUP_BAT_CONFIGURE_E;
                        }
                        else 
                        {
                           BatMonitorState = DM_BAT_TEST_REPORT_E;
                        }
                     }
                  }
                  else 
                  {
                     CO_PRINT_B_1(DBG_ERROR_E," Battery Monitor DM_BAT_TEST_BAT_3_E FAILED error=%d\r\n", result);
                     //Abort test
                     IdleCount = 0U;
                     BatMonitorState = DM_BAT_STARTUP_E;
                  }
               }
               else 
               {
                  CO_PRINT_B_1(DBG_ERROR_E," Battery Monitor DM_BAT_TEST_BAT_2_E FAILED error=%d\r\n", result);
                  //Abort test
                  IdleCount = 0U;
                  BatMonitorState = DM_BAT_STARTUP_E;
               }
            }
            else 
            {
               CO_PRINT_B_1(DBG_ERROR_E," Battery Monitor DM_BAT_TEST_BAT_1_E FAILED error=%d\r\n", result);
               //Abort test
               IdleCount = 0U;
               BatMonitorState = DM_BAT_STARTUP_E;
            }
            
            //If we failed to take a reading, remove the 'primary battery missing' restart flag for the next test.
            if (DM_BAT_STARTUP_E == BatMonitorState)
            {
               DM_BAT_SetBatteryRestartFlag(DM_BAT_NORMAL_RESTART_E);
            }
            
            break;
         }
#endif

         case DM_BAT_BACKUP_BAT_CONFIGURE_E:
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_BACKUP_BAT_CONFIGURE_E\r\n");
            /* Turn the battery test flag in NVM.  It should be reset on test completion */
            DM_BAT_SetBatteryRestartFlag(DM_BAT_BACKUP_RESTART_E);
            /* Turn backup battery ON */
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
            /* Turn primary batteries OFF */
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
            /* Turn the battery load circuit ON */
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
            BatterySampleNumber = 0U;
            SettlingCount = 0U;
            BatMonitorState = DM_BAT_BACKUP_BAT_SETTLE_DELAY_E;
            break;
         
         case DM_BAT_BACKUP_BAT_SETTLE_DELAY_E:
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_BACKUP_BAT_SETTLE_DELAY_E\r\n");
            SettlingCount++;
            if ( DM_BAT_MONITOR_SETTLING_TIME <= SettlingCount )
            {
               BatMonitorState = DM_BAT_TEST_BACKUP_BAT_VOLTAGE_E;
               BackupBatteryVoltage = 0U;
               BackupBatteryReferenceVoltage_mv = 0U;
#ifdef DM_BAT_DO_50ms_TEST
               /* We are using the 50ms reading interval, start the timer */
               TE_StartBatteryTest();
#endif
            }
            break;

         case DM_BAT_TEST_BACKUP_BAT_VOLTAGE_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_TEST_BACKUP_BAT_VOLTAGE_E\r\n");

            /* Read the ADC reference voltage */
            if ( 0U == BackupBatteryReferenceVoltage_mv )
            {
                DM_ADC_ReadReferenceVoltage(&BackupBatteryReferenceVoltage_mv);
            }
            /* Read the battery voltage */
            ErrorCode_t result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E);

            if ( SUCCESS_E == result )
            {
                if ( Battery[DM_BAT_BATTERY_VBAT_E].Value > BackupBatteryVoltage )
                {
                    BackupBatteryVoltage = Battery[DM_BAT_BATTERY_VBAT_E].Value;
                    CO_PRINT_B_1(DBG_INFO_E,"New max Backup Battery raw adc=%d\r\n", BackupBatteryVoltage);
                }
            }
            else
            {
                CO_PRINT_B_1(DBG_ERROR_E,"Backup Battery Monitor FAILED error=%d\r\n", result);
            }

            BatteryVoltageReadings[DM_BAT_BATTERY_VBAT_E][BatterySampleNumber] = Battery[DM_BAT_BATTERY_VBAT_E].Value;
            uint32_t battery_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(BackupBatteryReferenceVoltage_mv, BatteryVoltageReadings[DM_BAT_BATTERY_VBAT_E][BatterySampleNumber], ADC_RESOLUTION_12B);
            battery_mv *= MCP_HARDWARE_VOLTAGE_CORRECTION;
            BatterySampleNumber++;

            if ( DM_BAT_NUM_OF_BATTERY_READINGS <= BatterySampleNumber )
            {
                /* Turn the battery load circuit OFF */
                DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
                /* Turn primary batteries ON */
                DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);

                BatterySampleNumber = 0U;
                BatteryTestResult_t validation_result = DM_BAT_ValidateBatteryMCP(DM_BAT_BATTERY_VBAT_E, DM_BAT_VOLTAGE_TEST_E, DM_BAT_BACKUP_E);

                if ( DM_BAT_TEST_OK_E == validation_result )
                {
                    CO_PRINT_A_0(DBG_INFO_E,"Backup Battery OK\r\n");
                    gBackupTestResult = DM_BAT_TEST_OK_E;
                    BackupTimerCount = 0U;
                    BackupLowVoltageNumOf10Min = 0U;
                    BackupLowVoltageEnable = false;
                }
                else if ( DM_BAT_VOLTAGE_LOW_CHECK_E == validation_result )
                {
                    CO_PRINT_A_0(DBG_INFO_E,"Backup Battery Test - Low voltage check...\r\n");
                    /* No need to re-test low voltage if it has already been done and confirmed */
                    if (DM_BAT_VOLTAGE_LOW_E != gBackupTestResult)
                    {
                       /* This flag indicates low voltage has been detected first time */
                       /* but may not be detected 4 times yet                          */
                       BackupLowVoltageEnable = true;
							  /* Clear the backup battery check loop counter values  */
							  BackupTimerCount = 0U;
							  BackupLowVoltageNumOf10Min = 0U;				 
                    }
                }
                else
                {
                    /* Do nothing here */
                }

                BatMonitorState = DM_BAT_TEST_REPORT_E;
            }
         }
            break;

         case DM_BAT_VBAT_MISSING_TEST_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"state= DM_BAT_VBAT_MISSING_TEST_E\r\n");
            /* Called if primary batteries OR backup battery are missing */
            /* Read the ADC reference voltage */
            DM_ADC_ReadReferenceVoltage(&PrimaryBatteryReferenceVoltage_mv);
            CO_PRINT_B_1(DBG_INFO_E,"DM_BAT_VBAT_MISSING_TEST_E ADC Ref voltage=%d\r\n", PrimaryBatteryReferenceVoltage_mv);

            if ( 0U != PrimaryBatteryReferenceVoltage_mv )
            {
               /* Set the backup reference voltage so that the 'get backup voltage' function calculates correctly */
               BackupBatteryReferenceVoltage_mv = PrimaryBatteryReferenceVoltage_mv;

               if ( Battery[DM_BAT_BATTERY_VBAT_E].BatteryDetected )
               {
                  /* Primary batteries are detected */
                  /* Turn primary batteries ON */
                  DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
                  /* Turn backup battery OFF */
                  DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_OFF_E);
               }
               else
               {
                  /* Backup battery is detected */
                  /* Turn backup battery ON */
                  DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
                  /* Turn primary batteries OFF */
                  DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
               }
               /* Turn the battery load circuit ON */
               DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);

               osDelay(20);

               /* Take a Vbat reading */
               ErrorCode_t result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E);
               if ( SUCCESS_E == result )
               {
                  if ( Battery[DM_BAT_BATTERY_VBAT_E].BatteryDetected )
                  {
                     PrimaryBatteryVoltage = Battery[DM_BAT_BATTERY_VBAT_E].Value;
                     BackupBatteryVoltage = 0U; /* adc value */
                  }
                  else
                  {
                     BackupBatteryVoltage = Battery[DM_BAT_BATTERY_VBAT_E].Value;
                     PrimaryBatteryVoltage = 0U; /* adc value */
                  }
                  /* Start the amber battery LED double flash pattern */
                  DM_LedPatternRequest(LED_MISSING_BATTERY_E);
                  /* Send the battery fault result to the application (control panel) */
                  CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_BATTERY_ERROR_E, true, false, false);
                  /* Update the application (control panel) with NO low battery error in case it was set previously */
                  CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_LOW_BATTERY_E, false, false, false);
                  CO_PRINT_B_2(DBG_INFO_E,"Primary battery reading = %d, backup=%d\r\n", PrimaryBatteryVoltage, BackupBatteryVoltage);
                  CO_PRINT_B_0(DBG_INFO_E,"Send Battery 2\r\n");
                  gBatteryResultsUpdated = true;
               }
            }

            /* Turn primary batteries ON */
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
            /* Turn backup battery ON */
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
            /* Turn the battery load circuit OFF */
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);

            /* Get voltages in mV */
            uint32_t primary_voltage_mv = DM_BAT_GetPrimaryBatteryVoltage();
            uint32_t backup_voltage_mv = DM_BAT_GetBackupBatteryVoltage();

            if ( Battery[DM_BAT_BATTERY_VBAT_E].BatteryDetected ) /* Primary detected */
            {
               if ( CFG_GetBatPriLowVoltThresh() > primary_voltage_mv )
               {
                  /* This flag indicates low voltage has been detected first time */
                  /* but may not be detected 4 times yet                          */
                  PrimaryLowVoltageEnable = true;
						/* Clear the Primary battery check loop counter values  */
						PrimaryTimerCount = 0U;
                  PrimaryLowVoltageNumOf10Min = 0U;
               }
            }
            else /* Backup detected */
            {
               if ( CFG_GetBatBkpLowVoltThresh() > backup_voltage_mv )
               {
                  /* This flag indicates low voltage has been detected first time */
                  /* but may not be detected 4 times yet                          */
                  BackupLowVoltageEnable = true;
						/* Clear the backup battery check loop counter values  */
						BackupTimerCount = 0U;
						BackupLowVoltageNumOf10Min = 0U;	 
               }
            }

            /* Check for flat batteries */
            if ( (CFG_GetBatPriLowVoltThresh() > primary_voltage_mv) && (CFG_GetBatBkpLowVoltThresh() > backup_voltage_mv) )
            {
               CO_PRINT_A_2(DBG_INFO_E,"Batteries depleted: prim= %dmV, bkp=%d mV\r\n", primary_voltage_mv, backup_voltage_mv);
               CO_PRINT_A_2(DBG_INFO_E,"Primary threshold= %dmV, Backup threshold= %dmV\r\n", CFG_GetBatPriLowVoltThresh(), CFG_GetBatBkpLowVoltThresh());
               gBatteryDepleted = true;
            }

            BatMonitorState = DM_BAT_IDLE_E;
         }
            break;

         case DM_BAT_TEST_REPORT_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_TEST_REPORT_E\r\n");
            uint32_t primary_mv = DM_BAT_GetPrimaryBatteryVoltage();
            uint32_t backup_mv = DM_BAT_GetBackupBatteryVoltage();
            CO_PRINT_A_4(DBG_INFO_E,"prim_raw=%d, prim=%dmV, bkp_raw=%d, bkp=%dmV\r\n", PrimaryBatteryVoltage, primary_mv, BackupBatteryVoltage, backup_mv);
            CO_PRINT_B_2(DBG_INFO_E,"gPrimaryTestResult=%d, gBackupTestResult=%d\r\n", gPrimaryTestResult, gBackupTestResult);
            CO_PRINT_B_2(DBG_INFO_E,"PrimaryLowVoltageEnable=%d, BackupLowVoltageEnable=%d\r\n", PrimaryLowVoltageEnable, BackupLowVoltageEnable);

            /* If we haven't had a reading of the backup schedule it now */
            if ( 0U == BackupBatteryVoltage )
            {
               BatMonitorState = DM_BAT_BACKUP_BAT_CONFIGURE_E;
            }
            else 
            {
               BatteryFault = false;
               BatteryWarning = false;
               
               BatteryTestResult_t battery_status = DM_BAT_GetBatteryStatus();
               if ( DM_BAT_ERROR_E == battery_status )
               {
                  BatteryFault = true;
                  /* Start the amber battery LED double flash pattern */
                  DM_LedPatternRequest(LED_MISSING_BATTERY_E);
               }
               else if ( DM_BAT_VOLTAGE_LOW_E == battery_status )
               {
                  BatteryWarning = true;
                  /* Start the amber battery LED double flash pattern */
                  DM_LedPatternRequest(LED_MISSING_BATTERY_E);
               }
               else if ( DM_BAT_TEST_OK_E == battery_status )
               {
                  /* No faults. Remove the amber battery LED double flash pattern */
                  DM_LedPatternRemove(LED_MISSING_BATTERY_E);
               }
               else
               {
                  DM_LedPatternRemove(LED_MISSING_BATTERY_E);
               }
               
               /* Check for flat batteries */
               if ( (DM_BAT_VOLTAGE_LOW_E == gPrimaryTestResult) && (DM_BAT_VOLTAGE_LOW_E == gBackupTestResult) )
               {
                  CO_PRINT_A_2(DBG_INFO_E,"Batteries depleted - prim=%dmV, bkp=%dmV\r\n", primary_mv, backup_mv);
                  gBatteryDepleted = true;
               }
               
               /* Update the control panel */
               CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_BATTERY_ERROR_E, BatteryFault, false, false);
               CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_LOW_BATTERY_E, BatteryWarning, false, false);
               BatMonitorState = DM_BAT_TEST_COMPLETE_E;
            }
         }
            break;
            
         case DM_BAT_TEST_COMPLETE_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_TEST_COMPLETE_E\r\n");
            /* Turn battery load circuit OFF */
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
            /* Turn backup battery ON */
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
            if ( (DM_BAT_ERROR_E == gPrimaryTestResult) && (DM_BAT_TEST_OK_E == gBackupTestResult) )
            {
               /* Turn primary batteries OFF */
               DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
            }
            else
            {
               /* Turn primary batteries ON */
               DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
            }
            
            DM_BAT_SetBatteryRestartFlag(DM_BAT_NORMAL_RESTART_E);
            BatMonitorState = DM_BAT_IDLE_E;
            gBatteryResultsUpdated = true;

            /* Set the battery test completion flag */
            gBatteryTestCompleteFlag = 1;
            /* Reset the battery test start flag  */
            gBatteryTestStartFlag = 0U;

            CO_PRINT_B_0(DBG_INFO_E,"Battery test complete\r\n");
            /* If there was a tamper sw cycled and a pri/bkp battery low voltage */
            /* then increase the count by 28s to compensate for the time in      */
            /* configuring and reading the batteries                             */
            /* i.e. period from IDLE state to COMPLETE state                     */
            if (TamperSwCycledPrimaryLowVoltageEnable)
            {
               PrimaryTimerCount += DM_BAT_PERIOD_28SEC;
               TamperSwCycledPrimaryLowVoltageEnable = false;
            }
            if (TamperSwCycledBackupLowVoltageEnable)
            {
               BackupTimerCount += DM_BAT_PERIOD_28SEC;
               TamperSwCycledBackupLowVoltageEnable = false;
            }
         }
            break;

         default:
            BatMonitorState = DM_BAT_IDLE_E;
            break;
      }
   }
}

/*****************************************************************************
*  Function:      DM_BAT_RunConstructionBatteryMonitor
*  Description:   Periodically called to run the battery monitor state machine
*                 for the Construction configuration.
*
*  param          None
*  return         None
*
*  Notes:
*****************************************************************************/
void DM_BAT_RunConstructionBatteryMonitor(void)
{
   if ( BASE_CONSTRUCTION_E == gBaseType )
   {
      switch( BatMonitorState )
      {
         case DM_BAT_STARTUP_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_STARTUP_E\r\n");
            DM_BAT_CheckPriLowBatteryConstr();
            DM_BAT_CheckBkpLowBatteryConstr();
            /* CO_PRINT_A_2(DBG_INFO_E,"gPrimaryTestResult=%d, gBackupTestResult=%d\r\n", gPrimaryTestResult,gBackupTestResult); */
            /* CO_PRINT_A_2(DBG_INFO_E,"PrimaryLowVoltageEnable=%d, BackupLowVoltageEnable=%d\r\n", PrimaryLowVoltageEnable, BackupLowVoltageEnable); */
            /* CO_PRINT_B_1(DBG_INFO_E,"CFG_GetBatPriLowVoltThreshC()= %d\r\n", CFG_GetBatPriLowVoltThreshC()); */
            /* CO_PRINT_B_1(DBG_INFO_E,"CFG_GetBatBkpLowVoltThreshC()= %d\r\n", CFG_GetBatBkpLowVoltThreshC()); */
            IdleCount++;
            //Wait for the start-up delay then trigger a battery test
            if ( DM_BAT_INITIAL_TEST_DELAY <= IdleCount )
            {
               IdleCount = BATTERY_TEST_INTERVAL;
               BatMonitorState = DM_BAT_IDLE_E;
            }
            break;
         }
         case DM_BAT_IDLE_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_IDLE_E\r\n");
            DM_BAT_CheckPriLowBatteryConstr();
            DM_BAT_CheckBkpLowBatteryConstr();
            CO_PRINT_B_2(DBG_INFO_E,"gPrimaryTestResult=%d, gBackupTestResult=%d\r\n", gPrimaryTestResult,gBackupTestResult);
            CO_PRINT_B_2(DBG_INFO_E,"PrimaryLowVoltageEnable=%d, BackupLowVoltageEnable=%d\r\n", PrimaryLowVoltageEnable, BackupLowVoltageEnable);
            IdleCount++;
            CO_PRINT_B_1(DBG_INFO_E,"IdleCount= %d\r\n", IdleCount);
            if ( BATTERY_TEST_INTERVAL <= IdleCount )
            {
               IdleCount = 0U;
               gRestartNvmFlagslags = DM_BAT_GetBatteryRestartFlag();
               CO_PRINT_B_1(DBG_INFO_E,"gRestartNvmFlagslags= %d\r\n", gRestartNvmFlagslags);
               if ( DM_BAT_PRIMARY_RESTART_E == gRestartNvmFlagslags )
               {
                  CO_PRINT_B_0(DBG_INFO_E,"Postponing battery check - primary batteries missing\r\n");
                  Battery[DM_BAT_BATTERY_VBAT_E].BatteryDetected = false;
                  Battery[DM_BAT_BATTERY_3_E].BatteryDetected = true;
                  gPrimaryTestResult = DM_BAT_ERROR_E;
                  BatMonitorState = DM_BAT_VBAT_MISSING_TEST_E;
               }
               else if ( DM_BAT_BACKUP_RESTART_E == gRestartNvmFlagslags )
               {
                  CO_PRINT_B_0(DBG_INFO_E,"Postponing battery check - backup batteries missing\r\n");
                  Battery[DM_BAT_BATTERY_VBAT_E].BatteryDetected = true;
                  Battery[DM_BAT_BATTERY_3_E].BatteryDetected = false;
                  gBackupTestResult = DM_BAT_ERROR_E;
                  BatMonitorState = DM_BAT_VBAT_MISSING_TEST_E;
               }
               else
               {
                  /* Don't start the battery test if the outputs are active */
                  if ( DM_OP_OutputsActive(true) )
                  {
                     BatMonitorState = DM_BAT_TEST_POSTPONED_E;
                     CO_PRINT_B_0(DBG_INFO_E,"Postponing battery check because outputs are active\r\n");
                  }
                  else
                  {
                     BatterySampleNumber = 0U;
                     BatMonitorState = DM_BAT_PRIMARY_BAT_CONFIGURE_E;
                     DM_BAT_ClearBatteryData();

                     /* Set the battery test start flag  */
                     gBatteryTestStartFlag = 1;
                     /* Reset the battery test restart flag  */
                     gBatteryTestRestartFlag = 0U;

                     CO_PRINT_B_0(DBG_INFO_E,"Running Battery Test\r\n");
                     /* Don't cancel any previous double amber battery LED double flash pattern yet */
                     /* as it will be cleared in REPORT state if no battery error found             */
                     /* DM_LedPatternRemove(LED_MISSING_BATTERY_E); */
                  }
               }
            }
            else if ( gBatteryResultsUpdated )
            {
               //The battery check has been done and we have returned to IDLE.  Send new battery values.
               CO_PRINT_B_0(DBG_INFO_E,"IDLE state: Send Battery 1\r\n");
               DM_BAT_SendBatteryStatus(false);
               gBatteryResultsUpdated = false;
               /* Reset the battery test completion flag */
               gBatteryTestCompleteFlag = 0U;
            }
            else if ( gBatteryTestRestartFlag )
            {
               if( gBatteryTestDelay)
               {
                  IdleCount = 0U;
                  gBatteryTestDelay = 0U;
					   PrimaryLowVoltageEnable = 0U;
					   BackupLowVoltageEnable = 0U;
					   PrimaryTimerCount = 0U;
					   BackupTimerCount = 0U;
               }
               if( BATTERY_TEST_RESTART_INTERVAL <= IdleCount )
               {
                  /* Reset battery test restart flag  */
                  gBatteryTestRestartFlag = 0U;
                  //Schedule battery test
                   DM_BAT_RequestBatteryCheck(true);
               }

            }
            break;
         }
            
         case DM_BAT_TEST_POSTPONED_E:
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_TEST_POSTPONED_E\r\n");
            /* The test was postponed because an output was active */
            /* Start the test when the outputs are no longer active */
            if ( false == DM_OP_OutputsActive(true) )
            {
               DM_BAT_RequestBatteryCheck(true);
            }
            break;
               
         case DM_BAT_PRIMARY_BAT_CONFIGURE_E:
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_PRIMARY_BAT_CONFIGURE_E\r\n");
            /* Request sensor values to be updated for inclusion in the battery status message */
            CO_RequestSensorValues();
            /* Turn the battery test flag in NVM. It should be reset on test completion */
            DM_BAT_SetBatteryRestartFlag(DM_BAT_PRIMARY_RESTART_E);
            /* Turn backup battery load circuit OFF */
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
            /* Turn primary battery ON */
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
            /* Turn the backup battery OFF */
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_OFF_E);
            /* Turn primary battery load circuit ON */
            DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_ON_E);
            BatterySampleNumber = 0U;
            SettlingCount = 0U;
            BatMonitorState = DM_BAT_PRIMARY_BAT_SETTLE_DELAY_E;
            break;
         
         case DM_BAT_PRIMARY_BAT_SETTLE_DELAY_E:
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_PRIMARY_BAT_SETTLE_DELAY_E\r\n");
            SettlingCount++;
            if ( DM_BAT_MONITOR_SETTLING_TIME <= SettlingCount )
            {
               BatMonitorState = DM_BAT_TEST_PRIMARY_BATTERY_E;
               PrimaryBatteryVoltage = 0U;
               PrimaryBatteryReferenceVoltage_mv = 0U;
#ifdef DM_BAT_DO_50ms_TEST
               /* We are using the 50ms reading interval, start the timer */
               TE_StartBatteryTest();
#endif
            }
            break;
               
         case DM_BAT_TEST_PRIMARY_BATTERY_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_TEST_PRIMARY_BATTERY_E\r\n");
            /* Read the ADC reference voltage */
            if ( 0U == PrimaryBatteryReferenceVoltage_mv )
            {
               DM_ADC_ReadReferenceVoltage(&PrimaryBatteryReferenceVoltage_mv);
            }

            /* Read the battery voltage */
            ErrorCode_t result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E);

            if ( SUCCESS_E == result )
            {
               if ( Battery[DM_BAT_BATTERY_VBAT_E].Value > PrimaryBatteryVoltage )
               {
                  PrimaryBatteryVoltage = Battery[DM_BAT_BATTERY_VBAT_E].Value;
                  CO_PRINT_B_1(DBG_INFO_E,"New max Primary Battery raw adc=%d\r\n", PrimaryBatteryVoltage);
               }
            }
            else
            {
                CO_PRINT_B_1(DBG_ERROR_E,"Primary Battery Monitor FAILED error=%d\r\n", result);
            }

            BatteryVoltageReadings[DM_BAT_BATTERY_VBAT_E][BatterySampleNumber] = Battery[DM_BAT_BATTERY_VBAT_E].Value;
            uint32_t battery_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(PrimaryBatteryReferenceVoltage_mv, BatteryVoltageReadings[DM_BAT_BATTERY_VBAT_E][BatterySampleNumber], ADC_RESOLUTION_12B);
            battery_mv *= CONSTRUCTION_HARDWARE_VOLTAGE_CORRECTION;
            BatterySampleNumber++;

            if ( DM_BAT_NUM_OF_BATTERY_READINGS <= BatterySampleNumber )
            {
                /* Turn battery load circuit OFF */
                DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);
                /* Turn backup battery ON */
                DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);

                BatterySampleNumber = 0U;
                BatteryTestResult_t validation_result = DM_BAT_ValidateBatteryConstruction(DM_BAT_BATTERY_VBAT_E, DM_BAT_VOLTAGE_TEST_E, DM_BAT_PRIMARY_E);

                if ( DM_BAT_TEST_OK_E == validation_result )
                {
                    CO_PRINT_A_0(DBG_INFO_E,"Primary Battery OK\r\n");
                    gPrimaryTestResult = DM_BAT_TEST_OK_E;
                    PrimaryTimerCount = 0U;
                    PrimaryLowVoltageNumOf10Min = 0U;
                    PrimaryLowVoltageEnable = false;
                }
                else if ( DM_BAT_VOLTAGE_LOW_CHECK_E == validation_result )
                {
                    CO_PRINT_A_0(DBG_ERROR_E,"Primary Battery Test - Low voltage check...\r\n");
                    /* No need to re-test low voltage if it has already been done and confirmed */
                    if (DM_BAT_VOLTAGE_LOW_E != gPrimaryTestResult)
                    {
                       /* This flag indicates low voltage has been detected first time */
                       /* but may not be detected 4 times yet                          */
                       PrimaryLowVoltageEnable = true;
							  /* Clear the Primary battery check loop counter values  */
							  PrimaryTimerCount = 0U;
                       PrimaryLowVoltageNumOf10Min = 0U;
                    }
                }
                else
                {
                    /* Do nothing here */
                }

                BatMonitorState = DM_BAT_BACKUP_BAT_CONFIGURE_E;
            }
         }
            break;

         case DM_BAT_BACKUP_BAT_CONFIGURE_E:
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_BACKUP_BAT_CONFIGURE_E\r\n");
            /* Turn the battery test flag in NVM. It should be reset on test completion */
            DM_BAT_SetBatteryRestartFlag(DM_BAT_BACKUP_RESTART_E);
            /* Turn backup battery ON */
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
            /* Turn backup battery load circuit ON */
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
            /* Turn primary battery OFF */
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
            /* Turn primary battery load circuit OFF */
            DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);
            BatterySampleNumber = 0U;
            SettlingCount = 0U;
            BatMonitorState = DM_BAT_BACKUP_BAT_SETTLE_DELAY_E;
            break;
         
         case DM_BAT_BACKUP_BAT_SETTLE_DELAY_E:
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_BACKUP_BAT_SETTLE_DELAY_E\r\n");
            SettlingCount++;
            if ( DM_BAT_MONITOR_SETTLING_TIME <= SettlingCount )
            {
               BatMonitorState = DM_BAT_TEST_BACKUP_BAT_VOLTAGE_E;
               BackupBatteryVoltage = 0U;
               BackupBatteryReferenceVoltage_mv = 0U;
#ifdef DM_BAT_DO_50ms_TEST
               /* We are using the 50ms reading interval, start the timer */
               TE_StartBatteryTest();
#endif
            }
            break;
               
         case DM_BAT_TEST_BACKUP_BAT_VOLTAGE_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_TEST_BACKUP_BAT_VOLTAGE_E\r\n");

            /* Read the ADC reference voltage */
            if ( 0U == BackupBatteryReferenceVoltage_mv )
            {
                DM_ADC_ReadReferenceVoltage(&BackupBatteryReferenceVoltage_mv);
            }
            /* Read the battery voltage */
            ErrorCode_t result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_3_E);

            if ( SUCCESS_E == result )
            {
                if ( Battery[DM_BAT_BATTERY_VBAT_E].Value > BackupBatteryVoltage )
                {
                    BackupBatteryVoltage = Battery[DM_BAT_BATTERY_3_E].Value;
                    CO_PRINT_B_1(DBG_INFO_E,"New max Backup Battery raw adc=%d\r\n", BackupBatteryVoltage);
                }
            }
            else
            {
                CO_PRINT_B_1(DBG_ERROR_E,"Backup Battery Monitor FAILED error=%d\r\n", result);
            }

            BatteryVoltageReadings[DM_BAT_BATTERY_3_E][BatterySampleNumber] = Battery[DM_BAT_BATTERY_3_E].Value;
            uint32_t battery_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(BackupBatteryReferenceVoltage_mv, BatteryVoltageReadings[DM_BAT_BATTERY_3_E][BatterySampleNumber], ADC_RESOLUTION_12B);
            battery_mv *= CONSTRUCTION_HARDWARE_VOLTAGE_CORRECTION;
            /* CO_PRINT_B_2(DBG_ERROR_E,"Battery[DM_BAT_BATTERY_3_E].Value=%d, BackupBatteryVoltage=%d\r\n", Battery[DM_BAT_BATTERY_3_E].Value, BackupBatteryVoltage); */
            /* CO_PRINT_B_2(DBG_ERROR_E,"BatteryVoltageReadings[DM_BAT_BATTERY_3_E][BatterySampleNumber]=%d, battery_mv=%dmV\r\n", BatteryVoltageReadings[DM_BAT_BATTERY_3_E][BatterySampleNumber], battery_mv); */
            BatterySampleNumber++;

            if ( DM_BAT_NUM_OF_BATTERY_READINGS <= BatterySampleNumber )
            {
                /* Turn the battery load circuit OFF */
                DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
                /* Turn primary battery ON */
                DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);

                BatterySampleNumber = 0U;
                BatteryTestResult_t validation_result = DM_BAT_ValidateBatteryConstruction(DM_BAT_BATTERY_3_E, DM_BAT_VOLTAGE_TEST_E, DM_BAT_BACKUP_E);

                if ( DM_BAT_TEST_OK_E == validation_result )
                {
                    CO_PRINT_A_0(DBG_INFO_E,"Backup Battery OK\r\n");
                    gBackupTestResult = DM_BAT_TEST_OK_E;
                    BackupTimerCount = 0U;
                    BackupLowVoltageNumOf10Min = 0U;
                    BackupLowVoltageEnable = false;
                }
                else if ( DM_BAT_VOLTAGE_LOW_CHECK_E == validation_result )
                {
                    CO_PRINT_A_0(DBG_INFO_E,"Backup Battery Test - Low voltage check...\r\n");
                    /* No need to re-test low voltage if it has already been done and confirmed */
                    if (DM_BAT_VOLTAGE_LOW_E != gBackupTestResult)
                    {
                       /* This flag indicates low voltage has been detected first time */
                       /* but may not be detected 4 times yet                          */
                       BackupLowVoltageEnable = true;
							  /* Clear the Backup battery check loop counter values  */
							  BackupTimerCount = 0U;
                       BackupLowVoltageNumOf10Min = 0U;
                    }
                }
                else
                {
                    /* Do nothing here */
                }

                BatMonitorState = DM_BAT_TEST_REPORT_E;
            }
         }
            break;

         case DM_BAT_VBAT_MISSING_TEST_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_VBAT_MISSING_TEST_E\r\n");
            /* Read the ADC reference voltage */
            DM_ADC_ReadReferenceVoltage(&PrimaryBatteryReferenceVoltage_mv);
            BackupBatteryReferenceVoltage_mv = PrimaryBatteryReferenceVoltage_mv;

            CO_PRINT_B_1(DBG_INFO_E,"ADC Ref voltage=%d\r\n", PrimaryBatteryReferenceVoltage_mv);

            if ( 0U != PrimaryBatteryReferenceVoltage_mv )
            {
               /* Called if batteries are missing to send what values we have to the panel */
               if ( Battery[DM_BAT_BATTERY_VBAT_E].BatteryDetected )
               {
                  /* Primary battery is detected */
                  /* Backup batteries are missing */
                  BackupBatteryVoltage = 0U;              /* adc value */
                  Battery[DM_BAT_BATTERY_3_E].Value = 0U; /* adc value */
                  /* Update the Application (control panel) with the battery error */
                  CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_BATTERY_ERROR_E, true, false, false);
                  /* Update the application (control panel) with NO low battery error in case it was set previously */
                  CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_LOW_BATTERY_E, false, false, false);
                  /* Start the amber battery LED double flash pattern */
                  DM_LedPatternRequest(LED_MISSING_BATTERY_E);
                  
                  /* Turn primary batteries ON */
                  DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
                  /* Turn the battery load circuit ON */
                  DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_ON_E);
                  /* Turn backup battery OFF */
                  DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_OFF_E);
                  /* Turn the backup battery load circuit OFF */
                  DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
                  osDelay(20);
                  /* Take a Vbat reading */
                  ErrorCode_t result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E);
                  /* Turn the primary battery load circuit OFF */
                  DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);

                  if ( SUCCESS_E == result )
                  {
                     PrimaryBatteryVoltage = Battery[DM_BAT_BATTERY_VBAT_E].Value;
                     CO_PRINT_B_0(DBG_INFO_E,"Send Battery 4\r\n");
                     gBatteryResultsUpdated = true;
                  }
               }
               else if ( Battery[DM_BAT_BATTERY_3_E].BatteryDetected )
               {
                  /* Backup batteries are detected */
                  /* Primary battery is missing */
                  PrimaryBatteryVoltage = 0U;                /* adc value */
                  Battery[DM_BAT_BATTERY_VBAT_E].Value = 0U; /* adc value */
                  /* Update the Application (control panel) with the battery error */
                  CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_BATTERY_ERROR_E, true, false, false);
                  /* Update the application (control panel) with NO low battery error in case it was set previously */
                  CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_LOW_BATTERY_E, false, false, false);
                  /* Start the amber battery LED double flash pattern */
                  DM_LedPatternRequest(LED_MISSING_BATTERY_E);

                  /* Turn backup battery ON */
                  DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
                  /* Turn the backup battery load circuit ON */
                  DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
                  /* Turn primary battery OFF */
                  DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
                  /* Turn the primary battery load circuit OFF */
                  DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);
                  osDelay(20);
                  /* Take a Vbat reading */
                  ErrorCode_t result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_3_E);

                  if ( SUCCESS_E == result )
                  {
                     BackupBatteryVoltage = Battery[DM_BAT_BATTERY_3_E].Value;
                     CO_PRINT_B_0(DBG_INFO_E,"Send Battery 5\r\n");
                     gBatteryResultsUpdated = true;
                  }
               }
               else
               {
                  /* Do nothing here */
               }
            }

            /* Turn primary batteries ON */
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
            /* Turn the primary battery load circuit OFF */
            DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);
            /* Turn backup battery ON */
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
            /* Turn the backup battery load circuit OFF */
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);

            /* Get voltages in mV */
            uint32_t primary_voltage_mv = DM_BAT_GetPrimaryBatteryVoltage();
            uint32_t backup_voltage_mv = DM_BAT_GetBackupBatteryVoltage();

            if ( Battery[DM_BAT_BATTERY_VBAT_E].BatteryDetected ) /* Primary detected */
            {
               if ( CFG_GetBatPriLowVoltThreshC() > primary_voltage_mv )
               {
                  /* This flag indicates low voltage has been detected first time */
                  /* but may not be detected 4 times yet                          */
                  PrimaryLowVoltageEnable = true;
						/* Clear the Primary battery check loop counter values  */
						PrimaryTimerCount = 0U;
                  PrimaryLowVoltageNumOf10Min = 0U;
               }
            }
            else if ( Battery[DM_BAT_BATTERY_3_E].BatteryDetected ) /* Backup detected */
            {
               if ( CFG_GetBatBkpLowVoltThreshC() > backup_voltage_mv )
               {
                  /* This flag indicates low voltage has been detected first time */
                  /* but may not be detected 4 times yet                          */
                  BackupLowVoltageEnable = true;
						/* Clear the Backup battery check loop counter values  */
						BackupTimerCount = 0U;
                  BackupLowVoltageNumOf10Min = 0U;
               }
            }
            else
            {
               /* Do nothing here */
            }

            /* Check for flat batteries */
            if ( (CFG_GetBatPriLowVoltThreshC() > primary_voltage_mv) && (CFG_GetBatBkpLowVoltThreshC() > backup_voltage_mv) )
            {
               CO_PRINT_A_2(DBG_INFO_E,"Batteries depleted: prim= %dmV, bkp=%d mV\r\n", primary_voltage_mv, backup_voltage_mv);
               CO_PRINT_A_2(DBG_INFO_E,"Primary threshold= %dmV, Backup threshold= %dmV\r\n", CFG_GetBatPriLowVoltThreshC(), CFG_GetBatBkpLowVoltThreshC());
               gBatteryDepleted = true;
            }

            BatMonitorState = DM_BAT_IDLE_E;
         }
            break;

         case DM_BAT_TEST_REPORT_E:
         {
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_TEST_REPORT_E\r\n");
            uint32_t primary_mv = DM_BAT_GetPrimaryBatteryVoltage();
            uint32_t backup_mv = DM_BAT_GetBackupBatteryVoltage();
            CO_PRINT_A_4(DBG_INFO_E,"prim_raw=%d, prim=%dmV, bkp_raw=%d, bkp=%dmV\r\n", PrimaryBatteryVoltage, primary_mv, BackupBatteryVoltage, backup_mv);
            CO_PRINT_B_2(DBG_INFO_E,"gPrimaryTestResult=%d, gBackupTestResult=%d\r\n", gPrimaryTestResult, gBackupTestResult);
            CO_PRINT_B_2(DBG_INFO_E,"PrimaryLowVoltageEnable=%d, BackupLowVoltageEnable=%d\r\n", PrimaryLowVoltageEnable, BackupLowVoltageEnable);
            
            /* If we haven't had a reading of the backup schedule it now */
            if ( 0U == BackupBatteryVoltage )
            {
               BatMonitorState = DM_BAT_BACKUP_BAT_CONFIGURE_E;
            }
            else 
            {
               BatteryFault = false;
               BatteryWarning = false;

               BatteryTestResult_t battery_status = DM_BAT_GetBatteryStatus();
               if ( DM_BAT_ERROR_E == battery_status )
               {
                  BatteryFault = true;
                  /* Start the amber battery LED double flash pattern */
                  DM_LedPatternRequest(LED_MISSING_BATTERY_E);
               }
               else if ( DM_BAT_VOLTAGE_LOW_E == battery_status )
               {
                  BatteryWarning = true;
                  /* Start the amber battery LED double flash pattern */
                  DM_LedPatternRequest(LED_MISSING_BATTERY_E);
               }
               else if ( DM_BAT_TEST_OK_E == battery_status )
               {
                  /* No faults. Remove the amber battery LED double flash pattern */
                  DM_LedPatternRemove(LED_MISSING_BATTERY_E);
               }
               else
               {
                  DM_LedPatternRemove(LED_MISSING_BATTERY_E);
               }

               /* Check for flat batteries */
               if ( (DM_BAT_VOLTAGE_LOW_E == gPrimaryTestResult) && (DM_BAT_VOLTAGE_LOW_E == gBackupTestResult) )
               {
                  CO_PRINT_A_2(DBG_INFO_E,"Batteries depleted - prim=%dmV, bkp=%dmV\r\n", primary_mv, backup_mv);
                  gBatteryDepleted = true;
               }

               /* Update the control panel */
               CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_BATTERY_ERROR_E, BatteryFault, false, false);
               CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_LOW_BATTERY_E, BatteryWarning, false, false);

               BatMonitorState = DM_BAT_TEST_COMPLETE_E;
            }
         }
            break;
            
         case DM_BAT_TEST_COMPLETE_E:
         {		 
            CO_PRINT_B_0(DBG_INFO_E,"bmstate= DM_BAT_TEST_COMPLETE_E\r\n");
			   /* Turn primary battery load circuit OFF */
            DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);
            /* Turn backup battery load circuit OFF */
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
            /* Turn backup battery ON */
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
            if ( (DM_BAT_ERROR_E == gPrimaryTestResult) && (DM_BAT_TEST_OK_E == gBackupTestResult) )
            {
               /* Turn primary battery OFF */
               DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
            }
            else
            {
               /* Turn primary battery ON */
               DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
            }
            
            DM_BAT_SetBatteryRestartFlag(DM_BAT_NORMAL_RESTART_E);
            BatMonitorState = DM_BAT_IDLE_E;
            gBatteryResultsUpdated = true;

            /* Set the battery test completion flag */
            gBatteryTestCompleteFlag = 1;
            /* Reset the battery test start flag  */
            gBatteryTestStartFlag = 0U;

            CO_PRINT_B_0(DBG_INFO_E,"Battery test complete\r\n");
            /* If there was a tamper sw cycled and a pri/bkp battery low voltage */
            /* then increase the count by 15s to compensate for the time in      */
            /* configuring and reading the batteries                             */
            /* i.e. period from IDLE state to COMPLETE state                     */
            if (TamperSwCycledPrimaryLowVoltageEnable)
            {
               PrimaryTimerCount += DM_BAT_PERIOD_15SEC;
               TamperSwCycledPrimaryLowVoltageEnable = false;
            }
            if (TamperSwCycledBackupLowVoltageEnable)
            {
               BackupTimerCount += DM_BAT_PERIOD_15SEC;
               TamperSwCycledBackupLowVoltageEnable = false;
            }
         }
            break;
         default:
            BatMonitorState = DM_BAT_IDLE_E;
            break;
      }
   }
}

#endif //SE_BATTERY

/*****************************************************************************
*  Function:      DM_BAT_ReadBatteryValue
*  Description:   Reads the voltage of a specified battery.
*
*  param   battery   The ID of the battery to be read.
*  return            SUCCESS_E or ERR_INIT_FAIL_E
*
*  Notes:
*****************************************************************************/
ErrorCode_t DM_BAT_ReadBatteryValue(const BatteryID_t battery)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( Battery[battery].ADC )
   {
      result = DM_ADC_Read( Battery[battery].ADC, Battery[battery].Channel, 0, &Battery[battery].Value);
      CO_PRINT_B_3(DBG_INFO_E,"Battery  %d value %d, Err=%d\r\n", battery, Battery[battery].Value, result);
   }
   return result;
}

/*****************************************************************************
*  Function:      DM_BAT_ValidateBatteryMCP
*  Description:   Analyses the latest battery readings and checks for good health.
*                 Call this after reading the voltage of a battery for 
*                 DM_BAT_NUM_OF_BATTERY_READINGS samples, passing in the
*                 ID of the battery that was read.
*
*  param    battery_id   The ID of the battery to be validated
*  param    test_type    Determines which validation to apply
*  param    type         Determines which type i.e. primary or backup
*
*  return   test_result  Result code of validation test
*
*  Note:    For SmartNet
*****************************************************************************/
BatteryTestResult_t DM_BAT_ValidateBatteryMCP(const BatteryID_t battery_id, const BatteryTestType_t test_type, const DM_BAT_Battery_t type)
{
   uint32_t battery_mv = 0U;
   uint32_t battery_low_count = 0U;
   BatteryTestResult_t test_result = DM_BAT_TEST_OK_E;
   if ( DM_BAT_VOLTAGE_TEST_E == test_type )
   {
      /* Establish the correct threshold for low battery */
      if ( 0U != PrimaryBatteryReferenceVoltage_mv )
      {
         /* Default state to 'battery OK' */
         Battery[battery_id].State = DM_BAT_TEST_OK_E;
         
         for ( uint8_t bat_reading = 0U; bat_reading < DM_BAT_NUM_OF_BATTERY_READINGS; bat_reading++ )
         {
            battery_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(PrimaryBatteryReferenceVoltage_mv, BatteryVoltageReadings[battery_id][bat_reading], ADC_RESOLUTION_12B);
            battery_mv *= MCP_HARDWARE_VOLTAGE_CORRECTION;

            if (DM_BAT_PRIMARY_E == type)
            {
               if ( CFG_GetBatPriLowVoltThresh() > battery_mv )
               {
                  battery_low_count++;
               }
            }
            else if (DM_BAT_BACKUP_E == type)
            {
               if ( CFG_GetBatBkpLowVoltThresh() > battery_mv )
               {
                  battery_low_count++;
               }
            }
            else
            {
               /* Do nothing here */
            }
            CO_PRINT_B_4(DBG_INFO_E,"Battery %d reading %d, voltage=%d, fail threshold=%d\r\n", battery_id, (bat_reading + 1U), battery_mv,
               (DM_BAT_PRIMARY_E == type)? CFG_GetBatPriLowVoltThresh(): CFG_GetBatBkpLowVoltThresh());
         }
         
         if ( DM_BAT_NUM_OF_BATTERY_READINGS <= battery_low_count )
         {
            /* Voltage test has seen three quick consecutive low readings */
            test_result = DM_BAT_VOLTAGE_LOW_CHECK_E;
         }
            
         Battery[battery_id].State = test_result;
      }
      else 
      {
         test_result = DM_BAT_INVALID_TEST_E;
         CO_PRINT_A_0(DBG_ERROR_E,"Failed to read ADC reference\r\n");
      }
   }
   else if ( DM_BAT_CURRENT_TEST_E == test_type )
   {
#ifdef DM_BAT_DO_SMARTNET_CURRENT_TEST
      if ( DM_BAT_BATTERY_VBAT_E == battery_id )
      {
         Battery[DM_BAT_BATTERY_1_E].State = DM_BAT_ERROR_E;
         Battery[DM_BAT_BATTERY_2_E].State = DM_BAT_ERROR_E;
         Battery[DM_BAT_BATTERY_3_E].State = DM_BAT_ERROR_E;
         Battery[DM_BAT_BATTERY_1_E].BatteryDetected = false;
         Battery[DM_BAT_BATTERY_2_E].BatteryDetected = false;
         Battery[DM_BAT_BATTERY_3_E].BatteryDetected = false;
         
         for ( uint8_t sample = 0U; sample < DM_BAT_NUM_OF_BATTERY_READINGS; sample++ )
         {
#ifdef DM_BAT_DO_SMARTNET_CURRENT_WINDOW_TEST
            bool cell_1_current_ok = false;
            bool cell_2_current_ok = false;
            bool cell_3_current_ok = false;
            uint32_t battery_total = BatteryCurrentReadings[sample][DM_BAT_BATTERY_1_E] + BatteryCurrentReadings[sample][DM_BAT_BATTERY_2_E] + BatteryCurrentReadings[sample][DM_BAT_BATTERY_3_E];
            uint32_t low_charge_threshold = (battery_total * DM_BAT_LOW_CURRENT_PERCENTAGE) / 100;
            if ( BatteryCurrentReadings[sample][DM_BAT_BATTERY_1_E] > low_charge_threshold )
            {
               //battery 1 is contributing current
               cell_1_current_ok = true;
               Battery[DM_BAT_BATTERY_1_E].State = DM_BAT_TEST_OK_E;
               Battery[DM_BAT_BATTERY_1_E].BatteryDetected = true;
            }
            else 
            {
               //battery 1 is not contributing current
               Battery[DM_BAT_BATTERY_1_E].BatteryDetected = false;
            }
            
            if ( BatteryCurrentReadings[sample][DM_BAT_BATTERY_2_E] > low_charge_threshold )
            {
               //battery 2 is contributing current
               cell_2_current_ok = true;
               Battery[DM_BAT_BATTERY_2_E].State = DM_BAT_TEST_OK_E;
               Battery[DM_BAT_BATTERY_2_E].BatteryDetected = true;
            }
            else 
            {
               //battery 2 is not contributing current
               Battery[DM_BAT_BATTERY_2_E].BatteryDetected = false;
            }
            
            if ( BatteryCurrentReadings[sample][DM_BAT_BATTERY_3_E] > low_charge_threshold )
            {
               //battery 3 is contributing current
               cell_3_current_ok = true;
               Battery[DM_BAT_BATTERY_3_E].State = DM_BAT_TEST_OK_E;
               Battery[DM_BAT_BATTERY_3_E].BatteryDetected = true;
            }
            else 
            {
               //battery 3 is not contributing current
               Battery[DM_BAT_BATTERY_3_E].BatteryDetected = false;
            }
            
            CO_PRINT_B_4(DBG_INFO_E,"Current - Threshold=%d, Bat1=%d, Bat2=%d, Bat3=%d\r\n", low_charge_threshold, BatteryCurrentReadings[0][DM_BAT_BATTERY_1_E], BatteryCurrentReadings[0][DM_BAT_BATTERY_2_E], BatteryCurrentReadings[0][DM_BAT_BATTERY_3_E]);
            
            if ( (false == cell_1_current_ok) || (false == cell_2_current_ok) || (false == cell_3_current_ok) )
            {
               test_result = DM_BAT_ERROR_E;
            }
#endif
            if ( DM_BAT_DETECTION_THRESHOLD <= BatteryCurrentReadings[sample][DM_BAT_BATTERY_1_E] )
            {
               //battery 1 is contributing current
               Battery[DM_BAT_BATTERY_1_E].BatteryDetected = true;
            }
            
            if ( DM_BAT_DETECTION_THRESHOLD <= BatteryCurrentReadings[sample][DM_BAT_BATTERY_2_E] )
            {
               //battery 2 is contributing current
               Battery[DM_BAT_BATTERY_2_E].BatteryDetected = true;
            }
            
            if ( DM_BAT_DETECTION_THRESHOLD <= BatteryCurrentReadings[sample][DM_BAT_BATTERY_3_E] )
            {
               //battery 3 is contributing current
               Battery[DM_BAT_BATTERY_3_E].BatteryDetected = true;
            }
         }
         
         if ( Battery[DM_BAT_BATTERY_1_E].BatteryDetected )
         {
            CO_PRINT_B_0(DBG_INFO_E,"Primary Battery 1 detected\r\n");
         }
         else 
         {
            CO_PRINT_B_0(DBG_INFO_E,"Primary Battery 1 missing\r\n");
            test_result = DM_BAT_ERROR_E;
         }
         
         if ( Battery[DM_BAT_BATTERY_2_E].BatteryDetected )
         {
            CO_PRINT_B_0(DBG_INFO_E,"Primary Battery 2 detected\r\n");
         }
         else 
         {
            CO_PRINT_B_0(DBG_INFO_E,"Primary Battery 2 missing\r\n");
            test_result = DM_BAT_ERROR_E;
         }
         
         if ( Battery[DM_BAT_BATTERY_3_E].BatteryDetected )
         {
            CO_PRINT_B_0(DBG_INFO_E,"Primary Battery 3 detected\r\n");
         }
         else 
         {
            CO_PRINT_B_0(DBG_INFO_E,"Primary Battery 3 missing\r\n");
            test_result = DM_BAT_ERROR_E;
         }
      }
      else 
      {
         test_result = DM_BAT_INVALID_TEST_E;
      }
#else
      //Not checking individual batteries.  Assume all three are present to prevent false warnings.
      Battery[DM_BAT_BATTERY_1_E].BatteryDetected = true;
      Battery[DM_BAT_BATTERY_2_E].BatteryDetected = true;
      Battery[DM_BAT_BATTERY_3_E].BatteryDetected = true;
#endif
   }
   
   return test_result;
}

/*****************************************************************************
*  Function:      DM_BAT_ValidateBatteryConstruction
*  Description:   Analyses the latest battery readings and checks for good health.
*                 Call this after reading the voltage of a battery for 
*                 DM_BAT_NUM_OF_BATTERY_READINGS samples.
*
*  param    battery_id   The ID of the battery to be validated
*  param    test_type    Determines which validation to apply
*  param    type         Determines which type i.e. primary or backup
*
*  return   test_result  Result code of validation test
*
*  Note:    For Construction (SiteNet)
*****************************************************************************/
BatteryTestResult_t DM_BAT_ValidateBatteryConstruction(const BatteryID_t battery_id, const BatteryTestType_t test_type, const DM_BAT_Battery_t type)
{
   uint32_t battery_mv = 0U;
   uint32_t battery_low_count = 0U;
   BatteryTestResult_t test_result = DM_BAT_TEST_OK_E;
   if ( DM_BAT_VOLTAGE_TEST_E == test_type )
   {
      /* Establish the correct threshold for low battery */
      if ( 0U != PrimaryBatteryReferenceVoltage_mv )
      {
         /* Default state to 'battery OK' */
         Battery[battery_id].State = DM_BAT_TEST_OK_E;

         for ( uint8_t bat_reading = 0U; bat_reading < DM_BAT_NUM_OF_BATTERY_READINGS; bat_reading++ )
         {
            battery_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(PrimaryBatteryReferenceVoltage_mv, BatteryVoltageReadings[battery_id][bat_reading], ADC_RESOLUTION_12B);
            battery_mv *= CONSTRUCTION_HARDWARE_VOLTAGE_CORRECTION;

            if (DM_BAT_PRIMARY_E == type)
            {
               if ( CFG_GetBatPriLowVoltThreshC() > battery_mv )
               {
                  battery_low_count++;
               }
            }
            else if (DM_BAT_BACKUP_E == type)
            {
               if ( CFG_GetBatBkpLowVoltThreshC() > battery_mv )
               {
                  battery_low_count++;
               }
            }
            else
            {
               /* Do nothing here */
            }
            CO_PRINT_B_4(DBG_INFO_E,"Battery %d reading %d, voltage=%d, fail threshold=%d\r\n", battery_id, (bat_reading + 1U), battery_mv,
               (DM_BAT_PRIMARY_E == type)? CFG_GetBatPriLowVoltThreshC(): CFG_GetBatBkpLowVoltThreshC());
         }

         if ( DM_BAT_NUM_OF_BATTERY_READINGS <= battery_low_count )
         {
            /* Voltage test has seen three quick consecutive low readings */
            test_result = DM_BAT_VOLTAGE_LOW_CHECK_E;
         }

         Battery[battery_id].State = test_result;
      }
      else
      {
         test_result = DM_BAT_INVALID_TEST_E;
         CO_PRINT_A_0(DBG_ERROR_E,"Failed to read ADC reference\r\n");
      }
   }
   
   return test_result;
}

/*************************************************************************************/
/**
* function name   : DM_BAT_SendBatteryStatus
* description     : Send the battery level to the Application
*
* @param - sendOnDulch     TRUE if the message should go on the DULCH channel.
*
* @return - ErrorCode_t.  SUCCESS_E or fail code.
*/
ErrorCode_t DM_BAT_SendBatteryStatus(const bool sendOnDulch)
{
   ErrorCode_t status = ERR_NO_MEMORY_E;
   uint32_t primary_voltage_mv;
   uint32_t backup_voltage_mv;
   
   /* Send the voltage to the Application */
   CO_Message_t* pFaultMessage = osPoolAlloc(AppPool);
   if (pFaultMessage)
   {
      //Convert the raw ADC readings to mV.
      if ( CFG_GetBatteryCheckEnabled() )
      {
         primary_voltage_mv = DM_BAT_GetPrimaryBatteryVoltage();
         backup_voltage_mv = DM_BAT_GetBackupBatteryVoltage();
         CO_PRINT_B_2(DBG_INFO_E,"DM_BAT_SendBatteryStatus 1: Primary=%dmV, Backup=%dmV\r\n", primary_voltage_mv, backup_voltage_mv);
      }
      else
      {
         //Battery checks are disabled.  Send '0' or dummy readings.
#ifdef SEND_DUMMY_BATTERY_READINGS
         if ( BASE_CONSTRUCTION_E == gBaseType )
         {
            primary_voltage_mv = 9000U;
            backup_voltage_mv = 6000U;
         }
         else 
         {
            primary_voltage_mv = 3600U;
            backup_voltage_mv = 3000U;
         }
#else
         primary_voltage_mv = 0U;
         backup_voltage_mv = 0U;
         CO_PRINT_B_2(DBG_INFO_E,"DM_BAT_SendBatteryStatus 2: Primary=%dmV, Backup=%dmV\r\n", primary_voltage_mv, backup_voltage_mv);
#endif
      }

      //POLICY:  Always send a non-zero battery reading.
      //The control panel sees 0V as 'test not done' and doesn't update the display
      if ( 0U == primary_voltage_mv )
      {
         primary_voltage_mv = 1U;
      }
      if ( 0U == backup_voltage_mv )
      {
         backup_voltage_mv = 1U;
      }

      CO_PRINT_B_2(DBG_INFO_E,"DM_BAT_SendBatteryStatus 3: Primary=%dmV, Backup=%dmV\r\n", primary_voltage_mv, backup_voltage_mv);

		// if not got either - don't bother sending msg
		if ( primary_voltage_mv == 1U && backup_voltage_mv == 1U )
		{
			status = SUCCESS_E;
		}
		else
		{
			CO_PRINT_B_2(DBG_INFO_E,"Sending battery values Primary=%dmV, Backup=%dmV\r\n", primary_voltage_mv, backup_voltage_mv);

			//Send a message to the App.
			CO_BatteryStatusData_t batteryData;
			batteryData.PrimaryBatteryVoltage = primary_voltage_mv;
			batteryData.BackupBatteryVoltage = backup_voltage_mv;
			batteryData.DelaySending = sendOnDulch;
			pFaultMessage->Type = CO_MESSAGE_GENERATE_BATTERY_STATUS_E;
			memcpy(pFaultMessage->Payload.PhyDataReq.Data, &batteryData, sizeof(CO_BatteryStatusData_t));
			pFaultMessage->Payload.PhyDataReq.Size = sizeof(CO_FaultData_t);
			
			osStatus osStat = osMessagePut(AppQ, (uint32_t)pFaultMessage, 0U);
			
			if (osOK != osStat)
			{
				 /* failed to put message in the head queue */
				 osPoolFree(AppPool, pFaultMessage);
				 status = ERR_QUEUE_OVERFLOW_E;
			}
			else  
			{
				 status = SUCCESS_E;
			}
		}
   }
   
   return status;
}

/*************************************************************************************/
/**
* function name   : DM_BAT_GetBatteryStatus
* description     : Return the general battery status.
*
* @param - None.
*
* @return - BatteryTestResult_t
*/
BatteryTestResult_t DM_BAT_GetBatteryStatus( void )
{
   BatteryTestResult_t status = DM_BAT_TEST_OK_E;
   
#ifndef SE_BATTERY
      if ( (DM_BAT_ERROR_E == gPrimaryTestResult) ||
           (DM_BAT_ERROR_E == gBackupTestResult) )
      {
         status = DM_BAT_ERROR_E;
      }
      else if ( (DM_BAT_VOLTAGE_LOW_E == gPrimaryTestResult) ||
                (DM_BAT_VOLTAGE_LOW_E == gBackupTestResult) )
      {
         status = DM_BAT_VOLTAGE_LOW_E;
      }
      else if ( (DM_BAT_TEST_OK_E == gPrimaryTestResult) &&
                (DM_BAT_TEST_OK_E == gBackupTestResult) )
      {
         status = DM_BAT_TEST_OK_E;
      }
      else 
      {
         /* Do nothing here */
      }
#else
   if ( SE_BATTERY_SHUTDOWN_VOLTAGE_CONSTRUCTION_PRIMARY > PrimaryBatteryVoltage )
   {
      status = DM_BAT_ERROR_E;
   }
#endif
   return status;
}

/*************************************************************************************/
/**
* function name   : DM_BAT_SetBatteryEnable
* description     : Switch the specified battery to on or off.
*
* @param - battery      The battery to switch on/off (DM_BAT_PRIMARY_E or DM_BAT_BACKUP_E)
* @param - enable       The on/off state to apply
*
* @return - None.
*/
void DM_BAT_SetBatteryEnable(const DM_BAT_Battery_t battery, const DM_BAT_BatteryEnable_t enable)
{
   //Range check the parameters
   if ( (DM_BAT_TYPE_MAX_E > battery) && (DM_BAT_BATTERY_STATE_MAX_E > enable) )
   {
      //Apply the settings.  Note that the pin-out is different for SiteNet and SmartNet configurations.
      
      if ( DM_BAT_PRIMARY_E == battery )
      {
         GpioWrite(&PriBatteryEn, enable);
         CO_PRINT_B_1(DBG_INFO_E,"Primary battery set to %s\r\n", DM_BAT_BATTERY_ON_E == enable ? "ON":"OFF");
      }
      else 
      {
#ifdef USE_OLD_CONSTRUCTION_BKP_BATTERY_PIN_LOGIC
         //Older construction boards use reverse polarity for backup battery on/off
         if ( enable )
         {
            GpioWrite(&BkpBatEn, 0);
         }
         else 
         {
            GpioWrite(&BkpBatEn, 1);
         }
         CO_PRINT_B_1(DBG_INFO_E,"Backup battery monitor set to %s\r\n", DM_BAT_BATTERY_MONITOR_ON_E == enable ? "OFF":"ON");
#else
         GpioWrite(&BkpBatEn, enable);
         CO_PRINT_B_1(DBG_INFO_E,"Backup battery set to %s\r\n", DM_BAT_BATTERY_ON_E == enable ? "ON":"OFF");
#endif
      }
   }

}


/*************************************************************************************/
/**
* function name   : DM_BAT_SetBatteryMonitor
* description     : Switch the specified battery monitor to on or off.
*
* @param - battery      The battery monitor to switch on/off (DM_BAT_PRIMARY_E or DM_BAT_BACKUP_E)
* @param - enable       The on/off state to apply
*
* @return - None.
*/
void DM_BAT_SetBatteryMonitor(const DM_BAT_Battery_t battery, const DM_BAT_BatteryMonitor_t enable)
{
   //Range check the parameters
   if ( (DM_BAT_TYPE_MAX_E > battery) && (DM_BAT_BATTERY_MONITOR_STATE_MAX_E > enable) )
   {
      //Apply the settings.  Note that these are the same for SiteNet and SmartNet configurations.
      if ( BASE_CONSTRUCTION_E == gBaseType )
      {
         if ( DM_BAT_PRIMARY_E == battery )
         {
            GpioWrite(&VbatMonEn, enable);
            CO_PRINT_B_1(DBG_INFO_E,"Primary battery monitor set to %s\r\n", DM_BAT_BATTERY_MONITOR_ON_E == enable ? "ON":"OFF");
         }
         else 
         {
            GpioWrite(&VBkpBatMonEn, enable);
            CO_PRINT_B_1(DBG_INFO_E,"Backup battery monitor set to %s\r\n", DM_BAT_BATTERY_MONITOR_ON_E == enable ? "ON":"OFF");
         }
      }
      else 
      {
         if ( DM_BAT_BACKUP_E == battery )
         {
            GpioWrite(&BatMonEn, enable);
            CO_PRINT_B_1(DBG_INFO_E,"Battery monitor set to %s\r\n", DM_BAT_BATTERY_MONITOR_ON_E == enable ? "ON":"OFF");
         }
      }
   }

}


/*************************************************************************************/
/**
* function name   : DM_BAT_SetTestMode
* description     : Put the battery monitor into test mode
*
* @param - mode      the test mode to apply
*
* @return - None.
*/
void DM_BAT_SetTestMode(const DM_BatteryTestMode_t mode)
{
   if ( BASE_CONSTRUCTION_E == gBaseType )
   {
      switch ( mode )
      {
         case DM_BAT_TEST_MODE_OFF_E:
            //Ensure primary batteries ON
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
            //Set backup battery ON
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
            //Switch off the battery load circuit
            DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);
            //Switch off the backup battery load circuit
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
            gBatteryTestMode = DM_BAT_TEST_MODE_OFF_E;
            break;
         case DM_BAT_PRIMARY_STATIC_TEST_E:
            //set primary battery ON
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
            //Ensure backup battery is ON
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
            //Switch on the primary battery load circuit
            DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_ON_E);
            gBatteryTestMode = DM_BAT_PRIMARY_STATIC_TEST_E;
            break;
         case DM_BAT_BACKUP_STATIC_TEST_E:
            //set primary battery ON
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
            //Ensure backup battery is OFF
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
            //Switch on the backup battery load circuit
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
            gBatteryTestMode = DM_BAT_BACKUP_STATIC_TEST_E;
            break;
         default:
            //Ensure primary batteries ON
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
            //Set backup battery ON
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
            //Switch off the battery load circuit
            DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);
            //Switch off the backup battery load circuit
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
            gBatteryTestMode = DM_BAT_TEST_MODE_OFF_E;
            break;
      }
   }
   else 
   {
      switch ( mode )
      {
         case DM_BAT_TEST_MODE_OFF_E:
            //Ensure primary batteries ON
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
            //Set backup battery ON
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
            //Switch off the battery load circuit
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);

            gBatteryTestMode = DM_BAT_TEST_MODE_OFF_E;
            break;
         case DM_BAT_PRIMARY_STATIC_TEST_E:
            //Ensure primary batteries ON
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
            //Set backup battery OFF
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_OFF_E);
            //Switch on the battery load circuit
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
            gBatteryTestMode = DM_BAT_PRIMARY_STATIC_TEST_E;
            break;
         case DM_BAT_BACKUP_STATIC_TEST_E:
            //Set backup battery ON
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
            //Ensure primary batteries OFF
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
            //Switch on the battery load circuit
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
            gBatteryTestMode = DM_BAT_BACKUP_STATIC_TEST_E;
            break;
         case DM_BAT_PRIMARY_CURRENT_TEST_E:
            //Ensure primary batteries ON
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
            //Set backup battery OFF
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_OFF_E);
            //Switch on the battery load circuit
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
            gBatteryTestMode = DM_BAT_PRIMARY_CURRENT_TEST_E;
            break;
         default:
            //Ensure primary batteries ON
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
            //Set backup battery ON
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
            //Switch on the battery load circuit
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
            gBatteryTestMode = DM_BAT_TEST_MODE_OFF_E;
            break;
      }
   }
}

/*************************************************************************************/
/**
* function name   : DM_BAT_GetTestMode
* description     : Read the current test mode of the battery monitor
*
* @param - mode      the test mode to apply
*
* @return - DM_BatteryTestMode_t    The current test mode
*/
DM_BatteryTestMode_t DM_BAT_GetTestMode(void)
{
   return gBatteryTestMode;
}


#ifdef ENABLE_CONTINUOUS_LOAD_BATTERY_CURRENT_TEST
/*************************************************************************************/
/**
* function name   : DM_BAT_RunSmartNetBatteryCurrenttest
* description     : Report the primary battery current readings over the debug port
*                   at regular intervals, with the battery load permanently on.
*
* @param - None.
*
* @return - None..
*/
void DM_BAT_RunSmartNetBatteryCurrenttest(void)
{
   //Ensure primary batteries ON
   DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
   //Set backup battery OFF
   DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_OFF_E);
   //Switch on the battery load circuit
   DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
            
   //First read the reference voltage
   DM_ADC_ReadReferenceVoltage(&PrimaryBatteryReferenceVoltage_mv);
   
   //Read the battery voltage
   ErrorCode_t result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E);
   
   if ( SUCCESS_E != result )
   {
      CO_PRINT_B_1(DBG_ERROR_E,"Primary Battery - failed to read battery voltage - error=%d\r\n", result);
   }
            
   //Read current for Battery 1
   result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_1_E);
   if ( SUCCESS_E != result )
   {
      CO_PRINT_B_1(DBG_INFO_E,"Battery 1 - failed to read battery current - error=%d\r\n", result);
   }
      
   //Read current for Battery 2
   result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_2_E);
   if ( SUCCESS_E != result )
   {
      CO_PRINT_B_1(DBG_INFO_E,"Battery 2 - failed to read battery current - error=%d\r\n", result);
   }
         
   //Read current for Battery 3
   result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_3_E);
   if ( SUCCESS_E != result )
   {
      CO_PRINT_B_1(DBG_INFO_E,"Battery 3 - failed to read battery current - error=%d\r\n", result);
   }
   
   uint32_t primary_voltage_mv = __HAL_ADC_CALC_DATA_TO_VOLTAGE(PrimaryBatteryReferenceVoltage_mv, Battery[DM_BAT_BATTERY_VBAT_E].Value, ADC_RESOLUTION_12B);
   uint32_t cell_1_voltage_mv = ((Battery[DM_BAT_BATTERY_1_E].Value * PrimaryBatteryReferenceVoltage_mv)/THRESHOLD_PRODUCT) / 42;//gain of current sense is 42.3
   uint32_t cell_2_voltage_mv = ((Battery[DM_BAT_BATTERY_2_E].Value * PrimaryBatteryReferenceVoltage_mv)/THRESHOLD_PRODUCT) / 42;
   uint32_t cell_3_voltage_mv = ((Battery[DM_BAT_BATTERY_3_E].Value * PrimaryBatteryReferenceVoltage_mv)/THRESHOLD_PRODUCT) / 42;
   uint32_t cell_1_current_ma = (100 * cell_1_voltage_mv) / 22;
   uint32_t cell_2_current_ma = (100 * cell_2_voltage_mv) / 22;
   uint32_t cell_3_current_ma = (100 * cell_3_voltage_mv) / 22;
   
   CO_PRINT_B_0(DBG_INFO_E,"\r\n");
   CO_PRINT_B_1(DBG_INFO_E,"MCU voltage reference = %dmV\r\n", PrimaryBatteryReferenceVoltage_mv);
   
   CO_PRINT_B_2(DBG_INFO_E,"Primary Battery voltage = %dmV (adc=%d)\r\n", primary_voltage_mv, Battery[DM_BAT_BATTERY_VBAT_E].Value);
   CO_PRINT_B_2(DBG_INFO_E,"Battery 1 current sense = %dmA (adc=%d)\r\n", cell_1_current_ma, Battery[DM_BAT_BATTERY_1_E].Value);
   CO_PRINT_B_2(DBG_INFO_E,"Battery 2 current sense = %dmA (adc=%d)\r\n", cell_2_current_ma, Battery[DM_BAT_BATTERY_2_E].Value);
   CO_PRINT_B_2(DBG_INFO_E,"Battery 3 current sense = %dmA (adc=%d)\r\n", cell_3_current_ma, Battery[DM_BAT_BATTERY_3_E].Value);
      
   // leave the current load on
}
#endif

#ifdef ENABLE_BATTERY_VOLTAGE_COMMAND
/*************************************************************************************/
/**
* function name   : DM_BAT_ReadBatteryVoltages
* description     : Report the battery readings over the debug port.
*
* @param - None.
*
* @return - None..
*/
ErrorCode_t DM_BAT_ReadBatteryVoltages(DM_BatteryReadings_t* pBatReadings)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   uint32_t BatteryReferenceVoltage_mv = 0U;
   if ( pBatReadings )
   {
      //Make sure that no battery checks are in progress
      if ( DM_BAT_IDLE_E == BatMonitorState )
      {
         //Read ADC reference
         DM_ADC_ReadReferenceVoltage(&BatteryReferenceVoltage_mv);
         pBatReadings->AdcReference = BatteryReferenceVoltage_mv;
            
         if ( BASE_CONSTRUCTION_E == gBaseType )
         {
            if ( Battery[DM_BAT_BATTERY_VBAT_E].BatteryDetected )
            {
               //Construction
               //Primary battery
               //set primary batteries ON
               DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
               //Switch on the battery load circuit
               DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_ON_E);
               //Ensure backup battery is OFF
               DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_OFF_E);
               //Ensure the backup battery load circuit is off
               DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
               
               result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E);
               if ( SUCCESS_E == result )
               {
                  pBatReadings->PrimaryRawADC = Battery[DM_BAT_BATTERY_VBAT_E].Value;
                  pBatReadings->PrimaryVoltage = DM_BAT_GetPrimaryBatteryVoltage();
               }
               else 
               {
                  CO_PRINT_B_0(DBG_ERROR_E,"Failed to read primary battery ADC\r\n");
               }
               //Switch off the battery load circuit
               DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);
            }

            if ( Battery[DM_BAT_BATTERY_1_E].BatteryDetected )
            {
               //Backup Batteries
               
               //Backup battery is ON
               DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
               //wait
               osDelay(20);
               //set primary batteries OFF
               DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
               //Switch on the backup battery load circuit
               DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
               //wait
               osDelay(100);
               //Read cell 1
               result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_1_E);
               if ( SUCCESS_E == result )
               {
                  pBatReadings->BackupCell1RawADC = Battery[DM_BAT_BATTERY_1_E].Value;
                  pBatReadings->BackupCell1Voltage = DM_BAT_GetBackupBatteryVoltage();
                  //read total bkp battery
                  result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_3_E);
                  if ( SUCCESS_E == result )
                  {
                     pBatReadings->BackupCell3RawADC = Battery[DM_BAT_BATTERY_3_E].Value;
                     pBatReadings->BackupRawADC = Battery[DM_BAT_BATTERY_3_E].Value;
                     pBatReadings->BackupCell3Voltage = __HAL_ADC_CALC_DATA_TO_VOLTAGE(BatteryReferenceVoltage_mv, Battery[DM_BAT_BATTERY_3_E].Value, ADC_RESOLUTION_12B);
                     pBatReadings->BackupCell3Voltage *= CONSTRUCTION_HARDWARE_VOLTAGE_CORRECTION;
                     pBatReadings->BackupVoltage = pBatReadings->BackupCell3Voltage;
                     //calc cell 2
                     pBatReadings->BackupCell2RawADC = pBatReadings->BackupCell3RawADC - pBatReadings->BackupCell1RawADC;
                     pBatReadings->BackupCell2Voltage = __HAL_ADC_CALC_DATA_TO_VOLTAGE(BatteryReferenceVoltage_mv, pBatReadings->BackupCell2RawADC, ADC_RESOLUTION_12B);
                     pBatReadings->BackupCell2Voltage *= CONSTRUCTION_HARDWARE_VOLTAGE_CORRECTION;
                  }
                  else 
                  {
                     CO_PRINT_B_0(DBG_ERROR_E,"Failed to read back-up ADC\r\n");
                  }
               }
               else 
               {
                  CO_PRINT_B_0(DBG_ERROR_E,"Failed to read bkp cell1 ADC\r\n");
               }
            }
            //Restore the battery settings
            //set backup battery ON
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
            //set primary battery ON
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
            //Switch off the battery load circuit
            DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);
            //Switch off the backup battery load circuit
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
         }
         else //base type RBU
         {
            if ( Battery[DM_BAT_BATTERY_VBAT_E].BatteryDetected )
            {
               //Ensure primary batteries ON
               DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
               //Set backup battery OFF
               DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_OFF_E);

               //Switch on the battery load circuit
               DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
               
               osDelay(20);

               //Take a Vbat reading
               result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E);
               //Switch off the primary battery load circuit
               DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);
               if ( SUCCESS_E == result )
               {
                  PrimaryBatteryVoltage = Battery[DM_BAT_BATTERY_VBAT_E].Value;
                  pBatReadings->PrimaryRawADC = PrimaryBatteryVoltage;
                  pBatReadings->PrimaryVoltage = DM_BAT_GetPrimaryBatteryVoltage();
               }
            }
            else 
            {
               pBatReadings->PrimaryRawADC = 0;
               pBatReadings->PrimaryVoltage = 0;
               result = SUCCESS_E;
            }
            
            if ( Battery[DM_BAT_BATTERY_1_E].BatteryDetected ) 
            {
               //Set backup battery ON
               DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
               //Ensure primary batteries OFF
               DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_OFF_E);
               //Switch on the battery load circuit
               DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
               
               osDelay(20);

               //Take a Vbat reading
               result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E);
               //Switch off the primary battery load circuit
               DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);
               if ( SUCCESS_E == result )
               {
                  BackupBatteryVoltage = Battery[DM_BAT_BATTERY_VBAT_E].Value;
                  pBatReadings->BackupRawADC = BackupBatteryVoltage;
                  pBatReadings->BackupVoltage = DM_BAT_GetBackupBatteryVoltage();
               }
            }
            else 
            {
               pBatReadings->BackupRawADC = 0;
               pBatReadings->BackupVoltage = 0;
               result = SUCCESS_E;
            }
            //Ensure primary batteries ON
            DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
            //Set backup battery ON
            DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
            //Switch of the battery load circuit
            DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
         }
      }
   }
   return result;
}
#endif



#ifdef SE_BATTERY
/*************************************************************************************/
/**
* function name   : DM_BAT_EnterSleepmode
* description     : Put the device into sleep mode if the batteries are too low.
*                   For LiPo rechargeable (SE_BATTERY) to prevent over-discharge.
*
* @param - None.
*
* @return - None..
*/
void DM_BAT_EnterSleepmode(void)
{
   osStatus osStat;
   CO_CommandData_t CmdMessage;
   
   uint32_t primary_mv = DM_BAT_GetPrimaryBatteryVoltage();
   uint32_t bkp_mv = DM_BAT_GetBackupBatteryVoltage();
   CO_PRINT_A_2(DBG_INFO_E,"BATTERY LOW.  Entering sleep mode.  Prim=%d, Bkp=%d\r\n", primary_mv, bkp_mv);
   CO_PRINT_PORT_2(PPU_UART_E,"BATTERY LOW.  Entering sleep mode.  Prim=%d, Bkp=%d\r\n", primary_mv, bkp_mv);
   osDelay(1000);
   
   CmdMessage.TransactionID = 0;
   CmdMessage.Source = 0;
   CmdMessage.Destination = gNetworkAddress;
   CmdMessage.Parameter1 = 0;
   CmdMessage.Parameter2 = 0;
   CmdMessage.CommandType = PARAM_TYPE_TEST_MODE_E;
   CmdMessage.ReadWrite = 1;
   CmdMessage.Value = MC_MAC_TEST_MODE_SLEEP_E;
   
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
   }
}

/*************************************************************************************/
/**
* function name   : DM_BAT_RunSEBatteryTest
* description     : Do a simple voltage test on the batteries.
*                   For LiPo rechargeable (SE_BATTERY).
*
* @param - None.
*
* @return - None..
*/
void DM_BAT_RunSEBatteryTest(void)
{
   static int32_t interval_count = 0U;
   uint32_t shut_down_voltage = 0U;
   
   
   switch( BatMonitorState )
   {
      case DM_BAT_STARTUP_E:
      {
         IdleCount++;
         //Wait for the start-up delay then trigger a battery test
         if ( DM_BAT_INITIAL_TEST_DELAY <= IdleCount )
         {
            IdleCount=0U;
            interval_count = BATTERY_TEST_INTERVAL;
            BatMonitorState = DM_BAT_PRETEST_E;
         }
         break;
      }
      
      case DM_BAT_PRETEST_E:
         BatMonitorState = DM_BAT_IDLE_E;
         break;
      
      case DM_BAT_IDLE_E:
      {
         if ( BatteryCheckRequested )
         {
            interval_count = BATTERY_TEST_INTERVAL;
            BatteryCheckRequested = false;
            CO_PRINT_B_0(DBG_INFO_E,"Running battery test\r\n");
         }
         
         //Test at the required interval
         interval_count++;
         if ( BATTERY_TEST_INTERVAL < interval_count )
         {
            interval_count=0U;
            
            //Turn on the battery monitor.  No load.
            if ( BASE_CONSTRUCTION_E == gBaseType )
            {
               //set primary battery ON
               DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
               //Switch on the primary battery load circuit
               DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_ON_E);
            }
            else 
            {
               //Ensure primary batteries ON
               DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
               //Set backup battery OFF
               DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_OFF_E);
               //Switch on the battery load circuit
               DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_ON_E);
            }
            
            osDelay(20);
            
            //Read the ADC reference voltage
            DM_ADC_ReadReferenceVoltage(&PrimaryBatteryReferenceVoltage_mv);
            BackupBatteryReferenceVoltage_mv = PrimaryBatteryReferenceVoltage_mv;

            //Take a Vbat reading
            ErrorCode_t result = DM_BAT_ReadBatteryValue(DM_BAT_BATTERY_VBAT_E);
            
            if ( SUCCESS_E == result )
            {
               PrimaryBatteryVoltage = Battery[DM_BAT_BATTERY_VBAT_E].Value;
               BackupBatteryVoltage = 1U;
               //Get PrimaryBatteryVoltage in mV
               uint32_t primary_mv = DM_BAT_GetPrimaryBatteryVoltage();
               CO_PRINT_B_3(DBG_INFO_E,"ADC Ref=%d  pbat_raw=%d Primary Battery=%dmV\r\n",PrimaryBatteryReferenceVoltage_mv, PrimaryBatteryVoltage, primary_mv);
               if ( BASE_CONSTRUCTION_E == gBaseType )
               {
                  //If no 9V battery is fitted, VBAT will be the backup batteries so adjust the expected voltage.
                  if ( Battery[DM_BAT_BATTERY_VBAT_E].BatteryDetected )
                  {
                     shut_down_voltage = SE_BATTERY_SHUTDOWN_VOLTAGE_CONSTRUCTION_PRIMARY;
                  }
                  else 
                  {
                     shut_down_voltage = SE_BATTERY_SHUTDOWN_VOLTAGE_CONSTRUCTION_BACKUP;
                  }
               }
               else
               {
                  //RBU base type.  Set voltage threshold for single battery.
                  shut_down_voltage = SE_BATTERY_SHUTDOWN_VOLTAGE;
               }
               
               if ( shut_down_voltage > primary_mv )
               {
                  //Turn off the battery monitor.  No load.
                  if ( BASE_CONSTRUCTION_E == gBaseType )
                  {
                     //set primary battery ON
                     DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
                     //Switch on the primary battery load circuit
                     DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);
                  }
                  else
                  {
                     //Set backup battery ON
                     DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
                     //Switch off the battery load circuit
                     DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
                  }
                  CO_PRINT_B_0(DBG_INFO_E,"Entering sleep mode to protect battery\r\n");
                  DM_BAT_EnterSleepmode();
               }
               else 
               {
                  CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_BATTERY_FAULT_E, BatteryFault, false, false);
                  CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_LOW_BATTERY_E, BatteryWarning, false, false);
                  gBatteryResultsUpdated = true;
               }
            }
            else
            {
               CO_PRINT_B_1(DBG_ERROR_E,"Failed to read battery value.  Error=%d\r\n",result);
            }
            
            //Turn off the battery monitor.  No load.
            if ( BASE_CONSTRUCTION_E == gBaseType )
            {
               //set primary battery ON
               DM_BAT_SetBatteryEnable(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_ON_E);
               //Switch on the primary battery load circuit
               DM_BAT_SetBatteryMonitor(DM_BAT_PRIMARY_E, DM_BAT_BATTERY_MONITOR_OFF_E);
            }
            else
            {
               //Set backup battery ON
               DM_BAT_SetBatteryEnable(DM_BAT_BACKUP_E, DM_BAT_BATTERY_ON_E);
               //Switch ooff the battery load circuit
               DM_BAT_SetBatteryMonitor(DM_BAT_BACKUP_E, DM_BAT_BATTERY_MONITOR_OFF_E);
            }
            
            BatMonitorState = DM_BAT_TEST_COMPLETE_E;
         }
         else if ( gBatteryResultsUpdated )
         {
            //The battery check has been done and we have returned to IDLE.  Send new battery values.
            CO_PRINT_B_0(DBG_INFO_E,"Send Battery 6\r\n");
            DM_BAT_SendBatteryStatus(false);
            gBatteryResultsUpdated = false;
         }

         
      break;
      }
      
      case DM_BAT_TEST_COMPLETE_E:
         BatMonitorState = DM_BAT_IDLE_E;
         break;
      
      default:
         BatMonitorState = DM_BAT_STARTUP_E;
         break;
   }
}
#endif
