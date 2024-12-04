
/***************************************************************************
* File name: DM_UartHandler.h
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
* First written on 18/05/19 by Matt Ault
*
* File History:
* Change History:
* Rev Date Description Author
* 1.0 18/05/18 Initial File
*
* Description:
* Uart Handling
*
************************************************************************/


#ifndef _DM_UART_HANDLER_
#define _DM_UART_HANDLER_


/* System Include Files
**************************************************************************/
#include <stm32f7xx_hal.h>

#include "DM_Handlers.h"



struct UartHandler
{
	UartHandler( UART_HandleTypeDef* uh );

	
	virtual void UartTxComplete( UART_HandleTypeDef* uh );
	virtual void UartRxComplete( UART_HandleTypeDef* uh );
	virtual void UartError( UART_HandleTypeDef* uh );
	virtual void UartCommand( UART_HandleTypeDef* uh );
	virtual void UartStartPacket( UART_HandleTypeDef* uh );
	
	virtual void RxISR( UART_HandleTypeDef* huart);
	
	static HandlerManager<UART_HandleTypeDef, UartHandler, 7> manager;
	
	static void StaticUartTxComplete( UART_HandleTypeDef* uh );
	static void StaticUartRxComplete( UART_HandleTypeDef* uh );
	static void StaticUartError( UART_HandleTypeDef* uh );
	
	unsigned int lastrx;
	unsigned int lasttx;
	unsigned int bytetime;
};



#endif
