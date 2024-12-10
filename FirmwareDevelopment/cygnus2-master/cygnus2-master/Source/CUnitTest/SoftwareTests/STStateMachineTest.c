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
*  File         : STStateMachineTest.c
*
*  Description  : Implementation of the State Machine tests
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
#include "CO_ErrorCode.h"
#include "STStateMachineTest.h"
#include "SM_StateMachine.h"
#include "gpio.h"
#include "MessageQueueStub.h"
#include "MC_SessionManagement.h"
#include "MC_AckManagement.h"
#include "MC_PUP.h"
#include "MC_MeshFormAndHeal.h"
#include "MC_MAC.h"
#include "CO_Message.h"
#include "MC_SyncPublic.h"
#include "MC_SyncPrivate.h"

/* Definitions
*************************************************************************************/

#define NUM_CHILD_IDX_3   (3)
#define NUM_CHILD_IDX_4   (4)
#define NUM_CHILD_IDX_5   (5)
#define NUM_CHILD_IDX_6   (6)
#define NUM_CHILD_IDX_7   (7)

#define RSSI_HIGH         (-40)
#define RSSI_LOW          (-50)
#define SNR_IDX_HIGH      (20)
#define SNR_IDX_LOW       (MIN_ALLOWED_SNR)
#define SNR_IDX_TOO_LOW   (MIN_ALLOWED_SNR - 1)  // This is below the Minimum Attenuation Reserve SNR Index value.

/* Private Functions Prototypes
*************************************************************************************/
static void SM_RBUMeshFormingRank1Test(void);
static void SM_RBUMeshFormingAndReformingTest1(void);
static void SM_RBUMeshFormingMixedSNRTest(void);
static void SM_RBUMeshFormingMaxRankTest(void);
static void SM_SingleUnitAddressValidTest(void);
static void SM_SingleZoneAddressValidTest(void);
static void SM_AllUnitsAddressValidTest(void);
static void SM_AddressInvalidTest(void);
static void SM_GenerateLogonTest(void);
static void SM_RBUReceiveLogonTest(void);
static void SM_NCUGenerateHeartbeatTest(void);
static void SM_RBUGenerateHeartbeatTest(void);
static void SM_NCUStateTransitionTest(void);
static void SM_RBUStateTransitionTest(void);
static void SM_RBUReceivedHeartbeatTest(void);
static void SM_NCUGenerateOutputSignalTest(void);
static void SM_RBUGenerateOutputSignalTest(void);
static void SM_NCUReceiveOutputSignalTest(void);
static void SM_RBUReceiveOutputSignalTest(void);
static void SM_RBUReceiveFireSignalTest(void);
static void SM_RBUReceiveAlarmSignalTest(void);
static void SM_RBUReceiveFaultSignalTest(void);
static void SM_RBUTestModeTest(void);
static void SM_RBUGenerateTestSignalTest(void);
static void SM_NCUGenerateTestSignalTest(void);
static void SM_NCUGenerateSetStateTest(void);
static void SM_RBUGenerateSetStateTest(void);
static void SM_NCUReceiveSetStateTest(void);
static void SM_RBUReceiveSetStateTest(void);
static void SM_RBUAverageSNRTest(void);
static void SM_NCUGenerateRBUDisableTest(void);
static void SM_RBUDisableMessageTest(void);
static void SM_GenerateRouteDropTest(void);
static void SM_RouteDropTest(void);
static void PPPTheHeartbeats(FrameHeartbeat_t * pHeartbeat, CO_Message_t* pMessage, uint32_t LongFrameIndex, uint8_t SetSNRveryLow);

void SM_PopulateHeartbeatMessage(
   FrameHeartbeat_t * pHeartbeat,
   CO_Message_t * pMessage,
   uint32_t LongFrameIndex,
   uint32_t ShortFrameIndex,
   uint32_t SlotIndex,
   uint32_t Rank,
   uint32_t NoOfChildrenIdx,
   int16_t  RSSI,
   int8_t SNR);
void SM_SendNewLongFrameMessage();

/* Global Variables
*************************************************************************************/
extern uint16_t gNetworkAddress;
extern uint32_t gZoneNumber;
extern uint8_t gDeviceRank;   // Correct value for an NCU: get's overwritten in the RBU before it is read.
extern MC_TrackingNodes_t gaTrackingNodes;  // Stores details of the selected parent and tracking nodes.
extern uint16_t gPrelimTrackingSlot;
extern osPoolId MeshPool;
extern SMGR_ParentDetails_t gParent[];
extern bool  bErrorHandlerCalled;

extern void Error_Handler(char* error_msg);
extern void MC_InitMeshFormAndHeal(const uint32_t address);
extern ErrorCode_t MC_ResetSyncDatabase(void);
extern CO_State_t gSM_ScheduledDeviceState;

extern const SerialChannelDefinition_t SerialChannelDefinition[MAX_SUPPORTED_SERIAL_PORT];

/* Table containing the test settings */
CU_TestInfo ST_StateMachineTests[] =
{
   { "RBUMeshFormingMixedSNRTest",           SM_RBUMeshFormingMixedSNRTest },
   { "RBUMeshFormingAndReformingTest1",       SM_RBUMeshFormingAndReformingTest1 },
   { "RBUMeshFormingRank1Test",              SM_RBUMeshFormingRank1Test },
   { "RBUMeshFormingMaxRankTest",            SM_RBUMeshFormingMaxRankTest },
   { "SingleUnitAddressValidTest",           SM_SingleUnitAddressValidTest },
   { "SingleZoneAddressValidTest",           SM_SingleZoneAddressValidTest },
   { "AllUnitsAddressValidTest",             SM_AllUnitsAddressValidTest },
   { "AddressInvalidTest",                   SM_AddressInvalidTest },
   { "GenerateLogonTest",                    SM_GenerateLogonTest },
   { "RBUReceiveLogonTest",                  SM_RBUReceiveLogonTest },
   { "NCUGenerateHeartbeatTest",             SM_NCUGenerateHeartbeatTest },
   { "RBUGenerateHeartbeatTest",             SM_RBUGenerateHeartbeatTest },
   { "NCUStateTransitionTest",               SM_NCUStateTransitionTest },
   { "RBUStateTransitionTest",               SM_RBUStateTransitionTest },
   { "RBUReceivedHeartbeatTest",             SM_RBUReceivedHeartbeatTest },
   { "NCUGenerateOutputSignalTest",          SM_NCUGenerateOutputSignalTest },
   { "RBUGenerateOutputSignalTest",          SM_RBUGenerateOutputSignalTest },
   { "NCUReceiveOutputSignalTest",           SM_NCUReceiveOutputSignalTest },
   { "RBUReceiveOutputSignalTest",           SM_RBUReceiveOutputSignalTest },
   { "RBUReceiveFireSignalTest",             SM_RBUReceiveFireSignalTest },
   { "RBUReceiveAlarmSignalTest",            SM_RBUReceiveAlarmSignalTest },
   { "RBUReceiveFaultSignalTest",            SM_RBUReceiveFaultSignalTest },
   { "RBUTestModeTest",                      SM_RBUTestModeTest },
   { "RBUGenerateTestSignalTest",            SM_RBUGenerateTestSignalTest },
   { "NCUGenerateTestSignalTest",            SM_NCUGenerateTestSignalTest },
   { "NCUGenerateSetStateTest",              SM_NCUGenerateSetStateTest },
   { "RBUGenerateSetStateTest",              SM_RBUGenerateSetStateTest },
   { "NCUReceiveSetStateTest",               SM_NCUReceiveSetStateTest },
   { "RBUReceiveSetStateTest",               SM_RBUReceiveSetStateTest },
   { "RBUAverageSNRTest",                    SM_RBUAverageSNRTest },
   { "NCUGenerateRBUDisableTest",            SM_NCUGenerateRBUDisableTest },
   { "RBUDisableMessageTest",                SM_RBUDisableMessageTest },
   { "GenerateRouteDropTest",                SM_GenerateRouteDropTest },
   { "RouteDropTest",                        SM_RouteDropTest },

   CU_TEST_INFO_NULL,
};



/* Private Variables
*************************************************************************************/

/* Test utilities
*************************************************************************************/

#define SYSTEM_ID 0x12345678
#define WRONG_SYSTEM_ID 0x87654321

// Send a Generate Heartbeat request to the state machine
void GenerateHeartbeat(uint8_t slot_number)
{
   /* create a Generate Heartbeat message */
   CO_MessagePayloadGenerateHeartbeat_t hbData;
   hbData.LongFrameIndex = 0;
   hbData.ShortFrameIndex = 0;
   hbData.SlotIndex = slot_number;


   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      memcpy(&pMessage->Payload.GenerateHeartbeat, &hbData, sizeof(CO_MessagePayloadGenerateHeartbeat_t));

      SMHandleEvent(EVENT_GENERATE_HEARTBEAT_E, (uint8_t *)pMessage);
   }
}

void MoveRBUToOperationalState(uint16_t rbu_node_id, uint16_t parent_node_id, uint32_t systemId)
{
   /* An RBU moves to OPERATIONAL when it receives an ack to its Logon message.
      To move an RBU into the operational state for our tests, we need to put a RouteAdd message
      into AckManagement, then acknowledge it, then send a Logon message and ack it.
      The AckManager needs to recognise the parent, so we must first add the parent.
      The session manager will generate the route Add message for us if we call MC_SMGR_ConnectToParent*/
   uint16_t ack_slot = 14;/*secondary rach ack*/
   MC_ACK_Initialise();
   bool result = false;

   /* create a RouteAdd message */
   uint16_t parent_slot = MC_SMGR_CalculateDCHSlot(parent_node_id);
   result = MC_SMGR_SetParentNodes(parent_slot, MC_SMGR_BAD_NODE_ID);
   CU_ASSERT_TRUE(result);
   /* create a route add response */
   MC_SMGR_RouteAddResponse(parent_node_id, true);
   CU_ASSERT_TRUE(MC_SMGR_IsParentNode(parent_node_id));

   /* now ack the route add message */
   ErrorCode_t ack_status = ERR_UNEXPECTED_ERROR_E;
   FrameAcknowledge_t FrameAcknowledgement;
   CO_Message_t *pPhyDataInd = NULL;
   osStatus osStat = osErrorOS;

   FrameAcknowledgement.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   FrameAcknowledgement.MACDestinationAddress = rbu_node_id;
   FrameAcknowledgement.MACSourceAddress = parent_node_id;
   FrameAcknowledgement.SystemId = systemId;

   /* Pack ACK message and pass it to the state machine.*/
   pPhyDataInd = osPoolAlloc(MeshPool);
   if (NULL != pPhyDataInd)
   {
      pPhyDataInd->Type = CO_MESSAGE_PHY_DATA_REQ_E;
      pPhyDataInd->Payload.PhyDataInd.slotIdxInSuperframe = ack_slot;
      pPhyDataInd->Payload.PhyDataInd.slotIdxInLongframe = ack_slot;
      pPhyDataInd->Payload.PhyDataInd.slotIdxInShortframe = (uint8_t)ack_slot;
      ack_status = MC_PUP_PackAck(&FrameAcknowledgement, &pPhyDataInd->Payload.PhyDataReq);

      if (SUCCESS_E == ack_status)
      {
         SMHandleEvent(EVENT_ACK_SIGNAL_E, (uint8_t *)pPhyDataInd);
      }

      SM_SetCurrentDeviceState(STATE_ACTIVE_E);
   }
}

uint16_t gRxDoneRssi;

//uint32_t LogonCnfCallback_Count = 0;
//uint32_t LogonCnfCallback_LastHandle = 0;
//uint32_t LogonCnfCallback_LastStatus = 0;
//void ST_LogonCnfCallback(const uint32_t Handle, const uint32_t Status)
//{
//   LogonCnfCallback_Count++;
//   LogonCnfCallback_LastHandle = Handle;
//   LogonCnfCallback_LastStatus = Status;
//}
//
//uint32_t  OutputSignalInd_Count = 0;
//uint32_t  OutputSignalInd_Handle = 0;
//uint16_t OutputSignalInd_Zone = 0;
//uint8_t  OutputSignalInd_OutputProfile = 0;
//uint8_t  OutputSignalInd_OutputsActivated = 0;
//void ST_OutputSignalInd(const uint32_t Handle, const uint16_t Zone, const uint8_t OutputProfile, const uint8_t OutputsActivated)
//{
//   OutputSignalInd_Count++;
//   OutputSignalInd_Handle = Handle;
//   OutputSignalInd_Zone = Zone;
//   OutputSignalInd_OutputProfile = OutputProfile;
//   OutputSignalInd_OutputsActivated = OutputsActivated;
//}
//uint32_t LogonIndCallback_Count = 0;
//uint32_t LogonIndCallback_LastHandle = 0;
//uint32_t LogonIndCallback_LastSerialNumber = 0;
//uint32_t LogonIndCallback_LastDeviceCombination = 0;
//uint32_t LogonIndCallback_LastSourceAddress = 0;
//uint32_t LogonIndCallback_LastStatus = 0;
//void ST_LogonIndCallback(const uint32_t Handle, const uint32_t SerialNumber, const uint8_t DeviceCombination, const uint16_t StatusFlags, const uint16_t SourceAddress)
//{
//   LogonIndCallback_Count++;
//   LogonIndCallback_LastHandle = Handle;
//   LogonIndCallback_LastSerialNumber = SerialNumber;
//   LogonIndCallback_LastDeviceCombination = DeviceCombination;
//   LogonIndCallback_LastSourceAddress = SourceAddress;
//   LogonIndCallback_LastStatus = StatusFlags;
//}
//
//uint32_t  RBUDisableInd_Count = 0;
//uint32_t  RBUDisableInd_Handle = 0;
//uint16_t  RBUDisableInd_UnitAddress = 0;
//void ST_RBUDisableInd(const uint32_t Handle, const uint16_t UnitAddress)
//{
//   RBUDisableInd_Count++;
//   RBUDisableInd_Handle = Handle;
//   RBUDisableInd_UnitAddress = UnitAddress;
//}
//
/*************************************************************************************/
/**
* SM_SingleUnitAddressValidTest
* test addressing of nodes using individual address - valid address
*
* no params
*
* @return - none

*/
static void SM_SingleUnitAddressValidTest(void)
{
   uint32_t zone = 0;
   bool isValid = false;

   // set RBU address to unit 1
   gNetworkAddress = 1;
   // set RBU zone to 1
   gZoneNumber = 1;

   // test dest indiv addresses in zone 0
   isValid = SMCheckDestinationAddress(0x000, zone);
   CU_ASSERT_EQUAL(isValid, false);

   isValid = SMCheckDestinationAddress(0x001, zone);
   CU_ASSERT_EQUAL(isValid, true);

   isValid = SMCheckDestinationAddress(0x011, zone);
   CU_ASSERT_EQUAL(isValid, false);


   // test dest indiv addresses in zone 1
   zone = 1;
   isValid = SMCheckDestinationAddress(0x000, zone);
   CU_ASSERT_EQUAL(isValid, false);

   isValid = SMCheckDestinationAddress(0x001, zone);
   CU_ASSERT_EQUAL(isValid, true);

   isValid = SMCheckDestinationAddress(0x011, zone);
   CU_ASSERT_EQUAL(isValid, false);

   // set RBU address to zone 63, unit 31
   zone = ZONE_GLOBAL;

   // test dest indiv addresses in zone 0
   isValid = SMCheckDestinationAddress(0x000, zone);
   CU_ASSERT_EQUAL(isValid, false);

   isValid = SMCheckDestinationAddress(0x001, zone);
   CU_ASSERT_EQUAL(isValid, true);

   isValid = SMCheckDestinationAddress(0x002, zone);
   CU_ASSERT_EQUAL(isValid, false);

   //Check that zone activates when the address is global
   zone = 1;
   isValid = SMCheckDestinationAddress(ADDRESS_GLOBAL, zone);
   CU_ASSERT_EQUAL(isValid, true);
}


/*************************************************************************************/
/**
* SM_SingleZoneAddressValidTest
* test addressing of all nodes in a zone using zone address - valid address
*
* no params
*
* @return - none

*/
static void SM_SingleZoneAddressValidTest(void)
{
   // set RBU address to zone 0, unit 1
   uint32_t zone = 0;
   bool isValid = false;

   // set RBU address to unit 1
   gNetworkAddress = 1;
   // set RBU zone to 1
   gZoneNumber = 1;

   // test dest zone 0
   isValid = SMCheckDestinationAddress(ADDRESS_GLOBAL, zone);
   CU_ASSERT_EQUAL(isValid, false);

   // test dest zone 1
   zone = 1;
   isValid = SMCheckDestinationAddress(ADDRESS_GLOBAL, zone);
   CU_ASSERT_EQUAL(isValid, true);

   // test dest zone 63
   zone = 63;
   isValid = SMCheckDestinationAddress(ADDRESS_GLOBAL, zone);
   CU_ASSERT_EQUAL(isValid, false);
}


/*************************************************************************************/
/**
* SM_AllUnitsAddressValidTest
* test addressing of all nodes in the network using global address - valid address case
*
* no params
*
* @return - none

*/
static void SM_AllUnitsAddressValidTest(void)
{
   uint32_t zone = 1;
   bool isValid = false;


   // set RBU address to unit 1
   gNetworkAddress = 1;
   // set RBU zone to 1
   gZoneNumber = 1;

   // test dest global address
   isValid = SMCheckDestinationAddress(ADDRESS_GLOBAL, zone);
   CU_ASSERT_EQUAL(isValid, true);

   // test zone filter
   zone = 2;
   isValid = SMCheckDestinationAddress(ADDRESS_GLOBAL, zone);
   CU_ASSERT_EQUAL(isValid, false);

   //Test all nodes in all zones
   isValid = SMCheckDestinationAddress(ADDRESS_GLOBAL, ZONE_GLOBAL);
   CU_ASSERT_EQUAL(isValid, true);

}


/*************************************************************************************/
/**
* SM_AddressInvalidTest
* test handling of invalid addresses
*
* no params
*
* @return - none

*/
static void SM_AddressInvalidTest(void)
{
   bool isValid = false;
   uint32_t zone = 0;

   // test invalid destination address
   isValid = SMCheckDestinationAddress(0x1000, zone);
   CU_ASSERT_EQUAL(isValid, false);

   // Invalid zone test
   zone = ZONE_GLOBAL + 1;
   isValid = SMCheckDestinationAddress(ADDRESS_GLOBAL, zone);
   CU_ASSERT_EQUAL(isValid, false);
}

/*************************************************************************************/
/**
* SM_GenerateLogonTest
* Test the logon procedure
*
* no params
*
* @return - none

*/
void SM_GenerateLogonTest(void)
{
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t LogonReq;
   CO_LogonData_t logonData;
   uint16_t parent_slot_number = 16;
   uint16_t parent_id = 0;
   uint32_t handle = 77;
   uint32_t system_id = SYSTEM_ID;

   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));

   //LogonCnfCallback_Count = 0;
   //LogonCnfCallback_LastHandle = 0;
   //LogonCnfCallback_LastStatus = 0;

   SMInitialise(false, 1, system_id);
   MC_ACK_Initialise();

 //  SMSetLogonCnfCb(&ST_LogonCnfCallback);

   /* NULL data test */
   SMHandleEvent(EVENT_GENERATE_LOGON_REQ_E, NULL);
   //CU_ASSERT_EQUAL(LogonCnfCallback_Count, 1);
   //CU_ASSERT_EQUAL(LogonCnfCallback_LastHandle, 0);
   //CU_ASSERT_EQUAL(LogonCnfCallback_LastStatus, ERR_INVALID_POINTER_E);

   /* create LogonReq message */
   logonData.Handle = handle;
   logonData.SerialNumber = 0;
   logonData.DeviceCombination = 0;
   logonData.ZoneNumber = 0;
   LogonReq.Type = CO_MESSAGE_GENERATE_LOGON_REQ;
   memcpy(&LogonReq.Payload.PhyDataReq.Data, &logonData, sizeof(CO_LogonData_t));

   /* test for failure due to no parents to send the message to*/
   //LogonCnfCallback_Count = 0;
   //LogonCnfCallback_LastHandle = 0;
   //LogonCnfCallback_LastStatus = 0;
   SMHandleEvent(EVENT_GENERATE_LOGON_REQ_E, (uint8_t*)&LogonReq);

   //CU_ASSERT_EQUAL(LogonCnfCallback_Count, 1);
   //CU_ASSERT_EQUAL(LogonCnfCallback_LastHandle, handle);
   //CU_ASSERT_EQUAL(LogonCnfCallback_LastStatus, WARNING_NO_NEIGHBOUR_FOUND_E);

   /*set up parents in the ack manager and session manager*/
   MC_SMGR_SetParentNodes(parent_slot_number, 0);
   /* pretend we have had a response from the parent*/
   MC_SMGR_RouteAddResponse(parent_id, true);

   // Need to set a Node ID for the tracking node.  We will set it to 0, which would be that of the NCU.
   gTrackingNode_id = 0;

   /* Test for failure due to no mem pool resources */
   /* Fill the MeshPool with fake allocations */
   while (0 < osPoolFreeSpace(MeshPool))
   {
      osPoolAlloc(MeshPool);
   }
   //LogonCnfCallback_Count = 0;
   //LogonCnfCallback_LastHandle = 0;
   //LogonCnfCallback_LastStatus = 0;
   SMHandleEvent(EVENT_GENERATE_LOGON_REQ_E, (uint8_t*)&LogonReq);

   //CU_ASSERT_EQUAL(LogonCnfCallback_Count, 1);
   //CU_ASSERT_EQUAL(LogonCnfCallback_LastHandle, handle);
   //CU_ASSERT_EQUAL(LogonCnfCallback_LastStatus, ERR_NO_RESOURCE_E);

   /*discard the fake pool allocations*/
   osPoolFlush(MeshPool);


   /* test successful case*/

   //Reset the Ack Manager and verify that it has got no message queued
   MC_ACK_Initialise();
   bool msg_ready = MC_ACK_MessageReadyToSend(ACK_RACHS_E);
   CU_ASSERT_FALSE(msg_ready);

   //LogonCnfCallback_Count = 0;
   //LogonCnfCallback_LastHandle = 0;
   //LogonCnfCallback_LastStatus = 0;
   SMHandleEvent(EVENT_GENERATE_LOGON_REQ_E, (uint8_t*)&LogonReq);
   //There will be no application callback if the logon was generated successfully (that's done when the response comes back)
   //CU_ASSERT_EQUAL(LogonCnfCallback_Count, 0);


   //Check that the Ack Manager has got a message queued
   msg_ready = MC_ACK_MessageReadyToSend(ACK_RACHS_E);
   CU_ASSERT_TRUE(msg_ready);
}

