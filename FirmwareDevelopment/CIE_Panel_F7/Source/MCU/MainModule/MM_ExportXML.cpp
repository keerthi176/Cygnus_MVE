/***************************************************************************
* File name: MM_ExportXML.cpp
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
#include <stdarg.h>

 
/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "xml.h"
#include "MM_ExportXML.h"
#include "CO_Site.h"
#include "MM_USBSerial.h"


/* Defines
**************************************************************************/



#define DEFAULT else
#define SWITCH(s) char* ___sw = s; if (0);
#define CASE(c) else if ( !strcmp( c, ___sw ) )
 

 		
/*************************************************************************/
/**  \fn      static char* GetActionsString( uint16_t bits )
***  \brief   Helper
**************************************************************************/


static char* GetActionsString( uint16_t bits )
{
	static char buff[ 128];
	
	*buff = 0;
	
	for( int bit = 0; bit <= SITE_NUM_OUTPUT_ACTIONS; bit++ )
	{
		if ( bits & ( 1 << bit ) ) 
		{
			if ( *buff != 0 ) STRNCAT( buff, ",", 128 );
			
			switch( bit )
			{	
				case ACTION_SHIFT_FIRE: 			STRNCAT( buff, "fire", 		  	128 ); break;
				case ACTION_SHIFT_FIRST_AID:		STRNCAT( buff, "first-aid", 	128 ); break;
				case ACTION_SHIFT_SECURITY:		STRNCAT( buff, "security", 	128 ); break;
				case ACTION_SHIFT_GENERAL	:		STRNCAT( buff, "general", 		128 ); break;
				case ACTION_SHIFT_EVACUATION :  	STRNCAT( buff, "evacuate", 	128 ); break;
				case ACTION_SHIFT_FAULT : 			STRNCAT( buff, "fault", 		128 ); break;
				case ACTION_SHIFT_ROUTING_ACK	: 	STRNCAT( buff, "routing-ack", 128 ); break;
				
				case SITE_NUM_OUTPUT_ACTIONS : 	STRNCAT( buff, "mesh-events", 128 ); break;	// special case for SMS
			} 
		}
	}
	
	return buff;
}

		
/*************************************************************************/
/**  \fn      static const char* ActionName( uint8_t a )
***  \brief   Helper
**************************************************************************/

const char* ActionName( uint8_t a )
{
	switch( a )
	{		
		case ACTION_SHIFT_FIRE: 			return "fire" ; 
		case ACTION_SHIFT_FIRST_AID:		return "first-aid" ; 	
		case ACTION_SHIFT_SECURITY:		return "security" ;		
		case ACTION_SHIFT_EVACUATION :  	return "evacuate" ;	
		case ACTION_SHIFT_GENERAL :  		return "general" ;		
		case ACTION_SHIFT_FAULT : 			return "fault" ;
		case ACTION_SHIFT_RESET : 			return "reset" ;
		case ACTION_SHIFT_SILENCE : 		return "silence" ;
		case ACTION_SHIFT_ROUTING_ACK	: 	return "routing-ack" ;
		case ACTION_SHIFT_UNDEFINED : 	return "undefined" ;
		case ACTION_SHIFT_CAE_OUTPUT  : 	return "output" ;
		default : return "";
	}	
}	


/*************************************************************************/
/**  \fn      sstatic char* GetProfile( ProfileEnum a )
***  \brief   Helper
**************************************************************************/

static const char* GetProfile( ProfileEnum a )
{
	switch ( a )
	{		
		case PROFILE_FIRE		 	: return "fire" ;		
	  	case PROFILE_FIRST_AID	: return "first-aid" ;
		case PROFILE_SECURITY	: return "security" ;  		
		case PROFILE_GENERAL		: return "general" ;  		
		case PROFILE_EVACUATION	: return "evacuate" ;		
		case PROFILE_FAULT		: return "fault" ; 
	 	case PROFILE_ROUTING_ACK: return "routing-ack" ;
		case PROFILE_SILENT		: return "silent" ;	
		case PROFILE_TEST			: return "test" ; 	
		default:						  return "";
	}
}	

