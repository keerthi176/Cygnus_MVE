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


/* Defines
**************************************************************************/



/* Globals
**************************************************************************/         
int 					Log::logging;
SDFATFS* 			Log::sdfs = NULL;
EmbeddedWizard* 	Log::ew = NULL;		
const char*			Log::type_name[ LOG_MAX_TYPE + 1 ] = { "0:/gen.dat", "0:/evt.dat", "0:/sys.dat", "0:/set.dat", "0:/acc.dat",
																			"0:/flt.dat", "0:/fir.dat", "0:/tst.dat", "0:/logy.txt" };

FIL Log::type[ LOG_MAX_TYPE + 1 ];																			
FIL* Log::mixed = Log::type + LOG_MAX_TYPE;
unsigned int Log::position[ LOG_MAX_TYPE + 1 ]; 
																			

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
	sdfs = (SDFATFS*) app.FindModule( "SDFATFS" );
	ew   = (EmbeddedWizard*) app.FindModule( "EmbeddedWizard" );
	
	if ( sdfs != NULL && ew != NULL )
	{	
		if ( sdfs->mounted )
		{
			logging = true;
			
			for( int n = 0; n <= LOG_MAX_TYPE; n++ )
			{
				// try opening existing...
				if ( f_open( type + n, type_name[ n], FA_OPEN_ALWAYS | FA_OPEN_APPEND | FA_WRITE | FA_READ ) != FR_OK )
				{
					// if not open new...
					if ( f_open( type + n, type_name[ n], FA_CREATE_ALWAYS | FA_WRITE  ) != FR_OK )
					{
						Fault::AddFault( FAULT_COULD_NOT_OPEN_LOG_FILE );
			
						logging = false;
						return false;
					}
				}
				unsigned int size = f_size( type + n ); 
				
				EEPROM::GetVar( filepos[ n], position[ n] );
				
				if ( position[ n] > size / LINE_LENGTH ) 
				{
					position[ n] = 0;
					EEPROM::SetVar( filepos[ n],  position[ n] );
				}
				if ( f_tell( type + n ) != position[ n] * LINE_LENGTH )
					f_lseek( type + n, position[ n] * LINE_LENGTH );				
			}
		
			mixed = type + LOG_MAX_TYPE;
					
			Msg( "System start-up." );
			
			return true;
		}
	}	
	return false;
}


/*************************************************************************/
/**  \fn      Log::Msg( char* fmt, ... )
***  \brief   Constructor for class
**************************************************************************/

void Log::Msg( const char* fmt, ... )
{
	va_list args;
			
	va_start( args, fmt );
	
	Msg( LOG_GEN, fmt, args );
	
	va_end( args );
}


/*************************************************************************/
/**  \fn      void Log::UpdateGUI( )
***  \brief   Class function
**************************************************************************/

void Log::UpdateGUI( )
{
	// Update display
	if ( ew != NULL )
	{
		Message msg;
		msg.type = EW_UPDATE_LOG_LIST_MSG;
		msg.to = ew;
		
		app.Send( &msg );
	}
}

/*************************************************************************/
/**  \fn      Log::Msg( LogType lt, char* fmt, ... )
***  \brief   Constructor for class
**************************************************************************/

void Log::Msg( LogType lt, const char* fmt, ... )
{
	va_list args;
			
	va_start( args, fmt );
	
	Msg( lt, fmt, args );
	
	va_end( args );	
}

				
/*************************************************************************/
/**  \fn      void write( FILE* f, char* )
***  \brief   Constructor for class
**************************************************************************/

int Log::Write( LogType lt, char* s )
{
	FIL* f = Log::type + lt;
	unsigned int len = f_tell( f);
				
	if ( len >= MAX_FILE_SIZE )
	{
		f_lseek( f, 0 );
		position[ lt] = 0;
	}
	
	unsigned int written;
		
	if ( f_write( f, (const void*)s, LINE_LENGTH, &written ) == FR_OK )
	{
		position[ lt]++;
	}
	
	f_sync( f );
	EEPROM::SetVar( filepos[ lt ], position[ lt ] );
	
	return written;
}


/*************************************************************************/
/**  \fn      Log::Msg( LogType lt, char* fmt, ... )
***  \brief   Constructor for class
**************************************************************************/

void Log::Msg( LogType lt, const char* fmt, va_list args )
{
	if ( logging )
	{
		if ( fmt != NULL )
		{
			char buffer[ LINE_LENGTH ];
			char output[ LINE_LENGTH + 1 ];
 
			vsnprintf( buffer, LINE_LENGTH - 20, fmt, args );
			
			time_t t = now( ); 
			
			struct tm *tp = localtime( &t );
			
			char tstr[ 17];
			snprintf( tstr, 17, "%02d:%02d %02d/%02d/%04d", tp->tm_hour, tp->tm_min, tp->tm_mday, tp->tm_mon, tp->tm_year + 1900 );
				
			char typechar[ 3] = { (char)( '0' + lt ), ' ', 0 };
								
			int len = strlen( buffer );
	
			char spaces[ LINE_LENGTH - 19 ]  ;
			
			memset( spaces, ' ', ( LINE_LENGTH - 19 ) - len );
			spaces[ ( LINE_LENGTH - 22 ) - len ] = 0;
						
			snprintf( output, LINE_LENGTH+1, "%s%s  %s%s\r\n", typechar, tstr, buffer, spaces );
			
			Write( LOG_MAX_TYPE, output );
			Write( lt, output );
		}	
	}
}
 
 
/*************************************************************************/
/**  \fn      int Log::NumLogLines( int filter )
***  \brief   Class function
**************************************************************************/

