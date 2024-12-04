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
#include "usbh_ftdi.h"
 
	
void db( const char* fmt, ... ); 

/** @addtogroup USBH_LIB
* @{
*/

/** @addtogroup USBH_CLASS
* @{
*/

/** @addtogroup USBH_FTDI_CLASS
* @{
*/

/** @defgroup USBH_FTDI_CORE
* @brief    This file includes CDC Layer Handlers for USB Host CDC class.
* @{
*/

/** @defgroup USBH_FTDI_CORE_Private_TypesDefinitions
* @{
*/
/**
* @}
*/


/** @defgroup USBH_FTDI_CORE_Private_Defines
* @{
*/
#define USBH_FTDI_BUFFER_SIZE                 1024
/**
* @}
*/


/** @defgroup USBH_FTDI_CORE_Private_Macros
* @{
*/
/**
* @}
*/


/** @defgroup USBH_FTDI_CORE_Private_Variables
* @{
*/
/**
* @}
*/


/** @defgroup USBH_FTDI_CORE_Private_FunctionPrototypes
* @{
*/


static USBH_StatusTypeDef Reset( USBH_HandleTypeDef *phost ); 
static USBH_StatusTypeDef SetBaud(USBH_HandleTypeDef *phost, int baud );
	
	

static USBH_StatusTypeDef USBH_FTDI_InterfaceInit  (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_FTDI_InterfaceDeInit  (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_FTDI_Process(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_FTDI_SOFProcess(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_FTDI_ClassRequest (USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef GetLineCoding(USBH_HandleTypeDef *phost,
                                                 FTDI_LineCodingTypeDef *linecoding);

static USBH_StatusTypeDef SetLineCoding(USBH_HandleTypeDef *phost,
                                                 FTDI_LineCodingTypeDef *linecoding);

static void FTDI_ProcessTransmission(USBH_HandleTypeDef *phost);

static void FTDI_ProcessReception(USBH_HandleTypeDef *phost);

USBH_ClassTypeDef  FTDI_Class =
{
  "FTDI",
  VENDOR_SPECIFIC,
  USBH_FTDI_InterfaceInit,
  USBH_FTDI_InterfaceDeInit,
  USBH_FTDI_ClassRequest,
  USBH_FTDI_Process,
  USBH_FTDI_SOFProcess,
  NULL,
};
/**
* @}
*/


/** @defgroup USBH_FTDI_CORE_Private_Functions
* @{
*/


static int ftdi_convert_baudrate(int baudrate, FTDI_HandleTypeDef* ftdi,
										 unsigned short *value, unsigned short *index);


/**
  * @brief  USBH_FTDI_InterfaceInit
  *         The function init the CDC class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_FTDI_InterfaceInit (USBH_HandleTypeDef *phost)
{

  USBH_StatusTypeDef status = USBH_FAIL ;
  uint8_t interface;
  FTDI_HandleTypeDef *FTDI_Handle;

  interface = USBH_FindInterface(phost,
                                 VENDOR_SPECIFIC,		// Class  
                                 VENDOR_SPECIFIC,		// SubClass
                                 VENDOR_SPECIFIC  );	// Protocol

  if(interface == 0xFFU) /* No Valid Interface */
  {
		db( "Cannot Find the interface for Device Class.", phost->pActiveClass->Name );
  }
  else
  {
    USBH_SelectInterface( phost, interface );
    phost->pActiveClass->pData = (FTDI_HandleTypeDef *)USBH_malloc (sizeof(FTDI_HandleTypeDef));
    FTDI_Handle =  (FTDI_HandleTypeDef*) phost->pActiveClass->pData;
	  
	  FTDI_Handle->CommItf.NotifPipe = NULL;

#ifdef NOTIFY_INTERFACE
    /*Collect the notification endpoint address and length*/
	 
    if(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80U)
    {
      FTDI_Handle->CommItf.NotifEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
		                               
      FTDI_Handle->CommItf.NotifEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
    }

    /*Allocate the length for host channel number in*/
    FTDI_Handle->CommItf.NotifPipe = USBH_AllocPipe(phost, FTDI_Handle->CommItf.NotifEp);

    /* Open pipe for Notification endpoint */
    USBH_OpenPipe  (phost,
                    FTDI_Handle->CommItf.NotifPipe,
                    FTDI_Handle->CommItf.NotifEp,
                    phost->device.address,
                    phost->device.speed,
                    USB_EP_TYPE_INTR,
                    FTDI_Handle->CommItf.NotifEpSize);

    USBH_LL_SetToggle (phost, FTDI_Handle->CommItf.NotifPipe, 0U);
#endif
	 
     //USBH_FindInterface(phost,
                    //               DATA_INTERFACE_CLASS_CODE,
                      //             RESERVED,
                        //           NO_CLASS_SPECIFIC_PROTOCOL_CODE);

    if(interface == 0xFFU) /* No Valid Interface */
    {
      USBH_DbgLog ("Cannot Find the interface for Data Interface Class.", phost->pActiveClass->Name);
    }
    else
    {
      /*Collect the class specific endpoint address and length*/
      if(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80U)
      {
        FTDI_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
        FTDI_Handle->DataItf.InEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
      }
      else
      {
        FTDI_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
        FTDI_Handle->DataItf.OutEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
      }

      if(phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress & 0x80U)
      {
        FTDI_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
        FTDI_Handle->DataItf.InEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
      }
      else
      {
        FTDI_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
        FTDI_Handle->DataItf.OutEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
      }

      /*Allocate the length for host channel number out*/
      FTDI_Handle->DataItf.OutPipe = USBH_AllocPipe(phost, FTDI_Handle->DataItf.OutEp);

      /*Allocate the length for host channel number in*/
      FTDI_Handle->DataItf.InPipe = USBH_AllocPipe(phost, FTDI_Handle->DataItf.InEp);

      /* Open channel for OUT endpoint */
      USBH_OpenPipe  (phost,
                      FTDI_Handle->DataItf.OutPipe,
                      FTDI_Handle->DataItf.OutEp,
                      phost->device.address,
                      phost->device.speed,
                      USB_EP_TYPE_BULK,
                      FTDI_Handle->DataItf.OutEpSize);
      /* Open channel for IN endpoint */
      USBH_OpenPipe  (phost,
                      FTDI_Handle->DataItf.InPipe,
                      FTDI_Handle->DataItf.InEp,
                      phost->device.address,
                      phost->device.speed,
                      USB_EP_TYPE_BULK,
                      FTDI_Handle->DataItf.InEpSize);

      FTDI_Handle->state = FTDI_IDLE_STATE;

      USBH_LL_SetToggle (phost, FTDI_Handle->DataItf.OutPipe, 0U);
      USBH_LL_SetToggle (phost, FTDI_Handle->DataItf.InPipe, 0U);
		
		
		
		 // Try to guess chip type
		// Bug in the BM type chips: bcdDevice is 0x200 for serial == 0
		if ( phost->device.DevDesc.bcdDevice == 0x400 ||
				( phost->device.DevDesc.bcdDevice == 0x200 && phost->device.DevDesc.iSerialNumber == 0 ))			
			FTDI_Handle->type = TYPE_BM;
		else if (phost->device.DevDesc.bcdDevice == 0x200)
			 FTDI_Handle->type = TYPE_AM;
		else if (phost->device.DevDesc.bcdDevice == 0x500)
			 FTDI_Handle->type = TYPE_2232C;
		else if (phost->device.DevDesc.bcdDevice == 0x600)
			 FTDI_Handle->type = TYPE_R;
		else if (phost->device.DevDesc.bcdDevice == 0x700)
			 FTDI_Handle->type = TYPE_2232H;
		else if (phost->device.DevDesc.bcdDevice == 0x800)
			 FTDI_Handle->type = TYPE_4232H;
		else if (phost->device.DevDesc.bcdDevice == 0x900)
			 FTDI_Handle->type = TYPE_232H;
		else if (phost->device.DevDesc.bcdDevice == 0x1000)
			 FTDI_Handle->type = TYPE_230X;
					
      status = USBH_OK;
    }
  }
  return status;
}



/**
  * @brief  USBH_FTDI_InterfaceDeInit
  *         The function DeInit the Pipes used for the CDC class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_FTDI_InterfaceDeInit (USBH_HandleTypeDef *phost)
{
  FTDI_HandleTypeDef *FTDI_Handle =  (FTDI_HandleTypeDef*) phost->pActiveClass->pData;

  if ( FTDI_Handle->CommItf.NotifPipe)
  {
    USBH_ClosePipe(phost, FTDI_Handle->CommItf.NotifPipe);
    USBH_FreePipe  (phost, FTDI_Handle->CommItf.NotifPipe);
    FTDI_Handle->CommItf.NotifPipe = 0U;     /* Reset the Channel as Free */
  }

  if ( FTDI_Handle->DataItf.InPipe)
  {
    USBH_ClosePipe(phost, FTDI_Handle->DataItf.InPipe);
    USBH_FreePipe  (phost, FTDI_Handle->DataItf.InPipe);
    FTDI_Handle->DataItf.InPipe = 0U;     /* Reset the Channel as Free */
  }

  if ( FTDI_Handle->DataItf.OutPipe)
  {
    USBH_ClosePipe(phost, FTDI_Handle->DataItf.OutPipe);
    USBH_FreePipe  (phost, FTDI_Handle->DataItf.OutPipe);
    FTDI_Handle->DataItf.OutPipe = 0U;    /* Reset the Channel as Free */
  }

  if(phost->pActiveClass->pData)
  {
    USBH_free (phost->pActiveClass->pData);
    phost->pActiveClass->pData = 0U;
  }

  return USBH_OK;
}

uint8_t rxdata[ 32];
/**
  * @brief  USBH_FTDI_ClassRequest
  *         The function is responsible for handling Standard requests
  *         for CDC class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_FTDI_ClassRequest (USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef status = USBH_OK ;

  /*Issue the get line coding request*/
	static int reset =0;
	
	if ( reset == 0)
		status =  Reset(  phost );
	
	if(status == USBH_OK)
	{
		reset = 1;
	}
	if ( reset == 1 )
	{
		status = SetBaud( phost, 115200 );
	
	  if(status == USBH_OK)
	  {
		 phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
		 USBH_FTDI_Receive( phost, rxdata, 32 ); 
		  
	  }
  }
  return status;
}


/**
  * @brief  USBH_FTDI_Process
  *         The function is for managing state machine for CDC data transfers
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_FTDI_Process (USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef status = USBH_BUSY;
  USBH_StatusTypeDef req_status = USBH_OK;
  FTDI_HandleTypeDef *FTDI_Handle =  (FTDI_HandleTypeDef*) phost->pActiveClass->pData;

  switch(FTDI_Handle->state)
  {

  case FTDI_IDLE_STATE:
    status = USBH_OK;
    break;

  case FTDI_SET_LINE_CODING_STATE:
    req_status = SetLineCoding(phost, FTDI_Handle->pUserLineCoding);

    if(req_status == USBH_OK)
    {
      FTDI_Handle->state = FTDI_GET_LAST_LINE_CODING_STATE;
    }

    else
    {
      if(req_status != USBH_BUSY)
      {
        FTDI_Handle->state = FTDI_ERROR_STATE;
      }
    }
    break;


  case FTDI_GET_LAST_LINE_CODING_STATE:
    req_status = GetLineCoding(phost, &(FTDI_Handle->LineCoding));

    if(req_status == USBH_OK)
    {
      FTDI_Handle->state = FTDI_IDLE_STATE;

      if((FTDI_Handle->LineCoding.b.bCharFormat == FTDI_Handle->pUserLineCoding->b.bCharFormat) &&
         (FTDI_Handle->LineCoding.b.bDataBits == FTDI_Handle->pUserLineCoding->b.bDataBits) &&
         (FTDI_Handle->LineCoding.b.bParityType == FTDI_Handle->pUserLineCoding->b.bParityType) &&
         (FTDI_Handle->LineCoding.b.dwDTERate == FTDI_Handle->pUserLineCoding->b.dwDTERate))
      {
        USBH_FTDI_LineCodingChanged(phost);
      }
    }
    else
    {
      if(req_status != USBH_BUSY)
      {
        FTDI_Handle->state = FTDI_ERROR_STATE;
      }
    }
    break;

  case FTDI_TRANSFER_DATA:
    FTDI_ProcessTransmission(phost);
    FTDI_ProcessReception(phost);
    break;

  case FTDI_ERROR_STATE:
    req_status = USBH_ClrFeature(phost, 0x00U);

    if(req_status == USBH_OK )
    {
      /*Change the state to waiting*/
      FTDI_Handle->state = FTDI_IDLE_STATE ;
    }
    break;

  default:
    break;

  }

  return status;
}

