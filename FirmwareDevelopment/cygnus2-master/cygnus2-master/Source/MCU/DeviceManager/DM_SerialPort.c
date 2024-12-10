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
*  File         : DM_SerialPort.c
*
*  Description  :
*
*   This module provides functionality for:
*   -Handling UART Rx interrupt
*   -Handling UART Tx interrupt
*   -Writing Tx buffer
*   -Reading Rx buffer
*   -Determining number of bytes in RX buffer
*   -Searching <CR><LF> sequence in input buffer
*   -Opening serial port and automatically setting baud rate generator
*
**************************************************************************************
*   UART routines usage
*
*   1. open the serial port with serial_port_open(baudrate), make sure SYSTEM_FOSC is 
*      set properly in systemdefs.h
*   2. use function serial_port_bytes_in_rx_buffer to determine the number of bytes in
*      the rx buffer
*   3. use function serial_port_read_rx_buffer to read incoming bytes from the rx buffer
*   4. use function serial_port_write_tx_buffer to write bytes to the tx buffer
*
**************************************************************************************
*   UART routines Mechanism
*
*   Two circular buffers of 256 bytes are implemented. The receive interrupt
*   routines will copy all the received bytes into the buffer, if the buffer is 
*   full the bytes are discarted. The main program should check regularly the buffer
*   to process the incoming bytes.
*   The function serial_port_write_tx_buffer copy as string into the transmit buffer and 
*   enables the UART tx interrupt
*
**************************************************************************************
*   UART routines Limitations
*
*   The UART rx input buffer is limited to 256 bytes, if data is not read before
*   buffer gets full, the new bytes will be lost.
*
*   If the tx buffer is full the serial_port_write_tx_buffer routine will wait until
*   there is enough room in the buffer to copy the data. This will degrade
*   performance. Global interrupts and peripheral interrupts should be enabled,
*   and the UART should be open before using this function to avoid locking up
*   the execution
*************************************************************************************/



/* System Include files
*******************************************************************************/
#include "stdio.h"
#include "string.h"
#include "cmsis_os.h"

/* User Include files
*******************************************************************************/
#include "stm32l4xx.h"
#include "MM_ConfigSerialTask.h"
#include "board.h"
#include "DM_SerialPort.h"
#include "DM_SerialPort_cfg.h"

/* Private Functions Prototypes
*************************************************************************************/
static void SerialPortEnableInterrupts(UART_HandleTypeDef *huart);
static void SerialPortGpioInit(uint8_t uart_port);

/* Global Variables
*************************************************************************************/

__align(4)
dm_serial_data_buffer_t dm_serial_data_buffer;

extern const SerialChannelDefinition_t SerialChannelDefinition[MAX_SUPPORTED_SERIAL_PORT_E];
extern osMutexId(DebugPortMutexId);

/* Private Variables
*************************************************************************************/

uart_port_data_t uart_port_data[MAX_SUPPORTED_SERIAL_PORT_E];


/*************************************************************************************/
/**
* function name   : SerialPortDataInit(uint8_t uart_port)
* description      : Initialise the data structures associated with the uart module
*
* @param - uint8_t uart_port : uart port (see UartComm_t in DM_SerialPort.h)
*
* @return - nothing
*/
void SerialPortDataInit(uint8_t uart_port)
{
   if (uart_port < MAX_SUPPORTED_SERIAL_PORT_E)
   {
      buffer_run_data_t *const p_buffer_run_data = &SerialChannelDefinition[uart_port].p_port_data->buffer_run_data;      

      p_buffer_run_data->rx_buffer_read_ptr = 0u;
      p_buffer_run_data->rx_buffer_write_ptr = 0u;
      p_buffer_run_data->tx_buffer_write_ptr = 0u;
      p_buffer_run_data->tx_buffer_read_ptr = 0u;
   }
}

/*************************************************************************************/
/**
* function name   : SerialPort_Init
* description      : Initialise this module
*
* @param - none
*
* @return - SUCCESS_E if everything is ok
*/
ErrorCode_t SerialPort_Init(void)
{
   for(uint8_t port = 0; port < NUMBER_OF_UARTS; port++)
   {
      SerialPortDataInit(port);
      uart_port_data[port].hdma_rx_handle.Instance = NULL;
      uart_port_data[port].hdma_tx_handle.Instance = NULL;
      uart_port_data[port].uart_handle.Instance = NULL;
   }
  
   return SUCCESS_E;
}

/*************************************************************************************/
/**
* function name   : SerialPortInit(uint8_t uart_port)
* description      : Open uart module and sets the configuration registers
*
* @param - uint8_t uart_port : uart port (see UartComm_t in DM_SerialPort.h)
*
* @return - SerialPortOperationResult_t (see DM_SerialPort.h)
*/
SerialPortOperationResult_t SerialPortInit(uint8_t uart_port)
{
   SerialPortOperationResult_t result = ERROR_UART_NO_FOUND_E;

   if (MAX_SUPPORTED_SERIAL_PORT_E > uart_port)
   {
      SerialChannelDefinition_t *const p_this_channel_def = (SerialChannelDefinition_t*)&SerialChannelDefinition[uart_port];

      /* Initialise the data of this port */
      SerialPortDataInit(uart_port);

      /* Uart Peripheral clock enable */
      if(USART1 == p_this_channel_def->uart_instance)
      {
         __HAL_RCC_USART1_CLK_ENABLE();
      }
      else if(USART2 == p_this_channel_def->uart_instance)
      {
         __HAL_RCC_USART2_CLK_ENABLE();
      }
      else if(USART3 == p_this_channel_def->uart_instance)
      {
         __HAL_RCC_USART3_CLK_ENABLE();
      }
      else if(UART4 == p_this_channel_def->uart_instance)
      {
         __HAL_RCC_UART4_CLK_ENABLE();
      }
      else if(LPUART1 == p_this_channel_def->uart_instance)
      {
         __HAL_RCC_LPUART1_CLK_ENABLE();
      }
      
      UART_HandleTypeDef *const uart_handle = &p_this_channel_def->p_port_data->uart_handle;
      
      uart_handle->Index = uart_port;
      uart_handle->Instance = p_this_channel_def->uart_instance;
      uart_handle->Init.BaudRate = p_this_channel_def->baudrate;
      uart_handle->Init.WordLength = UART_WORDLENGTH_8B;
      uart_handle->Init.StopBits = UART_STOPBITS_1;
      uart_handle->Init.Parity = p_this_channel_def->parity;
      uart_handle->Init.Mode = UART_MODE_TX_RX;
      uart_handle->Init.HwFlowCtl = UART_HWCONTROL_NONE;
      uart_handle->Init.OverSampling = UART_OVERSAMPLING_16;
      uart_handle->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
      uart_handle->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

      // Init uart register
      if (HAL_OK == HAL_UART_Init(uart_handle))
      {
         result = UART_SUCCESS_E;
         
         /* Clear RXNE interrupt flag */
         __HAL_UART_SEND_REQ(uart_handle, UART_RXDATA_FLUSH_REQUEST);
      }
      else
      {
         result = ERROR_UART_INIT_E;
      }
   }

   return result;
}


