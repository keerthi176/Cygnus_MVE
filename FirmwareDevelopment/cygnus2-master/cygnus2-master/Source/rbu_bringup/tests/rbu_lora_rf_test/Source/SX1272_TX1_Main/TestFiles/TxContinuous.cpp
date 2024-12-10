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
*  File         : TxContinuous.cpp
*
*  Description  : Implementation for continuous Carrier output
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/
#include "TxContinuous.h"

/* User Definitions
*************************************************************************************/
#define NUM_FREQUENCY_CHANS 11
uint32_t const frequencyChannels865[NUM_FREQUENCY_CHANS] = {865150000, 865450000, 865750000, 866050000, 866350000,
                                                         866650000, 866950000, 867250000, 867550000, 867850000, 868150000};
uint32_t const frequencyChannels915[NUM_FREQUENCY_CHANS] = {918150000, 918750000, 919350000, 919950000, 920550000,
                                                         921150000, 921750000, 922350000, 922950000, 923550000, 924150000};
#define FREQUENCY_BAND_865 0
#define FREQUENCY_BAND_915 1

#ifdef FREQUENCY_BAND_915MHZ
static uint32_t const frequencyChannels[NUM_FREQUENCY_CHANS] = {920150000, 920450000, 920750000, 921050000, 921350000,
                                                                 921650000, 921950000, 922250000, 922550000, 922850000, 923150000};
#else
static uint32_t const frequencyChannels[NUM_FREQUENCY_CHANS] = {865150000, 865450000, 865750000, 866050000, 866350000,
                                                                 866650000, 866950000, 867250000, 867550000, 867850000, 868150000};
#endif
#define LORA_BANDWIDTH_865                          1         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_BANDWIDTH_915                          2         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]

#define RF_CHANNEL                                       0
#define TX_OUTPUT_POWER                                  7

#define TX_TIMEOUT                                  0xFFFFFFF //Max timeout
#define FSK_FDEV                                    25000     // Hz
#define FSK_DATARATE                                19200     // bps
#define FSK_BANDWIDTH                               50000     // Hz
#define FSK_AFC_BANDWIDTH                           83333     // Hz
#define FSK_PREAMBLE_LENGTH                         12        // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   true
#define FSK_CRC_ENABLED                             false





/* Type Declarations
*************************************************************************************/


/*************************************************************************************/
/**
* function name   : TxContinuous
* description     : Constructor
*
* @param - None.
*
* @return - nothing
*/
TxContinuous::TxContinuous( void ) : TestBase(), 
   m_Frequency(RF_CHANNEL),
   m_FrequencyBand(FREQUENCY_BAND_915),
   m_TxPower(TX_OUTPUT_POWER),
   m_CurrentBit(false)
{
}

/*************************************************************************************/
/**
* function name   : TxContinuous
* description     : Destructor
*
* @param - None.
*
* @return - nothing
*/
TxContinuous::~TxContinuous( void )
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
bool TxContinuous::Initialise( void )
{
   bool status = true;
   uint32_t frequency;
   
   m_CurrentBit = false;
   
   if ( FREQUENCY_BAND_915 == m_FrequencyBand )
   {
      frequency = frequencyChannels915[m_Frequency];
   }
   else 
   {
      frequency = frequencyChannels865[m_Frequency];
   }

   debug( "\n\n\r     SX1272 TX Continuous Carrier\n\n\r" );
   debug( "Frequency = %d (%d)\r\n", m_Frequency, frequency);
   debug( "Tx Power  = %d\r\n", m_TxPower);

   Radio.Reset();
   Radio.Sleep();
   Radio.SetTxContinuousMode( frequency, m_TxPower, FSK_FDEV, FSK_BANDWIDTH, FSK_DATARATE, false, TX_TIMEOUT );
   Radio.Send(NULL,0);
   
   Radio.SetContinouousDatabit( m_CurrentBit );
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
bool TxContinuous::RunProcess( void )
{
   //nothing to do
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
void TxContinuous::OnTxDone( void )
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
void TxContinuous::OnTxTimeout( void )
{
   uint32_t frequency;
   
   if ( FREQUENCY_BAND_915 == m_FrequencyBand )
   {
      frequency = frequencyChannels915[m_Frequency];
   }
   else 
   {
      frequency = frequencyChannels865[m_Frequency];
   }
   // Restarts continuous wave transmission when timeout expires
   Radio.SetTxContinuousMode( frequency, m_TxPower, FSK_FDEV, FSK_BANDWIDTH, FSK_DATARATE, false, TX_TIMEOUT );
   Radio.Send(NULL,0);
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
void TxContinuous::OnFhssChangeChannel( uint8_t channelIndex )
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
bool TxContinuous::ProcessCommand(const char* pCommand)
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
      else if ( 0 == strncmp(pCommand, "frequency", 9) )
      {
         char* pValue = (char*)pCommand + 9;
         if( *pValue == '=' )
         {
            pValue++;
            int32_t value = atoi(pValue);
            if ( value >= 0 && value < NUM_FREQUENCY_CHANS)
            {
               m_Frequency = value;
               Radio.Sleep();
               Initialise();
               result = true;
            }
         }
      }
      else if ( 0 == strncmp(pCommand, "power", 5) )
      {
         char* pValue = (char*)pCommand + 5;
         if( *pValue == '=' )
         {
            pValue++;
            int32_t value = atoi(pValue);
            if ( value >= 0 )//&& value < 11)
            {
               m_TxPower = value;
               Radio.Sleep();
               Initialise();
               result = true;
            }
         }
      }
      else if ( 0 == strncmp(pCommand, "bit", 3) )
      {
         char* pValue = (char*)pCommand + 3;
         if( *pValue == '=' )
         {
            pValue++;
            int32_t value = atoi(pValue);
            if ( value == 0 || value == 1)
            {
               if ( value == 0 )
               {
                  m_CurrentBit = false;
               }
               else
               {
                  m_CurrentBit = true;
               }
               Radio.SetContinouousDatabit(m_CurrentBit);
               result = true;
            }
         }
      }
      else if ( 0 == strncmp(pCommand, "Band", 4) )
      {
         char* pValue = (char*)pCommand + 4;
         if( *pValue == '=' )
         {
            pValue++;
            int32_t value = atoi(pValue);
            if ( 865 == value )
            {
               m_FrequencyBand = FREQUENCY_BAND_865;
               result = true;
   }
            else if ( 915 == value )
            {
               m_FrequencyBand = FREQUENCY_BAND_865;
               result = true;
            }
            else
            {
               result = false;
            }
            
            if ( result )
            {
               Radio.Sleep();
               Initialise();
            }
         }
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
void TxContinuous::PrintHelp(Serial &serialPort)
{
   serialPort.printf("Currently loaded test = continuous carrier\r\n\n");
   serialPort.printf("Supported commands:\r\n");
   serialPort.printf("frequency=x (%d)\r\n", m_Frequency);
   serialPort.printf("Band=x (%d)  (865 or 915)\r\n", m_FrequencyBand == FREQUENCY_BAND_865 ? 865:915 );
   serialPort.printf("bit=x       (%d)\r\n", m_CurrentBit);
   serialPort.printf("power=x     (%d)\r\n", m_TxPower);
   serialPort.printf("reset\r\n");
   serialPort.printf("\r\n");
}



