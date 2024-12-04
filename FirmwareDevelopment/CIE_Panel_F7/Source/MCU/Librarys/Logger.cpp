/***************************************************************************
* File name: MM_Logger.cpp
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
* Logger control library
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
#include <string.h>
#include "stm32f7xx_hal_usart.h"


/* User Include Files
**************************************************************************/
 
#include "MM_fault.h"
#include "MM_CUtils.h"
#include "MM_SDFATFS.h"
#include "MM_EEPROM.h"
#include "MM_Utilities.h"


/* Defines
**************************************************************************/
#define FERR(s, r) app.DebOut( s": %s\n", f_err[ r ] );

#define LOG_MAX_ITERATIONS 24

/* Globals
**************************************************************************/         

int Decode24( int val );
int Encode24( int val );

const char* f_err[] = {
	"FR_OK",				
	"FR_DISK_ERR",			
	"FR_INT_ERR",			
	"FR_NOT_READY",			
	"FR_NO_FILE",			
	"FR_NO_PATH",			
	"FR_INVALID_NAME",		
	"FR_DENIED",				
	"FR_EXIST",				
	"FR_INVALID_OBJECT",	
	"FR_WRITE_PROTECTED",	
	"FR_INVALID_DRIVE",	
	"FR_NOT_ENABLED",		
	"FR_NO_FILESYSTEM",	
	"FR_MKFS_ABORTED",		
	"FR_TIMEOUT",			
	"FR_LOCKED",				
	"FR_NOT_ENOUGH_CORE",	
	"FR_TOO_MANY_OPEN_FILES",
	"FR_INVALID_PARAMETER" };


 

/*************************************************************************/
/**  \fn      Logger::Logger( )
***  \brief   Constructor for class
**************************************************************************/

bool Logger::Init( SDFATFS* _sdfs, const char* _filename, BYTE mode )  
{	
	FRESULT r;

	sdfs 		=  _sdfs;
	filename = _filename;
	logging  = false;
	
	if ( sdfs != NULL )
	{	
		if ( sdfs->mounted )
		{
			// try opening existing...
			if ( f_open( &file, filename, mode ) != FR_OK )
			{
				// if not open new...
				if ( f_open( &file, filename, FA_CREATE_ALWAYS | mode ) != FR_OK )
				{					
					Fault::AddFault( FAULT_COULD_NOT_OPEN_LOG_FILE );
		
					return false;
				}				
			}
	
			int size = f_size( &file );
			
			size = size & ~(LINE_LENGTH - 1);
			
			if ( size > 0 )
			{
				unsigned int read;
				char buff[ LINE_LENGTH ];
				
				if ( ( r = f_lseek( &file, size - LINE_LENGTH ) ) == FR_OK )
				{
					if ( ( r = f_read( &file, buff, LINE_LENGTH, &read ) ) == FR_OK )
					{
						Decode( count, (int*) buff );
						
						if ( Total( -1, count ) != size / LINE_LENGTH )
						{
							Restart( );
							return true;
						}
					}
					else
					{
						Fault::AddFault( FAULT_COULD_NOT_OPEN_LOG_FILE );
						FERR( "f_read", r );
						f_close( &file );
			
						return false;
					}
				}
				else
				{
					Fault::AddFault( FAULT_LOG_CORRUPTION );
					FERR( "seek", r );
					f_close( &file );
				}	
			}
			else
			{
				for( int n = 0 ; n < LOG_MAX_TYPE; n++ )
				{
					count[ n] = 0;
				}
			}
			
			f_close( &file );			
			logging = true;	
							
			return true;
		}
	}	
	return false;
}


/*************************************************************************/
/**  \fn      Logger::Restart(  )
***  \brief   Constructor for class
**************************************************************************/
 
void Logger::Restart( )
{
	f_lseek( &file, 0 );
	f_truncate( &file );
	f_close( &file );
	
	for( int n = 0 ; n < LOG_MAX_TYPE; n++ )
	{
		count[ n] = 0;
	}
	
	va_list args;
	
	logging = true;
		
	Msg( LOG_SYS, "Log restarted due to file corruption.", args );
}
 
				
/*************************************************************************/
/**  \fn      int write( char* )
***  \brief   class function
**************************************************************************/

