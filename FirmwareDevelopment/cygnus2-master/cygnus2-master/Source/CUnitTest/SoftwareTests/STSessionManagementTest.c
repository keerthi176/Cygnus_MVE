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
*  File         : STSessionManagementTest.c
*
*  Description  : Implementation for the Session Management tests
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdarg.h>


/* User Include Files
*************************************************************************************/
#include "lptim.h"
#include "CUnit.h"
#include "CO_Defines.h"
#include "CO_Message.h"
#include "STSessionManagementTest.h"
#include "MC_SessionManagement.h"
#include "CO_ErrorCode.h"
#include "MC_AckManagement.h"
#include "MessageQueueStub.h"
#include "MC_MAC.h"
#include "MC_MeshFormAndHeal.h"
#include "MC_SACH_Management.h"

#define ACK_NEIGHBOUR_UNKNOWN 0xffffffff

uint16_t gNetworkAddress = 0xffff;
uint32_t GetNeighbourIndex_return = ACK_NEIGHBOUR_UNKNOWN;
LPTIM_HandleTypeDef hlptim1;
extern MC_TrackingNodes_t gaTrackingNodes;

extern uint8_t gDeviceRank;
extern ErrorCode_t MC_MAC_Init(const bool isMaster, const uint32_t frequency, const uint32_t systemID, const uint32_t address);
extern bool gResetRequired;
extern CO_State_t gSM_ScheduledDeviceState;
extern CO_State_t gSM_CurrentDeviceState;
extern SASMsgQueue_t sasQueue[SACH_NUMBER_OF_SAS_QUEUES];

/* Private Functions Prototypes
*************************************************************************************/
static void MC_SMGR_CalculateNodeIDTest(void);
static void MC_SMGR_InitialiseTest(void);
static void MC_SMGR_SetParentNodesTest(void);
static void MC_SMGR_IsParentNodeTest(void);
static void MC_SMGR_GetNextParentForUplinkTest(void);
static void MC_SMGR_ConnectToParentTest(void);
static void MC_SMGR_RouteAddResponseTest(void);
static void MC_SMGR_RouteAddResponseTimeoutTest(void);
static void MC_SMGR_RouteAddRequestTest(void);
static void MC_SMGR_RouteDropRequestTest(void);
static void MC_SMGR_IsAChildTest(void);
static void MC_SMGR_SetHeartbeatMonitorTest(void);
static void MC_SMGR_StartStopHeartbeatMonitorTest(void);
static void MC_SMGR_MissedNodeHeartbeatTest(void);
static void MC_SMGR_LosePrimaryParent(void);
static void MC_SMGR_LoseSecondaryParent(void);
static void MC_SMGR_LosePrimaryTrackingNode(void);
static void MC_SMGR_LoseSecondaryTrackingNode(void);
static void MC_SMGR_GetNeighbourInformationTest(void);
static void MC_SMGR_GetNumberOfChildrenIndexTest(void);
static bool MC_SMGR_SetHeartbeatMonitor_SlotNo(const uint32_t super_slot_index, const bool set_monitor);
static void MC_SMGR_CheckParentStatesTest(void);
static void MC_SMGR_GetPrimaryParentIDTest(void);
static void MC_SMGR_NotifyMessageFailureTest(void);

/* Externs
*************************************************************************************/
extern uint16_t MC_SMGR_CalculateNodeID(const uint32_t slot_index);
extern uint16_t MC_SMGR_CalculateDCHSlot(const uint16_t node_id);
extern SMGR_ParentDetails_t gParent[NUMBER_OF_PARENTS_AND_TRACKING_NODES];
extern SMGR_ChildDetails_t gChildren[MAX_NUMBER_OF_CHILDREN];


/* Table containing the test settings */
CU_TestInfo ST_SessMgmtTests[] =
{
   { "InitialiseTest",                  MC_SMGR_InitialiseTest },
   { "CalculateNodeIDTest",             MC_SMGR_CalculateNodeIDTest },
   { "SetParentNodesTest",              MC_SMGR_SetParentNodesTest },
   { "IsParentNodeTest",                MC_SMGR_IsParentNodeTest },
   { "GetNextParentForUplinkTest",      MC_SMGR_GetNextParentForUplinkTest },
   { "ConnectToParentTest",             MC_SMGR_ConnectToParentTest },
   { "RouteAddResponseTest",            MC_SMGR_RouteAddResponseTest },
   { "RouteAddResponseTimeoutTest",     MC_SMGR_RouteAddResponseTimeoutTest },
   { "RouteAddRequestTest",             MC_SMGR_RouteAddRequestTest },
   { "RouteDropRequestTest",            MC_SMGR_RouteDropRequestTest },
   { "IsAChildTest",                    MC_SMGR_IsAChildTest },
   { "SetHeartbeatMonitorTest",         MC_SMGR_SetHeartbeatMonitorTest },
   { "StartStopHeartbeatMonitorTest",   MC_SMGR_StartStopHeartbeatMonitorTest },
   { "MissedNodeHeartbeatTest",         MC_SMGR_MissedNodeHeartbeatTest },
   { "LosePrimaryParent",               MC_SMGR_LosePrimaryParent },
   { "LoseSecondaryParent",             MC_SMGR_LoseSecondaryParent },
   { "LosePrimaryTrackingNode",         MC_SMGR_LosePrimaryTrackingNode },
   { "LoseSecondaryTrackingNode",       MC_SMGR_LoseSecondaryTrackingNode },
   { "GetNeighbourInformationTest",     MC_SMGR_GetNeighbourInformationTest },
   { "GetNumberOfChildrenIndexTest",    MC_SMGR_GetNumberOfChildrenIndexTest },
   { "CheckParentStatesTest",           MC_SMGR_CheckParentStatesTest },
   { "GetPrimaryParentIDTest",          MC_SMGR_GetPrimaryParentIDTest },
   { "NotifyMessageFailureTest",        MC_SMGR_NotifyMessageFailureTest },

   CU_TEST_INFO_NULL,
};

/* Test utilities
*************************************************************************************/

#define SYSTEM_ID 0x12345678
#define WRONG_SYSTEM_ID 0x87654321

/*************************************************************************************
* STUBBED FUNCTIONS
*************************************************************************************/


/*************************************************************************************
* TEST FUNCTIONS
*************************************************************************************/

/*************************************************************************************/
/**
* MC_SMGR_InitialiseTest
* Test the operation of function MC_SMGR_Initialise;
*
* no params
*
* @return - none

*/
void MC_SMGR_InitialiseTest(void)
{
   gDeviceRank = 0;
   gNetworkAddress = 3;
   MC_MAC_Init(false, 0, SYSTEM_ID, gNetworkAddress);
   /* preset the parents and children with dummy values*/
   gParent[PRIMARY_PARENT].NodeInfo = 1;
   gParent[PRIMARY_PARENT].IsActive = true;
   gParent[SECONDARY_PARENT].NodeInfo = 1;
   gParent[SECONDARY_PARENT].IsActive = true;

   for (uint32_t index = 0; index < MAX_NUMBER_OF_CHILDREN; index++)
   {
      gChildren[index].NodeInfo = 1;
   }

   /* Call the Initialise function */
   MC_SMGR_Initialise(false, gNetworkAddress);

   /* Check that the parents and children are initialised */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, NOT_ASSIGNED);
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].IsActive, false);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo,NOT_ASSIGNED);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].IsActive, false);

   for (uint32_t index = 0; index < MAX_NUMBER_OF_CHILDREN; index++)
   {
      CU_ASSERT_EQUAL(gChildren[index].NodeInfo, NOT_ASSIGNED);
   }

}

/*************************************************************************************/
/**
* MC_SMGR_CalculateNodeIDTest
* Test the operation of function MC_SMGR_CalculateNodeID(slot_number);
*
* no params
*
* @return - none

*/
void MC_SMGR_CalculateNodeIDTest(void)
{
   uint32_t max_short_frames = SHORT_FRAMES_PER_LONG_FRAME;// *LONG_FRAMES_PER_SUPER_FRAME;
   uint16_t expected_node_id = 0;
   uint32_t slot_number;
   uint16_t node_id;

   /* positive tests */
   for (uint32_t frame_number = 0; frame_number < max_short_frames; frame_number++)
   {
      slot_number = frame_number * SLOTS_PER_SHORT_FRAME + 16;
      node_id = MC_SMGR_CalculateNodeID(slot_number);
      CU_ASSERT_EQUAL(node_id, expected_node_id);
      slot_number++;
      expected_node_id++;
      node_id = MC_SMGR_CalculateNodeID(slot_number);
      CU_ASSERT_EQUAL(node_id, expected_node_id);
      slot_number++;
      expected_node_id++;
      node_id = MC_SMGR_CalculateNodeID(slot_number);
      CU_ASSERT_EQUAL(node_id, expected_node_id);
      slot_number++;
      expected_node_id++;
      node_id = MC_SMGR_CalculateNodeID(slot_number);
      CU_ASSERT_EQUAL(node_id, expected_node_id);
      expected_node_id++;
   }


   /* negative tests */
   for (uint32_t frame_number = 0; frame_number < max_short_frames; frame_number++)
   {
      slot_number = frame_number * SLOTS_PER_SHORT_FRAME;
      for (uint32_t short_slot = 0; short_slot < 16; short_slot++)
      {
         node_id = MC_SMGR_CalculateNodeID(slot_number);
         CU_ASSERT_EQUAL(MC_SMGR_BAD_NODE_ID, node_id);
         slot_number++;
      }
   }

   /* out of range test */
   slot_number = SLOTS_PER_SUPER_FRAME;
   node_id = MC_SMGR_CalculateNodeID(slot_number);
   CU_ASSERT_EQUAL(MC_SMGR_BAD_NODE_ID, node_id);
}

/*************************************************************************************/
/**
* MC_SMGR_SetParentNodesTest
* Test the operation of function MC_SMGR_SetParentNodes(primary, secondary);
*
* no params
*
* @return - none

*/
void MC_SMGR_SetParentNodesTest()
{
   const uint16_t primary_parent_id = 0;
   const uint32_t secondary_parent_id = 1;
   const uint16_t primary_slot = 16;
   const uint16_t secondary_slot = 17;
   gNetworkAddress = 3;

   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));

   /* clear out any parent info */
   MC_SMGR_Initialise(false, gNetworkAddress);

   /* bad parameter check */
   bool result = MC_SMGR_SetParentNodes(0, 0);
   CU_ASSERT_FALSE(result);


   /* Set new primary parent */
   result = MC_SMGR_SetParentNodes(primary_slot, 0);
   CU_ASSERT_TRUE(result);

   /* Check the parents are as expected */
   CU_ASSERT_EQUAL(NODE_ID(gParent[PRIMARY_PARENT].NodeInfo), primary_parent_id);
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].IsActive, false);
   CU_ASSERT_EQUAL(GET_MISSED_HEARTBEATS(gParent[PRIMARY_PARENT].NodeInfo), 0);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, NOT_ASSIGNED);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].IsActive, false);

   /* clear out any parent info */
   MC_SMGR_Initialise(false, gNetworkAddress);

   /* Set new secondary parent */
   result = MC_SMGR_SetParentNodes(0, secondary_slot);
   CU_ASSERT_TRUE(result);

   /* Check the parents are as expected */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, NOT_ASSIGNED);
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].IsActive, false);
   CU_ASSERT_EQUAL(NODE_ID(gParent[SECONDARY_PARENT].NodeInfo), secondary_parent_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].IsActive, false);
   
   /* clear out any parent info */
   MC_SMGR_Initialise(false, gNetworkAddress);

   /* Set new primary and secondary parent */
   result = MC_SMGR_SetParentNodes(primary_slot, secondary_slot);
   CU_ASSERT_TRUE(result);

   /* Check the parents are as expected */
   CU_ASSERT_EQUAL(NODE_ID(gParent[PRIMARY_PARENT].NodeInfo), primary_parent_id);
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].IsActive, false);
   CU_ASSERT_EQUAL(GET_MISSED_HEARTBEATS(gParent[PRIMARY_PARENT].NodeInfo), 0);
   CU_ASSERT_EQUAL(NODE_ID(gParent[SECONDARY_PARENT].NodeInfo), secondary_parent_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].IsActive, false);
   CU_ASSERT_EQUAL(GET_MISSED_HEARTBEATS(gParent[SECONDARY_PARENT].NodeInfo), 0);

}

