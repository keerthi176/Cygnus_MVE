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
*  File         : STRBUApplicationTest.c
*
*  Description  : Implementation for the RBU Application tests
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
#include "CO_Message.h"
#include "SM_StateMachine.h"
#include "MM_ApplicationCommon.h"
#include "STRBUApplicationTest.h"
#include "MM_RBUApplicationTask.h"
#include "gpio.h"
#include "DM_NVM_cfg.h"
#include "eeprom_emul.h"


/* Private Functions Prototypes
*************************************************************************************/
static void ST_MemPoolTest(void);
static void ST_MessageQTest(void);
static void ST_SemaphoreTest(void);
static void ReleaseSem(void* sem_data);
static void RBU_GetNextHandleTest(void);
static void RBU_TaskInitTest(void);
static void RBU_RBUApplicationInitialiseTest(void);
static void RBU_RBUApplicationFireSignalReqTest(void);
static void RBU_RBUApplicationAlarmSignalReqTest(void);
static void RBU_RBUApplicationLogonReqTest(void);
static void RBU_RBUApplicationTaskProcessMessageTest(void);
static void MM_RBUApplicationProcessOutputMessageTest(void);

/* Global Variables
*************************************************************************************/
static uint32_t LastHandle = 0;
static bool bPrintToConsole = false; //Set this to 'true' in your test to have SerialDebugPrint write to the console.
//static bool bSerialDebugPrintCalled = false;
//static int SerialDebugPrintCount = 0;
bool bFaultSignalReqCalled = false;
bool bFaultSignalReqFail = false;
bool bFireSignalReqCalled = false;
bool bFireSignalReqFail = false;
bool bAlarmSignalReqCalled = false;
bool bAlarmSignalReqFail = false;
bool bOutputSignalReqCalled = false;
bool bOutputSignalReqFail = false;
bool bLogonReqCalled = false;
bool bLogonReqFail = false;
bool bCommandReqCalled = false;
bool bCommandReqFail = false;
bool bResponseReqCalled = false;
bool bResponseReqFail = false;
bool bTestMessageReqCalled = false;
bool bTestMessageReqFail = false;
bool bRBUDisableReqCalled = false;
bool bRBUDisableReqFail = false;
bool bGenerateStatusSignalReqCalled = false;
bool bGenerateStatusSignalReqFail = false;
bool bGenerateFaultSignalReqCalled = false;
bool bGenerateFaultSignalReqFail = false;
bool bGenerateSetStateReqCalled = false;
bool bGenerateSetStateReqFail = false;


uint32_t Test_Handle;
uint16_t Test_StatusFlags;
uint8_t Test_RUChannelIndex;
uint8_t Test_SensorValue;
uint8_t Test_OutputProfile;
uint8_t Test_OutputsActivated;
uint16_t Test_Destination;
uint32_t Test_SerialNumber;
uint8_t Test_DeviceCombination;
uint16_t Test_ZoneNumber;
char sLastDebugPrint[256];

CO_CommandData_t Test_Command;
CO_ResponseData_t Test_Response;

/* Externs
*************************************************************************************/
extern void MM_RBUApplicationInitialise(void);
extern void MM_RBUApplicationTaskProcessMessage(const uint32_t timeout);
extern void MM_RBUApplicationProcessOutputMessage(const OutputSignalIndication_t* OutputData);
extern void MM_RBUApplicationFireSignalReq(const CO_RBUSensorData_t* pHeadData);
extern void MM_RBUApplicationAlarmSignalReq(const CO_RBUSensorData_t* pSensorData);
extern void MM_RBUApplicationLogonReq(void);
extern void SetEepromAddress(uint8_t* address);

extern EE_Status eeprom_return_value;
extern uint32_t gZoneNumber;
extern uint32_t gBank1VersionStringLocation;
extern uint32_t gBank2VersionStringLocation;

/* Table containing the test settings */
CU_TestInfo ST_RBUAppTests[] =
{
   { "MemPoolTest",                          ST_MemPoolTest },
   { "MessageQTest",                         ST_MessageQTest },
   { "SemaphoreTest",                        ST_SemaphoreTest },
   { "GetNextHandleTest",                    RBU_GetNextHandleTest },
   { "TaskInitTest",                         RBU_TaskInitTest },
   { "RBUApplicationInitialiseTest",         RBU_RBUApplicationInitialiseTest },
   { "RBUApplicationFireSignalReqTest",      RBU_RBUApplicationFireSignalReqTest },
   { "RBUApplicationAlarmSignalReqTest",     RBU_RBUApplicationAlarmSignalReqTest },
   { "RBUApplicationLogonReqTest",           RBU_RBUApplicationLogonReqTest },
   { "RBUApplicationTaskProcessMessageTest", RBU_RBUApplicationTaskProcessMessageTest },
   { "RBUApplicationProcessOutputMessageTest",     MM_RBUApplicationProcessOutputMessageTest },

   CU_TEST_INFO_NULL,
};



/* Private Variables
*************************************************************************************/

/* stubs for test
*************************************************************************************/
#include "MessageQueueStub.h"