/*
int Log::NumLogLines( int filter )
{
	if ( logging && filter != 0 )
	{
		uint64_t lencount = 0;
		uint64_t orig_pos;
		int total = 0;
		
		for( int n = 0; n < LOG_MAX_TYPE; n++ )
		{
			if ( ( 1 << n ) & filter )
			{
				uint64_t length = f_size( type + n );
				total += length / (uint64_t) LINE_LENGTH;
			}
		}
		return total;
	}
	return 0;
}
*/


#ifdef LOG_FILTERED
/*************************************************************************/
/**  \fn      int Log::NumLogLines( int filter )
***  \brief   Class function
**************************************************************************/

int Log::NumLogLines( int filter )
{
	if ( logging && filter != 0 )
	{
		uint64_t lencount;
		uint64_t orig_pos;
		uint64_t length = lencount = sdfs->GetSize( );
		uint64_t cur_pos = orig_pos = sdfs->GetPos( );
		
		int n = 0;
		
		while ( lencount > 0 )
		{
			char ltype = 0xFF;

			cur_pos = ( cur_pos - LINE_LENGTH ) & ( MAX_FILE_SIZE - 1 );
			lencount -= LINE_LENGTH;
			
			if ( cur_pos < length )
			{
				sdfs->Seek( cur_pos );
				
				if ( sdfs->Read( &ltype, 1 ) )
				{ 				
					int type = ltype - '0';
					
					if ( type >= LOG_MAX_TYPE )
					{
						
						return 0;
					}
					
					// if matches criteria
					if ( ( 1 << type ) & filter )
					{
						n++;
					}
				}
				else
				{
					sdfs->Seek( orig_pos );
					return 0; // read fail
				}
			}
			else
			{
				sdfs->Seek( orig_pos );
				return 0; // past EOF
			}
		}
		sdfs->Seek( orig_pos );
		return n;
	}
		
	return 0; // fail
}
#endif
 
/*************************************************************************/
/**  \fn      int Log::NumLogLines( int filter )
***  \brief   Class function
**************************************************************************/

int Log::NumLogLines( int filter )
{
	if ( logging )
	{
		LogType lt;
		
		if ( filter == 255 || filter == 0 ) // all
		{
			lt = LOG_MAX_TYPE;
		}
		else
		{
			lt = (LogType) POSITION_VAL( filter );
		}
			
		if ( lt <= LOG_MAX_TYPE )
		{
			uint64_t length = f_size( type + lt );
			return length / (uint64_t) LINE_LENGTH;
		}
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int Log::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int Log::GetLine( int line, char* msg, int& mtype, int filter )
{
	if ( logging )
	{
		LogType lt;
		
		if ( filter == 255 || filter == 0 ) // all
		{
			lt = LOG_MAX_TYPE;
		}
		else
		{
			lt = (LogType) POSITION_VAL( filter );
		}
			
		if ( lt > LOG_MAX_TYPE ) return false;
		
		uint64_t lines = f_size( type + lt ) / LINE_LENGTH;
		 		
		uint64_t pos = ( line + position[ lt] ) % lines;
		
		f_lseek( type + lt, pos * LINE_LENGTH );
			
		unsigned int read;
		
		if ( f_read( type + lt, (void*)msg, LINE_LENGTH, &read ) == FR_OK )
		{
			mtype = msg[ 0] - '0';
			msg[ LINE_LENGTH - 2 ] = 0;		// add terminator
			
			f_lseek( type + lt, position[ lt] * LINE_LENGTH );

			return true;
		}
		f_lseek( type + lt, position[ lt] * LINE_LENGTH);
		
	}
	return false;
}


#ifdef LOG_FILTERED
/*************************************************************************/
/**  \fn      int Log::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int Log::GetFliteredLine( int line, int filter, char* msg, int& type )
{
	if ( logging && filter != 0 )
	{
		uint64_t orig_pos;
		uint64_t length = sdfs->GetSize( );
		uint64_t cur_pos = orig_pos = sdfs->GetPos( );
		
		int n = 0;
		
		while ( n <= line )
		{
			char ltype[ 2];

			cur_pos = ( cur_pos - LINE_LENGTH ) & ( MAX_FILE_SIZE - 1 );
			
			if ( cur_pos < length )
			{
				sdfs->Seek( cur_pos );
				
				if ( sdfs->Read( ltype, 2 ) )
				{
					type = ltype[ 0] - '0';
					
					// if matches criteria
					if ( ( 1 << type ) & filter )
					{
						if ( n == line )
						{
							// read line
							sdfs->Read( msg, LINE_LENGTH - 4 );
							
							// add terminator
							msg[ LINE_LENGTH - 4 ] = 0;	
							
							// restore position
							sdfs->Seek( orig_pos );
							return true;
						}
						else
						{
							// next line
							n++;
						}
					}
					else
					{
						// ignore
					}
				}
				else
				{
					sdfs->Seek( orig_pos );
					return false;  // coundnt read
				}
			}
			else
			{
				sdfs->Seek( orig_pos );
				return false; // past EOF
			}
		}
		sdfs->Seek( orig_pos );
	}
	return false;
}
#endif

/*************************************************************************/
/**  \fn      int Log::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int Log::Receive( Event e )
{
	switch( e )
	{
		case EVENT_RESET_BUTTON:
			Log::Msg( LOG_EVT, "System Reset." );
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
			cmd->int0 = NumLogLines( cmd->int0 );
			
			return CMD_OK;
		}
		
		case CMD_GET_LOG_LINE :
		{
			if ( GetLine( cmd->int0, (char*) cmd->int2, cmd->int3, cmd->int1 ) )
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


 
