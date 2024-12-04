/***************************************************************************
* File name: Backlight.cpp
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
* Backlight control module
*
**************************************************************************/

/* System Include Files
**************************************************************************/

/* User Include Files
**************************************************************************/
#include "Application.h"
#include "Command.h"
#include "Backlight.h"

Backlight::Backlight( Application* app ) : Module( "Backlight", 0, app )
{
   /* Init backlight control */
   if ( HAL_TIM_PWM_Start( app->backlight_timer, TIM_CHANNEL_1) != HAL_OK )
   {
		/* PWM Generation Error */
		// Error_Handler();
   }
} 


int Backlight::Receive( Command* cmd )
{
	if ( cmd->type == CMD_SET_BACKLIGHT_LEVEL )
	{
		app->backlight_timer->Instance->CCR1 = cmd->int0;
		
		return MODULE_CMD_ISSUED;
	}
	return MODULE_CMD_UNKNOWN;
}


int Backlight::Receive( Message* msg )
{
	return MODULE_MSG_UNKNOWN;
}
