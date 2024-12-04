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
#ifndef __USBH_FTDI_H
#define __USBH_FTDI_H

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

/** @addtogroup USBH_FTDI_CLASS
* @{
*/

/** @defgroup USBH_FTDI_CORE
* @brief This file is the Header file for usbh_core.c
* @{
*/




/*Communication Class codes*/
#define USB_FTDI_CLASS                                           0x02U
#define USB_VENDOR_CLASS													 0x00U

#define DEVICE_CLASS_CODE													 0x00U
#define COMMUNICATION_INTERFACE_CLASS_CODE                      0x02U


/*Data Interface Class Codes*/
#define DATA_INTERFACE_CLASS_CODE                               0x0AU



/*Communication sub class codes*/
#define RESERVED                                                0x00U
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
#define FTDI_PAGE_SIZE_64                                        0x40U

/*Class-Specific Request Codes*/
#define FTDI_SEND_ENCAPSULATED_COMMAND                           0x00U
#define FTDI_GET_ENCAPSULATED_RESPONSE                           0x01U
#define FTDI_SET_COMM_FEATURE                                    0x02U
#define FTDI_GET_COMM_FEATURE                                    0x03U
#define FTDI_CLEAR_COMM_FEATURE                                  0x04U

#define FTDI_SET_AUX_LINE_STATE                                  0x10U
#define FTDI_SET_HOOK_STATE                                      0x11U
#define FTDI_PULSE_SETUP                                         0x12U
#define FTDI_SEND_PULSE                                          0x13U
#define FTDI_SET_PULSE_TIME                                      0x14U
#define FTDI_RING_AUX_JACK                                       0x15U

#define FTDI_SET_LINE_CODING                                     0x20U
#define FTDI_GET_LINE_CODING                                     0x21U
#define FTDI_SET_CONTROL_LINE_STATE                              0x22U
#define FTDI_SEND_BREAK                                          0x23U

#define FTDI_SET_RINGER_PARMS                                    0x30U
#define FTDI_GET_RINGER_PARMS                                    0x31U
#define FTDI_SET_OPERATION_PARMS                                 0x32U
#define FTDI_GET_OPERATION_PARMS                                 0x33U
#define FTDI_SET_LINE_PARMS                                      0x34U
#define FTDI_GET_LINE_PARMS                                      0x35U
#define FTDI_DIAL_DIGITS                                         0x36U
#define FTDI_SET_UNIT_PARAMETER                                  0x37U
#define FTDI_GET_UNIT_PARAMETER                                  0x38U
#define FTDI_CLEAR_UNIT_PARAMETER                                0x39U
#define FTDI_GET_PROFILE                                         0x3AU

#define FTDI_SET_ETHERNET_MULTICAST_FILTERS                      0x40U
#define FTDI_SET_ETHERNET_POWER_MANAGEMENT_PATTERN FILTER        0x41U
#define FTDI_GET_ETHERNET_POWER_MANAGEMENT_PATTERN FILTER        0x42U
#define FTDI_SET_ETHERNET_PACKET_FILTER                          0x43U
#define FTDI_GET_ETHERNET_STATISTIC                              0x44U

#define FTDI_SET_ATM_DATA_FORMAT                                 0x50U
#define FTDI_GET_ATM_DEVICE_STATISTICS                           0x51U
#define FTDI_SET_ATM_DEFAULT_VC                                  0x52U
#define FTDI_GET_ATM_VC_STATISTICS                               0x53U


/* wValue for SetControlLineState*/
#define FTDI_ACTIVATE_CARRIER_SIGNAL_RTS                         0x0002U
#define FTDI_DEACTIVATE_CARRIER_SIGNAL_RTS                       0x0000U
#define FTDI_ACTIVATE_SIGNAL_DTR                                 0x0001U
#define FTDI_DEACTIVATE_SIGNAL_DTR                               0x0000U

#define LINE_CODING_STRUCTURE_SIZE                              0x07U
/**
  * @}
  */

/** @defgroup USBH_FTDI_CORE_Exported_Types
* @{
*/

/* States for CDC State Machine */
typedef enum
{
  FTDI_IDLE = 0U,
  FTDI_SEND_DATA,
  FTDI_SEND_DATA_WAIT,
  FTDI_RECEIVE_DATA,
  FTDI_RECEIVE_DATA_WAIT,
}
FTDI_DataStateTypeDef;

typedef enum
{
  FTDI_IDLE_STATE = 0U,
  FTDI_SET_LINE_CODING_STATE,
  FTDI_GET_LAST_LINE_CODING_STATE,
  FTDI_TRANSFER_DATA,
  FTDI_ERROR_STATE,
}
FTDI_StateTypeDef;


