/***************************************************************************
* File name: MM_Alarms.cpp
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
* Alarm list management
*
**************************************************************************/
/* System Include Files
**************************************************************************/
 
/* Defines
**************************************************************************/
#define ITER(T) std::__rw_list_iter<T, std::ptrdiff_t,T*,T&>


/* User Include Files
**************************************************************************/
#include "MM_Alarms.h"
#include "MM_Application.h"


/*************************************************************************/
/**  \fn      Alarm::Alarm( char h, char m, char d, char k, void (*f)
***  \brief   Constructor for class
**************************************************************************/

Alarm::Alarm( char h, char m, char d, char k, void (*f)( Module*), Module* u ) : hour(h), min(m), wday(d), keep(k), func(f), user(u)
{
	
}

/*************************************************************************/
/**  \fn      Alarm::Alarm( )
***  \brief   Constructor for class
**************************************************************************/

Alarm::Alarm( ) : hour(0), min(0), wday(0), keep(0), func( NULL ), user( NULL )
{
	
}

static Alarms* nasty_global_this = NULL;
/*************************************************************************/
/**  \fn      static bool compare( const Alarm& first, const Alarm& second )
***  \brief   Local helper function
**************************************************************************/

static bool compare( const Alarm& first, const Alarm& second )
{  
   return ( nasty_global_this->SecondsTill( first ) < nasty_global_this->SecondsTill( second ) );  // return ealiest
}


	
/*************************************************************************/
/**  \fn      Alarms::Alarms( )
***  \brief   Constructor for class
**************************************************************************/

Alarms::Alarms( ) : Module( "Alarms", 0 )
{

}

#define SECS(d,h,m,s) ((d)*24*60*60+(h)*60*60+(m)*60+(s)) 

/*************************************************************************/
/**  \fn      int Alarms::SecondsTill( const Alarm& b )
***  \brief   Class function
**************************************************************************/

int Alarms::SecondsTill( const Alarm& b )
{
	RTC_TimeTypeDef now;
	int day = ALARMS_EVERY_DAY;
	
	if ( b.wday != ALARMS_EVERY_DAY )
	{
		RTC_DateTypeDef today;
		HAL_RTC_GetDate( app.rtc, &today, RTC_FORMAT_BIN );
		
		day = today.WeekDay;
	}
	
	HAL_RTC_GetTime( app.rtc, &now, RTC_FORMAT_BIN );
	
	return SECS( b.wday, b.hour, b.min, 0 ) - SECS( day, now.Hours, now.Minutes, now.Seconds );
}	


/*************************************************************************/
/**  \fn      void Alarms::Add( Alarm& alarm )
***  \brief   Class function
**************************************************************************/

void Alarms::Add( Alarm& alarm )
{	
	int secs = SecondsTill( alarm );
	
	// If time hasnt already passed
	if ( secs >= -1 )
	{
		// add it
		alarms.push_back( alarm );
	
		// re-order list
		nasty_global_this = this;
		alarms.sort( &compare );
	
		// get earliest
		Alarm& earliest = alarms.front( );
	
		// activate it 
		Set( earliest );
	}
}


/*************************************************************************/
/**  \fn      HAL_StatusTypeDef Alarms::Set( Alarm& a )
***  \brief   Class function
**************************************************************************/

HAL_StatusTypeDef Alarms::Set( Alarm& a )
{
	RTC_AlarmTypeDef sAlarm;
	//RTC_DateTypeDef today;
	
	//HAL_RTC_GetDate( app.rtc, &today, RTC_FORMAT_BIN );
	
   sAlarm.AlarmTime.Hours = a.hour;
   sAlarm.AlarmTime.Minutes = a.min;
   sAlarm.AlarmTime.Seconds = 0;
   sAlarm.AlarmTime.SubSeconds = 0;
   sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
   sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
   sAlarm.AlarmMask = a.wday == ALARMS_EVERY_DAY ? RTC_ALARMMASK_DATEWEEKDAY : RTC_ALARMMASK_NONE;
   sAlarm.AlarmSubSecondMask  = RTC_ALARMSUBSECONDMASK_ALL;
   sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
   sAlarm.AlarmDateWeekDay    = a.wday;
   sAlarm.Alarm = RTC_ALARM_B;	
	
	return HAL_RTC_SetAlarm_IT( app.rtc, &sAlarm, RTC_FORMAT_BIN);
}



/*************************************************************************/
/**  \fn      int Alarms::Receive( Command* )
***  \brief   Class function
**************************************************************************/

int Alarms::Receive( Command* )
{
	return CMD_ERR_UNKNOWN_CMD;
}



/*************************************************************************/
/**  \fn      void HAL_RTC_AlarmAEventCallback( RTC_HandleTypeDef* hrtc )
***  \brief   Global helper function
**************************************************************************/

extern "C" void HAL_RTC_AlarmAEventCallback( RTC_HandleTypeDef* hrtc )
{
	 
}

/*************************************************************************/
/**  \fn      void HAL_RTCEx_AlarmBEventCallback( RTC_HandleTypeDef* hrtc )
***  \brief   Global helper function
**************************************************************************/

extern "C" void HAL_RTCEx_AlarmBEventCallback( RTC_HandleTypeDef* hrtc )
{
	// TODO:
	/*
	if ( event == EVENT_RTC_ALARM_B )
	{
		Alarm& alarm = alarms.front( );
		
		if ( !alarm.keep )
		{
			alarms.pop_front( );
		}
		
		alarm.func( alarm.user );
		
		// Set next alarm in list if exists
		if ( alarms.size( ) > 0 )
		{
			alarms.sort( &compare );
			Set( alarms.front( ) );
		}	
	}
	return 0;	 */
}


/*************************************************************************/
/**  \fn      int Alarms::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int Alarms::Receive( Event event )
{
	return 0;
} 


/*************************************************************************/
/**  \fn      int Backlight::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int Alarms::Receive( Message* )
{
   return 0;
}
