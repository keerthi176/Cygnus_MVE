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
*  File         : MM_TimedEventTask.h
*
*  Description  : Source for the task that handles timed events
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include "cmsis_os.h"

/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "board.h"
#include "stm32l4xx_hal_tim.h"
#include "lptim.h"
#include "CO_SerialDebug.h"
#include "MM_GpioTask.h"
#include "DM_OutputManagement.h"
#include "DM_LED.h"
#include "DM_NVM.h"
#include "MM_ApplicationCommon.h"
#include "MM_TimedEventTask.h"

/* Private Functions Prototypes
*************************************************************************************/
typedef void(*TimerEventFunction_t)(void);

static void TE_FireMCPDebounce(void);
static void TE_FirstAidMCPDebounce(void);
static void TE_SiteNetSounder(void);
static void TE_BatteryTest(void);
static void TE_200HourTest(void);
static void TE_StartTimer(void);
static void TE_StopTimer(void);
static ErrorCode_t TE_SendSensorDataToApplication(const CO_RBUSensorData_t* const pSensorData);
static ErrorCode_t TE_SendOutputToApplication(const CO_ChannelIndex_t systemChannel, const CO_OutputProfile_t profile, const bool activated);
static void TE_StartFireMCPDebounce(const GPIO_PinState mcpState);
static void TE_StartFirstAidMCPDebounce(const GPIO_PinState mcpState);

/* Global Variables
*************************************************************************************/
osThreadId tid_TimedEventTask;
osThreadDef(TE_TimedEventTaskMain, osPriorityNormal, 1, 512);

osSemaphoreDef(TimedEventSem);
osSemaphoreId(TimedEventSemId);

#define USE_TE_MUTEX
#ifdef USE_TE_MUTEX
osMutexId(TE_MutexId) = NULL;
osMutexDef(TE_Mutex);
#endif

extern osPoolId AppPool;
extern osMessageQId(AppQ);

bool gEventTimerRunning = false;

extern uint32_t IOUPeriodicTimerArg;
extern DM_BaseType_t gBaseType;

/* Private Variables
*************************************************************************************/

#define TE_TIMED_EVENT_TICK_PERIOD_US 50000u //50ms

#define FIRE_MCP_DEBOUNCE_COUNT 1         //50ms
#define FIRST_AID_MCP_DEBOUNCE_COUNT 1   //50ms
#define TWO_HUNDRED_HOURS 14400000U
#define ONE_HOUR 72000U

#define MCP_CURRENT_STATE 0
#define MCP_TRANSITION_STATE 1

#define LED_STEP_COUNT 3
#define LED_PATTERN 0
#define LED_STEP 1
#define LED_MAX_STEPS 4

#define TE_BATTERY_TEST_PERIOD 1 //50ms - 1 tick of the TimedEventTask


typedef struct
{
   bool Interrupted;
   GPIO_PinState PinState;
}InterruptTrigger_t;

enum 
{
   TE_FIRE_MCP_E,
   TE_FIRST_AID_MCP_E,
   TE_SITENET_SOUNDER_E,
   TE_BATTERY_TEST_E,
   TE_200_HOUR_TEST_E,
   TE_MAX_E
};


struct
{
   bool Running;
   uint32_t Period;
   uint32_t Count;
   uint32_t Data[3];
   TimerEventFunction_t Function;
}
static TimedEvent[TE_MAX_E] =
{
   [TE_FIRE_MCP_E] =
   {
      .Running = false,
      .Period = FIRE_MCP_DEBOUNCE_COUNT,
      .Count = 0,
      .Data = { GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET },
      .Function = TE_FireMCPDebounce
   },
   
   [TE_FIRST_AID_MCP_E] =
   {
      .Running = false,
      .Period = FIRST_AID_MCP_DEBOUNCE_COUNT,
      .Count = 0,
      .Data = { GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET },
      .Function = TE_FirstAidMCPDebounce
   },
   
   [TE_SITENET_SOUNDER_E] =
   {
      .Running = false,
      .Period = 0,  //indefinite
      .Count = 0,
      .Data = { 0, 0, 0 },
      .Function = TE_SiteNetSounder
   },
   
   [TE_BATTERY_TEST_E] =
   {
      .Running = false,
      .Period = 0,  //indefinite
      .Count = 0,
      .Data = { 0, 0, 0 },
      .Function = TE_BatteryTest
   },
   
   [TE_200_HOUR_TEST_E] =
   {
      .Running = false,
      .Period = TWO_HUNDRED_HOURS,
      .Count = 0,
      .Data = { 0, CO_CHANNEL_BEACON_E, 0 },
      .Function = TE_200HourTest
   }
};

