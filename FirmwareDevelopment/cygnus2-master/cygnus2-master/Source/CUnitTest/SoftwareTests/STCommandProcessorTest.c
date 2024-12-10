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
*  File         : STSessionManagementTest.c
*
*  Description  : Implementation for the Session Management tests
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
#include "STCommandProcessorTest.h"
#include "MessageQueueStub.h"
#include "MM_CommandProcessor.h"
#include "MM_ATHandleTask.h"
#include "DM_svi.h"
#include "MM_CIEQueueManager.h"



/* Private Functions Prototypes
*************************************************************************************/
static void ProcessCommandNegativeTests(void);
static void NCURejectHeadCommandsTest(void);
static void AnalogueValueTest(void);
static void RemoteNodeTests(void);
static void PluginEnableTest(void);
static void PreAlarmThresholdTest(void);
static void AlarmThresholdTest(void);
static void FaultThresholdTest(void);
static void FlashRateTest(void);
static void ToneSelectionTest(void);
static void PluginTestModeTest(void);
static void PluginSerialNumberTest(void);
static void NeighbourInfoTest(void);
static void StatusFlagsTest(void);
static void DeviceCombinationTest(void);
static void RBUSerialNumberTest(void);
static void RBUEnableTest(void);
static void ModulationBandwidthTest(void);
static void SpreadingFactorTest(void);
static void FrequencyTest(void);
static void CodingRateTest(void);
static void TxPowerTest(void);
static void TestModeTest(void);
static void FirmwareInfoTest(void);
static void FirmwareActiveImageTest(void);
static void RebootTest(void);

/* Externs
*************************************************************************************/
extern bool bCommandReqCalled;
extern bool bCommandReqFail;
extern bool bResponseReqCalled;
extern bool bResponseReqFail;
extern bool bSemaphoreReleaseCalled;
extern bool bSystemResetCalled;
extern CO_CommandData_t Test_Command;
extern CO_ResponseData_t Test_Response;
extern uint16_t gFaultStatusFlags;

extern HAL_StatusTypeDef HAL_I2C_Mem_Read_Response;
extern bool HAL_I2C_Mem_Read_CallCount;
extern uint8_t HAL_I2C_Mem_Read_Response_Data[];
extern HAL_StatusTypeDef HAL_I2C_Mem_Write_Response;
extern bool HAL_I2C_Mem_Write_CallCount;
extern uint8_t HAL_I2C_Mem_Write_Data[];
extern void NVMTestInit(void);
extern void NVMTestDeInit(void);
extern CO_State_t gCurrentDeviceState;

/* local declarations
*************************************************************************************/
static const uint8_t CMD_READ = COMMAND_READ;
static const uint8_t CMD_WRITE = 1;


/* Table containing the test settings */
CU_TestInfo ST_CmdProcTests[] =
{
   { "ProcessCommandNegativeTests",        ProcessCommandNegativeTests },
   { "RemoteNodeTests",                    RemoteNodeTests },
   { "NCURejectHeadCommandsTest",          NCURejectHeadCommandsTest },
   { "AnalogueValueTest",                  AnalogueValueTest },
   { "PluginEnableTest",                   PluginEnableTest },
   { "PreAlarmThresholdTest",              PreAlarmThresholdTest },
   { "AlarmThresholdTest",                 AlarmThresholdTest },
   { "FaultThresholdTest",                 FaultThresholdTest },
   { "FlashRateTest",                      FlashRateTest },
   { "ToneSelectionTest",                  ToneSelectionTest },
   { "PluginTestModeTest",                 PluginTestModeTest },
   { "PluginSerialNumberTest",             PluginSerialNumberTest },
   { "NeighbourInfoTest",                  NeighbourInfoTest },
   { "StatusFlagsTest",                    StatusFlagsTest },
   { "DeviceCombinationTest",              DeviceCombinationTest },
   { "RBUSerialNumberTest",                RBUSerialNumberTest },
   { "ModulationBandwidthTest",            ModulationBandwidthTest },
   { "SpreadingFactorTest",                SpreadingFactorTest },
   { "FrequencyTest",                      FrequencyTest },
   { "CodingRateTest",                     CodingRateTest },
   { "TxPowerTest",                        TxPowerTest },
   { "TestModeTest",                       TestModeTest },
   { "FirmwareInfoTest",                   FirmwareInfoTest },
   //{ "FirmwareActiveImageTest",            FirmwareActiveImageTest },
   { "RebootTest",                         RebootTest },

   CU_TEST_INFO_NULL,
};


/*************************************************************************************
* UTILITY FUNCTIONS
*************************************************************************************/
/*************************************************************************************/
/**
* ST_CmdProc_TestSuiteInit
* Initialise this test suite
*
* no params
*
* @return - int
*/
int ST_CmdProc_TestSuiteInit(void)
{
	NVMTestInit();

	return 0;
}

/*************************************************************************************/
/**
* ST_CmdProc_TestSuiteCleanup
* Clean up after this test suite
*
* no params
*
* @return - int
*/
int ST_CmdProc_TestSuiteCleanup(void)
{
	NVMTestDeInit();
	return 0;
}

