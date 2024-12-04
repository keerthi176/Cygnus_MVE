/***************************************************************************
* File name: MM_Device.cpp
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
* device structures
*
**************************************************************************/

/* Defines
**************************************************************************/
#define SHOULDNT_HAPPEN 0


/* System Include Files
**************************************************************************/
#ifdef STM32F767xx
#include "stm32f7xx_hal_rtc.h"
#else
#include "stm32f4xx_hal_rtc.h"
#endif


#include <time.h>
#include <assert.h>


/* User Include Files
**************************************************************************/
#include "MM_Device.h"
#include "MM_CUtils.h"
#include "MM_Application.h"
#include "CO_Site.h"
#include "MM_IOStatus.h"
#include "MM_Log.h"



LogType GetLogType( int actionbit, char** typetext );

	

/*************************************************************************/
/**  \fn      Device::Device( void* ptr, Module* m )
***  \brief   Constructor for class
**************************************************************************/

Device::Device( void* ptr, DeviceConfig* conf, Module* m )
	: ptr( ptr ), owner( m ), config( conf )
{
	assert( conf != NULL );
	
	for( int ch = 0; ch < conf->numInputs; ch++ )
	{
		flags[ ch ] = 0;
		timeAsserted[ ch ] = 0;
		menuSelection[ ch/2] = 0;
	}
	
	for( int ch = 0; ch < conf->numOutputs; ch++ )
	{	
		flags[ ch ] = 0;
		current_profile[ ch] = UCHAR_MAX;
		profile[ ch] = UCHAR_MAX;
		
	}
	
	for( int ch = 0; ch < sizeof(analogue) ; ch++ )
	{	
		analogue[ ch ] = 0;
	}
	settings = 0;
}


/*************************************************************************/
/**  \fn      void Device::OutputChange( int channel, int action, int value )
***  \brief   Class function
**************************************************************************/
void Device::OutputChange( int channel, int action, int value )
{
	// for devices with no outputs
	assert( config->numOutputs == 0 );
}


/*************************************************************************/
/**  \fn      void Device::Indicate( int asserted )
***  \brief   Class function
**************************************************************************/

void Device::Indicate( int asserted, int prof )
{
	// assuming device can't indicate
}


/*************************************************************************/
/**  \fn      int Device::InputAction( int channel )
***  \brief   Class function
**************************************************************************/

int Device::InputAction( int channel )
{
	return config->input[ channel ].action;
}


/*************************************************************************/
/**  \fn      int Device::IsLatched( int channel )
***  \brief   Class function
**************************************************************************/

int Device::IsLatched( int channel )
{
	return config->input[ channel ].flags & CHANNEL_OPTION_LATCHING;
}	
 
 

/*************************************************************************/
/**  \fn      int Device::IsInputChannel( int channel )
***  \brief   Class function
**************************************************************************/

int Device::IsInputChannel( int channel )
{
	return config->input[ channel ].type != 0;
}	

/*************************************************************************/
/**  \fn      int Device::IsInputSkippingOutputDelays( int channel )
***  \brief   Class function
**************************************************************************/

int Device::IsInputSkippingOutputDelays( int channel )
{
   return config->input[ channel ].flags & CHANNEL_OPTION_SKIP_OUTPUT_DELAYS;
}

/*************************************************************************/
/**  \fn      int Device::IsInputSkippingOutputDelays( int channel )
***  \brief   Class function
**************************************************************************/

int Device::IsOutputIgnoringNightDelays( int channel )
{
   return config->output[ channel ].flags & CHANNEL_OPTION_NO_NIGHT_DELAY;
}


/*************************************************************************/
/**  \fn      int Device::IsInverted( int channel )
***  \brief   Class function
**************************************************************************/

int Device::IsInputInverted( int channel )
{
	return config->input[ channel ].flags & CHANNEL_OPTION_INVERTED;
}


/*************************************************************************/
/**  \fn      int Device::IsSustained( int channel, int& period )
***  \brief   Class function
**************************************************************************/

int Device::IsSustained( int channel, int& period )
{
	period = config->input[ channel ].delay;
	
	return period > 0;
}


/*************************************************************************/
/**  \fn      int Device::IsDisabled( )
***  \brief   Class function
**************************************************************************/

int Device::IsDisabled( )
{
	if ( app.site != NULL )
	{
		if ( ( IOStatus::zoneflags[ app.site->currentIndex ][ config->zone ] & ZONE_RULED_DISABLED ) ||
			  ( settings & SETTING_RULED_DISABLED ) || config->IsDisabled( ) )
		{
			return true;
		}
	}
	return false;
}


/*************************************************************************/
/**  \fn      int Device::IsInputDisabled( int channel )
***  \brief   Class function
**************************************************************************/

