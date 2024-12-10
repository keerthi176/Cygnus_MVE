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
*  File         : TxSurveyTool.cpp
*
*  Description  : Implementation of the transmitter for the survey tool.
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/
#include "TxSurveyTool.h"

/* User Definitions
*************************************************************************************/
#define NUM_FREQUENCY_CHANS 11

#ifdef FREQUENCY_BAND_915MHZ
static uint32_t const frequencyChannels[NUM_FREQUENCY_CHANS] = {920150000, 920450000, 920750000, 921050000, 921350000,
                                                                 921650000, 921950000, 922250000, 922550000, 922850000, 923150000};
#else
static uint32_t const frequencyChannels[NUM_FREQUENCY_CHANS] = {865150000, 865450000, 865750000, 866050000, 866350000,
                                                                 866650000, 866950000, 867250000, 867550000, 867850000, 868150000};
#endif

#define RF_CHANNEL                                       5
#define TX_OUTPUT_POWER                                  7


#define LORA_BANDWIDTH                              1         // [0: 125 kHz,
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

#define STTX_DEFAULT_MESSAGE_DELAY  500

/* Type Declarations
*************************************************************************************/
static CRC_HandleTypeDef crc_handle_8b_data_16b_crc;

/*************************************************************************************/
/**
* function name   : TxSurveyTool
* description     : Constructor
*
* @param - None.
*
* @return - nothing
*/
TxSurveyTool::TxSurveyTool( void ) : TestBase(), 
   m_SystemID(0xFFFFFFFF),
   m_Count(0),
   m_Event(STTX_EVENT_NONE_E),
   m_Frequency(RF_CHANNEL),
   m_TxPower(TX_OUTPUT_POWER),
   m_SpreadingFactor(LORA_SPREADING_FACTOR),
   m_CodingRate(LORA_CODINGRATE),
   m_LoraCrc(LORA_CRC_ENABLED),
   m_MessageInterval(STTX_DEFAULT_MESSAGE_DELAY),
   m_TxDoneCount(0),
   m_TxTimeoutCount(0),
   m_ChannelChangeCount(0)
{
}

/*************************************************************************************/
/**
* function name   : TxSurveyTool
* description     : Destructor
*
* @param - None.
*
* @return - nothing
*/
TxSurveyTool::~TxSurveyTool( void )
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
bool TxSurveyTool::Initialise( void )
{
   bool status = true;
   m_Count = 0;
   m_TxDoneCount = 0;
   m_TxTimeoutCount = 0;
   m_ChannelChangeCount = 0;
  
   Radio.SetChannel( frequencyChannels[m_Frequency] );
   Radio.SetTxConfig( MODEM_LORA, m_TxPower, 0, LORA_BANDWIDTH,
                   m_SpreadingFactor, m_CodingRate,
                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                   m_LoraCrc, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                   LORA_IQ_INVERSION_ON, 2000000, true );

   if ( m_LoraCrc )
   {
      debug( "\n\n\r     SX1272 TX Survery Tool\n\n\r" );
   }

   debug( "Tx Power           = %d\r\n", m_TxPower);
   debug( "Frequency          = %d (%d)\r\n", m_Frequency, frequencyChannels[m_Frequency]);
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
   
   m_Event = STTX_EVENT_TXDONE_E;
   
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
bool TxSurveyTool::RunProcess( void )
{
   static char buffer[STTX_MESSAGE_LENGTH_WITH_CRC];
   
   SurveyToolMessage_t* pMessage = (SurveyToolMessage_t*)buffer;
   
   switch ( m_Event )
   {
      case STTX_EVENT_TXDONE_E:
         if ( 0 < m_MessageInterval )
         {
            wait_ms(m_MessageInterval);
         }
         
         m_Count++;
         if ( 9999 < m_Count )
         {
            m_Count = 0;
         }
         
         
         memset(buffer, 0, STTX_MESSAGE_LENGTH);
         pMessage->SystemID = m_SystemID;
         sprintf(pMessage->Payload,"%d", m_Count);
         
         debug("%s\r\n",pMessage->Payload);


         if ( !m_LoraCrc )
         {
            
            buffer[STTX_MESSAGE_LENGTH - 1] = 0;
            uint16_t CrcValue =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)&buffer, STTX_MESSAGE_LENGTH);
            uint16_t* pMsgCRC = (uint16_t*)&buffer[STTX_MESSAGE_LENGTH];
            *pMsgCRC = CrcValue;
            Radio.Send( (uint8_t*)buffer, STTX_MESSAGE_LENGTH_WITH_CRC );
            //sprintf(buffer,"%d,\tcrc=%x", m_Count, CrcValue);
         }
         else
         {
            Radio.Send( (uint8_t*)buffer, STTX_MESSAGE_LENGTH );
         }
         //debug("%s\r\n", buffer);
         m_Event = STTX_EVENT_NONE_E;
         break;
      case STTX_EVENT_TXTIMEOUT_E:
         // Nothing to do.  Schedule another transmission.
         m_Event = STTX_EVENT_TXDONE_E;
         break;
      case STTX_EVENT_CHANNEL_E:
         // Nothing to do.  Schedule another transmission.
         m_Event = STTX_EVENT_TXDONE_E;
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
void TxSurveyTool::OnTxDone( void )
{
   Radio.Sleep();
   m_TxDoneCount++;
   m_Event = STTX_EVENT_TXDONE_E;
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
void TxSurveyTool::OnTxTimeout( void )
{
   Radio.Sleep();
   m_TxTimeoutCount++;
   m_Event = STTX_EVENT_TXTIMEOUT_E;
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
void TxSurveyTool::OnFhssChangeChannel( uint8_t channelIndex )
{
   Radio.Sleep();
   m_ChannelChangeCount++;
   m_Event = STTX_EVENT_CHANNEL_E;
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
bool TxSurveyTool::ProcessCommand(const char* pCommand)
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
void TxSurveyTool::PrintHelp(Serial &serialPort)
{
   serialPort.printf("Currently loaded test = Survey Tool Tx\r\n\n");
   serialPort.printf("Supported commands:\r\n");
   serialPort.printf("frequency=x (%d)\r\n", m_Frequency);
   serialPort.printf("power=x (%d)\r\n", m_TxPower);
   serialPort.printf("sf=x (%d)\r\n", m_SpreadingFactor);
   serialPort.printf("cr=x (%d)  (1=4/5, 2=4/6, 3=4/7, 4=4/8\r\n", m_CodingRate);
   serialPort.printf("crc=s (%s) (lora or mcu)\r\n", m_LoraCrc ? "lora":"mcu");
   serialPort.printf("delay=x (%d) milliseconds message interval\r\n", m_MessageInterval);
   serialPort.printf("reset\r\n");
   serialPort.printf("\r\n\n\n");
}