/*************************************************************************************/
/**
* SM_RBUReceiveLogonTest
* Test the RBU receive of logon messages.
* Logon Messages should only be accepted in OPERATIONAL state and should always be 
* repeated uplink towards the NCU, except when the hop count limit is reached.
*
* no params
*
* @return - none

*/
void SM_RBUReceiveLogonTest(void)
{
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   const uint16_t NCU_DCH_SLOT = 16;
   uint32_t rachsq_count;
   uint16_t rbu_node1_id = 1;
   uint16_t rbu_node2_id = 2;
   uint16_t parent_node_id = 0;
   uint32_t system_id = SYSTEM_ID;
   uint16_t primary_parent_slot = 2596;
   uint16_t secondary_parent_slot = 2597;

   gNetworkAddress = 1;

   RACHSQ = osMessageCreate(osMessageQ(RACHSQ), NULL);

   osPoolFlush(MeshPool);
   osMessageQFlush(RACHSQ);

   rachsq_count = osMessageCount(RACHSQ);
   CU_ASSERT_EQUAL(rachsq_count, 0);

   MC_ACK_Initialise();
   SMInitialise(false, rbu_node1_id, system_id);//RBU
//   SMSetLogonIndCb(&ST_LogonIndCallback);

   /* initialise the session manager */
   MC_SMGR_Initialise(false, rbu_node1_id);
   MC_SMGR_SetParentNodes(primary_parent_slot, secondary_parent_slot);
   gParent[PRIMARY_PARENT].IsActive = true;

   MC_SMGR_RouteAddRequest(rbu_node2_id, 0);
   CU_ASSERT_TRUE(MC_SMGR_IsAChild(rbu_node2_id));

   /* create logon message */
   LogOnMessage_t logonMsg;
   logonMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   logonMsg.Header.MACDestinationAddress = rbu_node1_id;
   logonMsg.Header.MACSourceAddress = rbu_node2_id;
   logonMsg.Header.HopCount = 0;
   logonMsg.Header.DestinationAddress = ADDRESS_NCU;
   logonMsg.Header.SourceAddress = rbu_node2_id;
   logonMsg.Header.MessageType = APP_MSG_TYPE_LOGON_E;
   logonMsg.DeviceCombination = 34;
   logonMsg.SerialNumber = 1;
   logonMsg.ZoneNumber = 0;
   logonMsg.SystemId = system_id;


   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      ErrorCode_t result = MC_PUP_PackLogOnMsg(&logonMsg, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      //The state machine should be in the STATE_CONFIG_E state
      CO_State_t state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

      SMHandleEvent(EVENT_LOGON_REQ_E, (uint8_t *)pMessage);

      //There should be no output when in state STATE_CONFIG_E
      rachsq_count = osMessageCount(RACHSQ);
      CU_ASSERT_EQUAL(rachsq_count, 0);

      // Now change the state to STATE_ACTIVE_E by using the correct system ID
      MoveRBUToOperationalState(rbu_node1_id, parent_node_id, system_id);

      //The state machine should be in the STATE_ACTIVE_E state
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

      //Send the logon signal again
      SMHandleEvent(EVENT_LOGON_REQ_E, (uint8_t *)pMessage);

      //The message was addressed to another node so it should be scheduled for re-broadcast.
      bool ready_to_send = MC_ACK_MessageReadyToSend(ACK_RACHS_E);
      CU_ASSERT_TRUE(ready_to_send);

      /* check the msg type*/
      ApplicationLayerMessageType_t msg_type;
      uint32_t handle;
      MC_ACK_AcknowledgeMessage(ACK_RACHS_E, ADDRESS_NCU, true, &msg_type, &handle);
      CU_ASSERT_EQUAL(msg_type, APP_MSG_TYPE_LOGON_E);

      //Repeat logon msg with Hop Count = MAX_MSG_HOP_THRESHOLD - 1.
      // The message should not be sent uplink
      logonMsg.Header.HopCount = MAX_MSG_HOP_THRESHOLD - 1;
      result = MC_PUP_PackLogOnMsg(&logonMsg, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      //Send the logon signal again
      SMHandleEvent(EVENT_LOGON_REQ_E, (uint8_t *)pMessage);
      //The message hop limit was reached so it should not be re-broadcast.
      ready_to_send = MC_ACK_MessageReadyToSend(ACK_RACHS_E);
      CU_ASSERT_FALSE(ready_to_send);
   }
}


/*************************************************************************************/
/**
* SM_NCUGenerateHeartbeatTest
* Test the NCU heartbeat generation
*
* no params
*
* @return - none

*/
void SM_NCUGenerateHeartbeatTest(void)
{
   uint32_t system_id = SYSTEM_ID;

   DCHQ = osMessageCreate(osMessageQ(DCHQ), NULL);

   SMInitialise(true, 0, system_id);

   /* set the state to IDLE */
   gSM_CurrentDeviceState = STATE_IDLE_E;

   /* send a 'generate heartbeat' event. */
   CO_Message_t msgGenHeartbeat;

   msgGenHeartbeat.Type = CO_MESSAGE_GENERATE_HEARTBEAT_E;
   msgGenHeartbeat.Payload.GenerateHeartbeat.SlotIndex = 16;
   msgGenHeartbeat.Payload.GenerateHeartbeat.ShortFrameIndex = 1;
   msgGenHeartbeat.Payload.GenerateHeartbeat.LongFrameIndex = 2;

   SMHandleEvent(EVENT_GENERATE_HEARTBEAT_E, (uint8_t*)&msgGenHeartbeat);

   /*Check that no heartbeat was generated in IDLE mode */
   uint32_t message_count = osMessageCount(DCHQ);
   CU_ASSERT_EQUAL(message_count, 0);

   /* set the state to CONFIG_SYNC */
   gSM_CurrentDeviceState = STATE_CONFIG_SYNC_E;

   SMHandleEvent(EVENT_GENERATE_HEARTBEAT_E, (uint8_t*)&msgGenHeartbeat);

   /*Check that a message was dropped into the DCH queue */
   message_count = osMessageCount(DCHQ);
   CU_ASSERT_EQUAL(message_count, 1);
   /*Check that the message is a heartbeat */
   osEvent event;
   event = osMessageGet(DCHQ, 0);
   CU_ASSERT_EQUAL(osEventMessage, event.status);
   CO_Message_t* pMessage = (CO_Message_t*)event.value.p;
   CU_ASSERT_TRUE(pMessage != NULL);
   CU_ASSERT_EQUAL(pMessage->Type, CO_MESSAGE_PHY_DATA_REQ_E);
   FrameHeartbeat_t heartbeat;
   ErrorCode_t status = MC_PUP_UnpackHeartbeat((CO_MessagePayloadPhyDataInd_t*)&pMessage->Payload.PhyDataReq, &heartbeat);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(heartbeat.FrameType, FRAME_TYPE_HEARTBEAT_E);
   CU_ASSERT_EQUAL(heartbeat.SlotIndex, 16);
   CU_ASSERT_EQUAL(heartbeat.ShortFrameIndex, 1);
   CU_ASSERT_EQUAL(heartbeat.LongFrameIndex, 2);
   CU_ASSERT_EQUAL(heartbeat.SystemId, system_id)
}

/*************************************************************************************/
/**
* SM_RBUGenerateHeartbeatTest
* Test the RB heartbeat generation
*
* no params
*
* @return - none

*/
void SM_RBUGenerateHeartbeatTest(void)
{
   uint32_t system_id = SYSTEM_ID;
   uint16_t node_id = 1;
   uint16_t parent_node_id = 0;

   DCHQ = osMessageCreate(osMessageQ(DCHQ), NULL);

   SMInitialise(false, node_id, system_id);

   /* send a 'generate heartbeat' event.  No Heartbeat should be issued in CONNECTING state. */
   CO_Message_t msgGenHeartbeat;

   msgGenHeartbeat.Type = CO_MESSAGE_GENERATE_HEARTBEAT_E;
   msgGenHeartbeat.Payload.GenerateHeartbeat.SlotIndex = 17;
   msgGenHeartbeat.Payload.GenerateHeartbeat.ShortFrameIndex = 1;
   msgGenHeartbeat.Payload.GenerateHeartbeat.LongFrameIndex = 2;

   /*Check that no heartbeats are issued for the first MAX_MISSED_HEARTBEATS long frames */
   // removed for CYG2-916 wrong slot sync
   //for (int frame = 0; frame < MAX_MISSED_HEARTBEATS; frame++)
   //{
   //   SMHandleEvent(EVENT_GENERATE_HEARTBEAT_E, (uint8_t*)&msgGenHeartbeat);
   //   /*Check that a message was dropped into the DCH queue */
   //   uint32_t message_count = osMessageCount(DCHQ);
   //   CU_ASSERT_EQUAL(message_count, 0);
   //}

   /* The next EVENT_GENERATE_HEARTBEAT_E should result in a heartbeat being isued */
   SMHandleEvent(EVENT_GENERATE_HEARTBEAT_E, (uint8_t*)&msgGenHeartbeat);
   /*Check that a message was dropped into the DCH queue */
   uint32_t message_count = osMessageCount(DCHQ);
   CU_ASSERT_EQUAL(message_count, 1);


   /*Check that the message is a heartbeat */
   osEvent event;
   event = osMessageGet(DCHQ, 0);
   CU_ASSERT_EQUAL(osEventMessage, event.status);
   CO_Message_t* pMessage = (CO_Message_t*)event.value.p;
   CU_ASSERT_TRUE(pMessage != NULL);
   CU_ASSERT_EQUAL(pMessage->Type, CO_MESSAGE_PHY_DATA_REQ_E);
   FrameHeartbeat_t heartbeat;
   ErrorCode_t status = MC_PUP_UnpackHeartbeat((CO_MessagePayloadPhyDataInd_t*)&pMessage->Payload.PhyDataReq, &heartbeat);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(heartbeat.FrameType, FRAME_TYPE_HEARTBEAT_E);
   CU_ASSERT_EQUAL(heartbeat.SlotIndex, 17);
   CU_ASSERT_EQUAL(heartbeat.ShortFrameIndex, 1);
   CU_ASSERT_EQUAL(heartbeat.LongFrameIndex, 2);
   CU_ASSERT_EQUAL(heartbeat.SystemId, system_id);

   /* Move the state to OPERATIONAL */
   MoveRBUToOperationalState(node_id, parent_node_id, system_id);

   /* send another hearbeat request.  A heartbeat should be issued in OPERATIONAL state */
   SMHandleEvent(EVENT_GENERATE_HEARTBEAT_E, (uint8_t*)&msgGenHeartbeat);

   /*Check that a message was dropped into the DCH queue */
   message_count = osMessageCount(DCHQ);
   CU_ASSERT_EQUAL(message_count, 1);

   /*Check that the message is a heartbeat */
   event = osMessageGet(DCHQ, 0);
   CU_ASSERT_EQUAL(osEventMessage, event.status);
   pMessage = (CO_Message_t*)event.value.p;
   CU_ASSERT_TRUE(pMessage != NULL);
   CU_ASSERT_EQUAL(pMessage->Type, CO_MESSAGE_PHY_DATA_REQ_E);
   status = MC_PUP_UnpackHeartbeat((CO_MessagePayloadPhyDataInd_t*)&pMessage->Payload.PhyDataReq, &heartbeat);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(heartbeat.FrameType, FRAME_TYPE_HEARTBEAT_E);
   CU_ASSERT_EQUAL(heartbeat.SlotIndex, 17);
   CU_ASSERT_EQUAL(heartbeat.ShortFrameIndex, 1);
   CU_ASSERT_EQUAL(heartbeat.LongFrameIndex, 2);
   CU_ASSERT_EQUAL(heartbeat.SystemId, system_id);

}

/*************************************************************************************/
/**
* SM_NCUStateTransitionTest
* Test the NCU state transitions
*
* no params
*
* @return - none

*/
void SM_NCUStateTransitionTest(void)
{
   uint32_t system_id = SYSTEM_ID;

   DCHQ = osMessageCreate(osMessageQ(DCHQ), NULL);

   SMInitialise(true, 0, system_id);
   osPoolFlush(MeshPool);

   /*check for start up state of IDLE */
   CO_State_t state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(STATE_IDLE_E, state);

   /* set the state to CONFIG_SYNC */
   gSM_CurrentDeviceState = STATE_CONFIG_SYNC_E;

   /* send a 'generate heartbeat' event.  The state should change to OPERATIONAL. */
   CO_Message_t msgGenHeartbeat;

   msgGenHeartbeat.Type = CO_MESSAGE_GENERATE_HEARTBEAT_E;
   msgGenHeartbeat.Payload.GenerateHeartbeat.SlotIndex = 16;
   msgGenHeartbeat.Payload.GenerateHeartbeat.ShortFrameIndex = 1;
   msgGenHeartbeat.Payload.GenerateHeartbeat.LongFrameIndex = 2;

   SMHandleEvent(EVENT_GENERATE_HEARTBEAT_E, (uint8_t*)&msgGenHeartbeat);

   /*Check that a message was dropped into the DCH queue */
   uint32_t message_count = osMessageCount(DCHQ);
   CU_ASSERT_EQUAL(message_count, 1);
   /*Check that the message is a heartbeat */
   osEvent event;
   event = osMessageGet(DCHQ, 0);
   CU_ASSERT_EQUAL(osEventMessage, event.status);
   CO_Message_t* pMessage = (CO_Message_t*)event.value.p;
   CU_ASSERT_TRUE(pMessage != NULL);
   CU_ASSERT_EQUAL(pMessage->Type, CO_MESSAGE_PHY_DATA_REQ_E);
   FrameHeartbeat_t heartbeat;
   ErrorCode_t status = MC_PUP_UnpackHeartbeat((CO_MessagePayloadPhyDataInd_t*)&pMessage->Payload.PhyDataReq, &heartbeat);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(heartbeat.FrameType, FRAME_TYPE_HEARTBEAT_E);
   CU_ASSERT_EQUAL(heartbeat.SlotIndex, 16);
   CU_ASSERT_EQUAL(heartbeat.ShortFrameIndex, 1);
   CU_ASSERT_EQUAL(heartbeat.LongFrameIndex, 2);
   CU_ASSERT_EQUAL(heartbeat.SystemId, system_id);
}

/*************************************************************************************/
/**
* SM_NCUStateTransitionTest
* Test the NCU state transitions
*
* no params
*
* @return - none

*/
void SM_RBUStateTransitionTest(void)
{
   bool result = false;
   CO_Message_t LogonReq;
   CO_LogonData_t logonData;
   uint16_t rbu_node1_id = 1;
   uint16_t parent_id_1 = 0;
   uint16_t parent_slot_1 = 16;
   uint16_t parent_id_2 = 2;
   uint16_t parent_slot_2 = 18;
   uint32_t handle = 77;
   uint32_t system_id = SYSTEM_ID;

   osPoolFlush(MeshPool);
   osMessageQFlush(MeshQ);

   /* initialise the session manager */
   MC_SMGR_Initialise(false, rbu_node1_id);

   SMInitialise(false, rbu_node1_id, system_id);

   /*set up parents in the ack manager and session manager*/
   MC_SMGR_SetParentNodes(parent_slot_1, 0);


   /*check for start up state of CONNECTING */
   CO_State_t state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(STATE_CONFIG_SYNC_E, state);


   /* ACK the RouteAdd and Logon message and see if the state changes to OPERATIONAL*/

   ErrorCode_t ack_status = ERR_UNEXPECTED_ERROR_E;
   FrameAcknowledge_t FrameAcknowledgement;
   CO_Message_t AckMessage;
   gNetworkAddress = 1;
   FrameAcknowledgement.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   FrameAcknowledgement.MACDestinationAddress = gNetworkAddress;
   FrameAcknowledgement.MACSourceAddress = parent_id_1;

   // Test using the wrong system ID doesn't cause later problems.
   FrameAcknowledgement.SystemId = WRONG_SYSTEM_ID;// system_id;

   AckMessage.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   AckMessage.Payload.PhyDataInd.slotIdxInSuperframe = 14;
   AckMessage.Payload.PhyDataInd.slotIdxInLongframe = 14;
   AckMessage.Payload.PhyDataInd.slotIdxInShortframe = 14;
   ack_status = MC_PUP_PackAck(&FrameAcknowledgement, (CO_MessagePayloadPhyDataReq_t*)&AckMessage.Payload.PhyDataInd);
   CU_ASSERT_TRUE(SUCCESS_E == ack_status);
   if (SUCCESS_E == ack_status)
   {
      //LogonCnfCallback_Count = 0;
      //LogonCnfCallback_LastHandle = 0;
      //LogonCnfCallback_LastStatus = 0;
      SMHandleEvent(EVENT_ACK_SIGNAL_E, (uint8_t*)&AckMessage);  // This Ack message contains the wrong System ID, so will be ignored.
   }

   // Now test that using the right system ID gives the desired results.
   FrameAcknowledgement.SystemId = system_id;

   AckMessage.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   AckMessage.Payload.PhyDataInd.slotIdxInSuperframe = 14;
   AckMessage.Payload.PhyDataInd.slotIdxInLongframe = 14;
   AckMessage.Payload.PhyDataInd.slotIdxInShortframe = 14;
   ack_status = MC_PUP_PackAck(&FrameAcknowledgement, (CO_MessagePayloadPhyDataReq_t*)&AckMessage.Payload.PhyDataInd);
   CU_ASSERT_TRUE(SUCCESS_E == ack_status);
   if (SUCCESS_E == ack_status)
   {
      //LogonCnfCallback_Count = 0;
      //LogonCnfCallback_LastHandle = 0;
      //LogonCnfCallback_LastStatus = 0;

      SMHandleEvent(EVENT_ACK_SIGNAL_E, (uint8_t*)&AckMessage);
//      CU_ASSERT_EQUAL(LogonCnfCallback_Count, 0);

      // Need to set a Node ID for the tracking node.  We will set it to 0, which would be that of the NCU.
      gTrackingNode_id = 0;

      /*Send logon message*/
      /* create LogonReq message */
      logonData.Handle = handle;
      logonData.SerialNumber = 0;
      logonData.DeviceCombination = 0;
      logonData.ZoneNumber = 0;
      LogonReq.Type = CO_MESSAGE_GENERATE_LOGON_REQ;
      memcpy(&LogonReq.Payload.PhyDataReq.Data, &logonData, sizeof(CO_LogonData_t));
      //LogonCnfCallback_Count = 0;
      //LogonCnfCallback_LastHandle = 0;
      //LogonCnfCallback_LastStatus = 0;
      SMHandleEvent(EVENT_GENERATE_LOGON_REQ_E, (uint8_t*)&LogonReq);

      /* no call back means the message was queued OK */
//      CU_ASSERT_EQUAL(LogonCnfCallback_Count, 0);

      /* ack the logon message */
      SMHandleEvent(EVENT_ACK_SIGNAL_E, (uint8_t*)&AckMessage);
//      CU_ASSERT_EQUAL(LogonCnfCallback_Count, 1);
   }

   /* should still be be CONFIG */
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(STATE_CONFIG_SYNC_E, state);

   // move to ACTIVE state
   SM_SetCurrentDeviceState(STATE_ACTIVE_E);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(STATE_ACTIVE_E, state);

   /*Check that the state reverts to CONNECTING if the parents are lost */

   /*Create two parent sessions */
   MC_SMGR_Initialise(false, 1);
   MC_SMGR_SetParentNodes(parent_slot_1, parent_slot_2);
   /*send ACKs to make the parents active */
   SMHandleEvent(EVENT_ACK_SIGNAL_E, (uint8_t*)&AckMessage);
   FrameAcknowledgement.MACSourceAddress = parent_id_2;
   ack_status = MC_PUP_PackAck(&FrameAcknowledgement, (CO_MessagePayloadPhyDataReq_t*)&AckMessage.Payload.PhyDataInd);
   CU_ASSERT_TRUE(SUCCESS_E == ack_status);
   if (SUCCESS_E == ack_status)
   {
      SMHandleEvent(EVENT_ACK_SIGNAL_E, (uint8_t*)&AckMessage);
   }
   /* should be OPERATIONAL */
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(STATE_ACTIVE_E, state);
   /*check that we have both parents*/
   CU_ASSERT_TRUE(MC_SMGR_IsParentNode(parent_id_1));
   CU_ASSERT_TRUE(MC_SMGR_IsParentNode(parent_id_2));

   /*Start reporting missing heartbeats for parent 1*/
   bool response = false;
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(parent_id_1);
      CU_ASSERT_TRUE(response);
   }
   /* one more should remove the parent*/
   CO_Message_t MissedHbMsg;
   MissedHbMsg.Type = CO_MESSAGE_MAC_EVENT_E;
   MACEventMessage_t macEvent;
   macEvent.EventType = CO_MISSING_HEARTBEAT_E;
   macEvent.SuperframeSlotIndex = parent_slot_1;
   memcpy(MissedHbMsg.Payload.PhyDataInd.Data, &macEvent, sizeof(MACEventMessage_t));
   SMHandleEvent(EVENT_MAC_EVENT_E, (uint8_t*)&MissedHbMsg);


   /* The primary parent should have been removed*/
   CU_ASSERT_FALSE(MC_SMGR_IsParentNode(parent_id_1));

   /* we should still be operational because we have a secondary parent*/
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(STATE_ACTIVE_E, state);

   /*Start reporting missing heartbeats for parent 2*/
   for (uint16_t count = 1; count < MAX_MISSED_HEARTBEATS; count++)
   {
      response = MC_SMGR_MissedNodeHeartbeat(parent_id_2);
      CU_ASSERT_TRUE(response);
   }
   /* one more should remove the parent*/
   macEvent.SuperframeSlotIndex = parent_slot_2;
   memcpy(MissedHbMsg.Payload.PhyDataInd.Data, &macEvent, sizeof(MACEventMessage_t));
   SMHandleEvent(EVENT_MAC_EVENT_E, (uint8_t*)&MissedHbMsg);

   /* The secondary parent should have been removed*/
   CU_ASSERT_FALSE(MC_SMGR_IsParentNode(parent_id_2));
}

