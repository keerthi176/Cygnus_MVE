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
*  File         : STNVMTest.c
*
*  Description  : Implementation for the Non-Volatile Memory driver tests
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
#include "STNVMTest.h"
#include "DM_NVM.h"
#include "STWin32_stubs.h"



/* Private Functions Prototypes
*************************************************************************************/
static void NV_ParameterInitTest(void);
static void NV_ParameterReadTest(void);
static void NV_ParameterWriteTest(void);

/* Global Variables
*************************************************************************************/

/* Table containing the test settings */
CU_TestInfo ST_NVMTests[] =
{
   { "ParameterInitTest",                  NV_ParameterInitTest },
   { "ParameterReadTest",                  NV_ParameterReadTest },
   { "ParameterWriteTest",                 NV_ParameterWriteTest },

   CU_TEST_INFO_NULL,
};

extern const uint16_t DM_NVM_ParameterVirtualAddr[NV_MAX_PARAM_ID_E];
extern const uint16_t DM_NVM_ParameterSize[NV_MAX_PARAM_ID_E];


/* Private Variables
*************************************************************************************/
static uint8_t *NVbuffer = NULL;
static uint32_t buffer_size = 0u;

void NVMTestInit(void)
{
	for (uint8_t idx = 0; idx < NV_MAX_PARAM_ID_E; idx++)
	{
		buffer_size += DM_NVM_ParameterSize[idx] * 4;
	}

	NVbuffer = malloc(buffer_size + 4u);
	memset(NVbuffer, 0, buffer_size);

	SetEepromAddress(NVbuffer);

	/* Set all zeros*/
	memset(NVbuffer, 0, buffer_size);

	eeprom_return_value = EE_OK;
}

void NVMTestDeInit(void)
{
	if (NVbuffer != NULL) free(NVbuffer);
}

/*************************************************************************************/
/**
* NV_TestSuiteInit
* Initialise this test suite
*
* no params
*
* @return - int
*/
int NV_TestSuiteInit(void)
{
	NVMTestInit();

	return 0;
}

/*************************************************************************************/
/**
* NV_TestSuiteCleanup
* Clean up after this test suite
*
* no params
*
* @return - int
*/
int NV_TestSuiteCleanup(void)
{
	NVMTestDeInit();

	return 0;
}

/*************************************************************************************/
/**
* NV_ParameterInitTest
* test the initialisation function of the NVM module
*
* no params
*
* @return - none

*/
static void NV_ParameterInitTest(void)
{
	ErrorCode_t ret;

	eeprom_return_value = EE_OK;
	/* Check the returned value from the DM_NVMInit() function */
	ret = DM_NVMInit();
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	eeprom_return_value = EE_ERASE_ERROR;
	/* Check the returned value from the DM_NVMInit() function */
	ret = DM_NVMInit();
	CU_ASSERT_EQUAL(ret, ERR_INIT_FAIL_E);
}

