/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Helper functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include <stdlib.h>
#include <stdio.h>
#include "cmsis_os.h"
#include "stm32l4xx_hal.h"
#include "board.h"
#include "lptim.h"
#include "MC_MacConfiguration.h"
#include "CFG_Device_cfg.h"
#include "utilities.h"
#include "DM_Device.h"


#define ONE_SECOND_IN_LPTIM_TICKS 16350U
#define ONE_HUNDRED_MILLISECONDS_IN_LPTIM_TICKS 1635U
#define ONE_MILLISECOND_IN_LPTIM_TICKS 163U

/*!
 * Redefinition of rand() and srand() standard C functions.
 * These functions are redefined in order to get the same behaviour across
 * different compiler toolchains implementations.
 */
// Standard random functions redefinition start
#define RAND_LOCAL_MAX 2147483647L

static uint32_t next = 1;

extern osMessageQId(AppQ);        // Queue to the application
extern osPoolId AppPool;         // Pool definition for use with AppQ.

extern uint32_t MC_SlotsPerLongFrame(void);
extern uint32_t MC_SlotsPerSuperFrame(void);


#ifdef DULCH_USE_BIT_REVERSE
//Look-up table for node ID bit reversal
const uint16_t node_id_bit_reversed[MAX_DEVICES_PER_SYSTEM] = {
   0, 256, 128, 384, 64, 320, 192, 448, 32, 288, 160, 416, 96, 352, 224, 480, 16, 272, 144, 400, 80, 336, 208, 464, 48, 304, 176, 432, 112, 368, 240, 496, 
   8, 264, 136, 392, 72, 328, 200, 456, 40, 296, 168, 424, 104, 360, 232, 488, 24, 280, 152, 408, 88, 344, 216, 472, 56, 312, 184, 440, 120, 376, 248, 504, 
   4, 260, 132, 388, 68, 324, 196, 452, 36, 292, 164, 420, 100, 356, 228, 484, 20, 276, 148, 404, 84, 340, 212, 468, 52, 308, 180, 436, 116, 372, 244, 500, 
   12, 268, 140, 396, 76, 332, 204, 460, 44, 300, 172, 428, 108, 364, 236, 492, 28, 284, 156, 412, 92, 348, 220, 476, 60, 316, 188, 444, 124, 380, 252, 508, 
   2, 258, 130, 386, 66, 322, 194, 450, 34, 290, 162, 418, 98, 354, 226, 482, 18, 274, 146, 402, 82, 338, 210, 466, 50, 306, 178, 434, 114, 370, 242, 498, 10, 
   266, 138, 394, 74, 330, 202, 458, 42, 298, 170, 426, 106, 362, 234, 490, 26, 282, 154, 410, 90, 346, 218, 474, 58, 314, 186, 442, 122, 378, 250, 506, 6, 262, 
   134, 390, 70, 326, 198, 454, 38, 294, 166, 422, 102, 358, 230, 486, 22, 278, 150, 406, 86, 342, 214, 470, 54, 310, 182, 438, 118, 374, 246, 502, 14, 270, 142, 
   398, 78, 334, 206, 462, 46, 302, 174, 430, 110, 366, 238, 494, 30, 286, 158, 414, 94, 350, 222, 478, 62, 318, 190, 446, 126, 382, 254, 510, 1, 257, 129, 385, 
   65, 321, 193, 449, 33, 289, 161, 417, 97, 353, 225, 481, 17, 273, 145, 401, 81, 337, 209, 465, 49, 305, 177, 433, 113, 369, 241, 497, 9, 265, 137, 393, 73, 
   329, 201, 457, 41, 297, 169, 425, 105, 361, 233, 489, 25, 281, 153, 409, 89, 345, 217, 473, 57, 313, 185, 441, 121, 377, 249, 505, 5, 261, 133, 389, 69, 325, 
   197, 453, 37, 293, 165, 421, 101, 357, 229, 485, 21, 277, 149, 405, 85, 341, 213, 469, 53, 309, 181, 437, 117, 373, 245, 501, 13, 269, 141, 397, 77, 333, 205, 
   461, 45, 301, 173, 429, 109, 365, 237, 493, 29, 285, 157, 413, 93, 349, 221, 477, 61, 317, 189, 445, 125, 381, 253, 509, 3, 259, 131, 387, 67, 323, 195, 451, 
   35, 291, 163, 419, 99, 355, 227, 483, 19, 275, 147, 403, 83, 339, 211, 467, 51, 307, 179, 435, 115, 371, 243, 499, 11, 267, 139, 395, 75, 331, 203, 459, 43, 
   299, 171, 427, 107, 363, 235, 491, 27, 283, 155, 411, 91, 347, 219, 475, 59, 315, 187, 443, 123, 379, 251, 507, 7, 263, 135, 391, 71, 327, 199, 455, 39, 295, 
   167, 423, 103, 359, 231, 487, 23, 279, 151, 407, 87, 343, 215, 471, 55, 311, 183, 439, 119, 375, 247, 503, 15, 271, 143, 399, 79, 335, 207, 463, 47, 303, 175, 
   431, 111, 367, 239, 495, 31, 287, 159, 415, 95, 351, 223, 479, 63, 319, 191, 447, 127, 383, 255, 511 };
