/***************************************************************************
* File name: MM_TouchScreen.h
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
* TouchScreen control module
*
**************************************************************************/


#ifndef _TOUCH_SCREEN_MONITOR_
#define _TOUCH_SCREEN_MONITOR_

/* System Include Files
**************************************************************************/
#include "common/ts.h"


/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_EEPROM.h"
#include "ewrte.h"
#include "MM_PSU.h"


/* Defines
**************************************************************************/


class GPIOManager;
class EmbeddedWizard;


/* Default calibration values */

#define XMIN  0x62
#define YMIN  0x99
#define XMAX  0xFB9
#define YMAX  0xEAB 

#define TS_SWAP_NONE                    0x00
#define TS_SWAP_X                       0x01
#define TS_SWAP_Y                       0x02
#define TS_SWAP_XY                      0x04  
 

typedef struct
{
  uint16_t TouchDetected;
  uint16_t X;
  uint16_t Y;
  uint16_t Z;
}TS_StateTypeDef; 
 
typedef enum 
{
  TS_OK       = 0x00,
  TS_ERROR    = 0x01,
  TS_TIMEOUT  = 0x02
} TS_StatusTypeDef;

#ifdef __cplusplus

class IOStatus;

class TouchScreen : public Module
{
	public:
	
	TouchScreen( );
	virtual int Init( );
	
	virtual int Receive( Command* cmd );
	virtual int Receive( Event event );
	virtual int Receive( Message* );
	
	virtual void Poll( );
	
	int LoadSettings( );
	
	void EnableIT( void );
	void DisableIT( void );
	void GetState( TS_StateTypeDef* TsState );
	
	int Calibrate( int xmin, int ymin, int xmax, int ymax, int x_xmin, int x_ymin, int x_xmax, int x_ymax );	
	int GetPosition( XPoint* aPos );
	
	short ts_xmin;
	short ts_ymin;
	short ts_xmax;
	short ts_ymax;
	
	IOStatus* iostatus;

	static time_t 	last_tap;
	int	 	gone_to_fire;
	uint16_t xraw, yraw;
	
	int backlight_off;
	
	TS_DrvTypeDef*	 	TsDrv;
	uint16_t 			TsAddr;
	
	GPIOManager* 		gpios;
	EEPROM*				eeprom;
	EmbeddedWizard*  	ew;
	Module*				psu;
};

#endif
#endif


