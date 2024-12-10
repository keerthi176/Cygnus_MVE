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
*  File         : PacketPpuCommand.cpp
*
*  Description  : Implementation of the test that sends a packet with an incrementing
*                 number in the payload, represented as an ascii string.
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include "string.h"


/* User Include Files
*************************************************************************************/
#include "PacketPPU.h"

/* User Definitions
*************************************************************************************/
#define NUM_FREQUENCY_CHANS 11
uint32_t const frequencyChannels[NUM_FREQUENCY_CHANS] = {865150000, 865450000, 865750000, 866050000, 866350000,
                                                         866650000, 866950000, 867250000, 867550000, 867850000, 868150000};

#define RF_CHANNEL                                       4
#define PPUB_RF_CHANNEL                                  10
#define TX_OUTPUT_POWER                                  7
                                                         
#define PACKET_MODE                                 true
#define LORA_BANDWIDTH                              1         // [0: 125 kHz,
                                                              //  1: 250 kHz,
                                                              //  2: 500 kHz,
                                                              //  3: Reserved]
#define LORA_SPREADING_FACTOR                       7         // [SF7..SF12]
#define LORA_CODINGRATE                             1         // [1: 4/5,
                                                              //  2: 4/6,
                                                              //  3: 4/7,
                                                              //  4: 4/8]
#define LORA_PREAMBLE_LENGTH                        16        // Same for Tx and Rx
#define LORA_SYMBOL_TIMEOUT                         64        // Symbols
#define LORA_FIX_LENGTH_PAYLOAD_ON                  1         // required for SF6
#define LORA_FIX_LENGTH_PAYLOAD_OFF                 0
#define LORA_FHSS_ENABLED                           false  
#define LORA_NB_SYMB_HOP                            4     
#define LORA_IQ_INVERSION_ON                        false
#define LORA_CRC_ENABLED                            false
#define LORA_TX_TIMEOUT_US                          1000000
#define LORA_RX_TIMEOUT_US                          60000000

#define PPU_DEFAULT_MESSAGE_DELAY  1000

#define REG_PLL_BW_300KHZ              0xD0  // From SX1272 data sheet, register 0x5C, page 107.

#define PPU_MESSAGE_LENGTH 5
#define PPU_MESSAGE_LENGTH_WITH_CRC  (PPU_MESSAGE_LENGTH + 2)

/* Type Declarations
*************************************************************************************/
static CRC_HandleTypeDef crc_handle_8b_data_16b_crc;

extern Serial serialPort;
extern DigitalOut ant_tx_nrx;
extern DigitalOut ant_rx_ntx;

/*************************************************************************************/
/**
* function name   : PacketPpuCommand
* description     : Constructor
*
* @param - None.
*
* @return - nothing
*/
PacketPpuCommand::PacketPpuCommand( void ) : TestBase(), 
   m_Count(0),
   m_Event(PPU_EVENT_NONE_E),
   m_State(PPU_STATE_IDLE_E),
   m_Frequency(RF_CHANNEL),
   m_TxPower(TX_OUTPUT_POWER),
   m_SpreadingFactor(LORA_SPREADING_FACTOR),
   m_CodingRate(LORA_CODINGRATE),
   m_LoraCrc(LORA_CRC_ENABLED),
   m_MessageInterval(PPU_DEFAULT_MESSAGE_DELAY),
   m_TxDoneCount(0),
   m_TxTimeoutCount(0),
   m_ChannelChangeCount(0)
{
}

/*************************************************************************************/
/**
* function name   : PacketPpuCommand
* description     : Destructor
*
* @param - None.
*
* @return - nothing
*/
PacketPpuCommand::~PacketPpuCommand( void )
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
bool PacketPpuCommand::Initialise( void )
{
   bool status = true;
   m_Count = 0;
   m_TxDoneCount = 0;
   m_TxTimeoutCount = 0;
   m_ChannelChangeCount = 0;
  
   SetRadioForTransmit(false);
   
   if ( m_LoraCrc )
   {
      debug( "\n\n\r     SX1272 TX PPU Incrementing Payload Packet Continuously - LoRa CRC\n\n\r" );
   }
   else
   {
      debug( "\n\n\r     SX1272 TX PPU Incrementing Payload Packet Continuously - MCU CRC\n\n\r" );
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
   Radio.Standby( );
   Radio.Write( REG_LR_PLL, REG_PLL_BW_300KHZ); 
   Radio.SetChannel( frequencyChannels[PPUB_RF_CHANNEL] );
   Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_OFF, 0,
                      false, LORA_FHSS_ENABLED, 0, 
                      LORA_IQ_INVERSION_ON, true, PACKET_MODE );

   
   m_State = PPU_STATE_IDLE_E;
   
   return status;
}

/*************************************************************************************/
/**
* function name   : SetRadioForTransmit
* description     : Configure the radio for transmit
*
* @param - forPpu   True for PPU commands (non-fixed length), false for normal RBU.
*
* @return - None.
*/
void PacketPpuCommand::SetRadioForTransmit(const bool forPpu)
{
   bool fixed_payload = LORA_FIX_LENGTH_PAYLOAD_ON;
   
   if ( forPpu )
   {
      fixed_payload = LORA_FIX_LENGTH_PAYLOAD_OFF;
   }
   
   debug( "SetRadioForTransmit fix=%d\n\r", fixed_payload );
   /* put radio into standby for config */
   Radio.Standby( );
   Radio.Write( REG_LR_PLL, REG_PLL_BW_300KHZ); 
   Radio.SetChannel( frequencyChannels[m_Frequency] );
   Radio.SetTxConfig( MODEM_LORA, m_TxPower, 0, LORA_BANDWIDTH,
                   LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                   LORA_PREAMBLE_LENGTH, fixed_payload,
                   false, LORA_FHSS_ENABLED, 0, 
                   LORA_IQ_INVERSION_ON, LORA_TX_TIMEOUT_US, true );
   ant_tx_nrx = 0;
   ant_rx_ntx = 1;
}

/*************************************************************************************/
/**
* function name   : StartReceive
* description     : Configure the radio for receive
*
* @param - None.
*
* @return - None.
*/
void PacketPpuCommand::StartReceive(void)
{
//   debug( "StartReceive fix=%d freq=%d\n\r", LORA_FIX_LENGTH_PAYLOAD_OFF, m_Frequency );
//   /* put radio into standby for config */
//   Radio.Standby( );
//   Radio.Write( REG_LR_PLL, REG_PLL_BW_300KHZ); 
//   Radio.SetChannel( frequencyChannels[m_Frequency] );
//   Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
//                      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
//                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_OFF, 0,
//                      false, LORA_FHSS_ENABLED, 0, 
//                      LORA_IQ_INVERSION_ON, true, PACKET_MODE );
//   ant_tx_nrx = 1;
//   ant_rx_ntx = 0;
//   Radio.Rx(LORA_RX_TIMEOUT_US);
   Radio.Standby( );
   Radio.SetChannel( frequencyChannels[PPUB_RF_CHANNEL] );
   wait_ms(1);
   Radio.Rx( LORA_RX_TIMEOUT_US );
}