int Logger::Write( char* s )
{
	int r = DoWrite( s );

	if ( r == 0 )
	{
		HAL_Delay( 10 );
		// Try again..
		return DoWrite( s );
	}
	
	return r;
}


/*************************************************************************/
/**  \fn      int DoWrite( char* )
***  \brief   class function
**************************************************************************/
	
int Logger::DoWrite( char* s )
{	
	FRESULT r;
  
	if ( !open )
	{
		open = true;	
		if ( ( r = f_open( &file, filename, FA_OPEN_ALWAYS | FA_OPEN_APPEND | FA_WRITE | FA_READ ) ) == FR_OK )
		{		
			unsigned int written;
			
			FSIZE_t pos = f_tell( &file );
			
			if ( pos & ( LINE_LENGTH - 1 ) )
			{
				pos &= LINE_LENGTH - 1;
				
				f_lseek( &file, pos );
			}
						
			if ( ( r = f_write( &file, (const void*)s, LINE_LENGTH, &written ) ) != FR_OK )
			{
				FERR( "write", r );
				HAL_Delay( 50 ); // let SD settle
			}
		
			if ( ( r = f_close( &file ) ) == FR_OK )
			{
				open = false;
				return written;
			}
			else
			{
				FERR( "close", r );
				HAL_Delay( 50 ); // let SD settle
			}
		}	
		else
		{
			FERR( "open", r );

			// if locked/open
			
		}	
		open = false;
	}			
	return 0;
}


#define MSG_LEN (LINE_LENGTH - 52)


/*************************************************************************/
/**  \fn      Logger::Msg( LoggerType lt, char* fmt, ... )
***  \brief   Constructor for class
**************************************************************************/

void Logger::Msg( int lt, const char* fmt, va_list args )
{
	if ( logging )
	{
		if ( fmt != NULL )
		{
			// LINE
			// [8 * 32bit indexe = 32 ][' ' = 1][ type = 1 ][' ' = 1][ hh:mm dd/mm/yy = 14 ][ ' '=1][ Msg 128 - 1 - 1 -1 - 14 - 2 - 32 = 76 ][CR LF = 2 ]
			
			char msg[ MSG_LEN + 1];
			char output[ LINE_LENGTH + 1 ];
			char indexes[ 33];
			
			/* indexes */
			
			count[ lt ]++; // increase type count
			Encode( (int*) indexes, count );
			indexes[ 32 ] = 0; // terminate
			
			/* message */
			
			vsnprintf( msg, MSG_LEN + 1, fmt, args );
			msg[ MSG_LEN ] = 0;
			
			/* remove any linefeeds */
			
			char* p;
			while ( ( p = strchr( msg, '\n' ) ) != NULL )
			{
				*p = ' ';
			}

			
			/* date */
			
			time_t t = now( ); 
			
			struct tm *tp = localtime( &t );
			
			char tstr[ 15];
			snprintf( tstr, 15, "%02d:%02d %02d/%02d/%02d", tp->tm_hour, tp->tm_min, tp->tm_mday, tp->tm_mon + 1, tp->tm_year + 1900 - 2000 );
				
			/* type */	
			char typechar = '0' + lt;
			
			/* output */
			memset( output, ' ', LINE_LENGTH - 2 );
						
			snprintf( output, LINE_LENGTH - 1, "%s %c %s %s", indexes, typechar, tstr, msg );
			output[ strlen( output ) ] = ' ';
			
			/* terminator */
			
			output[ LINE_LENGTH - 2 ] = 13;
			output[ LINE_LENGTH - 1 ] = 10;
			
			if ( Write( output ) != LINE_LENGTH  )
			{
				count[ lt ]--; // undi count..
				app.DebOut( "SD Write fail! " );
			}
			app.DebOut( output + 32 );
		}	
	}
}


