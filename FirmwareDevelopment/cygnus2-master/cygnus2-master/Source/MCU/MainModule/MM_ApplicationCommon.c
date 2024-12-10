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
*  File         : MM_ApplicationCommon.c
*
*  Description  : Functions that are common to NCU and RBU applications.
*
                  Includes functions for managing the child records at the application level.
*                 This deals with ensuring that each child's output state corresponds
*                 to the last output signal.
*                 Not to be confused with the child management in the session manager,
*                 which is for a different purpose (maintaining mesh links).
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>



/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "stm32l4xx.h"
#include "CO_ErrorCode.h"
#include "MM_RBUApplicationTask.h"
#include "MM_MeshAPI.h"
#include "DM_SerialPort.h"
#include "MM_CommandProcessor.h"
#include "MM_CIEQueueManager.h"
#ifdef USE_NEW_HEAD_INTERFACE
#include "MM_PluginInterfaceTask.h"
#else
#include "MM_HeadInterfaceTask.h"
#endif
#include "CFG_Device_cfg.h"
#include "MC_MacConfiguration.h"
#include "DM_OutputManagement.h"
#include "MM_TimedEventTask.h"
#include "DM_LED.h"
#include "DM_MCUPower.h"
#include "MM_ApplicationCommon.h"




osTimerDef(PeriodicTimer, PeriodicTimerCallback);
osTimerDef(IOUPeriodicTimer, PeriodicTimerCallback);
uint32_t PeriodicTimerArg = 1;
static osTimerId PeriodicTimerID;
int32_t gTimerCommandInProgress = 0;

static osTimerId IouPeriodicTimerID;
uint32_t IOUPeriodicTimerArg = 2;
int32_t gIOUTimerCommandInProgress = 0;

uint16_t gLastCommandedChannel = 0;
uint8_t gLastCommandedOutputProfile = 0;
uint8_t gLastCommandedOutputActive = 0;
uint16_t gLastCommandedOutputDuration = 0;
uint8_t gLastCommandedSilenceableMask = 0;
uint8_t gLastCommandedUnsilenceableMask = 0;
uint8_t gLastCommandedDelayMask = 0;

uint16_t gNodeAddress;
uint32_t gZoneNumber;
extern const char ParamCommandMap[PARAM_TYPE_MAX_E][5];

CO_State_t gCurrentDeviceState = STATE_IDLE_E;


ApplicationChildArray_t childList;

const char MESH_EVENT_STR[][64] = 
{
   "No event ",
   "Added Child node ",
   "Dropped Child node ",
   "Added Primary Parent node ",
   "Dropped Primary Parent node ",
   "Added Secondary Parent node ",
   "Dropped Secondary Parent node ",
   "Added Primary Tracking node ",
   "Dropped Primary Tracking node ",
   "Added Secondary Tracking node ",
   "Dropped Secondary Tracking node ",
   "Promoted Secondary Parent to Primary - node = ",
   "Promoted Primary Tracking node to Secondary Parent - node = ",
   "Reporting Associated Nodes ",
   "Zone Number Changed ",
   "State Changed "
};


/* External declarations
*************************************************************************************/
extern osMessageQId(AppQ);       // Queue to the application
extern osPoolId AppPool;         // Pool definition for use with AppQ.
extern DM_BaseType_t gBaseType;
extern uint16_t gDeviceCombination;
extern Gpio_t SndrBeaconOn;

/* Public Functions
*************************************************************************************/
/*************************************************************************************/
/**
* GetNextHandle
* Generates the message tag that is required by the Mesh API for mapping callbacks
* to the message that was sent.
*
* @param - void
*
* @return - void
*/
uint32_t GetNextHandle(void)
{
   static uint32_t handle = 0;
   handle++;
   /* reserve handle=0 for non-application sources */
   if ( 0 == handle )
   {
      handle++;
   }
   return handle;
}

/*************************************************************************************/
/**
* GetNextTransactionID
* Generates the TransactionID tag that is included in command messages, range 0-7.
*
* @param - void
*
* @return - void
*/
uint8_t GetNextTransactionID(void)
{
   static uint8_t transaction = 0;
   transaction++;
   transaction &= 0x07;
   return transaction;
}

/*************************************************************************************/
/**
* GetCurrentDeviceState
* Returns the current network state of the device.
*
* @param - void
*
* @return - void
*/
CO_State_t GetCurrentDeviceState(void)
{
   return gCurrentDeviceState;
}

