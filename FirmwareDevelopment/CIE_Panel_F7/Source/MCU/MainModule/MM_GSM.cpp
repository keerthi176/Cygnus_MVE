/***************************************************************************
* File name: MM_GSM.cpp
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
* GSM Module
*
**************************************************************************/

/* System Include Files
**************************************************************************/
#ifdef STM32F767xx
#include "stm32f7xx_hal_dma.h"
#include "stm32f7xx_hal_usart.h"
#include "stm32f7xx_hal_rtc.h"
#else
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_usart.h"
#include "stm32f4xx_hal_rtc.h"
#endif
#include <string.h>
#include "stdio.h"


/* User Include Files
**************************************************************************/
#include "MM_GSM.h"
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_Application.h"
#include "MM_Events.h"
#include "MM_fault.h"
#include "MM_device.h"
#include "MM_error.h"
#include "CO_Site.h"
#include "CO_Site.h"
#include "MM_GPIO.h"
#include "MM_QuadSPI.h"
#include "MM_CUtils.h" 
#include "MM_Utilities.h" 

#ifdef GSM_HTTP			
#include "MM_AppSupport.h"
#else
#include "MM_MQTTSupport.h"
#endif	



/* Defines
**************************************************************************/
#define GSM_TITLE  "Cygnus Panel Alert"
#define GSM_TITLE2 "===================="


#define GSM_REQUEST_SMS			1
#define GSM_REQUEST_TCPIP		2 
#define GSM_REQUEST_SIGNAL 	3
#define GSM_REQUEST_STARTUP   4

#define MQTT_PORT_NUMBER 8883
#define MINIMUM_RESET_DIFF 200
	
const char* tcp[] = {	
"CLOSED",
"LISTENING",
"SYNC_SENT",
"SYNC_RECEIVED",
"ESTABLISHED",
"FIN_WAITING1",
"FIN_WAITING2",
"CLOSE_WAIT",
"CLOSING",
"LAST_ACK",
"TIME_WAIT",
};



/*************************************************************************/ 
/**  \fn      Socket::Socket( const char* domain, const char* url, int port n )
***  \brief   Constructor
**************************************************************************/

Socket::Socket( const char* domain, const char* url, int port, Protocol protocol, bool stay_connected )
    	: stay_connected( (char) stay_connected ), protocol( (char) protocol ), dest_domain( domain ), dest_url( url ), dest_port( port )
{ 
	*(int*)dest_ip = 0;
}	


/*************************************************************************/ 
/**  \fn      Socket::Create( )
***  \brief   virt. class function
**************************************************************************/

void Socket::Create( )
{
	//
}


/*************************************************************************/ 
/**  \fn      Socket::Sent( )
***  \brief   virt. class function
**************************************************************************/

void Socket::Sent( )
{
	//
}

/*************************************************************************/ 
/**  \fn      Socket::Reset( )
***  \brief   virt. class function
**************************************************************************/

void Socket::Reset( )
{
	//
}

static int last_rx_count = 0;

/*************************************************************************/
/**  \fn      GSMText::GSMText( )
***  \brief   Constructor for class
**************************************************************************/

GSMText::GSMText( SMSContact* contact, int actions ) : contact( contact ), actions( actions ), faultgroup( 0 ), count( 0 )
{
	
}

GSMText::GSMText( SMSContact* contact, int actions, char faultgroup, char count ) : contact( contact ), actions( actions ), faultgroup( faultgroup ), count( count )
{
	
}

GSMText::GSMText( SMSContact* contact, short unit, short parent, bool connected, short total ) : contact( contact ), 
	actions( connected ? ACTION_BIT_DEVICE_CONNECT | unit : ACTION_BIT_DEVICE_DISCONNECT | unit ), faultgroup( total & 255 ), count( ( total >> 8 ) & 255 ), parent( parent )
{
	
}

char GSM::apn[ 40 ] = { 0 };
char GSM::pwd[ 40 ] = { 0 };

static GSM* dis = NULL;
int GSM::connected = false;

Socket* GSM::socket = NULL;
GSMState GSM::state = GSM_RESET_COMS;
GSMState GSM::next_state = GSM_ECHO_OFF;
 

char* 				GSM::out_data;
int 					GSM::out_data_len;
char* 				GSM::out_header;
int 				   GSM::out_header_len;
static GSMState   checked;

static int check_signal = 0;
static bool still_not_working = false;
static int long_reset = 0;
bool GSM::txready = true;

static bool powered_down = false;

/*************************************************************************/
/**  \fn      GSM::GSM( )
***  \brief   Constructor for class
**************************************************************************/

GSM::GSM( ) : Module( "GSM", GSM_PRIORITY, EVENT_CLEAR | EVENT_RESET ), textqueue( (GSMText*) textqueuebuff, GSM_TEXT_QUEUE_SIZE )					
{
	signal_strength = -1;

	Reset();
 
	dis = this;
	
	if ( !EEPROM::GetVar( SMS_Enabled, enabled ) )
	{
		if ( !EEPROM::GetVar( SMS_Enabled, enabled ) )
		{
			Fault::AddFault( FAULT_GSM_SMS_ENEBLED_GET );
			//Log::Msg(LOG_SYS, "01_GSM.cpp -- ADD");
		}
	}
	
	app.gsm_uart->TxCpltCallback = HAL_UART_TxCpltCallback;
		
	ClearFaultCount( );
}


/*************************************************************************/
/**  \fn      int GSM::Reset( )
***  \brief   Initializer for class
**************************************************************************/

void GSM::Reset( )
{
	static time_t last_reset = 0;
	
	if ( still_not_working || last_reset + MINIMUM_RESET_DIFF > now( ) )
	{
		app.DebOut( "GSM Long reset!\n" );
		// start long reset
		long_reset = 80;
		HAL_GPIO_WritePin( GSM_RESET_GPIO_Port,  GSM_RESET_Pin, GPIO_PIN_SET ) ;
		 
		still_not_working = false;
	}
	else
	{		
		app.DebOut( "GSM quick reset!\n" );
		still_not_working = true;
		last_reset = now();
		 			
		// Set power high ( low on module ) 
		HAL_GPIO_WritePin( GSM_EN_GPIO_Port,  GSM_EN_Pin, GPIO_PIN_SET );
		
		// Wait 160ms
		HAL_Delay( 160 );
		
		// Bring power back to low ( high on module ) 
		HAL_GPIO_WritePin( GSM_EN_GPIO_Port,  GSM_EN_Pin, GPIO_PIN_RESET );
		
		HAL_Delay( 180 );
		
		HAL_GPIO_WritePin( GPIOD, GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_SET );
		HAL_Delay( 180 );
		HAL_GPIO_WritePin( GPIOD, GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET );
	}
	 
	
	txready = true;
	
	// for some reason get stuck after reset - so skip reset if on echo off..
	if ( state != GSM_ECHO_OFF )
	{
		state = GSM_RESET_COMS;
	}
	
	if ( socket != NULL )
	{
		socket->Reset( );
		socket->is_connected = false;
		socket->has_data = false;
	}
}


/*************************************************************************/
/**  \fn      int GSM::Init( )
***  \brief   Initializer for class
**************************************************************************/

int GSM::Init( )
{
	qspi 	  = (QuadSPI*)  app.FindModule( "QuadSPI" ) ;
	settings = (Settings*) app.FindModule( "Settings" );
	
	assert( qspi != NULL );
	assert( settings != NULL );
 	 
	*apn  = 0;
	*pwd = 0;
		
	if ( !EEPROM::GetVar( AppSupp_APN, *apn ) )
	{
		if ( !EEPROM::GetVar( AppSupp_APN, *apn ) )
		{
			Fault::AddFault( FAULT_GSM_APN_GET );
			//Log::Msg(LOG_SYS, "02_GSM.cpp -- ADD");
		}
	}
	apn[ 39] = 0;  // tut tut
 	
	if ( !EEPROM::GetVar( AppSupp_Password, *pwd ) )
	{
		if ( !EEPROM::GetVar( AppSupp_Password, *pwd ) )
		{
			Fault::AddFault( FAULT_GSM_PASSWORD_GET );
			//Log::Msg(LOG_SYS, "03_GSM.cpp -- ADD");
		}
	}
	pwd[ 39] = 0;
	
	signal_period = 3 * 60;
	
	strcpy( network_name, "<Unknown>" );
		
	urat = GPRS;
	
	 
	
	return true;  // OK as MCC etc not used
}