/*************************************************************************************/
/**
* NV_ParameterReadTest
* test reading of parameters from NV Memory
*
* no params
*
* @return - none

*/
static void NV_ParameterReadTest(void)
{
   ErrorCode_t ret;
   uint32_t value = 0;

   /* Initialise CRC stub value */
   stub_crc_value = 0x1234u;

   /* parameter id boundary test */
   memset(NVbuffer, 0, buffer_size);
   value = 0x11223344;
   memcpy(NVbuffer + (4 * DM_NVM_ParameterVirtualAddr[NV_ADDRESS_E]), &value, DM_NVM_ParameterSize[NV_ADDRESS_E]*4);
   value = 0;
   eeprom_return_value = EE_OK;
   
   ret = DM_NVMRead(NV_ADDRESS_E, (void*)&value, sizeof(value));
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(value, 0x11223344);

   memset(NVbuffer, 0, buffer_size);
   value = 0x11223344;
   memcpy(NVbuffer + (4 * DM_NVM_ParameterVirtualAddr[NV_IS_SYNC_MASTER_E]), &value, DM_NVM_ParameterSize[NV_IS_SYNC_MASTER_E]*4);
   value = 0;
   eeprom_return_value = EE_OK;

   ret = DM_NVMRead(NV_IS_SYNC_MASTER_E, (void*)&value, sizeof(value));
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(value, 0x11223344);

   /* Out of boundary Check */
   value = 0;
   ret = DM_NVMRead(NV_MAX_PARAM_ID_E, (void*)&value, sizeof(value));
   CU_ASSERT_EQUAL(ret, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(value, 0);

   /* No Valid Data */
   memset(NVbuffer, 0, buffer_size);
   value = 0x11223344;
   memcpy(NVbuffer + (4 * DM_NVM_ParameterVirtualAddr[NV_IS_SYNC_MASTER_E]), &value, DM_NVM_ParameterSize[NV_IS_SYNC_MASTER_E] * 4);
   value = 0;
   eeprom_return_value = EE_NO_DATA;

   ret = DM_NVMRead(NV_IS_SYNC_MASTER_E, (void*)&value, sizeof(value));
   CU_ASSERT_EQUAL(ret, ERR_DATA_INTEGRITY_CHECK_E);
   CU_ASSERT_EQUAL(value, 0x11223344);

   /* Wrong size: inconsistent configuration or usage */
   memset(NVbuffer, 0, buffer_size);
   value = 0x11223344;
   memcpy(NVbuffer + (4 * DM_NVM_ParameterVirtualAddr[NV_IS_SYNC_MASTER_E]), &value, DM_NVM_ParameterSize[NV_IS_SYNC_MASTER_E] * 4);
   value = 0;
   eeprom_return_value = EE_OK;

   ret = DM_NVMRead(NV_IS_SYNC_MASTER_E, (void*)&value, 2u);
   CU_ASSERT_EQUAL(ret, ERR_OUT_OF_RANGE_E);
   CU_ASSERT_EQUAL(value, 0u);

   /* null pointer test */
   ret = DM_NVMRead(NV_ADDRESS_E, NULL,4);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
}

/*************************************************************************************/
/**
* NV_ParameterWriteTest
* test writing a parameter to NV memory
*
* no params
*
* @return - none

*/
static void NV_ParameterWriteTest(void)
{
   ErrorCode_t ret;
   uint32_t value= 0;

   /* Initialise CRC stub value */
   stub_crc_value = 0x1234u;

   /* parameter id boundary test */
   memset(NVbuffer, 0, buffer_size);
   value = 0x11223344;
   ret = DM_NVMWrite(NV_ADDRESS_E, (void*)&value, sizeof(value));
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   value = 0x11223344;
   CU_ASSERT_EQUAL(memcmp(NVbuffer + (4 * DM_NVM_ParameterVirtualAddr[NV_ADDRESS_E]), &value, DM_NVM_ParameterSize[NV_ADDRESS_E] * 4), 0);
   value = 0x12345678;
   //CU_ASSERT_EQUAL(memcmp(NVbuffer + ParameterInfo[NV_ADDRESS_E].crc_addr_offset, &value, 4u), 0);

   memset(NVbuffer, 0, buffer_size);
   value = 0x11223344;
   ret = DM_NVMWrite(NV_IS_SYNC_MASTER_E, (void*)&value, sizeof(value));
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   value = 0x11223344;
   CU_ASSERT_EQUAL(memcmp(NVbuffer + (4 * DM_NVM_ParameterVirtualAddr[NV_IS_SYNC_MASTER_E]), &value, DM_NVM_ParameterSize[NV_IS_SYNC_MASTER_E] * 4), 0);
   value = 0x12345678;

   /* Wrong size: inconsistent configuration or usage */
   memset(NVbuffer, 0, buffer_size);
   value = 0x11223344;
   ret = DM_NVMWrite(NV_IS_SYNC_MASTER_E, (void*)&value, 2);
   CU_ASSERT_EQUAL(ret, ERR_INVALID_POINTER_E);
   value = 0x0u;
   CU_ASSERT_EQUAL(memcmp(NVbuffer + (4*DM_NVM_ParameterVirtualAddr[NV_IS_SYNC_MASTER_E]), &value, DM_NVM_ParameterSize[NV_IS_SYNC_MASTER_E] * 4), 0);
   value = 0x0u;
   eeprom_return_value = EE_NO_DATA;

   /* Out of boundary Check */
   value = 0xFFFFu;
   ret = DM_NVMWrite(NV_MAX_PARAM_ID_E, (void*)&value, sizeof(value));
   CU_ASSERT_EQUAL(ret, ERR_OUT_OF_RANGE_E);

   /* null pointer test */
   ret = DM_NVMWrite(NV_ADDRESS_E, NULL, 4);
   CU_ASSERT_EQUAL(ret, ERR_INVALID_POINTER_E)
}