/*Line coding structure*/
typedef union _FTDI_LineCodingStructure
{
  uint8_t Array[LINE_CODING_STRUCTURE_SIZE];

  struct
  {

    uint32_t             dwDTERate;     /*Data terminal rate, in bits per second*/
    uint8_t              bCharFormat;   /*Stop bits
    0 - 1 Stop bit
    1 - 1.5 Stop bits
    2 - 2 Stop bits*/
    uint8_t              bParityType;   /* Parity
    0 - None
    1 - Odd
    2 - Even
    3 - Mark
    4 - Space*/
    uint8_t                bDataBits;     /* Data bits (5, 6, 7, 8 or 16). */
  }b;
}
FTDI_LineCodingTypeDef;



/* Header Functional Descriptor
--------------------------------------------------------------------------------
Offset|  field              | Size  |    Value   |   Description
------|---------------------|-------|------------|------------------------------
0     |  bFunctionLength    | 1     |   number   |  Size of this descriptor.
1     |  bDescriptorType    | 1     |   Constant |  CS_INTERFACE (0x24)
2     |  bDescriptorSubtype | 1     |   Constant |  Identifier (ID) of functional
      |                     |       |            | descriptor.
3     |  bcdCDC             | 2     |            |
      |                     |       |   Number   | USB Class Definitions for
      |                     |       |            | Communication Devices Specification
      |                     |       |            | release number in binary-coded
      |                     |       |            | decimal
------|---------------------|-------|------------|------------------------------
*/
typedef struct _FunctionalDescriptorHeader
{
  uint8_t     bLength;            /*Size of this descriptor.*/
  uint8_t     bDescriptorType;    /*CS_INTERFACE (0x24)*/
  uint8_t     bDescriptorSubType; /* Header functional descriptor subtype as*/
  uint16_t    bcdCDC;             /* USB Class Definitions for Communication
                                    Devices Specification release number in
                                  binary-coded decimal. */
}
FTDI_HeaderFuncDesc_TypeDef;
/* Call Management Functional Descriptor
--------------------------------------------------------------------------------
Offset|  field              | Size  |    Value   |   Description
------|---------------------|-------|------------|------------------------------
0     |  bFunctionLength    | 1     |   number   |  Size of this descriptor.
1     |  bDescriptorType    | 1     |   Constant |  CS_INTERFACE (0x24)
2     |  bDescriptorSubtype | 1     |   Constant |  Call Management functional
      |                     |       |            |  descriptor subtype.
3     |  bmCapabilities     | 1     |   Bitmap   | The capabilities that this configuration
      |                     |       |            | supports:
      |                     |       |            | D7..D2: RESERVED (Reset to zero)
      |                     |       |            | D1: 0 - Device sends/receives call
      |                     |       |            | management information only over
      |                     |       |            | the Communication Class
      |                     |       |            | interface.
      |                     |       |            | 1 - Device can send/receive call
      |                     \       |            | management information over a
      |                     |       |            | Data Class interface.
      |                     |       |            | D0: 0 - Device does not handle call
      |                     |       |            | management itself.
      |                     |       |            | 1 - Device handles call
      |                     |       |            | management itself.
      |                     |       |            | The previous bits, in combination, identify
      |                     |       |            | which call management scenario is used. If bit
      |                     |       |            | D0 is reset to 0, then the value of bit D1 is
      |                     |       |            | ignored. In this case, bit D1 is reset to zero for
      |                     |       |            | future compatibility.
4     | bDataInterface      | 1     | Number     | Interface number of Data Class interface
      |                     |       |            | optionally used for call management.
------|---------------------|-------|------------|------------------------------
*/
typedef struct _CallMgmtFunctionalDescriptor
{
  uint8_t    bLength;            /*Size of this functional descriptor, in bytes.*/
  uint8_t    bDescriptorType;    /*CS_INTERFACE (0x24)*/
  uint8_t    bDescriptorSubType; /* Call Management functional descriptor subtype*/
  uint8_t    bmCapabilities;      /* bmCapabilities: D0+D1 */
  uint8_t    bDataInterface;      /*bDataInterface: 1*/
}
FTDI_CallMgmtFuncDesc_TypeDef;
/* Abstract Control Management Functional Descriptor
--------------------------------------------------------------------------------
Offset|  field              | Size  |    Value   |   Description
------|---------------------|-------|------------|------------------------------
0     |  bFunctionLength    | 1     |   number   |  Size of functional descriptor, in bytes.
1     |  bDescriptorType    | 1     |   Constant |  CS_INTERFACE (0x24)
2     |  bDescriptorSubtype | 1     |   Constant |  Abstract Control Management
      |                     |       |            |  functional  descriptor subtype.
3     |  bmCapabilities     | 1     |   Bitmap   | The capabilities that this configuration
      |                     |       |            | supports ((A bit value of zero means that the
      |                     |       |            | request is not supported.) )
                                                   D7..D4: RESERVED (Reset to zero)
      |                     |       |            | D3: 1 - Device supports the notification
      |                     |       |            | Network_Connection.
      |                     |       |            | D2: 1 - Device supports the request
      |                     |       |            | Send_Break
      |                     |       |            | D1: 1 - Device supports the request
      |                     \       |            | combination of Set_Line_Coding,
      |                     |       |            | Set_Control_Line_State, Get_Line_Coding, and the
                                                   notification Serial_State.
      |                     |       |            | D0: 1 - Device supports the request
      |                     |       |            | combination of Set_Comm_Feature,
      |                     |       |            | Clear_Comm_Feature, and Get_Comm_Feature.
      |                     |       |            | The previous bits, in combination, identify
      |                     |       |            | which requests/notifications are supported by
      |                     |       |            | a Communication Class interface with the
      |                     |       |            |   SubClass code of Abstract Control Model.
------|---------------------|-------|------------|------------------------------
*/
typedef struct _AbstractCntrlMgmtFunctionalDescriptor
{
  uint8_t    bLength;            /*Size of this functional descriptor, in bytes.*/
  uint8_t    bDescriptorType;    /*CS_INTERFACE (0x24)*/
  uint8_t    bDescriptorSubType; /* Abstract Control Management functional
                                  descriptor subtype*/
  uint8_t    bmCapabilities;      /* The capabilities that this configuration supports */
}
FTDI_AbstCntrlMgmtFuncDesc_TypeDef;
/* Union Functional Descriptor
--------------------------------------------------------------------------------
Offset|  field              | Size  |    Value   |   Description
------|---------------------|-------|------------|------------------------------
0     |  bFunctionLength    | 1     |   number   |  Size of this descriptor.
1     |  bDescriptorType    | 1     |   Constant |  CS_INTERFACE (0x24)
2     |  bDescriptorSubtype | 1     |   Constant |  Union functional
      |                     |       |            |  descriptor subtype.
3     |  bMasterInterface   | 1     |   Constant | The interface number of the
      |                     |       |            | Communication or Data Class interface
4     | bSlaveInterface0    | 1     | Number     | nterface number of first slave or associated
      |                     |       |            | interface in the union.
------|---------------------|-------|------------|------------------------------
*/
typedef struct _UnionFunctionalDescriptor
{
  uint8_t    bLength;            /*Size of this functional descriptor, in bytes*/
  uint8_t    bDescriptorType;    /*CS_INTERFACE (0x24)*/
  uint8_t    bDescriptorSubType; /* Union functional descriptor SubType*/
  uint8_t    bMasterInterface;   /* The interface number of the Communication or
                                 Data Class interface,*/
  uint8_t    bSlaveInterface0;   /*Interface number of first slave*/
}
FTDI_UnionFuncDesc_TypeDef;

