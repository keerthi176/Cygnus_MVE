/***************************************************************************
* File name: MM_Settings.cpp
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


/* System Include Files
**************************************************************************/
#include <assert.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>


/* User Include Files
**************************************************************************/
#include "MM_Settings.h"

#include "MM_USBSerial.h"
#include "MM_QuadSPI.h"
#include "MM_Command2.h"
#include "MM_PanelIO.h"
#include "MM_MemCheck.h"

#include "MM_EEPROM.h"
#include "MM_Fault.h" 
#include "MM_NCU.h"
#include "CO_Site.h"
#include "CO_Device.h"
#include "MM_issue.h"
#include "MM_LED.h"
#include "MM_Log.h"
#include "MM_RadioDevice.h"
#include "usbh_ftdi.h"
#include "usbh_hub.h"
#include "MM_Utilities.h"


#ifdef GSM_HTTP
#include "MM_AppSupport.h"
#else
#include "MM_MQTTSupport.h"
#endif


/* Defines
**************************************************************************/

#define CHANGED( x ) (prev_settings & (x)) != (cmd->int1 & (x))

#define MAX_NO_CHANGE_CHECKS					250000000

#define ITEMS(x) (sizeof(x)/sizeof(*x))
	
 
 
/* Globals
**************************************************************************/

int Settings::access_level = 1;

static uint32_t combination[] = 
{
   0,
   DEV_SMOKE_DETECTOR | DEV_INDICATOR_LED,
   DEV_HEAT_DETECTOR_TYPE_A1R | DEV_INDICATOR_LED,
   DEV_HEAT_DETECTOR_TYPE_B | DEV_INDICATOR_LED,
   DEV_SMOKE_DETECTOR | DEV_HEAT_DETECTOR_TYPE_A1R | DEV_INDICATOR_LED,
   DEV_SMOKE_DETECTOR | DEV_PIR_DETECTOR | DEV_INDICATOR_LED,                                                                                                  // 5
   DEV_BEACON_W_2_4_7_5,
   DEV_BEACON_W_3_1_11_3,
   DEV_BEACON_C_3_8_9,
   DEV_BEACON_C_3_15,
   DEV_REMOTE_INDICATOR,                                                                                                                                        // 10
   DEV_SOUNDER | DEV_INDICATOR_LED,
   DEV_SOUNDER | DEV_SMOKE_DETECTOR | DEV_INDICATOR_LED,
   DEV_SOUNDER | DEV_HEAT_DETECTOR_TYPE_A1R | DEV_INDICATOR_LED,
   DEV_SOUNDER | DEV_HEAT_DETECTOR_TYPE_B | DEV_INDICATOR_LED,
   DEV_SOUNDER | DEV_SMOKE_DETECTOR | DEV_HEAT_DETECTOR_TYPE_A1R | DEV_INDICATOR_LED,                                                                           //15
   DEV_SOUNDER | DEV_SMOKE_DETECTOR | DEV_PIR_DETECTOR | DEV_INDICATOR_LED,
   DEV_SOUNDER | DEV_BEACON_W_2_4_7_5,
   DEV_SOUNDER | DEV_BEACON_W_3_1_11_3,
   DEV_SOUNDER | DEV_BEACON_C_3_8_9,
   DEV_SOUNDER | DEV_BEACON_C_3_15,                                                                                                        //20
   DEV_SOUNDER_VI | DEV_VISUAL_INDICATOR,
   DEV_SOUNDER_VI | DEV_VISUAL_INDICATOR | DEV_SMOKE_DETECTOR | DEV_INDICATOR_LED,
   DEV_SOUNDER_VI | DEV_VISUAL_INDICATOR | DEV_HEAT_DETECTOR_TYPE_A1R | DEV_INDICATOR_LED,
   DEV_SOUNDER_VI | DEV_VISUAL_INDICATOR | DEV_HEAT_DETECTOR_TYPE_B | DEV_INDICATOR_LED,
   DEV_SOUNDER_VI | DEV_VISUAL_INDICATOR | DEV_SMOKE_DETECTOR | DEV_HEAT_DETECTOR_TYPE_A1R | DEV_INDICATOR_LED,                                                //25
   DEV_SOUNDER_VI | DEV_VISUAL_INDICATOR | DEV_SMOKE_DETECTOR | DEV_PIR_DETECTOR | DEV_INDICATOR_LED,
  DEV_FIRE_CALL_POINT,
   DEV_IO_UNIT_INPUT_1 | DEV_IO_UNIT_OUTPUT_1 | DEV_IO_UNIT_INPUT_2 | DEV_IO_UNIT_OUTPUT_2,
   DEV_FIRE_CALL_POINT | DEV_SOUNDER_85DBA,                                                     // Construction Site Fire combination 1
   DEV_FIRE_CALL_POINT | DEV_SOUNDER_VI_CONSTR,                                                 // Construction Site Fire combination 2                        //30
   DEV_FIRE_CALL_POINT | DEV_PIR_DETECTOR | DEV_SOUNDER_VI_CONSTR,                              // Construction Site Fire combination 3
   DEV_FIRST_AID_CALL_POINT | DEV_SOUNDER_VI_CONSTR,                                            // Construction Site First Aid combination
   DEV_FIRE_CALL_POINT | DEV_FIRST_AID_CALL_POINT | DEV_SOUNDER_VI_CONSTR,                      // Construction Site Fire and First Aid combination 1
   DEV_FIRE_CALL_POINT | DEV_FIRST_AID_CALL_POINT | DEV_PIR_DETECTOR | DEV_SOUNDER_VI_CONSTR,   // Construction Site Fire and First Aid combination 2
   DEV_SMOKE_DETECTOR | DEV_HEAT_DETECTOR_TYPE_A1R | DEV_SOUNDER_85DBA,                                                                                        //35
   DEV_FIRE_CALL_POINT,
   DEV_FIRST_AID_CALL_POINT,
   DEV_FIRE_CALL_POINT | DEV_SOUNDER_85DBA | DEV_PIR_DETECTOR,
   DEV_FIRST_AID_CALL_POINT | DEV_SOUNDER_VI_CONSTR | DEV_PIR_DETECTOR,
   DEV_SMOKE_DETECTOR | DEV_HEAT_DETECTOR_TYPE_A1R | DEV_SOUNDER_85DBA | DEV_PIR_DETECTOR                                                                      //40
};


int Settings::current_pos = -1;
int Settings::rom_address = SETTINGS_ROM_ADDRESS;
int Settings::new_unit = -1;

char Settings::acu_key[ 33 ];
AcuCmd	Settings::acu_cmd;
int Settings::acu_id;
int Settings::acu_encrypt;

bool Settings::unit_sent = false;

bool IsOutput( ChannelType );

ATJob					Settings::job = JOB_NONE;
ATJob					Settings::next_job = JOB_NONE;

ATCmd            Settings::commands_buff[ ATCMD_MAX_COMMANDS ];
ATCmd*				Settings::cmd = NULL;
Circular<ATCmd> Settings::commands( commands_buff, ATCMD_MAX_COMMANDS );

int Settings::reply_ok;
char Settings::reply[ 8];
bool Settings::at_errors = false;
DeviceConfig Settings::new_device;

extern USBH_HandleTypeDef hUSBHost[ MAX_HUB_PORTS + 1];


/*************************************************************************/
/**  \fn      static void SendError( const char* fmt, ... )
***  \brief   Helper func to send error string to GUI
**************************************************************************/

static void SendError( const char* fmt, ... )
{
	char buff[ 128];
	
	va_list args;
	
	va_start( args, fmt );
	
	vsnprintf( buff, 128, fmt, args );
	
	va_end( args );

	AppDeviceClass_TriggerRBUProgressEvent( app.DeviceObject, -1, (int) buff ); 
}
 
 

/*************************************************************************/
/**  \fn      Settings::Settings( )
***  \brief   Constructor for class
**************************************************************************/

Settings::Settings( ) : Module( "Settings", 20, EVENT_RESET_BUTTON | EVENT_FTDI_ACTIVE | EVENT_FTDI_REMOVED | EVENT_DAY_TIME | EVENT_NIGHT_TIME)
{	
	// No site 
	current_pos = -1;
	
	assert( SITE_ALLOCATION_SIZE > sizeof(Site) );
	
	job = JOB_NONE;
	
	acu_connected = false;
}


/*************************************************************************/
/**  \fn      int Settings::Init( )
***  \brief   Class function
**************************************************************************/

int Settings::Init( )
{
	assert( ( gpio     = (GPIOManager*) 	app.FindModule( "GPIOManager" ) ) );
	assert( ( iostatus = (IOStatus*)	  		app.FindModule( "IOStatus" ) ) );
	assert( ( ew		 = (EmbeddedWizard*)	app.FindModule( "EmbeddedWizard" ) ) );	
	assert( ( quadspi  = (QuadSPI*)			app.FindModule( "QuadSPI" ) ) );	
	assert( ( ncu 		 = (NCU*) 				app.FindModule( "NCU" ) ) );

	current_user = -1;
	
	if ( !LoadSettings( ) )
	{
		current_pos = -1;
	}

	factory_resetting = false;	
	
	Send( MSG_INIT_SITE, this, current_pos );
	
	return true;
}


/*************************************************************************/
/**  \fn      int Settings::CheckDelayStatus( )
***  \brief   Class function
**************************************************************************/

int Settings::CheckDelayStatus( )
{
	if ( app.panel != NULL )
	{
		if ( app.panel->settings & SITE_SETTING_DISABLE_OUTPUT_DELAYS )
		{
			LED::Off( GPIO_DelaysActive );
			return false;
		}
	
		for( DeviceConfig* dc = app.panel->devs; dc < app.panel->devs + app.panel->NumDevs( ); dc++ )
		{
			for( int ch = 0; ch < dc->numOutputs; ch++ )
			{
				if ( dc->InitialDelay( ch ) > 0 )
				{
					LED::On( GPIO_DelaysActive );		
					return true;
				}	
			}
		}
	}	
		
	LED::Off( GPIO_DelaysActive );
	
	return false;
}


 
/*************************************************************************/
/**  \fn      int Settings::Init( )
***  \brief   Class function
**************************************************************************/

