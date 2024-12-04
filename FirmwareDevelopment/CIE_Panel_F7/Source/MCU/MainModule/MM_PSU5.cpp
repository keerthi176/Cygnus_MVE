/***************************************************************************
* File name: MM_ElmPSU.cpp
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
* ElmPSU Watcher
*
**************************************************************************/

/* System Include Files
**************************************************************************/
#include <string.h>
#ifdef STM32F767xx
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_uart.h"
#else
#include "stm32f4xx_hal.h"
#endif
#include <assert.h>
#include <stdio.h>

/* Defines
**************************************************************************/
 

/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_log.h"
#include "MM_Events.h"
#include "MM_PSU5.h"
#include "MM_fault.h"
#include "MM_CUtils.h"
#include "ewrte.h"



typedef enum
{
	V_OUT,
	V_BATTERY,
	V_CHARGER,
	FAULTS,
	STATUS,
	SERIAL
} ElmCommand;


struct CommandDef
{
	ElmCommand 	cmd;
	const char* request;
	int 			reply_len;
	const char* reply;
};


static CommandDef commands[ ELMPSU_NUM_CMDS ] = {  

	{ V_OUT,			"\n06RH0A\r", 11, "DHVL%04d" },
	{ V_BATTERY,	"\n06RI0B\r", 11, "DIVB%04d" },
	{ V_CHARGER,	"\n06RV18\r", 11, "DVVC%04d" },
	{ FAULTS,		"\n06RB04\r", 11, "DB%06X" },
	{ STATUS,		"\n06RC05\r", 11, "DC%06X" },
};
 

/*************************************************************************/
/**  \fn      ElmPSU::ElmPSU( )
***  \brief   Constructor for class
**************************************************************************/

ElmPSU::ElmPSU( ) : Module( "ElmPSU", 200, EVENT_CLEAR ), UartHandler( app.psu_uart5 ), infault( 0 )
{	
 
}


int ElmPSU::Init( )
{
	GPIO_PinState s1 = HAL_GPIO_ReadPin( GPIOB, GPIO_PIN_12 );
	GPIO_PinState s2 = HAL_GPIO_ReadPin( GPIOB, GPIO_PIN_13 );
	
	if ( s1 == GPIO_PIN_SET && s2 == GPIO_PIN_SET )
	{
		priority = 200;
	}
	else
	{
		priority = 0;
	}	 		
	 
	psu_last_received = now();
	
	return true;
}
 
static int bank = 0;
static int index = 0;
static int received = true;

/*************************************************************************/
/**  \fn      void ElmPSU::Poll(  )
***  \brief   Class function
**************************************************************************/

void ElmPSU::Poll( )
{
	int status = HAL_OK;
	uint8_t buff[ 10];	
	
	if ( got_results )
	{
		ProcessResults( );
		got_results = false;
	}
	
	 	 	 
	if ( psu_last_received > now() - 100 )
	{
		if ( infault )
		{
			Fault::RemoveFault( FAULT_NO_PSU_COMMS );
			infault = false;
		}
	}
	else
	{		
		if ( !infault )
		{
			Fault::AddFault( FAULT_NO_PSU_COMMS );
			infault = true;
		}
		HAL_UART_AbortTransmit( app.psu_uart5 );
	}
	
	HAL_UART_AbortReceive( app.psu_uart5 );
	HAL_UART_Transmit_IT( app.psu_uart5, (uint8_t*) commands[ index].request, 8 );
}


/*************************************************************************/
/**  \fn      void ElmPSU::UartRxComplete( UART_HandleTypeDef* uart  )
***  \brief   Class function
**************************************************************************/

void ElmPSU::UartTxComplete( UART_HandleTypeDef* uart )
{
	HAL_UART_Receive_IT( app.psu_uart5, buff, commands[ index ].reply_len );
}


/*************************************************************************/
/**  \fn      int ElmPSU::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int ElmPSU::Receive( Message* )
{
   return 0;
}

/*************************************************************************/
/**  \fn      void ElmPSU::UartRxComplete( UART_HandleTypeDef* uart  )
***  \brief   Class function
**************************************************************************/

void ElmPSU::UartRxComplete( UART_HandleTypeDef* uart )
{	
	buff[ commands[ index ].reply_len ] = 0;	
	
	sscanf( (const char*)buff + 3, commands[ index ].reply, results + index ); 
	
	if ( ++index == ELMPSU_NUM_CMDS )
	{
		index = 0;
		got_results = true;
	}
	
	psu_last_received = now( );
}


/*************************************************************************/
/**  \fn      void ElmPSU::UartError( UART_HandleTypeDef* uart  )
***  \brief   Class function
**************************************************************************/

void ElmPSU::UartError( UART_HandleTypeDef* uart )
{
	HAL_UART_AbortReceive( app.psu_uart5 );
	 
}
 	 

/*************************************************************************/
/**  \fn      void ElmPSU::UartAbort( UART_HandleTypeDef* uart  )
***  \brief   Class function
**************************************************************************/

void ElmPSU::UartAbort( UART_HandleTypeDef* uart )
{
	 
	
}

 