/*************************************************************************************/
/**
* MC_SMGR_IsParentNodeTest
* Test the operation of function MC_SMGR_IsParentNode(parent_id);
*
* no params
*
* @return - none

*/
void MC_SMGR_IsParentNodeTest(void)
{
   const uint16_t primary_parent_id = 0;
   const uint32_t secondary_parent_id = 1;
   const uint16_t primary_slot = 16;
   const uint16_t secondary_slot = 17;
   gNetworkAddress = 3;
   /* clear out any parent info */
   MC_SMGR_Initialise(false, gNetworkAddress);

   /* add primary and secondary parents */
   bool result = MC_SMGR_SetParentNodes(primary_slot, secondary_slot);
   CU_ASSERT_TRUE(result);

   /* positive checks */
   bool is_parent = MC_SMGR_IsParentNode(primary_parent_id);
   CU_ASSERT_TRUE(is_parent);
   is_parent = MC_SMGR_IsParentNode(secondary_parent_id);
   CU_ASSERT_TRUE(is_parent);

   /* negative tests */
   is_parent = MC_SMGR_IsParentNode(3);
   CU_ASSERT_FALSE(is_parent);
   is_parent = MC_SMGR_IsParentNode(0xfff);
   CU_ASSERT_FALSE(is_parent);

}


/*************************************************************************************/
/**
* MC_SMGR_GetNextParentForUplinkTest
* Test the operation of function MC_SMGR_GetNextParentForUplink();
*
* no params
*
* @return - none

*/
void MC_SMGR_GetNextParentForUplinkTest(void)
{
   const uint16_t primary_parent_id = 0;
   const uint32_t secondary_parent_id = 1;
   const uint16_t primary_slot = 16;
   const uint16_t secondary_slot = 17;
   gNetworkAddress = 3;

   /* clear out any parent info */
   MC_SMGR_Initialise(false, gNetworkAddress);

   /* check with no parents */
   uint16_t parent_id = MC_SMGR_GetNextParentForUplink();
   CU_ASSERT_EQUAL(parent_id, MC_SMGR_BAD_NODE_ID);
   /* the function alternates between parents, so call a second time */
   parent_id = MC_SMGR_GetNextParentForUplink();
   CU_ASSERT_EQUAL(parent_id, MC_SMGR_BAD_NODE_ID);

   /* add primary parent only */
   /* We should still get MC_SMGR_BAD_NODE_ID until the parent is active.
      Note: parents are set to active after the Route Add message has been acknowledged. */
   bool result = MC_SMGR_SetParentNodes(primary_slot, 0);
   CU_ASSERT_TRUE(result);
   parent_id = MC_SMGR_GetNextParentForUplink();
   CU_ASSERT_EQUAL(parent_id, MC_SMGR_BAD_NODE_ID);
   /*activate the parent*/
   result = MC_SMGR_RouteAddResponse(primary_parent_id, true);
   CU_ASSERT_TRUE(result);
   /* call MC_SMGR_GetNextParentForUplink 3 times. we should always get the primary parent */
   parent_id = MC_SMGR_GetNextParentForUplink();
   CU_ASSERT_EQUAL(parent_id, primary_parent_id);
   parent_id = MC_SMGR_GetNextParentForUplink();
   CU_ASSERT_EQUAL(parent_id, primary_parent_id);
   parent_id = MC_SMGR_GetNextParentForUplink();
   CU_ASSERT_EQUAL(parent_id, primary_parent_id);

   /* clear out any parent info */
   MC_SMGR_Initialise(false, gNetworkAddress);

   /* add secondary parent only */
   result = MC_SMGR_SetParentNodes(0, secondary_slot);
   CU_ASSERT_TRUE(result);
   parent_id = MC_SMGR_GetNextParentForUplink();
   CU_ASSERT_EQUAL(parent_id, MC_SMGR_BAD_NODE_ID);
   /*activate the parent*/
   result = MC_SMGR_RouteAddResponse(secondary_parent_id, true);
   CU_ASSERT_TRUE(result);
   /* call MC_SMGR_GetNextParentForUplink 3 times. we should always get the secondary parent */
   parent_id = MC_SMGR_GetNextParentForUplink();
   CU_ASSERT_EQUAL(parent_id, secondary_parent_id);
   parent_id = MC_SMGR_GetNextParentForUplink();
   CU_ASSERT_EQUAL(parent_id, secondary_parent_id);
   parent_id = MC_SMGR_GetNextParentForUplink();
   CU_ASSERT_EQUAL(parent_id, secondary_parent_id);

   /* clear out any parent info */
   MC_SMGR_Initialise(false, gNetworkAddress);

   /* add primary and secondary parents */
   result = MC_SMGR_SetParentNodes(primary_slot, secondary_slot);
   CU_ASSERT_TRUE(result);
   parent_id = MC_SMGR_GetNextParentForUplink();
   CU_ASSERT_EQUAL(parent_id, MC_SMGR_BAD_NODE_ID);
   parent_id = MC_SMGR_GetNextParentForUplink();
   CU_ASSERT_EQUAL(parent_id, MC_SMGR_BAD_NODE_ID);
   /*activate the parents*/
   result = MC_SMGR_RouteAddResponse(primary_parent_id, true);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(secondary_parent_id, true);
   CU_ASSERT_TRUE(result);
   /* call MC_SMGR_GetNextParentForUplink and ensure the parents are returned alternately */
   parent_id = MC_SMGR_GetNextParentForUplink();
   CU_ASSERT_EQUAL(parent_id, primary_parent_id);
   parent_id = MC_SMGR_GetNextParentForUplink();
   CU_ASSERT_EQUAL(parent_id, secondary_parent_id);
   parent_id = MC_SMGR_GetNextParentForUplink();
   CU_ASSERT_EQUAL(parent_id, primary_parent_id);
   parent_id = MC_SMGR_GetNextParentForUplink();
   CU_ASSERT_EQUAL(parent_id, secondary_parent_id);
}

/*************************************************************************************/
/**
* MC_SMGR_ConnectToParentTest
* Test the operation of function MC_SMGR_ConnectToParent();
*
* no params
*
* @return - none

*/
void MC_SMGR_ConnectToParentTest(void)
{
   uint16_t parent_id = 0;
   gNetworkAddress = 1;

   MeshQ = osMessageCreate(osMessageQ(MeshQ), NULL);
   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));

   MC_SMGR_Initialise(false, gNetworkAddress);
   osMessageQFlush(MeshQ);

   // Connecting to parent should issue a route add request to the MeshQ
   bool result = MC_SMGR_ConnectToParent(parent_id,false);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(1, osMessageCount(MeshQ));

}

/*************************************************************************************/
/**
* MC_SMGR_RouteAddResponseTest
* Test the operation of function MC_SMGR_RouteAddResponse();
*
* no params
*
* @return - none

*/
void MC_SMGR_RouteAddResponseTest()
{
   const uint16_t primary_parent_id = 0;
   const uint32_t secondary_parent_id = 1;
   const uint16_t primary_slot = 16;
   const uint16_t secondary_slot = 17;
   gNetworkAddress = 3;

   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));

   /* clear out any parent info */
   MC_SMGR_Initialise(false, gNetworkAddress);

   /* Set new primary and secondary parent */
   bool result = MC_SMGR_SetParentNodes(primary_slot, secondary_slot);
   CU_ASSERT_TRUE(result);

   /* Check the parents are as expected */
   CU_ASSERT_EQUAL(NODE_ID(gParent[PRIMARY_PARENT].NodeInfo), primary_parent_id);
   CU_ASSERT_FALSE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(GET_MISSED_HEARTBEATS(gParent[PRIMARY_PARENT].NodeInfo), 0);
   CU_ASSERT_EQUAL(NODE_ID(gParent[SECONDARY_PARENT].NodeInfo), secondary_parent_id);
   CU_ASSERT_FALSE(gParent[SECONDARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(GET_MISSED_HEARTBEATS(gParent[SECONDARY_PARENT].NodeInfo), 0);

   result = MC_SMGR_RouteAddResponse(primary_parent_id, true);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_TRUE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_FALSE(gParent[SECONDARY_PARENT].IsActive);

   result = MC_SMGR_RouteAddResponse(secondary_parent_id, true);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_TRUE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_TRUE(gParent[SECONDARY_PARENT].IsActive);

   result = MC_SMGR_RouteAddResponse(secondary_parent_id, false);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_TRUE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_FALSE(gParent[SECONDARY_PARENT].IsActive);

   result = MC_SMGR_RouteAddResponse(primary_parent_id, false);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_FALSE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_FALSE(gParent[SECONDARY_PARENT].IsActive);

}

/*************************************************************************************/
/**
* MC_SMGR_RouteAddResponseTimeoutTest
* Test the operation of the timeout for RouteAddResponse
*
* no params
*
* @return - none

*/
void MC_SMGR_RouteAddResponseTimeoutTest()
{
   const uint16_t primary_parent_id = 0;
   const uint32_t secondary_parent_id = 1;
   const uint16_t primary_slot = 16;
   const uint16_t secondary_slot = 17;
   CO_Message_t msgRouteAdd;

   gNetworkAddress = 3;

 
   MeshQ = osMessageCreate(osMessageQ(MeshQ), NULL);
   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));
   osMessageQFlush(MeshQ);
   osPoolFlush(MeshPool);

   /* clear out any parent info */
   MC_SMGR_Initialise(false, gNetworkAddress);

   /*Initialise the SACH Manager*/
   MC_SACH_Initialise(gNetworkAddress, gNetworkAddress);
   /*add a message to the SACH Manager*/
   MC_SACH_ScheduleMessage(secondary_parent_id, SACH_ZONE_E, APP_MSG_TYPE_ROUTE_ADD_E, &msgRouteAdd);
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 1);

   /* Set new primary and secondary parent */
   bool result = MC_SMGR_SetParentNodes(primary_slot, secondary_slot);
   CU_ASSERT_TRUE(result);

   /* Check the parents are as expected */
   CU_ASSERT_EQUAL(NODE_ID(gParent[PRIMARY_PARENT].NodeInfo), primary_parent_id);
   CU_ASSERT_FALSE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(GET_MISSED_HEARTBEATS(gParent[PRIMARY_PARENT].NodeInfo), 0);
   CU_ASSERT_EQUAL(NODE_ID(gParent[SECONDARY_PARENT].NodeInfo), secondary_parent_id);
   CU_ASSERT_FALSE(gParent[SECONDARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(GET_MISSED_HEARTBEATS(gParent[SECONDARY_PARENT].NodeInfo), 0);

   // Set up the timeout parameters on the primary parent
   gParent[PRIMARY_PARENT].WaitingForRouteAddResponse = true;
   gParent[PRIMARY_PARENT].ShortFramesSinceRouteAdd = 0;
   gParent[PRIMARY_PARENT].RouteAddRetries = 0;

   osMessageQFlush(MeshQ);
   osPoolFlush(MeshPool);
   /* The timeout is measured in short frames by counting RACHS slots.  Start supplying RACHS notifications*/
   uint32_t atempts = 0;
   uint32_t rach_count = 0;
   uint32_t message_count = osMessageCount(MeshQ);
   while (atempts < (SMGR_ROUTE_ADD_MAX_ATTEMPTS-1))
   {
      atempts++;
      /* Send RACHS updates until we are one short of the timeout*/
      while(rach_count < (SMGR_ROUTE_ADD_RESPONSE_TIMEOUT-1) )
      {
         rach_count++;
         MC_SMGR_UpdateRachCount(SECONDARY_RACH);
         CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].ShortFramesSinceRouteAdd, rach_count);
      }
      rach_count = 0;

      /* Send one more to trip the timeout */
      MC_SMGR_UpdateRachCount(SECONDARY_RACH);
      CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].ShortFramesSinceRouteAdd, 0);
      CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].RouteAddRetries, atempts);
      message_count = osMessageCount(MeshQ);
      CU_ASSERT_EQUAL(message_count, 1);
      osMessageQFlush(MeshQ);
      osPoolFlush(MeshPool);
   }

   /* Do one more retry and check that the primary parent is replaced by the secondary parent*/
   while (rach_count < (SMGR_ROUTE_ADD_RESPONSE_TIMEOUT - 1))
   {
      rach_count++;
      MC_SMGR_UpdateRachCount(SECONDARY_RACH);
      CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].ShortFramesSinceRouteAdd, rach_count);
   }

   /* Send one more to trip the timeout */
   MC_SMGR_UpdateRachCount(SECONDARY_RACH);
   /* The secondary parent should have been promoted*/ 
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, secondary_parent_id);
   /* check that the secondary parent's routeAdd msg was removed from the SACH when it was promoted*/
   CU_ASSERT_EQUAL(sasQueue[SACH_ZONE_E].MessageCount, 0);
}

