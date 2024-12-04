/***************************************************************************
* File name: MM_EEPROM.cpp
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
* EEPROM Manager - NOTE:  Variables will not be accessible when
* settingDefaults is true. 
*
**************************************************************************/
#ifndef _MM_EEPROM_H_
#define _MM_EEPROM_H_

 

/* System Include Files
**************************************************************************/



/* Defines
**************************************************************************/
 
#define SetVar( member, value )  Write( (uint8_t*) &(value), sizeof( ((EEPROMMap*)0)->member), (int)( &((EEPROMMap*)0)->member ) )
#define GetVar( member, value )  Read( (uint8_t*) &(value), sizeof( ((EEPROMMap*)0)->member), (int)( &((EEPROMMap*)0)->member ) )




/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "CO_Site.h"
#include "MM_Log.h"


/* Globals
**************************************************************************/
 
 

typedef struct
{	 
	char		PanelName[ SITE_MAX_STRING_LEN ];	
	short		SMS_Enabled;
	short		AppSupp_Enabled;																
	int 		CurrentSiteAddress; 				
	int 		PreviousSiteAddress; 				

	short    GSM_MNC;	
	short		GSM_MCC;
	short		GSM_PIN;
 
	short 	TouchXmin; 							
	short 	TouchYmin; 							
	short 	TouchXmax; 							
	short 	TouchYmax; 							
	
	short 	ActiveBrightness; 					
	short 	InactiveBrightness; 				
	short 	BatteryActiveBrightness; 		
	short 	BatteryInactiveBrightness; 	
	
	short		checksumValid;
	int		checksumValue;

	char 		AppSupp_SiteId[ 40];
	char		AppSupp_Password[ 20];
	char 		AppSupp_APN[ 60];
	char		AppSupp_URL[ 60];
	
	short     ExpectedZoneLEDs;
} EEPROMMap;
 

class QuadSPI;


class EEPROM : public Module
{
	public:
	
	EEPROM( );

	static int 	device;
	static EEPROMMap defaults;
	
	virtual int Receive( Message* );
	virtual int Receive( Event );
	virtual int Receive( Command* cmd );
	
	int Set( uint8_t* val, int address );
	int Get( uint8_t* val, int address ); 
	
	int SetDefaults( int factory_reset );

	void GetCurrent( EEPROMMap* dump );
	
	static int Write( uint8_t* value, int size, int address );
	static int Read( uint8_t* value, int size, int address );
	static int WritePartial( uint8_t* value, int size, int address );
	static int ReadPartial( uint8_t* value, int size, int address );
	 
};

 

#endif