typedef struct
{
   void(*pFireSignalInd)(const uint32_t Handle, const uint8_t RUChannelIndex, const uint8_t SensorValue, const uint16_t SourceAddress);
   void(*pFireSignalCnf)(uint32_t Handle, uint32_t Status);
   void(*pAlarmSignalInd)(const uint32_t Handle, const uint8_t RUChannelIndex, const uint8_t SensorValue, const uint16_t SourceAddress);
   void(*pAlarmSignalCnf)(const uint32_t Handle, const uint32_t Status);
   void(*pOutputSignalInd)(const uint32_t Handle, const uint8_t OutputProfile, const uint8_t OutputsActivated);
   void(*pOutputSignalCnf)(const uint32_t Handle, const uint32_t Status);
   void(*pLogonInd)(const uint32_t Handle, const uint32_t SerialNumber, const uint8_t DeviceCombination, const uint16_t StatusFlags, const uint16_t SourceAddress);
   void(*pLogonCnf)(const uint32_t Handle, const uint32_t Status);
} MeshAPIConfig_t;

MeshAPIConfig_t callbacks;

void RBU_TEST_MeshAPIInit(void)
{
   callbacks.pFireSignalInd = NULL;
   callbacks.pFireSignalCnf = NULL;
   callbacks.pAlarmSignalInd = NULL;
   callbacks.pAlarmSignalCnf = NULL;
   callbacks.pAlarmSignalCnf = NULL;
   callbacks.pOutputSignalInd = NULL;
   callbacks.pOutputSignalCnf = NULL;
   callbacks.pLogonInd = NULL;
   callbacks.pLogonCnf = NULL;
}

void MM_MeshAPIInit(MeshAPIConfig_t *pConfig)
{
   callbacks.pFireSignalInd = pConfig->pFireSignalInd;
   callbacks.pFireSignalCnf = pConfig->pFireSignalCnf;
   callbacks.pAlarmSignalInd = pConfig->pAlarmSignalInd;
   callbacks.pAlarmSignalCnf = pConfig->pAlarmSignalCnf;
   callbacks.pAlarmSignalCnf = pConfig->pAlarmSignalCnf;
   callbacks.pOutputSignalInd = pConfig->pOutputSignalInd;
   callbacks.pOutputSignalCnf = pConfig->pOutputSignalCnf;
   callbacks.pLogonInd = pConfig->pLogonInd;
   callbacks.pLogonCnf = pConfig->pLogonCnf;
}


bool MM_MeshAPIFireSignalReq(const uint32_t Handle, const uint8_t RUChannelIndex, const uint8_t SensorValue)
{
   bFireSignalReqCalled = true;

   if (bFireSignalReqFail)
   {
      return false;
   }

   Test_Handle = Handle;
   Test_RUChannelIndex = RUChannelIndex;
   Test_SensorValue = SensorValue;

   return true;
}

bool MM_MeshAPIAlarmSignalReq(const uint32_t Handle, const uint8_t RUChannelIndex, const uint8_t SensorValue)
{
   bAlarmSignalReqCalled = true;

   if (bAlarmSignalReqFail)
   {
      return false;
   }

   Test_Handle = Handle;
   Test_RUChannelIndex = RUChannelIndex;
   Test_SensorValue = SensorValue;

   return true;
}

bool MM_MeshAPIOutputSignalReq(const uint32_t Handle, const uint8_t OutputProfile, const uint8_t OutputsActivated, const uint16_t Destination)
{
   bOutputSignalReqCalled = true;

   if (bOutputSignalReqFail)
   {
      return false;
   }

   Test_Handle = Handle;
   Test_OutputProfile = OutputProfile;
   Test_OutputsActivated = OutputsActivated;
   Test_Destination = Destination;

   return true;
}

bool MM_MeshAPILogonReq(const uint32_t Handle, const uint32_t SerialNumber, const uint8_t DeviceCombination, const uint16_t ZoneNumber)
{
   bLogonReqCalled = true;

   if (bLogonReqFail)
   {
      return false;
   }

   Test_Handle = Handle;
   Test_SerialNumber = SerialNumber;
   Test_DeviceCombination = DeviceCombination;
   Test_ZoneNumber = ZoneNumber;

   return true;
}

bool MM_MeshAPIResponseReq(const uint32_t Handle, const uint16_t Source, const uint16_t Destination, const uint8_t CommandType,
                           const uint8_t Parameter1, const uint8_t Parameter2, const uint32_t Value, const uint8_t ReadWrite)
{
   bResponseReqCalled = true;

   if (bResponseReqFail)
   {
      return false;
   }

   Test_Response.CommandType = CommandType;
   Test_Response.Destination = Destination;
   Test_Response.Parameter1 = Parameter1;
   Test_Response.Parameter2 = Parameter2;
   Test_Response.ReadWrite = ReadWrite;
   Test_Response.Source = Source;
   Test_Response.TransactionID = Handle;
   Test_Response.Value = Value;

   return true;
}

