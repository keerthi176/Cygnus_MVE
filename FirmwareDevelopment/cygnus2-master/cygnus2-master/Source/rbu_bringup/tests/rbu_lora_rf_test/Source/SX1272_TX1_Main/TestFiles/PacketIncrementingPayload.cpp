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
*  File         : PacketIncrementingPayload.cpp
*
*  Description  : Implementation of the test that sends a packet with an incrementing
*                 number in the payload, represented as an ascii string.
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/
#include "PacketIncrementingPayload.h"

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

#define RF_CHANNEL                                       0
#define TX_OUTPUT_POWER                                  7


#define LORA_BANDWIDTH_865                          1         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_BANDWIDTH_915                          2         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
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

#define PIP_MESSAGE_LENGTH 5
#define PIP_MESSAGE_LENGTH_WITH_CRC  (PIP_MESSAGE_LENGTH + 2)
#define PIP_DEFAULT_MESSAGE_DELAY  100


/* Type Declarations
*************************************************************************************/
static CRC_HandleTypeDef crc_handle_8b_data_16b_crc;

/*************************************************************************************/
/**
* function name   : PacketIncrementingPayload
* description     : Constructor
*
* @param - None.
*
* @return - nothing
*/
PacketIncrementingPayload::PacketIncrementingPayload( void ) : TestBase(), 
   m_Count(0),
   m_Event(PIP_EVENT_NONE_E),
   m_Frequency(RF_CHANNEL),
   m_TxPower(TX_OUTPUT_POWER),
   m_SpreadingFactor(LORA_SPREADING_FACTOR),
   m_CodingRate(LORA_CODINGRATE),
   m_FrequencyBand(FREQUENCY_BAND_915),
   m_LoraCrc(LORA_CRC_ENABLED),
   m_MessageInterval(PIP_DEFAULT_MESSAGE_DELAY),
#ifdef CONTROL_PANEL
   m_EnableOutput(false),
#else
   m_EnableOutput(true),
#endif
   m_TxDoneCount(0),
   m_TxTimeoutCount(0),
   m_ChannelChangeCount(0)
{
}

