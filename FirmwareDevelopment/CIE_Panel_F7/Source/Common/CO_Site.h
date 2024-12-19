/***************************************************************************
* File name: CO_Site.h
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
* Site definitions
*
**************************************************************************/


#ifndef _CO_SITE_H_
#define _CO_SITE_H_


/* System Include Files
**************************************************************************/
#include <stdint.h>


/* User Include Files
**************************************************************************/
#ifdef __cplusplus
#include "MM_List.h"
#endif

/* Defines
**************************************************************************/


#define SITE_MAX_CONTACTS 				30
#define SITE_MAX_STRING_LEN	  		40
#define SITE_MAX_CHANNEL_PARAMS		 4		
#define SITE_MAX_ANALOGUE_CHANNELS   4
#define SITE_MAX_INPUT_CHANNELS  	32
#define SITE_MAX_OUTPUT_CHANNELS  	32
#define SITE_MAX_RADIO_DEVICES	  511
#define SITE_MAX_USERS					20
#define SITE_MAX_PANELS					 5
#define SITE_MAX_PHONE_DIGITS			24
#define SITE_MAX_CAE_RULES			  300
#define SITE_MAX_ZONES					96
#define SITE_MAX_CAE_INPUTS 		  300
#define SITE_MAX_CAE_OUTPUTS		  300
#define SITE_MAX_CAE_LIST_LEN_IN	  512
#define SITE_MAX_CAE_LIST_LEN_OUT  128

#define SITE_MAX_PANEL_DEVICES		 3
 

#define SITE_MAX_DEVICES				( SITE_MAX_RADIO_DEVICES + SITE_MAX_PANEL_DEVICES )
 
#define SITE_NUM_PROFILES 					9
#define SITE_NUM_OUTPUT_ACTIONS			7
#define SITE_NUM_INPUT_ACTIONS			10
#define SITE_ALLOCATION_SIZE			0x600000

#define SITE_MAX_BATTERY_TYPES		4

#define CAE_OPTION_DAYTIME_ONLY					1
#define CAE_OPTION_NIGHTTIME_ONLY				2
#define CAE_OPTION_COINCIDENCE_SAME_ZONE		4
#define CAE_OPTION_COINCIDENCE_SAME_DEVICE	8
#define CAE_OPTION_COINCIDENCE_LIGHT_FIRST	16

#define EVAC_DEVICE_TYPE 255


#define ATCMD_MAX_ARGS 4
#define ATCMD_MAX_COMMANDS	36


struct FactorySettings
{
	char 		zones;
	char 		leds;
	short 	devices;
};
	

struct ATCmd
{
	const char* fmt;
	int param[ ATCMD_MAX_ARGS];
};


typedef enum
{
	ACTION_BIT_FIRE 			=	1,
	ACTION_BIT_FIRST_AID		=  2,
	
	ACTION_BIT_EVACUATION	=  4,
	ACTION_BIT_SECURITY		=  8,
	ACTION_BIT_GENERAL		=  16,	
	ACTION_BIT_FAULT			=  32,
	
 	ACTION_BIT_ROUTING_ACK	=  64,	
	
	ACTION_BIT_MESH_FOR_SMS = 128,
	
	ACTION_BIT_TEST_PROFILE =  128,
	
	ACTION_BIT_ACKNOWLEDGE_EVENT = 128,
	
	ACTION_BIT_RESOUND		 = 256,	
	
 	ACTION_BIT_RESET			=  512,
 	ACTION_BIT_SILENCE		=  1024,
	
 	ACTION_BIT_UNDEFINED    =  2048,
	ACTION_BIT_CONFIRM_EVENT = 2048,
	
	ACTION_BIT_CAE_OUTPUT	= 4096,
	
	ACTION_BIT_ACKNOWLEDGE_FIRE	= 1 << 13,
	ACTION_BIT_CONFIRM_FIRE			= 1 << 14,

	ACTION_BIT_DEVICE_CONNECT		= 1 << 15,	
	ACTION_BIT_DEVICE_DISCONNECT	= 1 << 16,
	
	ACTION_BIT_DEVICE_CLEAR			= 1 << 17,
	
	ACTION_BITS_OTHER_EVENTS = ( ACTION_BIT_FIRST_AID | ACTION_BIT_EVACUATION | ACTION_BIT_SECURITY | ACTION_BIT_GENERAL | ACTION_BIT_ROUTING_ACK ),
	
	ACTION_BIT_DEVICE_ACTIVITY = ( ACTION_BIT_DEVICE_CONNECT | ACTION_BIT_DEVICE_DISCONNECT ),

} ActionBits;