InterruptTrigger_t FireInterrupt;
InterruptTrigger_t FirstAidInterrupt;

/*************************************************************************************/
/**
* MM_TimedEventTaskInit
* Initialisation function for the GPIO Task
*
* @param - None.
*
* @return - ErrorCode_t  // Returns an error code, else SUCCESS_E

*/
ErrorCode_t MM_TimedEventTaskInit(void)
{
   gEventTimerRunning = false;
   FireInterrupt.Interrupted = false;
   FirstAidInterrupt.Interrupted = false;
   
#ifdef USE_TE_MUTEX
   /* Create the Mutex */
   TE_MutexId = osMutexCreate(osMutex(TE_Mutex));
   CO_ASSERT_RET_MSG(NULL != TE_MutexId, ERR_INIT_FAIL_E, "ERROR - Failed to create Timed Event mutex");
   //Release the mutex
   osMutexRelease(TE_MutexId);
#endif
   /* create semaphore */
   TimedEventSemId = osSemaphoreCreate(osSemaphore(TimedEventSem), 1);
   CO_ASSERT_RET_MSG(NULL != TimedEventSemId, ERR_INIT_FAIL_E, "ERROR - Failed to create Timed Event semaphore");

   // Create thread (return on error)
   tid_TimedEventTask = osThreadCreate(osThread(TE_TimedEventTaskMain), NULL);
   CO_ASSERT_RET_MSG(NULL != tid_TimedEventTask, ERR_FAILED_TO_CREATE_THREAD_E, "ERROR - Failed to create Timed Event thread");
   
   return(SUCCESS_E);
}

/*************************************************************************************/
/**
* TE_StartTimer
* Start the timer running
*
* @param - arguments
*
* @return - void

*/
void TE_StartTimer(void)
{
   if ( false == gEventTimerRunning )
   {
      gEventTimerRunning = true;
   }
}

/*************************************************************************************/
/**
* TE_StopTimer
* Stop the timer running
*
* @param - arguments
*
* @return - void

*/
void TE_StopTimer(void)
{
      gEventTimerRunning = false;
}

/*************************************************************************************/
/**
* TE_CanEnterSleepMode
* Stop the timer running
*
* @param - arguments
*
* @return - bool - True if sleep mode is alowwed, false if not

*/
bool TE_CanEnterSleepMode(void)
{
   //OK to sleep if the timer isn't running
   return !gEventTimerRunning;
}

/*************************************************************************************/
/**
* TE_SendSensorDataToApplication
* Send GPIO changes to the application
*
* @param - pSensorData  Structure containg the sensor status
*
* @return - ErrorCode_t SUCCESS_E on success, or error code.

*/
ErrorCode_t TE_SendSensorDataToApplication(const CO_RBUSensorData_t* const pSensorData)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pSensorData )
   {
      //Send the new state to the application.
      // create Fire MCP message and put into App queue
      CO_Message_t* pSignalReq = osPoolAlloc(AppPool);
      if (pSignalReq)
      {
         if ( CO_FIRE_CALLPOINT_E == pSensorData->SensorType )
         {
            pSignalReq->Type = CO_MESSAGE_GENERATE_FIRE_SIGNAL_E;
         }
         else 
         {
            pSignalReq->Type = CO_MESSAGE_GENERATE_ALARM_SIGNAL_E;
         }
         memcpy(pSignalReq->Payload.PhyDataReq.Data, pSensorData, sizeof(CO_RBUSensorData_t));
         osStatus osStat = osMessagePut(AppQ, (uint32_t)pSignalReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pSignalReq);
            result = ERR_QUEUE_OVERFLOW_E;
         }
         else 
         {
            result = SUCCESS_E;
         }
      }
      else 
      {
         result = ERR_NO_RESOURCE_E;
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* TE_SendOutputToApplication
* Send Output change commands to the application
*
* @param - systemChannel  The output channel
* @param - profile        The output profile
* @param - activated      True if the alarm state is 'active'
*
* @return - ErrorCode_t SUCCESS_E on success, or error code.

*/
ErrorCode_t TE_SendOutputToApplication(const CO_ChannelIndex_t systemChannel, const CO_OutputProfile_t profile, const bool activated)
{
   ErrorCode_t result;
   
      //Send the ouput state to the application.
      CO_Message_t* pAlarmSignalReq = osPoolAlloc(AppPool);
      if (pAlarmSignalReq)
      {
         pAlarmSignalReq->Type = CO_MESSAGE_SET_OUTPUT_E;
         CO_OutputData_t output_data;
         output_data.OutputChannel = systemChannel;
         output_data.OutputProfile = profile;
         output_data.OutputsActivated = activated;
         memcpy(pAlarmSignalReq->Payload.PhyDataReq.Data, &output_data, sizeof(CO_OutputData_t));
         osStatus osStat = osMessagePut(AppQ, (uint32_t)pAlarmSignalReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pAlarmSignalReq);
            result = ERR_QUEUE_OVERFLOW_E;
         }
         else 
         {
            result = SUCCESS_E;
         }
      }
      else 
      {
         result = ERR_NO_RESOURCE_E;
      }
   return result;
}