/**
  * @brief  USBH_FTDI_SOFProcess
  *         The function is for managing SOF callback
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_FTDI_SOFProcess (USBH_HandleTypeDef *phost)
{
  return USBH_OK;
}


/**
  * @brief  USBH_FTDI_Stop
  *         Stop current CDC Transmission
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef  USBH_FTDI_Stop(USBH_HandleTypeDef *phost)
{
  FTDI_HandleTypeDef *FTDI_Handle =  (FTDI_HandleTypeDef*) phost->pActiveClass->pData;

  if(phost->gState == HOST_CLASS)
  {
    FTDI_Handle->state = FTDI_IDLE_STATE;

  //  USBH_ClosePipe(phost, FTDI_Handle->CommItf.NotifPipe);
    USBH_ClosePipe(phost, FTDI_Handle->DataItf.InPipe);
    USBH_ClosePipe(phost, FTDI_Handle->DataItf.OutPipe);
  }
  return USBH_OK;
}


static USBH_StatusTypeDef Reset( USBH_HandleTypeDef *phost )
{
    FTDI_HandleTypeDef* ftdi =  (FTDI_HandleTypeDef*) phost->pActiveClass->pData;

   // if (libusb_control_transfer(ftdi->usb_dev, FTDI_DEVICE_OUT_REQTYPE,
   //                             SIO_RESET_REQUEST, SIO_RESET_SIO,
   //                             ftdi->index, NULL, 0, ftdi->usb_write_timeout) < 0)
   //     ftdi_error_return(-1,"FTDI reset failed");

    // Invalidate data in the readbuffer
    ftdi->readbuffer_offset = 0;
    ftdi->readbuffer_remaining = 0;
	 
	phost->Control.setup.b.bmRequestType = FTDI_DEVICE_OUT_REQTYPE;
                                          

	phost->Control.setup.b.bRequest = FTDI_SIO_RESET; // SIO_SET_BAUDRATE_REQUEST
	 
  
	phost->Control.setup.b.wValue.w = 0;

	phost->Control.setup.b.wIndex.w = 1;

	phost->Control.setup.b.wLength.w = 0;

	return USBH_CtlReq(phost, NULL, 0 );
}
 

static USBH_StatusTypeDef SetBaud(USBH_HandleTypeDef *phost, int baudrate )
{
	FTDI_HandleTypeDef* ftdi =  (FTDI_HandleTypeDef*) phost->pActiveClass->pData;
	
   unsigned short value, index;
    int actual_baudrate;

     
    if (ftdi->bitbang_enabled)
    {
        baudrate = baudrate*4;
    }

    actual_baudrate = ftdi_convert_baudrate(baudrate, ftdi, &value, &index);
	 
    if (actual_baudrate <= 0)
	 {
		db( "USB - Silly baudrate <= 0.");
		 return USBH_FAIL;
	 }
	 
    // Check within tolerance (about 5%)
    if ((actual_baudrate * 2 < baudrate /* Catch overflows */ )
            || ((actual_baudrate < baudrate)
                ? (actual_baudrate * 21 < baudrate * 20)
                : (baudrate * 21 < actual_baudrate * 20)))
	 {
        db( "Unsupported baudrate. Note: bitbang baudrates are automatically multiplied by 4");
		 return USBH_FAIL;
	 }
 
    ftdi->baudrate = baudrate;      
       // uint8_t rv = pUsb->ctrlReq(bAddress, 0, bmREQ_FTDI_OUT, FTDI_SIO_SET_BAUD_RATE, baud_value & 0xff, baud_value >> 8, baud_index, 0, 0, NULL, NULL);
		  
	phost->Control.setup.b.bmRequestType = FTDI_DEVICE_OUT_REQTYPE;
                                          

	phost->Control.setup.b.bRequest = FTDI_SIO_SET_BAUD_RATE; // SIO_SET_BAUDRATE_REQUEST
  
	phost->Control.setup.b.wValue.w = value;

	phost->Control.setup.b.wIndex.w = index;

	phost->Control.setup.b.wLength.w = 0;

	return USBH_CtlReq(phost, NULL, 0 );
}
/**
  * @brief  This request allows the host to find out the currently
  *         configured line coding.
  * @param  pdev: Selected device
  * @retval USBH_StatusTypeDef : USB ctl xfer status
  */
