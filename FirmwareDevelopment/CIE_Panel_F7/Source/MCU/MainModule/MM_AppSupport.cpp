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

/* System Include Files
**************************************************************************/
 


/* User Include Files
**************************************************************************/
 
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_NCU.h"
#include "MM_Events.h"
#include "MM_fault.h"
#include "MM_device.h"
#include "MM_error.h"
#include "CO_Site.h"
#include "CO_Device.h"
#include "MM_AppSupport.h"
#include "MM_PanelNetwork.h"
#include "MM_GSM.h"
#include "MM_Utilities.h"
#include "MM_CUtils.h"
#include "JSON.h"
#include <time.h>
#include <string.h>


/* GDefines
**************************************************************************/


#define DEST_DOMAIN 	"cygnus-systemsappserver.com"
#define DEST_URL		"cygnus-systemsappserver.com"
 

#define APP_SUPP_MAX_TRYS 60


/* Globals
**************************************************************************/

char AppSupport::token[ 850 ] = { 0 };
char AppSupport::reftoken[ 100 ] = { 0 };

short AppSupport::enabled = 1;		// On at the start by default..
bool AppSupport::ready = true;
bool AppSupport::re_auth = false;
bool AppSupport::cleared = false;
time_t AppSupport::tokenreceived = 0;

uint64_t 		AppSupport::refreshtokenexpiry;
uint64_t 		AppSupport::tokenexpiry;	
JSON 				AppSupport::json;
TokenHandler 	AppSupport::tokenhandler;	

PanelNetwork* 		AppSupport::network;

IOStatus* 			AppSupport::iostatus = NULL;

char AppSupport::queuebuff[ sizeof( AppMessage ) * APP_SUPPORT_QUEUE_SIZE ];
Circular<AppMessage> AppSupport::outqueue( (AppMessage*) queuebuff, APP_SUPPORT_QUEUE_SIZE );

char AppSupport::eventqueuebuff[ sizeof( AppEventMessage ) * APP_SUPPORT_EVENT_QUEUE_SIZE ];
Circular<AppEventMessage> AppSupport::outeventqueue( (AppEventMessage*) eventqueuebuff, APP_SUPPORT_EVENT_QUEUE_SIZE );


AppMessage::AppMessage( unsigned int unit, bool faults, bool warnings, bool present )
{
	DeviceUpdate* du = &this->deviceupdate;
	
	du->type 	 = APP_INFO_DEVICE_UPDATE;
	du->unit 	 = unit;
	du->faults	 = faults;
	du->warnings = warnings;
	du->present	 = present;
}
	

AppMessage::AppMessage( AppSupportEvent e )
{
	PanelEvent* pe = &this->panelevent;
	
	pe->type 	 = APP_INFO_EVENT;
   pe->event    = e;
}

	
AppMessage::AppMessage( unsigned int unit, FaultType fault, unsigned int chan, bool status )
{
	DeviceFault* df = &this->devicefault;
	
	df->type = APP_INFO_DEVICE_FAULT;
	df->unit = unit;
	df->chan = chan;
	df->fault = fault;
	df->status = status;
}


AppMessage::AppMessage( FaultType fault, bool status )
{
	PanelFault* pf = &this->panelfault;
	
	pf->type 	= APP_INFO_PANEL_FAULT;
	pf->fault	= fault;
	pf->status	= status;	
}


inline uint32_t AppEventMessage::ZoneGroup( int i )
{
	return ((int*)zonebits)[ i ];
}

	
 
AppEventMessage::AppEventMessage( unsigned int isClear, unsigned int panel, unsigned int event, uint32_t* bits ) : event( event), isClear( isClear ), panel( panel )
{
	((int*)zonebits)[ 0] = bits[0];
	((int*)zonebits)[ 1] = bits[1];
	((int*)zonebits)[ 2] = bits[2];
}



char* TokenHandler::ReadTo( char* id )
{
	if ( !strcmp( id, "token" ) )
	{
		return AppSupport::token;
	}
	else if ( !strcmp( id, "refreshToken" ) )
	{
		return AppSupport::reftoken;
	}
	else if ( !strcmp( id, "tokenDuration" ) )
	{
		return (char*) &AppSupport::tokenexpiry;
	}
	else if ( !strcmp( id, "refreshTokenDuration" ) )
	{
		return (char*) &AppSupport::refreshtokenexpiry;
	}	
	return NULL;
}
 

