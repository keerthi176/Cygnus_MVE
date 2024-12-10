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
*  File         : STI2CTest.c
*
*  Description  : Implementation file for I2C driver tests
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>



/* User Include Files
*************************************************************************************/
#include "CUnit.h"
#include "CO_Defines.h"
#include "CO_ErrorCode.h"
#include "MessageQueueStub.h"
#include "stm32l4xx.h"
#include "DM_i2c.h"
#include "STI2CTest.h"


#define I2C_TEST_RX_BUFFER_SIZE 4U

/* Private Functions Prototypes
*************************************************************************************/
static void DM_I2C_InitialiseTest(void);
static void HAL_I2C_MspInitTest(void);
static void HAL_I2C_MspDeInitTest(void);
static void DM_I2C_ReadTest(void);
static void DM_I2C_WriteTest(void);
static void DM_I2C_HandleErrorTest(void);


/* Global Variables
*************************************************************************************/

/* Table containing the test settings */
CU_TestInfo ST_I2CTests[] =
{
   { "DM_I2C_Initialise",                  DM_I2C_InitialiseTest },
   { "HAL_I2C_MspInit",                    HAL_I2C_MspInitTest },
   { "HAL_I2C_MspDeInit",                  HAL_I2C_MspDeInitTest },
   { "DM_I2C_Read",                        DM_I2C_ReadTest },
   { "DM_I2C_Write",                       DM_I2C_WriteTest },
   { "DM_I2C_HandleError",                 DM_I2C_HandleErrorTest },

   CU_TEST_INFO_NULL,
};

extern void DM_I2C_HandleError(uint32_t hal_error);

/* Test Stubs
*************************************************************************************/

I2C_TypeDef I2C1_concrete;
I2C_TypeDef* I2C1 = &I2C1_concrete;
I2C_TypeDef I2C2_concrete;
I2C_TypeDef* I2C2 = &I2C2_concrete;

GPIO_TypeDef GPIOB_concrete;
GPIO_TypeDef* GPIOB = &GPIOB_concrete;

bool HAL_GPIO_Init_Do_Tests = false;
uint32_t HAL_GPIO_Init_CallCount = 0;
void HAL_GPIO_Init(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init)
{
   HAL_GPIO_Init_CallCount++;
   if (GPIOx && GPIO_Init)
   {
      if (HAL_GPIO_Init_Do_Tests)
      {
         CU_ASSERT_EQUAL(GPIOx, GPIOB);
         CU_ASSERT_EQUAL(GPIO_Init->Mode, GPIO_MODE_AF_OD);
         CU_ASSERT_EQUAL(GPIO_Init->Pull, GPIO_PULLUP);
         CU_ASSERT_EQUAL(GPIO_Init->Speed, GPIO_SPEED_FREQ_VERY_HIGH);


         if (GPIO_Init->Pin == GPIO_PIN_10)
         {
            CU_ASSERT_EQUAL(GPIO_Init->Alternate, GPIO_AF4_I2C1);
         }
         else if (GPIO_Init->Pin == GPIO_PIN_11)
         {
            CU_ASSERT_EQUAL(GPIO_Init->Alternate, GPIO_AF4_I2C1);
         }
         else
         {
            CU_FAIL("GPIO_Init->Pin is incorrect");
         }
      }
   }
   else
   {
      CU_FAIL("HAL_GPIO_Init received NULL parameter");
   }
}

bool HAL_GPIO_DeInit_Do_Tests = false;
uint32_t HAL_GPIO_DeInit_CallCount = 0;
void HAL_GPIO_DeInit(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin)
{
   HAL_GPIO_DeInit_CallCount++;
   if (GPIOx)
   {
      if (HAL_GPIO_DeInit_Do_Tests)
      {
         CU_ASSERT_EQUAL(GPIOx, GPIOB);
         CU_ASSERT_EQUAL(GPIO_Pin, (GPIO_PIN_10 | GPIO_PIN_11));
      }
   }
}


