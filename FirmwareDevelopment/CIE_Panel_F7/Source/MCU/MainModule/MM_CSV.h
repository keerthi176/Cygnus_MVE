/***************************************************************************
* File name: MM_CSV.h
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
* CSV Output module
*
**************************************************************************/

#ifndef _MM_CSV_H
#define _MM_CSV_H

/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_NCU.h"
 
 
class CSV : public Module
{
	public:
	
	CSV( );
	virtual int Init( );
	
	static int GetLine( char* buff, int line );
	static int EN54( DeviceConfig* );
	
	static NCU* ncu;
	static IOStatus* iostatus;
};



#endif