/*************************************************************************/
/**  \fn      virtual void GSM::Poll( )
***  \brief   Class function
**************************************************************************/

void GSM::Poll( )
{
	static int  second = 0;
	static int  retrys = 0;
	static int  prev_state = GSM_IDLE;
	static int  prev_state_count = 0;
	static int  errors = 0;
	static int  same_state_count = 0;
	static int  delay_secs = 0;
	
#ifdef GSM_HTTP
	// If SMS or App server enabled
	if ( AppSupport::enabled || GSM::enabled )
#else
	// If SMS or App server enabled
	if ( MQTTSupport::enabled || GSM::enabled )
#endif		
	{
		if ( powered_down )
		{
			state = GSM_POWER_UP;
			next_state = GSM_ECHO_OFF;
			powered_down = false;
		}
		
		// if during long reset count down
		if ( long_reset > 0 )
		{
			// if end of count
			if ( --long_reset == 0 )
			{
				// Set reset high again (gpio low)
				HAL_GPIO_WritePin( GSM_RESET_GPIO_Port, GSM_RESET_Pin, GPIO_PIN_RESET );
				HAL_Delay( 180 );
				HAL_GPIO_WritePin( GSM_EN_GPIO_Port, GSM_EN_Pin, GPIO_PIN_SET );
				HAL_Delay( 180 );
				HAL_GPIO_WritePin( GSM_EN_GPIO_Port, GSM_EN_Pin, GPIO_PIN_RESET );
				HAL_Delay( 180 );
				HAL_GPIO_WritePin( GPIOD, GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_SET );
				HAL_Delay( 180 );
				HAL_GPIO_WritePin( GPIOD, GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET );

			}
			return;
		}
		else if ( delay_secs-- > 0 )
		{
			// if countdown complete
			if ( delay_secs == 0 )
			{
				app.DebOut( "countdown complete..\n" );
				Reset( );
			}
			return;
		}	
		else if ( prev_state == state && state != GSM_IDLE )
		{
			// if stuck in same state?
			if ( prev_state_count++ > 70 )
			{
				if ( state == GSM_CHECK_ATTATCHMENT || state == GSM_CHECK_REGISTRATION )
				{
					if ( prev_state_count > 200 )
					{
						prev_state_count = 0;
						state = GSM_RESET_COMS;
						app.DebOut( "..can take time - go back..\n" );
					}
				}
				else 
				{
					prev_state_count = 0;
					app.DebOut( "GSM - stuck on same thing..\n" );
					Reset( );
				}
			}
		}
		else
		{
			prev_state_count = 0;
		}
		prev_state = state;
	
		if ( state == GSM_IDLE )
		{
			if ( !( check_signal++ % signal_period ) )
			{
				state = GSM_SIGNAL_QUALITY;
			}
			else if ( !textqueue.Empty( ) )
			{
				state = GSM_ASCI_TEXT_MODE;			
			}
			else if ( socket != NULL )
			{		 
				if ( socket->has_data )
				{
					if ( socket->stay_connected && socket->is_connected )
					{
						state = GSM_WRITE_TO_SOCKET;		
						
						socket->lastrequest = now( );
						
						if ( out_header_len > 0 )
						{
							write = GSM_WRITE_HEADER; 
						}
						else 
						{
							write = GSM_WRITE_BODY;
						}
					}
					else
					{		
						state = GSM_LOOKUP_IP;		
					}
		 
					txready = true;				 
				}
			}
		}
		else // not idle..
		{	
			if ( txready )
			{
				ProcessTx( );
				txready = false;
			}
			
			// if some receive data
			
		//	app.gsm_uart->RxXferCount = app.gsm_uart->hdmarx->Instance->NDTR;
				
			int bytes = RX_BUFFER_SIZE - app.gsm_uart->RxXferCount;
			
			if ( bytes > 0 )
			{
				if ( last_rx_count == app.gsm_uart->RxXferCount )
				{
					 
					if ( ( rxbuffer[ bytes - 1 ] == 10 && rxbuffer[ bytes - 2 ] == 13 ) ||
							rxbuffer[ bytes - 1 ] == '@' || rxbuffer[ bytes- 1 ] == '>' || rxbuffer[ bytes - 2 ]  == '>' ) 
					{
						//	app.gsm_uart->pRxBuffPtr = rxbuffer + bytes;
						
						if ( rxbuffer[ bytes- 1 ] == '>' || rxbuffer[ bytes - 2 ]  == '>' )
						{
							state = GSM_SEND_NUMBER; // => GSM_SEND_TEXT
						}
						
						*app.gsm_uart->pRxBuffPtr = 0;
						
						
						ProcessRx( bytes );
						 
						if ( strstr( rxbuffer, "ERROR" ) )
						{
							if ( errors++ > 9 )
							{
								app.DebOut( "Too many errors - counting down to reset!\n" );
								delay_secs = 35;  // start count till reset
								errors = 0; 
							}	
						}
						else
						{
							errors = 0;
							delay_secs = 0;	// cancel reset count
							still_not_working = false;
						}
						txready = true; 
						
						second = 0;
						retrys = 0;
						last_rx_count = -1;
						return;
					}
				}
				last_rx_count = app.gsm_uart->RxXferCount;			
			}
			
				
			if ( ++second > 30 ) // (not quite seconds)
			{
				second = 0;
				txready = true;
				
				app.DebOut( "GSM Try %d\n", retrys );
							
				if ( retrys++ > 5 )
				{
					app.DebOut( "GSM Give up\n" );
					retrys = 0;
					still_not_working = true;	// force long reset
					Reset( );
					/*
					switch( state )
					{
						case GSM_CHECK_REGISTRATION :
							 
							break;
						case GSM_SET_ACCESS_POINT :
							break;			// takes ages
						
						case GSM_CREATE_SOCKET :					
						case GSM_READ_FROM_SOCKET:
						case GSM_OPEN_SOCKET:
						case GSM_WRITE_TO_SOCKET:
						case GSM_WRITE_BODY:
						case GSM_WRITE_HEADER:
						case GSM_CHECK_ATTATCHMENT:
						{
							state = GSM_CLOSE_SOCKET;
							break;
						}
						default:
						{
							Reset( );
							break;
						}
					}	*/
				}
			}
		}
	}
	else
	{
		if ( !powered_down )
		{
			state = GSM_POWER_DOWN;
			SendReceive( "AT+CFUN=0\r\n");
			powered_down = true;
		}
	}
}			


#ifdef UART_HNADLER

/*************************************************************************/
/**  \fn      virtual void UartCommand( UART_HandleTypeDef* uh )
***  \brief   Class function
**************************************************************************/

void GSM::UartCommand( UART_HandleTypeDef* uh )
{
	ProcessRx( );
}

/*************************************************************************/
/**  \fn      virtual void GSM::UartError( UART_HandleTypeDef* uh )
***  \brief   Class function
**************************************************************************/

void GSM::UartError( UART_HandleTypeDef* uh )
{
	
}


/*************************************************************************/
/**  \fn      virtual void UartTxComplete( UART_HandleTypeDef* uh )
***  \brief   Class function
**************************************************************************/

void GSM::UartTxComplete( UART_HandleTypeDef* uh )
{
	HAL_UART_Receive_IT( app.gsm_uart, (uint8_t *)rxbuffer, RX_BUFFER_SIZE );	
}
#endif

extern "C" void HAL_UART_TxCpltCallback( UART_HandleTypeDef* uh )
{
//	HAL_UART_Receive_DMA( app.gsm_uart, (uint8_t *)dis->rxbuffer, RX_BUFFER_SIZE );	
	HAL_UART_Receive_IT( app.gsm_uart, (uint8_t *)dis->rxbuffer, RX_BUFFER_SIZE );	
	
	last_rx_count = RX_BUFFER_SIZE;
}
 

/*************************************************************************/
/**  \fn      char* GSM::GetGroup( int flags )
***  \brief   Class function
**************************************************************************/

