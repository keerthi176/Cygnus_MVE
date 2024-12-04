/***************************************************************************
* File name: MM_CauseAndEffect.cpp
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
* Cause And Effect control module
*
**************************************************************************/
 
 

/* User Include Files
**************************************************************************/

#include "CO_Site.h"
#include "MM_Device.h"
#include "MM_IOStatus.h"



#define CAE_DISABLEMENTS 1


class CauseAndEffect : public Module
{
	public:
	
	CauseAndEffect( );

	virtual int Init( );
	
	int GetInputStatus( int input, IOStatus* iostatus, Device* list, int numDevs, CAEInput* inputs, ActionEnum action, Rule* );
	int SetOutputStatus( CAERule* r, Device* list, int numDevs, CAEOutput* outputs, bool set_silenceable, int istrue );
};