#endif
/****************************************************************************/
/*!
 * Function name: rand1
 * \brief Generate a random number
 *
 * \retval random number
 */
int32_t rand1( void )
{
    return ( ( next = next * 1103515245L + 12345L ) % RAND_LOCAL_MAX );
}

/****************************************************************************/
/*!
 * Function name: srand1
 * \brief Initializes the pseudo random generator initial value
 *
 * \param [IN] seed Pseudo random generator initial value
 */
void srand1( uint32_t seed )
{
    next = seed;
}
// Standard random functions redefinition end

/****************************************************************************/
/*!
 * Function name: randr
 * \brief Computes a random number between min and max
 *
 * \param [IN] min range minimum value
 * \param [IN] max range maximum value
 * \retval random random value in range min..max
 */
int32_t randr( int32_t min, int32_t max )
{
    return ( int32_t )rand1( ) % ( max - min + 1 ) + min;
}

/****************************************************************************/
/*!
 * Function name: memcpy1
 * \brief Copies size elements of src array to dst array
 * 
 * \remark STM32 Standard memcpy function only works on pointers that are aligned
 *
 * \param [OUT] dst  Destination array
 * \param [IN]  src  Source array
 * \param [IN]  size Number of bytes to be copied
 */
void memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size )
{
    while( size-- )
    {
        *dst++ = *src++;
    }
}

/****************************************************************************/
/*!
 * Function name: memcpyr
 * \brief Copies size elements of src array to dst array reversing the byte order
 *
 * \param [OUT] dst  Destination array
 * \param [IN]  src  Source array
 * \param [IN]  size Number of bytes to be copied
 */
void memcpyr( uint8_t *dst, const uint8_t *src, uint16_t size )
{
    dst = dst + ( size - 1 );
    while( size-- )
    {
        *dst-- = *src++;
    }
}

/****************************************************************************/
/*!
 * Function name: memset1
 * \brief Set size elements of dst array with value 
 * 
 * \remark STM32 Standard memset function only works on pointers that are aligned
 *
 * \param [OUT] dst   Destination array
 * \param [IN]  value Default value
 * \param [IN]  size  Number of bytes to be copied
 */
void memset1( uint8_t *dst, uint8_t value, uint16_t size )
{
    while( size-- )
    {
        *dst++ = value;
    }
}

/****************************************************************************/
/*!
 * Function name: Nibble2HexChar
 * \brief Converts a nibble to an hexadecimal character
 * 
 * \param [IN] a   Nibble to be converted
 * \retval hexChar Converted hexadecimal character
 */
int8_t Nibble2HexChar( uint8_t a )
{
    if( a < 10 )
    {
        return '0' + a;
    }
    else if( a < 16 )
    {
        return 'A' + ( a - 10 );
    }
    else
    {
        return '?';
    }
}

/****************************************************************************/
/*!
 * Function name: HexChar2Nibble
 * \brief Converts a hexadecimal character to a nibble
 * 
 * \param [IN] valueChar   Pointer to the string to be converted
 * \param [OUT] pValue     Converted number value
 * \retval bool            True if succeeded
 */
