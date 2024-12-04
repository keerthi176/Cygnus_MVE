/***************************************************************************
* File name: CrystalMonitor.cpp
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
#include "stm32f4xx_hal_rcc.h"

/* User Include Files
**************************************************************************/
#include "CrystalMonitor.h"
#include "Module.h"
#include "Message.h"
#include "Application.h"

/* Defines
**************************************************************************/


CrystalMonitor::CrystalMonitor( Application* app ) : Module( "CrystalMonitor", 0, app )
{
	HAL_RCC_EnableCSS( );
}


