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
*  File         : RxSurveyTool.cpp
*
*  Description  : Source for the Survey Tool Receiver.
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include "lptim.h"


/* User Include Files
*************************************************************************************/
#include "RxSurveyTool.h"

/* User Definitions
*************************************************************************************/

//Disable this to send debug reports over the usart instead of the survey tool lights
//Note that enabling this reconfigures the MCU pins and the code must be run on the
//survey tool hardware.  It doesn't work on a standard RBU.
//#define ENABLE_LEDS

#define SHOW_DEBUG 0
//#define USE_PPU_FOR_DEBUG_OUT

#define NUM_FREQUENCY_CHANS 10
uint32_t const frequencyChannels[NUM_FREQUENCY_CHANS] = {865150000, 865450000, 865750000, 866050000, 866350000,
                                                         866650000, 866950000, 867250000, 867550000, 867850000};

#define RF_CHANNEL                                  5

#define PACKET_MODE                                 true
#define USE_CAD                                     0       // 0 : do not use CAD, 1 : use CAD
#define LORA_BANDWIDTH                              1       // [0: 125 kHz,
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


#define RX_TIMEOUT_VALUE               3000000   // in us

//This is used to determine whether a Rx Timeout occurs because there was no message or because a received message failed to decode.
#define MESSAGE_ON_AIR_TIME 185 //LPTIM ticks for 12 preamble symbols + 14 char payload
#define MESSAGE_TIMEOUT_THRESHOLD ((RX_TIMEOUT_VALUE / 61) - MESSAGE_ON_AIR_TIME)

#define NUM_PER_SAMPLES 100

#define NUMBER_OF_LEDS 11  //10 static LEDs plus 'no signal' flash for index 0

#define RSSI_BANK 0
#define SNR_BANK 1
#define TIMER_INTERVAL_us 1250
#define BANK_TRANSFER_RATE 5
#define NO_SIGNAL_FLASH_INTERVAL 100
#define NO_SIGNAL_THRESHOLD 2400

/* Type Declarations
*************************************************************************************/
Ticker RxSurveyToolTimer;
static CRC_HandleTypeDef crc_handle_8b_data_16b_crc;
static bool dropped_message[NUM_PER_SAMPLES];
static bool NoSignal = true;
static uint32_t  timer_count_since_last_signal = 0;
static int16_t rxd_rssi;
static int8_t rxd_snr;
#ifdef ENABLE_LEDS
static uint32_t no_signal_led_1 = 0;
static uint32_t current_bank = RSSI_BANK;
static uint32_t bank_transfer_count = 0;
static uint32_t timer_count_flash_control = 0;
#endif
/* Global Variables
*************************************************************************************/
extern Serial serialPort;  //enable to put debug out of the PPU port @ 2MHz baud using serialPort.printf("txt %d\r\n",value);  the debug usart isn't connected in the survey tool.

static int32_t RssiTable[][NUMBER_OF_LEDS] = 
{
   {-122, -119,-116,-113,-110,-107,-104,-101,-98,-95,-92},//STRX_MODE_SITENET_TO_SITENET_E
   {-116, -113,-110,-107,-104,-101,-98,-95,-92,-89,-86},//STRX_MODE_SITENET_TO_SMARTNET_E
   {-110,-107,-104,-101,-98,-95,-92,-89,-86,-83,-80} //STRX_MODE_SMARTNET_TO_SMARTNET_E
};

static int32_t SnrTable[][NUMBER_OF_LEDS] = 
{
   {-5,-3,-1,1,3,5,7,9,11,13,15},//STRX_MODE_SITENET_TO_SITENET_E
   {-5,-3,-1,1,3,5,7,9,11,13,15},//STRX_MODE_SITENET_TO_SMARTNET_E
   {-5,-3,-1,1,3,5,7,9,11,13,15} //STRX_MODE_SMARTNET_TO_SMARTNET_E
};

#ifdef ENABLE_LEDS

