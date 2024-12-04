/***************************************************************************
* File name: MM_LED.cpp
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
* LED control module
*
**************************************************************************/

/* System Include Files
**************************************************************************/
 

/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_LED.h"
#include "MM_Buzzer.h" 
#include "MM_Log.h"

/* Defines
**************************************************************************/



/* Globals
**************************************************************************/
 
int LED::testMode = 0;

char LED::state[ GPIO_END ] = {0};
unsigned int LED::zone[ SITE_MAX_ZONES / sizeof(short) ];



/*************************************************************************/
/**  \fn      LED::LED( )
***  \brief   Constructor for class
**************************************************************************/

LED::LED( ) : Module( "LED", 4, EVENT_ENTER_SAFE_MODE ), TimerHandler( app.led_flash_timer )
{
	flash = FLASH_DONT;
}


/*************************************************************************/
/**  \fn      LED::Init( )
***  \brief   Initializer for class
**************************************************************************/

int LED::Init( )
{
 	HAL_TIM_PWM_Start_IT( app.led_flash_timer, TIM_CHANNEL_1 );  
	
	app.led_flash_timer->Instance->DIER |= TIM_DIER_UIE;
	return true;
}



/*************************************************************************/
/**  \fn      void LED::On( GPIO gpio )
***  \brief   Constructor for class
**************************************************************************/

void LED::On( GPIO gpio )
{
	Set( gpio, LED_ON );
}


/*************************************************************************/
/**  \fn      void LED::Off( GPIO gpio )
***  \brief   Constructor for class
**************************************************************************/

void LED::Off( GPIO gpio )
{
	Set( gpio, LED_OFF );
}


/*************************************************************************/
/**  \fn      void LED::Off( GPIO gpio )
***  \brief   Constructor for class
**************************************************************************/

void LED::Flash( GPIO gpio )
{
	Set( gpio, LED_FLASH );
}


/*************************************************************************/
/**  \fn      void LED::ZoneOn( int n )
***  \brief   Constructor for class
**************************************************************************/

void LED::ZoneOn( int n )
{
	SetZone( n, 1 );
}


/*************************************************************************/
/**  \fn      void LED::ZoneOff( int n )
***  \brief   Constructor for class
**************************************************************************/

void LED::ZoneOff( int n )
{
	SetZone( n, 0 );
}


/*************************************************************************/
/**  \fn      void LED::Set( GPIO gpio, int state )
***  \brief   class function
**************************************************************************/

void LED::Set( GPIO gpio, LEDStatus _state )
{
	if ( state[ gpio ] != _state )
	{
		if ( !testMode )
		{
			if ( _state == LED_ON )
			{
				GPIOManager::Set( gpio, IO_PIN_SET );
			}
			else if ( _state == LED_OFF )
			{
				GPIOManager::Set( gpio, IO_PIN_RESET );
			}
		} 
		state[ gpio ] = _state;			
	}
}


/*************************************************************************/
/**  \fn      void LED::SetZone( GPIO gpio, int state )
***  \brief   class function
**************************************************************************/

void LED::SetZone( int n, int _state )
{
	if ( !testMode )
	{
		GPIOManager::SetZone( n, (IO_PinState) _state );
	}
	
	n--;
	
	if ( _state )
	{
		zone[ (n/16) ] |= 1 << ( 15 -  (n & 15) );
	}
	else
	{
		zone[ (n/16) ] &= ~(1 << ( 15 - (n & 15) ) );
	}			
}


/*************************************************************************/
/**  \fn      void LED::SetZone( GPIO gpio, int state )
***  \brief   class function
**************************************************************************/

int LED::GetZone( int n )
{	
	n--;
	
	return ( zone[ (n/16) ] >> ( 15 - ( n & 15 ) )) & 1;	 	
}


/*************************************************************************/
/**  \fn      void LED::Set( GPIO gpio, int state )
***  \brief   class function
**************************************************************************/

