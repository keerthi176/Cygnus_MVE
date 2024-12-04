/***************************************************************************
* File name: MM_ADC.h
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
* ADC Manager
*
**************************************************************************/
#ifndef _MM_ADC_H_
#define _MM_ADC_H_


/* System Include Files
**************************************************************************/
//#include <string.h>
//#ifdef STM32F767xx
//#include "stm32f7xx_hal_spi.h"
//#else
//#include "stm32f4xx_hal_spi.h"
//#endif
#include "stm32f7xx_hal.h"

/* Defines
**************************************************************************/
#ifdef ADC
#undef ADC
#endif

/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_Command2.h"
#include "MM_log.h"
#include "MM_Events.h"


class PanelIO;

  
class ADC : public Module
{
	public:
	
	ADC( );
	
	virtual int Init( );
	virtual void Poll( );
	
	virtual int Receive( Event );
	virtual int Receive( Command* );
   virtual int Receive( Message* );

	static int GetValue( int chan );
	static int GetAvgValue( int chan );
	static GPIO_TypeDef* 	port;
	static uint16_t		 	pin;
	
	static int complete;
	
	HAL_StatusTypeDef TriggerChannels( );
};

 


#endif

