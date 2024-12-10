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
**************************************************************************************
*  File         : DP_Protocol.h
*
*  Description  :
*
*************************************************************************************/
#ifndef	HDF_DP_PROTOCOL_H
#define	HDF_DP_PROTOCOL_H

	 
#include	"DP_System.h"
#include	"DP_LoRa.h"
#include	"DP_Debug.h"

/*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*
 *+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*/
#ifdef __cplusplus
extern "C"
{
/*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*
 *+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*/

		// Commands
#define	HDF_DX_BATTERY_LEVEL					0x0100

#define	HDF_DX_CHAR								0x0120
#define	HDF_DX_SHORT							0x0121
#define	HDF_DX_INT								0x0122
#define	HDF_DX_LONG								0x0123
#define	HDF_DX_FLOAT							0x0124
#define	HDF_DX_DOUBLE							0x0125

#define	HDF_DX_STRING							0x0130
#define	HDF_DX_STRING_RAW						0x0131
#define	HDF_DX_BYTESTREAM						0x0132

#define	HDF_DX_TEMPERATURE					0x0140
#define	HDF_DX_PRESSURE						0x0142
#define	HDF_DX_HUMIDITY						0x0146

#define	HDF_DX_PIR								0x0150
#define	HDF_DX_RADAR							0x0152
#define	HDF_DX_LUX								0x0154

#define	HDF_DX_C02								0x0160
#define	HDF_DX_C03								0x0162
#define	HDF_DX_FLAME							0x0164

#define	HDF_DX_ACCEL_XYZ						0x0170
#define	HDF_DX_ACCEL_X							0x0171
#define	HDF_DX_ACCEL_Y							0x0172
#define	HDF_DX_ACCEL_Z							0x0173

#define	HDF_DX_GYRO_XYZ						0x0180
#define	HDF_DX_GYRO_X							0x0181
#define	HDF_DX_GYRO_Y							0x0182
#define	HDF_DX_GYRO_Z							0x0183

#define	HDF_DX_GPS								0x0191
#define	HDF_DX_GPS_LONG						0x0192
#define	HDF_DX_GPS_LATT						0x0193
#define	HDF_DX_GPS_ALTI						0x0194

#define	HDF_DX_ADC								0x01a0

#define	HDF_DX_TOUCH							0x01b0

#define	HDF_DX_TIME_INTERNAL					0x01c0
#define	HDF_DX_TIME_EXTERNAL					0x01d1
#define	HDF_DX_TIME_NTP						0x01e2

#define	HDF_DX_BLIP_TEST						0x0abc




#define	HDF_DX_CYGNUS_PULSE					0x0400		// For testing LoRa packets burst & sizes.




#define	HDF_DX_ERR_CHECKSUM					0x0001		// Checksum error.
#define	HDF_DX_ERR_COMMAND_UNKNOWN			0x0002		// Unknown command (command in lower 3 nibbles).
#define	HDF_DX_ERR_UNSUPPORTED_FEATURE	0x0004		// Device doesn't support this feature (ie: GPS, PIR, External Clock, etc.
#define	HDF_DX_ERR_BUSY						0x0008		// Device busy, try command request again.
#define	HDF_DX_ERR_SYS_MODE_REQUIRED		0x0010		// Command needs to be sent in "Security Mode".
#define	HDF_DX_ERR_INVALID_PARAM_COUNT	0x0020		// Invalid parameter count for command.
#define	HDF_DX_ERR_INVALID_PACKET_SIZE	0x0040		// Invalid data packet size.


#define	HDF_DX_BIT_REQUEST_DATA				0x1000		// [Bit #13] == Request data
#define	HDF_DX_BIT_REPLY_WITH_OK			0x4000		// [Bit #14] == Command OK (sent back to requester with this bit set, if requested).
#define	HDF_DX_BIT_REPLY_WITH_ERROR		0x8000		// [Bit #15] == Error code - stored in first int16_t of command data recieved.

			// Mask onto TX command in reply.
#define	HDF_DX_MASK_CMD_STATUS_NIBBLE		0xf000		// For masking the top nibble to check for RX response

#define	HDF_DX_HEARTBEAT						0x0ffe

			// Strips command without status nibble.
#define	HDF_DX_MASK_COMMANDS					0x0fff		// No top nibble set == only recieving command with data


#define	HDF_DX_MAX_BLIP_TEXT					64				// Maximum characters in HDF_DX_BLIP_TEST
#define	HDF_DX_MAX_BYTE_STREAM				160


#define	HDF_DX_MAX_CYGNUS_PACKET			256			// BYTES - For Cynus Packet transfer tester.




class	CDP_Protocol
{
public:
	CDP_Protocol();
	~CDP_Protocol();


	CDP_LoRa		obLoRaDX;
	CDP_Debug	*pobDebug;



	ENUM_DEBUG_MASK	enuDebugOutputType;

	// For variable sized LoRa packets in test transmissions.
	typedef	struct
	{
		int16_t	iCounter;									// Running counter to check if packets are delivered or missed.
		int16_t	iDataSize;									// Size of data to be sent.
		int8_t	acData[HDF_DX_MAX_CYGNUS_PACKET];	// Data to be sent.

	} _xCygnusPulse;

	
	typedef	struct
	{
		int16_t	iCounter;
		int8_t	pszString[HDF_DX_MAX_BLIP_TEXT];
		int8_t	cNULL;
		//
	} _xBlipTest;


	typedef	struct
	{
		int8_t	cHours;
		int8_t	cMinutes;
		int8_t	cSeconds;
		int8_t	cDayOfTheWeek;			// Day of week (1 == Sunday, 7 == Saturday)
		int8_t	cDayOfTheMonth;		// Date (1 to 31)
		int8_t	cMonth;
		int8_t	cYear;
		//
	} _xTime;


	typedef	struct
	{
		uint16_t	usByteCount;
		uint8_t	aucBytes[HDF_DX_MAX_BYTE_STREAM];	// #1 less than HDF_DX_MAX_COMMAND_DATA for ucByteCount;
		//
	} _xByteStream;


	/* MUST be a multiple of #4 else compiler will pad #zeros to next data-type after the structure to that,
	 * which must not happen after this structure. As this is for a linear transmission & would waste bytes.
	 */
	typedef	struct
	{
		uint16_t		usDeviceID;
		uint16_t		usDeviceID_SendingTo;
		uint16_t		usChecksum;
		uint16_t		usPacketLength;
		uint16_t		usCommand;
		uint8_t		aucPadding2[2];		// To the make the structure a multiple of #4 bytes (can use later).

	} _xCommsHeaderDX;
	/* MUST be a multiple of #4 else compiler will pad #zeros to next data-type after the structure to that,
	 * which must not happen after this structure. As this is for a linear transmission & would waste bytes.
	 */


	/* A union of all data-types, so the data packet can validily access all elements.
	 */
	typedef	union
	{
		int8_t		cChar;
		uint8_t		ucChar;
		int16_t		sShort;
		uint16_t		usShort;
		int32_t		lLong;
		uint32_t		ulLong;
		float			fFloat;
		double		dDouble;

		uint16_t		usErrorCode;

		uint16_t		usBattery;
		int16_t		sLux;

		_xBlipTest			xBlipTest;		// For testing transmission of different data packet sizes.
		_xCygnusPulse		xCygnusPulse;
		_xByteStream		xByteStream;

	} _uxDataType;


#define	HDF_DX_MAX_COMMAND_DATA				sizeof(_uxDataType)
#define	HDF_DX_MAX_PACKET_SIZE				sizeof(_xCommsHeaderDX) + HDF_DX_MAX_COMMAND_DATA


	typedef	union
	{
		_uxDataType	uxDataType;
		uint8_t	ucRAW[HDF_DX_MAX_COMMAND_DATA];

	} _uxCommandData;

	typedef	struct
	{
		_xCommsHeaderDX	xCommsHeaderDX;
		_uxCommandData		uxCommandData;

	} _xCommsDX;

	

	/*
	 *
	 *
	 */


	void			m_Init						( void );
	void			m_KeywordDebugType		( ENUM_KEYWORD_TYPE enuKeyWordType );
	uint16_t		m_KeywordRX					( ENUM_KEYWORD_TYPE enuKeyWordType );
	uint16_t		m_KeyWordTX					( ENUM_KEYWORD_TYPE enuKeyWordType, uint16_t usCommand, ... );
	bool			m_KeyWordValidate			( ENUM_KEYWORD_TYPE enuKeyWordType );
	uint16_t		m_KeyWordDecode			( ENUM_KEYWORD_TYPE enuKeyWordType );
	bool			m_ValidPacket 				( ENUM_KEYWORD_TYPE enuKeywordType );


	_xCommsDX			xLoRaDX;


private:
	_xCommsDX			*pxCommsDX;
	uint8_t				*paucRX;

	uint16_t				usPacketLength;
	int8_t				pszCommsType[64];
	bool					blValidLoRaPacket;
};


/*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*
 *+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*/
} // extern "C" 
#endif // #ifdef __cplusplus
/*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*
 *+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*+*/


#endif	// HDF_DP_PROTOCOL_H


/***************************************************************************************************
 ***************************************************************************************************
 **	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!
 ***************************************************************************************************
 **************************************************************************************************/
