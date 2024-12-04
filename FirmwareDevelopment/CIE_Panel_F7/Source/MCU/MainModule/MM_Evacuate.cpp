/***************************************************************************
* File name: MM_Evacuate.cpp
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
* Evacuate Module
*
**************************************************************************/

/* System Include Files
**************************************************************************/
 


/* User Include Files
**************************************************************************/
 
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_Application.h"
#include "MM_Events.h"
#include "MM_fault.h"
#include "MM_device.h"
#include "MM_error.h"
#include "CO_Site.h"
#include "CO_Device.h"
#include "MM_Evacuate.h"
#include "MM_Device.h"
#include "MM_CUtils.h"


/* Defines
**************************************************************************/
 
  

/*************************************************************************/
/**  \fn      EvacButton::EvacButton( )
***  \brief   Constructor for class
**************************************************************************/

EvacButton::EvacButton( DeviceConfig* config ) : Device( NULL, config )
{
	
}	


/*************************************************************************/
/**  \fn      Evacuate::Evacuate( )
***  \brief   Constructor for class
**************************************************************************/

Evacuate::Evacuate( ) : Module( "Evacuate", 0, EVENT_FIRE_RESET | EVENT_EVACUATE_BUTTON | EVENT_RESET ), io( NULL ), settings( NULL )
{
 
}



/*************************************************************************/
/**  \fn      int Evacuate::Init( )
***  \brief   Class function
**************************************************************************/

int Evacuate::Init( )
{
	io  = (IOStatus*) app.FindModule( "IOStatus" );
	settings = (Settings*)    app.FindModule( "Settings" );
 
	assert( settings != NULL );		
	assert( io!= NULL );
	
	return true;
}


/*************************************************************************/
/**  \fn      int Evacuate::Receive( Message* msg )
***  \brief   Class function
**************************************************************************/

int Evacuate::Receive( Message* msg )
{
	return MODULE_MSG_UNKNOWN;
}


/*************************************************************************/
/**  \fn      int Evacuate::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int Evacuate::Receive( Command* cmd )
{
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int Evacuate::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int Evacuate::Receive( Event e )
{
	switch ( e )
	{
		case EVENT_FIRE_RESET :
		case EVENT_RESET : 
		{
			evac_config = settings->FindType( DEVICE_TYPE_EVACUATE );
			 
			if ( evac_config != NULL )
			{
				// Create device
				EvacButton evacButton( evac_config );
					
				// Register it
				 io->RegisterDevice( &evacButton );
			}
			break;
		}
	
		case EVENT_EVACUATE_BUTTON :
		{
			if ( evac_config == NULL )
			{
				evac_config = settings->FindType( DEVICE_TYPE_EVACUATE );
			}
			
			if ( evac_config != NULL )
			{
				// Generate evac event by asserting 'evac device'
				Device* evacbutton = io->FindDevice( evac_config );
				
				if ( evacbutton != NULL )
				{
					evacbutton->InputAssert( 0, now( ), 99 );
					evacbutton->InputDeAssert( 0, now( ), 0 );
				}
			}
			break;			
		}
							
		default:;
	}
	return 0;
}

