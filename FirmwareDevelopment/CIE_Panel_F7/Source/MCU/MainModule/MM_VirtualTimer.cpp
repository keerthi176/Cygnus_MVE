/***************************************************************************
* File name: MM_VirtualTimer.cpp
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
* virtual timer module.
*
**************************************************************************/

/* System Include Files
**************************************************************************/
#ifdef STM32F767xx
#include "stm32f7xx_hal_crc.h"
#else
#include "stm32f4xx_hal_crc.h"
#endif
#include <assert.h>


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_VirtualTimer.h"
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_log.h"
#include "MM_fault.h"
#include "MM_error.h"



/*************************************************************************/
/**  \fn      VirtualTimer::VirtualTimer( )
***  \brief   Constructor for class
**************************************************************************/

VirtualTimer::VirtualTimer( ) : Module( "VirtualTimer", 0, EVENT_CENTISECOND )
{
	timer = timers;
}
  

/*************************************************************************/
/**  \fn      VTimer* VirtualTimer::CreateTimer( int period, VTimerCallback* callback, void* userptr )
***  \brief   Class function
**************************************************************************/

VTimer* VirtualTimer::CreateTimer( int period, VTimerCallback* callback, void* userptr )
{
	timer->fn 		= callback;
	timer->period 	= timer->count = period;
	timer->user 	= userptr;
	timer->suspend = 0;
	
	timer++;
	
	assert( timer < timers + MAX_TIMERS );

	return timer - 1;
}


/*************************************************************************/
/**  \fn      void VirtualTimer::DestroyTimer( VTimer* vt )
***  \brief   Class function
**************************************************************************/

void VirtualTimer::DestroyTimer( VTimer* p )
{
	assert( p != NULL );
	
	while ( ++p < timer )
	{
		p[-1] = *p;
	}
	timer--;
}


/*************************************************************************/
/**  \fn      int VirtualTimer::Receive( Command* )
***  \brief   Class function
**************************************************************************/

int VirtualTimer::Receive( Command* )
{
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int VirtualTimer::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int VirtualTimer::Receive( Message* )
{
   return 0;
}


/*************************************************************************/
/**  \fn      int VirtualTimer::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int VirtualTimer::Receive( Event e )
{
//	if ( e ==  EVENT_CENTISECOND ) [ is only event ]
	
	VTimer* p = timers;
	
	while ( p < timer )
	{
		if ( !p->suspend )
		{
			p->count--;
			
			if ( p->count <= 0 )
			{
				p->fn( p->user );
				p->count = p->period;
			}
		}
		p++;
	}
	return 0;
}



