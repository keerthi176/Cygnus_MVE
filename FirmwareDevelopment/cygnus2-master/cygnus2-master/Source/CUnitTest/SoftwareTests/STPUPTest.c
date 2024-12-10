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
*  File         : STPUPTest.c
*
*  Description  : Implementation for the Mesh Message Packing and Unpacking tests
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
#include "STPUPTest.h"
#include "MC_PUP.h"
#include "CO_Defines.h"

/* Private definitions
*************************************************************************************/
#define SHIFT_REG_SIZE_BYTES                          64
#define SHIFT_REG_SIZE_BITS                           (SHIFT_REG_SIZE_BYTES * BITS_PER_BYTE)

/* Private Functions Prototypes
*************************************************************************************/
static void MC_PUP_ParameterGetTest(void);
static void MC_PUP_GetMessageTypeValidDataTest(void);
static void MC_PUP_GetMessageTypeInvalidDataTest(void);
static void MC_PUP_PackHeartbeatValidDataTest(void);
static void MC_PUP_PackHeartbeatInvalidDataTest(void);
static void MC_PUP_PackAlarmSignalValidDataTest(void);
static void MC_PUP_PackAlarmSignalInvalidDataTest(void);
static void MC_PUP_PackAckValidDataTest(void);
static void MC_PUP_PackAckInvalidDataTest(void);
static void MC_PUP_PackFaultSignalValidDataTest(void);
static void MC_PUP_PackFaultSignalInvalidDataTest(void);
static void MC_PUP_PackLogOnMsgValidDataTest(void);
static void MC_PUP_PackLogOnMsgInvalidDataTest(void);
static void MC_PUP_PackStatusIndMsgValidDataTest(void);
static void MC_PUP_PackStatusIndMsgInvalidDataTest(void);
static void MC_PUP_PackRouteAddValidDataTest(void);
static void MC_PUP_PackRouteAddInvalidDataTest(void);
static void MC_PUP_PackRouteDropValidDataTest(void);
static void MC_PUP_PackRouteDropInvalidDataTest(void);
static void MC_PUP_PackResponseMsgInvalidDataTest(void);
static void MC_PUP_PackResponseMsgAnalogueValueTest(void);
static void MC_PUP_PackResponseMsgNeighbourInfoTest(void);
static void MC_PUP_PackResponseMsgRebootTest(void);
static void MC_PUP_PackCommandMsgAnalogueValueTest(void);
static void MC_PUP_PackOutputSignalValidDataTest(void);
static void MC_PUP_PackOutputSignalInvalidDataTest(void);
static void MC_PUP_PackSetAddressValidDataTest(void);
static void MC_PUP_PackSetAddressInvalidDataTest(void);
static void MC_PUP_PackSetStateValidDataTest(void);
static void MC_PUP_PackSetStateInvalidDataTest(void);
static void MC_PUP_PackRouteAddResponseValidDataTest(void);
static void MC_PUP_PackRouteAddResponseInvalidDataTest(void);
static void MC_PUP_PackLoadBalanceValidDataTest(void);
static void MC_PUP_PackLoadBalanceInvalidDataTest(void);
static void MC_PUP_PackLoadBalanceResponseValidDataTest(void);
static void MC_PUP_PackLoadBalanceResponseInvalidDataTest(void);
static void MC_PUP_PackRBUDisableValidDataTest(void);
static void MC_PUP_PackRBUDisableInvalidDataTest(void);
static void MC_PUP_PackOutputStateMessageTest(void);

/* Global Variables
*************************************************************************************/
/* Table containing the test settings */
CU_TestInfo ST_PUPTests[] =
{
   { "ParameterGetTest",                   MC_PUP_ParameterGetTest },
   { "GetMessageTypeValidDataTest",        MC_PUP_GetMessageTypeValidDataTest },
   { "GetMessageTypeInvalidDataTest",      MC_PUP_GetMessageTypeInvalidDataTest },
   { "PackHeartbeatValidDataTest",         MC_PUP_PackHeartbeatValidDataTest },
   { "PackHeartbeatInvalidDataTest",       MC_PUP_PackHeartbeatInvalidDataTest },
   { "PackAlarmSignalValidDataTest",       MC_PUP_PackAlarmSignalValidDataTest },
   { "PackAlarmSignalInvalidDataTest",     MC_PUP_PackAlarmSignalInvalidDataTest },
   { "PackAckValidDataTest",               MC_PUP_PackAckValidDataTest },
   { "PackAckInvalidDataTest",             MC_PUP_PackAckInvalidDataTest },
   { "PackFaultSignalValidDataTest",       MC_PUP_PackFaultSignalValidDataTest },
   { "PackFaultSignalInvalidDataTest",     MC_PUP_PackFaultSignalInvalidDataTest },
   { "PackLogOnMsgValidDataTest",          MC_PUP_PackLogOnMsgValidDataTest },
   { "PackLogOnMsgInvalidDataTest",        MC_PUP_PackLogOnMsgInvalidDataTest },
   { "PackStatusIndMsgValidDataTest",      MC_PUP_PackStatusIndMsgValidDataTest },
   { "PackStatusIndMsgInvalidDataTest",    MC_PUP_PackStatusIndMsgInvalidDataTest },
   { "PackRouteAddValidDataTest",          MC_PUP_PackRouteAddValidDataTest },
   { "PackRouteAddInvalidDataTest",        MC_PUP_PackRouteAddInvalidDataTest },
   { "PackRouteDropValidDataTest",         MC_PUP_PackRouteDropValidDataTest },
   { "PackRouteDropInvalidDataTest",       MC_PUP_PackRouteDropInvalidDataTest },
   { "PackResponseMsgInvalidDataTest",     MC_PUP_PackResponseMsgInvalidDataTest },
   { "PackResponseMsgAnalogueValueTest",   MC_PUP_PackResponseMsgAnalogueValueTest },
   { "PackResponseMsgNeighbourInfoTest",   MC_PUP_PackResponseMsgNeighbourInfoTest },
   { "PackResponseMsgRebootTest",          MC_PUP_PackResponseMsgRebootTest },
   { "PackCommandMsgAnalogueValueTest",    MC_PUP_PackCommandMsgAnalogueValueTest },
   { "PackOutputSignalValidDataTest",      MC_PUP_PackOutputSignalValidDataTest },
   { "PackOutputSignalInvalidDataTest",    MC_PUP_PackOutputSignalInvalidDataTest },
   { "PackSetAddressValidDataTest",        MC_PUP_PackSetAddressValidDataTest },
   { "PackSetAddressInvalidDataTest",      MC_PUP_PackSetAddressInvalidDataTest },
   { "PackSetStateValidDataTest",          MC_PUP_PackSetStateValidDataTest },
   { "PackSetStateInvalidDataTest",        MC_PUP_PackSetStateInvalidDataTest },
   { "PackRouteAddResponseValidDataTest",  MC_PUP_PackRouteAddResponseValidDataTest },
   { "PackRouteAddResponseInvalidDataTest",MC_PUP_PackRouteAddResponseInvalidDataTest },
   { "PackLoadBalanceValidDataTest",       MC_PUP_PackLoadBalanceValidDataTest },
   { "PackLoadBalanceInvalidDataTest",     MC_PUP_PackLoadBalanceInvalidDataTest },
   { "PackLoadBalanceResponseValidDataTest",MC_PUP_PackLoadBalanceResponseValidDataTest },
   { "PackLoadBalanceResponseInvalidDataTest",MC_PUP_PackLoadBalanceResponseInvalidDataTest },
   { "PackRBUDisableValidDataTest",        MC_PUP_PackRBUDisableValidDataTest },
   { "PackRBUDisableInvalidDataTest",      MC_PUP_PackRBUDisableInvalidDataTest },
   { "PackOutputStateMessageTest",         MC_PUP_PackOutputStateMessageTest },
CU_TEST_INFO_NULL
};



/* Private Variables
*************************************************************************************/


/*************************************************************************************/
/**
* MC_PUP_ParameterGetTest
* test unpacking of data
*
* no params
*
* @return - none

*/
static void MC_PUP_ParameterGetTest(void)
{
   uint32_t result;

   uint8_t buf0[] = { 0x03,0xc1,0x23 };
   result = MC_PUP_ParameterGet(buf0, 12, 12);
   CU_ASSERT_EQUAL(result, 0x123);

   uint8_t buf1[] = { 0x00,0x80,0x00 };
   result = MC_PUP_ParameterGet(buf1, 8, 1);
   CU_ASSERT_EQUAL(result, 1);

   uint8_t buf2[] = { 0x00,0x08,0x00 };
   result = MC_PUP_ParameterGet(buf2, 12, 1);
   CU_ASSERT_EQUAL(result, 1);

   uint8_t buf3[] = { 0x00,0x01,0x00 };
   result = MC_PUP_ParameterGet(buf3, 15, 1);
   CU_ASSERT_EQUAL(result, 1);

   uint8_t buf4[] = { 0xff,0x7f,0xff };
   result = MC_PUP_ParameterGet(buf4, 8, 1);
   CU_ASSERT_EQUAL(result, 0);

   uint8_t buf5[] = { 0xff,0xf7,0xff };
   result = MC_PUP_ParameterGet(buf5, 12, 1);
   CU_ASSERT_EQUAL(result, 0);

   uint8_t buf6[] = { 0xff,0xfe,0xff };
   result = MC_PUP_ParameterGet(buf6, 15, 1);
   CU_ASSERT_EQUAL(result, 0);

   uint8_t buf7[] = { 0x00,0x81,0x23, 0x45, 0x67, 0x89, 0x22 };
   result = MC_PUP_ParameterGet(buf7, 12, 32);
   CU_ASSERT_EQUAL(result, 0x12345678);

   result = MC_PUP_ParameterGet(buf7, 8, 32);
   CU_ASSERT_EQUAL(result, 0x81234567);

   uint8_t buf8[] = { 0x50,0x01,0x04, 0xD2, 0x44, 0x43, 0x42 };
   result = MC_PUP_ParameterGet(buf8, 0, 4);
   CU_ASSERT_EQUAL(result, 0x05);

   uint8_t buf9[] = { 0x53,0xc1,0x23 };
   result = MC_PUP_ParameterGet(buf8, 0, 4);
   CU_ASSERT_EQUAL(result, 0x05);
}

/*************************************************************************************/
/**
* MC_PUP_GetMessageTypeValidDataTest
* test unpacking of valid data
*
* no params
*
* @return - none

*/
static void MC_PUP_GetMessageTypeValidDataTest(void)
{
   CO_MessagePayloadPhyDataInd_t *pPhyDataInd;
   FrameType_t frameType;
   bool appMsgPresent;
   ApplicationLayerMessageType_t appMsgType;
   ErrorCode_t status = SUCCESS_E;
   uint8_t validHeartbeat[] = { 0x00 };
   uint8_t validFireSignal[] = { 0x12, 0x22, 0x33, 0x34, 0x45, 0x55, 0x66, 0x60, 0x78, 0x80 };


   pPhyDataInd = (CO_MessagePayloadPhyDataInd_t *)validHeartbeat;
   frameType = 0;
   appMsgPresent = false;
   appMsgType = 0;
   status = MC_PUP_GetMessageType(pPhyDataInd, &frameType, &appMsgPresent, &appMsgType);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(frameType, FRAME_TYPE_HEARTBEAT_E);
   CU_ASSERT_EQUAL(appMsgPresent, false);

   pPhyDataInd = (CO_MessagePayloadPhyDataInd_t *)validFireSignal;
   frameType = 0;
   appMsgPresent = false;
   appMsgType = 0;
   status = MC_PUP_GetMessageType(pPhyDataInd, &frameType, &appMsgPresent, &appMsgType);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(frameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(appMsgPresent, true);
   CU_ASSERT_EQUAL(appMsgType, APP_MSG_TYPE_FIRE_SIGNAL_E);
}

/*************************************************************************************/
/**
* MC_PUP_GetMessageTypeInvalidDataTest
* test unpacking of invalid data
*
* no params
*
* @return - none

*/
static void MC_PUP_GetMessageTypeInvalidDataTest(void)
{
   CO_MessagePayloadPhyDataInd_t *pPhyDataInd;
   FrameType_t frameType;
   bool appMsgPresent;
   ApplicationLayerMessageType_t appMsgType;
   ErrorCode_t status = SUCCESS_E;
   uint8_t minInvalidFrameType[] = { 0x60 };
   uint8_t maxInvalidFrameType[] = { 0xf0 };
   uint8_t minInvalidMessageType[] = { 0x12, 0x22, 0x33, 0x34, 0x45, 0x55, 0x66, 0x6B, 0x78, 0x80 };
   uint8_t maxInvalidMessageType[] = { 0x12, 0x22, 0x33, 0x34, 0x45, 0x55, 0x66, 0x6f, 0x78, 0x80 };

   pPhyDataInd = (CO_MessagePayloadPhyDataInd_t *)minInvalidFrameType;
   frameType = 0;
   appMsgPresent = false;
   appMsgType = 0;
   status = MC_PUP_GetMessageType(pPhyDataInd, &frameType, &appMsgPresent, &appMsgType);
   CU_ASSERT_EQUAL(status, ERR_OUT_OF_RANGE_E);

   pPhyDataInd = (CO_MessagePayloadPhyDataInd_t *)maxInvalidFrameType;
   frameType = 0;
   appMsgPresent = false;
   appMsgType = 0;
   status = MC_PUP_GetMessageType(pPhyDataInd, &frameType, &appMsgPresent, &appMsgType);
   CU_ASSERT_EQUAL(status, ERR_OUT_OF_RANGE_E);

   //pPhyDataInd = (CO_MessagePayloadPhyDataInd_t *)minInvalidMessageType;
   //frameType = 0;
   //appMsgPresent = false;
   //appMsgType = 0;
   //status = MC_PUP_GetMessageType(pPhyDataInd, &frameType, &appMsgPresent, &appMsgType);
   //CU_ASSERT_EQUAL(status, ERR_OUT_OF_RANGE_E);

   //pPhyDataInd = (CO_MessagePayloadPhyDataInd_t *)maxInvalidMessageType;
   //frameType = 0;
   //appMsgPresent = false;
   //appMsgType = 0;
   //status = MC_PUP_GetMessageType(pPhyDataInd, &frameType, &appMsgPresent, &appMsgType);
   //CU_ASSERT_EQUAL(status, ERR_OUT_OF_RANGE_E);
}

/*************************************************************************************/
/**
* MC_PUP_PackHeartbeatValidDataTest
* test packing of valid data
*
* no params
*
* @return - none

*/
void MC_PUP_PackHeartbeatValidDataTest(void)
{
	ErrorCode_t status = SUCCESS_E;
	FrameHeartbeat_t heartbeat = { 0 };
	FrameHeartbeat_t heartbeat2 = { 0 };
	CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
	CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
	uint32_t size = 0;

	heartbeat.FrameType = FRAME_TYPE_HEARTBEAT_E;
	heartbeat.SlotIndex = 17;
	heartbeat.ShortFrameIndex = 2;
	heartbeat.LongFrameIndex = 3;
	heartbeat.State = 4;
	heartbeat.Rank = 6;
   heartbeat.NoOfChildrenIdx = 7;
   heartbeat.NoOfChildrenOfPTNIdx = 7;
   heartbeat.SystemId = 8;

	memset(&phyDataReq, 0, sizeof(phyDataReq));
	status = MC_PUP_PackHeartbeat(&heartbeat, &phyDataReq);
	CU_ASSERT_EQUAL(status, SUCCESS_E);
	CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x02);
	CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x04);
	CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x1A);
	CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x0C);
	CU_ASSERT_EQUAL(phyDataReq.Data[4], 0xEE);
	CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x00);
	CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x00);
	CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x00);
	CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x10);
	CU_ASSERT_EQUAL(phyDataReq.Size, 9);

	/* map across to phy data ind message */
	size = phyDataReq.Size;
	memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
	phyDataInd.Size = size;
	phyDataInd.RSSI = 23;
	phyDataInd.SNR = 45;

	/* unpack phydata ind */
	status = MC_PUP_UnpackHeartbeat(&phyDataInd, &heartbeat2);
	CU_ASSERT_EQUAL(status, SUCCESS_E);
	CU_ASSERT_EQUAL(heartbeat2.FrameType, FRAME_TYPE_HEARTBEAT_E);
	CU_ASSERT_EQUAL(heartbeat2.SlotIndex, 17);
	CU_ASSERT_EQUAL(heartbeat2.ShortFrameIndex, 2);
	CU_ASSERT_EQUAL(heartbeat2.LongFrameIndex, 3);
	CU_ASSERT_EQUAL(heartbeat2.State, 4);
	CU_ASSERT_EQUAL(heartbeat2.Rank, 6);
   CU_ASSERT_EQUAL(heartbeat2.NoOfChildrenIdx, 7);
   CU_ASSERT_EQUAL(heartbeat2.NoOfChildrenOfPTNIdx, 7);
   CU_ASSERT_EQUAL(heartbeat2.SystemId, 8);

}