void LED::LampTest( int stage )
{
	stage = stage % 24;
	Log::Msg( LOG_TST, "LED Lamp Test Stage %d in Progress\n", stage );
	GPIOManager::Set( GPIO_FireRoutingFault, 	stage == 0 );
	GPIOManager::Set( GPIO_PSUFault, 			stage == 0 );	
	GPIOManager::Set( GPIO_SounderFault, 		stage == 0 );	
	GPIOManager::Set( GPIO_Fire, 					stage == 0 );
	GPIOManager::Set( GPIO_GenFault, 			stage == 0 );	
	GPIOManager::Set( GPIO_SysFault, 			stage == 0 );	
	
	GPIOManager::Set( GPIO_DelaysActive, 		stage == 1 );
	GPIOManager::Set( GPIO_TestMode, 			stage == 1 );	
	GPIOManager::Set( GPIO_OtherEvents, 		stage == 1 );	
	GPIOManager::Set( GPIO_Disablements, 		stage == 1 );	
	GPIOManager::Set( GPIO_FireRoutingActive, stage == 1 );		
	
	
	GPIOManager::SetZones( 0, 0xFFFF, stage == 2 );
	GPIOManager::SetZones( 1, 0xFFFF, stage == 3 );
	GPIOManager::SetZones( 2, 0xFFFF, stage == 4 );
	GPIOManager::SetZones( 3, 0xFFFF, stage == 5 );
	GPIOManager::SetZones( 4, 0xFFFF, stage == 6 );
	GPIOManager::SetZones( 5, 0xFFFF, stage == 7 );
	
	if ( stage > 7 )
	{
		int i = stage - 8;
			
		GPIOManager::SetZones( 0, 1 << i, 1 );
		GPIOManager::SetZones( 1, 1 << i, 1 );
		GPIOManager::SetZones( 2, 1 << i, 1 );
		GPIOManager::SetZones( 3, 1 << i, 1 );
		GPIOManager::SetZones( 4, 1 << i, 1 );
		GPIOManager::SetZones( 5, 1 << i, 1 );
	}
}


/*************************************************************************/
/**  \fn      void LED::Receive( Event e )
***  \brief   class function
**************************************************************************/

int LED::Receive( Event e )
{
	if ( e == EVENT_ENTER_SAFE_MODE )
	{
		LED::Off( GPIO_TestMode );
		LED::Off( GPIO_DelaysActive );
		LED::Off( GPIO_TestMode );
		LED::Off( GPIO_OtherEvents );    
		LED::Off( GPIO_Disablements );   
		LED::Off( GPIO_FireRoutingActive );	
	}
		
	return 0;
}


/*************************************************************************/
/**  \fn      void LED::Receive( Message* )
***  \brief   class function
**************************************************************************/

int LED::Receive( Message* )
{
	return 0;
}
 


/*************************************************************************/
/**  \fn      void LED::RestoreLEDS( )
***  \brief   class function
**************************************************************************/

void LED::RestoreLEDS( )
{
	for( int n = 0; n < GPIO_END; n++ )
	{
		if ( state[ n] == LED_ON ) GPIOManager::Set( (GPIO) n, IO_PIN_SET );
		else if ( state[ n] == LED_OFF ) GPIOManager::Set( (GPIO) n, IO_PIN_RESET );
	}	
	
	for( int zb= 0; zb <= SITE_MAX_ZONES / 16; zb++ )
	{
		GPIOManager::SetZones( zb, 0xFFFF, 0 );
		GPIOManager::SetZones( zb, zone[ zb ], 1 );
	}
}


/*************************************************************************/
/**  \fn      void LED::AllLEDSOff( )
***  \brief   class function
**************************************************************************/

void LED::AllLEDSOff( )
{
	for( int n = 0; n < GPIO_END; n++ )
	{
		if ( state[ n] == LED_ON || state[ n] == LED_OFF ) GPIOManager::Set( (GPIO) n, IO_PIN_RESET );;
	}
	
}


/*************************************************************************/
/**  \fn      void LED::TimerPulseFinished( TIM_HandlerTypeDef* )
***  \brief   class function
**************************************************************************/

void LED::TimerPulseFinished( TIM_HandleTypeDef* )
{
	flash = FLASH_OFF;
}


