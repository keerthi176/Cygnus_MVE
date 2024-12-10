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
*  File         : STSyncAlgoTest.c
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
#include "STSyncAlgoTest.h"
#include "MC_SyncPublic.h"
#include "MC_SyncPrivate.h"

typedef struct
{
   SlotIndex_t  SlotIndex;     // The slot index (0 to 2719) of the Timestamp being described.
   McuCtr_t     McuCtr;        // This is the value tracked by the MCU RTC.  (These set the 16 LSBs of the LongCounter.)
} Timestamp_t;



/* Private Functions Prototypes
*************************************************************************************/
static void SyncTest1(void);


/* Global Variables
*************************************************************************************/
/* Table containing the test settings */
CU_TestInfo ST_SyncAlgoTests[] =
{
   { "SyncTest1",    SyncTest1 },

   CU_TEST_INFO_NULL,
};


/* Private Variables
*************************************************************************************/
/* Private Constants
*************************************************************************************/
// CONFIGURE THE TEST PARAMETERS
// Set the number of tracking nodes to the maximum supported by the algorithm
#define NO_OF_TRACKING_NODES (MAX_NO_TRACKING_NODES)  // 4
// The four tracking nodes occur at 1/4 long frame intervals.
#define TRACKING_NODE_SLOT_INCREMENT (SLOTS_PER_LONG_FRAME / NO_OF_TRACKING_NODES)  // 680
// Define the number of simulated slots per tracking slot.
#define SIM_SLOTS_PER_TRACKING_SLOT (20)
// The number of slots between successive interrupts.  Max is 127.
#define SIMULATION_SLOT_INCREMENT (TRACKING_NODE_SLOT_INCREMENT / SIM_SLOTS_PER_TRACKING_SLOT)  // 34
// The number of simulated slots per long frame
#define NO_OF_SIMULATED_SLOTS (SLOTS_PER_LONG_FRAME / SIMULATION_SLOT_INCREMENT)  // 80
// Sets the offset of the tracking node indexes in units of simulated slots. 
#define TRACKING_NODE_SLOT_OFFSET (5)
// Set which iteration of the long frame in which to add the tracking nodes
#define LONG_FRAME_INDEX_TO_ADD_TRACKING_NODES (0)

#define HI_DRIFT_RATE (3)  // See description of TRACKING_NODE_DRIFT
#define LO_DRIFT_RATE (1)  // See description of TRACKING_NODE_DRIFT

// The number of long frames for which the simulation will run.
// (This is limited by the drift rate and the valid range over which drift can occur; otherwise values sent to algorithm will be unrealistic.)
#define LONG_FRAMES_DURATION ((RX_DONE_OFFSET_CYCLES / HI_DRIFT_RATE) - 1) 

#define LONG_FRAME_INCREMENT (0)                // The number of cycles by which the tracking node long frames lengthen.
#define LONG_FRAME_INCREMENT_PERIOD (10)        // The interval between tracking node long frame lengthening events.
#define LOCAL_LONG_FRAME_EXCESS_DURATION (0)    // The number of excess cycles that the local long frame lasts (MCU clock TimingDelta).
#define START_OF_MISSING_PACKETS (20)           // First long-frame index at which packets not received.
#define END_OF_MISSING_PACKETS (22)             // Last  long-frame index at which packets not received.  Set lower than START to disable.
#define TRACKING_NODE_INITIAL_ERROR {0,0,0,0}   // Initial timing TimingDelta of the tracking nodes.

// Set the maximum permitted averaged discrepencies between calculated and expected time of next slot (TEST 10).
#define AVG_ERROR_TEST_THRESHOLD_PRESENT 3                            // This value has been arrived at by observing the behaviour of the working algorithm.
#define AVG_ERROR_TEST_THRESHOLD_MISSING ((HI_DRIFT_RATE * 7) + 2.0)  // This value has been arrived at by observing the behaviour of the working algorithm.

// Define the relative duration errors of the tracking nodes' long frames.
#define TRACKING_NODE_DRIFT {HI_DRIFT_RATE, LO_DRIFT_RATE, -LO_DRIFT_RATE, -HI_DRIFT_RATE+1}  // The +1 is to make the drifts non-symmetrical.

