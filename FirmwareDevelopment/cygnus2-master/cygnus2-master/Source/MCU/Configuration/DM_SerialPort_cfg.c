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
*  File         : DM_SerialPort_cfg.c
*
*  Description  :configuration of the Serial Port Device Manager module
*
*************************************************************************************/



/* System Include files
*******************************************************************************/


/* User Include files
*******************************************************************************/
#include "stm32l4xx.h"
#include "DM_SerialPort_cfg.h"
#include "DM_SerialPort.h"

/* Private Functions Prototypes
*************************************************************************************/


/* Global Variables
*************************************************************************************/

const SerialChannelDefinition_t SerialChannelDefinition[MAX_SUPPORTED_SERIAL_PORT_E] =
{
   [DEBUG_UART_E] =
   {
      /* Configuration of the UART port */
      .uart_instance    = UART4,
      .baudrate         = 2000000u,
      .parity           = UART_PARITY_NONE,
      .uart_irq_number  = UART4_IRQn,

      /* Configuration of the DMA Channels */
      .tx_dma_channel     = DMA2_Channel3,
      .tx_dma_request_nbr = DMA_REQUEST_2,
      .tx_dma_irq_number  = DMA2_Channel3_IRQn,

      .rx_dma_channel     = DMA2_Channel5,
      .rx_dma_request_nbr = DMA_REQUEST_2,
      .rx_dma_irq_number  = DMA2_Channel5_IRQn,

      /* Tx/Rx pin configuration */
      .p_gpio_port = GPIOC,
      .rx_pin_id   = GPIO_PIN_11,
      .tx_pin_id   = GPIO_PIN_10,
      .alternate_selection = GPIO_AF8_UART4,
      .rx_pin_name = PC_11,

      /* UART port related data */
      .p_port_data = &uart_port_data[DEBUG_UART_E],            

       /* UART port Tx/Rx Data buffer definitions */
      .tx_buffer_ptr    = dm_serial_data_buffer.individual_buffers.serial_port_0_tx_buffer,
      .tx_buffer_size   = SERIAL_PORT_0_TX_BUFFER_SIZE,
      .rx_buffer_ptr    = dm_serial_data_buffer.individual_buffers.serial_port_0_rx_buffer,
      .rx_buffer_size   = SERIAL_PORT_0_RX_BUFFER_SIZE,
   },

   [PPU_UART_E] =
   {
      /* Configuration of the UART port */
      .uart_instance    = USART1,
      .baudrate         = 115200,
      .parity           = UART_PARITY_NONE,
      .uart_irq_number  = USART1_IRQn,
      
      /* Configuration of the DMA Channels */
      .tx_dma_channel     = DMA1_Channel4,
      .tx_dma_request_nbr = DMA_REQUEST_2,
      .tx_dma_irq_number  = DMA1_Channel4_IRQn,

      .rx_dma_channel     = DMA1_Channel5,
      .rx_dma_request_nbr = DMA_REQUEST_2,
      .rx_dma_irq_number  = DMA1_Channel5_IRQn,
      
      /* Tx/Rx pin configuration */
      .p_gpio_port = GPIOA,
      .rx_pin_id   = GPIO_PIN_10,
      .tx_pin_id   = GPIO_PIN_9,
      .alternate_selection = GPIO_AF7_USART1,
      .rx_pin_name = PA_10,

      /* UART port related data */
      .p_port_data = &uart_port_data[PPU_UART_E],
      
       /* UART port Tx/Rx Data buffer definitions */
      .tx_buffer_ptr    = dm_serial_data_buffer.individual_buffers.serial_port_1_tx_buffer,
      .tx_buffer_size   = SERIAL_PORT_1_TX_BUFFER_SIZE,
      .rx_buffer_ptr    = dm_serial_data_buffer.individual_buffers.serial_port_1_rx_buffer,
      .rx_buffer_size   = SERIAL_PORT_1_RX_BUFFER_SIZE,
   },

   [HEAD_UART_E]=
   {
      /* Configuration of the UART port */
      .uart_instance    = LPUART1,
      .baudrate         = 19200u,
      .parity           = UART_PARITY_NONE,
      .uart_irq_number  = LPUART1_IRQn,

      /* Configuration of the DMA Channels */
      .tx_dma_channel     = DMA2_Channel6,
      .tx_dma_request_nbr = DMA_REQUEST_4,
      .tx_dma_irq_number  = DMA2_Channel6_IRQn,
      .rx_dma_channel     = DMA2_Channel7,
      .rx_dma_request_nbr = DMA_REQUEST_4,
      .rx_dma_irq_number  = DMA2_Channel7_IRQn,
      
      /* Tx/Rx pin configuration */
      .p_gpio_port = GPIOC,
      .rx_pin_id   = GPIO_PIN_0,
      .tx_pin_id   = GPIO_PIN_1,
      .alternate_selection = GPIO_AF8_LPUART1,
      .rx_pin_name = PC_0,

      /* UART port related data */
      .p_port_data = &uart_port_data[HEAD_UART_E],
      
       /* UART port Tx/Rx Data buffer definitions */
      .tx_buffer_ptr    = dm_serial_data_buffer.individual_buffers.serial_port_2_tx_buffer,
      .tx_buffer_size   = SERIAL_PORT_2_TX_BUFFER_SIZE,
      .rx_buffer_ptr    = dm_serial_data_buffer.individual_buffers.serial_port_2_rx_buffer,
      .rx_buffer_size   = SERIAL_PORT_2_RX_BUFFER_SIZE,
   },

   [NCU_SBC_UART_E]=
   {
      /* Configuration of the UART port */
      .uart_instance    = USART3,
      .baudrate         = 115200u,
      .parity           = UART_PARITY_NONE,
      .uart_irq_number  = USART3_IRQn,

      .tx_dma_channel     = DMA1_Channel2,
      .tx_dma_request_nbr = DMA_REQUEST_2,
      .tx_dma_irq_number  = DMA1_Channel2_IRQn,
      .rx_dma_channel     = DMA1_Channel3,
      .rx_dma_request_nbr = DMA_REQUEST_2,
      .rx_dma_irq_number  = DMA1_Channel3_IRQn,

      /* Tx/Rx pin configuration */
      .p_gpio_port = GPIOC,
      .rx_pin_id   = GPIO_PIN_5,
      .tx_pin_id   = GPIO_PIN_4,
      .alternate_selection = GPIO_AF7_USART3,
      .rx_pin_name = PC_5,

      /* UART port related data */
      .p_port_data = &uart_port_data[NCU_SBC_UART_E],
      
       /* UART port Tx/Rx Data buffer definitions */
      .tx_buffer_ptr    = dm_serial_data_buffer.individual_buffers.serial_port_2_tx_buffer,
      .tx_buffer_size   = SERIAL_PORT_2_TX_BUFFER_SIZE,
      .rx_buffer_ptr    = dm_serial_data_buffer.individual_buffers.serial_port_2_rx_buffer,
      .rx_buffer_size   = SERIAL_PORT_2_RX_BUFFER_SIZE,
   },

   [NCU_CP_UART_E] =
   {
      /* Configuration of the UART port */
      .uart_instance    = USART2,
      .baudrate         = 115200u,
      .parity           = UART_PARITY_NONE,
      .uart_irq_number  = USART2_IRQn,

      /* Configuration of the DMA Channels */
      .tx_dma_channel     = DMA1_Channel7,
      .tx_dma_request_nbr = DMA_REQUEST_2,
      .tx_dma_irq_number  = DMA1_Channel7_IRQn,
      .rx_dma_channel     = DMA1_Channel6,
      .rx_dma_request_nbr = DMA_REQUEST_2,
      .rx_dma_irq_number  = DMA1_Channel6_IRQn,
      
      /* Tx/Rx pin configuration */
      .p_gpio_port = GPIOA,
      .rx_pin_id   = GPIO_PIN_3,
      .tx_pin_id   = GPIO_PIN_2,
      .alternate_selection = GPIO_AF7_USART2,
      .rx_pin_name = PA_3,

      /* UART port related data */
      .p_port_data = &uart_port_data[NCU_CP_UART_E],
      
       /* UART port Tx/Rx Data buffer definitions */
      .tx_buffer_ptr    = dm_serial_data_buffer.individual_buffers.serial_port_3_tx_buffer,
      .tx_buffer_size   = SERIAL_PORT_3_TX_BUFFER_SIZE,
      .rx_buffer_ptr    = dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer,
      .rx_buffer_size   = SERIAL_PORT_3_RX_BUFFER_SIZE,
   },   
};

/* Private Variables
*************************************************************************************/