const char* GSM::GetGroup( int n )
{
	if ( n == FAULTGROUP_GENERAL )
	{
		return "General CIE Fault";
	}
	if ( n == FAULTGROUP_DEVICE )
	{
		return "Device Fault";
	}
	if ( n == FAULTGROUP_TAMPER )
	{
		return "Tamper Fault";
	}
	if ( n == FAULTGROUP_INSTALLATION )
	{
		return "Installation Fault";
	}
	if ( n == FAULTGROUP_IOU)
	{
		return "I/O Unit Fault";
	}
	if ( n == FAULTGROUP_MAINS)
	{
		return "Mains Power Fault";
	}
	if ( n == FAULTGROUP_BATTERY )
	{
		return "Battery Fault";
	}
	if ( n == FAULTGROUP_SYSTEM )
	{
		return "Serious System Fault";
	}
	return "";
}


/*************************************************************************/
/**  \fn      void GSM::SendReceive( char* s )
***  \brief   Class function
**************************************************************************/

void GSM::SendReceive( )
{	
	int tosend = strlen( txbuffer );
			
	app.DebOut( "GSM(out %d): %s\n", tosend, txbuffer );
	
 	HAL_UART_AbortTransmit( app.gsm_uart );
 	HAL_UART_AbortReceive( app.gsm_uart );
				
 	memset( rxbuffer, 0, RX_BUFFER_SIZE );
  
	HAL_UART_Transmit_DMA( app.gsm_uart, (uint8_t*) txbuffer, tosend );
}	



/*************************************************************************/
/**  \fn      void GSM::SendReceive( char* s )
***  \brief   Class function
**************************************************************************/

void GSM::SendReceiveBig( const char* _txbuffer, int tosend )
{		
	app.DebOut( "GSM(out %d): %s\n", tosend, _txbuffer );
	
 	HAL_UART_AbortTransmit( app.gsm_uart );
 	HAL_UART_AbortReceive( app.gsm_uart );
				
 	memset( rxbuffer, 0, RX_BUFFER_SIZE );
  
	HAL_UART_Transmit_DMA( app.gsm_uart, (uint8_t*) _txbuffer, tosend );
}	
 



/*************************************************************************/
/**  \fn      void GSM::SendReceive( char* s )
***  \brief   Class function
**************************************************************************/

void GSM::SendReceive( const char* fmt, ... )
{
	va_list args;
		
	va_start( args, fmt );
	
	vsnprintf( txbuffer, TX_BUFFER_SIZE, fmt, args );
	
	txbuffer[ TX_BUFFER_SIZE - 1 ] = 0;			//	ensure termination
	
	int tosend = strlen( txbuffer );
			
	app.DebOut( "GSM(out %d): %s\n", tosend, txbuffer );
	
 	HAL_UART_AbortTransmit( app.gsm_uart );
 	HAL_UART_AbortReceive( app.gsm_uart );
				
 	memset( rxbuffer, 0, RX_BUFFER_SIZE );
  
	HAL_UART_Transmit_DMA( app.gsm_uart, (uint8_t*) txbuffer, tosend );
}	
	

#define MAX_SMS_LENGTH	160

/*************************************************************************/
/**  \fn      void GSM::CreateText( char* buff, const char* desc, const char* event )
***  \brief   Class function
**************************************************************************/

void GSM::CreateText( char* buff, int actions, int group, int count )
{
	assert( buff != NULL );
	
	*buff = 0;

	STRNCAT( buff, GSM_TITLE"\n", MAX_SMS_LENGTH );
	STRNCAT( buff, GSM_TITLE2"\n\n", MAX_SMS_LENGTH );
	
	STRNCAT( buff, "Panel: ", MAX_SMS_LENGTH );
	
	if ( app.panel != NULL )
	{
		STRNCAT( buff, app.panel->name, MAX_SMS_LENGTH );
	}
	STRNCAT( buff, "\n", MAX_SMS_LENGTH );
	
	int numactions = 0;
		
	if ( group == 0 )
	{
		for( int action = 0; action < SITE_NUM_OUTPUT_ACTIONS; action++ )
		{
			if ( actions & ( 1 << action ) )
			{
				STRNCAT( buff, "Event: ", MAX_SMS_LENGTH );
				STRNCAT( buff, GetActionString( action ), MAX_SMS_LENGTH );
				STRNCAT( buff, "\n", MAX_SMS_LENGTH);
		
				numactions++;
			}
			if ( numactions == 2 ) break;	// cant fit any more!
		}	
	}
	else
	{
		switch( count )
		{	
			
			case 1: STRNCAT( buff, GetGroup( group ), MAX_SMS_LENGTH );
							STRNCAT( buff, " reported.", MAX_SMS_LENGTH );
							break;
			case 2: STRNCAT( buff, "Two or more ", MAX_SMS_LENGTH );
							STRNCAT( buff, GetGroup( group ), MAX_SMS_LENGTH );
							STRNCAT( buff, "s reported.", MAX_SMS_LENGTH );
							break;
			
		}
	}		

	strcat( buff, "\x1A" );				
}

/*************************************************************************/
/**  \fn      void GSM::CreateDeviceText( char* buff  )
***  \brief   Class function
**************************************************************************/

void GSM::CreateDeviceClearText( char* buff )
{
	if ( buff != NULL )
	{	
		sprintf( buff, "Mesh restarted.\nTotal units on mesh: 0.\n");
		
		strcat( buff, "\x1A" );	
	}
}



#if 0
extern char PrivateKey;
extern char RootCertificate;
extern char ClientCertificate;
extern char CertificateEnd;

/*************************************************************************/
/**  \fn      void GSM::LoadCertificate
***  \brief   Class function
**************************************************************************/

int GSM::LoadCertificates(  )
{
	HAL_UART_AbortTransmit( app.gsm_uart );
 	HAL_UART_AbortReceive( app.gsm_uart );
	
	char cmd[ 100 ];
	char response[ 100 ];
	 
	sprintf( cmd, "AT+USECMNG=0,1,\"MattsDeviceClient\",%d\r\n", &CertificateEnd - &ClientCertificate );
		
	HAL_UART_Transmit( app.gsm_uart, (uint8_t*)cmd, strlen( cmd ), 5000 );	
	HAL_UART_Receive( app.gsm_uart, (uint8_t*)response, 1, 5000 );
	
	if ( response[ 0] == '>' )
	{
		HAL_UART_Transmit( app.gsm_uart, (uint8_t*)&ClientCertificate, &CertificateEnd - &ClientCertificate, 500 );
	}	
	else
	{
		return 0;
	}
	HAL_UART_Receive( app.gsm_uart, (uint8_t*)response, 3, 50 );

	if ( strncmp( "OK", response, 2 ) ) return 0;
	
	
	sprintf( cmd, "AT+USECMNG=0,0,\"MattsDeviceRoot\",%d\r\n", &ClientCertificate - &RootCertificate );
		
	HAL_UART_Transmit( app.gsm_uart, (uint8_t*)cmd, strlen( cmd ), 5000 );	
	HAL_UART_Receive( app.gsm_uart, (uint8_t*)response, 1, 5000 );
	
	if ( response[ 0] == '>' )
	{
		HAL_UART_Transmit( app.gsm_uart, (uint8_t*)&RootCertificate, &ClientCertificate - &RootCertificate, 5000 );
	}	

	HAL_UART_Receive( app.gsm_uart, (uint8_t*)response, 3, 50 );

	if ( strncmp( "OK", response, 2 ) ) return 0;
	
	
	sprintf( cmd, "AT+USECMNG=0,2,\"MattsDeviceKey\",%d\r\n", &RootCertificate - &PrivateKey );
		
	HAL_UART_Transmit( app.gsm_uart, (uint8_t*)cmd, strlen( cmd ), 5000 );	
	HAL_UART_Receive( app.gsm_uart, (uint8_t*)response, 1, 50 );
	
	if ( response[ 0] == '>' )
	{
		HAL_UART_Transmit( app.gsm_uart, (uint8_t*)&RootCertificate, &ClientCertificate - &RootCertificate, 5000 );
	}	

	HAL_UART_Receive( app.gsm_uart, (uint8_t*)response, 3, 50 );

	if ( strncmp( "OK", response, 2 ) ) return 0;	
	
	
	return 1;
}
#endif


