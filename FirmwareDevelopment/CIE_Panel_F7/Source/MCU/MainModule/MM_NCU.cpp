/***************************************************************************
* File name: MM_NCU.cpp
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

/* System Include Files
**************************************************************************/
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
 

#ifdef STM32F767xx
#include "stm32f7xx_hal_dma.h"
#include "stm32f7xx_hal_uart.h"
#else
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_uart.h"
#endif
#include "stm32f7xx_hal_rtc.h"

#include "usbd_cdc_if.h"
#include <stdlib.h>


/* User Include Files
**************************************************************************/

extern "C"
{
#include "_AppLibClass.h"
}

#include "DM_BatteryRam.h"
#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_NCU.h"
#include "MM_Settings.h"
#include "MM_fault.h"
#include "MM_log.h"
#include "CO_Site.h"
#include "CO_RBU.h"
#include "MM_IOStatus.h"
#include "MM_CUtils.h"
#include "MM_RadioDevice.h"


#ifdef GSM_HTTP
#include "MM_AppSupport.h"
#else
#include "MM_MQTTSupport.h"
#endif


#include "MM_CUtils.h"
#include "CO_Site.h"
#include "Logger.h" 

#define MAGIC_NUM 69
 
enum
{
	READ = 1,
	WRITE,
	MISC,
	DELAYED,
	PLUSOK,
	AUTO,
	CUSTOM,
};

enum
{
	EVENT_NCU = 0,
	EVENT_CHILD_NODE_ADDED,
	EVENT_CHILD_NODE_DROPPED,
	EVENT_PRIMARY_PARENT_ADDED,
	EVENT_PRIMARY_PARENT_DROPPED,
	EVENT_SECONDARY_PARENT_ADDED,
	EVENT_SECONDARY_PARENT_DROPPED,
	EVENT_PRIMARY_TRACKING_NODE_ADDED,
	EVENT_PRIMARY_TRACKING_NODE_DROPPED,
	EVENT_SECONDARY_TRACKING_NODE_ADDED,
	EVENT_SECONDARY_TRACKING_NODE_DROPPED,
	EVENT_SECONDARY_PARENT_PROMOTED,
};


uint8_t NCU::usbbuff[ 64];
uint8_t* NCU::usbbuffptr;
int		NCU::usbcount;


NCUMeshStatus NCU::current_status;

short NCU::unused = 0x8000;
int NCU::MsgReceivedOK = 0;
int NCU::directing_to_USB_eek = false;

Logger NCU::logger;

short NCU::currentState = NCU_STATE_UNKNOWN;

static NCUQuery  send_buffer[ NCU_MAX_QUEUE_MESSAGES ];
static NCUQuery  fast_buffer[ NCU_MAX_FAST_QUEUE_MESSAGES ];

static NCUQuery delay_buffer[ NCU_MAX_QUEUE_MESSAGES ];

char NCU::unit_buffer[ SITE_MAX_RADIO_DEVICES * sizeof(RadioUnit) ];
RadioUnit* 		NCU::units = (RadioUnit*) unit_buffer;
RadioUnit* 		NCU::unit_pos = NCU::units;


/* Defines
**************************************************************************/

#define NCU_MAX_FAILS 10
#define NCU_MAX_RANKS 33

#define MAX_SERVER_GAP 60*15*20

#define USB_BUFF_SIZE 64

#define ENDOF(x) (x + SITE_MAX_RADIO_DEVICES)

#define ZONE( array, zone ) (( array[ (zone-1) >> 5 ] >> ((zone-1) & 31 ) ) & 1 ) 


#define NCU_ALL_ZONES	4095
#define NCU_ALL_UNITS 	4095

#define NCU_MAX_RETRYS 	4

#define MSG_DAY_NIGHT	1
 

const static NCUMsgFormat formatlist[ ] =
{
	{ NCU_PLUS_PLUS_PLUS, 		"+++\r\n", PLUSOK, -1, NULL  },
	{ NCU_GET_STAGE, 				"ATMFS?\r\n", READ, MESHLOG_STATE, "MFS: %d", "Mode: %d" },
	{ NCU_SET_STAGE,				"ATMFS=%d\r\n", WRITE, -1, NULL, NULL },
	{ NCU_MSG_STATUS, 			"ATMSN?\r\n", READ, -1, "MSN: %d,%d\n%d,%d\n%d,%d\n%d,%d\n%d", NULL },
	{ NCU_NUM_DEVICES, 			"ATNOD?\r\n", READ, MESHLOG_MISC, "NOD: %d,%d", "Zones: %d, Devices: %d" },
	{ NCU_FIRE_MSG, 				"ATQFE?\r\n", READ, MESHLOG_ALARM, "QFE: Z%dU%d,%d,%d,%d", "Fire zone %d unit %d chan %d state %d" },
	{ NCU_ALARM_MSG,				"ATQAM?\r\n", READ, MESHLOG_ALARM, "QAM: Z%dU%d,%d,%d,%d", "Alarm zone %d unit %d chan %d state %d" },
	{ NCU_FAULT_MSG, 				"ATQFT?\r\n", READ, MESHLOG_FAULT, "QFT: Z%dU%d,%d,%d,%d,%d", "Fault zone %d unit %d chan %d num %d Fault %d" },
	{ NCU_MISC_MSG,  				"ATQMC?\r\n", MISC, -1, NULL, NULL },

	{ NCU_CLEAR_QUEUE,			"ATXMQ=%d,%d\n%d,%d\n%d,%d\n%d,%d\n%d\r\n", WRITE, -1, NULL, NULL }, 
	{ NCU_FW_VERSION, 			"ATFI?\r\n", READ, MESHLOG_MISC, "FI: %d.%d.%d,%d/%d/%d", "Firmware %d.%d.%d %d/%d/%d" },  
	{ NCU_SERIAL_NUM,  			"ATSERNO?\r\n", READ, MESHLOG_MISC, "SERNO: %04d-%02d-%05d", "NCU Serial: %04d-%02d-%05d" },
	{ NCU_ID, 						"ATBID?\r\n", READ, MESHLOG_MISC, "BID: %d", "NCU ID: %d" },
	{ NCU_READ_LED,				"ATLED?Z%dU%d\r\n", DELAYED, MESHLOG_STATE, "LED:Z%dU%d,%d", "LED zone %d unit %d state %d" },
	{ NCU_WRITE_LED,				"ATLED=Z%dU%d,%d\r\n", DELAYED, -1, "LED:Z%dU%d,%d", NULL },
			
	{ NCU_OPTIMIZE_MESH, 		"ATSCAN+Z%dU%d\r\n", WRITE, -1, NULL, NULL }, 		
	{ NCU_READ_ANALOGUE, 		"ATANA?Z%dU%d,%d\r\n", DELAYED, MESHLOG_MISC, "ANA: Z%dU%d,%d,0,1,%d", "Analogue zone %d unit %d chan %d state %d" },
	{ NCU_SET_FLASH_RATE, 		"ATBFR=Z%dU%d,%d,%d,%d\r\n", WRITE, -1, NULL },	
	{ NCU_GET_FLASH_RATE, 		"ATBFR?Z%dU%d,%d,%d,%d\r\n", DELAYED, -1, "BFR: Z%dU%d,%d,%d,1,%d", NULL },
	{ NCU_GET_COMBINATION, 		"ATDEC?Z%dU%d\r\n", DELAYED, MESHLOG_STATE, "DEC: Z%dU%d,0,0,1,%d", "Combination zone %d unit %d num %d" }, 
	{ NCU_GET_NEIGHBOUR,  		"ATDNI?Z%dU%d\r\n",  DELAYED, -1, "DNI: Z%dU%d,%d,%d,%d,%d", NULL  },
	{ NCU_GET_FLAGS,				"ATDSF?Z%dU%d\r\n",  DELAYED, -1, "DSF: Z%dU%d,0,0,1,%d", NULL }, 
	{ NCU_ENABLE_PLUGIN,			"ATEDP=Z%dU%d,%d,%d\r\n", WRITE, -1, NULL }, 
	{ NCU_ENABLE_DEVICE,  		"ATEDP?Z%dU%d,%d\r\n", DELAYED, -1, "EDP: Z%dU%d,%d,0,1,%d", NULL },
	{ NCU_ENABLE_RADIO,			"????", WRITE, -1, NULL, NULL },
	{ NCU_GET_RADIO_FW,			"ATFIR?Z%dU%d\r\n", DELAYED, MESHLOG_STATE, "FIR: Z%dU%d,0,0,1,%d.%d.%d", "Ping zone %d unit %d" },	
	{ NCU_GET_PLUGIN_FW,			"ATFIP?Z%dU%d\r\n", DELAYED, -1, "FIP: Z%dU%d,0,0,2,%d.%d.%d,%d/%d/%d", NULL },	
	{ NCU_GET_MESH_STATUS,		"ATMSR?Z%dU%d\r\n", WRITE, MESHLOG_STATUS, "MSR:Z%dU%d,%d,%d,9,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",   "Unit %d 1st %d 2nd %d rssi %d rssi2 %d batt %d rank %d type %d fault %d" },
	{ NCU_GET_MESH_OTA_STATUS,	"ATMSR?Z%dU%d,1\r\n", WRITE, MESHLOG_STATUS, "MSR:Z%dU%d,%d,%d,9,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d", "Unit %d 1st %d 2nd %d rssi %d rssi2 %d batt %d rank %d type %d fault %d" },
	{ NCU_ACTIVATE,				"ATOUT+Z%dU%d,%d,%d,%d,%d\r\n", WRITE, -1, NULL, NULL },
	{ NCU_REBOOT_RADIO,			"ATRRU+Z%dU%d\r\n", WRITE, -1, NULL, NULL },
	{ NCU_PLUGIN_SERIAL_NUM,	"ATSNP?Z%dU%d\r\n", DELAYED, MESHLOG_STATE, "SNP: Z%dU%d,0,0,1,%02d%02-%d-%d", "Plugin serial zone %d unit %d %02d%02-%d-%d" },
	{ NCU_RADIO_SERIAL_NUM, 	"ATSNR?Z%dU%d\r\n", DELAYED, MESHLOG_STATE, "SNR:Z%dU%d,0,0,1,%02d%02-%d-%d", "Radio serial zone %d unit %d %02d%02-%d-%d" },
	{ NCU_SET_TONE,				"ATSTS=Z%dU%d,%d,%d,%d\r\n", WRITE, -1, NULL, NULL },
	{ NCU_GET_TONE,				"ATSTS?Z%dU%d,%d,%d\r\n", DELAYED, -1, "STS: Z%dU%d,%d,%d,1,%d", NULL },
	{ NCU_SET_UPPER,				"ATTHU=Z%dU%d,%d,%d\r\n", WRITE, -1, NULL, NULL },
	{ NCU_GET_UPPER,				"ATTHU?Z%dU%d,%d\r\n", DELAYED, -1, "THU: Z%dU%d,%d,0,1,%d", NULL },
	{ NCU_SET_LOWER,		 		"ATTHL=Z%dU%d,%d,%d\r\n", WRITE, -1, NULL, NULL },
	{ NCU_GET_LOWER,				"ATTHL?Z%dU%d,%d\r\n", DELAYED, -1, "THL: Z%dU%d,%d,0,1,%d", NULL },
	{ NCU_PLUGIN_TEST_MODE,		"ATTMP=Z%dU%d,%d,%d\r\n", WRITE, -1, NULL, NULL },
	{ NCU_CHECK_PLUGIN_MODE,	"ATTMP?Z%dU%d,%d\r\n", DELAYED, NULL, "TMP: Z%dU%d,%d,0,1,%d", NULL },
	{ NCU_MISC,						NULL,	AUTO, MESHLOG_STATUS, "MSC:Z%dU%d,%d,%d,%d,%d,%d,%d,%d", "Unit %d, batt %d, backup %d, combo %d, zone %d, smoke %d, heat %d, pir %d" },
	{ NCU_STATUS,					NULL,	AUTO, MESHLOG_STATUS, "MSI:Z%dU%d,%d,%d,9,%d,%d,%d,%d,%d,%d,%d,%d,%d", "Unit %d 1st %d 2nd %d rssi %d rssi2 %d batt %d rank %d type %d fault %d" },																																				
	{ NCU_MESH,						NULL, AUTO, 0, "MESH:%d,%d,%d" },
	
	{ NCU_SET_VOLUME,				"ATSL=Z%dU%d,%d,%d\r\n", WRITE, -1, NULL, NULL },
	
	{ NCU_RESET,					"ATR+Z%dU%d\r\n", WRITE, MESHLOG_MISC, NULL },	
	{ NCU_SET_PPU_DEVICE,		"ATPPUMD+Z%dU%d\r\n", WRITE, MESHLOG_MISC, NULL },
	{ NCU_SET_PPU_ZONE,			"ATPPUMD+Z%dU4095\r\n", WRITE, MESHLOG_MISC, NULL },
	{ NCU_SET_PPU_ALL,			"ATPPUMD+\r\n", WRITE, MESHLOG_MISC, NULL },
  /* Commands added */
  { NCU_EXIT_TEST_DEVICE,		 "ATETEST+Z%dU%d\r\n", WRITE, MESHLOG_MISC, NULL },
	{ NCU_EXIT_TEST_ZONE,	     "ATETEST+Z%dU4095\r\n", WRITE, MESHLOG_MISC, NULL },
	{ NCU_EXIT_TEST_ALL,	     "ATETEST+\r\n", WRITE, MESHLOG_MISC, NULL },
	 
	{ NCU_BROADCAST_ANALOGUE_REQUEST, "ATSENV+\r\n", WRITE, MESHLOG_BROADCAST, NULL },		
	{ NCU_BROADCAST_OUTPUT,			"ATAOS+%d,%d,%d\r\n", WRITE, MESHLOG_BROADCAST, NULL },
	{ NCU_BROADCAST_TEST_TONE,		"ATTOS+%d\r\n", WRITE, MESHLOG_BROADCAST, NULL },
	{ NCU_BROADCAST_RESET,			"ATRST+\r\n", WRITE, MESHLOG_BROADCAST, NULL },
	{ NCU_BROADCAST_CONFIRM_FIRE,	"ATCONF+\r\n", WRITE, MESHLOG_BROADCAST, NULL },
	{ NCU_BROADCAST_CONFIRM_EVENT,"ATCONE+\r\n", WRITE, MESHLOG_BROADCAST, NULL },
	
	{ NCU_BROADCAST_ACKNOWLEDGE_FIRE,	"ATACKF+\r\n", WRITE, MESHLOG_BROADCAST, NULL },
	{ NCU_BROADCAST_ACKNOWLEDGE_EVENT,	"ATACKE+\r\n", WRITE, MESHLOG_BROADCAST, NULL },
	{ NCU_BROADCAST_ENABLE_DELAYS, "ATAGD+%d\r\n", WRITE, MESHLOG_BROADCAST, NULL },
	
	{ NCU_BROADCAST_ENABLE_ZONE,  "ATDISZ+%d,%d\r\n", WRITE, MESHLOG_BROADCAST, NULL },
	{ NCU_BROADCAST_ENABLE_DEVICE, "ATDISD+Z%dU%d,%d\r\n", WRITE, MESHLOG_BROADCAST, NULL },   
	{ NCU_BROADCAST_ENABLE_CHANNEL, "ATDISC+Z%dU%d,%d,%d,%d\r\n", WRITE, MESHLOG_BROADCAST, NULL },
	{ NCU_BROADCAST_DAYNIGHT,		"ATSDN=%d\r\n", WRITE, MESHLOG_BROADCAST, NULL },	
	
	{ NCU_BROADCAST_ENABLE_LOWER_ZONES, "ATLOZ=%d\r\n", WRITE, MESHLOG_BROADCAST, NULL },
	{ NCU_BROADCAST_ENABLE_UPPER_ZONES, "ATUPZ=%d\r\n", WRITE, MESHLOG_BROADCAST, NULL },
	
	{ NCU_BROADCAST_GLOBAL_SETTINGS, "ATGSET+%d,%d,%d,%d,%d,%d,%d\r\n", WRITE, MESHLOG_BROADCAST, NULL },
		
	{ NCU_BROADCAST_GLOBAL_DELAY, "ATGADC=%d,%d\r\n", WRITE, MESHLOG_BROADCAST, NULL },
	
	{ NCU_BROADCAST_GLOBAL_OVERRIDE, "ATGDLY=%d\r\n", WRITE, MESHLOG_BROADCAST, NULL },
	
	{ NCU_BROADCAST_FW_CHECK, "ATCHFW+\r\n", WRITE, MESHLOG_BROADCAST, NULL },
	{ NCU_BROADCAST_MESH_RESET, "ATR+Z4095U4095\r\n",WRITE, MESHLOG_BROADCAST, NULL }, 
	
	{ NCU_CONFIG_OUTPUT_FLAGS,	   "ATCHF=Z%dU%d,%d,%d\r\n", WRITE, -1, NULL },	
	{ NCU_CONFIG_OUTPUT_ACTIONS,	"ATACS=Z%dU%d,%d,%d\r\n", WRITE, -1 , NULL},	   
	{ NCU_CONFIG_OUTPUT_DELAYS,	"ATADC=Z%dU%d,%d,%d,%d\r\n", WRITE, -1, NULL },	
	{ NCU_CONFIG_LOCAL_DELAY,		"ATLDLY=Z%dU%d,%d,%d\r\n", WRITE, -1, NULL },	
	
	{ NCU_EVENT_FAULT_RESET,		"ATRST+Z%dU%d,%d\r\n", WRITE, -1, NULL },	
	
	{ NCU_CUSTOM_MSG,					NULL, CUSTOM, MESHLOG_MISC, NULL }
};				  
	

