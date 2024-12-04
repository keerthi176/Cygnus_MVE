/***************************************************************************
* File name: MM_HTTP.h
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
* HTTP support
*
*****************************************************************************/
 
#ifndef _HTTP_
#define _HTTP_
 
 
/* User Include Files
**************************************************************************/
#include "MM_Module.h"
 
#include <time.h>

/* Defines 
**************************************************************************/

class Socket;

#define HTTP_PORT 80 
#define HTTPS_PORT 443

 
class HTTPSocket : public Socket
{
	public:
		
	HTTPSocket( const char* domain, const char* url, bool secure = true );
		
	int Put( const char* resource, char* token, void (*callback)( int code, char* data, int length ), int id, char* data = "" );
	int Delete( const char* resource, char* token, void (*callback)( int code, char* data, int length ), int id, char* data = "" );
	
	int Post( const char* resource, char* token, void (*callback)( int code, char* data, int length ), char* data  );

	virtual void Receive( char* data, int len, int packet_num, bool more );
	
	char* token;
	time_t lastrequest;
	
	private:
		
	void (*response)( int resp, char* data, int length ); 
		
	int Request( const char* prefix, const char* resource, char* data, void (*callback)( int code, char* data, int length ), char* token = NULL );
}; 
 

#endif

