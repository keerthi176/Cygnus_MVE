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
* First written on 18/05/19 by Matt Ault
*
* File History:
* Change History:
* Rev Date Description Author
* 1.0 18/05/18 Initial File
*
* Description:
* Panel Networking
*
**************************************************************************/


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
#include <string.h>
#include <stdio.h>



/* User Include Files
**************************************************************************/

#include "MM_PanelNetwork.h"
#include "CO_Site.h"
#include "MM_Events.h"
#include "MM_EmbeddedWizard.h"
#include "MM_IOStatus.h"
#include "MM_CUtils.h"
#include "MM_Fault.h" 
 
 
#include "MM_NCU.h"

	


/* Defines
**************************************************************************/
#define PANEL_NET_MAX_COMS_GAP 	20
#define PANEL_NET_RAND_MAX		 	5000
#define PANEL_NET_RAND_MIN			1000
#define PANEL_NET_MAX_NODES  		6
#define PANEL_NET_PING_PERIOD		1000
#define PANEL_NET_RXPING_SIZE		sizeof(NetPing)
#define PANEL_NET_TXPING_SIZE		sizeof(NetPing)
#define PANEL_NET_START_BYTE		0x55
#define PANEL_NET_MAX_FAILS		10
#define PANEL_NET_MAX_RX_FAILS	30

#define MAXUINT( max,a) { max = 0; for(int i = 0; i < sizeof(a)/sizeof(*a); i++ ) if ( a[i] > max ) max = a[ i];}

	
/* Globals
**************************************************************************/
uint32_t PanelNetwork::mesh_count[ SITE_MAX_PANELS ];
	
static int ignore_next_round = false;
	
/*************************************************************************/
/**  \fn      void NetPing::Send( UART_HandleTypeDef* uart, GPIO de )
***  \brief   Class function
**************************************************************************/

void NetPing::Send( UART_HandleTypeDef* uart, GPIO de )
{
		// switch to transmit
	GPIOManager::Set( de, IO_PIN_SET ); 
	
	// stop receiveing
	HAL_UART_AbortReceive( uart );
	
	// stop receiveing
	HAL_UART_AbortTransmit( uart );
	 
	// Start transmit
	HAL_UART_Transmit_DMA( uart, (uint8_t*) this, PANEL_NET_TXPING_SIZE ); 			
}


/*************************************************************************/
/**  \fn      unsigned short NetPing::GetSum( )
***  \brief   Class function
**************************************************************************/

unsigned short NetPing::GetSum( )
{
	int total = 0;

#if 0
	total ^= sender;
	total ^= startbyte;
	
	for( int n = 0; n < SITE_MAX_ZONES / 32; n++ )
	{
		for( int a = 0; a < SITE_NUM_OUTPUT_ACTIONS; a++ )
		{
			total ^= zones[ a][ n];
		}
	}
#else 

	for( short* s = (short*) this; s < (short*) ( (char*) this + PANEL_NET_TXPING_SIZE ); s++ )
	{
		if ( s != (short*) &checksum )
		{
			total += *s;
		}
	}
#endif	
	
	return total;
}


/*************************************************************************/
/**  \fn      void Encrypt( uint32_t key )
***  \brief   Class function
**************************************************************************/

void NetPing::Crypt( uint32_t key )
{
 	for( int* w = (int*) this; w <= (int*)((char*)this + sizeof(this)) - 1; w++ )
 	{
 		*w ^= key;
 	}		
}


bool 					PanelNetwork::got_packet;
bool 					PanelNetwork::sent_packet;

static PanelNetwork* me;		// static access hack

PanelNetwork::PanelNetwork( ) : Module( "PanelNetwork", 1, EVENT_CLEAR | EVENT_RESET ), TimerHandler( app.network_timer ),
																														rxfails( 0 ), net_uart( &huart6 ), net_timer( app.network_timer ),
																														 data_enable( GPIO_UART6_DE ), iostatus( NULL ), settings( NULL ) 
{
	
}


