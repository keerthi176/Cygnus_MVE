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
*  File         : TxPingTest.h
*
*  Description  : Implementation for the test that sends a ping to a number of RBUs in turn,
*                 and checks for a matching payload in the response.
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include "lptim.h"


/* User Include Files
*************************************************************************************/
#include "TxPingTest.h"

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

#define RF_CHANNEL                                       0
#define TX_OUTPUT_POWER                                  7


#define PACKET_MODE                                 true
#define USE_CAD                                     0       // 0 : do not use CAD, 1 : use CAD
#define LORA_BANDWIDTH                              1         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       6         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        20        // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         24        // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  true      // required for SF6
#define LORA_FHSS_ENABLED                           false  
#define LORA_NB_SYMB_HOP                            4     
#define LORA_IQ_INVERSION_ON                        false
#define LORA_CRC_ENABLED                            false

#define TXPING_DEFAULT_MESSAGE_DELAY  1000
#define TXPING_NODE_RESPONSE_TIMEOUT_US  90000
#define MESSAGE_TIMEOUT_OFFSET 93
#define MESSAGE_DONE_OFFSET 81


/* Type Declarations
*************************************************************************************/
static CRC_HandleTypeDef crc_handle_8b_data_16b_crc;

extern DigitalOut led_r;
extern DigitalOut led_b;
extern DigitalOut ant_tx_nrx;
extern DigitalOut ant_rx_ntx;


static Timer m_Timer;
/*************************************************************************************/
/**
* function name   : TxPingTest
* description     : Constructor
*
* @param - None.
*
* @return - nothing
*/
TxPingTest::TxPingTest( void ) : TestBase(), 
   m_Count(0),
   m_Event(TXPING_EVENT_NONE_E),
   m_Frequency(RF_CHANNEL),
   m_TxPower(TX_OUTPUT_POWER),
   m_SpreadingFactor(LORA_SPREADING_FACTOR),
   m_CodingRate(LORA_CODINGRATE),
   m_LoraCrc(LORA_CRC_ENABLED),
   m_MessageInterval(TXPING_DEFAULT_MESSAGE_DELAY),
   m_EnableOutput(true),
   m_WrapAround(false),
   m_TxDoneCount(0),
   m_TxTimeoutCount(0),
   m_ChannelChangeCount(0)
{
}

