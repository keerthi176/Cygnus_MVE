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
*  File         : MC_PUP.c
*
*  Description  : Routines to pack and unpack mesh protocol messages using the 
*                 formats described in the following document;
*
*                 HKD-16-0015-D_A37 Mesh Protocol Design
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdlib.h>
#include <string.h>



/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "CO_ErrorCode.h"
#include "MC_PUP.h"
#include "MC_MacConfiguration.h"

/* Private Functions Prototypes
*************************************************************************************/
static void MC_PUP_ShiftRegInit(void);
static void MC_PUP_ShiftRegLoadOnRight(uint32_t value, uint32_t size);
static void MC_PUP_ShiftRegPadOnRight(const uint32_t size);
static void MC_PUP_ShiftRegDiscardOnRight(const uint32_t size);
static void MC_PUP_ShiftRegExtractFromRight(uint32_t *pLocation, uint32_t sizeBits);
static void MC_PUP_ShiftRegAlignLeft(void);
static void MC_PUP_ShiftRegAlignRight(void);
static uint32_t MC_PUP_ReverseOrder(const uint32_t value);

/* Global Variables
*************************************************************************************/


/* Private Variables
*************************************************************************************/
#define SHIFT_REG_SIZE_BYTES                          64
#define SHIFT_REG_SIZE_BITS                           (SHIFT_REG_SIZE_BYTES * BITS_PER_BYTE)
#define ENCRYPTED_PAYLOAD_SIZE_BITS                   64
#define MAX_FOR_16_BITS                               65536
#define MAX_FOR_12_BITS                               4096
#define MAX_FOR_9_BITS                                512
#define MAX_FOR_8_BITS                                256
#define MAX_FOR_7_BITS                                128
#define MAX_FOR_1_BITS                                2
static uint8_t shiftReg[SHIFT_REG_SIZE_BYTES];
static uint32_t messageLengthBits;


/*************************************************************************************/
/**
* MC_PUP_ShiftRegInit
* Function to initialise the shift register used for packing and unpacking messages
* This clears the register and the message length.
*
* @param - none
*
* @return - void
*/
static void MC_PUP_ShiftRegInit(void)
{
   memset(shiftReg, 0, sizeof(shiftReg));
   messageLengthBits = 0;
}


/*************************************************************************************/
/**
* MC_PUP_ShiftRegLoadOnRight
* Function to shift the contents of the shift register to the left and load a new
* value at the right end of the shift register.
*
* The parameters must be loaded in sequence from the first to the last
*
* This function supports parameters up to 32 bits wide
*
* @param - value to be loaded INPUT
* @param - sizeBits - width (in bits) of the parameter to be loaded INPUT
*
* @return - void
*/
static void MC_PUP_ShiftRegLoadOnRight(uint32_t value, uint32_t sizeBits)
{
   if ((32 >= sizeBits) && (0 < sizeBits) && (SHIFT_REG_SIZE_BITS >= messageLengthBits + sizeBits))
   {
      uint32_t byteShift = sizeBits >> 3;
      uint32_t bitShift = sizeBits & 7;
      uint32_t i;
      uint8_t p1;
      uint8_t p2;
      uint32_t mask;

      /* shift everything to the left by size bits */
      for (i = 0; i < SHIFT_REG_SIZE_BYTES; i++)
      {
         if (i + byteShift < SHIFT_REG_SIZE_BYTES - 1)
         {
            p1 = shiftReg[i + byteShift] << bitShift;
            p2 = shiftReg[i + byteShift + 1] >> (BITS_PER_BYTE - bitShift);
            shiftReg[i] = p1 | p2;
         }
         else if (i + byteShift == SHIFT_REG_SIZE_BYTES - 1)
         {
            p1 = shiftReg[i + byteShift] << bitShift;
            p2 = 0;
            shiftReg[i] = p1 | p2;
         }
         else
         {
            shiftReg[i] = 0;
         }
      }

      /* create mask */
      mask = 0;
      for (i = 0; i < sizeBits; i++)
      {
         mask = mask | (1 << i);
      }

      /* load value at right end */
      for (i = 0 ; i < BYTES_PER_WORD ; i++)
      {
         p1 = shiftReg[SHIFT_REG_SIZE_BYTES - BYTES_PER_WORD + i];
         p2 = (value & mask) >> (BYTES_PER_WORD - 1 - i) * BITS_PER_BYTE;
         shiftReg[SHIFT_REG_SIZE_BYTES - BYTES_PER_WORD + i] = p1 | p2;
      }

      /* increment message length*/
      messageLengthBits = messageLengthBits + sizeBits;
   }
}

/*************************************************************************************/
/**
* MC_PUP_ShiftRegPadOnRight
* Function to shift the contents of the shift register to the left and pad with
* zeros.
*
* @param - sizeBits - Number of Padding 'zero' bits to add. INPUT
*
* @return - void
*/
void MC_PUP_ShiftRegPadOnRight(const uint32_t size)
{
   uint32_t numberOfWholeWords = 0;
   uint32_t numberOfRemainingBits = 0;
   uint32_t numberOfPaddingBits = size;
   uint32_t wordCount = 0;
   
   //Limit the extent of the shift operation to prevent messageLengthBits
   //exceeding the size of the shift buffer.
   if ( SHIFT_REG_SIZE_BITS < (messageLengthBits + size) )
   {
      numberOfPaddingBits = SHIFT_REG_SIZE_BITS - messageLengthBits;
   }
   
   // Efficient code to avoid division and modulo operations requires the following to be true
   CO_ASSERT_VOID(BITS_PER_WORD == 32)
   CO_ASSERT_VOID(BITS_PER_WORD_SHIFT == 5)
   CO_ASSERT_VOID(BITS_PER_WORD_MASK == 0X1F)
   
   //Break the required padding down to whole words and remaining bits.
   numberOfWholeWords = numberOfPaddingBits >> BITS_PER_WORD_SHIFT;   // Efficient form of "numberOfPaddingBits / BITS_PER_WORD";
   numberOfRemainingBits = numberOfPaddingBits & BITS_PER_WORD_MASK;  // Efficient form of "numberOfPaddingBits % BITS_PER_WORD";
   
   //Write the padded words one word at a time
   for ( wordCount = 0; wordCount < numberOfWholeWords; wordCount++ )
   {
      MC_PUP_ShiftRegLoadOnRight( 0, BITS_PER_WORD );
   }
   //Now write the remaining bits
   if ( numberOfRemainingBits > 0 )
   {
      MC_PUP_ShiftRegLoadOnRight( 0, numberOfRemainingBits );
   }
}

/*************************************************************************************/
/**
* MC_PUP_ShiftRegDiscardOnRight
* Function to strip a specified number of bits from the right of the shift buffer
* and discard them.  Used for removing padded bits.
*
* @param - sizeBits - Number of Padding 'zero' bits to remove. INPUT
*
* @return - void
*/
void MC_PUP_ShiftRegDiscardOnRight(const uint32_t size)
{
   uint32_t numberOfWholeWords = 0;
   uint32_t numberOfRemainingBits = 0;
   uint32_t numberOfPaddingBits = size;
   uint32_t wordCount = 0;
   uint32_t discardedWord = 0;
   
   //Limit the extent of the shift operation to prevent messageLengthBits
   //exceeding the size of the shift buffer.
   if ( SHIFT_REG_SIZE_BITS < (messageLengthBits + size) )
   {
      numberOfPaddingBits = SHIFT_REG_SIZE_BITS - messageLengthBits;
   }
   
   // Efficient code to avoid division and modulo operations requires the following to be true
   CO_ASSERT_VOID(BITS_PER_WORD == 32)
   CO_ASSERT_VOID(BITS_PER_WORD_SHIFT == 5)
   CO_ASSERT_VOID(BITS_PER_WORD_MASK == 0X1F)

   //Break the required padding down to whole words and remaining bits.
   numberOfWholeWords = numberOfPaddingBits >> BITS_PER_WORD_SHIFT;   // Efficient form of "numberOfPaddingBits / BITS_PER_WORD";
   numberOfRemainingBits = numberOfPaddingBits & BITS_PER_WORD_MASK;  // Efficient form of "numberOfPaddingBits % BITS_PER_WORD";
   
   //Write the padded words one word at a time
   for ( wordCount = 0; wordCount < numberOfWholeWords; wordCount++ )
   {
      MC_PUP_ShiftRegExtractFromRight( &discardedWord, BITS_PER_WORD );
   }
   //Now write the remaining bits
   if ( numberOfRemainingBits > 0 )
   {
      MC_PUP_ShiftRegExtractFromRight( &discardedWord, numberOfRemainingBits );
   }
}

/*************************************************************************************/
/**
* MC_PUP_ShiftRegExtractFromRight
* Function to read a parameter from the right hand end of the shift register and then
* shift the remaining contents of the shift register to the right so that the next
* parameter is aligned with the right hand side of the shift register.
*
* The parameters must be extracted in sequence from the last to the first
*
* This function supports parameters up to 32 bits wide
*
* @param - pLocation - location to put extracted value OUTPUT
* @param - sizeBits  - width (in bits) of the parameter to be loaded INPUT
*
* @return - void
*/
static void MC_PUP_ShiftRegExtractFromRight(uint32_t *pLocation, uint32_t sizeBits)
{
   if ( pLocation )
   {
      if ((32 >= sizeBits) && (0 < sizeBits) && (sizeBits <= messageLengthBits))
      {
         uint32_t byteShift = sizeBits >> 3;
         uint32_t bitShift = sizeBits & 7;
         int32_t i;
         uint8_t p1;
         uint8_t p2;
         uint32_t mask;
         uint32_t temp = 0;
         uint32_t maskShift;

         /* create mask */
         mask = 0;
         for (maskShift = 0; maskShift < sizeBits; maskShift++)
         {
            mask = mask | (1 << maskShift);
         }

         /* extract value at right end */
         temp = shiftReg[SHIFT_REG_SIZE_BYTES - 4] << 24 |
                 shiftReg[SHIFT_REG_SIZE_BYTES - 3] << 16 |
               shiftReg[SHIFT_REG_SIZE_BYTES - 2] << 8 |
               shiftReg[SHIFT_REG_SIZE_BYTES - 1];
         *pLocation = temp & mask;

         /* shift everything to the right by size bits */
         for (i = SHIFT_REG_SIZE_BYTES - 1; i >= 0; i--)
         {
            if ((int32_t)(i - byteShift) > 0)
            {
               p1 = shiftReg[i - byteShift] >> bitShift;
               p2 = shiftReg[i - byteShift - 1] << (BITS_PER_BYTE - bitShift);
               shiftReg[i] = p1 | p2;
            }
            else if (i - byteShift == 0)
            {
               p1 = shiftReg[i - byteShift] >> bitShift;
               p2 = 0;
               shiftReg[i] = p1 | p2;
            }
            else
            {
               shiftReg[i] = 0;
            }
         }

         /* decrement message length*/
         messageLengthBits = messageLengthBits - sizeBits;
      }
   }
}


/*************************************************************************************/
/**
* MC_PUP_ShiftRegAlignLeft
* Function to shift the contents of the shift register so that it is aligned with the
* Left hand end of the shift register.
*
* @param - none
*
* @return - void
*/
static void MC_PUP_ShiftRegAlignLeft(void)
{
   uint32_t totalBitShift = SHIFT_REG_SIZE_BITS - messageLengthBits;
   uint32_t i;
   uint32_t p1;
   uint32_t p2;

   /* split total shift into bytes and bits */
   uint32_t byteShift = totalBitShift >> 3;
   uint32_t bitShift = totalBitShift & 7;

   /* shift everything to the left by size bits */
   for (i = 0; i < SHIFT_REG_SIZE_BYTES; i++)
   {
      if (i + byteShift < SHIFT_REG_SIZE_BYTES - 1)
      {
         p1 = shiftReg[i + byteShift] << bitShift;
         p2 = shiftReg[i + byteShift + 1] >> (BITS_PER_BYTE - bitShift);
         shiftReg[i] = p1 | p2;
      }
      else if (i + byteShift == SHIFT_REG_SIZE_BYTES - 1)
      {
         p1 = shiftReg[i + byteShift] << bitShift;
         p2 = 0;
         shiftReg[i] = p1 | p2;
      }
      else
      {
         shiftReg[i] = 0;
      }
   }
}


/*************************************************************************************/
/**
* MC_PUP_ShiftRegAlignRight
* Function to shift the contents of the shift register so that it is aligned with the
* Right hand end of the shift register.
*
* @param - none
*
* @return - void
*/
static void MC_PUP_ShiftRegAlignRight(void)
{
   uint32_t totalBitShift = SHIFT_REG_SIZE_BITS - messageLengthBits;
   int32_t i;
   uint32_t p1;
   uint32_t p2;

   /* split total shift into bytes and bits */
   uint32_t byteShift = totalBitShift >> 3;
   uint32_t bitShift = totalBitShift & 7;

   /* shift everything to the right by message length bits */
   for (i = SHIFT_REG_SIZE_BYTES - 1; i >= 0; i--)
   {
      if ((int32_t)(i - byteShift) > 0)
      {
         p1 = shiftReg[i - byteShift] >> bitShift;
         p2 = shiftReg[i - byteShift - 1] << (BITS_PER_BYTE - bitShift);
         shiftReg[i] = p1 | p2;
      }
      else if (i - byteShift == 0)
      {
         p1 = shiftReg[i - byteShift] >> bitShift;
         p2 = 0;
         shiftReg[i] = p1 | p2;
      }
      else
      {
         shiftReg[i] = 0;
      }
   }
}