int Settings::ValidateSite( int offset, bool fault )
{		  
	Site* site = (Site*) (rom_address + offset) ;
			
	if ( CheckSite( site ) )
	{	
		Fault::RemoveFault( FAULT_BAD_SITE_FILE );		
		
		if ( SiteLegal( site ) )
		{
			Fault::RemoveFault( FAULT_ILLEGAL_SITE_FILE );				 										 					 
		
			app.DebOut( "Using site @ +0x%0X\n", site - rom_address );
			app.DebOut( "Old site @ 0x%0X\n", prev_pos );
				
			app.DebOut( "ROM address @ 0x%X\n", site );
		 
			app.site = (Site*) site;
			app.panel = app.site->panels + app.site->currentIndex;
			
			UpdateRoutingSounderLEDs( );
			
			return true;
		}
		else
		{
			if ( fault ) Fault::AddFault( FAULT_ILLEGAL_SITE_FILE );
		}
	}
	else
	{
		if ( fault ) Fault::AddFault( FAULT_BAD_SITE_FILE );
	}
	 			
	return false;
}


/*************************************************************************/
/**  \fn      int Settings::GetWriteAddress( )
***  \brief   Class function
**************************************************************************/

int Settings::GetWriteAddress( )
{
	if ( current_pos == SITE_ALLOCATION_SIZE )
	{
		return 0;
	}
	else
	{
		return SITE_ALLOCATION_SIZE;
	}
}


/*************************************************************************/
/**  \fn      int Settings::SetReadAddress( int addr )
***  \brief   Class function
**************************************************************************/

int Settings::SetReadAddress( int addr )
{	
	prev_pos = current_pos;
	
	if ( !EEPROM::SetVar( PreviousSiteAddress, prev_pos ) )
	{
		if ( !EEPROM::SetVar( PreviousSiteAddress, prev_pos ) )
		{
			Fault::AddFault( FAULT_SETTINGS_PREVIOUS_ADDR_SET );
			//Log::Msg(LOG_SYS, "01_Settings.cpp -- ADD");
		}
		else
		{
			Fault::RemoveFault( FAULT_SETTINGS_PREVIOUS_ADDR_SET );
			//Log::Msg(LOG_SYS, "02_Settings.cpp -- REMOVE");
		}
	}
	else
	{
		Fault::RemoveFault( FAULT_SETTINGS_PREVIOUS_ADDR_SET );
		//Log::Msg(LOG_SYS, "03_Settings.cpp -- REMOVE");
	}
	
	current_pos = addr;
	
	if ( !EEPROM::SetVar( CurrentSiteAddress, current_pos ) )
	{
		if ( !EEPROM::SetVar( CurrentSiteAddress, current_pos ) )
		{
			Fault::AddFault( FAULT_SETTINGS_CURRENT_ADDR_SET );
			//Log::Msg(LOG_SYS, "04_Settings.cpp -- ADD");
		}
		else
		{
			Fault::RemoveFault( FAULT_SETTINGS_PREVIOUS_ADDR_SET );
			//Log::Msg(LOG_SYS, "05_Settings.cpp -- REMOVE");
		}
	}
	else
	{
		Fault::RemoveFault( FAULT_SETTINGS_PREVIOUS_ADDR_SET );
		//Log::Msg(LOG_SYS, "06_Settings.cpp -- REMOVE");
	}
 
	return prev_pos;
}


/*************************************************************************/
/**  \fn      int Settings::LoadSettings( )
***  \brief   Class function
**************************************************************************/

int Settings::LoadSettings( )
{
	if ( EEPROM::GetVar( CurrentSiteAddress, current_pos ) &&
		  EEPROM::GetVar( PreviousSiteAddress, prev_pos )  )
	{		
		Fault::RemoveFault( FAULT_SETTINGS_CURRENT_ADDR_SET );
		Fault::RemoveFault( FAULT_SETTINGS_PREVIOUS_ADDR_SET );
		//Log::Msg(LOG_SYS, "07_Settings.cpp -- REMOVE");
		return true;		
	}
	 	
	Fault::AddFault( FAULT_SETTINGS_CURRENT_ADDR_SET );
	//Log::Msg(LOG_SYS, "08_Settings.cpp -- ADD");
			
	current_pos = 0;
	
	return false;					  
}	


/*************************************************************************/
/**  \fn      Panel* Settings::GetCurrentPanel( )
***  \brief   Class function
**************************************************************************/

Panel* Settings::GetCurrentPanel( )
{
	if ( current_pos != -1 )
	{
   	Site* site = (Site*)( rom_address + current_pos );
				
		if ( site->currentIndex < SITE_MAX_PANELS )
		{
			return site->panels + site->currentIndex;
		}
	}
	return NULL;
}


/*************************************************************************/
/**  \fn      Site* Settings::GetCurrentSite( )
***  \brief   Class function
**************************************************************************/
 
Site* Settings::GetCurrentSite( )
{
	if ( current_pos != -1 )
	{
 		Site* site = (Site*)( rom_address + current_pos );
	 
		return site;
	}
	return NULL;
}
 

/*************************************************************************/
/**  \fn      DeviceConfig* Settings::FindConfig( int channel_type )
***  \brief   Class function
**************************************************************************/

DeviceConfig* Settings::FindConfig( int unit )
{
	if ( app.panel != NULL )
	{
		for( DeviceConfig* d = app.panel->devs; d < app.panel->devs + app.panel->NumDevs(); d++ )
		{
			if ( unit == d->unit )
			{
				return d;
			}
		}
	}

	return NULL;
}


/*************************************************************************/
/**  \fn      DeviceConfig* Settings::FindFireRoutingDevice( DeviceConfig* start )
***  \brief   Class function
**************************************************************************/

DeviceConfig* Settings::FindFireRoutingDevice( DeviceConfig* start )
{
	if ( app.panel != NULL )
	{
		for( DeviceConfig* d = start; d < app.panel->devs + app.panel->NumDevs(); d++ )
		{
			for( int ch = 0; ch < d->numOutputs; ch++ )
			{
				if ( d->output[ ch ].type == CO_CHANNEL_PANEL_ROUTING_OUTPUT ||
  					  d->output[ ch ].type == CO_CHANNEL_OUTPUT_ROUTING_E )
				{
					return d;
				}
			}
		}
	}

	return NULL;
}


/*************************************************************************/
/**  \fn      DeviceConfig* Settings::FindType( int type )
***  \brief   Class function
**************************************************************************/

DeviceConfig* Settings::FindType( int type )
{
	if ( app.panel != NULL )
	{
		for( DeviceConfig* d = app.panel->devs; d < app.panel->devs + app.panel->NumDevs(); d++ )
		{
			if ( type == d->type )
			{
				return d;
			}
		}
	}

	return NULL;
}


/*************************************************************************/
/**  \fn      int Settings::WriteDevice( DeviceConfig* pold, DEVICE& newdev )
***  \brief   Class function
**************************************************************************/

int Settings::WriteDevice( DeviceConfig* p, DeviceConfig& dev )
{
	int flash_addr = (int)p  - rom_address;
	
	int size = sizeof( DeviceConfig );
	
	QSPI_Result r = quadspi->WriteToFlash( (char*) &dev, flash_addr, size );

	if ( r != QSPI_RESULT_OK && r != QSPI_RESULT_WRITE_PROTECTED )
	{
		Fault::AddFault( FAULT_NO_QSPI_COMMS );
	}
	else
	{
		UpdateRevision( );
		// Update IO
		Send( MSG_REFRESH_OUTPUTS, iostatus );
		
		// Update Site ( UI )
		AppDeviceClass_TriggerSiteChangedEvent( app.DeviceObject );
		
		// Update Delay status
		CheckDelayStatus( );				
	}
	
	return r;
}

 
/*************************************************************************/
/**  \fn      int LowestFreeUnit( )
***  \brief   Class function
**************************************************************************/

int LowestFreeUnit( )
{
	if ( app.panel != NULL )
	{
		if ( app.panel->numRadioDevs >= app.panel->maxDevices )
		{                                                                                                
			return INT_MAX;
		}
		
		int maxdevs = FACTORY_SETTINGS->devices;
		
		if ( maxdevs < 16 || maxdevs > 1024 ) maxdevs = 511;
		
		// check max devices
		if ( app.panel->numRadioDevs >= maxdevs )
		{
			return INT_MAX;
		}
					
		DeviceConfig* last = app.panel->devs + app.panel->NumDevs( );
		
		for( int unit = 1; unit <= SITE_MAX_RADIO_DEVICES; unit++ )
		{
			DeviceConfig* dc;
			
			for( dc = app.panel->devs; dc < last; dc++ )
			{
				 if ( dc->unit == unit ) break;
			}
			if ( dc == last )
			{	
				return unit;
			}
		}		
	}
	
	return INT_MAX;
}
 

/*************************************************************************/
/**  \fn      int Settings::GetChannelFeat( Command* cmd )
***  \brief   Class function
**************************************************************************/

ChannelType Settings::GetChannelFeat( DM_DeviceType_t dt )
{
	switch( dt )
	{
		case DEV_SMOKE_DETECTOR_E:				return CO_CHANNEL_SMOKE_E;
		case DEV_HEAT_DETECTOR_TYPE_A1R_E:	return CO_CHANNEL_HEAT_A1R_E;
		case DEV_HEAT_DETECTOR_TYPE_B_E:		return CO_CHANNEL_HEAT_B_E;
		case DEV_PIR_DETECTOR_E:				return CO_CHANNEL_PIR_E;
		case DEV_FIRE_CALL_POINT_E:			return CO_CHANNEL_FIRE_CALLPOINT_E;
		case DEV_FIRST_AID_CALL_POINT_E:		return CO_CHANNEL_MEDICAL_CALLPOINT_E;
		case DEV_SOUNDER_VI_E:
		case DEV_SOUNDER_85DBA_E:
		case DEV_SOUNDER_E:						return CO_CHANNEL_SOUNDER_E;
		case DEV_BEACON_W_2_4_7_5_E:
		case DEV_BEACON_W_3_1_11_3_E:
		case DEV_BEACON_C_3_8_9_E:
		case DEV_BEACON_C_3_15_E:				return CO_CHANNEL_BEACON_E;
		case DEV_VISUAL_INDICATOR_E:			return CO_CHANNEL_VISUAL_INDICATOR_E;   					        
//		case DEV_REMOTE_INDICATOR_E:
		case DEV_SOUNDER_VI_CONSTR_E:			return CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E;
		case DEV_INDICATOR_LED_E:      		return CO_CHANNEL_STATUS_INDICATOR_LED_E;
		case DEV_IO_UNIT_INPUT_1_E:			return CO_CHANNEL_INPUT_1_E;
		case DEV_IO_UNIT_OUTPUT_1_E:			return CO_CHANNEL_OUTPUT_1_E;
		case DEV_IO_UNIT_INPUT_2_E:   		return CO_CHANNEL_INPUT_2_E;
		case DEV_IO_UNIT_OUTPUT_2_E:			return CO_CHANNEL_OUTPUT_2_E;
	}
	return CO_CHANNEL_NONE_E;
}

