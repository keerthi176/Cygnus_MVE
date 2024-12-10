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
*  File         : STCIEQueueManagerTest.c
*
*  Description  : Implementation for the NCU's CIE Queue Manager tests
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
#include <process.h>

/* User Include Files
*************************************************************************************/
#include "CUnit.h"
#include "CO_ErrorCode.h"
#include "MessageQueueStub.h"
#include "MM_CIEQueueManager.h"
#include "STCIEQueueManagerTest.h"

/* File under test
*************************************************************************************/
//#include "MM_CIEQueueManager.c"

/* Private Functions Prototypes
*************************************************************************************/
static void MM_CIEQ_InitialiseTest(void);
static void FireQueueTest(void);
static void AlarmQueueTest(void);
static void FaultQueueTest(void);
static void MiscQueueTest(void);

/* Global Variables
*************************************************************************************/
static uint32_t LastHandle = 0;


/* Externs
*************************************************************************************/
extern MessageStatusRegister_t CieMessageStatusRegister;
extern void SM_SendEventToApplication(const CO_MessageType_t MeshEventType, const ApplicationMessage_t* pEventMessage);


/* Table containing the test settings */
CU_TestInfo ST_CieQueueMgrTests[] =
{
   { "CIEQ_InitialiseTest",                  MM_CIEQ_InitialiseTest },
   { "FireQueueTest",                        FireQueueTest },
   { "AlarmQueueTest",                       AlarmQueueTest },
   { "FaultQueueTest",                       FaultQueueTest },
   { "MiscQueueTest",                        MiscQueueTest },


   CU_TEST_INFO_NULL,
};



/* Private Variables
*************************************************************************************/

/* stubs for test
*************************************************************************************/
#include "MessageQueueStub.h"

/* CUnit tests for MM_NCUApplicationStubTask
*************************************************************************************/

/*************************************************************************************/
/**
* MM_CIEQ_InitialiseTest
* Test the initialisation of the message queues
*
* no params
*
* @return - none

*/
void MM_CIEQ_InitialiseTest(void)
{
   CieMessageStatusRegister.CieAlarmQCount = 1;
   CieMessageStatusRegister.CieAlarmQLostMessageCount = 1;
   CieMessageStatusRegister.CieFaultQCount = 1;
   CieMessageStatusRegister.CieFaultQLostMessageCount = 1;
   CieMessageStatusRegister.CieFireQCount = 1;
   CieMessageStatusRegister.CieFireQLostMessageCount = 1;
   CieMessageStatusRegister.CieMiscQCount = 1;
   CieMessageStatusRegister.CieMiscQLostMessageCount = 1;
   CieMessageStatusRegister.CieTxBufferQCount = 1;
   CieMessageStatusRegister.CieTxBufferQLostMessageCount = 1;


   //Call the initialise function
   MM_CIEQ_Initialise();
   osPoolFlush(CieMsgPool);
   CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(CieMsgPool), 0);

   //Check for initialisation
   CU_ASSERT_EQUAL(CieMessageStatusRegister.CieAlarmQCount, 0);
   CU_ASSERT_EQUAL(CieMessageStatusRegister.CieAlarmQLostMessageCount, 0);
   CU_ASSERT_EQUAL(CieMessageStatusRegister.CieFaultQCount, 0);
   CU_ASSERT_EQUAL(CieMessageStatusRegister.CieFaultQLostMessageCount, 0);
   CU_ASSERT_EQUAL(CieMessageStatusRegister.CieFireQCount, 0);
   CU_ASSERT_EQUAL(CieMessageStatusRegister.CieFireQLostMessageCount, 0);
   CU_ASSERT_EQUAL(CieMessageStatusRegister.CieMiscQCount, 0);
   CU_ASSERT_EQUAL(CieMessageStatusRegister.CieMiscQLostMessageCount, 0);
   CU_ASSERT_EQUAL(CieMessageStatusRegister.CieTxBufferQCount, 0);
   CU_ASSERT_EQUAL(CieMessageStatusRegister.CieTxBufferQLostMessageCount, 0);

   CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(CieMsgPool), 0);
   CU_ASSERT_EQUAL(osMessageCount(CieFireQ), 0);
   CU_ASSERT_EQUAL(osMessageCount(CieAlarmQ), 0);
   CU_ASSERT_EQUAL(osMessageCount(CieFaultQ), 0);
   CU_ASSERT_EQUAL(osMessageCount(CieMiscQ), 0);
}

