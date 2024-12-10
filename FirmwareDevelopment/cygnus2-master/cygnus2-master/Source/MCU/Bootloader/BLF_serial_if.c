/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2018 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : BLF_serial_if.c
*
*  Description  :
*
*   This module provides the interfaces to the serial module needed by the ST modules
*   for the bootloader (ymodem for e.g.)
*
*************************************************************************************/



/* System Include files
*******************************************************************************/
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

/* User Include files
*******************************************************************************/
#include "stm32l4xx.h"
#include "cmsis_os.h"
#include "common.h"
#include "CO_ErrorCode.h"
#include "DM_NVM.h"
#include "DM_SerialPort.h"
#include "BLF_serial_if.h"


/* Global Variables
*******************************************************************************/

/* UART identifier */
static UartComm_t bootloader_uart_id = PPU_UART_E;
static UartComm_t bootdebug_uart_id  = DEBUG_UART_E;
extern const SerialChannelDefinition_t SerialChannelDefinition[MAX_SUPPORTED_SERIAL_PORT_E]; 

static UART_HandleTypeDef* bootloader_uart_handle;
static UART_HandleTypeDef* bootdebug_uart_handle;


/*************************************************************************************/
/**
* function name: debug_out
* description  : Debug output
*
* @param - none
*
* @return - none
*/
HAL_StatusTypeDef debug_out( const char* format, ... );



/*************************************************************************************/
/**
* function name: serial_init
* description  : Initialisation routine of this module
*
* @param - none
*
* @return - none
*/
void serial_init(void)
{
   ErrorCode_t error_code = SUCCESS_E;

   /* Check if we have to use the portable programmer UART port or the debug port
   * The PPU port is used by default
   */
   uint32_t use_ppu_port = 1;
   //error_code = DM_NVMRead(NV_REPROG_USE_PP_UART_E, &use_ppu_port, sizeof(use_ppu_port));

   if ((SUCCESS_E != error_code) || (1u != use_ppu_port))
   {
      /* USe the Debug UART port for new image download */
      bootloader_uart_id = DEBUG_UART_E;
      bootdebug_uart_id  = PPU_UART_E; 
   }
   else
   {
      /* Use the Portable Programmer UART Port for new image download */
      bootloader_uart_id = PPU_UART_E;    
      bootdebug_uart_id  = DEBUG_UART_E; 
   }

   if ( UART_SUCCESS_E != SerialPortInit(bootloader_uart_id) ||
        UART_SUCCESS_E != SerialPortInit(bootdebug_uart_id) )
   {
      /* Initialization Error */
      HAL_NVIC_SystemReset();
   }

   //Read the PPU uart handle from the serial port channel definition
   SerialChannelDefinition_t* p_this_channel_def = (SerialChannelDefinition_t*)&SerialChannelDefinition[bootloader_uart_id];
   bootloader_uart_handle = &p_this_channel_def->p_port_data->uart_handle;
   //Read the DEBUG uart handle from the serial port channel definition
   p_this_channel_def = (SerialChannelDefinition_t*)&SerialChannelDefinition[bootdebug_uart_id];
   bootdebug_uart_handle = &p_this_channel_def->p_port_data->uart_handle;
}


/*************************************************************************************/
/**
* function name: serial_deinit
* description  : De-Initialisation routine of this module
*
* @param - none
*
* @return - none
*/
void serial_deinit(void)
{
   SerialPortDeInit(bootloader_uart_id);
   SerialPortDeInit(bootdebug_uart_id);   
}

/*************************************************************************************/
/**
* function name: Serial_PutString
* description  : Transmit a string on the serial port  NOTE: This goes to debug port NOT bootloader port! 
*
* @param - p_string: The string to be sent
*
* @return - HAL_StatusTypeDef HAL_OK if OK
*/
HAL_StatusTypeDef Serial_PutString(const uint8_t *const p_string)
   {
   HAL_StatusTypeDef status = HAL_ERROR;
   if ( p_string )
   {
      /* send string  */
      debug_out( (const char*) p_string );
      status = HAL_OK;
   }
   return status;
   }


/*************************************************************************************/
/**
* function name: Serial_PutByte
* description  : Transmit a byte on the serial port NOTE: This goes to bootloader port NOT debug port! 
*
* @param - param: The byte to be sent
*
* @return - HAL_StatusTypeDef HAL_OK if OK
*/
HAL_StatusTypeDef Serial_PutByte( const uint8_t param )
{
   /* send byte  */
   return HAL_UART_Transmit( bootloader_uart_handle, (uint8_t*) &param, 1, TX_TIMEOUT );
}