/*************************************************************************************/
/**
* MC_PUP_ParameterGet
* Function to read a single parameter from a location in an array of bytes. This 
* routine is able to cope with parameters that span byte boundaries.
*
* @param - pointer to buffer containing the initial array of bytes INPUT
* @param - start - index (in bits) of the start location of the requested parameter INPUT
* @param - size - width (in bits) of the requestes parameter INPUT
*
* @return - parameter value

*/
uint32_t MC_PUP_ParameterGet(const uint8_t * const pBuffer, const uint32_t start, 
                                                               const uint32_t size)
{
   uint16_t temp1 = 0;
   uint8_t temp2[4] = { 0 };
   uint32_t i = 0;
   uint32_t end = start + size;
   uint8_t shift = 0;
   uint32_t value = 0;
   uint32_t mask = 0xffffffff;
   
   if ( pBuffer )
   {

      /* calculate shift - byte aligned parameters will have shift=0 */
      if (0 != (end & 7))
      {
         shift = 8 - (end & 7);
      }

      /* step through bytes of output - number of bytes is size / 8 rounded up */
      for (i = 0; i < ((size + 7 ) >> 3); i++)
      {
         temp1 = pBuffer[((end - 1) >> 3) - i];
         if (0 == shift)
         {
            temp2[i] = (uint8_t)temp1;
         }
         else
         {
            /* Ensure we don't index beyond the beginning of pBuffer. */
            int32_t byteIndex = ((end - 1) >> 3) - i - 1;
            if (byteIndex < 0)
            {
               temp1 &= 0x00ff;
            }
            else
            {
               temp1 = temp1 | (pBuffer[byteIndex] << 8);
            }
            temp2[i] = temp1 >> shift;
         }
      }

      if (size < 32)
      {
         mask = (1 << size) - 1;
      }

      value = (temp2[3] << 24 | temp2[2] << 16 | temp2[1] << 8 | temp2[0]) & mask;
   }
   return value;
}
   

/*************************************************************************************/
/**
* MC_PUP_GetMessageType
* Function to take a Phy_Data_Ind message received from the LoRa modem and extract the
* types of frame and application layer message within the message.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to Frame Type OUTPUT
* @param - pointer to flag indicating if Application Layer Message is present OUTPUT
* @param - pointer to Application Layer Message Type OUTPUT
*
* @return - status - 0 for success, any other value is an error

*/
ErrorCode_t MC_PUP_GetMessageType(CO_MessagePayloadPhyDataInd_t *pPhyDataInd, 
                                  FrameType_t *pFrameType, 
                                  bool *pAppMsgPresent, 
                                  ApplicationLayerMessageType_t *pAppMsgType)
{
   ErrorCode_t status = SUCCESS_E;
   FrameType_t frameType = (FrameType_t)0;
   
   /* check input parameters */
   if (pPhyDataInd == NULL)
   {
      return ERR_INVALID_POINTER_E;
   }
   if (pFrameType == NULL)
   {
      return ERR_INVALID_POINTER_E;
   }
   if (pAppMsgPresent == NULL)
   {
      return ERR_INVALID_POINTER_E;
   }
   if (pAppMsgType == NULL)
   {
      return ERR_INVALID_POINTER_E;
   }

   frameType = (FrameType_t)MC_PUP_ParameterGet((uint8_t *)pPhyDataInd, PUP_FRAME_TYPE_START,
                                                                        PUP_FRAME_TYPE_SIZE);
   
   switch (frameType)
   {
      case FRAME_TYPE_HEARTBEAT_E:
      {
         *pFrameType = FRAME_TYPE_HEARTBEAT_E;
         *pAppMsgPresent = false;
         *pAppMsgType = (ApplicationLayerMessageType_t)NULL;
         break;
      }
      
      case FRAME_TYPE_DATA_E:
      {
         *pFrameType = FRAME_TYPE_DATA_E;
         ApplicationLayerMessageType_t messageType = (ApplicationLayerMessageType_t)
                                                      MC_PUP_ParameterGet((uint8_t *)pPhyDataInd,
                                                                        DATA_MESSAGE_TYPE_START,
                                                                        DATA_MESSAGE_TYPE_SIZE);
         if (APP_MSG_TYPE_MAX_E > messageType)
         {
            *pAppMsgPresent = true;
            *pAppMsgType = messageType;
         }
         else
         {
            status = ERR_OUT_OF_RANGE_E;
         }
         break;
      }

      case FRAME_TYPE_ACKNOWLEDGEMENT_E:
      {
         *pFrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
         *pAppMsgPresent = false;
         *pAppMsgType = (ApplicationLayerMessageType_t)NULL;
         break;
      }
      case FRAME_TYPE_TEST_MESSAGE_E:
      {
         *pFrameType = FRAME_TYPE_TEST_MESSAGE_E;
         *pAppMsgPresent = true;
         *pAppMsgType = APP_MSG_TYPE_TEST_SIGNAL_E;
         break;
      }
      case FRAME_TYPE_PPU_MODE_E:
      {
         *pFrameType = FRAME_TYPE_PPU_MODE_E;
         *pAppMsgPresent = true;
         *pAppMsgType = APP_MSG_TYPE_PPU_MODE_E;
         break;
      }
       case FRAME_TYPE_EXIT_TEST_MODE_E:
      {
         *pFrameType = FRAME_TYPE_EXIT_TEST_MODE_E;
         *pAppMsgPresent = true;
         *pAppMsgType = APP_MSG_TYPE_EXIT_TEST_MODE_E;
         break;
      }     
      case FRAME_TYPE_AT_E:
      {
         *pFrameType = FRAME_TYPE_AT_E;
         *pAppMsgPresent = true;
         *pAppMsgType = APP_MSG_TYPE_AT_COMMAND_E;
         break;
      }
      default:
         status = ERR_OUT_OF_RANGE_E;
         break;
   }


   return status;
}


/*************************************************************************************/
/**
* MC_PUP_ReverseOrder
* Function to reverse the byte order of a uint32_t.
*
* @param - pointer to Test Signal message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
uint32_t MC_PUP_ReverseOrder(const uint32_t value)
{
   uint32_t rev_order;
   
   rev_order = value >> 24;
   rev_order |= (( value >> 8 ) & 0x0000ff00 );
   rev_order |= (( value << 8 ) & 0x00ff0000 );
   rev_order |= ( value << 24 );
   
   return rev_order;
}

/*************************************************************************************/
/**
* MC_PUP_PackHeartbeat
* Function to pack a mesh protocol heartbeat message into a Phy_Data_Req message to be 
* passed to the LoRa modem. 
*
* @param - pointer to Heartbeat message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - 0 for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackHeartbeat(FrameHeartbeat_t *pHeartbeat, 
                                 CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   //CO_PRINT_B_0("MC_PUP_PackHeartbeat+\r\n");
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;
   
   /* check input parameters */
   if ((pHeartbeat == NULL) || (pPhyDataReq == NULL))
   {
      return ERR_INVALID_POINTER_E;
   }

   MC_PUP_ASSERT(pHeartbeat->FrameType == FRAME_TYPE_HEARTBEAT_E);
   MC_PUP_ASSERT(pHeartbeat->SlotIndex < SLOTS_PER_SHORT_FRAME);
//   MC_PUP_ASSERT(pHeartbeat->SlotIndex < FIRST_PRACH_ACK_SLOT_IDX);
   MC_PUP_ASSERT(pHeartbeat->ShortFrameIndex < MAX_SHORT_FRAMES_PER_LONG_FRAME);
   MC_PUP_ASSERT(pHeartbeat->LongFrameIndex < LONG_FRAMES_PER_SUPER_FRAME);
   MC_PUP_ASSERT(pHeartbeat->State < STATE_MAX_E);
   MC_PUP_ASSERT(pHeartbeat->Rank < 64);//Max value of 15 can fit into 6 bits of Rank
   MC_PUP_ASSERT(pHeartbeat->NoOfChildrenIdx < 16);//Max value that fits into 4 bits
   MC_PUP_ASSERT(pHeartbeat->NoOfChildrenOfPTNIdx < 16);//Max value that fits into 4 bits
   MC_PUP_ASSERT(pHeartbeat->ActiveFrameLength < 2);//Max for 1 bit

   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pHeartbeat->FrameType, HEARTBEAT_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pHeartbeat->SlotIndex, HEARTBEAT_SLOT_INDEX_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pHeartbeat->ShortFrameIndex, HEARTBEAT_SHORT_FRAME_INDEX_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pHeartbeat->LongFrameIndex, HEARTBEAT_LONG_FRAME_INDEX_SIZE);
   
   uint32_t state = (uint32_t)pHeartbeat->State;
   MC_PUP_ShiftRegLoadOnRight(state, HEARTBEAT_STATE_SIZE);
   
   MC_PUP_ShiftRegLoadOnRight(pHeartbeat->Rank, HEARTBEAT_RANK_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pHeartbeat->ActiveFrameLength, HEARTBEAT_ACTIVE_FRAME_LENGTH_SIZE);
   // TODO CYG2-806: The NCU should enter 0 for NoOfChildrenIdx and NoOfChildrenOfPTNIdx.
   MC_PUP_ShiftRegLoadOnRight(pHeartbeat->NoOfChildrenIdx, HEARTBEAT_NUMBER_OF_CHILDREN_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pHeartbeat->NoOfChildrenOfPTNIdx, HEARTBEAT_NUMBER_OF_CHILDREN_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pHeartbeat->SystemId, HEARTBEAT_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;
   
   
   return status;
}


