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
*  File         : DM_SerialPort_cfg.h
*
*  Description  : Header file of the configuration of the Serial Port Device Manager module
*
*************************************************************************************/

#ifndef SERIAL_PORT_CFG_H
#define SERIAL_PORT_CFG_H


/* System Include files
*******************************************************************************/



/* User Include files
*******************************************************************************/


/* Public Structures
*******************************************************************************/
// The size of the buffers 
#define SERIAL_PORT_0_RX_BUFFER_SIZE   	128
#define SERIAL_PORT_0_TX_BUFFER_SIZE   	512

#define SERIAL_PORT_1_RX_BUFFER_SIZE   	128
#define SERIAL_PORT_1_TX_BUFFER_SIZE   	128

#define SERIAL_PORT_2_RX_BUFFER_SIZE   	128
#define SERIAL_PORT_2_TX_BUFFER_SIZE   	128

#define SERIAL_PORT_3_RX_BUFFER_SIZE   	128
#define SERIAL_PORT_3_TX_BUFFER_SIZE   	128

/* Public Enumerations
*******************************************************************************/ 
typedef enum
{
   DEBUG_UART_E,
   PPU_UART_E,
   HEAD_UART_E,
   NCU_CP_UART_E,
   NCU_SBC_UART_E,
   MAX_SUPPORTED_SERIAL_PORT_E,
   MESH_INTERFACE_E,
   PPU_INTERFACE_E,
   INVALID_SERIAL_PORT_E
} UartComm_t;

/* Public functions prototypes
*******************************************************************************/


/* Public Constants
*******************************************************************************/
#define NUMBER_OF_UARTS    (4u)

/* Macros
*******************************************************************************/

/* Debug port Interrupt handler definitions */
#define DEBUG_UART_IRQHandler        UART4_IRQHandler
#define DEBUG_UART_DMA_Tx_IRQHandler DMA2_Channel3_IRQHandler
#define DEBUG_UART_DMA_Rx_IRQHandler DMA2_Channel5_IRQHandler

/* Portable programmer port Interrupt handler definitions */
#define PPU_UART_IRQHandler         USART1_IRQHandler
#define PPU_UART_DMA_Tx_IRQHandler  DMA1_Channel4_IRQHandler
#define PPU_UART_DMA_Rx_IRQHandler  DMA1_Channel5_IRQHandler

/* Head Interface port Interrupt handler definitions */
#define HEAD_UART_IRQHandler        LPUART1_IRQHandler
#define HEAD_UART_DMA_Tx_IRQHandler DMA2_Channel6_IRQHandler
#define HEAD_UART_DMA_Rx_IRQHandler DMA2_Channel7_IRQHandler

/* Control Panel port Interrupt handler definitions */
#define CP_UART_IRQHandler          USART2_IRQHandler
#define CP_UART_DMA_Tx_IRQHandler   DMA1_Channel7_IRQHandler
#define CP_UART_DMA_Rx_IRQHandler   DMA1_Channel6_IRQHandler

/* Single Board Computer port Interrupt handler definitions */
#define SBC_UART_IRQHandler         USART3_IRQHandler
#define SBC_UART_DMA_Tx_IRQHandler  DMA1_Channel2_IRQHandler
#define SBC_UART_DMA_Rx_IRQHandler  DMA1_Channel3_IRQHandler

#endif
