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
*  File         : RxContinuous.cpp
*
*  Description  : Header for the continuous receive test.  This test reports the '0' 
*                 or '1' value on the data output pin of the SX1273 chip in continuous
*                 FSK mode.
*                 Used in conjunction with Tx test TxContinuous.h
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include "mbed.h"


/* User Include Files
*************************************************************************************/
#include "RxContinuous.h"

/* User Definitions
*************************************************************************************/

#define RXC_READ_INTERVAL_us 1000000

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
Ticker RxTimer;


/*************************************************************************************/
/**
* function name   : RxContinuousTest
* description     : Constructor
*
* @param - None.
*
* @return - nothing
*/
RxContinuousTest::RxContinuousTest( void ) : RxTestBase(), 
   m_Event(RXC_EVENT_NONE_E),
   m_CurrentFrequency(RF_CHANNEL),
   m_FrequencyBand(FREQUENCY_BAND_915),
   m_EnableOutput(true),
   m_CurrentData(false)
{
}

/*************************************************************************************/
/**
* function name   : RxContinuousTest
* description     : Destructor
*
* @param - None.
*
* @return - nothing
*/
RxContinuousTest::~RxContinuousTest( void )
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
bool RxContinuousTest::Initialise( void )
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

   //Initialise the timer for Rx time.
   RxTimer.detach();
   RxTimer.attach_us(Callback<void(void)>(this, &RxContinuousTest::TimerCallback), RXC_READ_INTERVAL_us);

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
void RxContinuousTest::TimerCallback(void)
{
   m_Event = RXC_EVENT_TIMER_E;
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
void RxContinuousTest::StartReceiving(void)
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
bool RxContinuousTest::RunProcess( void )
{
      
   if ( RXC_EVENT_TIMER_E == m_Event )
   {
      m_Event = RXC_EVENT_NONE_E;
      m_CurrentData = Radio.GetContinouousDatabit(false);
      if ( m_EnableOutput )
      {
      debug("%d\r\n", m_CurrentData);
      }
      wait_ms(1000);
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
void RxContinuousTest::OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror)
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
void RxContinuousTest::OnRxTimeout( void )
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
void RxContinuousTest::OnRxError( void )
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
void RxContinuousTest::OnCadDone( bool channelActivityDetected )
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
void RxContinuousTest::OnFhssChangeChannel( uint8_t channelIndex )
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
bool RxContinuousTest::ProcessCommand(const char* pCommand)
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
      else if ( 0 == strncmp(pCommand, "lna", 3) )
      {
         char* pValue = (char*)pCommand + 3;
         if( *pValue == '=' )
         {
            int32_t value;
            pValue++;
            if ( pValue[0] == '0' && pValue[1] == 'x' )
            {
               pValue+=2;
               value = HexTextToUint8(pValue);
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
            else
            {
               value = atoi(pValue);
            }
            
            if ( value == 0 || value == 1 )
            {
               //mask out the reserved bits
               Radio.SetModem( MODEM_FSK );
               m_Lna = Radio.Read(REG_LNA);
               if ( 0 == value )
               {
                  m_Lna &= 0xE0;
               }
               else
               {
                  m_Lna |= 0x03;
               }
               Radio.Write(REG_LNA, m_Lna);
               Radio.SetModem( MODEM_LORA );
               result = true;
            }
         }
      }
      else if ( 0 == strncmp(pCommand, "x", 1) )
      {
         m_EnableOutput = !m_EnableOutput;
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
void RxContinuousTest::PrintHelp(Serial &serialPort)
{
   m_Lna = Radio.Read(REG_LNA);
   serialPort.printf("Currently loaded test = RxContinuous\r\n\n");
   serialPort.printf("Supported commands:\r\n");
   serialPort.printf("frequency=x (%d)\r\n", m_CurrentFrequency);
   serialPort.printf("lna=x (0x%x)  lna=0 to turn off LNA.  lna=1 to turn on\r\n", m_Lna);
   serialPort.printf("x    Enable/Disable output messages\r\n");
   serialPort.printf("reset\r\n");
   serialPort.printf("\r\n");
}
