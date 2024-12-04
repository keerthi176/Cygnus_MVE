/***************************************************************************
* File name: MM_PSU.cpp
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
* PSU Watcher
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

/* Defines
**************************************************************************/
#define POLY 0xA001
#define MAX_BAUD_DIFF		50


/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_log.h"
#include "MM_Events.h"
#include "MM_PSU.h"
#include "MM_fault.h"
#include "MM_CUtils.h"
#include "ewrte.h"


char psu_mains_failure = 0;
time_t psu_last_received = 0;


/*************************************************************************/
/**  \fn      int PSUPacket::temp( )
***  \brief   Class function
**************************************************************************/

int PSUPacket::temp( )
{
	return ( temp_msb << 8 ) + temp_lsb;
}

/*************************************************************************/
/**  \fn      int PSUPacket::battery_resistance( )
***  \brief   Class function
**************************************************************************/

int PSUPacket::battery_resistance( )
{
	return ( battery_no_load() - battery_on_load() ) * 10;
}


/*************************************************************************/
/**  \fn      int PSUPacket::battery_charge( )
***  \brief   Class function
**************************************************************************/

int PSUPacket::battery_charge( )
{
	return ( battery_charge_msb << 8 ) + battery_charge_lsb;
}


/*************************************************************************/
/**  \fn      int PSUPacket::battery_on_load( )
***  \brief   Class function
**************************************************************************/

int PSUPacket::battery_on_load( )
{
	return ( battery_on_load_msb << 8 ) + battery_on_load_lsb;
}


/*************************************************************************/
/**  \fn      int PSUPacket::battery_no_load( )
***  \brief   Class function
**************************************************************************/

int PSUPacket::battery_no_load( )
{
	return ( battery_no_load_msb << 8 ) + battery_no_load_lsb;
}

PSUPacket*  PSU::rxbuffer;	
PSUPacket*  PSU::prevbuffer;	

bool PSU::got_packet = false;
int PSU::infault = false;

/*************************************************************************/
/**  \fn      PSU::PSU( )
***  \brief   Constructor for class
**************************************************************************/

PSU::PSU( ) : Module( "PSU", 35, EVENT_CLEAR ) 
{	
	rxbuffer = packet_buffer;
	prevbuffer = packet_buffer + 1;
	got_packet = false;
	
	app.psu_uart->RxEventCallback = PSU::UartRxComplete;
}


int PSU::Init( )
{
	HAL_StatusTypeDef status = HAL_OK;
	uint8_t buff;	
		
	GPIO_PinState s1 = HAL_GPIO_ReadPin( GPIOB, GPIO_PIN_12 );
	GPIO_PinState s2 = HAL_GPIO_ReadPin( GPIOB, GPIO_PIN_13 );
	
	if ( s1 == GPIO_PIN_SET && s2 == GPIO_PIN_SET )
	{
		priority = 0;
	}
	else
	{
		priority = 35;
	
		wait_for_idle = false;
		 	
		// Start rx
#if 0
		if ( HAL_UARTEx_ReceiveToIdle_IT( app.psu_uart, (uint8_t*) &rxbuffer->start, PSU_PACKET_SIZE ) != HAL_OK )
		{
			/* Transfer error in reception process */
			Fault::AddFault( FAULT_NO_PSU_COMMS );
			infault = true;
		}	
#endif

		Receive( EVENT_CLEAR) ;
	}	
	return true;
}


/*************************************************************************/
/**  \fn      void PSU::UartRxComplete( UART_HandleTypeDef* uart  )
***  \brief   Class function
**************************************************************************/

void PSU::UartRxComplete( UART_HandleTypeDef* uh, uint16_t bytes )
{
	got_packet = true;
	
	PSUPacket* tmp = rxbuffer;
	rxbuffer = prevbuffer;
	prevbuffer = tmp;
	
	if ( HAL_UARTEx_ReceiveToIdle_IT( app.psu_uart, (uint8_t*) &rxbuffer->start, PSU_PACKET_SIZE ) != HAL_OK )
	{
		/* Transfer error in reception process */
		Fault::AddFault( FAULT_NO_PSU_COMMS );
		infault = true;
	}	
}

/*************************************************************************/
/**  \fn      void PSU::Poll(  )
***  \brief   Class function
**************************************************************************/

