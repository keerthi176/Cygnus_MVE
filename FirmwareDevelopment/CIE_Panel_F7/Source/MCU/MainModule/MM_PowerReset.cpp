/***************************************************************************
* File name: MM_PowerReset.cpp
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
* Panel settings class. Stores network, mesh, etc
*
**************************************************************************/

#include <assert.h>


/* System Include Files
**************************************************************************/
#include "stm32f7xx_hal_pwr.h"
#include "stm32f7xx_hal_flash_ex.h"
#include "iwdg.h"

/* User Include Files
**************************************************************************/
#include "MM_PowerReset.h"
#include "MM_Fault.h"
#include "DM_BatteryRam.h"
#include "MM_Log.h"

/* Globals
**************************************************************************/
extern int __rcc_csr;


PowerReset::PowerReset( ) : Module( "PowerReset", 0, EVENT_CLEAR ), dogging( false ) 
{	
	HAL_IWDG_Refresh( app.iwdg );	

	AllLEDs( GPIO_PIN_SET );	
	HAL_Delay( 750 );
	AllLEDs( GPIO_PIN_RESET );	
			
	app.DebOut( "Reset Flags = 0x%X.\n", __rcc_csr );
	
	if ( __rcc_csr & RCC_CSR_IWDGRSTF )  
	{
		( (int*)BKPSRAM_BASE)[ BATT_RAM_WATCHDOG_FAULT] = 0x55555555;
		app.DebOut( "Was watchdog.\n" );		
	}
	if ( __rcc_csr & RCC_CSR_PINRSTF ) 
	{
		app.DebOut( "Pin reset.\n" );
		GPIOManager::Set( GPIO_FireRoutingActive, IO_PIN_SET );
	}		
	if ( __rcc_csr & RCC_CSR_BORRSTF ) 
	{
		app.DebOut( "Brown one.\n" );
		GPIOManager::Set( GPIO_PSUFault, IO_PIN_SET );
	}				
	if ( __rcc_csr & RCC_CSR_SFTRSTF ) 
	{
		app.DebOut( "Soft reset.\n" );
		GPIOManager::Set( GPIO_FireRoutingFault, IO_PIN_SET );
	}				
	if ( __rcc_csr & RCC_CSR_LPWRRSTF ) 
	{
		app.DebOut( "Low power reset.\n" );
		GPIOManager::Set( GPIO_DelaysActive, IO_PIN_SET );
	}
	 
	
	if ( ( (int*)BKPSRAM_BASE)[ BATT_RAM_CRYSTAL_FAULT] == 0x55555555 )
	{
		GPIOManager::Set( GPIO_SysFault, IO_PIN_SET );
		GPIOManager::Set( GPIO_GenFault, IO_PIN_SET );	
					 
		Fault::AddFault( FAULT_CRYSTAL_FAILURE ); 
	__rcc_csr |= 1 << 20;
	}
	else
	{
		__rcc_csr &= ~(1 << 20);
	}
	
	if ( ( (int*)BKPSRAM_BASE)[ BATT_RAM_NCU_FAULT] == 0x55555555 )
	{
		GPIOManager::Set( GPIO_SysFault, IO_PIN_SET );
		GPIOManager::Set( GPIO_GenFault, IO_PIN_SET );	
					 
		Fault::AddFault( FAULT_NO_NCU_COMMS ); 
	}
	if ( ( (int*)BKPSRAM_BASE)[ BATT_RAM_WATCHDOG_FAULT] == 0x55555555 )  
	{
		GPIOManager::Set( GPIO_SysFault, IO_PIN_SET );
		GPIOManager::Set( GPIO_GenFault, IO_PIN_SET );		
				
		Fault::AddFault( FAULT_WATCHDOG );
	}

	// Reset CSR	
	RCC->CSR = 0xFF000000;

	PWR_PVDTypeDef pvd;
	
	pvd.PVDLevel = PWR_PVDLEVEL_4;
	pvd.Mode 	 = PWR_PVD_MODE_NORMAL;
	
	HAL_PWR_ConfigPVD( &pvd );	
	HAL_PWR_EnablePVD( );
	
	HAL_NVIC_EnableIRQ( (IRQn_Type) (PVD_IRQn) );

	HAL_Delay( 750 );
		
	GPIOManager::Set( GPIO_SounderFault, IO_PIN_RESET );
	GPIOManager::Set( GPIO_FireRoutingActive, IO_PIN_RESET );
	GPIOManager::Set( GPIO_PSUFault, IO_PIN_RESET );
	GPIOManager::Set( GPIO_FireRoutingFault, IO_PIN_RESET );
	GPIOManager::Set( GPIO_DelaysActive, IO_PIN_RESET );
	
	HAL_IWDG_Refresh( app.iwdg ); 		
}	



