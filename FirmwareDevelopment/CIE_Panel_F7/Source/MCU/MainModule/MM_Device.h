/***************************************************************************
* File name: MM_Device.h
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
* device structures
*
**************************************************************************/
#ifndef _MM_DEVICE_H_
#define _MM_DEVICE_H_


/* System Include Files
**************************************************************************/
#include <stdint.h>
#include <time.h>

#ifdef STM32F767xx
#include "stm32f7xx_hal_rtc.h"
#else
#include "stm32f4xx_hal_rtc.h"
#endif

/* User includes
**************************************************************************/

#include "CO_Site.h"

/* Defines
**************************************************************************/

#define INPUT_ASSERTED   		   1
#define INPUT_SILENCED			   2
#define INPUT_LATCHED			   4
#define INPUT_RULED_DISABLED	   8
#define INPUT_RULED_ON_TEST	  16
#define INPUT_ON_TEST		     32
#define INPUT_IN_FAULTS		     64
#define INPUT_ACTIVE_ON_TEST   128
#define INPUT_ACTIVE			    256
#define INPUT_NOTICED_ON_TEST	 512
#define INPUT_NOT_FOUND	 		1024


#define OUTPUT_DELAYED		  	2048
 
#define OUTPUT_RULED_DISABLED	8192
#define OUTPUT_ON_TEST		  16384
#define OUTPUT_ASSERTED		  32768

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
  
 

#define SETTING_DISABLE_FIRE_EXPANDED		 1
#define SETTING_DISABLE_EXPANDED				 2
#define SETTING_ON_TEST 						 4
#define SETTING_TEST_EXPANDED	  				 8
#define SETTING_DISABLE_ROUTING_EXPANDED	16
#define SETTING_RULED_ON_TEST					32
#define SETTING_RULED_DISABLED				64
#define SETTING_RULED_LIGHT_ZONE				128



#ifndef __cplusplus
typedef void* Module;
#endif

#ifdef __cplusplus
class Module;



class Device	
{	
	public:
		
	Device( void* ptr, DeviceConfig* config, Module* m = NULL );

	
	// Input 

	int IsDisabled( );
	
	int IsInputChannel( int channel );
	int IsLatched( int channel );
	int IsSustained( int channel, int& period );
	int IsInputInverted( int channel );
	int InputAction( int channel );
	
	int IsInputSkippingOutputDelays( int channel );
	int IsInputValid( int channel );
	int IsInputDisabled( int channel );
	int IsInputOnTest( int channel );
		
	void InputAssert( int channel, time_t time, int value );
	void InputDeAssert( int channel, time_t time, int value );
	
	int InTestMode( int channel );
	void UnSetTestMode( int channel );
	
	private:
	
	void AssertInput( int channel, time_t time, int value );
	void DeAssertInput( int channel );
	
	public:

	
	// Output
	
	int OutputActions( int channel );
	
	int IsSilenceable( int channel );
	int IsOutputChannel( int channel );
	int IsOutputInverted( int channel );
	int IsOutputDisabled( int channel );	
	int IsOutputIgnoringNightDelays( int channel );
	
	void OutputAssert( int channel, int action );
	void OutputDeAssert( int channel );
	
#else
typedef struct
{	
	void*    __vptr;
	#endif	

	void*		ptr;
	Module*	owner;
	DeviceConfig*  config;
 
	unsigned short flags[ MAX(SITE_MAX_INPUT_CHANNELS,SITE_MAX_OUTPUT_CHANNELS) ];
	char				menuSelection[ SITE_MAX_INPUT_CHANNELS/2 ];
	char				analogue[ SITE_MAX_ANALOGUE_CHANNELS ];
	time_t			timeAsserted[ SITE_MAX_INPUT_CHANNELS ];
 	time_t			prevAssertion[ SITE_MAX_INPUT_CHANNELS ];
		
 	unsigned char 	profile[ SITE_MAX_OUTPUT_CHANNELS ];
	unsigned char  current_profile[ SITE_MAX_OUTPUT_CHANNELS ];
 	unsigned char  settings;
	
 
#ifndef __cplusplus	
} Device;
#else	
	virtual void OutputChange( int channel, int action, int value );
	virtual void Indicate( int asserted, int prof );
 
	virtual int Broadcasted( );
	virtual int Broadcast( int silenceable_status, int unsilenceable_status, int skip_bits );

};
#endif


#undef MAX

	

	
#endif