/*************************************************************************************/
/**
* TE_TimedEventTaskMain
* Main function for the Timed Event Task
*
* @param - arguments
*
* @return - void

*/
void TE_TimedEventTaskMain(void const *argument)
{
   static int32_t available_tokens = 0;
   static uint32_t timeout;
   static uint32_t number_of_active_events = 0;
   
   while(true)
   {
      timeout = osWaitForever;
      do
      {
         available_tokens = osSemaphoreWait(TimedEventSemId, timeout);
      
         // Flush all the tokens after the first one
         timeout = 0;
      } while ( available_tokens > 0);

      if ( FireInterrupt.Interrupted )
      {
         FireInterrupt.Interrupted = false;
         if ( false == TimedEvent[TE_FIRE_MCP_E].Running )
         {
            TE_StartFireMCPDebounce(FireInterrupt.PinState);
         }
      }
      if ( FirstAidInterrupt.Interrupted )
      {
         FirstAidInterrupt.Interrupted = false;
         if ( false == TimedEvent[TE_FIRST_AID_MCP_E].Running )
         {
            TE_StartFirstAidMCPDebounce(FirstAidInterrupt.PinState);
         }
      }
      
#ifdef USE_TE_MUTEX
      //Claim the mutex
      if ( osOK == osMutexWait(TE_MutexId, 1) )
      {
#endif
         //Iterate through the timed events servicing any that are running
         number_of_active_events = 0;
         for (uint32_t event = 0; event < TE_MAX_E; event++)
         {
            if ( TimedEvent[event].Running )
            {
               TimedEvent[event].Function();
               number_of_active_events++;
            }
         }
         
         if ( 0 == number_of_active_events )
         {
            TE_StopTimer();
         }
#ifdef USE_TE_MUTEX      
         //Release the mutex
         osMutexRelease(TE_MutexId);
      }
   else 
   {
      CO_PRINT_A_0(DBG_ERROR_E,"TE mutex\r\n");
   }
#endif
   }
}

/*************************************************************************************/
/**
* TE_FireMCPStateChange
* Set a new state for the fire MCP.  This is a mechanism to defer calling TE_StartFireMCPDebounce
* until the timed event thread runs again.  This function is called by the fire ISR which can't
* call TE_StartFireMCPDebounce directly because it contains a mutex.
*
* @param    id.      The ID of the timer that triggered this function to be called.
*
* @return - None.
*/
void TE_FireMCPStateChange(const GPIO_PinState mcpState)
{
   if ( false == FireInterrupt.Interrupted )
   {
      FireInterrupt.Interrupted = true;
      FireInterrupt.PinState = mcpState;
      TE_StartTimer();
   }
}

