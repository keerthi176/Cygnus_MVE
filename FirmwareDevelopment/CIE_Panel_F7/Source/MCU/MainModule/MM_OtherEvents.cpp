/***************************************************************************
* File name: MM_OtherEvent.cpp
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
* Panel settings class. Stores network, mesh, etc
*
**************************************************************************/


/* System Include Files
**************************************************************************/
#include "stm32f7xx_hal_pwr.h"
#include "stm32f7xx_hal_flash_ex.h"
#include <assert.h>


/* User Include Files
**************************************************************************/
#include "MM_OtherEvents.h"
#include "MM_CUtils.h"
#include "MM_Buzzer.h"


/* Globals
**************************************************************************/
extern int __rcc_csr;


/* Defines
**************************************************************************/

#define NUM_ITEMS( x ) ( sizeof(x) / sizeof(*x) )
#define MAX_OTHER_EVENTS 200

	
static OtherEventItem event_list[ MAX_OTHER_EVENTS ]; 

GPIOManager* OtherEvents::gpio;								 
OtherEventItem* OtherEvents::event_pos = event_list;								 



/*************************************************************************/
/**  \fn      int OtherEvent::OtherEvent( )
***  \brief   Constructor class function
**************************************************************************/
								 
OtherEvents::OtherEvents( ) : Module( "OtherEvents", 0, EVENT_RESET_BUTTON ) 
{	
	gpio = (GPIOManager*) app.FindModule( "GPIOManager" );
}	


/*************************************************************************/
/**  \fn      int OtherEvent::AddOtherEvent( const char* msg )
***  \brief   Class function
**************************************************************************/

void OtherEvents::AddEvent( Event type, const char* msg, char zone, short unit, char chan, bool latching )
{
	if ( event_pos - event_list >= MAX_OTHER_EVENTS )
	{
		return;
	}
		
	for( OtherEventItem* ev = event_list; ev < event_pos; ev++ )
	{
		if ( !strcmp( ev->msg, msg ) )
		{
			return;		// event already reported
		}
	}
	
	event_pos->type 	= (unsigned char) type;
	event_pos->time 	= Now( app.rtc );
	event_pos->flags 	= latching ? EVENTFLAG_LATCH : 0;
	event_pos->msg 	= msg;
	event_pos->zone   = zone;
	event_pos->unit   = unit;
	event_pos->chan   = chan;
	event_pos++;
	
	// Sound buzzer
	Buzzer::AddReason( BUZZFOR_OTHER_EVENTS );
		
	gpio->Set( GPIO_OtherEvents, IO_PIN_SET ); 
	
	// Update display
	ApplicationDeviceClass_TriggerEventListChangeEvent( app.DeviceObject );
}
		
#ifdef MAYBE_NEED_IT
	
/*************************************************************************/
/**  \fn      int OtherEvent::AddOtherEvent( OtherEventType t )
***  \brief   Class function
**************************************************************************/
/*
void OtherEvents::AddEvent( OtherEventType t, char zone, short unit, char chan, bool latching )
{
	if ( event_pos - event_list >= MAX_EVENTS )
	{
		return;
	}
	
	for( OtherEventItem* ev = event_list; ev < event_pos; ev++ )
	{
		if ( ev->type == t && ev->zone == zone && ev->unit == unit && ev->chan == chan )
		{
			return;		// event already reported
		}
	}
	
	OtherEventDef* f = event + t;
	
	if ( f->flags & EVENTFLAG_SYSTEM ) latching = true;
		
	event_pos->type 	= t;
	event_pos->time 	= Now( app.rtc );
	event_pos->flags 	= f->flags | ( latching ? EVENTFLAG_LATCH : 0 );
	event_pos->msg 	= f->msg; 
	event_pos->zone   = zone;
	event_pos->unit   = unit;
	event_pos->chan   = chan;
	
	event_pos++;
	
	// Sound buzzer
	Buzzer::AddReason( BUZZFOR_EVENTS );
	
	// Light LED
	if ( f->flags & EVENTFLAG_SYSTEM )
	{
		HAL_GPIO_WritePin( SYS_EVENT_LED_GPIO_Port, SYS_EVENT_LED_Pin, GPIO_PIN_SET );
	}
	 
	HAL_GPIO_WritePin( GEN_EVENT_LED_GPIO_Port, GEN_EVENT_LED_Pin, GPIO_PIN_SET );	
	
	// Update display
	ApplicationDeviceClass_TriggerOtherEventListChangeEvent( app.DeviceObject );
}
*/
#endif
	

/*************************************************************************/
/**  \fn      int OtherEvent::RemoveOtherEvent( OtherEventType t )
***  \brief   Class function
**************************************************************************/

