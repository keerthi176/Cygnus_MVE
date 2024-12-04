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
* USB Virtual serial port device manager
*
**************************************************************************/
#ifndef __USB_SERIAL__
#define __USB_SERIAL__


/* Defines
**************************************************************************/


/* System Include Files
**************************************************************************/



/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_Pipe.h"
#include "MM_ATCommand.h"
#include "MM_USBSerial.h"
#include "MM_ImportXML.h"
#include "MM_EmbeddedWizard.h"


class Settings;
class QuadSPI;

enum USBReceiverMode
{
	USBDATA_ATCMD,
	USBDATA_XML,
	USBDATA_BMP,
	USBDATA_VIDEO,
	USBDATA_NCU,
	USBDATA_EXPORT_XML,
	USBDATA_EXPORT_LOG,
	USBDATA_END,
};


class USBReceiver
{
	public:
		
	int ReceiveUSBData( char* data, int size );
};
		

 


class USBSerial : public Module, public Progressor
{
	public:
	
	USBSerial( );
	
	virtual int Init( );
	
	virtual void Poll( );
	
	virtual int Receive( Event );
	virtual int Receive( Message* );
	virtual int Receive( Command* );
 
	static int8_t CDC_Init_FS( void );
	static int8_t CDC_DeInit_FS( void );
	static int8_t CDC_Control_FS( uint8_t cmd, uint8_t* pbuf, uint16_t length );
	static int8_t CDC_Receive_FS( uint8_t* pbuf, uint32_t *Len );
	static int8_t CDC_TransmitCplt_FS( uint8_t *Buf, uint32_t *Len, uint8_t epnum );
	static int TxString( char* data );
	
	static int RegisterReceiver( USBReceiver*, USBReceiverMode );
	
	static void WriteBitmap( char* data, int size, int start );
//	static void WriteVideo( char* data, int size, int start );
	
	int redirect_countdown;
	
	static void GetScreen( );
	
	static USBReceiverMode  currentMode; 
	static int 				expectedXMLbytes;
	
	static USBSerial* me;
	
	static Site* orig_site_addr;
	static Panel* orig_panel_addr;
	
	static bool tx_complete;
	
	protected:
		
	static int log_line;
	static int mesh_log_line;
		
	static int expectedpixels;
	static int pixelsreceived;
	static int bitmapWidth;
	static int bitmapHeight;
		
	virtual int GetPercentage( );
	virtual void Cancel( );
	
	static int transmitting;
	static int receiving;
	static int errorwas;
	
	static QuadSPI*			qspi;
	static ATCommands* 		atcmds;
	static ImportXML* 		ixml;
	static EmbeddedWizard*	ew;
	static Settings* 			settings;
	
	static ExportXML* 		oxml;
		 
	static int 					bytesreceived;
};

#endif
