/***************************************************************************
* File name: MM_ImportXML.cpp
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


/* System Include Files
**************************************************************************/
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "xml.h"
#include "MM_ImportXML.h"
#include "MM_ExportXML.h"
#include "CO_Site.h"
#include "MM_Fault.h"
#include <stdlib.h>

/* Defines
**************************************************************************/
 

#define DEFAULT else
#define SWITCH(s) char* ___sw = s; if (0);
#define CASE(c) else if ( !strcmp( c, ___sw ) )
 
 

static XMLList lib;
static ImportXML* xmli = NULL;


static int GetActionBits( char* str )
{
	uint16_t a = 0;
	int last = false;
	
	if ( *str == 0 ) return 0;
	
	char* s = str;
	
	do
	{
		char* p = strchr( s, ',' );
		
		if ( p != NULL )
		{
			*p = 0;
		}
		else
		{
			last = true;
		}
		
		SWITCH( s )	
			
			CASE( "fire" )				a |= ACTION_BIT_FIRE;
			CASE( "first-aid" )  	a |= ACTION_BIT_FIRST_AID;
			CASE( "security" )  		a |= ACTION_BIT_SECURITY;
			CASE( "evacuate" )  		a |= ACTION_BIT_EVACUATION;
			CASE( "fault" )  			a |= ACTION_BIT_FAULT;
			CASE( "general" )  		a |= ACTION_BIT_GENERAL;
			CASE( "routing-ack" )	a |= ACTION_BIT_ROUTING_ACK;
			CASE( "mesh-events" )	a |= ACTION_BIT_MESH_FOR_SMS;

			DEFAULT return -1;
		
		s = p + 1;
		
	} while ( !last );
	
	return a;
}
		

ActionEnum Action( char* s )
{
	ActionEnum a = (ActionEnum) -1;
	
	SWITCH( s )	
		CASE( "fire" )				a = ACTION_SHIFT_FIRE;		 
		CASE( "first-aid" )  	a = ACTION_SHIFT_FIRST_AID;	
		CASE( "firstaid" )  		a = ACTION_SHIFT_FIRST_AID;		
		CASE( "security" )  		a = ACTION_SHIFT_SECURITY;		
		CASE( "general" )  		a = ACTION_SHIFT_GENERAL;		
		CASE( "evacuate" )  		a = ACTION_SHIFT_EVACUATION;			
		CASE( "fault" )   		a = ACTION_SHIFT_FAULT;
		CASE( "reset" ) 			a = ACTION_SHIFT_RESET;
		CASE( "silence" ) 		a = ACTION_SHIFT_SILENCE;
		CASE( "routing-ack" ) 	a = ACTION_SHIFT_ROUTING_ACK;
		CASE( "routingack" ) 	a = ACTION_SHIFT_ROUTING_ACK;
		CASE( "undefined" ) 		a = ACTION_SHIFT_UNDEFINED;
		CASE( "confirm" ) 		a = ACTION_SHIFT_CONFIRM_FIRE;
		CASE( "output" )			a = ACTION_SHIFT_CAE_OUTPUT;
	
	return a;
}	



static ProfileEnum GetProfile( char* s )
{
	ProfileEnum a = PROFILE_NONE;
	
	SWITCH( s )	
		CASE( "fire" )				a = PROFILE_FIRE;		 
		CASE( "first-aid" )  	a = PROFILE_FIRST_AID;	
		CASE( "firstaid" )  		a = PROFILE_FIRST_AID;	
		CASE( "security" )  		a = PROFILE_SECURITY;		
		CASE( "general" )  		a = PROFILE_GENERAL;		
		CASE( "evacuate" )  		a = PROFILE_EVACUATION;		
		CASE( "evacuation" )  	a = PROFILE_EVACUATION;			
		CASE( "fault" )   		a = PROFILE_FAULT;
	 	CASE( "routing-ack" ) 	a = PROFILE_ROUTING_ACK;
		CASE( "routingack" ) 	a = PROFILE_ROUTING_ACK;
		CASE( "silent" ) 			a = PROFILE_SILENT;
		CASE( "silenttest" ) 	a = PROFILE_SILENT;
		CASE( "test" ) 			a = PROFILE_TEST;
	return a;
}	

static CAEAction Effect( char* s )
{
	CAEAction a;
	
	SWITCH( s )	
		CASE( "activate" )		a = CAE_ACTIVATE;		 
		CASE( "enable" )  		a = CAE_ENABLE;		
		CASE( "disable" )  		a = CAE_DISABLE;	
		CASE( "test" )				a = CAE_TEST;
		DEFAULT 						a = CAE_NONE;
		
	return a;
}	


/*************************************************************************/
/**  \fn      static ChannelType GetChannelType( char* s )
***  \brief   Helper
**************************************************************************/


static int GetBoolean( int& r, char* s )
{
	SWITCH( s )
		CASE( "True" ) 		r = true;
		CASE( "true" ) 		r = true;
		CASE( "false" ) 		r = false;
		CASE( "False" ) 		r = false;	
		DEFAULT return false;
	return true;
}

 
/*************************************************************************/
/**  \fn      static ChannelType GetChannelType( char* s )
***  \brief   Helper
**************************************************************************/


static ChannelType GetChannelType( char* s )
{
	ChannelType t = CO_CHANNEL_NONE_E;
	int i, o;
	
	SWITCH( s )
		CASE( "smoke" ) 		t = CO_CHANNEL_SMOKE_E;
		CASE( "a1rheat" ) 	t = CO_CHANNEL_HEAT_A1R_E;
		CASE( "heatb" ) 		t = CO_CHANNEL_HEAT_B_E;
		CASE( "co" ) 			t = CO_CHANNEL_CO_E;
		CASE( "pir" ) 			t = CO_CHANNEL_PIR_E;
		CASE( "sounder" ) 	t = CO_CHANNEL_SOUNDER_E;
		CASE( "sitesounder" ) t = CO_CHANNEL_SITENET_SOUNDER_E;
	
		CASE( "beacon" ) 		t = CO_CHANNEL_BEACON_E;
		CASE( "fire" ) 		t = CO_CHANNEL_FIRE_CALLPOINT_E;
		CASE( "statusled" ) 	t = CO_CHANNEL_STATUS_INDICATOR_LED_E;
		CASE( "vi" ) 			t = CO_CHANNEL_VISUAL_INDICATOR_E;
		CASE( "soundervi" ) 	t = CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E;
		CASE( "firstaid" ) 	t = CO_CHANNEL_MEDICAL_CALLPOINT_E;
		CASE( "evac" ) 		t = CO_CHANNEL_EVAC_CALLPOINT_E;
		CASE( "routing" )		t = CO_CHANNEL_OUTPUT_ROUTING_E;
	

		CASE( "panelinput1" ) t = CO_CHANNEL_PANEL_ROUTING_ACK;
		CASE( "panelinput2" ) t = CO_CHANNEL_PANEL_CALLPOINT;
		CASE( "panelinput3" ) t = CO_CHANNEL_PANEL_ALARM_INPUT;
		CASE( "panelinput4" ) t = CO_CHANNEL_PANEL_SILENCE_INPUT;
		CASE( "panelinput5" ) t = CO_CHANNEL_PANEL_FAULT_INPUT;
		CASE( "panelinput6" ) t = CO_CHANNEL_PANEL_FUNCTION_BUTTON_1;
		CASE( "panelinput7" ) t = CO_CHANNEL_PANEL_FUNCTION_BUTTON_2;
		CASE( "panelinput8" ) t = CO_CHANNEL_PANEL_EVACUATE_BUTTON;
	
		
		CASE( "paneloutput1" ) t = CO_CHANNEL_PANEL_ALARM_RELAY;
		CASE( "paneloutput2" ) t = CO_CHANNEL_PANEL_FAULT_RELAY;
		CASE( "paneloutput3" ) t = CO_CHANNEL_PANEL_FIRE_RELAY;
		CASE( "paneloutput4" ) t = CO_CHANNEL_PANEL_ROUTING_OUTPUT;
		CASE( "paneloutput5" ) t = CO_CHANNEL_PANEL_SOUNDERS_RELAY;
		
	else if ( sscanf( s, "input%d", &i ) == 1 ) 
	{
		t = (ChannelType) ( CO_CHANNEL_INPUT_1_E + i - 1 );
	}
	else if ( sscanf( s, "output%d", &o ) == 1 ) 	
	{
		t = (ChannelType) ( CO_CHANNEL_OUTPUT_1_E + o - 1 );
	}		
		
	return t;
}