/*************************************************************************************/
/**
* function name   : GetExpectedResponseSize
* description     : Return the expected message length of a response to a command
*
* @param - None.
*
* @return - The expected message length.
*/
uint32_t PacketPpuCommand::GetExpectedResponseSize(void)
{
   uint32_t msg_length;
   
   switch ( m_PpuTxMessage.Command )
   {
      case PPUB_CMD_PING:
         msg_length = PPUB_RESPONSE_PING_LENGTH;
         break;
      case PPUB_CMD_PPU_VERSION:
         msg_length = PPUB_RESPONSE_VERSION_LENGTH;
         break;
      case PPUB_CMD_PPU_FIRMWARE_BIT_REQ:
         msg_length = PPUB_RESPONSE_BIT_REQ_LENGTH;
         break;
      case PPUB_CMD_PPU_CONNECT:
         msg_length = PPUB_ANNOUNCEMENT_LENGTH;
      default:
         msg_length = 255;
      break;
   }
   
   return msg_length;
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
bool PacketPpuCommand::RunProcess( void )
{
   static char buffer[sizeof(PPU_Message_t)];
//   static PPU_Message_t Message;
   
   switch ( m_Event )
   {
      case PPU_EVENT_TXDONE_E:
         if ( m_State == PPU_STATE_WAITING_FOR_RADIO_SEND_CONFIRMATION_E )
         {
            //Not expecting a response from the RBU.  Just send "OK" to the usart.
            serialPort.printf("OK\r\n");
            wait_ms(1);
            StartReceive();
         }
         else if ( m_State == PPU_STATE_WAITING_FOR_RADIO_RESPONSE_E )
         {
            wait_ms(1);
            //We expect a response from the RBU.  Configure to receive.
            StartReceive();
         }
         m_Event = PPU_EVENT_NONE_E;
         break;
      case PPU_EVENT_TXTIMEOUT_E:
         m_Event = PPU_EVENT_NONE_E;
         StartReceive();
         break;
      case PPU_EVENT_RXDONE_E:
         ProcessRadioMessage();
         wait_ms(1);
         //Restart the receiver
         StartReceive();
         m_Event = PPU_EVENT_NONE_E;
         break;
      case PPU_EVENT_RXTIMEOUT_E:
         m_Event = PPU_EVENT_NONE_E;
         debug("Rto\r\n");
         StartReceive();
         break;
      case PPU_EVENT_RXERROR_E:
         m_Event = PPU_EVENT_NONE_E;
         StartReceive();
         break;
      case PPU_EVENT_RXCRCERROR_E:
         sprintf(buffer,"Rx CRC fail.  msg=%x, calc=%x\r\n", m_PpuRxMessage.Checksum, m_PpuRxMessage.CalculatedChecksum);
         debug(buffer);
         StartReceive();
         m_Event = PPU_EVENT_NONE_E;
         break;
      case PPU_EVENT_CHANNEL_E:
         // Nothing to do.  Schedule another transmission.
         m_Event = PPU_EVENT_NONE_E;
         break;
      case PPU_EVENT_USART_CMD_E:
         ProcessUsartCommand();
         m_Event = PPU_EVENT_NONE_E;
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
void PacketPpuCommand::OnTxDone( void )
{
   m_TxDoneCount++;
   m_Event = PPU_EVENT_TXDONE_E;
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
void PacketPpuCommand::OnTxTimeout( void )
{
   m_TxTimeoutCount++;
   m_Event = PPU_EVENT_TXTIMEOUT_E;
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
void PacketPpuCommand::OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror)
{
   memset(m_Buffer,0, PPUB_MAX_MESSAGE_LENGTH);
   memcpy( m_Buffer, payload, PPUB_MAX_MESSAGE_LENGTH );
   m_RssiValue = rssi;
   m_SnrValue = snr;
   m_Ferror = ferror;
   m_FeiValue = FeiVal;
   
   if ( m_PpuRxMessage.Deserialise(payload) )
   {
      m_Event = PPU_EVENT_RXDONE_E;
   }
   else
   {
      m_Event = PPU_EVENT_RXERROR_E;
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
void PacketPpuCommand::OnRxTimeout( void )
{
   m_Event = PPU_EVENT_RXTIMEOUT_E;
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
void PacketPpuCommand::OnRxError( void )
{
   m_Event = PPU_EVENT_RXERROR_E;
};

/*************************************************************************************/
/**
* function name   : OnFhssChangeChannel
* description     : Handler for Fhss Change Channel
*
* @param - None.
*
* @return - nothing
*/
void PacketPpuCommand::OnFhssChangeChannel( uint8_t channelIndex )
{
   m_ChannelChangeCount++;
   m_Event = PPU_EVENT_CHANNEL_E;
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
bool PacketPpuCommand::ProcessCommand(const char* pCommand)
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
            if ( value >= 0 && value < 11)
            {
               m_Frequency = value;
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
      else if ( 0 == strncmp(pCommand, "cmd", 3) )
      {
         char* pValue = (char*)pCommand + 3;
         if( *pValue == '=' )
         {
            pValue++;
            result = ReadPpuCommand(pValue);
         }
      }
      else if ( 0 == strncmp(pCommand, "PPUMode", 7) )
      {
         uint16_t destination;
         char* pValue = (char*)pCommand + 7;
         if( *pValue == '=' )
         {
            pValue++;
            destination = (uint16_t)atoi(pValue);
            SendEnterPPUMode(destination);
            result = true;
         }
      }
      else if ( 0 == strncmp(pCommand, "ament", 5) )
      {
         SendAnnouncement();
         result = true;
      }
      else if ( 0 == strncmp(pCommand, "Ping", 4) )
      {
         uint16_t destination;
         char* pValue = (char*)pCommand + 4;
         if( *pValue == '=' )
         {
            pValue++;
            destination = (uint16_t)atoi(pValue);
            result = SendPing(destination);
         }
      }
      else if ( 0 == strncmp(pCommand, "PpuCnx", 6) )
      {
         uint16_t destination;
         char* pValue = (char*)pCommand + 6;
         if( *pValue == '=' )
         {
            pValue++;
            destination = (uint16_t)atoi(pValue);
            result = SendPpuConnect(destination);
         }
      }
      else if ( 0 == strncmp(pCommand, "VerReq", 6) )
      {
         uint16_t destination;
         char* pValue = (char*)pCommand + 6;
         if( *pValue == '=' )
         {
            pValue++;
            destination = (uint16_t)atoi(pValue);
            result = SendPpuVersionRequest(destination);
         }
      }
      else if ( 0 == strncmp(pCommand, "ResetFW", 7) )
      {
         uint16_t destination;
         char* pValue = (char*)pCommand + 7;
         if( *pValue == '=' )
         {
            pValue++;
            destination = (uint16_t)atoi(pValue);
            result = SendPpuResetToRadioUpdate(destination);
         }
      }
      else if ( 0 == strncmp(pCommand, "Reset", 5) )
      {
         uint16_t destination;
         char* pValue = (char*)pCommand + 5;
         if( *pValue == '=' )
         {
            pValue++;
            destination = (uint16_t)atoi(pValue);
            result = SendPpuReset(destination);
         }
      }
      else if ( 0 == strncmp(pCommand, "Update", 6) )
      {
         uint16_t destination;
         uint16_t packet_size;
         uint16_t number_of_packets;
         uint32_t count = 0;
         char* pValue = (char*)pCommand + 6;
         if( *pValue == '=' )
         {
            pValue++;
            destination = (uint16_t)atoi(pValue);
            while (*pValue != ',' && count < 4 )
            {
               pValue++;
               count++;
            }
            if ( count < 4 )
            {
               //jump over the comma
               pValue++;
               count = 0;
               packet_size = (uint16_t)atoi(pValue);
               while (*pValue != ',' && count < 4 )
               {
                  pValue++;
                  count++;
               }
               if ( count < 4 )
               {
                  //jump over the comma
                  pValue++;
                  number_of_packets = (uint16_t)atoi(pValue);
                  result = SendPpuUpdate(destination, packet_size, number_of_packets);
               }
            }
         }
      }
      else if ( 0 == strncmp(pCommand, "lostpkt", 7) )
      {
         uint16_t destination;
         char* pValue = (char*)pCommand + 7;
         if( *pValue == '=' )
         {
            pValue++;
            destination = (uint16_t)atoi(pValue);
            result = SendPpuLostPacketRequest(destination);
         }
      }
      else if ( 0 == strncmp(pCommand, "ATCMD", 5) )
      {
         uint16_t destination;
         char* pValue = (char*)pCommand + 5;
         char* pAtCommand = (char*)strstr(pCommand, ",");
         if ( pAtCommand )
         {
            //jump over the ','
            pAtCommand++;
         }
         if( *pValue == '=' )
         {
            pValue++;
            destination = (uint16_t)atoi(pValue);
            
            result = SendAtCommand(destination, pAtCommand);
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
* @param - SerialPort - The port to print to.
*
* @return - nothing
*/
void PacketPpuCommand::PrintHelp(Serial &SerialPort)
{
   SerialPort.printf("Currently loaded test = increment\r\n\n");
   SerialPort.printf("Supported commands:\r\n");
   SerialPort.printf("frequency=x (%d)\r\n", m_Frequency);
   SerialPort.printf("power=x (%d)\r\n", m_TxPower);
   SerialPort.printf("sf=x (%d)\r\n", m_SpreadingFactor);
   SerialPort.printf("cr=x (%d)  (1=4/5, 2=4/6, 3=4/7, 4=4/8\r\n", m_CodingRate);
   SerialPort.printf("crc=s (%s) (lora or mcu)\r\n", m_LoraCrc ? "lora":"mcu");
   SerialPort.printf("delay=x (%d) milliseconds message interval\r\n", m_MessageInterval);
   SerialPort.printf("reset\r\n");
   SerialPort.printf("cmd=id,command,len,data[]\r\n");
   SerialPort.printf("rdata=data[]\r\n");
   SerialPort.printf("\r\n\n\n");
}

/*************************************************************************************/
/**
* function name   : IsHexChar
* description     : Returns true if the supplied char is a hex character
*
* @param - pChar - The character to be tested.
*
* @return - TRUE if the char is a Hex value
*/
bool PacketPpuCommand::IsHexChar(const char* pChar)
{
   bool result = false;
   
   if ( (*pChar >= '0') && (*pChar < 'G'))
   {
      if ( (*pChar <= '9') || (*pChar >= 'A') )
      {
         result = true;
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* function name   : CharToHexValue
* description     : Returns a hex value from the supplied character
*
* @param - pChar - The character to be converted.
*
* @return - TRUE if the char is a Hex value
*/
uint8_t PacketPpuCommand::CharToHexValue(const char* pChar)
{
   uint8_t value = false;
   
   if ( IsHexChar(pChar) )
   {
      if ( (*pChar >= '0') && (*pChar <= '9') )
      {
         value = *pChar - '0';
      }
      else 
      {
         value = *pChar - 'A' + 10;
      }
   }
   
   return value;
}

/*************************************************************************************/
/**
* function name   : HexTextToUint8
* description     : Read Hex chars into a uint8_t
*
* @param - pText - The first byte of the Hex chars to be converted.
*
* @return - the uint8_t value of the supplied Hex chars
*/
uint8_t PacketPpuCommand::HexTextToUint8(const char* pText)
{
   uint8_t value = 0;
   uint32_t count = 0;
   char* pPtr = (char*)pText;
   
   while ( IsHexChar(pPtr) && (count < 2))
   {
      value <<= 4;
      value += CharToHexValue(pPtr);
      pPtr++;
      count++;
   }
   
   return value;
}

/*************************************************************************************/
/**
* function name   : HexTextToUint16
* description     : Read Hex chars into a uint16_t
*
* @param - pText - The first byte of the Hex chars to be converted.
*
* @return - the uint16_t value of the supplied Hex chars
*/
uint16_t PacketPpuCommand::HexTextToUint16(const char* pText)
{
   uint16_t value = 0;
   uint32_t count = 0;
   char* pPtr = (char*)pText;
   
   while ( IsHexChar(pPtr) && (count < 4))
   {
      value <<= 4;
      value += CharToHexValue(pPtr);
      pPtr++;
      count++;
   }
   
   return value;
}

/*************************************************************************************/
/**
* function name   : HexTextToUint32
* description     : Read Hex chars into a uint32_t
*
* @param - pText - The first byte of the Hex chars to be converted.
*
* @return - the uint16_t value of the supplied Hex chars
*/
uint32_t PacketPpuCommand::HexTextToUint32(const char* pText)
{
   uint32_t value = 0;
   uint32_t count = 0;
   char* pPtr = (char*)pText;
   
   while ( IsHexChar(pPtr) && (count < 8))
   {
      value <<= 4;
      value += CharToHexValue(pPtr);
      pPtr++;
      count++;
   }
   
   return value;
}


/*************************************************************************************/
/**
* function name   : ReadPpuCommand
* description     : Read usart commands into a command structure for transmission
*                   cmd=<source>,<destination>,<systemID|SerNo>,<paketLength>,<command type>,<data array>
*                   where:
*                   <desination> is the target RBU node address
*                   <command type> is the PPU command to be sent (2 bytes)
*                   <data len> is the length of the following data array
*                   <data array> is the payload to be sent (hex chars)
*
* @param - pValue    Pointer to the received command string from the usart.
*
* @return - bool  true if command succeeded, false otherwise.
*/
bool PacketPpuCommand::ReadPpuCommand(const char* pValue)
{
   bool result = false;
   uint32_t count = 0;
   char* pPtr = (char*)pValue;
//   debug("1\r\n");
   if ( PPU_EVENT_NONE_E == m_Event )
   {
      //Read the Source Node (PPU Master)
      m_UsartCommand.Source = HexTextToUint16(pPtr);
      //Advance to next comma
      while( (*pPtr != ',') && (count < 5) )
      {
         pPtr++;
         count++;
      }
      
      if ( count < 5 )
      {
         count = 0;
         //step over comma
         pPtr++;
         //Read the destination Node
         m_UsartCommand.Destination = HexTextToUint16(pPtr);
         //Advance to next comma
         while( (*pPtr != ',') && (count < 5) )
         {
            pPtr++;
            count++;
         }
         
         if ( count < 5 )
         {
            count = 0;
            //step over comma
            pPtr++;
            
            //Read the systemID/Serial number
            m_UsartCommand.SystemID = HexTextToUint32(pPtr);
            //Advance to next comma
            while( (*pPtr != ',') && (count < 9) )
            {
               pPtr++;
               count++;
            }
            
            if ( count < 9 )
            {
               count = 0;
               //step over comma
               pPtr++;
               
               //Read the Packet length
               m_UsartCommand.PacketLength = HexTextToUint16(pPtr);
               if ( m_UsartCommand.PacketLength <= PPUB_MAX_PAYLOAD_LENGTH )
               {
                  //Advance to next comma
                  while( (*pPtr != ',') && (count < 5) )
                  {
                     pPtr++;
                     count++;
                  }
                  
                  if ( count < 5 )
                  {
                     count = 0;
                     //step over comma
                     pPtr++;
                     //Read the command
                     m_UsartCommand.Command = HexTextToUint16(pPtr);;
                     //Advance to next comma
                     while( (*pPtr != ',') && (count < 5) )
                     {
                        pPtr++;
                        count++;
                     }
                     
                     if ( count < 5 )
                     {
                        count = 0;
                        //step over comma
                        pPtr++;
                        uint32_t payload_length = m_UsartCommand.PacketLength - PPUB_MESSAGE_HEADER_LENGTH;
                        //Read in the data array
                        while ( (count < payload_length) && IsHexChar(pPtr) )
                        {
                           m_UsartCommand.Payload[count] = HexTextToUint8(pPtr);
                           count++;
                           //anvance to next value
                           pPtr+=2;
                        }
                        
                        m_Event = PPU_EVENT_USART_CMD_E;
                        result = true;
                     }
                     else 
                     {
                        if ( 0 == m_UsartCommand.PacketLength )
                        {
                           m_Event = PPU_EVENT_USART_CMD_E;
                           result = true;
                        }
                        else 
                        {
                           debug("missing data[]\r\n");
                        }
                     }
                  }
                  else 
                  {
                     debug("fail command=%d\r\n", m_UsartCommand.Command);
                  }
               }
               else 
               {
                  debug("fail len=%d\r\n", m_UsartCommand.PacketLength);
               }
            }
            else 
            {
               debug("fail count=%d\r\n", count);
            }
         }
         else 
         {
            debug("fail count=%d\r\n", count);
         }
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* function name   : ProcessUsartCommand
* description     : Send the contents of m_UsartCommand over the radio and set m_State
*                   to enable correct testing of the response
*
* @param - None.
*
* @return - None.
*/
void PacketPpuCommand::ProcessUsartCommand(void)
{
   static char usart_buffer[128];
   static uint8_t send_buffer[sizeof(PPU_Message_t)];
   uint32_t message_length = 0;
   
   memset( send_buffer, 0 , sizeof(PPU_Message_t));
   //transfer the content of m_UsartCommand into m_PpuTxMessage
   m_PpuTxMessage.PpuId = m_UsartCommand.Source;
   m_PpuTxMessage.DeviceId = m_UsartCommand.Destination;
   m_PpuTxMessage.Command = m_UsartCommand.Command;
   m_PpuTxMessage.PacketLength = m_UsartCommand.PacketLength;
   uint32_t payload_length = m_UsartCommand.PacketLength - PPUB_MESSAGE_HEADER_LENGTH;
   memcpy(m_PpuTxMessage.Payload, m_UsartCommand.Payload, payload_length);
   
   m_PpuTxMessage.Serialise(send_buffer);
   
   sprintf(usart_buffer,"Tx msg, cmd=%x, crc=%x\r\n", m_PpuTxMessage.Command, m_PpuTxMessage.Checksum);
   debug(usart_buffer);
   
#ifdef EMBEDDED_CHECKSUM
   message_length = m_PpuTxMessage.PacketLength;
#else
   message_length = m_PpuTxMessage.PacketLength + 2;
#endif
   
   //Send over the radio
   SetRadioForTransmit(true);
   Radio.Send( send_buffer, message_length );
   
   if ( (m_PpuTxMessage.Command == PPUB_CMD_PPU_FIRMWARE_SEND_PACKET) || 
        (m_PpuTxMessage.Command == PPUB_CMD_PPU_FIRMWARE_UPDATE_CONFIG) || 
        (m_PpuTxMessage.Command == PPUB_CMD_PPU_RESET) )
   {
      m_State = PPU_STATE_WAITING_FOR_RADIO_SEND_CONFIRMATION_E;
   }
   else 
   {
      m_State = PPU_STATE_WAITING_FOR_RADIO_RESPONSE_E;
   }
}

/*************************************************************************************/
/**
* function name   : ProcessRadioMessage
* description     : Process a response message received over the radio
*
* @param - None.
*
* @return - None.
*/
void PacketPpuCommand::ProcessRadioMessage(void)
{
   switch ( m_PpuRxMessage.Command )
   {
      case PPUP_RESPONSE_PING_ACKNOWLEDGE:
         debug("Rxd ACK msg from device %d\r\n", m_PpuRxMessage.DeviceId);
         break;
      case PPUB_RESPONSE_PPU_VERSION:
         debug("Rxd from device %d, VERSION=%s\r\n", m_PpuRxMessage.DeviceId,m_PpuRxMessage.Payload);
         break;
      case PPUB_RESPONSE_FIRMWARE_BIT_REQ:
      {
         uint16_t* pLostPackets = (uint16_t*)m_PpuRxMessage.Payload;
         debug("Rxd from device %d, Lost Packets=%d,%d,%d,%d,%d,%d,%d,%d\r\n", m_PpuRxMessage.DeviceId, pLostPackets[0], pLostPackets[1], pLostPackets[2], pLostPackets[3], pLostPackets[4], pLostPackets[5], pLostPackets[6], pLostPackets[7]);
      }
         break;
      case PPUB_ANNOUNCEMENT_MSG:
         debug("Rxd ANNOUNCEMENT from device %d\r\n", m_PpuRxMessage.DeviceId);
         break;
      default:
         debug("Rxd unknown msg from device %d\r\n", m_PpuRxMessage.DeviceId);
         break;
   }
}


/*************************************************************************************/
/**
* function name   : PPU_Message_t::Serialise
* description     : Serlialise the contents of a PPU message structure into a character array
*
* @param - pBuffer   [OUT] The character buffer to write to.
*
* @return - None.
*/
void PPU_Message_t::Serialise(uint8_t* pBuffer)
{
   uint8_t* pPtr;
   uint32_t payload_length = PacketLength - PPUB_MESSAGE_HEADER_LENGTH;
   
   if ( pBuffer )
   {
      pBuffer[0] = PpuId & 0xFF;
      pBuffer[1] = (PpuId >> 8) & 0xFF;
      pBuffer[2] = DeviceId & 0xFF;
      pBuffer[3] = (DeviceId >> 8) & 0xFF;
      pBuffer[4] = SystemID & 0xFF;
      pBuffer[5] = (SystemID >> 8) & 0xFF;
      pBuffer[6] = (SystemID >> 16) & 0xFF;
      pBuffer[7] = (SystemID >> 24) & 0xFF;
#ifdef EMBEDDED_CHECKSUM
      pBuffer[8] = 0;
      pBuffer[9] = 0;
      pBuffer[10] = PacketLength & 0xFF;
      pBuffer[11] = (PacketLength >> 8) & 0xFF;
      pBuffer[12] = Command & 0xFF;
      pBuffer[13] = (Command >> 8) & 0xFF;
#else
      pBuffer[8] = PacketLength & 0xFF;
      pBuffer[9] = (PacketLength >> 8) & 0xFF;
      pBuffer[10] = Command & 0xFF;
      pBuffer[11] = (Command >> 8) & 0xFF;
#endif
      if ( PPUB_MAX_PAYLOAD_LENGTH < payload_length )
      {
         payload_length = PPUB_MAX_PAYLOAD_LENGTH;
      }
      
      pPtr = pBuffer;
      pPtr += PPUB_MESSAGE_HEADER_LENGTH;
      memcpy(pPtr, Payload, payload_length);
      //Calculate the checksum
      __HAL_RCC_CRC_FORCE_RESET();
      __HAL_RCC_CRC_RELEASE_RESET();
      Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)pBuffer, PacketLength);
      
#ifdef EMBEDDED_CHECKSUM
      pBuffer[8] = (uint8_t)(Checksum & 0xFF);
      pBuffer[9] = (uint8_t)((Checksum >> 8) & 0xFF);
#else
      pPtr = pBuffer + PacketLength;
      pPtr[0] = (uint8_t)((Checksum >> 8) & 0xFF);
      pPtr[1] = (uint8_t)(Checksum & 0xFF);
#endif
   
      debug("HAL_CRC_Calculate(len=%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x)=%04x\r\n",PacketLength, pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3], pBuffer[4], pBuffer[5], pBuffer[6], pBuffer[7], pBuffer[8], pBuffer[9], Checksum);
   }
}

/*************************************************************************************/
/**
* function name   : PPU_TxMessage_t::Deserialise
* description     : Deserlialise the contents of a character array into a PPU message structure
*
* @param - pBuffer   The character buffer to deserialise.
*
* @return - TRUE if the calculated checksum matches the message checksum.
*/
bool PPU_Message_t::Deserialise(const uint8_t* pBuffer)
{
   static uint8_t msgBuffer[PPUB_MAX_MESSAGE_LENGTH];
   bool result = false;
   uint8_t* pPtr;
   uint32_t payload_length;
   
   if ( pBuffer )
   {
      memcpy(msgBuffer, pBuffer, PPUB_MAX_MESSAGE_LENGTH);
      
      PpuId = (uint16_t)msgBuffer[0] + ((uint16_t)msgBuffer[1] << 8);
      DeviceId = (uint16_t)msgBuffer[2] + ((uint16_t)msgBuffer[3] << 8);
      SystemID = (uint16_t)msgBuffer[4] + ((uint16_t)msgBuffer[5] << 8) + ((uint16_t)msgBuffer[6] << 16) + ((uint16_t)msgBuffer[7] << 24);
#ifdef EMBEDDED_CHECKSUM
      Checksum = (uint16_t)msgBuffer[8] + ((uint16_t)msgBuffer[9] << 8);
      PacketLength = (uint16_t)msgBuffer[10] + ((uint16_t)msgBuffer[11] << 8);
      Command = (uint16_t)msgBuffer[12] + ((uint16_t)msgBuffer[13] << 8);
#else
      PacketLength = (uint16_t)msgBuffer[8] + ((uint16_t)msgBuffer[9] << 8);
      Command = (uint16_t)msgBuffer[10] + ((uint16_t)msgBuffer[11] << 8);
#endif
      
      payload_length = PacketLength - PPUB_MESSAGE_HEADER_LENGTH;
      
      if ( PPUB_MAX_PAYLOAD_LENGTH < payload_length )
      {
         payload_length = PPUB_MAX_PAYLOAD_LENGTH;
      }
      
      pPtr = (uint8_t*)msgBuffer + PPUB_MESSAGE_HEADER_LENGTH;
      memcpy(Payload, pPtr, payload_length);
      
   //Calculate the checksum
   //For embedded checksum, the checksum field must be zeroed to get the correct calculation
#ifdef EMBEDDED_CHECKSUM
      msgBuffer[8] = 0;
      msgBuffer[9] = 0;
#else
      PacketLength -= 2;  //the last two bytes are the message checksum
      Checksum = pPtr[PacketLength] + ((uint16_t)pPtr[PacketLength+1] << 8);
#endif
   
      __HAL_RCC_CRC_FORCE_RESET();
      __HAL_RCC_CRC_RELEASE_RESET();
      CalculatedChecksum = (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)msgBuffer, PacketLength);

      if ( CalculatedChecksum == Checksum )
      {
         result = true;
      }
   }
   return result;
}

/*************************************************************************************/
/**
* function name   : SendEnterPPUMode
* description     : Send the 'enter ppu ode' command to the specified address
*
* @param - destination   [OUT] The destination address.
*
* @return - None.
*/
void PacketPpuCommand::SendEnterPPUMode(const uint16_t destination)
{
   uint8_t buffer[16];
   uint16_t Checksum;
   //build the message
   memset(buffer,0,16);
   buffer[0] = 0x40;
   buffer[1] = (uint8_t)(destination & 0xFF);
   buffer[2] = (uint8_t)((destination >> 8) & 0xFF);
   buffer[3] = 0x00;
   buffer[4] = 0x00;
   buffer[5] = 0xFF;
   buffer[6] = 0xFF;
   buffer[7] = 0xFF;
   buffer[8] = 0xFF;
   //Calculate the checksum
   __HAL_RCC_CRC_FORCE_RESET();
   __HAL_RCC_CRC_RELEASE_RESET();
   Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, 9);
   buffer[9] = (uint8_t)((Checksum >> 8) & 0xFF);
   buffer[10] = (uint8_t)(Checksum & 0xFF);
   
   serialPort.printf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10]);
   serialPort.printf("crc=%04X\r\n", Checksum);
   //Send over the radio
   m_Frequency = 4;
   SetRadioForTransmit(false);
   Radio.Send( buffer, 11 );
   m_Frequency = PPUB_RF_CHANNEL;
   m_State = PPU_STATE_WAITING_FOR_RADIO_SEND_CONFIRMATION_E;
}

/*************************************************************************************/
/**
* function name   : SendAnnouncement
* description     : Send the PPU Announcement to the specified address
*
* @param - None.
*
* @return - None.
*/
void PacketPpuCommand::SendAnnouncement(void)
{
   static uint8_t buffer[32];
   uint16_t Checksum;
   //build the message
   memset(buffer,0,32);
   buffer[0] = (uint8_t)(512 & 0xFF);
   buffer[1] = (uint8_t)((512 >> 8) & 0xFF);
   buffer[2] = 0x08;
   buffer[3] = 0x02;
   buffer[4] = 0xFF;
   buffer[5] = 0xFF;
   buffer[6] = 0xFF;
   buffer[7] = 0xFF;
   buffer[8] = 0xFF;
   buffer[9] = 0xFF;
   buffer[10] = 0xFF;
   buffer[11] = 0xFF;
   //Calculate the checksum
   __HAL_RCC_CRC_FORCE_RESET();
   __HAL_RCC_CRC_RELEASE_RESET();
   Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, 12);
   buffer[12] = (uint8_t)((Checksum >> 8) & 0xFF);
   buffer[13] = (uint8_t)(Checksum & 0xFF);
   
   serialPort.printf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13]);
   serialPort.printf("crc=%04X\r\n", Checksum);
   //Send over the radio
   m_Frequency = PPUB_RF_CHANNEL;
   SetRadioForTransmit(true);
   Radio.Send( buffer, 14 );
   m_State = PPU_STATE_WAITING_FOR_RADIO_SEND_CONFIRMATION_E;
}

/*************************************************************************************/
/**
* function name   : SendAnnouncement
* description     : Send the PPU Announcement to the specified address
*
* @param - None.
*
* @return - None.
*/
bool PacketPpuCommand::SendPing(const uint16_t nodeID)
{
   static uint8_t buffer[32];
   uint16_t Checksum;
   uint32_t system_id = 107;
   //build the message
   memset(buffer,0,32);
   buffer[0] = 0x00;
   buffer[1] = 0x02; //PPU Master address = 512
   buffer[2] = 0x01;
   buffer[3] = 0x00;  //target RBU address 1
   buffer[4] = (uint8_t)(system_id & 0xFF);
   buffer[5] = (uint8_t)((system_id >> 8) & 0xFF);
   buffer[6] = (uint8_t)((system_id >> 16) & 0xFF);
   buffer[7] = (uint8_t)((system_id >> 24) & 0xFF);
#ifdef EMBEDDED_CHECKSUM
   buffer[8] = 0;//checksum
   buffer[9] = 0; 
   buffer[10] = (uint8_t)(PPUB_MESSAGE_HEADER_LENGTH & 0xFF);//  pkt len
   buffer[11] = (uint8_t)(((uint16_t)PPUB_MESSAGE_HEADER_LENGTH >> 8) & 0xFF);
   buffer[12] = (uint8_t)(PPUB_CMD_PING & 0xFF);
   buffer[13] = (uint8_t)((PPUB_CMD_PING >> 8) & 0xFF);
   //Calculate the checksum
   __HAL_RCC_CRC_FORCE_RESET();
   __HAL_RCC_CRC_RELEASE_RESET();
   Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, PPUB_MESSAGE_HEADER_LENGTH);
   buffer[8] = (uint8_t)(Checksum & 0xFF);
   buffer[9] = (uint8_t)((Checksum >> 8) & 0xFF);
#else
   buffer[8] = (uint8_t)(PPUB_MESSAGE_HEADER_LENGTH & 0xFF);// pkt len
   buffer[9] = (uint8_t)(((uint16_t)PPUB_MESSAGE_HEADER_LENGTH >> 8) & 0xFF);
   buffer[10] = (uint8_t)(PPUB_CMD_PING & 0xFF);
   buffer[11] = (uint8_t)((PPUB_CMD_PING >> 8) & 0xFF);
   //Calculate the checksum
   __HAL_RCC_CRC_FORCE_RESET();
   __HAL_RCC_CRC_RELEASE_RESET();
   Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, PPUB_MESSAGE_HEADER_LENGTH);
   buffer[12] = (uint8_t)((Checksum >> 8) & 0xFF);
   buffer[13] = (uint8_t)(Checksum & 0xFF);
#endif
   
   serialPort.printf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13]);
   serialPort.printf("crc=%04X\r\n", Checksum);
   //Send over the radio
   m_Frequency = PPUB_RF_CHANNEL;
   SetRadioForTransmit(true);
   Radio.Send( buffer, PPUB_MESSAGE_HEADER_LENGTH );
   m_State = PPU_STATE_WAITING_FOR_RADIO_RESPONSE_E;
   return true;
}

/*************************************************************************************/
/**
* function name   : SendPpuConnect
* description     : Send the PPU Connect message to the specified address
*
* @param - None.
*
* @return - None.
*/
bool PacketPpuCommand::SendPpuConnect(const uint16_t nodeID)
{
   static uint8_t buffer[32];
   uint16_t Checksum;
   uint32_t system_id = 107;
   //build the message
   memset(buffer,0,32);
   buffer[0] = 0x00;
   buffer[1] = 0x02; //PPU Master address = 512
   buffer[2] = (uint8_t)(nodeID & 0xFF);
   buffer[3] = (uint8_t)((nodeID >> 8) & 0xFF);
   buffer[4] = (uint8_t)(system_id & 0xFF);
   buffer[5] = (uint8_t)((system_id >> 8) & 0xFF);
   buffer[6] = (uint8_t)((system_id >> 16) & 0xFF);
   buffer[7] = (uint8_t)((system_id >> 24) & 0xFF);
#ifdef EMBEDDED_CHECKSUM
   buffer[8] = 0;//checksum
   buffer[9] = 0; 
   buffer[10] = (uint8_t)(PPUB_MESSAGE_HEADER_LENGTH & 0xFF);//  pkt len
   buffer[11] = (uint8_t)(((uint16_t)PPUB_MESSAGE_HEADER_LENGTH >> 8) & 0xFF);
   buffer[12] = (uint8_t)(PPUB_CMD_PPU_CONNECT & 0xFF);
   buffer[13] = (uint8_t)((PPUB_CMD_PPU_CONNECT >> 8) & 0xFF);
   //Calculate the checksum
   __HAL_RCC_CRC_FORCE_RESET();
   __HAL_RCC_CRC_RELEASE_RESET();
   Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, PPUB_MESSAGE_HEADER_LENGTH);
   buffer[8] = (uint8_t)(Checksum & 0xFF);
   buffer[9] = (uint8_t)((Checksum >> 8) & 0xFF);
#else
   buffer[8] = (uint8_t)(PPUB_MESSAGE_HEADER_LENGTH & 0xFF);// pkt len
   buffer[9] = (uint8_t)(((uint16_t)PPUB_MESSAGE_HEADER_LENGTH >> 8) & 0xFF);
   buffer[10] = (uint8_t)(PPUB_CMD_PPU_CONNECT & 0xFF);
   buffer[11] = (uint8_t)((PPUB_CMD_PPU_CONNECT >> 8) & 0xFF);
   //Calculate the checksum
   __HAL_RCC_CRC_FORCE_RESET();
   __HAL_RCC_CRC_RELEASE_RESET();
   Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, PPUB_MESSAGE_HEADER_LENGTH);
   buffer[12] = (uint8_t)((Checksum >> 8) & 0xFF);
   buffer[13] = (uint8_t)(Checksum & 0xFF);
#endif
   
   serialPort.printf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13]);
   serialPort.printf("crc=%04X\r\n", Checksum);
   //Send over the radio
   m_Frequency = PPUB_RF_CHANNEL;
   SetRadioForTransmit(true);
   Radio.Send( buffer, PPUB_MESSAGE_HEADER_LENGTH );
   m_State = PPU_STATE_WAITING_FOR_RADIO_RESPONSE_E;
   return true;
}