/*************************************************************************************/
/**
* function name   : TxPingTest
* description     : Destructor
*
* @param - None.
*
* @return - nothing
*/
TxPingTest::~TxPingTest( void )
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
bool TxPingTest::Initialise( void )
{
   bool status = true;
   m_Count = 0;
   m_TxDoneCount = 0;
   m_TxTimeoutCount = 0;
   m_ChannelChangeCount = 0;
   m_NodeID = 0xffff;
   m_LastResponseReceived = false;
   
   addr.count_addr = (uint32_t)&m_Count;
   addr.rxt_addr = (uint32_t)&m_RadioBoard[TXPING_MAX_NODES-1].m_RxTimeoutCount;
   
   for (uint32_t index = 0; index < TXPING_MAX_NODES; index++)
   {
      m_RadioBoard[index].SetNodeID(index);
   }

   //initialise timer
   MX_LPTIM1_Init();
   
   int flash_status = m_Flash.init();
   if ( 0 == flash_status )
   {
      uint32_t flash_address = m_Flash.get_flash_start();
      flash_address += NODE_ID_FLASH_OFFSET_ADDRESS;
      uint32_t node_read;
      flash_status = m_Flash.read(&node_read, flash_address, sizeof(uint32_t));
      
      if ( 0 == flash_status )
      {
         m_NodeID = node_read;
      }
   }
   
   
   
   Radio.SetChannel( frequencyChannels[m_Frequency] );
   Radio.SetTxConfig( MODEM_LORA, m_TxPower, 0, LORA_BANDWIDTH,
                   m_SpreadingFactor, m_CodingRate,
                   LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                   m_LoraCrc, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                   LORA_IQ_INVERSION_ON, 2000000, true );
   

   uint8_t payload_length = TXPING_MESSAGE_LENGTH;
   if( !m_LoraCrc )
   {
      payload_length = TXPING_MESSAGE_LENGTH_WITH_CRC;
   }
   
   Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, m_SpreadingFactor,
                      m_CodingRate, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, payload_length,
                      m_LoraCrc, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                      LORA_IQ_INVERSION_ON, true, PACKET_MODE );


   if ( m_LoraCrc )
   {
      debug( "\n\n\r     SX1272 TX Ping Test - LoRa CRC\n\n\r" );
   }
   else
   {
      debug( "\n\n\r     SX1272 TX Ping Test - MCU CRC\n\n\r" );
   }
   debug( "Node ID            = %d\r\n", m_NodeID);
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
   
   m_TimeNow = LPTIM_ReadCounter(&hlptim1);
   
   StartNextTransmission();
   
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
bool TxPingTest::RunProcess( void )
{
   static char buffer[TXPING_MESSAGE_LENGTH_WITH_CRC];
   
   switch ( m_Event )
   {
      case TXPING_EVENT_WAIT_E:
      {
          uint32_t m_TimeNow = m_Timer.read_ms();
         if ( m_TimeNow > m_MessageInterval )
         {
            m_Timer.stop();
            m_Event = TXPING_EVENT_SEND_E;
         }
      }
         break;
      case    TXPING_EVENT_SEND_E:
         ant_tx_nrx = 0;
         ant_rx_ntx = 1;
         m_Count++;
         memset(buffer, 0, TXPING_MESSAGE_LENGTH);
         
         sprintf(buffer,"T%d", m_Count);

         if ( !LORA_CRC_ENABLED )
         {
            buffer[TXPING_MESSAGE_LENGTH - 1] = 0;
            uint16_t CrcValue =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)&buffer, TXPING_MESSAGE_LENGTH);
            uint16_t* pMsgCRC = (uint16_t*)&buffer[TXPING_MESSAGE_LENGTH];
            *pMsgCRC = CrcValue;
            led_r=1;
            Radio.Send( (uint8_t*)buffer, TXPING_MESSAGE_LENGTH_WITH_CRC );
         }
         else
         {
            led_r=1;
            Radio.Send( (uint8_t*)buffer, TXPING_MESSAGE_LENGTH );
         }
         if ( m_EnableOutput )
         {
            debug("%s\r\n", buffer);
         }
         m_Event = TXPING_EVENT_NONE_E;
         break;
      case TXPING_EVENT_TXDONE_E:
         RadioBoard::IncrementPacketCount();
         m_ExpectedNode = 0;
         m_Event = TXPING_EVENT_RECEIVE_E;
         wait_ms(1);
         break;
      case TXPING_EVENT_TXTIMEOUT_E:
         // Nothing to do.  Schedule another transmission.
         StartNextTransmission();
         break;
      case TXPING_EVENT_CHANNEL_E:
         // Nothing to do.  Schedule another transmission.
         m_Event = TXPING_EVENT_TXDONE_E;
         break;
      case TXPING_EVENT_RECEIVE_E:
      {
         ant_tx_nrx = 1;
         ant_rx_ntx = 0;
         if ( 0 != m_ExpectedNode )
         {
            if ( m_LastResponseReceived )
            {
               wait_ms(MESSAGE_DONE_OFFSET);
            }
            else
            {
               wait_ms(MESSAGE_TIMEOUT_OFFSET);
            }
         }
         StartReceiving(TXPING_NODE_RESPONSE_TIMEOUT_US);
         m_Event = TXPING_EVENT_NONE_E;
      }
         break;
      case TXPING_EVENT_RXDONE_E:
         m_LastResponseReceived = true;
      
         //Update the records for the radio board
         m_RadioBoard[m_ExpectedNode].SetRSSI(m_RssiValue);
         m_RadioBoard[m_ExpectedNode].SetSNR(m_SnrValue);
         m_RadioBoard[m_ExpectedNode].SetFrequencyDeviation(m_Ferror);
         m_RadioBoard[m_ExpectedNode].ProcessResponse(m_Buffer);
         if ( m_EnableOutput )
         {
            debug("Node %d\tsend rssi %d\trespose rssi %d\tsent %d\tlost %d\tPkt Loss %03f\t\r\n", m_ExpectedNode, m_RadioBoard[m_ExpectedNode].GetRSSI(),
               m_RssiValue, RadioBoard::GetPacketCount(), m_RadioBoard[m_ExpectedNode].GetLostPacketCount(), m_RadioBoard[m_ExpectedNode].GetPacketLoss());
         }
      
         m_ExpectedNode++;
         if ( m_ExpectedNode >= TXPING_MAX_NODES )
         {
            StartNextTransmission();
         }
         else 
         {
            m_Event = TXPING_EVENT_RECEIVE_E;
         }
         break;
      case TXPING_EVENT_RXTIMEOUT_E:
         m_LastResponseReceived = false;
         m_RadioBoard[m_ExpectedNode].ProcessLostPacket(TXPING_EVENT_RXTIMEOUT_E);
         debug("Node %d Rx Timeout\r\n", m_ExpectedNode);
         m_ExpectedNode++;
         if ( m_ExpectedNode >= TXPING_MAX_NODES )
         {
            StartNextTransmission();
         }
         else 
         {
            m_Event = TXPING_EVENT_RECEIVE_E;
         }
         break;
      case TXPING_EVENT_RXERROR_E:
         m_LastResponseReceived = true;
         m_RadioBoard[m_ExpectedNode].ProcessLostPacket(TXPING_EVENT_RXERROR_E);
         debug("Node %d Rx Error\r\n", m_ExpectedNode);
         m_ExpectedNode++;
         if ( m_ExpectedNode >= TXPING_MAX_NODES )
         {
            StartNextTransmission();
         }
         else 
         {
            m_Event = TXPING_EVENT_RECEIVE_E;
         }
         break;
      case TXPING_EVENT_RXCRCERROR_E:
         m_LastResponseReceived = true;
         m_RadioBoard[m_ExpectedNode].ProcessLostPacket(TXPING_EVENT_RXCRCERROR_E);
         debug("Node %d Rx CRC Fail\r\n", m_ExpectedNode);
         m_ExpectedNode++;
         if ( m_ExpectedNode >= TXPING_MAX_NODES )
         {
            StartNextTransmission();
         }
         else 
         {
            m_Event = TXPING_EVENT_RECEIVE_E;
         }
         break;
      case TXPING_EVENT_CADDONE_E:
         debug("Node %d Rx CAD = %d\r\n", m_ExpectedNode, m_CadDone);
         if ( m_CadDone )
         {
            m_Event = TXPING_EVENT_RECEIVE_E;
         }
         else 
         {
            StartNextTransmission();
         }
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
void TxPingTest::OnTxDone( void )
{
   led_r=0;
   Radio.Sleep();
   m_TxDoneCount++;
   m_Event = TXPING_EVENT_TXDONE_E;
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
void TxPingTest::OnTxTimeout( void )
{
   led_r=0;
   Radio.Sleep();
   m_TxTimeoutCount++;
   m_Event = TXPING_EVENT_TXTIMEOUT_E;
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
void TxPingTest::OnFhssChangeChannel( uint8_t channelIndex )
{
   Radio.Sleep();
   m_ChannelChangeCount++;
   m_Event = TXPING_EVENT_CHANNEL_E;
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
bool TxPingTest::ProcessCommand(const char* pCommand)
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
      else if ( 0 == strncmp(pCommand, "id", 2) )
      {
         char* pValue = (char*)pCommand + 2;
         if( *pValue == '=' )
         {
            pValue++;
            int32_t value = atoi(pValue);
            if ( value >= 0 && value < 10)
            {
               m_NodeID = value;
               uint32_t flash_address = m_Flash.get_flash_start();
               flash_address += NODE_ID_FLASH_OFFSET_ADDRESS;

               int flash_status = m_Flash.program(&m_NodeID, flash_address, 8);
               if ( 0 == flash_status )
               {
                  result = true;
               }
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
* function name   : OnRxDone
* description     : Handler for TxDone
*
* @param - None.
*
* @return - nothing
*/
void TxPingTest::OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror)
{
   led_b=0;
   m_EventTime = LPTIM_ReadCounter(&hlptim1);
   
   Radio.Sleep();

   memset(m_Buffer,0, TXPING_MESSAGE_LENGTH_WITH_CRC);
   memcpy( m_Buffer, payload, TXPING_MESSAGE_LENGTH_WITH_CRC );
   m_RssiValue = rssi;
   m_SnrValue = snr;
   m_Ferror = ferror;
   m_FeiValue = FeiVal;

   if ( !m_LoraCrc )
   {
      uint16_t CrcValue =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)&m_Buffer, TXPING_MESSAGE_LENGTH);
      uint16_t* pMsgCRC = (uint16_t*)&m_Buffer[TXPING_MESSAGE_LENGTH];
      
      m_CrcValue = *pMsgCRC;
      if ( m_CrcValue != CrcValue )
      {
         m_LastMessageErrorDetected = true;
         m_RxErrorCount++;
         m_Event = TXPING_EVENT_RXCRCERROR_E;
      }
      else
      {
         m_RxDoneCount++;
         m_Event = TXPING_EVENT_RXDONE_E;
      }
   }
   else
   {
      m_Event = TXPING_EVENT_RXDONE_E;
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
void TxPingTest::OnRxTimeout( void )
{
   led_b=0;
   m_EventTime = LPTIM_ReadCounter(&hlptim1);
   Radio.Sleep();
   m_RxTimeoutCount++;
   m_Event = TXPING_EVENT_RXTIMEOUT_E;
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
void TxPingTest::OnRxError( void )
{
   led_b=0;
   m_EventTime = LPTIM_ReadCounter(&hlptim1);
   Radio.Sleep();
   m_RxErrorCount++;
   m_Event = TXPING_EVENT_RXERROR_E;
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
void TxPingTest::OnCadDone( bool channelActivityDetected )
{
   if ( !channelActivityDetected )
   {
      led_b=0;
   }
   m_EventTime = LPTIM_ReadCounter(&hlptim1);
   Radio.Sleep();
   m_CadDone = channelActivityDetected;
   m_CadDoneCount++;
   m_Event = TXPING_EVENT_CADDONE_E;
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
void TxPingTest::StartReceiving(const uint32_t timeout)
{
   m_Timer.stop();
   m_Timer.reset();
   m_Timer.start();
   
   led_b=1;
   if (USE_CAD == 1)
   {
      Radio.StartCad();
   }
   else
   {
      Radio.Rx( timeout );
   }
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
void TxPingTest::StartNextTransmission()
{
   m_Timer.stop();
   m_Timer.reset();
   m_Timer.start();
   m_Event = TXPING_EVENT_WAIT_E;
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
void TxPingTest::PrintHelp(Serial &serialPort)
{
   serialPort.printf("Currently loaded test = increment\r\n\n");
   serialPort.printf("Supported commands:\r\n");
   serialPort.printf("id=x (%d)\r\n", m_NodeID);
   serialPort.printf("frequency=x (%d)\r\n", m_Frequency);
   serialPort.printf("power=x (%d)\r\n", m_TxPower);
   serialPort.printf("sf=x (%d)\r\n", m_SpreadingFactor);
   serialPort.printf("cr=x (%d)  (1=4/5, 2=4/6, 3=4/7, 4=4/8\r\n", m_CodingRate);
   serialPort.printf("crc=s (%s) (lora or mcu)\r\n", m_LoraCrc ? "lora":"mcu");
   serialPort.printf("delay=x (%d) milliseconds message interval\r\n", m_MessageInterval);
   serialPort.printf("reset\r\n");
   serialPort.printf("\r\n\n\n");
}


// RadioBoad class ----------------------------------------------------------------------

uint32_t RadioBoard::m_TxPacketCount = 0;

/*************************************************************************************/
/**
* function name   : RadioBoard
* description     : Constructor.
*
* @param - None.
*
* @return - none.
*/
RadioBoard::RadioBoard() :
   m_RxDoneCount(0),
   m_RxTimeoutCount(0),
   m_RxErrorCount(0),
   m_RxCrcFailCount(0),
   m_RssiValue(0),
   m_SnrValue(0),
   m_FrequencyDeviation(0),
   m_LostPacketCount(0),
   m_PacketNumber(0),
   m_PacketLoss(0.0)

{
}

/*************************************************************************************/
/**
* function name   : RadioBoard
* description     : Destructor.
*
* @param - None.
*
* @return - none.
*/
RadioBoard::~RadioBoard()
{
}

/*************************************************************************************/
/**
* function name   : ProcessLostPacket
* description     : Updates the ping response statistics for the radio board for a
*                   lost packet.
*
* @param - event -  The reason for the packet loss.
*
* @return - true on success.
*/
bool RadioBoard::ProcessLostPacket(const TxPingEvent_t event)
{
   bool result = true;
   
   switch ( event )
   {
      case TXPING_EVENT_RXTIMEOUT_E:
         m_LostPacketCount++;
         m_RxTimeoutCount++;
         UpdatePacketLoss();
         break;
      
      case TXPING_EVENT_RXERROR_E:
         m_LostPacketCount++;
         m_RxErrorCount++;
         UpdatePacketLoss();
         break;
      
      case TXPING_EVENT_RXCRCERROR_E:
         m_LostPacketCount++;
         m_RxCrcFailCount++;
         UpdatePacketLoss();
         break;
      
      default:
         result = false;
         break;
   }
   
   return result;
}


/*************************************************************************************/
/**
* function name   : ProcessResponse
* description     : Updates the ping response statistics for the radio board for a
*                   received packet.
*
* @param - pMessage - Pointer to the received packet.
*
* @return - true on success.
*/
bool RadioBoard::ProcessResponse(const char* pMessage)
{
   bool result = false;
   
   if ( pMessage )
   {
      if ( ParseReceivedPacket(pMessage) )
      {
         UpdatePacketLoss();
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* function name   : ParseReceivedPacket
* description     : Extracts the data from a received packet.
*
* @param - pMessage - Pointer to the received packet.
*
* @return - true on success.
*/
bool RadioBoard::ParseReceivedPacket(const char* pMessage)
{
   bool result = false;
   const char* pSource;
   char* str_end;

   if ( pMessage )
   {
      pSource = (char*)pMessage;

      /*read the Destination */
      uint16_t unit_address;
      
      unit_address = (uint32_t)strtol((char*)pSource, &str_end, 10);
 
      if ( unit_address == m_NodeID && str_end )
      {
         /* find the first ',' */
         pSource = strchr(pMessage, ',');
         if ( pSource )
         {
            pSource++;/* step over the comma */
            //Read the packet number
            m_PacketNumber = (uint32_t)strtol((char*)pSource, &str_end, 10);
            pSource = str_end;

            if ( pSource )
            {
               pSource++;/* step over the comma */
               //Read the rssi
               m_RssiValue = (int32_t)strtol((char*)pSource, &str_end, 10);
               pSource = str_end;
               
               if( pSource )
               {
                  pSource++;/* step over the comma */
                  /*read the SNR */
                  m_SnrValue = (int32_t)strtol((char*)pSource, &str_end, 10);
                  pSource = str_end;
                  if( pSource )
                  {
                     pSource++;/* step over the comma */
                     /*read the frequency deviation */
                     m_FrequencyDeviation = (uint32_t)strtol((char*)pSource, &str_end, 10);
                     pSource = str_end;
                     if(!*pSource)
                     {
                        result = true;
                     }
                  }
               }
            }
         }
      }
   }

   return result;
}

/*************************************************************************************/
/**
* function name   : UpdatePacketLoss
* description     : Calculate packet loss from internal values.
*
* @param - None.
*
* @return - none.
*/
void RadioBoard::UpdatePacketLoss()
{
   uint32_t total_packets = RadioBoard::GetPacketCount();
   
   m_PacketLoss = 100.0 * (double)(m_LostPacketCount) / (double)total_packets;
}

