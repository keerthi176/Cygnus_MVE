/***************************************************************************
* File name: CO_Site.cpp
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
* Site definitions
*
**************************************************************************/


#include "CO_Site.h"
#include "MM_Application.h"
#include "MM_RealTimeClock.h"

/* Defines
**************************************************************************/


#define MAX(val1,val2) ((val1 > val2) ? val1 : val2) 
		

/*************************************************************************/
/**  \fn      int DeviceConfig::IsDisabled(  )
***  \brief   Class function
**************************************************************************/

int DeviceConfig::IsDisabled(  )
{
	if ( app.panel == NULL ) return false;
	
	Zone* z = Settings::SiteZone( zone );
			
	if ( z != NULL )
	{
		if ( z->flags & ZONEFLAG_DISABLED )
		{
			return true;
		}
	}
	 
	if ( flags & DEVCONFIG_DISABLED ) return true;
	
	return false;
}	


/*************************************************************************/
/**  \fn      int DeviceConfig::IsInputDisabled( int i, int c )
***  \brief   Class function
**************************************************************************/

int DeviceConfig::IsInputDisabled( int c )
{
	if ( IsDisabled( ) ) return true;
	
	if ( ( input[ c ].flags & CHANNEL_OPTION_DISABLED_DAYTIME ) && RealTimeClock::daytime )
	{
		return true;
	}
	
	if ( ( input[ c ].flags & CHANNEL_OPTION_DISABLED_NIGHTTIME ) && !RealTimeClock::daytime )
	{
		return true;
	}
	return false;	
}





/*************************************************************************/
/**  \fn      int DeviceConfig::IsOutputDisabled( int c )
***  \brief   Class function
**************************************************************************/

int DeviceConfig::IsOutputDisabled( int c )
{
	if ( IsDisabled( ) ) return true;
	 
	if ( app.panel->settings & SITE_SETTING_FIRE_ALARMS_DISABLED )
	{
		if ( IsAlarmOutputChannel( c ) )
		{
			return true;
		}
	}
	 	
	if ( app.panel->settings & SITE_SETTING_FIRE_ROUTING_DISABLED )
	{
		if ( IsRoutingOutputChannel( c ) )
		{
			return true;
		}
	}
		
	if ( ( output[ c ].flags & CHANNEL_OPTION_DISABLED_ANYTIME ) )
	{
		return true;
	}	
	return false;
}



/*************************************************************************/
/**  \fn      int DeviceConfig::InitialDelay( int channel )
***  \brief   Class function
**************************************************************************/

int DeviceConfig::InitialDelay( int channel )
{
	if ( app.panel == NULL ) return 0;
	
	if ( !RealTimeClock::daytime && ( output[ channel ].flags & CHANNEL_OPTION_NO_NIGHT_DELAY ) )
	{
		return 0;
	}
	
	if ( ( ( app.panel->settings & SITE_SETTING_GLOBAL_DELAY_OVERRIDE ) &&
						 ( output[ channel ].actions & ACTION_BIT_FIRE ) ) ||
					    ( output[ channel ].flags & CHANNEL_OPTION_USE_GLOBAL_DELAY ) )
	{
		return app.panel->delay1;
	}
	return output[ channel ].delay1;
}


/*************************************************************************/
/**  \fn      int DeviceConfig::InvestigativeDelay( int c )
***  \brief   Class function
**************************************************************************/

int DeviceConfig::InvestigativeDelay( int channel )
{
	if ( app.panel == NULL ) return 0;
	
	if ( !RealTimeClock::daytime && ( output[ channel ].flags & CHANNEL_OPTION_NO_NIGHT_DELAY ) )
	{
		return 0;
	}
	
	if ( ( ( app.panel->settings & SITE_SETTING_GLOBAL_DELAY_OVERRIDE ) &&
						 ( output[ channel ].actions & ACTION_BIT_FIRE ) ) ||
					  ( output[ channel ].flags & CHANNEL_OPTION_USE_GLOBAL_DELAY ) )
	{
		return app.panel->delay2;
	}
	return output[ channel ].delay2;
}


