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
*  File         : MM_NeighbourInfo.c
*
*  Description  : Maintains a record of the node relationships within the mesh network
*
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include "cmsis_os.h"


/* User Include Files
*************************************************************************************/
#include "MM_NeighbourInfo.h"
#include "MM_CIEQueueManager.h"

/* Private definitions
*************************************************************************************/
#define NI_DEFAULT_SNR_VALUE 10U
#define NI_DEFAULT_RSSI_VALUE -100

/* Private Functions Prototypes
*************************************************************************************/
static bool MM_NI_UpdatePrimaryParentAdded( const uint16_t SourceNode, const uint16_t ParentNode, const uint16_t ParentRssi );
static bool MM_NI_UpdatePrimaryParentDropped( const uint16_t SourceNode, const uint16_t ParentNode, const bool autoPromote );
static bool MM_NI_UpdateSecondaryParentAdded( const uint16_t SourceNode, const uint16_t ParentNode, const uint16_t ParentRssi );
static bool MM_NI_UpdateSecondaryParentDropped( const uint16_t SourceNode, const uint16_t ParentNode );
static bool MM_NI_UpdateSecondaryParentPromoted( const uint16_t SourceNode, const uint16_t ParentNode, const uint16_t ParentRssi );
static bool MM_NI_UpdatePrimaryTrackingNodeAdded( const uint16_t SourceNode, const uint16_t ParentNode );
static bool MM_NI_UpdatePrimaryTrackingNodeDropped( const uint16_t SourceNode, const uint16_t ParentNode );
static bool MM_NI_UpdateSecondaryTrackingNodeAdded( const uint16_t SourceNode, const uint16_t ParentNode );
static bool MM_NI_UpdateSecondaryTrackingNodeDropped( const uint16_t SourceNode, const uint16_t ParentNode );
static bool MM_NI_UpdatePrimaryTrackingNodePromoted( const uint16_t SourceNode, const uint16_t TrackingNode, const uint16_t TrackingRssi );
static bool MM_NI_UpdateChildDropped( const uint16_t SourceNode, const uint16_t ChildNode );
static bool MM_NI_UpdateChildAdded( const uint16_t SourceNode, const uint16_t ChildNode );
static void MM_NI_SendMeshUpdateToCIE( const CIEMeshEvent_t event, const uint16_t parentID, const uint16_t childID, const uint16_t sourceNode );
static void MM_NI_PurgeNodeTree( const uint16_t lostNode );

/* Global Variables
*************************************************************************************/
#ifdef RUN_BUILD
NeighbourInformation_t neighbourArray[MAX_DEVICES_PER_SYSTEM]; 
#else
NeighbourInformation_t neighbourArray[MAX_DEVICES_PER_SYSTEM] __attribute__( ( section( "NeighbourRecords"),used) );
#endif

/* Private Variables
*************************************************************************************/
   
/*************************************************************************************/
/**
* MM_NI_Initialise
* Initialise the neighbour information data
*
* param - void
*
* return - void
*/
void MM_NI_Initialise( void )
{
   for ( uint16_t index = 0; index < MAX_DEVICES_PER_SYSTEM; index++ )
   {
      neighbourArray[index].PrimaryParentID = NI_ID_NOT_ASSIGNED;
      neighbourArray[index].PrimaryParentRssi = NI_RSSI_NOT_ASSIGNED;
      neighbourArray[index].SecondaryParentID = NI_ID_NOT_ASSIGNED;
      neighbourArray[index].SecondaryParentRssi = NI_RSSI_NOT_ASSIGNED;
      neighbourArray[index].PrimaryTrackingNodeID = NI_ID_NOT_ASSIGNED;
      neighbourArray[index].SecondaryTrackingNodeID = NI_ID_NOT_ASSIGNED;
      neighbourArray[index].LastMeshEvent = NI_MESH_NODE_LOG_OFF_E;
      neighbourArray[index].LastMeshEventParentID = NI_ID_NOT_ASSIGNED;
      neighbourArray[index].LastMeshEventChildID = NI_ID_NOT_ASSIGNED;
      neighbourArray[index].backupBattery_mv = 0;
      neighbourArray[index].DeviceCombination = 0;
      neighbourArray[index].FailedResponseCount = 0;
      neighbourArray[index].LastStatusEvent = 0;
      neighbourArray[index].LastStatusEventNodeId = 0;
      neighbourArray[index].LostHeartbeatCount = 0;
      neighbourArray[index].LostHeartbeatPingCount = 0;
      neighbourArray[index].LostHeartbeatPingInterval = 0;
      neighbourArray[index].PingState = PR_IDLE_E;
      neighbourArray[index].NodeID = CO_BAD_NODE_ID;
      neighbourArray[index].primaryBattery_mv = 0;
      neighbourArray[index].Rank = UNASSIGNED_RANK;
      neighbourArray[index].RSSI = 0;
      neighbourArray[index].SNR = 0;
      neighbourArray[index].state = 0;
      neighbourArray[index].ZoneNumber = 0;
      neighbourArray[index].NumberOfChildren = 0;
      neighbourArray[index].FaultStatus = 0;
   }
}


