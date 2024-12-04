/***************************************************************************
* File name: MM_CauseAndEffect.cpp
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
* Cause And Effect control module
*
**************************************************************************/

//////////////////////////// UNFINISHED ///////////////////////////////////


/* System Include Files
**************************************************************************/
#include <limits.h>
#include <assert.h>


/* User Include Files
**************************************************************************/
#include "MM_CauseAndEffect.h"
#include "MM_device.h"
#include "MM_CUtils.h"
#include "MM_Fault.h"
#include "MM_Buzzer.h"


 
#define MAX_ITERATIONS 5

 

#define GETZONE( array, zone ) (( array[ (zone-1) >> 5 ] >> ((zone-1) & 31 ) ) & 1 ) 
#define SETZONE( array, zone ) array[ (zone-1) >> 5 ] |= 1 << ((zone-1) & 31 );


struct Coincidence
{
	time_t latestDeAssertion[ SITE_MAX_ZONES ];
	time_t earliestAssertion[ SITE_MAX_ZONES ];	
	time_t secondEarliestAssertion[ SITE_MAX_ZONES ];
	time_t secondLatestDeAssertion[ SITE_MAX_ZONES ];
	Device* latestDevice[ SITE_MAX_ZONES ];
	Device* earliestDevice[ SITE_MAX_ZONES ];
	Device* countdev[ SITE_MAX_ZONES ];
	short count[ SITE_MAX_ZONES ];
};

static int 			global_setsilenceabled;
static Device* 	global_devs;
static int			global_numdevs;
static int			global_iterations;
static int			global_outputchanges;
static Rule*		global_rule;

int	global_flagchanges;

static int 			gotresult;
static Coincidence co;
static NCU* ncu;



 
bool IsOutput( ChannelType chantype )
{	
	switch( chantype )
	{
		case CO_CHANNEL_SOUNDER_E:									 
		case CO_CHANNEL_BEACON_E:										 
		case CO_CHANNEL_STATUS_INDICATOR_LED_E:  				 
		case CO_CHANNEL_VISUAL_INDICATOR_E:						 
		case CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E:	 
		case CO_CHANNEL_OUTPUT_ROUTING_E:
		case CO_CHANNEL_OUTPUT_1_E:		 
		case CO_CHANNEL_OUTPUT_2_E:
		case CO_CHANNEL_OUTPUT_3_E:
		case CO_CHANNEL_OUTPUT_4_E:
		case CO_CHANNEL_OUTPUT_5_E:
		case CO_CHANNEL_OUTPUT_6_E:
		case CO_CHANNEL_OUTPUT_7_E:
		case CO_CHANNEL_OUTPUT_8_E:
		case CO_CHANNEL_OUTPUT_9_E:
		case CO_CHANNEL_OUTPUT_10_E:
		case CO_CHANNEL_OUTPUT_11_E:   
		case CO_CHANNEL_OUTPUT_12_E:
		case CO_CHANNEL_OUTPUT_13_E:
		case CO_CHANNEL_OUTPUT_14_E:
		case CO_CHANNEL_OUTPUT_15_E:
		case CO_CHANNEL_OUTPUT_16_E:

		case CO_CHANNEL_PANEL_ALARM_RELAY  :
		case CO_CHANNEL_PANEL_FAULT_RELAY  :
		case CO_CHANNEL_PANEL_FIRE_RELAY  :
		case CO_CHANNEL_PANEL_ROUTING_OUTPUT  :
		case CO_CHANNEL_PANEL_SOUNDERS_RELAY : return true;
		
		default: return false;
	}
}


int GetInputChannel( Device* dev, int type )
{
	if ( dev == NULL ) return -1;
	
	for( int chan = 0; chan < dev->config->numInputs; chan++ )
	{
		if ( dev->config->input[ chan ].type == type ) 
		{
			return chan;
		}
	}
	return -1;
}	

int GetOutputChannel( Device* dev, int type )
{
	if ( dev == NULL ) return -1;
	
	for( int chan = 0; chan < dev->config->numOutputs; chan++ )
	{
		if ( dev->config->output[ chan ].type == type ) 
		{
			return chan;
		}
	}
	return -1;
}	
 

/*************************************************************************/
/**  \fn      const char* GetActionString( int action )
***  \brief   Helper function
**************************************************************************/

