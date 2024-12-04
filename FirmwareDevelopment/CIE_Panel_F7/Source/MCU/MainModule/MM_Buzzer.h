/***************************************************************************
* File name: MM_Buzzer.h
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
* Buzzer manager
*
**************************************************************************/
#ifndef _MM_Buzzer_H_
#define _MM_Buzzer_H_

/* System Include Files
**************************************************************************/
 
 


/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_Events.h"
#include "MM_EmbeddedWizard.h"


/* Defines
**************************************************************************/
 

enum
{
	BUZZ_FOR_FAULT 	= 1,
	BUZZ_FOR_FIRE  	= 2,
	BUZZ_FOR_LOG_OFF 	= 4,
	BUZZ_FOR_TESTING  = 8,
	BUZZ_FOR_GENERAL  = 16,
	BUZZ_FOR_FIRST_AID  = 32,
	BUZZ_FOR_SECURITY  = 64

};


class Buzzer : public Module 
{
	public:
	
	Buzzer( );
	
	virtual int Init( );
	
	static void Buzz( int reason );
	static void Mute( int reason );
	
 	
	virtual int Receive( Command* cmd );
	virtual int Receive( Event event );
	virtual int Receive( Message* );
	
	EmbeddedWizard* ew;
 
	static int buzzing;
	static int reasons;
	static int buzzer_disabled;
	
};
	

#endif

