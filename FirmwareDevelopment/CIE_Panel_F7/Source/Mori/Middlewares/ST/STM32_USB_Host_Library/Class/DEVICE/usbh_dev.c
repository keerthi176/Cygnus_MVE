/**
  ******************************************************************************
  * @file    usbh_cdc.c
  * @author  MCD Application Team
  * @brief   This file is the CDC Layer Handlers for USB Host CDC class.
  *
  *  @verbatim
  *
  *          ===================================================================
  *                                CDC Class Driver Description
  *          ===================================================================
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as CDC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Union Functional collection (using 1 IN endpoint for control)
  *             - Data interface class
  *
  *  @endverbatim
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      http://www.st.com/SLA0044
  *
  ******************************************************************************
  */

  /* BSPDependencies
  - "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
  - "stm32xxxxx_{eval}{discovery}_io.c"
  - "stm32xxxxx_{eval}{discovery}{adafruit}_sd.c"
  - "stm32xxxxx_{eval}{discovery}{adafruit}_lcd.c"
  - "stm32xxxxx_{eval}{discovery}_sdram.c"
  EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbh_dev.h"
 
	
void db( const char* fmt, ... ); 

/** @addtogroup USBH_LIB
* @{
*/

/** @addtogroup USBH_CLASS
* @{
*/

/** @addtogroup USBH_DEVICE_CLASS
* @{
*/

/** @defgroup USBH_DEVICE_CORE
* @brief    This file includes CDC Layer Handlers for USB Host CDC class.
* @{
*/

/** @defgroup USBH_DEVICE_CORE_Private_TypesDefinitions
* @{
*/
/**
* @}
*/


/** @defgroup USBH_DEVICE_CORE_Private_Defines
* @{
*/
#define USBH_DEVICE_BUFFER_SIZE                 1024
/**
* @}
*/


/** @defgroup USBH_DEVICE_CORE_Private_Macros
* @{
*/
/**
* @}
*/


/** @defgroup USBH_DEVICE_CORE_Private_Variables
* @{
*/
/**
* @}
*/


/** @defgroup USBH_DEVICE_CORE_Private_FunctionPrototypes
* @{
*/


static USBH_StatusTypeDef Reset( USBH_HandleTypeDef *phost ); 
static USBH_StatusTypeDef SetBaud(USBH_HandleTypeDef *phost, int baud );
	
	

