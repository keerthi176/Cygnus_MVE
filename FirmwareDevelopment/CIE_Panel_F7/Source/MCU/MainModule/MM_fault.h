/***************************************************************************
* File name: MM_fault.h
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
* Fault define list.
*
**************************************************************************/
#ifndef _FAULT_H_
#define _FAULT_H_

/* System Include Files
**************************************************************************/
#include <time.h>


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Module.h"
#include "MM_GPIO.h"
#include "MM_EmbeddedWizard.h"


/* Defines
**************************************************************************/

 
typedef enum
{
	FAULT_SIGNAL_NO_ERROR_E = 0,                 //0
   FAULT_SIGNAL_TYPE_MISMATCH_E,                //1
   FAULT_SIGNAL_DEVICE_ID_MISMATCH_E,           //2
   FAULT_SIGNAL_FAULTY_SENSOR_E,                //3
   FAULT_SIGNAL_DIRTY_SENSOR_E,                 //4
   FAULT_SIGNAL_FAULTY_DIRTY_SENSOR_E,          //5
   FAULT_SIGNAL_INTERNAL_FAULT_E,               //6
   FAULT_SIGNAL_SOUNDER_FAULT_E,                //7
   FAULT_SIGNAL_BEACON_LED_FAULT,               //8
   FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E,    //9
   FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E,     //10
   FAULT_SIGNAL_OUTPUT_FAULT,                   //11
   FAULT_SIGNAL_INSTALLATION_TAMPER_E,          //12
   FAULT_SIGNAL_CASE_DISMANTLE_TAMPER_E,        //13
   FAULT_SIGNAL_LOW_BATTERY_E,                  //14
   FAULT_SIGNAL_BATTERY_ERROR_E,                //15
   FAULT_SIGNAL_LOW_LINK_QUALITY_E,             //16
   FAULT_SIGNAL_LOW_PARENT_COUNT_FAULT_E,       //17
   FAULT_SIGNAL_DEVICE_DROPPED_E,               //18
	FAULT_SIGNAL_HEAD_DISMANTLE_E,					//19
	FAULT_SIGNAL_FW_MISMATCH_E,						//20

	FAULT_UNKNOWN_UNIT,									// 21
	FAULT_INCORRECT_DEVICE_TYPE,						// 22
	FAULT_23,
	FAULT_24,
	FAULT_25,
	FAULT_26,
	FAULT_27,
	FAULT_28,
	FAULT_29,
	FAULT_30,
	FAULT_SIGNAL_NO_WARNINGS,
	
	FAULT_WATCHDOG,
	FAULT_BAD_SITE_FILE,
	FAULT_NO_PSU_COMMS,
	FAULT_CRYSTAL_FAILURE,
	FAULT_NO_NCU_COMMS,	// 36
	FAULT_NO_I2C1_COMMS,  //  37
	FAULT_NO_QSPI_COMMS,  		  // 38	
   FAULT_MAINS_FAILURE,						
   FAULT_BATTERY_LOW,							
   FAULT_BATTERY_CRITICAL,	 
   FAULT_CHARGER_FAILED,						
   FAULT_PSU_CONFIG_ERROR,					
   FAULT_BATTERY_RESISTANCE_TOO_HIGH,	
   FAULT_PSU_OVER_VOLTAGE,		
	FAULT_MISSING_ZONE_LEDS,
	FAULT_USB_OVERCURRENT,
	FAULT_UNABLE_TO_MOUNT_SDCARD,
	FAULT_SD_CARD_NOT_DETECTED,
	FAULT_UNABLE_TO_FORMAT_SD_CARD,
	FAULT_COULD_NOT_OPEN_LOG_FILE,
	FAULT_LOG_CORRUPTION,	
	FAULT_USER_INPUT,


   FAULT_NO_I2C2_COMMS, // 54
	FAULT_EEPROM_COMMS,
	FAULT_BAD_I2C2_COMMS,
	FAULT_ADC_FAILURE,
	FAULT_ROUTING_OUTPUT_SUPPLY,
	FAULT_SOUNDER1_SUPPLY,
	FAULT_SOUNDER2_SUPPLY,	
	FAULT_CAE_ENGINE,
	FAULT_SITE_DATA_CORRUPTION,	
	FAULT_UNABLE_TO_READ_GSM_SETTINGS,
	FAULT_SD_ERROR,	
	FAULT_OVERLOAD_AUX,	
	FAULT_NETWORK_TX_FAIL,
	FAULT_NETWORK_RX_FAIL,
	FAULT_USB_TIMEOUT,
	FAULT_UNABLE_TO_CONNECT_TO_SERVER,
	FAULT_BAD_CREDENTIALS,
	FAULT_BATTERY_NOT_DETECTED,	
	FAULT_PSU_BATTERY_HEALTH,
	FAULT_ILLEGAL_SITE_FILE,		// 73
	FAULT_ZONE_LED_FAIL,				// 74
	FAULT_OPEN_CIRCUIT,
	FAULT_SHORT_CIRCUIT,
	FAULT_EVENT_TRIGGERED,
	FAULT_SOUNDER_OPEN_CIRCUIT,
	FAULT_SOUNDER_SHORT_CIRCUIT,
	FAULT_ROUTING_OPEN_CIRCUIT,
	FAULT_ROUTING_SHORT_CIRCUIT,

	FAULT_AT_APP_URL_SET,
	FAULT_AT_APP_APN_SET,
	FAULT_AT_APP_PASSWORD_SET,
	
	FAULT_BACKLIGHT_PARAMS_GET,

	FAULT_GSM_SMS_ENEBLED_GET,
	FAULT_GSM_APN_GET,
	FAULT_GSM_APN_SET,
	FAULT_GSM_PASSWORD_SET,
	FAULT_GSM_PASSWORD_GET,
	FAULT_GSM_SMS_ENEBLED_SET,

	FAULT_MEM_CHECH_SUM_GET,
	FAULT_MEM_CHECH_SUM_SET,

	FAULT_MQTT_APP_ENABLED_GET,
	FAULT_MQTT_APP_SITE_GET,
	FAULT_MQTT_APP_URL_GET,
	FAULT_MQTT_APP_APN_GET,
	FAULT_MQTT_APP_PASSWORD_GET,

	FAULT_MQTT_APP_ENABLED_SET,
	FAULT_MQTT_APP_SITE_SET,
	FAULT_MQTT_APP_URL_SET,
	FAULT_MQTT_APP_APN_SET,
	FAULT_MQTT_APP_PASSWORD_SET,

	FAULT_SETTINGS_PREVIOUS_ADDR_SET,
	FAULT_SETTINGS_CURRENT_ADDR_SET,

	FAULT_SETTINGS_PREVIOUS_ADDR_GET,
	FAULT_SETTINGS_CURRENT_ADDR_GET,

	FAULT_TOUCH_PARAMS_GET,


	FAULT_ANY	=	0xFF,
} FaultType;	


