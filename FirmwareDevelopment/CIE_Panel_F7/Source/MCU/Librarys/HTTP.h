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
*******************************
 

/* Defines
**************************************************************************/


#define DEST_DOMAIN "panelwebapplication1.azurewebsites.net"
#define DEST_PORT 80 


class HTTP
{
	public:
		
	static int Put( char* buff, const char* resource, char* token, const char* data, ... );
	static int Post( char* buff, const char* resource, char* token, const char* data, ... );

	char* Response( char* buff, int& length );
	
	private:
		
	static int Request( char* buff, const char* prefix, const char* resource, const char* data, char* token = NULL );
}; 
 