/*************************************************************************/
/**  \fn      int Settings::NumZonesUsed( )
***  \brief   Class function
**************************************************************************/

/*
int Settings::NumZonesUsed( int panel )
{ 
	int numzones = 0;	
	
	if ( app.site != NULL )
	{
		unsigned int zbits[ SITE_MAX_ZONES / (8*sizeof(unsigned int)) ];
		memset( zbits, 0, sizeof( zbits ) );
		
		Panel* p = app.site->panels + panel ;

		for( DeviceConfig* d = p->devs; d < p->devs + p->NumDevs( ); d++ )
		{
			if ( !IFSET( d->zone, zbits ) )
			{
				numzones++;
				SET( d->zone, zbits );
			}
		}
	}
	return numzones;
}*/
	

	
int Settings::fire_routing_disabled;	
int Settings::alarms_disabled;
	
void Settings::UpdateRoutingSounderLEDs( )
{
	if ( app.panel != NULL )
	{		
		int settings = app.panel->settings;
		
		// nasty globla usage..
		fire_routing_disabled = false;
		alarms_disabled = false;
	 
		if ( settings & SITE_SETTING_FIRE_ALARMS_DISABLED )
		{
			alarms_disabled = true;
		}
		else
		{				
			// check individual devices
		
			for( DeviceConfig* dc = app.panel->devs; dc < app.panel->devs + app.panel->NumDevs( ); dc++ ) 
			{		
				Device* d = IOStatus::FindDevice( dc->unit );
				 
				for( int ch = 0; ch < dc->numOutputs; ch++ )
				{
					if ( d != NULL )
					{
						if ( d->IsOutputDisabled( ch ) )
						{
							if ( dc->IsAlarmOutputChannel( ch ) )
							{
								alarms_disabled = true;
								goto exit_loop;
							}
						}
					}
					else
					{
						if ( dc->IsOutputDisabled( ch ) )
						{
							if ( dc->IsAlarmOutputChannel( ch ) )
							{
								alarms_disabled = true;
								goto exit_loop;
							}
						}
					}
				}
			}
		}
	
exit_loop:
		
		if ( settings & ( SITE_SETTING_FIRE_ROUTING_DISABLED | SITE_SETTING_FIRE_ALARMS_DISABLED ) )
		{
			fire_routing_disabled = true;
		}
		else 			
		{
			// check individual channels
			
			bool allset = true;
			bool found = false;
			
			for( DeviceConfig* dc = app.panel->devs; dc < app.panel->devs + app.panel->NumDevs( ); dc++ ) 
			{
				for( OutputChannel* o = dc->output; o < dc->output + dc->numOutputs; o++ )
				{
					if ( o->type == CO_CHANNEL_OUTPUT_ROUTING_E || o->type == CO_CHANNEL_PANEL_ROUTING_OUTPUT )
					{
						Device* d = IOStatus::FindDevice( dc->unit );
						
						found = true;
						
						if ( d != NULL )
						{
							if ( !d->IsOutputDisabled( o - dc->output ) )
							{
								allset = false;
								break;
							}
						}
						else
						{
							if ( !dc->IsOutputDisabled( o - dc->output ) )
							{
								allset = false;
								break;
							}
						}
					}
				}
			} 
			
			if ( found && allset )
			{
				fire_routing_disabled = true;
			}
		}
	}
	
	if ( Fault::fire_routing_faulty ) 
	{
		LED::Flash( GPIO_FireRoutingFault );
	}
	else
	{
		if ( fire_routing_disabled )
		{
			LED::On( GPIO_FireRoutingFault );
		}
		else
		{
			 LED::Off( GPIO_FireRoutingFault );
		}
	}

	if ( Fault::sounder_faulty )
	{
		LED::Flash( GPIO_SounderFault );
	}
	else
	{
		if ( alarms_disabled )
		{
			LED::On( GPIO_SounderFault );
		}
		else
		{
			 LED::Off( GPIO_SounderFault );
		}
	}	
}
 		
		
/*************************************************************************/
/**  \fn      int Settings::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

 
extern uint8_t* buffpos;

#define SET( z, buff ) buff[ (z-1) / 32 ] |= 1 << ((z-1) & 31 );
#define IFSET( z, buff ) ((buff[ (z-1) / 32 ] >> ((z-1) & 31 )) & 1)


Zone* Settings::SiteZone( int zonenum )
{
	if ( app.panel != NULL )
	{
		for( Zone* zp = app.panel->zone; zp < app.panel->zone + app.panel->numDefinedZones; zp++ )
		{
			if ( zp->num == zonenum )
			{
				return zp;
			}
		}
	}
	return NULL;
}

int Settings::ZoneDisabled( int zone )
{
	if ( app.panel != NULL )
	{  
		for( Zone* zp = app.panel->zone; zp < app.panel->zone + app.panel->numDefinedZones; zp++ )
		{
			if ( zp->num == zone )
			{
				return zp->flags & ZONEFLAG_DISABLED;
			}
		}
	}
	return 0;
}	

	

int Settings::Receive( Command* cmd )
{
	if ( cmd->type == CMD_GET_ZONE_DISABLED )
	{
		if ( app.panel != NULL )
		{
			cmd->int1 = 0;
			
			if ( cmd->int0 > 0 && cmd->int0 <= SITE_MAX_ZONES ) 
			{
				for( Zone* zp = app.panel->zone; zp < app.panel->zone + app.panel->numDefinedZones; zp++ )
				{
					if ( zp->num == cmd->int0 )
					{
						cmd->int1 = zp->flags & ZONEFLAG_DISABLED;
						break;
					}
				}
			}
			else
			{
				int settings = app.panel->settings;
				
				if ( cmd->int0 == GROUP_FIRE_ALARMS )
				{
					cmd->int1 = settings & SITE_SETTING_FIRE_ALARMS_DISABLED;
				}
				else if ( cmd->int0 == GROUP_FIRE_ROUTING )
				{
					cmd->int1 = settings & SITE_SETTING_FIRE_ROUTING_DISABLED;
				}
				
			}	
			return CMD_OK;
		}
		return CMD_ERR_NO_SITE;
	}
		
	
	else if ( cmd->type == CMD_SET_ZONE_DISABLED )
	{
		if ( app.panel != NULL )
		{
	 		Send( MSG_REFRESH_OUTPUTS, iostatus );
			
			// if 'normal' zone
			if ( cmd->int0 > 0 && cmd->int0 <= SITE_MAX_ZONES ) 
			{
				for( Zone* zp = app.panel->zone; zp < app.panel->zone + app.panel->numDefinedZones; zp++ )
				{
					if ( zp->num == cmd->int0 )
					{
						char flags = zp->flags;
						
						if ( cmd->int1 )
						{
							ncu->QueueWriteMsg( true, NCU_BROADCAST_ENABLE_ZONE, this, NULL, cmd->int0, 0 );
							flags |= ZONEFLAG_DISABLED;
							Log::Msg( LOG_DIS, "Zone %d disabled.", cmd->int0 );
						}
						else
						{
							ncu->QueueWriteMsg( true, NCU_BROADCAST_ENABLE_ZONE, this, NULL, cmd->int0, 1 );
							flags &= ~ZONEFLAG_DISABLED;
							Log::Msg( LOG_DIS, "Zone %d enabled.", cmd->int0 );
						}
						int index = zp - app.panel->zone;
						
						if ( quadspi->WriteToPanel( flags, zone[ index ].flags ) == QSPI_RESULT_OK )
						{
							UpdateRevision( );
							UpdateRoutingSounderLEDs( );
							return CMD_OK;
						}
						else
						{
							return CMD_ERR_DEVICE_BUSY;
						}
					}
				}
			}
			else // special zone group ( i.e outputs / routing equipment
			{
				int settings = app.panel->settings;
				
				if ( cmd->int0 == GROUP_FIRE_ALARMS )
				{
					if ( cmd->int1 )
					{
						settings |= SITE_SETTING_FIRE_ALARMS_DISABLED;
					}
					else
					{
						settings &= ~SITE_SETTING_FIRE_ALARMS_DISABLED;
					}
				}
				
				if ( cmd->int0 == GROUP_FIRE_ROUTING )
				{
					if ( cmd->int1 )
					{
						settings |= SITE_SETTING_FIRE_ROUTING_DISABLED;
					}
					else
					{
						settings &= ~SITE_SETTING_FIRE_ROUTING_DISABLED;
					}			
				}
				
				// if write to flash successfull
				if ( quadspi->WriteToPanel( settings, settings ) == QSPI_RESULT_OK )
				{
					UpdateRevision( );
					 
					UpdateRoutingSounderLEDs( );
					 
					return CMD_OK;
				}
				else
				{
					return CMD_ERR_DEVICE_BUSY;
				}
			}
											
		}
		return CMD_ERR_NO_SITE;
	}
	else if ( cmd->type == CMD_SEACH_DEV_TYPE_STRING )
	{
		DeviceConfig* dc = FindType( cmd->int0 );
		
		if ( dc != NULL )
		{
			cmd->int1 = (int) dc->typestring;
		}
		else
		{
			cmd->int1 = 0;
		}
		return CMD_OK;
	}
	else if ( cmd->type == CMD_SET_PASSKEY )
	{
		if ( app.site != NULL )		
		if ( cmd->int1 >= 0 && cmd->int1 < app.site->numUsers )
		{
			if ( app.site != NULL )
			{
				char key[ 5];
				snprintf( key, 5, "%04d", cmd->int0 );
				
				if ( quadspi->WriteToSite( key, users[ cmd->int1 ].pin ) == QSPI_RESULT_OK )
				{
					UpdateRevision( );
					return CMD_OK;
				}
				return CMD_ERR_DEVICE_BUSY;
			}
			return CMD_ERR_NO_SITE;
		}
		return CMD_ERR_OUT_OF_RANGE;
	}	
	else if ( cmd->type == CMD_CHECK_PASSKEY )
	{
		if ( app.panel != NULL )
		{
			char pkey[ 5];
			
			snprintf( pkey, 5, "%04d", cmd->int0 );
			
			for( int n = 0; n < app.site->numUsers; n++ )
			{
				if ( !strcmp( pkey, app.site->users[ n].pin ) )
				{
					cmd->int0 = app.site->users[ n].level;
					cmd->int1 = n;
				
					return CMD_OK;
				}
			}
			cmd->int0 = -1;
			return CMD_OK;	 
		}
		return CMD_ERR_NO_SITE;
	}
	else if ( cmd->type == CMD_LOGON_ACCESS )
	{
		if ( app.panel != NULL )
		{
			char* user = "Key user";
			
			current_user = cmd->int1;
			access_level = cmd->int0;
							
			if ( current_user >= 0 && current_user < SITE_MAX_USERS )
			{
				user = app.site->users[ current_user].name;
			}

			if ( cmd->int2 )
			{
				Log::Msg( LOG_ACC, "%s logged on at level %d.", user, cmd->int0 );
				
			}
			else
			{
				Log::Msg( LOG_ACC, "%s logged off.", user );
			}		
			return CMD_OK;			
		}
		return CMD_ERR_NO_SITE;
	}
	else if ( cmd->type == CMD_GET_RADIO_SETTINGS )
	{
		if ( app.site != NULL )
		{
			cmd->int0 = app.site->panels[ 0 ].systemId & 32767;
			cmd->int1 = (int) app.site->uid;
			cmd->int2 = acu_connected;
			return CMD_OK;
		}
		return CMD_ERR_NO_SITE;
	}
	
	else if ( cmd->type == CMD_SET_RADIO_SETTINGS )
	{
		if ( app.site != NULL )
		{
			acu_id = cmd->int0;
			STRNCPY( acu_key, (char*) cmd->int1, 33 ); 
			acu_encrypt = cmd->int2; 
			acu_baud = cmd->int3; 
			
			job = JOB_ACU_CMD;
			acu_cmd = ACU_INIT;
			
			app.DebOut( "Set radio\n" );
			reply_ok = true;
			
			return CMD_OK;
		}
		return CMD_ERR_NO_SITE;
	}
	else if ( cmd->type == CMD_CHANGE_RADIO_SETTINGS )
	{
		job = JOB_NONE; // abort
		return CMD_OK;
	}
	else if ( cmd->type == CMD_EXIT_RADIO_SETTINGS )
	{
		job = JOB_NONE;
		
		// restore baud
		USBH_FTDI_SetBAUD( hUSBHost + 0, 115200 );
		
		return CMD_OK;
	}	
	
	else if ( cmd->type == CMD_GET_RADIO_SETTINGS )
	{
		if ( app.site != NULL )
		{
			cmd->int0 = app.site->panels[ 0 ].systemId & 32767;
			cmd->int1 = (int) app.site->uid;
			return CMD_OK;
		}
		return CMD_ERR_NO_SITE;
	}
	
	else if ( cmd->type == CMD_GET_SETTINGS )
	{
		if ( app.panel != NULL )
		{
			cmd->int0 = app.panel->settings;
			return CMD_OK;
		}
		return CMD_ERR_NO_SITE;
	}
	
	else if ( cmd->type == CMD_SET_SETTINGS ) // Note not disablementsw
	{
		// if have panel data
		if ( app.panel != NULL )
		{
			// if any settings are different
			if ( memcmp( &cmd->char0, &app.panel->autoSilence, 12 ) )
			{
				// get copy of prev settings
				int prev_settings = app.panel->settings;
				 
				// if successfully write settings / delays 
				if ( quadspi->WriteToFlash( &cmd->char0, ((int)&app.panel->autoSilence) - SETTINGS_ROM_ADDRESS, 12 ) == QSPI_RESULT_OK )
				{		
					// get owner as string
					const char* user = current_user >= 0 ? app.site->users[ current_user ].name : "key owner";
					
					if ( CHANGED( SITE_SETTING_DISABLE_OUTPUT_DELAYS ) ) 
					{
						Log::Msg( LOG_SET, "Output delays switched %s by %s.", ( cmd->int0 & SITE_SETTING_DISABLE_OUTPUT_DELAYS ) ? "off" : "on", user );					
					}
					
					if ( CHANGED( SITE_SETTING_GLOBAL_DELAY_OVERRIDE ) )
					{
						Log::Msg( LOG_SET, "Global delay override switched %s by %s.", ( cmd->int0 & SITE_SETTING_GLOBAL_DELAY_OVERRIDE ) ? "on" : "off", user );				 
					}
					
					if ( CHANGED( SITE_SETTING_IGNORE_SECURITY_IN_DAY ) )
					{
						Log::Msg( LOG_SET, "Daytime security switched %s by %s.", ( cmd->int0 & SITE_SETTING_IGNORE_SECURITY_IN_DAY ) ? "off" : "on", user );							 
					}
					
					if ( CHANGED( SITE_SETTING_IGNORE_SECURITY_AT_NIGHT ) )
					{
						Log::Msg( LOG_SET, "Security switched %s by %s.", ( cmd->int0 & SITE_SETTING_IGNORE_SECURITY_AT_NIGHT ) ? "off" : "on", user );							 
					}
					
					if ( CHANGED( SITE_SETTING_RESOUND_NEW_ZONE ) )
					{
						Log::Msg( LOG_SET, "Re-sound for same zone switched %s by %s.", ( cmd->int0 & SITE_SETTING_RESOUND_NEW_ZONE ) ? "on" : "off", user );
					}			

					ncu->QueueWriteMsg( true, NCU_BROADCAST_GLOBAL_SETTINGS, this, NULL, ! ( app.panel->settings & SITE_SETTING_DISABLE_OUTPUT_DELAYS ),
																									 !! ( app.panel->settings & SITE_SETTING_GLOBAL_DELAY_OVERRIDE ),
																									app.panel->delay1/30, app.panel->delay2/30, !!RealTimeClock::daytime,
																								  !! (app.panel->settings & SITE_SETTING_IGNORE_SECURITY_IN_DAY),
																									!! (app.panel->settings & SITE_SETTING_IGNORE_SECURITY_AT_NIGHT ) );	
					// Update site revision			 
					UpdateRevision( );				
						
					// reflect new delay status
					CheckDelayStatus( );
				}
				else
				{
					return CMD_ERR_DEVICE_BUSY;
				}
			}
			return CMD_OK;
		}
		return CMD_ERR_NO_SITE;
	}
	else if ( cmd->type == CMD_GET_PANEL_NAME )
	{		
		if ( app.site != NULL && cmd->int0 < SITE_MAX_PANELS && cmd->int0 >= 0 )
		{
			if ( app.site->numPanels == 1 )
			{
				cmd->int0 = (int) "";
			}
			else if ( app.site->panels + cmd->int0 == app.panel )
			{
				cmd->int0 = (int) "(This panel)";
			}
			else
			{
				cmd->int0 = (int) app.site->panels[ cmd->int0 ].name;
			}
			return CMD_OK;
		}
		else
		{
			cmd->int0 = 0;
		}
		return CMD_ERR_OUT_OF_RANGE;
	}
	else if ( cmd->type == CMD_GET_CLOUD_DETAILS )
	{
		cmd->int0 = (int) MQTTSupport::device_id;
		cmd->int1 = (int) MQTTSupport::url;
		cmd->int2 = (int) MQTTSupport::connected;
		cmd->int3 = (int) MQTTSupport::outqueue.items;

		return CMD_OK;
	}
	else if ( cmd->type == CMD_GET_PANEL )
	{
		cmd->int3 = (int) Utilities::UID;
		
		if ( app.panel != NULL && app.site != NULL )
		{
			cmd->int0 = (int) app.panel;
			cmd->int1 = app.panel - app.site->panels;
			cmd->int2 = (int) app.site;
			
			return CMD_OK;
		}
		else
		{
			cmd->int0 = 0;
			return CMD_ERR_NO_SITE;
		}
	}
	else if ( cmd->type == CMD_GET_SITE )
	{
		if ( app.site != NULL )
		{
			cmd->int0 = (int) app.site;
			return CMD_OK;
		}
		else
		{
			cmd->int0 = 0;
			return CMD_ERR_NO_SITE;
		}
	}
	
	if ( cmd->type == CMD_SET_DEVICE_ALL )
	{
		if ( app.panel != NULL )
		{
			if ( quadspi->IsWriteProtected( ) )
			{
				return CMD_ERR_WRITE_PROTECTED;
			}		
		 
			pold = FindConfig( cmd->int0 );
			
			// if config exists
			if ( cmd->int1 != 0 && pold != NULL )
			{
				dnew = *(DeviceConfig*)cmd->int1;
			
				if ( cmd->int2 )
				{
					UpdateDevice( pold, &dnew );	
					WriteDevice( pold, dnew );
				}
				else
				{
					WriteDevice( pold, dnew );
				}
				
				UpdateRoutingSounderLEDs( );
				
				return CMD_OK;
			}		
		}
		return CMD_ERR_DEVICE_BUSY;
	}	
	
	if ( cmd->type == CMD_SET_DEVICE )
	{
		if ( app.panel != NULL )
		{
			DeviceConfig* p = FindConfig( cmd->int0 );
			
			if ( p != NULL )
			{
				DeviceConfig dev = *p;
				 
				STRNCPY( dev.location, (const char*) cmd->int1, 40 );
	 
				dev.zone  = cmd->short6;
				
				if ( WriteDevice( p, dev ) )
				{
					Send( MSG_REFRESH_OUTPUTS, iostatus );
					return CMD_OK;
				}
			}
		}		
		return CMD_ERR_OUT_OF_RANGE;
	}	
	
	if ( cmd->type == CMD_GET_NUM_MAX_ZONES )
	{
		if ( app.panel != NULL )
		{
			int maxzones = FACTORY_SETTINGS->zones;
			if ( maxzones < 16 || maxzones > 192 ) maxzones = 96;
			cmd->int0 = maxzones;
		}
		else cmd->int0 = 0;
		
		return CMD_OK;
	}

	if ( cmd->type == CMD_GET_DEVICE )
	{
		if (  app.panel != NULL )
		{
			DeviceConfig* d = FindConfig( cmd->int0 );
			cmd->int0 = (int) d;
		
			return CMD_OK;
		}
		else
		{
			cmd->int0 = (int) 0;
			return CMD_ERR_OUT_OF_RANGE;
		}
	}				
			
	if ( cmd->type == CMD_GET_DEV_IN_ZONE )
	{
		if ( app.panel != NULL )
		{
			int count = 0;
			
			// if alarm output device
			if ( cmd->int0 == GROUP_FIRE_ALARMS )
			{
				for( DeviceConfig* d = app.panel->devs; d < app.panel->devs + app.panel->NumDevs( ); d++ ) 
				{
					if ( d->IsAlarmOutputDevice( ) )
					{
						if ( cmd->int1 == count )
						{
							cmd->int0 = (int) d;
							
							return CMD_OK;
						}
						else
						{	
							count++;
						}
					}
				}
			}
			// else if routing device
			else if ( cmd->int0 == GROUP_FIRE_ROUTING )
			{
				DeviceConfig* d = app.panel->devs;
				count = 0;
				while ( d != NULL )
				{
					d = FindFireRoutingDevice( d );
					if ( d != NULL )
					{
						if ( count == cmd->int1 ) 
						{
							cmd->int0 = (int) d;
							return CMD_OK;
						}
						count++;  
						d++;						
					}
				}
			}
			else
			{
				for( DeviceConfig* d = app.panel->devs; d < app.panel->devs + app.panel->NumDevs( ); d++ ) 
				{
					if ( d->zone == cmd->int0 )
					{
						if ( cmd->int1 == count )
						{
							cmd->int0 = (int) d;
							 
							return CMD_OK;
						}
						else count++;
					}
				}				
			}
		}
		cmd->int0 = 0;

		return CMD_OK;
	}
	
	if ( cmd->type == CMD_REMOVE_DEVICE )
	{
		int d = (int) FindConfig( cmd->int0 );
		
		if ( d != (int)NULL )
		{
			void* endofdevs = app.panel->devs + app.panel->NumDevs();
			if ( quadspi->MoveFlashLower( d - SETTINGS_ROM_ADDRESS + sizeof( DeviceConfig ), d - SETTINGS_ROM_ADDRESS, (int)endofdevs - d - sizeof( DeviceConfig ) ) == QSPI_RESULT_OK )
			{
				short num = app.panel->numRadioDevs - 1;
				quadspi->WriteToPanel( num, numRadioDevs );
				
				Send( MSG_REFRESH_OUTPUTS, iostatus );
				return CMD_OK;
			}
			else
			{
				return CMD_ERR_WRITE_FAIL;
			}
			
		}
		return CMD_ERR_OUT_OF_RANGE;
	}		
		
	if ( cmd->type == CMD_GET_NUM_ZONES )
	{
		 
		int numzones = 0;
		
		if ( app.panel != NULL )
		{
			unsigned int zbits[ SITE_MAX_ZONES / (8*sizeof(unsigned int)) ];
			memset( zbits, 0, sizeof( zbits ) );
 
			for( DeviceConfig* d = app.panel->devs; d < app.panel->devs + app.panel->NumDevs( ); d++ )
			{
				if ( !IFSET( d->zone, zbits ) )
				{
					numzones++;
					SET( d->zone, zbits );
				}
			}
		}
		
		cmd->int0 = numzones;
		
		return CMD_OK;			
	}
	
	// Moved to utils
/*	
	if ( cmd->type == CMD_GET_NUM_DEVS_IN_ZONE )
	{
		int count = 0;
		
		if ( app.panel != NULL )
		{						
			// if alarm output
			if ( cmd->int0 == -1 )
			{
				for( DeviceConfig* d = app.panel->devs; d < app.panel->devs + app.panel->NumDevs( ); d++ ) 
				{
					if ( d->IsAlarmOutputDevice( ) )
					{	
						count++;
						break;	
					}
				}
			}
			// else if routing device
			else if ( cmd->int0 == 0 )
			{
				DeviceConfig* d = app.panel->devs;
				
				while ( d != NULL ))
				{
					d = FindFireRoutingDevice( d );
					if ( d != NULL ) 
					{
						count++;  
						d++;
					}
				}
			}
			else // normal zone
			{
				for( DeviceConfig* d = app.panel->devs; d < app.panel->devs + app.panel->NumDevs( ); d++ ) 
				{
					if ( d->zone == cmd->int0 )
					{
						count++;
					}
				}
			}
			
		}
		cmd->int0 = count;
		
		return CMD_OK;
	}		
*/

	if ( cmd->type == CMD_GET_DEVICE_LISTS )
	{
		if ( app.panel != NULL )
		{			
			if ( ncu != NULL )
			{
				cmd->int0 = (int) app.panel->devs; 
				cmd->int1 = (int) ncu->units;
				cmd->short4 = app.panel->numRadioDevs;
				cmd->short5 = ncu->unit_pos - ncu->units; 
				cmd->short6 = app.panel->NumDevs( );
			 				
				return CMD_OK;
			}
		}
		cmd->int0 = 0; 
		cmd->int1 = 0;
		cmd->short4 = 0;
		cmd->short5 = 0; 
		cmd->short6 = 0; 
	 		
		return CMD_OK;
	}
	
	else if ( cmd->type == CMD_DETECT_RBU )
	{
		if ( cmd->int0 == -1 )
		{
			job = JOB_NONE;
		}
		else if ( app.panel != NULL )
		{
			if ( LowestFreeUnit( ) <= app.panel->maxDevices )
			{
				job = JOB_ATMODE;
				next_job = JOB_GET_ZONE;
				reply_ok = true;
			}
			else
			{
				SendError( "No unit addresses spare." );
			}
		}
		return CMD_OK;
	}
				
	else if ( cmd->type == CMD_SETUP_RBU )
	{	
		if ( app.panel != NULL )
		{
			if ( new_unit == -1 ) return CMD_ERR_OUT_OF_RANGE;
			if ( new_unit > app.panel->maxDevices ) return CMD_ERR_OUT_OF_RANGE;
			
			if ( new_unit != LowestFreeUnit() )
			{
				return CMD_ERR_ALREADY_EXISTS;
			}
			
			if ( SetupDevice( cmd->int0, new_unit, cmd->int1 ) )
			{
				if ( FlashDevice( ) )
				{
					cmd->int3 = new_unit;
					return CMD_OK;	
				}
				return CMD_ERR_WRITE_FAIL;
			}	 			 
		}
		return CMD_ERR_DEVICE_BUSY;
	}
	else if ( cmd->type == CMD_GET_NUM_ROUTING )
	{
		cmd->int0 = 0;
		
		if ( app.panel != NULL )
		{
			for( DeviceConfig* d = app.panel->devs; d < app.panel->devs + app.panel->NumDevs( ); d++ ) 
			{
				for( OutputChannel* o = d->output; o < d->output + d->numOutputs; o++ )
				{
					if ( o->type == CO_CHANNEL_OUTPUT_ROUTING_E || o->type == CO_CHANNEL_PANEL_ROUTING_OUTPUT )
					{
						cmd->int0++;
						break;
					}
				}
			} 
				 
			return CMD_OK;
		}
		return CMD_ERR_NO_SITE;
	}
	else if ( cmd->type == CMD_PROGRAM_RBU )
	{
		if ( app.panel != NULL )
		{
			new_device.SetupStart( &commands );
			job = JOB_ATMODE;
			next_job = JOB_START;
			reply_ok = true;
			return CMD_OK;	
		}
		return CMD_ERR_NO_SITE;
	}
	else if ( cmd->type == CMD_FACTORY_RESET )
	{	
		eeprom->SetDefaults( true );
	
		current_pos = -1;
		prev_pos 	= -1;
		
		static FactorySettings *f;
		FactorySettings fact_sett; 

		fact_sett.zones 	 = *(char*)  0x90C00000;
    fact_sett.leds  	 = *(char*)  0x90C00001;
    fact_sett.devices  = *(short*) 0x90C00002;
				
		int res = EraseFlash( ) ? CMD_OK : CMD_ERR_DEVICE_BUSY;
		
		// Write back the erase.
		if(quadspi->WriteToFlash( (char*) &fact_sett, QSPI_SETTINGS_ADDRESS  , sizeof( fact_sett ) ) )
		{
			if ( quadspi->WriteToFlash( (char*) &fact_sett, QSPI_SETTINGS_ADDRESS , sizeof( fact_sett ) ) != QSPI_RESULT_OK )
			{
				// Fault::AddFault( FAULT_NO_QSPI_COMMS );
				Log::Msg(LOG_EVT,"Failed to write to flash");
			}
		}
		 
		fact_sett.zones 	 = *(char*)  0x90C00000;
    fact_sett.leds  	 = *(char*)  0x90C00001;
    fact_sett.devices  = *(short*) 0x90C00002;
		
		LED::AllLEDSOff( );
		
		app.Send( EVENT_CLEAR );
		
		app.panel = NULL;
		app.site = NULL;
		
		Fault::AddFault( FAULT_BAD_SITE_FILE );
				 
		app.Send( EVENT_ENTER_SAFE_MODE );		
		
		app.Send( EVENT_UPDATE );
		
		return res;
	}
	 
	return CMD_ERR_UNKNOWN_CMD;		
}

 
/*************************************************************************/
/**  \fn      void CheckUpdate( Module*, int success )
***  \brief   Callback function
******************************************************************/

