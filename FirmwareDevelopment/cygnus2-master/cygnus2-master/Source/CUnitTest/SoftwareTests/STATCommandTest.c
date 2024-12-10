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
*  File         : STATCommandTest.c
*
*  Description  : Implementation for testing MM_ATCommand.c
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
#include "cmsis_os.h"
#include "lptim.h"
#include "CUnit.h"
#include "CO_Defines.h"
#include "CO_Message.h"
#include "CO_ErrorCode.h"
#include "MessageQueueStub.h"
#include "MM_ATCommand.h"


/* Private Functions Prototypes
*************************************************************************************/
static void MM_ATEncodeSerialNumberTest(void);
static void MM_ATDecodeSerialNumberTest(void);
static void MM_ATCommand_RBUDisableTest(void);
static void ATCommand_OutputSignalTest(void);

/* Externs
*************************************************************************************/
extern uint32_t gNetworkAddress;
extern bool MM_ATEncodeSerialNumber(const char* pParamBuffer, uint32_t *value);
extern bool MM_ATDecodeSerialNumber(const uint32_t value, char* pParamBuffer);
extern osMessageQId(AppQ);							      // Queue to the application
extern osPoolId AppPool;									// Pool definition for use with AppQ.

/* Table containing the test settings */
CU_TestInfo ST_ATCommandTests[] =
{
   { "MM_ATEncodeSerialNumberTest",    MM_ATEncodeSerialNumberTest },
   { "MM_ATDecodeSerialNumberTest",    MM_ATDecodeSerialNumberTest },
   { "MM_ATCommand_RBUDisableTest",    MM_ATCommand_RBUDisableTest },
   { "ATCommand_OutputSignalTest",     ATCommand_OutputSignalTest },

   CU_TEST_INFO_NULL,
};

/* Test utilities
*************************************************************************************/


/*************************************************************************************
* STUBBED FUNCTIONS
*************************************************************************************/
HAL_StatusTypeDef FLASH_WaitForLastOperation_return_value;
bool bFLASH_WaitForLastOperation_called = false;
HAL_StatusTypeDef FLASH_WaitForLastOperation(uint32_t Timeout)
{
   bFLASH_WaitForLastOperation_called = true;
   return FLASH_WaitForLastOperation_return_value;
}

bool bMM_SetTransparentMode_called = false;
void MM_SetTransparentMode(void)
{
   bMM_SetTransparentMode_called = true;
}

char strtok_r_return_value[256];
char *strtok_r(char * s1, const char * s2, char ** ptr)
{
   return strtok_r_return_value;
}

/*************************************************************************************
* TEST FUNCTIONS
*************************************************************************************/

