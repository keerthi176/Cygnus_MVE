/***************************************************************************
* File name: Module.cpp
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
/* System Include Files
**************************************************************************/
#ifdef _USE_RTOS_	
#include "cmsis_os.h"
#endif
#include "stdlib.h"

/* User Include Files
**************************************************************************/
#include "Module.h"
#include "Message.h"
#include "Application.h"

/* Defines
**************************************************************************/

// name         name of the thread function.
/// \param         priority     initial priority of the thread function.
/// \param         instances    number of possible thread instances.
/// \param         stacksz  


Module::Module( const char* name, int priority, Application* app ) : app( app ), name( name ), priority( priority )
{
	app->RegisterModule( this );
}	
	

void Module::Poll( )
{
}


void Module::Post( Message* m, int type, Module* to )
{
	m->to = to;
	m->from = this;
	m->type = type;
	
	app->Post( m );
}


int Module::Receive( Message* m )
{
	return MODULE_MSG_UNKNOWN;
}


int Module::Receive( Command* m )
{
	return MODULE_CMD_UNKNOWN;
}

