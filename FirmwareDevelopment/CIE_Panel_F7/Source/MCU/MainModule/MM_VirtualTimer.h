/***************************************************************************
* File name: MM_VirtualTimer.h
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
* Virtual Timer module.
*
**************************************************************************/
#ifndef _VIRTUAL_TIMER_H_
#define _VIRTUAL_TIMER_H_

/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "MM_Module.h"


/* Defines
**************************************************************************/
#define MAX_TIMERS 16

/*************************************************************************/
/**  \fn      typedef void VTimerCallback( void* )
***  \brief   Local helper function
**************************************************************************/

typedef void VTimerCallback( void* );

struct VTimer
{
	VTimerCallback* fn;
	int 				 period;
	int 				 count;
	void* 			 user;
	int 				 suspend;
};


class VirtualTimer : public Module
{
	public:
	
	VirtualTimer( );

   VTimer* CreateTimer( int period, VTimerCallback *callback, void* userptr );
	void DestroyTimer( VTimer* vt );
	
	virtual int Receive( Event );
	virtual int Receive( Command* );
   virtual int Receive( Message* );
 	
	
	private:
	
	VTimer timers[ MAX_TIMERS ];
	VTimer* timer;
};

#endif

	
	