/*************************************************************************/
/**  \fn      int SiteHandler::SiteHandler( "site", &xml )
***  \brief   Class constructor
**************************************************************************/

SiteHandler::SiteHandler( ) : XMLElementHandler( ".site", &lib )
{

}


/*************************************************************************/
/**  \fn      int SiteHandler::Begin( )
***  \brief   Class function
**************************************************************************/

int SiteHandler::Begin( )
{
	xmli->site = (Site*) xmli->site_buff;	
	memset( xmli->site, 0, OFFSET( Site, panels ) );
	
	xmli->site->numPanels = 0;
	xmli->site->numContacts = 0;
	xmli->site->numUsers = 0;
	xmli->site->flags = 0;
	xmli->site->currentIndex = 0;
	
	xmli->site->batt_low[ 0] = 6900;
	xmli->site->batt_med[ 0] = 7800;
	xmli->site->batt_high[ 0] = 8700;
	xmli->site->batt_low[ 1] = 2900;
	xmli->site->batt_med[ 1] = 3100;
	xmli->site->batt_high[ 1] = 3200;
	xmli->site->batt_panel = 23010; 
		
	return xmli->site != NULL ;
}


/*************************************************************************/
/**  \fn      int SiteHandler::Attribute( char* att )
***  \brief   Class function
**************************************************************************/

int SiteHandler::Attribute( char* attr, char* value )
{
	SWITCH( attr )
		CASE( "name" )
		{	
			STRCHK( xmli->site->name, value, SITE_MAX_STRING_LEN );						 
		}
		CASE( "uniqueid" )
		{	
			STRCHK( xmli->site->uid, value, SITE_MAX_STRING_LEN );
		}
		CASE( "version" )		xmli->site->version = atoi( value );
		CASE( "batterylow" ) xmli->site->batt_low[ 0] = atoi( value );
		CASE( "batterymed" ) xmli->site->batt_med[ 0] = atoi( value );
		CASE( "batteryhigh" ) xmli->site->batt_high[ 0] = atoi( value );
		CASE( "batterylowsn" ) xmli->site->batt_low[ 1] = atoi( value );
		CASE( "batterymedsn" ) xmli->site->batt_med[ 1] = atoi( value );
		CASE( "batteryhighsn" ) xmli->site->batt_high[ 1] = atoi( value ); 
		CASE( "panelbatterylow" ) xmli->site->batt_panel = atoi( value ); 
		
	return XML_OK;
}


/*************************************************************************/
/**  \fn      int SiteHandler::End( )
***  \brief   Class function
**************************************************************************/

int SiteHandler::End( )
{
	xmli->Write( 0, xmli->site, OFFSET( Site, panels ) );
	 
	app.DebOut( "end site\n" );
	
	return XML_OK;
}


	
/*************************************************************************/
/**  \fn      int PanelHandler::PanelHandler( "site", &xml )
***  \brief   Class constructor
**************************************************************************/

PanelHandler::PanelHandler( ) : XMLElementHandler( ".site.panels.panel", &lib )
{

}


/*************************************************************************/
/**  \fn      int PanelHandler::Begin( )
***  \brief   Class function
**************************************************************************/

int PanelHandler::Begin( )
{
	xmli->panel = (Panel*) xmli->panel_buff;
	
	xmli->panel->numPanelDevs = 0;
	xmli->panel->numRadioDevs = 0;
	xmli->panel->numDefinedZones = 0;
	xmli->panel->numCaeRules = 0;
	xmli->panel->numCaeInputs = 0;
	xmli->panel->numCaeOutputs = 0;
	xmli->panel->settings = 0;
				
	for( int i = 0; i < 7; i++ )
	{
		xmli->panel->day[ i].start.hour = 25;
	}

		
	return true;
}


/*************************************************************************/
/**  \fn      int SiteHandler::Attribute( char* att )
***  \brief   Class function
**************************************************************************/

int PanelHandler::Attribute( char* attr, char* value )
{
	SWITCH( attr )
		CASE( "name" )
		{
			STRCHK( xmli->panel->name, value, SITE_MAX_STRING_LEN );
			STRCHK( xmli->names[ xmli->site->numPanels ], value, SITE_MAX_STRING_LEN );
			
			if ( !strcmp( value, xmli->panel_name ) )
			{
				xmli->site->currentIndex = xmli->site->numPanels;
			}
		}	
			
		CASE( "address" )	
		{	
			xmli->panel->address = atoi( value );
			if ( xmli->panel->address < 1 || xmli->panel->address > SITE_MAX_PANELS )
			{
				return XML_BAD_VALUE;
			}
		}	
		CASE( "numLEDS" ) 
		{
			xmli->panel->numZoneLeds = atoi( value );
		}	
		
		CASE( "systemID" )	
		{
			xmli->panel->systemId = atoi( value );
			if ( xmli->panel->systemId == 0 ) return XML_BAD_NUMBER_VALUE;
		}	
		CASE( "startfrequency" )
		{
			xmli->panel->freq = atoi( value );
		}
		CASE( "alldelaysoff" )
		{
			int result;
			
			if ( GetBoolean( result, value ) )
			{
				if ( result )
				{
					xmli->panel->settings |= SITE_SETTING_DISABLE_OUTPUT_DELAYS;
				}
			}
			else  return XML_BAD_VALUE;
		}
		CASE( "framelength" )
		{
			xmli->panel->framelength = atoi( value );
		}
		CASE( "maxdevices" )
		{
			xmli->panel->maxDevices = atoi( value );
		}
		CASE( "numRadioDevs" )
		{
			xmli->panel->numRadioDevs = atoi( value );
		} 
		CASE( "ignoresecuritynighttime" )
		{
			int result;
			
			if ( GetBoolean( result, value ) )
			{
				if ( result )
				{
					xmli->panel->settings |= SITE_SETTING_IGNORE_SECURITY_AT_NIGHT;
				}
			}
			else return XML_BAD_VALUE;
		}
		
		CASE( "ignoresecuritydaytime" )
		{
			int result;
			
			if ( GetBoolean( result, value ) )
			{
				if ( result )
				{
					xmli->panel->settings |= SITE_SETTING_IGNORE_SECURITY_IN_DAY;
				}
			}
			else return XML_BAD_VALUE;
		}
		
		CASE( "resoundsamezone" )
		{
			int result;
			
			if ( GetBoolean( result, value ) )
			{
				if ( !result )
				{
					xmli->panel->settings |= SITE_SETTING_RESOUND_NEW_ZONE;
				}
			}
			else return XML_BAD_VALUE;
		}
		CASE( "useglobaldelays" )
		{
			int result;
			
			if ( GetBoolean( result, value ) )
			{
				if ( result )
				{
					xmli->panel->settings |= SITE_SETTING_GLOBAL_DELAY_OVERRIDE;
				}
			}
			else return XML_BAD_VALUE;		
		}
		CASE( "delay1" )
		{
			xmli->panel->delay1 = atoi( value );
		}
		CASE( "delay2" )
		{
			xmli->panel->delay2 = atoi( value );
		}
		CASE( "gsm" )
		{
			int result;
			
			if ( GetBoolean( result, value ) )
			{
				if ( !result )
				{
					xmli->panel->settings |= SITE_SETTING_GSM_AVAILABLE;
				}
			}
			else return XML_BAD_VALUE;
		}	 
			 
	return XML_OK;
}


/*************************************************************************/
/**  \fn      int PanelHandler::End( )
***  \brief   Class function
**************************************************************************/