/*************************************************************************/
/**  \fn      void GSM::CreateDeviceText( char* buff, const char* desc, const char* event )
***  \brief   Class function
**************************************************************************/

void GSM::CreateDeviceText( char* buff, int actions, int unit, int parent, int total )
{
	if ( buff != NULL )
	{	
		*buff = 0;

		STRNCAT( buff, GSM_TITLE"\n", MAX_SMS_LENGTH );
		STRNCAT( buff, GSM_TITLE2"\n\n", MAX_SMS_LENGTH );
		
		STRNCAT( buff, "Panel: ", MAX_SMS_LENGTH );
		
		if ( app.panel != NULL )
		{
			STRNCAT( buff, app.panel->name, MAX_SMS_LENGTH );
		}
		STRNCAT( buff, "\n\n", MAX_SMS_LENGTH );
		
		char content[ 64];
		
		if ( actions & ACTION_BIT_DEVICE_CONNECT )
		{
			snprintf( content, 64, "Unit %d connected to parent %d.\n", unit, parent );
		}
		else
		{		
			snprintf( content, 64, "Unit %d disconnected.\n", unit );
		}
		
		STRNCAT( buff, content, MAX_SMS_LENGTH );	
		
		snprintf( content, 64, "Total units on mesh: %d.\n", total );
		STRNCAT( buff, content, MAX_SMS_LENGTH );	
		
		snprintf( content, 64, "%s\n", strnow( ) );
		STRNCAT( buff, content, MAX_SMS_LENGTH );	
			 
		strcat( buff, "\x1A" );				
	}
}


/*************************************************************************/
/**  \fn      int GSM::ResultOK( )
***  \brief   Class function
**************************************************************************/

int GSM::ResultOK( )
{		
	char* ok = strstr( rxbuffer, "OK" );
	
	if ( ok != NULL )
	{	
		return true;
	}
	return false;
}


/*************************************************************************/
/**  \fn      int GSM::ResultOK( )
***  \brief   Class function
**************************************************************************/

int GSM::NotOK( )
{		
	char* notok = strstr( rxbuffer, "ERROR" );
	
	if ( notok != NULL )
	{	
		return true;
	}
	return false;
} 


void ShowBytes( char* bytes, int n )
{
	 if ( n > 10 )
	 {
		 n = 10;
	 }
	 
	 for( int i = 0; i < n; i++ )
	 {
		 if ( i != 0 ) app.DebOut( ", " );
		 app.DebOut( "%02X", bytes[ i] );
	 }
	 app.DebOut( "\n" );
 }


/*************************************************************************/
/**  \fn      void GSM::ProcessRx( )
***  \brief   Class function
**************************************************************************/