NCUQuery* NCU::msgAwaiting 	= NULL;	


const char* ErrorStr( int c )
{
	switch( c )
	{        
		case HAL_UART_ERROR_PE       : return "Parity error";           
		case HAL_UART_ERROR_NE       : return "Noise error";           
		case HAL_UART_ERROR_FE       : return "Frame error";           
		case HAL_UART_ERROR_ORE      : return "Overrun error";           
		case HAL_UART_ERROR_DMA 	  : return "DMA Error";
		default: return "No error";  
	}
}


/*************************************************************************/
/**  \fn		static void TxStage( Module* me, int succeeded )
***  \brief	Local helper function
**************************************************************************/

static void TxStage( Module* me, int succeeded )
{
	// OK
}
 

/*************************************************************************/
/**  \fn		void RxStatus( Module* me, int succeeded )
***  \brief	Global helper function
**************************************************************************/

void RxStatus( Module* me, int succeeded );

void DumpUart( UART_HandleTypeDef* huart )
{
	app.DebOut( "Error code: %s\n", ErrorStr( huart->ErrorCode ) );
	
	int isr = huart->Instance->ISR;
	
	app.DebOut( "ISR: %X\n", isr );
	
	if ( isr & USART_ISR_PE_Msk   	)   app.DebOut( "Parity Error                       \n" ); 
	if ( isr & USART_ISR_FE_Msk   	)   app.DebOut( "Framing Error                      \n" ); 
	if ( isr & USART_ISR_NE_Msk   	)   app.DebOut( "Noise detected Flag                \n" ); 
	if ( isr & USART_ISR_ORE_Msk  	)   app.DebOut( "OverRun Error                      \n" ); 
	if ( isr & USART_ISR_IDLE_Msk 	)   app.DebOut( "IDLE line detected                 \n" ); 
	if ( isr & USART_ISR_RXNE_Msk 	)   app.DebOut( "Read Data Register Not Empty       \n" ); 
	if ( isr & USART_ISR_TC_Msk   	)   app.DebOut( "Transmission Complete              \n" ); 
	if ( isr & USART_ISR_TXE_Msk  	)   app.DebOut( "Transmit Data Register Empty       \n" ); 
	if ( isr & USART_ISR_LBDF_Msk 	)   app.DebOut( "LIN Break Detection Flag           \n" ); 
	if ( isr & USART_ISR_CTSIF_Msk	)   app.DebOut( "CTS interrupt flag                 \n" ); 
	if ( isr & USART_ISR_CTS_Msk  	)   app.DebOut( "CTS flag                           \n" ); 
	if ( isr & USART_ISR_RTOF_Msk 	)   app.DebOut( "Receiver Time Out                  \n" ); 
	if ( isr & USART_ISR_EOBF_Msk 	)   app.DebOut( "End Of Block Flag                  \n" ); 
	if ( isr & USART_ISR_ABRE_Msk 	)   app.DebOut( "Auto-Baud Rate Error               \n" ); 
	if ( isr & USART_ISR_ABRF_Msk 	)   app.DebOut( "Auto-Baud Rate Flag                \n" ); 
	if ( isr & USART_ISR_BUSY_Msk 	)   app.DebOut( "Busy Flag                          \n" ); 
	if ( isr & USART_ISR_CMF_Msk  	)   app.DebOut( "Character Match Flag               \n" ); 
	if ( isr & USART_ISR_SBKF_Msk 	)   app.DebOut( "Send Break Flag                    \n" ); 
	if ( isr & USART_ISR_RWU_Msk  	)   app.DebOut( "Receive Wake Up from mute mode Flag\n" ); 
	if ( isr & USART_ISR_TEACK_Msk	)   app.DebOut( "Transmit Enable Acknowledge Flag   \n" );
	
	app.DebOut( "\n" );
}

int dummy_byte;

/*************************************************************************/
/**  \fn		void NCU::UartError( UART_HandleTypeDef* huart )
***  \brief	Constructor for class
**************************************************************************/

void NCU::UartError( UART_HandleTypeDef* huart )
{
	app.DebOut( "UART ERROR!\n" );
	 
	
	//DumpUart( huart );
	
   if ( huart->Instance->ISR & USART_ISR_ORE_Msk )
	{
		dummy_byte = huart->Instance->RDR; // clear read		
		NCU::reset_rx_request = true;
	}
	 
 	// Reset status
	
	
	
}	



/*************************************************************************/
/**  \fn		NCU::NCU( )
***  \brief	Constructor for class
**************************************************************************/

NCU::NCU( ) : Module( "NCU", 1, EVENT_FIRE_RESET | EVENT_UPDATE | EVENT_DAY_TIME | EVENT_NIGHT_TIME | EVENT_RESET | EVENT_CLEAR ),
										UartHandler( app.ncu_uart ), TimerHandler( app.second_timer ), fastqueue( fast_buffer, NCU_MAX_FAST_QUEUE_MESSAGES ),
										sendqueue( send_buffer, NCU_MAX_QUEUE_MESSAGES ), delayedqueue( delay_buffer, NCU_MAX_QUEUE_MESSAGES )										
{ 
	currentState = NCU_STATE_UNKNOWN;	
	targetState = NCU_STATE_UNKNOWN;
	
	stats_device = NULL;
	stats_ready = false;
	fails = 0;
	rx_fail_count = 0;	
	reset_rx_request = false;

	faulting = false; 
	unit_pos = units;
	
	quick_advance = true;
	restartingMesh = false;
	
	time_to_update_mesh = 0;
} 


/*************************************************************************/
/**  \fn		void Init( )
***  \brief	Local helper function
**************************************************************************/

int NCU::Init( )
{
	settings = ( Settings*) 		app.FindModule( "Settings" );
	iostatus = ( IOStatus*) 		app.FindModule( "IOStatus" );
	ew			= (EmbeddedWizard*)  app.FindModule( "EmbeddedWizard" );
	gsm		= (GSM*)					app.FindModule( "GSM" );
	SDFATFS* sdfatfs = (SDFATFS*) app.FindModule( "SDFATFS" );
	
	assert( settings != NULL );
	assert( iostatus != NULL );
	assert( ew != NULL );
	assert( sdfatfs != NULL );
	assert( gsm != NULL );
	
	logger.Init( sdfatfs, "0:/mesh.log", FA_OPEN_ALWAYS | FA_OPEN_APPEND | FA_WRITE | FA_READ );
	
//	GenMesh();
	
   HAL_TIM_Base_Start_IT( app.second_timer );	
	global_status_broadcasted = false;
	
	lastRxTime = now( );
	got_ncu_fw = false;
	
	return true;
} 
  

/*************************************************************************/
/**  \fn		static void RxAnalogue( Module* me, int succeeded )
***  \brief	Local helper function
**************************************************************************/