/*************************************************************************************/
/**
* FireQueueTest
* Test the operation of the fire queue
*
* no params
*
* @return - none

*/
void FireQueueTest(void)
{
   ErrorCode_t error;
   CieFireQ = osMessageCreate(osMessageQ(CieFireQ), NULL);
   CieBuffer_t read_message;

   MM_CIEQ_Initialise();

   osPoolFlush(CieMsgPool);
   CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(CieMsgPool), 0);

   uint32_t message_count = MM_CIEQ_MessageCount(CIE_Q_FIRE_E);
   CU_ASSERT_EQUAL(message_count, 0);

   //Try reading an empty queue
   error = MM_CIEQ_Pop(CIE_Q_FIRE_E, &read_message);
   CU_ASSERT_EQUAL(error, ERR_NOT_FOUND_E);


   //Add a message
   CieBuffer_t message1;
   strcpy(message1.Buffer, "message1");
   error = MM_CIEQ_Push(CIE_Q_FIRE_E, &message1);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(CieMsgPool), 1);
   message_count = MM_CIEQ_MessageCount(CIE_Q_FIRE_E);
   CU_ASSERT_EQUAL(message_count, 1);

   //Read the message back
   error = MM_CIEQ_Pop(CIE_Q_FIRE_E, &read_message);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   int delta = strcmp(read_message.Buffer, message1.Buffer);
   CU_ASSERT_EQUAL(delta, 0);

   //Read the message back again.  it should be the same one
   memset(read_message.Buffer, 0, sizeof(CieBuffer_t));
   error = MM_CIEQ_Pop(CIE_Q_FIRE_E, &read_message);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   delta = strcmp(read_message.Buffer, message1.Buffer);
   CU_ASSERT_EQUAL(delta, 0);

   //Clear the message and read again. 
   //We should get ERROR_NOT_FOUND and the queue should be empty
   error = MM_CIEQ_Discard(CIE_Q_FIRE_E);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   error = MM_CIEQ_Pop(CIE_Q_FIRE_E, &read_message);
   CU_ASSERT_EQUAL(error, ERR_NOT_FOUND_E);
   message_count = MM_CIEQ_MessageCount(CIE_Q_FIRE_E);
   CU_ASSERT_EQUAL(message_count, 0);

   //Test the queue capacity
   /* make sure there are no lost messages yet*/
   message_count = MM_CIEQ_LostMessageCount(CIE_Q_FIRE_E);
   CU_ASSERT_EQUAL(message_count, 0);
   //Now fill the queue
   CieBuffer_t testMsg;
   for (int index = 1; index <= CIE_Q_SIZE; index++)
   {
      sprintf(testMsg.Buffer, "message%d", index);
      error = MM_CIEQ_Push(CIE_Q_FIRE_E, &testMsg);
      CU_ASSERT_EQUAL(error, SUCCESS_E);
      CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(CieMsgPool), index);
      message_count = MM_CIEQ_MessageCount(CIE_Q_FIRE_E);
      CU_ASSERT_EQUAL(message_count, index);
   }
   CU_ASSERT_EQUAL(message_count, CIE_Q_SIZE);


   //The queue should be full.  The next message should be rejected with ERR_NO_RESOURCE_E.
   sprintf(testMsg.Buffer, "message%d", CIE_Q_SIZE + 1);
   error = MM_CIEQ_Push(CIE_Q_FIRE_E, &testMsg);
   CU_ASSERT_EQUAL(error, ERR_NO_RESOURCE_E);
   /* Check that the lost message count has increased*/
   message_count = MM_CIEQ_LostMessageCount(CIE_Q_FIRE_E);
   CU_ASSERT_EQUAL(message_count, 1);
   //Try again and check that another lost message is counted
   error = MM_CIEQ_Push(CIE_Q_FIRE_E, &testMsg);
   CU_ASSERT_EQUAL(error, ERR_NO_RESOURCE_E);
   message_count = MM_CIEQ_LostMessageCount(CIE_Q_FIRE_E);
   CU_ASSERT_EQUAL(message_count, 2);
   /* test the reset function for the lost message count*/
   error = MM_CIEQ_ResetLostMessageCount(CIE_Q_FIRE_E);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   message_count = MM_CIEQ_LostMessageCount(CIE_Q_FIRE_E);
   CU_ASSERT_EQUAL(message_count, 0);

   //Now read back all of the messages in the queue and check they are correct
   for (int index = 1; index <= CIE_Q_SIZE; index++)
   {
      sprintf(testMsg.Buffer, "message%d", index);
      error = MM_CIEQ_Pop(CIE_Q_FIRE_E, &read_message);
      CU_ASSERT_EQUAL(error, SUCCESS_E);
      delta = strcmp(read_message.Buffer, testMsg.Buffer);
      CU_ASSERT_EQUAL(delta, 0);
      error = MM_CIEQ_Discard(CIE_Q_FIRE_E);
      CU_ASSERT_EQUAL(error, SUCCESS_E);
      message_count = MM_CIEQ_MessageCount(CIE_Q_FIRE_E);
      CU_ASSERT_EQUAL(message_count, CIE_Q_SIZE - index);
   }
   CU_ASSERT_EQUAL(message_count, 0);

   /* check the queue flush function*/
   for (int index = 0; index < 5; index++)
   {
      error = MM_CIEQ_Push(CIE_Q_FIRE_E, &testMsg);
      CU_ASSERT_EQUAL(error, SUCCESS_E);
   }
   message_count = MM_CIEQ_MessageCount(CIE_Q_FIRE_E);
   CU_ASSERT_EQUAL(message_count, 5);

   error = MM_CIEQ_ResetMessageQueue(CIE_Q_FIRE_E);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   message_count = MM_CIEQ_MessageCount(CIE_Q_FIRE_E);
   CU_ASSERT_EQUAL(message_count, 0);

   //Try reading the empty queue
   error = MM_CIEQ_Pop(CIE_Q_FIRE_E, &read_message);
   CU_ASSERT_EQUAL(error, ERR_NOT_FOUND_E);
}

