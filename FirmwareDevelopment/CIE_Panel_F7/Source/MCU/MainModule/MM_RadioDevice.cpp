/***************************************************************************
* File name: MM_RadioDevice.cpp
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
* Radio device class
*
**************************************************************************/


/* System Include Files
**************************************************************************/
#include <assert.h>


/* User Include Files
**************************************************************************/
#include "MM_RadioDevice.h"
#include "MM_NCU.h"
#include "CO_Site.h"


/* Defines
**************************************************************************/
#define ACTIVATE_BEACONS 				( 2+4+8+16)
#define ACTIVATE_SOUNDER  				( 1 )
#define ACTIVATE_VISUAL_INDICATOR	( 32 )
#define ACTIVATE_REMOTE_INDICATOR   ( 64 )
#define ACTIVATE_GENERIC			   ( 128 )

#define BATT_HIGH 		3
#define BATT_MED	 		2
#define BATT_LOW			1
#define BATT_EMPTY		0
#define BATT_UNKNOWN   -1

#define RADIO_CHANNEL_MASK 63

/*************************************************************************/
/**  \fn      RadioDevice::RadioDevice( RadioUnit* ru, int channel, time_t time )
***  \brief   Constructor for class
**************************************************************************/

RadioDevice::RadioDevice( RadioUnit* ru, DeviceConfig* conf, Module* owner )
			: Device( ru, conf, owner )
{
	 *(int*)analogue = 0;
}


/*************************************************************************/
/**  \fn      RadioDevice::Broadcasted( )
***  \brief   Constructor for class
**************************************************************************/

int RadioDevice::Broadcasted( )			 
{
	return true;
}


/*************************************************************************/
/**  \fn      int RadioDevice::Broadcast( )
***  \brief   Constructor for class
**************************************************************************/

int RadioDevice::Broadcast( int silenceable_status, int unsilenceable_status, int skip_bits )			 
{
	return (( NCU*) owner)->QueueWriteMsg( true, NCU_BROADCAST_OUTPUT, owner, NULL, silenceable_status, unsilenceable_status, skip_bits );
}


/*************************************************************************/
/**  \fn      RadioDevice::SetBattery( int )
***  \brief   Constructor for class
**************************************************************************/

void RadioDevice::SetBattery( int millivolts )		 
{
	DeviceConfig* dc = config;
	RadioUnit* ru = (RadioUnit*) ptr;
	
	if ( millivolts == 1 ) millivolts = 0;
	
	int index = 0;	// Assume sitenet
	
	if ( strstr( dc->typestring, ".IOU" ) )
	{
		index = 1;
	}
	else if ( strstr( dc->typestring, "SN." ) )   // SmartNet!
	{
		index = 1;
	}
	
	if ( app.site != NULL )
	{
		if ( millivolts > app.site->batt_high[ index] ) ru->battery = BATT_HIGH;
		else if ( millivolts > app.site->batt_med[ index] ) ru->battery = BATT_MED;
		else if ( millivolts > app.site->batt_low[ index] ) ru->battery = BATT_LOW;
		else if ( millivolts > 0 ) ru->battery = BATT_EMPTY;
		else ru->battery = BATT_UNKNOWN;
	}
	else ru->battery = BATT_UNKNOWN;		
}



/*************************************************************************/
/**  \fn      void IndicateCallback( Module* m, int success )
***  \brief   Global helper function
**************************************************************************/

void RemovedLED( Module* m, int success )
{
	
}


/*************************************************************************/
/**  \fn      void IndicateCallback( Module* m, int success )
***  \brief   Global helper function
**************************************************************************/

void IndicateCallback( Module* m, int success )
{
// donnt bother retrying..
}


/*************************************************************************/
/**  \fn      void RadioDevice::Indicate( int asserted )
***  \brief   Class function
**************************************************************************/

void RadioDevice::Indicate( int asserted, int prof )
{
	app.DebOut( "LED: %d zone %d unit %d\n", asserted, config->zone, config->unit );
	
	NCU* ncu = (NCU*) owner;
	
	if ( prof > PROFILE_ROUTING_ACK ) prof = PROFILE_FIRE;
	
	ncu->QueueWriteMsgFast( true, NCU_ACTIVATE, ncu, NULL, config->zone, config->unit, CO_CHANNEL_STATUS_INDICATOR_LED_E, prof, !!asserted, 0 );			
}

 
/*************************************************************************/
/**  \fn      void AssertedOutput( Module* m, int success )
***  \brief   Global helper function
**************************************************************************/

void AssertedOutput( Module* m, int success )
{
   
}


/*************************************************************************/
/**  \fn      void RadioDevice::OutputChange( int channel, int action )
***  \brief   Class function
**************************************************************************/

void RadioDevice::OutputChange( int channel, int action, int value  )
{
	NCU* 			ncu = (NCU*) owner;
	RadioUnit* 	ru  = (RadioUnit*) ptr;
	
	app.DebOut( "Changing output\n");
		
	static int prof;
	prof 	= action;
	static int type;
	type = config->output[ channel ].type & RADIO_CHANNEL_MASK; 
	static int valu;
	valu = value;
		
	ncu->QueueWriteMsgFast( true, NCU_ACTIVATE, ncu, AssertedOutput, ru->zone, ru->unit, type, prof, valu, 0 );			
}


 