/*************************************************************************/
/**  \fn      RadioDevice::IsIOUnit( )
***  \brief   Class function
**************************************************************************/

int DeviceConfig::IsIOUnit( )
{
	for( int i = 0; i < numInputs; i++ )
	{
		if ( input[ i].type >= CO_CHANNEL_INPUT_1_E && input[ i].type <= CO_CHANNEL_INPUT_32_E )
		{
			return true;
		}
	}
	for( int o = 0; o < numOutputs; o++ )
	{
		if ( output[ o].type >= CO_CHANNEL_OUTPUT_1_E && output[ o].type <= CO_CHANNEL_OUTPUT_16_E )
		{
			return true;
		}
	}
	return false;
}


/*************************************************************************/
/**  \fn      RadioDevice::Command( char* cmd )
***  \brief   Class function
**************************************************************************/

int DeviceConfig::Push( Circular<ATCmd>* buff, const char* fmt, ... )
{
	if ( fmt != NULL )
	{
		ATCmd cmd;
		va_list args;
		
		va_start( args, fmt );
		
		cmd.fmt = fmt;
		
		for( int n = 0; n < ATCMD_MAX_ARGS; n++ )
		{
			cmd.param[ n] = va_arg( args, int );
		}
		
		buff->Push( cmd );
		 
		va_end( args );
		
		return true;
	}	
	return false;
}


/*************************************************************************/
/**  \fn      void RadioDevice::IsInputChannel( int ch, int& index )
***  \brief   Class function
**************************************************************************/

int DeviceConfig::IsInputChannel( int ch, int& index )
{
	for( InputChannel* i = input; i < input + numInputs; i++ )
	{
		if ( i->type == ch ) 
		{
			index = i - input;
			return true;
		}
	}
	return false;
}


/*************************************************************************/
/**  \fn      void RadioDevice::IsOutputChannel( int ch, int& index )
***  \brief   Class function
**************************************************************************/

int DeviceConfig::IsOutputChannel( int ch, int& index )
{
	for( OutputChannel* o = output; o < output + numOutputs; o++ )
	{
		if ( o->type == ch ) 
		{
			index = o - output;
			return true;
		}
	}
	return false;
}


/*************************************************************************/
/**  \fn      void RadioDevice::IsAlarmOutputDevice( )
***  \brief   Class function
**************************************************************************/

int DeviceConfig::IsAlarmOutputChannel( int ch )
{
	OutputChannel* o = output + ch;
	
	if ( 
		o->type == CO_CHANNEL_SOUNDER_E ||
		o->type == CO_CHANNEL_PANEL_SOUNDERS_RELAY || 
		o->type == CO_CHANNEL_PANEL_FIRE_RELAY || 
		o->type == CO_CHANNEL_PANEL_ALARM_RELAY ||
		o->type == CO_CHANNEL_BEACON_E ||
		o->type == CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E ||
		o->type == CO_CHANNEL_VISUAL_INDICATOR_E )
	{
		return true;
	}
	
	return false;
}

/*************************************************************************/
/**  \fn      void RadioDevice::IsAlarmOutputDevice( )
***  \brief   Class function
**************************************************************************/

int DeviceConfig::IsRoutingOutputChannel( int ch )
{
	OutputChannel* o = output + ch;
	
	if ( o->type == CO_CHANNEL_PANEL_ROUTING_OUTPUT ||
		  o->type == CO_CHANNEL_OUTPUT_ROUTING_E  )
	{
		return true;
	}
	
	return false;
}

/*************************************************************************/
/**  \fn      void RadioDevice::IsAlarmOutputDevice( )
***  \brief   Class function
**************************************************************************/

int DeviceConfig::IsAlarmOutputDevice( )
{
	for( int ch = 0; ch < numOutputs; ch++ )
	{
		if ( IsAlarmOutputChannel( ch ) )
		{
			return true;
		}
	}
	return false;
}


