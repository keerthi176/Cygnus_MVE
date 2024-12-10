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
*  File        : MC_ChanHopSeqGen.c
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
#include "MC_ChanHopSeqGenPublic.h"
#include "MC_ChanHopSeqGenPrivate.h"
#include "CO_Defines.h"

/* Private Functions Prototypes
*************************************************************************************/
// These are defined in MC_ChanHopSeqGenPrivate.h


/* Global Variables
*************************************************************************************/
/* Private Variables
*************************************************************************************/
/* Macros
*************************************************************************************/
//#define SYNC_DEBUG        // Enable this to turn on debug printing.
#ifdef SYNC_DEBUG
# define DEBUG_PRINT(x)      CO_PRINT_B_0(x)
# define DEBUG_PRINT_1(x, y) CO_PRINT_B_0(x, y)
#else
# define DEBUG_PRINT(x)      // Do nothing
# define DEBUG_PRINT_1(x, y) // Do nothing
#endif


/* Public Functions Definitions
*************************************************************************************/

/*************************************************************************************/
/**
* function name : MC_GenCHanHopSeq
* description   : Generates a channel hopping sequence and stores it in the array
*                 accessed via the pointer passed to the function.  This function calls
*                 the function GenChanHopSeq() repeatedly to generate a candidate sequence, then checks 
*                 the relative frequency with which the different channels are used in the sequence 
*                 (MaxUse / MinUse).
*                 If the ratio is excessive (signifying a big imbalance in the channel selections)
*                 then the process is repeated.  Tests found a maximum of 51 iterations was needed:
*                 the loop is therefore set to terminate after 100 iterations and to report an error code.
*
* INPUTS
* @param - Channel_t * const pChanHopSeqArray : Pointer to the array to contain the hopping sequence.
* @param - const uint16_t SeqLength : Number of hops in the whole sequence (after which, sequence should be repeated).
* @param - const Channel_t NumChans : Number of hopping channels.  Should not exceed the s/w constant NUM_HOPPING_CHANS.
* @param - const uint16_t MinChanInterval : Minimum number of channels to hop (e.g. EN54 specifies minimum of 1.0 MHz)
*          If channel spacing is not constant, specify the worst case interval.
*
* @return - Type is ErrorCode_t.  Returns SUCCESS_E (0) if the function succeeds, else
*           returns one of the other values of ErrorCode_t to convey the reason for
*           the error or warning.
*/
extern ErrorCode_t MC_GenChanHopSeq(
   Channel_t * const pChanHopSeqArray,
   const uint16_t SeqLength,
   const Channel_t NumChans,
   const uint16_t MinChanInterval)
{
   ErrorCode_t ErrorCode = SUCCESS_E;
   uint16_t MaxCount = 1;     // Initialised to 1 so that while loop runs first time - see below.
   uint16_t MinCount = 0;     // Initialised to 0 so that while loop runs first time - see below.
   uint16_t aChannelCount[NUM_HOPPING_CHANS];
   uint16_t Num = 0;
   uint16_t Den = 2;          // Initialised to 2 - see explanation as for "Num" in code below.
   uint16_t Iteration = 0;

   /////////////////////////
   // Range check the inputs
   /////////////////////////

   if (NULL == pChanHopSeqArray)
   {
      DEBUG_PRINT("ERROR - MC_GenChanHopSeq(). NULL pointer.\n");
      return ERR_INVALID_POINTER_E;
   }

   // PRBS generator has only been tested such that there are always three channels that can be hopped to.
   // This test ensure that the function is used in similar conditions.
   if (NumChans < (2 * MinChanInterval + 2) )
   {
      DEBUG_PRINT("ERROR - MC_GenChanHopSeq(). There must be at least three other channels that can be hopped to.\n");
      return ERR_TOO_FEW_CHANNEL_OPTIONS_E;
   }

   // Check that the specified number of channels doesn't exceed the defined array size.
   if (NUM_HOPPING_CHANS < NumChans)
   {
      DEBUG_PRINT("ERROR - MC_GenChanHopSeq(). The number of channels specified exceeds the s/w constant NUM_HOPPING_CHANS.\n");
      return ERR_TOO_MANY_CHANNEL_SPECIFIED_E;
   }

   if (16 > SeqLength)
   {
      DEBUG_PRINT("ERROR - MC_GenChanHopSeq(). The shortest hopping sequence that has been tested is 16.\n");
      return ERR_HOPPING_SEQUENCE_LENGTH_TOO_SHORT_E;
   }

   ////////////////////////////////////////
   // Generate the channel hopping sequence
   ////////////////////////////////////////

   // Decide whether to apply weak (for short sequences) or strong (for long sequences) channel 
   // selection balancing criteria.
   // Weak: Channel selection balancing must not exceed Num/Den = 4/2 = 2.0 ratio.
   // Strong: Channel selection balancing must not exceed Num/Den = 3/2 = 1.5 ratio.
   Num = 3 + (SeqLength < LONG_SEQ_MIN_LENGTH);

   // Repeat the generation until the ratio of MaxCount/MinCount falls to the specified maximum 
   // value, or lower.  Loop will break and generate an error code if the number of iterations 
   // exceeds MAX_ITERATIONS.
   while (MaxCount > ((Num * MinCount) / Den))
   {
      MaxCount = 0;      // Initial value to find a max.
      MinCount = 65535;  // Initial value to find a min.

      // Initilise the ChannelCount array to all zeros.
      memset(aChannelCount, 0, NUM_HOPPING_CHANS * sizeof(aChannelCount[0]));

      // Generate a hopping sequence
      ErrorCode = GenChanHopSeq(pChanHopSeqArray, SeqLength, NumChans, MinChanInterval);

      // Count the number of times each channel is used.
      for (uint16_t i = 0; i < SeqLength; i++)
      {
         aChannelCount[pChanHopSeqArray[i]]++;
      }

      // Find the max and min number of times each channel is used.
      // (The while loop runs until the ratio of these meets the channel selection balancing criteria.)
      for (uint16_t i = 0; i < NumChans; i++)
      {
         MaxCount = MAX(MaxCount, aChannelCount[i]);
         MinCount = MIN(MinCount, aChannelCount[i]);
      }

      ErrorCode = TestChanHopSeq(pChanHopSeqArray, SeqLength, NumChans, MinChanInterval);
      if (ErrorCode)
      {
         DEBUG_PRINT("\nCall to TestChanHopSeq() returned error code.");
         return ErrorCode;
      }

      // If the process is taking too long, terminate the while loop and set an error code.
      // (In tests, the code completed within 51 iterations when tested for 100,000 hopping sequences 
      // of each logical channel.)
      if (MAX_ITERATIONS < ++Iteration)
      {
         ErrorCode = ERR_HOP_SEQ_GEN_REACHED_MAX_ITERATIONS_E;
         break;
      }
   }

   // Return success
   return ErrorCode;
}