static void RxAnalogue( Module* me, int succeeded )
{
	if ( succeeded && app.panel != NULL )
	{
		NCU* dis = (NCU*) me;
		Device* d = dis->iostatus->FindDevice( dis->anaunit );
		
		if ( d != NULL )
		{
			for( int i = 0; i < d->config->numInputs; i++ )
			{
				if ( d->config->input[ i].type == dis->anachan )
				{				
					d->analogue[ i ] = dis->anavalue;
					Message m;
				
					m.type = EW_ON_TEST_CHANGE_MSG;
					m.value = (int) d->config;
					m.to = dis->ew;
					app.Send( &m ); 
				}				
			}		
		}
	}
}

//extern "C" XString AppLibClass_GetCombiName( AppLibClass _this, XInt32 type );


/*************************************************************************/
/**  \fn		int NCU::Receive( Command* cmd )
***  \brief	Class function
**************************************************************************/

int NCU::Receive( Command* cmd )
{
	if ( cmd == NULL ) return CMD_ERR_NULL ;
	
	if ( cmd->type == CMD_GET_MESH_LOG_LENGTH )
	{
		cmd->int0 = logger.NumLogLines( cmd->int0 );
		
		return CMD_OK;
	}
	else if ( cmd->type == CMD_ADD_ADDITIONAL )
	{
		/*    CURRENTLY DISABLED FEATURE 
		
		bool failed = false;
		int done = 0;
		
		for( RadioUnit* ru = units; ru < unit_pos; ru++ )
		{			 
			if ( ( ru->valid & (  VALID_ANONYMOUS | VALID_SELECTED )) == ( VALID_ANONYMOUS | VALID_SELECTED ) )
			{
				if ( ru->zone < 1 ) ru->zone = 1;
				
				char ansi[ 32 ] = "<unknown>";
				
				if ( ru->combination <= 63 )
				{
					XString name = AppLibClass_GetCombiName( NULL, ru->combination );
					 
					EwStringToAnsi( name, ansi, 32, '_' );
				}	 
				
				if ( settings->SetupDevice( ru->zone, ru->unit, ru->combination, ansi ) )
				{
					if ( !settings->FlashDevice( ) )
					{
						failed = true;
					}
					else
					{
						done++;
					}
				}
				else
				{
					failed = true;
				}
			}
		}
		
		cmd->int0 = done;
		
		if ( failed ) return CMD_ERR_WRITE_FAIL;
		*/
		return CMD_OK;
	}
	else if ( cmd->type == CMD_PPU_MODE )
	{
		int zone = cmd->int0;
		int unit = cmd->int1;
		
		if ( zone == NCU_ALL_ZONES )
		{
			QueueWriteMsg( false, NCU_SET_PPU_ALL, this, NULL );
		}
		else
		{
			if ( unit == NCU_ALL_UNITS )
			{
				QueueWriteMsg( false, NCU_SET_PPU_ZONE, this, NULL, zone );
			}
			else
			{
				QueueWriteMsg( false, NCU_SET_PPU_DEVICE, this, NULL, zone, unit );
			}
		}
	}
  /* Received command for exiting the test mode from the panel GUI */
  else if ( cmd->type == CMD_EXIT_TEST )
	{
		int zone = cmd->int0;
		int unit = cmd->int1;
		/* When all the zones are unchecked */
		if ( zone == NCU_ALL_ZONES )
		{
			QueueWriteMsg( false, NCU_EXIT_TEST_ALL, this, NULL );
		}
		else
		{
			/* When individual zone is unchecked */
			if ( unit == NCU_ALL_UNITS )
			{
				QueueWriteMsg( false, NCU_EXIT_TEST_ZONE, this, NULL, zone );
			}
			/* When individual device is unchecked */
			else
			{
				QueueWriteMsg( false, NCU_EXIT_TEST_DEVICE, this, NULL, zone, unit );
			}
		}
	}

	else if ( cmd->type == CMD_OPTIMIZE_MESH )
	{
		QueueWriteMsg( false, NCU_OPTIMIZE_MESH, this, NULL, NCU_ALL_ZONES, NCU_ALL_UNITS );
		
		return CMD_OK;
	}
	else if ( cmd->type == CMD_GET_ANALOGUE_VALUE )
	{
		anazone = 1;
		anaunit = cmd->int0;
		anachan = cmd->int1;
		 
		QueueMsg( false, NCU_READ_ANALOGUE, this, RxAnalogue, anazone, anaunit, anachan, anavalue );
				
		return CMD_OK;
	}
		
	else if ( cmd->type == CMD_GET_MESH_LOG_LINE )
	{
		logger.line = cmd->int0;
		
		if ( logger.GetLine( (char*) cmd->int2, cmd->int3, cmd->int1 ) )
		{
			return CMD_OK;
		}
		else
		{
			return CMD_ERR_OUT_OF_RANGE;
		}
	}
	
	else if ( cmd->type == CMD_SET_NCU_ADVANCE )
	{
		quick_advance = cmd->int0;
		return CMD_OK;
	}
	
	if ( cmd->type == CMD_CHECK_FW )
	{
		Fault::RemoveAllFaults( NCU_BROADCAST_FW_CHECK );
		QueueWriteMsg( false, NCU_BROADCAST_FW_CHECK, this, NULL );
	}
		
	if ( cmd->type == CMD_CHECK_TREE )
	{
		if ( HasAnyCircularDependance( ) )
		{
			return CMD_ERR_OUT_OF_RANGE;
		}
		return CMD_OK;
	}
	if ( cmd->type == CMD_CHECK_ON_TREE )
	{
		cmd->int0 = OnTree( cmd->int0 );
		
		return CMD_OK;
	}
	
	if ( cmd->type == CMD_GET_MESH_STAGE )
	{
		cmd->int0 = currentState;
		
		return CMD_OK;
	}
	else if ( cmd->type == CMD_RESTART_MESH )
	{
		QueueWriteMsg( false, NCU_BROADCAST_MESH_RESET, this, NULL );
		
		Log::Msg( LOG_SYS, "Mesh Restart button pressed." );
				 
		restartingMesh = true;
		
		for( RadioUnit* ru = units; ru < unit_pos; ru++ )
		{
			Device* d = iostatus->FindDevice( ru->unit );
			if ( d != NULL ) iostatus->RemoveDevice( d );
			
			ru->primary = -1;
			ru->secondary = -1;
			ru->droptime = now( );
			ru->state = UNIT_DROPPED;
			ru->rssiprimary = -256;
		}
		UpdateMeshStatus( );
		  
		return CMD_OK;
	}
	else if ( cmd->type == CMD_SET_CURRENT_DEVICE )
	{
		if ( cmd->int0 != -1 && app.panel != NULL )
		{
			RadioUnit* ru = FindUnit( cmd->int0 );
			
			if  ( ru != NULL )
			{
				stats_device = ru;
				ru->sent 	 = 0;
				ru->received = 0;

				checkunit    = ru->unit;
				stats_ready = true;
			
			}
		}	
		else
		{
			stats_device = NULL;
		}
			
		return CMD_OK;		
	}
	else if ( cmd->type == CMD_GET_RADIO_UNIT_CHILD )
	{
		if ( app.panel != NULL )
		{
			RadioUnit* start = (RadioUnit*) cmd->int0;
			int parent = cmd->int1;
			
			app.DebOut( "Get child of %d ", parent );
			
			if ( start == 0 )
			{
				start = units;
			}
			else
			{
				start++;
			}
			
			for( RadioUnit* ru = start; ru < unit_pos; ru++ )
			{
				if ( ru->primary == parent )
				{
					cmd->int0 =	(int) ru;
					
					app.DebOut( "= %d (%X)\n", ru->unit, ru );
					
					
					return CMD_OK;
				}
			}
		}
		
		app.DebOut( "not found.\n" );
		
		cmd->int0 = NULL;
				
		return CMD_OK;
	}
	else if ( cmd->type == CMD_GET_RADIO_UNIT_NOT_ON_MESH )
	{
		int n = 0; 
		
		for( RadioUnit* ru = units; ru < unit_pos; ru++ )
		{
			// if not on mesh
			if ( ru->primary == -1 )
			{
				// find in site
				DeviceConfig* dc = settings->FindConfig( ru->unit );
				
				// if is in site
				if ( dc != NULL )
				{
					// is th is th nth
					if ( cmd->int0 ==	n )
					{
						// return unit
						cmd->int0 = (int) ru;
						return CMD_OK;
					}
					else n++;
				}
			}
		}
		cmd->int0 = 0;
		
		return CMD_ERR_OUT_OF_RANGE;
	}
	else if ( cmd->type == CMD_GET_NUM_MESH_ITEMS )
	{
		int n = 0; 
		for( RadioUnit* ru = units; ru < unit_pos; ru++ )
		{
			if ( ru->primary != -1 )
			{
				n++;
			}
		}
		cmd->int0 = n;
		
		return CMD_OK;		
	}
	else if ( cmd->type == CMD_GET_NCU_FW_VERSION )
	{
		cmd->char0 = ncufw2;
		cmd->char1 = ncufw1;
		cmd->char2 = ncufw0;
		cmd->char3 = 0;
		
		return CMD_OK;
	}	
	else if ( cmd->type == CMD_GET_NUM_EXTRA_RADIO_UNITS )
	{
		int n = 0; 
		for( RadioUnit* ru = units; ru < unit_pos; ru++ )
		{
			if ( ru->valid & VALID_ANONYMOUS )
			{
				n++;
			}
		}
		cmd->int0 = n;
		
		return CMD_OK;				
	}
	else if ( cmd->type == CMD_GET_EXTRA_RADIO_NUM )
	{
		int n = 0; 
		for( RadioUnit* ru = units; ru < unit_pos; ru++ )
		{
			if ( ru->valid & VALID_ANONYMOUS )
			{
				
				if ( n == cmd->int0 )
				{
					cmd->int1 = (int) ru;
					return CMD_OK;
				}
				n++;
			}
		}

		return CMD_ERR_OUT_OF_RANGE;				
	}
	else if ( cmd->type == CMD_FLASH_EXTRA_RADIO_NUM )
	{
		int n = 0; 
		for( RadioUnit* ru = units; ru < unit_pos; ru++ )
		{
			if ( ru->valid & VALID_ANONYMOUS )
			{
				n++;
				if ( n == cmd->int0 )
				{
					QueueWriteMsgFast( false, NCU_ACTIVATE, this, NULL, ru->zone, ru->unit, CO_CHANNEL_STATUS_INDICATOR_LED_E, PROFILE_FIRST_AID, 1, 0 );			
					QueueWriteMsgFast( false, NCU_ACTIVATE, this, NULL, ru->zone, ru->unit, CO_CHANNEL_STATUS_INDICATOR_LED_E, PROFILE_FIRST_AID, 0, 0 );			
					return CMD_OK;
				}
			}
		}

		return CMD_ERR_OUT_OF_RANGE;				
	}
	else if ( cmd->type == CMD_GET_RADIO_UNIT )
	{
		for( RadioUnit* ru = units; ru < unit_pos; ru++ )
		{
			if ( ru->unit == cmd->int0 )
			{
				cmd->int0 = (int) ru;
				return CMD_OK;
			}
		}
		cmd->int0 = (int) NULL;
		
		return CMD_OK;				
	}
	else if ( cmd->type == CMD_RESET_FAULT )
	{
		NCU::QueueWriteMsg( false, NCU_EVENT_FAULT_RESET, this, NULL, cmd->int0, cmd->int1, 2 ); 	// 2 = FAULTS ONLY
		
		Fault::RemoveAllFaultsAndWarnings( cmd->int1 );
		
		return CMD_OK;
	}
	
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn		int NCU::Receive( Message* msg )
***  \brief	Class function
**************************************************************************/

int NCU::Receive( Message* msg )
{
	switch ( msg->type )
	{
		case MSG_DAY_NIGHT:
		{			
			if ( app.panel != NULL )
			{
				QueueWriteMsg( false, NCU_BROADCAST_DAYNIGHT, this, NULL, msg->value );
			}
			break;
		}
	}		
	return MODULE_MSG_UNKNOWN;
}




/*************************************************************************/
/**  \fn		static void UpdateSiteSettings( Module* me, int succeeded )
***  \brief	Local helper function
**************************************************************************/

void NCU::UpdateSiteSettings( void )
{
	if ( app.panel )
	{

		QueueWriteMsg( true, NCU_BROADCAST_GLOBAL_SETTINGS, this, NULL, ! ( app.panel->settings & SITE_SETTING_DISABLE_OUTPUT_DELAYS ),
																						 !! ( app.panel->settings & SITE_SETTING_GLOBAL_DELAY_OVERRIDE ),
																						app.panel->delay1/30, app.panel->delay2/30, !!RealTimeClock::daytime,
																					  !! (app.panel->settings & SITE_SETTING_IGNORE_SECURITY_IN_DAY),
																						!! (app.panel->settings & SITE_SETTING_IGNORE_SECURITY_AT_NIGHT ) );		 

	 	QueueMsg( true, NCU_FW_VERSION, this, NULL, ncufw0, ncufw1, ncufw2, ncufwdd, ncufwmm, ncufwyy );
		 
		rtc->CheckDayTime( );

		//	Enable all...
		uint16_t zones[ SITE_MAX_ZONES / 16 ] = { -1, -1, -1, -1, -1, -1 };
		
		for( int zi = 0; zi < app.panel->numDefinedZones; zi++ )
		{
			// if zone disabled
			if ( app.panel->zone[ zi ].flags & ZONEFLAG_DISABLED )
			{
				// unflag it so
				int i = app.panel->zone[ zi ].num - 1;
				zones[ i / 16 ]  &= ~( 1 << ( i & 15 ) );
			}
		}		
		 
		static char buff1[ 24 ];
		static char buff2[ 24 ];
		
		snprintf( buff1, 24, "ATLOZ=%llu\r\n", (uint64_t) zones[ 0] | ((uint64_t)zones[ 1] << 16 ) | ((uint64_t)zones[ 2] << 32 ) );  
		snprintf( buff2, 24, "ATUPZ=%llu\r\n", (uint64_t) zones[ 3] | ((uint64_t)zones[ 4] << 16 ) | ((uint64_t)zones[ 5] << 32 ) );  
				
		// Enable/disable zones
		QueueMsg( true, NCU_CUSTOM_MSG, buff1, this, NULL );
		// Enable/disable zones
		QueueMsg( true, NCU_CUSTOM_MSG, buff2, this, NULL );
																									 
	}
}


/*************************************************************************/
/**  \fn		static void RxStage( Module* me, int succeeded )
***  \brief	Local helper function
**************************************************************************/

static void RxStage( Module* me, int succeeded )
{
	NCU* dis = (NCU*) me;
 
	if ( dis != NULL && succeeded )
	{	
		if ( dis->currentState == NCU_STATE_IDLE )
		{		
			for( RadioUnit* ru = dis->units; ru < dis->unit_pos; ru++ )
			{
				Device* d = dis->iostatus->FindDevice( ru->unit );
				if ( d != NULL ) dis->iostatus->RemoveDevice( d );
				ru->primary = -1;
				ru->secondary = -1;
			}
			
			dis->gsm->MeshRestartNotification( );
			dis->unit_pos = dis->units;	// clear all units;			
		}	
		else if ( dis->currentState == NCU_STATE_SYNC )
		{
		 	for( RadioUnit* ru = dis->units; ru < dis->unit_pos; ru++ )
			{
				Device* d = dis->iostatus->FindDevice( ru->unit );
				if ( d != NULL ) dis->iostatus->RemoveDevice( d );
				
			 	ru->primary = -1;
			 	ru->secondary = -1;
			}
		/*	
			// if restarting, need to pause for 10 seconds, get back into AT mode, then work back up to active mode.
			if ( dis->restartingMesh )
			{
				dis->targetState = NCU_STATE_ACTIVE;
				dis->restartingMesh = false;
			}*/
			
			dis->global_status_broadcasted = false;
		}
 
		if ( app.panel != NULL )
		{
			if ( dis->targetState == NCU_STATE_UNKNOWN ) 
			{
				if ( dis->currentState == NCU_STATE_IDLE )
				{
					dis->targetState = NCU_STATE_ACTIVE;
				}
					
				else if ( dis->currentState > NCU_STATE_SYNC )
				{
					dis->QueueWriteMsgFast( true, NCU_BROADCAST_RESET, dis, NULL ); 
					dis->QueueWriteMsg( true, NCU_GET_MESH_STATUS, dis, NULL, NCU_ALL_ZONES, NCU_ALL_UNITS );
					
					
					dis->targetState = dis->currentState;
				}
				else
				{
					dis->QueueWriteMsgFast( true, NCU_BROADCAST_MESH_RESET, dis, NULL );
					dis->targetState = dis->currentState;
				}
			}
			else
			{
				if ( dis->targetState < dis->currentState )
				{
					static short down_one;
					down_one = dis->currentState - 1;
					dis->QueueMsg( false, NCU_SET_STAGE, dis, TxStage, down_one ); 
				}
				else if ( dis->targetState > dis->currentState )
				{
					static short up_one;
					up_one = dis->currentState + 1;
					dis->QueueMsg( false, NCU_SET_STAGE, dis, TxStage, up_one ); 
				}
			}
			dis->UpdateMeshStatus( );
		}
	}
}	
	

/*************************************************************************/
/**  \fn		static void RxPlus( Module* me, int succeeded )
***  \brief	Local helper function
**************************************************************************/

static void RxPlus( Module* me, int succeeded )	
{
	NCU* dis = (NCU*) me;
	 
	if ( succeeded && dis != NULL)
	{
		if ( !dis->got_ncu_fw )
		{
			dis->QueueMsg( false, NCU_FW_VERSION, dis, NULL, dis->ncufw0, dis->ncufw1, dis->ncufw2, dis->ncufwdd, dis->ncufwmm, dis->ncufwyy );
			dis->got_ncu_fw = true;
		}

		dis->QueueMsg( false, NCU_GET_STAGE, dis, RxStage, dis->currentState ); 
	}
}	



/*************************************************************************/
/**  \fn		static void RxFireMsg( Module* me, int succeeded )
***  \brief	Local helper function
**************************************************************************/

static void RxFireMsg( Module* me, int succeeded )	
{
	NCU* dis = (NCU*) me;
	
	if ( dis != NULL )
	{
		if ( dis->Fire_Msg > 0 ) dis->Fire_Msg--;
		
		assert( me != NULL );

		if ( succeeded && app.panel != NULL )
		{
			app.DebOut( "FIRE: Unit %d chan %d state %d val %d\n", dis->fireunit, dis->firechannel, dis->firestate, dis->firevalue );
			
			Device* dev = dis->iostatus->FindDevice( dis->fireunit );
			
			if ( dev != NULL )
			{		
				for( int chan = 0 ; chan < dev->config->numInputs; chan++ )
				{
					if ( dev->config->input[ chan ].type == dis->firechannel )
					{		
						if ( dis->firestate )
						{
							dev->InputAssert( chan, now( ), dis->firevalue ); 			
						}
						else
						{
							dev->InputDeAssert( chan, now( ), dis->firevalue ); 			
						}
						break;
					}
				}
				 
			}
			RxStatus( me, succeeded );				 
		}
	}
}
	 

/*************************************************************************/
/**  \fn		static void RxAlarmMsg( Module* me, int succeeded )
***  \brief	Local helper function
**************************************************************************/

static void RxAlarmMsg( Module* me, int succeeded )
{
	NCU* dis = (NCU*) me;

	if ( dis != NULL )
	{		
		if ( dis->Alarm_Msg > 0 ) dis->Alarm_Msg--;
		
		if ( succeeded && app.panel != NULL )
		{
			app.DebOut( "ALARM: Unit %d Chan %d state %d value %d\n", dis->alarmunit, dis->alarmchannel, dis->alarmstate, dis->alarmvalue );

			Device* dev = dis->iostatus->FindDevice( dis->alarmunit );
			
			if ( dev != NULL )
			{			 
				if ( dis->alarmstate )
				{
					for( int chan = 0 ; chan < dev->config->numInputs; chan++ )
					{
						if ( dev->config->input[ chan ].type == dis->alarmchannel )
						{
							dev->InputAssert( chan, now( ), dis->alarmvalue ); 	
							break;
						}
					}
				}
				else  
				{
					for( int chan = 0 ; chan < dev->config->numInputs; chan++ )
					{
						if ( dev->config->input[ chan ].type == dis->alarmchannel )
						{
							dev->InputDeAssert( chan, now( ), dis->alarmvalue ); 	
							break;
						}
					}
				}				
			}	
			RxStatus( dis, succeeded );			 
		}
	}		 
}


/*************************************************************************/
/**  \fn		int NCU::HandleFaultFlags( )
***  \brief	Helper function 
**************************************************************************/

void HandleFaultFlags( NCU* dis, int morefaults, RadioUnit* ru )
{
	// if no faults or warnings
	if ( !( morefaults & 3 ) )
	{
		// if current faults or warnings
		if ( ru->valid & ( VALID_MORE_FAULTS | VALID_MORE_WARNINGS ) )
		{
			Fault::RemoveAllFaultsAndWarnings( ru->unit );
			ru->valid &= ~ ( VALID_MORE_FAULTS | VALID_MORE_WARNINGS );
			dis->Send( EW_UPDATE_MESH_STATUS_MSG, dis->ew, (int) &NCU::current_status );
		}
	}
	else 
	{
		// if faults
		if ( morefaults & 1 )
		{
			// if no faults currently
			if ( ! ( ru->valid & VALID_MORE_FAULTS ) )
			{
				ru->valid |= VALID_MORE_FAULTS;
				dis->Send( EW_UPDATE_MESH_STATUS_MSG, dis->ew, (int) &NCU::current_status );
			}
		}
		else //if no faults
		{
			// but current faults
			if ( ru->valid & VALID_MORE_FAULTS )
			{
				Fault::RemoveAllFaults( ru->unit );
				ru->valid &= ~VALID_MORE_FAULTS;
				dis->Send( EW_UPDATE_MESH_STATUS_MSG, dis->ew, (int) &NCU::current_status );
			}				
		}
	
		// if warnings
		if ( morefaults & 2 )
		{
			// if no warnings currently
			if ( ! ( ru->valid & VALID_MORE_WARNINGS ) )
			{
				ru->valid |= VALID_MORE_WARNINGS;
				dis->Send( EW_UPDATE_MESH_STATUS_MSG, dis->ew, (int) &NCU::current_status );
			}
		}
		else // if no warnings
		{
			// but current warnings
			if ( ru->valid & VALID_MORE_WARNINGS )
			{
				Fault::RemoveAllWarnings( ru->unit );
				ru->valid &= ~VALID_MORE_WARNINGS;
				dis->Send( EW_UPDATE_MESH_STATUS_MSG, dis->ew, (int) &NCU::current_status );		
			}
		}	
	}
}



/*************************************************************************/
/**  \fn		static void RxFaultMsg( Module* me, int succeeded )
***  \brief	Local helper function
**************************************************************************/

static void RxFaultMsg( Module* me, int succeeded )
{
	NCU* dis = (NCU*) me;

	if ( dis != NULL )
	{				
		if ( succeeded && app.panel != NULL )
		{
			DeviceConfig* dc = dis->settings->FindConfig( dis->faultunit );
			
			RadioUnit* ru = dis->FindUnit( dis->faultunit );
			
			dis->Fault_Msg--;
			
			if ( ru != NULL )
			{
				HandleFaultFlags( dis, dis->faultmore, ru );
			}
			
			// if device found and type is actual fault
			if ( dc != NULL && dis->faulttype != FAULT_SIGNAL_NO_ERROR_E )
			{	
				app.DebOut( "FAULT: Unit %d chan %d type %d value %d more %d\n", dc->zone, dis->faultunit, dis->faultchannel, dis->faulttype, dis->faultvalue, dis->faultmore );
				
				if ( dis->faultvalue )
				{			
					Fault::AddFault( (FaultType) dis->faulttype, dc->zone, dis->faultunit, dis->faultchannel );
				}
				else 
				{
					Fault::RemoveFault( (FaultType) dis->faulttype, dc->zone, dis->faultunit, dis->faultchannel );
				}
			}
		
			RxStatus( dis, succeeded );
		}
	}		 
}

 


#define LOWER_BUFFER_LEVEL 5
#define HIGHER_BUFFER_LEVEL 10
 
	

/*************************************************************************/
/**  \fn		static void RxStatus( Module* me, int succeeded )
***  \brief	Local helper function
**************************************************************************/

static void RxStatus( Module* me, int succeeded )
{
	NCU* dis = (NCU*) me;
	
	if ( succeeded && app.panel != NULL && dis != NULL )
	{	
		dis->zoneone = 1;
	
		if ( dis->Fire_Msg > 0 )
		{
			dis->QueueMsgFast( true, NCU_FIRE_MSG, dis, RxFireMsg, dis->zoneone, dis->fireunit, dis->firechannel, dis->firestate, dis->firevalue );
		}
		else if ( dis->Alarm_Msg > 0 )
		{
			// throttle at 10 per group
			if ( dis->Alarm_Msg > 10 )	
			{
				dis->Alarm_Msg = 10;
			}
			dis->QueueMsgFast( true, NCU_ALARM_MSG, dis, RxAlarmMsg, dis->zoneone, dis->alarmunit, dis->alarmchannel, dis->alarmstate, dis->alarmvalue );
		}
		else if ( dis->Fault_Msg > 0 )
		{
			// throttle at 5 per group
			if ( dis->Fault_Msg > 5 )  dis->Fault_Msg = 5;
			
			dis->QueueMsg( true, NCU_FAULT_MSG, dis, RxFaultMsg, dis->zoneone, dis->faultunit, dis->faultchannel, dis->faulttype, dis->faultvalue, dis->faultmore );
		}
		else if ( dis->Misc_Msg > 0 )
		{
			// throttle at 5 per group
			if ( dis->Misc_Msg > 5 )  dis->Misc_Msg = 5;
			
			dis->QueueMsg( true, NCU_MISC_MSG, dis, NULL );
		}
		else if ( dis->Fire_Lost > 0 || dis->Fault_Lost > 0 || dis->Misc_Lost > 0 )
		{
			app.Report( ISSUE_NCU_BUFFER_OVERFLOW );
			dis->QueueWriteMsg( false, NCU_CLEAR_QUEUE, dis, NULL, 0,1, 0,1, 0,1, 0,1, 0 );				
			dis->Fire_Lost = 0;
			dis->Fault_Lost = 0;
			dis->Misc_Lost = 0;
		}
		else if ( succeeded == MAGIC_NUM )
		{
		 	dis->QueueMsg( false, NCU_MSG_STATUS, dis, RxStatus, dis->Fire_Msg, dis->Fire_Lost, dis->Alarm_Msg, dis->Alarm_Lost, dis->Fault_Msg, dis->Fault_Lost, dis->Misc_Msg, dis->Misc_Lost, dis->Tx_Buffer );	 
		}	
	}	 
}


/*************************************************************************/
/**  \fn      void NCU::UartRxComplete( UART_HandleTypeDef* uart  )
***  \brief   Class function
**************************************************************************/

void NCU::UartCommand( UART_HandleTypeDef* uart )
{	
	if ( directing_to_USB_eek ) 
	{  
		int numbytes = uart->RxXferSize - uart->RxXferCount;
		uint8_t* data = uart->pRxBuffPtr - numbytes;
	//	data[ numbytes ] = 0;
	//	app.DebOut("NCU Rx: ");
	//	app.DebOut( (char*) data ); 
		CDC_Transmit_FS( data, numbytes );
		HAL_UART_AbortReceive( uart );
		HAL_UART_Receive_IT( uart, (uint8_t*) rxbuffer, NCU_RX_BUFFER_SIZE );
	}
	else
	{
	  MsgReceivedOK = true;
	}
}


/*************************************************************************/
/**  \fn      void NCU::UartAbort( UART_HandleTypeDef* uart  )
***  \brief   Class function
**************************************************************************/

void NCU::UartAbort( UART_HandleTypeDef* uart )
{
	NCU::finished_tx = false;
}


/*************************************************************************/
/**  \fn		int NCU::Receive( Event e )
***  \brief	Class function
**************************************************************************/

int NCU::Receive( Event e )
{ 	
	switch ( e )
	{
		case EVENT_DAY_TIME:
		{
			Send( MSG_DAY_NIGHT, this, 0 );
			break;
		}
		
		case EVENT_NIGHT_TIME:
		{
			Send( MSG_DAY_NIGHT, this, 1 );
			break;
		}
		
		case EVENT_CLEAR :
		{
			faulting = false;
			break;
		}
		
		case EVENT_RESET :
		{
			for( RadioUnit* p = this->units; p < unit_pos; p++ )
			{
				p->valid &= ~(VALID_MORE_FAULTS | VALID_MORE_WARNINGS );
			 
				if ( p->state == UNIT_DROPPED )
				{
					Fault::AddFault( FAULT_SIGNAL_DEVICE_DROPPED_E, p->zone, p->unit, 0 );
				}					
				else if ( p->state == UNIT_ACTIVE || quick_advance )
				{
					DeviceConfig* conf = settings->FindConfig( p->unit );
					
					if ( conf != NULL )
					{
						RadioDevice rd( p, conf, this ); 
						
						if ( conf->type != p->combination )
						{
							if ( p->combination != conf->type )
							{
								Fault::AddFault( FAULT_INCORRECT_DEVICE_TYPE, conf->zone, conf->unit, 0 );
							}
						}
								
						iostatus->RegisterDevice( &rd );		
					}
				}
			} 
			QueueWriteMsgFast( true, NCU_BROADCAST_RESET, this, NULL );
			
			break;
		}
		
		case EVENT_FIRE_RESET :
		{			 
			for( RadioUnit* p = this->units; p < unit_pos; p++ )
			{
				if ( p->state == UNIT_DROPPED )
				{
					 // nevermind..
				}					
				else if ( p->state == UNIT_ACTIVE || quick_advance )
				{
					DeviceConfig* conf = settings->FindConfig( p->unit );
					
					if ( conf != NULL )
					{
						RadioDevice rd( p, conf, this ); 
						
						iostatus->RegisterDevice( &rd );		
					}
				}
			} 
	 
			QueueWriteMsgFast( true, NCU_BROADCAST_RESET, this, NULL );
			
			break;
		}
		
	
		case EVENT_UPDATE :
		{
			UpdateMeshStatus( );
			break;
		}
		
	 
	
	}
	return 0;
}


/*************************************************************************/
/**  \fn		void NCU::CheckStatus( )
***  \brief	Class function
**************************************************************************/

void NCU::CheckStatus( )
{
	static int stage_seconds = 0;
	
	if ( currentState == NCU_STATE_PRE_AT || currentState == NCU_STATE_UNKNOWN )
	{	
		if ( targetState != NCU_STATE_PRE_AT )
		{
			// get into AT mode
			
			QueueMsg( false, NCU_PLUS_PLUS_PLUS, this, RxPlus );
			QueueMsg( false, NCU_PLUS_PLUS_PLUS, this, RxPlus );	// needs 2x +++ sometimes
		}
	}
	else if ( app.panel != NULL )
	{
		RxStatus( this, MAGIC_NUM );
			
		// if active check queue
		if ( stats_device != NULL && currentState == NCU_STATE_ACTIVE )
		{
			if ( stats_ready && stats_device->sent < 10 )
			{
				checkzone = 1;
				sent_device = stats_device;  
				QueueWriteMsg( false, NCU_GET_MESH_OTA_STATUS, this, NULL, checkzone, checkunit );
				stats_device->sent++;
				stats_ready = false;
			}
		}
			
		if ( currentState != targetState && !( stage_seconds & 7 ) )
		{
			QueueMsg( false, NCU_GET_STAGE, this, RxStage, currentState );
		}
		
		if ( !( stage_seconds & 63 ) )
		{
			for( RadioUnit* ru = units; ru < unit_pos; ru++ )
			{
				time_t nw = now( ); 
				
				if ( ru->state == UNIT_DROPPED )
				{
					if ( nw > ru->droptime + 60 * 32 )
					{
						ru->droptime = now( );
						QueueWriteMsg( false, NCU_GET_MESH_OTA_STATUS, this, NULL, 0, ru->unit );
						app.DebOut( "Searching for unit %d\n", ru->unit );
					}
				}
			}
		}			
		stage_seconds++;
	}
}


/*************************************************************************/
/**  \fn		static int countargs( const char* s )
***  \brief	Local helper function
**************************************************************************/

static int countargs( const char* s )
{
	if ( s != NULL )
	{			
		const char* pos = s - 2;
		int count = 0;
		
		while ( 0 != ( pos = strstr( pos + 2, "%d" ) ) ) count++;
		
		return count;
	}
	return 0;
}


/*************************************************************************/
/**  \fn		int NCU::DecodeOK( NCUQuery* q )
***  \brief	Class function
**************************************************************************/

int NCU::DecodeOK( NCUQuery* q )
{
	assert( q != NULL );
	
	if ( strstr( rxbuffer, "ERROR" ) )
	{
		return false;
	}
	
	if ( !strncmp( rxbuffer, q->fmt->reply, 4 ) )
	{
		// vsscanf assumes int pointers, so our shorts will be overflowed
		// need to use temporary ints
		
		int a[ MAX_MSG_ARGS ];		// temp ints
		int* ap[ MAX_MSG_ARGS ];	// ptrs to them
		
		for( int i = 0; i < q->rxargs; i++ ) ap[ i] = a + i;		// fill ptr array
		
		// ptr list
		va_list args;
		
		args.__ap = ap;

		int num = vsscanf( rxbuffer, q->fmt->reply, args );	// write to temporary ints
		
		// int list
		va_list args2;
		args2.__ap = a;
		
		LogMsg( q, args2 );
		
		if ( num == q->rxargs )
		{		
			for( int i = 0; i < num; i++ ) *( (short*) q->arg[ i] ) = a[ i];		// finally copy to our shorts
		
			return true;
		}
	}
	return false;
}


/*************************************************************************/
/**  \fn		void NCU::QueueWriteMsg( bool autoresend, NCUMessage msg, Module* m, NCUCallback* func, int& a, int& b, int& c, int& d, int& e, int& f,
																		int& g, int& h, int& i, int& j, int& k )
***  \brief	Class function
**************************************************************************/

int NCU::QueueWriteMsg( bool autoresend, NCUMessage msg, Module* m, NCUCallback* func, short a, short b, short c, short d, short e, short f,
																		short g, short h, short i, short j, short k )
{
	if ( formatlist[ msg ].type == WRITE )
	{  
		return QueueMsg( autoresend, msg, m, func, a, b, c, d, e, f, g, h, i, j, k );
	}
	return false;
}
   

/*************************************************************************/
/**  \fn		void NCU::QueueMsg( bool autoresend, NCUMessage msg, Module* m, NCUCallback* func, short& a, short& b, short& c, short& d, short& e, short& f,
																		short& g, short& h, short& i, short& j, short& k )
***  \brief	Class function
**************************************************************************/

int NCU::QueueMsg( bool autoresend, NCUMessage msg, Module* m, NCUCallback* func, short& a, short& b, short& c, short& d, short& e, short& f,
																		short& g, short& h, short& i, short& j, short& k )
{	
	NCUQuery* q = CreateQuery( autoresend, msg, m, func, a,  b, c, d, e, f,
																		 g,  h, i, j, k );
	
	// app.DebOut( "Queuing %s\n", q->fmt->query );  
	
	// fail to queue
	return sendqueue.Push( *q );
}


/*************************************************************************/
/**  \fn		void NCU::QueueWriteMsgFast( bool autoresend, NCUMessage msg, Module* m, NCUCallback* func, int& a, int& b, int& c, int& d, int& e, int& f,
																		int& g, int& h, int& i, int& j, int& k )
***  \brief	Class function
**************************************************************************/

int NCU::QueueWriteMsgFast( bool autoresend, NCUMessage msg, Module* m, NCUCallback* func, short a, short b, short c, short d, short e, short f,
																		short g, short h, short i, short j, short k )
{
	if ( formatlist[ msg ].type == WRITE )
	{  
		return QueueMsgFast( autoresend, msg, m, func, a, b, c, d, e, f, g, h, i, j, k );
	}
	return false;
}
 

/*************************************************************************/
/**  \fn		void NCU::QueueMsg( bool autoresend, NCUMessage msg, Module* m, NCUCallback* func, short& a, short& b, short& c, short& d, short& e, short& f,
																		short& g, short& h, short& i, short& j, short& k )
***  \brief	Class function
**************************************************************************/

int NCU::QueueMsgFast( bool autoresend, NCUMessage msg, Module* m, NCUCallback* func, short& a, short& b, short& c, short& d, short& e, short& f,
																		short& g, short& h, short& i, short& j, short& k )
{	
	NCUQuery* q = CreateQuery( autoresend, msg, m, func, a,  b, c, d, e, f,
																		 g,  h, i, j, k );
	
	// app.DebOut( "Queuing %s\n", q->fmt->query );  
	
	// fail to queue
	return sendqueue.Push( *q );
}

/*************************************************************************/
/**  \fn		void NCU::CreateQuery( bool autoresend, NCUMessage msg, Module* m, NCUCallback* func, short& a, short& b, short& c, short& d, short& e, short& f,
																		short& g, short& h, short& i, short& j, short& k )
***  \brief	Class function
**************************************************************************/
																			
NCUQuery* NCU::CreateQuery( bool autoresend, NCUMessage msg, Module* m, NCUCallback* func, short& a, short& b, short& c, short& d, short& e, short& f,
																		short& g, short& h, short& i, short& j, short& k ) 
{
	static NCUQuery q;
	
	q.fmt  = formatlist + (unsigned int)msg;
	q.func =	func;
	q.user = m;
	q.resend = autoresend;
	q.retries = 3;
	
	assert( q.fmt->msg == msg );
	assert( q.fmt->query != NULL );

	int query_args = 0;
	int reply_args = 0; 
		
	if ( q.fmt->query != NULL ) query_args = countargs( q.fmt->query );	
	if ( q.fmt->reply != NULL ) reply_args = countargs( q.fmt->reply );
	
	q.txargs = query_args;
	q.rxargs = reply_args;
	
	// If write use actual values
	
	if ( q.fmt->type == WRITE )
	{
		q.arg[ 0] = a;
		q.arg[ 1] = b;
		q.arg[ 2] = c;
		q.arg[ 3] = d;
		q.arg[ 4] = e;
		q.arg[ 5] = f;
		q.arg[ 6] = g;
		q.arg[ 7] = h;
		q.arg[ 8] = i;
		q.arg[ 9] = j;
		q.arg[10] = k;
	}
	else // if read then take address of params
	{
		q.arg[ 0] = (int) &a;
		q.arg[ 1] = (int) &b;
		q.arg[ 2] = (int) &c;
		q.arg[ 3] = (int) &d;
		q.arg[ 4] = (int) &e;
		q.arg[ 5] = (int) &f;
		q.arg[ 6] = (int) &g;
		q.arg[ 7] = (int) &h;
		q.arg[ 8] = (int) &i;
		q.arg[ 9] = (int) &j;
		q.arg[10] = (int) &k;
	}
	
	return &q;
}

 


/*************************************************************************/
/**  \fn		int NCU::QueueMsg( bool autoresend, NCUMessage msg, char* cmsg, Module* m, void (*func)( char* ))
***  \brief	Class function
**************************************************************************/

int NCU::QueueMsg( bool autoresend, NCUMessage msg, char* cmsg, Module* m, void (*func)( char* ))
{	
	NCUQuery q;
	
	q.cmsg = cmsg;
	q.fmt  = formatlist + (unsigned int)msg;
	q.func =	(NCUCallback*) func;
	q.user = m;
	q.resend = autoresend;
	q.retries = 3;

	int query_args = 0;
	int reply_args = 0; 
		
	q.txargs = query_args;
	q.rxargs = reply_args;
	
	
	// app.DebOut( "Queuing %s\n", q.cmsg );  
	
	// fail to queue
	return sendqueue.Push( q );
}




/*************************************************************************/
/**  \fn		void NCU::Execute( NCUQuery* q )
***  \brief	Class function
**************************************************************************/

void NCU::Execute( NCUQuery* q )
{
	int arg[ MAX_MSG_ARGS ];
	
	assert( q != NULL );
		  
	for( int n = 0; n < q->txargs; n++ )
	{  
		if ( q->fmt->type == WRITE )
		{
			arg[ n] = q->arg[ n];
		}
		else
		{
			arg[ n] = *( (short*)q->arg[ n] );
		}
	}	

	va_list args;
	
	args.__ap = (void*) arg;

	if ( q->fmt->type == CUSTOM )
	{
		snprintf( txbuffer, 64, q->cmsg );
	}
	else
	{
		vsnprintf( txbuffer, 64, q->fmt->query, args );
	}
	
	// Cancel previous transactions
		
	if ( strncmp( txbuffer + 2, "MSN", 3 ) )
	{
		app.DebOut( "\n%d Tx: %s", HAL_GetTick( ), txbuffer );
	}
 
	finished_tx = false;
	memset( rxbuffer, 0, NCU_RX_BUFFER_SIZE );
	 
	HAL_UART_AbortTransmit( app.ncu_uart );
	HAL_UART_AbortReceive( app.ncu_uart );
	
   HAL_UART_Transmit_DMA( app.ncu_uart, (uint8_t*) txbuffer, strlen( txbuffer ) );
	HAL_UART_Receive_DMA( app.ncu_uart, (uint8_t*) rxbuffer, NCU_RX_BUFFER_SIZE );
	
	
/*	
	HAL_UART_AbortReceive( app.ncu_uart );
	HAL_UART_Transmit_IT( app.ncu_uart, (uint8_t*) buff, strlen( buff ) );
	MsgReceivedOK = false;	*/
	
	rx_fail_count = 0;
}


volatile int NCU::finished_tx = 1;

/*************************************************************************/
/**  \fn		void NCU::UartTxComplete( UART_HandleTypeDef *huart )
***  \brief	Global helper function
**************************************************************************/

void NCU::UartTxComplete( UART_HandleTypeDef *huart )
{
	if ( huart == app.ncu_uart )
	{
		NCU::finished_tx = true;
#if 0		
		  /* Disable the UART Parity Error Interrupt and RXNE interrupt*/
		CLEAR_BIT(app.ncu_uart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));

		/* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
		CLEAR_BIT(app.ncu_uart->Instance->CR3, USART_CR3_EIE);

		/* Rx process is completed, restore huart->RxState to Ready */
		app.ncu_uart->RxState = HAL_UART_STATE_READY;

		memset( rxbuffer, 0, NCU_RX_BUFFER_SIZE );
		HAL_UART_AbortReceive( app.ncu_uart );
		HAL_UART_Receive_DMA( app.ncu_uart, (uint8_t*) rxbuffer, NCU_RX_BUFFER_SIZE );
#endif
	}
}


