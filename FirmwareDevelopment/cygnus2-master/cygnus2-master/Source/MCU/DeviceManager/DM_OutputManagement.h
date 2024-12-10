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
*  File         : DM_OutputManagement.h
*
*  Description  : Header for the management of the output silenceable/unsilenceable
*                 bit masks and output control in accordance with an Alarm Output
*                 State signal
*
*************************************************************************************/

#ifndef DM_OUTPUTMANAGEMENT_H
#define DM_OUTPUTMANAGEMENT_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "stm32l4xx.h"
#include "CO_ErrorCode.h"
#include "CO_Message.h"
#include "DM_Device.h"

/* User Include Files
*************************************************************************************/



/* Public Enumerations
*************************************************************************************/
typedef enum
{
   DM_OP_CHANNEL_SOUNDER_E,
   DM_OP_CHANNEL_BEACON_E,
   DM_OP_CHANNEL_STATUS_INDICATOR_LED_E,
   DM_OP_CHANNEL_VISUAL_INDICATOR_E,
   DM_OP_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E,
   DM_OP_CHANNEL_OUTPUT_ROUTING_E,
   DM_OP_CHANNEL_OUTPUT_1_E,
   DM_OP_CHANNEL_OUTPUT_2_E,
   DM_OP_CHANNEL_MAX_E
} DM_OP_OutputChannel_t;

typedef enum
{
   DM_OP_ALARM_EVENT_FIRE_E,
   DM_OP_ALARM_EVENT_FIRST_AID_E,	
   DM_OP_ALARM_EVENT_EVACUATION_E,
   DM_OP_ALARM_EVENT_SECURITY_E,
   DM_OP_ALARM_EVENT_GENERAL_E,
   DM_OP_ALARM_EVENT_FAULT_E,
   DM_OP_ALARM_EVENT_ROUTING_ACK_E,
   DM_OP_ALARM_EVENT_TEST_E,
   DM_OP_ALARM_EVENT_SILENT_TEST_E,
   DM_OP_ALARM_EVENT_MAX_E
} DM_OP_AlarmEvent_t;

typedef enum
{
   DM_OP_TIMER_NOT_RUNNING_E,
   DM_OP_TIMER_RUNNING_E,
   DM_OP_TIMER_EXPIRED_E
} DM_OP_TimerState_t;

typedef enum
{
   DM_OP_SITENET_SOUNDER_PATTERN_NONE_E,      //No Setting
   DM_OP_SITENET_SOUNDER_PATTERN_ON_E,        //Continuous ON
   DM_OP_SITENET_SOUNDER_PATTERN_1000_7000_E, // 1s ON : 7s OFF
   DM_OP_SITENET_SOUNDER_PATTERN_500_700_E,   // 0.5s ON : 0.7s OFF
   DM_OP_SITENET_SOUNDER_PATTERN_500_300_E,   // (0.5s ON : 0.5 OFF)*3 : 1.5s OFF
   DM_OP_SITENET_SOUNDER_PATTERN_100_2000_e,  // 0.1s ON : 2s OFF
   DM_OP_SITENET_SOUNDER_PATTERN_OFF_E,       // Continuous OFF
   DM_OP_SITENET_SOUNDER_PATTERN_MAX_E
} DM_SiteNetSounderPattern_t;

/* Public Structures
*************************************************************************************/
typedef struct 
{
   bool Activated;
   CO_OutputProfile_t ActiveProfile;
   bool Silenced;
   DM_OP_TimerState_t Delay1State;
   uint16_t Delay1Count;
   DM_OP_TimerState_t Delay2State;
   uint16_t Delay2Count;
   bool TimedOutputActive;
   int32_t OutputTimeoutCount;
}DM_OP_AlarmState_t;

typedef struct
{
   bool Enabled;
   bool Inverted;
   bool IgnoreNightDelays;
   bool Silenceable;
   bool LocalDelays;
   uint16_t Delay1;
   uint16_t Delay2;
   uint8_t ProfileMask;
} AlarmOutputStateConfig_t;