/*************************************************************************************/
/**
* function name : MC_GetRandNumber
* description   : Generates the next in a sequence of 65535 random 16-bit numbers, excluding 0.
*                 If called more than 65535 times, the series will repeat.
*                 If a seed is provided, it will additionally reset then seed the PRBS sequence
*                 before generating the first random number in that re-seeded sequence.
*
* INPUTS
* @param - uint16_t * const pRandomNumber : Pointer to the variable to contain the random number.
*          If this is set to NULL the function does not return the random number.
* @param - const PRBSGeneratorSeed_t * const pPRBSGeneratorSeed : Pointer to the seed structure.
*          If this is set to NULL the function will return the next random number in the series.
*          If it points to a valid byte array, and the length of the array is specified as non-zero,
*          the function will re-seed the random number generator and will generate the first number
*          in the series.
*
* @return - Type is ErrorCode_t.  Returns SUCCESS_E (0) if the function succeeds, else
*           returns one of the other values of ErrorCode_t to convey the reason for
*           the error or warning.
*/
extern ErrorCode_t MC_GetRandNumber(
   uint16_t * const pRandomNumber,
   const PRBSGeneratorSeed_t * const pPRBSGeneratorSeed)
{
#define LEN_PRB_SEQ (16)
   // These are all uint16_t because the PRBS is a 16-bit sequence.
   static uint16_t RegWord = 0;  // The 2-byte word in the shift register.
   uint16_t BitNo = 0;
   uint16_t ByteNo = 0;
   uint16_t SeedBit = 0;
   uint16_t Mask = (1 << LEN_PRB_SEQ) - 1;
   uint16_t NewLSB = 0;

   uint8_t * pByte = NULL;

   // If the input pointer is not NULL, seed the PRBS generator.
   if (pPRBSGeneratorSeed)
   {
      if (0 == pPRBSGeneratorSeed->NumBytes || NULL == pPRBSGeneratorSeed->pSeedByteArray)
      {
         DEBUG_PRINT("ERROR - MC_GetRandNumber(). The structure containing the PRBS seed value does not contain valid values.\n");
         return ERR_PRBS_SEED_NOT_SET_CORRECTLY_E;
      }

      // Initialise the register word to 1 before applying the seed.
      // 0x0000 is an invalid value for the RegWord, and would cause problems if the seed itself were all zeros.
      RegWord = 1;

      for (ByteNo = 0; ByteNo < pPRBSGeneratorSeed->NumBytes; ByteNo++)
      {
         // Set pointer to the next byte.
         pByte = &pPRBSGeneratorSeed->pSeedByteArray[ByteNo];

         for (BitNo = 0; BitNo < 8; BitNo++)
         {
            // Read the next bit from the seed byte
            SeedBit = *pByte & 1;

            // Bitshift the seed byte, to move the next bit to the LSB position.
            *pByte >>= 1;

            // Iterate the PRBS generator (from https://uk.mathworks.com/help/comm/ref/pnsequencegenerator.html)
            NewLSB = (((RegWord >> 15) ^ (RegWord >> 14) ^ (RegWord >> 12) ^ (RegWord >> 3) ^ SeedBit) & 1);
            RegWord = ((RegWord << 1) | NewLSB) & Mask;
         }
      }
   }
   else
   {
      if (0 == RegWord)
      {
         DEBUG_PRINT("ERROR - MC_GetRandNumber(). The PRBS random number generator was called without having first seeded it.\n");
         return ERR_RAND_NUM_GENERATOR_NOT_SEEDED_E;
      }

      for (BitNo = 0; BitNo < LEN_PRB_SEQ; BitNo++)
      {
         // Iterate the PRBS generator (from https://uk.mathworks.com/help/comm/ref/pnsequencegenerator.html)
         NewLSB = (((RegWord >> 15) ^ (RegWord >> 14) ^ (RegWord >> 12) ^ (RegWord >> 3)) & 1);
         RegWord = ((RegWord << 1) | NewLSB) & Mask;
      }
   }

   // Set the output value and return success.
   if (NULL != pRandomNumber)
   {
      *pRandomNumber = RegWord;
   }
   
   return SUCCESS_E;
}

