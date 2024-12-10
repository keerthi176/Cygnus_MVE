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
*  File         : STMBOTest.c
*
*  Description  : Implementation for the RACH ACK back-off functions 
*                 in source file MC_MsgBackoff.c
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
#include "STAckManagementTest.h"
#include "MC_AckManagement.h"
#include "MC_SessionManagement.h"


/* Private Functions Prototypes
*************************************************************************************/
static void ACK_InitialisationTest(void);
static void ACK_AddMessageRACH1Test(void);
static void ACK_AddMessageRACH2Test(void);
static void ACK_AcknowledgeMessageTest(void);
static void ACK_WaitingForACKTest(void);
static void ACK_UpdateSlotTest(void);
static void ACK_GetRandomDelayTest(void);
static void ACK_GetMessageTest(void);
static void ACK_RerouteMessageTest(void);
static void ACK_SendOnNextSlotTest(void);
static void ACK_GetMacDestinationOfQueuedMessageTest(void);
static void ACK_RedirectMessagesForNodeRACHPTest(void);
static void ACK_RedirectMessagesForNodeRACHSTest(void);
static void ACK_RedirectMessagesForNodeNonHopTest(void);

/* definitions
*************************************************************************************/
#define ALARM_SIGNAL_BINARY_SIZE 20
#define ROUTEADD_BINARY_SIZE 20


/* Global Variables
*************************************************************************************/
extern UnackedMsgQueue_t unackQueue[ACK_NUMBER_OF_ACKNOWLEDGEMENT_QUEUES];
extern const uint32_t BackoffExponentFilter[ACK_MAX_BACKOFF_EXPONENT];

uint8_t alarmSignalData1[ALARM_SIGNAL_BINARY_SIZE] = { 0x10,0x01,0x20,0x20,0x43,0x03,0x40,0x40,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
uint8_t alarmSignalData2[ALARM_SIGNAL_BINARY_SIZE] = { 0x10,0x02,0x00,0x20,0x43,0x03,0x40,0x40,0x52,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
uint8_t alarmSignalData3[ALARM_SIGNAL_BINARY_SIZE] = { 0x10,0x02,0x00,0x30,0x43,0x03,0x40,0x40,0x53,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
uint8_t fireSignalData1[ALARM_SIGNAL_BINARY_SIZE] = { 0x10,0x01,0x20,0x20,0x43,0x03,0x40,0x41,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
uint8_t fireSignalData2[ALARM_SIGNAL_BINARY_SIZE] = { 0x10,0x01,0x00,0x20,0x43,0x03,0x40,0x41,0x55,0xa0,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
uint8_t fireSignalData3[ALARM_SIGNAL_BINARY_SIZE] = { 0x10,0x01,0x00,0x30,0x43,0x03,0x40,0x41,0x53,0x30,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };

extern bool MC_ACK_RerouteMessage(const AcknowledgedQueueID_t channelID, const uint8_t queue_index);
extern uint16_t MC_ACK_GetMacDestinationOfQueuedMessage(const AcknowledgedQueueID_t channelID, const uint8_t queue_index);

/* Table containing the test settings */
CU_TestInfo ST_AckMgrTests[] =
{
   { "InitialisationTest",                  ACK_InitialisationTest },
   { "AddMessageRACH1Test",                 ACK_AddMessageRACH1Test },
   { "AddMessageRACH2Test",                 ACK_AddMessageRACH2Test },
   { "AcknowledgeMessageTest",              ACK_AcknowledgeMessageTest },
   { "WaitingForACKTest",                   ACK_WaitingForACKTest },
   { "UpdateSlotTest",                      ACK_UpdateSlotTest },
   { "GetRandomDelayTest",                  ACK_GetRandomDelayTest },
   { "GetMessageTest",                      ACK_GetMessageTest },
   { "RerouteMessageTest",                  ACK_RerouteMessageTest },
   { "SendOnNextSlotTest",                  ACK_SendOnNextSlotTest },
   { "GetMacDestinationOfQueuedMessageTest",ACK_GetMacDestinationOfQueuedMessageTest },
   { "RedirectMessagesForNodeRACHPTest",    ACK_RedirectMessagesForNodeRACHPTest },
   { "RedirectMessagesForNodeRACHSTest",    ACK_RedirectMessagesForNodeRACHSTest },
   { "RedirectMessagesForNodeNonHopTest",   ACK_RedirectMessagesForNodeNonHopTest },

   CU_TEST_INFO_NULL,
};


/* Private Variables
*************************************************************************************/



/*************************************************************************************/
/**
* ACK_InitialisationTest
* Test function MC_ACK_Initialise()
*
* no params
*
* @return - none

*/
void ACK_InitialisationTest(void)
{

      /* set the channel dependent attributes */
      for (int32_t channel = 0; channel < ACK_NUMBER_OF_ACK_CHANNELS; channel++)
      {
         unackQueue[channel].BackoffExp = 1;
         unackQueue[channel].BackoffCounter = 2;
         unackQueue[channel].MessageCount = 3;
         /* set the unack'd message list */
         for (int32_t index = 0; index < ACK_MAX_UNACKED_MESSAGES; index++)
         {
            unackQueue[channel].channelQueue[index].Message.Type = CO_MESSAGE_GENERATE_HEARTBEAT_E;
            unackQueue[channel].channelQueue[index].Message.Payload.PhyDataReq.Size = 1;
            unackQueue[channel].channelQueue[index].Message.Payload.PhyDataReq.Data[0] = 0xff;
         }
      }


   MC_ACK_Initialise();

   /* check the channel dependent attributes */
   for (int32_t channel = 0; channel < ACK_NUMBER_OF_ACKNOWLEDGEMENT_QUEUES; channel++)
   {
      CU_ASSERT_EQUAL(unackQueue[channel].BackoffExp, 0);
      CU_ASSERT_EQUAL(unackQueue[channel].BackoffCounter, 0);
      CU_ASSERT_EQUAL(unackQueue[channel].MessageCount, 0);
      CU_ASSERT_FALSE(unackQueue[channel].ReadyToSend);

      /* check the unack'd message list */
      for (int32_t index = 0; index < ACK_MAX_UNACKED_MESSAGES; index++)
      {
         CU_ASSERT_EQUAL(unackQueue[channel].channelQueue[index].MessageHandle, 0);
         CU_ASSERT_FALSE(unackQueue[channel].channelQueue[index].RouteDiversity);
         CU_ASSERT_EQUAL(unackQueue[channel].channelQueue[index].Message.Type, CO_MESSAGE_PHY_DATA_REQ_E);
         CU_ASSERT_EQUAL(unackQueue[channel].channelQueue[index].Message.Payload.PhyDataReq.Size, 0);
         CU_ASSERT_EQUAL(unackQueue[channel].channelQueue[index].Message.Payload.PhyDataReq.Data[0], 0);
      }
   }
}

/*************************************************************************************/
/**
* ACK_AddMessageRACH1Test
* Partial test for function MC_ACK_AddMessage
*
* no params
*
* @return - none

*/

void ACK_AddMessageRACH1Test(void)
{
   uint32_t neighbourID = 1234;
   uint32_t handle = 1;

   bool result = false;

   /* clear the Back-off message queues */
   MC_ACK_Initialise();

   /*Create a mesage and add it to the store*/
   CO_Message_t fireSignalMsg;
   fireSignalMsg.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg.Payload.PhyDataReq.Data, fireSignalData1, ALARM_SIGNAL_BINARY_SIZE);

   /*push the fire signal into the back-off store*/
   result = MC_ACK_AddMessage(neighbourID, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, false, handle, &fireSignalMsg, true);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 1);

   /* check that the ACK Management doesn't queue duplicate messages */
   handle = 2;
   result = MC_ACK_AddMessage(neighbourID, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, false, handle, &fireSignalMsg, true);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 1);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].channelQueue[0].MessageHandle, handle);

   /* check the store contents */
   uint32_t expectedMsgIndex = 0;

   /* check the channel dependent attributes */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffExp, 0);
   CU_ASSERT_TRUE( 3 > unackQueue[ACK_RACHP_E].BackoffCounter );
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 1);

   /* check the unack'd message list */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].channelQueue[expectedMsgIndex].Message.Type, CO_MESSAGE_PHY_DATA_REQ_E);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].channelQueue[expectedMsgIndex].Message.Payload.PhyDataReq.Size, ALARM_SIGNAL_BINARY_SIZE);
   for (int32_t index = 0; index < ALARM_SIGNAL_BINARY_SIZE; index++)
   {
      CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].channelQueue[expectedMsgIndex].Message.Payload.PhyDataReq.Data[index], fireSignalMsg.Payload.PhyDataReq.Data[index]);
   }


   /*add another message and check it is in the right place in the queue (channelQueue[ACK_RACHP][1])*/
   memcpy(fireSignalMsg.Payload.PhyDataReq.Data, fireSignalData2, ALARM_SIGNAL_BINARY_SIZE);

   /*push the fire signal into the back-off store*/
   result = MC_ACK_AddMessage(neighbourID, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, false, handle, &fireSignalMsg, true);
   CU_ASSERT_TRUE(result);

   /* check the store contents */
   expectedMsgIndex = 1;

   /* check the channel dependent attributes */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffExp, 0);
   CU_ASSERT_TRUE(3 > unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 2);

   /* check the unack'd message list */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].channelQueue[expectedMsgIndex].Message.Type, CO_MESSAGE_PHY_DATA_REQ_E);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].channelQueue[expectedMsgIndex].Message.Payload.PhyDataReq.Size, ALARM_SIGNAL_BINARY_SIZE);
   for (int32_t index = 0; index < ALARM_SIGNAL_BINARY_SIZE; index++)
   {
      CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].channelQueue[expectedMsgIndex].Message.Payload.PhyDataReq.Data[index], fireSignalMsg.Payload.PhyDataReq.Data[index]);
   }


   /* Add messages until we reach ACK_MAX_UNACKED_MESSAGES.  A further attempt to add a message should fail*/
   expectedMsgIndex++;
   uint8_t newAddress = 2;
   while (expectedMsgIndex < ACK_MAX_UNACKED_MESSAGES)
   {
      fireSignalMsg.Payload.PhyDataReq.Data[2] = newAddress;
      result = MC_ACK_AddMessage(neighbourID, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, false, handle, &fireSignalMsg, true);
      CU_ASSERT_TRUE(result);
      expectedMsgIndex++;
      newAddress++;
   }
   /* This one should fail because the back-off message queue is full */
   result = MC_ACK_AddMessage(neighbourID, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, false, handle, &fireSignalMsg, true);
   CU_ASSERT_FALSE(result);

   /* The message count should be ACK_MAX_UNACKED_MESSAGES */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, ACK_MAX_UNACKED_MESSAGES);

   /* Ensure that nothing went into the RACH2 queue */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 0);
}

