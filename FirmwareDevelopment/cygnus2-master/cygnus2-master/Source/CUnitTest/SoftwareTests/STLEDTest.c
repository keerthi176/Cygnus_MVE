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
*  File         : STLEDTest.c
*
*  Description  : Implementation for the LED driver tests
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
#include "STLEDTest.h"
#include "DM_LED.h"
#include "gpio.h"

/*///////////////////\\\\\\\\\\\\\\\\\\\*/
/*          File Under Test             */
#include "DM_LED.c"
/*                                      */
/*\\\\\\\\\\\\\\\\\\\///////////////////*/

/* Private Functions Prototypes
*************************************************************************************/
static void DM_LedPatternRequestTest(void);
static void DM_LedExternalControlTest(void);
static void DM_LedPeriodicTickTest(void);


/* Global Variables
*************************************************************************************/
extern uint32_t os_Kernel_Sys_Tick;
Gpio_t StatusLedRed;
Gpio_t StatusLedGreen;
Gpio_t StatusLedBlue;

/* Table containing the test settings */
CU_TestInfo ST_LEDTests[] =
{	
	{ "DM_LedPatternRequest",                DM_LedPatternRequestTest },
	{ "DM_LedExternalControl",               DM_LedExternalControlTest },
    { "DM_LedPeriodicTick",                  DM_LedPeriodicTickTest },
   CU_TEST_INFO_NULL,
};


/* Private Variables
*************************************************************************************/


/*************************************************************************************/
/**
* ST_LEDTestSuiteInit
* routine to initialise the LED test suite
*
* no params
*
* @return - int  - 0=success
*/
int ST_LEDTestSuiteInit(void)
{
   /* clear the led variables */
   memset(&StatusLedRed, 0, sizeof(Gpio_t));
   memset(&StatusLedGreen, 0, sizeof(Gpio_t));
   memset(&StatusLedBlue, 0, sizeof(Gpio_t));

   return 0;
}


/*************************************************************************************/
/**
* ST_LEDTestSuiteCleanup
* routine to clean up after the LED test suite
*
* no params
*
* @return - int  - 0=success
*/
int ST_LEDTestSuiteCleanup(void)
{
   return 0;
}

/*************************************************************************************/
/**
* DM_LedPatternRequestTest
* test the DM_LedPatternRequest() function
*
* no params
*
* @return - none

*/
static void DM_LedPatternRequestTest(void)
{
	DM_LedInit();

	/* No running pattern */
	DM_LedCurrentPattern = LED_PATTERN_NOT_DEFINED_E;
	DM_LedNextPattern = LED_PATTERN_NOT_DEFINED_E;
	DM_LedPatternRequest(LED_MESH_STATE_CONFIG_SYNC_E);
	CU_ASSERT_EQUAL(DM_LedCurrentPattern, LED_MESH_STATE_CONFIG_SYNC_E);
	CU_ASSERT_EQUAL(DM_LedNextPattern, LED_PATTERN_NOT_DEFINED_E);

	/* A running pattern but no pending one */
	DM_LedCurrentPattern = LED_MESH_STATE_CONFIG_SYNC_E;
	DM_LedNextPattern = LED_PATTERN_NOT_DEFINED_E;
	DM_LedPatternRequest(LED_BUILT_IN_TEST_FAIL_E);
	CU_ASSERT_EQUAL(DM_LedCurrentPattern, LED_MESH_STATE_CONFIG_SYNC_E );
	CU_ASSERT_EQUAL(DM_LedNextPattern, LED_BUILT_IN_TEST_FAIL_E);

	/* A running pattern and a high priority pending one */
	DM_LedCurrentPattern = LED_BUILT_IN_TEST_FAIL_E;
	DM_LedNextPattern = LED_MESH_STATE_ACTIVE_E;
	DM_LedPatternRequest(LED_MESH_STATE_CONFIG_SYNC_E);
	CU_ASSERT_EQUAL(DM_LedCurrentPattern, LED_BUILT_IN_TEST_FAIL_E);
	CU_ASSERT_EQUAL(DM_LedNextPattern, LED_MESH_STATE_ACTIVE_E);

	/* A running pattern and a low priority pending one */
	DM_LedCurrentPattern = LED_BUILT_IN_TEST_FAIL_E;
	DM_LedNextPattern = LED_MESH_STATE_CONFIG_SYNC_E;
	DM_LedPatternRequest(LED_MESH_STATE_ACTIVE_E);
	CU_ASSERT_EQUAL(DM_LedCurrentPattern, LED_BUILT_IN_TEST_FAIL_E);
	CU_ASSERT_EQUAL(DM_LedNextPattern, LED_MESH_STATE_ACTIVE_E);
}

