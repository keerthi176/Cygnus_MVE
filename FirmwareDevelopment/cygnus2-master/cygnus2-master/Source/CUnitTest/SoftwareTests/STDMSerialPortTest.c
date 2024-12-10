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
*  File         : STDMSerialPortTest.c
*
*  Description  : Implementation for the Serial Port tests
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdarg.h>
#include <process.h>

/* User Include Files
*************************************************************************************/
#include "CUnit.h"
#include "STDMSerialPortTest.h"
#include "DM_SerialPort.h"



/* Private Functions Prototypes
*************************************************************************************/
static void FindATModeSequenceTest(void);
static void FindCRLFTest(void);
static void ReadRxBufferTest(void);
static void FullSequenceTest(void);

/* Global Variables
*************************************************************************************/
static uint32_t LastHandle = 0;
static bool bPrintToConsole = false; //Set this to 'true' in your test to have SerialDebugPrint write to the console.

/* Externs
*************************************************************************************/
extern const SerialChannelDefinition_t SerialChannelDefinition[MAX_SUPPORTED_SERIAL_PORT];

/* Table containing the test settings */
CU_TestInfo ST_SerialPortTests[] =
{
   { "FindATModeSequenceTest",               FindATModeSequenceTest },
   { "FindCRLFTest",                         FindCRLFTest },
   { "ReadRxBufferTest",                     ReadRxBufferTest },
   { "FullSequenceTest",                     FullSequenceTest },

   CU_TEST_INFO_NULL,
};



/* Private Variables
*************************************************************************************/

/* stubs for test
*************************************************************************************/





/*************************************************************************************
* TEST FUNCTIONS
*************************************************************************************/

/*************************************************************************************
* FindATModeSequenceTest
* Test the operation of function SerialPortFindATModeSequenceRxBuffer
*
* no params
*
* @return - none
*
*************************************************************************************/
void FindATModeSequenceTest(void)
{
   DMA_HandleTypeDef dummy_rx_handle;
   /* put the AT sequence into the rx buffer "+++<lf><cr>"*/
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[0] = '+';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[1] = '+';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[2] = '+';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[3] = 0x0d;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[4] = 0x0a;

   /* initialise the rx pointers */
   SerialChannelDefinition[NCU_CP_UART].p_port_data->buffer_run_data.rx_buffer_read_ptr = 0;
   SerialChannelDefinition[NCU_CP_UART].p_port_data->buffer_run_data.rx_buffer_write_ptr = 0;
   /*set the DMA count*/
   hal_dma_counter = SERIAL_PORT_3_RX_BUFFER_SIZE - 5;

   /*add a NULL DMA Rx handle*/
   SerialChannelDefinition[NCU_CP_UART].p_port_data->uart_handle.hdmarx = NULL;
   /*call the function under test.  It should return 0 because of the NULL DMA Rx Handle*/
   uint8_t number_of_chars = SerialPortFindATModeSequenceRxBuffer(NCU_CP_UART);
   CU_ASSERT_EQUAL(number_of_chars, 0);

   /*add a dummy DMA Rx handle*/
   SerialChannelDefinition[NCU_CP_UART].p_port_data->uart_handle.hdmarx = &dummy_rx_handle;
   /*call the function under test.  It should return a count of 5 chars */
   number_of_chars = SerialPortFindATModeSequenceRxBuffer(NCU_CP_UART);
   CU_ASSERT_EQUAL(number_of_chars, 5);

   /*Check that it doesn't hang for a dma count of 0 when no AT sequence is present*/
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[0] = 0;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[1] = 0;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[2] = 0;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[3] = 0;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[4] = 0;

   hal_dma_counter = 0;
   number_of_chars = SerialPortFindATModeSequenceRxBuffer(NCU_CP_UART);
   CU_ASSERT_EQUAL(number_of_chars, 0);

}