/*************************************************************************************/
/**
* SM_RBUReceivedHeartbeatTest
* Test the RBU's handling of received heartbeats
*
* no params
*
* @return - none

*/
void SM_RBUReceivedHeartbeatTest(void)
{
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   uint32_t system_id = SYSTEM_ID;

   SMInitialise(false, 1, system_id);//NCU
   MC_InitMeshFormAndHeal(1);
   MC_SMGR_Initialise(false, 1);

   /* create a heartbeat message */
   FrameHeartbeat_t heartbeat;
   heartbeat.FrameType = FRAME_TYPE_HEARTBEAT_E;
   heartbeat.LongFrameIndex = 1;  // Mustn't set this to 0, as State Machine will compare it to LongFrameIndexToStopSNRAveraging (value = 0) and will terminate the neighbour scan process.
   heartbeat.ShortFrameIndex = 0;
   heartbeat.SlotIndex = 19;
   heartbeat.Rank = 1;
   heartbeat.NoOfChildrenIdx = 1;
   heartbeat.NoOfChildrenOfPTNIdx = 1;
   heartbeat.State = 8;
   heartbeat.SystemId = system_id;

   MC_SMGR_SetParentNodes(heartbeat.SlotIndex, 0);

   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      status = MC_PUP_PackHeartbeat(&heartbeat, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(status, SUCCESS_E);

      if (SUCCESS_E == status)
      {
         pMessage->Payload.PhyDataInd.slotIdxInLongframe = heartbeat.SlotIndex;
         pMessage->Payload.PhyDataInd.RSSI = 0;
         pMessage->Payload.PhyDataInd.SNR = 10;
         pMessage->Payload.PhyDataInd.isInTDMSyncSlaveMode = true;

         // The SM initialises in state STATE_CONFIG_SYNC_E
         SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

         // check that the mesh forming state has incremented.
         CU_ASSERT_EQUAL(gMeshFormingStage, REFINING_INITIAL_TRACKING_NODE_E);

         // Change state to STATE_CONFIG_FORM_E and gMeshFormingStage to NEIGHBOUR_SCAN_E, then send another heartbeat
         gMeshFormingStage = NEIGHBOUR_SCAN_E;
         SM_SetCurrentDeviceState(STATE_CONFIG_FORM_E);
         MC_ScheduleMeshFormStages(1);
         SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

         //check that a node was added to the neighbour list and that the prelim tracking node was set
         CU_ASSERT_EQUAL(gMeshFormingStage, NEIGHBOUR_SCAN_E);
         CU_ASSERT_EQUAL(gPrelimTrackingSlot, 19);

         // Check that the number of missing heartbeats is 0.
         uint16_t NoOfMissedHeartbeats = GET_MISSED_HEARTBEATS(gParent[PRIMARY_PARENT].NodeInfo);
         CU_ASSERT_EQUAL(NoOfMissedHeartbeats, 0);

         // Send a heartbeat with the wrong system ID, so that the heartbeat is processed but fails the validation test.
         heartbeat.SystemId = system_id + 1;
         status = MC_PUP_PackHeartbeat(&heartbeat, &pMessage->Payload.PhyDataReq);
         SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

         // Check that the number of missing heartbeats is 1.
         NoOfMissedHeartbeats = GET_MISSED_HEARTBEATS(gParent[PRIMARY_PARENT].NodeInfo);
         CU_ASSERT_EQUAL(NoOfMissedHeartbeats, 1);

         // Restore the packed heartbeat to have the correct System ID.
         heartbeat.SystemId = system_id;
         status = MC_PUP_PackHeartbeat(&heartbeat, &pMessage->Payload.PhyDataReq);

         // Corrupt the data field, then send another heartbeat.  Check that the neighbour list hit count remains unchanged.  Then restore the slot index.
         uint8_t Data2 = pMessage->Payload.PhyDataInd.Data[2];
         uint8_t Data3 = pMessage->Payload.PhyDataInd.Data[3];
         uint8_t Data4 = pMessage->Payload.PhyDataInd.Data[4];
         uint8_t Data5 = pMessage->Payload.PhyDataInd.Data[5];
         uint8_t Data6 = pMessage->Payload.PhyDataInd.Data[6];
         uint8_t Data7 = pMessage->Payload.PhyDataInd.Data[7];
         pMessage->Payload.PhyDataInd.Data[2] = 0;
         pMessage->Payload.PhyDataInd.Data[3] = 0;
         pMessage->Payload.PhyDataInd.Data[4] = 0;
         pMessage->Payload.PhyDataInd.Data[5] = 0;
         pMessage->Payload.PhyDataInd.Data[6] = 0;
         pMessage->Payload.PhyDataInd.Data[7] = 0;

         // Send the message with the corrupted data field.  This will cause the code to fail the validity test.
         SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

         // Check that the number of missing heartbeats is 2.
         NoOfMissedHeartbeats = GET_MISSED_HEARTBEATS(gParent[PRIMARY_PARENT].NodeInfo);
         CU_ASSERT_EQUAL(NoOfMissedHeartbeats, 2);

         // Restore the data to the correct values.
         pMessage->Payload.PhyDataInd.Data[2] = Data2;
         pMessage->Payload.PhyDataInd.Data[3] = Data3;
         pMessage->Payload.PhyDataInd.Data[4] = Data4;
         pMessage->Payload.PhyDataInd.Data[5] = Data5;
         pMessage->Payload.PhyDataInd.Data[6] = Data6;
         pMessage->Payload.PhyDataInd.Data[7] = Data7;

         // Send a good heartbeat. Check that the number of missed heartbeats goes back to 0
         SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);
         NoOfMissedHeartbeats = GET_MISSED_HEARTBEATS(gParent[PRIMARY_PARENT].NodeInfo);
         CU_ASSERT_EQUAL(NoOfMissedHeartbeats, 0);

         // Corrupt the system ID in the heartbeat payload, then send another heartbeat.  Check that the neighbour list hit count remains unchanged.  Then restore the system ID.
         heartbeat.SystemId = system_id + 1;
         MC_PUP_PackHeartbeat(&heartbeat, &pMessage->Payload.PhyDataReq);
         SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);
         heartbeat.SystemId = system_id;
         MC_PUP_PackHeartbeat(&heartbeat, &pMessage->Payload.PhyDataReq);

         // Check that the number of missing heartbeats is 1.
         NoOfMissedHeartbeats = GET_MISSED_HEARTBEATS(gParent[PRIMARY_PARENT].NodeInfo);
         CU_ASSERT_EQUAL(NoOfMissedHeartbeats, 1);

         // Send a missing heartbeat event.
         uint8_t parent_slot_1 = heartbeat.SlotIndex;
         CO_Message_t MissedHbMsg;
         MissedHbMsg.Type = CO_MESSAGE_MAC_EVENT_E;
         MACEventMessage_t macEvent;
         macEvent.EventType = CO_MISSING_HEARTBEAT_E;
         macEvent.SuperframeSlotIndex = parent_slot_1;
         macEvent.SlotIndex = parent_slot_1;
         macEvent.ShortFrameIndex = 0;
         memcpy(MissedHbMsg.Payload.PhyDataInd.Data, &macEvent, sizeof(MACEventMessage_t));
         SMHandleEvent(EVENT_MAC_EVENT_E, (uint8_t*)&MissedHbMsg);

         // Check that the number of missing heartbeats is 2, and that the neighbour list hit count remains unchanged.
         NoOfMissedHeartbeats = GET_MISSED_HEARTBEATS(gParent[PRIMARY_PARENT].NodeInfo);
         CU_ASSERT_EQUAL(NoOfMissedHeartbeats, 2);

         // Set the header slot indicies - necessary since device will now have entered TDM Sync Slave mode.
         pMessage->Payload.PhyDataInd.slotIdxInSuperframe = 19;
         pMessage->Payload.PhyDataInd.slotIdxInLongframe = 19;
         pMessage->Payload.PhyDataInd.slotIdxInShortframe = 19;

         //Send the heartbeat again.  The neighbour hit count should increase to 2, and the number of missing heartbeats should have been reset to 0.
         SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);
         NoOfMissedHeartbeats = GET_MISSED_HEARTBEATS(gParent[PRIMARY_PARENT].NodeInfo);
         CU_ASSERT_EQUAL(NoOfMissedHeartbeats, 0);

         //Check that the session manager is updated.
         //We can do that by setting up a parent with a missed heartbeat then testing
         //whether the next heatbeat cancels the missed heatbeat count.
         gParent[0].NodeInfo = 19;
         SET_MISSED_HEARTBEATS(gParent[0].NodeInfo, 1);
         //test the set up
         uint32_t missed_heartbeats = GET_MISSED_HEARTBEATS(gParent[0].NodeInfo);
         CU_ASSERT_EQUAL(missed_heartbeats, 1);
         //Send the heartbeat again.  The neighbour hit count should increase to 3
         SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);
         //check that the missed heartbeat count was reset
         missed_heartbeats = GET_MISSED_HEARTBEATS(gParent[0].NodeInfo);
         
         /* check the Reset action when parents are at a lower state than this node */
         bErrorHandlerCalled = false;
         MC_ResetSyncDatabase();/*reset the sync database to avoid the framelength Error_Hander call.*/
         gSM_ScheduledDeviceState = STATE_NOT_DEFINED_E;
         gSM_CurrentDeviceState = STATE_ACTIVE_E;
         gParent[PRIMARY_PARENT].NodeInfo = 1;
         gParent[PRIMARY_PARENT].state = STATE_CONFIG_FORM_E;
         gParent[PRIMARY_PARENT].IsActive = true;
         gParent[SECONDARY_PARENT].NodeInfo = 2;
         gParent[SECONDARY_PARENT].state = STATE_CONFIG_FORM_E;
         gParent[SECONDARY_PARENT].IsActive = true;
         heartbeat.State = (uint32_t)STATE_CONFIG_FORM_E;
         heartbeat.LongFrameIndex = 1;
         status = MC_PUP_PackHeartbeat(&heartbeat, &pMessage->Payload.PhyDataReq);
         CU_ASSERT_EQUAL(status, SUCCESS_E);
      }
   }
}

/*************************************************************************************/
/**
* SM_NCUGenerateOutputSignalTest
* Test the NCU Generate Ouput Signal function
*
* no params
*
* @return - none

*/
void SM_NCUGenerateOutputSignalTest(void)
{
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   const uint8_t NCU_DCH_SLOT = 16;
   uint32_t dlcchpq_count;
   uint32_t system_id = SYSTEM_ID;

   DLCCHPQ = osMessageCreate(osMessageQ(DLCCHPQ), NULL);
   DLCCHSQ = osMessageCreate(osMessageQ(DLCCHSQ), NULL);
   DCHQ = osMessageCreate(osMessageQ(DCHQ), NULL);

   osPoolFlush(MeshPool);
   osMessageQFlush(DLCCHPQ);
   osMessageQFlush(DLCCHSQ);
   osMessageQFlush(DCHQ);

   dlcchpq_count = osMessageCount(DLCCHPQ);
   CU_ASSERT_EQUAL(dlcchpq_count, 0);

   SMInitialise(true, 0, system_id);//NCU
   /* set the state to CONFIG_SYNC */
   gSM_CurrentDeviceState = STATE_CONFIG_SYNC_E;

   /* create a Generate Output signal message */
   CO_OutputData_t outData;
   outData.Handle = 1;
   outData.Destination = 2;
   outData.Source = 0;
   outData.zone = 1;
   outData.OutputProfile = CO_PROFILE_FIRE_E;
   outData.OutputsActivated = 4;
   outData.NumberToSend = 3;

   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      memcpy(pMessage->Payload.PhyDataReq.Data, &outData, sizeof(CO_OutputData_t));
      pMessage->Payload.PhyDataReq.Size = sizeof(CO_OutputData_t);

      //The state machine should be in the STATE_CONFIG_E state
      CO_State_t state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

      SMHandleEvent(EVENT_GENERATE_OUTPUT_SIGNAL_E, (uint8_t *)pMessage);

      //There should be no output when in state STATE_CONFIG_E
      dlcchpq_count = osMessageCount(DLCCHPQ);
      CU_ASSERT_EQUAL(dlcchpq_count, 0);

      //change the state to STATE_ACTIVE_E
      SM_SetCurrentDeviceState(STATE_ACTIVE_E);

      //The state machine should be in the STATE_ACTIVE_E state
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

      //Send the output signal again
      SMHandleEvent(EVENT_GENERATE_OUTPUT_SIGNAL_E, (uint8_t *)pMessage);

      //This time 3 messages should be in DLCCHPQ
      dlcchpq_count = osMessageCount(DLCCHPQ);
      CU_ASSERT_EQUAL(dlcchpq_count, 3);

      /* Check that non-fire signals go to DLCCHSQ */
      outData.OutputProfile = CO_PROFILE_FIRST_AID_E;
      memcpy(pMessage->Payload.PhyDataReq.Data, &outData, sizeof(CO_OutputData_t));
      SMHandleEvent(EVENT_GENERATE_OUTPUT_SIGNAL_E, (uint8_t *)pMessage);
      //This time 3 messages should be in DLCCHSQ
      dlcchpq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchpq_count, 3);
   }
}

/*************************************************************************************/
/**
* SM_RBUGenerateOutputSignalTest
* Test the RBU does not generate Ouput Signals in either state
*
* no params
*
* @return - none

*/
void SM_RBUGenerateOutputSignalTest(void)
{
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   const uint16_t NCU_DCH_SLOT = 16;
   uint32_t dlcchpq_count;
   uint16_t rbu_node_id = 1;
   uint16_t parent_node_id = 0;
   uint32_t system_id = SYSTEM_ID;

   osPoolFlush(MeshPool);
   osMessageQFlush(DLCCHPQ);
   osMessageQFlush(DLCCHSQ);
   osMessageQFlush(DCHQ);

   dlcchpq_count = osMessageCount(DLCCHPQ);
   CU_ASSERT_EQUAL(dlcchpq_count, 0);

   /* initialise the session manager */
   MC_SMGR_Initialise(false, rbu_node_id);

   SMInitialise(false, rbu_node_id, system_id);//RBU


   /* create an output signal message */
   CO_OutputData_t outData;
   outData.Handle = 1;
   outData.Destination = 2;
   outData.OutputProfile = CO_PROFILE_FIRE_E;
   outData.OutputsActivated = 4;

   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      memcpy(pMessage->Payload.PhyDataReq.Data, &outData, sizeof(CO_OutputData_t));
      pMessage->Payload.PhyDataReq.Size = sizeof(CO_OutputData_t);

      //The state machine should be in the STATE_CONFIG_E state
      CO_State_t state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

      SMHandleEvent(EVENT_GENERATE_OUTPUT_SIGNAL_E, (uint8_t *)pMessage);

      //There should be no output when in state STATE_CONFIG_E
      dlcchpq_count = osMessageCount(DLCCHPQ);
      CU_ASSERT_EQUAL(dlcchpq_count, 0);

      // Now change the state to STATE_ACTIVE_E by calling with the correct system ID
      MoveRBUToOperationalState(rbu_node_id, parent_node_id, system_id);

      //The state machine should be in the STATE_ACTIVE_E state
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

      //Send the Generate Output signal again
      SMHandleEvent(EVENT_GENERATE_OUTPUT_SIGNAL_E, (uint8_t *)pMessage);

      //No output message should have been issued
      dlcchpq_count = osMessageCount(DLCCHPQ);
      CU_ASSERT_EQUAL(dlcchpq_count, 0);
   }
}

/*************************************************************************************/
/**
* SM_NCUReceiveOutputSignalTest
* Test the NCU response to received output signals.
* NCUs don't use output messages so shouldn't act upon them
*
* no params
*
* @return - none

*/
void SM_NCUReceiveOutputSignalTest(void)
{
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   const uint8_t NCU_DCH_SLOT = 16;
   uint32_t dlcchpq_count;
   uint32_t dlcchsq_count;
   uint16_t rbu_node_id = 1;
   uint16_t parent_node_id = 0;
   uint32_t system_id = SYSTEM_ID;

   osPoolFlush(MeshPool);
   osMessageQFlush(DLCCHPQ);
   osMessageQFlush(DLCCHSQ);
   osMessageQFlush(DCHQ);



   dlcchpq_count = osMessageCount(DLCCHPQ);
   CU_ASSERT_EQUAL(dlcchpq_count, 0);
   dlcchsq_count = osMessageCount(DLCCHSQ);
   CU_ASSERT_EQUAL(dlcchsq_count, 0);

   SMInitialise(true, 0, system_id);//NCU
   CU_ASSERT_EQUAL(gSM_CurrentDeviceState, STATE_IDLE_E);
   /* set the state to CONFIG_SYNC */
   gSM_CurrentDeviceState = STATE_CONFIG_SYNC_E;


   /* create an output signal message */
   OutputSignal_t outputsignal;
   outputsignal.Header.FrameType = FRAME_TYPE_DATA_E;
   outputsignal.Header.MACDestinationAddress = rbu_node_id;
   outputsignal.Header.MACSourceAddress = 2;
   outputsignal.Header.HopCount = 0;
   outputsignal.Header.DestinationAddress = rbu_node_id;
   outputsignal.Header.SourceAddress = 2;
   outputsignal.Header.MessageType = APP_MSG_TYPE_OUTPUT_SIGNAL_E;
   outputsignal.Zone = 1;
   outputsignal.OutputProfile = CO_PROFILE_FIRE_E;
   outputsignal.OutputsActivated = 3;
   outputsignal.OutputDuration = 0;
   outputsignal.SystemId = system_id;

   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      ErrorCode_t result = MC_PUP_PackOutputSignal(&outputsignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      //The state machine should be in the STATE_CONFIG_E state
      CO_State_t state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

      SMHandleEvent(EVENT_OUTPUT_SIGNAL_E, (uint8_t *)pMessage);

      //There should be no output when in state STATE_CONFIG_E
      dlcchpq_count = osMessageCount(DLCCHPQ);
      CU_ASSERT_EQUAL(dlcchpq_count, 0);

      //Generate a heartbeat to change the state to STATE_ACTIVE_E
      SM_SetCurrentDeviceState(STATE_ACTIVE_E);

      //The state machine should be in the STATE_ACTIVE_E state
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

      //Send the Output signal again
      SMHandleEvent(EVENT_OUTPUT_SIGNAL_E, (uint8_t *)pMessage);

      //There should still be no message issued
      dlcchpq_count = osMessageCount(DLCCHPQ);
      CU_ASSERT_EQUAL(dlcchpq_count, 0);

      /*check that non-fire alarms are not actioned*/
      outputsignal.OutputProfile = CO_PROFILE_FIRST_AID_E;
      result = MC_PUP_PackOutputSignal(&outputsignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);
      SMHandleEvent(EVENT_OUTPUT_SIGNAL_E, (uint8_t *)pMessage);
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, 0);


      /*Now check what happens when the output signal is addressed to this NCU
      There should be no message in the AppQ*/
      osMessageQFlush(AppQ);
      outputsignal.OutputProfile = CO_PROFILE_FIRE_E;
      outputsignal.Header.DestinationAddress = ADDRESS_NCU;
      result = MC_PUP_PackOutputSignal(&outputsignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);
      SMHandleEvent(EVENT_OUTPUT_SIGNAL_E, (uint8_t *)pMessage);
      uint32_t msg_count = osMessageCount(AppQ);
      CU_ASSERT_EQUAL(msg_count, 0);

   }
}