void OtherEvents::RemoveEvent( Event t, char zone, short unit, char channel )
{
	int found = false;
	int allacked = true;
	 
	for( OtherEventItem* ev = event_list; ev < event_pos; ev++ )
	{
		if ( ev->type == t && ev->zone == zone && ev->unit == unit && ev->chan == channel ) 
		{
			if ( ev->flags & EVENTFLAG_LATCH )
			{
				ev->flags |= EVENTFLAG_IS_LATCHED;
				return;
			}
			found = true;
		}
		else
		{
			if ( ( ev->flags & EVENTFLAG_ACKED ) == 0 ) allacked = false;
		}
		
		if ( found )
		{
			*ev = ev[ 1];
		}
	}
	
	if ( allacked )
	{
		Buzzer::RemoveReason( BUZZFOR_OTHER_EVENTS );
	}
	
	if ( found )
	{
		event_pos--;
		ApplicationDeviceClass_TriggerEventListChangeEvent( app.DeviceObject );
	}
}


#ifdef MIGHT_NEED

/*************************************************************************/
/**  \fn      int OtherEvent::RemoveOtherEvent( char* msg )
***  \brief   Class function
**************************************************************************/
/*
void OtherEvents::RemoveEvent( Event t, const char* msg, char zone, short unit, char chan  )
{
	int found = false;
	int allacked = true;
		
	for( OtherEventItem* ev = event_list; ev < event_pos; ev++ )
	{
		if ( ev->msg == msg && ev->zone == zone && ev->unit == unit && ev->chan == chan ) 
		{
			if ( ev->flags & EVENTFLAG_LATCH )
			{
				ev->flags |= EVENTFLAG_IS_LATCHED;
				return;
			}
			found = true;
		}
		else
		{
			if ( ( ev->flags & EVENTFLAG_ACKED ) == 0 ) allacked = false;
		}
		
		if ( found )
		{
			*ev = ev[ 1];
		}
	}
	
	if ( allacked )
	{
		Buzzer::RemoveReason( BUZZFOR_EVENTS );
	}
	
	if ( found )
	{
		event_pos--;
		ApplicationDeviceClass_TriggerOtherEventListChangeEvent( app.DeviceObject );
	}
}
*/
#endif

/*************************************************************************/
/**  \fn      int OtherEvent::Receive( Message* msg )
***  \brief   Class function
**************************************************************************/
	
int OtherEvents::Receive( Message* msg )
{
	switch( msg->type )
	{
	
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int OtherEvent::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int OtherEvents::Receive( Command* cmd ) 
{
	assert( cmd != NULL );
	
	switch ( cmd->type )
	{
		case CMD_GET_NUM_EVENTS:
		{
			cmd->int0 = event_pos - event_list;
			
			return CMD_OK;
		}
		
		case CMD_GET_EVENT_ITEM:
		{
			OtherEventItem* ev = event_list + cmd->int0;
			
			if ( ev < event_pos )
			{
				//OtherEventDef* fd = event + fi->type;
				static char tstr[ 20 ];
				
				struct tm *tp = localtime( &ev->time );
				
				snprintf( tstr, 20, "%02d:%02d %02d/%02d/%04d", tp->tm_hour, tp->tm_min, tp->tm_mday, tp->tm_mon, tp->tm_year + 1900 );
				
				cmd->ushort0 = ev->zone;
				cmd->ushort1 = ev->unit;				
				cmd->int1 = (int) ev->msg;
				cmd->int2 = ev->type;
				cmd->int3 = (int) tstr;
		
				return CMD_OK;
			}
			else
			{
				cmd->int1 = 0;
				cmd->int2 = 0;
				cmd->int3 = 0;
				
				return CMD_ERR_OUT_OF_RANGE;
			}
		}
	}
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int OtherEvent::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int OtherEvents::Receive( Event event ) 
{	
	switch ( event )
	{
		case EVENT_BUZZER_MUTED :
		{
			for( OtherEventItem* ev = event_list; ev < event_pos; ev++ )
			{
				ev->flags |= EVENTFLAG_ACKED;
			}
			break;
		}
		case EVENT_RESET_BUTTON :
		{
			for( OtherEventItem* ev = event_list; ev < event_pos; ev++ )
			{
				if ( ev->flags & EVENTFLAG_IS_LATCHED )
				{
					for( OtherEventItem* e = ev; e < event_pos; e++ )
					{
						*e = e[1];
					}
					event_pos--;
				}	
			}
			CheckNoEvents( );
			
			ApplicationDeviceClass_TriggerEventListChangeEvent( app.DeviceObject );
			
			break;
		}
	}
	return 0;
}


/*************************************************************************/
/**  \fn      void OtherEvents::CheckNoEvents( )
***  \brief   Class function
**************************************************************************/

void OtherEvents::CheckNoEvents( ) 
{
	
	if ( event_pos - event_list == 0 )
	{
		gpio->Set( GPIO_OtherEvents, IO_PIN_SET );		
	}
}

