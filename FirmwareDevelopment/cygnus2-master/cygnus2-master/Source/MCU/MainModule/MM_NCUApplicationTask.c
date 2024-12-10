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
*  File         : MM_NCUApplicationTask.c
*
*  Description  : NCU Application
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "CO_Defines.h"
#include "CFG_Device_cfg.h"
#include "MM_MeshAPI.h"
#include "DM_SerialPort.h"
#include "MM_ATHandleTask.h"
#include "MM_CommandProcessor.h"
#include "DM_InputMonitor.h"
#include "MM_CIEQueueManager.h"
#include "MM_ApplicationCommon.h"
#include "MM_NeighbourInfo.h"
#include "MC_MacConfiguration.h"
#include "MM_ConfigSerialTask.h"
#include "common.h"
#include "MM_Main.h"
#include "MM_PpuManagement.h"
#include "MM_NCUApplicationTask.h"

/* Private Functions Prototypes
*************************************************************************************/
static ErrorCode_t MM_NCUApplicationInit(void);
static void MM_NCUApplicationTaskProcessMessage(const uint32_t timeout);
static void MM_NCUApplicationUpdateStatusFlags( CO_FaultData_t* FaultData );
static void MM_NCUApplicationProcessNetworkMessage(const ApplicationMessage_t* pMessage);
static void MM_NCUApplicationProcessOutputMessage(const uint16_t Destination, const uint16_t Zone, const uint8_t OutputChannel, const uint8_t OutputProfile, const uint16_t OutputsActivated, const uint8_t OutputDuration);
static void MM_NCUProcessMACEvent(AppMACEventData_t* pEventData);
static void MM_NCUApplicationProcessFaultStatusIndication(const ApplicationMessage_t* pMessage);
static void MM_NCUApplicationProcessLogOnIndication(const ApplicationMessage_t* pMessage);
static void MM_NCUApplicationProcessFireSignalIndication(const ApplicationMessage_t* pMessage);
static void MM_NCUApplicationProcessFirstAidSignalIndication(const ApplicationMessage_t* pMessage);
static void MM_NCUApplicationProcessStatusIndication(const ApplicationMessage_t* pAppMessage);
static void MM_NCUApplicationProcessMeshStatusEvent(const ApplicationMessage_t* pAppMessage);
static void MM_NCUApplicationProcessCommandMessage(const ApplicationMessage_t* pAppMessage);
static void MM_NCUApplicationProcessResponseMessage(const ApplicationMessage_t* pAppMessage);
static void MM_NCUApplicationProcessSetStateMessage(const ApplicationMessage_t* pAppMessage);
static void MM_NCUApplicationProcessConfirmation(const ApplicationMessage_t* pAppMessage);
static void MM_NCUApplicationProcessTxBufferQueue(void);
static void MM_NCUApplicationSendNextMeshMessage(void);
static void MM_NCUApplicationProcessOutputSignalQueue(void);
static void MM_NCUApplicationProcessBatteryStatusIndication(const ApplicationMessage_t* pMessage);
static void MM_NCUApplicationProcessNcuCacheRequest(void);
static void MM_NCUApplicationProcessAtCommandMessage(const ApplicationMessage_t* pAppMessage);

/* Global Variables
*************************************************************************************/
//osThreadId tid_NCUTask;
//osThreadDef (MM_NCUApplicationTaskMain, osPriorityNormal, 1, 1200);
extern osPoolId AppPool;
extern osMessageQId(AppQ);
extern osPoolId ATHandlePool;
extern osMessageQId(ATHandleQ);
extern osMessageQId(ConfigSerialQ);       // Queue definition from ConfigSerialTask
extern osPoolId ConfigSerialPool;         // Pool definition from ConfigSerialTask
extern uint16_t gLastCommandedChannel;
extern uint8_t gLastCommandedOutputProfile;
extern uint8_t gLastCommandedOutputActive;
extern uint16_t gLastCommandedOutputDuration;
extern uint8_t gLastCommandedSilenceableMask;
extern uint8_t gLastCommandedUnsilenceableMask;
extern uint8_t gLastCommandedDelayMask;
extern uint16_t gNodeAddress;
extern uint32_t gZoneNumber;
extern const char MESH_EVENT_STR[][64];
extern const char ParamCommandMap[PARAM_TYPE_MAX_E][5];
extern const char CIE_ERROR_CODE_STR[AT_CIE_ERROR_MAX_E][3];
extern NeighbourInformation_t neighbourArray[MAX_DEVICES_PER_SYSTEM];
extern bool gTimerCommandInProgress;
extern uint32_t gResetToPpenMode;
extern uint32_t rbu_pp_mode_request;

/* Private Variables
*************************************************************************************/
static bool gReportStatusFromNcuCache = false;
static uint16_t gReportStatusRequestedStatusNode = 0;
static uint16_t gReportStatusNodeIndex = 0;
static bool MeshAtCommandInProgress = false;    //Set to true if we have received an AT command over the radio and are currently processing it
static AtMessageData_t MeshAtCommand;           //This is a copy of the last received AT command over the radio, for return addressing.


/*************************************************************************************/
/**
* MM_NCUApplicationTaskInit
* Startup  function for the NCU Application task
*
* @param - void
*
* @return - ErrorCode_t   SUCCESS_E if the NCU Application thread was started or error code on failure.
*/
ErrorCode_t MM_NCUApplicationTaskInit(const uint16_t address, const uint16_t device_configuration, const uint32_t unitSerialNo, const uint32_t defaultZoneNumber)
{
   ErrorCode_t result = SUCCESS_E;
   gNodeAddress = address;
   
   MM_CMD_Initialise(address, device_configuration, true, unitSerialNo);
  
   result = MM_NCUApplicationInit();
   CO_ASSERT_RET_MSG(SUCCESS_E == result, result, "ERROR - Failed to intialise NCU Task");
//   /* create thread */
//   tid_NCUTask = osThreadCreate (osThread(MM_NCUApplicationTaskMain), NULL);
//   CO_ASSERT_RET_MSG(NULL != tid_NCUTask, ERR_FAILED_TO_CREATE_THREAD_E, "ERROR - Failed to create NCU Task thread");
   
   return result;
}


/*************************************************************************************/
/**
* MM_NCUApplicationInit
* Initialisation function for the NCU Application Stub
*
* @param - void
*
* @return - ErrorCode_t SUCCESS_E or error code
*/
ErrorCode_t MM_NCUApplicationInit(void)
{
   ErrorCode_t result = SUCCESS_E;
   /*This is the current node.  Read the info from NVM*/
   uint32_t deviceCombination;
   uint32_t unitSerialNo;
   uint32_t zone;
   DM_NVMRead(NV_DEVICE_COMBINATION_E, &deviceCombination, sizeof(deviceCombination));
   DM_NVMRead(NV_UNIT_SERIAL_NO_E, &unitSerialNo, sizeof(unitSerialNo));
   DM_NVMRead(NV_ZONE_NUMBER_E, &zone, sizeof(zone));
   
   neighbourArray[gNodeAddress].NodeID = gNodeAddress;
   neighbourArray[gNodeAddress].SerialNumber = unitSerialNo;
   neighbourArray[gNodeAddress].DeviceCombination = deviceCombination;
   neighbourArray[gNodeAddress].ZoneNumber = zone;

#ifdef USE_NEW_HEAD_INTERFACE
   /* initialise the message queues for the CIE */
   result = MM_CIEQ_Initialise();
   CO_ASSERT_RET_MSG(SUCCESS_E == result, result, "ERROR - Failed to initialise CIE queue");
#endif
   /* initialise the child records */
   MM_ApplicationInitialiseChildArray();
   /* Init neighbour information array */
   MM_NI_Initialise();
   /* start up in IDLE mode */
   MM_ApplicationUpdateDeviceState(STATE_IDLE_E);
   
   return result;
}

/*************************************************************************************/
/**
* MM_NCUApplicationTaskMain
* Main function for RBU Application task.
*
* @param - argument  Pointer to passed in data.
*
* @return - void
*/
void MM_NCUApplicationTaskMain (void)
{
   CO_PRINT_B_0(DBG_INFO_E, "NCU Application running\r\n");
    
   StartPeriodicTimer();
   
   while(true)
   {
      MM_NCUApplicationTaskProcessMessage( osWaitForever );
   };
}

