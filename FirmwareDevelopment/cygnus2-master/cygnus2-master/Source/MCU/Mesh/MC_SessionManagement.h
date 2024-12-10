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
*  File        : MC_SessionManagement.h
*
*  Description : Header for the management of parent and child nodes.
*
*************************************************************************************/

#ifndef MC_SESSION_MANAGEMENT_H_H
#define MC_SESSION_MANAGEMENT_H_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>


/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "MC_MAC.h"
#include "MC_TDM.h"

/* Public definitions
*************************************************************************************/
#define PRIMARY_PARENT 0
#define SECONDARY_PARENT 1
#define PRIMARY_TRACKING_NODE 2
#define SECONDARY_TRACKING_NODE 3
#define NUMBER_OF_PARENTS_AND_TRACKING_NODES (4)
#define MAX_NUMBER_OF_SYSTEM_NODES 513
#define MAX_NUMBER_OF_CHILDREN  (MAX_CHILDREN_PER_PARENT)
#define NODE_INFO_MASK_NODEID 0x0fff
#define NODE_INFO_MASK_MISSING_HB_COUNT 0xf000
#define MAX_MISSED_HEARTBEATS 1
#define SECONDARY_PARENT_NOT_ASSIGNED CO_BAD_NODE_ID
#define PRIMARY_PARENT_NOT_ASSIGNED CO_BAD_NODE_ID
#define SECONDARY_TRACKING_NODE_NOT_ASSIGNED CO_BAD_NODE_ID
#define PRIMARY_TRACKING_NODE_NOT_ASSIGNED CO_BAD_NODE_ID
#define NEIGHBOUR_TYPE_PARENT 0
#define NEIGHBOUR_TYPE_CHILD 1
#define PARENT_NOT_RESPONDING_THRESHOLD ACK_MAX_BACKOFF_EXPONENT  /* how many consecutive messages go without an ACK before we drop a parent */
#define PRIMARY_RACH 0
#define SECONDARY_RACH 1
#define NETWORK_BUSY_THRESHOLD 6
#define SMGR_ROUTE_ADD_RESPONSE_TIMEOUT 20
#define SMGR_ROUTE_ADD_MAX_ATTEMPTS 3
#define MAX_CHILD_IDX_BITS            4
#define MAX_CHILD_IDX                 ((1 << MAX_CHILD_IDX_BITS) - 1)  // Maximum value of child Idx field in heartbeat is 15.

typedef struct
{
   uint16_t NodeID;
   bool     IsActive;/*has acknowledged the join message */
   bool     WaitingForRouteAddResponse;
   uint8_t  RouteAddResponsesReceived;
   uint16_t ShortFramesSinceRouteAdd;  /* a count of short frames since route add was sent. Used for timeout of message */
   uint8_t  RouteAddRetries;           /* The number of times a route add msg has been resent */
   uint8_t  PublishedRank;             /* The rank that was published in the previous heartbeat (could be unpublished due to missed parent Hb)*/
} SMGR_ParentDetails_t;


/* Public Structures
*************************************************************************************/

/* Public functions
*************************************************************************************/
void MC_SMGR_Initialise(const bool isMaster, const uint16_t address);
uint16_t MC_SMGR_CalculateDCHSlot(const uint16_t node_id);
void MC_SMGR_SetRank(const uint8_t rank);
uint8_t MC_SMGR_GetRank(void);
bool MC_SMGR_SetParentNodes(const uint16_t primary_address, const uint16_t secondary_address);
bool MC_SMGR_SetTrackingNodes(const uint16_t primary_tracking_nodeID, const uint16_t secondary_tracking_nodeID);
bool MC_SMGR_IsParentNode(const uint16_t node_id);
uint16_t MC_SMGR_GetPrimaryParentID(void);
uint16_t MC_SMGR_GetSecondaryParentID(void);
uint16_t MC_SMGR_GetNextParentForUplink(void);
bool MC_SMGR_ConnectToParent(const uint16_t node_id, const bool use_sach);
bool MC_SMGR_RouteAddResponse(const RouteAddResponseMessage_t* pResponse);
bool MC_SMGR_RouteAddRequest(const uint16_t node_id, const bool is_primary);
bool MC_SMGR_RouteDropRequest(const uint16_t node_id);
bool MC_SMGR_IsAChild(const uint16_t node_id);
uint16_t MC_SMGR_GetNumberOfChildren(void);
uint16_t MC_SMGR_GetNumberOfChildrenIndex(void);
uint16_t MC_SMGR_GetNumberOfParents(void);
uint16_t MC_SMGR_GetNumberOfTrackingNodes(void);
bool MC_SMGR_ReceivedNodeHeartbeat(const uint16_t node_id, const int8_t snr, const int8_t rssi, const CO_State_t state, const int8_t rank,
                                   const uint8_t  NoOfChildrenIdx);
