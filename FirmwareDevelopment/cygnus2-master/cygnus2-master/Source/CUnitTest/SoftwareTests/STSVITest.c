/*************************************************************************************
*  CONFIDENTIAL between PA and Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2018 PA Consulting Group. All rights reserved.
*  Cambridge Technology Centre
*  Melbourn Royston
*  Herts SG8 6DP UK
*  Tel: <44> 1763 261222
*
**************************************************************************************
*  File         : STSVITest.c
*
*  Description  : Implementation file for the SVI interface code tests
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>



/* User Include Files
*************************************************************************************/
#include "CUnit.h"
#include "CO_Defines.h"
#include "CO_ErrorCode.h"
#include "MessageQueueStub.h"
#include "DM_svi.h"
#include "STSVITest.h"


#define I2C_TEST_RX_BUFFER_SIZE 4U

extern uint8_t HAL_I2C_Mem_Data[I2C_TEST_RX_BUFFER_SIZE];
extern HAL_StatusTypeDef HAL_I2C_Mem_Read_Response;
extern bool HAL_I2C_Mem_Read_CallCount;
extern HAL_StatusTypeDef HAL_I2C_Mem_Write_Response;
extern bool HAL_I2C_Mem_Write_CallCount;
extern uint8_t HAL_I2C_Mem_Write_Data[I2C_TEST_RX_BUFFER_SIZE];

/* Private Functions Prototypes
*************************************************************************************/
static void DM_SVI_InitialiseTest(void);
static void DM_SVI_GetRegisterTest(void);
static void DM_SVI_ReadRegisterTest(void);
static void DM_SVI_WriteRegisterTest(void);
/* Global Variables
*************************************************************************************/

/* Table containing the test settings */
CU_TestInfo ST_SVITests[] =
{
   { "DM_SVI_Initialise",                  DM_SVI_InitialiseTest },
   { "DM_SVI_GetRegister",                 DM_SVI_GetRegisterTest },
   { "DM_SVI_ReadRegister",                DM_SVI_ReadRegisterTest },
   { "DM_SVI_WriteRegister",               DM_SVI_WriteRegisterTest },

   CU_TEST_INFO_NULL,
};


/* Private Variables
*************************************************************************************/


/*************************************************************************************/
/**
* DM_SVI_InitialiseTest
* Test the function DM_SVI_Initialise
*
* no params
*
* @return - none

*/
static void DM_SVI_InitialiseTest(void)
{
   ErrorCode_t result;
   const uint32_t own_address = 1;

   result = DM_SVI_Initialise(own_address);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
}

/*************************************************************************************/
/**
* DM_SVI_GetRegisterTest
* Test the function DM_SVI_GetRegister
*
* no params
*
* @return - none

*/
static void DM_SVI_GetRegisterTest(void)
{
   ErrorCode_t result;
   uint8_t registerAddress;

   // NULL pointer check
   result = DM_SVI_GetRegister(DM_SVI_TYPE_E, NULL);
   CU_ASSERT_EQUAL(result, ERR_INVALID_POINTER_E);

   // Out of range test
   result = DM_SVI_GetRegister(DM_SVI_PARAMETER_TYPE_MAX_E, &registerAddress);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);

   // Valid parameter checks
   result = DM_SVI_GetRegister(DM_SVI_TYPE_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_TYPE);

   result = DM_SVI_GetRegister(DM_SVI_ENABLE_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_ENABLE);

   result = DM_SVI_GetRegister(DM_SVI_TEST_MODE_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_TEST_MODE);

   result = DM_SVI_GetRegister(DM_SVI_FLASH_RATE_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_FLASH_RATE);

   result = DM_SVI_GetRegister(DM_SVI_TONE_ATTEN_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_TONE_ATTEN);

   result = DM_SVI_GetRegister(DM_SVI_FAULT_STATUS_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_FAULT_STATUS);

   result = DM_SVI_GetRegister(DM_SVI_FW_MAJOR_LS_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_FW_MAJOR_LS);

   result = DM_SVI_GetRegister(DM_SVI_FW_MAJOR_MS_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_FW_MAJOR_MS);

   result = DM_SVI_GetRegister(DM_SVI_FW_MINOR_LS_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_FW_MINOR_LS);

   result = DM_SVI_GetRegister(DM_SVI_FW_MINOR_MS_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_FW_MINOR_MS);

   result = DM_SVI_GetRegister(DM_SVI_FW_MAINT_REV_LS_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_FW_MAINT_REV_LS);

   result = DM_SVI_GetRegister(DM_SVI_FW_MAINT_REV_MS_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_FW_MAINT_REV_MS);

   result = DM_SVI_GetRegister(DM_SVI_FW_DATE_DAY_LS_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_FW_DATE_DAY_LS);

   result = DM_SVI_GetRegister(DM_SVI_FW_DATE_DAY_MS_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_FW_DATE_DAY_MS);

   result = DM_SVI_GetRegister(DM_SVI_FW_DATE_MONTH_LS_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_FW_DATE_MONTH_LS);

   result = DM_SVI_GetRegister(DM_SVI_FW_DATE_MONTH_MS_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_FW_DATE_MONTH_MS);

   result = DM_SVI_GetRegister(DM_SVI_FW_DATE_YEAR_LS_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_FW_DATE_YEAR_LS);

   result = DM_SVI_GetRegister(DM_SVI_FW_DATE_YEAR_MS_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_FW_DATE_YEAR_MS);

   result = DM_SVI_GetRegister(DM_SVI_SERIAL_NUMBER_0_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_SERIAL_NUMBER_0);

   result = DM_SVI_GetRegister(DM_SVI_SERIAL_NUMBER_1_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_SERIAL_NUMBER_1);

   result = DM_SVI_GetRegister(DM_SVI_SERIAL_NUMBER_2_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_SERIAL_NUMBER_2);

   result = DM_SVI_GetRegister(DM_SVI_SERIAL_NUMBER_3_E, &registerAddress);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(registerAddress, DM_I2C_SVI_REG_SERIAL_NUMBER_3);

}