/*************************************************************************************/
/**
* function name : MC_SelectInitialChannel
* description   : Scans the channel hopping sequence array to choose which channel is used twice
*                 and of those has the most equal spacing between usage.  E.g. a DCH channel might
*                 be used with intervals of 7 and 9 long frames, but it is better to use one with 
*                 intervals of 8 and 8 long frames as this minimises the maximum time to acquire sync.
*
* INPUTS
* @param - Channel_t * const pChanHopSeqArray : Pointer to the array to contain the hopping sequence.
* @param - const uint16_t SeqLength : Number of hops in the whole sequence (after which, sequence should be repeated).
* @param - const Channel_t NumChans : Number of hopping channels.  Should not exceed the s/w constant NUM_HOPPING_CHANS.
* @param - uint16_t * const pInitialSearchChannel : Pointer to uint16_t, to store the recommended initial search channel 
*          for the DCH.
*
* @return - Type is ErrorCode_t.  Returns SUCCESS_E (0) if the function succeeds, else
*           returns one of the other values of ErrorCode_t to convey the reason for
*           the error or warning.
*/
ErrorCode_t MC_SelectInitialChannel(
   Channel_t * const pChanHopSeqArray,
   const uint16_t SeqLength,
   const Channel_t NumChans,
   uint16_t * const pInitialSearchChannel)
{
   if (NULL == pChanHopSeqArray || NULL == pInitialSearchChannel)
   {
      DEBUG_PRINT("ERROR - MC_SelectInitialChannel(). NULL pointer.\n");
      return ERR_INVALID_POINTER_E;
   }
   
   const uint16_t halfSeqLength = (uint16_t)SeqLength/2;
   uint16_t position[2] = {0,0};      // Hard-coded 2, since we search for the first and second instance of each channel
   uint16_t instance = 0;             // Used to count instances of each channel found within pChanHopSeqArray.
   uint16_t index = 0;                // Used to track the index within pChanHopSeqArray
   uint16_t bestInterval = SeqLength; // We search for the shortest interval (approx SeqLength/2), so initialise this to a larger value.
   uint16_t bestChanIndex = 0;        // Used to store the index of the channel having the lowest value of bestInterval.
   
   // Loop over the different channel indices: we will consider them one at a time.
   for (uint16_t chanIdx = 0; chanIdx < NumChans; chanIdx++)
   {
      // Count the interval between the first and possibly second instances of this channel 
      // appearing the channel hopping sequence array.
      index = 0;
      instance = 0;
      
      // Populate the array position[] with the locations of the chanIdx in the array pChanHopSeqArray
      while (instance < 2 && index < SeqLength)
      {
         if (pChanHopSeqArray[index] == chanIdx)
         {
            position[instance++] = index;
         }
         index++;
      }
      
      // If fewer than 2 instances of the channel were found, move onto the next index
      if (2 > instance)
      {
         continue;
      }
      
      // Calculate the interval between the two positions of this channel index.
      uint16_t interval = position[1] - position[0];
      
      // If the measured interval is the shorter of the two (e.g. 7 instead of 9 for a sequence that is 16 long)
      // then convert it to the longer of the two.
      if (interval < halfSeqLength)
      {
         interval = SeqLength - interval;
      }
      
      // Record the shortest interval thus measured, and its index.
      // If some intervals are equal, it really doesn't matter which one we return.
      if (interval < bestInterval)
      {
         bestInterval = interval;
         bestChanIndex = chanIdx;
      }
   }
   
   // Set the return value
   *pInitialSearchChannel = bestChanIndex;
   
   return SUCCESS_E;
}

/*************************************************************************************/
/**
* function name : MC_RotateHoppingSequence
* description   : Performs a cyclic shift of the hopping sequence, so that the best
*                 channel of the sequence is rotated to a desired .
*
* INPUTS
* @param - Channel_t * const pChanHopSeqArray : Pointer to the array containing the hopping sequence.
* @param - const uint16_t SeqLength : Length of the hopping sequence.
* @param - const Channel_t NumChans : Number of hopping channels.  Should not exceed the s/w constant NUM_HOPPING_CHANS.
* @param - uint16_t const CurrentBestChannel : The current best channel of the hopping sequence.
* @param - uint16_t const CurrentBestChannel : The new (desired) best channel of the hopping sequence.
*
* @return - Type is ErrorCode_t.  Returns SUCCESS_E (0) if the function succeeds, else
*           returns one of the other values of ErrorCode_t to convey the reason for
*           the error or warning.
*/
ErrorCode_t MC_RotateHoppingSequence(
   Channel_t * const pChanHopSeqArray,
   const uint16_t SeqLength,
   const Channel_t NumChans,
   uint16_t const CurrentBestChannel,
   uint16_t const NewBestChannel)
{
   uint16_t Rotation = 0;
   
   // Don't waste time if no change needed
   if (CurrentBestChannel == NewBestChannel)
   {
      return SUCCESS_E;
   }
   
   // Find the number of channels rotation required.
   if (NewBestChannel > CurrentBestChannel)
   {
      Rotation = NewBestChannel - CurrentBestChannel;
   }
   else   
   {
      Rotation = (NewBestChannel + NumChans) - CurrentBestChannel;
   }
   
   // Rotate the channels
   for (Channel_t i = 0; i < SeqLength; i++)
   {
      if (pChanHopSeqArray[i] + Rotation < NumChans)
      {
         pChanHopSeqArray[i] += Rotation;
      }
      else
      {
         pChanHopSeqArray[i] -= (NumChans - Rotation);
      }
   }
   
   return SUCCESS_E;
}


