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
*  File         : STChanHopSeqGenTest.c
*
*  Description  : Implementation for the synchronisation tests
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
#include "STChanHopSeqGenTest.h"
#include "MC_ChanHopSeqGenPublic.h"
#include "MC_ChanHopSeqGenPrivate.h"
#include "MC_SyncPublic.h"


/* Private Functions Prototypes
*************************************************************************************/
static void ChanHopSeqGenTest1(void);
static ErrorCode_t TestChanHopSeq2(
   Channel_t * pChanHopSeqArray,
   uint16_t   SeqLength,
   Channel_t NumChans,
   uint16_t MinChanInterval,
   uint32_t * pSumMinCount,
   uint32_t * pSumMaxCount,
   uint32_t * pSumChannelCount);


/* Global Variables
*************************************************************************************/
/* Table containing the test settings */
CU_TestInfo ST_ChanHopSeqGenTests[] =
{
   { "ChanHopSeqGenTest1",    ChanHopSeqGenTest1 },

   CU_TEST_INFO_NULL,
};


/* Private Variables
*************************************************************************************/
/* Private Constants
*************************************************************************************/
// CONFIGURE THE TEST PARAMETERS
#define DCH_SEQ_LEN            (16)
#define RACH_SEQ_LEN           (68)
#define DLCCH_SEQ_LEN          (68)
#define MIN_CHAN_INTERVAL      (4)   // Minimum number of channels separation to give at least 1.0 MHz hops
#define NUM_BYTES_IN_PRBS_SEED (16)

// CONFIGURE THE DEBUG OUTPUT
//#define PRINT_DEBUG                           // Define this to turn on the debug printouts

/* Public Variables
*************************************************************************************/
// Declaring these parameters as public, as this usefully allows their values to be 
// seen whilst debugging the main code.

extern Channel_t    aDchChanHopSeq[DCH_SEQ_LEN];
Channel_t    aDlcchChanHopSeq[DLCCH_SEQ_LEN];
Channel_t    aRachChanHopSeq[RACH_SEQ_LEN];


/* Private Functions Definitions
*************************************************************************************/
// Note, whilst the function below is defined as static (private), the global (public)
// array ST_SyncAlgoTests[] contains a pointer to this function, thus making it 
// publically accessible.

/*************************************************************************************/
/**
* Test1
* Tests the MC_ChanHopSeqGen.c code.
*
* @return - none

*/
static void ChanHopSeqGenTest1(void)
{
   bool         TestPassed = true;
   ErrorCode_t  ErrorCode = SUCCESS_E;
   uint8_t      aPRBSSeed[NUM_BYTES_IN_PRBS_SEED];
   uint16_t     LastRandNum = 0;
   uint16_t     FirstRandNum = 0;
   PRBSGeneratorSeed_t  NumBytesInPRBSSeed = { NUM_BYTES_IN_PRBS_SEED, aPRBSSeed };
   static bool  PRBSGenIsSeeded = false;

   // Initialise the PRBS seed with random bytes
   srand(1);
   for (int32_t i = 0; i < NUM_BYTES_IN_PRBS_SEED; i++)
   {
      aPRBSSeed[i] = rand() % 256;
   }

   // Try calling the PRBS generator without having first seeded it.
   // We cannot un-seed the PRBS generator, so if ChanHopSeqGenTest1 is re-run from TestHarness.exe this test omitted from subsequent runs.
   if (false == PRBSGenIsSeeded)
   {
      ErrorCode = MC_GetRandNumber(&FirstRandNum, NULL);
      if (ERR_RAND_NUM_GENERATOR_NOT_SEEDED_E != ErrorCode)
      {
         printf("\nPRBS generator failed to report that it was called unseeded.");
         TestPassed = false;
      }
      PRBSGenIsSeeded = true;
   }

   // Seed the PRBS generator.
   ErrorCode = MC_GetRandNumber(&FirstRandNum, &NumBytesInPRBSSeed);
   if (ErrorCode)
   {
      printf("\nUnexpected error code when seeding the PRBS generator.");
      TestPassed = false;
   }
   if (23666 != FirstRandNum)
   {
      printf("\nUnexpected random number returned when seeding the PRBS generator.");
      TestPassed = false;
   }

   // Test that the PRBS generator doesn't regenerate the first value in less than 65535 iterations.
   for (int32_t i = 0; i < 65534; i++)
   {
      ErrorCode = MC_GetRandNumber(&LastRandNum, NULL);
      if (ErrorCode)
      {
         printf("\nUnexpected error code when calling the PRBS generator.");
         TestPassed = false;
      }
      if (LastRandNum == FirstRandNum)
      {
         printf("\nPRBS Sequence repeated within 65535 iterations.");
         TestPassed = false;
         break;
      }
   }

   // Test that the PRBS generator regenerates the first value on the 65,535'th iteration.
   ErrorCode = MC_GetRandNumber(&LastRandNum, NULL);
   if (ErrorCode)
   {
      printf("\nUnexpected error code when calling the PRBS generator.");
      TestPassed = false;
   }
   if (LastRandNum != FirstRandNum)
   {
      printf("\nPRBS Sequence failed to repeat in 65535 iterations.");
      TestPassed = false;
   }

   // Generate DCH, RACH and DLCCH hopping sequences 1000 times each.
   // (This test has also been run for 100,000 loop iterations, and also passed.)
   for (int32_t i = 0; i < 1000; i++)
   {
      // Reset all prior to each run run
      memset(aDchChanHopSeq, 0, DCH_SEQ_LEN * sizeof(Channel_t));
      memset(aRachChanHopSeq, 0, RACH_SEQ_LEN * sizeof(Channel_t));
      memset(aDlcchChanHopSeq, 0, DLCCH_SEQ_LEN * sizeof(Channel_t));

      ErrorCode = MC_GenChanHopSeq(aDlcchChanHopSeq, DLCCH_SEQ_LEN, NUM_HOPPING_CHANS, MIN_CHAN_INTERVAL);
      if (ErrorCode)
      {
         printf("Call to MC_GenChanHopSeq() for DlcchChanHopSeq returned error code on iteration i=%d of the test loop.\n", i);
         TestPassed = false;
         break;
      }

      ErrorCode = MC_GenChanHopSeq(aRachChanHopSeq, RACH_SEQ_LEN, NUM_HOPPING_CHANS, MIN_CHAN_INTERVAL);
      if (ErrorCode)
      {
         printf("Call to MC_GenChanHopSeq() for RachChanHopSeq returned error code on iteration i=%d of the test loop.\n", i);
         TestPassed = false;
         break;
      }

      ErrorCode = MC_GenChanHopSeq(aDchChanHopSeq, DCH_SEQ_LEN, NUM_HOPPING_CHANS, MIN_CHAN_INTERVAL);
      if (ErrorCode)
      {
         printf("Call to MC_GenChanHopSeq() for DchChanHopSeq returned error code on iteration i=%d of the test loop.\n", i);
         TestPassed = false;
         break;
      }
   }

#ifdef PRINT_DEBUG
   printf("\nDLCCH Max/Min Ratio = %2.3f, RACH Max/Min Ratio = %2.3f, DCH Max/Min Ratio = %2.3f",
      (float)DlcchSumMaxCount / (float)DlcchSumMinCount,
      (float)RachSumMaxCount / (float)RachSumMinCount,
      (float)DchSumMaxCount / (float)DchSumMinCount);
#endif

   // Print the outcome of the test.
   printf("Channel Hopping Sequence Generation Test Result = %s\n", TestPassed ? "PASS" : "FAIL");

   CU_ASSERT_EQUAL(TestPassed, true);
}
