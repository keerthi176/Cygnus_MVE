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
*  Description  : Dongle Proxy.
*
*************************************************************************************/
#ifndef	HDF_DP_PROTOCOL_H
#define	HDF_DP_PROTOCOL_H

	 
#include	"DP_System.h"
#include	"DP_LoRa.h"
#include	"DP_Debug.h"



// Type types of transmissions.
	typedef	enum
	{
		ENU_KW_RS232 = 1,
		ENU_KW_LORA,
		ENU_KW_SOCKET,
		ENU_KW_BLUETOOTH,
		ENU_KW_CANBUS,
		ENU_KW_USB,
		ENU_KW_RS485,
		ENU_KW_NFC,
		ENU_KW_LIN
		//
	} ENUM_KEYWORD_TYPE;


		// Commands

	
#define	HDF_DX_PPU_PING_SEND						0x0200
#define	HDF_DX_PPU_PING_REPLY					0x0201
#define	HDF_DX_PPU_GET_VERSION					0x0202
#define	HDF_DX_PPU_LIST_VISIBLE_UNITS			0x0204
#define	HDF_DX_PPU_RBU_ANNOUCEMENT				0x0206
#define	HDF_DX_PPU_AT_COMMAND					0x0208
	
#define	HDF_DX_PPU_FIRMWARE_UPDATE_CONNECT	0x0280
#define	HDF_DX_PPU_FIRMWARE_UPDATE_OTA		0x0282
#define	HDF_DX_PPU_FIRMWARE_TX_PAGE			0x0286

#define	HDF_DX_PPU_FIRMWARE_REQUEST_MISSED	0x02a0
	
#define	HDF_DX_RBU_SET_LED_GPIO					0x0300

	
	
	
#define	HDF_DX_TEST_VALUE							0x0400

#define	HDF_DX_CHAR									0x0420
#define	HDF_DX_SHORT								0x0422
#define	HDF_DX_INT									0x0424
#define	HDF_DX_LONG									0x0426
#define	HDF_DX_FLOAT								0x0428
#define	HDF_DX_DOUBLE								0x042a

#define	HDF_DX_STRING								0x0430
#define	HDF_DX_STRING_RAW							0x0432
#define	HDF_DX_BYTESTREAM							0x0434

#define	HDF_DX_ENV_TEMPERATURE					0x0440
#define	HDF_DX_ENV_PRESSURE						0x0442
#define	HDF_DX_ENV_HUMIDITY						0x0444

#define	HDF_DX_PIR									0x0450
#define	HDF_DX_RADAR								0x0452
#define	HDF_DX_LUX									0x0454

#define	HDF_DX_C02									0x0460
#define	HDF_DX_C03									0x0462
#define	HDF_DX_FLAME								0x0464

#define	HDF_DX_ACCEL_XYZ							0x0470
#define	HDF_DX_ACCEL_X								0x0472
#define	HDF_DX_ACCEL_Y								0x0474
#define	HDF_DX_ACCEL_Z								0x0476

#define	HDF_DX_GYRO_XYZ							0x0480
#define	HDF_DX_GYRO_X								0x0482
#define	HDF_DX_GYRO_Y								0x0484
#define	HDF_DX_GYRO_Z								0x0486

#define	HDF_DX_GPS_LLA								0x0490
#define	HDF_DX_GPS_LONG							0x0492
#define	HDF_DX_GPS_LATT							0x0494
#define	HDF_DX_GPS_ALTI							0x0496

#define	HDF_DX_ADC									0x04a0

#define	HDF_DX_TOUCH								0x04b0

#define	HDF_DX_TIME_INTERNAL						0x04c0
#define	HDF_DX_TIME_EXTERNAL						0x04c2
#define	HDF_DX_TIME_NTP							0x04c4


	// !!!! TEST VALUES !!!!
	// !!!! TEST VALUES !!!!
#define	HDF_DX_BLIP_TEST							0x0f02	// !!!! DEBUG TEST
#define	HDF_DX_CYGNUS_PULSE						0x0f04	// !!!! DEBUG TEST


#define	HDF_DX_PPU_RESET							0x0fff
	// !!!! TEST VALUES !!!!
	// !!!! TEST VALUES !!!!








	// RESPONSES - RESPONSES - RESPONSES - RESPONSES - RESPONSES - RESPONSES
	// RESPONSES - RESPONSES - RESPONSES - RESPONSES - RESPONSES - RESPONSES
	// RESPONSES - RESPONSES - RESPONSES - RESPONSES - RESPONSES - RESPONSES
	// RESPONSES - RESPONSES - RESPONSES - RESPONSES - RESPONSES - RESPONSES


