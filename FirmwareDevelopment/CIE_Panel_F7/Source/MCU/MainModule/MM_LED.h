/***************************************************************************
* File name: MM_LED.h
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
* LED control module
*
**************************************************************************/

#ifndef __LED__
#define __LED__




/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "MM_GPIO.h"
#include "DM_TimerHandler.h"
#include "CO_Site.h"



enum LEDStatus
{
	LED_NONE = 0,
	LED_OFF = 3,
	LED_ON = 1,
	LED_FLASH = 2,
};

enum FlashStatus
{
	FLASH_ON,
	FLASH_OFF,
	FLASH_DONT,
};
 
 
class LED : public Module, public TimerHandler
{
	public:
	
	LED( );
	
	virtual int Init( );
	
	virtual void TimerPulseFinished( TIM_HandleTypeDef* );
	virtual void TimerPeriodElapsed( TIM_HandleTypeDef* );
	
	virtual void Poll( );
	
	virtual int Receive( Message* );
	virtual int Receive( Event );
	virtual int Receive( Command* );
	
	static void Set( GPIO gpio, LEDStatus state );
	static void On( GPIO gpio );
	static void Off( GPIO gpio );
	static void Flash( GPIO gpio );
	

	static int  GetZone( int n );
	static void SetZone( int n, int state );
	static void ZoneOn( int n );
	static void ZoneOff( int n );
	
	void RestoreLEDS( );
	static void AllLEDSOff( );
	
	static int testMode;

	void LampTest( int stage );

	static char state[ GPIO_END ];
	static unsigned int zone[ SITE_MAX_ZONES / sizeof(short) ];
	
	FlashStatus flash;
};
 
#endif