/*************************************************************************************
* FindCRLFTest
* Test the operation of function SerialPortFindCRLFRxBuffer
*
* no params
*
* @return - none
*
*************************************************************************************/
void FindCRLFTest(void)
{
   DMA_HandleTypeDef dummy_rx_handle;
   /* put the AT sequece into the rx buffer "+++<lf><cr>"*/
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[0] = '+';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[1] = '+';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[2] = '+';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[3] = 0x0d;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[4] = 0x0a;

   /* initialise the rx pointers */
   SerialChannelDefinition[NCU_CP_UART].p_port_data->buffer_run_data.rx_buffer_read_ptr = 0;
   SerialChannelDefinition[NCU_CP_UART].p_port_data->buffer_run_data.rx_buffer_write_ptr = 0;
   /*set the DMA count*/
   hal_dma_counter = SERIAL_PORT_3_RX_BUFFER_SIZE - 5;

   /*add a NULL DMA Rx handle*/
   SerialChannelDefinition[NCU_CP_UART].p_port_data->uart_handle.hdmarx = NULL;
   /*call the function under test.  It should return 0 because of the NULL DMA Rx Handle*/
   uint8_t number_of_chars = SerialPortFindCRLFRxBuffer(NCU_CP_UART);
   CU_ASSERT_EQUAL(number_of_chars, 0);

   /*add a dummy DMA Rx handle*/
   SerialChannelDefinition[NCU_CP_UART].p_port_data->uart_handle.hdmarx = &dummy_rx_handle;
   /*call the function under test.  It should return a count of 5 chars*/
   number_of_chars = SerialPortFindCRLFRxBuffer(NCU_CP_UART);
   CU_ASSERT_EQUAL(number_of_chars, 5);

   /*Check that it doesn't hang for a dma count of 0 when no <CR><LF> is present*/
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[0] = 0;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[1] = 0;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[2] = 0;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[3] = 0;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[4] = 0;

   hal_dma_counter = 0;
   number_of_chars = SerialPortFindCRLFRxBuffer(NCU_CP_UART);
   CU_ASSERT_EQUAL(number_of_chars, 0);

}

/*************************************************************************************
* ReadRxBufferTest
* Test the operation of function SerialPortReadRxBuffer
*
* no params
*
* @return - none
*
*************************************************************************************/
void ReadRxBufferTest(void)
{
   DMA_HandleTypeDef dummy_rx_handle;
   uint8_t buffer[SERIAL_PORT_3_RX_BUFFER_SIZE+1];
   uint32_t buffer_overrun = SERIAL_PORT_3_RX_BUFFER_SIZE + 1;

   /* put the disable RBU command (ATDISRB=1) into the rx buffer "+++<lf><cr>ATDISRB=1<lf><cr>"*/
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[0] = 'A';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[1] = 'T';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[2] = 'D';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[3] = 'I';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[4] = 'S';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[5] = 'R';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[6] = 'B';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[7] = '=';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[8] = '1';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[9] = 0x0d;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[10] = 0x0a;

   /* initialise the rx pointers */
   SerialChannelDefinition[NCU_CP_UART].p_port_data->buffer_run_data.rx_buffer_read_ptr = 0;
   SerialChannelDefinition[NCU_CP_UART].p_port_data->buffer_run_data.rx_buffer_write_ptr = 0;
   /*set the DMA count*/
   hal_dma_counter = SERIAL_PORT_3_RX_BUFFER_SIZE - 11;

   /*add a NULL DMA Rx handle*/
   SerialChannelDefinition[NCU_CP_UART].p_port_data->uart_handle.hdmarx = NULL;
   /*call the function under test.  It should return 0 because of the NULL DMA Rx Handle*/
   uint8_t number_of_chars = SerialPortReadRxBuffer(NCU_CP_UART, buffer, sizeof(buffer));
   CU_ASSERT_EQUAL(number_of_chars, 0);

   /*add a dummy DMA Rx handle*/
   SerialChannelDefinition[NCU_CP_UART].p_port_data->uart_handle.hdmarx = &dummy_rx_handle;
   /*call the function under test.  It should return 11 chars ATDISRB=6<CR><LF>*/
   number_of_chars = SerialPortReadRxBuffer(NCU_CP_UART, buffer, sizeof(buffer));
   CU_ASSERT_EQUAL(number_of_chars, 11);
   /* check the returned content*/
   int diff = memcmp(dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer, buffer, 11);
   CU_ASSERT_EQUAL(diff, 0);

   /*Check that it doesn't hang for a dma count of 0 when no data is present*/
   memset(dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer, 0, SERIAL_PORT_3_RX_BUFFER_SIZE);
   SerialChannelDefinition[NCU_CP_UART].p_port_data->buffer_run_data.rx_buffer_read_ptr = 0;
   hal_dma_counter = 0;
   number_of_chars = SerialPortReadRxBuffer(NCU_CP_UART, buffer, sizeof(buffer));
   CU_ASSERT_FALSE(number_of_chars == buffer_overrun);
   CU_ASSERT_EQUAL(number_of_chars, 0);
}

