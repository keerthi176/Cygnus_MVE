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

#ifndef _REAL_TIME_CLOCK_H_
#define _REAL_TIME_CLOCK_H_


/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_Alarms.h"
#include "MM_Settings.h"
#include "MM_QuadSPI.h"


typedef enum
{
	ALARM_DAYTIME,
	ALARM_NIGHTTIME,
} AlarmPeriod;



class RealTimeClock : public Module
{
	public:
		
	RealTimeClock( );
	virtual int Init( );

	virtual int Receive( Command* );
	virtual int Receive( Message* );
	virtual int Receive( Event );
	
	void CheckDayTime( );
		
	static int daytime;
	static void SetAlarm( AlarmPeriod period, int hour, int min, int day );
	static int Today( );
	static int Tomorrow( );
	
	QuadSPI* 			qspi;	
	Settings*			settings;
};


#endif