//LED outputs
DigitalOut led_1(PC_11);
DigitalOut led_2(PA_11);
DigitalOut led_3(PB_14);
DigitalOut led_4(PB_1);
DigitalOut led_5(PB_13);
DigitalOut led_6(PC_10);
DigitalOut led_7(PA_8);
DigitalOut led_8(PC_1);
DigitalOut led_9(PC_4);
DigitalOut led_10(PB_6);

//Display Select
DigitalOut display_select(PC_9);

//Select line inputs (selcts SightNet-SiteNet, SiteNet-SmartNet, SmartNet-SmartNet)
DigitalIn sel_A(PA_0);
DigitalIn sel_B(PC_0);

#endif

/*************************************************************************************/
/**
* function name   : PacketLossTest
* description     : Constructor
*
* @param - None.
*
* @return - nothing
*/
SurveyToolRx::SurveyToolRx( void ) : RxTestBase(), 
   m_SystemID(0xFFFFFFFF),
   m_Count(0),
   m_Event(STRX_EVENT_NONE_E),
   m_LastMessageErrorDetected(false),
   m_Frequency(RF_CHANNEL),
   m_SpreadingFactor(LORA_SPREADING_FACTOR),
   m_CodingRate(LORA_CODINGRATE),
   m_LoraCrc(LORA_CRC_ENABLED),
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
* function name   : SurveyToolRx
* description     : Destructor
*
* @param - None.
*
* @return - nothing
*/
SurveyToolRx::~SurveyToolRx( void )
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
bool SurveyToolRx::Initialise( void )
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
   NoSignal = true;
#ifdef ENABLE_LEDS
   no_signal_led_1 = 0;
   bank_transfer_count = 0;
   current_bank = RSSI_BANK;
   timer_count_flash_control = 0;
#endif
   
   //initialise timer
   MX_LPTIM1_Init();
  
   Radio.SetChannel( frequencyChannels[m_Frequency] );
   uint8_t payload_length = STRX_MESSAGE_LENGTH;
   if( !m_LoraCrc )
   {
      payload_length = STRX_MESSAGE_LENGTH_WITH_CRC;
   }
   
   Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, m_SpreadingFactor,
                      m_CodingRate, 0, LORA_PREAMBLE_LENGTH,
                      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, payload_length,
                      m_LoraCrc, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                      LORA_IQ_INVERSION_ON, true, PACKET_MODE );

#ifndef ENABLE_LEDS
   #ifdef USE_PPU_FOR_DEBUG_OUT
   serialPort.printf( "\r\n\n     Survey Tool Rx\r\n\n",NULL );
   serialPort.printf( "Frequency          = %d (%d)\r\n", m_Frequency, frequencyChannels[m_Frequency]);
   serialPort.printf( "Spreading Factor   = %d\r\n",m_SpreadingFactor);
   serialPort.printf( "Coding Rate        = 4/%d\r\n\n",(m_CodingRate+4));
   #else
   debug( "\r\n\n     Survey Tool Rx\r\n\n",NULL );
   debug( "Frequency          = %d (%d)\r\n", m_Frequency, frequencyChannels[m_Frequency]);
   debug( "Spreading Factor   = %d\r\n",m_SpreadingFactor);
   debug( "Coding Rate        = 4/%d\r\n\n",(m_CodingRate+4));
   #endif
#else
   led_1 = 0;
   led_2 = 0;
   led_3 = 0;
   led_4 = 0;
   led_5 = 0;
   led_6 = 0;
   led_7 = 0;
   led_8 = 0;
   led_9 = 0;
   led_10 = 0;
   sel_A.mode(PullUp);
   sel_B.mode(PullUp);
#endif   
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
#ifndef ENABLE_LEDS
#ifdef USE_PPU_FOR_DEBUG_OUT
         serialPort.printf( "CRC module initialisation failed!\n\r", NULL );
#else
         debug( "CRC module initialisation failed!\n\r", NULL );
