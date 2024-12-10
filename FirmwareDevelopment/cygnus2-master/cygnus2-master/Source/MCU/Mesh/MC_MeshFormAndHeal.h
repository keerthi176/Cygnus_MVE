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
*  File         : MC_MeshFormAndHeal.h
*
*  Description  : Mesh forming and healing module header file
*
*************************************************************************************/

#ifndef MC_MESH_FORM_AND_HEAL_H
#define MC_MESH_FORM_AND_HEAL_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>


/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "MC_SessionManagement.h"

/* Public Constants
*************************************************************************************/
#define MFH_MAX_SHORT_LIST_SIZE 512
#define MFH_RANK_NOT_PUBLISHED 63
#define MFH_STALE_HEARTBEAT_THRESHOLD 2
#define MAX_NO_TRACKING_NODES         4         // The maximum number of tracking nodes supported by the sync algorithm

/* Public Structures
*************************************************************************************/ 
typedef struct
{
   uint16_t NodeID;
   int16_t SNR;
   int16_t RSSI;
   uint32_t LastTimestamp;
   uint32_t AveragePeriod;
   int32_t FrameCount;
   uint8_t Rank;
   uint8_t MissedHeartbeats;
   uint32_t LongFrameOffset;
   uint32_t LongFramesSinceLastUpdate;
   uint8_t NumberOfChildren;
   bool Unused;
} ShortListElement_t;

typedef enum
{
   PRIMARY_PARENT_E,
   SECONDARY_PARENT_E,
   PRIMARY_TRACKING_NODE_E,
   SECONDARY_TRACKING_NODE_E,
   NOT_ASSIGNED_E
} MC_TrackingNodeRole_t;

typedef struct
{
   MC_TrackingNodeRole_t role;
   uint16_t  slotIdx;
   uint16_t  node_id;
} MC_TrackingNodeListEntry_t;

typedef struct
{
   MC_TrackingNodeListEntry_t aTrackingNodeList[MAX_NO_TRACKING_NODES];
   uint8_t   noOfTrackingNodes;
} MC_TrackingNodes_t;

/* Public Enumerations
*************************************************************************************/
typedef enum 
{
   MFH_STATE_WAITING_FOR_SYNC_E,
   MFH_STATE_REFINING_SYNC_NODE_E,
   MFH_STATE_MAINTAINING_SHORT_LIST_E,
   MFH_STATE_SELECTING_PARENTS_E,
   MFH_STATE_SCANNING_FOR_DEVICES_E,
} MC_MFH_State_t;

/* Public Functions Prototypes
*************************************************************************************/
void MC_MFH_Init(const bool isNCU);
void MC_MFH_Update(const uint16_t nodeID, const uint32_t slotInSuperframe, const int16_t snr, const int16_t rssi, const uint32_t lastTimeStamp, const uint8_t rank, const CO_State_t state, const uint8_t numberOfChildren, const bool valid);
void MC_MFH_RankUpdate(const uint8_t rank);
void MC_MFH_FreeNode(const uint16_t nodeID);
uint16_t MC_MFH_GetStrongestNode(void);
ErrorCode_t MC_MFH_FindNode(const uint16_t nodeID, ShortListElement_t* const pNode);
bool MC_MFH_DoParentSelection(void);
void MC_MFH_SelectNewParent(void);
void MC_MFH_InhibitNodeSelection(const uint16_t nodeID);
bool MC_MFH_InitialTrackingNodeActive(void);
void MC_MFH_SetLoggedOn(void);
int16_t MC_MFH_GetSNR(const uint16_t nodeID);
int16_t MC_MFH_GetRSSI(const uint16_t nodeID);
ErrorCode_t MC_MFH_DeleteNode(const uint16_t nodeID, const bool updateSessionManagement);
void MC_MFH_AdvanceLongFrame(void);
MC_MFH_State_t MC_MFH_GetState(void);
void MC_MFH_ResetMissedHeartbeatCount(const uint16_t nodeID);
void MC_MFH_DumpVisibleDevices(const uint32_t firstNode, const uint32_t lastNode, const UartComm_t uart_port );
void MC_MFH_NewLongFrame(void);
void MC_MFH_ScanForDevices(void);
bool MC_MFH_ScanForDevicesInProgress(void);
bool MC_MFH_NodeResponding(const uint16_t nodeID);
ErrorCode_t MC_MFH_HandleLostNode(const uint16_t nodeID);
#endif // MC_MESH_FORM_AND_HEAL_H