HAL_StatusTypeDef HAL_I2C_Init_Response = HAL_ERROR;
uint32_t HAL_I2C_Init_CallCount = 0;
I2C_HandleTypeDef* HAL_I2C_Init_hi2c = NULL;
HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c)
{
   HAL_I2C_Init_CallCount++;
   HAL_I2C_Init_hi2c = hi2c;
   return HAL_I2C_Init_Response;
}


HAL_StatusTypeDef HAL_I2CEx_ConfigAnalogFilter_Response = HAL_ERROR;
uint32_t HAL_I2CEx_ConfigAnalogFilter_CallCount = 0;
I2C_HandleTypeDef* HAL_I2CEx_ConfigAnalogFilter_hi2c = NULL;
uint32_t HAL_I2CEx_ConfigAnalogFilter_AnalogFilter = 0;
HAL_StatusTypeDef HAL_I2CEx_ConfigAnalogFilter(I2C_HandleTypeDef *hi2c, uint32_t AnalogFilter)
{
   HAL_I2CEx_ConfigAnalogFilter_CallCount++;
   HAL_I2CEx_ConfigAnalogFilter_hi2c = hi2c;
   HAL_I2CEx_ConfigAnalogFilter_AnalogFilter = AnalogFilter;
   return HAL_I2CEx_ConfigAnalogFilter_Response;
}

HAL_StatusTypeDef HAL_I2CEx_ConfigDigitalFilter_Response = HAL_ERROR;
uint32_t HAL_I2CEx_ConfigDigitalFilter_CallCount = 0;
I2C_HandleTypeDef* HAL_I2CEx_ConfigDigitalFilter_hi2c = NULL;
uint32_t HAL_I2CEx_ConfigDigitalFilter_DigitalFilter = 0;
HAL_StatusTypeDef HAL_I2CEx_ConfigDigitalFilter(I2C_HandleTypeDef *hi2c, uint32_t DigitalFilter)
{
   HAL_I2CEx_ConfigDigitalFilter_CallCount++;
   HAL_I2CEx_ConfigDigitalFilter_hi2c = hi2c;
   HAL_I2CEx_ConfigDigitalFilter_DigitalFilter = DigitalFilter;
   return HAL_I2CEx_ConfigDigitalFilter_Response;
}

uint8_t HAL_I2C_Mem_Data[I2C_TEST_RX_BUFFER_SIZE] = { 0x01,0x02,0x03,0x04 };
HAL_StatusTypeDef HAL_I2C_Mem_Read_Response = HAL_ERROR;
uint32_t HAL_I2C_Mem_Read_CallCount = 0;

HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
   HAL_I2C_Mem_Read_CallCount++;
   if (pData)
   {
      size_t copy_length = Size;
      if (copy_length > I2C_TEST_RX_BUFFER_SIZE)
      {
         copy_length = I2C_TEST_RX_BUFFER_SIZE;
      }
      memcpy(pData, HAL_I2C_Mem_Data, copy_length);
   }
   return HAL_I2C_Mem_Read_Response;
}

HAL_StatusTypeDef HAL_I2C_Mem_Write_Response = HAL_ERROR;
uint32_t HAL_I2C_Mem_Write_CallCount = 0;
uint8_t HAL_I2C_Mem_Write_Data[I2C_TEST_RX_BUFFER_SIZE];
HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
   HAL_I2C_Mem_Write_CallCount++;
   if (pData)
   {
      size_t copy_length = Size;
      if (copy_length > I2C_TEST_RX_BUFFER_SIZE)
      {
         copy_length = I2C_TEST_RX_BUFFER_SIZE;
      }
      memcpy(HAL_I2C_Mem_Write_Data, pData, copy_length);
   }
   return HAL_I2C_Mem_Write_Response;
}

uint32_t HAL_I2C_GetError_Response = HAL_I2C_ERROR_TIMEOUT;
uint32_t HAL_I2C_GetError(I2C_HandleTypeDef *hi2c)
{
   return HAL_I2C_GetError_Response;
}


/* Private Variables
*************************************************************************************/