// silent profile bit is recycled as is not an actual event.



typedef enum 
{
	ACTION_SHIFT_FIRE 					=	0,
	ACTION_SHIFT_FIRST_AID				=  1,
	ACTION_SHIFT_EVACUATION				=  2,	
	ACTION_SHIFT_SECURITY				=  3,
	ACTION_SHIFT_GENERAL					=  4,
	ACTION_SHIFT_FAULT					=  5, 
			
	ACTION_SHIFT_ROUTING_ACK			=  6,
			
	ACTION_SHIFT_TEST_PROF 				=  7,
	ACTION_SHIFT_RESOUND					=  8,
			
			
	ACTION_SHIFT_RESET					=  9,
	ACTION_SHIFT_SILENCE					= 10,
	ACTION_SHIFT_UNDEFINED				= 11,	
	ACTION_SHIFT_CAE_OUTPUT				= 12,	
	
	ACTION_SHIFT_ACKNOWLEDGE_FIRE			= 13,
	ACTION_SHIFT_CONFIRM_FIRE					= 14,
	
	ACTION_SHIFT_ACKNOWLEDGE_EVENT			= 7,
	ACTION_SHIFT_CONFIRM_EVENT					= 11,
} ActionEnum;	


typedef enum 
{
	PROFILE_FIRE 			=	0,
	PROFILE_FIRST_AID		,
	PROFILE_EVACUATION	,	
	PROFILE_SECURITY		,
	PROFILE_GENERAL		,
	PROFILE_FAULT		, 
	PROFILE_ROUTING_ACK	,
	PROFILE_TEST			,	
	PROFILE_SILENT		   ,  
	PROFILE_NONE = 255,
} ProfileEnum;	


#define CHANNEL_OPTION_LATCHING 				1
#define CHANNEL_OPTION_DISABLED_ANYTIME	2
#define CHANNEL_OPTION_INVERTED				4
#define CHANNEL_OPTION_DISABLED_DAYTIME	8
#define CHANNEL_OPTION_DISABLED_NIGHTTIME	16
#define CHANNEL_OPTION_SKIP_OUTPUT_DELAYS	32
#define CHANNEL_OPTION_SILENCEABLE	   	64
#define CHANNEL_OPTION_COINCIDENCE	   	128
#define CHANNEL_OPTION_USE_GLOBAL_DELAY	256
#define CHANNEL_OPTION_NO_NIGHT_DELAY		512



#define SITE_RBU_START 						1
#define SITE_PANEL_INPUT_UNIT_START		2048
#define SITE_PANEL_OUTPUT_UNIT_START 	(2048+256)
#define SITE_EXPANSION_UNIT_START		4096
#define SITE_VIRTUAL_UNIT_START			8192


#define SITE_SETTING_RESOUND_NEW_ZONE				1
#define SITE_SETTING_DISABLE_OUTPUT_DELAYS		2
#define SITE_SETTING_FIRE_ROUTING_DISABLED		4
#define SITE_SETTING_FIRE_ALARMS_DISABLED			8
#define SITE_SETTING_IGNORE_SECURITY_AT_NIGHT	16
#define SITE_SETTING_IGNORE_SECURITY_IN_DAY		32
#define SITE_SETTING_GLOBAL_DELAY_OVERRIDE		64
#define SITE_SETTING_GSM_AVAILABLE					128
 


#define DEVCONFIG_DISABLED		1

#define ZONEFLAG_DISABLED		1

#define GROUP_FIRE_ALARMS 		-1
#define GROUP_FIRE_ROUTING		0


#define PREVENT_BLANK ( UINT_MAX )

typedef struct 
{	
	int   	actions;
   char 		name[ SITE_MAX_STRING_LEN ];
   char		number[ SITE_MAX_PHONE_DIGITS ];
} SMSContact;


typedef struct
{
   uint8_t 	hour;
   uint8_t 	minute;
} Time;


