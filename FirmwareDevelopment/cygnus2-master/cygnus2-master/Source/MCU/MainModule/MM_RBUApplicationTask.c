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
*  File         : MM_RBUApplicationTask.c
*
*  Description  : RBU Application code
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>



/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "CO_ErrorCode.h"
#include "MM_RBUApplicationTask.h"
#include "MM_MeshAPI.h"
#include "DM_SerialPort.h"
#include "DM_LED.h"
#include "board.h"
#include "MM_ATHandleTask.h"
#include "MM_CommandProcessor.h"
#include "DM_svi.h"
#include "DM_BatteryMonitor.h"
#include "DM_RelayControl.h"
#include "DM_InputMonitor.h"
#include "DM_OutputManagement.h"
#include "MC_MacConfiguration.h"
#include "MM_GpioTask.h"
#include "MM_TimedEventTask.h"
#include "CFG_Device_cfg.h"
#include "MM_NeighbourInfo.h"
#include "MM_ApplicationCommon.h"
#include "DM_FA_NonLatchingButton.h"
#include "common.h"
#include "MM_PpuManagement.h"
#include "MC_StateManagement.h"
#include "MM_Main.h"
#include "MM_ConfigSerialTask.h"


/* Private definitions
*************************************************************************************/
#define LOST_CONNECTION_TIMEOUT 2u
#define STARTUP_STATUS_REPORT_DELAY 100 //number of seconds after going active before sending fault and battery status
#define INPUT_DEACTIVATING_DEBOUNCE_COUNT 2000        //The debounce period in milliseconds for change to 'SET' in the analog inputs.
#define INPUT_ACTIVATING_DEBOUNCE_COUNT 1000          //The debounce period in milliseconds for changes away from 'SET' in the analog inputs.
#define MANUAL_RESET_LED_PERIOD 2u                    //The time that the LED is held on during manual reset
#define MANUAL_RESET_RESPONSE_PERIOD 3u               //The time that the user has to release the tamper switch after the LED goes off
#define BATTERY_STATUS_RESCHEDULE_PERIOD 600u         //The resend period if the device fails to send the battery status
#define PIR_LATCH_PERIOD 60u
typedef enum
{
   RBU_INITIALISING_E,
   RBU_JOINING_MESH_E,
   RBU_WAITING_FOR_LOGON_E,
   RBU_OPERATIONAL_E
} RBUState_t;

typedef enum
{
   PIR_EVENT_TIMER_E,
   PIR_EVENT_PIR_ACTIVE_E,
   PIR_EVENT_PIR_IDLE_E,
   PIR_EVENT_RESET_E,
   PIR_EVENT_MAX_E
} PirEvent_t;



/* Private Functions Prototypes
*************************************************************************************/
void MM_RBUApplicationInitialise(void);
void MM_RBUApplicationTaskProcessMessage(const uint32_t timeout);
void MM_RBUApplicationLogonReq(void);
void MM_RBUApplicationFireSignalReq(const CO_RBUSensorData_t* pHeadData);
void MM_RBUApplicationAlarmSignalReq(const CO_RBUSensorData_t* pSensorData);
static void MM_RBUApplicationProcessNetworkMessage(ApplicationMessage_t* pMessage);
static void MM_RBUProcessMACEvent(AppMACEventData_t* pEventData);

static bool MM_RBUApplicationProcessMeshStatusEvent(const CO_StatusIndicationData_t* pEventData);
static void MM_RBUApplicationProcessCommandMessage(const ApplicationMessage_t* pAppMessage);
static void MM_RBUApplicationProcessResponseMessage(const ApplicationMessage_t* pAppMessage);
static void MM_RBUApplicationProcessSetStateMessage(const ApplicationMessage_t* pAppMessage);
static void MM_RBUApplicationProcessRBUDisableMessage(const ApplicationMessage_t* pAppMessage);
static void MM_RBUApplicationProcessConfirmation(const ApplicationMessage_t* pAppMessage);
static void MM_RBUApplicationLogonConfirmation(const uint32_t Handle, const uint32_t Status);
static void MM_RBUApplicationProcessStatusIndication(const ApplicationMessage_t* pAppMessage);
static void MM_RBUApplicationProcessZoneEnableMessage(const ApplicationMessage_t* pAppMessage);
static void MM_RBUApplicationProcessAtCommandMessage(const ApplicationMessage_t* pAppMessage);
static void MM_RBUApplicationProcessGlobalDelaysMessage(const ApplicationMessage_t* pAppMessage);
static void MM_ReadInputChannels(void);
       void MM_ApplicationSendFaultReports(const bool sendOnDULCH);
static bool MM_RBUApplicationUpdateStatusFlags( CO_FaultData_t* FaultStatus );
       void MM_ApplicationSendAlarmSignals(void);
static bool MM_ChannelEnabled(const uint16_t channelIndex);
static bool MM_SendSensorValues(void);
static ErrorCode_t MM_RBU_RequestSensorValues(void);
static void MM_ApplicationResetPir(const bool sendCancelMessage);
static void MM_ApplicationHandlePirEvent(const PirEvent_t pirEvent);
static void MM_AppUpdateFaultStatusLED(void);
static bool MM_AppBitFailReported(void);

/* Global Variables
*************************************************************************************/
extern uint16_t gDeviceCombination;
extern uint16_t gLastCommandedChannel;
extern uint8_t gLastCommandedOutputProfile;
extern uint8_t gLastCommandedOutputActive;
extern uint16_t gLastCommandedOutputDuration;
extern uint8_t gLastCommandedSilenceableMask;
extern uint8_t gLastCommandedUnsilenceableMask;
extern uint8_t gLastCommandedDelayMask;
static uint32_t gPirLatched = false;
extern uint32_t gZoneNumber;
extern CO_State_t gCurrentDeviceState;
extern uint16_t gNodeAddress;
extern osSemaphoreId(GpioSemId);
extern osPoolId AppPool;
extern osMessageQId(AppQ);
extern osMessageQId(HeadQ);
extern int32_t gHeadQueueCount;
#ifndef USE_NEW_HEAD_INTERFACE
extern osSemaphoreId HeadInterfaceSemaphoreId;
#endif
extern osPoolId ATHandlePool;
extern osMessageQId(ATHandleQ);
extern osMessageQId(ConfigSerialQ);       // Queue definition from ConfigSerialTask
extern osPoolId ConfigSerialPool;         // Pool definition from ConfigSerialTask

extern DM_BaseType_t gBaseType;
extern Gpio_t SndrBeaconOn;
extern Gpio_t EnDet1;
extern Gpio_t EnDet2;

extern bool gTimerCommandInProgress;
extern uint32_t PeriodicTimerArg;

extern bool gIOUTimerCommandInProgress;
extern uint32_t IOUPeriodicTimerArg;

extern bool gFreqLockAchieved;

extern uint32_t HeadPeriodicTimerArg ;

extern uint32_t gResetToPpenMode;

extern uint32_t rbu_pp_mode_request;

extern bool gBatteryTestRestartFlag;    /* 0:Battery test request not generated, 1: Battery test request generated  */
extern bool gBatteryTestStartFlag;     /* 0:Battery test not started, 1: Battery test started  */
extern bool gBatteryTestCompleteFlag;  /* 0:Battery test not completed, 1: Battery test completed  */
extern bool gBatteryTestDelay;

/* Private Variables
*************************************************************************************/

static RBUState_t rbu_state = RBU_INITIALISING_E;
static uint32_t logonRequestHandle = 0;
//osThreadId tid_RBUTask;
//osThreadDef (MM_RBUApplicationTaskMain, osPriorityNormal, 1, 1824);

static InputState_t gInputStates[MAX_INPUT_CHANNELS];
static uint32_t gInputDebounceCount[MAX_INPUT_CHANNELS];
static bool gLostConnectionToMesh = false;
static uint32_t gLostConnectionToMeshCount = 0;
static uint32_t gDelayedStatusReportCount = 0;
static bool gPrimaryParentConnected = false;
static uint32_t HeatTimerCount = 0;
static uint32_t gFaultStatusFlags;
static bool gBitFaultStatus[BIT_SOURCE_MAX_E];
static uint8_t gSmokeAnalogueValue;
static uint8_t gHeatAnalogueValue;
static uint8_t gPirAnalogueValue;
static bool gAnalogueValuesRequested;
static bool gWaitingForSmokeAnalogueValue;
static bool gWaitingForHeatAnalogueValue;
static bool gWaitingForPirAnalogueValue;
static uint32_t gBatteryStatusMessageReschuled;
static bool MeshAtCommandInProgress = false;    //Set to true if we have received an AT command over the radio and are currently processing it
static AtMessageData_t MeshAtCommand;           //This is a copy of the last received AT command over the radio, for return addressing.
static bool gJoiningPhaseComplete = false;

/*************************************************************************************/
/**
* MM_RBUApplicationTaskInit
* Startup  function for the RBU Application task
*
* @param - void
*
* @return - int32_t   0 if the RBU Application thread was started or -1 on failure.
*/
ErrorCode_t MM_RBUApplicationTaskInit(const uint16_t address, const uint16_t device_configuration, const uint32_t unitSerialNo, const uint32_t defaultZoneNumber)
{
   ErrorCode_t result = ERR_INVALID_PARAMETER_E;
   gNodeAddress = address;
   gZoneNumber = defaultZoneNumber;
   gFaultStatusFlags = 0;
   gPirLatched = 0;
   gSmokeAnalogueValue = 0;
   gHeatAnalogueValue = 0;
   gPirAnalogueValue = 0;
   gAnalogueValuesRequested = false;
   gWaitingForSmokeAnalogueValue = false;
   gWaitingForHeatAnalogueValue = false;
   gWaitingForPirAnalogueValue = false;
   gBatteryStatusMessageReschuled = 0;
   gJoiningPhaseComplete = false;
   
   
   if ( ( 0 < address ) && (MAX_DEVICES_PER_SYSTEM > address) && (DC_NUMBER_OF_DEVICE_COMBINATIONS_E > device_configuration) )
   {
      if ( DC_NUMBER_OF_DEVICE_COMBINATIONS_E > device_configuration )
      {
         /* Init neighbour information array */
         MM_NI_Initialise();
         //Initialise the command processor
         MM_CMD_Initialise(address, device_configuration, false, unitSerialNo);
         result = SUCCESS_E;
      }
      else 
      {
         CO_PRINT_A_1(DBG_ERROR_E,"RBU Configured with bad device combination - %d\r\n", device_configuration);
      }
   }
   else 
   {
      CO_PRINT_A_1(DBG_ERROR_E,"RBU Configured with bad address - %d\r\n", address);
   }
   
   
   return result;
}


/*************************************************************************************/
/**
* MM_RBUApplicationInitialise
* Initialisation function for the RBU Application
*
* @param - void
*
* @return - void
*/
void MM_RBUApplicationInitialise(void)
{
   /* initialise the child records */
   MM_ApplicationInitialiseChildArray();
   /* start the RBU in CONFIG_SYNC */
   MM_ApplicationUpdateDeviceState(STATE_IDLE_E);
   
   /* Initialise the Output Management */
   DM_OP_Initialise(true);
   
   //Initialise the non-latching first aid button handler
   DM_FA_Initialise();
   
   // For all non-NCU types, initialise the battery monitor
   if ( BASE_NCU_E != gBaseType )
   {
      DM_BAT_Initialise(gBaseType);
   }
   
   //For base type IOU, initiaise the relay states
   if ( BASE_IOU_E == gBaseType )
   {
      DM_RC_Initialise();
   }
   
   //Initialise the input channel states
   for ( uint32_t index = 0; index <  MAX_INPUT_CHANNELS; index++)
   {
      gInputStates[index] = INPUT_STATE_RESET_E;
   }
   
   //Initialise the BIT fault states
   for ( uint32_t index = 0; index <  BIT_SOURCE_MAX_E; index++)
   {
      gBitFaultStatus[index] = false;
   }
   
   MC_ResetTimeActive();
}


/*************************************************************************************/
/**
* MM_RBUApplicationTaskMain
* Main function for RBU Application task.
*
* @param - argument  Pointer to passed in data.
*
* @return - void
*/
void MM_RBUApplicationTaskMain(void)
{
   CO_PRINT_B_0(DBG_INFO_E, "RBU Application running\r\n");
    
   while(true)
   {
      switch ( rbu_state )
      {
         case RBU_INITIALISING_E:
               MM_RBUApplicationInitialise();
               StartPeriodicTimer();
               rbu_state = RBU_JOINING_MESH_E;
            break;
         case RBU_JOINING_MESH_E:
#ifdef SEND_LOG_ON
            if ( 0 == logonRequestHandle )
            {
               MM_RBUApplicationLogonReq();
            }
            MM_RBUApplicationTaskProcessMessage( 100 );
#else
            rbu_state = RBU_OPERATIONAL_E;
#endif
            break;
         case RBU_WAITING_FOR_LOGON_E:
            /* nothing to do but wait for the call to MM_RBUApplicationLogonCnfCb */
            MM_RBUApplicationTaskProcessMessage( 100 );
            break;
         case RBU_OPERATIONAL_E:
            MM_RBUApplicationTaskProcessMessage( osWaitForever );
            break;
         default:
            break;
      }
   
   };
}

