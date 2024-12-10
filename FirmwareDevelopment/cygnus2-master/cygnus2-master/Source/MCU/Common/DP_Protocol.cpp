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
*  File         : DP_Protocol.cpp
*
*  Description  :
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/

#include	"string.h"

#include	"DP_Protocol.h"



/**************************************************************************************************
 **************************************************************************************************
 ** @brief	: Constructor
 ** @param	:
 ** @return	:
 **************************************************************************************************
 **************************************************************************************************/
CDP_Protocol::CDP_Protocol()
{
	blValidLoRaPacket = false;
}




/**************************************************************************************************
 **************************************************************************************************
 ** @brief	:
 ** @param	:
 ** @return	:
 **************************************************************************************************
 **************************************************************************************************/
CDP_Protocol::~CDP_Protocol()
{
}





/*************************************************************************************************
 **************************************************************************************************
 * @member	: m_KeywordDebugType
 * @brief	: Debug output, debug messages & point to approriate structure to receive data.
 * @param	: Which KeyWord interface. eg: Socket, LoRa, 232, etc...
 * @return	: NONE
 **************************************************************************************************
 **************************************************************************************************/
void	CDP_Protocol::m_KeywordDebugType ( ENUM_KEYWORD_TYPE enuKeyWordType )
{
	if ( ENU_KW_LORA == enuKeyWordType )
	{
		pxCommsDX = &xLoRaDX;
		strcpy( (char*)pszCommsType, (char*)"LoRa_" );
		enuDebugOutputType = ENU_DEBUG_LORA;
	}
	else
	{
		strcpy( (char*)pszCommsType, (char*)"????_" );
	}
}




/**************************************************************************************************
 **************************************************************************************************
 ** @member	: KeyWordRX
 ** @brief  : Recieve data packet.
 ** @param	: Command type.
 ** @param	: Command data.
 ** @return	: Packet Length
 **************************************************************************************************
**************************************************************************************************/
uint16_t	CDP_Protocol::m_KeywordRX ( ENUM_KEYWORD_TYPE enuKeyWordType )
{
	int8_t		acBuffer[128];
	bool			blValidHeader;
	int16_t		usOK;

	
	m_KeywordDebugType( enuKeyWordType );

	paucRX = (uint8_t*)pxCommsDX;
	blValidHeader = false;
	usOK = 0;



	// ?? Vaildate LoRa KeyWord header ??
	if ( enuKeyWordType == ENU_KW_LORA )
	{
		// YES! 
		//usPacketLength = (uint16_t)LoRa.parsePacket();
		//obLoRaDX.iRSSI = LoRa.packetRssi();
		obLoRaDX.m_ParsePacket();
		obLoRaDX.iRSSI = obLoRaDX.m_GetRSSI();

		// Read packet
		while ( obLoRaDX.m_Available() && usPacketLength > 0 )
		{
			if ( usPacketLength <= sizeof( _xCommsDX ) )
			{
				sprintf( (char*)acBuffer, (char*)"\n%sRX: >>>>>>  START - %s", pszCommsType, HDF_DONGLE_BUILD_NAME );
				pobDebug->m_PrintCR( enuDebugOutputType, acBuffer );

				//LoRa.readBytes( paucRX, usPacketLength );
				//obLoRaDX.m_ReadBytes( paucRX, usPacketLength );
				//pobBase->m_BlinkCommsLED( ENU_LED_BLINK_LORA_RX );
				
				// !!!!!!  DEBUG CHECK!
			}
			else
			{
				sprintf( (char*)acBuffer, "\n\n\n%sRX: Length = %d OVERSIZED - ERR!\n\n", pszCommsType, usPacketLength );
				pobDebug->m_PrintCR( enuDebugOutputType, acBuffer );
			}

			sprintf( (char*)acBuffer, "%sRX: LoRa Size = %d", pszCommsType, usPacketLength );
			pobDebug->m_PrintCR( enuDebugOutputType, acBuffer );

			sprintf( (char*)acBuffer, "%sRX: RSSI = %d", pszCommsType, obLoRaDX.iRSSI );
			pobDebug->m_PrintCR( enuDebugOutputType, acBuffer );

			blValidHeader = true;
		}
	} // ?? Vaildate LoRa KeyWord header ??
	else
	{
		pobDebug->m_PrintCR( enuDebugOutputType, (int8_t*)"UNKNOWN RX - ERR!" );
	}



	// ?? Valid header KeyWord. ??
	if ( blValidHeader )
	{
		// ?? Valid keyword packet. ??
		if ( m_KeyWordValidate( enuKeyWordType ) == true )
		{
			// Decode packet we've just recieved - & act on it.
			usOK = m_KeyWordDecode( enuKeyWordType );

			sprintf( (char*)acBuffer, (char*)"%sRX: >>>>>>  END - %s\n\n", pszCommsType, HDF_DONGLE_BUILD_NAME );
			pobDebug->m_PrintCR( enuDebugOutputType, acBuffer  );
		}
		else
		{
			usOK = 0;
		}

		if ( usOK )
		{
			blValidLoRaPacket = enuKeyWordType == ENU_KW_LORA ? true : false;
		}
	}
	return ( usOK );
}




