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
*  File         : STNCUApplicationTest.c
*
*  Description  : Implementation for the NCU Application tests
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
#include "MM_ApplicationCommon.h"
#include "MM_NCUApplicationStub.h"
#include "gpio.h"
#include "STNCUApplicationTest.h"


/* Private Functions Prototypes
*************************************************************************************/
static void NCU_GetNextHandleTest(void);
static void NCU_TaskInitTest(void);


/* Global Variables
*************************************************************************************/
static uint32_t LastHandle = 0;


/* Externs
*************************************************************************************/
extern void SM_SendEventToApplication(const CO_MessageType_t MeshEventType, const ApplicationMessage_t* pEventMessage);


/* Table containing the test settings */
CU_TestInfo ST_NCUAppTests[] =
{
   { "GetNextHandle",                        NCU_GetNextHandleTest },
   { "TaskInitTest",                         NCU_TaskInitTest },

   CU_TEST_INFO_NULL,
};



/* Private Variables
*************************************************************************************/

/* stubs for test
*************************************************************************************/
#include "MessageQueueStub.h"

extern bool MM_MeshAPIFireSignalReq(const uint32_t Handle, const uint8_t RUChannelIndex, const uint8_t SensorValue);
extern bool MM_MeshAPIAlarmSignalReq(const uint32_t Handle, const uint8_t RUChannelIndex, const uint8_t SensorValue);
extern bool MM_MeshAPIOutputSignalReq(const uint32_t Handle, const uint8_t OutputProfile, const uint8_t OutputsActivated, const uint16_t Destination);
extern bool MM_MeshAPILogonReq(const uint32_t Handle, const uint32_t SerialNumber, const uint8_t DeviceCombination, const uint16_t StatusFlags);
extern bool MM_MeshAPIResponseReq(const uint32_t Handle, const uint16_t Source, const uint16_t Destination, const uint8_t CommandType,
   const uint8_t Parameter1, const uint8_t Parameter2, const uint32_t Value, const uint8_t ReadWrite);
extern bool MM_MeshAPICommandReq(const uint32_t Handle, const uint16_t Source, const uint16_t Destination, const uint8_t CommandType,
                                 const uint8_t Parameter1, const uint8_t Parameter2, const uint32_t Value, const uint8_t ReadWrite);
extern bool MM_MeshAPIGenerateTestMessageReq(const char* pText, const uint16_t count);
extern bool MM_MeshAPIGenerateRBUDisableReq(const uint32_t Handle, const uint16_t unitAddress);
extern bool MM_MeshAPIGenerateRBUReportNodesReq(void);
extern bool MM_MeshAPIGenerateStatusSignalReq(const uint32_t Handle, const uint8_t Event, const uint16_t EventNodeId, const bool NcuRequested);
extern bool MM_MeshAPIGenerateSetStateReq(const CO_State_t state);
extern int MM_NCUApplicationTaskInit(const uint16_t address, const uint16_t device_configuration, const uint32_t unitSerialNo, const uint32_t defaultZoneNumber);

/* CUnit tests for MM_NCUApplicationStubTask
*************************************************************************************/

/*************************************************************************************/
/**
* NCU_GetNextHandleTest
* Test the message handle generation.  The returned value should increment for each call.
*
* no params
*
* @return - none

*/
void NCU_GetNextHandleTest(void)
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
* NCU_TaskInitTest
* Test the task init function
*
* no params
*
* @return - none

*/
void NCU_TaskInitTest(void)
{
   bThreadInitCalled = false;
   bThreadInitFail = false;
   int32_t result = MM_NCUApplicationTaskInit(1, 34, 1111, 1);
   CU_ASSERT_EQUAL(0, result);
   CU_ASSERT_TRUE(bThreadInitCalled);

   bThreadInitCalled = false;
   bThreadInitFail = true;
   result = MM_NCUApplicationTaskInit(1,34, 1111, 1);
   CU_ASSERT_EQUAL(-1, result);
   CU_ASSERT_TRUE(bThreadInitCalled);
}