/*************************************************************************************/
/**
* MC_SMGR_RouteAddRequestTest
* Test the operation of function MC_SMGR_RouteAddRequest();
*
* no params
*
* @return - none

*/
void MC_SMGR_RouteAddRequestTest()
{
   uint16_t primary_parent_slot = 16;
   uint16_t secondary_parent_slot = 17;
   gNetworkAddress = 3;

   /* clear out any old info */
   MC_SMGR_Initialise(false, gNetworkAddress);

   /* check the initialised state */
   CU_ASSERT_EQUAL(gChildren[0].NodeInfo, NOT_ASSIGNED);

   /* make sure the call fails if there are no parents*/
   bool result = MC_SMGR_RouteAddRequest(1, 0);
   CU_ASSERT_FALSE(result);

   /* add parents */
   MC_SMGR_SetParentNodes(primary_parent_slot, secondary_parent_slot);

   /* add a node */
   result = MC_SMGR_RouteAddRequest(1, 0);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(gChildren[0].NodeInfo, 1);
   CU_ASSERT_EQUAL(MC_SMGR_GetNumberOfChildren(), 1);

   /* add a second node */
   result = MC_SMGR_RouteAddRequest(2, 0);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(gChildren[1].NodeInfo, 2);
   CU_ASSERT_EQUAL(MC_SMGR_GetNumberOfChildren(), 2);

   /* do a fill test */
   /* first clear out any old info */
   MC_SMGR_Initialise(false,  gNetworkAddress);
   MC_SMGR_SetParentNodes(primary_parent_slot, secondary_parent_slot);
   gParent[PRIMARY_PARENT].IsActive = true;

   for (uint32_t index = 0; index < MAX_NUMBER_OF_CHILDREN; index++)
   {
      result = MC_SMGR_RouteAddRequest(index, 0);
      CU_ASSERT_TRUE(result);
      CU_ASSERT_EQUAL(gChildren[index].NodeInfo, index);
      CU_ASSERT_EQUAL(MC_SMGR_GetNumberOfChildren(), (index+1));
   }

   /* Another attempt to add a child should fail */ 
   result = MC_SMGR_RouteAddRequest(MAX_NUMBER_OF_CHILDREN, 0);
   CU_ASSERT_FALSE(result);

   /* Add an out-of-range child id - should fail */ 
   result = MC_SMGR_RouteAddRequest(MAX_NUMBER_OF_SYSTEM_NODES, 0);
   CU_ASSERT_FALSE(result);

}

/*************************************************************************************/
/**
* MC_SMGR_RouteDropRequestTest
* Test the operation of function MC_SMGR_RouteDropRequest();
*
* no params
*
* @return - none

*/
void MC_SMGR_RouteDropRequestTest()
{
   bool result;
   uint16_t primary_parent_slot = 2596;
   uint16_t secondary_parent_slot = 2597;
   gNetworkAddress = 3;

   /*Initialise the session management then fill the child array */
   MC_SMGR_Initialise(false,  gNetworkAddress);
   MC_SMGR_SetParentNodes(primary_parent_slot, secondary_parent_slot);
   gParent[PRIMARY_PARENT].IsActive = true;
   for (uint32_t index = 0; index < MAX_NUMBER_OF_CHILDREN; index++)
   {
      result = MC_SMGR_RouteAddRequest(index, 0);
      CU_ASSERT_TRUE(result);
      CU_ASSERT_EQUAL(gChildren[index].NodeInfo, index);
      CU_ASSERT_EQUAL(MC_SMGR_GetNumberOfChildren(), (index + 1));
   }

   CU_ASSERT_EQUAL(MC_SMGR_GetNumberOfChildren(), MAX_NUMBER_OF_CHILDREN);

   /* remove the children */
   uint32_t index;
   for (uint32_t count = 0; count < MAX_NUMBER_OF_CHILDREN; count++)
   {
      index = (MAX_NUMBER_OF_CHILDREN - 1) - count;
      result = MC_SMGR_RouteDropRequest(index);
      CU_ASSERT_TRUE(result);
      CU_ASSERT_EQUAL(gChildren[index].NodeInfo, NOT_ASSIGNED);
      CU_ASSERT_EQUAL(MC_SMGR_GetNumberOfChildren(), index);
   }

   CU_ASSERT_EQUAL(MC_SMGR_GetNumberOfChildren(), 0);

}

/*************************************************************************************/
/**
* MC_SMGR_IsAChildTest
* Test the operation of function MC_SMGR_IsAChild();
*
* no params
*
* @return - none

*/
void MC_SMGR_IsAChildTest()
{
   uint16_t primary_parent_slot = 2596;
   uint16_t secondary_parent_slot = 2597;
   const uint16_t child1 = 23;
   const uint16_t child2 = 31;
   const uint16_t child3 = 479;
   const uint16_t child4 = 15;

   gNetworkAddress = 3;

   /* clear out any old info */
   MC_SMGR_Initialise(false,  gNetworkAddress);
   MC_SMGR_SetParentNodes(primary_parent_slot, secondary_parent_slot);
   gParent[PRIMARY_PARENT].IsActive = true;

   /* Add some nodes */
   bool result = MC_SMGR_RouteAddRequest(child1, 0);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddRequest(child2, 0);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddRequest(child3, 0);
   CU_ASSERT_TRUE(result);

   /* check that both are recognised */
   result = MC_SMGR_IsAChild(child1);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_IsAChild(child2);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_IsAChild(child3);
   CU_ASSERT_TRUE(result);

   /* negative test.  there should be no record of child4 */
   result = MC_SMGR_IsAChild(child4);
   CU_ASSERT_FALSE(result);
}


/*************************************************************************************/
/**
* MC_SMGR_SetHeartbeatMonitorTest
* Test the operation of function MC_SMGR_SetHeartbeatMonitor_SlotNo();
*
* no params
*
* @return - none

*/
void MC_SMGR_SetHeartbeatMonitorTest(void)
{
   gNetworkAddress = 0;
   MC_MAC_SlotBehaviour_t slot_behaviour;
   MC_MAC_TDMIndex_t TDMIndex;
   TDMIndex.slotInSuperframeIdx = 16;

   // Replicate the behaviour of the function MC_MAC_CalcTDMIndices to update important fields within TDMIndex
   TDMIndex.slotIndex = SLOT_IDX(TDMIndex.slotInSuperframeIdx);
   TDMIndex.shortFrameIndex = SHORT_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
   TDMIndex.longFrameIndex = LONG_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
   TDMIndex.slotIndexInLongFrame = LONG_FRAME_SLOT_IDX(TDMIndex.slotInSuperframeIdx);

   /* clear out any old info */
   MC_SMGR_Initialise(false,  gNetworkAddress);
   /*check that the DCH slot for gNetworkAddress is enabled */
   slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
   CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_DCH_TX_E, slot_behaviour);

   /* set node 0 */
   bool result = MC_SMGR_SetHeartbeatMonitor_SlotNo(16, true);
   CU_ASSERT_TRUE(result);
   slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
   CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_DCH_TX_E, slot_behaviour);

   /* set node 1 */
   result = MC_SMGR_SetHeartbeatMonitor_SlotNo(17, true);
   CU_ASSERT_TRUE(result);
   TDMIndex.slotInSuperframeIdx = 17;
   // Replicate the behaviour of the function MC_MAC_CalcTDMIndices to update important fields within TDMIndex
   {
      TDMIndex.slotIndex = SLOT_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.shortFrameIndex = SHORT_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.longFrameIndex = LONG_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.slotIndexInLongFrame = LONG_FRAME_SLOT_IDX(TDMIndex.slotInSuperframeIdx);
   }
   slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
   CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_DCH_RX_E, slot_behaviour);

   /* reset node 0.  This is the local node address and should always be MC_MAC_BEHAVIOUR_DCH_TX_E */
   result = MC_SMGR_SetHeartbeatMonitor_SlotNo(16, false);
   CU_ASSERT_TRUE(result);
   TDMIndex.slotInSuperframeIdx = 16;
   // Replicate the behaviour of the function MC_MAC_CalcTDMIndices to update important fields within TDMIndex
   {
      TDMIndex.slotIndex = SLOT_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.shortFrameIndex = SHORT_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.longFrameIndex = LONG_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.slotIndexInLongFrame = LONG_FRAME_SLOT_IDX(TDMIndex.slotInSuperframeIdx);
   }
   slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
   CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_DCH_TX_E, slot_behaviour);

   /* reset node 1 */
   result = MC_SMGR_SetHeartbeatMonitor_SlotNo(17, false);
   CU_ASSERT_TRUE(result);
   TDMIndex.slotInSuperframeIdx = 17;
   // Replicate the behaviour of the function MC_MAC_CalcTDMIndices to update important fields within TDMIndex
   {
      TDMIndex.slotIndex = SLOT_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.shortFrameIndex = SHORT_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.longFrameIndex = LONG_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.slotIndexInLongFrame = LONG_FRAME_SLOT_IDX(TDMIndex.slotInSuperframeIdx);
   }
   slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
   CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_NONE_E, slot_behaviour);

   /* check for tracking node.  Should always be MC_MAC_BEHAVIOUR_DCH_RX_TRACKING_E*/
   uint16_t trackedNodeSlot = 17;
   uint16_t trackedNode = MC_SMGR_CalculateNodeID(trackedNodeSlot);

   gaTrackingNodes.aTrackingNodeList[0].slotIdx = trackedNodeSlot;
   gaTrackingNodes.noOfTrackingNodes++;

   result = MC_SMGR_SetHeartbeatMonitor_SlotNo(trackedNodeSlot, true);
   CU_ASSERT_TRUE(result);
   TDMIndex.slotInSuperframeIdx = trackedNodeSlot;
   slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
   CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_DCH_RX_TRACKING_E, slot_behaviour);

   result = MC_SMGR_SetHeartbeatMonitor_SlotNo(trackedNodeSlot, false);
   CU_ASSERT_TRUE(result);
   TDMIndex.slotInSuperframeIdx = trackedNodeSlot;
   slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
   CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_DCH_RX_TRACKING_E, slot_behaviour);


   /* loop through all DCH slots checking set and reset */
   for (uint32_t long_frame = 0; long_frame < LONG_FRAMES_PER_SUPER_FRAME; long_frame++)
   {
      /* short frames per long frame */
      for (uint32_t short_frame = 0; short_frame < SHORT_FRAMES_PER_LONG_FRAME; short_frame++)
      {
         /* slots per short frame */
         for (uint32_t short_slot = 0; short_slot < DCH_SLOTS_PER_SHORT_FRAME; short_slot++)
         {
            uint32_t super_frame_slot = (long_frame * SLOTS_PER_LONG_FRAME) + (short_frame * SLOTS_PER_SHORT_FRAME) + (short_slot + 16);
            uint16_t node_id = MC_SMGR_CalculateNodeID(super_frame_slot);

            /* call the function to set the heartbeat monitor */
            result = MC_SMGR_SetHeartbeatMonitor_SlotNo(super_frame_slot, true);
            CU_ASSERT_TRUE(result);
            TDMIndex.slotInSuperframeIdx = super_frame_slot;

            // Replicate the behaviour of the function MC_MAC_CalcTDMIndices to update important fields within TDMIndex
            TDMIndex.slotIndex = SLOT_IDX(TDMIndex.slotInSuperframeIdx);
            TDMIndex.shortFrameIndex = SHORT_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
            TDMIndex.longFrameIndex = LONG_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
            TDMIndex.slotIndexInLongFrame = LONG_FRAME_SLOT_IDX(TDMIndex.slotInSuperframeIdx);
            
            slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
            if (node_id == gNetworkAddress)
            {
               CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_DCH_TX_E, slot_behaviour);
            }
            else if (node_id == trackedNode)
            {
               CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_DCH_RX_TRACKING_E, slot_behaviour);
            }
            else
            {
               CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_DCH_RX_E, slot_behaviour);
            }


            /* reset and test for bit clear */
            result = MC_SMGR_SetHeartbeatMonitor_SlotNo(super_frame_slot, false);
            CU_ASSERT_TRUE(result);
            slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
            if (node_id == gNetworkAddress)
            {
               CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_DCH_TX_E, slot_behaviour);
            }
            else if (node_id == trackedNode)
            {
               CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_DCH_RX_TRACKING_E, slot_behaviour);
            }
            else
            {
               CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_NONE_E, slot_behaviour);
            }

         }
      }
   }


   /* negative test - check that all non-DCH slots return false*/

   for (uint32_t slot = 0; slot < SLOTS_PER_SUPER_FRAME; slot++)
   {
      if (16 > (slot % SLOTS_PER_SHORT_FRAME))/* assumes DCH slots are slots 16,17,18,19 in short frame */
      {
         result = MC_SMGR_SetHeartbeatMonitor_SlotNo(slot, true);
         CU_ASSERT_FALSE(result);
      }
   }

   /* clean global change so that it doesn't affact later tests */
   gaTrackingNodes.aTrackingNodeList[0].slotIdx = 0;
   gaTrackingNodes.noOfTrackingNodes--;

}

