/***************************************************************************
* File name: MM_Log.cpp
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
* Log control module
*
**************************************************************************/


/* System Include Files
**************************************************************************/
#ifdef STM32F767xx
#include "stm32f7xx_hal.h"
#else
#include "stm32f4xx_hal.h"
#endif
#include "sd_diskio.h"

#include <assert.h>
#include <stdlib.h>
#include "stm32f7xx_hal_usart.h"


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_Log.h"
#include "MM_fault.h"
#include "MM_log.h"
#include "MM_CUtils.h"
#include "MM_SDFATFS.h"
#include "MM_EEPROM.h"

#include "version.h"

/* Defines
**************************************************************************/
#define FERR(s, r) app.DebOut( s": %s\n", f_err[ r ] );


/* Globals
**************************************************************************/         
Logger Log::logger;


/*************************************************************************/
/**  \fn      Log::Log( )
***  \brief   Constructor for class
**************************************************************************/

Log::Log( ) : Module( "Log", 0, EVENT_RESET_BUTTON )
{	
		
}


/*************************************************************************/
/**  \fn      Log::Init( )
***  \brief   Initializer for class
**************************************************************************/

int Log::Init( )
{			
	SDFATFS* sdfatfs;
	
	assert( sdfatfs = (SDFATFS*) (SDFATFS*) app.FindModule( "SDFATFS" ) ); 
	
	if ( logger.Init( sdfatfs, "0:/log.txt", FA_OPEN_ALWAYS | FA_OPEN_APPEND | FA_WRITE | FA_READ ) )
	{  
		Msg( LOG_SYS, "System start-up. (FW %0.2f)", FIRMWARE_VERSION );
		return true;
	}
	return false;
}
 

/*************************************************************************/
/**  \fn      Log::Msg( LogType lt, char* fmt, ... )
***  \brief   Constructor for class
**************************************************************************/

void Log::Msg( LogType lt, const char* fmt, ... )
{
	va_list args;

	if((pca9535e_I2CIOExp_ReadPin(0x40, 11)) ^ (1 << 0))
	{
		va_start( args, fmt );
	
		logger.Msg( lt, fmt, args );
	
		va_end( args );
	}
}


/*************************************************************************/
/**  \fn      int Log::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int Log::Receive( Event e )
{
	switch( e )
	{
		case EVENT_RESET_BUTTON:
			Log::Msg( LOG_SYS, "System Reset." );
			break;
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int Log::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int Log::Receive( Message* )
{
	return 0;
}
	

/*************************************************************************/
/**  \fn      int Log::Receive( Command* )
***  \brief   Class function
**************************************************************************/

int Log::Receive( Command* cmd )
{
	switch ( cmd->type )
	{
		case CMD_GET_LOG_LENGTH :
		{
			cmd->int0 = logger.NumLogLines( cmd->int0 );
			
			return CMD_OK;
		}
		
		case CMD_GET_LOG_LINE :
		{
			logger.line = cmd->int0;
			
			if ( logger.GetLine( (char*) cmd->int2, cmd->int3, cmd->int1 ) )
			{
				return CMD_OK;
			}
			else
			{
				return CMD_ERR_OUT_OF_RANGE;
			}
		}
		
	} 		
	return CMD_ERR_UNKNOWN_CMD;
}


