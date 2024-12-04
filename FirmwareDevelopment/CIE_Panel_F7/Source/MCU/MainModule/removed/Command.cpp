/***************************************************************************
* File name: Command.cpp
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


/* User Include Files
**************************************************************************/
#include "Application.h"
#include "Command.h"

/* Defines
**************************************************************************/


/* Globals
**************************************************************************/
extern Application* app;


extern "C" void Command1( int type, int param )
{
	Command cmd;
	
	cmd.type = type;
	cmd.int0 = param;
	
	app->Post( &cmd );
}

extern "C" void Command2( int type, int p0, int p1 )
{
	Command cmd;
	
	cmd.type = type;
	cmd.int0 = p0;
	cmd.int1 = p1;
	
	app->Post( &cmd );
}

extern "C" void Command3( int type, int p0, int p1, int p2 )
{
	Command cmd;
	
	cmd.type = type;
	cmd.int0 = p0;
	cmd.int1 = p1;
	cmd.int2 = p2;
	
	app->Post( &cmd );
}	

extern "C" void Command8S( int type, short p0, short p1, short p2, short p3, short p4, short p5, short p6, short p7 )
{
	Command cmd;
	
	cmd.type = type;
	cmd.short0 = p0;
	cmd.short1 = p1;
	cmd.short2 = p2;
	cmd.short3 = p3;
	cmd.short4 = p4;
	cmd.short5 = p5;
	cmd.short6 = p6;
	cmd.short7 = p7;
	
	app->Post( &cmd );
}	