/*************************************************************************************/
/**
* SM_RBUReceiveOutputSignalTest
* Test the RBU receive of output signals
*
* no params
*
* @return - none

*/
void SM_RBUReceiveOutputSignalTest(void)
{
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   const uint16_t NCU_DCH_SLOT = 16;
   uint32_t dlcchpq_count;
   uint32_t dlcchsq_count;
   uint16_t rbu_node_id = 1;
   uint16_t parent_node_id = 0;
   uint32_t system_id = SYSTEM_ID;

   osPoolFlush(MeshPool);
   osPoolFlush(AppPool);
   osMessageQFlush(DLCCHPQ);
   osMessageQFlush(DLCCHSQ);
   osMessageQFlush(DCHQ);

   dlcchpq_count = osMessageCount(DLCCHPQ);
   CU_ASSERT_EQUAL(dlcchpq_count, 0);

   /* initialise the session manager */
   MC_SMGR_Initialise(false, rbu_node_id);
   gParent[PRIMARY_PARENT].NodeInfo = parent_node_id;
   gParent[PRIMARY_PARENT].IsActive = true;
   MC_SMGR_RouteAddRequest(7,0);

   SMInitialise(false, rbu_node_id, system_id);//RBU

   /* create an output signal message */
   OutputSignal_t outputsignal;
   outputsignal.Header.FrameType = FRAME_TYPE_DATA_E;
   outputsignal.Header.MACDestinationAddress = rbu_node_id;
   outputsignal.Header.MACSourceAddress = parent_node_id;
   outputsignal.Header.HopCount = 0;
   outputsignal.Header.DestinationAddress = 2;
   outputsignal.Header.SourceAddress = parent_node_id;
   outputsignal.Header.MessageType = APP_MSG_TYPE_OUTPUT_SIGNAL_E;
   outputsignal.Zone = 1;
   outputsignal.OutputProfile = CO_PROFILE_FIRE_E;
   outputsignal.OutputDuration = 0;
   outputsignal.OutputsActivated = 3;

   // Test the behaviour if using the wrong system ID for the output signal
   outputsignal.SystemId = WRONG_SYSTEM_ID;

   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      ErrorCode_t result = MC_PUP_PackOutputSignal(&outputsignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      //The state machine should be in the STATE_CONFIG_E state
      CO_State_t state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

      SMHandleEvent(EVENT_OUTPUT_SIGNAL_E, (uint8_t *)pMessage);

      //There should be no output when in state STATE_CONFIG_E
      dlcchpq_count = osMessageCount(DLCCHPQ);
      CU_ASSERT_EQUAL(dlcchpq_count, 0);

      // Change the state to STATE_ACTIVE_E.
      MoveRBUToOperationalState(rbu_node_id, parent_node_id, system_id);

      //The state machine should be in the STATE_ACTIVE_E state
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

      //Send the Output signal again
      SMHandleEvent(EVENT_OUTPUT_SIGNAL_E, (uint8_t *)pMessage);

      // The message was addressed to another node so it should be re-broadcast.
      // Because is is a FIRE output it should be sent on DLCCHPQ.
      // However, we expect the message count to not increase because the system ID is wrong.
      dlcchpq_count = osMessageCount(DLCCHPQ);
      CU_ASSERT_EQUAL(dlcchpq_count, 0);
   }

   // Now test the behaviour if using the correct system ID for the output signal
   outputsignal.SystemId = system_id;

   pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      ErrorCode_t result = MC_PUP_PackOutputSignal(&outputsignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      //The state machine should be in the STATE_ACTIVE_E state
      CO_State_t state = SM_GetCurrentDeviceState();
       CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

      //Send the Output signal again
      SMHandleEvent(EVENT_OUTPUT_SIGNAL_E, (uint8_t *)pMessage);

      //The message was addressed to another node so it should be re-broadcast.
      //Because is is a FIRE output it should be sent on DLCCHPQ.
      dlcchpq_count = osMessageCount(DLCCHPQ);
      CU_ASSERT_EQUAL(dlcchpq_count, 1);

      /*check that non-fire alarms are sent to DLCCHSQ*/
      outputsignal.OutputProfile = CO_PROFILE_FIRST_AID_E;
      result = MC_PUP_PackOutputSignal(&outputsignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);
      SMHandleEvent(EVENT_OUTPUT_SIGNAL_E, (uint8_t *)pMessage);
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, 1);


      /*Now check what happens when the output signal is addressed to this RBU
         We should get a message in the AppQ */
      osMessageQFlush(AppQ);
      outputsignal.OutputProfile = CO_PROFILE_FIRE_E;
      outputsignal.Header.DestinationAddress = rbu_node_id;
      result = MC_PUP_PackOutputSignal(&outputsignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);
      SMHandleEvent(EVENT_OUTPUT_SIGNAL_E, (uint8_t *)pMessage);
      uint32_t msg_count = osMessageCount(AppQ);
      CU_ASSERT_EQUAL(msg_count, 1);
   }
}

/*************************************************************************************/
/**
* SM_RBUReceiveFireSignalTest
* Test the RBU receive of fire signal messages.
* Fire Signal Messages should only be accepted in OPERATIONAL state and should always be
* repeated uplink towards the NCU, except when the hop count limit is reached.
*
* no params
*
* @return - none

*/
void SM_RBUReceiveFireSignalTest(void)
{
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   const uint16_t NCU_DCH_SLOT = 16;
   uint32_t rachpq_count;
   uint16_t rbu_node1_id = 1;
   uint16_t rbu_node2_id = 2;
   uint16_t parent_node_id = 0;
   uint32_t system_id = SYSTEM_ID;
   uint16_t primary_parent_slot = 2596;
   uint16_t secondary_parent_slot = 2597;

   RACHPQ = osMessageCreate(osMessageQ(RACHPQ), NULL);

   osPoolFlush(MeshPool);
   osMessageQFlush(RACHPQ);

   rachpq_count = osMessageCount(RACHPQ);
   CU_ASSERT_EQUAL(rachpq_count, 0);

   SMInitialise(false, rbu_node1_id, system_id);//RBU

   /* initialise the session manager */
   MC_SMGR_Initialise(false, rbu_node1_id);
   MC_SMGR_SetParentNodes(primary_parent_slot, secondary_parent_slot);
   gParent[PRIMARY_PARENT].IsActive = true;
   /* add a child */
   MC_SMGR_RouteAddRequest(rbu_node2_id, 0);
   CU_ASSERT_TRUE(MC_SMGR_IsAChild(rbu_node2_id));


   /* create fire signal */
   AlarmSignal_t fireSignal;
   fireSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   fireSignal.Header.MACDestinationAddress = rbu_node1_id;
   fireSignal.Header.MACSourceAddress = rbu_node2_id;
   fireSignal.Header.HopCount = 0;
   fireSignal.Header.DestinationAddress = ADDRESS_NCU;
   fireSignal.Header.SourceAddress = rbu_node2_id;
   fireSignal.Header.MessageType = APP_MSG_TYPE_FIRE_SIGNAL_E;
   fireSignal.RUChannel = 0;
   fireSignal.SensorValue = 0;

   // Test the behaviour if using the wrong system ID for the output signal
   fireSignal.SystemId = WRONG_SYSTEM_ID;

   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      ErrorCode_t result = MC_PUP_PackAlarmSignal(&fireSignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      //The state machine should be in the STATE_CONFIG_E state
      CO_State_t state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

      SMHandleEvent(EVENT_FIRE_SIGNAL_E, (uint8_t *)pMessage);

      //There should be no output when in state STATE_CONFIG_E
      rachpq_count = osMessageCount(RACHPQ);
      CU_ASSERT_EQUAL(rachpq_count, 0);

      //Change the state to STATE_ACTIVE_E
      MoveRBUToOperationalState(rbu_node1_id, parent_node_id, system_id);

      //The state machine should be in the STATE_ACTIVE_E state
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

      //Send the fire signal again
      SMHandleEvent(EVENT_FIRE_SIGNAL_E, (uint8_t *)pMessage);

      //The message was addressed to another node so it should be re-broadcast.
      // However, we expect the rach message counter to not increment as we are using the wrong system ID
      rachpq_count = osMessageCount(RACHPQ);
      CU_ASSERT_EQUAL(rachpq_count, 0);

      //Repeat fire msg with Hop Count = MAX_MSG_HOP_THRESHOLD - 1.
      // The message should not be sent uplink
      fireSignal.Header.HopCount = MAX_MSG_HOP_THRESHOLD - 1;
      result = MC_PUP_PackAlarmSignal(&fireSignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      osMessageQFlush(RACHPQ);
      rachpq_count = osMessageCount(RACHPQ);
      CU_ASSERT_EQUAL(rachpq_count, 0);

      //Send the fire signal again
      SMHandleEvent(EVENT_FIRE_SIGNAL_E, (uint8_t *)pMessage);
      //The message hop limit was reached so it should not be re-broadcast.
      rachpq_count = osMessageCount(RACHPQ);
      CU_ASSERT_EQUAL(rachpq_count, 0);
   }

   // Now test the behaviour if using the correct system ID for the output signal
   fireSignal.SystemId = system_id;
   fireSignal.Header.HopCount = 0;

   pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      ErrorCode_t result = MC_PUP_PackAlarmSignal(&fireSignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      //The state machine should be in the STATE_ACTIVE_E state
      CO_State_t state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

      //Send the fire signal again
      SMHandleEvent(EVENT_FIRE_SIGNAL_E, (uint8_t *)pMessage);

      //The message was addressed to another node so it should be queued for re-broadcast.
      rachpq_count = osMessageCount(RACHPQ);
      CU_ASSERT_EQUAL(rachpq_count, 1);

      /* check the msg type*/
      ApplicationLayerMessageType_t msg_type;
      uint32_t handle;
      MC_ACK_AcknowledgeMessage(ACK_RACHP_E, ADDRESS_NCU, true, &msg_type, &handle);
      CU_ASSERT_EQUAL(msg_type, APP_MSG_TYPE_FIRE_SIGNAL_E);

      //Repeat fire msg with Hop Count = MAX_MSG_HOP_THRESHOLD - 1.
      // The message should not be sent uplink
      fireSignal.Header.HopCount = MAX_MSG_HOP_THRESHOLD - 1;
      result = MC_PUP_PackAlarmSignal(&fireSignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      //Send the fire signal again
      SMHandleEvent(EVENT_FIRE_SIGNAL_E, (uint8_t *)pMessage);
      //The message hop limit was reached so it should not be re-broadcast.
      bool ready_to_send = MC_ACK_MessageReadyToSend(ACK_RACHP_E);
      CU_ASSERT_FALSE(ready_to_send);
   }
}

/*************************************************************************************/
/**
* SM_RBUReceiveAlarmSignalTest
* Test the RBU receive of alarm signal messages.
* Alarm Signal Messages should only be accepted in OPERATIONAL state and should always be
* repeated uplink towards the NCU, except when the hop count limit is reached.
*
* no params
*
* @return - none

*/
void SM_RBUReceiveAlarmSignalTest(void)
{
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   const uint16_t NCU_DCH_SLOT = 16;
   uint32_t rachsq_count;
   uint16_t rbu_node1_id = 1;
   uint16_t rbu_node2_id = 2;
   uint16_t parent_node_id = 0;
   uint32_t system_id = SYSTEM_ID;
   uint16_t primary_parent_slot = 2596;
   uint16_t secondary_parent_slot = 2597;

   osPoolFlush(MeshPool);
   osMessageQFlush(RACHSQ);

   rachsq_count = osMessageCount(RACHSQ);
   CU_ASSERT_EQUAL(rachsq_count, 0);

   SMInitialise(false, rbu_node1_id, system_id);//RBU

   /* initialise the session manager */
   MC_SMGR_Initialise(false, rbu_node1_id);
   MC_SMGR_SetParentNodes(primary_parent_slot, secondary_parent_slot);
   gParent[PRIMARY_PARENT].IsActive = true;
   MC_SMGR_RouteAddRequest(rbu_node2_id, 0);
   CU_ASSERT_TRUE(MC_SMGR_IsAChild(rbu_node2_id));


   /* create alarm message */
   AlarmSignal_t AlarmSignal;
   AlarmSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   AlarmSignal.Header.MACDestinationAddress = rbu_node1_id;
   AlarmSignal.Header.MACSourceAddress = rbu_node2_id;
   AlarmSignal.Header.HopCount = 0;
   AlarmSignal.Header.DestinationAddress = ADDRESS_NCU;
   AlarmSignal.Header.SourceAddress = rbu_node2_id;
   AlarmSignal.Header.MessageType = APP_MSG_TYPE_ALARM_SIGNAL_E;
   AlarmSignal.RUChannel = 0;
   AlarmSignal.SensorValue = 0;

   // Test the behaviour if using the wrong system ID for the output signal
   AlarmSignal.SystemId = WRONG_SYSTEM_ID;//system_id;

   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      ErrorCode_t result = MC_PUP_PackAlarmSignal(&AlarmSignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      //The state machine should be in the STATE_CONFIG_E state
      CO_State_t state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

      SMHandleEvent(EVENT_ALARM_SIGNAL_E, (uint8_t *)pMessage);

      //There should be no output when in state STATE_CONFIG_E
      rachsq_count = osMessageCount(RACHSQ);
      CU_ASSERT_EQUAL(rachsq_count, 0);

      //Change the state to STATE_ACTIVE_E
      MoveRBUToOperationalState(rbu_node1_id, parent_node_id, system_id);

      //The state machine should be in the STATE_ACTIVE_E state
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

      //Send the alarm signal again
      SMHandleEvent(EVENT_ALARM_SIGNAL_E, (uint8_t *)pMessage);

      //The message was addressed to another node so it should be re-broadcast.
      // However, the rach counter should not increment because we are sending it with the wrong system ID
      rachsq_count = osMessageCount(RACHSQ);
      CU_ASSERT_EQUAL(rachsq_count, 0);

      //Repeat alarm msg with Hop Count = MAX_MSG_HOP_THRESHOLD - 1.
      // The message should not be sent uplink
      AlarmSignal.Header.HopCount = MAX_MSG_HOP_THRESHOLD - 1;
      result = MC_PUP_PackAlarmSignal(&AlarmSignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      osMessageQFlush(RACHSQ);
      rachsq_count = osMessageCount(RACHSQ);
      CU_ASSERT_EQUAL(rachsq_count, 0);

      //Send the alarm signal again
      SMHandleEvent(EVENT_ALARM_SIGNAL_E, (uint8_t *)pMessage);
      //The message hop limit was reached so it should not be re-broadcast.
      rachsq_count = osMessageCount(RACHSQ);
      CU_ASSERT_EQUAL(rachsq_count, 0);
   }

   // Now test the behaviour if using the correct system ID for the output signal
   AlarmSignal.SystemId = system_id;
   AlarmSignal.Header.HopCount = 0;

   pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      ErrorCode_t result = MC_PUP_PackAlarmSignal(&AlarmSignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      //Send the alarm signal
      SMHandleEvent(EVENT_ALARM_SIGNAL_E, (uint8_t *)pMessage);

      //The message was addressed to another node so it should be queued for re-broadcast.
      rachsq_count = osMessageCount(RACHSQ);
      CU_ASSERT_EQUAL(rachsq_count, 1);

      /* check the msg type*/
      ApplicationLayerMessageType_t msg_type;
      uint32_t handle;
      MC_ACK_AcknowledgeMessage(ACK_RACHS_E, ADDRESS_NCU, true, &msg_type, &handle);
      CU_ASSERT_EQUAL(msg_type, APP_MSG_TYPE_ALARM_SIGNAL_E);

      //Repeat alarm msg with Hop Count = MAX_MSG_HOP_THRESHOLD - 1.
      // The message should not be sent uplink
      AlarmSignal.Header.HopCount = MAX_MSG_HOP_THRESHOLD - 1;
      result = MC_PUP_PackAlarmSignal(&AlarmSignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      //Send the alarm signal again
      SMHandleEvent(EVENT_ALARM_SIGNAL_E, (uint8_t *)pMessage);
      //The message hop limit was reached so it should not be re-broadcast.
      bool ready_to_send = MC_ACK_MessageReadyToSend(ACK_RACHS_E);
      CU_ASSERT_FALSE(ready_to_send);
   }
}

/*************************************************************************************/
/**
* SM_RBUReceiveFaultSignalTest
* Test the RBU receive of fault signal messages.
* Fault Signal Messages should only be accepted in OPERATIONAL state and should always be
* repeated uplink towards the NCU, except when the hop count limit is reached.
*
* no params
*
* @return - none

*/
void SM_RBUReceiveFaultSignalTest(void)
{
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   const uint16_t NCU_DCH_SLOT = 16;
   uint32_t rachsq_count;
   uint16_t rbu_node1_id = 1;
   uint16_t rbu_node2_id = 2;
   uint16_t parent_node_id = 0;
   uint32_t system_id = SYSTEM_ID;
   uint16_t primary_parent_slot = 2596;
   uint16_t secondary_parent_slot = 2597;

   osPoolFlush(MeshPool);
   osMessageQFlush(RACHSQ);

   rachsq_count = osMessageCount(RACHSQ);
   CU_ASSERT_EQUAL(rachsq_count, 0);

   SMInitialise(false, rbu_node1_id, system_id);//RBU

   /* initialise the session manager */
   MC_SMGR_Initialise(false, rbu_node1_id);
   MC_SMGR_SetParentNodes(primary_parent_slot, secondary_parent_slot);
   gParent[PRIMARY_PARENT].IsActive = true;
   MC_SMGR_RouteAddRequest(rbu_node2_id, 0);
   CU_ASSERT_TRUE(MC_SMGR_IsAChild(rbu_node2_id));


   /* create fault message */
   FaultSignal_t faultSignal;
   faultSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   faultSignal.Header.MACDestinationAddress = rbu_node1_id;
   faultSignal.Header.MACSourceAddress = rbu_node2_id;
   faultSignal.Header.HopCount = 0;
   faultSignal.Header.DestinationAddress = ADDRESS_NCU;
   faultSignal.Header.SourceAddress = rbu_node2_id;
   faultSignal.Header.MessageType = APP_MSG_TYPE_FAULT_SIGNAL_E;
   faultSignal.Zone = 1;
   faultSignal.StatusFlags = 0;
   faultSignal.DetectorFault = 0;
   faultSignal.BeaconFault = 0;
   faultSignal.SounderFault = 0;

   // Test the behaviour if using the wrong system ID for the output signal
   faultSignal.SystemId = WRONG_SYSTEM_ID;//system_id;

   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      ErrorCode_t result = MC_PUP_PackFaultSignal(&faultSignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      //The state machine should be in the STATE_CONFIG_E state
      CO_State_t state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

      SMHandleEvent(EVENT_FAULT_SIGNAL_E, (uint8_t *)pMessage);

      //There should be no output when in state STATE_CONFIG_E
      rachsq_count = osMessageCount(RACHSQ);
      CU_ASSERT_EQUAL(rachsq_count, 0);

      //Change the state to STATE_ACTIVE_E
      MoveRBUToOperationalState(rbu_node1_id, parent_node_id, system_id);

      //The state machine should be in the STATE_ACTIVE_E state
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

      //Send the fault signal again
      SMHandleEvent(EVENT_FAULT_SIGNAL_E, (uint8_t *)pMessage);

      //The message was addressed to another node so it should be re-broadcast.
      rachsq_count = osMessageCount(RACHSQ);
      CU_ASSERT_EQUAL(rachsq_count, 0);

      //Repeat fault msg with Hop Count = MAX_MSG_HOP_THRESHOLD - 1.
      // The message should not be sent uplink
      faultSignal.Header.HopCount = MAX_MSG_HOP_THRESHOLD - 1;
      result = MC_PUP_PackFaultSignal(&faultSignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      osMessageQFlush(RACHSQ);
      rachsq_count = osMessageCount(RACHSQ);
      CU_ASSERT_EQUAL(rachsq_count, 0);

      //Send the fault signal again
      SMHandleEvent(EVENT_FAULT_SIGNAL_E, (uint8_t *)pMessage);
      //The message hop limit was reached so it should not be re-broadcast.
      rachsq_count = osMessageCount(RACHSQ);
      CU_ASSERT_EQUAL(rachsq_count, 0);
   }

   // Now test the behaviour if using the correct system ID for the output signal
   faultSignal.SystemId = system_id;
   faultSignal.Header.HopCount = 0;

   pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      ErrorCode_t result = MC_PUP_PackFaultSignal(&faultSignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      //The state machine should be in the STATE_ACTIVE_E state
      CO_State_t state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

      //Send the fault signal again
      SMHandleEvent(EVENT_FAULT_SIGNAL_E, (uint8_t *)pMessage);

      //The message was addressed to another node so it should be scheduled for re-broadcast.
      bool ready_to_send = MC_ACK_MessageReadyToSend(ACK_RACHS_E);
      CU_ASSERT_TRUE(ready_to_send);

      /* check the msg type*/
      ApplicationLayerMessageType_t msg_type;
      uint32_t handle;
      MC_ACK_AcknowledgeMessage(ACK_RACHS_E, ADDRESS_NCU, true, &msg_type, &handle);
      CU_ASSERT_EQUAL(msg_type, APP_MSG_TYPE_FAULT_SIGNAL_E);

      //Repeat fault msg with Hop Count = MAX_MSG_HOP_THRESHOLD - 1.
      // The message should not be sent uplink
      faultSignal.Header.HopCount = MAX_MSG_HOP_THRESHOLD - 1;
      result = MC_PUP_PackFaultSignal(&faultSignal, &pMessage->Payload.PhyDataReq);
      CU_ASSERT_EQUAL(result, SUCCESS_E);

      //Send the fault signal again
      SMHandleEvent(EVENT_FAULT_SIGNAL_E, (uint8_t *)pMessage);
      //The message hop limit was reached so it should not be re-broadcast.
      ready_to_send = MC_ACK_MessageReadyToSend(ACK_RACHS_E);
      CU_ASSERT_FALSE(ready_to_send);
   }
}


/*************************************************************************************/
/**
* SM_RBUTestModeTest
* Test the RBU transition to test mode
*
* no params
*
* @return - none

*/
static void SM_RBUTestModeTest(void)
{
   CO_State_t state = STATE_TEST_MODE_E;
   uint16_t ncu_node_id = 0;
   uint16_t rbu_node1_id = 1;
   uint32_t system_id = SYSTEM_ID;
   const bool bNCU = true;
   const bool bRBU = false;

   SMInitialise(bNCU, ncu_node_id, system_id);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_IDLE_E);

   SM_SetCurrentDeviceState(STATE_TEST_MODE_E);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_TEST_MODE_E);


   SMInitialise(bRBU, rbu_node1_id, system_id);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

   SM_SetCurrentDeviceState(STATE_TEST_MODE_E);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_TEST_MODE_E);
}

/*************************************************************************************/
/**
* SM_RBUGenerateTestSignalTest
* Test the RBU Generate Test Signal function
*
* no params
*
* @return - none

*/
static void SM_RBUGenerateTestSignalTest(void)
{
   CO_State_t state = STATE_TEST_MODE_E;
   uint16_t rbu_node1_id = 1;
   uint32_t system_id = SYSTEM_ID;
   const bool bRBU = false;

   RACHSQ = osMessageCreate(osMessageQ(RACHSQ), NULL);
   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));
   osPoolFlush(MeshPool);
   osMessageQFlush(RACHSQ);
   CU_ASSERT_EQUAL(osMessageCount(RACHSQ), 0);

   SMInitialise(bRBU, rbu_node1_id, system_id);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

   SM_SetCurrentDeviceState(STATE_TEST_MODE_E);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_TEST_MODE_E);

   MC_MAC_SetTestMode(MC_MAC_TEST_MODE_TRANSMIT_E);


   /* send a command to generate a test message */
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t cmdReq;
   uint8_t payload[TEST_PAYLOAD_SIZE] = { 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d };

   cmdReq.Type = CO_MESSAGE_GENERATE_TEST_MESSAGE_E;
   memcpy(cmdReq.Payload.PhyDataReq.Data, payload, TEST_PAYLOAD_SIZE);

   SMHandleEvent(EVENT_GENERATE_TEST_MESSAGE_REQ_E, (uint8_t*)&cmdReq);

   /* there should be a message in RACHSQ */
   CU_ASSERT_EQUAL(osMessageCount(RACHSQ), 1);

   /* Check for a generated message */
   osEvent Event = osMessageGet(RACHSQ, 0);
   CU_ASSERT_EQUAL(0, osMessageCount(RACHSQ));
   osStatus status = osPoolFree(MeshPool, Event.value.p);
   CU_ASSERT_EQUAL(osOK, status);
   CU_ASSERT_EQUAL(0, osPoolBlocksAllocatedCount(MeshPool));

   CO_Message_t* pResponse = (CO_Message_t*)Event.value.p;
   CU_ASSERT_TRUE(pResponse != NULL);
   if (pResponse)
   {
      /* check the message length */
      CU_ASSERT_EQUAL(pResponse->Payload.PhyDataReq.Size, DATA_MESSAGE_SIZE_BYTES);

      /* check the frame type */
      uint8_t frame_type = pResponse->Payload.PhyDataReq.Data[0] >> 4;
      CU_ASSERT_EQUAL(frame_type, (uint8_t)FRAME_TYPE_TEST_MESSAGE_E);

      /* check the node id */
      uint16_t node_id = ((uint16_t)pResponse->Payload.PhyDataReq.Data[0] << 8) & 0xf00;
      node_id |= (uint16_t)pResponse->Payload.PhyDataReq.Data[1];
      CU_ASSERT_EQUAL(node_id, rbu_node1_id);

      /* check the payload */
      uint8_t* pMsgPayload = &pResponse->Payload.PhyDataReq.Data[2];
      for (uint32_t index = 0; index < TEST_PAYLOAD_SIZE; index++)
      {
         CU_ASSERT_EQUAL(pMsgPayload[index], payload[index]);
      }
   }

   /* check that no test messages are generated in CONNECTING state.  */
   SM_SetCurrentDeviceState(STATE_CONFIG_SYNC_E);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

   SMHandleEvent(EVENT_GENERATE_TEST_MESSAGE_REQ_E, (uint8_t*)&cmdReq);
   /* there should be no message in RACHSQ */
   CU_ASSERT_EQUAL(osMessageCount(RACHSQ), 0);

   /* check that no test messages are generated in OPERATIONAL state.  */
   SM_SetCurrentDeviceState(STATE_ACTIVE_E);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

   SMHandleEvent(EVENT_GENERATE_TEST_MESSAGE_REQ_E, (uint8_t*)&cmdReq);
   /* there should be no message in RACHSQ */
   CU_ASSERT_EQUAL(osMessageCount(RACHSQ), 0);
}