/* Private Functions Definitions
*************************************************************************************/

/*************************************************************************************/
/**
* function name : GenChanHopSeq
* description   : Generates a channel hopping sequence and stores it in the array
*                 accessed via the pointer passed to the function.
*
* INPUTS
* @param - Channel_t * const pChanHopSeqArray : Pointer to the array to contain the hopping sequence.
* @param - const uint16_t SeqLength : Number of hops in the whole sequence (after which, sequence should be repeated).
* @param - const Channel_t NumChans : Number of hopping channels.  Should not exceed the s/w constant NUM_HOPPING_CHANS.
* @param - const uint16_t MinChanInterval : Minimum number of channels to hop (e.g. EN54 specifies minimum of 1.0 MHz)
*          If channel spacing is not constant, specify the worst case interval.
*
* @return - Type is ErrorCode_t.  Returns SUCCESS_E (0) if the function succeeds, else
*           returns one of the other values of ErrorCode_t to convey the reason for
*           the error or warning.
*/
static ErrorCode_t GenChanHopSeq(
   Channel_t * const pChanHopSeqArray,
   const uint16_t SeqLength,
   const Channel_t NumChans,
   const uint16_t MinChanInterval)
{
   ErrorCode_t ErrorCode = SUCCESS_E;
   int16_t     HopSeqIndex = 0;
   uint16_t     NumPermittedChans = 0;
   int16_t     NumSeqEndSteps = 0;
   uint16_t    SelectedChannel = 0;
   uint16_t    RandomNumber = 0;
   Channels_t  aSeqEndCandidates[MAX_SEQ_END_LENGTH];
   Channel_t   aPermittedChanIndices[NUM_HOPPING_CHANS];
   bool        aPermittedChanFlags[NUM_HOPPING_CHANS];
   uint16_t    aChannelWeights[NUM_HOPPING_CHANS];

   // Initialise arrays
   memset(aPermittedChanIndices, 0, NUM_HOPPING_CHANS * sizeof(aPermittedChanIndices[0]));
   memset(aPermittedChanFlags, true, NUM_HOPPING_CHANS * sizeof(aPermittedChanFlags[0]));
   memset(aSeqEndCandidates, false, NUM_HOPPING_CHANS * MAX_SEQ_END_LENGTH * sizeof(aSeqEndCandidates[0].aChanAvailableFlags[0]));
   
   // Initialise each of the channel weights array to the approx average number of occurrences expected.
   // Setting CHAN_WEIGHTS_OFFSET = 2 has been found empirically to give the most even distribution of channels from the set.)
   for (uint16_t i = 0; i < NUM_HOPPING_CHANS; i++)
   {
      aChannelWeights[i] = (SeqLength / NumChans) + CHAN_WEIGHTS_OFFSET;
   }

   // Choose a channel at random for the first hop, and decrement the weighting of that channel for future selections.
   ErrorCode = MC_GetRandNumber(&SelectedChannel, NULL);
   if (ErrorCode)
   {
      return ErrorCode;
   }
   SelectedChannel %= NumChans;
   pChanHopSeqArray[HopSeqIndex++] = (Channel_t)SelectedChannel;
   DecrementChannelWeight(&aChannelWeights[SelectedChannel]);

   // Get a list of permitted channel indices for the next hop.
   // The second adjacent channel is set to the same as the first adjacent channel to prevent prohibiting an additional channel.
   // Function only returns SUCCESS_E, so no error checking needed after function call.
   GetPermittedChans(
      pChanHopSeqArray[HopSeqIndex - 1], pChanHopSeqArray[HopSeqIndex - 1],
      MinChanInterval, AND_THE_FLAGS_E, NULL, &NumPermittedChans, aPermittedChanIndices);

   // Choose a channel at random for the second hop, and decrement the weighting of that channel for future selections.
   ErrorCode = MC_GetRandNumber(&RandomNumber, NULL);
   if (ErrorCode)
   {
      return ErrorCode;
   }
   RandomNumber %= NumPermittedChans;
   SelectedChannel = aPermittedChanIndices[RandomNumber];

   pChanHopSeqArray[HopSeqIndex++] = (Channel_t)SelectedChannel;
   DecrementChannelWeight(&aChannelWeights[SelectedChannel]);

   // Generate list of candidate channels for the sequence end.
   ErrorCode = GenSeqEndCandidates(pChanHopSeqArray[0], pChanHopSeqArray[1], MinChanInterval, &NumSeqEndSteps, aSeqEndCandidates);
   if (ErrorCode)
   {
      return ErrorCode;
   }

   // Choose the hopping channels for the remainder of the sequence.
   while (HopSeqIndex < SeqLength)
   {
      // Initialise arrays.
      // During the sequence end period, the flags are based on aChanAvailableFlags values, but before that all are set to true.
      for (int16_t i = 0; i < NUM_HOPPING_CHANS; i++)
      {
         // The SeqEndIndex counts down as the HopSeqIndex counts up towards the end of the sequence.
         // The final value of HopSeqIndex (SeqLength - 1) corresponds to SeqEndIndex value of 2.
         uint16_t SeqEndIndex = SeqLength + 1 - HopSeqIndex;
         bool IsBeforeSeqEndPeriod = HopSeqIndex < SeqLength - NumSeqEndSteps;

         // Initialise the arrays.
         aPermittedChanFlags[i] = aSeqEndCandidates[SeqEndIndex].aChanAvailableFlags[i] || IsBeforeSeqEndPeriod;
         aPermittedChanIndices[i] = 0;
      }

      // Get the permitted channels for the current hop, based on the channels used in the first (-1) and second (-2) adjacent hops.
      GetPermittedChans(pChanHopSeqArray[HopSeqIndex - 1], pChanHopSeqArray[HopSeqIndex - 2],
         MinChanInterval, AND_THE_FLAGS_E, aPermittedChanFlags, &NumPermittedChans, aPermittedChanIndices);

      // Test case that NumPermittedChans == 0 and if so return an error code.
      if (0 == NumPermittedChans)
      {
         return ERR_HOP_SEQ_GEN_HAS_NO_AVAILABLE_CHANNELS_E;
      }

      // Sum the weights of the permitted channels.
      uint16_t TotalNumberOfWeightedChannelOptions = 0;
      for (uint16_t i = 0; i < NumPermittedChans; i++)
      {
         TotalNumberOfWeightedChannelOptions += aChannelWeights[aPermittedChanIndices[i]];
      }

      // Generate a random number in the range 0 to TotalNumberOfWeightedChannelOptions - 1.
      ErrorCode = MC_GetRandNumber(&RandomNumber, NULL);
      if (ErrorCode)
      {
         return ErrorCode;
      }
      RandomNumber %= TotalNumberOfWeightedChannelOptions;

      // Select a channel from the permitted channel index list, such that weighted selection probabilities are applied.
      for (uint16_t i = 0; i < NumPermittedChans; i++)
      {
         uint16_t Chan = aPermittedChanIndices[i];
         if (RandomNumber < aChannelWeights[Chan])
         {
            SelectedChannel = Chan;
            break;  // Terminate the for loop when the channel has been selected.
         }
         RandomNumber -= aChannelWeights[Chan];
      }

      // Add the channel to the current hop in the sequence, and decrement the weighting of that channel for use in future selections.
      pChanHopSeqArray[HopSeqIndex++] = (Channel_t)SelectedChannel;
      DecrementChannelWeight(&aChannelWeights[SelectedChannel]);
   }

   // Return success
   return SUCCESS_E;
}


