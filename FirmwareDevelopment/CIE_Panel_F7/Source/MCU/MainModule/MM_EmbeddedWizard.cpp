/*******************************************************************************
*
* E M B E D D E D   W I Z A R D   P R O J E C T
*
*                                                Copyright (c) TARA Systems GmbH
*                                    written by Paul Banach and Manfred Schweyer
*
********************************************************************************
*
* This software is delivered "as is" and shows the usage of other software
* components. It is provided as an example software which is intended to be
* modified and extended according to particular requirements.
*
* TARA Systems hereby disclaims all warranties and conditions with regard to the
* software, including all implied warranties and conditions of merchantability
* and non-infringement of any third party IPR or other rights which may result
* from the use or the inability to use the software.
*
********************************************************************************
*
* DESCRIPTION:
*   This file implements a generic framework for running Embedded Wizard
*   generated applications on a STM32 target with or without FreeRTOS.
*   The main() function initializes all needed peripheral components and
*   starts the Embedded Wizard applications main loop. This main loop
*   initializes the Embedded Wizard Runtime Environment / Graphics Engine,
*   creates an instance of the application class and drives the message
*   translation and screen updates.
*
*   In order to keep this framework independent from the particular GUI
*   application, the application class and the screen size are taken from the
*   generated code. In this manner, it is not necessary to modify this file
*   when creating new GUI applications. Just set the attributes 'ScreenSize'
*   and 'ApplicationClass' of the profile in the Embedded Wizard IDE.
*
*   This program demonstrates how to integrate an application developed using
*   Chora and Mosaic class library on a STM32 target.
*
*******************************************************************************/

extern "C"
{
#ifdef STM32F767xx
#include "stm32f7xx_hal.h"

#else
#include "stm32469i_eval.h"
#include "stm32469i_eval_io.h"
#include "stm32469i_eval_sdram.h"

#endif
	


//#include "usbd_cdc_if.h"
#include <string.h>
#include "ltdc.h"
#include <stdio.h>

#include "iwdg.h"

#if EW_USE_FREE_RTOS == 1
  #include "cmsis_os.h"
#endif

#include "xprintf.h"
#include "tlsf.h"

#include "ewrte.h"
#include "ewgfx.h"
#include "ewextgfx.h"
#include "ewgfxdefs.h"
#include "Core.h"
#include "Graphics.h"

#include "ew_bsp_system.h"
#include "ew_bsp_clock.h"
#include "ew_bsp_event.h"
#include "ew_bsp_display.h"
#include "ew_bsp_touch.h"
#include "ew_bsp_serial.h"

}
#include "App.h"

#include "MM_EmbeddedWizard.h"
#include "MM_Application.h"
#include "MM_NCU.h"
#include "CO_Site.h"
#include "MM_TouchScreen.h"
#include "MM_QuadSPI.h"
#include "MM_CUtils.h"

//#include "usbh_hid_mouse.h"


tlsf_t MemPool;


#if EW_USE_FREE_RTOS == 1
  #define semtstSTACK_SIZE    configMINIMAL_STACK_SIZE * 10
#endif


#undef USE_TERMINAL_INPUT



/*******************************************************************************
* FUNCTION:
*   Update
*
* DESCRIPTION:
*   The function Update performs the screen update of the dirty area.
*
* ARGUMENTS:
*   aViewPort    - Viewport used for the screen update.
*   aApplication - Root object used for the screen update.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
/*************************************************************************/
/**  \fn      void EmbeddedWizard::Update( XViewport* aViewport, CoreRoot aApplication )
***  \brief   Class function
**************************************************************************/

