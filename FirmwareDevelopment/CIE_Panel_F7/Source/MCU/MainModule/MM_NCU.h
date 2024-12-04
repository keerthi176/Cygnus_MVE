/***************************************************************************
* File name: MM_NCU.h
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
* NCU control module
*
**************************************************************************/

#ifndef _MM_NCU_H
#define _MM_NCU_H

/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_list.h"
#include "MM_Device.h"
#include "MM_IOStatus.h"
#include "MM_RealTimeClock.h"
#include "CO_RBU.h"
#include "MM_EmbeddedWizard.h"
#include "DM_UartHandler.h"
#include "Logger.h"


/* Defines
**************************************************************************/
#define NCU_RX_BUFFER_SIZE 	   		64
#define NCU_TX_MSG_SIZE 		   		64 
#define NCU_MAX_QUEUE_MESSAGES   		520
#define NCU_MAX_FAST_QUEUE_MESSAGES   	520


#define NCU_FAULT_INSTALLATION_TAMPER  		1
#define NCU_FAULT_DISMANTLE_TAMPER 				2
#define NCU_FAULT_LOW_BATTERY						4
#define NCU_FAULT_LOW_LINK_QUALITY				8
#define NCU_FAULT_LOW_NUMBER_OF_PARENTS		16
#define NCU_FAULT_DETECTOR_FAULT					32
#define NCU_FAULT_BEACON_FAULT					64
#define NCU_FAULT_SOUNDER_FAULT					128



typedef enum
{
	NCU_PLUS_PLUS_PLUS = 0, 		
	NCU_GET_STAGE, 			
	NCU_SET_STAGE,				
	NCU_MSG_STATUS, 			
	NCU_NUM_DEVICES, 			
	NCU_FIRE_MSG, 				
	NCU_ALARM_MSG,				
	NCU_FAULT_MSG, 			
	NCU_MISC_MSG,  			
//	NCU_REMOVE_FIRE, 			
//	NCU_REMOVE_FAULT,			
//	NCU_REMOVE_MISC,	 		
//	NCU_REMOVE_ALARM,			
	NCU_CLEAR_QUEUE,			
	NCU_FW_VERSION, 			
	NCU_SERIAL_NUM,  			
	NCU_ID, 			
	NCU_READ_LED,
	NCU_WRITE_LED,
	
	NCU_OPTIMIZE_MESH,
			
	NCU_READ_ANALOGUE, 		
	NCU_SET_FLASH_RATE, 		
	NCU_GET_FLASH_RATE, 		
	NCU_GET_COMBINATION, 	
	NCU_GET_NEIGHBOUR,  		
	NCU_GET_FLAGS,   			
	NCU_ENABLE_PLUGIN,   	
	NCU_ENABLE_DEVICE,  		
	NCU_ENABLE_RADIO,			
	NCU_GET_RADIO_FW,			
	NCU_GET_PLUGIN_FW,		
	NCU_GET_MESH_STATUS,		
	NCU_GET_MESH_OTA_STATUS,
	NCU_ACTIVATE,				
	NCU_REBOOT_RADIO,			
	NCU_PLUGIN_SERIAL_NUM,	
	NCU_RADIO_SERIAL_NUM, 	
	NCU_SET_TONE,				
	NCU_GET_TONE,				
	NCU_SET_UPPER,				
	NCU_GET_UPPER,				
	NCU_SET_LOWER,       	
	NCU_GET_LOWER,				
	NCU_PLUGIN_TEST_MODE,	
	NCU_CHECK_PLUGIN_MODE,	
	NCU_MISC,			
	NCU_STATUS,		
	NCU_MESH,
	NCU_SET_VOLUME,
	NCU_RESET,
	NCU_SET_PPU_DEVICE,
	NCU_SET_PPU_ZONE,
	NCU_SET_PPU_ALL,
    /* Commands added for CYG2-1707 bug resolution */
	NCU_EXIT_TEST_DEVICE,
	NCU_EXIT_TEST_ZONE,
	NCU_EXIT_TEST_ALL,
	
	NCU_BROADCAST_ANALOGUE_REQUEST,
	NCU_BROADCAST_OUTPUT,
	NCU_BROADCAST_TEST_TONE,
	NCU_BROADCAST_RESET,
	NCU_BROADCAST_CONFIRM_FIRE,
	NCU_BROADCAST_CONFIRM_EVENT,
	NCU_BROADCAST_ACKNOWLEDGE_FIRE,
	NCU_BROADCAST_ACKNOWLEDGE_EVENT,
	NCU_BROADCAST_ENABLE_DELAYS,
	NCU_BROADCAST_ENABLE_ZONE,
	NCU_BROADCAST_ENABLE_DEVICE,
	NCU_BROADCAST_ENABLE_CHANNEL,
	NCU_BROADCAST_DAYNIGHT,
	
	NCU_BROADCAST_ENABLE_LOWER_ZONES,
	NCU_BROADCAST_ENABLE_UPPER_ZONES,
	NCU_BROADCAST_GLOBAL_SETTINGS,
	
	NCU_BROADCAST_GLOBAL_DELAY,
	NCU_BROADCAST_GLOBAL_OVERRIDE,	
	
	NCU_BROADCAST_FW_CHECK,
	NCU_BROADCAST_MESH_RESET,
	
	NCU_CONFIG_OUTPUT_FLAGS,	 	
	NCU_CONFIG_OUTPUT_ACTIONS,	 
	NCU_CONFIG_OUTPUT_DELAYS, 
	NCU_CONFIG_LOCAL_DELAY,
	
	NCU_EVENT_FAULT_RESET,

	NCU_CUSTOM_MSG,
	
} NCUMessage;	


