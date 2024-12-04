
/***************************************************************************
* File name: MM_Buzzer.cpp
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
* Buzzer manager
*
**************************************************************************/


/* System Include Files
**************************************************************************/
#include <assert.h>


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Buzzer.h"
#include "MM_Module.h"
#include "MM_Events.h" 
#include "MM_GPIO.h"




/* Defines
**************************************************************************/


 
int Buzzer::buzzing = false;
int Buzzer::reasons = 0;
int Buzzer::buzzer_disabled = 0;


/*************************************************************************/
/**  \fn      Buzzer::Buzzer( )
***  \brief   Constructor for class
**************************************************************************/

Buzzer::Buzzer( ) : Module( "Buzzer", 0, EVENT_MUTE_BUZZER_BUTTON | EVENT_CLEAR )
{	
   
}


/*************************************************************************/
/**  \fn      int PanelIO::Init( )
***  \brief   Class function
**************************************************************************/

int Buzzer::Init( )
{
	ew = (EmbeddedWizard*)	app.FindModule( "EmbeddedWizard" );
	
	assert( ew != NULL );
	
	return true;
}


/*************************************************************************/
/**  \fn      void Buzzer::Buzz( bool forever )
***  \brief   Constructor for class
**************************************************************************/

void Buzzer::Buzz( int reason )
{
	reasons |= reason;
	
	if ( !buzzing && !buzzer_disabled )
	{
		GPIOManager::Set( GPIO_Buzzer, GPIO_PIN_SET );
		buzzing = true;		
	}
}	
		

/*************************************************************************/
/**  \fn      Buzzer::Mute( )
***  \brief   Constructor for class
**************************************************************************/

void Buzzer::Mute( int reason )
{
	reasons &= ~reason;
	
	if ( !reasons && buzzing )
	{
		GPIOManager::Set( GPIO_Buzzer, GPIO_PIN_RESET );
		buzzing = false;
	}
}	


/*************************************************************************/
/**  \fn      int Buzzer::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int Buzzer::Receive( Command* cmd )
{
	if ( cmd->type == CMD_SET_BUZZER_STATE )
	{
		buzzer_disabled = cmd->int0;
		
		if ( !buzzer_disabled && buzzing )
		{
			GPIOManager::Set( GPIO_Buzzer, GPIO_PIN_SET );
		}
		if ( buzzer_disabled )
		{
			GPIOManager::Set( GPIO_Buzzer, GPIO_PIN_RESET );
		}
		
		Send( EW_UPDATE_DISABLEMENTS_MSG, ew );
		
		return CMD_OK;	
	}	
	else if ( cmd->type == CMD_BUZZER )
	{
		if ( cmd->int1 )
		{
			Buzz( 1 << cmd->int0 );
		}
		else
		{
			Mute( 1 << cmd->int0 );
		}
		return CMD_OK;
	}
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int Buzzer::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int Buzzer::Receive( Event event ) 
{
	switch ( event )
	{
		case EVENT_CLEAR:
		case EVENT_MUTE_BUZZER_BUTTON :
		{
			Mute( -1 );
		}
		default:;
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int Buzzer::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int Buzzer::Receive( Message* ) 
{
	return 0;
}
