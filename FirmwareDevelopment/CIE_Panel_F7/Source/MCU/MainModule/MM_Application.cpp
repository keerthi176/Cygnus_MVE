/***************************************************************************
* File name: MM_Application.cpp
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
/* System Include Files
**************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#ifdef STM32F767xx
#include "stm32f7xx_hal_crc.h"
#include "stm32f7xx_hal_dma.h"
#include "stm32f7xx_hal_usart.h"
#include "stm32f7xx_hal.h"
#else
#include "stm32f4xx_hal_crc.h"
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_usart.h"
#include "stm32f4xx_hal.h"
#endif

//#include "usbh_core.h"



/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_log.h"
#include "MM_Events.h"
#include "MM_error.h"

#include "MM_Settings.h"

#include "ewrte.h"



Application::Application(  UART_HandleTypeDef* gsm_uart, 			
									UART_HandleTypeDef* debug_uart,
									UART_HandleTypeDef* ncu_uart,
									DMA_HandleTypeDef*  ncu_dma,
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
								//	USBH_HandleTypeDef* usb_host,
									USBD_HandleTypeDef* usb_device,
									QSPI_HandleTypeDef* qspi,
									SPI_HandleTypeDef*  spi,
									CAN_HandleTypeDef*  can
)
									:
									gsm_uart( gsm_uart ),
									debug_uart( debug_uart ),
									ncu_uart( ncu_uart ),
									ncu_dma( ncu_dma ),
									panel_network_uart( panel_network_uart ),
									psu_uart( psu_uart ),
									psu_uart5( psu_uart5 ),
									log_uart( log_uart ),
									touch_i2c( touch_i2c ),
									backlight_timer( backlight_timer ),
									second_timer( second_timer ),
									network_timer( network_timer ),
									led_flash_timer( led_flash_timer ),
									rtc( rtc ),
									crc( crc ),
									iwdg( iwdg ),
						//			usb_host( usb_host ),
									usb_device( usb_device ),
									qspi( qspi ),
									adc_spi( spi ),
									can( can ),
                           message_queue( message_queue_buffer, MESSAGE_BUFFER_SIZE )
{
	loop_count = 0;
	module_list_pos = module_list;
	suspended = false;
	loading = true;
	touch_addr = NULL;
}


/*************************************************************************/
/**  \fn      void Application::Go( )
***  \brief   Class function
**************************************************************************/

void Application::Go( )
{
	while ( 1 )
	{
		Message* msg = NULL;
		loop_count++;
						
		// Service each module

		for ( iter = module_list; iter < module_list_pos; iter++ )
		{
			if ( (*iter)->priority != 0 )
			{	
				if ( loop_count % (*iter)->priority == 0 )
				{
					(*iter)->Poll( );
				}
			}
		}
		
		// Reset WD counter.
		HAL_IWDG_Refresh( app.iwdg );
		
		// Check message queue
		while ( message_queue.items > 0 )
		{
			// Pop off queue
			msg = message_queue.Pop( );
			
			// if msg
			if ( msg != NULL )
			{
				if ( msg->to != NULL )	msg->to->Receive( msg );
			}
			else
			{
				app.DebOut( "NULL msg!\n" );
			}
		}
	
	}
}


/*************************************************************************/
/**  \fn      void Application::Send( Event event )
***  \brief   Class function
**************************************************************************/

void Application::Send( Event event )
{
// Check each module
	Module** iter;
	                                        
	for ( iter = module_list; iter < module_list_pos; iter++ )
	{
		if ( (*iter)->event_mask & event )
		{
			(*iter)->Receive( event );
		}
	}
}

/*************************************************************************/
/**  \fn      void Application::Send( Message* msg )
***  \brief   Class function
**************************************************************************/

void Application::Send( Message* msg )
{
	if ( msg != NULL )
		message_queue.Push( *msg );
}


/*************************************************************************/
/**  \fn      int Application::InitModules( )
***  \brief   Class function
**************************************************************************/

void Application::InitModules( )
{	
	for ( Module** iter = module_list; iter < module_list_pos; iter++ )
	{
		if ( ! (*iter)->Init( ) )
		{
			app.DebOut( "Module '%s' failed to init properly.\n", (*iter)->name );
		}
	}
}
 