/*************************************************************************/
/**  \fn		RadioUnit* NCU::FindUnit( int unit )
***  \brief	Class function
**************************************************************************/

RadioUnit* NCU::FindUnit( int unit )
{
	for( RadioUnit* p = units; p < unit_pos; p++ )
	{
		if ( p->unit == unit ) return p;
	}
	return NULL;
}



/*************************************************************************/
/**  \fn		RadioUnit* NCU::NumPrimaryChildren( int unit )
***  \brief	Class function
**************************************************************************/

int NCU::NumPrimaryChildren( int unit )
{
	int children = 0;
	
	for( RadioUnit* p = units; p < unit_pos; p++ )
	{
		if ( p->primary == unit )
		{
			children++;
		}
	}
	return children;
}


/*************************************************************************/
/**  \fn		RadioUnit* NCU::NumSecondaryChildren( int unit )
***  \brief	Class function
**************************************************************************/

int NCU::NumSecondaryChildren( int unit )
{
	int children = 0;
	
	for( RadioUnit* p = units; p < unit_pos; p++ )
	{
		if ( p->secondary == unit )
		{
			children++;
		}
	}
	return children;
}


/*************************************************************************/
/**  \fn		void GetSerial( )
***  \brief	Helper function
**************************************************************************/

void GetSerial( Module* me, int succeeded )
{
	if ( succeeded )
	{
		NCU* dis = (NCU*) me;
		RadioUnit* ru = dis->FindUnit( dis->serialunit );
	
		if ( ru != NULL )
		{
			snprintf( ru->serial, 14, "%02d%02d-%02d-%05d", dis->serialyy, dis->serialmm, dis->serialbb, dis->serialss );
		}
		dis->UpdateMeshStatus( );
	}
}



