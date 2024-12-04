/***************************************************************************
* File name: GPIO.cpp
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
* GPIO control module
*
**************************************************************************/

/* System Include Files
**************************************************************************/
#include <assert.h>

#ifdef STM32F767xx	
#include "stm32f7xx_hal_gpio.h"
//#include "stm32f769i_eval_io.h"
#else
#include "stm32f4xx_hal_gpio.h"
#include "stm32469i_eval_io.h"
#endif


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_GPIO.h"
#include "MM_fault.h"

#include "MM_Events.h"
#include "MM_Message.h"
#include "MM_EmbeddedWizard.h"
#include "MM_EEPROM.h"
#include "MM_Settings.h"



/* Defines
**************************************************************************/


int zonedev[ MAX_ZONE_BOARDS ] = { PCA9535_ADDRESS + 4, PCA9535_ADDRESS + 6,  PCA9535_ADDRESS + 12, 
											  PCA9535_ADDRESS + 14, PCA9535_ADDRESS + 8,  PCA9535_ADDRESS + 10 };

int GPIOManager::led_zones;
											  
/************************************************************************** 
***  \bried Empty functions
**************************************************************************/

void       MCU_Init(uint16_t)   { }
uint16_t   MCU_ReadID(uint16_t) { return 0; }
void       MCU_Reset(uint16_t)  { }         
void       MCU_Start(uint16_t, uint32_t) 		{ }
void 		  MCU_EnableIT( uint16_t )				{ }
void       MCU_DisableIT(uint16_t)				{ }
uint32_t   MCU_ITStatus(uint16_t, uint32_t)	{ return 0; }
void       MCU_ClearIT(uint16_t, uint32_t)	{ }



/*************************************************************************/
/**  \fn		MCU_Config( uint16_t, uint32_t pin, IO_ModeTypedef mode )
***  \brief	Config function
**************************************************************************/

