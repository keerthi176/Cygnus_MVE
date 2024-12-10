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
*  File         : STTestModeTest.c
*
*  Description  : Implementation for testing MC_TestMode.c
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdarg.h>


/* User Include Files
*************************************************************************************/
#include "lptim.h"
#include "CUnit.h"
#include "CO_Defines.h"
#include "CO_Message.h"
#include "CO_ErrorCode.h"
#include "MessageQueueStub.h"
#include "MC_PUP.h"
#include "MC_MAC.h"
#include "MC_TestMode.h"


/* Private Functions Prototypes
*************************************************************************************/
static void TM_rxHeartbeatTest(void);
static void TM_rxFireSignalTest(void);
static void TM_rxAlarmSignalTest(void);
static void TM_rxOutputSignalTest(void);
static void TM_rxAckSignalTest(void);
static void TM_rxRouteAddTest(void);
static void TM_rxRouteDropTest(void);
static void TM_rxFaultSignalTest(void);
static void TM_rxLogonRequestTest(void);
static void TM_rxCommandTest(void);
static void TM_rxResponseTest(void);
static void TM_rxTestSignalTest(void);
static void TM_rxTestReportTest(void);
static void TM_generateTestMessageTest(void);
static void TM_rxSetStateTest(void);
static void TM_NetworkMonitorTest(void);

/* Externs
*************************************************************************************/
extern uint32_t gNetworkAddress;
extern dm_serial_data_buffer_t dm_serial_data_buffer;
extern const SerialChannelDefinition_t SerialChannelDefinition[MAX_SUPPORTED_SERIAL_PORT];


/* Table containing the test settings */
CU_TestInfo ST_TestModeTests[] =
{
   { "rxHeartbeatTest",                  TM_rxHeartbeatTest },
   { "rxFireSignalTest",                 TM_rxFireSignalTest },
   { "rxAlarmSignalTest",                TM_rxAlarmSignalTest },
   { "rxOutputSignalTest",               TM_rxOutputSignalTest },
   { "rxAckSignalTest",                  TM_rxAckSignalTest },
   { "rxRouteAddTest",                   TM_rxRouteAddTest },
   { "rxRouteDropTest",                  TM_rxRouteDropTest },
   { "rxFaultSignalTest",                TM_rxFaultSignalTest },
   { "rxLogonRequestTest",               TM_rxLogonRequestTest },
   { "rxCommandTest",                    TM_rxCommandTest },
   { "rxResponseTest",                   TM_rxResponseTest },
   { "rxTestSignalTest",                 TM_rxTestSignalTest },
   { "rxTestReportTest",                 TM_rxTestReportTest },
   { "generateTestMessageTest",          TM_generateTestMessageTest },
   { "rxSetStateTest",                   TM_rxSetStateTest },
   { "NetworkMonitorTest",               TM_NetworkMonitorTest },

   CU_TEST_INFO_NULL,
};

/* Test utilities
*************************************************************************************/

#define SYSTEM_ID 0x12345678
#define WRONG_SYSTEM_ID 0x87654321

bool gUartBuffEmptyFlag = true;

/*************************************************************************************
* STUBBED FUNCTIONS
*************************************************************************************/



/*************************************************************************************
* TEST FUNCTIONS
*************************************************************************************/

/*************************************************************************************
* TM_rxHeartbeatTest
* Test the operation of function TM_rxHeartbeat;
*
* no params
*
* @return - none
*
*************************************************************************************/
void TM_rxHeartbeatTest(void)
{
   FrameHeartbeat_t heartbeat;
   CO_Message_t testMessage;

   TM_Initialise(SYSTEM_ID, 0);

   /* create a test heartbeat structure */
   heartbeat.FrameType = FRAME_TYPE_HEARTBEAT_E;
   heartbeat.SlotIndex = 17;
   heartbeat.ShortFrameIndex = 0;
   heartbeat.LongFrameIndex = 1;
   heartbeat.State = 3;
   heartbeat.Rank = 1;
   heartbeat.NoOfChildrenIdx = 5;
   heartbeat.NoOfChildrenOfPTNIdx = 5;
   heartbeat.SystemId = SYSTEM_ID;
   ErrorCode_t status = MC_PUP_PackHeartbeat(&heartbeat, &testMessage.Payload.PhyDataReq );
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 17;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 17;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 17;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1234;
   testMessage.Payload.PhyDataInd.Size = 11;

   /* reset the stats for calls to the serial port */
   SerialPortDataInit(DEBUG_UART);
   /* for SerialPortDebugPrint */
   bSerialDebugPrintToConsole = false;
   bSerialDebugPrintCalled = false;
   SerialDebugPrintCount = 0;
   SerialDebugLastPrint[0] = 0;
   /* Ignore in test mode OFF check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
   TM_rxHeartbeat((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore in transparent mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSPARENT_E);
   TM_rxHeartbeat((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore NULL parameter check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxHeartbeat(NULL);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Check output in monitoring mode */
   //                             <-- Serial port only --> <------------ Heartbeat Payload ----------->
   uint8_t expectedOutput[16] = { 0x12,0x11,0x00,0x0a,0x07,0x02,0x00,0x09,0x82,0xaa,0x24,0x68,0xac,0xf0 };
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_MONITORING_E);
   TM_rxHeartbeat((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 16);
   int32_t diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutput, 14);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Check output in receive mode */
   uint8_t expectedOutputString[256];
   //                                                         <----Heartbeat Payload--->
   sprintf(expectedOutputString, "Rx HEARTBEAT      1 OK   10 :   7 : 02 00 09 82 AA 24 68 AC F0 \r\n");
   int32_t expectedLength = strlen(expectedOutputString);
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxHeartbeat((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, expectedLength);
   diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutputString, expectedLength);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
}