void CheckUpdate( Module* m, int success )
{
	Settings* s = (Settings*) m;
	
	if ( success )
	{
		AppDeviceClass_TriggerUpdateDeviceEvent( app.DeviceObject, ++s->checkcount * 100 / s->checkflags );
		app.DebOut( "Check %d / %d\n", s->checkcount, s->checkflags );		 
	}
	else
	{
		app.DebOut( "CheckUpdate fail.\n" );
		AppDeviceClass_TriggerUpdateDeviceEvent( app.DeviceObject, -1 );
	}
}


/*************************************************************************/
/**  \fn      void Settings::UpdateDevice( const DeviceConfig* pold, const DeviceConfig* pnew )
***  \brief   Class function
**************************************************************************/

void Settings::UpdateDevice( const DeviceConfig* pold, const DeviceConfig* pnew )
{				
	Device* dev = iostatus->FindDevice( pold->unit );
	
	// if device exists
	if ( dev != NULL )
	{		
		checkflags = 0;
		checkcount = 0;
	
			
		// if disabled flag different
		if ( pold == NULL || ( pold->flags & DEVCONFIG_DISABLED ) != ( pnew->flags & DEVCONFIG_DISABLED ) )
		{
			int disabled = pnew->flags & DEVCONFIG_DISABLED;
			// Broadcast disable state ( if broadcastable )
			if ( dev->Broadcasted( ) )
			{
				ncu->QueueWriteMsg( true, NCU_BROADCAST_ENABLE_DEVICE, this, CheckUpdate, pold->zone, pold->unit, !disabled ); 
				checkflags++;
			}
			
			Log::Msg( LOG_DIS, "Unit %d (Zone %d) %s.", pnew->unit, pnew->zone, disabled ? "disabled" : "enabled" );
		}
		
		// Check for channel disablements and set broadcast settings
		for( int n = 0; n < pold->numInputs; n++ )
		{
			ChannelType channel = pnew->input[ n ].type;
			
			int daychange = true;
			int nightchange = true;
			 
			
			if ( pold != NULL )
			{
			   daychange =   ( pold->input[ n ].flags & CHANNEL_OPTION_DISABLED_DAYTIME )   != ( pnew->input[ n ].flags & CHANNEL_OPTION_DISABLED_DAYTIME );
			   nightchange = ( pold->input[ n ].flags & CHANNEL_OPTION_DISABLED_NIGHTTIME ) != ( pnew->input[ n ].flags & CHANNEL_OPTION_DISABLED_NIGHTTIME );
			}
			
			int daydis   = ( pnew->input[ n ].flags & CHANNEL_OPTION_DISABLED_DAYTIME ) ? 0 : 8;
			int nightdis = ( pnew->input[ n ].flags & CHANNEL_OPTION_DISABLED_NIGHTTIME ) ? 0 : 16;
			
			if ( ( daychange || nightchange ) && dev->Broadcasted( ) )
			{  
				int flags = daydis | nightdis;
				
				ncu->QueueWriteMsg( true, NCU_CONFIG_OUTPUT_FLAGS, this, CheckUpdate, pnew->zone, pnew->unit, channel, flags );
				
				checkflags++;
			}
					
			// log it
			if ( daychange ) 
			{
				Log::Msg( LOG_DIS, "Unit %d (Zone %d) %s %s in daytime.", pnew->unit, pnew->zone, GetTypeName( channel ), !daydis ? "disabled" : "enabled" );
			}
			if ( nightchange )
			{
				Log::Msg( LOG_DIS, "Unit %d (Zone %d) %s %s in nighttime.", pnew->unit, pnew->zone, GetTypeName( channel ), !nightdis ? "disabled" : "enabled" );
			}
		}
		
		// Check for channel disablements and broadcast settings
		for( int n = 0; n < pnew->numOutputs; n++ )
		{
			ChannelType channel = pnew->output[ n ].type;
			
			int newflags = pnew->output[ n].flags & CHANNEL_OPTION_DISABLED_ANYTIME;
			
			if ( pold == NULL || newflags != ( pold->output[ n].flags & CHANNEL_OPTION_DISABLED_ANYTIME ) )
			{
				Log::Msg( LOG_DIS, "Unit %d (Zone %d) %s %s.", pnew->unit, pnew->zone, GetTypeName( channel ), newflags ? "disabled" : "enabled" );
			}
			
			if ( dev->Broadcasted( ) )
			{				
				// if channel disabled different
				int mask = CHANNEL_OPTION_INVERTED | CHANNEL_OPTION_NO_NIGHT_DELAY | CHANNEL_OPTION_SILENCEABLE | CHANNEL_OPTION_DISABLED_ANYTIME ;
				
				if ( pold == NULL || ( pnew->output[ n].flags & mask ) != ( pold->output[ n].flags & mask ) )
				{
					int flags = pnew->output[ n].flags & CHANNEL_OPTION_INVERTED     ? 1 : 0;
					flags |=    pnew->output[ n].flags & CHANNEL_OPTION_NO_NIGHT_DELAY ? 2 : 0;
					flags |=    pnew->output[ n].flags & CHANNEL_OPTION_SILENCEABLE  ? 4 : 0;
					flags |=    pnew->output[ n].flags & CHANNEL_OPTION_DISABLED_ANYTIME ? 0 : 8 + 16;
					
					ncu->QueueWriteMsg( true, NCU_CONFIG_OUTPUT_FLAGS, this, CheckUpdate, pnew->zone, pnew->unit, channel, flags );	
					checkflags++; 
				}
				
				if ( pold == NULL || ( pnew->output[ n].delay1 != pold->output[ n ].delay1 || pnew->output[ n].delay2 != pold->output[ n ].delay2 ) )
				{
					ncu->QueueWriteMsg( true, NCU_CONFIG_OUTPUT_DELAYS, this, CheckUpdate, pnew->zone, pnew->unit, channel, pnew->output[ n].delay1, pnew->output[ n].delay2 );
					CheckDelayStatus( ); 
					checkflags++;
				}
				
				if ( pold == NULL || ( pnew->output[ n].flags & CHANNEL_OPTION_USE_GLOBAL_DELAY ) != ( pold->output[ n].flags & CHANNEL_OPTION_USE_GLOBAL_DELAY ) )
				{
					ncu->QueueWriteMsg( true, NCU_CONFIG_LOCAL_DELAY, this, CheckUpdate, pnew->zone, pnew->unit, channel, ! ( pnew->output[ n].flags & CHANNEL_OPTION_USE_GLOBAL_DELAY ) );
					checkflags++;
					CheckDelayStatus( ); 
				}
				
				if ( pold == NULL || pnew->output[ n].actions != pold->output[ n].actions )
				{
					ncu->QueueWriteMsg( true, NCU_CONFIG_OUTPUT_ACTIONS, this, CheckUpdate, pnew->zone, pnew->unit, channel, pnew->output[ n].actions );
					checkflags++;
				}
			}
			else
			{
				dev->current_profile[ n] = 0xFE;				// set to invalid
			}
		}
		
		// Check for cextra params
		for( int n = 0; n < pold->numParams; n++ )
		{
			if ( pold == NULL || pnew->param[ n ].value != pold->param[ n].value )
			{
				if ( pnew->param[ n].code == Param_Volume )
				{
					int chan = pnew->output[ pnew->param[ n ].channel ].type;
					
					int volume = pnew->param[ n ].value;
					
					if ( pnew->param[ n].min > 0 ) volume -= pnew->param[ n].min;
					
					ncu->QueueWriteMsg( true, NCU_SET_VOLUME, this, CheckUpdate, pnew->zone, pnew->unit, chan, volume );
					checkflags++;
				}
			}
		}
		
		app.DebOut( "Checks = %d\n", checkflags );
			
		if	( checkflags == 0 )
		{
			checkflags = 1;
			
			CheckUpdate( this, true );
		}
	}				
}	