#define	HDF_DX_ERR_CHECKSUM					0x0001		// Checksum error.
#define	HDF_DX_ERR_COMMAND_UNKNOWN			0x0002		// Unknown command (command in lower 3 nibbles).
#define	HDF_DX_ERR_UNSUPPORTED_FEATURE	0x0004		// Device doesn't support this feature (ie: GPS, PIR, External Clock, etc).
#define	HDF_DX_ERR_BUSY						0x0008		// Device busy, try command request again.
#define	HDF_DX_ERR_SYS_MODE_REQUIRED		0x0010		// Command needs to be sent in "Security Mode".
#define	HDF_DX_ERR_INVALID_PARAM_COUNT	0x0020		// Invalid parameter count for command.
#define	HDF_DX_ERR_INVALID_PACKET_SIZE	0x0040		// Invalid data packet size.


#define	HDF_DX_BIT_REQUEST_DATA				0x1000		// [Bit #13] == Request data
#define	HDF_DX_BIT_REPLY_WITH_OK			0x4000		// [Bit #14] == Command OK (sent back to requester with this bit set, if requested).
#define	HDF_DX_BIT_REPLY_WITH_ERROR		0x8000		// [Bit #15] == Error code - stored in first int16_t of command data recieved.


			// Mask onto TX command in reply.
#define	HDF_DX_MASK_CMD_STATUS_NIBBLE		0xf000		// For masking the top nibble to check for RX response

			// Strips command without status nibble.
#define	HDF_DX_MASK_COMMANDS					0x0fff		// No top nibble set == only recieving command with data



#define	HDF_DX_MAX_BLIP_TEXT					64				// Maximum characters in HDF_DX_BLIP_TEST
#define	HDF_DX_MAX_BYTE_STREAM				160


#define	HDF_DX_MAX_LORA_RAW_PACKET			255			// BYTES - Max LoRa Transmission 
#define	HDF_DX_MAX_CYGNUS_PACKET			224			// BYTES - For Cynus Packet transfer.
#define	HDF_DX_OTA_PAYLOAD_MAX				128			//
#define	HDF_DX_AT_BUFFER_MAX					128			//


	// 