bool MC_SMGR_MissedNodeHeartbeat(const uint16_t node_id);
bool MC_SMGR_SetHeartbeatMonitor_NodeId(const uint16_t node_id, const bool set_monitor);
bool MC_SMGR_StartHeartbeatMonitorForNode(const uint16_t node_id);
bool MC_SMGR_StopHeartbeatMonitorForNode(const uint16_t node_id);
void MC_SMGR_SendStatusIndication(const CO_MeshEvent_t event, const uint16_t nodeID, const uint32_t nodeData, const bool sendOnDulch);
bool MC_SMGR_ParentSessionIsActive(void);
void MC_SMGR_SetAllDCHSlotBehaviour(const MC_TDM_DCHBehaviour_t defaultBehaviour);
bool MC_SMGR_GetNeighbourInformation(const uint8_t neighbour_type, const uint8_t neighbour_index, uint32_t* pNeighbourInfo);
bool MC_SMGR_ChangeNodeRole(const uint8_t srcIdx, const uint8_t dstIdx);
void MC_SMGR_CheckAndFixParentConnections(void);
bool MC_SMGR_CheckParentStates(const CO_State_t localState, const uint8_t LongFrameIndex);
bool MC_SMGR_DeleteNode( const uint16_t node_id, const char* reason );
int16_t MC_SMGR_GetAverageSNRForTrackingNode(const uint16_t TrackingNodeID);
int16_t MC_SMGR_GetAverageRssiForTrackingNode(const uint16_t TrackingNodeID);
uint8_t MC_SMGR_GetNumberOfChildrenForPrimaryTrackingNode(void);
void MC_SMGR_ReportAssociatedNodes(const uint32_t source);
void MC_SMGR_SendRouteDrop(const uint16_t node_id, const ResetReason_t reason, const bool wait_for_ack, const bool is_downlink, const uint32_t handle);
bool MC_SMGR_IsATrackingNode(const uint16_t node_id);
bool MC_SMGR_NotifyMessageFailure(const ApplicationLayerMessageType_t MessageType, const uint16_t MacDestination);
void MC_SMGR_ProcessParentAckReport(const uint16_t parent_nodeID, const bool ack_received, const ApplicationLayerMessageType_t message_type);
void MC_SMGR_ProcessParentStatusReport( const StatusIndicationMessage_t* const pStatusMessage);
void MC_SMGR_UpdateNetworkStatistics(const uint8_t channel, const bool messageDetected);
void MC_SMGR_UpdateRachCount(const uint8_t channel);
void MC_SMGR_DoLongframeStateValidation(void);
void MC_SMGR_ProcessPingResponse(const uint16_t source_node, const bool ack_received);
bool MC_SMGR_RemakeLink(const uint16_t source_node);
bool MC_SMGR_PopulateRouteAddResponse( const bool accepted, const uint8_t zone, const uint8_t zoneMapLowerUpper, CO_RouteAddResponseData_t* pResponseData);
void MC_SMGR_SetPowerStatus(const uint32_t status);
uint8_t MC_SMGR_GetTxPower(void);
void MC_SMGR_CalculateTxPower(void);
void MC_SMGR_SetLoggedOn(void);
bool MC_SMGR_IsLoggedOn(void);
void MC_SMGR_DelayedReset(const char* pReset);
uint8_t MC_SMGR_GetPublishedRank(void);
void MC_SMGR_ProcessRouteAddFailure(const uint16_t node_id);
uint16_t MC_SMGR_GetChildIDByIndex(const uint16_t index);
void MC_SMGR_PurgeNodes(const uint16_t nodeID);
bool MC_SMGR_NetworkBusy(const uint8_t channel);
#ifdef SHOW_NETWORK_USAGE
void MC_SMGR_ShowNetworkUsage(void);
#endif
#ifdef ENABLE_LINK_ADD_OR_REMOVE
void MC_SMGR_ForceAddNode(const uint16_t node_id, const bool is_child);
#endif
#endif /* MC_SESSION_MANAGEMENT_H_H */
