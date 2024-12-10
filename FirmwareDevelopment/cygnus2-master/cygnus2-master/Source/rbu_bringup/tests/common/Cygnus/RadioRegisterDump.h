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
*  File         : RadioRegisterDump.h
*
*  Description  : Header for the utility for outputing the LoRa chip's register values to a usart.
*
*************************************************************************************/

#ifndef RADIOREGISTERDUMP_H
#define RADIOREGISTERDUMP_H

/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include "sx1272.h"

/* User Include Files
*************************************************************************************/


/* Type Declarations
*************************************************************************************/


class RadioRegisterDump
{
   
   public:
      RadioRegisterDump( void );
      ~RadioRegisterDump( void);
      
      void DumpLoraRegisters(Radio &radio, Serial &serialPort);
      void DumpLoraRegistersRaw(Radio &radio, Serial &serialPort);
   
      void SerialSend(Serial &serialPort, char* label, const uint8_t value);
};
#endif //RADIOREGISTERDUMP_H
