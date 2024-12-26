/***************************************************************************
* File name: MM_MQTTSupport.cpp
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
* MQTTSupport Module
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
#include "MM_MQTTSupport.h"
#include "MM_PanelNetwork.h"
#include "MM_GSM.h"
#include "MM_Utilities.h"
#include "MM_CUtils.h"
#include "JSON.h"
#include <time.h>
#include <string.h>
#include "base64.h"

#include "version.h"

#include <string.h>

/* Defines
**************************************************************************/


#define DEST_DOMAIN 	"cygnus-systemsappserver.com"
#define DEST_URL		"cygnus-systemsappserver.com"
 
#define MQTT_PERIOD  ( 7 * 24 * 60 * 60 )
#define MQTT_MARGIN ( 125 * 60 )
 

#define APP_SUPP_MAX_TRYS 60

#define DEVICE "matt-panel-dev"
#define HUB "cygnus-iot-hub.azure-devices.net"


/* Globals
**************************************************************************/
 

short MQTTSupport::enabled = 1;		// On at the start by default..
bool MQTTSupport::ready = true;
bool MQTTSupport::cleared = false;

IOStatus* 			MQTTSupport::iostatus = NULL;
PanelNetwork* 		MQTTSupport::network = NULL;

JSON 				MQTTSupport::json;
 
char MQTTSupport::key[ 45] = "";
char MQTTSupport::logged_in = 0;
char MQTTSupport::connected = 0;

char MQTTSupport::queuebuff[ sizeof( AppMessage ) * APP_SUPPORT_QUEUE_SIZE ];
Circular<AppMessage> MQTTSupport::outqueue( (AppMessage*) queuebuff, APP_SUPPORT_QUEUE_SIZE );

char MQTTSupport::eventqueuebuff[ sizeof( AppEventMessage ) * APP_SUPPORT_EVENT_QUEUE_SIZE ];
Circular<AppEventMessage>  MQTTSupport::outeventqueue( (AppEventMessage*) eventqueuebuff, APP_SUPPORT_EVENT_QUEUE_SIZE );

char MQTTSupport::url[ 60] = "";
char MQTTSupport::device_id[ 18] = "";

AppEventMessage::AppEventMessage( unsigned int isClear, unsigned int panel, unsigned int event, uint32_t* bits ) : event( event), isClear( isClear ), panel( panel )
{
	((int*)zonebits)[ 0] = bits[0];
	((int*)zonebits)[ 1] = bits[1];
	((int*)zonebits)[ 2] = bits[2];
}



AppMessage::AppMessage( unsigned int unit, bool present )
{
	DeviceUpdate* du = &this->deviceupdate;
	
	du->type 	 = APP_INFO_DEVICE_UPDATE;
	du->unit 	 = unit;
	du->present	 = present;
}
	

AppMessage::AppMessage( AppSupportEvent e )
{
	PanelEvent* pe = &this->panelevent;
	
	pe->type 	 = APP_INFO_EVENT;
   pe->event    = e;
}

	
AppMessage::AppMessage( unsigned int unit, FaultType fault, unsigned int chan, bool status,  bool faults, bool warnings )
{
	DeviceFault* df = &this->devicefault;
	
	df->type = APP_INFO_DEVICE_FAULT;
	df->unit = unit;
	df->chan = chan;
	df->fault = fault;
	df->status = status;
	df->faults	 = faults;
	df->warnings = warnings;
}


AppMessage::AppMessage( FaultType fault, bool status )
{
	PanelFault* pf = &this->panelfault;
	
	pf->type 	= APP_INFO_PANEL_FAULT;
	pf->fault	= fault;
	pf->status	= status;	
}

 

uint32_t Rev32( uint32_t x )
{
	return ( x << 24) | ( x >> 24 ) | (( x & 0xFF00 ) << 8 ) | (( x & 0xFF0000 ) >> 8 );
}


/*************************************************************************/
/**  \fn      MQTTSupport::MQTTSupport( )
***  \brief   Constructor for class
**************************************************************************/

