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
*  File         : MM_NCUApplicationStub.c
*
*  Description  : NCU Application Test Stub
*
*************************************************************************************/
#ifdef USE_NCU_STUB

/* System Include Files
*************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "MM_NCUApplicationStub.h"
#include "MM_MeshAPI.h"
#include "DM_SerialPort.h"
#include "CO_Defines.h"
#include "MM_ATHandleTask.h"
#include "MM_CommandProcessor.h"
#include "DM_InputMonitor.h"
#include "MM_ApplicationCommon.h"

/* Private Functions Prototypes
*************************************************************************************/
static void MM_NCUApplicationStubTaskMain (void const *argument);
static void MM_NCUApplicationStubInit(void);
static void MM_NCUApplicationStubTaskProcessMessage(const uint32_t timeout);
static void MM_NCUApplicationStubUpdateStatusFlags( CO_FaultData_t* FaultStatus );
static void MM_NCUApplicationStubProcessNetworkMessage(const ApplicationMessage_t* pMessage);
static void MM_NCUApplicationStubProcessOutputMessage(const uint16_t destination, const uint16_t Zone, const uint8_t OutputChannel, const uint8_t OutputProfile, const uint16_t OutputsActivated, const uint8_t OutputDuration);
static void MM_NCUProcessMACEvent(AppMACEventData_t* pEventData);
static void MM_NCUApplicationStubProcessFaultStatusIndication(const ApplicationMessage_t* pMessage);
static void MM_NCUApplicationStubProcessLogOnIndication(const ApplicationMessage_t* pMessage);
static void MM_NCUApplicationStubProcessFireSignalIndication(const ApplicationMessage_t* pMessage);
static void MM_NCUApplicationStubProcessFirstAidSignalIndication(const ApplicationMessage_t* pMessage);
static void MM_NCUApplicationStubProcessStatusIndication(const ApplicationMessage_t* pAppMessage);
static void MM_NCUApplicationStubProcessMeshStatusEvent(const ApplicationMessage_t* pAppMessage);
static void MM_NCUApplicationStubProcessCommandMessage(const ApplicationMessage_t* pAppMessage);
static void MM_NCUApplicationStubProcessResponseMessage(const ApplicationMessage_t* pAppMessage);
static void MM_NCUApplicationStubProcessSetStateMessage(const ApplicationMessage_t* pAppMessage);
static void MM_NCUApplicationStubProcessOutputStateMessage(const ApplicationMessage_t* pAppMessage);
static void MM_NCUApplicationStubProcessConfirmation(const ApplicationMessage_t* pAppMessage);
static void MM_NCUApplicationStubOutputStateRequestConfirmation(const uint32_t Handle, const uint32_t NewHandle, const uint32_t Status);

/* Global Variables
*************************************************************************************/
osThreadId tid_NCUStubTask;
osThreadDef (MM_NCUApplicationStubTaskMain, osPriorityNormal, 1, 800);
extern osPoolId AppPool;
extern osMessageQId(AppQ);
extern osPoolId ATHandlePool;
extern osMessageQId(ATHandleQ);
extern uint16_t gLastCommandedChannel;
extern uint8_t gLastCommandedOutputProfile;
extern uint8_t gLastCommandedOutputActive;
extern uint16_t gLastCommandedOutputDuration;
extern ApplicationChildArray_t childList;
extern uint16_t gNodeAddress;
extern uint32_t gZoneNumber;
extern const char MESH_EVENT_STR[][64];

/* Private Variables
*************************************************************************************/
static CO_OutputData_t gLastFireSignalOutputData;
static CO_OutputData_t gLastFirstAidOutputData;

/*************************************************************************************/
/**
* MM_NCUApplicationStubTaskInit
* Startup  function for the NCU Application task
*
* @param - void
*
* @return - int32_t   0 if the NCU Application thread was started or -1 on failure.
*/
int32_t MM_NCUApplicationStubTaskInit(const uint16_t address, const uint16_t device_configuration, const uint32_t unitSerialNo, const uint32_t defaultZoneNumber)
{
   gNodeAddress = address;
   
   MM_CMD_Initialise(address, device_configuration, true, unitSerialNo);
  
   MM_NCUApplicationStubInit();
   
   /* create thread */
   tid_NCUStubTask = osThreadCreate (osThread(MM_NCUApplicationStubTaskMain), NULL);
   if (!tid_NCUStubTask)
   {
      return(-1);
   }
   
   
   return 0;
}