/*************************************************************************************/
/**
* TE_StartFireMCPDebounce
* Start the Fire MCP debounce process
*
* @param    id.      The ID of the timer that triggered this function to be called.
*
* @return - None.
*/
void TE_StartFireMCPDebounce(const GPIO_PinState mcpState)
{
#ifdef USE_TE_MUTEX
   //Claim the mutex
   if ( osOK == osMutexWait(TE_MutexId, 1) )
   {
#endif
      if ( !TimedEvent[TE_FIRE_MCP_E].Running )
      {
         if ( (uint32_t)mcpState != TimedEvent[TE_FIRE_MCP_E].Data[MCP_CURRENT_STATE] )
         {
            TimedEvent[TE_FIRE_MCP_E].Count = 0;
            TimedEvent[TE_FIRE_MCP_E].Period = FIRE_MCP_DEBOUNCE_COUNT;
            TimedEvent[TE_FIRE_MCP_E].Data[MCP_TRANSITION_STATE] = (uint32_t)mcpState;
            TimedEvent[TE_FIRE_MCP_E].Running = true;
            TE_StartTimer();
         }
      }
      else 
      {
         if ( (uint32_t)mcpState == TimedEvent[TE_FIRE_MCP_E].Data[MCP_CURRENT_STATE] )
         {
            //the state of the call point has reverted before the debounce period.  Stop the debounce process
            TimedEvent[TE_FIRE_MCP_E].Count = 0;
            TimedEvent[TE_FIRE_MCP_E].Running = false;
         }
      }
      
#ifdef USE_TE_MUTEX      
      //Release the mutex
      osMutexRelease(TE_MutexId);
   }
   else 
   {
      CO_PRINT_A_0(DBG_ERROR_E,"FIRE MCP failed to get TE mutex\r\n");
   }
#endif
}

/*************************************************************************************/
/**
* TE_FireMCPDebounce
* Implement the debounce for the Fire MCP.
*
* @param    id.      The ID of the timer that triggered this function to be called.
*
* @return - None.
*/
void TE_FireMCPDebounce(void)
{
   uint32_t current_state = GpioRead(&FireMCP);
   
   if ( current_state == TimedEvent[TE_FIRE_MCP_E].Data[MCP_TRANSITION_STATE] )
   {
      TimedEvent[TE_FIRE_MCP_E].Count++;
      if ( FIRE_MCP_DEBOUNCE_COUNT <= TimedEvent[TE_FIRE_MCP_E].Count )
      {
         //The change of input has remained consistent.
         
         //publish the new state
         DM_InputMonitorState_t FireCallPointState = (current_state == GPIO_PIN_SET ? IN_MON_IDLE_E : IN_MON_ACTIVE_E);
         DM_SetFireCallPointState(FireCallPointState);
         
      
         //Send the new state to the application.
         CO_RBUSensorData_t sensorData;
         sensorData.SensorType = CO_FIRE_CALLPOINT_E;
         sensorData.SensorValue = ( FireCallPointState == IN_MON_ACTIVE_E ? GPIO_ACTIVE : GPIO_IDLE);
         sensorData.AlarmState = ( FireCallPointState == IN_MON_ACTIVE_E ? 1 : 0);
         sensorData.RUChannelIndex = CO_CHANNEL_FIRE_CALLPOINT_E;
         
         if ( SUCCESS_E == TE_SendSensorDataToApplication( &sensorData ) )
         {
            //The message went.  Restore for next change.
            TimedEvent[TE_FIRE_MCP_E].Count = 0;
            TimedEvent[TE_FIRE_MCP_E].Running = false;
            TimedEvent[TE_FIRE_MCP_E].Data[MCP_CURRENT_STATE] = current_state;
         }
      }
   }
   else 
   {
      //The input has changed back.  Stop the debounce process
      TimedEvent[TE_FIRE_MCP_E].Count = 0;
      TimedEvent[TE_FIRE_MCP_E].Running = false;
   }
}

/*************************************************************************************/
/**
* TE_FirstAidMCPStateChange
* Set a new state for the first aid MCP.  This is a mechanism to defer calling TE_StartFireMCPDebounce
* until the timed event thread runs again.  This function is called by the fire ISR which can't
* call TE_StartFireMCPDebounce directly because it contains a mutex.
*
* @param    id.      The ID of the timer that triggered this function to be called.
*
* @return - None.
*/
void TE_FirstAidMCPStateChange(const GPIO_PinState mcpState)
{
   if ( false == FirstAidInterrupt.Interrupted )
   {
      FirstAidInterrupt.Interrupted = true;
      FirstAidInterrupt.PinState = mcpState;
      TE_StartTimer();
   }
}

