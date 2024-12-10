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
*  File         : STSACHManagementTest.c
*
*  Description  : Implementation for testing the SACH Management functions 
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
#include "CO_Defines.h"
#include "STSACHManagementTest.h"
#include "SM_StateMachine.h"
#include "MC_SACH_Management.h"
#include "MC_AckManagement.h"
#include "MC_SessionManagement.h"


/* Private Functions Prototypes
*************************************************************************************/
static void SACH_InitialisationTest(void);
static void SACH_ScheduleMessageTest(void);
static void SACH_UpdateSACHManagementTest(void);
static void SACH_DispatchMessageTest(void);
static void SACH_GetSASSlotTypeTest(void);
static void SACH_RedirectMessagesTest(void);

/* definitions
*************************************************************************************/
#define RACH_MSG_DATA_SIZE 20
uint8_t MeshMessageData1[RACH_MSG_DATA_SIZE] = { 0x10,0x01,0x20,0x20,0x43,0x03,0x40,0x40,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
uint8_t MeshMessageData2[RACH_MSG_DATA_SIZE] = { 0x10,0x01,0x20,0x20,0x53,0x03,0x40,0x40,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
uint8_t MeshMessageData3[RACH_MSG_DATA_SIZE] = { 0x10,0x01,0x20,0x20,0x63,0x03,0x40,0x40,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
uint8_t ZoneMessageData1[RACH_MSG_DATA_SIZE] = { 0x10,0x01,0x20,0x20,0x73,0x03,0x40,0x40,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
uint8_t ZoneMessageData2[RACH_MSG_DATA_SIZE] = { 0x10,0x01,0x20,0x20,0x83,0x03,0x40,0x40,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
uint8_t ZoneMessageData3[RACH_MSG_DATA_SIZE] = { 0x10,0x01,0x20,0x20,0x93,0x03,0x40,0x40,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
uint8_t ChildMessageData1[RACH_MSG_DATA_SIZE] = { 0x10,0x01,0x20,0x20,0x03,0x03,0x40,0x40,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
uint8_t ChildMessageData2[RACH_MSG_DATA_SIZE] = { 0x10,0x01,0x20,0x20,0x13,0x03,0x40,0x40,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
uint8_t ChildMessageData3[RACH_MSG_DATA_SIZE] = { 0x10,0x01,0x20,0x20,0x23,0x03,0x40,0x40,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };

#define UNIT_0_MESH_SLOT 4
#define UNIT_0_ZONE_SLOT 24
#define UNIT_0_CHILD_SLOT 44
#define UNIT_1_MESH_SLOT 84
#define UNIT_1_ZONE_SLOT 104
#define UNIT_1_CHILD_SLOT 124

#define SYSTEM_ID 0x12345678

/* Global Variables
*************************************************************************************/
extern SASMsgQueue_t sasQueue[SACH_NUMBER_OF_SAS_QUEUES];
extern uint16_t unitNumberWithinZone;
extern uint16_t unitAddress;
extern CO_State_t gSM_CurrentMeshState;

extern UnackedMsgQueue_t unackQueue[ACK_NUMBER_OF_ACKNOWLEDGEMENT_QUEUES];//from Ack Manager
extern bool MC_SACH_DispatchMessage(ScheduledRachQueueID_t queueID, AcknowledgedQueueID_t rach_channel);

/* Table containing the test settings */
CU_TestInfo ST_SACHMgrTests[] =
{
   { "InitialisationTest",                   SACH_InitialisationTest },
   { "ScheduleMessageTest",                  SACH_ScheduleMessageTest },
   { "UpdateSACHManagementTest",             SACH_UpdateSACHManagementTest },
   { "DispatchMessageTest",                  SACH_DispatchMessageTest },
   { "GetSASSlotTypeTest",                   SACH_GetSASSlotTypeTest },
   { "SACH_RedirectMessagesTest",            SACH_RedirectMessagesTest },

   CU_TEST_INFO_NULL,
};


/* Private Variables
*************************************************************************************/



/*************************************************************************************/
/**
* SACH_InitialisationTest
* Test function MC_SACH_Initialise
*
* no params
*
* @return - none

*/
void SACH_InitialisationTest(void)
{
   /* set unitNumberWithinZone and unitAddress to 0xff*/
   unitNumberWithinZone = 0xff;
   unitAddress = 0xff;
   /* overwrite all of the sasQueue array with 0xff */
   memset(sasQueue, 0xff, sizeof(sasQueue));

   //call MC_SACH_Initialise
   MC_SACH_Initialise(1, 3);

   // check that the passed parameters were rcorded
   CU_ASSERT_EQUAL(unitNumberWithinZone, 3);
   CU_ASSERT_EQUAL(unitAddress, 1);

   // check that the message queues have been reset.
   for (uint32_t queue = 0; queue < SACH_NUMBER_OF_SAS_QUEUES; queue++)
   {
      CU_ASSERT_EQUAL(sasQueue[queue].MessageCount, 0);

      for (uint32_t index = 0; index < SACH_MAX_MESSAGES; index++)
      {
         CU_ASSERT_EQUAL(sasQueue[queue].channelQueue[index].Destination, 0);
         CU_ASSERT_EQUAL(sasQueue[queue].channelQueue[index].MessageType, APP_MSG_TYPE_UNKNOWN_E);
         uint8_t* pMsg = (uint8_t*)&sasQueue[queue].channelQueue[index].Message;
         for (uint32_t msg_index = 0; msg_index < sizeof(CO_Message_t); msg_index++)
         {
            CU_ASSERT_EQUAL(pMsg[msg_index], 0);
         }
      }
   }
}

/*************************************************************************************/
/**
* void SACH_ScheduleMessageTest
* Test function MC_SACH_ScheduleMessage
*
* no params
*
* @return - none

*/
void SACH_ScheduleMessageTest(void)
{
   CO_Message_t testMessage;

   //Initialise the queues
   MC_SACH_Initialise(1, 3);

   /*check for NULL message*/
   bool result = MC_SACH_ScheduleMessage(0, SACH_CHILD_E, APP_MSG_TYPE_ROUTE_ADD_E, NULL);
   CU_ASSERT_FALSE(result);
   /* check for out of range SACH channel */
   result = MC_SACH_ScheduleMessage(0, SACH_MAX_QUEUE_ID_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_FALSE(result);

   //Test addition to Mesh Q
   testMessage.Type = FRAME_TYPE_DATA_E;
   testMessage.Payload.PhyDataReq.Size = RACH_MSG_DATA_SIZE;
   memcpy(&testMessage.Payload.PhyDataReq.Data, MeshMessageData1, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(1, SACH_MESH_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 1);
   memcpy(&testMessage.Payload.PhyDataReq.Data, MeshMessageData2, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(2, SACH_MESH_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 2);
   memcpy(&testMessage.Payload.PhyDataReq.Data, MeshMessageData3, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(3, SACH_MESH_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 3);
   // adding one more should fail
   result = MC_SACH_ScheduleMessage(4, SACH_MESH_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_FALSE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 3);

   // Check that the other queues were not affected
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 0);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 0);

   //Test addition to Zone Q
   testMessage.Type = FRAME_TYPE_DATA_E;
   testMessage.Payload.PhyDataReq.Size = RACH_MSG_DATA_SIZE;
   memcpy(&testMessage.Payload.PhyDataReq.Data, ZoneMessageData1, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(4, SACH_ZONE_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 1);
   memcpy(&testMessage.Payload.PhyDataReq.Data, ZoneMessageData2, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(5, SACH_ZONE_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 2);
   memcpy(&testMessage.Payload.PhyDataReq.Data, ZoneMessageData3, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(6, SACH_ZONE_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 3);
   // adding one more should fail
   result = MC_SACH_ScheduleMessage(7, SACH_ZONE_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_FALSE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 3);

   // Check that the other queues were not affected
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 3);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 0);

   //Test addition to Child Q
   testMessage.Type = FRAME_TYPE_DATA_E;
   testMessage.Payload.PhyDataReq.Size = RACH_MSG_DATA_SIZE;
   memcpy(&testMessage.Payload.PhyDataReq.Data, ChildMessageData1, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(7, SACH_CHILD_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 1);
   memcpy(&testMessage.Payload.PhyDataReq.Data, ChildMessageData2, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(8, SACH_CHILD_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 2);
   memcpy(&testMessage.Payload.PhyDataReq.Data, ChildMessageData3, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(9, SACH_CHILD_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 3);
   // adding one more should fail
   result = MC_SACH_ScheduleMessage(10, SACH_CHILD_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_FALSE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 3);

   // Check that the other queues were not affected
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 3);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 3);

}

/*************************************************************************************/
/**
* SACH_UpdateSACHManagementTest
* Test function MC_SACH_UpdateSACHManagement
*
* no params
*
* @return - none

*/
void SACH_UpdateSACHManagementTest(void)
{
   CO_Message_t testMessage;
   bool result;
   uint8_t SlotIndex = 0;
   uint8_t ShortFrameIndex = 0;
   uint8_t LongFrameIndex = 0;

   //Initialise the queues
   MC_SACH_Initialise(1, 1);

   //Initialise the stae machine.
   //The current state determines which RACH queue is used.
   SMInitialise(false, 1, SYSTEM_ID);

   //Initialise the Ack Manager
   //This is where the SACH Manager will forward messages to.
   MC_ACK_Initialise();


   // check that nothing is sent for a valid slot with no messages waiting
   SlotIndex = UNIT_0_MESH_SLOT % SLOTS_PER_SHORT_FRAME;
   ShortFrameIndex = UNIT_0_MESH_SLOT / SLOTS_PER_SHORT_FRAME;
   LongFrameIndex = UNIT_0_MESH_SLOT / SLOTS_PER_LONG_FRAME;

   result = MC_SACH_UpdateSACHManagement(SlotIndex, ShortFrameIndex, LongFrameIndex);
   CU_ASSERT_FALSE(result);

   // Schedule a MESH SACH message
   testMessage.Type = FRAME_TYPE_DATA_E;
   testMessage.Payload.PhyDataReq.Size = RACH_MSG_DATA_SIZE;
   memcpy(&testMessage.Payload.PhyDataReq.Data, MeshMessageData1, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(1, SACH_MESH_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 1);
   // Schedule a ZONE SACH message
   memcpy(&testMessage.Payload.PhyDataReq.Data, ZoneMessageData1, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(4, SACH_ZONE_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 1);
   // Schedule a CHILD SACH message
   memcpy(&testMessage.Payload.PhyDataReq.Data, ChildMessageData1, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(7, SACH_CHILD_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 1);


   //update the SACH Management with an incrementing super slot number
   //It should return true on slot 85
   uint16_t super_slot_number = 0;
   while (super_slot_number < 85)
   {
      SlotIndex = super_slot_number % SLOTS_PER_SHORT_FRAME;
      ShortFrameIndex = super_slot_number / SLOTS_PER_SHORT_FRAME;
      LongFrameIndex = super_slot_number / SLOTS_PER_LONG_FRAME;
      result = MC_SACH_UpdateSACHManagement(SlotIndex, ShortFrameIndex, LongFrameIndex);
      CU_ASSERT_FALSE(result);
      super_slot_number++;
   }

   SlotIndex = super_slot_number % SLOTS_PER_SHORT_FRAME;
   ShortFrameIndex = super_slot_number / SLOTS_PER_SHORT_FRAME;
   LongFrameIndex = super_slot_number / SLOTS_PER_LONG_FRAME;

   //A message should be dispatched this time
   result = MC_SACH_UpdateSACHManagement(SlotIndex, ShortFrameIndex, LongFrameIndex);
   CU_ASSERT_TRUE(result);

   //check the sach queues
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 0);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 1);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 1);

   // check that the Ack Manager queued a message on P-RACH
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 1);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 0);

   // continue advancing the super slot number.
   // The ZONE SACH message should be issued in slot 105
   super_slot_number++;
   while (super_slot_number < 105)
   {
      SlotIndex = super_slot_number % SLOTS_PER_SHORT_FRAME;
      ShortFrameIndex = super_slot_number / SLOTS_PER_SHORT_FRAME;
      LongFrameIndex = super_slot_number / SLOTS_PER_LONG_FRAME;

      result = MC_SACH_UpdateSACHManagement(SlotIndex, ShortFrameIndex, LongFrameIndex);
      CU_ASSERT_FALSE(result);
      super_slot_number++;
   }

   SlotIndex = super_slot_number % SLOTS_PER_SHORT_FRAME;
   ShortFrameIndex = super_slot_number / SLOTS_PER_SHORT_FRAME;
   LongFrameIndex = super_slot_number / SLOTS_PER_LONG_FRAME;

   //A message should be dispatched this time
   result = MC_SACH_UpdateSACHManagement(SlotIndex, ShortFrameIndex, LongFrameIndex);
   CU_ASSERT_TRUE(result);

   //check the sach queues
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 0);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 0);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 1);

   // check that the Ack Manager queued a message on P-RACH
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 2);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 0);

   // continue advancing the super slot number.
   // The CHILD SACH message should be issued in slot 125
   super_slot_number++;
   while (super_slot_number < 125)
   {
      SlotIndex = super_slot_number % SLOTS_PER_SHORT_FRAME;
      ShortFrameIndex = super_slot_number / SLOTS_PER_SHORT_FRAME;
      LongFrameIndex = super_slot_number / SLOTS_PER_LONG_FRAME;

      result = MC_SACH_UpdateSACHManagement(SlotIndex, ShortFrameIndex, LongFrameIndex);
      CU_ASSERT_FALSE(result);
      super_slot_number++;
   }

   SlotIndex = super_slot_number % SLOTS_PER_SHORT_FRAME;
   ShortFrameIndex = super_slot_number / SLOTS_PER_SHORT_FRAME;
   LongFrameIndex = super_slot_number / SLOTS_PER_LONG_FRAME;

   //A message should be dispatched this time
   result = MC_SACH_UpdateSACHManagement(SlotIndex, ShortFrameIndex, LongFrameIndex);
   CU_ASSERT_TRUE(result);

   //check the sach queues
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 0);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 0);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 0);

   // check that the Ack Manager queued 3 messages on P-RACH
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 3);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 0);

   //Advance the state machine to ACTIVE and repeat the tests
   //We should now be sending on S-RACH

   gSM_CurrentMeshState = STATE_ACTIVE_E;

   //Clear the Ack Manager Queues.
   MC_ACK_Initialise();
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 0);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 0);

   // Schedule a MESH SACH message
   testMessage.Type = FRAME_TYPE_DATA_E;
   testMessage.Payload.PhyDataReq.Size = RACH_MSG_DATA_SIZE;
   memcpy(&testMessage.Payload.PhyDataReq.Data, MeshMessageData1, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(1, SACH_MESH_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 1);
   // Schedule a ZONE SACH message
   memcpy(&testMessage.Payload.PhyDataReq.Data, ZoneMessageData1, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(4, SACH_ZONE_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 1);
   // Schedule a CHILD SACH message
   memcpy(&testMessage.Payload.PhyDataReq.Data, ChildMessageData1, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(7, SACH_CHILD_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 1);


   //update the SACH Management with an incrementing super slot number
   //It should return true on slot 325
   super_slot_number++;
   while (super_slot_number < 325)
   {
      SlotIndex = super_slot_number % SLOTS_PER_SHORT_FRAME;
      ShortFrameIndex = super_slot_number / SLOTS_PER_SHORT_FRAME;
      LongFrameIndex = super_slot_number / SLOTS_PER_LONG_FRAME;

      result = MC_SACH_UpdateSACHManagement(SlotIndex, ShortFrameIndex, LongFrameIndex);
      CU_ASSERT_FALSE(result);
      super_slot_number++;
   }

   SlotIndex = super_slot_number % SLOTS_PER_SHORT_FRAME;
   ShortFrameIndex = super_slot_number / SLOTS_PER_SHORT_FRAME;
   LongFrameIndex = super_slot_number / SLOTS_PER_LONG_FRAME;

   //A message should be dispatched this time
   result = MC_SACH_UpdateSACHManagement(SlotIndex, ShortFrameIndex, LongFrameIndex);
   CU_ASSERT_TRUE(result);

   //check the sach queues
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 0);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 1);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 1);

   // check that the Ack Manager queued a message on S-RACH
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 0);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 1);

   // continue advancing the super slot number.
   // The ZONE SACH message should be issued in slot 405
   super_slot_number++;
   while (super_slot_number < 405)
   {
      SlotIndex = super_slot_number % SLOTS_PER_SHORT_FRAME;
      ShortFrameIndex = super_slot_number / SLOTS_PER_SHORT_FRAME;
      LongFrameIndex = super_slot_number / SLOTS_PER_LONG_FRAME;

      result = MC_SACH_UpdateSACHManagement(SlotIndex, ShortFrameIndex, LongFrameIndex);
      CU_ASSERT_FALSE(result);
      super_slot_number++;
   }
   SlotIndex = super_slot_number % SLOTS_PER_SHORT_FRAME;
   ShortFrameIndex = super_slot_number / SLOTS_PER_SHORT_FRAME;
   LongFrameIndex = super_slot_number / SLOTS_PER_LONG_FRAME;

   //A message should be dispatched this time
   result = MC_SACH_UpdateSACHManagement(SlotIndex, ShortFrameIndex, LongFrameIndex);
   CU_ASSERT_TRUE(result);

   //check the sach queues
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 0);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 0);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 1);

   // check that the Ack Manager queued a message on S-RACH
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 0);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 2);

   // continue advancing the super slot number.
   // The CHILD SACH message should be issued in slot 485
   super_slot_number++;
   while (super_slot_number < 485)
   {
      SlotIndex = super_slot_number % SLOTS_PER_SHORT_FRAME;
      ShortFrameIndex = super_slot_number / SLOTS_PER_SHORT_FRAME;
      LongFrameIndex = super_slot_number / SLOTS_PER_LONG_FRAME;

      result = MC_SACH_UpdateSACHManagement(SlotIndex, ShortFrameIndex, LongFrameIndex);
      CU_ASSERT_FALSE(result);
      super_slot_number++;
   }

   SlotIndex = super_slot_number % SLOTS_PER_SHORT_FRAME;
   ShortFrameIndex = super_slot_number / SLOTS_PER_SHORT_FRAME;
   LongFrameIndex = super_slot_number / SLOTS_PER_LONG_FRAME;

   //A message should be dispatched this time
   result = MC_SACH_UpdateSACHManagement(SlotIndex, ShortFrameIndex, LongFrameIndex);
   CU_ASSERT_TRUE(result);

   //check the sach queues
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 0);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 0);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 0);

   // check that the Ack Manager queued a message on S-RACH
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 0);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 3);

   //Check that it reports a failure when the queue is full
   memcpy(&testMessage.Payload.PhyDataReq.Data, MeshMessageData1, RACH_MSG_DATA_SIZE);
   while (sasQueue[SACH_MESH_E].MessageCount < SACH_MAX_MESSAGES)
   {
      result = MC_SACH_ScheduleMessage(1, SACH_MESH_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
      CU_ASSERT_TRUE(result);
   }
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, SACH_MAX_MESSAGES);

   //Adding further messages should fail
   result = MC_SACH_ScheduleMessage(1, SACH_MESH_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_FALSE(result);
}