int PanelHandler::End( )
{
	for( int i = 0; i < 7; i++ )
	{
		if ( xmli->panel->day[ i].start.hour == 25 )
		{
			return XML_MISSING_DATA;
		}
	}	
	
	xmli->Write( OFFSET( Site, panels ) + xmli->site->numPanels * sizeof(Panel), xmli->panel, OFFSET( Panel, devs ) );
	 	
	xmli->site->numPanels++;
		
	app.DebOut( "end penel\n" );
	
	return XML_OK;
}


/*************************************************************************/
/**  \fn      int ProfileHandler::ProfileHandler( "site", &xml )
***  \brief   Class constructor
**************************************************************************/

ProfileHandler::ProfileHandler( ) : XMLElementHandler( ".site.profiles.profile", &lib )
{

}


/*************************************************************************/
/**  \fn      int ProfileHandler::Begin( )
***  \brief   Class function
**************************************************************************/

int ProfileHandler::Begin( )
{
	index = -1;
	prof.ext_sounder = 255;
	prof.flash = 255;
	
	return true;
}


/*************************************************************************/
/**  \fn      int ProfileHandler::Attribute( char* att )
***  \brief   Class function
**************************************************************************/

int ProfileHandler::Attribute( char* attr, char* value )
{
	if ( !strcmp( attr, "selection" ) )
	{
		prof.tone = atoi( value );
	}		
	else if ( !strcmp( attr, "action" ) )
	{
		ProfileEnum pe = GetProfile( value );
		if ( pe == PROFILE_NONE ) return XML_BAD_VALUE;
		index = pe;		 
	}
	else if ( !strcmp( attr, "extsndr" ) )
	{
		prof.ext_sounder = atoi( value );
	}		
	else if ( !strcmp( attr, "vifr" ) )
	{
		prof.flash = atoi( value );
	}
	 
	return XML_OK;
}


static uint8_t default_extsnd[ ] = { 1, 2, 3, 4, 6, 6, 6, 5, 6 };
static uint8_t default_flash_bits = 4;

/*************************************************************************/
/**  \fn      int ProfileHandler::End( )
***  \brief   Class function
**************************************************************************/

int ProfileHandler::End( )
{	
	if ( index >= 0 && index < SITE_NUM_PROFILES )
	{
		if ( prof.ext_sounder == 255 )
		{
			prof.ext_sounder = default_extsnd[ index ];
		}
		
		if ( prof.flash == 255 )
		{
			prof.flash = 1;
		}
		
		xmli->site->profiles[ index] = prof;
	}
	else
	{
		return XML_MISSING_ATTRIBUTE;
	}
	 
	return XML_OK;
}



/*************************************************************************/
/**  \fn      int ProfilesHandler::ProfilesHandler( "site", &xml )
***  \brief   Class constructor
**************************************************************************/

ProfilesHandler::ProfilesHandler( ) : XMLElementHandler( ".site.profiles", &lib )
{

}


/*************************************************************************/
/**  \fn      int ProfilesHandlers::End( )
***  \brief   Class function
**************************************************************************/

int ProfilesHandler::Begin( )
{
	for( int n = 0; n < SITE_NUM_PROFILES; n++ )
	{
		xmli->site->profiles[ n].tone = 255;
	}
	return true;
}


/*************************************************************************/
/**  \fn      int ProfilesHandlers::End( )
***  \brief   Class function
**************************************************************************/

int ProfilesHandler::End( )
{
	for( int n = 0; n < SITE_NUM_PROFILES; n++ )
	{
		if ( xmli->site->profiles[ n].tone == 255 )
		{
			return XML_MISSING_DATA;
		}
	}
		
	return XML_OK;
}






/*************************************************************************/
/**  \fn      int NetEventHandler::NetEventHandler( "site", &xml )
***  \brief   Class constructor
**************************************************************************/

NetEventHandler::NetEventHandler( ) : XMLElementHandler( ".site.networking.receivepanel.transmitpanel.networkevent", &lib )
{

}


/*************************************************************************/
/**  \fn      int NetEventHandler::Begin( )
***  \brief   Class function
**************************************************************************/

int NetEventHandler::Begin( )
{
	xmli->action_index = -1;
	data = 0;
	
	return true;
}


/*************************************************************************/
/**  \fn      int NetEventHandler::Attribute( char* att )
***  \brief   Class function
**************************************************************************/

int NetEventHandler::Attribute( char* attr, char* value )
{
	if ( !strcmp( attr, "event" ) )
	{
		xmli->action_index = Action( value );
		if ( ( xmli->action_index >= 0 && xmli->action_index < SITE_NUM_OUTPUT_ACTIONS ) ||
				xmli->action_index == ACTION_SHIFT_SILENCE || xmli->action_index == ACTION_SHIFT_RESET || xmli->action_index == ACTION_SHIFT_CONFIRM_FIRE )
			;	// ok
			else return XML_BAD_VALUE;
		
		data |= 1;
	}	
	else if ( !strcmp( attr, "display" ) )
	{
		if ( xmli->receive_panel_index >= 0 && xmli->transmit_panel_index >= 0 && xmli->action_index >= 0 &&
			xmli->transmit_panel_index < SITE_MAX_PANELS )
		{	
			data |= 2;

			if ( ! strcmp( value, "true" ) )
			{
				if ( xmli->action_index < SITE_NUM_OUTPUT_ACTIONS && xmli->action_index != ACTION_SHIFT_FAULT )
				{
					for( int i = 0; i < SITE_MAX_ZONES / 32; i++ )
					{								
						xmli->net[ xmli->transmit_panel_index ][ xmli->action_index ].display[ i] = UINT_MAX;
					}
				}
				else 
				{
					xmli->net[ xmli->transmit_panel_index ][ ACTION_SHIFT_FAULT ].display[ 0] |= 1 << xmli->action_index;
				}
			}
			else if ( strcmp( value, "partial" ) && strcmp( value, "false" ) ) return XML_BAD_VALUE;
		}
	}	
	else if ( !strcmp( attr, "process" ) )
	{
		if ( xmli->receive_panel_index >= 0 && xmli->transmit_panel_index >= 0 && xmli->action_index >= 0 &&
					xmli->transmit_panel_index < SITE_MAX_PANELS )
		{	
			data |= 4;
			
			if ( ! strcmp( value, "true" ) )
			{
				if ( xmli->action_index < SITE_NUM_OUTPUT_ACTIONS && xmli->action_index != ACTION_SHIFT_FAULT )
				{
					for( int i = 0; i < SITE_MAX_ZONES / 32; i++ )
					{								
						xmli->net[ xmli->transmit_panel_index ][ xmli->action_index ].process[ i] = UINT_MAX;
					}
				}
				else
				{
					xmli->net[ xmli->transmit_panel_index ][ ACTION_SHIFT_FAULT ].process[ 0] |= 1 <<  xmli->action_index;
				}
			 
			}
			else if ( strcmp( value, "partial" ) && strcmp( value, "false" ) ) return XML_BAD_VALUE;
		}
	}

	return XML_OK;
}



/*************************************************************************/
/**  \fn      int NetEventHandler::End( )
***  \brief   Class function
**************************************************************************/

int NetEventHandler::End( )
{
	if ( data != 7 )
	{
		return XML_MISSING_ATTRIBUTE;
	}
	 
	return XML_OK;
}



/*************************************************************************/
/**  \fn      int NetworkZoneHandler::NetworkZoneHandler( "site", &xml )
***  \brief   Class constructor
**************************************************************************/

NetworkZoneHandler::NetworkZoneHandler( ) : XMLElementHandler( ".site.networking.receivepanel.transmitpanel.networkevent.zone", &lib )
{

}


/*************************************************************************/
/**  \fn      int NetworkZoneHandler::Begin( )
***  \brief   Class function
**************************************************************************/

int NetworkZoneHandler::Begin( )
{
	index = -1;
	return true;
}


/*************************************************************************/
/**  \fn      int NetworkZoneHandler::Attribute( char* att )
***  \brief   Class function
**************************************************************************/

