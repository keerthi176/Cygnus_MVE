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
* Panel I/O Manager
*
**************************************************************************/
#ifndef _Panel_IO_
#define _Panel_IO_


/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Module.h"
#include "MM_Device.h"
#include "MM_GPIO.h"
#include "CO_Site.h"
#include "MM_IOStatus.h"
#include "MM_Settings.h"




class PanelIO;


enum PanelInputIndex
{
	PANEL_INPUT_UNKNOWN = -1,
	PANEL_INPUT_ROUTING_ACK = 0,
	PANEL_INPUT_CALL_POINT,
	PANEL_INPUT_ALARM,
	PANEL_INPUT_SILENCE,
	PANEL_INPUT_FAULT,
};

enum PanelOutputIndex
{
	PANEL_OUTPUT_UNKNOWN = -1,
	PANEL_OUTPUT_ALARM = 0,
	PANEL_OUTPUT_FAULT,
	PANEL_OUTPUT_FIRE,
	PANEL_OUTPUT_FIRE_ROUTING,
	PANEL_OUTPUT_SOUNDERS,
};	


typedef enum
{
	STATE_UNKNOWN   	= 0,
	STATE_ASSERTED    = 1,
	
	STATE_OPEN_CIRCUIT = 2,
	STATE_SHORT_CIRCUIT = 4,
	STATE_SUPPLY_FAIL	= 8, 
	STATE_CIRCUIT1_FAIL = 16,
	STATE_CIRCUIT2_FAIL = 32,
	STATE_TEST_TONE = 64,
} ADCState;


typedef enum
{
	ADC_ROUTING_OUTPUT = 0,  //  monitoring 
	ADC_ROUTING_POWER,       //   on/off
	ADC_SOUNDER_CIRCUIT_1,	  //on/off
	ADC_SOUNDER_CIRCUIT_2,    //on/off
	ADC_SOUNDER_MONITORING,   // monitoring
	ADC_CALL_POINT_INPUT,
	ADC_ROUTING_ACK_INPUT,
	ADC_SILENCE_INPUT,
	
	ADC_ALARM_INPUT,
	ADC_FAULT_INPUT,	
	ADC_PSU_24V_SUPPLY,	// 10 / 110k * 24
	ADC_DC_5V_SUPPLY,		//  /2 = 2.5
	ADC_PSU_FAULT_RELAY,	// ??
	ADC_EARTH_FAULT,		// 10 / 110 * 15
	ADC_AUX_24V,			// 10 / 110 * 24
	ADC_SPARE,
	ADC_NONE = 99
} ADCChannel;

	
struct PanelInput
{	
	ADCChannel 	ADCchannel;
	int 			ADClevel;	
	short		 	state;	
};


struct PanelOutput
{
	ADCChannel 		ADCchannel;	
	GPIO_TypeDef* 	port;
	uint16_t			pin;
	short		 	   state;	
};



 

struct PanelDevice : public Device
{
	PanelDevice( PanelOutput*, DeviceConfig*, PanelIO* );
	
	virtual void OutputChange( int channel, int action, int value );
};



struct PanelFunctionButtonDevice : public Device
{
	PanelFunctionButtonDevice( DeviceConfig* dc );
};
 
 
class PanelIO : public Module
{
	public:
	
	PanelIO( );
	
	virtual int Init( );
	
	void InitSite( );

	virtual int Receive( Message* );
	virtual int Receive( Event );
	virtual int Receive( Command* );
	
	virtual void Poll( );
	
	void CheckOutput( PanelOutputIndex outp, int chanIndex, ChannelType chantype, int level, Device* dev );

	static void ADCComplete( );
	
	static int adc_complete;
 
	int test_tone_count;	
	
	static DeviceConfig* panelio_config;
	
	private:
	
	Settings* settings;
	IOStatus* iostatus;
	
	DeviceConfig* buttons_config;
};


#endif