/*************************************************************************************
* TM_rxFireSignalTest
* Test the operation of function TM_rxFireSignal;
*
* no params
*
* @return - none
*
*************************************************************************************/
void TM_rxFireSignalTest(void)
{
   AlarmSignal_t firesignal = { 0 };
   CO_Message_t testMessage;

   /* create a test fire signal */
   firesignal.Header.FrameType = FRAME_TYPE_DATA_E;
   firesignal.Header.MACDestinationAddress = 1;
   firesignal.Header.MACSourceAddress = 2;
   firesignal.Header.HopCount = 0;
   firesignal.Header.DestinationAddress = 1;
   firesignal.Header.SourceAddress = 2;
   firesignal.Header.MessageType = APP_MSG_TYPE_FIRE_SIGNAL_E;
   firesignal.RUChannel = 1;
   firesignal.SensorValue = 0x5a;
   firesignal.Zone = 1;
   firesignal.SystemId = SYSTEM_ID;
   

   ErrorCode_t status = MC_PUP_PackAlarmSignal(&firesignal, &testMessage.Payload.PhyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 12;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1234;
   testMessage.Payload.PhyDataInd.Size = 20;

   /* reset the stats for calls to the serial port */
   SerialPortDataInit(DEBUG_UART);
   /* for SerialPortDebugPrint */
   bSerialDebugPrintToConsole = false;
   bSerialDebugPrintCalled = false;
   SerialDebugPrintCount = 0;
   SerialDebugLastPrint[0] = 0;

   /* Ignore in test mode OFF check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
   TM_rxFireSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);

   /* Ignore in transparent mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSPARENT_E);
   TM_rxFireSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore NULL parameter check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxFireSignal(NULL);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Check output in monitoring mode */
   uint8_t expectedOutput[25] = { 0x00,0x02,0x00,0x0a,0x07,0x10,0x01,0x00,0x20,0x00,0x01,0x00,0x20,0x05,0x68,0x00,0x04,0x00,0x00,0x00,0x01,0x23,0x45,0x67,0x80 };
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_MONITORING_E);
   TM_rxFireSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 25);
   int32_t diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutput, 25);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Check output in receive mode */
   uint8_t expectedOutputString[256];
   sprintf(expectedOutputString, "Rx FIRESIG        2 OK   10 :   7 : 10 01 00 20 00 01 00 20 05 68 00 04 00 00 00 01 23 45 67 80 \r\n");
   int32_t expectedLength = strlen(expectedOutputString);
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxFireSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, expectedLength);
   diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutputString, expectedLength);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
}

/*************************************************************************************
* TM_rxAlarmSignalTest
* Test the operation of function TM_rxAlarmSignal;
*
* no params
*
* @return - none
*
*************************************************************************************/
void TM_rxAlarmSignalTest(void)
{
   AlarmSignal_t alarmSignal = { 0 };
   CO_Message_t testMessage;

   /* create a test fire signal */
   alarmSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   alarmSignal.Header.MACDestinationAddress = 1;
   alarmSignal.Header.MACSourceAddress = 2;
   alarmSignal.Header.HopCount = 0;
   alarmSignal.Header.DestinationAddress = 1;
   alarmSignal.Header.SourceAddress = 2;
   alarmSignal.Header.MessageType = APP_MSG_TYPE_ALARM_SIGNAL_E;
   alarmSignal.RUChannel = 1;
   alarmSignal.SensorValue = 0x5a;
   alarmSignal.SystemId = SYSTEM_ID;

   ErrorCode_t status = MC_PUP_PackAlarmSignal(&alarmSignal, &testMessage.Payload.PhyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 12;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1234;
   testMessage.Payload.PhyDataInd.Size = 20;

   /* reset the stats for calls to the serial port */
   SerialPortDataInit(DEBUG_UART);
   /* for SerialPortDebugPrint */
   bSerialDebugPrintToConsole = false;
   bSerialDebugPrintCalled = false;
   SerialDebugPrintCount = 0;
   SerialDebugLastPrint[0] = 0;
   /* Ignore in test mode OFF check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
   TM_rxAlarmSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
      /* Ignore in transparent mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSPARENT_E);
   TM_rxAlarmSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore NULL parameter check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxAlarmSignal(NULL);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Check output in monitoring mode */
   uint8_t expectedOutput[25] = { 0x01,0x02,0x00,0x0a,0x07,0x10,0x01,0x00,0x20,0x00,0x01,0x00,0x20,0x85,0x68,0x00,0x00,0x00,0x00,0x00,0x01,0x23,0x45,0x67,0x80 };
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_MONITORING_E);
   TM_rxAlarmSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 25);
   int32_t diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutput, 25);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Check output in receive mode */
   uint8_t expectedOutputString[256];
   sprintf(expectedOutputString, "Rx ALARMSIG       2 OK   10 :   7 : 10 01 00 20 00 01 00 20 85 68 00 00 00 00 00 01 23 45 67 80 \r\n");
   int32_t expectedLength = strlen(expectedOutputString);
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxAlarmSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, expectedLength);
   diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutputString, expectedLength);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
}