bool MM_MeshAPICommandReq(const uint32_t Handle, const uint16_t Source, const uint16_t Destination, const uint8_t CommandType,
                           const uint8_t Parameter1, const uint8_t Parameter2, const uint32_t Value, const uint8_t ReadWrite)
{
   bCommandReqCalled = true;

   if (bCommandReqFail)
   {
      return false;
   }

   Test_Command.CommandType = CommandType;
   Test_Command.Destination = Destination;
   Test_Command.Parameter1 = Parameter1;
   Test_Command.Parameter2 = Parameter2;
   Test_Command.ReadWrite = ReadWrite;
   Test_Command.Source = Source;
   Test_Command.TransactionID = Handle;
   Test_Command.Value = Value;

   return true;
}

bool MM_MeshAPIGenerateTestMessageReq(const char* pText, const uint16_t count)
{
   bTestMessageReqCalled = true;

   if (bTestMessageReqFail)
   {
      return false;
   }

   return true;
}

bool MM_MeshAPIGenerateRBUDisableReq(const uint32_t Handle, const uint16_t unitAddress)
{
   bRBUDisableReqCalled = true;

   if (bRBUDisableReqFail)
   {
      return false;
   }

   return true;
}

//Not tested.  Debug function only
bool MM_MeshAPIGenerateRBUReportNodesReq(void)
{
   return true;
}

bool MM_MeshAPIGenerateStatusSignalReq(const uint32_t Handle, const uint8_t Event, const uint16_t EventNodeId, const bool NcuRequested)
{
   bGenerateStatusSignalReqCalled = true;

   if (bGenerateStatusSignalReqFail)
   {
      return false;
   }

   return true;
}

bool MM_MeshAPIGenerateSetStateReq(const CO_State_t state)
{
   bGenerateSetStateReqCalled = true;

   if (bGenerateSetStateReqFail)
   {
      return false;
   }

   return true;
}

bool MM_MeshAPIGenerateFaultSignalReq(const uint32_t Handle, const uint16_t Zone, const uint16_t StatusFlags, const uint8_t DetectorFault, const uint8_t BeaconFault, const uint8_t SounderFault)
{
   bGenerateFaultSignalReqCalled = true;
   Test_ZoneNumber = Zone;
   Test_StatusFlags = StatusFlags;

   if (bGenerateFaultSignalReqFail)
   {
      return false;
   }

   return true;
}

bool MM_MeshAPIGenerateOutputStateReq(const uint32_t Handle, const uint16_t NodeAddress)
{
   return true;
}

bool MM_MeshAPIGenerateOutputStateMessage(const uint32_t Handle, const uint8_t outputs)
{
   return true;
}

/* validate the stubbed implementation of memory pools and message queues
*************************************************************************************/