void ResetTestData(void)
{
   memset(&Test_Command, 0, sizeof(CO_CommandData_t));
   memset(&Test_Response, 0, sizeof(CO_ResponseData_t));
   bCommandReqCalled = false;
   bCommandReqFail = false;
   bResponseReqCalled = false;
   bResponseReqFail = false;
   bSystemResetCalled = false;
}

bool TestHeadQueue(const HeadMessage_t* pExpectedHeadMsg)
{
   bool result = false;
   osEvent headEvent;
   HeadMessage_t RxdMessage;
   osStatus osStat = osErrorOS;

   headEvent.status = osErrorOS;
   headEvent = osMessageGet(HeadQ, 100);
   CU_ASSERT_EQUAL(osEventMessage, headEvent.status);
   CO_Message_t* pMessage = (CO_Message_t*)headEvent.value.p;
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage && pExpectedHeadMsg)
   {
      //We got a message. Verify the contents.
      CU_ASSERT_EQUAL(CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E, pMessage->Type);
      memcpy(&RxdMessage, pMessage->Payload.PhyDataReq.Data, sizeof(HeadMessage_t));
      CU_ASSERT_EQUAL(pExpectedHeadMsg->ChannelNumber, RxdMessage.ChannelNumber);
      CU_ASSERT_EQUAL(pExpectedHeadMsg->MessageType, RxdMessage.MessageType);
      CU_ASSERT_EQUAL(pExpectedHeadMsg->ProfileIndex, RxdMessage.ProfileIndex);
      CU_ASSERT_EQUAL(pExpectedHeadMsg->TransactionID, RxdMessage.TransactionID);
      CU_ASSERT_EQUAL(pExpectedHeadMsg->Value, RxdMessage.Value);

      //Release the memory pool slot
      osStat = osPoolFree(AppPool, pMessage);
      CU_ASSERT_EQUAL(osOK, osStat);

      result = true;
   }

   return result;
}

bool TestATHandleQueue(const char* pExpectedMsg)
{
   bool result = false;
   osEvent atEvent;
   Cmd_Reply_t RxdMessage;
   osStatus osStat = osErrorOS;

   atEvent.status = osErrorOS;
   atEvent = osMessageGet(ATHandleQ, 100);
   CU_ASSERT_EQUAL(osEventMessage, atEvent.status);
   Cmd_Reply_t* pMessage = (Cmd_Reply_t*)atEvent.value.p;
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage && pExpectedMsg)
   {
      //We got a message. Verify the contents.
      memcpy(&RxdMessage, pMessage, sizeof(Cmd_Reply_t));

      CU_ASSERT_TRUE(pMessage->length > 0);
      if (pMessage->length > 0)
      {
         int32_t delta = strcmp(pExpectedMsg, pMessage->data_buffer);
         CU_ASSERT_EQUAL(delta, 0);
      }


      //Release the memory pool slot
      osStat = osPoolFree(ATHandlePool, pMessage);
      CU_ASSERT_EQUAL(osOK, osStat);

      result = true;
   }

   return result;
}


/*************************************************************************************
* TEST FUNCTIONS
*************************************************************************************/

/*************************************************************************************
* ProcessCommandNegativeTests
* Test MM_CMD_ProcessCommand rejection of bad parameters;
*
* no params
*
* @return - none
*************************************************************************************/
void ProcessCommandNegativeTests(void)
{
   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_ANALOGUE_VALUE_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_READ;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 0;

   // NULL command
   ErrorCode_t error_code = MM_CMD_ProcessCommand(NULL, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_INVALID_POINTER_E);

   // Bad message type
   command.CommandType = PARAM_TYPE_MAX_E;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_MESSAGE_FAIL_E);
}

/*************************************************************************************
* RemoteNodeTests
* Test MM_CMD_ProcessCommand handling of command for another RBU;
*
* no params
*
* @return - none
*************************************************************************************/
void RemoteNodeTests(void)
{
   uint16_t address = 1;
   uint16_t remote_address = 2;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);
   MM_CIEQ_Initialise();
   osPoolFlush(CieMsgPool);
   osMessageQFlush(CieTxBufferQ);
   gCurrentDeviceState = STATE_ACTIVE_E;

   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_ANALOGUE_VALUE_E;
   command.Destination = remote_address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_READ;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 0;

   ResetTestData();

   //The message should be queued in the TxBuffer queue for sending to the mesh
   ErrorCode_t error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   uint32_t message_count = osMessageCount(CieTxBufferQ);
   CU_ASSERT_EQUAL(message_count, 1);
}

/*************************************************************************************
* NCURejectHeadCommandsTest
* Test that the head commands fail on the NCU;
*
* no params
*
* @return - none
*************************************************************************************/
void NCURejectHeadCommandsTest(void)
{
   uint16_t address = 0;
   uint16_t device_combination = 34;
   bool isMaster = true;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_ANALOGUE_VALUE_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_READ;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 0;

   ErrorCode_t error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_MESSAGE_FAIL_E);

   command.CommandType = PARAM_TYPE_ALARM_THRESHOLD_E;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_MESSAGE_FAIL_E);

   command.CommandType = PARAM_TYPE_PRE_ALARM_THRESHOLD_E;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_MESSAGE_FAIL_E);

   command.CommandType = PARAM_TYPE_FAULT_THRESHOLD_E;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_MESSAGE_FAIL_E);

   command.CommandType = PARAM_TYPE_PLUGIN_SERIAL_NUMBER_E;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_MESSAGE_FAIL_E);

   command.CommandType = PARAM_TYPE_PLUGIN_ENABLE_E;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_MESSAGE_FAIL_E);

   command.CommandType = PARAM_TYPE_PLUGIN_TEST_MODE_E;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_MESSAGE_FAIL_E);

}

