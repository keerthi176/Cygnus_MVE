/***************************************************************************
* File name: MM_IOStatus.cpp
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
* IO Status
*
**************************************************************************/

#ifndef _MM_IO_STATUS_H_
#define _MM_IO_STATUS_H_


/* System Include Files
**************************************************************************/
#include <list>
#include "rtc.h"
#include <time.h>


/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_Application.h"
#include "MM_Settings.h"
#include "MM_Device.h"
#include "MM_IOStatus.h"
#include "MM_CUtils.h"
#include "MM_Buzzer.h"
#include "MM_Fault.h"
#include "MM_Log.h"
#include "MM_LED.h"
#include "MM_EmbeddedWizard.h"
#include "MM_CauseAndEffect.h"
#include "CO_Site.h"
#include "CO_Device.h"

#include "MM_Evacuate.h"
#include "CO_Device.h"
#include "MM_PanelIO.h"

#include "MM_TouchScreen.h"

#include "App.h"
#include "MM_Utilities.h"

#ifdef GSM_HTTP			
#include "MM_AppSupport.h"
#else
#include "MM_MQTTSupport.h"
#endif	

#include <stdio.h>

/* Defines
**************************************************************************/

#define PANEL_IO_INPUT_BASE 	0x800
#define PANEL_IO_OUTPUT_BASE	0x900



#define TEST_SYSTEM		0
#define TEST_LOCAL		1
#define TEST_SILENT		2

#define TEST_PULSE_LENGTH  35

#define SET( z, buff ) buff[ (z-1)/32] |= 1 << ((z-1)&31)
#define IFSET( z, buff ) ((buff[ (z-1)/32] >> ((z-1)&31)) & 1)


Rule  IOStatus::rule[ SITE_MAX_CAE_RULES ];

short IOStatus::_zoneflags[ SITE_MAX_PANELS ][ SITE_MAX_ZONES + 2 ] = { 0 };
short (* IOStatus::zoneflags) [ SITE_MAX_ZONES + 2 ] = (short(*) [ SITE_MAX_ZONES + 2 ]) ((short*)_zoneflags + 1 ) ;
 
int   IOStatus::test_everything = 0;
int   IOStatus::prev_unsilenceable_status = 0;

time_t IOStatus::last_ontest_activity = PREVENT_BLANK;
time_t IOStatus::last_event_activity = PREVENT_BLANK;
time_t IOStatus::last_disablement_activity = PREVENT_BLANK;
time_t IOStatus::last_fire_activity = PREVENT_BLANK;


Device* IOStatus::device_list = (Device*) DEVICE_BUFFER_ADDR;
Device* IOStatus::device_list_pos = (Device*) DEVICE_BUFFER_ADDR;


/*************************************************************************/
/**  \fn      IOStatus::IOStatus( )
***  \brief   Constructor for class
**************************************************************************/

IOStatus::IOStatus( ) : Module( "IOStatus", 5, EVENT_CLEAR | EVENT_SILENCE_BUTTON | EVENT_RESOUND_BUTTON | EVENT_ACKNOWLEDGE_FIRE_BUTTON | EVENT_RESET |
									   	EVENT_FIRE_CLEAR | EVENT_CONFIRM_FIRE_BUTTON | EVENT_CONFIRM_EVENT_BUTTON | EVENT_ACKNOWLEDGE_EVENT_BUTTON ),
									      test_profile_bit( 0 ), resound_enabled( false )													  
{
	Receive( EVENT_CLEAR );
	silence_timer = UINT_MAX - 120 * 60;
}


/*************************************************************************/
/**  \fn      int IOStatus::Init( )
***  \brief   Constructor for class
**************************************************************************/

int IOStatus::Init( )
{
	gpios    = 	(GPIOManager*)    app.FindModule( "GPIOManager" );
	settings = 	(Settings*)		   app.FindModule( "Settings" );
	rtc      =  (RealTimeClock*)  app.FindModule( "RealTimeClock" );
 	gsm      =  (GSM*)			   app.FindModule( "GSM" );
	cae		=  (CauseAndEffect*) app.FindModule( "CauseAndEffect" );
	network  =  (PanelNetwork*)   app.FindModule( "PanelNetwork" );
	ncu		=  (NCU*)				app.FindModule( "NCU" );
	ew			=  (EmbeddedWizard*) app.FindModule( "EmbeddedWizard" );
	
	
	assert( gpios 		!= NULL );
	assert( settings 	!= NULL );
	assert( rtc 		!= NULL );
	assert( gsm 		!= NULL );	
	assert( cae 		!= NULL );	
	assert( network 	!= NULL );	
	assert( ncu		 	!= NULL );
	assert( ew   	 	!= NULL );
	
	prev_broadcast_silenceable_status   = 0;
	prev_broadcast_unsilenceable_status = 0;
	prev_broadcast_skipdelays_status    = 0;
	
	return true;
}

 
/*************************************************************************/
/**  \fn      InputDevice* IOStatus::FindDevice( void* id )
***  \brief   Class function
**************************************************************************/
 
Device* IOStatus::FindDevice( void* ptr )
{	
	for( Device* i = device_list; i < device_list_pos; i++ )
	{
		if ( ptr == i->ptr )
		{
			return i;
		}
	}
	return NULL;
}
 

/*************************************************************************/
/**  \fn      void IOStatus::ClearDevices(  )
***  \brief   Class function
**************************************************************************/

void IOStatus::ClearDevices( )
{	
	device_list_pos = device_list; 
}


/*************************************************************************/
/**  \fn      InputDevice* IOStatus::FindDevice( int zone, int unit )
***  \brief   Class function
**************************************************************************/

Device* IOStatus::FindDevice( int zone, int unit )
{  
	for( Device* i = device_list; i < device_list_pos; i++ )
	{
      if ( i->config->zone == zone && i->config->unit == unit )
      {
         return i;
      }
   }
   return NULL;
}


/*************************************************************************/
/**  \fn      InputDevice* IOStatus::FindDevice( int unit )
***  \brief   Class function
**************************************************************************/

Device* IOStatus::FindDevice( int unit )
{  
	for( Device* i = device_list; i < device_list_pos; i++ )
	{
      if ( i->config->unit == unit )
      {
         return i;
      }
   }
   return NULL;
}


/*************************************************************************/
/**  \fn      Device* IOStatus::FindDevice( DeviceConfig* conf )
***  \brief   Class function
**************************************************************************/


Device* IOStatus::FindDevice( DeviceConfig* conf )
{  
	for( Device* i = device_list; i < device_list_pos; i++ )
	{
      if ( i->config == conf )
      {
         return i;
      }
   }
   return NULL;
}


/*************************************************************************/
/**  \fn      void IOStatus::RegisterDevice( Device* od )
***  \brief   Class function
**************************************************************************/

Device* IOStatus::RegisterDevice( Device* od )
{
	assert( od != NULL );
	
	if ( device_list_pos < device_list + SITE_MAX_DEVICES )
	{	
		memcpy( device_list_pos, od, sizeof( Device ) );	
	
		return device_list_pos++;
	}
	return NULL;
}	


/*************************************************************************/
/**  \fn      void IOStatus::RemoveDevice( Device* od )
***  \brief   Class function
**************************************************************************/

void IOStatus::RemoveDevice( Device* od )
{
	if ( od != NULL )
	{
		int found = false;
		
		for( Device* d = device_list; d < device_list_pos; d++ )
		{
			if ( d == od ) found = true;
				 
			if ( found )
			{
				memcpy( d, d + 1, sizeof( Device ) );
			}
		}
		
		if ( found ) device_list_pos--;
	}	
}
 
/*************************************************************************/
/**  \fn      char* GetChanType( int type )
***  \brief   Static function
**************************************************************************/

const char* GetChanType( int type )
{
   switch ( type )
   {
  	   case AppChannelTypeNone : return "";								
      case AppChannelTypeSmoke : return "Smoke";				
      case AppChannelTypeHeatB : return "Heat B";				
      case AppChannelTypeCO : return "CO";					
      case AppChannelTypePIR : return "PIR";								
      case AppChannelTypeSounder : return "Sounder";							
      case AppChannelTypeBeacon : return "Beacon";								
      case AppChannelTypeFireCallpoint : return "Fire Callpoint";					
      case AppChannelTypeStatus_indicator_led : return "lED";			
      case AppChannelTypeVisualIndicator : return "Visual Indicator";
      case AppChannelTypeSounderVisualIndicatorCombined : return "Sounder/VI";	
      case AppChannelTypeMedicalCallpoint : return "First-Aid Callpoint";
      case AppChannelTypeEvacCallpoint : return "Evac Callpoint";
      case AppChannelTypeOutputRouting : return "Output Routing";
      case AppChannelTypeHeatA1R : return "Heat A1R";
              
      case AppChannelTypePanelAlarmInput : return "Alarm Input";
      case AppChannelTypePanelAlarmRelay : return "Alarm Relay";
      case AppChannelTypePanelCallpoint  : return "Callpoint Input";
      case AppChannelTypePanelFaultInput : return "Fault Input";
      case AppChannelTypePanelFaultRelay : return "Fault Relay";
      case AppChannelTypePanelFireRelay : return "Fire Relay";
      case AppChannelTypePanelRoutingAck : return "Routing Ack Input";
      case AppChannelTypePanelRoutingOutput: return "Routing Output";
      case AppChannelTypePanelSilenceInput: return "Silence Input";
      case AppChannelTypePanelSoundersRelay: return "Sounders Relay";
      case AppChannelTypePanelFunction1: return "Function 1";
      case AppChannelTypePanelFunction2: return "Function 2";
      case AppChannelTypePanelEvacuate: return "Evacuate";


	   default :
      {
           if ( type <= AppChannelTypeInput32 &&
                type >= AppChannelTypeInput1 )
           {
				  static char bf[ 12];
				  snprintf( bf, 12, "Channel %d", ( type + 1 ) -  AppChannelTypeInput1 );
              return bf;
           }
           if ( type <= AppChannelTypeOutput16 &&
                 type >= AppChannelTypeOutput1 )
           {
              static char bf[ 12];
				  snprintf( bf, 12, "Channel %d", ( type + 1 ) -  AppChannelTypeOutput1 );
              return bf;
           }
       }
    }
	return "";
}

#ifdef CREATE_DEVICE_NAME

/*************************************************************************/
/**  \fn      char* GetChanType( int type )
***  \brief   Static function
**************************************************************************/

const char* GetChanAbbrev( int type )
{
   switch ( type )
   {
  	   case AppChannelTypeNone : return "";								
      case AppChannelTypeSmoke : return "Smk";				
      case AppChannelTypeHeatB : return "HB";				
      case AppChannelTypeCO : return "CO";					
      case AppChannelTypePIR : return "PIR";								
      case AppChannelTypeSounder : return "Snd";							
      case AppChannelTypeBeacon : return "Bcn";								
      case AppChannelTypeFireCallpoint : return "Fire";					
      case AppChannelTypeStatus_indicator_led : return "";			
      case AppChannelTypeVisualIndicator : return "VI";
      case AppChannelTypeSounderVisualIndicatorCombined : return "SndVi";	
      case AppChannelTypeMedicalCallpoint : return "FA";
      case AppChannelTypeEvacCallpoint : return "Evac";
      case AppChannelTypeOutputRouting : return "Rout";
      case AppChannelTypeHeatA1R : return "A1R";
          
	   default :
      {
           if ( type <= AppChannelTypeInput32 &&
                type >= AppChannelTypeInput1 )
           {
				  static char bf[ 12];
				  snprintf( bf, 12, "I%d", ( type + 1 ) -  AppChannelTypeInput1 );
              return bf;
           }
           if ( type <= AppChannelTypeOutput32 &&
                 type >= AppChannelTypeOutput1 )
           {
              static char bf[ 12];
				  snprintf( bf, 12, "O%d", ( type + 1 ) -  AppChannelTypeOutput1 );
              return bf;
           }
       }
    }
	return "";
}


void IOStatus::CreateName( DeviceConfig* dc, char* buff, int len )
{
	int n = 0;
	
	for( InputChannel* i = dc->input; i < dc->input + dc->numInputs; i++ )
	{
		strncat( buff, GetChanAbbrev( i->type ), len );
	}
	for( OutputChannel* o = dc->output; o < dc->output + dc->numOutputs; o++ )
	{
		strncat( buff, GetChanAbbrev( o->type ), len );
	}
}

#endif

/*************************************************************************/
/**  \fn      int IOStatus::ResetInputs( )
***  \brief   Class function
**************************************************************************/

void IOStatus::ResetInputs( )
{
	// now clear zone action state
	memset( zonebits, 0, sizeof(zonebits) );
	
	// clear new fire flag
	new_zone_in_fire = false;
}
  

/*************************************************************************/
/**  \fn      int IOStatus::RefreshInputs( )
***  \brief   Class function
**************************************************************************/