/*************************************************************************************/
/**
* ST_MemPoolTest
* Test the test harness memory pool defined in MessageQueueStub.h
*
* no params
*
* @return - none

*/
void ST_MemPoolTest(void)
{
   AppQ = osMessageCreate(osMessageQ(AppQ), NULL);
   AppPool = osPoolCreate((const osPoolDef_t *)osPool(AppPool));
   ATHandleQ = osMessageCreate(osMessageQ(ATHandleQ), NULL);
   ATHandlePool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));

   
   // Test mem pool
   //Pool Size
   uint32_t poolSize = osPoolSize(osPool(AppPool));
   CU_ASSERT_EQUAL(poolSize, APP_POOL_SIZE);
   // Pool Free Space
   uint32_t poolSpace = osPoolFreeSpace(AppPool);
   CU_ASSERT_EQUAL(poolSpace, poolSize);

   //Pool Alloc
   CO_Message_t* pFireSignalReq = NULL;
   pFireSignalReq = osPoolAlloc(AppPool);
   CU_ASSERT_TRUE(pFireSignalReq != NULL);
   uint32_t block_count = osPoolBlocksAllocatedCount(AppPool);
   CU_ASSERT_EQUAL(1, block_count);

   osStatus osStat = osErrorOS;
   CO_RBUSensorData_t SensorData;

   if (NULL != pFireSignalReq)
   {
      SensorData.SensorType = CO_FIRE_CALLPOINT_E;
      SensorData.SensorValue = 0x77;
      pFireSignalReq->Type = CO_MESSAGE_GENERATE_FIRE_SIGNAL_E;
      memcpy(pFireSignalReq->Payload.PhyDataReq.Data, &SensorData, sizeof(CO_RBUSensorData_t));
   }

   // Check Pool Contents
   CO_Message_t* pMessage = NULL;
   pMessage = (CO_Message_t*)osPoolEntry(AppPool, 0);
   CU_ASSERT_TRUE(pMessage != NULL);
   CU_ASSERT_EQUAL(CO_MESSAGE_GENERATE_FIRE_SIGNAL_E, pMessage->Type);
   CO_RBUSensorData_t* pHeadData = (CO_RBUSensorData_t*)pMessage->Payload.PhyDataReq.Data;
   CU_ASSERT_TRUE(pHeadData != NULL);
   CU_ASSERT_EQUAL(CO_FIRE_CALLPOINT_E, pHeadData->SensorType);
   CU_ASSERT_EQUAL(0x77, pHeadData->SensorValue);

   //Create a second entry
   CO_Message_t* pFireSignalReq2 = NULL;
   pFireSignalReq2 = osPoolAlloc(AppPool);
   CU_ASSERT_TRUE(pFireSignalReq2 != NULL);
   block_count = osPoolBlocksAllocatedCount(AppPool);
   CU_ASSERT_EQUAL(2, block_count);

   CO_RBUSensorData_t SensorData2;

   if (NULL != pFireSignalReq2)
   {
      SensorData2.SensorType = CO_FIRST_AID_CALLPOINT_E;
      SensorData2.SensorValue = 0x5a;
      pFireSignalReq2->Type = CO_MESSAGE_GENERATE_OUTPUT_SIGNAL_E;
      memcpy(pFireSignalReq2->Payload.PhyDataReq.Data, &SensorData2, sizeof(CO_RBUSensorData_t));
   }

   // Check Pool Contents
   CO_Message_t* pMessage2 = NULL;
   pMessage2 = (CO_Message_t*)osPoolEntry(AppPool, 1);
   CU_ASSERT_TRUE(pMessage2 != NULL);
   CU_ASSERT_EQUAL(CO_MESSAGE_GENERATE_OUTPUT_SIGNAL_E, pMessage2->Type);
   CO_RBUSensorData_t* pHeadData2 = (CO_RBUSensorData_t*)pMessage2->Payload.PhyDataReq.Data;
   CU_ASSERT_TRUE(pHeadData2 != NULL);
   CU_ASSERT_EQUAL(CO_FIRST_AID_CALLPOINT_E, pHeadData2->SensorType);
   CU_ASSERT_EQUAL(0x5a, pHeadData2->SensorValue);

   // test mem pool dealloc
   osStatus status;
   status = osPoolFree(AppPool, pFireSignalReq);
   CU_ASSERT_EQUAL(osOK, status);
   block_count = osPoolBlocksAllocatedCount(AppPool);
   CU_ASSERT_EQUAL(1, block_count);
   CU_ASSERT_EQUAL(OSPOOL_UNUSED, pMessage->Type);

   status = osPoolFree(AppPool, pFireSignalReq2);
   CU_ASSERT_EQUAL(osOK, status);
   poolSpace = osPoolFreeSpace(AppPool);
   CU_ASSERT_EQUAL(poolSpace, poolSize);
   CU_ASSERT_EQUAL(OSPOOL_UNUSED, pMessage->Type);

   // Fill the pool and check that allocations fail
   CO_Message_t* pFireSignals[APP_POOL_SIZE+1];
   uint32_t count;
   for ( count = 0; count < APP_POOL_SIZE; count++ )
   {
      pFireSignals[count] = osPoolAlloc(AppPool);
      CU_ASSERT_TRUE(pFireSignals[count] != NULL);
      block_count = osPoolBlocksAllocatedCount(AppPool);
      CU_ASSERT_EQUAL((count + 1), block_count);
   }

   pFireSignals[count] = osPoolAlloc(AppPool);
   CU_ASSERT_TRUE(pFireSignals[count] == NULL);
   poolSpace = osPoolFreeSpace(AppPool);
   CU_ASSERT_EQUAL(0, poolSpace);

   // release all
   for (count = 0; count < APP_POOL_SIZE; count++)
   {
      status = osPoolFree(AppPool, pFireSignals[count]);
      CU_ASSERT_EQUAL(osOK, status);
   }
   poolSpace = osPoolFreeSpace(AppPool);
   CU_ASSERT_EQUAL(APP_POOL_SIZE, poolSpace);
}

