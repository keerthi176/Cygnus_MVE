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
*  File         : MM_MeshAPI.c
*
*  Description  : Mesh Protocol API functions
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <string.h>

/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "MM_MeshAPI.h"
#include "CO_Message.h"
#include "SM_StateMachine.h"
#include "MC_PUP.h"
#include "DM_NVM.h"
#include "MC_SessionManagement.h"
#include "DM_SerialPort.h"
#include "CFG_Device_cfg.h"

/* Private Functions Prototypes
*************************************************************************************/



/* Global Variables
*************************************************************************************/
extern osPoolId MeshPool;
extern osMessageQId(MeshQ);
extern osPoolId AppPool;
extern osMessageQId(AppQ);


/* Private Variables
*************************************************************************************/


/*************************************************************************************/
/**
* MM_MeshAPIFireSignalReq
* Function to send a fire signal message to the NCU.
*
* @param - Handle          A tag to identify this request.  Should be quoted in future callbacks.
* @param - RUChannelIndex  An identifier for the sensor head that triggered the fire signal.
* @param - SensorValue     The analogue value recorded by the senor.
*
* @return - void
*/
bool MM_MeshAPIFireSignalReq( const uint32_t Handle, const CO_RBUSensorData_t* pSensorData )
{
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t *pFireSignalReq = NULL;
   CO_RBUSensorData_t sensorData;

   if ( pSensorData )
   {
      /* create FireSignalReq message and put into mesh queue */
      pFireSignalReq = ALLOCMESHPOOL;
      if (pFireSignalReq)
      {
         sensorData.Handle = Handle;
         sensorData.SensorType = pSensorData->SensorType;
         sensorData.RUChannelIndex = pSensorData->RUChannelIndex;
         sensorData.AlarmState = pSensorData->AlarmState;
         sensorData.SensorValue = pSensorData->SensorValue;
         sensorData.HopCount = 0;
         sensorData.Zone = pSensorData->Zone;
         pFireSignalReq->Type = CO_MESSAGE_GENERATE_FIRE_SIGNAL_E;
         memcpy(pFireSignalReq->Payload.PhyDataReq.Data, &sensorData, sizeof(CO_RBUSensorData_t));
         osStat = osMessagePut(MeshQ, (uint32_t)pFireSignalReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            FREEMESHPOOL(pFireSignalReq);
         }
         else
         {
            result = true;
         }
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* MM_MeshAPIAlarmSignalReq
* Function to send a alarm signal message to the NCU.
*
* @param - Handle          A tag to identify this request.  Should be quoted in future callbacks.
* @param - RUChannelIndex  An identifier for the sensor head that triggered the fire signal.
* @param - SensorValue     The analogue value recorded by the senor.
*
* @return - void
*/
bool MM_MeshAPIAlarmSignalReq( const uint32_t Handle, const CO_RBUSensorData_t* pSensorData )
{
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t *pAlarmSignalReq = NULL;
   CO_RBUSensorData_t sensorData;

   if ( pSensorData )
   {
      /* create AlarmSignalReq message and put into mesh queue */
      pAlarmSignalReq = ALLOCMESHPOOL;
      if (pAlarmSignalReq)
      {
         sensorData.Handle = Handle;
         sensorData.SensorType = pSensorData->SensorType;
         sensorData.RUChannelIndex = pSensorData->RUChannelIndex;
         sensorData.AlarmState = pSensorData->AlarmState;
         sensorData.SensorValue = pSensorData->SensorValue;
         sensorData.HopCount = 0;
         sensorData.Zone = pSensorData->Zone;
         pAlarmSignalReq->Type = CO_MESSAGE_GENERATE_ALARM_SIGNAL_E;
         memcpy(pAlarmSignalReq->Payload.PhyDataReq.Data, &sensorData, sizeof(CO_RBUSensorData_t));
         osStat = osMessagePut(MeshQ, (uint32_t)pAlarmSignalReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            FREEMESHPOOL(pAlarmSignalReq);
         }
         else
         {
            result = true;
         }
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* MM_MeshAPIOutputSignalReq
* Function to send an output signal message.
*
* @param - Handle             A tag to identify this request.  Should be quoted in future callbacks.
* @param - Destination        The network node address of the target unit.
* @param - Zone               The zone number in which the outputs should be activated.
* @param - OutputChannel      The output channel number.
* @param - OutputProfile      An identifier for the output type (fire/first aid/PIR/Test/Silent Test).
* @param - OutputsActivated   The outputs that are to be activated/deactivated.
* @param - OutputDuration     The duration that the output should be activated/deactivated.
* @param - NumberToSend       The number of messages to send consecutively.
*
* @return - bool              True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPIOutputSignalReq( const uint32_t Handle, const uint16_t Destination, const uint16_t zone, const uint8_t OutputChannel,
                                 const uint8_t OutputProfile, const uint16_t OutputsActivated, const uint8_t OutputDuration, const uint8_t NumberToSend )
{
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t *pAlarmSignalReq = NULL;
   CO_OutputData_t outputData;

   /* create OutputSignalReq message and put into mesh queue */
   pAlarmSignalReq = ALLOCMESHPOOL;
   if (pAlarmSignalReq)
   {
      outputData.Handle = Handle;
      outputData.Destination = Destination;
      outputData.zone = zone;
      outputData.OutputChannel = OutputChannel;
      outputData.OutputProfile = OutputProfile;
      outputData.OutputsActivated = OutputsActivated;
      outputData.OutputDuration = OutputDuration;
      outputData.NumberToSend = NumberToSend;
      pAlarmSignalReq->Type = CO_MESSAGE_GENERATE_OUTPUT_SIGNAL_E;
      memcpy(pAlarmSignalReq->Payload.PhyDataReq.Data, &outputData, sizeof(CO_OutputData_t));
      osStat = osMessagePut(MeshQ, (uint32_t)pAlarmSignalReq, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         FREEMESHPOOL(pAlarmSignalReq);
      }
      else
      {
         result = true;
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* MM_MeshAPIAlarmOutputSignalReq
* Function to send an Alarm Output Signal (AOS) message.
*
* @param - Handle             A tag to identify this request.  Should be quoted in future callbacks.
* @param - pAlarmOutputState  Pointer to alarm data structure.
*
* @return - bool              True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPIAlarmOutputSignalReq( const uint32_t Handle, const CO_AlarmOutputStateData_t *pAlarmOutputState )
{
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t *pAlarmSignalReq = NULL;

   if ( pAlarmOutputState )
   {
      /* create OutputSignalReq message and put into mesh queue */
      pAlarmSignalReq = ALLOCMESHPOOL;
      if (pAlarmSignalReq)
      {
         CO_AlarmOutputStateData_t stateData;
         stateData.Handle = pAlarmOutputState->Handle;
         stateData.Source = pAlarmOutputState->Source;
         stateData.Silenceable = pAlarmOutputState->Silenceable;
         stateData.Unsilenceable = pAlarmOutputState->Unsilenceable;
         stateData.DelayMask = pAlarmOutputState->DelayMask;
         stateData.NumberToSend = 1;
         pAlarmSignalReq->Type = CO_MESSAGE_GENERATE_ALARM_OUTPUT_STATE_E;
         
         memcpy(pAlarmSignalReq->Payload.PhyDataReq.Data, &stateData, sizeof(CO_AlarmOutputStateData_t));
         osStat = osMessagePut(MeshQ, (uint32_t)pAlarmSignalReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            FREEMESHPOOL(pAlarmSignalReq);
         }
         else
         {
            result = true;
         }
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* MM_MeshAPILogonReq
* Function to send a logon request message.
*
* @param - Handle             A tag to identify this request.  Should be quoted in future callbacks.
* @param - SerialNumber       The serial number of the local unit.
* @param - DeviceCombination  Identifies which sensor heads and call points are fitted.
* @param - ZoneNumber         The programmed zone for the local unit.
*
* @return - bool              True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPILogonReq( const uint32_t Handle,const uint32_t SerialNumber, const uint8_t DeviceCombination, const uint16_t ZoneNumber )
{
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t *pLogonReq = NULL;
   CO_LogonData_t logonData;

   /* create LogonReq message and put into mesh queue */
   pLogonReq = ALLOCMESHPOOL;
   if (pLogonReq)
   {
      logonData.Handle = Handle;
      logonData.SerialNumber = SerialNumber;
      logonData.DeviceCombination = DeviceCombination;
      logonData.ZoneNumber = ZoneNumber;
      pLogonReq->Type = CO_MESSAGE_GENERATE_LOGON_REQ;
      memcpy(pLogonReq->Payload.PhyDataReq.Data, &logonData, sizeof(CO_LogonData_t));
      osStat = osMessagePut(MeshQ, (uint32_t)pLogonReq, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         FREEMESHPOOL(pLogonReq);
      }
      else
      {
         result = true;
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* MM_MeshAPIResponseReq
* Function to send a response message.
*
* @param - Handle             A tag to identify this request.  Should be quoted in future callbacks.
* @param - Source             The source node for the response.
* @param - Destination        The destination node for the response.
* @param - CommandType        The parameter that the command reads/writes.
* @param - Parameter1         P1, as defined in the Parameters section of HKD-17-0142-D_A3 Mesh Protocol API Description
* @param - Parameter2         P2, as defined in the Parameters section of HKD-17-0142-D_A3 Mesh Protocol API Description
* @param - Value              Value that is read or written.
* @param - ReadWrite          Identifies a read or write action for the command.
*
* @return - bool              True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPIResponseReq( const uint32_t Handle, const uint16_t Source, const uint16_t Destination, const uint8_t CommandType,
                           const uint8_t Parameter1, const uint8_t Parameter2, const uint32_t Value, const uint8_t ReadWrite )
{
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t *pResponseReq = NULL;
   CO_ResponseData_t response;

   response.TransactionID = Handle;
   response.Source = Source;
   response.Destination = Destination;
   response.CommandType = CommandType;
   response.Parameter1 = Parameter1;
   response.Parameter2 = Parameter2;
   response.Value = Value;
   response.ReadWrite = ReadWrite;
   
   /* create response message and put into mesh queue */
   pResponseReq = ALLOCMESHPOOL;
   if (pResponseReq)
   {
      pResponseReq->Type = CO_MESSAGE_GENERATE_RESPONSE_SIGNAL_E;
      memcpy(pResponseReq->Payload.PhyDataReq.Data, &response, sizeof(CO_ResponseData_t));
      osStat = osMessagePut(MeshQ, (uint32_t)pResponseReq, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         FREEMESHPOOL(pResponseReq);
      }
      else
      {
         result = true;
      }
   }

   
   return result;
}

/*************************************************************************************/
/**
* MM_MeshAPICommandReq
* Function to send a Command message.
*
* @param - Handle             A tag to identify this request.  Should be quoted in future callbacks.
* @param - Source             The source node for the command.
* @param - Destination        The destination node for the command.
* @param - CommandType        The parameter that the command reads/writes.
* @param - Parameter1         P1, as defined in the Parameters section of HKD-17-0142-D_A3 Mesh Protocol API Description
* @param - Parameter2         P2, as defined in the Parameters section of HKD-17-0142-D_A3 Mesh Protocol API Description
* @param - Value              Value that is read or written.
* @param - ReadWrite          Identifies a read or write action for the command.
*
* @return - bool              True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPICommandReq( const uint32_t Handle, const uint16_t Source, const uint16_t Destination, const uint8_t CommandType,
                           const uint8_t Parameter1, const uint8_t Parameter2, const uint32_t Value, const uint8_t ReadWrite, const uint8_t NumberToSend )
{
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdReq = NULL;
   CO_CommandData_t command;
   
   command.TransactionID = Handle;
   command.Source = Source;
   command.Destination = Destination;
   command.CommandType = CommandType;
   command.Parameter1 = Parameter1;
   command.Parameter2 = Parameter2;
   command.Value = Value;
   command.ReadWrite = ReadWrite;
   command.NumberToSend = NumberToSend;

   /* create cmd message and put into mesh queue */
   pCmdReq = ALLOCMESHPOOL;
   if (pCmdReq)
   {
      pCmdReq->Type = CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E;
      memcpy(pCmdReq->Payload.PhyDataReq.Data, &command, sizeof(CO_CommandData_t));
      osStat = osMessagePut(MeshQ, (uint32_t)pCmdReq, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         FREEMESHPOOL(pCmdReq);
      }
      else
      {
         result = true;
      }
   }


   return result;
}

/*************************************************************************************/
/**
* MM_MeshAPIGenerateTestMessageReq
* Function to send a test message.
*
* @param - text      The text component of the test message.
* @param - count     The counter component of the test message.
*
* @return - bool     True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPIGenerateTestMessageReq( const char* const payload )
{
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdReq;
   
   if ( payload )
   {
      /* create cmd message and put into mesh queue */
      pCmdReq = ALLOCMESHPOOL;
      if (pCmdReq)
      {
         pCmdReq->Type = CO_MESSAGE_GENERATE_TEST_MESSAGE_E;
         memcpy(pCmdReq->Payload.PhyDataReq.Data, payload, TEST_PAYLOAD_SIZE);
         osStat = osMessagePut(MeshQ, (uint32_t)pCmdReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            FREEMESHPOOL(pCmdReq);
         }
         else
         {
            result = true;
         }
      }
   }

   return result;
}

/*************************************************************************************/
/**
* MM_MeshAPIGenerateSetStateReq
* Function to send a system State change message.
*
* @param - state  The commanded state value.
*
* @return - bool  True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPIGenerateSetStateReq( const CO_State_t state )
{
   bool result = false;
    osStatus osStat = osErrorOS;
   CO_Message_t *pCmdReq;
 
   /* create cmd message and put into mesh queue */
   pCmdReq = ALLOCMESHPOOL;
   if (pCmdReq)
   {
      
      pCmdReq->Type = CO_MESSAGE_GENERATE_SET_STATE_E;
      pCmdReq->Payload.PhyDataReq.Data[0] = (uint8_t)state;
      pCmdReq->Payload.PhyDataReq.Size = 1;
      osStat = osMessagePut(MeshQ, (uint32_t)pCmdReq, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         FREEMESHPOOL(pCmdReq);
      }
      else
      {
         result = true;
      }
   }
  
   return result;
}

/*************************************************************************************/
/**
* MM_MeshAPIGenerateSetStateReq
* Function to send a system State change message.
*
* @param - state  The commanded state value.
*
* @return - bool  True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPIGenerateRBUDisableReq( const uint32_t Handle, const uint16_t unitAddress )
{
   bool result = false;
    osStatus osStat = osErrorOS;
   CO_Message_t *pCmdReq;
   CO_RBUDisableData_t rbuDisableData;
   
   rbuDisableData.Handle = Handle;
   rbuDisableData.UnitAddress = unitAddress;
   
   /* create message and put into mesh queue */
   pCmdReq = ALLOCMESHPOOL;
   if (pCmdReq)
   {
      pCmdReq->Type = CO_MESSAGE_GENERATE_RBU_DISABLE_MESSAGE_E;
      memcpy(pCmdReq->Payload.PhyDataReq.Data, &rbuDisableData, sizeof(CO_RBUDisableData_t));
      osStat = osMessagePut(MeshQ, (uint32_t)pCmdReq, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         FREEMESHPOOL(pCmdReq);
      }
      else
      {
         result = true;
      }
   }


   return result;
}

/*************************************************************************************/
/**
* MM_MeshAPIGenerateRBUReportNodesReq
* Send a command to the session manager to print a report of assiciated nodes 
* on the DEBUG UART.
*
* @param - source    The source usart that the response should be sent to.
*
* @return - bool     True if the message is queued, false otherwise
*/

bool MM_MeshAPIGenerateRBUReportNodesReq( const uint32_t source )
{
   bool result = false;

   CO_Message_t* pMsg;
   pMsg = ALLOCMESHPOOL;
   if (pMsg)
   {
      pMsg->Type = CO_MESSAGE_MAC_EVENT_E;
      MACEventMessage_t macEvent;
      macEvent.EventType = CO_REPORT_NODES_E;
      macEvent.Value = source;
      memcpy(pMsg->Payload.PhyDataInd.Data, &macEvent, sizeof(MACEventMessage_t));
      osStatus osStat = osMessagePut(MeshQ, (uint32_t)pMsg, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         FREEMESHPOOL(pMsg);
      }
      else
      {
         result = true;
      }
   }

   return result;   
}

/*************************************************************************************/
/**
* MM_MeshAPIGenerateStatusSignalReq
* Function to send the System Status message to the NCU.
*
* @param - Handle       A transaction ID generated by the application.
* @param - Event        The change in network status ( map to CO_MeshEvent_t ). 
* @param - EventNodeId  The node that the network event refers to. 
* @param - EventData    General purpose data field for the event. 
* @param - DelaySending True if responding to NCU global request. False for dirctly addressed request or internal status change.
*
* @return - bool  True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPIGenerateStatusSignalReq( const uint32_t Handle, const uint8_t Event, const uint16_t EventNodeId, const uint32_t EventData, const uint8_t OverallFaultStatus, const bool DelaySending )
{
   bool result = false;
    osStatus osStat = osErrorOS;
   CO_Message_t *pCmdReq;
   CO_StatusIndicationData_t statusData;
   
   statusData.Handle = Handle;
   statusData.Event = (CO_MeshEvent_t)Event;
   statusData.EventNodeId = EventNodeId;
   statusData.EventData = EventData;
   statusData.OverallFault = OverallFaultStatus;
   statusData.DelaySending = DelaySending;
   
   /* create message and put into mesh queue */
   pCmdReq = ALLOCMESHPOOL;
   if (pCmdReq)
   {
      pCmdReq->Type = CO_MESSAGE_GENERATE_STATUS_INDICATION_E;
      memcpy(pCmdReq->Payload.PhyDataReq.Data, &statusData, sizeof(CO_StatusIndicationData_t));
      osStat = osMessagePut(MeshQ, (uint32_t)pCmdReq, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         FREEMESHPOOL(pCmdReq);
      }
      else
      {
         result = true;
      }
   }


   return result;
}

/*************************************************************************************/
/**
* MM_MeshAPIGenerateFaultSignalReq
* Function to send the Fault Status message to the NCU.
*
* @param - Handle       A transaction ID generated by the application.
* @param - StatusFlags  The current status flagd for the RBU. 
* @param - NodeAddress  The Node ID of the relavent child (link stability fault only). 
*
* @return - bool  True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPIGenerateFaultSignalReq( const CO_FaultData_t* const faultData )
{
   bool result = false;
    osStatus osStat = osErrorOS;
   CO_Message_t *pCmdReq;
   CO_FaultData_t fault_data;
   
   if ( faultData )
   {
      fault_data.Handle = faultData->Handle;
      fault_data.RUChannelIndex = faultData->RUChannelIndex;
      fault_data.FaultType = faultData->FaultType;
      fault_data.Value = faultData->Value;
      fault_data.DelaySending = faultData->DelaySending;
      fault_data.OverallFault = faultData->OverallFault;
      
      /* create message and put into mesh queue */
      pCmdReq = ALLOCMESHPOOL;
      if (pCmdReq)
      {
         pCmdReq->Type = CO_MESSAGE_GENERATE_FAULT_SIGNAL_E;
         memcpy(pCmdReq->Payload.PhyDataReq.Data, &fault_data, sizeof(CO_FaultData_t));
         osStat = osMessagePut(MeshQ, (uint32_t)pCmdReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            FREEMESHPOOL(pCmdReq);
         }
         else
         {
            result = true;
         }
      }
   }
   return result;
}



/*************************************************************************************/
/**
* MM_MeshAPIGenerateBatteryStatusSignalReq
* Function to send the Battery Status message to the NCU.
*
* @param - batteryData  Pointer to structure containing battery status. 
*
* @return - bool  True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPIGenerateBatteryStatusSignalReq( const CO_BatteryStatusData_t* const batteryData )
{
   bool result = false;
    osStatus osStat = osErrorOS;
   CO_Message_t *pCmdReq;
   CO_BatteryStatusData_t batteryStatusData;
   
   if ( batteryData )
   {
      batteryStatusData.Handle = batteryData->Handle;
      batteryStatusData.PrimaryBatteryVoltage = batteryData->PrimaryBatteryVoltage;
      batteryStatusData.BackupBatteryVoltage = batteryData->BackupBatteryVoltage;
      batteryStatusData.DeviceCombination = batteryData->DeviceCombination;
      batteryStatusData.ZoneNumber = batteryData->ZoneNumber;
      batteryStatusData.SmokeAnalogueValue = batteryData->SmokeAnalogueValue;
      batteryStatusData.HeatAnalogueValue = batteryData->HeatAnalogueValue;
      batteryStatusData.PirAnalogueValue = batteryData->PirAnalogueValue;
      batteryStatusData.DelaySending = batteryData->DelaySending;
      
      CO_PRINT_B_3(DBG_INFO_E, "API Battery Status Signal.  smoke=%d heat=%d, pir=%d\r\n", batteryStatusData.SmokeAnalogueValue, batteryStatusData.HeatAnalogueValue, batteryStatusData.PirAnalogueValue);
      
      /* create message and put into mesh queue */
      pCmdReq = ALLOCMESHPOOL;
      if (pCmdReq)
      {
         pCmdReq->Type = CO_MESSAGE_GENERATE_BATTERY_STATUS_E;
         memcpy(pCmdReq->Payload.PhyDataReq.Data, &batteryStatusData, sizeof(CO_BatteryStatusData_t));
         osStat = osMessagePut(MeshQ, (uint32_t)pCmdReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            FREEMESHPOOL(pCmdReq);
         }
         else
         {
            result = true;
         }
      }
   }
   return result;
}




/*************************************************************************************/
/**
* MM_MeshAPIApplicationCommand
* Function for the application to update the mesh layer, or for requesting information from the mesh layer to be
* sent back to the application.
*
* @param MsgType    The message type that describes what information is being supplied to/requested from the mesh.
* @param value1     Data to be sent to the mesh.
* @param value2     Data to be sent to the mesh.
* @param source     The original source (e.g. debug uart) that the response should be directed to.
*
* @return - bool    True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPIApplicationCommand(const ApplicationLayerMessageType_t MsgType, const int32_t value1, const int32_t value2, const uint8_t source, const uint32_t handle)
{
   bool result = false;
    osStatus osStat = osErrorOS;
   CO_Message_t *pCmdReq;
   ApplicationMessage_t appMessage;
   int32_t* pValue;
   
   appMessage.MessageType = MsgType;
   appMessage.Source = source;
   pValue = (int32_t*)appMessage.MessageBuffer;
   *pValue = value1;
   pValue++;
   *pValue = value2;
   appMessage.Handle = handle;
   
   CO_PRINT_B_4(DBG_INFO_E,"MM_MeshAPIApplicationCommand MsgType=%d, value1=%d, value2=%d, source=%d\r\n", MsgType, value1, value2, source);
   
   /* create message and put into mesh queue */
   pCmdReq = ALLOCMESHPOOL;
   if (pCmdReq)
   {
      pCmdReq->Type = CO_MESSAGE_APPLICATION_REQUEST_E;
      memcpy(pCmdReq->Payload.PhyDataReq.Data, &appMessage, sizeof(ApplicationMessage_t));
      osStat = osMessagePut(MeshQ, (uint32_t)pCmdReq, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         FREEMESHPOOL(pCmdReq);
      }
      else
      {
         result = true;
      }
   }
   return result;
}

/*************************************************************************************/
/**
* MM_MeshAPIGenerateZoneEnableReq
* Function to send the Zone Enable message to the NCU.
*
* @param - zoneData  Pointer to structure containing the zone info. 
*
* @return - bool  True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPIGenerateZoneEnableReq( const ZoneEnableData_t* const zoneData )
{
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdReq;
   
   if ( zoneData )
   {
      /* create message and put into mesh queue */
      pCmdReq = ALLOCMESHPOOL;
      if (pCmdReq)
      {
         pCmdReq->Type = CO_MESSAGE_GENERATE_ZONE_ENABLE_E;
         memcpy(pCmdReq->Payload.PhyDataReq.Data, zoneData, sizeof(ZoneEnableData_t));
         osStat = osMessagePut(MeshQ, (uint32_t)pCmdReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            FREEMESHPOOL(pCmdReq);
         }
         else
         {
            result = true;
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* MM_MeshAPIGenerateAtMessage
* Function to send an AT command/respond for transmission over the air.
*
* @param - atMessage  Pointer to structure containing the zone info. 
*
* @return - bool  True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPIGenerateAtMessage( const AtMessageData_t* const atMessage )
{
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdReq;
   
   if ( atMessage )
   {
      /* create message and put into mesh queue */
      pCmdReq = ALLOCMESHPOOL;
      if (pCmdReq)
      {
         pCmdReq->Type = CO_MESSAGE_GENERATE_ATTX_COMMAND_E;
         memcpy(pCmdReq->Payload.PhyDataReq.Data, atMessage, sizeof(AtMessageData_t));
         osStat = osMessagePut(MeshQ, (uint32_t)pCmdReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            FREEMESHPOOL(pCmdReq);
         }
         else
         {
            result = true;
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* MM_MeshAPIGenerateStartAtModeMessage
* Function to send a 'start AT mode' over the air.
*
* @param - atMessage  Pointer to structure containing the command. 
*
* @return - bool  True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPIGenerateStartAtModeMessage( const OtaMode_t* const atMessage )
{
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdReq;
   
   if ( atMessage )
   {
      /* create message and put into mesh queue */
      pCmdReq = ALLOCMESHPOOL;
      if (pCmdReq)
      {
         pCmdReq->Type = CO_MESSAGE_GENERATE_START_OTA_AT_MODE_E;
         memcpy(pCmdReq->Payload.PhyDataReq.Data, atMessage, sizeof(OtaMode_t));
         osStat = osMessagePut(MeshQ, (uint32_t)pCmdReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            FREEMESHPOOL(pCmdReq);
         }
         else
         {
            result = true;
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* MM_MeshAPIGeneratePingRequest
* Function to send a ping over the air.
*
* @param - pingRequest  Pointer to structure containing the ping request. 
*
* @return - bool  True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPIGeneratePingRequest( const PingRequest_t* const pingRequest )
{
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t *pPingReq;
   
   if ( pingRequest )
   {
      /* create cmd message and put into mesh queue */
      pPingReq = osPoolAlloc(MeshPool);
      if (pPingReq)
      {
         pPingReq->Type = CO_MESSAGE_GENERATE_PING_REQUEST_E;
         memcpy(pPingReq->Payload.PhyDataReq.Data, pingRequest, sizeof(PingRequest_t));
         osStat = osMessagePut(MeshQ, (uint32_t)pPingReq, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(MeshPool, pPingReq);
         }
      }
   }
   return result;
}


/*************************************************************************************/
/**
* MM_MeshAPIGeneratePpuMessage
* Function to send a PPU message over the air.
*
* @param - pPpuMessage  Pointer to structure containing the PPU Message. 
*
* @return - bool  True if the message is queued for the Mesh, false otherwise
*/
bool MM_MeshAPIGeneratePpuMessage(const PPU_Message_t* const pPpuMessage)
{
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t *pMessage;
   
   CO_PRINT_B_0(DBG_INFO_E,"MM_MeshAPIGeneratePpuMessage+\r\n");
   
   if ( pPpuMessage )
   {
      /* create cmd message and put into mesh queue */
      pMessage = osPoolCAlloc(MeshPool);
      if (pMessage)
      {
         pMessage->Type = CO_MESSAGE_PPU_RESPONSE_E;
         memcpy(pMessage->Payload.PhyDataReq.Data, pPpuMessage, sizeof(PPU_Message_t));
         osStat = osMessagePut(MeshQ, (uint32_t)pMessage, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(MeshPool, pMessage);
         }
         else 
         {
            result = true;
            CO_PRINT_B_0(DBG_INFO_E,"Sent ppu msg to mesh\r\n");
         }
      }
   }
   return result;
}
