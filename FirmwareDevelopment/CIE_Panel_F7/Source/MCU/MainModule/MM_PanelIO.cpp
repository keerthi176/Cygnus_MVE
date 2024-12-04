/***************************************************************************
* File name: MM_PanelIO.cpp
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
* Various utiltiies
*
**************************************************************************/

/* System Include Files
**************************************************************************/

#ifdef STM32F767xx
#include "stm32f7xx_hal_rcc.h"
#include "stm32f767xx.h"
#else
#include "stm32f4xx_hal_rcc.h"
#include "stm32f469xx.h"
#endif


#include <assert.h>


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_GPIO.h"
#include "MM_fault.h"
#include "MM_PanelIO.h"
#include "MM_Events.h"
#include "MM_Message.h"
#include "stmpe811.h"
#include "CO_Site.h"
#include "MM_IOStatus.h"
#include "MM_CUtils.h"
#include "MM_ADC.h"
#include "CO_device.h"
#include "MM_LED.h"
#include "MM_Utilities.h"




/* Defines
**************************************************************************/

#define TEST_TONE_LENGTH 	2
#define ITEMS(x) (sizeof(x)/sizeof(*x))

#define ASSERTION_MIN 		559
#define ASSERTION_MAX 		1241
#define DEASSERTION_MIN 	2234
#define DEASSERTION_MAX 	3227
#define SHORT_CIRCUIT		434
#define OPEN_CIRCUIT			3351


#define VOLTS( adc ) ( (float)( adc * 36 ) / 4096.0 )
#define ADCUNITS( v )	( ( v * 4096 ) / 36 )
	

/* 5k values?? 
#define OUTPUT_SHORT_MAX	1867
#define OUTPUT_NORMAL_MIN	1992
#define OUTPUT_NORMAL_MAX	2316
#define OUTPUT_OPEN_MIN		2850
*/

#define OUTPUT_OFF		10

// 10K Values ( Guessed! )
#define OUTPUT_SHORT_MAX	500
#define OUTPUT_NORMAL_MIN	( 2150 - 200 )
#define OUTPUT_NORMAL_MAX	( 2150 + 200 )
#define OUTPUT_OPEN_MIN		( 2711 - 150 )
#define OUTPUT_SUPPLY_MIN	1500

/* Globals
**************************************************************************/


static PanelInput panelInput[ ] = {
	{ ADC_ROUTING_ACK_INPUT, -1, STATE_UNKNOWN },
	{ ADC_CALL_POINT_INPUT, -1,  STATE_UNKNOWN },
	{ ADC_ALARM_INPUT, 		-1,  STATE_UNKNOWN },
	{ ADC_SILENCE_INPUT, 	-1,  STATE_UNKNOWN },
	{ ADC_FAULT_INPUT, 		-1,  STATE_UNKNOWN },
};


static PanelOutput panelOutput[ ] = {
	{ ADC_NONE, 					ALARM_RELAY_GPIO_Port, 		ALARM_RELAY_Pin 	, STATE_UNKNOWN },
	{ ADC_NONE,	 					FAULT_RELAY_GPIO_Port, 		FAULT_RELAY_Pin 	, STATE_UNKNOWN },
	{ ADC_NONE, 					FIRE_RELAY_GPIO_Port, 		FIRE_RELAY_Pin 	, STATE_UNKNOWN },
	{ ADC_ROUTING_OUTPUT, 		FIRE_ROUTING_GPIO_Port,		FIRE_ROUTING_Pin 	, STATE_UNKNOWN },
	{ ADC_SOUNDER_MONITORING, 	SOUNDERS_RELAY_GPIO_Port, 	SOUNDERS_RELAY_Pin, STATE_UNKNOWN },
};
	
 

DeviceConfig* PanelIO::panelio_config = NULL;

 
/*************************************************************************/
/**  \fn      PanelDevice::PanelDevice( PanelOutput* pi, DeviceConfig* conf, PanelIO* owner ) : Device( pi, conf, owner )
***  \brief   Constructor for class
**************************************************************************/