/*************************************************************************************/
/**
* function name : DecrementChannelWeight
* description   : Decrements the value of the variable passed to it via the input pointer,
*                 but never lower than 1.  In the context of a channel weight values, 
*                 this decreases the relative likelihood of this channel being chosen in future.
*                 The weight does not drop lower than 1, otherwise the channel would never be
*                 selected, and could result in all possible candidates becoming unavailable.
*
* INPUTS
* @param - uint16_t * const pChannelWeight : Pointer to the channel weight value to be decremented.
*
* @return - void.
*/
static inline void DecrementChannelWeight(
   uint16_t * const pChannelWeight)
{
   // Decrement value of the specified channel in the channel weights array.
   // This decreases the relative likelihood of this channel being chosen in future.
   // The weight does not drop lower than 1, otherwise the channel would never be selected,
   // and could result in all possible candidates becoming unavailable.
   *pChannelWeight -= (*pChannelWeight > 1);
}


/*************************************************************************************/
/**
* function name : GenSeqEndCandidates
* description   : Generates a restricted set of candidate channels, to restrict the channel
*                 selections when calling the function GetPermittedChans as the end of the
*                 hopping sequence is approached.
*                 This guarantees that the two rules applicable to the hopping sequence
*                 are retained when the sequence repeats.  Those rules are: a) the channel
*                 should change by at least a specified minimum number of channels (to ensure
*                 hops are >= 1MHz), and b) that the same channel is not used within three
*                 hops of each other (to prevent the hopping sequence from merely toggling
*                 between two channels for extended periods).
*                 This function should be called after the first two members of the hopping
*                 sequence have been calculated.
*                 See "HKD-17-0076-D Software Architecture and Design Specification", Section 3.3.2 
*                 for a full description of how this works.
*
* INPUTS
* @param - const Channel_t FirstChanInSeq : Index of first channel in hopping sequence.
* @param - const Channel_t SecondChanInSeq : Index of second channel in hopping sequence. 
* @param - const uint16_t MinChanInterval : Minimum interval between successive hopping channels.
*          (To ensure that the minimum frequency between successive hops is at least 1.0 MHz.)
* @param - int16_t * const pNumEndOfSeqChans : Returns the length of the end of sequence series.
*          (This lets the algorithm know when to apply the restricted candidate channel sets.)
* @param - Channels_t * const pSeqEndCandidates : Pointer to array that will hold the sequence-end 
*          candidate channels generated by this function.
*
* @return - Type is ErrorCode_t.  Returns SUCCESS_E (0) if the function succeeds, else
*           returns one of the other values of ErrorCode_t to convey the reason for
*           the error or warning.
*/
static ErrorCode_t GenSeqEndCandidates(
   const Channel_t FirstChanInSeq,
   const Channel_t SecondChanInSeq,
   const uint16_t MinChanInterval,
   int16_t * const pNumEndOfSeqChans,
   Channels_t * const pSeqEndCandidates)
{
   ErrorCode_t ErrorCode = SUCCESS_E;
   uint16_t    CurrentHopIndex = 0;
   uint16_t    FirstAdjHopIndex = 0;
   uint16_t    SecondAdjHopIndex = 0;
   uint16_t    LoopTerminationCounter = 0;
   uint16_t    NumPermittedChans = 0;
   Channel_t   aSecondCandidateChanIndices[NUM_HOPPING_CHANS];
   bool        aFirstCandidateChanFlags[NUM_HOPPING_CHANS];

   // Initialise the value of NumEndOfSeqChans
   *pNumEndOfSeqChans = 0;

   // General initialisation of TurnAroundChans array
   memset(pSeqEndCandidates, false, MAX_SEQ_END_LENGTH * NUM_HOPPING_CHANS * sizeof(bool));
   memset(aSecondCandidateChanIndices, 0, NUM_HOPPING_CHANS * sizeof(aSecondCandidateChanIndices[0]));

   // Mark the first two slots as available in the channels used for the first two hops.
   // Note: aSeqEndCandidates[] is indexed in the reverse time direction, with index 0 associated with 
   // the second member of the hopping sequence, and index 1 associated with the first member of the
   // hopping sequence.
   pSeqEndCandidates[0].aChanAvailableFlags[SecondChanInSeq] = true;
   pSeqEndCandidates[1].aChanAvailableFlags[FirstChanInSeq] = true;

   ///////////////////////////////////////////////////
   // Populate the remaining rows of aSeqEndCandidates
   ///////////////////////////////////////////////////

   // Note how this continues to populate the array in the reverse time direction, with index 2 
   // representing the last member of the hopping sequence, index 3 the second from last, etc.  

   // This is initialised to 2 because the first two rows of the array are initialised outside the loop.
   CurrentHopIndex = 2;

   // While loop runs until there have been two consecutive hops having all the channels permitted.
   while (LoopTerminationCounter < 2)
   {
      // Test that CurrentHopIndex has not exceed the array size.
      if (MAX_SEQ_END_LENGTH <= CurrentHopIndex)
      {
         return ERR_END_SEQ_EXCEEDS_ARRAY_SIZE_E;
      }

      // Set the indicies of the first and second adjacent hops.
      FirstAdjHopIndex  = CurrentHopIndex - 1;
      SecondAdjHopIndex = CurrentHopIndex - 2;

      ///////////////////////////////////////////////////////////////////////////
      // Generate first list of candidate channels.
      ///////////////////////////////////////////////////////////////////////////

      // If a channel is permitted by the < 1.0 MHz rule for any permitted 
      // channels at index (n+1) then it is a candidate channel for index (n).
      // The lists are ORd together for each possible First Adjacent Hop channel.

      memset(aFirstCandidateChanFlags, false, NUM_HOPPING_CHANS * sizeof(bool));
      for (uint8_t i = 0; i < NUM_HOPPING_CHANS; i++)
      {
         if (pSeqEndCandidates[FirstAdjHopIndex].aChanAvailableFlags[i])
         {
            // We set the second adjacent hop channel to the same value as the first adjacent hop 
            // channel (both set to i) to stop GetPermittedChans() from excluding an additional
            // second adjacent hop channel value.  (There are many possibilities for the second 
            // adjacent hop channel, so we can't consider them yet.)
            GetPermittedChans(i, i, MinChanInterval, OR_THE_FLAGS_E,
               aFirstCandidateChanFlags, &NumPermittedChans, NULL);
         }
      }

      ///////////////////////////////////////////////////////////////////////////////////////////////
      // Test candidate channels and remove any that don't comply with second adjacent channel rules.
      ///////////////////////////////////////////////////////////////////////////////////////////////

      // Loop over first adjacent channel indicies
      for (uint16_t FirstAdjHopChan = 0; FirstAdjHopChan < NUM_HOPPING_CHANS; FirstAdjHopChan++)
      {
         // Loop over second adjacent channel indicies
         for (uint16_t SecondAdjHopChan = 0; SecondAdjHopChan < NUM_HOPPING_CHANS; SecondAdjHopChan++)
         {
            // Find valid pairs of FirstAdjHopChan and SecondAdjHopChan (i.e. for which the aChanAvailableFlags[] are set to true.)
            // Using these we will generate a second list of candidate channels.
            // The second list of candidate channels are those channels that are permitted in the second adjacent hop, given the 
            // currently seleted current hop channel and first adjacent hop channel.  If there is no channel in common between the 
            // second candidate list and the second adjacent hop channels list, then the current hop channel is not valid and will
            // be changed from "permitted" to "not permitted" in the first candidate list.
            if (pSeqEndCandidates[FirstAdjHopIndex].aChanAvailableFlags[FirstAdjHopChan] &&
                pSeqEndCandidates[SecondAdjHopIndex].aChanAvailableFlags[SecondAdjHopChan] && aFirstCandidateChanFlags[SecondAdjHopChan])
            {
               // Reset the second candidate channels array before passing its address to the function below.
               memset(aSecondCandidateChanIndices, 0, NUM_HOPPING_CHANS * sizeof(Channel_t));

               // Generate the second candidate list in aSecondCandidateChanIndices.
               // Note, aSecondCandidateChanIndices will not include the channel used for SecondAdjHopChan.
               // The OR_THE_FLAGS_E has no effect, because the next variable is NULL.
               GetPermittedChans((Channel_t)FirstAdjHopChan, (Channel_t)SecondAdjHopChan, MinChanInterval,
                  OR_THE_FLAGS_E, NULL, &NumPermittedChans, aSecondCandidateChanIndices);

               // Initalise ChannelPermitted to false.  It may later be set to true after testing that specific conditions are met.
               bool ChannelPermitted = false;

               // Compare the channels in aSecondCandidateChanIndices[] with those permitted in the second adjacent hop 
               // (i.e. aSeqEndCandidates[CurrentHopIndex - 2].aChanAvailableFlags[]).  If there is at least one channel in common
               // then this proves that SecondAdjHopChan is a valid permitted channel for the current hop.
               // Recall that the second candidate list does include SecondAdjHopChan as a member.
               for (uint16_t k = 0; k < NumPermittedChans; k++)
               {
                  ChannelPermitted = ChannelPermitted || (pSeqEndCandidates[SecondAdjHopIndex].aChanAvailableFlags[aSecondCandidateChanIndices[k]]);
               }

               // If none of the channels are permitted, remove this channel from the first candidate list.
               if (!ChannelPermitted)
               {
                  // Remove SecondAdjHopChan from the first candidate list.
                  aFirstCandidateChanFlags[SecondAdjHopChan] = false;
                  // Break to terminate the inner for loop.
                  break;
               }
            }
         }
      }

      // Count the number of available channels, and copy them to aSeqEndCandidates
      NumPermittedChans = 0;
      for (uint16_t i = 0; i < NUM_HOPPING_CHANS; i++)
      {
         pSeqEndCandidates[CurrentHopIndex].aChanAvailableFlags[i] = aFirstCandidateChanFlags[i];
         NumPermittedChans += (pSeqEndCandidates[CurrentHopIndex].aChanAvailableFlags[i]);
      }

      // Increment counters for next iteration of the while loop.
      (*pNumEndOfSeqChans)++;
      CurrentHopIndex++;

      // Count of the number of consecutive hops in which all the channels are permitted.
      // When this reaches a vale of 2, the sequence end candidates list is then deemed complete, and the while loop will stop.
      LoopTerminationCounter = (LoopTerminationCounter + 1) * (NUM_HOPPING_CHANS == NumPermittedChans);
   }

   return ErrorCode;
}


