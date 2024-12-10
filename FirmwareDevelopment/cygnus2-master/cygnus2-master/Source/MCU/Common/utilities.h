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
#ifndef __UTILITIES_H__
#define __UTILITIES_H__

#include <stdint.h>
#include <stdbool.h>
#include "CO_Message.h"

/****************************************************************************/
/*!
 * Function name: POW2
 * \brief Returns 2 raised to the power of n
 *
 * \param [IN] n power value
 * \retval result of raising 2 to the power n
 */
#define POW2( n ) ( 1 << n )

/****************************************************************************/
/*!
 * Function name: rand1
 * \brief Generate a random number
 *
 * \retval random number
 */
int32_t rand1( void );

/****************************************************************************/
/*!
 * Function name: srand1
 * \brief Initializes the pseudo random generator initial value
 *
 * \param [IN] seed Pseudo random generator initial value
 */
void srand1( uint32_t seed );

/****************************************************************************/
/*!
 * Function name: randr
 * \brief Computes a random number between min and max
 *
 * \param [IN] min range minimum value
 * \param [IN] max range maximum value
 * \retval random random value in range min..max
 */
int32_t randr( int32_t min, int32_t max );

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
void memcpy1( uint8_t *dst, const uint8_t *src, uint16_t size );

/****************************************************************************/
/*!
 * Function name: memcpyr
 * \brief Copies size elements of src array to dst array reversing the byte order
 *
 * \param [OUT] dst  Destination array
 * \param [IN]  src  Source array
 * \param [IN]  size Number of bytes to be copied
 */
void memcpyr( uint8_t *dst, const uint8_t *src, uint16_t size );

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
void memset1( uint8_t *dst, uint8_t value, uint16_t size );

/****************************************************************************/
/*!
 * Function name: Nibble2HexChar
 * \brief Converts a nibble to an hexadecimal character
 * 
 * \param [IN] a   Nibble to be converted
 * \retval hexChar Converted hexadecimal character
 */
int8_t Nibble2HexChar( uint8_t a );

/****************************************************************************/
/*!
 * Function name: HexChar2Nibble
 * \brief Converts a hexadecimal character to a nibble
 * 
 * \param [IN] valueChar   Pointer to the string to be converted
 * \param [OUT] pValue     Converted number value
 * \retval bool            True if succeeded
 */
bool HexChar2Nibble( const char valueChar, int8_t* const pValue );

/****************************************************************************/
/*!
 * Function name: HexChar2Short
 * \brief Converts an ascii representation hexadecimal number to an uint16
 * 
 * \param [IN] pValueStr   Pointer to the string to be converted
 * \param [OUT] pValue     Converted number value
 * \retval bool            True if succeeded
 */
bool HexChar2Short(const char* pValueStr, uint16_t* const pValue);

/****************************************************************************/
/*!
 * Function name: Short2HexChar
 * \brief Converts an ascii representation hexadecimal number to an uint16
 * 
 * \param [IN] pValueStr   Pointer to the string to be converted
 * \param [OUT] pValue     Converted number value
 * \retval bool            True if succeeded
 */
bool Short2HexChar(const uint16_t value, char* const pStr);

/****************************************************************************/
/*!
 * Function name: IsAlpha
 * \brief Checks if an ascii char is in the range a-z
 * 
 * \param [IN] chr   Character to be tested
 * \retval bool      True if char is aplha
 */
bool IsAlpha(const char chr);

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
void CO_InlineDelay(const uint32_t milliseconds);

/*************************************************************************************/
/**
* function name   : CO_SendFaultSignal
* description     : Send a fault message to the Application
*
* @param - Source.        The System raising the fault (radio, SVI, etc)
* @param - channel.       The channel index
* @param - faultType.     Describes the fault
* @param - value.         sensor value, usualy 1 or 0 ( fault set/reset )
* @param - sendOnDULCH    If true send on delayed uplink channel, if false send immediately
* @param - forceSend      If true, send regardless of whether the fault flag has changed.
*
* @return - ErrorCode_t.  SUCCESS_E or fail code.
*/
ErrorCode_t CO_SendFaultSignal( const BITFaultSource_t Source, const CO_ChannelIndex_t channel, const FaultSignalType_t faultType, const uint16_t value, const bool sendOnDULCH, const bool forceSend );


/*****************************************************************************
*  Function:   CO_RequestSensorValues
*  Description:         Request that sensor values be updated in the application.
*
*  Param - None.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*****************************************************************************/
ErrorCode_t CO_RequestSensorValues(void);


/*************************************************************************************/
/**
* function name   : CO_InlineDelayTicks
* description     : Block for the sepcified number of LPTIM ticks.
*
* @param - lptimTicks - The requested time period.
*
* @return - None.
*/
void CO_InlineDelayTicks(const uint32_t lptimTicks);

/*************************************************************************************/
/**
* function name   : CO_IsOutputChannel
* description     : Determine whether the supplied channel number is an output channel.
*
* @param - channel.  The channel index
*
* @return - bool.    true if the channel is an output
*/
bool CO_IsOutputChannel( const uint32_t channel );

/*************************************************************************************/
/**
* function name   : CO_IsInputChannel
* description     : Determine whether the supplied channel number is an input channel.
*
* @param - channel.  The channel index
*
* @return - bool.    true if the channel is an input
*/
bool CO_IsInputChannel( const uint32_t channel );

/*************************************************************************************/
/**
* function name   : CO_CalculateNodeID
* description     : Calculate the node ID for a given DCH slot.
*
* @param - channel.     The long frame slot index
*
* @return - uint16_t    The node ID if the slot is DCH, otherwise CO_BAD_NODE_ID
*/
uint16_t CO_CalculateNodeID(const uint32_t slot_index);

/*************************************************************************************/
/**
* function name   : CO_CalculateLongFrameSlotForDCH
* description     : Calculate the long frame slot for the heartbeat for a node ID.
*
* @param - node_id.     The node ID to calculate the slot for
*
* @return - uint16_t    The calculated long frame slot
*/
uint16_t CO_CalculateLongFrameSlotForDCH(const uint16_t node_id);

/*************************************************************************************/
/**
* function name   : CO_IsDulchSlot
* description     : Apply the DULCH algorith for the supplied node ID and super frame slot.
*
* @param - node_id.              The node ID
* @param - super_frame_slot.     The super frame slot
*
* @return - bool    TRUE if the supplied slot is a dulch slot for the node ID.
*/
bool CO_IsDulchSlot(const uint16_t node_id, const uint32_t super_frame_slot);

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
bool CO_IsDulchShortFrame(const uint32_t super_frame_slot);

/*************************************************************************************/
/**
* function name   : CO_IsValidNetworAddress
* description     : Checks supplied adress is in the valis range.
*
* @param - address.     The address to check
*
* @return - bool    TRUE if the supplied address is valid.
*/
bool CO_IsValidNetworAddress(const uint16_t address);


#endif // __UTILITIES_H__
