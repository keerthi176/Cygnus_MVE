/***************************************************************************
* File name: MM_DebugPort.h
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
#ifndef _MM_DEBUG_PORT_H_
#define _MM_DEBUG_PORT_H_

/* System Include Files
**************************************************************************/



/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "DM_UartHandler.h"


/* Defines
**************************************************************************/
#define RX_BUFF_LEN ( 128 )

class ATCommands;

class DebugPort : public Module, public UartHandler 
{
	public:
	
	DebugPort( );
		
	virtual int Receive( Command* cmd );
	virtual int Receive( Event event );
	virtual int Receive( Message* );
	virtual int Init( );
	virtual void UartCommand( UART_HandleTypeDef* uart );
	
	private:
	
	char rxbuffer[ RX_BUFF_LEN ];
	ATCommands* atcmds;
};
	

#endif

