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
*  File         : THSuiteManager.c
*
*  Description  : Stores the test suite information
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
#include "THSuiteManager.h"
#include "STSyncAlgoTest.h"
#include "STPUPTest.h"
#include "STChanHopSeqGenTest.h"
#include "STNVMTest.h"
#include "STInputMonitorTest.h"
#include "STackManagementTest.h"
#include "STRBUApplicationTest.h"
#include "STNCUApplicationTest.h"
#include "STSessionManagementTest.h"
#include "STStateMachineTest.h"
#include "STDeviceTest.h"
#include "STMACTest.h"
#include "STCommandProcessorTest.h"
#include "STDataManagerTest.h"
#include "STTestModeTest.h"
#include "STBlfBoot.h"
#include "STBlf_AppUpdater.h"
#include "STATCommandTest.h"
#include "STLEDTest.h"
#include "STSACHManagementTest.h"
#include "STI2CTest.h"
#include "STSVITest.h"
#include "STDMSerialPortTest.h"
#include "STCIEQueueManagerTest.h"

/* Private Functions Prototypes
*************************************************************************************/



/* Global Variables
*************************************************************************************/



/* Private Variables
*************************************************************************************/
/* Test suite settings for software tests */
static CU_SuiteInfo SoftwareTests[] = 
{
   { "SyncAlgoTest",      NULL, NULL, NULL, NULL, ST_SyncAlgoTests },
   { "PUPTest",           NULL, NULL, NULL, NULL, ST_PUPTests },
   { "ChanHopSeqGenTest", NULL, NULL, NULL, NULL, ST_ChanHopSeqGenTests },
   { "NVMTest",           NV_TestSuiteInit, NV_TestSuiteCleanup, NULL, NULL, ST_NVMTests },
   { "InputMonitorTest",  NULL, NULL, NULL, NULL, ST_InputMonitorTests },
   { "AckManagementTest", NULL, NULL, NULL, NULL, ST_AckMgrTests },
   { "RBUAppTests",       NULL, NULL, NULL, NULL, ST_RBUAppTests },
   { "SessMgmtTests",     NULL, NULL, NULL, NULL, ST_SessMgmtTests },
   { "StateMachineTests", NULL, NULL, NULL, NULL, ST_StateMachineTests },
   { "DeviceTests",       NULL, NULL, NULL, NULL, ST_DeviceTests },
   { "MACTests",          NULL, NULL, NULL, NULL, ST_MACTests },
   { "CommandProcTest",   ST_CmdProc_TestSuiteInit, ST_CmdProc_TestSuiteCleanup, NULL, NULL, ST_CmdProcTests },
   { "DataManagerTest",   DataManager_TestSuiteInit, DataManager_TestSuiteCleanup, NULL, NULL, ST_DataManagerTests },
   { "TestModeTest",      NULL, NULL, NULL, NULL, ST_TestModeTests },
   { "BLF Boot",          BlfBoot_TestSuiteInit, BlfBoot_TestSuiteCleanup, NULL, NULL, ST_BlfBootTests },
   { "BLF AppUpdater",    BlfAppUpdater_TestSuiteInit, BlfAppUpdater_TestSuiteCleanup, NULL, NULL, ST_BlfAppUpdaterTests },
   { "ATCommandTests",    NULL, NULL, NULL, NULL, ST_ATCommandTests },
   { "LEDTest",           ST_LEDTestSuiteInit, ST_LEDTestSuiteCleanup, NULL, NULL, ST_LEDTests },
   { "SachManagementTests",NULL, NULL, NULL, NULL, ST_SACHMgrTests },
   { "I2CTests",          NULL, NULL, NULL, NULL, ST_I2CTests },
   { "SVITests",          NULL, NULL, NULL, NULL, ST_SVITests },
   { "SerialPortTests",   NULL, NULL, NULL, NULL, ST_SerialPortTests },
   { "NCUAppTests",       NULL, NULL, NULL, NULL, ST_NCUAppTests },
   { "CieQueueTests",       NULL, NULL, NULL, NULL, ST_CieQueueMgrTests },

   CU_SUITE_INFO_NULL,
};


/*************************************************************************************
*
*  Function Name  : TH_AddTests
*  Parameters     : void
*  Return  Value  : None
*  Description    : Registers the test suites to the C-Unit framework
*
*************************************************************************************/
void TH_AddTests(void)
{
  CU_ErrorCode Result = CUE_SUCCESS;

  assert(NULL != CU_get_registry());
  assert(CU_FALSE == CU_is_test_running());

  /* Register test suites */
  Result = CU_register_nsuites(1, SoftwareTests);

  if (CUE_SUCCESS != Result)
  {
    fprintf(stderr, "Suite registration failed - %s\n", CU_get_error_msg());
    exit(EXIT_FAILURE);
  }
}
