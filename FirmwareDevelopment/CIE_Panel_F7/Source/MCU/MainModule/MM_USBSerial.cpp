/***************************************************************************
* File name: MM_USBSerial.cpp
* Project name: CIE_Panel
*
* Copyright 2018 Bull Products Ltd as an unpublished work.
* All Rights Reserved.
*
* The information contained herein is confidential property of Bull
* Products Ltd. The use, copying, transfer or disclosure of such
* information is prohibited except by express written agreement with Bull
* Products Ltd.
*
* First written on 18/05/18 by Matt Ault
*
* File History:
* Change History:
* Rev Date Description Author
* 1.0 18/05/18 Initial File
*
* Description:
* USB Serial interface
*
**************************************************************************/


/* System Include Files
**************************************************************************/
#include <string.h>
#include "usbd_cdc_if.h"
#include <assert.h>
#include <stdio.h>
#include "stm32f7xx_hal_uart.h"
#include "stm32f7xx_hal_crc.h"


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_USBSerial.h"
#include "MM_Module.h"
#include "MM_Pipe.h"
#include "CO_Site.h"
#include "MM_fault.h"
#include "MM_issue.h"
#include "MM_QuadSPI.h"
#include "MM_Settings.h"
#include "MM_CUtils.h"
#include "ewextgfx.h"
#include "MM_NCU.h"
#include "MM_CSV.h"
#include "MM_MeshLog.h"


/* Defines
**************************************************************************/
#define APP_RX_DATA_SIZE  1024
#define APP_TX_DATA_SIZE  1024

#define MAX_TIME_DIFF 6

#define BMP_WIDTH		180
#define BMP_HEIGHT 	64

#define MSG_GET_SCREEN 		1
#define MSG_FLIP_ON 			2
#define MSG_FLIP_OFF 		3
#define MSG_USB_TIMEOUT		4

extern uint8_t UserRxBufferFS[ APP_RX_DATA_SIZE + 1];
extern uint8_t UserTxBufferFS[ APP_TX_DATA_SIZE ];


extern USBD_HandleTypeDef hUsbDeviceFS;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

int   		USBSerial::transmitting;
int   		USBSerial::receiving;

bool 			USBSerial::tx_complete;

ATCommands* 		USBSerial::atcmds;
ImportXML* 			USBSerial::ixml;
ExportXML* 			USBSerial::oxml;
EmbeddedWizard*	USBSerial::ew;
Settings*			USBSerial::settings;
QuadSPI*				USBSerial::qspi;

Site*  USBSerial::orig_site_addr;
Panel* USBSerial::orig_panel_addr;

int 	USBSerial::bitmapWidth = 0;
int   USBSerial::bitmapHeight = 0;

int 					USBSerial::pixelsreceived;
int					USBSerial::expectedpixels;
int 					USBSerial::expectedXMLbytes;
USBReceiverMode   USBSerial::currentMode; 
int 					USBSerial::bytesreceived;

USBSerial* 			USBSerial::me;
int 					USBSerial::errorwas;

static Panel* panel_addr;
static Site* site_addr;

int USBSerial::log_line = -1;
int USBSerial::mesh_log_line = -1;
 
#if 0
static USBD_CDC_ItfTypeDef USBD_fops_FS =
{
	USBSerial::CDC_Init_FS,
	USBSerial::CDC_DeInit_FS,
	USBSerial::CDC_Control_FS,
	USBSerial::CDC_Receive_FS,
	USBSerial::CDC_TransmitCplt_FS,
};
#endif

/**
  * @brief  DInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
/*************************************************************************/
/**  \fn      int8_t USBSerial::CDC_Init_FS( void )
***  \brief   Class function
**************************************************************************/