#endif
#endif
         wait( 1 );
         status = false;
      }
   }
   
   ResetRadio();
   
   m_EventTime = LPTIM_ReadCounter(&hlptim1);// ************************* This Fn never returns because the timer advances too fast to get two identical readings

   //Initialise the timer for the flashing 'no signal' LED.
   RxSurveyToolTimer.detach();
   RxSurveyToolTimer.attach_us(Callback<void(void)>(this, &SurveyToolRx::TimerCallback), TIMER_INTERVAL_us);

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
void SurveyToolRx::StartReceiving(void)
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
void SurveyToolRx::ResetRadio(void)
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
bool SurveyToolRx::RunProcess( void )
{
   switch ( m_Event )
   {
      case STRX_EVENT_RXDONE_E:
         {
            ResetRadio();
//            serialPort.printf("RxDone1\r\n");
            SurveyToolMessage_t* pMessage = (SurveyToolMessage_t*)m_Buffer;
            //Make sure it's our system
            if ( pMessage->SystemID == m_SystemID )
            {
               timer_count_since_last_signal = 0;
               m_RssiValue = rxd_rssi;
               m_SnrValue = rxd_snr;
               m_PayloadNumber = atoi(( const char* )pMessage->payload);
               uint32_t per_index = m_PayloadNumber % NUM_PER_SAMPLES;
               
//               serialPort.printf("Msg %d r=%d s=%d\r\n", m_PayloadNumber, m_RssiValue, m_SnrValue);
#ifndef ENABLE_LEDS
   #ifdef USE_PPU_FOR_DEBUG_OUT
         serialPort.printf( "Msg %d rssi=%d snr=%d\r\n", m_PayloadNumber, m_RssiValue, m_SnrValue);
   #else
         debug( "Msg %d rssi=%d snr=%d\r\n", m_PayloadNumber, m_RssiValue, m_SnrValue);
   #endif
#endif               
               DeviceMode_t mode = GetDeviceMode();
               if ( RssiTable[mode][1] < m_RssiValue )
               {
                  NoSignal = false;
               }

               
               
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
              
               m_PreviousEventTime = m_EventTime;
               
            }
            
            m_Event = STRX_EVENT_NONE_E;
            
         }
         break;
      case STRX_EVENT_RXTIMEOUT_E:
      {
         ResetRadio();
         m_Event = STRX_EVENT_NONE_E;
 #ifndef ENABLE_LEDS
//      debug("T\r\n");
#endif
     }
         break;
      case STRX_EVENT_RXERROR_E:
         ResetRadio();
#ifndef ENABLE_LEDS
//      debug("E\r\n");
#endif
      m_Event = STRX_EVENT_NONE_E;
         break;
      case STRX_EVENT_RXCRCERROR_E:
         ResetRadio();
#ifndef ENABLE_LEDS
//      debug("CRC\r\n");
#endif
      m_Event = STRX_EVENT_NONE_E;
         break;
      case STRX_EVENT_CADDONE_E:
         StartReceiving();
         m_Event = STRX_EVENT_NONE_E;
         break;
      case STRX_EVENT_CHANNEL_E:
         ResetRadio();
         m_Event = STRX_EVENT_NONE_E;
         break;
      case STRX_EVENT_TIMER_E:
      {
         m_Event = STRX_EVENT_NONE_E;
      }
         break;

      default:
         break;
   };
   
   
   return true;
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
void SurveyToolRx::TimerCallback(void)
{
#ifdef ENABLE_LEDS
   static uint32_t index = 0;
   
   
   if ( NO_SIGNAL_THRESHOLD < timer_count_since_last_signal )
   {
      NoSignal = true;
   }
   else 
   {
      timer_count_since_last_signal++;
   }
   
   
      //Change the state of the 'no signal' flashing LED
   if ( NoSignal )
   {
      timer_count_flash_control++;
      if ( NO_SIGNAL_FLASH_INTERVAL < timer_count_flash_control )
      {
         timer_count_flash_control = 0;
         if ( 0 == no_signal_led_1 )
         {
            no_signal_led_1 = 1;
         }
         else 
         {
            no_signal_led_1 = 0;
         }
      }
   }
         
         
         
   //Periodically switch between the RSSI LED bank and the SNR LED bank and update the LEDs
   bank_transfer_count++;
   if ((BANK_TRANSFER_RATE-1) == bank_transfer_count)
   {
      led_1 = 0;
      led_2 = 0;
      led_3 = 0;
      led_4 = 0;
      led_5 = 0;
      led_6 = 0;
      led_7 = 0;
      led_8 = 0;
      led_9 = 0;
      led_10 = 0;
   }
   else if (BANK_TRANSFER_RATE == bank_transfer_count)
   {
      if ( RSSI_BANK == current_bank )
      {
         current_bank = SNR_BANK;
      }
      else
      {
         current_bank = RSSI_BANK;
      }
      
      display_select = current_bank;
      
      if ( false == NoSignal )
      {
         if ( RSSI_BANK == current_bank )
         {
            index = GetRssiIndex(m_RssiValue);
         }
         else
         {
            index = GetSnrIndex(m_SnrValue);
         }
      }
   }
   else if (BANK_TRANSFER_RATE < bank_transfer_count)
   {
      bank_transfer_count = 0;
      
      if ( NoSignal )
      {
         led_1 = no_signal_led_1;
         led_2 = 0;
         led_3 = 0;
         led_4 = 0;
         led_5 = 0;
         led_6 = 0;
         led_7 = 0;
         led_8 = 0;
         led_9 = 0;
         led_10 = 0;
      }
      else 
      {
         SetLeds(index);
      }
   }
      
#else
         static uint8_t led_1 = 0;

      if ( 10 < timer_count_since_last_signal )
      {
         if ( NoSignal )
         {
            if ( 0 == led_1 )
            {
               led_1 = 1;
            }
            else 
            {
               led_1 = 0;
            }
            //debug(" LED1 FLASH %d\r\n", led_1);
         }
      }
#endif
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
void SurveyToolRx::OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror)
{
   m_EventTime = LPTIM_ReadCounter(&hlptim1);// ************************* This Fn never returns because the timer advances too fast to get two identical readings
   
   Radio.Sleep();
//   serialPort.printf("RxDone\r\n");
   memset(m_Buffer,0, STRX_MESSAGE_LENGTH_WITH_CRC);
   memcpy( m_Buffer, payload, STRX_MESSAGE_LENGTH_WITH_CRC );
//#ifdef ENABLE_LEDS
//   rxd_rssi = rssi;
//#else
//   rxd_rssi = rssi - 45;//offset to reduce signal on test bench
//#endif
   rxd_rssi = rssi;
   rxd_snr = snr;
   m_Ferror = ferror;
   m_FeiValue = FeiVal;

   if ( !m_LoraCrc )
   {
      uint16_t CrcValue =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)&m_Buffer, STRX_MESSAGE_LENGTH);
      uint16_t* pMsgCRC = (uint16_t*)&m_Buffer[STRX_MESSAGE_LENGTH];
      
      m_CrcValue = *pMsgCRC;
      if ( m_CrcValue != CrcValue )
      {
         m_LastMessageErrorDetected = true;
         m_RxErrorCount++;
         m_Event = STRX_EVENT_RXCRCERROR_E;
//         debug_if( SHOW_DEBUG, "> OnRxError mcu crc=%x\n\r", CrcValue );
      }
      else
      {
         m_RxDoneCount++;
//         debug_if( SHOW_DEBUG, "> OnRxDone crc=%x\n\r", CrcValue );
         m_Event = STRX_EVENT_RXDONE_E;
      }
   }
   else
   {
      m_Event = STRX_EVENT_RXDONE_E;
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
void SurveyToolRx::OnRxTimeout( void )
{
   m_EventTime = LPTIM_ReadCounter(&hlptim1);
   Radio.Sleep();
   m_RxTimeoutCount++;
   m_Event = STRX_EVENT_RXTIMEOUT_E;
//   serialPort.printf("RxT\r\n");
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
void SurveyToolRx::OnRxError( void )
{
   m_EventTime = LPTIM_ReadCounter(&hlptim1);
   Radio.Sleep();
   m_RxErrorCount++;
   m_Event = STRX_EVENT_RXERROR_E;
   m_LastMessageErrorDetected = true;
//   serialPort.printf("RxE\r\n");
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
void SurveyToolRx::OnCadDone( bool channelActivityDetected )
{
   m_EventTime = LPTIM_ReadCounter(&hlptim1);
   Radio.Sleep();
   m_CadDoneCount++;
   m_Event = STRX_EVENT_CADDONE_E;
//   serialPort.printf("RxC\r\n");
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
void SurveyToolRx::OnFhssChangeChannel( uint8_t channelIndex )
{
   m_EventTime = LPTIM_ReadCounter(&hlptim1);
   Radio.Sleep();
   m_ChannelChangeCount++;
   m_Event = STRX_EVENT_CHANNEL_E;
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
bool SurveyToolRx::ProcessCommand(const char* pCommand)
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
void SurveyToolRx::PrintHelp(Serial &serialPort)
{
   serialPort.printf("Currently loaded test = SurveyToolRx\r\n\n");
   serialPort.printf("Supported commands:\r\n");
   serialPort.printf("frequency=x (%d)\r\n", m_Frequency);
   serialPort.printf("sf=x (%d)\r\n", m_SpreadingFactor);
   serialPort.printf("cr=x (%d)  (1=4/5, 2=4/6, 3=4/7, 4=4/8\r\n", m_CodingRate);
   serialPort.printf("crc=s (%s) (lora or mcu)\r\n", m_LoraCrc ? "lora":"mcu");
   serialPort.printf("reset\r\n");
   serialPort.printf("\r\n\n\n");
}


/*************************************************************************************/
/**
* function name   : GetDeviceMode
* description     : Read the selection switch to determine the device mode being tested
*
* @param - None
*
* @return - DeviceMode_t      The device mode
*/
DeviceMode_t SurveyToolRx::GetDeviceMode(void)
{
   DeviceMode_t device_mode = STRX_MODE_SMARTNET_TO_SMARTNET_E;
#ifdef ENABLE_LEDS
   if ( (0 == sel_A) && (0 == sel_B) )
   {
      device_mode = STRX_MODE_SITENET_TO_SITENET_E;
   }
   else if ( (1 == sel_A) && (0 == sel_B) )
   {
      device_mode = STRX_MODE_SITENET_TO_SMARTNET_E;
   }
   else if ( (0 == sel_A) && (1 == sel_B) )
   {
      device_mode = STRX_MODE_SMARTNET_TO_SMARTNET_E;
   }
   else if ( (1 == sel_A) && (1 == sel_B) )
   {
      device_mode = STRX_MODE_SMARTNET_TO_SMARTNET_E;
   }
   
#endif
   return device_mode;
}

/*************************************************************************************/
/**
* function name   : SetLeds
* description     : Light the LEDs on 'display_select' up to the level indicated by
*                   m_RssiValue or m_SnrValue as required
*
* @param - index  How many LEDs to light up
*
* @return - nothing
*/
void SurveyToolRx::SetLeds(const uint32_t index)
{
#ifdef ENABLE_LEDS
   
   switch ( index )
   {
      case 1:
         led_1 = 1;
         led_2 = 0;
         led_3 = 0;
         led_4 = 0;
         led_5 = 0;
         led_6 = 0;
         led_7 = 0;
         led_8 = 0;
         led_9 = 0;
         led_10 = 0;
      break;
      case 2:
         led_1 = 1;
         led_2 = 1;
         led_3 = 0;
         led_4 = 0;
         led_5 = 0;
         led_6 = 0;
         led_7 = 0;
         led_8 = 0;
         led_9 = 0;
         led_10 = 0;
      break;
      case 3:
         led_1 = 1;
         led_2 = 1;
         led_3 = 1;
         led_4 = 0;
         led_5 = 0;
         led_6 = 0;
         led_7 = 0;
         led_8 = 0;
         led_9 = 0;
         led_10 = 0;
      break;
      case 4:
         led_1 = 1;
         led_2 = 1;
         led_3 = 1;
         led_4 = 1;
         led_5 = 0;
         led_6 = 0;
         led_7 = 0;
         led_8 = 0;
         led_9 = 0;
         led_10 = 0;
      break;
      case 5:
         led_1 = 1;
         led_2 = 1;
         led_3 = 1;
         led_4 = 1;
         led_5 = 1;
         led_6 = 0;
         led_7 = 0;
         led_8 = 0;
         led_9 = 0;
         led_10 = 0;
      break;
      case 6:
         led_1 = 1;
         led_2 = 1;
         led_3 = 1;
         led_4 = 1;
         led_5 = 1;
         led_6 = 1;
         led_7 = 0;
         led_8 = 0;
         led_9 = 0;
         led_10 = 0;
      break;
      case 7:
         led_1 = 1;
         led_2 = 1;
         led_3 = 1;
         led_4 = 1;
         led_5 = 1;
         led_6 = 1;
         led_7 = 1;
         led_8 = 0;
         led_9 = 0;
         led_10 = 0;
      break;
      case 8:
         led_1 = 1;
         led_2 = 1;
         led_3 = 1;
         led_4 = 1;
         led_5 = 1;
         led_6 = 1;
         led_7 = 1;
         led_8 = 1;
         led_9 = 0;
         led_10 = 0;
      break;
      case 9:
         led_1 = 1;
         led_2 = 1;
         led_3 = 1;
         led_4 = 1;
         led_5 = 1;
         led_6 = 1;
         led_7 = 1;
         led_8 = 1;
         led_9 = 1;
         led_10 = 0;
      break;
      case 10:
         led_1 = 1;
         led_2 = 1;
         led_3 = 1;
         led_4 = 1;
         led_5 = 1;
         led_6 = 1;
         led_7 = 1;
         led_8 = 1;
         led_9 = 1;
         led_10 = 1;
      break;
      case 0: // intentional drop-through
      default:
         if ( RSSI_BANK == current_bank )
         {
            NoSignal = true;
         }
         break;
   }
#else
   uint32_t rssi_index = GetRssiIndex(m_RssiValue);
   debug("LED %d\r\n", rssi_index);
   if ( 0 == index )
   {
      NoSignal = true;
   }
#endif
}


/*************************************************************************************/
/**
* function name   : GetRssiIndex
* description     : Use the param 'rssi' to determine how many LEDs should be lit.
*
* @param - rssi         The received rssi.
*
* @return - uint32_t    The number of LEDs to light.
*/
uint32_t SurveyToolRx::GetRssiIndex(const int16_t rssi)
{
   uint32_t index = 0;
   DeviceMode_t mode = GetDeviceMode();
   uint32_t max_value = NUMBER_OF_LEDS - 1;
   
   for ( index = 0; index < NUMBER_OF_LEDS; index++)
   {
      if ( RssiTable[mode][index] > rssi )
      {
         break;
      }
   }
   
   if ( index > max_value )
   {
      index = max_value;
   }
   
   return index;
}


/*************************************************************************************/
/**
* function name   : GetSnrIndex
* description     : Use the param 'snr' to determine how many LEDs should be lit.
*
* @param - rssi         The received snr.
*
* @return - uint32_t    The number of LEDs to light.
*/
uint32_t SurveyToolRx::GetSnrIndex(const int8_t snr)
{
   uint32_t index = 0;
   uint32_t max_value = NUMBER_OF_LEDS - 1;
   DeviceMode_t mode = GetDeviceMode();
   
   for ( index = 0; index < NUMBER_OF_LEDS; index++)
   {
      if ( SnrTable[mode][index] > snr )
      {
         break;
      }
   }
   
   if ( index > max_value )
   {
      index = max_value;
   }

   return index;
}


