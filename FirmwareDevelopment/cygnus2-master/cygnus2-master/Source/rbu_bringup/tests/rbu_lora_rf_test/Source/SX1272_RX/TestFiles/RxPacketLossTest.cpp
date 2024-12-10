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
*  File         : PacketLossTest.h
*
*  Description  : Implementation for the Packet loss test.
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include "lptim.h"


/* User Include Files
*************************************************************************************/
#include "RxPacketLossTest.h"

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

#define RF_CHANNEL                                  0

#define PACKET_MODE                                 true
#define USE_CAD                                     0       // 0 : do not use CAD, 1 : use CAD
#define LORA_BANDWIDTH_865                          1         // [0: 125 kHz,
                                                            //  1: 250 kHz,
                                                            //  2: 500 kHz,
                                                            //  3: Reserved]
#define LORA_BANDWIDTH_915                          2         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7       // [SF7..SF12]
#define LORA_CODINGRATE                             1       // [1: 4/5,
                                                            //  2: 4/6,
                                                            //  3: 4/7,
                                                            //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        12      // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         64      // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  true    // required for SF6
#define LORA_FHSS_ENABLED                           false  
#define LORA_NB_SYMB_HOP                            4     
#define LORA_IQ_INVERSION_ON                        false
#define LORA_CRC_ENABLED                            false


#define RX_TIMEOUT_VALUE               150000000   // in us

//This is used to determine whether a Rx Timeout occurs because there was no message or because a received message failed to decode.
#define MESSAGE_ON_AIR_TIME 370 //LPTIM ticks for 12 preamble symbols + 14 char payload
#define MESSAGE_TIMEOUT_THRESHOLD ((RX_TIMEOUT_VALUE / 61) - MESSAGE_ON_AIR_TIME)

#define NUM_PER_SAMPLES 100

/* Type Declarations
*************************************************************************************/
static CRC_HandleTypeDef crc_handle_8b_data_16b_crc;
static bool dropped_message[NUM_PER_SAMPLES];


/*************************************************************************************/
/**
* function name   : PacketLossTest
* description     : Constructor
*
* @param - None.
*
* @return - nothing
*/
PacketLossTest::PacketLossTest( void ) : RxTestBase(), 
   m_Count(0),
   m_Event(PL_EVENT_NONE_E),
   m_LastMessageErrorDetected(false),
   m_Frequency(RF_CHANNEL),
   m_SpreadingFactor(LORA_SPREADING_FACTOR),
   m_CodingRate(LORA_CODINGRATE),
   m_FrequencyBand(FREQUENCY_BAND_915),
   m_LoraCrc(LORA_CRC_ENABLED),
   m_EnableOutput(true),
   m_RxDoneCount(0),
   m_RxTimeoutCount(0),
   m_RxErrorCount(0),
   m_CadDoneCount(0),
   m_ChannelChangeCount(0),
   m_PayloadNumber(0),
   m_LastReceivedPayload(0),
   m_LostMessageCount(0)
{
}

