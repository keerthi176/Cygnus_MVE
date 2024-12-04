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

#ifndef _LOGGER_H_
#define _LOGGER_H_


/* System Include Files
**************************************************************************/
#include <stdarg.h>
#include "ff.h"

/* User Include Files
**************************************************************************/
#include "MM_Module.h"



/* Defines
**************************************************************************/
#define LINE_LENGTH		128
#define MAX_LOG_LINES	64 	// 256 * 1024 )
#define MAX_FILE_SIZE 	( LINE_LENGTH * MAX_LOG_LINES )
#define LOG_MAX_TYPE		8

void CreateLogMessage(char* logMessageBuffer, char* logMessage,  uint8_t msgSize);

class SDFATFS;
class EmbeddedWizard;
	
 
class Logger  
{
	public:
		
	bool Init( SDFATFS* sd, const char* filename, BYTE mode );
 
	void Msg( int lt, const char* fmt, va_list args );

	int GetLine( char* msg, int& type, int filter );
	int NumLogLines( int filter );
	bool GetLine( char* msg, int line );
	bool GetLength( unsigned int &length );
	
	int 	line;
	bool  open;
	int iterations;
	
	private:
	
	int DoWrite( char* s );	
	int Write( char* s );	
	void Decode( int* ouput, int* input );
	void Encode( int* ouput, int* input );
	int BinarySearch( ); // int min, int max, char* buff, int line, int filter );
		
	void Restart( );
	
	const char* filename;	
	int 			min;
	int 			max;
	char* 		buff;
	int 			filter;
	SDFATFS* 	sdfs;
	FIL 			file;
	
	int Total( int filter, int* array );

	int count[ LOG_MAX_TYPE ]; 

	int logging;
};


#endif

