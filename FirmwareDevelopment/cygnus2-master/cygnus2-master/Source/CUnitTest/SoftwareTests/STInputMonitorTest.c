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
*  File         : STInputMonitorTest.c
*
*  Description  : Implementation for the Input Monitor tests
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stdbool.h"
#include <assert.h>
#include <inttypes.h>


/* User Include Files
*************************************************************************************/
#include "CUnit.h"
#include "stm32l4xx.h"
#include "STWin32_stubs.h"
#include "STInputMonitorTest.h"
#include "DM_InputMonitor.h"

/* Private Functions Prototypes
*************************************************************************************/
static void IN_MON_ActiveAtResetTest(void);
static void IN_MON_InactiveAtResetTest(void);
static void IN_MON_InvalidDataTest(void);
static void IN_MON_CallbackFail(void);

/* Global Variables
*************************************************************************************/

/* Table containing the test settings */
CU_TestInfo ST_InputMonitorTests[] =
{
   { "IN_MON_ActiveAtResetTest",      IN_MON_ActiveAtResetTest },
   { "IN_MON_InactiveAtResetTest",    IN_MON_InactiveAtResetTest },
   { "IN_MON_InvalidDataTest",        IN_MON_InvalidDataTest },
   { "IN_MON_CallbackFail",           IN_MON_CallbackFail },

   CU_TEST_INFO_NULL,
};

/* Private Variables
*************************************************************************************/
static uint32_t TestActivateCount = 0;
static uint32_t TestDeactivateCount = 0;
static uint32_t DummyActivateCount = 0;
static uint32_t DummyDeactivateCount = 0;
static ErrorCode_t callback_fail_return;

void MM_FireMCPInputIrq(void)
{
}

void MM_FirstAidMCPInputIrq(void)
{
}

void MM_TamperSwitchInputIrq(void)
{
}

void MM_MainBatteryInputIrq(void)
{
}

static ErrorCode_t TestStateChangeCallback(DM_InputMonitorState_t newState)
{
   if (IN_MON_IDLE_E == newState)
   {
      TestDeactivateCount++;
   }
   else if (IN_MON_ACTIVE_E == newState)
   {
      TestActivateCount++;
   }

   return SUCCESS_E;
}

static ErrorCode_t DummylStateChangeCallback(DM_InputMonitorState_t newState)
{
   if (IN_MON_IDLE_E == newState)
   {
      DummyDeactivateCount++;
   }
   else if (IN_MON_ACTIVE_E == newState)
   {
      DummyActivateCount++;
   }

   return SUCCESS_E;
}

static ErrorCode_t CallbackFailure(DM_InputMonitorState_t newState)
{
	if (IN_MON_IDLE_E == newState)
	{
		TestDeactivateCount++;
	}
	else if (IN_MON_ACTIVE_E == newState)
	{
		TestActivateCount++;
	}

	return callback_fail_return;
}

