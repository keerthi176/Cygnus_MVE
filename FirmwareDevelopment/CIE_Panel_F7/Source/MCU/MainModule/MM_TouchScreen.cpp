/***************************************************************************
* File name: MM_TouchScreen.cpp
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

/* System Include Files
**************************************************************************/
#include "stmpe811.h"
#include "ft5316.h"
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_TouchScreen.h"
#include "MM_fault.h"
#include "MM_issue.h"
#include "MM_EmbeddedWizard.h"
#include "MM_CUtils.h"
#include "MM_IOStatus.h"
#include "MM_Device.h"
#include "MM_Utilities.h"



//#define TEST_HARNESS
#define BLANK_SCREEN

/* Defines
**************************************************************************/


#define XRES 800
#define YRES 480


#define XPIX(x) (((x - ts_xmin) * XRES ) / (ts_xmax - ts_xmin))
#define YPIX(y) (((y - ts_ymin) * YRES ) / (ts_ymax - ts_ymin))

#define XTOUCH(x) ((x * (ts_xmax - ts_xmin) / XRES ) + ts_xmin)
#define YTOUCH(y) ((y * (ts_ymax - ts_ymin) / YRES ) + ts_ymin)

#define XTRIM(x) (( x < 0 ? 0 : x ) < XRES ? ( x < 0 ? 0 : x ) : ( XRES - 1 ) )
#define YTRIM(y) (( y < 0 ? 0 : y ) < YRES ? ( y < 0 ? 0 : y ) : ( YRES - 1 ) )

#define SCREEN_SLEEP_TIME_IN_SECONDS              (time_t)3600
#define PSU_COMM_LAST_RECEIVED_TIMEOUT_IN_SECONDS (time_t)300


/* Globals
**************************************************************************/
time_t 	TouchScreen::last_tap = 0;


/*************************************************************************/
/**  \fn      TouchScreen::TouchScreen( )
***  \brief   Constructor for class
**************************************************************************/

TouchScreen::TouchScreen( ) : Module( "TouchScreen", 50 )
{
	gone_to_fire = false;
	backlight_off = false;
} 


/*************************************************************************/
/**  \fn      int TouchScreen::LoadSettings( )
***  \brief   Class function
**************************************************************************/

int TouchScreen::LoadSettings( )
{
	if ( !eeprom->GetVar( TouchXmin, ts_xmin ) ||
	     !eeprom->GetVar( TouchYmin, ts_ymin ) ||
	     !eeprom->GetVar( TouchXmax, ts_xmax ) ||
	     !eeprom->GetVar( TouchYmax, ts_ymax ) )
	{	
		return false;
	}
	else 
	{
		return true;
	}
}


/*************************************************************************/
/**  \fn      int TouchScreen::Init( )
***  \brief   Class function
**************************************************************************/

int TouchScreen::Init( )
{
	eeprom = (EEPROM*) 	      app.FindModule( "EEPROM" );
	ew     = (EmbeddedWizard*) app.FindModule( "EmbeddedWizard" );	
	iostatus = (IOStatus*)		app.FindModule( "IOStatus" );
	
	psu = app.FindModule( "PSU" );
	
	if ( psu == NULL )
	{
		psu = app.FindModule( "ElmPSU" );
	}
	
	assert( psu    != NULL );	
	assert( eeprom != NULL );
	assert( ew		!= NULL );
	assert( iostatus != NULL );
	
	
	if ( !LoadSettings( ) )
	{
		Fault::AddFault( FAULT_TOUCH_PARAMS_GET );
		//Log::Msg(LOG_SYS, "01_TouchScreen.cpp -- ADD");
		
		ts_xmin = EEPROM::defaults.TouchXmin;
		ts_ymin = EEPROM::defaults.TouchYmin;
		ts_xmax = EEPROM::defaults.TouchXmax;
		ts_ymax = EEPROM::defaults.TouchYmax;		
	}
	else
	{
		Fault::RemoveFault( FAULT_TOUCH_PARAMS_GET );
		//Log::Msg(LOG_SYS, "01_TouchScreen.cpp -- REMOVE");
	}
	
	/* Read ID and verify if ready */
	if ( ft5316_ts_drv.ReadID( FT5316_I2C_SLAVE_ADDRESS ) == FT5316_ID_VALUE )
	{
		TsDrv  = &ft5316_ts_drv;
		TsAddr = FT5316_I2C_SLAVE_ADDRESS;
		
	}
	else if ( stmpe811_ts_drv.ReadID( TS_I2C_ADDRESS ) == STMPE811_ID )
	{
		 /* Initialize the TS driver structure */
		 TsDrv = &stmpe811_ts_drv;
		 TsAddr = TS_I2C_ADDRESS;
	}
	else
	{
		TsDrv = NULL;
		return 0;
	}	
	
	/* Initialize the LL TS Driver */

 	TsDrv->Start(TS_I2C_ADDRESS);	
 
	last_tap = now( );
	
	return 1;
}