static USBH_StatusTypeDef USBH_DEVICE_InterfaceInit  (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_DEVICE_InterfaceDeInit  (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_DEVICE_Process(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_DEVICE_SOFProcess(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_DEVICE_ClassRequest (USBH_HandleTypeDef *phost);
 

static void DEVICE_ProcessTransmission(USBH_HandleTypeDef *phost);

static void DEVICE_ProcessReception(USBH_HandleTypeDef *phost);

USBH_ClassTypeDef  DEVICE_Class =
{
  "DEVICE",
  DEVICE_CLASS,
  USBH_DEVICE_InterfaceInit,
  USBH_DEVICE_InterfaceDeInit,
  USBH_DEVICE_ClassRequest,
  USBH_DEVICE_Process,
  USBH_DEVICE_SOFProcess,
  NULL,
};
/**
* @}
*/


/** @defgroup USBH_DEVICE_CORE_Private_Functions
* @{
*/

 

/**
  * @brief  USBH_DEVICE_InterfaceInit
  *         The function init the CDC class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_DEVICE_InterfaceInit (USBH_HandleTypeDef *phost)
{

  USBH_StatusTypeDef status = USBH_FAIL ;
  uint8_t interface;
  DEVICE_HandleTypeDef *DEVICE_Handle;

  interface = USBH_FindInterface(phost,
                                 DEVICE_CLASS,		// Class  
                                 RESERVED_CODE,		// SubClass
                                 NO_CLASS_SPECIFIC_PROTOCOL_CODE  );	// Protocol

  if(interface == 0xFFU) /* No Valid Interface */
  {
		db( "Cannot Find the interface for Device Class.", phost->pActiveClass->Name );
  }
  else
  {
    USBH_SelectInterface( phost, interface );
	  
	 phost->USBH_ClassTypeDef_pData[phost->interfaces] =  (DEVICE_HandleTypeDef *)USBH_malloc (sizeof(DEVICE_HandleTypeDef));
	 memset(phost->USBH_ClassTypeDef_pData[phost->interfaces], 0, sizeof(DEVICE_HandleTypeDef));
	  
	// DEVICE_HandleTypeDef *p = (DEVICE_HandleTypeDef *)phost->USBH_ClassTypeDef_pData[phost->interfaces];
	// p->state = HID_ERROR;
    phost->interfaces++;
	  
    DEVICE_Handle->CommItf.NotifPipe = NULL;

#ifdef NOTIFY_INTERFACE
    /*Collect the notification endpoint address and length*/
	 
    if(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80U)
    {
      DEVICE_Handle->CommItf.NotifEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
		                               
      DEVICE_Handle->CommItf.NotifEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
    }

    /*Allocate the length for host channel number in*/
    DEVICE_Handle->CommItf.NotifPipe = USBH_AllocPipe(phost, DEVICE_Handle->CommItf.NotifEp);

    /* Open pipe for Notification endpoint */
    USBH_OpenPipe  (phost,
                    DEVICE_Handle->CommItf.NotifPipe,
                    DEVICE_Handle->CommItf.NotifEp,
                    phost->device.address,
                    phost->device.speed,
                    USB_EP_TYPE_INTR,
                    DEVICE_Handle->CommItf.NotifEpSize);

    USBH_LL_SetToggle (phost, DEVICE_Handle->CommItf.NotifPipe, 0U);
#endif
	 
      
    if(interface == 0xFFU) /* No Valid Interface */
    {
      USBH_DbgLog ("Cannot Find the interface for Data Interface Class.", phost->pActiveClass->Name);
    }
    else
    {
      /*Collect the class specific endpoint address and length*/
      if(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80U)
      {
        DEVICE_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
        DEVICE_Handle->DataItf.InEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
      }
      else
      {
        DEVICE_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
        DEVICE_Handle->DataItf.OutEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
      }

      if(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress & 0x80U)
      {
        DEVICE_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
        DEVICE_Handle->DataItf.InEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
      }
      else
      {
        DEVICE_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
        DEVICE_Handle->DataItf.OutEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
      }

 		
      /*Allocate the length for host channel number out*/
      DEVICE_Handle->DataItf.OutPipe = USBH_AllocPipe(phost, DEVICE_Handle->DataItf.OutEp);

      /*Allocate the length for host channel number in*/
      DEVICE_Handle->DataItf.InPipe = USBH_AllocPipe(phost, DEVICE_Handle->DataItf.InEp);

      /* Open channel for OUT endpoint */
      USBH_OpenPipe  (phost,
                      DEVICE_Handle->DataItf.OutPipe,
                      DEVICE_Handle->DataItf.OutEp,
                      phost->device.address,
                      phost->device.speed,
                      USB_EP_TYPE_BULK,
                      DEVICE_Handle->DataItf.OutEpSize);
      /* Open channel for IN endpoint */
      USBH_OpenPipe  (phost,
                      DEVICE_Handle->DataItf.InPipe,
                      DEVICE_Handle->DataItf.InEp,
                      phost->device.address,
                      phost->device.speed,
                      USB_EP_TYPE_BULK,
                      DEVICE_Handle->DataItf.InEpSize);

 
      USBH_LL_SetToggle (phost, DEVICE_Handle->DataItf.OutPipe, 0U);
      USBH_LL_SetToggle (phost, DEVICE_Handle->DataItf.InPipe, 0U);
 	
      		
      status = USBH_OK;
    }
  }
  return status;
}



/**
  * @brief  USBH_DEVICE_InterfaceDeInit
  *         The function DeInit the Pipes used for the CDC class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_DEVICE_InterfaceDeInit (USBH_HandleTypeDef *phost)
{
	uint8_t idx = 0;
	for(; idx < phost->interfaces; ++idx)
	{
		DEVICE_HandleTypeDef *DEVICE_Handle = phost->USBH_ClassTypeDef_pData[idx];
  
	  if ( DEVICE_Handle->CommItf.NotifPipe)
	  {
		 USBH_ClosePipe(phost, DEVICE_Handle->CommItf.NotifPipe);
		 USBH_FreePipe  (phost, DEVICE_Handle->CommItf.NotifPipe);
		 DEVICE_Handle->CommItf.NotifPipe = 0U;     /* Reset the Channel as Free */
	  }

	  if ( DEVICE_Handle->DataItf.InPipe)
	  {
		 USBH_ClosePipe(phost, DEVICE_Handle->DataItf.InPipe);
		 USBH_FreePipe  (phost, DEVICE_Handle->DataItf.InPipe);
		 DEVICE_Handle->DataItf.InPipe = 0U;     /* Reset the Channel as Free */
	  }

	  if ( DEVICE_Handle->DataItf.OutPipe)
	  {
		 USBH_ClosePipe(phost, DEVICE_Handle->DataItf.OutPipe);
		 USBH_FreePipe  (phost, DEVICE_Handle->DataItf.OutPipe);
		 DEVICE_Handle->DataItf.OutPipe = 0U;    /* Reset the Channel as Free */
	  }

	  if(phost->USBH_ClassTypeDef_pData[idx] != NULL)
			USBH_free(phost->USBH_ClassTypeDef_pData[idx]);

		phost->USBH_ClassTypeDef_pData[idx] = NULL;
  }
  return USBH_OK;
}