/*************************************************************************************
* AnalogueValueTest
* Test the analog value command;
*
* no params
*
* @return - none
*************************************************************************************/
void AnalogueValueTest(void)
{
   HeadQ = osMessageCreate(osMessageQ(HeadQ), NULL);
   AppPool = osPoolCreate((const osPoolDef_t *)osPool(AppPool));
   HeadInterfaceSemaphoreId = osSemaphoreCreate(osSemaphore(HeadInterfaceSemaphore), 1);

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_ANALOGUE_VALUE_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_READ;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 0;

   bSemaphoreReleaseCalled = false;
   osMessageQFlush(HeadQ);
   ErrorCode_t error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   CU_ASSERT_TRUE(bSemaphoreReleaseCalled);
   CU_ASSERT_EQUAL(osMessageCount(HeadQ), 1);

   HeadMessage_t ExpectedHeadMsg;
   ExpectedHeadMsg.ChannelNumber = DEV_PIR_DETECTOR_HEAD_E;
   ExpectedHeadMsg.MessageType = HEAD_READ_ANALOGUE_VALUE_E;
   ExpectedHeadMsg.ProfileIndex = command.Parameter2;
   ExpectedHeadMsg.TransactionID = command.TransactionID;
   ExpectedHeadMsg.Value = command.Value;
   bool msgok = TestHeadQueue(&ExpectedHeadMsg);
   CU_ASSERT_TRUE(msgok);

   command.TransactionID = 2;
   command.ReadWrite = CMD_WRITE;
   command.Value = 1;
   bSemaphoreReleaseCalled = false;
   osMessageQFlush(HeadQ);
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_MESSAGE_FAIL_E);
   CU_ASSERT_FALSE(bSemaphoreReleaseCalled);
}

/*************************************************************************************
* PluginEnableTest
* Test the Plugin Enable command;
*
* no params
*
* @return - none
*************************************************************************************/
void PluginEnableTest(void)
{
   HeadQ = osMessageCreate(osMessageQ(HeadQ), NULL);
   AppPool = osPoolCreate((const osPoolDef_t *)osPool(AppPool));
   HeadInterfaceSemaphoreId = osSemaphoreCreate(osSemaphore(HeadInterfaceSemaphore), 1);

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_PLUGIN_ENABLE_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_READ;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 0;

   bSemaphoreReleaseCalled = false;
   osMessageQFlush(HeadQ);
   ErrorCode_t error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   CU_ASSERT_TRUE(bSemaphoreReleaseCalled);
   CU_ASSERT_EQUAL(osMessageCount(HeadQ), 1);

   HeadMessage_t ExpectedHeadMsg;
   ExpectedHeadMsg.ChannelNumber = DEV_PIR_DETECTOR_HEAD_E;
   ExpectedHeadMsg.MessageType = HEAD_READ_ENABLED_E;
   ExpectedHeadMsg.ProfileIndex = command.Parameter2;
   ExpectedHeadMsg.TransactionID = command.TransactionID;
   ExpectedHeadMsg.Value = command.Value;
   bool msgok = TestHeadQueue(&ExpectedHeadMsg);
   CU_ASSERT_TRUE(msgok);

   command.TransactionID = 2;
   command.ReadWrite = CMD_WRITE;
   command.Value = 1;
   ExpectedHeadMsg.TransactionID = command.TransactionID;
   ExpectedHeadMsg.MessageType = HEAD_WRITE_ENABLED_E;
   ExpectedHeadMsg.Value = command.Value;
   bSemaphoreReleaseCalled = false;
   osMessageQFlush(HeadQ);
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   CU_ASSERT_TRUE(bSemaphoreReleaseCalled);
   CU_ASSERT_EQUAL(osMessageCount(HeadQ), 1);
   msgok = TestHeadQueue(&ExpectedHeadMsg);
   CU_ASSERT_TRUE(msgok);
}


/*************************************************************************************
* PreAlarmThresholdTest
* Test the Pre-Alarm Threshold command;
*
* no params
*
* @return - none
*************************************************************************************/
void PreAlarmThresholdTest(void)
{
   HeadQ = osMessageCreate(osMessageQ(HeadQ), NULL);
   AppPool = osPoolCreate((const osPoolDef_t *)osPool(AppPool));
   HeadInterfaceSemaphoreId = osSemaphoreCreate(osSemaphore(HeadInterfaceSemaphore), 1);

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_PRE_ALARM_THRESHOLD_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_READ;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 0;

   bSemaphoreReleaseCalled = false;
   osMessageQFlush(HeadQ);
   ErrorCode_t error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   CU_ASSERT_TRUE(bSemaphoreReleaseCalled);
   CU_ASSERT_EQUAL(osMessageCount(HeadQ), 1);

   HeadMessage_t ExpectedHeadMsg;
   ExpectedHeadMsg.ChannelNumber = DEV_PIR_DETECTOR_HEAD_E;
   ExpectedHeadMsg.MessageType = HEAD_READ_PREALARM_THRESHOLD_E;
   ExpectedHeadMsg.ProfileIndex = command.Parameter2;
   ExpectedHeadMsg.TransactionID = command.TransactionID;
   ExpectedHeadMsg.Value = command.Value;
   bool msgok = TestHeadQueue(&ExpectedHeadMsg);
   CU_ASSERT_TRUE(msgok);

   command.TransactionID = 2;
   command.ReadWrite = CMD_WRITE;
   command.Value = 1;
   ExpectedHeadMsg.TransactionID = command.TransactionID;
   ExpectedHeadMsg.MessageType = HEAD_WRITE_PREALARM_THRESHOLD_E;
   ExpectedHeadMsg.Value = command.Value;
   bSemaphoreReleaseCalled = false;
   osMessageQFlush(HeadQ);
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   CU_ASSERT_TRUE(bSemaphoreReleaseCalled);
   CU_ASSERT_EQUAL(osMessageCount(HeadQ), 1);
   msgok = TestHeadQueue(&ExpectedHeadMsg);
   CU_ASSERT_TRUE(msgok);
}


