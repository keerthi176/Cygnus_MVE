/***************************************************************************
* File name: MM_Events.h
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
* Event enumeration
*
**************************************************************************/
#ifndef _EVENTS_H_
#define _EVENTS_H_



/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
 


/* Defines
**************************************************************************/

typedef enum
{
	EVENT_FIRE_RESET						=  0x1,
	EVENT_FIRE_CLEAR						=  0x2,
	EVENT_RESOUND_BUTTON					=  0x4,
	
	EVENT_SILENCE_BUTTON					=  0x8,
	EVENT_EVACUATE_BUTTON				=  0x10,
	
	EVENT_MUTE_BUZZER_BUTTON			=  0x20,

	EVENT_GUI_STARTING					=  0x40,
		
   EVENT_SD_CARD_CHANGED   			=	0x80,
		
	EVENT_USBH_STORAGE_READY 			=  0x100,
		
	EVENT_USBH_STORAGE_DISCONNECT 	=  0x200,
	
	EVENT_ACKNOWLEDGE_FIRE_BUTTON		=  0x400,			

	EVENT_CALIBRATION_REQUEST			=  0x800,
	
	EVENT_CLEAR								=  0x1000,		// clear settings, flags, lits, etc
 	EVENT_RESET								=  0x2000,		// re-initialize 
	EVENT_UPDATE 							=  0x4000,		// update GUI, etc

	EVENT_CONFIRM_FIRE_BUTTON			=  0x8000,
		
	EVENT_DAY_TIME							=  0x10000,
	EVENT_NIGHT_TIME						=  0x20000,
	
	EVENT_NCU_REPLIED						=  0x40000,
	
	EVENT_TIME_CHANGED					=  0x80000,
 
	EVENT_ADC_RESULT						=  0x100000,	
	EVENT_PVD								=  0x200000,
	 
	EVENT_LOADING_CENTISECOND			=  0x400000,
	EVENT_RESET_BUTTON					=  0x800000,
	
	EVENT_FTDI_REMOVED					= 0x1000000,
	
	EVENT_CONFIRM_EVENT_BUTTON			= 0x2000000,
	EVENT_ACKNOWLEDGE_EVENT_BUTTON	= 0x4000000,
	
	EVENT_KEY_SWITCH						= 0x8000000,
	
	EVENT_STMPE811_ADC_COMPLETE		= 0x10000000,
	
	EVENT_ENTER_SAFE_MODE				= 0x20000000,
	
	EVENT_FTDI_ACTIVE						= 0x40000000,
	
} Event;

 


#endif