/*************************************************************************************/
/**
* MC_SMGR_StartStopHeartbeatMonitorTest
* Test the operation of function MC_SMGR_StartHeartbeatMonitorForNode() 
* and MC_SMGR_StopHeartbeatMonitorForNode
*
* no params
*
* @return - none

*/
void MC_SMGR_StartStopHeartbeatMonitorTest(void)
{
   uint16_t node1 = 1;
   uint16_t node7 = 7;
   uint16_t node11 = 11;
   uint16_t node14 = 14;
   gNetworkAddress = 3;
   MC_MAC_TDMIndex_t TDMIndex;
   MC_MAC_SlotBehaviour_t slot_behaviour;
   /* clear out any old info */
   MC_SMGR_Initialise(false,  gNetworkAddress);

   /* establish parents on the Mesh */
   bool result = MC_SMGR_SetParentNodes(16, 37);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(0, true);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(5, true);
   CU_ASSERT_TRUE(result);


   /*start heartbeat monitoring*/
   result = MC_SMGR_StartHeartbeatMonitorForNode(node1);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_StartHeartbeatMonitorForNode(node7);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_StartHeartbeatMonitorForNode(node11);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_StartHeartbeatMonitorForNode(node14);
   CU_ASSERT_TRUE(result);

   /*check that heartbeats are expected*/
   TDMIndex.slotInSuperframeIdx = MC_SMGR_CalculateDCHSlot(node1);
   // Replicate the behaviour of the function MC_MAC_CalcTDMIndices to update important fields within TDMIndex
   {
      TDMIndex.slotIndex = SLOT_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.shortFrameIndex = SHORT_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.longFrameIndex = LONG_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.slotIndexInLongFrame = LONG_FRAME_SLOT_IDX(TDMIndex.slotInSuperframeIdx);
   }
   slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
   CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_DCH_RX_E, slot_behaviour);

   TDMIndex.slotInSuperframeIdx = MC_SMGR_CalculateDCHSlot(node7);
   // Replicate the behaviour of the function MC_MAC_CalcTDMIndices to update important fields within TDMIndex
   {
      TDMIndex.slotIndex = SLOT_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.shortFrameIndex = SHORT_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.longFrameIndex = LONG_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.slotIndexInLongFrame = LONG_FRAME_SLOT_IDX(TDMIndex.slotInSuperframeIdx);
   }
   slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
   CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_DCH_RX_E, slot_behaviour);

   TDMIndex.slotInSuperframeIdx = MC_SMGR_CalculateDCHSlot(node11);
   // Replicate the behaviour of the function MC_MAC_CalcTDMIndices to update important fields within TDMIndex
   {
      TDMIndex.slotIndex = SLOT_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.shortFrameIndex = SHORT_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.longFrameIndex = LONG_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.slotIndexInLongFrame = LONG_FRAME_SLOT_IDX(TDMIndex.slotInSuperframeIdx);
   }
   slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
   CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_DCH_RX_E, slot_behaviour);

   TDMIndex.slotInSuperframeIdx = MC_SMGR_CalculateDCHSlot(node14);
   // Replicate the behaviour of the function MC_MAC_CalcTDMIndices to update important fields within TDMIndex
   {
      TDMIndex.slotIndex = SLOT_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.shortFrameIndex = SHORT_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.longFrameIndex = LONG_FRAME_IDX(TDMIndex.slotInSuperframeIdx);
      TDMIndex.slotIndexInLongFrame = LONG_FRAME_SLOT_IDX(TDMIndex.slotInSuperframeIdx);
   }
   slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
   CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_DCH_RX_E, slot_behaviour);


   /*stop heartbeat monitoring*/
   result = MC_SMGR_StopHeartbeatMonitorForNode(node1);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_StopHeartbeatMonitorForNode(node7);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_StopHeartbeatMonitorForNode(node11);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_StopHeartbeatMonitorForNode(node14);
   CU_ASSERT_TRUE(result);

   /*check that heartbeats are no longer expected*/
   TDMIndex.slotInSuperframeIdx = MC_SMGR_CalculateDCHSlot(node1);
   slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
   CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_NONE_E, slot_behaviour);

   TDMIndex.slotInSuperframeIdx = MC_SMGR_CalculateDCHSlot(node7);
   slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
   CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_NONE_E, slot_behaviour);

   TDMIndex.slotInSuperframeIdx = MC_SMGR_CalculateDCHSlot(node11);
   slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
   CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_NONE_E, slot_behaviour);

   TDMIndex.slotInSuperframeIdx = MC_SMGR_CalculateDCHSlot(node14);
   slot_behaviour = MC_MAC_GetSlotBehaviour(&TDMIndex);
   CU_ASSERT_EQUAL(MC_MAC_BEHAVIOUR_NONE_E, slot_behaviour);

}

