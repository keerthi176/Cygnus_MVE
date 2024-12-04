/***************************************************************************
* File name: MM_event.h
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
* Other Events list.
*
**************************************************************************/
#ifndef _OTHER_EVENT_H_
#define _OTHER_EVENT_H_

/* System Include Files
**************************************************************************/
#include <time.h>


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Module.h"
#include "MM_GPIO.h"



/* Defines
**************************************************************************/



#define EVENTFLAG_IS_LATCHED		1
#define EVENTFLAG_ACKED				4
#define EVENTFLAG_LATCH				8


 

typedef struct
{
	char		 	type;
	char      	flags;
	char 		 	chan;
	char 			zone;
	short       unit;
	
	time_t	 	time;
	const char* msg;
} OtherEventItem;


class OtherEvents : public Module
{
	public:
		
	OtherEvents( );
 
	static void AddEvent( Event type, const char* msg, char zone = 255, short unit = -1, char chan = 255, bool latching = false );
	static void RemoveEvent( Event type, char zone = 255, short unit = -1, char chan = 255 );

//	static void AddOtherEvents( const char*, char zone, short unit, char chan, bool latching = false );
//	static void RemoveOtherEvents( const char*, char zone, short unit, char chan );
	
	static void CheckNoEvents( );
	
	static OtherEventItem* event_pos;
	
	virtual int Receive( Event );
	virtual int Receive( Message* msg );
	virtual int Receive( Command* cmd ); 
	
	private:
		
	static GPIOManager* gpio;	
};

 




#ifdef OLD_OTHER_EVENTS

/* SYSTEM OTHER_EVENTS */


#define  OTHER_EVENT_DATA_INTEGRITY  					0x1001
#define  OTHER_EVENT_BACKLIGHT_PWM_START				0x1002
#define  OTHER_EVENT_CRYSTAL_FAILURE					0x1004
#define  OTHER_EVENT_PSU_UART_FAILURE					0x1008
#define  OTHER_EVENT_GSM_UART_FAILURE					0x1010


/* GENERAL OTHER_EVENTS */

#define OTHER_EVENT_I2C_FAIL								0x2000
#define OTHER_EVENT_NCU_BUFFER_OVERFLOW				0x2001



/* POWER OTHER_EVENTS */

#define  OTHER_EVENT_POWER_SHORTAGE_RESET				0x3001
#define  OTHER_EVENT_MAINS_FAILURE						0x3005
#define  OTHER_EVENT_BATTERY_LOW							0x3006
#define  OTHER_EVENT_BATTERY_CRITICAL					0x3007
#define  OTHER_EVENT_CHARGER_FAILED						0x3008
#define  OTHER_EVENT_PSU_CONFIG_ERROR					0x3009
#define  OTHER_EVENT_BATTERY_RESISTANCE_TOO_HIGH	0x300A
#define  OTHER_EVENT_PSU_OVER_VOLTAGE					0x300B

#endif

#endif