uint8_t  MCU_Config( uint16_t port, uint32_t pin, IO_ModeTypedef mode )
{
   GPIO_InitTypeDef GPIO_InitStruct;
   
   GPIO_InitStruct.Pin   = pin;
   
   switch ( mode )
   {
      case IO_MODE_INPUT:   /* input floating */
         GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
         GPIO_InitStruct.Pull  = GPIO_NOPULL;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;      
      case IO_MODE_OUTPUT:      /* output Push Pull */
         GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
         GPIO_InitStruct.Pull  = GPIO_NOPULL;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_IT_RISING_EDGE:   /* float input - irq detect on rising edge */
         GPIO_InitStruct.Mode  = GPIO_MODE_IT_RISING;
         GPIO_InitStruct.Pull  = GPIO_NOPULL;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_IT_FALLING_EDGE:  /* float input - irq detect on falling edge */
         GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
         GPIO_InitStruct.Pull  = GPIO_NOPULL;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_IT_LOW_LEVEL:     /* float input - irq detect on low level */
         GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
         GPIO_InitStruct.Pull  = GPIO_NOPULL;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_IT_HIGH_LEVEL:    /* float input - irq detect on high level */
         GPIO_InitStruct.Mode  = GPIO_MODE_IT_RISING;
         GPIO_InitStruct.Pull  = GPIO_NOPULL;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_ANALOG:           /* analog mode */
         GPIO_InitStruct.Mode  = GPIO_MODE_ANALOG;
         GPIO_InitStruct.Pull  = GPIO_NOPULL;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_OFF:              /* when pin isn't used*/
         GPIO_InitStruct.Mode  = GPIO_MODE_ANALOG;
         GPIO_InitStruct.Pull  = GPIO_NOPULL;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_INPUT_PU:         /* input with internal pull up resistor */
         GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
         GPIO_InitStruct.Pull  = GPIO_PULLUP;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_INPUT_PD:         /* input with internal pull down resistor */
         GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
         GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_OUTPUT_OD:          /* Open Drain output without internal resistor */
         GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
         GPIO_InitStruct.Pull  = GPIO_NOPULL;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_OUTPUT_OD_PU:       /* Open Drain output with  internal pullup resistor */
         GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
         GPIO_InitStruct.Pull  = GPIO_PULLUP;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_OUTPUT_OD_PD:       /* Open Drain output with  internal pulldown resistor */
         GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;
         GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_OUTPUT_PP:          /* PushPull output without internal resistor */
         GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
         GPIO_InitStruct.Pull  = GPIO_NOPULL;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_OUTPUT_PP_PU:       /* PushPull output with  internal pullup resistor */
         GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
         GPIO_InitStruct.Pull  = GPIO_PULLUP;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_OUTPUT_PP_PD:       /* PushPull output with  internal pulldown resistor */
         GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
         GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_IT_RISING_EDGE_PU:   /* push up resistor input - irq on rising edge  */
         GPIO_InitStruct.Mode  = GPIO_MODE_IT_RISING;
         GPIO_InitStruct.Pull  = GPIO_PULLUP;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_IT_RISING_EDGE_PD:   /* push dw resistor input - irq on rising edge  */
         GPIO_InitStruct.Mode  = GPIO_MODE_IT_RISING;
         GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_IT_FALLING_EDGE_PU:  /* push up resistor input - irq on falling edge */
         GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
         GPIO_InitStruct.Pull  = GPIO_PULLUP;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_IT_FALLING_EDGE_PD:  /* push dw resistor input - irq on falling edge */
         GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
         GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_IT_LOW_LEVEL_PU:     /* push up resistor input - irq detect on low level */
         GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
         GPIO_InitStruct.Pull  = GPIO_PULLUP;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_IT_LOW_LEVEL_PD:     /* push dw resistor input - irq detect on low level */
         GPIO_InitStruct.Mode  = GPIO_MODE_IT_FALLING;
         GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_IT_HIGH_LEVEL_PU:    /* push up resistor input - irq detect on high level */
         GPIO_InitStruct.Mode  = GPIO_MODE_IT_RISING;
         GPIO_InitStruct.Pull  = GPIO_PULLUP;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
      case IO_MODE_IT_HIGH_LEVEL_PD:    /* push dw resistor input - irq detect on high level */
         GPIO_InitStruct.Mode  = GPIO_MODE_IT_RISING;
         GPIO_InitStruct.Pull  = GPIO_PULLDOWN;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
         break;
		case IO_MODE_IT_RISING_FALLING:
			GPIO_InitStruct.Mode  = GPIO_MODE_IT_RISING_FALLING;
         GPIO_InitStruct.Pull  = GPIO_NOPULL;
         GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; 
   }
      
   HAL_GPIO_Init( (GPIO_TypeDef*) ( AHB1PERIPH_BASE + port ), &GPIO_InitStruct );   
	
	return 0;
}


/*************************************************************************/
/**  \fn		void MCU_WritePin( uint16_t, uint32_t pin, uint8_t state )
***  \brief	Config function
**************************************************************************/

int MCU_WritePin( uint16_t port, uint32_t pin, uint8_t state )
{
   if ( state != 0 )
   {
      HAL_GPIO_WritePin( (GPIO_TypeDef*) ( AHB1PERIPH_BASE + port ), pin, GPIO_PIN_SET );
   }
   else
   {
		HAL_GPIO_WritePin( (GPIO_TypeDef*) ( AHB1PERIPH_BASE + port ), pin, GPIO_PIN_RESET );
	}
	return 1;
}


/*************************************************************************/
/**  \fn		void MCU_ReadPin( uint16_t, uint32_t pin, uint8_t state )
***  \brief	Config function
**************************************************************************/

uint32_t MCU_ReadPin( uint16_t port, uint32_t pin )
{
	return HAL_GPIO_ReadPin( (GPIO_TypeDef*) ( AHB1PERIPH_BASE + port ), pin );
}


IO_DrvTypeDef mcu_io_drv = {
	MCU_Init,
	MCU_ReadID,
	MCU_Reset,
	MCU_Start,
	MCU_Config,
	MCU_WritePin,
	MCU_ReadPin,
	MCU_EnableIT,
	MCU_DisableIT,
	MCU_ITStatus,
	MCU_ClearIT
};	


// NOTE: Must be in order !!

