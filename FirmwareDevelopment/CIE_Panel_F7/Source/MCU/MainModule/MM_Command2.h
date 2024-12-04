/***************************************************************************
* File name: MM_Command2.h
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
* Command helper functions
*
**************************************************************************/

#ifndef _COMMAND_2_H_
#define _COMMAND_2_H_


/* System Include Files
**************************************************************************/
#include <stdint.h>


/* User Include Files
**************************************************************************/


/* Defines
**************************************************************************/

#define CMD_OK								0
#define CMD_ERR_ARG_TYPE				1
#define CMD_ERR_NUM_ARGS				2
#define CMD_ERR_UNKNOWN_ARG_TYPE 	3
#define CMD_ERR_OUT_OF_RANGE			4
#define CMD_ERR_DEVICE_BUSY			5
#define CMD_ERR_UNKNOWN_CMD			6
#define CMD_ERR_MODULE_NOT_FOUND		7
#define CMD_ERR_PIPE_FAIL				8
#define CMD_ERR_NO_SITE					9
#define CMD_ERR_WRITE_PROTECTED		10
#define CMD_ERR_ALREADY_EXISTS		11
#define CMD_ERR_WRITE_FAIL				12
#define CMD_ERR_DOESNT_EXIST			13
#define CMD_ERR_NULL						14



