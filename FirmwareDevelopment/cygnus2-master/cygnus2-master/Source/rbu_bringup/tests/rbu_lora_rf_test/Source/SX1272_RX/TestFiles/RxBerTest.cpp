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
*  File         : RxBerTest.h
*
*  Description  : Header for the Bit Error Rate test.
*                 Used in conjunction with Tx test TxBerTestSignal.cpp
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include "mbed.h"


/* User Include Files
*************************************************************************************/
#include "RxBerTest.h"

/* User Definitions
*************************************************************************************/

#define BER_READ_INTERVAL_us 1000000

#define SHOW_DEBUG 0

#define NUM_FREQUENCY_CHANS 11
uint32_t const frequencyChannels865[NUM_FREQUENCY_CHANS] = {865150000, 865450000, 865750000, 866050000, 866350000,
                                                         866650000, 866950000, 867250000, 867550000, 867850000, 868150000};
uint32_t const frequencyChannels915[NUM_FREQUENCY_CHANS] = {918150000, 918750000, 919350000, 919950000, 920550000,
                                                         921150000, 921750000, 922350000, 922950000, 923550000, 924150000};
#define FREQUENCY_BAND_865 0
#define FREQUENCY_BAND_915 1

#define RF_CHANNEL                                       4

#define PACKET_MODE                                 false
#define USE_CAD                                     0       // 0 : do not use CAD, 1 : use CAD
#define FSK_FDEV                                    25000     // Hz
#define FSK_DATARATE                                19200     // bps
#define FSK_BANDWIDTH                               50000     // Hz
#define FSK_AFC_BANDWIDTH                           83333     // Hz
#define FSK_PREAMBLE_LENGTH                         12        // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   true
#define FSK_CRC_ENABLED                             false

#define USE_SEQUENCER                               true
   
/* Type Declarations
*************************************************************************************/
static CRC_HandleTypeDef crc_handle_8b_data_16b_crc;


/* Global Variables
*************************************************************************************/
Ticker BerTimer;


/*************************************************************************************/
/**
* function name   : RxBerTest
* description     : Constructor
*
* @param - None.
*
* @return - nothing
*/
RxBerTest::RxBerTest( void ) : RxTestBase(), 
   m_Count(0),
   m_Event(BER_EVENT_NONE_E),
   m_CurrentFrequency(RF_CHANNEL),
   m_FrequencyBand(FREQUENCY_BAND_915),
   m_CurrentPeriod(BER_READ_INTERVAL_us)
{
}

/*************************************************************************************/
/**
* function name   : RxBerTest
* description     : Destructor
*
* @param - None.
*
* @return - nothing
*/
RxBerTest::~RxBerTest( void )
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
bool RxBerTest::Initialise( void )
{
   bool status = true;
   uint32_t frequency;
   
  
   if ( FREQUENCY_BAND_915 == m_FrequencyBand )
   {
      frequency = frequencyChannels915[m_CurrentFrequency];
   }
   else 
   {
      frequency = frequencyChannels865[m_CurrentFrequency];
   }
   
  
   Radio.SetChannel( frequency );
   
   debug( "\r\n\n     SX1272 RX BER Test\r\n\n" );

   debug( "Frequency = %d (%d)\r\n", m_CurrentFrequency, frequency);
   debug( "period    = %d seconds\r\n\n", (m_CurrentPeriod/1000000));

   //Initialise the timer for Rx time.
   BerTimer.detach();
   BerTimer.attach_us(Callback<void(void)>(this, &RxBerTest::TimerCallback), m_CurrentPeriod);

   Radio.SetRxContinuousMode( frequency, FSK_FDEV, FSK_BANDWIDTH, FSK_DATARATE, FSK_CRC_ENABLED, USE_SEQUENCER );
   StartReceiving();

   return status;
}

/*************************************************************************************/
/**
* function name   : TimerCallback
* description     : Called by Timer periodically.
*
* @param - None.
*
* @return - none.
*/
void RxBerTest::TimerCallback(void)
{
   m_Event = BER_EVENT_TIMER_E;
}


/*************************************************************************************/
/**
* function name   : StartReceiving
* description     : Start the reciever
*
* @param - None.
*
* @return - None.
*/
void RxBerTest::StartReceiving(void)
{
   Radio.Rx( 0 );
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
bool RxBerTest::RunProcess( void )
{
      
   if ( BER_EVENT_TIMER_E == m_Event )
   {
      m_Event = BER_EVENT_NONE_E;
      uint32_t zeroCount;
      uint32_t oneCount;
      double bitError = 0.0;
      Radio.ReadAndResetSXDataCounters(&zeroCount, &oneCount, &bitError);
      if ( m_RunningAveBitError > -0.00001 && m_RunningAveBitError < 0.00001 )
      {
         m_RunningAveBitError = bitError;
      }
      else 
      {
         m_RunningAveBitError = (m_RunningAveBitError + bitError) / 2;
      }
      int32_t bitCount = zeroCount + oneCount;
      m_RssiValue = Radio.GetFskRSSI();
      uint8_t lna_reg = Radio.Read(REG_LNA);
      uint8_t lna_gain = lna_reg >> 5;
      uint8_t lna_boost = lna_reg & 0x03;
      uint8_t rxconfig_reg = Radio.Read(REG_RXCONFIG);
      uint8_t agc = rxconfig_reg & 0x008;
      debug("'0'=%d\t'1'=%d\tbits=%d\tber=%0.3f%%\trssi=%d\tLNAboost=%d\tLNAgain=%d\tagc=%s\r\n", zeroCount, oneCount, bitCount, m_RunningAveBitError, m_RssiValue, lna_boost, lna_gain, (agc == 0 ?"off":"on"));
   }

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
void RxBerTest::OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror)
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
void RxBerTest::OnRxTimeout( void )
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
void RxBerTest::OnRxError( void )
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
void RxBerTest::OnCadDone( bool channelActivityDetected )
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
void RxBerTest::OnFhssChangeChannel( uint8_t channelIndex )
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
bool RxBerTest::ProcessCommand(const char* pCommand)
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
               m_CurrentFrequency = value;
               Radio.Sleep();
               Initialise();
               result = true;
            }
         }
      }
      else if ( 0 == strncmp(pCommand, "period", 6) )
      {
         char* pValue = (char*)pCommand + 6;
         if( *pValue == '=' )
         {
            pValue++;
            int32_t value = atoi(pValue);
            if ( value > 0 )
            {
               m_CurrentPeriod = value * 1000000;
               BerTimer.detach();
               BerTimer.attach_us(Callback<void(void)>(this, &RxBerTest::TimerCallback), m_CurrentPeriod);
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
void RxBerTest::PrintHelp(Serial &serialPort)
{
   serialPort.printf("Currently loaded test = BERTest\r\n\n");
   serialPort.printf("Supported commands:\r\n");
   serialPort.printf("frequency=x (%d)\r\n", m_CurrentFrequency);
   serialPort.printf("Band=x (%d)  (865 or 915)\r\n", m_FrequencyBand == FREQUENCY_BAND_865 ? 865:915 );
   serialPort.printf("period=x (%d seconds)\r\n", (m_CurrentPeriod/1000000));
   serialPort.printf("reset\r\n");
   serialPort.printf("\r\n");
}
