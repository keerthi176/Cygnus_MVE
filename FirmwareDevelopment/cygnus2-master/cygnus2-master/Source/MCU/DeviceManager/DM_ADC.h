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
*  File         : DM_ADC.h
*
*  Description  : Header for the ADC interface
*
*************************************************************************************/

#ifndef DM_ADC_H
#define DM_ADC_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"

#include "CO_ErrorCode.h"

/* User Include Files
*************************************************************************************/



/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t DM_ADC_Initialise(const ADC_TypeDef * const adc, const uint32_t channel);
ErrorCode_t DM_ADC_Deinitialise(const ADC_TypeDef * const adc);
ErrorCode_t DM_ADC_ReadReferenceVoltage(uint32_t* voltage);
ErrorCode_t DM_ADC_Read(ADC_HandleTypeDef* adc, const uint32_t channel, const uint32_t settlingTime, uint32_t* adcValue);

/* Public Constants
*************************************************************************************/
#define NBR_OF_VALUES_3  3

/* Macros
*************************************************************************************/



#endif // DM_ADC_H