void EmbeddedWizard::Update( XViewport* aViewport, CoreRoot aApplication )
{
  XBitmap*       bitmap;
  GraphicsCanvas canvas     = EwNewObject( GraphicsCanvas, 0 );
  XRect          updateRect = {{ 0, 0 }, { 0, 0 }};

#if EW_USE_DOUBLE_BUFFER

  bitmap = EwBeginUpdate( aViewport );

  /* let's redraw the dirty area of the screen. Cover the returned bitmap
     objects within a canvas, so Mosaic can draw to it. */
  if ( bitmap && canvas )
  {
    GraphicsCanvas__AttachBitmap( canvas, (XUInt32)bitmap );
    updateRect = CoreRoot__UpdateGE20( aApplication, canvas );
    GraphicsCanvas__DetachBitmap( canvas );
  }

  /* complete the update */
  if ( bitmap )
    EwEndUpdate( aViewport, updateRect );

#else

  int field = 0;

  /* start screen update */
  int regions = CoreRoot__BeginUpdate( aApplication );

  /* iterate through all fields (horizontal stripes) of the display */
  while ( regions && ( field < NUMBER_OF_FIELDS ))
  {
    /* determine rectangular area of current field */
    #if EW_SURFACE_ROTATION == 0
      updateRect = EwNewRect( 0, field * FRAME_BUFFER_HEIGHT / NUMBER_OF_FIELDS,
        FRAME_BUFFER_WIDTH, ( field + 1 ) * FRAME_BUFFER_HEIGHT / NUMBER_OF_FIELDS );
    #endif

    #if EW_SURFACE_ROTATION == 90
      updateRect = EwNewRect( field * FRAME_BUFFER_HEIGHT / NUMBER_OF_FIELDS, 0,
        ( field + 1 ) * FRAME_BUFFER_HEIGHT / NUMBER_OF_FIELDS, FRAME_BUFFER_WIDTH );
    #endif

    #if EW_SURFACE_ROTATION == 180
      updateRect = EwNewRect( 0, FRAME_BUFFER_HEIGHT - ( field + 1 ) * FRAME_BUFFER_HEIGHT / NUMBER_OF_FIELDS,
        FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT - field * FRAME_BUFFER_HEIGHT / NUMBER_OF_FIELDS );
    #endif

    #if EW_SURFACE_ROTATION == 270
      updateRect = EwNewRect( FRAME_BUFFER_HEIGHT - ( field + 1 ) * FRAME_BUFFER_HEIGHT / NUMBER_OF_FIELDS,
        0, FRAME_BUFFER_HEIGHT - field * FRAME_BUFFER_HEIGHT / NUMBER_OF_FIELDS, FRAME_BUFFER_WIDTH );
    #endif

    /* next field */
    field++;

    /* sync on start line of next field to ensure save drawing operation */
    EwBspSyncOnDisplayLine(( field % NUMBER_OF_FIELDS ) * FRAME_BUFFER_HEIGHT / NUMBER_OF_FIELDS );

    /* draw area into current field */
    bitmap = EwBeginUpdateArea( aViewport, updateRect );
    GraphicsCanvas__AttachBitmap( canvas, (XUInt32)bitmap );
    CoreRoot__UpdateCanvas( aApplication, canvas, updateRect.Point1 );
    GraphicsCanvas__DetachBitmap( canvas );
    EwEndUpdate( aViewport, updateRect );
  }
  CoreRoot__EndUpdate( aApplication );
	
#endif
}


/*******************************************************************************
* FUNCTION:
*   main
*
* DESCRIPTION:
*   The main function of the whole application. The main function initializes all
*   necessary drivers and provides a for-ever loop to drive the EmWi application.
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   Zero if successful.
*
*******************************************************************************/

extern TIM_HandleTypeDef htim1;
/* User defined auto object: 'Application::Device' */
/*************************************************************************/
/**  \fn      EW_DECLARE_AUTOOBJECT( AppDevice, AppDeviceClass )
***  \brief   Global helper function
**************************************************************************/


#ifdef BOOT_MENU
EW_DECLARE_AUTOOBJECT( AppDevice, BootMenuDeviceClass );
#else
EW_DECLARE_AUTOOBJECT( AppDevices, AppDeviceClass );
#endif


/*************************************************************************/
/**  \fn      EmbeddedWizard::EmbeddedWizard( )
***  \brief   Constructor for class
**************************************************************************/

EmbeddedWizard::EmbeddedWizard( ) : Module( "EmbeddedWizard", 1, EVENT_UPDATE | EVENT_TIME_CHANGED | EVENT_DAY_TIME | EVENT_NIGHT_TIME )
{	

}


