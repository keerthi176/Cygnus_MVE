/***************************************************************************
* File name: MM_MeshLog.cpp
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
* MeshLog control module
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
#include "MM_MeshLog.h"
#include "MM_fault.h"
#include "MM_MeshLog.h"
#include "MM_CUtils.h"
#include "MM_SDFATFS.h"


/* Defines
**************************************************************************/
#define MAX_LINE_LENGTH 100


/* Globals
**************************************************************************/         
 
FIL MeshLog::meshlog;																		
SDFATFS*  MeshLog::sdfs = NULL;
bool MeshLog::logging = false;																

/*************************************************************************/
/**  \fn      MeshLog::MeshLog( )
***  \brief   Constructor for class
**************************************************************************/

MeshLog::MeshLog( ) : Module( "MeshLog", 0 ) 
{	
		
}


/*************************************************************************/
/**  \fn      MeshLog::Init( )
***  \brief   Initializer for class
**************************************************************************/

int MeshLog::Init( )
{
	assert( sdfs = (SDFATFS*) app.FindModule( "SDFATFS" ) );
	
	logging = false;
 
	if ( sdfs != NULL )
	{	
		if ( sdfs->mounted )
		{
			// try opening existing...
			if ( f_open( &meshlog, "0:/mesh.log", FA_OPEN_ALWAYS | FA_OPEN_APPEND | FA_WRITE | FA_READ ) != FR_OK )
			{
				Fault::AddFault( FAULT_COULD_NOT_OPEN_LOG_FILE );	
			}
			else 
			{
				logging = true;
			}		
		
			return true;
		}
	}	
	return false;
}


/*************************************************************************/
/**  \fn      MeshLog::Tx( const char* fmt, ... )
***  \brief    class function
**************************************************************************/

void MeshLog::Tx( const char* fmt, ... )
{
	va_list args;
			
	va_start( args, fmt );
	
	Msg( false, fmt, args );
	
	va_end( args );
}


/*************************************************************************/
/**  \fn      MeshLog::Rx( const char* fmt, ... )
***  \brief   class function
**************************************************************************/

void MeshLog::Rx( const char* fmt, ... )
{
	va_list args;
			
	va_start( args, fmt );
	
	Msg( true, fmt, args );
	
	va_end( args );
}


/*************************************************************************/
/**  \fn      MeshLog::Msg( MeshLogType lt, char* fmt, ... )
***  \brief   Constructor for class
**************************************************************************/

void MeshLog::Msg( bool rx, const char* fmt, va_list args )
{
	if ( fmt != NULL )
	{
		char buffer[ MAX_LINE_LENGTH ];
		char output[ MAX_LINE_LENGTH ];

		vsnprintf( buffer, MAX_LINE_LENGTH, fmt, args );
		
		time_t t = now( ); 
		
		snprintf( output, MAX_LINE_LENGTH, "%10d: %s %s", t, rx ? "RX" : "TX", buffer );
			
		int len = strlen( output );
		unsigned int written;
		
		if ( logging )
		{
			f_write( &meshlog, (const void*)output, len, &written );
			f_sync( &meshlog );			 	
		}
		HAL_UART_Transmit( &huart1, (uint8_t* )output, len, 100 );
	}	
}
 
 