/*************************************************************************************/
/**
* function name : GetPermittedChans
* description   : Generates a list of permitted channels.
*                 It uses two rules applicable to sequential hops in the hopping sequence
*                 Those rules are:
*                 a) the channel should change by at least a specified minimum number of channels,
*                 to ensure hops are >= 1MHz),
*                 b) that the same channel is not used within three hops of each other (to prevent 
*                 the hopping sequence from merely toggling between two channels for extended periods).
*                 Furthermore, it will either AND or OR the calculated channels with those passed
*                 to the function, under the control of the input BoolAlgebraOption.
*                 It returns an array of flags (one per channel) with true indicating the channel can be used.
*                 (Its pointer can be set to NULL if the array not used.)
*                 It returns the number of permitted channels (calculated post AND or OR action if used).
*                 It returns an array containing the indicies of the permitted channels.
*                 (Its pointer can be set to NULL if the array not used.)
*
* INPUTS
* @param - const Channel_t FirstAdjHopChan : The channel of the previous hop.
* @param - const Channel_t SecondAdjHopChan : The channel of the second from previous hop.
* @param - const uint16_t MinChanInterval : Minimum number of channels to hop (e.g. EN54 
*          specifies minimum of 1.0 MHz) 
* @param - const BoolAlgebraType_t BoolAlgebraOption : Specifies whether to AND or OR channel
*          availability flags with those passed to the function using pPermittedChanFlags.
* @param - bool * pPermittedChanFlags : Pointer to array of channel permitted flags.
*          This can be set to NULL, in which the pointer is not used to pass values in or out.
*          If the pointer is not NULL, each flag is read to see which channels can be selected by the function.
* @param - uint16_t * const pNumPermittedChans : Variable into which is written the number of permitted channels.
8          This typically corresponds to the number of true flags returned with pPermittedChanFlags,
*          and/or the number of elements of the array specified with pPermittedChanIndices that contain
*          valid channel indicies.
* @param - Channel_t * const pPermittedChanIndices : Pointer to array of permitted channel indicies.
*          This can be set to NULL, in which case the pointer is not used to pass values in or out.
*
* @return - Type is ErrorCode_t.  Returns SUCCESS_E (0).
*/
static ErrorCode_t GetPermittedChans(
   const Channel_t FirstAdjHopChan,
   const Channel_t SecondAdjHopChan,
   const uint16_t MinChanInterval,
   const BoolAlgebraType_t BoolAlgebraOption,
   bool * pPermittedChanFlags,                   // Can be NULL - describe this in header.
   uint16_t * const pNumPermittedChans,
   Channel_t * const pPermittedChanIndices)      // Can be NULL - describe this in header.
{
   bool aPermittedChans[NUM_HOPPING_CHANS];
   uint8_t FirstProhibitedIndex = 0;
   uint8_t LastProhibitedIndex = 0;
   uint8_t i = 0;

   // Initialise the array to mark all the channels as available.
   memset(aPermittedChans, true, NUM_HOPPING_CHANS * sizeof(bool));

   // Step 1: mark as unavailable any channels not meeting the rules for being 
   // at least MinChanInterval from the channel of the first adjacent slot.

   // Calculate the start and terminating indices of the for loop used to mark the prohibited channels.
   FirstProhibitedIndex = (FirstAdjHopChan - MinChanInterval + 1 + NUM_HOPPING_CHANS) % NUM_HOPPING_CHANS;
   LastProhibitedIndex = FirstProhibitedIndex + (2 * MinChanInterval) - 1;

   // Mark the prohibited channels as false.
   for (i = FirstProhibitedIndex; i < LastProhibitedIndex; i++)
   {
      aPermittedChans[i % NUM_HOPPING_CHANS] = false;
   }

   // Step 2: mark as unavailable any channels not meeting the rule for not using
   // the same channel as was used for the second adjacent slot.
   aPermittedChans[SecondAdjHopChan] = false;

   // If pPermittedChanFlags is NULL, set the pointer to the address of the array aPermittedChans.
   // (Set this pointer to NULL when you just want to count, but not list, the number of permitted channels.)
   if (!pPermittedChanFlags)
   {
      pPermittedChanFlags = aPermittedChans;
   }
   else
   {
      // Step 3: logically combine the locally generated channels with the values from the input pointer.
      // Note that if pPermittedChanFlags was passed to this function as NULL, this step is not necessary.
      if (AND_THE_FLAGS_E == BoolAlgebraOption)
      {
         for (i = 0; i < NUM_HOPPING_CHANS; i++)
         {
            pPermittedChanFlags[i] = pPermittedChanFlags[i] && aPermittedChans[i];
         }
      }
      else
      {
         for (i = 0; i < NUM_HOPPING_CHANS; i++)
         {
            pPermittedChanFlags[i] = pPermittedChanFlags[i] || aPermittedChans[i];
         }
      }
   }

   // Step 4: count the number of allowed channels
   uint16_t NumPermittedChans = 0;
   for (i = 0; i < NUM_HOPPING_CHANS; i++)
   {
      if (true == pPermittedChanFlags[i])
      {
         // If pPermittedChanIndices isn't NULL, write the channel index to the array it points to. 
         if (pPermittedChanIndices)
         {
            pPermittedChanIndices[NumPermittedChans] = i;
         }

         // Increment NumPermittedChans.
         NumPermittedChans++;
      }
   }
   *pNumPermittedChans = NumPermittedChans;

   return SUCCESS_E;
}