/*************************************************************************************
* TM_rxOutputSignalTest
* Test the operation of function TM_rxOutputSignal;
*
* no params
*
* @return - none
*
*************************************************************************************/
void TM_rxOutputSignalTest(void)
{
   OutputSignal_t outputSignal = { 0 };
   CO_Message_t testMessage;

   /* create a test fire signal */
   outputSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   outputSignal.Header.MACDestinationAddress = 1;
   outputSignal.Header.MACSourceAddress = 2;
   outputSignal.Header.HopCount = 0;
   outputSignal.Header.DestinationAddress = 1;
   outputSignal.Header.SourceAddress = 2;
   outputSignal.Header.MessageType = APP_MSG_TYPE_OUTPUT_SIGNAL_E;
   outputSignal.Zone = 1;
   outputSignal.OutputProfile = 1;
   outputSignal.OutputsActivated = 0x5a;
   outputSignal.SystemId = SYSTEM_ID;

   ErrorCode_t status = MC_PUP_PackOutputSignal(&outputSignal, &testMessage.Payload.PhyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 12;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1234;
   testMessage.Payload.PhyDataInd.Size = 20;

   /* reset the stats for calls to the serial port */
   SerialPortDataInit(DEBUG_UART);
   /* for SerialPortDebugPrint */
   bSerialDebugPrintToConsole = false;
   bSerialDebugPrintCalled = false;
   SerialDebugPrintCount = 0;
   SerialDebugLastPrint[0] = 0;

   /* Ignore in test mode OFF check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
   TM_rxOutputSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore in transparent mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSPARENT_E);
   TM_rxOutputSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore NULL parameter check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxOutputSignal(NULL);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Check output in monitoring mode */
   uint8_t expectedOutput[25] = { 0x03,0x02,0x00,0x0a,0x07,0x10,0x01,0x00,0x20,0x00,0x01,0x00,0x21,0x80,0x08,0x80,0x2d,0x00,0x00,0x00,0x01,0x23,0x45,0x67,0x80 };
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_MONITORING_E);
   TM_rxOutputSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 25);
   int32_t diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutput, 25);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Check output in receive mode */
   uint8_t expectedOutputString[256];
   sprintf(expectedOutputString, "Rx OUTPUTSIG      2 OK   10 :   7 : 10 01 00 20 00 01 00 21 80 08 80 2D 00 00 00 01 23 45 67 80 \r\n");
   int32_t expectedLength = strlen(expectedOutputString);
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxOutputSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, expectedLength);
   diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutputString, expectedLength);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
}

/*************************************************************************************
* TM_rxAckSignalTest
* Test the operation of function TM_rxAckSignal;
*
* no params
*
* @return - none
*
*************************************************************************************/
void TM_rxAckSignalTest(void)
{
   FrameAcknowledge_t ackSignal = { 0 };
   CO_Message_t testMessage;

   /* create a test fire signal */
   ackSignal.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   ackSignal.MACDestinationAddress = 1;
   ackSignal.MACSourceAddress = 2;
   ackSignal.SystemId = SYSTEM_ID;

   ErrorCode_t status = MC_PUP_PackAck(&ackSignal, &testMessage.Payload.PhyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 12;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1234;
   testMessage.Payload.PhyDataInd.Size = 8;

   /* reset the stats for calls to the serial port */
   SerialPortDataInit(DEBUG_UART);
   /* for SerialPortDebugPrint */
   bSerialDebugPrintToConsole = false;
   bSerialDebugPrintCalled = false;
   SerialDebugPrintCount = 0;
   SerialDebugLastPrint[0] = 0;

   /* Ignore in test mode OFF check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
   TM_rxAckSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore in transparent mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSPARENT_E);
   TM_rxAckSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore NULL parameter check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxAckSignal(NULL);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Check output in monitoring mode */
   uint8_t expectedOutput[13] = { 0x11,0x02,0x00,0x0a,0x07,0x20,0x01,0x00,0x21,0x23,0x45,0x67,0x80 };
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_MONITORING_E);
   TM_rxAckSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 13);
   int32_t diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutput, 13);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Check output in receive mode */
   uint8_t expectedOutputString[256];
   sprintf(expectedOutputString, "Rx ACK            2 OK   10 :   7 : 20 01 00 21 23 45 67 80 \r\n");
   int32_t expectedLength = strlen(expectedOutputString);
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxAckSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, expectedLength);
   diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutputString, expectedLength);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
}


/*************************************************************************************
* TM_rxRouteAddTest
* Test the operation of function TM_rxRouteAdd;
*
* no params
*
* @return - none
*
*************************************************************************************/
void TM_rxRouteAddTest(void)
{
   RouteAddMessage_t routeAddSignal = { 0 };
   CO_Message_t testMessage;

   /* create a test fire signal */
   routeAddSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   routeAddSignal.Header.MACDestinationAddress = 1;
   routeAddSignal.Header.MACSourceAddress = 2;
   routeAddSignal.Header.HopCount = 0;
   routeAddSignal.Header.DestinationAddress = 1;
   routeAddSignal.Header.SourceAddress = 2;
   routeAddSignal.Header.MessageType = APP_MSG_TYPE_ROUTE_ADD_E;
   routeAddSignal.Rank = 1;
   routeAddSignal.SystemId = SYSTEM_ID;

   ErrorCode_t status = MC_PUP_PackRouteAddMsg(&routeAddSignal, &testMessage.Payload.PhyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 12;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1234;
   testMessage.Payload.PhyDataInd.Size = 20;

   /* reset the stats for calls to the serial port */
   SerialPortDataInit(DEBUG_UART);
   /* for SerialPortDebugPrint */
   bSerialDebugPrintToConsole = false;
   bSerialDebugPrintCalled = false;
   SerialDebugPrintCount = 0;
   SerialDebugLastPrint[0] = 0;

   /* Ignore in test mode OFF check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
   TM_rxRouteAdd((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore in transparent mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSPARENT_E);
   TM_rxRouteAdd((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore NULL parameter check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxRouteAdd(NULL);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);

   /* Check output in monitoring mode */
   uint8_t expectedOutput[25] = { 0x09,0x02,0x00,0x0a,0x07,0x10,0x01,0x00,0x20,0x00,0x01,0x00,0x24,0x82,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23,0x45,0x67,0x80 };
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_MONITORING_E);
   TM_rxRouteAdd((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 25);
   int32_t diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutput, 25);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Check output in receive mode */
   uint8_t expectedOutputString[256];
   sprintf(expectedOutputString, "Rx ROUTE ADD      2 OK   10 :   7 : 10 01 00 20 00 01 00 24 82 00 00 00 00 00 00 01 23 45 67 80 \r\n");
   int32_t expectedLength = strlen(expectedOutputString);
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxRouteAdd((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, expectedLength);
   diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutputString, expectedLength);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
}