void PSU::Poll( )
{
	if ( got_packet )
	{		
		got_packet = false;
		
		if ( ProcessPacket( prevbuffer ) )
		{
			packets_missed = 0;
			
			if ( infault )
			{
				Fault::RemoveFault( FAULT_NO_PSU_COMMS );
				infault = false;
			}
		}	
	}
	else if ( wait_for_idle )
	{
		uint8_t dummy;
		
		HAL_UART_AbortReceive( app.psu_uart );
	 
		wait_for_idle = false;
		
		if ( HAL_UARTEx_ReceiveToIdle_IT( app.psu_uart, (uint8_t*) &rxbuffer->start, PSU_PACKET_SIZE ) != HAL_OK )
		{
			/* Transfer error in reception process */
			Fault::AddFault( FAULT_NO_PSU_COMMS );
			infault = true;
		}				
		 
	}
	else // if no packet recevied
	{	
		if ( ( ++packets_missed & 7 ) == 0 )
		{			
			wait_for_idle = true;	
		}			
		if ( packets_missed > 40 )
		{
			// if not already  in
			if ( !infault ) 
			{
				/* Transfer error in reception process */
				Fault::AddFault( FAULT_NO_PSU_COMMS );
				infault = true;
			}
		}		
	}
}


/*************************************************************************/
/**  \fn      int PSU::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int PSU::Receive( Message* )
{
   return 0;
}


 

/*************************************************************************/
/**  \fn      int PSU::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int PSU::Receive( Event event )
{	
	//if event == EVENT_CLEAR
	
	psu_mains_failure 		 = false;
	battery_low  				 = false;
	battery_critical 			 = false;
	charger_failed    		 = false;
	no_battery					 = false;
	battery_resistance_fault = false;
	over_voltage				 = false;
	battery_not_detected		= false;
	
	return 0;
}


/*************************************************************************/
/**  \fn      int PSU::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int PSU::Receive( Command* cmd )
{
	assert( cmd != NULL );

	if ( !priority )
	{
		return CMD_ERR_UNKNOWN_CMD;
	}
	
	switch ( cmd->type )
	{
		case CMD_GET_PSU_STATUS :
		{
			BatteryStatus batt = BATT_GOOD;
			PSUPacket* p = prevbuffer; 
			PSU_TYPE psu_type = PSU_TYPE_CTEC;
			
			cmd->uint3  = p->timestamp;
			cmd->ushort0 = p->temp( );
			cmd->short1 = p->battery_charge( );
			cmd->short2 = p->battery_on_load( );
			cmd->short3 = p->battery_no_load( );
			cmd->short4 = p->battery_resistance( );	

			if ( p->psu_sb_charger_failed ) 
			{
				batt = BATT_CHARGE_FAIL;
			}
			else if ( p->psu_sb_battery_resistance_fault )
			{
				batt = BATT_HIGHQ;
			}
			else if ( p->psu_sb_battery_critical )
			{
				batt = BATT_CRIT;
			}
			else if ( p->psu_sb_battery_low )
			{
				batt = BATT_LOW;
			}
			//cmd->short5 = (short) batt;
			//cmd->short6 = (short)psu_type;
			
			cmd->charA = (char) batt;
			cmd->charB = (char)psu_type;

			return CMD_OK;
		}
		
		default: return CMD_ERR_UNKNOWN_CMD;
	}
}	


int PSU::ProcessPacket( PSUPacket* buffer )
{
	// if OK
	if ( !CheckSum( buffer ) && buffer->start == PSU_START_OF_HEADER && buffer->end == PSU_END_OF_TX )
	{
		if ( buffer->psu_sb_mains_failure )
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
		
		if ( buffer->psu_sb_battery_critical )
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
			
			if ( buffer->psu_sb_battery_critical ) Fault::AddFault( FAULT_BATTERY_CRITICAL );			
		}
				
		if ( buffer->psu_sb_battery_critical )
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
				int batt_charge = buffer->battery_charge();
				
				if ( batt_charge < app.site->batt_panel )
				{
					Fault::AddFault( FAULT_BATTERY_LOW);
				}
			}
		}
		
		if ( buffer->psu_sb_charger_failed )
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
		
		if ( buffer->psu_sb_no_battery )
		{
			if ( !no_battery )
			{
				Fault::AddFault( FAULT_PSU_CONFIG_ERROR );
				no_battery = true;
			}
		}
		else if ( no_battery )
		{
			Fault::RemoveFault( FAULT_PSU_CONFIG_ERROR );
			no_battery = false;
		}
		
		if ( buffer->psu_sb_battery_resistance_fault )
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
		
		if ( buffer->psu_sb_over_voltage )
		{
			if ( !over_voltage )
			{
				Fault::AddFault( FAULT_PSU_OVER_VOLTAGE );
				over_voltage = true;
			}
		}
		else if ( over_voltage )
		{
			Fault::RemoveFault( FAULT_PSU_OVER_VOLTAGE );
			over_voltage = false;
		}
		
	 
		if ( app.site != NULL )
		{
			int batt_charge = buffer->battery_charge();
			
			if ( batt_charge < app.site->batt_panel )
			{
				if ( !battery_low && !battery_critical && !battery_not_detected)
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

		psu_last_received = buffer->timestamp = now( );
			
		return true;
	}
	
	return false;
}
