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
*  File        : MC_SyncPublic.h
*
*  Description : Public header file MC_SyncAlgorithm.c
*
*************************************************************************************/

#ifndef MC_SYNC_PUBLIC_H
#define MC_SYNC_PUBLIC_H

/* System Include Files
*************************************************************************************/
#include <stdbool.h>
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "CO_ErrorCode.h"
#include "MC_MAC.h"

/* Public Constants
*************************************************************************************/
/* Macros
*************************************************************************************/

/* Public Enumerations
*************************************************************************************/
/* Public Structures
*************************************************************************************/

/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t MC_AddTrackingNode(const uint16_t SlotIndex);

ErrorCode_t MC_DeleteTrackingNode(const uint16_t SlotIndex);

bool MC_IsTrackingNode(const uint16_t SlotIndex);

ErrorCode_t MC_ProcessSyncMessage(const uint16_t SlotIndex, const uint16_t McuCtr,
                  const bool SyncTimestampValid, bool * const pFreqLockAchieved);

ErrorCode_t MC_ResetSyncDatabase(void);

   
   
#ifdef USE_FIXED_ACTIVE_FRAME_LENGTH
ErrorCode_t MC_UpdateToActiveFrameLength(const MC_MAC_TDMIndex_t* pCurrentTDM);
ErrorCode_t MC_UpdateTrackingNodeForActiveFrameLength(const uint32_t TrackingNodeIndex, const MC_MAC_TDMIndex_t* pCurrentTDM);
#endif
#endif // MC_SYNC_PUBLIC_H