/*************************************************************************************/
/**
* MC_SMGR_MissedNodeHeartbeatTest
* Test the operation of function MC_SMGR_MissedNodeHeartbeat();
*
* no params
*
* @return - none

*/
void MC_SMGR_MissedNodeHeartbeatTest()
{
   const uint16_t primary_parent_id = 0;
   const uint16_t secondary_parent_id = 1;
   const uint16_t primary_tracking_node_id = 2;
   const uint16_t secondary_tracking_node_id = 3;
   const uint16_t primary_parent_slot = 16;
   const uint16_t secondary_parent_slot = 17;
   const uint16_t primary_tracking_node_slot = 18;
   const uint16_t secondary_tracking_node_slot = 19;
   const uint16_t child1 = 4;
   const uint16_t child2 = 7;
   const uint16_t child3 = 12;
   const uint16_t child4 = 15;
   gNetworkAddress = 3;
   bool response = false;

   gParent[PRIMARY_PARENT].NodeInfo = primary_parent_id;
   gParent[PRIMARY_PARENT].IsActive = true;
   gParent[SECONDARY_PARENT].NodeInfo = secondary_parent_id;
   gParent[SECONDARY_PARENT].IsActive = true;
   gParent[PRIMARY_TRACKING_NODE].NodeInfo = primary_tracking_node_id;
   gParent[SECONDARY_TRACKING_NODE].NodeInfo = secondary_tracking_node_id;
   gaTrackingNodes.aTrackingNodeList[PRIMARY_PARENT].slotIdx = primary_parent_slot;
   gaTrackingNodes.aTrackingNodeList[SECONDARY_PARENT].slotIdx = secondary_parent_slot;
   gaTrackingNodes.aTrackingNodeList[PRIMARY_TRACKING_NODE].slotIdx = primary_tracking_node_slot;
   gaTrackingNodes.aTrackingNodeList[SECONDARY_TRACKING_NODE].slotIdx = secondary_tracking_node_slot;
   gaTrackingNodes.noOfTrackingNodes = 4;

   RACHSQ = osMessageCreate(osMessageQ(RACHSQ), NULL);
   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));

   /* clear out any old info */
   MC_SMGR_Initialise(false,  gNetworkAddress);

   /* Set parents */
   bool result = MC_SMGR_SetParentNodes(primary_parent_slot, secondary_parent_slot);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(primary_parent_id, true);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(secondary_parent_id, true);
   CU_ASSERT_TRUE(result);

   /* Set one tracking node */
   result = MC_SMGR_SetTrackingNodes(PRIMARY_TRACKING_NODE_NOT_ASSIGNED, secondary_tracking_node_slot);
   CU_ASSERT_TRUE(result);

   /* Add some children */
   result = MC_SMGR_RouteAddRequest(child1, 0);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddRequest(child2, 0);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddRequest(child3, 0);
   CU_ASSERT_TRUE(result);

   /* check out of range handling */
   response = MC_SMGR_MissedNodeHeartbeat(DCH_SLOTS_PER_LONG_FRAME + 1);
   CU_ASSERT_FALSE(response);

   /* secondary tracking node missed heartbeats*/
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(secondary_tracking_node_id);
      CU_ASSERT_TRUE(response);
      uint16_t missed_heartbeats = GET_MISSED_HEARTBEATS(gParent[SECONDARY_TRACKING_NODE].NodeInfo);
      CU_ASSERT_EQUAL(missed_heartbeats, count);
   }
   /* one more missed heartbeat should cause the removal of the secondary tracking node */
   response = MC_SMGR_MissedNodeHeartbeat(secondary_tracking_node_id);
   CU_ASSERT_FALSE(response);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);
   CU_ASSERT_FALSE(gParent[SECONDARY_TRACKING_NODE].IsActive);


   /* secondary parent missed heartbeats*/
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(secondary_parent_id);
      CU_ASSERT_TRUE(response);
      uint16_t missed_heartbeats = GET_MISSED_HEARTBEATS(gParent[SECONDARY_PARENT].NodeInfo);
      CU_ASSERT_EQUAL(missed_heartbeats, count);
   }

   /* one more missed heartbeat should cause the removal of the secondary parent */
   response = MC_SMGR_MissedNodeHeartbeat(secondary_parent_id);
   CU_ASSERT_FALSE(response);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, NOT_ASSIGNED);
   CU_ASSERT_FALSE(gParent[SECONDARY_PARENT].IsActive);



   /* primary parent missed heartbeats*/
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(primary_parent_id);
      CU_ASSERT_TRUE(response);
      uint16_t missed_heartbeats = GET_MISSED_HEARTBEATS(gParent[PRIMARY_PARENT].NodeInfo);
      CU_ASSERT_EQUAL(missed_heartbeats, count);
   }

   /* one more missed heartbeat should cause the removal of the primary parent */
   response = MC_SMGR_MissedNodeHeartbeat(primary_parent_id);
   CU_ASSERT_FALSE(response);
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, NOT_ASSIGNED);
   CU_ASSERT_FALSE(gParent[PRIMARY_PARENT].IsActive);

   /* child1 */
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(child1);
      CU_ASSERT_TRUE(response);
      uint16_t missed_heartbeats = GET_MISSED_HEARTBEATS(gChildren[0].NodeInfo);
      CU_ASSERT_EQUAL(missed_heartbeats, count);
   }
   /* one more missed heartbeat should cause the removal of the child */
   response = MC_SMGR_MissedNodeHeartbeat(child1);
   CU_ASSERT_FALSE(response);
   CU_ASSERT_EQUAL(gChildren[0].NodeInfo, NOT_ASSIGNED);


   /* child2 */
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(child2);
      CU_ASSERT_TRUE(response);
      uint16_t missed_heartbeats = GET_MISSED_HEARTBEATS(gChildren[1].NodeInfo);
      CU_ASSERT_EQUAL(missed_heartbeats, count);
   }
   /* one more missed heartbeat should cause the removal of the child */
   response = MC_SMGR_MissedNodeHeartbeat(child2);
   CU_ASSERT_FALSE(response);
   CU_ASSERT_EQUAL(gChildren[1].NodeInfo, NOT_ASSIGNED);

   /* child3 */
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(child3);
      CU_ASSERT_TRUE(response);
      uint16_t missed_heartbeats = GET_MISSED_HEARTBEATS(gChildren[2].NodeInfo);
      CU_ASSERT_EQUAL(missed_heartbeats, count);
   }
   /* one more missed heartbeat should cause the removal of the child */
   response = MC_SMGR_MissedNodeHeartbeat(child3);
   CU_ASSERT_FALSE(response);
   CU_ASSERT_EQUAL(gChildren[2].NodeInfo, NOT_ASSIGNED);

   /* negative test.  unrecognised child */
   result = MC_SMGR_MissedNodeHeartbeat(child4);
   CU_ASSERT_FALSE(result);
}

/*************************************************************************************/
/**
* MC_SMGR_LosePrimaryParent
* Test the operation of function MC_SMGR_CheckAndFixParentConnections(). This handles
* loss of primary parent node by changing the role of lower priority nodes.
*
* no params
*
* @return - none

*/
void MC_SMGR_LosePrimaryParent()
{
   const uint16_t node1_id = 0;
   const uint16_t node2_id = 1;
   const uint16_t node3_id = 2;
   const uint16_t node4_id = 3;
   const uint16_t node1_slot = 16;
   const uint16_t node2_slot = 17;
   const uint16_t node3_slot = 18;
   const uint16_t node4_slot = 19;
   gNetworkAddress = 3;
   bool response = false;
   extern SM_NeighbourList_t gaMeshNeighbourList[];

   RACHSQ = osMessageCreate(osMessageQ(RACHSQ), NULL);
   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));

   /* clear out any old info */
   MC_SMGR_Initialise(false,  gNetworkAddress);

   /* Set parents */
   bool result = MC_SMGR_SetParentNodes(node1_slot, node2_slot);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(node1_id, true);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(node2_id, true);
   CU_ASSERT_TRUE(result);

   /* Set tracking nodes */
   result = MC_SMGR_SetTrackingNodes(node3_slot, node4_slot);
   CU_ASSERT_TRUE(result);

   /* check parent table */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, node1_id);
   CU_ASSERT_TRUE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, node2_id);
   CU_ASSERT_TRUE(gParent[SECONDARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, node3_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, node4_id);

   // Manually configure the tracking nodes in the sync module.
   gaTrackingNodes.noOfTrackingNodes = 4;
   gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase = true;
   gaTrackingNodes.aTrackingNodeList[0].assigned = true;
   gaTrackingNodes.aTrackingNodeList[0].slotIdx = node1_slot;
   gaTrackingNodes.aTrackingNodeList[0].node_id = node1_id;
   gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase = true;
   gaTrackingNodes.aTrackingNodeList[1].assigned = true;
   gaTrackingNodes.aTrackingNodeList[1].slotIdx = node2_slot;
   gaTrackingNodes.aTrackingNodeList[1].node_id = node2_id;
   gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase = true;
   gaTrackingNodes.aTrackingNodeList[2].assigned = true;
   gaTrackingNodes.aTrackingNodeList[2].slotIdx = node3_slot;
   gaTrackingNodes.aTrackingNodeList[2].node_id = node3_id;
   gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase = true;
   gaTrackingNodes.aTrackingNodeList[3].assigned = true;
   gaTrackingNodes.aTrackingNodeList[3].slotIdx = node4_slot;
   gaTrackingNodes.aTrackingNodeList[3].node_id = node4_id;

   // Manually configure the tracking nodes in the mesh form and heal module.
   gaMeshNeighbourList[0].Bitfield.InUse = true;
   gaMeshNeighbourList[1].Bitfield.InUse = true;
   gaMeshNeighbourList[2].Bitfield.InUse = true;
   gaMeshNeighbourList[3].Bitfield.InUse = true;

   // Check that the above settings have been correctly applied.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 4);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].node_id, node1_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].node_id, node2_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].node_id, node3_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].node_id, node4_id);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node1_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node2_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node3_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node4_id].Bitfield.InUse, true);

   /* node1 missed heartbeats*/
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(node1_id);
      CU_ASSERT_TRUE(response);
      uint16_t missed_heartbeats = GET_MISSED_HEARTBEATS(gParent[PRIMARY_PARENT].NodeInfo);
      CU_ASSERT_EQUAL(missed_heartbeats, count);
   }
   /* one more missed heartbeat should cause the removal of node1 and replace with node2 */
   response = MC_SMGR_MissedNodeHeartbeat(node1_id);
   CU_ASSERT_FALSE(response);

   // Check that the sync module and mesh form and heal module have both been updated correctly.
   // Tracking nodes have all been promoted to one rank higher.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 3);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].node_id, node2_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].node_id, node3_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].node_id, node4_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].node_id, DCH_SLOTS_PER_LONG_FRAME);
   // MeshNeighbour entry for node1_id now false.
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node1_id].Bitfield.InUse, false);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node2_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node3_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node4_id].Bitfield.InUse, true);

   /* node3 has be moved from a tracking node to a parent node - send route add response */
   result = MC_SMGR_RouteAddResponse(node3_id, true);
   CU_ASSERT_TRUE(result);

   /* check parent table */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, node2_id);
   CU_ASSERT_TRUE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, node3_id);
   CU_ASSERT_TRUE(gParent[SECONDARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, node4_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);

   /* node2 missed heartbeats*/
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(node2_id);
      CU_ASSERT_TRUE(response);
      uint16_t missed_heartbeats = GET_MISSED_HEARTBEATS(gParent[PRIMARY_PARENT].NodeInfo);
      CU_ASSERT_EQUAL(missed_heartbeats, count);
   }
   /* one more missed heartbeat should cause the removal of node2 and replace with node3 */
   response = MC_SMGR_MissedNodeHeartbeat(node2_id);
   CU_ASSERT_FALSE(response);

   // Check that the sync module and mesh form and heal module have both been updated correctly.
   // Tracking nodes have all been promoted to one rank higher.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 2);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].node_id, node3_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].node_id, node4_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].node_id, DCH_SLOTS_PER_LONG_FRAME);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].node_id, DCH_SLOTS_PER_LONG_FRAME);
   // MeshNeighbour entry for node2_id now false.
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node1_id].Bitfield.InUse, false);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node2_id].Bitfield.InUse, false);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node3_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node4_id].Bitfield.InUse, true);

   /* node4 has be moved from a tracking node to a parent node - send route add response */
   result = MC_SMGR_RouteAddResponse(node4_id, true);
   CU_ASSERT_TRUE(result);

   /* check parent table */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, node3_id);
   CU_ASSERT_TRUE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, node4_id);
   CU_ASSERT_TRUE(gParent[SECONDARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);

   /* node3 missed heartbeats*/
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(node3_id);
      CU_ASSERT_TRUE(response);
      uint16_t missed_heartbeats = GET_MISSED_HEARTBEATS(gParent[PRIMARY_PARENT].NodeInfo);
      CU_ASSERT_EQUAL(missed_heartbeats, count);
   }
   /* one more missed heartbeat should cause the removal of node3 and replace with node4 */
   response = MC_SMGR_MissedNodeHeartbeat(node3_id);
   CU_ASSERT_FALSE(response);

   // Check that the sync module and mesh form and heal module have both been updated correctly.
   // Tracking nodes have all been promoted to one rank higher.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 1);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].node_id, node4_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].node_id, DCH_SLOTS_PER_LONG_FRAME);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].node_id, DCH_SLOTS_PER_LONG_FRAME);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].node_id, DCH_SLOTS_PER_LONG_FRAME);
   // MeshNeighbour entry for node3_id now false.
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node1_id].Bitfield.InUse, false);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node2_id].Bitfield.InUse, false);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node3_id].Bitfield.InUse, false);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node4_id].Bitfield.InUse, true);

   /* check parent table */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, node4_id);
   CU_ASSERT_TRUE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, NOT_ASSIGNED);
   CU_ASSERT_FALSE(gParent[SECONDARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);

   /* node4 missed heartbeats*/
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(node4_id);
      CU_ASSERT_TRUE(response);
      uint16_t missed_heartbeats = GET_MISSED_HEARTBEATS(gParent[PRIMARY_PARENT].NodeInfo);
      CU_ASSERT_EQUAL(missed_heartbeats, count);
   }
   /* one more missed heartbeat should cause the removal of node4 - no replacement available */
   response = MC_SMGR_MissedNodeHeartbeat(node4_id);
   CU_ASSERT_FALSE(response);

   // Check that the sync module and mesh form and heal module have both been updated correctly.
   // Tracking nodes have all been promoted to one rank higher.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 0);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].node_id, DCH_SLOTS_PER_LONG_FRAME);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].node_id, DCH_SLOTS_PER_LONG_FRAME);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].node_id, DCH_SLOTS_PER_LONG_FRAME);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].node_id, DCH_SLOTS_PER_LONG_FRAME);
   // MeshNeighbour entry for node3_id now false.
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node1_id].Bitfield.InUse, false);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node2_id].Bitfield.InUse, false);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node3_id].Bitfield.InUse, false);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node4_id].Bitfield.InUse, false);

   /* check parent table */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, NOT_ASSIGNED);
   CU_ASSERT_FALSE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, NOT_ASSIGNED);
   CU_ASSERT_FALSE(gParent[SECONDARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);
}