int IOStatus::RefreshInputs( int& silenceable_action_bits, int& unsilenceable_action_bits, int& skip_delay_bits )
{
	// Reset stats
	int listchanged = 0;
	int activated_test_profile = 0;
	
	if ( app.panel != NULL && app.site != NULL )
	
	// for each input
	for( Device* id = device_list; id < device_list_pos; id++ )
	{
		// each channel
		for( int channel = 0; channel < id->config->numInputs; channel++ )
		{
			// if activated under test
			if ( id->flags[ channel ] & INPUT_ACTIVE_ON_TEST )
			{	
				// if we havent 'noticed' it yet
				if ( ! ( id->flags[ channel ] & INPUT_NOTICED_ON_TEST ) )
				{
					// flag that we have noticed it
					id->flags[ channel ] |= INPUT_NOTICED_ON_TEST;
					
					// if we havent activated the test _profile
					if ( !activated_test_profile )
					{
						// if ttest profile
						if ( test_profile_bit == TEST_SYSTEM )
						{
							// broadcast audible tone
							ncu->QueueWriteMsg( true, NCU_BROADCAST_TEST_TONE, this, NULL, 1 );
							
							// sound panel too..
							Device* paneldev = IOStatus::FindDevice( PanelIO::panelio_config );
				
							if ( paneldev != NULL )
							{
								for( int ch = 0; ch < PanelIO::panelio_config->numOutputs; ch++ )
								{
									if ( PanelIO::panelio_config->output[ ch ].type == CO_CHANNEL_PANEL_SOUNDERS_RELAY )
									{
										if ( !paneldev->IsOutputDisabled( ch ) )
										{
											paneldev->OutputChange( ch, ACTION_SHIFT_TEST_PROF, 1 ); 
										}
									}
								}
							}
						}
						else if ( test_profile_bit == TEST_LOCAL )
						{
							// each output channel
							for( int outchan = 0; outchan < id->config->numOutputs; outchan++ )
							{
								if ( id->config->output[ outchan ].type == CO_CHANNEL_SOUNDER_E ||
										id->config->output[ outchan ].type == CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E ||
										id->config->output[ outchan ].type == CO_CHANNEL_BEACON_E ||
										id->config->output[ outchan ].type == CO_CHANNEL_VISUAL_INDICATOR_E || 
										id->config->output[ outchan ].type == CO_CHANNEL_PANEL_SOUNDERS_RELAY )
								{ 
									id->OutputChange( outchan, ACTION_SHIFT_TEST_PROF, 1 ); 
								}																		
							}
						}
						activated_test_profile = true;
					}	
				}
			}
			else
			{
				// get input action
				int input_action = id->InputAction( channel );
							
				// filter unwanted events
				if ( input_action == ACTION_SHIFT_SECURITY )
				{
					// if security switched off day or nighttime
					if ( ( ( app.panel->settings & SITE_SETTING_IGNORE_SECURITY_AT_NIGHT ) && !RealTimeClock::daytime ) ||
						( ( ( app.panel->settings & SITE_SETTING_IGNORE_SECURITY_IN_DAY ) && RealTimeClock::daytime ) ) )
					{
						 continue;
					}
				}
							
				int actionbit = 1 << input_action;		
				
				// if not valid
				if ( !id->IsInputValid( channel ) )
				{
					// but was active 
					if ( id->flags[ channel ] & INPUT_ACTIVE )
					{	
						// flag change
						listchanged |= actionbit;
						
						// flag it unactive
						id->flags[ channel ] &= ~INPUT_ACTIVE;
											
						// if input generated a fault
						if ( id->flags[ channel ] & INPUT_IN_FAULTS )
						{
							// Remove the fault
							Fault::RemoveFault( FAULT_EVENT_TRIGGERED, id->config->zone, id->config->unit, id->config->input[ channel ].type );
							
							// Remove flag
							id->flags[ channel ] &= ~INPUT_IN_FAULTS;
						}
					}
				}
				else // if valid
				{				
					// but was inactive							
					if ( ( id->flags[ channel ] & INPUT_ACTIVE ) == 0 )
					{
						// flag as active
						id->flags[ channel ] |= INPUT_ACTIVE;
						
						// flag list as changed
						listchanged |= actionbit;
						
						// if action is Fault
						if ( actionbit == ACTION_BIT_FAULT )
						{
							// and not flagged as in fault
							if ( ( id->flags[ channel] & INPUT_IN_FAULTS ) == 0 )
							{
								// add to faults
								Fault::AddFault( FAULT_EVENT_TRIGGERED, id->config->zone, id->config->unit, id->config->input[ channel ].type, id->IsLatched( channel ) );
								
								// flag as so
								id->flags[ channel] |= INPUT_IN_FAULTS;
							}
						}
						// if entire zones can be silenced
						if ( app.panel->settings & SITE_SETTING_RESOUND_NEW_ZONE )
						{					
							// if zone has been silenced then auto silence 
							if ( ( zoneflags[ app.site->currentIndex ][ id->config->zone ] & ZONE_IS_SILENCED ) )
							{
								id->flags[ channel ] |= INPUT_SILENCED;
							}
						}
						
						const char* ttxt;
						LogType logtype = GetLogType( input_action, &ttxt );
									
						Log::Msg( logtype, "Zone %d, unit %d %s '%s' active (%s).", id->config->zone, id->config->unit, GetChanType( id->config->input[ channel ].type ), id->config->location, ttxt );
					}
					
					// if action is fire
					if ( actionbit == ACTION_BIT_FIRE )
					{
						// if not in fire
						if ( ! ( zoneflags[ app.site->currentIndex ][ id->config->zone ] & ZONE_LED_IN_FIRE ) )
						{
							// if not coincidence only
							if ( ( ( id->config->input[ channel ].flags & CHANNEL_OPTION_COINCIDENCE ) == 0 ) ||
								( id->settings & SETTING_RULED_LIGHT_ZONE ) )
							{
								// flag zone in fire
								zoneflags[ app.site->currentIndex ][ id->config->zone ] |= ZONE_LED_IN_FIRE;
								new_zone_in_fire = true;
							 
							}
						}
					}	
					
					// flag input action ( per zone ) for networking
					if ( input_action < SITE_NUM_OUTPUT_ACTIONS )
					{
						int zone_index = ( id->config->zone - 1 ) / 32;
						int zone_shift = ( id->config->zone - 1 ) & 31;
						
						zonebits[ input_action ][ zone_index ] |= 1 << zone_shift;
					}					
				
					// if not coincidence only
					if ( ( id->config->input[ channel ].flags & CHANNEL_OPTION_COINCIDENCE ) == 0 )
					{
						// does input skip output delays?
						if ( id->IsInputSkippingOutputDelays( channel ) )
						{
							// flag it
							skip_delay_bits |= actionbit;
						}			
	 
						// if input isnt silenced
						if ( ( id->flags[ channel ] & INPUT_SILENCED ) == 0 )
						{
							// include input action for silencable devices
							silenceable_action_bits |= actionbit;
						}
						// include in other devices
						unsilenceable_action_bits |= actionbit;	 
					}
				}
			}
		}
	}
	
	return listchanged;	
}


/*************************************************************************/
/**  \fn      int IOStatus::GetEventStatus( )
***  \brief   Class function
**************************************************************************/

int IOStatus::GetEventStatus( )
{
	return prev_unsilenceable_status;
}


/*************************************************************************/
/**  \fn      void IOStatus::ProcessNetworkStatus( int silenceable_status, int unsilencable_status, int& skip_status )
***  \brief   Class function
**************************************************************************/

int IOStatus::ProcessNetworkStatus( int& silenceable_status, int& unsilenceable_status, int& skip_status )
{
	static uint32_t prev_processzones[ SITE_MAX_PANELS ][ SITE_NUM_OUTPUT_ACTIONS ][ SITE_MAX_ZONES/32 ] = { 0 };
	
	if ( network != NULL )
	{
		// for each panel
		for( int pan = 0; pan < SITE_MAX_PANELS; pan++ )
		{
			for( TimedDeviceRef* tr = netAlertList[ pan ]; tr < netAlertList[ pan ] + MAX_NET_REFS; tr++ )
			{
				if ( tr->index == ALERT_UNUSED )
				{
					break;
				}
				
				int action = tr->action;
				
				int shift = 0;
				int set = 0;
				
				// We dont want to go into Fire/vent just cos CAE switched something
				
				if ( tr->type == TYPE_CAE )
				{
					CAERule* r = app.site->panels[ pan ].caeRules + tr->index;
					 
					if ( r->action == ACTION_SHIFT_CAE_OUTPUT ) continue;
				}
				
				
				if ( tr->dispzone > 0 && tr->dispzone <= SITE_MAX_ZONES )
				{
					shift = ( tr->dispzone - 1 ) & 31;
					set = ( tr->dispzone - 1 ) >> 5;
				}
			 
				// if zone isnt masked out
				if (  tr->dispzone == 0 || ( ( app.panel->net[ pan ][ tr->action ].process[ set ] >> shift ) & 1 ) )
				{
					int actionbit = 1 << tr->action;
					
					if ( tr->type != TYPE_CAE )
					{
						DeviceConfig* dc = app.site->panels[ pan ].devs + tr->index;
						
						InputChannel* ic = dc->input + tr->chan;
						
						if ( ic->flags & CHANNEL_OPTION_SKIP_OUTPUT_DELAYS )
						{
							skip_status |= actionbit;
						}
					}
					
					// if input isnt silenced
					if ( !tr->silenced )
					{
						// include input action for silencable devices
						silenceable_status |= actionbit;
					}
					// include in other devices
					unsilenceable_status |= actionbit;	 
				}
			}			
		}
	}
	
	// we dont know if changes here so we leave it to network module to indicate changes.. 
	return 0;
}


extern int global_flagchanges;
extern const char* GetActionString( int action );

/*************************************************************************/
/**  \fn      void IOStatus::CauseAndEffect( int silenceable_status, int unsilencable_status )
***  \brief   Class function
**************************************************************************/

int IOStatus::ProcessCauseAndEffect( int& silenceable_status, int& unsilenceable_status, int& output_changes )
{
	// Cause And Effect !!!
	
	int n = 0;
	int input_changes = 0;
	
	global_flagchanges = 0;
	
	if ( app.panel != NULL && app.site != NULL )
	
	// for each rule
	for( CAERule* r = app.panel->caeRules; r < app.panel->caeRules + app.panel->numCaeRules; r++, n++ )
	{
		int status = 0;
		
		// if not dirct output
		if ( r->action != ACTION_SHIFT_CAE_OUTPUT )
		{
			// generate status 
			status = 1 << r->action;			 
		}
		
		int rule_result = !!( cae->GetInputStatus( r->input, this, device_list, device_list_pos - device_list, app.panel->caeInputs,
									r->action == ACTION_SHIFT_CAE_OUTPUT ? (ActionEnum)r->profile : r->action, rule + n ) );
		
		if ( r->inverted ) rule_result = !rule_result;
		
		// If rule = TRUE
		if ( rule_result )
		{
			// how many seconds since year 2000 I wonder?
			time_t _now = now( );
			
			// if rule was previously FALSE?
			if ( !( rule[ n].flags & RULE_TRUE ) )
			{
				rule[ n].time = _now;				 // timestamp rule
				rule[ n].flags |= RULE_TRUE;		 // remember its true now
				rule[ n].flags &= ~RULE_SILENCED; // reset silenced state
			}
		
			// If delay has passed
			if ( _now >= rule[ n].time + r->delay )
			{
				// if rule previously unactive
				if ( !( rule[ n].flags & RULE_ACTIVE ) )
				{
					rule[ n].flags |= RULE_ACTIVE ;
					
					app.DebOut( "Rule %s now active!", r->name );
					
					input_changes |= status;				
					 
					if ( r->action == ACTION_SHIFT_CAE_OUTPUT )
					{
						CAEOutput* o = app.panel->caeOutputs + r->output;
						Log::Msg( LOG_FIRE, "C&E Rule %d '%s' activated '%s' event.", n + 1, r->name, GetActionString( r->action ) );
						
						if ( o->effect == CAE_ACTIVATE )
						{
							input_changes |= ( 1 << r->profile );
						}
						
						else if ( o->effect == CAE_DISABLE )
						{
							cae->SetOutputStatus( r, device_list, device_list_pos - device_list, app.panel->caeOutputs, false, true );
						}
					}
					// if action is fire.
					else if ( r->action == ACTION_SHIFT_FIRE )
					{
						Log::Msg( LOG_FIRE, "C&E Rule %d '%s' activated.", n + 1, r->name );
					}
					
					// if action is fault.
					else if ( r->action == ACTION_SHIFT_FAULT )
					{
						Log::Msg( LOG_FIRE, "C&E Rule %d '%s' activated '%s' event.", n + 1, r->name, GetActionString( r->action ) );
					}			
				}
			}
		}
		else // rule false
		{
			rule[ n].flags &= ~RULE_TRUE;
			
			// clear zone bits
			rule[ n].zone[ 0] = 0;
			rule[ n].zone[ 1] = 0;
			rule[ n].zone[ 2] = 0;
			
			if ( rule[ n].flags & RULE_ACTIVE )
			{				
				if ( r->latching )
				{
					rule[ n].flags |= RULE_LATCHED;
				}
				else
				{
					if ( r->action == ACTION_SHIFT_CAE_OUTPUT )
					{
						CAEOutput* o = app.panel->caeOutputs + r->output;
						
						if ( o->effect == CAE_DISABLE )
						{
							cae->SetOutputStatus( r, device_list, device_list_pos - device_list, app.panel->caeOutputs, false, false );
						}
						else if ( o->effect == CAE_ACTIVATE )
						{
							input_changes |= ( 1 << r->profile );
						}
					}
					
					input_changes |= status;
					rule[ n].flags = 0;	// reset flags
				}
			}
		}		
				
				
		if ( rule[ n].flags & RULE_ACTIVE )	
		{	
			if ( r->action == ACTION_SHIFT_CAE_OUTPUT )
			{
				CAEOutput* o = app.panel->caeOutputs + r->output;
				
				if ( o->effect == CAE_ACTIVATE )
				{
					output_changes |= cae->SetOutputStatus( r, device_list, device_list_pos - device_list, app.panel->caeOutputs, !( rule[ n].flags & RULE_SILENCED ), true );
				}
			}
			else
			{
				// append status
				unsilenceable_status |= status;
				
				// If rule aint silenced
				if ( !( rule[ n].flags & RULE_SILENCED ) )
				{
					// append to silencable
					silenceable_status |= status;
				}
					 
				// if zone if displayed
				if ( r->displayzone > 0 && r->displayzone <= SITE_MAX_ZONES )
				{
					// if action is fire
					if ( r->action == ACTION_SHIFT_FIRE )
					{
						// if not flaged in fire
						if ( ! ( zoneflags[ app.site->currentIndex ][ r->displayzone ] & ZONE_LED_IN_FIRE ) )
						{
							/// indicate zone is in fire
							zoneflags[ app.site->currentIndex ][ r->displayzone ] |= ZONE_LED_IN_FIRE;
							new_zone_in_fire = true;
						}
					}
				
					int zindex = r->displayzone - 1;
					
					// flag input action ( per zone ) for networking
					if (  r->action < SITE_NUM_OUTPUT_ACTIONS )
					{
						int zone_index = zindex / 32;
						int zone_shift = zindex & 31;
						
						zonebits[ r->action ][ zone_index ] |= 1 << zone_shift;
					}	
				}
			
			}			
		}		
	}
	
	if ( global_flagchanges & CAE_DISABLEMENTS ) 
	{
		Send( EW_UPDATE_DISABLEMENTS_MSG, ew );	
	}
		
	return input_changes;
}		


	
/*************************************************************************/
/**  \fn      int IOStatus::RefreshOutputStatus( int silenceable_status, int unsilencable_status, int skip_outputdelays, int& output_changes )
***  \brief   Class function
**************************************************************************/

