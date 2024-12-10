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
*  File         : DM_Log.c
*
*  Description  : Implementation of logging functions
*
*                 This module provides functionality for:
*                    - Logging messages transmitted over the Mesh
*                    - Logging messages received over the mesh
*
*************************************************************************************/



/* System Include files
*******************************************************************************/


/* User Include files
*******************************************************************************/
#include "DM_Log.h"
#include "DM_SerialPort.h"
#include "CO_Defines.h"
#include "utilities.h"
#ifdef SHOW_FREQUENCY_DEVIATION
#include "MC_MAC.h"
#endif

/* Global Variables
*******************************************************************************/
#define DATA_BUFFER_SIZE 51   // Note, code has been written for efficiency, requiring this value to be odd, so that DATA_MAX_LENGTH is even.
#define DATA_MAX_LENGTH (DATA_BUFFER_SIZE - 1)  // Note, code has been written for efficiency, requiring this value to be even.



/*************************************************************************************/
/**
* function name   : DM_LogPhyDataReq
* description     : Routine to log a message transmitted over the mesh
*
* @param - const uint32_t slot : slot index in superframe
* @param - const uint16_t address : unit address
* @param - const uint8_t frequency : frequency index
* @param - const uint8_t *pData : pointer to data to transmit
* @param - const uint32_t size : length in bytes of data to transmit
*
* @return - void
*/
void DM_LogPhyDataReq(const uint32_t slot, const uint16_t address, const uint8_t frequency, 
                      const uint8_t *pData, const uint32_t size)
{
#if (defined ENABLE_DEBUG_PRINT || defined _DEBUG)
   uint16_t x=0;
   uint16_t y=0;
   char buff[DATA_BUFFER_SIZE];
   const uint8_t length = MIN(size << 1, DATA_MAX_LENGTH);  // Note, code has been written for efficiency, requiring length to be even.
   uint8_t *pByte = (uint8_t *)pData;
   
   // suppress warnings
   (void)buff;

   // Copy data from pByte to buff[], noting that buff holds[] nibbles rather than bytes.
   // This copy algorithm requires that variable "length" takes an even number.
   for(x=0, y=1; x<length; x+=2, y+=2)
   {
      /* get hex char from top nibble and increment pointer to next byte */
      buff[x] = Nibble2HexChar(*pByte >> 4);
      /* get hex char from bottom nibble and increment pointer to next byte */
      buff[y] = Nibble2HexChar(*pByte & 0xf);
      pByte++;
   }
   
   /* terminate string */
   buff[length] = '\0';
   
   CO_PRINT_B_4(DBG_INFO_E, "[%d,t,%d,%d,,,%s]\n\r", slot, address, frequency, buff);
#endif
}


/*************************************************************************************/
/**
* function name   : DM_LogPhyDataInd
* description     : Routine to log a message received over the mesh
*
* @param - const uint32_t slot : slot index in superframe
* @param - const uint16_t address : unit address
* @param - const uint8_t frequency : frequency index
* @param - const int8_t rssi : received signal strength
* @param - const int8_t snr : received signal to noise ratio
* @param - const uint8_t *pData : pointer to data to transmit
* @param - const uint32_t size : length in bytes of data to transmit
* @param - const int32_t frequency_deviation : the frequency deviation reported by the LoRa chip.
*
* @return - void
*/
void DM_LogPhyDataInd(const uint32_t slot, const uint16_t address, const uint8_t frequency, 
                      const int8_t rssi, const int8_t snr, const uint8_t *pData, const uint32_t size, const int32_t frequency_deviation)
{
#if (defined ENABLE_DEBUG_PRINT || defined _DEBUG)
   uint16_t x=0;
   uint16_t y=0;
   char buff[DATA_BUFFER_SIZE];
   const uint8_t length = MIN(size << 1, DATA_MAX_LENGTH);  // Note, code has been written for efficiency, requiring length to be even.
   uint8_t *pByte = (uint8_t *)pData;
   
   // suppress warnings
   (void)buff;

   // Copy data from pByte to buff[], noting that buff holds[] nibbles rather than bytes.
   // This copy routine requires that variable "length" takes an even number.
   for(x=0, y=1; x<length; x+=2, y+=2)
   {
      /* get hex char from top nibble */
      buff[x] = Nibble2HexChar(*pByte >> 4);
      /* get hex char from bottom nibble and increment pointer to next byte */
      buff[y] = Nibble2HexChar(*pByte & 0xf);
      pByte++;
   }
   
   /* terminate string */
   buff[length] = '\0';
   
   CO_PRINT_B_6(DBG_INFO_E, "[%d,r,%d,%d,%d,%d,%s]\n\r", slot, address, frequency, rssi,snr, buff);
#ifdef SHOW_FREQUENCY_DEVIATION
   if ( MC_MAC_TEST_MODE_OFF_E == MC_GetTestMode() )
   {
      CO_PRINT_B_1(DBG_INFO_E, "Frequency deviation = %dHz\n\r", frequency_deviation);
   }
#endif
   
#endif
}

