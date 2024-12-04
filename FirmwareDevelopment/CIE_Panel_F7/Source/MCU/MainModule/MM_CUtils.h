/***************************************************************************
* File name: MM_CUtils.h
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
* C Utilties
*
**************************************************************************/
#ifndef __C_UTILS_H_
#define __C_UTILS_H_

/* System Include Files
**************************************************************************/
#ifdef STM32F767xx
#include "stm32f7xx_hal_rtc.h"
#else
#include "stm32f4xx_hal_rtc.h"
#endif


#include <time.h>

/* User Include Files
**************************************************************************/
#include "CO_RBU.h"
#include "CO_Site.h"
#include "MM_Events.h"
 



#define ALERT_CAE 		128
#define ALERT_NETCAE		64
#define ALERT_DEV			32
#define ALERT_NETDEV		16
 
#define ALERT_SILENCED	1
#define ALERT_NONE		0


#ifdef __cplusplus
extern "C"
{
	

#endif
	

/*************************************************************************/
/**  \fn      mattcpy( void* dst, void* src, int bytes )
***  \brief   helper function so dumb compiler doesnt optimize out the memcpy
**************************************************************************/
	
void mattcpy( void* dst, void* src, int bytes );
	
/*************************************************************************/
/**  \fn      void db( const char* fmt, ... )
***  \brief   Global helper function
**************************************************************************/
	
void db( const char* fmt, ... );
	
	
/*************************************************************************/
/**  \fn      RadioUnit* get_child_unit( RadioUnit* start, int parent )
***  \brief   Global helper function
**************************************************************************/

RadioUnit* get_child_unit( RadioUnit* start, int parent );
	

/*************************************************************************/
/**  \fn      int get_num_devs_in_zone( int zone )
***  \brief   Global helper function
**************************************************************************/

int get_num_devs_in_zone( int zone );

	
/*************************************************************************/
/**  \fn      DeviceConfig* get_dev_config( int unit )
***  \brief   Global helper function
**************************************************************************/
	
DeviceConfig* get_dev_config( int unit );
	
	
/*************************************************************************/
/**  \fn      void RTCString( char* out, RTC_TimeTypeDef sTime, RTC_DateTypeDef sDate )
***  \brief   Global helper function
**************************************************************************/

void RTCString( char* out, RTC_TimeTypeDef sTime, RTC_DateTypeDef sDate );

/*************************************************************************/
/**  \fn      time_t RTCToUnix( RTC_TimeTypeDef* sTime, RTC_DateTypeDef* sDate )
***  \brief   Global helper function
**************************************************************************/

time_t RTCToUnix( RTC_TimeTypeDef* sTime, RTC_DateTypeDef* sDate );

	
/*************************************************************************/
/**  \fn      time_t Now( )
***  \brief   Global helper function
**************************************************************************/

time_t now( void );		
	
	
/*************************************************************************/
/**  \fn      char* strnow( void );	
***  \brief   Global helper function
**************************************************************************/

char* strnow( void );		


/*************************************************************************/
/**  \fn      void event( Event e )
***  \brief   Global helper function
**************************************************************************/

void event( Event e );		
	
	
#ifdef __cplusplus
}
#endif


#endif