/*************************************************************************/
/**  \fn		void GetCombo( )
***  \brief	Helper function
**************************************************************************/

void GetCombo( Module* me, int succeeded )
{
	if ( succeeded )
	{
		NCU* dis = (NCU*) me;
		RadioUnit* ru = dis->FindUnit( dis->combunit );
	
		if ( ru != NULL )
		{
			ru->combination = dis->combvalue;		
		}
		dis->UpdateMeshStatus( );
	}
}


/*************************************************************************/
/**  \fn		void GetNeighbour( )
***  \brief	Helper function
**************************************************************************/

void GetNeighbour( Module* me, int succeeded )
{
	NCU* dis = (NCU*) me;
	
	if ( dis->meshprimary == -1 && dis->meshsecondary == -1 )
	{
		return;
	}
	
	RadioUnit* ru = dis->FindUnit( dis->meshunit );
	
	if ( ru != NULL )
	{
		ru->primary 	  = dis->meshprimary;
		ru->rssiprimary   = dis->meshrssiprimary;
		ru->secondary    = dis->meshsecondary;
		ru->rssisecondary = dis->meshrssisecondary;
	}
	dis->UpdateMeshStatus( );
}


/*************************************************************************/
/**  \fn		void NCU::UpdateMeshStatus( )
***  \brief	Helper function
**************************************************************************/

