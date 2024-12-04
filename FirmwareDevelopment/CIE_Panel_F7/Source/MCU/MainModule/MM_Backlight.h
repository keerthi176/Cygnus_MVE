/***************************************************************************
* File name: MM_Backlight.h
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
* Backlight control module
*
**************************************************************************/

#ifndef _BACKLIGHT_MONITOR_
#define _BACKLIGHT_MONITOR_

#include "MM_EEPROM.h"


/* System Include Files
**************************************************************************/

class Backlight : public Module
{
	public:
	
	Backlight( );
	
	virtual int Receive( Command* cmd );
	virtual int Receive( Event event );
   virtual int Receive( Message* msg );
   
	virtual int LoadSettings( );
	
	int activeBrightness;
	int inactiveBrightness;
	int active;
	
	EEPROM* eeprom;
};

#endif