/*************************************************************************/
/**  \fn      int TouchScreen::Calibrate( int xmin, int ymin, int xmax, int ymax, int x_xmin, int x_ymin, int x_xmax, int x_ymax )
***  \brief   Class function
**************************************************************************/

int TouchScreen::Calibrate( int xmin, int ymin, int xmax, int ymax, int x_xmin, int x_ymin, int x_xmax, int x_ymax )
{
	int txmin = xmax;
	int txmax = xmin;
	int tymin = ymin;
	int tymax = ymax;
	
	ts_xmin = txmin - ( x_xmin * (txmax - txmin)) / (x_xmax - x_xmin);
	ts_ymin = tymin - ( x_ymin * (tymax - tymin)) / (x_ymax - x_ymin);
	
	ts_xmax = txmax + ( (XRES - x_xmax ) * (txmax - txmin)) / (x_xmax - x_xmin);
	ts_ymax = tymax + ( (YRES - x_ymax ) * (tymax - tymin)) / (x_ymax - x_ymin);	

	if ( !eeprom->SetVar( TouchXmin, ts_xmin ) ) app.Report( ISSUE_DATA_INTEGRITY );
	if ( !eeprom->SetVar( TouchYmin, ts_ymin ) ) app.Report( ISSUE_DATA_INTEGRITY );
	if ( !eeprom->SetVar( TouchXmax, ts_xmax ) ) app.Report( ISSUE_DATA_INTEGRITY );
	if ( !eeprom->SetVar( TouchYmax, ts_ymax ) ) app.Report( ISSUE_DATA_INTEGRITY );
	
	return false;
}	


/*************************************************************************/
/**  \fn      void TouchScreen::EnableIT( void )
***  \brief   Class function
**************************************************************************/

void TouchScreen::EnableIT( void )
{
   /* Enable the TS ITs */
   if ( TsDrv != NULL ) TsDrv->EnableIT(TsAddr);
}



/*************************************************************************/
/**  \fn      void TouchScreen::DisableIT( void )
***  \brief   clears all touch screen interrupts
**************************************************************************/

void TouchScreen::DisableIT( void )
{
	/* Clear TS IT pending bits */
	if ( TsDrv != NULL ) TsDrv->ClearIT( TsAddr ); 
}

static uint32_t offset_shift = 0;

/*************************************************************************/
/**  \fn      void TouchScreen::GetState( TS_StateTypeDef* TsState )
***  \brief   Class function
**************************************************************************/

