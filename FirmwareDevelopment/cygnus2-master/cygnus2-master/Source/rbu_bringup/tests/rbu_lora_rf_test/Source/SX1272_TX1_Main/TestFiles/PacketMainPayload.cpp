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
*  File         : PacketMainPayload.cpp
*
*  Description  : Header for the test that sends a fixed number of packets with the
*                 payload set to "ONE1".
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/
#include "PacketMainPayload.h"

/* User Definitions
*************************************************************************************/
#define SHOW_DEBUG
#define NUM_FREQUENCY_CHANS 10
uint32_t const frequencyChannels[NUM_FREQUENCY_CHANS] = {865150000, 865450000, 865750000, 866050000, 866350000,
                                                         866650000, 866950000, 867250000, 867550000, 867850000};

#define RF_CHANNEL                                  4
#define TX_OUTPUT_POWER                             10

#define LORA_BANDWIDTH                              1         // [0: 125 kHz,
                                                            //  1: 250 kHz,
                                                            //  2: 500 kHz,
                                                            //  3: Reserved]
#define LORA_SPREADING_FACTOR                       6         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                            //  2: 4/6,
                                                            //  3: 4/7,
                                                            //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        12         // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         16         // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  true      // required for SF6
#define LORA_FHSS_ENABLED                           false  
#define LORA_NB_SYMB_HOP                            4     
#define LORA_IQ_INVERSION_ON                        false
#define LORA_CRC_ENABLED                            false

#define PMP_MESSAGE_LENGTH 16
#define PMP_MESSAGE_LENGTH_WITH_CRC PMP_MESSAGE_LENGTH+2
#define PMP_MESSAGE_COUNT  200
#define PMP_MESSAGE_DELAY  100

/* Type Declarations
*************************************************************************************/
static CRC_HandleTypeDef crc_handle_8b_data_16b_crc;

/*************************************************************************************/
/**
* function name   : PacketMainPayload
* description     : Constructor
*
* @param - None.
*
* @return - nothing
*/
PacketMainPayload::PacketMainPayload( void ) : TestBase(), 
   m_Count(0),
   m_MaxMessages(PMP_MESSAGE_COUNT),
   m_Event(PMP_EVENT_NONE_E),
   m_Frequency(RF_CHANNEL),
   m_TxPower(TX_OUTPUT_POWER),
   m_SpreadingFactor(LORA_SPREADING_FACTOR),
   m_CodingRate(LORA_CODINGRATE),
   m_LoraCrc(LORA_CRC_ENABLED),
   m_EnableOutput(true),
   m_TxDoneCount(0),
   m_TxTimeoutCount(0),
   m_ChannelChangeCount(0)
{
}

/*************************************************************************************/
/**
* function name   : PacketMainPayload
* description     : Destructor
*
* @param - None.
*
* @return - nothing
*/
PacketMainPayload::~PacketMainPayload( void )
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
bool PacketMainPayload::Initialise( void )
{
   bool status = true;
   m_Count = 0;
   m_TxDoneCount = 0;
   m_TxTimeoutCount = 0;
   m_ChannelChangeCount = 0;
  
   ant_tx_nrx = 0;
   ant_rx_ntx = 1;

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

   Radio.SetChannel( frequencyChannels[m_Frequency] );
   Radio.SetTxConfig( MODEM_LORA, m_TxPower, 0, LORA_BANDWIDTH,
                   m_SpreadingFactor, m_CodingRate,
                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                   m_LoraCrc, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                   LORA_IQ_INVERSION_ON, 2000000, true );

   debug( "\n\n\r     SX1272 TX Main Packet (ONE) %d Messages \n\n\r", m_MaxMessages );
   debug( "Frequency = %d (%d)\r\n", RF_CHANNEL, frequencyChannels[m_Frequency]);
   debug( "Tx Power  = %d\r\n", m_TxPower);

   m_Event = PMP_EVENT_TXDONE_E;
   
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
bool PacketMainPayload::RunProcess( void )
{
   static char buffer[PMP_MESSAGE_LENGTH_WITH_CRC];
   
   switch ( m_Event )
   {
      case PMP_EVENT_TXDONE_E:
         m_Count++;
         wait_ms(PMP_MESSAGE_DELAY);
         memset( buffer, 0, PMP_MESSAGE_LENGTH_WITH_CRC);
         sprintf( buffer, "ONE%d", m_Count);
         if ( !m_LoraCrc )
         {
            buffer[PMP_MESSAGE_LENGTH - 1] = 0;
            uint16_t CrcValue =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)&buffer, PMP_MESSAGE_LENGTH);
            uint16_t* pMsgCRC = (uint16_t*)&buffer[PMP_MESSAGE_LENGTH];
            *pMsgCRC = CrcValue;
            Radio.Send( (uint8_t*)buffer, PMP_MESSAGE_LENGTH_WITH_CRC );
            if ( m_EnableOutput)
            {
               debug("%d\t%s crc=%x\r\n", m_Count, buffer, CrcValue);
            }
         }
         else
         {
            Radio.Send( (uint8_t*)buffer, PMP_MESSAGE_LENGTH );
            if ( m_EnableOutput )
            {
               debug("%d\t%s\r\n", m_Count, buffer);
            }
         }

         break;
      case PMP_EVENT_TXTIMEOUT_E:
         // Nothing to do.  Schedule another transmission.
         m_Event = PMP_EVENT_TXDONE_E;
         break;
      case PMP_EVENT_CHANNEL_E:
         // Nothing to do.  Schedule another transmission.
         m_Event = PMP_EVENT_TXDONE_E;
         break;
      default:
         // Nothing to do.
         break;
   };

   return ( m_Count < m_MaxMessages );
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
void PacketMainPayload::OnTxDone( void )
{
   Radio.Sleep();
   m_TxDoneCount++;
   m_Event = PMP_EVENT_TXDONE_E;
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
void PacketMainPayload::OnTxTimeout( void )
{
   Radio.Sleep();
   m_TxTimeoutCount++;
   m_Event = PMP_EVENT_TXTIMEOUT_E;
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
void PacketMainPayload::OnFhssChangeChannel( uint8_t channelIndex )
{
   Radio.Sleep();
   m_ChannelChangeCount++;
   m_Event = PMP_EVENT_CHANNEL_E;
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
bool PacketMainPayload::ProcessCommand(const char* pCommand)
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
      else if ( 0 == strncmp(pCommand, "count", 5) )
      {
         char* pValue = (char*)pCommand + 5;
         if( *pValue == '=' )
         {
            pValue++;
            int32_t value = atoi(pValue);
            if ( value > 0 )
            {
               m_MaxMessages = value;
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
void PacketMainPayload::PrintHelp(Serial &serialPort)
{
   serialPort.printf("Currently loaded test = main\r\n\n");
   serialPort.printf("Supported commands:\r\n");
   serialPort.printf("frequency=x (%d)\r\n", m_Frequency);
   serialPort.printf("power=x (%d)\r\n", m_TxPower);
   serialPort.printf("sf=x (%d)\r\n", m_SpreadingFactor);
   serialPort.printf("cr=x (%d)    (1=4/5, 2=4/6, 3=4/7, 4=4/8)\r\n", m_CodingRate);
   serialPort.printf("crc=s (%s)   (lora or mcu)\r\n", m_LoraCrc ? "lora":"mcu");
   serialPort.printf("count=x (%d) (Number of messages in the test)\r\n", m_MaxMessages);
   serialPort.printf("x    Enable/Disable output messages\r\n");
   serialPort.printf("reset\r\n");
   serialPort.printf("\r\n");
}



