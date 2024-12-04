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
* Evacuate button
*
**************************************************************************/
#ifndef _MM_EVACUATE_H_
#define _MM_EVACUATE_H_

/* System Include Files
**************************************************************************/
 
 


/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_Events.h"
#include "MM_IOStatus.h"


/* Defines
**************************************************************************/

 

class EvacButton : public Device
{
	public:
	
	EvacButton( DeviceConfig* conf );
};

 
class Evacuate : public Module 
{
	public:
	
	Evacuate( );
	
	virtual int Init( );
	virtual int Receive( Command* cmd );
	virtual int Receive( Event event );
	virtual int Receive( Message* );

	private:
		
	static DeviceConfig default_evacconfig;
	IOStatus*  		io;
	Settings*  	   settings;
	QuadSPI*   		quadspi;
	DeviceConfig*  evac_config;
};
	

#endif

