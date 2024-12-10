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
*  File         : RxOriginalPacketTest.cpp
*
*  Description  : Implementation of the Packet Loss Test.
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/
#include "RxOriginalPacketTest.h"

/* User Definitions
*************************************************************************************/

#define SHOW_DEBUG 0

#define NUM_FREQUENCY_CHANS 10
uint32_t const frequencyChannels[NUM_FREQUENCY_CHANS] = {865150000, 865450000, 865750000, 866050000, 866350000,
                                                         866650000, 866950000, 867250000, 867550000, 867850000};

#define RF_CHANNEL                                  4

#define PACKET_MODE                                 true
#define USE_CAD                                     0       // 0 : do not use CAD, 1 : use CAD
#define LORA_BANDWIDTH                              1       // [0: 125 kHz,
                                                            //  1: 250 kHz,
                                                            //  2: 500 kHz,
                                                            //  3: Reserved]
#define LORA_SPREADING_FACTOR                       6       // [SF7..SF12]
#define LORA_CODINGRATE                             1       // [1: 4/5,
                                                            //  2: 4/6,
                                                            //  3: 4/7,
                                                            //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        12      // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         16      // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  true    // required for SF6
#define LORA_FHSS_ENABLED                           false  
#define LORA_NB_SYMB_HOP                            4     
#define LORA_IQ_INVERSION_ON                        false
#define LORA_CRC_ENABLED                            false


#define RX_TIMEOUT_VALUE               3500000   // in us

#define TEST_TIME_LIMIT                60 //seconds
#define MESSAGE_BATCH_SIZE             200

/* Type Declarations
*************************************************************************************/
static CRC_HandleTypeDef crc_handle_8b_data_16b_crc;
static char MainMsg[] = "ONE";
static char InterfererMsg[] = "TWO";

/*************************************************************************************/
/**
* function name   : RxOriginalPacketTest
* description     : Constructor
*
* @param - None.
*
* @return - nothing
*/
RxOriginalPacketTest::RxOriginalPacketTest( void ) : RxTestBase(), 
   m_Count(0),
   m_Event(OPT_EVENT_NONE_E),
   m_LastMessageErrorDetected(false),
   m_Frequency(RF_CHANNEL),
   m_SpreadingFactor(LORA_SPREADING_FACTOR),
   m_CodingRate(LORA_CODINGRATE),
   m_LoraCrc(LORA_CRC_ENABLED),
   m_MessageBatchSize(MESSAGE_BATCH_SIZE),
   m_RxDoneCount(0),
   m_RxTimeoutCount(0),
   m_RxErrorCount(0),
   m_CadDoneCount(0),
   m_ChannelChangeCount(0),
   m_PayloadNumber(0),
   m_LastReceivedPayload(0),
   m_LostMessageCount(0),
   m_MainMsgCount(0),
   m_InterfererMsgCount(0),
   m_UnknownMsgCount(0),
   m_RssiValue(0),
   m_SnrValue(0),
   m_Ferror(0),
   m_FeiValue(0),
   m_CadDetectedCount(0),
   m_PllLockCount(0)
{
}

/*************************************************************************************/
/**
* function name   : RxOriginalPacketTest
* description     : Destructor
*
* @param - None.
*
* @return - nothing
*/
RxOriginalPacketTest::~RxOriginalPacketTest( void )
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
bool RxOriginalPacketTest::Initialise( void )
{
   bool status = true;
   
   m_RxDoneCount = 0;
   m_RxTimeoutCount = 0;
   m_RxErrorCount = 0;
   m_CadDoneCount = 0;
   m_ChannelChangeCount = 0;
   m_PayloadNumber = 0;
   m_LastReceivedPayload = 0;
   m_LostMessageCount = 0;
   m_MainMsgCount = 0;
   m_InterfererMsgCount = 0;
   m_UnknownMsgCount = 0;
   m_CadDetectedCount = 0;
   m_PllLockCount = 0;
  
   Radio.SetChannel( frequencyChannels[m_Frequency] );
   uint8_t payload_length = OPT_MESSAGE_LENGTH;
   if( !m_LoraCrc )
   {
      payload_length = OPT_MESSAGE_LENGTH_WITH_CRC;
   }
   
   Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, m_SpreadingFactor,
                      m_CodingRate, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, payload_length,
                      m_LoraCrc, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                      LORA_IQ_INVERSION_ON, true, PACKET_MODE );


   if ( m_LoraCrc )
   {
      debug( "\r\n\n     SX1272 RX Original 200 Packet Test - LoRa CRC\r\n\n" );
   }
   else
   {
      debug( "\r\n\n     SX1272 RX Original 200 Packet Test - MCU CRC\r\n\n" );
   }
   
   debug( "Frequency          = %d (%d)\r\n", m_Frequency, frequencyChannels[m_Frequency]);
   debug( "Spreading Factor   = %d\r\n",m_SpreadingFactor);
   debug( "Coding Rate        = 4/%d\r\n",(m_CodingRate+4));
   debug( "Message Batch Size = %d\r\n\n", m_MessageBatchSize);
   
   if ( !m_LoraCrc )
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
   
   set_time(0);
  
   m_Event = OPT_EVENT_START_E;
   
   return status;
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
void RxOriginalPacketTest::StartReceiving(void)
{
   Radio.Rx( RX_TIMEOUT_VALUE );
}

