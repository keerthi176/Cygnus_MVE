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
*  File        : MC_SyncPrivate.h
*
*  Description : Private header file for MC_SyncAlgorithm.c
*                This file should only be inlcuded by MC_SyncAlgorithm.c,
*                and by its test harness MC_SyncTestHarness.
*                The file MC_SyncPublic.h should be included before this one.
*
*************************************************************************************/

#ifndef MC_SYNC_PRIVATE_H
#define MC_SYNC_PRIVATE_H

/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"

/* Macros
*************************************************************************************/
//#define SUM_WRAP(x1, x2, mod)      (x1 + x2 < mod ? x1 + x2 : (x1 + x2) - mod)  // Returns (x1 + x2) with restricted-range wrapping
//#define SUBTRACT_WRAP(x1, x2, mod) (x1 >= x2      ? x1 - x2 : (mod + x1) - x2)  // Returns (x1 - x2) with restricted-range wrapping
//#define ABS_DIFFERENCE(x1, x2)     (x1 >= x2      ? x1 - x2 : x2 - x1)

#define SYNCH_LOCK_TOLERANCE 35U //2ms in LPTIM ticks

#endif // MC_SYNC_PRIVATE_H
