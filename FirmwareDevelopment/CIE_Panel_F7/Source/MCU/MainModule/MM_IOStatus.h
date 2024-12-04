/***************************************************************************
* File name: MM_IOStatus.h
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
* Main IO management
*
**************************************************************************/

#ifndef _MM_INPUT_STATUS_H_
#define _MM_INPUT_STATUS_H_


/* System Include Files
**************************************************************************/



/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Module.h"
#include "MM_device.h"
#include "MM_GPIO.h"
#include "MM_Settings.h"
#include "MM_RealTimeClock.h"
#include "MM_GSM.h"
#include "CO_Site.h"
#include "MM_PanelNetwork.h"
#include "MM_NCU.h"


/* Defines
**************************************************************************/
#define ZONE_LED_IN_FIRE				1
#define ZONE_IS_SILENCED		2		
#define ZONE_MODIFY_EXPANDED	4
#define ZONE_ON_TEST				8


#define ZONE_TEST_EXPANDED 	32
#define ZONE_DISABLE_EXPANDED	64
#define ZONE_FIRE_EXPANDED		128

#define ZONE_RULED_DISABLED	256
 


#define MSG_REFRESH_OUTPUTS 	1
#define MSG_RESET_BUTTON		2
#define MSG_SILENCE_BUTTON		3
#define MSG_RESET_FIRE			4


#define VIRTUAL_ALARM_ZONE 	-1
#define VIRTUAL_ROUTING_ZONE	0


struct Rule
{
	time_t 	time;
	int		flags;
	int 		zone[ 3];
};

#define RULE_SILENCED	1
#define RULE_TRUE			2
#define RULE_ACTIVE 		4
#define RULE_NOT_FOUND 	8
#define RULE_LATCHED		16


class CauseAndEffect;		


struct Alert
{
	char* 	description;
	short		zone;
	short		unit;
	char     channel_img;
	char		flags;
	char		action;
	signed char panel_index;
	time_t	timestamp;
};
/*
struct Alert
{
	char* 	description;
	int		zone : 7;
	int		unit : 9
		int      channel : 5;
	char		flags : 3;
	int   panel_index : 3;
	time_t	timestamp;
};*/

	
struct ZoneAlert
{
	short		zone;
	short		panel;
};


#define MASK_NET 4
#define MASK_CAE 2
#define MASK_DEV 1


class IOStatus : public Module 
{
	public:
	
	IOStatus( );
	
	virtual int Init( );

	virtual int Receive( Message* );
	virtual int Receive( Event );
	virtual int Receive( Command* cmd );
	virtual void Poll( );
	
	int GetNumZonesInFire( );
	ZoneAlert* GetNthZone( int n );
	
	bool DevicesOnTestInAlarm( );
	bool DevicesOnTest( );
	
	Alert* GetNthAlert( int n, int actions, int zone = -1, int panel = -1, int mask = 0 );
	int    GetNumAlerts( int actions, int inzone = -1, int panel = -1, int mask = 0 );

	time_t GetNextDeviceInAlarm( Device* &dev, int& chan_num, int actions, int& action, int inzone = -1 );
 
	time_t GetNextCAEInAlarm( int& rule_num, int actions, int& action, int inzone = -1 );
 
	time_t GetNextNetworkAlertInAlarm( TimedDeviceRef* &alertref, int actions, int inzone, int inpanel, int& panel );			
		
	time_t GetTimestamp( DeviceRef &ref, int action, int panel, CAERule* &rule, DeviceConfig* &dev );	
		
	void ActivateIndividualOutputs( );
	
	void GetAlertList( DeviceRef* dest, int max );
	 
	
	void ClearDevices( );

	void Change( Device* id, int value );
	//void InputAssert( Device* );
	//void InputDeAssert( Device* );
	//void InputRemove( Device* );	
	
   static Device* FindDevice( void* ptr );
	static Device* FindDevice( int zone, int unit );
	static Device* FindDevice( int unit );
	static Device* FindDevice( DeviceConfig* );
	
	
	static Device* device_list;
	static Device* device_list_pos;
		
	Device* RegisterDevice( Device* );
		
	void RemoveDevice( Device* );

	int GetEventStatus( );
	
	int NumNetworkAlarms( int actions, int inzone = -1, int inpanel = -1 );
	int NumCAEAlarms( int actions, int inzone = -1 );
	
 
	int test_profile_bit;
			 
	time_t 			unsilenceable_assert_time[ SITE_NUM_OUTPUT_ACTIONS ];
	
	static int   test_everything;
	unsigned int   zonebits[ SITE_NUM_OUTPUT_ACTIONS][ SITE_MAX_ZONES / 32 ];
	
	
	
	int prev_silenceable_status;
	static int prev_unsilenceable_status;
	
	static time_t last_ontest_activity;
	static time_t last_event_activity;
	static time_t last_disablement_activity;
	static time_t last_fire_activity;
	
	private:
		
	int prev_broadcast_silenceable_status   = 0;
	int prev_broadcast_unsilenceable_status = 0;
	int prev_broadcast_skipdelays_status    = 0;
	 
			
	int prev_skipdelays_status;
	 
	time_t silence_timer;
	bool refresh_silence;
		
	void HandleDelays( int delays );
	
	int RefreshInputs( int& unsilenceable, int& silencable, int& skipdelays );//, char (&netoutput)[ SITE_MAX_RADIO_DEVICES + 1 ] );
	int RefreshOutputStatus( int unsilenceable, int silencable, int skipdelays, int& output_changes, bool broadcasted );
	
	int ProcessCauseAndEffect( int& unsilenceable, int& silencable, int& output_changes );	
	
	void ResetInputs( );
	void ResetOutputs( );
		
	void Assert( Device* );
	void DeAssert( Device* );
		 
	
	time_t 			ack_fire_time;
	time_t 			ack_event_time;
	time_t 			confirm_fire_time;
	time_t 			confirm_event_time;	
	
	int 				nonbroadcastmask;
	int 				refresh_outputs_needed;
	
	bool 				new_zone_in_fire;
	
 
	GSM*					gsm;
	GPIOManager* 		gpios;
	Settings*			settings;
	RealTimeClock* 	rtc;
	CauseAndEffect* 	cae;
	PanelNetwork*  	network;
	NCU*					ncu;
	EmbeddedWizard* 	ew;
 
	
   int ProcessNetworkStatus( int& silenceable_status, int& unsilenceable_status, int& skip_status );
	
	unsigned int   silencebits[ SITE_MAX_PANELS ][ SITE_NUM_OUTPUT_ACTIONS][ SITE_MAX_ZONES / 32 ];
		
	int resound_enabled;
	
	public:
		
	static Rule				rule[ SITE_MAX_CAE_RULES ];

	TimedDeviceRef* FindRef( int panel, DeviceRef* ref );
		
	TimedDeviceRef		netAlertList[ SITE_MAX_PANELS ][ MAX_NET_REFS ];
			 
	static short _zoneflags[ SITE_MAX_PANELS ][ SITE_MAX_ZONES + 2 ];
	static short (*zoneflags)[ SITE_MAX_ZONES + 2 ]; 
};




#endif

