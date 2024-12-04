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

/* System Include Files
**************************************************************************/
#include <assert.h>


/* Defines
**************************************************************************/
#define RX_BUFF_SIZE 		256

/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Module.h"


class PanelNetwork : public Module
{
	public:

	PanelNetwork( );
		
	int Send( uint8_t* buff );
	
	uint8_t rxbuff[ RX_BUFF_SIZE ];
	
	virtual int Receive( Event );
	virtual int Receive( Message* );
	virtual int Receive( Command* );
	
};
