/***************************************************************************
* File name: MM_DFU.h
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
* EEPROM Manager - NOTE:  Variables will not be accessible when
* settingDefaults is true. 
*
**************************************************************************/
#ifndef _MM_DFU_H_
#define _MM_DFU_H_


/* System Include Files
**************************************************************************/
 

#include "stm32f7xx_hal.h"



/* Defines
**************************************************************************/



/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Module.h"
 

/* Globals
**************************************************************************/

	
class DFUMode : public Module
{
	public:
	
	DFUMode( );
	
	virtual int Receive( Message* );
	virtual int Receive( Event );
	virtual int Receive( Command* cmd );	
};


#endif