int IOStatus::RefreshOutputStatus( int silenceable_status, int unsilencable_status, int skip_outputdelays, int& output_changes, bool broadcast )
{
	int delays = 0;
	
	if ( app.panel != NULL )
	
	// for each assertable output
	for( Device* o = device_list; o < device_list_pos; o++ )
	{
		// if broadcast config
		if ( !!o->Broadcasted( ) == !!broadcast )
		{
			// for each output channel on device
			for( int ch = 0; ch < o->config->numOutputs; ch++ )
			{
				// if channel is enabled  
				if ( !o->IsOutputDisabled( ch ) ) 
				{
					// get output action config
					int action_bits = o->OutputActions( ch );
					
					// choose status based on output type
					int status = o->IsSilenceable( ch ) ? silenceable_status : unsilencable_status;
							
					// if no actions match status 
					if ( ( action_bits & status ) == 0 )
					{	
					 
						// try next channel - there's nothing else to do
						continue;
					}
									
					// get first delay value
					unsigned int first = o->config->InitialDelay( ch );
					
					// get current time
					time_t _now = now( );
					
					int delayed_bits = ( action_bits & status ) & ~skip_outputdelays;
					int skipped_bits = ( action_bits & status ) &  skip_outputdelays;
					
					// if all delays off.. 
					if ( app.panel->settings & SITE_SETTING_DISABLE_OUTPUT_DELAYS )
					{
						// skip um all
						skipped_bits = action_bits & status;
					}					
					
					// for each action ( in order of priority ) 
					for( int bitpos = 0; bitpos < SITE_NUM_OUTPUT_ACTIONS; bitpos++ )
					{
						// if action_bit delay is skipped 
						if ( ( 1 << bitpos ) & skipped_bits )
						{	
							// if higher profile
							if ( bitpos	< o->profile[ ch ] )
							{	
								// set new profile
								o->profile[ ch ] = bitpos;				
								
								// if different from current profile
								if ( o->current_profile[ ch ] != bitpos )
								{
									// flag the change
									output_changes |= ( 1 << bitpos );
								}
							}
							// forget other action events - this is higher priority
							break;
						}
						// else if delayed
						else if ( ( 1 << bitpos ) & delayed_bits )
						{			
							// if not acknowledged (or cant be acknowledged - e.g no second delay)
							if ( ( bitpos == ACTION_SHIFT_FIRE ? ack_fire_time : ack_event_time ) <= unsilenceable_assert_time[ bitpos ] || o->config->InvestigativeDelay( ch ) == 0 )
							{
								// if delay time has passed - or user has skipped delay
								if ( ( unsilenceable_assert_time[ bitpos ] + o->config->InitialDelay( ch )
										<= _now ) 
										// or confirm pressed since
										|| ( bitpos == ACTION_SHIFT_FIRE ? confirm_fire_time : confirm_event_time ) > unsilenceable_assert_time[ bitpos ] )
								{
									// activate output profile ( if higherpriority )
									if ( bitpos	< o->profile[ ch ] )
									{
										// set new profile
										o->profile[ ch ] = bitpos;	
										
										// if different from current profile
										if ( o->current_profile[ ch ] != bitpos )
										{
											// flag the change
											output_changes |= ( 1 << bitpos );
										}
									}
									
									break;
								}
								else
								{
									// flag delays is active
									delays |= 1 << bitpos;
									
									// if there is a second delay, this one can be acknowledged
									if (  o->config->InvestigativeDelay( ch ) )
									{
										// flag is acknowledgeable
										delays |= 1 << ( bitpos + SITE_NUM_OUTPUT_ACTIONS );
									}
								}
							}
							else	// if achnowledged
							{
								unsigned int period = o->config->InvestigativeDelay( ch );							
								
								//app.DebOut( "Acked time2 = %d\n", (assert_time[ bitpos ] + total )
									//	- now );
								
								// if second delay time has been exeeded - 
								if ( ( ( bitpos == ACTION_SHIFT_FIRE ? ack_fire_time : ack_event_time ) + period <= _now ) 
									// or confirm pressed since
										|| ( bitpos == ACTION_SHIFT_FIRE ? confirm_fire_time : confirm_event_time ) > unsilenceable_assert_time[ bitpos ] )											 
								{
									// activate output profile (if higherpriority)
									if ( bitpos	< o->profile[ ch ] )
									{
										// if different from current profile
										if ( o->current_profile[ ch ] != bitpos )
										{
											// set new profile
											o->profile[ ch ] = bitpos;
											// flag the change
											output_changes |= ( 1 << bitpos );
										}
									}
									
									// reset acknowledge
									ack_fire_time = 0;
									 
									break;
								}
								else
								{
									// delay still active
									delays |= 1 << bitpos;
								}
							}
						}
					}
				}	
			}				
		}
	}			
	return delays;
}


void IOStatus::HandleDelays( int delays )
{
	static int confirm_button = 0;
	static int confirm_events_button = 0;
	
	static int ack_button = 0;
	static int ack_event_button = 0;
	
	static int current_delays = 0;
	
	if ( delays )
	{
		if ( !current_delays )
		{			
			LED::Flash( GPIO_DelaysActive );
			current_delays = true;
		}
	}
	else if (current_delays )
	{
		
		LED::On( GPIO_DelaysActive );
		current_delays = false;
	}
 
	if ( delays & ACTION_BIT_FIRE )
	{
		if ( !confirm_button )
		{
			AppDeviceClass__TriggerConfirmFireAvailable( app.DeviceObject, 1 ); 
			confirm_button = 1;
		}
	}
	else if ( confirm_button )
	{
		AppDeviceClass__TriggerConfirmFireAvailable( app.DeviceObject, 0 ); 
		confirm_button = 0;
	}
	
	if ( delays & ACTION_BITS_OTHER_EVENTS )
	{
		if ( !confirm_events_button )
		{
			AppDeviceClass__TriggerConfirmEventAvailable( app.DeviceObject, 1 ); 
			confirm_events_button = 1;
		}
	}
	else if ( confirm_events_button )
	{
		AppDeviceClass__TriggerConfirmEventAvailable( app.DeviceObject, 0 ); 
		confirm_events_button = 0;
	}
	
	
	
	if ( ( delays >> SITE_NUM_OUTPUT_ACTIONS ) & ACTION_BIT_FIRE )
	{
		if ( !ack_button )
		{
			AppDeviceClass__TriggerAcknowledgeFireAvailable( app.DeviceObject, 1 ); 
			ack_button = 1;
		}
	}
	else if ( ack_button )
	{
		AppDeviceClass__TriggerAcknowledgeFireAvailable( app.DeviceObject, 0 ); 
		ack_button = 0;
	}
	
		
	if ( ( delays >> SITE_NUM_OUTPUT_ACTIONS ) & ACTION_BITS_OTHER_EVENTS )
	{
		if ( !ack_event_button )
		{
			AppDeviceClass__TriggerAcknowledgeEventAvailable( app.DeviceObject, 1 ); 
			ack_event_button = 1;
		}
	}
	else if ( ack_event_button )
	{
		AppDeviceClass__TriggerAcknowledgeEventAvailable( app.DeviceObject, 0 ); 
		ack_event_button = 0;
	}
}			

static char output[] = {
   CO_CHANNEL_SOUNDER_E,										
   CO_CHANNEL_BEACON_E,										
   CO_CHANNEL_VISUAL_INDICATOR_E,							
   CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E,		
   CO_CHANNEL_OUTPUT_ROUTING_E,  
   CO_CHANNEL_OUTPUT_1_E,		// 46
   CO_CHANNEL_OUTPUT_2_E,
   CO_CHANNEL_OUTPUT_3_E,
   CO_CHANNEL_OUTPUT_4_E,
   CO_CHANNEL_OUTPUT_5_E,
   CO_CHANNEL_OUTPUT_6_E,
   CO_CHANNEL_OUTPUT_7_E,
   CO_CHANNEL_OUTPUT_8_E,
   CO_CHANNEL_OUTPUT_9_E,
   CO_CHANNEL_OUTPUT_10_E,
   CO_CHANNEL_OUTPUT_11_E,   // 56
   CO_CHANNEL_OUTPUT_12_E,
   CO_CHANNEL_OUTPUT_13_E,
   CO_CHANNEL_OUTPUT_14_E,
   CO_CHANNEL_OUTPUT_15_E,
   CO_CHANNEL_OUTPUT_16_E };
	
	
#define ISSET 		128
#define CANCEL 	64
#define DIFFERENT	32
#define PROFILES  31	
	
/*************************************************************************/
/**  \fn      void IOStatus::ActivateIndividualOutputs( )
***  \brief   Class function
**************************************************************************/

void IOStatus::ActivateIndividualOutputs( )
{	
	// optimize zone outputs
	unsigned char zone[ SITE_MAX_ZONES + 1 ][ CO_CHANNEL_HEAT_A1R_E ];
	
	memset( zone, 0, sizeof(zone) );
	 
	// for each assertable output
	for( Device* o = device_list; o < device_list_pos; o++ )
	{
		// for each output channel
		for( int ch = 0; ch < o->config->numOutputs; ch++ )
		{
			if ( o->config->output[ ch ].type >= CO_CHANNEL_SOUNDER_E && o->config->output[ ch ].type < CO_CHANNEL_HEAT_A1R_E )
			{
				// if chan not disabled
				if ( !o->IsOutputDisabled( ch ) )
				{			
					unsigned char* z = zone[ o->config->zone ] + o->config->output[ ch ].type;
					
					if ( o->profile[ ch ] != o->current_profile[ ch ] ) *z |= DIFFERENT;
					
					// if zone not set
					if ( !( *z & ISSET ) )
					{
						// set to profile
						*z |= ISSET | ( o->profile[ ch ] & PROFILES );
					}
					else  
					{	
						// if different
						if ( ( *z & PROFILES ) != ( o->profile[ ch ] & PROFILES ) )
						{
							// cancel zone optimization 
							*z |= CANCEL;
						}
					}			
				}
			}
		}
	}		
						

	for( int i = 1; i < SITE_MAX_ZONES + 1; i++ )	
	{
		for( int c = CO_CHANNEL_SOUNDER_E; c < CO_CHANNEL_HEAT_A1R_E; c++ )
		{
			unsigned char* z = zone[ i] + c;
			
			// if differences in zone
			if ( *z & DIFFERENT )
			{				
				// if set and not cancelled
				if ( ( *z & ISSET ) && !( *z & CANCEL ) )
				{
					int prof = *z & PROFILES;
					
					// if profile = NONE
					if ( prof == PROFILES )
					{
						// switch off zone/channel
						ncu->QueueWriteMsgFast( true, NCU_ACTIVATE, ncu, NULL, i, 4095, c, 0, 0, 0 );		
					}
					else
					{
						// switch on zone/channel
						ncu->QueueWriteMsgFast( true, NCU_ACTIVATE, ncu, NULL, i, 4095, c, prof, 1, 0 );	
					}
					
					// flag as set
					for( Device* o = device_list; o < device_list_pos; o++ )
					{
						// if zone matches
						if ( o->config->zone == i )
						{
							// for each output channel on device
							for( int ch = 0; ch < o->config->numOutputs; ch++ )
							{
								// if type matches
								if ( o->config->output[ ch ].type == c )
								{
									if ( prof == PROFILES )
									{
										// = OFF
										o->current_profile[ ch ] = UCHAR_MAX;
									}
									else
									{
										// = profile
										o->current_profile[ ch ] = prof;
									}
								}
							}
						}
					}
				}
			}
		}
	}
									
			
	// for each assertable output
	for( Device* o = device_list; o < device_list_pos; o++ )
	{
		// for each output channel
		for( int ch = 0; ch < o->config->numOutputs; ch++ )
		{
			// if chan not disabled
			if ( !o->IsOutputDisabled( ch ) )
			{		
				// if new profile = off
				if ( o->profile[ ch ] == UCHAR_MAX )
				{
					if ( o->current_profile[ ch ] != UCHAR_MAX ) 
					{
						o->current_profile[ ch ] = o->profile[ ch];  
						o->OutputDeAssert( ch );
					}
				}
				else
				{
					// if profile changed
					if ( o->current_profile[ ch] != o->profile[ ch] )
					{	
						// set as current
						o->current_profile[ ch ] = o->profile[ ch];  
					 
						// Assert output
						o->OutputAssert( ch, o->profile[ ch ] );
					}	
				}
			}
			else // set as de-active if disabled
			{
				// if not de-active
				if ( o->current_profile[ ch ] != UCHAR_MAX )
				{
					// update
					o->current_profile[ ch ] = UCHAR_MAX;
					
					// de-assert
					o->OutputDeAssert( ch );
				}
			}
		}
	}
}