/*************************************************************************************/
/**
* function name : TestChanHopSeq
* description   : Tests that the channel hopping rules have been correctly applied.
*                 Performs the test over two complete cycles of the sequence, to 
*                 ensure rules are followed throughout, including when the sequence wraps.
*
* INPUTS
* @param - Channel_t * const pChanHopSeqArray : Pointer to the array to contain the hopping sequence.
* @param - const uint16_t SeqLength : Number of hops in the whole sequence (after which, sequence should be repeated).
* @param - const Channel_t NumChans : Number of hopping channels.  Should not exceed the s/w constant NUM_HOPPING_CHANS.
* @param - const uint16_t MinChanInterval : Minimum number of channels to hop (e.g. EN54 specifies minimum of 1.0 MHz)
*
* @return - Type is ErrorCode_t.  Returns SUCCESS_E (0) if the function succeeds, else
*           returns one of the other values of ErrorCode_t to convey the reason for
*           the error or warning.
*/
static ErrorCode_t TestChanHopSeq(
   Channel_t * pChanHopSeqArray,
   uint16_t   SeqLength,
   Channel_t NumChans,
   uint16_t MinChanInterval)
{
   ErrorCode_t  ErrorCode = SUCCESS_E;
   uint16_t MinValue = 65535;    // Initial value to track minimum value of a uint16_t variable.
   uint16_t MaxValue = 0;        // Initial value to track maximum value of a uint16_t variable.

   // Loop through the entire sequence twice, checking compliance with rules for first adjacent and second adjacent hops.
   // Also, record the incidences of each channel, to allow for channel balancing checks later.
   for (uint16_t n = 2; n < ((2 * SeqLength) + 2); n++)
   {
      uint16_t CurrentIndex = (n - 0) % SeqLength;
      uint16_t FirstAdjIndex = (n - 1) % SeqLength;
      uint16_t SecondAdjIndex = (n - 2) % SeqLength;

      // Check that the channel for hop(n) is different to the channel in hop(n-2).
      if (pChanHopSeqArray[CurrentIndex] == pChanHopSeqArray[SecondAdjIndex])
      {
         DEBUG_PRINT("\nCAHNNEL HOPPING SEQUENCE GENERATION FAILED.  The channel used for hop(n) is the same as hop(n-2).");
         return ERR_SECOND_ADJACENT_HOP_CLASH_E;
      }

      // Check that the channel for hop(n) is "sufficiently far away" from the channel in hop(n-1).
      MinValue = MIN(pChanHopSeqArray[CurrentIndex], pChanHopSeqArray[FirstAdjIndex]);
      MaxValue = MAX(pChanHopSeqArray[CurrentIndex], pChanHopSeqArray[FirstAdjIndex]);

      // For cyclic hopping sequences, "sufficiently far away" means <= MinChanInterval using a cyclic channel numbering scheme.
      if (MaxValue - MinValue < MinChanInterval || MaxValue - MinValue >(NUM_HOPPING_CHANS - MinChanInterval))
      {
         DEBUG_PRINT("\nCAHNNEL HOPPING SEQUENCE GENERATION FAILED.  The channel used for hop(n) is too close to that for hop(n-1).");
         return ERR_FIRST_ADJACENT_HOP_CLASH_E;
      }
   }

   return ErrorCode;
}

