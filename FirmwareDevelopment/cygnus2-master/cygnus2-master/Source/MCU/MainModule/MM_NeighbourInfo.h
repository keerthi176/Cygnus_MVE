/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2018 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : MM_NeighbourInfo.h
*
*  Description  : Header for node relationships functionality
*
*************************************************************************************/

#ifndef MM_NEIGHBOURINFO_H
#define MM_NEIGHBOURINFO_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>



/* User Include Files
*************************************************************************************/
#include "CO_Message.h"
#include "MC_PingRecovery.h"

/* Public Constants
*************************************************************************************/
#define NI_ID_NOT_ASSIGNED -1
#define NI_SNR_NOT_ASSIGNED -100
#define NI_SNR_MINIMUM_UINT8 0x20
#define NI_RSSI_NOT_ASSIGNED -256

/* Macros
*************************************************************************************/


/* Public Enumerations
*************************************************************************************/
typedef enum 
{
   NI_MESH_LINK_ADDED_E,
   NI_MESH_LINK_DROPPED_E,
   NI_MESH_NODE_LOG_OFF_E
} CIEMeshEvent_t;



/* Public Structures
*************************************************************************************/ 
typedef struct
{
   uint16_t NodeID;//NCU Cache
   uint8_t  DeviceCombination;
   uint32_t SerialNumber;
   uint16_t ZoneNumber;

   uint8_t  LostHeartbeatCount;  //SMGR_ChildDetails_t
   uint8_t  FailedResponseCount;
   int8_t   SNR;
   int8_t   RSSI;
   int8_t   state;
   
   PingRecoveryState_t PingState;     //Ping Recovery
   uint8_t  LostHeartbeatPingCount;
   uint8_t  LostHeartbeatPingInterval;
   
   int16_t PrimaryParentID; //NeighbourInformation_t
   int16_t PrimaryParentRssi;
   int16_t SecondaryParentID;
   int16_t SecondaryParentRssi;
   int16_t PrimaryTrackingNodeID;
   int16_t SecondaryTrackingNodeID;
   uint16_t LastStatusEvent;
   uint16_t LastStatusEventNodeId;
   CIEMeshEvent_t LastMeshEvent;
   int16_t LastMeshEventParentID;
   int16_t LastMeshEventChildID;
   uint16_t primaryBattery_mv;            /* The last reported primary battery voltage in mv */
   uint16_t backupBattery_mv;            /* The last reported back-up battery voltage in mv */
   uint8_t Rank;
   uint16_t NumberOfChildren;
   uint8_t FaultStatus;
} NeighbourInformation_t;



/* Public Functions Prototypes
*************************************************************************************/
void MM_NI_Initialise( void );
bool MM_NI_UpdateMeshStatus( const StatusIndication_t* const pStatus );
bool MM_NI_UpdateBatteryStatus( const BatteryStatusIndication_t* const pStatus );
bool MM_NI_GetNeighbourInformation( const uint16_t SourceNode, NeighbourInformation_t* const pNeighbourInfo );
void MM_NI_DumpNeighbourInformation(const uint32_t firstNode, const uint32_t lastNode );
void MM_NI_SetFaultStatus(const uint32_t node, const uint8_t faultStatus );

#endif // MM_INTERRUPTS_H