/*************************************************************************************/
/**
* function name   : SendPpuVersionRequest
* description     : Send a PPU Version request to the specified address
*
* @param - nodeID    the Node to send the request to.
*
* @return - None.
*/
bool PacketPpuCommand::SendPpuVersionRequest(const uint16_t nodeID)
{
   static uint8_t buffer[32];
   uint16_t Checksum;
   uint32_t system_id = 107;
   //build the message
   memset(buffer,0,32);
   buffer[0] = 0x00;
   buffer[1] = 0x02; //PPU Master address = 512
   buffer[2] = (uint8_t)(nodeID & 0xFF);
   buffer[3] = (uint8_t)((nodeID >> 8) & 0xFF);
   buffer[4] = (uint8_t)(system_id & 0xFF);
   buffer[5] = (uint8_t)((system_id >> 8) & 0xFF);
   buffer[6] = (uint8_t)((system_id >> 16) & 0xFF);
   buffer[7] = (uint8_t)((system_id >> 24) & 0xFF);
#ifdef EMBEDDED_CHECKSUM
   buffer[8] = 0;//checksum
   buffer[9] = 0; 
   buffer[10] = (uint8_t)(PPUB_MESSAGE_HEADER_LENGTH & 0xFF);//  pkt len
   buffer[11] = (uint8_t)(((uint16_t)PPUB_MESSAGE_HEADER_LENGTH >> 8) & 0xFF);
   buffer[12] = (uint8_t)(PPUB_CMD_PPU_VERSION & 0xFF);
   buffer[13] = (uint8_t)((PPUB_CMD_PPU_VERSION >> 8) & 0xFF);
   //Calculate the checksum
   __HAL_RCC_CRC_FORCE_RESET();
   __HAL_RCC_CRC_RELEASE_RESET();
   Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, PPUB_MESSAGE_HEADER_LENGTH);
   buffer[8] = (uint8_t)(Checksum & 0xFF);
   buffer[9] = (uint8_t)((Checksum >> 8) & 0xFF);