/*************************************************************************************/
/**
* ACK_AddMessageRACH2Test
* Partial test for function MC_ACK_AddMessage
*
* no params
*
* @return - none

*/

void ACK_AddMessageRACH2Test(void)
{
   uint32_t neighbourID1 = 1;
   uint32_t neighbourID2 = 2;
   uint32_t handle = 1;
   bool result = false;

   /* clear the Back-off message queues */
   MC_ACK_Initialise();

   /*Create a mesage and add it to the store*/
   CO_Message_t alarmSignalMsg;
   alarmSignalMsg.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   alarmSignalMsg.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(alarmSignalMsg.Payload.PhyDataReq.Data, alarmSignalData1, ALARM_SIGNAL_BINARY_SIZE);

   /*push the fire signal into the back-off store*/
   result = MC_ACK_AddMessage(neighbourID1, ACK_RACHS_E, APP_MSG_TYPE_ALARM_SIGNAL_E, false, handle, &alarmSignalMsg, true);
   CU_ASSERT_TRUE(result);

   /* check the store contents */
   uint32_t expectedMsgIndex = 0;

   /* check the channel dependent attributes */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].BackoffExp, 0);
   CU_ASSERT_TRUE(3 > unackQueue[ACK_RACHS_E].BackoffCounter);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 1);

   /* check the unack'd message list */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].channelQueue[expectedMsgIndex].Message.Type, CO_MESSAGE_PHY_DATA_REQ_E);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].channelQueue[expectedMsgIndex].Message.Payload.PhyDataReq.Size, ALARM_SIGNAL_BINARY_SIZE);
   for (int32_t index = 0; index < ALARM_SIGNAL_BINARY_SIZE; index++)
   {
      CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].channelQueue[expectedMsgIndex].Message.Payload.PhyDataReq.Data[index], alarmSignalMsg.Payload.PhyDataReq.Data[index]);
   }


   /*add another message and check it is in the right place in the queue (channelQueue[ACK_RACHS][1])*/
   memcpy(alarmSignalMsg.Payload.PhyDataReq.Data, alarmSignalData2, ALARM_SIGNAL_BINARY_SIZE);

   /*push the alarm signal into the back-off store*/
   result = MC_ACK_AddMessage(neighbourID2, ACK_RACHS_E, APP_MSG_TYPE_ALARM_SIGNAL_E, false, handle, &alarmSignalMsg, true);
   CU_ASSERT_TRUE(result);

   /* check the store contents */
   expectedMsgIndex = 1;

   /* check the channel dependent attributes */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].BackoffExp, 0);
   CU_ASSERT_TRUE(3 > unackQueue[ACK_RACHS_E].BackoffCounter);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 2);

   /* check the unack'd message list */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].channelQueue[expectedMsgIndex].Message.Type, CO_MESSAGE_PHY_DATA_REQ_E);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].channelQueue[expectedMsgIndex].Message.Payload.PhyDataReq.Size, ALARM_SIGNAL_BINARY_SIZE);
   for (int32_t index = 0; index < ALARM_SIGNAL_BINARY_SIZE; index++)
   {
      CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].channelQueue[expectedMsgIndex].Message.Payload.PhyDataReq.Data[index], alarmSignalMsg.Payload.PhyDataReq.Data[index]);
   }


   /* Add messages until we reach ACK_MAX_UNACKED_MESSAGES.  A further attempt to add a message should fail*/
   expectedMsgIndex++;
   uint8_t newAddress = 2;
   while (expectedMsgIndex < ACK_MAX_UNACKED_MESSAGES)
   {
      alarmSignalMsg.Payload.PhyDataReq.Data[2] = newAddress;
      result = MC_ACK_AddMessage(neighbourID1, ACK_RACHS_E, APP_MSG_TYPE_ALARM_SIGNAL_E, false, handle, &alarmSignalMsg, true);
      CU_ASSERT_TRUE(result);
      expectedMsgIndex++;
      newAddress++;
   }
   /* This one should fail because the back-off message queue is full */
   result = MC_ACK_AddMessage(neighbourID2, ACK_RACHS_E, APP_MSG_TYPE_ALARM_SIGNAL_E, false, handle, &alarmSignalMsg, true);
   CU_ASSERT_FALSE(result);

   /* The message count should be ACK_MAX_UNACKED_MESSAGES */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, ACK_MAX_UNACKED_MESSAGES);

   /* Ensure that nothing went into the RACH1 queue */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 0);
}

/*************************************************************************************/
/**
* ACK_AcknowledgeMessageTest
* Test for function MC_ACK_AcknowledgeMessage
*
* no params
*
* @return - none

*/