/*************************************************************************************/
/**
* ST_MessageQTest
* Test the test harness message queue defined in MessageQueueStub.h
*
* no params
*
* @return - none

*/
void ST_MessageQTest(void)
{
   //Put a fire signal into the message queue
   CO_Message_t* pFireSignalReq = NULL;
   pFireSignalReq = osPoolAlloc(AppPool);
   CU_ASSERT_TRUE(pFireSignalReq != NULL);

   osStatus osStat = osErrorOS;
   CO_RBUSensorData_t SensorData;

   osMessageQFlush(AppQ);
   CU_ASSERT_EQUAL(0, osMessageCount(AppQ));

   if (NULL != pFireSignalReq)
   {
      //populate the message and put it into the queue
      SensorData.SensorType = CO_FIRE_CALLPOINT_E;
      SensorData.SensorValue = 0x77;
      pFireSignalReq->Type = CO_MESSAGE_GENERATE_FIRE_SIGNAL_E;
      memcpy(pFireSignalReq->Payload.PhyDataReq.Data, &SensorData, sizeof(CO_RBUSensorData_t));
      osStat = osMessagePut(AppQ, (uint32_t)pFireSignalReq, 0);
      CU_ASSERT_EQUAL(osOK, osStat);
      CU_ASSERT_EQUAL(1, osMessageCount(AppQ));

      if (osOK != osStat)
      {
         /* failed to write */
         osPoolFree(AppPool, pFireSignalReq);
         pFireSignalReq = NULL;
      }
      else
      {
         //The message was sucessfully added to the queue.
         //Now read it back.
         osEvent fireEvent;
         fireEvent.status = osErrorOS;
         fireEvent = osMessageGet(AppQ, 100);
         CU_ASSERT_EQUAL(osEventMessage, fireEvent.status);
         CO_Message_t* pMessage = (CO_Message_t*)fireEvent.value.p;
         CU_ASSERT_TRUE(pMessage != NULL);
         if (pMessage)
         {
            //We got a message back. Verify the contents.
            CU_ASSERT_EQUAL(CO_MESSAGE_GENERATE_FIRE_SIGNAL_E, pMessage->Type);
            CO_RBUSensorData_t* pRxHeadData;
            pRxHeadData = (CO_RBUSensorData_t*)pMessage->Payload.PhyDataReq.Data;
            CU_ASSERT_TRUE(pRxHeadData != NULL);
            if (pRxHeadData)
            {
               CU_ASSERT_EQUAL(CO_FIRE_CALLPOINT_E, pRxHeadData->SensorType);
               CU_ASSERT_EQUAL(0x77, pRxHeadData->SensorValue);
            }

            //Release the memory pool slot
            osStat = osPoolFree(AppPool, pMessage);
            CU_ASSERT_EQUAL(osOK, osStat);
         }
      }
   }

   // Test for full queue.
   //Check that the queue is empty
   uint32_t messageCount = osMessageCount(AppQ);
   CU_ASSERT_EQUAL(0, messageCount);

   //create a new message and repeatedly write it to the queue.
   pFireSignalReq = osPoolAlloc(AppPool);
   CU_ASSERT_TRUE(pFireSignalReq != NULL);
   if (NULL != pFireSignalReq)
   {
      SensorData.SensorType = CO_FIRE_CALLPOINT_E;
      SensorData.SensorValue = 0x77;
      pFireSignalReq->Type = CO_MESSAGE_GENERATE_FIRE_SIGNAL_E;
      memcpy(pFireSignalReq->Payload.PhyDataReq.Data, &SensorData, sizeof(CO_RBUSensorData_t));

      uint32_t queueSize = osMessageQSize(AppQ);
      for (uint32_t msgNumber = 1; msgNumber <= queueSize; msgNumber++)
      {
         osStat = osMessagePut(AppQ, (uint32_t)pFireSignalReq, 0);
         CU_ASSERT_EQUAL(osOK, osStat);
         messageCount = osMessageCount(AppQ);
         CU_ASSERT_EQUAL(msgNumber, messageCount);
      }
      // The queue is now full, one more should result in a osErrorResource error
      osStat = osMessagePut(AppQ, (uint32_t)pFireSignalReq, 0);
      CU_ASSERT_EQUAL(osErrorResource, osStat);

      //read the messsages back out and check that the queue length reduces
      osEvent fireEvent;
      for (int msgNumber = queueSize-1; msgNumber >= 0; msgNumber--)
      {
         fireEvent.status = osErrorOS;
         fireEvent = osMessageGet(AppQ, 100);
         CU_ASSERT_EQUAL(osEventMessage, fireEvent.status);
         messageCount = osMessageCount(AppQ);
         CU_ASSERT_EQUAL(msgNumber, messageCount);
      }
      // check that the queue is empty
      messageCount = osMessageCount(AppQ);
      CU_ASSERT_EQUAL(0, messageCount);

      //Release the memory pool slot
      osStat = osPoolFree(AppPool, pFireSignalReq);
      CU_ASSERT_EQUAL(osOK, osStat);
   }
}

/*************************************************************************************/
/**
* ST_SemaphoreTest
* Test the test harness semaphore stub defined in MessageQueueStub.h
*
* no params
*
* @return - none

*/
void ST_SemaphoreTest(void)
{
   //create semaphore
   HeadInterfaceSemaphoreId = osSemaphoreCreate(osSemaphore(HeadInterfaceSemaphore), 1);

   uint32_t one_tenth_second = (100*CLOCKS_PER_SEC)/1000 ;
   uint32_t two_tenths_second = (200*CLOCKS_PER_SEC) /1000;

   // test the semaphore take and release by starting a background thread to 
   // release the semaphore in one second, then calling osSemaphoreWait with a 
   // two second timeout.  the function should return after one second if
   //all worked as expected.

   //create a test_sem_release_t to pass to the background thread
   static test_sem_release_t sem_data;
   sem_data.sem = HeadInterfaceSemaphoreId;
   sem_data.timeout = one_tenth_second;


   //start the background thread
   _beginthread(ReleaseSem, 1024, (void*)&sem_data);

   //check the timeout works on osSemaphoreWait
   clock_t start = clock();
   uint32_t result = osSemaphoreWait(HeadInterfaceSemaphoreId, one_tenth_second);
   clock_t stop = clock();
   uint32_t time_delta = stop - start;

   //Check that the semaphore was not released (0 should be returned).
   CU_ASSERT_EQUAL(0, result);

   // test the release function return values
   osStatus status = osSemaphoreRelease(NULL);
   CU_ASSERT_EQUAL(osErrorResource, status);
   status = osSemaphoreRelease(HeadInterfaceSemaphoreId);
   CU_ASSERT_EQUAL(osOK, status);

}