#if 0
static const char* DebCmd( int cmd )
{
	switch (cmd )
	{
		case CMD_SET_BACKLIGHT_LEVEL:											return "CMD_SET_BACKLIGHT_LEVEL";
		case CMD_SET_BACKLIGHT_ON:												return "CMD_SET_BACKLIGHT_ON";
		case CMD_SET_BACKLIGHT_OFF:												return "CMD_SET_BACKLIGHT_OFF";
		case CMD_SET_TIME:														return "CMD_SET_TIME";
		case CMD_SET_DATE:														return "CMD_SET_DATE";
		case CMD_SET_DST_START_DATE:												return "CMD_SET_DST_START_DATE";
		case CMD_SET_DST_END_DATE:												return "CMD_SET_DST_END_DATE";
		case CMD_SET_DST_START_HOUR:												return "CMD_SET_DST_START_HOUR";
		case CMD_SET_DST_END_HOUR:													return "CMD_SET_DST_END_HOUR";
		case CMD_SET_CALIB_VALUES:														return "CMD_SET_CALIB_VALUES";
		case CMD_GET_PSU_STATUS:														return "CMD_GET_PSU_STATUS	";
		case CMD_GSM_SIGNAL_QUALITY:													return "CMD_GSM_SIGNAL_QUALITY";
		case CMD_GSM_SIGNAL_PERIOD:												return "CMD_GSM_SIGNAL_PERIOD";
		case CMD_LOAD_SETTINGS_USB_SERIAL:												return "CMD_LOAD_SETTINGS_USB_SERIAL";
		case CMD_LOAD_SETTINGS_USB_SERIAL_MSG:												return "CMD_LOAD_SETTINGS_USB_SERIAL_MSG";
		case CMD_SAVE_SETTINGS_USB_SERIAL:												return "CMD_SAVE_SETTINGS_USB_SERIAL";
		case CMD_GET_PROGRESS:												return "CMD_GET_PROGRESS";
		case CMD_CANCEL_PROGRESS:												return "CMD_CANCEL_PROGRESS";
		case CMD_SAVE_SETTINGS_USB_HOST:												return "CMD_SAVE_SETTINGS_USB_HOST";
		case CMD_LOAD_SETTINGS_USB_HOST:												return "CMD_LOAD_SETTINGS_USB_HOST";
		case CMD_CANCEL_LOAD_SETTINGS:												return "CMD_CANCEL_LOAD_SETTINGS";
		case CMD_GET_DEVICE_LISTS:												return "CMD_GET_DEVICE_LISTS";
		case CMD_GET_MESH_STAGE:												return "CMD_GET_MESH_STAGE";
		case CMD_RESTART_MESH:												return "CMD_RESTART_MESH";
		case CMD_SYNCHRONISE_MESH:												return "CMD_SYNCHRONISE_MESH";
		case CMD_GET_BACKLIGHT_VOLTAGE:												return "CMD_GET_BACKLIGHT_VOLTAGE";
		case CMD_GET_NUM_FIRES:												return "CMD_GET_NUM_FIRES";
		case CMD_GET_VERSION_NUMBERS:												return "CMD_GET_VERSION_NUMBERS";
		case CMD_LOAD_FILE_AT_INDEX:												return "CMD_LOAD_FILE_AT_INDEX";
		case CMD_EXIT_BOOT_MENU:												return "CMD_EXIT_BOOT_MENU";
		case CMD_FACTORY_RESET:												return "CMD_FACTORY_RESET";
		case CMD_RESET_EEPROM:												return "CMD_RESET_EEPROM";
		case CMD_GET_FILE_AT_INDEX:												return "CMD_GET_FILE_AT_INDEX";
		case CMD_GET_ITEMS_IN_FOLDER:												return "CMD_GET_ITEMS_IN_FOLDER";
		case CMD_TOUCH_EMULATION:												return "CMD_TOUCH_EMULATION";
		case CMD_COPY_BITMAPS:												return "CMD_COPY_BITMAPS";
		case CMD_GET_DEVICE_CONFIG:												return "CMD_GET_DEVICE_CONFIG";
		case CMD_GET_NUM_ZONES:												return "CMD_GET_NUM_ZONES";
		case CMD_GET_NUM_DEVS_IN_ZONE:												return "CMD_GET_NUM_DEVS_IN_ZONE";
		case CMD_GET_DEV_IN_ZONE:												return "CMD_GET_DEV_IN_ZONE";
		case CMD_GET_DEVICE:												return "CMD_GET_DEVICE";
		case CMD_SET_OUTPUT_CHANNEL:												return "CMD_SET_OUTPUT_CHANNEL";
		case CMD_GET_NUM_MAX_ZONES:												return "CMD_GET_NUM_MAX_ZONES";
		case CMD_SET_DEVICE:												return "CMD_SET_DEVICE";
		case CMD_SET_DEVICE_ALL:												return "CMD_SET_DEVICE_ALL";
		case CMD_GET_LAST_RAW_TOUCH:												return "CMD_GET_LAST_RAW_TOUCH";
		case CMD_SET_INPUT_IN_TEST:												return "CMD_SET_INPUT_IN_TEST";
		case CMD_GET_DEVICE_FROM_CONFIG:												return "CMD_GET_DEVICE_FROM_CONFIG";
		case CMD_FIND_DEVICE_FROM_UNIT:												return "CMD_FIND_DEVICE_FROM_UNIT";
		case CMD_KILL_DOG:												return "CMD_KILL_DOG";
		case CMD_GET_FAULT_ITEM:												return "CMD_GET_FAULT_ITEM";
		case CMD_GET_NUM_FAULTS:												return "CMD_GET_NUM_FAULTS";
		case CMD_GET_NAME:												return "CMD_GET_NAME";
		case CMD_SET_NAME:													return "CMD_SET_NAME,";
		case CMD_GET_LOG_LENGTH:												return "CMD_GET_LOG_LENGTH";
		case CMD_GET_LOG_LINE:										return "CMD_GET_LOG_LINE";
		case CMD_GET_MESH_LOG_LENGTH:												return "CMD_GET_MESHLOG_LENGTH";
		case CMD_GET_MESH_LOG_LINE:										return "CMD_GET_MESHLOG_LINE";
				case CMD_GET_NUM_OTHER_EVENTS: 												return "CMD_GET_NUM_OTHER_EVENTS,";
		case CMD_LEDS_TEST:												return "CMD_LEDS_TEST";
		case CMD_LOGON_ACCESS:												return "CMD_LOGON_ACCESS";
		case CMD_GET_ON_TEST_DETAILS:												return "CMD_GET_ON_TEST_DETAILS";
		case CMD_GET_NUM_ON_TEST:												return "CMD_GET_NUM_ON_TEST";
		case CMD_SET_INPUT_DISABLED:												return "CMD_SET_INPUT_DISABLED";
		case CMD_SET_OUTPUT_DISABLED:												return "CMD_SET_OUTPUT_DISABLED";
		case CMD_GET_DISABLED_DETAILS:												return "CMD_GET_DISABLED_DETAILS";
		case CMD_GET_NUM_DISABLED:													return "CMD_GET_NUM_DISABLED,";
		case CMD_GET_RADIO_UNIT_CHILD:												return "CMD_GET_RADIO_UNIT_CHILD";
		case CMD_GET_NUM_MESH_ITEMS:												return "CMD_GET_NUM_MESH_ITEMS";
		case CMD_GET_RADIO_UNIT_NOT_ON_MESH:												return "CMD_GET_RADIO_UNIT_NOT_ON_MESH";
		case CMD_GET_NUM_EXTRA_RADIO_UNITS:												return "CMD_GET_NUM_EXTRA_RADIO_UNITS";
		case CMD_SET_CURRENT_DEVICE:												return "CMD_SET_CURRENT_DEVICE";
		case CMD_DFU_MODE:												return "CMD_DFU_MODE";
		case CMD_SET_FW_LENGTH:												return "CMD_SET_FW_LENGTH";
		case CMD_GET_ZONE_FLAGS:												return "CMD_GET_ZONE_FLAGS";
		case CMD_SET_ZONE_FLAGS:												return "CMD_SET_ZONE_FLAGS";
		case CMD_GET_RADIO_UNIT:												return "CMD_GET_RADIO_UNIT";
		case CMD_GET_SITE:												 return "CMD_GET_SITE";
		case CMD_GET_ALERT_DETAILS:												return "CMD_GET_ALERT_DETAILS";
		case CMD_GET_NTH_ZONE: return "CMD_GET_NTH_ZONE";
		case CMD_GET_NUM_ZONES_IN_FIRE: return "CMD_GET_NUM_ZONES_IN_FIRE";
		case CMD_GET_NUM_ALERTS_IN_ZONE: return "CMD_GET_NUM_ALERTS_IN_ZONE";
		case CMD_GET_PANEL_NAME: return "CMD_GET_PANEL_NAME";
		case CMD_GET_PANEL_ZONE_FLAGS: return "CMD_GET_PANEL_ZONE_FLAGS";
		case CMD_SET_PANEL_ZONE_FLAGS: return "CMD_SET_PANEL_ZONE_FLAGS";
		case CMD_PPU_MODE: return "CMD_PPU_MODE";
		case CMD_GET_DAY_TIME: return "CMD_GET_DAY_TIME";
		case CMD_SET_DAY_TIME: return "CMD_SET_DAY_TIME";
		case CMD_GET_SMS_NUMBER: return "CMD_GET_SMS_NUMBER";
		case CMD_SET_SMS_NUMBER: return "CMD_SET_SMS_NUMBER";
		case CMD_REMOVE_SMS_NUMBER: return "CMD_REMOVE_SMS_NUMBER";
		case CMD_GET_NUM_SMS_CONTACTS: return "CMD_GET_NUM_SMS_CONTACTS";
		case CMD_GET_SETTINGS: return "CMD_GET_SETTINGS";
		case CMD_SET_SETTINGS: return "CMD_SET_SETTINGS";
		case CMD_CHECK_PASSKEY: return "CMD_CHECK_PASSKEY";
		case CMD_SET_PASSKEY: return "CMD_SET_PASSKEY";
		case CMD_GET_ZONE_DISABLED: return "CMD_GET_ZONE_DISABLED";
		case CMD_SET_ZONE_DISABLED: return "CMD_SET_ZONE_DISABLED";
		case CMD_SET_TEST_PROFILE: return "CMD_SET_TEST_PROFILE";
	}
	return "";
}	
#endif 