/*************************************************************************************/
/**
* MC_SMGR_LoseSecondaryParent
* Test the operation of function MC_SMGR_CheckAndFixParentConnections(). This handles
* loss of secondary parent node by changing the role of lower priority nodes.
*
* no params
*
* @return - none

*/
void MC_SMGR_LoseSecondaryParent()
{
   const uint16_t node1_id = 0;
   const uint16_t node2_id = 1;
   const uint16_t node3_id = 2;
   const uint16_t node4_id = 3;
   const uint16_t node1_slot = 16;
   const uint16_t node2_slot = 17;
   const uint16_t node3_slot = 18;
   const uint16_t node4_slot = 19;
   gNetworkAddress = 3;
   bool response = false;
   extern SM_NeighbourList_t gaMeshNeighbourList[];

   RACHSQ = osMessageCreate(osMessageQ(RACHSQ), NULL);
   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));

   /* clear out any old info */
   MC_SMGR_Initialise(false,  gNetworkAddress);

   /* Set parents */
   bool result = MC_SMGR_SetParentNodes(node1_slot, node2_slot);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(node1_id, true);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(node2_id, true);
   CU_ASSERT_TRUE(result);

   /* Set tracking nodes */
   result = MC_SMGR_SetTrackingNodes(node3_slot, node4_slot);
   CU_ASSERT_TRUE(result);

   /* check parent table */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, node1_id);
   CU_ASSERT_TRUE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, node2_id);
   CU_ASSERT_TRUE(gParent[SECONDARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, node3_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, node4_id);

   // Manually configure the tracking nodes in the sync module.
   gaTrackingNodes.noOfTrackingNodes = 4;
   gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase = true;
   gaTrackingNodes.aTrackingNodeList[0].assigned = true;
   gaTrackingNodes.aTrackingNodeList[0].slotIdx = node1_slot;
   gaTrackingNodes.aTrackingNodeList[0].node_id = node1_id;
   gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase = true;
   gaTrackingNodes.aTrackingNodeList[1].assigned = true;
   gaTrackingNodes.aTrackingNodeList[1].slotIdx = node2_slot;
   gaTrackingNodes.aTrackingNodeList[1].node_id = node2_id;
   gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase = true;
   gaTrackingNodes.aTrackingNodeList[2].assigned = true;
   gaTrackingNodes.aTrackingNodeList[2].slotIdx = node3_slot;
   gaTrackingNodes.aTrackingNodeList[2].node_id = node3_id;
   gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase = true;
   gaTrackingNodes.aTrackingNodeList[3].assigned = true;
   gaTrackingNodes.aTrackingNodeList[3].slotIdx = node4_slot;
   gaTrackingNodes.aTrackingNodeList[3].node_id = node4_id;

   // Manually configure the tracking nodes in the mesh form and heal module.
   gaMeshNeighbourList[0].Bitfield.InUse = true;
   gaMeshNeighbourList[1].Bitfield.InUse = true;
   gaMeshNeighbourList[2].Bitfield.InUse = true;
   gaMeshNeighbourList[3].Bitfield.InUse = true;

   // Check that the above settings have been correctly applied.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 4);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].node_id, node1_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].node_id, node2_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].node_id, node3_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].node_id, node4_id);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node1_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node2_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node3_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node4_id].Bitfield.InUse, true);

   /* node2 missed heartbeats*/
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(node2_id);
      CU_ASSERT_TRUE(response);
      uint16_t missed_heartbeats = GET_MISSED_HEARTBEATS(gParent[SECONDARY_PARENT].NodeInfo);
      CU_ASSERT_EQUAL(missed_heartbeats, count);
   }
   /* one more missed heartbeat should cause the removal of node2 and replace with node3 */
   response = MC_SMGR_MissedNodeHeartbeat(node2_id);
   CU_ASSERT_FALSE(response);

   // Check that the sync module and mesh form and heal module have both been updated correctly.
   // Tracking nodes have all been promoted to one rank higher.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 3);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].node_id, node1_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].node_id, node3_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].node_id, node4_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].node_id, DCH_SLOTS_PER_LONG_FRAME);
   // MeshNeighbour entry for node1_id now false.
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node1_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node2_id].Bitfield.InUse, false);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node3_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node4_id].Bitfield.InUse, true);

   /* node3 has be moved from a tracking node to a parent node - send route add response */
   result = MC_SMGR_RouteAddResponse(node3_id, true);
   CU_ASSERT_TRUE(result);

   /* check parent table */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, node1_id);
   CU_ASSERT_TRUE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, node3_id);
   CU_ASSERT_TRUE(gParent[SECONDARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, node4_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);

   /* node3 missed heartbeats*/
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(node3_id);
      CU_ASSERT_TRUE(response);
      uint16_t missed_heartbeats = GET_MISSED_HEARTBEATS(gParent[SECONDARY_PARENT].NodeInfo);
      CU_ASSERT_EQUAL(missed_heartbeats, count);
   }
   /* one more missed heartbeat should cause the removal of node3 and replace with node4 */
   response = MC_SMGR_MissedNodeHeartbeat(node3_id);
   CU_ASSERT_FALSE(response);

   // Check that the sync module and mesh form and heal module have both been updated correctly.
   // Tracking nodes have all been promoted to one rank higher.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 2);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].node_id, node1_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].node_id, node4_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].node_id, DCH_SLOTS_PER_LONG_FRAME);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].node_id, DCH_SLOTS_PER_LONG_FRAME);
   // MeshNeighbour entry for node2_id now false.
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node1_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node2_id].Bitfield.InUse, false);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node3_id].Bitfield.InUse, false);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node4_id].Bitfield.InUse, true);

   /* node4 has be moved from a tracking node to a parent node - send route add response */
   result = MC_SMGR_RouteAddResponse(node4_id, true);
   CU_ASSERT_TRUE(result);

   /* check parent table */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, node1_id);
   CU_ASSERT_TRUE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, node4_id);
   CU_ASSERT_TRUE(gParent[SECONDARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);

   /* node4 missed heartbeats*/
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(node4_id);
      CU_ASSERT_TRUE(response);
      uint16_t missed_heartbeats = GET_MISSED_HEARTBEATS(gParent[SECONDARY_PARENT].NodeInfo);
      CU_ASSERT_EQUAL(missed_heartbeats, count);
   }
   /* one more missed heartbeat should cause the removal of node4 */
   response = MC_SMGR_MissedNodeHeartbeat(node4_id);
   CU_ASSERT_FALSE(response);

   // Check that the sync module and mesh form and heal module have both been updated correctly.
   // Tracking nodes have all been promoted to one rank higher.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 1);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].node_id, node1_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].node_id, DCH_SLOTS_PER_LONG_FRAME);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].node_id, DCH_SLOTS_PER_LONG_FRAME);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].node_id, DCH_SLOTS_PER_LONG_FRAME);
   // MeshNeighbour entry for node3_id now false.
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node1_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node2_id].Bitfield.InUse, false);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node3_id].Bitfield.InUse, false);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node4_id].Bitfield.InUse, false);

   /* check parent table */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, node1_id);
   CU_ASSERT_TRUE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, NOT_ASSIGNED);
   CU_ASSERT_FALSE(gParent[SECONDARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);

}

/*************************************************************************************/
/**
* MC_SMGR_LosePrimaryTrackingNode
* Test the operation of function MC_SMGR_CheckAndFixParentConnections(). This handles
* loss of primary tracking node by changing the role of lower priority nodes.
*
* no params
*
* @return - none

*/
void MC_SMGR_LosePrimaryTrackingNode()
{
   const uint16_t node1_id = 0;
   const uint16_t node2_id = 1;
   const uint16_t node3_id = 2;
   const uint16_t node4_id = 3;
   const uint16_t node1_slot = 16;
   const uint16_t node2_slot = 17;
   const uint16_t node3_slot = 18;
   const uint16_t node4_slot = 19;
   gNetworkAddress = 3;
   bool response = false;
   extern SM_NeighbourList_t gaMeshNeighbourList[];

   RACHSQ = osMessageCreate(osMessageQ(RACHSQ), NULL);
   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));

   /* clear out any old info */
   MC_SMGR_Initialise(false,  gNetworkAddress);

   /* Set parents */
   bool result = MC_SMGR_SetParentNodes(node1_slot, node2_slot);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(node1_id, true);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(node2_id, true);
   CU_ASSERT_TRUE(result);

   /* Set tracking nodes */
   result = MC_SMGR_SetTrackingNodes(node3_slot, node4_slot);
   CU_ASSERT_TRUE(result);

   /* check parent table */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, node1_id);
   CU_ASSERT_TRUE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, node2_id);
   CU_ASSERT_TRUE(gParent[SECONDARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, node3_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, node4_id);

   // Manually configure the tracking nodes in the sync module.
   gaTrackingNodes.noOfTrackingNodes = 4;
   gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase = true;
   gaTrackingNodes.aTrackingNodeList[0].assigned = true;
   gaTrackingNodes.aTrackingNodeList[0].slotIdx = node1_slot;
   gaTrackingNodes.aTrackingNodeList[0].node_id = node1_id;
   gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase = true;
   gaTrackingNodes.aTrackingNodeList[1].assigned = true;
   gaTrackingNodes.aTrackingNodeList[1].slotIdx = node2_slot;
   gaTrackingNodes.aTrackingNodeList[1].node_id = node2_id;
   gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase = true;
   gaTrackingNodes.aTrackingNodeList[2].assigned = true;
   gaTrackingNodes.aTrackingNodeList[2].slotIdx = node3_slot;
   gaTrackingNodes.aTrackingNodeList[2].node_id = node3_id;
   gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase = true;
   gaTrackingNodes.aTrackingNodeList[3].assigned = true;
   gaTrackingNodes.aTrackingNodeList[3].slotIdx = node4_slot;
   gaTrackingNodes.aTrackingNodeList[3].node_id = node4_id;

   // Manually configure the tracking nodes in the mesh form and heal module.
   gaMeshNeighbourList[0].Bitfield.InUse = true;
   gaMeshNeighbourList[1].Bitfield.InUse = true;
   gaMeshNeighbourList[2].Bitfield.InUse = true;
   gaMeshNeighbourList[3].Bitfield.InUse = true;

   // Check that the above settings have been correctly applied.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 4);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].node_id, node1_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].node_id, node2_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].node_id, node3_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].node_id, node4_id);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node1_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node2_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node3_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node4_id].Bitfield.InUse, true);

   /* node3 missed heartbeats*/
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(node3_id);
      CU_ASSERT_TRUE(response);
      uint16_t missed_heartbeats = GET_MISSED_HEARTBEATS(gParent[PRIMARY_TRACKING_NODE].NodeInfo);
      CU_ASSERT_EQUAL(missed_heartbeats, count);
   }
   /* one more missed heartbeat should cause the removal of node3 and replace with node4 */
   response = MC_SMGR_MissedNodeHeartbeat(node3_id);
   CU_ASSERT_FALSE(response);

   // Check that the sync module and mesh form and heal module have both been updated correctly.
   // Tracking nodes have all been promoted to one rank higher.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 3);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].node_id, node1_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].node_id, node2_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].node_id, node4_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].node_id, DCH_SLOTS_PER_LONG_FRAME);
   // MeshNeighbour entry for node1_id now false.
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node1_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node2_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node3_id].Bitfield.InUse, false);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node4_id].Bitfield.InUse, true);

   /* check parent table */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, node1_id);
   CU_ASSERT_TRUE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, node2_id);
   CU_ASSERT_TRUE(gParent[SECONDARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, node4_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);

   /* node4 missed heartbeats*/
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(node4_id);
      CU_ASSERT_TRUE(response);
      uint16_t missed_heartbeats = GET_MISSED_HEARTBEATS(gParent[PRIMARY_TRACKING_NODE].NodeInfo);
      CU_ASSERT_EQUAL(missed_heartbeats, count);
   }
   /* one more missed heartbeat should cause the removal of node4 and replace with node3 */
   response = MC_SMGR_MissedNodeHeartbeat(node4_id);
   CU_ASSERT_FALSE(response);

   // Check that the sync module and mesh form and heal module have both been updated correctly.
   // Tracking nodes have all been promoted to one rank higher.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 2);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].node_id, node1_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].node_id, node2_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].node_id, DCH_SLOTS_PER_LONG_FRAME);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].node_id, DCH_SLOTS_PER_LONG_FRAME);
   // MeshNeighbour entry for node2_id now false.
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node1_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node2_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node3_id].Bitfield.InUse, false);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node4_id].Bitfield.InUse, false);

   /* check parent table */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, node1_id);
   CU_ASSERT_TRUE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, node2_id);
   CU_ASSERT_TRUE(gParent[SECONDARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);
}