/*************************************************************************************/
/**
* ReleaseSem
* Release a semaphore after a delay
*
* @param sem_data    Pointer to a test_sem_release_t containing the semaphore ID and 
*                    the timeout to use.
*                    Passed as a void* so that this can be run as a background thread.
*
* @return - none

*/
void ReleaseSem(void* sem_data)
{
   test_sem_release_t* pSem_data = (test_sem_release_t*)sem_data;

   if (pSem_data)
   {
      clock_t start = clock();
      clock_t end_time = start + pSem_data->timeout;
      while (end_time > clock())
      {
      }
      osSemaphoreRelease(pSem_data->sem);
   }
}



/* CUnit tests for MM_RBUApplicationTask
*************************************************************************************/

/*************************************************************************************/
/**
* RBU_GetNextHandleTest
* Test the message handle generation.  The returned value should increment for each call.
*
* no params
*
* @return - none

*/
void RBU_GetNextHandleTest(void)
{

   LastHandle = GetNextHandle();

   uint32_t handle = GetNextHandle();
   CU_ASSERT_EQUAL(handle, (LastHandle + 1));
   handle = GetNextHandle();
   CU_ASSERT_EQUAL(handle, (LastHandle + 2));
   handle = GetNextHandle();
   CU_ASSERT_EQUAL(handle, (LastHandle + 3));
   LastHandle = handle;
}

/*************************************************************************************/
/**
* RBU_TaskInitTest
* Test the task init function
*
* no params
*
* @return - none

*/
void RBU_TaskInitTest(void)
{
   char bank_1_version_string[] = "11.22.33";
   char bank_2_version_string[] = "44.55.66";

   gBank1VersionStringLocation = (uint32_t)&bank_1_version_string;
   gBank2VersionStringLocation = (uint32_t)&bank_2_version_string;

   bThreadInitCalled = false;
   bThreadInitFail = false;
   int32_t result = MM_RBUApplicationTaskInit(1, 34, 1111, 1);
   CU_ASSERT_EQUAL(0, result);
   CU_ASSERT_TRUE(bThreadInitCalled);

   bThreadInitCalled = false;
   bThreadInitFail = true;
   result = MM_RBUApplicationTaskInit(1,34, 1111, 1);
   CU_ASSERT_EQUAL(-1, result);
   CU_ASSERT_TRUE(bThreadInitCalled);
}

/*************************************************************************************/
/**
* RBU_RBUApplicationInitialiseTest
* Test the Mesh API initialisation
*
* no params
*
* @return - none

*/
void RBU_RBUApplicationInitialiseTest(void)
{
   RBU_TEST_MeshAPIInit();

   MM_RBUApplicationInitialise();
}

/*************************************************************************************/
/**
* RBU_RBUApplicationFireSignalReqTest
* Test function MM_RBUApplicationFireSignalReq 
*
* no params
*
* @return - none

*/
void RBU_RBUApplicationFireSignalReqTest(void)
{
   CO_RBUSensorData_t SensorData;

   SensorData.Handle = 0;
   SensorData.SensorType = CO_FIRE_CALLPOINT_E;
   SensorData.RUChannelIndex = 2;
   SensorData.SensorValue = 0x01;
   SensorData.HopCount = 0;

   /* NULL param test */
   bFireSignalReqCalled = false;
   MM_RBUApplicationFireSignalReq(NULL);
   CU_ASSERT_FALSE(bFireSignalReqCalled);

   /* valid data test */
   Test_Handle = 0;
   Test_RUChannelIndex = 0;
   Test_SensorValue = 0;
   LastHandle = GetNextHandle();

   bFireSignalReqCalled = false;
   bFireSignalReqFail = false;
   MM_RBUApplicationFireSignalReq(&SensorData);
   CU_ASSERT_TRUE(bFireSignalReqCalled);
   CU_ASSERT_EQUAL(Test_Handle, (LastHandle+1));
   CU_ASSERT_EQUAL(2, Test_RUChannelIndex);
   CU_ASSERT_EQUAL(1, Test_SensorValue);

   LastHandle = Test_Handle;

   /* valid data but MeshAPI call MM_MeshAPIFireSignalReq returns false */
   Test_Handle = 0;
   Test_RUChannelIndex = 0;
   Test_SensorValue = 0;
   LastHandle = GetNextHandle();

   bFireSignalReqCalled = false;
   bFireSignalReqFail = true;
   MM_RBUApplicationFireSignalReq(&SensorData);
   CU_ASSERT_TRUE(bFireSignalReqCalled);
   CU_ASSERT_EQUAL(0, Test_Handle);
   CU_ASSERT_EQUAL(0, Test_RUChannelIndex);
   CU_ASSERT_EQUAL(0, Test_SensorValue);
}