void NCU::UpdateMeshStatus( )	
{
	time_to_update_mesh = now() + 3;
}


/*************************************************************************/
/**  \fn		void NCU::UpdateMeshStatusNow( )
***  \brief	Helper function
**************************************************************************/

void NCU::UpdateMeshStatusNow( )	
{
	NCUMeshStatus& s = current_status;
	
	if ( app.panel != NULL )
	{
		s.extra = 0;
		s.active = 0;
		s.meshed = 0;
		
		s.total = app.panel->numRadioDevs;
	
		for( RadioUnit* ru = units; ru < unit_pos; ru++ )
		{
			if ( ru->state != UNIT_DROPPED && ru->primary != -1 )
			{
				if ( ru->state >= UNIT_MESHED )
				{
					s.meshed++; 
				}
				if ( ru->state >= UNIT_ACTIVE )  
				{
					s.active++;
				}
			}
			if ( ru->valid & VALID_ANONYMOUS ) s.extra++;
		}

		if ( currentState == NCU_STATE_SYNC )
		{
			if ( quick_advance  )   
			{
				targetState = NCU_STATE_FORMATION;		 
			}
		}
	
		if ( currentState == NCU_STATE_FORMATION && quick_advance ) 
		{
			targetState = NCU_STATE_ACTIVE;
			 
		}
		
		if ( currentState == NCU_STATE_ACTIVE )
		{
			if ( !global_status_broadcasted )
			{
				global_status_broadcasted = true;
				
				UpdateSiteSettings( );				
			}
		}

		Send( EW_UPDATE_MESH_STATUS_MSG, ew, (int) &s );	
	}
}


void NCU::GenMesh( )
{
	unit_pos = units;	
	Panel* p = settings->GetCurrentPanel();
	for( DeviceConfig* dc = p->devs; dc < p->devs + p->NumDevs(); dc++ )
	{
		if ( dc->type < 64 )
		{			
			*unit_pos = RadioUnit( dc->zone, dc->unit, 0,0,0,0, 255 );
	
			unit_pos->primary = rand() % dc->unit;
			unit_pos->rssiprimary = rand() % 30;
			unit_pos->state = UNIT_ACTIVE;
			unit_pos->combination = dc->type;
					
			unit_pos++;		
		}
	}
}


/*************************************************************************/
/**  \fn		void NCU::HandleMisc( NCUMessage i )
***  \brief	Class function
**************************************************************************/

void NCU::HandleMisc( NCUMessage i )
{
	const char* fmt = formatlist[ i].reply;
	
	// If returns error exit
	if ( strstr( fmt, "ERROR" ) ) 
	{
		app.DebOut( "ERROR..\n" );
		return;
	}	 
	
	// Get new data
	int battprimary, battbackup, _meshzone, unit, combination, zone, heat, smoke, pir;
	 	
 	int params = sscanf( rxbuffer, fmt, &_meshzone, &unit, &battprimary, &battbackup, &combination, &zone, &smoke, &heat, &pir );
	
	if ( params == 9 )	// ensure correct number of parameters
	{
		// Output to mesh log ( as recevied )
		LogMsg( MESHLOG_STATUS, formatlist[ i].friendly, unit, battprimary, battbackup, combination, zone, smoke, heat, pir );
		
		app.DebOut( "MISC STATUS: Unit %d batt %d combo %d zone %d...\n", unit, battprimary, combination, zone );
		
		// Get device
		Device* d = iostatus->FindDevice( unit );
		
		// if found
		if ( d != NULL )
		{
			// Get config
			DeviceConfig* dc = d->config;

			// if config found
			if ( dc != NULL )
			{
				// for each channel
				for( InputChannel* ic = dc->input; ic < dc->input + dc->numInputs; ic++ )
				{
					// i = index
					int i = ic - dc->input;
					
					// if have a heat value
					if ( heat != 0 )
					{
						// and channel is heat
						if ( ic->type == CO_CHANNEL_HEAT_B_E )
						{
							// update value
							d->analogue[ i ] = heat;
						}
						else if ( ic->type == CO_CHANNEL_HEAT_A1R_E )
						{
							d->analogue[ i] = heat;
						}
					}
					// if have a smoke value
					if ( smoke != 0 )
					{
						// and channel is smoke
						if ( ic->type == CO_CHANNEL_SMOKE_E )
						{
							// update value
							d->analogue[ i ] = smoke;
						}
					}
				}
			}
		}	
		// Update radio settings
		Status( zone, unit, battprimary, battbackup, combination );
			
		AppMessage msg( unit, true );
		
		MQTTSupport::Send( msg );
	}
}


