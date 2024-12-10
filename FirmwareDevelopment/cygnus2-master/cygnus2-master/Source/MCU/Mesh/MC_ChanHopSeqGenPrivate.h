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
*  File        : MC_ChanHopSeqGenPrivate.h
*
*  Description : Private header file for MC_ChanHopSeqGen.c
*                This file should only be inlcuded by MC_ChanHopSeqGen.c,
*                and by its test harness STChanHopSeqGenTest.c
*                The file MC_ChanHopSeqGenPublic.h should be included before this one.
*
*************************************************************************************/

#ifndef MC_CHAN_HOP_SEQ_GEN_PRIVATE_H
#define MC_CHAN_HOP_SEQ_GEN_PRIVATE_H

/* System Include Files
*************************************************************************************/

/* User Include Files
*************************************************************************************/

/* Private Constants
*************************************************************************************/
#define MAX_SEQ_END_LENGTH   (16)  // The largest recorded index for this array has been 13, so sizing to 16 is sufficient.
#define MAX_ITERATIONS      (100)  // Maximum number of iterations for the hopping sequence generator.
#define CHAN_WEIGHTS_OFFSET   (2)  // A common offset value used when weighting the likelihood that a given channel will be selected.
#define LONG_SEQ_MIN_LENGTH  (68)  // Used to decide when to apply more agressive channel selection balancing criteria.




/* Private Structures
*************************************************************************************/
typedef struct {
   bool aChanAvailableFlags[NUM_HOPPING_CHANS];
} Channels_t;

/* Private Enumerations
*************************************************************************************/
typedef enum {
   OR_THE_FLAGS_E,
   AND_THE_FLAGS_E
} BoolAlgebraType_t;


/* Private Functions Prototypes
*************************************************************************************/
static ErrorCode_t GenChanHopSeq(
   Channel_t * const pChanHopSeqArray,
   const uint16_t SeqLength,
   const Channel_t NumChans,
   const uint16_t MinChanInterval);
   
static ErrorCode_t GenSeqEndCandidates(
   const Channel_t FirstChanInSeq,
   const Channel_t SecondChanInSeq,
   const uint16_t MinChanInterval,
   int16_t * const pNumEndOfSeqChans,
   Channels_t * const pSeqEndCandidates);

static inline void DecrementChannelWeight(
   uint16_t * const pChannelWeight);

static ErrorCode_t GetPermittedChans(
   const Channel_t FirstAdjChan,
   const Channel_t SecondAdjChan,
   const uint16_t MinChanInterval,
   const BoolAlgebraType_t BoolAlgebraOption,
   bool * pPermittedChanFlags,
   uint16_t * const pNumPermittedChans,
   Channel_t * const pPermittedChanIndices);

static ErrorCode_t TestChanHopSeq(
   Channel_t * pChanHopSeqArray,
   uint16_t   SeqLength,
   Channel_t NumChans,
   uint16_t MinChanInterval);


#endif // MC_CHAN_HOP_SEQ_GEN_PRIVATE_H
