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
*  File         : DM_LED_cfg.h
*
*  Description  : Header file of the configuration of the LED driver module
*
*************************************************************************************/

#ifndef DM_LED_CFG_H
#define DM_LED_CFG_H


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
   /* NOTE: These are ordered from low to high Priority */
   LED_CONSTANT_GREEN_E,                  // 0
   LED_CONSTANT_BLUE_E,                   // 1
   LED_CONSTANT_RED_E,                    // 2
   LED_CONSTANT_WHITE_E,                  // 3
   LED_CONSTANT_MAGENTA_E,                // 4
   LED_CONSTANT_CYAN_E,                   // 5
   LED_CONSTANT_AMBER_E,                  // 6
   LED_MISSING_BATTERY_E,                 // 7
   LED_INTERNAL_FAULT_E,                  // 8
   LED_MESH_SLEEP1_E,                     // 9
   LED_MESH_SLEEP2_E,                     // 10
   LED_MESH_IDLE_E,                       // 11
   LED_PPU_DISCONNECTED_E,                // 12
   LED_PPU_CONNECTED_E,                   // 13
   LED_MESH_CONNECTED_E,                  // 14
   LED_MESH_STATE_CONFIG_SYNC_E,          // 15
   LED_MESH_STATE_CONFIG_FORM_E,          // 16
   LED_MESH_STATE_READY_FOR_ACTIVE_E,     // 17
   LED_MESH_STATE_ACTIVE_E,               // 18
   LED_FIRST_AID_INDICATION_E,            // 19
   LED_FIRE_INDICATION_E,                 // 20
#ifdef SHOW_DOWNLINK_LED
   LED_DOWNLINK_MESSAGE_E,                 // 21
#endif
   LED_PATTERNS_MAX_E,
   LED_PATTERN_NOT_DEFINED_E
} DM_LedPatterns_t;


/* Public functions prototypes
*******************************************************************************/


/* Public Constants
*******************************************************************************/


/* Macros
*******************************************************************************/


#endif // DM_LED_CFG_H