/*************************************************************************/
/**  \fn      int Application::Send( Command* cmd )
***  \brief   Class function
**************************************************************************/

int Application::Send( Command* cmd )
{	
	int result;
	
	assert( cmd != NULL );
	
	for ( Module** iter = module_list; iter < module_list_pos; iter++ )
	{
		result = (*iter)->Receive( cmd );
		
		if ( result != CMD_ERR_UNKNOWN_CMD )
		{	
	 	//	app.DebOut( "%s: %s\n", (*iter)->name, DebCmd( cmd->type ) );
			return result;
		}
		// else try next module..	
	}
	return CMD_ERR_UNKNOWN_CMD;
	// ERROR( ERR_UNKNOWN_CMD );
}


/*************************************************************************/
/**  \fn      void Application::DebOut( const char* fmt, ... )
***  \brief   Class function
**************************************************************************/
 
void Application::DebOut( const char* fmt, ... )
{ 
	if ( fmt != NULL )
	{
		char buffer[ 800];
		va_list args;
		
		va_start( args, fmt );
		vsnprintf( buffer, 799, fmt, args );
		HAL_UART_Transmit( debug_uart, (uint8_t*) buffer, strlen( buffer ), 300 );
		va_end( args );
	}
}


/*************************************************************************/
/**  \fn      void db( const char* fmt, ... )
***  \brief   Global helper function
**************************************************************************/