/*************************************************************************/
/**  \fn      int ElmPSU::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int ElmPSU::Receive( Event event )
{	
	//if event == EVENT_CLEAR
	 
	
	infault 						 = false;
	psu_mains_failure 		 = false;
	battery_low  				 = false;
	battery_critical 			 = false;
	charger_failed    		 = false;
	no_battery					 = false;
	battery_resistance_fault = false;
	over_voltage				 = false;
	battery_not_detected		 = false;
	
	return 0;
}


/*************************************************************************/
/**  \fn      int ElmPSU::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int ElmPSU::Receive( Command* cmd )
{
	assert( cmd != NULL );
	PSU_TYPE psu_type = PSU_TYPE_ELMDENE;
	
	if ( !priority )
	{
		return CMD_ERR_UNKNOWN_CMD;
	}
	
	switch ( cmd->type )
	{
		case CMD_GET_PSU_STATUS :
		{
			cmd->uint3  = psu_last_received;
			cmd->ushort0 = 0;
			cmd->short1 = results[ 0] * 10; // p->battery_charge( );
			cmd->short2 = results[ 2] * 10; // p->battery_on_load( );
			cmd->short3 = results[ 1] * 10; // p->battery_no_load( );
			cmd->short4 = 0; // p->battery_resistance( );	

			if ( (*((ElmPSUFault1*) (results + 3))).ChargerFault ) 
			{
				batt = BATT_CHARGE_FAIL;
			}
			else if ( (*((ElmPSUFault1*) (results + 3))).BatteryHighImpedence )
			{
				batt = BATT_HIGHQ;
			}
			else if ( (*((ElmPSUFault1*) (results + 3))).BatteryLowNoMains )
			{
				batt = BATT_LOW;
			}

			if(battery_not_detected)
			{
				batt = BATT_CRIT;
			}
			else
			{
				batt = BATT_GOOD;
			}

			//cmd->short5 = (short) batt;
			//cmd->short6 = (short)PSU_TYPE_ELMDENE;
			
			cmd->charA = (char) batt;
			cmd->charB = (char)psu_type;

			return CMD_OK;
		}
		
		default: return CMD_ERR_UNKNOWN_CMD;
	}
}	


void ElmPSU::ProcessResults( )
{
	// if OK
	ElmPSUFault1* f = (ElmPSUFault1*) ( (char*)results + 14 ); 
	ElmPSUStatus* s = (ElmPSUStatus*) ( (char*)results + 17 ); 

	if ( !s->MainsPresent )
	{
		if ( !psu_mains_failure )
		{
			Fault::AddFault( FAULT_MAINS_FAILURE );
			psu_mains_failure = true;
		}
	}
	else if ( psu_mains_failure )
	{
		Fault::RemoveFault( FAULT_MAINS_FAILURE );
		psu_mains_failure = false;
	}
	
	if ( f->BatteryMissing )
	{
		if ( !battery_not_detected )
		{
			Fault::AddFault( FAULT_BATTERY_NOT_DETECTED );
			Fault::RemoveFault( FAULT_BATTERY_CRITICAL );
			Fault::RemoveFault( FAULT_BATTERY_LOW);
			battery_not_detected = true;
		}
		
	}
	else if ( battery_not_detected )
	{
		Fault::RemoveFault( FAULT_BATTERY_NOT_DETECTED );
		
		battery_not_detected = false;
		
		if ( f->BatteryLowNoMains ) Fault::AddFault( FAULT_BATTERY_CRITICAL );			
	}
			
	if (  f->BatteryLowNoMains  )
	{
		if ( !battery_critical && !battery_not_detected )
		{
			Fault::AddFault( FAULT_BATTERY_CRITICAL );
			Fault::RemoveFault( FAULT_BATTERY_LOW );
			battery_critical = true;
		}
	}
	else if ( battery_critical )
	{
		Fault::RemoveFault( FAULT_BATTERY_CRITICAL );
		battery_critical = false;
		
		if ( app.site != NULL )
		{
			int batt_charge = results[ V_BATTERY ];  
			
			if ( batt_charge < app.site->batt_panel )
			{
				Fault::AddFault( FAULT_BATTERY_LOW);
			}
		}
	}
	
	if ( f->ChargerFault )
	{
		if ( !charger_failed )
		{
			Fault::AddFault( FAULT_CHARGER_FAILED );
			charger_failed = true;
		}
	}
	else if ( charger_failed )
	{
		Fault::RemoveFault( FAULT_CHARGER_FAILED );
		charger_failed = false;
	}
	
	 
	
	if ( f->BatteryHighImpedence )
	{
		if ( !battery_resistance_fault )
		{
			Fault::AddFault( FAULT_BATTERY_RESISTANCE_TOO_HIGH );
			battery_resistance_fault = true;
		}
	}
	else if ( battery_resistance_fault )
	{
		Fault::RemoveFault( FAULT_BATTERY_RESISTANCE_TOO_HIGH );
		battery_resistance_fault = false;
	}
	
	if ( f->BatteryHeath )
	{
		if ( !over_voltage )
		{
			Fault::AddFault( FAULT_PSU_BATTERY_HEALTH );
			over_voltage = true;
		}
	}
	else if ( over_voltage )
	{
		Fault::RemoveFault( FAULT_PSU_BATTERY_HEALTH );
		over_voltage = false;
	}
	
 
	if ( app.site != NULL )
	{
		int batt_charge = results[ V_BATTERY ] * 10;
		
		if ( batt_charge < app.site->batt_panel )
		{
			if ( !battery_low && !battery_critical && !battery_not_detected )
			{
				Fault::AddFault( FAULT_BATTERY_LOW );
				battery_low = true;
			}
		}
		else if ( battery_low )
		{
			Fault::RemoveFault( FAULT_BATTERY_LOW );
			battery_low = false;
		}
	}
   
}
