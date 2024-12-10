/**************************************************************************************
*  Confidential between PA Consulting and Bull Products Ltd.
***************************************************************************************
*  Cygnus 2
*  Copyright 2017 PA Consulting Group. All rights reserved.
*  Cambridge Technology Centre
*  Melbourn Royston
*  Herts SG8 6DP UK
*  Tel: <44> 1763 261222
*
***************************************************************************************
*  File        : MC_SyncTestHarness.c
*
*  Description : Source code for the Mesh Comms Synchronisation Algorithm.
*                Described in HKD-17-0046-D Phy Layer Synchronisation.doc
*
*************************************************************************************/

/* System Include Files
*************************************************************************************/
#include <string.h>         // Required for memset()
#include <inttypes.h>       // Required for C99 fixed size integers.


/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "MC_TDM.h"
#include "MC_SyncAlgorithm.h"

/* Macros
*************************************************************************************/
#define SYNCH_CORRECTION_THRESHOLD 2

/* Private Functions Prototypes
*************************************************************************************/

/* Global Variables
*************************************************************************************/
#ifdef ENABLE_TDM_CALIBRATION
extern int16_t gDchOffset;
#endif

bool gFreqLockAchieved = false;
/* Private Variables
*************************************************************************************/
static uint16_t gSyncNode = CO_BAD_NODE_ID;
ShortListElement_t gPreviousSyncNode;
//static uint16_t gLostSyncCount = 0;

/* Public Functions Definitions
*************************************************************************************/

/* Test Features
*************************************************************************************/

/*************************************************************************************/
/**
* MC_SYNC_Initialise
* Initialise the sync module.
*
* @param - nodeID : Node ID of the sync node.
*
* @return - SUCCESS_E or error if the node is not viable
*/
void MC_SYNC_Initialise(const uint16_t nodeID)
{
   /*Do Nothing*/
}

/*************************************************************************************/
/**
* MC_SYNC_SyncLocked
* Initialise the sync module.
*
* @param - None.
*
* @return - bool : True if we have sync lock with the sync node
*/
bool MC_SYNC_SyncLocked(void)
{
   return gFreqLockAchieved;
}

/*************************************************************************************/
/**
* MC_SYNC_SetSyncNode
* Assign a node to be the current sync node.
*
* @param - nodeID : Node ID of the sync node.
*
* @return - SUCCESS_E or error if the node is not viable
*/
ErrorCode_t MC_SYNC_SetSyncNode(const uint16_t nodeID)
{
   ErrorCode_t result = ERR_NOT_FOUND_E;
   ShortListElement_t SyncNode;
   
   CO_PRINT_B_1(DBG_INFO_E,"MC_SYNC_SetSyncNode+ node %d\r\n", nodeID);
   
   //Preserve the old synch node for use in selecting a new one
   if ( SUCCESS_E == MC_MFH_FindNode( gSyncNode, &gPreviousSyncNode ) )
   {
      CO_PRINT_B_1(DBG_INFO_E,"Previous sync node %d recorded\r\n", gSyncNode);
   }
   else
   {
      CO_PRINT_B_1(DBG_INFO_E,"Failed to record previous sync node %d\r\n", gSyncNode);
   }
   
   if ( CO_BAD_NODE_ID == nodeID )
   {
      CO_PRINT_B_1(DBG_INFO_E,"SYNC node %d removed\r\n", gSyncNode);
      gSyncNode = CO_BAD_NODE_ID;
   }
   else 
   {
      //Get the node's info
      if( SUCCESS_E == MC_MFH_FindNode( nodeID, &SyncNode ) )
      {
         //Handle valid timestamp
         if ( NO_TIMESTAMP_RECORDED != SyncNode.LastTimestamp )
         {
            gSyncNode = nodeID;
            result = SUCCESS_E;
            CO_PRINT_B_2(DBG_INFO_E,"SYNC New sync node %d, offset=%d\r\n", nodeID, SyncNode.LongFrameOffset);
         }
         else 
         {
            CO_PRINT_B_1(DBG_INFO_E,"SYNC node %d rejected. No timestamp\r\n", nodeID);
            result = ERR_NO_SYNC_LOCK_E;
         }
      }
      else
      {
         CO_PRINT_B_1(DBG_INFO_E,"SYNC node %d not set. Failed to find record\r\n", nodeID);
      }
   }
   return result;
}


/*************************************************************************************/
/**
* MC_SYNC_GetSyncNode
* Return the node ID of the current sync node.
*
* @param - None.
*
* @return - Node ID
*/
uint16_t MC_SYNC_GetSyncNode(void)
{
   return gSyncNode;
}

/*************************************************************************************/
/**
* MC_SYNC_GetPreviousSyncNode
* Return the record of the previous sync node.  Used in 'mesh form and heal' for
* reselecting sync node.
*
* @param - pNode        [OUT]struct to hold previous sync node record.
*
* @return - uint16_t    Node ID of previous sync node
*/
uint16_t MC_SYNC_GetPreviousSyncNode( ShortListElement_t* pNode )
{
   if( pNode )
   {
      memcpy(pNode, &gPreviousSyncNode, sizeof(ShortListElement_t));
   }
   
   return gPreviousSyncNode.NodeID;
}