/*************************************************************************************
* AlarmThresholdTest
* Test the Alarm Threshold command;
*
* no params
*
* @return - none
*************************************************************************************/
void AlarmThresholdTest(void)
{
   HeadQ = osMessageCreate(osMessageQ(HeadQ), NULL);
   AppPool = osPoolCreate((const osPoolDef_t *)osPool(AppPool));
   HeadInterfaceSemaphoreId = osSemaphoreCreate(osSemaphore(HeadInterfaceSemaphore), 1);

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_ALARM_THRESHOLD_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_READ;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 0;

   bSemaphoreReleaseCalled = false;
   osMessageQFlush(HeadQ);
   ErrorCode_t error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   CU_ASSERT_TRUE(bSemaphoreReleaseCalled);
   CU_ASSERT_EQUAL(osMessageCount(HeadQ), 1);

   HeadMessage_t ExpectedHeadMsg;
   ExpectedHeadMsg.ChannelNumber = DEV_PIR_DETECTOR_HEAD_E;
   ExpectedHeadMsg.MessageType = HEAD_READ_ALARM_THRESHOLD_E;
   ExpectedHeadMsg.ProfileIndex = command.Parameter2;
   ExpectedHeadMsg.TransactionID = command.TransactionID;
   ExpectedHeadMsg.Value = command.Value;
   bool msgok = TestHeadQueue(&ExpectedHeadMsg);
   CU_ASSERT_TRUE(msgok);

   command.TransactionID = 2;
   command.ReadWrite = CMD_WRITE;
   command.Value = 1;
   ExpectedHeadMsg.TransactionID = command.TransactionID;
   ExpectedHeadMsg.MessageType = HEAD_WRITE_ALARM_THRESHOLD_E;
   ExpectedHeadMsg.Value = command.Value;
   bSemaphoreReleaseCalled = false;
   osMessageQFlush(HeadQ);
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   CU_ASSERT_TRUE(bSemaphoreReleaseCalled);
   CU_ASSERT_EQUAL(osMessageCount(HeadQ), 1);
   msgok = TestHeadQueue(&ExpectedHeadMsg);
   CU_ASSERT_TRUE(msgok);
}


/*************************************************************************************
* FaultThresholdTest
* Test the Fault Threshold command;
*
* no params
*
* @return - none
*************************************************************************************/
void FaultThresholdTest(void)
{
   HeadQ = osMessageCreate(osMessageQ(HeadQ), NULL);
   AppPool = osPoolCreate((const osPoolDef_t *)osPool(AppPool));
   HeadInterfaceSemaphoreId = osSemaphoreCreate(osSemaphore(HeadInterfaceSemaphore), 1);

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_FAULT_THRESHOLD_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_READ;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 0;

   bSemaphoreReleaseCalled = false;
   osMessageQFlush(HeadQ);
   ErrorCode_t error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   CU_ASSERT_TRUE(bSemaphoreReleaseCalled);
   CU_ASSERT_EQUAL(osMessageCount(HeadQ), 1);

   HeadMessage_t ExpectedHeadMsg;
   ExpectedHeadMsg.ChannelNumber = DEV_PIR_DETECTOR_HEAD_E;
   ExpectedHeadMsg.MessageType = HEAD_READ_FAULT_THRESHOLD_E;
   ExpectedHeadMsg.ProfileIndex = command.Parameter2;
   ExpectedHeadMsg.TransactionID = command.TransactionID;
   ExpectedHeadMsg.Value = command.Value;
   bool msgok = TestHeadQueue(&ExpectedHeadMsg);
   CU_ASSERT_TRUE(msgok);

   command.TransactionID = 2;
   command.ReadWrite = CMD_WRITE;
   command.Value = 1;
   ExpectedHeadMsg.TransactionID = command.TransactionID;
   ExpectedHeadMsg.MessageType = HEAD_WRITE_FAULT_THRESHOLD_E;
   ExpectedHeadMsg.Value = command.Value;
   bSemaphoreReleaseCalled = false;
   osMessageQFlush(HeadQ);
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   CU_ASSERT_TRUE(bSemaphoreReleaseCalled);
   CU_ASSERT_EQUAL(osMessageCount(HeadQ), 1);
   msgok = TestHeadQueue(&ExpectedHeadMsg);
   CU_ASSERT_TRUE(msgok);
}