/*************************************************************************/
/**  \fn      static ChannelType GetChannelType( char* s )
***  \brief   Helper
**************************************************************************/


static const char* GetBoolean( int r )
{
	if ( r )
	{
		return "true";
	}
	else
	{
		return "false";
	}
}

 
/*************************************************************************/
/**  \fn      static ChannelType GetChannelType( char* s )
***  \brief   Helper
**************************************************************************/


static const char* GetChannelType( ChannelType t )
{	
	switch( t )
	{
		case CO_CHANNEL_SMOKE_E 	:										return "smoke"; 		
		case CO_CHANNEL_HEAT_A1R_E   :                           return "a1rheat";	
		case CO_CHANNEL_HEAT_B_E   :                             return "heatb";
		case CO_CHANNEL_CO_E   :                               	return "co";			
		case CO_CHANNEL_PIR_E 		:										return "pir";			
		case CO_CHANNEL_SOUNDER_E  :                           	return "sounder";	
		case CO_CHANNEL_BEACON_E:                          		return "beacon";		
		case CO_CHANNEL_FIRE_CALLPOINT_E     :                  	return "fire"; 		
		case CO_CHANNEL_STATUS_INDICATOR_LED_E:             		return "led"; 			
		case CO_CHANNEL_VISUAL_INDICATOR_E:                  		return "vi";			
		case CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E:		return "soundervi"; 	
		case CO_CHANNEL_MEDICAL_CALLPOINT_E :               		return "firstaid";	
		case CO_CHANNEL_EVAC_CALLPOINT_E:                    		return "evac";	
		case CO_CHANNEL_OUTPUT_ROUTING_E:                     	return "routing";	
		case CO_CHANNEL_PANEL_ROUTING_ACK :								return "panelinput1";
		case CO_CHANNEL_PANEL_CALLPOINT :                        return "panelinput2";
		case CO_CHANNEL_PANEL_ALARM_INPUT :                      return "panelinput3";
		case CO_CHANNEL_PANEL_SILENCE_INPUT  :                   return "panelinput4";
		case CO_CHANNEL_PANEL_FAULT_INPUT :                      return "panelinput5";
		case CO_CHANNEL_PANEL_ALARM_RELAY :                      return "paneloutput1";
		case CO_CHANNEL_PANEL_FAULT_RELAY :                      return "paneloutput2";
		case CO_CHANNEL_PANEL_FIRE_RELAY :                       return "paneloutput3";
		case CO_CHANNEL_PANEL_ROUTING_OUTPUT :                   return "paneloutput4";
		case CO_CHANNEL_PANEL_SOUNDERS_RELAY :                   return "paneloutput5";
		case CO_CHANNEL_PANEL_FUNCTION_BUTTON_1:                 return "panelinput6";
		case CO_CHANNEL_PANEL_FUNCTION_BUTTON_2:                 return "panelinput7";
		case CO_CHANNEL_PANEL_EVACUATE_BUTTON :                  return "panelinput8";
		case CO_CHANNEL_SITENET_SOUNDER_E:								return "sitesounder";
			
		default:
		{
			if ( t >= CO_CHANNEL_OUTPUT_1_E && t <= CO_CHANNEL_OUTPUT_16_E )
			{
				static char buff[ 64];
				snprintf( buff, 64, "output%d", t - CO_CHANNEL_OUTPUT_1_E + 1 );
				return buff;
			}
			else if ( t >= CO_CHANNEL_INPUT_1_E && t <= CO_CHANNEL_INPUT_32_E )
			{
				static char buff[ 64];
				snprintf( buff, 64, "input%d", t - CO_CHANNEL_INPUT_1_E + 1 );
				return buff;
			}
			else 
			{
				return "";
			}
		}	
	}
}


