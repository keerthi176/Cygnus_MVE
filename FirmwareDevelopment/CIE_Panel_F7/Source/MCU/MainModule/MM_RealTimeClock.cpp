/***************************************************************************
* File name: MM_RealTimeClock.cpp
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
* First written on 30/05/18 by Matt Ault
*
* File History:
* Change History:
* Rev 	Date 			Description 		Author
* 1.0 	30/05/18 	Initial File
*
* Description:
* RTC Module
*
**************************************************************************/

 

/* System Include Files
**************************************************************************/
#ifdef STM32F767xx
#include "stm32f7xx_hal_rtc.h"
#else
#include "stm32f4xx_hal_rtc.h"
#endif
#include "_CoreTime.h"

/* User Include Files
**************************************************************************/
#include "MM_RealTimeClock.h"
#include "MM_Command2.h"
#include "MM_Application.h"
#include "MM_Settings.h"
#include "MM_Alarms.h"
#include "MM_EEPROM.h"
#include "CO_Site.h"
#include "MM_TouchScreen.h"
#include "MM_CUtils.h"


/* Defines
**************************************************************************/


int RealTimeClock::daytime;


/*************************************************************************/
/**  \fn      RealTimeClock::RealTimeClock( )
***  \brief   Constructor for class
**************************************************************************/

RealTimeClock::RealTimeClock( ) : Module( "RealTimeClock", 0, EVENT_RESET )
{

	
}


/*************************************************************************/
/**  \fn      int RealTimeClock::Init( )
***  \brief   Second constructor for class
**************************************************************************/

int RealTimeClock::Init( ) 
{
	assert( ( qspi = (QuadSPI*) app.FindModule( "QuadSPI" ) ) );
	assert( ( settings = (Settings*) app.FindModule( "Settings" ) ) );
		
	return true;	
}

/*************************************************************************/
/**  \fn      int RealTimeClock::Today( )
***  \brief   CClass funtion
**************************************************************************/


int RealTimeClock::Today( )
{
	RTC_DateTypeDef today;
	HAL_RTC_GetDate( app.rtc, &today, RTC_FORMAT_BIN );
	
	return today.WeekDay;
}


/*************************************************************************/
/**  \fn      void RealTimeClock::Tomorrow( )
***  \brief   CClass funtion
**************************************************************************/

int RealTimeClock::Tomorrow( )
{
	RTC_DateTypeDef today;
	HAL_RTC_GetDate( app.rtc, &today, RTC_FORMAT_BIN );
	
 
	int tomo = today.WeekDay + 1;
	if ( tomo > RTC_WEEKDAY_SUNDAY ) tomo = RTC_WEEKDAY_MONDAY;

	return tomo;
}


/*************************************************************************/
/**  \fn      void RealTimeClock::CheckDayTime( )
***  \brief   CClass funtion
**************************************************************************/

void RealTimeClock::CheckDayTime( )
{
	// What time is it?
	RTC_TimeTypeDef now;
	HAL_RTC_GetTime( app.rtc, &now, RTC_FORMAT_BIN );
		
	app.DebOut( "time: %d:%d.%d\n", now.Hours, now.Minutes, now.Seconds );
	
	if ( app.panel != NULL )
	{
		int today = Today( );
		int tomorrow  = Tomorrow( );
		
		Daytime* tod = app.panel->day + ( today - 1 );	
		Daytime* tom = app.panel->day + ( tomorrow - 1 );
		
		// if after day start
		if ( now.Hours > tod->start.hour || ( now.Hours == tod->start.hour && now.Minutes >= tod->start.minute ))
		{
			// if before day end
			if ( now.Hours < tod->end.hour   || ( now.Hours == tod->end.hour && now.Minutes < tod->end.minute ))
			{
				daytime = true;
				app.Send( EVENT_DAY_TIME );
				SetAlarm( ALARM_DAYTIME, tom->start.hour, tom->start.minute, tomorrow );
				SetAlarm( ALARM_NIGHTTIME, tod->end.hour, tod->end.minute, today );
			}
			else // is after day end
			{
				daytime = false;
				app.Send( EVENT_NIGHT_TIME );
				SetAlarm( ALARM_DAYTIME, tom->start.hour, tom->start.minute, tomorrow );
				SetAlarm( ALARM_NIGHTTIME, tom->end.hour, tom->end.minute, tomorrow );
			}
		}			
		else  // before day start
		{
			daytime = false;
			app.Send( EVENT_NIGHT_TIME );
			SetAlarm( ALARM_DAYTIME, tod->start.hour, tod->start.minute, today );
			SetAlarm( ALARM_NIGHTTIME, tod->end.hour, tod->end.minute, today );
		}			
		app.Send( EVENT_TIME_CHANGED );
	}	
}

 
/*************************************************************************/
/**  \fn      RealTimeClock::SetAlarm( AlarmPeriod period, int hour, int min, int day )
***  \brief   Class function
**************************************************************************/

void RealTimeClock::SetAlarm( AlarmPeriod period, int hour, int min, int day )
{
	RTC_AlarmTypeDef a;

	a.AlarmTime.Hours 	= hour;
	a.AlarmTime.Minutes 	= min;
	a.AlarmMask 			= RTC_ALARMMASK_SECONDS;
	a.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	a.AlarmDateWeekDaySel= RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	a.AlarmDateWeekDay   = day;
	a.Alarm 					= period == ALARM_DAYTIME ? RTC_ALARM_A : RTC_ALARM_B;
	
	HAL_RTC_SetAlarm_IT( app.rtc, &a, RTC_FORMAT_BIN );
}

 
/*************************************************************************/
/**  \fn      RTC_TimeTypeDef GetTime( Command* cmd )
***  \brief   Global helper function
**************************************************************************/