/*************************************************************************************/
/**
* MC_PUP_UnpackHeartbeat
* Function to unpack a mesh protocol heartbeat message from a Phy_Data_Ind message received 
* from the LoRa modem. 
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting Heartbeat message OUTPUT
*
* @return - status - 0 for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackHeartbeat(CO_MessagePayloadPhyDataInd_t *pPhyDataInd, 
                                    FrameHeartbeat_t *pHeartbeat)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ( (pPhyDataInd == NULL) || (pHeartbeat == NULL) )
   {
      status = ERR_INVALID_POINTER_E;
   }   
   else
   {
      //input parameters are OK.
      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = HEARTBEAT_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pHeartbeat->SystemId, HEARTBEAT_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pHeartbeat->NoOfChildrenOfPTNIdx, HEARTBEAT_NUMBER_OF_CHILDREN_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pHeartbeat->NoOfChildrenIdx, HEARTBEAT_NUMBER_OF_CHILDREN_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pHeartbeat->ActiveFrameLength, HEARTBEAT_ACTIVE_FRAME_LENGTH_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pHeartbeat->Rank, HEARTBEAT_RANK_SIZE);

      uint32_t state;
      MC_PUP_ShiftRegExtractFromRight(&state, HEARTBEAT_STATE_SIZE);
      pHeartbeat->State = (CO_State_t)state;

      MC_PUP_ShiftRegExtractFromRight(&pHeartbeat->LongFrameIndex, 
                                       HEARTBEAT_LONG_FRAME_INDEX_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pHeartbeat->ShortFrameIndex, 
                                       HEARTBEAT_SHORT_FRAME_INDEX_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pHeartbeat->SlotIndex, HEARTBEAT_SLOT_INDEX_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pHeartbeat->FrameType, HEARTBEAT_FRAME_TYPE_SIZE);

      MC_PUP_ASSERT(pHeartbeat->FrameType == FRAME_TYPE_HEARTBEAT_E);
      MC_PUP_ASSERT(pHeartbeat->SlotIndex < SLOTS_PER_SHORT_FRAME);
//      MC_PUP_ASSERT(pHeartbeat->SlotIndex < FIRST_PRACH_ACK_SLOT_IDX);
      MC_PUP_ASSERT(pHeartbeat->ShortFrameIndex < MAX_SHORT_FRAMES_PER_LONG_FRAME);
      MC_PUP_ASSERT(pHeartbeat->LongFrameIndex < LONG_FRAMES_PER_SUPER_FRAME);
      MC_PUP_ASSERT(pHeartbeat->State < STATE_MAX_E);
      MC_PUP_ASSERT(pHeartbeat->Rank < 64);//Max value of 15 can fit into 6 bits of Rank
      MC_PUP_ASSERT(pHeartbeat->NoOfChildrenIdx < 16);//Max value that fits into 4 bits
      MC_PUP_ASSERT(pHeartbeat->NoOfChildrenOfPTNIdx < 16);//Max value that fits into 4 bits
      MC_PUP_ASSERT(pHeartbeat->ActiveFrameLength < 2);//Max for 1 bit

   }

   return status;
}

/*************************************************************************************/
/**
* MC_PUP_PackAlarmSignal
* Function to pack a mesh protocol Alarm Signal message into a Phy_Data_Req message to be 
* passed to the LoRa modem.
* Also used to pack the fire signal, which has an identical structure.
*
* @param - pointer to Alarm Signal message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackAlarmSignal( AlarmSignal_t *pAlarmSignal, 
                                    CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ( (NULL == pPhyDataReq) || (NULL == pAlarmSignal) )
   {
      return ERR_INVALID_POINTER_E;
   }
   /* Range check the alarm signal values */
   MC_PUP_ASSERT(pAlarmSignal->Header.FrameType == FRAME_TYPE_DATA_E);
   MC_PUP_ASSERT(pAlarmSignal->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pAlarmSignal->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pAlarmSignal->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pAlarmSignal->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pAlarmSignal->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT((pAlarmSignal->Header.MessageType == APP_MSG_TYPE_FIRE_SIGNAL_E) ||
                  (pAlarmSignal->Header.MessageType == APP_MSG_TYPE_ALARM_SIGNAL_E));
   MC_PUP_ASSERT(pAlarmSignal->RUChannel < 128);//max for 7 bits
   MC_PUP_ASSERT(pAlarmSignal->SensorValue < MAX_FOR_8_BITS);//max for 8 bits

   /*build the message in the shift register */
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pAlarmSignal->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pAlarmSignal->Header.MACDestinationAddress,
                              DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pAlarmSignal->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pAlarmSignal->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pAlarmSignal->Header.DestinationAddress,
                              DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pAlarmSignal->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pAlarmSignal->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pAlarmSignal->RUChannel, ALARM_SIGNAL_RU_CHANNEL_INDEX_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pAlarmSignal->SensorValue, ALARM_SIGNAL_SENSOR_VALUE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pAlarmSignal->Zone, ALARM_SIGNAL_ZONE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pAlarmSignal->AlarmActive, ALARM_SIGNAL_ACTIVE_SIZE);
   MC_PUP_ShiftRegPadOnRight( ALARM_SIGNAL_PAYLOAD_PADDING_SIZE );
   MC_PUP_ShiftRegLoadOnRight(pAlarmSignal->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;
   
   
   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackAlarmSignal
* Function to unpack a mesh protocol alarm message from a Phy_Data_Req message received 
* from the LoRa modem. 
* Also used to unpack the fire signal, which has an identical structure.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - message size in bits INPUT
* @param - pointer to resulting alarm (or fire signal) message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackAlarmSignal(CO_MessagePayloadPhyDataInd_t *pPhyDataInd, 
                                       AlarmSignal_t *pAlarmSignal)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ( (NULL == pPhyDataInd) || (NULL == pAlarmSignal) )
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();
      
      MC_PUP_ShiftRegExtractFromRight(&pAlarmSignal->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(ALARM_SIGNAL_PAYLOAD_PADDING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pAlarmSignal->AlarmActive, ALARM_SIGNAL_ACTIVE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pAlarmSignal->Zone, ALARM_SIGNAL_ZONE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pAlarmSignal->SensorValue, ALARM_SIGNAL_SENSOR_VALUE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pAlarmSignal->RUChannel, 
                                       ALARM_SIGNAL_RU_CHANNEL_INDEX_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pAlarmSignal->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pAlarmSignal->Header.SourceAddress, 
                                       DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pAlarmSignal->Header.DestinationAddress, 
                                       DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pAlarmSignal->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pAlarmSignal->Header.MACSourceAddress, 
                                       DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pAlarmSignal->Header.MACDestinationAddress, 
                                       DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pAlarmSignal->Header.FrameType, DATA_FRAME_TYPE_SIZE);


      MC_PUP_ASSERT(pAlarmSignal->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pAlarmSignal->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pAlarmSignal->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pAlarmSignal->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pAlarmSignal->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pAlarmSignal->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT((pAlarmSignal->Header.MessageType == APP_MSG_TYPE_FIRE_SIGNAL_E) ||
                     (pAlarmSignal->Header.MessageType == APP_MSG_TYPE_ALARM_SIGNAL_E));
      MC_PUP_ASSERT(pAlarmSignal->RUChannel < 128);//max for 7 bits
      MC_PUP_ASSERT(pAlarmSignal->SensorValue < MAX_FOR_8_BITS);//max for 8 bits
   }



   return status;
}

/*************************************************************************************/
/**
* MC_PUP_PackAck
* Function to pack a mesh protocol Ack message into a Phy_Data_Req message to be 
* passed to the LoRa modem.
*
* @param - pointer to Ack message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackAck(FrameAcknowledge_t *pAck, CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ( (NULL == pPhyDataReq) || (NULL == pAck) )
   {
      return ERR_INVALID_POINTER_E;
   }
   /* Range check the ack signal values */
   MC_PUP_ASSERT(pAck->FrameType == FRAME_TYPE_ACKNOWLEDGEMENT_E);
   MC_PUP_ASSERT(pAck->MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pAck->MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits


   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pAck->FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pAck->MACDestinationAddress, DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pAck->MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pAck->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;   
   
   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackAck
* Function to unpack a mesh protocol Ack message from a Phy_Data_Req message received 
* from the LoRa modem. 
*
* @param - pointer to Phy_Data_Req message INPUT
* @param - message size in bits INPUT
* @param - pointer to resulting Ack message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackAck(CO_MessagePayloadPhyDataInd_t *pPhyDataInd, FrameAcknowledge_t *pAck)
{
   uint32_t size = 0;

   /* check input parameters */
   if ( (NULL == pPhyDataInd) || (NULL == pAck) )
   {
      return ERR_INVALID_POINTER_E;
   }

   MC_PUP_ShiftRegInit();

   /* load shift register with input */
   size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
   memcpy(shiftReg, &pPhyDataInd->Data, size);
   messageLengthBits = ACKNOWLEDGEMENT_SIZE;

   MC_PUP_ShiftRegAlignRight();
   
   MC_PUP_ShiftRegExtractFromRight(&pAck->SystemId, DATA_SYSTEM_ID_SIZE);
    MC_PUP_ShiftRegExtractFromRight(&pAck->MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegExtractFromRight(&pAck->MACDestinationAddress, DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegExtractFromRight(&pAck->FrameType, DATA_FRAME_TYPE_SIZE);

   /* Range check the ack signal values */
   MC_PUP_ASSERT(pAck->FrameType == FRAME_TYPE_ACKNOWLEDGEMENT_E);
   MC_PUP_ASSERT(pAck->MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pAck->MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits

   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_PackFaultSignal
* Function to pack a mesh protocol fault Signal message into a Phy_Data_Req message to be
* passed to the LoRa modem.
*
* @param - pointer to fault Signal message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackFaultSignal(FaultSignal_t *pFaultSignal, 
                                    CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataReq) || (NULL == pFaultSignal))
   {
      return ERR_INVALID_POINTER_E;
   }
   CO_PRINT_B_1(DBG_INFO_E,"MC_PUP_PackFaultSignal OverallFault=%d\r\n", pFaultSignal->OverallFault);
   /* Range check the alarm signal values */
   MC_PUP_ASSERT(pFaultSignal->Header.FrameType == FRAME_TYPE_DATA_E);
   MC_PUP_ASSERT(pFaultSignal->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pFaultSignal->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pFaultSignal->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pFaultSignal->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pFaultSignal->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pFaultSignal->Header.MessageType == APP_MSG_TYPE_FAULT_SIGNAL_E);//Fault Signal
   MC_PUP_ASSERT(pFaultSignal->RUChannelIndex < CO_CHANNEL_MAX_E);
   MC_PUP_ASSERT(pFaultSignal->Value < 2);//max for 1 bit
   MC_PUP_ASSERT(pFaultSignal->OverallFault < 4);//max for 2 bits
   MC_PUP_ASSERT(pFaultSignal->FaultType < FAULT_SIGNAL_MAX_E);

   /*build the message in the shift register */
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pFaultSignal->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pFaultSignal->Header.MACDestinationAddress,
      DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pFaultSignal->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pFaultSignal->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pFaultSignal->Header.DestinationAddress,
      DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pFaultSignal->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pFaultSignal->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pFaultSignal->RUChannelIndex, FAULT_SIGNAL_RU_CHANNEL_INDEX_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pFaultSignal->Value, FAULT_SIGNAL_FAULT_VALUE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pFaultSignal->FaultType, FAULT_SIGNAL_FAULT_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pFaultSignal->OverallFault, FAULT_SIGNAL_OVERALL_FAULT_SIZE);
   MC_PUP_ShiftRegPadOnRight(FAULT_SIGNAL_PAYLOAD_PADDING_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pFaultSignal->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;


   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackFaultSignal
* Function to unpack a mesh protocol fault message from a Phy_Data_Req message received
* from the LoRa modem.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting fault signal message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackFaultSignal(CO_MessagePayloadPhyDataInd_t *pPhyDataInd, 
                                       FaultSignal_t *pFaultSignal)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataInd) || (NULL == pFaultSignal))
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pFaultSignal->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(FAULT_SIGNAL_PAYLOAD_PADDING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pFaultSignal->OverallFault, 
                                       FAULT_SIGNAL_OVERALL_FAULT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pFaultSignal->FaultType, 
                                       FAULT_SIGNAL_FAULT_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pFaultSignal->Value, 
                                       FAULT_SIGNAL_FAULT_VALUE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pFaultSignal->RUChannelIndex, 
                                       FAULT_SIGNAL_RU_CHANNEL_INDEX_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pFaultSignal->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pFaultSignal->Header.SourceAddress, 
                                       DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pFaultSignal->Header.DestinationAddress, 
                                       DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pFaultSignal->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pFaultSignal->Header.MACSourceAddress, 
                                       DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pFaultSignal->Header.MACDestinationAddress, 
                                       DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pFaultSignal->Header.FrameType, DATA_FRAME_TYPE_SIZE);


      /* Range check the alarm signal values */
      MC_PUP_ASSERT(pFaultSignal->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pFaultSignal->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pFaultSignal->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pFaultSignal->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pFaultSignal->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pFaultSignal->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pFaultSignal->RUChannelIndex < CO_CHANNEL_MAX_E);
      MC_PUP_ASSERT(pFaultSignal->Value < 2);//max for 1 bit
      MC_PUP_ASSERT(pFaultSignal->OverallFault < 4);//max for 2 bits
      MC_PUP_ASSERT(pFaultSignal->FaultType < FAULT_SIGNAL_MAX_E);
   }



   return status;
}

/*************************************************************************************/
/**
* MC_PUP_PackLogOnMsg
* Function to pack a mesh protocol Log On message into a Phy_Data_Req message to be
* passed to the LoRa modem.
*
* @param - pointer to fault Signal message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackLogOnMsg(LogOnMessage_t *pLogOnMsg,
   CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataReq) || (NULL == pLogOnMsg))
   {
      return ERR_INVALID_POINTER_E;
   }
   /* Range check the message values */
   MC_PUP_ASSERT(pLogOnMsg->Header.FrameType == FRAME_TYPE_DATA_E);
   MC_PUP_ASSERT(pLogOnMsg->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pLogOnMsg->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pLogOnMsg->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pLogOnMsg->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pLogOnMsg->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pLogOnMsg->Header.MessageType == APP_MSG_TYPE_LOGON_E);//Log On
   MC_PUP_ASSERT(pLogOnMsg->DeviceCombination < MAX_FOR_8_BITS);
   MC_PUP_ASSERT(pLogOnMsg->ZoneNumber < MAX_FOR_8_BITS);//max for 8 bits

   /*build the message in the shift register */
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pLogOnMsg->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLogOnMsg->Header.MACDestinationAddress,
      DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLogOnMsg->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLogOnMsg->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLogOnMsg->Header.DestinationAddress,
      DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLogOnMsg->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLogOnMsg->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLogOnMsg->SerialNumber, LOGON_MSG_SERIAL_NUMBER_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLogOnMsg->DeviceCombination, LOGON_MSG_DEVICE_COMBINATION_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLogOnMsg->ZoneNumber, LOGON_MSG_ZONE_NUMBER_SIZE);
   MC_PUP_ShiftRegPadOnRight(LOGON_MSG_PADDING_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLogOnMsg->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;


   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackLogOnMsg
* Function to unpack a mesh Log On message from a Phy_Data_Ind message received
* from the LoRa modem.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting fault signal message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackLogOnMsg(CO_MessagePayloadPhyDataInd_t *pPhyDataInd, 
                                    LogOnMessage_t *pLogOnMsg)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataInd) || (NULL == pLogOnMsg))
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pLogOnMsg->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(LOGON_MSG_PADDING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLogOnMsg->ZoneNumber, LOGON_MSG_ZONE_NUMBER_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLogOnMsg->DeviceCombination, 
                                       LOGON_MSG_DEVICE_COMBINATION_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLogOnMsg->SerialNumber, LOGON_MSG_SERIAL_NUMBER_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLogOnMsg->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLogOnMsg->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLogOnMsg->Header.DestinationAddress, 
                                       DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLogOnMsg->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLogOnMsg->Header.MACSourceAddress, 
                                       DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLogOnMsg->Header.MACDestinationAddress, 
                                       DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLogOnMsg->Header.FrameType, DATA_FRAME_TYPE_SIZE);

      /* Range check the message values */
      MC_PUP_ASSERT(pLogOnMsg->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pLogOnMsg->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pLogOnMsg->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pLogOnMsg->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pLogOnMsg->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pLogOnMsg->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pLogOnMsg->Header.MessageType == APP_MSG_TYPE_LOGON_E);//Log On