static GPIODef gpio_list[ ] ={ 	
	{ GPIO_EEPROMEnable,			STMPE811_PIN_1,					&stmpe811_io_drv,    TS_I2C_ADDRESS,  		IO_MODE_OUTPUT,					IO_PIN_SET },
	{ GPIO_SpecialButton, 		STMPE811_PIN_2,					&stmpe811_io_drv,		TS_I2C_ADDRESS,  		IO_MODE_IT_FALLING_EDGE,		EVENT_CALIBRATION_REQUEST },
	{ GPIO_I2C1Heartbeat,		STMPE811_PIN_3,					&stmpe811_io_drv,		TS_I2C_ADDRESS,  		IO_MODE_OUTPUT,					IO_PIN_SET },
	
   { GPIO_DelaysActive,			STMPE1600_PIN_0,					&pca9535e_io_drv,   PCA9535_ADDRESS,		IO_MODE_OUTPUT,					IO_PIN_RESET },
	{ GPIO_TestMode,				STMPE1600_PIN_1,              &pca9535e_io_drv,   PCA9535_ADDRESS,		IO_MODE_OUTPUT,					IO_PIN_RESET },
	{ GPIO_OtherEvents,        STMPE1600_PIN_2,              &pca9535e_io_drv,   PCA9535_ADDRESS,		IO_MODE_OUTPUT,					IO_PIN_RESET },
	{ GPIO_Disablements,      	STMPE1600_PIN_3,              &pca9535e_io_drv,   PCA9535_ADDRESS,		IO_MODE_OUTPUT,					IO_PIN_RESET },
	{ GPIO_FireRoutingActive,  STMPE1600_PIN_4,              &pca9535e_io_drv,   PCA9535_ADDRESS,		IO_MODE_OUTPUT,					IO_PIN_RESET },
	{ GPIO_FireRoutingFault,  	STMPE1600_PIN_5,              &pca9535e_io_drv,   PCA9535_ADDRESS,		IO_MODE_OUTPUT,					IO_PIN_RESET }, 
	{ GPIO_PSUFault,      		STMPE1600_PIN_6,              &pca9535e_io_drv,   PCA9535_ADDRESS,		IO_MODE_OUTPUT,					IO_PIN_RESET },
	{ GPIO_SounderFault,			STMPE1600_PIN_7,		         &pca9535e_io_drv,   PCA9535_ADDRESS,		IO_MODE_OUTPUT,					IO_PIN_RESET },

	{ GPIO_I2C2Heartbeat, 		STMPE1600_PIN_8,              &pca9535e_io_drv,   PCA9535_ADDRESS,		IO_MODE_OUTPUT,					IO_PIN_SET },
	{ GPIO_ExpanderSpare1,  	STMPE1600_PIN_9,              &pca9535e_io_drv,   PCA9535_ADDRESS,		IO_MODE_OUTPUT,					IO_PIN_RESET },
	{ GPIO_ExpanderSpare2,   	STMPE1600_PIN_10,             &pca9535e_io_drv,   PCA9535_ADDRESS,		IO_MODE_OUTPUT,					IO_PIN_RESET }, 
	{ GPIO_SDCardDetect,       STMPE1600_PIN_11,             &pca9535e_io_drv,   PCA9535_ADDRESS,		IO_MODE_INPUT },
	{ GPIO_KeySwitch,				STMPE1600_PIN_12,					&pca9535e_io_drv,   PCA9535_ADDRESS,		IO_MODE_IT_RISING_FALLING,		EVENT_KEY_SWITCH }, 
	{ GPIO_13,   					STMPE1600_PIN_13,             &pca9535e_io_drv,   PCA9535_ADDRESS,		IO_MODE_OUTPUT,					IO_PIN_RESET }, 
	{ GPIO_14,       				STMPE1600_PIN_14,             &pca9535e_io_drv,   PCA9535_ADDRESS,		IO_MODE_INPUT },
	{ GPIO_15,       				STMPE1600_PIN_14,             &pca9535e_io_drv,   PCA9535_ADDRESS,		IO_MODE_INPUT },
	
		
	{ GPIO_MCUSpare1,				GPIO_PIN_3,							&mcu_io_drv,	  (uint16_t) GPIOE_BASE,	IO_MODE_OUTPUT,					IO_PIN_RESET },
	{ GPIO_MCUSpare2,				GPIO_PIN_13,						&mcu_io_drv,	  (uint16_t) GPIOC_BASE,	IO_MODE_OUTPUT,					IO_PIN_RESET },
	{ GPIO_MCUSpare3,				GPIO_PIN_4,							&mcu_io_drv,	  (uint16_t) GPIOA_BASE,	IO_MODE_OUTPUT,					IO_PIN_RESET },	
		
	{ GPIO_Fire,					FIRE_LED_Pin, 						&mcu_io_drv,       (uint16_t) GPIOJ_BASE, IO_MODE_OUTPUT,					IO_PIN_RESET },
	{ GPIO_GenFault, 				GEN_FAULT_LED_Pin,				&mcu_io_drv,       (uint16_t) GPIOH_BASE, IO_MODE_OUTPUT,					IO_PIN_LEAVE },
	{ GPIO_SysFault, 				SYS_FAULT_LED_Pin,				&mcu_io_drv,       (uint16_t) GPIOH_BASE, IO_MODE_OUTPUT,					IO_PIN_LEAVE },
	{ GPIO_UART6_DE,				GPIO_PIN_12,						&mcu_io_drv,		 (uint16_t) GPIOG_BASE,	IO_MODE_OUTPUT,					IO_PIN_RESET },		
	{ GPIO_Buzzer,					BUZZER_EN_Pin,						&mcu_io_drv,		 (uint16_t) GPIOJ_BASE,	IO_MODE_OUTPUT,					IO_PIN_LEAVE },		
};
								
