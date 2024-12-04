/***************************************************************************
* File name: MM_Utilities.cpp
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
* Various utiltiies
*
**************************************************************************/

/* System Include Files
**************************************************************************/

#ifdef STM32F767xx
#include "stm32f7xx_hal_rcc.h"
#include "stm32f767xx.h"
//#include "stm32f769i_eval_io.h"
#else
#include "stm32f4xx_hal_rcc.h"
#include "stm32f469xx.h"
#include "stm32469i_eval_io.h"
#endif


#include <assert.h>

/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_Utilities.h"

#include "MM_TouchScreen.h"

#include "MM_fault.h"

#include "MM_Events.h"
#include "MM_Message.h"
#include "stmpe811.h"

#include "version.h"
#include "MM_CUtils.h"

#include "MM_SDFATFS.h"

/* Define
**************************************************************************/

/* Globals
**************************************************************************/

int Utilities::AccessLevel = 1;
char Utilities::UID[ 25] = {0};

static uint8_t sdcPresent    = true;
static uint8_t sdcFaultFlag  = false;
static uint8_t sdcDetect;

enum
{
	MSG_UTILS_ACCESS_MODE_UPDATE = 5
};	
	
static char logMessageBuffer[64];

extern SD_HandleTypeDef hsd1;

/*************************************************************************/
/**  \fn      Utilities::Utilities( )
***  \brief   Constructor for class
**************************************************************************/

Utilities::Utilities( ) : Module( "Utilities", 16, EVENT_KEY_SWITCH | EVENT_CALIBRATION_REQUEST | 
																	EVENT_SD_CARD_CHANGED | EVENT_STMPE811_ADC_COMPLETE )
{	

	sprintf( Utilities::UID, "%08X%08X%08X", HAL_GetUIDw0(), HAL_GetUIDw1(), HAL_GetUIDw2() );
	
}	
	

/*************************************************************************/
/**  \fn      void Utilities::Poll( )
***  \brief   Constructor for class
**************************************************************************/

int Utilities::Init( )
{
	int level;
	
	eeprom  = (EEPROM*) app.FindModule( "EEPROM" );
 
	assert( eeprom != NULL );
	
	if ( GPIOManager::Get( GPIO_KeySwitch ) )
	{
		level = 2;
	}
	else
	{
		level = 1;
	}
			
	Send( MSG_UTILS_ACCESS_MODE_UPDATE, this, level );		
	
	return true;
}


/*************************************************************************/
/**  \fn      void Utilities::Poll( )
***  \brief   Constructor for class
**************************************************************************/

void Utilities::Poll( )
{
	static int count;
	char logMessage[] = "--- SD CARD REMOVED ---\0";
	
	if ( count++ & 1 )
	{
		gpioman->Set( GPIO_I2C1Heartbeat, 1 );
		gpioman->Set( GPIO_I2C2Heartbeat, 1 );

		HAL_GPIO_WritePin( I2C1_EN_GPIO_Port, I2C1_EN_Pin, GPIO_PIN_SET );

		if(!(count % 10))
		{
			/* SD Card Present     = SD_Detect_Pin = 0
			   SD Card NOT Present = SD_Detect_Pin = 1
			   So inverting the read pin output.
			*/
			sdcDetect = ((pca9535e_I2CIOExp_ReadPin(0x40, 11)) ^ (1 << 0) );

			if(sdcDetect == true)
			{
				sdcPresent = true;
			}
			else
			{
				sdcPresent = false;
			}

			if((sdcPresent == false) && (sdcFaultFlag != true))
			{
				Fault::AddFault( FAULT_SD_CARD_NOT_DETECTED );
				sdcFaultFlag = true;

				CreateLogMessage(logMessageBuffer, logMessage, sizeof(logMessage));
			}
			else if((sdcPresent == true) && (sdcFaultFlag != false))
			{
				HAL_SD_DeInit(&hsd1);

				SDFATFS::SDFATFS_Init();

				sdcFaultFlag = false;
				Fault::RemoveFault( FAULT_SD_CARD_NOT_DETECTED );

				Log::Msg(LOG_SYS, logMessageBuffer);
				Log::Msg(LOG_SYS, "--- SD Card Installed ---");
			}
			else
			{
				/* Do Nothing */
				/* Added this message for continuous log testing - Commenting out for release */
				//Log::Msg(LOG_SYS, "--- Reading SD Detect Pin ---");
			}
		}
	}
	else
	{
		gpioman->Set( GPIO_I2C1Heartbeat, 0 );
		gpioman->Set( GPIO_I2C2Heartbeat, 0 );
		HAL_GPIO_WritePin( I2C1_EN_GPIO_Port, I2C1_EN_Pin, GPIO_PIN_RESET );		
	}
}	
		
