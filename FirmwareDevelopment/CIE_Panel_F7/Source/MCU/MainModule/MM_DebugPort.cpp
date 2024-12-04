/***************************************************************************
* File name: MM_DebugPort.cpp
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
* Alarm list management
*
**************************************************************************/
/* System Include Files
**************************************************************************/
#include <stdio.h>
#include "stm32f7xx_hal_dma.h"
#include "stm32f7xx_hal_uart.h"
#include <string.h>


/* Defines
**************************************************************************/
#define ITEMS(x) (sizeof(x)/sizeof(*x)) 
#define MAX_PARAMS ( 8 )


/* User Include Files
**************************************************************************/
#include "MM_DebugPort.h"
#include "MM_Application.h"
#include "MM_ATCommand.h"


/*************************************************************************/
/**  \fn      DebugPort::DebugPort( )
***  \brief   Constructor for class
**************************************************************************/

DebugPort::DebugPort( ) : Module( "DebugPort", 0 ), UartHandler( app.debug_uart )
{   
 
}


/*************************************************************************/
/**  \fn      int DebugPort::Receive( Command* )
***  \brief   Class function
**************************************************************************/
int DebugPort::Init( ) 
{
	atcmds = (ATCommands*) app.FindModule( "ATCommands" );
	// initiate first command
	return HAL_UART_Receive_IT( app.debug_uart, (uint8_t*) rxbuffer, RX_BUFF_LEN ) == HAL_OK;
}
 

/*************************************************************************/
/**  \fn      int DebugPort::Receive( Command* )
***  \brief   Class function
**************************************************************************/

int DebugPort::Receive( Command* )
{
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int DebugPort::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int DebugPort::Receive( Message* )
{
   return 0;
}


/*************************************************************************/
/**  \fn      void DebugPort::UartCommand( UART_HandleTypeDef* uart  )
***  \brief   Class function
**************************************************************************/

void DebugPort::UartCommand( UART_HandleTypeDef* uart )
{
	char reply[ 64];
	
	if ( *rxbuffer == 'G' )
	{
		char* lf = strchr( rxbuffer, 10 ); 
		
		char* x = strchr( rxbuffer, 0x1A );
		if ( x != NULL )
		{
			x[1] = 0;
		}
		
		if ( lf != NULL ) lf[ 0] = 0;
			
		char gsmbuffer[ 256 ];
		memset( gsmbuffer, 0, sizeof(gsmbuffer) );
		
		HAL_UART_AbortTransmit( app.gsm_uart );
		HAL_UART_AbortReceive( app.gsm_uart ); 
		
		HAL_UART_Transmit( app.debug_uart, (uint8_t*) "GSM:", 4, 200 );
		HAL_UART_Transmit( app.debug_uart, (uint8_t*) rxbuffer + 1, strlen(rxbuffer + 1), 200 );
				
		HAL_UART_Transmit( app.gsm_uart, (uint8_t*) rxbuffer + 1, strlen(rxbuffer + 1), 200 );
		 
		HAL_UART_Receive( app.gsm_uart, (uint8_t*) gsmbuffer, sizeof(gsmbuffer), 4500 );
		 
 		
		HAL_UART_Transmit( app.debug_uart, (uint8_t*) "GSM->", 5, 200 );
		HAL_UART_Transmit( app.debug_uart, (uint8_t*) gsmbuffer, strlen( gsmbuffer ), 200 );
		
		// start new command
		memset( rxbuffer, 0, sizeof(rxbuffer) );
		HAL_UART_AbortReceive( app.debug_uart );
		HAL_UART_Receive_IT(	app.debug_uart, (uint8_t*) rxbuffer, RX_BUFF_LEN );
	}	
	else if ( atcmds != NULL )
	{		
		atcmds->DecodeCommand( rxbuffer, reply );
		
		HAL_UART_Transmit( app.debug_uart, (uint8_t*) reply, strlen( reply ), 200 );

		// start new command
		HAL_UART_AbortReceive( app.debug_uart );
		HAL_UART_Receive_IT(	app.debug_uart, (uint8_t*) rxbuffer, RX_BUFF_LEN );
	}
}


/*************************************************************************/
/**  \fn      int DebugPort::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int DebugPort::Receive( Event event )
{
	return 0;
} 



 

