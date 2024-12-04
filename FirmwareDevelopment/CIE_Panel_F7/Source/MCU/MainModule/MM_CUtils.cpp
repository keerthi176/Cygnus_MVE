/***************************************************************************
* File name: MM_CUtils.cpp
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
* C Utiltities
*
**************************************************************************/

/* System Include Files
**************************************************************************/
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#ifdef STM32F767xx	
#include "stm32f7xx_hal_rtc.h"
#else
#include "stm32f4xx_hal_rtc.h"
#endif

#include "MM_Application.h"	


/* User Include Files
**************************************************************************/

	
#include "CO_RBU.h"
#include "CO_Site.h"
#include "MM_CUtils.h"
#include "MM_NCU.h"
#include "MM_PanelIO.h"


extern "C"
{
	
void mattcpy( void* dst, void* src, int bytes )
{
	char* end = (char*) dst + bytes;
	char* srcp = (char*) src;
	
	for( char* p = (char*)dst; p < end; p++ ) *p = *srcp++;
}
	
Application* capp = &app; 	
	
/*************************************************************************/
/**  \fn      RTC_HandleTypeDef* GetRTC( )
***  \brief   Global helper function
**************************************************************************/

void event( Event e )
{
	app.Send( e );
}


/*************************************************************************/
/**  \fn      time_t Now( )
***  \brief   Global helper function
**************************************************************************/

time_t now( )
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	
	assert( app.rtc != NULL );
	
	HAL_RTC_GetTime( app.rtc, &time, RTC_FORMAT_BIN );	
	HAL_RTC_GetDate( app.rtc, &date, RTC_FORMAT_BIN );
	
	return RTCToUnix( &time, &date );
}
	

/*************************************************************************/
/**  \fn      DeviceConfig* get_dev_config( int unit )
***  \brief   Global helper function
**************************************************************************/

DeviceConfig* get_dev_config( int unit )
{
	if ( app.panel != NULL )
	for( DeviceConfig* d = app.panel->devs; d < app.panel->devs + app.panel->NumDevs( ); d++ )
	{
		if ( d->unit == unit ) return d;
	}
	return NULL;
}	
 

/*************************************************************************/
/**  \fn      RadioUnit* get_child_unit( RadioUnit* start, int parent )
***  \brief   Global helper function
**************************************************************************/

RadioUnit* get_child_unit( RadioUnit* start, int parent )
{
	if ( start == NULL )
	{
		start = NCU::units;
	}
	else
	{
		start++;
	}
	
	for( RadioUnit* ru = start; ru < NCU::unit_pos; ru++ )
	{
		if ( ru->primary == parent )
		{
			return ru;
		}
	}
 
	return NULL;
}	


/*************************************************************************/
/**  \fn      int get_num_devs_in_zone( int zone )
***  \brief   Global helper function
**************************************************************************/

int get_num_devs_in_zone( int zone )
{
	if ( app.panel != NULL )
	{				
		int count = 0;
		 	
		if ( app.panel != NULL )
		{						
			// if alarm output
			if ( zone == GROUP_FIRE_ALARMS )
			{
				for( DeviceConfig* d = app.panel->devs; d < app.panel->devs + app.panel->NumDevs( ); d++ ) 
				{
					if ( d->IsAlarmOutputDevice( ) )
					{	
						count++;
					}
				}
			}
			// else if routing device
			else if ( zone == GROUP_FIRE_ROUTING )
			{
				DeviceConfig* d = app.panel->devs;
				
				while ( d != NULL )
				{
					d = Settings::FindFireRoutingDevice( d ) ;
					if ( d != NULL )
					{ 
						count++;  
						d++;
					}
				}
			}
			else // normal zone
			{
				for( DeviceConfig* d = app.panel->devs; d < app.panel->devs + app.panel->NumDevs( ); d++ ) 
				{
					if ( d->zone == zone )
					{
						count++;
					}
				}
			}
			
		}
		return count;
	}		
	else return 0;
}	



/*************************************************************************/
/**  \fn      time_t RTCToUnix( RTC_TimeTypeDef* t, RTC_DateTypeDef* d )
***  \brief   Global helper function
**************************************************************************/

time_t RTCToUnix( RTC_TimeTypeDef* t, RTC_DateTypeDef* d )
{
	assert( t != NULL );
	assert( d != NULL );
	
	struct tm tim = {0};

	uint8_t hh = t->Hours;
	uint8_t mm = t->Minutes;
	uint8_t ss = t->Seconds;
	uint8_t dd = d->Date;
	uint8_t m = d->Month;
	uint16_t y = d->Year;
	uint16_t yr = (uint16_t)(y+2000-1900);

	tim.tm_year = yr;
	tim.tm_mon = m - 1;
	tim.tm_mday = dd;
	tim.tm_hour = hh;
	tim.tm_min = mm;
	tim.tm_sec = ss;
	
	return mktime( &tim );	
}

static const char* day[ 7] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };

/*************************************************************************/
/**  \fn      char* Now( );
***  \brief   Global helper function
**************************************************************************/

char* strnow( )
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	
	static char out[ 12 ];
	
	*out = 0;
	
	if ( app.rtc != NULL )
	{	
		HAL_RTC_GetTime( app.rtc, &time, RTC_FORMAT_BIN );	
		HAL_RTC_GetDate( app.rtc, &date, RTC_FORMAT_BIN );
		
		snprintf( out, 12, "%02d:%02d %s", time.Hours, time.Minutes, day[ date.WeekDay - 1] );
	}
	return out;
}
	

}