/*************************************************************************************/
/**
* DM_I2C_InitialiseTest
* Test the function DM_I2C_Initialise
*
* no params
*
* @return - none

*/
static void DM_I2C_InitialiseTest(void)
{
   ErrorCode_t result;
   const uint32_t own_address = 1;
   I2C_HandleTypeDef* hi2c1 = GetI2CHandle();
   memset(hi2c1, 0, sizeof(I2C_HandleTypeDef));

   //Set up stub function parameters
   HAL_I2C_Init_Response = HAL_ERROR;
   HAL_I2C_Init_CallCount = 0;
   HAL_I2C_Init_hi2c = NULL;
   HAL_I2CEx_ConfigAnalogFilter_Response = HAL_OK;
   HAL_I2CEx_ConfigAnalogFilter_CallCount = 0;
   HAL_I2CEx_ConfigDigitalFilter_Response = HAL_OK;
   HAL_I2CEx_ConfigDigitalFilter_CallCount = 0;

   //Check when call to HAL_I2C_Init fails
   result = DM_I2C_Initialise(own_address);
   CU_ASSERT_EQUAL(result, ERR_INIT_FAIL_E);
   CU_ASSERT_EQUAL(HAL_I2C_Init_CallCount, 1);
   CU_ASSERT_EQUAL(HAL_I2CEx_ConfigAnalogFilter_CallCount, 0);
   CU_ASSERT_EQUAL(HAL_I2CEx_ConfigDigitalFilter_CallCount, 0);

   //Check when call to HAL_I2CEx_ConfigAnalogFilter fails
   HAL_I2C_Init_CallCount = 0;
   HAL_I2C_Init_Response = HAL_OK;
   HAL_I2CEx_ConfigAnalogFilter_CallCount = 0;
   HAL_I2CEx_ConfigAnalogFilter_Response = HAL_ERROR;
   result = DM_I2C_Initialise(own_address);
   CU_ASSERT_EQUAL(result, ERR_INIT_FAIL_E);
   CU_ASSERT_EQUAL(HAL_I2C_Init_CallCount, 1);
   CU_ASSERT_EQUAL(HAL_I2CEx_ConfigAnalogFilter_CallCount, 1);
   CU_ASSERT_EQUAL(HAL_I2CEx_ConfigDigitalFilter_CallCount, 0);

   //Check when call to HAL_I2CEx_ConfigDigitalFilter fails
   HAL_I2C_Init_CallCount = 0;
   HAL_I2C_Init_Response = HAL_OK;
   HAL_I2CEx_ConfigAnalogFilter_CallCount = 0;
   HAL_I2CEx_ConfigAnalogFilter_Response = HAL_OK;
   HAL_I2CEx_ConfigDigitalFilter_CallCount = 0;
   HAL_I2CEx_ConfigDigitalFilter_Response = HAL_ERROR;
   result = DM_I2C_Initialise(own_address);
   CU_ASSERT_EQUAL(result, ERR_INIT_FAIL_E);
   CU_ASSERT_EQUAL(HAL_I2C_Init_CallCount, 1);
   CU_ASSERT_EQUAL(HAL_I2CEx_ConfigAnalogFilter_CallCount, 1);
   CU_ASSERT_EQUAL(HAL_I2CEx_ConfigDigitalFilter_CallCount, 1);

   //Test successful call

   //Set up the stub functions
   memset(hi2c1, 0, sizeof(I2C_HandleTypeDef));
   HAL_I2C_Init_CallCount = 0;
   HAL_I2C_Init_Response = HAL_OK;
   HAL_I2C_Init_hi2c = NULL;
   HAL_I2CEx_ConfigAnalogFilter_CallCount = 0;
   HAL_I2CEx_ConfigAnalogFilter_Response = HAL_OK;
   HAL_I2CEx_ConfigAnalogFilter_hi2c = NULL;
   HAL_I2CEx_ConfigAnalogFilter_AnalogFilter = 0;
   HAL_I2CEx_ConfigDigitalFilter_CallCount = 0;
   HAL_I2CEx_ConfigDigitalFilter_Response = HAL_OK;
   HAL_I2CEx_ConfigDigitalFilter_hi2c = NULL;
   HAL_I2CEx_ConfigDigitalFilter_DigitalFilter = 0;

   result = DM_I2C_Initialise(own_address);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(HAL_I2C_Init_CallCount, 1);
   CU_ASSERT_EQUAL(HAL_I2CEx_ConfigAnalogFilter_CallCount, 1);
   CU_ASSERT_EQUAL(HAL_I2CEx_ConfigDigitalFilter_CallCount, 1);
   CU_ASSERT_EQUAL(HAL_I2C_Init_hi2c, hi2c1);
   CU_ASSERT_EQUAL(HAL_I2CEx_ConfigAnalogFilter_hi2c, hi2c1);
   CU_ASSERT_EQUAL(HAL_I2CEx_ConfigDigitalFilter_hi2c, hi2c1);
   CU_ASSERT_EQUAL(HAL_I2CEx_ConfigAnalogFilter_AnalogFilter, I2C_ANALOGFILTER_ENABLE);
   CU_ASSERT_EQUAL(HAL_I2CEx_ConfigDigitalFilter_DigitalFilter, 0);

   //Check that hi2c1 was set up as expected
   CU_ASSERT_EQUAL(hi2c1->Instance, I2C1);
   CU_ASSERT_EQUAL(hi2c1->Init.Timing, I2C_TIMING_REGISTER_VALUE);
   CU_ASSERT_EQUAL(hi2c1->Init.AddressingMode, I2C_ADDRESSINGMODE_7BIT);
   CU_ASSERT_EQUAL(hi2c1->Init.DualAddressMode, I2C_DUALADDRESS_DISABLE);
   CU_ASSERT_EQUAL(hi2c1->Init.OwnAddress2, 0);
   CU_ASSERT_EQUAL(hi2c1->Init.OwnAddress2Masks, I2C_OA2_NOMASK);
   CU_ASSERT_EQUAL(hi2c1->Init.GeneralCallMode, I2C_GENERALCALL_ENABLE);
   CU_ASSERT_EQUAL(hi2c1->Init.NoStretchMode, I2C_NOSTRETCH_DISABLE);
   CU_ASSERT_EQUAL(hi2c1->Init.OwnAddress1, own_address);



}


