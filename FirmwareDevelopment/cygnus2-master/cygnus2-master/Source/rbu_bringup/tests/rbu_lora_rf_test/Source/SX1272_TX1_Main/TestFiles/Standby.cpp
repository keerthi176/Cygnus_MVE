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
*  File         : Standby.cpp
*
*  Description  : Implementation for continuous Carrier output
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/
#include "Standby.h"

/* User Definitions
*************************************************************************************/
//#define NUM_FREQUENCY_CHANS 10
//uint32_t const frequencyChannels[NUM_FREQUENCY_CHANS] = {865150000, 865450000, 865750000, 866050000, 866350000,
//                                                         866650000, 866950000, 867250000, 867550000, 867850000};

//#define RF_CHANNEL                                       4
//#define TX_OUTPUT_POWER                                  9

//#define TX_TIMEOUT                                  0xFFFFFFF //Max timeout
//#define FSK_FDEV                                    25000     // Hz
//#define FSK_DATARATE                                19200     // bps
//#define FSK_BANDWIDTH                               50000     // Hz
//#define FSK_AFC_BANDWIDTH                           83333     // Hz
//#define FSK_PREAMBLE_LENGTH                         12        // Same for Tx and Rx
//#define FSK_FIX_LENGTH_PAYLOAD_ON                   true
//#define FSK_CRC_ENABLED                             false





/* Type Declarations
*************************************************************************************/


/*************************************************************************************/
/**
* function name   : Standby
* description     : Constructor
*
* @param - None.
*
* @return - nothing
*/
Standby::Standby( void ) : TestBase(), 
   m_Frequency(0),
   m_TxPower(0)
{
}

/*************************************************************************************/
/**
* function name   : Standby
* description     : Destructor
*
* @param - None.
*
* @return - nothing
*/
Standby::~Standby( void )
{
}

/*************************************************************************************/
/**
* function name   : Initialise
* description     : Test setup function
*
* @param - None.
*
* @return - true if initialisation works, false otherwise.
*/
bool Standby::Initialise( void )
{
   bool status = true;
   
   debug( "\n\n\r     SX1272 Standby\n\n\r" );
//   debug( "Frequency = %d (%d)\r\n", m_Frequency, frequencyChannels[m_Frequency]);

   Radio.Reset();
   Radio.Sleep();
   return status;
}

/*************************************************************************************/
/**
* function name   : RunProcess
* description     : Main test cycle procedure
*
* @param - None.
*
* @return - true if test is on-going, false if the test is complete.
*/
bool Standby::RunProcess( void )
{
   //there are reports of occasional transmission.  make sure the radio stays asleep.
   wait_ms(10);
   Radio.Sleep();
   return true;
}

/*************************************************************************************/
/**
* function name   : OnTxDone
* description     : Handler for TxDone
*
* @param - None.
*
* @return - nothing
*/
void Standby::OnTxDone( void )
{
   //nothing to do
}

/*************************************************************************************/
/**
* function name   : OnTxTimeout
* description     : Handler for TxTimeout
*
* @param - None.
*
* @return - nothing
*/
void Standby::OnTxTimeout( void )
{
   //nothing to do
}
   
/*************************************************************************************/
/**
* function name   : OnFhssChangeChannel
* description     : Handler for Fhss Change Channel
*
* @param - None.
*
* @return - nothing
*/
void Standby::OnFhssChangeChannel( uint8_t channelIndex )
{
   //nothing to do
}

/*************************************************************************************/
/**
* function name   : ProcessCommand
* description     : Handler for usart commands
*
* @param - None.
*
* @return - bool  true if command succeeded, false otherwise.
*/
bool Standby::ProcessCommand(const char* pCommand)
{
   bool result = false;
   if ( pCommand )
   {
      if ( 0 == strncmp(pCommand, "reset", 5) )
      {
         Radio.Sleep();
         Initialise();
         result = true;
      }
   }
   return result;
}

/*************************************************************************************/
/**
* function name   : PrintHelp
* description     : Output help information to the supplied serial port
*
* @param - serialPort - The port to print to.
*
* @return - nothing
*/
void Standby::PrintHelp(Serial &serialPort)
{
   serialPort.printf("Currently loaded test = standby\r\n\n");
   serialPort.printf("Supported commands:\r\n");
   serialPort.printf("reset\r\n");
   serialPort.printf("\r\n");
}



