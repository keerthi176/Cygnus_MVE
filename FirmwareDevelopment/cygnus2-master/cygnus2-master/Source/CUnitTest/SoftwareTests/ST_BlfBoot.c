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
*  File         : STBlfBoot.c
*
*  Description  : Implementation of the Unit-Test suite of BLF Boot module
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
#include "stm32l4xx.h"
#include "STWin32_stubs.h"
#include "CO_ErrorCode.h"
#include "STBlfBoot.h"
#include "BLF_Boot.h"
#include "DM_NVM.h"


/*///////////////////\\\\\\\\\\\\\\\\\\\*/
/*          File Under Test             */
#include "BLF_Boot.c"
/*                                      */
/*\\\\\\\\\\\\\\\\\\\///////////////////*/

/* Private Functions Prototypes
*************************************************************************************/
static void BLF_BootManagerTest(void);
static void BLF_CheckReprogramRequestTest(void);

/* Global Variables
*************************************************************************************/

/* Table containing the test settings */
CU_TestInfo ST_BlfBootTests[] =
{
   { "BLF_CheckReprogramRequestTest",BLF_CheckReprogramRequestTest },
   { "BLF_BootManagerTest", BLF_BootManagerTest },

   CU_TEST_INFO_NULL,
};


/* Private Variables
*************************************************************************************/


extern void NVMTestInit(void);
extern void NVMTestDeInit(void);
/*************************************************************************************/
/**
* BlfBoot_TestSuiteInit
* Initialise this test suite
*
* no params
*
* @return - int
*/
int BlfBoot_TestSuiteInit(void)
{
   NVMTestInit();

   jump_to_application_success = false;

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
* BlfBoot_TestSuiteCleanup
* Clean up after this test suite
*
* no params
*
* @return - int
*/
int BlfBoot_TestSuiteCleanup(void)
{
   NVMTestDeInit();
   if (stwin32_stub_application_image_p!=NULL) free(stwin32_stub_application_image_p);

   /* Initialise CRC stub value */
   stub_crc_value = 0x1234u;
   jump_to_application_success = false;

    return 0;
}

/*************************************************************************************/
/**
* BLF_CheckReprogramRequestTest
* test the BLF_CheckReprogramRequest function
*
* no params
*
* @return - none
*/
void BLF_CheckReprogramRequestTest(void)
{
   bool reprog_req;

   /*------------------------------------------------------------------------*/

   /* No reprogramming request */
   reprogram_request_status = 0;
   reprog_req = BLF_CheckReprogramRequest();
   CU_ASSERT_EQUAL(reprog_req, false);
}


/*************************************************************************************/
/**
* BLF_BootManagerTest
* test the main boot manager function
*
* no params
*
* @return - none
*/
static void BLF_BootManagerTest(void)
{
   /* Initialise CRC stub value */
   stub_crc_value = 0x1234u;
   stwin32_stub_application_image_p->prog_check_value = stub_crc_value;
   stwin32_stub_application_image_p->top_of_main_stack_address = 0x20000000;

   /*------------------------------------------------------------------------*/
   /* Reset application call indicator*/
   jump_to_application_success = false;
   /* Reset the reprogramming request flag */
   reprogram_request_status = 0;

   BLF_BootManager();

   CU_ASSERT_EQUAL(reprogram_request_status, 0);
   CU_ASSERT_EQUAL(jump_to_application_success, true);
}
