/***************************************************************************
* File name: MM_PanelNetworking.cpp
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
* Panel networking
*
**************************************************************************/

#ifndef PANEL_NETWORKING_H
#define PANEL_NETWORKING_H


/* System Include Files
**************************************************************************/
#include <string.h>
#ifdef STM32F767xx
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_uart.h"
#include "stm32f7xx_hal_usart.h"

#else
#include "stm32f4xx_hal.h"
#endif
#include <assert.h>
#include <stdlib.h>
#include <time.h>


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Module.h"
#include "MM_Settings.h"
#include "DM_UartHandler.h"
#include "DM_TimerHandler.h"
#include "CO_Site.h"
#include "MM_Events.h"
#include "MM_GPIO.h"
#include "MM_EmbeddedWizard.h"


/* Defines
**************************************************************************/
 
#define MAX_NET_REFS 12 
 
#define FAULT_MSG_LEN 80
 
#define  ALERT_UNUSED 0x7FF
 
class IOStatus;

enum
{
	
	TYPE_DEV = 1,	 
	TYPE_CAE = 2,
	TYPE_CAN = 3,
	
};
 

struct DeviceRef
{
	unsigned short  index : 11;
 	unsigned char  action : 3;	
	unsigned char 	type 	 : 2;
	unsigned char	chan   : 6;	
};

struct TimedDeviceRef
{
	unsigned short index     : 11;
 	unsigned char  action    : 3;	
	unsigned char 	type      : 2;
	unsigned char	chan      : 6;	
	unsigned char	silenced  : 1;
	unsigned char	found		 : 1;
	unsigned char	dispzone;		 
	time_t			timestamp ;
};

struct NetPing
{
	char 				startbyte;
	unsigned char 	sender;
	short				numDevsOnMesh;	
	int 				systemID;
		 	
	DeviceRef alerts[ MAX_NET_REFS ];
	 	
	unsigned short	nonzonal;	
	unsigned short checksum;	
	
	unsigned short GetSum( );
	void Crypt( unsigned int key );
	
	void Send( UART_HandleTypeDef* uart, GPIO de );
};


class PanelNetwork : public Module, public TimerHandler
{
	public:
	
	int 						rxfails;	
	UART_HandleTypeDef* 	net_uart;
	TIM_HandleTypeDef* 	net_timer;	
	GPIO        			data_enable;
	
	NetPing		txping;
	NetPing		rxping;
	
	uint32_t ping_count[ SITE_MAX_PANELS ];
	uint32_t prev_ping_count[ SITE_MAX_PANELS ];
	time_t	last_ping[ SITE_MAX_PANELS ];	
	uint32_t fail_count[ SITE_MAX_PANELS ];
	static uint32_t mesh_count[ SITE_MAX_PANELS ];
	
		
//	unsigned int		current_dispzones[ SITE_MAX_PANELS ][ SITE_NUM_OUTPUT_ACTIONS ][ SITE_MAX_ZONES / 32];
//	unsigned int		current_processzones[ SITE_MAX_PANELS ][ SITE_NUM_OUTPUT_ACTIONS ][ SITE_MAX_ZONES / 32];
//	time_t				timestamp[ SITE_MAX_PANELS ][ SITE_NUM_OUTPUT_ACTIONS ][ SITE_MAX_ZONES / 32][32];
	
//	DeviceRef		first[ SITE_MAX_PANELS ][ SITE_NUM_OUTPUT_ACTIONS ];
//	DeviceRef		last[ SITE_MAX_PANELS ][ SITE_NUM_OUTPUT_ACTIONS ];
	
	char fault_msg[ SITE_MAX_PANELS ][ FAULT_MSG_LEN ];
	char fault_rx_msg[ SITE_MAX_PANELS ][ FAULT_MSG_LEN ];
	int current_fault[ SITE_MAX_PANELS ];	
	
   IOStatus*						iostatus;
	Settings* 						settings;
	EmbeddedWizard*				ew;
	int								inerror;
	static bool 					got_packet;
	static bool 					sent_packet;
	bool								time_to_send;
	
	unsigned short					toSend;
	
	void SendPacket( );

	PanelNetwork( ); 
	
	virtual int Init( );
	
	virtual int Receive( Event e );
	virtual int Receive( Command* );
	virtual int Receive( Message* );
	virtual void Poll( );
		
	static void UartTxComplete( UART_HandleTypeDef* uart );				
	static void UartRxComplete( UART_HandleTypeDef* uart );			
	static void UartError( UART_HandleTypeDef* uart );	
 
 
	virtual void TimerPeriodElapsed( TIM_HandleTypeDef* tim );
	
	
	NetPing* CheckPacket( int key );
	void GetAlertListFromPing( NetPing* );
	void Restart( int t );
};


#endif
