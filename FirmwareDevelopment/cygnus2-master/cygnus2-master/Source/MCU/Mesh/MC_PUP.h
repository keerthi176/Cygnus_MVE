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
***************************************************************************************
*  File        : MC_PUP.h
*
*  Description : Mesh Packing and Unpacking module header
*
*************************************************************************************/

#ifndef MC_PUP_H
#define MC_PUP_H


/* System Include Files
*************************************************************************************/
#include <stdbool.h>



/* User Include Files
*************************************************************************************/
#include "CO_Message.h"
#include "CO_ErrorCode.h"



/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
uint32_t MC_PUP_ParameterGet(const uint8_t * const pBuffer, const uint32_t start, 
                              const uint32_t size);

ErrorCode_t MC_PUP_GetMessageType(CO_MessagePayloadPhyDataInd_t *pPhyDataInd, 
                                  FrameType_t *pFrameType, 
                                  bool *pAppMsgPresent, 
                                  ApplicationLayerMessageType_t *pAppMsgType);

ErrorCode_t MC_PUP_PackHeartbeat(FrameHeartbeat_t *pHeartbeat, 
                                    CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackHeartbeat(CO_MessagePayloadPhyDataInd_t *pPhyDataInd, 
                                    FrameHeartbeat_t *pHeartbeat);
ErrorCode_t MC_PUP_PackAlarmSignal(AlarmSignal_t *pAlarmSignal, 
                                    CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackAlarmSignal(CO_MessagePayloadPhyDataInd_t *pPhyDataReq, 
                                       AlarmSignal_t *pAlarmSignal);
ErrorCode_t MC_PUP_PackAck(FrameAcknowledge_t *pAck, CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackAck(CO_MessagePayloadPhyDataInd_t *pPhyDataReq, FrameAcknowledge_t *pAck);
ErrorCode_t MC_PUP_PackFaultSignal( FaultSignal_t *pFaultSignal,
                                    CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackFaultSignal(CO_MessagePayloadPhyDataInd_t *pPhyDataInd, 
                                       FaultSignal_t *pFaultSignal);
ErrorCode_t MC_PUP_PackLogOnMsg(LogOnMessage_t *pLogOnMsg,
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackLogOnMsg(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       LogOnMessage_t *pLogOnMsg);
ErrorCode_t MC_PUP_PackStatusIndicationMsg(StatusIndicationMessage_t *pStatusInd,
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackStatusIndicationMsg(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       StatusIndicationMessage_t *pStatusInd);
ErrorCode_t MC_PUP_PackRouteAddMsg(RouteAddMessage_t *pRouteAdd,
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackRouteAddMsg(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       RouteAddMessage_t *pRouteAdd);
ErrorCode_t MC_PUP_PackRouteAddResponseMsg(RouteAddResponseMessage_t *pRouteAddResponse,
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackRouteAddResponseMsg(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       RouteAddResponseMessage_t *pRouteAddResponse);
ErrorCode_t MC_PUP_PackRouteDropMsg(RouteDropMessage_t *pRouteDrop,
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackRouteDropMsg(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       RouteDropMessage_t *pRouteDrop);
ErrorCode_t MC_PUP_PackCommandMessage(CommandMessage_t *pCommand,
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackCommandMessage(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       CommandMessage_t *pCommand);
ErrorCode_t MC_PUP_PackResponseMessage(ResponseMessage_t *pResponse,
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackResponseMessage(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       ResponseMessage_t *pResponse);

ErrorCode_t MC_PUP_PackOutputSignal(OutputSignal_t *pOutputSignal,
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackOutputSignal(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       OutputSignal_t *pOutputSignal);
ErrorCode_t MC_PUP_PackAlarmOutputState(AlarmOutputState_t *pOutputSignal,
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackAlarmOutputState(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       AlarmOutputState_t *pOutputSignal);
ErrorCode_t MC_PUP_PackTestSignal(FrameTestModeMessage_t *pTestSignal, 
                                    CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackTestSignal(CO_MessagePayloadPhyDataInd_t *pPhyDataReq, 
                                       FrameTestModeMessage_t *pTestSignal);
ErrorCode_t MC_PUP_PackSetState(SetStateMessage_t *pStateSignal, 
                                    CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackSetState(CO_MessagePayloadPhyDataInd_t *pPhyDataReq, 
                                       SetStateMessage_t *pStateSignal);
ErrorCode_t MC_PUP_PackLoadBalanceMsg(LoadBalanceMessage_t *pLoadBalance,
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackLoadBalanceMsg(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       LoadBalanceMessage_t *pLoadBalance);
ErrorCode_t MC_PUP_PackLoadBalanceResponseMsg(LoadBalanceResponseMessage_t *pLoadBalanceResponse,
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackLoadBalanceResponseMsg(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       LoadBalanceResponseMessage_t *pLoadBalanceResponse);
ErrorCode_t MC_PUP_PackRBUDisableMessage(RBUDisableMessage_t *pRBUDisableMsg,
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackRBUDisableMessage(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       RBUDisableMessage_t *pRBUDisableMsg);
ErrorCode_t MC_PUP_PackBatteryStatusMessage(BatteryStatusMessage_t *pBatteryStatusMessage,
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackBatteryStatusMessage(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       BatteryStatusMessage_t *pBatteryStatusMessage);
ErrorCode_t MC_PUP_PackPingRequestMessage(PingRequest_t* pingRequest, 
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackPingRequestMessage(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       PingRequest_t* pingRequest);
ErrorCode_t MC_PUP_PackZoneEnableMessage(ZoneEnableMessage_t *pZoneEnableMessage,
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackZoneEnableMessage(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       ZoneEnableMessage_t *pZoneEnableMessage);
ErrorCode_t MC_PUP_PackPpuMessage(PPU_Message_t *pPpuCommand,
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackPpuMessage(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       PPU_Message_t *pPpuCommand);
ErrorCode_t MC_PUP_PackGlobalDelaysMessage(CO_GlobalDelaysMessage_t *pGlobalDelays,
                                       CO_MessagePayloadPhyDataReq_t *pPhyDataReq);
ErrorCode_t MC_PUP_UnpackGlobalDelaysMessage(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       CO_GlobalDelaysMessage_t *pGlobalDelays);
/* Public Constants
*************************************************************************************/
// generic fields
#define NUM_OF_ZONE_BITS   8        // The number of zone bits in the various message fields.
#define NUM_OF_RANK_BITS   6        // The number of rank bits in the various message fields.

// parameter offsets
#define PUP_FRAME_TYPE_START  0

// parameter widths (in bits)
#define PUP_FRAME_TYPE_SIZE  4
                                    
// Heartbeat Frame
// parameter offsets
#define HEARTBEAT_FRAME_TYPE_START  0
#define HEARTBEAT_SLOT_INDEX_START  4
#define HEARTBEAT_SHORT_FRAME_INDEX_START  9
#define HEARTBEAT_LONG_FRAME_INDEX_START  17
#define HEARTBEAT_TPI_START  21
#define HEARTBEAT_ROOT_ADDRESS_START  25
#define HEARTBEAT_RANK_START  37
#define HEARTBEAT_NUMBER_OF_CHILDREN_START  43
#define HEARTBEAT_SYSTEM_ID_START  51

// parameter widths (in bits)
#define HEARTBEAT_FRAME_TYPE_SIZE  4
#define HEARTBEAT_SLOT_INDEX_SIZE  3
#define HEARTBEAT_SHORT_FRAME_INDEX_SIZE  8
#define HEARTBEAT_LONG_FRAME_INDEX_SIZE  6
#define HEARTBEAT_STATE_SIZE  4
#define HEARTBEAT_RANK_SIZE  NUM_OF_RANK_BITS
#define HEARTBEAT_NUMBER_OF_CHILDREN_SIZE  4
#define HEARTBEAT_SYSTEM_ID_SIZE  32
#define HEARTBEAT_ACTIVE_FRAME_LENGTH_SIZE 1

//Padding bits for 3DES encryption of MAC payload (needs to be 8-byte aligned)
//For heartbeat this is root_addr + rank + num_of_child + padding = 64 bits

                                    
// Data Frame
// parameter offsets
#define DATA_FRAME_TYPE_START  0
#define DATA_MAC_DESTINATION_ADDRESS_START 4
#define DATA_MAC_SOURCE_ADDRESS_START 16
#define DATA_HOP_COUNT_START 28
#define DATA_DESTINATION_ADDRESS_START 36
#define DATA_SOURCE_ADDRESS_START 48
#define DATA_MESSAGE_TYPE_START  60

// parameter widths (in bits)
#define DATA_FRAME_TYPE_SIZE   4
#define DATA_MAC_DESTINATION_ADDRESS_SIZE 12
#define DATA_MAC_SOURCE_ADDRESS_SIZE 12
#define DATA_HOP_COUNT_SIZE 8
#define DATA_DESTINATION_ADDRESS_SIZE 12
#define DATA_SOURCE_ADDRESS_SIZE 12
#define DATA_MESSAGE_TYPE_SIZE   5
#define DATA_SYSTEM_ID_SIZE 32

// Alarm/FireSignal Uplink payload
#define ALARM_SIGNAL_RU_CHANNEL_INDEX_SIZE 7
#define ALARM_SIGNAL_SENSOR_VALUE_SIZE 8
#define ALARM_SIGNAL_ZONE_SIZE 16
#define ALARM_SIGNAL_ACTIVE_SIZE 1
//Padding bits for 3DES encryption of Alarm/FireSignal payload (needs to be 8-byte aligned)
//For Alarm/FireSignal this is msg_type + ru_channel_index + sensor_value + padding = 64 bits
#define ALARM_SIGNAL_PAYLOAD_PADDING_SIZE 27

// FaultSignal Uplink payload
#define FAULT_SIGNAL_RU_CHANNEL_INDEX_SIZE 12
#define FAULT_SIGNAL_FAULT_TYPE_SIZE 8
#define FAULT_SIGNAL_FAULT_VALUE_SIZE 1
#define FAULT_SIGNAL_OVERALL_FAULT_SIZE 2
//Padding bits for 3DES encryption of Fault Signal payload (needs to be 8-byte aligned)
#define FAULT_SIGNAL_PAYLOAD_PADDING_SIZE 36

// LogOn Message Uplink payload
#define LOGON_MSG_SERIAL_NUMBER_SIZE 32
#define LOGON_MSG_DEVICE_COMBINATION_SIZE 8
#define LOGON_MSG_ZONE_NUMBER_SIZE NUM_OF_ZONE_BITS
//Padding bits for 3DES encryption of LogOn Msg payload (needs to be 8-byte aligned)
#define LOGON_MSG_PADDING_SIZE 11

// Status Indication Message Uplink payload
#define STATUS_IND_ADDRESS_SIZE 9
#define STATUS_IND_NUM_CHILDREN_SIZE 6
#define STATUS_IND_AVERAGE_RSSI_SIZE 9          
#define MIN_REPORTABLE_RSSI -256                 /* lowest value that fits into the 9 bit format of the status indication message. */
#define MIN_REPORTABLE_SNR -32                 /* lowest value that fits into the 6 bit format of the status indication message. */
#define MIN_REPORTABLE_UNSIGNED_SNR 0xFFFFFFE0 /* unsigned format for MIN_REPORTABLE_SNR. */
#define MIN_REPORTABLE_UNSIGNED_RSSI 0xFFFFFF00 /* unsigned format for MIN_REPORTABLE_RSSI. */
#define STATUS_IND_AVERAGE_SNR_MASK 0x3F       /* To mask-off the leading 1's so that the Packing asserts aren't triggered */
#define STATUS_IND_AVERAGE_RSSI_MASK 0x1FF      /* To mask-off the leading 1's so that the Packing asserts aren't triggered */
#define STATUS_IND_EVENT_SIZE 6
#define STATUS_IND_EVENT_DATA_SIZE 9
#define STATUS_IND_OVERALL_FAULT_SIZE 2
#define STATUS_IND_RANK_SIZE NUM_OF_RANK_BITS

// Route Add Message Uplink payload
#define ROUTE_ADD_MSG_RANK_SIZE NUM_OF_RANK_BITS
#define ROUTE_ADD_MSG_ISPRIMARY_SIZE 1
//Padding bits for 3DES encryption of Route Add Msg payload (needs to be 8-byte aligned)
#define ROUTE_ADD_MSG_PADDING_SIZE 44

// Route Add Response Message
#define ROUTE_ADD_RESPONSE_MSG_ACCEPTED_SIZE 1
#define ROUTE_ADD_RESPONSE_MSG_DAY_NIGHT_SIZE 1
#define ROUTE_ADD_RESPONSE_MSG_GLOBAL_DELAY_SIZE 1
#define ROUTE_ADD_RESPONSE_MSG_ZONE_ENABLED_SIZE 1
#define ROUTE_ADD_RESPONSE_MSG_ZONE_LOWER_UPPER_SIZE 1
#define ROUTE_ADD_RESPONSE_MSG_ZONE_MAP_WORD_SIZE 32
#define ROUTE_ADD_RESPONSE_MSG_ZONE_MAP_HALFWORD_SIZE 16
#define ROUTE_ADD_RESPONSE_MSG_FAULTS_ENABLED_SIZE 1
#define ROUTE_ADD_RESPONSE_MSG_GLOBAL_DELAY_OVERRIDE_SIZE 1
#define ROUTE_ADD_RESPONSE_MSG_PADDING_SIZE 4

// Load Balance Message
#define LOAD_BALANCE_MSG_PADDING_SIZE 59

// Load Balance Response Message
#define LOAD_BALANCE_RESPONSE_MSG_ACCEPTED_SIZE 1
#define LOAD_BALANCE_RESPONSE_MSG_PADDING_SIZE 58

//Route Drop
#define ROUTE_DROP_MSG_REASON_SIZE 8
//Padding bits for 3DES encryption of Route Drop Msg payload (needs to be 8-byte aligned)
#define ROUTE_DROP_MSG_PADDING_SIZE 51

// Response Message uplink payload
#define RESP_MESSAGE_PARAMETER_TYPE_SIZE 7
#define RESP_MESSAGE_PARAMETER_P_SIZE 8
#define RESP_MESSAGE_VALUE_TYPE_SIZE 32
#define RESP_MESSAGE_READWRITE_SIZE 1
#define RESP_MESSAGE_TRANSACTIONID_SIZE 3

// OutputSignal downlink payload
#define OUTPUT_SIGNAL_ZONE_SIZE NUM_OF_ZONE_BITS
#define OUTPUT_SIGNAL_OUTPUT_CHANNEL_SIZE 8
#define OUTPUT_SIGNAL_OUTPUT_PROFILE_SIZE 4
#define OUTPUT_SIGNAL_OUTPUTS_ACTIVATED_SIZE 16
#define OUTPUT_SIGNAL_OUTPUT_DURATION_SIZE 16
//Padding bits for 3DES encryption of OutputSignal payload (needs to be 8-byte aligned)
#define OUTPUT_SIGNAL_PAYLOAD_PADDING_SIZE 7

// Alarm Output Signal message
#define ALARM_OUTPUT_STATE_PROFILE_SIZE 9
#define ALARM_OUTPUT_STATE_PAYLOAD_PADDING_SIZE 32

/* State Signal Message */
#define STATE_SIGNAL_STATE_SIZE 4
#define STATE_SIGNAL_PAYLOAD_PADDING_SIZE 55

/* RBU Disable Message */
#define RBU_DISABLE_MSG_UNIT_ADDRESS_SIZE 9
#define RBU_DISABLE_MSG_PADDING_SIZE 50

/* Output State messages */
#define OUTPUT_STATE_REQUEST_PADDING_SIZE 59
#define OUTPUT_STATE_RESPONSE_OUTPUT_ACTIVATED_SIZE 1
#define OUTPUT_STATE_RESPONSE_PROFILE_SIZE 4
#define OUTPUT_STATE_RESPONSE_CHANNEL_SIZE 6
#define OUTPUT_STATE_RESPONSE_DURATION_SIZE 16
#define OUTPUT_STATE_RESPONSE_MASK_SIZE 8
#define OUTPUT_STATE_RESPONSE_ZONE_SIZE NUM_OF_ZONE_BITS

/* Battery Status Message */
#define BATTERY_STATUS_MSG_VOLTAGE_SIZE 14
#define BATTERY_STATUS_MSG_DEVICE_COMBINATION_SIZE 7
#define BATTERY_STATUS_MSG_ZONE_NUMBER_SIZE NUM_OF_ZONE_BITS
#define BATTERY_STATUS_MSG_SENSOR_READING_SIZE 7
#define BATTERY_STATUS_MSG_PIR_READING_SIZE 1
#define BATTERY_STATUS_MSG_PADDING_SIZE 1

//Ping
#define PING_REQUEST_PAYLOAD_SIZE 32
#define PING_REQUEST_PADDING_SIZE 27

//Zone Enable
#define ZONE_ENABLE_LOWHIGH_SIZE 1
#define ZONE_ENABLE_HALFWORD_SIZE 16
#define ZONE_ENABLE_WORD_SIZE 32
#define ZONE_ENABLE_PADDING_SIZE 10

//Global Delays
#define GLOBAL_DELAY_CHANNEL_LOCAL_SIZE 8
#define GLOBAL_DELAY_SIZE 16
#define GLOBAL_DELAY_PADDING_SIZE 19

/* Macros
*************************************************************************************/

#define MC_PUP_ASSERT(a) if (!(a)) return ERR_OUT_OF_RANGE_E;




#endif // MC_PUP_H
