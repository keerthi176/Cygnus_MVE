/***************************************************************************
* File name: Utilties.h
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
* Various utilties
*
**************************************************************************/

#ifndef _UTILITIES_
#define _UTILITIES_

#include "MM_Module.h"
#include "MM_GPIO.h"
#include "MM_EEPROM.h"
#include "MM_EmbeddedWizard.h"

/* System Include Files
**************************************************************************/

LogType GetLogType( int actionbit, const char** typetext );


class Utilities : public Module
{
	public:
	
	Utilities( );
	
	virtual void Poll( );
	
	virtual int Receive( Event );
	virtual int Receive( Message* msg );
	virtual int Receive( Command* cmd ); 
	virtual int Init( );
	
	int adc_voltage;
	int heartbeat;
	
	GPIOManager* 	 gpioman;
	EmbeddedWizard* ew;
	EEPROM* 			 eeprom;
	Message 			 msg;
	
	static int AccessLevel ;
	static char UID[ 25];
};

#endif