int Device::IsInputDisabled( int channel )
{
	if ( app.site != NULL )
	if ( ( settings & SETTING_RULED_DISABLED ) ||
		  ( IOStatus::zoneflags[ app.site->currentIndex ][ config->zone ] & ZONE_RULED_DISABLED ) ||
		  ( flags[ channel ] & INPUT_RULED_DISABLED ) ||
	     config->IsInputDisabled(  channel ) ) 
	{
		return true;
	}	
	return false;
}


/*************************************************************************/
/**  \fn      int Device::IsOutputDisabled( int channel )
***  \brief   Class function
**************************************************************************/

int Device::IsOutputDisabled( int channel )
{
	if ( app.site != NULL )
	if ( ( settings & SETTING_RULED_DISABLED ) ||
			 ( IOStatus::zoneflags[ app.site->currentIndex ][ config->zone ] & ZONE_RULED_DISABLED ) ||
			 ( flags[ channel ] & OUTPUT_RULED_DISABLED ) ||
			 config->IsOutputDisabled( channel ) )
	{
		return true;
	}	
 
	return false; 
}



/*************************************************************************/
/**  \fn      int Device::IsInputOnTest( int channel )
***  \brief   Class function
**************************************************************************/

int Device::IsInputOnTest( int channel )
{
	if ( app.site != NULL )
	if ( ( IOStatus::zoneflags[ app.site->currentIndex ][ config->zone ] & ZONE_ON_TEST ) ||
		  ( settings & SETTING_ON_TEST ) || IOStatus::test_everything ||
		  ( flags[ channel ] & INPUT_ON_TEST ) )
	{
		return true;
	}	
	
	return false;
}


/*************************************************************************/
/**  \fn      int Device::IsInputValid( int channel  )
***  \brief   Class function
**************************************************************************/

int Device::IsInputValid( int channel )
{
	int period;

	if ( flags[ channel ] & INPUT_LATCHED ) return true;
	 
	if ( ( flags[ channel ] & INPUT_ASSERTED ) && ( flags[ channel ] & INPUT_ACTIVE ) ) return true;
	
	if ( IsInputDisabled( channel ) ) return false;
		
	if ( flags[ channel ] & INPUT_ASSERTED )
	{
      if ( IsSustained( channel, period ) )
		{			
	 		if ( timeAsserted[ channel ] + period <= now( ) )
			{
				return true;
			}
		}
		else
		{
			return true;
		}
	}	
	
	return false;
}



/*************************************************************************/
/**  \fn      void IOStatus::Assert( InputDevice* id )
***  \brief   Class function
**************************************************************************/

void Device::InputAssert( int channel, time_t time, int value )
{
	
	if ( IsInputInverted( channel ) )
	{
		DeAssertInput( channel );
	}
	else
	{
		AssertInput( channel, time, value );
	}
}


/*************************************************************************/
/**  \fn      void Device::InputDeAssert( InputDevice* id )
***  \brief   Class function
**************************************************************************/

void Device::InputDeAssert( int channel, time_t time, int value )
{
	if ( IsInputInverted( channel ) )
	{
		AssertInput( channel, time, value );
	}
	else
	{
		DeAssertInput( channel );
	}
}


/*************************************************************************/
/**  \fn      void Device::UnSetTestMode( int channel )
***  \brief   Class function
**************************************************************************/

void Device::UnSetTestMode( int channel )
{
	flags[ channel ] &= ~INPUT_ON_TEST; 
}


/*************************************************************************/
/**  \fn      int Device::InTestMode( int channel )
***  \brief   Class function
**************************************************************************/

int Device::InTestMode( int channel )
{
	if ( app.site != NULL )
	if ( ( IOStatus::zoneflags[ app.site->currentIndex ][ config->zone ] & ZONE_ON_TEST ) ||
			  ( settings & SETTING_ON_TEST ) ||   
			  ( flags[ channel ] & INPUT_ON_TEST ) )
	{
		return true;
	}
	return false;
}


const char* GetChanType( int type );


/*************************************************************************/
/**  \fn      void  Device::AssertInput( InputDevice* id )
***  \brief   Private class function
**************************************************************************/