XViewport* EmbeddedWizard::viewport;

extern int EwMaxIssueTasks;
	
int EmbeddedWizard::Init( )
{
	EwMaxIssueTasks = 512;
	
	xdev_out( EwBspPutCharacter );
	EwBspConfigSystemTick( );
	
	touch_screen = (TouchScreen*) app.FindModule( "TouchScreen" );

	/* initialize display */
	EwPrint( "Initialize Display...  " );
	EwBspConfigDisplay( FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, FRAME_BUFFER_ADDR );
	EwPrint( "[OK]\n" );

	EwPrint( "Total mem: %5d\n", SDRAM_DEVICE_SIZE );
	EwPrint( "Disp mem: %5d\n", FRAME_BUFFER_SIZE + DOUBLE_BUFFER_SIZE );
	EwPrint( "Rest mem: %5d\n", SDRAM_DEVICE_SIZE - (FRAME_BUFFER_SIZE + DOUBLE_BUFFER_SIZE) );

	/* initialize tlsf memory manager */
	/* please note, that the first part of SDRAM is reserved for framebuffer */
	EwPrint( "Initialize Memory Manager...                 " );
	MemPool = tlsf_create_with_pool( MEMORY_POOL_ADDR, MEMORY_POOL_SIZE );
	EwPrint( "[OK]\n" );
	EwPrint( "MemoryPool at address 0x%08X size 0x%08X\n", MEMORY_POOL_ADDR, MEMORY_POOL_SIZE );

	/* initialize the Graphics Engine and Runtime Environment */
	EwPrint( "Initialize Graphics Engine...                " );
	if ( !EwInitGraphicsEngine( 0 ))
	 return false;
	EwPrint( "[OK]\n" );

	/* create the applications root object ... */
	EwPrint( "Create Embedded Wizard Root Object...        " );
	rootObject = (CoreRoot)EwNewObjectIndirect( EwApplicationClass, 0 );
	EwLockObject( rootObject );
	CoreRoot__Initialize( rootObject, EwScreenSize );
	EwPrint( "[OK]\n" );

	/* create Embedded Wizard viewport object to provide uniform access to the framebuffer */
	EwPrint( "Create Embedded Wizard Viewport...           " );
	viewport = EwInitViewport( EwScreenSize, EwNewRect( 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT ),
	 0, 255, FRAME_BUFFER_ADDR, DOUBLE_BUFFER_ADDR, 0, 0 );
	EwPrint( "[OK]\n" );

	touched  = 0;
	events   = 0;

#ifndef BOOT_MENU	
	app.DeviceObject = EwGetAutoObject( &AppDevices, AppDeviceClass );
#else
	app.DeviceObject = EwGetAutoObject( &BootMenuDevice, BootMenuDeviceClass );
#endif

	return true;
}

/* Embedded Wizard main loop and process all user inputs, timers and signals */

/*************************************************************************/
/**  \fn      void EmbeddedWizard::Poll( )
***  \brief   Class function
**************************************************************************/