/*************************************************************************************/
/**
* MM_NCUApplicationStubInit
* Initialisation function for the NCU Application Stub
*
* @param - void
*
* @return - void
*/
void MM_NCUApplicationStubInit(void)
{
   /* initialise the child records */
   MM_ApplicationInitialiseChildArray();
}

/*************************************************************************************/
/**
* MM_NCUApplicationStubTaskMain
* Main function for RBU Application task.
*
* @param - argument  Pointer to passed in data.
*
* @return - void
*/
static void MM_NCUApplicationStubTaskMain (void const *argument)
{
   CO_PRINT_B_0(DBG_INFO_E, "NCU Application running\r\n");
    
   while(true)
   {
      MM_NCUApplicationStubTaskProcessMessage( osWaitForever );
   };
}

/*************************************************************************************/
/**
* MM_NCUApplicationStubTaskProcessMessage
* Function to read a message from the AppQ and call the appropriate handling function.
*
* @param - timeout   Limits how long the function should wait for a message to arrive (milliseconds).
*
* @return - void
*/
void MM_NCUApplicationStubTaskProcessMessage( const uint32_t timeout )
{
   osEvent event;
   CO_Message_t *pMsg = NULL;
   CO_CommandData_t cmdMessage;
   uint8_t* pCmd = (uint8_t*)&cmdMessage;
  
   event = osMessageGet(AppQ, timeout);
   if (osEventMessage == event.status)
   {
      pMsg = (CO_Message_t *)event.value.p;
      if (pMsg)
      {
         switch (pMsg->Type)
         {
            case CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E:
            {
               uint8_t* pReq = (uint8_t*)pMsg->Payload.PhyDataReq.Data;
               
               if( pReq )
               {                                    
                  memcpy(pCmd, pReq, sizeof(CO_CommandData_t));

                  CommandSource_t cmd_source = MM_CMD_ConvertFromATCommandSource(cmdMessage.Source);

                  ErrorCode_t result = MM_CMD_ProcessCommand ( &cmdMessage, cmd_source );

                  if ( SUCCESS_E != result )
                  {
                     CO_PRINT_B_1(DBG_ERROR_E, "FAILED to process command.  Error=%d\r\n", result);
                  }
               }
            }
            break;
            case CO_MESSAGE_MESH_STATUS_EVENT_E:
            {
               ApplicationMessage_t* pAppMessage = (ApplicationMessage_t*)pMsg->Payload.PhyDataInd.Data;
               if ( pAppMessage )
               {
                  if ( APP_MSG_TYPE_STATUS_SIGNAL_E == pAppMessage->MessageType )
                  {
                     /* This is the local node status update (added child etc) */
                     MM_NCUApplicationStubProcessMeshStatusEvent(pAppMessage);                  
                  }
                  else if ( APP_MSG_TYPE_STATUS_INDICATION_E == pAppMessage->MessageType )
                  {
                     /* This is the status received from RBUs over the mesh*/
                     MM_NCUApplicationStubProcessStatusIndication(pAppMessage);                  
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
                  
                  CO_State_t state_value = (CO_State_t)commandData.Value;

                  CommandSource_t cmd_source = MM_CMD_ConvertFromATCommandSource(commandData.Source);

                  if( STATE_MAX_E > state_value )
                  {
                     if( false ==  MM_MeshAPIGenerateSetStateReq( state_value ) )
                     {              
                        MM_CMD_SendATMessage(AT_RESPONSE_FAIL, cmd_source);                  
                     }
                  }
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
                  
                  uint16_t unit_address = (uint16_t)commandData.Destination;

                  CommandSource_t cmd_source = MM_CMD_ConvertFromATCommandSource(commandData.SourceUSART);

                  uint32_t Handle = GetNextHandle();
                  
                  if( false ==  MM_MeshAPIGenerateRBUDisableReq( Handle, unit_address ) )
                  {              
                     MM_CMD_SendATMessage(AT_RESPONSE_FAIL, cmd_source);
                  }
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

                  CommandSource_t cmd_source = MM_CMD_ConvertFromATCommandSource(commandData.Source);

                  uint32_t Handle = GetNextHandle();
                  
                  if( MM_MeshAPIGenerateRBUReportNodesReq() )
                  {              
                     MM_CMD_SendATMessage(AT_RESPONSE_OK, cmd_source);
                  }
                  else
                  {              
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
                  MM_NCUApplicationStubProcessOutputMessage(outData.Destination, outData.zone, outData.OutputChannel, outData.OutputProfile, outData.OutputsActivated, outData.OutputDuration);
                  
                  /*instruct the Mesh to generate an output signal */
                  uint32_t Handle = GetNextHandle();
                  bool result = MM_MeshAPIOutputSignalReq( Handle, outData.Destination, outData.zone, outData.OutputChannel, outData.OutputProfile, outData.OutputsActivated, outData.OutputDuration, NUM_OUTPUT_SIGNAL_TX );

                  MM_CMD_SendATMessage(AT_RESPONSE_OK, source);

               }
            }
            break;
            case CO_MESSAGE_GENERATE_FAULT_SIGNAL_E:
            {
               CO_FaultData_t* pFaultStatus = (CO_FaultData_t*)pMsg->Payload.PhyDataReq.Data;
               if ( pFaultStatus )
               {
                  MM_NCUApplicationStubUpdateStatusFlags( pFaultStatus );
               }
            }
            break;
            case CO_MESSAGE_PHY_DATA_IND_E:
            {
               ApplicationMessage_t* pMessageData = (ApplicationMessage_t*)pMsg->Payload.PhyDataInd.Data;
               MM_NCUApplicationStubProcessNetworkMessage(pMessageData);
            }
               break;
            case CO_MESSAGE_MAC_EVENT_E:
            {
               ApplicationMessage_t* pMessageData = (ApplicationMessage_t*)pMsg->Payload.PhyDataInd.Data;
               if ( pMessageData )
               {
                  AppMACEventData_t* pEventData = (AppMACEventData_t*)pMessageData->MessageBuffer;
                  MM_NCUProcessMACEvent(pEventData);
               }
            }
               break;
            case CO_MESSAGE_OUTPUT_STATE_MESSAGE_E:
            {
               OutputStateData_t* pOutputData = (OutputStateData_t*)pMsg->Payload.PhyDataInd.Data;
               MM_ApplicationProcessOutputStateMessage(pOutputData);
            }
            break;
            case CO_MESSAGE_CONFIRMATION_E:
            {
               ApplicationMessage_t* pMessageData = (ApplicationMessage_t*)pMsg->Payload.PhyDataInd.Data;
               MM_NCUApplicationStubProcessConfirmation(pMessageData);
            }
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
* MM_NCUApplicationStubUpdateStatusFlags
* Function to update the fault status flags.
*
* @param - FaultStatus - pointer to structure containing fault status update
*
* @return - void
*/
void MM_NCUApplicationStubUpdateStatusFlags( CO_FaultData_t* FaultStatus )
{
   //Nothing to do.
}


/*************************************************************************************/
/**
* MM_NCUApplicationStubProcessNetworkMessage
* Function for processing a received network message.
*
* @param pMessage   The contents of the message.
*
* @return - void
*/
void MM_NCUApplicationStubProcessNetworkMessage(const ApplicationMessage_t* pMessage)
{
   //CO_PRINT_B_0(DBG_INFO_E, "MM_NCUApplicationStubProcessNetworkMessage+\r\n"); 
   if ( pMessage )
   {
      switch ( pMessage->MessageType )
      {
         case APP_MSG_TYPE_FIRE_SIGNAL_E:
            MM_NCUApplicationStubProcessFireSignalIndication(pMessage);
            break;
         case APP_MSG_TYPE_ALARM_SIGNAL_E:
            MM_NCUApplicationStubProcessFirstAidSignalIndication(pMessage);
            break;
         case APP_MSG_TYPE_OUTPUT_STATE_REQUEST_E:
         case APP_MSG_TYPE_OUTPUT_SIGNAL_E:
         {
            /* Not supported by NCU */ 
         }
            break;
         case APP_MSG_TYPE_FAULT_SIGNAL_E:
            MM_NCUApplicationStubProcessFaultStatusIndication(pMessage);
            break;
         case APP_MSG_TYPE_STATUS_INDICATION_E:
            MM_NCUApplicationStubProcessStatusIndication(pMessage);
            break;
         case APP_MSG_TYPE_LOGON_E:
         {
            //CO_PRINT_B_0(DBG_INFO_E, "APP_MSG_TYPE_LOGON_E\r\n"); 
            MM_NCUApplicationStubProcessLogOnIndication(pMessage);
         }
            break;
         case APP_MSG_TYPE_COMMAND_E:
            MM_NCUApplicationStubProcessCommandMessage(pMessage);
            break;
         case APP_MSG_TYPE_RESPONSE_E:
            MM_NCUApplicationStubProcessResponseMessage(pMessage);
            break;
         case APP_MSG_TYPE_STATE_SIGNAL_E:
            MM_NCUApplicationStubProcessSetStateMessage(pMessage);
            break;
         case APP_MSG_TYPE_OUTPUT_STATE_E:
            MM_NCUApplicationStubProcessOutputStateMessage(pMessage);
            break;
         default:
            CO_PRINT_B_1(DBG_ERROR_E,"NCU STUB Rx'd unknown msg type %d\r\n", pMessage->MessageType);
            break;
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationStubProcessOutputMessage
* Function for processing a received output command message.
*
* @param destination       The node that the output is addressed to.
* @param Zone              The zone of the node.
* @param OutputChannel     The output channel.
* @param OutputProfile     The output type.
* @param OutputsActivated  The output value.
* @param OutputDuration    The output duration.
*
* @return - void
*/
void MM_NCUApplicationStubProcessOutputMessage(const uint16_t destination, const uint16_t Zone, const uint8_t OutputChannel, const uint8_t OutputProfile, const uint16_t OutputsActivated, const uint8_t OutputDuration)
{
   //CO_PRINT_B_1(DBG_INFO_E, "NCU Application received Output Signal Ind callback - OutputsActivated=0x%x\r\n", OutputsActivated);   
   
   MM_ApplicationProcessOutputMessage(destination, Zone, OutputChannel, OutputProfile, OutputsActivated, OutputDuration);
}

/*************************************************************************************/
/**
* MM_NCUProcessMACEvent
* Function for processing a received network message.
*
* @param pEventData   The contents of the event.
*
* @return - void
*/
void MM_NCUProcessMACEvent(AppMACEventData_t* pEventData)
{
   if ( pEventData )
   {
      switch ( pEventData->EventType )
      {
         case CO_NEW_SHORT_FRAME_E:
            MM_ApplicationProcessNewShortFrame();
            break;
         default:
            break;
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationStubProcessFaultStatusIndication
* Function for processing a received network message.
*
* @param pMessage   The contents of the message.
*
* @return - void
*/
void MM_NCUApplicationStubProcessFaultStatusIndication(const ApplicationMessage_t* pMessage)
{
   if ( pMessage )
   {
      if ( APP_MSG_TYPE_FAULT_SIGNAL_E == pMessage->MessageType )
      {
#if defined _DEBUG
         FaultSignalIndication_t* pFault = (FaultSignalIndication_t*)pMessage->MessageBuffer;
#endif
         CO_PRINT_B_4(DBG_INFO_E,"NCU Stub Rx'd Fault Status from RBU %d, chan=0x%x, fault=0x%x, value=0x%x\r\n", 
            pFault->SourceAddress, pFault->RUChannelIndex, pFault->FaultType, pFault->Value);
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationStubProcessFaultStatusIndication
* Function for processing a received network message.
*
* @param pMessage   The contents of the message.
*
* @return - void
*/
void MM_NCUApplicationStubProcessLogOnIndication(const ApplicationMessage_t* pMessage)
{
   if ( pMessage )
   {
      if ( APP_MSG_TYPE_LOGON_E == pMessage->MessageType )
      {
#if defined _DEBUG
         LogOnSignalIndication_t* pLogonSignal = (LogOnSignalIndication_t*)pMessage->MessageBuffer;
#endif
         CO_PRINT_B_3(DBG_INFO_E,"NCU Stub Rx'd LOG ON from RBU %d, Zone=%d, DevComb=%d\r\n", 
         pLogonSignal->SourceAddress,pLogonSignal->ZoneNumber, pLogonSignal->DeviceCombination);
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationStubProcessFireSignalIndication
* Function for processing a received fire signal.
*
* @param pMessage   The contents of the message.
*
* @return - void
*/
void MM_NCUApplicationStubProcessFireSignalIndication(const ApplicationMessage_t* pMessage)
{
   if ( pMessage )
   {
      if ( APP_MSG_TYPE_FIRE_SIGNAL_E == pMessage->MessageType )
      {
         AlarmSignalIndication_t* pAlarmSignal = (AlarmSignalIndication_t*)pMessage->MessageBuffer;
         CO_PRINT_B_3(DBG_INFO_E,"NCU STUB Rx'd FIRE SIGNAL from RBU %d, Channel=0x%x, Value=%d\r\n", 
         pAlarmSignal->SourceAddress, pAlarmSignal->RUChannelIndex, pAlarmSignal->SensorValue);
         
         if (  (pAlarmSignal->RUChannelIndex != gLastFireSignalOutputData.OutputProfile) ||
               (pAlarmSignal->SensorValue != gLastFireSignalOutputData.OutputsActivated) ||
               (pAlarmSignal->SourceAddress != gLastFireSignalOutputData.Source) )
         {
            gLastFireSignalOutputData.OutputProfile = pAlarmSignal->RUChannelIndex;
            gLastFireSignalOutputData.OutputsActivated = pAlarmSignal->SensorValue;
            gLastFireSignalOutputData.Source = pAlarmSignal->SourceAddress;
            
            
            DM_InputMonitorState_t newState = (DM_InputMonitorState_t)pAlarmSignal->SensorValue;
            uint32_t handle = GetNextHandle();
            if (IN_MON_ACTIVE_E == newState)
            {
               /* send output signal */
               MM_MeshAPIOutputSignalReq( handle, ADDRESS_GLOBAL, ZONE_GLOBAL, 1, CO_PROFILE_FIRE_E, 0xff, 0, NUM_OUTPUT_SIGNAL_TX );
               MM_NCUApplicationStubProcessOutputMessage(ADDRESS_GLOBAL, ZONE_GLOBAL, CO_CHANNEL_SOUNDER_E, CO_PROFILE_FIRE_E, 0xff, 0);
            }
            else if (IN_MON_IDLE_E == newState)
            {
               MM_MeshAPIOutputSignalReq( handle, ADDRESS_GLOBAL, ZONE_GLOBAL, 1, CO_PROFILE_FIRE_E, 0x00, 0, NUM_OUTPUT_SIGNAL_TX );
               MM_NCUApplicationStubProcessOutputMessage(ADDRESS_GLOBAL, ZONE_GLOBAL, CO_CHANNEL_SOUNDER_E, CO_PROFILE_FIRE_E, 0x00, 0);
            }
         }

      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationStubProcessFirstAidSignalIndication
* Function for processing a received first aid signal.
*
* @param pMessage   The contents of the message.
*
* @return - void
*/
void MM_NCUApplicationStubProcessFirstAidSignalIndication(const ApplicationMessage_t* pMessage)
{
   if ( pMessage )
   {
      if ( APP_MSG_TYPE_ALARM_SIGNAL_E == pMessage->MessageType )
      {
         AlarmSignalIndication_t* pAlarmSignal = (AlarmSignalIndication_t*)pMessage->MessageBuffer;
         CO_PRINT_B_3(DBG_INFO_E,"NCU STUB Rx'd FIRST AID SIGNAL from RBU %d, Channel=0x%x, Value=%d\r\n", 
         pAlarmSignal->SourceAddress, pAlarmSignal->RUChannelIndex, pAlarmSignal->SensorValue);
         
         if ((pAlarmSignal->RUChannelIndex != gLastFirstAidOutputData.OutputProfile) ||
         (pAlarmSignal->SensorValue != gLastFirstAidOutputData.OutputsActivated) ||
         (pAlarmSignal->SourceAddress != gLastFirstAidOutputData.Source) )
         {
         
            gLastFirstAidOutputData.OutputProfile = pAlarmSignal->RUChannelIndex;
            gLastFirstAidOutputData.OutputsActivated = pAlarmSignal->SensorValue;
            gLastFirstAidOutputData.Source = pAlarmSignal->SourceAddress;
            
            
            DM_InputMonitorState_t newState = (DM_InputMonitorState_t)pAlarmSignal->SensorValue;
            uint32_t handle = GetNextHandle();
            if (IN_MON_ACTIVE_E == newState)
            {
               /* send output signal */
               MM_MeshAPIOutputSignalReq( handle, ADDRESS_GLOBAL, ZONE_GLOBAL, 1, CO_PROFILE_FIRST_AID_E, 0xff, 0, NUM_OUTPUT_SIGNAL_TX );
               MM_NCUApplicationStubProcessOutputMessage(ADDRESS_GLOBAL, ZONE_GLOBAL, CO_CHANNEL_SOUNDER_E, CO_PROFILE_FIRST_AID_E, 0xff, 0);
            }
            else if (IN_MON_IDLE_E == newState)
            {
               MM_MeshAPIOutputSignalReq( handle, ADDRESS_GLOBAL, ZONE_GLOBAL, 1, CO_PROFILE_FIRST_AID_E, 0x00, 0, NUM_OUTPUT_SIGNAL_TX );
               MM_NCUApplicationStubProcessOutputMessage(ADDRESS_GLOBAL, ZONE_GLOBAL, CO_CHANNEL_SOUNDER_E, CO_PROFILE_FIRST_AID_E, 0x00, 0);
            }
         }

      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationStubProcessStatusIndication
* Function to receive status information from the Mesh.
*
* @param pAppMessage   The contents of the message.
*
* @return - void
*/
void MM_NCUApplicationStubProcessStatusIndication(const ApplicationMessage_t* pAppMessage)
{
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_STATUS_INDICATION_E == pAppMessage->MessageType )
      {
         StatusIndication_t* pStatus = (StatusIndication_t*)pAppMessage->MessageBuffer;

         if ( pStatus )
         {
            CO_PRINT_B_1(DBG_INFO_E, "Status Ind - node %d\r\n", pStatus->SourceNode);
            CO_PRINT_B_1(DBG_INFO_E, "Num parents %d\r\n", pStatus->NumberOfParents);
            CO_PRINT_B_1(DBG_INFO_E, "Num Child %d\r\n", pStatus->NumberOfChildren);
            CO_PRINT_B_1(DBG_INFO_E, "Num Trk nodes %d\r\n", pStatus->NumberOfTrackingNodes);
            CO_PRINT_B_1(DBG_INFO_E, "Num child PTrk %d\r\n", pStatus->NumberOfChildrenOfPrimaryTrackingNode);
            CO_PRINT_B_1(DBG_INFO_E, "SNR P Parent %d\r\n", pStatus->AverageSNRPrimaryParent);
            CO_PRINT_B_1(DBG_INFO_E, "SNR S Parent %d\r\n", pStatus->AverageSNRSecondaryParent);
            CO_PRINT_B_1(DBG_INFO_E, "Rank %d\r\n", pStatus->Rank);
            CO_PRINT_B_2(DBG_INFO_E, "%s %d\r\n", MESH_EVENT_STR[pStatus->Event], pStatus->EventNodeId);
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationStubProcessMeshStatusEvent
* Function for processing status events received from the Mesh stack.
*
* @param pMessage     Pointer to the status indication message.
*
* @return - None.
*/
void MM_NCUApplicationStubProcessMeshStatusEvent(const ApplicationMessage_t* pAppMessage)
{ 
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_STATUS_SIGNAL_E == pAppMessage->MessageType )
      {
         CO_StatusIndicationData_t* pEventData = (CO_StatusIndicationData_t*)pAppMessage->MessageBuffer;
         if ( pEventData )
         {
            switch ( pEventData->Event )
            {
               case CO_MESH_EVENT_CHILD_NODE_ADDED_E:
                  CO_PRINT_B_1(DBG_INFO_E, "NCU Application : Added new child ID=%d\r\n", pEventData->EventNodeId);
                  MM_ApplicationAddChild(pEventData->EventNodeId, pEventData->EventData);
                  break;
               case CO_MESH_EVENT_CHILD_NODE_DROPPED_E:
                  CO_PRINT_B_1(DBG_ERROR_E, "NCU Application : Dropped child ID=%d\r\n", pEventData->EventNodeId);
                  MM_ApplicationDropChild(pEventData->EventNodeId);
                  break;
               case CO_MESH_EVENT_ZONE_CHANGE_E:
                  CO_PRINT_B_1(DBG_ERROR_E, "NCU Application : Zone number changed to %d\r\n", pEventData->EventData);
                  gZoneNumber = pEventData->EventData;
                  break;
               case CO_MESH_EVENT_STATE_CHANGE_E:
                  MM_ApplicationUpdateDeviceState(pEventData->EventData);
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
* MM_NCUApplicationStubProcessCommandMessage
* Function for processing command messages received from the Mesh stack.
*
* @param pAppMessage    Pointer to the App Message structure.
*
* @return - None.
*/
void MM_NCUApplicationStubProcessCommandMessage(const ApplicationMessage_t* pAppMessage)
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
               }      
            }
            else
            {
               CO_PRINT_B_1(DBG_INFO_E, "RBUApplication rejected duplicate command. TransactionID=%d\r\n", pCommandData->TransactionID);
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationStubProcessResponseMessage
* Function for processing response messages received from the Mesh stack.
*
* @param pAppMessage    Pointer to the App Message structure.
*
* @return - None.
*/
void MM_NCUApplicationStubProcessResponseMessage(const ApplicationMessage_t* pAppMessage)
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
* MM_NCUApplicationStubProcessSetStateMessage
* Function for processing Set State messages received from the Mesh stack.
*
* @param pAppMessage    Pointer to the App Message structure.
*
* @return - None.
*/
void MM_NCUApplicationStubProcessSetStateMessage(const ApplicationMessage_t* pAppMessage)
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
* MM_NCUApplicationStubProcessOutputStateMessage
* Function for processing OUTPUT STATE reports received from children.
*
* @param pAppMessage    Pointer to the App Message structure.
*
* @return - None.
*/
void MM_NCUApplicationStubProcessOutputStateMessage(const ApplicationMessage_t* pAppMessage)
{
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_OUTPUT_STATE_E == pAppMessage->MessageType )
      {
         OutputStateData_t* pOutputData = (OutputStateData_t*)pAppMessage->MessageBuffer;
         MM_ApplicationProcessOutputStateMessage(pOutputData);
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationStubProcessConfirmation
* Function for processing Confirmation messages from the Mesh Stack.
*
* @param pAppMessage    Pointer to the App Message structure.
*
* @return - None.
*/
void MM_NCUApplicationStubProcessConfirmation(const ApplicationMessage_t* pAppMessage)
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
               case APP_CONF_OUTPUT_STATE_REQUEST_E:
                  MM_NCUApplicationStubOutputStateRequestConfirmation(pConfirmation->Handle, pConfirmation->ReplacementHandle, pConfirmation->Error);
                  break;
               case APP_CONF_OUTPUT_STATE_SIGNAL_E:
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx Output State Signal Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
                  break;
               case APP_CONF_BATTERY_STATUS_SIGNAL_E:
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx Battery Status Signal Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
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
* MM_NCUApplicationStubOutputStateRequestConfirmation
* Callback function for Output State Request confirmation.
*
* @param Handle         The Transaction ID for the message.
* @param NewHandle      The replacement Transaction ID for a discarded duplicate message message.
* @param Status         Status flag.  0 for success, 1 for failure
*
* @return - void
*/
void MM_NCUApplicationStubOutputStateRequestConfirmation(const uint32_t Handle, const uint32_t NewHandle, const uint32_t Status)
{
   CO_PRINT_B_1(DBG_INFO_E, "App Rx o/p State Request Cnf - status %d\r\n", Status);
   if ( 0 != Handle )
   {
      int16_t childIndex = MM_ApplicationGetChildIndexFromHandle(Handle);
      if ( APP_CHILD_NOT_FOUND != childIndex )
      {
         switch ( Status )
         {
            case SUCCESS_E:
               childList.childRecord[childIndex].stateQueryHandle = 0;
               childList.childRecord[childIndex].isMuted = false;
               break;
            case ERR_MESSAGE_DUPLICATE_E:
               /* original message was replaced by later version. Record the new message handle */
               for ( uint32_t childIndex = 0; childIndex < MAX_CHILDREN_PER_PARENT; childIndex++ )
               {
                  if (( Handle == childList.childRecord[childIndex].stateQueryHandle ))
                  {
                     childList.childRecord[childIndex].stateQueryHandle = NewHandle;
                  }
               }
               break;
            case ERR_TIMEOUT_E:
               childList.childRecord[childIndex].stateQueryHandle = 0;
               childList.childRecord[childIndex].isMuted = true; /* don't query again */
               break;
            default:
               /* Failed to query the child. Force a retry */
               childList.childRecord[childIndex].stateQueryHandle = 0;
               break;
         }
      }
   }
}

#endif // USE_NCU_STUB