/**************************************************************************************************
 **************************************************************************************************
 ** @member	: m_ValidPacket
 ** @brief	: Valid data packet recieved
 ** @param	: ENU_KW_SOCKET or ENU_KW_LORA.
 ** @return	: TRUE == Valid : FALSE == Invalid.
 **************************************************************************************************
 **************************************************************************************************/
bool	CDP_Protocol::m_ValidPacket ( ENUM_KEYWORD_TYPE enuKeywordType )
{
bool	blValid;

	if ( ENU_KW_LORA == enuKeywordType )
	{
		blValid = blValidLoRaPacket;
	}
	else	// Should never get here, as parameter is enumerated.
	{
		blValid = false;
	}

	return ( blValid );
}



/**************************************************************************************************
 **************************************************************************************************
 ** @member	: m_KeyWordValidate
 ** @brief	: Validate a recieved Keyword packet.
 ** @param	: ENU_KW_SOCKET or ENU_KW_LORA.
 ** @Out		: TRUE == VALID  -  FALSE == INVALID.
 **************************************************************************************************
 *************************************************************************************************/
bool	CDP_Protocol::m_KeyWordValidate	( ENUM_KEYWORD_TYPE enuKeyWordType )
{
int8_t		acBuffer[128];
bool			blValidKeyWord;
uint16_t		usChecksum, usCheckTemp;
int16_t		i;


	m_KeywordDebugType( enuKeyWordType );


	blValidKeyWord = false;
	usChecksum = 0;
	usCheckTemp = pxCommsDX->xCommsHeaderDX.usChecksum;
	pxCommsDX->xCommsHeaderDX.usChecksum = 0;

	// ENU_DEBUG_SOCKET
	pobDebug->m_Print( enuDebugOutputType, pszCommsType );
	pobDebug->m_Print( enuDebugOutputType, (int8_t*)"RX: " );

	for ( i = 0; i < usPacketLength; i++ )
	{
		usChecksum += (char)paucRX[i];

		sprintf( (char*)acBuffer, (char*)"%.2X ", paucRX[i] );
		pobDebug->m_Print( enuDebugOutputType, acBuffer );
	}
	pxCommsDX->xCommsHeaderDX.usChecksum = usCheckTemp;




	// process & validate that packet is a KEYWORD packet.
	if ( pxCommsDX->xCommsHeaderDX.usPacketLength == usPacketLength )
	{
		// ?? Valid checksum ??
		if ( usChecksum == pxCommsDX->xCommsHeaderDX.usChecksum )
		{
			// Device ID & CMD.
			sprintf( (char*)acBuffer, (char*)"\n%sRX: CMD: [%.4X]  -  From Device = [%.4X]",
						pszCommsType,
						pxCommsDX->xCommsHeaderDX.usCommand,
						pxCommsDX->xCommsHeaderDX.usDeviceID
						);
			pobDebug->m_Print( enuDebugOutputType, acBuffer );

			// Checksum Recieved & Checksum Calculated (OK).
			sprintf( (char*)acBuffer, (char*)"%sRX: CHK VAL = [%.4X]  -  CHK CALC = [%.4X]  -  CHK OK",
						pszCommsType,
						pxCommsDX->xCommsHeaderDX.usChecksum,
						usChecksum
					);
			pobDebug->m_PrintCR( enuDebugOutputType, acBuffer  );
			blValidKeyWord = true;
		}
		else
		{
			// Checksum Recieved & Checksum Calculated (ERROR)
			sprintf( (char*)acBuffer, (char*)"%sRX: CHK VAL = [%.4X]  -  CHK CALC = [%.4X]  -  CHK = ERR!",
						pszCommsType,
						pxCommsDX->xCommsHeaderDX.usChecksum,
						usChecksum
					);
			pobDebug->m_PrintCR( enuDebugOutputType, acBuffer  );
		}
	} // if ( pxCommsDX->xCommsHeaderDX.usPacketLength == usPacketLength )
	else
	{
		//
		sprintf( (char*)acBuffer, (char*)"\n%sRX: ------ NON KEYWORD PACKET - Length! (%d)  -  ERR!", pszCommsType, usPacketLength );
		pobDebug->m_PrintCR( enuDebugOutputType, acBuffer  );
	}

	return ( blValidKeyWord );
}