/*************************************************************************************/
/**
* MM_RBUApplicationTaskProcessMessage
* Function to read a message from the AppQ and call the appropriate handling function.
*
* @param - timeout   Limits how long the function should wait for a message to arrive (milliseconds).
*
* @return - void
*/
void MM_RBUApplicationTaskProcessMessage( const uint32_t timeout )
{
   osEvent event;
   CO_Message_t *pMsg = NULL;
   uint32_t MessageHandle = 0;
     
   event = osMessageGet(AppQ, timeout);
   if (osEventMessage == event.status)
   {
      pMsg = (CO_Message_t *)event.value.p;
      if (pMsg)
      {
         switch (pMsg->Type)
         {
            case CO_MESSAGE_GENERATE_FIRE_SIGNAL_E:
            {
               CO_RBUSensorData_t* pSensorData = (CO_RBUSensorData_t*)pMsg->Payload.PhyDataReq.Data;
               if( pSensorData )
               {
                  if ( MM_ChannelEnabled(pSensorData->RUChannelIndex) )
                  {
                     MM_RBUApplicationFireSignalReq(pSensorData);
                  }
                  else 
                  {
                     CO_PRINT_B_1(DBG_INFO_E,"Discarding Fire Signal - channel %d disabled\r\n", pSensorData->RUChannelIndex);
                  }
               }
            }
            break;
            case CO_MESSAGE_GENERATE_ALARM_SIGNAL_E:
            {
               CO_RBUSensorData_t* pSensorData = (CO_RBUSensorData_t*)pMsg->Payload.PhyDataReq.Data;
               if( pSensorData )
               {
                  //Check that the channel is enabled
                  if ( MM_ChannelEnabled(pSensorData->RUChannelIndex) )
                  {
                     //If the channel is a medical MCP and the device has a non-latching button
                     if ( (CO_CHANNEL_MEDICAL_CALLPOINT_E == pSensorData->RUChannelIndex) && CFG_HasNonLatchingMedicalCallPoint() )
                     {
                        if ( GPIO_ACTIVE == pSensorData->SensorValue)
                        {
                           //The first aid button is pressed.  Start the non-latching behaviour monitor.
                           DM_FA_SetButtonState(true);
                        }
                        else 
                        {
                           //The first aid button is released.  Cancel the non-latching behaviour monitor.
                           DM_FA_SetButtonState(false);
                        }
                     }
                     else if ( CO_CHANNEL_PIR_E == pSensorData->RUChannelIndex )
                     {
                        CO_PRINT_B_1(DBG_INFO_E,"PIR state change to %d\r\n", pSensorData->AlarmState);
                        if ( pSensorData->AlarmState )
                        {
                           MM_ApplicationHandlePirEvent(PIR_EVENT_PIR_ACTIVE_E);
                        }
                        else 
                        {
                           MM_ApplicationHandlePirEvent(PIR_EVENT_PIR_IDLE_E);
                        }
                     }
                     else 
                     {
                        //Its not a non-latching first aid button or PIR.  Just end the alarm.
                        MM_RBUApplicationAlarmSignalReq(pSensorData);
                     }
                  }
                  else 
                  {
                     CO_PRINT_B_1(DBG_INFO_E,"Discarding Alarm Signal - channel %d disabled\r\n", pSensorData->RUChannelIndex);
                  }
                  //Make sure that changed input channels that have reported OPEN CCT or SHORT CCT faults cancel the previous state on the control panel
                  if ( BASE_IOU_E == gBaseType )
                  {
                     if ( (CO_CHANNEL_INPUT_1_E <= pSensorData->RUChannelIndex) && (CO_CHANNEL_INPUT_32_E >= pSensorData->RUChannelIndex) )
                     {
                        InputState_t previous_state;
                        if ( SUCCESS_E == DM_RC_GetLastReportedState((CO_ChannelIndex_t)pSensorData->RUChannelIndex, &previous_state) )
                        {
                           if ( INPUT_STATE_FAULT_SHORT_E == previous_state )
                           {
                              //The channel has moved away from SHORT CCT.  Send a fault message to cancel the fault
                              CO_SendFaultSignal(BIT_SOURCE_NONE_E,  (CO_ChannelIndex_t)pSensorData->RUChannelIndex, FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E, 0, false, false );
                           }
                           else if ( INPUT_STATE_FAULT_OPEN_E == previous_state )
                           {
                              //The channel has moved away from OPEN CCT.  Send a fault message to cancel the fault
                              CO_SendFaultSignal(BIT_SOURCE_NONE_E,  (CO_ChannelIndex_t)pSensorData->RUChannelIndex, FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E, 0, false, false );
                           }
                        }
                        
                        //Record the state that was sent to the control panel for future checks
                        if ( pSensorData->SensorValue )
                        {
                           DM_RC_SetLastReportedState((CO_ChannelIndex_t)pSensorData->RUChannelIndex, INPUT_STATE_RESET_E);
                        }
                        else 
                        {
                           DM_RC_SetLastReportedState((CO_ChannelIndex_t)pSensorData->RUChannelIndex, INPUT_STATE_SET_E);
                        }
                     }
                  }
               }
            }
            break;
            case CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E:
            {
               uint8_t* pCommand = (uint8_t*)pMsg->Payload.PhyDataReq.Data;
               if( pCommand )
               {
                  CO_CommandData_t commandData;
                  memcpy( &commandData, pCommand, sizeof(CO_CommandData_t));
                  CommandSource_t cmd_source = MM_CMD_ConvertFromATCommandSource(commandData.Source);
                  ErrorCode_t result = MM_CMD_ProcessCommand ( &commandData, cmd_source );
                  if ( SUCCESS_E != result )
                  {
                     CO_PRINT_B_1(DBG_ERROR_E, "FAILED to process command.  Error=%d\r\n", result);
                  }
               }
            }
            break;
            case CO_MESSAGE_GENERATE_RESPONSE_SIGNAL_E:
            {
               uint8_t* pResponse = (uint8_t*)pMsg->Payload.PhyDataReq.Data;
               if( pResponse )
               {
                  HeadMessage_t headMessage;
                  memcpy( &headMessage, pResponse, sizeof(HeadMessage_t));
                  ErrorCode_t result = MM_CMD_ProcessHeadMessage ( &headMessage );
                  if ( SUCCESS_E != result )
                  {
                     CO_PRINT_B_1(DBG_ERROR_E, "FAILED to process Head response.  Error=%d\r\n", result);
                  }
               }
            }
            break;            
            case CO_MESSAGE_MESH_STATUS_EVENT_E:
            {
               ApplicationMessage_t* pAppMessage = (ApplicationMessage_t*)pMsg->Payload.PhyDataInd.Data;
               if ( pAppMessage )
               {
                  switch (pAppMessage->MessageType)
                  {
                     case APP_MSG_TYPE_STATUS_SIGNAL_E:
                     {
                        /* This is the local node status update (added child etc) */
                        CO_StatusIndicationData_t* pEventData = (CO_StatusIndicationData_t*)pAppMessage->MessageBuffer;
                        if ( pEventData )
                        {
                           //Set the overall fault flag if any faults exist.
                           pEventData->OverallFault = MM_RBU_GetOverallFaults();
                           
                           bool sendStatusIndication = MM_RBUApplicationProcessMeshStatusEvent(pEventData);
                           
                           if ( sendStatusIndication )
                           {
                              MessageHandle = GetNextHandle();
                              MM_MeshAPIGenerateStatusSignalReq( MessageHandle, pEventData->Event, pEventData->EventNodeId, pEventData->EventData, pEventData->OverallFault, pEventData->DelaySending );
                           }
                        }
                     }
                        break;
                     case APP_MSG_TYPE_ROUTE_DROP_E:
                     {
                        CO_StatusIndicationData_t* pEventData = (CO_StatusIndicationData_t*)pAppMessage->MessageBuffer;
                        if ( pEventData )
                        {
                           MM_RBUApplicationProcessMeshStatusEvent(pEventData);
                        }
                        break;
                     }
                     case APP_MSG_TYPE_DAY_NIGHT_STATUS_E:
                     {
                        CO_StatusIndicationData_t* pStatusEvent = (CO_StatusIndicationData_t*)pAppMessage->MessageBuffer;
                        if ( pStatusEvent )
                        {
                           if ( CO_NUM_DAYNIGHT_SETTINGS_E > (CO_DayNight_t)pStatusEvent->EventData )
                           {
                              //Update the config
                              MM_CMD_ProcessDayNightSetting( (CO_DayNight_t)pStatusEvent->EventData );
                           }
                        }
                     }
                     break;
#ifdef ENABLE_LINK_ADD_OR_REMOVE
                     case APP_MSG_TYPE_DROP_NODE_LINK_E:
                     {
                        //From AT command to tell the session manager and 'mesh forming' to discard a node link
                        RouteDropData_t dropMessage;
                        memcpy((uint8_t*)&dropMessage, pAppMessage->MessageBuffer , sizeof(RouteDropData_t));
                        MM_MeshAPIApplicationCommand(APP_MSG_TYPE_DROP_NODE_LINK_E, dropMessage.NodeID, 0, 0, 0);
                     }
                        break;
                     case APP_MSG_TYPE_ADD_NODE_LINK_E:
                     {
                        //From AT command to tell the session manager and 'mesh forming' to add a node link
                        RouteDropData_t dropMessage;
                        memcpy((uint8_t*)&dropMessage, pAppMessage->MessageBuffer , sizeof(RouteDropData_t));
                        MM_MeshAPIApplicationCommand(APP_MSG_TYPE_ADD_NODE_LINK_E, dropMessage.NodeID, dropMessage.Downlink, CMD_SOURCE_APPLICATION_E, 0);
                     }
                        break;
#endif
                     default:
                        break;
                  }
               }
            }
            break;
            case CO_MESSAGE_GENERATE_TEST_MESSAGE_E:
            {
               ATTestMessageData_t testMessage;
               uint8_t* pReq = (uint8_t*)pMsg->Payload.PhyDataReq.Data;
               
               if( pReq )
               {                                    
                  memcpy((uint8_t*)&testMessage, pReq, sizeof(ATTestMessageData_t));
                  bool result = MM_MeshAPIGenerateTestMessageReq( testMessage.stringMsg );

                  CommandSource_t source = MM_CMD_ConvertFromATCommandSource(testMessage.source);

                  if( false == result )
                  {
                     MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
                  }
                  else
                  {
                     MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
                  }
               }
            }
            break;
            case CO_MESSAGE_GENERATE_SET_STATE_E:
            {
               uint8_t* pCommand = (uint8_t*)pMsg->Payload.PhyDataReq.Data;

               if( pCommand )
               {
                  CO_CommandData_t commandData;
                  memcpy( &commandData, pCommand, sizeof(CO_CommandData_t));
                  CommandSource_t cmd_source = MM_CMD_ConvertFromATCommandSource(commandData.Source);

                  /* The RBU does not support the Generate State Message comand */
                  MM_CMD_SendATMessage(AT_RESPONSE_FAIL, cmd_source);
               }
            }
            break;

            case CO_MESSAGE_GENERATE_RBU_DISABLE_MESSAGE_E:
            {
               uint8_t* pCommand = (uint8_t*)pMsg->Payload.PhyDataReq.Data;

               if( pCommand )
               {
                  CO_CommandData_t commandData;
                  memcpy( &commandData, pCommand, sizeof(CO_CommandData_t));
                  CommandSource_t cmd_source = MM_CMD_ConvertFromATCommandSource(commandData.SourceUSART);

                  /* The RBU Disable command cannot be executed locally */
                  MM_CMD_SendATMessage(AT_RESPONSE_FAIL, cmd_source);
               }
            }                   
            break;

            case CO_MESSAGE_GENERATE_RBU_REPORT_MESSAGE_E:
            {
               uint8_t* pCommand = (uint8_t*)pMsg->Payload.PhyDataReq.Data;

               if( pCommand )
               {
                  CO_CommandData_t commandData;
                  memcpy( &commandData, pCommand, sizeof(CO_CommandData_t));


                  if( false ==  MM_MeshAPIGenerateRBUReportNodesReq((uint32_t)commandData.Source) )
                  {              
                     CommandSource_t cmd_source = MM_CMD_ConvertFromATCommandSource(commandData.Source);
                     MM_CMD_SendATMessage(AT_RESPONSE_FAIL, cmd_source);
                  }
               }                             
            }
            break;
            case CO_MESSAGE_GENERATE_OUTPUT_SIGNAL_E:
            {
               /* extract the output data from the received message */
               CO_OutputData_t outData;
               uint8_t* pReq = (uint8_t*)pMsg->Payload.PhyDataReq.Data;
               
               if( pReq )
               {
                  memcpy((uint8_t*)&outData, pReq, sizeof(CO_OutputData_t));
                  
                  /* find out which usart to respond to */
                  CommandSource_t source = MM_CMD_ConvertFromATCommandSource(outData.Source);
                  
                  if ( (gNodeAddress == outData.Destination) || (ADDRESS_GLOBAL == outData.Destination) )
                  {
                     if ( BASE_CONSTRUCTION_E == gBaseType || BASE_RBU_E == gBaseType || BASE_IOU_E == gBaseType )
                     {
                        /* Drop a message into the Application queue as if it came OTA */
                        ApplicationMessage_t appMessage;
                        appMessage.MessageType = APP_MSG_TYPE_OUTPUT_SIGNAL_E;
                        OutputSignalIndication_t outputData;
                        outputData.nodeID = outData.Destination;
                        outputData.zone = outData.zone;
                        outputData.OutputChannel = outData.OutputChannel;
                        outputData.OutputProfile = outData.OutputProfile;
                        outputData.OutputsActivated = outData.OutputsActivated;
                        outputData.OutputDuration = outData.OutputDuration;
                        memcpy(appMessage.MessageBuffer, &outputData, sizeof(OutputSignalIndication_t));


                        /* create a status message and put into the App queue */
                        osStatus osStat = osErrorOS;
                        CO_Message_t *pMessage = NULL;
                        pMessage = osPoolAlloc(AppPool);
                        if ( pMessage )
                        {
                           pMessage->Type = CO_MESSAGE_PHY_DATA_IND_E;
                           memcpy(pMessage->Payload.PhyDataInd.Data, (uint8_t*)&appMessage, sizeof(ApplicationMessage_t));
                           osStat = osMessagePut(AppQ, (uint32_t)pMessage, 0);
                           if (osOK != osStat)
                           {
                              /* failed to write */
                              osPoolFree(AppPool, pMessage);
                              MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
                           }
                           else
                           {
                              MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
                           }
                        }
                     }
                     else 
                     {
                        MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
                     }
                  }
                  else
                  {
                     MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
                  }
               }
            }
            break;
				 
            case CO_MESSAGE_GENERATE_FAULT_SIGNAL_E:
            {
               //Send the fault signal to the mesh.
               CO_FaultData_t* pFaultStatus = (CO_FaultData_t*)pMsg->Payload.PhyDataReq.Data;
               if ( pFaultStatus )
               {
                  CO_FaultData_t faultData;
                  faultData.Handle = GetNextHandle();
                  faultData.Source = pFaultStatus->Source;
                  faultData.RUChannelIndex = pFaultStatus->RUChannelIndex;
                  faultData.FaultType = pFaultStatus->FaultType;
                  faultData.DelaySending = pFaultStatus->DelaySending;
                  faultData.Value = pFaultStatus->Value;
                  faultData.ForceSend = pFaultStatus->ForceSend;
                  
                  //If the RBU is still in the joining phase, force the use of DULCH
                  if ( false == gJoiningPhaseComplete )
                  {
                     faultData.DelaySending = true;
                  }
                  
                  CO_PRINT_B_5(DBG_INFO_E,"App Rxd fault: chan=%d, type=%d, value=%d, dulch=%d, force=%d\r\n", 
                  faultData.RUChannelIndex, faultData.FaultType, faultData.Value, faultData.DelaySending, faultData.ForceSend);
                  
                  //Make sure that changed input channels that have reported OPEN CCT or SHORT CCT faults cancel the previous state on the control panel
                  if ( BASE_IOU_E == gBaseType )
                  {
                     if ( (CO_CHANNEL_INPUT_1_E <= faultData.RUChannelIndex) && (CO_CHANNEL_INPUT_32_E >= faultData.RUChannelIndex) )
                     {
                        InputState_t previous_state;
                        if ( SUCCESS_E == DM_RC_GetLastReportedState((CO_ChannelIndex_t)faultData.RUChannelIndex, &previous_state) )
                        {
                           if ( (INPUT_STATE_FAULT_SHORT_E == faultData.FaultType) && 
                                (INPUT_STATE_FAULT_SHORT_E != previous_state) )
                           {
                              //The channel has moved away from SHORT CCT.  Send a fault message to cancel the fault
                              faultData.FaultType = FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E;
                              faultData.Value = 0;
                           }
                           else if ( (INPUT_STATE_FAULT_OPEN_E == faultData.FaultType) && 
                                (INPUT_STATE_FAULT_OPEN_E != previous_state) )
                           {
                              //The channel has moved away from OPEN CCT.  Send a fault message to cancel the fault
                              faultData.FaultType = FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E;
                              faultData.Value = 0;
                           }
                        }
                        
                        //Record the last fault sent to the control panel for the input
                        DM_RC_SetLastReportedState((CO_ChannelIndex_t)faultData.RUChannelIndex, (InputState_t)faultData.FaultType);
                     }
                  }
                  
                  //Update the local fault status flags
                  bool flags_changed = MM_RBUApplicationUpdateStatusFlags(&faultData);
                  
                  //Only send a fault message if the fault status flags are changed or 'ForceSend' is set.
                  if( flags_changed || faultData.ForceSend )
                  {
                     //Set the overall fault flag if any faults exist.
                     faultData.OverallFault = MM_RBU_GetOverallFaults();
                     
                     if ( CFG_GetFaultReportsEnabled() )
                     {
                        CO_PRINT_B_5(DBG_INFO_E,"App sending fault signal, Chan=%d, Type=%d, delayed=%d, overallFault=%d, Value=%d\r\n", pFaultStatus->RUChannelIndex, pFaultStatus->FaultType, pFaultStatus->DelaySending, faultData.OverallFault, pFaultStatus->Value);
                        MM_MeshAPIGenerateFaultSignalReq( &faultData );
                     }
                     else 
                     {
                        CO_PRINT_B_3(DBG_INFO_E,"App Rx'd fault signal - FAULTS DISABLED, Chan=%d, Type=%d, Value=%d\r\n", pFaultStatus->RUChannelIndex, pFaultStatus->FaultType, pFaultStatus->Value);
                     }
                  }
                  
               }
            }
            break;
            case CO_MESSAGE_GENERATE_BATTERY_STATUS_E:
            {
               //Send the battery voltage to the mesh.
               CO_BatteryStatusData_t *pBatteryData = (CO_BatteryStatusData_t*)pMsg->Payload.PhyDataReq.Data;
               if ( pBatteryData )
               {
                  uint16_t vPBat  = pBatteryData->PrimaryBatteryVoltage;
                  uint16_t vBkBat = pBatteryData->BackupBatteryVoltage;
                  CO_PRINT_B_2(DBG_INFO_E,"RBUAppTask: Sending battery Primary=%dmV, Backup=%dmV\r\n", vPBat, vBkBat);
                  pBatteryData->ZoneNumber = gZoneNumber;
                  pBatteryData->DeviceCombination = gDeviceCombination;
                  pBatteryData->SmokeAnalogueValue = gSmokeAnalogueValue;
                  pBatteryData->HeatAnalogueValue = gHeatAnalogueValue;
                  pBatteryData->PirAnalogueValue = gPirAnalogueValue;
                  MM_MeshAPIGenerateBatteryStatusSignalReq( pBatteryData );
               }
            }
            break;
            case CO_MESSAGE_BATTERY_STATUS_MESSAGE_E:
               if ( DM_BAT_BatteryDepleted() )
               {
#ifdef DEEP_SLEEP_ON_BATTERY_END_OF_LIFE
                  //Set the batteries for deep sleep
                  DM_BAT_ConfigureForDeepSleep();

                  /* put the unit to sleep */
                  CO_PRINT_A_0(DBG_INFO_E,"App Requsting Sleep Mode - depleted battery\r\n");
                  MC_STATE_SetDeviceState(STATE_TEST_MODE_E);
                  MC_SetTestMode( MC_MAC_TEST_MODE_SLEEP_E );
                  MM_ConfigureForTestModeSleep();
#else
                  CO_PRINT_A_0(DBG_INFO_E,"Batteries depleted\r\n");
#endif
               }
               break;
            case CO_MESSAGE_PHY_DATA_IND_E:
            {
               ApplicationMessage_t* pMessageData = (ApplicationMessage_t*)pMsg->Payload.PhyDataInd.Data;
               MM_RBUApplicationProcessNetworkMessage(pMessageData);
            }
               break;
            case CO_MESSAGE_MAC_EVENT_E:
            {
               ApplicationMessage_t* pMessageData = (ApplicationMessage_t*)pMsg->Payload.PhyDataInd.Data;
               if ( pMessageData )
               {
                  AppMACEventData_t* pEventData = (AppMACEventData_t*)pMessageData->MessageBuffer;
                  MM_RBUProcessMACEvent(pEventData);
               }
            }
               break;
            case CO_MESSAGE_CONFIRMATION_E:
            {
               ApplicationMessage_t* pMessageData = (ApplicationMessage_t*)pMsg->Payload.PhyDataInd.Data;
               MM_RBUApplicationProcessConfirmation(pMessageData);
            }
               break;
            case CO_MESSAGE_CIE_COMMAND_E:
            {
               uint8_t* pCmdType = (uint8_t*)pMsg->Payload.PhyDataReq.Data;
               
               if( pCmdType )
               {
                  CIECommand_t* pCommand = (CIECommand_t*)pMsg->Payload.PhyDataReq.Data;
                  
                  ErrorCode_t result = ERR_INVALID_POINTER_E;
                  if ( pCommand )
                  {
                     result = MM_CMD_ProcessCIECommand ( pCommand );
                     if ( SUCCESS_E != result )
                     {
                        CO_PRINT_B_3(DBG_ERROR_E, "FAILED to process CIE command.  cmdType=%d, Source=%d, Error=%d\r\n", (int32_t)pCommand->CommandType, (int32_t)pCommand->PortNumber, result);
                     }
                  }
               }
            }
               break;

            case CO_MESSAGE_TIMER_EVENT_E:
            {
               //CO_PRINT_A_0(DBG_INFO_E,"App Timer Event\r\n");
               // find out which timer triggerred the event
               uint32_t* pTimer_id = (uint32_t*)pMsg->Payload.PhyDataReq.Data;
               
               /* perform periodic actions */
               if ( pTimer_id && *pTimer_id == PeriodicTimerArg )
               {
                  //Do battery checks
                  DM_BAT_Run();
                  
                  /* poll the tamper switches */
                  osSemaphoreRelease(GpioSemId);
                  
                  HeatTimerCount++;
                  if ( 10 >= HeatTimerCount )
                  {
                     HeatTimerCount = 0;
                     //do head periodic checks
                     HeadPeriodicTimerCallback((const void*)HeadPeriodicTimerArg);
                  }

                  if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
                  {
                     /* Manage output delays */
                     DM_OP_ManageDelays();

                     /* SVI periodic checks */
                     if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SOUNDER_85DBA | DEV_SOUNDER_VI, DC_MATCH_ANY_E) )
                     {
                        DM_SVI_PeriodicCheck();
                     }
                     
                     if ( gBatteryStatusMessageReschuled )
                     {
                        gBatteryStatusMessageReschuled--;
                        if ( 0 == gBatteryStatusMessageReschuled )
                        {
                           //this will force a battery status to be generated below.
                           gAnalogueValuesRequested = true;
                        }
                     }
                     
                     if ( gAnalogueValuesRequested )
                     {
                        if ( (false == gWaitingForSmokeAnalogueValue) &&
                             (false == gWaitingForHeatAnalogueValue) &&
                             (false == gWaitingForPirAnalogueValue) )
                        {
                           CO_PRINT_B_0(DBG_INFO_E,"Sending Sensor Values\r\n");
                           gAnalogueValuesRequested = MM_SendSensorValues();
                        }
                     }
                     if ( 0 < gDelayedStatusReportCount )
                     {
                        gDelayedStatusReportCount--;
                        if ( 0 == gDelayedStatusReportCount )
                        {
                           MM_ApplicationSendFaultReports(true);
                        }
                     }
                     
                     //check the non-latching button
                     DM_FA_Update();
                     
                     
                     //Check for test-mode reset of the outputs
                     DM_OP_ResetTestMode();
                     
                     //check the latching PIR
                     MM_ApplicationHandlePirEvent(PIR_EVENT_TIMER_E);
                  
                     //See if the PPEN has been changed
                     if ( gResetToPpenMode )
                     {
                        //The PPEN mode has been changed.  We need to restart, but not until we have 
                        //Sent a route drop to enable connected neighbours to drop the link.
                        //A delayed reset will be invoked by the LoRa stack
                        MM_MeshAPIApplicationCommand(APP_MSG_TYPE_ROUTE_DROP_E, gResetToPpenMode, 0, gNodeAddress, 0);
                        gResetToPpenMode = 0;
                     }
                     if ( gLostConnectionToMesh )
                     {
                        gLostConnectionToMeshCount++;
                        CO_PRINT_B_1(DBG_INFO_E,"Initial tracking node lost - count=%d\r\n", gLostConnectionToMeshCount);
                        if ( LOST_CONNECTION_TIMEOUT <= gLostConnectionToMeshCount )
                        {
                           Error_Handler("Initial tracking node lost while forming\r\n");
                        }
                     }
                  }
                  
                  //Check for PPU periodic actions
                  if ( PPU_STATE_NOT_ACTIVE_E != MC_GetPpuMode() )
                  {
                     MM_PPU_PeriodicChecks();
                  }
                  
                  //Do the delayed output.
                  DM_OP_RunDelayedOutput();
                  
                  gTimerCommandInProgress--;
               }
               
               /* perform periodic actions */
               if ( pTimer_id && *pTimer_id == IOUPeriodicTimerArg )
               {
                  //If the device is in the active state, increment the TimeActive counter.
                  if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
                  {
                     MC_IncrementTimeActive();
                  }
                  
                  MM_ReadInputChannels();
                  
                  gIOUTimerCommandInProgress--;
               }
               
               /* perform 50ms battery readings */
               if ( pTimer_id && *pTimer_id == DM_BAT_TIMER_ID )
               {
                  if ( CFG_GetBatteryCheckEnabled() )
                  {
                     //CO_PRINT_B_0(DBG_INFO_E,"50ms battery check\r\n");
                     //Do battery checks
                     DM_BAT_Run();
                  }
               }
            }
               break;

            case CO_MESSAGE_GENERATE_ALARM_OUTPUT_STATE_E:
            {
               /* extract the output data from the received message */
               CO_AlarmOutputStateData_t alarmData;
               uint8_t* pReq = (uint8_t*)pMsg->Payload.PhyDataReq.Data;
               
               if( pReq )
               {
                  memcpy((uint8_t*)&alarmData, pReq, sizeof(CO_AlarmOutputStateData_t));
                  
                  /* find out which usart to respond to */
                  CommandSource_t source = MM_CMD_ConvertFromATCommandSource(alarmData.Source);
                  
                  /* Build a message and send to the Application, as if it came over the air */
                  ApplicationMessage_t appMessage;
                  appMessage.MessageType = APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E;
                  CO_AlarmOutputStateData_t outData;
                  outData.Source = alarmData.Source;
                  outData.Silenceable = alarmData.Silenceable;
                  outData.Unsilenceable = alarmData.Unsilenceable;
                  outData.DelayMask = alarmData.DelayMask;
                  memcpy(appMessage.MessageBuffer, &outData, sizeof(CO_AlarmOutputStateData_t));
            
                  //Push the mesage back into the AppQ
                  osStatus osStat = osErrorOS;
                  CO_Message_t *pMessage = NULL;

                  /* create a status message and put into the App queue */
                  pMessage = osPoolAlloc(AppPool);
                  if ( pMessage )
                  {
					 pMessage->Type = CO_MESSAGE_PHY_DATA_IND_E;
                     memcpy(pMessage->Payload.PhyDataInd.Data, (uint8_t*)&appMessage, sizeof(ApplicationMessage_t));
                     osStat = osMessagePut(AppQ, (uint32_t)pMessage, 0);
                     if (osOK != osStat)
                     {
                        /* failed to write */
                        osPoolFree(AppPool, pMessage);
                     }
                  }

                  MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
               }
            }
            break;
            case CO_MESSAGE_RESET_EVENT_E:
				{
					int* flags = (int*) pMsg->Payload.PhyDataReq.Data;
					
					if ( *flags & PARAM_RESET_EVENTS )
					{
#ifdef CANCEL_OUTPUTS_ON_RESET_MESSAGE
						//reset the output states to their start-up values
						DM_OP_Initialise(false);
						//Remove the LED patterns for FIRE and FIRST AID
						DM_LedPatternRemove(LED_FIRE_INDICATION_E);
						DM_LedPatternRemove(LED_FIRST_AID_INDICATION_E);
#endif
						// Reset AOS state
						DM_OP_ResetAlarmOutputState( );
						
						//Send any existing alarm states
						MM_ApplicationSendAlarmSignals();
						
						//cancel the PIR latch
						MM_ApplicationHandlePirEvent(PIR_EVENT_RESET_E);
					}
					
					if ( *flags & PARAM_RESET_FAULTS )
					{
						//Send current fault states and battery status on DULCH					
						MM_ApplicationSendFaultReports( !!( *flags & PARAM_RESET_DULCH ) );
					}
#ifdef SEND_BATTERY_ON_RESET						
               DM_BAT_SendBatteryStatus(true);
#endif

               break;
            }
            case CO_MESSAGE_SET_OUTPUT_E:
            {
               //Called by the TimedEventtask when an output needs to be set.
               CO_OutputData_t* pOutputData = (CO_OutputData_t*)pMsg->Payload.PhyDataReq.Data;
               if ( pOutputData )
               {
                  //Range check the enum parameters
                  if ( (CO_CHANNEL_MAX_E > pOutputData->OutputChannel) && (CO_PROFILE_MAX_E > pOutputData->OutputProfile) )
                  {
                     DM_OP_SetAlarmState((CO_ChannelIndex_t)pOutputData->OutputChannel, (CO_OutputProfile_t)pOutputData->OutputProfile, pOutputData->OutputsActivated, false);
                  }
               }
            }
            break;
            
            case CO_MESSAGE_SENSOR_DATA_E:
            {
               //Called by the Plugin Interface when new analogue values are received from the plug-in.
               CO_RBUSensorData_t* pSensorData = (CO_RBUSensorData_t*)pMsg->Payload.PhyDataReq.Data;
               if ( pSensorData )
               {
                  CO_PRINT_B_2(DBG_INFO_E,"App rx'd sensor reading.  Channel=%d, value=%d\r\n", pSensorData->RUChannelIndex, pSensorData->SensorValue);
                  if ( CO_CHANNEL_SMOKE_E == pSensorData->RUChannelIndex )
                  {
                     CO_PRINT_B_2(DBG_INFO_E,"App smoke reading changed from %d to %d\r\n", gSmokeAnalogueValue, pSensorData->SensorValue);
                     gSmokeAnalogueValue = pSensorData->SensorValue;
                     gWaitingForSmokeAnalogueValue = false;
                  }
                  else if ( (CO_CHANNEL_HEAT_A1R_E == pSensorData->RUChannelIndex) || (CO_CHANNEL_HEAT_B_E == pSensorData->RUChannelIndex) )
                  {
                     CO_PRINT_B_2(DBG_INFO_E,"App heat reading changed from %d to %d\r\n", gHeatAnalogueValue, pSensorData->SensorValue);
                     gHeatAnalogueValue = pSensorData->SensorValue;
                     gWaitingForHeatAnalogueValue = false;
                  }
                  else if ( CO_CHANNEL_PIR_E == pSensorData->RUChannelIndex )
                  {
                     CO_PRINT_B_2(DBG_INFO_E,"App PIR reading changed from %d to %d\r\n", gPirAnalogueValue, pSensorData->SensorValue);
                     gPirAnalogueValue = pSensorData->SensorValue;
                     gWaitingForPirAnalogueValue = false;
                  }
               }
            }
               break;
            
            case CO_MESSAGE_REQUEST_SENSOR_DATA_E:
               MM_RBU_RequestSensorValues();
               break;
            
            case CO_MESSAGE_GENERATE_ATTX_COMMAND_E:
            {
               AtMessageData_t* pCommand = (AtMessageData_t*)pMsg->Payload.PhyDataReq.Data;
               if( pCommand )
               {
                  if ( false == MM_MeshAPIGenerateAtMessage(pCommand) )
                  {
                     CO_PRINT_B_0(DBG_INFO_E,"Failed to send AT Command to the mesh\r\n");
                  }
               }
               break;
            }
            case CO_MESSAGE_GENERATE_ATTX_RESPONSE_E:
            {
               char* pResponse = (char*)pMsg->Payload.PhyDataReq.Data;
               if ( pResponse )
               {
                  AtMessageData_t AtResponse;
                  AtResponse.Source = MeshAtCommand.Destination;
                  AtResponse.Destination = MeshAtCommand.Source;
                  AtResponse.IsCommand = 0;
                  uint32_t response_length = strlen(pResponse);
                  if ( MAX_OTA_AT_COMMAND_LENGTH < response_length )
                  {
                     response_length = MAX_OTA_AT_COMMAND_LENGTH;
                  }
                  memset(AtResponse.Command, 0, MAX_OTA_AT_COMMAND_LENGTH);
                  memcpy(&AtResponse.Command, pResponse, response_length);
                  CO_PRINT_B_2(DBG_INFO_E,"Sending AT Response to node %d : %s\r\n", AtResponse.Destination, pResponse);
                  CO_PRINT_B_1(DBG_INFO_E,"AT RESPONSE PPU STATE = %d\r\n", MC_GetPpuMode());
                  if ( PPU_STATE_NOT_ACTIVE_E == MC_GetPpuMode() )
                  {
                     //This is an OTA AT command, not PPU.
                     // MeshAtCommandInProgress should be true.  If it isn't then the data in MeshAtCommand is stale.
                     if ( MeshAtCommandInProgress )
                     {
                        MeshAtCommandInProgress = false;
                        CO_PRINT_B_2(DBG_INFO_E,"Sending AT Response to node %d : %s\r\n", AtResponse.Destination, pResponse);
                     
                        if ( false  == MM_MeshAPIGenerateAtMessage( &AtResponse ) )
                        {
                           CO_PRINT_B_0(DBG_INFO_E,"Failed to send AT Response to the mesh\r\n");
                        }
                     }
                     else 
                     {
                        CO_PRINT_B_1(DBG_INFO_E,"App Rx'd AT Response with no command outstanding : %s\r\n", pResponse);
                     }
                  }
                  else if ( PPU_STATE_CONNECTED_E == MC_GetPpuMode() )
                  {
                     //We are in PPU mode.  Send the response back that way.
                     MM_PPU_ProcessAtResponseMessage(&AtResponse);
                  }
               }
               break;
            }
            case CO_MESSAGE_GENERATE_START_OTA_AT_MODE_E:
            {
               OtaMode_t* pCommand = (OtaMode_t*)pMsg->Payload.PhyDataReq.Data;
               if( pCommand )
               {
                  if ( false == MM_MeshAPIGenerateStartAtModeMessage(pCommand) )
                  {
                     CO_PRINT_B_0(DBG_INFO_E,"Failed to send AT Mode to the mesh\r\n");
                  }
               }
               break;
            }
            case CO_MESSAGE_GENERATE_PING_REQUEST_E:
            {
               PingRequest_t* pPingReq = (PingRequest_t*)pMsg->Payload.PhyDataReq.Data;
               MM_MeshAPIGeneratePingRequest(pPingReq);
               break;
            }
            case CO_MESSAGE_NVM_CLEANUP_E:
                CO_PRINT_B_0(DBG_INFO_E,"App Rxd NVM Clean-up message\r\n");
                //A NVM cleanup was done.  Check that we didn't miss a wake-up and lose synch.
                MM_MeshAPIApplicationCommand(APP_MSG_CHECK_TDM_SYNCH_E, 0, 0, 0, 0);
               break;
            default:
               break;
         }
         osPoolFree(AppPool, pMsg);
      }
   }
}

