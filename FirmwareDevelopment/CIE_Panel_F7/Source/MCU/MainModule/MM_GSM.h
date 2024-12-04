/***************************************************************************
* File name: MM_GSM.h
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
* Short Messaging Service Module
*
**************************************************************************/

#ifndef _GSM_
#define _GSM_

/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_Settings.h"
#include "MM_EEPROM.h"
#include "MM_QuadSPI.h"
#include "CO_Site.h"
#include "MM_list.h"
#include "DM_UartHandler.h"
#include "MM_Fault.h"



#define GSM_TEXT_QUEUE_SIZE ( 8 * SITE_MAX_CONTACTS )
#define RX_BUFFER_SIZE	1024
#define TX_BUFFER_SIZE  640

#define GSM_PRIORITY 	50

typedef __packed struct
{
	char number[15];
	char event_flags;
} PhoneRecord;


class Settings;
class GSM;

enum Protocol
{
	PROT_TCP	= 6,
	PROT_UDP = 17,
	PROT_MQTT = 99
};

class Socket
{
	public:
		
	friend class GSM;

	virtual void Receive( char* data, int len, int packet_num, bool more ) = 0;
	
	Socket( const char* domain, const char* url, int port, Protocol protocol = PROT_TCP, bool stay_connected = false );
	
	virtual void Create( );
	virtual void Reset( );
	virtual void Sent( );
	
	char 			dest_ip[ 4];
	const char* dest_domain;
	const char* dest_url;
	int 			dest_port;
	
	char is_connected;
	char has_data;
	
	char protocol;
	char stay_connected;
	int port;
	time_t lastrequest;
	
	private:
			

	int num;	
};

enum
{
	TCP_CLOSED = 0,
	TCP_LISTENING,
	TCP_SYNC_SENT,
	TCP_SYNC_RECEIVED,
	TCP_ESTABLISHED,
	TCP_FIN_WAITING1,
	TCP_FIN_WAITING2,
	TCP_CLOSE_WAIT,
	TCP_CLOSING,
	TCP_LAST_ACK,
	TCP_TIME_WAIT
};
	

typedef enum
{ 
  GSM_RESET_COMS,           // AT+CFUN=15
	GSM_ECHO_OFF,				  		// ATE0
	GSM_SET_MNO_PROFILE,      // AT+UMNOPROF=100
	GSM_GET_MNO_PROFILE,	     // AT+UMNOPROF?
	GSM_SELECT_2G_4G,		      // AT+URAT=8/9\r\n
	GSM_CHECK_REGISTRATION,	  // AT+CGREG?
	GSM_SET_DOMAIN,           // AT+USVCDOMAIN=2
	GSM_CHECK_DOMAIN,         // AT+USVCDOMAIN?
	GSM_CHECK_NETWORK,        // AT+COPS?	
	GSM_SEARCH_NETWORK,	     // AT+COPS=0
	GSM_CHECK_NETWORKS,		
	GSM_ADD_ACCESS_POINT,     // AT+CGDCONT=1,"IP","x.com"
	GSM_SET_PASSWORD,					// AT+UPSD=0,3,"pwd"
	GSM_SET_ACCESS_POINT,     // AT+CGACT=1,1
	GSM_CHECK_ATTATCHMENT,    // AT+CGATT?
	GSM_ATTACH_GPRS,          // AT+CGATT=1
	GSM_GET_IP,               // AT+CGPADDR=2
	GSM_LOOKUP_IP,            // AT+UDNSRN=0,"x.com"
	GSM_CREATE_SOCKET,		  // AT+USOCR=6
	GSM_OPEN_SOCKET,          // AT+USOCO=x
	GSM_CLOSE_SOCKET,			  // AT+USOCL=x
	GSM_DESTROY_SOCKET,			//
	GSM_WRITE_TO_SOCKET,		  // AT+USOWR 	  
	GSM_WRITE_HEADER,				// 
	GSM_WRITE_BODY,	
	GSM_READ_FROM_SOCKET,	  // AT+USORD=%	
	GSM_SOCKET_STATE,		// AT+USOCTL=0,10
	GSM_SOCKET_ERROR,		// AT+USOER
	GSM_SOCKET_OPTION,
	GSM_SET_VALIDATION,		  // AT+USECPRF=0,0,0
	GSM_SET_SSL_TLS_PROF,	  // AT+USECPRF=0,1,0
	GSM_SET_CIPHER_PROF,		  // AT+USECPRF=0,2,0
	GSM_SET_CERT_PROF,		  // AT+USECPRF=0,3,"cert"
	GSM_SET_SERVER_PROF,		  // AT+USECPRF=0,1,"host"
	
	GSM_RESET_PROFILE,		  // AT+USECPRF=0
	
	GSM_ASSOCIATE_PROFILE,		//AT+USOSEC=<socket>,<ssl_tls_status>[,<usecmng_profile_id>]
	
	GSM_ASCI_TEXT_MODE,       // AT+CMGF=1
 	GSM_POWER_UP,             // AT+CFUN=1
	GSM_CHECK_TEXT,           // AT+CMGS=?
	GSM_SEND_NUMBER,          // AT+CMGS=
	GSM_SEND_TEXT,            // >
	GSM_POWER_DOWN,           // AT+CFUN=0
	GSM_SIGNAL_QUALITY,       // AT+CSQ
	
	
	MQTT_SET_USER_PASSWORD,	  // AT+UMQTT=4,"test","abc123"
	MQTT_SET_CLIENT_ID,			  // AT+UMQTT=0,"id"
	MQTT_SET_PORT,				  // AT+UMQTT=1,x
	MQTT_SET_SERVER,			   // AT+UMQTT=2,"x.y.com",8883
	MQTT_SET_IP,			   	// AT+UMQTT=3,"1.2.3.4"
	MQTT_PUBLISH,				   // AT+AT+UMQTTC=2,0,0,0,"sensor/","msg"
	MQTT_SUBSCRIBE,				// AT+UMQTTC=4,0,"sensor/heat/#"
	MQTT_LOGON,
	GSM_IDLE,                 // 	
	
	
	GSM_DO_NOTHING,
	GSM_TEST_BAND,
	

} GSMState;


