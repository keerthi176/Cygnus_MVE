/***************************************************************************
* File name: GSM.cpp
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


/* Defines
**************************************************************************/


/*************************************************************************/
/**  \fn      GSM::GSM( Application* app )
***  \brief   Constructor for class
**************************************************************************/

GSM::GSM( Application* app ) : Module( "GSM", 0, app, EVENT_FIRE | EVENT_FIRST_AID | EVENT_RESET | EVENT_DISABLE |
																		EVENT_TEST | EVENT_EVACUATE  | EVENT_FAULT | EVENT_SECURITY )
{
	AT( "+CFUN=0" );	// Minimise power
	AT( "E0" );			// Echo off
	
	Set( GSM_IDLE );
}


/*************************************************************************/
/**  \fn      void GSM::AT( const char* s )
***  \brief   Class function
**************************************************************************/

void GSM::AT( const char* s )
{
	char buff[ 64];
	
	sprintf( buff, "AT%s\n", s );
	
	while ( HAL_UART_GetState( app->gsm_uart ) != HAL_UART_STATE_READY ) ;

	CHECK( HAL_USART_Transmit_IT( app->gsm_uart, (uint8_t*) buff, strlen( buff ) ) ); 
}	
	

/*************************************************************************/
/**  \fn      void GSM::AT( const char* s, int rxbytes )
***  \brief   Class function
**************************************************************************/

void GSM::AT( const char* s, int rxbytes )
{
	CHECK( HAL_UART_AbortReceive_IT( app->gsm_uart ) );
	
	AT( s );
	
	CHECK( HAL_UART_Receive_IT( app->gsm_uart, (uint8_t *)rxbuff, rxbytes ) );
}


/*************************************************************************/
/**  \fn      void GSM::GenerateText( char* buff, RTC_DateTypeDef* date, RTC_TimeTypeDef* time, char* desc, char* event )
***  \brief   Class function
**************************************************************************/

void GSM::GenerateText( char* buff, RTC_DateTypeDef* date, RTC_TimeTypeDef* time, char* desc, char* event )
{
	sprintf( buff, "%d/%d/%d  %d:%2d\n\n"  "%s\n\n" "%s\n", date->Date, date->Month, date->Year, time->Hours, time->Minutes, desc, event );
}


/*************************************************************************/
/**  \fn      void GSM::SendSMS( const char* _text, const char* _number )
***  \brief   Class function
**************************************************************************/

void GSM::SendSMS( const char* _text, const char* _number )
{
	text   = _text;
	number = _number;
	
	Set( GSM_POWER_RF );
}


/*************************************************************************/
/**  \fn      int GSM::ProcessRx( )
***  \brief   Class function
**************************************************************************/

int GSM::ProcessRx( )
{
	switch ( state )
	{
		case GSM_IDLE :
			break;
		
		case GSM_POWER_RF :
		{
			char result[12];
			
			_sscanf( rxbuffer, "\n\r\%s\n\r", result );
			
			if ( !strcmp( result, "OK" ) )
			{
				return Set( GSM_CHECK_NETWORK );
			}
		}
		case GSM_CHECK_NETWORK :
		{
			int num, stat;
			const char* result;
			
			_sscanf( rxbuffer, "\n\r+CREG:%d,%d\n\r\n\r%s\n\r", &num, &stat, result );
			
			if ( stat > 0 && !strcmp( result, "OK" ) )
			{
				return Set( GSM_CHECK_TEXT );
			}
			break;
		}	
		case GSM_CHECK_TEXT :
		{
			char result[ 12];
			
			_sscanf( rxbuffer, "\n\r\%s\n\r", result );
			
			if ( !strcmp( result, "OK" ) )
			{
				return Set( GSM_CHECK_NETWORK );
			}
			break;
		}	
		case GSM_SEND_NUMBER :
		{
			if ( *rxbuffer == '>' )
			{
				return Set( GSM_SEND_TEXT );
			}	
			break;
		}
		case GSM_SEND_TEXT:
		{
			char result[ 12];
			int num;
			
			_sscanf( rxbuffer, "\n\r\+CMGS:%d\n\r%s\n\r", &num, result );
			
			if ( !strcmp( result, "OK" ) )
			{
				return Set( GSM_IDLE );
			}
			break;
		}
	}
	return -1;   // fail
}


/*************************************************************************/
/**  \fn      int GSM::Set( GSMState _state )
***  \brief   Class function
**************************************************************************/

int GSM::Set( GSMState _state )
{
	mode  = GSM_TX;
	tries = 0;
	state = _state;
	
	return 0;
}
	
	
/*************************************************************************/
/**  \fn      void GSM::ProcessTx( )
***  \brief   Class function
**************************************************************************/

void GSM::ProcessTx( )
{
	switch ( state )
	{
		case GSM_IDLE :
			break;
		
		case GSM_POWER_RF :
		{
			AT( "+CFUN=1", 6 );	
			break;
		}
		case GSM_CHECK_NETWORK :
		{
			AT( "+CREG?", 20 );	
			break;
		}
		case GSM_CHECK_TEXT :
		{
			AT( "+CMSGS=?", 9 );
			break;
		}
		case GSM_SEND_NUMBER :
		{
			char buff[ 32];
			sprintf( buff, "+CMGS=\"%s\"", number );
			AT( buff, 1 );
			break;
		}
		case GSM_SEND_TEXT :
		{
			// change terminator for Crtl-Z
			char* term = strlen( text );
			*term = 0x1A;
			CHECK( HAL_USART_Transmit_IT( app->gsm_uart, (uint8_t*) text, 1 + term - text ) ); 
			break;
		}
	}
}
	

/*************************************************************************/
/**  \fn      void GSM::Receive( Event e )
***  \brief   Class function
**************************************************************************/

void GSM::Receive( Event e )
{
	const char* event = NULL;
	
	switch ( e )
	{
		case EVENT_FIRE      : event = "Fire event";
										break;
		case EVENT_FIRST_AID : event = "First Aid event";
										break;
		case EVENT_RESET     : event = "Reset event";
										break;
		case EVENT_DISABLE   : event = "Disable event";
										break;
		case EVENT_TEST      : event = "Test event";
										break;
		case EVENT_EVACUATE  : event = "Evacuate event";
										break;
		case EVENT_FAULT     : event = "Fault event";
										break;
		case EVENT_SECURITY  : event = "Security event";
										break;
		
		case EVENT_GSM_UART_RX:  	ProcessRx( );
											break;
				
		case EVENT_SECOND :			ProcessTx( );
											break;
					
	}
}	
	
	
	