int8_t USBSerial::CDC_Init_FS( void )
{
	USBD_CDC_SetTxBuffer( &hUsbDeviceFS, UserTxBufferFS, 0 );
	USBD_CDC_SetRxBuffer( &hUsbDeviceFS, UserRxBufferFS );

	return USBD_OK;
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
/*************************************************************************/
/**  \fn      int8_t USBSerial::CDC_DeInit_FS( void )
***  \brief   Class function
**************************************************************************/

int8_t USBSerial::CDC_DeInit_FS( void )
{
   return USBD_OK;
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
/*************************************************************************/
/**  \fn      int8_t  USBSerial::CDC_Control_FS( uint8_t cmd, uint8_t* pbuf, uint16_t length )
***  \brief   Class function
**************************************************************************/

int8_t  USBSerial::CDC_Control_FS( uint8_t cmd, uint8_t* pbuf, uint16_t length )
{
   return USBD_OK;
}

 
	
/**
  * @brief  CDC_TransmitCplt_FS
  *         Data transmited callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
extern "C" int8_t CDC_TransmitCplt_FS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 13 */
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);
	
	USBSerial::tx_complete = true;
	
  /* USER CODE END 13 */
  return result;
}

 

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will block any OUT packet reception on USB endpoint
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result
  *         in receiving more data while previous ones are still not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */

/*************************************************************************/
/**  \fn      int USBSerial::TxString( char* data )
***  \brief   Class function
**************************************************************************/

int USBSerial::TxString( char* data )
{
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
	
	if ( hcdc->TxState == 0 )
	{
		int len = strlen( data );
		
		if ( len > 0 )
		{
			USBD_CDC_SetTxBuffer( &hUsbDeviceFS, (uint8_t*) data, len );
			tx_complete = false;
			if ( USBD_OK == USBD_CDC_TransmitPacket( &hUsbDeviceFS ) )
			{
				return true; 
			}
		}
	}	
	return false;
}


/*************************************************************************/
/**  \fn      static char conv( char* p )
***  \brief   Helper function
**************************************************************************/

static char conv( char* p )
{
	if ( *p >= 'A' ) return ( *p - 'A' ) + 10;
	return *p - '0';
}

static int finished_ok = 0;
/*************************************************************************/
/**  \fn      void USBSerial::WriteBitmap( char* data, int size, int start )
***  \brief   Helper function
**************************************************************************/

void USBSerial::WriteBitmap( char* data, int size, int start )
{
	static int mempos = 0;
	char buff[ 72 ];
	short* buffp = (short*)buff;

	
	if ( size > 68 ) return;
	if ( size == 0 ) return;
	
	pixelsreceived += size / 4;

	data[ size ] = 0;
	
//	app.DebOut( "BMP data='%s'\n", data );
	
	char* p = strchr( data, '\r' );
		
	if ( p )
	{
		size = p - data;
		app.DebOut( "Received:%d\n", (mempos - QSPI_BITMAP_ADDRESS) + size );
		app.DebOut( "Expected:%d", bitmapWidth * bitmapHeight * 4 + 4 );
		
	 	currentMode = USBDATA_ATCMD;
		finished_ok = true;
	}

	static int w = 0;
	static int h = 0;
 
	if ( start ) 
	{
		w = 0;
		h = 0;
 		
		// Wriate and adjust for 4 byte header
		mempos = QSPI_BITMAP_ADDRESS + 4;
		sprintf( buff, "BMP=" );
			
		buffp += 2;
		
		finished_ok = false;
	}
	
 
	for( int n = 0; n < size; n+=4 )
	{
		unsigned short c16 = (conv( data + n ) << 4 ) + (conv( data + n + 1) << 0);
		c16 |= (conv( data + n + 2) << 12 ) + (conv( data + n + 3) << 8);

		*buffp++ = c16;
 
		w++;
	 	if ( w == bitmapWidth )
	 	{
			w = 0;
	 
			h++;
			if ( h >= bitmapHeight && w != 0 )
			{					
				app.DebOut( "BMP too much data!" );
				currentMode = USBDATA_ATCMD;
				mempos = 0;
			}				
		}
	}
	
	if ( mempos != 0 ) 
	{
		int block = (char*)buffp - (char*)buff;
		if (qspi->WriteDontEraseToFlash( buff, mempos, block ) != QSPI_RESULT_OK )
		{
			app.DebOut( "Write fail!" );
			currentMode = USBDATA_ATCMD;
		}
		mempos += block;
	}
		
	if ( currentMode == USBDATA_ATCMD )
	{
		ew->StopProgress( finished_ok );
		
		if ( finished_ok )
		{
			me->Send( EW_NEW_BITMAP_LOGO_MSG, ew );
			
			TxString( "BDATA:OK\r\n" );
		}
		else TxString( "BDATA:ERROR\r\n" );
	}
}

