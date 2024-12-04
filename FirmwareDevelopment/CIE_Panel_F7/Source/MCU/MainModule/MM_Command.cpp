/***************************************************************************
* File name: MM_Command.cpp
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
* Command helper functions
*
**************************************************************************/


/* System Include Files
**************************************************************************/
#include <assert.h>


/* User Include Files
**************************************************************************/
#include "MM_Application.h"


/* Defines
**************************************************************************/

/*************************************************************************/
/**  \fn      Command::Command( CommandType type )
***  \brief   Constructor
**************************************************************************/

Command::Command( CommandType type ) : type( type )
{
}


/*************************************************************************/
/**  \fn      Command::Command( CommandType type )
***  \brief   Constructor
**************************************************************************/

Command::Command( )
{
}


/*************************************************************************/
/**  \fn      extern "C" int Command0( CommandType type, Command* cmd )
***  \brief   C Helper func
**************************************************************************/

extern "C" int Command0( CommandType type, Command* cmd )
{
	assert( cmd != NULL );
	
	cmd->type = type;
	
	return app.Send( cmd );
}


/*************************************************************************/
/**  \fn      extern "C" int Command1( CommandType type, int param )
***  \brief   C Helper func
**************************************************************************/

extern "C" int Command1( CommandType type, int param )
{
	Command cmd;
	
	cmd.type = type;
	cmd.int0 = param;
	
	return app.Send( &cmd );
}


/*************************************************************************/
/**  \fn      extern "C" int Command2( CommandType type, int p0, int p1 )
***  \brief   C Helper func
**************************************************************************/

extern "C" int Command2( CommandType type, int p0, int p1 )
{
	Command cmd;
	
	cmd.type = type;
	cmd.int0 = p0;
	cmd.int1 = p1;
	
	return app.Send( &cmd );
}


/*************************************************************************/
/**  \fn      extern "C" int Command3( CommandType type, int p0, int p1, int p2 )
***  \brief   C Helper func
**************************************************************************/

extern "C" int Command3( CommandType type, int p0, int p1, int p2 )
{
	Command cmd;
	
	cmd.type = type;
	cmd.int0 = p0;
	cmd.int1 = p1;
	cmd.int2 = p2;
	
	return app.Send( &cmd );
}	


/*************************************************************************/
/**  \fn      extern "C" int Command4( CommandType type, int p0, int p1, int p2, p3 )
***  \brief   C Helper func
**************************************************************************/

extern "C" int Command4( CommandType type, int p0, int p1, int p2, int p3 )
{
	Command cmd;
	
	cmd.type = type;
	cmd.int0 = p0;
	cmd.int1 = p1;
	cmd.int2 = p2;
	cmd.int3 = p3;
	
	return app.Send( &cmd );
}	


/*************************************************************************/
/**  \fn      extern "C" int Command8S( CommandType type, short p0, short p1, short p2, short p3, short p4, short p5, short p6, short p7 )
***  \brief   C Helper func
**************************************************************************/

extern "C" int Command8S( CommandType type, short p0, short p1, short p2, short p3, short p4, short p5, short p6, short p7 )
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
	
	return app.Send( &cmd );
}	


int Command8U( CommandType type, uint16_t p0, uint16_t p1, uint16_t p2, uint16_t p3, uint16_t p4, uint16_t p5, uint16_t p6, uint16_t p7 )
{
	Command cmd;
	
	cmd.type = type;
	cmd.ushort0 = p0;
	cmd.ushort1 = p1;
	cmd.ushort2 = p2;
	cmd.ushort3 = p3;
	cmd.ushort4 = p4;
	cmd.ushort5 = p5;
	cmd.ushort6 = p6;
	cmd.ushort7 = p7;
	
	return app.Send( &cmd );
}	

