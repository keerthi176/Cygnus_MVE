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
*  Description  : Implementation file for the ADC interface
*
*************************************************************************************/

/* System Include Files
*************************************************************************************/

/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "stm32l4xx.h"
#include "board.h"
#include "CO_Defines.h"
#include "DM_ADC.h"


/* Private Functions Prototypes
*************************************************************************************/
ErrorCode_t DM_ADC_Configure(ADC_InitTypeDef* adcConfig);

/* Private definitions
*************************************************************************************/

/* Global Varbles
*************************************************************************************/
extern DM_BaseType_t gBaseType;

ADC_HandleTypeDef adc1Handle;

/* Private Variables
*************************************************************************************/

/* Functions
*************************************************************************************/

/*****************************************************************************
*  Function:      DM_ADC_Initialise
*  Description:   Initialisation function for the ADC channels 
*
*  param    adc      Pointer to the ADC module to be initialised.  ADC1, ADC2, ADC3.
*  param    channel  The ADC Channel to be initialised.
*  return            Error code.
*
*  Notes:
*****************************************************************************/
ErrorCode_t DM_ADC_Initialise(const ADC_TypeDef * const adc, const uint32_t channel)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   ADC_MultiModeTypeDef multimode = {0};
   ADC_ChannelConfTypeDef sConfig = {0};

   if ( adc )
   {
      result = ERR_OUT_OF_RANGE_E;
      
      if ( ADC1 == adc )
      {
         result = ERR_INIT_FAIL_E;
         if ( SUCCESS_E == DM_ADC_Configure(&adc1Handle.Init) )
         {
            if ( HAL_OK == HAL_ADC_Init(&adc1Handle) )
            {
               multimode.Mode = ADC_MODE_INDEPENDENT;
               if ( HAL_OK == HAL_ADCEx_MultiModeConfigChannel(&adc1Handle, &multimode) )
               {
                  sConfig.Channel = channel;
                  sConfig.Rank = ADC_REGULAR_RANK_1;
                  sConfig.SamplingTime = ADC_SAMPLETIME_24CYCLES_5;
                  sConfig.SingleDiff = ADC_SINGLE_ENDED;
                  sConfig.OffsetNumber = ADC_OFFSET_NONE;
                  sConfig.Offset = 0;
                  if ( HAL_OK == HAL_ADC_ConfigChannel(&adc1Handle, &sConfig) )
                  {
                     result = SUCCESS_E;
                     osDelay(1);//Time for the ADC to stabilise
                  }
               }
            }
         }
      }
      else 
      {
         result = ERR_OUT_OF_RANGE_E;
      }

   }
   
   return result;
}

/*****************************************************************************
*  Function:      DM_ADC_Initialise
*  Description:   Initialisation function for the ADC channels 
*
*  param    adc      The ADC module to be initialised.  ADC1, ADC2, ADC3.
*  return            Error code.
*
*  Notes:
*****************************************************************************/
ErrorCode_t DM_ADC_Deinitialise(const ADC_TypeDef * const adc)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( adc )
   {
      if ( ADC1 == adc )
      {
         if ( HAL_OK == HAL_ADC_DeInit(&adc1Handle) )
         {
            result = SUCCESS_E;
         }
      }
      else 
      {
         result = ERR_OUT_OF_RANGE_E;
      }

   }
   
   return result;
}

