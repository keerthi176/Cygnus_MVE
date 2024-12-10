#include "mbed.h"
#include "main.h"
#include "sx1272-hal.h"
#include "mbed_debug.h"
#include "main_global.h"

#define USE_CAD 0    // 0 : do not use CAD, 1 : use CAD

/* Set this flag to '1' to display debug messages on the console */
#define DEBUG_MESSAGE   0

/* Set this flag to '1' to use the LoRa modulation or to '0' to use FSK modulation */
#define USE_MODEM_LORA  1
#define USE_MODEM_FSK   !USE_MODEM_LORA

#define RF_FREQUENCY                                    865450000 // Hz // Hz
#define TX_OUTPUT_POWER                                 7         // 7 dBm

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
    #define LORA_PREAMBLE_LENGTH                        8         // Same for Tx and Rx
    #define LORA_SYMBOL_TIMEOUT                         5         // Symbols
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
    #define FSK_PREAMBLE_LENGTH                         5         // Same for Tx and Rx
    #define FSK_FIX_LENGTH_PAYLOAD_ON                   false
    #define FSK_CRC_ENABLED                             true
    
#else
    #error "Please define a modem in the compiler options."
#endif

#define RX_TIMEOUT_VALUE                                0         // in us
#define BUFFER_SIZE                                     32        // Define the payload size here

#if( defined ( TARGET_KL25Z ) || defined ( TARGET_LPC11U6X ) )
//DigitalOut led(LED2);
#else
//DigitalOut led(LED1);
#endif

DigitalOut led_r(CN_STATUS_LED1);
DigitalOut led_g(CN_STATUS_LED2);
DigitalOut led_b(CN_STATUS_LED3);

#if defined ( TARGET_RBU )
DigitalOut ant_sel1(ANT1_SEL);
DigitalOut ant_sel2(ANT2_SEL);
DigitalOut cn_head_pwr(CN_HEAD_PWR_ON);
#endif

/*
 *  Global variables declarations
 */
typedef enum
{
    LOWPOWER = 0,
    IDLE,
    SYNC_RX,
    
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    
    TX,
    TX_TIMEOUT,
    
    CAD,
    CAD_DONE
}AppStates_t;

volatile AppStates_t State = SYNC_RX;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*
 *  Global variables declarations
 */
SX1272MB2xAS Radio( NULL );

const uint8_t TestMsg[] = "TWO2";

uint8_t TxBufferSize = BUFFER_SIZE;
uint8_t RxBufferSize = BUFFER_SIZE;
uint8_t TxBuffer[256];
uint8_t RxBuffer[256];

int16_t RssiValue = 0.0;
int8_t SnrValue = 0.0;

uint16_t RxDone_Count = 0;
uint16_t RxTimeout_Count = 0;
uint16_t RxError_Count = 0;
uint16_t TxDone_Count = 0;
uint16_t TxTimeout_Count = 0;
uint16_t CADDone_Count = 0;
uint16_t CADDetected_Count = 0;
uint16_t FHSS_Count = 0;

uint16_t Transmit_Number = 100;

bool channelActivityDetectedFlag = false;

int main() 
{
    uint8_t i;

    led_r = 0;
    led_g = 0;
    led_b = 1;

    #if defined ( TARGET_RBU )
	// Antenna select
    ant_sel1 = 1;
    ant_sel2 = 0;
    cn_head_pwr = 1;
    #endif

    debug( "\n\n\r     SX1272 TX Only - 1 : Main (with initial SYNC from RX) \n\n\r" );
    debug( "Waiting for SYNC from RX...\r\n" );

    // Initialize Radio driver
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.CadDone = OnCadDone;
    RadioEvents.RxError = OnRxError;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.FhssChangeChannel = OnFhssChangeChannel;
    Radio.Init( &RadioEvents );
    
    // verify the connection with the board
    while( Radio.Read( REG_VERSION ) == 0x00  )
    {
        debug_if( DEBUG_MESSAGE, "Radio could not be detected!\n\r", NULL );
        wait( 1 );
    }
            
    debug_if( ( DEBUG_MESSAGE & ( Radio.DetectBoardType( ) == SX1272MB2XAS ) ) , "\n\r > Board Type: SX1272MB2xAS < \n\r" );
    
    Radio.SetChannel( RF_FREQUENCY ); 

#if USE_MODEM_LORA == 1
    
    debug_if( DEBUG_MESSAGE&LORA_FHSS_ENABLED, "\n\n\r             > LORA FHSS Mode < \n\n\r");
    debug_if( DEBUG_MESSAGE&(!LORA_FHSS_ENABLED), "\n\n\r             > LORA Mode < \n\n\r");

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                         LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                         LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                         LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                         LORA_IQ_INVERSION_ON, 2000000 );
    
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                         LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                         LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, RxBufferSize,
                         LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                         LORA_IQ_INVERSION_ON, true );
                         
#elif USE_MODEM_FSK == 1

    debug_if( DEBUG_MESSAGE, "\n\n\r              > FSK Mode < \n\n\r");
    Radio.SetTxConfig( MODEM_FSK, TX_OUTPUT_POWER, FSK_FDEV, 0,
                         FSK_DATARATE, 0,
                         FSK_PREAMBLE_LENGTH, FSK_FIX_LENGTH_PAYLOAD_ON,
                         FSK_CRC_ENABLED, 0, 0, 0, 2000000 );

    Radio.SetRxConfig( MODEM_FSK, FSK_BANDWIDTH, FSK_DATARATE,
                         0, FSK_AFC_BANDWIDTH, FSK_PREAMBLE_LENGTH,
                         0, FSK_FIX_LENGTH_PAYLOAD_ON, 0, FSK_CRC_ENABLED,
                         0, 0, false, true );                         