static RTC_TimeTypeDef GetTime( Command* cmd )
{
	RTC_TimeTypeDef sTime = { 0 };
	
	sTime.Hours 	= cmd->int0;
	sTime.Minutes 	= cmd->int1;
	sTime.Seconds 	= cmd->int2;	
	
	return sTime;
}


/**
  * @brief  Converts Command payload to date struct
  * @param  Message sent
  * @retval Result status
  */
/*************************************************************************/
/**  \fn      RTC_DateTypeDef Date( Command* cmd )
***  \brief   Global helper function
**************************************************************************/

static RTC_DateTypeDef Date( Command* cmd )
{
	RTC_DateTypeDef sDate = { 0 };
	
	int d = cmd->int0;
	int y = cmd->int2;
	
	sDate.Month	= cmd->int1;
	sDate.Year 	= y - 2000;	
	sDate.WeekDay = ((d += sDate.Month < 3 ? y-- : y - 2, 23*sDate.Month/9 + d + 4 + y/4- y/100 + y/400) % 7) + 1; 
	sDate.Date 	= cmd->int0;
	
	return sDate;
}


/*************************************************************************/
/**  \fn      int RealTimeClock::Receive( Message* cmd )
***  \brief   Class function
**************************************************************************/

int RealTimeClock::Receive( Message* )
{
	return 0;
}	


/*************************************************************************/
/**  \fn      int RealTimeClock::Receive( Event )
***  \brief   Class function
**************************************************************************/

int RealTimeClock::Receive( Event )
{
	// if egvent = EVENT_SITE_RESET
	
//	CheckDayTime( );
	
	return 0;	
}


/*************************************************************************/
/**  \fn      int RealTimeClock::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int RealTimeClock::Receive( Command* cmd )
{
	switch( cmd->type )
	{
		case CMD_SET_TIME :		
		{		
			time_t before = now( );
			
			RTC_TimeTypeDef sTime = GetTime( cmd );
			
			HAL_RTC_SetTime( app.rtc, &sTime, RTC_FORMAT_BIN );
	 
			TouchScreen::last_tap = now( );
			
			CheckDayTime( );
			
			return CMD_OK;
		}
		
		case CMD_SET_DATE :
		{
			RTC_DateTypeDef sDate = Date( cmd );
			
			HAL_RTC_SetDate( app.rtc, &sDate, RTC_FORMAT_BIN );
			
			// Calc proper weekday
			sDate.WeekDay = ( ( CoreTime_OnGetDayOfWeek( CoreTime_OnGetCurrentTime( NULL ) ) + 6) % 7 ) + 1;
			
			// Set it
			HAL_RTC_SetDate( app.rtc, &sDate, RTC_FORMAT_BIN );
			
			CheckDayTime( );			
			
			return CMD_OK;
		}
		
		case CMD_GET_DAY_TIME:
		{
			if ( app.panel != NULL )
			{
				Daytime* d = app.panel->day + cmd->int0;
				
				cmd->int0 = d->start.hour;
				cmd->int1 = d->start.minute;
				cmd->int2 = d->end.hour;
				cmd->int3 = d->end.minute;
				
				return CMD_OK;
			}
			return CMD_ERR_NO_SITE;
		}
		
		case CMD_SET_DAY_TIME:
		{
			if ( app.panel != NULL )
			{
				Daytime d;
				
				d.start.hour   = cmd->short2;
				d.start.minute = cmd->short3;
				d.end.hour     = cmd->short4; 
				d.end.minute   = cmd->short5;
				
				if ( *(int*)&d != *(int*)(app.panel->day +cmd->int0 ) )
				{
					if ( qspi->WriteToPanel( d, day[ cmd->int0 ] ) == QSPI_RESULT_OK )
					{
						settings->UpdateRevision( );
						CheckDayTime( );	
					
						return CMD_OK;
					}
					return CMD_ERR_DEVICE_BUSY;
				}
				return CMD_OK;
			}
			return CMD_ERR_NO_SITE;
		}
	}
	return CMD_ERR_UNKNOWN_CMD;	
}


/*************************************************************************/
/**  \fn      void HAL_RTC_AlarmAEventCallback( RTC_HandleTypeDef* hrtc )
***  \brief   Global helper function
**************************************************************************/

extern "C" void HAL_RTC_AlarmAEventCallback( RTC_HandleTypeDef* hrtc )
{
	RealTimeClock::daytime = true;
	app.Send( EVENT_DAY_TIME );
	
	int tomorrow  = RealTimeClock::Tomorrow( );

	if ( app.panel != NULL )
	{
		Daytime* tom = app.panel->day + ( tomorrow - 1 );
	
		RealTimeClock::SetAlarm( ALARM_DAYTIME, tom->start.hour, tom->start.minute, tomorrow );
	}
}


/*************************************************************************/
/**  \fn      void HAL_RTCEx_AlarmBEventCallback( RTC_HandleTypeDef* hrtc )
***  \brief   Global helper function
**************************************************************************/

extern "C" void HAL_RTCEx_AlarmBEventCallback( RTC_HandleTypeDef* hrtc )
{
	RealTimeClock::daytime = false;
	app.Send( EVENT_NIGHT_TIME );
	
	int tomorrow  = RealTimeClock::Tomorrow( );
	
	Daytime* tom = app.panel->day + ( tomorrow - 1 );
	
	RealTimeClock::SetAlarm( ALARM_NIGHTTIME, tom->end.hour, tom->end.minute, tomorrow );
}
