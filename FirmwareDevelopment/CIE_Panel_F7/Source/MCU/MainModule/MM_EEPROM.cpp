/***************************************************************************
* File name: MM_EEPROM.cpp
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
* EEPROM Manager
*
**************************************************************************/

/* System Include Files
**************************************************************************/
#include <stm32f7xx_hal.h>


/* Defines
**************************************************************************/



/* User Include Files
**************************************************************************/

#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_log.h"
#include "MM_fault.h"
#include "MM_Settings.h"


/* Defines
**************************************************************************/
#define EEPROM_PAGESIZE 		32
#define EEPROM_ADDRESS_SIZE   I2C_MEMADD_SIZE_16BIT
#define EEPROM_ADDRESS     	0xA0
#define EEPROM_SIZE        	8*1024
#define EEPROM_TIMEOUT			140

  


/* Globals
**************************************************************************/

EEPROMMap EEPROM::defaults  = {					"Panel Name",
																	0,0,
																		 0   ,
                                                   	-1   ,
	
																		40,
																		240,
																	  1234,
	
																	 0x62,
                                                    0x99,
                                                   0xFB9,
                                                   0xEAB,
															 
																		500,
																		300,
																		400, 
																		250,
																		0,
																		0,
	//"CYGNUS10001", "d8022023c9", "global", "cygnus-iot-liv.azure-devices.net",
	"CYGNUS10001", "d8022023c9", "global", "",
	96
	
};
																	
 

int 	EEPROM::device = EEPROM_ADDRESS; 


/*************************************************************************/
/**  \fn      EEPROM::GetCurrent( EEPROMMap* dump )
***  \brief   class function
**************************************************************************/

void EEPROM::GetCurrent( EEPROMMap* dump )
{  
	Read( (uint8_t*) dump, sizeof(EEPROMMap), 0 );
	memset(dump->AppSupp_URL,0,60);
}


/*************************************************************************/
/**  \fn      EEPROM::EEPROM( )
***  \brief   Constructor for class
**************************************************************************/

EEPROM::EEPROM( ) : Module( "EEPROM", 0 )
{	
	assert( app.FindModule( "QuadSPI" ) != NULL );
	
	EEPROMMap dump;

	dump.TouchXmax = 0;
	dump.TouchYmin = 0;
	
	GetCurrent( &dump ); 
	
	int button = GPIOManager::Get( GPIO_SpecialButton );
	
	if ( !button || ( dump.TouchXmin == -1 && dump.TouchXmax == -1 ) || ( dump.TouchYmin == -1 && dump.TouchXmax == -1 ) )
	{
		// Create defaults
		
		app.DebOut( "Resetting EEPROM...\n" );
		SetDefaults( false );
		
		while ( !button )
		{
			button = GPIOManager::Get( GPIO_SpecialButton );
		// wait for release
		}
		
		//reset
		HAL_NVIC_SystemReset( );
	}
	 
	app.DebOut( "ActiveBrightness           %d\n", dump.ActiveBrightness);
	app.DebOut( "InactiveBrightness         %d\n", dump.InactiveBrightness);
	app.DebOut( "BatteryActiveBrightness    %d\n", dump.BatteryActiveBrightness);
	app.DebOut( "BatteryInactiveBrightness  %d\n", dump.BatteryInactiveBrightness);
	app.DebOut( "CurrentSiteAddress         %d\n", dump.CurrentSiteAddress);
	app.DebOut( "PreviousSiteAddress        %d\n", dump.PreviousSiteAddress);
	 
	app.DebOut( "TouchXmin                  %d\n", dump.TouchXmin);
	app.DebOut( "TouchYmin                  %d\n", dump.TouchYmin);
	app.DebOut( "TouchXmax                  %d\n", dump.TouchXmax);
	app.DebOut( "TouchYmax                  %d\n", dump.TouchYmax);

	app.DebOut( "GSM MMC                    %d\n", dump.GSM_MCC);
	app.DebOut( "GSM MNC                    %d\n", dump.GSM_MNC);
	app.DebOut( "GSM PIN                    %d\n", dump.GSM_PIN);
		
	app.DebOut( "SMS_Enabled           		  %d\n", dump.SMS_Enabled);
	app.DebOut( "AppSupp_Enabled          	%d\n", dump.AppSupp_Enabled);
	app.DebOut( "PanelName                  %s\n", dump.PanelName);

	app.DebOut( "Checksum valid             %d\n", dump.checksumValid);
	app.DebOut( "Checksum value             %d\n", dump.checksumValue);
 
	app.DebOut( "AppSupp SiteId             %s\n", dump.AppSupp_SiteId);
	app.DebOut( "AppSupp Password           %s\n", dump.AppSupp_Password); 
	app.DebOut( "AppSupp APN                %s\n", dump.AppSupp_APN ); 
	
	app.DebOut( "AppSupp URL                %s\n", dump.AppSupp_URL ); 
}