/*************************************************************************/
/**  \fn      void IOStatus::ResetOutputs( )
***  \brief   Class function
**************************************************************************/

void IOStatus::ResetOutputs( )
{
	// for each assertable output
	for( Device* o = device_list; o < device_list_pos; o++ )
	{
		// for each output channel
		for( int ch = 0; ch < o->config->numOutputs; ch++ )
		{
			o->profile[ ch ] = UCHAR_MAX;			
		}
	}
		
}



/*************************************************************************/
/**  \fn      int IOStatus::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int IOStatus::Receive( Message* m )
{
	if ( m->type == MSG_RESET_FIRE )
	{
		app.Send( EVENT_FIRE_CLEAR );
		app.Send( EVENT_FIRE_RESET );
		
		prev_broadcast_silenceable_status   = 0;
	   prev_broadcast_unsilenceable_status = 0;
	   prev_broadcast_skipdelays_status    = 0;
		
		app.Send( EVENT_UPDATE );
	}
	else if ( m->type == MSG_REFRESH_OUTPUTS	)
	{
		refresh_outputs_needed = true;
	}
	else if ( m->type == MSG_RESET_BUTTON )
	{
		app.Send( EVENT_RESET_BUTTON );		 
	}
	else if ( m->type == MSG_SILENCE_BUTTON )
	{
		app.Send( EVENT_SILENCE_BUTTON );		 
	}
	
	return 0;
}


/*************************************************************************/
/**  \fn      int IOStatus::Receive( Event )
***  \brief   Class function
**************************************************************************/

int IOStatus::Receive( Event e )
{
	switch ( e )
	{	
		// NOT USED : We leave everything on for now ?
		// ******************************************* 
		case EVENT_ENTER_SAFE_MODE :
		{
			ncu->QueueWriteMsg( true, NCU_BROADCAST_OUTPUT, this, NULL, 0, 0, 0 );	
			
			// for each assertable output
			for( Device* o = device_list; o < device_list_pos; o++ )
			{
				if ( !o->Broadcasted( ) )
				{
					// for each output channel
					for( int ch = 0; ch < o->config->numOutputs; ch++ )
					{
						 o->OutputDeAssert( ch );
					}
				}
			}
			break;
		}	
 	
		case EVENT_ACKNOWLEDGE_FIRE_BUTTON :
		{
			// timestamp acknowledgement
			ack_fire_time = now( );
			
			ncu->QueueWriteMsg( true, NCU_BROADCAST_ACKNOWLEDGE_FIRE, this, NULL );
			refresh_outputs_needed = true;
			
			break;
		}
		
		case EVENT_ACKNOWLEDGE_EVENT_BUTTON :
		{
			// timestamp acknowledgement
			ack_event_time = now( );
			
			ncu->QueueWriteMsg( true, NCU_BROADCAST_ACKNOWLEDGE_EVENT, this, NULL );
			refresh_outputs_needed = true;
			
			break;
		}
		
		case EVENT_TIME_CHANGED :
		{
			Send( MSG_REFRESH_OUTPUTS );
			break;
		}
		
		case EVENT_SILENCE_BUTTON :
		{
			if ( app.panel != NULL && app.site != NULL )
			{				
				// silence active rules
				for( int r = 0; r < app.panel->numCaeRules; r++ )
				{
					//if ( rule[ r].flags & RULE_ACTIVE )
					{
						rule[ r].flags |= RULE_SILENCED;
					}
				}
				app.DebOut( "Rules silenced" ); 
				
				// silence each asserted input
				for( Device* id = device_list; id < device_list_pos; id++ )
				{
					for( int ch = 0 ; ch < id->config->numInputs; ch++ )
					{
						// if input is active
						if ( id->flags[ ch] & INPUT_ACTIVE )
						{
							// silence it
							id->flags[ ch] |= INPUT_SILENCED;
							
							// if entire zones can be silenced
							if ( app.panel->settings & SITE_SETTING_RESOUND_NEW_ZONE )
							{					
								// flag zone as silenced
								zoneflags[ app.site->currentIndex ][ id->config->zone ] |= ZONE_IS_SILENCED;
							}	
						}
					}	
				}
				
				// for network just copy current zone bits to silence against
				if ( network != NULL )
				{
					// for each panel
					for( int pan = 0; pan < SITE_MAX_PANELS; pan++ )
					{
						for( TimedDeviceRef* tr = netAlertList[ pan ]; tr < netAlertList[ pan ] + MAX_NET_REFS; tr++ )
						{
							if ( tr->index == ALERT_UNUSED )
							{
								break;
							}
							tr->silenced = 1;
						}
					}
				}
				
				AppDeviceClass__TriggerResoundStateEvent( app.DeviceObject, true );			
			
				// Update GUI
				Send( EW_UPDATE_FIRE_LIST_MSG, ew );
				Send( EW_UPDATE_EVENT_LIST_MSG, ew );
				
				// Force
				refresh_silence = true;
			}				
			break;
		}				
		
		case EVENT_RESOUND_BUTTON:
		{
			if ( app.panel != NULL && app.site != NULL )
			{
				AppDeviceClass__TriggerResoundStateEvent( app.DeviceObject, false );			
				
				// Un-silence rules
				for( int n = 0; n < app.panel->numCaeRules; n++ ) 
				{
					rule[ n].flags &= ~RULE_SILENCED;
				}
				
				// Un-silence zones
				for( int i = 1; i <= SITE_MAX_ZONES; i++ )
				{	
					// remove silenced flag
					zoneflags[ app.site->currentIndex ][ i ] &= ~ZONE_IS_SILENCED;		
				}
			
				// Un-silence devices
				for( Device* id = device_list; id < device_list_pos; id++ )
				{	
					for( int ch = 0 ; ch < id->config->numInputs; ch++ )
					{
						// remove silenced flag
						id->flags[ ch ] &= ~INPUT_SILENCED;			
					}
				}
		
				if ( network != NULL )
				{
					// for each panel
					for( int pan = 0; pan < SITE_MAX_PANELS; pan++ )
					{
						for( TimedDeviceRef* tr = netAlertList[ pan ]; tr < netAlertList[ pan ] + MAX_NET_REFS; tr++ )
						{
							if ( tr->index == ALERT_UNUSED )
							{
								break;
							}
							tr->silenced = 0;
						}
					}
				}
				
				Send( EW_UPDATE_FIRE_LIST_MSG, ew );
				Send( EW_UPDATE_EVENT_LIST_MSG, ew );				
			}
			break;
		}
		
		 
		case EVENT_CONFIRM_FIRE_BUTTON :
		{
	 		ack_fire_time = 0;	// undo acknowledge
			
			ncu->QueueWriteMsg( true, NCU_BROADCAST_CONFIRM_FIRE, this, NULL );
			
			confirm_fire_time = now( );
			 
			break;
		}
		
		case EVENT_CONFIRM_EVENT_BUTTON :
		{
			ack_event_time = 0;	// undo acknowledge
			
			ncu->QueueWriteMsg( true, NCU_BROADCAST_CONFIRM_EVENT, this, NULL );
			
			confirm_event_time = now( );
			
			break;
		}
		
		case EVENT_RESET:
		{
			// Assume NCU has sent out reset which is seen as an AOS 0,0,0
			prev_broadcast_silenceable_status   = 0;
		   prev_broadcast_unsilenceable_status = 0;
		   prev_broadcast_skipdelays_status    = 0;
			
			refresh_outputs_needed = true;
			
			AppDeviceClass_TriggerResetEvent( app.DeviceObject ); 
			 
		}
		
		case EVENT_FIRE_CLEAR :
		case EVENT_CLEAR :
		{
			ack_fire_time             = 0;
			ack_event_time            = 0;
			confirm_fire_time         = 0;
			confirm_event_time        = 0;
 			
			test_everything = 0;
													    													  
			ClearDevices( );
			
			for( int p = 0; p < SITE_MAX_PANELS; p++ )
			{
				netAlertList[ p][0].index = ALERT_UNUSED;
			}
		 
			for( int i = 1; i <= SITE_MAX_ZONES; i++ )
			{	
				LED::ZoneOff( i );
				for( int p = 0; p < SITE_MAX_PANELS; p++ )
				{
					zoneflags[ p ][ i] = 0;
				}
			}
			if ( app.panel != NULL )
			for( int r = 0; r < app.panel->numCaeRules; r++ )
			{
				rule[ r].flags = 0;
			}
			app.DebOut( "Rules cleared" );
		}
		
		default:;
	}
	return 0;
}
 


/*************************************************************************/
/**  \fn      time_t IOStatus::GetNextDeviceInAlarm( Device* prev_dev, int prev_chan, Device* &dev, int& chan_num, int actions )
***  \brief   Class function
**************************************************************************/

time_t IOStatus::GetNextDeviceInAlarm( Device* &dev, int& chan_num, int actions, int& action, int inzone )
{
	time_t timesofar = UINT_MAX;
	 
	for( Device* i = device_list; i < device_list_pos; i++ )
	{
		if ( inzone == -1 || i->config->zone == inzone )
		{
			for( int ch = 0 ; ch < i->config->numInputs; ch++ )
			{
				// if active and not found so far
				if ( ( i->flags[ ch ] & ( INPUT_ACTIVE | INPUT_NOT_FOUND ) ) == ( INPUT_ACTIVE | INPUT_NOT_FOUND ) )
				{
					// if active in actions
					if ( ( 1 << i->InputAction( ch ) ) & actions )
					{
						if ( i->timeAsserted[ ch ] < timesofar )
						{
							timesofar = i->timeAsserted[ ch ];
							dev = i;
							chan_num = ch;
							action = i->InputAction( ch );
						}
					}
				}
			}
		}
	}
	
	return timesofar;
}
  

/*************************************************************************/
/**  \fn      time_t IOStatus::GetNextNetworkDeviceInAlarm( Device* prev_dev, int prev_chan, Device* &dev, int& chan_num, int actions )
***  \brief   Class function
**************************************************************************/

time_t IOStatus::GetNextNetworkAlertInAlarm( TimedDeviceRef* &alertref, int actions, int inzone, int inpanel, int& panel )
{
	time_t timesofar = UINT_MAX;
	
	if ( network != NULL && app.panel != NULL && app.site != NULL )
	{			
		for( int n = 0; n < SITE_MAX_PANELS; n++ )
		{
			if ( ( inpanel == -1 || n == inpanel ) && app.panel != app.site->panels + n )
			{ 
				for( TimedDeviceRef* tr = netAlertList[ n ]; tr < netAlertList[ n ] + MAX_NET_REFS; tr++ )
				{
					if ( tr->index == ALERT_UNUSED )
					{
						break;
					}
					if ( ! tr->found && tr->dispzone != 0 )
					{
						if ( inzone == -1 || inzone == tr->dispzone )
						{
							if ( ( 1 << tr->action ) & actions )
							{
								if ( tr->timestamp < timesofar )
								{
									timesofar = tr->timestamp;
									panel = n; 
									alertref = tr;
								}
							}
						}
					}
				}
			}
		}
	}
			
	return timesofar;
}


/*************************************************************************/
/**  \fn      TimedDeviceRef IOStatus::FindRef( int panel, DeviceRef* ref )
***  \brief   Class function
**************************************************************************/

TimedDeviceRef* IOStatus::FindRef( int panel, DeviceRef* ref )
{
	for( TimedDeviceRef* tr = netAlertList[ panel ]; tr < netAlertList[ panel ] + MAX_NET_REFS; tr++ )
	{
		if ( tr->index == ALERT_UNUSED ) return NULL;
		
		if ( *((uint16_t*)ref) == *((uint16_t*)tr) &&  ref->chan == tr->chan )
		{
			return tr;
		}
	}
	return NULL;
}

 
/*************************************************************************/
/**  \fn      time_t IOStatus::GetNextCAEInAlarm( int prev_rule, int& rule_num, int action )
***  \brief   Class function
**************************************************************************/