#else
   buffer[8] = (uint8_t)(PPUB_MESSAGE_HEADER_LENGTH & 0xFF);// pkt len
   buffer[9] = (uint8_t)(((uint16_t)PPUB_MESSAGE_HEADER_LENGTH >> 8) & 0xFF);
   buffer[10] = (uint8_t)(PPUB_CMD_PPU_VERSION & 0xFF);
   buffer[11] = (uint8_t)((PPUB_CMD_PPU_VERSION >> 8) & 0xFF);
   //Calculate the checksum
   __HAL_RCC_CRC_FORCE_RESET();
   __HAL_RCC_CRC_RELEASE_RESET();
   Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, PPUB_MESSAGE_HEADER_LENGTH);
   buffer[12] = (uint8_t)((Checksum >> 8) & 0xFF);
   buffer[13] = (uint8_t)(Checksum & 0xFF);
#endif
   
   serialPort.printf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13]);
   serialPort.printf("crc=%04X\r\n", Checksum);
   //Send over the radio
   m_Frequency = PPUB_RF_CHANNEL;
   SetRadioForTransmit(true);
   Radio.Send( buffer, PPUB_MESSAGE_HEADER_LENGTH );
   m_State = PPU_STATE_WAITING_FOR_RADIO_RESPONSE_E;
   return true;
}

