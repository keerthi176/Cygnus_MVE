/***************************************************************************
* File name: MM_Reset.h
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
* Reset button manager
*
**************************************************************************/
#ifndef _MM_RESET_H_
#define _MM_RESET_H_

/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_Events.h"


/* Defines
**************************************************************************/


class Buttons : public Module 
{
	public:
	
	Reset( );
	
	virtual int Receive( Command* cmd );
	virtual int Receive( Event event );
	virtual int Receive( Message* );
		
};
	

#endif
