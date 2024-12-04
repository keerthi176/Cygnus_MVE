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
#include "MM_IOStatus.h"
#include "MM_GSM.h"

#include "MM_HTTP.h" 
 
#include "MM_CUtils.h"

 
/*************************************************************************/
/**  \fn      HTTPSocket::HTTPSocket( )
***  \brief   Constructor for class
**************************************************************************/

HTTPSocket::HTTPSocket( const char* domain, const char* url, bool secure ) : Socket( domain, url, HTTPS_PORT )
{
	
}

 
/*************************************************************************/
/**  \fn      char* HTTP::Receive( char* buff, int& length )
***  \brief   Class function
**************************************************************************/

void HTTPSocket::Receive( char* buff, int len, int packet_num, bool more )
{
	static int resp;
	
	if ( buff == NULL )
	{
		return response( 0, NULL, 0 ); 
	}
	
	if ( packet_num == 0 )
	{	
		char* html = strstr( buff, "HTTP/1.1" );
		
		if ( html != NULL )
		{
			if ( sscanf( html, "HTTP/1.1 %d", &resp ) == 1 )
			{		
				char* chunked = strstr( html, "Encoding: chunked" );
				
				char* body = strstr( html, "\r\n\r\n" ) + 4;
				
				if ( body == NULL ) response( resp, NULL, 0 ); 
				
				if ( chunked != NULL )
				{
					body = strstr( body, "\r\n" ) + 2;
				}
				
				if ( body == NULL ) return response( resp, NULL, 0 ); 		
				
				int length = ( buff + len ) - body;
				
				if ( length != 0 )
				{					
					return response( resp, body, length );
				}
				else
				{
					return response( resp, NULL, 0 );
				}				  
			}
		}
		
		return response( 0, NULL, 0 ); 		
	}
	else
	{
		return response( resp, buff, len );
	}	 
}


/*************************************************************************/
/**  \fn      int HTTPSocket::Put( const char* resource, char* token, void (*callback)( int code, char* data, int length ), int id )
***  \brief   Class function
**************************************************************************/

int HTTPSocket::Put( const char* resource, char* token, void (*callback)( int code, char* data, int length ), int id, char* data )
{
	if ( GSM::state == GSM_IDLE )
	{
		char res[ 128 ];
		
		snprintf( res, sizeof( res), "%s/%d", resource, id );
		
		return Request( "PUT", res, data, callback, token );
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int HTTPSocket::Delete( const char* resource, char* token, void (*callback)( char* data, int length ), int id )
***  \brief   Class function
**************************************************************************/

int HTTPSocket::Delete( const char* resource, char* token, void (*callback)( int code, char* data, int length ), int id, char* data )
{
	if ( GSM::state == GSM_IDLE )
	{
		char res[ 128 ];
		
		snprintf( res, sizeof( res), "%s/%d", resource, id );
		
		return Request( "DELETE", res, data, callback, token );
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int HTTP::Post( char* buff, const char* resource, const char* data, char* token )
***  \brief   Class function
**************************************************************************/

int HTTPSocket::Post( const char* resource, char* token, void (*callback)( int code, char* data, int length ), char* data )
{	
	if ( GSM::state == GSM_IDLE )
	{		
		return Request( "POST", resource, data, callback, token );
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int HTTP::HTTPRequest( char* prefix, char* resource, char* data, char* token )
***  \brief   Class function
**************************************************************************/

int HTTPSocket::Request( const char* prefix, const char* resource, char* data, void (*callback)( int code, char* data, int length ), char* token )
{
	static char header[ 1024 ];
	
	app.DebOut( "%s: %s\n", prefix, data );
	
	snprintf( header, 1024, "%s /%s HTTP/1.1\r\nHost: %s\r\n%s%s%sAccept: */*\r\nContent-Type: application/json\r\nContent-length: %d\r\n\r\n",
				prefix, resource, dest_url, token != NULL ? "Authorization: Bearer " : "", token != NULL ? token : "", token != NULL ? "\r\n":"", strlen( data ) );
	
	response = callback;
	
	
	
	return GSM::Send( this, header, data, strlen( data ) );
}


 