/*************************************************************************/
/**  \fn      int  PanelNetwork::Init( )
***  \brief   Class function
**************************************************************************/

int PanelNetwork::Init( )
{
	settings = (Settings*) app.FindModule( "Settings" );
	iostatus = (IOStatus*) app.FindModule( "IOStatus" );
	ew		   = (EmbeddedWizard*) app.FindModule( "EmbeddedWizard" );
	
	toSend = 0;
	sent_packet = false;
	got_packet = false;
	
	assert( settings != NULL );
	assert( iostatus != NULL );
	assert( ew != NULL );
	
	Receive( EVENT_CLEAR );
	
	Restart( rand( ) & 65535 );
	
	net_uart->TxCpltCallback = UartTxComplete;
	net_uart->RxCpltCallback = UartRxComplete;
	net_uart->ErrorCallback = UartError;
	
	me = this;
	
	return true;
}


/*************************************************************************/
/**  \fn      int  PanelNetwork::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int PanelNetwork::Receive( Event e )
{
	if ( e == EVENT_CLEAR )
	{
		memset( ping_count, 0, sizeof( ping_count ) );
		memset( prev_ping_count, 0xFF, sizeof( prev_ping_count ) );
		memset( last_ping, 0, sizeof( last_ping) );
		
		got_packet = false;
		
		for( int p = 0 ; p < SITE_MAX_PANELS; p++ )
		{
			current_fault[ p] = false;
			
		}
	}
	else if ( e == EVENT_FIRE_CLEAR )
	{
		 
	}
	else if ( e == EVENT_RESET )
	{
		if ( app.site != NULL && app.panel != NULL )
		{		
			for( int i = 0 ; i < app.site->numPanels; i++ )
			{
				snprintf( fault_msg[ i ], FAULT_MSG_LEN, "Panel '%s' has one or more faults", app.site->panels[ i ].name );
				snprintf( fault_rx_msg[ i ], FAULT_MSG_LEN, "Nothing received from panel '%s'.", app.site->panels[ i ].name );
			}
		}
	}
	 
	return 0;
}



/*************************************************************************/
/**  \fn      int PanelNetwork::Receive( Command* )
***  \brief   Class function
**************************************************************************/