int NetworkZoneHandler::Attribute( char* attr, char* value )
{
	if ( !strcmp( attr, "num" ) )
	{
		index = atoi( value );
	}
	else if ( xmli->receive_panel_index >= 0 && xmli->transmit_panel_index >= 0 && xmli->action_index >= 0 &&
			xmli->action_index < SITE_NUM_OUTPUT_ACTIONS && xmli->action_index != ACTION_SHIFT_FAULT &&
	  xmli->receive_panel_index < SITE_MAX_PANELS && xmli->transmit_panel_index < SITE_MAX_PANELS )
	{
		if ( index > 0 ) 
		{
			int zi = (index-1) / 32;
			int zshift = (index-1) & 31;
		
			if ( !strcmp( attr, "process" ) )
			{
				xmli->net[ xmli->transmit_panel_index ][ xmli->action_index ].display[ zi ] |= 1 << zshift;
				
				if ( !strcmp( value, "true" ) )
				{
					xmli->net[ xmli->transmit_panel_index ][ xmli->action_index ].process[ zi ] |= 1 << zshift;
				}
				data = 1;
			}
		}	
	}
	
	return XML_OK;
}		


/*************************************************************************/
/**  \fn      int NetworkZoneHandler::End( )
***  \brief   Class function
**************************************************************************/

int NetworkZoneHandler::End( )
{
	if ( data == 1 )return XML_OK;
	return XML_MISSING_DATA;
}




/*************************************************************************/
/**  \fn      int ContactHandler::ContactHandler( "site", &xml )
***  \brief   Class constructor
**************************************************************************/

ContactHandler::ContactHandler( ) : XMLElementHandler( ".site.sms.contact", &lib )
{
 
}


/*************************************************************************/
/**  \fn      int ContactHandler::Attribute( char* att )
***  \brief   Class function
**************************************************************************/

int ContactHandler::Attribute( char* attr, char* value )
{
	SWITCH( attr )
		CASE( "name" ) 
		{
			STRCHK( xmli->site->contacts[ index ].name, value, SITE_MAX_STRING_LEN );
			data  |= 1;
		}
		CASE( "number" )
		{
			STRCHK( xmli->site->contacts[ index ].number, value, SITE_MAX_PHONE_DIGITS );
			if ( *xmli->site->contacts[ index ].number == 0 ) return XML_MISSING_DATA; 
			data |= 2;
		}
		CASE( "actions" )
		{
			int a = GetActionBits( value );
			
			if (  a == -1 )
			{
				return XML_BAD_VALUE;
			}
			
			xmli->site->contacts[ index ].actions = a;
					
			data |= 4;
		}	
	 
	return XML_OK;
}


/*************************************************************************/
/**  \fn      int ContactHandler::End( )
***  \brief   Class function
**************************************************************************/

int ContactHandler::End( )
{
	if ( data == 7 )
	{
		index++;
		xmli->site->numContacts++;
		return XML_OK;
	}
	else 
	{
		return XML_MISSING_ATTRIBUTE;
	}
}


/*************************************************************************/
/**  \fn      int UserHandler::UserHandler(   )
***  \brief   Class constructor
**************************************************************************/

UserHandler::UserHandler( ) : XMLElementHandler( ".site.users.user", &lib )
{

}


/*************************************************************************/
/**  \fn      int ContactHandler::Attribute( char* att )
***  \brief   Class function
**************************************************************************/

int UserHandler::Attribute( char* attr, char* value )
{
	SWITCH( attr )
		CASE( "name" ) 
		{
			STRCHK( xmli->site->users[ index ].name, value, SITE_MAX_STRING_LEN );
			data  |= 1;
		}
		CASE( "level" )
		{
			xmli->site->users[ index ].level = atoi( value );
			
			if ( xmli->site->users[ index ].level > 1 && xmli->site->users[ index ].level < 5 )
			{
				data |= 2;
			}
			else
			{
				return XML_BAD_NUMBER_VALUE;
			}
		}
		CASE( "passcode" )
		{
			if ( strlen( value ) != 4 )
			{
				return XML_BAD_VALUE;
			}	
			
			STRCHK( xmli->site->users[ index ].pin, value, 5 );
			
			data |= 4;
		}	
		 
	return XML_OK;
}


/*************************************************************************/
/**  \fn      int UserHandler::End( )
***  \brief   Class function
**************************************************************************/

int UserHandler::End( )
{
	if ( data == 7 )
	{
		index++;
		xmli->site->numUsers++;
		return XML_OK;
	}
	else 
	{
		return XML_MISSING_ATTRIBUTE;
	}
}


/*************************************************************************/
/**  \fn      int GetTime( Time& out, char* in )
***  \brief   Class function
**************************************************************************/

static int GetTime( Time& out, char* in )
{
	int h, m;
	if ( sscanf( in, "%d:%d", &h, &m ) == 2 )
	{
		out.hour 	= h;
		out.minute 	= m;
		return XML_OK;
	}
	return XML_BAD_VALUE;
}
	


/*************************************************************************/
/**  \fn      int DayTimeHandler::DayTimeHandler( "site", &xml )
***  \brief   Class constructor
**************************************************************************/

DayTimeHandler::DayTimeHandler( ) : XMLElementHandler( ".site.panels.panel.daynight.time", &lib )
{

}


/*************************************************************************/
/**  \fn      int DayTimeHandler::Begin( )
***  \brief   Class function
**************************************************************************/

int DayTimeHandler::Begin( )
{
	index = -1;
	
	data = 0;
	
	return true;
}


/*************************************************************************/
/**  \fn      int DayTimeHandler::Attribute( char* att )
***  \brief   Class function
**************************************************************************/

int DayTimeHandler::Attribute( char* attr, char* value )
{
	SWITCH( attr )
		CASE( "day" ) 
		{
			SWITCH( value )
				CASE( "mon" ) index = 0;
				CASE( "tue" ) index = 1;
				CASE( "wed" ) index = 2;
				CASE( "thu" ) index = 3;
				CASE( "fri" ) index = 4;
				CASE( "sat" ) index = 5;
				CASE( "sun" ) index = 6;
				DEFAULT return XML_BAD_VALUE;
		}
		CASE( "start" )
		{
			if ( index != -1 )
			{
				int r = GetTime( xmli->panel->day[ index ].start, value );
				
				if ( r == XML_OK )
				{
					data |= 1;
				}
				return r;
			}
		}
		CASE( "finish" )
		{
			if ( index != -1 )
			{
				int r = GetTime( xmli->panel->day[ index ].end, value );
				
				if ( r == XML_OK )
				{
					data |= 2;
				}
				return r;
			}		
		}
	 
	return XML_OK;
}


/*************************************************************************/
/**  \fn      int DayTimeHandler::End( )
***  \brief   Class function
**************************************************************************/

int DayTimeHandler::End( )
{
	index = -1;
	
	if ( data != 3 )
	{
		return XML_MISSING_DATA;
	}
	data = 0;
		
	return XML_OK;
}





/*************************************************************************/
/**  \fn      int DeviceHandler::DeviceHandler(   )
***  \brief   Class constructor
**************************************************************************/

DeviceHandler::DeviceHandler( ) : XMLElementHandler( ".site.panels.panel.mesh.device", &lib )
{

}
 

/*************************************************************************/
/**  \fn      int DeviceHandler::Begin( )
***  \brief   Class function
**************************************************************************/

int DeviceHandler::Begin( )
{
	memset( &xmli->dev, 0, sizeof( DeviceConfig ) );
	data = 0;
	xmli->dev.type = 0xFF;
	xmli->dev.zone = 0xFF;
	xmli->dev.unit = 0xFFFF;
	xmli->dev.numInputs = 0;
	xmli->dev.numOutputs = 0;
	xmli->dev.numParams = 0;
	return true;
}


/*************************************************************************/
/**  \fn      int DeviceHandler::Attribute( char* att, char* value  )
***  \brief   Class function
**************************************************************************/