/*************************************************************************************/
/**
* MM_RBUApplicationUpdateStatusFlags
* Function to update the fault status flags.
*
* @param - FaultStatus - pointer to structure containing fault status update
*
* @return - bool         TRUE if the update changed the fault status flags.
*/
bool MM_RBUApplicationUpdateStatusFlags( CO_FaultData_t* FaultStatus )
{
   bool flags_changed = false;
   
   if( FaultStatus )
   {
      uint16_t temp_fault_status_flags = gFaultStatusFlags;

      CO_PRINT_B_4(DBG_INFO_E,"App flags update: chan=%d, type=%d, value=%d, old_flags=%X\r\n", 
      FaultStatus->RUChannelIndex, FaultStatus->FaultType, FaultStatus->Value, gFaultStatusFlags);
      switch ( FaultStatus->RUChannelIndex )
      {
         case CO_CHANNEL_NONE_E:
            if ( FaultStatus->Value )
            {
               if ( FAULT_SIGNAL_INSTALLATION_TAMPER_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags |= CO_INSTALLATION_TAMPER_FAULT_E;
               }
               else if ( FAULT_SIGNAL_DISMANTLE_TAMPER_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags |= CO_DISMANTLE_TAMPER_FAULT_E;
               }
               else if ( FAULT_SIGNAL_BATTERY_ERROR_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags |= CO_BATTERY_ERROR_E;
               }
               else if ( FAULT_SIGNAL_LOW_BATTERY_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags |= CO_BATTERY_WARNING_E;
               }
               else if ( FAULT_SIGNAL_HEAD_REMOVED_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags |= CO_DISMANTLE_HEAD_FAULT_E;
               }
               else if ( FAULT_SIGNAL_TYPE_MISMATCH_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags |= CO_MISMATCH_HEAD_FAULT_E;
               }
               else if ( FAULT_SIGNAL_DEVICE_ID_MISMATCH_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags |= CO_DEVICE_ID_MISMATCH_E;
               }
               else if ( FAULT_SIGNAL_INTERNAL_FAULT_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags |= CO_INTERNAL_FAULT_E;
                  if ( BIT_SOURCE_MAX_E > FaultStatus->Source )
                  {
                     gBitFaultStatus[FaultStatus->Source] = true;
                  }
               }
            }
            else
            {
               if ( FAULT_SIGNAL_INSTALLATION_TAMPER_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags &= ~CO_INSTALLATION_TAMPER_FAULT_E;
               }
               else if ( FAULT_SIGNAL_DISMANTLE_TAMPER_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags &= ~CO_DISMANTLE_TAMPER_FAULT_E;
               }
               else if ( FAULT_SIGNAL_BATTERY_ERROR_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags &= ~CO_BATTERY_ERROR_E;
               }
               else if ( FAULT_SIGNAL_LOW_BATTERY_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags &= ~CO_BATTERY_WARNING_E;
               }
               else if ( FAULT_SIGNAL_HEAD_REMOVED_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags &= ~CO_DISMANTLE_HEAD_FAULT_E;
               }
               else if ( FAULT_SIGNAL_TYPE_MISMATCH_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags &= ~CO_MISMATCH_HEAD_FAULT_E;
               }
               else if ( FAULT_SIGNAL_DEVICE_ID_MISMATCH_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags &= ~CO_DEVICE_ID_MISMATCH_E;
               }
               else if ( FAULT_SIGNAL_INTERNAL_FAULT_E == FaultStatus->FaultType )
               {
                  CO_PRINT_B_1(DBG_INFO_E,"Cancel FAULT_SIGNAL_INTERNAL_FAULT_E Source=%d\r\n", FaultStatus->Source);
                  if ( BIT_SOURCE_MAX_E > FaultStatus->Source )
                  {
                     gBitFaultStatus[FaultStatus->Source] = false;
                     
                     if ( false == MM_AppBitFailReported() )
                     {
                        temp_fault_status_flags &= ~CO_INTERNAL_FAULT_E;
                        CO_PRINT_B_1(DBG_INFO_E,"fault flags changed : %X\r\n", temp_fault_status_flags);
                     }
                     else 
                     {
                        CO_PRINT_B_3(DBG_INFO_E,"Internal fault maintained. Radio=%d, SVI=%d, Plugin=%d\r\n", gBitFaultStatus[BIT_SOURCE_RADIO_E], gBitFaultStatus[BIT_SOURCE_SVI_E], gBitFaultStatus[BIT_SOURCE_PLUGIN_E]);
                     }
                  }
               }
            }
            break;
         case CO_CHANNEL_SMOKE_E:
         case CO_CHANNEL_HEAT_B_E:
         case CO_CHANNEL_CO_E:
         case CO_CHANNEL_PIR_E:
         case CO_CHANNEL_HEAT_A1R_E:
            if ( FaultStatus->Value )
            {
               temp_fault_status_flags |= CO_DETECTOR_FAULT_E;
            }
            else
            {
               temp_fault_status_flags &= ~CO_DETECTOR_FAULT_E;
            }
            break;
         case CO_CHANNEL_BEACON_E:
            if ( FaultStatus->Value )
            {
               temp_fault_status_flags |= CO_BEACON_FAULT_E;
            }
            else
            {
               temp_fault_status_flags &= ~CO_BEACON_FAULT_E;
            }
           break;
         case CO_CHANNEL_SOUNDER_E:
            if ( FaultStatus->Value )
            {
               if ( FAULT_SIGNAL_INTERNAL_FAULT_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags |= CO_INTERNAL_FAULT_E;
                  gBitFaultStatus[BIT_SOURCE_SVI_E] = true;
               }
               else 
               {
                  temp_fault_status_flags |= CO_SOUNDER_FAULT_E;
               }
            }
            else
            {
               if ( FAULT_SIGNAL_INTERNAL_FAULT_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags &= ~CO_INTERNAL_FAULT_E;
                  gBitFaultStatus[BIT_SOURCE_SVI_E] = false;
               }
               else 
               {
                  temp_fault_status_flags &= ~CO_SOUNDER_FAULT_E;
               }
               
            }
           break;
         case CO_CHANNEL_RADIO_E:
            if ( FAULT_SIGNAL_INTERNAL_FAULT_E == FaultStatus->FaultType )
            {
               if ( FaultStatus->Value )
               {
                  temp_fault_status_flags |= CO_INTERNAL_FAULT_E;
                  gBitFaultStatus[BIT_SOURCE_RADIO_E] = true;
               }
               else
               {
                  temp_fault_status_flags &= ~CO_INTERNAL_FAULT_E;
                  gBitFaultStatus[BIT_SOURCE_RADIO_E] = false;
               }
            }
            break;
         case CO_CHANNEL_INPUT_1_E:
             if ( FaultStatus->Value )
            {
               temp_fault_status_flags |= CO_IO_INPUT_1_FAULT_E;
               
               if ( FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags |= CO_INPUT_SHORT_CIRCUIT_FAULT_E;
               }
               else if ( FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags |= CO_INPUT_OPEN_CIRCUIT_FAULT_E;
               }
               else if ( FAULT_SIGNAL_OUTPUT_FAULT_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags |= CO_OUTPUT_FAULT_E;
               }
            }
            else
            {
               temp_fault_status_flags &= ~CO_IO_INPUT_1_FAULT_E;
               
               if ( FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags &= ~CO_INPUT_SHORT_CIRCUIT_FAULT_E;
               }
               else if ( FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags &= ~CO_INPUT_OPEN_CIRCUIT_FAULT_E;
               }
               else if ( FAULT_SIGNAL_OUTPUT_FAULT_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags &= ~CO_OUTPUT_FAULT_E;
               }
            }
           break;
         case CO_CHANNEL_INPUT_2_E:
             if ( FaultStatus->Value )
            {
               temp_fault_status_flags |= CO_IO_INPUT_2_FAULT_E;
               
               if ( FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags |= CO_INPUT_SHORT_CIRCUIT_FAULT_E;
               }
               else if ( FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags |= CO_INPUT_OPEN_CIRCUIT_FAULT_E;
               }
               else if ( FAULT_SIGNAL_OUTPUT_FAULT_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags |= CO_OUTPUT_FAULT_E;
               }
            }
            else
            {
               temp_fault_status_flags &= ~CO_IO_INPUT_2_FAULT_E;
               
               if ( FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags &= ~CO_INPUT_SHORT_CIRCUIT_FAULT_E;
               }
               else if ( FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags &= ~CO_INPUT_OPEN_CIRCUIT_FAULT_E;
               }
               else if ( FAULT_SIGNAL_OUTPUT_FAULT_E == FaultStatus->FaultType )
               {
                  temp_fault_status_flags &= ~CO_OUTPUT_FAULT_E;
               }
            }
            break;
         default:
            CO_PRINT_B_1(DBG_INFO_E,"Fault Channel Unknown: %d\r\n", FaultStatus->RUChannelIndex);
            break;
      }

      
      if (temp_fault_status_flags != gFaultStatusFlags)
      {
         /* Update the Bit Map value */
         gFaultStatusFlags = temp_fault_status_flags;
         CO_PRINT_B_1(DBG_INFO_E, "App : Fault Status Flags updated to %x\r\n", gFaultStatusFlags);
         //set the yellow LED flash pattern
         MM_AppUpdateFaultStatusLED();
         
         flags_changed = true;
      }
   }
   
   return flags_changed;
}

