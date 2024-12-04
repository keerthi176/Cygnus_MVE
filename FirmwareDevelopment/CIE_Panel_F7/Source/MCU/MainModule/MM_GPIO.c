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

/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_GPIO.h"

#include "MM_fault.h"


typedef enum
{
	STMPE811_IRQ,
	MicroSD,
	CalibrationButton,
	BrightnessLevel,
	EEPROMEnable,
	I2CHeartbeat,
} GPIO;


typedef struct
{	
	GPIO 					id;
	uint32_t				pin;
	IO_DrvTypeDef		driver;
	uint16_t				device;			
	IO_ModeTypedef		mode;
	IO_PinState			init;
} GPIODef;


static GPIODef gpio_list[ ] ={ { STMPE811_IRQ, 			MFX_GPIO14, 		mfxstm32l152_io_drv, MFXSTM32L152_ID_1,  	IO_MODE_IT_RISING_EDGE },  
											MicroSD, 				MFX_GPIO15, 		mfxstm32l152_io_drv, MFXSTM32L152_ID_1,  	IO_MODE_IT_RISING_EDGE },  

											CalibrationButton,	STMPE811_PIN_1,	stmpe811_io_drv,     TS_I2C_ADDRESS,  		IO_MODE_IT_RISING_EDGE },
											BrightnessLevel, 		STMPE811_PIN_2,	stmpe811_io_drv,		TS_I2C_ADDRESS,  		IO_MODE_ANALOG },
											EEPROMEnable,			STMPE811_PIN_0,	stmpe811_io_drv,		TS_I2C_ADDRESS,  		IO_MODE_OUTPUT,					IO_PIN_RESET },
											I2CHeartbeat,			STMPE811_PIN_3,	stmpe811_io_drv,		TS_I2C_ADDRESS,  		IO_MODE_OUTPUT,					IO_PIN_SET },
										};
										


											
/*************************************************************************/
/**  \fn      GPIO::GPIO( Application* app )
***  \brief   Constructor for class
**************************************************************************/

GPIO::GPIO( Application* app ) : Module( "GPIO", 0, app )
{
	for( GPIODef* gpio = gpio_list; gpio < gpio_list + sizeof( gpio_list ) / sizeof( GPIODef ); gpio++ )
	{
		gpio->driver->
		
} 


void GPIO::