/*************************************************************************/
/**  \fn      void LED::TimerPulseFinished( TIM_HandlerTypeDef* )
***  \brief   class function
**************************************************************************/

void LED::TimerPeriodElapsed( TIM_HandleTypeDef* )
{
	flash = FLASH_ON;
}

static int stage = 0;

/*************************************************************************/
/**  \fn      void LED::Receive( Command*)
***  \brief   class function
**************************************************************************/

int LED::Receive( Command* cmd )
{
	switch ( cmd->type )
	{
		case CMD_LEDS_TEST:
		{
			if ( cmd->int0 == 0 )
			{
				testMode = false;
				RestoreLEDS( );
				Buzzer::Mute( BUZZ_FOR_TESTING );
			}
			else
			{
				testMode = true;
				stage = 0;
				AllLEDSOff( );
				Buzzer::Buzz( BUZZ_FOR_TESTING );
			}
			return CMD_OK;
		}
	}
				
	return CMD_ERR_UNKNOWN_CMD;
}



/*************************************************************************/
/**  \fn      void LED::Poll( )
***  \brief   class function
**************************************************************************/

void LED::Poll( )
{
	if ( !testMode )
	{
		if ( flash == FLASH_ON )
		{
			flash = FLASH_DONT;
			
			if ( state[ GPIO_DelaysActive ] == LED_FLASH )
			{
				GPIOManager::Set( GPIO_DelaysActive, IO_PIN_SET );
			}
			if ( state[ GPIO_FireRoutingActive ] == LED_FLASH )
			{
				GPIOManager::Set( GPIO_FireRoutingActive, IO_PIN_SET );
			}
			if ( state[ GPIO_FireRoutingFault ] == LED_FLASH )
			{
				GPIOManager::Set( GPIO_FireRoutingFault, IO_PIN_SET );
			}
			if ( state[ GPIO_SounderFault ] == LED_FLASH )
			{
				GPIOManager::Set( GPIO_SounderFault, IO_PIN_SET );
			}
			if ( state[ GPIO_SysFault ] == LED_FLASH )
			{
				GPIOManager::Set( GPIO_SysFault, IO_PIN_SET );
			}
			if ( state[ GPIO_GenFault ] == LED_FLASH )
			{
				GPIOManager::Set( GPIO_GenFault, IO_PIN_SET );
			}
			if ( state[ GPIO_PSUFault ] == LED_FLASH )
			{
				GPIOManager::Set( GPIO_PSUFault, IO_PIN_SET );
			}
		}
		else if ( flash ==  FLASH_OFF )
		{
			flash = FLASH_DONT;
			
			if ( state[ GPIO_DelaysActive ] == LED_FLASH )
			{
				GPIOManager::Set( GPIO_DelaysActive, IO_PIN_RESET );
			}
			if ( state[ GPIO_FireRoutingActive ] == LED_FLASH )
			{
				GPIOManager::Set( GPIO_FireRoutingActive, IO_PIN_RESET );
			}
			if ( state[ GPIO_FireRoutingFault ] == LED_FLASH )
			{
				GPIOManager::Set( GPIO_FireRoutingFault, IO_PIN_RESET );
			}
			if ( state[ GPIO_SounderFault ] == LED_FLASH )
			{
				GPIOManager::Set( GPIO_SounderFault, IO_PIN_RESET );
			}
			if ( state[ GPIO_SysFault ] == LED_FLASH )
			{
				GPIOManager::Set( GPIO_SysFault, IO_PIN_RESET );
			}
			if ( state[ GPIO_GenFault ] == LED_FLASH )
			{
				GPIOManager::Set( GPIO_GenFault, IO_PIN_RESET );
			}
			if ( state[ GPIO_PSUFault ] == LED_FLASH )
			{
				GPIOManager::Set( GPIO_PSUFault, IO_PIN_RESET );
			}
		}
	}	
	else 	
	{
		if ( flash == FLASH_ON )
		{
			static int count = 0;
			
			if ( ( ++count & 3 ) == 0 )
			{
				LampTest( stage++ );
			}
		}
	}
}