const char* GetActionString( int action )
{
	const char* event = NULL;
	
	switch ( action )
	{
		case ACTION_SHIFT_FIRE      : event = "Fire";
												break;
		case ACTION_SHIFT_FIRST_AID : event = "First Aid";
												break;
		case ACTION_SHIFT_SECURITY  : event = "Security";
												break;
		case ACTION_SHIFT_GENERAL    : event = "General";
												break;
		case ACTION_SHIFT_EVACUATION: event = "Evacuate";
												break;
		case ACTION_SHIFT_FAULT : 		event = "Fault";
												break;
		case ACTION_SHIFT_ROUTING_ACK : 	event = "Routing Ack";
												break;
	}		
	return event;
}

	
/*************************************************************************/
/**  \fn      uint16_t Panel::NumDevs( )
***  \brief   Class function
**************************************************************************/

uint16_t Panel::NumDevs( )
{
	return numPanelDevs + numRadioDevs;
}	




enum ItemType
{
	ITEM_ZONE,
	ITEM_UNIT,
	ITEM_CHANNEL,
	ITEM_LIST,
};

	
struct ListParser
{
	char* sp;
	
	ListParser( char* str ) : sp( str ) { if ( sp != NULL ) sp--; }
			
	int Next( ItemType& type, int& index, int& devindex )
	{
		if ( sp == NULL ) return false;
		
		switch ( *++sp )
		{
			case 'u' : type = ITEM_UNIT;	break;
			case 'z' : type = ITEM_ZONE;	break;
			case 'l' : type = ITEM_LIST;	break;
			case 'c' : type = ITEM_CHANNEL; break;
			default: return false;		
		}
		if ( sscanf( ++sp, "%d", &index ) == 1)
		{
			if ( type == ITEM_CHANNEL )
			{
				sp = strchr( sp, 'u' );
				if ( sp != NULL )
				{
					if ( sscanf( ++sp, "%d", &devindex ) != 1)
					{
						return false;
					}
				}
				else return false;
			}
						
			sp = strchr( sp, ',' );
			return true;
		}
		return false;
	}
};	


Device* FindUnit( int unit  )
{
	for( Device* d = global_devs; d < global_devs + global_numdevs; d++ )
	{
		if ( d->config->unit == unit ) return d;
	}
	return NULL;
}


static void GetChannelLogic( CAEInput* i, Device* dev, int chan, int& result )
{
	int state = false;
	
	if ( dev == NULL )
	{
		result = false;
		// TODO: create fault
		return;
	}
			
	if ( dev->IsInputChannel( chan ) )
	{		 	
		gotresult = true;
				
		// is device live  		
		if ( dev->IsInputValid( chan ) )
		{
			state = true;
			
			// is it flagged for co-incidence
			if ( dev->config->input[ chan ].flags & CHANNEL_OPTION_COINCIDENCE ) 
			{
				if ( !( global_rule->flags & RULE_ACTIVE ) )
				if ( dev->timeAsserted[ chan ] + i->within * 60 < now( ) )
				{
					app.DebOut( "Timeout!\n" );
					if ( dev->flags[ chan ] & INPUT_LATCHED )
					{
						dev->flags[ chan ] &= ~INPUT_LATCHED;
						dev->flags[ chan ] &= ~INPUT_ASSERTED;
						app.DebOut( "Unlatching chan %d unit %d.\n", chan, dev->config->unit ); 
					}
					else if ( dev->flags[ chan ] & INPUT_ASSERTED )
					{
						dev->InputDeAssert( chan, now( ), 0 );
						dev->flags[ chan ] &= ~INPUT_LATCHED;
						ncu->QueueWriteMsg( false, NCU_EVENT_FAULT_RESET, NULL, NULL, dev->config->zone, dev->config->unit, 1 ); 	// 1 = EVENTS ONLY
						app.DebOut( "Resettings chan %d unit %d.\n", chan, dev->config->unit );
					}	
					return;		
				}
					
				// do we light a zone?
				if ( i->flags & CAE_OPTION_COINCIDENCE_LIGHT_FIRST || global_rule->flags & RULE_ACTIVE )
				{
					// flag to light zone
					dev->settings |= SETTING_RULED_LIGHT_ZONE;
				}
			}
	 
			// if coincidence and fire
			if ( i->logic == CAE_LOGIC_COINCIDENT && dev->config->input[ chan ].action == ACTION_SHIFT_FIRE )
			{
				int z = 0;
				
				if ( i->flags & CAE_OPTION_COINCIDENCE_SAME_ZONE )
				{
					z = dev->config->zone - 1;
					if ( z < 0 ) return;
				}
				
				 /*
				if ( ! GETZONE( global_rule->zone, dev->config->zone ) )
				{
					global_rule->flags &= ~RULE_SILENCED;
					
					SETZONE( global_rule->zone, dev->config->zone );
					
					if ( global_rule->flags & RULE_TRUE ) 
					{
						Buzzer::Buzz( BUZZ_FOR_FIRE );
					}
				}*/

				 
				if ( co.countdev[ z] != dev || ( i->flags & CAE_OPTION_COINCIDENCE_SAME_DEVICE ) )
				{
					co.count[ z]++;
					co.countdev[ z] = dev;
					app.DebOut( "Including zone %d chan %d unit %d (%d).\n", z, chan, dev->config->unit, co.count[ z] );
				}
				if ( i->flags & CAE_OPTION_COINCIDENCE_SAME_DEVICE )
				{  
					// include knock 1 too !
					if ( dev->prevAssertion[ chan ] > now( ) - i->within * 60 )
					{
						co.count[ z]++;
						app.DebOut( "Including early zone %d chan %d unit %d in rule. (%d)\n", z, chan, dev->config->unit, co.count[ z] );
					}
				}
			}
		}	 
		
		if ( i->logic == CAE_LOGIC_AND )
		{	 
			if ( !state ) result = false;
		}
		else if ( i->logic == CAE_LOGIC_OR )
		{
			if ( state ) result = true;
		}
		else if ( i->logic == CAE_LOGIC_ANY2 )
		{
			if ( state )
			{
				result++;
			}
		}
	}
}


