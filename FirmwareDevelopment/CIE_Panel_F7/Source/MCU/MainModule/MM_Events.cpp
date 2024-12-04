/***************************************************************************
* File name: MM_Events.cpp
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
* Generates misc. events
*
**************************************************************************/


/* System Include Files
**************************************************************************/
#include "stm32f7xx_hal_pwr.h"
#include <assert.h>

/* User Include Files
**************************************************************************/
#include "MM_Events.h"
#include "MM_Events2.h"
#include "MM_Application.h"


/* Defines
**************************************************************************/





/*************************************************************************/
/**  \fn      Events::Events( )
***  \brief   Constructor for class
**************************************************************************/

Events::Events( ) : Module( "Events", 0 ) 
{ 
 
}


/*************************************************************************/
/**  \fn      int Events::Receive( Command* cmd )
***  \brief   virtual class member
**************************************************************************/

int Events::Receive( Command* cmd )
{
	switch( cmd->type )
	{
		case CMD_CATCH_ERROR: 
		{
			assert( 0 );
			return CMD_OK;
		}
		default:;
	}
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int Events::Receive( Message* )
***  \brief   virtual class member
**************************************************************************/

int Events::Receive( Message* )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int Events::Receive( Event )
***  \brief   virtual class member
**************************************************************************/

int Events::Receive( Event )
{
	return 0;
}


extern "C"
{

/*************************************************************************/
/**  \fn      void HAL_PWR_PVDCallback( void )
***  \brief   Global helper function
**************************************************************************/
	
void HAL_PWR_PVDCallback( void )
{
	app.Send( EVENT_PVD );		
}
 
}