void GSM::ProcessRx( int bytes )
{
	if ( state != GSM_IDLE ) app.DebOut( "GSM(in %d): %s\n", app.gsm_uart->pRxBuffPtr - (uint8_t*)rxbuffer, rxbuffer );
	
	switch ( state )
	{		
		case GSM_RESET_COMS :
		{
			if ( 	ResultOK() )
			{				
				state = GSM_ECHO_OFF;
			}
			break;
		}
		
		
		case GSM_ECHO_OFF :
		{
			if ( ResultOK() ) state = GSM_SET_MNO_PROFILE;  
			break;
		}
		
		case GSM_SET_MNO_PROFILE :
		{
			state = GSM_GET_MNO_PROFILE;
				 	 
			break;
		}
		
		case GSM_GET_MNO_PROFILE :
		{
			//if ( strstr( rxbuffer, "UMNOPROF: 2" ) )
				state = GSM_SELECT_2G_4G;
		 
			break;
		}
 
		case GSM_SELECT_2G_4G :
		{
			if ( ResultOK( ) )///	{ state = GSM_RESET_COMS; next_state = GSM_ADD_ACCESS_POINT; };		 
			{ 
				state = GSM_POWER_UP; next_state = GSM_ADD_ACCESS_POINT;
			}
			else
			{
				state = GSM_RESET_COMS;
			}
			break;
		}
	
		
		case GSM_ADD_ACCESS_POINT:
		{
			if ( ResultOK() )
			{
				// if password not empty..
				if ( *pwd != 0 )
				{
					state = GSM_SET_PASSWORD;
				}
				else
				{
					state = GSM_SEARCH_NETWORK;
				}
			};
			break;
		}
		
		case GSM_SET_PASSWORD :
		{
			state = GSM_SEARCH_NETWORK;	// nopt always supported
			break;
		}
						
		
		case GSM_SEARCH_NETWORK :
		{
			if ( ResultOK() ) state = GSM_CHECK_ATTATCHMENT;  
			break;
		}
		
/*		
		case GSM_SET_REGISTRATION :
		{
			if ( ResultOK() )  state = GSM_CHECK_ATTATCHMENT; 
			break;
		}
*/
		
		case GSM_CHECK_ATTATCHMENT :
		{
			char* cgatt = strstr( rxbuffer, "CGATT:" );
			
			network_attached = 0;
			
			if ( cgatt != NULL )
			{
				int got = sscanf( cgatt, "CGATT: %d", &network_attached );
			
				if ( ( got == 1 ) && network_attached ) 
				{
					 state = GSM_CHECK_DOMAIN;
				}
				 
			}
			break;
		}	
		
		case GSM_SET_DOMAIN :
		{
			if ( ResultOK() )	{ state = GSM_CHECK_DOMAIN; }
			break;
		}
		
		case GSM_CHECK_DOMAIN :
		{
			char* dom = strstr( rxbuffer, "USVCDOMAIN:" );
			
			if ( dom != NULL )
			{
				int network_domain = 0;
				int got = sscanf( dom, "USVCDOMAIN: %d\r", &network_domain );
				
				if ( network_domain == 2 && got == 1 )
				{
					state = GSM_CHECK_REGISTRATION;
				}	
				else
				{
						state = GSM_SET_DOMAIN;
				}
				
			}
			else 
			{
				state = GSM_SET_DOMAIN;
			}
			break;
		}
		
		
		case GSM_CHECK_REGISTRATION :
		{
			char* err;
			static char ebuf[ 40];
			static bool inerr = false;
			
			if ( err = strstr( rxbuffer, "ERROR" ) )
			{
				if ( !inerr )
				{
					err += 7;
					char* crlf = strchr( err, '\r' );
					
					if ( crlf != NULL )
					{
						*crlf = 0;
						snprintf( ebuf, 40, "GSM Error: '%s'", err );
						Fault::AddFault( ebuf, 255, -1, 0 );
						inerr = true;
					
						if ( strstr( rxbuffer, "SIM" ) )
						{
							network_state = 11; // sim error
						}
					}
				}
				break;
			}
			else
			{
				if ( inerr )
				{
					inerr = false;
					Fault::RemoveFault( ebuf, 255, -1, 255 );
				}
			}
			
			char* cgeg;

			
			cgeg = strstr( rxbuffer, "CREG:" ) + 6;
			
			 
			if ( cgeg != NULL )
			{
				network_state = 0;
				int got = sscanf( cgeg, "%d,%d", &network_num, &network_state );
				
				if ( got == 2 )	
				{
					if (network_state >= 5 || network_state == 1 )
					{
						state = GSM_CHECK_NETWORK;
					}
					 
				}
		
			}
			break;
		}

		case GSM_CHECK_NETWORK :
		{
			char* cops = strstr( rxbuffer, "COPS:" );
		
			if ( cops != NULL )
			{
				int mode, format, act;
				
				if (sscanf( cops, "COPS: %d,%d,\"%s\",%d", &mode, &format, network_name, &act ) >= 3 )
				{
					if ( format == 2 )
					{
						strcpy( network_name, "<Default>"  );
					}
					else
					{
						// remove quote if present
						char* q = strchr( network_name, '"' );
						if ( q != NULL ) *q = 0;
						
					}
				}
				else
				{
					strcpy( network_name, "<Unknown>" );
				}
			}
			
			if ( MQTTSupport::enabled )
			{
				state = GSM_SET_ACCESS_POINT;
			}
			else
			{
				state = GSM_IDLE;
			}
			
			break;
		}
		
	
		
		 
		case GSM_SET_ACCESS_POINT:
		{
			if ( ResultOK() ) state = GSM_GET_IP;
		 
			break;
		}
		 
				
		case GSM_GET_IP:
		{
			int ip0, ip1, ip2, ip3;
			
			char* ipaddr = strstr( rxbuffer, "+CGPADDR:" );
			
			if ( ipaddr != NULL )
			if ( sscanf( ipaddr, "+CGPADDR: 1,%d.%d.%d.%d\r\n", &ip0, &ip1, &ip2, &ip3 ) == 4 )
			{
				ip[0] = ip0;
				ip[1] = ip1;
				ip[2] = ip2;
				ip[3] = ip3;
				
				if ( ( ip0 + ip1 + ip2 + ip3 ) != 0 )
				{
					state = GSM_RESET_PROFILE;
				}
				else
				{
				 	state = GSM_CHECK_ATTATCHMENT;
				}
			}
			
			break;
		}
		
		
		case GSM_RESET_PROFILE:
		{
			if ( ResultOK( ) )
			{
				state = GSM_IDLE;
			}
			break;
		}
		
		case GSM_LOOKUP_IP:
		{
			int d0, d1, d2, d3;
			char* ip = strstr( rxbuffer, "UDNSRN:" );
			static char domain_err[ 127 ];
			static char dns_err = false;
			
			if ( ip != NULL && socket != NULL )
			{
				if ( sscanf( ip, "UDNSRN: \"%d.%d.%d.%d\"", &d0, &d1, &d2, &d3 ) == 4 )
				{
					socket->dest_ip[0] = d0;
					socket->dest_ip[1] = d1;
					socket->dest_ip[2] = d2;
					socket->dest_ip[3] = d3;
					
					state = GSM_CREATE_SOCKET; 
					
					if ( dns_err )
					{
						Fault::RemoveFault( domain_err );
						dns_err = false;
					}
						
					break;
				}
			}
		 	
			snprintf( domain_err, 127, "DNS looked up failed. %s", socket->dest_domain );
			
			Fault::AddFault( domain_err );
			dns_err = true;
			
			state = GSM_SELECT_2G_4G;
			
			break;
		}
		
	
		
		case GSM_TEST_BAND:
		{
			state = GSM_DO_NOTHING;			
		}
		
		case GSM_DO_NOTHING:
		{
			//
		}
		
		
		case GSM_POWER_UP :
		{
			if ( ResultOK() )
			{
				state = next_state;
			}				
			break;
		}	 
		
		case GSM_SIGNAL_QUALITY :
		{
			signal_strength = -1;
			char* csq = strstr( rxbuffer, "+CSQ:" );
			if ( csq != NULL )
			{
				sscanf( csq, "+CSQ: %d", &signal_strength );	
			}
			state = GSM_IDLE;
			break;
		}
				
		//  DATA
			
	 
		
		// socksts
		
		case GSM_CREATE_SOCKET:
		{
			char* sock = strstr( rxbuffer, "+USOCR:" );
			
			if ( sock != NULL )
			if ( sscanf( sock, "+USOCR: %d", &socket->num ) == 1 )
			{
				if ( socket->stay_connected )
				{
					state = GSM_ASSOCIATE_PROFILE; //GSM_SOCKET_OPTION;
				}
				else
				{
					state = GSM_ASSOCIATE_PROFILE;
				}	
			}
			break;
		}
		
		case GSM_SOCKET_OPTION:
		{
			if ( ResultOK( ) )
			{
				state = GSM_ASSOCIATE_PROFILE;
			}
		}
		
		case GSM_ASSOCIATE_PROFILE:
		{
			if ( ResultOK( ) )
			{
				state = GSM_OPEN_SOCKET;
			}
			else
			{
				state = GSM_CLOSE_SOCKET;
			}
			break;
		}			
		
		static int write_trys = 0;
		
		case GSM_OPEN_SOCKET:
		{
			if ( ResultOK() )
			{
				socket->is_connected = true;
				write_trys = 0;
			 
				if ( socket->stay_connected )
				{
					socket->Reset( );
					
					out_header_len = 0;
					out_data_len = 0;
					out_data = NULL;
					out_header = NULL;
					
					socket->has_data = false;
					state = GSM_IDLE;
				}
				else
				{
					state = GSM_WRITE_TO_SOCKET;
				
					if ( out_header_len > 0 )
					{
						write = GSM_WRITE_HEADER; 
					}
					else if ( out_data_len > 0 )
					{
						write = GSM_WRITE_BODY; 
					}
					else 
					{
						state = GSM_CLOSE_SOCKET;
					}
				}
			}
			else
			{		 
				state = GSM_CLOSE_SOCKET;
			}
			break;
		}
		
		case GSM_WRITE_TO_SOCKET:
		{
			if ( strchr( (const char*) rxbuffer, '@' ) )
			{
				state = write;
			}
			else
			{
				if ( socket->stay_connected )
				{
				//	if ( write_trys++ > 5 )
					{						
						state = GSM_OPEN_SOCKET;
					}
				}
				else
				{
					state = GSM_CLOSE_SOCKET;
				}
			}
			break;
		}
		
		case GSM_WRITE_HEADER:
		{
			if ( ResultOK() )
			{
				write = GSM_WRITE_BODY;
				state = GSM_WRITE_TO_SOCKET;
			}
			else
			{
				if ( socket->stay_connected )
				{
					state =  GSM_OPEN_SOCKET;
				}
				else
				{
					state = GSM_CLOSE_SOCKET;
				}
			}
			
			break;
		}
		
		case GSM_WRITE_BODY :
		{
			if ( ResultOK() )
			{
				data_in_length = 768;
			 	state = GSM_READ_FROM_SOCKET;
				packet_num = 0;
				socket->has_data = false;
				socket->Sent( );
			}
			else 
			{
				if ( socket->stay_connected )
				{
					state =  GSM_OPEN_SOCKET; 
				}
				else
				{
					state = GSM_CLOSE_SOCKET;
				}
			}
			
			break;
		}
		
		case GSM_READ_FROM_SOCKET :
		{
			char* p = strstr( rxbuffer, "+USORD:" );
			if ( p != NULL && socket != NULL )
			{	
				int num, data = -1;
				static bool more = true;
				
				int len = -1;
				int args = sscanf( p, "+USORD: %d,%d,%n", &num, &len, &data );
				
			  if ( len > 0 && data != -1 && ( len + data ) < bytes )
				{
			//		app.DebOut( "Received " );
			//		ShowBytes( p + data + 1, len  );
					
					// if socketnum= num
					more = ( len == data_in_length );
					socket->Receive( p + data + 1, len, packet_num++, more );
				 
					if ( !more )
					{
						if ( !socket->stay_connected ) 
						{
							state = GSM_CLOSE_SOCKET;
						}
						else
						{
							state = GSM_IDLE;
						}
						more = true;
					}
				}	
				else
				{	 
					// if expecting more
					if ( more )
					{
						socket->Receive( NULL, 0, 0, false ); 		// end it ?
					}
					if ( !socket->stay_connected ) 
					{
						state = GSM_CLOSE_SOCKET;
					}
					else
					{
						state = GSM_IDLE;
					}		
					more = true;
				}
			}
			else
			{
				socket->Receive( NULL, 0, 0, false ); 
				
				if ( socket->stay_connected )
				{
					state =  GSM_OPEN_SOCKET;; 
				}
				else
				{
					state = GSM_CLOSE_SOCKET;
				}
			}
			break;
		}
		
		
		case GSM_SOCKET_STATE:
		{
			char* p = strstr( rxbuffer, "+USOCTL" );
			
			if ( p != NULL && socket != NULL )
			{	
				int socknum, pam, status;
				 
				int args = sscanf( p, "+USOCTL: %d,%d,%d", &socknum, &pam, &status );
			  
				if ( args == 3 && socknum == socket->num && pam == 10 )
				{
					switch( status )
					{
						case TCP_CLOSED:			app.DebOut( "sock closed\n" );  state = GSM_CLOSE_SOCKET; break;
						case TCP_LISTENING:		app.DebOut( "sock listng\n" ); break;
						case TCP_SYNC_SENT:		app.DebOut( "sock sync sent\n" ); break;
						case TCP_SYNC_RECEIVED: app.DebOut( "sock sync rcv\n" ); break;
						case TCP_ESTABLISHED:	app.DebOut( "sock estb\n" ); break;
						case TCP_FIN_WAITING1:	app.DebOut( "sock wait1\n" ); break;
						case TCP_FIN_WAITING2:	app.DebOut( "sock waqit2\n" ); break;
						case TCP_CLOSE_WAIT:		app.DebOut( "sock close wait\n" ); break;
						case TCP_CLOSING:			app.DebOut( "sock closing\n" ); break;
						case TCP_LAST_ACK:		app.DebOut( "sock last ack\n" ); break;
						case TCP_TIME_WAIT:		app.DebOut( "sock time wait\n" ); break;
					}				 
				}
			}
			state = GSM_IDLE;
			break;
		}
		
		case GSM_SOCKET_ERROR:
		{
			char* p = strstr( rxbuffer, "+USOER" );
			
			if ( p != NULL && socket != NULL )
			{	
				int err;
				 
				int args = sscanf( p, "+USOER: %d", &err );
			  
				if ( args == 1 )
				{
					app.DebOut( "Error:%d\n", err ); break; 
				}
			}
			else
			{
				state = GSM_CLOSE_SOCKET;
			}
		 
			break;
		}
		
		case GSM_CLOSE_SOCKET:
		{
			socket->is_connected = false;
			state = GSM_IDLE;  
			
			break;
		}
		 
		
		// MQTT
		 
		case MQTT_SET_CLIENT_ID:			  // AT+UMQTT=0,"id"
		{
			if ( ResultOK( ) )
			{
				state = MQTT_SET_PORT;
			}
			break;
		}
		
		case MQTT_SET_PORT:					  // AT+UMQTT=1,x
		{
			if ( ResultOK( ) )
			{
				state = MQTT_SET_SERVER;
			}
			break;
		}
		
		case MQTT_SET_SERVER:				   // UMQTT=2,"192.168.1.0",8883
		{
			if ( ResultOK( ) )
			{
				state = MQTT_SET_IP;
			}
			break;
		}
		
		case MQTT_SET_IP:				   // UMQTT=3,"192.168.1.0",8883
		{
			if ( ResultOK( ) )
			{
				state = MQTT_SET_USER_PASSWORD;
			}
			break;
		}
		
		case MQTT_SET_USER_PASSWORD:
		{
			if ( ResultOK( ) )
			{
				state = MQTT_LOGON;
			}
			break;
		}
		
		case MQTT_LOGON:
		{
			char* p = strstr( rxbuffer, "+UMQTTC:" );
			int a, b;
			if ( p != NULL )
			if ( sscanf( p, "+UMQTTC: %d,%d", &a, &b ) == 2 )
			{
				 
				state = GSM_IDLE;
				socket->is_connected = true;
			}
		}
		
		case MQTT_PUBLISH:				   // AT+UMQTTC=2,0,0,0,"sensor/","msg"
		{
			if ( ResultOK() )
			{				
				state = GSM_IDLE;
			}	
			break;
		}
		
		case MQTT_SUBSCRIBE:			// AT+UMQTTC=4,0,"sensor/heat/#"
		{
			if ( ResultOK() )
			{				
				state = GSM_IDLE;
			}	
			break;
		}
		 

		// MISC
		
		
		case GSM_POWER_DOWN:
		{
			state = GSM_IDLE;
			powered_down = true; 
			break;
		}
		
		// SMS
		
		case GSM_ASCI_TEXT_MODE :
		{
			if ( ResultOK( ) ) state = GSM_CHECK_TEXT;
			break;
		}
		
		case GSM_CHECK_TEXT :
		{
			if ( ResultOK() ) state = GSM_SEND_NUMBER;
			break;
		}	
		case GSM_SEND_NUMBER :
		{
	 		if ( strchr( rxbuffer, '>' ) )
			{
				state = GSM_SEND_TEXT;
		 
				senttext = false;
			}			
			break;
		}
		case GSM_SEND_TEXT:
		{	
			state = GSM_IDLE;	
			textqueue.Pop( );
			 
			break;
		}
		
		case GSM_IDLE:
		{		
			break;
		}		 
	}
	
}


