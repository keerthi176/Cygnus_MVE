/***************************************************************************
* File name: MM_ADC.cpp
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
* ADC Watcher
*
**************************************************************************/

/* System Include Files
**************************************************************************/
#include <string.h>
#ifdef STM32F767xx
#include "stm32f7xx_hal_dma.h"
#include "stm32f7xx_hal_spi.h"
#else
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_spi.h"
#endif
#include <assert.h>


/* User Include Files
**************************************************************************/

#include "MM_Events.h"
#include "MM_fault.h"
#include "MM_Application.h"
#include "MM_ADC.h"
#include "MM_PanelIO.h"


/* Defines
**************************************************************************/

#define MAX_ADC_CHANNELS 8
#define mV * 4095 / 3300
#define CH(x) (uint8_t)(6 + ( x >> 2 )), (uint8_t)(x << 6), 0, 0
#define MSG_ABORT_IT 1

enum
{
	MONITOR_UNUSED = 0,
	MONITOR_EARTH,
	MONITOR_INPUT,
};	

// ADC averageing

static int first [ MAX_ADC_CHANNELS * 2] = { 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250 } ;
static int second[ MAX_ADC_CHANNELS * 2] = { 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250 } ;
static int third [ MAX_ADC_CHANNELS * 2] = { 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250, 2250 } ;
	
 // ADC buffer

static uint8_t output[ 2 * MAX_ADC_CHANNELS * 4 ] = { 0 }; 
static uint8_t rxData[ 2 * MAX_ADC_CHANNELS * 4 ] = { 0 }; 
static uint8_t txData[ 2 * MAX_ADC_CHANNELS * 4 ] = { CH(0), CH(1), CH(2), CH(3), CH(4), CH(5), CH(6), CH(7), CH(0), CH(1), CH(2), CH(3), CH(4), CH(5), CH(6), CH(7) };

int ADC::complete = true; 

/*************************************************************************/
/**  \fn      void CheckRX( ADC* psu )
***  \brief   Local helper function
**************************************************************************/

extern "C" void HAL_SPI_TxRxCpltCallback( SPI_HandleTypeDef *hspi )
{
 /*
	app.DebOut( "ADC_ROUTING_OUTPUT = 0,  //  monitoring %d", ADC::GetValue(  ADC_ROUTING_OUTPUT  ));
	app.DebOut( "ADC_ROUTING_POWER,       //   on/off    %d", ADC::GetValue(  ADC_ROUTING_POWER     )); 
	app.DebOut( "ADC_SOUNDER_CIRCUIT_1,	  //on/off       %d", ADC::GetValue(  ADC_SOUNDER_CIRCUIT_1));	
	app.DebOut( "ADC_SOUNDER_CIRCUIT_2,    //on/off      %d", ADC::GetValue(  ADC_SOUNDER_CIRCUIT_2)); 
	app.DebOut( "ADC_SOUNDER_MONITORING,   // monitoring %d", ADC::GetValue(  ADC_SOUNDER_MONITORING));
	app.DebOut( "ADC_CALL_POINT_INPUT,                   %d", ADC::GetValue(  ADC_CALL_POINT_INPUT ));
	app.DebOut( "ADC_ROUTING_ACK_INPUT,                  %d", ADC::GetValue(  ADC_ROUTING_ACK_INPUT));
	app.DebOut( "ADC_SILENCE_INPUT,                      %d", ADC::GetValue(  ADC_SILENCE_INPUT    ));
	app.DebOut( "ADC_ALARM_INPUT,                        %d", ADC::GetValue(  ADC_ALARM_INPUT      ));

	app.DebOut( "ADC_PSU_24V_SUUPLY,							  %d", ADC::GetValue( ADC_PSU_24V_SUUPLY		));
	app.DebOut( "ADC_DC_5V_SUPPLY,            			  %d", ADC::GetValue( ADC_DC_5V_SUPPLY     ) );
	app.DebOut( "ADC_PSU_FAULT_RELAY,         			  %d", ADC::GetValue( ADC_PSU_FAULT_RELAY  ) );
	app.DebOut( "ADC_EARTH_FAULT,             			  %d", ADC::GetValue( ADC_EARTH_FAULT      ) );
	app.DebOut( "ADC_AUX_24V,                 			  %d", ADC::GetValue( ADC_AUX_24V          ) );
*/ 
	memcpy( output, rxData, sizeof(rxData) );
	ADC::complete = true;
}

uint16_t 		ADC::pin = 0;
GPIO_TypeDef* 	ADC::port = NULL;

/*************************************************************************/
/**  \fn      ADC::ADC( )
***  \brief   Constructor for class
**************************************************************************/

ADC::ADC( ) : Module( "ADC", 100 )
{	

}


/*************************************************************************/
/**  \fn      ADC::Init( )
***  \brief   Constructor for class
**************************************************************************/

int ADC::Init( )
{	
	 return true;
}


/*************************************************************************/
/**  \fn      int ADC::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int ADC::Receive( Message* m )
{
	if ( m->type == MSG_ABORT_IT )
	{
		HAL_SPI_Abort( app.adc_spi );
	}
   return 0;
}


/*************************************************************************/
/**  \fn      HAL_StatusTypeDef ADC::TriggerChannel( int channel )
***  \brief   Class function
**************************************************************************/