int PanelNetwork::Receive( Command* cmd )
{
	switch ( cmd->type )
	{
		case CMD_NETWORK:
		{
			toSend |= ( 1 << cmd->int0 );
			ignore_next_round = true;
			
			return CMD_OK;
		}
		case CMD_GET_NET_STATS:
		{
			if ( cmd->int0 < SITE_MAX_PANELS )
			{
				cmd->int1 = ping_count[ cmd->int0 ];
				cmd->int2 = now( ) - last_ping[ cmd->int0 ];
				 				
				return CMD_OK;
			}
			else return CMD_ERR_OUT_OF_RANGE;
		}
	}
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int PanelNetwork::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int PanelNetwork::Receive( Message* )
{
	return 0;
}
	


/*************************************************************************/
/**  \fn      int PanelNetwork::Receive( Message* )
***  \brief   Class function
**************************************************************************/

void PanelNetwork::UartTxComplete( UART_HandleTypeDef* uart )
{
	 
	// flag tx finished
	sent_packet = true;
	
	// End transmit
	GPIOManager::Set( GPIO_UART6_DE, IO_PIN_RESET );
}


/*************************************************************************/
/**  \fn      int PanelNetwork::CheckPacket(  )
***  \brief   Class function
**************************************************************************/

NetPing* PanelNetwork::CheckPacket( int key )
{
	NetPing* ping = &rxping;
		
	if ( rxping.startbyte != PANEL_NET_START_BYTE )
	{
		ping = (NetPing*) (((char*) &rxping) + 1);
		
		if ( ping->startbyte != PANEL_NET_START_BYTE )
		{
			return NULL;
		}
		app.DebOut("******    Bad net packet\n" );
		//char* p = (char*) &rxping;
		//app.DebOut("******    wrong ? %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\n", p[0],p[1],p[2],p[3],p[4],p[5],p[6],p[7],p[8],p[9],p[10],p[11]  );
		//return false;
	}
	
		// Decrypt
	//ping->Crypt( key );
	
	// Checksum
	else if ( ping->GetSum( ) ^ ping->checksum )
	{
		app.DebOut("******    chksumfail " );
		
		return NULL;
	}
 	else if ( ping->systemID != key )
 	{
 		app.DebOut("******    not for us! %X!=%X\n", key, ping->systemID );
		
	 	return NULL;
 	}	
	else if ( ping->sender >= app.site->numPanels )
	{
		app.DebOut("******    panel index out of range " );
		
		return NULL;
	}
	 	
	//app.DebOut("******    Sender = %d  &%X\n", ping->sender, ping->systemID );
	return ping;	
}

const char* GetActionString( int action );


int alertcmp( TimedDeviceRef* a, TimedDeviceRef* b, int num )
{
	for( int i = 0; i < num; i++ )
	{
		if ( (*(int*)a | ( 1 << 23 )) != (*(int*)b | ( 1 << 23 )) )
		{
			return 0;
		}
	 
		if ( a->timestamp != b->timestamp )
		{
			return 0;
		}
		a++;
		b++;
	}
	return 1;
}


/*************************************************************************/
/**  \fn      void PanelNetwork::GetAlertListStatus( NetPing* ping )
***  \brief   Class function
**************************************************************************/

void PanelNetwork::GetAlertListFromPing( NetPing* ping )
{
	static uint16_t prev_actionbits[ SITE_NUM_OUTPUT_ACTIONS ] = {0};
	int _now = now( );
	
	if ( app.panel != NULL )
	{	
		TimedDeviceRef tmplist[ MAX_NET_REFS ];
		TimedDeviceRef* tmp = tmplist;
		
		DeviceConfig* paneldevs  = app.site->panels[ ping->sender ].devs;
		CAERule* 	  panelrules = app.site->panels[ ping->sender ].caeRules;
		 
		int actionbits = 0;
		
		for( DeviceRef* dr = ping->alerts; dr < ping->alerts + MAX_NET_REFS; dr++ ) 
		{  
			int zone;
			
			if ( dr->index == ALERT_UNUSED )
			{
				// flag last item
				*(uint32_t*)tmp = UINT_MAX;
				*((uint32_t*)tmp + 1 ) = UINT_MAX;
				tmp++;
				
				// finish
				break;
			}
		
			if ( dr->type == TYPE_DEV )
			{
				DeviceConfig* dc = paneldevs + dr->index;
				
				zone = dc->zone;
			}
			else if ( dr->type == TYPE_CAE )
			{
				CAERule* r = panelrules + dr->index;
				 
				zone = r->displayzone;
			}
			
		
			int shift = ( zone - 1 ) & 31;
			int set = ( zone - 1 ) >> 5;
		 
			// if zone isnt masked out
			if ( zone == 0 || ( ( app.panel->net[ ping->sender ][ dr->action ].display[ set ] >> shift ) & 1 ) )
			{
				TimedDeviceRef* tr = iostatus->FindRef( ping->sender, dr );
				
				// if device already in list
				if ( tr != NULL )
				{
					// keep existing timestamp
					tmp->timestamp = tr->timestamp;
					tmp->silenced = tr->silenced;
					tmp->found = tr->found;
				}
				else 
				{	
					// create new time stamp
					tmp->timestamp = now();
					tmp->silenced = 0;
					tmp->found = 0;
				}
				
				// copy to temp list
				*((short*)tmp) = *((short*)dr);
				tmp->chan = dr->chan;
				
				// add zone/action			 
				tmp->dispzone = zone;
				tmp++;	

				actionbits |= 1 << dr->action;
				
			}
		}
		
		// if different
		if ( !alertcmp( iostatus->netAlertList[ ping->sender ], tmplist, tmp - tmplist ) )
		{
			// copy new net list in
			memcpy( iostatus->netAlertList[ ping->sender ], tmplist, (char*)tmp - (char*)tmplist );
		
			// get which events have changed
			uint32_t action_update = ( actionbits ^ prev_actionbits[ ping->sender ] ) | actionbits;
			
			for( int action = 0; action <	SITE_NUM_OUTPUT_ACTIONS; action++ )
			{	
				//	if needs update
				if ( action_update & ( 1 << action ) )
				{			
					// if fire
					if ( action == ACTION_SHIFT_FIRE )
					{
						Send( EW_UPDATE_FIRE_LIST_MSG, ew );
						app.DebOut("******    Fire list change triggered." );
					}
					else // if event
					{
						Send( EW_UPDATE_EVENT_LIST_MSG, ew );	
					}											
				}	
			}
			
			prev_actionbits[ ping->sender ] = (uint16_t) actionbits;
		}
		
		
		// if fault flagged		
		if ( ping->nonzonal & ACTION_BIT_FAULT )
		{
			// if process fault required
#if 0
			if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].process[ 0 ] & ( 1 << ACTION_SHIFT_FAULT ) )
			{
				// if not in fault already
				if ( !current_fault[ ping->sender ] )
				{
					Fault::AddFault( fault_msg[ ping->sender ], 255, -1, 0, FAULTFLAG_NETWORK |FAULTFLAG_HIDDEN );
					current_fault[ ping->sender ] = true;
				}
			}	// else if just display
			else if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].display[ 0 ] & ( 1 << ACTION_SHIFT_FAULT ) )
			{
				// if not in fault already
				if ( !current_fault[ ping->sender ] )
				{
					Fault::AddFault( fault_msg[ ping->sender ], 255, -1, 0, FAULTFLAG_NETWORK | FAULTFLAG_DONT_ALARM );
					current_fault[ ping->sender ] = true;
				}
			}
