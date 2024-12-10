/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2019 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : DM_ADC.c
*
*  Description  : Implementation file for the Random Number generator
*
*************************************************************************************/

/* System Include Files
*************************************************************************************/

/* User Include Files
*************************************************************************************/
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_def.h"
#include "CO_SerialDebug.h"
#include "DM_RNG.h"


/* Private Functions Prototypes
*************************************************************************************/


/* Private definitions
*************************************************************************************/

/* Global Varbles
*************************************************************************************/
RNG_HandleTypeDef RngHandle;

/* Private Variables
*************************************************************************************/

/* Functions
*************************************************************************************/

/*************************************************************************************/
/**
* function name   : DM_RNG_Initialise
* description     : Initialise the RNG.
*
* @param - void.
*
* @return - ErrorCode_t  SUCCESS_E or an error code
*/
ErrorCode_t DM_RNG_Initialise(void)
{
   ErrorCode_t result = SUCCESS_E;
   
   RngHandle.Instance = RNG;
   
   __disable_irq();
   if ( HAL_RNG_Init(&RngHandle) != HAL_OK )
   {
      RngHandle.State = HAL_RNG_STATE_READY;
   }
   else 
   {
      //CO_PRINT_B_0(DBG_INFO_E,"Random number generator initialisation failed, retrying\r\n");
      DM_RNG_Deinitialise();
      if ( HAL_RNG_Init(&RngHandle) != HAL_OK )
      {
         RngHandle.State = HAL_RNG_STATE_READY;
      }
      else 
      {
         //CO_PRINT_B_0(DBG_INFO_E,"Random number generator initialisation failed\r\n");
         result = ERR_OPERATION_FAIL_E;
      }
   }
   __enable_irq();
   return result;
}

/*************************************************************************************/
/**
* function name   : DM_RNG_Deinitialise
* description     : Deinitialise the RNG.
*
* @param - void.
*
* @return - ErrorCode_t  SUCCESS_E or an error code
*/
ErrorCode_t DM_RNG_Deinitialise(void)
{
   ErrorCode_t result = SUCCESS_E;
   if ( HAL_RNG_DeInit(&RngHandle) != HAL_OK )
   {
      //CO_PRINT_B_0(DBG_INFO_E,"Random number generator deinitialise failed\r\n");
      result = ERR_OPERATION_FAIL_E;
   }
   return result;
}


/*************************************************************************************/
/**
* function name   : DM_RNG_Lock
* description     : Lock access to the RNG.
*
* @param - void.
*
* @return - ErrorCode_t  SUCCESS_E or an error code
*/
ErrorCode_t DM_RNG_Lock( void ) 
{
   ErrorCode_t result;
   if( RngHandle.Lock == HAL_LOCKED)
   {
      result = ERR_BUSY_E;
   }
   else
   {
      RngHandle.Lock = HAL_LOCKED;
      result = SUCCESS_E;
   }
   return result;
}

/*************************************************************************************/
/**
* function name   : DM_RNG_Unlock
* description     : Unlock access to the RNG.
*
* @param - void.
*
* @return - ErrorCode_t  SUCCESS_E
*/
ErrorCode_t DM_RNG_Unlock( void ) 
{
   RngHandle.Lock = HAL_UNLOCKED;
   return SUCCESS_E;
}

/*************************************************************************************/
/**
* function name   : DM_RNG_GetRandomNumber
* description     : Gets a random number from the RNG.
*
* @param - max_value        The max value to be returned.
*
* @return - uint32_t        Random integer within range 1 to max_value
*/
uint32_t DM_RNG_GetRandomNumber(const uint32_t max_value)
{
   uint32_t random = 0;
   ErrorCode_t status = SUCCESS_E;
   
   if ( (NULL == RngHandle.Instance) || (RngHandle.State != HAL_RNG_STATE_READY) )
   {
      status = DM_RNG_Initialise();
   }
   
   if ( SUCCESS == status )
   {
      HAL_StatusTypeDef hal_status = HAL_RNG_GenerateRandomNumber(&RngHandle, &random);
      
      if (HAL_OK == hal_status)
      {
         random %= max_value;
         random += 1;
      }
      else 
      {
         CO_PRINT_B_2(DBG_ERROR_E,"Random number generator failed.  Error=%d,  RNG State=%d\r\n", hal_status, RngHandle.State);
      }
   }
   else 
   {
      CO_PRINT_B_2(DBG_ERROR_E,"Random number generator INIT failed.  Error=%d,  RNG State=%d\r\n", status, RngHandle.State);
   }
   
   return random;
}

/*************************************************************************************/
/**
* function name   : HAL_RNG_MspInit
* description     : Override of weak HAL_RNG_MspInit for STM32L4.
*                    This function configures the hardware resources used in this example:
*                    Peripheral's clock enable
*
* @param - hrng      RNG handle pointer.
*
* @return - void
*/
void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng)
{
   //CO_PRINT_B_0(DBG_ERROR_E,"HAL_RNG_MspInit+\r\n");
   RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

   /*Select PLLQ output as RNG clock source */
   PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RNG;
   PeriphClkInitStruct.RngClockSelection = RCC_RNGCLKSOURCE_PLL;
   HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);

   /* RNG Peripheral clock enable */
   __HAL_RCC_RNG_CLK_ENABLE();
}

/*************************************************************************************/
/**
* function name   : HAL_RNG_MspDeInit
* description     : Override of weak HAL_RNG_MspDeInit for STM32L4.
*                    This function freeze the hardware resources used in this example:
*                    Disable the Peripheral's clock
*
* @param - hrng      RNG handle pointer.
*
* @return - void
*/
void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng)
{
   //CO_PRINT_B_0(DBG_ERROR_E,"HAL_RNG_MspDeInit+\r\n");
   /* Enable RNG reset state */
   __HAL_RCC_RNG_FORCE_RESET();

   /* Release RNG from reset state */
   __HAL_RCC_RNG_RELEASE_RESET();
}