void EmbeddedWizard::Poll( )
{
	static XPoint     touchPos;

	/* receive touch inputs and provide the application with them */

	if ( app.touch_addr != NULL )
	{
		static int count = 0;
		if ( app.touch_addr->start )
		{
			static Touch last;
			
			// Check for stuck tap
			if ( app.touch_addr->x == last.x &&
				  app.touch_addr->y == last.y &&
				  app.touch_addr->pressure == last.pressure )
			{
				if ( ++count > 4 )
				{
						
					CoreRoot__DriveCursorHitting( rootObject, 0, 0, touchPos );
					touched = 0;
					events  = 1;
					CheckGUI( );
					app.touch_addr->start = 0;
					count = 0;
					return;
				}
			}
			else
			{
				count = 0;
			}
			last = *app.touch_addr;
			
			touchPos.X = (app.touch_addr->x * 800) >> 15;
			touchPos.Y = (app.touch_addr->y * 600) >> 15;
			touchPos.Y -= 60;
			
			{
				// begin of touch cycle  
				if ( touched == 0 )
				{	
				//	app.DebOut( "touch start.\n" );
					CoreRoot__DriveCursorHitting( rootObject, 1, 0, touchPos );
				}	

				// movement during touch cycle  
				else if ( touched == 1 )
				{
				//	app.DebOut( "." );
					CoreRoot__DriveCursorMovement( rootObject, touchPos );
				}
				
				touched = 1;
				events  = 1;
			}
		}			
		else
		{
			if ( touched == 1 )
			{
//				app.DebOut( "touch end.\n" );
				CoreRoot__DriveCursorHitting( rootObject, 0, 0, touchPos );
				touched = 0;
				events  = 1;
			}
			count = 0;
		}
	}
	else if ( touch_screen->GetPosition( &touchPos ) )
	{
		/* begin of touch cycle */
		if ( touched == 0 )
		{	
//			app.DebOut( "touch start.\n" );
			CoreRoot__DriveCursorHitting( rootObject, 1, 0, touchPos );
		}	

		/* movement during touch cycle */
		else if ( touched == 1 )
		{
	//		app.DebOut( "." );
			CoreRoot__DriveCursorMovement( rootObject, touchPos );
		}
		
		touched = 1;
		events  = 1;
	}
	
	else if ( touched == 1 )
	{
//		app.DebOut( "touch end.\n" );
		CoreRoot__DriveCursorHitting( rootObject, 0, 0, touchPos );
		touched = 0;
		events  = 1;
	}
	/* end of touch cycle */

	CheckGUI( );
}


/*************************************************************************/
/**  \fn      void EmbeddedWizard::CheckGUI( )
***  \brief   Class function
**************************************************************************/

void EmbeddedWizard::CheckGUI( )
{
	static int	first_update 	= 1;	

	/* process expired timers */
	int timers = EwProcessTimers( );
	
	/* process the pending signals */
	int signals = EwProcessSignals();
		 
	/* refresh the screen, if something has changed and draw its content */
	if ( timers || signals || events )
	{
		if ( CoreRoot__DoesNeedUpdate( rootObject ))
		{
			// Check if first update
			if ( first_update ) 
			{
				first_update = 0;
				app.Send( EVENT_GUI_STARTING );
			}
			Update( viewport, rootObject );
			//SCB_CleanDCache( );
		//	SCB_InvalidateDCache_by_Addr( (uint32_t*) FRAME_BUFFER_ADDR, FRAME_BUFFER_SIZE );
		}	

		/* after each processed message start the garbage collection */
		EwReclaimMemory();

		/* print current memory statistic to serial interface - uncomment if needed */
		//  EwPrintProfilerStatistic( 0 );
	}
	/* otherwise sleep/suspend the UI application until a certain event occurs or a timer expires... */
	else
	{	
		EwBspWaitForSystemEvent( EwNextTimerExpiration( ));
	}
}


EmbeddedWizard::~EmbeddedWizard( )
{

	/* finished -> release unused resources and memory */
	EwPrint( "Shutting down application...                 " );
	EwDoneViewport( viewport );
	EwUnlockObject( rootObject );
	EwReclaimMemory();

	/* ... and deinitialize the Graphics Engine */
	EwDoneGraphicsEngine();

	EwPrint( "[OK]\n" );

	tlsf_destroy( MemPool );
}