/*************************************************************************************/
/**
* IN_MON_ActiveAtResetTest
* test active pin case
*
* no params
*
* @return - none
*/
static void IN_MON_ActiveAtResetTest(void)
{
   ErrorCode_t ret;
   uint32_t state_change_os_tick_value = 10000u;

   /* Initialise test data */
   TestFireMCPState = GPIO_PIN_SET;
   TestFirstAidMCPState = GPIO_PIN_SET;
   TestTamper1State = GPIO_PIN_RESET;
   TestTamper2State = GPIO_PIN_RESET;
   TestMainBatteryState = GPIO_PIN_SET;
   TestActivateCount = 0;
   TestDeactivateCount = 0;
   DummyActivateCount = 0;
   DummyDeactivateCount = 0;

   os_Kernel_Sys_Tick = state_change_os_tick_value;

   ret = DM_InputMonitorInit(IN_MON_FIRE_E, TestStateChangeCallback);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);

   ret = DM_InputMonitorInit(IN_MON_FIRST_AID_E, DummylStateChangeCallback);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);

   ret = DM_InputMonitorInit(IN_MON_TAMPER1_E, DummylStateChangeCallback);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);

   ret = DM_InputMonitorInit(IN_MON_TAMPER2_E, DummylStateChangeCallback);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);

   ret = DM_InputMonitorInit(IN_MON_MAIN_BATTERY_E, DummylStateChangeCallback);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);

   /* Restart the polling */
   DM_InputMonitorPollingStart();
   CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), true);

   os_Kernel_Sys_Tick = state_change_os_tick_value + FIRE_DEACTIVATE_DEBOUNCE_US - 10;

   ret = DM_InputMonitorPoll(IN_MON_FIRE_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_FIRST_AID_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_TAMPER1_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_TAMPER2_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_MAIN_BATTERY_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);

   CU_ASSERT_EQUAL(TestActivateCount, 0);
   CU_ASSERT_EQUAL(TestDeactivateCount, 0);
   CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), true);

   os_Kernel_Sys_Tick = state_change_os_tick_value + FIRE_DEACTIVATE_DEBOUNCE_US + 10;

   ret = DM_InputMonitorPoll(IN_MON_FIRE_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_FIRST_AID_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_TAMPER1_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_TAMPER2_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_MAIN_BATTERY_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);

   CU_ASSERT_EQUAL(TestActivateCount, 0);
   CU_ASSERT_EQUAL(TestDeactivateCount, 0);
   CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), false);

   TestFireMCPState = GPIO_PIN_RESET;
   TestFirstAidMCPState = GPIO_PIN_SET;
   TestTamper1State = GPIO_PIN_RESET;
   TestTamper2State = GPIO_PIN_RESET;
   TestMainBatteryState = GPIO_PIN_SET;

   state_change_os_tick_value = 200000u;
   os_Kernel_Sys_Tick = state_change_os_tick_value;

   DM_InputMonitorPollingStart();
   CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), true);

   os_Kernel_Sys_Tick = state_change_os_tick_value + FIRE_ACTIVATE_DEBOUNCE_US - 10;

   ret = DM_InputMonitorPoll(IN_MON_FIRE_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_FIRST_AID_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_TAMPER1_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_TAMPER2_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_MAIN_BATTERY_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);

   CU_ASSERT_EQUAL(TestActivateCount, 0);
   CU_ASSERT_EQUAL(TestDeactivateCount, 0);
   CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), true);

   os_Kernel_Sys_Tick = state_change_os_tick_value + FIRE_ACTIVATE_DEBOUNCE_US + 10;

   ret = DM_InputMonitorPoll(IN_MON_FIRE_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_FIRST_AID_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_TAMPER1_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_TAMPER2_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_MAIN_BATTERY_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);

   CU_ASSERT_EQUAL(TestActivateCount, 1);
   CU_ASSERT_EQUAL(TestDeactivateCount, 0);

   TestFireMCPState = GPIO_PIN_SET;
   TestFirstAidMCPState = GPIO_PIN_SET;
   TestTamper1State = GPIO_PIN_RESET;
   TestTamper2State = GPIO_PIN_RESET;
   TestMainBatteryState = GPIO_PIN_SET;

   state_change_os_tick_value = 300000u;
   os_Kernel_Sys_Tick = state_change_os_tick_value;

   DM_InputMonitorPollingStart();
   CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), true);

   os_Kernel_Sys_Tick = state_change_os_tick_value + FIRE_DEACTIVATE_DEBOUNCE_US - 10;

   ret = DM_InputMonitorPoll(IN_MON_FIRE_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_FIRST_AID_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_TAMPER1_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_TAMPER2_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_MAIN_BATTERY_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);

   CU_ASSERT_EQUAL(TestActivateCount, 1);
   CU_ASSERT_EQUAL(TestDeactivateCount, 0);
   CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), true);

   os_Kernel_Sys_Tick = state_change_os_tick_value + FIRE_DEACTIVATE_DEBOUNCE_US + 10;

   ret = DM_InputMonitorPoll(IN_MON_FIRE_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_FIRST_AID_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_TAMPER1_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_TAMPER2_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   ret = DM_InputMonitorPoll(IN_MON_MAIN_BATTERY_E);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);

   CU_ASSERT_EQUAL(TestActivateCount, 1);
   CU_ASSERT_EQUAL(TestDeactivateCount, 1);

   /* Check that no other callbacks occured for the other pins */
   CU_ASSERT_EQUAL(DummyActivateCount, 0);
   CU_ASSERT_EQUAL(DummyDeactivateCount, 0);

   CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), false);
}