/*************************************************************************/
/**  \fn		void NCU::Status( int zone, int unit, int battprimary, int battbackup, int combination )
***  \brief	Class function
**************************************************************************/

RadioUnit* NCU::Status( int zone, int unit, int battprimary, int battbackup, int combination )
{
	// Is unit addres ok?
	if ( unit <= SITE_MAX_RADIO_DEVICES && unit > 0 && app.panel != NULL )
	{		
		static char extra[ (SITE_MAX_RADIO_DEVICES + 8) >> 3 ] = { 0 };
				
		DeviceConfig* dc = settings->FindConfig( unit );	
		
		int index = unit >> 3;
		int bit = 1 << ( unit & 7 );
		
		if ( dc != NULL )	// If valid device found
		{
			if ( combination == 0 )	// if unknown 
			{
				combination = dc->type;		// set to configured type
			}
			else 
			{
				if ( combination != dc->type )	// if doesnt match configured type 
				{
					Fault::AddFault( FAULT_INCORRECT_DEVICE_TYPE, dc->zone, dc->unit, 0 );
				}
			}
			
			if ( zone == -1 ) zone = dc->zone;	// if unknown set to configured zone
			
			if ( extra[ index ] & bit )	// if reported unknown fault, remove it
			{
				extra[ index ] &= ~bit;
				Fault::RemoveFault( FAULT_UNKNOWN_UNIT, 255, unit, 0 );
			}		
		}
		else // unknown unit - generate fault
		{
			Fault::AddFault( FAULT_UNKNOWN_UNIT, 255, unit, 0 );		
			
			
			extra[ index ] |= bit;				
		}
		
		// Do we have this RBU?
		RadioUnit* p = FindUnit( unit );
		
		// If RBU unknown
		if ( p == NULL )
		{		
			if ( unit_pos < ENDOF( units ) )
			{
				// Create new Unit object
				*unit_pos = RadioUnit( zone, unit, 0,0,0,0, combination );
				
				// inc RBU  list
				p = unit_pos++;
				
				// timestamp it
				p->starttime = now( );	
				
				p->valid |= dc == NULL ? VALID_ANONYMOUS : 0;
				
				// if maybe dropped
				if ( dc != NULL ) 
				{	
					p->zone = dc->zone;
					// remove fault
					Fault::RemoveFault( FAULT_SIGNAL_DEVICE_DROPPED_E, dc->zone, unit, 0 );	
				}	
			}
		}
		
		if ( p != NULL )
		{
			if ( zone != -1 && zone != 0 ) p->zone = zone;
			if ( combination != 0 ) p->combination = combination;
				
			 
			// set as active
			p->state = UNIT_ACTIVE;
						 
			// Do we have a device for this RBU?
			RadioDevice* dev = (RadioDevice*) iostatus->FindDevice( p );
			
			// no?
			if ( dev == NULL )
			{		
				// if unit in site
				if ( dc != NULL )
				{
					// Create new device for RBU
					RadioDevice rd( p, dc, this );
							
					// reigister it
					dev = (RadioDevice*) iostatus->RegisterDevice( &rd );		
					
					// set battery level
					if ( dev != NULL )
					{
						if ( battprimary != 0 )
						{	
							if ( battprimary == 1 )
							{
								p->battprimary = 0;
							}
							else
							{
								p->battprimary = battprimary;
							}
							if ( battbackup == 1 )
							{
								p->battbackup = 0;
							}
							else
							{
								p->battbackup = battbackup;
							}
							 
							dev->SetBattery( battprimary );
						}
					}
				}
				else
				{
					p->valid |= VALID_ANONYMOUS;
				}
				
				// Update GUI
				UpdateMeshStatus( );
			}
			else
			{
				if ( battprimary != 0 )
				{	
					p->battprimary = battprimary;
					p->battbackup = battbackup;
					dev->SetBattery( battprimary );
				}
			
	   		AppDeviceClass_TriggerMeshUnitChangeEvent( app.DeviceObject, p->unit );
			}
					
			return p;
		}
	}
	return NULL;
}


/*************************************************************************/
/**  \fn		int NCU::HandleStatus( )
***  \brief	Class function
**************************************************************************/

int NCU::HandleStatus( NCUMessage i )
{
	const char* fmt = formatlist[ i].reply;
	
	if ( strstr( rxbuffer, "ERROR" ) ) 
	{
		app.DebOut( "ERROR..\n" );
		return -1;
	}	
	
	// scanf needs all ints
 
	int  combination, eventdata, battprimary, battbackup, _meshzone, _meshunit,  parents, children, nodes, childnodes, rank, rssiprimary, rssisecondary, morefaults;
	 	
 	int params = sscanf( rxbuffer, fmt, &_meshzone, &_meshunit, &battprimary, &battbackup, &meshprimary, &meshsecondary, 
												&rssiprimary, &rssisecondary, &status_event, &eventdata, &rank, &combination, &morefaults );
	
	LogMsg( MESHLOG_STATUS, formatlist[ i].friendly, _meshunit, meshprimary, meshsecondary, rssiprimary, rssisecondary, battprimary, rank, combination, morefaults );
 	
	if ( params == 13 )
 	{
		// Not interested in NCU
		if ( _meshunit == 0 ) return 0;
		
		if ( status_event == EVENT_PRIMARY_PARENT_DROPPED ) return 0;
		
		LinkParent( _meshunit, meshprimary, true );
					
		 
		RadioUnit* ru = Status( _meshzone, _meshunit, battprimary, battbackup, combination );
		
		if ( ru != NULL )
		{
			if ( status_event != EVENT_SECONDARY_PARENT_DROPPED )
			{
				ru->secondary = meshsecondary;
			}
			else
			{
				ru->secondary = -1;
			}
			
			ru->SetStatus( parents, children, rssiprimary, rssisecondary, nodes, childnodes, rank );
				
			HandleFaultFlags( (NCU*) this, morefaults, ru );
			
			ru->lastinfo = now( );
		}	
		
		AppMessage msg( _meshunit, true );
		
		MQTTSupport::Send( msg );
	 
		return _meshunit;
	}
	else
	{
		return -1;
	}	
}

	
		
/*************************************************************************/
/**  \fn		void NCU::HasCircularDependance( int unit )
***  \brief	Class function
**************************************************************************/

int NCU::HasCircularDependance( int unit )
{
	RadioUnit* r = FindUnit( unit );
	
	int count = 0;
	
	while ( r != NULL && r->primary != -1 )
	{
		if ( r->primary == r->unit ) return true;
		if ( r->primary == 0 ) return false;
		r = FindUnit( r->primary );		
		if ( ++count > NCU_MAX_RANKS ) return true;
	}
	return false;
}


/*************************************************************************/
/**  \fn		void NCU::CheckAllForCircularDependance( )
***  \brief	Class function
**************************************************************************/

int NCU::HasAnyCircularDependance( )
{
	for( RadioUnit* ru = units; ru < unit_pos; ru++ )
	{
		if ( HasCircularDependance( ru->unit ) ) 
		{
			return true;
		}
	}
	return false;
}


/*************************************************************************/
/**  \fn		void NCU::OnTree( int unit )
***  \brief	Class function
**************************************************************************/

int NCU::OnTree( int unit )
{
	if ( HasCircularDependance( unit ) ) return false;
		
	RadioUnit* r = FindUnit( unit );
	
	while ( r != NULL && r->primary != -1 )
	{
		if ( r->primary == 0 ) return true;
		r = FindUnit( r->primary );
	}
	return false;
}


/*************************************************************************/
/**  \fn		void NCU::LinkParent( int child, int parent, bool definite )
***  \brief	Class function
**************************************************************************/

void NCU::LinkParent( int child, int parent, bool definite )
{
	if ( ( parent & 4095 ) == 4095 ) 
	{
		parent = -1;
	}
	
	int prevparent = -1;
	
	// Find radio unit
	RadioUnit* p = FindUnit( child );
	
	if ( p != NULL )
	{
		prevparent = p->primary;
	}
	
	// Find config
	DeviceConfig* conf = settings->FindConfig( child );
			
	// if no unit found
	if ( p == NULL && unit_pos < ENDOF( units ) )
	{
		// if have config
		if ( conf != NULL )
		{
			// create new unit
			*unit_pos = RadioUnit( conf->zone, child, 0,0,0,0, conf->type );
			
			// remove fault
			Fault::RemoveFault( FAULT_SIGNAL_DEVICE_DROPPED_E, conf->zone, child, 0 );	
		}
		else
		{
			// create anonymous unit
			*unit_pos = RadioUnit( 0, child, 0,0,0,0, 0 );	
			unit_pos->valid |= VALID_ANONYMOUS;
		}
		p = unit_pos;
	
		p->starttime = now( );
		 
		unit_pos++;	
	}
	else
	{
		// if RBU previously dropped
		if ( p->state == UNIT_DROPPED && conf != NULL ) 
		{	
			// remove fault
			Fault::RemoveFault( FAULT_SIGNAL_DEVICE_DROPPED_E, conf->zone, child, 0 );	
		}	
	}	
	
	if ( definite || ! ( p->valid & VALID_PRIMARY ) )
	{
		p->primary = parent;
	}
	
	if ( definite ) p->valid |= VALID_PRIMARY;
	
	if ( p->state != UNIT_ACTIVE ) p->state = UNIT_MESHED;		
	
	if ( prevparent != parent )
	{		
	
		gsm->DeviceNotification( child, parent, true, current_status.meshed );
		LogMsg( MESHLOG_LINK, "Link parent %d to child %d\n", parent, child );
		UpdateMeshStatus( );
	}
}


/*************************************************************************/
/**  \fn		void NCU::HandleMesh( )
***  \brief	Class function
**************************************************************************/

void NCU::HandleMesh( )
{
	int parent, child, event;
	
	const NCUMsgFormat* fmt = formatlist + NCU_MESH;
	
	int pams = sscanf( rxbuffer, fmt->reply, &event, &parent, &child );
	
	if ( pams != 3 ) return;
	
	 // link to new parent
	if ( event == 0 )
	{
		LinkParent( child, parent, false );	
	}
	
	else if ( event == 2 )
	{
		DropUnit( parent );
	}
}


/*************************************************************************/
/**  \fn		void NCU::HandleMesh( )
***  \brief	Class function
**************************************************************************/

void NCU::DropUnit( int unit )
{
	RadioUnit* lostdev = FindUnit( unit );
	
	if ( lostdev != NULL )
	{	
		// if not already dropped
		if ( lostdev->state != UNIT_DROPPED )
		{
			LogMsg( MESHLOG_DROP, "Dropped unit %d\n", unit );
			
			AppMessage msg( unit, false );
						
#ifdef GSM_HTTP			
			AppSupport::Send( msg );
#else
			MQTTSupport::Send( msg );
#endif	
			lostdev->lastinfo = now( );
			
			DeviceConfig* dc = settings->FindConfig( lostdev->unit );
			
			if ( dc != NULL )
			{
				Fault::AddFault( FAULT_SIGNAL_DEVICE_DROPPED_E, dc->zone, lostdev->unit, 0 );	
				
				Device* d = iostatus->FindDevice( lostdev->unit );
				
				// Check is not active before removal..
				if ( d != NULL )
				{
					int ch;
					for( ch = 0; ch < dc->numInputs; ch++ )
					{
						// if input active
						if ( d->flags[ ch ] & INPUT_ACTIVE )
						{
							// dont remove
							break;
						}
					}
					if ( ch == dc->numInputs )
					{
						iostatus->RemoveDevice( d );
					}
				}
			}
			
			lostdev->droptime = now( );
			lostdev->state = UNIT_DROPPED;
			lostdev->primary = -1;
			lostdev->rssiprimary = 0;
			
			UpdateMeshStatus( );
			
			gsm->DeviceNotification( lostdev->unit, -1, false, current_status.meshed ); 		
			lostdev->valid &= ~VALID_NOTIFIED_LINKED;
		}
	}
}