void ACK_AcknowledgeMessageTest(void)
{
   uint32_t neighbourID_1 = 1;
   uint32_t neighbourID_2 = 2;
   uint32_t handle = 1;
   ApplicationLayerMessageType_t MessageType;

   bool result = false;

   /* clear the Back-off message queues */
   MC_ACK_Initialise();

   /*Create a fire signal */
   CO_Message_t fireSignalMsg;
   fireSignalMsg.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg.Payload.PhyDataReq.Data, fireSignalData1, ALARM_SIGNAL_BINARY_SIZE);
   /*push the fire signal into the back-off store.
      Note: we won't be using this signal for our test, we just want to ensure it remains
      unaltered in the message queue when we test the RACH2 channel */
   result = MC_ACK_AddMessage(neighbourID_1, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, false, handle, &fireSignalMsg, true);
   CU_ASSERT_TRUE(result);


   /*Create an alarm signal */
   CO_Message_t alarmSignalMsg;
   alarmSignalMsg.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   alarmSignalMsg.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(alarmSignalMsg.Payload.PhyDataReq.Data, alarmSignalData1, ALARM_SIGNAL_BINARY_SIZE);

   /*push the alarm signal into the back-off store for both neighbours*/
   handle++;
   result = MC_ACK_AddMessage(neighbourID_1, ACK_RACHS_E, APP_MSG_TYPE_ALARM_SIGNAL_E, false, handle, &alarmSignalMsg, true);
   CU_ASSERT_TRUE(result);



   /* add another message */
   memcpy(alarmSignalMsg.Payload.PhyDataReq.Data, alarmSignalData2, ALARM_SIGNAL_BINARY_SIZE);

   /*push the alarm signal into the back-off store*/
   handle++;
   result = MC_ACK_AddMessage(neighbourID_2, ACK_RACHS_E, APP_MSG_TYPE_ALARM_SIGNAL_E, false, handle, &alarmSignalMsg, true);
   CU_ASSERT_TRUE(result);

   /* check the stored message count */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 1);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 2);

   /* send an ACK */
   uint32_t ReturnHandle;
   result = MC_ACK_AcknowledgeMessage(ACK_RACHS_E, neighbourID_1, true, &MessageType, &ReturnHandle);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(2, ReturnHandle);
   CU_ASSERT_EQUAL(APP_MSG_TYPE_ALARM_SIGNAL_E, MessageType);


   /* Now there should be only one message in the queue */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 1);

   /* That message should be marked for immediate transmission */
   CU_ASSERT_TRUE(unackQueue[ACK_RACHS_E].ReadyToSend);


   /* check the channel dependent attributes */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].BackoffExp, 0);
   CU_ASSERT_TRUE(3 > unackQueue[ACK_RACHS_E].BackoffCounter);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 1);

   /* check the unack'd message list */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].channelQueue[0].Message.Type, CO_MESSAGE_PHY_DATA_REQ_E);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].channelQueue[0].Message.Payload.PhyDataReq.Size, ALARM_SIGNAL_BINARY_SIZE);
   for (int32_t index = 0; index < ALARM_SIGNAL_BINARY_SIZE; index++)
   {
      CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].channelQueue[0].Message.Payload.PhyDataReq.Data[index], alarmSignalData2[index]);
   }

   /* check that the other neighbour still has one message queued */
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 1);

   /*Now check that the fire signal that we pushed is still in the RACH1 queue */

      CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffExp, 0);
      CU_ASSERT_TRUE(3 > unackQueue[ACK_RACHP_E].BackoffCounter);
      CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 1);

      /* check the unack'd message list */
      CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].channelQueue[0].Message.Type, CO_MESSAGE_PHY_DATA_REQ_E);
      CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].channelQueue[0].Message.Payload.PhyDataReq.Size, ALARM_SIGNAL_BINARY_SIZE);
      for (int32_t index = 0; index < ALARM_SIGNAL_BINARY_SIZE; index++)
      {
         CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].channelQueue[0].Message.Payload.PhyDataReq.Data[index], fireSignalData1[index]);
      }

}

/*************************************************************************************/
/**
* void ACK_WaitingForACKTest(void);

* Test for function MC_ACK_WaitingForACK
*
* no params
*
* @return - none

*/
void ACK_WaitingForACKTest(void)
{
   uint32_t neighbourID_1 = 1;
   uint32_t handle = 1;
   ApplicationLayerMessageType_t MessageType;

   bool result = false;

   MC_ACK_Initialise();


   /* Test with no messages waiting */
   result = MC_ACK_WaitingForACK(ACK_RACHP_E);
   CU_ASSERT_FALSE(result);
   result = MC_ACK_WaitingForACK(ACK_RACHS_E);
   CU_ASSERT_FALSE(result);
 
   /* create a test message */
   CO_Message_t fireSignalMsg;
   fireSignalMsg.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg.Payload.PhyDataReq.Data, fireSignalData1, ALARM_SIGNAL_BINARY_SIZE);

   /* push the message onto neighbourID_1 RACH1 */ 
   result = MC_ACK_AddMessage(neighbourID_1, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, false, handle, &fireSignalMsg, true);
   CU_ASSERT_TRUE(result);

   /* Test again */
   result = MC_ACK_WaitingForACK(ACK_RACHP_E);
   CU_ASSERT_TRUE(result);
   result = MC_ACK_WaitingForACK(ACK_RACHS_E);
   CU_ASSERT_FALSE(result);

   /* push the message onto neighbourID_1 RACH2 */
   handle++;
   result = MC_ACK_AddMessage(neighbourID_1, ACK_RACHS_E, APP_MSG_TYPE_ALARM_SIGNAL_E, false, handle, &fireSignalMsg, true);

   /* Test again */
   result = MC_ACK_WaitingForACK(ACK_RACHP_E);
   CU_ASSERT_TRUE(result);
   result = MC_ACK_WaitingForACK(ACK_RACHS_E);
   CU_ASSERT_TRUE(result);
 
   /* ack RACH1 */
   uint32_t returned_handle;
   result = MC_ACK_AcknowledgeMessage(ACK_RACHP_E, neighbourID_1, true, &MessageType, &returned_handle);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(1, returned_handle);
   CU_ASSERT_EQUAL(APP_MSG_TYPE_FIRE_SIGNAL_E, MessageType);

   /* Test again */
   result = MC_ACK_WaitingForACK(ACK_RACHP_E);
   CU_ASSERT_FALSE(result);
   result = MC_ACK_WaitingForACK(ACK_RACHS_E);
   CU_ASSERT_TRUE(result);

   /* ack RACH2 */
   result = MC_ACK_AcknowledgeMessage(ACK_RACHS_E, neighbourID_1, true, &MessageType, &returned_handle);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(2, returned_handle);
   CU_ASSERT_EQUAL(APP_MSG_TYPE_ALARM_SIGNAL_E, MessageType);


   /* Test again */
   result = MC_ACK_WaitingForACK(ACK_RACHP_E);
   CU_ASSERT_FALSE(result);
   result = MC_ACK_WaitingForACK(ACK_RACHS_E);
   CU_ASSERT_FALSE(result);
}