static Message message = { NULL, NULL, 0 };

										
											
/*************************************************************************/
/**  \fn      GPIOManager::GPIOManager( )
***  \brief   Constructor for class
**************************************************************************/

GPIOManager::GPIOManager( ) : Module( "GPIOManager", 0 )
{
	message.to = message.from = this;
 	
	if ( stmpe811_io_drv.ReadID( TS_I2C_ADDRESS ) == STMPE811_ID )
	{
		 /* Initialize the stmpe811 IO driver */
		
		 stmpe811_io_drv.Init( TS_I2C_ADDRESS );
		 stmpe811_io_drv.Start( TS_I2C_ADDRESS, IO_PIN_ALL );
	}
	else
	{
		if ( stmpe811_io_drv.ReadID( TS_I2C_ADDRESS ) == STMPE811_ID )
		{
			 /* Initialize the stmpe811 IO driver */
			
			 stmpe811_io_drv.Init( TS_I2C_ADDRESS );
			 stmpe811_io_drv.Start( TS_I2C_ADDRESS, IO_PIN_ALL );
		}
		else
		{
			app.Report( ISSUE_I2C_FAIL );
		}
	}

	// ADC
	stmpe811_SetupADC( TS_I2C_ADDRESS, STMPE811_ADC_SAMPLE_TIME_124, STMPE811_ADC_12BITS, STMPE811_ADC_CLOCK_1_625 );
	
	stmpe811_SetITPolarity( TS_I2C_ADDRESS, STMPE811_POLARITY_LOW );
	stmpe811_SetITType( TS_I2C_ADDRESS, STMPE811_TYPE_LEVEL );
	stmpe811_EnableITSource( TS_I2C_ADDRESS, STMPE811_GIT_TOUCH );
	stmpe811_EnableITSource( TS_I2C_ADDRESS, STMPE811_GIT_ADC );
	
	stmpe811_IO_ClearIT( TS_I2C_ADDRESS, STMPE811_PIN_ALL );
	stmpe811_EnableGlobalIT( TS_I2C_ADDRESS );
	
	for( GPIODef* gpio = gpio_list; gpio < gpio_list + sizeof( gpio_list ) / sizeof( GPIODef ); gpio++ )
	{
 		assert( (int) gpio->id == gpio - gpio_list );
		
	 	Configure( gpio );
		
		if ( gpio->mode == IO_MODE_OUTPUT && gpio->setting != IO_PIN_LEAVE ) gpio->driver->WritePin( gpio->device, gpio->pin, gpio->setting );
	}	
	
	int maxleds = FACTORY_SETTINGS->leds;
	
	if ( (maxleds & 15) || maxleds < 16 || maxleds > 192 )
	{
		maxleds = 96;
	}
	
	for( int zoneboard = 0; zoneboard < MAX_ZONE_BOARDS; zoneboard++ )
	{
		if ( HAL_I2C_IsDeviceReady( &hi2c2, zonedev[ zoneboard ], 3, 200 ) == HAL_OK )
		{
			pca9535e_io_drv.WritePin( zonedev[ zoneboard ], 0xFFFF, IO_PIN_RESET ); 
			pca9535e_io_drv.Config( zonedev[ zoneboard ], 0xFFFF, IO_MODE_OUTPUT );		 
			
			led_zones += 16;
		}
	}
	
	if ( led_zones != maxleds )
	{
		Fault::AddFault( FAULT_MISSING_ZONE_LEDS );
	}
	
	if ( led_zones > maxleds )
	{
		led_zones = maxleds;
	}
	
		/* Enable EXTI Interrupt */
	HAL_NVIC_SetPriority((IRQn_Type)(EXTI0_IRQn), 0x0F, 0x0);
	HAL_NVIC_EnableIRQ((IRQn_Type)(EXTI0_IRQn));
	
	/* Enable EXTI Interrupt */
	HAL_NVIC_SetPriority((IRQn_Type)(EXTI9_5_IRQn), 0x09, 0x0);
	HAL_NVIC_EnableIRQ((IRQn_Type)(EXTI9_5_IRQn));
	
	/* Enable and set GPIO EXTI Interrupt to the lowest priority */
	HAL_NVIC_SetPriority((IRQn_Type)(EXTI15_10_IRQn), 0x0F, 0x0);
	HAL_NVIC_EnableIRQ((IRQn_Type)(EXTI15_10_IRQn));

	/* Enable and set GPIO EXTI Interrupt to the lowest priority */
	HAL_NVIC_SetPriority((IRQn_Type)(EXTI1_IRQn), 0x0F, 0x0);
	HAL_NVIC_EnableIRQ((IRQn_Type)(EXTI1_IRQn));
	
	/* Enable and set GPIO EXTI Interrupt to the lowest priority */
	HAL_NVIC_SetPriority((IRQn_Type)(EXTI2_IRQn), 0x0F, 0x0);
	HAL_NVIC_EnableIRQ((IRQn_Type)(EXTI2_IRQn));

	/* Enable and set GPIO EXTI Interrupt to the lowest priority */
	HAL_NVIC_SetPriority((IRQn_Type)(EXTI3_IRQn), 0x0F, 0x0);
	HAL_NVIC_EnableIRQ((IRQn_Type)(EXTI3_IRQn));
	
	/* Enable and set GPIO EXTI Interrupt to the lowest priority */
	HAL_NVIC_SetPriority((IRQn_Type)(EXTI4_IRQn), 0x0F, 0x0);
	HAL_NVIC_EnableIRQ((IRQn_Type)(EXTI4_IRQn));
		

	stmpe811_IO_EnableIT( TS_I2C_ADDRESS );
} 

