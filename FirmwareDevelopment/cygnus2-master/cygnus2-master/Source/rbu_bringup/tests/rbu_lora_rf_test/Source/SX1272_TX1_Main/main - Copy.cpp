#include <stdio.h>
#include "mbed.h"
#include "main.h"
#include "sx1272-hal.h"
#include "mbed_debug.h"
#include "main_global.h"

// To compile for mbed board, remove -DTARGET_RBU from C/C++ Misc Controls.

//#define CONTINUOUS_TRANSMIT

/* CRC handler declaration */
static CRC_HandleTypeDef crc_handle_8b_data_16b_crc;
uint16_t gCRC = 0;
#define NUM_FREQUENCY_CHANS 10
uint32_t const frequencyChannels[NUM_FREQUENCY_CHANS] = {865150000, 865450000, 865750000, 866050000, 866350000,
                                                         866650000, 866950000, 867250000, 867550000, 867850000};

#define RF_CHANNEL                                       5
#define TX_OUTPUT_POWER                                  8

const uint32_t RF_FREQUENCY = frequencyChannels[RF_CHANNEL];

#define USE_SEQUENCER 1 //0 =  don't activate sequencer in continuous data mode.  1 = enable the 1273 sequencer so DCLK comes out on DIO1
#define PACKET_MODE                                   1         // set to 0 for Tx Continuous mode, 1 for Packet mode
#define SEND_INCREMENTING_PAYLOAD                     0
#define SEND_TEST_PACKETS                             1
#define SEND_TEST_PACKETS_REPEAT                      1
                                                         
#ifdef CONTINUOUS_TRANSMIT


#define TX_TIMEOUT                                  0xFFFFFFF //Max timeout
#define FSK_FDEV                                    25000     // Hz
#define FSK_DATARATE                                19200     // bps
#define FSK_BANDWIDTH                               50000     // Hz
#define FSK_AFC_BANDWIDTH                           83333     // Hz
#define FSK_PREAMBLE_LENGTH                         12        // Same for Tx and Rx
#define FSK_FIX_LENGTH_PAYLOAD_ON                   true
#define FSK_CRC_ENABLED                             false

/*
 *  Global variables declarations
 */
SX1272MB2xAS Radio( NULL );
/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;


/*!
 * \brief Function executed on Radio Tx Timeout event
 */
void OnRadioTxTimeout( void )
{
    // Restarts continuous wave transmission when timeout expires
    Radio.SetTxContinuousMode( RF_FREQUENCY, TX_OUTPUT_POWER, FSK_FDEV, FSK_BANDWIDTH, FSK_DATARATE, false, TX_TIMEOUT );
    Radio.Send(NULL,0);
}


int main( void )
{
   if ( RF_CHANNEL >= NUM_FREQUENCY_CHANS )
   {
      debug("\r\n\r\nERROR : RF_CHANNEL out of bounds (%d/%d)\r\n\r\n", RF_CHANNEL, NUM_FREQUENCY_CHANS-1);
      while(1){};
   }

   debug( "\n\n\r     SX1272 TX Continuous \n\n\r" );
   debug( "Frequency = %d\r\n", RF_FREQUENCY);
   debug( "Tx Power  = %d\r\n", TX_OUTPUT_POWER);


    // Radio initialization
    RadioEvents.TxTimeout = OnRadioTxTimeout;
    Radio.Init( &RadioEvents );


    // Set the radio in CW Tx mode (unmodulated carrier)
    Radio.SetTxContinuousMode( RF_FREQUENCY, TX_OUTPUT_POWER, FSK_FDEV, FSK_BANDWIDTH, FSK_DATARATE, false, TX_TIMEOUT );
    Radio.Send(NULL,0);
       
    while( 1 )
    {
    }
}

#else
/* Set this flag to '1' to display debug messages on the console */
#define DEBUG_MESSAGE   1


/* Set this flag to '1' to use the LoRa modulation or to '0' to use FSK modulation */
#define USE_MODEM_LORA  1
#define USE_MODEM_FSK   !USE_MODEM_LORA


#if USE_MODEM_LORA == 1

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
    #define LORA_CRC_ENABLED                            true
    
#elif USE_MODEM_FSK == 1

   #define FSK_FDEV                                    25000     // Hz
   #define FSK_DATARATE                                19200     // bps
   #define FSK_BANDWIDTH                               50000     // Hz
   #define FSK_AFC_BANDWIDTH                           83333     // Hz
   #define FSK_PREAMBLE_LENGTH                         12        // Same for Tx and Rx
   #define FSK_FIX_LENGTH_PAYLOAD_ON                   true
   #define FSK_CRC_ENABLED                             false

#else
    #error "Please define a modem in the compiler options."
#endif

#define RX_TIMEOUT_VALUE                                3500000   // in us
#define BUFFER_SIZE                                     12        // Define the payload size here
#define BUFFER_SIZE_WITH_CRC                            BUFFER_SIZE + 2

#if( defined ( TARGET_KL25Z ) || defined ( TARGET_LPC11U6X ) )
//DigitalOut led(LED2);
#else
//DigitalOut led(LED1);
#endif

