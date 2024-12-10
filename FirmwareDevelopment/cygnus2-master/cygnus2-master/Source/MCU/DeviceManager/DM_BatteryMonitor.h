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
*  File         : DM_BatteryMonitor.h
*
*  Description  : Header for the battery management code
*
*************************************************************************************/

#ifndef DM_BATTERYMONITOR_H
#define DM_BATTERYMONITOR_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "stm32l4xx.h"
#include "CO_ErrorCode.h"
#include "CO_Defines.h"

/* User Include Files
*************************************************************************************/



/* Public Enumerations
*************************************************************************************/
typedef enum 
{
   DM_BAT_TEST_MODE_OFF_E,
   DM_BAT_PRIMARY_STATIC_TEST_E,
   DM_BAT_BACKUP_STATIC_TEST_E,
   DM_BAT_PRIMARY_CURRENT_TEST_E,
   DM_BAT_TEST_MODE_MAX_E
} DM_BatteryTestMode_t;

typedef enum 
{
   DM_BAT_BATTERY_1_E,
   DM_BAT_BATTERY_2_E,
   DM_BAT_BATTERY_3_E,
   DM_BAT_BATTERY_VBAT_E,
   DM_BAT_BATTERY_MAX_E
} BatteryID_t;

typedef enum 
{
   DM_BAT_NORMAL_RESTART_E,
   DM_BAT_PRIMARY_RESTART_E,
   DM_BAT_BACKUP_RESTART_E
} DM_BAT_RestartFlags_t;

typedef enum 
{
   DM_BAT_PRIMARY_E,
   DM_BAT_BACKUP_E,
   DM_BAT_TYPE_MAX_E
} DM_BAT_Battery_t;

typedef enum 
{
   DM_BAT_BATTERY_ON_E,
   DM_BAT_BATTERY_OFF_E,
   DM_BAT_BATTERY_STATE_MAX_E
} DM_BAT_BatteryEnable_t;

typedef enum 
{
   DM_BAT_BATTERY_MONITOR_OFF_E,
   DM_BAT_BATTERY_MONITOR_ON_E,
   DM_BAT_BATTERY_MONITOR_STATE_MAX_E
} DM_BAT_BatteryMonitor_t;

/* Public Structures
*************************************************************************************/ 
typedef struct 
{
   uint32_t AdcReference;
   uint32_t PrimaryVoltage;
   uint32_t BackupVoltage;
   uint32_t BackupCell1Voltage;
   uint32_t BackupCell2Voltage;
   uint32_t BackupCell3Voltage;
   uint32_t PrimaryRawADC;
   uint32_t BackupRawADC;
   uint32_t BackupCell1RawADC;
   uint32_t BackupCell2RawADC;
   uint32_t BackupCell3RawADC;
} DM_BatteryReadings_t;

typedef enum 
{
   DM_BAT_TEST_OK_E,
   DM_BAT_VOLTAGE_LOW_CHECK_E,
   DM_BAT_VOLTAGE_LOW_E,
   DM_BAT_ERROR_E,
   DM_BAT_INVALID_TEST_E
} BatteryTestResult_t;

/* Public Functions Prototypes
*************************************************************************************/
void DM_BAT_Initialise(const DM_BaseType_t baseType);
void DM_BAT_Run(void);
void DM_BAT_Cancel(void);
void DM_BAT_Inhibit(const bool inhibit);
ErrorCode_t DM_BAT_ReadBatteryValue(const BatteryID_t battery);
void DM_BAT_RequestBatteryCheck(const bool delayed);
bool DM_BAT_BatteryDepleted(void);
void DM_BAT_ConfigureForDeepSleep(void);
ErrorCode_t DM_BAT_SendBatteryStatus(const bool sendOnDulch);
void DM_BAT_SetTestMode(const DM_BatteryTestMode_t mode);
DM_BatteryTestMode_t DM_BAT_GetTestMode(void);
bool DM_BAT_SetBatteryRestartFlag(const DM_BAT_RestartFlags_t flags);
DM_BAT_RestartFlags_t DM_BAT_GetBatteryRestartFlag(void);
uint32_t DM_BAT_GetPrimaryBatteryVoltage(void);
uint32_t DM_BAT_GetBackupBatteryVoltage(void);
BatteryTestResult_t DM_BAT_GetBatteryStatus(void);
#ifdef ENABLE_BATTERY_VOLTAGE_COMMAND
ErrorCode_t DM_BAT_ReadBatteryVoltages(DM_BatteryReadings_t* pBatReadings);
#endif

/* Public Constants
*************************************************************************************/
#define DM_BAT_MCP_FAIL_THRESHOLD_MV                      (2000U)
#define DM_BAT_CONSTRUCTION_PRIMARY_FAIL_THRESHOLD_MV     (6500U)
#define DM_BAT_CONSTRUCTION_BACKUP_FAIL_THRESHOLD_CELL_MV (2000U)
#define DM_BAT_CONSTRUCTION_BACKUP_FAIL_THRESHOLD_MV      (4000U)

/* Macros
*************************************************************************************/


#endif // DM_ADC_H