/*************************************************************************************/
/**
* MC_SYNC_UpdateSync
* Update the time stamp of the sync node and perform sync checks.
*
* @param - timeStamp : The time stamp for the received heartbeat.
*
* @return - int32_t    the sync offset that was applied to the TDM..
*/
int32_t MC_SYNC_UpdateSync(ShortListElement_t* pSyncNode, const uint32_t timeStamp)
{
   static int32_t last_frame_sync = 0;
   int32_t sync_correction = 0;
   uint16_t time_stamp = (uint16_t)timeStamp;
   int16_t drift_compensation = 0;
   bool rolled_over = false;
   
   if ( pSyncNode && (pSyncNode->NodeID == gSyncNode) )
   {
      //Handle valid timestamp
      if ( NO_TIMESTAMP_RECORDED != pSyncNode->LastTimestamp )
      {
         //check for synch lock
         int32_t mod_ave = pSyncNode->AveragePeriod % LPTIM_MAX;
         uint16_t expected_timestamp = (uint16_t)SUM_WRAP(pSyncNode->LastTimestamp, mod_ave, LPTIM_MAX);
         
         if ( NO_TIMESTAMP_RECORDED == timeStamp )
         {
            time_stamp = expected_timestamp;
         }

         //Calculate the deviation of the timestamp from the expected value
         uint32_t delta = ABS_DIFFERENCE(time_stamp, expected_timestamp);
         //check for roll-over
         if ( LPTIM_HALF <= delta )
         {
            delta = LPTIM_MAX - delta;
            rolled_over = true;
         }
            
         //drift compensation.
         //The sync correction aligns the parent and child slots, then the child drifts away throughout the long frame.
         //Move the sync correction by an extra tick so the child drifts across the parent sync, rather than away.
         if ( rolled_over )
         {
            drift_compensation = 1;
         }
         else 
         {
            if ( time_stamp > expected_timestamp )
            {
               drift_compensation = 1;
            }
            else if ( expected_timestamp > time_stamp )
            {
               drift_compensation = -1;
            }
         }

#ifdef ENABLE_TDM_CALIBRATION
         int32_t sync_node_offset = pSyncNode->LongFrameOffset + gDchOffset;
#else
         int32_t sync_node_offset = pSyncNode->LongFrameOffset + TDM_DCH_MESSAGE_RXDONE_OFFSET;
#endif
         
#ifdef DEFER_SYNCH_TO_NEXT_LONG_FRAME
         int32_t ticks_to_next_longframe = pSyncNode->AveragePeriod - sync_node_offset + drift_compensation;
         uint16_t long_frame_sync = SUM_WRAP(time_stamp, ticks_to_next_longframe, LPTIM_MAX);
#else
         int32_t ticks_since_start_of_longframe = sync_node_offset + drift_compensation;
         uint32_t ticks_since_LF_mod = ticks_since_start_of_longframe % LPTIM_MAX;
         uint16_t long_frame_sync = SUBTRACT_WRAP(time_stamp, ticks_since_LF_mod, LPTIM_MAX);
#endif
         
         //CO_PRINT_B_4(DBG_INFO_E,"lt=%d,ave=%d,offset=%d, tolf=%d\r\n", pSyncNode->LastTimestamp, pSyncNode->AveragePeriod, pSyncNode->LongFrameOffset, ticks_to_next_longframe);
         
         //CO_PRINT_B_4(DBG_INFO_E,"timeStamp=%d, expected=%d, nextLFStart=%d, sync_node_offset=%d\r\n", time_stamp, expected_timestamp, next_long_frame_sync, sync_node_offset);
         
         if ( NO_TIMESTAMP_RECORDED != timeStamp )
         {
            MC_TDM_SetSynchReferenceTime(long_frame_sync, pSyncNode->AveragePeriod);
            
#ifndef DEFER_SYNCH_TO_NEXT_LONG_FRAME
            if ( SYNCH_CORRECTION_THRESHOLD < pSyncNode->FrameCount )
            {
               sync_correction = last_frame_sync - ticks_since_start_of_longframe;
            }
            
            last_frame_sync = ticks_since_start_of_longframe;
#endif
            if ( SYNCH_LOCK_TOLERANCE > delta )
            {
               CO_PRINT_B_3(DBG_INFO_E,"Sync Heartbeat Rx - node %d Rx=%d expected=%d\r\n", gSyncNode, time_stamp, expected_timestamp);
               gFreqLockAchieved = true;
            }
         }
         else 
         {
            //We've missed a sync node heartbeat.  If frequency lock hasn't been achieved we have failed to sync with the mesh.
            if ( !gFreqLockAchieved )
            {
               //Error_Handler("Failed to achieve lock with Sync node");
               MC_SMGR_DelayedReset("Failed to achieve lock with Sync node");
            }
         }
      }
   }
   
   return sync_correction;
}