/*************************************************************************************/
/**
* AlarmQueueTest
* Test the operation of the alarm queue
*
* no params
*
* @return - none

*/
void AlarmQueueTest(void)
{
   ErrorCode_t error;
   CieAlarmQ = osMessageCreate(osMessageQ(CieAlarmQ), NULL);
   CieBuffer_t read_message;

   MM_CIEQ_Initialise();

   osPoolFlush(CieMsgPool);
   CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(CieMsgPool), 0);

   uint32_t message_count = MM_CIEQ_MessageCount(CIE_Q_ALARM_E);
   CU_ASSERT_EQUAL(message_count, 0);

   //Try reading an empty queue
   error = MM_CIEQ_Pop(CIE_Q_ALARM_E, &read_message);
   CU_ASSERT_EQUAL(error, ERR_NOT_FOUND_E);


   //Add a message
   CieBuffer_t message1;
   strcpy(message1.Buffer, "message1");
   error = MM_CIEQ_Push(CIE_Q_ALARM_E, &message1);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(CieMsgPool), 1);
   message_count = MM_CIEQ_MessageCount(CIE_Q_ALARM_E);
   CU_ASSERT_EQUAL(message_count, 1);

   //Read the message back
   error = MM_CIEQ_Pop(CIE_Q_ALARM_E, &read_message);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   int delta = strcmp(read_message.Buffer, message1.Buffer);
   CU_ASSERT_EQUAL(delta, 0);

   //Read the message back again.  it should be the same one
   memset(read_message.Buffer, 0, sizeof(CieBuffer_t));
   error = MM_CIEQ_Pop(CIE_Q_ALARM_E, &read_message);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   delta = strcmp(read_message.Buffer, message1.Buffer);
   CU_ASSERT_EQUAL(delta, 0);

   //Clear the message and read again. 
   //We should get ERROR_NOT_FOUND and the queue should be empty
   error = MM_CIEQ_Discard(CIE_Q_ALARM_E);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   error = MM_CIEQ_Pop(CIE_Q_ALARM_E, &read_message);
   CU_ASSERT_EQUAL(error, ERR_NOT_FOUND_E);
   message_count = MM_CIEQ_MessageCount(CIE_Q_ALARM_E);
   CU_ASSERT_EQUAL(message_count, 0);

   //Test the queue capacity
   /* make sure there are no lost messages yet*/
   message_count = MM_CIEQ_LostMessageCount(CIE_Q_ALARM_E);
   CU_ASSERT_EQUAL(message_count, 0);
   //Now fill the queue
   CieBuffer_t testMsg;
   for (int index = 1; index <= CIE_Q_SIZE; index++)
   {
      sprintf(testMsg.Buffer, "message%d", index);
      error = MM_CIEQ_Push(CIE_Q_ALARM_E, &testMsg);
      CU_ASSERT_EQUAL(error, SUCCESS_E);
      CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(CieMsgPool), index);
      message_count = MM_CIEQ_MessageCount(CIE_Q_ALARM_E);
      CU_ASSERT_EQUAL(message_count, index);
   }
   CU_ASSERT_EQUAL(message_count, CIE_Q_SIZE);


   //The queue should be full.  The next message should be rejected with ERR_NO_RESOURCE_E.
   sprintf(testMsg.Buffer, "message%d", CIE_Q_SIZE + 1);
   error = MM_CIEQ_Push(CIE_Q_ALARM_E, &testMsg);
   CU_ASSERT_EQUAL(error, ERR_NO_RESOURCE_E);
   /* Check that the lost message count has increased*/
   message_count = MM_CIEQ_LostMessageCount(CIE_Q_ALARM_E);
   CU_ASSERT_EQUAL(message_count, 1);
   //Try again and check that another lost message is counted
   error = MM_CIEQ_Push(CIE_Q_ALARM_E, &testMsg);
   CU_ASSERT_EQUAL(error, ERR_NO_RESOURCE_E);
   message_count = MM_CIEQ_LostMessageCount(CIE_Q_ALARM_E);
   CU_ASSERT_EQUAL(message_count, 2);
   /* test the reset function for the lost message count*/
   error = MM_CIEQ_ResetLostMessageCount(CIE_Q_ALARM_E);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   message_count = MM_CIEQ_LostMessageCount(CIE_Q_ALARM_E);
   CU_ASSERT_EQUAL(message_count, 0);

   //Now read back all of the messages in the queue and check they are correct
   for (int index = 1; index <= CIE_Q_SIZE; index++)
   {
      sprintf(testMsg.Buffer, "message%d", index);
      error = MM_CIEQ_Pop(CIE_Q_ALARM_E, &read_message);
      CU_ASSERT_EQUAL(error, SUCCESS_E);
      delta = strcmp(read_message.Buffer, testMsg.Buffer);
      CU_ASSERT_EQUAL(delta, 0);
      error = MM_CIEQ_Discard(CIE_Q_ALARM_E);
      CU_ASSERT_EQUAL(error, SUCCESS_E);
      message_count = MM_CIEQ_MessageCount(CIE_Q_ALARM_E);
      CU_ASSERT_EQUAL(message_count, CIE_Q_SIZE - index);
   }
   CU_ASSERT_EQUAL(message_count, 0);

   /* check the queue flush function*/
   for (int index = 0; index < 5; index++)
   {
      error = MM_CIEQ_Push(CIE_Q_ALARM_E, &testMsg);
      CU_ASSERT_EQUAL(error, SUCCESS_E);
   }
   message_count = MM_CIEQ_MessageCount(CIE_Q_ALARM_E);
   CU_ASSERT_EQUAL(message_count, 5);

   error = MM_CIEQ_ResetMessageQueue(CIE_Q_ALARM_E);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   message_count = MM_CIEQ_MessageCount(CIE_Q_ALARM_E);
   CU_ASSERT_EQUAL(message_count, 0);

   //Try reading the empty queue
   error = MM_CIEQ_Pop(CIE_Q_ALARM_E, &read_message);
   CU_ASSERT_EQUAL(error, ERR_NOT_FOUND_E);
}