#define AVG_NUM 8


/*************************************************************************/
/**  \fn      int Utilities::Receive( Message* msg )
***  \brief   Class function
**************************************************************************/

int Utilities::Receive( Message* msg )
{
	switch( msg->type )
	{
		case MSG_UTILS_ACCESS_MODE_UPDATE:
		{
			AppDeviceClass_TriggerAccessModeChangeEvent( app.DeviceObject, msg->value );
			break;
		}	
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int Utilities::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int Utilities::Receive( Command* cmd ) 
{
	assert( cmd != NULL );
	
	switch ( cmd->type )
	{ 					 
		case CMD_GET_VERSION_NUMBERS :
		{
			int value = 200;
			
			cmd->int0 = (int) ((float)FIRMWARE_VERSION * 100.0f);
			cmd->int1 = value;
			
			return CMD_OK;
		}
		case CMD_SET_ACCESS_LEVEL :
		{
			AccessLevel = cmd->int0;
			
			Send( MSG_UTILS_ACCESS_MODE_UPDATE, this, AccessLevel );
			return CMD_OK;
		}
		
		default:
			return CMD_ERR_UNKNOWN_CMD;
	}
}


/*************************************************************************/
/**  \fn      int Utilities::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int Utilities::Receive( Event event ) 
{
	// Using shared I2C so handle outside irq
	
	switch ( event )
	{		
		case EVENT_CALIBRATION_REQUEST :
		{
			app.DebOut( "Calibration request" );
			break;
		}

		case EVENT_SD_CARD_CHANGED:
		{
			app.DebOut( "SD Card inserted/removed" );
			break;
		}
		
		case EVENT_KEY_SWITCH:
		{
			int level = 1;
			
			if ( gpioman->Get( GPIO_KeySwitch ) )
			{
				level = 2;
			}
			
			TouchScreen::last_tap = now( ); // key op counts 
				
			Send( MSG_UTILS_ACCESS_MODE_UPDATE, this, level );
		
			break;
		}
	}
	return 0;
}

 


/*************************************************************************/
/**  \fn      LogType GetLogType( int actionbit, char** typetext )
***  \brief   Static function
**************************************************************************/

LogType GetLogType( int actionbit, const char** typetext )
{
	LogType logtype = LOG_EVT;
	*typetext = "";
	
	switch ( actionbit ) 
	{
		case ACTION_SHIFT_FIRE : 	 		logtype = LOG_FIRE;
													*typetext = "Fire";
													break;
		case ACTION_SHIFT_FIRST_AID: 		*typetext = "First Aid";												 
													break;
		case ACTION_SHIFT_SECURITY:  		*typetext = "Security";												 
													break;
		case ACTION_SHIFT_GENERAL:			*typetext = "General";												 
													break;
		case ACTION_SHIFT_EVACUATION : 	*typetext = "Evacuate";													 
													break;	
		case ACTION_SHIFT_FAULT: 			*typetext = "Fault";
													logtype = LOG_FLT;
													break;
		case ACTION_SHIFT_ROUTING_ACK : *typetext = "Routing Ack";
													break;	
		case ACTION_SHIFT_RESET : 			*typetext = "Reset";
													break;	
		case ACTION_SHIFT_SILENCE : 		*typetext = "Silence";
													break;	
		case ACTION_SHIFT_UNDEFINED : 	*typetext = "Undefined";
													break;
		default:									*typetext = "Unknown";
	}
	return logtype;
}

