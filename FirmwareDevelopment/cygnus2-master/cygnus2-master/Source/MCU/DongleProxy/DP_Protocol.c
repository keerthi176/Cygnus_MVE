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
*  File       	: DP_Protocol.cpp
*
*  Description	: Takes care & processes communiucation protocol data for both RX & TX
*
*************************************************************************************/


/*************************************************************************************
 *************************************************************************************
 **  System Includes
 *************************************************************************************
 ************************************************************************************/

#include	"string.h"

#include "board.h"
#include "radio.h"
#include "sx1272/sx1272.h"
#include "sx1272-board.h"




/*************************************************************************************
 *************************************************************************************
 **  User Includes
 *************************************************************************************
 ************************************************************************************/
#include	"DP_Protocol.h"
#include "CO_Defines.h"
#include "CO_SerialDebug.h"



/*************************************************************************************
 *************************************************************************************
 **  Externs
 *************************************************************************************
 *************************************************************************************/



/*************************************************************************************
 *************************************************************************************
 **  Macros / Definitions
 *************************************************************************************
 *************************************************************************************/

	// Cater for max flash size of 320K.
#define	HDF_OTA_FLASH_SIZE_MAX			320*1024	

	// Default - will me made dynanic, as in packet request.
#define	HDF_OTA_BLOCK_SIZE				32

	// 1 bit to represent each block.
#define	HDF_OTA_BITMASK_SIZE	(HDF_OTA_FLASH_SIZE_MAX / HDF_OTA_BLOCK_SIZE / 8)

	// LoRa maximum buffer size for AT commands ( 128 larger that we need for now, but final size to be decided ).
/*************************************************************************************
 *************************************************************************************
 **  Private Variables
 *************************************************************************************
 *************************************************************************************/
	static	uint16_t			usPacketLength;
	static	int8_t			pszCommsType[64];
	static	_xCommsDX		*pxBufferDX;
	static	uint8_t			*paucBufferRX;
				
	static	bool				blValidLoRaPacket = false;

	ENUM_DEBUG_MASK			enuDebugOutputType;

	typedef	struct
	{
		bool			blPPU_Connected;

		uint16_t		usTransferSizeTotal;
		uint16_t		usPageSize;
		uint16_t		usCurrentPage;
		
		bool		ablBitMask[HDF_OTA_BITMASK_SIZE];
		uint8_t	aucDataRX[HDF_OTA_BLOCK_SIZE];
		uint16_t	ausPagesMissing[HDF_PAGES_MISSING16];
			
		//
	} _xOTA;

	typedef	struct
	{
		uint16_t		usID_PPU;
		uint16_t		usID_RBU;

		uint32_t		usVersion;			// 32-bit BCD  (xx.xx)
		uint16_t		usSerialNumber;	// 

		_xCommsDX	xLoRaRX;
		_xCommsDX	xLoRaTX;

		_xOTA			xOTA;
		//
	} _xRadioBoard;
	
	
	_xRadioBoard	xRadioBoard;


	
	

	
	
/*************************************************************************************
 *************************************************************************************
 **  Private Functions Prototypes
 *************************************************************************************
 *************************************************************************************/
	void 	DP_Protocol_SendLoRaPacket 	( uint8_t *pacBuffer, const uint8_t ucSize );
	void	DP_Protocol_RX_ProcessPacket	( void );
	void	DP_Protocol_LED_GPIO 			( uint8_t ucLED_GPIO );
	void	DP_Protocol_ListVisibleUnits	( void );
	//void	DP_Protocol_PingSend				( uint16_t usDeviceID );
	//void	DP_Protocol_PingReply			( void );
	bool	DP_Protocol_ProcessAT_Command	( void );

	
	
/*!
 **************************************************************************************************
 **************************************************************************************************
 ** @function	: m_KeywordDebugType
 **
 ** @brief		: Debug output, debug messages & point to approriate structure to receive data.
 **
 ** @param		: Which KeyWord interface. eg: ENU_KW_LORA, ENU_KW_RS232, etc...
 **
 ** @return		: VOID
 **************************************************************************************************
 **************************************************************************************************/
void	DP_Protocol_KeywordDebugType ( ENUM_KEYWORD_TYPE enuKeyWordType )
{
	if ( ENU_KW_LORA == enuKeyWordType )
	{
		pxBufferDX = &xRadioBoard.xLoRaTX;
		strcpy( (char*)pszCommsType, (char*)"LoRa-" );
		enuDebugOutputType = ENU_DEBUG_LORA;
	}
	else
	{
		strcpy( (char*)pszCommsType, (char*)"????-" );
	}
}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: KeyWordRX
 **
 ** @brief  	: Receive data packet.
 **
 ** @param		: Command type.
 **
 ** @return		: Packet Length.
 **************************************************************************************************