typedef enum
{
	CMD_SET_BACKLIGHT_LEVEL,
	CMD_GET_BACKLIGHT_LEVEL,
	CMD_SET_BACKLIGHT_ON,
	CMD_SET_BACKLIGHT_OFF,
	CMD_SET_TIME,				
	CMD_SET_DATE,				
	CMD_SET_DST_START_DATE,	
	CMD_SET_DST_END_DATE,		
	CMD_SET_DST_START_HOUR,	
	CMD_SET_DST_END_HOUR,		
	CMD_SET_CALIB_VALUES,		
	CMD_GET_PSU_STATUS,		
	CMD_GSM_SIGNAL_QUALITY,	
	CMD_GSM_SIGNAL_PERIOD,
	CMD_SET_GSM_SETTINGS,
	CMD_GET_GSM_SETTINGS,
	CMD_LOAD_SETTINGS_USB_SERIAL,
	CMD_LOAD_SETTINGS_USB_SERIAL_MSG,
	CMD_SAVE_SETTINGS_USB_SERIAL,
	CMD_GET_PROGRESS,
	CMD_CANCEL_PROGRESS,
	CMD_SAVE_SETTINGS_USB_HOST,
	CMD_LOAD_SETTINGS_USB_HOST,
	CMD_CANCEL_LOAD_SETTINGS,
	CMD_GET_DEVICE_LISTS,
	CMD_GET_MESH_STAGE,
	CMD_CHECK_ON_TREE,
	CMD_CHECK_TREE,
	CMD_RESTART_MESH,
	CMD_SYNCHRONISE_MESH,
	CMD_GET_BACKLIGHT_VOLTAGE,
	CMD_GET_NUM_FIRES,
	CMD_GET_VERSION_NUMBERS,
	CMD_LOAD_FILE_AT_INDEX,
	CMD_EXIT_BOOT_MENU,
	CMD_FACTORY_RESET,
	CMD_RESET_EEPROM,
	CMD_GET_FILE_AT_INDEX,
	CMD_GET_ITEMS_IN_FOLDER,
	CMD_TOUCH_EMULATION,
	CMD_COPY_BITMAPS,
	CMD_GET_DEVICE_CONFIG,
	CMD_GET_NUM_ZONES,
	CMD_GET_NUM_DEVS_IN_ZONE,
	CMD_GET_DEV_IN_ZONE,
	CMD_GET_DEVICE,
	CMD_SET_OUTPUT_CHANNEL,
	CMD_GET_NUM_MAX_ZONES,
	CMD_SET_DEVICE,
	CMD_SET_DEVICE_ALL,
	CMD_GET_LAST_RAW_TOUCH,
	CMD_SET_NUM_FIRES,
	CMD_SET_INPUT_IN_TEST,
	CMD_GET_DEVICE_FROM_CONFIG,
	CMD_FIND_DEVICE_FROM_UNIT,
	CMD_KILL_DOG,
	CMD_GET_FAULT_ITEM,
	CMD_GET_NUM_FAULTS,
	CMD_GET_NAME,
   CMD_SET_NAME,	
	CMD_GET_LOG_LENGTH,
	CMD_GET_LOG_LINE,
	CMD_GET_MESH_LOG_LENGTH,
	CMD_GET_MESH_LOG_LINE,
	CMD_GET_NUM_OTHER_EVENTS, 
	CMD_LEDS_TEST,
	CMD_LOGON_ACCESS,
	CMD_GET_ON_TEST_DETAILS,
	CMD_GET_ON_TEST_ZONE_DETAILS,
	CMD_GET_NUM_ON_TEST,
	CMD_SET_INPUT_DISABLED,
	CMD_SET_OUTPUT_DISABLED,
	CMD_GET_DISABLED_DETAILS,
	CMD_DISABLED_ZONE_DETAILS,
	CMD_GET_NUM_DISABLED,	
	CMD_GET_RADIO_UNIT_CHILD,
	CMD_GET_NUM_MESH_ITEMS,
	CMD_GET_RADIO_UNIT_NOT_ON_MESH,
	CMD_GET_NUM_EXTRA_RADIO_UNITS,
	CMD_SET_CURRENT_DEVICE,
	CMD_DFU_MODE,
	CMD_SET_FW_LENGTH,
	CMD_GET_ZONE_FLAGS,
	CMD_SET_ZONE_FLAGS,
	CMD_GET_RADIO_UNIT,
	CMD_GET_SITE,
	CMD_GET_PANEL,
	CMD_GET_ALERT_DETAILS,
	CMD_GET_NTH_ZONE,
	CMD_GET_NUM_ZONES_IN_FIRE,
	CMD_GET_NUM_ALERTS_IN_ZONE,
	CMD_GET_PANEL_NAME,
	CMD_GET_PANEL_ZONE_FLAGS,
	CMD_SET_PANEL_ZONE_FLAGS,
	CMD_PPU_MODE,
	CMD_RESET_FAULT,
	CMD_GET_DAY_TIME,
	CMD_SET_DAY_TIME,
	CMD_GET_SMS_NUMBER,
	CMD_SET_SMS_NUMBER,
	CMD_NEW_SMS_NUMBER,
	CMD_REMOVE_SMS_NUMBER,
	CMD_GET_NUM_SMS_CONTACTS,
	CMD_GET_SETTINGS,
	CMD_SET_SETTINGS,
	CMD_CHECK_PASSKEY,
	CMD_SET_PASSKEY,
	CMD_SEACH_DEV_TYPE_STRING,
	CMD_GET_ZONE_DISABLED,
	CMD_SET_ZONE_DISABLED,
	CMD_SET_TEST_PROFILE,
	CMD_CATCH_ERROR,
	CMD_BUZZER,
	CMD_SET_ACCESS_LEVEL,
	CMD_SETUP_RBU,
	CMD_DETECT_RBU,
	CMD_PROGRAM_RBU,
	CMD_SET_NCU_ADVANCE,
	CMD_CHECK_FW,
	CMD_GET_NCU_FW_VERSION,
	CMD_GET_ANALOGUE_VALUE,
	CMD_RAISE_ALARM,
	CMD_FUNCTION_BUTTON,
	CMD_TEST_EVERYTHING,
	CMD_SET_BUZZER_STATE,
	CMD_GET_USER_BUTTON_ACTION,
	CMD_GET_NET_STATS,
	CMD_GET_RADIO_SETTINGS,
	CMD_SET_RADIO_SETTINGS,
	CMD_EXIT_RADIO_SETTINGS,
	CMD_CHANGE_RADIO_SETTINGS,
	CMD_GET_NUM_ENABLE_DISABLE,
	CMD_GET_NUM_TEST,
	CMD_REMOVE_DEVICE,
	CMD_OPTIMIZE_MESH,
	CMD_SET_SMS,
	CMD_SET_APP_SUPP,
	CMD_GET_SMS,
	CMD_GET_APP_SUPP,
	CMD_SETUP_APP,
	CMD_GET_APP_SETUP,
	CMD_GET_NUM_MODIFY,
	CMD_GET_TEST_EVERYTHING,
	CMD_ADD_ADDITIONAL,
	CMD_FLASH_EXTRA_RADIO_NUM,
	CMD_GET_EXTRA_RADIO_NUM,
	CMD_RESET_FIRE_STATE,
	CMD_CHECK_ACTIVE_ON_TEST,
	CMD_GET_NUM_ROUTING,
	CMD_NETWORK,
	CMD_GET_CLOUD_DETAILS,
	/* Nilesh Addded for CYG2-1707 bug fixing */
	CMD_EXIT_TEST,
} CommandType;