/*************************************************************************************/
/**
* MM_AppUpdateFaultStatusLED
* Function to set the amber LED flash pattern to reflect the current fault conditions
* according to gFaultStatusFlags.
*
* @param - None.
*
* @return - void
*/
void MM_AppUpdateFaultStatusLED(void)
{
   //Check for single flash conditions
   if ( gFaultStatusFlags & LED_SINGLE_FLASH_FAULTS )
   {
      DM_LedPatternRequest(LED_INTERNAL_FAULT_E);
   }
   else 
   {
      DM_LedPatternRemove(LED_INTERNAL_FAULT_E);
   }
}

/*************************************************************************************/
/**
* MM_AppBitFailReported
* Returns TRUE if one of the RBU systems are reporting a BIT failure.
*
* @param - None.
*
* @return - bool  TRUE if BIT fail present, FALSE otherwise
*/
bool MM_AppBitFailReported(void)
{
   return (gBitFaultStatus[BIT_SOURCE_RADIO_E] | gBitFaultStatus[BIT_SOURCE_SVI_E] | gBitFaultStatus[BIT_SOURCE_PLUGIN_E]);
}

/*************************************************************************************/
/**
* MM_RBUApplicationFireSignalReq
* Function to send a fire signal request to the Mesh.
*
* @param - pSensorData Pointer to struct that holds the fire signal information
*
* @return - void
*/
void MM_RBUApplicationFireSignalReq(const CO_RBUSensorData_t* pSensorData)
{
   
   if ( pSensorData )
   {
      uint32_t Handle = GetNextHandle();
      
      CO_RBUSensorData_t alarmData;
      alarmData.AlarmState = pSensorData->AlarmState;
      alarmData.Handle =Handle;
      alarmData.HopCount = 0;
      alarmData.RUChannelIndex = pSensorData->RUChannelIndex;
      alarmData.SensorType = pSensorData->SensorType;
      alarmData.SensorValue = pSensorData->SensorValue;
      alarmData.Zone = gZoneNumber;
      
      if ( (CO_CHANNEL_MAX_E > alarmData.RUChannelIndex) && (DM_ChannelEnabled((CO_ChannelIndex_t)alarmData.RUChannelIndex)) )
      {
         CO_PRINT_B_1(DBG_INFO_E, "App sending Fire Signal Req Handle=%d\r\n", Handle);
      if ( false == MM_MeshAPIFireSignalReq( Handle, &alarmData ) )
      {
         CO_PRINT_B_0(DBG_INFO_E, "MM_RBUApplicationFireSignalReq FAILED\r\n");
      }
   }
   }
}

/*************************************************************************************/
/**
* MM_RBUApplicationAlarmSignalReq
* Function to send a alarm signal request to the Mesh.
*
* @param - pSensorData Pointer to struct that holds the alarm signal information
*
* @return - void
*/
void MM_RBUApplicationAlarmSignalReq(const CO_RBUSensorData_t* pSensorData)
{
   
   if ( pSensorData )
   {
      uint32_t Handle = GetNextHandle();

      CO_RBUSensorData_t alarmData;
      alarmData.AlarmState = pSensorData->AlarmState;
      alarmData.Handle = Handle;
      alarmData.HopCount = 0;
      alarmData.RUChannelIndex = pSensorData->RUChannelIndex;
      alarmData.SensorType = pSensorData->SensorType;
      alarmData.SensorValue = pSensorData->SensorValue;
      alarmData.Zone = gZoneNumber;

      if ( (CO_CHANNEL_MAX_E > alarmData.RUChannelIndex) && (DM_ChannelEnabled((CO_ChannelIndex_t)alarmData.RUChannelIndex)) )
      {
         CO_PRINT_B_4(DBG_INFO_E, "App sending Alarm Signal Req Handle=%d, chan=%d, Sensortype=%d, SensorValue=%d\r\n", Handle, pSensorData->RUChannelIndex, pSensorData->SensorType, pSensorData->SensorValue);
      if ( false == MM_MeshAPIAlarmSignalReq( Handle, &alarmData ) )
      {
         CO_PRINT_B_0(DBG_INFO_E, "MM_RBUApplicationAlarmSignalReq FAILED\r\n");
      }
   }
   }
}


/*************************************************************************************/
/**
* MM_RBUApplicationOutputSignalReq
* Function to send an output signal request to the Mesh.
*
* ### The NCU would normally generate this.  Do we need this functionality in the RBU? ######
*
* @param - pOutputData Pointer to struct that holds the Output signal information
*
* @return - void
*/
void MM_RBUApplicationOutputSignalReq(const CO_OutputData_t* pOutputData)
{
   CO_PRINT_B_0(DBG_INFO_E, "App Rx o/p Signal Req\r\n");
   
   if ( pOutputData )
   {
      uint32_t Handle = GetNextHandle();
      
      if ( false == MM_MeshAPIOutputSignalReq( Handle, pOutputData->Destination, pOutputData->zone, pOutputData->OutputChannel, pOutputData->OutputProfile, pOutputData->OutputsActivated, pOutputData->OutputDuration, 1 ) )
      {
         CO_PRINT_B_0(DBG_ERROR_E, "MM_RBUApplicationOutputSignalReq FAILED\r\n");
      }
   }
}


/*************************************************************************************/
/**
* MM_RBUApplicationLogonReq
* Function to send a logon request to the NCU.
*
* @param - void
*
* @return - void
*/
void MM_RBUApplicationLogonReq( )
{
   if ( gFreqLockAchieved )
   {
      /*CO_PRINT_B_0(DBG_INFO_E, "RBU Application Attempting to LOGON\r\n");*/
      
      /* TODO:  Populate these variables with data from the unit */
      uint32_t SerialNumber = 0;
      DM_NVMRead(NV_UNIT_SERIAL_NO_E, &SerialNumber, sizeof(uint32_t));
      uint32_t DeviceCombination = 0;
      DM_NVMRead(NV_DEVICE_COMBINATION_E, &DeviceCombination, sizeof(uint32_t));


      logonRequestHandle = GetNextHandle();
      
      //const uint32_t Handle,const uint32_t SerialNumber, const uint8_t DeviceCombination, const uint16_t StatusFlags
      if ( false == MM_MeshAPILogonReq( logonRequestHandle, SerialNumber, (uint8_t)DeviceCombination, gZoneNumber ) )
      {
         /* Failed to send logon message.  Probably because the Mesh is not established yet. 
            Force a retry */
         logonRequestHandle = 0;
      }
      else
      {
         CO_PRINT_B_0(DBG_INFO_E, "Logon Message SENT\r\n");
         rbu_state = RBU_WAITING_FOR_LOGON_E;
      }
   }
}



/*************************************************************************************/
/**
* MM_RBUProcessMACEvent
* Function for processing a received network message.
*
* @param pEventData   The contents of the event.
*
* @return - void
*/
void MM_RBUProcessMACEvent(AppMACEventData_t* pEventData)
{
   if ( pEventData )
   {
      switch ( pEventData->EventType )
      {
         case CO_NEW_LONG_FRAME_E:
            CO_PRINT_B_1(DBG_INFO_E,"Long frame %d\r\n", pEventData->Value);
            break;
         case CO_JOINING_PHASE_COMPLETE_E:
            gJoiningPhaseComplete = true;
            CO_PRINT_B_0(DBG_INFO_E,"JOINING PHASE COMPLETE\r\n");
            break;
         case CO_ENTER_PPU_OTA_MODE_E:
            FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
            
            //rbu_pp_mode_request = RBU_RADIO_PP_MODE_REQ;
            rbu_pp_mode_request = STARTUP_RADIO_BOOTLOADER;
  
            /* Request a micro reset */
            Error_Handler("PPU Master Requested PPU Mode");;
            break;
         default:
            break;
      }
   }
}