void CreateLogMessage(char* logMessageBuffer, char* logMessage, uint8_t msgSize)
{
	char tstr[15];

	/* Date and Time*/
	time_t t = now( );
	struct tm *tp = localtime( &t );

	/* Formatting date and time into readable format */
	snprintf(tstr, 15, "%02d:%02d %02d/%02d/%02d", tp->tm_hour, tp->tm_min, tp->tm_mday, tp->tm_mon + 1, tp->tm_year + 1900 - 2000);

	/* Copying date and time into Log Message Buffer */
	memcpy(&logMessageBuffer[0], &tstr[0], sizeof(tstr));

	/* Copying LogMessage into LogMessageBuffer */
	memcpy(&logMessageBuffer[sizeof(tstr)], &logMessage[0], msgSize);

	logMessageBuffer[ strlen( logMessageBuffer ) ] = ' ';
}

/*************************************************************************/
/**  \fn      int Logger::Total( int filter, int* array )
***  \brief   Class function
**************************************************************************/

int Logger::Total( int filter, int* array )
{
	int total = 0;
		
	for( int n = 0; n < LOG_MAX_TYPE; n++ )
	{
		if ( ( 1 << n ) & filter )
		{
			total += array[ n];
		}
	}
	return total;
}


/*************************************************************************/
/**  \fn      int Logger::NumLoggerLines( int filter )
***  \brief   Class function
**************************************************************************/

int Logger::NumLogLines( int filter )
{
	if ( logging )
	{
		return Total( filter, count );
	}
	return 0;
}


/*************************************************************************/
/**  \fn      void Logger::BinarySearch( int min, int max, char* buff, int line, int filter )
***  \brief   Class function
**************************************************************************/

int Logger::BinarySearch( )
{
	static int mid;
	static unsigned int nread;
	static int index[ LOG_MAX_TYPE ];
	
	mid = ( min + max ) / 2;
	
	if ( iterations++ < LOG_MAX_ITERATIONS )
	{	
		f_lseek( &file, mid * LINE_LENGTH );
			
		if ( f_read( &file, buff, LOG_MAX_TYPE * 4, &nread ) == FR_OK )
		{		
			Decode( index, (int*) buff );
			
			int i = Total( filter, index );
			
			if ( i == line + 1 ) return true;
			
			if ( i > line + 1 )
			{
				max = mid - 1;
				return BinarySearch( );
			}
			else 
			{
				min = mid + 1;
				return BinarySearch( );
			}
		}
	}
	
	return false;
}


/*************************************************************************/
/**  \fn      int Logger::GetLine( char* msg, int line )
***  \brief   Class function
**************************************************************************/

bool Logger::GetLength( unsigned int &length )
{
	FRESULT r;
	if ( logging )
	{
		if ( !open )
		{
			open = true;
			if ( ( r = f_open( &file, filename, FA_OPEN_ALWAYS | FA_READ ) ) == FR_OK )
			{			
				length = f_size( &file ) / LINE_LENGTH;
				f_close( &file );
				open = false;
				return true;
			}
			open = false;
		}
	}
	return false;
}
							

/*************************************************************************/
/**  \fn      int Logger::GetLine( char* msg, int line )
***  \brief   Class function
**************************************************************************/

bool Logger::GetLine( char* msg, int line )
{
	FRESULT r;
	if ( logging )
	{
		if ( !open )
		{
			open = true;
			if ( ( r = f_open( &file, filename, FA_OPEN_ALWAYS | FA_READ ) ) == FR_OK )
			{			
				if ( line * LINE_LENGTH < f_size( &file ) )
				{
					if ( f_lseek( &file, line * LINE_LENGTH + LOG_MAX_TYPE * 4 + 3) == FR_OK )
					{
						uint32_t read;
						if ( f_read( &file, msg, LINE_LENGTH - LOG_MAX_TYPE * 4 - 3, &read ) == FR_OK )
						{
							f_close( &file );
							open = false;
							msg[ LINE_LENGTH - LOG_MAX_TYPE * 4 - 3 ] = 0;
							return true;
						}
					}
				}
				f_close( &file );	
			}
		}
	}
	open = false;
	return false;
}
					