/**************************************************************************************************
 **************************************************************************************************
 ** @member	: m_KeyWordDecode
 ** @brief	: Decode & process a recieved Keyword data packet.
 ** @param	: Keyword type to decode - ENU_KW_SOCKET or ENU_KW_LORA.
 ** @return	: Valid Keyword packet - TRUE/FALSE.
 **************************************************************************************************
 *************************************************************************************************/
uint16_t CDP_Protocol::m_KeyWordDecode	( ENUM_KEYWORD_TYPE enuKeyWordType  )
{
uint16_t				usOK;
int8_t				acBuffer[128];
int8_t				pszError[32];
uint16_t				usRX_Command;


	m_KeywordDebugType( enuKeyWordType );


	usRX_Command = pxCommsDX->xCommsHeaderDX.usCommand;

	usOK = usRX_Command;

	// ?? ERROR CHECK - Have we recieved a command the with error-code HDF_DX_BIT_REPLY_WITH_ERROR bit SET ??
	if ( HDF_DX_BIT_REPLY_WITH_ERROR == (usRX_Command & HDF_DX_MASK_CMD_STATUS_NIBBLE) )
	{
		switch ( pxCommsDX->uxCommandData.uxDataType.usErrorCode )
		{
		case HDF_DX_ERR_CHECKSUM:					// 0x0001
			strcat( (char*)pszError, "CHECKSUM!" );
			usOK = 0;
			break;
		
		case HDF_DX_ERR_COMMAND_UNKNOWN:			// 0x0002
			strcat( (char*)pszError, "UNKNOWN COMMAND!" );
			usOK = 0;
			break;
		
		case HDF_DX_ERR_UNSUPPORTED_FEATURE:	// 0x0004
			strcat( (char*)pszError, "UNSUPPORTED FEATURE!" );
			usOK = 0;
			break;
		
		case HDF_DX_ERR_BUSY:						// 0x0008
			strcat( (char*)pszError, "BUSY - TRY AGAIN!" );
			usOK = 0;
			break;
		
		case HDF_DX_ERR_SYS_MODE_REQUIRED:		// 0x0010
			strcat( (char*)pszError, "SYSTEM MODE NEEDED!" );
			usOK = 0;
			break;
		
		case HDF_DX_ERR_INVALID_PARAM_COUNT:	// 0x0020
			strcat( (char*)pszError, "INVALID PARAM COUNT?!" );
			usOK = 0;
			break;
		
		case HDF_DX_ERR_INVALID_PACKET_SIZE:	// 0x0040
			strcat( (char*)pszError, "INVALID PACKET SIZE!" );
			usOK = 0;
			break;

		default:
			strcat( (char*)pszError, "UNKNOWN ERROR CODE!" );
			usOK = 0;
			break;
		}

		sprintf( (char*)acBuffer, (char*)"%s: ERROR = <<<< %s >>>>", pszCommsType, pszError );
		pobDebug->m_PrintCR( enuDebugOutputType, acBuffer );
	}
	// ?? REQUEST CHECK - Was bit #12 set to indicate that data requested & needs a reply ??
	else if ( HDF_DX_BIT_REQUEST_DATA ==(usRX_Command & HDF_DX_MASK_CMD_STATUS_NIBBLE) )
	{
		// Recieved a request for command data (eg: current device temperature).
		sprintf( (char*)acBuffer, (char*)"%s_RX: Device [%.4X] Requested Data For CMD [%.4X]  -  CMD = [%.4X]",
					pszCommsType,
					pxCommsDX->xCommsHeaderDX.usDeviceID,
					usRX_Command,
					usRX_Command & HDF_DX_MASK_COMMANDS
				);
		pobDebug->m_PrintCR( enuDebugOutputType, acBuffer );


			/*
			 * DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND.
			 * DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND.
			 * DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND.
			 * DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND.
			 */

		// Strip top nibble mask & check which command request we've recieved.
		switch ( HDF_DX_MASK_COMMANDS & usRX_Command )
		{
		case HDF_DX_TEMPERATURE:
			// TX (Socket or LoRa)

			// !! DEBUG - Fixed value "26.2", just testing a value return !!
			m_KeyWordTX( enuKeyWordType, HDF_DX_TEMPERATURE | HDF_DX_BIT_REPLY_WITH_OK, 26.2 );
			break;
		
		
		default:
			// Unknown request = reply with requested command having HDF_DX_BIT_REPLY_WITH_ERROR bit set
			sprintf( (char*)acBuffer, (char*)"%s_RX: From Device [%.4X]  -  Request UNKNOWN!!! [%.4X]", pszCommsType, pxCommsDX->xCommsHeaderDX.usDeviceID, usRX_Command );
			pobDebug->m_PrintCR( enuDebugOutputType, acBuffer );

			// TX (Socket or LoRa)
			m_KeyWordTX( enuKeyWordType, (usRX_Command & HDF_DX_MASK_COMMANDS) | HDF_DX_ERR_COMMAND_UNKNOWN | HDF_DX_BIT_REPLY_WITH_ERROR );
			usOK = 0;
		}

			/*
			 * DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND.
			 * DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND.
			 * DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND.
			 * DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND.
			 */
	}
	// Process HDF_DX_BIT_REPLY_WITH_OK (for debug output)
	else if ( (HDF_DX_MASK_CMD_STATUS_NIBBLE & usRX_Command) & HDF_DX_BIT_REPLY_WITH_OK )
	{
		// Recieved OK mask (HDF_DX_BIT_REPLY_WITH_OK) set in response to last command sent
		sprintf( (char*)acBuffer, (char*)"%s_RX: [%.4X] = {{ ACK OK }} From Device [%.4X]  -  [%.4X]",
					pszCommsType,
					usRX_Command,
					pxCommsDX->xCommsHeaderDX.usDeviceID,
					usRX_Command & HDF_DX_MASK_COMMANDS
				);
		pobDebug->m_PrintCR( enuDebugOutputType, acBuffer );
	}
	// Precess 'ONE SHOT' CMD - HDF_DX_MASK_CMD_STATUS_NIBBLE command status bit not set
	else if ( (HDF_DX_MASK_CMD_STATUS_NIBBLE & usRX_Command) == 0 )
	{
		// Recieved 'ONE SHOT' CMD - sent data (eg: movement sensor from another device)
		sprintf( (char*)acBuffer, (char*)"%sRX: ONE SHOT CMD [%.4X] = ",
					pszCommsType,
					usRX_Command );
		pobDebug->m_Print( enuDebugOutputType, acBuffer );


		switch ( usRX_Command  )
		{
		case HDF_DX_BLIP_TEST:
			pobDebug->m_PrintCR( enuDebugOutputType, (int8_t*)"BLIP_TEST1" );
			break;

		case	HDF_DX_CYGNUS_PULSE:
			pobDebug->m_PrintCR( enuDebugOutputType, (int8_t*)"CYGNUS_PULSE" );
			break;

		default:
			// Unknown 'ONE SHOT' CMD - received with no status bit set
			sprintf( (char*)acBuffer, (char*)"From Device [%.4X]  -  Request UNKNOWN!!! [%.4X]", pxCommsDX->xCommsHeaderDX.usDeviceID, usRX_Command );
			pobDebug->m_PrintCR( enuDebugOutputType, acBuffer );

			// ?? Socket or LoRa ??
			m_KeyWordTX( enuKeyWordType, usRX_Command | HDF_DX_BIT_REPLY_WITH_ERROR );
			usOK = 0;
			break;
		}
	}
	else
	{
		// Should never, never get here, as all commands are either defined or captured if they're not.
		sprintf( (char*)acBuffer, (char*)"%sRX: WTF ~ [%.4X]  -  From Device [%.4X]", pszCommsType, pxCommsDX->xCommsHeaderDX.usCommand, pxCommsDX->xCommsHeaderDX.usDeviceID );
		pobDebug->m_PrintCR( enuDebugOutputType, acBuffer );
		usOK = 0;
	}

	return ( usOK );
}