/*************************************************************************************/
/**
* function name   : SendPpuReset
* description     : Send a 'normal' Reset command to the specified address
*
* @param - nodeID    the Node to send the reset to.
*
* @return - None.
*/
bool PacketPpuCommand::SendPpuReset(const uint16_t nodeID)
{
   static uint8_t buffer[32];
   uint16_t Checksum;
   uint32_t system_id = 107;
   //build the message
   memset(buffer,0,32);
   buffer[0] = 0x00;
   buffer[1] = 0x02; //PPU Master address = 512
   buffer[2] = (uint8_t)(nodeID & 0xFF);
   buffer[3] = (uint8_t)((nodeID >> 8) & 0xFF);
   buffer[4] = (uint8_t)(system_id & 0xFF);
   buffer[5] = (uint8_t)((system_id >> 8) & 0xFF);
   buffer[6] = (uint8_t)((system_id >> 16) & 0xFF);
   buffer[7] = (uint8_t)((system_id >> 24) & 0xFF);
#ifdef EMBEDDED_CHECKSUM
   buffer[8] = 0;//checksum
   buffer[9] = 0; 
   buffer[10] = (uint8_t)(PPUB_MESSAGE_HEADER_LENGTH & 0xFF);//  pkt len
   buffer[11] = (uint8_t)(((uint16_t)PPUB_MESSAGE_HEADER_LENGTH >> 8) & 0xFF);
   buffer[12] = (uint8_t)(PPUB_CMD_PPU_RESET_NORMAL & 0xFF);
   buffer[13] = (uint8_t)((PPUB_CMD_PPU_RESET_NORMAL >> 8) & 0xFF);
   //Calculate the checksum
   __HAL_RCC_CRC_FORCE_RESET();
   __HAL_RCC_CRC_RELEASE_RESET();
   Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, PPUB_MESSAGE_HEADER_LENGTH);
   buffer[8] = (uint8_t)(Checksum & 0xFF);
   buffer[9] = (uint8_t)((Checksum >> 8) & 0xFF);
