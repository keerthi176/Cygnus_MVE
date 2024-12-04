/***************************************************************************
* File name: MM_Fault.cpp
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
#include "stm32f7xx_hal_pwr.h"
#include "stm32f7xx_hal_flash_ex.h"
#include <assert.h>
#include <stdio.h>
#include "App.h"
#include "_AppLibClass.h"
#include <limits.h>
 

/* User Include Files
**************************************************************************/
#include "MM_Fault.h"
#include "MM_CUtils.h"
#include "MM_Buzzer.h"
#include "MM_Log.h"
#include "MM_LED.h"
#include "MM_Settings.h"
#include "MM_GSM.h"
#include "MM_Utilities.h"

#ifdef GSM_HTTP			
#include "MM_AppSupport.h"
#else
#include "MM_MQTTSupport.h"
#endif	
#include "MM_NCU.h"
#include "MM_EmbeddedWizard.h"

#include "MM_TouchScreen.h"

/* Defines
**************************************************************************/

#define NUM_ITEMS( x ) ( sizeof(x) / sizeof(*x) )
#define MAX_FAULTS 250
#define MAX_DEVICE_FAULT_NUM 32


/* Globals
**************************************************************************/
extern int __rcc_csr;

int Fault::fire_routing_faulty = 0;
int Fault::sounder_faulty		 = 0;
 
								 
static FaultItem fault_list[ MAX_FAULTS ]; 

time_t Fault::last_fault_activity = PREVENT_BLANK;
 
								 
FaultItem* 		 Fault::fault_pos = fault_list;		
GPIOManager* 	 Fault::gpio = NULL;								
EmbeddedWizard* Fault::ew = NULL;								 
GSM*				 Fault::gsm = NULL;
 

