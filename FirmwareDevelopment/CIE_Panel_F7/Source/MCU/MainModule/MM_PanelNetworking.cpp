/***************************************************************************
* File name: MM_PanelNetworking.cpp
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
* Panel networking
*
**************************************************************************/


//////////////////////////// UNFINISHED ///////////////////////////////////


/* System Include Files
**************************************************************************/
#include <assert.h>
#include <string.h>


/* User Include Files
**************************************************************************/
#include "MM_PanelNetworking.h"



/*************************************************************************/
/**  \fn      PanelNetworking::PanelNetworking( )
***  \brief   Constructor for class
**************************************************************************/

PanelNetworking::PanelNetworking( ) : Module( "PanelNetworking", 0, EVENT_SECOND | EVENT_PANEL_UART_RX )
{
	HAL_UART_Receive_IT( app.panel_network_uart, rxbuff, 1 );
	
	
	
//	Send( (uint8_t*) "Testing, testing 1, 2, 3\n" ); 
	HAL_GPIO_WritePin( GPIOA, GPIO_PIN_6, GPIO_PIN_RESET );
/*	
	HAL_UART_Transmit( app.panel_network_uart, (uint8_t*) "WBA\n", 4, 50 ); 
	HAL_Delay(500);
	HAL_UART_Transmit( app.panel_network_uart,(uint8_t*) "WR\n", 4, 50 ); 
	HAL_Delay(500);
	HAL_UART_Transmit( app.panel_network_uart,(uint8_t*) "WE\n", 4, 50 ); 
	*/
}


/*************************************************************************/
/**  \fn      int PanelNetworking::Send( uint8_t* buff )
***  \brief   Class function
**************************************************************************/

int PanelNetworking::Send( uint8_t* buff )
{
	HAL_GPIO_WritePin( GPIOA, GPIO_PIN_6, GPIO_PIN_SET );
	
	HAL_UART_Transmit( app.panel_network_uart, buff, strlen( (const char*)buff) + 1, 50 ); 
	
	HAL_GPIO_WritePin( GPIOA, GPIO_PIN_6, GPIO_PIN_RESET );
	
	return 0;
}



/*************************************************************************/
/**  \fn      int PanelNetworking::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int PanelNetworking::Receive( Message* )
{
	return 0;
}


	
/*************************************************************************/
/**  \fn      int PanelNetworking::Receive( Command* )
***  \brief   Class function
**************************************************************************/

int PanelNetworking::Receive( Command* )
{
	return CMD_ERR_UNKNOWN_CMD;
}

/*************************************************************************/
/**  \fn      int PanelNetworking::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int PanelNetworking::Receive( Event event )
{
	switch ( event )
	{
		case EVENT_PANEL_UART_RX :
		{
		//	app.DebOut( "%c", *rxbuff );
			app.DebOut( "[%2X] '%c'", *rxbuff, *rxbuff );
	//	  Send( rxbuff );
			HAL_UART_Receive_IT( app.panel_network_uart, rxbuff, 1 );
				
			
			wait = (unsigned int)-1;
			break;
		}
		
		case EVENT_SECOND :
		{
		 //	if ( wait-- == 0 )
			{
		//	   Send( (uint8_t*) "Testing, testing 1, 2, 3\n" ); 
			//	HAL_RNG_GenerateRandomNumber( app.randgen, &wait );
				wait &= 5; 
			}
			break;
		}
	}
	return 0;
}