#pragma anon_unions

#ifndef __cplusplus
typedef
#endif
struct
#ifdef __cplusplus
Command
#endif	
{
#ifdef __cplusplus
	Command( CommandType );
	Command( );
#endif	
	CommandType  type;
	union
	{
		struct
		{
			int	int0;
			int	int1;
			int	int2;	
			int   int3;
		};
		struct
		{
			unsigned int	uint0;
			unsigned int	uint1;
			unsigned int	uint2;	
			unsigned int   uint3;
		};
		struct
		{
			char	char0;
			char	char1;
			char	char2;	
			char	char3;	
			char	char4;
			char	char5;
			char	char6;	
			char	char7;
			char	char8;
			char	char9;
			char	charA;	
			char	charB;	
			char	charC;
			char	charD;
			char	charE;	
			char	charF;
		};
		struct
		{
			short	short0;
			short	short1;
			short	short2;	
			short	short3;	
			short	short4;
			short	short5;
			short	short6;	
			short	short7;
		};
		struct
		{
			unsigned short	ushort0;
			unsigned short	ushort1;
			unsigned short	ushort2;	
			unsigned short	ushort3;	
			unsigned short	ushort4;
			unsigned short	ushort5;
			unsigned short	ushort6;	
			unsigned short	ushort7;
		};
	};
}
#ifndef __cplusplus
Command
#endif
;

#ifdef __cplusplus
 extern "C" 
 {
#endif
 
/*************************************************************************/
/**  \fn      void Command1( CommandType type, int p0 )
***  \brief   Global helper function
**************************************************************************/

int Command1( CommandType type, int p0 );
	 
	 
/*************************************************************************/
/**  \fn      int Command2( CommandType type, int p0, int p1 )
***  \brief   Global helper function
**************************************************************************/

int Command2( CommandType type, int p0, int p1 );
	 
	 
/*************************************************************************/
/**  \fn      int Command3( CommandType type, int p0, int p1, int p2 )
***  \brief   Global helper function
**************************************************************************/

int Command3( CommandType type, int p0, int p1, int p2 );
	 
/*************************************************************************/
/**  \fn      int Command4( CommandType type, int p0, int p1, int p2 )
***  \brief   Global helper function
**************************************************************************/

int Command4( CommandType type, int p0, int p1, int p2, int p3 );	 
	 
/*************************************************************************/
/**  \fn      int Command8S( CommandType type, short, short, short, short, short, short, short, short )
***  \brief   Global helper function
**************************************************************************/

int Command8S( CommandType type, short, short, short, short, short, short, short, short );


/*************************************************************************/
/**  \fn      int Command8U( CommandType type, ushort, ushort, ushort, ushort, ushort, ushort, ushort, ushort )
***  \brief   Global helper function
**************************************************************************/

int Command8U( CommandType type, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t );


/*************************************************************************/
/**  \fn      int Command0( CommandType type, Command* cmd )
***  \brief   Global helper function
**************************************************************************/

int Command0( CommandType type, Command* cmd );
	 
	 

#ifdef __cplusplus
 } 
#endif

#endif