#else

#error "Please define a modem in the compiler options."

#endif

    if( TxBufferSize > 0 )
    {
        // Send the next frame            
        strcpy( ( char* )TxBuffer, ( char* )TestMsg );
        // We fill the buffer with numbers for the payload 
        for( i = 4; i < TxBufferSize; i++ )
        {
            TxBuffer[i] = i - 4;
        }
    }
    
    while( 1 )
    {
        switch( State )
        {
        case SYNC_RX:
            wait_ms( 500 ); 
            Radio.Rx( RX_TIMEOUT_VALUE );
            State = LOWPOWER;
            break;
        case RX:
//            debug( "Synchronised, starting transmit...\r\n" );
            State = TX;
            break;
        case RX_ERROR:
            debug( "*RXE* RX:%u, RXT:%u, RXE:%u, CAD:%u, CADD:%u, LOCK:%u, RSSI:%i, SNR:%i\r\n", RxDone_Count, RxTimeout_Count, RxError_Count, CADDone_Count, CADDetected_Count, PLLLock_Count, RssiValue, SnrValue);
            debug( "SYNC Error - Reset all boards...\r\n" );
/*
            if (USE_CAD == 1) {
               Radio.StartCad();
            } else {
               Radio.Rx( RX_TIMEOUT_VALUE );
            }
*/
            State = SYNC_RX;
            break;
        case RX_TIMEOUT:
            debug( "*RXT* RX:%u, RXT:%u, RXE:%u, CAD:%u, CADD:%u, LOCK:%u, RSSI:%i, SNR:%i\r\n", RxDone_Count, RxTimeout_Count, RxError_Count, CADDone_Count, CADDetected_Count, PLLLock_Count, RssiValue, SnrValue);
            debug( "SYNC Timeout - Reset all boards...\r\n" );
/*
            if (USE_CAD == 1) {
               Radio.StartCad();
            } else {
               Radio.Rx( RX_TIMEOUT_VALUE );
            }
*/
            State = LOWPOWER;
            break;
        case CAD_DONE:
            if (channelActivityDetectedFlag == true) {
/*                debug( "*CAD* RX:%u, RXT:%u, RXE:%u, CAD:%u, CADD:%u, LOCK:%u, RSSI:%i, SNR:%i\r\n", RxDone_Count, RxTimeout_Count, RxError_Count, CADDone_Count, CADDetected_Count, PLLLock_Count, RssiValue, SnrValue); */
                Radio.Rx( RX_TIMEOUT_VALUE );
            } else {
                Radio.StartCad();
            }
            State = LOWPOWER;
            break;
        case TX:
//            debug( "*TX * TX:%u, TXT:%u, FHSS:%u\r\n", TxDone_Count, TxTimeout_Count, FHSS_Count);
            led_r = 1;
            led_g = 0;
            led_b = 0;
            if( TxDone_Count < Transmit_Number) {
                Radio.Send( TxBuffer, TxBufferSize );
            } else if ( TxDone_Count == Transmit_Number) {
                debug( "*TX * Complete: transmitted %u\r\n", TxDone_Count);
            }
            State = LOWPOWER;
            break;
        case TX_TIMEOUT:
            debug( "*TXT* TX:%u, TXT:%u, FHSS:%u\r\n", TxDone_Count, TxTimeout_Count, FHSS_Count);
            State = TX;
            break;
        case LOWPOWER:
            break;
        default:
            State = TX;
            break;
        }    
    }
}

void OnTxDone( void )
{
    Radio.Sleep( );
    TxDone_Count++;
    State = SYNC_RX;
    debug_if( DEBUG_MESSAGE, "> OnTxDone\n\r" );
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    TxTimeout_Count++;
    State = TX_TIMEOUT;
    debug_if( DEBUG_MESSAGE, "> OnTxTimeout\n\r" );
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    Radio.Sleep( );
/*    RxDone_Count++;
    RxBufferSize = size;
    memcpy( RxBuffer, payload, TxBufferSize );
    RssiValue = rssi;
    SnrValue = snr;*/
    State = RX;
    debug_if( DEBUG_MESSAGE, "> OnRxDone\n\r" );
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    RxTimeout_Count++;
    RxBuffer[ RxBufferSize ] = 0;
    State = RX_TIMEOUT;
    debug_if( DEBUG_MESSAGE, "> OnRxTimeout\n\r" );
}

void OnRxError( void )
{
    Radio.Sleep( );
    RxError_Count++;
    State = RX_ERROR;
    debug_if( DEBUG_MESSAGE, "> OnRxError\n\r" );
}

void OnCadDone( bool channelActivityDetected )
{
    Radio.Sleep( );
    CADDone_Count++;
    channelActivityDetectedFlag = channelActivityDetected;
    if (channelActivityDetected) {
        CADDetected_Count++;
    }
    State = CAD_DONE;
    debug_if( DEBUG_MESSAGE, "> OnCadDone\n\r" );
}

void OnFhssChangeChannel( uint8_t channelIndex )
{
    Radio.Sleep( );
    FHSS_Count++;
    debug_if( DEBUG_MESSAGE, "> OnFhssChangeChannel\n\r" );
}