/*************************************************************************/
/**  \fn      void GSM::ProcessTx( )
***  \brief   Class function
**************************************************************************/

void GSM::ProcessTx( )
{
	switch ( state )
	{
		
		// Startup
		
		case GSM_ECHO_OFF :
		{
			SendReceive( "ATE0\r\n" );			// Echo off
			break;
		}
	
		case GSM_SET_MNO_PROFILE :
		{
			SendReceive( "AT+UMNOPROF=100\r\n" );			
			break;
		}
		
		case GSM_GET_MNO_PROFILE :
		{
			SendReceive( "AT+UMNOPROF?\r\n" );			 
			break;
		}

		case GSM_SELECT_2G_4G :
		{
			char buff[ 32];
			sprintf( buff, "AT+URAT=7,9\r\n" );
			SendReceive( buff );			 
			break;
		}
		
		case GSM_CHECK_REGISTRATION :
		{
			SendReceive( "AT+CREG?\r\n" );		 
			break;
		}
		
		case GSM_SET_DOMAIN :
		{
			SendReceive( "AT+USVCDOMAIN=2\r\n" );
			break;
		}
		
		case GSM_CHECK_DOMAIN :
		{
			SendReceive( "AT+USVCDOMAIN?\r\n" );
			break;
		}
		
		case GSM_CHECK_NETWORK :
		{
			SendReceive( "AT+COPS?\r\n");	
			break;
		}
	
		case GSM_SEARCH_NETWORK:		
		{
			SendReceive( "AT+COPS=0\r\n");	
			break;
		}
/*		
		case GSM_SET_REGISTRATION:
		{
			SendReceive( "AT+CREG=1\r\n");	
			break;
		}
*/		
		case GSM_CHECK_NETWORKS:
		{
			SendReceive( "AT+COPS=?\r\n");	
			break;
		}
		
		case GSM_CHECK_ATTATCHMENT :
		{
			SendReceive( "AT+CGATT?\r\n");	
			break;
		}
		 
 		case GSM_ADD_ACCESS_POINT :
 		{
			if ( *apn != 0 )
			{
				SendReceive( "AT+CGDCONT=1,\"IP\",\"%s\"\r\n", apn ); 
				//SendReceive( "AT+CGDCONT=1,\"IP\",\"goto.virginmobile.uk\",\"0.0.0.0\",0,0,0,0\r\n" );
			}
 			break;
 		}	
				
		case GSM_SET_PASSWORD:
		{
			if ( *pwd != 0 )
			{
				SendReceive( "AT+UPSD=0,3,\"%s\"\r\n", pwd ); 
				
			}
			break;
		}
		
		case GSM_SET_ACCESS_POINT:
		{
			SendReceive( "AT+CGACT=1,1\r\n" );
			break;
		}
					
		case GSM_GET_IP:
		{
			SendReceive( "AT+CGPADDR=1\r\n" );
			break;
		}
		
		case GSM_CREATE_SOCKET:
		{
			SendReceive( "AT+USOCR=%d\r\n", socket->protocol );
			break;
		}
		
		case GSM_RESET_PROFILE:
		{
			SendReceive( "AT+USECPRF=0\r\n" );
			break;
		}
		
		case GSM_ASSOCIATE_PROFILE:
		{
			SendReceive( "AT+USOSEC=%d,1\r\n", socket->num );
			break;
		}
		
		case GSM_LOOKUP_IP:
		{
			SendReceive( "AT+UDNSRN=0,\"%s\"\r\n", socket->dest_domain );
			break;
		}
		
		case GSM_OPEN_SOCKET:
		{
			SendReceive( "AT+USOCO=%d,\"%d.%d.%d.%d\",%d\r\n", socket->num, socket->dest_ip[0], socket->dest_ip[ 1], socket->dest_ip[ 2], socket->dest_ip[ 3], socket->dest_port );
			break;
		}
		
		case GSM_SOCKET_OPTION:
		{
			SendReceive( "AT+USOSO=%d,65535,8,1\r\n", socket->num );
			break;
		}
		
		case GSM_WRITE_TO_SOCKET:
		{
			if ( write == GSM_WRITE_HEADER )
			{
				SendReceive( "AT+USOWR=%d,%d\r\n", socket->num, out_header_len );
			}
			else
			{
				SendReceive( "AT+USOWR=%d,%d\r\n", socket->num, out_data_len );
			}
				
			break;
		}
		
		case GSM_WRITE_HEADER:
		{
			if ( out_header != NULL ) SendReceiveBig( out_header, out_header_len );
			break;
		}
		
		case GSM_WRITE_BODY:
		{
		//	app.DebOut( "Sending " );
		//	ShowBytes( out_data, out_data_len );
			if ( out_data != NULL ) SendReceiveBig( out_data, out_data_len );
			break;
		}
		
		case GSM_READ_FROM_SOCKET:
		{
			SendReceive( "AT+USORD=%d,%d\r\n", socket->num, data_in_length );
			break;
		}
			
		case GSM_SOCKET_STATE:
		{
			SendReceive( "AT+USOCTL=%d,10\r\n", socket->num  );
			break;
		}
		
		case GSM_CLOSE_SOCKET:
		{
			SendReceive( "AT+USOCL=%d\r\n", socket->num );
			break;
		}
		
		case GSM_SOCKET_ERROR:
		{
			SendReceive( "AT+USOER\r\n", socket->num );
			break;
		}
		
		case GSM_DESTROY_SOCKET:
		{
			SendReceive( "AT+USODR\r\n", socket->num );
			break;
		}
		
		// Misc

		case GSM_SIGNAL_QUALITY :
		{
			SendReceive( "AT+CSQ\r\n" );	
			break;
		}
		case GSM_POWER_DOWN :
		{
			SendReceive( "AT+CFUN=0\r\n");
			break;
		}
		case GSM_POWER_UP :
		{
			SendReceive( "AT+CFUN=1\r\n");	
			break;
		}
		
		case GSM_RESET_COMS :
		{
			SendReceive( "AT+CFUN=15\r\n" );			 
			break;
		}
		
		// SMS
		
		case GSM_ASCI_TEXT_MODE :
		{
			SendReceive( "AT+CMGF=1\r\n" );	// ASCI Text mode
			break;
		}

		case GSM_CHECK_TEXT :
		{
			SendReceive( "AT+CMGS=?\r\n" );
			break;
		}
		case GSM_SEND_NUMBER :
		{
			char buff[ 32];
			
			GSMText* text = textqueue.Peek( );
			
			if ( text != NULL )
			{			
				snprintf( buff, 32, "AT+CMGS=\"%s\"\r\n", text->contact->number );
				SendReceive( buff, 3 );
			}
			else
			{
				state = GSM_IDLE;
			}
			break;
		}
		case GSM_SEND_TEXT :
		{			
			GSMText* text = textqueue.Peek( );
			
			if ( text != NULL )
			{
				char msg[ 162 ];
				
				if ( text->actions & ACTION_BIT_DEVICE_CLEAR )
				{
					CreateDeviceClearText( msg );
				}
				else if ( text->actions & ( ACTION_BIT_DEVICE_CONNECT | ACTION_BIT_DEVICE_DISCONNECT ) )
				{
					CreateDeviceText( msg, text->actions & ( ACTION_BIT_DEVICE_CONNECT | ACTION_BIT_DEVICE_DISCONNECT ),
											text->actions & ( ACTION_BIT_DEVICE_CONNECT - 1 ), text->parent, text->faultgroup + ( text->count << 8 ));
				}
				else
				{
					CreateText( msg, text->actions, text->faultgroup, text->count );
				}
		
				SendReceive( msg );
				senttext = true;
				break;
			}
			else
			{
				const char* x = "\x1A";
				SendReceive( x );
			}
								
			state = GSM_IDLE;
			break;
		}
		
		// Idle
		
		case GSM_IDLE:
		{
			break;
		}
 

/*		
		// MQTT
		
		
		 
		case MQTT_SET_CLIENT_ID:			  // AT+UMQTT=0,"id"
		{
			SendReceive( "AT+UMQTT=0,\"%s\"\r\n", socket->dest_url );
			break;
		}
		
		case MQTT_SET_PORT:					  // AT+UMQTT=1,x
		{
			SendReceive( "AT+UMQTT=1,%d\r\n", socket->port );
			break;
		}
		
		case MQTT_SET_SERVER:				   // UMQTT=2,"192.168.1.0",8883
		{
			SendReceive( "AT+UMQTT=2,\"%s\",%d\r\n", socket->dest_domain, socket->dest_port );
			break;
		}
		
		case MQTT_SET_IP:				   // UMQTT=3,"192.168.1.0",8883
		{
			SendReceive( "AT+UMQTT=3,\"%d.%d.%d.%d\",%d\r\n",
					socket->dest_ip[0], socket->dest_ip[1], socket->dest_ip[2], socket->dest_ip[3], socket->dest_port );
			break;
		}
		
		case MQTT_SET_USER_PASSWORD:
		{
			char txbuff[ 600 ];
			
			char* p = txbuff + sprintf( txbuff, "AT+UMQTT=4,\"%s/%s/?api-version=0\",\"", socket->dest_domain, socket->dest_url );
			
		//	MakeSASToken( socket, 60 * 60 * 4, p );
			
			strncat( txbuff, "\"\r\n", 600 );  
			
	//		SendReceiveBig( txbuff );
			break;
		}
		
		case MQTT_LOGON:
		{
			SendReceive( "AT+UMQTTC=1\r\n" );
			break;
		}
		
		case MQTT_PUBLISH:				   // AT+UMQTTC=2,0,0,0,"sensor/","msg"
		{
			SendReceive( "AT+UMQTTC=2,0,0,0,\"devices/%s/%s/$.ct=application%2Fjson%3Bcharset%3Dutf-8\",\"%s\"\r\n", socket->dest_url, out_header, out_data );
			break;
		}
		
		case MQTT_SUBSCRIBE:			// AT+UMQTTC=4,0,"sensor/heat/#"
		{
			SendReceive( "AT+UMQTTC=4,0,\"devices/%s/messages/devicebound/#\"\r\n", socket->dest_url );
			break;
		}
		*/
	}
}

 