typedef struct
{
   Time		start;
   Time		end;
} Daytime;


/* RU Channel mapping */

typedef enum
{
   CO_CHANNEL_NONE_E = 0,										// 	0
   CO_CHANNEL_SMOKE_E,											// 	1
   CO_CHANNEL_HEAT_B_E,											// 	2
   CO_CHANNEL_CO_E,												// 	3
   CO_CHANNEL_PIR_E,												// 	4
   CO_CHANNEL_SOUNDER_E,										// 	5
   CO_CHANNEL_BEACON_E,											// 	6
   CO_CHANNEL_FIRE_CALLPOINT_E,								// 	7
   CO_CHANNEL_STATUS_INDICATOR_LED_E,  					// 	8
   CO_CHANNEL_VISUAL_INDICATOR_E,							// 	9
   CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E,		//	  10
   CO_CHANNEL_MEDICAL_CALLPOINT_E,
   CO_CHANNEL_EVAC_CALLPOINT_E,
   CO_CHANNEL_OUTPUT_ROUTING_E,
   CO_CHANNEL_INPUT_1_E,
   CO_CHANNEL_INPUT_2_E,
   CO_CHANNEL_INPUT_3_E,										// 16
   CO_CHANNEL_INPUT_4_E,
   CO_CHANNEL_INPUT_5_E,
   CO_CHANNEL_INPUT_6_E,
   CO_CHANNEL_INPUT_7_E,
   CO_CHANNEL_INPUT_8_E,
   CO_CHANNEL_INPUT_9_E,
   CO_CHANNEL_INPUT_10_E,
   CO_CHANNEL_INPUT_11_E,
   CO_CHANNEL_INPUT_12_E,
   CO_CHANNEL_INPUT_13_E,										// 26
   CO_CHANNEL_INPUT_14_E,
   CO_CHANNEL_INPUT_15_E,
   CO_CHANNEL_INPUT_16_E,
   CO_CHANNEL_INPUT_17_E,
   CO_CHANNEL_INPUT_18_E,
   CO_CHANNEL_INPUT_19_E,
   CO_CHANNEL_INPUT_20_E,
   CO_CHANNEL_INPUT_21_E,
   CO_CHANNEL_INPUT_22_E,
   CO_CHANNEL_INPUT_23_E,										// 36
   CO_CHANNEL_INPUT_24_E,
   CO_CHANNEL_INPUT_25_E,
   CO_CHANNEL_INPUT_26_E,
   CO_CHANNEL_INPUT_27_E,
   CO_CHANNEL_INPUT_28_E,
   CO_CHANNEL_INPUT_29_E,
   CO_CHANNEL_INPUT_30_E,
   CO_CHANNEL_INPUT_31_E,
   CO_CHANNEL_INPUT_32_E,
   CO_CHANNEL_OUTPUT_1_E,		// 46
   CO_CHANNEL_OUTPUT_2_E,
   CO_CHANNEL_OUTPUT_3_E,
   CO_CHANNEL_OUTPUT_4_E,
   CO_CHANNEL_OUTPUT_5_E,
   CO_CHANNEL_OUTPUT_6_E,
   CO_CHANNEL_OUTPUT_7_E,
   CO_CHANNEL_OUTPUT_8_E,
   CO_CHANNEL_OUTPUT_9_E,
   CO_CHANNEL_OUTPUT_10_E,
   CO_CHANNEL_OUTPUT_11_E,   // 56
   CO_CHANNEL_OUTPUT_12_E,
   CO_CHANNEL_OUTPUT_13_E,
   CO_CHANNEL_OUTPUT_14_E,
   CO_CHANNEL_OUTPUT_15_E,
   CO_CHANNEL_OUTPUT_16_E,
//   CO_CHANNEL_OUTPUT_17_E,
//   CO_CHANNEL_OUTPUT_18_E,
//   CO_CHANNEL_OUTPUT_19_E,
//   CO_CHANNEL_OUTPUT_20_E,
//   CO_CHANNEL_OUTPUT_21_E,     // 66    
//   CO_CHANNEL_OUTPUT_22_E,
//   CO_CHANNEL_OUTPUT_23_E,
//   CO_CHANNEL_OUTPUT_24_E,
//   CO_CHANNEL_OUTPUT_25_E,
//   CO_CHANNEL_OUTPUT_26_E,
//   CO_CHANNEL_OUTPUT_27_E,
//   CO_CHANNEL_OUTPUT_28_E,
//   CO_CHANNEL_OUTPUT_29_E,
//   CO_CHANNEL_OUTPUT_30_E,
//   CO_CHANNEL_OUTPUT_31_E,       // 76
//   CO_CHANNEL_OUTPUT_32_E,
	CO_CHANNEL_HEAT_A1R_E = 62,
	
	CO_RESERVED_FOR_MESH_E = 63,
	
	CO_CHANNEL_PANEL_ROUTING_ACK  ,
	CO_CHANNEL_PANEL_CALLPOINT  ,
	CO_CHANNEL_PANEL_ALARM_INPUT  ,
	CO_CHANNEL_PANEL_SILENCE_INPUT  ,
	CO_CHANNEL_PANEL_FAULT_INPUT ,
	
	
	CO_CHANNEL_PANEL_ALARM_RELAY  ,
	CO_CHANNEL_PANEL_FAULT_RELAY  ,
	CO_CHANNEL_PANEL_FIRE_RELAY  ,
	CO_CHANNEL_PANEL_ROUTING_OUTPUT  ,
	CO_CHANNEL_PANEL_SOUNDERS_RELAY ,
	
	CO_CHANNEL_PANEL_FUNCTION_BUTTON_1,
	CO_CHANNEL_PANEL_FUNCTION_BUTTON_2,
	CO_CHANNEL_PANEL_EVACUATE_BUTTON,
	
	// MIRROR CHANNELS
	
	CO_CHANNEL_SITENET_SOUNDER_E = 128 + CO_CHANNEL_SOUNDER_E,
	
   CO_CHANNEL_MAX_E

} ChannelType;




