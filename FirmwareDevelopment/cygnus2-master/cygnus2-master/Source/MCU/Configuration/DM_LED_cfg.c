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
*  File         : DM_LED_cfg.c
*
*  Description  :
*
*   Configuration of the LED driver module
*
*************************************************************************************/



/* System Include files
*******************************************************************************/
#include <string.h>

/* User Include files
*******************************************************************************/

#include "stm32l4xx.h"
#include "CO_Defines.h"
#include "DM_LED.h"


/* Global Variables
*******************************************************************************/

const DM_LedPatternConfig_t DM_LedPatternConfig[LED_PATTERNS_MAX_E] = 
{
   [LED_CONSTANT_GREEN_E] = 
   {
      .state_steps_def[0] = 
      {
         .led_state = DM_LED_GREEN_E,
         .step_duration_us = LED_NO_TIME_LIMIT,
      },

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = false,
      .step_on_event   = false,
   },

   [LED_CONSTANT_BLUE_E] = 
   {
      .state_steps_def[0] = 
      {
         .led_state = DM_LED_BLUE_E,
         .step_duration_us = LED_NO_TIME_LIMIT,
      },

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = false,
      .step_on_event   = false,
   },

   [LED_CONSTANT_RED_E] = 
   {
      .state_steps_def[0] = 
      {
         .led_state = DM_LED_RED_E,
         .step_duration_us = LED_NO_TIME_LIMIT,
      },

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = false,
      .step_on_event   = false,
   },
   
   [LED_CONSTANT_WHITE_E] = 
   {
      .state_steps_def[0] = 
      {
         .led_state = DM_LED_WHITE_E,
         .step_duration_us = LED_NO_TIME_LIMIT,
      },

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = false,
      .step_on_event   = false,
   },
   
   [LED_CONSTANT_MAGENTA_E] = 
   {
      .state_steps_def[0] = 
      {
         .led_state = DM_LED_MAGENTA_E,
         .step_duration_us = LED_NO_TIME_LIMIT,
      },

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = false,
      .step_on_event   = false,
   },
   
   [LED_CONSTANT_CYAN_E] = 
   {
      .state_steps_def[0] = 
      {
         .led_state = DM_LED_CYAN_E,
         .step_duration_us = LED_NO_TIME_LIMIT,
      },

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = false,
      .step_on_event   = false,
   },
   
   [LED_CONSTANT_AMBER_E] = 
   {
      .state_steps_def[0] = 
      {
         .led_state = DM_LED_AMBER_E,
         .step_duration_us = LED_NO_TIME_LIMIT,
      },

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = false,
      .step_on_event   = false,
   },
   
   [LED_MESH_STATE_ACTIVE_E] =
   {
      .state_steps_def = 
      { 
         {
            .led_state = DM_LED_GREEN_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_GREEN_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_GREEN_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_GREEN_E,
            .step_duration_us = 0,
         },
      },

      .max_duration_us = (uint32_t)2e6,
      .isRepeated      = false,
      .step_on_event   = true,
   },

   [LED_MISSING_BATTERY_E] =
   {
      .state_steps_def = 
      { 
         {
            .led_state = DM_LED_AMBER_E,
            .step_duration_us = (uint32_t)150e3,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = (uint32_t)500e3,
         },
         {
            .led_state = DM_LED_AMBER_E,
            .step_duration_us = (uint32_t)150e3,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = (uint32_t)10e6,
         },
      }, 

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = true,
      .step_on_event   = false,
   },  

   [LED_INTERNAL_FAULT_E] = 
   {
      .state_steps_def = 
      { 
         {
            .led_state = DM_LED_AMBER_E,
            .step_duration_us = (uint32_t)150e3,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = (uint32_t)10e6,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
      }, 

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = true,
      .step_on_event   = false, 
   },
   
   [LED_PPU_DISCONNECTED_E] =
   {
      .state_steps_def = 
      { 
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_RED_E,
            .step_duration_us = (uint32_t)400e3,
         },
         {
            .led_state = DM_LED_GREEN_E,
            .step_duration_us = (uint32_t)400e3,
         },
         {
            .led_state = DM_LED_BLUE_E,
            .step_duration_us = (uint32_t)400e3,
         },
      },

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = true,
      .step_on_event   = false,
   },
   
   [LED_PPU_CONNECTED_E] =
   {
      .state_steps_def = 
      { 
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_RED_E,
            .step_duration_us = (uint32_t)200e3,
         },
         {
            .led_state = DM_LED_GREEN_E,
            .step_duration_us = (uint32_t)200e3,
         },
         {
            .led_state = DM_LED_BLUE_E,
            .step_duration_us = (uint32_t)200e3,
         },
      },

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = true,
      .step_on_event   = false,
   },
   
   [LED_MESH_SLEEP1_E] =
   {
      .state_steps_def = 
      { 
         {
            .led_state = DM_LED_BLUE_E,
            .step_duration_us = (uint32_t)150e3,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = (uint32_t)30e6,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
      }, 

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = true,
      .step_on_event   = false, 
   },
   
   [LED_MESH_SLEEP2_E] =
   {
      .state_steps_def = 
      { 
         {
            .led_state = DM_LED_BLUE_E,
            .step_duration_us = (uint32_t)150e3,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = (uint32_t)60e6,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
      }, 

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = true,
      .step_on_event   = false, 
   },
   

   [LED_MESH_IDLE_E] = 
   {
      .state_steps_def = 
      { 
         {
            .led_state = DM_LED_BLUE_E,
            .step_duration_us = (uint32_t)150e3,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = (uint32_t)300e3,
         },
         {
            .led_state = DM_LED_BLUE_E,
            .step_duration_us = (uint32_t)150e3,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 10e6,
         },
      }, 

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = true,
      .step_on_event   = false, 
   },
   
   [LED_MESH_CONNECTED_E] =
   {
      .state_steps_def = 
      { 
         {
            .led_state = DM_LED_GREEN_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
      },

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = true,
      .step_on_event   = true,
   },

   [LED_MESH_STATE_READY_FOR_ACTIVE_E] =
   {
      .state_steps_def = 
      { 
         {
            .led_state = DM_LED_GREEN_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
      },

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = true,
      .step_on_event   = true,
   },

   [LED_MESH_STATE_CONFIG_FORM_E] =
   {
      .state_steps_def = 
      { 
         {
            .led_state = DM_LED_MAGENTA_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
      },

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = true,
      .step_on_event   = true,
   },

   [LED_MESH_STATE_CONFIG_SYNC_E] =
   {
      .state_steps_def = 
      { 
         {
            .led_state = DM_LED_BLUE_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
         {
            .led_state = DM_LED_OFF_E,
            .step_duration_us = 0,
         },
      }, 

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = true,
      .step_on_event   = true,
   },

   [LED_FIRST_AID_INDICATION_E] = 
   {
      .state_steps_def[0] = 
      {
         .led_state = DM_LED_GREEN_E,
         .step_duration_us = LED_NO_TIME_LIMIT,
      },

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = false,
      .step_on_event   = false,
   },
   
   [LED_FIRE_INDICATION_E] = 
   {
      .state_steps_def[0] = 
      {
         .led_state = DM_LED_RED_E,
         .step_duration_us = LED_NO_TIME_LIMIT,
      },

      .max_duration_us = LED_NO_TIME_LIMIT,
      .isRepeated      = false,
      .step_on_event   = false,
   },
   
#ifdef SHOW_DOWNLINK_LED
   [LED_DOWNLINK_MESSAGE_E] =
   {
      .state_steps_def = 
      { 
         {
            .led_state = DM_LED_GREEN_E,
            .step_duration_us = 100000,
         },
         {
            .led_state = DM_LED_RED_E,
            .step_duration_us = 100000,
         },
         {
            .led_state = DM_LED_GREEN_E,
            .step_duration_us = 100000,
         },
         {
            .led_state = DM_LED_RED_E,
            .step_duration_us = 100000,
         },
      },

      .max_duration_us = (uint32_t)2e6,
      .isRepeated      = true,
      .step_on_event   = false,
   },
#endif
   
};