/*************************************************************************/
/**  \fn      AppSupport::AppSupport( )
***  \brief   Constructor for class
**************************************************************************/

AppSupport::AppSupport( ) : Module( "AppSupport", 100, EVENT_RESET ), http( DEST_DOMAIN, DEST_URL, NULL ), trys( 0 ) 
{
	token[ 0 ] = 0;
	reftoken[ 0 ] = 0;
	
	re_auth = false;
  
	if ( !EEPROM::GetVar( AppSupp_Enabled, enabled ) )
	{
		if ( !EEPROM::GetVar( AppSupp_Enabled, enabled ) )
		{
			Fault::AddFault( FAULT_EEPROM_COMMS );
			//Log::Msg(LOG_SYS, "01_AppSupport.cpp -- ADD");
		}
	}
	
	if ( !EEPROM::GetVar( AppSupp_SiteId, *site_id ) )
	{
		if ( !EEPROM::GetVar( AppSupp_Enabled, *site_id  ) )
		{
			Fault::AddFault( FAULT_EEPROM_COMMS );
			//Log::Msg(LOG_SYS, "02_AppSupport.cpp -- ADD");
		}
	}
	site_id[ 39 ]= 0;
	if ( !EEPROM::GetVar( AppSupp_Password, *password ) )
	{
		if ( !EEPROM::GetVar( AppSupp_Password, *password  ) )
		{
			Fault::AddFault( FAULT_EEPROM_COMMS );
			//Log::Msg(LOG_SYS, "03_AppSupport.cpp -- ADD");
		}
	}
	password[ 19] = 0;
	
	if ( !EEPROM::GetVar( AppSupp_URL, *url ) )
	{
		if ( !EEPROM::GetVar( AppSupp_URL, *url  ) )
		{
			Fault::AddFault( FAULT_EEPROM_COMMS );
			//Log::Msg(LOG_SYS, "04_AppSupport.cpp -- ADD");
			return;
		}
	}
	memset(url,0,60);
	url[59] = 0;
	
	http.dest_domain = url;
	http.dest_url = url;	
}


/*************************************************************************/
/**  \fn      int AppSupport::Init( )
***  \brief   Class function
**************************************************************************/

int AppSupport::Init( )
{
	ncu = (NCU*) app.FindModule( "NCU" );
	settings = (Settings*) app.FindModule( "Settings" );
	iostatus = (IOStatus*) app.FindModule( "IOStatus" );
	network  =  (PanelNetwork*) app.FindModule( "PanelNetwork" );
	
	cleared = false;
	
	if ( enabled )
	{
		http.Create( );
	}
	
	return true;
}

  
/*************************************************************************/
/**  \fn      int AppSupport::Receive( Message* m )
***  \brief   Class function
**************************************************************************/

