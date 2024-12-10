/*************************************************************************************
*  CONFIDENTIAL between PA and Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2017 PA Consulting Group. All rights reserved.
*  Cambridge Technology Centre
*  Melbourn Royston
*  Herts SG8 6DP UK
*  Tel: <44> 1763 261222
*
**************************************************************************************
*  File         : DM_InputMonitor.h
*
*  Description  : Header for the Input Monitor device
*
*************************************************************************************/

#ifndef DM_INPUT_MONITOR_H
#define DM_INPUT_MONITOR_H


/* System Include Files
*************************************************************************************/
#include <stdbool.h>
#include <stdint.h>

/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"
#include "gpio.h"
#include "MM_Interrupts.h"

/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/
typedef enum
{
   IN_MON_UNKNOWN_STATE_E,
   IN_MON_IDLE_E,
   IN_MON_ACTIVE_E,
   IN_MON_MAX_STATE_E
} DM_InputMonitorState_t;


typedef enum
{
   IN_MON_INSTALLATION_TAMPER_E,
   IN_MON_DISMANTLE_TAMPER_E,
   IN_MON_MAIN_BATTERY_E,
   IN_MON_PIR_E,
   IN_MON_HEAD_TAMPER_E,
   IN_MON_MAX_IDX_E
} DM_InputMonitorIdx_t;


/* Public Functions Prototypes
*************************************************************************************/
typedef ErrorCode_t(*pfInputMonitor_StateChangeCb_t)(DM_InputMonitorState_t newState);

typedef struct 
{
   bool IsEnabled;
   uint32_t ActivateThreshold;
   uint32_t DeactivateThreshold;
   uint32_t InputMonitor_time_us;
   Gpio_t* pGpio_def;
   GPIO_PinState PinIdleState;
   DM_InputMonitorState_t ConfirmedState;
   GpioIrqHandler* pfIrqHandler;
   pfInputMonitor_StateChangeCb_t pStateChangeCallback;
	uint32_t Count;
	uint32_t StrikeCount;
} DM_InputMonitorConfig_t;



ErrorCode_t DM_InputMonitorInit(DM_InputMonitorIdx_t idx, pfInputMonitor_StateChangeCb_t pfStateChangeCb, const uint32_t pullup);

ErrorCode_t DM_InputMonitorPoll(DM_InputMonitorIdx_t idx);

bool DM_InputMonitorGetPollingEnabled(void);
void DM_InputMonitorPollingStart(void);
void DM_InputMonitorPollingEnd(void);
void DM_SetFireCallPointState(const DM_InputMonitorState_t state);
DM_InputMonitorState_t DM_GetFireCallPointState(void);
void DM_SetFirstAidCallPointState(const DM_InputMonitorState_t state);
DM_InputMonitorState_t DM_GetFirstAidCallPointState(void);
DM_InputMonitorState_t DM_GetPirState(void);
void DM_InputMonitorSleep(void);
DM_InputMonitorConfig_t* DM_GetPirConfig(void);

/* Public Constants
*************************************************************************************/

// Debounce Delays
#define FIRE_ACTIVATE_DEBOUNCE_US              (500u)
#define FIRE_DEACTIVATE_DEBOUNCE_US            (2000u)
#define FIRST_AID_ACTIVATE_DEBOUNCE_US         (500u)
#define FIRST_AID_DEACTIVATE_DEBOUNCE_US       (2000u)
#define TAMPER_SWITCH_ACTIVATE_DEBOUNCE_US     (500u)
#define TAMPER_SWITCH_DEACTIVATE_DEBOUNCE_US   (2000u)
#define MAIN_BATTERY_ACTIVATE_DEBOUNCE_US      (1000u)
#define MAIN_BATTERY_DEACTIVATE_DEBOUNCE_US    (1000u)
#define PIR_ACTIVATE_DEBOUNCE_US               (1000u)
#define PIR_DEACTIVATE_DEBOUNCE_US             (2000u)
#define HEAD_TAMPER_ACTIVATE_DEBOUNCE_US       (500u)
#define HEAD_TAMPER_DEACTIVATE_DEBOUNCE_US     (2000u)

//Call point - reported sensor values
#define GPIO_IDLE    (0u)
#define GPIO_ACTIVE  (99u)

/* Macros
*************************************************************************************/
#define DM_IN_MON_ASSERT(a) if (!(a)) return ERR_OUT_OF_RANGE_E;



#endif // DM_INPUT_MONITOR_H