bool HexChar2Nibble( const char valueChar, int8_t* const pValue )
{
   bool result = false;
   int8_t value = valueChar & 0x0f;
   
   if ( pValue )
   {
      if ( ('0' <= value) && ('F' >= value) )
      {
          if( 'A' <= value )
          {
             /* account for the 7 char gap between '9' and 'A' */
             value -= 7; 
          }
          /* rebase the value to 0 */
          value -= '0';
          
          if ( (0 <= value) && (16 > value) )
          {
             *pValue = value;
          }
       }
    }

    return result;
}

/****************************************************************************/
/*!
 * Function name: HexChar2Short
 * \brief Converts an ascii representation hexadecimal number to an uint16
 * 
 * \param [IN] pValueStr   Pointer to the string to be converted
 * \param [OUT] pValue     Converted number value
 * \retval bool            True if succeeded
 */
bool HexChar2Short(const char* pValueStr, uint16_t* const pValue)
{
   bool result = false;
   uint8_t nibble;
   int8_t nibble_value;
   uint16_t value = 0;
   char* pStr = (char*)pValueStr;
   
   if ( pValueStr && pValue )
   {
      /* Start with the hi nibble */
      pStr++;
      nibble = ( *pStr >> 4 );
      if ( HexChar2Nibble( nibble, &nibble_value ) )
      {
         value = nibble_value;
         value <<= 4;
         
         /* next nibble */
         nibble = ( *pStr & 0x0f );
         if ( HexChar2Nibble( nibble, &nibble_value ) )
         {
            value += nibble_value;
            value <<= 4;
            
            /* next nibble */
            pStr = (char*)pValueStr;
            nibble = ( *pStr >> 4 );
            if ( HexChar2Nibble( nibble, &nibble_value ) )
            {
               value += nibble_value;
               value <<= 4;
               
               /* last nibble */
               nibble = ( *pStr & 0x0f );
               if ( HexChar2Nibble( nibble, &nibble_value ) )
               {
                  value += nibble_value;
                  *pValue = value;
                  result = true;
               }
            }
         }
      }
   }
   
   return result;
}

/****************************************************************************/
/*!
 * Function name: Short2HexChar
 * \brief Converts an ascii representation hexadecimal number to an uint16
 * 
 * \param [IN] value       uint16 to be converted
 * \param [OUT] pStr       Converted string
 * \retval bool            True if succeded
 */
bool Short2HexChar(const uint16_t value, char* const pStr)
{
   bool result = false;
   int8_t nibble = 0;
   char valueStr[5];
   
   if ( pStr )
   {
      nibble = (int8_t)((value >> 12) & 0x0f);
      valueStr[0] = Nibble2HexChar( nibble );
      if ( '?' != valueStr[0] )
      {
         nibble = (int8_t)((value >> 8) & 0x0f);
         valueStr[1] = Nibble2HexChar( nibble );
         if ( '?' != valueStr[1] )
         {
            nibble = (int8_t)((value >> 4) & 0x0f);
            valueStr[2] = Nibble2HexChar( nibble );
            if ( '?' != valueStr[2] )
            {
               nibble = (int8_t)((value >> 12) & 0x0f);
               valueStr[3] = Nibble2HexChar( nibble );
               if ( '?' != valueStr[3] )
               {
                  valueStr[4] = 0;
                  strncpy(pStr, valueStr, 4);
                  result = true;
               }
            }
         }
      }
   }
   
   
   return result;
}

/****************************************************************************/
/*!
 * Function name: IsAlpha
 * \brief Checks if an ascii char is in the range a-z
 * 
 * \param [IN] chr   Character to be tested
 * \retval bool      True if char is aplha
 */
bool IsAlpha(const char chr)
{
   bool result = false;
   
   if ( (('A' <= chr) && ('Z' >= chr)) || (('a' <= chr) && ('z' >= chr )) )
   {
      result = true;
   }
   return result;
}

