/***************************************************************************
* File name: GPIO.cpp
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
* GPIO control module
*
**************************************************************************/

/* System Include Files
**************************************************************************/
#include <stdlib.h>
#include <time.h>


/* User Include Files
**************************************************************************/

#include "CO_RBU.h"
#include "CO_DataStorage.h"
#include "MM_CUtils.h"


/*************************************************************************/
/**  \fn      DEVICE* FindConfigDevice( DEVICE* base, int n, int zone, int unit )
***  \brief   Global helper function
**************************************************************************/

DEVICE* FindConfigDevice( DEVICE* base, int n, int zone, int unit )
{
	for( DEVICE* r = base; r < base + n; r++ )
	{
		if ( r->rZone == zone && r->rUnit == unit )
		{
			 return r;
		}
	}
	return NULL;
}


/*************************************************************************/
/**  \fn      RadioUnit* FindRadioDevice( RadioUnit* base, int n, int zone, int unit )
***  \brief   Global helper function
**************************************************************************/

RadioUnit* FindRadioDevice( RadioUnit* base, int n, int zone, int unit )
{
	for( RadioUnit* d = base; d < base + n; d++ )
	{
		if ( d->zone == zone && d->unit == unit )
		{
			 return d;
		}
	}
	return NULL;
}


/*************************************************************************/
/**  \fn      time_t Now( RTC_HandleTypeDef* rtc )
***  \brief   Global helper function
**************************************************************************/

time_t Now( RTC_HandleTypeDef* rtc )
{
	RTC_TimeTypeDef time;
	RTC_DateTypeDef date;
	
	HAL_RTC_GetTime( rtc, &time, RTC_FORMAT_BIN );	
	HAL_RTC_GetDate( rtc, &date, RTC_FORMAT_BIN );
	
	return RTCToUnix( &time, &date );
}
	
	
/*************************************************************************/
/**  \fn      time_t RTCToUnix( RTC_TimeTypeDef* t, RTC_DateTypeDef* d )
***  \brief   Global helper function
**************************************************************************/

time_t RTCToUnix( RTC_TimeTypeDef* t, RTC_DateTypeDef* d )
{
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