/*************************************************************************************/
/**
* void ACK_UpdateSlotTest(void);

* Test for function MC_ACK_UpdateSlot
*
* no params
*
* @return - none

*/
void ACK_UpdateSlotTest(void)
{
   uint32_t neighbourID_1 = 1234;
   uint32_t handle = 1;

   bool result = false;

   MC_ACK_Initialise();
   /*Set up the session manager so that it doesn't interfere with the backoff process
      by deleting messages for unresponsive parents*/
   MC_SMGR_Initialise(false,2);
   MC_SMGR_SetParentNodes(18,19);


   /* Test with no messages waiting */
   result = MC_ACK_UpdateSlot(ACK_RACHP_E);
   CU_ASSERT_FALSE(result);
   result = MC_ACK_UpdateSlot(ACK_RACHS_E);
   CU_ASSERT_FALSE(result);

   /* create a test message */
   CO_Message_t fireSignalMsg;
   fireSignalMsg.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg.Payload.PhyDataReq.Data, fireSignalData1, ALARM_SIGNAL_BINARY_SIZE);

   /* push the message onto neighbourID_1 RACHP */
   result = MC_ACK_AddMessage(neighbourID_1, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, false, handle, &fireSignalMsg, true);
   CU_ASSERT_TRUE(result);

   /* set all the back-off counters to 2 */
   unackQueue[ACK_RACHP_E].BackoffCounter = 2;
   unackQueue[ACK_RACHS_E].BackoffCounter = 2;
   unackQueue[ACK_RACHP_E].ReadyToSend = false;
   unackQueue[ACK_RACHS_E].ReadyToSend = false;
 
   /* update the slot and check the back-off counter is decermented on the 
      ACK_RACHP_E channel, but not on the other channel.*/
   result = MC_ACK_UpdateSlot(ACK_RACHP_E);
   CU_ASSERT_FALSE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffCounter, 1);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].BackoffCounter, 2);

   /*There should be no messages marked for immediate transmission */
   CU_ASSERT_FALSE(unackQueue[ACK_RACHP_E].ReadyToSend);
   CU_ASSERT_FALSE(unackQueue[ACK_RACHS_E].ReadyToSend);


   /*update RACHS.  there should be no change beause ther are no RACHS messages queued */
   result = MC_ACK_UpdateSlot(ACK_RACHS_E);
   CU_ASSERT_FALSE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffCounter, 1);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].BackoffCounter, 2);
   /*There should be no messages marked for immediate transmission */
   CU_ASSERT_FALSE(unackQueue[ACK_RACHP_E].ReadyToSend);
   CU_ASSERT_FALSE(unackQueue[ACK_RACHS_E].ReadyToSend);

   /* Update RACHP again. RACHP count should be set to 0.  Then function should return false*/
   result = MC_ACK_UpdateSlot(ACK_RACHP_E);
   CU_ASSERT_FALSE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffExp, 0);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffCounter, 0);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].BackoffCounter, 2);

   CU_ASSERT_FALSE(unackQueue[ACK_RACHP_E].ReadyToSend);
   CU_ASSERT_FALSE(unackQueue[ACK_RACHS_E].ReadyToSend);


   /* Update RACHP again. RACHP exponent should increase to 1 and the count be set in the range 1 to 2.  Then function should return true*/
   result = MC_ACK_UpdateSlot(ACK_RACHP_E);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffExp, 1);
   CU_ASSERT_TRUE(3 > unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_TRUE(0 < unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].BackoffCounter, 2);
   /*Only unackQueue[ACK_RACHP_E] should be marked for immediate transmission */
   CU_ASSERT_TRUE(unackQueue[ACK_RACHP_E].ReadyToSend);
   CU_ASSERT_FALSE(unackQueue[ACK_RACHS_E].ReadyToSend);

   //Keep updating RACHP channel until BackoffCounter is 0
   while (0 < unackQueue[ACK_RACHP_E].BackoffCounter)
   {
      result = MC_ACK_UpdateSlot(ACK_RACHP_E);
      CU_ASSERT_FALSE(result);
   }
   /* Update RACHP again. RACHP exponent should increase to 2 and the count be set in the range 1 to 4.  Then function should return true*/
   result = MC_ACK_UpdateSlot(ACK_RACHP_E);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffExp, 2);
   CU_ASSERT_TRUE(5 > unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_TRUE(0 < unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].BackoffCounter, 2);
   /*Only unackQueue[ACK_RACHP_E] should be marked for immediate transmission */
   CU_ASSERT_TRUE(unackQueue[ACK_RACHP_E].ReadyToSend);
   CU_ASSERT_FALSE(unackQueue[ACK_RACHS_E].ReadyToSend);

   //Keep updating RACHP channel until BackoffCounter is 0
   while (0 < unackQueue[ACK_RACHP_E].BackoffCounter)
   {
      result = MC_ACK_UpdateSlot(ACK_RACHP_E);
      CU_ASSERT_FALSE(result);
   }
   /* Update RACHP again. RACHP exponent should increase to 3 and the count be set in the range 1 to 8.  Then function should return true*/
   result = MC_ACK_UpdateSlot(ACK_RACHP_E);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffExp, 3);
   CU_ASSERT_TRUE(9 > unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_TRUE(0 < unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].BackoffCounter, 2);
   /*Only unackQueue[ACK_RACHP_E] should be marked for immediate transmission */
   CU_ASSERT_TRUE(unackQueue[ACK_RACHP_E].ReadyToSend);
   CU_ASSERT_FALSE(unackQueue[ACK_RACHS_E].ReadyToSend);

   //Keep updating RACHP channel until BackoffCounter is 0
   while (0 < unackQueue[ACK_RACHP_E].BackoffCounter)
   {
      result = MC_ACK_UpdateSlot(ACK_RACHP_E);
      CU_ASSERT_FALSE(result);
   }
   /* Update RACHP again. RACHP exponent should increase to 4 and the count be set in the range 1 to 16.  Then function should return true*/
   result = MC_ACK_UpdateSlot(ACK_RACHP_E);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffExp, 4);
   CU_ASSERT_TRUE(17 > unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_TRUE(0 < unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].BackoffCounter, 2);
   /*Only unackQueue[ACK_RACHP_E] should be marked for immediate transmission */
   CU_ASSERT_TRUE(unackQueue[ACK_RACHP_E].ReadyToSend);
   CU_ASSERT_FALSE(unackQueue[ACK_RACHS_E].ReadyToSend);

   //Keep updating RACHP channel until BackoffCounter is 0
   while (0 < unackQueue[ACK_RACHP_E].BackoffCounter)
   {
      result = MC_ACK_UpdateSlot(ACK_RACHP_E);
      CU_ASSERT_FALSE(result);
   }
   /* Update RACHP again. RACHP exponent should increase to 5 and the count be set in the range 1 to 32.  Then function should return true*/
   result = MC_ACK_UpdateSlot(ACK_RACHP_E);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffExp, 5);
   CU_ASSERT_TRUE(33 > unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_TRUE(0 < unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].BackoffCounter, 2);
   /*Only unackQueue[ACK_RACHP_E] should be marked for immediate transmission */
   CU_ASSERT_TRUE(unackQueue[ACK_RACHP_E].ReadyToSend);
   CU_ASSERT_FALSE(unackQueue[ACK_RACHS_E].ReadyToSend);

   //Keep updating RACHP channel until BackoffCounter is 0
   while (0 < unackQueue[ACK_RACHP_E].BackoffCounter)
   {
      result = MC_ACK_UpdateSlot(ACK_RACHP_E);
      CU_ASSERT_FALSE(result);
   }
   /* Update RACHP again. RACHP exponent should increase to 6 and the count be set in the range 1 to 64.  Then function should return true*/
   result = MC_ACK_UpdateSlot(ACK_RACHP_E);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffExp, 6);
   CU_ASSERT_TRUE(65 > unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_TRUE(0 < unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].BackoffCounter, 2);
   /*Only unackQueue[ACK_RACHP_E] should be marked for immediate transmission */
   CU_ASSERT_TRUE(unackQueue[ACK_RACHP_E].ReadyToSend);
   CU_ASSERT_FALSE(unackQueue[ACK_RACHS_E].ReadyToSend);

   //Keep updating RACHP channel until BackoffCounter is 0
   while (0 < unackQueue[ACK_RACHP_E].BackoffCounter)
   {
      result = MC_ACK_UpdateSlot(ACK_RACHP_E);
      CU_ASSERT_FALSE(result);
   }
   /* Update RACHP again. RACHP exponent should increase to 7 and the count be set in the range 1 to 128.  Then function should return true*/
   result = MC_ACK_UpdateSlot(ACK_RACHP_E);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffExp, 7);
   CU_ASSERT_TRUE(129 > unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_TRUE(0 < unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].BackoffCounter, 2);
   /*Only unackQueue[ACK_RACHP_E] should be marked for immediate transmission */
   CU_ASSERT_TRUE(unackQueue[ACK_RACHP_E].ReadyToSend);
   CU_ASSERT_FALSE(unackQueue[ACK_RACHS_E].ReadyToSend);

   //Keep updating RACHP channel until BackoffCounter is 0
   while (0 < unackQueue[ACK_RACHP_E].BackoffCounter)
   {
      result = MC_ACK_UpdateSlot(ACK_RACHP_E);
      CU_ASSERT_FALSE(result);
   }
   /* Update RACHP again. RACHP exponent should increase to 8 and the count be set in the range 1 to 256.  Then function should return true*/
   result = MC_ACK_UpdateSlot(ACK_RACHP_E);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffExp, 8);
   CU_ASSERT_TRUE(257 > unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_TRUE(0 < unackQueue[ACK_RACHP_E].BackoffCounter);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].BackoffCounter, 2);
   /*Only unackQueue[ACK_RACHP_E] should be marked for immediate transmission */
   CU_ASSERT_TRUE(unackQueue[ACK_RACHP_E].ReadyToSend);
   CU_ASSERT_FALSE(unackQueue[ACK_RACHS_E].ReadyToSend);

   //Test that the message is discarded when the backoff algorithm has run its full course.
   //Keep updating RACHP channel until BackoffCounter is 0
   while (0 < unackQueue[ACK_RACHP_E].BackoffCounter)
   {
      result = MC_ACK_UpdateSlot(ACK_RACHP_E);
      CU_ASSERT_FALSE(result);
   }
   /* Update RACHP again. RACHP exponent should be set to 0 and the message discarded.  The function should return false*/
   result = MC_ACK_UpdateSlot(ACK_RACHP_E);
   CU_ASSERT_FALSE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffExp, 0);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 0);
}

