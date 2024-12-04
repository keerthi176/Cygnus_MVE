/***************************************************************************
* File name: MM_Access.cpp
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
* Access control module
*
**************************************************************************/

/* System Include Files
**************************************************************************/
 

/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_Access.h"
#include "MM_fault.h"
#include "MM_Log.h"


int Access::level = 1;

/*************************************************************************/
/**  \fn      Access::Access( )
***  \brief   Constructor for class
**************************************************************************/

Access::Access( ) : Module( "Access", 0 )
{
	 
} 



/*************************************************************************/
/**  \fn      int Access::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int Access::Receive( Command* cmd )
{
	if ( cmd->type == CMD_LOGON_ACCESS )
	{
		if ( cmd->int2 )
		{
			Log::Msg( LOG_ACC, "User logged on at level %d.", cmd->int0 );
		}
		else
		{
			Log::Msg( LOG_ACC, "User logged off at level %d.", cmd->int0 );
		}							
		return CMD_OK;
	}
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int Access::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int Access::Receive( Event event )
{
	
	return 0;
}


/*************************************************************************/
/**  \fn      int Access::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int Access::Receive( Message* )
{
   return 0;
}