// CONFIGURE THE DEBUG OUTPUT
//#define PRINT_DEBUG                           // Define this to turn on the debug printouts
#define SHOW_ALL_NODES (true)                 // Set this to control whether all slots or just tracking slots are displayed.
#define PRINT_DEBUG_FROM_OUTER_LOOP_INDEX (0)   // Use this to prevent debug output until the specified LongFrameCounter.


/* Public Variables
*************************************************************************************/
// Declaring these parameters as public, as this usefully allows their values to be 
// seen whilst debugging the main code.
long int LongFrameCounter = 0;
int InnerLoopIndex = 0;



/* Private Functions Definitions
*************************************************************************************/
// Note, whilst the function below is defined as static (private), the global (public)
// array ST_SyncAlgoTests[] contains a pointer to this function, thus making it 
// publically accessible.

/*************************************************************************************/
/**
* Test1
* Tests the MC_SyncAlgorithm.c code.
* See descriptions of TEST 1 to TEST 16 in the code itself.
*
* The test is fully configured using the #defines above.
*
* @return - none

*/
static void SyncTest1(void)
{
   Timestamp_t  CurrentTimestamp;
   NextTimestamp_t  NextTimestamp;
   NextTimestamp_t* pNextTimestamp = NULL;
   Timestamp_t  ReferenceNextTimestamp;
   int64_t      aCount[NO_OF_TRACKING_NODES] = TRACKING_NODE_INITIAL_ERROR;
   uint16_t     TrackingNodeIndex = NO_OF_TRACKING_NODES - 1;
   uint32_t     SlotOffset = 0;
   int64_t      McuCtrSlot0 = 0;
   int64_t      McuCtrVal = 0;
   int64_t      RefLongCtr = 0;
   int32_t      TimingDelta = 0;
   int32_t      SumOfAllSlotsDeltas = 0;
   int32_t      SumOfTrackingNodesDeltas = 0;
   float        AvgOfAllSlotsDeltas = 0;
   float        AvgOfTrackingNodesDeltas = 0;
   uint32_t     InputCyclesPerLongFrame = 0;
   double       LongFrameDurationRatio = 0;
   int32_t      LocalCyclesPerLongFrame = CYCLES_PER_LONGFRAME + LOCAL_LONG_FRAME_EXCESS_DURATION;
   int32_t      TnRelativeDriftPerLongFrame[NO_OF_TRACKING_NODES] = TRACKING_NODE_DRIFT; // These values are relative to local long frame duration
   bool         IsTrackingSlot = false;
   bool         TrackingIsActive = false;
   bool         TestPassed = true;
   ErrorCode_t  ErrorCode = SUCCESS_E;
   uint16_t     SlotIndex = 0;
   bool         FreqLockAchieved = false;
   bool         FreqLockShouldBeActive = false;

   // Initialise timestamp variables
   memset(&CurrentTimestamp, 0, sizeof(CurrentTimestamp));
   memset(&NextTimestamp, 0, sizeof(NextTimestamp));
   memset(&ReferenceNextTimestamp, 0, sizeof(ReferenceNextTimestamp));

   // TEST 1
   // Add four nodes.  All should succeed.
   for (SlotIndex = 0; SlotIndex < NO_OF_TRACKING_NODES; SlotIndex++)
   {
      ErrorCode = MC_AddTrackingNode(SlotIndex);
      TestPassed = TestPassed && (ErrorCode == SUCCESS_E);
      CU_ASSERT_EQUAL(TestPassed, true);
   }
   if (!TestPassed)
   {
      printf("\nSYNC ALGO TEST 1 FAILED.  Testing calls to MC_AddTrackingNode.");
   }

   // TEST 2
   // Try adding another node.  Should return error code.
   TestPassed = TestPassed && (MC_AddTrackingNode(SlotIndex) == ERR_ALL_TRACKING_NODES_ALLOCATED_E);
   CU_ASSERT_EQUAL(TestPassed, true);
   if (!TestPassed)
   {
      printf("\nSYNC ALGO TEST 2 FAILED.  Additional call to MC_AddTrackingNode succeeded, which is not correct.");
   }

   // TEST 3
   // Delete a node.  Should succeed.
   TestPassed = TestPassed && (MC_DeleteTrackingNode(0) == SUCCESS_E);
   CU_ASSERT_EQUAL(TestPassed, true);
   if (!TestPassed)
   {
      printf("\nSYNC ALGO TEST 3 FAILED.  Testing call to MC_DeleteTrackingNode failed.");
   }

   // TEST 4
   // Try adding another node again.  Should pass this time.
   TestPassed = TestPassed && (MC_AddTrackingNode(SlotIndex) == SUCCESS_E);
   CU_ASSERT_EQUAL(TestPassed, true);
   if (!TestPassed)
   {
      printf("\nSYNC ALGO TEST 4 FAILED.  Testing call to MC_AddTrackingNode failed.");
   }

   // TEST 5
   // Try getting an estimated timestamp, but with the pointer set to NULL.  Should return error code.
   pNextTimestamp = NULL;
   TestPassed = TestPassed && (MC_GetNextTimestamp(SlotIndex, IsTrackingSlot, SlotOffset, pNextTimestamp) == ERR_INVALID_POINTER_E);
   CU_ASSERT_EQUAL(TestPassed, true);
   if (!TestPassed)
   {
      printf("\nSYNC ALGO TEST 5 FAILED.  Invalid call to MC_GetNextTimestamp didn't return INVALID_POINTER_E.");
   }

   // TEST 6
   // Reset the sync database, to completely reset the sync algorithm.  Should succeed.
   TestPassed = TestPassed && (MC_ResetSyncDatabase() == SUCCESS_E);
   CU_ASSERT_EQUAL(TestPassed, true);
   if (!TestPassed)
   {
      printf("\nSYNC ALGO TEST 7 FAILED.  Call to MC_ResetSyncDatabase didn't return SUCCESS_E.");
   }

   // TESTS 7 to 10
   // Run a long simulation in which the tracking nodes' timing slowly drift apart.
   for (LongFrameCounter = 0; LongFrameCounter < LONG_FRAMES_DURATION; LongFrameCounter++)
   {
      // Initialise variables used to sum values
      SumOfTrackingNodesDeltas = 0;
      SumOfAllSlotsDeltas = 0;

      // Loop on the the active slots.  ActiveSlotIdx is multiplied by SIMULATION_SLOT_INCREMENT to give actual active SlotIndex(s).
      for (InnerLoopIndex = 0; InnerLoopIndex < NO_OF_SIMULATED_SLOTS; InnerLoopIndex++)
      {
         // Set the slot index value for this tracking node.
         SlotIndex = (InnerLoopIndex * SIMULATION_SLOT_INCREMENT);

         // Decide if this slot is a tracking node, and if so increment the TrackingNodeIndex (modulo 4).
         IsTrackingSlot = (0 == ((InnerLoopIndex - TRACKING_NODE_SLOT_OFFSET) % SIM_SLOTS_PER_TRACKING_SLOT));

         // Calculate the node's McuCtrSlot0 event for this long frame.  This is the time origin for calculating Ctr values for other slots.
         if (IsTrackingSlot)
         {
            TrackingNodeIndex = (TrackingNodeIndex + IsTrackingSlot) % NO_OF_TRACKING_NODES;

            // Calculate the current duration of the long frame of the current tracking node.
            InputCyclesPerLongFrame = LocalCyclesPerLongFrame + TnRelativeDriftPerLongFrame[TrackingNodeIndex];

            // Duration of tracking nodes can vary with time, so array values are integrated (rather than being a scalar multiple of time).
            aCount[TrackingNodeIndex] += (int64_t)(InputCyclesPerLongFrame);
            McuCtrSlot0 = aCount[TrackingNodeIndex];
         }
         else
         {
            // Non-tracking nodes duration are equal to that of this receiving node.
            McuCtrSlot0 = (LongFrameCounter + 1) * (CYCLES_PER_LONGFRAME + LOCAL_LONG_FRAME_EXCESS_DURATION);
         }

         // Calculate the value of the Tracking Node's RxDone event for the current slot
         LongFrameDurationRatio = (float)InputCyclesPerLongFrame / CYCLES_PER_LONGFRAME;
         SlotOffset = (uint32_t)((CYCLES_PER_SLOT * SlotIndex) * LongFrameDurationRatio);
         McuCtrVal = (McuCtrSlot0 + (int64_t)SlotOffset);

         // Set the current timestamp.  Use only the first 16-bits of the McuCtrVal.
         CurrentTimestamp.McuCtr.Value = (uint16_t)GET_LSBS(McuCtrVal);
         CurrentTimestamp.McuCtr.Valid = true;
         CurrentTimestamp.SlotIndex.Value = SlotIndex;
         CurrentTimestamp.SlotIndex.Valid = true;

         // Simulate a period of missing packets
         if ((LongFrameCounter >= START_OF_MISSING_PACKETS && LongFrameCounter <= END_OF_MISSING_PACKETS))
         {
            // Simulate a RxTimeout in the LoRa Modem, giving an unknown timestamp.
            CurrentTimestamp.McuCtr.Value = 0;
            CurrentTimestamp.McuCtr.Valid = false;
         }

         // Calculate the reference values for the next call to this function.
         ReferenceNextTimestamp.SlotIndex.Value = (SlotIndex + SIMULATION_SLOT_INCREMENT) % SLOTS_PER_LONG_FRAME;
         RefLongCtr = (McuCtrVal + ((InputCyclesPerLongFrame * SIMULATION_SLOT_INCREMENT) / SLOTS_PER_LONG_FRAME));
         ReferenceNextTimestamp.McuCtr.Value = (uint16_t)GET_LSBS(RefLongCtr);

         // In the specified long frame, add the tracking nodes.
         if ((LONG_FRAME_INDEX_TO_ADD_TRACKING_NODES == LongFrameCounter) && IsTrackingSlot)
         {
            // TEST 7
            // Add a tracking node and test it returns success.
            TestPassed = TestPassed && (MC_AddTrackingNode(SlotIndex) == SUCCESS_E);
            if (!TestPassed)
            {
               printf("\nSYNC ALGO TEST 7 FAILED.  Call to MC_AddTrackingNode didn't return SUCCESS_E.");
               printf("\nLong Frame Index = %5ld: Inner Loop Index = %ld ", LongFrameCounter, InnerLoopIndex);
               break;
            }
            TrackingIsActive = true;
         }

         // Once we get to exactly one long frame after the first tracking node was added,
         // set flag to show that the frequency lock should now be achieved by the algorithm.
         if ((LONG_FRAME_INDEX_TO_ADD_TRACKING_NODES < LongFrameCounter) && IsTrackingSlot)
         {
            FreqLockShouldBeActive = true;
         }

         if (TrackingIsActive)
         {
            uint16_t NextSlotIndex = (SlotIndex + SIMULATION_SLOT_INCREMENT) % SLOTS_PER_LONG_FRAME;
            ErrorCode = MC_GetNextTimestamp(SlotIndex, IsTrackingSlot, NextSlotIndex, &NextTimestamp);

            TestPassed = TestPassed && ErrorCode == SUCCESS_E;
            CU_ASSERT_EQUAL(TestPassed, true);
            if (!TestPassed)
            {
               printf("\nSYNC ALGO TEST FAILED.  Call to MC_GetNextTimestamp didn't return SUCCESS_E.");
               printf("\nLong Frame Index = %5ld: Inner Loop Index = %ld ", LongFrameCounter, InnerLoopIndex);
               break;
            }

            if (IsTrackingSlot)
            {
               ErrorCode = MC_ProcessSyncMessage(CurrentTimestamp.SlotIndex.Value, CurrentTimestamp.McuCtr.Value, CurrentTimestamp.McuCtr.Valid, &FreqLockAchieved);

               TestPassed = TestPassed && ErrorCode == SUCCESS_E;
               CU_ASSERT_EQUAL(TestPassed, true);
               if (!TestPassed)
               {
                  printf("\nSYNC ALGO TEST FAILED.  Second call to MC_UpdateSyncDatabase didn't return SUCCESS_E.");    // Remove "first/second call" later.
                  printf("\nLong Frame Index = %5ld: Inner Loop Index = %ld ", LongFrameCounter, InnerLoopIndex);
                  break;
               }
            }

            // TEST 8
            // Check that the Frequency Lock reported as TRUE only when it should be, and as FALSE at all other times.
            TestPassed = TestPassed && ((FreqLockShouldBeActive && FreqLockAchieved) || (!FreqLockShouldBeActive && !FreqLockAchieved));
            CU_ASSERT_EQUAL(TestPassed, true);
            if (!TestPassed)
            {
               printf("\nSYNC ALGO TEST 8 FAILED.  Frequency Lock not reported correctly.");
               printf("\nLong Frame Index = %5ld: Inner Loop Index = %ld ", LongFrameCounter, InnerLoopIndex);
               break;
            }

            // TEST 9
            // Now process the results of the call to MC_UpdateSyncDatabase()
            // If function didn't return success, mark this as a test fail.
            if (SUCCESS_E > START_OF_ERROR_CODES_E)
            {
               TestPassed = TestPassed && false;
               printf("\nSYNC ALGO TEST 9 FAILED.  Call to MC_UpdateSyncDatabase didn't return SUCCESS_E.");
               printf("\nLong Frame Index = %5ld: Inner Loop Index = %ld ", LongFrameCounter, InnerLoopIndex);
               break;
            }
            else  // Otherwise, process the results
            {
               // Calculate TimingDelta, which is the difference between the test harness next timestamp and that returned by the function.
               TimingDelta = ReferenceNextTimestamp.McuCtr.Value - NextTimestamp.McuCtr;
               TimingDelta += MCU_CTR_LSB_WRAP * (TimingDelta < -MCU_CTR_LSB_HALF);  // Correct for wrapping TimingDelta at one end of MCU Counter scale.
               TimingDelta -= MCU_CTR_LSB_WRAP * (TimingDelta > MCU_CTR_LSB_HALF);   // Correct for wrapping TimingDelta at other end of MCU Counter scale.

               // Sum the TimingDeltas, so that we can later use division to get the average timing deltas for this long frame. 
               SumOfAllSlotsDeltas += TimingDelta;
               SumOfTrackingNodesDeltas += TimingDelta * (bool)IsTrackingSlot;

               if ((IsTrackingSlot || SHOW_ALL_NODES) && LongFrameCounter >= PRINT_DEBUG_FROM_OUTER_LOOP_INDEX)
               {
#ifdef PRINT_DEBUG
                  printf("\nLF: %4d, Reference:%6d, Calculated:%6d, Timing Delta:%4d", LongFrameCounter,
                     ReferenceNextTimestamp.McuCtr.Value, NextTimestamp.McuCtr, TimingDelta);
#endif
               }

               if (IsTrackingSlot && SHOW_ALL_NODES && LongFrameCounter >= PRINT_DEBUG_FROM_OUTER_LOOP_INDEX)
               {
#ifdef PRINT_DEBUG
                  printf("   Tracking Slot");  // To indicate a tracking slot.
#endif
               }

               // If this was the last active slot of the long frame, calculate the average delta values
               if ((NO_OF_SIMULATED_SLOTS - 1) == InnerLoopIndex)
               {
                  AvgOfTrackingNodesDeltas = (float)SumOfTrackingNodesDeltas / NO_OF_TRACKING_NODES;
                  AvgOfAllSlotsDeltas = ((float)SumOfAllSlotsDeltas * SIMULATION_SLOT_INCREMENT) / SLOTS_PER_LONG_FRAME;
#ifdef PRINT_DEBUG
                  if (LongFrameCounter >= PRINT_DEBUG_FROM_OUTER_LOOP_INDEX)
                  {
                     printf("\nSum of Tracking Node deltas   = %3ld", SumOfTrackingNodesDeltas);
                     printf("   Average Tracking Node deltas = %3.2f", AvgOfTrackingNodesDeltas);
                     printf("\nSum of All Node deltas        = %3ld", SumOfAllSlotsDeltas);
                     printf("   Average All Node deltas      = %3.2f\n", AvgOfAllSlotsDeltas);
                  }
#endif
               }

               // TEST 10
               // Test that the averaged values are within test limits for the period when no packets are missing
               if (LongFrameCounter > 0 && (NO_OF_SIMULATED_SLOTS - 1) == InnerLoopIndex && 
                     (LongFrameCounter < START_OF_MISSING_PACKETS || LongFrameCounter > END_OF_MISSING_PACKETS) )
               {
                  // After the packets have been missing for a while, we need to relax the pass threshold
                  // slightly for two long frames to allow the sync algorithm time to recover.
                  uint16_t SettlingTimeScalingFactor = 1;
                  if ((LongFrameCounter == END_OF_MISSING_PACKETS + 1) || (LongFrameCounter == END_OF_MISSING_PACKETS + 2))
                  {
                     SettlingTimeScalingFactor = HI_DRIFT_RATE;
                  }

                  TestPassed = TestPassed && fabs(AvgOfTrackingNodesDeltas) <= (AVG_ERROR_TEST_THRESHOLD_PRESENT * SettlingTimeScalingFactor);
                  CU_ASSERT_EQUAL(TestPassed, true);
               }

               // Test that the averaged values are within test limits for the period when packets are missing
               if ((NO_OF_SIMULATED_SLOTS - 1) == InnerLoopIndex && LongFrameCounter >= START_OF_MISSING_PACKETS && LongFrameCounter <= END_OF_MISSING_PACKETS)
               {
                  TestPassed = TestPassed && fabs(AvgOfTrackingNodesDeltas) < (AVG_ERROR_TEST_THRESHOLD_MISSING);
                  CU_ASSERT_EQUAL(TestPassed, true);
               }

               if (!TestPassed)
               {
                  printf("\nSYNC ALGO TEST 10 FAILED.  Average measurements outside pass threshold.");
                  printf("\nLong Frame Index = %5ld: Inner Loop Index = %ld ", LongFrameCounter, InnerLoopIndex);
                  break;
               }
            }
         }

      } // End of InnerLoopIndex loop.

      // If the test has failed, terminate the outer loop.
      if (!TestPassed)
      {
         break;
      }
   } // End of LongFrameCounter loop

   // TEST 11
   for (int x = 0; x < NO_OF_TRACKING_NODES; x++)
   {
      // Delete the tracking nodes, checking function returns success each time.
      SlotIndex = (TRACKING_NODE_SLOT_INCREMENT * x) + (TRACKING_NODE_SLOT_OFFSET * SIMULATION_SLOT_INCREMENT);
      ErrorCode_t ret = MC_DeleteTrackingNode(SlotIndex);
	  CU_ASSERT_EQUAL(ret, SUCCESS_E);
	  TestPassed = TestPassed && (ret == SUCCESS_E);
     CU_ASSERT_EQUAL(TestPassed, true);
   }
   if (!TestPassed)
   {
      printf("\nSYNC ALGO TEST 11 FAILED.  Calls to MC_DeleteTrackingNode didn't return SUCCESS_E.");
   }

   // TEST 12 - This one is to test a failure mode that was discovered whilst running the actual hardware: CYG2-843.
   // The values in the function calls are exactly those that occured on the hardware, but the returned values have been adapted for 85-second long frames, as used in the test harness.
   MC_AddTrackingNode(79);
   MC_ProcessSyncMessage(79, 21073, true, &FreqLockAchieved);
   MC_GetNextTimestamp(5, false, 7, &NextTimestamp);
   // Hardware gave a timestamp value of 49745, which is 1 long frame too early (a.k.a. 1.5 seconds too late).
   // With 85 second long frames, the expected return value is 593.  Without the fix the return value was 49745.
   CU_ASSERT_EQUAL(NextTimestamp.McuCtr, 593);  
   CU_ASSERT_EQUAL(NextTimestamp.SlotIndex, 7);

   // Print the outcome of the test.
   printf("Synchronisation Algorithm Test Result = %s\n", TestPassed ? "PASS" : "FAIL");

   CU_ASSERT_EQUAL(TestPassed, true);
}

