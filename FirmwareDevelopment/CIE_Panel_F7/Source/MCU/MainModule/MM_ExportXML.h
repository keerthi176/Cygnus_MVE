/***************************************************************************
* File name: MM_ImportXML.h
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
* xml encode/decode
*
**************************************************************************/

#ifndef _XML_EXPORT_H_
#define _XML_EXPORT_H_

/* System Include Files
**************************************************************************/
#include <stdarg.h>


/* User Include Files
**************************************************************************/
#include "xml.h"
#include "MM_Module.h"
#include "CO_Site.h"



enum ExportType
{
	EXPORT_TO_USBSERIAL,
	EXPORT_TO_DEBUG,
	EXPORT_TO_USB_STORAGE,
	EXPORT_TO_SDCARD,
};


class ExportXML : public Module 
{
	public:
	
	ExportXML( );
	
	virtual int Receive( Message* );
	virtual int Receive( Event );
	virtual int Receive( Command* cmd );
	
	virtual void Poll( );
	
	bool Out( const char* fmt, ... );
 
	
	void DoOut( const char * fmt, va_list args );
	
	void Output( );
	
	void Start( Site* s, ExportType t );
	void Scan(  );
	
	ExportType 		direction;	
	Site*				site;
	
	int prev_line;
	int line;
 	
	bool finished;
	bool usb_timeout_fault; 
};



#endif
