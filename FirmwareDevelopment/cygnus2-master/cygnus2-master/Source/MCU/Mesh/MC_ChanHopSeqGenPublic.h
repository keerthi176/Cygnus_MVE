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
*  File        : MC_ChanHopSeqGenPublic.h
*
*  Description : Public header file MC_ChanHopSeqGen.c
*
*************************************************************************************/

#ifndef MC_CHAN_HOP_SEQ_GEN_PUBLIC_H
#define MC_CHAN_HOP_SEQ_GEN_PUBLIC_H

/* System Include Files
*************************************************************************************/
#include "stdbool.h"


/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"
#include "CO_Defines.h"


/* Public Constants
*************************************************************************************/

/* Macros
*************************************************************************************/

/* Public Enumerations
*************************************************************************************/

/* Public Structures
*************************************************************************************/
typedef struct {
   uint8_t   NumBytes;
   uint8_t * pSeedByteArray;
} PRBSGeneratorSeed_t;

/* Public Typedefs
*************************************************************************************/
// Define the data type for the channel hopping channels.
// uint16_t gives tidier display in the Visual Studio watch window, but uint8_t is more memory efficient.
typedef uint8_t Channel_t;


/* Public Functions Prototypes
*************************************************************************************/
extern ErrorCode_t MC_GenChanHopSeq(
   Channel_t * const pChanHopSeqArray,
   const uint16_t SeqLength,
   const Channel_t NumChans,
   const uint16_t MinChanInterval);

extern ErrorCode_t MC_GetRandNumber(
   uint16_t * const pRandomNumber,
   const PRBSGeneratorSeed_t * const pPRBSGeneratorSeed);

extern ErrorCode_t MC_SelectInitialChannel(
   Channel_t * const pChanHopSeqArray,
   const uint16_t SeqLength,
   const Channel_t NumChans,
   uint16_t * const pInitialSearchChannel);

extern ErrorCode_t MC_RotateHoppingSequence(
   Channel_t * const pChanHopSeqArray,
   const uint16_t SeqLength,
   const Channel_t NumChans,
   uint16_t const CurrentBestChannel,
   uint16_t const NewBestChannel);

#endif // MC_CHAN_HOP_SEQ_GEN_PUBLIC_H