#define FAULTFLAG_IS_LATCHED		1
#define FAULTFLAG_SYSTEM 			2
#define FAULTFLAG_ALARM_DEVICE	4
#define FAULTFLAG_LATCH				8
#define FAULTFLAG_PSU				16
#define FAULTFLAG_SOUNDER			32
#define FAULTFLAG_OPEN_CIRCUIT	64
#define FAULTFLAG_CLOSED_CIRCUIT 128
#define FAULTFLAG_NETWORK 			256
#define FAULTFLAG_HIDDEN			512
#define FAULTFLAG_DONT_ALARM 		1024
#define FAULTFLAG_SILENCED			2048
#define FAULTFLAG_ROUTING_OUTPUT	4096
#define FAULTFLAG_SAFE_STATE		8192
#define FAULTFLAG_WARNING			16384
#define FAULTFLAG_MUTED				32768
#define FAULTFLAG_LEVEL_2			65536


#define FAULTGROUP_GENERAL			1
#define FAULTGROUP_DEVICE			2
#define FAULTGROUP_TAMPER			3
#define FAULTGROUP_INSTALLATION	4
#define FAULTGROUP_IOU				5
#define FAULTGROUP_MAINS			6
#define FAULTGROUP_BATTERY			7
#define FAULTGROUP_SYSTEM			8


 
char* GetTypeName( ChannelType type );

typedef struct
{
	FaultType 	type;
	const char*	msg;
	int 		flags;
	char			group;
} FaultDef;


typedef struct
{
	unsigned char		 	type;
	char 		 	chan;
	char 			zone;
	short       unit;
	int   flags;
	time_t	 	time;
	const char* msg;
} FaultItem;

class GSM;

class Fault : public Module
{
	public:
		
	Fault( );
 
	static void AddFault( FaultType, char zone = 255, short unit = -1, char chan = 0, bool latching = false );
	static void RemoveFault( FaultType, char zone = 255, short unit = -1, char chan = 0, int flags = 0, bool ignore = false, bool sendtoserver = true );

	static void AddFault( const char*, char zone = 255, short unit = -1, char chan = 0, int flags = 0, bool latching = false );
	static void RemoveFault( const char*, char zone = 255, short unit = -1, char chan = 0 );

	static void RemoveFault( FaultItem* );
	static void RemoveAllFaults( short unit );
	static void RemoveAllWarnings( short unit );
	static void RemoveAllFaultsAndWarnings( short unit );
	static void RemoveAllPanelFaultsAndWarnings(   );
	static void RemoveAllFaults( FaultType );
	
	static void CheckLEDs( );
	static int NumFaults( int& extra, int& reduced );
	
	static void CheckNewFaults( );
	
	static void GetFaultStatus( int& silenceable_status, int& unsilenceable_status );
	
	static void CheckNoFaults( );
	static void CheckFlags( FaultItem* );
	
	static void UpdateGUI( bool jumpToScreen );
	static FaultItem* fault_pos;
	
	virtual int Receive( Event );
	virtual int Receive( Message* msg );
	virtual int Receive( Command* cmd ); 
	
	static int IsValid( FaultItem* );

	static int fire_routing_faulty;
	static int sounder_faulty;
	static time_t last_fault_activity;
	
	private:
		
	int dogging;
	static GPIOManager* gpio;	
	static EmbeddedWizard* ew;
	static int faults_changed;
	static GSM* gsm;
};

 




#ifdef OLD_FAULTS

/* SYSTEM FAULTS */


#define  FAULT_DATA_INTEGRITY  					0x1001
#define  FAULT_BACKLIGHT_PWM_START				0x1002
#define  FAULT_CRYSTAL_FAILURE					0x1004
#define  FAULT_PSU_UART_FAILURE					0x1008
#define  FAULT_GSM_UART_FAILURE					0x1010


/* GENERAL FAULTS */

#define FAULT_I2C_FAIL								0x2000
#define FAULT_NCU_BUFFER_OVERFLOW				0x2001



/* POWER FAULTS */

#define  FAULT_POWER_SHORTAGE_RESET				0x3001
#define  FAULT_MAINS_FAILURE						0x3005
#define  FAULT_BATTERY_LOW							0x3006
#define  FAULT_BATTERY_CRITICAL					0x3007
#define  FAULT_CHARGER_FAILED						0x3008
#define  FAULT_PSU_CONFIG_ERROR					0x3009
#define  FAULT_BATTERY_RESISTANCE_TOO_HIGH	0x300A
#define  FAULT_PSU_OVER_VOLTAGE					0x300B

#endif

#endif