/*************************************************************************************/
/**
* function name   : PacketLossTest
* description     : Destructor
*
* @param - None.
*
* @return - nothing
*/
PacketLossTest::~PacketLossTest( void )
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
bool PacketLossTest::Initialise( void )
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
   m_EventTime = 0;
   m_PreviousEventTime = 0;
   m_MessageInterval = 0;
   uint32_t bandwidth;
   uint32_t frequency;
   
   //initialise timer
   MX_LPTIM1_Init();
  
   if ( FREQUENCY_BAND_915 == m_FrequencyBand )
   {
      Radio.SetChannel( frequencyChannels915[m_Frequency] );
      frequency = frequencyChannels915[m_Frequency];
      bandwidth = LORA_BANDWIDTH_915;
   }
   else 
   {
      Radio.SetChannel( frequencyChannels865[m_Frequency] );
      frequency = frequencyChannels865[m_Frequency];
      bandwidth = LORA_BANDWIDTH_865;
   }
   
   uint8_t payload_length = PL_MESSAGE_LENGTH;
   if( !m_LoraCrc )
   {
      payload_length = PL_MESSAGE_LENGTH_WITH_CRC;
   }
   
   Radio.SetRxConfig( MODEM_LORA, bandwidth, m_SpreadingFactor,
                      m_CodingRate, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, payload_length,
                      m_LoraCrc, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                      LORA_IQ_INVERSION_ON, true, PACKET_MODE );

   
   if ( m_LoraCrc )
   {
      debug( "\r\n\n     SX1272 RX Incrementing Payload Packet Loss Test - LoRa CRC\r\n\n" );
   }
   else
   {
      debug( "\r\n\n     SX1272 RX Incrementing Payload Packet Loss Test - MCU CRC\r\n\n" );
   }
   debug( "Frequency          = %d (%d)\r\n", m_Frequency, frequency);
   debug( "Spreading Factor   = %d\r\n",m_SpreadingFactor);
   debug( "Coding Rate        = 4/%d\r\n\n",(m_CodingRate+4));

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
   
   ResetRadio();
   
   m_EventTime = LPTIM_ReadCounter(&hlptim1);// ************************* This Fn never returns because the timer advances too fast to get two identical readings

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
void PacketLossTest::StartReceiving(void)
{
//   //Reset the LoRa registers for every Rx
//   Radio.SetChannel( frequencyChannels[m_Frequency] );
//   uint8_t payload_length = PL_MESSAGE_LENGTH;
//   if( !m_LoraCrc )
//   {
//      payload_length = PL_MESSAGE_LENGTH_WITH_CRC;
//   }
//   
//   Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, m_SpreadingFactor,
//                      m_CodingRate, 0, LORA_PREAMBLE_LENGTH,
//                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, payload_length,
//                      m_LoraCrc, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
//                      LORA_IQ_INVERSION_ON, true, PACKET_MODE );

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
void PacketLossTest::ResetRadio(void)
{
//   //Reset the LoRa registers for every Rx
//   Radio.SetChannel( frequencyChannels[m_Frequency] );
//   uint8_t payload_length = PL_MESSAGE_LENGTH;
//   if( !m_LoraCrc )
//   {
//      payload_length = PL_MESSAGE_LENGTH_WITH_CRC;
//   }
//   
//   Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, m_SpreadingFactor,
//                      m_CodingRate, 0, LORA_PREAMBLE_LENGTH,
//                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, payload_length,
//                      m_LoraCrc, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
//                      LORA_IQ_INVERSION_ON, true, PACKET_MODE );

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
bool PacketLossTest::RunProcess( void )
{
   switch ( m_Event )
   {
      case PL_EVENT_RXDONE_E:
         {
            ResetRadio();
            m_PayloadNumber = atoi(( const char* )m_Buffer);
            uint32_t per_index = m_PayloadNumber % NUM_PER_SAMPLES;
            
//            if ( (m_PayloadNumber - m_LastReceivedPayload) > 1 )
//            {
//               if ( !m_LastMessageErrorDetected )
//               {
//                  m_LostMessageCount++;
//                  
//               }
//            }
            
            if ( m_LastReceivedPayload > m_PayloadNumber )
            {
               m_LastReceivedPayload = -1;
            }
            
            if ( (m_PayloadNumber - m_LastReceivedPayload) == 1 ) 
            {
               per_index = m_PayloadNumber % NUM_PER_SAMPLES;
               dropped_message[per_index] = false;
               
               //record the time between messages
               if ( m_PreviousEventTime > m_EventTime )
               {
                  //counter wrapped around
                  m_MessageInterval = (m_EventTime + 65535) - m_PreviousEventTime;
               }
               else 
               {
                  m_MessageInterval = m_EventTime - m_PreviousEventTime;
               }
            }
            else 
            {
               uint32_t previous_payload = m_LastReceivedPayload + 1;
               while ( previous_payload != m_PayloadNumber )
               {
//                  if ( m_EnableOutput )
//                  {
//                     debug("%d\tX\r\n", previous_payload);
//                  }
                  previous_payload++;
                  m_LostMessageCount++;
                  per_index = previous_payload % NUM_PER_SAMPLES;
                  dropped_message[per_index] = true;
               }
            }
            
            m_LastReceivedPayload = m_PayloadNumber;
            m_LastMessageErrorDetected = false;
            
            int32_t lost_packet_count = 0;
            for( int32_t index = 0; index < NUM_PER_SAMPLES; index++ )
            {
               if ( dropped_message[index] )
               {
                  lost_packet_count++;
               }
            }
           
            uint32_t lost_messages = m_RxErrorCount + m_LostMessageCount;
//            double packetLoss = 100.0 * (double)(lost_messages) / (double)m_PayloadNumber;
            double packetLoss = 100.0 * (double)(lost_packet_count) / (double)NUM_PER_SAMPLES;
            if ( m_EnableOutput )
            {
               debug("%s\t%d\t%d\t%0.3f%%\r\n",(char*)m_Buffer, m_RssiValue, lost_messages, packetLoss);
               if ( packetLoss > 99.9 )
               {
                  m_LastReceivedPayload = m_PayloadNumber;
                  m_RxErrorCount = 0;
                  m_LostMessageCount = 0;
                  for( int32_t index = 0; index < NUM_PER_SAMPLES; index++ )
                  {
                     dropped_message[index] = false;
                  }
               }
            }
            m_Event = PL_EVENT_NONE_E;
            m_PreviousEventTime = m_EventTime;
         }
         break;
      case PL_EVENT_RXTIMEOUT_E:
      {
         ResetRadio();
         uint32_t timeout_time;
         //calculate the timeout period
         if ( m_PreviousEventTime > m_EventTime )
         {
            //counter wrapped around
            timeout_time = (m_EventTime + 65535) - m_PreviousEventTime;
         }
         else 
         {
            timeout_time = m_EventTime - m_PreviousEventTime;
         }
         
         if ( timeout_time < MESSAGE_TIMEOUT_THRESHOLD )
         {
            //Message failed to decode
            //debug("D\r\n");
         }
         else
         {
            if ( m_EnableOutput )
            {
               debug("T\r\n");
            }
         }

         m_Event = PL_EVENT_NONE_E;
      }
         break;
      case PL_EVENT_RXERROR_E:
         ResetRadio();
         debug("E\r\n");
         m_Event = PL_EVENT_NONE_E;
         break;
      case PL_EVENT_RXCRCERROR_E:
         ResetRadio();
         debug("CRC\r\n");
         m_Event = PL_EVENT_NONE_E;
         break;
      case PL_EVENT_CADDONE_E:
         StartReceiving();
         m_Event = PL_EVENT_NONE_E;
         break;
      case PL_EVENT_CHANNEL_E:
         ResetRadio();
         m_Event = PL_EVENT_NONE_E;
         break;
      default:
         break;
   };
   
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
void PacketLossTest::OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror)
{
   m_EventTime = LPTIM_ReadCounter(&hlptim1);// ************************* This Fn never returns because the timer advances too fast to get two identical readings
   
   Radio.Sleep();

   memset(m_Buffer,0, PL_MESSAGE_LENGTH_WITH_CRC);
   memcpy( m_Buffer, payload, PL_MESSAGE_LENGTH_WITH_CRC );
   m_RssiValue = rssi;
   m_SnrValue = snr;
   m_Ferror = ferror;
   m_FeiValue = FeiVal;

   if ( !m_LoraCrc )
   {
      uint16_t CrcValue =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)&m_Buffer, PL_MESSAGE_LENGTH);
      uint16_t* pMsgCRC = (uint16_t*)&m_Buffer[PL_MESSAGE_LENGTH];
      
      m_CrcValue = *pMsgCRC;
      if ( m_CrcValue != CrcValue )
      {
         m_LastMessageErrorDetected = true;
         m_RxErrorCount++;
         m_Event = PL_EVENT_RXCRCERROR_E;
         debug_if( SHOW_DEBUG, "> OnRxError mcu crc=%x\n\r", CrcValue );
      }
      else
      {
         m_RxDoneCount++;
         debug_if( SHOW_DEBUG, "> OnRxDone crc=%x\n\r", CrcValue );
         m_Event = PL_EVENT_RXDONE_E;
      }
   }
   else
   {
      m_Event = PL_EVENT_RXDONE_E;
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
void PacketLossTest::OnRxTimeout( void )
{
   m_EventTime = LPTIM_ReadCounter(&hlptim1);
   Radio.Sleep();
   m_RxTimeoutCount++;
   m_Event = PL_EVENT_RXTIMEOUT_E;
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
void PacketLossTest::OnRxError( void )
{
   m_EventTime = LPTIM_ReadCounter(&hlptim1);
   Radio.Sleep();
   m_RxErrorCount++;
   m_Event = PL_EVENT_RXERROR_E;
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
void PacketLossTest::OnCadDone( bool channelActivityDetected )
{
   m_EventTime = LPTIM_ReadCounter(&hlptim1);
   Radio.Sleep();
   m_CadDoneCount++;
   m_Event = PL_EVENT_CADDONE_E;
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
void PacketLossTest::OnFhssChangeChannel( uint8_t channelIndex )
{
   m_EventTime = LPTIM_ReadCounter(&hlptim1);
   Radio.Sleep();
   m_ChannelChangeCount++;
   m_Event = PL_EVENT_CHANNEL_E;
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
bool PacketLossTest::ProcessCommand(const char* pCommand)
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
            else
            {
               value = atoi(pValue);
            }
            
            if ( value == 0 || value == 1 )
            {
               //mask out the reserved bits
               m_Lna = Radio.Read(REG_LNA);
               if ( 0 == value )
               {
                  m_Lna &= 0xE0;
               }
               else
               {
                  m_Lna |= 0x03;
               }
               Radio.SetModem( MODEM_FSK );
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
void PacketLossTest::PrintHelp(Serial &serialPort)
{
   m_Lna = Radio.Read(REG_LNA);
   serialPort.printf("Currently loaded test = PacketLossTest\r\n\n");
   serialPort.printf("Supported commands:\r\n");
   serialPort.printf("frequency=x (%d)\r\n", m_Frequency);
   serialPort.printf("Band=x (%d)  (865 or 915)\r\n", m_FrequencyBand == FREQUENCY_BAND_865 ? 865:915 );
   serialPort.printf("sf=x (%d)\r\n", m_SpreadingFactor);
   serialPort.printf("cr=x (%d)  (1=4/5, 2=4/6, 3=4/7, 4=4/8\r\n", m_CodingRate);
   serialPort.printf("crc=s (%s) (lora or mcu)\r\n", m_LoraCrc ? "lora":"mcu");
   serialPort.printf("lna=x (0x%x)  lna=0 to turn off LNA.  lna=1 to turn on\r\n", m_Lna);
   serialPort.printf("x    Enable/Disable output messages\r\n");
   serialPort.printf("reset\r\n");
   serialPort.printf("\r\n\n\n");
}
