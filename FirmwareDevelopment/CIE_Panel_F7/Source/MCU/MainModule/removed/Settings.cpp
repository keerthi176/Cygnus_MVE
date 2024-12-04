/***************************************************************************
* File name: Settings.cpp
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
* Panel settings class. Stores network, mesh, etc
*
**************************************************************************/


/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "Settings.h"

/* Defines
**************************************************************************/


int Settings::GetSize()
{
	return 16 * 1024 ;
}
	

int Settings::GetROMAddress()
{
	return 0xC0000000;
}



