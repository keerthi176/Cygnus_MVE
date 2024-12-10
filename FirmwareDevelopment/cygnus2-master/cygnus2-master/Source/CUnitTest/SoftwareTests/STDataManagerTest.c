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
*  File         : STDataManagerTest.c
*
*  Description  : Implementation for the Data Manager tests
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
#include "STWin32_stubs.h"
#include "STDataManagerTest.h"
#include "DM_NVM.h"
#include "MM_DataManager.h"


/* Private Functions Prototypes
*************************************************************************************/
static void DataManagerInitTest(void);
static void DataManagerCheckDataIntegrityTest(void);
static void DataManagerUpdProtectedData(void);

/* Global Variables
*************************************************************************************/

/* Table containing the test settings */
CU_TestInfo ST_DataManagerTests[] =
{
   { "DataManagerInit",                   DataManagerInitTest },
   { "DataManagerCheckDataIntegrityTest", DataManagerCheckDataIntegrityTest },
   { "DataManagerUpdProtectedDataTest",   DataManagerUpdProtectedData },

   CU_TEST_INFO_NULL,
};

extern const uint16_t DM_NVM_ParameterVirtualAddr[NV_MAX_PARAM_ID_E];
extern const uint16_t DM_NVM_ParameterSize[NV_MAX_PARAM_ID_E];


/* Private Variables
*************************************************************************************/

extern void NVMTestInit(void);
extern void NVMTestDeInit(void);
/*************************************************************************************/
/**
* DataManager_TestSuiteInit
* Initialise this test suite
*
* no params
*
* @return - int
*/
int DataManager_TestSuiteInit(void)
{
	NVMTestInit();

	return 0;
}

/*************************************************************************************/
/**
* DataManager_TestSuiteCleanup
* Clean up after this test suite
*
* no params
*
* @return - int
*/
int DataManager_TestSuiteCleanup(void)
{
	NVMTestDeInit();
	return 0;
}

/*************************************************************************************/
/**
* DataManagerInit
* test the initialisation function of the Data Manager module
*
* no params
*
* @return - none

*/
static void DataManagerInitTest(void)
{
	ErrorCode_t ret;
	uint32_t value;

	eeprom_return_value = EE_OK;

	for (uint8_t idx = 0; idx < NV_MAX_PARAM_ID_E; idx++)
	{
		ret = DM_NVMWrite(idx, (void*)&value, sizeof(value));
	}

	/* Check the returned value from the DM_NVMInit() function */
	ret = MM_DataManagerInit();
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
}

/*************************************************************************************/
/**
* DataManagerCheckDataIntegrityTest
* test reading of parameters from NV Memory
*
* no params
*
* @return - none

*/
static void DataManagerCheckDataIntegrityTest(void)
{
	ErrorCode_t ret;
	uint32_t value = 0;

	eeprom_return_value = EE_OK;

	for (uint8_t idx = 0; idx < NV_MAX_PARAM_ID_E; idx++)
	{
		ret = DM_NVMWrite(idx, (void*)&value, sizeof(value));
	}

	stub_crc_value = 0x1234u;

	/* Check the returned value from the DM_NVMInit() function */
	ret = MM_DataManagerInit();
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	/* Generate a different CRC value */
	stub_crc_value = 0x4321u;
	ret = MM_DataManagerCheckDataIntegrity();
	CU_ASSERT_EQUAL(ret, ERR_DATA_INTEGRITY_CHECK_E);
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
static void DataManagerUpdProtectedData(void)
{

}