int AppSupport::Receive( Message* )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int AppSupport::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int AppSupport::Receive( Command* cmd )
{
	if ( cmd->type == CMD_SET_APP_SUPP )
	{
		enabled = cmd->short0;
			 
		if ( !EEPROM::SetVar( AppSupp_Enabled, enabled ) )
		{
			if ( !EEPROM::SetVar( AppSupp_Enabled, enabled ) )
			{
				Fault::AddFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "05_AppSupport.cpp -- ADD");
			}
			else
			{
				Fault::RemoveFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "06_AppSupport.cpp -- REMOVE");
			}
		}
		else
		{
			Fault::RemoveFault( FAULT_EEPROM_COMMS );
			//Log::Msg(LOG_SYS, "07_AppSupport.cpp -- REMOVE");
		}
		
		return CMD_OK;
	}
	else if ( cmd->type == CMD_GET_APP_SUPP )
	{
		cmd->short0 = enabled;

		 		
		return CMD_OK;
	}	
	else if ( cmd->type == CMD_GET_APP_SETUP )
	{
		cmd->int0 = (int) site_id;
		cmd->int1 = (int) password;
		cmd->int2 = (int) GSM::apn;
		
		return CMD_OK;
	}
	else if ( cmd->type == CMD_SETUP_APP )
	{
		strncpy( site_id,  (char*) cmd->int0, sizeof(site_id) );
		strncpy( password, (char*) cmd->int1, sizeof(password) );
		strncpy( GSM::apn, (char*) cmd->int2, sizeof(GSM::apn) );
		
		if ( !EEPROM::SetVar( AppSupp_SiteId, *site_id ) )
		{
			if ( !EEPROM::SetVar( AppSupp_SiteId, *site_id ) )
			{
				Fault::AddFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "08_AppSupport.cpp -- ADD");
			}
			else
			{
				Fault::RemoveFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "09_AppSupport.cpp -- REMOVE");
			}
		}
		else
		{
			Fault::RemoveFault( FAULT_EEPROM_COMMS );			
			//Log::Msg(LOG_SYS, "10_AppSupport.cpp -- REMOVE");
		}
		if ( !EEPROM::SetVar( AppSupp_Password, *password ) )
		{
			if ( !EEPROM::SetVar( AppSupp_Password, *password ) )
			{
				Fault::AddFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "11_AppSupport.cpp -- ADD");
			}
			else
			{
				Fault::RemoveFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "12_AppSupport.cpp -- REMOVE");
			}
		}
		else
		{
			Fault::RemoveFault( FAULT_EEPROM_COMMS );			
			//Log::Msg(LOG_SYS, "13_AppSupport.cpp -- REMOVE");
		}
		
		if ( !EEPROM::SetVar( AppSupp_APN, *GSM::apn ) )
		{
			if ( !EEPROM::SetVar( AppSupp_APN, *GSM::apn ) )
			{
				Fault::AddFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "14_AppSupport.cpp -- ADD");
			}
			else
			{
				Fault::RemoveFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "15_AppSupport.cpp -- REMOVE");
			}
		}
		else
		{
			Fault::RemoveFault( FAULT_EEPROM_COMMS );
			//Log::Msg(LOG_SYS, "16_AppSupport.cpp -- REMOVE");
		}
		
		return CMD_OK;
	}	
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int AppSupport::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int AppSupport::Receive( Event e )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int AppSupport::Send( AppSupportInfo info, int id, int parent )
***  \brief   Class function
**************************************************************************/

	
int AppSupport::Send( AppMessage& msg )
{	 
	if ( enabled && ! outqueue.Full( ) )
	{	 
		outqueue.Push( msg );
		
		return true;
	}
	return false;
}
	

static AppMessage* msg = NULL;
 
static uint32_t prev_zg[ ACTION_SHIFT_FAULT][ SITE_MAX_PANELS][ SITE_MAX_ZONES / sizeof(uint32_t) ];


/*************************************************************************/
/**  \fn      int AppSupport::Receive( Event e )
***  \brief   Class function
**************************************************************************/

static int CountZones( uint32_t* zonebits )
{
	int count = 0;
	
	for( int i = 0; i < 3; i++ )
	{
		for( int b  = 0 ; b < 32; b++ )
		{
			if ( ( zonebits[ i] >> b )& 1 ) 
			{
				count++;
			}
		}
	}
	return count;
}


/*************************************************************************/
/**  \fn      int AppSupport::Receive( Event e )
***  \brief   Class function
**************************************************************************/

void AppSupport::EventMsgReceived( int code, char* data, int length )
{
	if ( code == 0 )
	{
		// timeout / fail
	}
	if ( code == 401 )
	{
		token[0] = 0;		// token expired
	}
	else if ( code == 200 )
	{
		outeventqueue.Pop( );
		
		char* req = strstr( data, "request:" );
				
		if ( req != NULL )
		{
			if ( strstr( req + 8, "silence" ) )
			{
				app.Send( EVENT_SILENCE_BUTTON );
			}
			else if ( strstr( req + 8, "reset" ) )
			{
				app.Send( EVENT_RESET_BUTTON );
			}
			else if ( strstr( req + 8, "resound" ) )
			{
				app.Send( EVENT_RESOUND_BUTTON );
			}
		}
	}	
	else	
	{
		//Log::Msg( LOG_SYS, "HTTP code %d received.", code );
	}
}
 
 
					
/*************************************************************************/
/**  \fn      void AppSupport::CheckEventStatus( )
***  \brief   Class function
**************************************************************************/