void Device::AssertInput( int channel, time_t time, int val )
{
	Indicate( true, config->input[ channel ].action );
	
	app.DebOut("dev %d ch % asserted\n", config->unit, channel );
	
	if ( IsInputOnTest( channel ) )
	{
		// Flag it
		if ( !( flags[ channel ] & INPUT_ACTIVE_ON_TEST ) )
		{
			flags[ channel ] |= INPUT_ACTIVE_ON_TEST;
			flags[ channel ] &= ~INPUT_NOTICED_ON_TEST;
		}
						
		Log::Msg( LOG_TST, "Zone %d unit %d %s '%s' active on test.", config->zone, config->unit, GetChanType( config->input[ channel ].type ), config->location );

		//  Update GUI
		Message m;
		m.to = app.FindModule( "EmbeddedWizard" );
		m.type = EW_ON_TEST_CHANGE_MSG;
		m.value = (int) config;
		app.Send( &m ); 	
	}
	else
	{	 		
		if ( !(flags[ channel ] & INPUT_ASSERTED  ) )
		{
			prevAssertion[ channel ] = timeAsserted[ channel ];
			timeAsserted[ channel ] = time;
			  
			flags[ channel ] &= ~( INPUT_SILENCED | INPUT_LATCHED | INPUT_IN_FAULTS );
			flags[ channel ] |= INPUT_ASSERTED;
		}		
	}	
	
}


/*************************************************************************/
/**  \fn      void IOStatus::DeAssert( InputDevice* id )
***  \brief   Private class function
**************************************************************************/

void Device::DeAssertInput( int channel )
{	 	
	flags[ channel ] &= ~INPUT_ACTIVE_ON_TEST;
	
	if ( IsInputOnTest( channel ) )
	{		//  Update GUI
		Message m;
		m.to = app.FindModule( "EmbeddedWizard" );
		m.type = EW_ON_TEST_CHANGE_MSG;
		app.Send( &m );
	}	 
	
	Indicate( false, config->input[ channel ].action );
 
	if ( flags[ channel ] & INPUT_ASSERTED ) 
	{
		if ( IsLatched( channel ) )
		{
			if ( !IsInputDisabled( channel ) )	flags[ channel ] |= INPUT_LATCHED;
		}
	}
	
	app.DebOut("dev %d ch % de-asserted\n", config->unit, channel );
	
	// unset assertion flag
	flags[ channel ] &= ~INPUT_ASSERTED;
}




/*************************************************************************/
// Output 
/*************************************************************************/

/*************************************************************************/
/**  \fn      void Device::OutputAssert( int channel, int action )
***  \brief   Class function
**************************************************************************/

void Device::OutputAssert( int channel, int action )
{
	if ( IsOutputInverted( channel ) )
	{
		if ( flags[ channel ] & OUTPUT_ASSERTED )
		{
			OutputChange( channel, 0, 0 );
			
			flags[ channel ] &= ~OUTPUT_ASSERTED;
		}			
	}
	else //if ( !(flags[ channel ] & OUTPUT_ASSERTED ) )
	{			
		OutputChange( channel, action, 1 );
		
		flags[ channel ] |= OUTPUT_ASSERTED;
	}
}


/*************************************************************************/
/**  \fn      void Device::OutputDeAssert( int channel )
***  \brief   Class function
**************************************************************************/

void Device::OutputDeAssert( int channel )
{
	if ( IsOutputInverted( channel ) )
	{
		if ( !(flags[ channel ] & OUTPUT_ASSERTED ) )
		{
			OutputChange( channel, 0, 1 );
			
			flags[ channel ] |= OUTPUT_ASSERTED;
		}		
	}	
	else if ( flags[ channel ] & OUTPUT_ASSERTED )
	{
		OutputChange( channel, 0, 0 );
		
		flags[ channel ] &= ~OUTPUT_ASSERTED;
	}	
}


/*************************************************************************/
/**  \fn      int Device::Actions( int channel )
***  \brief   Class function
**************************************************************************/

int Device::OutputActions( int channel )
{
	return config->output[ channel ].actions;
}


/*************************************************************************/
/**  \fn      int Device::IsOutputChannel( )
***  \brief   Class function
**************************************************************************/

int Device::IsOutputChannel( int channel )
{
	return config->output[ channel ].type != 0;
}	


/*************************************************************************/
/**  \fn      int Device::IsInverted( int )
***  \brief   Class function
**************************************************************************/

int Device::IsOutputInverted( int channel )
{
	return config->output[ channel ].flags & CHANNEL_OPTION_INVERTED;
}


/*************************************************************************/
/**  \fn      int Device::IsSilenceable( int )
***  \brief   Class function
**************************************************************************/

int Device::IsSilenceable( int channel )
{
	return config->output[ channel ].flags & CHANNEL_OPTION_SILENCEABLE;
}


/*************************************************************************/
/**  \fn      int Device::Broadcasted( )
***  \brief   Class function
**************************************************************************/

int Device::Broadcasted( )
{
	return false;
}


/*************************************************************************/
/**  \fn      int Device::Broadcast( unsigned int silenceable_status, unsigned int unsilenceable_status, unsigned int skip_bits )
***  \brief   Class function
**************************************************************************/

int Device::Broadcast( int silenceable_status, int unsilenceable_status, int skip_bits )
{
	return false;
}
