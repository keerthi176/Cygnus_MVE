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
*  File         : TxBerTestSignal.h
*
*  Description  : Implementation for the Bit Error Rate test.
*                 Used in conjunction with Rx test TxBerTestSignal.cpp
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include "mbed.h"


/* User Include Files
*************************************************************************************/
#include "TxBerTestSignal.h"

/* User Definitions
*************************************************************************************/

#define SHOW_DEBUG 0

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

#define RF_CHANNEL                                       0
#define TX_OUTPUT_POWER                                  7

#define PACKET_MODE                                 false
#define USE_CAD                                     0       // 0 : do not use CAD, 1 : use CAD
#define FSK_FDEV                                    25000     // Hz
#define FSK_DATARATE                                19200     // bps
#define FSK_BANDWIDTH                               50000     // Hz
#define FSK_AFC_BANDWIDTH                           83333     // Hz
#define FSK_PREAMBLE_LENGTH                         12        // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   true
#define FSK_CRC_ENABLED                             false

#define USE_SEQUENCER                               false
   
/* Type Declarations
*************************************************************************************/


/* Global Variables
*************************************************************************************/


/*************************************************************************************/
/**
* function name   : TxBerTestSignal
* description     : Constructor
*
* @param - None.
*
* @return - nothing
*/
TxBerTestSignal::TxBerTestSignal( void ) : TestBase(),
   m_Event(BER_EVENT_NONE_E),
   m_Frequency(RF_CHANNEL),
   m_FrequencyBand(FREQUENCY_BAND_915),
   m_TxPower(TX_OUTPUT_POWER)
{
}

/*************************************************************************************/
/**
* function name   : TxBerTestSignal
* description     : Destructor
*
* @param - None.
*
* @return - nothing
*/
TxBerTestSignal::~TxBerTestSignal( void )
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
bool TxBerTestSignal::Initialise( void )
{
   bool status = true;
   uint32_t frequency;
   
   
   if ( FREQUENCY_BAND_915 == m_FrequencyBand )
   {
      frequency = frequencyChannels915[m_Frequency];
   }
   else 
   {
      frequency = frequencyChannels865[m_Frequency];
   }

   debug( "\r\n\n     TX BER Test Signal\r\n\n" );

   debug( "Frequency = %d (%d)\r\n", m_Frequency, frequency);
   

   Radio.EnableContinouousDatabitToggle(true);
   debug("\n\n\r> FSK Toggle Data Mode < \n\n\r");
   Radio.SetTxContinuousMode( frequency, m_TxPower, FSK_FDEV, FSK_BANDWIDTH, FSK_DATARATE, USE_SEQUENCER, 0xffffffff );
   Radio.Send(NULL,0);

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
bool TxBerTestSignal::RunProcess( void )
{
      
   // nothing to do

   return true;
}

/*************************************************************************************/
/**
* function name   : OnRxDone
* description     : Handler for TxDone
*
* @param - None.
*
* @return - nothing
*/
void TxBerTestSignal::OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror)
{
// nothing to do
}

/*************************************************************************************/
/**
* function name   : OnRxTimeout
* description     : Handler for TxTimeout
*
* @param - None.
*
* @return - nothing
*/
void TxBerTestSignal::OnRxTimeout( void )
{
// nothing to do
}
   
/*************************************************************************************/
/**
* function name   : OnRxError
* description     : Handler for Rx Error
*
* @param - None.
*
* @return - nothing
*/
void TxBerTestSignal::OnRxError( void )
{
// nothing to do
};

/*************************************************************************************/
/**
* function name   : OnRxError
* description     : Handler for Rx Error
*
* @param - None.
*
* @return - nothing
*/
void TxBerTestSignal::OnCadDone( bool channelActivityDetected )
{
// nothing to do
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
void TxBerTestSignal::OnFhssChangeChannel( uint8_t channelIndex )
{
// nothing to do
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
bool TxBerTestSignal::ProcessCommand(const char* pCommand)
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
            if ( value >= 0 && value < 10)
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
void TxBerTestSignal::PrintHelp(Serial &serialPort)
{
   serialPort.printf("Currently loaded test = ber\r\n\n");
   serialPort.printf("Supported commands:\r\n");
   serialPort.printf("frequency=x (%d)\r\n", m_Frequency);
   serialPort.printf("Band=x (%d)  (865 or 915)\r\n", m_FrequencyBand == FREQUENCY_BAND_865 ? 865:915 );
   serialPort.printf("power=x (%d)\r\n", m_TxPower);
   serialPort.printf("reset\r\n");
   serialPort.printf("\r\n");
}