/*************************************************************************************/
/**
* FaultQueueTest
* Test the operation of the fault queue
*
* no params
*
* @return - none

*/
void FaultQueueTest(void)
{
   ErrorCode_t error;
   CieFaultQ = osMessageCreate(osMessageQ(CieFaultQ), NULL);
   CieBuffer_t read_message;

   MM_CIEQ_Initialise();

   osPoolFlush(CieMsgPool);
   CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(CieMsgPool), 0);

   uint32_t message_count = MM_CIEQ_MessageCount(CIE_Q_FAULT_E);
   CU_ASSERT_EQUAL(message_count, 0);

   //Try reading an empty queue
   error = MM_CIEQ_Pop(CIE_Q_FAULT_E, &read_message);
   CU_ASSERT_EQUAL(error, ERR_NOT_FOUND_E);


   //Add a message
   CieBuffer_t message1;
   strcpy(message1.Buffer, "message1");
   error = MM_CIEQ_Push(CIE_Q_FAULT_E, &message1);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(CieMsgPool), 1);
   message_count = MM_CIEQ_MessageCount(CIE_Q_FAULT_E);
   CU_ASSERT_EQUAL(message_count, 1);

   //Read the message back
   error = MM_CIEQ_Pop(CIE_Q_FAULT_E, &read_message);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   int delta = strcmp(read_message.Buffer, message1.Buffer);
   CU_ASSERT_EQUAL(delta, 0);

   //Read the message back again.  it should be the same one
   memset(read_message.Buffer, 0, sizeof(CieBuffer_t));
   error = MM_CIEQ_Pop(CIE_Q_FAULT_E, &read_message);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   delta = strcmp(read_message.Buffer, message1.Buffer);
   CU_ASSERT_EQUAL(delta, 0);

   //Clear the message and read again. 
   //We should get ERROR_NOT_FOUND and the queue should be empty
   error = MM_CIEQ_Discard(CIE_Q_FAULT_E);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   error = MM_CIEQ_Pop(CIE_Q_FAULT_E, &read_message);
   CU_ASSERT_EQUAL(error, ERR_NOT_FOUND_E);
   message_count = MM_CIEQ_MessageCount(CIE_Q_FAULT_E);
   CU_ASSERT_EQUAL(message_count, 0);

   //Test the queue capacity
   /* make sure there are no lost messages yet*/
   message_count = MM_CIEQ_LostMessageCount(CIE_Q_FAULT_E);
   CU_ASSERT_EQUAL(message_count, 0);
   //Now fill the queue
   CieBuffer_t testMsg;
   for (int index = 1; index <= CIE_Q_SIZE; index++)
   {
      sprintf(testMsg.Buffer, "message%d", index);
      error = MM_CIEQ_Push(CIE_Q_FAULT_E, &testMsg);
      CU_ASSERT_EQUAL(error, SUCCESS_E);
      CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(CieMsgPool), index);
      message_count = MM_CIEQ_MessageCount(CIE_Q_FAULT_E);
      CU_ASSERT_EQUAL(message_count, index);
   }
   CU_ASSERT_EQUAL(message_count, CIE_Q_SIZE);


   //The queue should be full.  The next message should be rejected with ERR_NO_RESOURCE_E.
   sprintf(testMsg.Buffer, "message%d", CIE_Q_SIZE + 1);
   error = MM_CIEQ_Push(CIE_Q_FAULT_E, &testMsg);
   CU_ASSERT_EQUAL(error, ERR_NO_RESOURCE_E);
   /* Check that the lost message count has increased*/
   message_count = MM_CIEQ_LostMessageCount(CIE_Q_FAULT_E);
   CU_ASSERT_EQUAL(message_count, 1);
   //Try again and check that another lost message is counted
   error = MM_CIEQ_Push(CIE_Q_FAULT_E, &testMsg);
   CU_ASSERT_EQUAL(error, ERR_NO_RESOURCE_E);
   message_count = MM_CIEQ_LostMessageCount(CIE_Q_FAULT_E);
   CU_ASSERT_EQUAL(message_count, 2);
   /* test the reset function for the lost message count*/
   error = MM_CIEQ_ResetLostMessageCount(CIE_Q_FAULT_E);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   message_count = MM_CIEQ_LostMessageCount(CIE_Q_FAULT_E);
   CU_ASSERT_EQUAL(message_count, 0);

   //Now read back all of the messages in the queue and check they are correct
   for (int index = 1; index <= CIE_Q_SIZE; index++)
   {
      sprintf(testMsg.Buffer, "message%d", index);
      error = MM_CIEQ_Pop(CIE_Q_FAULT_E, &read_message);
      CU_ASSERT_EQUAL(error, SUCCESS_E);
      delta = strcmp(read_message.Buffer, testMsg.Buffer);
      CU_ASSERT_EQUAL(delta, 0);
      error = MM_CIEQ_Discard(CIE_Q_FAULT_E);
      CU_ASSERT_EQUAL(error, SUCCESS_E);
      message_count = MM_CIEQ_MessageCount(CIE_Q_FAULT_E);
      CU_ASSERT_EQUAL(message_count, CIE_Q_SIZE - index);
   }
   CU_ASSERT_EQUAL(message_count, 0);

   /* check the queue flush function*/
   for (int index = 0; index < 5; index++)
   {
      error = MM_CIEQ_Push(CIE_Q_FAULT_E, &testMsg);
      CU_ASSERT_EQUAL(error, SUCCESS_E);
   }
   message_count = MM_CIEQ_MessageCount(CIE_Q_FAULT_E);
   CU_ASSERT_EQUAL(message_count, 5);

   error = MM_CIEQ_ResetMessageQueue(CIE_Q_FAULT_E);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   message_count = MM_CIEQ_MessageCount(CIE_Q_FAULT_E);
   CU_ASSERT_EQUAL(message_count, 0);

   //Try reading the empty queue
   error = MM_CIEQ_Pop(CIE_Q_FAULT_E, &read_message);
   CU_ASSERT_EQUAL(error, ERR_NOT_FOUND_E);
}

