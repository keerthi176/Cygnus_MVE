/***************************************************************************
* File name: HTML.cpp
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
* HTML support
*
**************************************************************************/

/* System Include Files
**************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


/* User Include Files
**************************************************************************/
 
#include "HTTP.h" 
 
 
 
/*************************************************************************/
/**  \fn      char* HTTP::Response( char* buff, int& length )
***  \brief   Class function
**************************************************************************/

char* HTTP::Response( char* buff, int& length )
{
	char* p = strstr( buff, "Content-Length:" );
		 
	if ( p != NULL )
	{
		buff = p;
		if ( sscanf( p + 16, "%d\r", &length ) != 1 )
		{
			length = 0;
		}		
	}
	p = strstr( p + 17, "\r\n\r\n" );
	
	if ( p != NULL ) p += 4;
	
	return p;
}


/*************************************************************************/
/**  \fn      int HTTP::Put( char* buff, const char* resource, const char* data, char* token )
***  \brief   Class function
**************************************************************************/

int HTTP::Put( char* buff, const char* resource, char* token, const char* data, ... )
{
	char tbuff[ 400];
	va_list args;
		
	va_start( args, data );
	
	vsnprintf( tbuff, sizeof(tbuff), data, args );
	
	va_end( args );
	
	return Request( buff, "PUT", resource, tbuff, token );
}


/*************************************************************************/
/**  \fn      int HTTP::Post( char* buff, const char* resource, const char* data, char* token )
***  \brief   Class function
**************************************************************************/

int HTTP::Post( char* buff, const char* resource, char* token, const char* data, ... )
{
	char tbuff[ 400];
	va_list args;
		
	va_start( args, data );
	
	vsnprintf( tbuff, sizeof(tbuff), data, args );
	
	va_end( args );
	
	return Request( buff, "POST", resource, data, token );
}


/*************************************************************************/
/**  \fn      int HTTP::HTTPRequest( char* prefix, char* resource, char* data, char* token )
***  \brief   Class function
**************************************************************************/

int HTTP::Request( char* buff, const char* prefix, const char* resource, const char* data, char* token )
{
	return sprintf( buff, "%s /%s HTTP/1.1\r\nHost: %s\r\n%s%s%sUser-Agent: matts/panel\r\nAccept: */*\r\nContent-Type: text/json\r\nContent-length: %d\r\n\r\n%s",
				prefix, resource, DEST_DOMAIN, token != NULL ? "Authorization: Bearer " : "", token != NULL ? token : "", token != NULL ? "\r\n":"", strlen( data ), data );
}


 