/*************************************************************************************
* ReadRxBufferTest
* Test the handling of sequece +++<cr><lf>+++<cr><lf>ATMSN?<cr><lf>
*
* no params
*
* @return - none
*
*************************************************************************************/
void FullSequenceTest(void)
{
   uint8_t buffer[SERIAL_PORT_3_RX_BUFFER_SIZE + 1];
   DMA_HandleTypeDef dummy_rx_handle;
   /* put the AT sequence into the rx buffer "+++<lf><cr>+++<lf><cr>ATMSN?<lf><cr>"*/
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[0] = '+';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[1] = '+';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[2] = '+';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[3] = 0x0d;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[4] = 0x0a;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[5] = '+';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[6] = '+';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[7] = '+';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[8] = 0x0d;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[9] = 0x0a;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[10] = 'A';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[11] = 'T';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[12] = 'M';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[13] = 'S';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[14] = 'N';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[15] = '?';
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[16] = 0x0d;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[17] = 0x0a;
   dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[18] = 0x00;

   /* initialise the rx pointers */
   SerialChannelDefinition[NCU_CP_UART].p_port_data->buffer_run_data.rx_buffer_read_ptr = 0;
   SerialChannelDefinition[NCU_CP_UART].p_port_data->buffer_run_data.rx_buffer_write_ptr = 0;
   /*set the DMA count*/
   hal_dma_counter = SERIAL_PORT_3_RX_BUFFER_SIZE - 5;

   /*add a NULL DMA Rx handle*/
   SerialChannelDefinition[NCU_CP_UART].p_port_data->uart_handle.hdmarx = NULL;
   /*call the function under test.  It should return 0 because of the NULL DMA Rx Handle*/
   uint8_t number_of_chars = SerialPortFindATModeSequenceRxBuffer(NCU_CP_UART);
   CU_ASSERT_EQUAL(number_of_chars, 0);

   /*add a dummy DMA Rx handle*/
   SerialChannelDefinition[NCU_CP_UART].p_port_data->uart_handle.hdmarx = &dummy_rx_handle;
   /*call the function under test.  It should return a count of 5 chars */
   number_of_chars = SerialPortFindATModeSequenceRxBuffer(NCU_CP_UART);
   CU_ASSERT_EQUAL(number_of_chars, 5);

   /* read the +++ */
   number_of_chars = SerialPortReadRxBuffer(NCU_CP_UART, buffer, sizeof(buffer));
   CU_ASSERT_EQUAL(number_of_chars, 5);
   int diff = memcmp((char*)&dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[0], buffer, 5);
   CU_ASSERT_EQUAL(diff, 0);

   /*check the read pointer is 5*/
   CU_ASSERT_EQUAL(SerialChannelDefinition[NCU_CP_UART].p_port_data->buffer_run_data.rx_buffer_read_ptr, 5);

   hal_dma_counter = SERIAL_PORT_3_RX_BUFFER_SIZE - 10;

   /* repeat the read */
   number_of_chars = SerialPortFindATModeSequenceRxBuffer(NCU_CP_UART);
   CU_ASSERT_EQUAL(number_of_chars, 5);

   /* read the second +++ */
   number_of_chars = SerialPortReadRxBuffer(NCU_CP_UART, buffer, sizeof(buffer));
   CU_ASSERT_EQUAL(number_of_chars, 5);
   diff = memcmp((char*)&dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[5], buffer, 5);
   CU_ASSERT_EQUAL(diff, 0);

   /*check the read pointer is 10*/
   CU_ASSERT_EQUAL(SerialChannelDefinition[NCU_CP_UART].p_port_data->buffer_run_data.rx_buffer_read_ptr, 10);

   /* Read the AT command */
   hal_dma_counter = SERIAL_PORT_3_RX_BUFFER_SIZE - 18;

   number_of_chars = SerialPortReadRxBuffer(NCU_CP_UART, buffer, sizeof(buffer));
   CU_ASSERT_EQUAL(number_of_chars, 8);
   /* check the returned content*/
   diff = memcmp((char*)&dm_serial_data_buffer.individual_buffers.serial_port_3_rx_buffer[10], buffer, 8);
   CU_ASSERT_EQUAL(diff, 0);

}