/*************************************************************************/
/**  \fn      void GSM::Connect( Socket* s )
***  \brief   Class function
**************************************************************************/

int GSM::Send( Socket* s, char* header, char* data, int len )	
{
	socket = s;
	out_header = header;
	out_data = data;
	out_data_len = len;
	out_header_len = strlen( header );
	s->has_data = true;
	
	return 0;
}
		

/*************************************************************************/
/**  \fn      int GSM::Send( Socket* s, char* data, int len )
***  \brief   Class function
**************************************************************************/

int GSM::Send( Socket* s, char* data, int len )	
{
	socket = s;
	out_header = NULL;
	out_data = data;
	out_data_len = len;
	out_header_len = 0;
	s->has_data = true;
	
	return 0;
}


/*************************************************************************/
/**  \fn      int GSM::Receive( Message* msg )
***  \brief   Class function
**************************************************************************/

int GSM::Receive( Message* msg )
{
	return MODULE_MSG_UNKNOWN;
}


/*************************************************************************/
/**  \fn      int GSM::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int GSM::Receive( Command* cmd )
{
	switch ( cmd->type )
	{
		case CMD_GSM_SIGNAL_QUALITY:
		{
			cmd->int0 = signal_strength;
			cmd->int1 = network_state;
			cmd->int2 = state;
			cmd->int3 = (int) network_name;
			
			return CMD_OK;
		}
		
		case CMD_SET_SMS :
		{
			enabled = cmd->short0;
			
			if ( !EEPROM::SetVar( SMS_Enabled, enabled ) )
			{
				if ( !EEPROM::SetVar( SMS_Enabled, enabled ) )
				{
					Fault::AddFault( FAULT_GSM_SMS_ENEBLED_GET );
					//Log::Msg(LOG_SYS, "04_GSM.cpp -- ADD");
				}
				else
				{
					Fault::RemoveFault( FAULT_GSM_SMS_ENEBLED_GET );
					//Log::Msg(LOG_SYS, "05_GSM.cpp -- REMOVE");
				}
			}
			else
			{
				Fault::RemoveFault( FAULT_GSM_SMS_ENEBLED_GET );
				//Log::Msg(LOG_SYS, "06_GSM.cpp -- REMOVE");
			}
			
			return CMD_OK;
		}
		
		case CMD_GET_SMS :
		{
			cmd->short0 = enabled; 
			
			return CMD_OK;
		}
		
		case CMD_GSM_SIGNAL_PERIOD:
		{
			signal_period = cmd->int0 * 500;
			return CMD_OK;
		}
		
		case CMD_GET_NUM_SMS_CONTACTS:
		{
			if ( app.site != NULL )
			{
				cmd->int0 = app.site->numContacts;
			}
			else
			{
				cmd->int0 = 0;
			}
			return CMD_ERR_NO_SITE;
		}
		
		case CMD_REMOVE_SMS_NUMBER:
		{
			bool found = false;
			
			if ( app.site != NULL )
			{	
				SMSContact sms[ SITE_MAX_CONTACTS ];
				memcpy( sms, app.site->contacts, sizeof( sms ) );
				
				short contacts = app.site->numContacts - 1;
				
				for( int n = 0; n < app.site->numContacts; n++ )
				{
					if ( n == cmd->int0 ) found = true;
					
					if ( found )
					{
						sms[ n ] = sms[ n + 1 ];
					}
				}
				if ( found )
				{
					if ( qspi->WriteToSite( sms, contacts ) == QSPI_RESULT_OK )
					{
						if ( qspi->WriteToSite(contacts, numContacts ) == QSPI_RESULT_OK )
						{
							settings->UpdateRevision( );
						}
					}
				}
			}
			
			if ( found ) return CMD_OK;
			
			return CMD_ERR_OUT_OF_RANGE;
		}
		
		case CMD_NEW_SMS_NUMBER:
		{
			// room for one more
			if ( app.site != NULL && app.site->numContacts < SITE_MAX_CONTACTS )
			{	
				uint8_t numcontacts = app.site->numContacts + 1;
			
				// Create contact
				SMSContact c;
				
				c.actions = 0;
				STRNCPY( c.name, (const char*) cmd->int0, SITE_MAX_STRING_LEN );
				STRNCPY( c.number, (const char*) cmd->int1, SITE_MAX_PHONE_DIGITS );
				
				if ( qspi->WriteToSite( c, contacts[ app.site->numContacts ] ) == QSPI_RESULT_OK )
				{
					if ( qspi->WriteToSite( numcontacts, numContacts ) == QSPI_RESULT_OK )
					{
						settings->UpdateRevision( );
						return CMD_OK;
					}
				}
				return CMD_ERR_DEVICE_BUSY;
			}
			return CMD_ERR_NO_SITE;
		}		
			
		case CMD_GET_SMS_NUMBER:
		{
			if ( app.site != NULL )
			{
				if ( cmd->int0 < app.site->numContacts )
				{
					SMSContact* sms = app.site->contacts + cmd->int0;
					
					cmd->int1 = (int) sms->name;
					cmd->int2 = (int) sms->number;
					cmd->int3 = (int) sms->actions;
					
					return CMD_OK;
				}
				return CMD_ERR_OUT_OF_RANGE;
			}
			return CMD_ERR_NO_SITE;
		}
		
		case CMD_SET_SMS_NUMBER:
		{
			if ( app.site != NULL )
			{
				if ( cmd->int0 < app.site->numContacts )
				{
					SMSContact sms = app.site->contacts[ cmd->int0 ];
					
					sms.actions = cmd->int1; 
					
					if ( qspi->WriteToSite( sms, contacts[ cmd->int0 ] ) == QSPI_RESULT_OK )
					{
						settings->UpdateRevision( );
				 	
						return CMD_OK;
					}
					else
					{
						return CMD_ERR_DEVICE_BUSY;
					}
				}
				return CMD_ERR_OUT_OF_RANGE;
			}
			return CMD_ERR_NO_SITE;
		}
		default:	return CMD_ERR_UNKNOWN_CMD;
	}			
}



/*************************************************************************/
/**  \fn      int GSM::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int GSM::Receive( Event e )
{
	switch ( e )
	{		 	
		case EVENT_CLEAR :
		{
		//	state = GSM_IDLE;
			textqueue.Clear( );
			ClearFaultCount( );
			break;
		}	
		
		case EVENT_DAY_TIME:
		{
			ClearFaultCount( );
			break;
		}	
	 
				
		default:
			return 0;	 
	}
	return 0;
}


/*************************************************************************/
/**  \fn      void GSM::ClearFaultCount( )
***  \brief   Class function
**************************************************************************/