/*************************************************************************************/
/**
* MM_ApplicationInitialiseChildArray
* Function to initialise the childList array.
*
* @param - None.
*
* @return - void
*/
void MM_ApplicationInitialiseChildArray()
{
   childList.numberOfCorrectionCycles = 0;
   childList.shortFrameCount = 0;
   for (uint16_t index = 0; index < MAX_CHILDREN_PER_PARENT; index++)
   {
      childList.childRecord[index].isPrimaryChild = false;
      childList.childRecord[index].isMuted = false;
      childList.childRecord[index].nodeID = 0;
      childList.childRecord[index].stateQueryHandle = 0;
      childList.childRecord[index].outputChannel = 0;
      childList.childRecord[index].outputProfile = 0;
      childList.childRecord[index].outputValue = 0;
      childList.childRecord[index].outputDuration = 0;
      childList.childRecord[index].lastReportedChannel = 0;
      childList.childRecord[index].lastReportedProfile = 0;
      childList.childRecord[index].lastReportedValue = 0;
      childList.childRecord[index].lastReportedDuration = 0;
   }
}
   
/*************************************************************************************/
/**
* MM_ApplicationAddChild
* Function to add a new child to the childList array.
*
* @param - nodeID    - Network Address of the new child.
* @param - isPrimary - True if the child selected this node as primary parent.
*
* @return - void
*/
void MM_ApplicationAddChild(const uint16_t nodeID, const bool isPrimary)
{
   uint16_t firstFreeIndex = MAX_CHILDREN_PER_PARENT;
   bool recordFound = false;
   
   for ( uint16_t index = 0; index < MAX_CHILDREN_PER_PARENT; index++)
   {
      if ( (MAX_CHILDREN_PER_PARENT == firstFreeIndex) && (0 == childList.childRecord[index].nodeID) )
      {
         firstFreeIndex = index;
      }
      else if( nodeID == childList.childRecord[index].nodeID )
      {
         recordFound = true;
         /* the sending child may have promoted this node to primary parent, so record the change */
         if ( 0 == isPrimary )
         {
            childList.childRecord[index].isPrimaryChild = false;
         }
         else 
         {
            childList.childRecord[index].isPrimaryChild = true;
         }
         break;
      }
   }
   
   if ( false == recordFound && (MAX_CHILDREN_PER_PARENT > firstFreeIndex))
   {
      /* new child */
      childList.childRecord[firstFreeIndex].nodeID = nodeID;     
      childList.childRecord[firstFreeIndex].stateQueryHandle = 0;
      childList.childRecord[firstFreeIndex].isMuted = false;
      childList.childRecord[firstFreeIndex].outputChannel = 0;
      childList.childRecord[firstFreeIndex].outputProfile = 0;
      childList.childRecord[firstFreeIndex].outputValue = 0;
      childList.childRecord[firstFreeIndex].outputDuration = 0;
      childList.childRecord[firstFreeIndex].lastReportedChannel = 0;
      childList.childRecord[firstFreeIndex].lastReportedProfile = 0;
      childList.childRecord[firstFreeIndex].lastReportedValue = 0;
      childList.childRecord[firstFreeIndex].lastReportedDuration = 0;
      
      if ( 0 == isPrimary )
      {
         childList.childRecord[firstFreeIndex].isPrimaryChild = false;
      }
      else 
      {
         childList.childRecord[firstFreeIndex].isPrimaryChild = true;
      }
   }
}

/*************************************************************************************/
/**
* MM_ApplicationDropChild
* Function to remove a child from the childList array.
*
* @param - nodeID - Network Address of the new child.
*
* @return - void
*/
void MM_ApplicationDropChild(const uint16_t nodeID)
{
   for ( uint16_t index = 0; index < MAX_CHILDREN_PER_PARENT; index++)
   {
      if( nodeID == childList.childRecord[index].nodeID )
      {
         /* found child */
         childList.childRecord[index].isPrimaryChild = false;
         childList.childRecord[index].isMuted = false;
         childList.childRecord[index].nodeID = 0;
         childList.childRecord[index].stateQueryHandle = 0;
         childList.childRecord[index].outputChannel = 0;
         childList.childRecord[index].outputProfile = 0;
         childList.childRecord[index].outputValue = 0;
         childList.childRecord[index].outputDuration = 0;
         childList.childRecord[index].lastReportedChannel = 0;
         childList.childRecord[index].lastReportedProfile = 0;
         childList.childRecord[index].lastReportedValue = 0;
         childList.childRecord[index].lastReportedDuration = 0;
         break;
      }
   }
}




