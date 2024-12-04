/***************************************************************************
* File name: MM_PSU.h
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
* PSU Watcher
*
**************************************************************************/

#ifndef _MM_PSU_H_
#define _MM_PSU_H_


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


/* Defines
**************************************************************************/
#define PSU_PACKET_SIZE 		 (  sizeof(PSUPacket)-sizeof(time_t ))
#define PSU_START_OF_HEADER    ( 0x2 )
#define PSU_END_OF_TX			 ( 0x4 )


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

extern char psu_mains_failure;
extern time_t psu_last_received;

typedef enum
{
	BATT_GOOD = 0,
	BATT_LOW,
	BATT_CRIT,
	BATT_HIGHQ,
	BATT_CHARGE_FAIL
} BatteryStatus;

typedef enum
{
	PSU_TYPE_CTEC    = 0,
	PSU_TYPE_ELMDENE,
	PSU_TYPE_MAX
}PSU_TYPE;


__packed class PSUPacket
{
	public:
		
	char 	dummy_byte;   // to make shorts on even address
	char 	start;
	char 	temp_msb;
	char 	temp_lsb;
	char 	battery_charge_msb;
	char 	battery_charge_lsb;
	char 	battery_on_load_msb;
	char 	battery_on_load_lsb;
	char 	battery_no_load_msb;
	char 	battery_no_load_lsb;

	int 	psu_sb_mains_failure            : 1;
	int 	psu_sb_battery_low              : 1;
	int 	psu_sb_battery_critical         : 1;
	int 	psu_sb_charger_failed           : 1;
	int 	psu_sb_charger_on               : 1;
	int 	psu_sb_no_battery               : 1;
	int 	psu_sb_battery_resistance_fault : 1;
	int 	psu_sb_over_voltage             : 1;

	char 	end;
	char 	checksum_msb;
	char 	checksum_lsb;

	
	char dummy_data[ 16];
	
	time_t timestamp;
	
	int temp( );
	int battery_resistance( );
	int battery_charge( );
	int battery_on_load( );
	int battery_no_load( );
};

 

class PSU : public Module 
{
	public:
	
	PSU( );
	
	virtual int Receive( Event );
	virtual int Receive( Command* );
   virtual int Receive( Message* );
	
   static void UartRxComplete( UART_HandleTypeDef* uh, uint16_t bytes );
// 	virtual void UartError( UART_HandleTypeDef* uh );	
//	virtual void UartAbort( UART_HandleTypeDef* uh );
//	virtual void UartStartPacket( UART_HandleTypeDef* uh );
	
	virtual int Init( );
	virtual void Poll( );
	
	static int lastsum;
		
	int CheckSum( PSUPacket* p );
	
	int ProcessPacket( PSUPacket* p );
		
	PSUPacket 		packet_buffer[ 2];

	static bool got_packet;
	int packets_missed;
	
	static PSUPacket* 		rxbuffer;	
	static PSUPacket*     prevbuffer;

	static int  infault;
	int wait_for_idle;
	 
	int battery_low             : 1;
	int battery_critical        : 1;
	int charger_failed          : 1;
	int charger_on              : 1;
	int no_battery              : 1;
	int battery_resistance_fault: 1;
	int over_voltage            : 1;
	int battery_not_detected	: 1;
};



#endif

