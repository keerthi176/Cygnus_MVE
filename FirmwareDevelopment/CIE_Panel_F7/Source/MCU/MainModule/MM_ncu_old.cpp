/***************************************************************************
* File name: NCU.cpp
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
#include <assert.h>
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_uart.h"


/* Defines
**************************************************************************/
#define NCU_MAX_FAILS 5000000


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_NCU.h"
#include "MM_VirtualTimer.h"

#include "MM_fault.h"
#include "MM_log.h"


/*************************************************************************/
/**  \fn      void cleared( Module* m, int success )
***  \brief   Global helper function
**************************************************************************/

void cleared( Module* m, int success )
{
}

/*************************************************************************/
/**  \fn      NCU::NCU( Application* app )
***  \brief   Constructor for class
**************************************************************************/

NCU::NCU( Application* app ) : Module( "NCU", 10, app, EVENT_SECOND )
{	
	currentState = targetState = NCU_STATE_PRE_AT;

	VirtualTimer* vmanager = ( VirtualTimer*) app->FindModule( "VirtualTimer" );
	
	sendqueue    = new Circular<NCUQuery>( 16 );
	delayedqueue = new Circular<NCUQuery>( 16 );
	
	SetState( NCU_STATE_ACTIVE );
	
	int a=1, b=2, c=3, d=4, e=5, f=6, g=7;
	
	QueueMsg( NCU_CLEAR_QUEUE, cleared, a, b, c, d, e, f, g );
	
	NCUQuery* q = sendqueue->Pop( );
	
	Execute( q );
} 

 
/*************************************************************************/
/**  \fn      void NCU::SetState( NCUState state )
***  \brief   Class function
**************************************************************************/

void NCU::SetState( NCUState state )
{
	//targetState = state;
}


/*************************************************************************/
/**  \fn      int NCU::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int NCU::Receive( Event e )
{/*
	// if e == EVENT_SECOND
	
	if ( msgOutType == NCU_NONE )
	{
		if ( currentState == NCU_STATE_PRE_AT )
		{
			msgOutType = NCU_PLUS_PLUS_PLUS;
		}		
		else if ( targetState != currentState )
		{
			msgOutType = NCU_GET_FORMATION_STATE;
		}
		else if ( currentState == NCU_STATE_ACTIVE )
		{
			msgOutType = NCU_MSG_STATUS;
		}
	}
			*/
	return 0;
}


/*************************************************************************/
/**  \fn      int NCU::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int NCU::Receive( Command* cmd )
{
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int NCU::Receive( Message* msg )
***  \brief   Class function
**************************************************************************/

int NCU::Receive( Message* msg )
{
	return MODULE_MSG_UNKNOWN;
}

/*
/*************************************************************************/
/**  \fn      void NCU::AT( const char* tx )
***  \brief   Class function
**************************************************************************/

void NCU::AT( const char* tx )
{
	HAL_UART_Abort( app->ncu_uart );
	
	app->DebOut( "Tx: %s", tx );
	
	HAL_UART_Transmit_DMA( app->ncu_uart, (uint8_t*) tx, strlen( tx ) );

//	msgInType = msgOutType;
	
	HAL_UART_Receive_DMA( app->ncu_uart, (uint8_t*) rxbuffer, NCU_RX_BUFFER_SIZE );
	
//	msgAwaiting = true;
//	rx_fail_count = 0;
}	

 
/*************************************************************************/
/**  \fn      void NCU::DecodeFormationState( )
***  \brief   Class function
**************************************************************************/

void NCU::DecodeFormationState( )
{
	if ( !strncmp( "MFS:", rxbuffer, 4 ) )
	{
		sscanf( rxbuffer, "MFS: %d\r\n", &currentState );
		
		if ( currentState < targetState )
		{
			switch ( currentState )
			{
				case NCU_STATE_IDLE : msgOutType = NCU_SET_SYNC_STAGE;
					break;
				case NCU_STATE_SYNC : msgOutType = NCU_SET_FORMATION_STAGE;
					break;
				case NCU_STATE_FORMATION : msgOutType = NCU_SET_ACTIVE_STAGE;
					break;
				case NCU_STATE_ACTIVE : msgOutType = NCU_SET_TEST_MODE;
					break;
			}
		}
		else if ( currentState > targetState )
		{
			switch ( currentState )
			{
				case NCU_STATE_SYNC : msgOutType = NCU_SET_IDLE_STATE;
					break;
				case NCU_STATE_FORMATION : msgOutType = NCU_SET_SYNC_STAGE;
					break;
				case NCU_STATE_ACTIVE : msgOutType = NCU_SET_FORMATION_STAGE;
					break;
				case NCU_STATE_TEST_MODE : msgOutType = NCU_SET_ACTIVE_STAGE;
			}
		}
		else msgOutType = NCU_NONE;
	}
}	


