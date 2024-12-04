/***************************************************************************
* File name: MM_USBFDU.cpp
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
* USB Serial interface
*
**************************************************************************/


/* Defines
**************************************************************************/
#define APP_RX_DATA_SIZE  2048
#define APP_TX_DATA_SIZE  2048
#define OFFSET(s,m) ((int)&((s*)0)->m)

/* System Include Files
**************************************************************************/

#include "usbd_dfu_if.h"


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_USBFDU.h"
#include "MM_Module.h"
#include "MM_Pipe.h"
#include "CO_DataStorage.h"
#include "MM_Settings.h"
#include "MM_fault.h"
#include "MM_issue.h"


static uint8_t UserRxBufferA[ APP_RX_DATA_SIZE ];
static uint8_t UserRxBufferB[ APP_RX_DATA_SIZE ];
static uint8_t* UserRxBuffer = UserRxBufferA;


static uint8_t UserTxBufferFS[ APP_TX_DATA_SIZE ];

extern USBD_HandleTypeDef hUsbDeviceFS;


Settings* 	USBFDU::settings;
Pipe* 		USBFDU::pipe;
int   		USBFDU::transmitting;
int   		USBFDU::receiving;



USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
	USBFDU::CDC_Init_FS,
	USBFDU::CDC_DeInit_FS,
	USBFDU::CDC_Control_FS,
	USBFDU::CDC_Receive_FS
};

/**
  * @brief  DInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
/*************************************************************************/
/**  \fn      int8_t USBFDU::CDC_Init_FS( void )
***  \brief   Class function
**************************************************************************/

int8_t USBFDU::CDC_Init_FS( void )
{
	USBD_CDC_SetTxBuffer( &hUsbDeviceFS, UserTxBufferFS, 0 );
	USBD_CDC_SetRxBuffer( &hUsbDeviceFS, UserRxBuffer );

	return USBD_OK;
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
/*************************************************************************/
/**  \fn      int8_t USBFDU::CDC_DeInit_FS( void )
***  \brief   Class function
**************************************************************************/

int8_t USBFDU::CDC_DeInit_FS( void )
{
   return USBD_OK;
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
/*************************************************************************/
/**  \fn      int8_t  USBFDU::CDC_Control_FS( uint8_t cmd, uint8_t* pbuf, uint16_t length )
***  \brief   Class function
**************************************************************************/

int8_t  USBFDU::CDC_Control_FS( uint8_t cmd, uint8_t* pbuf, uint16_t length )
{
   return USBD_OK;
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will block any OUT packet reception on USB endpoint
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result
  *         in receiving more data while previous ones are still not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
/*************************************************************************/
/**  \fn      int8_t USBFDU::CDC_Receive_FS( uint8_t* Buf, uint32_t *Len )
***  \brief   Class function
**************************************************************************/


int8_t USBFDU::CDC_Receive_FS( uint8_t* data, uint32_t* len )
{
	if ( USBFDU::transmitting )
	{
		//::app->DebOut( "USB rx %d bytes.\n", *len );
		
		USBFDU::pipe->totalsize = 1189468;	
		 
		USBFDU::pipe->Send( (char*) data, *len );		
		
		
#if 0		
		// Get file length from header
		if ( USBFDU::pipe->totalsize == SETTINGS_FILE_ALLOCATION_SIZE &&
			USBFDU::pipe->sent >= OFFSET( FILEHEADER, fileLength ) + sizeof( int ) )
		{
			// Get base
 			SITEFILE* f = (SITEFILE*) ( (char*) settings->rom_address + settings->write_pos );
			
			// Calc percentage complete
		 	USBFDU::pipe->totalsize = f->header.fileLength;
		}
	
		if ( *len & 3 )
		{
			assert( 0 );
		}
		else
		{
			HAL_CRC_Accumulate( ::app->crc, (uint32_t*) data, *len / 4 ); 
		}
 
		
		
#endif
		
	}

	if ( data == UserRxBufferA )
	{
		USBD_CDC_SetRxBuffer( &hUsbDeviceFS, UserRxBufferB );
	}
	else
	{
		USBD_CDC_SetRxBuffer( &hUsbDeviceFS, UserRxBufferA );
	}
	
   USBD_CDC_ReceivePacket(&hUsbDeviceFS);

	
   return (USBD_OK);
   /* USER CODE END 6 */
}



/*************************************************************************/
/**  \fn      int USBFDU::EndTransmission( Pipe* p, int sucess )
***  \brief   Class function
**************************************************************************/

void USBFDU::EndTransmission( Pipe* p, int sucess )
{
	assert( p != NULL );
	
	USBFDU::transmitting = false;
}



/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
/*************************************************************************/
/**  \fn      uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
***  \brief   Global helper function
**************************************************************************/

uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  return result;
}


/*************************************************************************/
/**  \fn      USBFDU::USBFDU( Application* app )
***  \brief   Constructor for class
**************************************************************************/

USBFDU::USBFDU( Application* app ) : Module( "USBFDU", 0, app )
{
	settings = (Settings*) app->FindModule( "Settings" );
	
	assert( settings != NULL );
		
	USBD_CDC_RegisterInterface( &hUsbDeviceFS, &USBD_Interface_fops_FS );

   USBD_Start( &hUsbDeviceFS );
	
	receiving = 0;
	transmitting = 0;
}	

 
/*************************************************************************/
/**  \fn      int USBFDU::StartTransmission( Pipe* p )
***  \brief   Class function
**************************************************************************/

int USBFDU::StartTransmission( Pipe* p )
{
	assert( p != NULL );
	
	transmitting = true;
	pipe = p;
	
	/* Reset CRC Calculation Unit */
  __HAL_CRC_DR_RESET( app->crc ); 
	
	return true;
}


/*************************************************************************/
/**  \fn      int USBFDU::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int USBFDU::Receive( Event event )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int USBFDU::Receive( Command* cmd  )
***  \brief   Class function
**************************************************************************/

int USBFDU::Receive( Command* cmd )
{
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int USBFDU::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int USBFDU::Receive( Message* )
{
	return 0;
}