//      MC_PUP_ASSERT(pLogOnMsg->DeviceCombination < DC_NUMBER_OF_DEVICE_COMBINATIONS_E);
      MC_PUP_ASSERT(pLogOnMsg->ZoneNumber < MAX_DEVICES_PER_SYSTEM);//max number of RBUs in a system
   }

   return status;
}


/*************************************************************************************/
/**
* MC_PUP_PackStatusIndicationMsg
* Function to pack a mesh protocol Status Indication message into a Phy_Data_Req message 
* to be passed to the LoRa modem.
*
* @param - pointer to status message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackStatusIndicationMsg(StatusIndicationMessage_t *pStatusInd,
   CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataReq) || (NULL == pStatusInd))
   {
      return ERR_INVALID_POINTER_E;
   }
   
   /* Range check the signal values */
   MC_PUP_ASSERT(pStatusInd->Header.FrameType == FRAME_TYPE_DATA_E);
   MC_PUP_ASSERT(pStatusInd->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pStatusInd->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pStatusInd->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pStatusInd->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pStatusInd->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pStatusInd->Header.MessageType == APP_MSG_TYPE_STATUS_INDICATION_E);//Status indication
   MC_PUP_ASSERT(pStatusInd->PrimaryParentID < MAX_DEVICES_PER_SYSTEM);
   MC_PUP_ASSERT(pStatusInd->SecondaryParentID < MAX_DEVICES_PER_SYSTEM);
   MC_PUP_ASSERT(pStatusInd->AverageRssiPrimaryParent < MAX_FOR_9_BITS);
   MC_PUP_ASSERT(pStatusInd->AverageRssiSecondaryParent < MAX_FOR_9_BITS);
   MC_PUP_ASSERT(pStatusInd->Rank < 64);//max for 6 bits
   MC_PUP_ASSERT(pStatusInd->Event < 64);//max for 6 bits
   MC_PUP_ASSERT(pStatusInd->EventData < 512);//max for 9 bits
   MC_PUP_ASSERT(pStatusInd->OverallFault < 4);//max for 2 bits
   
   
   /*build the message in the shift register */
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pStatusInd->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStatusInd->Header.MACDestinationAddress,
      DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStatusInd->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStatusInd->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStatusInd->Header.DestinationAddress,
      DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStatusInd->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStatusInd->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStatusInd->PrimaryParentID, STATUS_IND_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStatusInd->SecondaryParentID, STATUS_IND_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStatusInd->AverageRssiPrimaryParent, STATUS_IND_AVERAGE_RSSI_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStatusInd->AverageRssiSecondaryParent, STATUS_IND_AVERAGE_RSSI_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStatusInd->Rank, STATUS_IND_RANK_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStatusInd->Event, STATUS_IND_EVENT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStatusInd->EventData, STATUS_IND_EVENT_DATA_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStatusInd->OverallFault, STATUS_IND_OVERALL_FAULT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStatusInd->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;


   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackStatusIndicationMsg
* Function to unpack a mesh Status Indication message from a Phy_Data_Ind message received
* from the LoRa modem.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting fault signal message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackStatusIndicationMsg(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
   StatusIndicationMessage_t *pStatusInd)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataInd) || (NULL == pStatusInd))
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pStatusInd->SystemId, DATA_SYSTEM_ID_SIZE);
      //No padding
      MC_PUP_ShiftRegExtractFromRight(&pStatusInd->OverallFault, STATUS_IND_OVERALL_FAULT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStatusInd->EventData, STATUS_IND_EVENT_DATA_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStatusInd->Event, STATUS_IND_EVENT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStatusInd->Rank, STATUS_IND_RANK_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStatusInd->AverageRssiSecondaryParent, STATUS_IND_AVERAGE_RSSI_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStatusInd->AverageRssiPrimaryParent, STATUS_IND_AVERAGE_RSSI_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStatusInd->SecondaryParentID, STATUS_IND_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStatusInd->PrimaryParentID, STATUS_IND_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStatusInd->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStatusInd->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStatusInd->Header.DestinationAddress, 
                                       DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStatusInd->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStatusInd->Header.MACSourceAddress, 
                                       DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStatusInd->Header.MACDestinationAddress, 
                                       DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStatusInd->Header.FrameType, DATA_FRAME_TYPE_SIZE);
 
   
      /* Range check the alarm signal values */
      MC_PUP_ASSERT(pStatusInd->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pStatusInd->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pStatusInd->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pStatusInd->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pStatusInd->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pStatusInd->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pStatusInd->Header.MessageType == APP_MSG_TYPE_STATUS_INDICATION_E);//Status indication
      MC_PUP_ASSERT(pStatusInd->PrimaryParentID < MAX_DEVICES_PER_SYSTEM);
      MC_PUP_ASSERT(pStatusInd->SecondaryParentID < MAX_DEVICES_PER_SYSTEM);
      MC_PUP_ASSERT(pStatusInd->AverageRssiPrimaryParent < MAX_FOR_9_BITS);
      MC_PUP_ASSERT(pStatusInd->AverageRssiSecondaryParent < MAX_FOR_9_BITS);
      MC_PUP_ASSERT(pStatusInd->Rank < 64);//max for 6 bits
      MC_PUP_ASSERT(pStatusInd->Event < 64);//max for 6 bits
      MC_PUP_ASSERT(pStatusInd->EventData < 512);//max for 9 bits
      MC_PUP_ASSERT(pStatusInd->OverallFault < 4);//max for 2 bits
   }

   return status;
}


/*************************************************************************************/
/**
* MC_PUP_PackRouteAddMsg
* Function to pack a mesh protocol Route Add message into a Phy_Data_Req message
* to be passed to the LoRa modem.
*
* @param - pointer to Route Add message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackRouteAddMsg(RouteAddMessage_t *pRouteAdd,
   CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataReq) || (NULL == pRouteAdd))
   {
      return ERR_INVALID_POINTER_E;
   }
   /* Range check the message values */
   MC_PUP_ASSERT(pRouteAdd->Header.FrameType == FRAME_TYPE_DATA_E);
   MC_PUP_ASSERT(pRouteAdd->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pRouteAdd->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pRouteAdd->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pRouteAdd->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pRouteAdd->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pRouteAdd->Header.MessageType == APP_MSG_TYPE_ROUTE_ADD_E);//Route Add
   MC_PUP_ASSERT(pRouteAdd->Rank < 64);//max for 6 bits
   MC_PUP_ASSERT(pRouteAdd->IsPrimary < 2);//max for 1 bit
   MC_PUP_ASSERT(pRouteAdd->Zone < 256);//max for 8 bits
                                               
   /*build the message in the shift register */
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pRouteAdd->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAdd->Header.MACDestinationAddress,
      DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAdd->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAdd->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAdd->Header.DestinationAddress,
      DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAdd->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAdd->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAdd->Rank, ROUTE_ADD_MSG_RANK_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAdd->IsPrimary, ROUTE_ADD_MSG_ISPRIMARY_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAdd->Zone, NUM_OF_ZONE_BITS);
   MC_PUP_ShiftRegPadOnRight(ROUTE_ADD_MSG_PADDING_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAdd->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;


   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackRouteAddMsg
* Function to unpack a mesh Route Add message from a Phy_Data_Ind message received
* from the LoRa modem.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting Route Add message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackRouteAddMsg(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
   RouteAddMessage_t *pRouteAdd)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataInd) || (NULL == pRouteAdd))
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pRouteAdd->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(ROUTE_ADD_MSG_PADDING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAdd->Zone, NUM_OF_ZONE_BITS);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAdd->IsPrimary, ROUTE_ADD_MSG_ISPRIMARY_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAdd->Rank, ROUTE_ADD_MSG_RANK_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAdd->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAdd->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAdd->Header.DestinationAddress, 
                                             DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAdd->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAdd->Header.MACSourceAddress, 
                                             DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAdd->Header.MACDestinationAddress, 
                                             DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAdd->Header.FrameType, DATA_FRAME_TYPE_SIZE);

      /* Range check the message values */
      MC_PUP_ASSERT(pRouteAdd->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pRouteAdd->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pRouteAdd->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pRouteAdd->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pRouteAdd->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pRouteAdd->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pRouteAdd->Header.MessageType == APP_MSG_TYPE_ROUTE_ADD_E);//Route Add
      MC_PUP_ASSERT(pRouteAdd->Rank < 64);//max for 6 bits
      MC_PUP_ASSERT(pRouteAdd->IsPrimary < 2);//max for 1 bit
      MC_PUP_ASSERT(pRouteAdd->Zone < 256);//max for 8 bits
   }

   return status;
}

/*************************************************************************************/
/**
* MC_PUP_PackRouteAddResponseMsg
* Function to pack a mesh protocol Route Add Response message into a Phy_Data_Req message
* to be passed to the LoRa modem.
*
* @param - pointer to Route Add message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackRouteAddResponseMsg(RouteAddResponseMessage_t *pRouteAddResponse,
   CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataReq) || (NULL == pRouteAddResponse))
   {
      return ERR_INVALID_POINTER_E;
   }
   /* Range check the message values */
   MC_PUP_ASSERT(pRouteAddResponse->Header.FrameType == FRAME_TYPE_DATA_E);
   MC_PUP_ASSERT(pRouteAddResponse->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pRouteAddResponse->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pRouteAddResponse->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pRouteAddResponse->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pRouteAddResponse->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pRouteAddResponse->Header.MessageType == APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E);//Route Add Response
   MC_PUP_ASSERT(pRouteAddResponse->Accepted < 2);//max for 1 bits
   MC_PUP_ASSERT(pRouteAddResponse->DayNight < 2);//max for 1 bits
   MC_PUP_ASSERT(pRouteAddResponse->GlobalDelayEnabled < 2);//max for 1 bits
   MC_PUP_ASSERT(pRouteAddResponse->ZoneLowerUpper < 2);//max for 1 bits
   MC_PUP_ASSERT(pRouteAddResponse->ZoneMapHalfWord < 65536);//max for 16 bits
   MC_PUP_ASSERT(pRouteAddResponse->FaultsEnabled < 2);//max for 1 bits
   MC_PUP_ASSERT(pRouteAddResponse->GlobalDelayOverride < 2);//max for 1 bits
                                               
   /*build the message in the shift register */
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->Header.MACDestinationAddress,
      DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->Header.DestinationAddress,
      DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->Accepted, ROUTE_ADD_RESPONSE_MSG_ACCEPTED_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->DayNight, ROUTE_ADD_RESPONSE_MSG_DAY_NIGHT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->GlobalDelayEnabled, ROUTE_ADD_RESPONSE_MSG_GLOBAL_DELAY_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->ZoneEnabled, ROUTE_ADD_RESPONSE_MSG_ZONE_ENABLED_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->ZoneLowerUpper, ROUTE_ADD_RESPONSE_MSG_ZONE_LOWER_UPPER_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->ZoneMapWord, ROUTE_ADD_RESPONSE_MSG_ZONE_MAP_WORD_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->ZoneMapHalfWord, ROUTE_ADD_RESPONSE_MSG_ZONE_MAP_HALFWORD_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->FaultsEnabled, ROUTE_ADD_RESPONSE_MSG_FAULTS_ENABLED_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->GlobalDelayOverride, ROUTE_ADD_RESPONSE_MSG_GLOBAL_DELAY_OVERRIDE_SIZE);
   MC_PUP_ShiftRegPadOnRight(ROUTE_ADD_RESPONSE_MSG_PADDING_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteAddResponse->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();


   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;


   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackRouteAddResponseMsg
* Function to unpack a mesh Route Add Response message from a Phy_Data_Ind message received
* from the LoRa modem.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting Route Add message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackRouteAddResponseMsg(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
   RouteAddResponseMessage_t *pRouteAddresponse)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataInd) || (NULL == pRouteAddresponse))
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(ROUTE_ADD_RESPONSE_MSG_PADDING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->GlobalDelayOverride, ROUTE_ADD_RESPONSE_MSG_GLOBAL_DELAY_OVERRIDE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->FaultsEnabled, ROUTE_ADD_RESPONSE_MSG_FAULTS_ENABLED_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->ZoneMapHalfWord, ROUTE_ADD_RESPONSE_MSG_ZONE_MAP_HALFWORD_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->ZoneMapWord, ROUTE_ADD_RESPONSE_MSG_ZONE_MAP_WORD_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->ZoneLowerUpper, ROUTE_ADD_RESPONSE_MSG_ZONE_LOWER_UPPER_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->ZoneEnabled, ROUTE_ADD_RESPONSE_MSG_ZONE_ENABLED_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->GlobalDelayEnabled, ROUTE_ADD_RESPONSE_MSG_GLOBAL_DELAY_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->DayNight, ROUTE_ADD_RESPONSE_MSG_DAY_NIGHT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->Accepted, ROUTE_ADD_RESPONSE_MSG_ACCEPTED_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->Header.DestinationAddress, 
                                             DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->Header.MACSourceAddress, 
                                             DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->Header.MACDestinationAddress, 
                                             DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteAddresponse->Header.FrameType, DATA_FRAME_TYPE_SIZE);

      /* Range check the message values */
      MC_PUP_ASSERT(pRouteAddresponse->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pRouteAddresponse->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pRouteAddresponse->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pRouteAddresponse->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pRouteAddresponse->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pRouteAddresponse->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pRouteAddresponse->Header.MessageType == APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E);//Route Add
      MC_PUP_ASSERT(pRouteAddresponse->Accepted < 2);//max for 1 bit
      MC_PUP_ASSERT(pRouteAddresponse->DayNight < 2);//max for 1 bits
      MC_PUP_ASSERT(pRouteAddresponse->GlobalDelayEnabled < 2);//max for 1 bits
      MC_PUP_ASSERT(pRouteAddresponse->ZoneMapHalfWord < 65536);//max for 16 bits
      MC_PUP_ASSERT(pRouteAddresponse->FaultsEnabled < 2);//max for 1 bits
      MC_PUP_ASSERT(pRouteAddresponse->GlobalDelayOverride < 2);//max for 1 bits
   }

   return status;
}

