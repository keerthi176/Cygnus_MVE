/*************************************************************************************
*  CONFIDENTIAL between PA and Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2017 PA Consulting Group. All rights reserved.
*  Cambridge Technology Centre
*  Melbourn Royston
*  Herts SG8 6DP UK
*  Tel: <44> 1763 261222
*
**************************************************************************************
*  File         : DM_SerialPort.h
*
*  Description  : Header file for serial_port module
*
*************************************************************************************/

#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H


/* System Include files
*******************************************************************************/
#include <stdbool.h>
#include <stdint.h>


/* User Include files
*******************************************************************************/
#include "stm32l4xx.h"
#include "main.h"
#include "CO_ErrorCode.h"
#include "gpio.h"
#include "DM_SerialPort_cfg.h"

/* Public Structures
*******************************************************************************/
typedef void (*wake_up_callback_function_t)(void);

typedef struct
{
   uint16_t rx_buffer_read_ptr;
   uint16_t rx_buffer_write_ptr;
   uint16_t tx_buffer_write_ptr;
   uint16_t tx_buffer_read_ptr;
}buffer_run_data_t;

typedef struct
{
   buffer_run_data_t buffer_run_data;
   /* UART instance Handle */
   UART_HandleTypeDef uart_handle;

   /* Handles of the DMA channels */
   DMA_HandleTypeDef  hdma_tx_handle;
   DMA_HandleTypeDef  hdma_rx_handle;
}uart_port_data_t;

typedef struct
{
   /* Configuration of the UART port */
   USART_TypeDef *uart_instance;
   uint32_t      baudrate;
   uint32_t      parity;

   /* IRQ number of the UART */
   IRQn_Type     uart_irq_number;

   /* Configuration of the DMA Channels */
   DMA_Channel_TypeDef* tx_dma_channel;
   uint32_t             tx_dma_request_nbr;
   DMA_Channel_TypeDef* rx_dma_channel;
   uint32_t             rx_dma_request_nbr;

   /* IRQ number of the DMA channels */
   IRQn_Type     tx_dma_irq_number;
   IRQn_Type     rx_dma_irq_number;

   /* Tx/Rx pin configuration */
   GPIO_TypeDef *p_gpio_port;
   uint16_t     rx_pin_id;
   uint16_t     tx_pin_id;
   uint32_t     alternate_selection;
   PinNames     rx_pin_name;

   /* Info related to the actual port */
   uart_port_data_t *p_port_data;

   /* UART port Tx/Rx Data buffer definitions */
   uint8_t       *tx_buffer_ptr;
   uint8_t       *rx_buffer_ptr;
   uint16_t      tx_buffer_size;
   uint16_t      rx_buffer_size;

}SerialChannelDefinition_t;

typedef union
{
   struct {
      /* Port 0 */
      uint8_t serial_port_0_rx_buffer[SERIAL_PORT_0_RX_BUFFER_SIZE];
      uint8_t serial_port_0_tx_buffer[SERIAL_PORT_0_TX_BUFFER_SIZE];

      /* Port 1 */
      uint8_t serial_port_1_rx_buffer[SERIAL_PORT_1_RX_BUFFER_SIZE];
      uint8_t serial_port_1_tx_buffer[SERIAL_PORT_1_TX_BUFFER_SIZE];

      /* Port 2 */
      uint8_t serial_port_2_rx_buffer[SERIAL_PORT_2_RX_BUFFER_SIZE];
      uint8_t serial_port_2_tx_buffer[SERIAL_PORT_2_TX_BUFFER_SIZE];

      /* Port 3 */
      uint8_t serial_port_3_rx_buffer[SERIAL_PORT_3_RX_BUFFER_SIZE];
      uint8_t serial_port_3_tx_buffer[SERIAL_PORT_3_TX_BUFFER_SIZE];
   }individual_buffers;
   uint8_t global_buffer[SERIAL_PORT_0_RX_BUFFER_SIZE + SERIAL_PORT_0_TX_BUFFER_SIZE + SERIAL_PORT_1_RX_BUFFER_SIZE + SERIAL_PORT_1_TX_BUFFER_SIZE
      + SERIAL_PORT_2_RX_BUFFER_SIZE + SERIAL_PORT_2_TX_BUFFER_SIZE + SERIAL_PORT_3_RX_BUFFER_SIZE + SERIAL_PORT_3_TX_BUFFER_SIZE];
}dm_serial_data_buffer_t;

extern dm_serial_data_buffer_t dm_serial_data_buffer;
extern uart_port_data_t uart_port_data[MAX_SUPPORTED_SERIAL_PORT_E];


/* Public Enumerations
*******************************************************************************/ 
typedef enum
{
   UART_SUCCESS_E,
   ERROR_UART_INIT_E,
   ERROR_UART_NO_FOUND_E,
   ERROR_UART_BUFFER_EMPTY_E,
   ERROR_UART_E,
   UART_BUSY_E,
   UART_TIMEOUT_E
} SerialPortOperationResult_t;

/* Public functions prototypes
*******************************************************************************/
ErrorCode_t SerialPort_Init(void);
void SerialPortDataInit(uint8_t uart_port);
SerialPortOperationResult_t SerialPortInit(uint8_t uart_port);
SerialPortOperationResult_t SerialPortDeInit(uint8_t uart_port);
void SerialPortGpioDeInit(uint8_t uart_port);
SerialPortOperationResult_t SerialPortWriteTxBufferRom(uint8_t uart_port, uint8_t* pData);
SerialPortOperationResult_t SerialPortWriteTxBuffer(uint8_t uart_port, uint8_t* pData, uint16_t length);
SerialPortOperationResult_t SerialPortStartReception(uint8_t uart_port);
SerialPortOperationResult_t SerialPort_StartSingleReception(uint8_t uart_port, uint8_t *p_rx_buffer, uint16_t length);
uint16_t SerialPortBytesInRxBuffer(uint8_t uart_port);
uint16_t SerialPort_RemainingByteToReceive(uint8_t uart_port);
uint8_t SerialPortFindATModeSequenceRxBuffer(uint8_t uart_port);
void SerialPortFlushRxBuffer(uint8_t uart_port);
uint8_t SerialPortFindCRLFRxBuffer(uint8_t uart_port);
uint8_t SerialPortReadRxBuffer(uint8_t uart_port, uint8_t* pData, uint16_t length);
bool SerialPort_CanGoToSleep(void);
bool SerialPortEnterStopMode(uint8_t uart_port, wake_up_callback_function_t callback);
void SerialPortLeaveStopMode(uint8_t uart_port);
bool SerialPort_IsTxInProgress(uint8_t uart_port);
bool SerialPort_IsRxInProgress(uint8_t uart_port);


/* Public Constants
*******************************************************************************/
#define PARITY_EVEN                       0
#define PARITY_ODD                        1
#define PARITY_NONE                       2

#define RS_232                            0
#define RS_485                            1
#define RS_NONE                           2

#define UART_NORMAL                       0
#define UART_MODBUS                       1


/* Macros
*******************************************************************************/




#endif