/*************************************************************************/
/**  \fn      void NCU::TimerPeriodElapsed( TIM_HandleTypeDef *htim )
***  \brief   Global helper function
**************************************************************************/

void NCU::TimerPeriodElapsed( TIM_HandleTypeDef *htim )
{
	time_to_check = true;
}
		
 
/*************************************************************************/
/**  \fn		void NCU::Poll( )
***  \brief	Class function
**************************************************************************/

void NCU::Poll( )
{
	if ( !directing_to_USB_eek )
	{
		time_t _now = now();
		
		if ( _now >= time_to_update_mesh )
		{
			time_to_update_mesh = _now + 60 * 60 * 6;
			UpdateMeshStatusNow( );
		}
			
		// Are we waiting on a reply?
		if ( msgAwaiting != NULL )
		{
			char* lf = strchr( rxbuffer, 13 );
			// If msg received?
			if ( lf != NULL )
			{
				lf[1] = 0;
				fails = 0;
				rx_fail_count = 0;
				lastRxTime = _now;
				
				if ( faulting )
				{
					Fault::RemoveFault( FAULT_NO_NCU_COMMS );
					faulting = false;
				}			
			 
		 	//	app.DebOut( "%d Rx: %s", HAL_GetTick(), rxbuffer );
				
				const NCUMsgFormat* fmt = msgAwaiting->fmt;
				 
				switch ( msgAwaiting->fmt->type )
				{
					case PLUSOK : 
					{
						if ( msgAwaiting->func != NULL )
						{
							msgAwaiting->func( msgAwaiting->user, !strncmp( rxbuffer, "OK", 2 ) );
						}
					}
					break;
					
					case CUSTOM:
					{
						if ( msgAwaiting->func != NULL )
						{
							(( void (*)( char* ) )msgAwaiting->func)( rxbuffer );
						}
						break;
					}
					
					case WRITE :
					{
						if ( msgAwaiting->func != NULL )
						{
							int success = false;
							
							if ( strstr( rxbuffer, "OK" ) )
							{
								int chars = 3;
								if ( rxbuffer[ 2] == ':' ) chars = 2;
								success = !strncmp( rxbuffer, fmt->query + 2, chars );
							}					 
							
							msgAwaiting->func( msgAwaiting->user, success );
						}
					}
					break;
					
					case MISC :
					{
						app.DebOut( "Rx: " );
						app.DebOut( rxbuffer );
						
						if ( Misc_Msg > 0 ) Misc_Msg--;
						
						if ( app.panel != NULL )
						{
							if ( !strncmp( rxbuffer, formatlist[ NCU_MISC ].reply, 4 ) )
							{
								HandleMisc( NCU_MISC );
							}	
							else if ( !strncmp( rxbuffer, formatlist[ NCU_STATUS ].reply, 4 ) )
							{
								HandleStatus( NCU_STATUS );
							}	
							else if ( !strncmp( rxbuffer, formatlist[ NCU_GET_MESH_STATUS ].reply, 4 ) )
							{
								int unit = HandleStatus( NCU_GET_MESH_STATUS );
								 
								if ( stats_device != NULL )
								{
									if ( stats_device->unit == unit )
									{
										stats_device->received++;
									}
									stats_ready = true;
								}
							}
							else if ( !strncmp( rxbuffer, formatlist[ NCU_MESH ].reply, 4 ) )
							{
								HandleMesh( );
							}											
							else if ( ! delayedqueue.Empty( ) )
							{
								app.DebOut( "Delayed q size:%d\n", delayedqueue.items );
								
								int n;
								for( n = 0; n < delayedqueue.items; n++ )
								{
									app.DebOut( "%4s", delayedqueue[ n].fmt->reply );
									
									if ( !strncmp( rxbuffer, delayedqueue[ n].fmt->reply, 3 ) )
									{
										int ok = DecodeOK( & (delayedqueue[ n]) );
										
										app.DebOut( " = %s %s\n", rxbuffer, ok ? "OK":"FAIL" );
										
										if ( delayedqueue[ n].func != NULL )
										{
											 delayedqueue[ n].func( delayedqueue[ n].user, ok );
										}
										if ( !ok && delayedqueue[ n].resend )
										{
											if ( delayedqueue[ n].retries-- > 0 )
											{
												// Requeue !
												sendqueue.Push( delayedqueue[ n] );
											}
											else
											{
												app.DebOut( "Give up! no more retrys.\n" );
											}
										}
										delayedqueue.Remove( & (delayedqueue[ n]) );
										break;
									}
									else
									{
										app.DebOut( "!= %s\n", rxbuffer );
									}
								}
								if ( n == delayedqueue.items )
								{
									app.DebOut( "Failed to match delayed item.\n" );
								}
							}
						}
						RxStatus( this, true );
					}
					break;
					
					case READ :
					{
						if ( (*(int*)rxbuffer) != ':NSM' )
						{
							app.DebOut( "Rx: " );
							app.DebOut( rxbuffer );
						}
						
						int ok = DecodeOK( msgAwaiting );
						
						if ( msgAwaiting->func != NULL )
						{
							msgAwaiting->func( msgAwaiting->user, ok );
						}
						if ( !ok && msgAwaiting->resend && msgAwaiting->retries-- > 0 )
						{
							// Re-Queue
							sendqueue.Push( *msgAwaiting );
						}
					}
					break;			
				}			
				msgAwaiting = NULL;
			}
		}
		
		//---------------------------
#if 0
		if ( (rand()&15) == 3 )
		{
			int unit = (rand()>> (rand()&23)) & 63;
			
			unit++;
			
			RadioUnit* u = FindUnit( unit );
			
			if ( u != NULL )
			{
				Device* d = iostatus->FindDevice( unit );
				
				DeviceConfig* dc = settings->FindConfig( unit );
				
				if ( d != NULL )
				{
					DropUnit( unit );
				}
				else
				{
					if ( dc != NULL )
					{
						Status( dc->zone, unit, 500, 4000, dc->type );
					}
					else
					{
						Status( -1, unit, 500, 4000, 40 );
					}
					Fault::AddFault( FAULT_SIGNAL_TYPE_MISMATCH_E,      0, unit, 1 );             
					Fault::AddFault( FAULT_SIGNAL_DEVICE_ID_MISMATCH_E, 0, unit, 1 );
					Fault::AddFault( FAULT_SIGNAL_FAULTY_SENSOR_E,      1, unit, 1 );
					 
				}
			}
			else
			{
				LinkParent( unit, 0, true );
				Fault::RemoveAllFaults( unit );
			}
		}
#endif			
		//------------------------
		
		if ( msgAwaiting != NULL )
		{
			if ( ++rx_fail_count > NCU_MAX_FAILS )
			{
				app.DebOut( "[T]\n" );	// timeout
				
				rx_fail_count = 0;
				
				// get back in to AT mode..
				if ( fails++ > 5 )
				{
					currentState = NCU_STATE_PRE_AT;	// go back to +++
					
					fails = 0;
				}
							
				if ( !faulting )
				{
					// report NCU fault before 100 seconds
					if ( lastRxTime + 90 < _now )
					{
						Fault::AddFault( FAULT_NO_NCU_COMMS );
						((int*)BKPSRAM_BASE)[ BATT_RAM_NCU_FAULT] = 0x55555555;
						
						HAL_UART_AbortReceive( app.ncu_uart );
						
						faulting = true;
					}
				}
				
				// if no resend or out of retrys, give up
				if ( !msgAwaiting->resend || --msgAwaiting->retries <= 0 )
				{				 
					// if call back func
					if ( msgAwaiting->func != NULL ) 
					{
						// if not custom
						if ( msgAwaiting->fmt->type != CUSTOM )
						{
							msgAwaiting->func( msgAwaiting->user, false );			
						}
					}
					msgAwaiting = NULL;  					 
				}
				else
				{
					Execute( msgAwaiting );
				}
			}
		}
		
		if ( msgAwaiting == NULL )
		{		
			if ( sendqueue.items < 4 && time_to_check  )
			{
				CheckStatus( );
				time_to_check = false;
			}	
				
			if ( finished_tx )
			{
				if ( ! fastqueue.Empty( ) )
				{
					msgAwaiting = fastqueue.Pop( );
			//		app.DebOut( "Fast Q sz:%d\n", fastqueue.items );
					 
					if ( msgAwaiting != NULL ) 
					{
						Execute( msgAwaiting );
					}
				}
				else if ( ! sendqueue.Empty( ) )
				{
					msgAwaiting = sendqueue.Pop( );
			//		app.DebOut( "Send Q sz:%d\n", sendqueue.items );
					 
					if ( msgAwaiting != NULL ) 
					{
						Execute( msgAwaiting );
											
						if ( msgAwaiting->fmt->type == DELAYED )
						{
							delayedqueue.Push( *msgAwaiting );
						}
					}
				}
			}
		}
		
		if ( reset_rx_request )
		{
			msgAwaiting = NULL;
			finished_tx = true;
			reset_rx_request = false;
		}
	}
}


/*************************************************************************/
/**  \fn      Log::Msg( LogType lt, char* fmt, ... )
***  \brief   Constructor for class
**************************************************************************/

void NCU::LogMsg( short lt, const char* fmt, ... )
{
	if ( lt != -1 && fmt != NULL)
	{
		va_list args;
				
		va_start( args, fmt );
		
		logger.Msg( lt, fmt, args );
		
		va_end( args );	
	}
}

/*************************************************************************/
/**  \fn      Log::Msg( LogType lt, char* fmt, ... )
***  \brief   Constructor for class
**************************************************************************/

void NCU::LogMsg( NCUQuery* q, va_list args )
{
	if ( q->fmt->logtype != -1 && q->fmt->friendly != NULL )
	{
		logger.Msg( q->fmt->logtype, q->fmt->friendly, args );		
	}
}

#if 0 
/*************************************************************************/
/**  \fn      void NCU::TimerPeriodElapsed( TIM_HandleTypeDef *htim )
***  \brief   Global helper function
**************************************************************************/
 
void NCU::RxISR( UART_HandleTypeDef *huart )
{
  uint16_t uhMask = huart->Mask;
  uint16_t  uhdata;

  /* Check that a Rx process is ongoing */
  if (huart->RxState == HAL_UART_STATE_BUSY_RX)
  {
	   if ( directing_to_USB_eek ) 
		{ 
			uint8_t byte = huart->Instance->RDR;
			
			*usbbuffptr++ = byte;
			usbcount--;
			
			if ( byte != '\n' )
			{				
				if ( usbcount == 0 )  
				{
					CDC_Transmit_FS( usbbuff, USB_BUFF_SIZE );
					usbbuffptr = usbbuff;
					usbcount = USB_BUFF_SIZE;
				}
			}
			else
			{
				CDC_Transmit_FS( usbbuff, USB_BUFF_SIZE - usbcount );
				usbbuffptr = usbbuff;
				usbcount = USB_BUFF_SIZE;
			}
		}
		else
		{ 
		  
		 uhdata = (uint16_t) READ_REG(huart->Instance->RDR);
		 *huart->pRxBuffPtr = (uint8_t)(uhdata & (uint8_t)uhMask);
		 huart->pRxBuffPtr++;
		 huart->RxXferCount--;

		 if (huart->RxXferCount == 0U)
		 {
			/* Disable the UART Parity Error Interrupt and RXNE interrupts */
			CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));

			/* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
			CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

			/* Rx process is completed, restore huart->RxState to Ready */
			huart->RxState = HAL_UART_STATE_READY;

			/* Clear RxISR function pointer */
			huart->RxISR = NULL;

	#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
			/*Call registered Rx complete callback*/
			huart->RxCpltCallback(huart);
	#else
			/*Call legacy weak Rx complete callback*/
			HAL_UART_RxCpltCallback(huart);
	#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
		 }
	  
		  else
		  {
			 /* Clear RXNE interrupt flag */
			 __HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);
		  }
	  }
  }
}	
#endif
