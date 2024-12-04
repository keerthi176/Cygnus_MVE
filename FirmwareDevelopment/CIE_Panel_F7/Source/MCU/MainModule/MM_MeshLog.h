/***************************************************************************
* File name: MM_MeshLog.h
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
* MeshLog define list.
*
**************************************************************************/

#ifndef _MeshLog_H_
#define _MeshLog_H_


/* System Include Files
**************************************************************************/
#include <stdarg.h>
#include "ff.h"

/* User Include Files
**************************************************************************/
#include "MM_Module.h"



/* Defines
**************************************************************************/


class SDFATFS;
 
class MeshLog : public Module 
{
	public:
		
	MeshLog( );
	
	virtual int Init( );
	 	
	static void Tx( const char* fmt, ... );
	static void Rx( const char* fmt, ... );
	static void Msg( bool rx, const char* fmt, va_list args );
 		
	private:
		
	static FIL 			meshlog;
	static SDFATFS* 	sdfs;	
	static bool 		logging;
	
 
};


#endif

