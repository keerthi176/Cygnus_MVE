/***************************************************************************
* File name: Application.cpp
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
#include "stm32f4xx_hal_crc.h"
#ifdef _USE_RTOS_	
#include "cmsis_os.h"
#endif

/* User Include Files
**************************************************************************/
#include "Module.h"
#include "Message.h"
#include "Application.h"
#include "Command.h"
#include "log.h"

/* Defines
**************************************************************************/
#define ITER(x) std::__rw_list_iter<x, std::ptrdiff_t, x*, x&> 



Application::Application(  UART_HandleTypeDef* gsm_uart, 			
									UART_HandleTypeDef* debug_uart,
									UART_HandleTypeDef* panel_network_uart,
									I2C_HandleTypeDef*  touch_i2c,
									TIM_HandleTypeDef*  backlight_timer,
									TIM_HandleTypeDef*  centisecond_timer,
									RTC_HandleTypeDef*  rtc,
									CRC_HandleTypeDef*  crc,
									IWDG_HandleTypeDef* iwdg )
									:
									gsm_uart( gsm_uart ),
									debug_uart( debug_uart ),
									panel_network_uart( panel_network_uart ),
									touch_i2c( touch_i2c ),
									backlight_timer( backlight_timer ),
									centisecond_timer( centisecond_timer ),
									rtc( rtc ),
									crc( crc ),
									iwdg( iwdg )
{
	loop_count = 0;
	module_list_pos = module_list;
}

#ifdef _USE_RTOS_

void low_priority( const void* module )
{
	Module* m = (Module*) module;
	while (1) m->Poll( );
}

void normal_priority( const void* module )
{
	Module* m = (Module*) module;
	while (1) m->Poll( );
}

void high_priority( const void* module )
{
	Module* m = (Module*) module;
	while (1) m->Poll( );
}

void realtime_priority( const void* module )
{
	Module* m = (Module*) module;
	while (1) m->Poll( );
}


static osPoolDef( msg_pool, MESSAGE_BUFFER_SIZE, Message );                    // Define message memory pool
static osPoolDef( cmd_pool, COMMAND_BUFFER_SIZE, Message );                    // Define message memory pool

static osPoolId  msg_pool;
static osPoolId  cmd_pool;

static osMessageQDef( MsgBox, MESSAGE_BUFFER_SIZE, &Message );              // Define message queue
static osMessageQDef( CmdBox, COMMAND_BUFFER_SIZE, &Command );              // Define message queue

static osMessageQId  MsgBox;
static osMessageQId  CmdBox;


void recv_message( const void* _app )
{
	Application* app = (Application*) app;
   Message  *rptr;
   osEvent  evt;
   
   for (;;)
	{
		evt = osMessageGet( MsgBox, osWaitForever);  // wait for message
		
		if ( evt.status == osEventMessage )
		{
			rptr = (Message*) evt.value.p;
			
			for ( Module** iter = app->module_list; iter < app->module_list_pos; iter++ )
			{	
				if ( *iter == rptr->to )
				{
					(*iter)->Receive( rptr );
				}
			}
				
			osPoolFree( msg_pool, rptr );                  // free memory allocated for message
      }
   }
}


void recv_command( const void* _app )
{
	Application* app = (Application*) app;
   Command* rptr;
   osEvent  evt;
   
   for (;;)
	{
		evt = osMessageGet( CmdBox, osWaitForever );  // wait for message
		
		if ( evt.status == osEventMessage )
		{
			rptr = (Command*) evt.value.p;
			
			for ( Module** iter = app->module_list; iter < app->module_list_pos; iter++ )
			{	
				if ( (*iter)->Receive( rptr ) == MODULE_CMD_ISSUED )
				{
					osPoolFree( msg_pool, rptr );                  // free memory allocated for message
					return;
				}
			}
			Log( LOG_UNKNOWN_CMD );
			osPoolFree( msg_pool, rptr );                  // free memory allocated for message
      }
   }
} 

static osThreadDef( low_priority, osPriorityLow, MAX_MODULES, 0 );
static osThreadDef( normal_priority, osPriorityNormal, MAX_MODULES, 0 );
static osThreadDef( high_priority, osPriorityHigh, MAX_MODULES, 0 );
static osThreadDef( realtime_priority, osPriorityRealtime, MAX_MODULES, 0 );