typedef struct _USBH_CDCInterfaceDesc
{
  FTDI_HeaderFuncDesc_TypeDef           FTDI_HeaderFuncDesc;
  FTDI_CallMgmtFuncDesc_TypeDef         FTDI_CallMgmtFuncDesc;
  FTDI_AbstCntrlMgmtFuncDesc_TypeDef    FTDI_AbstCntrlMgmtFuncDesc;
  FTDI_UnionFuncDesc_TypeDef            FTDI_UnionFuncDesc;
}
FTDI_InterfaceDesc_Typedef;


/* Structure for CDC process */
typedef struct
{
  uint8_t              NotifPipe;
  uint8_t              NotifEp;
  uint8_t              buff[8];
  uint16_t             NotifEpSize;
}
FTDI_CommItfTypedef ;

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
FTDI_DataItfTypedef ;



enum ftdi_chip_type
{
  TYPE_AM=0,
  TYPE_BM=1,
  TYPE_2232C=2,
  TYPE_R=3,
  TYPE_2232H=4,
  TYPE_4232H=5,
  TYPE_232H=6,
  TYPE_230X=7,
};


enum ftdi_interface
{
    INTERFACE_ANY = 0,
    INTERFACE_A   = 1,
    INTERFACE_B   = 2,
    INTERFACE_C   = 3,
    INTERFACE_D   = 4
};

