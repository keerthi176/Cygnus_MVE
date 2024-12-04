/***************************************************************************
* File name: MM_RBU.cpp
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
* Crystal Monitoring Module
*
**************************************************************************/

/* System Include Files
**************************************************************************/
#ifdef STM32F767xx
#include "stm32f7xx_hal_rcc.h"
#else
#include "stm32f4xx_hal_rcc.h"
#endif
#include <assert.h>
#include "stm32f7xx_hal_rtc.h"


/* User Include Files
**************************************************************************/
#include "MM_RBU.h"
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_Application.h"
#include "MM_Events.h"
#include "MM_fault.h"
#include "MM_CUtils.h"


/* Defines
**************************************************************************/

 
/*************************************************************************/
/**  \fn      RBU::RBU( )
***  \brief   Constructor for class
**************************************************************************/

RBU::RBU( ) : Module( "RBU", 0 )
{

}


/*************************************************************************/
/**  \fn      int RBU::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int RBU::Receive( Event event )
{
 
	return 0;
}


/*************************************************************************/
/**  \fn      int RBU::Receive( Command* cmd  )
***  \brief   Class function
**************************************************************************/

int RBU::Receive( Command* cmd  )
{
	
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int RBU::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int RBU::Receive( Message* )
{
	return 0;
}



int RBU::AddDevice( )
{
	