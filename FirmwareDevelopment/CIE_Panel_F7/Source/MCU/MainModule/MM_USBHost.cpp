/***************************************************************************
* File name: MM_USBHost.cpp
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
* USB Host Module
*
**************************************************************************/


/* System Include Files
**************************************************************************/
#ifdef STM32F767xx
#include "stm32f7xx_hal_rcc.h"
#else
#include "stm32f4xx_hal_rcc.h"
#endif
 
#include "usbh_ftdi.h"
#include "usbh_hid.h"
#include "usbh_core.h"
#include "usbh_hub.h"


/* User Include Files
**************************************************************************/
#include "MM_USBHost.h"
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_Application.h"
#include "MM_Events.h"
#include "MM_fault.h"

/* Defines
**************************************************************************/

#define MSG_OVERCURRENT 1

extern USBH_HandleTypeDef hUSBHost[ MAX_HUB_PORTS + 1];
USBHost* USBHost::me = NULL;

/*************************************************************************/
/**  \fn      static void USBH_UserProcess( USBH_HandleTypeDef *phost, uint8_t id )
***  \brief   Local helper function
**************************************************************************/

 

/*************************************************************************/
/**  \fn      USBHost::USBHost( )
***  \brief   Constructor for class
**************************************************************************/

USBHost::USBHost( ) : Module( "USBHost", 1 )
{
	 me = this;
}


/*************************************************************************/
/**  \fn      void USBHost::Poll( )
***  \brief   Class function
**************************************************************************/

void USBHost::Poll( )
{
	/* USB Host Background task */
   static uint8_t current_loop = 0xFF;
	static USBH_HandleTypeDef *_phost = 0;

	if(_phost != NULL && _phost->valid == 1)
	{
		USBH_Process(_phost);

		if(_phost->busy)
			return;
	}

	while(1)
	{
		current_loop++;

		if(current_loop > MAX_HUB_PORTS)
			current_loop = 0;

		if(hUSBHost[current_loop].valid)
		{
			_phost = &hUSBHost[current_loop];
			USBH_LL_SetupEP0(_phost);

			if(_phost->valid == 3)
			{
//LOG("PROCESSING ATTACH %d", _phost->address);
				_phost->valid = 1;
				_phost->busy  = 1;
			}

			break;
		}
	}
}


extern "C" void GPIO_0_Handler( void )
{
	if ( HAL_GPIO_ReadPin( GPIOB, USB_OVER_CURRENT_Pin ) == GPIO_PIN_RESET )
	{
		Fault::AddFault( FAULT_USB_OVERCURRENT );
		USBHost::me->Send( MSG_OVERCURRENT, USBHost::me, 1 );
	}
	else
	{
		Fault::RemoveFault( FAULT_USB_OVERCURRENT );
		USBHost::me->Send( MSG_OVERCURRENT, USBHost::me, 0 );
	}
}

	
/*************************************************************************/
/**  \fn      int USBHost::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int USBHost::Receive( Event event )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int USBHost::Receive( Command* cmd  )
***  \brief   Class function
**************************************************************************/

int USBHost::Receive( Command* cmd )
{
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int USBHost::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int USBHost::Receive( Message* m )
{
	if ( m->type == MSG_OVERCURRENT )
	{
		if ( m->value )
		{
			Fault::AddFault( FAULT_USB_OVERCURRENT );
		}
		else
		{
			Fault::RemoveFault( FAULT_USB_OVERCURRENT );
		}
	}
	return 0;
}



extern "C" void USBH_UserProcess( USBH_HandleTypeDef *phost, uint8_t id )
{
	switch( id )
	{
		case HOST_USER_SELECT_CONFIGURATION:
			break;

		case HOST_USER_DISCONNECTION:
			 
			if ( phost->pActiveClass == USBH_HID_CLASS )
			{
				app.touch_addr = NULL;
			   app.DebOut("Mouse/Keyboard disconnected!\n" );
			}
			else if ( phost->pActiveClass == USBH_FTDI_CLASS )
			{
				app.DebOut( "FTDI cable disconnected." );
				app.Send( EVENT_FTDI_REMOVED );
			}
	 
		break;

		case HOST_USER_CLASS_ACTIVE:
		 
			if ( phost->pActiveClass == USBH_HID_CLASS )
			{
				uint8_t idx = phost->device.current_interface;
				HID_HandleTypeDef* HID_Handle = (HID_HandleTypeDef*) phost->USBH_ClassTypeDef_pData[ idx ];
				
		  	   app.touch_addr = (Touch*) HID_Handle->pData;
				
			   app.DebOut("Mouse ready.\n" );
			  	
			
			  app.DebOut("Mouse/Keyboard connected on port %d\n", phost->device.current_interface );
			}
			else if ( phost->pActiveClass == USBH_FTDI_CLASS )
			{
				app.DebOut( "FTDI cable connected on port %d\n", phost->device.current_interface );
				
				app.Send( EVENT_FTDI_ACTIVE );
			}
			break;

		 
		default:
			break;
  }
}