/*************************************************************************************
* TM_rxRouteDropTest
* Test the operation of function TM_rxRouteDrop;
*
* no params
*
* @return - none
*
*************************************************************************************/
void TM_rxRouteDropTest(void)
{
   RouteDropMessage_t routeDropSignal = { 0 };
   CO_Message_t testMessage;

   /* create a test fire signal */
   routeDropSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   routeDropSignal.Header.MACDestinationAddress = 1;
   routeDropSignal.Header.MACSourceAddress = 2;
   routeDropSignal.Header.HopCount = 0;
   routeDropSignal.Header.DestinationAddress = 1;
   routeDropSignal.Header.SourceAddress = 2;
   routeDropSignal.Header.MessageType = APP_MSG_TYPE_ROUTE_DROP_E;
   routeDropSignal.SystemId = SYSTEM_ID;

   ErrorCode_t status = MC_PUP_PackRouteDropMsg(&routeDropSignal, &testMessage.Payload.PhyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 12;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1234;
   testMessage.Payload.PhyDataInd.Size = 20;

   /* reset the stats for calls to the serial port */
   SerialPortDataInit(DEBUG_UART);
   /* for SerialPortDebugPrint */
   bSerialDebugPrintToConsole = false;
   bSerialDebugPrintCalled = false;
   SerialDebugPrintCount = 0;
   SerialDebugLastPrint[0] = 0;

   /* Ignore in test mode OFF check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
   TM_rxRouteDrop((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore in transparent mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSPARENT_E);
   TM_rxRouteDrop((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore NULL parameter check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxRouteDrop(NULL);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Check output in monitoring mode */
   uint8_t expectedOutput[25] = { 0x0b,0x02,0x00,0x0a,0x07,0x10,0x01,0x00,0x20,0x00,0x01,0x00,0x25,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23,0x45,0x67,0x80 };
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_MONITORING_E);
   TM_rxRouteDrop((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 25);
   int32_t diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutput, 25);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Check output in receive mode */
   uint8_t expectedOutputString[256];
   sprintf(expectedOutputString, "Rx ROUTEDROP      2 OK   10 :   7 : 10 01 00 20 00 01 00 25 80 00 00 00 00 00 00 01 23 45 67 80 \r\n");
   int32_t expectedLength = strlen(expectedOutputString);
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxRouteDrop((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, expectedLength);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutputString, expectedLength);
   CU_ASSERT_EQUAL(diff, 0);
}


/*************************************************************************************
* TM_rxFaultSignalTest
* Test the operation of function TM_rxFaultSignal;
*
* no params
*
* @return - none
*
*************************************************************************************/
void TM_rxFaultSignalTest(void)
{
   FaultSignal_t faultSignal = { 0 };
   CO_Message_t testMessage;

   /* create a test fire signal */
   faultSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   faultSignal.Header.MACDestinationAddress = 1;
   faultSignal.Header.MACSourceAddress = 2;
   faultSignal.Header.HopCount = 0;
   faultSignal.Header.DestinationAddress = 1;
   faultSignal.Header.SourceAddress = 2;
   faultSignal.Header.MessageType = APP_MSG_TYPE_FAULT_SIGNAL_E;
   faultSignal.Zone = 1;
   faultSignal.StatusFlags = 0x45;
   faultSignal.DetectorFault = CO_DETECTOR_SENSOR_FAULT;
   faultSignal.BeaconFault = CO_BEACON_ID_MISMATCH;
   faultSignal.SounderFault = CO_SOUNDER_INTERNAL_FAULT;
   faultSignal.SystemId = SYSTEM_ID;

   ErrorCode_t status = MC_PUP_PackFaultSignal(&faultSignal, &testMessage.Payload.PhyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 12;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1234;
   testMessage.Payload.PhyDataInd.Size = 20;

   /* reset the stats for calls to the serial port */
   SerialPortDataInit(DEBUG_UART);
   /* for SerialPortDebugPrint */
   bSerialDebugPrintToConsole = false;
   bSerialDebugPrintCalled = false;
   SerialDebugPrintCount = 0;
   SerialDebugLastPrint[0] = 0;
   /* Ignore in test mode OFF check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
   TM_rxFaultSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore in transparent mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSPARENT_E);
   TM_rxFaultSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore NULL parameter check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxFaultSignal(NULL);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Check output in monitoring mode */
   uint8_t expectedOutput[25] = { 0x02,0x02,0x00,0x0a,0x07,0x10,0x01,0x00,0x20,0x00,0x01,0x00,0x21,0x00,0x00,0x82,0x29,0x91,0x80,0x00,0x01,0x23,0x45,0x67,0x80 };
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_MONITORING_E);
   TM_rxFaultSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 25);
   int32_t diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutput, 25);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Check output in receive mode */
   uint8_t expectedOutputString[256];
   sprintf(expectedOutputString, "Rx FAULTSIG       2 OK   10 :   7 : 10 01 00 20 00 01 00 21 00 00 82 29 91 80 00 01 23 45 67 80 \r\n");
   int32_t expectedLength = strlen(expectedOutputString);
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxFaultSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, expectedLength);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutputString, expectedLength);
   CU_ASSERT_EQUAL(diff, 0);
}


