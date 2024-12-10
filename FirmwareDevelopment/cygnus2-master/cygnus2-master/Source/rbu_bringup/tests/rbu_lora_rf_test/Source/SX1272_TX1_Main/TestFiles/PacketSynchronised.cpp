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
*  File         : PacketSynchronised.cpp
*
*  Description  : Implementation for the test that sends a packet with an incrementing
*                 number in the payload, represented as an ascii string with an
*                 accurately timed interval to enable the receiving side to
*                 synchronise its receiver.
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include "mbed.h"


/* User Include Files
*************************************************************************************/
#include "PacketSynchronised.h"

/* User Definitions
*************************************************************************************/
#define MESSAGE_INTERVAL_us 100000

#define NUM_FREQUENCY_CHANS 10
uint32_t const frequencyChannels[NUM_FREQUENCY_CHANS] = {865150000, 865450000, 865750000, 866050000, 866350000,
                                                         866650000, 866950000, 867250000, 867550000, 867850000};

#define RF_CHANNEL                                       4
#define TX_OUTPUT_POWER                                  5

const uint32_t RF_FREQUENCY = frequencyChannels[RF_CHANNEL];

#define LORA_BANDWIDTH                              0         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       6         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        12        // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         16        // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  true      // required for SF6
#define LORA_FHSS_ENABLED                           false  
#define LORA_NB_SYMB_HOP                            4     
#define LORA_IQ_INVERSION_ON                        false
#define LORA_CRC_ENABLED                            false

#define PS_MESSAGE_LENGTH 12
#define PS_MESSAGE_LENGTH_WITH_CRC  (PS_MESSAGE_LENGTH + 2)
#define PS_MESSAGE_DELAY  5
/* Type Declarations
*************************************************************************************/
static CRC_HandleTypeDef crc_handle_8b_data_16b_crc;


/* Global Variables
*************************************************************************************/
Ticker MessageTimer;

/*************************************************************************************/
/**
* function name   : PacketSynchronisedTest
* description     : Constructor
*
* @param - None.
*
* @return - nothing
*/
PacketSynchronisedTest::PacketSynchronisedTest( void ) : TestBase(), 
   m_Count(0),
   m_Event(PS_EVENT_NONE_E),
   m_TxDoneCount(0),
   m_TxTimeoutCount(0),
   m_ChannelChangeCount(0)
{
}

/*************************************************************************************/
/**
* function name   : PacketSynchronisedTest
* description     : Destructor
*
* @param - None.
*
* @return - nothing
*/
PacketSynchronisedTest::~PacketSynchronisedTest( void )
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
bool PacketSynchronisedTest::Initialise( void )
{
   bool status = true;
   
   m_TxDoneCount = 0;
   m_TxTimeoutCount = 0;
   m_ChannelChangeCount = 0;
  
   Radio.SetChannel( RF_FREQUENCY );
   Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                   LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                   LORA_IQ_INVERSION_ON, 2000000, true );

   if ( LORA_CRC_ENABLED )
   {
      debug( "\n\n\r     SX1272 TX Incrementing Payload Packet Continuously - LoRa CRC\n\n\r" );
   }
   else
   {
      debug( "\n\n\r     SX1272 TX Incrementing Payload Packet Continuously - MCU CRC\n\n\r" );
   }
   debug( "Tx Power  = %d\r\n", TX_OUTPUT_POWER);
   debug( "Frequency          = %d (%d)\r\n", RF_CHANNEL, RF_FREQUENCY);
   debug( "Spreading Factor   = %d\r\n",LORA_SPREADING_FACTOR);
   debug( "Coding Rate        = 4/%d\r\n",(LORA_CODINGRATE+4));

   if ( !LORA_CRC_ENABLED )
   {
      /* Initialise the CRC module */
      crc_handle_8b_data_16b_crc.Instance                     = CRC;
      crc_handle_8b_data_16b_crc.Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_DISABLE;
      crc_handle_8b_data_16b_crc.Init.GeneratingPolynomial    = 0x8005U;
      crc_handle_8b_data_16b_crc.Init.CRCLength               = CRC_POLYLENGTH_16B;
      crc_handle_8b_data_16b_crc.Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_DISABLE;
      crc_handle_8b_data_16b_crc.Init.InitValue               = 0xFFFFU;
      crc_handle_8b_data_16b_crc.Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_NONE;
      crc_handle_8b_data_16b_crc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
      crc_handle_8b_data_16b_crc.InputDataFormat              = CRC_INPUTDATA_FORMAT_BYTES;

         /* Enable the CRC clock */
      __HAL_RCC_CRC_CLK_ENABLE();
      /* Configure the CRC module */
      if (HAL_OK != HAL_CRC_Init(&crc_handle_8b_data_16b_crc))
      {
         debug( "CRC module initialisation failed!\n\r", NULL );
         wait( 1 );
         status = false;
      }
   }
   
   //Initialise the timer for Tx time.
   MessageTimer.attach_us(Callback<void(void)>(this, &PacketSynchronisedTest::TimerCallback), MESSAGE_INTERVAL_us);
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
void PacketSynchronisedTest::TimerCallback(void)
{
   m_Event = PS_EVENT_TIMER_E;
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
bool PacketSynchronisedTest::RunProcess( void )
{
   static char buffer[PS_MESSAGE_LENGTH_WITH_CRC];
   
   switch ( m_Event )
   {
      case PS_EVENT_TIMER_E:
         m_Count++;
         memset(buffer, 0, PS_MESSAGE_LENGTH);
         
         sprintf(buffer,"%d", m_Count);
         if ( !LORA_CRC_ENABLED )
         {
            
            buffer[PS_MESSAGE_LENGTH - 1] = 0;
            uint16_t CrcValue =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)&buffer, PS_MESSAGE_LENGTH);
            uint16_t* pMsgCRC = (uint16_t*)&buffer[PS_MESSAGE_LENGTH];
            *pMsgCRC = CrcValue;
            Radio.Send( (uint8_t*)buffer, PS_MESSAGE_LENGTH_WITH_CRC );
            sprintf(buffer,"%d,\tcrc=%x", m_Count, CrcValue);
         }
         else
         {
            Radio.Send( (uint8_t*)buffer, PS_MESSAGE_LENGTH );
         }
         debug("%s\r\n", buffer);
         m_Event = PS_EVENT_NONE_E;
         break;
      case PS_EVENT_TXDONE_E:
         break;
      case PS_EVENT_TXTIMEOUT_E:
         // Nothing to do.
         break;
      case PS_EVENT_CHANNEL_E:
         // Nothing to do.
         break;
      default:
         // Nothing to do.
         break;
   };

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
void PacketSynchronisedTest::OnTxDone( void )
{
   Radio.Sleep();
   m_TxDoneCount++;
   m_Event = PS_EVENT_TXDONE_E;
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
void PacketSynchronisedTest::OnTxTimeout( void )
{
   Radio.Sleep();
   m_TxTimeoutCount++;
   m_Event = PS_EVENT_TXTIMEOUT_E;
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
void PacketSynchronisedTest::OnFhssChangeChannel( uint8_t channelIndex )
{
   Radio.Sleep();
   m_ChannelChangeCount++;
   m_Event = PS_EVENT_CHANNEL_E;
}


