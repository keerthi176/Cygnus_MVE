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
*  File         : DM_Log.h
*
*  Description  : Header file for the Logging module
*
*                 This logs messages to the debug serial port at key events
*                 e.g. when messages are transmitted or received over the radio
*
*************************************************************************************/

#ifndef DM_LOG_H
#define DM_LOG_H


/* System Include files
*******************************************************************************/
#include <stdint.h>


/* User Include files
*******************************************************************************/


/* Public Structures
*******************************************************************************/


/* Public Enumerations
*******************************************************************************/ 



/* Public functions prototypes
*******************************************************************************/
void DM_LogPhyDataReq(const uint32_t slot, const uint16_t address, const uint8_t frequency, 
                      const uint8_t *pData, const uint32_t size);
void DM_LogPhyDataInd(const uint32_t slot, const uint16_t address, const uint8_t frequency, 
                      const int8_t rssi, const int8_t snr, const uint8_t *pData, 
                      const uint32_t size, const int32_t frequency_deviation);



/* Public Constants
*******************************************************************************/


/* Macros
*******************************************************************************/



#endif // DM_LOG_H