DigitalOut led_r(CN_STATUS_LED1);
DigitalOut led_g(CN_STATUS_LED2);
DigitalOut led_b(CN_STATUS_LED3);

#if defined ( TARGET_RBU )
DigitalOut ant_tx_nrx(ANT_TX_NRX);
DigitalOut ant_rx_ntx(ANT_RX_NTX);
DigitalOut cn_head_pwr(CN_HEAD_PWR_ON);
#endif

DigitalIn Tamper(PA_8, PullNone);


/*
 *  Global variables declarations
 */
typedef enum
{
    LOWPOWER = 0,
    IDLE,
    
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    
    TX,
    TX_TIMEOUT,
    
    CAD,
    CAD_DONE
}AppStates_t;

volatile AppStates_t State = TX;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*
 *  Global variables declarations
 */
SX1272MB2xAS Radio( NULL );

const uint8_t TestMsg[] = "ONE1";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE_WITH_CRC] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
//uint8_t Buffer[BUFFER_SIZE_WITH_CRC] = { 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x00, 0x00 };
//uint8_t Buffer[BUFFER_SIZE_WITH_CRC] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 0x00, 0x00 };

int16_t RssiValue = 0.0;
int8_t SnrValue = 0.0;

uint32_t RxDone_Count = 0;
uint32_t RxTimeout_Count = 0;
uint32_t RxError_Count = 0;
uint32_t TxDone_Count = 0;
uint32_t TxTimeout_Count = 0;
uint32_t CADDone_Count = 0;
uint32_t CADDetected_Count = 0;
uint32_t FHSS_Count = 0;

uint32_t Transmit_Number = 200;

bool channelActivityDetectedFlag = false;
bool RestartPacketTest = false;

int32_t PreviousSwitchState = 0;
int32_t PreviousLoRaClock = 0;

int main() 
{
   if ( RF_CHANNEL >= NUM_FREQUENCY_CHANS )
   {
      debug("\r\n\r\nERROR : RF_CHANNEL out of bounds (%d/%d)\r\n\r\n", RF_CHANNEL, NUM_FREQUENCY_CHANS-1);
      while(1){};
   }
   
    set_time(0);
    time_t current_time_secs;

    uint8_t i;

    led_r = 0;
    led_g = 0;
    led_b = 1;
    
   // #if defined ( TARGET_RBU )
	// Antenna select
    
   // cn_head_pwr = 1;
   // #endif

    debug( "\n\n\r     SX1272 TX Only - 1 : Main \n\n\r" );

    // Initialize Radio driver
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.FhssChangeChannel = OnFhssChangeChannel;
    Radio.Init( &RadioEvents );
    
    // verify the connection with the board
    while( Radio.Read( REG_VERSION ) == 0x00  )
    {
        debug( "Radio could not be detected!\n\r", NULL );
        wait( 1 );
    }
            
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
      }
   }

   debug_if( ( DEBUG_MESSAGE & ( Radio.DetectBoardType( ) == SX1272MB2XAS ) ) , "\n\r > Board Type: SX1272MB2xAS < \n\r" );
    
    Radio.SetChannel( RF_FREQUENCY ); 
    
    debug("\n\n\r> Tx Power %d", TX_OUTPUT_POWER);
    debug("\n\n\r> Frequency channel %d (%dHz)", RF_CHANNEL, RF_FREQUENCY);

#if USE_MODEM_LORA == 1
    
    debug_if( LORA_FHSS_ENABLED, "\n\n\r             > LORA FHSS Mode < \n\n\r");
    debug_if( !LORA_FHSS_ENABLED, "\n\n\r             > LORA Mode < \n\n\r");

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                         LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                         LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                         LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                         LORA_IQ_INVERSION_ON, 2000000, true );
                         
#elif USE_MODEM_FSK == 1

    if ( PACKET_MODE )
    {
       debug("\n\n\r              > FSK Packet Mode < \n\n\r");
       
       Radio.SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, FSK_BANDWIDTH,
                            FSK_DATARATE, 0,
                            FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                            FSK_CRC_ENABLED, false, 0, false, 2000000, true );
    }
    else 
    {
       Radio.EnableContinouousDatabitToggle(false);
       debug("\n\n\r              > FSK Continuous Data Mode < \n\n\r");
       Radio.SetTxContinuousMode( RF_FREQUENCY, TX_OUTPUT_POWER, FSK_FDEV, FSK_BANDWIDTH, FSK_DATARATE, USE_SEQUENCER, 0xffffffff );
       Radio.Send(NULL,0);
    }
                         
#else

#error "Please define a modem in the compiler options."

