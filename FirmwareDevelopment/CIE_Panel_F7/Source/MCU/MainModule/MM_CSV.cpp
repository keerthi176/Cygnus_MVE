
/***************************************************************************
* File name: MM_CSV.cpp
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
* CSV Output module
*
**************************************************************************/

/* System Include Files
**************************************************************************/

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>


/* System Include Files
**************************************************************************/

#include "MM_CSV.h"
#include "CO_Site.h"
#include "MM_NCU.h"


/* Defines
**************************************************************************/


#define INPUT_DELAYS				( 1 << 0 )
#define SILENCE_GENERATION		( 1 << 1 )
#define RESET_GENERATION		( 1 << 2 )
#define NON_LATCHING				( 1 << 3 )
#define DELAYED_CALLPOINTS    ( 1 << 4 )
 

NCU* 			CSV::ncu = NULL;
IOStatus* 	CSV::iostatus = NULL;


/*************************************************************************/
/**  \fn      int  CSV::CSV( )
***  \brief   Class constructor
**************************************************************************/

CSV::CSV( ) : Module( "CSV", 0 )
{
	
}


/*************************************************************************/
/**  \fn      int  CSV::Init( )
***  \brief   Class function
**************************************************************************/

int CSV::Init( )
{
	assert( ncu = (NCU*) app.FindModule( "NCU" ) );
	assert( iostatus = (IOStatus*) app.FindModule( "IOStatus" ) );
	
	return true;
}


/*************************************************************************/
/**  \fn      int  CSV::EN54( DeviceConfig* dc)
***  \brief   Class function
**************************************************************************/

int CSV::EN54( DeviceConfig* dc )
{
	int r = 0;

	for( InputChannel* i = dc->input; i < dc->input + dc->numInputs; i++ )
	{
		if ( i->delay > 0 )
		{
			r |= INPUT_DELAYS;
		}
	 
		if ( i->action == ACTION_SHIFT_SILENCE )
		{
			r |= SILENCE_GENERATION;
		}
	 
		if ( i->action == ACTION_SHIFT_RESET )
		{
			r |= RESET_GENERATION;
		}
	 
		if ( !(i->flags & CHANNEL_OPTION_SKIP_OUTPUT_DELAYS ) )
		if ( i->type == CO_CHANNEL_PANEL_CALLPOINT || i->type == CO_CHANNEL_PANEL_EVACUATE_BUTTON || i->type == CO_CHANNEL_FIRE_CALLPOINT_E )
		{
			r |= DELAYED_CALLPOINTS;			
		}
	 
		if ( i->action == ACTION_SHIFT_FIRE )
		if ( !( i->flags & CHANNEL_OPTION_LATCHING ) )
		{
			r |= NON_LATCHING;
		}
	}

	return r;
}

/*************************************************************************/
/**  \fn      int CSV::GetLine( char* buff, int line )
***  \brief   Class function
**************************************************************************/

int CSV::GetLine( char* buff, int line )
{
	if ( !line )
	{
		strcpy( buff, "\"Zone\",\"Unit\",\"Type\",\"Primary RSSI\",\"Secondary RSSI\",\"Ambient Heat\",\"Ambient Smoke\",\"Battery\",\"Backup Battery\",\"Parent\",\"Secondary\",\"Primary Children\",\
		\"Secondary Children\",\"Rank\",\"Input Delays\",\"Silence Generation\",\"Reset Generation\",\"Non Latching\",\"Delayed Callpoint\",\"Location\"\n" );
		return true;
	}
	else if ( app.panel != NULL )
	{
		DeviceConfig* dc = app.panel->devs;
		int n = 0;
		
		while( dc < app.panel->devs + app.panel->NumDevs( ) )
		{
			if ( dc->type < 64 ) // mesh device
			{
				if ( ++n == line )
				{
					RadioUnit* ru = ncu->FindUnit( dc->unit );
					Device* dev = iostatus->FindDevice( dc->unit );
					
					char heat = 0;
					char smoke = 0;
					
					int r = EN54( dc );
					
					if ( dev != NULL )
					{
						for( InputChannel* i = dc->input; i < dc->input + sizeof( dev->analogue ); i++ )
						{
							if ( i->type == CO_CHANNEL_HEAT_A1R_E || i->type == CO_CHANNEL_HEAT_B_E )
							{
								heat = dev->analogue[ i - dc->input ];
							}
							if ( i->type == CO_CHANNEL_SMOKE_E )
							{
								smoke = dev->analogue[ i - dc->input ];
							}
						}
					}		
					
					if ( ru != NULL )
					{
						snprintf( buff, 255, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\"%s\"\n", dc->zone, dc->unit, dc->type, ru->rssiprimary,  ru->rssisecondary, heat, smoke, ru->battprimary, ru->battbackup,
										ru->primary, ru->secondary, ncu->NumPrimaryChildren( dc->unit ), ncu->NumSecondaryChildren( dc->unit ), ru->rank,
										!!( r & INPUT_DELAYS ), !!( r & SILENCE_GENERATION ), !!( r & RESET_GENERATION ), !!( r & NON_LATCHING ), !!( r & DELAYED_CALLPOINTS ), dc->location );
					}
					else
					{
						snprintf( buff, 255, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\"%s\"\n", dc->zone, dc->unit, dc->type, -1, -1, heat, smoke, -1, -1, -1, -1, 0, 0, 0,
									!!( r & INPUT_DELAYS ), !!( r & SILENCE_GENERATION ), !!( r & RESET_GENERATION ), !!( r & NON_LATCHING ), !!( r & DELAYED_CALLPOINTS ), dc->location );
					}
					return true;
				}
			}
			dc++;
		}
	}
	
	*buff = 0;
	return false;	
}
		