/*************************************************************************************
* TM_rxLogonRequestTest
* Test the operation of function TM_rxLogonRequest;
*
* no params
*
* @return - none
*
*************************************************************************************/
void TM_rxLogonRequestTest(void)
{
   LogOnMessage_t logonMessage = { 0 };
   CO_Message_t testMessage;

   /* create a test fire signal */
   logonMessage.Header.FrameType = FRAME_TYPE_DATA_E;
   logonMessage.Header.MACDestinationAddress = 1;
   logonMessage.Header.MACSourceAddress = 2;
   logonMessage.Header.HopCount = 0;
   logonMessage.Header.DestinationAddress = 1;
   logonMessage.Header.SourceAddress = 2;
   logonMessage.Header.MessageType = APP_MSG_TYPE_LOGON_E;
   logonMessage.SerialNumber = 0x1234;
   logonMessage.DeviceCombination = 0x15;
   logonMessage.ZoneNumber = 0x01;
   logonMessage.SystemId = SYSTEM_ID;

   ErrorCode_t status = MC_PUP_PackLogOnMsg(&logonMessage, &testMessage.Payload.PhyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 12;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1234;
   testMessage.Payload.PhyDataInd.Size = 20;

   /* reset the stats for calls to the serial port */
   SerialPortDataInit(DEBUG_UART);
   /* for SerialPortDebugPrint */
   bSerialDebugPrintToConsole = false;
   bSerialDebugPrintCalled = false;
   SerialDebugPrintCount = 0;
   SerialDebugLastPrint[0] = 0;
   /* Ignore in test mode OFF check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
   TM_rxLogonRequest((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore in transparent mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSPARENT_E);
   TM_rxLogonRequest((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore NULL parameter check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxLogonRequest(NULL);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Check output in monitoring mode */
   uint8_t expectedOutput[25] = { 0x06,0x02,0x00,0x0a,0x07,0x10,0x01,0x00,0x20,0x00,0x01,0x00,0x23,0x00,0x00,0x09,0x1a,0x0a,0x80,0x00,0x81,0x23,0x45,0x67,0x80 };
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_MONITORING_E);
   TM_rxLogonRequest((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 25);
   int32_t diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutput, 25);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Check output in receive mode */
   uint8_t expectedOutputString[256];
   sprintf(expectedOutputString, "Rx LOGON          2 OK   10 :   7 : 10 01 00 20 00 01 00 23 00 00 09 1A 0A 80 00 81 23 45 67 80 \r\n");
   int32_t expectedLength = strlen(expectedOutputString);
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxLogonRequest((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, expectedLength);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutputString, expectedLength);
   CU_ASSERT_EQUAL(diff, 0);
}


/*************************************************************************************
* TM_rxCommandTest
* Test the operation of function TM_rxCommand;
*
* no params
*
* @return - none
*
*************************************************************************************/
void TM_rxCommandTest(void)
{
   CommandMessage_t commandMsg = { 0 };
   CO_Message_t testMessage;

   /* create a test fire signal */
   commandMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   commandMsg.Header.MACDestinationAddress = 1;
   commandMsg.Header.MACSourceAddress = 2;
   commandMsg.Header.HopCount = 0;
   commandMsg.Header.DestinationAddress = 1;
   commandMsg.Header.SourceAddress = 2;
   commandMsg.Header.MessageType = APP_MSG_TYPE_COMMAND_E;
   commandMsg.ParameterType = 1;
   commandMsg.ReadWrite = 1;
   commandMsg.TransactionID = 1;
   commandMsg.P1 = 1;
   commandMsg.P2 = 2;
   commandMsg.Value = 0x1234;
   commandMsg.SystemId = SYSTEM_ID;

   ErrorCode_t status = MC_PUP_PackCommandMessage(&commandMsg, &testMessage.Payload.PhyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 12;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1234;
   testMessage.Payload.PhyDataInd.Size = 20;

   /* reset the stats for calls to the serial port */
   SerialPortDataInit(DEBUG_UART);
   /* for SerialPortDebugPrint */
   bSerialDebugPrintToConsole = false;
   bSerialDebugPrintCalled = false;
   SerialDebugPrintCount = 0;
   SerialDebugLastPrint[0] = 0;
   /* Ignore in test mode OFF check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
   TM_rxCommand((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore in transparent mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSPARENT_E);
   TM_rxCommand((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore NULL parameter check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxCommand(NULL);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Check output in monitoring mode */
   uint8_t expectedOutput[25] = { 0x04,0x02,0x00,0x0a,0x07,0x10,0x01,0x00,0x20,0x00,0x01,0x00,0x22,0x01,0x90,0x10,0x20,0x00,0x01,0x23,0x41,0x23,0x45,0x67,0x80 };
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_MONITORING_E);
   TM_rxCommand((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 25);
   int32_t diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutput, 25);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Check output in receive mode */
   uint8_t expectedOutputString[256];
   sprintf(expectedOutputString, "Rx COMMAND        2 OK   10 :   7 : 10 01 00 20 00 01 00 22 01 90 10 20 00 01 23 41 23 45 67 80 \r\n");
   int32_t expectedLength = strlen(expectedOutputString);
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxCommand((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, expectedLength);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutputString, expectedLength);
   CU_ASSERT_EQUAL(diff, 0);
}