MQTTSupport::MQTTSupport( ) : Module( "MQTTSupport", 100, EVENT_RESET ), mqtt( HUB ), trys( 0 ) 
{
	if ( !EEPROM::GetVar( AppSupp_Enabled, enabled ) )
	{
		if ( !EEPROM::GetVar( AppSupp_Enabled, enabled ) )
		{
			//Fault::AddFault( FAULT_MQTT_APP_ENABLED_GET );
			//Log::Msg(LOG_SYS, "01_MQTTSupport.cpp -- ADD");
		}
	}
	
	if ( !EEPROM::GetVar( AppSupp_SiteId, *site_id ) )
	{
		if ( !EEPROM::GetVar( AppSupp_Enabled, *site_id  ) )
		{
			Fault::AddFault( FAULT_MQTT_APP_SITE_GET );
			//Log::Msg(LOG_SYS, "02_MQTTSupport.cpp -- ADD");
		}
	}
	site_id[ 39 ]= 0;
	 
	if ( !EEPROM::GetVar( AppSupp_URL, *url ) )
	{
		if ( !EEPROM::GetVar( AppSupp_URL, *url ) )
		{
			Fault::AddFault( FAULT_MQTT_APP_URL_GET );
			//Log::Msg(LOG_SYS, "03_MQTTSupport.cpp -- ADD");
		 
		}
	}
//	memset(url,0,60);
	url[59] = 0;
}


/*************************************************************************/
/**  \fn      int MQTTSupport::ResetHandler( )
***  \brief   Class function
**************************************************************************/

void MQTTSupport::ResetHandler( )
{
	logged_in = false;
	connected = false;	
}


/*************************************************************************/
/**  \fn      int MQTTSupport::Init( )
***  \brief   Class function
**************************************************************************/

int MQTTSupport::Init( )
{
	ncu = (NCU*) app.FindModule( "NCU" );
	settings = (Settings*) app.FindModule( "Settings" );
	iostatus = (IOStatus*) app.FindModule( "IOStatus" );
	network  =  (PanelNetwork*) app.FindModule( "PanelNetwork" );
	
	cleared = false;
	
	mqtt.dest_domain = url;
		 
	// Base encode UID to make mqtt device id..
	uint32_t uid[ 3] = {  Rev32(HAL_GetUIDw0()), Rev32(HAL_GetUIDw1()), Rev32(HAL_GetUIDw2()) };
	memcpy( device_id, Base64Enc( (char*) uid, 12 ), 16 );
	
	expiry = now() + MQTT_PERIOD;
	
	char* d = device_id;
	
	for( char* k = key + 15; k >= key; k-- )
	{
		*k = *d++;
	}
	
	d = device_id;
	
	for( char* k = key + 31; k >= key + 16; k-- )
	{
		*k = *d++;
	}
	
	strncpy( key + 32, site_id, 45-32 );
	
	return true;
}

  
/*************************************************************************/
/**  \fn      int MQTTSupport::Receive( Message* m )
***  \brief   Class function
**************************************************************************/