/*************************************************************************************/
/**
* SM_NCUGenerateTestSignalTest
* Test the RBU Generate Test Signal function
*
* no params
*
* @return - none

*/
static void SM_NCUGenerateTestSignalTest(void)
{
   CO_State_t state = STATE_TEST_MODE_E;
   uint16_t rbu_node1_id = 1;
   uint32_t system_id = SYSTEM_ID;
   const bool bNCU = true;

   RACHSQ = osMessageCreate(osMessageQ(RACHSQ), NULL);
   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));
   osPoolFlush(MeshPool);
   osMessageQFlush(RACHSQ);
   CU_ASSERT_EQUAL(osMessageCount(RACHSQ), 0);

   SMInitialise(bNCU, rbu_node1_id, system_id);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_IDLE_E);

   SM_SetCurrentDeviceState(STATE_TEST_MODE_E);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_TEST_MODE_E);


   /* send a command to generate a test message */
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t cmdReq;
   uint8_t payload[TEST_PAYLOAD_SIZE] = { 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d };

   cmdReq.Type = CO_MESSAGE_GENERATE_TEST_MESSAGE_E;
   memcpy(cmdReq.Payload.PhyDataReq.Data, payload, TEST_PAYLOAD_SIZE);

   SMHandleEvent(EVENT_GENERATE_TEST_MESSAGE_REQ_E, (uint8_t*)&cmdReq);

   /* there should be a message in RACHSQ */
   CU_ASSERT_EQUAL(osMessageCount(RACHSQ), 1);

   /* Check for a generated message */
   osEvent Event = osMessageGet(RACHSQ, 0);
   CU_ASSERT_EQUAL(0, osMessageCount(RACHSQ));
   osStatus status = osPoolFree(MeshPool, Event.value.p);
   CU_ASSERT_EQUAL(osOK, status);
   CU_ASSERT_EQUAL(0, osPoolBlocksAllocatedCount(MeshPool));

   CO_Message_t* pResponse = (CO_Message_t*)Event.value.p;
   CU_ASSERT_TRUE(pResponse != NULL);
   if (pResponse)
   {
      /* check the message length */
      CU_ASSERT_EQUAL(pResponse->Payload.PhyDataReq.Size, DATA_MESSAGE_SIZE_BYTES);

      /* check the frame type */
      uint8_t frame_type = pResponse->Payload.PhyDataReq.Data[0] >> 4;
      CU_ASSERT_EQUAL(frame_type, (uint8_t)FRAME_TYPE_TEST_MESSAGE_E);

      /* check the node id */
      uint16_t node_id = ((uint16_t)pResponse->Payload.PhyDataReq.Data[0] << 8) & 0xf00;
      node_id |= (uint16_t)pResponse->Payload.PhyDataReq.Data[1];
      CU_ASSERT_EQUAL(node_id, rbu_node1_id);

      /* check the payload */
      uint8_t* pMsgPayload = &pResponse->Payload.PhyDataReq.Data[2];
      for (uint32_t index = 0; index < TEST_PAYLOAD_SIZE; index++)
      {
         CU_ASSERT_EQUAL(pMsgPayload[index], payload[index]);
      }
   }

   /* check that no test messages are generated in CONNECTING state.  */
   SM_SetCurrentDeviceState(STATE_CONFIG_SYNC_E);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

   SMHandleEvent(EVENT_GENERATE_TEST_MESSAGE_REQ_E, (uint8_t*)&cmdReq);
   /* there should be no message in RACHSQ */
   CU_ASSERT_EQUAL(osMessageCount(RACHSQ), 0);

   /* check that no test messages are generated in OPERATIONAL state.  */
   SM_SetCurrentDeviceState(STATE_ACTIVE_E);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

   SMHandleEvent(EVENT_GENERATE_TEST_MESSAGE_REQ_E, (uint8_t*)&cmdReq);
   /* there should be no message in RACHSQ */
   CU_ASSERT_EQUAL(osMessageCount(RACHSQ), 0);
}















/*************************************************************************************/
/**
* SM_NCUGenerateSetStateTest
* Test the NCU Generate SetState function
*
* no params
*
* @return - none

*/
void SM_NCUGenerateSetStateTest(void)
{
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   const uint8_t NCU_DCH_SLOT = 16;
   uint32_t dlcchsq_count;
   uint32_t system_id = SYSTEM_ID;


   DLCCHSQ = osMessageCreate(osMessageQ(DLCCHSQ), NULL);

   osPoolFlush(MeshPool);
   osMessageQFlush(DLCCHSQ);

   dlcchsq_count = osMessageCount(DLCCHSQ);
   CU_ASSERT_EQUAL(dlcchsq_count, 0);

   SMInitialise(true, 0, system_id);//NCU

   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      //The state machine should be in the IDLE state
      CO_State_t state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_IDLE_E);

      /* set the state to CONFIG_SYNC */
      gSM_CurrentDeviceState = STATE_CONFIG_SYNC_E;

      pMessage->Payload.PhyDataReq.Data[0] = STATE_CONFIG_FORM_E;
      pMessage->Payload.PhyDataReq.Size = 1;

      SMHandleEvent(EVENT_GENERATE_SET_STATE_REQ_E, (uint8_t *)pMessage);

      //There should be NUM_STATE_CHANGE_TX messages in the DLCCH queue
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, NUM_STATE_CHANGE_TX);

      osMessageQFlush(DLCCHSQ);
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, 0);

      //The state machine should still be in the STATE_CONFIG_SYNC_E state
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

      // the scheduled state should be STATE_CONFIG_FORM_E
      CU_ASSERT_EQUAL(gSM_ScheduledDeviceState, STATE_CONFIG_FORM_E);

      //initiate the state change
      SM_ActivateStateChange(0);

      //The state machine should be in the STATE_CONFIG_FORM_E state
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_CONFIG_FORM_E);

      // there should be no scheduled state
      CU_ASSERT_EQUAL(gSM_ScheduledDeviceState, STATE_NOT_DEFINED_E);

      // Send a message to change the state to ACTIVE
      pMessage->Payload.PhyDataReq.Data[0] = STATE_ACTIVE_E;

      SMHandleEvent(EVENT_GENERATE_SET_STATE_REQ_E, (uint8_t *)pMessage);

      //There should be NUM_STATE_CHANGE_TX messages in the DLCCH queue
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, NUM_STATE_CHANGE_TX);

      osMessageQFlush(DLCCHSQ);
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, 0);

      //The state machine should still be in the STATE_CONFIG_FORM_E state
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_CONFIG_FORM_E);

      // the scheduled state should be STATE_ACTIVE_E
      CU_ASSERT_EQUAL(gSM_ScheduledDeviceState, STATE_ACTIVE_E);

      //initiate the state change
      SM_ActivateStateChange(0);

      //The state machine should be in the STATE_ACTIVE_E state
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

      // there should be no scheduled state
      CU_ASSERT_EQUAL(gSM_ScheduledDeviceState, STATE_NOT_DEFINED_E);

      // Setting the state back to STATE_CONFIG_SYNC_E should result in the
      // error handler being called to reset the unit
      bErrorHandlerCalled = false;
      // Send a message to change the state to ACTIVE
      pMessage->Payload.PhyDataReq.Data[0] = STATE_CONFIG_SYNC_E;
      SMHandleEvent(EVENT_GENERATE_SET_STATE_REQ_E, (uint8_t *)pMessage);

      //There should be NUM_STATE_CHANGE_TX messages in the DLCCH queue
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, NUM_STATE_CHANGE_TX);

      osMessageQFlush(DLCCHSQ);
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, 0);

      //The state machine should still be in the STATE_ACTIVE_E state
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

      // the scheduled state should be STATE_CONFIG_SYNC_E
      CU_ASSERT_EQUAL(gSM_ScheduledDeviceState, STATE_CONFIG_SYNC_E);

      //initiate the state change
      SM_ActivateStateChange(0);

      //check that ther error handler was called
      CU_ASSERT_TRUE(bErrorHandlerCalled);
   }
}

/*************************************************************************************/
/**
* SM_RBUGenerateSetStateTest
* Test the RBU Generate SetState function
*
* no params
*
* @return - none

*/
void SM_RBUGenerateSetStateTest(void)
{
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   const uint8_t NCU_DCH_SLOT = 16;
   uint32_t dlcchsq_count;
   uint32_t system_id = SYSTEM_ID;
   uint8_t state = 1;

   DLCCHSQ = osMessageCreate(osMessageQ(DLCCHSQ), NULL);

   osPoolFlush(MeshPool);
   osMessageQFlush(DLCCHSQ);

   dlcchsq_count = osMessageCount(DLCCHSQ);
   CU_ASSERT_EQUAL(dlcchsq_count, 0);

   SMInitialise(false, 1, system_id);//RBU


   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      pMessage->Payload.PhyDataReq.Data[0] = state;
      pMessage->Payload.PhyDataReq.Size = 1;

      //The state machine should be in the STATE_CONFIG_E state
      CO_State_t state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

      SMHandleEvent(EVENT_GENERATE_SET_STATE_REQ_E, (uint8_t *)pMessage);

      //There should no output message when in state STATE_CONFIG_E
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, 0);


      //Change the state to STATE_ACTIVE_E
      SM_SetCurrentDeviceState(STATE_ACTIVE_E);
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

      //Send the output signal again
      SMHandleEvent(EVENT_GENERATE_SET_STATE_REQ_E, (uint8_t *)pMessage);

      //Again, there should be no output
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, 0);

      //Set the state machine to STATE_TEST_MODE_E state
      SM_SetCurrentDeviceState(STATE_TEST_MODE_E);
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_TEST_MODE_E);

      //Send the output signal again
      SMHandleEvent(EVENT_GENERATE_SET_STATE_REQ_E, (uint8_t *)pMessage);

      //This time there should be no msg in DLCCHSQ
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, 0);

   }
}

/*************************************************************************************/
/**
* SM_NCUReceiveSetStateTest
* Test the NCU response to received SetState signals.
* NCUs don't use output messages so shouldn't act upon them
*
* no params
*
* @return - none

*/
void SM_NCUReceiveSetStateTest(void)
{
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   const uint8_t NCU_DCH_SLOT = 16;
   uint32_t dlcchsq_count;
   uint32_t system_id = SYSTEM_ID;
   uint8_t state = 1;
   SetStateMessage_t setState1 = { 0 };
   DMA_HandleTypeDef dummy_tx_handle;

   DLCCHSQ = osMessageCreate(osMessageQ(DLCCHSQ), NULL);

   osPoolFlush(MeshPool);
   osMessageQFlush(DLCCHSQ);

   dlcchsq_count = osMessageCount(DLCCHSQ);
   CU_ASSERT_EQUAL(dlcchsq_count, 0);

   SMInitialise(true, 0, system_id);//NCU

   CO_Message_t testMessage;

   setState1.Header.FrameType = FRAME_TYPE_DATA_E;
   setState1.Header.MACDestinationAddress = 0x101;
   setState1.Header.MACSourceAddress = 0x202;
   setState1.Header.HopCount = 4;
   setState1.Header.DestinationAddress = 0x303;
   setState1.Header.SourceAddress = 0x404;
   setState1.Header.MessageType = APP_MSG_TYPE_STATE_SIGNAL_E;
   setState1.State = 1;
   setState1.SystemId = SYSTEM_ID;

   status = MC_PUP_PackSetState(&setState1, &testMessage.Payload.PhyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 12;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1234;
   testMessage.Payload.PhyDataInd.Size = 20;

   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      memcpy(pMessage, &testMessage, sizeof(CO_Message_t));

      //The state machine should be in the IDLE state
      CO_State_t state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_IDLE_E);

      SMHandleEvent(EVENT_SET_STATE_SIGNAL_E, (uint8_t *)pMessage);

      //There should be no output when in state STATE_CONFIG_E
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, 0);

      //Generate a heartbeat to change the state to STATE_ACTIVE_E
      GenerateHeartbeat(NCU_DCH_SLOT);

      //Set STATE_ACTIVE_E state
      SM_SetCurrentDeviceState(STATE_ACTIVE_E);
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

      //Send the output signal again
      SMHandleEvent(EVENT_SET_STATE_SIGNAL_E, (uint8_t *)pMessage);

      //No message should be in DLCCHSQ
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, 0);

      osMessageQFlush(DLCCHSQ);
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, 0);

      //Set the state machine to STATE_TEST_MODE_E state
      MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
      SM_SetCurrentDeviceState(STATE_TEST_MODE_E);
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_TEST_MODE_E);

      /* Check that test mode sends data to the USART in test mode RECEIVE*/

      /* usart set up */
      memset(dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, 0, SERIAL_PORT_0_TX_BUFFER_SIZE);
      SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_read_ptr = 0;
      SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr = 0;
      /*add a dummy DMA Tx handle*/
      SerialChannelDefinition[DEBUG_UART].p_port_data->uart_handle.hdmatx = &dummy_tx_handle;

      //Send the output signal again
      SMHandleEvent(EVENT_SET_STATE_SIGNAL_E, (uint8_t *)pMessage);
      //There should be no output from the serial port because we are not in RECEIVE mode
      CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
      // set the test mode to RECEIVE
      MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
      //Send the output signal again
      SMHandleEvent(EVENT_SET_STATE_SIGNAL_E, (uint8_t *)pMessage);
      //A message should have been output to the serial port
      CU_ASSERT_TRUE(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr > 0);
   }
}

/*************************************************************************************/
/**
* SM_RBUReceiveSetStateTest
* Test the RBU response to received SetState signals.
*
* no params
*
* @return - none

*/
void SM_RBUReceiveSetStateTest(void)
{
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   const uint8_t NCU_DCH_SLOT = 16;
   uint32_t dlcchsq_count;
   uint32_t system_id = SYSTEM_ID;
   uint8_t state = 1;
   SetStateMessage_t setState1 = { 0 };
   DMA_HandleTypeDef dummy_tx_handle;

   DLCCHSQ = osMessageCreate(osMessageQ(DLCCHSQ), NULL);

   osPoolFlush(MeshPool);
   osMessageQFlush(DLCCHSQ);

   dlcchsq_count = osMessageCount(DLCCHSQ);
   CU_ASSERT_EQUAL(dlcchsq_count, 0);

   SMInitialise(false, 3, system_id);//RBU
   // set up the session manager 
   MC_SMGR_Initialise(false, 3);
   gParent[PRIMARY_PARENT].NodeInfo = 1;

   CO_Message_t testMessage;

   setState1.Header.FrameType = FRAME_TYPE_DATA_E;
   setState1.Header.MACDestinationAddress = 0xfff;
   setState1.Header.MACSourceAddress = 1;
   setState1.Header.HopCount = 1;
   setState1.Header.DestinationAddress = 0xfff;
   setState1.Header.SourceAddress = 0;
   setState1.Header.MessageType = APP_MSG_TYPE_STATE_SIGNAL_E;
   setState1.State = 1;
   setState1.SystemId = SYSTEM_ID;

   status = MC_PUP_PackSetState(&setState1, &testMessage.Payload.PhyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   testMessage.Type = CO_MESSAGE_PHY_DATA_IND_E;
   testMessage.Payload.PhyDataInd.RSSI = 10;
   testMessage.Payload.PhyDataInd.slotIdxInSuperframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInLongframe = 12;
   testMessage.Payload.PhyDataInd.slotIdxInShortframe = 12;
   testMessage.Payload.PhyDataInd.SNR = 7;
   testMessage.Payload.PhyDataInd.timer = 1234;
   testMessage.Payload.PhyDataInd.Size = 20;

   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      memcpy(pMessage, &testMessage, sizeof(CO_Message_t));

      //The state machine should be in the STATE_CONFIG_E state
      CO_State_t state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

      SMHandleEvent(EVENT_SET_STATE_SIGNAL_E, (uint8_t *)pMessage);

      //There should be a msg in the DLCCHSQ queue
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, 1);

      osMessageQFlush(DLCCHSQ);
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, 0);


      //Set STATE_ACTIVE_E state
      SM_SetCurrentDeviceState(STATE_ACTIVE_E);
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

      //Send the output signal again
      SMHandleEvent(EVENT_SET_STATE_SIGNAL_E, (uint8_t *)pMessage);

      //One message should be in DLCCHSQ
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, 1);

      osMessageQFlush(DLCCHSQ);
      dlcchsq_count = osMessageCount(DLCCHSQ);
      CU_ASSERT_EQUAL(dlcchsq_count, 0);

      //Set the state machine to STATE_TEST_MODE_E state
      MC_MAC_SetTestMode(MC_MAC_TEST_MODE_OFF_E);
      SM_SetCurrentDeviceState(STATE_TEST_MODE_E);
      state = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(state, STATE_TEST_MODE_E);

      /* Check that test mode sends data to the USART in test mode RECEIVE*/

      /* usart set up */
      memset((void*)dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer, 0, SERIAL_PORT_0_TX_BUFFER_SIZE);
      SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_read_ptr = 0;
      SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr = 0;
      /*add a dummy DMA Tx handle*/
      SerialChannelDefinition[DEBUG_UART].p_port_data->uart_handle.hdmatx = &dummy_tx_handle;

      //Send the output signal again
      SMHandleEvent(EVENT_SET_STATE_SIGNAL_E, (uint8_t *)pMessage);
      //There should be no output from the serial port because we are not in RECEIVE mode
      CU_ASSERT_EQUAL(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr, 0);
      // set the test mode to RECEIVE
      MC_MAC_SetTestMode(MC_MAC_TEST_MODE_RECEIVE_E);
      //Send the output signal again
      SMHandleEvent(EVENT_SET_STATE_SIGNAL_E, (uint8_t *)pMessage);
      //A message should have been output to the serial port
      CU_ASSERT_TRUE(SerialChannelDefinition[DEBUG_UART].p_port_data->buffer_run_data.tx_buffer_write_ptr > 0);
   }
}