/*************************************************************************/
/**  \fn      EEPROM::Read( uint8_t* value, int size, int address )
***  \brief   Any size read
**************************************************************************/

int EEPROM::Read( uint8_t* value, int size, int address )
{
	if ( HAL_OK == HAL_I2C_IsDeviceReady( app.touch_i2c, device, 5, EEPROM_TIMEOUT ) )
	{
		int n = 0;
		
		int offset = address & ( EEPROM_PAGESIZE - 1 );
			
		if ( offset > 0 )
		{
			int startsize = EEPROM_PAGESIZE - offset;
			
			if ( startsize > size ) startsize = size;
			
			if ( !ReadPartial( value, startsize, address ) ) return false;
			
			address += startsize;
			size  -= startsize;
			value += startsize;
		}
		
		for( n = 0; n <= size - EEPROM_PAGESIZE; n += EEPROM_PAGESIZE )
		{			
			if ( !ReadPartial( value, EEPROM_PAGESIZE, address ) ) return false;
			address += EEPROM_PAGESIZE;
			value += EEPROM_PAGESIZE;
		}
				
		size = size & ( EEPROM_PAGESIZE - 1 );
		
		if (size > 0 )
		{
			if ( !ReadPartial( value, size, address ) ) return false;
		}
		
		
		if ( size > 16 )
		{
			// assume string - stop returning FFFFFFFFFF..
			if ( *value == 0xFF )
			{
				// if unwritten terminate string
				*value = 0; 
			}
		}
		return true;
	}	
	return false;
}

/*************************************************************************/
/**  \fn      int EEPROM::ReadPartial( uint8_t* p, int size, int address )
***  \brief   Class function
**************************************************************************/

int EEPROM::ReadPartial( uint8_t* p, int size, int address )
{
	if ( HAL_I2C_Mem_Read( app.touch_i2c, device, address, EEPROM_ADDRESS_SIZE, (uint8_t*) p, size, EEPROM_TIMEOUT ) != HAL_OK )
	{	
		if ( HAL_I2C_Mem_Read( app.touch_i2c, device, address, EEPROM_ADDRESS_SIZE, (uint8_t*) p, size, EEPROM_TIMEOUT ) != HAL_OK )
		{	
			Fault::AddFault( FAULT_NO_I2C1_COMMS );
			return false;
		}
	}
		
	return true;
}


/*************************************************************************/
/**  \fn      int EEPROM::WritePartial( uint8_t* value, int size, int address )
***  \brief   Class function
**************************************************************************/

int EEPROM::WritePartial( uint8_t* value, int size, int address )
{
	if ( HAL_OK == HAL_I2C_Mem_Write( app.touch_i2c, device, address, EEPROM_ADDRESS_SIZE, value, size, EEPROM_TIMEOUT ) )
	{
		uint8_t loaded[ EEPROM_PAGESIZE ];
		
		HAL_Delay( 10 );
					
		if ( HAL_OK == HAL_I2C_Mem_Read( app.touch_i2c, device, address, EEPROM_ADDRESS_SIZE, loaded, size, EEPROM_TIMEOUT ) )
		{
			if ( !memcmp( loaded, value, size ) )
			{				
				return true;
			}
			else
			{
				Fault::AddFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "01_EEPROM.cpp -- ADD");
			}					
		}
		else
		{
			Fault::AddFault( FAULT_NO_I2C1_COMMS );
		}
	}
	else if ( HAL_OK == HAL_I2C_Mem_Write( app.touch_i2c, device, address, EEPROM_ADDRESS_SIZE, value, size, EEPROM_TIMEOUT ) )
	{
		uint8_t loaded[ EEPROM_PAGESIZE ];
		
		HAL_Delay( 10 );
					
		if ( HAL_OK == HAL_I2C_Mem_Read( app.touch_i2c, device, address, EEPROM_ADDRESS_SIZE, loaded, size, EEPROM_TIMEOUT ) )
		{
			if ( !memcmp( loaded, value, EEPROM_PAGESIZE ) )
			{				
				return true;
			}
			else
			{
				Fault::AddFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "02_EEPROM.cpp -- ADD");
			}					
		}
		else
		{
			Fault::AddFault( FAULT_NO_I2C1_COMMS );
		}
	}
	else
	{
		Fault::AddFault( FAULT_NO_I2C1_COMMS );
	}
	
	return false;
}


/*************************************************************************/
/**  \fn      int EEPROM::Write( uint8_t* value, int size, int address )
***  \brief   Class function
**************************************************************************/

