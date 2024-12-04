/***************************************************************************
* File name: MM_Settings.h
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
* Panel settings class. Site specific data (Stores network, mesh, etc)
*
**************************************************************************/

#ifndef _SETTINGS_H_
#define _SETTINGS_H_


/* System Include Files
**************************************************************************/
#include <assert.h>
#ifdef STM32F767xx
#include "stm32f7xx_hal_flash.h"
#include "stm32f7xx_hal_rtc.h"
#else
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_rtc.h"
#endif



/* User Include Files
**************************************************************************/
#include "MM_EEPROM.h"
#include "CO_Site.h"
#include "CO_RBU.h"
#include "MM_List.h"
#include "MM_RadioDevice.h"
#include "CO_Device.h"
#include "MM_QuadSPI.h"


/* Defines
**************************************************************************/
#define MEM(x) (int)&((UserSettings*)0)->x, sizeof(((UserSettings*)0)->x)
#define SETTINGS_ROM_ADDRESS 					( 0x90000000 ) 
	
#define FACTORY_SETTINGS ((FactorySettings*)(SETTINGS_ROM_ADDRESS + QSPI_SETTINGS_ADDRESS))


enum
{
	MSG_SET_PANEL_NAME,
	MSG_SITE_TRANSFER_SUCCESS,
	MSG_SITE_TRANSFER_FAIL,
	MSG_INIT_SITE,
	MSG_ACUMESH_CONNECT,
	MSG_UPDATE,
};
 

enum AcuCmd
{
	ACU_INIT = 2,
	ACU_START,				 
	ACU_ID,
	ACU_KEY,	
	ACU_ENCRYPT,
	ACU_BAUD,
	ACU_DONE,
};

enum ATJob
{
	JOB_NONE,
	JOB_ACU_CMD, JOB_ACU_WR, JOB_ACU_AC, JOB_ACU_CN, JOB_ACU_PLUS,
	JOB_ATMODE,
	JOB_GET_ZONE,
	JOB_GET_COMBI,
	JOB_START,
	JOB_INPUTS,
	JOB_OUTPUTS,
	JOB_PROFILES,
};


class	QuadSPI;
class	USBFATFS;
class	NCU;
class Pipe;
class QuadSPI;
class PipeManager;
class USBFATFS;
class EEPROM;
class GPIOManager;
class IOStatus;
class EmbeddedWizard;

class Settings : public Module
{
	public:
		
	Settings( );
	
	virtual int Init( );
	virtual void Poll( );
	
	int GetSize();
			
	static Panel* 	GetCurrentPanel( );
	static Site*    GetCurrentSite( );
	 
		
	int ValidateSite( int offset, bool fault );
	
	public:
		
	static ATCmd           commands_buff[ ATCMD_MAX_COMMANDS ];
	static Circular<ATCmd> commands;
	
	DeviceConfig* pold;
	DeviceConfig  dnew;
	

	static bool at_errors;
	static AcuCmd acu_cmd;
	static int acu_encrypt;

	static int acu_id;
	static char acu_key[ 33];
	static int  reply_ok;
	static char reply[ 8];
	static DeviceConfig new_device;
		
	int acu_connected;
	int acu_baud;
	
	int SetupDevice( int zone, int unit, int type );	
	int FlashDevice( );
	
	static int ZoneDisabled( int zone );
	
	int GetPanelIndex( char* name );

	int		GetCurrentLength( );	
	int*    	GetChecksum( );
	
	ChannelType GetChannelFeat( DM_DeviceType_t dt );   
	
	int GetWriteAddress( );
	int SetReadAddress( int );
	
	static DeviceConfig*  FindConfig( int unit );
	DeviceConfig*  FindType( int type );
	
	int WriteDevice( DeviceConfig* dest, DeviceConfig& dev );

	void CheckPipes( );

	int SiteLegal( Site* s );
	static int CheckSite( Site* );
	void DumpPanel( Panel* );
	
	static void UpdateRoutingSounderLEDs( );
		
	QuadSPI* 			quadspi;
	USBFATFS* 			usbfatfs;
	NCU*					ncu;
	EEPROM*				eeprom;
	GPIOManager* 		gpio;
	IOStatus*  	   	iostatus;
	EmbeddedWizard* 	ew;
	
	static int new_unit;
	
	static int 		rom_address;
	int 				prev_pos;
	static int  	current_pos;	
	static bool		unit_sent;
	
	static int fire_routing_disabled;	
	static int alarms_disabled;

	static ATJob		job;
	static ATJob		next_job;
	
	static ATCmd* 		cmd;
 
	int 		current_user;
	static int access_level;
 
	int 		factory_resetting;
	int 		blanklength;
	
	virtual int Receive( Command* );
	virtual int Receive( Event event );
	virtual int Receive( Message* m );
	
	RadioDevice* 	newdevice;
	
	int 				checkflags;
	int 				checkcount;
 
 	int	LoadSettings( );
	int   EraseFlash( );
 
	void UpdateAllDevices( Panel* old, int num_old );
	void UpdateDevice( const DeviceConfig* pold, const DeviceConfig* pnew );
	
	static DeviceConfig* FindFireRoutingDevice( DeviceConfig* start );
	
	char* DateFileName( char* s );
	
	int CheckDelayStatus( );
	
	int UpdateRevision( );
	
	static Zone* SiteZone( int zonenum );
//	int NumZonesUsed( int panel );
	
};


#endif