enum
{
	MSG_GPIO_PCA9535_IRQ,
	MSG_GPIO_STMPE8_IRQ,
};


/*************************************************************************/
/**  \fn      int GPIOManager::Configure( GPIODef* gpio )
***  \brief   Class function
**************************************************************************/

void GPIOManager::Configure( GPIODef* gpio )
{
	if ( gpio->mode == IO_MODE_ANALOG )
	{
		if ( gpio->driver == &stmpe811_io_drv )
		{
			stmpe811_IO_EnableAF( TS_I2C_ADDRESS, gpio->pin );
			
			stmpe811_EnableADC( TS_I2C_ADDRESS, adc_pin[ gpio->pin] );
		}				
	}
	else
	{
		if ( gpio->driver == &stmpe811_io_drv ) stmpe811_IO_DisableAF( TS_I2C_ADDRESS, gpio->pin );
		gpio->driver->Config( gpio->device, gpio->pin, gpio->mode );
	}	
}

static int boardfail = 0;

/*************************************************************************/
/**  \fn      int GPIOManager::SetZone( int n, GPIO_PinState  )
***  \brief   Class function
**************************************************************************/

void GPIOManager::SetZone( int n, IO_PinState state )
{
	n--;
	
	int board = n  / 16;
	int pin   = 15 - ( n  & 15 ); 
	
	if ( n < led_zones )
	{
		int boardf = boardfail;
		
		if ( ! pca9535e_IO_WritePin( zonedev[ board ], 1 << pin, state ) )
		{
			boardfail |= 1 <<  board;
		}
		else 
		{
			boardfail &= ~( 1 <<  board );
		}
		
		if ( !!boardfail != !!boardf )
		{
			if ( boardfail )
			{
				Fault::AddFault( FAULT_ZONE_LED_FAIL );
			}
			else	
			{
				Fault::RemoveFault( FAULT_ZONE_LED_FAIL );
			}
		}
	}
}