/*************************************************************************/
/**  \fn      int EmbeddedWizard::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int EmbeddedWizard::Receive( Event e )
{	
	switch ( e )
	{
		case EVENT_UPDATE : 					Send( EW_EVENT_SITE_CHANGED_MSG );			
			break;
		case EVENT_DAY_TIME:
		case EVENT_NIGHT_TIME:	
		case EVENT_TIME_CHANGED : 			Send( EW_EVENT_TIME_CHANGED_MSG );
			
	}
	return 0;	
}

 
/*************************************************************************/
/**  \fn      int EmbeddedWizard::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int EmbeddedWizard::Receive( Message* m )
{
	switch ( m->type )
	{
		case EW_NEW_BITMAP_LOGO_MSG:
		{
			AppDeviceClass_TriggerNewBitmapLogoEvent( app.DeviceObject );
			break;
		}
		case EW_WRITE_PROTECTION_MSG:
		{
			AppDeviceClass_TriggerWriteProtectedEvent( app.DeviceObject );
			break;
		}
		case EW_ENTER_SAFE_STATE_MSG:
		{
			AppDeviceClass_TriggerSafeStateEvent( app.DeviceObject, m->value ); 
			break;
		}
		
		case EW_GOTO_PRIORITY_SCREEN_MSG:
		{
			AppDeviceClass_TriggerGotoPriorityDisplay( app.DeviceObject ); 
			break;
		}
		case EW_ON_TEST_CHANGE_MSG:
		{
			AppDeviceClass_TriggerOnTestChangeEvent( app.DeviceObject ); 
			break;
		}
		
		case EW_UPDATE_FIRE_LIST_MSG:
		{
			AppDeviceClass_TriggerFireListChangeEvent( app.DeviceObject );
			break;
		}
		
		case EW_UPDATE_EVENT_LIST_MSG:
		{
			AppDeviceClass_TriggerEventListChangeEvent( app.DeviceObject );
			break;
		}
		
		case EW_UPDATE_FAULT_LIST_MSG :
		{
			// Update display
			AppDeviceClass_TriggerFaultListChangeEvent( app.DeviceObject, m->value );
			break;
		}
		case EW_UPDATE_LOG_LIST_MSG :
		{
			// Update display
			AppDeviceClass_TriggerLogListChangeEvent( app.DeviceObject );
			break;
		}
		
		case EW_PROGRESS_START_MSG :
		{
 
			AppDeviceClass_TriggerStartProgressEvent( app.DeviceObject, 0 );

			break;
		}
		
		case EW_DFU_MODE_MSG :
		{
 
			AppDeviceClass_TriggerDFUEvent( app.DeviceObject );
 		 
			break;
		}		
		
		case EW_PROGRESS_SUCCESS_MSG :
		{
			AppDeviceClass_TriggerEndProgressEvent( app.DeviceObject, 100 );
 		
			break;
		}
		
		case EW_PROGRESS_FAILED_MSG :
		{
			AppDeviceClass_TriggerEndProgressEvent( app.DeviceObject, -1 );
			
			break;
		}
		
		case EW_EVENT_SITE_CHANGED_MSG : 
			AppDeviceClass_TriggerSiteChangedEvent( app.DeviceObject );
			break;
		
		case EW_UPDATE_MESH_STATUS_MSG : 
		{
			NCUMeshStatus* mesh = (NCUMeshStatus*) m->value;
			AppDeviceClass_TriggerMeshListChangeEvent( app.DeviceObject, mesh->total, 
																		mesh->active, mesh->meshed, mesh->extra );
			
			break;
		}
 	
		case EW_UPDATE_MESH_STAGE_MSG : 
		{
			NCUMeshStage* mesh = (NCUMeshStage*) m->value;
			AppDeviceClass_TriggerMeshStatusChangeEvent( app.DeviceObject, mesh->target, mesh->current );
			break;
		}
		
		case EW_MSG_GOTO_SYSTEM_PAGE : 
		{
			AppDeviceClass_TriggerGotoSysPageEvent( app.DeviceObject );
			break;
		}
		
		case EW_EVENT_TIME_CHANGED_MSG :
		{
			AppDeviceClass_TriggerTimeChangeEvent( app.DeviceObject, RealTimeClock::daytime );
		}
		
		case EW_UPDATE_DISABLEMENTS_MSG :
		{
			AppDeviceClass_TriggerDisablementChangeEvent( app.DeviceObject );
		}
	}
	 
	 				
	return MODULE_MSG_UNKNOWN;
}
	

/*************************************************************************/
/**  \fn      void EmbeddedWizard::StartProgress( Proggressor* )
***  \brief   Class function
**************************************************************************/

int EmbeddedWizard::StartProgress( Progressor* p )
{
	Send( EW_PROGRESS_START_MSG, this );
	
	return true;
}


/*************************************************************************/
/**  \fn      void EmbeddedWizard::StopProgress( Proggressor*, bool success )
***  \brief   Class function
**************************************************************************/

