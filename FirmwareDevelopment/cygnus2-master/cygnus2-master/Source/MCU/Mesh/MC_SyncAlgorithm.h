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
***************************************************************************************
*  File        : MC_SyncAlgorithm.h
*
*  Description : Public header file MC_SyncAlgorithm.c
*
*************************************************************************************/
/* System Include Files
*************************************************************************************/
#include <stdbool.h>
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "CO_ErrorCode.h"
#include "MC_MeshFormAndHeal.h"

#ifndef MC_SYNC_ALGORITHM_H
#define MC_SYNC_ALGORITHM_H

/* Public Constants
*************************************************************************************/
#define SYNCH_LOCK_TOLERANCE 35U //2ms in LPTIM ticks
#define MFH_ROLL_OVER_THRESHOLD 100
#define MAX_SYNC_LOSS_COUNT 8U

/* Macros
*************************************************************************************/

/* Public Enumerations
*************************************************************************************/
/* Public Structures
*************************************************************************************/
/* Public Functions Prototypes
*************************************************************************************/
void MC_SYNC_Initialise(const uint16_t nodeID);
ErrorCode_t MC_SYNC_SetSyncNode(const uint16_t nodeID);
uint16_t MC_SYNC_GetSyncNode(void);
uint16_t MC_SYNC_GetPreviousSyncNode( ShortListElement_t* pNode );
int32_t MC_SYNC_UpdateSync(ShortListElement_t* pSyncNode, const uint32_t timeStamp);
bool MC_SYNC_SyncLocked(void);

#endif // MC_SYNC_ALGORITHM_H