/*************************************************************************************/
/**
* DM_SVI_ReadRegisterTest
* Test the function DM_SVI_ReadRegister
*
* no params
*
* @return - none

*/
static void DM_SVI_ReadRegisterTest(void)
{
   ErrorCode_t result;
   uint8_t registerAddress = 0x00;
   uint8_t response;

   // NULL parameter test 
   result = DM_SVI_ReadRegister(registerAddress, NULL);
   CU_ASSERT_EQUAL(result, ERR_INVALID_POINTER_E);

   // Valid parameter test
   HAL_I2C_Mem_Data[0] = 0xa5;
   HAL_I2C_Mem_Read_CallCount = 0;
   HAL_I2C_Mem_Read_Response = SUCCESS_E;

   result = DM_SVI_ReadRegister(registerAddress, &response);
   CU_ASSERT_EQUAL(result, HAL_I2C_Mem_Read_Response);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Read_CallCount, 1);
   CU_ASSERT_EQUAL(response, HAL_I2C_Mem_Data[0]);

}
extern bool HAL_I2C_Mem_Write_CallCount;
extern uint8_t HAL_I2C_Mem_Write_Data[I2C_TEST_RX_BUFFER_SIZE];

/*************************************************************************************/
/**
* DM_SVI_WriteRegisterTest
* Test the function DM_SVI_WriteRegister
*
* no params
*
* @return - none

*/
static void DM_SVI_WriteRegisterTest(void)
{
   ErrorCode_t result;
   uint8_t value = 0;;

   // Read-only parameter test 
   HAL_I2C_Mem_Write_CallCount = 0;
   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_TYPE, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_FAULT_STATUS, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_FW_MAJOR_LS, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_FW_MAJOR_MS, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_FW_MINOR_LS, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_FW_MINOR_MS, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_FW_MAINT_REV_LS, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_FW_MAINT_REV_MS, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_FW_DATE_DAY_LS, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_FW_DATE_DAY_MS, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_FW_DATE_MONTH_LS, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_FW_DATE_MONTH_MS, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_FW_DATE_YEAR_LS, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_FW_DATE_YEAR_MS, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_SERIAL_NUMBER_0, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_SERIAL_NUMBER_1, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_SERIAL_NUMBER_2, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_SERIAL_NUMBER_3, value);
   CU_ASSERT_EQUAL(result, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);


   // Valid parameter test
   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_ENABLE, value);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 1);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_Data[0], value);

   value++;
   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_TEST_MODE, value);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 2);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_Data[0], value);

   value++;
   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_FLASH_RATE, value);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 3);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_Data[0], value);

   value++;
   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_TONE_SELECT, value);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 4);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_Data[0], value);

   value++;
   result = DM_SVI_WriteRegister(DM_I2C_SVI_REG_TONE_ATTEN, value);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 5);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_Data[0], value);
}

