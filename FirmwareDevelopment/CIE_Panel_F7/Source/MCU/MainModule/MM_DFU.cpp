/***************************************************************************
* File name: MM_DFU.cpp
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
* DFU switcher
*
**************************************************************************/

/* System Include Files
**************************************************************************/
#ifdef STM32F767xx
#include "stm32f7xx_hal_rcc.h"
#else
#include "stm32f4xx_hal_rcc.h"
#endif
#include <assert.h>
#include "stm32f7xx_hal_rtc.h"
#include "usbd_core.h"
#include "stm32f7xx_hal_rcc_ex.h"
#include <stdlib.h>


/* User Include Files
**************************************************************************/
#include "MM_DFU.h"
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_Application.h"
#include "MM_Events.h"
#include "MM_fault.h"
#include "MM_CUtils.h"
#include "DM_BatteryRam.h"


extern USBD_HandleTypeDef hUsbDeviceFS;


/* Defines
**************************************************************************/

 

/*************************************************************************/
/**  \fn      DFUMode::DFUMode( )
***  \brief   Constructor for class
**************************************************************************/

DFUMode::DFUMode( ) : Module( "DFU Mode", 0 )
{
 
}


/*************************************************************************/
/**  \fn      int DFUMode::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int DFUMode::Receive( Event event )
{
	return 0;
}

extern LTDC_LayerCfgTypeDef pLayerCfg;
extern LTDC_HandleTypeDef hltdc;

/*************************************************************************/
/**  \fn      int DFUMode::Receive( Command* cmd  )
***  \brief   Class function
**************************************************************************/

int DFUMode::Receive( Command* cmd  )
{
	if ( cmd->type == CMD_SET_FW_LENGTH )
	{
		((int*)BKPSRAM_BASE)[ BATT_RAM_DFU_IMAGE_SIZE ] = cmd->int0;
		
		return CMD_OK;
	}
		
	if ( cmd->type == CMD_DFU_MODE )
	{
		USBD_Stop( &hUsbDeviceFS );
		
		// wait for retract to be at top
		while ( ( hltdc.Instance->CPSR & 0xFFF ) > 100 );

		// copy front buff to back
		memcpy( pLayerCfg.FBStartAdress == 0xC0000000 ? (void*)(0xC0000000 + ( 800 * 480 * 2 )) : (void*)0xC0000000, (void*)pLayerCfg.FBStartAdress, 800 * 480 * 2 );
		
		FLASH_OBProgramInitTypeDef OBInit;
		
		/* Allow Access to Flash control registers and user Flash */
		HAL_FLASH_Unlock( );

		/* Allow Access to option bytes sector */ 
		HAL_FLASH_OB_Unlock( );

		/* set both to dfu mode prog addr */ 
		OBInit.OptionType = OPTIONBYTE_BOOTADDR_0 | OPTIONBYTE_BOOTADDR_1;
		OBInit.BootAddr0  = __HAL_FLASH_CALC_BOOT_BASE_ADR( 0x081C0000 );
		OBInit.BootAddr1  = __HAL_FLASH_CALC_BOOT_BASE_ADR( 0x081C0000 );

		if ( HAL_FLASHEx_OBProgram(&OBInit) != HAL_OK )
		{
			/*
			Error occurred while setting option bytes configuration.
			User can add here some code to deal with this error.
			To know the code error, user can call function 'HAL_FLASH_GetError()'
			*/

			app.Report( ISSUE_FLASH_OPTION_BYTES_FAIL );

			return CMD_ERR_DEVICE_BUSY;
		}

		/* Start the Option Bytes programming process */  
		if ( HAL_FLASH_OB_Launch( ) != HAL_OK )
		{
			/*
			Error occurred while reloading option bytes configuration.
			User can add here some code to deal with this error.
			To know the code error, user can call function 'HAL_FLASH_GetError()'
			*/

			app.Report( ISSUE_FLASH_OPTION_BYTES_FAIL );

			return CMD_ERR_DEVICE_BUSY;
		}

		/* Prevent Access to option bytes sector */
		HAL_FLASH_OB_Lock( );

		/* Disable the Flash option control register access (recommended to protect 
		the option Bytes against possible unwanted operations) */
		HAL_FLASH_Lock( );

		/* Initiate a system reset request to reset the MCU */
		HAL_NVIC_SystemReset( );
		
		return CMD_OK;  // ( never actually reached though! )
	}
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int DFUMode::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int DFUMode::Receive( Message* )
{
	return 0;
}

 