static const FaultDef fault[] = {
	
	{ FAULT_SIGNAL_NO_ERROR_E,                  "<No error>?" },
	{ FAULT_SIGNAL_TYPE_MISMATCH_E,             "Device detected wrong head.",       0 ,                  FAULTGROUP_INSTALLATION },
	{ FAULT_SIGNAL_DEVICE_ID_MISMATCH_E,        "Device ID mismatch.",               0 | FAULTFLAG_LEVEL_2 | FAULTFLAG_WARNING, FAULTGROUP_INSTALLATION },
	{ FAULT_SIGNAL_FAULTY_SENSOR_E,             "Device faulty sensor.",             0,                   FAULTGROUP_DEVICE       },
	{ FAULT_SIGNAL_DIRTY_SENSOR_E,              "Device dirty smoke sensor.",        0 | FAULTFLAG_LEVEL_2 | FAULTFLAG_WARNING, FAULTGROUP_DEVICE },
	{ FAULT_SIGNAL_FAULTY_DIRTY_SENSOR_E,       "Device faulty/dirty smoke sensor.", 0 | FAULTFLAG_LEVEL_2 | FAULTFLAG_WARNING, FAULTGROUP_DEVICE },
	{ FAULT_SIGNAL_INTERNAL_FAULT_E,            "Device internal fault.",            FAULTFLAG_LEVEL_2 | FAULTFLAG_ALARM_DEVICE|FAULTFLAG_WARNING, FAULTGROUP_DEVICE },
	{ FAULT_SIGNAL_SOUNDER_FAULT_E,             "Device sounder fault.",             FAULTFLAG_LEVEL_2 | FAULTFLAG_SOUNDER|FAULTFLAG_WARNING, FAULTGROUP_DEVICE },
	{ FAULT_SIGNAL_BEACON_LED_FAULT,            "Device beacon LED fault.",          0 | FAULTFLAG_LEVEL_2 | FAULTFLAG_WARNING, FAULTGROUP_DEVICE },
	{ FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E, "Device input short circuit.",       0, FAULTGROUP_IOU},
	{ FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E,  "Device input open circuit.",        0, FAULTGROUP_IOU },
	{ FAULT_SIGNAL_OUTPUT_FAULT,                "Device output fail.",               0,                      FAULTGROUP_IOU },
	{ FAULT_SIGNAL_INSTALLATION_TAMPER_E,       "Device installation tamper.",       FAULTFLAG_ALARM_DEVICE, FAULTGROUP_TAMPER },
	{ FAULT_SIGNAL_CASE_DISMANTLE_TAMPER_E,     "Device case dismantle tamper.",     FAULTFLAG_ALARM_DEVICE, FAULTGROUP_TAMPER },
	{ FAULT_SIGNAL_LOW_BATTERY_E,               "Device low battery.",               FAULTFLAG_ALARM_DEVICE, FAULTGROUP_BATTERY },
	{ FAULT_SIGNAL_BATTERY_ERROR_E,             "Device battery Error.",             FAULTFLAG_WARNING, FAULTGROUP_BATTERY },
	{ FAULT_SIGNAL_LOW_LINK_QUALITY_E,          "Device low link quality.",          FAULTFLAG_WARNING, FAULTGROUP_INSTALLATION  },
	{ FAULT_SIGNAL_LOW_PARENT_COUNT_FAULT_E,    "Device low parent count.",          FAULTFLAG_WARNING, FAULTGROUP_INSTALLATION },
	{ FAULT_SIGNAL_DEVICE_DROPPED_E,            "Device lost communication.",        FAULTFLAG_ALARM_DEVICE, FAULTGROUP_INSTALLATION },
	{ FAULT_SIGNAL_HEAD_DISMANTLE_E,            "Device head dismantle tamper.",     FAULTFLAG_ALARM_DEVICE, FAULTGROUP_TAMPER },
	{ FAULT_SIGNAL_FW_MISMATCH_E,               "Device firmware mismatch.",         0 | FAULTFLAG_LEVEL_2|FAULTFLAG_WARNING, FAULTGROUP_DEVICE },
									 
	{ FAULT_UNKNOWN_UNIT,                       "Unknown unit found on mesh.",                  0| FAULTFLAG_LEVEL_2 | FAULTFLAG_WARNING, FAULTGROUP_INSTALLATION },
	{ FAULT_INCORRECT_DEVICE_TYPE,				"Device type different to site configuration.", 0| FAULTFLAG_LEVEL_2 | FAULTFLAG_WARNING, FAULTGROUP_DEVICE },
                      
	{ FAULT_23, "", 0, 0 },
	{ FAULT_24, "", 0, 0 },
	{ FAULT_25, "", 0, 0 },
	{ FAULT_26, "", 0, 0 },
	{ FAULT_27, "", 0, 0 },
	{ FAULT_28, "", 0, 0 },
	{ FAULT_29, "", 0, 0 },
	{ FAULT_30, "", 0, 0 },
	
	{ FAULT_SIGNAL_NO_WARNINGS, "", 0, 0 },

	{ FAULT_WATCHDOG,                    "SYSTEM: Panel reset due to watchdog.", FAULTFLAG_SYSTEM, FAULTGROUP_SYSTEM },
	{ FAULT_BAD_SITE_FILE,               "SYSTEM: No site file / Bad site file.", FAULTFLAG_SYSTEM|FAULTFLAG_SAFE_STATE, FAULTGROUP_SYSTEM },
	 
	{ FAULT_NO_PSU_COMMS,                "POWER: PSU not responding.", FAULTFLAG_PSU, FAULTGROUP_GENERAL },
	{ FAULT_CRYSTAL_FAILURE,             "SYSTEM: Panel reset due to external crystal failure.", FAULTFLAG_SYSTEM | FAULTFLAG_LEVEL_2 |FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_NO_NCU_COMMS,                "SYSTEM: NCU not responding.", FAULTFLAG_SYSTEM, FAULTGROUP_SYSTEM },
	{ FAULT_NO_I2C1_COMMS,               "WARNING: Error code 01.", FAULTFLAG_WARNING, FAULTGROUP_GENERAL },
	{ FAULT_NO_QSPI_COMMS,               "WARNING: Error code 02.", FAULTFLAG_WARNING, FAULTGROUP_GENERAL },
			
	{ FAULT_MAINS_FAILURE,               "POWER: Panel mains power loss.", FAULTFLAG_PSU, FAULTGROUP_MAINS },
	{ FAULT_BATTERY_LOW,                 "POWER: Panel battery low.", FAULTFLAG_PSU, FAULTGROUP_BATTERY },
	{ FAULT_BATTERY_CRITICAL,            "POWER: Panel battery critical", FAULTFLAG_PSU, FAULTGROUP_BATTERY },
	{ FAULT_CHARGER_FAILED,              "POWER: Panel battery charge fail.", FAULTFLAG_PSU, FAULTGROUP_BATTERY },
	{ FAULT_PSU_CONFIG_ERROR,            "POWER: PSU configuration error.", FAULTFLAG_PSU, FAULTGROUP_GENERAL },
	{ FAULT_BATTERY_RESISTANCE_TOO_HIGH, "POWER: Panel battery resistance too high.", FAULTFLAG_PSU, FAULTGROUP_BATTERY },
	{ FAULT_PSU_OVER_VOLTAGE,            "POWER: Panel PSU over voltage.", FAULTFLAG_PSU, FAULTGROUP_GENERAL },
			
	{ FAULT_MISSING_ZONE_LEDS,           "Unexpected number of LED zone boards.", 0 | FAULTFLAG_LEVEL_2 | FAULTFLAG_WARNING , FAULTGROUP_GENERAL },
	{ FAULT_USB_OVERCURRENT,             "USB Over current. Remove device.", 0 | FAULTFLAG_LEVEL_2 |FAULTFLAG_WARNING, FAULTGROUP_GENERAL },

	{ FAULT_UNABLE_TO_MOUNT_SDCARD,      "Unable to mount SD Card.", 0, FAULTGROUP_GENERAL },
	{ FAULT_SD_CARD_NOT_DETECTED,        "SD Card not detected.", 0, FAULTGROUP_GENERAL },
	{ FAULT_UNABLE_TO_FORMAT_SD_CARD,    "Unable to format SD card.", 0, FAULTGROUP_GENERAL },
	{ FAULT_COULD_NOT_OPEN_LOG_FILE,     "Unable to open log file.", 0, FAULTGROUP_GENERAL },
	{ FAULT_LOG_CORRUPTION,              "Log corruption detected.", 0, FAULTGROUP_GENERAL },
			
	{ FAULT_USER_INPUT,                            "" },
	 
	{ FAULT_NO_I2C2_COMMS,      /*54*/    "WARNING: Error code 03.", FAULTFLAG_WARNING, FAULTGROUP_GENERAL },
	{ FAULT_EEPROM_COMMS,                 "WARNING: EEPROM Read/Write Error.", FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_BAD_I2C2_COMMS,               "WARNING: Error code 05.", FAULTFLAG_WARNING, FAULTGROUP_GENERAL  },
	{ FAULT_ADC_FAILURE,                  "Panel I/O failure.", 0, FAULTGROUP_GENERAL },

	{ FAULT_ROUTING_OUTPUT_SUPPLY,        "ROUTING: Fire routing supply failure.", FAULTFLAG_ROUTING_OUTPUT, FAULTGROUP_GENERAL },
	{ FAULT_SOUNDER1_SUPPLY,              "SOUNDER: Circuit 1 supply failure.", FAULTFLAG_SOUNDER, FAULTGROUP_GENERAL },
	{ FAULT_SOUNDER2_SUPPLY,              "SOUNDER: Circuit 2 supply failure.", FAULTFLAG_SOUNDER, FAULTGROUP_GENERAL },
			
	{ FAULT_CAE_ENGINE,                   "Bad Cause And Effect rule.", 0 | FAULTFLAG_LEVEL_2 | FAULTFLAG_WARNING, FAULTGROUP_GENERAL },
	{ FAULT_SITE_DATA_CORRUPTION,         "SYSTEM: Integrity check fail. Site may be corrupted.", FAULTFLAG_SYSTEM | FAULTFLAG_SAFE_STATE, FAULTGROUP_SYSTEM },
	{ FAULT_UNABLE_TO_READ_GSM_SETTINGS,  "Unable to retrieve GSM settings.", 0 | FAULTFLAG_LEVEL_2 | FAULTFLAG_WARNING, FAULTGROUP_GENERAL },
	{ FAULT_SD_ERROR,                     "SD Card fault.", 0, FAULTGROUP_GENERAL },

	{ FAULT_OVERLOAD_AUX,                 "Auxillary power overload.", 0 | FAULTFLAG_LEVEL_2, FAULTGROUP_GENERAL },
	{ FAULT_NETWORK_TX_FAIL,              "Network transmit fail.", 0| FAULTFLAG_LEVEL_2 |FAULTFLAG_WARNING, FAULTGROUP_GENERAL },
	{ FAULT_NETWORK_RX_FAIL,              "Network receive fail.", 0 | FAULTFLAG_LEVEL_2 | FAULTFLAG_WARNING, FAULTGROUP_GENERAL },
	{ FAULT_USB_TIMEOUT,                  "USB Transfer timeout.", FAULTFLAG_IS_LATCHED, FAULTGROUP_GENERAL },

	{ FAULT_UNABLE_TO_CONNECT_TO_SERVER,  "Unable to connect to server. Check site key/system time.", 0 | FAULTFLAG_LEVEL_2 | FAULTFLAG_WARNING, FAULTGROUP_GENERAL },

	{ FAULT_BAD_CREDENTIALS,			  "Bad app credentials, check app settings.", 0 | FAULTFLAG_LEVEL_2 | FAULTFLAG_WARNING, FAULTGROUP_GENERAL },
	{ FAULT_BATTERY_NOT_DETECTED,         "POWER: Panel battery critical/not detected.", FAULTFLAG_PSU, FAULTGROUP_BATTERY },
	{ FAULT_PSU_BATTERY_HEALTH,			  "POWER: Battery health suspect.", FAULTFLAG_PSU, FAULTGROUP_BATTERY },
	{ FAULT_ILLEGAL_SITE_FILE,			  "SYSTEM: Too many zones or devices in site file.", FAULTFLAG_SYSTEM | FAULTFLAG_LEVEL_2 | FAULTFLAG_SAFE_STATE|FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_ZONE_LED_FAIL,				  "Zone board failed to light zone.", 0 | FAULTFLAG_LEVEL_2 | FAULTFLAG_WARNING, FAULTGROUP_GENERAL },
	{ FAULT_OPEN_CIRCUIT,				  "Panel I/O open circuit.", 0, FAULTGROUP_GENERAL },
	{ FAULT_SHORT_CIRCUIT,				  "Panel I/O short circuit.", 0, FAULTGROUP_GENERAL },
	{ FAULT_EVENT_TRIGGERED,			  "Fault event triggered.", 0, FAULTGROUP_GENERAL },
	{ FAULT_SOUNDER_OPEN_CIRCUIT,		  "SOUNDER: Panel I/O open circuit.", FAULTFLAG_SOUNDER, FAULTGROUP_GENERAL },
	{ FAULT_SOUNDER_SHORT_CIRCUIT,		  "SOUNDER: Panel I/O short circuit.", FAULTFLAG_SOUNDER, FAULTGROUP_GENERAL },
	{ FAULT_ROUTING_OPEN_CIRCUIT,		  "ROUTING: Panel I/O open circuit.", FAULTFLAG_ROUTING_OUTPUT, FAULTGROUP_GENERAL },
	{ FAULT_ROUTING_SHORT_CIRCUIT,		  "ROUTING: Panel I/O short circuit.", FAULTFLAG_ROUTING_OUTPUT, FAULTGROUP_GENERAL },
	
	{ FAULT_AT_APP_URL_SET,             "WARNING: FAULT_AT_APP_URL_SET",       FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_AT_APP_APN_SET,             "WARNING: FAULT_AT_APP_APN_SET",       FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_AT_APP_PASSWORD_SET,        "WARNING: FAULT_AT_APP_PASSWORD_SET",  FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },

	{ FAULT_BACKLIGHT_PARAMS_GET,       "WARNING: FAULT_BACKLIGHT_PARAMS_GET", FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },

	{ FAULT_GSM_SMS_ENEBLED_GET,        "WARNING: FAULT_GSM_SMS_ENEBLED_GET", FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_GSM_APN_GET,                "WARNING: FAULT_GSM_APN_GET",         FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_GSM_APN_SET,                "WARNING: FAULT_GSM_APN_SET",         FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_GSM_PASSWORD_SET,           "WARNING: FAULT_GSM_PASSWORD_SET",    FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_GSM_PASSWORD_GET,           "WARNING: FAULT_SMS_ENEBLED_GET",     FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_GSM_SMS_ENEBLED_SET,        "WARNING: FAULT_GSM_PASSWORD_GET",    FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },

	{ FAULT_MEM_CHECH_SUM_GET,          "WARNING: FAULT_MEM_CHECK_SUM_GET",   FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_MEM_CHECH_SUM_SET,          "WARNING: FAULT_MEM_CHECK_SUM_SET",   FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	
	{ FAULT_MQTT_APP_ENABLED_GET,       "WARNING: FAULT_MQTT_APP_ENABLED_GET",  FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_MQTT_APP_SITE_GET,          "WARNING: FAULT_MQTT_APP_SITE_GET",     FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_MQTT_APP_URL_GET,           "WARNING: FAULT_MQTT_APP_URL_GET",      FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_MQTT_APP_APN_GET,           "WARNING: FAULT_MQTT_APP_APN_GET",      FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_MQTT_APP_PASSWORD_GET,      "WARNING: FAULT_MQTT_APP_PASSWORD_GET", FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },

	{ FAULT_MQTT_APP_ENABLED_SET,       "WARNING: FAULT_MQTT_APP_ENABLED_SET",  FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_MQTT_APP_SITE_SET,          "WARNING: FAULT_MQTT_APP_SITE_SET",     FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_MQTT_APP_URL_SET,           "WARNING: FAULT_MQTT_APP_URL_SET",      FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_MQTT_APP_APN_SET,           "WARNING: FAULT_MQTT_APP_APN_SET",      FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_MQTT_APP_PASSWORD_SET,      "WARNING: FAULT_MQTT_APP_PASSWORD_SET", FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },

	{ FAULT_SETTINGS_PREVIOUS_ADDR_SET, "WARNING: FAULT_SETTINGS_PREVIOUS_ADDR_SET", FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_SETTINGS_CURRENT_ADDR_SET,  "WARNING: FAULT_SETTINGS_CURRENT_ADDR_SET",  FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_SETTINGS_PREVIOUS_ADDR_GET, "WARNING: FAULT_SETTINGS_PREVIOUS_ADDR_GET", FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_SETTINGS_CURRENT_ADDR_GET,  "WARNING: FAULT_SETTINGS_CURRENT_ADDR_GET",  FAULTFLAG_WARNING, FAULTGROUP_SYSTEM },
	{ FAULT_TOUCH_PARAMS_GET,           "WARNING: FAULT_TOUCH_PARAMS_GET",           FAULTFLAG_WARNING, FAULTGROUP_SYSTEM }

};
/* NOW missing infor from GUI - kept for reference									
	if ( cmd.short4 & 2 ) 
     {
        type =  EwNewStringAnsi("SYSTEM FAULT");
     }
     else if ( cmd.int2 & 16 ) 
     {
        type = EwNewStringAnsi("PANEL PSU FAULT");
     }
     else if ( cmd.int2 & 64 ) 
     {
        type = EwNewStringAnsi("OPEN CIRCUIT FAULT");
     }       
     else if ( cmd.int2 & 128 )
     {
        type = EwNewStringAnsi("SHORT CIRCUIT FAULT");
     }
     else if ( cmd.int2 & 32 ) 
     {
        type = EwNewStringAnsi("SOUNDER FAULT");
     }
     else if ( cmd.int2 & 256 )
     {
        type =  EwNewStringAnsi("NETWORK FAULT");
     }
     else if ( cmd.int2 & 16384 )
     {
        type =  EwNewStringAnsi("WARNING");
     }
     else
     {
        type =  EwNewStringAnsi("GENERAL FAULT");
     }
								 };
*/

XString AppLibClass_TypeName( AppLibClass _this, XEnum type );



static char typebuff[ 96 ];


/*************************************************************************/
/**  \fn      char* GetTypeName( ChannelType type )
***  \brief   Helper function
**************************************************************************/

char* GetTypeName( ChannelType type )
{
	XString xs = AppLibClass_TypeName( EwGetAutoObject( &AppLib, AppLibClass ), (XEnum) type );
	
	if ( EwStringToAnsi( xs, typebuff, 96, '-' ) )
	{	
		return typebuff;
	}
	else return (char*)"";
}
	

/*************************************************************************/
/**  \fn      int Fault::Fault( )
***  \brief   Constructor class function
**************************************************************************/
								 
Fault::Fault( ) : Module( "Fault", 0, EVENT_CLEAR | EVENT_UPDATE | EVENT_SILENCE_BUTTON | EVENT_RESOUND_BUTTON | EVENT_MUTE_BUZZER_BUTTON ) 
{	
	ew = (EmbeddedWizard*) app.FindModule( "EmbeddedWizard" );
	gsm      =  (GSM*)	  app.FindModule( "GSM" );
	
	assert( ew != NULL );
 
}	

/*************************************************************************/
/**  \fn      void LogFault( char* msg, char zone, short unit, char chan )
***  \brief   Helper function
**************************************************************************/

static void LogFaultEnd( const char* msg, char zone, short unit, char chan )
{
	char buff[ 96 ] = "";
	char* p = buff;
	int n = 96;
	
	int a = snprintf( p, n, "CLEARED ");
	p += a;
	n -= a;

	if ( zone != 255 )
	{
		a = snprintf( p, n, "Zone %d ", zone );
		p += a;
		n -= a;
	}
	if ( unit != -1 )
	{
		a = snprintf( p, n, "Unit %d ", unit );
		p += a;
		n -= a;
	}
	if ( chan != 0 )
	{
		a = snprintf( p, n, "(%s) ", GetTypeName( (ChannelType) chan ) ); 
		p += a;
		n -= a;	
	}
	
	snprintf( p, n, msg );
	
	Log::Msg( LOG_FLT, buff );
}

/*************************************************************************/
/**  \fn      void LogFault( char* msg, char zone, short unit, char chan )
***  \brief   Helper function
**************************************************************************/

static void LogFault( const char* msg, char zone, short unit, char chan )
{
	char buff[ 96 ] = "";
	char* p = buff;
	int n = 96;

	if ( zone != 255 )
	{
		int a = snprintf( p, n, "Zone %d ", zone );
		p += a;
		n -= a;
	}
	if ( unit != -1 )
	{
		int a = snprintf( p, n, "Unit %d ", unit );
		p += a;
		n -= a;
	}
	if ( chan != 0 )
	{
		int a = snprintf( p, n, "(%s) ", GetTypeName( (ChannelType) chan ) ); 
		p += a;
		n -= a;	
	}
	
	snprintf( p, n, msg );
	
	Log::Msg( LOG_FLT, buff );
}


/*************************************************************************/
/**  \fn      int Fault::NumFaults(  )
***  \brief   Class function
**************************************************************************/

int Fault::NumFaults( int& extra, int& reduced )
{
	int count = 0;
	
	extra = false;
	reduced = false;
				
	for( FaultItem* fi = fault_list; fi < fault_pos; fi++ )
	{
		if ( IsValid( fi ) )
		{
			if ( fi->flags & FAULTFLAG_HIDDEN )
			{
				extra = true;
				fi->flags &= ~FAULTFLAG_HIDDEN;
			}
			
			count++;
		}
		else
		{
			if ( ! ( fi->flags & FAULTFLAG_HIDDEN ) )
			{
				reduced = true;
				fi->flags |= FAULTFLAG_HIDDEN;
			}
		}
		
	}
	return count;
}


/*************************************************************************/
/**  \fn      int Fault::AddFault( const char* msg, char zone, short unit, char chan, int flags, bool latching )
***  \brief   Class function
**************************************************************************/

void Fault::AddFault( const char* msg, char zone, short unit, char chan, int flags, bool latching )
{
	if ( fault_pos - fault_list >= MAX_FAULTS )
	{
		return;
	}
		 
	
	for( FaultItem* ft = fault_list; ft < fault_pos; ft++ )
	{
		if ( !strcmp( ft->msg, msg ) && ( ft->flags & flags ) == flags && chan == ft->chan && zone == ft->zone && unit == ft->unit )
		{
			return;		// fault already reported
		}
	}
	
	app.DebOut( "Adding fault '%s'\n", msg );
		
	// Log fault
	
	LogFault( msg, zone, unit, chan ); 
	
	bool alarm_device = false;
	
	if ( ( flags & FAULTFLAG_ALARM_DEVICE ) && unit != -1 )
	{
		DeviceConfig* dc = Settings::FindConfig( unit );
		
		if ( dc != NULL ) if ( dc->IsAlarmOutputDevice( ) )
		{
			alarm_device = true;
		}
	}
	
	
	fault_pos->type 	= FAULT_USER_INPUT;
	fault_pos->time 	= now( );
	fault_pos->flags 	= FAULTFLAG_HIDDEN | flags | ( latching ? FAULTFLAG_LATCH : 0  | ( ( alarm_device && ( flags & FAULTFLAG_ALARM_DEVICE ) ) ? FAULTFLAG_SOUNDER : 0 ) );
	fault_pos->msg 	= msg;
	fault_pos->zone   = zone;
	fault_pos->unit   = unit;
	fault_pos->chan   = chan;
	
	CheckFlags( fault_pos );
	fault_pos++;

	if ( gsm != NULL ) gsm->ProcessFault( FAULTGROUP_GENERAL );	
}


/*************************************************************************/
/**  \fn      void Fault::CheckFlags( int flags )
***  \brief   Class function
**************************************************************************/

void Fault::CheckLEDs( )
{
	sounder_faulty = false;
	fire_routing_faulty = false;
	
	bool psu_faulty = false;
	bool routing_faulty = false;
	bool sys_faulty = false;
	bool gen_faulty = false;
	bool buzz = false;
	
	for( FaultItem* fi = fault_list; fi < fault_pos; fi++ )
	{
		if ( IsValid( fi ) )
		{
			if ( fi->flags & FAULTFLAG_PSU )
			{
				psu_faulty = true;
			}
			
			if ( fi->flags & FAULTFLAG_SOUNDER )
			{
				sounder_faulty = true;
			}
			
			if ( fi->flags & FAULTFLAG_ROUTING_OUTPUT )
			{
				fire_routing_faulty = true;
			}
			
			if ( fi->flags & FAULTFLAG_SYSTEM )
			{
				sys_faulty = true;
			}
			
			if ( !( fi->flags & FAULTFLAG_WARNING ) )
			{
				// LED::Flash( GPIO_GenFault );		
				 gen_faulty = true;
			}	
			
			if ( !( fi->flags & FAULTFLAG_MUTED ) )
			{
				buzz = true;
			}
		}
	}
	
	if ( psu_faulty )
	{
		LED::Flash( GPIO_PSUFault );
	}
	else
	{
		LED::Off( GPIO_PSUFault );
	}
	
	if ( sys_faulty )
	{
		LED::Flash( GPIO_SysFault );
	}
	else
	{
		LED::Off( GPIO_SysFault );
	}
	
	if ( gen_faulty )
	{
		LED::Flash( GPIO_GenFault );		
	}
	else
	{
		LED::Off( GPIO_GenFault );		
	}
	
	if ( buzz )
	{
		Buzzer::Buzz( BUZZ_FOR_FAULT );
	}
	else
	{
		Buzzer::Mute( BUZZ_FOR_FAULT );
	}
	
	Settings::UpdateRoutingSounderLEDs( );	
}


/*************************************************************************/
/**  \fn      void Fault::CheckFlags( int flags )
***  \brief   Class function
**************************************************************************/

void Fault::CheckFlags( FaultItem* fi )
{
	if ( IsValid( fi ) )
	{
		if ( fi == fault_list && fi->type == FAULT_MAINS_FAILURE )
		{
			last_fault_activity = now( );
		}
		else
		{
			last_fault_activity = PREVENT_BLANK;
		}
		
		if ( fi->flags & FAULTFLAG_PSU )
		{
			LED::Flash( GPIO_PSUFault );
		}
		
		if ( fi->flags & FAULTFLAG_SOUNDER )
		{
			sounder_faulty = true;
		}
		
		if ( fi->flags & FAULTFLAG_ROUTING_OUTPUT )
		{
			fire_routing_faulty = true;
		}
		
		Settings::UpdateRoutingSounderLEDs( );
		
		if ( fi->flags & FAULTFLAG_SYSTEM )
		{
			LED::Flash( GPIO_SysFault );
		}
		
		if ( !( fi->flags & FAULTFLAG_WARNING ) )
		{
			LED::Flash( GPIO_GenFault );		
			
			// Sound buzzer
			Buzzer::Buzz( BUZZ_FOR_FAULT );
		}	

		if ( fi->flags & FAULTFLAG_SAFE_STATE )
		{
			Message msg;
			msg.type = EW_ENTER_SAFE_STATE_MSG;
			msg.to = ew;
			msg.value = true;
			
			app.Send( &msg );		

			app.site = NULL;
			app.panel = NULL;
		}

		UpdateGUI( true );
	}
	
  if ( (fi->flags & FAULTFLAG_LEVEL_2) && Utilities::AccessLevel == 2 )
	{
		UpdateGUI( true );
	}

}
	

/*************************************************************************/
/**  \fn      void Fault::UpdateGUI( )
***  \brief   Class function
**************************************************************************/

void Fault::UpdateGUI( bool jumpToScreen )
{
	// Update display
	if ( ew != NULL )
	{
		Message msg;
		msg.type = EW_UPDATE_FAULT_LIST_MSG;
		msg.value = jumpToScreen;
		msg.to = ew;
		
		app.Send( &msg );
	}
}


	
/*************************************************************************/
/**  \fn      int Fault::AddFault( FaultType t, char zone, short unit, char chan, bool latching )
***  \brief   Class function
**************************************************************************/

int stopfualt = 0;

void Fault::AddFault( FaultType t, char zone, short unit, char chan, bool latching )
{
	DeviceConfig* dc = NULL;
	
	if ( fault_pos - fault_list >= MAX_FAULTS )
	{
		return;
	}
	
	if ( unit > 0 && unit < 520 ) 
	{
		dc = Settings::FindConfig( unit );
		
		if ( dc != NULL ) zone = dc->zone;
	}
	
	for( FaultItem* ft = fault_list; ft < fault_pos; ft++ )
	{
		if ( ft->type == t && ft->zone == zone && ft->unit == unit && ft->chan == chan )
		{
			return;		// fault already reported
		}
	}			 
					
	if ( t < MAX_DEVICE_FAULT_NUM )
	{	
		bool faults = 0;
		bool warnings = 0;
		RadioUnit* ru = NCU::FindUnit( unit );
		
		if ( ru != NULL )
		{
			if ( ru->valid & VALID_MORE_FAULTS )
			{
				faults = 1;
			}
			if ( ru->valid & VALID_MORE_WARNINGS )
			{
				warnings = 1;
			}
		}
		AppMessage msg( unit, t, chan, true, faults, warnings );
		
#ifdef GSM_HTTP			
			AppSupport::Send( msg );
#else
			MQTTSupport::Send( msg );
#endif	
	}
	else
	{
		AppMessage msg( t, true );

#ifdef GSM_HTTP			
			AppSupport::Send( msg );
#else
			MQTTSupport::Send( msg );
#endif	
	}
	
	app.DebOut( "Adding fault '%s'\n", fault[ t].msg );
	
	const FaultDef* f = fault + t;
	 
	// Log fault
	
	LogFault( f->msg, zone, unit, chan ); 
	if ( f->flags & FAULTFLAG_SYSTEM ) latching = true;		

	bool alarm_device = false;
	
	if ( ( f->flags & FAULTFLAG_ALARM_DEVICE ) && unit != -1 )
	{
		DeviceConfig* dc = Settings::FindConfig( unit );
		
		if ( dc != NULL ) if ( dc->IsAlarmOutputDevice( ) )
		{
			alarm_device = true;
		}
	}
		
	fault_pos->type 	= t;
	fault_pos->time 	= now( );
	fault_pos->flags 	= FAULTFLAG_HIDDEN | f->flags | ( latching ? FAULTFLAG_LATCH : 0  | ( ( alarm_device && ( f->flags & FAULTFLAG_ALARM_DEVICE ) ) ? FAULTFLAG_SOUNDER : 0 ) );
	fault_pos->msg 	= f->msg; 
	fault_pos->zone   = zone;
	fault_pos->unit   = unit;
	fault_pos->chan   = chan;
		
	CheckFlags( fault_pos );
	fault_pos++;
	
	if ( gsm != NULL ) gsm->ProcessFault( f->group );
}
	

/*************************************************************************/
/**  \fn      int Fault::RemoveFault( FaultType t )
***  \brief   Class function
**************************************************************************/

void Fault::RemoveFault( FaultType t, char zone, short unit, char channel, int flags, bool ignore, bool sendtoserver )
{
	FaultItem* found = NULL;
	
	app.DebOut( "Removing fault '%s'\n", t == FAULT_ANY ? "<multiple>" : fault[ t].msg );
		
	for( FaultItem* ft = fault_list; ft < fault_pos; ft++ )
	{
		// if not found yet
		if ( found == NULL )
		{
			// if matching fault
			if ( ( ft->type == t 	   || (char)t == (char)FAULT_ANY ) &&
				  ( ft->zone == zone    || zone    == 0 ) && 
				  ( ft->unit == unit    || unit    == -1 ) &&
			     ( ft->chan == channel || channel == 0 ) ) 
			{
				if ( flags )
				{
					// if ingoring flag
					if ( ignore )
					{
						if ( ft->flags & flags ) continue;
					}
					else
					{
						if ( !( ft->flags & flags ) ) continue;
					}
				}
				if ( ft->flags & FAULTFLAG_LATCH )
				{
					// dont delete, just latch
					ft->flags |= FAULTFLAG_IS_LATCHED;
				}
				else
				{
					// we've found the fault to delete
					found = ft;
					 
					// log it
					LogFaultEnd( fault[ found->type ].msg, found->zone, found->unit, found->chan ); 
					
					if ( sendtoserver )
					{
						// is it e device fault?
						if ( found->type < MAX_DEVICE_FAULT_NUM )
						{		
							bool faults = 0;
							bool warnings = 0;
							RadioUnit* ru = NCU::FindUnit( found->unit );
							
							if ( ru != NULL )
							{
								if ( ru->valid & VALID_MORE_FAULTS )
								{
									faults = 1;
								}
								if ( ru->valid & VALID_MORE_WARNINGS )
								{
									warnings = 1;
								}
							}
							// create device fault msg
							AppMessage msg( found->unit, (FaultType) found->type, found->chan, false, faults, warnings );

				#ifdef GSM_HTTP			
							AppSupport::Send( msg );
				#else
							MQTTSupport::Send( msg );
				#endif	
						}
						else 
						{
							// create panel fault msg
							AppMessage msg( (FaultType) found->type, false );
							
				#ifdef GSM_HTTP			
							AppSupport::Send( msg );
				#else
							MQTTSupport::Send( msg );
				#endif	
						}
					}
				}
			}
		}
		else
		{
			ft[-1] = *ft;
		}
	}
	
	// if we found it
	if ( found != NULL )
	{	 
		// adjust count
		fault_pos--;
	 
		// update LEDs
		CheckNoFaults( );	 
		
		// update GUI
		UpdateGUI( false );
	}
}


/*************************************************************************/
/**  \fn      void Fault::RemoveAllFaults( short unit )
***  \brief   Class function
**************************************************************************/

void Fault::RemoveAllFaults( short unit )
{
	FaultItem* prev = Fault::fault_pos;
	FaultItem* last;

	do
	{	
		last = Fault::fault_pos;		
		RemoveFault( FAULT_ANY, 0, unit, 0, FAULTFLAG_WARNING, true, false );
	} while ( last != Fault::fault_pos );	
	
	// create device fault msg
	AppMessage msg( unit, (FaultType) FAULT_SIGNAL_NO_ERROR_E, (int) IOT_ClearDeviceFaults, false, false, false );

	MQTTSupport::Send( msg );
}

/*************************************************************************/
/**  \fn      void Fault::RemoveAllWarnings( short unit )
***  \brief   Class function
**************************************************************************/

void Fault::RemoveAllWarnings( short unit )
{
	FaultItem* prev = Fault::fault_pos;
	FaultItem* last;
	
	do
	{	
		last = Fault::fault_pos;		
		RemoveFault( FAULT_ANY, 0, unit, 0, FAULTFLAG_WARNING, false, false );
	} while ( last != Fault::fault_pos );	

 // create device fault msg
	AppMessage msg( unit, (FaultType) FAULT_SIGNAL_NO_ERROR_E, (int) IOT_ClearDeviceWarnings, false, false, false );

	MQTTSupport::Send( msg );
	 
}


/*************************************************************************/
/**  \fn      void Fault::RemoveAllFaultsAndWarnings( short unit )
***  \brief   Class function
**************************************************************************/

void Fault::RemoveAllFaultsAndWarnings( short unit )
{
	FaultItem* prev = Fault::fault_pos;
	FaultItem* last;
	
	do
	{	
		last = Fault::fault_pos;		
		RemoveFault( FAULT_ANY, 0, unit, 0, 0, false, false );
	} while ( last != Fault::fault_pos );	
	
 // create device fault msg
	
	AppMessage msg( unit, (FaultType) FAULT_SIGNAL_NO_ERROR_E, (int) IOT_ClearDeviceFaultsAndWarnings, false, false, false );

	RadioUnit* ru = NCU::FindUnit( unit );
	
	if ( ru != NULL )
	{
		if ( ru->state == UNIT_DROPPED ) 
		{
			Fault::AddFault( FAULT_SIGNAL_DEVICE_DROPPED_E, ru->zone, unit, 0 );
		}
		else if ( ru->state != UNIT_UNKNOWN )
		{
			DeviceConfig* conf = Settings::FindConfig( unit );
			
			if ( conf != NULL )
		   {                       
				if ( conf->type != ru->combination )
				{
					Fault::AddFault( FAULT_INCORRECT_DEVICE_TYPE, conf->zone, unit, 0 );
				}
			}
		}
	}
	
	MQTTSupport::Send( msg );
}



/*************************************************************************/
/**  \fn      void Fault::RemoveAllPanelFaultsAndWarnings( void )
***  \brief   Class function
**************************************************************************/

void Fault::RemoveAllPanelFaultsAndWarnings( void )
{
	for( FaultItem* ft = fault_list; ft < fault_pos; ft++ )
	{
		if ( ft->unit == -1 || ft->unit > 511 )
		{
			RemoveFault( ft );
			ft--;
		}
	}
	
	UpdateGUI( false );

	CheckNoFaults( );	
}


/*************************************************************************/
/**  \fn      void Fault::RemoveAllFaults( FaultType t)
***  \brief   Class function
**************************************************************************/

void Fault::RemoveAllFaults( FaultType t )
{
	FaultItem* prev = Fault::fault_pos;
	FaultItem* last;
	
	do
	{	
		last = Fault::fault_pos;		
		RemoveFault( t, 0, -1, 0 );
	} while ( last != Fault::fault_pos );	
	 
}


/*************************************************************************/
/**  \fn      int Fault::RemoveFault( FaultItem* fi )
***  \brief   Class function
**************************************************************************/

void Fault::RemoveFault( FaultItem* fi )
{
	int found = false;
 
	for( FaultItem* ft = fi; ft < fault_pos; ft++ )
	{
		*ft = ft[ 1];
	}
	
	fault_pos--;
	
	TouchScreen::last_tap = now( );
}


/*************************************************************************/
/**  \fn      int Fault::RemoveFault( char* msg )
***  \brief   Class function
**************************************************************************/

void Fault::RemoveFault( const char* msg, char zone, short unit, char chan  )
{
	int found = false;
	
	app.DebOut( "Removing fault '%s'\n", msg );
		
	for( FaultItem* ft = fault_list; ft < fault_pos; ft++ )
	{
		if ( ft->msg == msg && ft->zone == zone && ft->unit == unit && ft->chan == chan ) 
		{
			if ( ft->flags & FAULTFLAG_LATCH )
			{
				ft->flags |= FAULTFLAG_IS_LATCHED;
			}
			else
			{
				found = true;
			}
		}
		
		if ( found )
		{
			*ft = ft[ 1];
		}
	}
	
	if ( found )
	{
		fault_pos--;
		LogFaultEnd( msg, zone, unit, chan ); 
		
		CheckNoFaults( );
		
		UpdateGUI( false );
	}
}
 

/*************************************************************************/
/**  \fn      void Fault::CheckNoFaults( )
***  \brief   Class function
**************************************************************************/

void Fault::CheckNoFaults( ) 
{
	bool nosysfaults = true;
	bool nopsufaults = true;
	bool nosndfaults = true;
	bool nogenfaults = true;
	bool noroufaults = true;
	bool allmuted    = true;
	bool has_faults_other_than_no_mains = false;
	
	for( FaultItem* ft = fault_list; ft < fault_pos; ft++ )
	{
		if ( IsValid( ft ) )
		{
			if ( ft->flags & FAULTFLAG_SYSTEM )  			nosysfaults = false;
			if ( ft->flags & FAULTFLAG_PSU ) 	 			nopsufaults = false;
			if ( ft->flags & FAULTFLAG_SOUNDER ) 			nosndfaults = false;
			if ( ft->flags & FAULTFLAG_ROUTING_OUTPUT ) 	noroufaults = false;
			if ( ! (ft->flags & FAULTFLAG_WARNING ) )		nogenfaults = false;
			if ( ! (ft->flags & FAULTFLAG_MUTED ) )		allmuted = false;
			if ( ft->type != FAULT_MAINS_FAILURE ) 		has_faults_other_than_no_mains = true;
		}
	}
	
	if ( nosysfaults ) LED::Off( GPIO_SysFault );
	if ( nopsufaults ) LED::Off( GPIO_PSUFault );
	
	if ( !has_faults_other_than_no_mains )
	{
		if ( last_fault_activity == PREVENT_BLANK )
		{
			last_fault_activity = now( );
		}
	}
	else
	{
		last_fault_activity = PREVENT_BLANK;
	}
	
	if ( nosndfaults ) 
	{
		sounder_faulty = false;
	}
	if ( noroufaults )
	{
		fire_routing_faulty = false;
	}
	
	Settings::UpdateRoutingSounderLEDs( );
	
	if ( nogenfaults )
	{ 
		LED::Off( GPIO_GenFault );			
	}
	if ( allmuted )
	{
		Buzzer::Mute( BUZZ_FOR_FAULT );
	}
}


/*************************************************************************/
/**  \fn      int Fault::Receive( Message* msg )
***  \brief   Class function
**************************************************************************/
	
int Fault::Receive( Message* msg )
{
	switch( msg->type )
	{
	
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int Fault::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int Fault::IsValid( FaultItem* fi )
{
	if( ! (fi->flags & FAULTFLAG_WARNING ) || ((Utilities::AccessLevel > 1 ) && (Utilities::AccessLevel < 4 )))
	{
			if ( fi->unit == -1 ) return true;
	}
	
	if ( ! (fi->flags & FAULTFLAG_WARNING ) || (Utilities::AccessLevel == 4) )
	{
		if ( fi->zone == 255 || !Settings::ZoneDisabled( fi->zone ) )
		{
			if ( fi->unit == -1 ) return true;
			
			Device* d = IOStatus::FindDevice( fi->unit );
			 
			DeviceConfig* config;
			
			if ( d != NULL )
			{
				if ( d->IsDisabled( ) ) return false;
				config = d->config;
			}
			else
			{
				config = Settings::FindConfig( fi->unit );
				
				if ( config->IsDisabled( ) ) return false;
			}
			 
			if ( fi->chan == 0 ) return true;
			 
			int i;
			 
			if ( config->IsInputChannel( fi->chan, i ) )
			{
				if ( d != NULL )
				{
					if ( !d->IsInputDisabled( i ) ) return true;
				}
				else
				{
					if ( !config->IsInputDisabled( i ) ) return true;
				}
			}
			 
			if ( config->IsOutputChannel( fi->chan, i ) )
			{
				if ( d != NULL )
				{
					if ( !d->IsOutputDisabled( i ) ) return true;
				}
				else
				{
					if ( !config->IsOutputDisabled( i ) ) return true;
				}
			}
		}
	}
	return false;
}
	




/*************************************************************************/
/**  \fn      int Fault::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int Fault::Receive( Command* cmd ) 
{
	assert( cmd != NULL );
	
	switch ( cmd->type )
	{		
		case CMD_GET_NUM_FAULTS:
		{
			CheckLEDs( );
			// if num changes - so can LEDs!
			
			cmd->int0 = Fault::NumFaults( cmd->int1, cmd->int2 );	
			
			return CMD_OK;
		}
	 		
		case CMD_GET_FAULT_ITEM:
		{
			FaultItem* fi;
			
			int count = 0;
			
			for( fi = fault_list; fi < fault_pos; fi++ )
			{
				if ( IsValid( fi ) )
				{
					if ( count++ == cmd->int0 )
					{
						break;
					}
				}
			}
			 
			
			if ( fi < fault_pos && fi >= fault_list )
			{
				static char tstr[ 20 ];
				
				struct tm *tp = localtime( &fi->time );
				
				snprintf( tstr, 20, "%02d:%02d %02d/%02d/%02d", tp->tm_hour, tp->tm_min, tp->tm_mday, tp->tm_mon+1, tp->tm_year - 100 );
				
				cmd->char0   = fi->zone;
				cmd->char1   = fi->chan;
				cmd->ushort1 = fi->unit;				
				cmd->int1    = (int) fi->msg;
				cmd->int3    = (int) tstr;
				cmd->ushort5 = fi->chan;
				cmd->int2    = 0;
				
				if ( fi->zone != 255 && fi->unit != -1 )
				{
					DeviceConfig* dc = Settings::FindConfig( fi->unit );
					
					if ( dc != NULL )
					{
						cmd->int2 = (int) dc->location;
					}
				}
				return CMD_OK;
			}
			else
			{
				cmd->int1 = 0;
				cmd->int2 = 0;
				cmd->int3 = 0;
				
				return CMD_ERR_OUT_OF_RANGE;
			}
		}
	}
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int Fault::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int Fault::Receive( Event event ) 
{	
	switch ( event )
	{
		case EVENT_SILENCE_BUTTON :
		{
			for( FaultItem* ft = fault_list; ft < fault_pos; ft++ )
			{
				if ( IsValid( ft ) )
				{
					ft->flags |= FAULTFLAG_SILENCED;
				}
			}
			break;
		}
		
		case EVENT_RESOUND_BUTTON :
		{
			for( FaultItem* ft = fault_list; ft < fault_pos; ft++ )
			{
				if ( IsValid( ft ) )
				{
					ft->flags &= ~FAULTFLAG_SILENCED;
				}
			}
			break;
		}		
		
		case EVENT_MUTE_BUZZER_BUTTON :
		{
			for( FaultItem* ft = fault_list; ft < fault_pos; ft++ )
			{
				if ( IsValid( ft ) )
				{
					ft->flags |= FAULTFLAG_MUTED;
				}
			}
			break;
		}
		
		case EVENT_CLEAR :
		{
			RemoveAllPanelFaultsAndWarnings( );
			
			CheckNoFaults();

#ifdef APP_FAULT_REMOVAL			
			AppSupport::Send( APP_INFO_PANEL_FAULT, 0, 0 );
			AppSupport::Send( APP_INFO_DEVICE_FAULT, 0, FAULT_SIGNAL_NO_WARNINGS );
			AppSupport::Send( APP_INFO_DEVICE_FAULT, 0, FAULT_SIGNAL_NO_ERROR_E );
#endif			
			// Clear safe screen
			Message msg;
			msg.type = EW_ENTER_SAFE_STATE_MSG;
			msg.to = ew;
			msg.value = false;
			
			app.Send( &msg );	
			break;
		}
	 
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int Fault::GetFaultStatus( int& silenceable_status, int& unsilenceable_status )
***  \brief   Class function
**************************************************************************/

void Fault::GetFaultStatus( int& silenceable_status, int& unsilenceable_status )
{
	if ( app.site != NULL ) for( FaultItem* ft = fault_list; ft < fault_pos; ft++ )
	{		
		if ( IsValid( ft ) )
		{
			// if not a silent fault or a warning
			if ( !( ft->flags & FAULTFLAG_DONT_ALARM ) && !( ft->flags & FAULTFLAG_WARNING ) )
			{
				// if zone in range
				if ( ft->zone > 0 && ft->zone <= 96 )
				{
					// if zone on test
					if ( IOStatus::zoneflags[ app.site->currentIndex ][ft->zone] & ZONE_ON_TEST )
					{
						// ignore fault
						continue;
					}
				}
				// if unit in range	
				if ( ft->unit > 0 && ft->unit < 520 ) 
				{
					// get device from unit
					Device* dev = IOStatus::FindDevice( ft->unit );
					
					if ( dev != NULL )
					{
						// if device on test ignore..
						if ( dev->settings & SETTING_ON_TEST ) continue;
						
						DeviceConfig* dc = dev->config;
			
						// for each input channel
						for( InputChannel* i = dc->input; i < dc->input + dc->numInputs; i++ )
						{
							// if fault chan matches
							if ( i->type == ft->chan )
							{
								// if channel on test
								if ( dev->IsInputOnTest( i - dc->input ) )
								{
									// ignore
									continue;
								}
							}
						}
					}			
				}
		 
				unsilenceable_status |= ACTION_BIT_FAULT;
				
				if ( !(ft->flags & FAULTFLAG_SILENCED ) )
				{
					silenceable_status |= ACTION_BIT_FAULT;
				}
			}
		}
	}
}
				