/*************************************************************************************/
/**
* MC_PUP_PackHeartbeatInvalidDataTest
* test for rejection of invalid data
*
* no params
*
* @return - none

*/
void MC_PUP_PackHeartbeatInvalidDataTest(void)
{
	ErrorCode_t status = SUCCESS_E;
	FrameHeartbeat_t heartbeat = { 0 };
	FrameHeartbeat_t heartbeat2 = { 0 };
	CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
	CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
	uint32_t size = 0;

   heartbeat.FrameType = FRAME_TYPE_HEARTBEAT_E;
   heartbeat.SlotIndex = 1;
   heartbeat.ShortFrameIndex = 2;
   heartbeat.LongFrameIndex = 3;
   heartbeat.State = 4;
   heartbeat.Rank = 6;
   heartbeat.NoOfChildrenIdx = 7;
   heartbeat.NoOfChildrenOfPTNIdx = 0;
   heartbeat.SystemId = 8;

	//Check Pack for NULL parameter rejection
	status = MC_PUP_PackHeartbeat(&heartbeat, NULL);
	CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

	status = MC_PUP_PackHeartbeat(NULL, &phyDataReq);
	CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);


	//Check Unpack for NULL parameter rejection
	status = MC_PUP_UnpackHeartbeat(&phyDataInd, NULL);
	CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

	status = MC_PUP_UnpackHeartbeat(NULL, &heartbeat2);
	CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   // check heartbeat value rejection
   heartbeat.FrameType = 17;
   CU_ASSERT_EQUAL(MC_PUP_PackHeartbeat(&heartbeat, &phyDataReq), ERR_OUT_OF_RANGE_E);
   heartbeat.FrameType = FRAME_TYPE_HEARTBEAT_E;
   heartbeat.SlotIndex = 32;
   CU_ASSERT_EQUAL(MC_PUP_PackHeartbeat(&heartbeat, &phyDataReq), ERR_OUT_OF_RANGE_E);
   heartbeat.SlotIndex = 1;
   heartbeat.ShortFrameIndex = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackHeartbeat(&heartbeat, &phyDataReq), ERR_OUT_OF_RANGE_E);
   heartbeat.ShortFrameIndex = 2;
   heartbeat.LongFrameIndex = 16;
   CU_ASSERT_EQUAL(MC_PUP_PackHeartbeat(&heartbeat, &phyDataReq), ERR_OUT_OF_RANGE_E);
   heartbeat.LongFrameIndex = 3;
   heartbeat.State = 16;
   CU_ASSERT_EQUAL(MC_PUP_PackHeartbeat(&heartbeat, &phyDataReq), ERR_OUT_OF_RANGE_E);
   heartbeat.State = 4;
   heartbeat.Rank = 64;
   CU_ASSERT_EQUAL(MC_PUP_PackHeartbeat(&heartbeat, &phyDataReq), ERR_OUT_OF_RANGE_E);
   heartbeat.Rank = 6;
   heartbeat.NoOfChildrenIdx = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackHeartbeat(&heartbeat, &phyDataReq), ERR_OUT_OF_RANGE_E);

   /* check that the unpack function detects out of range values */
   /* bad frame type */
   uint8_t badFrameType[11] = { 0x10,0x81,0x1a,0x00,0x28,0xc0,0xe0,0x00,0x00,0x01,0x00 };
   memcpy(&phyDataInd.Data, badFrameType, 11);
   phyDataInd.Size = 11;
   CU_ASSERT_EQUAL(MC_PUP_UnpackHeartbeat(&phyDataInd, &heartbeat), ERR_OUT_OF_RANGE_E);
   /* bad slot number */
   uint8_t badSlotNumber[11] = { 0x0a,0x01,0x1a,0x00,0x28,0xc0,0xe0,0x00,0x00,0x01,0x00 };
   memcpy(&phyDataInd.Data, badSlotNumber, 11);
   CU_ASSERT_EQUAL(MC_PUP_UnpackHeartbeat(&phyDataInd, &heartbeat), ERR_OUT_OF_RANGE_E);
   /* bad short frame */
   uint8_t badShortFrame[11] = { 0x01,0x11,0x1a,0x00,0x28,0xc0,0xe0,0x00,0x00,0x01,0x00 };
   memcpy(&phyDataInd.Data, badShortFrame, 11);
   CU_ASSERT_EQUAL(MC_PUP_UnpackHeartbeat(&phyDataInd, &heartbeat), ERR_OUT_OF_RANGE_E);

   /* The remaining heartbeat parameters use their full range, so we can't inject invalid values. */
}