static USBH_StatusTypeDef GetLineCoding(USBH_HandleTypeDef *phost, FTDI_LineCodingTypeDef *linecoding)
{

  phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_TYPE_CLASS | \
                              USB_REQ_RECIPIENT_INTERFACE;

  phost->Control.setup.b.bRequest = FTDI_GET_LINE_CODING;
  phost->Control.setup.b.wValue.w = 0U;
  phost->Control.setup.b.wIndex.w = 0U;
  phost->Control.setup.b.wLength.w = LINE_CODING_STRUCTURE_SIZE;

  return USBH_CtlReq(phost, linecoding->Array, LINE_CODING_STRUCTURE_SIZE);
}


/**
  * @brief  This request allows the host to specify typical asynchronous
  * line-character formatting properties
  * This request applies to asynchronous byte stream data class interfaces
  * and endpoints
  * @param  pdev: Selected device
  * @retval USBH_StatusTypeDef : USB ctl xfer status
  */
static USBH_StatusTypeDef SetLineCoding(USBH_HandleTypeDef *phost,
                                        FTDI_LineCodingTypeDef *linecoding)
{
  phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_TYPE_CLASS |
                                         USB_REQ_RECIPIENT_INTERFACE;

  phost->Control.setup.b.bRequest = FTDI_SET_LINE_CODING;
  phost->Control.setup.b.wValue.w = 0U;

  phost->Control.setup.b.wIndex.w = 0U;

  phost->Control.setup.b.wLength.w = LINE_CODING_STRUCTURE_SIZE;

  return USBH_CtlReq(phost, linecoding->Array, LINE_CODING_STRUCTURE_SIZE);
}