/** Automatic loading / unloading of kernel modules */
enum ftdi_module_detach_mode
{
    AUTO_DETACH_SIO_MODULE = 0,
    DONT_DETACH_SIO_MODULE = 1,
    AUTO_DETACH_REATACH_SIO_MODULE = 2
};


/* Structure for CDC process */
typedef struct _FTDI_Process
{
  FTDI_CommItfTypedef                CommItf;
  FTDI_DataItfTypedef                DataItf;
  uint8_t                           *pTxData;
  uint8_t                           *pRxData;
  uint32_t                           TxDataLength;
  uint32_t                           RxDataLength;
  FTDI_InterfaceDesc_Typedef         FTDI_Desc;
  FTDI_LineCodingTypeDef             LineCoding;
  FTDI_LineCodingTypeDef             *pUserLineCoding;
  FTDI_StateTypeDef                  state;
  FTDI_DataStateTypeDef              data_tx_state;
  FTDI_DataStateTypeDef              data_rx_state;
  uint8_t                           Rx_Poll;
 
	
 /* USB specific */
   /** libusb's context */
  
   /** libusb's usb_dev_handle */
 
   /** usb read timeout */
   int usb_read_timeout;
   /** usb write timeout */
   int usb_write_timeout;

   /* FTDI specific */
   /** FTDI chip type */
   enum ftdi_chip_type type;
   /** baudrate */
   int baudrate;
   /** bitbang mode state */
   unsigned char bitbang_enabled;
   /** pointer to read buffer for ftdi_read_data */
   unsigned char *readbuffer;
   /** read buffer offset */
   unsigned int readbuffer_offset;
   /** number of remaining data in internal read buffer */
   unsigned int readbuffer_remaining;
   /** read buffer chunk size */
   unsigned int readbuffer_chunksize;
   /** write buffer chunk size */
   unsigned int writebuffer_chunksize;
   /** maximum packet size. Needed for filtering modem status bytes every n packets. */
   unsigned int max_packet_size;

   /* FTDI FT2232C requirecments */
   /** FT2232C interface number: 0 or 1 */
   int interface;   /* 0 or 1 */
   /** FT2232C index number: 1 or 2 */
   int index;       /* 1 or 2 */
   /* Endpoints */
   /** FT2232C end points: 1 or 2 */
   int in_ep;
   int out_ep;      /* 1 or 2 */

   /** Bitbang mode. 1: (default) Normal bitbang mode, 2: FT2232C SPI bitbang mode */
   unsigned char bitbang_mode;

   /** Decoded eeprom structure */
   struct ftdi_eeprom *eeprom;

   /** String representation of last error */
   const char *error_str;

   /** Defines behavior in case a kernel module is already attached to the device */
   enum ftdi_module_detach_mode module_detach_mode;	
}
FTDI_HandleTypeDef;


#define USB_REQUEST_TYPE_VENDOR 		(0x02 << 5)
#define USB_RECIPIENT_DEVICE			 0x0
#define USB_ENDPOINT_IN 		 		 0x80
#define USB_ENDPOINT_OUT				 0x00 


#define FTDI_DEVICE_OUT_REQTYPE (USB_REQUEST_TYPE_VENDOR | USB_RECIPIENT_DEVICE | USB_ENDPOINT_OUT)
#define FTDI_DEVICE_IN_REQTYPE (USB_REQUEST_TYPE_VENDOR | USB_RECIPIENT_DEVICE | USB_ENDPOINT_IN)


#define FTDI_SIO_RESET                  0  /* Reset the port */
#define FTDI_SIO_MODEM_CTRL             1  /* Set the modem control register */
#define FTDI_SIO_SET_FLOW_CTRL          2  /* Set flow control register */
#define FTDI_SIO_SET_BAUD_RATE          3  /* Set baud rate */
#define FTDI_SIO_SET_DATA               4  /* Set the data characteristics of the port */
#define FTDI_SIO_GET_MODEM_STATUS       5  /* Retrieve current value of modem status register */
#define FTDI_SIO_SET_EVENT_CHAR         6  /* Set the event character */
#define FTDI_SIO_SET_ERROR_CHAR         7  /* Set the error character */
#define FTDI_SIO_SET_LATENCY_TIMER      9  /* Set the latency timer */
#define FTDI_SIO_GET_LATENCY_TIMER      10 /* Get the latency timer */

#define FTDI_SIO_RESET_SIO              0
#define FTDI_SIO_RESET_PURGE_RX         1
#define FTDI_SIO_RESET_PURGE_TX         2