/*************************************************************************************
* FlashRateTest
* Test the Flash Rate command;
*
* no params
*
* @return - none
*************************************************************************************/
void FlashRateTest(void)
{
   HeadQ = osMessageCreate(osMessageQ(HeadQ), NULL);
   AppPool = osPoolCreate((const osPoolDef_t *)osPool(AppPool));
   HeadInterfaceSemaphoreId = osSemaphoreCreate(osSemaphore(HeadInterfaceSemaphore), 1);
   
   osPoolFlush(AppPool);
   osMessageQFlush(HeadQ);

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   /* Test READ command */
   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_SVI_E;
   command.Destination = address;
   command.Parameter1 = DM_SVI_FLASH_RATE_E;
   command.Parameter2 = 1;
   command.ReadWrite = CMD_READ;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 1;
   bSemaphoreReleaseCalled = false;

   HAL_I2C_Mem_Read_Response = HAL_OK;
   HAL_I2C_Mem_Read_CallCount = 0;
   ErrorCode_t error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Read_CallCount, 1);

   command.TransactionID = 2;
   command.ReadWrite = CMD_WRITE;
   command.Value = 5;
   HAL_I2C_Mem_Write_Response = HAL_OK;
   HAL_I2C_Mem_Write_CallCount = 0;

   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 1);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_Data[0], command.Value);
}

/*************************************************************************************
* ToneSelectionTest
* Test the Tone Selection command;
*
* no params
*
* @return - none
*************************************************************************************/
void ToneSelectionTest(void)
{
   HeadQ = osMessageCreate(osMessageQ(HeadQ), NULL);
   AppPool = osPoolCreate((const osPoolDef_t *)osPool(AppPool));
   HeadInterfaceSemaphoreId = osSemaphoreCreate(osSemaphore(HeadInterfaceSemaphore), 1);

   osPoolFlush(AppPool);
   osMessageQFlush(HeadQ);

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   /* Test READ command */
   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_SVI_E;
   command.Destination = address;
   command.Parameter1 = DM_SVI_TONE_SELECT_E;
   command.Parameter2 = 1;
   command.ReadWrite = CMD_READ;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 1;

   HAL_I2C_Mem_Read_Response = HAL_OK;
   HAL_I2C_Mem_Read_CallCount = 0;
   ErrorCode_t error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Read_CallCount, 1);

   command.TransactionID = 2;
   command.ReadWrite = CMD_WRITE;
   command.Value = 5;
   HAL_I2C_Mem_Write_Response = HAL_OK;
   HAL_I2C_Mem_Write_CallCount = 0;

   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 1);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_Data[0], command.Value);

}


/*************************************************************************************
* PluginTestModeTest
* Test the Plugin Test mode command;
*
* no params
*
* @return - none
*************************************************************************************/
void PluginTestModeTest(void)
{
   HeadQ = osMessageCreate(osMessageQ(HeadQ), NULL);
   AppPool = osPoolCreate((const osPoolDef_t *)osPool(AppPool));
   HeadInterfaceSemaphoreId = osSemaphoreCreate(osSemaphore(HeadInterfaceSemaphore), 1);

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_PLUGIN_TEST_MODE_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 1;
   command.ReadWrite = CMD_READ;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 1;

   bSemaphoreReleaseCalled = false;
   osMessageQFlush(HeadQ);
   ErrorCode_t error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   CU_ASSERT_TRUE(bSemaphoreReleaseCalled);
   CU_ASSERT_EQUAL(osMessageCount(HeadQ), 1);

   HeadMessage_t ExpectedHeadMsg;
   ExpectedHeadMsg.ChannelNumber = DEV_PIR_DETECTOR_HEAD_E;
   ExpectedHeadMsg.MessageType = HEAD_READ_TEST_MODE_E;
   ExpectedHeadMsg.ProfileIndex = 1;
   ExpectedHeadMsg.TransactionID = command.TransactionID;
   ExpectedHeadMsg.Value = command.Value;
   bool msgok = TestHeadQueue(&ExpectedHeadMsg);
   CU_ASSERT_TRUE(msgok);

   command.TransactionID = 2;
   command.ReadWrite = CMD_WRITE;
   command.Value = 5;
   ExpectedHeadMsg.TransactionID = command.TransactionID;
   ExpectedHeadMsg.MessageType = HEAD_WRITE_TEST_MODE_E;
   ExpectedHeadMsg.Value = command.Value;
   bSemaphoreReleaseCalled = false;
   osMessageQFlush(HeadQ);
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   CU_ASSERT_TRUE(bSemaphoreReleaseCalled);
   CU_ASSERT_EQUAL(osMessageCount(HeadQ), 1);
   msgok = TestHeadQueue(&ExpectedHeadMsg);
   CU_ASSERT_TRUE(msgok);
}

