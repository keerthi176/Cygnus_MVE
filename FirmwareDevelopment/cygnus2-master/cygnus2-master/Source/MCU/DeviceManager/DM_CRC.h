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
*  File         : DM_CRC.h
*
*  Description  : Header for the CRC module
*
*************************************************************************************/

#ifndef DM_CRC_H
#define DM_CRC_H


/* System Include Files
*************************************************************************************/


/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"


/* Public Structures
*************************************************************************************/ 


/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t DM_CrcInit(void);
ErrorCode_t DM_Crc16bCalculate8bDataWidth(uint8_t *pBuffer, uint32_t BufferLength, uint16_t *crc_value, uint32_t timeout_ms);


/* Public Constants
*************************************************************************************/



/* Macros
*************************************************************************************/



#endif // DM_CRC_H