/*************************************************************************************/
/**
* HAL_I2C_MspInitTest
* Test our override of the weak function HAL_I2C_MspInit
*
* no params
*
* @return - none

*/
void HAL_I2C_MspInitTest(void)
{
   ErrorCode_t result;
   const uint32_t own_address = 1;
   I2C_HandleTypeDef* hi2c1 = GetI2CHandle();
   I2C_HandleTypeDef hi2c2;

   /* initialise the I2C */
   memset(hi2c1, 0, sizeof(I2C_HandleTypeDef));
   result = DM_I2C_Initialise(own_address);
   CU_ASSERT_EQUAL(result, SUCCESS_E);

   hi2c2.Instance = I2C2;
   //Do a NULL parameter test
   HAL_GPIO_Init_CallCount = 0;
   HAL_GPIO_Init_Do_Tests = false;
   HAL_I2C_MspInit(NULL);
   CU_ASSERT_EQUAL(HAL_GPIO_Init_CallCount, 0);

   //Check response to wrong instance
   HAL_I2C_MspInit(&hi2c2);
   CU_ASSERT_EQUAL(HAL_GPIO_Init_CallCount, 0);

   //Check the successful call
   HAL_GPIO_Init_Do_Tests = true;
   HAL_GPIO_Init_CallCount = 0;
   /* HAL_I2C_MspInit doesn't produce any directly observable result except that it calls the stub function HAL_GPIO_Init (twice).
      There are more tests for this in the stub function HAL_GPIO_Init at the top of this file */
   HAL_I2C_MspInit(hi2c1);
   CU_ASSERT_EQUAL(HAL_GPIO_Init_CallCount, 2);

   // Switch off the tests in HAL_GPIO_Init. We don't want this to affect future tests.
   HAL_GPIO_Init_Do_Tests = false;
}