typedef enum
{
	Param_None = 0,
	Param_Volume,
	Param_FlashRate,
} ParamType;	


typedef struct
{
	ParamType 		code;
	uint8_t			channel;
	short 			value;
	short 			min;
	short				max;
	char				name[ SITE_MAX_STRING_LEN ];
} Parameter;


typedef struct
{
   uint16_t  		flags;
   uint8_t   		action;   
   uint16_t   		delay;    
	ChannelType   	type;		
} InputChannel;


typedef struct
{
   uint16_t   		flags;    
   uint16_t   		actions;  
   uint16_t    	delay1;   
   uint16_t    	delay2;   
	ChannelType 	type;
} OutputChannel;


typedef struct
{
   char      		location[ SITE_MAX_STRING_LEN ];
	char				typestring[ SITE_MAX_STRING_LEN ];
   uint16_t  		unit;          
   uint8_t   		type;            
   uint8_t   		zone;            
 
	uint8_t   		branding;
  uint8_t   		flags;         
	uint8_t			numParams;
	uint8_t			numInputs;
	uint8_t        numOutputs;
	  
   InputChannel  	input[ SITE_MAX_INPUT_CHANNELS ];
   OutputChannel 	output[ SITE_MAX_OUTPUT_CHANNELS ];
	Parameter		param[ SITE_MAX_CHANNEL_PARAMS ]; 
	
#ifdef __cplusplus	
	int				IsInputDisabled( int c );
	int				IsOutputDisabled( int c );
	
	int InitialDelay( int channel );
	int InvestigativeDelay( int channel );
	
	int IsIOUnit( );
	int IsInputChannel( int ch, int& index );
	int IsOutputChannel( int ch, int& index );
	
	int IsDisabled( );
	int IsAlarmOutputDevice( );
	int IsAlarmOutputChannel( int ch );
	int IsRoutingOutputChannel( int ch );
	void SetupStart( Circular<ATCmd>* buff );
	void SetupProfiles( Circular<ATCmd>* buff );
	void SetupInputs( Circular<ATCmd>* buff );
	void SetupOutputs( Circular<ATCmd>* buff );
	
	int Push( Circular<ATCmd>* buff, const char* fmt, ... );
		
#endif	
	
} DeviceConfig;


typedef struct
{
	char 				name[ SITE_MAX_STRING_LEN ];
	int				level;
	char				pin[ 5];
} LogonUser;


