/***************************************************************************
* File name: MM_Module.cpp
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
#include "stdlib.h"
#include <assert.h>


/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_Application.h"



/* Defines
**************************************************************************/


/*************************************************************************/
/**  \fn      Module::Module( const char* name, int priority, Application* app, uint64_t event_mask )
***  \brief   Constructor for class
**************************************************************************/

Module::Module( const char* name, int priority, uint64_t event_mask ) 
		 : name( name ), priority( priority ), event_mask( event_mask )
{
	app.RegisterModule( this );
}	
	

/*************************************************************************/
/**  \fn      void Module::Poll( )
***  \brief   Class function
**************************************************************************/

void Module::Poll( )
{
	assert( 0 );
}


/*************************************************************************/
/**  \fn      int Module::Init( )
***  \brief   Class function
**************************************************************************/

int Module::Init( )
{
	return true;
}


/*************************************************************************/
/**  \fn      void Module::Send( Message* m, int type, Module* to )
***  \brief   Class function
**************************************************************************/

void Module::Send( int type, Module* to, int value )
{
	Message m;
	
	assert( to != NULL );
	
	if ( to == (Module*) -1 ) to = this;
	
	m.to = to;
	m.from = this;
	m.type = type;
	m.value = value;
	
//	app.DebOut( "Queing msg %X\n", m.type );
	
	app.Send( &m );
}


/*************************************************************************/
/**  \fn      int Module::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int Module::Receive( Event event )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int Module::Receive( Command* m )
***  \brief   Class function
**************************************************************************/

int Module::Receive( Command* m )
{
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int Module::Receive( Message* m )
***  \brief   Class function
**************************************************************************/

int Module::Receive( Message* m )
{
	return 0;
}