**************************************************************************************************/
uint16_t	DP_Protocol_KeywordRX ( ENUM_KEYWORD_TYPE enuKeyWordType )
{
	int8_t		acBuffer[128];
	bool			blValidHeader;
	int16_t		usOK;

	// For debug output-prefix
	DP_Protocol_KeywordDebugType( enuKeyWordType );
	
	paucBufferRX = (uint8_t*)pxBufferDX;
	blValidHeader = false;
	usOK = 0;


	// ?? Vaildate LoRa KeyWord header ??
	if ( enuKeyWordType == ENU_KW_LORA )
	{
		// YES! 
		//usPacketLength = (uint16_t)LoRa.parsePacket();
		//obLoRaDX.iRSSI = LoRa.packetRssi();
		DP_LoRa_ParsePacket();

		// Read packet
		//while ( DP_LoRa_Available() && usPacketLength > 0 )
		while ( usPacketLength > 0 )
		{
			if ( usPacketLength <= sizeof( _xCommsDX ) )
			{
				sprintf( (char*)acBuffer, (char*)"\n%sRX: >>>>>>  START - %s", pszCommsType, HDF_SYSTEM_PPU_NAME );
				DP_Debug_PrintCR( enuDebugOutputType, acBuffer );

				//LoRa.readBytes( paucBufferRX, usPacketLength );
				//m_BlinkCommsLED( ENU_LED_BLINK_LORA_RX );
				
				// !!!!!!  DEBUG CHECK!
			}
			else
			{
				sprintf( (char*)acBuffer, "\n\n\n%sRX: Length = %d OVERSIZED - ERR!\n\n", pszCommsType, usPacketLength );
				DP_Debug_PrintCR( enuDebugOutputType, acBuffer );
			}

			sprintf( (char*)acBuffer, "%sRX: LoRa Size = %d", pszCommsType, usPacketLength );
			DP_Debug_PrintCR( enuDebugOutputType, acBuffer );

			// Header data is good.
			blValidHeader = true;
		}
	} // ?? Vaildate LoRa KeyWord header ??
	else
	{
		DP_Debug_PrintCR( enuDebugOutputType, (int8_t*)"UNKNOWN RX - ERR!" );
	}



	// ?? Valid header KeyWord. ??
	if ( blValidHeader )
	{
		// ?? Valid keyword packet. ??
		if ( DP_Protocol_KeyWordValidate( enuKeyWordType ) == true )
		{
			// Decode packet we've just recieved - & act on it.
			usOK = DP_Protocol_KeyWordDecode( enuKeyWordType );

			sprintf( (char*)acBuffer, (char*)"%sRX: >>>>>>  END - %s\n\n", pszCommsType, HDF_SYSTEM_PPU_NAME );
			DP_Debug_PrintCR( enuDebugOutputType, acBuffer  );
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




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: m_ValidPacket
 **
 ** @brief		: Valid data packet recieved
 **
 ** @param		: ENU_KW_SOCKET or ENU_KW_LORA.
 **
 ** @return		: TRUE == Valid : FALSE == Invalid.
 **************************************************************************************************
 **************************************************************************************************/
bool	DP_Protocol_ValidPacket ( ENUM_KEYWORD_TYPE enuKeywordType )
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




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: m_KeyWordValidate
 **
 ** @brief		: Validate a recieved Keyword packet.
 **
 ** @param		: ENU_KW_LORA or ENU_KW_SOCKET
 **
 ** @Out			: TRUE == VALID  -  FALSE == INVALID.
 **************************************************************************************************
 *************************************************************************************************/
bool	DP_Protocol_KeyWordValidate	( ENUM_KEYWORD_TYPE enuKeyWordType )
{
int8_t		acBuffer[128];
bool			blValidKeyWord;
uint16_t		usChecksum, usCheckTemp;
int16_t		i;

	// For debug output (get string for type of data were processing).
	DP_Protocol_KeywordDebugType( enuKeyWordType );


	blValidKeyWord = false;
	usChecksum = 0;
	usCheckTemp = pxBufferDX->xCommsHeaderDX.usChecksum;
	pxBufferDX->xCommsHeaderDX.usChecksum = 0;

	// ENU_DEBUG_SOCKET
	DP_Debug_Print( enuDebugOutputType, pszCommsType );
	DP_Debug_Print( enuDebugOutputType, (int8_t*)"RX: " );

	for ( i = 0; i < usPacketLength; i++ )
	{
		usChecksum += (char)paucBufferRX[i];

		sprintf( (char*)acBuffer, (char*)"%.2X ", paucBufferRX[i] );
		DP_Debug_Print( enuDebugOutputType, acBuffer );
	}
	pxBufferDX->xCommsHeaderDX.usChecksum = usCheckTemp;




	// process & validate that packet is a KEYWORD packet.
	if ( pxBufferDX->xCommsHeaderDX.usPacketLength == usPacketLength )
	{
		// ?? Valid checksum ??
		if ( usChecksum == pxBufferDX->xCommsHeaderDX.usChecksum )
		{
			// Device ID & CMD.
			sprintf( (char*)acBuffer, (char*)"\n%sRX: CMD: [%.4X]  -  From Device = [%.4X]",
						pszCommsType,
						pxBufferDX->xCommsHeaderDX.usCommand,
						pxBufferDX->xCommsHeaderDX.usHostID
						);
			DP_Debug_Print( enuDebugOutputType, acBuffer );

			// Checksum Recieved & Checksum Calculated (OK).
			sprintf( (char*)acBuffer, (char*)"%sRX: CHK VAL = [%.4X]  -  CHK CALC = [%.4X]  -  CHK OK",
						pszCommsType,
						pxBufferDX->xCommsHeaderDX.usChecksum,
						usChecksum
					);
			DP_Debug_PrintCR( enuDebugOutputType, acBuffer  );
			blValidKeyWord = true;
		}
		else
		{
			// Checksum Recieved & Checksum Calculated (ERROR)
			sprintf( (char*)acBuffer, (char*)"%sRX: CHK VAL = [%.4X]  -  CHK CALC = [%.4X]  -  CHK = ERR!",
						pszCommsType,
						pxBufferDX->xCommsHeaderDX.usChecksum,
						usChecksum
					);
			DP_Debug_PrintCR( enuDebugOutputType, acBuffer  );
		}
	} // if ( pxBufferDX->xCommsHeaderDX.usPacketLength == usPacketLength )
	else
	{
		sprintf( (char*)acBuffer, (char*)"\n%sRX: ------ NON KEYWORD PACKET - Length! (%d)  -  ERR!", pszCommsType, usPacketLength );
		DP_Debug_PrintCR( enuDebugOutputType, acBuffer  );
	}

	return ( blValidKeyWord );
}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: m_KeyWordDecode
 **
 ** @brief		: Decode & process a recieved Keyword data packet.
 **
 ** @param		: Keyword type to decode - ENU_KW_SOCKET or ENU_KW_LORA.
 **
 ** @return		: Valid Keyword packet - TRUE/FALSE.
 **************************************************************************************************
 *************************************************************************************************/
uint16_t DP_Protocol_KeyWordDecode	( ENUM_KEYWORD_TYPE enuKeyWordType  )
{
uint16_t				usOK;
int8_t				acBuffer[128];
int8_t				pszError[32];
uint16_t				usRX_Command;
_xCommsDX			xKeyWord;

	// called to determine type of debug text prefix we're going to use.
	DP_Protocol_KeywordDebugType( enuKeyWordType );

	usRX_Command = pxBufferDX->xCommsHeaderDX.usCommand;

	usOK = usRX_Command;

	// ?? ERROR CHECK - Have we recieved a command the with error-code HDF_DX_BIT_REPLY_WITH_ERROR bit SET ??
	if ( HDF_DX_BIT_REPLY_WITH_ERROR == (usRX_Command & HDF_DX_MASK_CMD_STATUS_NIBBLE) )
	{
		switch ( pxBufferDX->uxCommandData.uxDataType.usErrorCode )
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
		DP_Debug_PrintCR( enuDebugOutputType, acBuffer );
	}
	
	
	// ?? REQUEST CHECK - Was bit #12 set to indicate that the data requested & needs a reply ??
	// ?? REQUEST CHECK - Was bit #12 set to indicate that the data requested & needs a reply ??
	else if ( HDF_DX_BIT_REQUEST_DATA ==(usRX_Command & HDF_DX_MASK_CMD_STATUS_NIBBLE) )
	{
		// Recieved a request for command data (eg: current device temperature).
		sprintf( (char*)acBuffer, (char*)"%s_RX: Device [%.4X] Requested Data For CMD [%.4X]  -  CMD = [%.4X]",
					pszCommsType,
					pxBufferDX->xCommsHeaderDX.usHostID,
					usRX_Command,
					usRX_Command & HDF_DX_MASK_COMMANDS
				);

		DP_Debug_PrintCR( enuDebugOutputType, acBuffer );

			/*
			 * DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND.
			 * DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND.
			 * DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND.
			 * DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND		DECODE COMMAND REQUEST & RESPOND.
			 */
		// Strip top nibble mask & check which command request we've recieved.
		switch ( HDF_DX_MASK_COMMANDS & usRX_Command )
		{
		case HDF_DX_TEST_VALUE:
			// !! DEBUG - Fixed value "2468", just to test any return value while debugging !!
			xKeyWord.xCommsHeaderDX.usCommand = (HDF_DX_TEST_VALUE | HDF_DX_BIT_REPLY_WITH_OK);
			xKeyWord.uxCommandData.uxDataType.usTestValue= 0x2468;	// !!!! TEST !!!!
			// send a response back to the sending device.
			DP_Protocol_KeyWordTX( enuKeyWordType, &xKeyWord, xRadioBoard.xLoRaTX.xCommsHeaderDX.usHostID );
			
			// !! DEBUG - Fixed value "2468", just to test any return value while debugging !!
			break;

		case HDF_DX_PPU_FIRMWARE_REQUEST_MISSED:
			xKeyWord.xCommsHeaderDX.usCommand = (HDF_DX_PPU_FIRMWARE_REQUEST_MISSED | HDF_DX_BIT_REPLY_WITH_OK);
			DP_Protocol_Reply_GetMissingUpdatePackets();
			DP_Protocol_KeyWordTX( enuKeyWordType, &xKeyWord, xRadioBoard.xLoRaTX.xCommsHeaderDX.usHostID );
			break;

		
		default:
			// Unknown request == reply with requested command having HDF_DX_BIT_REPLY_WITH_ERROR bit set
			sprintf( (char*)acBuffer, (char*)"%s_RX: From Device [%.4X]  -  Request UNKNOWN!!! [%.4X]", pszCommsType, pxBufferDX->xCommsHeaderDX.usHostID, usRX_Command );
			DP_Debug_PrintCR( enuDebugOutputType, acBuffer );
			// TX (Socket or LoRa)
			// DP_Protocol_KeyWordTX( enuKeyWordType, (usRX_Command & HDF_DX_MASK_COMMANDS) | HDF_DX_ERR_COMMAND_UNKNOWN | HDF_DX_BIT_REPLY_WITH_ERROR );

			xKeyWord.xCommsHeaderDX.usCommand = (usRX_Command & HDF_DX_MASK_COMMANDS) | HDF_DX_ERR_COMMAND_UNKNOWN | HDF_DX_BIT_REPLY_WITH_ERROR;
			xKeyWord.uxCommandData.uxDataType.usTestValue = 0x2468;	// !!!! TEST !!!!
			// send response back to the sending device
			DP_Protocol_KeyWordTX( enuKeyWordType, &xKeyWord, xRadioBoard.xLoRaTX.xCommsHeaderDX.usHostID );

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
		// Recieved OK mask (HDF_DX_BIT_REPLY_WITH_OK) sent in response to last command sent
		sprintf( (char*)acBuffer, (char*)"%s_RX: [%.4X] = {{ ACK OK }} From Device [%.4X]  -  [%.4X]",
					pszCommsType,
					usRX_Command,
					pxBufferDX->xCommsHeaderDX.usHostID,
					usRX_Command & HDF_DX_MASK_COMMANDS
				);
		DP_Debug_PrintCR( enuDebugOutputType, acBuffer );
	}
	// Process 'ONE SHOT' CMD - HDF_DX_MASK_CMD_STATUS_NIBBLE command status bit not set
	// Process 'ONE SHOT' CMD - HDF_DX_MASK_CMD_STATUS_NIBBLE command status bit not set
	// Process 'ONE SHOT' CMD - HDF_DX_MASK_CMD_STATUS_NIBBLE command status bit not set
	// Process 'ONE SHOT' CMD - HDF_DX_MASK_CMD_STATUS_NIBBLE command status bit not set
	else if ( (HDF_DX_MASK_CMD_STATUS_NIBBLE & usRX_Command) == 0 )
	{
		// Recieved 'ONE SHOT' CMD - sent data (eg: movement sensor from another device)
		sprintf( (char*)acBuffer, (char*)"%sRX: ONE SHOT - CMD = [%.4X] = ",
					pszCommsType,
					usRX_Command );
		DP_Debug_Print( enuDebugOutputType, acBuffer );


		switch ( usRX_Command  )
		{
		// !!!! DEBUG !!!!
		case	HDF_DX_BLIP_TEST:
			DP_Debug_PrintCR( enuDebugOutputType, (int8_t*)"BLIP_TEST" );
			break;

		
		// !!!! DEBUG !!!!
		case	HDF_DX_CYGNUS_PULSE:
			DP_Debug_PrintCR( enuDebugOutputType, (int8_t*)"CYGNUS_PULSE" );
			break;


		
		// !!!! OTA !!!!
		// !!!! OTA !!!!
		// !!!! OTA !!!!
		// !!!! OTA !!!!

		// Send firmware page data.
		case	HDF_DX_PPU_FIRMWARE_UPDATE_OTA:
#if 0
			// Total number of pages.
			xRadioBoard.xOTA.usTotalPages = pxBufferDX->uxCommandData.uxDataType.xDataDX.usPagesTotal;
			// Current page number.
			xRadioBoard.xOTA.usCurrentPage = pxBufferDX->uxCommandData.uxDataType.xDataDX.usPageNumber;
			// Number of bytes in current page.
			xRadioBoard.xOTA.usPageSize = pxBufferDX->uxCommandData.uxDataType.xDataDX.usPageSize;
			// Number of blocks 
			xRadioBoard.xOTA.usCurrentBlock = pxBufferDX->uxCommandData.uxDataType.xDataDX.ucBlockNumber;
#endif

			// Store data-block recieved in OTA buffer - into the structure arrayto be read by flash routine.
			for ( uint16_t i = 0; i < pxBufferDX->uxCommandData.uxDataType.xDataDX.usPageSize; i++ )
			{
				xRadioBoard.xOTA.aucDataRX[i] = pxBufferDX->uxCommandData.uxDataType.xDataDX.aucPayload[i];
			}
			
			// !!!! DEBUG INFO !!!!
			sprintf( (char*)acBuffer,
						(char*)"FIRMWARE - PAGE %3.0d - PSIZE %3.0d",
						pxBufferDX->uxCommandData.uxDataType.xDataDX.usPageNumber,
						pxBufferDX->uxCommandData.uxDataType.xDataDX.usPageSize
						);
			// !!!! DEBUG INFO !!!!
			break;

					
		case HDF_DX_PPU_FIRMWARE_REQUEST_MISSED:
			break;

		// !!!! OTA !!!!
		// !!!! OTA !!!!
		// !!!! OTA !!!!
		// !!!! OTA !!!!
		
		case HDF_DX_PPU_PING_SEND:
			//DP_Protocol_PingSend( pxBufferDX->uxCommandData.uxDataType.xPing.usDeviceID );
			break;
				
		case HDF_DX_PPU_PING_REPLY:
			//DP_Protocol_PingSend( pxBufferDX->uxCommandData.uxDataType.xDataDX.usRBU_ID );
			break;
				
		case HDF_DX_PPU_GET_VERSION:
			//DP_Protocol_GetVersion();
			break;
		
		case HDF_DX_PPU_LIST_VISIBLE_UNITS:
			DP_Protocol_ListVisibleUnits();
			break;
		
		case HDF_DX_RBU_SET_LED_GPIO:
			//
			break;
		
		case HDF_DX_PPU_AT_COMMAND:
			DP_Protocol_ProcessAT_Command();
			break;
		
		//case HDF_DX_PPU
		
		default:
			// Unknown 'ONE SHOT' CMD - received with no status bit set
			sprintf( (char*)acBuffer, (char*)"From Device [%.4X]  -  Request UNKNOWN!!! [%.4X]", pxBufferDX->xCommsHeaderDX.usHostID, usRX_Command );
			DP_Debug_PrintCR( enuDebugOutputType, acBuffer );

			// ?? Socket or LoRa ??
			xKeyWord.xCommsHeaderDX.usCommand = (usRX_Command | HDF_DX_BIT_REPLY_WITH_ERROR);
			// send response back to the sending device
			DP_Protocol_KeyWordTX( enuKeyWordType, &xKeyWord, xRadioBoard.xLoRaTX.xCommsHeaderDX.usHostID );
		
		
			usOK = 0;
			break;
		}
	}
	else
	{
		// Should never get here, as all commands are either defined or captured if they're not. ('else' for debug purposes).
		sprintf( (char*)acBuffer, (char*)"%sRX: DEV - UNCAPTURED! ~ [%.4X]  -  From Device [%.4X]", pszCommsType, pxBufferDX->xCommsHeaderDX.usCommand, pxBufferDX->xCommsHeaderDX.usHostID );
		DP_Debug_PrintCR( enuDebugOutputType, acBuffer );
		usOK = 0;
	}

	return ( usOK );
}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_SendLoRaPacket
 **
 ** @brief		: 
 **
 ** @param		: Buffer pointer.
 ** @param		: Buffer size in bytes.
 **
 ** @return		: VOID
 **************************************************************************************************
 *************************************************************************************************/
void 	DP_Protocol_SendLoRaPacket ( uint8_t *pacBuffer, const uint8_t ucSize )
{
#if 0		// NO CRC during development - CRC for RELEASE!!!!
   uint16_t calculated_crc;
	
   /* Calculate the CRC over the complete packet */
   DM_Crc16bCalculate8bDataWidth( (uint8_t*)pacBuffer, ucSize - 2u, &calculated_crc, 1u );
   
   /* Insert the CRC value in the Tx buffer */
   pacBuffer[ucSize-2] = ( (calculated_crc >> 8) & 0xFF );
   pacBuffer[ucSize-1] = ( calculated_crc & 0xFF );
#endif

   /* Set the antenna switch to transmit */
   GpioWrite( &AntTxNrx, 1 );
   GpioWrite( &AntRxNtx, 0 );
   
   /* Send the actual packet */
   Radio.Send( (uint8_t*)pacBuffer, ucSize );
   SX1272SetTx( 0 );	
}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_LED_GPIO
 **
 ** @brief		: Set LED RGB GPIO Pins.
 **
 ** @param		: uint8_t
 **
 ** @return		: VOID
 **************************************************************************************************
 *************************************************************************************************/
void DP_Protocol_LED_GPIO 	( uint8_t ucLED_GPIO )
{
	xRadioBoard.xLoRaTX.xCommsHeaderDX.usCommand = HDF_DX_INT;
	xRadioBoard.xLoRaTX.uxCommandData.uxDataType.ucChar = ucLED_GPIO;

	// send response back to the sendingdevice
	DP_Protocol_KeyWordTX( ENU_KW_LORA, &xRadioBoard.xLoRaTX, xRadioBoard.xLoRaTX.xCommsHeaderDX.usHostID );
}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_ListVisibleUnits
 **
 ** @brief		:
 **
 ** @param		:
 **
 ** @return		: NONE
 **************************************************************************************************
 *************************************************************************************************/
void DP_Protocol_ListVisibleUnits ( void )
{
}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_BlipTest
 **
 ** @brief		: Test Transmisstion - Cycling though the NATO phonetic alaphabet.
 ** @brief		: DEBUG! Used while developing the comms.
 **
 ** @param		: A 32-bit test value is sent with a string.
 **
 ** @return		: VOID
 **************************************************************************************************
 *************************************************************************************************/
void	DP_Protocol_BlipTest 	( uint32_t uiBlip )
{
	// !!!! DEBUG TX DATA !!!!
	const	int8_t acMessage[][32] =
	{
		{ "ALPHA"		},
		{ "BRAVO"		},
		{ "CHARLIE"		},
		{ "DELTA"		},
		{ "ECHO"			},
		{ "FOXTROT"		},
		{ "GOLF"			},
		{ "HOTEL"		},
		{ "INDIGO"		},
		{ "JULIETTE"	},
		{ "KILO"			},
		{ "LIMA"			},
		{ "MIKE"			},
		{ "NOVEMBER"	},
		{ "OSCAR"		},
		{ "PAPA"			},
		{ "QUEBEC"		},
		{ "ROMEO"		},
		{ "SIERRA"		},
		{ "TANGO"		},
		{ "UNIFORM"		},
		{ "VICTOR"		},
		{ "WHISKEY"		},
		{ "X-RAY"		},
		{ "YANKEE"		},
		{ "ZULU"			}
	};
static	uint8_t	ucIndex = 0;

	xRadioBoard.xLoRaTX.xCommsHeaderDX.usCommand = HDF_DX_BLIP_TEST;
	xRadioBoard.xLoRaTX.uxCommandData.uxDataType.xBlipTest.iCounter = uiBlip;
	//xRadioBoard.xLoRaTX.xCommsHeaderDX.usHostID = 0xCEFA;	// !!!! DEBUG TEST VALUE !!!!
	xRadioBoard.xLoRaTX.xCommsHeaderDX.usHostID = HDF_SYSTEM_PPU_ID;
	strcpy( (char*)xRadioBoard.xLoRaTX.uxCommandData.uxDataType.xBlipTest.pszString, (char*)acMessage[ucIndex] );
	
	ucIndex++;
	if ( 26 <= ucIndex )
	{
		 ucIndex = 0;
	}

	// send response back to the sending device,
	//DP_Protocol_KeyWordTX( ENU_KW_LORA, &xRadioBoard.xLoRaTX, 0xEBAE );	// !!!! DEBUG !!!! Send to EBAE (0xffff == broadcast(send to all))

	// !!!! TEST SX1272WriteFifo !!!!
	SX1272WriteFifo( acMessage, strlen((char*)acMessage ) );

/*
	uint8_t	aucBuffer[] = { "ABCDEFGHIJKLMNOP" };
	// !!!! TEST SX1272WriteFifo !!!!
	SX1272WriteFifo( aucBuffer, 16 );
*/
}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_SetRBU_ID
 **
 ** @brief		: set the ID value of an RBU
 **
 ** @param		: uint16_t ID value
 **
 ** @return		: VOID
 **************************************************************************************************
 *************************************************************************************************/
bool	DP_Protocol_SetRBU_ID 	( uint16_t usDeviceID )
{
bool	blOK = false;
	
	// ?? New RBU value in range ??
	if ( HDF_SYSTEM_RBU_ID_MAX <= usDeviceID && HDF_SYSTEM_RBU_ID_MAX >= usDeviceID )
	{
		// YES! Set new value.
		xRadioBoard.usID_RBU = usDeviceID;
		
		blOK = true;
	}
	
	// 
	return ( blOK );
}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_SetPPU_ID
 **
 ** @brief		: Set the ID of the PPU.
 **
 ** @param		: uint16_t ID value
 **
 ** @return		: true == set, false == fail.
 **************************************************************************************************
 *************************************************************************************************/
bool	DP_Protocol_SetPPU_ID 	( uint16_t usDeviceID )	
{
bool	blOK = false;
	
	// ?? New RBU value in range ??
	if ( HDF_SYSTEM_PPU_ID_MAX <= usDeviceID && HDF_SYSTEM_PPU_ID_MAX >= usDeviceID )
	{
		// YES! Set new value.
		xRadioBoard.usID_PPU = usDeviceID;
		blOK = true;
	}
	
	// 
	return ( blOK );
}


/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_SetDeviceSerialNumber
 **
 ** @brief		: Set the ID of the PPU.
 **
 ** @param		: uint16_t ID value
 ** @param		: uint32_t Serial
 **
 ** @return		: true == set, false == fail.
 **************************************************************************************************
 *************************************************************************************************/
void	DP_Protocol_SetDeviceSerialNumber	( uint16_t usDeviceID, uint32_t uiSerial )
{
	xRadioBoard.usSerialNumber = uiSerial;
}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_SendOTA 
 **
 ** @brief		: Send data to a device(s) in small transmissions of pages & blocks.
 **
 ** @param		: Device ID we're sending to (0xffff == broadcast to all).
 ** @param		: Size of full binary data we're sending.
 ** @param		: Total number of pages to send.
 ** @param		: Current page being sent.
 ** @param		: Size of current page being sent.
 ** @param		: Total number of blocks to send
 ** @param		: Block per page.
 ** @param		: Current Block Number.
 ** @param		: Pointer to payload.
 **
 ** @return		: VOID
 **************************************************************************************************
 *************************************************************************************************/
void	DP_Protocol_SendOTA 	( uint16_t usDeviceID, uint32_t uiDataSizeTotal,
										uint16_t usPagesTotal, uint16_t usPageNumber, uint16_t ucPageSize,
										uint8_t ucBlockNumber, uint8_t ucBlockSize,
										uint8_t *paucPageDataPayload )
{
char	acBuffer[200];

		xRadioBoard.xLoRaTX.uxCommandData.uxDataType.xDataDX.usRBU_ID = usDeviceID;
		xRadioBoard.xLoRaTX.uxCommandData.uxDataType.xDataDX.uiDataSizeTotal = uiDataSizeTotal;

		xRadioBoard.xLoRaTX.uxCommandData.uxDataType.xDataDX.usPageNumber = usPageNumber;
		xRadioBoard.xLoRaTX.uxCommandData.uxDataType.xDataDX.usPageSize = ucPageSize;

	
	// !!!! DEBUG !!!! outputting [Page & Block Info] to terminal for debug.
	// !!!! DEBUG !!!! outputting [Page & Block Info] to terminal for debug.
	sprintf( acBuffer, 	"Dev ID = %3.0d - Page No = %3.0d - Page Sz %3.0d\r\n",
								xRadioBoard.xLoRaTX.uxCommandData.uxDataType.xDataDX.usDongleID,
								xRadioBoard.xLoRaTX.uxCommandData.uxDataType.xDataDX.usPageNumber,
								xRadioBoard.xLoRaTX.uxCommandData.uxDataType.xDataDX.usPageSize
				);
	
	CO_PRINT_A_0( DBG_INFO, acBuffer );
	// !!!! DEBUG !!!! outputting [Page & Block Info] to terminal for debug.
	// !!!! DEBUG !!!! outputting [Page & Block Info] to terminal for debug.

}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_SendBuffer
 **
 ** @brief		: Semd a data array. (to implement for future use).
 **
 ** @param		: PPU/RBU Device ID.
 ** @param		: Buffer pointer.
 ** @param		: Buffer size.
 **
 ** @return		: VOID
 **************************************************************************************************
 *************************************************************************************************/
void	DP_Protocol_SendBuffer 	( uint16_t usDeviceID, uint16_t *paucBuffer, uint8_t iBufferSize )
{
	// Merge with PC's Send
}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_PingDevice
 **
 ** @brief		:
 **
 ** @param		:
 **
 ** @return		:
 **************************************************************************************************
 *************************************************************************************************/
int32_t	DP_Protocol_PingDevice		( uint16_t usDeviceID )
{
	return ( usDeviceID );
}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_RX_ProcessPacket
 **
 ** @brief		: Process & act on data received in the RX buffer.
 **
 ** @param		: VOID
 **
 ** @return		: VOID
 **************************************************************************************************
 *************************************************************************************************/
void	DP_Protocol_RX_ProcessPacket	( void )
{
int16_t	usKeyWord;
int16_t	sCount;
int8_t	acTextBuffer[64];	

	// Check that the data we've received is valid.
	usKeyWord = DP_Protocol_KeywordRX( ENU_KW_LORA );
	
	// ?? Valid Data Packet == non-zero value ??
	if ( usKeyWord )
	{
		// YES!!
		DP_Debug_PrintCR( ENU_DEBUG_LORA, (int8_t*)"\n****** LORA KEYWORD - PROCESSING START ******\n" );

		switch ( usKeyWord )
		{
		// Firmware update initiated.
		case	HDF_DX_PPU_FIRMWARE_UPDATE_CONNECT:
			DP_Protocol_PPU_Connect();
			break;

		case HDF_DX_PPU_FIRMWARE_UPDATE_OTA:
/*
			// !!!! DEBUG INFO !!!!
			sprintf( (char*)acTextBuffer, (char*)"FIRMWARE - UPDATE!" );
			DP_Debug_PrintCR( enuDebugOutputType, (char*)acTextBuffer );
			// !!!! DEBUG INFO !!!!
*/
			xRadioBoard.xOTA.usTransferSizeTotal = pxBufferDX->uxCommandData.uxDataType.xDataDX.uiDataSizeTotal;
			xRadioBoard.xOTA.usCurrentPage = pxBufferDX->uxCommandData.uxDataType.xDataDX.usPageNumber;
			xRadioBoard.xOTA.ablBitMask[sCount] = false;
		
			//xRadioBoard.xLoRaRX.uxCommandData.uxDataType.xDataDX.

			break;
		
		case HDF_DX_PPU_PING_REPLY:
			DP_Protocol_Reply_Ping();
			break;
		
		case HDF_DX_PPU_GET_VERSION:
			DP_Protocol_Reply_Version();
			break;

		case HDF_DX_PPU_LIST_VISIBLE_UNITS:
			DP_Protocol_Reply_VisibleUnits();
			break;
		
		case HDF_DX_PPU_FIRMWARE_REQUEST_MISSED:
			DP_Protocol_Reply_GetMissingUpdatePackets();
			break;
		
		default:
			break;
		
		} // switch ( usKeyWord )

		DP_Debug_PrintCR( ENU_DEBUG_LORA, (int8_t*)"\n****** LORA KEYWORD - PROCESSING END ******\n" );

	} // if ( usKeyWord )
}








/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_Reply_Ping
 **
 ** @brief		:
 **
 ** @param		:
 **
 ** @return		:
 **************************************************************************************************
 *************************************************************************************************/
void	DP_Protocol_Reply_Ping ( void )
{
}





/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_Reply_VisibleUnits
 **
 ** @brief		:
 **
 ** @param		:
 **
 ** @return		:
 **************************************************************************************************
 *************************************************************************************************/
void	DP_Protocol_Reply_VisibleUnits ( void )
{
}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_Reply_Version
 **
 ** @brief		:
 **
 ** @param		:
 **
 ** @return		:
 **************************************************************************************************
 *************************************************************************************************/
void	DP_Protocol_Reply_Version ( void )
{
	
}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_Reply_GetMissingUpdatePackets
 **
 ** @brief		: Return a list of missing OTA firmware pages (if any).
 **
 ** @param		: NONE
 **
 ** @return		: @int16_t Number of missing pages
 **************************************************************************************************
 *************************************************************************************************/
int16_t	DP_Protocol_Reply_GetMissingUpdatePackets ( void )
{
int16_t	iMissing;
int16_t	iPacketNumber;
	
	// Mark 16 packet statuses at-a-time, as 0xffff - not missing.
	for ( iMissing = 0; iMissing < HDF_PAGES_MISSING16; iMissing++ )
	{
		xRadioBoard.xOTA.ausPagesMissing[iMissing] = 0xffff;
	}

	// Scan through the bit-mask of the pages received.
	for ( iPacketNumber = 0; iPacketNumber < HDF_OTA_BITMASK_SIZE || HDF_PAGES_MISSING16 == iMissing; iPacketNumber++ )
	{
		// ?? Has the current packet been received ??
		if ( true == xRadioBoard.xOTA.ablBitMask[iPacketNumber] )
		{
			// NO! Log the number of the "missing page" in its array - Max HDF_PAGES_MISSING16 at a time
			xRadioBoard.xOTA.ausPagesMissing[iMissing] = iPacketNumber;
			xRadioBoard.xLoRaTX.uxCommandData.uxDataType.xPacketsMissing.ausPages[iMissing] = iPacketNumber;
			iMissing++;
		}
	}
	
	xRadioBoard.xLoRaTX.uxCommandData.uxDataType.xPacketsMissing.iMissing = iMissing;

	return ( iMissing );
}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_ProcessAT_Command
 **
 ** @brief		: Call functions associated with received AT command & its paramters
 ** @brief		: AT command parameters are within the received data packet & not passed to this function
 **
 ** @param		: NONE
 **
 ** @return		: NONE
 **************************************************************************************************
 *************************************************************************************************/
bool	DP_Protocol_ProcessAT_Command	( void )
{
bool		blStatus;
int8_t	CommandAT[HDF_DX_AT_BUFFER_MAX];
	
	blStatus = true;
	
	if ( HDF_DX_AT_BUFFER_MAX > xRadioBoard.xLoRaTX.xCommsHeaderDX.usPacketLength - sizeof(xRadioBoard.xLoRaTX.xCommsHeaderDX) )
	{
		// contains AT Command
		//xRadioBoard.xLoRaTX.uxCommandData.uxDataType.xAT_Command.acBuffer
		blStatus = true;
	}
	else
	{
		blStatus = true;
	}
	
}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_PPU_Connect
 **
 ** @brief		:
 **
 ** @param		:
 **
 ** @return		:
 **************************************************************************************************
 *************************************************************************************************/
void	DP_Protocol_PPU_Connect ( void )
{
	int8_t	acTextBuffer[64];
	int16_t	sCount;
	
	
	// !!!! DEBUG INFO !!!!
	sprintf( (char*)acTextBuffer, (char*)"UPDATE MODE  - CONNECT!" );
	DP_Debug_PrintCR( enuDebugOutputType, (int8_t*)acTextBuffer );
	// !!!! DEBUG INFO !!!!

	// Clear all bits in the flash-map which indicate valid pages that have been received.
	for ( sCount = 0; sCount < HDF_OTA_BITMASK_SIZE; sCount++ )
	{
		xRadioBoard.xOTA.ablBitMask[sCount] = false;
	}
	xRadioBoard.xOTA.usTransferSizeTotal = pxBufferDX->uxCommandData.uxDataType.xDataDX.uiDataSizeTotal;
	xRadioBoard.xOTA.usPageSize = pxBufferDX->uxCommandData.uxDataType.xDataDX.usPageSize;
	
	xRadioBoard.xOTA.blPPU_Connected = true;
}


/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_PPU_Disconnect
 **
 ** @brief		:
 **
 ** @param		:
 **
 ** @return		:
 **************************************************************************************************
 *************************************************************************************************/
void	DP_Protocol_PPU_Disconnect ( void )
{
	xRadioBoard.xOTA.blPPU_Connected = false;
}





/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_PPU_Connected
 **
 ** @brief		: Status of the PPU to PBU connection (no activity timeout to be added).
 **
 ** @param		: NONE
 **
 ** @return		: true == connected : false == not connected
 **************************************************************************************************
 *************************************************************************************************/
bool	DP_Protocol_PPU_Connected ( void )
{
	return( xRadioBoard.xOTA.blPPU_Connected );
}




#if HDF_VARADIC_TX	// 
/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @fucntion	: DP_Protocol_KeyWordTX_Variadic
 **
 ** @brief		: Process & transmit a KeyWord command.
 **
 ** @param		: ENUM_KEYWORD_TYPE Transmition type - ENU_KW_SOCKET, ENU_KW_LORA, etc...
 ** @param		: int16_t* Command.
 ** @param		: Data/Parameters.
 **
 ** @return		: uint16_t Command Length.
 **************************************************************************************************
 *************************************************************************************************/
uint16_t	DP_Protocol_KeyWordTX_Variadic ( ENUM_KEYWORD_TYPE enuKeyWordType, uint16_t usCommand, ... )
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
	DP_Protocol_KeywordDebugType( enuKeyWordType );


	sprintf( (char*)acTextBuff, (char*)"\n\n%sTX: >>>>>>  START - %s", pszCommsType, HDF_SYSTEM_PPU_NAME );
	DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );


	sCommandLength = 0;
	va_start( vaArgsList, usCommand );

	sprintf( (char*)acTextBuff, (char*)"%sTX: COMMAND ID = $%.4X ", pszCommsType, usCommand );
	DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );


	// ?? Are we sending a reply with the ERROR bit set (HDF_DX_BIT_REPLY_WITH_ERROR) ??
	if ( (HDF_DX_MASK_CMD_STATUS_NIBBLE & usCommand) == HDF_DX_BIT_REPLY_WITH_ERROR )
	{
		sprintf( (char*)acTextBuff, (char*)"%sTX: REPLY - ERROR CODE = %.4X ", pszCommsType, usCommand );
		pxBufferDX->uxCommandData.uxDataType.usErrorCode = (int16_t)va_arg( vaArgsList, int32_t );
		DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
		sCommandLength += sizeof( int16_t );
	}
	// ?? Are we sending a reply with the OK bit set (HDF_DX_BIT_REPLY_WITH_OK) ??
	else if ( (HDF_DX_MASK_CMD_STATUS_NIBBLE & usCommand) == HDF_DX_BIT_REPLY_WITH_OK )
	{
		sprintf( (char*)acTextBuff, (char*)"%sTX: REPLY - {{ ACK OK }} = %.4X ", pszCommsType, usCommand );
		DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
	}
	// We're requesting data or sending a one shot command.
	else
	{
		sprintf( (char*)acTextBuff, (char*)"%sTX: ", pszCommsType );
		DP_Debug_Print( enuDebugOutputType, acTextBuff );


		// ?? If bit #12 set we are requesting data from another device, else sending data.
		sprintf( (char*)acTextBuff, (char*)"%s", usCommand & HDF_DX_BIT_REQUEST_DATA ? (char*)"REQUEST " : "ONE SHOT SEND " );
		DP_Debug_Print( enuDebugOutputType, acTextBuff );

		// Mask away status/info nibble to decode command.
		switch ( HDF_DX_MASK_COMMANDS & usCommand )
		{
		// Initiate firmware update.
		case	HDF_DX_PPU_FIRMWARE_UPDATE_BEGIN:
			sprintf( (char*)acTextBuff, (char*)"FIRMWARE - UPDATE!" );
			DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
			break;

		// Send firmware page data.
		case	HDF_DX_PPU_FIRMWARE_UPDATE_PAGE
			// Get current page number.
			pxBufferDX->uxCommandData.uxDataType.xPageOTA.uiPageNumber = (uint16_t)va_arg( vaArgsList, uint32_t );
			sCommandLength += sizeof( int8_t );

			// Get total number of pages.
			pxBufferDX->uxCommandData.uxDataType.xPageOTA.uiPageTotal = (uint16_t)va_arg( vaArgsList, uint32_t );
			sCommandLength += sizeof( int8_t );

			// Get number of bytes in current page.
			pxBufferDX->uxCommandData.uxDataType.xPageOTA.ucPageSize = (uint8_t)va_arg( vaArgsList, uint32_t );
			sCommandLength += sizeof( int8_t );

			//
			sprintf( (char*)acTextBuff,
						(char*)"FIRMWARE - SEND PAGE %d of %d",
						pxBufferDX->uxCommandData.uxDataType.xPageOTA.uiPageNumber,
						pxBufferDX->uxCommandData.uxDataType.xPageOTA.uiPageTotal
						);

		
			// Get address of sent data buffer.
			paucBuffer = (uint8_t*)va_arg( vaArgsList, uint32_t );

			// Store bytes from data buffer into the transmission structure.
			for ( uint8_t i = 0; i < pxBufferDX->uxCommandData.uxDataType.xPageOTA.ucPageSize; i++ )
			{
				pxBufferDX->uxCommandData.uxDataType.xPageOTA.aucPageData[i] = paucBuffer[i];
			}
			sCommandLength += pxBufferDX->uxCommandData.uxDataType.xPageOTA.ucPageSize;
			break;

		case HDF_DX_INT:
			DP_Debug_PrintCR( enuDebugOutputType, (int8_t*)"[INT]" );
			pxBufferDX->uxCommandData.uxDataType.usShort = (int16_t)va_arg( vaArgsList, uint32_t );
			sCommandLength += sizeof( int16_t );
			break;


		case HDF_DX_FLOAT:
			DP_Debug_PrintCR( enuDebugOutputType, (int8_t*)"[FLOAT]" );
			pxBufferDX->uxCommandData.uxDataType.fFloat = (float)va_arg( vaArgsList, double );
			sCommandLength += sizeof( float );
			break;


		case HDF_DX_STRING:
				paucBuffer = (uint8_t*)va_arg( vaArgsList, uint32_t );
				pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount = strlen( (char*)paucBuffer );

				sprintf( (char*)acTextBuff, (char*)"STRING LEN = %d - [ %s ]", pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount, paucBuffer );
				DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );

				// String size check.
				if ( HDF_DX_MAX_COMMAND_DATA > pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount )
				{
					for ( sCount = 0; sCount < pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount; sCount++ )
					{
						pxBufferDX->uxCommandData.uxDataType.xByteStream.aucBytes[sCount] = paucBuffer[sCount];
					}

					sCommandLength += (pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount + sizeof(pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount));
				}
				else
				{
					// !! ERRRM!
					pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount = 0;
					DP_Debug_PrintCR( enuDebugOutputType, (int8_t*)"STRING - ERR!" );
				}
			break;


		case HDF_DX_STRING_RAW:
				paucBuffer = (uint8_t*)va_arg( vaArgsList, uint32_t );
				pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount = strlen( (char*)paucBuffer );
				sCommandLength += sizeof(pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount);

				sprintf( (char*)acTextBuff, (char*)"STRING RAW ( %d )", pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount );
				DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );

				// String size check.
				if ( HDF_DX_MAX_COMMAND_DATA > pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount )
				{
					for ( sCount = 0; sCount < pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount; sCount++ )
					{
						pxBufferDX->uxCommandData.uxDataType.xByteStream.aucBytes[sCount] = paucBuffer[sCount];
					}

					sCommandLength += pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount;
				}
				else
				{
					// !! ERRRM!
					pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount = 0;
					DP_Debug_PrintCR( enuDebugOutputType, (int8_t*)"STRING RAW - ERR!" );
				}
			break;


		case HDF_DX_BYTESTREAM:
				// Get byte count.
				pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount = (int8_t)va_arg( vaArgsList, uint32_t );
				sCommandLength += sizeof(pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount);

				sprintf( (char*)acTextBuff, (char*)"[BYTESTREAM] ( %d )", pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount );
				DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );

				// Bytestream size check.
				if ( HDF_DX_MAX_COMMAND_DATA > pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount )
				{
					for ( sCount = 0; sCount < pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount; sCount++ )
					{
						pxBufferDX->uxCommandData.uxDataType.xByteStream.aucBytes[sCount] = (int8_t)va_arg( vaArgsList, uint32_t );
					}
					sCommandLength += pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount + 1;
				}
				else
				{
					// !! ERRRM!
					pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount = 0;
					DP_Debug_PrintCR( enuDebugOutputType, (int8_t*)"BYTESTREAM!" );
				}
			break;

				
				// CYGNUS DATA-SIZE TRANSFER TEST!!!!
				// CYGNUS DATA-SIZE TRANSFER TEST!!!!
		case HDF_DX_CYGNUS_PULSE:
			sprintf( (char*)acTextBuff, (char*)"[CYGNUS PULSE]"  );
			DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );

			// Running Counter.
			pxBufferDX->uxCommandData.uxDataType.xCygnusPulse.iCounter = va_arg( vaArgsList, uint32_t );
			sCommandLength += sizeof( uint16_t );
		
			// Size of data-packet to send (in bytes).
			pxBufferDX->uxCommandData.uxDataType.xCygnusPulse.iDataSize = va_arg( vaArgsList, uint32_t );
			sCommandLength += sizeof( uint16_t );
			
			// Data array.
			paucBuffer = (uint8_t*)va_arg( vaArgsList, uint32_t );
			if ( HDF_DX_MAX_CYGNUS_PACKET > pxBufferDX->uxCommandData.uxDataType.xCygnusPulse.iDataSize )
			{
				for ( sCount = 0; sCount < pxBufferDX->uxCommandData.uxDataType.xCygnusPulse.iDataSize; sCount++ )
				{
					pxBufferDX->uxCommandData.uxDataType.xCygnusPulse.acData[sCount] = paucBuffer[sCount];
				}
				
				sCommandLength += pxBufferDX->uxCommandData.uxDataType.xCygnusPulse.iDataSize;
			}
			else
			{
				// !! ERRRM!
				pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount = 0;
				DP_Debug_PrintCR( enuDebugOutputType, (int8_t*)"ERR: CYGNUS PULSE SIZE" );
			}
		
			break;
				// CYGNUS DATA-SIZE TRANSFER TEST!!!!
				// CYGNUS DATA-SIZE TRANSFER TEST!!!!

		
		case HDF_DX_BLIP_TEST:
			sprintf( (char*)acTextBuff, (char*)"[BLIP_TEST]"  );
			DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
			// Interger value.
			pxBufferDX->uxCommandData.uxDataType.xBlipTest.iCounter = va_arg( vaArgsList, uint32_t );
			sCommandLength += sizeof( uint16_t );

			// Pointer to passed string.
			paucBuffer = (uint8_t*)va_arg( vaArgsList, uint32_t );

			if ( HDF_DX_MAX_BLIP_TEXT > strlen( (char*)paucBuffer ) )
			{
				strcpy( (char*)pxBufferDX->uxCommandData.uxDataType.xBlipTest.pszString, (char*)paucBuffer );
				sCommandLength += strlen( (char*)pxBufferDX->uxCommandData.uxDataType.xBlipTest.pszString );

				sCommandLength++;	// For NULL Terminator.

				sprintf( (char*)acTextBuff, "%sTX: BLIP = \'%s\' - LENGTH: %d", pszCommsType, pxBufferDX->uxCommandData.uxDataType.xBlipTest.pszString, strlen( (char*)pxBufferDX->uxCommandData.uxDataType.xBlipTest.pszString) );
				DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
			}
			else
			{
				// !! ERRRM!
				sCommandLength = 0;

				sprintf( (char*)acTextBuff, (char*)"%sTX: BLIP SIZE - ERR!", pszCommsType );
				DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
			}
			break;

			
		case HDF_DX_BIT_REPLY_WITH_ERROR:
			pxBufferDX->uxCommandData.uxDataType.usErrorCode = va_arg( vaArgsList, uint32_t );
			sprintf( (char*)acTextBuff, (char*)"ERROR CODE = %.4X", pxBufferDX->uxCommandData.uxDataType.usErrorCode  );
			DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
			sCommandLength += sizeof( int16_t );
			break;


		default:
			sprintf( (char*)acTextBuff, (char*)">>> Unknown KeyWord CMD!!!   %.4X - ERR! <<<\n", usCommand  );
			DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
			sCommandLength = -1;
			break;
		} // switch ( usCommand & HDF_DX_MASK_COMMANDS )
	}
	va_end( vaArgsList );


	// Positive value == OK to process
	if ( 0 <= sCommandLength )
	{
		// Build protocol header.
		pxBufferDX->xCommsHeaderDX.usHostID = HDF_SYSTEM_PPU_ID;
		pxBufferDX->xCommsHeaderDX.usClientID = 0xffff;		//	ALL UNITS (For Now) !!!!
		pxBufferDX->xCommsHeaderDX.usChecksum = 0;
		pxBufferDX->xCommsHeaderDX.usPacketLength = sizeof( _xCommsHeaderDX ) + sCommandLength;
		pxBufferDX->xCommsHeaderDX.usCommand = usCommand;

		uint16_t	usChecksum;
		usChecksum = 0;
		paucBuffer = (uint8_t*)pxBufferDX;

		// Calculate checksum.
		sprintf ( (char*)acTextBuff, (char*)"%sTX: ", pszCommsType );
		DP_Debug_Print( enuDebugOutputType, acTextBuff );

		for ( uint16_t i = 0; i < pxBufferDX->xCommsHeaderDX.usPacketLength; i++ )
		{
			usChecksum += (uint8_t)paucBuffer[i];
			sprintf ( (char*)acTextBuff, (char*)"%.2X ", paucBuffer[i] );
			DP_Debug_Print( enuDebugOutputType, acTextBuff );
		}
		pxBufferDX->xCommsHeaderDX.usChecksum = usChecksum;

		// Packet Size
		sprintf( (char*)acTextBuff, (char*)"\n%sTX: PACKET SIZE = #%d", pszCommsType, pxBufferDX->xCommsHeaderDX.usPacketLength );
		DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
		// Packet Checksum
		sprintf( (char*)acTextBuff, (char*)"\n%sCHECKSUM = $%0.4X ", pszCommsType, pxBufferDX->xCommsHeaderDX.usChecksum );
		DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );




		if ( ENU_KW_LORA == enuKeyWordType )
		{
			sprintf( (char*)acTextBuff, "LoRa Begin" );
			DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );

