/***************************************************************************
* File name: MM_Application.h
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
* Main application
*
**************************************************************************/
#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#ifdef __cplusplus

	#include "MM_Message.h"
	#include "MM_Command2.h"
	#include "MM_Events.h"
	#include "MM_list.h"
	#include "CO_Site.h"

	class Module;


	#ifdef BOOT_MENU
	#include "_BootMenuDeviceClass.h"
	#include "BootMenu.h"
	#else
	#include "_AppDeviceClass.h"
	#endif

#endif

/* System Include Files
**************************************************************************/
//#include "usbh_def.h"
#include "usbd_def.h"

/* User Include Files
**************************************************************************/


#include "crc.h"
#include "usart.h"
#include "i2c.h"
#include "tim.h"
#include "rtc.h"
#include "iwdg.h"
#include "quadspi.h"
#include "spi.h"


#define STRNCPY( x, y, n )	{ strncpy( x, y, n ); (x)[ n - 1 ] = 0; }
#define STRNCAT( x, y, n )	{ strncat( x, y, n ); (x)[ n - 1 ] = 0; }


/* Defines
**************************************************************************/
#define MAX_MODULES 								41
#define MESSAGE_BUFFER_SIZE   				48 
 

#define Report( x ) ReportIssue(  #x )

 

//#define DebOut(...) panel; 

 

typedef struct
{
	char one;
	char start;
	short x;
	short y;
	short pressure;
} Touch;	
	

		
#ifdef __cplusplus

class Application
{
	public:
		
	
#else

typedef struct
{	
#endif
	
	UART_HandleTypeDef* 	gsm_uart; 			
	UART_HandleTypeDef* 	debug_uart;
	UART_HandleTypeDef* 	ncu_uart;
	DMA_HandleTypeDef* 	ncu_dma;
	UART_HandleTypeDef* 	panel_network_uart;
	UART_HandleTypeDef* 	psu_uart;
	UART_HandleTypeDef* 	psu_uart5;
	UART_HandleTypeDef* 	log_uart;
	I2C_HandleTypeDef*  	touch_i2c;
	LPTIM_HandleTypeDef* backlight_timer;
	TIM_HandleTypeDef*  	second_timer;
	TIM_HandleTypeDef*  	network_timer;
	TIM_HandleTypeDef*   led_flash_timer;
	RTC_HandleTypeDef*  	rtc;
	CRC_HandleTypeDef*  	crc;
	IWDG_HandleTypeDef* 	iwdg;
//	USBH_HandleTypeDef* 	usb_host;
	USBD_HandleTypeDef* 	usb_device;
	QSPI_HandleTypeDef* 	qspi;
	SPI_HandleTypeDef*  	adc_spi;
	CAN_HandleTypeDef*   can;
	
	Panel* panel;
	Site*	 site;
	
	Touch* touch_addr;
	
#ifndef __cplusplus
} Application;

#else	
	Application( UART_HandleTypeDef* gsm_uart, 			
					 UART_HandleTypeDef* debug_uart,
					 UART_HandleTypeDef* ncu_uart,
					 DMA_HandleTypeDef* ncu_dma,
					 UART_HandleTypeDef* panel_network_uart,
					 UART_HandleTypeDef* psu_uart,
					 UART_HandleTypeDef* psu_uart5,
					 UART_HandleTypeDef* log_uart,
					 I2C_HandleTypeDef*  touch_i2c,
					 LPTIM_HandleTypeDef*  backlight_timer,
					 TIM_HandleTypeDef*  second_timer,
					 TIM_HandleTypeDef*  network_timer,
					 TIM_HandleTypeDef*   led_flash_timer,
					 RTC_HandleTypeDef*  rtc,
					 CRC_HandleTypeDef*  crc,
					 IWDG_HandleTypeDef* iwdg,
//					 USBH_HandleTypeDef* usb_host,
					 USBD_HandleTypeDef* usb_device,
					 QSPI_HandleTypeDef* qspi,
					 SPI_HandleTypeDef*  spi,
					 CAN_HandleTypeDef*  can
					 );				   
		
	void Send( Message* );
	int  Send( Command* );
	void Send( Event );
	
	
	void InitModules( void );
	
	void SuspendActivities( );
	void ResumeActivities( );
	void RegisterModule( Module* );
	void ReportFault( int );
	void ReportIssue( const char* );
	void Log( int );
	void Error( const char*, int );
	
 
 	void DebOut( const char* fmt, ... );
 

	void Go( );
	
	Module* FindModule( const char* name );
	

	Module*				   module_list[ MAX_MODULES ];
	Module**					module_list_pos;
	
	Circular<Message>    message_queue;	
   Message              message_queue_buffer[ MESSAGE_BUFFER_SIZE ];




#ifndef BOOT_MENU

AppDeviceClass 	DeviceObject;

#else
	BootMenuDeviceClass 	DeviceObject;
#endif	
		
	int loop_count;
	int suspended;
	int loading;
	
	Module** iter;
};

#endif

extern Application app;

#ifndef __cplusplus
extern Application* capp;
#endif

#endif
