

/***************************************************************************
* File name: MM_MQTT.cpp
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
* MQTT Module
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
#include "MM_MQTT.h"
#include "MM_PanelNetwork.h"
#include "MM_GSM.h"
#include "MM_Utilities.h"
#include "MM_CUtils.h"
#include "JSON.h"
#include <time.h>



/* GDefines
**************************************************************************/


#define DEST_DOMAIN 	"cygnus-systemsappserver.com"
#define DEST_URL		"cygnus-systemsappserver.com"
 

#define APP_SUPP_MAX_TRYS 60


/* Globals
**************************************************************************/

char MQTT::token[ 850 ] = { 0 };
char MQTT::reftoken[ 100 ] = { 0 };

short MQTT::enabled = 1;		// On at the start by default..
bool MQTT::ready = true;
bool MQTT::re_auth = false;
bool MQTT::cleared = false;
time_t MQTT::tokenreceived = 0;

uint64_t 		MQTT::refreshtokenexpiry;
uint64_t 		MQTT::tokenexpiry;	
JSON 				MQTT::json;
TokenHandler 	MQTT::tokenhandler;	
 
 
/*************************************************************************/
/**  \fn      MQTT::MQTT( )
***  \brief   Constructor for class
**************************************************************************/

MQTTTest::MQTT( ) : Module( "MQTT", 100, EVENT_RESET ), mqtt( DEST_DOMAIN, DEST_URL, NULL ), trys( 0 ) 
{
	token[ 0 ] = 0;
	reftoken[ 0 ] = 0;
	
	re_auth = false;
  
	if ( !EEPROM::GetVar( AppSupp_Enabled, enabled ) )
	{
		if ( !EEPROM::GetVar( AppSupp_Enabled, enabled ) )
		{
			Fault::AddFault( FAULT_EEPROM_COMMS );
			//Log::Msg(LOG_SYS, "01_MQTT.cpp -- ADD");
		}
	}
	
	if ( !EEPROM::GetVar( AppSupp_SiteId, *site_id ) )
	{
		if ( !EEPROM::GetVar( AppSupp_Enabled, *site_id  ) )
		{
			Fault::AddFault( FAULT_EEPROM_COMMS );
			//Log::Msg(LOG_SYS, "02_MQTT.cpp -- ADD");
		}
	}
	else 
	{
		EEPROM::RemoveFault( FAULT_EEPROM_COMMS );
		//Log::Msg(LOG_SYS, "03_MQTT.cpp -- REMOVE");
	}
		
	site_id[ 39 ]= 0;
	
	
	if ( !EEPROM::GetVar( AppSupp_Password, *password ) )
	{
		if ( !EEPROM::GetVar( AppSupp_Password, *password  ) )
		{
			Fault::AddFault( FAULT_EEPROM_COMMS );
			//Log::Msg(LOG_SYS, "04_MQTT.cpp -- ADD");
		}
	}
	else 
	{
		EEPROM::RemoveFault( FAULT_EEPROM_COMMS );
		//Log::Msg(LOG_SYS, "05_MQTT.cpp -- REMOVE");
	}
	
	password[ 19] = 0;
}


/*************************************************************************/
/**  \fn      int MQTT::Init( )
***  \brief   Class function
**************************************************************************/

int MQTT::Init( )
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
/**  \fn      int MQTT::Receive( Message* m )
***  \brief   Class function
**************************************************************************/