/*************************************************************************************/
/**
* MC_PUP_PackAlarmSignalValidDataTest
* Test packing of valid Alarm Signal data
*
* no params
*
* @return - none

*/
void MC_PUP_PackAlarmSignalValidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   AlarmSignal_t alarmSignal = { 0 };
   AlarmSignal_t alarmSignal2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   alarmSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   alarmSignal.Header.MACDestinationAddress = 0x101;
   alarmSignal.Header.MACSourceAddress = 0x202;
   alarmSignal.Header.HopCount = 4;
   alarmSignal.Header.DestinationAddress = 0x303;
   alarmSignal.Header.SourceAddress = 0x404;
   alarmSignal.Header.MessageType = APP_MSG_TYPE_ALARM_SIGNAL_E;
   alarmSignal.RUChannel = 5;
   alarmSignal.SensorValue = 0xa5;
   alarmSignal.Zone = 1;
   alarmSignal.SystemId = 0xffffffff;


   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackAlarmSignal(&alarmSignal, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x11);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x43);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x03);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x96);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x94);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x04);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0x0f);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0xf0);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackAlarmSignal(&phyDataInd, &alarmSignal2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(alarmSignal2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(alarmSignal2.Header.MACDestinationAddress, 0x101);
   CU_ASSERT_EQUAL(alarmSignal2.Header.MACSourceAddress, 0x202);
   CU_ASSERT_EQUAL(alarmSignal2.Header.HopCount, 0x04);
   CU_ASSERT_EQUAL(alarmSignal2.Header.DestinationAddress, 0x303);
   CU_ASSERT_EQUAL(alarmSignal2.Header.SourceAddress, 0x404);
   CU_ASSERT_EQUAL(alarmSignal2.Header.MessageType, APP_MSG_TYPE_ALARM_SIGNAL_E);
   CU_ASSERT_EQUAL(alarmSignal2.RUChannel, 0x05);
   CU_ASSERT_EQUAL(alarmSignal2.SensorValue, 0xa5);
   CU_ASSERT_EQUAL(alarmSignal2.Zone, 1);
   CU_ASSERT_EQUAL(alarmSignal2.SystemId, 0xffffffff);
}

/*************************************************************************************/
/**
* MC_PUP_PackAlarmSignalInvalidDataTest
* Test packing of invalid Alarm Signal data
*
* no params
*
* @return - none

*/
void MC_PUP_PackAlarmSignalInvalidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   AlarmSignal_t alarmSignal = { 0 };
   AlarmSignal_t alarmSignal2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   alarmSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   alarmSignal.Header.MACDestinationAddress = 0x101;
   alarmSignal.Header.MACSourceAddress = 0x202;
   alarmSignal.Header.HopCount = 4;
   alarmSignal.Header.DestinationAddress = 0x303;
   alarmSignal.Header.SourceAddress = 0x404;
   alarmSignal.Header.MessageType = 0;
   alarmSignal.RUChannel = 5;
   alarmSignal.SensorValue = 0xa5;
   alarmSignal.SystemId = 0xffffffff;

   /* pack */
   status = MC_PUP_PackAlarmSignal(NULL, &phyDataReq);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_PackAlarmSignal(&alarmSignal, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* unpack */
   status = MC_PUP_UnpackAlarmSignal(NULL, &alarmSignal2);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_UnpackAlarmSignal(&phyDataInd,NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   // check heartbeat value rejection
   alarmSignal.Header.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   CU_ASSERT_EQUAL(MC_PUP_PackAlarmSignal(&alarmSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   alarmSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   alarmSignal.Header.MACDestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackAlarmSignal(&alarmSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   alarmSignal.Header.MACDestinationAddress = 0x101;
   alarmSignal.Header.MACSourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackAlarmSignal(&alarmSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   alarmSignal.Header.MACSourceAddress = 0x202;
   alarmSignal.Header.HopCount = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackAlarmSignal(&alarmSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   alarmSignal.Header.HopCount = 4;
   alarmSignal.Header.DestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackAlarmSignal(&alarmSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   alarmSignal.Header.DestinationAddress = 0x303;
   alarmSignal.Header.SourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackAlarmSignal(&alarmSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   alarmSignal.Header.SourceAddress = 0x404;
   alarmSignal.Header.MessageType = 2;
   CU_ASSERT_EQUAL(MC_PUP_PackAlarmSignal(&alarmSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   alarmSignal.Header.MessageType = 0;
   alarmSignal.RUChannel = 33;
   CU_ASSERT_EQUAL(MC_PUP_PackAlarmSignal(&alarmSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   alarmSignal.RUChannel = 5;
   alarmSignal.SensorValue = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackAlarmSignal(&alarmSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);


   /* check that the unpack function detects out of range values */
   /* bad frame type */
   uint8_t badFrameType[20] = { 0x01,0x01,0x20,0x20,0x43,0x03,0x40,0x40,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badFrameType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackAlarmSignal(&phyDataInd, &alarmSignal), ERR_OUT_OF_RANGE_E);
   /* bad msg type */
   uint8_t badMsgType[20] = { 0x11,0x01,0x20,0x20,0x43,0x03,0x40,0x4b,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badMsgType, 20);
   CU_ASSERT_EQUAL(MC_PUP_UnpackAlarmSignal(&phyDataInd, &alarmSignal), ERR_OUT_OF_RANGE_E);

   /* The remaining heartbeat parameters use their full range, so we can't inject invalid values. */

}

/*************************************************************************************/
/**
* MC_PUP_PackAckValidDataTest
* Test packing of valid Ack data
*
* no params
*
* @return - none

*/
void MC_PUP_PackAckValidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   FrameAcknowledge_t ackSignal = { 0 };
   FrameAcknowledge_t ackSignal2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   ackSignal.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   ackSignal.MACDestinationAddress = 0x101;
   ackSignal.MACSourceAddress = 0x202;
   ackSignal.SystemId = 0xffffffff;

   status = MC_PUP_PackAck(NULL, &phyDataReq);
   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackAck(&ackSignal, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x21);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x2f);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0xf0);
   CU_ASSERT_EQUAL(phyDataReq.Size, 8);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackAck(&phyDataInd, &ackSignal2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(ackSignal2.FrameType, FRAME_TYPE_ACKNOWLEDGEMENT_E);
   CU_ASSERT_EQUAL(ackSignal2.MACDestinationAddress, 0x101);
   CU_ASSERT_EQUAL(ackSignal2.MACSourceAddress, 0x202);
   CU_ASSERT_EQUAL(ackSignal2.SystemId, 0xffffffff);

}

/*************************************************************************************/
/**
* MC_PUP_PackAckInvalidDataTest
* Test packing of invalid Ack data
*
* no params
*
* @return - none

*/
void MC_PUP_PackAckInvalidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   FrameAcknowledge_t ackSignal = { 0 };
   FrameAcknowledge_t ackSignal2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   ackSignal.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   ackSignal.MACDestinationAddress = 0x101;
   ackSignal.MACSourceAddress = 0x202;
   ackSignal.SystemId = 0xffffffff;


   /* pack */
   status = MC_PUP_PackAck(NULL, &phyDataReq);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_PackAck(&ackSignal, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* unpack */
   status = MC_PUP_UnpackAck(NULL, &ackSignal2);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_UnpackAck(&phyDataInd, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* check ack value rejection */
   ackSignal.FrameType = FRAME_TYPE_DATA_E;
   CU_ASSERT_EQUAL(MC_PUP_PackAck(&ackSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   ackSignal.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   ackSignal.MACDestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackAck(&ackSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   ackSignal.MACDestinationAddress = 0x101;
   ackSignal.MACSourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackAck(&ackSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);

   /* check that the unpack function detects out of range values */
   /* bad frame type */
   uint8_t badFrameType[8] = { 0x01,0x01,0x20,0x2f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badFrameType, 8);
   phyDataInd.Size = 8;
   CU_ASSERT_EQUAL(MC_PUP_UnpackAck(&phyDataInd, &ackSignal), ERR_OUT_OF_RANGE_E);

   /* The remaining message parameters use their full range, so we can't inject invalid values. */
}


/*************************************************************************************/
/**
* MC_PUP_PackFaultSignalValidDataTest
* Test packing of valid fault Signal data
*
* no params
*
* @return - none

*/
void MC_PUP_PackFaultSignalValidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   FaultSignal_t faultSignal = { 0 };
   FaultSignal_t faultSignal2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   faultSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   faultSignal.Header.MACDestinationAddress = 0x1;
   faultSignal.Header.MACSourceAddress = 2;
   faultSignal.Header.HopCount = 0;
   faultSignal.Header.DestinationAddress = 0x1;
   faultSignal.Header.SourceAddress = 2;
   faultSignal.Header.MessageType = APP_MSG_TYPE_FAULT_SIGNAL_E;
   faultSignal.Zone = 1;
   faultSignal.StatusFlags = 0x45;
   faultSignal.DetectorFault = 3;
   faultSignal.BeaconFault = 2;
   faultSignal.SounderFault = 3;
   faultSignal.SystemId = 0x12345678;


   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackFaultSignal(&faultSignal, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x10);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x21);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0x82);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x29);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x91);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0x80);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0x23);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0x45);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0x67);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0x80);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackFaultSignal(&phyDataInd, &faultSignal2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(faultSignal2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(faultSignal2.Header.MACDestinationAddress, 1);
   CU_ASSERT_EQUAL(faultSignal2.Header.MACSourceAddress, 2);
   CU_ASSERT_EQUAL(faultSignal2.Header.HopCount, 0);
   CU_ASSERT_EQUAL(faultSignal2.Header.DestinationAddress, 1);
   CU_ASSERT_EQUAL(faultSignal2.Header.SourceAddress, 2);
   CU_ASSERT_EQUAL(faultSignal2.Header.MessageType, APP_MSG_TYPE_FAULT_SIGNAL_E);
   CU_ASSERT_EQUAL(faultSignal2.StatusFlags, 0x45);
   CU_ASSERT_EQUAL(faultSignal2.DetectorFault, 3);
   CU_ASSERT_EQUAL(faultSignal2.BeaconFault, 2);
   CU_ASSERT_EQUAL(faultSignal2.SounderFault, 3);
   CU_ASSERT_EQUAL(faultSignal2.SystemId, 0x12345678);
}

/*************************************************************************************/
/**
* MC_PUP_PackFaultSignalInvalidDataTest
* Test packing of invalid Fault Signal data
*
* no params
*
* @return - none

*/
void MC_PUP_PackFaultSignalInvalidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   FaultSignal_t faultSignal = { 0 };
   FaultSignal_t faultSignal2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   faultSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   faultSignal.Header.MACDestinationAddress = 0x1;
   faultSignal.Header.MACSourceAddress = 2;
   faultSignal.Header.HopCount = 0;
   faultSignal.Header.DestinationAddress = 0x1;
   faultSignal.Header.SourceAddress = 2;
   faultSignal.Header.MessageType = APP_MSG_TYPE_FAULT_SIGNAL_E;
   faultSignal.StatusFlags = 0x45;
   faultSignal.DetectorFault = 3;
   faultSignal.BeaconFault = 2;
   faultSignal.SounderFault = 3;
   faultSignal.SystemId = 0x12345678;

   /* pack */
   status = MC_PUP_PackFaultSignal(NULL, &phyDataReq);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_PackFaultSignal(&faultSignal, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* unpack */
   status = MC_PUP_UnpackFaultSignal(NULL, &faultSignal2);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_UnpackFaultSignal(&phyDataInd, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   // check rejection
   faultSignal.Header.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   CU_ASSERT_EQUAL(MC_PUP_PackFaultSignal(&faultSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   faultSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   faultSignal.Header.MACDestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackFaultSignal(&faultSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   faultSignal.Header.MACDestinationAddress = 1;
   faultSignal.Header.MACSourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackFaultSignal(&faultSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   faultSignal.Header.MACSourceAddress = 2;
   faultSignal.Header.HopCount = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackFaultSignal(&faultSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   faultSignal.Header.HopCount = 0;
   faultSignal.Header.DestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackFaultSignal(&faultSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   faultSignal.Header.DestinationAddress = 1;
   faultSignal.Header.SourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackFaultSignal(&faultSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   faultSignal.Header.SourceAddress = 2;
   faultSignal.Header.MessageType = 0;
   CU_ASSERT_EQUAL(MC_PUP_PackFaultSignal(&faultSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   faultSignal.Header.MessageType = APP_MSG_TYPE_FAULT_SIGNAL_E;
   faultSignal.StatusFlags = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackFaultSignal(&faultSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   faultSignal.StatusFlags = 0x0045;
   faultSignal.DetectorFault = 16;
   CU_ASSERT_EQUAL(MC_PUP_PackFaultSignal(&faultSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   faultSignal.DetectorFault = 3;
   faultSignal.BeaconFault = 16;
   CU_ASSERT_EQUAL(MC_PUP_PackFaultSignal(&faultSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   faultSignal.BeaconFault = 2;
   faultSignal.SounderFault = 16;
   CU_ASSERT_EQUAL(MC_PUP_PackFaultSignal(&faultSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   faultSignal.SounderFault = 3;

   /* check that the unpack function detects out of range values */
   /* bad frame type */
   uint8_t badFrameType[20] = { 0x01,0x01,0x20,0x20,0x33,0x03,0x40,0x42,0x00,0x07,0x50,0x50,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badFrameType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackFaultSignal(&phyDataInd, &faultSignal), ERR_OUT_OF_RANGE_E);
   /* bad msg type */
   uint8_t badMsgType[20] = { 0x11,0x01,0x20,0x20,0x33,0x03,0x40,0x40,0x00,0x07,0x50,0x50,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badMsgType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackFaultSignal(&phyDataInd, &faultSignal), ERR_OUT_OF_RANGE_E);
   /* bad status flags.  Only bits 0-7 should be used.  Bits 8-15 are reserved not used. */
   uint8_t badStatusFlags[20] = { 0x11,0x01,0x20,0x20,0x33,0x03,0x40,0x42,0x00,0x80,0x50,0x50,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badStatusFlags, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackFaultSignal(&phyDataInd, &faultSignal), ERR_OUT_OF_RANGE_E);

   /* The remaining message parameters use their full range, so we can't inject invalid values. */
}

/*************************************************************************************/
/**
* MC_PUP_PacklogOnValidDataTest
* Test packing of valid Log On message data
*
* no params
*
* @return - none

*/
void MC_PUP_PackLogOnMsgValidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   LogOnMessage_t logOnMsg = { 0 };
   LogOnMessage_t logOnMsg2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   logOnMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   logOnMsg.Header.MACDestinationAddress = 0x101;
   logOnMsg.Header.MACSourceAddress = 0x202;
   logOnMsg.Header.HopCount = 3;
   logOnMsg.Header.DestinationAddress = 0x303;
   logOnMsg.Header.SourceAddress = 0x404;
   logOnMsg.Header.MessageType = 6;
   logOnMsg.SerialNumber = 0x01020304;
   logOnMsg.DeviceCombination = 0x11;
   logOnMsg.ZoneNumber = 0x01;
   logOnMsg.SystemId = 0xffffffff;


   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackLogOnMsg(&logOnMsg, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x11);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x33);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x03);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x43);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x81);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x82);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x08);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0x80);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0x8f);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0xf0);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackLogOnMsg(&phyDataInd, &logOnMsg2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(logOnMsg2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(logOnMsg2.Header.MACDestinationAddress, 0x101);
   CU_ASSERT_EQUAL(logOnMsg2.Header.MACSourceAddress, 0x202);
   CU_ASSERT_EQUAL(logOnMsg2.Header.HopCount, 0x03);
   CU_ASSERT_EQUAL(logOnMsg2.Header.DestinationAddress, 0x303);
   CU_ASSERT_EQUAL(logOnMsg2.Header.SourceAddress, 0x404);
   CU_ASSERT_EQUAL(logOnMsg2.Header.MessageType, 0x06);
   CU_ASSERT_EQUAL(logOnMsg2.SerialNumber, 0x01020304);
   CU_ASSERT_EQUAL(logOnMsg2.DeviceCombination, 0x11);
   CU_ASSERT_EQUAL(logOnMsg2.ZoneNumber, 0x01);
   CU_ASSERT_EQUAL(logOnMsg2.SystemId, 0xffffffff);
}

/*************************************************************************************/
/**
* MC_PUP_PackLogOnMsgInvalidDataTest
* Test packing of invalid Log On Message data
*
* no params
*
* @return - none

*/
void MC_PUP_PackLogOnMsgInvalidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   LogOnMessage_t logOnMsg = { 0 };
   LogOnMessage_t logOnMsg2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   logOnMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   logOnMsg.Header.MACDestinationAddress = 0x101;
   logOnMsg.Header.MACSourceAddress = 0x202;
   logOnMsg.Header.HopCount = 4;
   logOnMsg.Header.DestinationAddress = 0x303;
   logOnMsg.Header.SourceAddress = 0x404;
   logOnMsg.Header.MessageType = 6;
   logOnMsg.SerialNumber = 0x01020304;
   logOnMsg.DeviceCombination = 0x22;
   logOnMsg.ZoneNumber = 0x01;
   logOnMsg.SystemId = 0xffffffff;

   /* pack */
   status = MC_PUP_PackLogOnMsg(NULL, &phyDataReq);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_PackLogOnMsg(&logOnMsg, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* unpack */
   status = MC_PUP_UnpackLogOnMsg(NULL, &logOnMsg2);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_UnpackLogOnMsg(&phyDataInd, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   // check heartbeat value rejection
   logOnMsg.Header.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   CU_ASSERT_EQUAL(MC_PUP_PackLogOnMsg(&logOnMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   logOnMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   logOnMsg.Header.MACDestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackLogOnMsg(&logOnMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   logOnMsg.Header.MACDestinationAddress = 0x101;
   logOnMsg.Header.MACSourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackLogOnMsg(&logOnMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   logOnMsg.Header.MACSourceAddress = 0x202;
   logOnMsg.Header.HopCount = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackLogOnMsg(&logOnMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   logOnMsg.Header.HopCount = 3;
   logOnMsg.Header.DestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackLogOnMsg(&logOnMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   logOnMsg.Header.DestinationAddress = 0x303;
   logOnMsg.Header.SourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackLogOnMsg(&logOnMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   logOnMsg.Header.SourceAddress = 0x404;
   logOnMsg.Header.MessageType = 0;
   CU_ASSERT_EQUAL(MC_PUP_PackLogOnMsg(&logOnMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   logOnMsg.Header.MessageType = 6;
   logOnMsg.DeviceCombination = 0x23;
   CU_ASSERT_EQUAL(MC_PUP_PackLogOnMsg(&logOnMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   logOnMsg.DeviceCombination = 0x22;
   logOnMsg.ZoneNumber = 0x1000;
   CU_ASSERT_EQUAL(MC_PUP_PackLogOnMsg(&logOnMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);

   /* check that the unpack function detects out of range values */
   /* bad frame type */
   uint8_t badFrameType[20] = { 0x01,0x01,0x20,0x20,0x33,0x03,0x40,0x46,0x01,0x02,0x03,0x04,0x11,0x00,0x07,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badFrameType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackLogOnMsg(&phyDataInd, &logOnMsg), ERR_OUT_OF_RANGE_E);
   /* bad msg type */
   uint8_t badMsgType[20] = { 0x11,0x01,0x20,0x20,0x33,0x03,0x40,0x40,0x01,0x02,0x03,0x04,0x11,0x00,0x07,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badMsgType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackLogOnMsg(&phyDataInd, &logOnMsg), ERR_OUT_OF_RANGE_E);
   /* bad Device Combination.  Only values 0-34 are assigned.  35-255 are unused */
   uint8_t badDevComb[20] = { 0x11,0x01,0x20,0x20,0x33,0x03,0x40,0x46,0x01,0x02,0x03,0x04,0x23,0x00,0x07,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badDevComb, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackLogOnMsg(&phyDataInd, &logOnMsg), ERR_OUT_OF_RANGE_E);
   /* bad status flags.  Only bits 0-7 should be used.  Bits 8-15 are reserved not used. */
   uint8_t badStatusFlags[20] = { 0x11,0x01,0x20,0x20,0x33,0x03,0x40,0x46,0x01,0x02,0x03,0x04,0x11,0x00,0x80,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badStatusFlags, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackLogOnMsg(&phyDataInd, &logOnMsg), ERR_OUT_OF_RANGE_E);

   /* The remaining message parameters use their full range, so we can't inject invalid values. */
}


/*************************************************************************************/
/**
* MC_PUP_PackStatusIndMsgValidDataTest
* Test packing of valid Status Indication message data
*
* no params
*
* @return - none

*/
void MC_PUP_PackStatusIndMsgValidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   StatusIndicationMessage_t statusMsg = { 0 };
   StatusIndicationMessage_t statusMsg2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   statusMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   statusMsg.Header.MACDestinationAddress = 0x101;
   statusMsg.Header.MACSourceAddress = 0x202;
   statusMsg.Header.HopCount = 3;
   statusMsg.Header.DestinationAddress = 0x303;
   statusMsg.Header.SourceAddress = 0x404;
   statusMsg.Header.MessageType = APP_MSG_TYPE_STATUS_INDICATION_E;
   statusMsg.NumberOfParents = 0x02;
   statusMsg.NumberOfChildren = 32;
   statusMsg.AverageSNRPrimaryParent = 0x05;
   statusMsg.AverageSNRSecondaryParent = 0x06;
   statusMsg.NumberOfTrackingNodes = 0x02;
   statusMsg.NumberOfChildrenOfPrimaryTrackingNode = 0x02;
   statusMsg.Rank = 1;
   statusMsg.Event = CO_MESH_EVENT_CHILD_NODE_ADDED_E;
   statusMsg.EventNodeId = 1;
   statusMsg.SystemId = 0xffffffff;


   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x11);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x33);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x03);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x43);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0xd0);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x0a);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0x34);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x10);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0x02);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0x0f);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0xf0);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackStatusIndicationMsg(&phyDataInd, &statusMsg2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(statusMsg2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(statusMsg2.Header.MACDestinationAddress, 0x101);
   CU_ASSERT_EQUAL(statusMsg2.Header.MACSourceAddress, 0x202);
   CU_ASSERT_EQUAL(statusMsg2.Header.HopCount, 0x03);
   CU_ASSERT_EQUAL(statusMsg2.Header.DestinationAddress, 0x303);
   CU_ASSERT_EQUAL(statusMsg2.Header.SourceAddress, 0x404);
   CU_ASSERT_EQUAL(statusMsg2.Header.MessageType, 0x07);
   CU_ASSERT_EQUAL(statusMsg2.NumberOfParents, 0x02);
   CU_ASSERT_EQUAL(statusMsg2.NumberOfChildren, 32);
    CU_ASSERT_EQUAL(statusMsg2.AverageSNRPrimaryParent, 0x05);
   CU_ASSERT_EQUAL(statusMsg2.AverageSNRSecondaryParent, 0x06);
   CU_ASSERT_EQUAL(statusMsg2.NumberOfTrackingNodes, 0x02);
   CU_ASSERT_EQUAL(statusMsg2.NumberOfChildrenOfPrimaryTrackingNode, 0x02);
   CU_ASSERT_EQUAL(statusMsg2.Rank, 0x01);
   CU_ASSERT_EQUAL(statusMsg2.Event, 0x01);
   CU_ASSERT_EQUAL(statusMsg2.EventNodeId, 0x01);
   CU_ASSERT_EQUAL(statusMsg2.SystemId, 0xffffffff);
}

/*************************************************************************************/
/**
* MC_PUP_PackStatusIndMsgInvalidDataTest
* Test packing of invalid Status Indication Message data
*
* no params
*
* @return - none

*/
void MC_PUP_PackStatusIndMsgInvalidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   StatusIndicationMessage_t statusMsg = { 0 };
   StatusIndicationMessage_t statusMsg2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   statusMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   statusMsg.Header.MACDestinationAddress = 0x101;
   statusMsg.Header.MACSourceAddress = 0x202;
   statusMsg.Header.HopCount = 4;
   statusMsg.Header.DestinationAddress = 0x303;
   statusMsg.Header.SourceAddress = 0x404;
   statusMsg.Header.MessageType = 7;
   statusMsg.NumberOfParents = 0x02;
   statusMsg.NumberOfChildren = 32;
   statusMsg.AverageSNRPrimaryParent = 0x05;
   statusMsg.AverageSNRSecondaryParent = 0x06;
   statusMsg.NumberOfTrackingNodes = 0x02;
   statusMsg.NumberOfChildrenOfPrimaryTrackingNode = 0x02;
   statusMsg.Rank = 0x01;
   statusMsg.Event = 0x01;
   statusMsg.EventNodeId = 0x01;
   statusMsg.SystemId = 0xffffffff;

   /* pack */
   status = MC_PUP_PackStatusIndicationMsg(NULL, &phyDataReq);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_PackStatusIndicationMsg(&statusMsg, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* unpack */
   status = MC_PUP_UnpackStatusIndicationMsg(NULL, &statusMsg2);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_UnpackStatusIndicationMsg(&phyDataInd, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   // check heartbeat value rejection
   statusMsg.Header.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   statusMsg.Header.MACDestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.Header.MACDestinationAddress = 0x101;
   statusMsg.Header.MACSourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.Header.MACSourceAddress = 0x202;
   statusMsg.Header.HopCount = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.Header.HopCount = 3;
   statusMsg.Header.DestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.Header.DestinationAddress = 0x303;
   statusMsg.Header.SourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.Header.SourceAddress = 0x404;
   statusMsg.Header.MessageType = 0;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.Header.MessageType = 7;
   statusMsg.NumberOfParents = 0x03;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.NumberOfParents = 0x00;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.NumberOfParents = 0x02;
   statusMsg.NumberOfChildren = 33;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.NumberOfChildren = 32;
   statusMsg.AverageSNRPrimaryParent = 0x40;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.AverageSNRPrimaryParent = 0x05;
   statusMsg.AverageSNRSecondaryParent = 0x40;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.AverageSNRSecondaryParent = 0x06;
   statusMsg.NumberOfTrackingNodes = 0x04;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.NumberOfTrackingNodes = 0x02;
   statusMsg.NumberOfChildrenOfPrimaryTrackingNode = 0x40;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.NumberOfChildrenOfPrimaryTrackingNode = 0x02;
   statusMsg.Rank = 0x40;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.Rank = 0x01;
   statusMsg.Event = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.Event = 0x01;
   statusMsg.EventNodeId = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackStatusIndicationMsg(&statusMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   statusMsg.EventNodeId = 0x01;

   /* check that the unpack function detects out of range values */
   /* bad frame type */
   uint8_t badFrameType[20] = { 0x01,0x01,0x20,0x20,0x33,0x03,0x40,0x43,0xd0,0x14,0xa8,0x20,0x40,0x40,0x04,0x7f,0xff,0xff,0xff,0xf8 };
   memcpy(&phyDataInd.Data, badFrameType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackStatusIndicationMsg(&phyDataInd, &statusMsg), ERR_OUT_OF_RANGE_E);
   /* bad msg type */
   uint8_t badMsgType[20] = { 0x11,0x01,0x20,0x20,0x33,0x03,0x40,0x40,0xd0,0x04,0xa8,0x20,0x40,0x40,0x04,0x7f,0xff,0xff,0xff,0xf8 };
   memcpy(&phyDataInd.Data, badMsgType, 20);
   phyDataInd.Size = 0;
   CU_ASSERT_EQUAL(MC_PUP_UnpackStatusIndicationMsg(&phyDataInd, &statusMsg), ERR_OUT_OF_RANGE_E);
   /* bad number of parents - high value 3*/
   uint8_t badNumParents2[20] = { 0x11,0x01,0x20,0x20,0x33,0x03,0x40,0x43,0xf0,0x14,0xa8,0x20,0x40,0x40,0x04,0x7f,0xff,0xff,0xff,0xf8 };
   memcpy(&phyDataInd.Data, badNumParents2, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackStatusIndicationMsg(&phyDataInd, &statusMsg), ERR_OUT_OF_RANGE_E);
   /* bad number of children - high value 33*/
   uint8_t badNumChildren[20] = { 0x11,0x01,0x20,0x20,0x33,0x03,0x40,0x43,0xd0,0x94,0xa8,0x20,0x40,0x40,0x04,0x7f,0xff,0xff,0xff,0xf8 };
   memcpy(&phyDataInd.Data, badNumChildren, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackStatusIndicationMsg(&phyDataInd, &statusMsg), ERR_OUT_OF_RANGE_E);

   /* The remaining message parameters use their full range, so we can't inject invalid values. */
}


/*************************************************************************************/
/**
* MC_PUP_PackRouteAddValidDataTest
* Test packing of valid Route Add data
*
* no params
*
* @return - none

*/
void MC_PUP_PackRouteAddValidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   RouteAddMessage_t routAddMsg = { 0 };
   RouteAddMessage_t routAddMsg2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   routAddMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   routAddMsg.Header.MACDestinationAddress = 0x101;
   routAddMsg.Header.MACSourceAddress = 0x202;
   routAddMsg.Header.HopCount = 3;
   routAddMsg.Header.DestinationAddress = 0x303;
   routAddMsg.Header.SourceAddress = 0x404;
   routAddMsg.Header.MessageType = APP_MSG_TYPE_ROUTE_ADD_E;
   routAddMsg.Rank = 5;
   routAddMsg.SystemId = 0xffffffff;


   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackRouteAddMsg(&routAddMsg, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x11);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x33);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x03);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x44);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x8a);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0x0f);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0xf0);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackRouteAddMsg(&phyDataInd, &routAddMsg2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(routAddMsg2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(routAddMsg2.Header.MACDestinationAddress, 0x101);
   CU_ASSERT_EQUAL(routAddMsg2.Header.MACSourceAddress, 0x202);
   CU_ASSERT_EQUAL(routAddMsg2.Header.HopCount, 0x03);
   CU_ASSERT_EQUAL(routAddMsg2.Header.DestinationAddress, 0x303);
   CU_ASSERT_EQUAL(routAddMsg2.Header.SourceAddress, 0x404);
   CU_ASSERT_EQUAL(routAddMsg2.Header.MessageType, 0x09);
   CU_ASSERT_EQUAL(routAddMsg2.Rank, 0x05);
   CU_ASSERT_EQUAL(routAddMsg2.SystemId, 0xffffffff);
}

/*************************************************************************************/
/**
* MC_PUP_PackRouteAddInvalidDataTest
* Test packing of invalid Route Add data
*
* no params
*
* @return - none

*/
void MC_PUP_PackRouteAddInvalidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   RouteAddMessage_t routAddMsg = { 0 };
   RouteAddMessage_t routAddMsg2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   routAddMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   routAddMsg.Header.MACDestinationAddress = 0x101;
   routAddMsg.Header.MACSourceAddress = 0x202;
   routAddMsg.Header.HopCount = 4;
   routAddMsg.Header.DestinationAddress = 0x303;
   routAddMsg.Header.SourceAddress = 0x404;
   routAddMsg.Header.MessageType = APP_MSG_TYPE_ROUTE_ADD_E;
   routAddMsg.Rank = 5;
   routAddMsg.SystemId = 0xffffffff;

   /* pack */
   status = MC_PUP_PackRouteAddMsg(NULL, &phyDataReq);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_PackRouteAddMsg(&routAddMsg, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* unpack */
   status = MC_PUP_UnpackRouteAddMsg(NULL, &routAddMsg2);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_UnpackRouteAddMsg(&phyDataInd, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   // check heartbeat value rejection
   routAddMsg.Header.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteAddMsg(&routAddMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   routAddMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   routAddMsg.Header.MACDestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteAddMsg(&routAddMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   routAddMsg.Header.MACDestinationAddress = 0x101;
   routAddMsg.Header.MACSourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteAddMsg(&routAddMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   routAddMsg.Header.MACSourceAddress = 0x202;
   routAddMsg.Header.HopCount = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteAddMsg(&routAddMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   routAddMsg.Header.HopCount = 4;
   routAddMsg.Header.DestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteAddMsg(&routAddMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   routAddMsg.Header.DestinationAddress = 0x303;
   routAddMsg.Header.SourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteAddMsg(&routAddMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   routAddMsg.Header.SourceAddress = 0x404;
   routAddMsg.Header.MessageType = 0;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteAddMsg(&routAddMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   routAddMsg.Header.MessageType = 9;
   routAddMsg.Rank = 64;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteAddMsg(&routAddMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);

   /* check that the unpack function detects out of range values */
   /* bad frame type */
   uint8_t badFrameType[20] = { 0x01,0x01,0x20,0x20,0x33,0x03,0x40,0x46,0x01,0x02,0x03,0x04,0x11,0x00,0x07,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badFrameType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackRouteAddMsg(&phyDataInd, &routAddMsg), ERR_OUT_OF_RANGE_E);
   /* bad msg type */
   uint8_t badMsgType[20] = { 0x11,0x01,0x20,0x20,0x33,0x03,0x40,0x40,0x01,0x02,0x03,0x04,0x11,0x00,0x07,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badMsgType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackRouteAddMsg(&phyDataInd, &routAddMsg), ERR_OUT_OF_RANGE_E);

   /* The remaining message parameters use their full range, so we can't inject invalid values. */
}

/*************************************************************************************/
/**
* MC_PUP_PackRouteDropValidDataTest
* Test packing of valid Route Drop data
*
* no params
*
* @return - none

*/
void MC_PUP_PackRouteDropValidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   RouteDropMessage_t routeDropMsg = { 0 };
   RouteDropMessage_t routeDropMsg2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   routeDropMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   routeDropMsg.Header.MACDestinationAddress = 0x101;
   routeDropMsg.Header.MACSourceAddress = 0x202;
   routeDropMsg.Header.HopCount = 3;
   routeDropMsg.Header.DestinationAddress = 0x303;
   routeDropMsg.Header.SourceAddress = 0x404;
   routeDropMsg.Header.MessageType = APP_MSG_TYPE_ROUTE_DROP_E;
   routeDropMsg.SystemId = 0xffffffff;


   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackRouteDropMsg(&routeDropMsg, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x11);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x33);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x03);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x45);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x80);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0x0f);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0xf0);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackRouteDropMsg(&phyDataInd, &routeDropMsg2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(routeDropMsg2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(routeDropMsg2.Header.MACDestinationAddress, 0x101);
   CU_ASSERT_EQUAL(routeDropMsg2.Header.MACSourceAddress, 0x202);
   CU_ASSERT_EQUAL(routeDropMsg2.Header.HopCount, 0x03);
   CU_ASSERT_EQUAL(routeDropMsg2.Header.DestinationAddress, 0x303);
   CU_ASSERT_EQUAL(routeDropMsg2.Header.SourceAddress, 0x404);
   CU_ASSERT_EQUAL(routeDropMsg2.Header.MessageType, 0x0b);
   CU_ASSERT_EQUAL(routeDropMsg2.SystemId, 0xffffffff);
}

/*************************************************************************************/
/**
* MC_PUP_PackRouteDropInvalidDataTest
* Test packing of invalid Route Drop data
*
* no params
*
* @return - none

*/
void MC_PUP_PackRouteDropInvalidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   RouteDropMessage_t routeDropMsg = { 0 };
   RouteDropMessage_t routeDropMsg2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   routeDropMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   routeDropMsg.Header.MACDestinationAddress = 0x101;
   routeDropMsg.Header.MACSourceAddress = 0x202;
   routeDropMsg.Header.HopCount = 4;
   routeDropMsg.Header.DestinationAddress = 0x303;
   routeDropMsg.Header.SourceAddress = 0x404;
   routeDropMsg.Header.MessageType = 10;
   routeDropMsg.SystemId = 0xffffffff;

   /* pack */
   status = MC_PUP_PackRouteDropMsg(NULL, &phyDataReq);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_PackRouteDropMsg(&routeDropMsg, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* unpack */
   status = MC_PUP_UnpackRouteDropMsg(NULL, &routeDropMsg2);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_UnpackRouteDropMsg(&phyDataInd, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   // check heartbeat value rejection
   routeDropMsg.Header.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteDropMsg(&routeDropMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   routeDropMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   routeDropMsg.Header.MACDestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteDropMsg(&routeDropMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   routeDropMsg.Header.MACDestinationAddress = 0x101;
   routeDropMsg.Header.MACSourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteDropMsg(&routeDropMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   routeDropMsg.Header.MACSourceAddress = 0x202;
   routeDropMsg.Header.HopCount = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteDropMsg(&routeDropMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   routeDropMsg.Header.HopCount = 4;
   routeDropMsg.Header.DestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteDropMsg(&routeDropMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   routeDropMsg.Header.DestinationAddress = 0x303;
   routeDropMsg.Header.SourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteDropMsg(&routeDropMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   routeDropMsg.Header.SourceAddress = 0x404;
   routeDropMsg.Header.MessageType = 0;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteDropMsg(&routeDropMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);

   /* check that the unpack function detects out of range values */
   /* bad frame type */
   uint8_t badFrameType[20] = { 0x01,0x01,0x20,0x20,0x33,0x03,0x40,0x4a,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badFrameType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackRouteDropMsg(&phyDataInd, &routeDropMsg), ERR_OUT_OF_RANGE_E);
   /* bad msg type */
   uint8_t badMsgType[20] = { 0x11,0x01,0x20,0x20,0x33,0x03,0x40,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badMsgType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackRouteDropMsg(&phyDataInd, &routeDropMsg), ERR_OUT_OF_RANGE_E);

   /* The remaining message parameters use their full range, so we can't inject invalid values. */

}

/*************************************************************************************/
/**
* MC_PUP_PackResponseMsgInvalidDataTest
* Test packing of invalid Response Message data
* The source code uses the Command packing code to pack Response messages, due to their
* identical content, so this is a valid test for the Command message also.
*
* no params
*
* @return - none

*/
void MC_PUP_PackResponseMsgInvalidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   ResponseMessage_t responseMsg = { 0 };
   ResponseMessage_t responseMsg2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   responseMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   responseMsg.Header.MACDestinationAddress = 0x101;
   responseMsg.Header.MACSourceAddress = 0x202;
   responseMsg.Header.HopCount = 4;
   responseMsg.Header.DestinationAddress = 0x303;
   responseMsg.Header.SourceAddress = 0x404;
   responseMsg.Header.MessageType = 5;
   responseMsg.ParameterType = 7;
   responseMsg.ReadWrite = 1;
   responseMsg.TransactionID = 1;
   responseMsg.P1 = 1;
   responseMsg.P2 = 2;
   responseMsg.Value = 0x1234;
   responseMsg.SystemId = 0xffffffff;

   /* pack */
   status = MC_PUP_PackResponseMessage(NULL, &phyDataReq);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_PackResponseMessage(&responseMsg, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* unpack */
   status = MC_PUP_UnpackResponseMessage(NULL, &responseMsg2);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_UnpackResponseMessage(&phyDataInd, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   // check heartbeat value rejection
   responseMsg.Header.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   CU_ASSERT_EQUAL(MC_PUP_PackResponseMessage(&responseMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   responseMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   responseMsg.Header.MACDestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackResponseMessage(&responseMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   responseMsg.Header.MACDestinationAddress = 0x101;
   responseMsg.Header.MACSourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackResponseMessage(&responseMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   responseMsg.Header.MACSourceAddress = 0x202;
   responseMsg.Header.HopCount = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackResponseMessage(&responseMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   responseMsg.Header.HopCount = 4;
   responseMsg.Header.DestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackResponseMessage(&responseMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   responseMsg.Header.DestinationAddress = 0x303;
   responseMsg.Header.SourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackResponseMessage(&responseMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   responseMsg.Header.SourceAddress = 0x404;
   responseMsg.Header.MessageType = 0;
   CU_ASSERT_EQUAL(MC_PUP_PackResponseMessage(&responseMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   responseMsg.Header.MessageType = 5;
   responseMsg.ParameterType = PARAM_TYPE_MAX_E;
   CU_ASSERT_EQUAL(MC_PUP_PackResponseMessage(&responseMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   responseMsg.ParameterType = 7;
   responseMsg.ReadWrite = 2;
   CU_ASSERT_EQUAL(MC_PUP_PackResponseMessage(&responseMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   responseMsg.ReadWrite = 1;
   responseMsg.TransactionID = 8;
   CU_ASSERT_EQUAL(MC_PUP_PackResponseMessage(&responseMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   responseMsg.TransactionID = 1;
   responseMsg.P1 = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackResponseMessage(&responseMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);
   responseMsg.P1 = 1;
   responseMsg.P2 = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackResponseMessage(&responseMsg, &phyDataReq), ERR_OUT_OF_RANGE_E);


   /* check that the unpack function detects out of range values */
   /* bad frame type */
   uint8_t badFrameType[20] = { 0x01,0x01,0x20,0x20,0x33,0x03,0x40,0x45,0x00,0x10,0x10,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badFrameType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackResponseMessage(&phyDataInd, &responseMsg), ERR_OUT_OF_RANGE_E);
   /* bad msg type */
   uint8_t badMsgType[20] = { 0x11,0x01,0x20,0x20,0x33,0x03,0x40,0x40,0x00,0x10,0x10,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badMsgType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackResponseMessage(&phyDataInd, &responseMsg), ERR_OUT_OF_RANGE_E);
   /* bad parameter type */
   uint8_t badParamType[20] = { 0x11,0x01,0x20,0x20,0x33,0x03,0x40,0x45,0x17,0x10,0x10,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badParamType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackResponseMessage(&phyDataInd, &responseMsg), ERR_OUT_OF_RANGE_E);

   /* The remaining message parameters use their full range, so we can't inject invalid values. */

}


/*************************************************************************************/
/**
* MC_PUP_PackResponseMsgAnalogueValueTest
* Test packing of valid Response message for Parameter Type 0, Analogue Value.
* The source code uses the Command packing code to pack Response messages, due to their
* identical content, so this is a valid test for the Command message also.
*
* no params
*
* @return - none

*/
void MC_PUP_PackResponseMsgAnalogueValueTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   ResponseMessage_t responseMsg = { 0 };
   ResponseMessage_t responseMsg2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   responseMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   responseMsg.Header.MACDestinationAddress = 0x101;
   responseMsg.Header.MACSourceAddress = 0x202;
   responseMsg.Header.HopCount = 3;
   responseMsg.Header.DestinationAddress = 0x303;
   responseMsg.Header.SourceAddress = 0x404;
   responseMsg.Header.MessageType = 5;//Response Msg
   responseMsg.ParameterType = PARAM_TYPE_ANALOGUE_VALUE_E;
   responseMsg.ReadWrite = 0;//read
   responseMsg.TransactionID = 1;
   responseMsg.P1 = 0x5a;
   responseMsg.P2 = 0;
   responseMsg.Value = 0xA5;
   responseMsg.SystemId = 0xffffffff;


   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackResponseMessage(&responseMsg, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x11);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x33);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x03);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x42);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x80);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x15);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0xa0);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0x0a);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0x5f);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0xf0);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackResponseMessage(&phyDataInd, &responseMsg2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(responseMsg2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(responseMsg2.Header.MACDestinationAddress, 0x101);
   CU_ASSERT_EQUAL(responseMsg2.Header.MACSourceAddress, 0x202);
   CU_ASSERT_EQUAL(responseMsg2.Header.HopCount, 0x03);
   CU_ASSERT_EQUAL(responseMsg2.Header.DestinationAddress, 0x303);
   CU_ASSERT_EQUAL(responseMsg2.Header.SourceAddress, 0x404);
   CU_ASSERT_EQUAL(responseMsg2.Header.MessageType, 0x05);
   CU_ASSERT_EQUAL(responseMsg2.ParameterType, PARAM_TYPE_ANALOGUE_VALUE_E);
   CU_ASSERT_EQUAL(responseMsg2.ReadWrite, 0);
   CU_ASSERT_EQUAL(responseMsg2.TransactionID, 1);
   CU_ASSERT_EQUAL(responseMsg2.P1, 0x5a);
   CU_ASSERT_EQUAL(responseMsg2.Value, 0xa5);
   CU_ASSERT_EQUAL(responseMsg2.SystemId, 0xffffffff);
}

/*************************************************************************************/
/**
* MC_PUP_PackResponseMsgNeighbourInfoTest
* Test packing of valid Response message for Parameter Type 1, Neighbour Information.
* The source code uses the Command packing code to pack Response messages, due to their
* identical content, so this is a valid test for the Command message also.
*
* no params
*
* @return - none

*/
void MC_PUP_PackResponseMsgNeighbourInfoTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   ResponseMessage_t responseMsg = { 0 };
   ResponseMessage_t responseMsg2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   responseMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   responseMsg.Header.MACDestinationAddress = 0x101;
   responseMsg.Header.MACSourceAddress = 0x202;
   responseMsg.Header.HopCount = 3;
   responseMsg.Header.DestinationAddress = 0x303;
   responseMsg.Header.SourceAddress = 0x404;
   responseMsg.Header.MessageType = 5;//Response Msg
   responseMsg.ParameterType = PARAM_TYPE_NEIGHBOUR_INFO_E;
   responseMsg.ReadWrite = 1;//write
   responseMsg.TransactionID = 0;
   responseMsg.P1 = 0x5a;
   responseMsg.P2 = 0x12;
   responseMsg.Value = 0xa5a5a;
   responseMsg.SystemId = 0xffffffff;


   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackResponseMessage(&responseMsg, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x11);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x33);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x03);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x42);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x81);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x85);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0xa1);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0xa5);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0xa5);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0xaf);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0xf0);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackResponseMessage(&phyDataInd, &responseMsg2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(responseMsg2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(responseMsg2.Header.MACDestinationAddress, 0x101);
   CU_ASSERT_EQUAL(responseMsg2.Header.MACSourceAddress, 0x202);
   CU_ASSERT_EQUAL(responseMsg2.Header.HopCount, 0x03);
   CU_ASSERT_EQUAL(responseMsg2.Header.DestinationAddress, 0x303);
   CU_ASSERT_EQUAL(responseMsg2.Header.SourceAddress, 0x404);
   CU_ASSERT_EQUAL(responseMsg2.Header.MessageType, 0x05);
   CU_ASSERT_EQUAL(responseMsg2.ParameterType, PARAM_TYPE_NEIGHBOUR_INFO_E);
   CU_ASSERT_EQUAL(responseMsg2.ReadWrite, 1);
   CU_ASSERT_EQUAL(responseMsg2.TransactionID, 0);
   CU_ASSERT_EQUAL(responseMsg2.P1, 0x5a);
   CU_ASSERT_EQUAL(responseMsg2.P2, 0x12);
   CU_ASSERT_EQUAL(responseMsg2.Value, 0xa5a5a);
   CU_ASSERT_EQUAL(responseMsg2.SystemId, 0xffffffff);
}

/*************************************************************************************/
/**
* MC_PUP_PackResponseMsgRebootTest
* Test packing of valid Response message for Parameter Type 22, Reboot.
* The source code uses the Command packing code to pack Response messages, due to their
* identical content, so this is a valid test for the Command message also.
*
* no params
*
* @return - none

*/
void MC_PUP_PackResponseMsgRebootTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   ResponseMessage_t responseMsg = { 0 };
   ResponseMessage_t responseMsg2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   responseMsg.Header.FrameType = FRAME_TYPE_DATA_E;
   responseMsg.Header.MACDestinationAddress = 0x101;
   responseMsg.Header.MACSourceAddress = 0x202;
   responseMsg.Header.HopCount = 3;
   responseMsg.Header.DestinationAddress = 0x303;
   responseMsg.Header.SourceAddress = 0x404;
   responseMsg.Header.MessageType = APP_MSG_TYPE_RESPONSE_E;
   responseMsg.ParameterType = PARAM_TYPE_REBOOT_E;
   responseMsg.ReadWrite = 1;//write
   responseMsg.TransactionID = 0;
   responseMsg.P1 = 0x5a;
   responseMsg.P2 = 0x12;
   responseMsg.Value = 0xa5a5a;
   responseMsg.SystemId = 0xffffffff;


   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackResponseMessage(&responseMsg, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x11);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x33);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x03);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x42);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x96);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x85);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0xa1);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0xa5);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0xa5);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0xaf);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0xf0);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackResponseMessage(&phyDataInd, &responseMsg2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(responseMsg2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(responseMsg2.Header.MACDestinationAddress, 0x101);
   CU_ASSERT_EQUAL(responseMsg2.Header.MACSourceAddress, 0x202);
   CU_ASSERT_EQUAL(responseMsg2.Header.HopCount, 0x03);
   CU_ASSERT_EQUAL(responseMsg2.Header.DestinationAddress, 0x303);
   CU_ASSERT_EQUAL(responseMsg2.Header.SourceAddress, 0x404);
   CU_ASSERT_EQUAL(responseMsg2.Header.MessageType, 0x05);
   CU_ASSERT_EQUAL(responseMsg2.ParameterType, PARAM_TYPE_REBOOT_E);
   CU_ASSERT_EQUAL(responseMsg2.ReadWrite, 1);
   CU_ASSERT_EQUAL(responseMsg2.TransactionID, 0);
   CU_ASSERT_EQUAL(responseMsg2.P1, 0x5a);
   CU_ASSERT_EQUAL(responseMsg2.P2, 0x12);
   CU_ASSERT_EQUAL(responseMsg2.Value, 0xa5a5a);
   CU_ASSERT_EQUAL(responseMsg2.SystemId, 0xffffffff);
}

/*************************************************************************************/
/**
* MC_PUP_PackCommandMsgAnalogueValueTest
* Test packing of valid Command message for Parameter Type 0, Analogue Value.
*
* no params
*
* @return - none

*/
void MC_PUP_PackCommandMsgAnalogueValueTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   CommandMessage_t pCommand = { 0 };
   CommandMessage_t pCommand2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   pCommand.Header.FrameType = FRAME_TYPE_DATA_E;
   pCommand.Header.MACDestinationAddress = 0x101;
   pCommand.Header.MACSourceAddress = 0x202;
   pCommand.Header.HopCount = 3;
   pCommand.Header.DestinationAddress = 0x303;
   pCommand.Header.SourceAddress = 0x404;
   pCommand.Header.MessageType = APP_MSG_TYPE_RESPONSE_E;
   pCommand.ParameterType = PARAM_TYPE_ANALOGUE_VALUE_E;
   pCommand.ReadWrite = 1;//write
   pCommand.TransactionID = 1;
   pCommand.P1 = 0x5a;
   pCommand.P2 = 0;
   pCommand.Value = 0xA5;
   pCommand.SystemId = 0xffffffff;


   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackCommandMessage(&pCommand, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x11);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x33);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x03);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x42);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x80);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x95);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0xa0);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0x0a);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0x5f);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0xf0);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackCommandMessage(&phyDataInd, &pCommand2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(pCommand2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(pCommand2.Header.MACDestinationAddress, 0x101);
   CU_ASSERT_EQUAL(pCommand2.Header.MACSourceAddress, 0x202);
   CU_ASSERT_EQUAL(pCommand2.Header.HopCount, 0x03);
   CU_ASSERT_EQUAL(pCommand2.Header.DestinationAddress, 0x303);
   CU_ASSERT_EQUAL(pCommand2.Header.SourceAddress, 0x404);
   CU_ASSERT_EQUAL(pCommand2.Header.MessageType, 0x05);
   CU_ASSERT_EQUAL(pCommand2.ParameterType, PARAM_TYPE_ANALOGUE_VALUE_E);
   CU_ASSERT_EQUAL(pCommand2.ReadWrite, 1);
   CU_ASSERT_EQUAL(pCommand2.TransactionID, 1);
   CU_ASSERT_EQUAL(pCommand2.P1, 0x5a);
   CU_ASSERT_EQUAL(pCommand2.Value, 0xa5);
   CU_ASSERT_EQUAL(pCommand2.SystemId, 0xffffffff);
}

/*************************************************************************************/
/**
* MC_PUP_PackOutputSignalValidDataTest
* Test packing of valid Output Signal data
*
* no params
*
* @return - none

*/
void MC_PUP_PackOutputSignalValidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   OutputSignal_t outputSignal = { 0 };
   OutputSignal_t outputSignal2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   outputSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   outputSignal.Header.MACDestinationAddress = 0x101;
   outputSignal.Header.MACSourceAddress = 0x202;
   outputSignal.Header.HopCount = 4;
   outputSignal.Header.DestinationAddress = 0x303;
   outputSignal.Header.SourceAddress = 0x404;
   outputSignal.Header.MessageType = APP_MSG_TYPE_OUTPUT_SIGNAL_E;
   outputSignal.Zone = 1;
   outputSignal.OutputProfile = 1;
   outputSignal.OutputsActivated = 0x5a;
   outputSignal.SystemId = 0xffffffff;


   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackOutputSignal(&outputSignal, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x11);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x43);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x03);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x41);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x80);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x08);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0x80);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x2d);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0x0f);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0xf0);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackOutputSignal(&phyDataInd, &outputSignal2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(outputSignal2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(outputSignal2.Header.MACDestinationAddress, 0x101);
   CU_ASSERT_EQUAL(outputSignal2.Header.MACSourceAddress, 0x202);
   CU_ASSERT_EQUAL(outputSignal2.Header.HopCount, 0x04);
   CU_ASSERT_EQUAL(outputSignal2.Header.DestinationAddress, 0x303);
   CU_ASSERT_EQUAL(outputSignal2.Header.SourceAddress, 0x404);
   CU_ASSERT_EQUAL(outputSignal2.Header.MessageType, 0x03);
   CU_ASSERT_EQUAL(outputSignal2.Zone, 0x01);
   CU_ASSERT_EQUAL(outputSignal2.OutputProfile, 0x01);
   CU_ASSERT_EQUAL(outputSignal2.OutputsActivated, 0x5a);
   CU_ASSERT_EQUAL(outputSignal2.SystemId, 0xffffffff);
}

/*************************************************************************************/
/**
* MC_PUP_PackOutputSignalInvalidDataTest
* Test packing of invalid Output Signal data
*
* no params
*
* @return - none

*/
void MC_PUP_PackOutputSignalInvalidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   OutputSignal_t outputSignal = { 0 };
   OutputSignal_t outputSignal2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   outputSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   outputSignal.Header.MACDestinationAddress = 0x101;
   outputSignal.Header.MACSourceAddress = 0x202;
   outputSignal.Header.HopCount = 4;
   outputSignal.Header.DestinationAddress = 0x303;
   outputSignal.Header.SourceAddress = 0x404;
   outputSignal.Header.MessageType = APP_MSG_TYPE_OUTPUT_SIGNAL_E;
   outputSignal.OutputProfile = 0;
   outputSignal.OutputsActivated = 0xa5;
   outputSignal.SystemId = 0xffffffff;

   /* pack */
   status = MC_PUP_PackOutputSignal(NULL, &phyDataReq);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_PackOutputSignal(&outputSignal, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* unpack */
   status = MC_PUP_UnpackOutputSignal(NULL, &outputSignal2);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_UnpackOutputSignal(&phyDataInd, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   // check heartbeat value rejection
   outputSignal.Header.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   CU_ASSERT_EQUAL(MC_PUP_PackOutputSignal(&outputSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   outputSignal.Header.FrameType = FRAME_TYPE_DATA_E;
   outputSignal.Header.MACDestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackOutputSignal(&outputSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   outputSignal.Header.MACDestinationAddress = 0x101;
   outputSignal.Header.MACSourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackOutputSignal(&outputSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   outputSignal.Header.MACSourceAddress = 0x202;
   outputSignal.Header.HopCount = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackOutputSignal(&outputSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   outputSignal.Header.HopCount = 4;
   outputSignal.Header.DestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackOutputSignal(&outputSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   outputSignal.Header.DestinationAddress = 0x303;
   outputSignal.Header.SourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackOutputSignal(&outputSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   outputSignal.Header.SourceAddress = 0x404;
   outputSignal.Header.MessageType = 2;
   CU_ASSERT_EQUAL(MC_PUP_PackOutputSignal(&outputSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   outputSignal.Header.MessageType = APP_MSG_TYPE_OUTPUT_SIGNAL_E;
   outputSignal.OutputProfile = CO_PROFILE_MAX_E;
   CU_ASSERT_EQUAL(MC_PUP_PackOutputSignal(&outputSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);
   outputSignal.OutputProfile = 0;
   outputSignal.OutputsActivated = 65536;
   CU_ASSERT_EQUAL(MC_PUP_PackOutputSignal(&outputSignal, &phyDataReq), ERR_OUT_OF_RANGE_E);

   /* check that the unpack function detects out of range values */
   /* bad frame type */
   uint8_t badFrameType[20] = { 0x01,0x01,0x20,0x20,0x33,0x03,0x40,0x43,0x0a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badFrameType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackOutputSignal(&phyDataInd, &outputSignal), ERR_OUT_OF_RANGE_E);
   /* bad msg type */
   uint8_t badMsgType[20] = { 0x11,0x01,0x20,0x20,0x33,0x03,0x40,0x40,0x0a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badMsgType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackOutputSignal(&phyDataInd, &outputSignal), ERR_OUT_OF_RANGE_E);
   /* bad parameter type */
   uint8_t badParamType[20] = { 0x11,0x01,0x20,0x20,0x33,0x03,0x40,0x43,0x5a,0x50,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badParamType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackOutputSignal(&phyDataInd, &outputSignal), ERR_OUT_OF_RANGE_E);

   /* The remaining message parameters use their full range, so we can't inject invalid values. */


}

/*************************************************************************************/
/**
* MC_PUP_PackSetAddressValidDataTest
* Test packing of valid SetAddress data
*
* no params
*
* @return - none

*/
void MC_PUP_PackSetAddressValidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   SetAddress_t SetAddress = { 0 };
   SetAddress_t SetAddress2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   SetAddress.FrameType = FRAME_TYPE_SET_ADDRESS_E;
   SetAddress.SerialNumber = 0x12345678;
   SetAddress.Address = 0x101;
   SetAddress.SystemId = 0xffffffff;

   status = MC_PUP_PackSetAddress(NULL, &phyDataReq);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_PackSetAddress(&SetAddress, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackSetAddress(&SetAddress, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x31);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x23);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x45);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x67);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x81);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Size, 10);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackSetAddress(&phyDataInd, &SetAddress2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(SetAddress2.FrameType, FRAME_TYPE_SET_ADDRESS_E);
   CU_ASSERT_EQUAL(SetAddress2.SerialNumber, 0x12345678);
   CU_ASSERT_EQUAL(SetAddress2.Address, 0x101);
   CU_ASSERT_EQUAL(SetAddress2.SystemId, 0xffffffff);

}

/*************************************************************************************/
/**
* MC_PUP_PackSetAddressInvalidDataTest
* Test packing of invalid SetAddress data
*
* no params
*
* @return - none

*/
void MC_PUP_PackSetAddressInvalidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   SetAddress_t SetAddress = { 0 };
   SetAddress_t SetAddress2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   SetAddress.FrameType = FRAME_TYPE_SET_ADDRESS_E;
   SetAddress.SerialNumber = 0x12345678;
   SetAddress.Address = 0x101;
   SetAddress.SystemId = 0xffffffff;


   /* pack */
   status = MC_PUP_PackSetAddress(NULL, &phyDataReq);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_PackSetAddress(&SetAddress, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* unpack */
   status = MC_PUP_UnpackSetAddress(NULL, &SetAddress2);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_UnpackSetAddress(&phyDataInd, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   // check ack value rejection
   SetAddress.FrameType = FRAME_TYPE_DATA_E;
   CU_ASSERT_EQUAL(MC_PUP_PackSetAddress(&SetAddress, &phyDataReq), ERR_OUT_OF_RANGE_E);
   SetAddress.FrameType = FRAME_TYPE_SET_ADDRESS_E;
   SetAddress.Address = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackSetAddress(&SetAddress, &phyDataReq), ERR_OUT_OF_RANGE_E);
}



/*************************************************************************************/
/**
* MC_PUP_PackSetStateValidDataTest
* Test packing of valid SetState data
*
* no params
*
* @return - none

*/
void MC_PUP_PackSetStateValidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   SetStateMessage_t setState1 = { 0 };
   SetStateMessage_t setState2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   setState1.Header.FrameType = FRAME_TYPE_DATA_E;
   setState1.Header.MACDestinationAddress = 0x101;
   setState1.Header.MACSourceAddress = 0x202;
   setState1.Header.HopCount = 4;
   setState1.Header.DestinationAddress = 0x303;
   setState1.Header.SourceAddress = 0x404;
   setState1.Header.MessageType = APP_MSG_TYPE_STATE_SIGNAL_E;
   setState1.State = 1;
   setState1.SystemId = 0xffffffff;


   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackSetState(&setState1, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x11);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x43);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x03);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x47);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x08);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0x0f);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0xf0);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackSetState(&phyDataInd, &setState2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(setState2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(setState2.Header.MACDestinationAddress, 0x101);
   CU_ASSERT_EQUAL(setState2.Header.MACSourceAddress, 0x202);
   CU_ASSERT_EQUAL(setState2.Header.HopCount, 0x04);
   CU_ASSERT_EQUAL(setState2.Header.DestinationAddress, 0x303);
   CU_ASSERT_EQUAL(setState2.Header.SourceAddress, 0x404);
   CU_ASSERT_EQUAL(setState2.Header.MessageType, APP_MSG_TYPE_STATE_SIGNAL_E);
   CU_ASSERT_EQUAL(setState2.State, 0x01);
   CU_ASSERT_EQUAL(setState2.SystemId, 0xffffffff);
}

/*************************************************************************************/
/**
* MC_PUP_PackSetStateInvalidDataTest
* Test packing of invalid SetState data
*
* no params
*
* @return - none

*/
void MC_PUP_PackSetStateInvalidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   SetStateMessage_t setState1 = { 0 };
   SetStateMessage_t setState2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   setState1.Header.FrameType = FRAME_TYPE_DATA_E;
   setState1.Header.MACDestinationAddress = 0x101;
   setState1.Header.MACSourceAddress = 0x202;
   setState1.Header.HopCount = 4;
   setState1.Header.DestinationAddress = 0x303;
   setState1.Header.SourceAddress = 0x404;
   setState1.Header.MessageType = APP_MSG_TYPE_STATE_SIGNAL_E;
   setState1.State = 1;
   setState1.SystemId = 0xffffffff;

   /* pack */
   status = MC_PUP_PackSetState(NULL, &phyDataReq);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_PackSetState(&setState1, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* unpack */
   status = MC_PUP_UnpackSetState(NULL, &setState2);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_UnpackSetState(&phyDataInd, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   // check heartbeat value rejection
   setState1.Header.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   CU_ASSERT_EQUAL(MC_PUP_PackSetState(&setState1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   setState1.Header.FrameType = FRAME_TYPE_DATA_E;
   setState1.Header.MACDestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackSetState(&setState1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   setState1.Header.MACDestinationAddress = 0x101;
   setState1.Header.MACSourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackSetState(&setState1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   setState1.Header.MACSourceAddress = 0x202;
   setState1.Header.HopCount = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackSetState(&setState1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   setState1.Header.HopCount = 4;
   setState1.Header.DestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackSetState(&setState1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   setState1.Header.DestinationAddress = 0x303;
   setState1.Header.SourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackSetState(&setState1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   setState1.Header.SourceAddress = 0x404;
   setState1.Header.MessageType = 2;
   CU_ASSERT_EQUAL(MC_PUP_PackSetState(&setState1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   setState1.Header.MessageType = APP_MSG_TYPE_OUTPUT_SIGNAL_E;
   setState1.State = 4;
   CU_ASSERT_EQUAL(MC_PUP_PackSetState(&setState1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   setState1.State = 0;

   /* check that the unpack function detects out of range values */
   /* bad frame type */
   uint8_t badFrameType[20] = { 0x01,0x01,0x20,0x20,0x43,0x03,0x40,0x4f,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badFrameType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackSetState(&phyDataInd, &setState1), ERR_OUT_OF_RANGE_E);
   /* bad msg type */
   uint8_t badMsgType[20] = { 0x11,0x01,0x20,0x20,0x43,0x03,0x40,0x4e,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badMsgType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackSetState(&phyDataInd, &setState1), ERR_OUT_OF_RANGE_E);
   /* bad state value */
   uint8_t badStateValue[20] = { 0x11,0x01,0x20,0x20,0x43,0x03,0x40,0x4f,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badStateValue, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackSetState(&phyDataInd, &setState1), ERR_OUT_OF_RANGE_E);

   /* The remaining message parameters use their full range, so we can't inject invalid values. */


}

/*************************************************************************************/
/**
* MC_PUP_PackRouteAddResponseValidDataTest
* Test packing/unpacking of RouteAddResponse message with good data
*
* no params
*
* @return - none

*/
void MC_PUP_PackRouteAddResponseValidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   RouteAddResponseMessage_t response1 = { 0 };
   RouteAddResponseMessage_t response2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   response1.Header.FrameType = FRAME_TYPE_DATA_E;
   response1.Header.MACDestinationAddress = 0x101;
   response1.Header.MACSourceAddress = 0x202;
   response1.Header.HopCount = 4;
   response1.Header.DestinationAddress = 0x303;
   response1.Header.SourceAddress = 0x404;
   response1.Header.MessageType = APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E;
   response1.Accepted = 1;
   response1.SystemId = 0xffffffff;

   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackRouteAddResponseMsg(&response1, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x11);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x43);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x03);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x45);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0x0f);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0xf0);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackRouteAddResponseMsg(&phyDataInd, &response2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(response2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(response2.Header.MACDestinationAddress, 0x101);
   CU_ASSERT_EQUAL(response2.Header.MACSourceAddress, 0x202);
   CU_ASSERT_EQUAL(response2.Header.HopCount, 0x04);
   CU_ASSERT_EQUAL(response2.Header.DestinationAddress, 0x303);
   CU_ASSERT_EQUAL(response2.Header.SourceAddress, 0x404);
   CU_ASSERT_EQUAL(response2.Header.MessageType, APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E);
   CU_ASSERT_EQUAL(response2.Accepted, 0x01);
   CU_ASSERT_EQUAL(response2.SystemId, 0xffffffff);

}

/*************************************************************************************/
/**
* MC_PUP_PackRouteAddResponseInvalidDataTest
* Test packing/unpacking of RouteAddResponse message with bad data
*
* no params
*
* @return - none

*/
void MC_PUP_PackRouteAddResponseInvalidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   RouteAddResponseMessage_t response1 = { 0 };
   RouteAddResponseMessage_t response2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   response1.Header.FrameType = FRAME_TYPE_DATA_E;
   response1.Header.MACDestinationAddress = 0x101;
   response1.Header.MACSourceAddress = 0x202;
   response1.Header.HopCount = 4;
   response1.Header.DestinationAddress = 0x303;
   response1.Header.SourceAddress = 0x404;
   response1.Header.MessageType = APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E;
   response1.Accepted = 1;
   response1.SystemId = 0xffffffff;


   /* pack - null pointer check */
   status = MC_PUP_PackRouteAddResponseMsg(NULL, &phyDataReq);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_PackRouteAddResponseMsg(&response1, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* unpack - null pointer check */
   status = MC_PUP_UnpackRouteAddResponseMsg(NULL, &response2);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_UnpackRouteAddResponseMsg(&phyDataInd, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* range checks on supplied data */
   response1.Header.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteAddResponseMsg(&response1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   response1.Header.FrameType = FRAME_TYPE_DATA_E;
   response1.Header.MACDestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteAddResponseMsg(&response1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   response1.Header.MACDestinationAddress = 0x101;
   response1.Header.MACSourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteAddResponseMsg(&response1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   response1.Header.MACSourceAddress = 0x202;
   response1.Header.HopCount = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteAddResponseMsg(&response1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   response1.Header.HopCount = 4;
   response1.Header.DestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteAddResponseMsg(&response1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   response1.Header.DestinationAddress = 0x303;
   response1.Header.SourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteAddResponseMsg(&response1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   response1.Header.SourceAddress = 0x404;
   response1.Header.MessageType = 2;
   CU_ASSERT_EQUAL(MC_PUP_PackRouteAddResponseMsg(&response1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   response1.Header.MessageType = APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E;


   /* check that the unpack function detects out of range values */
   /* bad frame type */
   uint8_t badFrameType[20] = { 0x01,0x01,0x20,0x20,0x43,0x03,0x40,0x45,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badFrameType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackRouteAddResponseMsg(&phyDataInd, &response1), ERR_OUT_OF_RANGE_E);
   /* bad msg type */
   uint8_t badMsgType[20] = { 0x11,0x01,0x20,0x20,0x43,0x03,0x40,0x4e,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badMsgType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackRouteAddResponseMsg(&phyDataInd, &response1), ERR_OUT_OF_RANGE_E);


   /* The remaining message parameters use their full range, so we can't inject invalid values. */

}

/*************************************************************************************/
/**
* MC_PUP_PackLoadBalanceValidDataTest
* Test packing/unpacking of Load Balance message with good data
*
* no params
*
* @return - none

*/
void MC_PUP_PackLoadBalanceValidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   LoadBalanceMessage_t msg1 = { 0 };
   LoadBalanceMessage_t msg2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   msg1.Header.FrameType = FRAME_TYPE_DATA_E;
   msg1.Header.MACDestinationAddress = 0x101;
   msg1.Header.MACSourceAddress = 0x202;
   msg1.Header.HopCount = 4;
   msg1.Header.DestinationAddress = 0x303;
   msg1.Header.SourceAddress = 0x404;
   msg1.Header.MessageType = APP_MSG_TYPE_LOAD_BALANCE_E;
   msg1.SystemId = 0xffffffff;

   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackLoadBalanceMsg(&msg1, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x11);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x43);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x03);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x47);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x80);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0x0f);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0xf0);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackLoadBalanceMsg(&phyDataInd, &msg2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(msg2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(msg2.Header.MACDestinationAddress, 0x101);
   CU_ASSERT_EQUAL(msg2.Header.MACSourceAddress, 0x202);
   CU_ASSERT_EQUAL(msg2.Header.HopCount, 0x04);
   CU_ASSERT_EQUAL(msg2.Header.DestinationAddress, 0x303);
   CU_ASSERT_EQUAL(msg2.Header.SourceAddress, 0x404);
   CU_ASSERT_EQUAL(msg2.Header.MessageType, APP_MSG_TYPE_LOAD_BALANCE_E);
   CU_ASSERT_EQUAL(msg2.SystemId, 0xffffffff);

}

/*************************************************************************************/
/**
* MC_PUP_PackLoadBalanceInvalidDataTest
* Test packing/unpacking of Load Balance message with bad data
*
* no params
*
* @return - none

*/
void MC_PUP_PackLoadBalanceInvalidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   LoadBalanceMessage_t msg1 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   msg1.Header.FrameType = FRAME_TYPE_DATA_E;
   msg1.Header.MACDestinationAddress = 0x101;
   msg1.Header.MACSourceAddress = 0x202;
   msg1.Header.HopCount = 4;
   msg1.Header.DestinationAddress = 0x303;
   msg1.Header.SourceAddress = 0x404;
   msg1.Header.MessageType = APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E;
   msg1.SystemId = 0xffffffff;


   /* pack - null pointer check */
   status = MC_PUP_PackLoadBalanceMsg(NULL, &phyDataReq);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_PackLoadBalanceMsg(&msg1, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* unpack - null pointer check */
   status = MC_PUP_UnpackLoadBalanceMsg(NULL, &msg1);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_UnpackLoadBalanceMsg(&phyDataInd, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* range checks on supplied data */
   msg1.Header.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   CU_ASSERT_EQUAL(MC_PUP_PackLoadBalanceMsg(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.FrameType = FRAME_TYPE_DATA_E;
   msg1.Header.MACDestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackLoadBalanceMsg(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.MACDestinationAddress = 0x101;
   msg1.Header.MACSourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackLoadBalanceMsg(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.MACSourceAddress = 0x202;
   msg1.Header.HopCount = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackLoadBalanceMsg(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.HopCount = 4;
   msg1.Header.DestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackLoadBalanceMsg(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.DestinationAddress = 0x303;
   msg1.Header.SourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackLoadBalanceMsg(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.SourceAddress = 0x404;
   msg1.Header.MessageType = 2;
   CU_ASSERT_EQUAL(MC_PUP_PackLoadBalanceMsg(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.MessageType = APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E;


   /* check that the unpack function detects out of range values */
   /* bad frame type */
   uint8_t badFrameType[20] = { 0x01,0x01,0x20,0x20,0x43,0x03,0x40,0x45,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badFrameType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackLoadBalanceMsg(&phyDataInd, &msg1), ERR_OUT_OF_RANGE_E);
   /* bad msg type */
   uint8_t badMsgType[20] = { 0x11,0x01,0x20,0x20,0x43,0x03,0x40,0x4e,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badMsgType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackLoadBalanceMsg(&phyDataInd, &msg1), ERR_OUT_OF_RANGE_E);


   /* The remaining message parameters use their full range, so we can't inject invalid values. */

}

/*************************************************************************************/
/**
* MC_PUP_PackLoadBalanceResponseValidDataTest
* Test packing/unpacking of Load Balance Response message with good data
*
* no params
*
* @return - none

*/
void MC_PUP_PackLoadBalanceResponseValidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   LoadBalanceResponseMessage_t msg1 = { 0 };
   LoadBalanceResponseMessage_t msg2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   msg1.Header.FrameType = FRAME_TYPE_DATA_E;
   msg1.Header.MACDestinationAddress = 0x101;
   msg1.Header.MACSourceAddress = 0x202;
   msg1.Header.HopCount = 4;
   msg1.Header.DestinationAddress = 0x303;
   msg1.Header.SourceAddress = 0x404;
   msg1.Header.MessageType = APP_MSG_TYPE_LOAD_BALANCE_RESPONSE_E;
   msg1.Accepted = 1;
   msg1.SystemId = 0xffffffff;

   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackLoadBalanceResponseMsg(&msg1, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x11);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x43);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x03);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x48);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0x0f);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0xf0);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackLoadBalanceResponseMsg(&phyDataInd, &msg2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(msg2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(msg2.Header.MACDestinationAddress, 0x101);
   CU_ASSERT_EQUAL(msg2.Header.MACSourceAddress, 0x202);
   CU_ASSERT_EQUAL(msg2.Header.HopCount, 0x04);
   CU_ASSERT_EQUAL(msg2.Header.DestinationAddress, 0x303);
   CU_ASSERT_EQUAL(msg2.Header.SourceAddress, 0x404);
   CU_ASSERT_EQUAL(msg2.Header.MessageType, APP_MSG_TYPE_LOAD_BALANCE_RESPONSE_E);
   CU_ASSERT_EQUAL(msg2.Accepted, 1);
   CU_ASSERT_EQUAL(msg2.SystemId, 0xffffffff);

}

/*************************************************************************************/
/**
* MC_PUP_PackLoadBalanceResponseInvalidDataTest
* Test packing/unpacking of Load Balance Response message with bad data
*
* no params
*
* @return - none

*/
void MC_PUP_PackLoadBalanceResponseInvalidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   LoadBalanceResponseMessage_t msg1 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   msg1.Header.FrameType = FRAME_TYPE_DATA_E;
   msg1.Header.MACDestinationAddress = 0x101;
   msg1.Header.MACSourceAddress = 0x202;
   msg1.Header.HopCount = 4;
   msg1.Header.DestinationAddress = 0x303;
   msg1.Header.SourceAddress = 0x404;
   msg1.Header.MessageType = APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E;
   msg1.Accepted = 1;
   msg1.SystemId = 0xffffffff;


   /* pack - null pointer check */
   status = MC_PUP_PackLoadBalanceResponseMsg(NULL, &phyDataReq);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_PackLoadBalanceResponseMsg(&msg1, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* unpack - null pointer check */
   status = MC_PUP_UnpackLoadBalanceResponseMsg(NULL, &msg1);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_UnpackLoadBalanceResponseMsg(&phyDataInd, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* range checks on supplied data */
   msg1.Header.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   CU_ASSERT_EQUAL(MC_PUP_PackLoadBalanceResponseMsg(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.FrameType = FRAME_TYPE_DATA_E;
   msg1.Header.MACDestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackLoadBalanceResponseMsg(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.MACDestinationAddress = 0x101;
   msg1.Header.MACSourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackLoadBalanceResponseMsg(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.MACSourceAddress = 0x202;
   msg1.Header.HopCount = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackLoadBalanceResponseMsg(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.HopCount = 4;
   msg1.Header.DestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackLoadBalanceResponseMsg(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.DestinationAddress = 0x303;
   msg1.Header.SourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackLoadBalanceResponseMsg(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.SourceAddress = 0x404;
   msg1.Header.MessageType = 2;
   CU_ASSERT_EQUAL(MC_PUP_PackLoadBalanceResponseMsg(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.MessageType = APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E;


   /* check that the unpack function detects out of range values */
   /* bad frame type */
   uint8_t badFrameType[20] = { 0x01,0x01,0x20,0x20,0x43,0x03,0x40,0x49,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badFrameType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackLoadBalanceResponseMsg(&phyDataInd, &msg1), ERR_OUT_OF_RANGE_E);
   /* bad msg type */
   uint8_t badMsgType[20] = { 0x11,0x01,0x20,0x20,0x43,0x03,0x40,0x4e,0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badMsgType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackLoadBalanceResponseMsg(&phyDataInd, &msg1), ERR_OUT_OF_RANGE_E);


   /* The remaining message parameters use their full range, so we can't inject invalid values. */

}

/*************************************************************************************/
/**
* MC_PUP_PackRBUDisableValidDataTest
* Test packing/unpacking of RBU Disable message with good data
*
* no params
*
* @return - none

*/
void MC_PUP_PackRBUDisableValidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   RBUDisableMessage_t msg1 = { 0 };
   RBUDisableMessage_t msg2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   msg1.Header.FrameType = FRAME_TYPE_DATA_E;
   msg1.Header.MACDestinationAddress = 0xFFF;
   msg1.Header.MACSourceAddress = 0x00;
   msg1.Header.HopCount = 0;
   msg1.Header.DestinationAddress = 0x01;
   msg1.Header.SourceAddress = 0x00;
   msg1.Header.MessageType = APP_MSG_TYPE_RBU_DISABLE_E;
   msg1.UnitAddress = 1;
   msg1.SystemId = 5870773;

   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackRBUDisableMessage(&msg1, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);

   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x1f);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x09);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x80);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0x05);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0x99);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0x4b);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0x50);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackRBUDisableMessage(&phyDataInd, &msg2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(msg2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(msg2.Header.MACDestinationAddress, msg1.Header.MACDestinationAddress);
   CU_ASSERT_EQUAL(msg2.Header.MACSourceAddress, msg1.Header.MACSourceAddress);
   CU_ASSERT_EQUAL(msg2.Header.HopCount, msg1.Header.HopCount);
   CU_ASSERT_EQUAL(msg2.Header.DestinationAddress, msg1.Header.DestinationAddress);
   CU_ASSERT_EQUAL(msg2.Header.SourceAddress, msg1.Header.SourceAddress);
   CU_ASSERT_EQUAL(msg2.Header.MessageType, APP_MSG_TYPE_RBU_DISABLE_E);
   CU_ASSERT_EQUAL(msg2.UnitAddress, msg1.UnitAddress);
   CU_ASSERT_EQUAL(msg2.SystemId, msg1.SystemId);

}

/*************************************************************************************/
/**
* MC_PUP_PackRBUDisableInvalidDataTest
* Test packing/unpacking of RBU Disable message with bad data
*
* no params
*
* @return - none

*/
void MC_PUP_PackRBUDisableInvalidDataTest(void)
{
   ErrorCode_t status = SUCCESS_E;
   RBUDisableMessage_t msg1 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   msg1.Header.FrameType = FRAME_TYPE_DATA_E;
   msg1.Header.MACDestinationAddress = 0x01;
   msg1.Header.MACSourceAddress = 0x00;
   msg1.Header.HopCount = 0;
   msg1.Header.DestinationAddress = 0x01;
   msg1.Header.SourceAddress = 0x00;
   msg1.Header.MessageType = APP_MSG_TYPE_RBU_DISABLE_E;
   msg1.UnitAddress = 1;
   msg1.SystemId = 5870773;


   /* pack - null pointer check */
   status = MC_PUP_PackRBUDisableMessage(NULL, &phyDataReq);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_PackRBUDisableMessage(&msg1, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* unpack - null pointer check */
   status = MC_PUP_UnpackRBUDisableMessage(NULL, &msg1);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);
   status = MC_PUP_UnpackRBUDisableMessage(&phyDataInd, NULL);
   CU_ASSERT_EQUAL(status, ERR_INVALID_POINTER_E);

   /* range checks on supplied data */
   msg1.Header.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
   CU_ASSERT_EQUAL(MC_PUP_PackRBUDisableMessage(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.FrameType = FRAME_TYPE_DATA_E;
   msg1.Header.MACDestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackRBUDisableMessage(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.MACDestinationAddress = 0x101;
   msg1.Header.MACSourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackRBUDisableMessage(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.MACSourceAddress = 0x202;
   msg1.Header.HopCount = 256;
   CU_ASSERT_EQUAL(MC_PUP_PackRBUDisableMessage(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.HopCount = 4;
   msg1.Header.DestinationAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackRBUDisableMessage(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.DestinationAddress = 0x303;
   msg1.Header.SourceAddress = 4096;
   CU_ASSERT_EQUAL(MC_PUP_PackRBUDisableMessage(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.SourceAddress = 0x404;
   msg1.Header.MessageType = 2;
   CU_ASSERT_EQUAL(MC_PUP_PackRBUDisableMessage(&msg1, &phyDataReq), ERR_OUT_OF_RANGE_E);
   msg1.Header.MessageType = APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E;


   /* check that the unpack function detects out of range values */
   /* bad frame type */
   uint8_t badFrameType[20] = { 0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x09,0x80,0x40,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badFrameType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackRBUDisableMessage(&phyDataInd, &msg1), ERR_OUT_OF_RANGE_E);
   /* bad msg type */
   uint8_t badMsgType[20] = { 0x10,0x01,0x00,0x00,0x00,0x01,0x00,0x08,0x80,0x40,0x00,0x00,0x00,0x00,0x00,0x0f,0xff,0xff,0xff,0xf0 };
   memcpy(&phyDataInd.Data, badMsgType, 20);
   phyDataInd.Size = 20;
   CU_ASSERT_EQUAL(MC_PUP_UnpackRBUDisableMessage(&phyDataInd, &msg1), ERR_OUT_OF_RANGE_E);


   /* The remaining message parameters use their full range, so we can't inject invalid values. */

}

/*************************************************************************************/
/**
* MC_PUP_PackRBUDisableInvalidDataTest
* Test packing/unpacking of RBU Disable message with bad data
*
* no params
*
* @return - none

*/
void MC_PUP_PackOutputStateMessageTest()
{
   ErrorCode_t status = SUCCESS_E;
   OutputStateMessage_t outputState = { 0 };
   OutputStateMessage_t outputState2 = { 0 };
   CO_MessagePayloadPhyDataReq_t phyDataReq = { 0 };
   CO_MessagePayloadPhyDataInd_t phyDataInd = { 0 };
   uint32_t size = 0;

   outputState.Header.FrameType = FRAME_TYPE_DATA_E;
   outputState.Header.MACDestinationAddress = 1;
   outputState.Header.MACSourceAddress = 2;
   outputState.Header.HopCount = 4;
   outputState.Header.DestinationAddress = 1;
   outputState.Header.SourceAddress = 2;
   outputState.Header.MessageType = APP_MSG_TYPE_OUTPUT_STATE;
   outputState.Zone = 1;
   outputState.OutputProfile = 1;
   outputState.OutputActivated = 1;
   outputState.SystemId = 0xffffffff;


   memset(&phyDataReq, 0, sizeof(phyDataReq));
   status = MC_PUP_PackOutputStateMessage(&outputState, &phyDataReq);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(phyDataReq.Data[0], 0x10);
   CU_ASSERT_EQUAL(phyDataReq.Data[1], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[2], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[3], 0x20);
   CU_ASSERT_EQUAL(phyDataReq.Data[4], 0x40);
   CU_ASSERT_EQUAL(phyDataReq.Data[5], 0x01);
   CU_ASSERT_EQUAL(phyDataReq.Data[6], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[7], 0x2b);
   CU_ASSERT_EQUAL(phyDataReq.Data[8], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[9], 0x08);
   CU_ASSERT_EQUAL(phyDataReq.Data[10], 0x80);
   CU_ASSERT_EQUAL(phyDataReq.Data[11], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[12], 0x80);
   CU_ASSERT_EQUAL(phyDataReq.Data[13], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[14], 0x00);
   CU_ASSERT_EQUAL(phyDataReq.Data[15], 0x0f);
   CU_ASSERT_EQUAL(phyDataReq.Data[16], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[17], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[18], 0xff);
   CU_ASSERT_EQUAL(phyDataReq.Data[19], 0xf0);
   CU_ASSERT_EQUAL(phyDataReq.Size, 20);

   /* map across to phy data ind message */
   size = phyDataReq.Size;
   memcpy(&phyDataInd.Data, &phyDataReq.Data, size);
   phyDataInd.Size = size;

   /* unpack phydata ind */
   status = MC_PUP_UnpackOutputStateMessage(&phyDataInd, &outputState2);
   CU_ASSERT_EQUAL(status, SUCCESS_E);
   CU_ASSERT_EQUAL(outputState2.Header.FrameType, FRAME_TYPE_DATA_E);
   CU_ASSERT_EQUAL(outputState2.Header.MACDestinationAddress, 1);
   CU_ASSERT_EQUAL(outputState2.Header.MACSourceAddress, 2);
   CU_ASSERT_EQUAL(outputState2.Header.HopCount, 0x04);
   CU_ASSERT_EQUAL(outputState2.Header.DestinationAddress, 1);
   CU_ASSERT_EQUAL(outputState2.Header.SourceAddress, 2);
   CU_ASSERT_EQUAL(outputState2.Header.MessageType, APP_MSG_TYPE_OUTPUT_STATE);
   CU_ASSERT_EQUAL(outputState2.Zone, 1);
   CU_ASSERT_EQUAL(outputState2.OutputProfile, 1);
   CU_ASSERT_EQUAL(outputState2.OutputActivated, 1);
   CU_ASSERT_EQUAL(outputState2.SystemId, 0xffffffff);
}