void GSM::ClearFaultCount( )
{
	for( int g = 0; g < FAULTGROUP_SYSTEM; g++ )
	{
		faultcount[ g ] = 0;
	}
}
		

/*************************************************************************/
/**  \fn      int GSM::ProcessFault( int group )
***  \brief   Class function
**************************************************************************/

int GSM::ProcessFault( int group )
{		
	if ( app.panel != NULL && enabled )
	{
		int i = group - 1;
		
		if ( faultcount[ i ] < 2 ) faultcount[ i ]++;
		else return 0;		
		
		if ( app.site != NULL )
		{
			SMSContact* contacts = app.site->contacts;
			
			for( SMSContact* con = contacts; con < contacts + app.site->numContacts; con++ )
			{
				if ( con->actions & ACTION_BIT_FAULT )
				{
					GSMText gt( con, ACTION_BIT_FAULT, group, faultcount[ i ] );
					textqueue.Push( gt );
				}
			}
		}
	}
	return true;
}

	
/*************************************************************************/
/**  \fn      void GSM::MeshRestartNotification( )
***  \brief   Class function
**************************************************************************/
	
void GSM::MeshRestartNotification( )
{
	if ( app.panel != NULL && enabled )
	{	
		if ( app.site != NULL )
		{
			SMSContact* contacts = app.site->contacts;
				
			for( SMSContact* con = contacts; con < contacts + app.site->numContacts; con++ )
			{
				if ( ( con->actions & ACTION_BIT_MESH_FOR_SMS ) && !textqueue.Full( ) )
				{
					GSMText gt( con, ACTION_BIT_DEVICE_CLEAR );
					textqueue.Push( gt );
				}
			}
		}
	}
}


/*************************************************************************/
/**  \fn      void GSM::DeviceNotification( )
***  \brief   Class function
**************************************************************************/
	
void GSM::DeviceNotification( short unit, short parent, bool connected, int total )
{
	if ( app.panel != NULL && enabled )
	{	
		if ( app.site != NULL )
		{
			SMSContact* contacts = app.site->contacts;
				
			for( SMSContact* con = contacts; con < contacts + app.site->numContacts; con++ )
			{
				if ( ( con->actions & ACTION_BIT_MESH_FOR_SMS ) && !textqueue.Full( ) )
				{
					GSMText gt( con, unit, parent, connected, total );
					textqueue.Push( gt );
				}
			}
		}
	}
}


/*************************************************************************/
/**  \fn      void GSM::ProcessEvents( int e )
***  \brief   Class function
**************************************************************************/
		
void GSM::ProcessEvents( int e )
{	
	if ( app.panel != NULL && enabled )
	{	
		if ( app.site != NULL )
		{
			SMSContact* contacts = app.site->contacts;
			
			for( SMSContact* con = contacts; con < contacts + app.site->numContacts; con++ )
			{
				if ( ( con->actions & e ) && !textqueue.Full( ) )
				{
					GSMText gt( con, con->actions & e );
					textqueue.Push( gt );
				}
			}
		}
	}
}


//#define _KEY "   gvbkTutWU/goJQ3m1uV+OOV+HF4ZTw6KYesq+dLkYYI=" 

 



 extern "C" void USART3_IRQHandler( void )
 {
 	HAL_UART_IRQHandler( app.gsm_uart );
 }
 