/*************************************************************************/
/**  \fn     void GPIOManager::SetZones( int board, int bits, int state )
***  \brief   Class function
**************************************************************************/

void GPIOManager::SetZones( int board, int bits, int state )
{	
	if ( board < MAX_ZONE_BOARDS )
	{
		int boardf = boardfail;
		
		if ( ! pca9535e_IO_WritePin( zonedev[ board ], bits, state ) )
		{
			boardfail |= 1 <<  board;
		}
		else 
		{
			boardfail &= ~( 1 <<  board );
		}
		
		if ( !!boardfail != !!boardf )
		{
			if ( boardfail )
			{
				Fault::AddFault( FAULT_ZONE_LED_FAIL );
			}
			else	
			{
				Fault::RemoveFault( FAULT_ZONE_LED_FAIL );
			}
		}
	}
}


/*************************************************************************/
/**  \fn      int GPIOManager::Receive( Message* m )
***  \brief   Class function
**************************************************************************/

int GPIOManager::Receive( Message* m )
{
	if ( m->type == MSG_GPIO_PCA9535_IRQ )
	{
		for( GPIODef* gpio = gpio_list; gpio < gpio_list + sizeof( gpio_list ) / sizeof( GPIODef ); gpio++ )
		{
			// Is pca9535
			if ( gpio->driver == &pca9535e_io_drv )
			{
				if ( gpio->mode == IO_MODE_IT_RISING_FALLING )
				{
					// Generate event
					if ( gpio->event != 0 ) app.Send( (Event) gpio->event );
				}
			}		
		}
	}
		
	if ( m->type == MSG_GPIO_STMPE8_IRQ )
	{	
	//	uint8_t global_irq = stmpe811_ReadGITStatus( TS_I2C_ADDRESS, STMPE811_GIT_TOUCH
		//																				| STMPE811_GIT_ADC
	//																					| STMPE811_GIT_IO );
		
		/*
		if ( global_irq & STMPE811_GIT_TOUCH )
		{
			stmpe811_ClearGlobalIT( TS_I2C_ADDRESS, STMPE811_TS_IT );
			touchdown = stmpe811_TS_GetCtrl( TS_I2C_ADDRESS ) & 128;
			if ( touchdown )
			{
#ifndef BD_TEST_ONLY
				app.DebOut( "touch down\n" );
#endif
				
				// In case of a quick touch, we ensure at least one coord is detected			
				touchgot = false; // flag that none detected yet
			}
			else 
			{
#ifndef BD_TEST_ONLY
				app.DebOut( "touch up\n" );
#endif
			}
				
		//	app.DebOut( "stmpe811 touch irq \n", X, Y );
		}
		
		if ( global_irq & STMPE811_GIT_ADC )
		{
			stmpe811_ClearGlobalIT( TS_I2C_ADDRESS, STMPE811_GIT_ADC );
			
			int adc_status = stmpe811_GetADCStatus( TS_I2C_ADDRESS );
			
			stmpe811_ClearADCStatus( TS_I2C_ADDRESS );
			
			if ( adc_status )
			{
				for( GPIODef* gpio = gpio_list; gpio < gpio_list + sizeof( gpio_list ) / sizeof( GPIODef ); gpio++ )
				{
					if ( gpio->mode == IO_MODE_ANALOG && gpio->driver == &stmpe811_io_drv )											
					{
						int adc = adc_pin[ gpio->pin ] ; 
						
						if ( ( 1 << adc ) & adc_status )
						{
							//app.DebOut( "adc irq.\n" );
							
							app.Send( (Event) gpio->event );
						}
					}
				}
			}
		}
		*/
		// Get stmpe pin irq status
	//	if ( global_irq & STMPE811_GIT_IO )
		{
		//	app.DebOut( "gpio irq.\n" );
			
			uint8_t int_status = stmpe811_IO_ITStatus( TS_I2C_ADDRESS, STMPE811_PIN_2 );
			stmpe811_ClearGlobalIT( TS_I2C_ADDRESS, STMPE811_GIT_TOUCH
		 																			| STMPE811_GIT_ADC
	 																			| STMPE811_GIT_IO );
			if ( int_status ) 
			{
				if ( app.panel == NULL )
				{
					AppDeviceClass_TriggerGotoSysPageEvent( app.DeviceObject );
				}
				 
				stmpe811_IO_ClearIT( TS_I2C_ADDRESS, int_status );
			}
			/*
			// Check each pin
			if ( int_status )
			{
				app.DebOut( "STMPE811 Edge Status: %X\n", int_status );
				// Clear pin irq
				stmpe811_IO_ClearIT( TS_I2C_ADDRESS, int_status );
				
				for( GPIODef* gpio = gpio_list; gpio < gpio_list + sizeof( gpio_list ) / sizeof( GPIODef ); gpio++ )
				{
					// Is stmpe811 pin?
					if ( gpio->driver == &stmpe811_io_drv )
					{
						if ( gpio->pin & int_status )
						{
							// Generate event
							if ( gpio->event != 0 ) app.Send( (Event) gpio->event );
						}
					}
				}
			}*/
		}
		
	}
		
	
	return 0;
}


