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
#ifndef _GPIO_MONITOR_
#define _GPIO_MONITOR_


/* Define
**************************************************************************/
#define MFX_I2C_ADDR  0x84
#define PCA9535_ADDRESS (0x40)


/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/


#ifdef  __cplusplus

#include "MM_Application.h"
#include "MM_Module.h"

#endif


#include "stmpe811.h"
#include "stmpe1600.h"
#include "pca9535e.h"


extern IO_DrvTypeDef mcu_io_drv;

#define IO_PIN_0                  ((uint32_t)0x0001)
#define IO_PIN_1                  ((uint32_t)0x0002)
#define IO_PIN_2                  ((uint32_t)0x0004)
#define IO_PIN_3                  ((uint32_t)0x0008)
#define IO_PIN_4                  ((uint32_t)0x0010)
#define IO_PIN_5                  ((uint32_t)0x0020)
#define IO_PIN_6                  ((uint32_t)0x0040)
#define IO_PIN_7                  ((uint32_t)0x0080)
#define IO_PIN_8                  ((uint32_t)0x0100)
#define IO_PIN_9                  ((uint32_t)0x0200)
#define IO_PIN_10                 ((uint32_t)0x0400)
#define IO_PIN_11                 ((uint32_t)0x0800)
#define IO_PIN_12                 ((uint32_t)0x1000)
#define IO_PIN_13                 ((uint32_t)0x2000)
#define IO_PIN_14                 ((uint32_t)0x4000)
#define IO_PIN_15                 ((uint32_t)0x8000)
#define IO_PIN_16               ((uint32_t)0x010000)
#define IO_PIN_17               ((uint32_t)0x020000)
#define IO_PIN_18               ((uint32_t)0x040000)
#define IO_PIN_19               ((uint32_t)0x080000)
#define IO_PIN_20               ((uint32_t)0x100000)
#define IO_PIN_21               ((uint32_t)0x200000)
#define IO_PIN_22               ((uint32_t)0x400000)
#define IO_PIN_23               ((uint32_t)0x800000)
#define IO_PIN_ALL              ((uint32_t)0xFFFFFF)  

typedef enum
{
	GPIO_PRIVATE = -1,			// privately managed

	 
	GPIO_EEPROMEnable = 0,
	GPIO_SpecialButton,
	GPIO_I2C1Heartbeat,
	
	GPIO_DelaysActive,
	GPIO_TestMode,			
	GPIO_OtherEvents,     
	GPIO_Disablements,    
	GPIO_FireRoutingActive,
	GPIO_FireRoutingFault,  		
	GPIO_PSUFault,    
	GPIO_SounderFault,		
	
	GPIO_I2C2Heartbeat, 			
	GPIO_ExpanderSpare1,  		
	GPIO_ExpanderSpare2,   		
	GPIO_SDCardDetect,       	
	GPIO_KeySwitch,				
	GPIO_13,   			    
	GPIO_14,       		
	GPIO_15,       		

	GPIO_MCUSpare1,		
	GPIO_MCUSpare2,		
	GPIO_MCUSpare3,		

	GPIO_Fire,			
	GPIO_GenFault, 		
	GPIO_SysFault, 		
	GPIO_UART6_DE,		
	GPIO_Buzzer,
	GPIO_END,	
} GPIO;


typedef struct
{	
	GPIO 					id;
	uint32_t				pin;
	IO_DrvTypeDef*		driver;
	uint16_t				device;			
	IO_ModeTypedef		mode;
	union
	{
		uint64_t			event;
		int 				setting;
	};		
} GPIODef;


#define MAX_ZONE_BOARDS 6


extern int zonedev[ MAX_ZONE_BOARDS ];

#ifdef  __cplusplus



class GPIOManager : public Module
{
	public:
	
	GPIOManager( );

	virtual int Receive( Message* );
	virtual int Receive( Event );
	virtual int Receive( Command* );
	
	void Configure( GPIODef* def );
	void TriggerADC( GPIO gpio );
	int  GetADC( GPIO gpio );
	
	static void Set( GPIO gpio, int state );
	static void Set( GPIODef* def, int state );
	
	static int  Get( GPIO gpio );
	static int  Get( GPIODef* def );
	
		
	static void SetZone( int n, IO_PinState state );
	static void SetZones( int board, int bits, int state );
	
	uint16_t touchx, touchy;
	int touchdown;
	int touchgot;
	
	static int led_zones;
};

#else

 

#endif

#endif