/*************************************************************************************/
/**
* HAL_I2C_MspDeInitTest
* Test our override of the weak function HAL_I2C_MspDeInit
*
* no params
*
* @return - none

*/
void HAL_I2C_MspDeInitTest(void)
{
   ErrorCode_t result;
   const uint32_t own_address = 1;
   I2C_HandleTypeDef* hi2c1 = GetI2CHandle();
   I2C_HandleTypeDef hi2c2;

   /* initialise the I2C */
   memset(hi2c1, 0, sizeof(I2C_HandleTypeDef));
   result = DM_I2C_Initialise(own_address);
   CU_ASSERT_EQUAL(result, SUCCESS_E);

   hi2c2.Instance = I2C2;
   //Do a NULL parameter test
   HAL_GPIO_DeInit_CallCount = 0;
   HAL_GPIO_DeInit_Do_Tests = false;
   HAL_I2C_MspDeInit(NULL);
   CU_ASSERT_EQUAL(HAL_GPIO_DeInit_CallCount, 0);

   //Check response to wrong instance
   HAL_I2C_MspDeInit(&hi2c2);
   CU_ASSERT_EQUAL(HAL_GPIO_DeInit_CallCount, 0);

   //Check the successful call
   HAL_GPIO_DeInit_Do_Tests = true;
   HAL_GPIO_DeInit_CallCount = 0;
   /* HAL_I2C_DeMspInit doesn't produce any directly observable result except that it calls the stub function HAL_GPIO_DeInit.
   There are more tests for this in the stub function HAL_GPIO_DeInit at the top of this file */
   HAL_I2C_MspDeInit(hi2c1);
   CU_ASSERT_EQUAL(HAL_GPIO_DeInit_CallCount, 1);

   // Switch off the tests in HAL_GPIO_Init. We don't want this to affect future tests.
   HAL_GPIO_DeInit_Do_Tests = false;
}

/*************************************************************************************/
/**
* DM_I2C_ReadTest
* Test function DM_I2C_Read.
*
* no params
*
* @return - none

*/
void DM_I2C_ReadTest(void)
{
   ErrorCode_t result;
   const uint32_t own_address = 0x12;
   const uint32_t slave_address = 0x10;
   const uint32_t register_address = 0x40;
   uint16_t data_length = 1;
   I2C_HandleTypeDef* hi2c1 = GetI2CHandle();
   uint8_t rx_buffer[I2C_TEST_RX_BUFFER_SIZE];


   /* initialise the I2C */
   memset(hi2c1, 0, sizeof(I2C_HandleTypeDef));
   result = DM_I2C_Initialise(own_address);
   CU_ASSERT_EQUAL(result, SUCCESS_E);

   //Test with NULL rx_buffer
   HAL_I2C_Mem_Read_CallCount = 0;
   result = DM_I2C_Read(slave_address, register_address, data_length, NULL, I2C_TEST_RX_BUFFER_SIZE);
   CU_ASSERT_EQUAL(result, ERR_INVALID_POINTER_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Read_CallCount, 0);

   //Test with rx_buffer too small

   //Test for HAL Read function returning error
   HAL_I2C_Mem_Read_Response = HAL_ERROR;
   HAL_I2C_Mem_Read_CallCount = 0;
   data_length = I2C_TEST_RX_BUFFER_SIZE;
   result = DM_I2C_Read(slave_address, register_address, data_length, rx_buffer, I2C_TEST_RX_BUFFER_SIZE);
   CU_ASSERT_EQUAL(result, ERR_MESSAGE_FAIL_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Read_CallCount, 1);

   //Test for normal operation for single byte
   HAL_I2C_Mem_Read_Response = HAL_OK;
   HAL_I2C_Mem_Read_CallCount = 0;
   data_length = 1;
   memset(rx_buffer, 0xFF, I2C_TEST_RX_BUFFER_SIZE);
   result = DM_I2C_Read(slave_address, register_address, data_length, rx_buffer, I2C_TEST_RX_BUFFER_SIZE);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Read_CallCount, 1);
   CU_ASSERT_EQUAL(rx_buffer[0], HAL_I2C_Mem_Data[0]);

   //Test for normal operation for multiple bytes
   HAL_I2C_Mem_Read_Response = HAL_OK;
   HAL_I2C_Mem_Read_CallCount = 0;
   data_length = I2C_TEST_RX_BUFFER_SIZE;
   memset(rx_buffer, 0xFF, I2C_TEST_RX_BUFFER_SIZE);
   result = DM_I2C_Read(slave_address, register_address, data_length, rx_buffer, I2C_TEST_RX_BUFFER_SIZE);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Read_CallCount, 1);
   uint32_t diff = memcmp(rx_buffer, HAL_I2C_Mem_Data, I2C_TEST_RX_BUFFER_SIZE);
   CU_ASSERT_EQUAL(diff, 0);
}

