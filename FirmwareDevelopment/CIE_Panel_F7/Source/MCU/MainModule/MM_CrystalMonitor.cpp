/***************************************************************************
* File name: MM_CrystalMonitor.cpp
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
#include "MM_CrystalMonitor.h"
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_Application.h"
#include "MM_Events.h"
#include "MM_fault.h"
#include "MM_CUtils.h"
#include "DM_BatteryRam.h"


/* Defines
**************************************************************************/


extern "C" void SystemClock_Config( int );

/*************************************************************************/
/**  \fn      CrystalMonitor::CrystalMonitor( )
***  \brief   Constructor for class
**************************************************************************/

CrystalMonitor::CrystalMonitor( ) : Module( "CrystalMonitor", 0 )
{

}


/*************************************************************************/
/**  \fn      int CrystalMonitor::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int CrystalMonitor::Receive( Event event )
{
 
	return 0;
}


/*************************************************************************/
/**  \fn      int CrystalMonitor::Receive( Command* cmd  )
***  \brief   Class function
**************************************************************************/

int CrystalMonitor::Receive( Command* cmd  )
{
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int CrystalMonitor::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int CrystalMonitor::Receive( Message* )
{
	return 0;
}


/*************************************************************************/
/**  \fn      void HAL_RCC_CSSCallback( void )
***  \brief   Global helper function
**************************************************************************/

int dummy;

extern "C" void HAL_RCC_CSSCallback( void )
{
	//HAL_RCC_DisableCSS( ); // no need to trap again

	// Flag fault
	((int*)BKPSRAM_BASE)[ BATT_RAM_CRYSTAL_FAULT] = 0x55555555;
	
 	for( double x = 1; x < 10; x = x*1.001) dummy++;
	
	HAL_NVIC_SystemReset( ); // Boom!
}