int MQTTSupport::Receive( Message* )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int MQTTSupport::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int MQTTSupport::Receive( Command* cmd )
{
	if ( cmd->type == CMD_SET_APP_SUPP )
	{
		if ( !!enabled != !!cmd->short0 )
		{
			enabled = cmd->short0;
				 
			if ( !EEPROM::SetVar( AppSupp_Enabled, enabled ) )
			{
				if ( !EEPROM::SetVar( AppSupp_Enabled, enabled ) )
				{
					Fault::AddFault( FAULT_MQTT_APP_ENABLED_SET );
					//Log::Msg(LOG_SYS, "04_MQTTSupport.cpp -- ADD");
				}
				else
				{
					Fault::RemoveFault( FAULT_MQTT_APP_ENABLED_SET );
					//Log::Msg(LOG_SYS, "05_MQTTSupport.cpp -- REMOVE");
				}
			}
			else
			{
				Fault::RemoveFault( FAULT_MQTT_APP_ENABLED_SET );
				//Log::Msg(LOG_SYS, "06_MQTTSupport.cpp -- REMOVE");
			}
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
		cmd->int1 = (int) GSM::pwd;
		cmd->int2 = (int) GSM::apn;
		
		return CMD_OK;
	}
	else if ( cmd->type == CMD_SETUP_APP )
	{
		// if site key different
		if ( strncmp( site_id,  (char*) cmd->int0, sizeof(site_id) ) )
		{
			// expire session
			expiry = MQTT_MARGIN;	
			
			// copy new
			strncpy( site_id,  (char*) cmd->int0, sizeof(site_id) );	
			
			// store to EEPROM
			if ( !EEPROM::SetVar( AppSupp_SiteId, *site_id ) )
			{
				if ( !EEPROM::SetVar( AppSupp_SiteId, *site_id ) )
				{
					Fault::AddFault( FAULT_MQTT_APP_SITE_SET );
					//Log::Msg(LOG_SYS, "07_MQTTSupport.cpp -- ADD");
				}
				else
				{
					Fault::RemoveFault( FAULT_MQTT_APP_SITE_SET );
					//Log::Msg(LOG_SYS, "08_MQTTSupport.cpp -- REMOVE");
				}
			}
			else
			{
				Fault::RemoveFault( FAULT_MQTT_APP_SITE_SET );
				//Log::Msg(LOG_SYS, "09_MQTTSupport.cpp -- REMOVE");
			}		 
		}
		
		// if APN different
		if ( strncmp( GSM::apn, (char*) cmd->int2, sizeof(GSM::apn) ) )
		{
			// restart GSM
			GSM::txready = true;
			GSM::state = GSM_ECHO_OFF;
			
			// copy new
			STRNCPY( GSM::apn, (char*) cmd->int2, sizeof(GSM::apn) );
		
			// store to EEPROM
			if ( !EEPROM::SetVar( AppSupp_APN, *GSM::apn ) )
			{
				if ( !EEPROM::SetVar( AppSupp_APN, *GSM::apn ) )
				{
					Fault::AddFault( FAULT_MQTT_APP_APN_SET );
					//Log::Msg(LOG_SYS, "10_MQTTSupport.cpp -- ADD");
				}
				else
				{
					Fault::RemoveFault( FAULT_MQTT_APP_APN_SET );
					//Log::Msg(LOG_SYS, "11_MQTTSupport.cpp -- REMOVE");
				}
			}
			else
			{
				Fault::RemoveFault( FAULT_MQTT_APP_APN_SET );
				//Log::Msg(LOG_SYS, "12_MQTTSupport.cpp -- REMOVE");
			}
		}
		
		
		// if APN different
		if ( strncmp( GSM::pwd, (char*) cmd->int1, sizeof(GSM::pwd) ) )
		{
			if ( *GSM::pwd != 0 )
			{
				// restart GSM
				GSM::txready = true;
				GSM::state = GSM_ECHO_OFF;
			}
			
			// copy new
			STRNCPY( GSM::pwd, (char*) cmd->int1, sizeof(GSM::pwd) );
		
			// store to EEPROM
			if ( !EEPROM::SetVar( AppSupp_Password, *GSM::pwd ) )
			{
				if ( !EEPROM::SetVar( AppSupp_Password, *GSM::pwd ) )
				{
					Fault::AddFault( FAULT_MQTT_APP_PASSWORD_SET );
					//Log::Msg(LOG_SYS, "13_MQTTSupport.cpp -- ADD");
				}
				else
				{
					Fault::RemoveFault( FAULT_MQTT_APP_PASSWORD_SET );
					//Log::Msg(LOG_SYS, "14_MQTTSupport.cpp -- REMOVE");
				}
			}
			else
			{
				Fault::RemoveFault( FAULT_MQTT_APP_PASSWORD_SET );
				//Log::Msg(LOG_SYS, "15_MQTTSupport.cpp -- REMOVE");
			}
		}
		
		return CMD_OK;
	}	
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int MQTTSupport::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int MQTTSupport::Receive( Event e )
{
	// if event == EVENT_RESET
	logged_in = false;
	
	return 0;
}


/*************************************************************************/
/**  \fn      int MQTTSupport::Send( MQTTSupportInfo info, int id, int parent )
***  \brief   Class function
**************************************************************************/

	
int MQTTSupport::Send( AppMessage& msg )
{	 
	if ( enabled && ! outqueue.Full( ) )
	{	 
		outqueue.Push( msg );
		
		app.DebOut( "Pushing: type %d (0x%0X) of %d\n", msg.type, *(int*)&msg, outqueue.items );
		
		return true;
	}
	return false;
}
	

static AppMessage* msg = NULL;


/*************************************************************************/
/**  \fn      void MQTTSupport::ConnectMsgReceived( int code, char* data, int length )
***  \brief   Class function
**************************************************************************/

void MQTTSupport::ConnectMsgReceived( char* data, int length )
{
	if ( *(short*)data == 0x220 && length >= 2 )	// conn. accepted
	{
		connected = true;
		app.DebOut( "MQTT connected." );
		//Log::Msg( LOG_SYS, "MQTT connected." );
	}
	else
	{
		app.DebOut( "MQTT connect fail. code %d\n", *(short*)data );
	}
}


/*************************************************************************/
/**  \fn      void MQTTSupport::PublishMsgReceived( char* data, int length )
***  \brief   Class function
**************************************************************************/
	
void MQTTSupport::EventMsgReceived( char* data, int length )
{
	if ( *(short*)data == 0x0240 )
	{
 	   AppEventMessage* msg = outeventqueue.Pop();
		app.DebOut("Popping: event %d", msg->event );
	}
	else
	{
		app.DebOut( "MQTT event msg fail. code %d\n", *(short*)data );
	}
}

/*************************************************************************/
/**  \fn      void MQTTSupport::PublishMsgReceived( char* data, int length )
***  \brief   Class function
**************************************************************************/
	
void MQTTSupport::QueueMsgReceived( char* data, int length )
{
	if ( *(short*)data == 0x0240 )
	{
 		AppMessage* msg = outqueue.Pop();
		app.DebOut( "Poppin: type %d (0x%0X) of %d\n", msg->type, *(int*)&msg, outqueue.items );
	}
	else
	{
		app.DebOut( "MQTT queue msg fail. code %d\n", *(short*)data );
	}
}

/*************************************************************************/
/**  \fn      void MQTTSupport::LoggedOnMsgReceived( char* data, int length )
***  \brief   Class function
**************************************************************************/
	
void MQTTSupport::LoggedInMsgReceived( char* data, int length )
{
	logged_in = true;
}


/*************************************************************************/
/**  \fn      void MQTTSupport::LoggedOnMsgReceived( char* data, int length )
***  \brief   Class function
**************************************************************************/
	
void MQTTSupport::PingMsgReceived( char* data, int length )
{
	// ping!
}

 
/*************************************************************************/
/**  \fn      uint32_t MQTTSupport::GetZoneGroup( ActionEnum event, int panel, int group )
***  \brief   Class function
**************************************************************************/

uint32_t MQTTSupport::GetZoneGroup( int event, int panel, int group )
{
	if ( panel == app.site->currentIndex )
	{
		return iostatus->zonebits[ event ][ group ];
	}
	else
	{
		uint32_t zg = 0;
		
		for( TimedDeviceRef* tr = iostatus->netAlertList[ panel ]; tr < iostatus->netAlertList[ panel ] + MAX_NET_REFS; tr++ )
		{
			if ( tr->index == ALERT_UNUSED )
			{
				break;
			}
			if ( tr->action == event )
			{
				if ( tr->dispzone != 0 )
				{
					zg |= 1 << (( tr->dispzone - 1 ) - group * 32 );
				}
			}
		}
		return zg;
	}
}


static uint32_t prev_zg[ ACTION_SHIFT_FAULT][ SITE_MAX_PANELS][ SITE_MAX_ZONES / sizeof(uint32_t) ];



/*************************************************************************/
/**  \fn      int  CountZones( uint32_t* zonebits )
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
/**  \fn      void AppSupport::CheckEventStatus( )
***  \brief   Class function
**************************************************************************/

void MQTTSupport::CheckEventStatus( )
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
				

char* CreateAmbient( Device* d )
{
	int heat = 0;
	int smoke = 0;
	
	DeviceConfig* dc = d->config;
	
	// if config found
	if ( dc != NULL )
	{
		// for each channel
		for( InputChannel* ic = dc->input; ic < dc->input + dc->numInputs; ic++ )
		{
			// i = index
			int i = ic - dc->input;
			 
			if ( i < SITE_MAX_ANALOGUE_CHANNELS )
			{
				// if channel is heat
				if ( ic->type == CO_CHANNEL_HEAT_B_E || ic->type == CO_CHANNEL_HEAT_A1R_E )
				{
					// update value
					heat = d->analogue[ i ];
				}
				else if ( ic->type == CO_CHANNEL_SMOKE_E )
				{
					// update value
					smoke = d->analogue[ i ] ;
				}
			}
		}
		
		// if have a smoke value
		if ( smoke != 0 )
		{ 
			if ( heat != 0 )
			{
				return JSON::Make( JSON_NUM, JSON_NUM, "Smoke", smoke, "Heat", heat );
			}
			else
			{
				return JSON::Make( JSON_NUM, "Smoke", smoke );
			}
		}
		else
		{
			if ( heat != 0 )
			{
				return JSON::Make( JSON_NUM, "Heat", heat );
			}
		}
	}
	return NULL;								 
}


#define INSERT(x) #x
#define STR(tok) INSERT(tok)

/*************************************************************************/
/**  \fn      virtual void MQTTSupport::Poll( )
***  \brief   Class function
**************************************************************************/

void MQTTSupport::Poll( )
{
	static time_t last_ping = 0;
	
	if ( enabled && app.panel != NULL && app.site != NULL )
	{	
		if ( GSM::state == GSM_IDLE )		
		{
			// if expiry nearly up..		
			if ( connected && now() > expiry - MQTT_MARGIN )
			{				
				 GSM::state = GSM_CLOSE_SOCKET;
				 connected = false;
			}
			
			else if ( !connected )
			{			 
				// create new expiry
				expiry = now() + MQTT_PERIOD;
				
				// re-create key ( in case sitekey ID changed )
				strncpy( key + 32, site_id, 45-32 );
				
				int len;
				// re-init mqtt
				char* conn = mqtt.Init( expiry, device_id, key, len, ResetHandler );  	  
			
				// then connect
				mqtt.Connect( conn, len, ConnectMsgReceived );
				
				if ( trys++ == APP_SUPP_MAX_TRYS )
				{
					Fault::AddFault( FAULT_UNABLE_TO_CONNECT_TO_SERVER );
					priority = 5000;
					trys = 0;
				}
				
				// force log on next
				logged_in = false;
			}
			else if ( !logged_in )
			{	 
				// check if connect failed previously
				if ( priority > 100 )
				{
					Fault::RemoveFault( FAULT_UNABLE_TO_CONNECT_TO_SERVER );
				
					priority = 100;
				}
				
				// full logon
				if ( mqtt.Post( IOT_PanelInit, LoggedInMsgReceived, json.Make( JSON_STR, JSON_NUM, JSON_BOOL, JSON_STR,
										"PanelUID", Utilities::UID,
										"MaxDevices", app.panel->numRadioDevs,
										"Restart", !cleared,
										"PanelFW", STR( FIRMWARE_VERSION ) ) ) ) 
				{
				
				}
				
				// Reset connecy trys
				trys = 0;
			}
			else 
			{
				
				cleared = true;
			
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
							mqtt.Post( IOT_PanelEvent, EventMsgReceived, json.Make( JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_STR, JSON_NUM, 
												"Action", eventmsg->event,
												"Zone1_32", 0,
												"Zone33_64", 0,
												"Zone65_96", 0,
												"PanelName", app.site->panels[ eventmsg->panel ].name,
												"PanelAddress", eventmsg->panel )
												);
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
								
								mqtt.Post( IOT_PanelEvent, EventMsgReceived, json.Make( JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_STR, JSON_NUM, JSON_SHORT_ARRAY,
												"Action", eventmsg->event,
												"Zone1_32",  ((int*)eventmsg->zonebits)[ 0],
												"Zone33_64", ((int*)eventmsg->zonebits)[ 1],
												"Zone65_96", ((int*)eventmsg->zonebits)[ 2],
												"PanelName", app.site->panels[ eventmsg->panel ].name,
												"PanelAddress", eventmsg->panel,
												"Units", dev_indexs,	count	) );
								posted = true;
							}
							else // is nother panel
							{
								// post without device list
								
								mqtt.Post( IOT_PanelEvent, EventMsgReceived, json.Make( JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_STR, JSON_NUM, 
												"Action", eventmsg->event,
												"Zone1_32",  ((int*)eventmsg->zonebits)[ 0],
												"Zone33_64", ((int*)eventmsg->zonebits)[ 1],
												"Zone65_96", ((int*)eventmsg->zonebits)[ 2],
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
								
								 mqtt.Post( IOT_PanelEvent, QueueMsgReceived, json.Make( JSON_NUM,
												"Action", e->event ) );	 
								break;
							}
							
							case APP_INFO_DEVICE_UPDATE :
							{
								DeviceUpdate* du = &msg->deviceupdate;
								 
								Device* d = iostatus->FindDevice( du->unit );
								
								RadioUnit* ru = NULL;
								DeviceConfig* dc = NULL;
								
								if ( d != NULL )
								{
									ru = ncu->FindUnit( du->unit );
									dc = d->config;
								}
								
								if ( ru == NULL || !du->present )
								{
									mqtt.Post( IOT_DeviceDrop, QueueMsgReceived, json.Make( JSON_NUM, "Unit", du->unit ) );
								}						
								else if ( dc != NULL )
								{
									const char* location = dc->location;
										
									if ( location[ 0] == 0 ) location = "--";
									
									char* zonename = NULL;
									/*
									for( Zone* z = app.panel->zone; z < app.panel->zone + app.panel->numDefinedZones; z++ )
									{
										if ( z->num == dc->zone )
										{
											zonename = z->name;
										}
									}*/
								//	if ( 1 )//zonename == NULL )
									{
										static char znamebuff[ 12];
										sprintf( znamebuff, "%d", dc->zone );
										zonename = znamebuff;
									}
									
									static char combined[ 1024];
									
									STRNCPY( combined, json.Make( JSON_NUM, JSON_STR, JSON_STR, JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_NUM, JSON_STR,
													"Unit", du->unit,
													"Zone", zonename,
													"Location", location,
													"RSSI", ru->rssiprimary,
													"SecondaryRSSI", ru->rssisecondary,
													"TypeCode", dc->type,
													"Parent", ru->primary,
													"Secondary", ru->secondary,
													"Rank", ru->rank,
													"Battery", ru->battery,
													"BatteryVoltage", ru->battprimary,
													"StartTime", ru->starttime,
													"Serial", ru->serial ), 1024 );
									
									// create extention json
									char* extention = CreateAmbient( d );
									
									// if is any
									if ( extention != NULL )
									{
										// glue together
										int len = strlen( combined );
										combined[ len - 1 ] = ',';
										combined[ len ] = ' ';
										STRNCPY( combined + len + 1, extention + 1, 1024 );
									}	
									mqtt.Post( IOT_DeviceUpdate, QueueMsgReceived, combined );
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
								
								mqtt.Post( IOT_PanelFault, QueueMsgReceived, json.Make( JSON_NUM, JSON_BOOL, "Fault", pf->fault, "Status", !pf->status ) );
				
								break;
							}
							
							case APP_INFO_DEVICE_FAULT :
							{
								bool iswarning = false;
								
								DeviceFault* df = &msg->devicefault;
								
								// if clear faults
								if ( df->fault == FAULT_SIGNAL_NO_ERROR_E )
								{
									mqtt.Post( (IOTMsg) df->chan, QueueMsgReceived, json.Make( JSON_NUM, "Unit", df->unit ) );
								}
								else  
								{ 
									if ( df->fault == FAULT_SIGNAL_BATTERY_ERROR_E ||
										  df->fault == FAULT_SIGNAL_LOW_LINK_QUALITY_E ||
										  df->fault == FAULT_SIGNAL_LOW_PARENT_COUNT_FAULT_E )
									{
										iswarning = true;
									}
								
									mqtt.Post( IOT_DeviceFault, QueueMsgReceived, json.Make( JSON_NUM, JSON_NUM, JSON_BOOL, JSON_NUM, JSON_BOOL, JSON_BOOL, JSON_BOOL, "Unit", df->unit, "Fault", df->fault, "IsWarning", iswarning, "Feature", df->chan, "Status", !df->status, "Faults", df->faults, "Warnings", df->warnings ) );
								}
								break;
							}
						}
					}
				}
				else
				{
					time_t n = now( );
					
					if ( n - last_ping > 27 )
					{
						mqtt.Ping( PingMsgReceived );
						
						last_ping = n;
					}
				}	
			}
		}
	}
}

				