/*************************************************************************/
/**  \fn      int NCU::Decode( char* fmt, NCUMessage next, ... )
***  \brief   Class function
**************************************************************************/

int NCU::Decode( char* fmt, NCUMessage next, ... )
{
	if ( !strncmp( fmt, rxbuffer, 4 ) )
	{
		va_list args;
		va_start( args, next );
		vsscanf( str, fmt, args );
		va_end( args );
		
		msgOutType = next;
		
		return true;
	}
}
		
 
/*************************************************************************/
/**  \fn      void NCU::DecodeMsgStatus( )
***  \brief   Class function
**************************************************************************/

void NCU::DecodeMsgStatus( )
{
	int Tx_Buffer;
	
	if ( !strncmp( "MSN:", rxbuffer, 4 ) )
	{
		sscanf( rxbuffer, "MSN: %d,%d\n%d,%d\n%d,%d\n%d,%d\n%d\r\n", &Fire_Msg, &Fire_Lost, &Alarm_Msg, &Alarm_Lost, &Fault_Msg, &Fault_Lost, &Misc_Msg, &Misc_Lost, &Tx_Buffer );
		
		if ( Tx_Buffer >= NCU_TX_MSG_SIZE ) app->Log( LOG_NCU_BUFFER_FULL );
		
		SetQueueMessage( );
	}
}


/*************************************************************************/
/**  \fn      void NCU::DecodeNumDevices( )
***  \brief   Class function
**************************************************************************/

void NCU::DecodeNumDevices( )
{	
	int Zones, Devices;

	if ( !strncmp( "NOD:", rxbuffer, 4 ) )
	{
		sscanf( rxbuffer, "NOD: %d,%d\r\n", &Zones, &Devices );
		
		SetQueueMessage( );
	}
}


/*************************************************************************/
/**  \fn      void NCU::DecodeFireMsg( )
***  \brief   Class function
**************************************************************************/

void NCU::DecodeFireMsg( )
{
	int unit, zone, RU_Channel_Index, Value;
	
	if ( !strncmp( "QFE:", rxbuffer, 4 ) )
	{
		sscanf( rxbuffer, "QFE: Z%dU%d,%d,%d\r\n",  &zone, &unit, &RU_Channel_Index, &Value );
		
		Fire_Msg--;		
		msgOutType = NCU_REMOVE_FIRE;
	}	
}


/*************************************************************************/
/**  \fn      void NCU::DecodeItemMsg( const char* at, const char* format )
***  \brief   Class function
**************************************************************************/

void NCU::DecodeItemMsg( const char* at, const char* format )
{
	int unit, zone, RU_Channel_Index, Value;
	
	if ( !strncmp( at, rxbuffer, 4 ) )
	{
		sscanf( rxbuffer, format, &zone, &unit, &RU_Channel_Index, &Value );
		
		Alarm_Msg--;
		msgOutType = NCU_REMOVE_ALARM;
	}	
}


/*************************************************************************/
/**  \fn      void NCU::DecodeAlarmMsg( )
***  \brief   Class function
**************************************************************************/

void NCU::DecodeAlarmMsg( )
{
	int unit, zone, RU_Channel_Index, Value;
	
	if ( !strncmp( "QAM:", rxbuffer, 4 ) )
	{
		sscanf( rxbuffer, "QAM: Z%dU%d,%d,%d\r\n", &zone, &unit, &RU_Channel_Index, &Value );
		
		Alarm_Msg--;
		msgOutType = NCU_REMOVE_ALARM;
	}	
}


/*************************************************************************/
/**  \fn      void NCU::DecodeFaultMsg( )
***  \brief   Class function
**************************************************************************/