void AppSupport::CheckEventStatus( )
{						
	// Check events first
	
	for( int event = ACTION_SHIFT_FIRE; event <= ACTION_SHIFT_GENERAL; event++ )
	{
		for( int panel = 0; panel < app.site->numPanels; panel++ )
		{
			uint32_t zg[ SITE_MAX_ZONES / sizeof(uint32_t) ];
			
			for( int zonegroup = 0; zonegroup < 3; zonegroup++ )
			{
				zg[ zonegroup ] = GetZoneGroup( event, panel, zonegroup );
			}
			
			// if event diff to prev. )
			if ( ( zg[ 0] ^ prev_zg[ event][ panel][ 0] ) |
			     ( zg[ 1] ^ prev_zg[ event][ panel][ 1] ) |
			     ( zg[ 2] ^ prev_zg[ event][ panel][ 2] ) )
			{	
				int newnumzones = CountZones( zg );
					
				int qsize = outeventqueue.items;
				
				// if only 1 (or none)
				if ( qsize == 0 || qsize == 1 )
				{
					AppEventMessage e( !newnumzones, panel, event, zg );
					
					// Just push
					outeventqueue.Push( e );
				}
				else // qsize >= 2
				{
					// Try to optimize..
					
					// for each item in queue (except first)
					for( int i = qsize - 1; i > 0; i-- )
					{
						AppEventMessage* p = &outeventqueue[ i ];    
						
						// if panel/event matches
						if ( p->event == event && p->panel == panel )
						{
							// if last event was clear
							if ( p->isClear )
							{
								// but new zones in fire..
								if ( newnumzones )
								{
									// We cant optimize
									AppEventMessage e( !newnumzones, panel, event, zg );
							
									// Just push new
									outeventqueue.Push( e );
								}
								else
								{
									// if last clear and new clear do nowt..
								}
							}
							else  // ( if last event not clear ) 
							{
								// and if current not clear
								if ( newnumzones )
								{
									// just update to new zone status
									AppEventMessage e( !newnumzones, panel, event, zg );
									
									*p = e;
								}
								else // if new event is clear
								{
									// cant optimize
									AppEventMessage e( !newnumzones, panel, event, zg );
									
									// Just push new
									outeventqueue.Push( e );
								}
							}
						}
					}
				}
			}
			prev_zg[ event][ panel][ 0] = zg[ 0];
			prev_zg[ event][ panel][ 1] = zg[ 1];
			prev_zg[ event][ panel][ 2] = zg[ 2];
		}
	}
}
				

/*************************************************************************/
/**  \fn      void AppSupport::MsgReceived( int code, char* data, int length )
***  \brief   Class function
**************************************************************************/

void AppSupport::MsgReceived( int code, char* data, int length )
{
	if ( code == 0 )
	{
		// timeout / fail
	}
	if ( code == 401 )
	{
		token[0] = 0;		// token expired
	}
	else 
	{
		if ( code == 200 )
		{
			if ( msg != NULL )
			{
				outqueue.Pop( );
			}
			
			char* req = strstr( data, "request:" );
					
			if ( req != NULL )
			{
				if ( strstr( req + 8, "silence" ) )
				{
					app.Send( EVENT_SILENCE_BUTTON );
				}
				else if ( strstr( req + 8, "reset" ) )
				{
					app.Send( EVENT_RESET_BUTTON );
				}
				else if ( strstr( req + 8, "resound" ) )
				{
					app.Send( EVENT_RESOUND_BUTTON );
				}
			}
		}	
	}		
}



/*************************************************************************/
/**  \fn      int AppSupport::PingMsgReceived( int code, char* data, int length )
***  \brief   Class function
**************************************************************************/

void AppSupport::PingMsgReceived( int code, char* data, int length )
{

}


/*************************************************************************/
/**  \fn      void ReceiveToken( char* token, int length )
***  \brief   Class function
**************************************************************************/
	

void AppSupport::TokenReceived( int code, char* data, int length )
{
	if ( code == 200 )
	{
		cleared = true;
		if ( data != NULL && length > 0 )
		{
			if ( json.Decode( data, length, &AppSupport::tokenhandler ) == JSON_DONE )
			{
				// all good
				tokenreceived = now( );
			}
		}
	}	 
	else if ( code == 401 )
	{
		Fault::AddFault( FAULT_BAD_CREDENTIALS );
	}
}



/*************************************************************************/
/**  \fn      void AppSupport::ReceiveAuth( char* token, int length )
***  \brief   Class function
**************************************************************************/
	

void AppSupport::ReAuthReceived( int code, char* data, int length )
{
	if ( code == 200 )
	{
		if ( data != NULL && length > 0 )
		{
			if ( json.Decode( data, length, &AppSupport::tokenhandler ) == JSON_DONE )
			{
				// all good
				tokenreceived = now( );
			}
		}
	}	 	 
	else if ( code == 401 )
	{
		*reftoken = 0;  // refreshtoken no good - kill it and re-logon		
	}
}