#define	HDF_PAGES_MISSING16					16				// Amount of blocks missing when requested after firmaware update (16 per request).




	typedef	struct
	{
		uint16_t		usDongle_ID;
		uint16_t		usRBU_ID;
		uint32_t		uiSystem_ID;
		uint32_t		uiSerialNumber;
		//
	} _xAnnoucementRBU;




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
		uint16_t	usDeviceID;
		//
	} _xPing;
	
	typedef	struct
	{
		int8_t	acBuffer[HDF_DX_AT_BUFFER_MAX];
		//
	} _xAT_Command;
	
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

	
	typedef	struct
	{
		int8_t	pszCommand[HDF_DX_MAX_BYTE_STREAM];
		int8_t	cNULL;
		//
	} _xCommandAT;

	

	


	
	typedef	struct
	{
		int32_t	iDataSize;
		int		iPacketSize;
	} _xPPU_Connect;

	
	typedef	struct
	{
		uint16_t				usDeviceID;					// Device we are sending to (0xffff == Broadcastto all).
		uint32_t				uiDataSizeTotal;			// Size of the whole binary to transmit.

		uint16_t				usPagesTotal;				// How many pages to transmit.
		uint16_t				usPageNumber;				// Current page number.
		uint16_t				usPageSize;					// Page size.

		uint8_t				ucBlocksPerPage;			// How many blocks per page.
		uint8_t				ucBlockNumber;				// Current block number.
		uint8_t				ucBlockSize;				// Block size.
		//
	} _xFlashMemory;

	
	typedef	struct
	{
		uint16_t				usDongleID;
		uint16_t				usRBU_ID;					// Device we are sending to (0xffff == Broadcastto all).

		uint32_t				uiDataSizeTotal;			// Size of the whole binary to transmit.

		uint16_t				usPageNumber;				// Current page number.
		uint16_t				usPageSize;					// Page size.

		uint8_t				aucPayload[HDF_DX_OTA_PAYLOAD_MAX];
		//
	} _xDataDX;


	

	
	/* MUST be a multiple of #4 - else the compiler will pad #zeros to next data-type after the structure,
	 * which must not happen after this structure.
	 * As this is for a linear transmission & would waste bytes.
	 */
	typedef	struct
	{
		uint16_t		usHostID;				// 0x8000 - Bit 15 set in the ID == PPU
		uint16_t		usClientID;				// 0xffff == All
		uint32_t		uiSerialNumber;
		uint16_t		usChecksum;
		uint16_t		usPacketLength;
		uint16_t		usCommand;
		//
	} _xCommsHeaderDX;
	/* MUST be a multiple of #4 bytes, else the compiler will pad #zeros into the structure's data-type,
	 * which must not happen after this structure.
	 * As this is for a linear transmission & would waste bytes.
	 *
	 * MUST be a multiple of 8 for encryption of the packet.
	 */

	
	
	typedef	struct
	{
		uint16_t	iMissing;
		uint16_t	ausPages[HDF_PAGES_MISSING16];
		//
	} _xPacketsMissing;

	
	// A union of all data-types that we transmit, so the data packet code can validly access all elements.
	// A union of all data-types that we transmit, so the data packet code can validly access all elements.
	typedef	union
	{
		int8_t				cChar;
		uint8_t				ucChar;
		int16_t				sShort;
		uint16_t				usShort;
		int32_t				lLong;
		uint32_t				ulLong;
		float					fFloat;
		double				dDouble;

		uint16_t				usErrorCode;
		uint16_t				usTestValue;
		int16_t				sLux;

		_xBlipTest			xBlipTest;			// !!!! DEBUG !!!! - For testing transmission of different data packet sizes.
		_xCygnusPulse		xCygnusPulse;		// !!!! DEBUG !!!! - For testing transmission of different data packet sizes.

		
		_xDataDX				xDataDX;
		_xByteStream		xByteStream;
		_xPacketsMissing	xPacketsMissing;
		_xPing				xPing;
		_xAT_Command		xAT_Command;
		
	} _uxDataType;
	// A union of all data-types that we transmit, so the data packet code can validly access all elements.
	// A union of all data-types that we transmit, so the data packet code can validly access all elements.

	
	
	
	
	

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
		//
	} _xCommsDX;



	/*
	 *
	 *
	 */

	
	

	// !!!! DEBUG TEST FUNCTIONS !!!!
	// !!!! DEBUG TEST FUNCTIONS !!!!

	void			DP_Protocol_BlipTest 					( uint32_t uiBlip );
	void			DP_Protocol_LED_GPIO 					( uint8_t ucLED_GPIO );

	// !!!! DEBUG TEST FUNCTIONS !!!!
	// !!!! DEBUG TEST FUNCTIONS !!!!
	

	void			DP_Protocol_Init								( void );
	void			DP_Protocol_KeywordDebugType				( ENUM_KEYWORD_TYPE enuKeyWordType );
	uint16_t		DP_Protocol_KeywordRX						( ENUM_KEYWORD_TYPE enuKeyWordType );
	//uint16_t		DP_Protocol_KeyWordTX_Variadic			( ENUM_KEYWORD_TYPE enuKeyWordType, uint16_t usCommand, ... );
	uint16_t		DP_Protocol_KeyWordTX 						( ENUM_KEYWORD_TYPE enuKeyWordType, _xCommsDX *pxCommandTX, uint16_t usClientID );
	bool			DP_Protocol_KeyWordValidate				( ENUM_KEYWORD_TYPE enuKeyWordType );
	uint16_t		DP_Protocol_KeyWordDecode					( ENUM_KEYWORD_TYPE enuKeyWordType );
	bool			DP_Protocol_ValidPacket 					( ENUM_KEYWORD_TYPE enuKeyWordType );

	void			DP_Protocol_SendBuffer 						( uint16_t usDeviceID, uint16_t *paucBuffer, uint8_t iBufferSize );
	
	bool			DP_Protocol_SetPPU_ID 						( uint16_t usDeviceID );
	bool			DP_Protocol_SetRBU_ID 						( uint16_t usDeviceID );
	void			DP_Protocol_SetDeviceSerialNumber		( uint16_t usDeviceID, uint32_t uiSerial );




	void			DP_Protocol_SendOTA 							(  uint16_t usDeviceID, uint32_t iDataSizeTotal,
																		uint16_t uiPagesTotal, uint16_t uiPageNumber, uint16_t uiPageSize,
																		uint8_t ucBlockNumber, uint8_t ucBlockSize,
																		uint8_t *paucPageDataPayload );

					// RBU ID or 0xffff to broadcast to all.
	void			DP_Protocol_Broadcast_StartOTA			( uint16_t usDeviceID );
	void			DP_Protocol_Broadcast_MemPageMask		( uint16_t usDeviceID );

	void			DP_Protocol_Request_MemPageMask			( uint16_t usDeviceID );
	

	void			DP_Protocol_Reply_Ping 						( void );
	void			DP_Protocol_Reply_VisibleUnits 			( void );
	void			DP_Protocol_Reply_Version 					( void );
	int16_t		DP_Protocol_Reply_GetMissingUpdatePackets	( void );


	void			DP_Protocol_PPU_Connect 					( void );
	void			DP_Protocol_PPU_Disconnect 				( void );
	bool			DP_Protocol_PPU_Connected 					( void );

#endif	// HDF_DP_PROTOCOL_H


/***************************************************************************************************
 ***************************************************************************************************
 **	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!
 ***************************************************************************************************
 **************************************************************************************************/