/*************************************************************************************/
/**
* TE_StartFirstAidMCPDebounce
* Start the First Aid MCP debounce process.
*
* @param    mcpState      The state of the MCP to validate
*
* @return - None.
*/
void TE_StartFirstAidMCPDebounce(const GPIO_PinState mcpState)
{
#ifdef USE_TE_MUTEX
   //Claim the mutex
   if ( osOK == osMutexWait(TE_MutexId, 1) )
   {
#endif
      if ( !TimedEvent[TE_FIRST_AID_MCP_E].Running )
      {
         if ( (uint32_t)mcpState != TimedEvent[TE_FIRST_AID_MCP_E].Data[MCP_CURRENT_STATE] )
         {
            TimedEvent[TE_FIRST_AID_MCP_E].Count = 0;
            TimedEvent[TE_FIRST_AID_MCP_E].Period = FIRST_AID_MCP_DEBOUNCE_COUNT;
            TimedEvent[TE_FIRST_AID_MCP_E].Data[MCP_TRANSITION_STATE] = (uint32_t)mcpState;
            TimedEvent[TE_FIRST_AID_MCP_E].Running = true;
            TE_StartTimer();
         }
      }
      else 
      {
         if ( (uint32_t)mcpState == TimedEvent[TE_FIRST_AID_MCP_E].Data[MCP_CURRENT_STATE] )
         {
            //the state of the call point has reverted before the debounce period.  Stop the debounce process
            TimedEvent[TE_FIRST_AID_MCP_E].Count = 0;
            TimedEvent[TE_FIRST_AID_MCP_E].Running = false;
         }
      }
#ifdef USE_TE_MUTEX      
      //Release the mutex
      osMutexRelease(TE_MutexId);
   }
   else 
   {
      CO_PRINT_A_0(DBG_ERROR_E,"FIRST AID MCP failed to get TE mutex\r\n");
   }
#endif
}

/*************************************************************************************/
/**
* TE_FirstAidMCPDebounce
* Implement the debounce for the First Aid MCP.
*
* @param    None.
*
* @return - None.
*/
void TE_FirstAidMCPDebounce(void)
{
   uint32_t current_state = GpioRead(&FirstAidMCP);
   
   if ( current_state == TimedEvent[TE_FIRST_AID_MCP_E].Data[MCP_TRANSITION_STATE] )
   {
      TimedEvent[TE_FIRST_AID_MCP_E].Count++;
      if ( FIRST_AID_MCP_DEBOUNCE_COUNT <= TimedEvent[TE_FIRST_AID_MCP_E].Count )
      {
         //The change of input has remained consistent.
         
         //publish the new state
         DM_InputMonitorState_t FirstAidCallPointState = (current_state == GPIO_PIN_SET ? IN_MON_IDLE_E : IN_MON_ACTIVE_E);
         DM_SetFirstAidCallPointState(FirstAidCallPointState);
      
         //Send the new state to the application.
         CO_RBUSensorData_t sensorData;
         sensorData.SensorType = CO_FIRST_AID_CALLPOINT_E;
         sensorData.SensorValue = ( FirstAidCallPointState == IN_MON_ACTIVE_E ? GPIO_ACTIVE : GPIO_IDLE);
         sensorData.AlarmState = ( FirstAidCallPointState == IN_MON_ACTIVE_E ? 1 : 0);
         sensorData.RUChannelIndex = CO_CHANNEL_MEDICAL_CALLPOINT_E;
         
         if ( SUCCESS_E == TE_SendSensorDataToApplication( &sensorData ) )
         {
            //The message went.  Restore for next change.
            TimedEvent[TE_FIRST_AID_MCP_E].Count = 0;
            TimedEvent[TE_FIRST_AID_MCP_E].Running = false;
            TimedEvent[TE_FIRST_AID_MCP_E].Data[MCP_CURRENT_STATE] = current_state;
         }
      }
   }
   else 
   {
      //The input has changed back.  Stop the debounce process
      TimedEvent[TE_FIRST_AID_MCP_E].Count = 0;
      TimedEvent[TE_FIRST_AID_MCP_E].Running = false;
   }
}


/*************************************************************************************/
/**
* TE_SetSitenetSounderOutput
* Start/stop the SiteNet sounder output sequence.
*
* @param    active      True to start the sitenet sounder, false to stop it.
*
* @return - None.
*/
void TE_SetSitenetSounderOutput(const bool active)
{
#ifdef USE_TE_MUTEX
   //Claim the mutex
   if ( osOK == osMutexWait(TE_MutexId, 10) )
   {
#endif
      if ( active )
      {
         if ( !TimedEvent[TE_SITENET_SOUNDER_E].Running )
         {
               TimedEvent[TE_SITENET_SOUNDER_E].Count = 0;
               TimedEvent[TE_SITENET_SOUNDER_E].Running = true;
               TE_StartTimer();
         }
      }
      else 
      {
         //Stop the sounder sequence
         TimedEvent[TE_SITENET_SOUNDER_E].Count = 0;
         TimedEvent[TE_SITENET_SOUNDER_E].Running = false;
      }
#ifdef USE_TE_MUTEX      
      //Release the mutex
      osMutexRelease(TE_MutexId);
   }
#endif
}

