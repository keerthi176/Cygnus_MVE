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
*  File         : STBlf_AppUpdater.c
*
*  Description  : Implementation of the Unit-Test suite of BLF_AppUpdater module
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>


/* User Include Files
*************************************************************************************/
#include "CUnit.h"
#include "BLF_AppUpdater.h"
#include "STBlf_AppUpdater.h"
#include "DM_NVM.h"
#include "stm32l4xx.h"
#include "STWin32_stubs.h"

/*///////////////////\\\\\\\\\\\\\\\\\\\*/
/*          File Under Test             */
#include "BLF_AppUpdater.c"
/*                                      */
/*\\\\\\\\\\\\\\\\\\\///////////////////*/

/* Private Functions Prototypes
*************************************************************************************/
static void BLF_LoaderUpdaterTest(void);
static void check_other_bank_app_validityTest(void);

/* Global Variables
*************************************************************************************/

/* Table containing the test settings */
CU_TestInfo ST_BlfAppUpdaterTests[] =
{
   { "check_other_bank_app_validityTest", check_other_bank_app_validityTest },
   { "BLF_LoaderUpdaterTest",          BLF_LoaderUpdaterTest },

   CU_TEST_INFO_NULL,
};

extern const uint16_t DM_NVM_ParameterVirtualAddr[NV_MAX_PARAM_ID_E];
extern const uint16_t DM_NVM_ParameterSize[NV_MAX_PARAM_ID_E];

/* Private Variables
*************************************************************************************/

extern void BLF_LoaderUpdater(reprogramming_mode_t reprogramming_mode);
extern void app_main(void);
extern void NVMTestInit(void);
extern void NVMTestDeInit(void);

unsigned char Checksum(unsigned char *data, unsigned char length)
{
	unsigned char count;
	unsigned int Sum = 0;

	for (count = 0; count < length; count++)
		Sum = Sum + data[count];

	return (Sum & 0xFF);
}

/*************************************************************************************/
/**
* BlfAppUpdater_TestSuiteInit
* Initialise this test suite
*
* no params
*
* @return - int
*/
int BlfAppUpdater_TestSuiteInit(void)
{
	NVMTestInit();

   /* Allocate space for Application stub */
   stwin32_stub_application_image_p = malloc(sizeof(stwin32_stub_application_image_t));

   if (stwin32_stub_application_image_p != NULL)
   {
      stwin32_stub_application_image_p->reset_function_address = (uint32_t)&app_main;
      stwin32_stub_application_image_p->top_of_main_stack_address = 0x20000000;
   }

   return 0;
}

/*************************************************************************************/
/**
* BlfAppUpdater_TestSuiteCleanup
* Clean up after this test suite
*
* no params
*
* @return - int
*/
int BlfAppUpdater_TestSuiteCleanup(void)
{
	NVMTestDeInit();
	if (stwin32_stub_application_image_p!=NULL) free(stwin32_stub_application_image_p);

   /* Initialise CRC stub value */
   stub_crc_value = 0x1234u;
   return 0;
}

/*************************************************************************************/
/**
* check_other_bank_app_validityTest
* test the check_other_bank_app_validity function
*
* no params
*
* @return - none
*/
static void check_other_bank_app_validityTest(void)
{
   blf_application_status_t application_status;

   stwin32_stub_application_image_p->top_of_main_stack_address = 0x20000000;
   stwin32_stub_application_image_p->reset_function_address = 0x08000100;

   /*------------------------------------------------------------------------*/
   // Valid Application Image
   stwin32_stub_application_image_p->prog_check_value = Checksum((unsigned char*)stwin32_stub_application_image_p, sizeof(stwin32_stub_application_image_t)-4u);
   application_status = check_other_bank_app_validity(sizeof(stwin32_stub_application_image_t));
   CU_ASSERT_EQUAL(application_status, VALID_APPLICATION_PRESENT_E);

   /*------------------------------------------------------------------------*/
   // No Valid Application Image
   stwin32_stub_application_image_p->prog_check_value = Checksum((unsigned char*)stwin32_stub_application_image_p, sizeof(stwin32_stub_application_image_t)-4u) - 1;
   application_status = check_other_bank_app_validity(sizeof(stwin32_stub_application_image_t));
   CU_ASSERT_EQUAL(application_status, NO_VALID_APPLICATION_E);

}

/*************************************************************************************/
/**
* BLF_LoaderUpdaterTest
* test the BLF_LoaderUpdater function
*
* no params
*
* @return - none
*/
static void BLF_LoaderUpdaterTest(void)
{
	blf_app_update_result_t result;
   eeprom_return_value = EE_OK;
   stwin32_stub_application_image_p->top_of_main_stack_address = 0x20000000;
   stwin32_stub_application_image_p->reset_function_address    = 0x08000100;

   /*------------------------------------------------------------------------*/
   // Successful Download and programing
   stwin32_stub_application_image_p->prog_check_value = Checksum((unsigned char*)stwin32_stub_application_image_p, sizeof(stwin32_stub_application_image_t) - 4u);
   stub_ymodem_result = COM_OK;
   result = BLF_SerialLoaderUpdater();
   CU_ASSERT_EQUAL(result, APP_UPDATE_SUCCESS_E);

   /*------------------------------------------------------------------------*/
   // Successful Download but programing failed
   stwin32_stub_application_image_p->prog_check_value = Checksum((unsigned char*)stwin32_stub_application_image_p, sizeof(stwin32_stub_application_image_t) - 4u) -1;
   stub_ymodem_result = COM_OK;
   result = BLF_SerialLoaderUpdater();
   CU_ASSERT_EQUAL(result, APP_UPDATE_FAIL_E);


   /*------------------------------------------------------------------------*/
   // Unsuccessful Download
   stwin32_stub_application_image_p->prog_check_value = stwin32_stub_application_image_p->top_of_main_stack_address + stwin32_stub_application_image_p->reset_function_address;
   stub_ymodem_result = COM_ERROR;
   result = BLF_SerialLoaderUpdater();
   CU_ASSERT_EQUAL(result, APP_UPDATE_FAIL_E);
}