/*************************************************************************************/
/**
* MC_SMGR_LoseSecondaryTrackingNode
* Test the operation of function MC_SMGR_CheckAndFixParentConnections(). This handles
* loss of secondary tracking node by changing the role of lower priority nodes.
*
* no params
*
* @return - none

*/
void MC_SMGR_LoseSecondaryTrackingNode()
{
   const uint16_t node1_id = 0;
   const uint16_t node2_id = 1;
   const uint16_t node3_id = 2;
   const uint16_t node4_id = 3;
   const uint16_t node1_slot = 16;
   const uint16_t node2_slot = 17;
   const uint16_t node3_slot = 18;
   const uint16_t node4_slot = 19;
   gNetworkAddress = 3;
   bool response = false;
   extern SM_NeighbourList_t gaMeshNeighbourList[];

   RACHSQ = osMessageCreate(osMessageQ(RACHSQ), NULL);
   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));

   /* clear out any old info */
   MC_SMGR_Initialise(false,  gNetworkAddress);

   /* Set parents */
   bool result = MC_SMGR_SetParentNodes(node1_slot, node2_slot);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(node1_id, true);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(node2_id, true);
   CU_ASSERT_TRUE(result);

   /* Set tracking nodes */
   result = MC_SMGR_SetTrackingNodes(node3_slot, node4_slot);
   CU_ASSERT_TRUE(result);

   /* check parent table */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, node1_id);
   CU_ASSERT_TRUE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, node2_id);
   CU_ASSERT_TRUE(gParent[SECONDARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, node3_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, node4_id);

   // Manually configure the tracking nodes in the sync module.
   gaTrackingNodes.noOfTrackingNodes = 4;
   gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase = true;
   gaTrackingNodes.aTrackingNodeList[0].assigned = true;
   gaTrackingNodes.aTrackingNodeList[0].slotIdx = node1_slot;
   gaTrackingNodes.aTrackingNodeList[0].node_id = node1_id;
   gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase = true;
   gaTrackingNodes.aTrackingNodeList[1].assigned = true;
   gaTrackingNodes.aTrackingNodeList[1].slotIdx = node2_slot;
   gaTrackingNodes.aTrackingNodeList[1].node_id = node2_id;
   gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase = true;
   gaTrackingNodes.aTrackingNodeList[2].assigned = true;
   gaTrackingNodes.aTrackingNodeList[2].slotIdx = node3_slot;
   gaTrackingNodes.aTrackingNodeList[2].node_id = node3_id;
   gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase = true;
   gaTrackingNodes.aTrackingNodeList[3].assigned = true;
   gaTrackingNodes.aTrackingNodeList[3].slotIdx = node4_slot;
   gaTrackingNodes.aTrackingNodeList[3].node_id = node4_id;

   // Manually configure the tracking nodes in the mesh form and heal module.
   gaMeshNeighbourList[0].Bitfield.InUse = true;
   gaMeshNeighbourList[1].Bitfield.InUse = true;
   gaMeshNeighbourList[2].Bitfield.InUse = true;
   gaMeshNeighbourList[3].Bitfield.InUse = true;

   // Check that the above settings have been correctly applied.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 4);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].node_id, node1_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].node_id, node2_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].node_id, node3_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].node_id, node4_id);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node1_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node2_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node3_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node4_id].Bitfield.InUse, true);

   /* node4 missed heartbeats*/
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(node4_id);
      CU_ASSERT_TRUE(response);
      uint16_t missed_heartbeats = GET_MISSED_HEARTBEATS(gParent[SECONDARY_TRACKING_NODE].NodeInfo);
      CU_ASSERT_EQUAL(missed_heartbeats, count);
   }
   /* one more missed heartbeat should cause the removal of node4 and replace with node2 */
   response = MC_SMGR_MissedNodeHeartbeat(node4_id);
   CU_ASSERT_FALSE(response);

   // Check that the sync module and mesh form and heal module have both been updated correctly.
   // Tracking nodes have all been promoted to one rank higher.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 3);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].node_id, node1_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].node_id, node2_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].node_id, node3_id);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].node_id, DCH_SLOTS_PER_LONG_FRAME);
   // MeshNeighbour entry for node1_id now false.
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node1_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node2_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node3_id].Bitfield.InUse, true);
   CU_ASSERT_EQUAL(gaMeshNeighbourList[node4_id].Bitfield.InUse, false);

   /* check parent table */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, node1_id);
   CU_ASSERT_TRUE(gParent[PRIMARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, node2_id);
   CU_ASSERT_TRUE(gParent[SECONDARY_PARENT].IsActive);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, node3_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);
}

/*************************************************************************************/
/**
* MC_SMGR_GetNeighbourInformationTest
* Test the operation of function MC_SMGR_GetNeighbourInformation();
*
* no params
*
* @return - none

*/
static void MC_SMGR_GetNeighbourInformationTest(void)
{
   const uint8_t rank = 1;
   const uint8_t PARENT = 0;
   const uint8_t CHILD = 1;
   const uint8_t PRIMARY = 0;
   const uint8_t SECONDARY = 1;
   const uint16_t primary_parent_id = 1;
   const uint16_t secondary_parent_id = 2;
   const uint16_t primary_slot = 17;
   const uint16_t secondary_slot = 18;
   const int8_t primary_parent_rssi = -80;
   const int8_t secondary_parent_rssi = -90;
   const uint8_t primary_parent_numChildrenOfPTNIdx = 1;
   const uint8_t secondary_parent_numChildrenOfPTNIdx = 2;
   const uint8_t child1 = 3;
   const uint8_t child2 = 4;
   const uint8_t child3 = 5;
   const int8_t child1_rssi = -56;
   const int8_t child2_rssi = -65;
   const uint8_t child1_numChildrenOfPTNIdx = 3;
   const uint8_t child2_numChildrenOfPTNIdx = 4;
   uint32_t value = 0;
   bool response = false;

   MC_SMGR_Initialise(false,  0);

   // Out of range checks - NULL value parameter.
   response = MC_SMGR_GetNeighbourInformation(PARENT, PRIMARY, NULL);
   CU_ASSERT_FALSE(response);
   // Out of range checks - Third parent parameter.
   response = MC_SMGR_GetNeighbourInformation(PARENT, 2, &value);
   CU_ASSERT_FALSE(response);
   CU_ASSERT_EQUAL(value, 0xffffffff);

   //Parent checks - No parents found
   value = 0;
   response = MC_SMGR_GetNeighbourInformation(PARENT, PRIMARY, &value);
   CU_ASSERT_FALSE(response);
   CU_ASSERT_EQUAL(value, 0xffffffff);
   value = 0;
   response = MC_SMGR_GetNeighbourInformation(PARENT, SECONDARY, &value);
   CU_ASSERT_FALSE(response);
   CU_ASSERT_EQUAL(value, 0xffffffff);

   /* Set parents */
   bool result = MC_SMGR_SetParentNodes(primary_slot, secondary_slot);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(primary_parent_id, true);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(secondary_parent_id, true);
   CU_ASSERT_TRUE(result);
   //Add a heartbeat for each parent
   result = MC_SMGR_ReceivedNodeHeartbeat(primary_parent_id, primary_parent_rssi, STATE_CONFIG_SYNC_E, rank, primary_parent_numChildrenOfPTNIdx);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_ReceivedNodeHeartbeat(secondary_parent_id, secondary_parent_rssi, STATE_CONFIG_SYNC_E, rank, secondary_parent_numChildrenOfPTNIdx);
   CU_ASSERT_TRUE(result);

   //Parent checks - Primary parent
   value = 0;
   response = MC_SMGR_GetNeighbourInformation(PARENT, PRIMARY, &value);
   CU_ASSERT_TRUE(response);
   CU_ASSERT_EQUAL(value, 0xB00001);
   //Parent checks - Secondary parent
   value = 0;
   response = MC_SMGR_GetNeighbourInformation(PARENT, SECONDARY, &value);
   CU_ASSERT_TRUE(response);
   CU_ASSERT_EQUAL(value, 0xA60002);


   // Child checks - no children
   value = 0;
   response = MC_SMGR_GetNeighbourInformation(CHILD, 0, &value);
   CU_ASSERT_FALSE(response);
   CU_ASSERT_EQUAL(value, 0xffffffff);

   // Add a couple of children
   result = MC_SMGR_RouteAddRequest(child1, 0);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddRequest(child2, 0);
   CU_ASSERT_TRUE(result);
   //Add a heartbeat for each child
   result = MC_SMGR_ReceivedNodeHeartbeat(child1, child1_rssi, STATE_CONFIG_SYNC_E, rank, child1_numChildrenOfPTNIdx);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_ReceivedNodeHeartbeat(child2, child2_rssi, STATE_CONFIG_SYNC_E, rank, child2_numChildrenOfPTNIdx);
   CU_ASSERT_TRUE(result);

   //Child checks - Child 1
   value = 0;
   response = MC_SMGR_GetNeighbourInformation(CHILD, 0, &value);
   CU_ASSERT_TRUE(response);
   CU_ASSERT_EQUAL(value, 0xC80003);
   //Child checks - Child 2
   value = 0;
   response = MC_SMGR_GetNeighbourInformation(CHILD, 1, &value);
   CU_ASSERT_TRUE(response);
   CU_ASSERT_EQUAL(value, 0xBF0004);

   //Child checks - Child 3 doesn't exist
   value = 0;
   response = MC_SMGR_GetNeighbourInformation(CHILD, 2, &value);
   CU_ASSERT_FALSE(response);
   CU_ASSERT_EQUAL(value, 0xffffffff);

   // restore session manager for future tests
   MC_SMGR_Initialise(false,  0);
}