static void GetDeviceLogic( CAEInput* i, int unit, int& result )
{	
	Device* dev = FindUnit( unit );

	if ( dev == NULL )
	{
		result = false;
		// TODO: create fault
		return;
	}
	
	for( int chan = 0; chan < dev->config->numInputs; chan++ )
	{
		if ( dev->IsInputChannel( chan ) )
		{			
			GetChannelLogic( i, dev, chan, result );
		}
	}
}


static void GetZoneLogic( CAEInput* i, int zone, int& result )
{
	for( Device* d = global_devs; d < global_devs + global_numdevs; d++ )
	{
		if ( d->config->zone == zone )
		{
			GetDeviceLogic( i, d->config->unit, result );
		}
	}	
}
			

static int GetLogic( CAEInput* inputs, int input_num )
{
	ListParser parser( inputs[ input_num ].list );
	ItemType type;
	int index, devindex;
	
	if ( ++global_iterations > MAX_ITERATIONS )
	{
		Fault::AddFault( FAULT_CAE_ENGINE );
		return false;
	}
	
	// set intial results
	int result = false;

	// current input
	CAEInput* i = inputs + input_num;
	
	if ( i->logic == CAE_LOGIC_AND ) result = true;
	
	// iterate through list
	while( parser.Next( type, index, devindex ) )
	{		
		switch ( type )
		{
			case ITEM_CHANNEL:
			{
				Device* dev = FindUnit( devindex );
				
				if ( dev == NULL ) return false;
				
				int chanindex = GetInputChannel( dev, index );	// convert channel type to index
					 
				if ( chanindex == -1 ) return false;
				
				GetChannelLogic( i, dev, chanindex, result );
					
				break;
			}
			case ITEM_UNIT:
			{
				GetDeviceLogic( i, index, result );
				break;	
			}
			case ITEM_LIST:
			{
				int subresult = GetLogic( inputs, index );
				switch( i->logic )
				{
					case CAE_LOGIC_COINCIDENT  : break;
					case CAE_LOGIC_ANY2 			: result += subresult; break;
					case CAE_LOGIC_OR   			: if ( subresult ) result = true; break;
					case CAE_LOGIC_AND   		: if ( !subresult ) result = false; break;
				}
				
				break;	
			}
			case ITEM_ZONE:
			{
				GetZoneLogic( i, index, result );
				break;
			}
			default:
			{
				return false;
			}
		}
	}
	
	return result;
	
}

 
static void SetChannelLogic( int chan, bool isOutput, Device* dev, CAEAction effect, int profile, int istrue )
{
	if ( dev == NULL )
	{
		// TODO: create fault
		return;
	}

	if ( effect == CAE_ACTIVATE )
	{	
		if ( dev->IsOutputChannel( chan ) )
		{	
			if ( istrue )
			{
				if ( dev->IsSilenceable( chan ) )
				{
					if ( global_setsilenceabled )
					{
						if ( profile < dev->profile[ chan ] )
						{	
							dev->profile[ chan ] = profile;
						}
					}
				}
				else
				{
					if ( profile < dev->profile[ chan ] )
					{
						dev->profile[ chan ] = profile;
					}
				}
			}
		
			global_outputchanges |=  1 << profile; 
		}
	}
	else if ( effect == CAE_DISABLE && !istrue ) 
	{		
		if ( isOutput && dev->IsOutputChannel( chan ) )
		{	
			/*  FEATURE DISABLED
			
			if ( dev->flags[ chan ] & OUTPUT_RULED_DISABLED )
			{
				dev->flags[ chan ] &= ~OUTPUT_RULED_DISABLED;
				global_flagchanges |= CAE_DISABLEMENTS;
				
				DeviceConfig*  devconf = dev->config;
				
				int flags = devconf->output[ chan ].flags & CHANNEL_OPTION_INVERTED     ? 1 : 0;
				flags |=    devconf->output[ chan ].flags & CHANNEL_OPTION_NO_NIGHT_DELAY ? 2 : 0;
				flags |=    devconf->output[ chan ].flags & CHANNEL_OPTION_SILENCEABLE  ? 4 : 0;
				flags |=    devconf->output[ chan ].flags & CHANNEL_OPTION_DISABLED_ANYTIME ? 0 : 8 + 16;
				
		//		ggrr - need to decide
				
		//		ncu->QueueWriteMsg( true, NCU_CONFIG_OUTPUT_FLAGS, this, CheckUpdate, pnew->zone, pnew->unit, channel, 8 + 16  ); // { night and day
			}
			*/
		}
 
		else if ( !isOutput && dev->IsInputChannel( chan ) )
		{	
			if ( dev->flags[ chan ] & INPUT_RULED_DISABLED )
			{
				dev->flags[ chan ] &= ~INPUT_RULED_DISABLED; 
				global_flagchanges |= CAE_DISABLEMENTS;
			}
		}		
	}
	else if ( effect == CAE_DISABLE && istrue ) 
	{	
		if ( isOutput && dev->IsOutputChannel( chan ) )
		{	
			if ( !(dev->flags[ chan ] & OUTPUT_RULED_DISABLED ) )
			{
				dev->flags[ chan ] |= OUTPUT_RULED_DISABLED;
				global_flagchanges |= CAE_DISABLEMENTS;
			}
		}
		else if ( !isOutput && dev->IsInputChannel( chan ) )
		{	
			if ( !( dev->flags[ chan ] & INPUT_RULED_DISABLED ) )
			{
				dev->flags[ chan ] |= INPUT_RULED_DISABLED;
				global_flagchanges |= CAE_DISABLEMENTS;
			}
		}		
	}
}