/*************************************************************************/
/**  \fn      uint32_t AppSupport::GetZoneGroup( ActionEnum event, int panel, int group )
***  \brief   Class function
**************************************************************************/

uint32_t AppSupport::GetZoneGroup( int event, int panel, int group )
{
	if ( panel == app.site->currentIndex )
	{
		return iostatus->zonebits[ event ][ group ];
	}
	else
	{
		if ( panel < SITE_MAX_PANELS )
		{
			return network->current_dispzones[ panel ][ event ][ group ] | network->current_processzones[ panel ][ event ][ group ];
		}
	}
	return 0;	
}


/*************************************************************************/
/**  \fn      virtual void AppSupport::Poll( )
***  \brief   Class function
**************************************************************************/

void AppSupport::Poll( )
{
	if ( enabled && app.panel != NULL && app.site != NULL  )
	{
		if ( tokenexpiry  < now() - tokenreceived + 60 )
		{
				*token = 0;		// expire token
		}
		if ( GSM::state == GSM_IDLE )
		{
			msg = NULL;
			
			// if no token;
			if ( !*token )
			{
				// if no refreshToken
				if ( !*reftoken )
				{
					// full logon
					if ( http.Post( "api/panelauthenticate/login", NULL, TokenReceived, json.Make( JSON_STR, JSON_STR, JSON_STR, JSON_NUM, JSON_BOOL,
											"SiteIdentifer", site_id, 
											"SitePassKey", password,
											"PanelUID", Utilities::UID,
											"MaxDevices", app.panel->numRadioDevs,
											"Restart", !cleared ) 
						  ) ) 
					{
						if ( trys++ == APP_SUPP_MAX_TRYS )
						{
							Fault::AddFault( FAULT_UNABLE_TO_CONNECT_TO_SERVER );
							priority = 5000;
							trys = 0;
						}
					}
				}
				else  // use refresh token
				{		 
					http.Post( "api/panelauthenticate/refresh", NULL, ReAuthReceived, json.Make( JSON_STR, JSON_STR,
								"PanelUID", Utilities::UID,
								"RefreshToken", reftoken ) );
				}
			}
			else 
			{
				// Reset logon trys
				trys = 0;
				 
				// check if connect failed previously
				if ( priority > 100 )
				{
					Fault::RemoveFault( FAULT_UNABLE_TO_CONNECT_TO_SERVER );
				
					priority = 100;
				}
				
				bool posted = false;
				
				// Check events queue first
				
				if ( !outeventqueue.Empty( ) ) 
				{		
					// peek top of queue
					AppEventMessage* eventmsg = outeventqueue.Peek( );
							
					// if new msg
					if ( eventmsg != NULL )
					{		
						if ( eventmsg->isClear )
						{
							http.Post( "api/panelauthenticate/event", token, EventMsgReceived, json.Make( JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_STR, JSON_NUM, 
												"Action", eventmsg->event,
												"Zone1_32", 0,
												"Zone33_64", 0,
												"Zone65_96", 0,
												"PanelName", app.site->panels[ eventmsg->panel ].name,
												"PanelAddress", eventmsg->panel ) );
							posted = true;
							
						}
						else // in fire/event!
						{
							// if this panel
							if ( eventmsg->panel == app.site->currentIndex )
							{
								// Find devices in fire/event 
								
								short dev_indexs[ 5 ];
								int numdevs = iostatus->GetNumAlerts( 1 << eventmsg->event, -1, app.site->currentIndex, MASK_CAE | MASK_NET );	// devs only
								
								if ( numdevs > 5 ) numdevs = 5;
								
								int count = 0;
								
								// create array of units
								
								for( int i = 0; i < numdevs; i++ )
								{
									Alert* alert = iostatus->GetNthAlert( i, 1 << eventmsg->event, -1, app.site->currentIndex, MASK_CAE | MASK_NET );
									
									if ( alert->flags != ALERT_NONE )
									{
										dev_indexs[ count++ ] = alert->unit;
									}
								}
								
								http.Post( "api/panelauthenticate/event", token, EventMsgReceived, json.Make( JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_STR, JSON_NUM, JSON_SHORT_ARRAY,
												"Action", eventmsg->event,
												"Zone1_32",  eventmsg->ZoneGroup( 0 ),
												"Zone33_64", eventmsg->ZoneGroup( 1 ),
												"Zone65_96", eventmsg->ZoneGroup( 2 ),
												"PanelName", app.site->panels[ eventmsg->panel ].name,
												"PanelAddress", eventmsg->panel,
												"Units", dev_indexs,	count	) );
								posted = true;
							}
							else // is nother panel
							{
								// post without device list
								
								http.Post( "api/panelauthenticate/event", token, EventMsgReceived, json.Make( JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_STR, JSON_NUM, 
												"Action", eventmsg->event,
												"Zone1_32",  eventmsg->ZoneGroup( 0 ),
												"Zone33_64", eventmsg->ZoneGroup( 1 ),
												"Zone65_96", eventmsg->ZoneGroup( 2 ),
												"PanelName", app.site->panels[ eventmsg->panel ].name,
												"PanelAddress", eventmsg->panel
												 ) );
								posted = true;
							}
						}
					}
				}
				
				if ( posted ) return;
				
				// Check queue
				 
				if ( !outqueue.Empty( ) ) 
				{		
					// peek top of queue
					msg = outqueue.Peek( );
							
					// if new msg
					if ( msg != NULL )
					{				
						switch( msg->type )
						{ 
							case APP_INFO_EVENT :
							{
								PanelEvent* e = &msg->panelevent;
								
								 http.Post( "api/panelauthenticate/event", token, MsgReceived, json.Make( JSON_NUM,
												"Action", e->event ) );	 
								break;
							}
							
							case APP_INFO_DEVICE_UPDATE :
							{
								DeviceUpdate* du = &msg->deviceupdate;
								
								RadioUnit* ru = ncu->FindUnit( du->unit );
								DeviceConfig* dc = settings->FindConfig( du->unit );
								
								if ( ru == NULL || !du->present )
								{
									http.Post( "api/panelauthenticate/devicedelete", token, MsgReceived, json.Make( JSON_NUM, "Unit", du->unit ) );
								}						
								else if ( dc != NULL )
								{
									const char* location = dc->location;
										
									if ( location[ 0] == 0 ) location = "Location undefined";
									
									char* zonename = NULL;
									
									for( Zone* z = app.panel->zone; z < app.panel->zone + app.panel->numDefinedZones; z++ )
									{
										if ( z->num == dc->zone )
										{
											zonename = z->name;
										}
									}
									if ( 1 )//zonename == NULL )
									{
										static char znamebuff[ 12];
										sprintf( znamebuff, "%d", dc->zone );
										zonename = znamebuff;
									}
									
									http.Post( "api/panelauthenticate/device", token, MsgReceived, json.Make( JSON_NUM, JSON_STR, JSON_STR, JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_BOOL, JSON_BOOL,
													"Unit", du->unit,
													"Zone", zonename,
													"Location", location,
													"RSSI", ru->rssiprimary,
													"TypeCode", dc->type,
													"Parent", ru->primary,
													"Battery", ru->battery,
													"BatteryVoltage", ru->battprimary,
													"Faults", du->faults,
													"Warnings", du->warnings ) );
									
								}	
								else
								{
									outqueue.Pop( );
								}
									
								break;
							}
							 
							case APP_INFO_PANEL_FAULT:
							{
								PanelFault* pf = &msg->panelfault;
								
								http.Post( "api/panelauthenticate/panelfault", token, MsgReceived, json.Make( JSON_NUM, JSON_BOOL, "Fault", pf->fault, "Status", !pf->status ) );
				
								break;
							}
							
							case APP_INFO_DEVICE_FAULT :
							{
								bool iswarning = false;
								
								DeviceFault* df = &msg->devicefault;
								
								if ( df->fault == FAULT_SIGNAL_BATTERY_ERROR_E ||
									  df->fault == FAULT_SIGNAL_LOW_LINK_QUALITY_E ||
									  df->fault == FAULT_SIGNAL_LOW_PARENT_COUNT_FAULT_E )
								{
									iswarning = true;
								}
							
								http.Post( "api/panelauthenticate/devicefault", token, MsgReceived, json.Make( JSON_NUM, JSON_NUM, JSON_BOOL, JSON_NUM, JSON_BOOL, "Unit", df->unit, "Fault", df->fault, "IsWarning", iswarning, "Feature", df->chan, "Status", !df->status ) );
								
								break;
							}
						}
					}
				}
				else
				{
					time_t n = now( );
					
					if ( n - http.lastrequest > 60 * 5 )
					{
						http.Post( "api/panelauthenticate/ping", token, PingMsgReceived, "{}" );
					}
				}	
			}
		}
	}
}

				
					
			
		
