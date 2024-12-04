
/***************************************************************************
* File name: MM_TestGSM.cpp
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
* TestGSM Module
*
**************************************************************************/

/* System Include Files
**************************************************************************/
 


/* User Include Files
**************************************************************************/
 
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_NCU.h"
#include "MM_Events.h"
#include "MM_fault.h"
#include "MM_device.h"
#include "MM_error.h"
#include "CO_Site.h"
#include "CO_Device.h"
#include "MM_TestGSM.h"
#include "MM_PanelNetwork.h"
#include "MM_GSM.h"
#include "MM_Utilities.h"
#include "MM_CUtils.h"
#include "JSON.h"
#include <time.h>



/* GDefines
**************************************************************************/


#define DEST_DOMAIN 	"de1.localtonet.com"
#define DEST_URL		"de1.localtonet.com"
 
#define MQTT_DEST_DOMAIN 	"smartnet.azure-devices.net"
 


/* Globals
**************************************************************************/
  
/*************************************************************************/
/**  \fn      TestGSM::TestGSM( )
***  \brief   Constructor for class
**************************************************************************/

TestGSM::TestGSM( ) : Module( "TestGSM", 100, EVENT_RESET )//, mqtt_socket( MQTT_DEST_DOMAIN, "MATTDEVICE1" )   
{
	 
}


/*************************************************************************/
/**  \fn      int TestGSM::Init( )
***  \brief   Class function
**************************************************************************/

int TestGSM::Init( )
{
 //  mqtt_socket.Create( );
	
	return true;
}

  
/*************************************************************************/
/**  \fn      int TestGSM::Receive( Message* m )
***  \brief   Class function
**************************************************************************/

int TestGSM::Receive( Message* )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int TestGSM::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int TestGSM::Receive( Command* cmd )
{
	 
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int TestGSM::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int TestGSM::Receive( Event e )
{
	return 0;
}


extern "C" void mqtt_client_init( void );

/*************************************************************************/
/**  \fn      virtual void TestGSM::Poll( )
***  \brief   Class function
**************************************************************************/

void TestGSM::Poll( )
{
	static int state = 0;
	/*
	if ( GSM::state == GSM_IDLE && !state )
	{
		mqtt_client_init( );
		state = 1;
	}*/
	
}

				
					
			
		