/*************************************************************************************/
/**
* MM_ApplicationChildIsAMemberOfZone
* Function for determining whether a child is in a particular zone.
*
* @param ChildIndex        The index of the child in childList[].
* @param Zone              The zone to check the child for.
*
* @return - True if the child is a member of the zone, false otherwise
*/
bool MM_ApplicationChildIsAMemberOfZone(const uint16_t ChildIndex, const uint16_t Zone)
{
   bool chid_is_in_zone = false;
   if ( (ZONE_GLOBAL == Zone) || (Zone == childList.childRecord[ChildIndex].zone))
   {
      chid_is_in_zone = true;
   }
   
   return chid_is_in_zone;
}

/*************************************************************************************/
/**
* MM_ApplicationProcessOutputMessage
* Function for processing a received output command message.
*
* @param Zone              The zone where the output should be applied.
* @param OutputChannel     The output channel.
* @param OutputProfile     The output profile.
* @param OutputsActivated  The output value to be applied.
* @param OutputDuration    The time that the output should remain active.
*
* @return - void
*/
void MM_ApplicationProcessOutputMessage(const uint16_t destination, const uint16_t Zone,const uint8_t OutputChannel, const uint8_t OutputProfile, const uint16_t OutputsActivated, const uint16_t OutputDuration)
{
   /* if the command is addressed to this zone, record the details */
   if( (Zone == ZONE_GLOBAL) || (Zone == CFG_GetZoneNumber()) )
   {
      gLastCommandedChannel = OutputChannel;
      gLastCommandedOutputProfile = OutputProfile;
      gLastCommandedOutputActive = OutputsActivated;
      gLastCommandedOutputDuration = OutputDuration;
   }
   
   /* update child records with their last commanded state*/
   for ( uint32_t childIndex = 0; (childIndex < MAX_CHILDREN_PER_PARENT); childIndex++ )
   {
      if ( ( 0 != childList.childRecord[childIndex].nodeID) && 
           ( childList.childRecord[childIndex].isPrimaryChild || ADDRESS_NCU == MC_GetNetworkAddress()) )
      {
         if ( MM_ApplicationChildIsAMemberOfZone(childIndex,Zone) &&
               (( ADDRESS_GLOBAL == destination) || (childList.childRecord[childIndex].nodeID == destination)) )
         {
            childList.childRecord[childIndex].outputValue = OutputsActivated;
            childList.childRecord[childIndex].outputChannel = OutputChannel;
            childList.childRecord[childIndex].outputProfile = OutputProfile;
            childList.childRecord[childIndex].outputDuration = OutputDuration;
            CO_PRINT_B_5(DBG_INFO_E, "Updated expected child output - node=%d channel=%d profile=%d output=%d duration=%d\r\n",
            childList.childRecord[childIndex].nodeID,
            OutputChannel,
            OutputProfile,
            OutputsActivated,
            OutputDuration
            );
         }
      }      
   }
   
}


/*************************************************************************************/
/**
* MM_ApplicationUpdateDeviceState
* Function for recording the current device state.
* The value passed across the MeshAPI is a uint32_t which needs to be converted
* back to a CO_State_t
*
* @param newState    The integer value of the device state.
*
* @return - void
*/
void MM_ApplicationUpdateDeviceState(const uint32_t newState)
{
   switch ( newState )
   {
      case STATE_IDLE_E:
         gCurrentDeviceState = STATE_IDLE_E;
         CO_PRINT_B_0(DBG_INFO_E,"APP : Device state Set to IDLE\r\n");
      break;
      case STATE_CONFIG_SYNC_E:
         gCurrentDeviceState = STATE_CONFIG_SYNC_E;
         CO_PRINT_B_0(DBG_INFO_E,"APP : Device state Set to CONFIG_SYNC\r\n");
      break;
      case STATE_CONFIG_FORM_E:
         gCurrentDeviceState = STATE_CONFIG_FORM_E;
         CO_PRINT_B_0(DBG_INFO_E,"APP : Device state Set to CONFIG_FORM\r\n");
      break;
      case STATE_ACTIVE_E:
         gCurrentDeviceState = STATE_ACTIVE_E;
         CO_PRINT_B_0(DBG_INFO_E,"APP : Device state Set to ACTIVE\r\n");
      break;
      case STATE_TEST_MODE_E:
         gCurrentDeviceState = STATE_TEST_MODE_E;
      CO_PRINT_B_0(DBG_INFO_E,"APP : Device state Set to TEST_MODE\r\n");
      break;
      case STATE_SLEEP_MODE_E:
         gCurrentDeviceState = STATE_SLEEP_MODE_E;
      CO_PRINT_B_0(DBG_INFO_E,"APP : Device state Set to SLEEP MODE\r\n");
      break;
      default:
         CO_PRINT_B_1(DBG_INFO_E,"APP : Device state change - unknown state %d\r\n", newState);
         break;
   }
}