/*************************************************************************************
* TM_rxResponseTest
* Test the operation of function TM_rxResponse;
*
* no params
*
* @return - none
*
*************************************************************************************/
void TM_rxResponseTest(void)
{
   ResponseMessage_t responseMsg = { 0 };
   CO_Message_t testMessage;

   /* create a test fire signal */
   responseMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   responseMsg.Header.MACDestinationAddress = 1;
   responseMsg.Header.MACSourceAddress = 2;
   responseMsg.Header.HopCount = 0;
   responseMsg.Header.DestinationAddress = 1;
   responseMsg.Header.SourceAddress = 2;
   responseMsg.Header.MessageType = APP_MSG_TYPE_RESPONSE_E;
   responseMsg.ParameterType = 1;
   responseMsg.ReadWrite = 1;
   responseMsg.TransactionID = 1;
   responseMsg.P1 = 1;
   responseMsg.P2 = 2;
   responseMsg.Value = 0x1234;
   responseMsg.SystemId = SYSTEM_ID;

   ErrorCode_t status = MC_PUP_PackResponseMessage(&responseMsg, &testMessage.Payload.PhyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 12;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1234;
   testMessage.Payload.PhyDataInd.Size = 20;

   /* reset the stats for calls to the serial port */
   SerialPortDataInit(DEBUG_UART);
   /* for SerialPortDebugPrint */
   bSerialDebugPrintToConsole = false;
   bSerialDebugPrintCalled = false;
   SerialDebugPrintCount = 0;
   SerialDebugLastPrint[0] = 0;
   /* Ignore in test mode OFF check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
   TM_rxResponse((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore in transparent mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSPARENT_E);
   TM_rxResponse((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore NULL parameter check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxResponse(NULL);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Check output in monitoring mode */
   uint8_t expectedOutput[25] = { 0x05,0x02,0x00,0x0a,0x07,0x10,0x01,0x00,0x20,0x00,0x01,0x00,0x22,0x81,0x90,0x10,0x20,0x00,0x01,0x23,0x41,0x23,0x45,0x67,0x80 };
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_MONITORING_E);
   TM_rxResponse((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 25);
   int32_t diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutput, 25);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Check output in receive mode */
   uint8_t expectedOutputString[256];
   sprintf(expectedOutputString, "Rx RESPONSE       2 OK   10 :   7 : 10 01 00 20 00 01 00 22 81 90 10 20 00 01 23 41 23 45 67 80 \r\n");
   int32_t expectedLength = strlen(expectedOutputString);
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxResponse((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, expectedLength);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutputString, expectedLength);
   CU_ASSERT_EQUAL(diff, 0);
}


/*************************************************************************************
* TM_rxTestSignalTest
* Test the operation of function TM_rxTestSignal;
*
* no params
*
* @return - none
*
*************************************************************************************/
void TM_rxTestSignalTest(void)
{
   FrameTestModeMessage_t testSignal = { 0 };
   CO_Message_t testMessage;

   /* create a test fire signal */
   testSignal.FrameType = FRAME_TYPE_TEST_MESSAGE_E;
   testSignal.SourceAddress = 1;
   testSignal.Payload[0] = 0x04030201;
   testSignal.Payload[1] = 0x08070605;
   testSignal.Payload[2] = 0x0c0b0a09;
   testSignal.Payload[3] = 0x0d;
   testSignal.SystemId = SYSTEM_ID;

   ErrorCode_t status = MC_PUP_PackTestSignal(&testSignal, &testMessage.Payload.PhyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 12;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1234;
   testMessage.Payload.PhyDataInd.Size = 20;

   /* reset the stats for calls to the serial port */
   SerialPortDataInit(DEBUG_UART);
   /* for SerialPortDebugPrint */
   bSerialDebugPrintToConsole = false;
   bSerialDebugPrintCalled = false;
   SerialDebugPrintCount = 0;
   SerialDebugLastPrint[0] = 0;
   /* Ignore NULL parameter check */
   TM_rxTestSignal(NULL);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore in test mode OFF check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
   TM_rxTestSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* transparent mode check */
   uint8_t expectedOutput1[13] = { 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d };
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSPARENT_E);
   TM_rxTestSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 13);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   int32_t diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutput1, 13);
   CU_ASSERT_EQUAL(diff, 0);

   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Check output in monitoring mode */
   uint8_t expectedOutput2[18] = { 0x0d,0x01,0x00,0x0a,0x07,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d };
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_MONITORING_E);
   TM_rxTestSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 18);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutput2, 18);
   CU_ASSERT_EQUAL(diff, 0);

   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Check output in receive mode */
   uint8_t expectedOutputString[256];
   sprintf(expectedOutputString, "Rx TESTSIG        1 OK   10 :   7 : 50 01 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 01 23 45 67 80 \r\n");
   int32_t expectedLength = strlen(expectedOutputString);
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxTestSignal((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, expectedLength);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutputString, expectedLength);
   CU_ASSERT_EQUAL(diff, 0);
}


