/***************************************************************************
* File name: MM_Module.h
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
* Module base class.
*
**************************************************************************/
#ifndef _MODULE_H_
#define _MODULE_H_


/* System Include Files
**************************************************************************/
#include <stdint.h>

/* User Include Files
**************************************************************************/
#include "MM_Events.h"
#include "MM_Command2.h"

/* Defines
**************************************************************************/


#define MODULE_MSG_UNKNOWN 				-1


class Message;
class Application;

class Module
{
	public:
		
	friend class Application;
	
	Module( const char* name, int priority, uint64_t event_mask = 0 );
   	
	virtual void Poll( void );
	
	private:
		
	virtual int Receive( Command* );
	virtual int Receive( Event );
	virtual int Receive( Message* );
	
	virtual int Init( );
	
	public:
	
	void Send( int type, Module* to = (Module*)-1, int value = 0 );
	
	const char*		name;
	int     			priority;
	uint64_t			event_mask;
};

#endif