int DeviceHandler::Attribute( char* attr, char* value )
{
	SWITCH( attr )
		CASE( "location" )
		{
			STRCHK( xmli->dev.location, value, SITE_MAX_STRING_LEN );
			data |= 8;	
		}
		CASE( "combination" )
		{
			xmli->dev.type = atoi( value );
			data |= 1;
		}	
		CASE( "branding" )
 		{
 			xmli->dev.branding = atoi( value );
 			data |= 32;
 		}
		CASE( "zone" )
		{
			xmli->dev.zone = atoi( value );
			data |= 2;
		}
		CASE( "unit" )
		{
			xmli->dev.unit = atoi( value );
			data |= 4;	
		}	
		CASE( "type" )
		{
			STRCHK( xmli->dev.typestring, value, SITE_MAX_STRING_LEN );
			data |= 16;		
		}		
	return XML_OK;
}


/*************************************************************************/
/**  \fn      int DeviceHandler::End( )
***  \brief   Class function
**************************************************************************/

int DeviceHandler::End( )
{
	if ( data == 63 )
	{		
		app.DebOut( "loc:%s z:%d u:%d\n", xmli->dev.location, xmli->dev.zone, xmli->dev.unit );
		
		if ( xmli->dev.type == 0xFF || xmli->dev.zone == 0xFF || xmli->dev.unit == 0xFFFF )
		{
			return XML_BAD_NUMBER_VALUE;
		}
		
		xmli->Write( OFFSET( Site, panels[ xmli->site->numPanels ].devs[ xmli->panel->NumDevs( ) ] ), &xmli->dev, sizeof( DeviceConfig ) );
		
		//	if not Panel IO
		if ( xmli->dev.type < DEVICE_TYPE_PANEL_IO )
		{
			xmli->panel->numRadioDevs++;		 
		}
		else  
		{
			xmli->panel->numPanelDevs++;
		}
		
		data = 0;
	
		return XML_OK;
	}
	else
	{		
		return XML_MISSING_DATA;
	}		
}



/*************************************************************************/
/**  \fn      int ParamHandler::ParamHandler(   )
***  \brief   Class constructor
**************************************************************************/

ParamHandler::ParamHandler( ) : XMLElementHandler( ".site.panels.panel.mesh.device.output.params.param", &lib )
{

}
 

/*************************************************************************/
/**  \fn      int DeviceHandler::Begin( )
***  \brief   Class function
**************************************************************************/

int ParamHandler::Begin( )
{
	return true;
}


/*************************************************************************/
/**  \fn      int ParamHandler::Attribute( char* att, char* value  )
***  \brief   Class function
**************************************************************************/

int ParamHandler::Attribute( char* attr, char* value )
{
	SWITCH( attr )
		CASE( "name" )
		{
			STRCHK( xmli->dev.param[ xmli->dev.numParams ].name, value, SITE_MAX_STRING_LEN );
			data |= 8;	
		}
		CASE( "value" )
		{
			xmli->dev.param[ xmli->dev.numParams ].value = atoi( value );
			data |= 1;
		}			
		CASE( "code" )
		{
			xmli->dev.param[ xmli->dev.numParams ].code = (ParamType) atoi( value );
			data |= 2;
		}
		CASE( "min" )
		{
			xmli->dev.param[ xmli->dev.numParams ].min = atoi( value );
			data |= 4;	
		}	
		CASE( "max" )
		{
			xmli->dev.param[ xmli->dev.numParams ].max = atoi( value );
			data |= 16;		
		}		
	 	 	
	return XML_OK;
}


/*************************************************************************/
/**  \fn      int ParamHandler::End( )
***  \brief   Class function
**************************************************************************/

int ParamHandler::End( )
{
	if ( data == 31 )
	{		
		xmli->dev.param[ xmli->dev.numParams++ ].channel = xmli->dev.numOutputs;
		
		return XML_OK;
	}
	else
	{		
		return XML_MISSING_DATA;
	}		
}


/*************************************************************************/
/**  \fn      int OutputHandler::OutputHandler( "site", &xml )
***  \brief   Class constructor
**************************************************************************/

OutputHandler::OutputHandler( ) : XMLElementHandler( ".site.panels.panel.mesh.device.output", &lib )
{

}


/*************************************************************************/
/**  \fn      int OutputHandler::Attribute( char* att, char* value  )
***  \brief   Class function
**************************************************************************/

int OutputHandler::Attribute( char* attr, char* value )
{
	SWITCH( attr )
		CASE( "type" )
		{
			xmli->dev.output[ xmli->dev.numOutputs ].type = GetChannelType( value );
			
			data |= 1;
			if ( xmli->dev.output[ xmli->dev.numOutputs ].type == CO_CHANNEL_NONE_E ) return XML_BAD_VALUE;
		}
		CASE( "actions" )
		{
			data |= 2;
			int actions = GetActionBits( value ); 
			if ( actions == -1 ) return XML_BAD_VALUE;
			xmli->dev.output[ xmli->dev.numOutputs ].actions = (unsigned int) actions;
		}					
		CASE( "ignorenightdelays" ) 
		{
			int r;
			if ( GetBoolean( r, value ) )
			{
				xmli->dev.output[ xmli->dev.numOutputs ].flags |= r ? CHANNEL_OPTION_NO_NIGHT_DELAY : 0;
				data |= 4;
			}
			else return XML_BAD_VALUE;
		}
		CASE( "inverted" )
		{
			int r;
			if ( GetBoolean( r, value ) )
			{
				xmli->dev.output[ xmli->dev.numOutputs ].flags |= r ? CHANNEL_OPTION_INVERTED : 0;
				data |= 8;
			}
			else return XML_BAD_VALUE;
		}
		CASE( "delay" )
		{
			int a, b;
			char global[ 8];
			
			int vars = sscanf( (const char*) value, "%d,%d,%s", &a, &b, global );
			
			if ( vars == 2 )
			{
				xmli->dev.output[ xmli->dev.numOutputs ].delay1 = a;
				xmli->dev.output[ xmli->dev.numOutputs ].delay2 = b;
			}
			else if ( vars == 3 )
			{
				int isglob;
				
				xmli->dev.output[ xmli->dev.numOutputs ].delay1 = a;
				xmli->dev.output[ xmli->dev.numOutputs ].delay2 = b;
				
				if ( GetBoolean( isglob, global ) )
				{
					if ( isglob ) xmli->dev.output[ xmli->dev.numOutputs ].flags |= CHANNEL_OPTION_USE_GLOBAL_DELAY;
				}
				else return XML_BAD_VALUE;
			}	
			else 
			{
				return XML_BAD_VALUE;
			}
			data |= 16;
		}
		CASE( "enabled" )
		{
			int r;
			if ( GetBoolean( r, value ) )
			{
				xmli->dev.output[ xmli->dev.numOutputs ].flags |= r ? 0 : CHANNEL_OPTION_DISABLED_ANYTIME;
				data |= 32;
			}
			else return XML_BAD_VALUE;
		}	
		CASE( "silenceable" )
		{
			int r;
			if ( GetBoolean( r, value ) )
			{
				xmli->dev.output[ xmli->dev.numOutputs ].flags |= r ? CHANNEL_OPTION_SILENCEABLE : 0;
				data |= 64;
			}
			else return XML_BAD_VALUE;
		}						
		 
	return XML_OK;
}


/*************************************************************************/
/**  \fn      int OutputHandler::End( )
***  \brief   Class function
**************************************************************************/

int OutputHandler::End( )
{
	if ( data == 127 )
	{		
		xmli->dev.numOutputs++;
		return XML_OK;
	}
	else
	{
		return XML_MISSING_DATA;
	}		
}


/*************************************************************************/
/**  \fn      int InputHandler::InputHandler( "site", &xml )
***  \brief   Class constructor
**************************************************************************/

InputHandler::InputHandler( ) : XMLElementHandler( ".site.panels.panel.mesh.device.input", &lib )
{

}


/*************************************************************************/
/**  \fn      int InputHandler::Attribute( char* att, char* value  )
***  \brief   Class function
**************************************************************************/