/*************************************************************************************/
/**
* MC_PUP_PackRouteDropMsg
* Function to pack a mesh protocol Route Drop message into a Phy_Data_Req message
* to be passed to the LoRa modem.
*
* @param - pointer to Route Drop message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackRouteDropMsg(RouteDropMessage_t *pRouteDrop,
   CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataReq) || (NULL == pRouteDrop))
   {
      return ERR_INVALID_POINTER_E;
   }
   /* Range check the message values */
   MC_PUP_ASSERT(pRouteDrop->Header.FrameType == FRAME_TYPE_DATA_E);
   MC_PUP_ASSERT(pRouteDrop->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pRouteDrop->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pRouteDrop->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pRouteDrop->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pRouteDrop->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pRouteDrop->Header.MessageType == APP_MSG_TYPE_ROUTE_DROP_E);//Route Drop
   MC_PUP_ASSERT(pRouteDrop->Reason < MAX_FOR_8_BITS);//max for 8 bits

   /*build the message in the shift register */
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pRouteDrop->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteDrop->Header.MACDestinationAddress,
      DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteDrop->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteDrop->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteDrop->Header.DestinationAddress,
      DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteDrop->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteDrop->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteDrop->Reason, ROUTE_DROP_MSG_REASON_SIZE);
   MC_PUP_ShiftRegPadOnRight(ROUTE_DROP_MSG_PADDING_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRouteDrop->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;


   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackRouteDropMsg
* Function to unpack a mesh Route Add message from a Phy_Data_Ind message received
* from the LoRa modem.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting Route Drop message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackRouteDropMsg(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
   RouteDropMessage_t *pRouteDrop)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataInd) || (NULL == pRouteDrop))
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pRouteDrop->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(ROUTE_DROP_MSG_PADDING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteDrop->Reason, ROUTE_DROP_MSG_REASON_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteDrop->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteDrop->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteDrop->Header.DestinationAddress, 
                                                DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteDrop->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteDrop->Header.MACSourceAddress, 
                                                DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteDrop->Header.MACDestinationAddress, 
                                                DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRouteDrop->Header.FrameType, DATA_FRAME_TYPE_SIZE);

      /* Range check the message values */
      MC_PUP_ASSERT(pRouteDrop->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pRouteDrop->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pRouteDrop->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pRouteDrop->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pRouteDrop->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pRouteDrop->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pRouteDrop->Header.MessageType == APP_MSG_TYPE_ROUTE_DROP_E);//Route Drop
      MC_PUP_ASSERT(pRouteDrop->Reason < MAX_FOR_8_BITS);//max for 8 bits
   }

   return status;
}


/*************************************************************************************/
/**
* MC_PUP_PackCommandMessage
* Function to pack a mesh protocol Command message into a Phy_Data_Req message
* to be passed to the LoRa modem.
*
* @param - pointer to Command message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackCommandMessage(CommandMessage_t *pCommand,
   CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;
   uint32_t paddingSize = 0;

   /* check input parameters */
   if ((NULL == pPhyDataReq) || (NULL == pCommand))
   {
      return ERR_INVALID_POINTER_E;
   }
   /* Range check the message values */
   MC_PUP_ASSERT(pCommand->Header.FrameType == FRAME_TYPE_DATA_E);
   MC_PUP_ASSERT(pCommand->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pCommand->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pCommand->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pCommand->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pCommand->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT((pCommand->Header.MessageType == APP_MSG_TYPE_COMMAND_E) || //Command Message
                 (pCommand->Header.MessageType == APP_MSG_TYPE_RESPONSE_E)) //Response Message
   MC_PUP_ASSERT(pCommand->ParameterType < PARAM_TYPE_MAX_E);
   MC_PUP_ASSERT(pCommand->ReadWrite < 2);// only 1 or 0 is valid
   MC_PUP_ASSERT(pCommand->TransactionID < 8);//max for 3 bits
   MC_PUP_ASSERT(pCommand->P1 < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pCommand->P2 < MAX_FOR_8_BITS);//max for 8 bits


   /*build the message in the shift register */
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pCommand->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pCommand->Header.MACDestinationAddress,
      DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pCommand->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pCommand->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pCommand->Header.DestinationAddress,
      DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pCommand->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pCommand->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pCommand->ParameterType, RESP_MESSAGE_PARAMETER_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pCommand->ReadWrite, RESP_MESSAGE_READWRITE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pCommand->TransactionID, RESP_MESSAGE_TRANSACTIONID_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pCommand->P1, RESP_MESSAGE_PARAMETER_P_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pCommand->P2, RESP_MESSAGE_PARAMETER_P_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pCommand->Value, RESP_MESSAGE_VALUE_TYPE_SIZE);
   MC_PUP_ShiftRegPadOnRight(paddingSize);
   MC_PUP_ShiftRegLoadOnRight(pCommand->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;


   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackCommandMessage
* Function to unpack a Command message from a Phy_Data_Ind message received
* from the LoRa modem.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting Command message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackCommandMessage(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
   CommandMessage_t *pCommand)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataInd) || (NULL == pCommand))
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pCommand->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pCommand->Value, RESP_MESSAGE_VALUE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pCommand->P2, RESP_MESSAGE_PARAMETER_P_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pCommand->P1, RESP_MESSAGE_PARAMETER_P_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pCommand->TransactionID, RESP_MESSAGE_TRANSACTIONID_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pCommand->ReadWrite, RESP_MESSAGE_READWRITE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pCommand->ParameterType, RESP_MESSAGE_PARAMETER_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pCommand->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pCommand->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pCommand->Header.DestinationAddress, 
                                                DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pCommand->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pCommand->Header.MACSourceAddress, 
                                                DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pCommand->Header.MACDestinationAddress, 
                                                DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pCommand->Header.FrameType, DATA_FRAME_TYPE_SIZE);

      /* Range check the message values */
      MC_PUP_ASSERT(pCommand->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pCommand->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pCommand->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pCommand->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pCommand->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pCommand->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT((pCommand->Header.MessageType == APP_MSG_TYPE_COMMAND_E) || //Command Message
                    (pCommand->Header.MessageType == APP_MSG_TYPE_RESPONSE_E)) //Response Message
      MC_PUP_ASSERT(pCommand->ParameterType < PARAM_TYPE_MAX_E);
      MC_PUP_ASSERT(pCommand->ReadWrite < 2);// only 1 or 0 is valid
      MC_PUP_ASSERT(pCommand->TransactionID < 8);//max for 3 bits
      MC_PUP_ASSERT(pCommand->P1 < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pCommand->P2 < MAX_FOR_8_BITS);//max for 8 bits
   }

   return status;
}

/*************************************************************************************/
/**
* MC_PUP_PackResponseMessage
* Function to pack a mesh protocol Response message into a Phy_Data_Req message
* to be passed to the LoRa modem.  The message structure is identical to the 
* Command message so this function calls the command packing function.
*
* @param - pointer to Response message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackResponseMessage(ResponseMessage_t *pResponse,
   CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   return MC_PUP_PackCommandMessage(pResponse, pPhyDataReq);
}

/*************************************************************************************/
/**
* MC_PUP_UnpackResponseMessage
* Function to unpack a Response message from a Phy_Data_Ind message received
* from the LoRa modem. The message structure is identical to the 
* Command message so this function calls the command unpacking function.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting Response message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackResponseMessage(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
   ResponseMessage_t *pResponse)
{
   return MC_PUP_UnpackCommandMessage( pPhyDataInd, pResponse);
}

/*************************************************************************************/
/**
* MC_PUP_PackOutputSignal
* Function to pack a mesh protocol Output Signal message into a Phy_Data_Req message to be
* passed to the LoRa modem.
*
* @param - pointer to Output Signal message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackOutputSignal(OutputSignal_t *pOutputSignal,
   CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataReq) || (NULL == pOutputSignal))
   {
      return ERR_INVALID_POINTER_E;
   }
   /* Range check the message values */
   MC_PUP_ASSERT(pOutputSignal->Header.FrameType == FRAME_TYPE_DATA_E);
   MC_PUP_ASSERT(pOutputSignal->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pOutputSignal->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pOutputSignal->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pOutputSignal->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pOutputSignal->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pOutputSignal->Header.MessageType == APP_MSG_TYPE_OUTPUT_SIGNAL_E);
   MC_PUP_ASSERT(pOutputSignal->Zone < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pOutputSignal->OutputChannel < CO_CHANNEL_MAX_E);
   MC_PUP_ASSERT(pOutputSignal->OutputProfile < CO_PROFILE_MAX_E);
   MC_PUP_ASSERT(pOutputSignal->OutputsActivated < 65536);//max for 16 bits
   MC_PUP_ASSERT(pOutputSignal->OutputDuration < 65356);//max for 16 bits

   /*build the message in the shift register*/
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Header.MACDestinationAddress,
      DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Header.DestinationAddress,
      DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Zone, OUTPUT_SIGNAL_ZONE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->OutputChannel, OUTPUT_SIGNAL_OUTPUT_CHANNEL_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->OutputProfile, OUTPUT_SIGNAL_OUTPUT_PROFILE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->OutputsActivated, OUTPUT_SIGNAL_OUTPUTS_ACTIVATED_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->OutputDuration, OUTPUT_SIGNAL_OUTPUT_DURATION_SIZE);
   MC_PUP_ShiftRegPadOnRight(OUTPUT_SIGNAL_PAYLOAD_PADDING_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;


   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackoutputSignal
* Function to unpack a mesh protocol output message from a Phy_Data_Ind message received
* from the LoRa modem.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting output signal message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackOutputSignal(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
   OutputSignal_t *pOutputSignal)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataInd) || (NULL == pOutputSignal))
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(OUTPUT_SIGNAL_PAYLOAD_PADDING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->OutputDuration, OUTPUT_SIGNAL_OUTPUT_DURATION_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->OutputsActivated, OUTPUT_SIGNAL_OUTPUTS_ACTIVATED_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->OutputProfile, OUTPUT_SIGNAL_OUTPUT_PROFILE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->OutputChannel, OUTPUT_SIGNAL_OUTPUT_CHANNEL_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Zone, OUTPUT_SIGNAL_ZONE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Header.SourceAddress,
         DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Header.DestinationAddress,
         DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Header.MACSourceAddress,
         DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Header.MACDestinationAddress,
         DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Header.FrameType, DATA_FRAME_TYPE_SIZE);

      /* Range check the message values */
      MC_PUP_ASSERT(pOutputSignal->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pOutputSignal->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pOutputSignal->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pOutputSignal->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pOutputSignal->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pOutputSignal->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pOutputSignal->Header.MessageType == APP_MSG_TYPE_OUTPUT_SIGNAL_E);//output signal
      MC_PUP_ASSERT(pOutputSignal->OutputChannel < CO_CHANNEL_MAX_E);
      MC_PUP_ASSERT(pOutputSignal->OutputProfile < CO_PROFILE_MAX_E);
      MC_PUP_ASSERT(pOutputSignal->OutputsActivated < 65536);//max for 16 bits
      MC_PUP_ASSERT(pOutputSignal->OutputDuration < MAX_FOR_8_BITS);//max for 8 bits
   }

   return status;
}