typedef enum
{
	CAE_LOGIC_AND,
	CAE_LOGIC_OR,
	CAE_LOGIC_ANY2,
	CAE_LOGIC_COINCIDENT,
} Logic;


typedef enum
{
	CAE_NONE,
	CAE_ACTIVATE,
	CAE_ENABLE,
	CAE_DISABLE,	
	CAE_TEST,	
} CAEAction;


typedef struct
{
	char 			name[ SITE_MAX_STRING_LEN ];
	char		 	list[ SITE_MAX_CAE_LIST_LEN_OUT ];
	CAEAction 	effect;
} CAEOutput;


typedef struct
{
	char 				name[ SITE_MAX_STRING_LEN ];
	char				list[ SITE_MAX_CAE_LIST_LEN_IN ];
	int				flags;
	Logic				logic;	
	unsigned short within;
} CAEInput;
 


#ifdef __cplusplus
struct Device;
#endif


typedef struct
{
	char 				input;
	char 				output;
 	char 				displayzone;	
	int				inverted : 1;
	int				latching : 1;

	ProfileEnum 	profile;
	unsigned short	delay;
	char  			name[ SITE_MAX_STRING_LEN ];
	ActionEnum		action;
	
} CAERule;


typedef struct
{
	uint32_t			process[ SITE_MAX_ZONES / 32 ];
	uint32_t			display[ SITE_MAX_ZONES / 32 ];
} NetFilter;


typedef struct
{
	char 				name[ SITE_MAX_STRING_LEN ];
	uint8_t 			flags;
	uint8_t			num;
} Zone;


typedef struct
{
	uint8_t 		tone;
	uint8_t 		flash;
	uint8_t		ext_sounder;
} Profile;


typedef struct 
{
   char      		name[ SITE_MAX_STRING_LEN ];

	Daytime			day[ 7];		 // Day 0 = Monday

   uint32_t			autoSilence;	
	uint32_t			settings;
	uint16_t			delay1;
	uint16_t       delay2;
	
   uint32_t  		systemId;
   uint8_t   		freq; 	
	uint8_t			address;
	
	uint16_t			framelength;
	uint16_t			maxDevices;
   uint16_t  		numRadioDevs;       
	uint16_t  		numPanelDevs;       
	uint16_t			numDefinedZones;
	uint16_t   		numZoneLeds;
   uint16_t  		numCaeRules;
	uint16_t			numCaeInputs;
	uint16_t			numCaeOutputs;
	uint16_t   		productrange;
	
   DeviceConfig 	devs[ SITE_MAX_DEVICES ];
	DeviceConfig 	units[ SITE_MAX_DEVICES ];
	CAEInput			caeInputs[ SITE_MAX_CAE_INPUTS ];		
	CAEOutput		caeOutputs[ SITE_MAX_CAE_OUTPUTS ];	
 	CAERule 			caeRules[ SITE_MAX_CAE_RULES ];   
	
	NetFilter    	net[ SITE_MAX_PANELS][ SITE_NUM_OUTPUT_ACTIONS ] ;

	Zone				zone[ SITE_MAX_ZONES ];
	
	
#ifdef __cplusplus		
	uint16_t 		NumDevs( );

#endif

} Panel;


typedef struct
{
   char      		name[ SITE_MAX_STRING_LEN ];
	char					uid[ SITE_MAX_STRING_LEN ];
	unsigned int	version;	

	unsigned int	productrange;

   uint8_t   		flags;
	uint8_t			numContacts;
   uint8_t   		numPanels;      // Max 5	
	uint8_t			numUsers;
	uint8_t			currentIndex;
	uint16_t			batt_low[ SITE_MAX_BATTERY_TYPES ];
	uint16_t			batt_med[ SITE_MAX_BATTERY_TYPES ];
	uint16_t			batt_high[ SITE_MAX_BATTERY_TYPES ];
	uint16_t			batt_panel;
	
   Profile   		profiles[ SITE_NUM_PROFILES ];		// action = index
   SMSContact   	contacts[ SITE_MAX_CONTACTS ];
	LogonUser 		users[ SITE_MAX_USERS ];

   Panel     		panels[ SITE_MAX_PANELS ];
	
	
} Site;

const char* GetActionString( int action );

 
#endif