#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif

#define MAX_MSG_ARGS 11

enum MeshLogType
{
	MESHLOG_LINK = 0,
	MESHLOG_STATUS,
	MESHLOG_DROP,
	MESHLOG_ALARM,
	MESHLOG_BROADCAST = 4,
	MESHLOG_MISC,
	MESHLOG_FAULT,
	MESHLOG_STATE,
} ;
	

typedef struct
{
	NCUMessage 		msg;           // index
	const char* 	query;         // query format string
	short				type;          // query type
	short				logtype;			// log type
	const char* 	reply;	      // reply format string
	const char*		friendly;
} NCUMsgFormat;	


// Callback on receipt of message sent
/*************************************************************************/
/**  \fn      typedef void NCUCallback( Module*, int success )
***  \brief   Local helper function
**************************************************************************/

typedef void NCUCallback( Module*, int success );


typedef struct
{
	const NCUMsgFormat* 	fmt;                    // Message format description
	int				txargs;                 // number of args int tx
	int 				rxargs;						// number of args int rx
	int     			arg[ MAX_MSG_ARGS ];    // pointer to args to fill
	NCUCallback* 	func;                   // callback
	Module*			user;                   // callback owner 
	char				resend;                 // auto-resend flag
	signed char		retries;						// retries
	char*				cmsg;
} NCUQuery;	


typedef enum
{
	NCU_STATE_UNKNOWN    = -3,             // Not in AT mode
	NCU_STATE_PRE_AT     = -2,
	NCU_STATE_AT         = -1,             // In AT mode
	NCU_STATE_IDLE       = 0,		         // Idle mode   
	NCU_STATE_SYNC       = 1,		         // Sync stage
	NCU_STATE_FORMATION  = 2,              // Formation stage
	NCU_STATE_ACTIVE     = 3,              // Active stage
	NCU_STATE_TEST_MODE  = 4,              // Test mode
} NCUState;


typedef struct 
{
	short active;
	short meshed;
	short total;
	short extra;	
} NCUMeshStatus;	


typedef struct 
{
	short current;
	short target;
} NCUMeshStage;	
	

class Settings;
class Message;


class NCU : public Module, public UartHandler, public TimerHandler
{
	public:
	
	NCU( );
	virtual int Init( );
	
   // Module imp.
	virtual int Receive( Event event );
	virtual int Receive( Command* );
	virtual int Receive( Message* ); 
	
	virtual void Poll( void );
	
	virtual void TimerPeriodElapsed( TIM_HandleTypeDef *htim );
	
 	virtual void UartCommand( UART_HandleTypeDef* uh );
 	virtual void UartTxComplete( UART_HandleTypeDef* uh );
 	virtual void UartError( UART_HandleTypeDef* uh );	
	virtual void UartAbort( UART_HandleTypeDef* uart );
  // virtual void RxISR( UART_HandleTypeDef *huart );
	
	void GenMesh( );
	void DropUnit( int unit );
	
	void SetState( NCUState state );
	void UpdateSiteSettings( void );
	
	void UpdateMeshStatusNow( void );
	
	time_t time_to_update_mesh;
	
	bool global_status_broadcasted;
	
	static NCUMeshStatus current_status;
	static short unused;
	
	int OnTree( int unit );
	
	int QueueMsg( bool autoresend, NCUMessage type, Module*, NCUCallback* func, short& a = unused, short& b = unused, short& c = unused, short& d = unused, short& e = unused, short& f = unused,
																		short& g = unused, short& h = unused, short& i = unused, short& j = unused, short& k = unused  );