void TouchScreen::GetState( TS_StateTypeDef* TsState )
{
	static int32_t _x = 0, _y = 0;

	int tx, ty, xDiff, yDiff;	

	if ( TsState != NULL && TsDrv != NULL ) 
	{
		TsState->TouchDetected = TsDrv->DetectTouch( TsAddr );
		
#if 0
//		TEST_HARNESS	
		
		static int toggle = 0;
				
		if ( (toggle++ & 3) == 3 )
		{
			if( !TsState->TouchDetected )
			{
				tx = rand() % 800;
				ty = rand() % 480;
				TsState->TouchDetected = true;
			}
			else
			{
				TsDrv->GetXY( TsAddr, &xraw, &yraw );
				tx = XPIX( (int) xraw );
				ty = YPIX( (int) yraw ); 
			}			
		}
		else
		{
			TsState->TouchDetected = false;
		}
		{
#else			
		if( TsState->TouchDetected )
		{
			gone_to_fire = false;
			last_tap = now( );
			offset_shift = 0;
				
			if ( backlight_off ) 
			{
				// swallow touch
				do
				{	
					TsState->TouchDetected = TsDrv->DetectTouch( TsAddr );
				} while ( TsState->TouchDetected );
				
				Command cmd;
				Command0( CMD_SET_BACKLIGHT_ON, &cmd );
				
				backlight_off = false;
						
				return;
			}
			
			gpios->touchgot = true;
			
			
			TsDrv->GetXY( TsAddr, &xraw, &yraw );
		
			if ( TsDrv == &stmpe811_ts_drv )
			{
				tx = XPIX( (int) xraw );
				ty = YPIX( (int) yraw ); 
			}
			else
			{
				tx = xraw;
				ty = yraw;
			}
#endif			
			tx = XTRIM(tx);  
			ty = YTRIM(ty);  

			xDiff = tx > _x ? (tx - _x): (_x - tx);
			yDiff = ty > _y ? (ty - _y): (_y - ty); 

			if ( xDiff + yDiff > 5 )
			{
				_x = tx;
				_y = ty; 
			}

			// Update the X position
			TsState->X = 799 - _x;

			if ( TsDrv == &ft5316_ts_drv )
			{
				// Update the Y position  
				TsState->Y = 479 - _y; 
			}
			else
			{
				TsState->Y = _y; 
			}
			
#ifndef BD_TEST_ONLY
	//		app.DebOut( "x:%d, y:%d\n", TsState->X, TsState->Y );
#endif
		}		
	}
	else
	{
		TsState->TouchDetected = false;
	}
}


/*************************************************************************/
/**  \fn      int TouchScreen::GetPosition( XPoint* aPos )
***  \brief   Class function
**************************************************************************/

int TouchScreen::GetPosition( XPoint* aPos )
{
	static TS_StateTypeDef touchPadState;
	static XPoint          lastPos;

	static unsigned int    lastTime = 0;

	/* access touch driver to receive current touch status and position */
	GetState( &touchPadState );

	if ( touchPadState.TouchDetected )
	{
      lastPos.X = aPos->X = touchPadState.X;
      lastPos.Y = aPos->Y = touchPadState.Y;

      lastTime = HAL_GetTick();

      /* return valid touch event */
      return 1;
	}

	/* in order to avoid that short undetected touch events will be interpreted
	  as an 'up' event - we return for some milliseconds the last detected event */
	if ( lastTime + 20 > HAL_GetTick() )
	{
		 aPos->X = lastPos.X;
		 aPos->Y = lastPos.Y;
		 return 1;
	}

	/* return no touch event */
	return 0;
}


/*************************************************************************/
/**  \fn      int TouchScreen::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int TouchScreen::Receive( Event event )
{
	
	return 0;
}


/*************************************************************************/
/**  \fn      int TouchScreen::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int TouchScreen::Receive( Message* )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int TouchScreen::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int TouchScreen::Receive( Command* cmd )
{
	if ( cmd->type == CMD_GET_LAST_RAW_TOUCH )
	{
		cmd->ushort0 = xraw;
		cmd->ushort1 = yraw;
		
		app.DebOut( "%d, %d\n", xraw, yraw );
		
		return CMD_OK;
	}
	if ( cmd->type == CMD_SET_CALIB_VALUES )
	{
		Calibrate( cmd->ushort0, cmd->ushort1, cmd->ushort2, cmd->ushort3, cmd->ushort4, cmd->ushort5, cmd->ushort6, cmd->ushort7 );
		
		return CMD_OK;
	}
	return CMD_ERR_UNKNOWN_CMD;
}

 

void TouchScreen::Poll( )
{
	time_t justnow = now();
	static int jumped_to_screen = false;
	
		
	if ( justnow > last_tap + 30 * 60 ) // 30 mins
	{
		if ( Utilities::AccessLevel > 1 )
		{
			Command cmd;
			Command0( CMD_GET_NUM_ON_TEST, &cmd );
			
			if ( cmd.int0 == 0 && cmd.int1 == 0 )
			{			
			 	Log::Msg( LOG_ACC, "Auto log off." );
			 	AppDeviceClass_TriggerAccessModeChangeEvent( app.DeviceObject, 0 ); // log off
				Utilities::AccessLevel = 1;
				
				// jump now w're in level 1
				Send( EW_GOTO_PRIORITY_SCREEN_MSG, ew );
			}
		}
	}
	
	if ( justnow > ( last_tap + offset_shift ) + 29 ) // 30 secs since last tap
	{
		// over 30 secs
		// Goto fire screen
		Send( EW_GOTO_PRIORITY_SCREEN_MSG, ew );
	
		offset_shift = justnow - last_tap;
	}	 
		
#ifdef BLANK_SCREEN		
		
	
	// if no mains power (on battery) and backlight isnt off
	if((psu_mains_failure) &&
	
	   // if PSU information is up-to-date ( within last 5 mins )
	   ((justnow - psu_last_received) < PSU_COMM_LAST_RECEIVED_TIMEOUT_IN_SECONDS) &&
		
	   // if no faults for an hour
	   ((Fault::last_fault_activity)  < (justnow - SCREEN_SLEEP_TIME_IN_SECONDS)) &&

	   // if no events for an hour
	   ((IOStatus::last_event_activity) < (justnow - SCREEN_SLEEP_TIME_IN_SECONDS)) &&
		
	   // if no fires for an hour
	   ((IOStatus::last_fire_activity) < (justnow - SCREEN_SLEEP_TIME_IN_SECONDS)) &&

	   // if nothing on test for an hour
	   ((IOStatus::last_ontest_activity) < (justnow - SCREEN_SLEEP_TIME_IN_SECONDS)) &&
		
	   // if no disablements for an hour
	   ((IOStatus::last_disablement_activity) < (justnow - SCREEN_SLEEP_TIME_IN_SECONDS)) &&

	   // if no tap for an hour
	   ((last_tap + SCREEN_SLEEP_TIME_IN_SECONDS) < (justnow))
	  )
	{
		// Turn backlight off
		if ( !backlight_off )
		{
			Command cmd;
			Command0( CMD_SET_BACKLIGHT_OFF, &cmd );
			
			backlight_off = true;
		}
	}			
	else
	{
		if ( backlight_off )
		{
			Command cmd;
			Command0( CMD_SET_BACKLIGHT_ON, &cmd );
			
			backlight_off = false;
		}
	}
#endif		
}