/*************************************************************************************/
/**
* MC_PUP_PackAlarmOutputState
* Function to pack a mesh protocol Alarm Output State message into a Phy_Data_Req message to be
* passed to the LoRa modem.
*
* @param - pointer to Alarm Output State message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackAlarmOutputState(AlarmOutputState_t *pOutputSignal, CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataReq) || (NULL == pOutputSignal))
   {
      return ERR_INVALID_POINTER_E;
   }
   /* Range check the message values */
   MC_PUP_ASSERT(pOutputSignal->Header.FrameType == FRAME_TYPE_DATA_E);
   MC_PUP_ASSERT(pOutputSignal->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pOutputSignal->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pOutputSignal->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pOutputSignal->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pOutputSignal->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pOutputSignal->Header.MessageType == APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E);
   MC_PUP_ASSERT(pOutputSignal->Silenceable < MAX_FOR_9_BITS);//max for 9 bits
   MC_PUP_ASSERT(pOutputSignal->Unsilenceable < MAX_FOR_9_BITS);//max for 9 bits
   MC_PUP_ASSERT(pOutputSignal->DelayMask < MAX_FOR_9_BITS);//max for 9 bits

   /*build the message in the shift register*/
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Header.MACDestinationAddress,
      DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Header.DestinationAddress,
      DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Silenceable, ALARM_OUTPUT_STATE_PROFILE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->Unsilenceable, ALARM_OUTPUT_STATE_PROFILE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->DelayMask, ALARM_OUTPUT_STATE_PROFILE_SIZE);
   MC_PUP_ShiftRegPadOnRight(ALARM_OUTPUT_STATE_PAYLOAD_PADDING_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pOutputSignal->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;


   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackAlarmOutputState
* Function to unpack a mesh protocol alarm output state from a Phy_Data_Ind message received
* from the LoRa modem.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting alarm output state message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackAlarmOutputState(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       AlarmOutputState_t *pOutputSignal)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataInd) || (NULL == pOutputSignal))
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(ALARM_OUTPUT_STATE_PAYLOAD_PADDING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->DelayMask, ALARM_OUTPUT_STATE_PROFILE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Unsilenceable, ALARM_OUTPUT_STATE_PROFILE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Silenceable, ALARM_OUTPUT_STATE_PROFILE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Header.SourceAddress,
         DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Header.DestinationAddress,
         DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Header.MACSourceAddress,
         DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Header.MACDestinationAddress,
         DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pOutputSignal->Header.FrameType, DATA_FRAME_TYPE_SIZE);

      /* Range check the message values */
      MC_PUP_ASSERT(pOutputSignal->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pOutputSignal->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pOutputSignal->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pOutputSignal->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pOutputSignal->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pOutputSignal->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pOutputSignal->Header.MessageType == APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E);//output signal
      MC_PUP_ASSERT(pOutputSignal->Silenceable < MAX_FOR_9_BITS);//max for 9 bits
      MC_PUP_ASSERT(pOutputSignal->Unsilenceable < MAX_FOR_9_BITS);//max for 9 bits
      MC_PUP_ASSERT(pOutputSignal->DelayMask < MAX_FOR_9_BITS);//max for 9 bits
   }

   return status;
}




/*************************************************************************************/
/**
* MC_PUP_PackTestSignal
* Function to pack a Test Mode message to be passed to the LoRa modem.
*
* @param - pointer to Test Signal message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackTestSignal( FrameTestModeMessage_t *pTestSignal, 
                                    CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ( (NULL == pPhyDataReq) || (NULL == pTestSignal) )
   {
      return ERR_INVALID_POINTER_E;
   }
   /* Range check the alarm signal values */
   MC_PUP_ASSERT(pTestSignal->FrameType == FRAME_TYPE_TEST_MESSAGE_E);
   MC_PUP_ASSERT(pTestSignal->SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   
   uint32_t payload1 = MC_PUP_ReverseOrder(pTestSignal->Payload[0]);
   uint32_t payload2 = MC_PUP_ReverseOrder(pTestSignal->Payload[1]);
   uint32_t payload3 = MC_PUP_ReverseOrder(pTestSignal->Payload[2]);
   uint32_t payload4 = pTestSignal->Payload[3] & 0xff;

   /*build the message in the shift register */
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pTestSignal->FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pTestSignal->SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(payload1, 32);
   MC_PUP_ShiftRegLoadOnRight(payload2, 32);
   MC_PUP_ShiftRegLoadOnRight(payload3, 32);
   MC_PUP_ShiftRegLoadOnRight(payload4, 8);
   MC_PUP_ShiftRegPadOnRight(4);
   MC_PUP_ShiftRegLoadOnRight(pTestSignal->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;
   
   
   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackTestSignal
* Function to unpack a Test mode message from a Phy_Data_Req message received 
* from the LoRa modem. 
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting test message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackTestSignal(CO_MessagePayloadPhyDataInd_t *pPhyDataInd, 
                                       FrameTestModeMessage_t *pTestSignal)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ( (NULL == pPhyDataInd) || (NULL == pTestSignal) )
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {
      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();
      
      MC_PUP_ShiftRegExtractFromRight(&pTestSignal->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(4);
      MC_PUP_ShiftRegExtractFromRight(&pTestSignal->Payload[3], 8);
      MC_PUP_ShiftRegExtractFromRight(&pTestSignal->Payload[2], 32);
      MC_PUP_ShiftRegExtractFromRight(&pTestSignal->Payload[1], 32);
      MC_PUP_ShiftRegExtractFromRight(&pTestSignal->Payload[0], 32);
      MC_PUP_ShiftRegExtractFromRight(&pTestSignal->SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pTestSignal->FrameType, DATA_FRAME_TYPE_SIZE);

      pTestSignal->Payload[0] = MC_PUP_ReverseOrder(pTestSignal->Payload[0]);
      pTestSignal->Payload[1] = MC_PUP_ReverseOrder(pTestSignal->Payload[1]);
      pTestSignal->Payload[2] = MC_PUP_ReverseOrder(pTestSignal->Payload[2]);
      pTestSignal->Payload[3] &= 0xff;
      

      MC_PUP_ASSERT(pTestSignal->FrameType == FRAME_TYPE_TEST_MESSAGE_E);
      MC_PUP_ASSERT(pTestSignal->SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   }

   return status;
}

/*************************************************************************************/
/**
* MC_PUP_PackSetState
* Function to pack a mesh protocol State Signal message into a Phy_Data_Req message to be
* passed to the LoRa modem.
*
* @param - pointer to State Signal message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackSetState(SetStateMessage_t *pStateSignal,
   CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataReq) || (NULL == pStateSignal))
   {
      return ERR_INVALID_POINTER_E;
   }
   /* Range check the message values */
   MC_PUP_ASSERT(pStateSignal->Header.FrameType == FRAME_TYPE_DATA_E);
   MC_PUP_ASSERT(pStateSignal->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pStateSignal->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pStateSignal->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pStateSignal->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pStateSignal->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pStateSignal->Header.MessageType == APP_MSG_TYPE_STATE_SIGNAL_E);
   MC_PUP_ASSERT(pStateSignal->State < STATE_MAX_E);

   /*build the message in the shift register */
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pStateSignal->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStateSignal->Header.MACDestinationAddress,
      DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStateSignal->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStateSignal->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStateSignal->Header.DestinationAddress,
      DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStateSignal->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStateSignal->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStateSignal->State, STATE_SIGNAL_STATE_SIZE);
   MC_PUP_ShiftRegPadOnRight(STATE_SIGNAL_PAYLOAD_PADDING_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pStateSignal->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;


   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackSetState
* Function to unpack a mesh protocol State message from a Phy_Data_Ind message received
* from the LoRa modem.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting State signal message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackSetState(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
   SetStateMessage_t *pStateSignal)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataInd) || (NULL == pStateSignal))
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pStateSignal->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(STATE_SIGNAL_PAYLOAD_PADDING_SIZE);
      MC_PUP_ShiftRegExtractFromRight((uint32_t*)&pStateSignal->State, STATE_SIGNAL_STATE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStateSignal->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStateSignal->Header.SourceAddress,
         DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStateSignal->Header.DestinationAddress,
         DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStateSignal->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStateSignal->Header.MACSourceAddress,
         DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStateSignal->Header.MACDestinationAddress,
         DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pStateSignal->Header.FrameType, DATA_FRAME_TYPE_SIZE);

      /* Range check the message values */
      MC_PUP_ASSERT(pStateSignal->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pStateSignal->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pStateSignal->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pStateSignal->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pStateSignal->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pStateSignal->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pStateSignal->Header.MessageType == APP_MSG_TYPE_STATE_SIGNAL_E);//output signal
      MC_PUP_ASSERT(pStateSignal->State < STATE_MAX_E);
   }

   return status;
}


/*************************************************************************************/
/**
* MC_PUP_PackLoadBalanceMsg
* Function to pack a mesh protocol Load Balance message into a Phy_Data_Req message
* to be passed to the LoRa modem.
*
* @param - pointer to Load balance message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackLoadBalanceMsg(LoadBalanceMessage_t *pLoadBalance,
   CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataReq) || (NULL == pLoadBalance))
   {
      return ERR_INVALID_POINTER_E;
   }
   /* Range check the message values */
   MC_PUP_ASSERT(pLoadBalance->Header.FrameType == FRAME_TYPE_DATA_E);
   MC_PUP_ASSERT(pLoadBalance->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pLoadBalance->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pLoadBalance->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pLoadBalance->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pLoadBalance->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pLoadBalance->Header.MessageType == APP_MSG_TYPE_LOAD_BALANCE_E);//Route Add
                                               
   /*build the message in the shift register */
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pLoadBalance->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLoadBalance->Header.MACDestinationAddress,
      DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLoadBalance->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLoadBalance->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLoadBalance->Header.DestinationAddress,
      DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLoadBalance->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLoadBalance->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegPadOnRight(LOAD_BALANCE_MSG_PADDING_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLoadBalance->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;


   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackLoadBalanceMsg
* Function to unpack a mesh Load Balance message from a Phy_Data_Ind message received
* from the LoRa modem.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting Load Balance message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackLoadBalanceMsg(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
   LoadBalanceMessage_t *pLoadBalance)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataInd) || (NULL == pLoadBalance))
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pLoadBalance->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(LOAD_BALANCE_MSG_PADDING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLoadBalance->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLoadBalance->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLoadBalance->Header.DestinationAddress, 
                                             DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLoadBalance->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLoadBalance->Header.MACSourceAddress, 
                                             DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLoadBalance->Header.MACDestinationAddress, 
                                             DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLoadBalance->Header.FrameType, DATA_FRAME_TYPE_SIZE);

      /* Range check the message values */
      MC_PUP_ASSERT(pLoadBalance->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pLoadBalance->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pLoadBalance->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pLoadBalance->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pLoadBalance->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pLoadBalance->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pLoadBalance->Header.MessageType == APP_MSG_TYPE_LOAD_BALANCE_E);//Route Add
   }

   return status;
}

/*************************************************************************************/
/**
* MC_PUP_PackLoadBalanceResponseMsg
* Function to pack a mesh protocol Load Balance Response message into a Phy_Data_Req message
* to be passed to the LoRa modem.
*
* @param - pointer to Load Balance Response message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackLoadBalanceResponseMsg(LoadBalanceResponseMessage_t *pLoadBalanceResponse,
   CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataReq) || (NULL == pLoadBalanceResponse))
   {
      return ERR_INVALID_POINTER_E;
   }
   /* Range check the message values */
   MC_PUP_ASSERT(pLoadBalanceResponse->Header.FrameType == FRAME_TYPE_DATA_E);
   MC_PUP_ASSERT(pLoadBalanceResponse->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pLoadBalanceResponse->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pLoadBalanceResponse->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pLoadBalanceResponse->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pLoadBalanceResponse->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pLoadBalanceResponse->Header.MessageType == APP_MSG_TYPE_LOAD_BALANCE_RESPONSE_E);//Load Balance Response
   MC_PUP_ASSERT(pLoadBalanceResponse->Accepted < 2);//max for 1 bits
                                               
   /*build the message in the shift register */
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pLoadBalanceResponse->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLoadBalanceResponse->Header.MACDestinationAddress,
      DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLoadBalanceResponse->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLoadBalanceResponse->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLoadBalanceResponse->Header.DestinationAddress,
      DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLoadBalanceResponse->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLoadBalanceResponse->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLoadBalanceResponse->Accepted, LOAD_BALANCE_RESPONSE_MSG_ACCEPTED_SIZE);
   MC_PUP_ShiftRegPadOnRight(LOAD_BALANCE_RESPONSE_MSG_PADDING_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pLoadBalanceResponse->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;


   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackLoadBalanceResponseMsg
* Function to unpack a mesh Load Balance Response message from a Phy_Data_Ind message received
* from the LoRa modem.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting Load Balance Response message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackLoadBalanceResponseMsg(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
   LoadBalanceResponseMessage_t *pLoadBalanceResponse)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataInd) || (NULL == pLoadBalanceResponse))
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pLoadBalanceResponse->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(LOAD_BALANCE_RESPONSE_MSG_PADDING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLoadBalanceResponse->Accepted, LOAD_BALANCE_RESPONSE_MSG_ACCEPTED_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLoadBalanceResponse->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLoadBalanceResponse->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLoadBalanceResponse->Header.DestinationAddress, 
                                             DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLoadBalanceResponse->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLoadBalanceResponse->Header.MACSourceAddress, 
                                             DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLoadBalanceResponse->Header.MACDestinationAddress, 
                                             DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pLoadBalanceResponse->Header.FrameType, DATA_FRAME_TYPE_SIZE);

      /* Range check the message values */
      MC_PUP_ASSERT(pLoadBalanceResponse->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pLoadBalanceResponse->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pLoadBalanceResponse->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pLoadBalanceResponse->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pLoadBalanceResponse->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pLoadBalanceResponse->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pLoadBalanceResponse->Header.MessageType == APP_MSG_TYPE_LOAD_BALANCE_RESPONSE_E);//Load Balance Response
      MC_PUP_ASSERT(pLoadBalanceResponse->Accepted < 2);//max for 1 bit
   }

   return status;
}

