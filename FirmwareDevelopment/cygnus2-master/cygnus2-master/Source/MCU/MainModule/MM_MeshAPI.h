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
*  File         : MM_MeshAPI.h
*
*  Description  : Mesh Protocol API header file
*
*************************************************************************************/

#ifndef MM_MESH_API_H
#define MM_MESH_API_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "CO_Message.h"

/* User Include Files
*************************************************************************************/



/* Public Structures
*************************************************************************************/ 
/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
bool MM_MeshAPIFireSignalReq( const uint32_t Handle, const CO_RBUSensorData_t* pSensorData );
bool MM_MeshAPIAlarmSignalReq( const uint32_t Handle, const CO_RBUSensorData_t* pSensorData );
bool MM_MeshAPIOutputSignalReq( const uint32_t Handle, const uint16_t Destination, const uint16_t zone, const uint8_t OutputChannel, const uint8_t OutputProfile, const uint16_t OutputsActivated, const uint8_t OutputDuration, const uint8_t NumberToSend );
bool MM_MeshAPIAlarmOutputSignalReq( const uint32_t Handle, const CO_AlarmOutputStateData_t *AlarmOutputState );
bool MM_MeshAPILogonReq( const uint32_t Handle,const uint32_t SerialNumber, const uint8_t DeviceCombination, const uint16_t ZoneNumber );
bool MM_MeshAPICommandReq(const uint32_t Handle, const uint16_t Source, const uint16_t Destination, const uint8_t CommandType,
                           const uint8_t Parameter1, const uint8_t Parameter2, const uint32_t Value, const uint8_t ReadWrite, const uint8_t NumberToSend);
bool MM_MeshAPIResponseReq(const uint32_t Handle, const uint16_t Source, const uint16_t Destination, const uint8_t CommandType,
                           const uint8_t Parameter1, const uint8_t Parameter2, const uint32_t Value, const uint8_t ReadWrite);
bool MM_MeshAPIGenerateTestMessageReq( const char* const payload );
bool MM_MeshAPIGenerateSetStateReq( const CO_State_t state );
bool MM_MeshAPIGenerateRBUDisableReq( const uint32_t Handle, const uint16_t unitAddress );
bool MM_MeshAPIGenerateStatusSignalReq( const uint32_t Handle, const uint8_t Event, const uint16_t EventNodeId, const uint32_t EventData, const uint8_t OverallFaultStatus, const bool DelaySending  );
bool MM_MeshAPIGenerateFaultSignalReq( const CO_FaultData_t* const faultFata );
bool MM_MeshAPIGenerateRBUReportNodesReq( const uint32_t source );
bool MM_MeshAPIApplicationCommand(const ApplicationLayerMessageType_t MsgType, const int32_t value1, const int32_t value2, const uint8_t source, const uint32_t handle);
bool MM_MeshAPIGenerateBatteryStatusSignalReq( const CO_BatteryStatusData_t* const batteryData );
bool MM_MeshAPIGenerateZoneEnableReq( const ZoneEnableData_t* const zoneData );
bool MM_MeshAPIGenerateAtMessage( const AtMessageData_t* const atCommand );
bool MM_MeshAPIGenerateStartAtModeMessage( const OtaMode_t* const atCommand );
bool MM_MeshAPIGeneratePingRequest( const PingRequest_t* const pingReq );
bool MM_MeshAPIGeneratePpuMessage(const PPU_Message_t* const pPpuMessage);

/* Public Constants
*************************************************************************************/



/* Macros
*************************************************************************************/



#endif // MM_MESH_API_H
