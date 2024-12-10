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
*  File         : RadioRegisterDump.cpp
*
*  Description  : Utility for outputing the LoRa chip's register values to a usart.
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/
#include "RadioRegisterDump.h"

/* User Definitions
*************************************************************************************/
#define CHAR 1

/* Type Declarations
*************************************************************************************/


/*************************************************************************************/
/**
* function name   : RadioRegisterDump
* description     : Constructor
*
* @param - None.
*
* @return - nothing
*/
RadioRegisterDump::RadioRegisterDump( void )
{
}

/*************************************************************************************/
/**
* function name   : RadioRegisterDump
* description     : Destructor
*
* @param - None.
*
* @return - nothing
*/
RadioRegisterDump::~RadioRegisterDump( void )
{
}

/*************************************************************************************/
/**
* function name   : DumpLoraRegisters
* description     : Read the contents of the radio registers (LoRa config) and output
*                   them on the supplied serial port in a human readable format.
*
* @param - radio     A reference to the radio object.
* @param - radio     A reference to the serial port object.
*
* @return - None.
*/
void RadioRegisterDump::DumpLoraRegisters(Radio &radio, Serial &serialPort)
{
   //FIFO not reported
   
   //RegOpMode
   uint8_t RegOpMode;
   radio.Read(REG_LR_OPMODE, &RegOpMode, CHAR);
   SerialSend(serialPort, (char*)"RegOpMode", RegOpMode);
   
   //TODO: add all of the remaining registers
}

/*************************************************************************************/
/**
* function name   : DumpLoraRegistersRaw
* description     : Read the contents of the radio registers (LoRa config) and output
*                   them in the format <address>  <value>.
*
* @param - radio     A reference to the radio object.
* @param - radio     A reference to the serial port object.
*
* @return - None.
*/
void RadioRegisterDump::DumpLoraRegistersRaw(Radio &radio, Serial &serialPort)
{
   uint8_t regValue;
   
   serialPort.printf("Raw Value Register Dump - Hex\r\n");
   serialPort.printf("Reg\tValue\r\n", REG_LR_FIFO, regValue);
   
   radio.Read(REG_LR_FIFO, &regValue, CHAR);
   serialPort.printf("0x%x\t0x%x\r\n", REG_LR_FIFO, regValue);

   radio.Read(REG_LR_OPMODE, &regValue, CHAR);
   serialPort.printf("0x%x\t0x%x\r\n", REG_LR_OPMODE, regValue);
   
   for ( uint32_t reg_index = REG_LR_FRFMSB; reg_index < REG_LR_INVERTIQ2; reg_index++)
   {
      radio.Read(reg_index, &regValue, CHAR);
      serialPort.printf("0x%x\t0x%x\r\n", reg_index, regValue);
   }
   
   for ( uint32_t reg_index = REG_LR_DIOMAPPING1; reg_index < REG_LR_AGCTHRESH3; reg_index++)
   {
      radio.Read(reg_index, &regValue, CHAR);
      serialPort.printf("0x%x\t0x%x\r\n", reg_index, regValue);
   }
   
   for ( uint32_t reg_index = REG_LR_DIOMAPPING1; reg_index < REG_LR_AGCTHRESH3; reg_index++)
   {
      radio.Read(reg_index, &regValue, CHAR);
      serialPort.printf("0x%x\t0x%x\r\n", reg_index, regValue);
   }
   
   radio.Read(REG_LR_PLLHOP, &regValue, CHAR);
   serialPort.printf("0x%x\t0x%x\r\n", REG_LR_PLLHOP, regValue);

   radio.Read(REG_LR_TCXO, &regValue, CHAR);
   serialPort.printf("0x%x\t0x%x\r\n", REG_LR_TCXO, regValue);
   
   radio.Read(REG_LR_PADAC, &regValue, CHAR);
   serialPort.printf("0x%x\t0x%x\r\n", REG_LR_PADAC, regValue);

   radio.Read(REG_LR_PLL, &regValue, CHAR);
   serialPort.printf("0x%x\t0x%x\r\n", REG_LR_PLL, regValue);
   
   radio.Read(REG_LR_PLLLOWPN, &regValue, CHAR);
   serialPort.printf("0x%x\t0x%x\r\n", REG_LR_PLLLOWPN, regValue);

   radio.Read(REG_LR_FORMERTEMP, &regValue, CHAR);
   serialPort.printf("0x%x\t0x%x\r\n", REG_LR_FORMERTEMP, regValue);
   
}

/*************************************************************************************/
/**
* function name   : SerialSend
* description     : Read the contents of the radio registers (LoRa config) and output
*                   them on the supplied serial port.
*
* @param - radio     A reference to the radio object.
* @param - radio     A reference to the serial port object.
*
* @return - None.
*/
void RadioRegisterDump::SerialSend(Serial &serialPort, char* pLabel, const uint8_t value)
{
   if ( pLabel )
   {
      serialPort.printf("%s\t\t0x%x\r\n", pLabel, value);
   }
}