/*************************************************************************************/
/**
* MC_PUP_PackRBUDisableMessage
* Function to pack a mesh protocol RBU Disable message into a Phy_Data_Req message to be
* passed to the LoRa modem.
*
* @param - pointer to RBU Disable data INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackRBUDisableMessage(RBUDisableMessage_t *pRBUDisableMsg,
   CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataReq) || (NULL == pRBUDisableMsg))
   {
      return ERR_INVALID_POINTER_E;
   }
   /* Range check the message values */
   MC_PUP_ASSERT(pRBUDisableMsg->Header.FrameType == FRAME_TYPE_DATA_E);
   MC_PUP_ASSERT(pRBUDisableMsg->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pRBUDisableMsg->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pRBUDisableMsg->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pRBUDisableMsg->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pRBUDisableMsg->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pRBUDisableMsg->Header.MessageType == APP_MSG_TYPE_RBU_DISABLE_E);
   MC_PUP_ASSERT(pRBUDisableMsg->UnitAddress < 512);//max for 9 bits

   /*build the message in the shift register */
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pRBUDisableMsg->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRBUDisableMsg->Header.MACDestinationAddress,
      DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRBUDisableMsg->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRBUDisableMsg->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRBUDisableMsg->Header.DestinationAddress,
      DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRBUDisableMsg->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRBUDisableMsg->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRBUDisableMsg->UnitAddress, RBU_DISABLE_MSG_UNIT_ADDRESS_SIZE);
   MC_PUP_ShiftRegPadOnRight(RBU_DISABLE_MSG_PADDING_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pRBUDisableMsg->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;

   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackRBUDisableMessage
* Function to unpack a mesh protocol RBU Disable message from a Phy_Data_Ind message received
* from the LoRa modem.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting RBU Disable message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackRBUDisableMessage(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
   RBUDisableMessage_t *pRBUDisableMsg)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataInd) || (NULL == pRBUDisableMsg))
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pRBUDisableMsg->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(RBU_DISABLE_MSG_PADDING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRBUDisableMsg->UnitAddress, RBU_DISABLE_MSG_UNIT_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRBUDisableMsg->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRBUDisableMsg->Header.SourceAddress,
         DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRBUDisableMsg->Header.DestinationAddress,
         DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRBUDisableMsg->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRBUDisableMsg->Header.MACSourceAddress,
         DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRBUDisableMsg->Header.MACDestinationAddress,
         DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pRBUDisableMsg->Header.FrameType, DATA_FRAME_TYPE_SIZE);

      /* Range check the message values */
      MC_PUP_ASSERT(pRBUDisableMsg->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pRBUDisableMsg->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pRBUDisableMsg->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pRBUDisableMsg->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pRBUDisableMsg->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pRBUDisableMsg->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pRBUDisableMsg->Header.MessageType == APP_MSG_TYPE_RBU_DISABLE_E);//rbu disable command
      MC_PUP_ASSERT(pRBUDisableMsg->UnitAddress < 512);//max for 9 bits
   }

   return status;
}


/*************************************************************************************/
/**
* MC_PUP_PackPingRequestMessage
* Function to pack a mesh protocol Ping Request message into a Phy_Data_Req message to be
* passed to the LoRa modem.
*
* @param pingRequest - pointer to received ping message
* @param pPhyDataReq - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackPingRequestMessage(PingRequest_t* pingRequest, CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ( NULL == pPhyDataReq )
   {
      return ERR_INVALID_POINTER_E;
   }

   /*build the message in the shift register */
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pingRequest->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pingRequest->Header.MACDestinationAddress, DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pingRequest->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pingRequest->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pingRequest->Header.DestinationAddress, DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pingRequest->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pingRequest->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pingRequest->Payload, PING_REQUEST_PAYLOAD_SIZE);
   MC_PUP_ShiftRegPadOnRight(PING_REQUEST_PADDING_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pingRequest->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;

   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackPingRequestMessage
* Function to unpack a mesh protocol Ping message from a Phy_Data_Ind message received
* from the LoRa modem.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting Ping message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackPingRequestMessage(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                             PingRequest_t* pingRequest)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataInd) || (NULL == pingRequest))
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pingRequest->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(PING_REQUEST_PADDING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pingRequest->Payload, PING_REQUEST_PAYLOAD_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pingRequest->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pingRequest->Header.SourceAddress,
         DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pingRequest->Header.DestinationAddress,
         DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pingRequest->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pingRequest->Header.MACSourceAddress,
         DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pingRequest->Header.MACDestinationAddress,
         DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pingRequest->Header.FrameType, DATA_FRAME_TYPE_SIZE);

      /* Range check the message values */
      MC_PUP_ASSERT(pingRequest->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pingRequest->Header.MACDestinationAddress < 4096);//max for 12 bits
      MC_PUP_ASSERT(pingRequest->Header.MACSourceAddress < 4096);//max for 12 bits
      MC_PUP_ASSERT(pingRequest->Header.HopCount < 256);//max for 8 bits
      MC_PUP_ASSERT(pingRequest->Header.DestinationAddress < 4096);//max for 12 bits
      MC_PUP_ASSERT(pingRequest->Header.SourceAddress < 4096);//max for 12 bits
      MC_PUP_ASSERT(pingRequest->Header.MessageType == APP_MSG_TYPE_PING_E);
   }

   return status;
}

/*************************************************************************************/
/**
* MC_PUP_PackBatteryStatusMessage
* Function to pack a mesh protocol Battery Status message into a Phy_Data_Req message to be
* passed to the LoRa modem.
*
* @param - pointer to Battery Status message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackBatteryStatusMessage(BatteryStatusMessage_t *pBatteryStatusMessage,
   CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataReq) || (NULL == pBatteryStatusMessage))
   {
      return ERR_INVALID_POINTER_E;
   }
   /* Range check the message values */
   MC_PUP_ASSERT(pBatteryStatusMessage->Header.FrameType == FRAME_TYPE_DATA_E);
   MC_PUP_ASSERT(pBatteryStatusMessage->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pBatteryStatusMessage->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pBatteryStatusMessage->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pBatteryStatusMessage->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pBatteryStatusMessage->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pBatteryStatusMessage->Header.MessageType == APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E);
   MC_PUP_ASSERT(pBatteryStatusMessage->PrimaryBatteryVoltage < MAX_FOR_16_BITS);//max for 12 bits
   MC_PUP_ASSERT(pBatteryStatusMessage->BackupBatteryVoltage < MAX_FOR_16_BITS);//max for 12 bits
   MC_PUP_ASSERT(pBatteryStatusMessage->DeviceCombination < MAX_FOR_8_BITS);
   MC_PUP_ASSERT(pBatteryStatusMessage->ZoneNumber < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pBatteryStatusMessage->SmokeAnalogueValue < MAX_FOR_7_BITS);//max for sensor reading
   MC_PUP_ASSERT(pBatteryStatusMessage->HeatAnalogueValue < MAX_FOR_7_BITS);//max for sensor reading
   MC_PUP_ASSERT(pBatteryStatusMessage->PirAnalogueValue < MAX_FOR_1_BITS);// 0 or 1


   /*build the message in the shift register */
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pBatteryStatusMessage->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pBatteryStatusMessage->Header.MACDestinationAddress,
      DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pBatteryStatusMessage->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pBatteryStatusMessage->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pBatteryStatusMessage->Header.DestinationAddress,
      DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pBatteryStatusMessage->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pBatteryStatusMessage->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pBatteryStatusMessage->PrimaryBatteryVoltage, BATTERY_STATUS_MSG_VOLTAGE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pBatteryStatusMessage->BackupBatteryVoltage, BATTERY_STATUS_MSG_VOLTAGE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pBatteryStatusMessage->DeviceCombination, BATTERY_STATUS_MSG_DEVICE_COMBINATION_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pBatteryStatusMessage->ZoneNumber, BATTERY_STATUS_MSG_ZONE_NUMBER_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pBatteryStatusMessage->SmokeAnalogueValue, BATTERY_STATUS_MSG_SENSOR_READING_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pBatteryStatusMessage->HeatAnalogueValue, BATTERY_STATUS_MSG_SENSOR_READING_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pBatteryStatusMessage->PirAnalogueValue, BATTERY_STATUS_MSG_PIR_READING_SIZE);
   MC_PUP_ShiftRegPadOnRight(BATTERY_STATUS_MSG_PADDING_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pBatteryStatusMessage->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;

   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackBatteryStatusMessage
* Function to unpack a mesh protocol Battery Status message from a Phy_Data_Ind message received
* from the LoRa modem.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting battery status OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackBatteryStatusMessage(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
   BatteryStatusMessage_t *pBatteryStatusMessage)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataInd) || (NULL == pBatteryStatusMessage))
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pBatteryStatusMessage->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(BATTERY_STATUS_MSG_PADDING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pBatteryStatusMessage->PirAnalogueValue, BATTERY_STATUS_MSG_PIR_READING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pBatteryStatusMessage->HeatAnalogueValue, BATTERY_STATUS_MSG_SENSOR_READING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pBatteryStatusMessage->SmokeAnalogueValue, BATTERY_STATUS_MSG_SENSOR_READING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pBatteryStatusMessage->ZoneNumber, BATTERY_STATUS_MSG_ZONE_NUMBER_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pBatteryStatusMessage->DeviceCombination, BATTERY_STATUS_MSG_DEVICE_COMBINATION_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pBatteryStatusMessage->BackupBatteryVoltage, BATTERY_STATUS_MSG_VOLTAGE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pBatteryStatusMessage->PrimaryBatteryVoltage, BATTERY_STATUS_MSG_VOLTAGE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pBatteryStatusMessage->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pBatteryStatusMessage->Header.SourceAddress,
         DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pBatteryStatusMessage->Header.DestinationAddress,
         DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pBatteryStatusMessage->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pBatteryStatusMessage->Header.MACSourceAddress,
         DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pBatteryStatusMessage->Header.MACDestinationAddress,
         DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pBatteryStatusMessage->Header.FrameType, DATA_FRAME_TYPE_SIZE);

      /* Range check the message values */
      MC_PUP_ASSERT(pBatteryStatusMessage->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pBatteryStatusMessage->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pBatteryStatusMessage->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pBatteryStatusMessage->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pBatteryStatusMessage->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pBatteryStatusMessage->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pBatteryStatusMessage->Header.MessageType == APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E);
      MC_PUP_ASSERT(pBatteryStatusMessage->PrimaryBatteryVoltage < MAX_FOR_16_BITS);//max for 16 bits
      MC_PUP_ASSERT(pBatteryStatusMessage->BackupBatteryVoltage < MAX_FOR_16_BITS);//max for 16 bits
      MC_PUP_ASSERT(pBatteryStatusMessage->DeviceCombination < MAX_FOR_8_BITS);
      MC_PUP_ASSERT(pBatteryStatusMessage->ZoneNumber < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pBatteryStatusMessage->SmokeAnalogueValue < MAX_FOR_7_BITS);//max for sensor reading
      MC_PUP_ASSERT(pBatteryStatusMessage->HeatAnalogueValue < MAX_FOR_7_BITS);//max for sensor reading
      MC_PUP_ASSERT(pBatteryStatusMessage->PirAnalogueValue < MAX_FOR_1_BITS);// 0 or 1
   }

   return status;
}