static const char* day[ 7] = {"mon", "tue","wed","thu", "fri", "sat", "sun" };  
			

/*************************************************************************/
/**  \fn      int ExportXML::ExportXML( )
***  \brief   Class con structor
**************************************************************************/

ExportXML::ExportXML( ) : Module( "ExportXML", 0 )
{
//	Alarms* alarms = (Alarms*) app.FindModule( "Alarms" );
  // assert( settings	= 		(Settings*) app.FindModule( "Settings" ) );
//	eeprom	= 		(EEPROM*) 	app.FindModule( "EEPROM" );
	finished = true;
	usb_timeout_fault = false;
}

 



/*************************************************************************/
/**  \fn      int ExportXML::Receive( Message* cmd )
***  \brief   Class function
**************************************************************************/

int ExportXML::Receive( Message* )
{
	return 0;
}	


/*************************************************************************/
/**  \fn      int ExportXML::Receive( Event )
***  \brief   Class function
**************************************************************************/

int ExportXML::Receive( Event )
{
	return 0;	
}


/*************************************************************************/
/**  \fn      int ExportXML::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int ExportXML::Receive( Command* cmd )
{
	switch( cmd->type )
	{
		
	}
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      void ExportXML::Start( Site* s, ExportType t )
***  \brief   Class function
**************************************************************************/
 
void ExportXML::Start( Site* s, ExportType t )
{
	finished = false;
	
	prev_line = 0;
	line = 0;
	 
	site = s;
}
 
 
 
/*************************************************************************/
/**  \fn      void ExportXML::OUT( const char* fmt, ... )
***  \brief   Class function
**************************************************************************/
 
bool ExportXML::Out( const char* fmt, ... )
{
	if ( ++line == prev_line + 1 )
	{
		prev_line = line;
		line = 0;
		 
		va_list args;
		
		va_start( args, fmt );
	
		char buff[ 320 ];
			
		vsnprintf( buff, 320, fmt, args );
		
		buff[ 319 ] = 0;
		app.DebOut( buff );
		USBSerial::TxString( buff );
		
		va_end( args );
		return true;
	}
	return false;
}

 
/*************************************************************************/
/**  \fn      const char* GetLogic( Logic l )
***  \brief   Helper function
**************************************************************************/

const char* GetLogic( Logic l )
{
	switch ( l )
	{
		case CAE_LOGIC_AND  : return "and";
		case CAE_LOGIC_OR   : return "or";
		case CAE_LOGIC_ANY2 : return "anytwo";
		case CAE_LOGIC_COINCIDENT : return "coincidence";
	}
	return "";
}


/*************************************************************************/
/**  \fn      const char* GetLogic( Logic l )
***  \brief   Helper function
**************************************************************************/

const char* GetEffect( CAEAction a )
{
	switch ( a )
	{
		case CAE_ACTIVATE  : return "activate";
		case CAE_ENABLE   : return "enable";
		case CAE_DISABLE : return "disable";
		case CAE_TEST : return "test";
	}
	return "";
}


/*************************************************************************/
/**  \fn      void ExportXML::Poll( )
***  \brief   Class function
**************************************************************************/
 
void ExportXML::Poll( )
{
	
}

#define OUT(...) if ( Out( __VA_ARGS__ ) ) return;

/*************************************************************************/
/**  \fn      void ExportXML::Output( Site* s, ExportType t )
***  \brief   Class function
**************************************************************************/