/*************************************************************************************/
/**
* TE_SiteNetSounder
* Call into the output manager to run the SiteNet sounder output sequence.
*
* @param    None.
*
* @return - None.
*/
void TE_SiteNetSounder(void)
{
   DM_OP_SiteNetCombinedSounderCycle(TimedEvent[TE_SITENET_SOUNDER_E].Count);
   TimedEvent[TE_SITENET_SOUNDER_E].Count++;
}

/*************************************************************************************/
/**
* TE_AnalogueInput
* Poll the IOU inputs and check the NVM for updates to the poll period.
*
* @param    None.
*
* @return - None.
*/
void TE_StartBatteryTest(void)
{
   if ( false == TimedEvent[TE_BATTERY_TEST_E].Running )
   {
      TimedEvent[TE_BATTERY_TEST_E].Count = 0;     //counts the period between readings
      TimedEvent[TE_BATTERY_TEST_E].Data[0] = 0;   //counts the number of readings
      TimedEvent[TE_BATTERY_TEST_E].Running = true;
#ifdef USE_TE_MUTEX
      //Claim the mutex
      if ( osOK == osMutexWait(TE_MutexId, 1) )
      {
         TE_StartTimer();
         //Release the mutex
         osMutexRelease(TE_MutexId);
      }
#endif
   }
}

/*************************************************************************************/
/**
* TE_BatteryTest
* Return the polling period for the analogue inputs (IOU).
*
* @param    None.
*
* @return - uint32_t    The polling period in milliseconds.
*/
void TE_BatteryTest(void)
{
   if ( TimedEvent[TE_BATTERY_TEST_E].Running )
   {
      TimedEvent[TE_BATTERY_TEST_E].Count++;
      if ( TE_BATTERY_TEST_PERIOD <= TimedEvent[TE_BATTERY_TEST_E].Count )
      {
         TimedEvent[TE_BATTERY_TEST_E].Count = 0;
         if ( DM_BAT_NUMBER_OF_BATTERY_READINGS > TimedEvent[TE_BATTERY_TEST_E].Data[0] )
         {
            //Time to take a battery reading. Send a message to the App.
            osStatus osStat = osErrorOS;
            CO_Message_t *pCmdReq = NULL;
            pCmdReq = osPoolAlloc(AppPool);
            if (pCmdReq)
            {
               pCmdReq->Type = CO_MESSAGE_TIMER_EVENT_E;
               uint32_t* pData = (uint32_t*)pCmdReq->Payload.PhyDataReq.Data;
               *pData = DM_BAT_TIMER_ID;
               osStat = osMessagePut(AppQ, (uint32_t)pCmdReq, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCmdReq);
               }
               else 
               {
                  //Increment the number of readings taken
                  TimedEvent[TE_BATTERY_TEST_E].Data[0]++;
               }
            }
         }
         else
         {
            //All readings have been taken.  stop the timer
            TimedEvent[TE_BATTERY_TEST_E].Running = false;
         }
      }
   }
}

/*************************************************************************************/
/**
* TE_200HourTestEnable
* Start/stop the 200 hour beacon test.
*
* @param    runTest  True to start the test.  False to stop it.
*
* @return - None.
*/
void TE_200HourTestEnable(const bool runTest, const CO_ChannelIndex_t channel)
{
#ifdef USE_TE_MUTEX
   //Claim the mutex
   if ( osOK == osMutexWait(TE_MutexId, 1) )
   {
#endif
      if ( runTest )
      {
         if ( false == TimedEvent[TE_200_HOUR_TEST_E].Running )
         {
            //We are starting the test
            TimedEvent[TE_200_HOUR_TEST_E].Count = 0;
            TimedEvent[TE_200_HOUR_TEST_E].Data[0] = 0;
            TimedEvent[TE_200_HOUR_TEST_E].Data[1] = channel;
            TimedEvent[TE_200_HOUR_TEST_E].Data[2] = ONE_HOUR - 1;//counter for 1 hour cycle
            TimedEvent[TE_200_HOUR_TEST_E].Running = true;
            TE_StartTimer();
         }
      }
      else 
      {
         //We are stopping the test.
         //To ensure that the test ends cleanly, set the
         //count to an expired value and allow the test function
         //to execute to completion
         TimedEvent[TE_200_HOUR_TEST_E].Count = TWO_HUNDRED_HOURS;
      }
#ifdef USE_TE_MUTEX      
      //Release the mutex
      osMutexRelease(TE_MutexId);
   }
#endif
}