static void SetDeviceLogic( int unit, CAEAction action, int profile, int istrue )
{	
	Device* dev = FindUnit( unit );
	
	if ( dev == NULL )
	{
		// TODO: create fault ?
		return;
	}	
	
	if ( action == CAE_DISABLE )
	{
		for( int chan = 0; chan < dev->config->numInputs; chan++ )
		{
			if ( dev->IsInputChannel( chan ) )
			{
				SetChannelLogic( chan, false, dev, action, profile, istrue );		
			}		
		}		
	}
	else 
	{
		for( int chan = 0; chan < dev->config->numOutputs; chan++ )
		{
			if ( dev->IsOutputChannel( chan ) )
			{
				SetChannelLogic( chan, true, dev, action, profile, istrue );		
			}		
		}
	}
}


static void SetZoneLogic( int zone, CAEAction action, int profile, int istrue )
{
	if ( action == CAE_DISABLE )
	{
		// let devices know..
		
		ncu->QueueWriteMsg( true, NCU_BROADCAST_ENABLE_ZONE, NULL, NULL, zone, !istrue );
	}
	for( Device* d = global_devs; d < global_devs + global_numdevs; d++ )
	{
		if ( d->config->zone == zone )
		{
			SetDeviceLogic(  d->config->unit, action, profile, istrue );
		}
	}	
}
			

