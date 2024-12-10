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
*  File         : DM_LED.c
*
*  Description  : Implementation of the LED module
*
*                 Application code passes in a configuration to define the behaviour of
*                 the LED. This consists of a sequence of steps where each step includes
*                 a state and duration.
*
*                 A tick function must be called from the application at regular intervals.
*                 This is used to control the LED sequence.
*
*************************************************************************************/

#define USE_NEW_LED_SYSTEM

/* System Include files
*******************************************************************************/


/* User Include files
*******************************************************************************/
#include "cmsis_os.h"
#include "board.h"
#include "MM_TimedEventTask.h"
#include "DM_LED.h"

#ifdef USE_NEW_LED_SYSTEM


/* Private Functions Prototypes
*************************************************************************************/
void DM_LedSetState(const DM_LedState_t state);
static void DM_LedEventHandler(const DM_LedPatterns_t led_pattern, const uint16_t step_req);
static void DM_LedPeriodicTickHandler(void);
static bool DM_LedPatternExpired(void);
static bool DM_LedStepExpired(void);
static bool DM_LedPatternNextPattern(void);
static bool DM_LedPatternNextStep(void);

/* Global Variables
*************************************************************************************/
extern const DM_LedPatternConfig_t DM_LedPatternConfig[LED_PATTERNS_MAX_E];


/* Private Variables
*************************************************************************************/
/* Mutex */
static osMutexId(LedMutexId);
static osMutexDef(LedMutex);

static struct
{
   bool active;
   uint8_t step;
   uint32_t step_start_time;
   uint32_t pattern_start_time;
} DM_PatternRequested[LED_PATTERNS_MAX_E];


/*************************************************************************************/
/**
* function name   : DM_LedSetState
* description     : Routine to set the state of the Radio Board LED
*
* @param - const DM_LedState_t state : led state
*
* @return - void
*/
void DM_LedSetState(const DM_LedState_t state)
{
   switch(state)
   {
      case DM_LED_RED_E:
         GpioWrite(&StatusLedRed, 1);
         GpioWrite(&StatusLedGreen, 0);
         GpioWrite(&StatusLedBlue, 0);
         break;
      
      case DM_LED_GREEN_E:
         GpioWrite(&StatusLedRed, 0);
         GpioWrite(&StatusLedGreen, 1);
         GpioWrite(&StatusLedBlue, 0);
         break;
      
      case DM_LED_BLUE_E:
         GpioWrite(&StatusLedRed, 0);
         GpioWrite(&StatusLedGreen, 0);
         GpioWrite(&StatusLedBlue, 1);
         break;
      
      case DM_LED_WHITE_E:
         GpioWrite(&StatusLedRed, 1);
         GpioWrite(&StatusLedGreen, 1);
         GpioWrite(&StatusLedBlue, 1);
         break;
      
      case DM_LED_CYAN_E:
         GpioWrite(&StatusLedRed, 0);
         GpioWrite(&StatusLedGreen, 1);
         GpioWrite(&StatusLedBlue, 1);
         break;
      
      case DM_LED_MAGENTA_E:
         GpioWrite(&StatusLedRed, 1);
         GpioWrite(&StatusLedGreen, 0);
         GpioWrite(&StatusLedBlue, 1);
         break;
      
      case DM_LED_AMBER_E:
         GpioWrite(&StatusLedRed, 1);
         GpioWrite(&StatusLedGreen, 1);
         GpioWrite(&StatusLedBlue, 0);
         break;
      
      case DM_LED_OFF_E:               //Intentional drop-through
      case DM_LED_STATE_MAX_E:
      default:
         /* turn off all leds */
         GpioWrite(&StatusLedRed, 0);
         GpioWrite(&StatusLedGreen, 0);
         GpioWrite(&StatusLedBlue, 0);
         break;
   }
}

/*************************************************************************************/
/**
* function name   : DM_LedInit
* description     : Routine to initialise the LED module
*
* @param - none
*
* @return - void
*/
void DM_LedInit(void)
{
   LedMutexId = osMutexCreate(osMutex(LedMutex));
   if(NULL == LedMutexId)
   {
      Error_Handler("Failed to create LED mutex");
   }
   
   for ( uint32_t pattern = 0; pattern < LED_PATTERNS_MAX_E; pattern++)
   {
      DM_PatternRequested[pattern].active = false;
      DM_PatternRequested[pattern].step = 0;
   }
}

