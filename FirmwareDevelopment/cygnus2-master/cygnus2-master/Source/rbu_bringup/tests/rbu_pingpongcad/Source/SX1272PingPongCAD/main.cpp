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

#define RX_TIMEOUT_VALUE                                3500000   // in us
#define BUFFER_SIZE                                     32        // Define the payload size here

#if( defined ( TARGET_KL25Z ) || defined ( TARGET_LPC11U6X ) )
//DigitalOut led(LED2);
#else
//DigitalOut led(LED1);
#endif

DigitalOut led_r(CN_STATUS_LED1);
DigitalOut led_g(CN_STATUS_LED2);
DigitalOut led_b(CN_STATUS_LED3);

//DigitalIn fa_sw(CN_FA_MCP_IN);
//DigitalIn fire_sw(CN_FIRE_MCP_IN);

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
    
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    
    TX,
    TX_TIMEOUT,
    
    CAD,
    CAD_DONE
}AppStates_t;

volatile AppStates_t State = LOWPOWER;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*
 *  Global variables declarations
 */
SX1272MB2xAS Radio( NULL );

const uint8_t PingMsg[] = "PING";
const uint8_t PongMsg[] = "PONG";

uint16_t BufferSize = BUFFER_SIZE;
uint8_t Buffer[BUFFER_SIZE];

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

bool channelActivityDetectedFlag = false;

int main() 
{
    uint8_t i;
    uint8_t led_count = 0;
    bool isMaster = true;
    
    #if defined ( TARGET_RBU )
	 // Antenna select
    ant_sel1 = 1;
    ant_sel2 = 0;
    cn_head_pwr = 1;
    #endif

    debug( "\n\n\r     SX1272 Ping Pong Demo Application \n\n\r" );

    // Initialize Radio driver
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.CadDone = OnCadDone;
    RadioEvents.RxError = OnRxError;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.FhssChangeChannel = OnFhssChangeChannel;
    Radio.Init( &RadioEvents );
    
    // verify the connection with the board
    while( Radio.Read( REG_VERSION ) == 0x00  )
    {
        debug( "Radio could not be detected!\n\r", NULL );
        wait( 1 );
    }
            
    debug_if( ( DEBUG_MESSAGE & ( Radio.DetectBoardType( ) == SX1272MB2XAS ) ) , "\n\r > Board Type: SX1272MB2xAS < \n\r" );
    
    Radio.SetChannel( RF_FREQUENCY ); 

#if USE_MODEM_LORA == 1
    
    debug_if( LORA_FHSS_ENABLED, "\n\n\r             > LORA FHSS Mode < \n\n\r");
    debug_if( !LORA_FHSS_ENABLED, "\n\n\r             > LORA Mode < \n\n\r");

    Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                         LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                         LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                         LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                         LORA_IQ_INVERSION_ON, 2000000 );
    
    Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                         LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                         LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, BufferSize,
                         LORA_CRC_ENABLED, LORA_FHSS_ENABLED, LORA_NB_SYMB_HOP, 
                         LORA_IQ_INVERSION_ON, true );
                         
#elif USE_MODEM_FSK == 1

    debug("\n\n\r              > FSK Mode < \n\n\r");
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

/*     
   debug( "Press FIRE for master, FIRST AID for slave...\n\r" );
   while (1) {        
      if (fa_sw.read() == 0) {
         debug( "FIRST AID pressed - setting to slave.\n\r" );
         isMaster = false;
         State = RX_TIMEOUT;
         break;
      } else if (fire_sw.read() == 0) {
         debug( "FIRE pressed - setting to master.\n\r" );
         isMaster = true;
         State = RX_TIMEOUT;
         break;
      }
   }
*/

    debug_if( DEBUG_MESSAGE, "Starting Ping-Pong loop\r\n" ); 
        
