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
*  File         : DM_CRC.c
*
*  Description  : Device Manager for the CRC module
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include "cmsis_os.h"


/* User Include Files
*************************************************************************************/
#include "stm32l4xx.h"
#include "stm32l4xx_ll_crc.h"
#include "CO_Defines.h"
#include "DM_CRC.h"

/* Private Functions Prototypes
*************************************************************************************/



/* Global Variables
*************************************************************************************/



/* Private Variables
*************************************************************************************/

/* CRC handler declaration */
static CRC_HandleTypeDef crc_handle_8b_data_16b_crc =
{
   .Instance = CRC,

   .Init =
   {
      .DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_DISABLE,
      .GeneratingPolynomial    = 0x8005U,
      .CRCLength               = CRC_POLYLENGTH_16B,
      
      /* The zero init value is used */
      .DefaultInitValueUse     = DEFAULT_INIT_VALUE_DISABLE,
      .InitValue               = 0xFFFFU,
      
      /* The input data are not inverted */
      .InputDataInversionMode  = CRC_INPUTDATA_INVERSION_NONE,
      
      /* The output data are not inverted */
      .OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE,
   },

   /* The input data are 8-bit long */
   .InputDataFormat              = CRC_INPUTDATA_FORMAT_BYTES,
};

/* Mutex to garantee exclusive access */
osMutexId(CRCMutexId) = NULL;
osMutexDef(CRCMutex);

/*************************************************************************************/
/**
* DM_CrcInit
* Initialisation function of the CRC module
*
* @param - void
*
* @return - ErrorCode_t - status 0=success else error
*/
ErrorCode_t DM_CrcInit(void)
{
   ErrorCode_t status = ERR_INIT_FAIL_E;

   /* Enable the CRC clock */
   __HAL_RCC_CRC_CLK_ENABLE();

   /* Configure the CRC module */
   if (HAL_OK == HAL_CRC_Init(&crc_handle_8b_data_16b_crc))
   {
      /* Create the Mutex if not done yet */
      if(NULL == CRCMutexId)
      {
         /* Create the Mutex */
         CRCMutexId = osMutexCreate(osMutex(CRCMutex));
         if(CRCMutexId)
         {
            status = SUCCESS_E;
         }
         else
         {
            Error_Handler("Failed to create CRC mutex");
         }
      }
   }

   return status;
}

/*************************************************************************************/
/**
* DM_Crc16bCalculate8bDataWidth
* Calculate CRC value on a block of 8b data, the CRC value is coded as 16 bit
*
* @param - pBuffer: Pointer to the data buffer
* @param - length: length of the data buffer in bytes
* @param - crc_value: pointer to where to store the calculated CRC
* @param - timeout: timeout in Ms to get the ownership of CRC engine
*
* @return - ErrorCode_t - status 0=success else error
*/
ErrorCode_t DM_Crc16bCalculate8bDataWidth(uint8_t *pBuffer, uint32_t length, uint16_t *crc_value, uint32_t timeout_ms)
{
   ErrorCode_t status = SUCCESS_E;

   /* Get Mutex, Wait maximum of 1ms */
   if (osOK == osMutexWait(CRCMutexId, timeout_ms)) 
   {
      __HAL_RCC_CRC_FORCE_RESET();
      __HAL_RCC_CRC_RELEASE_RESET();
      /* Calculate the CRC of the Block */
      *crc_value = (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)pBuffer, length);

      /* Release the Mutex */
      osMutexRelease(CRCMutexId);     
   }
   else
   {
      status = ERR_NO_RESOURCE_E;
   }
   
   return status;
}