time_t IOStatus::GetNextCAEInAlarm( int& rule_num, int actions, int& action, int inzone )
{
	time_t timesofar = UINT_MAX;
	
	if ( app.panel != NULL )
	{			
		for( int r = 0; r < app.panel->numCaeRules; r++ )
		{
			if ( ( rule[ r].flags & ( RULE_ACTIVE | RULE_NOT_FOUND ) ) == ( RULE_ACTIVE | RULE_NOT_FOUND ) )
			{
				if ( app.panel->caeRules[ r].displayzone != 0 )
				if ( inzone == -1 || app.panel->caeRules[ r].displayzone == inzone )
				{
					int act = app.panel->caeRules[ r].action;
					
					if ( act == ACTION_SHIFT_CAE_OUTPUT )
					{
						act = app.panel->caeRules[ r].profile;
					}
					
					if ( ( 1 << act ) & actions )
					{
						if ( rule[ r].time < timesofar )
						{
							rule_num = r;
							timesofar = rule[ r].time;
							action = act;
						}
					}
				}
			}
		}
	}
	
	return timesofar;					
}

#endif


enum FirstAlert
{
	NET_FIRST,
	DEV_FIRST,
	CAE_FIRST,
};


/*************************************************************************/
/**  \fn      static int CountIt( unsigned int* bits, int index )
***  \brief   Helper function
**************************************************************************/

static int IsSet( unsigned int bits[][ SITE_MAX_ZONES / 32 ], int zindex, int pindex )
{
	zindex--;
	int w = zindex / 32;
	int i = zindex & 31;
	
	return bits[ pindex][ w ] & ( 1 << i );
}


/*************************************************************************/
/**  \fn      static int SetIt( unsigned int* bits, int index )
***  \brief   Helper function
**************************************************************************/

static int SetIt( unsigned int bits[][ SITE_MAX_ZONES / 32 ], int zindex, int pindex )
{
	zindex--;
	int w = zindex / 32;
	int i = zindex & 31;
	
	return bits[ pindex ][ w ] |= ( 1 << i );
}


/*************************************************************************/
/**  \fn      int IOStatus::GetNumZonesInFire( )
***  \brief   Class function
**************************************************************************/
 
int IOStatus::GetNumZonesInFire( )
{
	int numzones = 0;

	for( int p = 0; p < SITE_MAX_PANELS; p++ )
	{
		// check each zone
		for( int i = 0; i < SITE_MAX_ZONES; i++ )
		{
			// if no fire in zone bits
			if (  zonebits[ ACTION_SHIFT_FIRE ][ i>>5 ] & ( 1 << ( i & 31 ) ) )
			{
				numzones++;
			}
		}
	}
	return numzones;
}	


/*************************************************************************/
/**  \fn      ZoneAlert* IOStatus::GetNthZone( int n )
***  \brief   Class function
**************************************************************************/

ZoneAlert* IOStatus::GetNthZone( int n )
{
	int numalerts = GetNumAlerts( ACTION_BIT_FIRE );	
	 
	int zonenum = 0;
	unsigned int found[ SITE_MAX_PANELS ][ SITE_MAX_ZONES / 32 ] = { 0 };
	
	static ZoneAlert zalert;
	
	app.DebOut( "Looking for nth zone (%d of %d)\n", n, numalerts ); 
	
	for( int alertnum = 0; alertnum < numalerts; alertnum++ )
	{
		// Find next ealiest alert
		app.DebOut( "GetNthAlert( %d ) of %d\n", alertnum, numalerts );
		
		Alert* alert = GetNthAlert( alertnum, ACTION_BIT_FIRE );
		
		if ( alert->flags == ALERT_NONE )
		{
			return NULL;
		}
		
		app.DebOut( "= zone %d\n", alert->zone ); 
		
		// Have we found this zone / panel?
		if ( !IsSet( found, alert->zone, alert->panel_index ) )
		{
			SetIt( found, alert->zone, alert->panel_index );
			app.DebOut( "%d of %d found\n", n, zonenum );
			
			// is this the nth we want	?	
			if ( n == zonenum )
			{
				zalert.zone  = alert->zone;
				zalert.panel = alert->panel_index;
				
				app.DebOut( "Done\n\n", n, zonenum );
				
				return &zalert;
			}
			else			
			{
				// try next..
				zonenum++;
			}
		}
	}
	return NULL;
}


/*************************************************************************/
/**  \fn      Alert* IOStatus::GetNthAlert( int n, int actions, int zone, panel, int mask )
***  \brief   Class function
**************************************************************************/

Alert* IOStatus::GetNthAlert( int n, int actions, int zone, int panel, int mask )
{
	static Alert a;
	int pos = 0;

	app.DebOut( "Getting alert %d", n );
	
	int maxpos = GetNumAlerts( actions );
	
	app.DebOut( " of %d (zone=%d)\n", maxpos, zone );
	
	if ( app.panel != NULL && app.site != NULL && maxpos > 0 && n < maxpos )
	{		
		// reset device list
		for( Device* j = device_list; j < device_list_pos; j++ )
		{
			for( int ch = 0 ; ch < j->config->numInputs; ch++ )
			{
				j->flags[ ch] |= INPUT_NOT_FOUND;
			}
		}
		
		
		// reset cae list 'found' flags
		for( int r = 0; r < app.panel->numCaeRules; r++ )
		{
			rule[ r].flags |= RULE_NOT_FOUND;
		}
		
		// reset network 'found' flags
		if ( panel == -1 )
		{
			for( int pan = 0; pan < SITE_MAX_PANELS; pan++ )
			{
				for( TimedDeviceRef* tr = netAlertList[ pan ]; tr < netAlertList[ pan ] + MAX_NET_REFS; tr++ )
				{			
					tr->found = 0;
				}
			}
		}
		else for( TimedDeviceRef* tr = netAlertList[ panel ]; tr < netAlertList[ panel ] + MAX_NET_REFS; tr++ )
		{			
			tr->found = 0;
		}
			
			
		//start seraching..	
		do
		{
			Device* dev;
			DeviceConfig* netdevice;
			CAERule* netrule;
			TimedDeviceRef* alertref;
			
			int zone_num;
			int panel_num;
			int rule_num;
			int chan_num;
			int action_num;
			int net_chan_num;
			
			time_t devtime = UINT_MAX;
			time_t caetime = UINT_MAX;
			time_t nettime = UINT_MAX;
			
			// if include local panel
			if ( panel == -1 || panel == app.site->currentIndex )
			{
				if ( ! ( mask & MASK_DEV ) ) devtime = GetNextDeviceInAlarm( dev, chan_num, actions, action_num, zone );
				if ( ! ( mask & MASK_CAE ) ) caetime = GetNextCAEInAlarm( rule_num, actions, action_num, zone );
			}
			
			if ( ! ( mask & MASK_NET ) ) nettime = GetNextNetworkAlertInAlarm( alertref, actions, zone, panel, panel_num );
		 
			// if none found - [ strange codeing due to compiler error ]
			if ( devtime == UINT_MAX && nettime == UINT_MAX && caetime == UINT_MAX )
			{
				// return blank alert
				a.description = "";
				a.flags = ALERT_NONE;
				app.DebOut( "Eeek - nowt\n" );
				return &a;
			}	
			
			// Now find the earliest out of the four
			
			FirstAlert al = DEV_FIRST;
			time_t time = devtime;
			
			if ( nettime < time )
			{
				time = nettime;
				al = NET_FIRST;
			}
			if ( caetime < time )
			{
				al = CAE_FIRST;
			}
		
			// if CAE earliest 		
			if ( al == CAE_FIRST )
			{
				// if this is the pos we want
				if ( n == pos )
				{
					CAERule* r = app.panel->caeRules + rule_num;
					a.description = r->name;
					a.timestamp = caetime;
					a.unit = rule_num + 1;
					a.zone = r->displayzone;
					a.flags = ALERT_CAE | ( rule[ rule_num ].flags & RULE_SILENCED ? ALERT_SILENCED : 0);
					a.panel_index = app.site->currentIndex;
					a.action = action_num;
					
					return &a;
				}
				else
				{
					pos++;
					rule[ rule_num].flags &= ~RULE_NOT_FOUND;
				}
			}
			else if ( al == DEV_FIRST )
			{
				// if this is the pos we want
				if ( n == pos )
				{
					a.description = dev->config->location;
					a.timestamp = dev->timeAsserted[ chan_num ];
					a.unit = dev->config->unit;
					a.zone = dev->config->zone;
					a.flags = ALERT_DEV | ( dev->flags[ chan_num ] & INPUT_SILENCED ? ALERT_SILENCED : 0 );
					a.channel_img = dev->config->input[ chan_num ].type;
					a.panel_index = app.site->currentIndex;
					a.action = dev->config->input[ chan_num ].action;
					
					return &a;
				}
				else
				{
					pos++;
					dev->flags[ chan_num ] &= ~INPUT_NOT_FOUND;
				}
			}
			else if ( al == NET_FIRST )
			{
				// if this is the pos we want
				if ( n == pos )
				{
					if ( alertref->type == TYPE_DEV )
					{
						DeviceConfig* dc = app.site->panels[ panel_num ].devs + alertref->index;
						a.description = dc->location;
						a.timestamp = nettime;
						a.unit = dc->unit;
						a.zone = dc->zone;
						a.channel_img = dc->input[ alertref->chan ].type;
						a.action = dc->input[ alertref->chan ].action;
						a.flags = ALERT_NETDEV | ( alertref->silenced ? ALERT_SILENCED : 0);
						a.panel_index = panel_num;
					}
					else if ( alertref->type == TYPE_CAE )
					{
						CAERule* r= app.site->panels[ panel_num ].caeRules + alertref->index;
						a.description = r->name;
						a.timestamp = nettime;
						a.unit = alertref->index + 1;
						a.zone = r->displayzone;
						a.flags = ALERT_CAE | ( alertref->silenced ? ALERT_SILENCED : 0);
						a.panel_index = panel_num;
						a.action = alertref->action;
						a.panel_index = panel_num;
					}
				
					return &a;
				}
				else
				{
					pos++;
					 
					alertref->found = 1;
				}
			}
		} while ( pos < maxpos );								
	}
	
	a.description = "";
	a.flags = ALERT_NONE;

	app.DebOut( "Eeek - nowt\n" );
	
	return &a;	
}



/*************************************************************************/
/**  \fn      int IOStatus::GetAlertList( DeviceRef* dest, int max )
***  \brief   Class function
**************************************************************************/

void IOStatus::GetAlertList( DeviceRef* dest, int max )
{
	DeviceRef* a = dest;
	int count;
	int pos = 0;
	
	if ( app.panel != NULL && app.site != NULL )
	{		
		// reset device list
		for( Device* dev = device_list; dev < device_list_pos; dev++ )
		{
			for( int ch = 0 ; ch < dev->config->numInputs; ch++ )
			{
				// if active
				if ( dev->flags[ ch ] & INPUT_ACTIVE )
				{
					// if active in actions
					int action = dev->InputAction( ch );
					
					if ( action < SITE_NUM_OUTPUT_ACTIONS && action != ACTION_SHIFT_FAULT )
					{
						a->type = TYPE_DEV;
						a->index = dev->config - app.panel->devs;
						a->chan = ch;
						a->action = action;
						a++;
						if ( a - dest >= max )
						{
							return;
						}
					}
				}
			}
		}
			 
	 
		// check cae list
		for( int r = 0; r < app.panel->numCaeRules; r++ )
		{
			if ( rule[ r].flags & RULE_ACTIVE )
			{
				CAERule* caerule = app.panel->caeRules + r;
				
				int action = caerule->action;
				
				if ( action == ACTION_SHIFT_CAE_OUTPUT )
				{
					action = caerule->profile;
				}
				
				if ( action < SITE_NUM_OUTPUT_ACTIONS && action != ACTION_SHIFT_FAULT )
				{
					a->type = TYPE_CAE;
					a->index = r;
					a->action = action;
					a++;
					 
					if ( a - dest >= max )
					{
						return;
					}
				}
			}			
		}					
	}
	while ( a - dest < max )
	{
		a->index = ALERT_UNUSED;
		a++;
	}
}
 
 

/*************************************************************************/
/**  \fn      int IOStatus::GetNumAlerts( int actions, int inzone, int inpanel )
***  \brief   Class function
**************************************************************************/

int IOStatus::GetNumAlerts( int actions, int inzone, int inpanel, int mask )
{
	if ( app.panel == NULL || app.site == NULL ) return 0;
	
	int result = 0;
	
	if ( inpanel ==-1 || inpanel == app.site->currentIndex )
	{
		if ( ! ( mask & MASK_DEV ) )
		{	
			for( Device* i = device_list; i < device_list_pos; i++ )
			{
				if ( inzone == -1 || i->config->zone == inzone )
				{						
					for( int ch = 0 ; ch < i->config->numInputs; ch++ )
					{
						if ( i->flags[ ch ] & INPUT_ACTIVE )
						{
							if ( ( 1 << i->InputAction( ch ) ) & actions )
							{
								result++;
							}
						}
					}
				}
			}
		}	
 		
		if ( ! ( mask & MASK_CAE ) ) result += NumCAEAlarms( actions, inzone );	
	}			
 
	if ( ! ( mask & MASK_NET ) )  result += NumNetworkAlarms( actions, inzone, inpanel );
	
//	app.DebOut( "%d alerts in zone/panel %d/%d\n", result, inzone, inpanel );
	
	return result;
}

