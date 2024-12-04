/***************************************************************************
* File name: MM_Backlight.cpp
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
#include "stm32f7xx_hal_lptim.h"
#include <assert.h>


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_Backlight.h"
#include "MM_EEPROM.h"
#include "MM_fault.h"
#include "ltdc.h"


#define PWM_WAVELENGTH 0x1A00

/*************************************************************************/
/**  \fn      Backlight::Backlight( )
***  \brief   Constructor for class
**************************************************************************/

Backlight::Backlight( ) : Module( "Backlight", 0 )
{
	eeprom = (EEPROM*) app.FindModule( "EEPROM" );
	
	assert( eeprom != NULL );
	
	active = true;
	
	if ( !LoadSettings( ) )
	{		 
		  /* Default  control */
		HAL_LPTIM_PWM_Start( app.backlight_timer, PWM_WAVELENGTH, PWM_WAVELENGTH - 0x200 );
	}
} 


/*************************************************************************/
/**  \fn      int Backlight::LoadSettings( )
***  \brief   Class function
**************************************************************************/

int Backlight::LoadSettings( )
{
	if ( eeprom->GetVar( ActiveBrightness, activeBrightness ) &&
		  eeprom->GetVar( InactiveBrightness, inactiveBrightness ) )
	{
		if ( active )
		{
	 		HAL_LPTIM_PWM_Start( app.backlight_timer, PWM_WAVELENGTH, PWM_WAVELENGTH - activeBrightness  );
		}
		else
		{
			HAL_LPTIM_PWM_Start( app.backlight_timer, PWM_WAVELENGTH, PWM_WAVELENGTH - inactiveBrightness  );
		}
		return true;
	}		

	Fault::AddFault( FAULT_BACKLIGHT_PARAMS_GET );
	//Log::Msg(LOG_SYS, "01_Backlight.cpp -- ADD");
	
	return false;
}


/*************************************************************************/
/**  \fn      int Backlight::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int display_on = true;

int Backlight::Receive( Command* cmd )
{
	if ( cmd->type == CMD_SET_BACKLIGHT_ON )
	{
		// Enable display	
		//XRect r = { 0, 0, 800, 480 };
		
		//EwBeginUpdateArea( EmbeddedWizard::viewport, r );
		//EwEndUpdate( EmbeddedWizard::viewport, r );
		// Above didnt work to refresh display
		
		HAL_LPTIM_PWM_Start( app.backlight_timer, PWM_WAVELENGTH, PWM_WAVELENGTH - activeBrightness );
		 __HAL_LPTIM_ENABLE( app.backlight_timer );
		HAL_LPTIM_PWM_Start( app.backlight_timer, PWM_WAVELENGTH, PWM_WAVELENGTH - activeBrightness );
		
		HAL_GPIO_WritePin(GPIOA, DISPLAY_EN_Pin, GPIO_PIN_SET );
		HAL_LTDC_MspInit( &hltdc );
		display_on = true;
		return CMD_OK;
	}
	
	if ( cmd->type == CMD_SET_BACKLIGHT_OFF )
	{
		// Stop PWM
		HAL_LPTIM_PWM_Stop( app.backlight_timer );
	 
		// Disable display
		HAL_GPIO_WritePin(GPIOA, DISPLAY_EN_Pin, GPIO_PIN_RESET );		
		HAL_LTDC_MspDeInit( &hltdc );
		display_on = false;
		
		return CMD_OK;
	}	
		
	if ( cmd->type == CMD_SET_BACKLIGHT_LEVEL )
	{
	 	activeBrightness = cmd->int0*10 + 2;
		app.backlight_timer->Instance->CMP = PWM_WAVELENGTH - activeBrightness;
		
		eeprom->SetVar( ActiveBrightness, activeBrightness );
		
		inactiveBrightness = activeBrightness >> 2;
		eeprom->SetVar( InactiveBrightness, inactiveBrightness );
				
		return CMD_OK;
	}
	if ( cmd->type == CMD_GET_BACKLIGHT_LEVEL )
	{
		eeprom->GetVar( ActiveBrightness, activeBrightness );
		
		cmd->int0 = ( activeBrightness - 2 ) / 10;
	 					
		return CMD_OK;
	}
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int Backlight::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int Backlight::Receive( Event event )
{
	
	return 0;
}


/*************************************************************************/
/**  \fn      int Backlight::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int Backlight::Receive( Message* )
{
   return 0;
}