/*************************************************************************************/
/**
* function name   : SerialPortDeInit(uint8_t uart_port, uint32_t baudrate, uint32_t parity)
* description      : De-initializes uart module
*
* @param - uint8_t uart_port : uart port (see UartComm_t in DM_SerialPort.h)
*
* @return - SerialPortOperationResult_t (see DM_SerialPort.h)
*/
SerialPortOperationResult_t SerialPortDeInit(uint8_t uart_port)
{
   SerialPortOperationResult_t result = ERROR_UART_NO_FOUND_E;

   if (MAX_SUPPORTED_SERIAL_PORT_E > uart_port)
   {
      UART_HandleTypeDef *const base_uart = &SerialChannelDefinition[uart_port].p_port_data->uart_handle;

      if (NULL != base_uart)
      {
         // De-Init uart register
         if (HAL_OK == HAL_UART_DeInit(base_uart))
         {
            result = UART_SUCCESS_E;
         }
         else
         {
            result = ERROR_UART_INIT_E;
         }
         
         /* Uart Peripheral clock disable */
         if(USART1 == base_uart->Instance)
         {
            __HAL_RCC_USART1_CLK_DISABLE();
         }
         else if(USART2 == base_uart->Instance)
         {
            __HAL_RCC_USART2_CLK_DISABLE();
         }
         else if(USART3 == base_uart->Instance)
         {
            __HAL_RCC_USART3_CLK_DISABLE();
         }
         else if(UART4 == base_uart->Instance)
         {
            __HAL_RCC_UART4_CLK_DISABLE();
         }
         else if(LPUART1 == base_uart->Instance)
         {
            __HAL_RCC_LPUART1_CLK_DISABLE();
         }
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* function name   : SerialPortGpioInit(UART_HandleTypeDef* uartHandle)
* description      : Configure GPIO lines associated with the uart module
*
* @param - UART_HandleTypeDef* uartHandle : pointer to uart handle
*
* @return - nothing
*/
static void SerialPortGpioInit(uint8_t uart_port)
{
   GPIO_InitTypeDef GPIO_InitStruct;
   SerialChannelDefinition_t *const p_this_channel_def = (SerialChannelDefinition_t*)&SerialChannelDefinition[uart_port];

   if( p_this_channel_def->p_gpio_port == GPIOA )
	{
      if(__HAL_RCC_GPIOA_IS_CLK_DISABLED() )
      {
         __HAL_RCC_GPIOA_CLK_ENABLE( );
      }
	}
   else if( p_this_channel_def->p_gpio_port == GPIOB )
	{
      if(__HAL_RCC_GPIOB_IS_CLK_DISABLED() )
      {
         __HAL_RCC_GPIOB_CLK_ENABLE( );
      }
	}
   else if( p_this_channel_def->p_gpio_port == GPIOC )
	{
      if(__HAL_RCC_GPIOC_IS_CLK_DISABLED() )
      {
         __HAL_RCC_GPIOC_CLK_ENABLE( );
      }
	}
   else if( p_this_channel_def->p_gpio_port == GPIOD )
	{
      if(__HAL_RCC_GPIOD_IS_CLK_DISABLED() )
      {
         __HAL_RCC_GPIOD_CLK_ENABLE( );
      }
	}
   else if( p_this_channel_def->p_gpio_port == GPIOH )
	{
      if(__HAL_RCC_GPIOH_IS_CLK_DISABLED() )
      {
         __HAL_RCC_GPIOH_CLK_ENABLE( );
      }
	}

   /* Now configure the GPIO pins */
   GPIO_InitStruct.Pin = p_this_channel_def->rx_pin_id|p_this_channel_def->tx_pin_id;
   GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
   GPIO_InitStruct.Pull = GPIO_PULLUP;
   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
   GPIO_InitStruct.Alternate = p_this_channel_def->alternate_selection;
   HAL_GPIO_Init(p_this_channel_def->p_gpio_port, &GPIO_InitStruct);
}

/*************************************************************************************/
/**
* function name   : SerialPortGpioDeInit(UART_HandleTypeDef* uartHandle)
* description      : Deinit GPIO lines associated with the uart module and stop peripheral
*                    clock   
*
* @param - UART_HandleTypeDef* uartHandle : pointer to uart handle
*
* @return - nothing
*/
void SerialPortGpioDeInit(uint8_t uart_port)
{
   SerialChannelDefinition_t *const p_this_channel_def = (SerialChannelDefinition_t*)&SerialChannelDefinition[uart_port];

   /* GPIO lines associated with Uart */
   HAL_GPIO_DeInit(p_this_channel_def->p_gpio_port, p_this_channel_def->rx_pin_id|p_this_channel_def->tx_pin_id);

   // Peripheral interrupt Deinit
   HAL_NVIC_DisableIRQ(p_this_channel_def->uart_irq_number);
}

/*************************************************************************************/
/**
* function name   :    SerialPortWriteTxBuffer(uint8_t uart_port, uint8_t* pData, uint16_t length)
* description      :    Copy the number of bytes specified in length variable from data
*                     into the tx buffer. The transmission is interrupt driven.
*
* @param - uint8_t uart_port : uart port (see UartComm_t in DM_SerialPort.h)
* @param - uint8_t* pData : pointer to buffer with data to be TX'd
* @param - uint8_t length : number of bytes to be TX'd
*
* @return - SerialPortOperationResult_t
*
* CAUTION:
*/
SerialPortOperationResult_t SerialPortWriteTxBuffer(uint8_t uart_port, uint8_t* pData, uint16_t length)
{
   UART_HandleTypeDef* base_uart;
   uint16_t temp;
   SerialPortOperationResult_t operation_result = ERROR_UART_NO_FOUND_E;
   bool access_granted = true;
   
   // validate function parameter
   if ( (MAX_SUPPORTED_SERIAL_PORT_E > uart_port) && (NULL != pData))
   {
      /* Acquire the mutex before access to the port */
      if ( (DEBUG_UART_E == uart_port) && (osMutexWait(DebugPortMutexId, 10u) != osOK) )
      {
         access_granted = false;
      }
      
      if ( access_granted )
      {
         // Identify UART object
         SerialChannelDefinition_t *const p_this_channel_def = (SerialChannelDefinition_t*)&SerialChannelDefinition[uart_port];
         base_uart = &p_this_channel_def->p_port_data->uart_handle;

         if (HAL_UART_STATE_RESET == base_uart->gState)
         {
            /* Port not initialised, we were in the stop mode */
            SerialPortLeaveStopMode(uart_port);
         }

         if (NULL != base_uart->hdmatx)
         {
            buffer_run_data_t *const p_buffer_run_data = &p_this_channel_def->p_port_data->buffer_run_data;

            HAL_NVIC_DisableIRQ(p_this_channel_def->uart_irq_number);
            
            /* This UART uses DMA for transmissions */
            if ( (base_uart->gState != HAL_UART_STATE_BUSY_TX) && (base_uart->gState != HAL_UART_STATE_BUSY_TX_RX) )
            {
               if (length <= p_this_channel_def->tx_buffer_size )
               {
                  /* No Transmission is in progress, start a fresh operation */
                  memcpy(p_this_channel_def->tx_buffer_ptr,pData,length);         
                  p_buffer_run_data->tx_buffer_write_ptr = length;
                  p_buffer_run_data->tx_buffer_read_ptr  = length;

                  /* Request a DMA transmition */
                  if (HAL_OK == HAL_UART_Transmit_DMA(base_uart, p_this_channel_def->tx_buffer_ptr, length))
                  {
                     /* Request was successful */
                     operation_result = UART_SUCCESS_E;
                  }
                  else
                  {
                     /* Something went wrong */
                     operation_result = ERROR_UART_E;
                  }
               }
               else
               {
                  /* Message is too long */
                  operation_result =  ERROR_UART_E;
               }
            }
            else
            {
               // Check if tx buffer is full
               temp = p_buffer_run_data->tx_buffer_write_ptr + length;

               if (temp < p_this_channel_def->tx_buffer_size)
               {
                  /* Check we have enough room for this message */
                  memcpy(&p_this_channel_def->tx_buffer_ptr[p_buffer_run_data->tx_buffer_write_ptr],pData,length);
                  p_buffer_run_data->tx_buffer_write_ptr = temp;
               }
               else
               {
                  /* Handle the Wrap around here */
                  if ( (temp % p_this_channel_def->tx_buffer_size) < p_buffer_run_data->tx_buffer_read_ptr)
                  {
                     /* Copy the first bit as trailing part to the Tx buffer */
                     uint16_t footer = p_this_channel_def->tx_buffer_size - p_buffer_run_data->tx_buffer_write_ptr;
                     memcpy(&p_this_channel_def->tx_buffer_ptr[p_buffer_run_data->tx_buffer_write_ptr], pData, footer);
                     
                     /* Copy the header */
                     uint16_t header = length - footer;
                     memcpy(&p_this_channel_def->tx_buffer_ptr[0], &pData[footer], header);
                     
                     /* Update the write pointer */
                     p_buffer_run_data->tx_buffer_write_ptr = header;

                     /* Request was successful */
                     operation_result = UART_SUCCESS_E;

                  }
                  else
                  {
                     /* Not enough available space */
                     operation_result = ERROR_UART_E;
                  }
               }
            }
            
            /* Restore this uart interrupt */
            HAL_NVIC_EnableIRQ(p_this_channel_def->uart_irq_number);
         }
         
         if ( DEBUG_UART_E == uart_port )
         {
            osMutexRelease(DebugPortMutexId);
         }
      }
   }

   /* Tx Request accepted */
   return operation_result;
}


/*************************************************************************************/
/**
* function name   :    SerialPortWriteTxBufferRom(uint8_t uart_port, uint8_t* pData)
* description      :    Copy a string from rom to the tx buffer, this function avoid the
*                     need of copying the strings into a temporary buffer
*
* @param - uint8_t uart_port : uart port (see UartComm_t in DM_SerialPort.h)
* @param - uint8_t* pData : pointer to buffer with data to be TX'd
*
* @return - SerialPortOperationResult_t
*
* CAUTION:    If the TX buffer is full, the function waits until there is enough space 
*            to copy everything
*/
SerialPortOperationResult_t SerialPortWriteTxBufferRom(uint8_t uart_port, uint8_t* pData)
{
   uint16_t Size;
   SerialPortOperationResult_t result = ERROR_UART_E;
   
   if (NULL != pData)
   {   
      // Get size of string
      Size = (uint16_t)strlen((char*)pData);
      
      result = SerialPortWriteTxBuffer(uart_port, (uint8_t*)pData, Size);
   }
   
   return result;
}

/*************************************************************************************/
/**
* function name   : SerialPort_StartSingleReception
* description     : Start a single Rx Operation
*
* @param - uint8_t uart_port : uart port (see UartComm_t in DM_SerialPort.h)
* @param - uint8_t* pData : pointer to buffer with data to be RX'd
* @param - uint8_t length : number of bytes to be RX'd
*
* @return - SerialPortOperationResult_t
*/
SerialPortOperationResult_t SerialPort_StartSingleReception(uint8_t uart_port, uint8_t *p_rx_buffer, uint16_t length)
{
   SerialPortOperationResult_t result = ERROR_UART_E;

   // validate function parameters
   if ((uart_port < MAX_SUPPORTED_SERIAL_PORT_E) && (NULL != p_rx_buffer))
   {      
      UART_HandleTypeDef *const base_uart = &SerialChannelDefinition[uart_port].p_port_data->uart_handle;
      DMA_HandleTypeDef *const rx_hdma = &SerialChannelDefinition[uart_port].p_port_data->hdma_rx_handle;
      HAL_StatusTypeDef hal_result;
      
      /* Set DMA to Normal mode */
      if ( HAL_IS_BIT_SET(rx_hdma->Instance->CCR, DMA_CCR_CIRC) )
      {
         CLEAR_BIT(rx_hdma->Instance->CCR, DMA_CCR_CIRC);
      }

      /* Reload the Rx DMA channel */
      hal_result = HAL_UART_Receive_DMA(base_uart, 
                                        p_rx_buffer,
                                        length);
      
      if (HAL_OK == hal_result)
      {
         result = UART_SUCCESS_E;
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* function name   : SerialPortStartReception
* description     : Start a new Rx Operation
*
* @param - uint8_t uart_port : uart port (see UartComm_t in DM_SerialPort.h)
*
*/
SerialPortOperationResult_t SerialPortStartReception(uint8_t uart_port)
{
   SerialPortOperationResult_t result = ERROR_UART_E;

   // validate function parameter
   if (uart_port < MAX_SUPPORTED_SERIAL_PORT_E)
   {      
      UART_HandleTypeDef *const base_uart = &SerialChannelDefinition[uart_port].p_port_data->uart_handle;
      DMA_HandleTypeDef *const rx_hdma = &SerialChannelDefinition[uart_port].p_port_data->hdma_rx_handle;

      HAL_StatusTypeDef hal_result;
      
      /* Set DMA to Circular mode */
      if ( HAL_IS_BIT_CLR(rx_hdma->Instance->CCR, DMA_CCR_CIRC) )
      {
         SET_BIT(rx_hdma->Instance->CCR, DMA_CCR_CIRC);
      }
      
      /* Reload the Rx DMA channel */
      hal_result = HAL_UART_Receive_DMA(base_uart, 
                                        SerialChannelDefinition[uart_port].rx_buffer_ptr,
                                        SerialChannelDefinition[uart_port].rx_buffer_size);
      
      if (HAL_OK == hal_result)
      {
         result = UART_SUCCESS_E;
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* function name   : SerialPortBytesInRxBuffer(uint8_t uart_port)
* description      : Return the number of bytes held in the RX buffer
*
* @param - uint8_t uart_port : uart port (see UartComm_t in DM_SerialPort.h)
*
* @return - uint16_t - number of bytes held in RX buffer
*/
uint16_t SerialPortBytesInRxBuffer(uint8_t uart_port)
{
   uint16_t temp = 0;
   uint16_t read_ptr, write_ptr, buffer_size;
   
   // validate function parameter
   if (uart_port < MAX_SUPPORTED_SERIAL_PORT_E)
   {      
      // Identify UART object
      SerialChannelDefinition_t *const p_this_channel_def = (SerialChannelDefinition_t*)&SerialChannelDefinition[uart_port];
      buffer_run_data_t *const p_buffer_run_data = &p_this_channel_def->p_port_data->buffer_run_data;
      DMA_HandleTypeDef *const p_hdma_rx_handle  = p_this_channel_def->p_port_data->uart_handle.hdmarx;
   
      if (NULL != p_hdma_rx_handle)
      {
         // Capture state of buffer
         read_ptr = p_buffer_run_data->rx_buffer_read_ptr;
         buffer_size = p_this_channel_def->rx_buffer_size;
         
         /* Calculate the Write pointer address */
         write_ptr = buffer_size - __HAL_DMA_GET_COUNTER(p_hdma_rx_handle);
         if ( write_ptr >= buffer_size)
         {
            write_ptr = 0;
         }

         if ( write_ptr >= read_ptr)
         {
            temp = write_ptr - read_ptr;
         }
         else
         {
            /* This is wrap arround */
            temp = write_ptr + buffer_size- read_ptr;
         }

      }
   }

   return temp;
}

/*************************************************************************************/
/**
* function name   : SerialPort_RemainingByteToReceive(uint8_t uart_port)
* description      : Return the number of remaining bytes to be received
*
* @param - uint8_t uart_port : uart port (see UartComm_t in DM_SerialPort.h)
*
* @return - uint16_t - Number of remaining bytes to be received
*/
uint16_t SerialPort_RemainingByteToReceive(uint8_t uart_port)
{
   uint16_t number_of_bytes = 0u;

   if (uart_port < MAX_SUPPORTED_SERIAL_PORT_E)
   {      
      // Identify UART object
      UART_HandleTypeDef *const base_uart = &SerialChannelDefinition[uart_port].p_port_data->uart_handle;
      
      number_of_bytes = __HAL_DMA_GET_COUNTER(base_uart->hdmarx);
   }

   return number_of_bytes;
   
}

/*************************************************************************************/
/**
* function name   : SerialPortFindATModeSequenceRxBuffer(uint8_t uart_port)
* description      : Check for +++ sequence in RX buffer of the Serial Port
*
* @param - uint8_t uart_port : uart port (see UartComm_t in DM_SerialPort.h)
*
* @return - uint8_t - number of bytes matching '+' in the the RX buffer
*/
uint8_t SerialPortFindATModeSequenceRxBuffer(uint8_t uart_port)
{
   uint8_t i = 0;
   uint16_t read_ptr, write_ptr, buffer_size;

   SerialChannelDefinition_t *const p_this_channel_def = (SerialChannelDefinition_t*)&SerialChannelDefinition[uart_port];
   buffer_run_data_t *const p_buffer_run_data = &p_this_channel_def->p_port_data->buffer_run_data;
   DMA_HandleTypeDef *const p_hdma_rx_handle  = p_this_channel_def->p_port_data->uart_handle.hdmarx;
   
   if (NULL != p_hdma_rx_handle)
   {
      /* Calculate the Write pointer address */
      p_buffer_run_data->rx_buffer_write_ptr = p_this_channel_def->rx_buffer_size - __HAL_DMA_GET_COUNTER(p_hdma_rx_handle);
      if ( p_buffer_run_data->rx_buffer_write_ptr >= p_this_channel_def->rx_buffer_size)
      {
         p_buffer_run_data->rx_buffer_write_ptr = 0;
      }
   }
   
   // Capture serial port data pointer
   read_ptr    = p_buffer_run_data->rx_buffer_read_ptr;
   write_ptr   = p_buffer_run_data->rx_buffer_write_ptr;
   buffer_size = p_this_channel_def->rx_buffer_size;

   /* Check that the RX buffer is not empty */
   if (read_ptr != write_ptr) 
   {  
      // Search for '+'
      while (  (p_this_channel_def->rx_buffer_ptr[read_ptr] == '+') 
             &&(read_ptr != write_ptr)) 
      {
         i++;
         if (++read_ptr >= buffer_size)
         {
            /* Wrap around, resume from the beginning of the buffer */
            read_ptr = 0u;
         }
      }

      // Check that the <CR><LF> is present
      if (  (p_this_channel_def->rx_buffer_ptr[read_ptr] == '\r')
          &&(read_ptr != write_ptr)) 
      {
         if (++read_ptr >= buffer_size)
         {
            /* Wrap around, resume from the beginning of the buffer */
            read_ptr = 0u;
         }
         
         if (  (p_this_channel_def->rx_buffer_ptr[read_ptr] == '\n')
             &&(read_ptr != write_ptr)) 
         {
            i += 2u;
         }
         else
         {
            /* Sequence is not valid or not complete yet */
            i = 0u;
         }
      }
      else
      {
         /* Sequence is not valid or not complete yet */
         i = 0u;
      }
   }
   return i;
}

/*************************************************************************************/
/**
* function name   : SerialPortFlushRxBuffer(uint8_t uart_port)
* description      : Empty serial_port_rx_buffer
*
* @param - uint8_t uart_port : uart port (see UartComm_t in DM_SerialPort.h)
*
* @return - nothing
*/
void SerialPortFlushRxBuffer(uint8_t uart_port)
{
   // validate function parameter
   if (MAX_SUPPORTED_SERIAL_PORT_E > uart_port)
   {
      // Identify UART object
      UART_HandleTypeDef *const base_uart = &SerialChannelDefinition[uart_port].p_port_data->uart_handle;
      
      HAL_UART_AbortReceive(base_uart);

      SerialChannelDefinition_t *const p_this_channel_def = (SerialChannelDefinition_t*)&SerialChannelDefinition[uart_port];
      buffer_run_data_t *const p_buffer_run_data = &p_this_channel_def->p_port_data->buffer_run_data;
      
      p_buffer_run_data->rx_buffer_read_ptr = 0;
      p_buffer_run_data->rx_buffer_write_ptr = 0;
   }
}

/*************************************************************************************/
/**
* function name   : SerialPortFindCRLFRxBuffer(uint8_t uart_port)
* description      : Search for <CR><LF> sequence and return the number of bytes
*                    in the RX buffer including <CR><LF>
*
* @param - uint8_t uart_port : uart port (see UartComm_t in DM_SerialPort.h)
*
* @return -  0 if there is no <CR><LF> sequence in the rx buffer
*            n=1-255 n is the number bytes in the buffer before the <CR><LF>
*            sequence + 2. (e.g. rx buffer = "seq 1<CR><LF>seq 2<CR><LF>seq3"
*            n will be 7, that is the length in bytes of seq 1<CR><LF>)
*/
uint8_t SerialPortFindCRLFRxBuffer(uint8_t uart_port)
{
   uint8_t i = 0;
   uint16_t read_ptr, write_ptr, buffer_size;

   SerialChannelDefinition_t *const p_this_channel_def = (SerialChannelDefinition_t*)&SerialChannelDefinition[uart_port];
   buffer_run_data_t *const p_buffer_run_data = &p_this_channel_def->p_port_data->buffer_run_data;
   UART_HandleTypeDef *const base_uart = &p_this_channel_def->p_port_data->uart_handle;

   if (NULL != base_uart->hdmarx)
   {
      /* Calculate the Write pointer address */
      p_buffer_run_data->rx_buffer_write_ptr = p_this_channel_def->rx_buffer_size - __HAL_DMA_GET_COUNTER(base_uart->hdmarx);
      if ( p_buffer_run_data->rx_buffer_write_ptr >= p_this_channel_def->rx_buffer_size)
      {
         p_buffer_run_data->rx_buffer_write_ptr = 0;
      }
   }   
       
   // Capture state of buffer
   read_ptr = p_buffer_run_data->rx_buffer_read_ptr;
   write_ptr = p_buffer_run_data->rx_buffer_write_ptr;
   buffer_size = p_this_channel_def->rx_buffer_size;
   bool sequence_found = false;
   
   /* Check that the RX buffer is not empty */
   if (read_ptr != write_ptr) 
   {        
      // Search the portion of the buffer for <CR><LF>
      do
      {
         if (  (p_this_channel_def->rx_buffer_ptr[read_ptr] == '\r')
             &&(read_ptr != write_ptr)) 
         {
            /* Move to the next Byte */
            i++;
            if (++read_ptr >= buffer_size)
            {
               /* Wrap around, resume from the beginning of the buffer */
               read_ptr = 0u;
            }
            
            if (  (p_this_channel_def->rx_buffer_ptr[read_ptr] == '\n')
                &&(read_ptr != write_ptr)) 
            {
               /* <CR><LF> was found */
               sequence_found = true;
               i ++;
            }            
         }
         else
         {
            /* Move to the next Byte */
            i++;
            if (++read_ptr >= buffer_size)
            {
               /* Wrap around, resume from the beginning of the buffer */
               read_ptr = 0u;
            }
         }
      } while((sequence_found == false) && (read_ptr != write_ptr));      
   }
   
   if (sequence_found == false)
   {
      /* Sequence not found, return size of Zero bytes */
      i = 0u;
   }
   
   return i;
}


/*************************************************************************************/
/**
* function name   : SerialPortReadRxBuffer(uint8_t uart_port, uint8_t* pData, uint16_t length)
* description      : Read length bytes from the RX buffer and copy them to pData pointer
*
* @param - uint8_t uart_port : uart port (see UartComm_t in DM_SerialPort.h)
* @param - uint8_t* pData: pointer to buffer
* @param - uint16_t length: number of bytes to copy from RX buffer
*
* @return -  Return the number of bytes read from the rx buffer
*/
uint8_t SerialPortReadRxBuffer(uint8_t uart_port, uint8_t* pData, uint16_t length)
{
   uint8_t i = 0;
    
   // validate function parameter
   if ( (MAX_SUPPORTED_SERIAL_PORT_E > uart_port) && (NULL != pData))
   {
      SerialChannelDefinition_t *const p_this_channel_def = (SerialChannelDefinition_t*)&SerialChannelDefinition[uart_port];
      buffer_run_data_t *const p_buffer_run_data = &p_this_channel_def->p_port_data->buffer_run_data;
      DMA_HandleTypeDef *const p_hdma_rx_handle = p_this_channel_def->p_port_data->uart_handle.hdmarx;

      if (NULL != p_hdma_rx_handle)
      {
         /* Calculate the Write pointer address */
         p_buffer_run_data->rx_buffer_write_ptr = p_this_channel_def->rx_buffer_size - __HAL_DMA_GET_COUNTER(p_hdma_rx_handle);
         if ( p_buffer_run_data->rx_buffer_write_ptr >= p_this_channel_def->rx_buffer_size)
         {
            p_buffer_run_data->rx_buffer_write_ptr = 0;
         }
         
      }
      
      while (i < length && (p_buffer_run_data->rx_buffer_read_ptr != p_buffer_run_data->rx_buffer_write_ptr)) 
      {
         pData[i++] = p_this_channel_def->rx_buffer_ptr[p_buffer_run_data->rx_buffer_read_ptr++];
         /* Check any buffer Wrap-around */
         if (p_buffer_run_data->rx_buffer_read_ptr >= p_this_channel_def->rx_buffer_size)
         {
            /* Wrap around od the Rx buffer */
            p_buffer_run_data->rx_buffer_read_ptr = 0u;
         }
      }
   }
   return i;
}

/*************************************************************************************/
/**
* function name   : SerialPortEnableInterrupts()
* description     : Enable interrupts used by a UART channel
*
* @param huart: UART handle pointer
*
* @return -  none
*/
static void SerialPortEnableInterrupts(UART_HandleTypeDef *huart)
{
   if ( NULL != huart)
   {
      SerialChannelDefinition_t *const p_this_channel_def = (SerialChannelDefinition_t*)&SerialChannelDefinition[huart->Index];

      /* NVIC for USART, to catch the TX complete */
      HAL_NVIC_SetPriority(p_this_channel_def->uart_irq_number, 1, 0);
      HAL_NVIC_EnableIRQ(p_this_channel_def->uart_irq_number);
      
      /* Enable the interrupts of the DMA channels linked to this UART */
      HAL_NVIC_SetPriority(p_this_channel_def->tx_dma_irq_number, 1, 0);
      HAL_NVIC_EnableIRQ(p_this_channel_def->tx_dma_irq_number);
      HAL_NVIC_SetPriority(p_this_channel_def->rx_dma_irq_number, 1, 0);
      HAL_NVIC_EnableIRQ(p_this_channel_def->rx_dma_irq_number);

   }
}

/**
  * @brief UART MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - DMA configuration for transmission request by peripheral 
  *           - NVIC configuration for DMA interrupt request enable
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
   if ( NULL != huart)
   {
      /* Get the UART index */
      const uint8_t uIndex = huart->Index;
      
      /* Get this channel definition */
      const SerialChannelDefinition_t *const p_this_channel_def = (SerialChannelDefinition_t*)&SerialChannelDefinition[uIndex];

      if (   (NULL != p_this_channel_def->tx_dma_channel)
          || (NULL != p_this_channel_def->rx_dma_channel))
      {
         /* Enable the DMA clock if required */
         if(__HAL_RCC_DMA1_IS_CLK_DISABLED() )
         {
            __HAL_RCC_DMA1_CLK_ENABLE();
         }
         
         if(__HAL_RCC_DMA2_IS_CLK_DISABLED() )
         {
            __HAL_RCC_DMA2_CLK_ENABLE();
         }

         /* Configure the Tx DMA channel if required */
         if (NULL != p_this_channel_def->tx_dma_channel)
         {
            DMA_HandleTypeDef *const dma_handle = &p_this_channel_def->p_port_data->hdma_tx_handle;
            
            dma_handle->Instance                 = p_this_channel_def->tx_dma_channel;
            dma_handle->Init.Request             = p_this_channel_def->tx_dma_request_nbr;         
            dma_handle->Init.Direction           = DMA_MEMORY_TO_PERIPH;
            dma_handle->Init.PeriphInc           = DMA_PINC_DISABLE;
            dma_handle->Init.MemInc              = DMA_MINC_ENABLE;
            dma_handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
            dma_handle->Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
            dma_handle->Init.Mode                = DMA_NORMAL;
            dma_handle->Init.Priority            = DMA_PRIORITY_LOW;
            HAL_DMA_Init(dma_handle);  
            
            /* Associate the initialized DMA handle to the the UART handle */
            __HAL_LINKDMA(huart, hdmatx, p_this_channel_def->p_port_data->hdma_tx_handle);
         }
         
         /* Configure the Rx DMA channel if required */
         if (NULL != p_this_channel_def->rx_dma_channel)
         {
            DMA_HandleTypeDef *const dma_handle = &p_this_channel_def->p_port_data->hdma_rx_handle;
            
            dma_handle->Instance                 = p_this_channel_def->rx_dma_channel;
            dma_handle->Init.Request             = p_this_channel_def->rx_dma_request_nbr;          
            dma_handle->Init.Direction           = DMA_PERIPH_TO_MEMORY;
            dma_handle->Init.PeriphInc           = DMA_PINC_DISABLE;
            dma_handle->Init.MemInc              = DMA_MINC_ENABLE;
            dma_handle->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
            dma_handle->Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
            dma_handle->Init.Mode                = DMA_CIRCULAR;
            dma_handle->Init.Priority            = DMA_PRIORITY_LOW;
            HAL_DMA_Init(dma_handle);  
            
            /* Associate the initialized DMA handle to the the UART handle */
            __HAL_LINKDMA(huart, hdmarx, p_this_channel_def->p_port_data->hdma_rx_handle);
         }
      }

      /* Configure the GPIO pins */
      SerialPortGpioInit(uIndex);

      /* Now enable the interrupts associated with this UART */
      SerialPortEnableInterrupts(huart);
   }
}

/**
  * @brief UART MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO, DMA and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
   if (NULL != huart)
   {
      /*##-3- Disable the DMA Streams ############################################*/
      /* De-Initialize the DMA Stream associate to transmission process */
      HAL_DMA_DeInit(huart->hdmatx); 
      /* De-Initialize the DMA Stream associate to reception process */
      HAL_DMA_DeInit(huart->hdmarx);
      
      SerialPortGpioDeInit(huart->Index);
   }
}

/*************************************************************************************/
/**
* function name   : SerialPort_CanGoToSleep
* description     : Checks if this module allows the system to enter the sleep mode
*
* @param - none
*
* @return - true if conditions are met to enter sleep mode
*/
bool SerialPort_CanGoToSleep(void)
{
   bool can_sleep = true;
   
   for(uint8_t uart_port = 0u; uart_port < NUMBER_OF_UARTS; uart_port++)
   {
      if (  (HAL_UART_STATE_BUSY_TX    == uart_port_data[uart_port].uart_handle.gState)
         || (HAL_UART_STATE_BUSY_TX_RX == uart_port_data[uart_port].uart_handle.gState))
      {
         /* This port is busy transmitting */
         can_sleep = false;
      }
   }
      
   return can_sleep;
}

/*************************************************************************************/
/**
* function name   : SerialPortEnterStopMode()
* description     : Reconfigures the uart rx pin as a gpio pin that can interrupt
*                   Stop Mode.
*
* @param - None
*
* @return -  true if stop mode is entered
*/
bool SerialPortEnterStopMode(uint8_t uart_port, wake_up_callback_function_t callback)
{
   bool result = false;
   Gpio_t rx_gpio;
   
   if ( callback )
   {
      /* Get this channel definition */
      const SerialChannelDefinition_t *const p_this_channel_def = (SerialChannelDefinition_t*)&SerialChannelDefinition[uart_port];
      UART_HandleTypeDef *const base_uart = &p_this_channel_def->p_port_data->uart_handle;

      if (HAL_UART_STATE_RESET != base_uart->gState)
      {
         /* Abort any on-going reception */
         HAL_UART_AbortReceive(base_uart);

         /* De-Init the UART port */
         HAL_UART_DeInit(base_uart);

         /* set up the interrupt for the RX pin */
         rx_gpio.pin = p_this_channel_def->rx_pin_name;
         rx_gpio.port = p_this_channel_def->p_gpio_port;
         rx_gpio.pinIndex = p_this_channel_def->rx_pin_id;
         GpioSetInterrupt(&rx_gpio, IRQ_FALLING_EDGE_E, IRQ_VERY_LOW_PRIORITY_E, callback, (uint32_t)PIN_PULL_UP_E );
         result = true;
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* function name   : SerialPortLeaveStopMode()
* description     : Reconfigures the uart port as a serial input
*
* @param - None
*
* @return -  none
*/
void SerialPortLeaveStopMode(uint8_t uart_port)
{
   /* Get this channel definition */
   const SerialChannelDefinition_t *const p_this_channel_def = (SerialChannelDefinition_t*)&SerialChannelDefinition[uart_port];
   UART_HandleTypeDef *const base_uart = &p_this_channel_def->p_port_data->uart_handle;

   /* remove the ISR of this port ex pin
    * Clear EXTI line configuration 
    */
   uint32_t temp = EXTI->IMR1;
   temp &= ~((uint32_t)p_this_channel_def->rx_pin_id);
   EXTI->IMR1 = temp;

   /* Re-enable the UART port */
   HAL_UART_Init(base_uart);
}

/*************************************************************************************/
/**
* function name   : SerialPort_IsTxInProgress
* description     : Checks if this port is transmitting
*
* @param - uart_port: port identifier
*
* @return - true if a transmission is in progress
*/
bool SerialPort_IsTxInProgress(uint8_t uart_port)
{
   bool transmit_in_progress = false;
   
   if(uart_port < NUMBER_OF_UARTS)
   {
      if (  (HAL_UART_STATE_BUSY_TX    == uart_port_data[uart_port].uart_handle.gState)
         || (HAL_UART_STATE_BUSY_TX_RX == uart_port_data[uart_port].uart_handle.gState))
      {
         /* This port is busy transmitting */
         transmit_in_progress = true;
      }
   }
      
   return transmit_in_progress;
}

/*************************************************************************************/
/**
* function name   : SerialPort_IsRxInProgress
* description     : Checks if an Rx operation is pending
*
* @param - uart_port: port identifier
*
* @return - true if a receiving is in progress
*/
bool SerialPort_IsRxInProgress(uint8_t uart_port)
{
   bool receive_in_progress = false;
   
   if(uart_port < NUMBER_OF_UARTS)
   {
      if (  (HAL_UART_STATE_BUSY_RX    == uart_port_data[uart_port].uart_handle.RxState)
         || (HAL_UART_STATE_BUSY_TX_RX == uart_port_data[uart_port].uart_handle.RxState))
      {
         /* This port is busy transmitting */
         receive_in_progress = true;
      }
   }
      
   return receive_in_progress;
}

/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle. 
  * @note   This example shows a simple way to report end of DMA Tx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
   SerialChannelDefinition_t *const p_this_channel_def = (SerialChannelDefinition_t*)&SerialChannelDefinition[huart->Index];

   buffer_run_data_t *const p_buffer_run_data = &p_this_channel_def->p_port_data->buffer_run_data;

   if (p_buffer_run_data->tx_buffer_read_ptr < p_buffer_run_data->tx_buffer_write_ptr )
   {
      const uint16_t tx_ptr = p_buffer_run_data->tx_buffer_read_ptr;
      const uint16_t temp_write_ptr = p_buffer_run_data->tx_buffer_write_ptr;
      const uint16_t size = temp_write_ptr - tx_ptr;

      if (HAL_OK == HAL_UART_Transmit_DMA(huart, &p_this_channel_def->tx_buffer_ptr[tx_ptr], size))
      {
         p_buffer_run_data->tx_buffer_read_ptr = temp_write_ptr;
      }
   }
   else if (p_buffer_run_data->tx_buffer_read_ptr > p_buffer_run_data->tx_buffer_write_ptr )
   {
      /* Handle the Wrap-around */
      const uint16_t tx_ptr = p_buffer_run_data->tx_buffer_read_ptr;
      const uint16_t size = p_this_channel_def->tx_buffer_size - tx_ptr;

      if (HAL_OK == HAL_UART_Transmit_DMA(huart, &p_this_channel_def->tx_buffer_ptr[tx_ptr], size))
      {
         /* Next time resume from the start of the buffer */
         p_buffer_run_data->tx_buffer_read_ptr = 0u;
      }
   }
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of DMA Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
   /*Do Nothing*/
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
   if ( UartHandle )
   {
      if (  (HAL_UART_STATE_BUSY_TX    == UartHandle->gState)
          ||(HAL_UART_STATE_BUSY_TX_RX == UartHandle->gState))
      {
         /* Service the pending Tx transaction */
         HAL_UART_TxCpltCallback(UartHandle);
      }

      /* Notify the Application of this error */
      MM_ConfigSerial_ErrorCallback(UartHandle->Index);
   }
}


/*************************************************************************************/
/**
* function name   : DEBUG_UART_IRQHandler
* description     : This function handles Debug UART Port global interrupt
*
* @param - void
*
* @return - void
*/
void DEBUG_UART_IRQHandler(void)
{
   /* Service the interrupt */
   HAL_UART_IRQHandler(&uart_port_data[DEBUG_UART_E].uart_handle);
}

/*************************************************************************************/
/**
* function name   : DEBUG_UART_DMA_Tx_IRQHandler
* description     : This function handles DMA interrupt of the Debug UART Tx channel
*
* @param - void
*
* @return - void
*/
void DEBUG_UART_DMA_Tx_IRQHandler(void)
{
   /* Service the interrupt */
   HAL_DMA_IRQHandler(uart_port_data[DEBUG_UART_E].uart_handle.hdmatx);
}

/*************************************************************************************/
/**
* function name   : DEBUG_UART_DMA_Rx_IRQHandler
* description     : This function handles DMA interrupt of the Debug UART Rx channel
*
* @param - void
*
* @return - void
*/
void DEBUG_UART_DMA_Rx_IRQHandler(void)
{
   /* Service the interrupt */
   HAL_DMA_IRQHandler(uart_port_data[DEBUG_UART_E].uart_handle.hdmarx);
}

/*************************************************************************************/
/**
* function name   : PPU_UART_IRQHandler
* description     : This function handles Debug PPU Port global interrupt
*
* @param - void
*
* @return - void
*/
void PPU_UART_IRQHandler(void)
{
   /* Service the interrupt */
   HAL_UART_IRQHandler(&uart_port_data[PPU_UART_E].uart_handle);
}


/*************************************************************************************/
/**
* function name   : PPU_UART_DMA_Tx_IRQHandler
* description     : This function handles DMA interrupt of the PPU UART Tx channel
*
* @param - void
*
* @return - void
*/
void PPU_UART_DMA_Tx_IRQHandler(void)
{
   /* Service the interrupt */
   HAL_DMA_IRQHandler(uart_port_data[PPU_UART_E].uart_handle.hdmatx);
}

/*************************************************************************************/
/**
* function name   : PPU_UART_DMA_Rx_IRQHandler
* description     : This function handles DMA interrupt of the PPU UART Rx channel
*
* @param - void
*
* @return - void
*/
void PPU_UART_DMA_Rx_IRQHandler(void)
{
   /* Service the interrupt */
   HAL_DMA_IRQHandler(uart_port_data[PPU_UART_E].uart_handle.hdmarx);
}

/*************************************************************************************/
/**
* function name   : HEAD_UART_IRQHandler
* description     : This function handles HEAD UART Port global interrupt
*
* @param - void
*
* @return - void
*/
void HEAD_UART_IRQHandler(void)
{
   /* Service the interrupt */
   HAL_UART_IRQHandler(&uart_port_data[HEAD_UART_E].uart_handle);
}

/*************************************************************************************/
/**
* function name   : HEAD_UART_DMA_Tx_IRQHandler
* description     : This function handles DMA interrupt of the HEAD UART Tx channel
*
* @param - void
*
* @return - void
*/
void HEAD_UART_DMA_Tx_IRQHandler(void)
{
   /* Service the interrupt */
   HAL_DMA_IRQHandler(uart_port_data[HEAD_UART_E].uart_handle.hdmatx);
}

/*************************************************************************************/
/**
* function name   : HEAD_UART_DMA_Rx_IRQHandler
* description     : This function handles DMA interrupt of the HEAD UART Rx channel
*
* @param - void
*
* @return - void
*/
void HEAD_UART_DMA_Rx_IRQHandler(void)
{
   /* Service the interrupt */
   HAL_DMA_IRQHandler(uart_port_data[HEAD_UART_E].uart_handle.hdmarx);
}

/*************************************************************************************/
/**
* function name   : CP_UART_IRQHandler
* description     : This function handles CP UART Port global interrupt
*
* @param - void
*
* @return - void
*/
void CP_UART_IRQHandler(void)
{
   /* Service the interrupt */
   HAL_UART_IRQHandler(&uart_port_data[NCU_CP_UART_E].uart_handle);
}

/*************************************************************************************/
/**
* function name   : CP_UART_DMA_Tx_IRQHandler
* description     : This function handles DMA interrupt of the CP UART Tx channel
*
* @param - void
*
* @return - void
*/
void CP_UART_DMA_Tx_IRQHandler(void)
{
   /* Service the interrupt */
   HAL_DMA_IRQHandler(uart_port_data[NCU_CP_UART_E].uart_handle.hdmatx);
}

/*************************************************************************************/
/**
* function name   : CP_UART_DMA_Rx_IRQHandler
* description     : This function handles DMA interrupt of the CP UART Rx channel
*
* @param - void
*
* @return - void
*/
void CP_UART_DMA_Rx_IRQHandler(void)
{
   /* Service the interrupt */
   HAL_DMA_IRQHandler(uart_port_data[NCU_CP_UART_E].uart_handle.hdmarx);
}

/*************************************************************************************/
/**
* function name   : SBC_UART_IRQHandler
* description     : This function handles SBC UART Port global interrupt
*
* @param - void
*
* @return - void
*/
void SBC_UART_IRQHandler(void)
{
   /* Service the interrupt */
   HAL_UART_IRQHandler(&uart_port_data[NCU_SBC_UART_E].uart_handle);
}

/*************************************************************************************/
/**
* function name   : SBC_UART_DMA_Tx_IRQHandler
* description     : This function handles DMA interrupt of the SBC UART Tx channel
*
* @param - void
*
* @return - void
*/
void SBC_UART_DMA_Tx_IRQHandler(void)
{
   /* Service the interrupt */
   HAL_DMA_IRQHandler(uart_port_data[NCU_SBC_UART_E].uart_handle.hdmatx);
}

/*************************************************************************************/
/**
* function name   : SBC_UART_DMA_Rx_IRQHandler
* description     : This function handles DMA interrupt of the SBC UART Rx channel
*
* @param - void
*
* @return - void
*/
void SBC_UART_DMA_Rx_IRQHandler(void)
{
   /* Service the interrupt */
   HAL_DMA_IRQHandler(uart_port_data[NCU_SBC_UART_E].uart_handle.hdmarx);
}