/*************************************************************************************/
/**
* SACH_DispatchMessageTest
* Test function MC_SACH_DispatchMessage
*
* no params
*
* @return - none

*/
void SACH_DispatchMessageTest(void)
{
   CO_Message_t testMessage;
   bool result = false;

   //Initialise the queues
   MC_SACH_Initialise(1, 1);

   //Initialise the stae machine.
   //The current state determines which RACH queue is used.
   SMInitialise(false, 1, SYSTEM_ID);

   //Initialise the Ack Manager
   //This is where the SACH Manager will forward messages to.
   MC_ACK_Initialise();

   // check that out-of-range queue ID is rejected
   result = MC_SACH_DispatchMessage(SACH_NUMBER_OF_SAS_QUEUES, ACK_RACHP_E);
   CU_ASSERT_FALSE(result);

   // check that it returns false if the queue is empty
   result = MC_SACH_DispatchMessage(SACH_MESH_E, ACK_RACHP_E);
   CU_ASSERT_FALSE(result);


   // Fill the message queues
   // MESH SACH messages
   testMessage.Type = FRAME_TYPE_DATA_E;
   testMessage.Payload.PhyDataReq.Size = RACH_MSG_DATA_SIZE;
   memcpy(&testMessage.Payload.PhyDataReq.Data, MeshMessageData1, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(1, SACH_MESH_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 1);
   memcpy(&testMessage.Payload.PhyDataReq.Data, MeshMessageData2, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(1, SACH_MESH_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 2);
   memcpy(&testMessage.Payload.PhyDataReq.Data, MeshMessageData3, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(1, SACH_MESH_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 3);
   // ZONE SACH messages
   memcpy(&testMessage.Payload.PhyDataReq.Data, ZoneMessageData1, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(4, SACH_ZONE_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 1);
   memcpy(&testMessage.Payload.PhyDataReq.Data, ZoneMessageData2, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(4, SACH_ZONE_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 2);
   memcpy(&testMessage.Payload.PhyDataReq.Data, ZoneMessageData3, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(4, SACH_ZONE_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount,3);
   // CHILD SACH messages
   memcpy(&testMessage.Payload.PhyDataReq.Data, ChildMessageData1, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(7, SACH_CHILD_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 1);
   memcpy(&testMessage.Payload.PhyDataReq.Data, ChildMessageData2, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(7, SACH_CHILD_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 2);
   memcpy(&testMessage.Payload.PhyDataReq.Data, ChildMessageData3, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(7, SACH_CHILD_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 3);

   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 3);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 3);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 3);

   /* push 7 messages into the ACK Mgr queues leaving only 3 spaces*/
   for (uint8_t index = 0; index < 7; index++)
   {
      testMessage.Payload.PhyDataReq.Data[2] = index;
      MC_ACK_AddMessage(index, ACK_RACHP_E, APP_MSG_TYPE_ROUTE_ADD_E, false, 0, &testMessage, false);
      MC_ACK_AddMessage(index, ACK_RACHS_E, APP_MSG_TYPE_ROUTE_ADD_E, false, 0, &testMessage, false);
   }
   // check that the Ack Manager queued 7 messages on P-RACH and S-RACH
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 7);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 7);

   //Send one of each message type to P-RACH
   result = MC_SACH_DispatchMessage(SACH_MESH_E, ACK_RACHP_E);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 8);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 7);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 2);
   result = MC_SACH_DispatchMessage(SACH_ZONE_E, ACK_RACHP_E);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 9);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 7);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 2);
   result = MC_SACH_DispatchMessage(SACH_CHILD_E, ACK_RACHP_E);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 10);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 7);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 2);

   // send one more to P-RACH.  The Ack Manager should reject it because it's full.
   // Verify that MC_SACH_DispatchMessage returns false and we still have 2 messages queued.
   result = MC_SACH_DispatchMessage(SACH_MESH_E, ACK_RACHP_E);
   CU_ASSERT_FALSE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 2);

   //Send one of each message type to S-RACH
   result = MC_SACH_DispatchMessage(SACH_MESH_E, ACK_RACHS_E);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 10);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 8);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 1);
   result = MC_SACH_DispatchMessage(SACH_ZONE_E, ACK_RACHS_E);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 10);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 9);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 1);
   result = MC_SACH_DispatchMessage(SACH_CHILD_E, ACK_RACHS_E);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 10);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 10);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 1);


   // send one more to S-RACH.  The Ack Manager should reject it because it's full.
   // Verify that MC_SACH_DispatchMessage returns false and we still have 1 message queued.
   result = MC_SACH_DispatchMessage(SACH_MESH_E, ACK_RACHS_E);
   CU_ASSERT_FALSE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 1);
}