#if 0
			/* Set the antenna switch to transmit */
			GpioWrite( &AntTxNrx, 1 );
			GpioWrite( &AntRxNtx, 0 );
			
			/* Send the actual packet */
			Radio.Send( (uint8_t*)pacBuffer, ucSize );
			SX1272SetTx( 0 );	

			m_SendLoRaPacket_TEST( (uint8_t*)pxBufferDX, pxBufferDX->xCommsHeaderDX.usPacketLength );
#endif
			sprintf( (char*)acTextBuff, "LoRa Done" );
			DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
		}
		else
		{
			// OOOPS! Should never get here, as TX types are enumerated & only existing types should ever get through.
			sprintf( (char*)acTextBuff, (char*)"Unknown KeyWord TX TYPE - %0.4X - ERR!\n", enuKeyWordType );
			DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
		}
	}


	sprintf ( (char*)acTextBuff, (char*)"%sTX: >>>>>>  END - %s\n\n", pszCommsType, (char*)"LoRa Processed\r\n\r\n");
	DP_Debug_Print( enuDebugOutputType, acTextBuff );

	return ( sCommandLength );
}
#endif	//	HDF_VARADIC_TX




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @fucntion	: DP_Protocol_KeyWordTX 
 **
 ** @brief		: Process & transmit a KeyWord command.
 **
 ** @param		: ENUM_KEYWORD_TYPE transmition type - ENU_KW_LORA, ENU_KW_SOCKET, etc...
 ** @param		: Buffer to transmit
 ** @param		: Client ID that we're transmitting to (0xffff == broadcast to all).
 **
 ** @return		: uint16_t Command length (bytes).
 **************************************************************************************************
 *************************************************************************************************/