/*************************************************************************************/
/**
* void ACK_GetRandomDelayTest(void);

* Test for function MC_ACK_GetRandomDelay
*
* no params
*
* @return - none

*/
void ACK_GetRandomDelayTest(void)
{
   uint32_t delay = MC_ACK_GetRandomDelay(0);
   CU_ASSERT_EQUAL(delay, 0);

   delay = MC_ACK_GetRandomDelay(128);
   CU_ASSERT_EQUAL(delay, 0);

   for (uint32_t backOff = 1; backOff < ACK_MAX_BACKOFF_EXPONENT; backOff++)
   {
      for (uint32_t count = 0; count < 100; count++)
      {
         delay = MC_ACK_GetRandomDelay(backOff);
         CU_ASSERT_TRUE(delay < BackoffExponentFilter[backOff]);
      }
   }
}

/*************************************************************************************/
/**
* void ACK_GetMessageTest(void);

* Test for function MC_ACK_GetMessage
*
* no params
*
* @return - none

*/
void ACK_GetMessageTest(void)
{
   uint32_t neighbourID_1 = 1;
   uint32_t neighbourID_2 = 2;
   uint32_t handle = 1;
   ApplicationLayerMessageType_t MessageType;
   bool result = false;

   MC_ACK_Initialise();

   /* do a NULL test */
   result = MC_ACK_GetMessage(ACK_RACHP_E, NULL);
   CU_ASSERT_FALSE(result);

   /* create test messages */
   CO_Message_t fireSignalMsg1;
   fireSignalMsg1.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg1.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg1.Payload.PhyDataReq.Data, fireSignalData1, ALARM_SIGNAL_BINARY_SIZE);
   CO_Message_t fireSignalMsg2;
   fireSignalMsg2.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg2.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg2.Payload.PhyDataReq.Data, fireSignalData2, ALARM_SIGNAL_BINARY_SIZE);

   CO_Message_t alarmSignalMsg1;
   alarmSignalMsg1.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   alarmSignalMsg1.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(alarmSignalMsg1.Payload.PhyDataReq.Data, alarmSignalData1, ALARM_SIGNAL_BINARY_SIZE);
   CO_Message_t alarmSignalMsg2;
   alarmSignalMsg2.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   alarmSignalMsg2.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(alarmSignalMsg2.Payload.PhyDataReq.Data, alarmSignalData2, ALARM_SIGNAL_BINARY_SIZE);
   CO_Message_t alarmSignalMsg3;
   alarmSignalMsg3.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   alarmSignalMsg3.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(alarmSignalMsg3.Payload.PhyDataReq.Data, alarmSignalData3, ALARM_SIGNAL_BINARY_SIZE);

   /* push a message onto RACH1 */
   result = MC_ACK_AddMessage(neighbourID_1, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, false, handle, &fireSignalMsg1, true);
   CU_ASSERT_TRUE(result);
   /* push a message onto RACH1 */
   handle++;
   result = MC_ACK_AddMessage(neighbourID_1, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, false, handle, &fireSignalMsg2, true);
   CU_ASSERT_TRUE(result);
   /* push 2 messages onto RACH2 */
   handle++;
   result = MC_ACK_AddMessage(neighbourID_1, ACK_RACHS_E, APP_MSG_TYPE_ALARM_SIGNAL_E, false, handle, &alarmSignalMsg1, true);
   CU_ASSERT_TRUE(result);
   handle++;
   result = MC_ACK_AddMessage(neighbourID_2, ACK_RACHS_E, APP_MSG_TYPE_ALARM_SIGNAL_E, false, handle, &alarmSignalMsg2, true);
   CU_ASSERT_TRUE(result);
   /* push a message onto RACH2 */
   handle++;
   result = MC_ACK_AddMessage(neighbourID_1, ACK_RACHS_E, APP_MSG_TYPE_ALARM_SIGNAL_E, false, handle, &alarmSignalMsg3, true);
   CU_ASSERT_TRUE(result);

   /* msg object to hold the response */
   CO_Message_t testMessage;

   /* Call MC_ACK_GetMessage for RACH1.  We should get the fire signal from neighbourID_1 */
   result = MC_ACK_GetMessage(ACK_RACHP_E, &testMessage);
   CU_ASSERT_TRUE(result);
   if (result)
   {
      /* check that the result is fileSignal1 */
      for (int32_t index = 0; index < ALARM_SIGNAL_BINARY_SIZE; index++)
      {
         CU_ASSERT_EQUAL(testMessage.Payload.PhyDataReq.Data[index], fireSignalMsg1.Payload.PhyDataReq.Data[index]);
      }
   }


   /* Call MC_ACK_GetMessage for RACH1 again.  We should get no message because we are waitng for an ack */
   result = MC_ACK_GetMessage(ACK_RACHP_E, &testMessage);
   CU_ASSERT_FALSE(result);


   /* Call MC_ACK_GetMessage for RACH2.  We should get alarmSignalMsg1 */
   result = MC_ACK_GetMessage(ACK_RACHS_E, &testMessage);
   CU_ASSERT_TRUE(result);
   if (result)
   {
      /* check that the result is alarmSignalMsg1 */
      for (int32_t index = 0; index < ALARM_SIGNAL_BINARY_SIZE; index++)
      {
         CU_ASSERT_EQUAL(testMessage.Payload.PhyDataReq.Data[index], alarmSignalMsg1.Payload.PhyDataReq.Data[index]);
      }
   }

   /* Call MC_ACK_GetMessage for RACH2 again.  We should get nothing back because we are waiting for an ack.*/
   result = MC_ACK_GetMessage(ACK_RACHS_E, &testMessage);
   CU_ASSERT_FALSE(result);

   /* Acknowledge the message on RACH2 */
   uint32_t returned_handle;
   result = MC_ACK_AcknowledgeMessage(ACK_RACHS_E, neighbourID_1, true, &MessageType, &returned_handle);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(3, returned_handle);
   CU_ASSERT_EQUAL(APP_MSG_TYPE_ALARM_SIGNAL_E, MessageType);

   /* Call MC_ACK_GetMessage for RACH2 again.  We should get alarmSignalMsg2 */
   result = MC_ACK_GetMessage(ACK_RACHS_E, &testMessage);
   CU_ASSERT_TRUE(result);
   if (result)
   {
      /* check that the result is alarmSignalMsg2 */
      for (int32_t index = 0; index < ALARM_SIGNAL_BINARY_SIZE; index++)
      {
         CU_ASSERT_EQUAL(testMessage.Payload.PhyDataReq.Data[index], alarmSignalMsg2.Payload.PhyDataReq.Data[index]);
      }
   }

   /* Call MC_ACK_GetMessage for RACH2 again.  We should get no message because we are waiting for acks from both neighbours*/
   result = MC_ACK_GetMessage(ACK_RACHS_E, &testMessage);
   CU_ASSERT_FALSE(result);

   /* Acknowledge the message on RACH2 */
   result = MC_ACK_AcknowledgeMessage(ACK_RACHS_E, neighbourID_2, true, &MessageType, &returned_handle);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(4, returned_handle);
   CU_ASSERT_EQUAL(APP_MSG_TYPE_ALARM_SIGNAL_E, MessageType);

}