// buffer for counting
static char dis_zone[ SITE_MAX_ZONES + 1 ];


bool IOStatus::DevicesOnTestInAlarm( )
{
	for( Device* d = device_list; d < device_list_pos; d++ )
	{
		for( int ch = 0; ch < d->config->numInputs; ch++ )
		{
			if ( !d->IsInputDisabled( ch ) )
			{
				if ( d->flags[ ch ] & INPUT_ACTIVE_ON_TEST )
				{
					return true;
				}
			}
		}
	}
	return false;
}

bool IOStatus::DevicesOnTest( )
{
	for( Device* d = device_list; d < device_list_pos; d++ )
	{
		for( int ch = 0; ch < d->config->numInputs; ch++ )
		{
			if ( !d->IsInputDisabled( ch ) )
			{
				if ( d->flags[ ch ] & INPUT_ON_TEST )
				{
					return true;
				}
			}
		}
	}
	return false;
}


/*************************************************************************/
/**  \fn      int IOStatus::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int IOStatus::Receive( Command* cmd )
{	
	assert( cmd != NULL );
	
	switch ( cmd->type )
	{	
		case CMD_RESET_FIRE_STATE :
		{
			Send( MSG_RESET_FIRE );

			return CMD_OK;
		}
		case CMD_SET_TEST_PROFILE:
		{
			test_profile_bit = cmd->int0;
			break;
		}
		case CMD_GET_NTH_ZONE :
		{
			ZoneAlert* za = GetNthZone( cmd->int0 );
			if ( za != NULL )
			{ 
				cmd->int0 = za->zone;
				cmd->int1 = za->panel;
			 			
				return CMD_OK;
			}	
			return CMD_ERR_OUT_OF_RANGE;
		}
		
		case CMD_SET_NUM_FIRES:
		{
			if ( cmd->int0 == 0 )
			{
				if ( last_fire_activity == PREVENT_BLANK ) last_fire_activity = now( );
			}
			else
			{
				last_fire_activity = PREVENT_BLANK;
			}
			return CMD_OK;
		}
		
		case CMD_GET_NUM_ZONES_IN_FIRE:
		{
			cmd->int0 = GetNumZonesInFire( );
			
			return CMD_OK;
		}
		
		case CMD_GET_NUM_ALERTS_IN_ZONE:
		{
			cmd->int0 = GetNumAlerts( ACTION_BIT_FIRE, cmd->int0, cmd->int1 );
			return CMD_OK;
		}
			
		case CMD_GET_ALERT_DETAILS :
		{
			Alert* al = GetNthAlert( cmd->int0, cmd->int1, cmd->int2, cmd->int3 );
				
			static char tstr[ 20 ];
				
			struct tm *tp = localtime( &al->timestamp );

			snprintf( tstr, 20, "%02d:%02d", tp->tm_hour, tp->tm_min );
			
			cmd->int0   = (int) al->description;
			cmd->char4  = al->zone;
			cmd->char5  = al->panel_index;
			cmd->short3 = al->unit;
			cmd->short4 = al->flags;
			cmd->charA  = al->channel_img;
			cmd->charB  = al->action;
			cmd->int3   = (int) tstr;
			 
			
			return CMD_OK;
		}
		
		case CMD_GET_NUM_ENABLE_DISABLE:
		{			 
			int num = 0; // virt. routing + virt. alarm zone
			
			if ( app.panel != NULL )
			{
				num = 2; // virt. routing + virt. alarm zone
				
				unsigned int zbits[ SITE_MAX_ZONES / (8*sizeof(unsigned int)) ];
				memset( zbits, 0, sizeof( zbits ) );
	 
				for( DeviceConfig* dc = app.panel->devs; dc < app.panel->devs + app.panel->NumDevs( ); dc++ )
				{
					Device* d = FindDevice( dc->unit );
					
					// Add zone 
					if ( !IFSET( dc->zone, zbits ) )
					{
						num++;
						SET( dc->zone, zbits );						
					}
					// Add device in zone
					
					
					
					if ( zoneflags[ app.site->currentIndex ][ dc->zone ] & ZONE_DISABLE_EXPANDED )
					{
						num++;
						
						// Add channels
						if ( d != NULL )
						if ( d->settings & SETTING_DISABLE_EXPANDED )
						{
							num += dc->numInputs + dc->numOutputs;
						}
					}
					
					// Add device in alarm set 
					if ( zoneflags[ app.site->currentIndex ][ VIRTUAL_ALARM_ZONE ] & ZONE_DISABLE_EXPANDED )
					{
						int firechans = 0;
					 
						for( int chan = 0; chan < dc->numOutputs; chan++ )
						{
							if ( dc->output[ chan].actions & ACTION_BIT_FIRE )
							{
								firechans++;
							}
						}
						
						// Add channels
						if ( d != NULL )
						if ( d->settings & SETTING_DISABLE_FIRE_EXPANDED )
						{
							num += firechans;
						}
						
						// Add device in alarm zone
						if ( firechans > 0 )
						{
							num++;
						}
					}
				 
					// Routing	
						
					if ( zoneflags[ app.site->currentIndex ][ VIRTUAL_ROUTING_ZONE ] & ZONE_DISABLE_EXPANDED )	 
					{
						int routingchans = 0;
					 
						for( int chan = 0; chan < dc->numOutputs; chan++ )
						{
							if ( dc->output[ chan ].type == CO_CHANNEL_PANEL_ROUTING_OUTPUT ||
								  dc->output[ chan ].type == CO_CHANNEL_OUTPUT_ROUTING_E )
							{
								routingchans++;
							}
						}
						
						if ( routingchans )
						{
							num++;
							if ( d != NULL )
							if ( d->settings & SETTING_DISABLE_ROUTING_EXPANDED )
							{
								num += routingchans;
							}
						}
					}
				}
			}
			cmd->int0 = num;
			
			return CMD_OK;
		}
		
		case CMD_GET_NUM_TEST:
		{			 
			int num = 0;  
			
			if ( app.panel != NULL )
			{
				unsigned int zbits[ SITE_MAX_ZONES / (8*sizeof(unsigned int)) ];
				memset( zbits, 0, sizeof( zbits ) );
	 
				for( DeviceConfig* dc = app.panel->devs; dc < app.panel->devs + app.panel->NumDevs( ); dc++ )
				{
					Device* d = FindDevice( dc->unit );
					
					// Add zone 
					if ( !IFSET( dc->zone, zbits ) )
					{
						num++;
						SET( dc->zone, zbits );						
					}
					
					// Add device in zone
					if ( zoneflags[ app.site->currentIndex ][ dc->zone ] & ZONE_TEST_EXPANDED )
					{
						num++;
						
						// Add channels
						if ( d != NULL )
						if ( d->settings & SETTING_TEST_EXPANDED )
						{
							num += dc->numInputs + dc->numOutputs;
						}
					}
				}
			}
			cmd->int0 = num;
			
			return CMD_OK;
		}
		
		
		case CMD_GET_NUM_MODIFY:
		{			 
			int num = 0; // virt. routing + virt. alarm zone
			
			if ( app.panel != NULL )
			{
				unsigned int zbits[ SITE_MAX_ZONES / (8*sizeof(unsigned int)) ];
				memset( zbits, 0, sizeof( zbits ) );
	 
				for( DeviceConfig* dc = app.panel->devs; dc < app.panel->devs + app.panel->NumDevs( ); dc++ )
				{
					Device* d = FindDevice( dc->unit );
					
					// Add zone 
					if ( !IFSET( dc->zone, zbits ) )
					{
						num++;
						SET( dc->zone, zbits );						
					}
					
					// Add device in zone
					if ( zoneflags[ app.site->currentIndex ][ dc->zone ] & ZONE_MODIFY_EXPANDED )
					{
						num++;
					}
				}
			}
			cmd->int0 = num;
			
			return CMD_OK;
		}
			
		case CMD_SET_OUTPUT_CHANNEL :
		{
			int channel = cmd->int1;
			int unit 	= cmd->int0;
			int state   = cmd->int2;
			int profile = cmd->int3;
				
			// for each assertable output
			for( Device* o = device_list; o < device_list_pos; o++ )
			{	
				if ( o->config->unit == unit || unit == -1 )
				{
					for( int ch = 0; ch < o->config->numOutputs; ch++ )
					{
						if ( ch == channel || channel == -1 )
						{
							// if channel is enabled
							if ( !o->IsOutputDisabled( ch ) )
							{
								// if not already asserted
								if ( o->current_profile[ ch ] != profile )
								{
									// if switch ON
									if ( state )
									{
										// if not already on test
										if ( !( o->flags[ ch ] & OUTPUT_ON_TEST ) )
										{
											// flag as on test
											o->flags[ ch ] |= OUTPUT_ON_TEST;
											
											// assert
											o->OutputAssert( ch, profile ); 
										}
									}
									else // switch off
									{
										// if on test
										if ( o->flags[ ch ] & OUTPUT_ON_TEST ) 
										{
											// flag as off
											o->flags[ ch ] &= ~OUTPUT_ON_TEST;
										
											// De-assert
											o->OutputDeAssert( ch ); 										
										}
									}							
								}					
							}
						}
					}
				}
			}
			return CMD_OK;
		}
		
		case CMD_TEST_EVERYTHING:
		{
			test_everything = cmd->int0;
			return CMD_OK;
		}
		
		case CMD_GET_TEST_EVERYTHING:
		{
			cmd->int0 = test_everything ;
			return CMD_OK;
		}
		
		case CMD_GET_ZONE_FLAGS:
		{
			if ( app.site != NULL )
			{
				if ( cmd->int0 > -2 && cmd->int0 <= SITE_MAX_ZONES )
				{
					cmd->int0 = zoneflags[ app.site->currentIndex ][cmd->int0 ];
					return CMD_OK;
				}
				else
				{
					cmd->int0 = 0;
					return CMD_ERR_OUT_OF_RANGE;
				}
			}
			else
			{
				return CMD_ERR_NO_SITE;
			}	
		}
		
		case CMD_GET_PANEL_ZONE_FLAGS:
		{
			if ( cmd->int1 >= 0 && cmd->int1 < SITE_MAX_PANELS )
			{
				if ( cmd->int0 > 0 && cmd->int0 <= SITE_MAX_ZONES )
				{	
					cmd->int0 = zoneflags[ cmd->int1 ][cmd->int0];
					return CMD_OK;
				}
			}
			cmd->int0 = 0;
			return CMD_ERR_OUT_OF_RANGE ;
		}
		
		case CMD_SET_ZONE_FLAGS:
		{
			if ( app.panel != NULL && app.site != NULL )
			{
				if ( cmd->int0 > -2 && cmd->int0 <= SITE_MAX_ZONES )
				{
					zoneflags[ app.site->currentIndex ][cmd->int0 ] = cmd->int1;
					return CMD_OK;
				}
		 
				return CMD_ERR_OUT_OF_RANGE;
			}
			return CMD_ERR_NO_SITE;
		}
		
		case CMD_SET_PANEL_ZONE_FLAGS:
		{
			if ( cmd->int0 > 0 && cmd->int0 <= SITE_MAX_ZONES && cmd->int1 >= 0 && cmd->int1 < SITE_MAX_PANELS )
			{
				zoneflags[ cmd->int1 ][cmd->int0 ] = cmd->int2;
				
				return CMD_OK;
			}
			return CMD_ERR_OUT_OF_RANGE;
		}
		
		case CMD_FIND_DEVICE_FROM_UNIT :
		{
			for( Device* o = device_list; o < device_list_pos; o++ )
			{	
				if ( o->config->unit == cmd->int0 )
				{
					cmd->int0 = (int) o;
					return CMD_OK;
				}
			}
			cmd->int0 = (int) NULL;
			return CMD_ERR_OUT_OF_RANGE;
		}				
		
		case CMD_CHECK_ACTIVE_ON_TEST:
		{
			cmd->int0 = DevicesOnTestInAlarm( );
			
			if ( ! cmd->int0 )
			{
				cmd->int1 = DevicesOnTest( );
			}
			
			return CMD_OK;
		}
		
		case CMD_GET_DEVICE_FROM_CONFIG:
		{
			for( Device* o = device_list; o < device_list_pos; o++ )
			{	
				if ( o->config == (DeviceConfig*) cmd->int0 )
				{
					cmd->int0 = (int) o;
					return CMD_OK;
				}
			}
			cmd->int0 = (int) NULL;
			return CMD_ERR_OUT_OF_RANGE;
		}	
		
		case CMD_GET_NUM_FIRES : 
		{
			cmd->int0 = GetNumAlerts( ACTION_BIT_FIRE );
			app.DebOut( "Fire alerts = %d\n", cmd->int0 );
			
			if ( cmd->int0 == 0 ) Buzzer::Mute( BUZZ_FOR_FIRE );
			
			return CMD_OK;
		}

		case CMD_GET_NUM_OTHER_EVENTS : 
		{
			cmd->int0 = GetNumAlerts( ACTION_BIT_ROUTING_ACK | ACTION_BIT_SECURITY | ACTION_BIT_FIRST_AID | ACTION_BIT_EVACUATION | ACTION_BIT_GENERAL );
			return CMD_OK;
		}

#define ZONE_NOT_CAE_DISABLED 	4 		
#define ZONE_IS_ENABLED 			2
#define ZONE_IS_IN_SITE 			1
		
		case CMD_GET_NUM_DISABLED :
		{
			if ( app.panel != NULL )
			{
				int n = 0;
				
				// Clear flags
				for( char* zp = dis_zone + 1; zp <= dis_zone + SITE_MAX_ZONES; zp++ ) *zp &= ~( ZONE_IS_IN_SITE | ZONE_IS_ENABLED | ZONE_NOT_CAE_DISABLED );
							
				// Flag every zone used by each device ( except evac button / virual devices )
				for( Device* d = device_list; d < device_list_pos; d++ ) if ( d->config->unit < SITE_VIRTUAL_UNIT_START ) 
				{
					// flag zone is in site
					dis_zone[ d->config->zone ] |= ZONE_IS_IN_SITE; 
					
					int i;
		
					for( i = 0; i < d->config->numInputs; i++ )
					{
						// if device input isnt disabled
						if ( !d->IsInputDisabled( i ) )
						{
							// Flag zone is used
							dis_zone[ d->config->zone ] |= ZONE_IS_ENABLED;
							break;
						}
						else // if disabled
						{
							// if configured disable, then its not disabled due to CAE
							if ( d->config->IsInputDisabled( i ) )
							{
								dis_zone[ d->config->zone ] |= ZONE_NOT_CAE_DISABLED;
							}
						}
					}
					if ( i < d->config->numInputs ) continue;
					
					for( int o = 0; o < d->config->numOutputs; o++ )
					{
						// if device output isnt disabled
						if ( !d->IsOutputDisabled( o ) )
						{
							// Flag zone is used
							dis_zone[ d->config->zone ] |= ZONE_IS_ENABLED;
							break;
						}
						else // if disabled
						{
							// if configured disable, then its not disabled due to CAE
							if ( d->config->IsInputDisabled( o ) )
							{
								dis_zone[ d->config->zone ] |= ZONE_NOT_CAE_DISABLED;
							}
						}
					}
				}
			
			
				for( char* zp = dis_zone + 1; zp <= dis_zone + SITE_MAX_ZONES; zp++ )
				{
					// if zone is in site, but not enabled, assume zone is disabled
					if ( (*zp & (ZONE_IS_IN_SITE | ZONE_IS_ENABLED) ) == ZONE_IS_IN_SITE ) n++;
				}
				
				if ( app.panel->settings & SITE_SETTING_FIRE_ROUTING_DISABLED ) 		
				{
					n++;
				}
				if ( app.panel->settings & SITE_SETTING_FIRE_ALARMS_DISABLED )
				{
					n++;
				}
				
				// Set number of zones
				cmd->int0 = n;
				
				n = 0;
				for( Device* d = device_list; d < device_list_pos; d++ ) if ( d->config->unit < SITE_VIRTUAL_UNIT_START ) 
				{
					// if device isnt included as part of zone disablements
					if ( ( dis_zone[ d->config->zone ] & (ZONE_IS_IN_SITE | ZONE_IS_ENABLED)) != ZONE_IS_IN_SITE )
					{						
						for( int ch = 0; ch < d->config->numOutputs; ch++ )
						{
							if ( d->IsOutputDisabled( ch ) )
							{
								if ( d->config->IsAlarmOutputChannel( ch ) )
								{
									if ( !(app.panel->settings & SITE_SETTING_FIRE_ALARMS_DISABLED ) ) 
									{
										n++;
									}
								}
								else if ( d->config->IsRoutingOutputChannel( ch ) )
								{
									if ( !(app.panel->settings & SITE_SETTING_FIRE_ROUTING_DISABLED ) )
									{
										n++;
									}
								}
								else
								{
									n++;
								}
							}
						}
						for( int ch = 0 ; ch < d->config->numInputs; ch++ )
						{
							if ( d->IsInputDisabled( ch ) )
							{	
								n++;
							}
						}
					}
				}
				// Number of devices disabled
				cmd->int1 = n;
				
				cmd->int2 = Buzzer::buzzer_disabled;
				
				if ( LED::state[ GPIO_Disablements ] != LED_FLASH )
				{
					LED::Set( GPIO_Disablements, cmd->int0 + cmd->int1 + cmd->int2 > 0 ? LED_ON : LED_OFF );
				}									
			}
			else
			{
				cmd->int0 = 0;
				cmd->int1 = 0;
				cmd->int2 = 0;
			}
				
			if ( cmd->int0 + cmd->int1 + cmd->int2 == 0 )
			{
				if ( last_disablement_activity == PREVENT_BLANK )
				{
					last_disablement_activity = now();
				}
			}
			else
			{
				last_disablement_activity = PREVENT_BLANK;
			}
			
			return CMD_OK;
		}
				 
		case CMD_DISABLED_ZONE_DETAILS :
		{
			if ( app.panel != NULL )
			{
				int z = 0;
				if ( app.panel->settings & SITE_SETTING_FIRE_ALARMS_DISABLED )
				{
					if ( z == cmd->int0 )
					{
						cmd->short2 = -1;
						cmd->short3 = -1;
						
						return CMD_OK;
					}
					else z++;
				}
					
				if ( app.panel->settings & SITE_SETTING_FIRE_ROUTING_DISABLED )
				{
					if ( z == cmd->int0 )
					{
						cmd->short2 = 0;
						cmd->short3 = -1;
						
						return CMD_OK;
					}
					else z++;
				}	
				
				for( int n = 1; n <= SITE_MAX_ZONES; n++ )
				{
					if ( ( dis_zone[ n ] & 3 ) == 1 )
					{
						if ( z == cmd->int0 )
						{
							cmd->short2 = n;
							cmd->short3 = -1;
							
							cmd->short6 = dis_zone[ n ] & ZONE_NOT_CAE_DISABLED ? 0 : 16;
							
							return CMD_OK;
						}
						else
						{
							z++;
						}
					}
				}
				return CMD_ERR_OUT_OF_RANGE;
			}
			return CMD_ERR_NO_SITE;
		}
		
		case CMD_GET_DISABLED_DETAILS :
		{				  
			if ( app.panel != NULL )
			{
				int n = 0;
				for( Device* d = device_list; d < device_list_pos; d++ )
				{
					if ( ( dis_zone[ d->config->zone ] & 3 ) != 1 )
					{
						for( int ch = 0 ; ch < d->config->numInputs; ch++ )
						{
							if ( d->config->input[ ch ].type != CO_CHANNEL_NONE_E )
							if ( d->IsInputDisabled( ch ) )
							{
								if ( n == cmd->int0 )
								{
									cmd->int0 = (int) d->config->location;
									cmd->short2 = d->config->zone;
									cmd->short3 = d->config->unit;
									cmd->short4 = d->config->input[ ch].type;
									cmd->short5 = ch;
									cmd->short6 = 1 | ( ( d->config->input[ ch].flags & CHANNEL_OPTION_DISABLED_DAYTIME ) ? 2 : 0 ) |
															( ( d->config->input[ ch].flags & CHANNEL_OPTION_DISABLED_NIGHTTIME ) ? 4 : 0 );
									
									if ( !d->config->IsInputDisabled( ch ) )
									{
										cmd->short6 |= 16;
									}
									
									return CMD_OK;
								}
								else
								{
									n++;
								}
							}
						}
						for( int ch = 0 ; ch < d->config->numOutputs; ch++ )
						{
							if ( d->config->output[ ch ].type != CO_CHANNEL_NONE_E )
							if ( d->IsOutputDisabled( ch ))
							{
								bool use_it = false;
								
								if ( d->config->IsAlarmOutputChannel( ch ) )
								{
									if ( !(app.panel->settings & SITE_SETTING_FIRE_ALARMS_DISABLED ) ) 
									{
										use_it = true;
									}
								}
								else if ( d->config->IsRoutingOutputChannel( ch ) )
								{
									if ( !(app.panel->settings & SITE_SETTING_FIRE_ROUTING_DISABLED ) )
									{
										use_it = true;
									}
								}
								else
								{
									use_it = true;
								}
							 
								if ( use_it )
								{
									if ( n == cmd->int0 )
									{
										cmd->int0 = (int) d->config->location;
										cmd->short2 = d->config->zone;
										cmd->short3 = d->config->unit;
										cmd->short4 = d->config->output[ ch].type;
										cmd->short5 = ch;
										cmd->short6 = 0;
										
										if ( !d->config->IsOutputDisabled( ch ) )
										{
											cmd->short6 |= 16;
										}
										
										return CMD_OK;
									}
									else
									{
										n++;
									}
								}
							}
						}
					}
				}
				
				cmd->int0 = (int) "<?>";
				cmd->short2 = 0;
				cmd->short3 = 0;
				cmd->short4 = 0;
				
				return CMD_ERR_OUT_OF_RANGE;
			}
			return CMD_ERR_NO_SITE;
		}	
	
		case CMD_GET_NUM_ON_TEST :
		{
			if ( app.panel != NULL && app.site != NULL )
			{
				int n = 0;
				for( char* zp = dis_zone + 1; zp <= dis_zone + SITE_MAX_ZONES; zp++ ) *zp &= ~12;
							
				for( Device* d = device_list; d < device_list_pos; d++ ) if ( d->config->type <= DEVICE_TYPE_PANEL_IO )  
				{			
					if ( d->config->numInputs > 0 )
					{						
						dis_zone[ d->config->zone ] |= 4; // zone is used
						int i;
			
						for( i = 0; i < d->config->numInputs; i++ )
						{
							if ( !d->IsInputOnTest( i ) )
							{
								dis_zone[ d->config->zone ] |= 8;
								break;
							}
						}
					}
				}
			
				for( int z = 1; z <= SITE_MAX_ZONES; z++ )
				{
					if ( ( dis_zone[ z] & 12 ) == 4 || ( zoneflags[ app.site->currentIndex ][ z ] & ZONE_ON_TEST ) )
					{
						n++;
					}
				}
				 
				// Set number of zones
				cmd->int0 = n;
				
				n = 0;
				for( Device* d = device_list; d < device_list_pos; d++ ) if ( d->config->type <= DEVICE_TYPE_PANEL_IO )   
				{
					int z = d->config->zone;
					
					if ( ( dis_zone[ z] & 12 ) != 4 )
					{
						for( int ch = 0 ; ch < d->config->numInputs; ch++ )
						{
							if ( d->IsInputOnTest( ch ) )
							{
								Log::Msg(LOG_TST,"Zone:%d Devices:%d Active in Test", d->config->zone, d->config->unit);
								n++;
							}
						}
					}
				}
				
				// Set number of deies
				cmd->int1 = n;
				
			
				// Bit of a hack, but may as well go here..
				LED::Set( GPIO_TestMode, cmd->int0 + cmd->int1 > 0 ? LED_ON : LED_OFF );
			}
			else
			{
				cmd->int0 = 0;
				cmd->int1 = 0;
			}
			
			if ( cmd->int0 + cmd->int1 == 0 )
			{
				if ( last_ontest_activity == PREVENT_BLANK ) last_ontest_activity = now();
			}
			else
			{
				last_ontest_activity = PREVENT_BLANK;
			}
			
			return CMD_OK;
		}
	 
		case CMD_GET_ON_TEST_ZONE_DETAILS : 
		{
			int z = 0;
		
			for( int n = 1; n <= SITE_MAX_ZONES; n++ )
			{
				if ( ( dis_zone[ n ] & 12 ) == 4 || ( zoneflags[ app.site->currentIndex ][ n ] & ZONE_ON_TEST ) )
				{
					if ( z == cmd->int0 )
					{
						cmd->short2 = n;
						cmd->short3 = -1;
						
						return CMD_OK;
					}
					else
					{
						z++;
					}
				}
			}
			return CMD_ERR_OUT_OF_RANGE;
		}
	
		case CMD_GET_ON_TEST_DETAILS :
		{				  
			if ( app.panel != NULL )
			{
				int n = 0;
				for( Device* d = device_list; d < device_list_pos; d++ ) if ( d->config->type <= DEVICE_TYPE_PANEL_IO )
				{
					if ( d->config->numInputs > 0 )
					{
						int z = d->config->zone;
						
						if ( ( dis_zone[ z ] & 12 ) != 4 )
						{
							for( int ch = 0; ch < d->config->numInputs; ch++ )
							{
								if ( d->IsInputOnTest( ch ) )
								{
									if ( n == cmd->int0 )
									{
										cmd->int0 = (int) d->config->location;
										cmd->short2 = d->config->zone;
										cmd->short3 = d->config->unit;
										cmd->short4 = d->config->input[ ch].type;
										
										return CMD_OK;
									}
									else
									{
										n++;
									}
								}
							}
						}
					}
				}
				
				cmd->int0 = (int) "<?>";
				cmd->short2 = 0;
				cmd->short3 = 0;
				cmd->short4 = 0;
				
				return CMD_ERR_OUT_OF_RANGE;
			}
			return CMD_ERR_NO_SITE;
		}	

		default:
				return CMD_ERR_UNKNOWN_CMD;
	}
	return CMD_ERR_UNKNOWN_CMD;
}
 

/*************************************************************************/
/**  \fn      int IOStatus::NumCAEAlarms( ActionEnum action )
***  \brief   Class function
**************************************************************************/

