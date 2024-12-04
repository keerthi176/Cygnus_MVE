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

#ifndef _XML_IMPORT_H_
#define _XML_IMPORT_H_


/* User Include Files
**************************************************************************/
#include "xml.h"
#include "MM_Module.h"
#include "CO_Site.h"
#include "MM_ExportXML.h"
#include "MM_Settings.h"
#include "MM_QuadSPI.h"
#include "MM_EEPROM.h"



/* Defines
**************************************************************************/
#define OFFSET(s,m) ((int)&((s*)0)->m)



class SiteHandler : public XMLElementHandler
{
	public:
	
	SiteHandler( );
	
	virtual int Begin( );
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
};	


class ProfileHandler : public XMLElementHandler
{
	public:
	
	ProfileHandler( );
	
	virtual int Begin( );
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
	
	private:
		
	Profile prof;
};	


class ProfilesHandler : public XMLElementHandler
{
	public:
	
	ProfilesHandler( );
	
	virtual int Begin( ); 
	virtual int End( );
	
 
	private:
		

	int selection;
};


class NetworkZoneHandler : public XMLElementHandler
{
	public:
	
	NetworkZoneHandler( );
	
	virtual int Begin( );
	virtual int End( );
	virtual int Attribute( char* name, char* value );
	 
	private:
 
};	


class NetEventHandler : public XMLElementHandler
{
	public:
	
	NetEventHandler( );
	
	virtual int Begin( ); 
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
	
	private:
 
};


class ContactHandler : public XMLElementHandler
{
	public:
	
	ContactHandler( );
	
 
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
	
		
	private:
		

	int data;
};	


class UserHandler : public XMLElementHandler
{
	public:
	
	UserHandler( );
	
 
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
};


class DayTimeHandler : public XMLElementHandler
{
	public:
	
	DayTimeHandler( );
	 
	virtual int Begin( );
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
};


class CalibrateHandler : public XMLElementHandler
{
	public:
	
	CalibrateHandler( );
	 
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
};


class DeviceHandler : public XMLElementHandler
{
	public:
	
	DeviceHandler( );
	 
	virtual int Begin( );
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );

};


class PanelHandler : public XMLElementHandler
{
	public:
	
	PanelHandler( );
	
	virtual int Begin( );
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
};


class InputHandler : public XMLElementHandler
{
	public:
	
	InputHandler( );
 
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
};


class PanelInputHandler : public InputHandler	
{
	public:
		
	PanelInputHandler( );
};


class OutputHandler : public XMLElementHandler
{
	public:
	
	OutputHandler( );
 
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
};


class PanelOutputHandler : public OutputHandler	
{
	public:
		
	PanelOutputHandler( );
};


class MeshDeviceHandler : public XMLElementHandler
{
	public:
	
	MeshDeviceHandler( );
	
	virtual int Begin( );
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
};


class CAEInputHandler : public XMLElementHandler
{
	public:
	
	CAEInputHandler( );
	
	virtual int Begin( );
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
};


class CAEOutputHandler : public XMLElementHandler
{
	public:
	
	CAEOutputHandler( );
	
	virtual int Begin( );
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
};


class CAERuleHandler : public XMLElementHandler
{
	public:
	
	CAERuleHandler( );
	
	virtual int Begin( );
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
};
  

class ReceivePanelHandler : public XMLElementHandler
{
	public:
	
	ReceivePanelHandler( );
		 
	virtual int Begin( );
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
};


class TransmitPanelHandler : public XMLElementHandler
{
	public:
	
	TransmitPanelHandler( );
		 
	virtual int Begin( );
	
	virtual int Attribute( char* name, char* value );
};


class ParamHandler : public XMLElementHandler
{
	public:
	
	ParamHandler( );
		 
	virtual int Begin( );
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
};


class ZoneHandler : public XMLElementHandler
{
	public:
	
	ZoneHandler( );
		 
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
	
	Zone zone;
};


class ImportXML : public Module 
{
	public:
	
	ImportXML( );
	
	virtual int Init( );
	
	virtual int Receive( Message* );
	virtual int Receive( Event );
	virtual int Receive( Command* cmd );
	
	virtual void Poll( );
	
	int 		Write( int offset, void* data, int size );
   char* 	GetErrMsg( int err );	
	
	int GetPanelIndex( char* name );
 	
	CAEInput			caeinput;
	CAEOutput		caeoutput;
	CAERule 			caerule;
	DeviceConfig   dev;
	
	int				receive_panel_index;
	int				transmit_panel_index;
	int				action_index;
	
	XMLParser		xmlparser;  
		
	NetFilter  		net[ SITE_MAX_PANELS][ SITE_NUM_OUTPUT_ACTIONS  ];

	Site* 			site;
	Panel*			panel;
	
	char 				panel_name[ SITE_MAX_STRING_LEN ];
	char				names[ SITE_MAX_PANELS ][ SITE_MAX_STRING_LEN ];
	

	
	XMLParser*  	xmlp;	
	int 				writeAddr;
	
	Settings*		settings;
	ExportXML*		exporter;
	QuadSPI* 		qspi;
	EEPROM*        eeprom;
	
	char				site_buff[ OFFSET( Site, panels ) ];
	char 				panel_buff[ OFFSET( Panel, devs ) ];
	
};



#endif
