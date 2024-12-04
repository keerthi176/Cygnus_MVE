/***************************************************************************
* File name: MM_EmbeddedWizard.h
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
* Embedded Wizard Module
*
**************************************************************************/

#ifndef _EMBEDDED_WIZARD_
#define _EMBEDDED_WIZARD_



/* User Include Files
**************************************************************************/
#include "MM_Device.h"
 

/* Defines
**************************************************************************/
#define EW_PROGRESS_SUCCESS_MSG 				1
#define EW_PROGRESS_FAILED_MSG  				2
#define EW_UPDATE_FAULT_LIST_MSG     		3
#define EW_UPDATE_LOG_LIST_MSG 				4
#define EW_ACCESS_LEVEL_MSG	  				5	
#define EW_PROGRESS_START_MSG	  				6
#define EW_DFU_MODE_MSG							7
#define EW_UPDATE_FIRE_LIST_MSG				8
#define EW_UPDATE_EVENT_LIST_MSG				9
#define EW_EVENT_SITE_CHANGED_MSG			10 
#define EW_UPDATE_MESH_STAGE_MSG 			11 
#define EW_UPDATE_MESH_STATUS_MSG 			12 
#define EW_ON_TEST_CHANGE_MSG					13
#define EW_GOTO_PRIORITY_SCREEN_MSG			14
#define EW_EVENT_TIME_CHANGED_MSG			15
#define EW_ENTER_SAFE_STATE_MSG				16
#define EW_WRITE_PROTECTION_MSG				17
#define EW_UPDATE_DISABLEMENTS_MSG			18
#define EW_NEW_BITMAP_LOGO_MSG				19
#define EW_MSG_GOTO_SYSTEM_PAGE				20


   
#define SDRAM_DEVICE_ADDR  ((uint32_t)0xC0000000)

/* SDRAM device size in Bytes */
#define SDRAM_DEVICE_SIZE  ((uint32_t)0x800000)


/* define pyhiscal dimension of the LCD framebuffer */
#define FRAME_BUFFER_WIDTH    800
#define FRAME_BUFFER_HEIGHT   480

/* calculated addresses for framebuffer(s) and memory manager */
#define FRAME_BUFFER_SIZE     ( FRAME_BUFFER_WIDTH * FRAME_BUFFER_HEIGHT * 2 )
#define FRAME_BUFFER_ADDR     (void*)(SDRAM_DEVICE_ADDR)

#if  EW_USE_DOUBLE_BUFFER  
  #define DOUBLE_BUFFER_ADDR  (void*)((unsigned char*)FRAME_BUFFER_ADDR + FRAME_BUFFER_SIZE)
  #define DOUBLE_BUFFER_SIZE  FRAME_BUFFER_SIZE
#else
  #define DOUBLE_BUFFER_ADDR  (void*)(0)
  #define DOUBLE_BUFFER_SIZE  0
	#define NUMBER_OF_FIELDS    3
#endif

#define DEVICE_BUFFER_SIZE		( SITE_MAX_DEVICES * sizeof(Device) )  
#define DEVICE_BUFFER_ADDR		(void*)((unsigned char*)FRAME_BUFFER_ADDR + FRAME_BUFFER_SIZE + DOUBLE_BUFFER_SIZE )
  
 

#define MEMORY_POOL_ADDR      (void*)((unsigned char*)DEVICE_BUFFER_ADDR + DEVICE_BUFFER_SIZE )
#define MEMORY_POOL_SIZE      (SDRAM_DEVICE_SIZE - FRAME_BUFFER_SIZE - DOUBLE_BUFFER_SIZE - DEVICE_BUFFER_SIZE )


/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "_CoreView.h"
#include "ewgfx.h"


class TouchScreen;

class Progressor
{
	public:
		
	virtual int GetPercentage( ) = 0;
	virtual void Cancel( );
};	
 
	

class EmbeddedWizard : public Module
{
	public:
		
	EmbeddedWizard( );
	~EmbeddedWizard( );
	
	virtual int Init( );
	virtual void Poll( );
	virtual int Receive( Event );
	virtual int Receive( Message* );
	virtual int Receive( Command* );
	
	int StartProgress( Progressor* );
	void StopProgress( bool success );
		
	void CheckGUI( );
	
	void Update( XViewport* aViewport, CoreRoot aApplication );
	
	Progressor*		progressfeed;
	int 				touched;
	int				events;
	static		   XViewport* 		viewport;
	CoreRoot   		rootObject;
	
	TouchScreen* 	touch_screen;
};



#endif


