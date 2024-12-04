/***************************************************************************
* File name: MM_BIST.h
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
* Built In Self Test
*
**************************************************************************/
#ifndef _MM_BIST_H_
#define _MM_BIST_H_


/* User Include Files
**************************************************************************/
#include "MM_Module.h"


/* Defines
**************************************************************************/


class BIST : public Module 
{
	public:
	
	BIST( Application* app );
	
	int CheckI2C( I2C_HandleTypeDef* bus, int device, int addr, char val );
	
	int CheckEEPROM( I2C_HandleTypeDef* bus, int device );
	
	void ListI2C( I2C_HandleTypeDef* bus );
}