/*************************************************************************************/
/**
* IN_MON_InactiveAtResetTest
* test first aid call point valid data
*
* no params
*
* @return - none
*/
static void IN_MON_InactiveAtResetTest(void)
{
	ErrorCode_t ret;
	uint32_t state_change_os_tick_value = 10000u;

	/* Initialise test data */
	TestFireMCPState = GPIO_PIN_RESET;
	TestFirstAidMCPState = GPIO_PIN_SET;
	TestTamper1State = GPIO_PIN_RESET;
	TestTamper2State = GPIO_PIN_RESET;
	TestMainBatteryState = GPIO_PIN_SET;
	TestActivateCount = 0;
	TestDeactivateCount = 0;
	DummyActivateCount = 0;
	DummyDeactivateCount = 0;

	os_Kernel_Sys_Tick = state_change_os_tick_value;

	ret = DM_InputMonitorInit(IN_MON_FIRE_E, TestStateChangeCallback);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	ret = DM_InputMonitorInit(IN_MON_FIRST_AID_E, DummylStateChangeCallback);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	ret = DM_InputMonitorInit(IN_MON_TAMPER1_E, DummylStateChangeCallback);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	ret = DM_InputMonitorInit(IN_MON_TAMPER2_E, DummylStateChangeCallback);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	ret = DM_InputMonitorInit(IN_MON_MAIN_BATTERY_E, DummylStateChangeCallback);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	/* Restart the polling */
	DM_InputMonitorPollingStart();
	CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), true);

	os_Kernel_Sys_Tick = state_change_os_tick_value + FIRE_ACTIVATE_DEBOUNCE_US - 10;

	ret = DM_InputMonitorPoll(IN_MON_FIRE_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_FIRST_AID_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_TAMPER1_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_TAMPER2_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_MAIN_BATTERY_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	CU_ASSERT_EQUAL(TestActivateCount, 0);
	CU_ASSERT_EQUAL(TestDeactivateCount, 0);
	CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), true);

	os_Kernel_Sys_Tick = state_change_os_tick_value + FIRE_ACTIVATE_DEBOUNCE_US + 10;

	ret = DM_InputMonitorPoll(IN_MON_FIRE_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_FIRST_AID_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_TAMPER1_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_TAMPER2_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_MAIN_BATTERY_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	CU_ASSERT_EQUAL(TestActivateCount, 1);
	CU_ASSERT_EQUAL(TestDeactivateCount, 0);
	CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), false);

	TestFireMCPState = GPIO_PIN_SET;
	TestFirstAidMCPState = GPIO_PIN_SET;
	TestTamper1State = GPIO_PIN_RESET;
	TestTamper2State = GPIO_PIN_RESET;
	TestMainBatteryState = GPIO_PIN_SET;

	state_change_os_tick_value = 200000u;
	os_Kernel_Sys_Tick = state_change_os_tick_value;

	DM_InputMonitorPollingStart();
	CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), true);

	os_Kernel_Sys_Tick = state_change_os_tick_value + FIRE_DEACTIVATE_DEBOUNCE_US - 10;

	ret = DM_InputMonitorPoll(IN_MON_FIRE_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_FIRST_AID_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_TAMPER1_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_TAMPER2_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_MAIN_BATTERY_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	CU_ASSERT_EQUAL(TestActivateCount, 1);
	CU_ASSERT_EQUAL(TestDeactivateCount, 0);
	CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), true);

	os_Kernel_Sys_Tick = state_change_os_tick_value + FIRE_DEACTIVATE_DEBOUNCE_US + 10;

	ret = DM_InputMonitorPoll(IN_MON_FIRE_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_FIRST_AID_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_TAMPER1_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_TAMPER2_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_MAIN_BATTERY_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	CU_ASSERT_EQUAL(TestActivateCount, 1);
	CU_ASSERT_EQUAL(TestDeactivateCount, 1);

	/* Check that no other callbacks occured for the other pins */
	CU_ASSERT_EQUAL(DummyActivateCount, 0);
	CU_ASSERT_EQUAL(DummyDeactivateCount, 0);

	CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), false);
}