/*************************************************************************************/
/**
* function name   : DM_LedPatternRequest
* description     : Routine to request an LED pattern
*
* @param - const DM_LedPattern_t led_pattern : Requested Pattern
*
* @return - void
*/
void DM_LedPatternRequest(const DM_LedPatterns_t led_pattern)
{
   if ( LED_PATTERNS_MAX_E > led_pattern )
   {
      CO_PRINT_B_1(DBG_INFO_E,"DM_LedPatternRequest %d\r\n", led_pattern);
      /* Acquire the mutex to avoid concurrent access */
      if (osOK == osMutexWait(LedMutexId, 100u))
      {
         DM_LedPatterns_t current_pattern = DM_LedGetCurrentPattern();
         //Only proceed if the pattern isn't currently active
         if ( led_pattern != current_pattern )
         {
            DM_PatternRequested[led_pattern].active = true;
            //CO_PRINT_B_1(DBG_INFO_E,"LED pattern activated %d\r\n", led_pattern);
            
            //For mesh transition state, cancel the previous mesh state
            switch ( led_pattern )
            {
               case LED_MESH_STATE_CONFIG_FORM_E:
                  DM_LedPatternRemove(LED_MESH_STATE_CONFIG_SYNC_E);
                  break;
               case LED_MESH_STATE_READY_FOR_ACTIVE_E:
                  DM_LedPatternRemove(LED_MESH_STATE_CONFIG_FORM_E);
                  break;
               case LED_MESH_STATE_ACTIVE_E:
                  DM_LedPatternRemove(LED_MESH_STATE_READY_FOR_ACTIVE_E);
                  break;
               default:
               break;
            }
            
            //See if the new pattern took priority
            current_pattern = DM_LedGetCurrentPattern();
            if ( current_pattern == led_pattern )
            {
               uint32_t now_time = osKernelSysTick();
               DM_PatternRequested[current_pattern].pattern_start_time = now_time;
               DM_PatternRequested[current_pattern].step = 0;
               DM_PatternRequested[current_pattern].step_start_time = now_time;
               
               // if the pattern is event driven or steady state (no flash), set the first LED step immediately
               if ( DM_LedPatternConfig[current_pattern].step_on_event ||
                   (LED_NO_TIME_LIMIT == DM_LedPatternConfig[current_pattern].state_steps_def[0].step_duration_us) )
               {
                  DM_LedSetState(DM_LedPatternConfig[current_pattern].state_steps_def[0].led_state);
               }
            }
         }
         /* Release the Mutex */
         osMutexRelease(LedMutexId);
      }
   }
}


/*************************************************************************************/
/**
* function name   : DM_LedPatternRemove
* description     : Routine to remove an LED pattern
*
* @param - const DM_LedPattern_t led_pattern : Requested Pattern
*
* @return - void
*/
void DM_LedPatternRemove(const DM_LedPatterns_t led_pattern)
{
   if ( LED_PATTERNS_MAX_E > led_pattern )
   {
      CO_PRINT_B_1(DBG_INFO_E,"DM_LedPatternRemove %d\r\n", led_pattern);
      /* Acquire the mutex to avoid concurrent access */
      if (osOK == osMutexWait(LedMutexId, 100u))
      {
         //Get which pattern is active
         DM_LedPatterns_t current_pattern = DM_LedGetCurrentPattern();
         //If the deactivated pattern was active, Call up the next pattern.
         if ( led_pattern == current_pattern )
         {
            //move to the next pattern.  This function will
            //automatically remove the current pattern before selecting the next.
            DM_LedPatternNextPattern();
         }
         else
         {
            //The pattern is currently overridden by a higher priority pattern
            //or there are no patterns active.
            //Mark it as inactive without selecting a new pattern
            DM_PatternRequested[led_pattern].active = false;
            //CO_PRINT_B_1(DBG_INFO_E,"LED pattern removed %d\r\n", led_pattern);
         }
         /* Release the Mutex */
         osMutexRelease(LedMutexId);
      }
   }
}