	int QueueMsgFast( bool autoresend, NCUMessage type, Module*, NCUCallback* func, short& a = unused, short& b = unused, short& c = unused, short& d = unused, short& e = unused, short& f = unused,
																		short& g = unused, short& h = unused, short& i = unused, short& j = unused, short& k = unused  );
	/*
	int QueueReplaceFront( bool autoresend, NCUMessage type, Module*, NCUCallback* func, short& a = unused, short& b = unused, short& c = unused, short& d = unused, short& e = unused, short& f = unused,
																		short& g = unused, short& h = unused, short& i = unused, short& j = unused, short& k = unused  );
*/
	NCUQuery* CreateQuery( bool autoresend, NCUMessage msg, Module* m, NCUCallback* func, short& a, short& b, short& c, short& d, short& e, short& f,
																		short& g, short& h, short& i, short& j, short& k );

	
   int QueueWriteMsg( bool autoresend, NCUMessage msg, Module* m, NCUCallback* func, short a =-1, short b =-1, short c =-1, short d =-1, short e =-1, short f =-1,
																		short g =-1, short h =-1, short i =-1, short j =-1, short k =-1 );
	
	int QueueWriteMsgFast( bool autoresend, NCUMessage msg, Module* m, NCUCallback* func, short a =-1, short b =-1, short c =-1, short d =-1, short e =-1, short f =-1,
																		short g =-1, short h =-1, short i =-1, short j =-1, short k =-1 );
	
	
	int QueueMsg( bool autoresend, NCUMessage msg, char* cmsg, Module* m, void (*func)( char* ) );
	
	 
	void Execute( NCUQuery* q );
	
	int NumPrimaryChildren( int unit );
	int NumSecondaryChildren( int unit );
	
	void UpdateMeshStatus( );	
	
	RadioUnit* Status( int zone, int unit, int battprimary, int battbackup,int combination );
 
	
	
	short stage;
	short Fire_Msg, Fire_Lost, Alarm_Msg, Alarm_Lost, Fault_Msg, Fault_Lost, Misc_Msg, Misc_Lost, Tx_Buffer;

	int dont_re_order;
	
	short targetState;
	static short currentState;
	
	short zoneone;
	
	short fireunit, firechannel, firestate, firevalue;
	short alarmunit, alarmchannel, alarmstate, alarmvalue;
	short faultunit, faultchannel, faulttype, faultvalue, faultmore;
	short meshunit, meshprimary, meshrssiprimary, meshsecondary, meshrssisecondary;
	short serialunit, serialmm, serialyy, serialbb, serialss;
	short combunit, combvalue;
	short checkzone, checkunit, checka, checkb, checkc;
	short ncufw2, ncufw1, ncufw0, ncufwdd, ncufwmm, ncufwyy, ncufwch;
	short anazone, anaunit, anachan, anavalue;
	
	static char	unit_buffer[ SITE_MAX_RADIO_DEVICES * sizeof(RadioUnit) ];	// to bypass contructors
	
	static RadioUnit* 		units;
	static RadioUnit* 		unit_pos;
	
	static volatile int 	finished_tx;	
	static int 				directing_to_USB_eek;
	static uint8_t 		usbbuff[ 64];
	static uint8_t* 		usbbuffptr;
	static int				usbcount;
	
	static RadioUnit* 		FindUnit( int unit ); 
	
	IOStatus* 		iostatus;
	RealTimeClock* rtc;
	EmbeddedWizard* ew;
	GSM*				gsm;
	
	RadioUnit* 		stats_device;	
	RadioUnit*     sent_device;
	int				stats_ready;
	int 				status_event;
	
	int				prevFault;
	static int 		MsgReceivedOK;	
	
	static NCUQuery*  msgAwaiting;
	Settings* 			settings;
	bool 					restartingMesh;
 
	char* txbuffer = (char*) 0x20000000;	// Use non cachable memory for DMA
	char* rxbuffer = (char*) 0x20000040;	// Use non cachable memory for DMA
	
	time_t lastRxTime = UINT_MAX - 100;
	
   Circular<NCUQuery> sendqueue;
	Circular<NCUQuery> fastqueue;
	
	int got_ncu_fw;
	
	private:
	
	Circular<NCUQuery> delayedqueue;

	int fails;   
	int time_to_check;
	int reset_rx_request;
	
 
	void HandleLogon( );
	int  HandleStatus( NCUMessage i );
	void HandleMesh( );
	void HandleMisc( NCUMessage i );
	void LinkParent( int child, int parent, bool definite );
	void CheckStatus( );

	int  DecodeOK( NCUQuery* q );
	int HasCircularDependance( int unit );
	int HasAnyCircularDependance( );
	void LogMsg( short lt, const char* fmt, ... );
	void LogMsg( NCUQuery* q, va_list args );
	
	int  rx_fail_count;	

	int  faulting;
	int quick_advance;

	public:
		
	static Logger logger;
};



#endif