int EEPROM::Write( uint8_t* value, int size, int address )
{
	if ( HAL_OK == HAL_I2C_IsDeviceReady( app.touch_i2c, device, 5, EEPROM_TIMEOUT ) )
	{
		do
		{
			// Unlock
			GPIOManager::Set( GPIO_EEPROMEnable, GPIO_PIN_RESET );
			HAL_Delay( 3 );
			
			int n = 0;
			
			int offset = address & ( EEPROM_PAGESIZE - 1 );
				
			if ( offset > 0 )
			{
				int startsize = EEPROM_PAGESIZE - offset;
				
				if ( startsize > size ) startsize = size;
				
				if ( !WritePartial( value, startsize, address ) ) break;
				
				address += startsize;
				size -= startsize;
				value += startsize;
			}
			
			for( n = 0; n <= size - EEPROM_PAGESIZE; n += EEPROM_PAGESIZE )
			{			
				if ( !WritePartial( value, EEPROM_PAGESIZE, address ) ) break;
				address += EEPROM_PAGESIZE;
				value += EEPROM_PAGESIZE;
			}
					
			size = size & ( EEPROM_PAGESIZE - 1 );
			
			if ( size > 0 )
			{
				if ( !WritePartial( value, size, address ) ) break;
			}
			return true;
		} while ( 0 );
		// Lock
		GPIOManager::Set( GPIO_EEPROMEnable, GPIO_PIN_SET );
	}	
	return false;
}


/*************************************************************************/
/**  \fn      int EEPROM::SetDefaults( int factory_reset )
***  \brief   Class function
**************************************************************************/

int EEPROM::SetDefaults( int factory_reset )
{
	// Create defaults
	SetVar(ActiveBrightness, 			defaults.ActiveBrightness);
	SetVar(InactiveBrightness, 		defaults.InactiveBrightness);
	SetVar(BatteryActiveBrightness, 	defaults.BatteryActiveBrightness);
	SetVar(BatteryInactiveBrightness,defaults.BatteryInactiveBrightness);

	SetVar(PreviousSiteAddress, 		defaults.PreviousSiteAddress);
	SetVar(TouchXmin, 					defaults.TouchXmin);
	SetVar(TouchYmin, 					defaults.TouchYmin);
	SetVar(TouchXmax, 					defaults.TouchXmax);
	SetVar(TouchYmax, 					defaults.TouchYmax);
	
	SetVar(GSM_MNC, 						defaults.GSM_MNC);
	SetVar(GSM_MCC, 						defaults.GSM_MCC);
	SetVar(GSM_PIN, 						defaults.GSM_PIN);
	
	SetVar(SMS_Enabled, 					defaults.SMS_Enabled);
	SetVar(AppSupp_Enabled, 			defaults.AppSupp_Enabled);
	SetVar(PanelName,						defaults.PanelName);  
	
	SetVar(AppSupp_URL, 					defaults.AppSupp_URL );
	SetVar(AppSupp_APN, 					defaults.AppSupp_APN);
	
	if ( factory_reset )
	{
		SetVar(AppSupp_Password, 		defaults.AppSupp_Password );
		SetVar(AppSupp_SiteId, 			defaults.AppSupp_SiteId );		
	}	
	
	int CurrentSiteAddress;
	
	// Get site address
	GetVar(CurrentSiteAddress, CurrentSiteAddress);
	
	// if address valid
	if ( CurrentSiteAddress == 0 || CurrentSiteAddress == SITE_ALLOCATION_SIZE )
	{			
		// check site
		if ( Settings::CheckSite( (Site*) ( SETTINGS_ROM_ADDRESS + CurrentSiteAddress ) ) )
		{
			// if ok dont change
			return true;
		}
	}
	
	// 
	if ( Settings::CheckSite( (Site*) ( SETTINGS_ROM_ADDRESS + SITE_ALLOCATION_SIZE ) ) )
	{
		CurrentSiteAddress = SITE_ALLOCATION_SIZE;			
	}
	else
	{
		CurrentSiteAddress = 0;
	}	
	
	SetVar(CurrentSiteAddress, CurrentSiteAddress);
	
	
	return true;
}
	


/*************************************************************************/
/**  \fn      int IOStatus::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int EEPROM::Receive( Message* )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int EEPROM::Receive( Event )
***  \brief   Class function
**************************************************************************/

int EEPROM::Receive( Event )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int EEPROM::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int EEPROM::Receive( Command* cmd )
{
	if ( cmd->type == CMD_RESET_EEPROM )
	{
		if ( SetDefaults( false ) )
		{
#ifndef BOOT_MENU	
#ifndef BD_TEST_ONLY				
			AppDeviceClass_TriggerEndProgressEvent( app.DeviceObject, 100 );
#endif			
#else
			BootMenuDeviceClass_TriggerEndProgressEvent( app.DeviceObject, 100 );
#endif			
			return CMD_OK;
		}
		else
		{
#ifndef BOOT_MENU			
#ifndef BD_TEST_ONLY				
			AppDeviceClass_TriggerEndProgressEvent( app.DeviceObject, -1 );
#endif			
#else
			BootMenuDeviceClass_TriggerEndProgressEvent( app.DeviceObject, -1 );
#endif				
			return CMD_ERR_DEVICE_BUSY;
		}
	}
	return CMD_ERR_UNKNOWN_CMD;
}