/*************************************************************************************/
/**
* function name   : CO_InlineDelay
* description     : This function performs a loop delay for a requested time period
*                   up to a maximum of one hundred milliseconds.  The constraint is 
*                   to guard against long delays that will trigger the watchdog.
*
* @param - milliseconds - The requested time period.
*
* @return - None.
*/
void CO_InlineDelay(const uint32_t milliseconds)
{
   uint32_t counter_offset = (ONE_SECOND_IN_LPTIM_TICKS * milliseconds) / 1000;
   
   if ( ONE_HUNDRED_MILLISECONDS_IN_LPTIM_TICKS < counter_offset )
   {
      counter_offset = ONE_HUNDRED_MILLISECONDS_IN_LPTIM_TICKS;
   }
   
   uint32_t lpTimeNow = LPTIM_ReadCounter(&hlptim1);
   /* Account for 16-bit counter wrap-around and don't allow the endTime to fall in the last millisecond
      because it would greatly increace the complexity of the time-out checks in the delay loop*/
   uint32_t endTime = (lpTimeNow + counter_offset) % (0x10000 - ONE_MILLISECOND_IN_LPTIM_TICKS); 
   bool not_done = true;
   
   int32_t time_delta = 0;
   while ( not_done )
   {
      lpTimeNow = LPTIM_ReadCounter(&hlptim1);
      
      if ( lpTimeNow == endTime )
      {
         /* This is the normal case where lpTimeNow caught up with endTime*/
         not_done = false;
      }
      else if ( lpTimeNow > endTime )
      {
         /* This catches two possible cases.
            1) The end time occurs beyond a roll-over of the 16 bit LPTIM counter
            2) This thread was interrupted causing lpTimeNow to overtake endTime
         */
         time_delta = lpTimeNow - endTime;
         if ( ONE_MILLISECOND_IN_LPTIM_TICKS > time_delta )
         {
            not_done = false;
         }
      }
   }
}

/*************************************************************************************/
/**
* function name   : CO_InlineDelayTicks
* description     : Block for the sepcified number of LPTIM ticks.
*
* @param - lptimTicks - The requested time period.
*
* @return - None.
*/
void CO_InlineDelayTicks(const uint32_t lptimTicks)
{
   bool not_done = true;
   uint32_t count = 0;
   uint16_t time_now;
   uint32_t last_time = LPTIM_ReadCounter(&hlptim1);
   
   while ( not_done )
   {
      time_now = LPTIM_ReadCounter(&hlptim1);
      
      if ( time_now != last_time)
      {
         count++;
         if ( count >= lptimTicks )
         {
            not_done = false;
         }
         last_time = time_now;
      }
   }
}

bool DM_OP_ChannelDisabled(const CO_ChannelIndex_t channelIndex);



/*************************************************************************************/
/**
* function name   : CO_SendFaultSignal
* description     : Send a fault message to the Application
*
* @param - channel.       The channel index
* @param - faultType.     Describes the fault
* @param - value.         sensor value, usualy 1 or 0 ( fault set/reset )
* @param - sendOnDULCH    If true send on delayed uplink channel, if false send immediately
* @param - forceSend      If true, send regardless of whether the fault flag has changed.
*
* @return - ErrorCode_t.  SUCCESS_E or fail code.
*/
ErrorCode_t CO_SendFaultSignal( const BITFaultSource_t Source, const CO_ChannelIndex_t channel, const FaultSignalType_t faultType, const uint16_t value, const bool sendOnDULCH, const bool forceSend )
{
   ErrorCode_t status = ERR_NO_MEMORY_E;
  
	// tamper/installation faults re-send on enablement, so block if disabled.
	if ( !( faultType == FAULT_SIGNAL_INSTALLATION_TAMPER_E || faultType == FAULT_SIGNAL_DISMANTLE_TAMPER_E ) ||
		( CFG_ZoneEnabled( ) && DM_RbuEnabled( ) ) )
	{		 
		 CO_FaultData_t faultData;
	
		CO_PRINT_B_3(DBG_INFO_E,"Sending fault to app: chan=%d, type=%d, value=%d\r\n", channel, faultType, value);
	
		faultData.Source = Source;
		faultData.RUChannelIndex = (uint16_t)channel;
		faultData.FaultType = (uint16_t)faultType;
		faultData.Value = value;
		faultData.DelaySending = sendOnDULCH;
		faultData.ForceSend = forceSend;

		/* Send the fault message to the Application */
		CO_Message_t* pFaultMessage = osPoolAlloc(AppPool);
		if (pFaultMessage)
		{
			pFaultMessage->Type = CO_MESSAGE_GENERATE_FAULT_SIGNAL_E;
			memcpy(pFaultMessage->Payload.PhyDataReq.Data, &faultData, sizeof(CO_FaultData_t));
			pFaultMessage->Payload.PhyDataReq.Size = sizeof(CO_FaultData_t);
			
			osStatus osStat = osMessagePut(AppQ, (uint32_t)pFaultMessage, 0);
			
			if (osOK != osStat)
			{
				/* failed to put message in the head queue */
				osPoolFree(AppPool, pFaultMessage);
				status = ERR_QUEUE_OVERFLOW_E;
			}
			else  
			{
				status = SUCCESS_E;
				CO_PRINT_B_5(DBG_INFO_E,"Sent fault to app: chan=%d, type=%d, value=%d, dulch=%d, force=%d\r\n", channel, faultType, value, sendOnDULCH, forceSend);
			}
		}
	}
		
   return status;
}