/*************************************************************************************/
/**
* void ACK_RerouteMessageTest(void);

* Test for function MC_ACK_RerouteMessage
*
* no params
*
* @return - none

*/
void ACK_RerouteMessageTest(void)
{
   uint32_t neighbourID_1 = 0x123;
   uint32_t neighbourID_2 = 0x012;
   bool result = false;

   uint8_t fireSignalPayload[ALARM_SIGNAL_BINARY_SIZE] = { 0x11,0x23,0x20,0x20,0x43,0x03,0x40,0x41,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };

   MC_ACK_Initialise();
   //Set up session manager with to parents
   MC_SMGR_Initialise(false, 0);
   uint16_t neighbour1DCHSlot = MC_SMGR_CalculateDCHSlot(neighbourID_1);
   uint16_t neighbour2DCHSlot = MC_SMGR_CalculateDCHSlot(neighbourID_2);
   MC_SMGR_SetParentNodes(neighbour1DCHSlot, neighbour2DCHSlot);
   MC_SMGR_RouteAddResponse(neighbourID_1, true);
   MC_SMGR_RouteAddResponse(neighbourID_2, true);

   /* do a NULL test */
   result = MC_ACK_GetMessage(ACK_RACHP_E, NULL);
   CU_ASSERT_FALSE(result);

   /* create test messages */
   CO_Message_t fireSignalMsg;
   fireSignalMsg.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg.Payload.PhyDataReq.Data, fireSignalPayload, ALARM_SIGNAL_BINARY_SIZE);

   // add msg to ACK Manager
   MC_ACK_AddMessage(neighbourID_1, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg, true);
   //check that it's queued with the correct mac destination address (neighbourID_1)
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 1);
   uint8_t* pMsg = unackQueue[ACK_RACHP_E].channelQueue[0].Message.Payload.PhyDataInd.Data;
   uint16_t macDestination = (uint16_t)(*pMsg) << 8;
   macDestination |= *(pMsg + 1);
   macDestination &= 0x0fff;
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);

   // call the reroute function
   result = MC_ACK_RerouteMessage(ACK_RACHP_E, 0);
   CU_ASSERT_TRUE(result);

   //Check that the mac destination has changed to neighbourID_2
   macDestination = (uint16_t)(*pMsg) << 8;
   macDestination |= *(pMsg + 1);
   macDestination &= 0x0fff;
   CU_ASSERT_EQUAL(macDestination, neighbourID_2);

   // call the reroute function again.  It should divert the message back to neighbourID_1
   result = MC_ACK_RerouteMessage(ACK_RACHP_E, 0);
   CU_ASSERT_TRUE(result);

   //Check that the mac destination has changes to neighbourID_1
   macDestination = (uint16_t)(*pMsg) << 8;
   macDestination |= *(pMsg + 1);
   macDestination &= 0x0fff;
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);

}

/*************************************************************************************/
/**
* void ACK_SendOnNextSlotTest(void);

* Test to check that new messages go out on the next available slot
*
* no params
*
* @return - none

*/
void ACK_SendOnNextSlotTest(void)
{
   uint32_t parentID = 1;
   uint32_t handle = 1;

   bool result = false;

   MC_ACK_Initialise();


   /* Test with no messages waiting */
   result = MC_ACK_UpdateSlot(ACK_RACHP_E);
   CU_ASSERT_FALSE(result);
   result = MC_ACK_UpdateSlot(ACK_RACHS_E);
   CU_ASSERT_FALSE(result);

   /* create a test message */
   CO_Message_t fireSignalMsg;
   fireSignalMsg.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg.Payload.PhyDataReq.Data, fireSignalData1, ALARM_SIGNAL_BINARY_SIZE);

   /* push the message onto neighbourID_1 RACHP */
   result = MC_ACK_AddMessage(parentID, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, false, handle, &fireSignalMsg, true);
   CU_ASSERT_TRUE(result);
   // Check that the message is available immediately
   CU_ASSERT_TRUE(unackQueue[ACK_RACHP_E].ReadyToSend);
   CO_Message_t Message;
   result = MC_ACK_GetMessage(ACK_RACHP_E, &Message);
   CU_ASSERT_TRUE(result);
   if (result)
   {
      /* check that the result is fileSignal1 */
      for (int32_t index = 0; index < ALARM_SIGNAL_BINARY_SIZE; index++)
      {
         CU_ASSERT_EQUAL(Message.Payload.PhyDataReq.Data[index], fireSignalData1[index]);
      }
   }

   // Check that new messages do not inherit the backoff counter of the previous message
   /* push the message onto RACHP */
   unackQueue[ACK_RACHP_E].MessageCount = 0;//reset the mesage count
   result = MC_ACK_AddMessage(parentID, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, false, handle, &fireSignalMsg, true);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 1);
   // Set the backoff counters
   unackQueue[ACK_RACHP_E].BackoffExp = 3;
   unackQueue[ACK_RACHP_E].BackoffCounter = 5;

   //run a couple of slot cycles
   result = MC_ACK_UpdateSlot(ACK_RACHP_E);
   CU_ASSERT_FALSE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffExp, 3);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffCounter, 4);
   result = MC_ACK_UpdateSlot(ACK_RACHP_E);
   CU_ASSERT_FALSE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffExp, 3);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffCounter, 3);

   // Acknowledge the message
   ApplicationLayerMessageType_t MessageType;
   result = MC_ACK_AcknowledgeMessage(ACK_RACHP_E, parentID, true, &MessageType, &handle);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 0);
   // Now add a new message and ensure it is ready for transmission immediately
   result = MC_ACK_AddMessage(parentID, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, false, handle, &fireSignalMsg, true);
   CU_ASSERT_TRUE(result);

   //run a slot cycle.  It should report ready to send (return true).
   result = MC_ACK_UpdateSlot(ACK_RACHP_E);
   CU_ASSERT_TRUE(result);
   // Check that the message is available immediately
   CU_ASSERT_TRUE(unackQueue[ACK_RACHP_E].ReadyToSend);

   if (result) // The state machine will only request a message if the previous MC_ACK_UpdateSlot returned true
   {
      result = MC_ACK_GetMessage(ACK_RACHP_E, &Message);
      CU_ASSERT_TRUE(result);
      if (result)
      {
         /* check that the result is fileSignal1 */
         for (int32_t index = 0; index < ALARM_SIGNAL_BINARY_SIZE; index++)
         {
            CU_ASSERT_EQUAL(Message.Payload.PhyDataReq.Data[index], fireSignalData1[index]);
         }
      }

      result = MC_ACK_AcknowledgeMessage(ACK_RACHP_E, parentID, true, &MessageType, &handle);
      CU_ASSERT_TRUE(result);
   }
   // If all went well there will be no messages left to send
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 0);
   // the backoff algorithm should have been reset.
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffExp, 0);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].BackoffCounter, 0);

}


/*************************************************************************************/
/**
* void ACK_GetMacDestinationOfQueuedMessageTest(void);

* Test for function MC_ACK_GetMacDestinationOfQueuedMessage
*
* no params
*
* @return - none

*/
void ACK_GetMacDestinationOfQueuedMessageTest(void)
{
   uint32_t neighbourID_1 = 0x123;
   uint32_t neighbourID_2 = 0x012;
   bool result = false;

   uint8_t fireSignalPayload1[ALARM_SIGNAL_BINARY_SIZE] = { 0x11,0x23,0x20,0x20,0x43,0x03,0x40,0x41,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   uint8_t fireSignalPayload2[ALARM_SIGNAL_BINARY_SIZE] = { 0x10,0x12,0x20,0x20,0x43,0x03,0x40,0x41,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   uint8_t fireSignalPayload3[ALARM_SIGNAL_BINARY_SIZE] = { 0x11,0x23,0x20,0x20,0x53,0x03,0x40,0x41,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };

   MC_ACK_Initialise();
   //Set up session manager with to parents
   MC_SMGR_Initialise(false, 0);
   uint16_t neighbour1DCHSlot = MC_SMGR_CalculateDCHSlot(neighbourID_1);
   uint16_t neighbour2DCHSlot = MC_SMGR_CalculateDCHSlot(neighbourID_2);
   MC_SMGR_SetParentNodes(neighbour1DCHSlot, neighbour2DCHSlot);
   MC_SMGR_RouteAddResponse(neighbourID_1, true);
   MC_SMGR_RouteAddResponse(neighbourID_2, true);

   /* do a NULL test */
   result = MC_ACK_GetMessage(ACK_RACHP_E, NULL);
   CU_ASSERT_FALSE(result);

   /* create test messages */
   CO_Message_t fireSignalMsg1;
   fireSignalMsg1.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg1.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg1.Payload.PhyDataReq.Data, fireSignalPayload1, ALARM_SIGNAL_BINARY_SIZE);

   // add msg to ACK Manager
   MC_ACK_AddMessage(neighbourID_1, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg1, true);
   //check the number of queued messages
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 1);

   CO_Message_t fireSignalMsg2;
   fireSignalMsg2.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg2.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg2.Payload.PhyDataReq.Data, fireSignalPayload2, ALARM_SIGNAL_BINARY_SIZE);

   // add msg to ACK Manager
   MC_ACK_AddMessage(neighbourID_2, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg2, true);
   //check the number of queued messages
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 2);

   CO_Message_t fireSignalMsg3;
   fireSignalMsg3.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg3.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg3.Payload.PhyDataReq.Data, fireSignalPayload3, ALARM_SIGNAL_BINARY_SIZE);

   // add msg to ACK Manager
   MC_ACK_AddMessage(neighbourID_1, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg3, true);
   //check the number of queued messages
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 3);


   //Check that the first queued msg has a mac destination is neighbourID_1
   uint16_t macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 0);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);

   //Check that the second queued msg has a mac destination is neighbourID_1
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 1);
   CU_ASSERT_EQUAL(macDestination, neighbourID_2);

   //Check that the third queued msg has a mac destination is neighbourID_1
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 2);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);

   //Check that the current msg has a mac destination is neighbourID_1
   macDestination = MC_ACK_GetMacDestinationOfCurrentMessage(ACK_RACHP_E);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);
}