/*************************************************************************************/
/**
* SACH_GetSASSlotTypeTest
* Tests the function MC_SACH_GetSASSlotType.
*
* no params
*
* @return - void

*/
void SACH_GetSASSlotTypeTest(void)
{
   uint16_t UnitAddress = 0;
   uint8_t SlotIndex = 0;
   uint8_t ShortFrameIndex = 0;
   uint8_t LongFrameIndex = 0;
   MC_SACH_SAS_SlotType_t SASSlotType = NOT_SAS_SLOT_E;

   gSM_CurrentMeshState = STATE_CONFIG_FORM_E;
   for (uint32_t SlotInSuperFrame = 0; SlotInSuperFrame < SLOTS_PER_SUPER_FRAME; SlotInSuperFrame++)
   {
      SlotIndex = SlotInSuperFrame % SLOTS_PER_SHORT_FRAME;
      ShortFrameIndex = (SlotInSuperFrame / SLOTS_PER_SHORT_FRAME) % SHORT_FRAMES_PER_LONG_FRAME;
      LongFrameIndex = (SlotInSuperFrame / (SLOTS_PER_SHORT_FRAME * SHORT_FRAMES_PER_LONG_FRAME)) % LONG_FRAMES_PER_SUPER_FRAME;

      // Call the function under test
      SASSlotType = MC_SACH_GetSASSlotType(SlotIndex, ShortFrameIndex, LongFrameIndex, &UnitAddress);

      if (FIRST_PRACH_ACK_SLOT_IDX == SlotIndex)// First PRACH slot, so could be Prinary SAS.
      {
         // Test that correct values are assigned in the first PRACH slot.
         uint32_t OneCountEveryFourShortFrames = (SlotInSuperFrame / SLOTS_PER_SHORT_FRAME) / 4;
         if (0 == (ShortFrameIndex % 4))
         {
            CU_ASSERT_EQUAL(SASSlotType, P_MESH_SAS_E);
            CU_ASSERT_EQUAL(UnitAddress, OneCountEveryFourShortFrames % MAX_SAS_PER_MESH);
         }
         else if (1 == (ShortFrameIndex % 4))
         {
            CU_ASSERT_EQUAL(SASSlotType, P_ZONE_SAS_E);
            CU_ASSERT_EQUAL(UnitAddress, OneCountEveryFourShortFrames % MAX_UNITS_PER_ZONE);
         }
         else if (2 == (ShortFrameIndex % 4))
         {
            CU_ASSERT_EQUAL(SASSlotType, P_CHILD_SAS_E);
            CU_ASSERT_EQUAL(UnitAddress, OneCountEveryFourShortFrames % MAX_CHILDREN_PER_PARENT);
         }
         else // I.e. (3 == (ShortFrameIndex % 4))
         {
            CU_ASSERT_EQUAL(SASSlotType, NOT_SAS_SLOT_E);
            CU_ASSERT_EQUAL(UnitAddress, 0);
         }
      }
      else
      {
         CU_ASSERT_EQUAL(SASSlotType, NOT_SAS_SLOT_E);
         CU_ASSERT_EQUAL(UnitAddress, 0);
      }
   }

   gSM_CurrentMeshState = STATE_ACTIVE_E;
   for (uint32_t SlotInSuperFrame = 0; SlotInSuperFrame < SLOTS_PER_SUPER_FRAME; SlotInSuperFrame++)
   {
      SlotIndex = SlotInSuperFrame % SLOTS_PER_SHORT_FRAME;
      ShortFrameIndex = (SlotInSuperFrame / SLOTS_PER_SHORT_FRAME) % SHORT_FRAMES_PER_LONG_FRAME;
      LongFrameIndex = (SlotInSuperFrame / (SLOTS_PER_SHORT_FRAME * SHORT_FRAMES_PER_LONG_FRAME)) % LONG_FRAMES_PER_SUPER_FRAME;

      // Call the function under test
      SASSlotType = MC_SACH_GetSASSlotType(SlotIndex, ShortFrameIndex, LongFrameIndex, &UnitAddress);

      if (FIRST_PRACH_ACK_SLOT_IDX == SlotIndex)// First PRACH slot, so could be Prinary SAS.
      {
         // Test that correct values are assigned in the first PRACH slot.
         uint32_t OneCountEvery16ShortFrames = (SlotInSuperFrame / SLOTS_PER_SHORT_FRAME) / 16;
         if (0 == (ShortFrameIndex % 16))
         {
            CU_ASSERT_EQUAL(SASSlotType, S_MESH_SAS_E);
            CU_ASSERT_EQUAL(UnitAddress, OneCountEvery16ShortFrames % MAX_SAS_PER_MESH);
         }
         else if (4 == (ShortFrameIndex % 16))
         {
            CU_ASSERT_EQUAL(SASSlotType, S_ZONE_SAS_E);
            CU_ASSERT_EQUAL(UnitAddress, OneCountEvery16ShortFrames % MAX_UNITS_PER_ZONE);
         }
         else if (8 == (ShortFrameIndex % 16))
         {
            CU_ASSERT_EQUAL(SASSlotType, S_CHILD_SAS_E);
            CU_ASSERT_EQUAL(UnitAddress, OneCountEvery16ShortFrames % MAX_CHILDREN_PER_PARENT);
         }
         else // I.e. (3 == (ShortFrameIndex % 16))
         {
            CU_ASSERT_EQUAL(SASSlotType, NOT_SAS_SLOT_E);
            CU_ASSERT_EQUAL(UnitAddress, 0);
         }
      }
      else
      {
         CU_ASSERT_EQUAL(SASSlotType, NOT_SAS_SLOT_E);
         CU_ASSERT_EQUAL(UnitAddress, 0);
      }
   }
}

