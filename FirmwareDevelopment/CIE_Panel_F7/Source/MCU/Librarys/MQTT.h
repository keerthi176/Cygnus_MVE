
/***************************************************************************
* File name: MM_MQTT.h
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
* MQTT support
*
*****************************************************************************/
 
#ifndef _MQTT_
#define _MQTT_
 
 
/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_GSM.h"
#include <time.h>

/* Defines 
**************************************************************************/



#define MQTT_PORT 8883 
 
class MQTTSocket : public Socket
{
	public:
		
	MQTTSocket( const char* domain, const char* device_url  );
		
	
	
	int Put( const char* resource, char* token, void (*callback)( int code, char* data, int length ), int id, char* data = "" );
	int Delete( const char* resource, char* token, void (*callback)( int code, char* data, int length ), int id, char* data = "" );
	
	int Post( const char* resource, char* token, void (*callback)( int code, char* data, int length ), const char* data  );

	virtual void Receive( char* data, int len, int packet_num, bool more );
	
	char* token;
	time_t lastrequest;
	
	private:
		
	void (*response)( int resp, char* data, int length ); 
		
	int Request( const char* prefix, const char* resource, const char* data, void (*callback)( int code, char* data, int length ), char* token = NULL );
}; 
 

#endif