uint16_t	DP_Protocol_KeyWordTX ( ENUM_KEYWORD_TYPE enuKeyWordType, _xCommsDX *pxCommandTX, uint16_t usClientID )
{
	int16_t	sCount;
	int16_t	sCommandLength;
	int8_t	acTextBuff[128];
	uint8_t	*paucBuffer;
	int16_t	usCommand;
	
	
	// Header type, for debug output.
	DP_Protocol_KeywordDebugType( enuKeyWordType );

	sprintf( (char*)acTextBuff, (char*)"\n\n%sTX: >>>>>>  START - %s", pszCommsType, HDF_SYSTEM_PPU_NAME );
	DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );

	sCommandLength = 0;
	usCommand = pxCommandTX->xCommsHeaderDX.usCommand;	// shorter & easier to type;

	sprintf( (char*)acTextBuff, (char*)"\n%sTX: COMMAND ID = $%0.4X ", pszCommsType, usCommand );
	DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );

	// ?? Are we sending a reply with the ERROR bit set (HDF_DX_BIT_REPLY_WITH_ERROR) ??
	if ( (HDF_DX_MASK_CMD_STATUS_NIBBLE & usCommand) == HDF_DX_BIT_REPLY_WITH_ERROR )
	{
		sprintf( (char*)acTextBuff, (char*)"\n%sTX: REPLY - ERROR CODE = %.4X ", pszCommsType, usCommand );

		pxBufferDX->xCommsHeaderDX.usCommand = pxCommandTX->xCommsHeaderDX.usCommand;

		DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
		sCommandLength += sizeof( int16_t );
	}
	// ?? Are we sending a reply with the OK bit set (HDF_DX_BIT_REPLY_WITH_OK) ??
	else if ( (HDF_DX_MASK_CMD_STATUS_NIBBLE & usCommand) == HDF_DX_BIT_REPLY_WITH_OK )
	{
		sprintf( (char*)acTextBuff, (char*)"\n%sTX: REPLY - {{ ACK OK }} = %.4X ", pszCommsType, usCommand );
		DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
	}
	// We're requesting data or sending a one shot command.
	else
	{
		sprintf( (char*)acTextBuff, (char*)"%sTX: ", pszCommsType );
		DP_Debug_Print( enuDebugOutputType, acTextBuff );


		// ?? If bit #12 set we are requesting data from another device, else sending data.
		sprintf( (char*)acTextBuff, (char*)"%s", usCommand & HDF_DX_BIT_REQUEST_DATA ? (char*)"REQUEST - " : "ONE SHOT SEND - " );
		DP_Debug_Print( enuDebugOutputType, acTextBuff );

		
		// Mask away status/info nibble to decode command.
		switch ( HDF_DX_MASK_COMMANDS & usCommand )
		{
#if 0
		// Initiate firmware update.
		case	HDF_DX_PPU_FIRMWARE_UPDATE_CONNECT:
			// !!!! DEBUG INFO !!!!
			sprintf( (char*)acTextBuff, (char*)"FIRMWARE - UPDATE!" );
			DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );

		
			// Clear all bits in the flash-map which indicate the pages that have been received.
			for ( sCount = 0; sCount < HDF_OTA_BITMASK_SIZE; sCount++ )
			{
				xRadioBoard.xOTA.ablBitMask[sCount] = false;
			}
			xRadioBoard.xOTA.uiCurrentBlock = 0;
			xRadioBoard.xOTA.uiCurrentPage = 0;
			xRadioBoard.xOTA.usTransferSizeTotal = 0;

			xRadioBoard.xOTA.usTransferSizeTotal = pxBufferDX->uxCommandData.uxDataType.xDataDX.uiDataSizeTotal;
			xRadioBoard.xOTA.usBlockSize = pxBufferDX->uxCommandData.uxDataType.xDataDX.ucBlockSize;

			xRadioBoard.xOTA.blPPU_Connected = true;
			break;

		// Send firmware page data.
		case	HDF_DX_PPU_FIRMWARE_UPDATE_OTA:
			// Current page number.
			pxBufferDX->uxCommandData.uxDataType.xDataDX.usPageNumber = pxCommandTX->uxCommandData.uxDataType.xDataDX.usPageNumber;
			sCommandLength += sizeof( int16_t );

			// Total number of pages.
			pxBufferDX->uxCommandData.uxDataType.xDataDX.usPagesTotal = pxCommandTX->uxCommandData.uxDataType.xDataDX.usPagesTotal;
			sCommandLength += sizeof( uint16_t );

			// Number of bytes in current page.
			pxBufferDX->uxCommandData.uxDataType.xDataDX.usPageSize = pxCommandTX->uxCommandData.uxDataType.xDataDX.usPageSize;
			sCommandLength += sizeof( uint8_t );

			// !!!! DEBUG INFO !!!!
			sprintf( (char*)acTextBuff,
						(char*)"FIRMWARE - PAGE %3.0d - PSIZE %3.0d - PTOTAL %3.0d  ::  BLOCK %3.0d - BSIZE %3.0d",
						pxBufferDX->uxCommandData.uxDataType.xDataDX.usPageNumber,
						pxBufferDX->uxCommandData.uxDataType.xDataDX.usPageSize,
						pxBufferDX->uxCommandData.uxDataType.xDataDX.usPagesTotal,

						pxBufferDX->uxCommandData.uxDataType.xDataDX.ucBlockNumber,
						pxBufferDX->uxCommandData.uxDataType.xDataDX.ucBlockSize
						);
			// !!!! DEBUG INFO !!!!

			// Store bytes from data buffer into the transmission structure.
			for ( uint8_t i = 0; i < pxBufferDX->uxCommandData.uxDataType.xDataDX.usPageSize; i++ )
			{
				pxBufferDX->uxCommandData.uxDataType.xDataDX.aucPayload[i] = pxCommandTX->uxCommandData.uxDataType.xDataDX.aucPayload[i];
			}
			sCommandLength += pxBufferDX->uxCommandData.uxDataType.xDataDX.usPageSize;
			break;
#endif
		case HDF_DX_INT:
			DP_Debug_PrintCR( enuDebugOutputType, (int8_t*)"uint16_t" );
			pxBufferDX->uxCommandData.uxDataType.usShort = pxCommandTX->uxCommandData.uxDataType.sShort;
			sCommandLength += sizeof( int16_t );
			break;


		case HDF_DX_FLOAT:
			DP_Debug_PrintCR( enuDebugOutputType, (int8_t*)"FLOAT" );
			pxBufferDX->uxCommandData.uxDataType.fFloat = pxCommandTX->uxCommandData.uxDataType.fFloat;
			sCommandLength += sizeof( float );
			break;


		case HDF_DX_STRING:
				pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount = strlen( (char*)pxCommandTX->uxCommandData.uxDataType.xByteStream.aucBytes );

				sprintf( (char*)acTextBuff, (char*)"STRING LEN = %d - [ %s ]",
										pxCommandTX->uxCommandData.uxDataType.xByteStream.usByteCount,
										pxCommandTX->uxCommandData.uxDataType.xByteStream.aucBytes );
				DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );

				// String size check.
				if ( HDF_DX_MAX_COMMAND_DATA > pxCommandTX->uxCommandData.uxDataType.xByteStream.usByteCount )
				{
					for ( sCount = 0; sCount < pxCommandTX->uxCommandData.uxDataType.xByteStream.usByteCount; sCount++ )
					{
						pxBufferDX->uxCommandData.uxDataType.xByteStream.aucBytes[sCount] = pxCommandTX->uxCommandData.uxDataType.xByteStream.aucBytes[sCount];
					}

					sCommandLength += (pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount + sizeof(pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount));
				}
				else
				{
					// !! ERRRM!
					pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount = 0;
					DP_Debug_PrintCR( enuDebugOutputType, (int8_t*)"STRING - ERR!" );
				}
			break;



		// CYGNUS DATA-SIZE TRANSFER TEST!!!!
		// CYGNUS DATA-SIZE TRANSFER TEST!!!!
		case HDF_DX_CYGNUS_PULSE:
			sprintf( (char*)acTextBuff, (char*)"CYGNUS PULSE" );
			DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );

			// Running Counter.
			pxBufferDX->uxCommandData.uxDataType.xCygnusPulse.iCounter = pxCommandTX->uxCommandData.uxDataType.xCygnusPulse.iCounter;
			sCommandLength += sizeof( uint16_t );
		
			// Size of data-packet to send (in bytes).
			pxBufferDX->uxCommandData.uxDataType.xCygnusPulse.iDataSize = pxCommandTX->uxCommandData.uxDataType.xCygnusPulse.iDataSize;
			sCommandLength += sizeof( uint16_t );
			
			// Data array.
			if ( HDF_DX_MAX_CYGNUS_PACKET > pxBufferDX->uxCommandData.uxDataType.xCygnusPulse.iDataSize )
			{
				for ( sCount = 0; sCount < pxBufferDX->uxCommandData.uxDataType.xCygnusPulse.iDataSize; sCount++ )
				{
					pxBufferDX->uxCommandData.uxDataType.xCygnusPulse.acData[sCount] = pxCommandTX->uxCommandData.uxDataType.xCygnusPulse.acData[sCount];
				}
				sCommandLength += pxBufferDX->uxCommandData.uxDataType.xCygnusPulse.iDataSize;
			}
			else
			{
				// !! ERRRM!
				pxBufferDX->uxCommandData.uxDataType.xByteStream.usByteCount = 0;
				DP_Debug_PrintCR( enuDebugOutputType, (int8_t*)"ERR: CYGNUS PULSE SIZE" );
			}
		
			break;
				// CYGNUS DATA-SIZE TRANSFER TEST!!!!
				// CYGNUS DATA-SIZE TRANSFER TEST!!!!

		
			
		case HDF_DX_BLIP_TEST:
			//sprintf( (char*)acTextBuff, (char*)"BLIP_TEST" );
			sprintf( (char*)acTextBuff, (char*)"PING_TEST" );
			DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
			// Interger value.
			pxBufferDX->uxCommandData.uxDataType.xBlipTest.iCounter = pxCommandTX->uxCommandData.uxDataType.xBlipTest.iCounter;
			sCommandLength += sizeof( uint16_t );

			if ( HDF_DX_MAX_BLIP_TEXT > strlen( (char*)paucBuffer ) )
			{
				strcpy( (char*)pxBufferDX->uxCommandData.uxDataType.xBlipTest.pszString, (char*)pxCommandTX->uxCommandData.uxDataType.xBlipTest.pszString );
				sCommandLength += strlen( (char*)pxBufferDX->uxCommandData.uxDataType.xBlipTest.pszString );

				sCommandLength++;	// For NULL Terminator.

				sprintf( (char*)acTextBuff, "%sTX: BLIP = \'%s\' - LENGTH: #%d",
							pszCommsType,
							pxBufferDX->uxCommandData.uxDataType.xBlipTest.pszString,
							strlen( (char*)pxBufferDX->uxCommandData.uxDataType.xBlipTest.pszString )
							);
				
				DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
			}
			else
			{
				// !! ERRRM!
				sCommandLength = 0;

				sprintf( (char*)acTextBuff, (char*)"%sTX: BLIP SIZE - ERR!", pszCommsType );
				DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
			}
			break;

		case HDF_DX_PPU_PING_SEND:
			sprintf( (char*)acTextBuff, (char*)"%sTX: PING", pszCommsType );
			DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
			pxBufferDX->uxCommandData.uxDataType.usShort = pxCommandTX->uxCommandData.uxDataType.xPing.usDeviceID;
			sCommandLength += sizeof( int16_t );
		
			break;
		

		case HDF_DX_PPU_FIRMWARE_REQUEST_MISSED:
			sprintf( (char*)acTextBuff, (char*)"%sTX: FIRMWARE_REQUEST_MISSED", pszCommsType );
			DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );

			pxBufferDX->xCommsHeaderDX.usCommand = pxCommandTX->xCommsHeaderDX.usCommand;
	
			// Counter with number of missing pages.
			pxBufferDX->uxCommandData.uxDataType.xPacketsMissing.iMissing = pxCommandTX->uxCommandData.uxDataType.xPacketsMissing.iMissing;

			// Array of 16-bit numbers for each missing page ( max 16 pages per repsonse).
			for ( sCount = 0; sCount < HDF_PAGES_MISSING16; sCount++ )
			{
				pxBufferDX->uxCommandData.uxDataType.xPacketsMissing.ausPages[sCount] = pxCommandTX->uxCommandData.uxDataType.xPacketsMissing.ausPages[sCount];
			}
			sCommandLength += (sizeof( int16_t ) + HDF_PAGES_MISSING16);
			break;

			
		case HDF_DX_BIT_REPLY_WITH_ERROR:
			pxBufferDX->uxCommandData.uxDataType.usErrorCode = pxCommandTX->uxCommandData.uxDataType.usErrorCode;
		
			sprintf( (char*)acTextBuff, (char*)"\nERROR CODE = %.4X", pxBufferDX->uxCommandData.uxDataType.usErrorCode );
			DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
			sCommandLength += sizeof( int16_t );
			break;

		
		default:
			sprintf( (char*)acTextBuff, (char*)"\nUnknown KeyWord CMD!!!   %.4X - ERR!\n", usCommand  );
			DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
			sCommandLength = -1;
			break;
		} // switch ( usCommand & HDF_DX_MASK_COMMANDS )
	}
	//va_end( vaArgsList );


	// ?? Zero or above == OK to process ??
	if ( 0 <= sCommandLength )
	{
		// YES! OK to process.
		
		// Build protocol header.
		pxBufferDX->xCommsHeaderDX.usHostID = HDF_SYSTEM_PPU_ID;
		//pxBufferDX->xCommsHeaderDX.usClientID = 0xffff;		//	0xffff = ALL UNITS!!!!
		pxBufferDX->xCommsHeaderDX.usClientID = usClientID;
		pxBufferDX->xCommsHeaderDX.usChecksum = 0;
		pxBufferDX->xCommsHeaderDX.usPacketLength = sizeof( _xCommsHeaderDX ) + sCommandLength;
		pxBufferDX->xCommsHeaderDX.usCommand = usCommand;

		uint16_t	usChecksum;
		usChecksum = 0;
		paucBuffer = (uint8_t*)pxBufferDX;

		// ?? Calculate checksum ??
		sprintf ( (char*)acTextBuff, (char*)"%sTX: ", pszCommsType );
		DP_Debug_Print( enuDebugOutputType, acTextBuff );

		for ( uint16_t i = 0; i < pxBufferDX->xCommsHeaderDX.usPacketLength; i++ )
		{
			usChecksum += (uint8_t)paucBuffer[i];
			sprintf ( (char*)acTextBuff, (char*)"%.2X ", paucBuffer[i] );
			CO_PRINT_A_0( DBG_NOPREFIX, (char*)acTextBuff );
		}
		pxBufferDX->xCommsHeaderDX.usChecksum = usChecksum;

		sprintf( (char*)acTextBuff, (char*)"\n%sTX: SIZE = #%d", pszCommsType, pxBufferDX->xCommsHeaderDX.usPacketLength );
		DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );

		sprintf( (char*)acTextBuff, (char*)"%sTX: CHECKSUM = $%0.4d", pszCommsType, pxBufferDX->xCommsHeaderDX.usChecksum );
		DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );

		// LoRa Packet ( Catering for multiple transmission types in future ).
		if ( ENU_KW_LORA == enuKeyWordType )
		{
			DP_Protocol_SendLoRaPacket ( (uint8_t*)pxBufferDX, pxBufferDX->xCommsHeaderDX.usPacketLength );
		}
		else
		{
			// Should never get here, as TX types are all enumer+6+ated & only existing types should ever get through.
			sprintf( (char*)acTextBuff, (char*)"\nUnknown KeyWord TX TYPE - %.4X - ERR!\n", enuKeyWordType );
			DP_Debug_PrintCR( enuDebugOutputType, acTextBuff );
		}
	}

	sprintf ( (char*)acTextBuff, (char*)"\n%sTX: >>>>>>  END - %s\n\n", pszCommsType, (char*)"LoRa Processed\r\n\r\n");
	DP_Debug_Print( enuDebugOutputType, acTextBuff );

	return ( sCommandLength );
}


/***************************************************************************************************
 ***************************************************************************************************
 **	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!
 ***************************************************************************************************
 **************************************************************************************************/