/*************************************************************************************/
/**
* StartPeriodicTimer
* Callback function for the periodic timer.  Triggers every RBU_PERIODIC_TIMER_PERIOD_MS
* milliseconds.
*
* @param    None.
*
* @return - None.
*/
void StartPeriodicTimer()
{
   PeriodicTimerID = osTimerCreate(osTimer(PeriodicTimer),osTimerPeriodic, (void*)PeriodicTimerArg);
   
   if ( NULL != PeriodicTimerID )
   {
      osTimerStart(PeriodicTimerID, RBU_PERIODIC_TIMER_PERIOD_MS);
   }
   
   if ( BASE_IOU_E == gBaseType )
   {
      IouPeriodicTimerID  = osTimerCreate(osTimer(IOUPeriodicTimer),osTimerPeriodic, (void*)IOUPeriodicTimerArg);
      if ( NULL != IouPeriodicTimerID )
      {
         osTimerStart(IouPeriodicTimerID, IUO_PERIODIC_TIMER_PERIOD_MS);
      }
   }
}

/*************************************************************************************/
/**
* PeriodicTimerCallback
* Callback function for the periodic timer.  Triggers every RBU_PERIODIC_TIMER_PERIOD_MS
* milliseconds.  Also triggered by the TimedEventtask to initiate IOU checks
*
* @param    id.      The ID of the timer that triggered this function to be called.
*
* @return - None.
*/
void PeriodicTimerCallback(void const *id)
{
   uint32_t timer_id = 0;
   
   if ( NULL != id )
   {
      timer_id = (uint32_t)id;
   }
   
   if ( ( 0 == gTimerCommandInProgress && timer_id == PeriodicTimerArg ) ||
        (0 == gIOUTimerCommandInProgress && timer_id == IOUPeriodicTimerArg ) )
   {
      /* defer the timer action to the main application trhead */
      osStatus osStat = osErrorOS;
      CO_Message_t *pCmdReq = NULL;
      pCmdReq = osPoolAlloc(AppPool);
      if (pCmdReq)
      {
         pCmdReq->Type = CO_MESSAGE_TIMER_EVENT_E;
         uint32_t* pData = (uint32_t*)pCmdReq->Payload.PhyDataReq.Data;
         *pData = timer_id;
         osStat = osMessagePut(AppQ, (uint32_t)pCmdReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pCmdReq);
         }
         else 
         {
            if ( timer_id == PeriodicTimerArg )
            {
               gTimerCommandInProgress++;
            }
            else if ( timer_id == IOUPeriodicTimerArg )
            {
               gIOUTimerCommandInProgress++;
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_SendFireSignal
* description     : Send a fire signal to the Application.
*
* @param - ruChannelIndex.    The channel that raised the fire alarm
* @param - sensorType.        The type of sensor raising the alarm
* @param - analogueValue.     The analogue reading from the sensor
* @param - alarmActive.       Set to true if the head is in an alarm condition
*
* @return - None.
*/
void MM_SendFireSignal( const uint8_t ruChannelIndex, const CO_RBUSensorType_t sensorType, const uint8_t analogueValue, const bool alarmActive )
{
   /* Push the fire signal request up to the application */
   osStatus osStat = osErrorOS;
   CO_Message_t *pFireSignalReq = NULL;
   CO_RBUSensorData_t headData;

   /* create FireSignalReq message and put into mesh queue */
   pFireSignalReq = osPoolAlloc(AppPool);
   if (pFireSignalReq)
   {
      /* Populate Head Data */
      headData.SensorType = sensorType;
      headData.AlarmState = alarmActive;
      headData.RUChannelIndex = ruChannelIndex;
      headData.SensorValue = analogueValue;
      pFireSignalReq->Type = CO_MESSAGE_GENERATE_FIRE_SIGNAL_E;
      memcpy(pFireSignalReq->Payload.PhyDataReq.Data, &headData, sizeof(CO_RBUSensorData_t));
      osStat = osMessagePut(AppQ, (uint32_t)pFireSignalReq, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         osPoolFree(AppPool, pFireSignalReq);
      }
      
      CO_PRINT_A_3(DBG_INFO_E, "FIRE SIGNAL : Chan=%d, Value=%d, ActiveFlag=%d\r\n",ruChannelIndex, analogueValue, alarmActive);
#ifdef SHOW_HEAD_TRAFFIC_ON_PPU_USART
      SerialDebug_Print(PPU_UART_E, DBG_INFO_E, "FIRE SIGNAL : Chan=%d, Value=%d, ActiveFlag=%d\r\n",ruChannelIndex, analogueValue, alarmActive);
#endif
   }
}

/*************************************************************************************/
/**
* function name   : MM_SendAlarmSignal
* description     : Send an alarm signal to the Application.
*
* @param - ruChannelIndex.    The channel that raised the fire alarm
* @param - sensorType.        The type of sensor raising the alarm
* @param - analogueValue.     The analogue reading from the sensor
* @param - alarmActive.       Set to true if the head is in an alarm condition
*
* @return - None.
*/
void MM_SendAlarmSignal( const uint8_t ruChannelIndex, const CO_RBUSensorType_t sensorType, const uint8_t analogueValue, const bool alarmActive )
{
   /* Push the fire signal request up to the application */
   osStatus osStat = osErrorOS;
   CO_Message_t *pFireSignalReq = NULL;
   CO_RBUSensorData_t headData;

   /* create FireSignalReq message and put into mesh queue */
   pFireSignalReq = osPoolAlloc(AppPool);
   if (pFireSignalReq)
   {
      /* Populate Head Data */
      headData.SensorType = sensorType;
      headData.AlarmState = alarmActive;
      headData.RUChannelIndex = ruChannelIndex;
      headData.SensorValue = analogueValue;
      pFireSignalReq->Type = CO_MESSAGE_GENERATE_ALARM_SIGNAL_E;
      memcpy(pFireSignalReq->Payload.PhyDataReq.Data, &headData, sizeof(CO_RBUSensorData_t));
      osStat = osMessagePut(AppQ, (uint32_t)pFireSignalReq, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         osPoolFree(AppPool, pFireSignalReq);
      }
      
      CO_PRINT_A_3(DBG_INFO_E, "ALARM SIGNAL : Chan=%d, Value=%d, ActiveFlag=%d\r\n",ruChannelIndex, analogueValue, alarmActive);
   }
}

/*************************************************************************************/
/**
* function name   : MM_ConfigureForTestModeSleep
* description     : Close down all radio board activity for minimum battery consumption.
*
* @param - None.
*
* @return - None.
*/
void MM_ConfigureForTestModeSleep(void)
{
   //stop the periodic timer
   osTimerStop(PeriodicTimerID);
   
   if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAD_DEVICES, DC_MATCH_ANY_E) )
   {
      //Tell the plugin manager to enter sleep mode
      HeadMessage_t head_message;
      head_message.MessageType = HEAD_SLEEP_MODE_E;
      head_message.Value = 1;
      MM_CMD_SendHeadMessage(CO_MESSAGE_APPLICATION_REQUEST_E, &head_message, CMD_SOURCE_APPLICATION_E);
   }
   
   //Stop the timed event timer
   TE_EnterSleepMode();
   
   //Stop GPIO polling
   DM_InputMonitorSleep();
   
   //switch off the LEDs
   DM_LedPatterns_t led_pattern = DM_LedGetCurrentPattern();
   while ( LED_PATTERN_NOT_DEFINED_E != led_pattern )
   {
      DM_LedPatternRemove(led_pattern);
      led_pattern = DM_LedGetCurrentPattern();
   }
   DM_LedTurnOff();
   
   //Stop all outputs
   CO_AlarmOutputStateData_t alarm_data;
   alarm_data.Silenceable = 0;
   alarm_data.Unsilenceable = 0;
   alarm_data.DelayMask = 0;
   DM_OP_ProcessAlarmOutputStateCommand(&alarm_data, true );

   //If we have a SiteNet 85dB sounder, power it off.
   if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SOUNDER_85DBA, DC_MATCH_ANY_E) )
   {
      //Disable the I2C to the sounder board via the sounder/beacon power line
      GpioWrite(&SndrBeaconOn, 0);
   }
   
   //Set the gpio for sleep mode
   DM_MCUPower_Enter_SLEEPMode();

}