PanelDevice::PanelDevice( PanelOutput* pi, DeviceConfig* conf, PanelIO* owner ) : Device( pi, conf, owner )
{
	for( int ch = 0; ch < conf->numOutputs; ch++ )
	{		
		flags[ ch ] |= IsOutputInverted( ch ) ? 0 : OUTPUT_ASSERTED; // so de-assert works!
		OutputDeAssert( ch );
	}		
}


/*************************************************************************/
/**  \fn      void PanelOutputDevice::Assert( int channel, int action )
***  \brief   Class function
**************************************************************************/

void PanelDevice::OutputChange( int channel, int action, int value )
{
	PanelOutput* p = (PanelOutput*) ptr;
	PanelOutput* out = NULL;
	
	switch ( config->output[ channel ].type )
	{
		case CO_CHANNEL_PANEL_ALARM_RELAY 	 : out = p + PANEL_OUTPUT_ALARM; break;
		case CO_CHANNEL_PANEL_FIRE_RELAY 	 : out = p + PANEL_OUTPUT_FIRE; break;
		case CO_CHANNEL_PANEL_SOUNDERS_RELAY : out = p + PANEL_OUTPUT_SOUNDERS; break;
		case CO_CHANNEL_PANEL_FAULT_RELAY    : out = p + PANEL_OUTPUT_FAULT; break;
		case CO_CHANNEL_PANEL_ROUTING_OUTPUT : out = p + PANEL_OUTPUT_FIRE_ROUTING; break;
	}
	
	if ( out != NULL )
	{		
		if ( value )
		{
			if ( config->output[ channel ].type == CO_CHANNEL_PANEL_ROUTING_OUTPUT )
			{
				if ( LED::state[ GPIO_FireRoutingActive ] == LED_OFF && action ==  ACTION_SHIFT_FIRE )
				{
					 LED::Flash( GPIO_FireRoutingActive );
				}	
			}
			
			if ( ! ( out->state & STATE_SHORT_CIRCUIT ) )
			{			
				//Fault replay is active low
				if ( config->output[ channel ].type == CO_CHANNEL_PANEL_FAULT_RELAY )
				{	
					HAL_GPIO_WritePin( FAULT_RELAY_GPIO_Port, FAULT_RELAY_Pin, GPIO_PIN_RESET );		
				}
				else  
				{	
					HAL_GPIO_WritePin( out->port, out->pin, GPIO_PIN_SET );	
				}				
			}
			
			if ( action == ACTION_SHIFT_TEST_PROF ) 
			{
				out->state |= STATE_TEST_TONE;
				((PanelIO*)owner)->test_tone_count = TEST_TONE_LENGTH;
			}						 
		}
		else	
		{
			if ( config->output[ channel ].type == CO_CHANNEL_PANEL_ROUTING_OUTPUT )
			{
				 LED::Off( GPIO_FireRoutingActive );
			}
			
			//Fault replay is active low
			if ( config->output[ channel ].type == CO_CHANNEL_PANEL_FAULT_RELAY )
			{			
				HAL_GPIO_WritePin( FAULT_RELAY_GPIO_Port, FAULT_RELAY_Pin, GPIO_PIN_SET );		
			}
			else
			{
				HAL_GPIO_WritePin( out->port, out->pin, GPIO_PIN_RESET );
			}	
		}
	}		
}
	

/*************************************************************************/
/**  \fn      PanelIO::PanelIO( )
***  \brief   Constructor for class
**************************************************************************/

PanelFunctionButtonDevice::PanelFunctionButtonDevice( DeviceConfig* dc ) : Device( NULL, dc )
{
	
}
	

/*************************************************************************/
/**  \fn      PanelIO::PanelIO( )
***  \brief   Constructor for class
**************************************************************************/

PanelIO::PanelIO( ) : Module( "PanelIO", 3, EVENT_FIRE_RESET | EVENT_FIRE_CLEAR | EVENT_RESET | EVENT_CLEAR )
{	
	buttons_config =  NULL;
	
	Receive( EVENT_CLEAR );
}	

	

/*************************************************************************/
/**  \fn      int PanelIO::Init( )
***  \brief   Class function
**************************************************************************/