/*************************************************************************************/
/**
* MiscQueueTest
* Test the operation of the misc queue
*
* no params
*
* @return - none

*/
void MiscQueueTest(void)
{
   ErrorCode_t error;
   CieMiscQ = osMessageCreate(osMessageQ(CieMiscQ), NULL);
   CieBuffer_t read_message;

   MM_CIEQ_Initialise();

   osPoolFlush(CieMsgPool);
   CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(CieMsgPool), 0);

   uint32_t message_count = MM_CIEQ_MessageCount(CIE_Q_MISC_E);
   CU_ASSERT_EQUAL(message_count, 0);

   CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(CieMsgPool), 0);

   //Try reading an empty queue
   error = MM_CIEQ_Pop(CIE_Q_MISC_E, &read_message);
   CU_ASSERT_EQUAL(error, ERR_NOT_FOUND_E);

   CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(CieMsgPool), 0);

   //Add a message
   CieBuffer_t message1;
   strcpy(message1.Buffer, "message1");
   error = MM_CIEQ_Push(CIE_Q_MISC_E, &message1);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(CieMsgPool), 1);
   message_count = MM_CIEQ_MessageCount(CIE_Q_MISC_E);
   CU_ASSERT_EQUAL(message_count, 1);

   //Read the message back
   error = MM_CIEQ_Pop(CIE_Q_MISC_E, &read_message);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   int delta = strcmp(read_message.Buffer, message1.Buffer);
   CU_ASSERT_EQUAL(delta, 0);

   //Read the message back again.  it should be the same one
   memset(read_message.Buffer, 0, sizeof(CieBuffer_t));
   error = MM_CIEQ_Pop(CIE_Q_MISC_E, &read_message);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   delta = strcmp(read_message.Buffer, message1.Buffer);
   CU_ASSERT_EQUAL(delta, 0);

   //Clear the message and read again. 
   //We should get ERROR_NOT_FOUND and the queue should be empty
   error = MM_CIEQ_Discard(CIE_Q_MISC_E);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   error = MM_CIEQ_Pop(CIE_Q_MISC_E, &read_message);
   CU_ASSERT_EQUAL(error, ERR_NOT_FOUND_E);
   message_count = MM_CIEQ_MessageCount(CIE_Q_MISC_E);
   CU_ASSERT_EQUAL(message_count, 0);

   //Test the queue capacity
   /* make sure there are no lost messages yet*/
   message_count = MM_CIEQ_LostMessageCount(CIE_Q_MISC_E);
   CU_ASSERT_EQUAL(message_count, 0);
   //Now fill the queue
   CieBuffer_t testMsg;
   for (int index = 1; index <= CIE_Q_SIZE; index++)
   {
      sprintf(testMsg.Buffer, "message%d", index);
      error = MM_CIEQ_Push(CIE_Q_MISC_E, &testMsg);
      CU_ASSERT_EQUAL(error, SUCCESS_E);
      CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(CieMsgPool), index);
      message_count = MM_CIEQ_MessageCount(CIE_Q_MISC_E);
      CU_ASSERT_EQUAL(message_count, index);
   }
   CU_ASSERT_EQUAL(message_count, CIE_Q_SIZE);


   //The queue should be full.  The next message should be rejected with ERR_NO_RESOURCE_E.
   sprintf(testMsg.Buffer, "message%d", CIE_Q_SIZE + 1);
   error = MM_CIEQ_Push(CIE_Q_MISC_E, &testMsg);
   CU_ASSERT_EQUAL(error, ERR_NO_RESOURCE_E);
   /* Check that the lost message count has increased*/
   message_count = MM_CIEQ_LostMessageCount(CIE_Q_MISC_E);
   CU_ASSERT_EQUAL(message_count, 1);
   //Try again and check that another lost message is counted
   error = MM_CIEQ_Push(CIE_Q_MISC_E, &testMsg);
   CU_ASSERT_EQUAL(error, ERR_NO_RESOURCE_E);
   message_count = MM_CIEQ_LostMessageCount(CIE_Q_MISC_E);
   CU_ASSERT_EQUAL(message_count, 2);
   /* test the reset function for the lost message count*/
   error = MM_CIEQ_ResetLostMessageCount(CIE_Q_MISC_E);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   message_count = MM_CIEQ_LostMessageCount(CIE_Q_MISC_E);
   CU_ASSERT_EQUAL(message_count, 0);

   //Now read back all of the messages in the queue and check they are correct
   for (int index = 1; index <= CIE_Q_SIZE; index++)
   {
      sprintf(testMsg.Buffer, "message%d", index);
      error = MM_CIEQ_Pop(CIE_Q_MISC_E, &read_message);
      CU_ASSERT_EQUAL(error, SUCCESS_E);
      delta = strcmp(read_message.Buffer, testMsg.Buffer);
      CU_ASSERT_EQUAL(delta, 0);
      error = MM_CIEQ_Discard(CIE_Q_MISC_E);
      CU_ASSERT_EQUAL(error, SUCCESS_E);
      message_count = MM_CIEQ_MessageCount(CIE_Q_MISC_E);
      CU_ASSERT_EQUAL(message_count, CIE_Q_SIZE - index);
   }
   CU_ASSERT_EQUAL(message_count, 0);

   /* check the queue flush function*/
   for (int index = 0; index < 5; index++)
   {
      error = MM_CIEQ_Push(CIE_Q_MISC_E, &testMsg);
      CU_ASSERT_EQUAL(error, SUCCESS_E);
   }
   message_count = MM_CIEQ_MessageCount(CIE_Q_MISC_E);
   CU_ASSERT_EQUAL(message_count, 5);

   error = MM_CIEQ_ResetMessageQueue(CIE_Q_MISC_E);
   CU_ASSERT_EQUAL(error, SUCCESS_E);
   message_count = MM_CIEQ_MessageCount(CIE_Q_MISC_E);
   CU_ASSERT_EQUAL(message_count, 0);

   //Try reading the empty queue
   error = MM_CIEQ_Pop(CIE_Q_MISC_E, &read_message);
   CU_ASSERT_EQUAL(error, ERR_NOT_FOUND_E);

}