int InputHandler::Attribute( char* attr, char* value )
{
	SWITCH( attr )
		CASE( "type" )
		{
			xmli->dev.input[ xmli->dev.numInputs ].type = GetChannelType( value );
			
			data |= 1;
			if ( xmli->dev.input[ xmli->dev.numInputs ].type == CO_CHANNEL_NONE_E ) return XML_BAD_VALUE;
		}
		CASE( "action" )
		{
			data |= 2;
			int  action = Action( value ); 
			if ( action == -1 ) return XML_BAD_VALUE;
			xmli->dev.input[ xmli->dev.numInputs ].action = action;
		}			
		CASE( "latching" )
		{
			int r;
			if ( GetBoolean( r, value ) )
			{
				xmli->dev.input[ xmli->dev.numInputs ].flags |= r ? CHANNEL_OPTION_LATCHING : 0;
				data |= 4;
			}
			else return XML_BAD_VALUE;
		}
		CASE( "ignoredelay" ) 
		{
			int r;
			if ( GetBoolean( r, value ) )
			{
				xmli->dev.input[ xmli->dev.numInputs ].flags |= r ? CHANNEL_OPTION_SKIP_OUTPUT_DELAYS : 0;
				data |= 8;
			}
			else return XML_BAD_VALUE;
		}
		CASE( "inverted" )
		{
			int r;
			if ( GetBoolean( r, value ) )
			{
				xmli->dev.input[ xmli->dev.numInputs ].flags |= r ? CHANNEL_OPTION_INVERTED : 0;
				data |= 16;
			}
			else return XML_BAD_VALUE;
		}
		CASE( "coincidenceonly" )
		{
			int r;
			if ( GetBoolean( r, value ) )
			{
				xmli->dev.input[ xmli->dev.numInputs ].flags |= r ? CHANNEL_OPTION_COINCIDENCE : 0;
				data |= 256;
			}
			else return XML_BAD_VALUE;
		}
		CASE( "delaytime" )
		{
			xmli->dev.input[ xmli->dev.numInputs ].delay = atoi( value );
			data |= 64;
		}						  	
		CASE( "enabledduringday" )
		{
			int r;
			if ( GetBoolean( r, value ) )
			{
				xmli->dev.input[ xmli->dev.numInputs ].flags |= r ? 0 : CHANNEL_OPTION_DISABLED_DAYTIME;
				data |= 128;
			}
			else return XML_BAD_VALUE;
		}						
		CASE( "enabledduringnight" )
		{
			int r;
			if ( GetBoolean( r, value ) )
			{
				xmli->dev.input[ xmli->dev.numInputs ].flags |= r ? 0 : CHANNEL_OPTION_DISABLED_NIGHTTIME;
				data |= 128;
			}
			else return XML_BAD_VALUE;
		}		
	return XML_OK;
}


/*************************************************************************/
/**  \fn      int inputHandler::End( )
***  \brief   Class function
**************************************************************************/

int InputHandler::End( )
{
	if ( data == 511 - 32 )
	{		
		xmli->dev.numInputs++;
		return XML_OK;
	}
	else
	{
		return XML_MISSING_DATA;
	}		
}


/*************************************************************************/
/**  \fn      int CalibrateHandler::CalibrateHandler( "site", &xml )
***  \brief   Class constructor
**************************************************************************/

CalibrateHandler::CalibrateHandler( ) : XMLElementHandler( ".site.panels.panel.calibrate", &lib )
{

}
 


/*************************************************************************/
/**  \fn      int CalibrateHandler::Attribute( char* att )
***  \brief   Class function
**************************************************************************/

int CalibrateHandler::Attribute( char* attr, char* value )
{
	SWITCH( attr )
		CASE( "time" )
		{
	//		int r = GetTime( xmli->panel->calibrateTime, value );
	//		if ( r == XML_OK ) data = true;
		//	return r;
		}			

 	//return XML_UNKNOWN_ATTRIBUTE;		
		return XML_OK;
}


/*************************************************************************/
/**  \fn      int CalibrateHandler::End( )
***  \brief   Class function
**************************************************************************/

int CalibrateHandler::End( )
{
	//if ( data )
	{		
		return XML_OK;
	}
//	else
	{
	//	return XML_MISSING_DATA;
	}		
}



/*************************************************************************/
/**  \fn      int ZoneHandler::ZoneHandler( "site", &xml )
***  \brief   Class constructor
**************************************************************************/

ZoneHandler::ZoneHandler( ) : XMLElementHandler( ".site.panels.panel.zones.zone", &lib )
{

}
 


/*************************************************************************/
/**  \fn      int ZoneHandler::Attribute( char* att )
***  \brief   Class function
**************************************************************************/

int ZoneHandler::Attribute( char* attr, char* value )
{
	SWITCH( attr )
		CASE( "num" )
		{
			data |= 1;
			zone.num = atoi( value );
			 
			return XML_OK;
		}	
		CASE(	"name" )
		{	
			data |= 4;		
			STRCHK( zone.name, value, sizeof( zone.name ) );
			return XML_OK;
		}
		CASE(	"enabled" )
		{
			int enabled;
			if ( GetBoolean( enabled, value ) )
			{
				data |= 2;
				zone.flags = enabled ? 0 : ZONEFLAG_DISABLED;
				return XML_OK;
			}
			else return XML_BAD_VALUE;
		}
 	return XML_OK;		
}


/*************************************************************************/
/**  \fn      int ZoneHandler::End( )
***  \brief   Class function
**************************************************************************/

int ZoneHandler::End( )
{
	if ( data >= 3  )
	{				 
		if ( !(data & 4) )
		{
			sprintf( zone.name, "Zone %d", index );
		}
		
		xmli->Write( OFFSET( Site, panels[ xmli->site->numPanels ].zone[ xmli->panel->numDefinedZones++ ] ), &zone, sizeof( Zone ) );
		 		
		return XML_OK;
	}
	else
	{
		return XML_MISSING_DATA;
	}		
}


 

/*************************************************************************/
/**  \fn      int CAEInputHandler::CAEInputHandler( "xml", &xml )
***  \brief   Class constructor
**************************************************************************/

CAEInputHandler::CAEInputHandler( ) : XMLElementHandler( ".site.panels.panel.cae.inputs.input", &lib )
{

}
 

/*************************************************************************/
/**  \fn      int CAEInputHandler::Begin( )
***  \brief   Class function
**************************************************************************/

int CAEInputHandler::Begin( )
{
	memset( &xmli->caeinput, 0, sizeof( xmli->caeinput ) );
 
	return true;
}
 

/*************************************************************************/
/**  \fn      int CAEInputHandler::Attribute( char* att, char* value  )
***  \brief   Class function
**************************************************************************/