HAL_StatusTypeDef ADC::TriggerChannels( )
{
 	HAL_GPIO_WritePin( ADC1_SELECT_GPIO_Port, ADC1_SELECT_Pin, GPIO_PIN_SET );
 	HAL_GPIO_WritePin( ADC::port = ADC0_SELECT_GPIO_Port, ADC::pin = ADC0_SELECT_Pin, GPIO_PIN_RESET );
		
	return HAL_SPI_TransmitReceive_IT( app.adc_spi, txData, rxData, sizeof( txData ) );	
}


static void ResetADC( )
{
	// Unselect ready for next 
	HAL_SPI_Abort_IT( app.adc_spi );	// Can timeout so, using async version. hopefully is sorted by next adc requst.
	
	HAL_GPIO_WritePin( ADC0_SELECT_GPIO_Port, ADC0_SELECT_Pin, GPIO_PIN_SET );	

	for( int i = 0; i < MAX_ADC_CHANNELS * 2; i++ )
	{
		first[ i] = second[ i] = third[ i] = 2250;
	}	
}

extern "C" void HAL_SPI_ErrorCallback( SPI_HandleTypeDef* hspi ) 
{
	// Unselect ready for next 
	ResetADC( );
}

	
extern "C" void HAL_SPI_IRQHandler2( SPI_HandleTypeDef* hspi )
{
	int step = hspi->RxXferCount & 3;
	
	if ( hspi->RxXferCount == 8 * 4 )
	{
		ADC::port = ADC1_SELECT_GPIO_Port;
		ADC::pin  = ADC1_SELECT_Pin;
	}
	
	if ( step == 0 && hspi->RxXferCount != 0 )
	{
		//HAL_GPIO_WritePin( ADC::port, ADC::pin, GPIO_PIN_RESET );	
		ADC::port->BSRR = (uint32_t)ADC::pin << 16;
	}
	else if ( step == 1 )
	{
		//HAL_GPIO_WritePin( ADC::port, ADC::pin, GPIO_PIN_SET );
		ADC::port->BSRR = ADC::pin;
	}
}


/*************************************************************************/
/**  \fn      int ADC::GetValue( )
***  \brief   Class function
**************************************************************************/

#define MARGIN 10



#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))



int ADC::GetValue( int channel )
{	
	return output[ channel * 4 + 2] + (( output[ channel * 4 + 1] & 15 ) << 8 );
}




int ADC::GetAvgValue( int channel )
{	
	third[ channel ] = second[ channel ];
	second[ channel ] = first[ channel ];
	first[ channel ] = output[ channel * 4 + 2] + (( output[ channel * 4 + 1] & 15 ) << 8 );
 	
	int max = MAX( first[ channel ], MAX( second[ channel ], third[ channel ] ) );
	int min = MIN( first[ channel ], MIN( second[ channel ], third[ channel ] ) );
	int mid;
	
	if ( first[ channel ] == max )
	{
		if ( second[ channel ] == min )
		{
			 mid = third[ channel ];  //  second[ channel ], third[ channel ], first[ channel ]
		}
		else 
		{
			mid = second[ channel ];    // third[ channel ], second[ channel ],  first[ channel ]   
		}
	}
	else
	{
		if ( second[ channel ] == min ) 
		{
			mid = first[ channel ];		// second[ channel ], first[ channel ], third[ channel ]
		}
		else 
		{
			if ( third[ channel ] == max ) 
			{
				mid = second[ channel ];   //  first[ channel ], second[ channel ], third[ channel ]
			}
			else   // second[ channel ] == max
			{
				if ( first[ channel ] == min )
				{
					mid = third[ channel ]; // first[ channel ], third[ channel ], second[ channel ]
				}
				else 
				{
					mid = first[ channel ];  // third[ channel ], first[ channel ], second[ channel ]
				}
			}
		}
	}
	
	if ( max - mid < mid - min - MARGIN )
	{
		return ( mid + max ) / 2;
	}
	else if ( max - mid - MARGIN > mid - min )
	{
		return ( min + mid ) / 2;
	}
	else
	{
		return ( min + mid + max ) / 3;
	}	
}



/*************************************************************************/
/**  \fn      int ADC::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int ADC::Receive( Event event )
{	
	return 0;
}

/*************************************************************************/
/**  \fn      int ADC::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int ADC::Receive( Command* cmd )
{
	switch ( cmd->type )
	{
	
		default: return CMD_ERR_UNKNOWN_CMD;
	}
}	
	
 
/*************************************************************************/
/**  \fn      int ADC::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

void ADC::Poll( )
{
	static int trys = 0;
	
	if ( !complete )
	{
		trys++;
		if ( trys > 5 )
		{
			Fault::AddFault( FAULT_ADC_FAILURE );		 
			ResetADC( );
		}
	}	
	else 	
	{
		if ( trys > 5 ) Fault::RemoveFault( FAULT_ADC_FAILURE );	
		trys = 0;
		PanelIO::ADCComplete( );
	}
	TriggerChannels( );
	complete = false;	
}