/*************************************************************************************/
/**
* void ACK_RedirectMessagesForNodeRACHPTest(void);

* Test for function MC_ACK_RedirectMessagesForNode for RACHP queue
*
* no params
*
* @return - none

*/
void ACK_RedirectMessagesForNodeRACHPTest(void)
{
   uint32_t neighbourID_1 = 0x123;
   uint32_t neighbourID_2 = 0x012;
   bool result = false;

   uint8_t fireSignalPayload1[ALARM_SIGNAL_BINARY_SIZE] = { 0x11,0x23,0x20,0x20,0x43,0x03,0x40,0x41,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   uint8_t fireSignalPayload2[ALARM_SIGNAL_BINARY_SIZE] = { 0x10,0x12,0x20,0x20,0x43,0x03,0x40,0x41,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   uint8_t fireSignalPayload3[ALARM_SIGNAL_BINARY_SIZE] = { 0x11,0x23,0x20,0x20,0x53,0x04,0x40,0x41,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };

   MC_ACK_Initialise();
   //Set up session manager with to parents
   MC_SMGR_Initialise(false, 0);
   uint16_t neighbour1DCHSlot = MC_SMGR_CalculateDCHSlot(neighbourID_1);
   uint16_t neighbour2DCHSlot = MC_SMGR_CalculateDCHSlot(neighbourID_2);
   MC_SMGR_SetParentNodes(neighbour1DCHSlot, neighbour2DCHSlot);
   MC_SMGR_RouteAddResponse(neighbourID_1, true);
 
   /* do a NULL test */
   result = MC_ACK_GetMessage(ACK_RACHP_E, NULL);
   CU_ASSERT_FALSE(result);

   /* create test messages */
   CO_Message_t fireSignalMsg1;
   fireSignalMsg1.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg1.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg1.Payload.PhyDataReq.Data, fireSignalPayload1, ALARM_SIGNAL_BINARY_SIZE);

   // add msg to ACK Manager
   MC_ACK_AddMessage(neighbourID_1, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg1, true);
   //check the number of queued messages
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 1);

   CO_Message_t fireSignalMsg2;
   fireSignalMsg2.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg2.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg2.Payload.PhyDataReq.Data, fireSignalPayload2, ALARM_SIGNAL_BINARY_SIZE);

   // add msg to ACK Manager
   MC_ACK_AddMessage(neighbourID_2, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg2, true);
   //check the number of queued messages
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 2);

   CO_Message_t fireSignalMsg3;
   fireSignalMsg3.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg3.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg3.Payload.PhyDataReq.Data, fireSignalPayload3, ALARM_SIGNAL_BINARY_SIZE);

   // add msg to ACK Manager
   MC_ACK_AddMessage(neighbourID_1, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg3, true);
   //check the number of queued messages
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 3);


   //Check that the first queued msg has a mac destination is neighbourID_1
   uint16_t macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 0);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);
   //Check that the second queued msg has a mac destination is neighbourID_1
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 1);
   CU_ASSERT_EQUAL(macDestination, neighbourID_2);
   //Check that the third queued msg has a mac destination is neighbourID_1
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 2);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);

   /* try to redirect the message for the secondary parent. */
   MC_ACK_RedirectMessagesForNode(neighbourID_2);

   /*All three messages should now be addressed to neighbourID_1*/
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 0);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 1);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 2);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);

   /* try to redirect the messages for neighbourID_1
      They should all be deleted because the secondary parent is not active */
   MC_ACK_RedirectMessagesForNode(neighbourID_1);
   //check the number of queued messages
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 0);

   /* repeat the test with an active secondary parent
      This time the messages should be redirected, not deleted */

   /*Activate the secondary parent*/
   MC_SMGR_RouteAddResponse(neighbourID_2, true);

   // add messages to the ACK Manager
   MC_ACK_AddMessage(neighbourID_1, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg1, true);
   MC_ACK_AddMessage(neighbourID_2, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg2, true);
   MC_ACK_AddMessage(neighbourID_1, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg3, true);
   //check the number of queued messages
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 3);

   //Check that the first queued msg has a mac destination is neighbourID_1
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 0);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);
   //Check that the second queued msg has a mac destination is neighbourID_1
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 1);
   CU_ASSERT_EQUAL(macDestination, neighbourID_2);
   //Check that the third queued msg has a mac destination is neighbourID_1
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 2);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);


   /* try to redirect the messages for the primary parent. */
   MC_ACK_RedirectMessagesForNode(neighbourID_1);

   //check the number of queued messages
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 3);

   /*All three messages should now be addressed to neighbourID_2*/
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 0);
   CU_ASSERT_EQUAL(macDestination, neighbourID_2);
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 1);
   CU_ASSERT_EQUAL(macDestination, neighbourID_2);
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 2);
   CU_ASSERT_EQUAL(macDestination, neighbourID_2);


}