int CAEInputHandler::Attribute( char* attr, char* value )
{
	SWITCH( attr )
		CASE( "name" )
		{
			STRCHK( xmli->caeinput.name, value, SITE_MAX_STRING_LEN );
			data |= 16;
		}
		CASE( "logic" )
		{
			if ( !strncmp( value, "and", 3 ) )
			{
				xmli->caeinput.logic = CAE_LOGIC_AND;
			}
			else if ( !strncmp( value, "or", 2 ) )
			{
				xmli->caeinput.logic = CAE_LOGIC_OR;
			}
			else if ( !strncmp( value, "anytwo", 6 ) )
			{
				xmli->caeinput.logic = CAE_LOGIC_ANY2;
			}
			else if ( !strncmp( value, "coincidence", 11 ) )
			{
				xmli->caeinput.logic = CAE_LOGIC_COINCIDENT;
			}
			else return XML_BAD_VALUE;
			data |= 1;
		}
		CASE( "dayonly" )
		{
			if ( !strncmp( value, "true", 4 ) )
			{
				xmli->caeinput.flags |= CAE_OPTION_DAYTIME_ONLY;
			}
			else if ( !strncmp( value, "false", 5 ) )
			{
				xmli->caeinput.flags &= ~CAE_OPTION_DAYTIME_ONLY;
			}
			else return XML_BAD_VALUE;
			data |= 2;
		}
		CASE( "nightonly" )
		{
			if ( !strncmp( value, "true", 4 ) )
			{
				xmli->caeinput.flags |= CAE_OPTION_NIGHTTIME_ONLY;
			}
			else if ( !strncmp( value, "false", 5 ) )
			{
				xmli->caeinput.flags &= ~CAE_OPTION_NIGHTTIME_ONLY;
			}
			else return XML_BAD_VALUE;
			data |= 4;
		}		
		CASE( "within" )
		{
			xmli->caeinput.within = atoi( value );
		
			data |= 32;
		}	
		CASE( "list" )
		{
			STRCHK( xmli->caeinput.list, value, SITE_MAX_CAE_LIST_LEN_IN );
			data |= 8;
		}
		CASE( "lightfirstdev" )
		{
			int res;
			
			if ( GetBoolean( res, value ) )
			{
				if ( res )
				{
					xmli->caeinput.flags |= CAE_OPTION_COINCIDENCE_LIGHT_FIRST;
				}
				else 
				{
					xmli->caeinput.flags &= ~CAE_OPTION_COINCIDENCE_LIGHT_FIRST;
				}
			}
			else return XML_BAD_VALUE;
			data |= 256;
		}
		CASE( "samezone" )
		{
			int res;
			
			if ( GetBoolean( res, value ) )
			{
				if ( res )
				{
					xmli->caeinput.flags |= CAE_OPTION_COINCIDENCE_SAME_ZONE;
				}
				else 
				{
					xmli->caeinput.flags &= ~CAE_OPTION_COINCIDENCE_SAME_ZONE;
				}
			}
			else return XML_BAD_VALUE;
			data |= 64;
	  	}
		CASE( "secondalarm" )
		{
			int res;
			
			if ( GetBoolean( res, value ) )
			{
				if ( res )
				{
					xmli->caeinput.flags |= CAE_OPTION_COINCIDENCE_SAME_DEVICE;
				}
				else 
				{
					xmli->caeinput.flags &= ~CAE_OPTION_COINCIDENCE_SAME_DEVICE; 
				}
			}
			else return XML_BAD_VALUE;
			data |= 128;
	  	}
	return XML_OK;
}


/*************************************************************************/
/**  \fn      int CAEInputHandler::End( )
***  \brief   Class function
**************************************************************************/

int CAEInputHandler::End( )
{
	if ( data >= 31 )
	{		
		xmli->Write( OFFSET( Site, panels[ xmli->site->numPanels ].caeInputs[ xmli->panel->numCaeInputs++ ] ), &xmli->caeinput, sizeof( CAEInput ) );
 
		return XML_OK;
	}
	else
	{
		return XML_MISSING_DATA;
	}		
}



/*************************************************************************/
/**  \fn      int CAEOutputHandler::CAEOutputHandler( "xml", &xml )
***  \brief   Class constructor
**************************************************************************/

CAEOutputHandler::CAEOutputHandler( ) : XMLElementHandler( ".site.panels.panel.cae.outputs.output", &lib )
{

}
 

/*************************************************************************/
/**  \fn      int CAEOutputHandler::Begin( )
***  \brief   Class function
**************************************************************************/

int CAEOutputHandler::Begin( )
{
	memset( &xmli->caeoutput, 0, sizeof( xmli->caeoutput ) );
 
	return true;
}
 

/*************************************************************************/
/**  \fn      int CAEOutputHandler::Attribute( char* att, char* value  )
***  \brief   Class function
**************************************************************************/

int CAEOutputHandler::Attribute( char* attr, char* value )
{
	SWITCH( attr )
		CASE( "list" )
		{
			STRCHK( xmli->caeoutput.list, value, SITE_MAX_CAE_LIST_LEN_OUT );
			data |= 1;
		}	 
		CASE( "name" )
		{
			STRCHK( xmli->caeoutput.name, value, SITE_MAX_STRING_LEN );
			data |= 2;
		}
		CASE( "effect" )
		{
			xmli->caeoutput.effect = Effect( value );
			
			if ( xmli->caeoutput.effect == CAE_NONE ) 
			{
				return XML_BAD_VALUE;
			}
			data |= 4;
		}
	 	 	
	return XML_OK;
}


/*************************************************************************/
/**  \fn      int CAEOutputHandler::End( )
***  \brief   Class function
**************************************************************************/

int CAEOutputHandler::End( )
{
	if ( data == 7 )
	{		
		xmli->Write( OFFSET( Site, panels[ xmli->site->numPanels ].caeOutputs[ xmli->panel->numCaeOutputs++ ] ), &xmli->caeoutput, sizeof( CAEOutput ) );
		
		return XML_OK;
	}
	else
	{
		return XML_MISSING_DATA;
	}		
}



/*************************************************************************/
/**  \fn      int CAERuleHandler::CAERuleHandler( "xml", &xml )
***  \brief   Class constructor
**************************************************************************/

CAERuleHandler::CAERuleHandler( ) : XMLElementHandler( ".site.panels.panel.cae.rules.rule", &lib )
{
	
}
 

/*************************************************************************/
/**  \fn      int CAERuleHandler::Begin( )
***  \brief   Class function
**************************************************************************/

int CAERuleHandler::Begin( )
{
	memset( &xmli->caerule, 0, sizeof( xmli->caerule ) );
	
	xmli->caerule.displayzone = 0;
 
	return true;
}


/*************************************************************************/
/**  \fn      int CAERuleHandler::Attribute( char* att, char* value  )
***  \brief   Class function
**************************************************************************/

int CAERuleHandler::Attribute( char* attr, char* value )
{
	SWITCH( attr )
		CASE( "name" )
		{
			STRCHK( xmli->caerule.name, value, SITE_MAX_STRING_LEN );
			data |= 1;		
		}
		CASE( "event" )
		{
			xmli->caerule.action = Action( value );
			
			if ( xmli->caerule.action != ACTION_SHIFT_CAE_OUTPUT && xmli->caerule.action >= (ActionEnum) SITE_NUM_OUTPUT_ACTIONS ) return XML_BAD_VALUE;
			if ( xmli->caerule.action == (ActionEnum) -1 ) return XML_BAD_VALUE;
			data |= 2;
	  	} 	
		CASE( "delaysecs" )
		{
			int d;
			if ( sscanf( value, "%d", &d ) == 1 )
			{
				xmli->caerule.delay = d;
				data |= 4;
			}
			else return XML_BAD_NUMBER_VALUE;
		}
		CASE( "input" )
		{
			int d;
			if ( sscanf( value, "%d", &d ) == 1 )
			{
				xmli->caerule.input = d;
				data |= 8;
			}
			else return XML_BAD_NUMBER_VALUE;
		}
		CASE( "output" )
		{
			int d;
			if ( sscanf( value, "%d", &d ) == 1 )
			{
				xmli->caerule.output = d;
				data |= 16;
			}
			 
		}
		CASE( "profile" )
		{
			ProfileEnum profile = GetProfile( value );
			
			if ( profile == PROFILE_NONE ) return XML_BAD_VALUE;
			
			xmli->caerule.profile = profile;
	 
			data |= 32;
		}
		CASE( "inversion" )
		{
			int res;
			
			if ( GetBoolean( res, value ) )
			{
				if ( res )
				{
					xmli->caerule.inverted = true;
				}
				else
				{
					xmli->caerule.inverted = false;
				}
			}
			else
			{
				return XML_BAD_VALUE;
			}
		}
		CASE( "latch" )
		{
			int res;
			
			if ( GetBoolean( res, value ) )
			{
				if ( res )
				{
					xmli->caerule.latching = true;
				}
				else
				{
					xmli->caerule.latching = false;
				}
			}
			else
			{
				return XML_BAD_VALUE;
			}
		}
 	
		CASE( "zonenumber" )
		{
			int d;
			data |= 64;
			
			if ( sscanf( value, "%d", &d ) )
			{
				xmli->caerule.displayzone = d;
			}
			else
			{
				xmli->caerule.displayzone = 0;
			}
		}
 		
	return XML_OK;
}