//static osThreadDef( recv_message, osPriorityRealtime, MAX_MODULES, 0 );
static osThreadDef( recv_command, osPriorityNormal, 1, 0 );





void Application::Go( )
{
	msg_pool = osPoolCreate( osPool( msg_pool) );                 // create message memory pool
	cmd_pool = osPoolCreate( osPool( cmd_pool) );                 // create command memory pool
	
	MsgBox = osMessageCreate( osMessageQ(MsgBox), NULL );  		// create msg queue
	CmdBox = osMessageCreate( osMessageQ(CmdBox), NULL );  		// create msg queue
	
	osThreadCreate( osThread( recv_command ), this );
	
	for ( Module** iter = module_list; iter < module_list_pos; iter++ )
	{	
		int p = (*iter)->priority;
		
		if ( p != 0 )
		{
			if ( p == 1 )
			{
				if ( !osThreadCreate( osThread( high_priority ), *iter ) )
				{
					return;
				}
			}
			else if ( p <= 5 )
			{
				if ( !osThreadCreate( osThread( high_priority ), *iter ) )
				{
					return;
				}
			}
			else if ( p <= 10 )
			{
				if ( !osThreadCreate( osThread( normal_priority ), *iter ) )
				{
					return;
				}
			}
			else 
			{
				if ( !osThreadCreate( osThread( low_priority ), *iter ) )
				{
					return;
				}
			}	
		}
	}
	
	
//	osThreadCreate( osThread( recv_message ), NULL );

	/* Dont create new thread - just use main */ 
	recv_message( this );
	//	osThreadTerminate( osThreadGetId( ) );
}

	
void Application::Post( Message* msg )
{
	Message* mptr = (Message*) osPoolAlloc( msg_pool );                     // Allocate memory for the message
	memcpy( mptr, msg, sizeof(Message) );

   osMessagePut( MsgBox, (uint32_t)mptr, osWaitForever );  // Send Message
}


void Application::Post( Command* cmd )
{
	Command* cptr = (Command*) osPoolAlloc( cmd_pool );                     // Allocate memory for the message
	memcpy( cptr, cmd, sizeof(Command) );

   osMessagePut( CmdBox, (uint32_t)cptr, osWaitForever );  // Send Message
}

#else

void Application::Go( )
{
	Message* msg;
	Command* cmd;
	Module** iter = module_list;
	
	while ( 1 )
	{
		
		// Check message queue
		if ( message_queue.size != 0 )
			msg = &message_queue.Pop( );
		
		// Check command queue
		if ( command_queue.size != 0 )
			cmd = &command_queue.Pop( );
				
		// Check each module
		for ( iter = module_list; iter < module_list_pos; iter++ )
		{
			if ( (*iter)->priority != 0 )
			{	
				if ( loop_count % (*iter)->priority == 0 )
				{
					(*iter)->Poll( );
				}
			}
			if ( msg != NULL )
			{
				if ( msg->to == *iter ) 
				{
					(*iter)->Receive( msg );
					msg = NULL;
				}
			}
			if ( cmd != NULL )
			{
				if ( (*iter)->Receive( cmd ) == MODULE_CMD_ISSUED )
				{
					cmd = NULL;
				}
			}
		}
		if ( cmd != NULL )
		{
			Log( LOG_UNKNOWN_CMD );
		}
		if ( msg != NULL )
		{
			Log( LOG_UNKNOWN_MODULE );
		}	
	}
}



void Application::Post( Message* msg )
{
	message_queue.Push( *msg );
}



void Application::Post( Command* cmd )
{
	command_queue.Push( *cmd );
}

#endif
		
void Application::ReportFault( int )
{

}


void Application::Log( int )
{

}	


void Application::RegisterModule( Module* m )
{

	*module_list_pos++ = m;
}	
	

Module* Application::FindModule( char* name )
{	
	for( Module** iter = module_list; iter < module_list_pos; iter++ )
	{
		if ( !strcmp( (*iter)->name, name ))
		{
			return *iter;
		}
	}
	return NULL;
}