/*************************************************************************************
* MM_ATEncodeSerialNumberTest
* Test the operation of function MM_ATEncodeSerialNumber;
*
* no params
*
* @return - none
*
*************************************************************************************/
void MM_ATEncodeSerialNumberTest(void)
{
   const char test_string_1[] = "1712-01-00001";
   uint32_t expected_1 = 0x23810001;
   const char test_string_2[] = "9901-31-65535";
   uint32_t expected_2 = 0xc63fffff;
   uint32_t encoded_value = 0;

   bool result = MM_ATEncodeSerialNumber(test_string_1, &encoded_value);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(encoded_value, expected_1);

   encoded_value = 0;
   result = MM_ATEncodeSerialNumber(test_string_2, &encoded_value);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(encoded_value, expected_2);

   /* negative tests */
   /* null param checks */
   result = MM_ATEncodeSerialNumber(NULL, &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber(test_string_1, NULL);
   CU_ASSERT_FALSE(result);

   /* invalid chars */
   result = MM_ATEncodeSerialNumber("A712-01-00001", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("1A12-01-00001", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("17A2-01-00001", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("171A-01-00001", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("1712-A1-00001", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("1712-0A-00001", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("1712-01-A0001", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("1712-01-0A001", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("1712-01-00A01", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("1712-01-000A1", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("1712-01-0000A", &encoded_value);
   CU_ASSERT_FALSE(result);
   /* bad format */
   result = MM_ATEncodeSerialNumber("1712-1-00001", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("1712-01-1", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("1712-01-000001", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("171-01-00001", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("1712.01-00001", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("1712-01 00001", &encoded_value);
   CU_ASSERT_FALSE(result);
   /* out of range values */
   result = MM_ATEncodeSerialNumber("1713-01-00001", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("1713-00-00001", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("1713-32-00001", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("1713-01-65536", &encoded_value);
   CU_ASSERT_FALSE(result);
   result = MM_ATEncodeSerialNumber("1713-01-00000", &encoded_value);
   CU_ASSERT_FALSE(result);

}

/*************************************************************************************
* MM_ATDecodeSerialNumberTest
* Test the operation of function MM_ATDecodeSerialNumber;
*
* no params
*
* @return - none
*
*************************************************************************************/
void MM_ATDecodeSerialNumberTest(void)
{
   uint32_t test_value_1 = 0x23810001;
   const char expected_string_1[] = "1712-01-00001";
   uint32_t test_value_2 = 0xc63fffff;
   const char expected_string_2[] = "9901-31-65535";
   char string_value[32];

   memset(string_value, 0, sizeof(string_value));
   bool result = MM_ATDecodeSerialNumber(test_value_1, string_value);
   CU_ASSERT_TRUE(result);
   int32_t compare_result = strcmp(string_value, expected_string_1);
   CU_ASSERT_EQUAL(compare_result, 0);

   memset(string_value, 0, sizeof(string_value));
   result = MM_ATDecodeSerialNumber(test_value_2, string_value);
   CU_ASSERT_TRUE(result);
   compare_result = strcmp(string_value, expected_string_2);
   CU_ASSERT_EQUAL(compare_result, 0);

   /* NULL parameter check */
   result = MM_ATDecodeSerialNumber(test_value_1, NULL);
   CU_ASSERT_FALSE(result);

}


/*************************************************************************************/
/**
* MM_ATCommand_RBUDisableTest
* Test the operation of function MM_ATCommand_RBUDisable;
*
* no params
*
* @return - none
*
*************************************************************************************/
void MM_ATCommand_RBUDisableTest(void)
{
   ATMessageStatus_t msgStatus;
   char nodeStr[] = "1";
   UartComm_t uart = DEBUG_UART;
   AppQ = osMessageCreate(osMessageQ(AppQ), NULL);
   AppPool = osPoolCreate((const osPoolDef_t *)osPool(AppPool));
   CO_Message_t * pMessage;
   CO_Message_t message;

   /* NULL parameter check */
   msgStatus = MM_ATCommand_RBUDisable(AT_COMMAND_WRITE,NULL, 0, uart,0);
   CU_ASSERT_EQUAL(msgStatus, AT_INVALID_PARAMETER);
   /* Empty buffer test */
   msgStatus = MM_ATCommand_RBUDisable(AT_COMMAND_WRITE, "", 0, uart, 0);
   CU_ASSERT_EQUAL(msgStatus, AT_INVALID_COMMAND);

   /*test response when the AppPool is full */
   /*fill the pool*/
   while (0 < osPoolFreeSpace(AppPool))
   {
      pMessage = osPoolAlloc(AppPool);
   }
   msgStatus = MM_ATCommand_RBUDisable(AT_COMMAND_WRITE, nodeStr, 0, uart, 0);
   CU_ASSERT_EQUAL(msgStatus, AT_QUEUE_ERROR);

   /* flush the AppPool for further testing */
   osPoolFlush(AppPool);

   /* test response when the AppQ is full */
   /* fill the AppQ */
   osStatus status = osOK;
   do
   {
      status = osMessagePut(AppQ, (uint32_t)&message, 0);
   } while (status == osOK);
   msgStatus = MM_ATCommand_RBUDisable(AT_COMMAND_WRITE, nodeStr, 0, uart, 0);
   CU_ASSERT_EQUAL(msgStatus, AT_QUEUE_ERROR);
   /* check that the AppPool was freed */
   CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(AppPool), 0);


   /* flush the AppQ for further testing */
   osMessageQFlush(AppQ);

   /* test with valid data */
   msgStatus = MM_ATCommand_RBUDisable(AT_COMMAND_WRITE, nodeStr, 0, uart, 0);
   CU_ASSERT_EQUAL(msgStatus, AT_NO_ERROR);
   /* there should be one allocated block in AppPool */
   CU_ASSERT_EQUAL(osPoolBlocksAllocatedCount(AppPool), 1);
   /* there should be a mesage in AppQ */
   CU_ASSERT_EQUAL(osMessageCount(AppQ), 1);

   /* Get the message from AppQ */
   osEvent event;
   event = osMessageGet(AppQ, 0);
   CU_ASSERT_EQUAL(osEventMessage, event.status);
   pMessage = (CO_Message_t*)event.value.p;
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      /* verify the message type */
      CU_ASSERT_EQUAL(pMessage->Type, CO_MESSAGE_GENERATE_RBU_DISABLE_MESSAGE_E);
      /* verify the message contents */
      CO_CommandData_t CmdMessage;
      memcpy((uint8_t*)&CmdMessage, pMessage->Payload.PhyDataReq.Data, sizeof(CO_CommandData_t));
      CU_ASSERT_EQUAL(CmdMessage.SourceUSART, (uint16_t)uart);
      CU_ASSERT_EQUAL(CmdMessage.Destination, 1);
   }
}

/*************************************************************************************/
/**
* ATCommand_OutputSignalTest
* Test the operation of function MM_ATCommand_compound for AT cmd to send an output signal
*
* no params
*
* @return - none
*
*************************************************************************************/
void ATCommand_OutputSignalTest()
{
   AppQ = osMessageCreate(osMessageQ(AppQ), NULL);
   AppPool = osPoolCreate((const osPoolDef_t *)osPool(AppPool));
   CO_Message_t * pMessage;
   char source_buffer[] = "Z2U1,3,4";
   uint8_t buffer_length = (uint8_t)strlen(source_buffer);

   /* flush the AppPool and AppQ before testing */
   osPoolFlush(AppPool);
   osMessageQFlush(AppQ);

   //Call the function MM_ATCommand_compound for an output signal type
   ATMessageStatus_t msg_status = MM_ATCommand_compound(AT_COMMAND_WRITE, source_buffer, buffer_length, DEBUG_UART,AT_COMPOUND_TYPE_OUTPUT_SIGNAL_E);
   CU_ASSERT_EQUAL(msg_status, AT_NO_ERROR);

   //Verify that the AppQ has got a message
   uint32_t used_mem_blocks = osPoolBlocksAllocatedCount(AppPool);
   CU_ASSERT_EQUAL(used_mem_blocks, 1);
   uint32_t app_msg_count = osMessageCount(AppQ);
   CU_ASSERT_EQUAL(app_msg_count, 1);

   //Validate the message
   osEvent event;
   event = osMessageGet(AppQ, 0);
   CU_ASSERT_EQUAL(osEventMessage, event.status);
   pMessage = (CO_Message_t*)event.value.p;
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      /* verify the message type */
      CU_ASSERT_EQUAL(pMessage->Type, CO_MESSAGE_GENERATE_OUTPUT_SIGNAL_E);
      /* verify the message contents */
      CO_OutputData_t outData;
      memcpy((uint8_t*)&outData, pMessage->Payload.PhyDataReq.Data, sizeof(CO_OutputData_t));
      CU_ASSERT_EQUAL(outData.Source, (uint16_t)DEBUG_UART);
      CU_ASSERT_EQUAL(outData.Destination, 1);
      CU_ASSERT_EQUAL(outData.zone, 2);
      CU_ASSERT_EQUAL(outData.OutputProfile, 3);
      CU_ASSERT_EQUAL(outData.OutputsActivated, 4);

      osPoolFree(AppPool, pMessage);
   }
   // verify that no extra resources were used
   used_mem_blocks = osPoolBlocksAllocatedCount(AppPool);
   CU_ASSERT_EQUAL(used_mem_blocks, 0);
   app_msg_count = osMessageCount(AppQ);
   CU_ASSERT_EQUAL(app_msg_count, 0);
}