/*************************************************************************************
* PluginSerialNumberTest
* Test the Plugin Serial Number command;
*
* no params
*
* @return - none
*************************************************************************************/
void PluginSerialNumberTest(void)
{
   HeadQ = osMessageCreate(osMessageQ(HeadQ), NULL);
   AppPool = osPoolCreate((const osPoolDef_t *)osPool(AppPool));
   HeadInterfaceSemaphoreId = osSemaphoreCreate(osSemaphore(HeadInterfaceSemaphore), 1);

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_PLUGIN_SERIAL_NUMBER_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_READ;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 0;

   bSemaphoreReleaseCalled = false;
   osMessageQFlush(HeadQ);
   ErrorCode_t error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   CU_ASSERT_TRUE(bSemaphoreReleaseCalled);
   CU_ASSERT_EQUAL(osMessageCount(HeadQ), 1);

   HeadMessage_t ExpectedHeadMsg;
   ExpectedHeadMsg.ChannelNumber = DEV_PIR_DETECTOR_HEAD_E;
   ExpectedHeadMsg.MessageType = HEAD_READ_SERIAL_NUMBER_E;
   ExpectedHeadMsg.ProfileIndex = command.Parameter2;
   ExpectedHeadMsg.TransactionID = command.TransactionID;
   ExpectedHeadMsg.Value = command.Value;
   bool msgok = TestHeadQueue(&ExpectedHeadMsg);
   CU_ASSERT_TRUE(msgok);

   command.TransactionID = 2;
   command.ReadWrite = CMD_WRITE;
   command.Value = 1;
   bSemaphoreReleaseCalled = false;
   osMessageQFlush(HeadQ);
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_MESSAGE_FAIL_E);
   CU_ASSERT_FALSE(bSemaphoreReleaseCalled);
}


/*************************************************************************************
* NeighbourInfoTest
* Test the Neighbour Information command;
*
* no params
*
* @return - none
*************************************************************************************/
void NeighbourInfoTest(void)
{
   HeadQ = osMessageCreate(osMessageQ(HeadQ), NULL);
   AppPool = osPoolCreate((const osPoolDef_t *)osPool(AppPool));
   HeadInterfaceSemaphoreId = osSemaphoreCreate(osSemaphore(HeadInterfaceSemaphore), 1);

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_NEIGHBOUR_INFO_E;
   command.Destination = address;
   command.Parameter1 = 1;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_READ;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 0;
   /* a READ command should generate a CommandReq call */
   bCommandReqCalled = false;
   ErrorCode_t error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_TRUE(bCommandReqCalled);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);

   /* a WRITE command is not supported and should should fail */
   bCommandReqCalled = false;
   command.TransactionID = 2;
   command.ReadWrite = CMD_WRITE;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_FALSE(bCommandReqCalled);
   CU_ASSERT_EQUAL(error_code, ERR_MESSAGE_FAIL_E);
}

/*************************************************************************************
* StatusFlagsTest
* Test the Status Flags command;
*
* no params
*
* @return - none
*************************************************************************************/
void StatusFlagsTest(void)
{
   ATHandleQ = osMessageCreate(osMessageQ(ATHandleQ), NULL);
   ATHandlePool = osPoolCreate((const osPoolDef_t *)osPool(ATHandlePool));

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;


   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();
   osMessageQFlush(ATHandleQ);

   /* NULL Parameter Test */
   ErrorCode_t error_code = MM_CMD_ProcessCommand(NULL, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_INVALID_POINTER_E);

   /* check that WRTIE command fails */
   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_STATUS_FLAGS_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_WRITE;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 0;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_MESSAGE_FAIL_E);
   bool result = TestATHandleQueue("ERROR");
   CU_ASSERT_TRUE(result);

   /* check the successful case should return the initialised default value of 0*/
   command.ReadWrite = CMD_READ;
   command.TransactionID = 2;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("0");
   CU_ASSERT_TRUE(result);

   /* change the flags and go again */
   gFaultStatusFlags = 26;
   command.TransactionID = 3;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("26");
   CU_ASSERT_TRUE(result);

}

/*************************************************************************************
* DeviceCombinationTest
* Test the Device Combination command;
*
* no params
*
* @return - none
*************************************************************************************/
void DeviceCombinationTest(void)
{
   ATHandleQ = osMessageCreate(osMessageQ(ATHandleQ), NULL);
   ATHandlePool = osPoolCreate((const osPoolDef_t *)osPool(ATHandlePool));

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   /* NULL Parameter Test */
   ErrorCode_t error_code = MM_CMD_ProcessCommand(NULL, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_INVALID_POINTER_E);

   /* check that WRTIE command fails */
   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_DEVICE_COMBINATION_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_WRITE;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 34;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   bool result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* check the successful case */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 2;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("34");
   CU_ASSERT_TRUE(result);
}

/*************************************************************************************
* RBUSerialNumberTest
* Test the RBUSerial Number command;
*
* no params
*
* @return - none
*************************************************************************************/
void RBUSerialNumberTest(void)
{
   ATHandleQ = osMessageCreate(osMessageQ(ATHandleQ), NULL);
   ATHandlePool = osPoolCreate((const osPoolDef_t *)osPool(ATHandlePool));

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   /* NULL Parameter Test */
   ErrorCode_t error_code = MM_CMD_ProcessCommand(NULL, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_INVALID_POINTER_E);

   /* check that WRTIE command fails */
   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_RBU_SERIAL_NUMBER_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_WRITE;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 1111;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   bool result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* check the successful case */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 2;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("1111");
   CU_ASSERT_TRUE(result);
}