static uint8_t rxdata[ 64];
/**
  * @brief  USBH_DEVICE_ClassRequest
  *         The function is responsible for handling Standard requests
  *         for CDC class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_DEVICE_ClassRequest (USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef status = USBH_OK ;
	 
	//	 phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
	phost->busy = 0;
	USBH_DEVICE_Receive( phost, rxdata, 32 );
	 
  return status;
}


/**
  * @brief  USBH_DEVICE_Process
  *         The function is for managing state machine for CDC data transfers
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_DEVICE_Process (USBH_HandleTypeDef *phost)
{
   USBH_StatusTypeDef status = USBH_BUSY;
   USBH_StatusTypeDef req_status = USBH_OK;
	
	uint8_t idx = phost->device.current_interface;
	DEVICE_HandleTypeDef *DEVICE_Handle = phost->USBH_ClassTypeDef_pData[ idx];
   
   switch ( DEVICE_Handle->state )
   {

  case DEVICE_IDLE_STATE:
    status = USBH_OK;
    break;


  case DEVICE_TRANSFER_DATA:
    DEVICE_ProcessTransmission(phost);
    DEVICE_ProcessReception(phost);
    break;

  case DEVICE_ERROR_STATE:
    req_status = USBH_ClrFeature(phost, 0x00U);

    if(req_status == USBH_OK )
    {
      /*Change the state to waiting*/
      DEVICE_Handle->state = DEVICE_IDLE_STATE ;
    }
    break;

  default:
    break;

  }

  return status;
}

/**
  * @brief  USBH_DEVICE_SOFProcess
  *         The function is for managing SOF callback
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_DEVICE_SOFProcess (USBH_HandleTypeDef *phost)
{
  return USBH_OK;
}


/**
  * @brief  USBH_DEVICE_Stop
  *         Stop current CDC Transmission
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef  USBH_DEVICE_Stop(USBH_HandleTypeDef *phost)
{
  uint8_t idx = phost->device.current_interface;
	DEVICE_HandleTypeDef *DEVICE_Handle = phost->USBH_ClassTypeDef_pData[ idx];
	
  if(phost->gState == HOST_CLASS)
  {
    DEVICE_Handle->state = DEVICE_IDLE_STATE;

  //  USBH_ClosePipe(phost, DEVICE_Handle->CommItf.NotifPipe);
    USBH_ClosePipe(phost, DEVICE_Handle->DataItf.InPipe);
    USBH_ClosePipe(phost, DEVICE_Handle->DataItf.OutPipe);
  }
  return USBH_OK;
}


static USBH_StatusTypeDef Reset( USBH_HandleTypeDef *phost )
{
   uint8_t idx = phost->device.current_interface;
	DEVICE_HandleTypeDef *DEVICE_Handle = phost->USBH_ClassTypeDef_pData[ idx];


	return USBH_OK;
}
 



/**
  * @brief  This function return last received data size
  * @param  None
  * @retval None
  */