/*************************************************************************************/
/**
* SM_RBUAverageSNRTest
* Test the RBU response to received SetState signals.
*
* no params
*
* @return - none

*/
void SM_RBUAverageSNRTest(void)
{
   static int16_t Q12_4AveragedSNR = 0;
   bool InitFilter = true;
   int8_t AverageSNR = 0;
   int8_t snr = 0;

   // Apply SNR step DOWN and check filter settles correctly.
   snr = 127;
   InitFilter = true;
   AverageSNR = MC_ApplyLowPassFilterToSNR(InitFilter, snr, &Q12_4AveragedSNR);
   CU_ASSERT_EQUAL(snr, AverageSNR);

   InitFilter = false;
   snr = -128;
   AverageSNR = MC_ApplyLowPassFilterToSNR(InitFilter, snr, &Q12_4AveragedSNR);
   CU_ASSERT_EQUAL(111, AverageSNR);

   InitFilter = false;
   snr = -128;
   AverageSNR = MC_ApplyLowPassFilterToSNR(InitFilter, snr, &Q12_4AveragedSNR);
   CU_ASSERT_EQUAL(96, AverageSNR);

   InitFilter = false;
   snr = -128;
   AverageSNR = MC_ApplyLowPassFilterToSNR(InitFilter, snr, &Q12_4AveragedSNR);
   CU_ASSERT_EQUAL(82, AverageSNR);

   InitFilter = false;
   snr = -128;
   AverageSNR = MC_ApplyLowPassFilterToSNR(InitFilter, snr, &Q12_4AveragedSNR);
   CU_ASSERT_EQUAL(69, AverageSNR);

   InitFilter = false;
   snr = -128;
   AverageSNR = MC_ApplyLowPassFilterToSNR(InitFilter, snr, &Q12_4AveragedSNR);
   CU_ASSERT_EQUAL(57, AverageSNR);

   InitFilter = false;
   snr = -128;
   AverageSNR = MC_ApplyLowPassFilterToSNR(InitFilter, snr, &Q12_4AveragedSNR);
   CU_ASSERT_EQUAL(45, AverageSNR);


   // Apply SNR step UP and check filter settles correctly.
   InitFilter = true;
   snr = -126;
   AverageSNR = MC_ApplyLowPassFilterToSNR(InitFilter, snr, &Q12_4AveragedSNR);
   CU_ASSERT_EQUAL(snr, AverageSNR);

   InitFilter = false;
   snr = 127;
   AverageSNR = MC_ApplyLowPassFilterToSNR(InitFilter, snr, &Q12_4AveragedSNR);
   CU_ASSERT_EQUAL(-110, AverageSNR);

   InitFilter = false;
   snr = 127;
   AverageSNR = MC_ApplyLowPassFilterToSNR(InitFilter, snr, &Q12_4AveragedSNR);
   CU_ASSERT_EQUAL(-95, AverageSNR);

   InitFilter = false;
   snr = 127;
   AverageSNR = MC_ApplyLowPassFilterToSNR(InitFilter, snr, &Q12_4AveragedSNR);
   CU_ASSERT_EQUAL(-81, AverageSNR);

   InitFilter = false;
   snr = 127;
   AverageSNR = MC_ApplyLowPassFilterToSNR(InitFilter, snr, &Q12_4AveragedSNR);
   CU_ASSERT_EQUAL(-68, AverageSNR);

   InitFilter = false;
   snr = 127;
   AverageSNR = MC_ApplyLowPassFilterToSNR(InitFilter, snr, &Q12_4AveragedSNR);
   CU_ASSERT_EQUAL(-56, AverageSNR);

   InitFilter = false;
   snr = 127;
   AverageSNR = MC_ApplyLowPassFilterToSNR(InitFilter, snr, &Q12_4AveragedSNR);
   CU_ASSERT_EQUAL(-45, AverageSNR);
}

/*************************************************************************************/
/**
* SM_RBUMeshFormingAndReformingTest1
* Test that an RBU will form with two parents and two tracking nodes, and will then 
* reform so that one parent is deleted and the primary tracking node is swapped to
* be the new primary parent.
*
* no params
*
* @return - none

*/
void SM_RBUMeshFormingAndReformingTest1(void)
{
   extern SMGR_ParentDetails_t gParent[NUMBER_OF_PARENTS_AND_TRACKING_NODES];
   extern SM_NeighbourList_t gaMeshNeighbourList[];

   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   uint32_t system_id = SYSTEM_ID;
   CO_State_t CurrentDeviceState = STATE_CONFIG_SYNC_E;
   bool success = false;

   // Create a message for the heartbeat
   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   FrameHeartbeat_t Heartbeat;
   FrameHeartbeat_t * pHeartbeat = &Heartbeat;
   uint32_t LongFrameIndex = 0;
   uint32_t ShortFrameIndex0 = 0;
   uint32_t ShortFrameIndex1 = 1;
   uint32_t SlotIndex17 = 17;
   uint32_t SlotIndex18 = 18;
   uint32_t SlotIndex19 = 19;
   uint32_t SlotIndex36 = 16;
   uint32_t SlotIndex37 = 17;
   uint32_t SlotIndex38 = 18;

   uint32_t Rank2 = 2;  // Only set this to 0 if SlotIndex is 16
   uint32_t Rank3 = 3;  // Only set this to 0 if SlotIndex is 16

   // In this test we don't set any SNRs to very low values.  Unlike in the test SM_RBUMeshFormingMixedSNRTest.
   uint8_t SetOneSNRveryLow = 0;

   // Initialise the RBU to address 1
   SMInitialise(false, 1, system_id);
   MC_SMGR_Initialise(false, 7);

   /**********************/
   /* MESH FORMING TESTS */
   /**********************/

   /* NOTE - IN THESE TESTS THE BASIC PROCEDURE IS TO SEND HEARTBEATS, MAKE TESTS, THEN INCREMENT THE FRAME */
   /* Sending heartbeats is done individually using SMHandleEvent(), or en-masses using PPPTheHeartbeats(); */
   /* Incrementing the long frame is done using SM_SendNewLongFrameMessage(++LongFrameIndex); 
      It is only required once the device has been put into state STATE_CONFIG_FORM_E. */

   // Initialise MeshFormAndHeal and Sync modules.
   MC_InitMeshFormAndHeal(1);
   MC_ResetSyncDatabase();

   // Test that the device has initialised with the correct mode and state.
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_SYNC_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, SEEKING_INITIAL_TRACKING_NODE_E);

   // Check that the Sync Module does not have the NCU registered as a tracking node.
   //   CU_ASSERT_EQUAL(MC_IsTrackingNode(SlotIndex), false);

   // Populate, pack and process heartbeat messages
   PPPTheHeartbeats(pHeartbeat, pMessage, LongFrameIndex, SetOneSNRveryLow);

   // Check that the Sync Module does have the NCU registered as a tracking node.
   CU_ASSERT_EQUAL(MC_IsTrackingNode(SlotIndex17), true);

   // Test that the SM has advanced to the Refining Initial Tracking Node stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_SYNC_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, REFINING_INITIAL_TRACKING_NODE_E);

   LongFrameIndex = 1;
   // Populate, pack and process heartbeat messages
   PPPTheHeartbeats(pHeartbeat, pMessage, LongFrameIndex, SetOneSNRveryLow);

   // Test that the SM has advanced to the Neighbour Scan stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_SYNC_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, NEIGHBOUR_SCAN_E);

   // Increment the MODE to STATE_CONFIG_FORM_E.
   // We need to call the function SM_ActivateStateChange to do this, because it schedules some of the mesh formation processes yet to happen.
   gSM_ScheduledDeviceState = STATE_CONFIG_FORM_E;
   SM_ActivateStateChange(LongFrameIndex);
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, NEIGHBOUR_SCAN_E);

   // Send enough hearbeats to complete the Neighbour Scan stage.
   for (uint32_t i = 0; i < NUM_NEIGHBOUR_SCANS; i++)
   {
      // Populate, pack and process heartbeat messages
      PPPTheHeartbeats(pHeartbeat, pMessage, LongFrameIndex, SetOneSNRveryLow);

      // Test that the SM is still in the Neighbour Scan stage
      CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
      CU_ASSERT_EQUAL(gMeshFormingStage, NEIGHBOUR_SCAN_E);

      // Start next long frame
      SM_SendNewLongFrameMessage(++LongFrameIndex);
   }

   // Test that the SM is in the Rank Selection stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, RANK_SELECTION_FIRST_PASS_E);

   // Send enough hearbeats to almost complete the first pass of Rank Selection stage.
   for (uint32_t i = 0; i < (Rank3 * LF_PER_RANK_ITERATION); i++)
   {
      // Populate, pack and process heartbeat messages
      PPPTheHeartbeats(pHeartbeat, pMessage, LongFrameIndex, SetOneSNRveryLow);

      // Start next long frame
      SM_SendNewLongFrameMessage(++LongFrameIndex);

      // Test that the SM has advanced to the Neighbour Scan stage
      CurrentDeviceState = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(CurrentDeviceState, STATE_CONFIG_FORM_E);
      CU_ASSERT_EQUAL(gMeshFormingStage, RANK_SELECTION_FIRST_PASS_E);
   }

   for (uint32_t i = 0; i < LF_PER_RANK_ITERATION; i++)
   {
      // Populate, pack and process heartbeat messages
      PPPTheHeartbeats(pHeartbeat, pMessage, LongFrameIndex, SetOneSNRveryLow);

      // Generate a MAC event to signal that a new long frame has started.
      SM_SendNewLongFrameMessage(++LongFrameIndex);
   }

   // Test that the SM has advanced to the Parent Selection stage
   CurrentDeviceState = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(CurrentDeviceState, STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, ADDING_PARENTS_E);

   // Send the heartbeat one at at time: this will cause the tracking nodes to be added in the synchronisation module.
   // Test that the mesh forming stage increments only after the last tracking node has been added.

   // Populate, pack and process heartbeat messages for slot 17
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex0, SlotIndex17, Rank2, NUM_CHILD_IDX_3, RSSI_LOW, SNR_IDX_LOW);
   status = MC_PUP_PackHeartbeat(pHeartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);
   CU_ASSERT_EQUAL(gMeshFormingStage, ADDING_PARENTS_E);

   // Populate, pack and process heartbeat messages for slot 17
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex0, SlotIndex18, Rank3, NUM_CHILD_IDX_7, RSSI_LOW, SNR_IDX_LOW);
   status = MC_PUP_PackHeartbeat(pHeartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);
   CU_ASSERT_EQUAL(gMeshFormingStage, ADDING_PARENTS_E);

   // Populate, pack and process heartbeat messages for slot 19
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex0, SlotIndex19, Rank3, NUM_CHILD_IDX_6, RSSI_LOW, SNR_IDX_LOW);
   status = MC_PUP_PackHeartbeat(pHeartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);
   CU_ASSERT_EQUAL(gMeshFormingStage, ADDING_PARENTS_E);

   // Populate, pack and process heartbeat messages for slot 36
   // RSSI set inappropriately to prove it selects parents based on the SNR
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex1, SlotIndex36, Rank3, NUM_CHILD_IDX_5, RSSI_HIGH, SNR_IDX_LOW);
   status = MC_PUP_PackHeartbeat(pHeartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);
   CU_ASSERT_EQUAL(gMeshFormingStage, ADDING_PARENTS_E);

   // Populate, pack and process heartbeat messages for slot 37
   // RSSI set inappropriately to prove it selects parents based on the SNR
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex1, SlotIndex37, Rank3, NUM_CHILD_IDX_6, RSSI_LOW, SNR_IDX_HIGH);
   status = MC_PUP_PackHeartbeat(pHeartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);
   CU_ASSERT_EQUAL(gMeshFormingStage, ADDING_PARENTS_E);

   // Populate, pack and process heartbeat messages for slot 38
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex1, SlotIndex38, Rank3, NUM_CHILD_IDX_4, RSSI_LOW, SNR_IDX_LOW);
   status = MC_PUP_PackHeartbeat(pHeartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);
   CU_ASSERT_EQUAL(gMeshFormingStage, MESH_FORMING_COMPLETE_E);

   // Test that the the candidate parents have been correctly identifed.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 4);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].slotIdx, 38);  // Because it has the lowest number of children in function PPPTheHeartbeats
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].slotIdx, 36);  // Because it has the 2nd lowest number of children in function PPPTheHeartbeats
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].slotIdx, 37);  // Because it has higher RSSI than slotIdx 19 in function PPPTheHeartbeats
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].slotIdx, 19);  // Because it has lower  RSSI than slotIdx 37 in function PPPTheHeartbeats

   // Check that the Sync Module has this device registered as a tracking node.
   CU_ASSERT_EQUAL(MC_IsTrackingNode(38), true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(36), true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(37), true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(19), true);

   // Check that the information sent to the session manager is as expected.
   CU_ASSERT_EQUAL(gParent[0].NodeInfo, 6);         // Primary parent node, slot 38, unit_id 6
   CU_ASSERT_EQUAL(gParent[1].NodeInfo, 4);         // Secondary parent node, slot 36, unit_id 4
   CU_ASSERT_EQUAL(gParent[2].NodeInfo, 5);         // Primary tracking node, slot 37, unit_id 5
   CU_ASSERT_EQUAL(gParent[3].NodeInfo, 3);         // Secondary tracking node, slot 19, unit_id 3

   /************************/
   /* MESH REFORMING TESTS */
   /************************/

   // Have a few attempts to delete the secondary tracking node, but specifying the wrong node ID.  DCH slot 19 is Node_ID 3
   success = MC_DeleteNode(0, SECONDARY_TRACKING_NODE_E);
   CU_ASSERT_EQUAL(success, false);
   success = MC_DeleteNode(1, SECONDARY_TRACKING_NODE_E);
   CU_ASSERT_EQUAL(success, false);
   success = MC_DeleteNode(2, SECONDARY_TRACKING_NODE_E);
   CU_ASSERT_EQUAL(success, false);

   // Have a few attempts to delete node 3, but specifying the wrong role.  DCH slot 19 is Node_ID 3
   success = MC_DeleteNode(0, PRIMARY_PARENT_E);
   CU_ASSERT_EQUAL(success, false);
   success = MC_DeleteNode(1, SECONDARY_PARENT_E);
   CU_ASSERT_EQUAL(success, false);
   success = MC_DeleteNode(2, PRIMARY_TRACKING_NODE_E);
   CU_ASSERT_EQUAL(success, false);

   // Delete the secondary tracking node.  DCH slot 19 is Node_ID 3
   success = MC_DeleteNode(3, SECONDARY_TRACKING_NODE_E);
   CU_ASSERT_EQUAL(success, true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(38), true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(36), true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(37), true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(19), false);   // Has been deleted
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 3);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].slotIdx, 38);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].slotIdx, 36);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].slotIdx, 37);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, false);  // Has been deleted
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].slotIdx, SLOTS_PER_LONG_FRAME);                  // Has been deleted

   // Delete the Primary Parent.  DCH slot 38 is Node_ID 6
   success = MC_DeleteNode(6, PRIMARY_PARENT_E);
   CU_ASSERT_EQUAL(success, true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(38), false);   // Has been deleted
   CU_ASSERT_EQUAL(MC_IsTrackingNode(36), true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(37), true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(19), false);
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 2);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, false);  // Has been deleted
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].slotIdx, SLOTS_PER_LONG_FRAME);                // Has been deleted
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].slotIdx, 36);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].slotIdx, 37);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].slotIdx, SLOTS_PER_LONG_FRAME);

   // Call the function to move the secondary parent to become primary tracking node.
   // DCH SLot 36 is node ID 4
   success = MC_ChangeNodeRole(4, SECONDARY_PARENT_E, PRIMARY_PARENT_E);
   CU_ASSERT_EQUAL(success, true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(38), false);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(36), true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(37), true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(19), false);
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 2);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);  // Has moved from secondary parent role
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].slotIdx, 36);                // Has moved from secondary parent role
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, false); // Has moved to primary parent role
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].slotIdx, SLOTS_PER_LONG_FRAME);                 // Has moved to primary parent role
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].slotIdx, 37);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].slotIdx, SLOTS_PER_LONG_FRAME);

   // Call the function to move the primary tracking node to become secondary parent node.
   // DCH SLot 37 is node ID 5
   success = MC_ChangeNodeRole(5, PRIMARY_TRACKING_NODE_E, SECONDARY_PARENT_E);
   CU_ASSERT_EQUAL(success, true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(38), false);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(36), true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(37), true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(19), false);
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 2);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].slotIdx, 36);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true); // Has moved from primary tracking node role
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].slotIdx, 37);                 // Has moved from primary tracking node role
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, false);  // Has moved to secondary parent role.
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].slotIdx, SLOTS_PER_LONG_FRAME);                // Has moved to secondary parent role.
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].slotIdx, SLOTS_PER_LONG_FRAME);

   // Check that we can activate the Parent Refresh Process
   success = MC_CheckIfParentRefreshProcessIsActive();
   CU_ASSERT_EQUAL(success, false);
   success = MC_ActivateParentRefreshProcess();
   CU_ASSERT_EQUAL(success, true);
   success = MC_CheckIfParentRefreshProcessIsActive();
   CU_ASSERT_EQUAL(success, true);

   // Increment the MODE to STATE_CONFIG_FORM_E.
   // We need to call the function SM_ActivateStateChange to do this, because it schedules some of the mesh formation processes yet to happen.
   gSM_ScheduledDeviceState = STATE_ACTIVE_E;
   SM_ActivateStateChange(LongFrameIndex);
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_ACTIVE_E);

   // Start next long frame.  In this iteration the parent refresh process will be scheduled to start on the next long frame.
   SM_SendNewLongFrameMessage(++LongFrameIndex);

   // Set power of slot 19 to a lower value, so that we can check that the SNR filter is initialised correctly.
   SetOneSNRveryLow = 2;

   // Send enough hearbeats to complete the Parent Refresh Process.
   for (uint32_t i = 0; i < NUM_NEIGHBOUR_SCANS; i++)
   {
      // Check that the Parent Refresh Process is still active
      success = MC_CheckIfParentRefreshProcessIsActive();
      CU_ASSERT_EQUAL(success, true);

      // Populate, pack and process heartbeat messages
      PPPTheHeartbeats(pHeartbeat, pMessage, LongFrameIndex, SetOneSNRveryLow);

      // Set the SNR back to the normal level
      SetOneSNRveryLow = 0;

      int16_t Q12_4HeartbeatAvgSNR = gaMeshNeighbourList[3].Q12_4HeartbeatAvgSNR;
      const int16_t SNRValues[4] = { -176, -160, -145, -131 };

      if (i < 4)  // Do this in case if the value of NUM_NEIGHBOUR_SCANS is increased.
      {
         // Check that the values stored in the filter initialises to -176 (-11 * 16), then iterates to -160, -145 and -131 as the filtering progress.
         CU_ASSERT_EQUAL(Q12_4HeartbeatAvgSNR, SNRValues[i]);
      }

      // Start next long frame
      SM_SendNewLongFrameMessage(++LongFrameIndex);
   }

   // Check that the Parent Refresh Process is no longer active
   success = MC_CheckIfParentRefreshProcessIsActive();
   CU_ASSERT_EQUAL(success, false);

   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 4);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].slotIdx, 36);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].slotIdx, 37);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, false);  // Hasn't yet been added to the sync database.
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].slotIdx, 38);  // Because it has the lowest number of children in function PPPTheHeartbeats
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, false);  // Hasn't yet been added to the sync database.
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].slotIdx, 18);  // Because it is the next best candidate.

   // Check that the Sync Module has this device registered as a tracking node.
   CU_ASSERT_EQUAL(MC_IsTrackingNode(38), true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(36), true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(37), true);
   CU_ASSERT_EQUAL(MC_IsTrackingNode(18), true);

   // Check that the information sent to the session manager is as expected.
   CU_ASSERT_EQUAL(gParent[2].NodeInfo, 6);  // Primary tracking node, slot 38, unit_id 6
   CU_ASSERT_EQUAL(gParent[3].NodeInfo, 2);  // Secondary tracking node, slot 19, unit_id 3
}