#endif
			if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].display[ 0 ] & ( 1 << ACTION_SHIFT_FAULT ) )
			{
				// if not in fault already
				if ( !current_fault[ ping->sender ] )
				{
					Fault::AddFault( fault_msg[ ping->sender ], 255, -1, 0, FAULTFLAG_NETWORK | FAULTFLAG_DONT_ALARM );
					current_fault[ ping->sender ] = true;
				}
			}
			else if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].process[ 0 ] & ( 1 << ACTION_SHIFT_FAULT ) )
			{
				// if not in fault already
				if ( !current_fault[ ping->sender ] )
				{
					Fault::AddFault( fault_msg[ ping->sender ], 255, -1, 0, FAULTFLAG_NETWORK);
					current_fault[ ping->sender ] = true;
				}
			}	// else if just display

			else // no fault
			{
				// if in fault
				if ( current_fault[ ping->sender ] )
				{
					Fault::RemoveFault( fault_msg[ ping->sender ], 255, -1, 0 ); 
					current_fault[ ping->sender ] = false;
				}
			}
		}
		else
		{
			// if in fault
			if ( current_fault[ ping->sender ] )
			{
				Fault::RemoveFault( fault_msg[ ping->sender ], 255, -1, 0 ); 
				current_fault[ ping->sender ] = false;
			}
		}
 	
		// Note - Fault zones bits are used for silence, reset, and re-sound, as only of the 96 zone bits is needed (as these events arent zonal)
		
		if ( ping->nonzonal & ACTION_BIT_SILENCE )
		{
			if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].process[ 0 ] & ( 1 << ACTION_SHIFT_SILENCE ) )
			{
				app.Send( EVENT_SILENCE_BUTTON );
			}
			if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].display[ 0 ] & ( 1 << ACTION_SHIFT_SILENCE ) )
			{
				Log::Msg( LOG_EVT, "'%s' Silenced", app.site->panels[ ping->sender ].name );
			}
		}
		
		if ( ping->nonzonal & ACTION_BIT_RESOUND )
		{
			if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].process[ 0 ]  & ( 1 << ACTION_SHIFT_SILENCE ) )
			{
				app.Send( EVENT_RESOUND_BUTTON );
			}
			if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].display[ 0 ] & ( 1 << ACTION_SHIFT_SILENCE ) )
			{
				Log::Msg( LOG_EVT, "'%s' Re-sounded", app.site->panels[ ping->sender ].name );
			}
		}
		
		if ( ping->nonzonal & ACTION_BIT_RESET )
		{
			if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].process[ 0 ] & ( 1 << ACTION_SHIFT_RESET ) )
			{
				app.Send( EVENT_RESET_BUTTON );
			}
			if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].display[ 0 ] & ( 1 << ACTION_SHIFT_RESET ) )
			{
				Log::Msg( LOG_EVT, "'%s' Reset", app.site->panels[ ping->sender ].name );
			}
		}
		
		if ( ping->nonzonal & ACTION_BIT_CONFIRM_FIRE )
		{
			if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].process[ 0 ] & ACTION_BIT_CONFIRM_FIRE )
			{
				app.Send( EVENT_CONFIRM_FIRE_BUTTON );
			}
			if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].display[ 0 ] & ACTION_BIT_CONFIRM_FIRE )
			{
				Log::Msg( LOG_EVT, "'%s' Confirm Fire", app.site->panels[ ping->sender ].name );
			}
		}
		
		if ( ping->nonzonal & ACTION_BIT_CONFIRM_EVENT )
		{
			if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].process[ 0 ] & ACTION_BIT_CONFIRM_FIRE )
			{
				app.Send( EVENT_CONFIRM_EVENT_BUTTON );
			}
			if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].display[ 0 ] & ACTION_BIT_CONFIRM_FIRE )
			{
				Log::Msg( LOG_EVT, "'%s' Confirm Event", app.site->panels[ ping->sender ].name );
			}
		}
	 
		if ( ping->nonzonal & ACTION_BIT_ACKNOWLEDGE_FIRE )
		{
			if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].process[ 0 ] & ACTION_BIT_CONFIRM_FIRE )
			{
				app.Send( EVENT_ACKNOWLEDGE_FIRE_BUTTON );
			}
			if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].display[ 0 ] & ACTION_BIT_CONFIRM_FIRE )
			{
				Log::Msg( LOG_EVT, "'%s' Acknowledge Fire", app.site->panels[ ping->sender ].name );
			}
		}
		
		if ( ping->nonzonal & ACTION_BIT_ACKNOWLEDGE_EVENT )
		{
			if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].process[ 0 ] & ACTION_BIT_CONFIRM_FIRE )
			{
				app.Send( EVENT_ACKNOWLEDGE_EVENT_BUTTON );
			}
			if ( app.panel->net[ ping->sender ][ ACTION_SHIFT_FAULT ].display[ 0 ] & ACTION_BIT_CONFIRM_FIRE )
			{
				Log::Msg( LOG_EVT, "'%s' Acknowledge Event", app.site->panels[ ping->sender ].name );
			}
		}
	//	app.DebOut("* Packet processed\n" );
	}
}
 

