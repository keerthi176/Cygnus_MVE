/***************************************************************************
* File name: MM_main.cpp
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
* Module base class.
*
**************************************************************************/
/* System Include Files
**************************************************************************/
#include "stm32f7xx_hal_crc.h"
#include "stm32f7xx_hal_dma.h"
#include "stm32f7xx_hal_uart.h"
#include "stm32f7xx_hal_usart.h"
#include <stdlib.h>



/* User Include Files
**************************************************************************/

#include "MM_Init.h"

#include "ewrte.h"
#include "ewgfx.h"
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_Application.h"
#include "MM_MemCheck.h"
#include "MM_Backlight.h"
#include "MM_EmbeddedWizard.h"
#include "MM_CrystalMonitor.h"
#include "MM_PSU.h"
#include "MM_PSU5.h"
#include "MM_SplashLoader.h"
#include "MM_GSM.h"
#include "MM_USBHost.h"
//#include "MM_USBFATFS.h"
#include "MM_RealTimeClock.h"
#include "MM_NCU.h"
#include "MM_IOStatus.h"
#include "MM_USBSerial.h"
#include "MM_QuadSPI.h"
#include "MM_GPIO.h"
#include "MM_Utilities.h"
#include "MM_PanelNetwork.h"
#include "MM_PanelIO.h"
#include "MM_ADC.h"
#include "MM_Ethernet.h"
#include "MM_CAN.h"
#include "MM_DebugPort.h"
#include "MM_PowerReset.h"
#include "MM_Fault.h"
#include "MM_Buzzer.h"
#include "MM_SDFATFS.h"
#include "MM_Evacuate.h"
#include "MM_Access.h"
#include "MM_ImportXML.h"
#include "MM_DFU.h"
#include "MM_CauseAndEffect.h"
#include "MM_LED.h"
#include "MM_TouchScreen.h"
#include "MM_Events2.h"
#include "MM_MeshLog.h"
#include "MM_CSV.h"
			
#ifdef GSM_HTTP			
#include "MM_AppSupport.h"
#else
#include "MM_MQTTSupport.h"
#endif	

#include "MM_TestGSM.h"


#ifdef BOOT_MENU
#include "BM_BootMenu.h"
#endif

#ifdef BD_TEST_ONLY
#include "..\BDTest\BDTest.h"
#endif


/* Defines
**************************************************************************/
 
#define MODULE( m ) m m##_instance
#define BOOT_MENU_ADDRESS ( 0x81C0000 ) 

/* Globals
**************************************************************************/

extern "C" int dbh( void*, const char* fmt, ... );



extern SPI_HandleTypeDef hspi1;
extern LPTIM_HandleTypeDef hlptim1;
extern CAN_HandleTypeDef hcan1;
extern USBD_HandleTypeDef hUsbDeviceFS;

extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim8;


static Init initialization;

Application app( 								/* gsm_uart 				*/ &huart3,
													  /* debug_uart 			*/ &huart7,
													  /* ncu_uart 				*/ &huart2,
													  /* ncu_dma				*/ NULL,
													  /* panel_network_uart	*/ &huart6,
													  /* psu_uart	*/ 			&huart4,	
													  /* psu_uart5   */        &huart5,
													  /* log_uart   */         &huart1,
													  /* touch_i2c 			*/ &hi2c1,
													  /* backlight_timer    */	&hlptim1,
													  /* second_timer  */		&htim7,
													  /* network_timer  */		&htim13,
													  /* led_flash_timer;*/  	&htim8,
													  /* rtc 					*/ &hrtc,
													  /* crc				   	*/ &hcrc,
													  /* watch dog */          &hiwdg,
																				//		&hUsbHostHS,
																						&hUsbDeviceFS,
																						&hqspi,
														/* panel i/o ADC */		&hspi1,  
																						&hcan1 ) ;

HandlerManager<UART_HandleTypeDef, UartHandler, 7> UartHandler::manager;
 
#ifndef BD_TEST_ONLY


#ifdef GSM_HTTP
static AppSupport				m39;
#else
 static MQTTSupport			   	 m39;  //first!
#endif
static Log                     m8;

// static TestGSM						m99;

static SplashLoader				 m;
static PowerReset 				 m0;	
static Settings					 m1;
static IOStatus                m2;
static NCU                     m3;
static LED                     m5;
static EmbeddedWizard          m6;
static SDFATFS                 m7;