void NCU::DecodeFaultMsg( )
{
	int unit, zone, RU_Channel_Index, Value;
	
	if ( !strncmp( "QFT:", rxbuffer, 4 ) )
	{
		sscanf( rxbuffer, "QFT: Z%dU%d,%d,%d\r\n", &zone, &unit, &RU_Channel_Index, &Value );
		
		Fault_Msg--;
		msgOutType = NCU_REMOVE_FAULT;
	}	
}


/*************************************************************************/
/**  \fn      void NCU::DecodeMiscMsg( )
***  \brief   Class function
**************************************************************************/

void NCU::DecodeMiscMsg( )
{
	int unit, zone, RU_Channel_Index, Value;
	
	if ( !strncmp( "QMC: ", rxbuffer, 4 ) )
	{
		sscanf( rxbuffer, "QMC: Z%dU%d,%d,%d\r\n", &zone, &unit, &RU_Channel_Index, &Value );
		
		Misc_Msg--;
		msgOutType = NCU_REMOVE_MISC;
	}		
}
*/
/*
/*************************************************************************/
/**  \fn      void NCU::DecodeID( )
***  \brief   Class function
**************************************************************************/

void NCU::DecodeID( )
{
	if ( !strncmp( "BID: ", rxbuffer, 5 ) )
	{	
		sscanf( rxbuffer, "BID: %d	
/*************************************************************************/
/**  \fn      void NCU::DecodeFWVersion( )
***  \brief   Class function
**************************************************************************/

void NCU::DecodeFWVersion( );
/*************************************************************************/
/**  \fn      void NCU::DecodeSerialNum( )
***  \brief   Class function
**************************************************************************/

void NCU::DecodeSerialNum( );

			NCU_ID,
	NCU_FW_VERSION,
	NCU_SERIAL_NUM,
	NCU_CLEAR_QUEUE,

/*************************************************************************/
/**  \fn      int NCU::DecodeOK( const char* prefix )
***  \brief   Class function
**************************************************************************/

int NCU::DecodeOK( const char* prefix )
{
	int n = strlen( prefix );
	
	if ( !strncmp( prefix, rxbuffer, n ) )
	{
		if ( !strncmp( rxbuffer + n, "OK\r\n", 4 ) )
		{
			return true;
		}
	}
	return false;
}
	

/*************************************************************************/
/**  \fn      int NCU::SetQueueMessage( )
***  \brief   Class function
**************************************************************************/

int NCU::SetQueueMessage( )
{
	if ( Fire_Msg > 0 )
	{
		msgOutType = NCU_FIRE_MSG;
	}
	else if ( Alarm_Msg > 0 )
	{
		msgOutType = NCU_ALARM_MSG;
	}
	else if ( Fault_Msg > 0 )
	{
		msgOutType = NCU_FAULT_MSG;
	}
	else if ( Misc_Msg > 0 )
	{
		msgOutType = NCU_MISC_MSG;
	}
	else
	{
		msgOutType = NCU_NONE;
	}
	return 0;
}
*/

/*************************************************************************/
/**  \fn      int NCU::MsgReceivedOK( )
***  \brief   Class function
**************************************************************************/

int NCU::MsgReceivedOK( )
{
	if ( msgAwaiting )
	{	
		int bytes_rx = NCU_RX_BUFFER_SIZE - app->ncu_uart->hdmarx->Instance->NDTR;
		
		if ( bytes_rx > 3 )
		{			
			if ( rxbuffer[ bytes_rx - 2 ] == '\r' && rxbuffer[ bytes_rx - 1 ] == '\n' )
			{
				rxbuffer[ bytes_rx ] = 0;
				app->DebOut( "Rx: %s", rxbuffer );

				return true;
			}
		}

		if ( ++rx_fail_count > NCU_MAX_FAILS )
		{
			 // give up
			
			msgAwaiting->func( msgAwaiting->user, false );			
	 		msgAwaiting = NULL;  
		}
	}
	return false;
}

	/*
/*************************************************************************/
/**  \fn      void NCU::Poll( )
***  \brief   Class function
**************************************************************************/

void NCU::Poll( )
{
	if ( MsgReceived() ) 
	{
		switch ( msgInType )
		{
			case NCU_PLUS_PLUS_PLUS : 	if ( DecodeOK( "" ) )
												{
													currentState = NCU_STATE_IDLE;
													msgOutType = NCU_NONE;
												}
				break;
												
			case NCU_GET_FORMATION_STATE : DecodeFormationState( );
				break;
												
			case NCU_SET_SYNC_STAGE :			
			case NCU_SET_FORMATION_STAGE :
			case NCU_SET_ACTIVE_STAGE :
			case NCU_SET_TEST_MODE :
				if ( DecodeOK( "MFS: " ) ) msgOutType = NCU_NONE;
				break;
			
			case NCU_MSG_STATUS : 	
				if ( Decode( "MSN: %d,%d\n%d,%d\n%d,%d\n%d,%d\n%d\r\n", NCU_NONE,
						&Fire_Msg, &Fire_Lost, &Alarm_Msg, &Alarm_Lost, &Fault_Msg, 
										&Fault_Lost, &Misc_Msg, &Misc_Lost, &Tx_Buffer ) )
					SetQueueMessage( );
				break;
				
			case NCU_NUM_DEVICES : 	Decode( "NOD: %d,%d\r\n", NCU_NONE, &Zones, &Devices );
				break;
				
			case NCU_FIRE_MSG : 		if ( Decode( "QFE: Z%dU%d,%d,%d\r\n", NCU_REMOVE_FIRE, 
														&zone, &unit, &RU_Channel_Index, &Value ) )
												HandleFireMsg( );	
											break;
			
			case NCU_ALARM_MSG : 	if ( Decode( "QAM: Z%dU%d,%d,%d\r\n", NCU_REMOVE_ALARM,
														&zone, &unit, &RU_Channel_Index, &Value ) )
												HandleAlarmMsg( );
											break;
			
			case NCU_FAULT_MSG : 	if ( Decode( "QFT: Z%dU%d,%d,%d\r\n", NCU_REMOVE_FAULT,
														&zone, &unit, &RU_Channel_Index, &Value ) )
												HandleFaultMsg( );
											break;
 
			case NCU_MISC_MSG : 		HandleMiscMsg( );
											break;
			 
			case NCU_REMOVE_FIRE :  if ( DecodeOK( "XFE: " ) ) SetQueueMessage( );
				break;
			
			case NCU_REMOVE_FAULT : if ( DecodeOK( "XFT: " ) ) SetQueueMessage( );
				break;
			
			case NCU_REMOVE_MISC :  if ( DecodeOK( "XMC: " ) ) SetQueueMessage( );
				break;
			
			case NCU_REMOVE_ALARM :	if ( DecodeOK( "XAM: " ) ) SetQueueMessage( );
				break;
			
			case NCU_CLEAR_QUEUE :	if ( DecodeOK( "XMQ: " ) ) msgOutType = NCU_NONE;
				break;
				
			case NCU_FW_VERSION : Decode( "FI: %d", NCU_NONE, &brandID );
				break;
			
			case NCU_SERIAL_NUM : Decode( "SERNO: %d", NCU_NONE, &serialNum );
				break;	
				
			case NCU_ID : Decode( "BID: %d", NCU_NONE, &brandID );
				break;	
				
			case NCU_NONE:				
				break;
		}
	}
	
	if ( !msgAwaiting )
	{
		switch ( msgOutType )
		{
			case NCU_PLUS_PLUS_PLUS : AT( "+++\r\n" );
				break;
			case NCU_GET_FORMATION_STATE : AT( "ATMFS?\r\n" );
				break;
			case NCU_SET_SYNC_STAGE : AT( "ATMFS=0\r\n" );
				break;
			case NCU_SET_FORMATION_STAGE : AT( "ATMFS=1\r\n" );
				break;
			case NCU_SET_ACTIVE_STAGE : AT( "ATMFS=2\r\n" );
				break;
			case NCU_SET_TEST_MODE : AT( "ATMFS=3\r\n" );
				break;
			case NCU_MSG_STATUS : 	AT( "ATMSN?\r\n" );
				break;
			case NCU_NUM_DEVICES :  AT( "ATNOD?\r\n" );
				break;
			case NCU_FIRE_MSG : 		AT( "ATQFE?\r\n" );
				break;
			case NCU_ALARM_MSG : 	AT( "ATQAM?\r\n" );
				break;
			case NCU_FAULT_MSG : 	AT( "ATQFT?\r\n" );
				break;
			case NCU_MISC_MSG : 		AT( "ATQMC?\r\n" );
				break;
			case NCU_REMOVE_FIRE :  AT( "ATXFE+\r\n" );
				break;
			case NCU_REMOVE_FAULT :	AT( "ATXFT+\r\n" );
				break;
			case NCU_REMOVE_MISC :	AT( "ATXMC+\r\n" );
				break;	
			case NCU_REMOVE_ALARM :	AT( "ATXAM+\r\n" );
				break;
			case NCU_CLEAR_QUEUE :	AT( "ATXMQ=1,1\n1,1\n1,1\n1,1\n1\r\n" ); 
				break;
			case NCU_FW_VERSION : 	AT( "ATFI?\r\n" );
				break;		
			case NCU_SERIAL_NUM : 	AT( "ATSERNO?\r\n" );
				break;	
			case NCU_ID : 				AT( "ATBID?\r\n" );
				break;	
			
			case 

			case NCU_NONE:
				break;
		}		
	}
}	
 */
	
enum
{
	READ = 1,
	WRITE,
	MISC,
	DELAYED,
	PLUSOK,
	AUTO,
};


int NCU::unused = 0xDEADBABE;


static NCUMsgFormat formatlist[ ] =
{
	{ NCU_PLUS_PLUS_PLUS, 		"+++\r\n", PLUSOK  },
	{ NCU_GET_STAGE, 				"ATMFS?\r\n", READ, "MFS: %d\r\n" },
	{ NCU_SET_STAGE,				"ATMFS=%d\r\n", WRITE },
	{ NCU_MSG_STATUS, 			"ATMSN?\r\n", READ, "MSN: %d,%d\n%d,%d\n%d,%d\n%d,%d\n%d\r\n" },
	{ NCU_NUM_DEVICES, 			"ATNOD?\r\n", READ, "NOD: %d,%d\r\n" },
	{ NCU_FIRE_MSG, 				"ATQFE?\r\n", READ, "QFE: Z%dU%d,%d,%d\r\n" },
	{ NCU_ALARM_MSG,				"ATQAM?\r\n", READ, "QAM: Z%dU%d,%d,%d\r\n" },
	{ NCU_FAULT_MSG, 				"ATQFT?\r\n", READ, "QFT: Z%dU%d,%d,%d\r\n" },
	{ NCU_MISC_MSG,  				"ATQMC?\r\n", MISC },
	{ NCU_REMOVE_FIRE, 			"ATXFE+\r\n" },
	{ NCU_REMOVE_FAULT,			"ATXFT+\r\n" },
	{ NCU_REMOVE_MISC,	 		"ATXMC+\r\n" },
	{ NCU_REMOVE_ALARM,			"ATXAM+\r\n" },
	{ NCU_CLEAR_QUEUE,			"ATXMQ=%d,%d\n%d,%d\n%d,%d\n%d,%d\n%d\r\n", WRITE }, 
	{ NCU_FW_VERSION, 			"ATFI?\r\n", READ, "FI: %d.%d.%d,%d/%d/%d\r\n" },
	{ NCU_SERIAL_NUM,  			"ATSERNO?\r\n", READ, "SERNO: %04d-%02d-%05d\r\n" },
	{ NCU_ID, 						"ATBID?\r\n", READ, "BID: %d\r\n" },
   
	 		
	{ NCU_READ_ANALOGUE, 		"ATANA?Z%dU%d,%d\r\n", DELAYED, "ANA: Z%dU%d,%d,0,1,%d" },
	{ NCU_SET_FLASH_RATE, 		"ATBFR=Z%dU%d,%d,%d,%d\r\n", WRITE }, 
	{ NCU_GET_FLASH_RATE, 		"ATBFR?Z%dU%d,%d,%d,%d\r\n", DELAYED, "BFR: Z%dU%d,%d,0,1,%d" },
	{ NCU_GET_COMBINATION, 		"ATDEC?Z%dU%d\r\n", DELAYED, "DEC: Z%dU%d,0,0,1,%d" }, 
	{ NCU_GET_NEIGHBOUR,  		"????",  DELAYED },
	{ NCU_GET_FLAGS,   			"ATDSF?Z%dU%d\r\n",  DELAYED, "DSF: Z%dU%d,0,0,1,%d" }, 
	{ NCU_ENABLE_PLUGIN,   		"ATEDP=Z%dU%d,%d,%d\r\n", WRITE }, 
	{ NCU_CHECK_DEVICE,  		"ATEDP?Z%dU%d,%d\r\n", DELAYED, "EDP: Z%dU%d,%d,0,1,%d" },
	{ NCU_ENABLE_RADIO,			"????", WRITE },
	{ NCU_GET_RADIO_FW,			"ATFIR?Z%dU%d\r\n", DELAYED, "FIR: Z%dU%d,0,0,2,%d.%d.%d,%d/%d/%d" },
	{ NCU_GET_PLUGIN_FW,			"ATFIP?Z%dU%d\r\n", DELAYED, "FIP: Z%dU%d,0,0,2,%d.%d.%d,%d/%d/%d" },
	{ NCU_GET_MESH_STATUS,		"ATMSR?Z%dU%d\r\n", DELAYED, "MSR: Z%dU%d,0,0,8,%d,%d,%d,%d,%d,%d,%d,%d" },
	{ NCU_ACTIVATE,				"ATOUT=Z%dU%d,%d,%d\r\n", WRITE },
	{ NCU_REBOOT_RADIO,			"ATRRU=Z%dU%d\r\n", WRITE },
	{ NCU_PLUGIN_SERIAL_NUM,	"ATSNP?Z%dU%d\r\n", DELAYED, "SNP: Z%dU%d,0,0,1,%02d%02-%d-%d" },
	{ NCU_RADIO_SERIAL_NUM, 	"ATSNR?Z%dU%d\r\n", DELAYED, "SNR: Z%dU%d,0,0,1,%02d%02-%d-%d" },
	{ NCU_SET_TONE,				"ATSTS=Z%dU%d,%d,%d,%d\r\n", WRITE },
	{ NCU_GET_TONE,				"ATSTS?Z%dU%d,%d,%d\r\n", DELAYED, "STS: Z%dU%d,%d,%d,1,%d\r\n" },
	{ NCU_SET_UPPER,				"ATTHU=Z%dU%d,%d,%d\r\n", WRITE },
	{ NCU_GET_UPPER,				"ATTHU?Z%dU%d,%d\r\n", DELAYED, "THU: Z%dU%d,%d,0,1,%d\r\n" },
	{ NCU_SET_LOWER,       		"ATTHL=Z%dU%d,%d,%d\r\n", WRITE },
	{ NCU_GET_LOWER,				"ATTHL?Z%dU%d,%d\r\n", DELAYED, "THL: Z%dU%d,%d,0,1,%d\r\n" },
	{ NCU_PLUGIN_TEST_MODE,		"ATTMP=Z%dU%d,%d,%d", WRITE },
	{ NCU_CHECK_PLUGIN_MODE,	"ATTMP?Z%dU%d,%d", DELAYED, "TMP: Z%dU%d,%d,0,1,%d\r\n" },
	{ NCU_RADIO_LOGON,			NULL,	AUTO, "LOG: Z%dU%d,0,0,3,02%d02%d-%d-%d,%d,%d\r\n" },
	{ NCU_STATUS,					NULL,	AUTO, "MSI: Z%dU%d,0,0,8,%d,%d,%d,%d,%d,%d,%d,%d" },
};				  
	

/*************************************************************************/
/**  \fn      static int countargs( const char* s )
***  \brief   Local helper function
**************************************************************************/

static int countargs( const char* s )
{
	const char* pos = s - 2;
	int count = 0;
	
	while ( pos = strstr( pos + 2, "%d" ) ) count++;
	
	return count;
}


/*************************************************************************/
/**  \fn      int NCU::DecodeOK( NCUQuery* q )
***  \brief   Class function
**************************************************************************/

int NCU::DecodeOK( NCUQuery* q )
{
	if ( strcmp( rxbuffer, q->fmt->reply, 4 ) )
	{
		va_list args;
		
		args.__ap = (void*) q->arg;

		vsscanf( rxbuffer, q->fmt->reply, args );	
		
		return true;
	}
	return false;
}
		

/*************************************************************************/
/**  \fn      void NCU::QueueMsg( NCUMessage msg, Module* m, NCUCallback* func, int& a, int& b, int& c, int& d, int& e, int& f,
																		int& g, int& h, int& i, int& j, int& k )
***  \brief   Class function
**************************************************************************/

void NCU::QueueMsg( NCUMessage msg, Module* m, NCUCallback* func, int& a, int& b, int& c, int& d, int& e, int& f,
																		int& g, int& h, int& i, int& j, int& k )
{
	NCUQuery q;
	
	q.fmt  = formatlist + (int)msg;
	q.func =	func;
	q.user = m;
	
	assert( q.fmt->msg == msg );
	assert( q.fmt->query != NULL );
	
	int query_args = countargs( q.fmt->query );	
	int reply_args = countargs( q.fmt->reply );
	
	q.args = MAX( query_args, reply_args );
	
	// Not all on stack so copy indvidually..
	
	q.arg[ 0] = &a;
	q.arg[ 1] = &b;
	q.arg[ 2] = &c;
	q.arg[ 3] = &d;
	q.arg[ 4] = &e;
	q.arg[ 5] = &f;
	q.arg[ 6] = &g;
	q.arg[ 7] = &h;
	q.arg[ 8] = &i;
	q.arg[ 9] = &j;
	q.arg[10] = &k;
	
	sendqueue->Push( q );
}


/*************************************************************************/
/**  \fn      void NCU::Execute( NCUQuery* q )
***  \brief   Class function
**************************************************************************/

void NCU::Execute( NCUQuery* q )
{
	char buff[ 200 ];
	int arg[ MAX_MSG_ARGS ];
	
	for( int n = 0; n < q->args; n++ )
	{
		arg[ n] = *q->arg[ n];
		assert( arg[ n] != unused );
	}
	
	va_list args;
	
	args.__ap = (void*) arg;

	vsprintf( buff, q->fmt->query, args );
	
	// Cancel previous transactions
	
	HAL_UART_Abort( app->ncu_uart );
	
	app->DebOut( "Tx: %s", buff );
	
	HAL_UART_Transmit_DMA( app->ncu_uart, (uint8_t*) buff, strlen( buff ) );
	
	HAL_UART_Receive_DMA( app->ncu_uart, (uint8_t*) rxbuffer, NCU_RX_BUFFER_SIZE );
}


/*************************************************************************/
/**  \fn      void NCU::Poll( )
***  \brief   Class function
**************************************************************************/

void NCU::Poll( )
{ 
	// Are we waiting on a reply?
	if ( msgAwaiting != NULL )
	{
		NCUMsgFormat* fmt = msgAwaiting->fmt;
	
		// If msg received?
		if ( MsgReceivedOK( ) ) switch ( msgAwaiting->fmt->type )
		{
			case PLUSOK : 
			{
				msgAwaiting->func( msgAwaiting->user, !strcmp( rxbuffer, "OK\r\n" ) );
			}
			break;
			
			case WRITE :
			{
				int success =  !strncmp( rxbuffer, fmt->query + 2, 3 ) ) && 
									!strncmp( rxbuffer + 3, ": OK", 4 )
				
				msgAwaiting->func( msgAwaiting->user, success );
			}
			break;
			
			case MISC :
			{
				if ( !strncmp( rxbuffer, formatlist[ NCU_RADIO_LOGON ].reply, 5 ) )
				{
					HandleLogon( );
				}
				else if ( !strncmp( rxbuffer, formatlist[ NCU_STATUS ].reply, 5 ) )
				{
					HandleStatus( );
				}
				else while ( ! delayedqueue->Empty( ) )
				{
					NCUQuery* q = delayedqueue->Pop( );
					
					int ok = DecodeOK( q );
					
					q->func( q->user, ok );
					
					if ( ok ) break;
				}
			}
			break;
			
			case READ :
			{
				msgAwaiting->func( msgAwaiting->user, DecodeOK( msgAwaiting ) );
			}
			break;			

		}
	}
	else 
	{
		if ( ! sendqueue->Empty( ) )
		{
			msgAwaiting = sendqueue->Pop( );
			
			Execute( msgAwaiting );

			if ( msgAwaiting->fmt->type == DELAYED )
			{
				delayedqueue->Push( *msgAwaiting );
			}
		}
	}
}
				