/***************************************************************************
* File name: MM_ElmPSU.h
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
* ElmPSU Watcher
*
**************************************************************************/

#ifndef _MM_ElmPSU_H_
#define _MM_ElmPSU_H_


/* System Include Files
**************************************************************************/
#include <string.h>
#include <time.h>

#ifdef STM32F767xx
#include "stm32f7xx_hal_dma.h"
#include "stm32f7xx_hal_uart.h"
#else
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_uart.h"
#endif

#include "MM_PSU.h"

/* Defines
**************************************************************************/
#define ElmPSU_PACKET_SIZE 		 (  sizeof(ElmPSUPacket)-sizeof(time_t ))
#define ElmPSU_START_OF_HEADER    ( 0x2 )
#define ElmPSU_END_OF_TX			 ( 0x4 )

#define ELMPSU_NUM_CMDS	5


/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_log.h"
#include "MM_Events.h"
#include "DM_UartHandler.h"

#include "ewrte.h"


struct ElmPSUStatus
{
	int MainsPresent		 	 	: 1;
	int ChargerActive        	: 1;
	int BatteryBulk          	: 1;
	int BatteryTop           	: 1;
	int BatteryFloat         	: 1;
	int BatteryFloatResting  	: 1;
	int PathActive           	: 1;
	int SkaforActive         	: 1;
};

struct ElmPSUFault1
{
	int BatteryMissing			: 1;
	int BatteryHeath				: 1;
	int BatteryLowNoMains		: 1;
	int BatteryHighImpedence	: 1;
	int BadCalibrationData		: 1;
	int ChargerFault				: 1;
	int Lockout						: 1;
	int UARTOverflow				: 1;
};


extern char psu_mains_failure;
extern time_t psu_last_received;
 

class ElmPSU : public Module, public UartHandler
{
	public:
	
	ElmPSU( );
	
	virtual int Receive( Event );
	virtual int Receive( Command* );
   virtual int Receive( Message* );
	
 	virtual void UartRxComplete( UART_HandleTypeDef* uh );
	virtual void UartTxComplete( UART_HandleTypeDef* uh );
 	virtual void UartError( UART_HandleTypeDef* uh );	
	virtual void UartAbort( UART_HandleTypeDef* uh );
 
	
	virtual int Init( );
	virtual void Poll( );
	
	void ProcessResults( );
	 
	uint8_t  buff[ 64];	 

	BatteryStatus batt;
	 

	
	
	int  infault								: 1;
	int got_results						: 1;
	 
 
	int 	battery_low                  : 1;
	int 	battery_critical             : 1;
	int 	charger_failed               : 1;
	int 	charger_on                   : 1;
	int 	no_battery                   : 1;
	int 	battery_resistance_fault     : 1;
	int 	over_voltage                 : 1;
	int   battery_not_detected			  : 1;
	
	uint32_t results[ ELMPSU_NUM_CMDS ];
  
};


#endif

