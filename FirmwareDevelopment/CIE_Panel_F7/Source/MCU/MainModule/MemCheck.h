/***************************************************************************
* File name: MemCheck.h
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
* Memory check module.
*
**************************************************************************/
/* System Include Files
**************************************************************************/
#include "stm32f4xx_hal_crc.h"

/* User Include Files
**************************************************************************/
#include "Module.h"

/* Defines
**************************************************************************/

class Message;
class ATCommand;

class MemCheck : public Module
{
	public:
		
	MemCheck( Application* app );
	
	virtual void Poll( void ) ;

	
	private:
		
	int* 	mem_pos;
	int* 	mem_start;
	int*	mem_end;
	int 	checksum;
	CRC_TypeDef* crc;
};