/*************************************************************************************/
/**
* RBU_RBUApplicationAlarmSignalReqTest
* Test function MM_RBUApplicationAlarmSignalReq
*
* no params
*
* @return - none

*/
void RBU_RBUApplicationAlarmSignalReqTest(void)
{
   CO_RBUSensorData_t SensorData;

   SensorData.Handle = 0;
   SensorData.SensorType = CO_FIRST_AID_CALLPOINT_E;
   SensorData.RUChannelIndex = 5;
   SensorData.SensorValue = 0x07;
   SensorData.HopCount = 0;

   /* NULL param test */
   bAlarmSignalReqCalled = false;
   MM_RBUApplicationAlarmSignalReq(NULL);
   CU_ASSERT_FALSE(bAlarmSignalReqCalled);

   /* valid data test */
   Test_RUChannelIndex = 0;
   Test_SensorValue = 0;
   LastHandle = GetNextHandle();

   bAlarmSignalReqCalled = false;
   bAlarmSignalReqFail = false;
   MM_RBUApplicationAlarmSignalReq(&SensorData);
   CU_ASSERT_TRUE(bAlarmSignalReqCalled);
   CU_ASSERT_EQUAL(Test_Handle, (LastHandle+1));
   CU_ASSERT_EQUAL(5, Test_RUChannelIndex);
   CU_ASSERT_EQUAL(7, Test_SensorValue);

   /* valid data but MeshAPI call MM_MeshAPIAlarmSignalReq returns false */
   Test_RUChannelIndex = 0;
   Test_SensorValue = 0;
   Test_Handle = 0;

   bAlarmSignalReqCalled = false;
   bAlarmSignalReqFail = true;
   MM_RBUApplicationAlarmSignalReq(&SensorData);
   CU_ASSERT_TRUE(bAlarmSignalReqCalled);
   CU_ASSERT_EQUAL(0, Test_Handle);
   CU_ASSERT_EQUAL(0, Test_RUChannelIndex);
   CU_ASSERT_EQUAL(0, Test_SensorValue);
}



/*************************************************************************************/
/**
* RBU_RBUApplicationLogonReqTest
* Test function MM_RBUApplicationLogonReq
*
* no params
*
* @return - none

*/
void RBU_RBUApplicationLogonReqTest(void)
{
   /* valid data test */
   Test_SerialNumber = 1;
   Test_DeviceCombination = 1;
   Test_ZoneNumber = 1;
   uint8_t eeprom_addr[NV_MAX_PARAM_ID_E*4];

   Test_Handle = 0;
   bLogonReqCalled = false;
   bLogonReqFail = false;
   SetEepromAddress(&eeprom_addr[0]);
   EE_WriteVariable32bits(DM_NVM_ParameterVirtualAddr[NV_UNIT_SERIAL_NO_E], 1234);
   EE_WriteVariable32bits(DM_NVM_ParameterVirtualAddr[NV_DEVICE_COMBINATION_E], 21);
   gZoneNumber = 17;
   eeprom_return_value = EE_OK;

   LastHandle = GetNextHandle();

   MM_RBUApplicationLogonReq();
   CU_ASSERT_TRUE(bLogonReqCalled);
   CU_ASSERT_EQUAL(Test_Handle, LastHandle+1);
   CU_ASSERT_EQUAL(1234, Test_SerialNumber);
   CU_ASSERT_EQUAL(21, Test_DeviceCombination);
   CU_ASSERT_EQUAL(17, Test_ZoneNumber);

   /* valid data but MeshAPI call LogonReq returns false */
   Test_SerialNumber = 1;
   Test_DeviceCombination = 1;
   Test_ZoneNumber = 1;
   Test_Handle = 0;

   bLogonReqCalled = false;
   bLogonReqFail = true;
   MM_RBUApplicationLogonReq();
   CU_ASSERT_TRUE(bLogonReqCalled);
   CU_ASSERT_EQUAL(0, Test_Handle);
   CU_ASSERT_EQUAL(1, Test_SerialNumber);
   CU_ASSERT_EQUAL(1, Test_DeviceCombination);
   CU_ASSERT_EQUAL(1, Test_ZoneNumber);
}