/* Public Functions Prototypes
*************************************************************************************/
void DM_OP_Initialise(const bool LoadProfiles);
void DM_OP_RefreshOutputStatus( void );
void DM_OP_ResetAlarmOutputState( void );
void DM_OP_InitialiseOutputs(void);
void DM_OP_ProcessOutputMessage(const OutputSignalIndication_t* OutputData);
ErrorCode_t DM_OP_RestoreAlarmOutputState( void );
ErrorCode_t DM_OP_ProcessAlarmOutputStateCommand(const CO_AlarmOutputStateData_t* pAlarmData, bool forceIt);
void DM_OP_ProcessFirstAidAlarmCycle(void);
CO_ChannelIndex_t DM_OP_OutputChannelToSystemChannel(const DM_OP_OutputChannel_t outputChannelIndex);
DM_OP_OutputChannel_t DM_OP_SystemChannelToOutputChannel(const CO_ChannelIndex_t systemChannelIndex);
ErrorCode_t DM_OP_SetAlarmState(const CO_ChannelIndex_t systemChannel, const CO_OutputProfile_t profile, const bool activated, const bool ignoreDisablement);
bool DM_OP_TypeIsActive(const uint8_t profile, const DM_OP_AlarmEvent_t alarmType);
void DM_OP_ManageDelays(void);
ErrorCode_t DM_OP_AlarmAcknowledge(const CO_OutputProfile_t profile);
ErrorCode_t DM_OP_AlarmEvacuate(void);
ErrorCode_t DM_OP_SetDisablement(const uint16_t channel, const DM_Enable_t enableDisable);
ErrorCode_t DM_OP_GetDisablement(const uint16_t channel, DM_Enable_t *enableDisable);
bool DM_OP_ChannelDisabled(const CO_ChannelIndex_t channelIndex);
bool DM_OP_IsActivated(DM_OP_OutputChannel_t channel);
bool DM_OP_OutputsActive(const bool ingnoreRelayOutputs);
ErrorCode_t DM_OP_SetSoundLevel(const uint8_t soundLevel);
ErrorCode_t DM_OP_TestOneShot(const bool silentOrStandard);
ErrorCode_t DM_OP_Confirmed(const CO_OutputProfile_t profile);
ErrorCode_t DM_OP_SetAlarmConfiguration(const CO_ChannelIndex_t systemChannel, const uint32_t bitField);
ErrorCode_t DM_OP_GetAlarmConfiguration(const CO_ChannelIndex_t systemChannel, uint32_t *bitField);
ErrorCode_t DM_OP_SetAlarmOptionFlags(const CO_ChannelIndex_t systemChannel, const uint32_t bitField);
ErrorCode_t DM_OP_GetAlarmOptionFlags(const CO_ChannelIndex_t systemChannel, uint32_t *bitField);
ErrorCode_t DM_OP_SetAlarmDelays(const CO_ChannelIndex_t systemChannel, const uint32_t bitField);
ErrorCode_t DM_OP_GetAlarmDelays(const CO_ChannelIndex_t systemChannel, uint32_t *bitField);
CO_OutputProfile_t DM_OP_GetHighestPriorityActiveProfile(void);
void DM_OP_ReportAlarmConfiguration(const UartComm_t port);
void DM_OP_SiteNetCombinedSounderCycle(const uint32_t count);
ErrorCode_t DM_OP_SetLocalOrGlobalDelayFlag(const CO_ChannelIndex_t systemChannel, const bool globalDelay);
bool DM_OP_GetLocalOrGlobalDelayFlag(const CO_ChannelIndex_t systemChannel);
ErrorCode_t DM_OP_SetGlobalDelayValues(const uint32_t CombinedDelay);
ErrorCode_t DM_OP_GetGlobalDelayValues(uint32_t* const pCombinedDelay);
uint32_t DM_OP_GetChannelFlags(const CO_ChannelIndex_t channel);
ErrorCode_t DM_OP_GetChannelOutputConfig(const CO_ChannelIndex_t channel, AlarmOutputStateConfig_t *pAlarmConfig);
void DM_OP_LoadProfiles(void);
ErrorCode_t DM_OP_StartDelayedOutput(const uint32_t initialDelay, const uint32_t duration, const CO_ChannelIndex_t channel);
void DM_OP_RunDelayedOutput(void);
void DM_OP_ResetTestMode(void);
/* Public Constants
*************************************************************************************/
#define DM_OP_BIT_FIRE           0x01
#define DM_OP_BIT_FIRST_AID      0x02
#define DM_OP_BIT_EVACUATION     0x04
#define DM_OP_BIT_SECURITY       0x08
#define DM_OP_BIT_GENERAL        0x10
#define DM_OP_BIT_FAULT          0x20
#define DM_OP_BIT_ROUTING_ACK    0x40
#define DM_OP_BIT_TEST           0x80U
#define DM_OP_BIT_SILENT_TEST    0x100U

#define DM_OP_BIT_CHANNEL_SOUNDER                              0x01
#define DM_OP_BIT_CHANNEL_BEACON                               0x02
#define DM_OP_BIT_CHANNEL_STATUS_INDICATOR_LED                 0x04
#define DM_OP_BIT_CHANNEL_VISUAL_INDICATOR                     0x08
#define DM_OP_BIT_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED    0x10
#define DM_OP_BIT_CHANNEL_OUTPUT_ROUTING                       0x20
#define DM_OP_BIT_CHANNEL_OUTPUT_1                             0x40
#define DM_OP_BIT_CHANNEL_OUTPUT_2                             0x80

/* Macros
*************************************************************************************/



#endif // DM_RELAYCONTROL_H