/*************************************************************************************/
/**
* function name: serial_request_new_packet
* description  : Request a new data packet from another host on the serial port
*
* @param - request_key: the key word to be sent to request a packet
* @param - p_data: pointer to where to store the received data
* @param - max_packet_size: maximum number of bytes to be received
* @param - received_packet_size: Actual number of bytes of the received data 
* @param - timeout_ms: timeout in ms
*
* @return - HAL_StatusTypeDef HAL_OK if OK
*/

 
HAL_StatusTypeDef serial_request_new_packet(const uint8_t request_key, const uint8_t *p_data, const uint16_t max_packet_size, uint16_t *const received_packet_size, const uint32_t timeout_ms)
   {
   HAL_StatusTypeDef result = HAL_ERROR;

   if ( p_data && received_packet_size )
   {
   *received_packet_size = 0;
      debug_out( "Requesting packet.." );

      //Introduce a short delay to give the serial interface time to clear before sending the packet request
      HAL_Delay( 150 );
      Serial_PutByte(request_key); /* Ask for a packet */
      debug_out( "key sent.\n.." );

   /* Start the reception operation */
      result = HAL_UART_Receive( bootloader_uart_handle, (uint8_t*)p_data, max_packet_size, timeout_ms );

   /* Calculate the number of received bytes */
      *received_packet_size = max_packet_size - bootloader_uart_handle->RxXferCount;

      if (HAL_TIMEOUT == result)
   {
         debug_out( "Packet timeout (got %d bytes)\n", *received_packet_size );
   }
   else
   {
         debug_out( "Packet Rx: %d bytes\n", *received_packet_size );
   }
   }

   return result;
}


/*************************************************************************************/
/**
* function name: serial_receive_data
* description  : Receive data on the serial port
*
* @param - p_data: pointer to where to store the received data
* @param - size: maximum number of bytes to be received
* @param - timeout_ms: timeout in ms
*
* @return - HAL_StatusTypeDef HAL_OK if OK
*/
HAL_StatusTypeDef serial_receive_data(const uint8_t *p_data, const uint16_t size, const uint32_t timeout_ms)
{
   HAL_StatusTypeDef status = HAL_ERROR;
   if ( p_data )
   {
      status = HAL_UART_Receive( bootloader_uart_handle, (uint8_t*) p_data, size, timeout_ms );;
   }
   return status;
      }
      

/*************************************************************************************/
/**
* function name: serial_send_data
* description  : Transmit a stream of bytes on the serial port
*
* @param - p_data: pointer on the data to be transmitted
* @param - timeout_ms: timeout in ms
* @param - size: size of the data
*
* @return - HAL_StatusTypeDef HAL_OK if OK
*/
 
HAL_StatusTypeDef serial_send_data(const uint8_t *const p_data, const uint16_t size, const uint32_t timeout_ms)
{
   HAL_StatusTypeDef status = HAL_ERROR;
   if ( p_data )
   {
      //Introduce a short delay to give the serial interface time to clear before sending a packet.
      HAL_Delay( 150 );

      status = HAL_UART_Transmit( bootloader_uart_handle, (uint8_t*) p_data, size, timeout_ms );
   }
   return status;
}



/*************************************************************************************/
/**
* function name: debug_out
* description  : Initialisation routine of this module
*
* @param - none
*
* @return - none
*/
HAL_StatusTypeDef debug_out( const char* format, ... )
   {
#if defined _DEBUG
   HAL_StatusTypeDef hal_status = HAL_ERROR;
   if ( format )
      {
      static char m_string[SERIAL_PORT_0_TX_BUFFER_SIZE];
      va_list argp;

      /* send the data */
      __va_start(argp, format);
      
      if(0 < vsprintf( m_string, format, argp))
      {
         hal_status = HAL_UART_Transmit( bootdebug_uart_handle, (uint8_t*)m_string, strlen(m_string), TX_TIMEOUT);
      }
      __va_end(argp);
      }

   return hal_status;
#else 
   UNUSED(bootdebug_uart_handle);//stop compiler warning for unused variable
   return HAL_OK;
#endif   
}