/*************************************************************************************/
/**
* MM_NI_UpdateMeshStatus
* Update the neighbour record with Mesh Status information
*
* param - pStatus   The status record for the update.
*
* return - bool     True if the update succeeds
*/
bool MM_NI_UpdateMeshStatus( const StatusIndication_t* const pStatus )
{
   bool result = false;
   
   if ( pStatus )
   {   
      if ( MAX_DEVICES_PER_SYSTEM > pStatus->SourceNode )
      {
         //Update the cache with the latest from the source node
         neighbourArray[pStatus->SourceNode].NodeID = pStatus->SourceNode;
         neighbourArray[pStatus->SourceNode].PrimaryParentID = pStatus->PrimaryParent;
         neighbourArray[pStatus->SourceNode].PrimaryParentRssi = (int16_t)(pStatus->AverageRssiPrimaryParent | RSSI_SIGNED_BITS);
         neighbourArray[pStatus->SourceNode].SecondaryParentID = pStatus->SecondaryParent;
         neighbourArray[pStatus->SourceNode].FaultStatus = pStatus->OverallFault;
//         if ( NI_SNR_MINIMUM_UINT8 == pStatus->AverageRssiSecondaryParent )
//         {
//            neighbourArray[pStatus->SourceNode].SecondaryParentRssi = NI_RSSI_NOT_ASSIGNED;
//         }
//         else 
//         {
//            neighbourArray[pStatus->SourceNode].SecondaryParentRssi = pStatus->AverageRssiSecondaryParent;
//         }
         neighbourArray[pStatus->SourceNode].SecondaryParentRssi = (int16_t)(pStatus->AverageRssiSecondaryParent | RSSI_SIGNED_BITS);
         neighbourArray[pStatus->SourceNode].Rank = pStatus->Rank;

         switch ( pStatus->Event )
         {
            case CO_MESH_EVENT_PRIMARY_PARENT_ADDED_E:
               result = MM_NI_UpdatePrimaryParentAdded(pStatus->SourceNode, pStatus->EventData, pStatus->AverageRssiPrimaryParent);
               break;
            case CO_MESH_EVENT_PRIMARY_PARENT_DROPPED_E:
               result = MM_NI_UpdatePrimaryParentDropped(pStatus->SourceNode, pStatus->EventData, false);
               break;
            case CO_MESH_EVENT_SECONDARY_PARENT_ADDED_E:
               result = MM_NI_UpdateSecondaryParentAdded(pStatus->SourceNode, pStatus->EventData, pStatus->AverageRssiSecondaryParent);
               break;
            case CO_MESH_EVENT_SECONDARY_PARENT_DROPPED_E:
               result = MM_NI_UpdateSecondaryParentDropped(pStatus->SourceNode, pStatus->EventData);
               break;
            case CO_MESH_EVENT_SECONDARY_PARENT_PROMOTED_E:
               result = MM_NI_UpdateSecondaryParentPromoted(pStatus->SourceNode, pStatus->EventData, pStatus->AverageRssiSecondaryParent);
               break;
            case CO_MESH_EVENT_PRIMARY_TRACKING_NODE_ADDED_E:
               result = MM_NI_UpdatePrimaryTrackingNodeAdded(pStatus->SourceNode, pStatus->EventData);
               break;
            case CO_MESH_EVENT_PRIMARY_TRACKING_NODE_DROPPED_E:
               result = MM_NI_UpdatePrimaryTrackingNodeDropped(pStatus->SourceNode, pStatus->EventData);
               break;
            case CO_MESH_EVENT_SECONDARY_TRACKING_NODE_ADDED_E:
               result = MM_NI_UpdateSecondaryTrackingNodeAdded(pStatus->SourceNode, pStatus->EventData);
               break;
            case CO_MESH_EVENT_SECONDARY_TRACKING_NODE_DROPPED_E:
               result = MM_NI_UpdateSecondaryTrackingNodeDropped(pStatus->SourceNode, pStatus->EventData);
               break;
            case CO_MESH_EVENT_PRIMARY_TRACKING_NODE_PROMOTED_E:
               result = MM_NI_UpdatePrimaryTrackingNodePromoted(pStatus->SourceNode, pStatus->EventData, pStatus->AverageRssiSecondaryParent);
               break;
            case CO_MESH_EVENT_CHILD_NODE_DROPPED_E:
               result = MM_NI_UpdateChildDropped(pStatus->SourceNode, pStatus->EventData);
               break;
            case CO_MESH_EVENT_CHILD_NODE_ADDED_E:
               //We don't record child relationships, but in the case of the NCU we must find the child record and add the NCU as a parent
               MM_NI_UpdateChildAdded( pStatus->SourceNode, pStatus->EventData );
               MM_NI_SendMeshUpdateToCIE( NI_MESH_LINK_ADDED_E, pStatus->SourceNode, pStatus->EventData, pStatus->SourceNode );
               result = true;
               break;
            default:
               /* The other events don't influence the records.  Just report success */
               result = true;
               break;
         }
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* MM_NI_UpdateBatteryStatus
* Update the neighbour record with Battery Status information
*
* param - pStatus   The status record for the update.
*
* return - bool     True if the update succeeds
*/
bool MM_NI_UpdateBatteryStatus( const BatteryStatusIndication_t* const pStatus )
{
   bool result = false;
   
   if ( pStatus )
   {   
      if ( MAX_DEVICES_PER_SYSTEM > pStatus->SourceAddress )
      {
         neighbourArray[pStatus->SourceAddress].primaryBattery_mv = pStatus->PrimaryBatteryVoltage;
         neighbourArray[pStatus->SourceAddress].backupBattery_mv = pStatus->BackupBatteryVoltage;
         neighbourArray[pStatus->SourceAddress].DeviceCombination = pStatus->DeviceCombination;
         neighbourArray[pStatus->SourceAddress].ZoneNumber = pStatus->ZoneNumber;
         result = true;
      }
   }
   
   return result;
}
/*************************************************************************************/
/**
* MM_NI_GetNeighbourInformation
* Get a node record from the neighbour array.
*
* param - SourceNode      The node ID of the record to be updated.
* param - pNeighbourInfo  [OUT] Pointer to the structure to write the results to.
*
* return - bool           True if pNeighbourInfo has been updated
*/
bool MM_NI_GetNeighbourInformation( const uint16_t SourceNode, NeighbourInformation_t* const pNeighbourInfo )
{
   bool result = false;
   
   if ( pNeighbourInfo )
   {
      if ( MAX_DEVICES_PER_SYSTEM > SourceNode )
      {
         pNeighbourInfo->NodeID = neighbourArray[SourceNode].NodeID;
         pNeighbourInfo->PrimaryParentID = neighbourArray[SourceNode].PrimaryParentID;
         pNeighbourInfo->PrimaryParentRssi = neighbourArray[SourceNode].PrimaryParentRssi;
         pNeighbourInfo->SecondaryParentID = neighbourArray[SourceNode].SecondaryParentID;
         pNeighbourInfo->SecondaryParentRssi = neighbourArray[SourceNode].SecondaryParentRssi;
         pNeighbourInfo->LastStatusEvent = neighbourArray[SourceNode].LastStatusEvent;
         pNeighbourInfo->LastStatusEventNodeId = neighbourArray[SourceNode].LastStatusEventNodeId;
         pNeighbourInfo->LastMeshEvent = neighbourArray[SourceNode].LastMeshEvent;
         pNeighbourInfo->LastMeshEventParentID = neighbourArray[SourceNode].LastMeshEventParentID;
         pNeighbourInfo->LastMeshEventChildID = neighbourArray[SourceNode].LastMeshEventChildID;
         pNeighbourInfo->primaryBattery_mv = neighbourArray[SourceNode].primaryBattery_mv;
         pNeighbourInfo->backupBattery_mv = neighbourArray[SourceNode].backupBattery_mv;
         pNeighbourInfo->DeviceCombination = neighbourArray[SourceNode].DeviceCombination;
         pNeighbourInfo->Rank = neighbourArray[SourceNode].Rank;
         pNeighbourInfo->NumberOfChildren = neighbourArray[SourceNode].NumberOfChildren;
         pNeighbourInfo->FaultStatus = neighbourArray[SourceNode].FaultStatus;
         result = true;
      }
   }
   return result;
}

/*************************************************************************************/
/**
* MM_NI_UpdatePrimaryParentAdded
* Update the source node with a new primary parent
*
* param - SourceNode   The source node of the event.
* param - ParentNode   The parent that was added.
* param - ParentRssi    The parent node's RSSI.
*
* return - bool        True if the update succeeds
*/
bool MM_NI_UpdatePrimaryParentAdded( const uint16_t SourceNode, const uint16_t ParentNode, const uint16_t ParentRssi )
{
   bool result = false;
   
   if ( MAX_DEVICES_PER_SYSTEM > SourceNode )
   {
      CO_PRINT_B_3(DBG_INFO_E,"Cache Update: Node %d, Primary Parent %d, SNR %d\r\n", SourceNode, ParentNode, ParentRssi);
      result = true;
      MM_NI_SendMeshUpdateToCIE( NI_MESH_LINK_ADDED_E, ParentNode, SourceNode, SourceNode );
   }
   
   return result;
}

/*************************************************************************************/
/**
* MM_NI_UpdatePrimaryParentDropped
* Update the source node to remove the primary parent
*
* param - SourceNode   The source node of the event.
* param - ParentNode   The parent that was dropped.
* param - autoPromote  True if called from the MM_NI_PurgeNodeTree function.
*
* return - bool        True if the update succeeds
*/
bool MM_NI_UpdatePrimaryParentDropped( const uint16_t SourceNode, const uint16_t ParentNode, const bool autoPromote  )
{
   bool result = false;
   
   if ( MAX_DEVICES_PER_SYSTEM > SourceNode )
   {
      CO_PRINT_B_4(DBG_INFO_E,"Cache Update: Node %d, Dropped Primary Parent %d, Promoted node %d, RSSI %d\r\n", SourceNode, ParentNode, neighbourArray[SourceNode].SecondaryParentID, neighbourArray[SourceNode].SecondaryParentRssi );
      if ( autoPromote )
      {
         /* Assume the RBU will promote the secondary parent */ 
         if ( NI_ID_NOT_ASSIGNED != neighbourArray[SourceNode].SecondaryParentID )
         {
            neighbourArray[SourceNode].PrimaryParentID = neighbourArray[SourceNode].SecondaryParentID;
            neighbourArray[SourceNode].PrimaryParentRssi = neighbourArray[SourceNode].SecondaryParentRssi;
            neighbourArray[SourceNode].SecondaryParentID = NI_ID_NOT_ASSIGNED;
            neighbourArray[SourceNode].SecondaryParentRssi = NI_RSSI_NOT_ASSIGNED;
         }
      }
      
      result = true;
      
#ifdef ENABLE_VERBOSE_CIE_MESH_UPDATES
      MM_NI_SendMeshUpdateToCIE( NI_MESH_LINK_DROPPED_E, ParentNode, SourceNode, SourceNode );
#endif
      
      //Has the node been orphaned?  If so purge it from the cache.
      if ( (NI_ID_NOT_ASSIGNED == neighbourArray[SourceNode].PrimaryParentID) &&
           (NI_ID_NOT_ASSIGNED == neighbourArray[SourceNode].SecondaryParentID) )
      {
         MM_NI_SendMeshUpdateToCIE( NI_MESH_NODE_LOG_OFF_E, SourceNode, 0, SourceNode );
         MM_NI_PurgeNodeTree(SourceNode);
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* MM_NI_UpdateSecondaryParentAdded
* Update the source node with a new secondary parent
*
* param - SourceNode   The source node of the event.
* param - ParentNode   The parent that was added.
* param - ParentRssi   The parent node's RSSI.
*
* return - bool        True if the update succeeds
*/
bool MM_NI_UpdateSecondaryParentAdded( const uint16_t SourceNode, const uint16_t ParentNode, const uint16_t ParentRssi )
{
   bool result = false;

   if ( MAX_DEVICES_PER_SYSTEM > SourceNode )
   {
      CO_PRINT_B_3(DBG_INFO_E,"Cache Update: Node %d, Secondary Parent %d, SNR %d\r\n", SourceNode, ParentNode, ParentRssi);
      result = true;
      MM_NI_SendMeshUpdateToCIE( NI_MESH_LINK_ADDED_E, ParentNode, SourceNode, SourceNode );
   }
   return result;
}

/*************************************************************************************/
/**
* MM_NI_UpdateSecondaryParentDropped
* Update the source node to remove the secondary parent
*
* param - SourceNode   The source node of the event.
* param - ParentNode   The parent that was dropped.
*
* return - bool        True if the update succeeds
*/
bool MM_NI_UpdateSecondaryParentDropped( const uint16_t SourceNode, const uint16_t ParentNode )
{
   bool result = false;

   if ( MAX_DEVICES_PER_SYSTEM > SourceNode )
   {
      CO_PRINT_B_2(DBG_INFO_E,"Cache Update: Node %d, Dropped Secondary Parent %d\r\n", SourceNode, ParentNode );
      result = true;
#ifdef ENABLE_VERBOSE_CIE_MESH_UPDATES
      MM_NI_SendMeshUpdateToCIE( NI_MESH_LINK_DROPPED_E, ParentNode, SourceNode, SourceNode );
#endif
   }
   return result;
}

/*************************************************************************************/
/**
* MM_NI_UpdateSecondaryParentPromoted
* Update the source node to promote the secondary parent to primary parent
*
* param - SourceNode   The source node of the event.
* param - ParentNode   The parent that was promoted.
* param - ParentRssi   The parent node's RSSI.
*
* return - bool        True if the update succeeds
*/
bool MM_NI_UpdateSecondaryParentPromoted( const uint16_t SourceNode, const uint16_t ParentNode, const uint16_t ParentRssi )
{
   bool result = false;

   if ( MAX_DEVICES_PER_SYSTEM > SourceNode )
   {
      CO_PRINT_B_3(DBG_INFO_E,"Cache Update: Node %d, Promoted Secondary Parent %d, SNR %d\r\n", SourceNode, ParentNode, ParentRssi );
      result = true;
   }
   return result;
}

/*************************************************************************************/
/**
* MM_NI_UpdateSecondaryParentDropped
* Update the source node to add the primary tracking node.
*
* param - SourceNode   The source node of the event.
* param - TrkNode      The tracking node that was dropped.
*
* return - bool        True if the update succeeds
*/
bool MM_NI_UpdatePrimaryTrackingNodeAdded( const uint16_t SourceNode, const uint16_t TrkNode )
{
   bool result = false;
   if ( MAX_DEVICES_PER_SYSTEM > SourceNode )
   {
      CO_PRINT_B_2(DBG_INFO_E,"Cache Update: Node %d, Added primary tracking node %d\r\n", SourceNode, TrkNode );
      neighbourArray[SourceNode].PrimaryTrackingNodeID = TrkNode;
      result = true;
   }
   return result;
}

/*************************************************************************************/
/**
* MM_NI_UpdatePrimaryTrackingNodeDropped
* Update the source node to drop the primary tracking node.
*
* param - SourceNode   The source node of the event.
* param - ParentNode   The parent that was dropped.
*
* return - bool        True if the update succeeds
*/
bool MM_NI_UpdatePrimaryTrackingNodeDropped( const uint16_t SourceNode, const uint16_t TrkNode )
{
   bool result = false;
   if ( MAX_DEVICES_PER_SYSTEM > SourceNode )
   {
      CO_PRINT_B_2(DBG_INFO_E,"Cache Update: Node %d, Dropped primary tracking node %d\r\n", SourceNode, TrkNode );
      neighbourArray[SourceNode].PrimaryTrackingNodeID = neighbourArray[SourceNode].SecondaryTrackingNodeID;
      neighbourArray[SourceNode].SecondaryTrackingNodeID = NI_ID_NOT_ASSIGNED;
      result = true;
   }
   return result;
}

/*************************************************************************************/
/**
* MM_NI_UpdateSecondaryTrackingNodeAdded
* Update the source node to add the secondary tracking node.
*
* param - SourceNode   The source node of the event.
* param - TrkNode      The tracking node that was added.
*
* return - bool        True if the update succeeds
*/
bool MM_NI_UpdateSecondaryTrackingNodeAdded( const uint16_t SourceNode, const uint16_t TrkNode )
{
   bool result = false;
   if ( MAX_DEVICES_PER_SYSTEM > SourceNode )
   {
      CO_PRINT_B_2(DBG_INFO_E,"Cache Update: Node %d, Added secondary tracking node %d\r\n", SourceNode, TrkNode );
      neighbourArray[SourceNode].SecondaryTrackingNodeID = TrkNode;
      result = true;
   }
   return result;
}

/*************************************************************************************/
/**
* MM_NI_UpdateSecondaryTrackingNodeDropped
* Update the source node to drop the secondary tracking node.
*
* param - SourceNode   The source node of the event.
* param - TrkNode      The tracking node that was dropped.
*
* return - bool        True if the update succeeds
*/
bool MM_NI_UpdateSecondaryTrackingNodeDropped( const uint16_t SourceNode, const uint16_t TrkNode )
{
   bool result = false;
   if ( MAX_DEVICES_PER_SYSTEM > SourceNode )
   {
      CO_PRINT_B_2(DBG_INFO_E,"Cache Update: Node %d, Dropped secondary tracking node %d\r\n", SourceNode, TrkNode );
      neighbourArray[SourceNode].SecondaryTrackingNodeID = NI_ID_NOT_ASSIGNED;
      result = true;
   }
   return result;
}

/*************************************************************************************/
/**
* MM_NI_UpdatePrimaryTrackingNodePromoted
* Update the source node to promote the secondary parent to primary parent
*
* param - SourceNode      The source node of the event.
* param - TrackingNode    The tracking node that was promoted.
* param - TrackingRssi    The tracking node's RSSI.
*
* return - bool           True if the update succeeds
*/
bool MM_NI_UpdatePrimaryTrackingNodePromoted( const uint16_t SourceNode, const uint16_t TrackingNode, const uint16_t TrackingRssi )
{
   bool result = false;

   if ( MAX_DEVICES_PER_SYSTEM > SourceNode )
   {
      CO_PRINT_B_3(DBG_INFO_E,"Cache Update: Node %d, Promoted Trk Node %d, SNR %d\r\n", SourceNode, TrackingNode, TrackingRssi );
      result = true;
      MM_NI_SendMeshUpdateToCIE( NI_MESH_LINK_ADDED_E, TrackingNode, SourceNode, SourceNode );
   }
   return result;
}

/*************************************************************************************/
/**
* MM_NI_UpdateChildDropped
* Update the child node to remove a parent
*
* param - SourceNode   The source node of the event.
* param - ChildNode    The child that was dropped.
*
* return - bool        True if the update succeeds
*/
bool MM_NI_UpdateChildDropped( const uint16_t SourceNode, const uint16_t ChildNode )
{
   bool result = false;

   if ( MAX_DEVICES_PER_SYSTEM > ChildNode )
   {
      if ( neighbourArray[ChildNode].PrimaryParentID == SourceNode )
      {
         neighbourArray[ChildNode].PrimaryParentID = NI_ID_NOT_ASSIGNED;
         neighbourArray[SourceNode].PrimaryParentRssi = NI_RSSI_NOT_ASSIGNED;
         MM_NI_UpdatePrimaryParentDropped( ChildNode, SourceNode, true );
         result = true;
#ifdef ENABLE_VERBOSE_CIE_MESH_UPDATES
         MM_NI_SendMeshUpdateToCIE( NI_MESH_LINK_DROPPED_E, SourceNode, ChildNode, SourceNode );
#endif
      }
      else if ( neighbourArray[ChildNode].SecondaryParentID == SourceNode )
      {
         neighbourArray[ChildNode].SecondaryParentID = NI_ID_NOT_ASSIGNED;
         neighbourArray[SourceNode].SecondaryParentRssi = NI_RSSI_NOT_ASSIGNED;
         MM_NI_UpdateSecondaryParentDropped( ChildNode, SourceNode );
         result = true;
#ifdef ENABLE_VERBOSE_CIE_MESH_UPDATES
         MM_NI_SendMeshUpdateToCIE( NI_MESH_LINK_DROPPED_E, SourceNode, ChildNode, SourceNode );
#endif
      }
      else 
      {
         CO_PRINT_B_4(DBG_ERROR_E,"Cache Update: Node %d, Dropped Parent mismatched ID msgParent %d, cache PP %d SP %d\r\n", ChildNode, SourceNode, neighbourArray[ChildNode].PrimaryParentID, neighbourArray[ChildNode].SecondaryParentID );
      }
   }
   /* check to see if the child has dropped out of the mesh (i.e. no parents left) */
   if ( (NI_ID_NOT_ASSIGNED == neighbourArray[ChildNode].PrimaryParentID) &&
        (NI_ID_NOT_ASSIGNED == neighbourArray[ChildNode].SecondaryParentID) )
   {
      CO_PRINT_B_1(DBG_INFO_E,"Cache Update: Node %d, Dropped from mesh\r\n", ChildNode );
      MM_NI_SendMeshUpdateToCIE(NI_MESH_NODE_LOG_OFF_E, ChildNode, 0, ChildNode);
      MM_NI_PurgeNodeTree( ChildNode );
   }
   return result;
}

/*************************************************************************************/
/**
* MM_NI_UpdateChildAdded
* Update the child node to add the source node as a parent.
*
* param - SourceNode   The parent node.
* param - ChildNode    The child that was added.
*
* return - bool        True if the update succeeds
*/
bool MM_NI_UpdateChildAdded( const uint16_t SourceNode, const uint16_t ChildNode )
{
   bool result = false;
   
   if ( (MAX_DEVICES_PER_SYSTEM > SourceNode) && (MAX_DEVICES_PER_SYSTEM > ChildNode) )
   {
      CO_PRINT_B_2(DBG_INFO_E,"Cache Update: Parent Node %d added child %d\r\n", SourceNode, ChildNode );
      //the NCU is a soecial case because we know it can only be the primary parent
      if ( ADDRESS_NCU == SourceNode )
      {
         //Don't overwrite if the child already has the NCU as a parent or we will lose the actual SNR value
         if ( ADDRESS_NCU != neighbourArray[ChildNode].PrimaryParentID )
         {
            neighbourArray[ChildNode].PrimaryParentID = ADDRESS_NCU;
            neighbourArray[ChildNode].PrimaryParentRssi = NI_DEFAULT_RSSI_VALUE;
         }
         else 
         {
            //Reset the last event values so that they don't persist from a previous connection.
            neighbourArray[ChildNode].LastMeshEvent = NI_MESH_NODE_LOG_OFF_E;
            neighbourArray[ChildNode].LastMeshEventParentID = NI_ID_NOT_ASSIGNED;
            neighbourArray[ChildNode].LastMeshEventChildID = NI_ID_NOT_ASSIGNED;
         }
      }

      result = true;
      MM_NI_SendMeshUpdateToCIE( NI_MESH_LINK_ADDED_E, SourceNode, ChildNode, SourceNode );
   }

   return result;
}

/*************************************************************************************/
/**
* MM_NI_PurgeNodeTree
* Called when it has been detrmined that a node has left the mesh.  This function
* searches the cache and removes all links to the dropped node.
* RECURSIVE FUNCTION.  This function calls MM_NI_UpdatePrimaryParentDropped which
* may call this function again if the call results in an orphaned child.
*
* param - lostNode    The node that left the mesh.
*
* return - bool       True if the update succeeds
*/
void MM_NI_PurgeNodeTree( const uint16_t lostNode )
{
   for ( uint16_t child = 0; child < MAX_DEVICES_PER_SYSTEM; child++ )
   {
      if ( neighbourArray[child].PrimaryParentID == lostNode )
      {
         neighbourArray[child].PrimaryParentID = NI_ID_NOT_ASSIGNED;
         neighbourArray[child].PrimaryParentRssi = NI_RSSI_NOT_ASSIGNED;
         //This function will promote the secondary parent if there is one.
         //If not, it recursively calls this function to purge the child.
         MM_NI_UpdatePrimaryParentDropped( child, lostNode, true );
      }
      else if ( neighbourArray[child].SecondaryParentID == lostNode )
      {
         neighbourArray[child].SecondaryParentID = NI_ID_NOT_ASSIGNED;
         MM_NI_UpdateSecondaryParentDropped( child, lostNode );
      }
   }
   
   //Clean out the old device record in case it rejoins.
   if ( MAX_DEVICES_PER_SYSTEM > lostNode )
   {
      neighbourArray[lostNode].PrimaryParentID = NI_ID_NOT_ASSIGNED;
      neighbourArray[lostNode].PrimaryParentRssi = NI_RSSI_NOT_ASSIGNED;
      neighbourArray[lostNode].SecondaryParentID = NI_ID_NOT_ASSIGNED;
      neighbourArray[lostNode].SecondaryParentRssi = NI_RSSI_NOT_ASSIGNED;
      neighbourArray[lostNode].LastMeshEvent = NI_MESH_NODE_LOG_OFF_E;
      neighbourArray[lostNode].LastMeshEventParentID = NI_ID_NOT_ASSIGNED;
      neighbourArray[lostNode].LastMeshEventChildID = NI_ID_NOT_ASSIGNED;
      neighbourArray[lostNode].backupBattery_mv = 0;
      neighbourArray[lostNode].DeviceCombination = 0;
      neighbourArray[lostNode].FailedResponseCount = 0;
      neighbourArray[lostNode].LastStatusEvent = 0;
      neighbourArray[lostNode].LastStatusEventNodeId = 0;
      neighbourArray[lostNode].LostHeartbeatCount = 0;
      neighbourArray[lostNode].LostHeartbeatPingCount = 0;
      neighbourArray[lostNode].LostHeartbeatPingInterval = 0;
      neighbourArray[lostNode].NodeID = CO_BAD_NODE_ID;
      neighbourArray[lostNode].primaryBattery_mv = 0;
      neighbourArray[lostNode].Rank = UNASSIGNED_RANK;
      neighbourArray[lostNode].RSSI = 0;
      neighbourArray[lostNode].SNR = 0;
      neighbourArray[lostNode].state = 0;
      neighbourArray[lostNode].ZoneNumber = 0;
      neighbourArray[lostNode].NumberOfChildren = 0;
      neighbourArray[lostNode].FaultStatus = 0;
   }
}

/*************************************************************************************/
/**
* MM_NI_SendMeshUpdateToCIE
* One of the parent/child links on the mesh has changed.  Send an event
* message to the CIE using the MISC queue.
*
* param - event      The network link change event.
* param - parentID   The parent node for the changed link.
* param - childID    The child node for the changed link.
* param = sourceNode The originating node of the mesh message
*
* return - void
*/
static void MM_NI_SendMeshUpdateToCIE( const CIEMeshEvent_t event, const uint16_t parentID, const uint16_t childID, const uint16_t sourceNode )
{
   static CIEMeshEvent_t last_event = NI_MESH_NODE_LOG_OFF_E;
   static uint16_t last_parent_id = CO_BAD_NODE_ID;
   static uint16_t last_child_id = CO_BAD_NODE_ID;
   CieBuffer_t message;
   
   //Filter-out duplicate messages
   if ( (event != last_event) ||
        (parentID != last_parent_id) ||
        (childID != last_child_id) )
   {
      //filter out duplicate messages for a specific node
      if ( neighbourArray[sourceNode].LastMeshEvent != event ||
           neighbourArray[sourceNode].LastMeshEventParentID != parentID ||
           neighbourArray[sourceNode].LastMeshEventChildID != childID )
      {

         snprintf((char*)&message.Buffer,sizeof(CieBuffer_t),"MESH:%d,%d,%d\r\n", (int32_t)event, parentID, childID );

#ifndef SEND_MESH_LINK_UPDATES
         //with the above defined, we only send MESH 2 messages (LOG OFF)
         if ( NI_MESH_NODE_LOG_OFF_E == event )
         {
#endif
         ErrorCode_t msg_status = MM_CIEQ_Push(CIE_Q_MISC_E, &message);
         if ( SUCCESS_E == msg_status )
         {
            neighbourArray[sourceNode].LastMeshEvent = event;
            neighbourArray[sourceNode].LastMeshEventParentID = parentID;
            neighbourArray[sourceNode].LastMeshEventChildID = childID;
            last_event = event;
            last_parent_id = parentID;
            last_child_id = childID;
         }
         else 
         {
            CO_PRINT_A_4(DBG_ERROR_E,"Mesh update to CIE FAILED. event=%d, parent=%d, child=%d, error=%d", (int32_t)event, parentID, childID, msg_status);
         }
#ifndef SEND_MESH_LINK_UPDATES
         }
#endif
      }
      else 
      {
         CO_PRINT_B_4(DBG_INFO_E,"MM_NI_SendMeshUpdateToCIE node %d filtered out repeat MESH:%d,%d,%d\r\n", sourceNode, (int32_t)event, parentID, childID);
      }
   }
   else 
   {
      CO_PRINT_B_3(DBG_INFO_E,"MM_NI_SendMeshUpdateToCIE filtered out consecutive MESH:%d,%d,%d\r\n",(int32_t)event, parentID, childID);
   }
}

/*************************************************************************************/
/**
* MM_NI_DumpNeighbourInfo
* Send the info for a range of nodes to the debug output.
*
* param - firstNode      The first node of the range.
* param - lastNode       The last node of the range.
*
* return - void
*/
void MM_NI_DumpNeighbourInformation(const uint32_t firstNode, const uint32_t lastNode )
{
   if ( MAX_DEVICES_PER_SYSTEM > firstNode )
   {
      char msg[128];
      uint32_t node = firstNode;
      while ((node <= lastNode) && (MAX_DEVICES_PER_SYSTEM > node))
      {
         sprintf(msg, "Node=%d, PP=%d, RSSI=%d, SP=%d, RSSI=%d, PTrk=%d, STrk=%d, primBat=%d, bkpBat=%d\r\n", node, 
                     neighbourArray[node].PrimaryParentID, neighbourArray[node].PrimaryParentRssi,
                     neighbourArray[node].SecondaryParentID, neighbourArray[node].SecondaryParentRssi,
                     neighbourArray[node].PrimaryTrackingNodeID, neighbourArray[node].SecondaryTrackingNodeID,
                     neighbourArray[node].primaryBattery_mv, neighbourArray[node].backupBattery_mv );
         
         CO_PRINT_A_1(DBG_INFO_E,"%S", msg);
         node++;
         osDelay(10);
      }
   }
}

/*************************************************************************************/
/**
* MM_NI_SetFaultStatus
* Set/reset the 'any faults' flag for a node.
*
* param - node       The node ID.
* param - hasFault   True if the node has any faults.
*
* return - void
*/
void MM_NI_SetFaultStatus(const uint32_t node, const uint8_t faultStatus )
{
   if ( MAX_DEVICES_PER_SYSTEM > node )
   {
      neighbourArray[node].FaultStatus = faultStatus;
   }
}