int IOStatus::NumCAEAlarms( int actions, int inzone )
{
	if ( app.panel == NULL ) return 0;
		
	int count = 0;
	
	for( int r = 0; r < app.panel->numCaeRules; r++ )
	{
		if ( app.panel->caeRules[ r].displayzone != 0 )
		if ( inzone == -1 || app.panel->caeRules[ r].displayzone == inzone )
		if ( rule[ r].flags & RULE_ACTIVE )
		{
			int action = app.panel->caeRules[ r].action;
			
			if ( action == ACTION_SHIFT_CAE_OUTPUT )
			{
				action = app.panel->caeRules[ r].profile;
			}
			
			if ( ( 1 << action ) & actions )
			{
				count++;
			}
		}
	}
	return count;
}


/*************************************************************************/
/**  \fn      int IOStatus::NumNetworkAlarms( int actions )
***  \brief   Class function
**************************************************************************/

int IOStatus::NumNetworkAlarms( int actions, int inzone, int inpanel )
{
	int alarms = 0;
	
	if ( network != NULL && app.site != NULL )
	{	
		for( int n = 0; n < SITE_MAX_PANELS; n++ )
		{
			if ( app.site->currentIndex != n )
			{
				if ( inpanel == -1 || n == inpanel )
				{
					for( TimedDeviceRef* tr = netAlertList[ n ]; tr < netAlertList[ n ] + MAX_NET_REFS; tr++ )
					{
						if ( tr->index == ALERT_UNUSED )
						{
							break;
						}
					 
						if ( ( 1 << tr->action ) & actions )
						{
							if ( tr->dispzone != 0 )
							if ( inzone == -1 || tr->dispzone == inzone )
							{
								alarms++;
							}
						}
					}
				}
			}
		}
	}
	
	return alarms;
}
		