/*************************************************************************************/
/**
* MM_NCUApplicationTaskProcessMessage
* Function to read a message from the AppQ and call the appropriate handling function.
*
* @param - timeout   Limits how long the function should wait for a message to arrive (milliseconds).
*
* @return - void
*/
void MM_NCUApplicationTaskProcessMessage( const uint32_t timeout )
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
            case CO_MESSAGE_CIE_COMMAND_E:
            {
               uint8_t* pCmdType = (uint8_t*)pMsg->Payload.PhyDataReq.Data;
               
               if( pCmdType )
               {
                  CIECommand_t* pCommand = (CIECommand_t*)pMsg->Payload.PhyDataReq.Data;
                  
                  ErrorCode_t result = ERR_INVALID_POINTER_E;
                  if ( pCommand )
                  {
                     //For the command to read the device mesh status, it could either be directed to the NCU cache
                     //or sent to the RBU over the mesh, depending on whether Parameter1 is set in the command.
                     if ( (CIE_CMD_READ_DEVICE_MESH_STATUS_E == pCommand->CommandType) && (0 == pCommand->Parameter1) )
                     {
                        //It's a request to read the status from the NCU cache
                        gReportStatusFromNcuCache = true;
                        gReportStatusRequestedStatusNode = pCommand->NodeID;
                        gReportStatusNodeIndex = pCommand->NodeID == ADDRESS_GLOBAL ? 1 : gReportStatusRequestedStatusNode;
                        result = SUCCESS_E;
                        //Acknowledge the command or the serial link will hang waiting for a response.
                        CommandSource_t cmd_source = MM_CMD_ConvertFromATCommandSource(pCommand->PortNumber);
                        MM_CMD_SendATMessage(AT_RESPONSE_OK, cmd_source);
											 
                     }
                     else
                     {
                        result = MM_CMD_ProcessCIECommand ( pCommand );
                     }
                     
                     if ( SUCCESS_E != result )
                     {
                        CO_PRINT_B_3(DBG_ERROR_E, "FAILED to process CIE command.  cmdType=%d, Source=%d, Error=%d\r\n", (int32_t)pCommand->CommandType, (int32_t)pCommand->PortNumber, result);
                     }
                  }
               }
            }
               break;
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
                  switch (pAppMessage->MessageType)
                  {
                     case APP_MSG_TYPE_STATUS_SIGNAL_E:
                        /* This is the local node status update (added child etc) */
                        MM_NCUApplicationProcessMeshStatusEvent(pAppMessage);
                        break;
                     case APP_MSG_TYPE_STATUS_INDICATION_E:
                        /* This is the status received from RBUs over the mesh*/
                        MM_NCUApplicationProcessStatusIndication(pAppMessage);
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
                        MM_MeshAPIApplicationCommand(APP_MSG_TYPE_ADD_NODE_LINK_E, dropMessage.NodeID, dropMessage.Downlink, 0, 0);
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
                  else 
                  {
                     MM_CMD_SendATMessage(AT_RESPONSE_OK, cmd_source);
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
                  
                  if( MM_MeshAPIGenerateRBUReportNodesReq((uint32_t)cmd_source) )
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
                  
                  /* If the command is targeted at the NCU, there's no point broadcasting.  Return an error, the NCU doesn't have outputs. */
                  if ( NCU_NETWORK_ADDRESS == outData.Destination )
                  {
                     MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
                  }
                  else
                  {
                     /* update local records so that the child outputs can be verified against the latest command */
                     MM_NCUApplicationProcessOutputMessage(outData.Destination, outData.zone, outData.OutputChannel, outData.OutputProfile, outData.OutputsActivated, outData.OutputDuration);
                     
                     outData.Handle = GetNextHandle();
                     /*instruct the Mesh to generate an output signal */
                     CieBuffer_t msgBuffer;
                     memcpy(&msgBuffer, (uint8_t*)&outData, sizeof(CO_OutputData_t));
                     msgBuffer.MessageType = APP_MSG_TYPE_OUTPUT_SIGNAL_E;
                     if ( SUCCESS_E == MM_CIEQ_Push(CIE_Q_OUTPUT_SIGNAL_E, &msgBuffer) )
                     {
                        CO_PRINT_B_1(DBG_INFO_E,"Pushed OUTPUT_SIGNAL onto Tx Queue, handle=%d\r\n", outData.Handle);
                     }
                     else 
                     {
                        CO_PRINT_B_0(DBG_INFO_E,"FAILED to push OUTPUT_SIGNAL onto Tx Queue\r\n");
                     }
                     
                     MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
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
                  
                  alarmData.Handle = GetNextHandle();
                  alarmData.Source = ADDRESS_GLOBAL;
                  /*instruct the Mesh to generate an output signal */
                  CieBuffer_t msgBuffer;
                  memcpy(&msgBuffer, (uint8_t*)&alarmData, sizeof(CO_AlarmOutputStateData_t));
                  msgBuffer.MessageType = APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E;
                  if ( SUCCESS_E == MM_CIEQ_Push(CIE_Q_OUTPUT_SIGNAL_E, &msgBuffer) )
                  {
                     CO_PRINT_B_1(DBG_INFO_E,"Pushed ALARM OUTPUT STATE onto Tx Queue, handle=%d\r\n", alarmData.Handle);
                     /* update local records so that the child outputs can be verified against the latest command */
                     gLastCommandedSilenceableMask = alarmData.Silenceable;
                     gLastCommandedUnsilenceableMask = alarmData.Unsilenceable;
                     gLastCommandedDelayMask = alarmData.DelayMask;
                  }
                  else 
                  {
                     CO_PRINT_B_0(DBG_INFO_E,"FAILED to push ALARM OUTPUT STATE onto Tx Queue\r\n");
                  }
                  MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
               }
            }
            break;
            case CO_MESSAGE_GENERATE_FAULT_SIGNAL_E:
            {
               CO_FaultData_t* pFaultStatus = (CO_FaultData_t*)pMsg->Payload.PhyDataReq.Data;
               if ( pFaultStatus )
               {
                  MM_NCUApplicationUpdateStatusFlags( pFaultStatus );
               }
            }
            break;
            case CO_MESSAGE_GENERATE_ZONE_ENABLE_E:
            {
               uint8_t* pCommand = (uint8_t*)pMsg->Payload.PhyDataReq.Data;

               if( pCommand )
               {
                  ZoneEnableData_t zoneData;
                  memcpy( &zoneData, pCommand, sizeof(CO_CommandData_t));
                  
                  zoneData.Handle = GetNextHandle();
                  
                  CommandSource_t cmd_source = MM_CMD_ConvertFromATCommandSource(zoneData.Source);

                  if( MM_MeshAPIGenerateZoneEnableReq( &zoneData ) )
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
            case CO_MESSAGE_PHY_DATA_IND_E:
            {
               ApplicationMessage_t* pMessageData = (ApplicationMessage_t*)pMsg->Payload.PhyDataInd.Data;
               MM_NCUApplicationProcessNetworkMessage(pMessageData);
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
            case CO_MESSAGE_CONFIRMATION_E:
            {
               ApplicationMessage_t* pMessageData = (ApplicationMessage_t*)pMsg->Payload.PhyDataInd.Data;
               MM_NCUApplicationProcessConfirmation(pMessageData);
            }
               break;
            case CO_MESSAGE_SERVICE_TX_BUFFER_QUEUE_E:
               MM_NCUApplicationProcessOutputSignalQueue();
               MM_NCUApplicationProcessTxBufferQueue();
               break;
            case CO_MESSAGE_TIMER_EVENT_E:
            {
               /* perform periodic actions */
               if ( gReportStatusFromNcuCache )
               {
                  MM_NCUApplicationProcessNcuCacheRequest();
               }
               //See if the PPEN has been changed
               if ( gResetToPpenMode )
               {
                  //The PPEN mode has been changed.  We need to restart, but not until we have 
                  //Sent a route drop to enable connected neighbours to drop the link.
                  //A delayed reset will be invoked by the LoRa stack
                  MM_MeshAPIApplicationCommand(APP_MSG_TYPE_ROUTE_DROP_E, gResetToPpenMode, 0, gNodeAddress, 0);
                  gResetToPpenMode = 0;
               }
               
               //If we are in PPU Disconnected mode, this will send the announcement messages
               MM_PPU_ApplicationProcessPpuPeriodicAnnouncement();
               gTimerCommandInProgress--;
            }
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
            }
               break;
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
               CO_PRINT_B_0(DBG_INFO_E,"NCU CO_MESSAGE_GENERATE_START_OTA_AT_MODE_E\r\n");
               OtaMode_t* pCommand = (OtaMode_t*)pMsg->Payload.PhyDataReq.Data;
               if( pCommand )
               {
                  CO_PRINT_B_0(DBG_INFO_E,"pCommand OK\r\n");
                  if ( false == MM_MeshAPIGenerateStartAtModeMessage(pCommand) )
                  {
                     CO_PRINT_B_0(DBG_INFO_E,"Failed to send AT Mode to the mesh\r\n");
                  }
               }
            }
               break;
            case CO_MESSAGE_PPU_MODE_REQUEST_E:
            {
               //Send a command to put a device into PPU Disconnected mode
               CO_PRINT_B_0(DBG_INFO_E,"NCU CO_MESSAGE_PPU_MODE_REQUEST_E\r\n");
               OtaMode_t* pCommand = (OtaMode_t*)pMsg->Payload.PhyDataReq.Data;
               if( pCommand )
               {
                  if ( false == MM_MeshAPIGenerateStartAtModeMessage(pCommand) )
                  {
                     CO_PRINT_B_0(DBG_INFO_E,"Failed to send PPU Mode to the mesh\r\n");
                  }
               }
            }
               break;
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
* MM_NCUApplicationUpdateStatusFlags
* Function to update the fault status flags.
*
* @param - FaultStatus - pointer to structure containing fault status update
*
* @return - void
*/
void MM_NCUApplicationUpdateStatusFlags( CO_FaultData_t* FaultData )
{
   if( FaultData )
   {
      if ( FAULT_SIGNAL_BATTERY_ERROR_E == FaultData->FaultType )
      {
         if ( FaultData->Value )
         {
            MM_MeshAPIApplicationCommand(APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E, 0, 0, 0, 0);
         }
         else
         {
            /* wake up the serial ports from sleep */
//            osSemaphoreRelease(ConfigSerialSemaphoreId);
            
            MM_MeshAPIApplicationCommand(APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E, 1, 0, 0, 0);
         }
      }
      
      /* Drop the fault status update into the CIE Fault queue */
      ApplicationMessage_t faultMessage;
      faultMessage.MessageType = APP_MSG_TYPE_FAULT_SIGNAL_E;
      FaultSignalIndication_t faultInd;
      faultInd.SourceAddress = gNodeAddress;
      faultInd.RUChannelIndex = FaultData->RUChannelIndex;
      faultInd.FaultType = FaultData->FaultType;
      faultInd.Value = FaultData->Value;
      faultInd.OverallFault = FaultData->OverallFault;
      memcpy(&faultMessage, &faultInd, sizeof(FaultSignalIndication_t));
      MM_NCUApplicationProcessFaultStatusIndication(&faultMessage);
   }
}


/*************************************************************************************/
/**
* MM_NCUApplicationProcessNetworkMessage
* Function for processing a received network message.
*
* @param pMessage   The contents of the message.
*
* @return - void
*/
void MM_NCUApplicationProcessNetworkMessage(const ApplicationMessage_t* pMessage)
{
   if ( pMessage )
   {
      switch ( pMessage->MessageType )
      {
         case APP_MSG_TYPE_FIRE_SIGNAL_E:
            MM_NCUApplicationProcessFireSignalIndication(pMessage);
            break;
         case APP_MSG_TYPE_ALARM_SIGNAL_E:
            MM_NCUApplicationProcessFirstAidSignalIndication(pMessage);
            break;
         case APP_MSG_TYPE_OUTPUT_SIGNAL_E:
         {
            /* Not supported by NCU */ 
         }
            break;
         case APP_MSG_TYPE_FAULT_SIGNAL_E:
            MM_NCUApplicationProcessFaultStatusIndication(pMessage);
            break;
         case APP_MSG_TYPE_STATUS_INDICATION_E:
            MM_NCUApplicationProcessStatusIndication(pMessage);
            break;
         case APP_MSG_TYPE_LOGON_E:
         {
            //CO_PRINT_B_0(DBG_INFO_E, "APP_MSG_TYPE_LOGON_E\r\n"); 
            MM_NCUApplicationProcessLogOnIndication(pMessage);
         }
            break;
         case APP_MSG_TYPE_COMMAND_E:
            MM_NCUApplicationProcessCommandMessage(pMessage);
            break;
         case APP_MSG_TYPE_RESPONSE_E:
            MM_NCUApplicationProcessResponseMessage(pMessage);
            break;
         case APP_MSG_TYPE_STATE_SIGNAL_E:
            MM_NCUApplicationProcessSetStateMessage(pMessage);
            break;
         case APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E:
            MM_NCUApplicationProcessBatteryStatusIndication(pMessage);
            break;
         case APP_MSG_TYPE_AT_COMMAND_E:
            MM_NCUApplicationProcessAtCommandMessage(pMessage);
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
         default:
            CO_PRINT_B_1(DBG_ERROR_E,"NCU Rx'd unknown msg type %d\r\n", pMessage->MessageType);
            break;
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationProcessOutputMessage
* Function for processing a received output command message.
*
* @param - Destination        The network node address of the target unit.
* @param - Zone               The zone number in which the outputs should be activated.
* @param - OutputChannel      The output channel number.
* @param - OutputProfile      An identifier for the output type (fire/first aid/PIR/Test/Silent Test).
* @param - OutputsActivated   The outputs that are to be activated/deactivated.
* @param - OutputDuration     The duration that the output should be activated/deactivated.
*
* @return - void
*/
void MM_NCUApplicationProcessOutputMessage(const uint16_t Destination, const uint16_t Zone, const uint8_t OutputChannel, const uint8_t OutputProfile, const uint16_t OutputsActivated, const uint8_t OutputDuration)
{
   MM_ApplicationProcessOutputMessage(Destination, Zone, OutputChannel, OutputProfile, OutputsActivated, OutputDuration);
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
            /* Send next downlink message */
            MM_NCUApplicationSendNextMeshMessage();
            break;
         case CO_ENTER_PPU_OTA_MODE_E:
            FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
            
            //rbu_pp_mode_request = RBU_RADIO_PP_MODE_REQ;
            rbu_pp_mode_request = STARTUP_RADIO_BOOTLOADER;
  
            /* Request a micro reset */
            HAL_NVIC_SystemReset();
            break;
         default:
            break;
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationProcessFaultStatusIndication
* Function for processing a received network message.
*
* @param pMessage   The contents of the message.
*
* @return - void
*/
void MM_NCUApplicationProcessFaultStatusIndication(const ApplicationMessage_t* pMessage)
{
   if ( pMessage )
   {
      if ( APP_MSG_TYPE_FAULT_SIGNAL_E == pMessage->MessageType )
      {
         FaultSignalIndication_t* pFault = (FaultSignalIndication_t*)pMessage->MessageBuffer;
         if ( pFault )
         {
            CO_PRINT_B_4(DBG_INFO_E,"NCU Rx'd Fault Status from NodeID %d, chan=0x%x, fault=0x%x, value=0x%x\r\n", 
               pFault->SourceAddress, pFault->RUChannelIndex, pFault->FaultType, pFault->Value);
            
            //Update the neighbour info fault flag
            MM_NI_SetFaultStatus(pFault->SourceAddress, (bool)pFault->OverallFault );

            /* create the fault msg for the CIE */
            CieBuffer_t faultMsg;
            snprintf((char*)faultMsg.Buffer, CIE_QUEUE_DATA_SIZE, "Z%dU%d,%d,%d,%d,%d", neighbourArray[pFault->SourceAddress].ZoneNumber, pFault->SourceAddress, 
                              pFault->RUChannelIndex, pFault->FaultType, pFault->Value, pFault->OverallFault);
            /*push the message into the CIE fault queue */
            ErrorCode_t error = MM_CIEQ_Push(CIE_Q_FAULT_E, &faultMsg);
            if ( SUCCESS_E != error )
            {
               CO_PRINT_A_1(DBG_ERROR_E,"Failed to add fault signal to CIE queue.  Error=%d\r\n", error);
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationProcessLogOnIndication
* Function for processing a received network message.
*
* @param pMessage   The contents of the message.
*
* @return - void
*/
void MM_NCUApplicationProcessLogOnIndication(const ApplicationMessage_t* pMessage)
{
   if ( pMessage )
   {
      if ( APP_MSG_TYPE_LOGON_E == pMessage->MessageType )
      {
         LogOnSignalIndication_t* pLogonSignal = (LogOnSignalIndication_t*)pMessage->MessageBuffer;
         CO_PRINT_B_3(DBG_INFO_E,"NCU Rx'd LOG ON from RBU %d, Zone=%d, DevComb=%d\r\n", 
         pLogonSignal->SourceAddress,pLogonSignal->ZoneNumber, pLogonSignal->DeviceCombination);
         
         /* update the NCU's record of units in the system */
         if ( MAX_DEVICES_PER_SYSTEM > pLogonSignal->SourceAddress )
         {
            neighbourArray[pLogonSignal->SourceAddress].NodeID = pLogonSignal->SourceAddress;
            neighbourArray[pLogonSignal->SourceAddress].SerialNumber = pLogonSignal->SerialNumber;
            neighbourArray[pLogonSignal->SourceAddress].DeviceCombination = pLogonSignal->DeviceCombination;
            neighbourArray[pLogonSignal->SourceAddress].ZoneNumber = pLogonSignal->ZoneNumber;
         }
         
         /* put a msg into the CIE Misc queue */
         char ser_no_str[16];
         if ( MM_ATDecodeSerialNumber( pLogonSignal->SerialNumber, ser_no_str) )
         {                  
            CieBuffer_t logBuffer;
            snprintf((char*)logBuffer.Buffer, CIE_QUEUE_DATA_SIZE, "LOG:Z%dU%d,0,0,2,%s,%d", pLogonSignal->ZoneNumber,
                           pLogonSignal->SourceAddress, ser_no_str, pLogonSignal->DeviceCombination);
            
            ErrorCode_t error = MM_CIEQ_Push(CIE_Q_MISC_E, &logBuffer);
            if ( SUCCESS_E != error )
            {
               CO_PRINT_A_2(DBG_ERROR_E,"Failed to add Logon message to CIE queue.  nodeId=%d, Error=%d\r\n", pLogonSignal->SourceAddress, error);
            }
         }         
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationProcessFireSignalIndication
* Function for processing a received fire signal.
*
* @param pMessage   The contents of the message.
*
* @return - void
*/
void MM_NCUApplicationProcessFireSignalIndication(const ApplicationMessage_t* pMessage)
{
   if ( pMessage )
   {
      if ( APP_MSG_TYPE_FIRE_SIGNAL_E == pMessage->MessageType )
      {
         AlarmSignalIndication_t* pAlarmSignal = (AlarmSignalIndication_t*)pMessage->MessageBuffer;
         CO_PRINT_B_4(DBG_INFO_E,"NCU Rx'd FIRE SIGNAL from RBU %d, Channel=0x%x, AlarmActive=%d, Value=%d\r\n", 
         pAlarmSignal->SourceAddress, pAlarmSignal->RUChannelIndex, pAlarmSignal->AlarmActive, pAlarmSignal->SensorValue);
         
         /* create the fire msg for the CIE */
         CieBuffer_t fireMsg;
//         sprintf((char*)fireMsg.Buffer, "Z%dU%d,%d,%d", pAlarmSignal->Zone, pAlarmSignal->SourceAddress, pAlarmSignal->RUChannelIndex, pAlarmSignal->SensorValue);
         sprintf((char*)fireMsg.Buffer, "Z%dU%d,%d,%d,%d", pAlarmSignal->Zone, pAlarmSignal->SourceAddress, pAlarmSignal->RUChannelIndex, pAlarmSignal->AlarmActive, pAlarmSignal->SensorValue);
         /*push the message into the CIE fire queue */
         ErrorCode_t error = MM_CIEQ_Push(CIE_Q_FIRE_E, &fireMsg);
         if ( SUCCESS_E != error )
         {
            CO_PRINT_A_1(DBG_ERROR_E,"Failed to add fire signal to CIE queue.  Error=%d\r\n", error);
         }

      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationProcessFirstAidSignalIndication
* Function for processing a received first aid signal.
*
* @param pMessage   The contents of the message.
*
* @return - void
*/
void MM_NCUApplicationProcessFirstAidSignalIndication(const ApplicationMessage_t* pMessage)
{
   if ( pMessage )
   {
      if ( APP_MSG_TYPE_ALARM_SIGNAL_E == pMessage->MessageType )
      {
         AlarmSignalIndication_t* pAlarmSignal = (AlarmSignalIndication_t*)pMessage->MessageBuffer;
         CO_PRINT_B_4(DBG_INFO_E,"NCU Rx'd FIRST AID SIGNAL from RBU %d, Channel=0x%x, AlarmActive=%d, Value=%d\r\n", 
         pAlarmSignal->SourceAddress, pAlarmSignal->RUChannelIndex, pAlarmSignal->AlarmActive, pAlarmSignal->SensorValue);
         /* create the alarm msg for the CIE */
         CieBuffer_t alarmMsg;
//         sprintf((char*)alarmMsg.Buffer, "Z%dU%d,%d,%d", pAlarmSignal->Zone, pAlarmSignal->SourceAddress, pAlarmSignal->RUChannelIndex, pAlarmSignal->SensorValue);
         sprintf((char*)alarmMsg.Buffer, "Z%dU%d,%d,%d,%d", pAlarmSignal->Zone, pAlarmSignal->SourceAddress, pAlarmSignal->RUChannelIndex, pAlarmSignal->AlarmActive, pAlarmSignal->SensorValue);
         /*push the message into the CIE alarm queue */
         ErrorCode_t error = MM_CIEQ_Push(CIE_Q_ALARM_E, &alarmMsg);
         if ( SUCCESS_E != error )
         {
            CO_PRINT_A_1(DBG_ERROR_E,"Failed to add alarm signal to CIE queue.  Error=%d\r\n", error);
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationProcessStatusIndication
* Function to receive status information from the Mesh.
*
* @param pAppMessage   The contents of the message.
*
* @return - void
*/
void MM_NCUApplicationProcessStatusIndication(const ApplicationMessage_t* pAppMessage)
{
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_STATUS_INDICATION_E == pAppMessage->MessageType )
      {
         StatusIndication_t* pStatus = (StatusIndication_t*)pAppMessage->MessageBuffer;

         if ( pStatus )
         {
            /* convert 9-bit RSSI to 32 bit, retaining the sign */
            int16_t primaryParentRssi = pStatus->AverageRssiPrimaryParent | RSSI_SIGNED_BITS;
            
            int16_t secondary_parent_id;
            int16_t secondaryParentRssi;
            if ( CO_BAD_NODE_ID == pStatus->SecondaryParent )
            {
               secondary_parent_id = -1;
               secondaryParentRssi = -256;
            }
            else
            {
               secondary_parent_id = (int16_t)pStatus->SecondaryParent;
               secondaryParentRssi = pStatus->AverageRssiSecondaryParent | RSSI_SIGNED_BITS;
            }
            
            //Only send to the control panel if the status has changed since the last one that was sent
            // or if Event == CO_MESH_EVENT_NONE_E ( CIE commanded )
            if ( neighbourArray[pStatus->SourceNode].LastStatusEvent != pStatus->Event ||
                 neighbourArray[pStatus->SourceNode].LastStatusEventNodeId != pStatus->EventData ||
                 pStatus->Event == CO_MESH_EVENT_NONE_E )
            {

               
               /* build a message string and push it into the CIE Misc Queue */
               NeighbourInformation_t neighbour;
               if ( MM_NI_GetNeighbourInformation( pStatus->SourceNode, &neighbour ))
               {
                  CieBuffer_t statusMsg = {0};
                  snprintf((char*)statusMsg.Buffer, CIE_QUEUE_DATA_SIZE, "%s:Z%dU%d,%d,%d,9,%d,%d,%d,%d,%d,%d,%d,%d,%d", (pStatus->Event == CO_MESH_EVENT_NONE_E ? "MSR":"MSI"),
                     neighbourArray[pStatus->SourceNode].ZoneNumber, pStatus->SourceNode,
                     neighbour.primaryBattery_mv,
                     neighbour.backupBattery_mv,
                     pStatus->PrimaryParent, secondary_parent_id, primaryParentRssi, secondaryParentRssi, 
                     pStatus->Event, pStatus->EventData, pStatus->Rank, neighbourArray[pStatus->SourceNode].DeviceCombination, pStatus->OverallFault);
                  
                  //Update the neighbour info fault flag
                  MM_NI_SetFaultStatus(pStatus->SourceNode, pStatus->OverallFault );

                  /*push the message into the CIE misc queue */
                  ErrorCode_t error = MM_CIEQ_Push(CIE_Q_MISC_E, &statusMsg);
                  if ( SUCCESS_E == error )
                  {
                     //Record the last status event for the source node so that we can filter out duplicates.
                     neighbourArray[pStatus->SourceNode].LastStatusEvent = pStatus->Event;
                     neighbourArray[pStatus->SourceNode].LastStatusEventNodeId = pStatus->EventData;
                  }
                  else
                  {
                     CO_PRINT_A_1(DBG_ERROR_E,"Failed to add Mesh Status to CIE misc queue.  Error=%d\r\n", error);
                  }
               }
               
               /* if the status was requested by the NCU we need to remove the command from the TxBuffer queue */
               if ( CO_MESH_EVENT_NONE_E == pStatus->Event )
               {
                  /* The NCU requeset the status update.  Remove the command from the TxBuffer */
                  
                  /* Get the message from the TxBuffer and verify that it matches the response */
                  CieBuffer_t cmdMessage;
                  if ( SUCCESS_E == MM_CIEQ_Pop(CIE_Q_TX_BUFFER_E, &cmdMessage) )
                  {
                     CO_CommandData_t* pCommandData = (CO_CommandData_t*)cmdMessage.Buffer;
                     /* check that the transaction ID and the message type matches */
                     if ( PARAM_TYPE_MESH_STATUS_E == pCommandData->CommandType )
                     {
                        /* The response is a match for the command*/
                        /* Clear the command message from the TxBuffer queue */
                        ErrorCode_t error = MM_CIEQ_Discard(CIE_Q_TX_BUFFER_E);
                        if ( SUCCESS_E != error )
                        {
                           CO_PRINT_B_1(DBG_ERROR_E,"On Response : Failed to remove MSR from the TxBuffer Queue. Error=%d\r\n", (int32_t)error);
                        }
                     }
                  }
                  
               }
            }
            
            /* Update the neighbour information array */
            if ( (uint8_t)CO_MESH_EVENT_PRIMARY_TRACKING_NODE_PROMOTED_E >= pStatus->Event )
            {
               if ( MM_NI_UpdateMeshStatus( pStatus ) )
               {
                  CO_PRINT_B_3(DBG_INFO_E,"Updated neighbour information for node %d.  Event=%s, EventNodeID=%d\r\n", pStatus->SourceNode, MESH_EVENT_STR[pStatus->Event], pStatus->EventData);
               }
               else
               {
                  CO_PRINT_B_0(DBG_INFO_E,"Failed to update neighbour information for node\r\n");
               }
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationProcessMeshStatusEvent
* Function for processing status events received from the Mesh stack.
*
* @param pMessage     Pointer to the status indication message.
*
* @return - None.
*/
void MM_NCUApplicationProcessMeshStatusEvent(const ApplicationMessage_t* pAppMessage)
{ 
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_STATUS_SIGNAL_E == pAppMessage->MessageType )
      {
         CO_StatusIndicationData_t* pEventData = (CO_StatusIndicationData_t*)pAppMessage->MessageBuffer;
         if ( pEventData )
         {
            StatusIndication_t statusInd;
            statusInd.Event = pEventData->Event;
            statusInd.SourceNode = NCU_NETWORK_ADDRESS;
            statusInd.EventData = pEventData->EventNodeId;
            
            switch ( pEventData->Event )
            {
               case CO_MESH_EVENT_CHILD_NODE_ADDED_E:
                  CO_PRINT_B_1(DBG_INFO_E, "NCU Application : Added new child ID=%d\r\n", pEventData->EventNodeId);
                  MM_ApplicationAddChild(pEventData->EventNodeId, pEventData->EventData);
                  MM_NI_UpdateMeshStatus(&statusInd);
                  break;
               case CO_MESH_EVENT_CHILD_NODE_DROPPED_E:
                  CO_PRINT_B_1(DBG_ERROR_E, "NCU Application : Dropped child ID=%d\r\n", pEventData->EventNodeId);
                  MM_ApplicationDropChild(pEventData->EventNodeId);
                  MM_NI_UpdateMeshStatus(&statusInd);
                  break;
               case CO_MESH_EVENT_ZONE_CHANGE_E:
                  CO_PRINT_B_1(DBG_INFO_E, "NCU Application : Zone number changed to %d\r\n", pEventData->EventData);
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
* MM_NCUApplicationProcessCommandMessage
* Function for processing command messages received from the Mesh stack.
*
* @param pAppMessage    Pointer to the App Message structure.
*
* @return - None.
*/
void MM_NCUApplicationProcessCommandMessage(const ApplicationMessage_t* pAppMessage)
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
                  CO_PRINT_B_1(DBG_ERROR_E, "NCUApplication FAILED to process command.  Error=%d\r\n", result);
               }      
            }
            else
            {
               CO_PRINT_B_1(DBG_INFO_E, "NCUApplication rejected duplicate command. TransactionID=%d\r\n", pCommandData->TransactionID);
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationProcessResponseMessage
* Function for processing response messages received from the Mesh stack.
*
* @param pAppMessage    Pointer to the App Message structure.
*
* @return - None.
*/
void MM_NCUApplicationProcessResponseMessage(const ApplicationMessage_t* pAppMessage)
{
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_RESPONSE_E == pAppMessage->MessageType )
      {
         CO_ResponseData_t* pResponseData = (CO_ResponseData_t*)pAppMessage->MessageBuffer;
         if ( pResponseData )
         {
            /* check that we were expecting a response */
            if ( 0 < MM_CIEQ_MessageCount(CIE_Q_TX_BUFFER_E) )
            {
               /* get the message from the TxBuffer and verify that it matches the response */
               CieBuffer_t cmdMessage;
               if ( SUCCESS_E == MM_CIEQ_Pop(CIE_Q_TX_BUFFER_E, &cmdMessage) )
               {
                  CO_CommandData_t* pCommandData = (CO_CommandData_t*)cmdMessage.Buffer;
                  /* check that the transaction ID and the message type matches */
                  if ( pCommandData &&
                       pResponseData->CommandType == pCommandData->CommandType &&
                       pResponseData->TransactionID == pCommandData->TransactionID )
                  {
                     /* The response is a match for the command.  Put a message in the Misc queue for the CIE */
                     CieBuffer_t cieMsg;
                     switch(pResponseData->CommandType)
                     {
                        case PARAM_TYPE_RBU_SERIAL_NUMBER_E:   /* intentional drop-through */
                        case PARAM_TYPE_PLUGIN_SERIAL_NUMBER_E:
                        {
                           char ser_no_str[16];
                           if ( MM_ATDecodeSerialNumber( pResponseData->Value, ser_no_str) )
                           {                           
                              snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "%s:Z%dU%d,%d,%d,1,%s\r\n", ParamCommandMap[pResponseData->CommandType],
                                    neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source,pResponseData->Parameter1,pResponseData->Parameter2,ser_no_str);
                           }
                           else 
                           {
                              snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "%s:ERROR\r\n", ParamCommandMap[pResponseData->CommandType]);
                           }
                        }
                        break;
                        case PARAM_TYPE_FIRMWARE_INFO_E:
                        {
                           uint32_t base_version = pResponseData->Value >> 14;
                           uint32_t revision = (pResponseData->Value >> 7) & 0x7f;
                           uint32_t sub_revision = pResponseData->Value & 0x7f;
                           
                           //char* pCommandStr = (char*)ParamCommandMap[pResponseData->CommandType];
                           
                           if ( FINDX_RBU_MAIN_IMAGE_E == pResponseData->Parameter1 )
                           {
                              snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "FIR:Z%dU%d,0,0,1,%d.%d.%d\r\n",
                                    neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source,base_version,revision,sub_revision);
                           }
                           else if ( FINDX_EXTERNAL_INTERFACE_1_HEAD_E == pResponseData->Parameter1 )
                           {
                              uint8_t version_str[20];
                              if ( MM_HeadDecodeFirmwareVersion(pResponseData->Value, version_str, 20) )
                              {
                                 /* The version number decoded OK */
                                 snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "FIP:Z%dU%d,0,0,2,%s\r\n",
                                       neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source, (char*)version_str);
                              }
                              else
                              {
                                 /* The version number failed to decode */
                                 snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "FIP:Z%dU%d,0,0,1,ERROR\r\n",
                                       neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source);
                              }
                           }
                           else 
                           {
                              /* Unrecognised target */
                              snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "%s:Z%dU%d,0,0,1,ERROR\r\n",ParamCommandMap[pResponseData->CommandType],
                                    neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source);

                           }
                        }
                        break;
                        case PARAM_TYPE_FLASH_RATE_E:       /* Intentional drop-through */
                        case PARAM_TYPE_TONE_SELECTION_E:
                        {
                           if ( ERROR_VALUE == pResponseData->Value )
                           {
                              snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "%s:Z%dU%d,ERROR\r\n", ParamCommandMap[pResponseData->CommandType],
                                 neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source);
                           }
                           else 
                           {
                              /*OutputsEnabled in byte0 of value and OutputProfile in byte1.*/
                              uint32_t OutputsEnabled = pResponseData->Value & 0xff;
                              uint32_t OutputProfile = (pResponseData->Value >> 8) & 0xff;
                              
                              snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "%s:Z%dU%d,%d,%d,1,%d\r\n", ParamCommandMap[pResponseData->CommandType],
                                       neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source,pResponseData->Parameter1, OutputProfile, OutputsEnabled);
                           }
                        }
                           break;
                        case PARAM_TYPE_INDICATOR_LED_E:
                              snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "%s:Z%dU%d,%d\r\n", ParamCommandMap[pResponseData->CommandType],
                                       neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source, pResponseData->Value);
                           break;
                        case PARAM_TYPE_SOUND_LEVEL_E:
                              snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "SL:Z%dU%d,%d\r\n",
                                       neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source, pResponseData->Value);
                           break;
                        case PARAM_TYPE_ALARM_DELAYS_E:
                           {
                              uint32_t delay1 = (pResponseData->Value & 0x1FF);
                              uint32_t delay2 = ((pResponseData->Value >> 9) & 0x1FF);
                              snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "ADC:Z%dU%d,%d,%d,%d\r\n",
                                       neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source, pResponseData->Parameter1, delay1, delay2);
                           }
                           break;
                        case PARAM_TYPE_CHANNEL_FLAGS_E:
                              snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "CHF:Z%dU%d,%d,%d\r\n",
                                       neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source, pResponseData->Parameter1, pResponseData->Value);
                           break;
                        case PARAM_TYPE_ALARM_CONFIG_E:
                              snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "ACS:Z%dU%d,%d,%d\r\n",
                                       neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source, pResponseData->Parameter1, pResponseData->Value);
                           break;
                        case PARAM_TYPE_MAX_RANK_E:
                              snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "RNK:Z%dU%d,%d\r\n",
                                       neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source, pResponseData->Value);
                           break;
                        case PARAM_TYPE_PRODUCT_CODE_E:
                        {
                           char product_code[16];
                           if ( MM_ATDecodeProductCode( pResponseData->Value, product_code) )
                           {
                              //CO_PRINT_B_3(DBG_INFO_E,"Rx'd from node %d value=0x%x, pcode=%s\r\n", pResponseData->Source, pResponseData->Value, product_code);
                              snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "SERPX:Z%dU%d,%s\r\n",
                                    neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source, (char*)product_code);
                              //CO_PRINT_B_1(DBG_INFO_E,"CIE Report: %s\r\n", (char*)cieMsg.Buffer);
                           }
                           else 
                           {
                              snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "SERPX:ERROR\r\n");
                           }
                        }
                        break;
                        case PARAM_TYPE_PPU_MODE_ENABLE_E:
                              snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "PPEN:Z%dU%d,%d\r\n",
                                       neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source, pResponseData->Value);
                           break;
                        case PARAM_TYPE_DEPASSIVATION_SETTINGS_E:
                        {
                           DePassivationSettings_t depassivation_settings;
                           CFG_DecompressDepassivationSettings(pResponseData->Value, &depassivation_settings);
                           snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "DPASD:Z%dU%d,%d,%d,%d,%d\r\n", neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source, 
                                    depassivation_settings.Voltage,depassivation_settings.Timeout,depassivation_settings.OnTime,depassivation_settings.OffTime);
                        }
                           break;
                        default:
                           snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "%s:Z%dU%d,%d,%d,1,%d\r\n", ParamCommandMap[pResponseData->CommandType],
                                    neighbourArray[pResponseData->Source].ZoneNumber, pResponseData->Source,pResponseData->Parameter1,pResponseData->Parameter2,pResponseData->Value);
                        break;
                     }
                     
                     
                     MM_CIEQ_Push(CIE_Q_MISC_E, &cieMsg);
                     CO_PRINT_B_1(DBG_INFO_E,"%s", (char*)cieMsg.Buffer);
                     
                     /* clear the command message from the TxBuffer queue */
                     ErrorCode_t error = MM_CIEQ_Discard(CIE_Q_TX_BUFFER_E);
                     if ( SUCCESS_E != error )
                     {
                        CO_PRINT_B_2(DBG_ERROR_E,"On Response : Failed to remove %s from the TxBuffer Queue. Error=%d\r\n", ParamCommandMap[pResponseData->CommandType], (int32_t)error);
                     }
                  }
               }
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationProcessSetStateMessage
* Function for processing Set State messages received from the Mesh stack.
*
* @param pAppMessage    Pointer to the App Message structure.
*
* @return - None.
*/
void MM_NCUApplicationProcessSetStateMessage(const ApplicationMessage_t* pAppMessage)
{
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_STATE_SIGNAL_E == pAppMessage->MessageType )
      {
#if defined _DEBUG
         uint8_t newState = pAppMessage->MessageBuffer[0];
#endif
         CO_PRINT_B_1(DBG_INFO_E,"NCUApplication set state to %d\r\n", newState);
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationProcessConfirmation
* Function for processing Confirmation messages from the Mesh Stack.
*
* @param pAppMessage    Pointer to the App Message structure.
*
* @return - None.
*/
void MM_NCUApplicationProcessConfirmation(const ApplicationMessage_t* pAppMessage)
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
               {
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx Output Signal Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
                  bool msg_sent = (0 == pConfirmation->Error ? true:false);
                  MM_CIEQ_OutputSignalConfirmation(pConfirmation->Handle, msg_sent);
               }
                  break;
               case APP_CONF_LOGON_SIGNAL_E:
                  break;
               case APP_CONF_STATUS_SIGNAL_E:
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx Status Signal Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
                  break;
               case APP_CONF_COMMAND_SIGNAL_E:
               {
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx Command Signal Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
                  bool msg_sent = (0 == pConfirmation->Error ? true:false);
                  MM_CIEQ_TxBufferConfirmation(pConfirmation->Handle, msg_sent);
                  break;
               }
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
               {
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx Alarm Output State Signal Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
                  bool msg_sent = (0 == pConfirmation->Error ? true:false);
                  //bool msg_outstanding = MM_CIEQ_OutputSignalWaitingForConfirm();
                  //Function MM_CIEQ_AlarmOutputSignalConfirmation will return false if the confirmation matched the queued output message
                  bool msg_matched = !MM_CIEQ_AlarmOutputSignalConfirmation(pConfirmation->Handle, msg_sent);
                  break;
               }
               case APP_CONF_ZONE_ENABLE_MESSAGE_E:
                  CO_PRINT_B_2(DBG_INFO_E, "App Rx Zone Enable Cnf Handle=%d, status=%d\r\n", pConfirmation->Handle, pConfirmation->Error);
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
* MM_NCUApplicationProcessTxBufferQueue
* Function to check whether the next TxBuffer command should be sent to the mesh.
*
* @param Handle         The Transaction ID for the message.
* @param NewHandle      The replacement Transaction ID for a discarded duplicate message message.
* @param Status         Status flag.  0 for success, 1 for failure
*
* @return - void
*/
void MM_NCUApplicationProcessTxBufferQueue(void)
{
   //The 'output signal' queue takes priority over the standard downlink queue (TxBuffer) so check that it is empty before proceeding
   if ( 0 == MM_CIEQ_MessageCount(CIE_Q_OUTPUT_SIGNAL_E) )
   {
      /* are there any messages in the Tx Buffer? */
      if ( 0 < MM_CIEQ_MessageCount(CIE_Q_TX_BUFFER_E) )
      {
         /* get the 1st message */
         CieBuffer_t message;
         ErrorCode_t error = MM_CIEQ_Pop(CIE_Q_TX_BUFFER_E, &message);
         if ( SUCCESS_E == error )
         {
            if ( APP_MSG_TYPE_COMMAND_E == message.MessageType )
            {
               CO_CommandData_t* pCommand = (CO_CommandData_t*)message.Buffer;
               /* check if the message has already been sent.  If not send it now */
               if ( false == MM_CIEQ_TxBufferWaitingForResponse() )
               {
                  CO_PRINT_B_0(DBG_INFO_E,"TxBuffer sending msg\r\n");
                  bool command_expects_response = false;
                  
                  if ( COMMAND_READ == pCommand->ReadWrite )
                  {
                     command_expects_response = MM_CMD_CommandExpectsResponse((ParameterType_t)pCommand->CommandType, pCommand->Destination);
                  }
                  
                  uint8_t number_to_send = CFG_GetNumberOfResends();
                  
                  MM_MeshAPICommandReq(pCommand->TransactionID, pCommand->Source, pCommand->Destination, pCommand->CommandType,
                                       pCommand->Parameter1, pCommand->Parameter2, pCommand->Value, pCommand->ReadWrite, number_to_send);
                  
                  //If no response is expected, discard the message from the front of the queue
                  if ( false == command_expects_response )
                  {
                     MM_CIEQ_Discard(CIE_Q_TX_BUFFER_E);
                  }
               }
               else if ( MM_CIEQ_TxBufferTimedOut() ) /* msg was already sent.  check for time-out */
               {
                  /* the message has timed out.  Report an error to the CIE via the Misc Queue and discard it*/
                  CieBuffer_t cieMsg;
                  snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "%s: ERROR,%s\r\n", ParamCommandMap[pCommand->CommandType],
                           CIE_ERROR_CODE_STR[AT_CIE_ERROR_NETWORK_TIMEOUT_E]);
                  MM_CIEQ_Push(CIE_Q_MISC_E, &cieMsg);
                  
                  error = MM_CIEQ_Discard(CIE_Q_TX_BUFFER_E);
                  if ( SUCCESS_E == error )
                  {
                     CO_PRINT_B_1(DBG_ERROR_E,"On Timeout : Discarded %s from the TxBuffer Queue\r\n", ParamCommandMap[pCommand->CommandType]);
                  }
                  else 
                  {
                     CO_PRINT_B_2(DBG_ERROR_E,"On Timeout : Failed to remove %s from the TxBuffer Queue. Error=%d\r\n", ParamCommandMap[pCommand->CommandType], (int32_t)error);
                  }            
               }
               else 
               {
                  CO_PRINT_B_0(DBG_INFO_E,"TxBuffer waiting for response\r\n");
               }
            }
            else
            {
               // Wrong signal type.  shouldn't be in this queue
               CO_PRINT_B_1(DBG_ERROR_E,"Wrong message type found in TxBuffer command queue. Type=5d\r\n",message.MessageType);
               error = MM_CIEQ_Discard(CIE_Q_TX_BUFFER_E);
               if ( SUCCESS_E == error )
               {
                  CO_PRINT_B_0(DBG_ERROR_E,"Discarded message from the TxBuffer Queue\r\n");
               }
               else 
               {
                  CO_PRINT_B_1(DBG_ERROR_E,"Failed to remove message from the TxBuffer Queue. Error=%d\r\n", (int32_t)error);
               }
            }
         }
         else 
         {
            CO_PRINT_B_1(DBG_ERROR_E,"Failed to pop the TxBuffer Queue - Error=%d\r\n", error);
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationProcessOutputSignalQueue
* Function to check whether the next output signal command should be sent to the mesh.
*
* @param None.
*
* @return - void
*/
void MM_NCUApplicationProcessOutputSignalQueue(void)
{
   bool new_message = false;
   CieBuffer_t message;
   CO_OutputData_t* pOutData = NULL;
   uint32_t number_to_send = 1;
   
   /* Get next message */
   if ( MM_CIEQ_GetNextOutputSignal(&message, &new_message) )
   {
      //send the message if it is new
      if ( new_message )
      {
         if ( APP_MSG_TYPE_OUTPUT_SIGNAL_E == message.MessageType )
         {
            pOutData = (CO_OutputData_t*)message.Buffer;

            if ( MM_MeshAPIOutputSignalReq( pOutData->Handle, pOutData->Destination, pOutData->zone, pOutData->OutputChannel, pOutData->OutputProfile, pOutData->OutputsActivated, pOutData->OutputDuration, number_to_send ) )
            {
               CO_PRINT_B_1(DBG_INFO_E,"Sending OUTPUT SIGNAL - Handle=%d\r\n", pOutData->Handle);
            }
            else 
            {
               ErrorCode_t error = MM_CIEQ_Discard(CIE_Q_OUTPUT_SIGNAL_E);
               if ( SUCCESS_E == error )
               {
                  CO_PRINT_B_0(DBG_ERROR_E,"On Send : Discarded OUTPUT SIGNAL\r\n");
               }
               else 
               {
                  CO_PRINT_B_1(DBG_ERROR_E,"On Send : Failed to remove OUTPUT SIGNAL from the TxBuffer Queue. Error=%d\r\n", (int32_t)error);
               }
            }
         }
         else if ( APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E == message.MessageType )
         {
            CO_AlarmOutputStateData_t* pOutData = (CO_AlarmOutputStateData_t*)message.Buffer;

            if ( MM_MeshAPIAlarmOutputSignalReq( pOutData->Handle, pOutData ) )
            {
               CO_PRINT_B_1(DBG_INFO_E,"Sending ALARM OUTPUT STATE - Handle=%d\r\n", pOutData->Handle);
            }
            else 
            {
               ErrorCode_t error = MM_CIEQ_Discard(CIE_Q_OUTPUT_SIGNAL_E);
               if ( SUCCESS_E == error )
               {
                  CO_PRINT_B_0(DBG_ERROR_E,"On Send : Discarded ALARM OUTPUT STATE\r\n");
               }
               else 
               {
                  CO_PRINT_B_1(DBG_ERROR_E,"On Send : Failed to remove ALARM OUTPUT STATE from the TxBuffer Queue. Error=%d\r\n", (int32_t)error);
               }
            }
         }
         else if ( APP_MSG_TYPE_RESET_E == message.MessageType )
         {
            uint8_t number_to_send = CFG_GetNumberOfResends();
            CO_CommandData_t* pCommandData = (CO_CommandData_t*)message.Buffer;
            
            if ( MM_MeshAPICommandReq(pCommandData->TransactionID, pCommandData->Source, pCommandData->Destination, pCommandData->CommandType,
                           pCommandData->Parameter1, pCommandData->Parameter2, pCommandData->Value, pCommandData->ReadWrite, number_to_send) )
            {
               CO_PRINT_B_1(DBG_INFO_E,"Sending RESET command - Handle=%d\r\n", pCommandData->TransactionID);
            }
            else 
            {
               CO_PRINT_B_1(DBG_ERROR_E,"FAILED to send RESET command - Handle=%d\r\n", pCommandData->TransactionID);
            }
            
            ErrorCode_t error = MM_CIEQ_Discard(CIE_Q_OUTPUT_SIGNAL_E);
            if ( SUCCESS_E != error )
            {
               CO_PRINT_B_1(DBG_ERROR_E,"On Send : Failed to remove RESET command from the TxBuffer Queue. Error=%d\r\n", (int32_t)error);
            }
         }
      }
      else 
      {
         // the message has already been sent.  Check for timeout.
         if ( MM_CIEQ_OutputSignalTimedOut() ) /* msg was already sent.  check for time-out */
         {
            /* the message has timed out.  Report an error to the CIE via the Misc Queue and discard it*/
            CieBuffer_t cieMsg;
            snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "OUT: ERROR,%s\r\n", CIE_ERROR_CODE_STR[AT_CIE_ERROR_NETWORK_TIMEOUT_E]);
            MM_CIEQ_Push(CIE_Q_MISC_E, &cieMsg);
            
            ErrorCode_t error = MM_CIEQ_Discard(CIE_Q_OUTPUT_SIGNAL_E);
            if ( SUCCESS_E == error )
            {
               CO_PRINT_B_0(DBG_ERROR_E,"On Timeout : Discarded OUTPUT SIGNAL from the TxBuffer Queue\r\n");
            }
            else 
            {
               CO_PRINT_B_1(DBG_ERROR_E,"On Timeout : Failed to remove OUTPUT SIGNAL from the TxBuffer Queue. Error=%d\r\n", (int32_t)error);
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationProcessBatteryStatusIndication
* Function for processing a received battery status network message.
*
* @param pMessage   The contents of the message.
*
* @return - void
*/
void MM_NCUApplicationProcessBatteryStatusIndication(const ApplicationMessage_t* pMessage)
{
   if ( pMessage )
   {
      if ( APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E == pMessage->MessageType )
      {
         BatteryStatusIndication_t* pBatteryStatus = (BatteryStatusIndication_t*)pMessage->MessageBuffer;
         if ( pBatteryStatus )
         {
            CO_PRINT_B_3(DBG_INFO_E,"NCU Rx'd Battery Status from NodeID %d, Primary=%dmV, Backup=%dmV\r\n", 
            pBatteryStatus->SourceAddress,pBatteryStatus->PrimaryBatteryVoltage, pBatteryStatus->BackupBatteryVoltage);
            
            //Update the child record with the reported voltages
            if ( MAX_DEVICES_PER_SYSTEM > pBatteryStatus->SourceAddress )
            {
               MM_NI_UpdateBatteryStatus(pBatteryStatus);
               //Update the control panel
               CieBuffer_t statusMsg = {0};
               snprintf((char*)statusMsg.Buffer, CIE_QUEUE_DATA_SIZE, "MSC:Z%dU%d,%d,%d,%d,%d,%d,%d,%d",
                  pBatteryStatus->ZoneNumber, pBatteryStatus->SourceAddress,
                  pBatteryStatus->PrimaryBatteryVoltage,
                  pBatteryStatus->BackupBatteryVoltage,
                  pBatteryStatus->DeviceCombination,
                  pBatteryStatus->ZoneNumber,
                  pBatteryStatus->SmokeAnalogueValue,
                  pBatteryStatus->HeatAnalogueValue,
                  pBatteryStatus->PirAnalogueValue );
                  
                  /*push the message into the CIE misc queue */
                  ErrorCode_t error = MM_CIEQ_Push(CIE_Q_MISC_E, &statusMsg);
                  if ( error )
                  {
                     CO_PRINT_A_1(DBG_ERROR_E,"Failed to add MSC Status to CIE misc queue.  Error=%d\r\n", error);
                  }
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationSendNextMeshMessage
* Schedules the next message to be sent over the mesh from the TxBuffer queue.
*
* @param - void
*
* @return - void
*/
void MM_NCUApplicationSendNextMeshMessage(void)
{
   CO_Message_t* pMessage = osPoolAlloc(AppPool);
   if ( pMessage )
   {
      pMessage->Type = CO_MESSAGE_SERVICE_TX_BUFFER_QUEUE_E;
      
      osStatus osStat = osMessagePut(AppQ, (uint32_t)pMessage, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         osPoolFree(AppPool, pMessage);
      }

   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationProcessNcuCacheRequest
* handles CIE request to send a status report for a device(s) from the NCU cache to the CIE queue.
*
* @param - void
*
* @return - void
*/
void MM_NCUApplicationProcessNcuCacheRequest(void)
{
   bool done = false;
   bool valid_node = false;
   NeighbourInformation_t neighbour;
   uint16_t max_node = MC_MaxNumberOfNodes();
   
   uint32_t message_count = MM_CIEQ_MessageCount(CIE_Q_MISC_E);
   if ( message_count >= CIE_Q_MAX_QUEUE_ENTRIES_FOR_CACHE_STATUS_REQUEST ) 
   {
      done = true;
   }

   
   if ( gReportStatusFromNcuCache && !done)
   {
      while ( !done && (gReportStatusNodeIndex < max_node) )
      {
         if ( ADDRESS_GLOBAL != gReportStatusRequestedStatusNode )
         {
            gReportStatusNodeIndex = gReportStatusRequestedStatusNode;
         }
         
         valid_node = MM_NI_GetNeighbourInformation( gReportStatusNodeIndex, &neighbour );
         
         if ( valid_node )
         {
            //Have we heard from this node?
            if ( CO_BAD_NODE_ID != neighbour.NodeID )
            {
               //Found a cache entry.  Send it to the CIE.
               CieBuffer_t statusMsg = {0};
               snprintf((char*)statusMsg.Buffer, CIE_QUEUE_DATA_SIZE, "MSR:Z%dU%d,%d,%d,9,%d,%d,%d,%d,0,0,%d,%d,%d",
                  neighbourArray[gReportStatusNodeIndex].ZoneNumber, gReportStatusNodeIndex,
                  neighbour.primaryBattery_mv, neighbour.backupBattery_mv,
                  neighbour.PrimaryParentID, neighbour.SecondaryParentID, neighbour.PrimaryParentRssi, neighbour.SecondaryParentRssi,
                  neighbour.Rank,
                  neighbour.DeviceCombination,
                  neighbour.FaultStatus
               );
               
               /*push the message into the CIE misc queue */
               ErrorCode_t error = MM_CIEQ_Push(CIE_Q_MISC_E, &statusMsg);
               if ( SUCCESS_E != error )
               {
                  CO_PRINT_A_1(DBG_ERROR_E,"Failed to add Mesh Status for to CIE misc queue.  Error=%d\r\n", error);
               }
               else 
               {
                  CO_PRINT_B_1(DBG_INFO_E,"<<%s\r\n", statusMsg.Buffer);
               }
            }
         }
         
         if ( ADDRESS_GLOBAL == gReportStatusRequestedStatusNode )
         {
            gReportStatusNodeIndex++;
            if ( gReportStatusNodeIndex >= max_node )
            {
               done = true;
               gReportStatusFromNcuCache = false;//Stop reporting
            }
            else 
            {
               //Don't over-fill the CIE Queue
               message_count = MM_CIEQ_MessageCount(CIE_Q_MISC_E);
               if ( message_count > (CIE_MISC_Q_SIZE / 2) ) 
               {
                  done = true;
               }
            }
         }
         else
         {
            //Force break from while loop
            done = true;
            gReportStatusFromNcuCache = false;//Stop reporting
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_NCUApplicationProcessAtCommandMessage
* Function to receive AT Commands from the Mesh.
*
* @param pAppMessage   The contents of the message.
*
* @return - void
*/
void MM_NCUApplicationProcessAtCommandMessage(const ApplicationMessage_t* pAppMessage)
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