#else
   buffer[8] = (uint8_t)(PPUB_MESSAGE_HEADER_LENGTH & 0xFF);// pkt len
   buffer[9] = (uint8_t)(((uint16_t)PPUB_MESSAGE_HEADER_LENGTH >> 8) & 0xFF);
   buffer[10] = (uint8_t)(PPUB_CMD_PPU_RESET_NORMAL & 0xFF);
   buffer[11] = (uint8_t)((PPUB_CMD_PPU_RESET_NORMAL >> 8) & 0xFF);
   //Calculate the checksum
   __HAL_RCC_CRC_FORCE_RESET();
   __HAL_RCC_CRC_RELEASE_RESET();
   Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, PPUB_MESSAGE_HEADER_LENGTH);
   buffer[12] = (uint8_t)((Checksum >> 8) & 0xFF);
   buffer[13] = (uint8_t)(Checksum & 0xFF);
#endif
   
   serialPort.printf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13]);
   serialPort.printf("crc=%04X\r\n", Checksum);
   //Send over the radio
   m_Frequency = PPUB_RF_CHANNEL;
   SetRadioForTransmit(true);
   Radio.Send( buffer, PPUB_MESSAGE_HEADER_LENGTH );
   m_State = PPU_STATE_WAITING_FOR_RADIO_RESPONSE_E;
   return true;
}