static void SetLogic( CAEOutput* outputs, int output_num, int profile, int istrue )
{
	ListParser parser( outputs[ output_num ].list );
	ItemType type;
	int index, devindex;
	
	if ( ++global_iterations > MAX_ITERATIONS )
	{
		Fault::AddFault( FAULT_CAE_ENGINE );
		return;
		
	}
	// iterate through list
	while( parser.Next( type, index, devindex ) )
	{		
		switch ( type )
		{
			case ITEM_CHANNEL:
			{		
				Device* dev	= FindUnit( devindex );
				
				if ( dev == NULL ) return;
				
				bool isOutput = IsOutput( (ChannelType) index );
				
				int chanindex;
				if ( isOutput )
					chanindex = GetOutputChannel( dev, index );
				else
					chanindex = GetInputChannel( dev, index );
								
				if ( chanindex != -1 )
					SetChannelLogic( chanindex, isOutput, dev, outputs[ output_num ].effect, profile, istrue );
				break;
			}
			case ITEM_UNIT:
			{
				SetDeviceLogic( index, outputs[ output_num ].effect, profile, istrue );
				break;	
			}
			case ITEM_LIST:
			{
				SetLogic( outputs, index, profile, istrue );
		 		break;	
			}
			case ITEM_ZONE:
			{
				SetZoneLogic( index, outputs[ output_num ].effect, profile, istrue );	
				break;
			}
			default:
			{
				return;
			}		 
		}
	}		
}


/*************************************************************************/
/**  \fn      CauseAndEffect::CauseAndEffect( )
***  \brief   Class constructor
**************************************************************************/

CauseAndEffect::CauseAndEffect( ) : Module( "CauseAndEffect", 0 )
{
	
}	


/*************************************************************************/
/**  \fn      int CauseAndEffect::Init( )
***  \brief   Class initializer
**************************************************************************/

int CauseAndEffect::Init( ) 
{
	ncu = (NCU*) app.FindModule( "NCU" );
	
	return ncu != NULL;
}	


/*************************************************************************/
/**  \fn      CAERule::GetInputStatus( Device* list, int numDevs )
***  \brief   Class function
**************************************************************************/

int CauseAndEffect::GetInputStatus( int input, IOStatus* iostatus, Device* devs, int numDevs, CAEInput* inputs, ActionEnum action, Rule* r )
{
	global_devs = devs;
	global_numdevs = numDevs;
	global_iterations = 0;
	global_rule = r;
 
	gotresult = false;
	
	CAEInput* i = inputs + input; 
	
	
	// Check day/night
	if ( ( i->flags & CAE_OPTION_DAYTIME_ONLY )   && !RealTimeClock::daytime ) return false;
	if ( ( i->flags & CAE_OPTION_NIGHTTIME_ONLY ) &&  RealTimeClock::daytime ) return false;
	
	
	for( int zi = 0; zi < SITE_MAX_ZONES; zi++ )
	{
		co.countdev[ zi] = NULL;
		co.count[ zi] = 0;
		
		// if not same bothered with same zone 
		if ( !(inputs[ input ].flags & CAE_OPTION_COINCIDENCE_SAME_ZONE ) )
		{
			// we're done
			break;
		}
	}
	
	
	int result = GetLogic( inputs, input );
		
	switch ( i->logic )
	{
		case CAE_LOGIC_AND			: if ( !gotresult ) return false; else return result;
		case CAE_LOGIC_OR				: return result;
		case CAE_LOGIC_ANY2			: if ( result > 1 ) return true;
		case CAE_LOGIC_COINCIDENT	:
		{		
			for( int z = 0; z < SITE_MAX_ZONES; z++ )
			{
				// if count 2 or more we have our confirmation..
				if ( co.count[ z] > 1 ) return true;
				
				// if not same bothered with same zone 
				if ( !(inputs[ input ].flags & CAE_OPTION_COINCIDENCE_SAME_ZONE ) )
				{
					// we're done
					break;
				}
			}
		}
	}
	return false;
}
	

/*************************************************************************/
/**  \fn      CAERule::SetOutputStatus( Device* list, int numDevs, bool setsilenceabled )
***  \brief   Class function
**************************************************************************/

int CauseAndEffect::SetOutputStatus( CAERule* rule, Device* devs, int numDevs, CAEOutput* outputs, bool setsilenceabled, int istrue )
{
	global_devs = devs;
	global_numdevs = numDevs;
	global_setsilenceabled = setsilenceabled;
	global_iterations = 0;
	global_outputchanges = 0;
	 

	SetLogic( outputs, rule->output, rule->profile, istrue );
		
	return global_outputchanges;
}
	