uint16_t USBH_DEVICE_GetLastReceivedDataSize(USBH_HandleTypeDef *phost)
{
  uint32_t dataSize;
  uint8_t idx = phost->device.current_interface;
  DEVICE_HandleTypeDef *DEVICE_Handle = phost->USBH_ClassTypeDef_pData[ idx];
	

  if(phost->gState == HOST_CLASS)
  {
    dataSize = USBH_LL_GetLastXferSize(phost, DEVICE_Handle->DataItf.InPipe);
  }
  else
  {
    dataSize =  0U;
  }

  return (uint16_t)dataSize;
}

/**
  * @brief  This function prepares the state before issuing the class specific commands
  * @param  None
  * @retval None
  */
USBH_StatusTypeDef  USBH_DEVICE_Transmit(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length)
{
  USBH_StatusTypeDef Status = USBH_BUSY;
  uint8_t idx = phost->device.current_interface;
  DEVICE_HandleTypeDef *DEVICE_Handle = phost->USBH_ClassTypeDef_pData[ idx];
	
  if((DEVICE_Handle->state == DEVICE_IDLE_STATE) || (DEVICE_Handle->state == DEVICE_TRANSFER_DATA))
  {
    DEVICE_Handle->pTxData = pbuff;
    DEVICE_Handle->TxDataLength = length;
    DEVICE_Handle->state = DEVICE_TRANSFER_DATA;
    DEVICE_Handle->data_tx_state = DEVICE_SEND_DATA;
    Status = USBH_OK;

#if (USBH_USE_OS == 1U)
    phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
    (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
    (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, NULL);
#endif
#endif
  }
  return Status;
}


/**
* @brief  This function prepares the state before issuing the class specific commands
* @param  None
* @retval None
*/
USBH_StatusTypeDef  USBH_DEVICE_Receive(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length)
{
  USBH_StatusTypeDef Status = USBH_BUSY;
  uint8_t idx = phost->device.current_interface;
  DEVICE_HandleTypeDef *DEVICE_Handle = phost->USBH_ClassTypeDef_pData[ idx];
		
  if((DEVICE_Handle->state == DEVICE_IDLE_STATE) || (DEVICE_Handle->state == DEVICE_TRANSFER_DATA))
  {
    DEVICE_Handle->pRxData = pbuff;
    DEVICE_Handle->RxDataLength = length;
    DEVICE_Handle->state = DEVICE_TRANSFER_DATA;
    DEVICE_Handle->data_rx_state = DEVICE_RECEIVE_DATA;
    Status = USBH_OK;

#if (USBH_USE_OS == 1U)
    phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
    (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
    (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, NULL);
#endif
#endif
  }
  return Status;
}

/**
* @brief  The function is responsible for sending data to the device
*  @param  pdev: Selected device
* @retval None
*/
static void DEVICE_ProcessTransmission(USBH_HandleTypeDef *phost)
{
  uint8_t idx = phost->device.current_interface;
  DEVICE_HandleTypeDef *DEVICE_Handle = phost->USBH_ClassTypeDef_pData[ idx];
	
  USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;

  switch (DEVICE_Handle->data_tx_state)
  {
  case DEVICE_SEND_DATA:
    if (DEVICE_Handle->TxDataLength > DEVICE_Handle->DataItf.OutEpSize)
    {
      USBH_BulkSendData (phost,
                         DEVICE_Handle->pTxData,
                         DEVICE_Handle->DataItf.OutEpSize,
                         DEVICE_Handle->DataItf.OutPipe,
                         1U);
    }
    else
    {
      USBH_BulkSendData (phost,
                         DEVICE_Handle->pTxData,
                         (uint16_t)DEVICE_Handle->TxDataLength,
                         DEVICE_Handle->DataItf.OutPipe,
                         1U);
    }

    DEVICE_Handle->data_tx_state = DEVICE_SEND_DATA_WAIT;
    break;

  case DEVICE_SEND_DATA_WAIT:

    URB_Status = USBH_LL_GetURBState(phost, DEVICE_Handle->DataItf.OutPipe);

    /* Check the status done for transmission */
    if (URB_Status == USBH_URB_DONE)
    {
      if (DEVICE_Handle->TxDataLength > DEVICE_Handle->DataItf.OutEpSize)
      {
        DEVICE_Handle->TxDataLength -= DEVICE_Handle->DataItf.OutEpSize;
        DEVICE_Handle->pTxData += DEVICE_Handle->DataItf.OutEpSize;
      }
      else
      {
        DEVICE_Handle->TxDataLength = 0U;
      }

      if (DEVICE_Handle->TxDataLength > 0U)
      {
        DEVICE_Handle->data_tx_state = DEVICE_SEND_DATA;
      }
      else
      {
        DEVICE_Handle->data_tx_state = DEVICE_IDLE;
        USBH_DEVICE_TransmitCallback(phost);
      }

#if (USBH_USE_OS == 1U)
      phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
      (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
      (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, NULL);
#endif
#endif
    }
    else
    {
      if (URB_Status == USBH_URB_NOTREADY)
      {
        DEVICE_Handle->data_tx_state = DEVICE_SEND_DATA;

#if (USBH_USE_OS == 1U)
        phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
        (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
        (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, NULL);
#endif
#endif
      }
    }
    break;

  default:
    break;
  }
}
/**
* @brief  This function responsible for reception of data from the device
*  @param  pdev: Selected device
* @retval None
*/


static void DEVICE_ProcessReception(USBH_HandleTypeDef *phost)
{
  uint8_t idx = phost->device.current_interface;
  DEVICE_HandleTypeDef *DEVICE_Handle = phost->USBH_ClassTypeDef_pData[ idx];
	
  USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;
  uint32_t length;

  switch(DEVICE_Handle->data_rx_state)
  {

  case DEVICE_RECEIVE_DATA:

    USBH_BulkReceiveData (phost,
                          DEVICE_Handle->pRxData,
                          DEVICE_Handle->DataItf.InEpSize,
                          DEVICE_Handle->DataItf.InPipe);

    DEVICE_Handle->data_rx_state = DEVICE_RECEIVE_DATA_WAIT;

    break;

  case DEVICE_RECEIVE_DATA_WAIT:

    URB_Status = USBH_LL_GetURBState(phost, DEVICE_Handle->DataItf.InPipe);

    /*Check the status done for reception*/
    if(URB_Status == USBH_URB_DONE)
    {
      length = USBH_LL_GetLastXferSize(phost, DEVICE_Handle->DataItf.InPipe);

      if(((DEVICE_Handle->RxDataLength - length) > 0U) && (length > DEVICE_Handle->DataItf.InEpSize))
      {
        DEVICE_Handle->RxDataLength -= length ;
        DEVICE_Handle->pRxData += length;
        DEVICE_Handle->data_rx_state = DEVICE_RECEIVE_DATA;
      }
      else
      {
        DEVICE_Handle->data_rx_state = DEVICE_IDLE;
		  if ( length > 2 )
		  {
			//  USBH_DEVICE_ReceiveCallback( phost, DEVICE_Handle->pRxData + 2, length - 2);
		  }
		  USBH_DEVICE_Receive( phost, rxdata, 32 ); // ask for more
      }

#if (USBH_USE_OS == 1U)
      phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
      (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
      (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, NULL);
#endif
#endif
    }
    break;

  default:
    break;
  }
}

/**
* @brief  The function informs user that data have been received
*  @param  pdev: Selected device
* @retval None
*/
__weak void USBH_DEVICE_TransmitCallback(USBH_HandleTypeDef *phost)
{

}

  /**
* @brief  The function informs user that data have been sent
*  @param  pdev: Selected device
* @retval None
*/
//__weak void USBH_DEVICE_ReceiveCallback(USBH_HandleTypeDef *phost)
//{
//
//}

  /**
* @brief  The function informs user that Settings have been changed
*  @param  pdev: Selected device
* @retval None
*/
__weak void USBH_DEVICE_LineCodingChanged(USBH_HandleTypeDef *phost)
{

}


/**
* @}
*/

/**
* @}
*/

/**
* @}
*/


/**
* @}
*/


/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