/*****************************************************************************
*  Function:   CO_RequestSensorValues
*  Description:         Request that sensor values be updated in the application.
*
*  Param - None.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t CO_RequestSensorValues(void)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;

   //Send a message to the App.
   CO_Message_t* pMessage = osPoolAlloc(AppPool);
   if (pMessage)
   {
      pMessage->Type = CO_MESSAGE_REQUEST_SENSOR_DATA_E;
      
      osStatus osStat = osMessagePut(AppQ, (uint32_t)pMessage, 0);
      
      if (osOK != osStat)
      {
         /* failed to put message in the head queue */
         osPoolFree(AppPool, pMessage);
         result = ERR_QUEUE_OVERFLOW_E;
      }
      else  
      {
         result = SUCCESS_E;
      }
   }
   return result;
}

/*************************************************************************************/
/**
* function name   : CO_IsOutputChannel
* description     : Determine whether the supplied channel number is an output channel.
*
* @param - channel.  The channel index
*
* @return - bool.    true if the channel is an output
*/
bool CO_IsOutputChannel( const uint32_t channel )
{
   bool isOutputChannel = false;
   
   if ( CO_CHANNEL_MAX_E > channel )
   {
      CO_ChannelIndex_t channel_index = (CO_ChannelIndex_t)channel;
      
      switch ( channel_index )
      {
         case CO_CHANNEL_SOUNDER_E:// intentional drop-through
         case CO_CHANNEL_BEACON_E:
         case CO_CHANNEL_STATUS_INDICATOR_LED_E:
         case CO_CHANNEL_VISUAL_INDICATOR_E:
         case CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E:
         case CO_CHANNEL_OUTPUT_ROUTING_E:
         case CO_CHANNEL_OUTPUT_1_E:
         case CO_CHANNEL_OUTPUT_2_E:
         case CO_CHANNEL_OUTPUT_3_E:
         case CO_CHANNEL_OUTPUT_4_E:
         case CO_CHANNEL_OUTPUT_5_E:
         case CO_CHANNEL_OUTPUT_6_E:
         case CO_CHANNEL_OUTPUT_7_E:
         case CO_CHANNEL_OUTPUT_8_E:
         case CO_CHANNEL_OUTPUT_9_E:
         case CO_CHANNEL_OUTPUT_10_E:
         case CO_CHANNEL_OUTPUT_11_E:
         case CO_CHANNEL_OUTPUT_12_E:
         case CO_CHANNEL_OUTPUT_13_E:
         case CO_CHANNEL_OUTPUT_14_E:
         case CO_CHANNEL_OUTPUT_15_E:
         case CO_CHANNEL_OUTPUT_16_E:
            isOutputChannel = true;
            break;
         default:
            break;
      }
   }
   
   return isOutputChannel;
}