/*************************************************************************************/
/**
* MM_RBUApplicationProcessNetworkMessage
* Function for processing a received network message.
*
* @param pMessageData   The contents of the message.
*
* @return - void
*/
void MM_RBUApplicationProcessNetworkMessage(ApplicationMessage_t* pMessage)
{
   CO_PRINT_B_0(DBG_INFO_E, "MM_RBUApplicationProcessNetworkMessage\r\n");
	
   if ( pMessage )
   {
      switch ( pMessage->MessageType )
      {
         case APP_MSG_TYPE_OUTPUT_SIGNAL_E:
         {
            CO_PRINT_B_0(DBG_INFO_E, "APP_MSG_TYPE_OUTPUT_SIGNAL_E\r\n"); 
				
            OutputSignalIndication_t outData;
            memcpy(&outData, pMessage->MessageBuffer, sizeof(OutputSignalIndication_t));
				
				CO_ChannelIndex_t channelIndex = (CO_ChannelIndex_t)outData.OutputChannel;
				
				DM_OP_OutputChannel_t op_channel = DM_OP_SystemChannelToOutputChannel( channelIndex );
		
				// if disabled
				if ( DM_OP_ChannelDisabled( channelIndex ) && outData.OutputsActivated != 0 )
				{
					//change to OFF
					outData.OutputsActivated = 0;
					CO_PRINT_B_0( DBG_INFO_E, "Setting actual alarm state to off\n" );

				}	
				
            DM_OP_ProcessOutputMessage(&outData);
            //if alarms are enabled, cancel the battery test
            if ( outData.OutputsActivated )
            {
               /* Cancel the battery test */
               DM_BAT_Cancel();

               /* Check if the battery test is cancelled or not: If cancelled then set the battery test restart flag to resume the battery test */
               if( ( gBatteryTestStartFlag ) && ( !gBatteryTestCompleteFlag ) ) 
               {
                  /* Set the flag to restart the battery test */
                  gBatteryTestRestartFlag = 1;
						gBatteryTestStartFlag = 0U;
                  gBatteryTestDelay = 1;
								 
               }

            }
            //Check for non-latching first aid button to cancel long-press checks when the alarm is cancelled
            if ( (CO_PROFILE_FIRST_AID_E == outData.OutputProfile) && ( false == outData.OutputsActivated) )
            {
               //it's a first aid cancellation.  Do we have a non-latching button?
               if ( CFG_HasNonLatchingMedicalCallPoint() )
               {
                  //Cancel the non-latching button
                  DM_FA_SetAlarmState(false);
               }
            }
         }
            break;
         case APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E:
         {
            CO_AlarmOutputStateData_t alarmData;
            memcpy(&alarmData, pMessage->MessageBuffer, sizeof(CO_AlarmOutputStateData_t));
            CO_PRINT_B_3(DBG_INFO_E, "APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E silenceable=%x, unsilenceable=%x, delayMask=%x\r\n", alarmData.Silenceable, alarmData.Unsilenceable, alarmData.DelayMask);
            ErrorCode_t result = DM_OP_ProcessAlarmOutputStateCommand(&alarmData, false );
            if ( SUCCESS_E == result )
            {
               //if alarms are enabled, cancel the battery test
               if ( alarmData.Silenceable || alarmData.Unsilenceable )
               {
                  /* Cancel the battery test */
                  DM_BAT_Cancel();

                  /* Check if the battery test is cancelled or not: If cancelled then set the battery test restart flag to resume the battery test */
                  if( (gBatteryTestStartFlag) && (!gBatteryTestCompleteFlag) ) 
                  {
                     /* Set the flag to restart the battery test */
                     gBatteryTestRestartFlag = 1;
							gBatteryTestStartFlag = 0U;
                     gBatteryTestDelay = 1;
                  }
               }
            }
            else
            {
               CO_PRINT_B_1(DBG_INFO_E,"Failed to process ALARM OUTPUT STATE SIGNAL.  Error=%d\r\n", result);
            }
            
            //Check for non-latching first aid button to cancel long-press checks when the alarm is cancelled
            //Get the channel profile from NVM
            AlarmOutputStateConfig_t  alarmConfig;
            if ( SUCCESS_E == DM_OP_GetChannelOutputConfig(CO_CHANNEL_MEDICAL_CALLPOINT_E, &alarmConfig) )
            {
               uint32_t bitField;
               //Identify the appropriate bit field from the received message
               if ( alarmConfig.Silenceable )
               {
                  bitField = alarmData.Silenceable;
               }
               else 
               {
                  bitField = alarmData.Unsilenceable;
               }
               //it's a first aid cancellation.  Do we have a non-latching button?
               if ( CFG_HasNonLatchingMedicalCallPoint() )
               {
                  if ( 0 == (DM_OP_BIT_FIRST_AID & bitField) )
                  {
                     //Cancel the non-latching button
                     DM_FA_SetAlarmState(false);
                  }
               }
            }
         }
            break;
         case APP_MSG_TYPE_COMMAND_E:
            MM_RBUApplicationProcessCommandMessage(pMessage);
            break;
         case APP_MSG_TYPE_RESPONSE_E:
            MM_RBUApplicationProcessResponseMessage(pMessage);
            break;
         case APP_MSG_TYPE_STATE_SIGNAL_E:
            MM_RBUApplicationProcessSetStateMessage(pMessage);
            break;
         case APP_MSG_TYPE_RBU_DISABLE_E:
            MM_RBUApplicationProcessRBUDisableMessage(pMessage);
            break;
         case APP_MSG_TYPE_STATUS_INDICATION_E:
            MM_RBUApplicationProcessStatusIndication(pMessage);
            break;
         case APP_MSG_TYPE_ZONE_ENABLE_E:
            MM_RBUApplicationProcessZoneEnableMessage(pMessage);
            break;
         case APP_MSG_TYPE_AT_COMMAND_E:
            MM_RBUApplicationProcessAtCommandMessage(pMessage);
            break;
         case APP_MSG_TYPE_PPU_MODE_E:
            MM_PPU_ApplicationProcessPpuModeMessage(pMessage);
            break;
         case APP_MSG_TYPE_EXIT_TEST_MODE_E:
            MM_ApplicationProcessExitTestMessage(pMessage);
            break;				 
         case APP_MSG_TYPE_PPU_COMMAND_E:
            CO_PRINT_B_0(DBG_INFO_E,"App Rxd PPU Command\r\n");
            MM_PPU_ApplicationProcessPpuCommand(pMessage);
            break;
         case APP_MSG_TYPE_GLOBAL_DELAYS_E:
            MM_RBUApplicationProcessGlobalDelaysMessage(pMessage);
            break;
         default:
            break;
      }
   }
}


/*************************************************************************************/
/**
* MM_RBUApplicationProcessMeshStatusEvent
* Function for processing status events received from the Mesh stack.
*
* @param pEventData     Pointer to the status indication structure.
*
* @return - bool        True if the RBU should send a status message to the NCU.
*/
bool MM_RBUApplicationProcessMeshStatusEvent(const CO_StatusIndicationData_t* pEventData)
{
   bool sendStatusIndication = false;
   
   if ( pEventData )
   {
#ifdef DEFAULT_SEND_STATUS_MSG		 
      sendStatusIndication = true;
#endif
      switch ( pEventData->Event )
      {
         case CO_MESH_EVENT_CHILD_NODE_ADDED_E:
            CO_PRINT_B_2(DBG_INFO_E, "App : Added new child ID=%d isPrimary=%d\r\n", pEventData->EventNodeId, pEventData->EventData);
            MM_ApplicationAddChild(pEventData->EventNodeId, pEventData->EventData);
				sendStatusIndication = true;
            break;
         case CO_MESH_EVENT_CHILD_NODE_DROPPED_E:
            CO_PRINT_B_1(DBG_INFO_E, "App : Dropped child ID=%d\r\n", pEventData->EventNodeId);
            MM_ApplicationDropChild(pEventData->EventNodeId);
				sendStatusIndication = true;
            break;
         case CO_MESH_EVENT_PRIMARY_PARENT_ADDED_E:
            {
               CO_PRINT_B_1(DBG_INFO_E, "App : Added Primary Parent ID=%d\r\n", pEventData->EventNodeId);

               //Start the timer for sending initial fault reports if we haven't already
               if ( false == gPrimaryParentConnected )
               {
                  gDelayedStatusReportCount = STARTUP_STATUS_REPORT_DELAY;
                  gPrimaryParentConnected = true;
               }
					sendStatusIndication = true;
            }
            break;
         case CO_MESH_EVENT_PRIMARY_PARENT_DROPPED_E:
            CO_PRINT_B_1(DBG_INFO_E, "App : Dropped Primary Parent ID=%d\r\n", pEventData->EventNodeId);
				sendStatusIndication = true;
            break;
         case CO_MESH_EVENT_SECONDARY_PARENT_ADDED_E:
            CO_PRINT_B_1(DBG_INFO_E, "App : Added Secondary Parent ID=%d\r\n", pEventData->EventNodeId);
				sendStatusIndication = true;
            break;
         case CO_MESH_EVENT_SECONDARY_PARENT_DROPPED_E:
            CO_PRINT_B_1(DBG_INFO_E, "App : Dropped Secondary Parent ID=%d\r\n", pEventData->EventNodeId);
				sendStatusIndication = true;
            break;
         case CO_MESH_EVENT_PRIMARY_TRACKING_NODE_ADDED_E:
            CO_PRINT_B_1(DBG_INFO_E, "App : Added Primary Tracking Node ID=%d\r\n", pEventData->EventNodeId);
            break;
         case CO_MESH_EVENT_PRIMARY_TRACKING_NODE_DROPPED_E:
            CO_PRINT_B_1(DBG_INFO_E, "App : Dropped Primary Tracking Node ID=%d\r\n", pEventData->EventNodeId);
            break;
         case CO_MESH_EVENT_SECONDARY_TRACKING_NODE_ADDED_E:
            CO_PRINT_B_1(DBG_INFO_E, "App : Added Secondary Tracking Node ID=%d\r\n", pEventData->EventNodeId);
            break;
         case CO_MESH_EVENT_SECONDARY_TRACKING_NODE_DROPPED_E:
            CO_PRINT_B_1(DBG_INFO_E, "App : Dropped Secondary Tracking Node ID=%d\r\n", pEventData->EventNodeId);
            break;
         case CO_MESH_EVENT_SECONDARY_PARENT_PROMOTED_E:
            CO_PRINT_B_1(DBG_INFO_E, "App : Promoted Secondary Parent to Primary parent ID=%d\r\n", pEventData->EventNodeId);
				sendStatusIndication = true;
            break;
         case CO_MESH_EVENT_PRIMARY_TRACKING_NODE_PROMOTED_E:
            CO_PRINT_B_1(DBG_INFO_E, "App : Promoted Primary Tracking Node to Secondary Parent ID=%d\r\n", pEventData->EventNodeId);
            break;
         case CO_MESH_EVENT_ZONE_CHANGE_E:
            gZoneNumber = pEventData->EventData;
            CO_PRINT_B_1(DBG_INFO_E, "App : Zone number changed to %d\r\n", gZoneNumber);
            sendStatusIndication = true;
            break;
         case CO_MESH_EVENT_STATE_CHANGE_E:
         {
            MM_ApplicationUpdateDeviceState(pEventData->EventData);
            if ( STATE_ACTIVE_E == pEventData->EventData )
            {
#ifdef SEND_STATUS_ON_STATE_ACTIVE						 
					sendStatusIndication = true;
#endif
               MC_ResetTimeActive();
               
               //Send all faults
                if ( 0 < gDelayedStatusReportCount )
                {
                   //The timer event decrements this and sends the fault status on reaching 0.
                   gDelayedStatusReportCount = 1;
                }
                
               //If we have a battery status message on delay, trigger it now.
               if ( gBatteryStatusMessageReschuled )
               {
                  //The timer event decrements this and sends the battery status on reaching 0.
                  gBatteryStatusMessageReschuled = 1;
               }
            }
         }
            break;
         case CO_MESH_EVENT_INITIAL_TRK_NODE_LOST_E:
         {
            gLostConnectionToMesh = true;
         }
         break;
         case CO_MESH_EVENT_SYNCH_LOSS_E:
            CO_PRINT_B_0(DBG_INFO_E, "App : Synch loss reported\r\n");
            break;
         case CO_MESH_EVENT_COMMANDED_RESET_E:
            CO_PRINT_B_0(DBG_INFO_E, "App : Commanded Reset\r\n");
            break;
         case CO_MESH_EVENT_COMMS_FAIL_E:
            CO_PRINT_B_0(DBG_INFO_E, "App : Radio Comms Failure\r\n");
            break;
         case CO_MESH_EVENT_PPU_MODE_REQUESTED_E:
            CO_PRINT_B_0(DBG_INFO_E, "App : PPU Mode Requested\r\n");
            break;
         default:
            CO_PRINT_B_1(DBG_ERROR_E, "App : Received unknown mesh event type=%d\r\n", pEventData->Event);
            sendStatusIndication = false;
            break;
      }
   }
   
   return sendStatusIndication;
}

