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
*  File         : STMainTest.c
*
*  Description  : Implementation for the software main tests
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>



/* User Include Files
*************************************************************************************/
#include "CUnit.h"
#include "STMainTest.h"



/* Private Functions Prototypes
*************************************************************************************/
static void Test1(void);



/* Global Variables
*************************************************************************************/
/* Table containing the test settings */
CU_TestInfo ST_MainTests[] =
{
  { "Test1",    Test1 },

  CU_TEST_INFO_NULL,
};



/* Private Variables
*************************************************************************************/



/*************************************************************************************/
/**
* Test1
* Example of a test
*
* no params
*
* @return - none

*/
static void Test1(void)
{
  //CU_ASSERT_EQUAL(a, b);
}