/*************************************************************************************/
/**
* function name   : CO_IsInputChannel
* description     : Determine whether the supplied channel number is an input channel.
*
* @param - channel.  The channel index
*
* @return - bool.    true if the channel is an input
*/
bool CO_IsInputChannel( const uint32_t channel )
{
   bool isInputChannel = false;
   
   if ( CO_CHANNEL_MAX_E > channel )
   {
      CO_ChannelIndex_t channel_index = (CO_ChannelIndex_t)channel;
      
      switch ( channel_index )
      {
         case CO_CHANNEL_SMOKE_E: // intentional drop-through
         case CO_CHANNEL_HEAT_B_E:
         case CO_CHANNEL_CO_E:
         case CO_CHANNEL_PIR_E:
         case CO_CHANNEL_FIRE_CALLPOINT_E:
         case CO_CHANNEL_MEDICAL_CALLPOINT_E:
         case CO_CHANNEL_EVAC_CALLPOINT_E:
         case CO_CHANNEL_HEAT_A1R_E:
         case CO_CHANNEL_INPUT_1_E:
         case CO_CHANNEL_INPUT_2_E:
         case CO_CHANNEL_INPUT_3_E:
         case CO_CHANNEL_INPUT_4_E:
         case CO_CHANNEL_INPUT_5_E:
         case CO_CHANNEL_INPUT_6_E:
         case CO_CHANNEL_INPUT_7_E:
         case CO_CHANNEL_INPUT_8_E:
         case CO_CHANNEL_INPUT_9_E:
         case CO_CHANNEL_INPUT_10_E:
         case CO_CHANNEL_INPUT_11_E:
         case CO_CHANNEL_INPUT_12_E:
         case CO_CHANNEL_INPUT_13_E:
         case CO_CHANNEL_INPUT_14_E:
         case CO_CHANNEL_INPUT_15_E:
         case CO_CHANNEL_INPUT_16_E:
         case CO_CHANNEL_INPUT_17_E:
         case CO_CHANNEL_INPUT_18_E:
         case CO_CHANNEL_INPUT_19_E:
         case CO_CHANNEL_INPUT_20_E:
         case CO_CHANNEL_INPUT_21_E:
         case CO_CHANNEL_INPUT_22_E:
         case CO_CHANNEL_INPUT_23_E:
         case CO_CHANNEL_INPUT_24_E:
         case CO_CHANNEL_INPUT_25_E:
         case CO_CHANNEL_INPUT_26_E:
         case CO_CHANNEL_INPUT_27_E:
         case CO_CHANNEL_INPUT_28_E:
         case CO_CHANNEL_INPUT_29_E:
         case CO_CHANNEL_INPUT_30_E:
         case CO_CHANNEL_INPUT_31_E:
         case CO_CHANNEL_INPUT_32_E:
            isInputChannel = true;
            break;
         default:
            break;
      }
   }
   
   return isInputChannel;
}

/*************************************************************************************/
/**
* function name   : CO_CalculateNodeID
* description     : Calculate the node ID for a given DCH slot.
*
* @param - channel.     The super frame slot index
*
* @return - uint16_t    The node ID if the slot is DCH, otherwise CO_BAD_NODE_ID
*/
uint16_t CO_CalculateNodeID(const uint32_t slot_index)
{
   uint32_t slots_per_long_frame = MC_SlotsPerLongFrame();
   
   uint16_t node_id = CO_BAD_NODE_ID;
   uint16_t long_frame_slot = slot_index % slots_per_long_frame;
   uint32_t slots_per_super_frame = MC_SlotsPerSuperFrame();
   uint16_t short_frame_slot = slot_index % SLOTS_PER_SHORT_FRAME;
   
   if (slots_per_super_frame > slot_index)
   {
      node_id = (DCH_SLOTS_PER_SHORT_FRAME * (long_frame_slot / SLOTS_PER_SHORT_FRAME)) + (short_frame_slot / 10);
   }
   return node_id;
}

/*************************************************************************************/
/**
* function name   : CO_CalculateLongFrameSlotForDCH
* description     : Calculate the long frame slot for the heartbeat for a node ID.
*
* @param - node_id.     The node ID to calculate the slot for
*
* @return - uint16_t    The calculated long frame slot
*/
uint16_t CO_CalculateLongFrameSlotForDCH(const uint16_t node_id)
{
   uint16_t long_frame_slot = 0;
   
   //How many short frames are required for the node ID at DCH_SLOTS_PER_SHORT_FRAME
   uint16_t complete_short_frames = node_id / DCH_SLOTS_PER_SHORT_FRAME;
   //How far into the short frame is the node ID
   uint16_t short_frame_slot = (node_id % DCH_SLOTS_PER_SHORT_FRAME) * 10;
   //Calculate the long frame slot
   long_frame_slot = (complete_short_frames * SLOTS_PER_SHORT_FRAME) + short_frame_slot;
   
   return long_frame_slot;
}