/*************************************************************************************/
/**
* TE_200HourTest
* Flash the beacon for 1 hour on and 1hour off for 200 hours.
* The timer alls this function once every second.
* TimedEvent[TE_200_HOUR_TEST_E].Data[0] = 0 for channel 'off' and 1 for channel 'on'
* TimedEvent[TE_200_HOUR_TEST_E].Data[1] = the output channel to switch
* TimedEvent[TE_200_HOUR_TEST_E].Data[2] = the counter for the 1 hour cycle.
*
* @param    None.
*
* @return - None.
*/
void TE_200HourTest(void)
{
   TimedEvent[TE_200_HOUR_TEST_E].Data[2]++;
   if ( ONE_HOUR <= TimedEvent[TE_200_HOUR_TEST_E].Data[2] )
   {
      if ( 0 == TimedEvent[TE_200_HOUR_TEST_E].Data[0] )
      {
         if ( TimedEvent[TE_200_HOUR_TEST_E].Data[1] & TEST_200HR_CHANNEL_SOUNDER )
         {
            if( SUCCESS_E == TE_SendOutputToApplication(CO_CHANNEL_SOUNDER_E, CO_PROFILE_FIRE_E, true) )
            {
               TimedEvent[TE_200_HOUR_TEST_E].Data[0] = 1;
               TimedEvent[TE_200_HOUR_TEST_E].Data[2] = 0;
            }
         }
         if ( TimedEvent[TE_200_HOUR_TEST_E].Data[1] & TEST_200HR_CHANNEL_BEACON )
         {
            if( SUCCESS_E == TE_SendOutputToApplication(CO_CHANNEL_BEACON_E, CO_PROFILE_FIRE_E, true) )
            {
               TimedEvent[TE_200_HOUR_TEST_E].Data[0] = 1;
               TimedEvent[TE_200_HOUR_TEST_E].Data[2] = 0;
            }
         }
         if ( TimedEvent[TE_200_HOUR_TEST_E].Data[1] & TEST_200HR_CHANNEL_SOUND_VISUAL_INDICATOR_COMBINED )
         {
            if( SUCCESS_E == TE_SendOutputToApplication(CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E, CO_PROFILE_FIRE_E, true) )
            {
               TimedEvent[TE_200_HOUR_TEST_E].Data[0] = 1;
               TimedEvent[TE_200_HOUR_TEST_E].Data[2] = 0;
            }
         }
         if ( TimedEvent[TE_200_HOUR_TEST_E].Data[1] & TEST_200HR_CHANNEL_VISUAL_INDICATOR )
         {
            if( SUCCESS_E == TE_SendOutputToApplication(CO_CHANNEL_VISUAL_INDICATOR_E, CO_PROFILE_FIRE_E, true) )
            {
               TimedEvent[TE_200_HOUR_TEST_E].Data[0] = 1;
               TimedEvent[TE_200_HOUR_TEST_E].Data[2] = 0;
            }
         }
      }
      else 
      {
         if ( TimedEvent[TE_200_HOUR_TEST_E].Data[1] & TEST_200HR_CHANNEL_SOUNDER )
         {
            if( SUCCESS_E == TE_SendOutputToApplication(CO_CHANNEL_SOUNDER_E, CO_PROFILE_FIRE_E, false) )
            {
               TimedEvent[TE_200_HOUR_TEST_E].Data[0] = 0;
               TimedEvent[TE_200_HOUR_TEST_E].Data[2] = 0;
            }
         }
         if ( TimedEvent[TE_200_HOUR_TEST_E].Data[1] & TEST_200HR_CHANNEL_BEACON )
         {
            if( SUCCESS_E == TE_SendOutputToApplication(CO_CHANNEL_BEACON_E, CO_PROFILE_FIRE_E, false) )
            {
               TimedEvent[TE_200_HOUR_TEST_E].Data[0] = 0;
               TimedEvent[TE_200_HOUR_TEST_E].Data[2] = 0;
            }
         }
         if ( TimedEvent[TE_200_HOUR_TEST_E].Data[1] & TEST_200HR_CHANNEL_SOUND_VISUAL_INDICATOR_COMBINED )
         {
            if( SUCCESS_E == TE_SendOutputToApplication(CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E, CO_PROFILE_FIRE_E, false) )
            {
               TimedEvent[TE_200_HOUR_TEST_E].Data[0] = 0;
               TimedEvent[TE_200_HOUR_TEST_E].Data[2] = 0;
            }
         }
         if ( TimedEvent[TE_200_HOUR_TEST_E].Data[1] & TEST_200HR_CHANNEL_VISUAL_INDICATOR )
         {
            if( SUCCESS_E == TE_SendOutputToApplication(CO_CHANNEL_VISUAL_INDICATOR_E, CO_PROFILE_FIRE_E, false) )
            {
               TimedEvent[TE_200_HOUR_TEST_E].Data[0] = 0;
               TimedEvent[TE_200_HOUR_TEST_E].Data[2] = 0;
            }
         }
      }
   }
   
   //Has the 200 hours expired?
   TimedEvent[TE_200_HOUR_TEST_E].Count++;
   if ( TimedEvent[TE_200_HOUR_TEST_E].Count >= TimedEvent[TE_200_HOUR_TEST_E].Period )
   {
      //make sure the test ends with the beacon off
      if( SUCCESS_E == TE_SendOutputToApplication((CO_ChannelIndex_t)TimedEvent[TE_200_HOUR_TEST_E].Data[1], CO_PROFILE_FIRE_E, false) )
      {
         TimedEvent[TE_200_HOUR_TEST_E].Data[0] = 0;
         TimedEvent[TE_200_HOUR_TEST_E].Data[2] = 0;
         TimedEvent[TE_200_HOUR_TEST_E].Running = false;
         
         if ( TimedEvent[TE_200_HOUR_TEST_E].Data[1] & TEST_200HR_CHANNEL_SOUNDER )
         {
            TE_SendOutputToApplication(CO_CHANNEL_SOUNDER_E, CO_PROFILE_FIRE_E, false);
         }
         if ( TimedEvent[TE_200_HOUR_TEST_E].Data[1] & TEST_200HR_CHANNEL_BEACON )
         {
            TE_SendOutputToApplication(CO_CHANNEL_BEACON_E, CO_PROFILE_FIRE_E, false);
         }
         if ( TimedEvent[TE_200_HOUR_TEST_E].Data[1] & TEST_200HR_CHANNEL_SOUND_VISUAL_INDICATOR_COMBINED )
         {
            TE_SendOutputToApplication(CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E, CO_PROFILE_FIRE_E, false);
         }
         if ( TimedEvent[TE_200_HOUR_TEST_E].Data[1] & TEST_200HR_CHANNEL_VISUAL_INDICATOR )
         {
            TE_SendOutputToApplication(CO_CHANNEL_VISUAL_INDICATOR_E, CO_PROFILE_FIRE_E, false);
         }
      }
   }
}