/*************************************************************************************/
/**
* function name   : DM_LedPatternRemoveAll
* description     : Routine to remove all LED patterns
*
* @param - void
*
* @return - Error code
*/
ErrorCode_t DM_LedPatternRemoveAll(void)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;
   CO_PRINT_B_0(DBG_INFO_E,"DM_LedPatternRemoveAll\r\n");
   /* Acquire the mutex to avoid concurrent access */
   if (osOK == osMutexWait(LedMutexId, 100u))
   {
      for ( uint32_t pattern = 0; pattern < LED_PATTERNS_MAX_E; pattern++)
      {
         DM_PatternRequested[pattern].active = false;
         DM_PatternRequested[pattern].step = 0;
      }
      //Turn the LEDs OFF
      DM_LedTurnOff();
      /* Release the Mutex */
      osMutexRelease(LedMutexId);
      
      result = SUCCESS_E;
   }
   
   return result;
}

/*************************************************************************************/
/**
* function name   : DM_LedExternalControl
* description     : Routine to control the state of the LEDs from an external module
*
* @param - const DM_LedPattern_t led_pattern : Requested Pattern
* @param - const uint16_t step_req : Requested step of the sequence
*
* @return - void
*/
void DM_LedExternalControl(const DM_LedPatterns_t led_pattern, const uint16_t step_req)
{
   /* Acquire the mutex to avoid concurrent access */
   if (osOK == osMutexWait(LedMutexId, 100u))
   {
      /* Call the main LED handler */
      DM_LedEventHandler(led_pattern, step_req);

      /* Release the Mutex */
      osMutexRelease(LedMutexId);
   }
}

/*************************************************************************************/
/**
* function name   : DM_LedTick
* description     : Periodic routine to update the state of the LEDs
*
* @param - none
*
* @return - void
*/
void DM_LedPeriodicTick(void)
{
   /* Acquire the mutex to avoid concurrent access */
   if (osOK == osMutexWait(LedMutexId, 100u)) 
   {
      /* Call the main LED handler */
      DM_LedPeriodicTickHandler();
   
      /* Release the Mutex */
      osMutexRelease(LedMutexId);
   }
}

/*************************************************************************************/
/**
* function name   : DM_LedPeriodicTickHandler
* description     : Handler to manage the LED Patterns that are timer driven
*
* @param - None.
* @return - void
*/
static void DM_LedPeriodicTickHandler(void)
{
   DM_LedPatterns_t current_pattern = DM_LedGetCurrentPattern();
   
   //Only act if a pattern is running
   if ( LED_PATTERN_NOT_DEFINED_E > current_pattern )
   {
      //Don't process event driven patterns
      if ( !DM_LedPatternConfig[current_pattern].step_on_event )
      {
         //Has the pattern expired?
         if ( DM_LedPatternExpired() )
         {
//            CO_PRINT_B_1(DBG_INFO_E,"LED pattern expired= %d\r\n", current_pattern);
            //The pattern has run its time.  Call up the next one
            DM_LedPatternNextPattern();
         }
         else 
         {
            //Has the step timed out
            if (DM_LedStepExpired())
            {
               if ( DM_LedPatternNextStep() )
               {
                  //The pattern has moved to the next step. set the LEDs.
                  uint32_t step = DM_PatternRequested[current_pattern].step;
                  DM_LedSetState(DM_LedPatternConfig[current_pattern].state_steps_def[step].led_state);
               }
               else 
               {
//                  CO_PRINT_B_1(DBG_INFO_E,"LED pattern expired= %d\r\n", current_pattern);
                  //The pattern ended.  Call up the next one.
                  DM_LedPatternNextPattern();
               }
            }
         }
         
      }
   }
}