//    led = 0;
        
    if (USE_CAD == 1) {
       Radio.StartCad();
    } else {
       Radio.Rx( RX_TIMEOUT_VALUE );
    }
    
    while( 1 )
    {
        switch( State )
        {
        case RX:
            debug( "*RX * RX:%u, RXT:%u, RXE:%u, CAD:%u, CADD:%u, LOCK:%u, RSSI:%i, SNR:%i\r\n", RxDone_Count, RxTimeout_Count, RxError_Count, CADDone_Count, CADDetected_Count, PLLLock_Count, RssiValue, SnrValue);
            if( isMaster == true )
            {
                if( BufferSize > 0 )
                {
                    if( strncmp( ( const char* )Buffer, ( const char* )PongMsg, 4 ) == 0 )
                    {
                        // led = !led;
                        led_count += 1;
                        led_r = ((led_count&0x1)>>0 == 0x1) ? 1 : 0;
                        led_g = ((led_count&0x2)>>1 == 0x1) ? 1 : 0;
                        led_b = ((led_count&0x4)>>2 == 0x1) ? 1 : 0;
/*                        debug( "...Pong\r\n" ); */
                        // Send the next PING frame            
                        strcpy( ( char* )Buffer, ( char* )PingMsg );
                        // We fill the buffer with numbers for the payload 
                        for( i = 4; i < BufferSize; i++ )
                        {
                            Buffer[i] = i - 4;
                        }
                        wait_ms( 10 ); 
                        Radio.Send( Buffer, BufferSize );
                    }
                    else if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 )
                    { // A master already exists then become a slave
/*                        debug( "...Ping\r\n" ); */
                        // led = !led;
                        led_count += 1;
                        led_r = ((led_count&0x1)>>0 == 0x1) ? 1 : 0;
                        led_g = ((led_count&0x2)>>1 == 0x1) ? 1 : 0;
                        led_b = ((led_count&0x4)>>2 == 0x1) ? 1 : 0;
                        isMaster = false;
                        // Send the next PONG frame            
                        strcpy( ( char* )Buffer, ( char* )PongMsg );
                        // We fill the buffer with numbers for the payload 
                        for( i = 4; i < BufferSize; i++ )
                        {
                            Buffer[i] = i - 4;
                        }
                        wait_ms( 10 ); 
                        Radio.Send( Buffer, BufferSize );
                    }
                    else // valid reception but neither a PING or a PONG message
                    {    // Set device as master ans start again
                        isMaster = true;

                        if (USE_CAD == 1) {
                           Radio.StartCad();
                        } else {
                           Radio.Rx( RX_TIMEOUT_VALUE );
                        }
                    }    
                }
            }
            else
            {
                if( BufferSize > 0 )
                {
                    if( strncmp( ( const char* )Buffer, ( const char* )PingMsg, 4 ) == 0 )
                    {
                        // led = !led;
                        led_count += 1;
                        led_r = ((led_count&0x1)>>0 == 0x1) ? 1 : 0;
                        led_g = ((led_count&0x2)>>1 == 0x1) ? 1 : 0;
                        led_b = ((led_count&0x4)>>2 == 0x1) ? 1 : 0;
/*                        debug( "...Ping\r\n" ); */
                        // Send the reply to the PING string
                        strcpy( ( char* )Buffer, ( char* )PongMsg );
                        // We fill the buffer with numbers for the payload 
                        for( i = 4; i < BufferSize; i++ )
                        {
                            Buffer[i] = i - 4;
                        }
                        wait_ms( 10 );  
                        Radio.Send( Buffer, BufferSize );
                    }
                    else // valid reception but not a PING as expected
                    {    // Set device as master and start again
                        isMaster = true;

                        if (USE_CAD == 1) {
                           Radio.StartCad();
                        } else {
                           Radio.Rx( RX_TIMEOUT_VALUE );
                        }
                    }    
                }
            }
            State = LOWPOWER;
            break;
        case TX:    
            debug( "*TX * RX:%u, RXT:%u, RXE:%u, CAD:%u, CADD:%u, LOCK:%u, RSSI:%i, SNR:%i\r\n", RxDone_Count, RxTimeout_Count, RxError_Count, CADDone_Count, CADDetected_Count, PLLLock_Count, RssiValue, SnrValue);
            // led = !led; 
            led_count += 1;
            led_r = ((led_count&0x1)>>0 == 0x1) ? 1 : 0;
            led_g = ((led_count&0x2)>>1 == 0x1) ? 1 : 0;
            led_b = ((led_count&0x4)>>2 == 0x1) ? 1 : 0;
            if( isMaster == true )  
            {
/*                debug( "Ping...\r\n" ); */
            }
            else
            {
/*                debug( "Pong...\r\n" ); */
            }

            if (USE_CAD == 1) {
               Radio.StartCad();
            } else {
               Radio.Rx( RX_TIMEOUT_VALUE );
            }

            State = LOWPOWER;
            break;
        case RX_TIMEOUT:
            debug( "*RXT* RX:%u, RXT:%u, RXE:%u, CAD:%u, CADD:%u, LOCK:%u, RSSI:%i, SNR:%i\r\n", RxDone_Count, RxTimeout_Count, RxError_Count, CADDone_Count, CADDetected_Count, PLLLock_Count, RssiValue, SnrValue);
            if( isMaster == true )
            {
                // Send the next PING frame
                strcpy( ( char* )Buffer, ( char* )PingMsg );
                for( i = 4; i < BufferSize; i++ )
                {
                    Buffer[i] = i - 4;
                }
                wait_ms( 10 ); 
                Radio.Send( Buffer, BufferSize );
            }
            else
            {
                if (USE_CAD == 1) {
                   Radio.StartCad();
                } else {
                   Radio.Rx( RX_TIMEOUT_VALUE );
                }
            }             
            State = LOWPOWER;
            break;
        case RX_ERROR:
            debug( "*RXE* RX:%u, RXT:%u, RXE:%u, CAD:%u, CADD:%u, LOCK:%u, RSSI:%i, SNR:%i\r\n", RxDone_Count, RxTimeout_Count, RxError_Count, CADDone_Count, CADDetected_Count, PLLLock_Count, RssiValue, SnrValue);
            // We have received a Packet with a CRC error, send reply as if packet was correct
            if( isMaster == true )
            {
                // Send the next PING frame
                strcpy( ( char* )Buffer, ( char* )PingMsg );
                for( i = 4; i < BufferSize; i++ )
                {
                    Buffer[i] = i - 4;
                }
                wait_ms( 10 );  
                Radio.Send( Buffer, BufferSize );
            }
            else
            {
                // Send the next PONG frame
                strcpy( ( char* )Buffer, ( char* )PongMsg );
                for( i = 4; i < BufferSize; i++ )
                {
                    Buffer[i] = i - 4;
                }
                wait_ms( 10 );  
                Radio.Send( Buffer, BufferSize );
            }
            State = LOWPOWER;
            break;
        case TX_TIMEOUT:
            debug( "*TXT* RX:%u, RXT:%u, RXE:%u, CAD:%u, CADD:%u, LOCK:%u, RSSI:%i, SNR:%i\r\n", RxDone_Count, RxTimeout_Count, RxError_Count, CADDone_Count, CADDetected_Count, PLLLock_Count, RssiValue, SnrValue);

            if (USE_CAD == 1) {
               Radio.StartCad();
            } else {
               Radio.Rx( RX_TIMEOUT_VALUE );
            }

            State = LOWPOWER;
            break;
        case LOWPOWER:
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
        default:
            State = LOWPOWER;
            break;
        }    
    }
}

void OnTxDone( void )
{
    Radio.Sleep( );
    TxDone_Count++;
    State = TX;
    debug_if( DEBUG_MESSAGE, "> OnTxDone\n\r" );
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    Radio.Sleep( );
    RxDone_Count++;
    BufferSize = size;
    memcpy( Buffer, payload, BufferSize );
    RssiValue = rssi;
    SnrValue = snr;
    State = RX;
    debug_if( DEBUG_MESSAGE, "> OnRxDone\n\r" );
}

void OnTxTimeout( void )
{
    Radio.Sleep( );
    TxTimeout_Count++;
    State = TX_TIMEOUT;
    debug_if( DEBUG_MESSAGE, "> OnTxTimeout\n\r" );
}

void OnRxTimeout( void )
{
    Radio.Sleep( );
    RxTimeout_Count++;
    Buffer[ BufferSize ] = 0;
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
    // ?? Force channelActivityDetectedFlag high for ping pong test
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