#endif
     
   debug_if( DEBUG_MESSAGE, "Starting transmit loop\r\n" ); 
        
   ant_tx_nrx = 1;
   ant_rx_ntx = 0;

   uint32_t message_count = 0;
    
   while( 1 )
   {
      switch( State )
      {
        case TX:
//            debug( "*TX * TX:%u, TXT:%u, FHSS:%u\r\n", TxDone_Count, TxTimeout_Count, FHSS_Count);
            led_r = 0;
            led_g = 1;
            led_b = 0;
            if ( SEND_TEST_PACKETS )
            {
               if( TxDone_Count < Transmit_Number) 
               {
                   if( BufferSize > 0 )
                   {
                     // Send the next frame
                     strcpy( ( char* )Buffer, ( char* )TestMsg );
                     // We fill the buffer with numbers for the payload 
                     for( i = 4; i < BufferSize; i++ )
                     {
                        Buffer[i] = i - 4;
                     }
                     if ( !LORA_CRC_ENABLED )
                     {
                        
                        Buffer[BUFFER_SIZE - 1] = 0;
                        uint16_t CrcValue =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)&Buffer, BUFFER_SIZE);
                        uint16_t* pMsgCRC = (uint16_t*)&Buffer[BUFFER_SIZE];
                        *pMsgCRC = CrcValue;
                     }

                     wait_ms( 200 ); 
                     if ( LORA_CRC_ENABLED )
                     {
                        Radio.Send( Buffer, BufferSize );
                     }
                     else
                     {
                        Radio.Send( Buffer, BUFFER_SIZE_WITH_CRC );
                     }
                  }
               } 
               else if ( SEND_TEST_PACKETS_REPEAT ) 
               {
                  RssiValue = 0.0;
                  SnrValue = 0.0;
                  RxDone_Count = 0;
                  RxTimeout_Count = 0;
                  RxError_Count = 0;
                  TxDone_Count = 0;
                  TxTimeout_Count = 0;
                  CADDone_Count = 0;
                  CADDetected_Count = 0;
                  FHSS_Count = 0;
                  RestartPacketTest = true;
               }
               else
               {
                   debug( "*TX * Complete: transmitted %u\r\n", TxDone_Count);
                   current_time_secs = time(NULL);
                   debug( "Test took %u seconds\r\n", current_time_secs);
               }
            }
            else
            {
               if ( PACKET_MODE )
               {
                  memset(Buffer, 0, BUFFER_SIZE_WITH_CRC);
                  if ( SEND_INCREMENTING_PAYLOAD )
                  {
                     message_count++;
                     sprintf((char*)Buffer,"%d", message_count);
                  }
                  if ( !LORA_CRC_ENABLED )
                  {
                     uint16_t CrcValue =  (uint16_t)HAL_CRC_Calculate(&crc_handle_8b_data_16b_crc, (uint32_t *)&Buffer, BUFFER_SIZE);
                     uint16_t* pMsgCRC = (uint16_t*)&Buffer[BUFFER_SIZE];
                     *pMsgCRC = CrcValue;
                     gCRC = CrcValue;
                     Radio.Send( Buffer, BUFFER_SIZE_WITH_CRC );
                  }
                  else
                  {
                     Radio.Send( Buffer, BufferSize );
                  }
               }
#ifdef TARGET_RBU                    
                 if ( ant_tx_nrx == 0 )
                 {
                    ant_tx_nrx = 1;
                    ant_rx_ntx = 0;
                 }
#endif

            }

            State = LOWPOWER;
            break;
        case TX_TIMEOUT:
            debug( "*TXT* TX:%u, TXT:%u, FHSS:%u\r\n", TxDone_Count, TxTimeout_Count, FHSS_Count);
            State = TX;
            break;
        case LOWPOWER:
        {
            if ( PACKET_MODE == 0)           
            {
               int32_t currentState = Tamper;
              if ( currentState != PreviousSwitchState )
              {
                  debug("Continuous bit toggle state = %d\r\n", currentState);
                  PreviousSwitchState = currentState;
                  Radio.EnableContinouousDatabitToggle(currentState);
              }
           }
            else
            {
               if ( RestartPacketTest )
               {
                  RestartPacketTest = false;
                  State = TX;
               }
            }
        }
            break;
        default:
            State = TX;
            break;
      }    
   }
}

void OnTxDone( void )
{
   if ( PACKET_MODE )
   {
      Radio.Sleep( );
   }
   TxDone_Count++;
   State = TX;
//    debug_if( DEBUG_MESSAGE, "> OnTxDone\n\r" );
   if ( LORA_CRC_ENABLED )
   {
      debug_if( DEBUG_MESSAGE, "%d\n\r", TxDone_Count );
   }
   else
   {
      debug_if( DEBUG_MESSAGE, "%d crc = %d\n\r", TxDone_Count, gCRC );
   }
}

void OnTxTimeout( void )
{
   if ( PACKET_MODE )
   {
      Radio.Sleep( );
   }
   TxTimeout_Count++;
//    State = TX_TIMEOUT;
    debug_if( DEBUG_MESSAGE, "> OnTxTimeout\n\r" );
    State = TX;
//   debug_if( DEBUG_MESSAGE, "Timeout %d\n\r", TxTimeout_Count );
}

void OnFhssChangeChannel( uint8_t channelIndex )
{
   if ( PACKET_MODE )
   {
      Radio.Sleep( );
   }
   FHSS_Count++;
//    debug_if( DEBUG_MESSAGE, "> OnFhssChangeChannel\n\r" );
}
#endif //CONTINUOUS_TRANSMIT