/*************************************************************************************/
/**
* function name   : DM_LedEventHandler
* description     : Handler to manage the LED Patterns that are driven by events
*
* @param - const DM_LedPattern_t led_pattern : Requested Pattern
* @param - const uint16_t step_req : Requested step of the sequence
* @return - void
*/
static void DM_LedEventHandler(const DM_LedPatterns_t led_pattern, const uint16_t step_req)
{
   DM_LedPatterns_t current_pattern = DM_LedGetCurrentPattern();
   
   //Only act if a pattern is running
   if ( LED_PATTERN_NOT_DEFINED_E != current_pattern )
   {
      //Check that the pattern is active and is event driven, and the requested step is valid.
      if ( (current_pattern == led_pattern) && 
           (LED_SEQUENCE_LENGTH > step_req) &&
           (DM_LedPatternConfig[current_pattern].step_on_event) )
      {
         //Has the pattern expired?
         if ( DM_LedPatternExpired() )
         {
            //The pattern has run its time.  Call up the next one
            DM_LedPatternNextPattern();
         }
         else 
         {
            //move to the requested step
            if ( 0 == step_req && ((LED_SEQUENCE_LENGTH-1) == DM_PatternRequested[current_pattern].step) )
            {
               //end of sequence.  Check for repeats.
               if ( DM_LedPatternConfig[current_pattern].isRepeated )
               {
                  DM_PatternRequested[current_pattern].step = step_req;
                  DM_LedSetState(DM_LedPatternConfig[current_pattern].state_steps_def[step_req].led_state);
               }
               else
               {
                  //Pattern doesn't repeat.  Call up the next one
                  DM_LedPatternNextPattern();
               }
            }
            else 
            {
               DM_PatternRequested[current_pattern].step = step_req;
               DM_LedSetState(DM_LedPatternConfig[current_pattern].state_steps_def[step_req].led_state);
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* function name   : DM_LedTurnOff
* description     : Routine to turn off leds to save power
*
*
* @param  - none
*
* @return - void
*/
void DM_LedTurnOff(void)
{
   DM_LedSetState(DM_LED_OFF_E);
}

/*************************************************************************************/
/**
* function name   : DM_LedGetCurrentPattern
* description     : Routine to get the current LED activity.
*
*
* @param  - none
*
* @return - current LED pattern
*/
DM_LedPatterns_t DM_LedGetCurrentPattern(void)
{
   DM_LedPatterns_t pattern = LED_PATTERN_NOT_DEFINED_E;
   
   int32_t pattern_index = LED_PATTERNS_MAX_E;
   do
   {
      pattern_index--;
      if ( DM_PatternRequested[pattern_index].active )
      {
         pattern = (DM_LedPatterns_t)pattern_index;
      }
   } while ( (LED_PATTERN_NOT_DEFINED_E == pattern) && (0 < pattern_index));
   
   
   return pattern;
}


/*************************************************************************************/
/**
* function name   : DM_LedPatternExpired
* description     : Routine to check if a timed pattern has expired.
*
*
* @param  - None.
*
* @return - true on expiry.
*/
bool DM_LedPatternExpired(void)
{
   bool expired = false;
   DM_LedPatterns_t current_pattern = DM_LedGetCurrentPattern();
   uint32_t now_time = osKernelSysTick();
   
   //Only act if a pattern is running
   if ( LED_PATTERN_NOT_DEFINED_E != current_pattern )
   {
      //Has the pattern expired?
      if ( LED_NO_TIME_LIMIT != DM_LedPatternConfig[current_pattern].max_duration_us )
      {
         // it is a timed pattern.  check for expiry.
         if ((now_time - DM_PatternRequested[current_pattern].pattern_start_time) > osKernelSysTickMicroSec(DM_LedPatternConfig[current_pattern].max_duration_us))
         {
            //expired
            expired = true;
            DM_LedTurnOff();
         }
      }
   }
   else 
   {
      expired = true;
   }

   return expired;
}

/*************************************************************************************/
/**
* function name   : DM_LedStepExpired
* description     : Routine to check if a timed step within pattern has expired.
*
*
* @param  - None.
*
* @return - true on expiry.
*/
bool DM_LedStepExpired(void)
{
   bool expired = false;
   DM_LedPatterns_t current_pattern = DM_LedGetCurrentPattern();
   uint32_t now_time = osKernelSysTick();
   
   //Only act if a pattern is running
   if ( LED_PATTERN_NOT_DEFINED_E != current_pattern )
   {
      uint32_t current_step = DM_PatternRequested[current_pattern].step;
      //is the current step timed?
      if ( 0 != DM_LedPatternConfig[current_pattern].state_steps_def[current_step].step_duration_us )
      {
         if ( LED_NO_TIME_LIMIT != DM_LedPatternConfig[current_pattern].state_steps_def[current_step].step_duration_us)
         {
            //it's a timed step.  Check for expiry
            if ((now_time - DM_PatternRequested[current_pattern].step_start_time) > osKernelSysTickMicroSec(DM_LedPatternConfig[current_pattern].state_steps_def[current_step].step_duration_us))
            {
               //Expired
               expired = true;
            }
         }
      }
      else 
      {
         expired = true;
      }
   }
   else 
   {
      expired = true;
   }

   return expired;
}


/*************************************************************************************/
/**
* function name   : DM_LedPatternNextPattern
* description     : Routine to advance to the next priority pattern.
*
*
* @param  - None.
*
* @return - true if another pattern became active.
*/
bool DM_LedPatternNextPattern(void)
{
   bool new_pattern_selected = false;
   DM_LedPatterns_t current_pattern = DM_LedGetCurrentPattern();
   uint32_t now_time = osKernelSysTick();
   
   //Make sure that the LEDs default to OFF
   DM_LedTurnOff();
   
   //Is there a pattern active?
   if ( LED_PATTERN_NOT_DEFINED_E != current_pattern )
   {
      //Cancel the current pattern
      DM_PatternRequested[current_pattern].active = false;
      CO_PRINT_B_1(DBG_INFO_E,"Cancelled LED pattern = %d\r\n", current_pattern);
      
      //Call up the next pattern
      DM_LedPatterns_t next_pattern = DM_LedGetCurrentPattern();
      if ( LED_PATTERN_NOT_DEFINED_E != next_pattern )
      {
         DM_PatternRequested[next_pattern].active = true;
         DM_PatternRequested[next_pattern].pattern_start_time = now_time;
         DM_PatternRequested[next_pattern].step = 0;
         DM_PatternRequested[next_pattern].step_start_time = now_time;
         new_pattern_selected = true;
         CO_PRINT_B_1(DBG_INFO_E,"New LED pattern = %d\r\n", next_pattern);
         DM_LedTurnOff();
         //If the pattern is a steady pattern (no flashing) or event driven set the LED now
         if ( (LED_NO_TIME_LIMIT == DM_LedPatternConfig[next_pattern].state_steps_def[0].step_duration_us) ||
              (DM_LedPatternConfig[next_pattern].step_on_event) )
         {
            DM_LedSetState(DM_LedPatternConfig[next_pattern].state_steps_def[0].led_state);
         }
      }
      else 
      {
         CO_PRINT_B_0(DBG_INFO_E,"LED pattern = OFF\r\n");
      }
   }

   return new_pattern_selected;
}



/*************************************************************************************/
/**
* function name   : DM_LedPatternNextStep
* description     : Routine to advance to the next step of a pattern.
*
*
* @param  - None.
*
* @return - true if the step was advanced. false if advancing the step expired the pattern.
*/
bool DM_LedPatternNextStep(void)
{
   bool step_advanced = true;
   DM_LedPatterns_t current_pattern = DM_LedGetCurrentPattern();
   
   //Only act if a pattern is running
   if ( LED_PATTERN_NOT_DEFINED_E != current_pattern )
   {
      uint32_t current_step = DM_PatternRequested[current_pattern].step;
      uint32_t next_step = (current_step + 1) % LED_SEQUENCE_LENGTH;
      
      DM_PatternRequested[current_pattern].step = next_step;
      
      //Has the pattern expired?
      if ( 0 == next_step )
      {
         if (false == DM_LedPatternConfig[current_pattern].isRepeated )
         {
            //The pattern is not a repeating pattern.  mark it complete.
            step_advanced = false;
         }
      }
   }
   else 
   {
      step_advanced = false;
   }
   
   //Record the time that the new step started
   if ( step_advanced )
   {
      DM_PatternRequested[current_pattern].step_start_time = osKernelSysTick();
   }

   return step_advanced;
}





#else

/* Private Functions Prototypes
*************************************************************************************/
static void DM_LedSetState(const DM_LedState_t state);
static void DM_LedHandler(const DM_LedPatterns_t led_pattern, const uint16_t step_req);


/* Global Variables
*************************************************************************************/
extern const DM_LedPatternConfig_t DM_LedPatternConfig[LED_PATTERNS_MAX_E];


/* Private Variables
*************************************************************************************/
static DM_LedPatterns_t DM_LedCurrentPattern;
static DM_LedPatterns_t DM_LedNextPattern;
static uint16_t DM_LedCurrentStep = 0u;
static uint32_t DM_LedStepStartTime = 0u;
static uint32_t DM_LedPatternStartTime = 0u;
/* Mutex */
static osMutexId(LedMutexId);
static osMutexDef(LedMutex);

static struct
{
   bool active;
   uint8_t step;
} DM_PatternRequested[LED_PATTERNS_MAX_E];

/*************************************************************************************/
/**
* function name   : DM_LedSetState
* description     : Routine to set the state of the Radio Board LED
*
* @param - const DM_LedState_t state : led state
*
* @return - void
*/
static void DM_LedSetState(const DM_LedState_t state)
{
   switch(state)
   {
      case DM_LED_RED_E:
         GpioWrite(&StatusLedRed, 1);
         GpioWrite(&StatusLedGreen, 0);
         GpioWrite(&StatusLedBlue, 0);
         break;
      
      case DM_LED_GREEN_E:
         GpioWrite(&StatusLedRed, 0);
         GpioWrite(&StatusLedGreen, 1);
         GpioWrite(&StatusLedBlue, 0);
         break;
      
      case DM_LED_BLUE_E:
         GpioWrite(&StatusLedRed, 0);
         GpioWrite(&StatusLedGreen, 0);
         GpioWrite(&StatusLedBlue, 1);
         break;
      
      case DM_LED_WHITE_E:
         GpioWrite(&StatusLedRed, 1);
         GpioWrite(&StatusLedGreen, 1);
         GpioWrite(&StatusLedBlue, 1);
         break;
      
      case DM_LED_CYAN_E:
         GpioWrite(&StatusLedRed, 0);
         GpioWrite(&StatusLedGreen, 1);
         GpioWrite(&StatusLedBlue, 1);
         break;
      
      case DM_LED_MAGENTA_E:
         GpioWrite(&StatusLedRed, 1);
         GpioWrite(&StatusLedGreen, 0);
         GpioWrite(&StatusLedBlue, 1);
         break;
      
      case DM_LED_AMBER_E:
         GpioWrite(&StatusLedRed, 1);
         GpioWrite(&StatusLedGreen, 1);
         GpioWrite(&StatusLedBlue, 0);
         break;
      
      case DM_LED_OFF_E:
      case DM_LED_STATE_MAX_E:
      default:
         /* turn off all leds */
         GpioWrite(&StatusLedRed, 0);
         GpioWrite(&StatusLedGreen, 0);
         GpioWrite(&StatusLedBlue, 0);
         break;
   }
}

/*************************************************************************************/
/**
* function name   : DM_LedInit
* description     : Routine to initialise the LED module
*
* @param - none
*
* @return - void
*/
void DM_LedInit(void)
{
   DM_LedCurrentPattern = LED_PATTERN_NOT_DEFINED_E,
   DM_LedNextPattern    = LED_PATTERN_NOT_DEFINED_E;
   DM_LedCurrentStep    = 0u;
   LedMutexId           = osMutexCreate(osMutex(LedMutex));
   if(NULL == LedMutexId)
   {
      Error_Handler("Failed to create LED mutex");
   }

   for ( uint32_t pattern = 0; pattern < LED_PATTERNS_MAX_E; pattern++)
   {
      DM_PatternRequested[pattern].active = false;
      DM_PatternRequested[pattern].step = 0;
   }
}

/*************************************************************************************/
/**
* function name   : DM_LedPatternRequest
* description     : Routine to request an LED pattern
*
* @param - const DM_LedPattern_t led_pattern : Requested Pattern
*
* @return - void
*/
void DM_LedPatternRequest(const DM_LedPatterns_t led_pattern)
{
   if ( (LED_PATTERNS_MAX_E    > led_pattern)
      &&(DM_LedCurrentPattern != led_pattern)
      &&(DM_LedNextPattern    != led_pattern))
   {
      /* Check there is no other pending pattern */
      if (LED_PATTERN_NOT_DEFINED_E == DM_LedCurrentPattern)
      {
         /* This is a new pattern */
         DM_LedCurrentPattern = led_pattern;
         
         DM_LedPatternStartTime = osKernelSysTick();
         
         /* Point to the configuration of this pattern */
         const DM_LedPatternConfig_t *const pattern_config_p = &DM_LedPatternConfig[DM_LedCurrentPattern];

         if (false == pattern_config_p->step_on_event)
         {
            /* Start now if it is not an on-event pattern */
            DM_LedSetState(pattern_config_p->state_steps_def[0].led_state);
            
            DM_LedStepStartTime = osKernelSysTick();
         }
         
      }
      else if (LED_PATTERN_NOT_DEFINED_E == DM_LedNextPattern)
      {
         /* This is a new pattern */
         DM_LedNextPattern = led_pattern;
      }
      else
      {
         /* Check that the new pattern has a higher priority */
         if (led_pattern > DM_LedNextPattern)
         {
            /* Overwrite the Next pattern */
            DM_LedNextPattern = led_pattern;
         }
      }
   }
}

/*************************************************************************************/
/**
* function name   : DM_LedPatternRemove
* description     : Routine to remove an LED pattern
*
* @param - const DM_LedPattern_t led_pattern : Requested Pattern
*
* @return - void
*/
void DM_LedPatternRemove(const DM_LedPatterns_t led_pattern)
{
   if ( LED_PATTERNS_MAX_E > led_pattern )
   {
      /* Acquire the mutex to avoid concurrent access */
      if (osOK == osMutexWait(LedMutexId, 1u)) 
      {
         if (DM_LedCurrentPattern == led_pattern)
         {
            /* Reset the Current Pattern Id */
            DM_LedCurrentPattern = LED_PATTERN_NOT_DEFINED_E;

            /* Switch off all the LEDs */
            DM_LedSetState(DM_LED_OFF_E);
         }
         else if (DM_LedNextPattern == led_pattern)
         {
            /* Reset the Next Pattern Id */
            DM_LedNextPattern = LED_PATTERN_NOT_DEFINED_E;
         }
      }
   }
}

/*************************************************************************************/
/**
* function name   : DM_LedTick
* description     : Routine to control the state of the LEDs from an external module
*
* @param - const DM_LedPattern_t led_pattern : Requested Pattern
* @param - const uint16_t step_req : Requested step of the sequence
*
* @return - void
*/
void DM_LedExternalControl(const DM_LedPatterns_t led_pattern, const uint16_t step_req)
{
   /* Acquire the mutex to avoid concurrent access */
   if (osOK == osMutexWait(LedMutexId, 1u))
   {
      /* Call the main LED handler */
      DM_LedHandler(led_pattern, step_req);

      /* Release the Mutex */
      osMutexRelease(LedMutexId);
   }
}

/*************************************************************************************/
/**
* function name   : DM_LedTick
* description     : Periodic routine to update the state of the LEDs
*
* @param - none
*
* @return - void
*/
void DM_LedPeriodicTick(void)
{
   /* Acquire the mutex to avoid concurrent access */
   if (osOK == osMutexWait(LedMutexId, 1u)) 
   {
      /* Call the main LED handler */
      DM_LedHandler(LED_PATTERN_NOT_DEFINED_E, 0);
   
      /* Release the Mutex */
      osMutexRelease(LedMutexId);
   }
}

/*************************************************************************************/
/**
* function name   : DM_LedHandler
* description     : Main handler to manage the LED Patterns and sequences
*
* @param - const DM_LedPattern_t led_pattern : Requested Pattern
* @param - const uint16_t step_req : Requested step of the sequence
* @return - void
*/
static void DM_LedHandler(const DM_LedPatterns_t led_pattern, const uint16_t step_req)
{
   const DM_LedPatternConfig_t *pattern_config_p;

   /* Catch the now time */
   uint32_t now_time = osKernelSysTick();

   /* Wait for the end of the ongoing sequence before loading a new one */
   if (0U == DM_LedCurrentStep)
   {
      if (LED_PATTERNS_MAX_E > DM_LedCurrentPattern)
      {
         /* Point to the configuration of this pattern */
         pattern_config_p = &DM_LedPatternConfig[DM_LedCurrentPattern];

         /* This sequence is terminated and it is not repeatable */
         if (false == pattern_config_p->isRepeated)
         {
            if (LED_NO_TIME_LIMIT != pattern_config_p->max_duration_us)
            {
               if ((now_time - DM_LedPatternStartTime) > osKernelSysTickMicroSec(pattern_config_p->max_duration_us))
               {
                  /* The sequence of this pattern is finished and is not repeatable */
                  DM_LedCurrentPattern = LED_PATTERN_NOT_DEFINED_E;

                  /* Switch off all the LEDs */
                  DM_LedSetState(DM_LED_OFF_E);
               }
            }
         }
         
         /* Check if this pattern didn't exceed its max allowe time */
         if (LED_NO_TIME_LIMIT != pattern_config_p->max_duration_us)
         {
            if ((now_time - DM_LedPatternStartTime) > osKernelSysTickMicroSec(pattern_config_p->max_duration_us))
            {
               /* This pattern has used its maximum allowed time */
               DM_LedCurrentPattern = LED_PATTERN_NOT_DEFINED_E;
               
               /* Switch off all the LEDs */
               DM_LedSetState(DM_LED_OFF_E);
            }
         }
      }
   }

   /* Check if there is a new requestded pattern */
   if (LED_PATTERNS_MAX_E > DM_LedNextPattern)
   {
      /* Check no sequence is ongoing or with a lower priority */
      if ( (LED_PATTERN_NOT_DEFINED_E == DM_LedCurrentPattern)
         ||(DM_LedNextPattern          > DM_LedCurrentPattern))
      {
         /* copy next sequence to be current sequence */
         DM_LedPatterns_t current_pattern = DM_LedCurrentPattern;
         DM_LedCurrentPattern = DM_LedNextPattern;
         DM_LedCurrentStep = 0;
         /* Switch off all the LEDs */
         DM_LedSetState(DM_LED_OFF_E);

         /* Save the last pattern to be resumed later */
         DM_LedNextPattern      = current_pattern;
         DM_LedPatternStartTime = now_time;
         
         /* Point to the configuration of this pattern */
         pattern_config_p = &DM_LedPatternConfig[DM_LedCurrentPattern];

         if (pattern_config_p->step_on_event == false)
         {
            /* Start now if it is not an on-event pattern */
            DM_LedSetState(pattern_config_p->state_steps_def[0].led_state);
            
            DM_LedStepStartTime = now_time;
         }
      }
   }

   if (LED_PATTERNS_MAX_E > DM_LedCurrentPattern)
   {
      const DM_LedPatternConfig_t *const pattern_config_p = &DM_LedPatternConfig[DM_LedCurrentPattern];
      if (pattern_config_p->step_on_event != false)
      {
         /* Only the owner of the pattern can control it */
         if(DM_LedCurrentPattern == led_pattern)
         {
            if (LED_SEQUENCE_LENGTH > step_req)
            {
               DM_LedCurrentStep = step_req;
            }
            
            DM_LedSetState(pattern_config_p->state_steps_def[DM_LedCurrentStep].led_state);
         }
      }
      else
      {
         if (LED_NO_TIME_LIMIT != pattern_config_p->state_steps_def[DM_LedCurrentStep].step_duration_us)
         {
            if ((now_time - DM_LedStepStartTime) > osKernelSysTickMicroSec(pattern_config_p->state_steps_def[DM_LedCurrentStep].step_duration_us))
            {
               /* Go to the next valid step */
               do
               {
                  DM_LedCurrentStep++;
                  DM_LedCurrentStep %= LED_SEQUENCE_LENGTH;
               } while(pattern_config_p->state_steps_def[DM_LedCurrentStep].step_duration_us == 0u);

               DM_LedSetState(pattern_config_p->state_steps_def[DM_LedCurrentStep].led_state);
               
               /* Reload the time value */
               DM_LedStepStartTime = osKernelSysTick();
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* function name   : DM_LedTurnOff
* description     : Routine to turn off leds to save power
*
*
* @param  - none
*
* @return - void
*/
void DM_LedTurnOff(void)
{
   DM_LedSetState(DM_LED_OFF_E);
}

/*************************************************************************************/
/**
* function name   : DM_LedGetCurrentPattern
* description     : Routine to get the current LED activity.
*
*
* @param  - none
*
* @return - current LED pattern
*/
DM_LedPatterns_t DM_LedGetCurrentPattern(void)
{
   return DM_LedCurrentPattern;
}

#endif