/*************************************************************************************/
/**
* SACH_RedirectMessagesTest
* Tests the function MC_SACH_RedirectMessagesForNode.
*
* no params
*
* @return - void

*/
void SACH_RedirectMessagesTest(void)
{
   CO_Message_t testMessage;
   bool result = false;
   uint32_t neighbourID_1 = 1;
   uint32_t neighbourID_2 = 2;

   /* initialise the SACH management */
   MC_SACH_Initialise(3, 3);

   /* initialise the session manager */
   MC_SMGR_Initialise(false, 3);
   uint16_t neighbour1DCHSlot = MC_SMGR_CalculateDCHSlot(neighbourID_1);
   uint16_t neighbour2DCHSlot = MC_SMGR_CalculateDCHSlot(neighbourID_2);
   MC_SMGR_SetParentNodes(neighbour1DCHSlot, neighbour2DCHSlot);
   MC_SMGR_RouteAddResponse(neighbourID_1, true);
   MC_SMGR_RouteAddResponse(neighbourID_2, true);

   // Fill the message queues
   // MESH SACH messages
   testMessage.Type = FRAME_TYPE_DATA_E;
   testMessage.Payload.PhyDataReq.Size = RACH_MSG_DATA_SIZE;
   memcpy(&testMessage.Payload.PhyDataReq.Data, MeshMessageData1, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(neighbourID_1, SACH_MESH_E, APP_MSG_TYPE_STATUS_INDICATION_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 1);
   memcpy(&testMessage.Payload.PhyDataReq.Data, MeshMessageData2, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(neighbourID_1, SACH_MESH_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 2);
   memcpy(&testMessage.Payload.PhyDataReq.Data, MeshMessageData3, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(neighbourID_2, SACH_MESH_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 3);
   // ZONE SACH messages
   memcpy(&testMessage.Payload.PhyDataReq.Data, ZoneMessageData1, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(neighbourID_1, SACH_ZONE_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 1);
   memcpy(&testMessage.Payload.PhyDataReq.Data, ZoneMessageData2, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(neighbourID_1, SACH_ZONE_E, APP_MSG_TYPE_STATUS_INDICATION_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 2);
   memcpy(&testMessage.Payload.PhyDataReq.Data, ZoneMessageData3, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(neighbourID_2, SACH_ZONE_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 3);
   // CHILD SACH messages
   memcpy(&testMessage.Payload.PhyDataReq.Data, ChildMessageData1, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(neighbourID_2, SACH_CHILD_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 1);
   memcpy(&testMessage.Payload.PhyDataReq.Data, ChildMessageData2, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(neighbourID_1, SACH_CHILD_E, APP_MSG_TYPE_ROUTE_ADD_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 2);
   memcpy(&testMessage.Payload.PhyDataReq.Data, ChildMessageData3, RACH_MSG_DATA_SIZE);
   result = MC_SACH_ScheduleMessage(neighbourID_2, SACH_CHILD_E, APP_MSG_TYPE_STATUS_INDICATION_E, &testMessage);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 3);

   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 3);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 3);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 3);


   /* Check the set up of the SACH_MESH_E queue */
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].channelQueue[0].Destination, neighbourID_1);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].channelQueue[1].Destination, neighbourID_1);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].channelQueue[2].Destination, neighbourID_2);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].channelQueue[0].MessageType, APP_MSG_TYPE_STATUS_INDICATION_E);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].channelQueue[1].MessageType, APP_MSG_TYPE_ROUTE_ADD_E);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].channelQueue[2].MessageType, APP_MSG_TYPE_ROUTE_ADD_E);

   /* Check the set up of the SACH_ZONE_E queue */
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].channelQueue[0].Destination, neighbourID_1);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].channelQueue[1].Destination, neighbourID_1);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].channelQueue[2].Destination, neighbourID_2);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].channelQueue[0].MessageType, APP_MSG_TYPE_ROUTE_ADD_E);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].channelQueue[1].MessageType, APP_MSG_TYPE_STATUS_INDICATION_E);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].channelQueue[2].MessageType, APP_MSG_TYPE_ROUTE_ADD_E);

   /* Check the set up of the SACH_CHILD_E queue */
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].channelQueue[0].Destination, neighbourID_2);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].channelQueue[1].Destination, neighbourID_1);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].channelQueue[2].Destination, neighbourID_2);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].channelQueue[0].MessageType, APP_MSG_TYPE_ROUTE_ADD_E);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].channelQueue[1].MessageType, APP_MSG_TYPE_ROUTE_ADD_E);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].channelQueue[2].MessageType, APP_MSG_TYPE_STATUS_INDICATION_E);

   /* redirect the neighbourID_1 messages */
   MC_SACH_RedirectMessagesForNode(neighbourID_1);

   /* check the number of messages in the queues */
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].MessageCount, 2);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 2);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].MessageCount, 2);

   /* Check the new state of the SACH_MESH_E queue */
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].channelQueue[0].Destination, neighbourID_2);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].channelQueue[1].Destination, neighbourID_2);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].channelQueue[0].MessageType, APP_MSG_TYPE_STATUS_INDICATION_E);
   CU_ASSERT_EQUAL(sasQueue[SACH_MESH_E].channelQueue[1].MessageType, APP_MSG_TYPE_ROUTE_ADD_E);
   
   /* Check the new state of the SACH_ZONE_E queue */
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].channelQueue[0].Destination, neighbourID_2);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].channelQueue[1].Destination, neighbourID_2);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].channelQueue[0].MessageType, APP_MSG_TYPE_STATUS_INDICATION_E);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].channelQueue[1].MessageType, APP_MSG_TYPE_ROUTE_ADD_E);

   /* Check the new state of the SACH_CHILD_E queue */
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].channelQueue[0].Destination, neighbourID_2);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].channelQueue[1].Destination, neighbourID_2);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].channelQueue[0].MessageType, APP_MSG_TYPE_ROUTE_ADD_E);
   CU_ASSERT_EQUAL(sasQueue[SACH_CHILD_E].channelQueue[1].MessageType, APP_MSG_TYPE_STATUS_INDICATION_E);

}