void EmbeddedWizard::StopProgress( bool success )
{
	if ( success )
	{
		Send( EW_PROGRESS_SUCCESS_MSG, this );	
		
	}
	else
	{
		Send( EW_PROGRESS_FAILED_MSG, this );	
	}
	progressfeed = NULL;
}


/*************************************************************************/
/**  \fn      int EmbeddedWizard::Receive( Command* )
***  \brief   Class function
**************************************************************************/

int EmbeddedWizard::Receive( Command* cmd )
{
	if ( cmd->type == CMD_GET_PROGRESS )
	{
		if ( progressfeed != NULL )
		{
			cmd->int0 = progressfeed->GetPercentage( );
			
			return CMD_OK;
		}
		else
		{
			return CMD_ERR_MODULE_NOT_FOUND;
		}
	}
		
	return CMD_ERR_UNKNOWN_CMD;
}



/*************************************************************************/
/**  \fn      void Progressor::Cancel( )
***  \brief   Class function
**************************************************************************/

void Progressor::Cancel( )
{
}



/*************************************************************************/
/**  \fn      static char c24( short c )
***  \brief   Helper function
**************************************************************************/

static unsigned int c24( unsigned short c )
{
	unsigned int b = c & 31;
	unsigned int g = ( c >> 5 ) & 63;
	unsigned int r = ( c >> 11 ) & 31;

	return ( r << (EW_COLOR_CHANNEL_BIT_OFFSET_RED + 3) ) + ( g << (EW_COLOR_CHANNEL_BIT_OFFSET_GREEN + 2) )
		+ ( b << (EW_COLOR_CHANNEL_BIT_OFFSET_BLUE + 3) ) | 0xFF000000;
}


/*************************************************************************/
/**  \fn      extern "C" XBitmap* EwLoadExternBitmap( XString aName )
***  \brief   Helper function
**************************************************************************/

extern "C" XBitmap* EwLoadExternBitmap( XString aName )
{
	char          name[32];

	XBitmap*      bitmap;
	XPoint        frameSize;
	XRect         lockArea;
	XBitmapLock*  lock;
	unsigned int* dest;


	/* Convert the 16-bit wide character string in 8-bit ANSI string */
	EwStringToAnsi( aName, name, sizeof( name ), 0 );

	if ( !strcmp( name, "Logo" ))
	{
		short* src = (short*) (QSPI_BASE + QSPI_BITMAP_ADDRESS);
	
		/* Create a new bitmap with the previously determined size */
		frameSize.X = *src++;
		frameSize.Y = *src++;		
			 
		// Check BMP exists
		if ( strncmp( (const char*) src, "BMP=", 4 ) )
		{
			return NULL;
		}
		
		src += 2;
		
		bitmap = EwCreateBitmap( EW_PIXEL_FORMAT_NATIVE, frameSize, 0, 1 );
	  
		/* Enough memory to create the bitmap? */
		if ( bitmap == NULL )
		 return NULL;

		/* STEP 4 */

		/* Lock the entire bitmap for write operation */
		lockArea.Point1.X = 0;
		lockArea.Point1.Y = 0;
		lockArea.Point2.X = frameSize.X;
		lockArea.Point2.Y = frameSize.Y;
		lock = EwLockBitmap( bitmap, 0, lockArea, 0, 1 );
	
		dest  = (unsigned int*)lock->Pixel1;
		
		int stride = frameSize.X + 2;

#if VGA_MODE==1
		dest += (frameSize.Y - 1) * stride;
#endif		
		
		for( int row = 0; row < frameSize.Y; row++ )
		{
			for( int pix = 0; pix < frameSize.X; pix++ )
			{
				
#if VGA_MODE==0				 
				dest[pix] = c24( *src++ );
#else
				dest[( frameSize.X - 1) - pix ] = c24( *src++ );	
#endif				
			}
#if VGA_MODE==0			
			dest += stride;
#else	
			dest -= stride;	
#endif			
			 
		 
		}	 

		/* Don't forget to unlock the bitmap when you are done! */
		EwUnlockBitmap( lock );

		/* Return the bitmap */
		return bitmap;
	}
  
	return NULL;
}
 