/*************************************************************************************/
/**
* MM_RBUApplicationProcessCommandMessage
* Function for processing command messages received from the Mesh stack.
*
* @param pAppMessage     Pointer to the message structure.
*
* @return - None.
*/
void MM_RBUApplicationProcessCommandMessage(const ApplicationMessage_t* pAppMessage)
{
   static uint8_t last_command_transaction = 9;
   
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_COMMAND_E == pAppMessage->MessageType )
      {
         CO_CommandData_t* pCommandData = (CO_CommandData_t*)pAppMessage->MessageBuffer;
         if ( pCommandData )
         {
            if ( pCommandData->TransactionID != last_command_transaction )
            {
               last_command_transaction = pCommandData->TransactionID;
               ErrorCode_t result = MM_CMD_ProcessCommand ( pCommandData, CMD_SOURCE_MESH_E );
               if ( SUCCESS_E != result )
               {
                  CO_PRINT_B_1(DBG_ERROR_E, "RBU APP FAILED to process command.  Error=%d\r\n", result);
                  /* create a response */
                  CO_ResponseData_t response = {0};
                  response.CommandType = pCommandData->CommandType;
                  response.Source = pCommandData->Source;
                  response.Destination = pCommandData->Destination;
                  response.Parameter1 = pCommandData->Parameter1;
                  response.Parameter2 = pCommandData->Parameter2;
                  response.ReadWrite = pCommandData->ReadWrite;
                  response.TransactionID = pCommandData->TransactionID;
                  response.Value = 0xFFFFFFFF;
                  /* respond over Mesh link */
                  bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                             response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
               }
               
               //See if the PIR latch needs to be cancelled
               if ( PARAM_TYPE_PIR_RESET_E == pCommandData->CommandType )
               {
                  HeadMessage_t response;
                  response.MessageType = HEAD_RESET_PIR_E;
                  response.TransactionID = pCommandData->TransactionID;
                  response.Value = 0;
                  //Send the response
                  MM_CMD_ProcessHeadMessage ( &response );
                  gPirLatched = 0;//setting this here causes the reset function to skip sending an alarm-cancel message to the panel
                  MM_ApplicationHandlePirEvent(PIR_EVENT_RESET_E);
               }
            }
            else
            {
               CO_PRINT_B_1(DBG_INFO_E, "RBU APP rejected duplicate command. TransactionID=%d\r\n", pCommandData->TransactionID);
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_RBUApplicationProcessResponseMessage
* Function for processing response messages received from the Mesh stack.
*
* @param pAppMessage    Pointer to the App Message structure.
*
* @return - None.
*/
void MM_RBUApplicationProcessResponseMessage(const ApplicationMessage_t* pAppMessage)
{
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_RESPONSE_E == pAppMessage->MessageType )
      {
         CO_ResponseData_t* pResponseData = (CO_ResponseData_t*)pAppMessage->MessageBuffer;
         if ( pResponseData )
         {
            MM_CMD_ProcessMeshResponse ( pResponseData->TransactionID, pResponseData->Source, pResponseData->CommandType, 
                                          pResponseData->Parameter1, pResponseData->Parameter2, pResponseData->Value, pResponseData->ReadWrite );
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_RBUApplicationProcessSetStateMessage
* Function for processing Set State messages received from the Mesh stack.
*
* @param pAppMessage    Pointer to the App Message structure.
*
* @return - None.
*/
void MM_RBUApplicationProcessSetStateMessage(const ApplicationMessage_t* pAppMessage)
{
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_STATE_SIGNAL_E == pAppMessage->MessageType )
      {
#if defined _DEBUG
         uint8_t newState = pAppMessage->MessageBuffer[0];
#endif
         CO_PRINT_B_1(DBG_INFO_E,"RBU APP set state to %d\r\n", newState);
      }
   }
}

/*************************************************************************************/
/**
* MM_RBUApplicationProcessRBUDisableMessage
* Function for processing RBU Disable messages received from the Mesh stack.
*
* @param pAppMessage    Pointer to the App Message structure.
*
* @return - None.
*/
void MM_RBUApplicationProcessRBUDisableMessage(const ApplicationMessage_t* pAppMessage)
{
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_RBU_DISABLE_E == pAppMessage->MessageType )
      {
         uint16_t* pNodeID = (uint16_t*)pAppMessage->MessageBuffer;
         if ( pNodeID )
         {
            CO_PRINT_B_1(DBG_INFO_E,"RBU APP received RBU Disable command for node %d\r\n", *pNodeID);
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_RBUApplicationProcessConfirmation
* Function for processing Confirmation messages from the Mesh Stack.
*
* @param pAppMessage    Pointer to the App Message structure.
*
* @return - None.
*/
void MM_RBUApplicationProcessConfirmation(const ApplicationMessage_t* pAppMessage)
{
   if ( pAppMessage )
   {
      if ( CO_MESSAGE_CONFIRMATION_E == pAppMessage->MessageType )
      {
         AppRequestConfirmation_t* pConfirmation = (AppRequestConfirmation_t*) pAppMessage->MessageBuffer;
         if ( pConfirmation )
         {
            switch ( pConfirmation->ConfirmationType )
            {
               case APP_CONF_FIRE_SIGNAL_E:
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx Fire Signal Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
                  break;
               case APP_CONF_ALARM_SIGNAL_E:
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx First Aid Signal Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
                  break;
               case APP_CONF_OUTPUT_SIGNAL_E:
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx Output Signal Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
                  break;
               case APP_CONF_LOGON_SIGNAL_E:
                  MM_RBUApplicationLogonConfirmation(pConfirmation->Handle, pConfirmation->Error);
                  break;
               case APP_CONF_STATUS_SIGNAL_E:
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx Status Signal Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
                  break;
               case APP_CONF_COMMAND_SIGNAL_E:
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx Command Signal Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
                  break;
               case APP_CONF_RESPONSE_SIGNAL_E:
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx Response Signal Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
                  break;
               case APP_CONF_SET_STATE_E:
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx Set State Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
                  break;
               case APP_CONF_RBU_DISABLE_E:
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx RBU Disable Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
                  break;
               case APP_CONF_ALARM_OUTPUT_STATE_SIGNAL_E:
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx Alarm Output State Signal Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
                  break;
               case APP_CONF_FAULT_SIGNAL_E:
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx Fault Signal Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
                  break;
               case APP_CONF_BATTERY_STATUS_SIGNAL_E:
                  if ( ERR_MESSAGE_FAIL_E == pConfirmation->Error )
                  {
                     gBatteryStatusMessageReschuled = BATTERY_STATUS_RESCHEDULE_PERIOD;
                  }
                  else 
                  {
#ifdef SEND_FOLLOW_UP_STATUS_MSG							
                     //Follow-up a battery status with a mesh status report so that the panel picks up the new battery voltages
                     BatteryTestResult_t battery_status = DM_BAT_GetBatteryStatus();
                     uint32_t handle = GetNextHandle();
							uint8_t overall_fault = MM_RBU_GetOverallFaults( );
                     
                     MM_MeshAPIGenerateStatusSignalReq( handle, CO_MESH_EVENT_BATTERY_STATUS_E, gNodeAddress, (uint32_t)battery_status, overall_fault, true );
#endif                     
                     //only send a fault message if there are no faults or warnings and it is enabled in the config
                     if ( CFG_GetFaultsClearEnable( ) )
                     {
								uint8_t overall_fault = MM_RBU_GetOverallFaults( );
								if ( 0 == overall_fault )
								{
									CO_SendFaultSignal(BIT_SOURCE_NONE_E,  CO_CHANNEL_NONE_E, FAULT_SIGNAL_NO_ERROR_E, 0, true, true );
								}
                     }
                  }
                  
                  if ( DM_BAT_BatteryDepleted() )
                  {
                     //The primary and backup battery fault has been successfully sent
                     //Tell the Application to go to sleep.
                     osStatus osStat = osErrorOS;
                     CO_Message_t *pMessage = NULL;

                     /* create a status message and put into the App queue */
                     pMessage = osPoolAlloc(AppPool);
                     if ( pMessage )
                     {
                        pMessage->Type = CO_MESSAGE_BATTERY_STATUS_MESSAGE_E;
                        
                        osStat = osMessagePut(AppQ, (uint32_t)pMessage, 0);
                        if (osOK != osStat)
                        {
                           /* failed to write */
                           osPoolFree(AppPool, pMessage);
                        }
                     }
                  }
                  break;
               case APP_CONF_ROUTE_DROP_SIGNAL_E:
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx Route Drop Signal Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
                  MM_PPU_ProcessRouteDropConfirmation(pConfirmation->Handle);
                  break;
               default:
                  break;
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_RBUApplicationLogonConfirmation
* Function for processing Logon Confirmation messages from the Mesh Stack.
*
* @param    Handle    The returned message ID for matching to the logon request message.
* @param    Status    The error code of the logon request.
*
* @return - None.
*/
void MM_RBUApplicationLogonConfirmation(const uint32_t Handle, const uint32_t Status)
{
    CO_PRINT_B_0(DBG_INFO_E, "App Rx Logon Confirmation\r\n");   
   
   if ( RBU_WAITING_FOR_LOGON_E == rbu_state )
   {
      if ( Handle == logonRequestHandle )
      {
         if ( SUCCESS_E == Status )
         {
            /* Logon was successful.  move on to normal operation */
            rbu_state = RBU_OPERATIONAL_E;
            CO_PRINT_B_0(DBG_INFO_E, "Logon SUCCESS.\r\n");
         }
         else
         {
            /* Logon failed.  Reset and try again */
            Error_Handler("Logon FAILED.  Resetting");
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_RBUApplicationProcessStatusIndication
* Function to receive status information from the Mesh.
*
* @param pAppMessage   The contents of the message.
*
* @return - void
*/
void MM_RBUApplicationProcessStatusIndication(const ApplicationMessage_t* pAppMessage)
{
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_STATUS_INDICATION_E == pAppMessage->MessageType )
      {
         StatusIndication_t* pStatus = (StatusIndication_t*)pAppMessage->MessageBuffer;

         if ( pStatus )
         {
            /* convert 6-bit SNR to 32 bit, retaining the sign */
//            int32_t primaryParentRssi = pStatus->AverageRssiPrimaryParent;
//            if ( 0 != (primaryParentSNR & 0x20) )
//            {
//               primaryParentSNR |= 0xffffffC0;
//            }
//            int32_t secondaryParentSNR = pStatus->AverageSNRSecondaryParent;
//            if ( 0 != (secondaryParentSNR & 0x20) )
//            {
//               secondaryParentSNR |= 0xffffffC0;
//            }
            
            //Get rank
            NeighbourInformation_t neighbour;
            if ( false == MM_NI_GetNeighbourInformation(gNodeAddress, &neighbour) )
            {
               neighbour.Rank = UNASSIGNED_RANK;
               neighbour.backupBattery_mv = 0;
               neighbour.primaryBattery_mv = 0;
            }

           
            /* build a message string and push it to the debug output */
            char AtBuffer[AT_RESPONSE_MAX_LENGTH] = {0};
            snprintf(AtBuffer, AT_RESPONSE_MAX_LENGTH, "MSR:Z%dU%d,%d,%d,9,%d,%d,%d,%d,%d,%d,%d,%d,%d\r\n", gZoneNumber, gNodeAddress,
               neighbour.primaryBattery_mv, neighbour.backupBattery_mv,
               pStatus->PrimaryParent, pStatus->SecondaryParent, pStatus->AverageRssiPrimaryParent, pStatus->AverageRssiSecondaryParent, 
               pStatus->Event, pStatus->EventData, pStatus->Rank, gDeviceCombination, pStatus->OverallFault);

            /* respond over AT USART link */
            uint32_t bufferSize = strlen(AtBuffer);
            UartComm_t source = MM_CMD_ConvertToATCommandSource((CommandSource_t)pAppMessage->Source);
            
            if ( DEBUG_UART_E == source )
            {
               /* The debug port is a special case because it is mutex protected */
               CO_PRINT_A_1(DBG_NOPREFIX_E,"%s\r\n",AtBuffer);
            }
            else 
            {
               SerialPortWriteTxBuffer(source, (uint8_t*)AtBuffer, bufferSize);
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_RBUApplicationProcessZoneEnableMessage
* Function to receive Zone Enablement information from the Mesh.
*
* @param pAppMessage   The contents of the message.
*
* @return - void
*/
void MM_RBUApplicationProcessZoneEnableMessage(const ApplicationMessage_t* pAppMessage)
{
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_ZONE_ENABLE_E == pAppMessage->MessageType )
      {
         ZoneEnableData_t* pZoneData = (ZoneEnableData_t*)pAppMessage->MessageBuffer;

         if ( pZoneData )
         {
            //The received message is for 'enabled' zones, but the local map is for 'disabled' zones so the rx'd value nust be inverted
            uint16_t half_word = ~pZoneData->HalfWord;
            uint32_t word = ~pZoneData->Word;
            //Update the zone flags
            CFG_SetSplitZoneDisabledMap(pZoneData->LowHigh, word, half_word);
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_RBUApplicationProcessAtCommandMessage
* Function to receive AT Commands from the Mesh.
*
* @param pAppMessage   The contents of the message.
*
* @return - void
*/
void MM_RBUApplicationProcessAtCommandMessage(const ApplicationMessage_t* pAppMessage)
{
   osStatus status;
   
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_AT_COMMAND_E == pAppMessage->MessageType )
      {
         AtMessageData_t* pAtCommand = (AtMessageData_t*)pAppMessage->MessageBuffer;

         if ( pAtCommand )
         {
            if ( pAtCommand->IsCommand )
            {
               CO_PRINT_B_1(DBG_INFO_E, "App Rx'd OTA AT Command - %s\r\n", pAtCommand->Command);
               //Record the command so that the response can be directed back to the source
               MeshAtCommand.Destination = pAtCommand->Destination;
               MeshAtCommand.Source = pAtCommand->Source;
               MeshAtCommand.IsCommand = pAtCommand->IsCommand;
               memcpy(MeshAtCommand.Command, pAtCommand->Command, MAX_OTA_AT_COMMAND_LENGTH);
               MeshAtCommandInProgress = true;
               
               Cmd_Message_t *pCmdMessage = NULL;
               pCmdMessage = osPoolAlloc(ConfigSerialPool);
               if (pCmdMessage)
               {
                  memset(pCmdMessage->data_buffer, 0, (AT_COMMAND_MAX_LENGTH+AT_PARAMETER_MAX_LENGTH+1u) );
                  pCmdMessage->port_id = MESH_INTERFACE_E;
                  pCmdMessage->length = strlen(pAtCommand->Command);
                  memcpy(pCmdMessage->data_buffer, pAtCommand->Command, pCmdMessage->length);

                  status = osMessagePut(ConfigSerialQ, (uint32_t)pCmdMessage, 0);
                  if (osOK != status)
                  {
                     osPoolFree(ConfigSerialPool, pCmdMessage);
                  }
               }
            }
            else 
            {
               //This is a response message
               CO_PRINT_A_1(DBG_INFO_E,"%s\r\n", pAtCommand->Command);
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_RBUApplicationProcessGlobalDelaysMessage
* Function to receive global delays from the Mesh.
*
* @param pAppMessage   The contents of the message.
*
* @return - void
*/
void MM_RBUApplicationProcessGlobalDelaysMessage(const ApplicationMessage_t* pAppMessage)
{
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_GLOBAL_DELAYS_E == pAppMessage->MessageType )
      {
         CO_GlobalDelaysData_t* pDelayData = (CO_GlobalDelaysData_t*)pAppMessage->MessageBuffer;

         if ( pDelayData )
         {
            //Set the global delays
            uint32_t combined_global_delays = pDelayData->Delay1 + (pDelayData->Delay2 << 16); //global delay 2 is stored in the upper 16 bits, global delay 1 in the lower 16 bits.
            ErrorCode_t error_code = DM_OP_SetGlobalDelayValues(combined_global_delays);
            if ( SUCCESS_E == error_code )
            {
               CO_PRINT_B_2(DBG_INFO_E,"Set new global delays. Delay1=%d, Delay2=%d\r\n", pDelayData->Delay1, pDelayData->Delay2);
            }
            else 
            {
               CO_PRINT_B_1(DBG_ERROR_E,"FAILED to set new global delays. Error=%d\r\n", error_code);
            }
            
            //Store the local/global delay channel settings
            if ( pDelayData->ChannelLocalOrGlobalDelay & DM_OP_BIT_CHANNEL_SOUNDER )
            {
               DM_OP_SetLocalOrGlobalDelayFlag(CO_CHANNEL_SOUNDER_E, 1);
            }
            else 
            {
               DM_OP_SetLocalOrGlobalDelayFlag(CO_CHANNEL_SOUNDER_E, 0);
            }
            
            if ( pDelayData->ChannelLocalOrGlobalDelay & DM_OP_BIT_CHANNEL_BEACON )
            {
               DM_OP_SetLocalOrGlobalDelayFlag(CO_CHANNEL_BEACON_E, 1);
            }
            else 
            {
               DM_OP_SetLocalOrGlobalDelayFlag(CO_CHANNEL_BEACON_E, 0);
            }
            
            if ( pDelayData->ChannelLocalOrGlobalDelay & DM_OP_BIT_CHANNEL_STATUS_INDICATOR_LED )
            {
               DM_OP_SetLocalOrGlobalDelayFlag(CO_CHANNEL_STATUS_INDICATOR_LED_E, 1);
            }
            else 
            {
               DM_OP_SetLocalOrGlobalDelayFlag(CO_CHANNEL_STATUS_INDICATOR_LED_E, 0);
            }
            
            if ( pDelayData->ChannelLocalOrGlobalDelay & DM_OP_BIT_CHANNEL_VISUAL_INDICATOR )
            {
               DM_OP_SetLocalOrGlobalDelayFlag(CO_CHANNEL_VISUAL_INDICATOR_E, 1);
            }
            else 
            {
               DM_OP_SetLocalOrGlobalDelayFlag(CO_CHANNEL_VISUAL_INDICATOR_E, 0);
            }
            
            if ( pDelayData->ChannelLocalOrGlobalDelay & DM_OP_BIT_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED )
            {
               DM_OP_SetLocalOrGlobalDelayFlag(CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E, 1);
            }
            else 
            {
               DM_OP_SetLocalOrGlobalDelayFlag(CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E, 0);
            }
            
            if ( pDelayData->ChannelLocalOrGlobalDelay & DM_OP_BIT_CHANNEL_OUTPUT_ROUTING )
            {
               DM_OP_SetLocalOrGlobalDelayFlag(CO_CHANNEL_OUTPUT_ROUTING_E, 1);
            }
            else 
            {
               DM_OP_SetLocalOrGlobalDelayFlag(CO_CHANNEL_OUTPUT_ROUTING_E, 0);
            }
            
            if ( pDelayData->ChannelLocalOrGlobalDelay & DM_OP_BIT_CHANNEL_OUTPUT_1 )
            {
               DM_OP_SetLocalOrGlobalDelayFlag(CO_CHANNEL_OUTPUT_1_E, 1);
            }
            else 
            {
               DM_OP_SetLocalOrGlobalDelayFlag(CO_CHANNEL_OUTPUT_1_E, 0);
            }
            
            if ( pDelayData->ChannelLocalOrGlobalDelay & DM_OP_BIT_CHANNEL_OUTPUT_2 )
            {
               DM_OP_SetLocalOrGlobalDelayFlag(CO_CHANNEL_OUTPUT_2_E, 1);
            }
            else 
            {
               DM_OP_SetLocalOrGlobalDelayFlag(CO_CHANNEL_OUTPUT_2_E, 0);
            }
            
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_ReadInputChannels
* IOU base types only.  Read the analogue inputs and send alarm and fault messages.
*
* @param    None.
*
* @return - None.
*/
void MM_ReadInputChannels(void)
{
   uint32_t debounce_period = INPUT_ACTIVATING_DEBOUNCE_COUNT;
//   //Read the polling period in uSeconds
//   uint32_t polling_period = TE_GetAnaloguePollingPeriod();
//   //convert to milliseconds.  Add 1 to ensure we never have a zero value.
//   polling_period = (polling_period / 1000) + 1;
   uint32_t polling_period = IUO_PERIODIC_TIMER_PERIOD_MS;
   
   if ( BASE_IOU_E == gBaseType )
   {
      //Read input channels
      InputState_t input_channel_1_state;
      InputState_t input_channel_2_state;
      ErrorCode_t error = DM_RC_ReadInputs(&input_channel_1_state, &input_channel_2_state);
      if ( SUCCESS_E == error )
      {
         //Process CO_CHANNEL_INPUT_1
         if ( input_channel_1_state != gInputStates[0] )
         {
            gInputDebounceCount[0] += polling_period;
            //The normal state is INPUT_STATE_SET_E.  If we are returning to this, use the deactivating debounce value.
            if ( INPUT_STATE_SET_E == input_channel_1_state )
            {
               debounce_period = INPUT_DEACTIVATING_DEBOUNCE_COUNT;
            }
         }
         else 
         {
            gInputDebounceCount[0] = 0;
         }
         
         if ( debounce_period <= gInputDebounceCount[0] )
         {
            CO_PRINT_B_1(DBG_INFO_E,"Input Channel 1 = %d\r\n", input_channel_1_state);
            switch ( input_channel_1_state )
            {
               case INPUT_STATE_FAULT_SHORT_E:
                  //If we are we changing out of an OC fault state, clear the fault
                  if ( INPUT_STATE_FAULT_OPEN_E == gInputStates[0] )
                  {
                     CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_INPUT_1_E, FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E, 0, false, false);
                     CO_PRINT_B_0(DBG_INFO_E,"Sending OC Fault Removed Channel 1\r\n");
                  }
                  //We weren't previously in a fault state, send a fault Signal
                  CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_INPUT_1_E, FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E, 1, false, false);
                  CO_PRINT_B_0(DBG_INFO_E,"Sending SC Fault Channel 1\r\n");
                  break;
               case INPUT_STATE_FAULT_OPEN_E:
                  //If we are we changing out of a SC state, clear the fault
                  if ( INPUT_STATE_FAULT_SHORT_E == gInputStates[0] )
                  {
                     CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_INPUT_1_E, FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E, 0, false, false);
                     CO_PRINT_B_0(DBG_INFO_E,"Sending SC Fault Removed Channel 1\r\n");
                  }
                  //We weren't previously in a fault state, send a fault Signal
                  CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_INPUT_1_E, FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E, 1, false, false);
                  CO_PRINT_B_0(DBG_INFO_E,"Sending OC Fault Channel 1\r\n");
                  break;
               case INPUT_STATE_RESET_E:
                  //We have change to RESET state.  Send an alarm signal
                  MM_RBUSendAlarmSignal(CO_IO_UNIT_INPUT_1_E, CO_CHANNEL_INPUT_1_E, true, 0);

                  //If we are we changing out of a fault state, clear the fault
                  if ( INPUT_STATE_FAULT_SHORT_E == gInputStates[0] )
                  {
                     CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_INPUT_1_E, FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E, 0, false, false);
                     CO_PRINT_B_0(DBG_INFO_E,"Sending SC Fault Removed Channel 1\r\n");
                  }
                  else if ( INPUT_STATE_FAULT_OPEN_E == gInputStates[0] )
                  {
                     CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_INPUT_1_E, FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E, 0, false, false);
                     CO_PRINT_B_0(DBG_INFO_E,"Sending OC Fault Removed Channel 1\r\n");
                  }

                  break;
               case INPUT_STATE_SET_E:
                  //We have changed to SET state.  Send an alarm signal
                  MM_RBUSendAlarmSignal(CO_IO_UNIT_INPUT_1_E, CO_CHANNEL_INPUT_1_E, false, 1);


                  //If we are we changing out of a fault state, clear the fault
                  if ( INPUT_STATE_FAULT_SHORT_E == gInputStates[0] )
                  {
                     CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_INPUT_1_E, FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E, 0, false, false);
                     CO_PRINT_B_0(DBG_INFO_E,"Sending SC Fault Removed Channel 1\r\n");
                  }
                  else if ( INPUT_STATE_FAULT_OPEN_E == gInputStates[0] )
                  {
                     CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_INPUT_1_E, FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E, 0, false, false);
                     CO_PRINT_B_0(DBG_INFO_E,"Sending OC Fault Removed Channel 1\r\n");
                  }
                  break;
               default:
                  break;
            }
            //Store the latest state for next time
            gInputStates[0] = input_channel_1_state;
            gInputDebounceCount[0] = 0;
         }


         //Process CO_CHANNEL_INPUT_2
         if ( input_channel_2_state != gInputStates[1] )
         {
            gInputDebounceCount[1] += polling_period;
            //The normal state is INPUT_STATE_SET_E.  If we are returning to this, use the deactivating debounce value.
            if ( INPUT_STATE_SET_E == input_channel_2_state )
            {
               debounce_period = INPUT_DEACTIVATING_DEBOUNCE_COUNT;
            }
         }
         else 
         {
            gInputDebounceCount[1] = 0;
         }
         
         if ( debounce_period <= gInputDebounceCount[1] )
         {
            CO_PRINT_B_1(DBG_INFO_E,"Input Channel 2 = %d\r\n", input_channel_2_state);
            switch ( input_channel_2_state )
            {
               case INPUT_STATE_FAULT_SHORT_E: //intentional drop-through
                  //If we are we changing out of an OC fault state, clear the fault
                  if ( INPUT_STATE_FAULT_OPEN_E == gInputStates[1] )
                  {
                     CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_INPUT_2_E, FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E, 0, false, false);
                     CO_PRINT_B_0(DBG_INFO_E,"Sending OC Fault Removed Channel 2\r\n");
                  }
                  //We were previously in a fault state, send a fault Signal
                  CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_INPUT_2_E, FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E, 1, false, false);
                  CO_PRINT_B_0(DBG_INFO_E,"Sending SC Fault Channel 2\r\n");
                  break;
               case INPUT_STATE_FAULT_OPEN_E:
                  //We were previously in a fault state, send a fault Signal
                  CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_INPUT_2_E, FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E, 1, false, false);
                  CO_PRINT_B_0(DBG_INFO_E,"Sending OC Fault Channel 2\r\n");
                  //If we are we changing out of a SC fault state, clear the fault
                  if ( INPUT_STATE_FAULT_SHORT_E == gInputStates[1] )
                  {
                     CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_INPUT_2_E, FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E, 0, false, false);
                     CO_PRINT_B_0(DBG_INFO_E,"Sending SC Fault Removed Channel 2\r\n");
                  }
                  break;
               case INPUT_STATE_RESET_E:
                     //We have change to RESET state.  Send an alarm signal
                     MM_RBUSendAlarmSignal(CO_IO_UNIT_INPUT_2_E, CO_CHANNEL_INPUT_2_E, true, 0);

                  //If we are we changing out of a fault state, clear the fault
                  if ( INPUT_STATE_FAULT_SHORT_E == gInputStates[1] )
                  {
                     CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_INPUT_2_E, FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E, 0, false, false);
                     CO_PRINT_B_0(DBG_INFO_E,"Sending SC Fault Removed Channel 2\r\n");
                  }
                  else if ( INPUT_STATE_FAULT_OPEN_E == gInputStates[1] )
                  {
                     CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_INPUT_2_E, FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E, 0, false, false);
                     CO_PRINT_B_0(DBG_INFO_E,"Sending OC Fault Removed Channel 2\r\n");
                  }

                  break;
               case INPUT_STATE_SET_E:
                  //We have changed to SET state.  Send an alarm signal
                  MM_RBUSendAlarmSignal(CO_IO_UNIT_INPUT_2_E, CO_CHANNEL_INPUT_2_E, false, 1);


                  //If we are we changing out of a fault state, clear the fault
                  if ( INPUT_STATE_FAULT_SHORT_E == gInputStates[1] )
                  {
                     CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_INPUT_2_E, FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E, 0, false, false);
                     CO_PRINT_B_0(DBG_INFO_E,"Sending SC Fault Removed Channel 2\r\n");
                  }
                  else if ( INPUT_STATE_FAULT_OPEN_E == gInputStates[1] )
                  {
                     CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_INPUT_2_E, FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E, 0, false, false);
                     CO_PRINT_B_0(DBG_INFO_E,"Sending OC Fault Removed Channel 2\r\n");
                  }
                  break;
               default:
                  break;
            }
            //Store the latest state for next time
            gInputStates[1] = input_channel_2_state;
            gInputDebounceCount[1] = 0;
         }

      }
      else 
      {
         CO_PRINT_A_1(DBG_ERROR_E,"FAILED to read input channels.  Error=%d\r\n", error);
      }
      
   }
}





/*************************************************************************************/
/**
* MM_RBUSendAlarmSignal
* Send an alarm message to the NCU
*
* @param - sensorType   - The type of sensor raising the alarm
* @param - channel      - The channel number of the sensor
* @param - alarmActive  - Set to '1' if the sensor is in an alarm state, '0' otherwise
* @param - sensorValue  - The raw reading from the sensor detector
*
* @return - ErrorCode_t - status 0=success
*/
ErrorCode_t MM_RBUSendAlarmSignal(const CO_RBUSensorType_t sensorType, const CO_ChannelIndex_t channel, const bool alarmActive, const uint16_t sensorValue)
{
   ErrorCode_t status = SUCCESS_E;
   osStatus osStat = osErrorOS;
   CO_Message_t *pAlarmSignalReq = NULL;
   CO_RBUSensorData_t sensorData;
      
   /* create AlarmSignalReq message and put into the App queue */
   pAlarmSignalReq = osPoolAlloc(AppPool);
   if (pAlarmSignalReq)
   {
      sensorData.SensorType = sensorType;
      sensorData.SensorValue = sensorValue;
      sensorData.AlarmState = alarmActive ? 1:0;
      sensorData.RUChannelIndex = channel;
      
      pAlarmSignalReq->Type = CO_MESSAGE_GENERATE_ALARM_SIGNAL_E;
      memcpy(pAlarmSignalReq->Payload.PhyDataReq.Data, &sensorData, sizeof(CO_RBUSensorData_t));
      osStat = osMessagePut(AppQ, (uint32_t)pAlarmSignalReq, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         osPoolFree(AppPool, pAlarmSignalReq);
         
         /* Return error code */
         status = ERR_MESSAGE_FAIL_E;
      }
   }
   
   return status;
}

/*************************************************************************************/
/**
* MM_ApplicationSendFaultReports
* Function to schedule all faults to be reported to the control panel.
* Typically called when the RBU moves to the 'active' mesh state to send faults to the
* control panel.
*
* @param - sendOnDULCH  If true send on delayed uplink channel, if false send immediately
*
* @return - None.
*/
void MM_ApplicationSendFaultReports(const bool sendOnDULCH)
{
   if ( gFaultStatusFlags )
   {
      if ( CO_INSTALLATION_TAMPER_FAULT_E & gFaultStatusFlags )
      {
         // send installation tamper fault
         CO_SendFaultSignal(BIT_SOURCE_NONE_E,  CO_CHANNEL_NONE_E, FAULT_SIGNAL_INSTALLATION_TAMPER_E, 1, sendOnDULCH, true );
      }
      
      if ( CO_DISMANTLE_TAMPER_FAULT_E & gFaultStatusFlags )
      {
         // send dismantle tamper fault
         CO_SendFaultSignal(BIT_SOURCE_NONE_E,  CO_CHANNEL_NONE_E, FAULT_SIGNAL_DISMANTLE_TAMPER_E, 1, sendOnDULCH, true );
      }
      
      if ( CO_DISMANTLE_HEAD_FAULT_E & gFaultStatusFlags )
      {
         // send dismantle head fault
         CO_SendFaultSignal(BIT_SOURCE_NONE_E,  CO_CHANNEL_NONE_E, FAULT_SIGNAL_HEAD_REMOVED_E, 1, sendOnDULCH, true );
      }
      
      if ( CO_MISMATCH_HEAD_FAULT_E & gFaultStatusFlags )  
      {
         // send mismatched head fault
         CO_SendFaultSignal(BIT_SOURCE_NONE_E,  CO_CHANNEL_NONE_E, FAULT_SIGNAL_TYPE_MISMATCH_E, 1, sendOnDULCH, true );
      }
		
		if ( CO_DEVICE_ID_MISMATCH_E & gFaultStatusFlags )
		{
			// send mismatched device fault
         CO_SendFaultSignal(BIT_SOURCE_NONE_E,  CO_CHANNEL_NONE_E, FAULT_SIGNAL_DEVICE_ID_MISMATCH_E, 1, sendOnDULCH, true );
		}
      
      if ( CO_BATTERY_ERROR_E & gFaultStatusFlags )
      {       
         CO_SendFaultSignal(BIT_SOURCE_NONE_E,  CO_CHANNEL_NONE_E, FAULT_SIGNAL_BATTERY_ERROR_E, 1, sendOnDULCH, true );
      }

      if ( CO_BATTERY_WARNING_E & gFaultStatusFlags )
      {
         CO_SendFaultSignal(BIT_SOURCE_NONE_E,  CO_CHANNEL_NONE_E, FAULT_SIGNAL_LOW_BATTERY_E, 1, sendOnDULCH, true );
      }

      if ( CO_DETECTOR_FAULT_E & gFaultStatusFlags )
      {
         // send detector fault - defer to plugin module
         HeadMessage_t headMsg;
         headMsg.ChannelNumber = CO_CHANNEL_MAX_E;//check all channels
         MM_CMD_SendHeadMessage(CO_MESSAGE_GENERATE_FAULT_SIGNAL_E, &headMsg, CMD_SOURCE_APPLICATION_E);
      }
      
      if ( CO_BEACON_FAULT_E & gFaultStatusFlags )
      {
         // send beacon fault
         CO_SendFaultSignal(BIT_SOURCE_NONE_E,  CO_CHANNEL_BEACON_E, FAULT_SIGNAL_BEACON_LED_FAULT_E, 1, sendOnDULCH, true );
      }
      
      if ( CO_SOUNDER_FAULT_E & gFaultStatusFlags )
      {
         // send sounder fault
         CO_SendFaultSignal(BIT_SOURCE_NONE_E,  CO_CHANNEL_SOUNDER_E, FAULT_SIGNAL_SOUNDER_FAULT_E, 1, sendOnDULCH, true );
      }
      
      if ( (CO_IO_INPUT_1_FAULT_E & gFaultStatusFlags) || (CO_IO_INPUT_2_FAULT_E & gFaultStatusFlags) )
      {
         // send input 1 fault
         InputState_t channel_1_state;
         InputState_t channel_2_state;
         ErrorCode_t result = DM_RC_ReadInputs(&channel_1_state, &channel_2_state);
         
         if ( SUCCESS_E == result )
         {
            if ( CO_IO_INPUT_1_FAULT_E & gFaultStatusFlags )
            {
               if ( INPUT_STATE_FAULT_SHORT_E == channel_1_state )
               {
                  CO_SendFaultSignal(BIT_SOURCE_NONE_E,  CO_CHANNEL_INPUT_1_E, FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E, 1, sendOnDULCH, true );
               }
               else if ( INPUT_STATE_FAULT_OPEN_E == channel_1_state )
               {
                  CO_SendFaultSignal(BIT_SOURCE_NONE_E,  CO_CHANNEL_INPUT_1_E, FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E, 1, sendOnDULCH, true );
               }
               else if ( INPUT_STATE_RESET_E == channel_1_state )
               {
                  MM_SendAlarmSignal( CO_CHANNEL_INPUT_1_E, CO_IO_UNIT_INPUT_1_E, 0, true );
               }
            }
            
            if ( CO_IO_INPUT_2_FAULT_E & gFaultStatusFlags )
            {
               if ( INPUT_STATE_FAULT_SHORT_E == channel_2_state )
               {
                  CO_SendFaultSignal(BIT_SOURCE_NONE_E,  CO_CHANNEL_INPUT_2_E, FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E, 1, sendOnDULCH, true );
               }
               else if ( INPUT_STATE_FAULT_OPEN_E == channel_2_state )
               {
                  CO_SendFaultSignal(BIT_SOURCE_NONE_E,  CO_CHANNEL_INPUT_2_E, FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E, 1, sendOnDULCH, true );
               }
               else if ( INPUT_STATE_RESET_E == channel_2_state )
               {
                  MM_SendAlarmSignal( CO_CHANNEL_INPUT_2_E, CO_IO_UNIT_INPUT_2_E, 0, true );
               }
            }
         }
      }
   }
   
}

/*************************************************************************************/
/**
* MM_GetFaultStatusFlags
* Return the current fault status flags
*
* @param - None.
*
* @return - uint16_t    The fault status.
*/
uint32_t MM_GetFaultStatusFlags(void)
{
   return gFaultStatusFlags;
}

/*************************************************************************************/
/**
* MM_ApplicationSendAlarmSignals
* Function to check all inputs and send alarms for any that are active
*
* @param - None.
*
* @return - None.
*/
void MM_ApplicationSendAlarmSignals(void)
{
   ErrorCode_t result;
   uint32_t channelIndex;
   
   //Iterate the channels and identify the ones that this device has
   for ( channelIndex = 0; channelIndex < CO_CHANNEL_MAX_E; channelIndex++)
   {
      // check that it's an input channel
      if ( CO_IsInputChannel(channelIndex) )
      {
         //Get the device type for the channel
         uint32_t deviceType;
         result = DM_DeviceGetDeviceCode(gDeviceCombination, channelIndex, &deviceType);
         if ( SUCCESS_E == result )
         {
            //Check if this RBU has got the device enabled
            if ( DM_DeviceIsEnabled(gDeviceCombination, deviceType, DC_MATCH_ANY_E) )
            {
               //This RBU does have the channel.  Check its alarm state.
               switch ( channelIndex )
               {
                  case CO_CHANNEL_SMOKE_E:
                  {
                     HeadActivationState_t activeState = MM_GetActivationState(CO_CHANNEL_SMOKE_E);
                     if ( HEAD_ACTIVATION_ALARM_E == activeState )
                     {
                        uint8_t value = MM_GetSmokeSensorValue();
                        MM_SendFireSignal( CO_CHANNEL_SMOKE_E, CO_SMOKE_E, value, HEAD_ACTIVATION_ALARM_E );
                     }
                  }
                     break;
                  case CO_CHANNEL_HEAT_A1R_E: //intentional drop-through
                  {
                     HeadActivationState_t activeState = MM_GetActivationState(CO_CHANNEL_HEAT_A1R_E);
                     if ( HEAD_ACTIVATION_ALARM_E == activeState )
                     {
                        uint8_t value = MM_GetHeatSensorValue();
                        MM_SendFireSignal( CO_CHANNEL_HEAT_A1R_E, CO_HEAT_A1R_E, value, HEAD_ACTIVATION_ALARM_E );
                     }
                  }
                     break;
                  case CO_CHANNEL_HEAT_B_E:
                  {
                     HeadActivationState_t activeState = MM_GetActivationState(CO_CHANNEL_HEAT_B_E);
                     if ( HEAD_ACTIVATION_ALARM_E == activeState )
                     {
                        uint8_t value = MM_GetHeatSensorValue();
                        MM_SendFireSignal( CO_CHANNEL_HEAT_B_E, CO_HEAT_B_E, value, HEAD_ACTIVATION_ALARM_E );
                     }
                  }
                     break;
                  case CO_CHANNEL_PIR_E:
                  {
                     uint8_t pirState = 0;
                     
                     if ( BASE_CONSTRUCTION_E == gBaseType )
                     {
                        if ( gPirLatched || gPirAnalogueValue )
                        {
                           pirState = 1;
                        }
                        else 
                        {
                           //Construction PIR
                           DM_InputMonitorState_t DM_PirState = GetConstructionPIRState();
                           if ( IN_MON_ACTIVE_E == DM_PirState )
                           {
                              pirState = 1;
                           }
                        }
                     }
                     else 
                     {
                        //Plug-in PIR
                        pirState = MM_GetPIRState();
                     }
                     
                     if ( pirState )
                     {
                        MM_ApplicationHandlePirEvent(PIR_EVENT_PIR_ACTIVE_E);
                     }
                     else 
                     {
                        MM_ApplicationHandlePirEvent(PIR_EVENT_PIR_IDLE_E);
                     }
                  }
                     break;
                  case CO_CHANNEL_FIRE_CALLPOINT_E:
                     if ( IN_MON_ACTIVE_E == DM_GetFireCallPointState() )
                     {
                        MM_SendFireSignal( CO_CHANNEL_FIRE_CALLPOINT_E, CO_FIRE_CALLPOINT_E, 1, 1 );
                     }
                     break;
                  case CO_CHANNEL_MEDICAL_CALLPOINT_E:
                     if ( IN_MON_ACTIVE_E == DM_GetFirstAidCallPointState() )
                     {
                        MM_SendAlarmSignal( CO_CHANNEL_MEDICAL_CALLPOINT_E, CO_FIRST_AID_CALLPOINT_E, 1, 1 );
                     }
                     break;
                  case CO_CHANNEL_EVAC_CALLPOINT_E:
                     break;
                  case CO_CHANNEL_INPUT_1_E: // intentional drop-through
                  case CO_CHANNEL_INPUT_2_E:
                  {
                     InputState_t state_channel_1;
                     InputState_t state_channel_2;
                     result = DM_RC_ReadInputs(&state_channel_1, &state_channel_2);
                     if ( SUCCESS_E == result )
                     {
                        if ( INPUT_STATE_RESET_E == state_channel_1 )
                        {
                           MM_SendAlarmSignal( CO_CHANNEL_INPUT_1_E, CO_IO_UNIT_INPUT_1_E, 0, true);
                        }
                        if ( INPUT_STATE_RESET_E == state_channel_2 )
                        {
                           MM_SendAlarmSignal( CO_CHANNEL_INPUT_2_E, CO_IO_UNIT_INPUT_2_E, 0, true );
                        }
                     }
                  }
                     break;
                  default:
                     break;
               }
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_ChannelEnabled
* Function to check if a channel is enabled
*
* @param - channelIndex    The channel to check.
*
* @return - bool           True if the channel is enabled.
*/
bool MM_ChannelEnabled(const uint16_t channelIndex)
{
   bool enabled = true; // assume the channel is enabled
   DM_Enable_t enable_disable;
   CO_DayNight_t day_night = CFG_GetDayNight();
   ErrorCode_t result = DM_OP_GetDisablement(channelIndex, &enable_disable);
   
   if ( SUCCESS_E == result )
   {
      switch ( day_night )
      {
         case CO_DAY_E:
            if ( (DM_ENABLE_DAY_E == enable_disable) || (DM_ENABLE_DAY_AND_NIGHT_E == enable_disable) )
            {
               enabled = true;
            }
            else 
            {
               enabled = false;
            }
            break;
         case CO_NIGHT_E:
            if ( (DM_ENABLE_NIGHT_E == enable_disable) || (DM_ENABLE_DAY_AND_NIGHT_E == enable_disable) )
            {
               enabled = true;
            }
            else 
            {
               enabled = false;
            }
            break;
         default:
            enabled = true;
         break;
      }
   }
   
   return enabled;
}


/*************************************************************************************/
/**
* MM_RBU_HasWarnings
* Function to check if the fault status flags contain any active warnings
*
* @param - None.
*
* @return - bool     TRUE is warnings are present.
*
* Note:  Uses global variable.
*        gFaultStatusFlags is used to identify warnings

*/
bool MM_RBU_HasWarnings(void)
{
   bool result = false;
   //all flags are faults except for for battery fault.
   uint16_t warning_fields = CO_BATTERY_WARNING_E;
   
   if ( 0 != (gFaultStatusFlags & warning_fields) )
   {
      result = true;
   }
   CO_PRINT_B_2(DBG_INFO_E,"HasWarnings=%d, flags=%x\r\n", result, gFaultStatusFlags);
   return result;
}


/*************************************************************************************/
/**
* MM_RBU_HasFaults
* Function to check if the fault status flags contain any active faults
*
* @param - None.
*
* @return - bool     TRUE if faults are present.
*
* Note:  Uses global variable.
*        gFaultStatusFlags is used to identify faults

*/
bool MM_RBU_HasFaults(void)
{
   bool result = false;
   //all flags are faults except for battery warning.  Mask out the unused bits.
   uint32_t fault_fields = ~CO_BATTERY_ERROR_E & CO_FAULT_MASK_E;

   if ( 0 != (gFaultStatusFlags & fault_fields) )
   {
      result = true;
   }
   
   CO_PRINT_B_2(DBG_INFO_E,"HasFaults=%d, flags=%x\r\n", result, gFaultStatusFlags);

   return result;
}

/*************************************************************************************/
/**
* MM_RBU_GetOverallFaults
* Function to return the overall faults value for status messages
*
* @param - None.
*
* @return - uint8_t     the overall faults value.
*
*/
uint8_t MM_RBU_GetOverallFaults(void)
{
   uint8_t overall_faults = 0;

   if ( MM_RBU_HasFaults() )
   {
      overall_faults |= 0x01;
   }
   
   if ( MM_RBU_HasWarnings() ) 
   {
      overall_faults |= 0x02;
   }
   
   return overall_faults;
}
/*************************************************************************************/
/**
* MM_SendSensorValues
* Function to send analogue values from the sensors to the CIE in the Battery Status message
*
* @param - None.
*
* @return - bool     TRUE if the message fails.
*
*/
bool MM_SendSensorValues(void)
{
   bool send_fail_status = true;
   
   CO_BatteryStatusData_t batteryStatus;
   batteryStatus.Handle = GetNextHandle();
   batteryStatus.PrimaryBatteryVoltage = DM_BAT_GetPrimaryBatteryVoltage();
   batteryStatus.BackupBatteryVoltage = DM_BAT_GetBackupBatteryVoltage();

   CO_PRINT_B_2(DBG_INFO_E,"MM_SendSensorValues: PrimaryAdc=%d, backupAdc=%d\r\n", batteryStatus.PrimaryBatteryVoltage, batteryStatus.BackupBatteryVoltage);

	 if ( batteryStatus.PrimaryBatteryVoltage <= 1 && batteryStatus.BackupBatteryVoltage <= 1 )
	 {
		 // dont send rubbish battery values
			return false;
	 }
		 
   //POLICY:  Always send a non-zero battery reading.
   //The control panel sees 0V as 'test not done' and doesn't update the display
	 
	 // ??? MWA If test ISNT done  then we dont want display updated!  removing 0 check...
	 
/*	
   if ( 0 == batteryStatus.PrimaryBatteryVoltage )
   {
      batteryStatus.PrimaryBatteryVoltage = 1;
   }
   if ( 0 == batteryStatus.BackupBatteryVoltage )
   {
      batteryStatus.BackupBatteryVoltage = 1;
   }
*/
   batteryStatus.DeviceCombination = gDeviceCombination;
   batteryStatus.ZoneNumber = gZoneNumber;
   batteryStatus.SmokeAnalogueValue = gSmokeAnalogueValue;
   batteryStatus.HeatAnalogueValue = gHeatAnalogueValue;
   batteryStatus.PirAnalogueValue = gPirAnalogueValue;
   batteryStatus.DelaySending = true;
   if ( MM_MeshAPIGenerateBatteryStatusSignalReq( &batteryStatus ) )
   {
      send_fail_status = false;
   }

   return send_fail_status;
}

/*************************************************************************************/
/**
* MM_RBU_RequestSensorValues
* Function to request analogue values from the sensors.  Requested by the CIE.
*
* @param - None.
*
* @return - bool     TRUE if the message fails.
*
*/
ErrorCode_t MM_RBU_RequestSensorValues(void)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;
   
   HeadMessage_t head_message;
   head_message.MessageType = HEAD_READ_ANALOGUE_VALUE_E;
   
   // Do we have smoke?
   if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SMOKE_DETECTOR, DC_MATCH_ANY_E) )
   {
      head_message.ChannelNumber = DEV_SMOKE_DETECTOR_HEAD_E;
      //Tell the plugin manager to report the smoke sensor value
      MM_CMD_SendHeadMessage(CO_MESSAGE_APPLICATION_REQUEST_E, &head_message, CMD_SOURCE_APPLICATION_E);
      gWaitingForSmokeAnalogueValue = true;
   }
   else 
   {
      gSmokeAnalogueValue = 0;
      gWaitingForSmokeAnalogueValue = false;
   }
   
   //Do we have heat?
   if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAT_DETECTOR_TYPE_A1R | DEV_HEAT_DETECTOR_TYPE_B, DC_MATCH_ANY_E) )
   {
      head_message.ChannelNumber = DEV_HEAT_DETECTOR_HEAD_E;
      //Tell the plugin manager to report the smoke sensor value
      MM_CMD_SendHeadMessage(CO_MESSAGE_APPLICATION_REQUEST_E, &head_message, CMD_SOURCE_APPLICATION_E);
      gWaitingForHeatAnalogueValue = true;
   }
   else 
   {
      gHeatAnalogueValue = 0;
      gWaitingForHeatAnalogueValue = false;
   }
   
   //Do we have PIR?
   if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_PIR_DETECTOR, DC_MATCH_ANY_E) )
   {
      if ( BASE_RBU_E == gBaseType )
      {
         head_message.ChannelNumber = DEV_PIR_DETECTOR_HEAD_E;
         //Tell the plugin manager to report the smoke sensor value
         MM_CMD_SendHeadMessage(CO_MESSAGE_APPLICATION_REQUEST_E, &head_message, CMD_SOURCE_APPLICATION_E);
         gWaitingForPirAnalogueValue = true;
      }
      else if ( BASE_CONSTRUCTION_E == gBaseType )
      {
         gPirAnalogueValue = (0 == gPirLatched ? 0:1);
         gWaitingForPirAnalogueValue = false;
      }
   }
   else 
   {
      gPirAnalogueValue = 0;
      gWaitingForPirAnalogueValue = false;
   }
   
   gAnalogueValuesRequested = true;
   
   return result;
}


/*************************************************************************************/
/**
* MM_ApplicationResetPir
* Function to reset the PIR latch.
*
* @param - sendCancelMessage  True if the PIR cancel-alarm message is to be sent.
*
* @return - None.
*
*/
void MM_ApplicationResetPir(const bool sendCancelMessage)
{
   //if the PIR was latched, send a message to the panel cancelling the PIR
   if ( gPirLatched )
   {
      //Unlatch software latch.
      gPirLatched = 0;
      gPirAnalogueValue = 0;
      CO_PRINT_B_0(DBG_INFO_E, "RBU APP PIR latch reset\r\n");
   }
   
   if ( sendCancelMessage )
   {
      CO_RBUSensorData_t sensor_data;
      sensor_data.AlarmState = 0;
      sensor_data.RUChannelIndex = CO_CHANNEL_PIR_E;
      sensor_data.SensorType = CO_PIR_E;
      sensor_data.SensorValue = gPirAnalogueValue;
      MM_RBUApplicationAlarmSignalReq(&sensor_data);
   }

   
   if ( BASE_RBU_E == gBaseType )
   {
      //RBU.  Send reset to head
      HeadMessage_t headMessage;
      headMessage.MessageType = HEAD_RESET_PIR_E;
      headMessage.Value = 0; 
      headMessage.TransactionID = 0;
      headMessage.ChannelNumber = CO_CHANNEL_PIR_E;
      headMessage.ProfileIndex = 0;
      MM_CMD_SendHeadMessage( CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E, &headMessage, CMD_SOURCE_APPLICATION_E );
   }
}

/*************************************************************************************/
/**
* MM_ApplicationResetPir
* Function to reset the PIR latch.
*
* @param - None.
*
* @return - None.
*
*/
void MM_ApplicationHandlePirEvent(const PirEvent_t pirEvent)
{
   switch ( pirEvent )
   {
      case PIR_EVENT_TIMER_E:
         if ( gPirLatched )
         {
            gPirLatched--;
            if ( 0 == gPirLatched )
            {
               gPirLatched = 1;//This will cause function MM_ApplicationResetPir to send a PIR cancel signal to the panel
               MM_ApplicationResetPir(true);
            }
         }
         break;
      case PIR_EVENT_PIR_ACTIVE_E:
            //Don't action PIR changes if we are not active on the mesh
         if ( STATE_ACTIVE_E == gCurrentDeviceState )
         {
            //Ignore if PIR is already latched
            if ( 0 == gPirLatched )
            {
               gPirAnalogueValue = 1;
               MM_RBUSendAlarmSignal(CO_PIR_E, CO_CHANNEL_PIR_E, true, 0);
               CO_PRINT_B_0(DBG_INFO_E, "RBU APP PIR latch set\r\n");
               CO_RBUSensorData_t sensor_data;
               sensor_data.AlarmState = 1;
               sensor_data.RUChannelIndex = CO_CHANNEL_PIR_E;
               sensor_data.SensorType = CO_PIR_E;
               sensor_data.SensorValue = gPirAnalogueValue;
               MM_RBUApplicationAlarmSignalReq(&sensor_data);
               gPirLatched = PIR_LATCH_PERIOD;
            }
            //Reset the timeout so that the PIR resets after PIR_LATCH_PERIOD seconds of inactivity
            gPirLatched = PIR_LATCH_PERIOD;
         }
         else 
         {
            //Not active on the mesh.  Reset the PIR.
            MM_ApplicationResetPir(false);
            CO_PRINT_B_0(DBG_INFO_E, "RBU APP PIR signal ignored. Not active on mesh.\r\n");
         }
         break;
      case PIR_EVENT_PIR_IDLE_E:
         //Don't action PIR changes if we are not active on the mesh
         if ( STATE_ACTIVE_E == gCurrentDeviceState )
         {
            //Reset the timeout so that the PIR resets after PIR_LATCH_PERIOD seconds of inactivity
            if ( gPirLatched )
            {
               gPirLatched = PIR_LATCH_PERIOD;
            }
         }
         else 
         {
            //Not active on the mesh.
            CO_PRINT_B_0(DBG_INFO_E, "RBU APP PIR signal ignored. Not active on mesh.\r\n");
         }
         
         break;
      case PIR_EVENT_RESET_E:
         MM_ApplicationResetPir(false);
         break;
      default:
         //paramterer out of range.  Ignore.
      break;
   }
}

/*************************************************************************************/
/**
* MM_ApplicationProcessExitTestMessage
* Function to process the command for exiting the test mode and turn OFF the RED LED due to fault condition.
*
* @param pAppMessage   The contents of the message.
*
* @return - bool        SUCCESS_E if the LED is turned OFF.  Error code on failure.
*/
ErrorCode_t MM_ApplicationProcessExitTestMessage(const ApplicationMessage_t* pAppMessage)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   CO_PRINT_B_0(DBG_INFO_E,"MM_ApplicationProcessExitTestMessage\r\n");
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_EXIT_TEST_MODE_E == pAppMessage->MessageType )
      {
            //the message confirmation comes back
            if ( STATE_ACTIVE_E == MC_STATE_GetDeviceState() )
            {
               CO_PRINT_B_0(DBG_INFO_E,"Exit Test Mode requested\r\n");
               /* Remove the LED from fire condition */
							 DM_LedPatternRemove(LED_FIRE_INDICATION_E);
               result = SUCCESS_E;
            }
            else 
            {
               /*  No task */;
            }
      }
      else 
      {
         CO_PRINT_B_1(DBG_INFO_E,"Exit Test Mode change Rx'd bad msg type = %d\r\n", pAppMessage->MessageType);
         result = ERR_INVALID_PARAMETER_E;
      }
   }
   else 
   {
      CO_PRINT_B_0(DBG_INFO_E,"Exit Test Mode change Rx'd NULL msg\r\n");
   }
   
   return result;
}
