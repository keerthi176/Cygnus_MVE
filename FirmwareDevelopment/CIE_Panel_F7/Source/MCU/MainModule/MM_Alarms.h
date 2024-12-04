/***************************************************************************
* File name: MM_Alarms.h
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
#ifndef _MM_ALARMS_H_
#define _MM_ALARMS_H_

/* System Include Files
**************************************************************************/
#ifdef STM32F767xx
#include "stm32f7xx_hal_rtc.h"
#else
#include "stm32f4xx_hal_rtc.h"
#endif


#include <list>


/* User Include Files
**************************************************************************/
#include "MM_Module.h"


/* Defines
**************************************************************************/
#define ALARMS_EVERY_DAY 0


/*************************************************************************/
/**  \fn      typedef void AlarmCallback( Module* )
***  \brief   Local helper function
**************************************************************************/

typedef void AlarmCallback( Module* );

struct Alarm
{
	Alarm( char h, char m, char d, char keep, AlarmCallback* func, Module* );
	Alarm( );
	
	char hour;
	char min;
	char wday;
	char keep;
	
	AlarmCallback* func;
	Module* user;
};


class Alarms : public Module 
{
	public:
	
	Alarms( );
	
	void Add( Alarm& );
	int SecondsTill( const Alarm& );
	HAL_StatusTypeDef Set( Alarm& );
	
	virtual int Receive( Command* cmd );
	virtual int Receive( Event event );
	virtual int Receive( Message* );
		
	std::list<Alarm> alarms;
};
	

#endif

