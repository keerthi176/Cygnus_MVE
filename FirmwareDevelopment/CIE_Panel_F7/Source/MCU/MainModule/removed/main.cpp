/***************************************************************************
* File name: main.cpp
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
* Module base class.
*
**************************************************************************/
/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/

#include "ewrte.h"
#include "ewgfx.h"
#include "Module.h"
#include "Message.h"
#include "Application.h"
#include "MemCheck.h"
#include "Backlight.h"
#include "EmbeddedWizard.h"
#include "CrystalMonitor.h"




/* Defines
**************************************************************************/


/* Globals  - eeek
**************************************************************************/
Application* app;


extern "C" void main_application( )
{	
	app = new Application( /* gsm_uart 				*/ &huart4,
								  /* debug_uart 			*/ &huart1,
								  /* panel_network_uart	*/ &huart3,
								  /* touch_i2c 			*/ &hi2c1,
								  /* backlight_timer    */	&htim1,
								  /* rtc 					*/ &hrtc,
								  /* crc				   	*/ &hcrc,
								  /* watch dog */          &hiwdg  );
	
	new TouchScreen( app );
	new MemCheck( app );
	new Backlight( app );
	new CrystalMonitor( app );
	new EmbeddedWizard( app );
	
	app->Go( );
}