/*************************************************************************/
/**  \fn      void Settings::UpdateAllDevices( Panel* old )
***  \brief   Class function
**************************************************************************/

void Settings::UpdateAllDevices( Panel* old, int num_old )
{
	if ( app.panel != NULL )
	{
		for( DeviceConfig* dc = app.panel->devs; dc < app.panel->devs + app.panel->NumDevs( ); dc++ )
		{
			for( DeviceConfig* olddc = old->devs; olddc < old->devs + num_old; olddc++ )
			{
				if ( dc->unit == olddc->unit && dc->type == olddc->type )
				{
					UpdateDevice( olddc, dc );
					break;
				}
			}
		}
		 
		Send( MSG_REFRESH_OUTPUTS, iostatus );
	}	
}


/*************************************************************************/
/**  \fn      int Settings::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int Settings::Receive( Event event )
{
	switch( event )
	{
		case EVENT_DAY_TIME:
		case EVENT_NIGHT_TIME:
		{
			CheckDelayStatus( );
			break;
		}			
		
		case EVENT_FTDI_ACTIVE:
		{
			// look at start of serial number (eek)
		//	if ( hUSBHost[ 0].device.DevDesc.idProduct == 0x6015 ) // for Acumesh?- ggrrrr this has changed
			{
				acu_connected = true;
				Send( MSG_ACUMESH_CONNECT, this, 1 );
			}
			unit_sent = false;
			
			break;
		}
		
		case EVENT_FTDI_REMOVED:
		{
			acu_connected = false;
			Send( MSG_ACUMESH_CONNECT, this, 0 );
			break;
		}
				
		case EVENT_RESET_BUTTON:
		{	
			AppMessage msg( APP_EVENT_RESET_FAULTS );
#ifdef GSM_HTTP			
			AppSupport::Send( msg );
#else
			MQTTSupport::Send( msg );
#endif
			//Log::Msg( LOG_SYS, "User Panel Reset." );
			
			// Clear/zero everything
			app.Send( EVENT_CLEAR );
			
			// Reset everything
			app.Send( EVENT_RESET );
			
			// Update/Refresh everything
			app.Send( EVENT_UPDATE );
			
			if ( app.site == NULL )
			{
				Fault::AddFault( FAULT_BAD_SITE_FILE );
				 
				app.Send( EVENT_ENTER_SAFE_MODE );		
			}
		}
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int Settings::Receive( Message* m )
***  \brief   Class function
**************************************************************************/