#define FTDI_SIO_SET_DATA_PARITY_NONE   (0x0 << 8 )
#define FTDI_SIO_SET_DATA_PARITY_ODD    (0x1 << 8 )
#define FTDI_SIO_SET_DATA_PARITY_EVEN   (0x2 << 8 )
#define FTDI_SIO_SET_DATA_PARITY_MARK   (0x3 << 8 )
#define FTDI_SIO_SET_DATA_PARITY_SPACE  (0x4 << 8 )
#define FTDI_SIO_SET_DATA_STOP_BITS_1   (0x0 << 11)
#define FTDI_SIO_SET_DATA_STOP_BITS_15  (0x1 << 11)
#define FTDI_SIO_SET_DATA_STOP_BITS_2   (0x2 << 11)
#define FTDI_SIO_SET_BREAK              (0x1 << 14)

#define FTDI_SIO_SET_DTR_MASK           0x1
#define FTDI_SIO_SET_DTR_HIGH           ( 1 | ( FTDI_SIO_SET_DTR_MASK  << 8))
#define FTDI_SIO_SET_DTR_LOW            ( 0 | ( FTDI_SIO_SET_DTR_MASK  << 8))
#define FTDI_SIO_SET_RTS_MASK           0x2
#define FTDI_SIO_SET_RTS_HIGH           ( 2 | ( FTDI_SIO_SET_RTS_MASK << 8 ))
#define FTDI_SIO_SET_RTS_LOW            ( 0 | ( FTDI_SIO_SET_RTS_MASK << 8 ))

#define FTDI_SIO_DISABLE_FLOW_CTRL      0x0
#define FTDI_SIO_RTS_CTS_HS             (0x1 << 8)
#define FTDI_SIO_DTR_DSR_HS             (0x2 << 8)
#define FTDI_SIO_XON_XOFF_HS            (0x4 << 8)

#define FTDI_SIO_CTS_MASK               0x10
#define FTDI_SIO_DSR_MASK               0x20
#define FTDI_SIO_RI_MASK                0x40
#define FTDI_SIO_RLSD_MASK              0x80

#define FTDI_SIO_SET_DATA_REQUEST_TYPE	 0x40


/**
* @}
*/

/** @defgroup USBH_FTDI_CORE_Exported_Defines
* @{
*/

/**
* @}
*/

/** @defgroup USBH_FTDI_CORE_Exported_Macros
* @{
*/
/**
* @}
*/

/** @defgroup USBH_FTDI_CORE_Exported_Variables
* @{
*/
extern USBH_ClassTypeDef  FTDI_Class;
#define USBH_FTDI_CLASS    &FTDI_Class

/**
* @}
*/

/** @defgroup USBH_FTDI_CORE_Exported_FunctionsPrototype
* @{
*/

USBH_StatusTypeDef  USBH_FTDI_SetLineCoding(USBH_HandleTypeDef *phost,
                                           FTDI_LineCodingTypeDef *linecoding);

USBH_StatusTypeDef  USBH_FTDI_GetLineCoding(USBH_HandleTypeDef *phost,
                                           FTDI_LineCodingTypeDef *linecoding);

USBH_StatusTypeDef  USBH_FTDI_Transmit(USBH_HandleTypeDef *phost,
                                      const uint8_t *pbuff,
                                      uint32_t length);

USBH_StatusTypeDef  USBH_FTDI_Receive(USBH_HandleTypeDef *phost,
                                     uint8_t *pbuff,
                                     uint32_t length);


uint16_t            USBH_FTDI_GetLastReceivedDataSize(USBH_HandleTypeDef *phost);

USBH_StatusTypeDef  USBH_FTDI_Stop(USBH_HandleTypeDef *phost);

void USBH_FTDI_LineCodingChanged(USBH_HandleTypeDef *phost);

void USBH_FTDI_TransmitCallback(USBH_HandleTypeDef *phost);

void USBH_FTDI_ReceiveCallback( USBH_HandleTypeDef *phost, uint8_t* buff, int length );
												  
USBH_StatusTypeDef USBH_FTDI_SetBAUD( USBH_HandleTypeDef *phost, uint32_t rate ); 
												  
USBH_StatusTypeDef USBH_FTDI_BreakCtrl( USBH_HandleTypeDef *phost, int break_state );
												  
USBH_StatusTypeDef USBH_FTDI_Setup( USBH_HandleTypeDef *phost, uint16_t stopbits, uint16_t parity, uint16_t databits );


/**
* @}
*/

#ifdef __cplusplus
}
#endif

#endif /* __USBH_FTDI_H */

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