/*************************************************************************************
* TM_rxTestReportTest
* Test the operation of function TM_rxTestReport;
*
* no params
*
* @return - none
*
*************************************************************************************/
void TM_rxTestReportTest(void)
{
   TestModeReport_t report;
   CO_Message_t testMessage;

   uint8_t corruptMessage[20] = { 0xff,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d, 0x0e,0x0f,0x10,0x11,0x12,0x13 };

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 12;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1234;
   testMessage.Payload.PhyDataInd.Size = 20;
   memcpy(testMessage.Payload.PhyDataInd.Data, corruptMessage, sizeof(corruptMessage));

   report.type = APP_MSG_TYPE_UNKNOWN_E;
   report.reason = ERR_MESSAGE_FAIL_E;
   report.pMessage = &testMessage;


   /* reset the stats for calls to the serial port */
   SerialPortDataInit(DEBUG_UART);

   /* Ignore NULL parameter check */
   TM_rxTestReport(NULL);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);

   /* Ignore in test mode OFF check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
   TM_rxTestReport((const uint8_t * const)&report);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);

   /* transparent mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSPARENT_E);
   TM_rxTestReport((const uint8_t * const)&report);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);

   /* transmit mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSMIT_E);
   bSerialDebugPrintToConsole = false;
   bSerialDebugPrintCalled = false;
   SerialDebugPrintCount = 0;
   SerialDebugLastPrint[0] = 0;
   TM_rxTestReport((const uint8_t * const)&report);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);

   /* monitoring mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_MONITORING_E);
   TM_rxTestReport((const uint8_t * const)&report);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);

   /* sleep mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_SLEEP_E);
   TM_rxTestReport((const uint8_t * const)&report);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);

   /* sleep mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_SLEEP_E);
   TM_rxTestReport((const uint8_t * const)&report);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);

   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Check output in receive mode */
   uint8_t expectedOutputString[256];
   sprintf(expectedOutputString, "Rx UNKNOWN   4095 ERR  10 :   7 : FF 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 \r\n");
   int32_t expectedLength = strlen(expectedOutputString);
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxTestReport((const uint8_t * const)&report);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, expectedLength);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   int32_t diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutputString, expectedLength);
   CU_ASSERT_EQUAL(diff, 0);
}


/*************************************************************************************
* TM_generateTestMessageTest
* Test the operation of function TM_generateTestMessage;
*
* no params
*
* @return - none
*
*************************************************************************************/
static void TM_generateTestMessageTest(void)
{
   static uint8_t test_cycle = 0;
   FrameTestModeMessage_t testSignal = { 0 };
   CO_Message_t testMessage;
   uint8_t testBuffer[TEST_PAYLOAD_SIZE] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d};

   test_cycle++;
   gNetworkAddress = 0;

   /* set up the mesh pool and RACHSQ */
   RACHSQ = osMessageCreate(osMessageQ(RACHSQ), NULL);
   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));
   osPoolFlush(MeshPool);
   osMessageQFlush(RACHSQ);
   uint32_t rachsq_count = osMessageCount(RACHSQ);
   CU_ASSERT_EQUAL(rachsq_count, 0);

   /* Start tests */

   /* Ignore in test mode OFF check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
   TM_generateTestMessage((const uint8_t * const)&testBuffer, SYSTEM_ID);
   rachsq_count = osMessageCount(RACHSQ);
   CU_ASSERT_EQUAL(rachsq_count, 0);

   /* Ignore in test mode RECEIVE check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_generateTestMessage((const uint8_t * const)&testBuffer, SYSTEM_ID);
   rachsq_count = osMessageCount(RACHSQ);
   CU_ASSERT_EQUAL(rachsq_count, 0);

   /* Ignore in test mode SLEEP check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_SLEEP_E);
   TM_generateTestMessage((const uint8_t * const)&testBuffer, SYSTEM_ID);
   rachsq_count = osMessageCount(RACHSQ);
   CU_ASSERT_EQUAL(rachsq_count, 0);

   /* NULL data test */
   uint8_t expectedPayload[DATA_MESSAGE_SIZE_BYTES];
   memset(expectedPayload, 0, DATA_MESSAGE_SIZE_BYTES);
   sprintf(expectedPayload, "Node 0 %x", test_cycle);

   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSMIT_E);
   TM_generateTestMessage(NULL, SYSTEM_ID);
   rachsq_count = osMessageCount(RACHSQ);
   CU_ASSERT_EQUAL(rachsq_count, 1);
   /* check the message content */
   osEvent Event = osMessageGet(RACHSQ, 0);
   CU_ASSERT_EQUAL(0, osMessageCount(RACHSQ));
   osStatus status = osPoolFree(MeshPool, Event.value.p);
   CU_ASSERT_EQUAL(osOK, status);
   CU_ASSERT_EQUAL(0, osPoolBlocksAllocatedCount(MeshPool));

   CO_Message_t* pResponse = (CO_Message_t*)Event.value.p;
   CU_ASSERT_TRUE(pResponse != NULL);
   if (pResponse)
   {
      /* check the message length */
      CU_ASSERT_EQUAL(pResponse->Payload.PhyDataReq.Size, DATA_MESSAGE_SIZE_BYTES);

      /* check the frame type */
      uint8_t frame_type = pResponse->Payload.PhyDataReq.Data[0] >> 4;
      CU_ASSERT_EQUAL(frame_type, (uint8_t)FRAME_TYPE_TEST_MESSAGE_E);

      /* check the node id */
      uint16_t node_id = ((uint16_t)pResponse->Payload.PhyDataReq.Data[0] << 8) & 0xf00;
      node_id |= (uint16_t)pResponse->Payload.PhyDataReq.Data[1];
      CU_ASSERT_EQUAL(node_id, gNetworkAddress);

      /* check the payload */
      uint8_t* pMsgPayload = &pResponse->Payload.PhyDataReq.Data[2];
      int32_t diff = memcmp(pMsgPayload, expectedPayload, TEST_PAYLOAD_SIZE);
      CU_ASSERT_EQUAL(diff, 0);
   }

   /* test with specified test data */
   /* create a test signal */
   memcpy(testMessage.Payload.PhyDataReq.Data, &testBuffer, TEST_PAYLOAD_SIZE);

   /*create expected result */
   uint8_t expectedPayload2[DATA_MESSAGE_SIZE_BYTES] = { 0x50,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x01,0x23,0x45,0x67,0x80 };

   /* call the function under test */
   TM_generateTestMessage((const uint8_t* const)&testMessage, SYSTEM_ID);
   /* there should be a message in RACHSQ */
   rachsq_count = osMessageCount(RACHSQ);
   CU_ASSERT_EQUAL(rachsq_count, 1);
   /* check the message content */
   Event = osMessageGet(RACHSQ, 0);
   CU_ASSERT_EQUAL(0, osMessageCount(RACHSQ));
   status = osPoolFree(MeshPool, Event.value.p);
   CU_ASSERT_EQUAL(osOK, status);
   CU_ASSERT_EQUAL(0, osPoolBlocksAllocatedCount(MeshPool));

   pResponse = (CO_Message_t*)Event.value.p;
   CU_ASSERT_TRUE(pResponse != NULL);
   if (pResponse)
   {
      /* check the message length */
      CU_ASSERT_EQUAL(pResponse->Payload.PhyDataReq.Size, DATA_MESSAGE_SIZE_BYTES);

      /* check the payload */
      uint8_t* pMsgPayload = (uint8_t*)&pResponse->Payload.PhyDataReq.Data;
      int32_t diff = memcmp(pMsgPayload, expectedPayload2, DATA_MESSAGE_SIZE_BYTES);
      CU_ASSERT_EQUAL(diff, 0);
   }
}