/*************************************************************************/
/**  \fn      void GPIOManager::Set( GPIO gpio, int state )
***  \brief   Class function
**************************************************************************/

void GPIOManager::Set( GPIO gpio, int state )
{
	if ( gpio < GPIO_END )
	{
		GPIODef* g = gpio_list + gpio;
	
		g->driver->WritePin( g->device, g->pin, state );	
	}
}


/*************************************************************************/
/**  \fn      void GPIOManager::Set( GPIODef* g, int state )
***  \brief   Class function
**************************************************************************/

void GPIOManager::Set( GPIODef* g, int state )
{	
	g->driver->WritePin( g->device, g->pin, state );	
}


/*************************************************************************/
/**  \fn      int GPIOManager::GetADC( GPIO gpio )
***  \brief   Class function
**************************************************************************/

int GPIOManager::GetADC( GPIO gpio )
{
	GPIODef* g = gpio_list + gpio;
	
	if ( g->driver == &stmpe811_io_drv )
	{
		int adc = adc_pin[ g->pin ];
		
		return stmpe811_GetADC( TS_I2C_ADDRESS, adc ) & 0xFFF;
	}
	return 0;
}


/*************************************************************************/
/**  \fn      void GPIOManager::TriggerADC( GPIO gpio )
***  \brief   Class function
**************************************************************************/

void GPIOManager::TriggerADC( GPIO gpio )
{
	GPIODef* g = gpio_list + gpio;
	
	if ( g->driver == &stmpe811_io_drv )
	{
		int adc = adc_pin[ g->pin ];

		stmpe811_EnableADC( TS_I2C_ADDRESS, adc );
		stmpe811_TriggerADC( TS_I2C_ADDRESS, adc );
	}
}


/*************************************************************************/
/**  \fn      int GPIOManager::Get( GPIO gpio )
***  \brief   Class function
**************************************************************************/

int GPIOManager::Get( GPIO gpio )
{
	GPIODef* g = gpio_list + gpio;

	return g->driver->ReadPin( g->device, g->pin );
}

 
/*************************************************************************/
/**  \fn      int GPIOManager::Get( GPIODef* g )
***  \brief   Class function
**************************************************************************/

int GPIOManager::Get( GPIODef* g )
{
	return g->driver->ReadPin( g->device, g->pin );
}


/*************************************************************************/
/**  \fn      int GPIOManager::Receive( Event )
***  \brief   Class function
**************************************************************************/

int GPIOManager::Receive( Event )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int GPIOManager::Receive( Command* )
***  \brief   Class function
**************************************************************************/

int GPIOManager::Receive( Command* )
{
	return CMD_ERR_UNKNOWN_CMD;
}


/**
  * @brief  This function handles external lines MFX interrupt request.
  * @param  None
  * @retval None
  */
extern "C" void GPIO_3_Handler( void )
{
	message.type = MSG_GPIO_PCA9535_IRQ;		
	app.Send( &message );
}

 

extern "C" void GPIO_5_Handler( )
{
	Message m;
		
	 message.type = MSG_GPIO_STMPE8_IRQ;
	 app.Send( &message );
	 
}	