/*************************************************************************************/
/**
* RBU_RBUApplicationTaskProcessMessageTest
* Test function MM_RBUApplicationTaskProcessMessage
*
* no params
*
* @return - none

*/
void RBU_RBUApplicationTaskProcessMessageTest(void)
{
   const uint32_t timeout_ms = 10;
   /* Test with no messages waiting in the queue */
   bFireSignalReqCalled = false;
   bAlarmSignalReqCalled = false;
   bOutputSignalReqCalled = false;
   bLogonReqCalled = false;
   bosMessageGetCalled = false;

   MM_RBUApplicationTaskProcessMessage(timeout_ms);
   CU_ASSERT_TRUE(bosMessageGetCalled);
   CU_ASSERT_FALSE(bFireSignalReqCalled);
   CU_ASSERT_FALSE(bAlarmSignalReqCalled);
   CU_ASSERT_FALSE(bOutputSignalReqCalled);
   CU_ASSERT_FALSE(bLogonReqCalled);

   /* Add a fire signal message */
   osStatus osStat = osErrorOS;
   CO_Message_t *pFireSignalReq = NULL;
   CO_RBUSensorData_t SensorData;

   /* create OutputSignalReq message and put into mesh queue */
   pFireSignalReq = osPoolAlloc(AppPool);
   if (NULL != pFireSignalReq)
   {
      SensorData.SensorType = CO_FIRE_CALLPOINT_E;
      SensorData.RUChannelIndex = 2;
      SensorData.SensorValue = 1;
      pFireSignalReq->Type = CO_MESSAGE_GENERATE_FIRE_SIGNAL_E;
      memcpy(pFireSignalReq->Payload.PhyDataReq.Data, &SensorData, sizeof(CO_RBUSensorData_t));
      osStat = osMessagePut(AppQ, (uint32_t)pFireSignalReq, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         osPoolFree(AppPool, pFireSignalReq);
      }
   }

   bFireSignalReqCalled = false;
   bAlarmSignalReqCalled = false;
   bOutputSignalReqCalled = false;
   bLogonReqCalled = false;
   bosMessageGetCalled = false;
   bFireSignalReqFail = false;
   LastHandle = GetNextHandle();

   //check that one msg pool object is allocated and one message is queued
   CU_ASSERT_EQUAL(1, osPoolBlocksAllocatedCount(AppPool));
   CU_ASSERT_EQUAL(1, osMessageCount(AppQ));

   //Process the queued message
   MM_RBUApplicationTaskProcessMessage(timeout_ms);
   CU_ASSERT_TRUE(bFireSignalReqCalled);
   CU_ASSERT_EQUAL(Test_Handle, (LastHandle + 1));
   CU_ASSERT_EQUAL(2, Test_RUChannelIndex);
   CU_ASSERT_EQUAL(1, Test_SensorValue);

   //check that the resources were released
   CU_ASSERT_EQUAL(0, osPoolBlocksAllocatedCount(AppPool));
   CU_ASSERT_EQUAL(0, osMessageCount(AppQ));

   // check for alarm signal
}


/*************************************************************************************/
/**
* MM_RBUApplicationProcessOutputMessageTest
* Test function MM_RBUApplicationProcessOutputMessage
*
* no params
*
* @return - none

*/
void MM_RBUApplicationProcessOutputMessageTest(void)
{
   AppQ = osMessageCreate(osMessageQ(AppQ), NULL);
//  MM_RBUApplicationOutputSignalIndCb(0, 1, 2);

/* Send the output request up to the Application */
   ApplicationMessage_t appMessage;
   appMessage.MessageType = APP_MSG_TYPE_OUTPUT_SIGNAL_E;
   OutputSignalIndication_t outData;
   outData.OutputDuration = 0;
   outData.zone = 1;
   outData.OutputProfile = CO_PROFILE_FIRE_E;
   outData.OutputsActivated = 255;
   memcpy(appMessage.MessageBuffer, &outData, sizeof(OutputSignalIndication_t));

   SM_SendEventToApplication(CO_MESSAGE_PHY_DATA_IND_E, &appMessage);

   CU_ASSERT_EQUAL(1, osMessageCount(AppQ));
   extern ApplicationChildArray_t childList;
   //extern uint16_t gZoneNumber;

   //clear the message from the queue and AppPool or it messes up later tests.
   osEvent Event = osMessageGet(AppQ, 0);
   CU_ASSERT_EQUAL(0, osMessageCount(AppQ));
   osStatus status = osPoolFree(AppPool, Event.value.p);
   CU_ASSERT_EQUAL(osOK, status);
   CU_ASSERT_EQUAL(0, osPoolBlocksAllocatedCount(AppPool));

   gZoneNumber = 2;

   /* set up a child in zone 3 */
   childList.childRecord[0].nodeID = 3;
   childList.childRecord[0].zone = ZONE_GLOBAL;
   childList.childRecord[0].lastReportedState[CO_PROFILE_FIRE_E] = 0;
   childList.childRecord[0].lastCommandedState[CO_PROFILE_FIRE_E] = 0;
   childList.childRecord[0].isPrimaryChild = true;

   
   MM_RBUApplicationProcessOutputMessage(&outData);
   CU_ASSERT_EQUAL(childList.childRecord[0].lastCommandedState[CO_PROFILE_FIRE_E], 0x00);

   childList.childRecord[0].zone = 2;
   MM_RBUApplicationProcessOutputMessage(&outData);
   CU_ASSERT_EQUAL(childList.childRecord[0].lastCommandedState[CO_PROFILE_FIRE_E], 0x00);

   childList.childRecord[0].zone = 1;
   MM_RBUApplicationProcessOutputMessage(&outData);
   CU_ASSERT_EQUAL(childList.childRecord[0].lastCommandedState[CO_PROFILE_FIRE_E], 0xff);

}