void ExportXML::Output( )
{
	Site* s = site;
	
	OUT( "<site name=\"%s\" uniqueid=\"%s\" version=\"%u\" panelbatterylow=\"%u\" batterylow=\"%u\" batterymed=\"%u\" batteryhigh=\"%u\" batterylowsn=\"%u\" batterymedsn=\"%u\" batteryhighsn=\"%u\" productrange=\"%u\">\n", s->name, s->uid, s->version, s->batt_panel, 
	s->batt_low[0], s->batt_med[0], s->batt_high[0], s->batt_low[1], s->batt_med[1], s->batt_high[1], s->productrange );
	OUT( "\t<profiles>\n" );
	
	for( int n = 0; n < SITE_NUM_PROFILES; n++ )
	{
		OUT( "\t\t<profile action=\"%s\" selection=\"%d\" extsndr=\"%d\" vifr=\"%d\" ></profile>\n", GetProfile( (ProfileEnum) n ), s->profiles[ n].tone, s->profiles[ n].ext_sounder, s->profiles[ n].flash );
	}
	
	OUT( "\t</profiles>\n" );

	OUT( "\t<sms>\n" );

	for( int n = 0; n < s->numContacts; n++ )
	{
		OUT( "\t\t<contact name=\"%s\" number=\"%s\" actions=\"%s\"></contact>\n", s->contacts[ n].name, s->contacts[ n].number, GetActionsString( s->contacts[ n].actions ) );
	}
	
	OUT( "\t</sms>\n" );
	
	OUT( "\t<users>\n" );
	
	for( int n = 0; n < s->numUsers; n++ )
	{
		OUT( "\t\t<user name=\"%s\" level=\"%d\" passcode=\"%s\"></user>\n", s->users[ n].name, s->users[ n].level, s->users[ n].pin );
	}
	
	OUT( "\t</users>\n" );
	
	OUT( "\t<panels>\n" );
	
	int n;
	
	for( n = 0 ; n < s->numPanels; n++ )
	{	
		Panel* p = s->panels + n;
		
		OUT( "\t\t<panel name=\"%s\" address=\"%d\" numLEDS=\"%d\" systemID=\"%d\" startfrequency=\"%d\" resoundsamezone=\"%s\" alldelaysoff=\"%s\"\
				maxdevices=\"%d\" framelength=\"%d\" useglobaldelays=\"%s\" delay1=\"%d\" delay2=\"%d\" ignoresecuritydaytime=\"%s\" ignoresecuritynighttime=\"%s\" gsm=\"%s\" productrange=\"%u\" >\n",
				p->name, p->address, p->numZoneLeds, p->systemId, p->freq, GetBoolean( !(p->settings & SITE_SETTING_RESOUND_NEW_ZONE ) ),
				GetBoolean( p->settings & SITE_SETTING_DISABLE_OUTPUT_DELAYS ),	p->maxDevices, p->framelength, GetBoolean( p->settings & SITE_SETTING_GLOBAL_DELAY_OVERRIDE ),
				p->delay1, p->delay2,  GetBoolean( p->settings & SITE_SETTING_IGNORE_SECURITY_IN_DAY ), GetBoolean( p->settings & SITE_SETTING_IGNORE_SECURITY_AT_NIGHT ), 
				GetBoolean( p->settings & SITE_SETTING_GSM_AVAILABLE), p->productrange );
		
		OUT( "\t\t\t<zones>\n" );
		
		int z;
		for( Zone* z = p->zone; z < p->zone + p->numDefinedZones; z++ )
		{
			OUT( "\t\t\t<zone name=\"%s\" num=\"%d\" enabled=\"%s\">\n", z->name, z->num, GetBoolean( !(z->flags & ZONEFLAG_DISABLED )) );   
			OUT( "\t\t\t</zone>\n" );
		}
		OUT( "\t\t\t</zones>\n" );
			
		OUT( "\t\t\t<daynight>\n" );
		
		for( int d = 0; d < 7; d++ )
		{
			OUT( "\t\t\t\t<time day=\"%s\" start=\"%02d:%02d\" finish=\"%02d:%02d\"></time>\n", day[ d ], p->day[ d].start.hour, p->day[ d].start.minute, p->day[ d].end.hour, p->day[ d].end.minute );
		}
		
		OUT( "\t\t\t</daynight>\n" );
		
//		OUT( 7, n, z + 9, "\t\t\t<calibrate time=\"%2d:%2d\"></calibrate>\n", p->calibrateTime.hour, p->calibrateTime.minute );
		
		OUT( "\t\t\t<mesh>\n" );
		
		int dv;
		for( dv = 0; dv < p->NumDevs( ); dv++ )
		{
			DeviceConfig* d = p->devs + dv;
	
			OUT( "\t\t\t\t<device combination=\"%u\" branding=\"%u\" zone=\"%u\" unit=\"%u\" location=\"%s\" type=\"%s\">\n", d->type, d->branding, d->zone, d->unit, d->location, d->typestring );
			
			for( int i = 0; i < d->numInputs; i++ )
			{
				InputChannel* in = d->input + i;
				
				if ( in->type != CO_CHANNEL_NONE_E )
				{
						OUT( "\t\t\t\t\t<input type=\"%s\" action=\"%s\" latching=\"%s\" ignoredelay=\"%s\" inverted=\"%s\" coincidenceonly=\"%s\"  delaytime=\"%u\" enabledduringday=\"%s\" enabledduringnight=\"%s\"></input>\n",
						GetChannelType( in->type ), ActionName( in->action ), GetBoolean( in->flags & CHANNEL_OPTION_LATCHING ), GetBoolean( in->flags & CHANNEL_OPTION_SKIP_OUTPUT_DELAYS ),
						GetBoolean( in->flags & CHANNEL_OPTION_INVERTED ), GetBoolean( in->flags & CHANNEL_OPTION_COINCIDENCE ), in->delay, GetBoolean( !( in->flags & CHANNEL_OPTION_DISABLED_DAYTIME ) ), GetBoolean( !( in->flags & CHANNEL_OPTION_DISABLED_NIGHTTIME ) ) );
				}
			}
			for( int o = 0; o < d->numOutputs; o++ )
			{
				OutputChannel* out = d->output + o;
				
				if ( out->type != CO_CHANNEL_NONE_E )
				{
					OUT( "\t\t\t\t\t<output type=\"%s\" actions=\"%s\" inverted=\"%s\" silenceable=\"%s\" ignorenightdelays=\"%s\" delay=\"%u,%u\"  enabled=\"%s\"></output>\n",
						GetChannelType( out->type ), GetActionsString( out->actions ), GetBoolean( out->flags & CHANNEL_OPTION_INVERTED ), GetBoolean( out->flags & CHANNEL_OPTION_SILENCEABLE ),
						GetBoolean( out->flags & CHANNEL_OPTION_NO_NIGHT_DELAY), out->delay1, out->delay2, GetBoolean( !( out->flags & CHANNEL_OPTION_DISABLED_ANYTIME) ) );
				}
			}
			OUT( "\t\t\t\t</device>\n" );
		 
		}
		
		OUT( "\t\t\t</mesh>\n" );
		
		OUT( "\t\t\t<cae>\n" );
		OUT( "\t\t\t\t<inputs>\n" );
		
		for( CAEInput* i = p->caeInputs; i < p->caeInputs + p->numCaeInputs; i++ )
		{
			OUT( "\t\t\t\t\t<input name=\"%s\" logic=\"%s\" within =\"%d\" dayonly=\"%s\" nightonly=\"%s\" samezone=\"%s\" secondalarm=\"%s\" list=\"%s\"></input>\n",
				i->name, GetLogic( i->logic ), i->within, GetBoolean( i->flags & CAE_OPTION_DAYTIME_ONLY ), GetBoolean( i->flags & CAE_OPTION_NIGHTTIME_ONLY ), GetBoolean( i->flags & CAE_OPTION_COINCIDENCE_SAME_ZONE ),
				GetBoolean( i->flags & CAE_OPTION_COINCIDENCE_SAME_DEVICE ), i->list );
		}
		
		OUT( "\t\t\t\t</inputs>\n" );
		OUT( "\t\t\t\t<outputs>\n" );
		
		
		for( CAEOutput* o = p->caeOutputs; o < p->caeOutputs + p->numCaeOutputs; o++ )
		{
			OUT( "\t\t\t\t\t<output name=\"%s\" effect=\"%s\" list=\"%s\"></output>\n",
				o->name, GetEffect( o->effect ), o->list );
		}
		OUT( "\t\t\t\t</outputs>\n" );
		OUT( "\t\t\t\t<rules>\n" );
		for( CAERule* r = p->caeRules; r < p->caeRules + p->numCaeRules; r++ )
		{
			OUT( "\t\t\t\t\t<rule name=\"%s\" event=\"%s\" delaysecs=\"%d\" input=\"%d\" output=\"%d\" latch=\"%s\" ></rule>\n",
				r->name, ActionName( r->action ), r->delay, r->input, r->output, GetBoolean( r->latching ) );
		}
		OUT("\t\t\t\t</rules>\n" );
		
		OUT("\t\t\t</cae>\n" );
		
		
//		OUT( "\t\t\t<panel-io>\n" );
//		
//		for( int n = 0; n < p->numPanelDevs; n++ )
//		{
//			DeviceConfig* d = p->devs + n;
//		
//			if ( d->unit >= SITE_PANEL_INPUT_UNIT_START )
//			{
//				OUT( "\t\t\t\t<device combination=\"%u\" zone=\"%u\" unit=\"%u\" location=\"%s\">\n", d->type, d->zone, d->unit, d->location );
//				
//				for( int i = 0; i < d->numInputs; i++ )
//				{
//					InputChannel* in = d->input + i;
//					
//					if ( in->type != CO_CHANNEL_NONE_E )
//					{
//						OUT( "\t\t\t\t\t<input type=\"%s\" action=\"%s\" latching=\"%s\" ignoredelay=\"%s\" inverted=\"%s\" delaytime=\"%u\" enabledday=\"%s\" enablednight=\"%s\"></input>\n",
//							GetChannelType( in->type ), Action( in->action ), GetBoolean( in->flags & CHANNEL_OPTION_LATCHING ), GetBoolean( in->flags & CHANNEL_OPTION_IGNORE_DELAY ),
//							GetBoolean( in->flags & CHANNEL_OPTION_INVERTED ), in->delay, GetBoolean( !( in->flags & CHANNEL_OPTION_DISABLED_DAYTIME ) ), GetBoolean( !( in->flags & CHANNEL_OPTION_DISABLED_NIGHTTIME ) ) );
//					}
//				}
//				for( int o = 0; o < d->numInputs; o++ )
//				{
//					OutputChannel* out = d->output + o;
//					
//					if ( out->type != CO_CHANNEL_NONE_E )
//					{
//							OUT( "\t\t\t\t\t<output type=\"%s\" actions=\"%s\" inverted=\"%s\" silenceable=\"%s\" ignorenightdelays=\"%s\" delay=\"%u,%u\"  enabled=\"%s\"></output>\n",
//							GetChannelType( out->type ), Action( out->actions ), GetBoolean( out->flags & CHANNEL_OPTION_INVERTED ), GetBoolean( out->flags & CHANNEL_OPTION_SILENCEABLE ),
//							GetBoolean( out->flags & CHANNEL_OPTION_IGNORE_DELAY), out->delay1, out->delay2, GetBoolean( !( out->flags & ( CHANNEL_OPTION_DISABLED_DAYTIME | CHANNEL_OPTION_DISABLED_NIGHTTIME ) ) ) );
//					}
//				}
//				OUT( "\t\t\t\t</device>\n" );
//			}
//		}
//		
//		OUT( "\t\t\t</panel-io>\n" );		
	
	
		OUT( "\t\t</panel>\n" );		
	}	
	OUT( "\t</panels>\n" );	
		
	OUT( "</site>\n" );
		
	if ( Out( "\r\n" ) ) finished = true;
}		