/*************************************************************************************/
/**
* DM_LedValidParameterTest
* test the DM_LedValidParameter() function
*
* no params
*
* @return - none

*/
static void DM_LedExternalControlTest(void)
{
   DM_LedInit();

   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 0);

   /* configure led driver to use test pattern LED_MESH_STATE_CONFIG_SYNC_E */
   DM_LedPatternRequest(LED_MESH_STATE_CONFIG_SYNC_E);
   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 0);

   DM_LedExternalControl(LED_MESH_STATE_CONFIG_SYNC_E, 0);
   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 1);
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 0);

   /* Invalid step */
   DM_LedExternalControl(LED_MESH_STATE_CONFIG_SYNC_E, 7);
   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 1);
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 0);

   /* Invalid Pattern */
   DM_LedExternalControl(LED_MESH_STATE_ACTIVE_E, 1);
   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 1);
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 0);

   /*Valid*/
   DM_LedExternalControl(LED_MESH_STATE_CONFIG_SYNC_E, 1);
   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 0);

   /* configure led driver to use test pattern LED_MESH_STATE_TEST_MODE_E */
   DM_LedPatternRequest(LED_MESH_STATE_TEST_MODE_E);
   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 0);

   DM_LedExternalControl(LED_MESH_STATE_TEST_MODE_E, 0);
   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 1);

   /* Invalid step */
   DM_LedExternalControl(LED_MESH_STATE_TEST_MODE_E, 6);
   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 1);

   /* Invalid Pattern */
   DM_LedExternalControl(LED_MESH_STATE_ACTIVE_E, 1);
   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 1);

   /* Valid */
   DM_LedExternalControl(LED_MESH_STATE_TEST_MODE_E, 1);
   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 0);
}

/*************************************************************************************/
/**
* DM_LedPeriodicTickTest
* Test DM_LedPeriodicTick() fcuntion
*
* no params
*
* @return - none

*/
static void DM_LedPeriodicTickTest(void)
{

   /* clear the led variables */
   memset(&StatusLedRed, 0, sizeof(Gpio_t));
   memset(&StatusLedGreen, 0, sizeof(Gpio_t));
   memset(&StatusLedBlue, 0, sizeof(Gpio_t));

   DM_LedInit();
   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 0);

   os_Kernel_Sys_Tick = 1000;

   /* configure led driver to use test pattern LED_BUILT_IN_TEST_FAIL_E */
   DM_LedPatternRequest(LED_BUILT_IN_TEST_FAIL_E);
   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 1); /*Starts straight away*/
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 0);

   os_Kernel_Sys_Tick = 1000 + osKernelSysTickMicroSec(DM_LedPatternConfig[LED_BUILT_IN_TEST_FAIL_E].state_steps_def[0].step_duration_us - 1);

   DM_LedPeriodicTick();
   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 1);
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 0);
   CU_ASSERT_EQUAL(DM_LedCurrentPattern, LED_BUILT_IN_TEST_FAIL_E);

   os_Kernel_Sys_Tick = 1000 + osKernelSysTickMicroSec(DM_LedPatternConfig[LED_BUILT_IN_TEST_FAIL_E].state_steps_def[0].step_duration_us + 1);
   DM_LedPeriodicTick();

   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 0);
   CU_ASSERT_EQUAL(DM_LedCurrentStep, 1);
   CU_ASSERT_EQUAL(DM_LedCurrentPattern, LED_BUILT_IN_TEST_FAIL_E);

   /* Pattern time elapsed but we need to wait for the start of a new sequence */
   os_Kernel_Sys_Tick = 1000 + 2*osKernelSysTickMicroSec(DM_LedPatternConfig[LED_BUILT_IN_TEST_FAIL_E].max_duration_us + 1);
   DM_LedCurrentStep = 3;
   DM_LedPeriodicTick();

   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 1);
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 0);
   CU_ASSERT_EQUAL(DM_LedCurrentStep, 0);
   CU_ASSERT_EQUAL(DM_LedCurrentPattern, LED_BUILT_IN_TEST_FAIL_E);

   /* Pattern finished */
   DM_LedPeriodicTick();
   CU_ASSERT_EQUAL(StatusLedRed.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedGreen.pinIndex, 0);
   CU_ASSERT_EQUAL(StatusLedBlue.pinIndex, 0);
   CU_ASSERT_EQUAL(DM_LedCurrentPattern, LED_PATTERN_NOT_DEFINED_E);
}