/*************************************************************************/
/**  \fn      int CAERuleHandler::End( )
***  \brief   Class function
**************************************************************************/

int CAERuleHandler::End( )
{
	if ( xmli->caerule.action != ACTION_SHIFT_CAE_OUTPUT ) // i.e event
	{
		if ( data >= 127 - 32 - 16 - 64 )
		{
			xmli->Write( OFFSET( Site, panels[ xmli->site->numPanels ].caeRules[ xmli->panel->numCaeRules++ ] ), &xmli->caerule, sizeof( CAERule ) );
		 
			return XML_OK;
		}
	}
	else	// specific outputs
	{
		if ( data >= 63 )
		{
			xmli->Write( OFFSET( Site, panels[ xmli->site->numPanels ].caeRules[ xmli->panel->numCaeRules++ ] ), &xmli->caerule, sizeof( CAERule ) );
		 
			return XML_OK;
		}
	}
	
	return XML_MISSING_DATA;
			
}


/*************************************************************************/
/**  \fn      TransmitPanelHandler::TransmitPanelHandler( )
***  \brief   Constructor
**************************************************************************/

TransmitPanelHandler::TransmitPanelHandler( ) : XMLElementHandler( ".site.networking.receivepanel.transmitpanel", &lib )
{
	
}


/*************************************************************************/
/**  \fn      TransmitPanelHandler::Begin( )
***  \brief   Virtual Class function
**************************************************************************/

int TransmitPanelHandler::Begin( )
{
	xmli->transmit_panel_index = -1;	
		
	return true;
}
	

/*************************************************************************/
/**  \fn      int TransmitPanelHandler::Attribute( char* name, char* value )
***  \brief   Virtual Class function
**************************************************************************/

int TransmitPanelHandler::Attribute( char* name, char* value )
{		
	if ( !strcmp( name, "name" ) )
	{
		xmli->transmit_panel_index = xmli->GetPanelIndex( value );
	}	
	
	return XML_OK;
}


/*************************************************************************/
/**  \fn      ReceivePanelHandler::ReceivePanelHandler( )
***  \brief   Constructor
**************************************************************************/

ReceivePanelHandler::ReceivePanelHandler( ) : XMLElementHandler( ".site.networking.receivepanel", &lib )
{
	
}


/*************************************************************************/
/**  \fn      int ReceivePanelHandler::Begin( )
***  \brief   Virtual Class function
**************************************************************************/

int ReceivePanelHandler::Begin( )
{
	xmli->receive_panel_index = -1;	
	
	memset( &xmli->net, 0, sizeof(  xmli->net ) );
	
	return true;
}
	

/*************************************************************************/
/**  \fn      int ReceivePanelHandler::Attribute( char* name, char* value )
***  \brief   Virtual Class function
**************************************************************************/

int ReceivePanelHandler::Attribute( char* name, char* value )
{		
	if ( !strcmp( name, "name" ) )
	{
		xmli->receive_panel_index = xmli->GetPanelIndex( value );
		
	}
	
	return XML_OK;
}
		 
 


/*************************************************************************/
/**  \fn      int ReceivePanelHandler::Attribute( char* name, char* value )
***  \brief   Virtual Class function
**************************************************************************/

int ReceivePanelHandler::End( )
{		
	if ( xmli->receive_panel_index >= 0 )
	{		
		// zWrite whole area 
		xmli->Write( OFFSET( Site, panels[ xmli->receive_panel_index ].net), &xmli->net, sizeof( xmli->net ) );
	
		return XML_OK;
	}
	else
	{
		return XML_MISSING_DATA;
	}		
}
	


static SiteHandler 				a;
static PanelHandler 				b;
static ProfileHandler 			c;
static ProfilesHandler 			d;
static ContactHandler 			e;
static UserHandler 				f;
static DayTimeHandler 			g;
static DeviceHandler 			h;
static OutputHandler 			i;
static InputHandler 				j;
static CalibrateHandler 		k;
static CAERuleHandler			o;
static CAEInputHandler			p;
static CAEOutputHandler			q;
static NetEventHandler 			r;
static NetworkZoneHandler 		s;
static TransmitPanelHandler 	t;
static ReceivePanelHandler 	u;
static ParamHandler				v;
static ZoneHandler				w;



/*************************************************************************/
/**  \fn      int ImportXML::ImportXML( )
***  \brief   Class con structor
**************************************************************************/

ImportXML::ImportXML( ) : Module( "ImportXML", 0 )
{
	
}



/*************************************************************************/
/**  \fn      int ImportXML::ImportXML( )
***  \brief   Class con structor
**************************************************************************/

int ImportXML::Init( )
{
	qspi = 			(QuadSPI*)  app.FindModule( "QuadSPI" );
 	settings	= 		(Settings*) app.FindModule( "Settings" );
 	eeprom	= 		(EEPROM*) 	app.FindModule( "EEPROM" );
	
	assert( qspi != NULL );
	assert( settings != NULL );
	assert( eeprom != NULL );
	
	writeAddr = settings->GetWriteAddress( );
	
	xmli = this;
 	xmlp = &xmlparser;
	xmlp->lib = &lib;
	 
	return true;
}


/*************************************************************************/
/**  \fn      int ImportXML::GetPanelIndex( char* name )
***  \brief   Constructor
**************************************************************************/

int ImportXML::GetPanelIndex( char* name )
{
	for( int i = 0; i < site->numPanels; i++ )
	{
		if ( !strcmp( names[ i], name ) )
		{
			return i;
		}
	}
	return -1;
}
	

/*************************************************************************/
/**  \fn      int ImportXML::Write( void* data, int size )
***  \brief   Class function
**************************************************************************/

int ImportXML::Write( int offset, void* data, int size )
{
	return qspi->WriteToFlash( (char*) data, writeAddr + offset, size );
}


/*************************************************************************/
/**  \fn      void ImportXML::Poll( )
***  \brief   Class function
**************************************************************************/

void ImportXML::Poll(  )
{
	 
}	


/*************************************************************************/
/**  \fn      int ImportXML::Receive( Message* cmd )
***  \brief   Class function
**************************************************************************/

int ImportXML::Receive( Message* )
{
	return 0;
}	


/*************************************************************************/
/**  \fn      int ImportXML::Receive( Event )
***  \brief   Class function
**************************************************************************/

int ImportXML::Receive( Event )
{
	return 0;	
}


/*************************************************************************/
/**  \fn      int ImportXML::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int ImportXML::Receive( Command* cmd )
{
	switch( cmd->type )
	{
		default:;
	}
	return CMD_ERR_UNKNOWN_CMD;
}
 

/*************************************************************************/
/**  \fn      char*  GetErrMsg( int err )
***  \brief   Class function
**************************************************************************/
/*

char* GetErrMsg( int err )
{
	switch( err )
	{
		case XML_OK								: return "XML_OK"						;
		case XML_MISMATCHED_ELEMENT 		: return "XML_MISMATCHED_ELEMENT" 	;
		case XML_UNKNOWN_ATTRIBUTE			: return "XML_UNKNOWN_ATTRIBUTE"	;
		case XML_MISSING_ATTRIBUTE			: return "XML_MISSING_ATTRIBUTE"	;
		case XML_EMPTY_ELEMENT				: return "XML_EMPTY_ELEMENT"			;
		case XML_UNEXPECTED_DATA			: return "XML_UNEXPECTED_DATA"		;
		case XML_BAD_VALUE					: return "XML_BAD_VALUE"				;
		case XML_BAD_NUMBER_VALUE			: return "XML_BAD_NUMBER_VALUE"		;
		case XML_MISSING_DATA				: return "XML_MISSING_DATA"			;
		case XML_UNEXPECTED_SYMBOL			: return "XML_UNEXPECTED_SYMBOL"	;
		case XML_INIT_FAIL				   : return "XML_INIT_FAIL"				;
	}
	return "";
}

*/