/*************************************************************************************/
/**
* DM_I2C_WriteTest
* Test function DM_I2C_Write.
*
* no params
*
* @return - none

*/
void DM_I2C_WriteTest(void)
{
   ErrorCode_t result;
   const uint32_t own_address = 0x12;
   const uint32_t slave_address = 0x10;
   const uint32_t register_address = 0x40;
   uint16_t data_length = 1;
   I2C_HandleTypeDef* hi2c1 = GetI2CHandle();
   uint8_t tx_buffer[I2C_TEST_RX_BUFFER_SIZE];


   /* initialise the I2C */
   memset(hi2c1, 0, sizeof(I2C_HandleTypeDef));
   result = DM_I2C_Initialise(own_address);
   CU_ASSERT_EQUAL(result, SUCCESS_E);

   //Test with NULL data_buffer
   HAL_I2C_Mem_Write_CallCount = 0;
   result = DM_I2C_Write(slave_address, register_address, data_length, NULL);
   CU_ASSERT_EQUAL(result, ERR_INVALID_POINTER_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 0);

   //Test for HAL Write function returning error
   HAL_I2C_Mem_Write_Response = HAL_ERROR;
   HAL_I2C_Mem_Write_CallCount = 0;
   data_length = 1;
   result = DM_I2C_Write(slave_address, register_address, data_length, tx_buffer);
   CU_ASSERT_EQUAL(result, ERR_MESSAGE_FAIL_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 1);

   //Test for normal operation for single byte
   HAL_I2C_Mem_Write_Response = HAL_OK;
   HAL_I2C_Mem_Write_CallCount = 0;
   data_length = 1;
   memset(HAL_I2C_Mem_Write_Data, 0xFF, I2C_TEST_RX_BUFFER_SIZE);
   result = DM_I2C_Write(slave_address, register_address, data_length, tx_buffer);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 1);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_Data[0], tx_buffer[0]);

   //Test for normal operation multiple bytes
   HAL_I2C_Mem_Write_Response = HAL_OK;
   HAL_I2C_Mem_Write_CallCount = 0;
   data_length = I2C_TEST_RX_BUFFER_SIZE;
   memcpy(tx_buffer, HAL_I2C_Mem_Data, I2C_TEST_RX_BUFFER_SIZE);
   memset(HAL_I2C_Mem_Write_Data, 0xFF, I2C_TEST_RX_BUFFER_SIZE);
   result = DM_I2C_Write(slave_address, register_address, data_length, tx_buffer);
   CU_ASSERT_EQUAL(result, SUCCESS_E);
   CU_ASSERT_EQUAL(HAL_I2C_Mem_Write_CallCount, 1);
   uint32_t diff = memcmp(tx_buffer, HAL_I2C_Mem_Write_Data, I2C_TEST_RX_BUFFER_SIZE);
   CU_ASSERT_EQUAL(diff, 0);
}