#if 0

char buff[ 4 * 1024 + 64 ]; 


/*************************************************************************/
/**  \fn      USBSerial::WriteVideo( char* data, int size, int start )
***  \brief   Helper function
**************************************************************************/

void USBSerial::WriteVideo( char* data, int size, int start )
{
	static int mempos = 0;
	static int spos = 0;
 
	if ( start ) mempos = QSPI_VIDEO_ADDRESS;
	
	memcpy( buff + spos, data, size );
	spos += size;
	
	HAL_CRC_Accumulate( app.crc, (uint32_t *) data, size/4 );
	
	if ( spos >= 4*1024 )
	{
		app.DebOut( "Received:%d\n", (mempos - QSPI_VIDEO_ADDRESS) + size );
	 
		if ( mempos + spos >= QSPI_END_ADDR )
		{
			spos = QSPI_END_ADDR - mempos;

			currentMode = USBDATA_ATCMD;		 
		}
		
		if ( spos > 0 )
		{
			qspi->WriteToFlash( buff, mempos, spos );
			
			mempos += spos;
		}
		spos = 0;
	}
}
#endif

/*************************************************************************/
/**  \fn      void USBSerial::GetScreen( )
***  \brief   Class function
**************************************************************************/

void USBSerial::GetScreen( )
{
	uint16_t* disp = (uint16_t*)0xC0000000;
	
	uint8_t buffer[ 60 ];
	uint16_t* ybuff = disp; 
	
	for( int y = 0; y < 480; y++ )
	{
		ybuff += 800;
		for( int x = 0; x < 800; x+= 20 )
		{
			uint16_t* xbuff = ybuff - x;
			uint8_t* buff = buffer;
			
			for( int p = 0; p < 20; p++ )
			{
				uint16_t c = *--xbuff;
				*buff++ = ( c & 31 ) << 3;
				*buff++ = ( ( c >> 5 ) & 63 ) << 2;
				*buff++ = ( ( c >> 11 ) & 31 ) << 3;
			}
			USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
			while ( hcdc->TxState != 0 );
			CDC_Transmit_FS( buffer, sizeof(buffer) );
		}
	}
}

static time_t last_time = UINT_MAX;
 
			
/*************************************************************************/
/**  \fn      int8_t USBSerial::CDC_Receive_FS( uint8_t* Buf, uint32_t *Len )
***  \brief   Class function
**************************************************************************/