/*************************************************************************************/
/**
* void ACK_RedirectMessagesForNodeRACHSTest(void);

* Test for function MC_ACK_RedirectMessagesForNode for RACHS queue
*
* no params
*
* @return - none

*/
void ACK_RedirectMessagesForNodeRACHSTest(void)
{
   uint32_t neighbourID_1 = 0x123;
   uint32_t neighbourID_2 = 0x012;
   bool result = false;

   uint8_t fireSignalPayload1[ALARM_SIGNAL_BINARY_SIZE] = { 0x11,0x23,0x20,0x20,0x43,0x03,0x40,0x41,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   uint8_t fireSignalPayload2[ALARM_SIGNAL_BINARY_SIZE] = { 0x10,0x12,0x20,0x20,0x43,0x03,0x40,0x41,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   uint8_t fireSignalPayload3[ALARM_SIGNAL_BINARY_SIZE] = { 0x11,0x23,0x20,0x20,0x53,0x03,0x40,0x41,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };

   MC_ACK_Initialise();
   //Set up session manager with to parents
   MC_SMGR_Initialise(false, 0);
   uint16_t neighbour1DCHSlot = MC_SMGR_CalculateDCHSlot(neighbourID_1);
   uint16_t neighbour2DCHSlot = MC_SMGR_CalculateDCHSlot(neighbourID_2);
   MC_SMGR_SetParentNodes(neighbour1DCHSlot, neighbour2DCHSlot);
   MC_SMGR_RouteAddResponse(neighbourID_1, true);

   /* do a NULL test */
   result = MC_ACK_GetMessage(ACK_RACHS_E, NULL);
   CU_ASSERT_FALSE(result);

   /* create test messages */
   CO_Message_t fireSignalMsg1;
   fireSignalMsg1.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg1.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg1.Payload.PhyDataReq.Data, fireSignalPayload1, ALARM_SIGNAL_BINARY_SIZE);

   // add msg to ACK Manager
   MC_ACK_AddMessage(neighbourID_1, ACK_RACHS_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg1, true);
   //check the number of queued messages
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 1);

   CO_Message_t fireSignalMsg2;
   fireSignalMsg2.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg2.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg2.Payload.PhyDataReq.Data, fireSignalPayload2, ALARM_SIGNAL_BINARY_SIZE);

   // add msg to ACK Manager
   MC_ACK_AddMessage(neighbourID_2, ACK_RACHS_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg2, true);
   //check the number of queued messages
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 2);

   CO_Message_t fireSignalMsg3;
   fireSignalMsg3.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg3.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg3.Payload.PhyDataReq.Data, fireSignalPayload3, ALARM_SIGNAL_BINARY_SIZE);

   // add msg to ACK Manager
   MC_ACK_AddMessage(neighbourID_1, ACK_RACHS_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg3, true);
   //check the number of queued messages
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 3);


   //Check that the first queued msg has a mac destination is neighbourID_1
   uint16_t macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHS_E, 0);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);
   //Check that the second queued msg has a mac destination is neighbourID_1
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHS_E, 1);
   CU_ASSERT_EQUAL(macDestination, neighbourID_2);
   //Check that the third queued msg has a mac destination is neighbourID_1
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHS_E, 2);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);

   /* try to redirect the message for the secondary parent. */
   MC_ACK_RedirectMessagesForNode(neighbourID_2);

   /*All three messages should now be addressed to neighbourID_1*/
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHS_E, 0);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHS_E, 1);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHS_E, 2);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);

   /* try to redirect the messages for neighbourID_1
   They should all be deleted because the secondary parent is not active */
   MC_ACK_RedirectMessagesForNode(neighbourID_1);
   //check the number of queued messages
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 0);

   /* repeat the test with an active secondary parent
   This time the messages should be redirected, not deleted */

   /*Activate the secondary parent*/
   MC_SMGR_RouteAddResponse(neighbourID_2, true);

   // add messages to the ACK Manager
   MC_ACK_AddMessage(neighbourID_1, ACK_RACHS_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg1, true);
   MC_ACK_AddMessage(neighbourID_2, ACK_RACHS_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg2, true);
   MC_ACK_AddMessage(neighbourID_1, ACK_RACHS_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg3, true);
   //check the number of queued messages
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 3);

   //Check that the first queued msg has a mac destination is neighbourID_1
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHS_E, 0);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);
   //Check that the second queued msg has a mac destination is neighbourID_1
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHS_E, 1);
   CU_ASSERT_EQUAL(macDestination, neighbourID_2);
   //Check that the third queued msg has a mac destination is neighbourID_1
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHS_E, 2);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);

   /* try to redirect the messages for the primary parent. */
   MC_ACK_RedirectMessagesForNode(neighbourID_1);

   //check the number of queued messages
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 3);

   /*All three messages should now be addressed to neighbourID_2*/
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHS_E, 0);
   CU_ASSERT_EQUAL(macDestination, neighbourID_2);
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHS_E, 1);
   CU_ASSERT_EQUAL(macDestination, neighbourID_2);
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHS_E, 2);
   CU_ASSERT_EQUAL(macDestination, neighbourID_2);
}

/*************************************************************************************/
/**
* void ACK_RedirectMessagesForNodeNonHopTest(void);

* Test for function MC_ACK_RedirectMessagesForNode for messages which are addressed
* to the parent node (final destination).  These messages cannot be rerouted so they
* should be deleted.
*
* no params
*
* @return - none

*/
void ACK_RedirectMessagesForNodeNonHopTest(void)
{
   uint32_t size = 0;
   uint32_t neighbourID_1 = 0x123;
   uint32_t neighbourID_2 = 0x012;
   bool result = false;

   uint8_t fireSignalPayload1[ALARM_SIGNAL_BINARY_SIZE] = { 0x11,0x23,0x20,0x20,0x43,0x03,0x40,0x41,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   uint8_t fireSignalPayload2[ALARM_SIGNAL_BINARY_SIZE] = { 0x10,0x12,0x20,0x20,0x43,0x03,0x40,0x41,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   uint8_t alarmSignalPayload[ALARM_SIGNAL_BINARY_SIZE] = { 0x11,0x23,0x20,0x20,0x43,0x03,0x40,0x41,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   uint8_t routeAddPayload[ROUTEADD_BINARY_SIZE]        = { 0x10,0x12,0x20,0x20,0x00,0x12,0x20,0x24,0x8a,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };

   MC_ACK_Initialise();
   //Set up session manager with two parents
   MC_SMGR_Initialise(false, 0);
   uint16_t neighbour1DCHSlot = MC_SMGR_CalculateDCHSlot(neighbourID_1);
   uint16_t neighbour2DCHSlot = MC_SMGR_CalculateDCHSlot(neighbourID_2);
   MC_SMGR_SetParentNodes(neighbour1DCHSlot, neighbour2DCHSlot);
   MC_SMGR_RouteAddResponse(neighbourID_1, true);
   MC_SMGR_RouteAddResponse(neighbourID_2, true);

   /* create test messages */
   CO_Message_t fireSignalMsg1;
   fireSignalMsg1.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg1.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg1.Payload.PhyDataReq.Data, fireSignalPayload1, ALARM_SIGNAL_BINARY_SIZE);

   CO_Message_t fireSignalMsg2;
   fireSignalMsg2.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   fireSignalMsg2.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(fireSignalMsg2.Payload.PhyDataReq.Data, fireSignalPayload2, ALARM_SIGNAL_BINARY_SIZE);

   CO_Message_t alarmSignalMsg1;
   alarmSignalMsg1.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   alarmSignalMsg1.Payload.PhyDataReq.Size = ALARM_SIGNAL_BINARY_SIZE;
   memcpy(alarmSignalMsg1.Payload.PhyDataReq.Data, alarmSignalPayload, ALARM_SIGNAL_BINARY_SIZE);

   CO_Message_t routeAdd1;
   routeAdd1.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   routeAdd1.Payload.PhyDataReq.Size = ROUTEADD_BINARY_SIZE;
   memcpy(routeAdd1.Payload.PhyDataReq.Data, routeAddPayload, ROUTEADD_BINARY_SIZE);

   /* push the messages into the Ack Manager*/
   MC_ACK_AddMessage(neighbourID_1, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg1, true);
   MC_ACK_AddMessage(neighbourID_2, ACK_RACHP_E, APP_MSG_TYPE_ROUTE_ADD_E, false, 0, &routeAdd1, true);
   MC_ACK_AddMessage(neighbourID_1, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, true, 0, &fireSignalMsg2, true);
   MC_ACK_AddMessage(neighbourID_2, ACK_RACHS_E, APP_MSG_TYPE_ROUTE_ADD_E, false, 0, &routeAdd1, true);
   MC_ACK_AddMessage(neighbourID_1, ACK_RACHS_E, APP_MSG_TYPE_ALARM_SIGNAL_E, true, 0, &alarmSignalMsg1, true);
   //check the number of queued messages
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 3);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 2);

   /* Now ask the Ack Manager to redirect the neighbourID_2 messages.
      Because they are non-hop i.e. that parent is the final destination, they should be deleted rather than rerouted*/
   MC_ACK_RedirectMessagesForNode(neighbourID_2);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].MessageCount, 2);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].MessageCount, 1);

   /* Check that the remaining messages are the correct ones */
   uint16_t macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 0);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHP_E, 1);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);
   macDestination = MC_ACK_GetMacDestinationOfQueuedMessage(ACK_RACHS_E, 0);
   CU_ASSERT_EQUAL(macDestination, neighbourID_1);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].channelQueue[0].MessageType, APP_MSG_TYPE_FIRE_SIGNAL_E);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHP_E].channelQueue[1].MessageType, APP_MSG_TYPE_FIRE_SIGNAL_E);
   CU_ASSERT_EQUAL(unackQueue[ACK_RACHS_E].channelQueue[0].MessageType, APP_MSG_TYPE_ALARM_SIGNAL_E);

}