/*************************************************************************************/
/**
* DM_I2C_HandleErrorTest
* Test function DM_I2C_HandleError.
*
* no params
*
* @return - none

*/
void DM_I2C_HandleErrorTest(void)
{
   uint32_t diff;
   bSerialDebugPrintCalled = false;
   SerialDebugPrintCount = 0;
   //char SerialDebugLastPrint[256];

   uint32_t error_number = HAL_I2C_ERROR_AF;
   DM_I2C_HandleError(error_number);
   CU_ASSERT_TRUE(bSerialDebugPrintCalled);
   CU_ASSERT_EQUAL(SerialDebugPrintCount, 1);
   diff = strcmp(SerialDebugLastPrint, "+ERR:I2C ACK\r\n");
   CU_ASSERT_EQUAL(diff, 0);

   error_number = HAL_I2C_ERROR_BERR;
   SerialDebugPrintCount = 0;
   DM_I2C_HandleError(error_number);
   CU_ASSERT_TRUE(bSerialDebugPrintCalled);
   CU_ASSERT_EQUAL(SerialDebugPrintCount, 1);
   diff = strcmp(SerialDebugLastPrint, "+ERR:I2C BERR\r\n");
   CU_ASSERT_EQUAL(diff, 0);

   error_number = HAL_I2C_ERROR_ARLO;
   SerialDebugPrintCount = 0;
   DM_I2C_HandleError(error_number);
   CU_ASSERT_TRUE(bSerialDebugPrintCalled);
   CU_ASSERT_EQUAL(SerialDebugPrintCount, 1);
   diff = strcmp(SerialDebugLastPrint, "+ERR:I2C ARLO\r\n");
   CU_ASSERT_EQUAL(diff, 0);

   error_number = HAL_I2C_ERROR_OVR;
   SerialDebugPrintCount = 0;
   DM_I2C_HandleError(error_number);
   CU_ASSERT_TRUE(bSerialDebugPrintCalled);
   CU_ASSERT_EQUAL(SerialDebugPrintCount, 1);
   diff = strcmp(SerialDebugLastPrint, "+ERR:I2C OVR\r\n");
   CU_ASSERT_EQUAL(diff, 0);

   error_number = HAL_I2C_ERROR_DMA;
   SerialDebugPrintCount = 0;
   DM_I2C_HandleError(error_number);
   CU_ASSERT_TRUE(bSerialDebugPrintCalled);
   CU_ASSERT_EQUAL(SerialDebugPrintCount, 1);
   diff = strcmp(SerialDebugLastPrint, "+ERR:I2C DMA\r\n");
   CU_ASSERT_EQUAL(diff, 0);

   error_number = HAL_I2C_ERROR_TIMEOUT;
   SerialDebugPrintCount = 0;
   DM_I2C_HandleError(error_number);
   CU_ASSERT_TRUE(bSerialDebugPrintCalled);
   CU_ASSERT_EQUAL(SerialDebugPrintCount, 1);
   diff = strcmp(SerialDebugLastPrint, "+ERR:I2C TIMEOUT\r\n");
   CU_ASSERT_EQUAL(diff, 0);

   error_number = HAL_I2C_ERROR_SIZE;
   SerialDebugPrintCount = 0;
   DM_I2C_HandleError(error_number);
   CU_ASSERT_TRUE(bSerialDebugPrintCalled);
   CU_ASSERT_EQUAL(SerialDebugPrintCount, 1);
   diff = strcmp(SerialDebugLastPrint, "+ERR:I2C SIZE MGMT\r\n");
   CU_ASSERT_EQUAL(diff, 0);

   error_number = HAL_I2C_ERROR_NONE;
   SerialDebugPrintCount = 0;
   DM_I2C_HandleError(error_number);
   CU_ASSERT_TRUE(bSerialDebugPrintCalled);
   CU_ASSERT_EQUAL(SerialDebugPrintCount, 1);
   diff = strcmp(SerialDebugLastPrint, "+ERR:I2C NONE\r\n");
   CU_ASSERT_EQUAL(diff, 0);

   error_number = 3;// invalid value
   SerialDebugPrintCount = 0;
   DM_I2C_HandleError(error_number);
   CU_ASSERT_TRUE(bSerialDebugPrintCalled);
   CU_ASSERT_EQUAL(SerialDebugPrintCount, 1);
   diff = strcmp(SerialDebugLastPrint, "+ERR:I2C UNKNOWN = 3\r\n");
   CU_ASSERT_EQUAL(diff, 0);

}