int8_t USBSerial::CDC_Receive_FS( uint8_t* data, uint32_t* len )
{
	__ALIGN_BEGIN
	static char  reply[ 320 ];
	__ALIGN_END;
 

	// Check receiver mode
	if ( currentMode == USBDATA_NCU )
	{
		if ( strncmp( (char*) data, "ATFLOP", 6 ) )
		{
			last_time = now( );
		//	data[ *len ] = 0;
		//	app.DebOut("NCU-> %s", data );
			app.DebOut("N:%d\n", *len );
		 
			if ( HAL_UART_Transmit( app.ncu_uart, data, *len, 1000 ) != HAL_OK )
			{
				app.DebOut(" failed.");
			}
		}
		else
		{
			app.DebOut( "Flip off\n" );
			me->Send( MSG_FLIP_OFF );
		
			currentMode = USBDATA_ATCMD;	
		}
	}
	else if ( currentMode == USBDATA_VIDEO )
	{
	//	WriteVideo( (char*) data, *len, false );
	}
	else if ( currentMode == USBDATA_BMP )
	{
		WriteBitmap( (char*) data, *len, false );
	}
	else if ( currentMode == USBDATA_ATCMD )
	{		
		data[ *len ] = 0;  // terminate
		app.DebOut("A:%d\n", *len );
	/*
		if ( !strncmp( (char*) data, "ATVIDEO=", 8 ) )
		{
			currentMode = USBDATA_VIDEO;
			 __HAL_CRC_DR_RESET( app.crc );
			WriteVideo( (char*) data + 8, *len - 8, true );			
		}	
		else*/
		
		if ( !strncmp( (char*) data, "ATXML?", 6 ) )	
		{
			oxml->Start( app.site, EXPORT_TO_USBSERIAL );
			
			currentMode = USBDATA_EXPORT_XML;
			
			*reply = 0;
		}
		else if ( !strncmp( (char*) data, "ATLOG?", 6 ) )	
		{
			uint32_t length = 0;
			
			Log::logger.GetLength( length );
			
			sprintf( reply, "LOG:%u\r\n", length );	// 0 if fail
			
			TxString( reply );
		}
		else if ( !strncmp( (char*) data, "ATLINE?", 7 ) )	
		{
			sscanf( (const char*)data + 7, "%u", &log_line );
		}
		else if ( !strncmp( (char*) data, "ATMLOG?", 7 ) )	
		{
			uint32_t length = 0;
			
			NCU::logger.GetLength( length );
			
			sprintf( reply, "MLOG:%u\r\n", length );	// 0 if fail
			
			TxString( reply );
		}
		else if ( !strncmp( (char*) data, "ATMLINE?", 8 ) )	
		{
			sscanf( (const char*)data + 8, "%u", &mesh_log_line );
		}
		else if ( !strncmp( (char*) data, "ATCSV?", 6 ) )	
		{
			uint32_t length = 0;
			
			if ( app.panel != NULL )
			{
				length = app.panel->numRadioDevs;
			}
			sprintf( reply, "CSV:%u\r\n", length );	// 0 if fail
			
			TxString( reply );
		}
		else if ( !strncmp( (char*) data, "ATROW?", 6 ) )	
		{
			uint32_t line;
			
			if ( sscanf( (const char*)data + 6, "%u", &line ) == 1 )
			{
				if ( CSV::GetLine( reply, line ) )
				{
					 // wahey
				}
				else
				{
					*reply = '\n';
					CDC_Transmit_FS( (uint8_t*)reply, 1 );			
					USBD_CDC_SetRxBuffer( &hUsbDeviceFS, UserRxBufferFS );
					USBD_CDC_ReceivePacket(&hUsbDeviceFS);		
					return  (USBD_OK);					
				}
			}
			int timeout = 0;
			
			while ( !tx_complete )
			{
				if ( timeout++ > 50 )
				{
					me->Send( MSG_USB_TIMEOUT );
			 
					tx_complete = true;
					break;
				}
				HAL_Delay(2);
			}
			 
			CDC_Transmit_FS( (uint8_t*)reply, strlen( reply ) );
		}
		else if ( !strncmp( (char*) data, "ATNOBMP", 7 ) )
		{
			if ( qspi->WriteDontEraseToFlash( (char*)"\0\0\0\0", QSPI_BITMAP_ADDRESS + 4, 4 ) != QSPI_RESULT_OK )
			{
				sprintf( reply, "ATNOBMP:ERROR\r\n" );
			}
			else
			{
				// Refresh GUI
				me->Send( EW_NEW_BITMAP_LOGO_MSG, ew );
				
				sprintf( reply, "ATNOBMP:OK\r\n" );
			}
			TxString( reply ); 
		}
		else if ( !strncmp( (char*) data, "ATBMP", 5 ) )
		{
			sprintf( reply, "BMP:ERROR\r\n" );
			
			if ( sscanf( (char*) data, "ATBMP=%d,%d\r\n", &bitmapWidth, &bitmapHeight ) == 2 )
			{
				short p[ 2];
				
			//	app.DebOut( (const char*) data );
				
				expectedpixels = bitmapWidth * bitmapHeight;
				int size = expectedpixels * 4;
				pixelsreceived = 0;
				
				int sectors = ( size + ( QSPI_SECTOR_SIZE - 1 ) ) / QSPI_SECTOR_SIZE;
				
				if ( sectors <= 12 )
				{
					int addr = QSPI_BITMAP_ADDRESS;
					
					for( int s = 0; s < sectors; s++ )
					{
						qspi->EraseSector( addr );
						addr += QSPI_SECTOR_SIZE;
					}
				 
					p[ 0] = (short) bitmapWidth;
					p[ 1] = (short) bitmapHeight;
					
					if ( qspi->WriteDontEraseToFlash( (char*) p, QSPI_BITMAP_ADDRESS, 4 ) == QSPI_RESULT_OK )
					{
						sprintf( reply, "BMP:OK\r\n" );
						ew->StartProgress( USBSerial::me );
					}
				}
			}
			TxString( reply ); 
		}
		else if ( !strncmp( (char*) data, "ATBDATA=", 8 ) )
		{
			if ( !qspi->IsWriteProtected( ) && bitmapWidth != 0 && bitmapHeight != 0 )
			{
				currentMode = USBDATA_BMP;
				WriteBitmap( (char*) data + 8, *len - 8, true );			
			}
			else
			{
				sprintf( reply, "BDATA:ERROR\r\n" );
			}
			TxString( reply );
		}
		else if ( !strncmp( (char*) data, "ATBDATA?", 8 ) )
		{
			if ( finished_ok )
			{
				TxString( "BDATA:OK\r\n" );
			}
			else
			{
				TxString( "BDATA:ERROR\r\n" );
			}	
		}
		else if ( !strncmp( (char*) data, "ATFLIP", 6 ) )
		{
			me->Send( MSG_FLIP_ON );
			last_time = now( );
		}
		else if ( !strncmp( (char*) data, "ATSCREEN", 8 ) )
		{
			me->Send( MSG_GET_SCREEN);
		}
		else if ( !strncmp( (char*) data, "ATXML,", 6 ) )
		{
			expectedXMLbytes = 0;
			sscanf( (char*) data, "ATXML,%d", &expectedXMLbytes);
			
			if ( expectedXMLbytes != 0 )
			{				
				if ( qspi->IsWriteProtected( ) )
				{
					sprintf( reply, "XML:ERROR\r\n" );
					me->Send( MSG_SITE_TRANSFER_FAIL, settings );
					atcmds->SetError( XML_WRITE_PROTECTED, 0 );
				}
				else
				{
					sprintf( reply, "XML:OK\r\n" );
				
					ew->StartProgress( USBSerial::me );
					
					atcmds->SetError( XML_INIT_FAIL, 1 );	// in case it does..
					
					ixml->xmlp->line = 1;
					ixml->writeAddr = USBSerial::settings->GetWriteAddress( );
					ixml->xmlp->Start( );
						
					currentMode = USBDATA_XML;
					bytesreceived = 0;
					errorwas = XML_OK;
				}	
				TxString( reply );			
			}		
		}
		// if LF termiated
		else if ( memchr( (const char*) data, '\n', 64 ) )
		{
			atcmds->DecodeCommand( (char*) data, reply );
			
			TxString( reply );
		}
	}
	else if ( currentMode == USBDATA_XML )
	{			
		if ( errorwas == XML_OK )
		{
			errorwas = ixml->xmlp->Parse( (char*) data, *len );
			atcmds->SetError( errorwas, ixml->xmlp->line );
		}
		
		bytesreceived += *len;
		if ( bytesreceived >= expectedXMLbytes )
		{
			if ( errorwas == XML_OK )
			{
				if ( settings->ValidateSite( ixml->writeAddr, false ) )
				{
					me->Send( MSG_SITE_TRANSFER_SUCCESS, settings, ixml->writeAddr );
				}
				else
				{
					atcmds->SetError( XML_ILLEGAL_SITE, 0 );
					me->Send( MSG_SITE_TRANSFER_FAIL, settings );
				}
			}
			else			
			{
				me->Send( MSG_SITE_TRANSFER_FAIL, settings );
			}		
			currentMode = USBDATA_ATCMD;
		}
	}
	
	USBD_CDC_SetRxBuffer( &hUsbDeviceFS, UserRxBufferFS );
   USBD_CDC_ReceivePacket(&hUsbDeviceFS);
	
   return (USBD_OK);
   /* USER CODE END 6 */
}