/*************************************************************************************/
/**
* MC_SMGR_GetNumberOfChildrenIndexTest
* Test the operation of function MC_SMGR_GetNumberOfChildrenIndex();
*
* no params
*
* @return - none

*/
static void MC_SMGR_GetNumberOfChildrenIndexTest(void)
{
   uint16_t primary_parent_slot = 2596;
   uint16_t secondary_parent_slot = 2597;

   gNetworkAddress = 3;

   /* clear out any old info */
   MC_SMGR_Initialise(false, gNetworkAddress);
   MC_SMGR_SetParentNodes(primary_parent_slot, secondary_parent_slot);
   gParent[PRIMARY_PARENT].IsActive = true;

   const uint32_t MAX_CHILD_COUNT = 32;
   uint8_t expected_response[] = { 0,1,1,1,1,1,2,2,2,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,13,14,15 };

   for (uint32_t index = 0; index <= MAX_CHILD_COUNT; index++)
   {
      uint16_t childIndex = MC_SMGR_GetNumberOfChildrenIndex();
      CU_ASSERT_EQUAL(childIndex, expected_response[index]);
      MC_SMGR_RouteAddRequest(index + 1, 0);//add another child
   }
}

/*****************************************************************************
*  Function:   MC_SMGR_SetHeartbeatMonitor_SlotNo
*  Description:         Wrapper function for MC_SMGR_SetHeartbeatMonitor_NodeId().
*                       Checks that the slot number is that of a DCH and if so converts
*                       the slot number to a node_id and calls MC_SMGR_SetHeartbeatMonitor_NodeId().
*  Param - slot index:  The slot index of the node's heartbeat.
*  Param - set_monitor: True to start heartbeat monitoring, false to stop it.
*  Returns:             true if the heartbeat monitor was updated, false otherwise.
*  Notes:               Should be called for each added parent or child node.
*****************************************************************************/
static bool MC_SMGR_SetHeartbeatMonitor_SlotNo(const uint32_t super_slot_index, const bool set_monitor)
{
   bool updated = false;

   // Convert slot index within long frame to DCH slot index within long frame
   uint16_t short_frame_slot = super_slot_index % SLOTS_PER_SHORT_FRAME;

   // Get the slot type for this index
   MC_MAC_SlotType_t slot_type = MC_MAC_GetSlotTypeFromSlotNumber(short_frame_slot);

   // Only if the slot type is for DCH should we take any action
   if (MC_MAC_SLOT_TYPE_DCH_E == slot_type)
   {
      // Get the node ID associated with this slot index
      uint16_t node_id = MC_SMGR_CalculateNodeID(super_slot_index);

      updated = MC_SMGR_SetHeartbeatMonitor_NodeId(node_id, set_monitor);
   }

   return updated;
}

/*************************************************************************************/
/**
* MC_SMGR_CheckParentStatesTest
* Test the operation of function MC_SMGR_CheckParentStates(). This compares the local
* state to the states of the active parents.
*
* The test covers the following cases:
* 1)no parents
* 2)all parents and tracking nodes have lower state than local state
* 3)one parent has state equal to local state
*
* no params
*
* @return - none

*/
void MC_SMGR_CheckParentStatesTest()
{
   const uint8_t rank = 1;
   const uint16_t node1_id = 0;
   const uint16_t node2_id = 1;
   const uint16_t node3_id = 2;
   const uint16_t node4_id = 3;
   const uint16_t node1_slot = 16;
   const uint16_t node2_slot = 17;
   const uint16_t node3_slot = 18;
   const uint16_t node4_slot = 19;
   const uint8_t node1_numChildrenOfPTNIdx = 1;
   const uint8_t node2_numChildrenOfPTNIdx = 2;
   const uint8_t node3_numChildrenOfPTNIdx = 3;
   const uint8_t node4_numChildrenOfPTNIdx = 4;
   const int8_t snr = 10;
   gNetworkAddress = 3;
   bool response = false;

   RACHSQ = osMessageCreate(osMessageQ(RACHSQ), NULL);
   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));

   /* clear out any old info */
   MC_SMGR_Initialise(false,  gNetworkAddress);

   /* 1)no parents */
   /* check parent state should return false */
   bool result = MC_SMGR_CheckParentStates(STATE_ACTIVE_E, 0);
   CU_ASSERT_FALSE(result);

   /* Set parents */
   result = MC_SMGR_SetParentNodes(node1_slot, node2_slot);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(node1_id, true);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_RouteAddResponse(node2_id, true);
   CU_ASSERT_TRUE(result);

   /* Set tracking nodes */
   result = MC_SMGR_SetTrackingNodes(node3_slot, node4_slot);
   CU_ASSERT_TRUE(result);

   /* 2)all parents and tracking nodes have lower state than local state */
   /* receive heartbeats to set state to config_form */
   result = MC_SMGR_ReceivedNodeHeartbeat(node1_id, snr, STATE_CONFIG_FORM_E, rank, node1_numChildrenOfPTNIdx);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_ReceivedNodeHeartbeat(node2_id, snr, STATE_CONFIG_FORM_E, rank, node2_numChildrenOfPTNIdx);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_ReceivedNodeHeartbeat(node3_id, snr, STATE_CONFIG_FORM_E, rank, node3_numChildrenOfPTNIdx);
   CU_ASSERT_TRUE(result);
   result = MC_SMGR_ReceivedNodeHeartbeat(node4_id, snr, STATE_CONFIG_FORM_E, rank, node4_numChildrenOfPTNIdx);
   CU_ASSERT_TRUE(result);

   /* check parent state should return true */
   result = MC_SMGR_CheckParentStates(STATE_ACTIVE_E, 0);
   CU_ASSERT_TRUE(result);

   /* 3)one parent has state equal to local state*/
   /* receive heartbeat to set state to active */
   result = MC_SMGR_ReceivedNodeHeartbeat(node3_id, snr, STATE_ACTIVE_E, rank, node3_numChildrenOfPTNIdx);
   CU_ASSERT_TRUE(result);

   /* check parent state should return false */
   result = MC_SMGR_CheckParentStates(STATE_ACTIVE_E, 0);
   CU_ASSERT_FALSE(result);

}


/*************************************************************************************/
/**
* MC_SMGR_GetPrimaryParentIDTest
* Test the operation of function MC_SMGR_GetPrimaryParentID().
*
* The test covers the following cases:
* 1)no primary parent
* 2)with primary parent
*
* no params
*
* @return - none

*/
void MC_SMGR_GetPrimaryParentIDTest()
{
   uint16_t primary_parent_slot = 17;
   uint16_t primary_parent_id = 1;

   /* clear out any old info */
   MC_SMGR_Initialise(false,  gNetworkAddress);

   /* check the response with no parents */
   uint16_t node_id = MC_SMGR_GetPrimaryParentID();
   CU_ASSERT_EQUAL(node_id, MC_SMGR_BAD_NODE_ID);

   bool result = MC_SMGR_SetParentNodes(primary_parent_slot, SECONDARY_PARENT_NOT_ASSIGNED);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, primary_parent_id);

   /* check that we get no ID back if the parent is inactive */
   CU_ASSERT_FALSE(gParent[PRIMARY_PARENT].IsActive);
   node_id = MC_SMGR_GetPrimaryParentID();
   CU_ASSERT_EQUAL(node_id, MC_SMGR_BAD_NODE_ID);

   /* Activate the parent then check that the session manager returns its node ID */
   gParent[PRIMARY_PARENT].IsActive = true;
   node_id = MC_SMGR_GetPrimaryParentID();
   CU_ASSERT_EQUAL(node_id, primary_parent_id);
}

/*************************************************************************************/
/**
* MC_SMGR_NotifyMessageFailureTest
* Test the operation of function MC_SMGR_NotifyMessageFailure().
*
*
* no params
*
* @return - none

*/
void MC_SMGR_NotifyMessageFailureTest(void)
{
   uint16_t primary_parent_slot = 17;
   uint16_t primary_parent_id = 1;
   uint16_t secondary_parent_slot = 18;
   uint16_t secondary_parent_id = 2;
   uint16_t primary_tracking_slot = 36;
   uint16_t primary_tracking_id = 4;
   uint16_t secondary_tracking_slot = 37;
   uint16_t secondary_tracking_id = 5;

   MeshQ = osMessageCreate(osMessageQ(MeshQ), NULL);
   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));
 
 /* clear out any old info */
   MC_SMGR_Initialise(false,  gNetworkAddress);

   uint16_t node_id = MC_SMGR_GetPrimaryParentID();
   CU_ASSERT_EQUAL(node_id, MC_SMGR_BAD_NODE_ID);

   bool result = MC_SMGR_SetParentNodes(primary_parent_slot, secondary_parent_slot);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, primary_parent_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, secondary_parent_id);

   /* add tracking nodes */
   result = MC_SMGR_SetTrackingNodes(primary_tracking_slot, secondary_tracking_slot);
   CU_ASSERT_TRUE(result);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, primary_tracking_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, secondary_tracking_id);

   /* check the primary parent isn't active */
   CU_ASSERT_FALSE(gParent[PRIMARY_PARENT].IsActive);

   /* clear old MeshQ messages */
   osMessageQFlush(MeshQ);

   /* call the function under test */
   /* inform the session management that the primary parent failed to acknowledge the Route Add message*/
   result = MC_SMGR_NotifyMessageFailure(APP_MSG_TYPE_ROUTE_ADD_E, primary_parent_id);
   CU_ASSERT_TRUE(result);
   /* the primary parent should be removed and the other nodes are all promoted one place*/
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, secondary_parent_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, primary_tracking_id);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, secondary_tracking_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);
   /* there should be 2 messages in the meshQ */
   CU_ASSERT_EQUAL(2, osMessageCount(MeshQ));
   /* read the first message.  It should be a Generate Route Add Request addressed to the secondary parent id */
   osEvent meshEvent;
   meshEvent.status = osErrorOS;
   meshEvent = osMessageGet(MeshQ, 0);
   CU_ASSERT_EQUAL(osEventMessage, meshEvent.status);
   CO_Message_t* pMessage = (CO_Message_t*)meshEvent.value.p;
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      /* check the message type */
      CU_ASSERT_EQUAL(pMessage->Type, CO_MESSAGE_GENERATE_ROUTE_ADD_REQUEST_E);
      /* check the destination address */
      uint16_t* pDestination = (uint16_t*)pMessage->Payload.PhyDataReq.Data;
      CU_ASSERT_EQUAL(*pDestination, secondary_parent_id);

      //Release the memory pool slot
      osStatus osStat = osPoolFree(MeshPool, pMessage);
      CU_ASSERT_EQUAL(osOK, osStat);
   }

   /* read the second message.  It should be a Generate Route Add Request addressed to the primary tracking id */
   meshEvent.status = osErrorOS;
   meshEvent = osMessageGet(MeshQ, 0);
   CU_ASSERT_EQUAL(osEventMessage, meshEvent.status);
   pMessage = (CO_Message_t*)meshEvent.value.p;
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      /* check the message type */
      CU_ASSERT_EQUAL(pMessage->Type, CO_MESSAGE_GENERATE_ROUTE_ADD_REQUEST_E);
      /* check the destination address */
      uint16_t* pDestination = (uint16_t*)pMessage->Payload.PhyDataReq.Data;
      CU_ASSERT_EQUAL(*pDestination, primary_tracking_id);

      //Release the memory pool slot
      osStatus osStat = osPoolFree(MeshPool, pMessage);
      CU_ASSERT_EQUAL(osOK, osStat);
   }
}