/**************************************************************************************************
 **************************************************************************************************
 ** @brief	: Process & transmit a KeyWord command.
 ** @param	: ENUM_KEYWORD_TYPE Transmition type - ENU_KW_SOCKET or ENU_KW_LORA, etc...
 ** @param	: int16_t* Command.
 ** @param	: Command Data/Parameters.
 ** @return	: uint16_t Command Length.
 **************************************************************************************************
 *************************************************************************************************/
uint16_t	CDP_Protocol::m_KeyWordTX ( ENUM_KEYWORD_TYPE enuKeyWordType, uint16_t usCommand, ... )
{
	va_list	vaArgsList;
	int16_t	sCount;
	int16_t	sCommandLength;
	int8_t	acTextBuff[128];
	uint8_t	*paucBuffer;
	
	
/*
	!! CAUTION !!

	The results of va_start are unpredictable if the argument values are not appropriate.
	In certain cases, arguments are converted when they are passed to another type.
	For instance, char and short arguments are converted to int , float to double , and array to pointer.
	When parameters of this sort are expected, va_arg must be issued with the type after conversion.

	FOR EXAMPLE:
		va_arg(ap, float) will fail to access a float argument value correctly, so you should use va_arg(ap, double).
		va_arg(ap, char) will fail to access a char argument value correctly, so you should use va_arg(ap, int).

	There is no way to test whether a particular argument is the last one in the list.
	Attempting to access arguments after the last one in the list produces unpredictable results.

	~~ PEMBO! ~~
*/

	// Header type, for debug output.
	m_KeywordDebugType( enuKeyWordType );


	sprintf( (char*)acTextBuff, (char*)"\n\n%sTX: >>>>>>  START - %s", pszCommsType, HDF_DONGLE_BUILD_NAME );
	pobDebug->m_PrintCR( enuDebugOutputType, acTextBuff );


	sCommandLength = 0;
	va_start( vaArgsList, usCommand );

	sprintf( (char*)acTextBuff, (char*)"%sTX: COMMAND = %.4X ", pszCommsType, usCommand );
	pobDebug->m_PrintCR( enuDebugOutputType, acTextBuff );


	// ?? Are we sending a reply with the ERROR bit set (HDF_DX_BIT_REPLY_WITH_ERROR) ??
	if ( (HDF_DX_MASK_CMD_STATUS_NIBBLE & usCommand) == HDF_DX_BIT_REPLY_WITH_ERROR )
	{
		sprintf( (char*)acTextBuff, (char*)"%sTX: REPLY - ERROR CODE = %.4X ", pszCommsType, usCommand );
		pxCommsDX->uxCommandData.uxDataType.usErrorCode = (int16_t)va_arg( vaArgsList, int32_t );
		pobDebug->m_PrintCR( enuDebugOutputType, acTextBuff );
		sCommandLength += sizeof( int16_t );
	}
	// ?? Are we sending a reply with the OK bit set (HDF_DX_BIT_REPLY_WITH_OK) ??
	else if ( (HDF_DX_MASK_CMD_STATUS_NIBBLE & usCommand) == HDF_DX_BIT_REPLY_WITH_OK )
	{
		sprintf( (char*)acTextBuff, (char*)"%sTX: REPLY - {{ ACK OK }} = %.4X ", pszCommsType, usCommand );
		pobDebug->m_PrintCR( enuDebugOutputType, acTextBuff );
	}
	// We're requesting data or sending a one shot command.
	else
	{
		sprintf( (char*)acTextBuff, (char*)"%sTX: ", pszCommsType );
		pobDebug->m_Print( enuDebugOutputType, acTextBuff );


		// ?? If bit #12 set we are requesting data from another device, else sending data.
		sprintf( (char*)acTextBuff, (char*)"%s", usCommand & HDF_DX_BIT_REQUEST_DATA ? (char*)"REQUEST " : "ONE SHOT SEND " );
		pobDebug->m_Print( enuDebugOutputType, acTextBuff );

		// Mask away status/info nibble to decode command.
		switch ( HDF_DX_MASK_COMMANDS & usCommand )
		{
		case HDF_DX_INT:
			pobDebug->m_PrintCR( enuDebugOutputType, (int8_t*)"uint16_t" );
			pxCommsDX->uxCommandData.uxDataType.usShort = (int16_t)va_arg( vaArgsList, uint32_t );
			sCommandLength += sizeof( int16_t );
			break;


		case HDF_DX_FLOAT:
			pobDebug->m_PrintCR( enuDebugOutputType, (int8_t*)"FLOAT" );
			pxCommsDX->uxCommandData.uxDataType.fFloat = (float)va_arg( vaArgsList, double );
			sCommandLength += sizeof( float );
			break;


		case HDF_DX_STRING:
				paucBuffer = (uint8_t*)va_arg( vaArgsList, uint32_t );
				pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount = strlen( (char*)paucBuffer );

				sprintf( (char*)acTextBuff, (char*)"STRING LEN = %d - [ %s ]", pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount, paucBuffer );
				pobDebug->m_PrintCR( enuDebugOutputType, acTextBuff );

				// String size check.
				if ( HDF_DX_MAX_COMMAND_DATA > pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount )
				{
					for ( sCount = 0; sCount < pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount; sCount++ )
					{
						pxCommsDX->uxCommandData.uxDataType.xByteStream.aucBytes[sCount] = paucBuffer[sCount];
					}

					sCommandLength += (pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount + sizeof(pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount));
				}
				else
				{
					// !! ERRRM!
					pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount = 0;
					pobDebug->m_PrintCR( enuDebugOutputType, (int8_t*)"STRING - ERR!" );
				}
			break;


		case HDF_DX_STRING_RAW:
				paucBuffer = (uint8_t*)va_arg( vaArgsList, uint32_t );
				pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount = strlen( (char*)paucBuffer );
				sCommandLength += sizeof(pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount);

				sprintf( (char*)acTextBuff, (char*)"STRING RAW ( %d )", pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount );
				pobDebug->m_PrintCR( enuDebugOutputType, acTextBuff );

				// String size check.
				if ( HDF_DX_MAX_COMMAND_DATA > pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount )
				{
					for ( sCount = 0; sCount < pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount; sCount++ )
					{
						pxCommsDX->uxCommandData.uxDataType.xByteStream.aucBytes[sCount] = paucBuffer[sCount];
					}

					sCommandLength += pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount;
				}
				else
				{
					// !! ERRRM!
					pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount = 0;
					pobDebug->m_PrintCR( enuDebugOutputType, (int8_t*)"STRING RAW - ERR!" );
				}
			break;


		case HDF_DX_BYTESTREAM:
				// Get byte count
				pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount = (int8_t)va_arg( vaArgsList, uint32_t );
				sCommandLength += sizeof(pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount);

				sprintf( (char*)acTextBuff, (char*)"BYTESTREAM ( %d )", pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount );
				pobDebug->m_PrintCR( enuDebugOutputType, acTextBuff );

				// Bytestream size check.
				if ( HDF_DX_MAX_COMMAND_DATA > pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount )
				{
					for ( sCount = 0; sCount < pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount; sCount++ )
					{
						pxCommsDX->uxCommandData.uxDataType.xByteStream.aucBytes[sCount] = (int8_t)va_arg( vaArgsList, uint32_t );
					}
					sCommandLength += pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount + 1;
				}
				else
				{
					// !! ERRRM!
					pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount = 0;
					pobDebug->m_PrintCR( enuDebugOutputType, (int8_t*)"BYTESTREAM!" );
				}
			break;

				
				// CYGNUS DATA-SIZE TRANSFER TEST!!!!
				// CYGNUS DATA-SIZE TRANSFER TEST!!!!
		case HDF_DX_CYGNUS_PULSE:
			sprintf( (char*)acTextBuff, (char*)"CYGNUS PULSE"  );
			pobDebug->m_PrintCR( enuDebugOutputType, acTextBuff );

			// Runninmg Counter.
			pxCommsDX->uxCommandData.uxDataType.xCygnusPulse.iCounter = va_arg( vaArgsList, uint32_t );
			sCommandLength += sizeof( uint16_t );
		
			// Size of data-packet to send (in bytes).
			pxCommsDX->uxCommandData.uxDataType.xCygnusPulse.iDataSize = va_arg( vaArgsList, uint32_t );
			sCommandLength += sizeof( uint16_t );
			
			// Data array.
			paucBuffer = (uint8_t*)va_arg( vaArgsList, uint32_t );
			if ( HDF_DX_MAX_CYGNUS_PACKET > pxCommsDX->uxCommandData.uxDataType.xCygnusPulse.iDataSize )
			{
				for ( sCount = 0; sCount < pxCommsDX->uxCommandData.uxDataType.xCygnusPulse.iDataSize; sCount++ )
				{
					pxCommsDX->uxCommandData.uxDataType.xCygnusPulse.acData[sCount] = paucBuffer[sCount];
				}
			}
			else
			{
				// !! ERRRM!
				pxCommsDX->uxCommandData.uxDataType.xByteStream.usByteCount = 0;
				pobDebug->m_PrintCR( enuDebugOutputType, (int8_t*)"CYGNUS PULSE" );
			}
		
			break;
				// CYGNUS DATA-SIZE TRANSFER TEST!!!!
				// CYGNUS DATA-SIZE TRANSFER TEST!!!!

		
		case HDF_DX_BLIP_TEST:
			sprintf( (char*)acTextBuff, (char*)"BLIP_TEST"  );
			pobDebug->m_PrintCR( enuDebugOutputType, acTextBuff );
			// Interger value.
			pxCommsDX->uxCommandData.uxDataType.xBlipTest.iCounter = va_arg( vaArgsList, uint32_t );
			sCommandLength += sizeof( uint16_t );

			// Pointer to passed string.
			paucBuffer = (uint8_t*)va_arg( vaArgsList, uint32_t );

			if ( HDF_DX_MAX_BLIP_TEXT > strlen( (char*)paucBuffer ) )
			{
				strcpy( (char*)pxCommsDX->uxCommandData.uxDataType.xBlipTest.pszString, (char*)paucBuffer );
				sCommandLength += strlen( (char*)pxCommsDX->uxCommandData.uxDataType.xBlipTest.pszString );

				sCommandLength++;	// For NULL Terminator.

				sprintf( (char*)acTextBuff, "%sTX: BLIP = \'%s\' - LENGTH: %d", pszCommsType, pxCommsDX->uxCommandData.uxDataType.xBlipTest.pszString, strlen( (char*)pxCommsDX->uxCommandData.uxDataType.xBlipTest.pszString) );
				pobDebug->m_PrintCR( enuDebugOutputType, acTextBuff );
			}
			else
			{
				// !! ERRRM!
				sCommandLength = 0;

				sprintf( (char*)acTextBuff, (char*)"%sTX: BLIP SIZE - ERR!", pszCommsType );
				pobDebug->m_PrintCR( enuDebugOutputType, acTextBuff );
			}
			break;


		case HDF_DX_BIT_REPLY_WITH_ERROR:
			pxCommsDX->uxCommandData.uxDataType.usErrorCode = va_arg( vaArgsList, uint32_t );
			sprintf( (char*)acTextBuff, (char*)"ERROR CODE = %.4X", pxCommsDX->uxCommandData.uxDataType.usErrorCode  );
			pobDebug->m_PrintCR( enuDebugOutputType, acTextBuff );
			sCommandLength += sizeof( int16_t );
			break;


		default:
			sprintf( (char*)acTextBuff, (char*)"Unknown KeyWord CMD!!!   %.4X - ERR!\n", usCommand  );
			pobDebug->m_PrintCR( enuDebugOutputType, acTextBuff );
			sCommandLength = -1;
			break;
		} // switch ( usCommand & HDF_DX_MASK_COMMANDS )
	}
	va_end( vaArgsList );


	// Positive value == OK to process
	if ( 0 <= sCommandLength )
	{
		// Build protocol header.
		pxCommsDX->xCommsHeaderDX.usDeviceID = HDF_DONGLE_ID;
		pxCommsDX->xCommsHeaderDX.usDeviceID_SendingTo = 0xffff;		//	ALL UNITS (For Now) !!!!
		pxCommsDX->xCommsHeaderDX.usChecksum = 0;
		pxCommsDX->xCommsHeaderDX.usPacketLength = sizeof( _xCommsHeaderDX ) + sCommandLength;
		pxCommsDX->xCommsHeaderDX.usCommand = usCommand;

		uint16_t	usChecksum;
		usChecksum = 0;
		paucBuffer = (uint8_t*)pxCommsDX;

		// Calculate checksum.
		sprintf ( (char*)acTextBuff, (char*)"%sTX: ", pszCommsType );
		pobDebug->m_Print( enuDebugOutputType, acTextBuff );

		for ( uint16_t i = 0; i < pxCommsDX->xCommsHeaderDX.usPacketLength; i++ )
		{
			usChecksum += (uint8_t)paucBuffer[i];
			sprintf ( (char*)acTextBuff, (char*)"%.2X ", paucBuffer[i] );
			pobDebug->m_Print( enuDebugOutputType, acTextBuff );
		}
		pxCommsDX->xCommsHeaderDX.usChecksum = usChecksum;

		sprintf( (char*)acTextBuff, (char*)"\n%sTX: Size = %d  ::  CHK = %.4X ", pszCommsType, pxCommsDX->xCommsHeaderDX.usPacketLength, pxCommsDX->xCommsHeaderDX.usChecksum );
		pobDebug->m_PrintCR( enuDebugOutputType, acTextBuff );




		if ( ENU_KW_LORA == enuKeyWordType )
		{
#if 0	// !!!! ~UPDATING !!!!
			// TRANSMIT LoRa packet.

			LoRa.beginPacket();
			pobDebug->m_PrintCR( enuDebugOutputType, (char*)"---LORA Begin = OK" );

			LoRa.write( (uchar*)pxCommsDX, pxCommsDX->xCommsHeaderDX.usPacketLength );
			pobDebug->m_PrintCR( enuDebugOutputType, (char*)"---LORA Write = OK" );

			LoRa.endPacket();
			pobDebug->m_PrintCR( enuDebugOutputType, (char*)"---LORA Ended = OK" );


			pobBase->m_BlinkCommsLED( ENU_LED_BLINK_LORA_TX );
#endif
		}
		else
		{
			// OOOPS! Should never get here, as TX types are enumerated & only existing types should ever get through.
			sprintf( (char*)acTextBuff, (char*)"Unknown KeyWord TX TYPE - %.4X - ERR!\n", enuKeyWordType );
			pobDebug->m_PrintCR( enuDebugOutputType, acTextBuff );
		}
	}


	sprintf ( (char*)acTextBuff, (char*)"%sTX: >>>>>>  END - %s\n\n", pszCommsType, (char*)"LoRa Processed");
	pobDebug->m_Print( enuDebugOutputType, acTextBuff );

	return ( sCommandLength );
}


/***************************************************************************************************
 ***************************************************************************************************
 **	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!
 ***************************************************************************************************
 **************************************************************************************************/