extern "C" int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
   return USBSerial::CDC_Receive_FS( Buf, Len );
}


/*************************************************************************/
/**  \fn      void USBSerial::Cancel( )
***  \brief   Class function
**************************************************************************/

void USBSerial::Cancel( )
{
	errorwas = XML_USER_CANCEL;
}


/*************************************************************************/
/**  \fn      int USBSerial::GetPercentage( )
***  \brief   Class function
**************************************************************************/
	
int USBSerial::GetPercentage( )
{
	if ( USBSerial::currentMode == USBDATA_BMP )
	{
		return USBSerial::pixelsreceived * 100 / USBSerial::expectedpixels;
	}
	else if ( USBSerial::currentMode == USBDATA_XML )
	{	
		return USBSerial::bytesreceived * 100 / USBSerial::expectedXMLbytes;
	}
	return 0;
}

 #if 0

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
/*************************************************************************/
/**  \fn      uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
***  \brief   Global helper function
**************************************************************************/

uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  return result;
}

#endif 

/*****************************************************************/
/**  \fn      USBSerial::USBSerial( )
***  \brief   Constructor for class
**************************************************************************/

USBSerial::USBSerial( ) : Module( "USBSerial", 3 )
{		

}	


 
/*************************************************************************/
/**  \fn      int USBSerial::Init( )
***  \brief   Class function
**************************************************************************/