/*************************************************************************************/
/**
* function name   : ResetRadio
* description     : Restart the reciever
*
* @param - None.
*
* @return - None.
*/
void RxOriginalPacketTest::ResetRadio(void)
{
   if (USE_CAD == 1)
   {
      Radio.StartCad();
   }
   else
   {
      Radio.Rx( RX_TIMEOUT_VALUE );
   }
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
bool RxOriginalPacketTest::RunProcess( void )
{

   switch ( m_Event )
   {
      case OPT_EVENT_RXDONE_E:
      {
         ResetRadio();
         if( strncmp( ( const char* )m_Buffer, ( const char* )MainMsg, 3 ) == 0 )  
         {
             debug_if( SHOW_DEBUG, "Main Message Received\r\n" );
             m_MainMsgCount++;
         }
         else if( strncmp( ( const char* )m_Buffer, ( const char* )InterfererMsg, 3 ) == 0 )  
         {
             debug_if( SHOW_DEBUG, "Interferer Message Received\r\n" );
             m_InterfererMsgCount++;
         }
         else
         {
            debug_if( SHOW_DEBUG, "Unknown Message received\r\n" );
            m_UnknownMsgCount++;
            m_PayloadNumber = atoi(( const char* )m_Buffer);
            if ( (m_PayloadNumber - m_LastReceivedPayload) > 1 )
            {
               m_LostMessageCount++;
            }
            m_LastReceivedPayload = m_PayloadNumber;
         }
         debug( "*RX * RX1:%u, RX2:%u, RXU:%u, RXT:%u, RXE:%u, CAD:%u, CADD:%u, LOCK:%u, RSSI:%i, SNR:%i\r\n", 
            m_MainMsgCount, m_InterfererMsgCount, m_UnknownMsgCount, m_RxTimeoutCount, m_RxErrorCount, m_CadDoneCount, 
            m_CadDetectedCount, m_PllLockCount, m_RssiValue, m_SnrValue);
         
         m_Event = OPT_EVENT_NONE_E;
      }
         break;
      case OPT_EVENT_RXTIMEOUT_E:
         ResetRadio();
         m_Event = OPT_EVENT_NONE_E;
         break;
      case OPT_EVENT_RXERROR_E:
         ResetRadio();
         m_Event = OPT_EVENT_NONE_E;
         break;
      case OPT_EVENT_CADDONE_E:
         StartReceiving();
         m_Event = OPT_EVENT_NONE_E;
         break;
      case OPT_EVENT_CHANNEL_E:
         ResetRadio();
         m_Event = OPT_EVENT_NONE_E;
         break;
      case OPT_EVENT_START_E:
         ResetRadio();
         m_Event = OPT_EVENT_NONE_E;
         break;
      default:
         break;
   };
   
   /* check for end of message batch or timeout */
   m_CurrentTimeSecs = time(NULL);
   uint32_t RxPackets = m_MainMsgCount + m_InterfererMsgCount + m_UnknownMsgCount;
   uint32_t total_message_count = RxPackets + m_RxErrorCount + m_LostMessageCount;
   if ((m_CurrentTimeSecs == TEST_TIME_LIMIT) || (total_message_count >= m_MessageBatchSize)) 
   {
      float good_packets = 100*(float)RxPackets/(float)(m_MessageBatchSize);
      float lost_packets = 100*(float)m_LostMessageCount/(float)m_MessageBatchSize;
      float crc_fail_packets = 100*(float)m_RxErrorCount/(float)(m_MessageBatchSize);

      float bad_packets = lost_packets + crc_fail_packets;
      debug( "\r\n\nGood:%0.1f%%, Bad:%0.1f%%, Lost:%0.1f%%, CRC:%0.1f%%\r\n\n", good_packets, bad_packets, lost_packets, crc_fail_packets);
      /* end the test */
      return false;
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
void RxOriginalPacketTest::OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror)
{
   Radio.Sleep();
   memset(m_Buffer,0, OPT_MESSAGE_LENGTH_WITH_CRC);
   memcpy( m_Buffer, payload, OPT_MESSAGE_LENGTH_WITH_CRC );
   m_RssiValue = rssi;
   m_SnrValue = snr;
   m_Ferror = ferror;
   m_FeiValue = FeiVal;

   if ( !m_LoraCrc )
   {
      uint16_t CrcValue =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)&m_Buffer, OPT_MESSAGE_LENGTH);
      uint16_t* pMsgCRC = (uint16_t*)&m_Buffer[OPT_MESSAGE_LENGTH];
      
      m_CrcValue = *pMsgCRC;
      if ( m_CrcValue != CrcValue )
      {
         m_LastMessageErrorDetected = true;
         m_RxErrorCount++;
         m_Event = OPT_EVENT_RXERROR_E;
         debug_if( SHOW_DEBUG, "> OnRxError mcu crc=%x\n\r", CrcValue );
      }
      else
      {
         m_RxDoneCount++;
         debug_if( SHOW_DEBUG, "> OnRxDone crc=%x\n\r", CrcValue );
         m_Event = OPT_EVENT_RXDONE_E;
      }
   }
   else
   {
      m_Event = OPT_EVENT_RXDONE_E;
   }
   
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
void RxOriginalPacketTest::OnRxTimeout( void )
{
   Radio.Sleep();
   m_RxTimeoutCount++;
   m_Event = OPT_EVENT_RXTIMEOUT_E;
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
void RxOriginalPacketTest::OnRxError( void )
{
   Radio.Sleep();
   m_RxErrorCount++;
   m_Event = OPT_EVENT_RXERROR_E;
   m_LastMessageErrorDetected = true;
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
void RxOriginalPacketTest::OnCadDone( bool channelActivityDetected )
{
   Radio.Sleep();
   m_CadDoneCount++;
   m_Event = OPT_EVENT_CADDONE_E;
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
void RxOriginalPacketTest::OnFhssChangeChannel( uint8_t channelIndex )
{
   Radio.Sleep();
   m_ChannelChangeCount++;
   m_Event = OPT_EVENT_CHANNEL_E;
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
bool RxOriginalPacketTest::ProcessCommand(const char* pCommand)
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
      else if ( 0 == strncmp(pCommand, "sf", 2) )
      {
         char* pValue = (char*)pCommand + 2;
         if( *pValue == '=' )
         {
            pValue++;
            int32_t value = atoi(pValue);
            if ( value > 5 && value < 9 )
            {
               m_SpreadingFactor = value;
               Radio.Sleep();
               Initialise();
               result = true;
            }
         }
      }
      else if ( 0 == strncmp(pCommand, "crc", 3 ) )
      {
         char* pValue = (char*)pCommand + 3;
         if( *pValue == '=' )
         {
            pValue++;
            if ( 0 == strncmp(pValue, "mcu", 3 ) )
            {
               m_LoraCrc = false;
               Radio.Sleep();
               Initialise();
               result = true;
            }
            else if ( 0 == strncmp(pValue, "lora", 4 ) )
            {
               m_LoraCrc = true;
               Radio.Sleep();
               Initialise();
               result = true;
            }
         }
      }
      else if ( 0 == strncmp(pCommand, "cr", 2) )
      {
         char* pValue = (char*)pCommand + 2;
         if( *pValue == '=' )
         {
            pValue++;
            int32_t value = atoi(pValue);
            if ( value > 0 && value < 5 )
            {
               m_CodingRate = value;
               Radio.Sleep();
               Initialise();
               result = true;
            }
         }
      }
      else if ( 0 == strncmp(pCommand, "count", 5) )
      {
         char* pValue = (char*)pCommand + 5;
         if( *pValue == '=' )
         {
            pValue++;
            int32_t value = atoi(pValue);
            if ( value > 0 )
            {
               m_MessageBatchSize = value;
               Radio.Sleep();
               Initialise();
               result = true;
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
void RxOriginalPacketTest::PrintHelp(Serial &serialPort)
{
   serialPort.printf("Currently loaded test = OriginalPacketTest\r\n\n");
   serialPort.printf("Supported commands:\r\n");
   serialPort.printf("frequency=x (%d)\r\n", m_Frequency);
   serialPort.printf("sf=x (%d)\r\n", m_SpreadingFactor);
   serialPort.printf("cr=x (%d)     (1=4/5, 2=4/6, 3=4/7, 4=4/8)\r\n", m_CodingRate);
   serialPort.printf("crc=s (%s)    (lora or mcu)\r\n", m_LoraCrc ? "lora":"mcu");
   serialPort.printf("count=x (%d)  (Number of messages in test)\r\n", m_MessageBatchSize);
   serialPort.printf("reset\r\n");
   serialPort.printf("\r\n\n");
}