/*************************************************************************************
* RBUEnableTest
* Test the RBU Enable command;
*
* no params
*
* @return - none
*************************************************************************************/
void RBUEnableTest(void)
{
   ATHandleQ = osMessageCreate(osMessageQ(ATHandleQ), NULL);
   ATHandlePool = osPoolCreate((const osPoolDef_t *)osPool(ATHandlePool));

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   /* NULL Parameter Test */
   ErrorCode_t error_code = MM_CMD_ProcessCommand(NULL, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_INVALID_POINTER_E);

   /* check that write command */
   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_RBU_ENABLE_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_WRITE;
   command.Source = 0;
   command.TransactionID = 1;
   command.Value = 1;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   bool result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* check the read command */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 2;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("1");
   CU_ASSERT_TRUE(result);

   /* now disable again */
   command.ReadWrite = CMD_WRITE;
   command.TransactionID = 2;
   command.Value = 0;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* read back the setting */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 3;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("0");
   CU_ASSERT_TRUE(result);

}

/*************************************************************************************
* ModulationBandwidthTest
* Test the Modulation Bandwidth command;
*
* no params
*
* @return - none
*************************************************************************************/
void ModulationBandwidthTest(void)
{
   ATHandleQ = osMessageCreate(osMessageQ(ATHandleQ), NULL);
   ATHandlePool = osPoolCreate((const osPoolDef_t *)osPool(ATHandlePool));

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   /* NULL Parameter Test */
   ErrorCode_t error_code = MM_CMD_ProcessCommand(NULL, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_INVALID_POINTER_E);

   /* check that write command */
   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_MODULATION_BANDWIDTH_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_WRITE;
   command.Source = 0;
   command.TransactionID = 0;
   command.Value = 1;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   bool result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* check the read command */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 1;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("1");
   CU_ASSERT_TRUE(result);

   /* now repeat with another value */
   command.ReadWrite = CMD_WRITE;
   command.TransactionID = 2;
   command.Value = 3;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* read back the setting */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 3;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("3");
   CU_ASSERT_TRUE(result);

}

/*************************************************************************************
* SpreadingFactorTest
* Test the Spreading Factor command;
*
* no params
*
* @return - none
*************************************************************************************/
void SpreadingFactorTest(void)
{
   ATHandleQ = osMessageCreate(osMessageQ(ATHandleQ), NULL);
   ATHandlePool = osPoolCreate((const osPoolDef_t *)osPool(ATHandlePool));

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   /* NULL Parameter Test */
   ErrorCode_t error_code = MM_CMD_ProcessCommand(NULL, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_INVALID_POINTER_E);

   /* check that write command */
   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_SPREADING_FACTOR_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_WRITE;
   command.Source = 0;
   command.TransactionID = 0;
   command.Value = 7;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   bool result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* check the read command */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 1;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("7");
   CU_ASSERT_TRUE(result);

   /* now repeat with another value */
   command.ReadWrite = CMD_WRITE;
   command.TransactionID = 2;
   command.Value = 6;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* read back the setting */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 3;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("6");
   CU_ASSERT_TRUE(result);

}

/*************************************************************************************
* FrequencyTest
* Test the Frequency command;
*
* no params
*
* @return - none
*************************************************************************************/
void FrequencyTest(void)
{
   ATHandleQ = osMessageCreate(osMessageQ(ATHandleQ), NULL);
   ATHandlePool = osPoolCreate((const osPoolDef_t *)osPool(ATHandlePool));

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   /* NULL Parameter Test */
   ErrorCode_t error_code = MM_CMD_ProcessCommand(NULL, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_INVALID_POINTER_E);

   /* check that write command */
   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_FREQUENCY_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_WRITE;
   command.Source = 0;
   command.TransactionID = 0;
   command.Value = 4;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   bool result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* check the read command */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 1;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("4");
   CU_ASSERT_TRUE(result);

   /* now repeat with another value */
   command.ReadWrite = CMD_WRITE;
   command.TransactionID = 2;
   command.Value = 0;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* read back the setting */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 3;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("0");
   CU_ASSERT_TRUE(result);

}

/*************************************************************************************
* CodingRateTest
* Test the Coding Rate command;
*
* no params
*
* @return - none
*************************************************************************************/
void CodingRateTest(void)
{
   ATHandleQ = osMessageCreate(osMessageQ(ATHandleQ), NULL);
   ATHandlePool = osPoolCreate((const osPoolDef_t *)osPool(ATHandlePool));

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   /* NULL Parameter Test */
   ErrorCode_t error_code = MM_CMD_ProcessCommand(NULL, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_INVALID_POINTER_E);

   /* check that write command */
   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_CODING_RATE_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_WRITE;
   command.Source = 0;
   command.TransactionID = 0;
   command.Value = 1;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   bool result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* check the read command */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 1;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("1");
   CU_ASSERT_TRUE(result);

   /* now repeat with another value */
   command.ReadWrite = CMD_WRITE;
   command.TransactionID = 2;
   command.Value = 2;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* read back the setting */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 3;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("2");
   CU_ASSERT_TRUE(result);

}