struct GSMText
{
	GSMText( SMSContact* contact, int actions, char group, char count );
	GSMText( SMSContact* contact, int actions );
	GSMText( SMSContact* contact, short unit, short parent, bool connected, short total );
	
	SMSContact* 	contact;
	int				actions;
	char				faultgroup;
	char				count;
	short				parent;

};

typedef enum
{	
	LTEM = 7,
	NB_IOT = 8,
	GPRS = 9,
} UratType;

class GSM : public Module//, public UartHandler
{
	public:
	
	GSM( );
	
	virtual int Init( );
	virtual int Receive( Event event );
	virtual int Receive( Command* );
	virtual int Receive( Message* ); 
	
	virtual void Poll( ); 
	
	void Reset( );
	
 //	virtual void UartCommand( UART_HandleTypeDef* uh );
 //	virtual void UartTxComplete( UART_HandleTypeDef* uh );
//	virtual void UartRxComplete( UART_HandleTypeDef* uh );
 //	virtual void UartError( UART_HandleTypeDef* uh );	
	
	void CreateText( char* buff, int actions, int group, int type );
	void CreateDeviceText( char* buff, int actions, int unit, int parent, int total );
	void CreateDeviceClearText( char* buff );
	void DeviceNotification( short unit, short parent, bool connected, int total );
	void MeshRestartNotification( );
	void ProcessEvents( int e );
	
	int SendData( char* data );
	 
	static Socket* socket;

	int network_state;
	int network_num;
	int network_domain;
	int network_attached;
	char network_name[ 40 ];

	static int connected;
	char ip[ 4];
	static char* out_data;
	static int out_data_len;
	static int out_header_len;
	static char* out_header;
	static GSMState next_state;
	int data_in_length; 
	static bool txready;
	 
	void ClearFaultCount( );
	
	int ProcessFault( int group );
	
	const char* GetGroup( int n );

	short enabled;
 
	char faultcount[ FAULTGROUP_SYSTEM ];
		
	void SendReceiveBig( const char* s, int len );
	void SendReceive( const char* s, ...);	
	void SendReceive( );	
	
	static int Send( Socket* s, char* data, int len );	
	static int Send( Socket* s, char* header, char* data, int len );	
 
	void ProcessTx( );
	void ProcessRx( int bytes );	
	
	int ResultOK( );
	int NotOK( );

	char* txbuffer = (char*) 0x20000080;  				// [ TX_BUFFER_SIZE ]; ( DMA'able )
	char* rxbuffer = (char*) 0x20000300;   // [ RX_BUFFER_SIZE ]; ( DMA'able )	 

	static GSMState state;
	GSMState write;
	static char apn[ 40];
	static char pwd[ 40];

	private:
		

	int request;		
	int packet_num;
	
	Circular<GSMText>		textqueue;
	char					   textqueuebuff[ GSM_TEXT_QUEUE_SIZE * sizeof(GSMText) ];
	
	int 	signal_strength;
	short signal_period;
	int 	waitseconds;
	int 	senttext;
	
	
	UratType		urat;
	
	EEPROM* 		eeprom;
	Settings* 	settings;
	QuadSPI* 	qspi;
};

	
#endif
