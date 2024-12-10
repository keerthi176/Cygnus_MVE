/*************************************************************************************
*  CONFIDENTIAL between PA and Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2018 PA Consulting Group. All rights reserved.
*  Cambridge Technology Centre
*  Melbourn Royston
*  Herts SG8 6DP UK
*  Tel: <44> 1763 261222
*
**************************************************************************************
*  File         : DM_LED.h
*
*  Description  : Header file for the LED module
*
*************************************************************************************/

#ifndef DM_LED_H
#define DM_LED_H


/* System Include files
*******************************************************************************/
#include <stdint.h>
#include <stdbool.h>


/* User Include files
*******************************************************************************/
#include "DM_LED_cfg.h"

/* Public Enumerations
*************************************************************************************/

typedef enum
{
   DM_LED_OFF_E,
   DM_LED_RED_E,
   DM_LED_GREEN_E,
   DM_LED_BLUE_E,
   DM_LED_WHITE_E,
   DM_LED_CYAN_E,
   DM_LED_MAGENTA_E,
   DM_LED_AMBER_E,
   DM_LED_STATE_MAX_E
} DM_LedState_t;



/* Public Constants
*************************************************************************************/
#define LED_SEQUENCE_SHIFT    (2U)
#define LED_SEQUENCE_LENGTH   (1U << LED_SEQUENCE_SHIFT)
#define LED_SEQUENCE_BITMASK  (LED_SEQUENCE_LENGTH - 1U)
#define LED_NO_TIME_LIMIT      0xFFFFFFFFu

/* Public Structures
*************************************************************************************/ 
typedef struct
{
   struct
   {
      DM_LedState_t led_state;
      uint32_t step_duration_us;
   }state_steps_def[LED_SEQUENCE_LENGTH];
   uint32_t max_duration_us;
   bool isRepeated;
   bool step_on_event;
} DM_LedPatternConfig_t;


/* Public Functions Prototypes
*************************************************************************************/
void DM_LedInit(void);
void DM_LedPatternRequest(const DM_LedPatterns_t led_pattern);
void DM_LedPatternRemove(const DM_LedPatterns_t led_pattern);
ErrorCode_t DM_LedPatternRemoveAll(void);
void DM_LedExternalControl(const DM_LedPatterns_t led_pattern, const uint16_t step_req);
void DM_LedPeriodicTick(void);
void DM_LedTurnOff(void);
DM_LedPatterns_t DM_LedGetCurrentPattern(void);
void DM_LedSetState(const DM_LedState_t state);

/* Macros
*************************************************************************************/


#endif // DM_LED_H