int Settings::Receive( Message* m )
{
	switch ( m->type )
	{
		case MSG_ACUMESH_CONNECT:
		{
			AppDeviceClass_TriggerAcumeshEvent( app.DeviceObject, m->value );
			break;
		}
		case MSG_INIT_SITE:
		{		 
			// valdiate new site
			if ( ValidateSite( m->value, true ) )
			{	
				ncu->QueueWriteMsg( false, NCU_CLEAR_QUEUE, this, NULL, 1,1, 1,1, 1,1, 1,1, 1 );				
				
				// Reset everything
				app.Send( EVENT_RESET );
				
				//force new logon to set device num
#ifdef GSM_HTTP				
				*AppSupport::token = 0;
				*AppSupport::reftoken = 0;
#else
				MQTTSupport::logged_in = false;
#endif				
							
				CheckDelayStatus( );
			}
			 
			// Update GUI
			app.Send( EVENT_UPDATE );
							
			break;
		}
		
		case MSG_SITE_TRANSFER_SUCCESS:
		{ 
			int num_old = 0;
			Panel* old_addr = NULL;
			
			if ( app.panel != NULL )
			{
				num_old = app.panel->NumDevs( );
				old_addr = app.panel;
			}
		 
			// Stop progress
			AppDeviceClass_TriggerEndProgressEvent( app.DeviceObject, 100 );
			
			// Clear/zero everything
			app.Send( EVENT_CLEAR );
				
			if ( current_pos != m->value )
			{
				// Update to new panel address
				SetReadAddress( m->value );	
			}
			 
			ncu->UpdateSiteSettings( ); 	
			
			CheckDelayStatus( );
			
			ncu->QueueWriteMsg( false, NCU_CLEAR_QUEUE, this, NULL, 1,1, 1,1, 1,1, 1,1, 1 );
			
			// Reset everything
			app.Send( EVENT_RESET );
			
			if ( old_addr != NULL ) UpdateAllDevices( old_addr, num_old );
			
			// Update GUI
			app.Send( EVENT_UPDATE );
	 
			break;
		}
	
		case MSG_SITE_TRANSFER_FAIL:
		{
			ew->StopProgress( false );
			
			// Dont do anything else - we keep current site..
			break;
		}
		  
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int Settings::GenerateBlank( )
***  \brief   Class function
**************************************************************************/

#define cpy( x, y ) STRNCPY( x, y, sizeof( x ) )

int Settings::EraseFlash( )
{
	app.site = NULL;
	app.panel = NULL;
	
	for( int p = 0; p < QSPI_END_ADDR; p += QSPI_SECTOR_SIZE )
	{
		if ( !quadspi->EraseSector( p ) )
		{
			Fault::AddFault( FAULT_NO_QSPI_COMMS );
			
			return false;
		}		
	}
	 
	return true;					 
}
	

#define ASS( x ) if ( !(x) ) { app.DebOut( "Site data corrupt: %s", #x ); return 0; }



/*************************************************************************/
/**  \fn      int Settings::SiteLegal( Site* s )
***  \brief   Class function
**************************************************************************/

int Settings::SiteLegal( Site* s )
{
	// check max devices
	int maxdevs = FACTORY_SETTINGS->devices;
	
	if ( maxdevs < 16 || maxdevs > 1024 ) maxdevs = 511;
	
	if ( s->panels[ s->currentIndex ].numRadioDevs > maxdevs )
	{
		return false;
	}
	
	// chek max zones
	int maxzones = FACTORY_SETTINGS->zones;
	
	if ( maxzones < 16 || maxzones > 192 ) maxzones = 96;
	
	DeviceConfig* start = s->panels[ s->currentIndex ].devs;
	DeviceConfig* end = start + s->panels[ s->currentIndex ].NumDevs();
	
	for( DeviceConfig* d = start; d < end; d++ )
	{
		if ( d->zone > maxzones ) return false;
	}
		
	// All good
	return true;
}


/*************************************************************************/
/**  \fn      int Settings::CheckSite( Site* s )
***  \brief   Class function
**************************************************************************/

int Settings::CheckSite( Site* s )
{
	ASS( s->numPanels > 0 && s->numPanels <= SITE_MAX_PANELS )
	
	ASS( s->currentIndex < SITE_MAX_PANELS )
	 	
	ASS( s->numContacts <= SITE_MAX_CONTACTS );	
   ASS( s->numUsers <= SITE_MAX_USERS );

	ASS( strlen( s->name ) < SITE_MAX_STRING_LEN )
	ASS( strlen( s->uid ) < SITE_MAX_STRING_LEN )
		
	for( int n = 0; n < s->numContacts; n++ )
	{
		ASS( strlen( s->contacts[ n].name ) < SITE_MAX_STRING_LEN )
		ASS( strlen( s->contacts[ n].number ) < SITE_MAX_PHONE_DIGITS )	
	}	
	
	for( int n = 0; n < s->numUsers; n++ )
	{
		ASS( strlen( s->users[ n].name ) < SITE_MAX_STRING_LEN )
		ASS( s->users[ n].level < 5 )		
		ASS( strlen( s->users[ n].pin ) < 5 )
	}	
	
	for( Panel* p = s->panels; p < s->panels + s->numPanels; p++ )
	{
		ASS( strlen( p->name ) < SITE_MAX_STRING_LEN )
 	 
		ASS( p->numRadioDevs <= SITE_MAX_RADIO_DEVICES )
		ASS( p->numPanelDevs <= SITE_MAX_PANEL_DEVICES )
		
		for( DeviceConfig* d = p->devs; d < p->devs + p->NumDevs( ); d++ )
		{
			ASS( strlen( d->location ) < SITE_MAX_STRING_LEN )
			
			ASS( d->numParams <= SITE_MAX_CHANNEL_PARAMS )
			 
			ASS( d->unit > 0 && d->unit <= SITE_MAX_DEVICES );
			
			ASS( d->zone > 0 && d->zone < 97 )
			
		}
		
		ASS( s->numContacts < SITE_MAX_CONTACTS );
	
		for( SMSContact* c = s->contacts; c < s->contacts + s->numContacts; c++ )
		{
			ASS( strlen( c->number ) < 16 )
		}
	}
	return true;	 
}		
		

void Settings::DumpPanel( Panel* panel )
{	
	app.DebOut( "Panel %s:\n", panel->name );
	app.DebOut( "%d devices\n", panel->NumDevs() );	
		 
	for( int n = 0; n < panel->NumDevs(); n++ )
	{	
		app.DebOut( "Location: %s\n", panel->devs[n].location );
		app.DebOut( "Actions: " );
		
		for( int c = 0; c < panel->devs[n].numInputs; c++ )
		{
			int f = panel->devs[n].input[c].flags;
			app.DebOut( "%d, ", f );
		}
		for( int c = 0; c < panel->devs[n].numInputs; c++ )
		{
			int f = panel->devs[n].input[c].flags;
			
			if ( f & CHANNEL_OPTION_SKIP_OUTPUT_DELAYS )
			{
				app.DebOut( "Skips output delays, " );
			}
			if ( f & CHANNEL_OPTION_INVERTED )
			{
				app.DebOut( "Inverted, " );
			}
			if ( f & CHANNEL_OPTION_LATCHING )
			{
				app.DebOut( "Latching, " );
			}
			if ( f & CHANNEL_OPTION_SILENCEABLE )
			{
				app.DebOut( "Silenceable, " );
			}
		}	 
		app.DebOut( "\n" );	 
	}
}


int Settings::SetupDevice( int zone, int unit, int type )
{
	if ( app.panel != NULL )
	{
		DeviceConfig* existing_dev = FindType( type );

		if ( existing_dev == NULL ) return false;
				
		DeviceConfig& dc = Settings::new_device;
		
		// copy settings over
		memcpy( &dc, existing_dev, sizeof(DeviceConfig) ); 
		
		dc.unit = unit;
	 	dc.zone = zone;
		dc.location[ 0] = 0;		// clear location
		
		return true;
		
		// Dont setup features / defaults - just use copied device settings..
		
/*		  
		uint64_t feats = combination[ type ];
		
		for( DM_DeviceType_t dt = (DM_DeviceType_t) 0; dt < DEV_MAX_DEV_TYPE_E; dt = (DM_DeviceType_t)(dt + 1))
		{			
			if ( feats & ( 1 << dt ) )
			{
				ChannelType ch = GetChannelFeat( dt );
				
				if ( ch != CO_CHANNEL_STATUS_INDICATOR_LED_E )
				{
					if ( IsOutput( ch ) )
					{
						dc.output[ dc.numOutputs ].type = ch;
						dc.output[ dc.numOutputs ].actions = ACTION_BIT_FIRE | ACTION_BIT_FIRST_AID | ACTION_BIT_EVACUATION;
						dc.output[ dc.numOutputs ].delay1 = 0;
						dc.output[ dc.numOutputs ].delay2 = 0;
						dc.output[ dc.numOutputs++ ].flags = CHANNEL_OPTION_SILENCEABLE;
						
						if ( ch == CO_CHANNEL_SOUNDER_E )
						{
							dc.param[ dc.numParams ].code = Param_Volume;
							dc.param[ dc.numParams ].value = 2;
							dc.param[ dc.numParams ].min = 1;
							dc.param[ dc.numParams ].max = 3;
							dc.param[ dc.numParams++ ].channel = ch;
						}
					}
					else
					{
						dc.input[ dc.numInputs ].type = ch;
						
						if ( ch == CO_CHANNEL_PIR_E )
						{
							dc.input[ dc.numInputs ].action = ACTION_SHIFT_SECURITY;
						}
						else if ( ch == CO_CHANNEL_MEDICAL_CALLPOINT_E )
						{
							dc.input[ dc.numInputs ].action = ACTION_SHIFT_FIRST_AID;
						}	
						else 
						{
							dc.input[ dc.numInputs ].action = ACTION_SHIFT_FIRE;
						}
						
						dc.input[ dc.numInputs++ ].flags = CHANNEL_OPTION_LATCHING;
						dc.input[ dc.numInputs ].delay = 0;
					}
				}
			}
		}
		*/
		
	}
	return false;
}


int Settings::FlashDevice( )
{
	if ( app.panel != NULL )
	{
		if ( quadspi->WriteToPanel( new_device, devs[ app.panel->NumDevs( ) ] ) == QSPI_RESULT_OK )
		{		
			uint16_t numRadioDevs = app.panel->numRadioDevs + 1;
			if ( quadspi->WriteToPanel( numRadioDevs, numRadioDevs ) == QSPI_RESULT_OK )
			{		
				UpdateRevision( );
				app.Send( EVENT_UPDATE );
				return true;			
			}
		}
	}
	return false;
}	


extern "C" void USBH_FTDI_ReceiveCallback( USBH_HandleTypeDef *phost, uint8_t* buff, int length )
{
	app.DebOut("FTDI In: ");
	buff[ length ] = 0;
	app.DebOut( (char*) buff );
	
	static char zone = 0; 
	
	if ( strstr( (const char*)buff, "OK" ) )
	{
		switch ( Settings::job )
		{
			case JOB_ACU_PLUS:
			{
				Settings::job = JOB_ACU_CMD;
				if ( Settings::acu_cmd == ACU_START )
				{
					Settings::acu_cmd = ACU_ID;
				}
				break;
			}
			case JOB_ACU_CMD:
			{
				if ( Settings::acu_cmd == ACU_START )
				{
					Settings::acu_cmd = ACU_ID;
				}
				else				
				{
					Settings::job = JOB_ACU_WR;
				}
				break;
			}
			case JOB_ACU_WR:
			{
				Settings::job = JOB_ACU_AC;
				break;
			}
			case JOB_ACU_AC:
			{
				Settings::job = JOB_ACU_CN;
				break;
			}
			case JOB_ACU_CN:
			{
				Settings::acu_cmd = (AcuCmd) ( (int) Settings::acu_cmd + 1 ); 
				
				Settings::job = JOB_ACU_CMD;
				break;
			}
				
			case JOB_ATMODE:
			{			 
				Settings::new_unit = LowestFreeUnit( );
				
				AppDeviceClass_TriggerRBUDetectedEvent( app.DeviceObject, Settings::new_unit );
							
				Settings::reply_ok = true;
				
				Settings::job = Settings::next_job;
				break;
			}	
		}	
	}	
	else if ( strstr( (const char*)buff, "ERROR" ) )
	{
		if ( Settings::job == JOB_ATMODE )
		{
			SendError( "'%s' command returned an error." );
			Settings::at_errors = true;
		}
		else if ( Settings::job < JOB_ATMODE )
		{
			Settings::job = JOB_ACU_PLUS;
			app.DebOut( "ACU ERRROR\n" );
		}
	}
	
	if ( Settings::job == JOB_GET_ZONE )
	{
		int n;
		
		if ( sscanf( (const char*)buff, "ZONE: %d\r\n", &n ) == 1 )
		{
			zone = (char) n;
			Settings::job = JOB_GET_COMBI;
			Settings::reply_ok = true;
		}
	}
	else if ( Settings::job == JOB_GET_COMBI )
	{
		int n;

		if ( sscanf( (const char*)buff, "DEVCF: %d\r\n", &n ) == 1 )
		{
			Settings::job = JOB_NONE;
			Settings::reply_ok = true;
			AppDeviceClass_TriggerNewZoneAndCombi( app.DeviceObject, zone, n );
		}
	}
	else if ( strstr( (const char*)buff, (const char*)Settings::reply ) )
	{
		Settings::reply_ok = true;		
	}
}


int Settings::UpdateRevision( )
{
	uint32_t new_rev = app.site->version + 1;
	
	return quadspi->WriteToSite( new_rev, version );
}

void Settings::Poll( )
{
	static int timeout = 0;
	
	if ( job == JOB_NONE ) return;

	if ( hUSBHost[ 0].pActiveClass != NULL )
	{
		if ( !strcmp( hUSBHost[ 0].pActiveClass->Name, "FTDI" ) )
		{			
			if ( reply_ok )
			{
				if ( commands.Empty() ) switch ( job )
				{
					case JOB_START:
					{
						AppDeviceClass_TriggerRBUProgressEvent( app.DeviceObject, 25, NULL ); 
						new_device.SetupInputs( &commands );
						job = JOB_INPUTS;
						if ( !commands.Empty() ) break;
					}
					case JOB_INPUTS:
					{
						AppDeviceClass_TriggerRBUProgressEvent( app.DeviceObject, 50, NULL ); 
						new_device.SetupOutputs( &commands );
						job = JOB_OUTPUTS;
						if ( !commands.Empty() ) break;
					}
					case JOB_OUTPUTS:
					{
						AppDeviceClass_TriggerRBUProgressEvent( app.DeviceObject, 75, NULL ); 
						new_device.SetupProfiles( &commands );
						job = JOB_PROFILES;
						if ( !commands.Empty() ) break;
					}
					case JOB_PROFILES:
					{
						USBH_FTDI_Transmit( hUSBHost + 0, (uint8_t*) "ATR+\r\n", 6 );
						
						if ( at_errors )
						{
							AppDeviceClass_TriggerRBUProgressEvent( app.DeviceObject, 100, (int) "Finished, but with errors." ); 
						}
						else
						{
							AppDeviceClass_TriggerRBUProgressEvent( app.DeviceObject, 100, (int) "Setup complete." );
						}
						
						job = JOB_NONE;
						
						ncu->UpdateMeshStatus( );
						
						app.Send( EVENT_UPDATE );
						
						return;
					}	
				}
				
				timeout = 0;
				
				if ( job == JOB_ACU_CMD )
				{ 
					AppDeviceClass_TriggerAcumeshEvent( app.DeviceObject, acu_cmd );
					 
					switch ( acu_cmd )
					{
						case ACU_INIT:
						{
							USBH_StatusTypeDef result = USBH_FTDI_SetBAUD( hUSBHost + 0, acu_baud );
							app.DebOut( "Setting baud %d, ", acu_baud );
							
							if ( result == USBH_FAIL )
							{	
								AppDeviceClass_TriggerAcumeshEvent( app.DeviceObject, -2 );
								job = JOB_NONE;
								app.DebOut( "failed\n" );
								return;
							}
							else if ( result == USBH_OK )					
							{
								acu_cmd = ACU_START;
							 
								app.DebOut( "done\n" );
							}
							else return;
						}
				 
						case ACU_START:
						{ 
					    	const static uint8_t start[] = { 0x7E, 0x00, 0x04, 0x08, 0x01, 0x41, 0x50, 0x65 };
							 
							USBH_FTDI_Transmit( hUSBHost + 0, start, sizeof( start ) );
							app.DebOut( "Start code\n" );
							break;
						}
						case ACU_ID:
						{
							static char id[ 12];
							
							snprintf( id, 12, "ATID%X\r", acu_id );
							
							USBH_FTDI_Transmit( hUSBHost + 0, (uint8_t*) id, strlen( id ) );
							app.DebOut( id );
							app.DebOut( "\n" );
							break;
						}
						case ACU_KEY:
						{
							static char key[ 38];
							
							snprintf( key, 38, "ATKY%s\r", acu_key );
							
							USBH_FTDI_Transmit( hUSBHost + 0, (uint8_t*) key, strlen( key ) );
							app.DebOut( key );
							app.DebOut( "\n" );
							break;
						}
						case ACU_ENCRYPT:
						{
							if ( acu_encrypt )
							{
								USBH_FTDI_Transmit( hUSBHost + 0, (uint8_t*) "ATEE1\r", 6 );
								app.DebOut( "ATEE1\n" );
							}
							else
							{
								USBH_FTDI_Transmit( hUSBHost + 0, (uint8_t*) "ATEE0\r", 6 );
								app.DebOut( "ATEE0\n" );
							}
							
							break;
						}
						case ACU_BAUD:
						{
							USBH_FTDI_Transmit( hUSBHost + 0, (uint8_t*) "ATBD3\r", 6 );
						 
							app.DebOut( "baud..." );
							break;
						}
						case ACU_DONE:
						{
							job = JOB_NONE;
							app.DebOut( "Done\n" );
						}
					}
					
				}
				else if ( job == JOB_ACU_WR )
				{
					USBH_FTDI_Transmit( hUSBHost + 0, (uint8_t*) "ATWR\r", 5 );
					app.DebOut( "WR\n" );
				}
				else if ( job == JOB_ACU_AC )
				{
					USBH_FTDI_Transmit( hUSBHost + 0, (uint8_t*) "ATAC\r", 5 );
					app.DebOut( "AC\n" );
				}
				else if ( job == JOB_ACU_CN )
				{
					USBH_FTDI_Transmit( hUSBHost + 0, (uint8_t*) "ATCN\r", 5 );
					app.DebOut( "CN\n" );
				}
				else if ( job == JOB_ACU_PLUS )
				{
					USBH_FTDI_Transmit( hUSBHost + 0, (uint8_t*) "+++", 3 );
					app.DebOut( "+++\n" );
				}
				
				// RBU setup
				
				else if ( job == JOB_ATMODE )
				{
					app.DebOut("FTDI Out: +++\r\n");
					USBH_FTDI_Transmit( hUSBHost + 0, (uint8_t*) "+++\r\n", 5 );			 
					timeout = 11;
				}
				else if ( job == JOB_GET_ZONE )		
				{
					app.DebOut("FTDI Zone?\r\n");
					USBH_FTDI_Transmit( hUSBHost + 0, (uint8_t*) "ATZONE?\r\n", 9 );	
				}
				else if ( job == JOB_GET_COMBI )		
				{
					app.DebOut("FTDI Zone?\r\n");
					USBH_FTDI_Transmit( hUSBHost + 0, (uint8_t*) "ATDEVCF?\r\n", 10 );	
				}
				else  
				{
					cmd = commands.Pop( );
								
					char fm2[ 64] = "AT";
					static char buff[ 64];
					
					STRNCPY( fm2 + 2, cmd->fmt, 62 );
					STRNCAT( fm2, "\r\n", 64 );
					
					snprintf( buff, 64, (const char*) fm2, cmd->param[ 0], cmd->param[ 1], cmd->param[ 2], cmd->param[ 3] );
					
					app.DebOut( "FTDI Out: " );
					app.DebOut( buff );
					USBH_FTDI_Transmit( hUSBHost + 0, (uint8_t*) buff, strlen( buff ) );
			
					const char* equals = strchr( cmd->fmt, '=' );
					if ( equals != NULL )
					{
						equals = strchr( cmd->fmt, '+' );
						if ( equals != NULL ) 
						{
							STRNCPY(	reply, cmd->fmt, equals - cmd->fmt );		
							reply[ equals - cmd->fmt ] = 0;
						}
					}
				}	
				reply_ok = false;		
				 
			}	
			else
			{
				timeout++;
				
				if ( timeout > 10 )
				{			 
					timeout = 0;
					
					reply_ok = true;
					
					if ( job > JOB_ATMODE )
					{
						at_errors = true;
						SendError( "Timeout waiting for '%s' command.", reply );
						job = JOB_ATMODE;
					}
					if ( job < JOB_ATMODE )
					{
						AppDeviceClass_TriggerAcumeshEvent( app.DeviceObject, -3 );
						if ( job == JOB_ACU_PLUS )
						{
							job = JOB_ACU_CMD;
						}
						else						
						{
							job = JOB_ACU_PLUS;
						}
					}
				}
			}
		}
	}					 
}	 
