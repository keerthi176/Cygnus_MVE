/***************************************************************************
* File name: MM_Events.h
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
* Event enumeration
*
**************************************************************************/
#ifndef _EVENTS_2_H_
#define _EVENTS_2_H_



/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "DM_TimerHandler.h"
#include "MM_Module.h"


class Events : public Module 
{
	public:
	
	Events( );
	
	virtual int Receive( Command* );
	virtual int Receive( Event );
	virtual int Receive( Message* );
	
	 
};


#endif