/*************************************************************************************/
/**
* TE_TimedEventTimerTick
* Callback function for the periodic timer.
*
* @param    None.
*
* @return - None.
*/
void TE_TimedEventTimerTick(void)
{
   static uint32_t last_tick_time = 0;
   
   uint32_t now_time = osKernelSysTick();
   
   if ( gEventTimerRunning )
   {
      if ((now_time - last_tick_time) > osKernelSysTickMicroSec(TE_TIMED_EVENT_TICK_PERIOD_US))
      {
         osSemaphoreRelease(TimedEventSemId);
         last_tick_time = now_time;
      }
   }
}


/*************************************************************************************/
/**
* TE_EnterSleepMode
* Function to shut down all timed events for minimum current consumption.
*
* @param    None.
*
* @return - None.
*/
void TE_EnterSleepMode(void)
{
   //stop the MCP debounce, sitenet sounder, analogue input and 200 hour tests, if they are active.
   TimedEvent[TE_FIRE_MCP_E].Running = false;
   TimedEvent[TE_FIRST_AID_MCP_E].Running = false;
   
   //Stop the SiteNet sounder.
   TE_SetSitenetSounderOutput(false);
   
   //stop the 200 hour test
   if ( TimedEvent[TE_200_HOUR_TEST_E].Running )
   {
      //Move the 200 hour counter to the last count and let it do its own clean-up
      TimedEvent[TE_200_HOUR_TEST_E].Count = TimedEvent[TE_200_HOUR_TEST_E].Period;
   }
}

