



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
 
#include "MM_GSM.h"

#include "MQTT.h" 
 
#include "MM_CUtils.h"

 
 
 
 
 
/*************************************************************************/
/**  \fn      MQTTSocket::MQTTSocket( )
***  \brief   Constructor for class
**************************************************************************/

MQTTSocket::MQTTSocket( const char* domain, const char* device_url ) : Socket( domain, device_url, MQTT_PORT, PROT_MQTT, true )
{
	port = 69;
}


  

 
/*************************************************************************/
/**  \fn      char* MQTT::Receive( char* buff, int& length )
***  \brief   Class function
**************************************************************************/

void MQTTSocket::Receive( char* buff, int len, int packet_num, bool more )
{
	 
}


/*************************************************************************/
/**  \fn      int MQTTSocket::Put( const char* resource, char* token, void (*callback)( int code, char* data, int length ), int id )
***  \brief   Class function
**************************************************************************/

int MQTTSocket::Put( const char* resource, char* token, void (*callback)( int code, char* data, int length ), int id, char* data )
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
/**  \fn      int MQTTSocket::Delete( const char* resource, char* token, void (*callback)( char* data, int length ), int id )
***  \brief   Class function
**************************************************************************/

int MQTTSocket::Delete( const char* resource, char* token, void (*callback)( int code, char* data, int length ), int id, char* data )
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
/**  \fn      int MQTT::Post( char* buff, const char* resource, const char* data, char* token )
***  \brief   Class function
**************************************************************************/

int MQTTSocket::Post( const char* resource, char* token, void (*callback)( int code, char* data, int length ), char* data )
{	
	if ( GSM::state == GSM_IDLE )
	{		
		return Request( "POST", resource, data, callback, token );
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int MQTT::MQTTRequest( char* prefix, char* resource, char* data, char* token )
***  \brief   Class function
**************************************************************************/

int MQTTSocket::Request( const char* prefix, const char* resource, char* data, void (*callback)( int code, char* data, int length ), char* token )
{
	static char header[ 1024 ];
	
	app.DebOut( "%s: %s\n", prefix, data );
	
	int datalen = strlen( data );
	
	sprintf( header, "%s /%s MQTT/1.1\r\nHost: %s\r\n%s%s%sUser-Agent: matts/panel\r\nAccept: */*\r\nContent-Type: application/json\r\nContent-length: %d\r\n\r\n",
				prefix, resource, dest_url, token != NULL ? "Authorization: Bearer " : "", token != NULL ? token : "", token != NULL ? "\r\n":"", datalen);
	
	response = callback;
	lastrequest = now( );
	
	return GSM::Send( this, header, data, datalen );
}


 