/***************************************************************************
* File name: MM_log.h
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
* Log define list.
*
**************************************************************************/

#ifndef _LOG_H_
#define _LOG_H_


/* System Include Files
**************************************************************************/
#include <stdarg.h>
#include "ff.h"

/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "Logger.h"
  

typedef enum
{
	LOG_DIS = 0, 
	LOG_EVT,
	LOG_SYS,
	LOG_SET,
	LOG_ACC,
	LOG_FLT,
	LOG_FIRE,
	LOG_TST,

} LogType;
 

class EmbeddedWizard;
	
class Log : public Module 
{
	public:
		
	Log( );
	
	virtual int Init( );
	
	virtual int Receive( Event );
	virtual int Receive( Message* );
	virtual int Receive( Command* );
 	
	static void Msg( LogType lt, const char* fmt, ... );
	static Logger logger;
	
	private:
		
	static EmbeddedWizard* ew;
	static void UpdateGUI( );
		
};


#endif