/*************************************************************************************/
/**
* SM_RBUMeshFormingMixedSNRTest
* Test that devices with SNR too low are not accepted as parents.
* This mesh forming is the same as for SM_RBUMeshFormingAndReformingTest1,
* but the SNR on the heartbeat in slotIDX 36 is set below the SNR threshold.
* This prevents it from being selected and becoming the secondary parent.
*
* no params
*
* @return - none

*/
void SM_RBUMeshFormingMixedSNRTest(void)
{
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   uint32_t system_id = SYSTEM_ID;
   CO_State_t CurrentDeviceState = STATE_CONFIG_SYNC_E;

   // Create a message for the heartbeat
   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   FrameHeartbeat_t Heartbeat;
   FrameHeartbeat_t * pHeartbeat = &Heartbeat;
   uint32_t LongFrameIndex = 0;
   uint32_t ShortFrameIndex0 = 0;
   uint32_t ShortFrameIndex1 = 1;
   uint32_t SlotIndex17 = 17;
   uint32_t SlotIndex18 = 18;
   uint32_t SlotIndex19 = 19;
   uint32_t SlotIndex36 = 16;
   uint32_t SlotIndex37 = 17;
   uint32_t SlotIndex38 = 18;

   uint32_t Rank2 = 2;  // Only set this to 0 if SlotIndex is 16
   uint32_t Rank3 = 3;  // Only set this to 0 if SlotIndex is 16

   // In this test we set power of slot 36 to a very low value.
   uint8_t SetOneSNRveryLow = 1;

   // Initialise the RBU to address 1
   SMInitialise(false, 1, system_id);

   // Initialise MeshFormAndHeal and Sync modules.
   MC_InitMeshFormAndHeal(1);
   MC_ResetSyncDatabase();

   // Test that the device has initialised with the correct mode and state.
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_SYNC_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, SEEKING_INITIAL_TRACKING_NODE_E);

   // Check that the Sync Module does not have the NCU registered as a tracking node.
   //   CU_ASSERT_EQUAL(MC_IsTrackingNode(SlotIndex), false);

   // Populate, pack and process heartbeat messages
   PPPTheHeartbeats(pHeartbeat, pMessage, LongFrameIndex, SetOneSNRveryLow);

   // Check that the Sync Module does have the NCU registered as a tracking node.
   CU_ASSERT_EQUAL(MC_IsTrackingNode(SlotIndex17), true);

   // Test that the SM has advanced to the Refining Initial Tracking Node stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_SYNC_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, REFINING_INITIAL_TRACKING_NODE_E);

   LongFrameIndex = 1;
   // Populate, pack and process heartbeat messages
   PPPTheHeartbeats(pHeartbeat, pMessage, LongFrameIndex, SetOneSNRveryLow);

   // Test that the SM has advanced to the Neighbour Scan stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_SYNC_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, NEIGHBOUR_SCAN_E);

   // Increment the MODE to STATE_CONFIG_FORM_E.
   // We need to call the function SM_ActivateStateChange to do this, because it schedules some of the mesh formation processes yet to happen.
   gSM_ScheduledDeviceState = STATE_CONFIG_FORM_E;
   SM_ActivateStateChange(LongFrameIndex);
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, NEIGHBOUR_SCAN_E);

   // Send enough hearbeats to complete the Neighbour Scan stage.
   for (uint32_t i = 0; i < NUM_NEIGHBOUR_SCANS; i++)
   {
      // Populate, pack and process heartbeat messages
      PPPTheHeartbeats(pHeartbeat, pMessage, LongFrameIndex, SetOneSNRveryLow);

      // Test that the SM is still in the Neighbour Scan stage
      CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
      CU_ASSERT_EQUAL(gMeshFormingStage, NEIGHBOUR_SCAN_E);

      // Start next long frame
      SM_SendNewLongFrameMessage(++LongFrameIndex);
   }

   // Test that the SM is in the Rank Selection stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, RANK_SELECTION_FIRST_PASS_E);

   // Send enough hearbeats to almost complete the first pass of Rank Selection stage.
   for (uint32_t i = 0; i < (Rank3 * LF_PER_RANK_ITERATION); i++)
   {
      // Populate, pack and process heartbeat messages
      PPPTheHeartbeats(pHeartbeat, pMessage, LongFrameIndex, SetOneSNRveryLow);

      // Start next long frame
      SM_SendNewLongFrameMessage(++LongFrameIndex);

      // Test that the SM has advanced to the Neighbour Scan stage
      CurrentDeviceState = SM_GetCurrentDeviceState();
      CU_ASSERT_EQUAL(CurrentDeviceState, STATE_CONFIG_FORM_E);
      CU_ASSERT_EQUAL(gMeshFormingStage, RANK_SELECTION_FIRST_PASS_E);
   }

   for (uint32_t i = 0; i < LF_PER_RANK_ITERATION; i++)
   {
      // Populate, pack and process heartbeat messages
      PPPTheHeartbeats(pHeartbeat, pMessage, LongFrameIndex, SetOneSNRveryLow);

      // Generate a MAC event to signal that a new long frame has started.
      SM_SendNewLongFrameMessage(++LongFrameIndex);
   }

   // Test that the SM has advanced to the Parent Selection stage
   CurrentDeviceState = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(CurrentDeviceState, STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, ADDING_PARENTS_E);

   // Send another heartbeat: this will cause the tracking node to be added in the synchronisation module.
   // Populate, pack and process heartbeat messages
   PPPTheHeartbeats(pHeartbeat, pMessage, LongFrameIndex, SetOneSNRveryLow);

   // Test that the the candidate parents have been correctly identifed.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 4);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].slotIdx, 38);  // Because it has the lowest number of children in function PPPTheHeartbeats
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[1].slotIdx, 37);  // Because it has higher SNR than slotIdx 19 in function PPPTheHeartbeats
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[2].slotIdx, 19);  // Because it has lower  SNR than slotIdx 37 in function PPPTheHeartbeats
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].addedToSyncDatabase, true);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[3].slotIdx, 18);  // Because it has lower  RSSI than slotIdx 37 in function PPPTheHeartbeats

   // Check that the Sync Module has these devices registered as tracking nodes.
   // slot 39 should not be selected because it has low RSSI
   CU_ASSERT_FALSE(MC_IsTrackingNode(39));
   CU_ASSERT_TRUE(MC_IsTrackingNode(19));
   CU_ASSERT_TRUE(MC_IsTrackingNode(18));  // Since the unit in slot 36 had too low snr, this one is selected instead.
   CU_ASSERT_TRUE(MC_IsTrackingNode(37));
   CU_ASSERT_TRUE(MC_IsTrackingNode(38));
}

/*************************************************************************************/
/**
* SM_RBUMeshFormingRank1Test
* Test the RBU's ability to join a mesh at rank 1
*
* no params
*
* @return - none

*/
void SM_RBUMeshFormingRank1Test(void)
{
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   uint32_t system_id = SYSTEM_ID;

   // Create a message for the heartbeat
   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   FrameHeartbeat_t Heartbeat;
   FrameHeartbeat_t * pHeartbeat = &Heartbeat;
   uint32_t LongFrameIndex = 0;
   uint32_t ShortFrameIndex = 0;
   uint32_t SlotIndex = 16;
   uint32_t Rank = 0;
   uint32_t NumChildrenIdx = 0;
   int16_t  RSSI = -50;
   int8_t   SNRIdx = 20;

   // Initialise the RBU to address 1
   SMInitialise(false, 1, system_id);

   // Initialise MeshFormAndHeal and Sync modules.
   MC_InitMeshFormAndHeal(1);
   MC_ResetSyncDatabase();

   // Test that the device has initialised with the correct mode and state.
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_SYNC_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, SEEKING_INITIAL_TRACKING_NODE_E);

   // Check that the Sync Module does not have the NCU registered as a tracking node.
   CU_ASSERT_EQUAL(MC_IsTrackingNode(SlotIndex), false);

   // Populate, pack and process heartbeat message from the NCU
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex, SlotIndex, Rank, NumChildrenIdx, RSSI, SNRIdx);
   status = MC_PUP_PackHeartbeat(&Heartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

   // Check that the Sync Module does have the NCU registered as a tracking node.
   CU_ASSERT_EQUAL(MC_IsTrackingNode(SlotIndex), true);

   // Test that the SM has advanced to the Refining Initial Tracking Node stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_SYNC_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, REFINING_INITIAL_TRACKING_NODE_E);

   // Populate, pack and process heartbeat message from the NCU
   LongFrameIndex = 1;
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex, SlotIndex, Rank, NumChildrenIdx, RSSI, SNRIdx);
   MC_PUP_PackHeartbeat(&Heartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

   // Test that the SM has advanced to the Neighbour Scan stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_SYNC_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, NEIGHBOUR_SCAN_E);

   // Increment the MODE to STATE_CONFIG_FORM_E.
   // We need to call the function SM_ActivateStateChange to do this, because it schedules some of the mesh formation processes yet to happen.
   gSM_ScheduledDeviceState = STATE_CONFIG_FORM_E;
   SM_ActivateStateChange(LongFrameIndex);
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, NEIGHBOUR_SCAN_E);

   // Send enough hearbeats to complete the Neighbour Scan stage.
   for (uint32_t i = 0; i < NUM_NEIGHBOUR_SCANS; i++)
   {
      // Populate, pack and process next heartbeat message from the NCU
      SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex, SlotIndex, Rank, NumChildrenIdx, RSSI, SNRIdx);
      MC_PUP_PackHeartbeat(&Heartbeat, &pMessage->Payload.PhyDataReq);
      SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

      // Test that the SM is still in the Neighbour Scan stage
      CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
      CU_ASSERT_EQUAL(gMeshFormingStage, NEIGHBOUR_SCAN_E);

      // Start next long frame
      SM_SendNewLongFrameMessage(++LongFrameIndex);
   }

   // Test that the SM is in the Rank Selection stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, RANK_SELECTION_FIRST_PASS_E);

   for (uint32_t i = 0; i < LF_PER_RANK_ITERATION; i++)
   {
      // Send one more hearbeat, in response to which the RBU should select the NCU as it's parent and set it's rank to 1.
      // Populate, pack and process next heartbeat message from the NCU
      SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex, SlotIndex, Rank, NumChildrenIdx, RSSI, SNRIdx);
      MC_PUP_PackHeartbeat(&Heartbeat, &pMessage->Payload.PhyDataReq);
      SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

      // Test that the SM is in the Rank Selection stage
      CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
      CU_ASSERT_EQUAL(gMeshFormingStage, RANK_SELECTION_FIRST_PASS_E);

      // Generate a MAC event to signal that a new long frame has started.
      SM_SendNewLongFrameMessage(++LongFrameIndex);
   }

   // Test that the SM has advanced to the Parent Selection stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, ADDING_PARENTS_E);

   // Test that the one candidate parent that has been identifed is in the list of tracking nodes.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 1);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].slotIdx, SlotIndex);

   // Send another heartbeat: this will cause the tracking node to be added in the synchronisation module.
   // Populate, pack and process next heartbeat message from the NCU
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex, SlotIndex, Rank, NumChildrenIdx, RSSI, SNRIdx);
   MC_PUP_PackHeartbeat(&Heartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

   // Test that the one candidate parent that has been identifed is in the list of tracking nodes.
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);

   // Check that the Sync Module has this device registered as a tracking node.
   CU_ASSERT_EQUAL(MC_IsTrackingNode(SlotIndex), true);
}

/*************************************************************************************/
/**
* SM_RBUMeshFormingMaxRankTest
* Test the RBU's ability to join a mesh at rank 1
*
* no params
*
* @return - none

*/
void SM_RBUMeshFormingMaxRankTest(void)
{
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   uint32_t system_id = SYSTEM_ID;

   // Create a message for the heartbeat
   CO_Message_t* pMessage = osPoolAlloc(MeshPool);
   CU_ASSERT_TRUE(pMessage != NULL);
   FrameHeartbeat_t Heartbeat;
   FrameHeartbeat_t * pHeartbeat = &Heartbeat;
   uint32_t LongFrameIndex = 0;
   uint32_t ShortFrameIndex = 0;
   uint32_t SlotIndex = 18;
   uint32_t Rank = MAX_RANK - 1;
   uint32_t NumChildrenIdx = 0;
   int16_t  RSSI = -50;
   int8_t   SNRIdx = 20;

   // Initialise the RBU to address 1
   SMInitialise(false, 1, system_id);

   // Initialise MeshFormAndHeal and Sync modules.
   MC_InitMeshFormAndHeal(1);
   MC_ResetSyncDatabase();

   // Test that the device has initialised with the correct mode and state.
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_SYNC_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, SEEKING_INITIAL_TRACKING_NODE_E);

   // Check that the Sync Module does not have the RBU registered as a tracking node.
   CU_ASSERT_EQUAL(MC_IsTrackingNode(SlotIndex), false);

   // Populate, pack and process heartbeat message from the RBU
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex, SlotIndex, Rank, NumChildrenIdx, RSSI, SNRIdx);
   status = MC_PUP_PackHeartbeat(&Heartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

   // Check that the Sync Module does now have the RBU registered as a tracking node.
   CU_ASSERT_EQUAL(MC_IsTrackingNode(SlotIndex), true);

   // Test that the SM has advanced to the Refining Initial Tracking Node stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_SYNC_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, REFINING_INITIAL_TRACKING_NODE_E);

   // Populate, pack and process next heartbeat message from the RBU
   LongFrameIndex = 1;
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex, SlotIndex, Rank, NumChildrenIdx, RSSI, SNRIdx);
   MC_PUP_PackHeartbeat(&Heartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

   // Test that the SM has advanced to the Neighbour Scan stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_SYNC_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, NEIGHBOUR_SCAN_E);

   // Increment the MODE to STATE_CONFIG_FORM_E.
   // We need to call the function SM_ActivateStateChange to do this, because it schedules some of the mesh formation processes yet to happen.
   gSM_ScheduledDeviceState = STATE_CONFIG_FORM_E;
   SM_ActivateStateChange(LongFrameIndex);
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, NEIGHBOUR_SCAN_E);

   // Send enough hearbeats to complete the Neighbour Scan stage.
   for (uint32_t i = 0; i < NUM_NEIGHBOUR_SCANS; i++)
   {
      // Populate, pack and process next heartbeat message from the NCU
      SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex, SlotIndex, Rank, NumChildrenIdx, RSSI, SNRIdx);
      MC_PUP_PackHeartbeat(&Heartbeat, &pMessage->Payload.PhyDataReq);
      SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

      // Test that the SM is still in the Neighbour Scan stage
      CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
      CU_ASSERT_EQUAL(gMeshFormingStage, NEIGHBOUR_SCAN_E);

      // Start next long frame
      SM_SendNewLongFrameMessage(++LongFrameIndex);
   }

   // Test that the SM is in the Rank Selection stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, RANK_SELECTION_FIRST_PASS_E);

   // Send enough hearbeats to almost complete the first pass of Rank Selection stage.
   for (uint32_t i = 0; i < ((MAX_RANK * LF_PER_RANK_ITERATION) - 1); i++)
   {
      // Populate, pack and process next heartbeat message from the NCU
      SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex, SlotIndex, Rank, NumChildrenIdx, RSSI, SNRIdx);
      MC_PUP_PackHeartbeat(&Heartbeat, &pMessage->Payload.PhyDataReq);
      SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

      // Test that the SM has advanced to the Neighbour Scan stage
      CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
      CU_ASSERT_EQUAL(gMeshFormingStage, RANK_SELECTION_FIRST_PASS_E);

      // Start next long frame
      SM_SendNewLongFrameMessage(++LongFrameIndex);
   }

   // Test that the SM is in the Rank Selection stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, RANK_SELECTION_FIRST_PASS_E);

   // Send one more hearbeat, in response to which the RBU should move to the second pass of the rank selection process (which will accept a single parent).
   // Populate, pack and process next heartbeat message from the NCU
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex, SlotIndex, Rank, NumChildrenIdx, RSSI, SNRIdx);
   MC_PUP_PackHeartbeat(&Heartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

   // Test that the SM is in the Rank Selection stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, RANK_SELECTION_FIRST_PASS_E);

   // Generate a MAC event to signal that a new long frame has started.
   SM_SendNewLongFrameMessage(++LongFrameIndex);

   // Test that the SM has advanced to the Parent Selection stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, RANK_SELECTION_SECOND_PASS_E);

   // Send enough hearbeats to almost complete the second pass of Rank Selection stage (which will accept a single parent).
   for (uint32_t i = 0; i < ((MAX_RANK * LF_PER_RANK_ITERATION) - 1); i++)
   {
      // Populate, pack and process next heartbeat message from the NCU
      SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex, SlotIndex, Rank, NumChildrenIdx, RSSI, SNRIdx);
      MC_PUP_PackHeartbeat(&Heartbeat, &pMessage->Payload.PhyDataReq);
      SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

      // Test that the SM has advanced to the Neighbour Scan stage
      CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
      CU_ASSERT_EQUAL(gMeshFormingStage, RANK_SELECTION_SECOND_PASS_E);

      // Start next long frame
      SM_SendNewLongFrameMessage(++LongFrameIndex);
   }

   // Test that the SM is in the Rank Selection stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, RANK_SELECTION_SECOND_PASS_E);

   // Send one more hearbeat, in response to which the RBU should now detect the single candidate parent.
   // Populate, pack and process next heartbeat message from the NCU
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex, SlotIndex, Rank, NumChildrenIdx, RSSI, SNRIdx);
   MC_PUP_PackHeartbeat(&Heartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

   // Test that the SM is in the Rank Selection stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, RANK_SELECTION_SECOND_PASS_E);

   // Generate a MAC event to signal that a new long frame has started.
   SM_SendNewLongFrameMessage(++LongFrameIndex);

   // Test that the SM has advanced to the Adding Parents stage
   CU_ASSERT_EQUAL(SM_GetCurrentDeviceState(), STATE_CONFIG_FORM_E);
   CU_ASSERT_EQUAL(gMeshFormingStage, ADDING_PARENTS_E);

   // Test that the one candidate parent that has been identifed is in the list of tracking nodes.
   CU_ASSERT_EQUAL(gaTrackingNodes.noOfTrackingNodes, 1);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, false);
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].slotIdx, SlotIndex);

   // Send another heartbeat: this will cause the tracking node to be added in the synchronisation module.
   // Populate, pack and process next heartbeat message from the NCU
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex, SlotIndex, Rank, NumChildrenIdx, RSSI, SNRIdx);
   MC_PUP_PackHeartbeat(&Heartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

   // Test that the one candidate parent that has been identifed is in the list of tracking nodes.
   CU_ASSERT_EQUAL(gaTrackingNodes.aTrackingNodeList[0].addedToSyncDatabase, true);

   // Check that the Sync Module has this device registered as a tracking node.
   CU_ASSERT_EQUAL(MC_IsTrackingNode(SlotIndex), true);
}

/*************************************************************************************/
/**
* SM_PopulateHeartbeatMessage
* Populates the heartbeat message via the pointer arguments.
*
* @param - FrameHeartbeat_t * pHeartbeat
* @param - CO_Message_t * pMessage
* @param - uint32_t LongFrameIndex
* @param - uint32_t ShortFrameIndex
* @param - uint32_t SlotIndex
* @param - uint32_t Rank
* @param - uint32_t NoOfChildrenIdx
* @param - uint16_t RSSI
* @param - uint8_t SNR
*
* @return - none

*/
void SM_PopulateHeartbeatMessage(
   FrameHeartbeat_t * pHeartbeat,
   CO_Message_t * pMessage,
   uint32_t LongFrameIndex,
   uint32_t ShortFrameIndex,
   uint32_t SlotIndex,
   uint32_t Rank,
   uint32_t NoOfChildrenIdx,
   int16_t  RSSI,
   int8_t SNR)
{
   // Populate the Heartbeat content
   pHeartbeat->FrameType = FRAME_TYPE_HEARTBEAT_E;
   pHeartbeat->LongFrameIndex = LongFrameIndex;
   pHeartbeat->ShortFrameIndex = ShortFrameIndex;
   pHeartbeat->SlotIndex = SlotIndex;
   pHeartbeat->Rank = Rank;
   pHeartbeat->NoOfChildrenIdx = NoOfChildrenIdx;
   pHeartbeat->NoOfChildrenOfPTNIdx = 0;
   pHeartbeat->State = 0;
   pHeartbeat->SystemId = SYSTEM_ID;

   // Set the Message content
   pMessage->Payload.PhyDataInd.slotIdxInShortframe = SlotIndex;
   pMessage->Payload.PhyDataInd.slotIdxInLongframe  = SlotIndex + SLOTS_PER_SHORT_FRAME * ShortFrameIndex;
   pMessage->Payload.PhyDataInd.slotIdxInSuperframe = SlotIndex + SLOTS_PER_SHORT_FRAME * ShortFrameIndex + SLOTS_PER_LONG_FRAME * LongFrameIndex;
   pMessage->Payload.PhyDataInd.RSSI = RSSI;
   pMessage->Payload.PhyDataInd.SNR = SNR;
   pMessage->Payload.PhyDataInd.isInTDMSyncSlaveMode = true;

   return;
}

/*************************************************************************************/
/**
* SM_SendNewLongFrameMessage
* Sends a MAC Event message to signal that a new long frame has started.
*
* @param - const uint32_t LongFrameIndex
*
* @return - none

*/
void SM_SendNewLongFrameMessage(const uint32_t LongFrameIndex)
{
   // Create a message for MAC events
   CO_Message_t MissedHbMsg;
   MissedHbMsg.Type = CO_MESSAGE_MAC_EVENT_E;
   MACEventMessage_t macEvent;
   macEvent.EventType = CO_NEW_LONG_FRAME_E;

   macEvent.LongFrameIndex = LongFrameIndex;
   memcpy(MissedHbMsg.Payload.PhyDataInd.Data, &macEvent, sizeof(MACEventMessage_t));
   SMHandleEvent(EVENT_MAC_EVENT_E, (uint8_t*)&MissedHbMsg);

   return;
}

/*************************************************************************************/
/**
* PPPTheHeartbeats
* Populate, pack and process heartbeat messages
*
* @param - const uint32_t LongFrameIndex
*
* @return - none

*/
static void PPPTheHeartbeats(FrameHeartbeat_t * pHeartbeat, CO_Message_t* pMessage, uint32_t LongFrameIndex, uint8_t SetSNRveryLow)
{
   ErrorCode_t status = SUCCESS_E;

   uint32_t ShortFrameIndex0 = 0;
   uint32_t ShortFrameIndex1 = 1;
   uint32_t SlotIndex17 = 17;
   uint32_t SlotIndex18 = 18;
   uint32_t SlotIndex19 = 19;
   uint32_t SlotIndex36 = 16;
   uint32_t SlotIndex37 = 17;
   uint32_t SlotIndex38 = 18;
   uint32_t SlotIndex39 = 19;
   uint32_t Rank2 = 2;  // Only set this to 0 if SlotIndex is 16
   uint32_t Rank3 = 3;  // Only set this to 0 if SlotIndex is 16
   int8_t   snr36 = 0;
   int8_t   snr19 = 0;
   int16_t  bad_rssi = MIN_CANDIDATE_RSSI - 1;

   // Set the SNR for the heartbeat sent in slot 36 to either just above or just below the SNR Minimum Attenuation Reserve threshold, as specified by the flag.
   snr36 = SNR_IDX_LOW;
   snr19 = SNR_IDX_LOW;
   if (1 == SetSNRveryLow)
   {
      snr36 = SNR_IDX_TOO_LOW;
   }
   if (2 == SetSNRveryLow)
   {
      snr19 = (SNR_IDX_LOW - 16);
   }

   // Populate, pack and process heartbeat messages for slot 17
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex0, SlotIndex17, Rank2, NUM_CHILD_IDX_3, RSSI_LOW, SNR_IDX_LOW);
   status = MC_PUP_PackHeartbeat(pHeartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

   // Populate, pack and process heartbeat messages for slot 18
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex0, SlotIndex18, Rank3, NUM_CHILD_IDX_7, RSSI_LOW, SNR_IDX_LOW);
   status = MC_PUP_PackHeartbeat(pHeartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

   // Populate, pack and process heartbeat messages for slot 19
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex0, SlotIndex19, Rank3, NUM_CHILD_IDX_6, RSSI_LOW, snr19);
   status = MC_PUP_PackHeartbeat(pHeartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

   // Populate, pack and process heartbeat messages for slot 36
   // RSSI set inappropriately to prove it selects parents based on the SNR
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex1, SlotIndex36, Rank3, NUM_CHILD_IDX_5, RSSI_HIGH, snr36);
   status = MC_PUP_PackHeartbeat(pHeartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

   // Populate, pack and process heartbeat messages for slot 37
   // RSSI set inappropriately to prove it selects parents based on the SNR
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex1, SlotIndex37, Rank3, NUM_CHILD_IDX_6, RSSI_LOW, SNR_IDX_HIGH);
   status = MC_PUP_PackHeartbeat(pHeartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

   // Populate, pack and process heartbeat messages for slot 38
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex1, SlotIndex38, Rank3, NUM_CHILD_IDX_4, RSSI_LOW, SNR_IDX_LOW);
   status = MC_PUP_PackHeartbeat(pHeartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);

   // Populate, pack and process heartbeat messages for slot 39.  Bad RSSI.
   SM_PopulateHeartbeatMessage(pHeartbeat, pMessage, LongFrameIndex, ShortFrameIndex1, SlotIndex39, Rank3, NUM_CHILD_IDX_3, bad_rssi, SNR_IDX_HIGH);
   status = MC_PUP_PackHeartbeat(pHeartbeat, &pMessage->Payload.PhyDataReq);
   SMHandleEvent(EVENT_HEARTBEAT_E, (uint8_t *)pMessage);
}