/*************************************************************************************/
/**
* function name   : CO_IsDulchSlot
* description     : Apply the DULCH algorithm for the supplied node ID and super frame slot.
*
* @param - node_id.              The node ID
* @param - super_frame_slot.     The super frame slot
*
* @return - bool    TRUE if the supplied slot is a dulch slot for the node ID.
*/
bool CO_IsDulchSlot(const uint16_t node_id, const uint32_t super_frame_slot)
{
   bool is_a_dulch_slot = false;
   bool is_dulch_short_frame;
   uint32_t short_frame_in_super_frame;
   uint16_t node_match;
#ifdef USE_DULCH_WRAP_AROUND
   uint32_t dulch_wrap = CFG_GetDulchWrap();
#else
   uint16_t max_devices_per_system = MC_MaxNumberOfNodes();
#endif

   
   //DULCH slots occur evey even-numbered short frame.
   //The check for a particular node is 'short frame within superframe' % (Max Address * 2) == (node_id * 2)
   //The dulch slot is defined by DULCH_SLOT_IDX, so dulch occurs when super_frame_slot % 'slots per short frame' == DULCH_SLOT_IDX
   //Is this slot in the dulch position within the short frame?
   if ( (super_frame_slot % SLOTS_PER_SHORT_FRAME) == DULCH_SLOT_IDX )
   {
      //is this a dulch short frame
      if ( CO_IsDulchShortFrame(super_frame_slot) )
      {
         //calculate how many short frames we are into the super frame
         short_frame_in_super_frame = super_frame_slot / SLOTS_PER_SHORT_FRAME;
         
         //Is this a short frame for the supplied node ID
#ifdef USE_DULCH_WRAP_AROUND
         node_match = node_id * 2;
         is_dulch_short_frame = (short_frame_in_super_frame % dulch_wrap) == node_match;
#else
#ifdef DULCH_USE_BIT_REVERSE
         node_match = node_id_bit_reversed[node_id] * 2;
#else
         node_match = node_id * 2;
#endif
         is_dulch_short_frame = (short_frame_in_super_frame % (max_devices_per_system * 2)) == node_match;
#endif
         if ( is_dulch_short_frame )
         {
            //This is a dulch slot for the supplied node ID
            is_a_dulch_slot = true;
         }
      }
   }
   
   return is_a_dulch_slot;
}

/*************************************************************************************/
/**
* function name   : CO_IsDulchShortFrame
* description     : Apply the DULCH algorithm for the supplied super frame slot to see
*                   if the slot lies in a DULCH short frame.
*
* @param - super_frame_slot.     The super frame slot
*
* @return - bool    TRUE if the supplied slot is in a short frame with any DULCH slot.
*/
bool CO_IsDulchShortFrame(const uint32_t super_frame_slot)
{
   bool is_a_dulch_short_frame = false;
   uint32_t short_frame_in_super_frame;
   
   //calculate how many short frames we are into the super frame
   short_frame_in_super_frame = super_frame_slot / SLOTS_PER_SHORT_FRAME;
   //All even numbered short frames contain a DULCH slot
   if ( 0 == (short_frame_in_super_frame & 0x01) )
   {
      is_a_dulch_short_frame = true;
   }
   return is_a_dulch_short_frame;
}

/*************************************************************************************/
/**
* function name   : CO_IsValidNetworAddress
* description     : Checks supplied adress is in the valis range.
*
* @param - address.     The address to check
*
* @return - bool    TRUE if the supplied address is valid.
*/
bool CO_IsValidNetworAddress(const uint16_t address)
{
   bool result = false;
   
   if ( (ADDRESS_GLOBAL == address) || (MAX_DEVICES_PER_SYSTEM > address) )
   {
      result = true;
   }
   
   return result;
}