/*****************************************************************************
*  Function:      DM_ADC_Configure
*  Description:   Populate an ADC Init structure 
*
*  param    adcConfig   The ADC Init structure
*  return               Error Code SUCCESS_E or ERR_INIT_FAIL_E.
*
*  Notes:
*****************************************************************************/
ErrorCode_t DM_ADC_Configure(ADC_InitTypeDef* adcConfig)
{
   ErrorCode_t result = ERR_INIT_FAIL_E;
   
   if ( adcConfig )
   {
      adcConfig->ClockPrescaler = ADC_CLOCK_ASYNC_DIV1; //ADC_CLOCKPRESCALER_PCLK_DIV4;
      adcConfig->Resolution = ADC_RESOLUTION_12B;
      adcConfig->DataAlign = ADC_DATAALIGN_RIGHT;
      adcConfig->ScanConvMode = ADC_SCAN_DISABLE;
      adcConfig->EOCSelection = ADC_EOC_SINGLE_CONV;
      adcConfig->LowPowerAutoWait = DISABLE;
      adcConfig->ContinuousConvMode = DISABLE;
      adcConfig->NbrOfConversion = 1;
      adcConfig->NbrOfDiscConversion = 0;
      adcConfig->DiscontinuousConvMode = DISABLE;
      adcConfig->ExternalTrigConv = ADC_SOFTWARE_START;
      adcConfig->ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
      adcConfig->DMAContinuousRequests = DISABLE;
      adcConfig->Overrun = ADC_OVR_DATA_OVERWRITTEN; //ADC_OVR_DATA_PRESERVED;
      adcConfig->OversamplingMode = DISABLE;
      
      result = SUCCESS_E;
   }
   
   return result;
}

/*****************************************************************************
*  Function:      HAL_ADC_MspInit
*  Description:   Override HAL weak function. Initialise ADC hardware resources 
*
*  param    hadc   The ADC Init structure
*  return   None
*
*  Notes:
*****************************************************************************/
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hadc->Instance==ADC1)
  {
    /* Peripheral clock enable */
    __HAL_RCC_ADC_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /**ADC1 GPIO Configuration
    PC2     ------> ADC1_IN3
    PC3     ------> ADC1_IN4
    PA2     ------> ADC1_IN7
    PA3     ------> ADC1_IN8 
    */
    if ( BASE_IOU_E == gBaseType )
    {
        GPIO_InitStruct.Pin |= (GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    }
    else 
    {
       GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    }
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}

/*****************************************************************************
*  Function:      HAL_ADC_MspInit
*  Description:   Override HAL weak function. Release ADC hardware resources 
*
*  param    hadc   The ADC Init structure
*  return   None
*
*  Notes:
*****************************************************************************/
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance==ADC1)
  {
    /* Peripheral clock disable */
    __HAL_RCC_ADC_CLK_DISABLE();
  
    /**ADC1 GPIO Configuration
    PC2     ------> ADC1_IN3
    PC3     ------> ADC1_IN4
    PA2     ------> ADC1_IN7
    PA3     ------> ADC1_IN8 
    */
    if ( BASE_IOU_E == gBaseType )
    {
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
    }
    else 
    {
         HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2|GPIO_PIN_3);
    }

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);
  }

}

/*****************************************************************************
*  Function:      DM_ADC_ReadReferenceVoltage
*  Description:   Reads the ADC reference voltage.
*
*  param   voltage   The reference voltage in mV.
*  return            SUCCESS_E or error code
*
*  Notes:
*****************************************************************************/
ErrorCode_t DM_ADC_ReadReferenceVoltage(uint32_t* voltage)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   uint32_t adcRef = 0U;
   uint32_t corrected_voltage_mv = 0U;
   uint32_t adcRefTemp[3U] = {0U};
   uint8_t i = 0U;
   
   if ( voltage )
   {
      result = DM_ADC_Initialise(adc1Handle.Instance, ADC_CHANNEL_VREFINT);
      
      if ( SUCCESS_E == result )
      {
         osDelay(1); /* 1ms settling time */
         HAL_ADC_Start(&adc1Handle);
         osDelay(1);
         HAL_StatusTypeDef status = HAL_ADC_PollForConversion(&adc1Handle, 100);
         if ( HAL_OK == status )
         {
            for (i=0U; i<NBR_OF_VALUES_3; i++)
            {
               adcRefTemp[i] = HAL_ADC_GetValue(&adc1Handle);
               osDelay(1);
            }
            result = SUCCESS_E;
            adcRef = (adcRefTemp[0U] + adcRefTemp[1U] + adcRefTemp[2U]) / 3U; /* Calculate average from 3 values */
            corrected_voltage_mv = __HAL_ADC_CALC_VREFANALOG_VOLTAGE(adcRef, LL_ADC_RESOLUTION_12B);
            *voltage = corrected_voltage_mv;
            CO_PRINT_B_2(DBG_INFO_E,"DM_ADC_ReadReferenceVoltage, adcRef (average)= %d, adcRefVoltage = %dmV\r\n", adcRef, corrected_voltage_mv);
         }
         else
         {
            result = ERR_TIMEOUT_E;
            CO_PRINT_B_1(DBG_ERROR_E,"DM_TMP_ReadTemperature PollForConversion FAILED error = %d\r\n", status);
         }
         
         HAL_ADC_Stop(&adc1Handle);
         DM_ADC_Deinitialise(adc1Handle.Instance);
      }
   }
   return result;
}