/*************************************************************************************/
/**
* function name   : SendPpuResetToRadioUpdate
* description     : Send a Reset For Firmware Update command to the specified address
*
* @param - nodeID    the Node to send the reset to.
*
* @return - None.
*/
bool PacketPpuCommand::SendPpuResetToRadioUpdate(const uint16_t nodeID)
{
   static uint8_t buffer[32];
   uint16_t Checksum;
   uint32_t system_id = 107;
   //build the message
   memset(buffer,0,32);
   buffer[0] = 0x00;
   buffer[1] = 0x02; //PPU Master address = 512
   buffer[2] = (uint8_t)(nodeID & 0xFF);
   buffer[3] = (uint8_t)((nodeID >> 8) & 0xFF);
   buffer[4] = (uint8_t)(system_id & 0xFF);
   buffer[5] = (uint8_t)((system_id >> 8) & 0xFF);
   buffer[6] = (uint8_t)((system_id >> 16) & 0xFF);
   buffer[7] = (uint8_t)((system_id >> 24) & 0xFF);
#ifdef EMBEDDED_CHECKSUM
   buffer[8] = 0;//checksum
   buffer[9] = 0; 
   buffer[10] = (uint8_t)(PPUB_MESSAGE_HEADER_LENGTH & 0xFF);//  pkt len
   buffer[11] = (uint8_t)(((uint16_t)PPUB_MESSAGE_HEADER_LENGTH >> 8) & 0xFF);
   buffer[12] = (uint8_t)(PPUB_CMD_PPU_RESET_TO_BOOT2 & 0xFF);
   buffer[13] = (uint8_t)((PPUB_CMD_PPU_RESET_TO_BOOT2 >> 8) & 0xFF);
   //Calculate the checksum
   __HAL_RCC_CRC_FORCE_RESET();
   __HAL_RCC_CRC_RELEASE_RESET();
   Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, PPUB_MESSAGE_HEADER_LENGTH);
   buffer[8] = (uint8_t)(Checksum & 0xFF);
   buffer[9] = (uint8_t)((Checksum >> 8) & 0xFF);
#else
   buffer[8] = (uint8_t)(PPUB_MESSAGE_HEADER_LENGTH & 0xFF);// pkt len
   buffer[9] = (uint8_t)(((uint16_t)PPUB_MESSAGE_HEADER_LENGTH >> 8) & 0xFF);
   buffer[10] = (uint8_t)(PPUB_CMD_PPU_RESET_TO_BOOT2 & 0xFF);
   buffer[11] = (uint8_t)((PPUB_CMD_PPU_RESET_TO_BOOT2 >> 8) & 0xFF);
   //Calculate the checksum
   __HAL_RCC_CRC_FORCE_RESET();
   __HAL_RCC_CRC_RELEASE_RESET();
   Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, PPUB_MESSAGE_HEADER_LENGTH);
   buffer[12] = (uint8_t)((Checksum >> 8) & 0xFF);
   buffer[13] = (uint8_t)(Checksum & 0xFF);
#endif
   
   serialPort.printf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13]);
   serialPort.printf("crc=%04X\r\n", Checksum);
   //Send over the radio
   m_Frequency = PPUB_RF_CHANNEL;
   SetRadioForTransmit(true);
   Radio.Send( buffer, PPUB_MESSAGE_HEADER_LENGTH );
   m_State = PPU_STATE_WAITING_FOR_RADIO_RESPONSE_E;
   return true;
}

/*************************************************************************************/
/**
* function name   : SendPpuUpdate
* description     : Send a 'Reset to radio update' command to the specified address
*
* @param - nodeID             The Node to send the update to.
* @param - packetSize         The payload size of the update packets.
* @param - numberOfPackets    The number of packets in the update.
*
* @return - None.
*/
bool PacketPpuCommand::SendPpuUpdate(const uint16_t nodeID, const uint16_t packetSize, const uint16_t numberOfPackets)
{
   static uint8_t buffer[32];
   uint16_t Checksum;
   uint32_t system_id = 107;
   uint32_t message_length = PPUB_MESSAGE_HEADER_LENGTH + 4;
   //build the message
   memset(buffer,0,32);
   buffer[0] = 0x00;
   buffer[1] = 0x02; //PPU Master address = 512
   buffer[2] = (uint8_t)(nodeID & 0xFF);
   buffer[3] = (uint8_t)((nodeID >> 8) & 0xFF);
   buffer[4] = (uint8_t)(system_id & 0xFF);
   buffer[5] = (uint8_t)((system_id >> 8) & 0xFF);
   buffer[6] = (uint8_t)((system_id >> 16) & 0xFF);
   buffer[7] = (uint8_t)((system_id >> 24) & 0xFF);
#ifdef EMBEDDED_CHECKSUM
   buffer[8] = 0;//checksum
   buffer[9] = 0; 
   buffer[10] = (uint8_t)(message_length & 0xFF);//  pkt len
   buffer[11] = (uint8_t)(((uint16_t)message_length >> 8) & 0xFF);
   buffer[12] = (uint8_t)(PPUB_CMD_PPU_FIRMWARE_UPDATE_CONFIG & 0xFF);
   buffer[13] = (uint8_t)((PPUB_CMD_PPU_FIRMWARE_UPDATE_CONFIG >> 8) & 0xFF);
   buffer[14] = (uint8_t)(packetSize & 0xFF);
   buffer[15] = (uint8_t)((packetSize >> 8) & 0xFF);
   buffer[16] = (uint8_t)(numberOfPackets & 0xFF);
   buffer[17] = (uint8_t)((numberOfPackets >> 8) & 0xFF);
   //Calculate the checksum
   __HAL_RCC_CRC_FORCE_RESET();
   __HAL_RCC_CRC_RELEASE_RESET();
   Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, message_length);
   buffer[8] = (uint8_t)(Checksum & 0xFF);
   buffer[9] = (uint8_t)((Checksum >> 8) & 0xFF);