int USBSerial::Init( )
{
	atcmds   = (ATCommands*)		app.FindModule( "ATCommands" );
	ixml		= (ImportXML*) 		app.FindModule( "ImportXML" );
	ew       = (EmbeddedWizard*) 	app.FindModule( "EmbeddedWizard" );
	settings = (Settings*)			app.FindModule( "Settings" );
	qspi		= (QuadSPI*)			app.FindModule( "QuadSPI" );
 	oxml     = 	(ExportXML*) 		app.FindModule( "ExportXML" );
	
	assert( atcmds 	!= NULL );
	assert( ixml 		!= NULL );
	assert( ew 			!= NULL );
	assert( settings 	!= NULL );
	assert( qspi 		!= NULL );

	currentMode = USBDATA_ATCMD;
	
	//USBD_CDC_RegisterInterface( &hUsbDeviceFS, &USBD_fops_FS );

 //  USBD_Start( &hUsbDeviceFS );
	
	receiving = 0;
	transmitting = 0;
	
	me = this;
	
	 
	
	return true;
}


/*************************************************************************/
/**  \fn      int USBSerial::Poll( )
***  \brief   Class function
**************************************************************************/

void USBSerial::Poll( )
{
	char reply[ 100 ];

	if ( mesh_log_line >= 0 )
	{		
		if ( NCU::logger.GetLine( reply, mesh_log_line ) )
		{
			 // wahey
			//app.DebOut( "line %d\n", log_line );
		}
		else
		{
			*reply = '\n';
			CDC_Transmit_FS( (uint8_t*)reply, 1 );			
	 
			app.DebOut( "LOG LINE FAIL!!!\n" );
			
			mesh_log_line = -1;
			return ;					
		}
		mesh_log_line = -1;
		
		int timeout = 0;
		
		while ( !tx_complete )
		{
			 
			if ( timeout++ > 100 )
			{
				me->Send( MSG_USB_TIMEOUT );
				app.DebOut( "USB TIMEOUT FAIL!!!\n" );
				tx_complete = true;
				break;
			}
			HAL_Delay(2);
		}
		char* p = reply + 90;
		while ( *p == ' ' && p > reply ) --p;
		*++p = '\n';
		CDC_Transmit_FS( (uint8_t*)reply, ( p - reply ) + 1 );
	}		
	else if ( log_line >= 0 )
	{		
		if ( Log::logger.GetLine( reply, log_line ) )
		{
			 // wahey
			//app.DebOut( "line %d\n", log_line );
		}
		else
		{
			*reply = '\n';
			CDC_Transmit_FS( (uint8_t*)reply, 1 );			
	//		USBD_CDC_SetRxBuffer( &hUsbDeviceFS, UserRxBufferFS );
		//	USBD_CDC_ReceivePacket(&hUsbDeviceFS);		
			app.DebOut( "LOG LNE FAIL!!!\n" );
			
			log_line = -1;
			return ;					
		}
		log_line = -1;
		
		int timeout = 0;
		
		while ( !tx_complete )
		{
			app.DebOut( "*" );
			if ( timeout++ > 100 )
			{
				me->Send( MSG_USB_TIMEOUT );
				app.DebOut( "USB TIMEOUT FAIL!!!\n" );
				tx_complete = true;
				break;
			}
			HAL_Delay(2);
		}
		char* p = reply + 90;
		while ( *p == ' ' && p > reply ) --p;
		*++p = '\n';
		CDC_Transmit_FS( (uint8_t*)reply, ( p - reply ) + 1 );
	}	
		
	else if ( currentMode == USBDATA_NCU )
	{
		uint32_t time = now( );
		
		// if not had data for a while, return to AT command mode
		if ( time > last_time + MAX_TIME_DIFF )
		{
			app.DebOut( "Flip off.\n");
			
			// Restore panel / site
			app.site = site_addr;
			app.panel = panel_addr;  				 
		
			// Stop redirection
		//	NCU::directing_to_USB_eek = false; 	
			redirect_countdown = 50;
			
			currentMode = USBDATA_ATCMD;
		}
	}
	else if ( currentMode == USBDATA_EXPORT_XML )
	{
		if ( !oxml->finished )
		{
			if ( app.site != NULL && app.panel != NULL )
			{
				if ( tx_complete )
				{
					oxml->Output( );
				}
			}
		}
		else
		{
			currentMode = USBDATA_ATCMD;
		}
	}
	else if ( NCU::directing_to_USB_eek )
	{
		if ( redirect_countdown > 0 )
		{
			if ( --redirect_countdown == 0 )
			{
				NCU::directing_to_USB_eek = false; 	
				NCU::currentState = NCU_STATE_PRE_AT;
			}
		}
	}
}


