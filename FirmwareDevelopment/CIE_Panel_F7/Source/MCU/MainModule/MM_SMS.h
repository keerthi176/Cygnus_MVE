/***************************************************************************
* File name: SMS.cpp
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
* Short Messaging Service Module
*
**************************************************************************/

#ifndef _GSM_
#define _GSM_

/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "MM_Module.h"


class GSM : public Module
{
	public:
	
	SMS( Application* app );
	
	virtual int Receive( Event event );
	
	
	char txbuff[ 32 ];
	char rxbuff[ 32 ];
};

	
#endif
