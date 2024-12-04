/**
  ******************************************************************************
  * @file    usbh_cdc.h
  * @author  MCD Application Team
  * @brief   This file contains all the prototypes for the usbh_cdc.c
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

/* Define to prevent recursive  ----------------------------------------------*/
#ifndef __USBH_DEVICE_H
#define __USBH_DEVICE_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"


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
* @brief This file is the Header file for usbh_core.c
* @{
*/




/*Communication Class codes*/
#define USB_DEVICE_CLASS                                           0x02U
#define USB_VENDOR_CLASS													 0x00U

#define DEVICE_CLASS 												 0x00U
#define COMMUNICATION_INTERFACE_CLASS_CODE                      0x02U


/*Data Interface Class Codes*/
#define DATA_INTERFACE_CLASS_CODE                               0x0AU



/*Communication sub class codes*/
#define RESERVED_CODE                                           0x00U
#define DIRECT_LINE_CONTROL_MODEL                               0x01U
#define ABSTRACT_CONTROL_MODEL                                  0x02U
#define TELEPHONE_CONTROL_MODEL                                 0x03U
#define MULTICHANNEL_CONTROL_MODEL                              0x04U
#define CAPI_CONTROL_MODEL                                      0x05U
#define ETHERNET_NETWORKING_CONTROL_MODEL                       0x06U
#define ATM_NETWORKING_CONTROL_MODEL                            0x07U


/*Communication Interface Class Control Protocol Codes*/
#define NO_CLASS_SPECIFIC_PROTOCOL_CODE                         0x00U
#define COMMON_AT_COMMAND                                       0x01U
#define VENDOR_SPECIFIC                                         0xFFU


#define CS_INTERFACE                                            0x24U 

 

/* States for CDC State Machine */
typedef enum
{
  DEVICE_IDLE = 0U,
  DEVICE_SEND_DATA,
  DEVICE_SEND_DATA_WAIT,
  DEVICE_RECEIVE_DATA,
  DEVICE_RECEIVE_DATA_WAIT,
}
DEVICE_DataStateTypeDef;

typedef enum
{
  DEVICE_IDLE_STATE = 0U,
  DEVICE_SET_LINE_CODING_STATE,
  DEVICE_GET_LAST_LINE_CODING_STATE,
  DEVICE_TRANSFER_DATA,
  DEVICE_ERROR_STATE,
}
DEVICE_StateTypeDef;
 
 

/* Structure for CDC process */
typedef struct
{
  uint8_t              NotifPipe;
  uint8_t              NotifEp;
  uint8_t              buff[8];
  uint16_t             NotifEpSize;
}
DEVICE_CommItfTypedef ;

typedef struct
{
  uint8_t              InPipe;
  uint8_t              OutPipe;
  uint8_t              OutEp;
  uint8_t              InEp;
  uint8_t              buff[8];
  uint16_t             OutEpSize;
  uint16_t             InEpSize;
}
DEVICE_DataItfTypedef ;


  


/* Structure for CDC process */
typedef struct _DEVICE_Process
{
  DEVICE_CommItfTypedef                CommItf;
  DEVICE_DataItfTypedef                DataItf;
  uint8_t                           *pTxData;
  uint8_t                           *pRxData;
  uint32_t                           TxDataLength;
  uint32_t                           RxDataLength;
  
  DEVICE_StateTypeDef                  state;
  DEVICE_DataStateTypeDef              data_tx_state;
  DEVICE_DataStateTypeDef              data_rx_state;
  uint8_t                           Rx_Poll;
 
	
 /* USB specific */
   /** libusb's context */
  
  
}
DEVICE_HandleTypeDef;


#define USB_REQUEST_TYPE_VENDOR 		(0x02 << 5)
#define USB_RECIPIENT_DEVICE			 0x0
#define USB_ENDPOINT_IN 		 		 0x80
#define USB_ENDPOINT_OUT				 0x00 


#define DEVICE_DEVICE_OUT_REQTYPE (USB_REQUEST_TYPE_VENDOR | USB_RECIPIENT_DEVICE | USB_ENDPOINT_OUT)
#define DEVICE_DEVICE_IN_REQTYPE (USB_REQUEST_TYPE_VENDOR | USB_RECIPIENT_DEVICE | USB_ENDPOINT_IN)


#define DEVICE_SIO_RESET                  0  /* Reset the port */
#define DEVICE_SIO_MODEM_CTRL             1  /* Set the modem control register */
#define DEVICE_SIO_SET_FLOW_CTRL          2  /* Set flow control register */
#define DEVICE_SIO_SET_BAUD_RATE          3  /* Set baud rate */
#define DEVICE_SIO_SET_DATA               4  /* Set the data characteristics of the port */
#define DEVICE_SIO_GET_MODEM_STATUS       5  /* Retrieve current value of modem status register */
#define DEVICE_SIO_SET_EVENT_CHAR         6  /* Set the event character */
#define DEVICE_SIO_SET_ERROR_CHAR         7  /* Set the error character */
#define DEVICE_SIO_SET_LATENCY_TIMER      9  /* Set the latency timer */
#define DEVICE_SIO_GET_LATENCY_TIMER      10 /* Get the latency timer */

#define DEVICE_SIO_RESET_SIO              0
#define DEVICE_SIO_RESET_PURGE_RX         1
#define DEVICE_SIO_RESET_PURGE_TX         2

#define DEVICE_SIO_SET_DATA_PARITY_NONE   (0x0 << 8 )
#define DEVICE_SIO_SET_DATA_PARITY_ODD    (0x1 << 8 )
#define DEVICE_SIO_SET_DATA_PARITY_EVEN   (0x2 << 8 )
#define DEVICE_SIO_SET_DATA_PARITY_MARK   (0x3 << 8 )
#define DEVICE_SIO_SET_DATA_PARITY_SPACE  (0x4 << 8 )
#define DEVICE_SIO_SET_DATA_STOP_BITS_1   (0x0 << 11)
#define DEVICE_SIO_SET_DATA_STOP_BITS_15  (0x1 << 11)
#define DEVICE_SIO_SET_DATA_STOP_BITS_2   (0x2 << 11)
#define DEVICE_SIO_SET_BREAK              (0x1 << 14)

#define DEVICE_SIO_SET_DTR_MASK           0x1
#define DEVICE_SIO_SET_DTR_HIGH           ( 1 | ( DEVICE_SIO_SET_DTR_MASK  << 8))
#define DEVICE_SIO_SET_DTR_LOW            ( 0 | ( DEVICE_SIO_SET_DTR_MASK  << 8))
#define DEVICE_SIO_SET_RTS_MASK           0x2
#define DEVICE_SIO_SET_RTS_HIGH           ( 2 | ( DEVICE_SIO_SET_RTS_MASK << 8 ))
#define DEVICE_SIO_SET_RTS_LOW            ( 0 | ( DEVICE_SIO_SET_RTS_MASK << 8 ))

#define DEVICE_SIO_DISABLE_FLOW_CTRL      0x0
#define DEVICE_SIO_RTS_CTS_HS             (0x1 << 8)
#define DEVICE_SIO_DTR_DSR_HS             (0x2 << 8)
#define DEVICE_SIO_XON_XOFF_HS            (0x4 << 8)

#define DEVICE_SIO_CTS_MASK               0x10
#define DEVICE_SIO_DSR_MASK               0x20
#define DEVICE_SIO_RI_MASK                0x40
#define DEVICE_SIO_RLSD_MASK              0x80

 
/** @defgroup USBH_DEVICE_CORE_Exported_Variables
* @{
*/
extern USBH_ClassTypeDef  DEVICE_Class;
#define USBH_DEVICE_CLASS    &DEVICE_Class

/**
* @}
*/

/** @defgroup USBH_DEVICE_CORE_Exported_FunctionsPrototype
* @{
*/
 
USBH_StatusTypeDef  USBH_DEVICE_Transmit(USBH_HandleTypeDef *phost,
                                      uint8_t *pbuff,
                                      uint32_t length);

USBH_StatusTypeDef  USBH_DEVICE_Receive(USBH_HandleTypeDef *phost,
                                     uint8_t *pbuff,
                                     uint32_t length);


uint16_t            USBH_DEVICE_GetLastReceivedDataSize(USBH_HandleTypeDef *phost);

USBH_StatusTypeDef  USBH_DEVICE_Stop(USBH_HandleTypeDef *phost);
 
void USBH_DEVICE_TransmitCallback(USBH_HandleTypeDef *phost);

void USBH_DEVICE_ReceiveCallback( USBH_HandleTypeDef *phost, uint8_t* buff, int length );

/**
* @}
*/

#ifdef __cplusplus
}
#endif

#endif /* __USBH_DEVICE_H */

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