/*************************************************************************/
/**  \fn      void IOStatus::Poll( )
***  \brief   Class function
**************************************************************************/

void IOStatus::Poll( )
{
	int silenceable_status   = 0;
	int unsilenceable_status = 0;
	int skipdelays_status    = 0;
	int output_changes       = 0;
	
	static int current_broadcast_delays = 0;
	static int current_individual_delays = 0;
	
	// No panel, no io
	if ( app.site == NULL || app.panel == NULL ) return;
	
	ResetInputs( );
	
	// Reset - all channel profile values set to 0xFF	
	ResetOutputs( );
	
	// Get input status
	int input_changes = RefreshInputs( silenceable_status, unsilenceable_status, skipdelays_status ); 
	
	// Get CAE status
	input_changes |= ProcessCauseAndEffect( silenceable_status, unsilenceable_status, output_changes ); 

	static int about_time = 0;

	// Get Network status
	input_changes |= ProcessNetworkStatus( silenceable_status, unsilenceable_status, skipdelays_status );

	if ( unsilenceable_status == 0 )
	{
		if ( last_event_activity == PREVENT_BLANK ) last_event_activity = now( );
	}
	else
	{
		last_event_activity = PREVENT_BLANK;
	}

	int prefault_status = unsilenceable_status & ACTION_BIT_FAULT;


	// Add fault status ~
	Fault::GetFaultStatus( silenceable_status, unsilenceable_status );
	
	// If faults addd to zone 1 for networking output
	if ( unsilenceable_status & ACTION_BIT_FAULT ) zonebits[ ACTION_SHIFT_FAULT ][ 0 ] |= 1;	

 
	
	// See whats new..		
	static int prev_input_unsilenceable_status = 0;
	static int prev_input_silenceable_status = 0;
	
	int unsilenceable_new_events = unsilenceable_status & ~prev_input_unsilenceable_status;
	int silenceable_new_events = silenceable_status & ~prev_input_silenceable_status;
 
	// Sound the buzzer
	if ( new_zone_in_fire )
	{
		Buzzer::Buzz( BUZZ_FOR_FIRE );
	}
	
	if ( unsilenceable_new_events & ACTION_BIT_FIRST_AID )
	{
		Buzzer::Buzz( BUZZ_FOR_FIRST_AID );
	}
	if ( !( unsilenceable_status & ACTION_BIT_FIRST_AID ) )
	{
		Buzzer::Mute( BUZZ_FOR_FIRST_AID );
	}
	
	if ( unsilenceable_new_events & ACTION_BIT_SECURITY )
	{
		Buzzer::Buzz( BUZZ_FOR_SECURITY );
	}
	if ( !( unsilenceable_status & ACTION_BIT_SECURITY ) )
	{
		Buzzer::Mute( BUZZ_FOR_SECURITY );
	}
	
	if ( unsilenceable_new_events & ACTION_BIT_GENERAL )
	{
		Buzzer::Buzz( BUZZ_FOR_GENERAL );
	}
	if ( !( unsilenceable_status & ACTION_BIT_GENERAL ) )
	{
		Buzzer::Mute( BUZZ_FOR_GENERAL );
	}
	
	
	prev_input_unsilenceable_status = unsilenceable_status;
	prev_input_silenceable_status = silenceable_status;
 	
	// timestamp new events ( if any )	
	if ( unsilenceable_new_events )
	{	
		// fire, first aid, fault, evac, security
		for( int bit = 0; bit < SITE_NUM_INPUT_ACTIONS; bit++ )
		{
			// if new event
			if ( unsilenceable_new_events & ( 1 << bit ) )
			{
				// timestamp it
				unsilenceable_assert_time[ bit ] = now( );
			}
		}			
	}
	
	// Poke app support..
	
	if ( input_changes || unsilenceable_status != prev_unsilenceable_status )
	{				
#ifdef GSM_HTTP			
			AppSupport::CheckEventStatus( );
#else
			MQTTSupport::CheckEventStatus( );
#endif	
		 
	}
	
	// Update GUI to show any changes
	
	// Fire page
	if ( input_changes & ACTION_BIT_FIRE  )
	{	
		//	Update fire list
		AppDeviceClass_TriggerFireListChangeEvent( app.DeviceObject );			
	}
	
	// Other events page
	if ( input_changes & ACTION_BITS_OTHER_EVENTS )
	{		
		// Update event list
		AppDeviceClass_TriggerEventListChangeEvent( app.DeviceObject );			
	}
	
	
	// Show fire LED if fire event
	if ( unsilenceable_status & ACTION_BIT_FIRE )
	{
		LED::On( GPIO_Fire );
	}
	else
	{
		LED::Off( GPIO_Fire );
	}	
	
	// if routing acknowlege 
	if ( unsilenceable_status & ACTION_BIT_ROUTING_ACK )
	{
		// if waiting for it 
		if ( LED::state[ GPIO_FireRoutingActive ] == LED_FLASH )
		{
			// Indicate acknowledge
			LED::On( GPIO_FireRoutingActive );
		}
	}
	
	// Show other events LED if other events active
	if ( unsilenceable_status & ACTION_BITS_OTHER_EVENTS )
	{
		LED::On( GPIO_OtherEvents );
	}
	else
	{
		LED::Off( GPIO_OtherEvents );
	}	

	// if new events
	if ( unsilenceable_new_events )
	{			
		// Remote Ack/Silence
		if ( unsilenceable_new_events & ACTION_BIT_SILENCE )
		{
			Receive( EVENT_SILENCE_BUTTON );
			network->toSend |=  ACTION_BIT_SILENCE;
		}
		
		// Remote Reset
		if ( unsilenceable_new_events & ACTION_BIT_RESET )
		{
			Send( MSG_RESET_BUTTON );
			network->toSend |=  ACTION_BIT_RESET;
		}
 
		// Check for texts
		if ( gsm != NULL )
		{
			if ( prefault_status )
			{
				gsm->ProcessEvents( unsilenceable_status );
			}
			else
			{
				gsm->ProcessEvents( unsilenceable_status & ~ACTION_BIT_FAULT );
			}		
		}
	}	
	 
	// if new Zone LEDS in fire
	if ( new_zone_in_fire || input_changes )
	{	 
		// check each zone
		for( int i = 0; i < SITE_MAX_ZONES; i++ )
		{
			// if no fires in zone
			if ( !( zonebits[ ACTION_SHIFT_FIRE ][ i>>5 ] & ( 1 << ( i & 31 ) ) ) )
			{
				// Clear zone LED flag
				zoneflags[ app.site->currentIndex ][ i + 1] &= ~ZONE_LED_IN_FIRE;
				// un-silence it
				zoneflags[ app.site->currentIndex ][ i + 1] &= ~ZONE_IS_SILENCED;
				// collpase zone
				zoneflags[ app.site->currentIndex ][ i + 1] &= ~ZONE_FIRE_EXPANDED;
			}		 
		}
	
		// each zone LED
		for( int n = 1; n <= GPIOManager::led_zones ; n++ )
		{
			// if zone LED in fire condition
			if ( zoneflags[ app.site->currentIndex ][ n] & ZONE_LED_IN_FIRE )
			{
				// if its not already lit
				if ( !LED::GetZone( n ) )
				{
					// light it
					LED::ZoneOn( n );
				}					 
			}
			else // if not in fire
			{
				// but lit
				if ( LED::GetZone( n ) )
				{
					// unlight it
					LED::ZoneOff( n );
				}
			}
		}
	}
	
	// Automatically skip fault outputs
	skipdelays_status |= ACTION_BIT_FAULT;
	
	// Check for alarm disablment
	if ( app.panel->settings & SITE_SETTING_FIRE_ALARMS_DISABLED )
	{
		// just clear flags - yes its a bit of a hack
		silenceable_status = 0;
		unsilenceable_status = 0;
	}		 
		
	// Set individual (non - broadcast) output status
	
	if ( refresh_silence )
	{
		// force silence
		silenceable_status = 0;
	}	
	
	if ( current_individual_delays || refresh_outputs_needed || output_changes ||
		  unsilenceable_status != prev_unsilenceable_status  ||
	     silenceable_status  != prev_silenceable_status || refresh_silence )
	{
		current_individual_delays = RefreshOutputStatus( silenceable_status, unsilenceable_status, skipdelays_status, output_changes, false );
	}
	
	// Activate individual outputs (if changes)
			
	if ( output_changes || refresh_outputs_needed || unsilenceable_status != prev_unsilenceable_status  ||
	     silenceable_status  != prev_silenceable_status  || input_changes)
	{	
		ActivateIndividualOutputs(  );
	}	
	
	// Get broadcast output status - just for the delay status...
	if ( current_broadcast_delays ||   silenceable_status != prev_silenceable_status || 
												unsilenceable_status != prev_unsilenceable_status )
	{
		current_broadcast_delays = RefreshOutputStatus( silenceable_status, unsilenceable_status, skipdelays_status, output_changes, true );
	}
	
	// Update GUI delay status
	HandleDelays( current_individual_delays | current_broadcast_delays );
	
	// Any events without current delays, set skip;
	skipdelays_status |= ~current_broadcast_delays;
	
	// dont try skip for events not set!
	skipdelays_status &= unsilenceable_status;
	
	uint32_t broadcast_mask = ( 1 << SITE_NUM_OUTPUT_ACTIONS ) - 1;
	

	// If broadcast status has changed - re-broadcast
	if ( ( silenceable_status   & broadcast_mask )  != prev_broadcast_silenceable_status ||
		  ( unsilenceable_status & broadcast_mask  ) != prev_broadcast_unsilenceable_status || 
	     ( skipdelays_status    & broadcast_mask  ) != prev_broadcast_skipdelays_status  ||
			refresh_silence )
	{
		prev_broadcast_silenceable_status   =  silenceable_status   & broadcast_mask;
		prev_broadcast_unsilenceable_status =	unsilenceable_status & broadcast_mask;
		prev_broadcast_skipdelays_status    =  skipdelays_status    & broadcast_mask;
		 
		ncu->QueueWriteMsgFast( true, NCU_BROADCAST_OUTPUT, this, NULL, prev_broadcast_silenceable_status, prev_broadcast_unsilenceable_status, prev_broadcast_skipdelays_status );									 						
		refresh_silence = false;
	}
	
	// Reset auto-silence if event change
	uint32_t mask = ACTION_BIT_FIRE | ACTION_BIT_FIRST_AID | ACTION_BIT_EVACUATION | ACTION_BIT_SECURITY;
	
	if ( silenceable_new_events & mask )
	{
		//reset timer
		silence_timer = now( );		
	}	
	
	if ( app.panel->autoSilence )
	if ( silence_timer + app.panel->autoSilence * 60 < now( ) )
	{
		//Send( MSG_SILENCE_BUTTON );
		Receive( EVENT_SILENCE_BUTTON );
		 
		silence_timer = UINT_MAX - 120 * 60;		
	}
	
	// Remember status
	prev_silenceable_status 	= silenceable_status;
	prev_unsilenceable_status 	= unsilenceable_status;	
	prev_skipdelays_status 		= skipdelays_status;	
	
	refresh_outputs_needed = false;

}
 