/*****************************************************************************
*  Function:      DM_ADC_Read
*  Description:   Reads the ADC channel specified and puts the result into parameter adcValue.
*
*  param   adc          The ADC to use.
*  param   channel      The ADC channel to read.
*  param   settlingTime Delay in milliseconds before taking the reading (not used, it is now fixed at 1ms).
*  param   adcValue     [OUT]A pointer to the output variable to receive the raw adc reading.
*
*  return               SUCCESS_E or error code
*
*  Notes:
*****************************************************************************/
ErrorCode_t DM_ADC_Read(ADC_HandleTypeDef* adc, const uint32_t channel, const uint32_t settlingTime, uint32_t* adcValue)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   uint32_t adcTemp[3U] = {0U};
   uint8_t i = 0U;
   
   if ( adc && adcValue )
   {
      result = DM_ADC_Initialise(adc->Instance, channel);
      
      if ( SUCCESS_E == result )
      {
         //CO_InlineDelay(settlingTime);
         osDelay(1); /* 1ms settling time */
         HAL_ADC_Start(adc);
         osDelay(1);
         result = ERR_INIT_FAIL_E;
         HAL_StatusTypeDef status = HAL_ADC_PollForConversion(adc, 1000);
         if ( HAL_OK == status )
         {
            for (i=0U; i<NBR_OF_VALUES_3; i++)
            {
                adcTemp[i] = HAL_ADC_GetValue(adc);
                CO_PRINT_B_2(DBG_INFO_E,"DM_ADC_Read channel=%d, adcValue = %d\r\n", channel, adcTemp[i]);
                osDelay(1);
            }
            result = SUCCESS_E;
            *adcValue = (adcTemp[0U] + adcTemp[1U] + adcTemp[2U]) / 3U; /* Calculate average from 3 values */
            CO_PRINT_B_1(DBG_INFO_E,"DM_ADC_Read adcValue (average) = %d\r\n", *adcValue);
         }
         else
         {
            CO_PRINT_B_2(DBG_ERROR_E,"DM_ADC_Read FAILED channel=%d, error = %d\r\n", channel, status);
         }
         
         HAL_ADC_Stop(adc);
         DM_ADC_Deinitialise(adc->Instance);
      }
   }
   return result;
}


/* Callback functions called by ADC Interrupt handlers */
void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc)
{
   /*Do Nothing*/
}
void HAL_ADCEx_InjectedQueueOverflowCallback(ADC_HandleTypeDef* hadc)
{
   /*Do Nothing*/
}
void HAL_ADCEx_LevelOutOfWindow2Callback(ADC_HandleTypeDef* hadc)
{
   /*Do Nothing*/
}
void HAL_ADCEx_LevelOutOfWindow3Callback(ADC_HandleTypeDef* hadc)
{
   /*Do Nothing*/
}
void HAL_ADCEx_EndOfSamplingCallback(ADC_HandleTypeDef* hadc)
{
   /*Do Nothing*/
}