/*************************************************************************/
/**  \fn      int PanelNetwork::Receive( Message* )
***  \brief   Class function
**************************************************************************/

void PanelNetwork::UartRxComplete( UART_HandleTypeDef* uart )
{
	//app.DebOut("******    RxEnd%d\n[\n",  HAL_GetTick( ) );
	
	got_packet = true;
}


/*************************************************************************/
/**  \fn      int PanelNetwork::Poll( )
***  \brief   Class function
**************************************************************************/

void PanelNetwork::Poll( )
{
#ifdef DO_NET_TEST_CODE_
	if ( app.site != NULL ) //if ( app.site->numPanels > 1 )
	{
		if ( time_to_send )
		{
			SendPacket( );
			
			time_to_send = false;
	
			txping.sender = 1;
			 
			GetAlertListFromPing( &txping );
				
		}
	}
#else	
	
	if ( app.site != NULL ) if ( app.site->numPanels > 1 )
	{
		if ( time_to_send )
		{
			SendPacket( );
			
			time_to_send = false;
			
			int me = app.site->currentIndex;
			int shuffled = false;
			static int showing_fail[ SITE_MAX_PANELS];
			
			for( int panel = 0; panel < app.site->numPanels; panel++ )
			{
				if ( ping_count[ panel ] != prev_ping_count[ panel ] + 1 )
				{
					if ( fail_count[ panel ]++ == PANEL_NET_MAX_FAILS )
					{
						if ( !showing_fail[ panel ] )
						{
							showing_fail[ panel ] = true;
							
							if ( panel == me )
							{
								Fault::AddFault( FAULT_NETWORK_TX_FAIL );
							}
							else
							{
								//Fault::AddFault( fault_rx_msg[ panel ] );
								Fault::AddFault( fault_rx_msg[ panel ], 255, -1, 0, FAULTFLAG_WARNING, FAULTGROUP_GENERAL);
							}
						}
						app.DebOut("******    Shuffle (no tx) %d\n", app.site->currentIndex );
						Restart( ( rand() & PANEL_NET_RAND_MAX ) + PANEL_NET_RAND_MIN );
						fail_count[ panel ] = 0;
						shuffled = true;
					}		 
				}
				else
				{
					if ( showing_fail[ panel ] )
					{
						if ( panel == me )
						{
							Fault::RemoveFault( FAULT_NETWORK_TX_FAIL );
							Fault::RemoveFault( fault_rx_msg[ panel ]);
						}
						else
						{
							Fault::RemoveFault( fault_rx_msg[ panel ] );
						}
						showing_fail[ panel ] = false;
					}
					fail_count[ panel ] = 0;
				}	
				prev_ping_count[ panel ] = ping_count[ panel ];
			}
		 
			if ( !shuffled ) Restart( PANEL_NET_MAX_NODES * PANEL_NET_PING_PERIOD );	
		}
		else if ( sent_packet )
		{			
		// End transmit
			
			ping_count[ app.site->currentIndex ]++;
			last_ping[ app.site->currentIndex ] = now( );
		 
			sent_packet = false;
			HAL_UART_AbortReceive( net_uart );
			HAL_UART_Receive_DMA( net_uart, (uint8_t*) &rxping, PANEL_NET_RXPING_SIZE );
		}
		
		if ( got_packet && !ignore_next_round )
		{
			got_packet = false;
			
			NetPing* ping = CheckPacket( *(int*)app.site->uid );
		
			if ( ping != NULL ) 
			{
				ping_count[ ping->sender ]++;
				last_ping[ ping->sender ] = now( );
							 
				mesh_count[ ping->sender ] = ping->numDevsOnMesh;
				
				GetAlertListFromPing( ping );
				
				app.DebOut("******    Restarting timer\n" );
				
				Restart( ( ( ( app.site->currentIndex + PANEL_NET_MAX_NODES ) - ping->sender ) % PANEL_NET_MAX_NODES ) * PANEL_NET_PING_PERIOD - sizeof(NetPing) );
			 				
				if ( HAL_UART_Receive_DMA( net_uart, (uint8_t*) &rxping, PANEL_NET_RXPING_SIZE ) != HAL_OK )
				{
					app.DebOut("******    Receive_IT fail.\n" );
				}
			}
		}		 
	}	
#endif	
}	


