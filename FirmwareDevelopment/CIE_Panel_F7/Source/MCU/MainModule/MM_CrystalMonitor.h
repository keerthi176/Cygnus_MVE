/***************************************************************************
* File name: MM_CrystalMonitor.h
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
* Crystal Monitoring Module
*
**************************************************************************/

#ifndef _CRYSTAL_MONITOR_
#define _CRYSTAL_MONITOR_

/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "MM_Module.h"


class CrystalMonitor : public Module
{
	public:
	
	CrystalMonitor( );
	
	virtual int Receive( Command* cmd );
	virtual int Receive( Event event );
	virtual int Receive( Message* );
};

	
#endif