/*************************************************************************/
/**  \fn      int Logger::GetLine( char* msg, int& mtype, int spfilter )
***  \brief   Class function
**************************************************************************/

int Logger::GetLine( char* msg, int& mtype, int spfilter )
{
	FRESULT r;
	if ( logging )
	{
		if ( !open )
		{
			open = true;
			if ( ( r = f_open( &file, filename, FA_OPEN_ALWAYS | FA_READ ) ) == FR_OK )
			{			
				if ( line < NumLogLines( spfilter ) && line >= 0 )
				{
					int size = f_size( &file );
					
					int lines = size / LINE_LENGTH;
					
					if ( lines > 0 && line < lines )
					{
						char spbuff[ LOG_MAX_TYPE * 4 ];
						
						min = 0;
						max = lines - 1;
						buff = spbuff;
						filter = spfilter;
						iterations = 0;
						
						if ( BinarySearch( ) )
						{
							char type[ 3];
							unsigned int got;
							bool found = false;
							
							while ( 1 )
							{
								if ( ( r = f_read( &file, type, 3, &got ) ) == FR_OK )
								{
									mtype = type[ 1] - '0';
									
									// if type within filter we're good
									if ( ( 1 << mtype ) & filter )
									{
										found = true;
										break;
									}
									
									int pos = f_tell( &file );
									
									if ( pos <= LOG_MAX_TYPE * 4 + 3 )
									{
										break; 		// hit start of file - cant go further
									}
									if ( ( pos & ( LINE_LENGTH - 1 ) ) != ( LOG_MAX_TYPE * 4 ) + 3)
									{
										break; 		// wrong offset - oh dear
									}
									if ( ( r = f_lseek( &file, pos - LINE_LENGTH - 3 ) ) != FR_OK )	// go back a line
									{
										FERR( "seek", r );
										break;
									}
								}
								else
								{
									FERR( "read", r );
									break;
								}
							}
							if ( found )
							{	
								int length = LINE_LENGTH - LOG_MAX_TYPE * 4 - 5;
								if ( ( r = f_read( &file, msg, length, &got ) ) == FR_OK )
								{
									msg[ length  ] = 0; //terminate
									f_close( &file ); 
									open = false;
									return true;
								}		
							}
						}
					}
				}
				f_close( &file ); 
			}
		}		
	}
	open = false;

	return false;
}
	
 
 

/*************************************************************************/
/**  \fn      int Encode( int value )
***  \brief   Encodes 24bit binary value into 32bit ASCII
**************************************************************************/

int Encode24( int val )
{
	char n[ 4];
	n[ 0] = ( (val >> 0 ) & 63 ) + 32;
	n[ 1] = ( (val >> 6 ) & 63 ) + 32;
	n[ 2] = ( (val >> 12) & 63 ) + 32;
	n[ 3] = ( (val >> 18) & 63 ) + 32;
	
	return *( (int*) n );
}

	
/*************************************************************************/
/**  \fn      Logger::Encode( char* buff )
***  \brief   Constructor for class
**************************************************************************/

void Logger::Encode( int* output, int* input )
{
	int* p = output;
	
	for( int n = 0; n < LOG_MAX_TYPE; n++ )
	{
		*p++ = Encode24( input[ n] );
	}
}


/*************************************************************************/
/**  \fn      int Decode24( int value )
***  \brief   Decodes 32bit  ASCII value into 24bit binary
**************************************************************************/

int Decode24( int val )
{
	char* p = (char*) &val;
	
	int result = 0;
	
	result += ( p[ 0] - 32 ) << 0;
	result += ( p[ 1] - 32 ) << 6;
	result += ( p[ 2] - 32 ) << 12;
	result += ( p[ 3] - 32 ) << 18;
	
	return result;
}

	
/*************************************************************************/
/**  \fn      Logger::Decode( char* buff )
***  \brief   Constructor for class
**************************************************************************/

void Logger::Decode( int* output, int* input )
{
	int* p = output;
	
	for( int n = 0; n < LOG_MAX_TYPE; n++ )
	{
		*p++ = Decode24( input[ n] );
	}
}

