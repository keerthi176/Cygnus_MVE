/***************************************************************************
* File name: AppInterface.cpp
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
* C interface to application
*
**************************************************************************/


/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "Application.h"
#include "Command2.h"

/* Defines
**************************************************************************/



extern "C" 
{
	void CommandInt( int type, int val )
	{
		Command cmd;
		cmd.type = type;
		cmd.val  = val;
		
		app.Post( &cmd );
	}
}
	