static DebugPort					 d0;
static QuadSPI                 m10;
static GPIOManager             m13;
static EEPROM                  m11;
static TouchScreen             m14;
static MemCheck                m15;
static Backlight               m16;
static CrystalMonitor          m17;
static ExportXML               m18;
static ImportXML 	             m19;
static Utilities               m20;
static RealTimeClock           m21;
static GSM                     m22;
static USBHost                 m23;
static Buzzer                  m24;
static ElmPSU                  m26a;
static PSU							 m26b;
static ADC                     m27;
static PanelIO                 m28;
static Evacuate                m29;
static DFUMode                 m30;
static CauseAndEffect          m31;
//static Ethernet                m32;
//static CAN                     m33;
static Fault  	                m34;
static ATCommands              m35;
static USBSerial 		          m36;
static PanelNetwork          	 m37;
static CSV							 m38;

 #endif	
 


 
/*************************************************************************/
/**  \fn      void SetBootOptions( )
***  \brief   Global helper function
**************************************************************************/

void SetBootOptions( )
{
	FLASH_OBProgramInitTypeDef OBInit;
	
	/* Allow Access to Flash control registers and user Flash */
	HAL_FLASH_Unlock( );

	/* Allow Access to option bytes sector */ 
	HAL_FLASH_OB_Unlock( );

	/* next boot will be executed from bank2 */ 
	OBInit.OptionType = OPTIONBYTE_BOOTADDR_0 | OPTIONBYTE_BOOTADDR_1 | OPTIONBYTE_USER | OPTIONBYTE_BOR ;
	OBInit.BootAddr0  = __HAL_FLASH_CALC_BOOT_BASE_ADR( 0x08000000 );
	OBInit.BootAddr1  = __HAL_FLASH_CALC_BOOT_BASE_ADR( 0x081C0000 );
	OBInit.BORLevel   = OB_BOR_LEVEL1;
	

	OBInit.USERConfig = OB_DUAL_BOOT_ENABLE | OB_WWDG_SW  | OB_STOP_NO_RST | OB_STDBY_NO_RST |  OB_IWDG_SW |
								OB_IWDG_STOP_FREEZE | OB_IWDG_STDBY_FREEZE | OB_NDBANK_SINGLE_BANK;
		
	if ( HAL_FLASHEx_OBProgram(&OBInit) != HAL_OK )
	{
		/*
		Error occurred while setting option bytes configuration.
		User can add here some code to deal with this error.
		To know the code error, user can call function 'HAL_FLASH_GetError()'
		*/

		app.Report( ISSUE_FLASH_OPTION_BYTES_FAIL );

		return;
	}

	/* Start the Option Bytes programming process */  
	if ( HAL_FLASH_OB_Launch( ) != HAL_OK )
	{
		/*
		Error occurred while reloading option bytes configuration.
		User can add here some code to deal with this error.
		To know the code error, user can call function 'HAL_FLASH_GetError()'
		*/

		app.Report( ISSUE_FLASH_OPTION_BYTES_FAIL );

		return;
	}

	/* Prevent Access to option bytes sector */
	HAL_FLASH_OB_Lock( );

	/* Disable the Flash option control register access (recommended to protect 
	the option Bytes against possible unwanted operations) */
	HAL_FLASH_Lock( );

	/* Initiate a system reset request to reset the MCU */
	 
	
}
 

extern int Stack_Mem;

/*************************************************************************/
/**  \fn      void main_application( )
***  \brief   C++ second main
**************************************************************************/

 
extern "C" void main_application( )
{		
	SetBootOptions( );
	
	app.DebOut( "Top stack: %X\n", &Stack_Mem );
 

	app.InitModules( );	
	app.Go( );
}	
	
 
 

#ifdef BD_TEST_ONLY
	
/*********************** PHIL'S AREA *****************************/	
/*      please dont change anywhere else                         */
/*****************************************************************/	
	
  // --- Modules BDTest requires to be running ---
   // --- Modules BDTest requires to be running ---
   NEW( ATCommands );
//   NEW( DebugPort );			PSL - Stops BDTest from receiving UART data
//   NEW( SplashLoader );  
   NEW( EEPROM );  
   NEW( USBFATFS );
   NEW( GPIOManager );
   NEW( TouchScreen );
//   NEW( MemCheck ); 
   NEW( Backlight );
   NEW( CrystalMonitor ); 	
   NEW( QuadSPI );
   NEW( Settings ); 
//   NEW( EmbeddedWizard );
   NEW( IOStatus );
 
//   NEW( NCU );	
   NEW( USBSerial );
   NEW( USBHost );
//   NEW( PowerReset );
//   NEW( Alarms );
//   NEW( RealTimeClock );		PSL - Caused crash!  
//   NEW( PSU ); 
//   NEW( GSM );
   NEW( ADC );
//   NEW( PanelIO );
//   NEW( PanelNetworking ); 
   NEW( Utilities );   
// NEW( CauseAndEffect );

   NEW( Ethernet ); 
// NEW( CAN );

	MX_UART5_Init();

  NEW( BDTest );
	
/*******************************************************************/	
	
	app.InitModules( );	
	app.Go( );
}

#endif

