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
*  File         : THTestHarness.c
*
*  Description  : Test engine based the C-Unit testing framework
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



/* User Include Files
*************************************************************************************/
#include "Console.h"
#include "Automated.h"
#include "THSuiteManager.h"



/* Private Functions Prototypes
*************************************************************************************/



/* Global Variables
*************************************************************************************/



/* Private Variables
*************************************************************************************/



/*************************************************************************************
*
*  Function Name  : main
*  Parameters     : argc - Number of input arguments
*                   argv - Reference to input argument array
*  Return  Value  : 0 if successful, non-zero when error occured
*  Description    : Runs the C-Unit test framework
*
*************************************************************************************/
int main(int argc, char* argv[])
{
  CU_BOOL RunTests = CU_TRUE;
  CU_BOOL ConsoleMode = CU_FALSE;

  /* Ignore test framework errors */
  CU_set_error_action(CUEA_IGNORE);

  if (argc > 1) 
  {
    if (0 == strcmp("-a", argv[1])) 
    {
      /* Runs all tests in automated mode */
    }
    else if (0 == strcmp("-c", argv[1])) 
    {
      /* Runs tests from console interface */
      ConsoleMode = CU_TRUE;
    }
    else 
    {
      printf("\nUsage:    TestHarness [option]\n\n"
             "Options:  -a Runs all tests in automated mode [default]\n"
             "          -c Run tests in console mode\n"
             "          -? Print help message\n\n");

      /* Do not run tests after printing usage */
      RunTests = CU_FALSE;
    }
  }

  /* Start testing */
  if (CU_TRUE == RunTests) 
  {
    if (CU_initialize_registry()) 
    {
      printf("Initialization of test registry failed\n");
    }
    else
    {
      TH_AddTests();

      if (CU_TRUE == ConsoleMode)
      {
        /* Console interface allows customisation of tests */
        CU_console_run_tests();
      }
      else
      {
        /* Runs in automated interface which generates test result XML */
        CU_set_output_filename("TestResults");
        CU_list_tests_to_file();
        CU_automated_run_tests();
      }

      CU_cleanup_registry();
    }
  }

  return 0;
}