#else
   buffer[8] = (uint8_t)(message_length & 0xFF);// pkt len
   buffer[9] = (uint8_t)(((uint16_t)message_length >> 8) & 0xFF);
   buffer[10] = (uint8_t)(PPUB_CMD_PPU_FIRMWARE_UPDATE_CONFIG & 0xFF);
   buffer[11] = (uint8_t)((PPUB_CMD_PPU_FIRMWARE_UPDATE_CONFIG >> 8) & 0xFF);
   buffer[12] = (uint8_t)(packetSize & 0xFF);
   buffer[13] = (uint8_t)((packetSize >> 8) & 0xFF);
   buffer[14] = (uint8_t)(numberOfPackets & 0xFF);
   buffer[15] = (uint8_t)((numberOfPackets >> 8) & 0xFF);
   //Calculate the checksum
   __HAL_RCC_CRC_FORCE_RESET();
   __HAL_RCC_CRC_RELEASE_RESET();
   Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, message_length-2);
   buffer[16] = (uint8_t)((Checksum >> 8) & 0xFF);
   buffer[17] = (uint8_t)(Checksum & 0xFF);
#endif
   
   serialPort.printf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13]);
   serialPort.printf("crc=%04X\r\n", Checksum);
   //Send over the radio
   m_Frequency = PPUB_RF_CHANNEL;
   SetRadioForTransmit(true);
   Radio.Send( buffer, message_length );
   m_State = PPU_STATE_WAITING_FOR_RADIO_RESPONSE_E;
   return true;
}

/*************************************************************************************/
/**
* function name   : SendPpuLostPacketRequest
* description     : Send a 'Lost Packet Request' command to the specified address
*
* @param - nodeID    the Node to send the reset to.
*
* @return - None.
*/
bool PacketPpuCommand::SendPpuLostPacketRequest(const uint16_t nodeID)
{
   static uint8_t buffer[32];
   uint16_t Checksum;
   uint32_t system_id = 107;
   //build the message
   memset(buffer,0,32);
   buffer[0] = 0x00;
   buffer[1] = 0x02; //PPU Master address = 512
   buffer[2] = (uint8_t)(nodeID & 0xFF);
   buffer[3] = (uint8_t)((nodeID >> 8) & 0xFF);
   buffer[4] = (uint8_t)(system_id & 0xFF);
   buffer[5] = (uint8_t)((system_id >> 8) & 0xFF);
   buffer[6] = (uint8_t)((system_id >> 16) & 0xFF);
   buffer[7] = (uint8_t)((system_id >> 24) & 0xFF);
#ifdef EMBEDDED_CHECKSUM
   buffer[8] = 0;//checksum
   buffer[9] = 0; 
   buffer[10] = (uint8_t)(PPUB_MESSAGE_HEADER_LENGTH & 0xFF);//  pkt len
   buffer[11] = (uint8_t)(((uint16_t)PPUB_MESSAGE_HEADER_LENGTH >> 8) & 0xFF);
   buffer[12] = (uint8_t)(PPUB_CMD_PPU_FIRMWARE_BIT_REQ & 0xFF);
   buffer[13] = (uint8_t)((PPUB_CMD_PPU_FIRMWARE_BIT_REQ >> 8) & 0xFF);
   //Calculate the checksum
   __HAL_RCC_CRC_FORCE_RESET();
   __HAL_RCC_CRC_RELEASE_RESET();
   Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, PPUB_MESSAGE_HEADER_LENGTH);
   buffer[8] = (uint8_t)(Checksum & 0xFF);
   buffer[9] = (uint8_t)((Checksum >> 8) & 0xFF);
#else
   buffer[8] = (uint8_t)(PPUB_MESSAGE_HEADER_LENGTH & 0xFF);// pkt len
   buffer[9] = (uint8_t)(((uint16_t)PPUB_MESSAGE_HEADER_LENGTH >> 8) & 0xFF);
   buffer[10] = (uint8_t)(PPUB_CMD_PPU_FIRMWARE_BIT_REQ & 0xFF);
   buffer[11] = (uint8_t)((PPUB_CMD_PPU_FIRMWARE_BIT_REQ >> 8) & 0xFF);
   //Calculate the checksum
   __HAL_RCC_CRC_FORCE_RESET();
   __HAL_RCC_CRC_RELEASE_RESET();
   Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, PPUB_MESSAGE_HEADER_LENGTH);
   buffer[12] = (uint8_t)((Checksum >> 8) & 0xFF);
   buffer[13] = (uint8_t)(Checksum & 0xFF);
#endif
   
   serialPort.printf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13]);
   serialPort.printf("crc=%04X\r\n", Checksum);
   //Send over the radio
   m_Frequency = PPUB_RF_CHANNEL;
   SetRadioForTransmit(true);
   Radio.Send( buffer, PPUB_MESSAGE_HEADER_LENGTH );
   m_State = PPU_STATE_WAITING_FOR_RADIO_RESPONSE_E;
   return true;
}

/*************************************************************************************/
/**
* function name   : SendAtCommand
* description     : Send an AT command to the specified address
*
* @param - nodeID    the Node to send the reset to.
*
* @return - None.
*/
bool PacketPpuCommand::SendAtCommand(const uint16_t nodeID, const char* command)
{
   static uint8_t buffer[256];
   bool result = false;
   uint16_t Checksum;
   uint32_t system_id = 107;
   uint32_t payload_length;
   uint32_t packet_length;
   uint8_t* payload = &buffer[PPUB_MESSAGE_HEADER_LENGTH];
   
   memset(buffer, 0, sizeof(buffer));
   
   if ( command )
   {
      payload_length = strlen(command) - 1;
      if ( payload_length > 0 )
      {
         packet_length = payload_length + PPUB_MESSAGE_HEADER_LENGTH;
         //build the message
         buffer[0] = 0x00;
         buffer[1] = 0x02; //PPU Master address = 512
         buffer[2] = (uint8_t)(nodeID & 0xFF);
         buffer[3] = (uint8_t)((nodeID >> 8) & 0xFF);
         buffer[4] = (uint8_t)(system_id & 0xFF);
         buffer[5] = (uint8_t)((system_id >> 8) & 0xFF);
         buffer[6] = (uint8_t)((system_id >> 16) & 0xFF);
         buffer[7] = (uint8_t)((system_id >> 24) & 0xFF);
#ifdef EMBEDDED_CHECKSUM
         buffer[8] = 0;//checksum
         buffer[9] = 0; 
         buffer[10] = (uint8_t)(packet_length & 0xFF);//  pkt len
         buffer[11] = (uint8_t)(((uint16_t)packet_length >> 8) & 0xFF);
         buffer[12] = (uint8_t)(PPUB_CMD_CONFIG & 0xFF);
         buffer[13] = (uint8_t)((PPUB_CMD_CONFIG >> 8) & 0xFF);
         memcpy((char*)payload, command, payload_length);
         //Calculate the checksum
         __HAL_RCC_CRC_FORCE_RESET();
         __HAL_RCC_CRC_RELEASE_RESET();
         Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, packet_length);
         buffer[8] = (uint8_t)(Checksum & 0xFF);
         buffer[9] = (uint8_t)((Checksum >> 8) & 0xFF);
#else
         buffer[8] = (uint8_t)(packet_length & 0xFF);//pkt length
         buffer[9] = (uint8_t)((packet_length >> 8) & 0xFF);
         buffer[10] = (uint8_t)(PPUB_CMD_CONFIG & 0xFF);
         buffer[11] = (uint8_t)((PPUB_CMD_CONFIG >> 8) & 0xFF);
         strcpy((char*)payload, command);
         //Calculate the checksum
         __HAL_RCC_CRC_FORCE_RESET();
         __HAL_RCC_CRC_RELEASE_RESET();
         Checksum =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)buffer, packet_length);
         buffer[packet_length] = (uint8_t)((Checksum >> 8) & 0xFF);
         buffer[packet_length+1] = (uint8_t)(Checksum & 0xFF);
         packet_length += 2;
#endif

         serialPort.printf("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\r\n",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10],buffer[11],buffer[12],buffer[13]);
         serialPort.printf("crc=%04X\r\n", Checksum);
         //Send over the radio
         m_Frequency = PPUB_RF_CHANNEL;
         SetRadioForTransmit(true);
         Radio.Send( buffer, packet_length );
         m_State = PPU_STATE_WAITING_FOR_RADIO_RESPONSE_E;
         result = true;
      }
   }
   return result;
}


