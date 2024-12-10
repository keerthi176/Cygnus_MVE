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
*  File         : CFG_Device_cfg.h
*
*  Description  : Header for the device configuration functions
*
*************************************************************************************/

#ifndef CFG_DEVICE_CONFIG_H
#define CFG_DEVICE_CONFIG_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/* User Include Files
*************************************************************************************/
#include "CO_Message.h"
#include "CO_ErrorCode.h"


/* Public Structures
*************************************************************************************/ 
typedef struct
{
   uint32_t LowerWord;
   uint32_t MiddleWord;
   uint32_t UpperWord;
} ZoneDisabledMap_t;


/* Public Enumerations
*************************************************************************************/
typedef enum
{
   CFG_AVE_DEFAULT_E,
   CFG_AVE_RESTART_E,
   CFG_AVE_DISABLE_E,
   CFG_AVE_MAX_E
} SnrRssiAveragingPolicy_t;


/* Public Constants
*************************************************************************************/
#define MAX_DULCH_WRAP                1024U
#define DEFAULT_PASSIVATION_SETTINGS 0x07CB4C1C  //v=3100, Tout=180, Ton=31, Toff=0
/* Macros
*************************************************************************************/


/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t CFG_InitialiseDeviceConfig(void);
ErrorCode_t CFG_SetZoneNumber(const uint32_t zone);
uint32_t CFG_GetZoneNumber(void);
void CFG_SetZoneEnabled(const bool enabled);
bool CFG_ZoneEnabled(void);
void CFG_UpdateZoneMap(const uint8_t zone, const bool enabled);
void CFG_SetDayNight(const CO_DayNight_t DayNight);
ErrorCode_t CFG_SetGlobalDelayEnabled(const bool enabled);
bool CFG_GetGlobalDelayEnabled(void);
ErrorCode_t CFG_SetSoundLevel(const uint8_t soundLevel);
uint8_t CFG_GetSoundLevel(void);
CO_DayNight_t CFG_GetDayNight(void);
bool CFG_GetZoneDisabledMap(ZoneDisabledMap_t* pZoneMap);
void CFG_SetSplitZoneDisabledMap(const bool upperHalf, const uint32_t zoneWord, const uint16_t zoneHalfWord);
bool CFG_GetSplitZoneDisabledMap(const bool upperHalf, uint32_t* pZoneWord, uint16_t* pZoneHalfWord);
bool CFG_RequestedZoneEnabled(const uint8_t zone);
void CFG_SetMaxNumberOfChildren(const uint32_t max_children);
uint32_t CFG_GetMaxNumberOfChildren(void);
void CFG_SetFaultReportsEnabled(const bool enabled);
bool CFG_GetFaultReportsEnabled(void);
uint32_t CFG_GetDeviceCombination(void);
uint32_t CFG_GetDulchWrap(void);
void CFG_SetGlobalDelay1(const uint16_t globalDelay1);
uint16_t CFG_GetGlobalDelay1(void);
void CFG_SetGlobalDelay2(const uint16_t globalDelay2);
uint16_t CFG_GetGlobalDelay2(void);
uint32_t CFG_GetLocalOrGlobalDelayBitmap(void);
bool CFG_SetMaxHops(const uint32_t maxHops);
uint32_t CFG_GetMaxHops(void);
void CFG_SetBatteryCheckEnabled(const bool enabled);
bool CFG_GetBatteryCheckEnabled(void);
void CFG_SetGlobalDelayOverride(const bool enable);
bool CFG_GetGlobalDelayOverride(void);
bool CFG_SetRssiMarConfiguration(const uint32_t config);
bool CFG_SetSnrJoiningThreshold(const int32_t config);
int32_t CFG_GetSnrJoiningThreshold(void);
int32_t CFG_GetRssiMarSensitivity(const bool two_parents);
void CFG_SetSecurityInhibitDay(const bool inhibit);
bool CFG_GetSecurityInhibitDay(void);
void CFG_SetSecurityInhibitNight(const bool inhibit);
bool CFG_GetSecurityInhibitNight(void);
bool CFG_SetSnrRssiAveragingPolicy(const uint32_t policy);
SnrRssiAveragingPolicy_t CFG_GetSnrRssiAveragingPolicy(void);
uint16_t CFG_GetPreformedPrimaryParent(void);
uint16_t CFG_GetPreformedSecondaryParent(void);
bool CFG_UsingPreformedMesh(void);
bool CFG_SetBatteryTestInterval(const uint32_t interval_days);
uint32_t CFG_GetBatteryTestInterval(void);
bool CFG_SetInitialListenPeriod(const uint32_t period);
uint32_t CFG_GetInitialListenPeriod(void);
bool CFG_SetPhase2Period(const uint32_t period);
uint32_t CFG_GetPhase2Period(void);
bool CFG_SetPhase2SleepPeriod(const uint32_t period);
uint32_t CFG_GetPhase2SleepPeriod(void);
bool CFG_SetPhase3SleepPeriod(const uint32_t period);
uint32_t CFG_GetPhase3SleepPeriod(void);
void CFG_SetSviPowerLock(const bool lock);
bool CFG_GetSviPowerLock(void);
void CFG_SetPollingDisabled(const bool disabled);
bool CFG_GetPollingDisabled(void);
bool CFG_HasNonLatchingMedicalCallPoint(void);
void CFG_SetBlankingPlateFitted(const bool fitted);
bool CFG_GetBlankingPlateFitted(void);
void CFG_SetNumberOfResends(const uint32_t num_resends);
uint32_t CFG_GetNumberOfResends(void);
void CFG_SetPPUMode(const bool ppuMode);
bool CFG_IsPPUMode(void);
ErrorCode_t CFG_SetZoneNumber(const uint32_t zone);
uint32_t CFG_GetZoneNumber(void);
bool CFG_IsSyncMaster(void);
uint32_t CFG_GetFirmwareVersion(void);
void CFG_SetPpuMode(const uint32_t ppuMode);
uint32_t CFG_GetPpuMode(void);
ErrorCode_t CFG_SetBatteryPassivationValues(const DePassivationSettings_t* const pDepassivationSettings);
ErrorCode_t CFG_GetBatteryPassivationValues(DePassivationSettings_t* pDepassivationSettings);
ErrorCode_t CFG_SetBatteryPassivationValuesCompound(const uint32_t depassivationSettings);
ErrorCode_t CFG_GetBatteryPassivationValuesCompound(uint32_t* pDepassivationSettings);
ErrorCode_t CFG_CompressDepassivationSettings(const DePassivationSettings_t* const pDepassivationSettings, uint32_t* pCompressedDepassivationSettings);
ErrorCode_t CFG_DecompressDepassivationSettings(const uint32_t depassivation_settings, DePassivationSettings_t* pDepassivationSettings);
ErrorCode_t CFG_SetFaultsClearEnable(const uint32_t enable);
uint32_t CFG_GetFaultsClearEnable(void);
uint32_t CFG_GetBatPriLowVoltThresh(void);
uint32_t CFG_GetBatBkpLowVoltThresh(void);
uint32_t CFG_GetBatPriLowVoltThreshC(void);
uint32_t CFG_GetBatBkpLowVoltThreshC(void);
bool CFG_SetBatPriLowVoltThresh(const uint32_t voltage_threshold);
bool CFG_SetBatBkpLowVoltThresh(const uint32_t voltage_threshold);
bool CFG_SetBatPriLowVoltThreshC(const uint32_t voltage_threshold);
bool CFG_SetBatBkpLowVoltThreshC(const uint32_t voltage_threshold);
uint32_t encode_version_string(const char* pVersion);
#ifdef APPLY_RSSI_OFFSET
void CFG_SetRssiOffset(const uint16_t node_id, const int8_t offset);
int8_t CFG_GetRssiOffset(const uint16_t node_id);
#endif

#endif // CFG_DEVICE_CONFIG_H