/*************************************************************************/
/**  \fn      int PowerReset::Init( )
***  \brief   Class function
**************************************************************************/
	

int PowerReset::Init( )
{
	if ( __rcc_csr & RCC_CSR_IWDGRSTF )  
	{
		switch ( ( (int*)BKPSRAM_BASE)[ BATT_RAM_WATCHDOG_REASON ] )
		{
			case WD_REASON_BUS_FAULT   : Log::Msg( LOG_SYS, "Watchdog Reset due to bus fault." );	break;
			case WD_REASON_HARD_FAULT  : Log::Msg( LOG_SYS, "Watchdog Reset due to hard fault." );	break;
			case WD_REASON_USAGE_FAULT : Log::Msg( LOG_SYS, "Watchdog Reset due to usage fault." );	break;
			case WD_REASON_MEM_FAULT   : Log::Msg( LOG_SYS, "Watchdog Reset due to mem fault." );	break;
			case WD_REASON_UNKNOWN     : Log::Msg( LOG_SYS, "Watchdog Reset (unknown)." );	break;
		}
	}
	if ( __rcc_csr & RCC_CSR_PINRSTF ) 
	{
		Log::Msg( LOG_SYS, "Pin reset." );	
	}		
	if ( __rcc_csr & RCC_CSR_BORRSTF ) 
	{
		Log::Msg( LOG_SYS, "Reset due to brown out." );	
	}				
	if ( __rcc_csr & RCC_CSR_SFTRSTF ) 
	{
		if ( __rcc_csr & ( 1 << 20 ) )
		{
			Log::Msg( LOG_SYS, "Self reset due to crystal failure." );	
		}
		else
		{
		 Log::Msg( LOG_SYS, "Reset due to software self reset." );	
		}
	}				
	if ( __rcc_csr & RCC_CSR_LPWRRSTF ) 
	{
		Log::Msg( LOG_SYS, "Reset due to low power." );	
	}
	  
  ((int*)BKPSRAM_BASE)[ BATT_RAM_WATCHDOG_REASON ] = WD_REASON_UNKNOWN;
	 
	return true;
}


/*************************************************************************/
/**  \fn      int PowerReset::Receive( Message* msg )
***  \brief   Class function
**************************************************************************/
	
int PowerReset::Receive( Message* msg )
{
	switch( msg->type )
	{
	
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int PowerReset::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int PowerReset::Receive( Command* cmd ) 
{
	assert( cmd != NULL );
	
	switch ( cmd->type )
	{
		case CMD_KILL_DOG:
		{
			while( 1) ; 	// infinite loop till watchdog reset
		}						
	}
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int PowerReset::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int PowerReset::Receive( Event event ) 
{
	// Using shared I2C so handle outside irq
	
	switch ( event )
	{
		case EVENT_CLEAR:
		{
			((int*)BKPSRAM_BASE)[ BATT_RAM_CRYSTAL_FAULT] = 0; // clear crystal fault
			((int*)BKPSRAM_BASE)[ BATT_RAM_WATCHDOG_FAULT] = 0; // clear WD fault
			((int*)BKPSRAM_BASE)[ BATT_RAM_NCU_FAULT] = 0; // clear NCU fault
			
			
			__rcc_csr &= ~RCC_CSR_IWDGRSTF; // clear watchdog fault
			break;
		}

	}
	return 0;
}



/*************************************************************************/
/**  \fn      int PowerReset::SelfTest
***  \brief   Class function
**************************************************************************/

void PowerReset::AllLEDs( GPIO_PinState state )
{
	HAL_GPIO_WritePin( BUZZER_EN_GPIO_Port, BUZZER_EN_Pin, state );
	
	GPIOManager::Set( GPIO_SounderFault, state );
	GPIOManager::Set( GPIO_FireRoutingActive, state );
	GPIOManager::Set( GPIO_PSUFault, state );
	GPIOManager::Set( GPIO_FireRoutingFault, state );
	GPIOManager::Set( GPIO_DelaysActive, state );
	GPIOManager::Set( GPIO_TestMode, state );
	GPIOManager::Set( GPIO_OtherEvents, state );
	GPIOManager::Set( GPIO_Disablements, state );	
	GPIOManager::Set( GPIO_Fire, state );
	GPIOManager::Set( GPIO_GenFault, state );
	GPIOManager::Set( GPIO_SysFault, state );
	
	for( int zoneboard = 0; zoneboard < MAX_ZONE_BOARDS; zoneboard++ )
	{
		if ( HAL_I2C_IsDeviceReady( &hi2c2, zonedev[ zoneboard ], 3, 200 ) == HAL_OK )
		{
			GPIOManager::SetZones( zoneboard, 0xFFFF, state ); 
		}
	}
}	 


extern "C" void HAL_PWR_PVDCallback(void)
{
	app.DebOut( "Power failing...\n" );
}