int MQTT::Receive( Message* )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int MQTT::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int MQTT::Receive( Command* cmd )
{
	if ( cmd->type == CMD_SET_APP_SUPP )
	{
		enabled = cmd->short0;
				
		if ( enabled && !http.created ) http.Create( );
		
		if ( !EEPROM::SetVar( AppSupp_Enabled, enabled ) )
		{
			if ( !EEPROM::SetVar( AppSupp_Enabled, enabled ) )
			{
				Fault::AddFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "06_MQTT.cpp -- ADD");
			}
			else
			{
				Fault::RemoveFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "07_MQTT.cpp -- REMOVE");
			}
		}
		else
		{
			Fault::RemoveFault( FAULT_EEPROM_COMMS );
			//Log::Msg(LOG_SYS, "08_MQTT.cpp -- REMOVE");
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
				//Log::Msg(LOG_SYS, "09_MQTT.cpp -- ADD");
			}
			else
			{
				Fault::RemoveFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "10_MQTT.cpp -- REMOVE");
			}
		}
		else
		{
			Fault::RemoveFault( FAULT_EEPROM_COMMS );
			//Log::Msg(LOG_SYS, "11_MQTT.cpp -- REMOVE");
		}
		if ( !EEPROM::SetVar( AppSupp_Password, *password ) )
		{
			if ( !EEPROM::SetVar( AppSupp_Password, *password ) )
			{
				Fault::AddFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "12_MQTT.cpp -- ADD");
			}
			else
			{
				Fault::RemoveFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "13_MQTT.cpp -- REMOVE");
			}
		}
		else
		{
			Fault::RemoveFault( FAULT_EEPROM_COMMS );
			//Log::Msg(LOG_SYS, "14_MQTT.cpp -- REMOVE");
		}
		
		if ( !EEPROM::SetVar( AppSupp_APN, *GSM::apn ) )
		{
			if ( !EEPROM::SetVar( AppSupp_APN, *GSM::apn ) )
			{
				Fault::AddFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "15_MQTT.cpp -- ADD");
			}
			else
			{
				Fault::RemoveFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "16_MQTT.cpp -- REMOVE");
			}
		}
		else
		{
			Fault::RemoveFault( FAULT_EEPROM_COMMS );
			//Log::Msg(LOG_SYS, "17_MQTT.cpp -- REMOVE");
		}
		
		return CMD_OK;
	}	
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int MQTT::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int MQTT::Receive( Event e )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int MQTT::Send( MQTTInfo info, int id, int parent )
***  \brief   Class function
**************************************************************************/

	
int MQTT::Send( AppMessage& msg )
{	 
	if ( enabled && ! outqueue.Full( ) )
	{	 
		outqueue.Push( msg );
		
		return true;
	}
	return false;
}
	

static AppMessage* msg = NULL;


/*************************************************************************/
/**  \fn      int MQTT::Receive( Event e )
***  \brief   Class function
**************************************************************************/

void MQTT::MsgReceived( int code, char* data, int length )
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
/**  \fn      void MQTT::ReceiveAuth( char* token, int length )
***  \brief   Class function
**************************************************************************/
	