/**
* @brief  This function prepares the state before issuing the class specific commands
* @param  None
* @retval None
*/
USBH_StatusTypeDef USBH_FTDI_SetLineCoding(USBH_HandleTypeDef *phost,
                                          FTDI_LineCodingTypeDef *linecoding)
{
  FTDI_HandleTypeDef *FTDI_Handle =  (FTDI_HandleTypeDef*) phost->pActiveClass->pData;

  if(phost->gState == HOST_CLASS)
  {
    FTDI_Handle->state = FTDI_SET_LINE_CODING_STATE;
    FTDI_Handle->pUserLineCoding = linecoding;

#if (USBH_USE_OS == 1U)
    phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
    (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
    (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, NULL);
#endif
#endif
  }

  return USBH_OK;
}

/**
* @brief  This function prepares the state before issuing the class specific commands
* @param  None
* @retval None
*/
USBH_StatusTypeDef  USBH_FTDI_GetLineCoding(USBH_HandleTypeDef *phost,
                                           FTDI_LineCodingTypeDef *linecoding)
{
  FTDI_HandleTypeDef *FTDI_Handle = (FTDI_HandleTypeDef*) phost->pActiveClass->pData;

  if((phost->gState == HOST_CLASS) || (phost->gState == HOST_CLASS_REQUEST))
  {
    *linecoding = FTDI_Handle->LineCoding;
    return USBH_OK;
  }
  else
  {
    return USBH_FAIL;
  }
}

/**
  * @brief  This function return last received data size
  * @param  None
  * @retval None
  */
uint16_t USBH_FTDI_GetLastReceivedDataSize(USBH_HandleTypeDef *phost)
{
  uint32_t dataSize;
  FTDI_HandleTypeDef *FTDI_Handle = (FTDI_HandleTypeDef*) phost->pActiveClass->pData;

  if(phost->gState == HOST_CLASS)
  {
    dataSize = USBH_LL_GetLastXferSize(phost, FTDI_Handle->DataItf.InPipe);
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
USBH_StatusTypeDef  USBH_FTDI_Transmit(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length)
{
  USBH_StatusTypeDef Status = USBH_BUSY;
  FTDI_HandleTypeDef *FTDI_Handle =  (FTDI_HandleTypeDef*) phost->pActiveClass->pData;
	
  pbuff[ length ] = 0;
	app.DebOut( "FTDI Out: " );
  app.DebOut( pbuff );
	app.DebOut( "\n" );

  if((FTDI_Handle->state == FTDI_IDLE_STATE) || (FTDI_Handle->state == FTDI_TRANSFER_DATA))
  {
    FTDI_Handle->pTxData = pbuff;
    FTDI_Handle->TxDataLength = length;
    FTDI_Handle->state = FTDI_TRANSFER_DATA;
    FTDI_Handle->data_tx_state = FTDI_SEND_DATA;
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
USBH_StatusTypeDef  USBH_FTDI_Receive(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length)
{
  USBH_StatusTypeDef Status = USBH_BUSY;
  FTDI_HandleTypeDef *FTDI_Handle =  (FTDI_HandleTypeDef*) phost->pActiveClass->pData;
	
  if((FTDI_Handle->state == FTDI_IDLE_STATE) || (FTDI_Handle->state == FTDI_TRANSFER_DATA))
  {
    FTDI_Handle->pRxData = pbuff;
    FTDI_Handle->RxDataLength = length;
    FTDI_Handle->state = FTDI_TRANSFER_DATA;
    FTDI_Handle->data_rx_state = FTDI_RECEIVE_DATA;
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
static void FTDI_ProcessTransmission(USBH_HandleTypeDef *phost)
{
  FTDI_HandleTypeDef *FTDI_Handle = (FTDI_HandleTypeDef*) phost->pActiveClass->pData;
  USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;

  switch (FTDI_Handle->data_tx_state)
  {
  case FTDI_SEND_DATA:
    if (FTDI_Handle->TxDataLength > FTDI_Handle->DataItf.OutEpSize)
    {
      USBH_BulkSendData (phost,
                         FTDI_Handle->pTxData,
                         FTDI_Handle->DataItf.OutEpSize,
                         FTDI_Handle->DataItf.OutPipe,
                         1U);
    }
    else
    {
      USBH_BulkSendData (phost,
                         FTDI_Handle->pTxData,
                         (uint16_t)FTDI_Handle->TxDataLength,
                         FTDI_Handle->DataItf.OutPipe,
                         1U);
    }

    FTDI_Handle->data_tx_state = FTDI_SEND_DATA_WAIT;
    break;

  case FTDI_SEND_DATA_WAIT:

    URB_Status = USBH_LL_GetURBState(phost, FTDI_Handle->DataItf.OutPipe);

    /* Check the status done for transmission */
    if (URB_Status == USBH_URB_DONE)
    {
      if (FTDI_Handle->TxDataLength > FTDI_Handle->DataItf.OutEpSize)
      {
        FTDI_Handle->TxDataLength -= FTDI_Handle->DataItf.OutEpSize;
        FTDI_Handle->pTxData += FTDI_Handle->DataItf.OutEpSize;
      }
      else
      {
        FTDI_Handle->TxDataLength = 0U;
      }

      if (FTDI_Handle->TxDataLength > 0U)
      {
        FTDI_Handle->data_tx_state = FTDI_SEND_DATA;
      }
      else
      {
        FTDI_Handle->data_tx_state = FTDI_IDLE;
        USBH_FTDI_TransmitCallback(phost);
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
        FTDI_Handle->data_tx_state = FTDI_SEND_DATA;

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


static void FTDI_ProcessReception(USBH_HandleTypeDef *phost)
{
  FTDI_HandleTypeDef *FTDI_Handle =  (FTDI_HandleTypeDef*) phost->pActiveClass->pData;
  USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;
  uint32_t length;

  switch(FTDI_Handle->data_rx_state)
  {

  case FTDI_RECEIVE_DATA:

    USBH_BulkReceiveData (phost,
                          FTDI_Handle->pRxData,
                          FTDI_Handle->DataItf.InEpSize,
                          FTDI_Handle->DataItf.InPipe);

    FTDI_Handle->data_rx_state = FTDI_RECEIVE_DATA_WAIT;

    break;

  case FTDI_RECEIVE_DATA_WAIT:

    URB_Status = USBH_LL_GetURBState(phost, FTDI_Handle->DataItf.InPipe);

    /*Check the status done for reception*/
    if(URB_Status == USBH_URB_DONE)
    {
      length = USBH_LL_GetLastXferSize(phost, FTDI_Handle->DataItf.InPipe);

      if(((FTDI_Handle->RxDataLength - length) > 0U) && (length > FTDI_Handle->DataItf.InEpSize))
      {
        FTDI_Handle->RxDataLength -= length ;
        FTDI_Handle->pRxData += length;
        FTDI_Handle->data_rx_state = FTDI_RECEIVE_DATA;
      }
      else
      {
        FTDI_Handle->data_rx_state = FTDI_IDLE;
		  if ( length > 2 )
		  {
			  USBH_FTDI_ReceiveCallback( phost, FTDI_Handle->pRxData + 2, length - 2);
		  }
		  USBH_FTDI_Receive( phost, rxdata, 32 ); // ask for more
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


	/* break_state = -1 to turn on break, and 0 to turn off break */
	/* see drivers/char/tty_io.c to see it used */
	/* last_set_data_value NEVER has the break bit set in it */

static void FTDI_BreakCtrl( USBH_HandleTypeDef *phost, int break_state )
{
   FTDI_HandleTypeDef *FTDI_Handle =  (FTDI_HandleTypeDef*) phost->pActiveClass->pData;
		 
	uint16 value;
   
	if (break_state)
		value = FTDI_Handle->last_set_data_value | FTDI_SIO_SET_BREAK;
	else
		value = priv->last_set_data_value;

	if (usb_control_msg(port->serial->dev,
			usb_sndctrlpipe(port->serial->dev, 0),
			FTDI_SIO_SET_DATA_REQUEST,
			FTDI_SIO_SET_DATA_REQUEST_TYPE,
			value , priv->interface,
			NULL, 0, WDR_TIMEOUT) < 0) {
		dev_err(&port->dev, "%s FAILED to enable/disable break state (state was %d)\n",
			__func__, break_state);
	}

	dev_dbg(&port->dev, "%s break state is %d - urb is %d\n", __func__,
		break_state, value);

}

/**
* @brief  The function informs user that data have been received
*  @param  pdev: Selected device
* @retval None
*/
__weak void USBH_FTDI_TransmitCallback(USBH_HandleTypeDef *phost)
{

}

  /**
* @brief  The function informs user that data have been sent
*  @param  pdev: Selected device
* @retval None
*/
//__weak void USBH_FTDI_ReceiveCallback(USBH_HandleTypeDef *phost)
//{
//
//}

  /**
* @brief  The function informs user that Settings have been changed
*  @param  pdev: Selected device
* @retval None
*/
__weak void USBH_FTDI_LineCodingChanged(USBH_HandleTypeDef *phost)
{

}


static int ftdi_to_clkbits_AM(int baudrate, unsigned long *encoded_divisor)

{
    static const char frac_code[8] = {0, 3, 2, 4, 1, 5, 6, 7};
    static const char am_adjust_up[8] = {0, 0, 0, 1, 0, 3, 2, 1};
    static const char am_adjust_dn[8] = {0, 0, 0, 1, 0, 1, 2, 3};
    int divisor, best_divisor, best_baud, best_baud_diff;
    int i;
    divisor = 24000000 / baudrate;

    // Round down to supported fraction (AM only)
    divisor -= am_adjust_dn[divisor & 7];

    // Try this divisor and the one above it (because division rounds down)
    best_divisor = 0;
    best_baud = 0;
    best_baud_diff = 0;
    for (i = 0; i < 2; i++)
    {
        int try_divisor = divisor + i;
        int baud_estimate;
        int baud_diff;

        // Round up to supported divisor value
        if (try_divisor <= 8)
        {
            // Round up to minimum supported divisor
            try_divisor = 8;
        }
        else if (divisor < 16)
        {
            // AM doesn't support divisors 9 through 15 inclusive
            try_divisor = 16;
        }
        else
        {
            // Round up to supported fraction (AM only)
            try_divisor += am_adjust_up[try_divisor & 7];
            if (try_divisor > 0x1FFF8)
            {
                // Round down to maximum supported divisor value (for AM)
                try_divisor = 0x1FFF8;
            }
        }
        // Get estimated baud rate (to nearest integer)
        baud_estimate = (24000000 + (try_divisor / 2)) / try_divisor;
        // Get absolute difference from requested baud rate
        if (baud_estimate < baudrate)
        {
            baud_diff = baudrate - baud_estimate;
        }
        else
        {
            baud_diff = baud_estimate - baudrate;
        }
        if (i == 0 || baud_diff < best_baud_diff)
        {
            // Closest to requested baud rate so far
            best_divisor = try_divisor;
            best_baud = baud_estimate;
            best_baud_diff = baud_diff;
            if (baud_diff == 0)
            {
                // Spot on! No point trying
                break;
            }
        }
    }
    // Encode the best divisor value
    *encoded_divisor = (best_divisor >> 3) | (frac_code[best_divisor & 7] << 14);
    // Deal with special cases for encoded value
    if (*encoded_divisor == 1)
    {
        *encoded_divisor = 0;    // 3000000 baud
    }
    else if (*encoded_divisor == 0x4001)
    {
        *encoded_divisor = 1;    // 2000000 baud (BM only)
    }
    return best_baud;
}


static int ftdi_to_clkbits(int baudrate, unsigned int clk, int clk_div, unsigned long *encoded_divisor)
 {
     static const char frac_code[8] = {0, 3, 2, 4, 1, 5, 6, 7};
     int best_baud = 0;
     int divisor, best_divisor;
     if (baudrate >=  clk/clk_div)
     {
         *encoded_divisor = 0;
         best_baud = clk/clk_div;
     }
     else if (baudrate >=  clk/(clk_div + clk_div/2))
     {
         *encoded_divisor = 1;
         best_baud = clk/(clk_div + clk_div/2);
     }
     else if (baudrate >=  clk/(2*clk_div))
     {
         *encoded_divisor = 2;
         best_baud = clk/(2*clk_div);
     }
     else
     {
         /* We divide by 16 to have 3 fractional bits and one bit for rounding */
         divisor = clk*16/clk_div / baudrate;
         if (divisor & 1) /* Decide if to round up or down*/
             best_divisor = divisor /2 +1;
         else
             best_divisor = divisor/2;
         if(best_divisor > 0x20000)
             best_divisor = 0x1ffff;
         best_baud = clk*16/clk_div/best_divisor;
         if (best_baud & 1) /* Decide if to round up or down*/
             best_baud = best_baud /2 +1;
         else
             best_baud = best_baud /2;
         *encoded_divisor = (best_divisor >> 3) | (frac_code[best_divisor & 0x7] << 14);
     }
     return best_baud;
 }
 /**
     ftdi_convert_baudrate returns nearest supported baud rate to that requested.
     Function is only used internally
     \internal
 */
static int ftdi_convert_baudrate(int baudrate, FTDI_HandleTypeDef* ftdi,
										 unsigned short *value, unsigned short *index)
{
  int best_baud;
  unsigned long encoded_divisor;

  if (baudrate <= 0)
  {
		// Return error
		return -1;
  }

#define H_CLK 120000000
#define C_CLK  48000000
  if ((ftdi->type == TYPE_2232H) || (ftdi->type == TYPE_4232H) || (ftdi->type == TYPE_232H))
  {
		if(baudrate*10 > H_CLK /0x3fff)
		{
			 /* On H Devices, use 12 000 000 Baudrate when possible
				 We have a 14 bit divisor, a 1 bit divisor switch (10 or 16)
				 three fractional bits and a 120 MHz clock
				 Assume AN_120 "Sub-integer divisors between 0 and 2 are not allowed" holds for
				 DIV/10 CLK too, so /1, /1.5 and /2 can be handled the same*/
			 best_baud = ftdi_to_clkbits(baudrate, H_CLK, 10, &encoded_divisor);
			 encoded_divisor |= 0x20000; /* switch on CLK/10*/
		}
		else
			 best_baud = ftdi_to_clkbits(baudrate, C_CLK, 16, &encoded_divisor);
  }
  else if ((ftdi->type == TYPE_BM) || (ftdi->type == TYPE_2232C) || (ftdi->type == TYPE_R) || (ftdi->type == TYPE_230X))
  {
		best_baud = ftdi_to_clkbits(baudrate, C_CLK, 16, &encoded_divisor);
  }
  else
  {
		best_baud = ftdi_to_clkbits_AM(baudrate, &encoded_divisor);
  }
  // Split into "value" and "index" values
  *value = (unsigned short)(encoded_divisor & 0xFFFF);
  if (ftdi->type == TYPE_2232H || ftdi->type == TYPE_4232H || ftdi->type == TYPE_232H)
  {
		*index = (unsigned short)(encoded_divisor >> 8);
		*index &= 0xFF00;
		*index |= ftdi->index;
  }
  else
		*index = (unsigned short)(encoded_divisor >> 16);

  // Return the nearest baud rate
  return best_baud;
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