/*************************************************************************************/
/**
* function name   : PacketIncrementingPayload
* description     : Destructor
*
* @param - None.
*
* @return - nothing
*/
PacketIncrementingPayload::~PacketIncrementingPayload( void )
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
bool PacketIncrementingPayload::Initialise( void )
{
   bool status = true;
   m_Count = 0;
   m_TxDoneCount = 0;
   m_TxTimeoutCount = 0;
   m_ChannelChangeCount = 0;
   uint32_t bandwidth;
   uint32_t frequency;
  
  
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
   Radio.SetTxConfig( MODEM_LORA, m_TxPower, 0, bandwidth,
                   m_SpreadingFactor, m_CodingRate,
                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                   m_LoraCrc, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                   LORA_IQ_INVERSION_ON, 2000000, true );

   if ( m_LoraCrc )
   {
      debug( "\n\n\r     SX1272 TX Incrementing Payload Packet Continuously - LoRa CRC\n\n\r" );
   }
   else
   {
      debug( "\n\n\r     SX1272 TX Incrementing Payload Packet Continuously - MCU CRC\n\n\r" );
   }
   debug( "Tx Power           = %d\r\n", m_TxPower);
   debug( "Frequency          = %d (%d)\r\n", m_Frequency, frequency);
   debug( "Spreading Factor   = %d\r\n",m_SpreadingFactor);
   debug( "Coding Rate        = 4/%d\r\n",(m_CodingRate+4));

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
   
   m_Event = PIP_EVENT_TXDONE_E;
   
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
bool PacketIncrementingPayload::RunProcess( void )
{
   static char buffer[PIP_MESSAGE_LENGTH_WITH_CRC];
   
   switch ( m_Event )
   {
      case PIP_EVENT_TXDONE_E:
         if ( 0 < m_MessageInterval )
         {
            wait_ms(m_MessageInterval);
         }
         
         m_Count++;
         if ( 9999 < m_Count )
         {
            m_Count = 0;
         }
         
//         //Introduce missed packets for Rx test purposes
//         if ( 500 > m_Count )
//         {
//            if ( 100 < m_Count )
//            {
//               if ( 0 == (m_Count % 15) )
//               {
//                  m_Count++;
//               }
//            }
//            if ( 200 < m_Count )
//            {
//               if ( 0 == (m_Count % 10) )
//               {
//                  m_Count++;
//               }
//            }
//            if ( 300 < m_Count )
//            {
//               if ( 0 == (m_Count % 20) )
//               {
//                  m_Count++;
//               }
//            }
//         }
         
         memset(buffer, 0, PIP_MESSAGE_LENGTH);
         
         sprintf(buffer,"%d", m_Count);


//         //Reload the LoRa registers for each msg
//         Radio.SetChannel( frequencyChannels[m_Frequency] );
//         Radio.SetTxConfig( MODEM_LORA, m_TxPower, 0, LORA_BANDWIDTH,
//                         m_SpreadingFactor, m_CodingRate,
//                         LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
//                         m_LoraCrc, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
//                         LORA_IQ_INVERSION_ON, 2000000, true );


         if ( !m_LoraCrc )
         {
            
            buffer[PIP_MESSAGE_LENGTH - 1] = 0;
            uint16_t CrcValue =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)&buffer, PIP_MESSAGE_LENGTH);
            uint16_t* pMsgCRC = (uint16_t*)&buffer[PIP_MESSAGE_LENGTH];
            *pMsgCRC = CrcValue;
            Radio.Send( (uint8_t*)buffer, PIP_MESSAGE_LENGTH_WITH_CRC );
            //sprintf(buffer,"%d,\tcrc=%x", m_Count, CrcValue);
         }
         else
         {
            Radio.Send( (uint8_t*)buffer, PIP_MESSAGE_LENGTH );
         }
         if ( m_EnableOutput )
         {
            debug("%s\r\n", buffer);
         }
         m_Event = PIP_EVENT_NONE_E;
         break;
      case PIP_EVENT_TXTIMEOUT_E:
         // Nothing to do.  Schedule another transmission.
         m_Event = PIP_EVENT_TXDONE_E;
         break;
      case PIP_EVENT_CHANNEL_E:
         // Nothing to do.  Schedule another transmission.
         m_Event = PIP_EVENT_TXDONE_E;
         break;
      default:
         // Nothing to do.  Schedule another transmission.
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
void PacketIncrementingPayload::OnTxDone( void )
{
   Radio.Sleep();
   m_TxDoneCount++;
   m_Event = PIP_EVENT_TXDONE_E;
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
void PacketIncrementingPayload::OnTxTimeout( void )
{
   Radio.Sleep();
   m_TxTimeoutCount++;
   m_Event = PIP_EVENT_TXTIMEOUT_E;
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
void PacketIncrementingPayload::OnFhssChangeChannel( uint8_t channelIndex )
{
   Radio.Sleep();
   m_ChannelChangeCount++;
   m_Event = PIP_EVENT_CHANNEL_E;
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
bool PacketIncrementingPayload::ProcessCommand(const char* pCommand)
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
      else if ( 0 == strncmp(pCommand, "delay", 5) )
      {
         char* pValue = (char*)pCommand + 5;
         if( *pValue == '=' )
         {
            pValue++;
            int32_t value = atoi(pValue);
            m_MessageInterval = value;
            Radio.Sleep();
            Initialise();
            result = true;
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
      else if ( 0 == strncmp(pCommand, "x", 1) )
      {
         m_EnableOutput = !m_EnableOutput;
         result = true;
      }
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
void PacketIncrementingPayload::PrintHelp(Serial &serialPort)
{
   serialPort.printf("Currently loaded test = TX Incrementing Payload\r\n\n");
   serialPort.printf("Supported commands:\r\n");
   serialPort.printf("frequency=x (%d)\r\n", m_Frequency);
   serialPort.printf("power=x (%d)\r\n", m_TxPower);
   serialPort.printf("sf=x (%d)\r\n", m_SpreadingFactor);
   serialPort.printf("cr=x (%d)  (1=4/5, 2=4/6, 3=4/7, 4=4/8\r\n", m_CodingRate);
   serialPort.printf("Band=x (%d)  (865 or 915)\r\n", m_FrequencyBand == FREQUENCY_BAND_865 ? 865:915 );
   serialPort.printf("crc=s (%s) (lora or mcu)\r\n", m_LoraCrc ? "lora":"mcu");
   serialPort.printf("delay=x (%d) milliseconds message interval\r\n", m_MessageInterval);
   serialPort.printf("x    Enable/Disable output messages\r\n");
   serialPort.printf("reset\r\n");
   
   serialPort.printf("\r\n\n\n");
}