/*************************************************************************************
* TM_rxSetStateTest
* Test the operation of function TM_rxSetState;
*
* no params
*
* @return - none
*
*************************************************************************************/
void TM_rxSetStateTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   SetStateMessage_t setState1 = { 0 };
   CO_Message_t testMessage;

   setState1.Header.FrameType = FRAME_TYPE_DATA_E;
   setState1.Header.MACDestinationAddress = 0x101;
   setState1.Header.MACSourceAddress = 0x202;
   setState1.Header.HopCount = 4;
   setState1.Header.DestinationAddress = 0x303;
   setState1.Header.SourceAddress = 0x404;
   setState1.Header.MessageType = APP_MSG_TYPE_STATE_SIGNAL_E;
   setState1.State = 1;
   setState1.SystemId = SYSTEM_ID;

   status = MC_PUP_PackSetState(&setState1, &testMessage.Payload.PhyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 12;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1234;
   testMessage.Payload.PhyDataInd.Size = 20;

   /* reset the stats for calls to the serial port */
   SerialPortDataInit(DEBUG_UART);


   /* Ignore in test mode OFF check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
   TM_rxSetState((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* Ignore in transparent mode check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSPARENT_E);
   TM_rxSetState((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Ignore NULL parameter check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxSetState(NULL);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Check output in monitoring mode */
   uint8_t expectedOutput[25] = { 0x0e,0x02,0x00,0x0a,0x07,0x11,0x01,0x20,0x20,0x43,0x03,0x40,0x47,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23,0x45,0x67,0x80 };
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_MONITORING_E);
   TM_rxSetState((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 25);
   int32_t diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutput, 25);
   CU_ASSERT_EQUAL(diff, 0);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   /* reset SerialPortWriteTxBuffer stats */
   SerialPortDataInit(DEBUG_UART);

   /* Check output in receive mode */
   uint8_t expectedOutputString[256];
   sprintf(expectedOutputString, "Rx STATE        514 OK   10 :   7 : 11 01 20 20 43 03 40 47 08 00 00 00 00 00 00 01 23 45 67 80 \r\n");
   int32_t expectedLength = strlen(expectedOutputString);
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
   TM_rxSetState((const uint8_t * const)&testMessage);
   CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, expectedLength);
   CU_ASSERT_FALSE(bSerialDebugPrintCalled);
   diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, expectedOutputString, expectedLength);
   CU_ASSERT_EQUAL(diff, 0);
}


/*************************************************************************************
* TM_NetworkMonitorTest
* Test the operation of the network monitor mode;
*
* no params
*
* @return - none
*
*************************************************************************************/
void TM_NetworkMonitorTest(void)
{
   FrameHeartbeat_t heartbeat;
   CO_Message_t testMessage;
   uint16_t next_wakeup_time = 0;


   TM_Initialise(SYSTEM_ID, 0);

   /* create a test heartbeat structure */
   heartbeat.FrameType = FRAME_TYPE_HEARTBEAT_E;
   heartbeat.SlotIndex = 17;
   heartbeat.ShortFrameIndex = 0;
   heartbeat.LongFrameIndex = 0;
   heartbeat.State = 3;
   heartbeat.Rank = 1;
   heartbeat.NoOfChildrenIdx = 0;
   heartbeat.NoOfChildrenOfPTNIdx = 0;
   heartbeat.SystemId = SYSTEM_ID;
   ErrorCode_t status = MC_PUP_PackHeartbeat(&heartbeat, &testMessage.Payload.PhyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 17;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 17;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 17;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1000;
   testMessage.Payload.PhyDataInd.Size = 11;

   /* reset the stats for calls to the serial port */
   SerialPortDataInit(DEBUG_UART);
   /* for SerialPortDebugPrint */
   bSerialDebugPrintToConsole = false;
   bSerialDebugPrintCalled = false;
   SerialDebugPrintCount = 0;
   SerialDebugLastPrint[0] = 0;
   /* Ignore in test mode OFF check */
   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);

   next_wakeup_time = TM_GetNextWakeupTime(488);
   CU_ASSERT_EQUAL(next_wakeup_time, 1000);

   TM_rxHeartbeat((const uint8_t * const)&testMessage);

   next_wakeup_time = TM_GetNextWakeupTime(1000);
   CU_ASSERT_EQUAL(next_wakeup_time, 1512);

   heartbeat.SlotIndex = 18;
   testMessage.Payload.PhyDataInd.timer = 1520;
   status = MC_PUP_PackHeartbeat(&heartbeat, &testMessage.Payload.PhyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   TM_rxHeartbeat((const uint8_t * const)&testMessage);

   next_wakeup_time = TM_GetNextWakeupTime(1520);
   CU_ASSERT_EQUAL(next_wakeup_time, 2032);

}