/*************************************************************************/
/**  \fn      int USBSerial::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int USBSerial::Receive( Event event )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int USBSerial::Receive( Command* cmd  )
***  \brief   Class function
**************************************************************************/

int USBSerial::Receive( Command* cmd )
{
	if (cmd->type == CMD_CANCEL_PROGRESS )
	{
		currentMode = USBDATA_ATCMD;
		return CMD_OK;
	}
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int USBSerial::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int USBSerial::Receive( Message* m )
{
	if ( m->type == MSG_GET_SCREEN )
	{
		GetScreen( );
	}
	else if ( m->type == MSG_USB_TIMEOUT )
	{
		Fault::AddFault( FAULT_USB_TIMEOUT ); 
		Fault::RemoveFault( FAULT_USB_TIMEOUT );
	}
	else if ( m->type == MSG_FLIP_ON ) 
	{
		static uint8_t dummy_buff[ 65 ];
		
		app.DebOut( "Flip on.\n");
		
		panel_addr = app.panel;
		site_addr = app.site;
			
		app.site = NULL;
		app.panel = NULL;  				// suspend panel - eek (to stop NCU queu overflow)
		currentMode = USBDATA_NCU;
		
		HAL_UART_AbortTransmit( app.ncu_uart );
		HAL_UART_AbortReceive( app.ncu_uart );
		
		NCU::directing_to_USB_eek = true;
		redirect_countdown = 0;
		
		HAL_UART_Receive_IT( app.ncu_uart, dummy_buff, 64 );
				
		TxString( (char*)"FLIP:OK\r\n" ); 		
	}		
	else if ( m->type == MSG_FLIP_OFF ) 
	{
		// Restore panelk / site
		app.site = site_addr;
		app.panel = panel_addr;  				 
	
		// Stop redirection
		
		redirect_countdown = 50;
	//	NCU::directing_to_USB_eek = false; 		
		 
	}		
	
	return 0;
}