extern "C" void db( const char* fmt, ... )
{
	if ( fmt != NULL )
	{
		char buffer[ 256];
		va_list args;
		
		va_start( args, fmt );
		vsnprintf( buffer, 256, fmt, args );
		HAL_UART_Transmit( app.debug_uart, (uint8_t*) buffer, strlen( buffer ), 300 );
		va_end( args );
	}
}


/*************************************************************************/
/**  \fn      void db( const char* fmt, ... )
***  \brief   Global helper function
**************************************************************************/

extern "C" int dbh( void*, const char* fmt, ... )
{
	if ( fmt != NULL )
	{
		char buffer[ 256];
		va_list args;
		
		va_start( args, fmt );
		vsnprintf( buffer, 256, fmt, args );
		HAL_UART_Transmit( app.debug_uart, (uint8_t*) buffer, strlen( buffer ), 300 );
		va_end( args );
	}
	return 0;
}


/*************************************************************************/
/**  \fn      void Application::ReportIssue( int )
***  \brief   Class function
**************************************************************************/

void Application::ReportIssue( const char* i )
{
	if ( i != NULL )
		app.DebOut( "Issue '%s'  \n", i );
}


/*************************************************************************/
/**  \fn      void Application::Log( int )
***  \brief   Class function
**************************************************************************/

void Application::Log( int )
{

}	


/*************************************************************************/
/**  \fn      void Application::RegisterModule( Module* m )
***  \brief   Class function
**************************************************************************/

void Application::RegisterModule( Module* m )
{
	assert( m != NULL );
	*module_list_pos++ = m;
}	
	

/*************************************************************************/
/**  \fn      Module* Application::FindModule( const char* name )
***  \brief   Class function
**************************************************************************/

Module* Application::FindModule( const char* name )
{	
	if ( name != NULL ) for( Module** iter = module_list; iter < module_list_pos; iter++ )
	{
		if ( !strcmp( (*iter)->name, name ))
		{
			return *iter;
			
		}
	}
	return NULL;
}


/*************************************************************************/
/**  \fn      Module* Application::ResumeActivities( )
***  \brief   Class function
**************************************************************************/

void Application::ResumeActivities( )
{
	app.DebOut( "Resuming...\n" );
	suspended = false;
}
	
	
/*************************************************************************/
/**  \fn      Module* Application::SuspendActivities( )
***  \brief   Class function
**************************************************************************/

void Application::SuspendActivities( )
{
	app.DebOut( "Suspending...\n" );
	suspended = true;
}