/*************************************************************************************
* TxPowerTest
* Test the Tx Power command;
*
* no params
*
* @return - none
*************************************************************************************/
void TxPowerTest(void)
{
   ATHandleQ = osMessageCreate(osMessageQ(ATHandleQ), NULL);
   ATHandlePool = osPoolCreate((const osPoolDef_t *)osPool(ATHandlePool));

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   /* NULL Parameter Test */
   ErrorCode_t error_code = MM_CMD_ProcessCommand(NULL, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_INVALID_POINTER_E);

   /* check that write command */
   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_TX_POWER_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_WRITE;
   command.Source = 0;
   command.TransactionID = 0;
   command.Value = 1;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   bool result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* check the read command */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 1;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("1");
   CU_ASSERT_TRUE(result);

   /* now repeat with another value */
   command.ReadWrite = CMD_WRITE;
   command.TransactionID = 2;
   command.Value = 2;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* read back the setting */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 3;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("2");
   CU_ASSERT_TRUE(result);

}

/*************************************************************************************
* TestModeTest
* Test the Test Mode command;
*
* no params
*
* @return - none
*************************************************************************************/
void TestModeTest(void)
{
   ATHandleQ = osMessageCreate(osMessageQ(ATHandleQ), NULL);
   ATHandlePool = osPoolCreate((const osPoolDef_t *)osPool(ATHandlePool));

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   /* NULL Parameter Test */
   ErrorCode_t error_code = MM_CMD_ProcessCommand(NULL, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_INVALID_POINTER_E);

   /* check that write command */
   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_TEST_MODE_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_WRITE;
   command.Source = 0;
   command.TransactionID = 0;
   command.Value = 1;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   bool result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* check the read command */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 1;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("1");
   CU_ASSERT_TRUE(result);

   /* now repeat with another value */
   command.ReadWrite = CMD_WRITE;
   command.TransactionID = 2;
   command.Value = 2;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* read back the setting */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 3;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("2");
   CU_ASSERT_TRUE(result);

}

/*************************************************************************************
* FirmwareInfoTest
* Test the Firmware Info command;
*
* no params
*
* @return - none
*************************************************************************************/
void FirmwareInfoTest(void)
{
   ATHandleQ = osMessageCreate(osMessageQ(ATHandleQ), NULL);
   ATHandlePool = osPoolCreate((const osPoolDef_t *)osPool(ATHandlePool));

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   /* NULL Parameter Test */
   ErrorCode_t error_code = MM_CMD_ProcessCommand(NULL, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_INVALID_POINTER_E);

   /* check that write command fails*/
   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_FIRMWARE_INFO_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_WRITE;
   command.Source = 0;
   command.TransactionID = 0;
   command.Value = 1;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_MESSAGE_FAIL_E);


   /* check the read command */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 1;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   bool result = TestATHandleQueue("16513");// = "1.1.1" in sw_version_nbr[]
   CU_ASSERT_TRUE(result);
}

/*************************************************************************************
* FirmwareActiveImageTest
* Test the Firmware Active Image command;
*
* no params
*
* @return - none
*************************************************************************************/
void FirmwareActiveImageTest(void)
{
   ATHandleQ = osMessageCreate(osMessageQ(ATHandleQ), NULL);
   ATHandlePool = osPoolCreate((const osPoolDef_t *)osPool(ATHandlePool));

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   /* NULL Parameter Test */
   ErrorCode_t error_code = MM_CMD_ProcessCommand(NULL, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_INVALID_POINTER_E);

   /* check that write command */
   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_FIRMWARE_ACTIVE_IMAGE_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_WRITE;
   command.Source = 0;
   command.TransactionID = 0;
   command.Value = 1;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   bool result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* check the read command */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 1;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("1");
   CU_ASSERT_TRUE(result);

   /* now repeat with another value */
   command.ReadWrite = CMD_WRITE;
   command.TransactionID = 2;
   command.Value = 0;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("OK");
   CU_ASSERT_TRUE(result);

   /* read back the setting */
   command.ReadWrite = CMD_READ;
   command.TransactionID = 3;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, SUCCESS_E);
   result = TestATHandleQueue("0");
   CU_ASSERT_TRUE(result);

}

/*************************************************************************************
* RebootTest
* Test the Reboot command;
*
* no params
*
* @return - none
*************************************************************************************/
void RebootTest(void)
{
   ATHandleQ = osMessageCreate(osMessageQ(ATHandleQ), NULL);
   ATHandlePool = osPoolCreate((const osPoolDef_t *)osPool(ATHandlePool));

   uint16_t address = 1;
   uint16_t device_combination = 34;
   bool isMaster = false;
   uint32_t unitSerialNo = 1111;

   MM_CMD_Initialise(address, device_combination, isMaster, unitSerialNo);

   ResetTestData();

   osMessageQFlush(ATHandleQ);

   /* NULL Parameter Test */
   ErrorCode_t error_code = MM_CMD_ProcessCommand(NULL, CMD_SOURCE_UART_CP);
   CU_ASSERT_EQUAL(error_code, ERR_INVALID_POINTER_E);

   /* check that write command */
   CO_CommandData_t command;
   command.CommandType = PARAM_TYPE_REBOOT_E;
   command.Destination = address;
   command.Parameter1 = 0;
   command.Parameter2 = 0;
   command.ReadWrite = CMD_WRITE;
   command.Source = 0;
   command.TransactionID = 0;
   command.Value = 1;
   error_code = MM_CMD_ProcessCommand(&command, CMD_SOURCE_UART_CP);
   CU_ASSERT_TRUE(bSystemResetCalled);
}