/*************************************************************************/
/**  \fn      void PanelNetwork::UartError( UART_HandleTypeDef* uart )
***  \brief   Class function
**************************************************************************/

void PanelNetwork::UartError( UART_HandleTypeDef* uart )
{
	GPIOManager::Set( GPIO_UART6_DE, IO_PIN_RESET );
	
	app.DebOut("******    ERROR %\n", app.site->currentIndex );
 
/*	
	HAL_UART_DeInit( uart );
	HAL_Delay( 5 );	//settle
	HAL_RS485Ex_Init( uart, UART_DE_POLARITY_LOW, 0, 0 );
	HAL_Delay( 5 );	//settle
	
	uart->TxCpltCallback = UartTxComplete;
	uart->RxCpltCallback = UartRxComplete;
	uart->ErrorCallback = UartError;
*/	
	HAL_UART_AbortReceive( uart );
	
	HAL_UART_Receive_DMA( uart, (uint8_t*) &me->rxping, PANEL_NET_RXPING_SIZE ); 
 
	me->rxfails = 0;
		 
	me->Restart( ( rand() & PANEL_NET_RAND_MAX ) + PANEL_NET_RAND_MIN );
}	


/*************************************************************************/
/**  \fn      void PanelNetwork::TimerElapsed( TIM_HandleTypeDef* tim )
***  \brief   Class function
**************************************************************************/

