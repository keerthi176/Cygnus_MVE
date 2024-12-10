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

#ifndef DM_RNG_H
#define DM_RNG_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "CO_ErrorCode.h"

/* User Include Files
*************************************************************************************/



/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t DM_RNG_Initialise(void);
ErrorCode_t DM_RNG_Deinitialise(void);
uint32_t DM_RNG_GetRandomNumber(const uint32_t max_value);
/* Public Constants
*************************************************************************************/

/* Macros
*************************************************************************************/



#endif // DM_RNG_H