/*************************************************************************************/
/**
* IN_MON_InvalidDataTest
* test invalid data for call point
*
* no params
*
* @return - none
*/
static void IN_MON_InvalidDataTest(void)
{
   ErrorCode_t ret;

   /* init boundary tests */
   ret = DM_InputMonitorInit(IN_MON_MAX_IDX_E, TestStateChangeCallback);
   CU_ASSERT_EQUAL(ret, ERR_OUT_OF_RANGE_E);

   ret = DM_InputMonitorInit(IN_MON_FIRST_AID_E, NULL);
   CU_ASSERT_EQUAL(ret, ERR_OUT_OF_RANGE_E);

   /* poll boundary checks */
   ret = DM_InputMonitorPoll(IN_MON_MAX_IDX_E);
   CU_ASSERT_EQUAL(ret, ERR_OUT_OF_RANGE_E);

}

/*************************************************************************************/
/**
* IN_MON_CallbackFail
* test callback Failure
*
* no params
*
* @return - none
*/
static void IN_MON_CallbackFail(void)
{
	ErrorCode_t ret;
	uint32_t state_change_os_tick_value = 10000u;

	/* Initialise test data */
	TestFireMCPState = GPIO_PIN_RESET;
	TestFirstAidMCPState = GPIO_PIN_SET;
	TestTamper1State = GPIO_PIN_RESET;
	TestTamper2State = GPIO_PIN_RESET;
	TestMainBatteryState = GPIO_PIN_SET;
	TestActivateCount = 0;
	TestDeactivateCount = 0;
	DummyActivateCount = 0;
	DummyDeactivateCount = 0;

	os_Kernel_Sys_Tick = state_change_os_tick_value;

	ret = DM_InputMonitorInit(IN_MON_FIRE_E, CallbackFailure);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	ret = DM_InputMonitorInit(IN_MON_FIRST_AID_E, DummylStateChangeCallback);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	ret = DM_InputMonitorInit(IN_MON_TAMPER1_E, DummylStateChangeCallback);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	ret = DM_InputMonitorInit(IN_MON_TAMPER2_E, DummylStateChangeCallback);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	ret = DM_InputMonitorInit(IN_MON_MAIN_BATTERY_E, DummylStateChangeCallback);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	/* Restart the polling */
	DM_InputMonitorPollingStart();
	CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), true);

	/* Set a fault retrun */
	callback_fail_return = ERR_MESSAGE_FAIL_E;

	os_Kernel_Sys_Tick = state_change_os_tick_value + FIRE_DEACTIVATE_DEBOUNCE_US + 10;

	ret = DM_InputMonitorPoll(IN_MON_FIRE_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_FIRST_AID_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_TAMPER1_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_TAMPER2_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_MAIN_BATTERY_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	CU_ASSERT_EQUAL(TestActivateCount, 1);
	CU_ASSERT_EQUAL(TestDeactivateCount, 0);
	CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), true);

	os_Kernel_Sys_Tick = state_change_os_tick_value + FIRE_DEACTIVATE_DEBOUNCE_US + 20;

	/* Set a successful retrun */
	callback_fail_return = SUCCESS_E;

	ret = DM_InputMonitorPoll(IN_MON_FIRE_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_FIRST_AID_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_TAMPER1_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_TAMPER2_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);
	ret = DM_InputMonitorPoll(IN_MON_MAIN_BATTERY_E);
	CU_ASSERT_EQUAL(ret, SUCCESS_E);

	CU_ASSERT_EQUAL(TestActivateCount, 2);
	CU_ASSERT_EQUAL(TestDeactivateCount, 0);
	CU_ASSERT_EQUAL(DM_InputMonitorGetPollingEnabled(), false);

	/* Check that no other callbacks occured for the other pins */
	CU_ASSERT_EQUAL(DummyActivateCount, 0);
	CU_ASSERT_EQUAL(DummyDeactivateCount, 0);

}