/*************************************************************************/
/**  \fn      void RadioDevice::SetupDevice( )
***  \brief   Class function
**************************************************************************/

void DeviceConfig::SetupStart( Circular<ATCmd>* buff )
{
	if ( app.panel != NULL )
	{
		Push( buff, "UA=%d", unit );
		Push( buff, "SYNC=%d", 0 );
		Push( buff, "FREQ=%d", app.panel->freq ); 
		Push( buff, "DEVCF=%d", type );
		Push( buff, "SYSID=%d", app.panel->systemId );
	//	Push( buff, "SERNO=%04d-%02d-%04d", 0, 0, 0 );
		Push( buff, "TXPLO=%d", 7 );
		Push( buff, "TXPHI=%d", 10 );
		Push( buff, "ZONE=%d", zone );
		Push( buff, "PFM=%d,%d", -1, -1 );
		Push( buff, "FLEN=%d", MAX( 16, ( app.panel->maxDevices + 1 ) / 4) );
		
		Push( buff, "DWRAP=%d", ( app.panel->maxDevices + 1 ) * 2 );
		
	
		if ( IsIOUnit( ) )
		{
			Push( buff, "IOUTH=%d,%d,%d", 333, 1728, 3443 );
			Push( buff, "IOUP=%d", 1000 );
		}
	}
}


/*************************************************************************/
/**  \fn      RadioDevice::SetupDevice( Circular<ATCmd>* buff )
***  \brief   Class function
**************************************************************************/

void DeviceConfig::SetupProfiles( Circular<ATCmd>* buff )
{
	for( int i = 0; i < SITE_NUM_PROFILES; i++ )
	{
		Profile* prof = app.site->profiles;
		
		Push( buff, "PRF=%d,%d,%d,%d", i, prof->tone, prof->flash, prof->ext_sounder );
	}
}


/*************************************************************************/
/**  \fn      void RadioDevice::SetupInputs( )
***  \brief   Class function
**************************************************************************/

void DeviceConfig::SetupInputs( Circular<ATCmd>* buff )
{	
	for( int n = 0; n < numInputs; n++ )
	{
		int flags = ( input[ n ].flags & CHANNEL_OPTION_DISABLED_DAYTIME ) ? 0 : 8;
		flags += ( input[ n ].flags & CHANNEL_OPTION_DISABLED_NIGHTTIME ) ? 0 : 16;
		
		Push( buff, "CHF=%d,%d", input[ n].type, flags );
	}
}


/*************************************************************************/
/**  \fn      void RadioDevice::SetupOutputs( )
***  \brief   Class function
**************************************************************************/

void DeviceConfig::SetupOutputs( Circular<ATCmd>* buff )
{
	for( int n = 0; n < numOutputs; n++ )
	{
		int chan = output[ n].type;
		int flags = output[ n].flags & CHANNEL_OPTION_INVERTED     	 	? 1 : 0;
		flags |=    output[ n].flags & CHANNEL_OPTION_NO_NIGHT_DELAY 	? 2 : 0;
		flags |=    output[ n].flags & CHANNEL_OPTION_SILENCEABLE  		? 4 : 0;
		flags |=    output[ n].flags & CHANNEL_OPTION_DISABLED_ANYTIME ? 0 : 8 + 16;
		
		Push( buff, "CHF=%d,%d", chan, flags );
		Push( buff, "ACS=%d,%d", chan, output[ n].actions );
		Push( buff, "ADC=%d,%d,%d", chan, output[ n].delay1, output[ n].delay2 );
	}
	for( int p = 0; p < numParams; p++ )
	{
		if ( param[ p].code == Param_Volume && param[ p].channel == CO_CHANNEL_SOUNDER_E )
		{
			Push( buff, "SL=%d,%d", param[ p].channel, param[ p].value );
		}	
	}
}	