/*************************************************************************************/
/**
* SM_NCUGenerateRBUDisableTest
* Test the NCU Generate RBU Disable global message
*
* no params
*
* @return - none

*/
static void SM_NCUGenerateRBUDisableTest(void)
{
   CO_State_t state;
   uint16_t rbu_node1_id = 1;
   uint16_t ncu_id = 0;
   uint32_t system_id = SYSTEM_ID;
   const bool bNCU = true;

   DLCCHSQ = osMessageCreate(osMessageQ(DLCCHSQ), NULL);
   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));
   osPoolFlush(MeshPool);
   osMessageQFlush(DLCCHSQ);
   CU_ASSERT_EQUAL(osMessageCount(DLCCHSQ), 0);

   SMInitialise(bNCU, ncu_id, system_id);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_IDLE_E);

   SM_SetCurrentDeviceState(STATE_ACTIVE_E);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);


   /* send a command to generate an RBU Disable */
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t cmdReq;
   CO_RBUDisableData_t testMsg = { 0 };

   testMsg.Handle = 1;
   testMsg.UnitAddress = 15;
 
   cmdReq.Type = CO_MESSAGE_GENERATE_RBU_DISABLE_MESSAGE_E;
   memcpy(cmdReq.Payload.PhyDataReq.Data, &testMsg, sizeof(CO_RBUDisableData_t));

   SMHandleEvent(EVENT_GENERATE_RBU_DISABLE_MESSAGE_E, (uint8_t*)&cmdReq);

   /* The messages are broadcast in triplicate. There should be 3 messages in DLCCHSQ */
   CU_ASSERT_EQUAL(osMessageCount(DLCCHSQ), 3);

   /* Check for a generated message */
   osEvent Event = osMessageGet(DLCCHSQ, 0);
   CU_ASSERT_EQUAL(Event.status, osEventMessage);
   CO_Message_t* pResponse = (CO_Message_t*)Event.value.p;
   CU_ASSERT_TRUE(pResponse != NULL);
   if (pResponse)
   {
      /* check the message length */
      CU_ASSERT_EQUAL(pResponse->Payload.PhyDataReq.Size, DATA_MESSAGE_SIZE_BYTES);

      /* check the frame type */
      uint8_t frame_type = pResponse->Payload.PhyDataReq.Data[0] >> 4;
      CU_ASSERT_EQUAL(frame_type, (uint8_t)FRAME_TYPE_DATA_E);

      /* check it's globally addressed */
      uint16_t node_id = ((uint16_t)pResponse->Payload.PhyDataReq.Data[0] << 8) & 0xf00;
      node_id |= (uint16_t)pResponse->Payload.PhyDataReq.Data[1];
      CU_ASSERT_EQUAL(node_id, ADDRESS_GLOBAL);

      /* check the payload */
      uint16_t targetNode = (uint16_t)pResponse->Payload.PhyDataReq.Data[9];
      targetNode >>= 6;
      targetNode |= (((uint16_t)pResponse->Payload.PhyDataReq.Data[8] & 0x7) << 2);
      
      CU_ASSERT_EQUAL(targetNode, testMsg.UnitAddress);
   }
}

/*************************************************************************************/
/**
* SM_RBUDisableMessageTest
* Test the rbuDisableMessage function
*
* no params
*
* @return - none

*/
static void SM_RBUDisableMessageTest(void)
{
   extern SM_NeighbourList_t gaMeshNeighbourList[];
   CO_State_t state;
   uint16_t rbu_node1_id = 1;
   uint16_t rbu_node2_id = 2;
   uint16_t node2_dch_slot = 18;
   uint16_t ncu_id = 0;
   uint16_t ncu_dch_slot = 16;
   uint32_t system_id = SYSTEM_ID;
   const bool bRBU = false;

   DLCCHSQ = osMessageCreate(osMessageQ(DLCCHSQ), NULL);
   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));
   osPoolFlush(MeshPool);
   osMessageQFlush(DLCCHSQ);
   CU_ASSERT_EQUAL(osMessageCount(DLCCHSQ), 0);

   SMInitialise(bRBU, rbu_node1_id, system_id);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

   SM_SetCurrentDeviceState(STATE_ACTIVE_E);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

   /* set the stubbed application callback function */
//   SMSetRBUDisableIndCb(ST_RBUDisableInd);

   /* set up the session manager */
   MC_SMGR_Initialise(false, rbu_node1_id);
   MC_SMGR_SetParentNodes(ncu_dch_slot, node2_dch_slot);
   MC_AddTrackingNode(node2_dch_slot);


   /* insert a valid RBU Disable message into the state machine */
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t cmdInd;
   uint8_t validMsgDisableNode1[DATA_MESSAGE_SIZE_BYTES] = { 0x1f,0xff,0x00,0x00,0x0f,0xff,0x00,0x09,0x80,0x40,0x00,0x00,0x00,0x00,0x00,0x01,0x23,0x45,0x67,0x80 };
   uint8_t validMsgDisableNode2[DATA_MESSAGE_SIZE_BYTES] = { 0x1f,0xff,0x00,0x00,0x0f,0xff,0x00,0x09,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x01,0x23,0x45,0x67,0x80 };
   uint8_t wrongNodeMsg[DATA_MESSAGE_SIZE_BYTES] = { 0x1f,0xff,0x00,0x00,0x0f,0xff,0x00,0x09,0x80,0xC0,0x00,0x00,0x00,0x00,0x00,0x01,0x23,0x45,0x67,0x80 };
   uint8_t wrongSystemMsg[DATA_MESSAGE_SIZE_BYTES] = { 0x1f,0xff,0x00,0x00,0x0f,0xff,0x00,0x09,0x80,0x40,0x00,0x00,0x00,0x00,0x00,0x01,0x23,0x45,0x67,0x00 };

   /* test with a NULL message  */
   //RBUDisableInd_Count = 0;
   //RBUDisableInd_Handle = 0;
   //RBUDisableInd_UnitAddress = 0;
   SMHandleEvent(EVENT_RBU_DISABLE_E, (uint8_t*)&cmdInd);
   CU_ASSERT_EQUAL(osMessageCount(DLCCHSQ), 0);
//   CU_ASSERT_EQUAL(RBUDisableInd_Count, 0);

   /* test with the wrong system ID */
   cmdInd.Type = CO_MESSAGE_PHY_DATA_IND_E;
   cmdInd.Payload.PhyDataInd.Size = DATA_MESSAGE_SIZE_BYTES;
   memcpy(cmdInd.Payload.PhyDataInd.Data, wrongSystemMsg, DATA_MESSAGE_SIZE_BYTES);
   //RBUDisableInd_Count = 0;
   //RBUDisableInd_Handle = 0;
   //RBUDisableInd_UnitAddress = 0;
   SMHandleEvent(EVENT_RBU_DISABLE_E, (uint8_t*)&cmdInd);
   CU_ASSERT_EQUAL(osMessageCount(DLCCHSQ), 0);
//   CU_ASSERT_EQUAL(RBUDisableInd_Count, 0);

   /* test with the wrong node ID */
   cmdInd.Type = CO_MESSAGE_PHY_DATA_IND_E;
   cmdInd.Payload.PhyDataInd.Size = DATA_MESSAGE_SIZE_BYTES;
   memcpy(cmdInd.Payload.PhyDataInd.Data, wrongNodeMsg, DATA_MESSAGE_SIZE_BYTES);
   //RBUDisableInd_Count = 0;
   //RBUDisableInd_Handle = 0;
   //RBUDisableInd_UnitAddress = 0;
   SMHandleEvent(EVENT_RBU_DISABLE_E, (uint8_t*)&cmdInd);
   /* the message should have been propogated to the DLCCHSQ */
   CU_ASSERT_EQUAL(osMessageCount(DLCCHSQ), 1);
   /* the application callback should not have been made */
//   CU_ASSERT_EQUAL(RBUDisableInd_Count, 0);


   /* Test the case where the secondary Parent Node has been disabled */
   /* We should be starting with a secondary parent, node id = rbu_node2_id */
   CU_ASSERT_TRUE(MC_SMGR_IsParentNode(rbu_node2_id));

   cmdInd.Type = CO_MESSAGE_PHY_DATA_IND_E;
   cmdInd.Payload.PhyDataInd.Size = DATA_MESSAGE_SIZE_BYTES;
   memcpy(cmdInd.Payload.PhyDataInd.Data, validMsgDisableNode2, DATA_MESSAGE_SIZE_BYTES);

   osPoolFlush(MeshPool);
   osMessageQFlush(DLCCHSQ);
   //RBUDisableInd_Count = 0;
   //RBUDisableInd_Handle = 1;
   //RBUDisableInd_UnitAddress = 0;
   //insert node id 2 into the tracking node list to check that it is removed by the call.
   gaTrackingNodes.aTrackingNodeList[SECONDARY_PARENT].node_id = rbu_node2_id;
   gaTrackingNodes.aTrackingNodeList[SECONDARY_PARENT].assigned = true;
   gaMeshNeighbourList[rbu_node2_id].Bitfield.InUse = true;

   SMHandleEvent(EVENT_RBU_DISABLE_E, (uint8_t*)&cmdInd);
   /* The message should have been propogated to the DLCCHSQ */
   CU_ASSERT_EQUAL(osMessageCount(DLCCHSQ), 1);
   /* The application IND callback should NOT have been made */
//   CU_ASSERT_EQUAL(RBUDisableInd_Count, 0);
   /* The secondary parent (rbu_node2_id) should have been removed */
   CU_ASSERT_FALSE(MC_SMGR_IsParentNode(rbu_node2_id));
   /* The secondary parent node should no be a tracking node anymore */
   CU_ASSERT_FALSE(MC_IsTrackingNode(node2_dch_slot));

   /* Test with the local node ID */
   cmdInd.Type = CO_MESSAGE_PHY_DATA_IND_E;
   cmdInd.Payload.PhyDataInd.Size = DATA_MESSAGE_SIZE_BYTES;
   memcpy(cmdInd.Payload.PhyDataInd.Data, validMsgDisableNode1, DATA_MESSAGE_SIZE_BYTES);

   osPoolFlush(MeshPool);
   osMessageQFlush(DLCCHSQ);
   //RBUDisableInd_Count = 0;
   //RBUDisableInd_Handle = 1;
   //RBUDisableInd_UnitAddress = 0;
   SMHandleEvent(EVENT_RBU_DISABLE_E, (uint8_t*)&cmdInd);
   /* the message should have been propogated to the DLCCHSQ */
   CU_ASSERT_EQUAL(osMessageCount(DLCCHSQ), 1);
   /* the application callback should have been made */
   //CU_ASSERT_EQUAL(RBUDisableInd_Count, 1);
   //CU_ASSERT_EQUAL(RBUDisableInd_Handle, 0);
   //CU_ASSERT_EQUAL(RBUDisableInd_UnitAddress, 1);

}

/*************************************************************************************/
/**
* SM_GenerateRouteDropTest
* Test the GenerateRouteDrop function
*
* no params
*
* @return - none

*/
static void SM_GenerateRouteDropTest(void)
{
   CO_State_t state;
   uint16_t rbu_node1_id = 1;
   uint16_t rbu_node2_id = 2;
   uint32_t system_id = SYSTEM_ID;
   const bool bNCU = false;

   RACHSQ = osMessageCreate(osMessageQ(RACHSQ), NULL);
   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));
   osPoolFlush(MeshPool);
   osMessageQFlush(RACHSQ);
   CU_ASSERT_EQUAL(osMessageCount(RACHSQ), 0);

   SMInitialise(bNCU, rbu_node1_id, system_id);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

   SM_SetCurrentDeviceState(STATE_ACTIVE_E);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);


   /* send a command to generate a Route Drop message */
   bool result = false;
   osStatus osStat = osErrorOS;
   CO_Message_t cmdReq;
   RouteDropData_t routeDropData = { 0 };

   routeDropData.NodeID = rbu_node2_id;
   routeDropData.WaitForAck = false;

   cmdReq.Type = CO_MESSAGE_GENERATE_RBU_DISABLE_MESSAGE_E;
   memcpy(cmdReq.Payload.PhyDataReq.Data, &routeDropData, sizeof(RouteDropData_t));

   SMHandleEvent(EVENT_GENERATE_ROUTE_DROP_REQUEST_E, (uint8_t*)&cmdReq);

   /* There should be a message in the RACHSQ */
   CU_ASSERT_EQUAL(osMessageCount(RACHSQ), 1);

   /* Check for a generated message */
   osEvent Event = osMessageGet(RACHSQ, 0);
   CU_ASSERT_EQUAL(Event.status, osEventMessage);
   CO_Message_t* pMessage = (CO_Message_t*)Event.value.p;
   CU_ASSERT_TRUE(pMessage != NULL);
   if (pMessage)
   {
      /* check the message length */
      CU_ASSERT_EQUAL(pMessage->Payload.PhyDataReq.Size, DATA_MESSAGE_SIZE_BYTES);

      /* check the frame type */
      uint8_t frame_type = pMessage->Payload.PhyDataReq.Data[0] >> 4;
      CU_ASSERT_EQUAL(frame_type, (uint8_t)FRAME_TYPE_DATA_E);

      /* check the message type */
      uint8_t msg_type = (((pMessage->Payload.PhyDataReq.Data[7] << 1) | (pMessage->Payload.PhyDataReq.Data[8] >> 7)) & 0x1F);
      CU_ASSERT_EQUAL(msg_type, (uint8_t)APP_MSG_TYPE_ROUTE_DROP_E);

      /* check it's correctly addressed */
      uint16_t node_id = ((uint16_t)pMessage->Payload.PhyDataReq.Data[0] << 8) & 0x0f00;
      node_id |= (uint16_t)pMessage->Payload.PhyDataReq.Data[1];
      CU_ASSERT_EQUAL(node_id, rbu_node2_id);
   }
}

/*************************************************************************************/
/**
* SM_RouteDropTest
* Test the routeDrop function
*
* no params
*
* @return - none

*/
static void SM_RouteDropTest(void)
{
   extern SMGR_ParentDetails_t gParent[NUMBER_OF_PARENTS_AND_TRACKING_NODES];
   extern SMGR_ChildDetails_t gChildren[MAX_NUMBER_OF_CHILDREN];

   CO_State_t state;
   uint16_t rbu_id = 10;
   uint16_t dropped_child_id = 11;
   uint32_t system_id = SYSTEM_ID;
   const bool bNCU = false;
   const uint16_t primary_parent_id = 1;
   const uint16_t primary_parent_slot = 17;
   const uint16_t secondary_parent_id = 2;
   const uint16_t secondary_parent_slot = 18;
   const uint16_t primary_tracking_id = 3;
   const uint16_t primary_tracking_slot = 19;
   const uint16_t secondary_tracking_id = 4;
   const uint16_t secondary_tracking_slot = 36;

   RACHSQ = osMessageCreate(osMessageQ(RACHSQ), NULL);
   ACKQ = osMessageCreate(osMessageQ(ACKQ), NULL);
   osMessageQFlush(ACKQ);
   osMessageQFlush(RACHSQ);
   MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));
   osPoolFlush(MeshPool);
   CU_ASSERT_EQUAL(osMessageCount(ACKQ), 0);
   CU_ASSERT_EQUAL(osMessageCount(RACHSQ), 0);

   SMInitialise(bNCU, rbu_id, system_id);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_CONFIG_SYNC_E);

   MC_SMGR_Initialise(false, rbu_id);
   //Fill the parent and tracking node array
   gParent[PRIMARY_PARENT].NodeInfo = primary_parent_id;
   gParent[PRIMARY_PARENT].IsActive = true;
   gParent[SECONDARY_PARENT].NodeInfo = secondary_parent_id;
   gParent[SECONDARY_PARENT].IsActive = true;
   gParent[PRIMARY_TRACKING_NODE].NodeInfo = primary_tracking_id;
   gParent[SECONDARY_TRACKING_NODE].NodeInfo = secondary_tracking_id;
   gaTrackingNodes.aTrackingNodeList[PRIMARY_PARENT].slotIdx = primary_parent_slot;
   gaTrackingNodes.aTrackingNodeList[SECONDARY_PARENT].slotIdx = secondary_parent_slot;
   gaTrackingNodes.aTrackingNodeList[PRIMARY_TRACKING_NODE].slotIdx = primary_tracking_slot;
   gaTrackingNodes.aTrackingNodeList[SECONDARY_TRACKING_NODE].slotIdx = secondary_tracking_slot;
   gaTrackingNodes.noOfTrackingNodes = 4;
   // Add a child node 
   CU_ASSERT_TRUE(MC_SMGR_RouteAddRequest(dropped_child_id, 0));

   SM_SetCurrentDeviceState(STATE_ACTIVE_E);
   state = SM_GetCurrentDeviceState();
   CU_ASSERT_EQUAL(state, STATE_ACTIVE_E);

   //create RouteDrop payloads for a parent, a tracking node and a child
   uint8_t dropPrimaryParent[DATA_MESSAGE_SIZE_BYTES]       = { 0x10,0x0a,0x00,0x10,0x00,0x0a,0x00,0x15,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23,0x45,0x67,0x80 };
   uint8_t dropPrimaryTrackingNode[DATA_MESSAGE_SIZE_BYTES] = { 0x10,0x0a,0x00,0x30,0x00,0x0a,0x00,0x35,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23,0x45,0x67,0x80 };
   uint8_t dropChild[DATA_MESSAGE_SIZE_BYTES]               = { 0x10,0x0a,0x00,0xb0,0x00,0x0a,0x00,0xb5,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x23,0x45,0x67,0x80 };

   //create a message to drop the child
   CO_Message_t cmdRouteDrop;
   cmdRouteDrop.Type = CO_MESSAGE_GENERATE_RBU_DISABLE_MESSAGE_E;
   memcpy(cmdRouteDrop.Payload.PhyDataReq.Data, dropChild, DATA_MESSAGE_SIZE_BYTES);

   SMHandleEvent(EVENT_ROUTE_DROP_E, (uint8_t*)&cmdRouteDrop);
   /* check that the message was ACK'd */
   CU_ASSERT_EQUAL(osMessageCount(ACKQ), 1);
   /* check that the child record was reset */
   CU_ASSERT_EQUAL(gChildren[0].NodeInfo, NOT_ASSIGNED);

   osMessageQFlush(ACKQ);

   //create a message to drop the primary tracking node
   memcpy(cmdRouteDrop.Payload.PhyDataReq.Data, dropPrimaryTrackingNode, DATA_MESSAGE_SIZE_BYTES);

   SMHandleEvent(EVENT_ROUTE_DROP_E, (uint8_t*)&cmdRouteDrop);
   /* check that the message was ACK'd */
   CU_ASSERT_EQUAL(osMessageCount(ACKQ), 1);
   /* Check that the tracking node record was updated.
      The secondary tracking node should now be primary*/
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, secondary_tracking_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);

   //restore the test conditions
   osMessageQFlush(ACKQ);
   gParent[PRIMARY_TRACKING_NODE].NodeInfo = primary_tracking_id;
   gParent[SECONDARY_TRACKING_NODE].NodeInfo = secondary_tracking_id;


   //create a message to drop the primary parent
   memcpy(cmdRouteDrop.Payload.PhyDataReq.Data, dropPrimaryParent, DATA_MESSAGE_SIZE_BYTES);

   SMHandleEvent(EVENT_ROUTE_DROP_E, (uint8_t*)&cmdRouteDrop);
   /* check that the message was ACK'd */
   CU_ASSERT_EQUAL(osMessageCount(ACKQ), 1);
   /* check that the parent and tracking node records were updated */
   CU_ASSERT_EQUAL(gParent[PRIMARY_PARENT].NodeInfo, secondary_parent_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_PARENT].NodeInfo, primary_tracking_id);
   CU_ASSERT_EQUAL(gParent[PRIMARY_TRACKING_NODE].NodeInfo, secondary_tracking_id);
   CU_ASSERT_EQUAL(gParent[SECONDARY_TRACKING_NODE].NodeInfo, NOT_ASSIGNED);

}