void MQTT::ReAuthReceived( int code, char* data, int length )
{
	if ( code == 200 )
	{
		if ( data != NULL && length > 0 )
		{
			if ( json.Decode( data, length, &MQTT::tokenhandler ) == JSON_DONE )
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
/**  \fn      uint32_t MQTT::GetZoneGroup( ActionEnum event, int panel, int group )
***  \brief   Class function
**************************************************************************/

uint32_t MQTT::GetZoneGroup( int event, int panel, int group )
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
/**  \fn      virtual void MQTT::Poll( )
***  \brief   Class function
**************************************************************************/

void MQTT::Poll( )
{
	char outbuff[ 400 ];
	static uint32_t evz[ ACTION_SHIFT_FAULT][ SITE_MAX_PANELS][ SITE_MAX_ZONES / sizeof(uint32_t) ]; 
	
	if ( enabled && app.panel != NULL && app.site != NULL && http.created )
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
					if ( http.Post( "ws/api/panelauthenticate/login", NULL, TokenReceived, json.Make( JSON_STR, JSON_STR, JSON_STR, JSON_NUM, JSON_BOOL,
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
					http.Post( "ws/api/panelauthenticate/refresh", NULL, ReAuthReceived, json.Make( JSON_STR, JSON_STR,
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
				
				for( int event = ACTION_SHIFT_FIRE; event <= ACTION_SHIFT_GENERAL; event++ )
				{
					for( int panel = 0; panel < app.site->numPanels; panel++ )
					{
						uint32_t zg[ SITE_MAX_ZONES / sizeof(uint32_t) ];
						
						for( int zonegroup = 0; zonegroup < 3; zonegroup++ )
						{
							zg[ zonegroup ] = GetZoneGroup( event, panel, zonegroup );
						}
						
						// if event all clear ( and not clear prev. )
						if ( ( zg[ 0] | zg[ 1] | zg[ 2] ) == 0 && 
							( evz[ event][ panel][ 0] |
							  evz[ event][ panel][ 1] |
							  evz[ event][ panel][ 2] ) != 0 )
						{
							http.Post( "ws/api/panelauthenticate/event", token, MsgReceived, json.Make( JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_STR, JSON_NUM, 
												"Action", event,
												"Zone1_32", 0,
												"Zone33_64", 0,
												"Zone65_96", 0,
												"PanelName", app.site->panels[ panel ].name,
												"PanelAddress", panel ) );
							posted = true;
							
						}
						else
						{
							uint32_t newbits = ( ( zg[ 0] ^ evz[ event][ panel][ 0]) & zg[ 0] ) |
													 ( ( zg[ 1] ^ evz[ event][ panel][ 2]) & zg[ 1] ) |
													 ( ( zg[ 2] ^ evz[ event][ panel][ 3]) & zg[ 2] );
							
							
							if ( newbits )
							{
								short dev_indexs[ 512 ];
								int numdevs = iostatus->GetNumAlerts( 1 << event, -1, app.site->currentIndex, MASK_CAE | MASK_NET );	// devs only
								
								if ( numdevs > 10 ) numdevs = 10;
								
								int count = 0;
								
								for( int i = 0; i < numdevs; i++ )
								{
									Alert* alert = iostatus->GetNthAlert( i, 1 << event, -1, app.site->currentIndex, MASK_CAE | MASK_NET );
									
									if ( alert->flags != ALERT_NONE )
									{
										dev_indexs[ count++ ] = alert->unit;
									}
								}
								
								http.Post( "ws/api/panelauthenticate/event", token, MsgReceived, json.Make( JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_STR, JSON_NUM, JSON_SHORT_ARRAY,
												"Action", event,
												"Zone1_32", zg[ 0],
												"Zone33_64", zg[ 1],
												"Zone65_96", zg[ 2],
												"PanelName", app.site->panels[ panel ].name,
												"PanelAddress", panel,
												"Units", dev_indexs,	count	) );
								posted = true;
	 
							}
						}	
								
						// save event status
						for( int zonegroup = 0; zonegroup < 3; zonegroup++ )
						{
							evz[ event][ panel][ zonegroup] = zg[ zonegroup ];
						}
					}
				}
				
				if ( posted ) return;
				 
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
								
								 http.Post( "ws/api/panelauthenticate/event", token, MsgReceived, json.Make( JSON_NUM,
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
									http.Post( "ws/api/panelauthenticate/devicedelete", token, MsgReceived, json.Make( JSON_NUM, "Unit", du->unit ) );
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
									
									http.Post( "ws/api/panelauthenticate/device", token, MsgReceived, json.Make( JSON_NUM, JSON_STR, JSON_STR, JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_BOOL, JSON_BOOL,
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
								
								http.Post( "ws/api/panelauthenticate/panelfault", token, MsgReceived, json.Make( JSON_NUM, JSON_BOOL, "Fault", pf->fault, "Status", !pf->status ) );
				
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
							
								http.Post( "ws/api/panelauthenticate/devicefault", token, MsgReceived, json.Make( JSON_NUM, JSON_NUM, JSON_BOOL, JSON_NUM, JSON_BOOL, "Unit", df->unit, "Fault", df->fault, "IsWarning", iswarning, "Feature", df->chan, "Status", !df->status ) );
								
								break;
							}
						}
					}
				}
				else
				{
					time_t n = now( );
					
					if ( n - http.lastrequest > 60 )
					{
						http.Post( "ws/api/panelauthenticate/ping", token, MsgReceived, "{}" );
					}
				}	
			}
		}
	}
}

				
					
			
		