int PanelIO::Init( )
{
	iostatus = (IOStatus*)    		app.FindModule( "IOStatus" );
	settings = (Settings*)    		app.FindModule( "Settings" );

	assert( iostatus != NULL );
	assert( settings != NULL );	

	
	return true;
}


/*************************************************************************/
/**  \fn      void PanelIO::InitSite( )
***  \brief   Class function
**************************************************************************/

void PanelIO::InitSite( )
{
	if ( app.panel != NULL )
	{
		panelio_config = settings->FindType( DEVICE_TYPE_PANEL_IO );
		
		if ( panelio_config != NULL )
		{
			PanelDevice paneldev( panelOutput, panelio_config, this );
			
			iostatus->RegisterDevice( &paneldev );					
		}	
		 
		
		buttons_config = settings->FindType( DEVICE_TYPE_FUNCTION_BUTTON );
		
		if ( buttons_config != NULL )
		{
			PanelFunctionButtonDevice panelFunctionButtonDevice( buttons_config );
			
			iostatus->RegisterDevice( &panelFunctionButtonDevice );					
		}	 
	}
}	


 
/*************************************************************************/
/**  \fn      int PanelIO::Receive( Message* msg )
***  \brief   Class function
**************************************************************************/

int PanelIO::Receive( Message* msg )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int PanelIO::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int PanelIO::Receive( Command* cmd ) 
{
	if ( cmd->type == CMD_GET_USER_BUTTON_ACTION )
	{
		if ( app.panel != NULL )
		{
			DeviceConfig* devconfig = settings->FindType( DEVICE_TYPE_FUNCTION_BUTTON );
		
			if ( devconfig != NULL )
			{
				if ( cmd->int0 < devconfig->numInputs )
				{
					int action = devconfig->input[ cmd->int0 ].action;
					
					if ( action < ACTION_SHIFT_UNDEFINED )
					{
						const char* typetext;
						GetLogType( action, &typetext );
						
						cmd->int1 = (int) typetext;
					}
					else if ( action == ACTION_SHIFT_UNDEFINED )
					{
						cmd->int1 = (int) "C&E";
					}
					else
					{
						cmd->int1 = (int) "";
					}
					return CMD_OK;
				}
			}
		}
		cmd->int1 = (int) "";
		return CMD_ERR_NO_SITE;
	}
	else if ( cmd->type == CMD_FUNCTION_BUTTON )
	{
		Device* buttons = iostatus->FindDevice( buttons_config );
		
		if ( buttons != NULL )
		{
			if ( ! ( buttons_config->input[ cmd->int0 ].flags & CHANNEL_OPTION_LATCHING ) )
			{
				// toggle 
				if ( cmd->int1 )
				{
					// if on
					if ( buttons->flags[ cmd->int0 ] & INPUT_ASSERTED )
					{
						// toggle off
						buttons->InputDeAssert( cmd->int0, now( ), 0 );
					}
					else // if off
					{
						// toggle on
						buttons->InputAssert( cmd->int0, now( ), 99 );
					}
				}
			}
			else // momentary
			{				
				if ( cmd->int1 )
				{
					buttons->InputAssert( cmd->int0, now( ), 99 );
				}
				else
				{
					buttons->InputDeAssert( cmd->int0, now( ), 0 );
				}
			}
			cmd->int1 = !! ( buttons->flags[ cmd->int0 ] & INPUT_ASSERTED );
			return CMD_OK;		
		}
		else
		{
			return CMD_ERR_OUT_OF_RANGE;
		}
	}
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      void PanelIO::CheckOutput( PanelOutputIndex outp, int chanIndex, ChannelType chantype, int level, Device* dev )
***  \brief   Class function
**************************************************************************/

void PanelIO::CheckOutput( PanelOutputIndex outp, int chanIndex, ChannelType chantype, int level, Device* dev )
{			
	if ( dev != NULL  )
	{	
		if ( level < OUTPUT_SHORT_MAX )
		{
			if ( !( panelOutput[ outp].state & STATE_SHORT_CIRCUIT ) )
			{
				panelOutput[ outp].state |= STATE_SHORT_CIRCUIT;
				
				Fault::AddFault( outp == PANEL_OUTPUT_SOUNDERS ? FAULT_SOUNDER_SHORT_CIRCUIT : ( outp == PANEL_OUTPUT_FIRE_ROUTING ? FAULT_ROUTING_SHORT_CIRCUIT : FAULT_SHORT_CIRCUIT ),
				dev->config->zone, dev->config->unit, chantype );
				
				dev->OutputChange( chanIndex, 0, 0 );
			}
		}
		else if ( panelOutput[ outp].state & STATE_SHORT_CIRCUIT )
		{
			panelOutput[ outp].state &= ~STATE_SHORT_CIRCUIT;
			
			Fault::RemoveFault( outp == PANEL_OUTPUT_SOUNDERS ? FAULT_SOUNDER_SHORT_CIRCUIT : ( outp == PANEL_OUTPUT_FIRE_ROUTING ? FAULT_ROUTING_SHORT_CIRCUIT : FAULT_SHORT_CIRCUIT ), dev->config->zone, dev->config->unit, chantype );
		}	
		
		if ( level > OUTPUT_OPEN_MIN && !dev->IsOutputDisabled( chanIndex ) )
		{
			if ( !( panelOutput[ outp].state & STATE_OPEN_CIRCUIT ) )
			{
				Fault::AddFault( outp == PANEL_OUTPUT_SOUNDERS ? FAULT_SOUNDER_OPEN_CIRCUIT : ( outp == PANEL_OUTPUT_FIRE_ROUTING ? FAULT_ROUTING_OPEN_CIRCUIT : FAULT_OPEN_CIRCUIT ),
						dev->config->zone, dev->config->unit, chantype );
					 
				panelOutput[ outp].state |= STATE_OPEN_CIRCUIT;
			}
		}	
		else if ( ( panelOutput[ outp].state & STATE_OPEN_CIRCUIT ) ) 
		{
			Fault::RemoveFault( outp == PANEL_OUTPUT_SOUNDERS ? FAULT_SOUNDER_OPEN_CIRCUIT : ( outp == PANEL_OUTPUT_FIRE_ROUTING ? FAULT_ROUTING_OPEN_CIRCUIT : FAULT_OPEN_CIRCUIT ), dev->config->zone, dev->config->unit, chantype );
			
			panelOutput[ outp].state &= ~STATE_OPEN_CIRCUIT;
		}
	}
}


/*************************************************************************/
/**  \fn      int PanelIO::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int PanelIO::Receive( Event event ) 
{
	switch ( event )
	{		 
		case EVENT_RESET :
		case EVENT_FIRE_RESET :
		{
			InitSite( );
			break;
		}
		
		case EVENT_FIRE_CLEAR:
		case EVENT_CLEAR :
		{
			for( int n = 0; n < ITEMS( panelOutput ); n++ )
			{
				panelInput[ n].state = STATE_UNKNOWN;
				panelOutput[ n].state = STATE_UNKNOWN;
			}
			
			panelio_config = NULL;
			buttons_config = NULL;
			
			break;
		}	
	}
	return 0;
}	

int PanelIO::adc_complete = false;

/*************************************************************************/
/**  \fn      int PanelIO::Receive( Event event )
***  \brief   Class function
**************************************************************************/

void PanelIO::ADCComplete( )
{
	adc_complete = true;
}


/*************************************************************************/
/**  \fn      static PanelInputIndex GetInputIndex( ChannelType type )
***  \brief   Helper function
**************************************************************************/

static int GetInputIndex( ChannelType type )
{
	for( InputChannel* i = PanelIO::panelio_config->input; i < PanelIO::panelio_config->input + PanelIO::panelio_config->numInputs; i++ )
	{
		if ( i->type == type ) return (PanelInputIndex) ( i - PanelIO::panelio_config->input );
	}

	return PANEL_INPUT_UNKNOWN;
}

/*************************************************************************/
/**  \fn      static PanelOutputIndex GetOutputIndex( ChannelType type )
***  \brief   Helper function
**************************************************************************/

static int GetOutputIndex( ChannelType type )
{
	for( OutputChannel* o = PanelIO::panelio_config->output; o < PanelIO::panelio_config->output + PanelIO::panelio_config->numOutputs; o++ )
	{
		if ( o->type == type ) return (PanelOutputIndex) ( o - PanelIO::panelio_config->output );
	}

	return PANEL_OUTPUT_UNKNOWN;
}


/*************************************************************************/
/**  \fn      void Poll( )
***  \brief   Class function
**************************************************************************/

void PanelIO::Poll( )
{
	if ( adc_complete )
	{			
		adc_complete = false;
		
		static bool overload_fault = false;
		
		int should_be_zero = ADC::GetValue( ADC_SPARE );
		
		if ( should_be_zero == 0 )
		{				
			int psu = ADC::GetAvgValue( ADC_PSU_24V_SUPPLY );
			int aux = ADC::GetAvgValue( ADC_AUX_24V );
			
 	//	 	app.DebOut( "AV PSU %f, AUX %f\n", VOLTS( psu ), VOLTS( aux ) );
   //	 	app.DebOut( "CU PSU %f, AUX %f\n", VOLTS( ADC::GetValue( ADC_PSU_24V_SUPPLY ) ), VOLTS( ADC::GetValue( ADC_AUX_24V ) ) );
			
			
			// if difference or value less than 2 volts assume fault
			if ( ( psu - aux ) > ADCUNITS( 2 ) || aux < ADCUNITS( 2 ) )
			{
				if ( !overload_fault )
				{
					Fault::AddFault( FAULT_OVERLOAD_AUX );
					overload_fault = true;
				}
			}
			else
			{
				if ( overload_fault )
				{
					Fault::RemoveFault( FAULT_OVERLOAD_AUX );
					overload_fault = false;
				}
			}
				
			if ( app.panel != NULL )
			{	
				static bool fire_routing_asserted = false;
				static bool sounders_asserted = false;
				static int routing_changecount = 0;
				static int sounders_changecount = 0;

				Device* dev = iostatus->FindDevice( panelio_config );
				
				if ( dev != NULL )
				{
					routing_changecount++;
					sounders_changecount++;
					
					// Routing output first..
					
					int routing_index = GetOutputIndex( CO_CHANNEL_PANEL_ROUTING_OUTPUT );
				
					// do we have a routing output
					if ( routing_index != -1 )
					{		
						// is it enabled
						if ( ! dev->IsOutputDisabled( routing_index ) )
						{
							// get avg voltage
							int routingpower = ADC::GetAvgValue( ADC_ROUTING_POWER );
						
							// if routing output asserted
							if ( dev->flags[ routing_index] & OUTPUT_ASSERTED )
							{		
								if ( !fire_routing_asserted ) routing_changecount = 0;
								fire_routing_asserted = true;
								
								if ( routing_changecount > 2 )
								{
									// if too low
									if ( routingpower < OUTPUT_SUPPLY_MIN )
									{
										// and not flagged
										if ( !(panelOutput[ PANEL_OUTPUT_FIRE_ROUTING ].state & STATE_SUPPLY_FAIL ))
										{
											// generate fault
											Fault::AddFault( FAULT_ROUTING_OUTPUT_SUPPLY );
											
											// flag fault
											panelOutput[ PANEL_OUTPUT_FIRE_ROUTING ].state |= STATE_SUPPLY_FAIL;
										}
									}
									// else ok ( 
									else 
									{
										if ( panelOutput[ PANEL_OUTPUT_FIRE_ROUTING ].state & STATE_SUPPLY_FAIL )
										{
											// Remove fault
											Fault::RemoveFault( FAULT_ROUTING_OUTPUT_SUPPLY );
											// Clear flag
											panelOutput[ PANEL_OUTPUT_FIRE_ROUTING ].state &= ~STATE_SUPPLY_FAIL;
										}
									
									}
								}
							}
							
							else if ( routingpower < OUTPUT_OFF )   
							{
								fire_routing_asserted = false;
								
								int routinglevel = ADC::GetAvgValue( panelOutput[ PANEL_OUTPUT_FIRE_ROUTING ].ADCchannel );
												
								CheckOutput( PANEL_OUTPUT_FIRE_ROUTING, routing_index, CO_CHANNEL_PANEL_ROUTING_OUTPUT, routinglevel, dev );
	#ifdef DONT_GUESS_FAULTS						
								// Remove 'assertion' faults
								if ( panelOutput[ PANEL_OUTPUT_FIRE_ROUTING ].state & STATE_SUPPLY_FAIL )
								{
									// Remove fault
									Fault::RemoveFault( FAULT_ROUTING_OUTPUT_SUPPLY );
									// Clear flag
									panelOutput[ PANEL_OUTPUT_FIRE_ROUTING ].state &= ~STATE_SUPPLY_FAIL;
								}
	#endif						
							}
						}	
					}
					
					// Sounders
					
					int sounders_index = GetOutputIndex( CO_CHANNEL_PANEL_SOUNDERS_RELAY );
					
					// if sounder loops found
					if ( sounders_index != -1 )
					{
						// dont want tome to last forever so switch off evenif disabled..
						if ( panelOutput[ PANEL_OUTPUT_SOUNDERS ].state & STATE_TEST_TONE )
						{
							test_tone_count--;
							
							if ( test_tone_count <= 0 )
							{
								panelOutput[ PANEL_OUTPUT_SOUNDERS ].state &= ~STATE_TEST_TONE;
 								
								dev->OutputChange( sounders_index, 0, 0 );
							}
						}
						
						// are sounders enabled
						if ( ! dev->IsOutputDisabled( sounders_index ) )
						{
						 
							// get voltage
							int sounderpower1 = ADC::GetAvgValue( ADC_SOUNDER_CIRCUIT_1 );
							int sounderpower2 = ADC::GetAvgValue( ADC_SOUNDER_CIRCUIT_2 );
									
							// if sounders asserted
							if ( dev->flags[ sounders_index ] & OUTPUT_ASSERTED )
							{										
								// if off before
								if ( !sounders_asserted )  sounders_changecount = 0;
								
								sounders_asserted = true;
								
								// if no supply
								if ( sounders_changecount > 2 )
								{
									if ( sounderpower1 < OUTPUT_SUPPLY_MIN )
									{
										// if failure not flagged
										if ( !(panelOutput[ PANEL_OUTPUT_SOUNDERS ].state & STATE_CIRCUIT1_FAIL ) )
										{
											// Add fault
											Fault::AddFault( FAULT_SOUNDER1_SUPPLY );
											
											// Flag fault
											panelOutput[ PANEL_OUTPUT_SOUNDERS ].state |= STATE_CIRCUIT1_FAIL;
										}
									}				
									else 	if ( panelOutput[ PANEL_OUTPUT_SOUNDERS ].state & STATE_CIRCUIT1_FAIL )
									{
										// Remove fault
										Fault::RemoveFault( FAULT_SOUNDER1_SUPPLY );
										
										// Remove flag
										panelOutput[ PANEL_OUTPUT_SOUNDERS ].state &= ~STATE_CIRCUIT1_FAIL;
									}

									
									if ( sounderpower2 < OUTPUT_SUPPLY_MIN )
									{
										if ( !(panelOutput[ PANEL_OUTPUT_SOUNDERS ].state & STATE_CIRCUIT2_FAIL ))
										{
											Fault::AddFault( FAULT_SOUNDER2_SUPPLY );
											panelOutput[ PANEL_OUTPUT_SOUNDERS ].state |= STATE_CIRCUIT2_FAIL;
										}
									}
									else if ( panelOutput[ PANEL_OUTPUT_SOUNDERS ].state & STATE_CIRCUIT2_FAIL )
									{
										Fault::RemoveFault( FAULT_SOUNDER2_SUPPLY );
										panelOutput[ PANEL_OUTPUT_SOUNDERS ].state &= ~STATE_CIRCUIT2_FAIL;
									}
								}
							}
							else // no assertion
							{
								if ( sounderpower1 < OUTPUT_OFF && sounderpower2 < OUTPUT_OFF )
								{
									sounders_asserted = false;
									
									int sounderlevel = ADC::GetAvgValue( panelOutput[ PANEL_OUTPUT_SOUNDERS ].ADCchannel);
									CheckOutput( PANEL_OUTPUT_SOUNDERS, sounders_index, CO_CHANNEL_PANEL_SOUNDERS_RELAY, sounderlevel, dev );
									
									// Remove 'assertion' faults
	#ifdef DONT_GUESS_FAULTS							
									if ( panelOutput[ PANEL_OUTPUT_SOUNDERS ].state & STATE_CIRCUIT1_FAIL )
									{
										// Remove fault
										Fault::RemoveFault( FAULT_SOUNDER1_SUPPLY );
										
										// Remove flag
										panelOutput[ PANEL_OUTPUT_SOUNDERS ].state &= ~STATE_CIRCUIT1_FAIL;
									}
									if ( panelOutput[ PANEL_OUTPUT_SOUNDERS ].state & STATE_CIRCUIT2_FAIL )
									{
										Fault::RemoveFault( FAULT_SOUNDER2_SUPPLY );
										panelOutput[ PANEL_OUTPUT_SOUNDERS ].state &= ~STATE_CIRCUIT2_FAIL;
									}
	#endif							
								}
							}
						}
					}											
				//	app.DebOut(	"ADC_ROUTING_OUTPUT     %d\n", ADC::GetAvgValue(  ADC_ROUTING_OUTPUT     ) );  *
				//	app.DebOut(	"ADC_ROUTING_POWER      %d\n", ADC::GetAvgValue(  ADC_ROUTING_POWER      ) );
				//	app.DebOut(	"ADC_SOUNDER_CIRCUIT_1  %d\n", ADC::GetAvgValue(  ADC_SOUNDER_CIRCUIT_1  ) );
				//	app.DebOut(	"ADC_SOUNDER_CIRCUIT_2  %d\n", ADC::GetAvgValue(  ADC_SOUNDER_CIRCUIT_2  ) );
				//	app.DebOut(	"ADC_SOUNDER_MONITORING %d\n", ADC::GetAvgValue(  ADC_SOUNDER_MONITORING ) );  *
				//	app.DebOut(	"ADC_PSU_24V_SUUPLY     %d\n", ADC::GetAvgValue(  ADC_PSU_24V_SUUPLY     ) );
				//	app.DebOut(	"ADC_DC_5V_SUPPLY       %d\n", ADC::GetAvgValue(  ADC_DC_5V_SUPPLY       ) );
				//	app.DebOut(	"ADC_PSU_FAULT_RELAY    %d\n", ADC::GetAvgValue(  ADC_PSU_FAULT_RELAY    ) );
				//	app.DebOut(	"ADC_EARTH_FAULT        %d\n", ADC::GetAvgValue(  ADC_EARTH_FAULT        ) );
				//	app.DebOut(	"ADC_AUX_24V            %d\n", ADC::GetAvgValue(  ADC_AUX_24V            ) );
					
				//	app.DebOut( "ADC_ROUTING_ACK_INPUT %d",ADC::GetAvgValue( ADC_ROUTING_ACK_INPUT    ) );
				//	app.DebOut( "ADC_CALL_POINT_INPUT  %d",ADC::GetAvgValue( ADC_CALL_POINT_INPUT     ) );
				//	app.DebOut( "ADC_ALARM_INPUT, 	  %d",ADC::GetAvgValue( ADC_ALARM_INPUT          ) );
				//	app.DebOut( "ADC_SILENCE_INPUT, 	  %d",ADC::GetAvgValue( ADC_SILENCE_INPUT        ) );
				//	app.DebOut( "ADC_FAULT_INPUT, 	  %d",ADC::GetAvgValue( ADC_FAULT_INPUT				) );
			 
					// For each panel input 
					
					for( PanelInput* pi = panelInput; pi < panelInput + ITEMS( panelInput ); pi++ )
					{
						
						// Get channel type from panelinput index
						ChannelType type = (ChannelType) ( CO_CHANNEL_PANEL_ROUTING_ACK + ( pi - panelInput ) );
						
						// Get device channel inxdex for this channel type
						int channel = GetInputIndex( type );
						
						// if valid
						if ( channel != -1 )
						{
							// if not disabled
							if ( !dev->config->IsInputDisabled( channel ) )
							{	
								// Get ADC value for this channel
								pi->ADClevel = ADC::GetValue( pi->ADCchannel );
								
							//	app.DebOut( "     ADC %d on (%d). (%d - %d)\n", pi - panelInput, pi->ADClevel, ASSERTION_MIN, ASSERTION_MAX );  
														
								// if in assertion range
								if (  pi->ADClevel >= ASSERTION_MIN && pi->ADClevel < ASSERTION_MAX )
								{
									// if not already asserted
									if ( !(pi->state & STATE_ASSERTED ) )
									{
										// assert input
										dev->InputAssert( channel, now( ), 100 );	
										
										// flag as asserted
										pi->state |= STATE_ASSERTED;
										
										app.DebOut( "ADC %d on (%d).\n", channel, pi->ADClevel ); 
									}
								}
								else // assume de-assertion for any other state .. if ( pi->ADClevel >= DEASSERTION_MIN && pi->ADClevel < DEASSERTION_MAX )
								{
									// if was asserted
									if ( pi->state & STATE_ASSERTED )
									{
										// de-asseert
										dev->InputDeAssert( channel, now( ), 0 );
										
										// flag as so
										pi->state &= ~STATE_ASSERTED;
									
										app.DebOut( "ADC %d off (%d).\n", channel, pi->ADClevel );
									}
								}
								
								// if in short circuit range
								if ( pi->ADClevel < SHORT_CIRCUIT )
								{
									// and not previously
									if ( !( pi->state & STATE_SHORT_CIRCUIT ) )
									{
										// flag as so
										pi->state |= STATE_SHORT_CIRCUIT;
										
										// Generate fault
										Fault::AddFault( type == CO_CHANNEL_PANEL_ROUTING_ACK ? FAULT_ROUTING_SHORT_CIRCUIT : FAULT_SHORT_CIRCUIT,
												dev->config->zone, dev->config->unit, type );
									}
								}
								// if not but was previously
								else if ( pi->state & STATE_SHORT_CIRCUIT )
								{
									// unflag it
									pi->state &= ~STATE_SHORT_CIRCUIT;
									
									// remove fault
									Fault::RemoveFault( type == CO_CHANNEL_PANEL_ROUTING_ACK ? FAULT_ROUTING_SHORT_CIRCUIT : FAULT_SHORT_CIRCUIT,
											dev->config->zone, dev->config->unit, type );
								}	
								
								// repeat for open
								
								if ( pi->ADClevel > OPEN_CIRCUIT )
								{
									if ( !( pi->state & STATE_OPEN_CIRCUIT ) )
									{
										Fault::AddFault( type == CO_CHANNEL_PANEL_ROUTING_ACK ? FAULT_ROUTING_OPEN_CIRCUIT : FAULT_OPEN_CIRCUIT,
												dev->config->zone, dev->config->unit, type );
												 
										pi->state |= STATE_OPEN_CIRCUIT;
									}
								}	
								else if ( pi->state & STATE_OPEN_CIRCUIT ) 
								{
									Fault::RemoveFault( type == CO_CHANNEL_PANEL_ROUTING_ACK ? FAULT_ROUTING_OPEN_CIRCUIT : FAULT_OPEN_CIRCUIT,
											dev->config->zone, dev->config->unit, type );
									
									pi->state &= ~STATE_OPEN_CIRCUIT;
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
	//	app.DebOut( "no adc.\n" );
	}
}




/*************************************************************************/
/**  int PanelIO::Receive( Event event )
***  \brief   Class function
**************************************************************************/

//extern "C" void GPIO_3_Handler( )  { panelInput[ 0].edgeCount++; }
//extern "C" void GPIO_4_Handler( )  { panelInput[ 1].edgeCount++; }
//extern "C" void GPIO_5_Handler( )  { panelInput[ 2].edgeCount++; }
//extern "C" void GPIO_6_Handler( )  { panelInput[ 3].edgeCount++; }