void PanelNetwork::TimerPeriodElapsed( TIM_HandleTypeDef* tim )
{	
	time_to_send = true;
}

	
/*************************************************************************/
/**  \fn      void PanelNetwork::SendPacket( )
***  \brief   Class function
**************************************************************************/

void PanelNetwork::SendPacket( )
{		
	if ( iostatus != NULL && !inerror )
	{		
		net_uart->RxXferCount = net_uart->hdmarx->Instance->NDTR;
		
		// Ensure a receive isn't being processed
		if ( net_uart->RxXferCount == net_uart->RxXferSize || net_uart->RxXferCount == 0 || 
			  ( ( net_uart->RxXferCount == net_uart->RxXferSize - 1 )  )
			)		
		{	
			// Set header
			
			txping.sender = app.site->currentIndex;
			txping.startbyte = 0x55;
			txping.systemID = *(int*)app.site->uid;
			txping.numDevsOnMesh = NCU::current_status.active;
	 	
			// Get cae/dev list
			iostatus->GetAlertList( txping.alerts, MAX_NET_REFS );
			
			// get  reset / resound / silnce bits
		 		 
			txping.nonzonal = toSend;
			toSend = 0;
			ignore_next_round = false;
			
			// add fault bit
			if ( iostatus->zonebits[ ACTION_SHIFT_FAULT ][ 0 ] & 1 )
			{
				if(app.panel->systemId == app.site->panels[txping.sender].systemId)
                txping.nonzonal |= ACTION_BIT_FAULT;
			}
			
			// Finally do checksum
			txping.checksum = txping.GetSum( );		
		
			// Dont encypt - let radio do that
			//	txping.Crypt( *(int*)app.site->uid );
		
	//		app.DebOut("******    Pan:%X %d ", txping.systemID, app.site->currentIndex );			
			
			txping.Send( net_uart, data_enable );
		}
		else 
		{
			static int last_rx_pos;
			
			// Recever stuck?
			if ( net_uart->RxXferCount == last_rx_pos )
			{
				GPIOManager::Set( data_enable, IO_PIN_RESET );
				
				app.DebOut("******    Rx stuck - reset\n" );
				HAL_UART_AbortReceive( net_uart );
				HAL_UART_Receive_DMA( net_uart, (uint8_t*) &rxping, PANEL_NET_RXPING_SIZE );
			}
			
			last_rx_pos = net_uart->RxXferCount;
					
			app.DebOut("******    Shuffle (collision=%d) %d\n", net_uart->RxXferCount, app.site->currentIndex );
			Restart( ( rand() & PANEL_NET_RAND_MAX ) + PANEL_NET_RAND_MIN );
		}		 
	}
}
 


/*************************************************************************/
/**  \fn      void PanelNetwork::Restart( int t )
***  \brief   Class function
**************************************************************************/

void PanelNetwork::Restart( int t )
{	
//	app.DebOut("******    %d: tx pan %d in %d",  HAL_GetTick( ), app.site->currentIndex, t );
	HAL_TIM_Base_Stop_IT( net_timer );
 	net_timer->Instance->ARR = t;
	net_timer->Instance->CNT = 1;
	net_timer->Instance->EGR = TIM_EGR_UG;
	net_timer->Instance->SR = 0;
	HAL_TIM_Base_Start_IT( net_timer );
}


/*************************************************************************/
/**  \fn      void USART6_IRQHandler(void)
***  \brief   IRQ Handler
**************************************************************************/

extern "C" void USART6_IRQHandler( void )
{
	HAL_UART_IRQHandler( app.panel_network_uart );
}
