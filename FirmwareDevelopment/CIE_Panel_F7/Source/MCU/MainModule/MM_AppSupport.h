/***************************************************************************
* File name: MM_AppSupport.cpp
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
* AppSupport Module
*
**************************************************************************/

#ifndef __APP_SUPPORT__
#define __APP_SUPPORT__


/* System Include Files
**************************************************************************/
 
#include <stdarg.h>

/* User Include Files
**************************************************************************/

#include "MM_list.h"
#include "MM_HTTP.h"
#include "MM_Settings.h"
#include "JSON.h"
#include "MM_Fault.h"


/* Defines
**************************************************************************/

#define APP_SUPPORT_CURRENT_PANEL 	7
#define APP_SUPPORT_QUEUE_SIZE 		1500
#define APP_SUPPORT_EVENT_QUEUE_SIZE 32

class PanelNetwork;

enum AppSupportEvent
{
	APP_EVENT_RESET_EVENTS = 10,
	APP_EVENT_RESET_FAULTS = 11,
	APP_EVENT_RESET_ALL   = 12,
};

enum AppSupportInfo
{
	APP_INFO_DEVICE_UPDATE,
	APP_INFO_PANEL_FAULT,
	APP_INFO_DEVICE_FAULT,
	APP_INFO_EVENT,
};

enum AppSupportRequest
{
	APP_REQ_SILENCE,
	APP_REQ_RE_SOUND,
	APP_REQ_CONFIRM,
	APP_REQ_ACKNOWLEDGE,
	APP_REQ_RESET,
	APP_REQ_DEVICE_STATUS,
	APP_REQ_FAULT_STATUS,
	APP_REQ_EVENT_STATUS,
	APP_REQ_ALL_DEVICES,
	APP_REQ_ALL_FAULTS,
	APP_REQ_ALL_EVENTS,
};


typedef struct
{
	unsigned int type  : 3;
	unsigned int unit  : 10;
	unsigned int fault : 5;
	unsigned int chan  : 7;
	unsigned int status: 1;
} DeviceFault;

typedef struct
{
	unsigned int type  : 3;
	unsigned int fault : 8;
	unsigned int status: 1;
} PanelFault;

typedef struct
{
	unsigned int type  : 3;
	unsigned int event : 4;
} PanelEvent;

typedef struct
{
	unsigned int type  	: 3;
	unsigned int unit  	: 10;
	unsigned int faults	: 1;
	unsigned int warnings: 1;
	unsigned int present : 1;	
} DeviceUpdate;

 
union AppMessage
{
	public:
		
	AppMessage( unsigned int unit, bool faults, bool warnings, bool present );
	AppMessage( unsigned int unit, FaultType fault, unsigned int chan, bool status );
	AppMessage( FaultType fault, bool status );
	AppMessage( AppSupportEvent e );
			 	 
	unsigned int type : 3;
	DeviceFault devicefault;
	PanelFault panelfault;
	DeviceUpdate deviceupdate;		 
	PanelEvent panelevent;
};
 
 
class AppEventMessage
{
	public:
		
	char zonebits[ 96/8 ];		 	 
	unsigned int event : 4;
	unsigned int panel : 3;
	unsigned int isClear : 1;
	
	AppEventMessage( unsigned int isClear, unsigned int panel, unsigned int event, uint32_t* bits );
	
	
	uint32_t ZoneGroup( int i );
	 
};


const char* ActionName( uint8_t a );
	

class TokenHandler : public JSONHandler
{
	virtual char* ReadTo( char* id );
};

	
class AppSupport : public Module
{
	public:
	
	AppSupport( );
	static int Send( AppMessage& ); 
	
	virtual int Init( );
	static uint32_t GetZoneGroup( int event, int panel, int group );
	
	static void MsgReceived( int code, char* data, int length );
	static void TokenReceived( int code, char* data, int length );
	static void ReAuthReceived( int code, char* data, int length );
	static void PingMsgReceived( int code, char* data, int length );
	static void EventMsgReceived( int code, char* data, int length );
	
	static void CheckEventStatus( );
	
	virtual void Poll( );
	virtual int Receive( Command* );
	virtual int Receive( Event );
	virtual int Receive( Message* ); 
	
	static char token[ 850 ];
	static char reftoken[ 100 ];
	static bool re_auth;
	
	static uint64_t refreshtokenexpiry;
	static uint64_t tokenexpiry;
	static time_t tokenreceived;
	
	static JSON json;
	static TokenHandler tokenhandler;	
	static short enabled;

	private:
		
	HTTPSocket http;
	
	static bool cleared;
	
	char site_id[ 40];
	char password[ 20];
	char url[ 60];
	
	static bool ready;
	int trys;
	
	NCU* 					ncu;
	Settings* 			settings;
	static IOStatus* 			iostatus;
	static PanelNetwork* 		network;
	
	static char queuebuff[ sizeof( AppMessage ) * APP_SUPPORT_QUEUE_SIZE ];
	static char eventqueuebuff[ sizeof( AppEventMessage ) * APP_SUPPORT_EVENT_QUEUE_SIZE ];
	
	static Circular<AppMessage> outqueue;
	
	static Circular<AppEventMessage> outeventqueue;
};	
	


#endif