/*************************************************************************************/
/**
* MC_PUP_PackZoneEnableMessage
* Function to pack a mesh protocol Zone Enable message into a Phy_Data_Req message to be
* passed to the LoRa modem.
*
* @param - pointer to Zone Enable message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackZoneEnableMessage(ZoneEnableMessage_t *pZoneEnableMessage,
   CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataReq) || (NULL == pZoneEnableMessage))
   {
      return ERR_INVALID_POINTER_E;
   }
   /* Range check the message values */
   MC_PUP_ASSERT(pZoneEnableMessage->Header.FrameType == FRAME_TYPE_DATA_E);
   MC_PUP_ASSERT(pZoneEnableMessage->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pZoneEnableMessage->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pZoneEnableMessage->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
   MC_PUP_ASSERT(pZoneEnableMessage->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pZoneEnableMessage->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
   MC_PUP_ASSERT(pZoneEnableMessage->Header.MessageType == APP_MSG_TYPE_ZONE_ENABLE_E);
   MC_PUP_ASSERT(pZoneEnableMessage->LowHigh < 2);//max for 1 bits
   MC_PUP_ASSERT(pZoneEnableMessage->HalfWord < MAX_FOR_16_BITS);//max for 16 bits

   /*build the message in the shift register */
   MC_PUP_ShiftRegInit();
   MC_PUP_ShiftRegLoadOnRight(pZoneEnableMessage->Header.FrameType, DATA_FRAME_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pZoneEnableMessage->Header.MACDestinationAddress,
      DATA_MAC_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pZoneEnableMessage->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pZoneEnableMessage->Header.HopCount, DATA_HOP_COUNT_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pZoneEnableMessage->Header.DestinationAddress, DATA_DESTINATION_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pZoneEnableMessage->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pZoneEnableMessage->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pZoneEnableMessage->LowHigh, ZONE_ENABLE_LOWHIGH_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pZoneEnableMessage->HalfWord, ZONE_ENABLE_HALFWORD_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pZoneEnableMessage->Word, ZONE_ENABLE_WORD_SIZE);
   MC_PUP_ShiftRegPadOnRight(ZONE_ENABLE_PADDING_SIZE);
   MC_PUP_ShiftRegLoadOnRight(pZoneEnableMessage->SystemId, DATA_SYSTEM_ID_SIZE);
   MC_PUP_ShiftRegAlignLeft();

   /* copy output from shift register */
   size = DIV_ROUNDUP(messageLengthBits, 8);
   memcpy(&pPhyDataReq->Data, shiftReg, size);
   pPhyDataReq->Size = size;

   return SUCCESS_E;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackZoneEnableMessage
* Function to unpack a mesh protocol Zone Enable message from a Phy_Data_Ind message received
* from the LoRa modem.
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting Zone Enable OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackZoneEnableMessage(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       ZoneEnableMessage_t *pZoneEnableMessage)
{
   ErrorCode_t status = SUCCESS_E;
   uint32_t size = 0;

   /* check input parameters */
   if ((NULL == pPhyDataInd) || (NULL == pZoneEnableMessage))
   {
      status = ERR_INVALID_POINTER_E;
   }
   else
   {

      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pZoneEnableMessage->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(ZONE_ENABLE_PADDING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pZoneEnableMessage->Word, ZONE_ENABLE_WORD_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pZoneEnableMessage->HalfWord, ZONE_ENABLE_HALFWORD_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pZoneEnableMessage->LowHigh, ZONE_ENABLE_LOWHIGH_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pZoneEnableMessage->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pZoneEnableMessage->Header.SourceAddress,
         DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pZoneEnableMessage->Header.DestinationAddress,
         DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pZoneEnableMessage->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pZoneEnableMessage->Header.MACSourceAddress,
         DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pZoneEnableMessage->Header.MACDestinationAddress,
         DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pZoneEnableMessage->Header.FrameType, DATA_FRAME_TYPE_SIZE);

      /* Range check the message values */
      MC_PUP_ASSERT(pZoneEnableMessage->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pZoneEnableMessage->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pZoneEnableMessage->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pZoneEnableMessage->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pZoneEnableMessage->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pZoneEnableMessage->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pZoneEnableMessage->Header.MessageType == APP_MSG_TYPE_ZONE_ENABLE_E);
      MC_PUP_ASSERT(pZoneEnableMessage->LowHigh < 2);//max for 1 bits
      MC_PUP_ASSERT(pZoneEnableMessage->HalfWord < MAX_FOR_16_BITS);//max for 16 bits
   }

   return status;
}

/*************************************************************************************/
/**
* MC_PUP_PackPpuMessage
* Function to pack a PPU message
*
* @param - pointer to the PPU message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackPpuMessage(PPU_Message_t *pPpuCommand, CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   ErrorCode_t status = ERR_INVALID_POINTER_E;
   uint8_t* pData;
   uint32_t payload_length = 0;
   
   if ( pPhyDataReq && pPpuCommand )
   {
      //read the Source address
      pData = (uint8_t*)pPhyDataReq->Data;
      //pack the PPU Master address
      pData[0] = pPpuCommand->PpuAddress & 0xFF;
      pData[1] = (pPpuCommand->PpuAddress >> 8) & 0xFF;
      //pack the RBU address
      pData[2] = pPpuCommand->RbuAddress & 0xFF;
      pData[3] = (pPpuCommand->RbuAddress >> 8) & 0xFF;
      //pack the systemID
      pData[4] = pPpuCommand->SystemID & 0xFF;
      pData[5] = (pPpuCommand->SystemID >> 8) & 0xFF;
      pData[6] = (pPpuCommand->SystemID >> 16) & 0xFF;
      pData[7] = (pPpuCommand->SystemID >> 24) & 0xFF;
#ifdef PPU_EMBEDDED_CHECKSUM
      //reserve space for checksum
      pData[8] = 0;
      pData[9] = 0;
      //pack the packet length
      pData[10] = pPpuCommand->PacketLength & 0xFF;
      pData[11] = (pPpuCommand->PacketLength >> 8) & 0xFF;
      //Pack the response code
      pData[12] = pPpuCommand->Command & 0xFF;
      pData[13] = (pPpuCommand->Command >> 8) & 0xFF;
#else
      //pack the packet length
      pData[8] = pPpuCommand->PacketLength & 0xFF;
      pData[9] = (pPpuCommand->PacketLength >> 8) & 0xFF;
      //Pack the response code
      pData[10] = pPpuCommand->Command & 0xFF;
      pData[11] = (pPpuCommand->Command >> 8) & 0xFF;
#endif
      //pack the payload
      payload_length = pPpuCommand->PacketLength - PPU_MESSAGE_HEADER_SIZE;
      if ( 0 == payload_length )
      {
         status = SUCCESS_E;
      }
      else if ( MAX_PPU_COMMAND_PAYLOAD_LENGTH > payload_length )
      {
         pData += PPU_MESSAGE_HEADER_SIZE;
         memcpy( pData, pPpuCommand->Payload, payload_length);
         status = SUCCESS_E;
      }
      else 
      {
         status = ERR_MESSAGE_FAIL_E;
      }
   }
   
   return status;
}

/*************************************************************************************/
/**
* MC_PUP_UnpackPpuMessage
* Function to unpack a PPU message
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting PPU Command OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackPpuMessage(CO_MessagePayloadPhyDataInd_t *pPhyDataInd, PPU_Message_t *pPpuCommand)
{
   ErrorCode_t status = ERR_INVALID_POINTER_E;
   uint8_t* pData;
   uint32_t payload_length;
   
   if ( pPhyDataInd && pPpuCommand )
   {
      //read the Source address
      pData = (uint8_t*)pPhyDataInd->Data;
      pPpuCommand->PpuAddress = pData[0] + ((uint16_t)pData[1] << 8 );
      //read the Target address
      pPpuCommand->RbuAddress = pData[2] + ((uint16_t)pData[3] << 8 );
      //read the SystemID
      pPpuCommand->SystemID = pData[4] + ((uint32_t)pData[5] << 8 ) + ((uint32_t)pData[6] << 16 ) + ((uint32_t)pData[7] << 24 );
#ifdef PPU_EMBEDDED_CHECKSUM
      //read the checksum
      pPpuCommand->Checksum = pData[8] + ((uint16_t)pData[9] << 8 );
      //read the Packet Length
      pPpuCommand->PacketLength = pData[10] + ((uint16_t)pData[11] << 8 );
      //read the Command
      pPpuCommand->Command = pData[12] + ((uint16_t)pData[13] << 8 );
      //calculate payload length
      pData += PPU_MESSAGE_HEADER_SIZE;
      payload_length = pPpuCommand->PacketLength - PPU_MESSAGE_HEADER_SIZE;
#else
      //read the Packet Length
      pPpuCommand->PacketLength = pData[8] + ((uint16_t)pData[9] << 8 );
      //read the Command
      pPpuCommand->Command = pData[10] + ((uint16_t)pData[11] << 8 );
      //calculate payload length
      pData += PPU_MESSAGE_HEADER_SIZE;
      payload_length = pPpuCommand->PacketLength - PPU_MESSAGE_HEADER_SIZE - PPU_MESSAGE_CHECKSUM_SIZE;
#endif
      CO_PRINT_B_5(DBG_INFO_E,"PpuCmd src=%d dst=%d sys-ser=%d pktlen=%d cmd=%04X\r\n",pPpuCommand->PpuAddress, pPpuCommand->RbuAddress, pPpuCommand->SerialNumber, pPpuCommand->PacketLength, pPpuCommand->Command);
      //read the payload
      if ( 0 == payload_length )
      {
         status = SUCCESS_E;
      }
      else if ( MAX_PPU_COMMAND_PAYLOAD_LENGTH > payload_length )
      {
         memcpy(pPpuCommand->Payload, pData, payload_length);
         status = SUCCESS_E;
      }
      else 
      {
         status = ERR_MESSAGE_FAIL_E;
      }
   }
   
   return status;
}

/*************************************************************************************/
/**
* MC_PUP_PackGlobalDelaysMessage
* Function to pack a Global Delays message
*
* @param - pointer to the PPU message INPUT
* @param - pointer to resulting Phy_Data_Req message OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_PackGlobalDelaysMessage(CO_GlobalDelaysMessage_t *pGlobalDelays, CO_MessagePayloadPhyDataReq_t *pPhyDataReq)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   uint32_t size = 0;
   
   /* check input parameters */
   if ( pPhyDataReq && pGlobalDelays )
   {
      /* Range check the message values */
      MC_PUP_ASSERT(pGlobalDelays->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pGlobalDelays->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pGlobalDelays->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pGlobalDelays->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pGlobalDelays->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pGlobalDelays->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pGlobalDelays->Header.MessageType == APP_MSG_TYPE_GLOBAL_DELAYS_E);

      /*build the message in the shift register */
      MC_PUP_ShiftRegInit();
      MC_PUP_ShiftRegLoadOnRight(pGlobalDelays->Header.FrameType, DATA_FRAME_TYPE_SIZE);
      MC_PUP_ShiftRegLoadOnRight(pGlobalDelays->Header.MACDestinationAddress,
         DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegLoadOnRight(pGlobalDelays->Header.MACSourceAddress, DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegLoadOnRight(pGlobalDelays->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegLoadOnRight(pGlobalDelays->Header.DestinationAddress, DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegLoadOnRight(pGlobalDelays->Header.SourceAddress, DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegLoadOnRight(pGlobalDelays->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegLoadOnRight(pGlobalDelays->Delay1, GLOBAL_DELAY_SIZE);
      MC_PUP_ShiftRegLoadOnRight(pGlobalDelays->Delay2, GLOBAL_DELAY_SIZE);
      MC_PUP_ShiftRegLoadOnRight(pGlobalDelays->ChannelLocalOrGlobalDelay, GLOBAL_DELAY_CHANNEL_LOCAL_SIZE);
      MC_PUP_ShiftRegPadOnRight(GLOBAL_DELAY_PADDING_SIZE);
      MC_PUP_ShiftRegLoadOnRight(pGlobalDelays->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegAlignLeft();

      /* copy output from shift register */
      size = DIV_ROUNDUP(messageLengthBits, 8);
      memcpy(&pPhyDataReq->Data, shiftReg, size);
      pPhyDataReq->Size = size;
      result = SUCCESS_E;
   }

   return result;
}
/*************************************************************************************/
/**
* MC_PUP_UnpackGlobalDelaysMessage
* Function to unpack a Global Delays message
*
* @param - pointer to Phy_Data_Ind message INPUT
* @param - pointer to resulting PPU Command OUTPUT
*
* @return - status - SUCCESS_E for success, any other value is an error

*/
ErrorCode_t MC_PUP_UnpackGlobalDelaysMessage(CO_MessagePayloadPhyDataInd_t *pPhyDataInd,
                                       CO_GlobalDelaysMessage_t *pGlobalDelays)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   uint32_t size = 0;

   /* check input parameters */
   if ( pPhyDataInd && pGlobalDelays )
   {
      MC_PUP_ShiftRegInit();

      /* load shift register with input */
      size = MIN(pPhyDataInd->Size, SHIFT_REG_SIZE_BYTES);
      memcpy(shiftReg, &pPhyDataInd->Data, size);
      messageLengthBits = DATA_MESSAGE_SIZE;

      MC_PUP_ShiftRegAlignRight();

      MC_PUP_ShiftRegExtractFromRight(&pGlobalDelays->SystemId, DATA_SYSTEM_ID_SIZE);
      MC_PUP_ShiftRegDiscardOnRight(GLOBAL_DELAY_PADDING_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pGlobalDelays->ChannelLocalOrGlobalDelay, GLOBAL_DELAY_CHANNEL_LOCAL_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pGlobalDelays->Delay2, GLOBAL_DELAY_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pGlobalDelays->Delay1, GLOBAL_DELAY_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pGlobalDelays->Header.MessageType, DATA_MESSAGE_TYPE_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pGlobalDelays->Header.SourceAddress,
         DATA_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pGlobalDelays->Header.DestinationAddress,
         DATA_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pGlobalDelays->Header.HopCount, DATA_HOP_COUNT_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pGlobalDelays->Header.MACSourceAddress,
         DATA_MAC_SOURCE_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pGlobalDelays->Header.MACDestinationAddress,
         DATA_MAC_DESTINATION_ADDRESS_SIZE);
      MC_PUP_ShiftRegExtractFromRight(&pGlobalDelays->Header.FrameType, DATA_FRAME_TYPE_SIZE);

      /* Range check the message values */
      MC_PUP_ASSERT(pGlobalDelays->Header.FrameType == FRAME_TYPE_DATA_E);
      MC_PUP_ASSERT(pGlobalDelays->Header.MACDestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pGlobalDelays->Header.MACSourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pGlobalDelays->Header.HopCount < MAX_FOR_8_BITS);//max for 8 bits
      MC_PUP_ASSERT(pGlobalDelays->Header.DestinationAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pGlobalDelays->Header.SourceAddress < MAX_FOR_12_BITS);//max for 12 bits
      MC_PUP_ASSERT(pGlobalDelays->Header.MessageType == APP_MSG_TYPE_GLOBAL_DELAYS_E);
      result = SUCCESS_E;
   }
   
   return result;
}



