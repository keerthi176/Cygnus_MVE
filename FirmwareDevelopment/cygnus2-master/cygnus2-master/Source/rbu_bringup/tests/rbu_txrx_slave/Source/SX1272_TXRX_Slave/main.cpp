#include "mbed.h"
#include "main.h"
#include "sx1272-hal.h"
#include "mbed_debug.h"
#include "main_global.h"
#include "rtc_api.h"
#include "stm32l4xx_ll_rcc.h"

#define USE_CAD 0    // 0 : do not use CAD, 1 : use CAD

/* Set this flag to '1' to display debug messages on the console */
#define DEBUG_MESSAGE   1

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

#define RX_TIMEOUT_VALUE                                10e6   // in us
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
    GET_CMD,
    
    RX,
    RX_TIMEOUT,
    RX_ERROR,
    
    TX,
    TX_TIMEOUT,
    
    CAD,
    CAD_DONE
}AppStates_t;

volatile AppStates_t State = GET_CMD;

/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;

/*
 *  Global variables declarations
 */
SX1272MB2xAS Radio( NULL );

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

bool channelActivityDetectedFlag = false;

int main() 
{
    char Command;
    uint8_t led_count = 0;
    uint32_t xtal_drive = 0;
    GPIO_InitTypeDef GPIO_InitStruct;
    
    #if defined ( TARGET_RBU )
	// Antenna select
    ant_sel1 = 1;
    ant_sel2 = 0;
    cn_head_pwr = 1;
    #endif

    debug( "\n\n\r     SX1272 TX Only - 1 : Main \n\n\r" );

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
                         LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON, RxBufferSize,
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
    
    while( 1 )
    {
        switch( State )
        {
        case GET_CMD:
            debug("Enter command: 'T <payload (string)>' (transmit), 'R' (receive), 'S' (switch test), 'L' (LED advance), 'C' (LSE clock)...\r\n");
            fscanf(stdin, "%c", &Command);
            if (Command == 'T') {
                fscanf(stdin, "%s", TxBuffer);
                
                if (!LORA_FIX_LENGTH_PAYLOAD_ON) {
                    TxBufferSize = strlen((char*)TxBuffer);
                }
                
                if( TxBufferSize > 0 )
                {
                    debug("Transmitting %u bytes, payload = %s...\r\n", TxBufferSize, TxBuffer);
                    Radio.Send( TxBuffer, TxBufferSize );
                    State = LOWPOWER;
                } else {
                    State = GET_CMD;
                }
            } else if (Command == 'R') {
                debug("Receiving...\r\n");

                if (USE_CAD == 1) {
                   Radio.StartCad();
                } else {
                   Radio.Rx( RX_TIMEOUT_VALUE );
                }

                State = LOWPOWER;
            } else if (Command == 'S') {
               DigitalIn fa_sw(CN_FA_MCP_IN);
               DigitalIn fire_sw(CN_FIRE_MCP_IN);
               DigitalIn head_tamp(CN_TAMPER1);
               DigitalIn base_tamp(CN_TAMPER2);

               debug( "Press FIRE, FIRST AID, HEAD TAMPER, or BASE TAMPER switch...\n\r" );
               while (1) {        
                  if (fa_sw.read() == 0) {
                     debug( "FIRST AID pressed.\n\r" );
                     State = GET_CMD;
                     wait_ms( 200 ); 
                     break;
                  } else if (fire_sw.read() == 0) {
                     debug( "FIRE pressed.\n\r" );
                     State = GET_CMD;
                     wait_ms( 200 ); 
                     break;
                  } else if (head_tamp.read() == 0) {
                     debug( "HEAD TAMPER pressed.\n\r" );
                     State = GET_CMD;
                     wait_ms( 200 ); 
                     break;
                  } else if (base_tamp.read() == 0) {
                     debug( "BASE TAMPER pressed.\n\r" );
                     State = GET_CMD;
                     wait_ms( 200 ); 
                     break;
                  }
               }
            } else if (Command == 'L') {
                led_count += 1;
                led_r = ((led_count&0x1)>>0 == 0x1) ? 1 : 0;
                led_g = ((led_count&0x2)>>1 == 0x1) ? 1 : 0;
                led_b = ((led_count&0x4)>>2 == 0x1) ? 1 : 0;
                State = GET_CMD;
            } else if (Command == 'C') {

               // Test LSE 32.768kHz RTC clock
                debug("Initialising LSE 32.768kHz RTC clock - NOTE: 5 second delay...\r\n");
                
                if(__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY)) {  // 32khz osc is running
                    debug("LSERDY = TRUE (LSE 32.768kHz RTC clock working).\r\n");
                } // else clause not required since rtc_init reports error
                xtal_drive = LL_RCC_LSE_GetDriveCapability();
                if (xtal_drive == LL_RCC_LSEDRIVE_MEDIUMLOW) {
                    debug("LSEDRV setting = LL_RCC_LSEDRIVE_MEDIUMLOW.\r\n\r\n");
                } else if (xtal_drive == LL_RCC_LSEDRIVE_MEDIUMHIGH) {
                    debug("LSEDRV setting = LL_RCC_LSEDRIVE_MEDIUMHIGH.\r\n\r\n");
                } else if (xtal_drive == LL_RCC_LSEDRIVE_HIGH) {
                    debug("LSEDRV setting = LL_RCC_LSEDRIVE_HIGH.\r\n\r\n");
                } else {
                    debug("LSEDRV setting = LL_RCC_LSEDRIVE_LOW.\r\n\r\n");
                }
                // Select LSE source for MCO output on PA8
                debug("Outputting LSE 32.768kHz RTC clock on PA8.\r\n");
                HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_LSE, RCC_MCODIV_1);
                GPIO_InitStruct.Pin = GPIO_PIN_8;
                GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
                GPIO_InitStruct.Pull = GPIO_NOPULL;
                GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
                GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
                HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
                State = GET_CMD;
            } else {
                debug("Unknown command %c\r\n", Command);
                State = GET_CMD;
            }
            fseek(stdin,0,SEEK_SET);
            break;
        case TX:
            debug( "*TX * TX:%u, TXT:%u, FHSS:%u\r\n", TxDone_Count, TxTimeout_Count, FHSS_Count);
            led_count += 1;
            led_r = ((led_count&0x1)>>0 == 0x1) ? 1 : 0;
            led_g = ((led_count&0x2)>>1 == 0x1) ? 1 : 0;
            led_b = ((led_count&0x4)>>2 == 0x1) ? 1 : 0;
            State = GET_CMD;
            break;
        case TX_TIMEOUT:
            debug( "*TXT* TX:%u, TXT:%u, FHSS:%u\r\n", TxDone_Count, TxTimeout_Count, FHSS_Count);
            if( TxBufferSize > 0 )
            {
                debug("Transmitting %u bytes, payload = %s...\r\n", TxBufferSize, TxBuffer);
                Radio.Send( TxBuffer, TxBufferSize );
                State = LOWPOWER;
            } else {
                State = GET_CMD;
            }
            break;
        case RX:    
            debug( "*RX * RX:%u, RXT:%u, RXE:%u, CAD:%u, CADD:%u, LOCK:%u, RSSI:%i, SNR:%i\r\n", RxDone_Count, RxTimeout_Count, RxError_Count, CADDone_Count, CADDetected_Count, PLLLock_Count, RssiValue, SnrValue);
            debug("Received %u bytes, payload = %s...\r\n", RxBufferSize, RxBuffer);
            led_count += 1;
            led_r = ((led_count&0x1)>>0 == 0x1) ? 1 : 0;
            led_g = ((led_count&0x2)>>1 == 0x1) ? 1 : 0;
            led_b = ((led_count&0x4)>>2 == 0x1) ? 1 : 0;
            State = GET_CMD;
            break;
        case RX_ERROR:
            debug( "*RXE* RX:%u, RXT:%u, RXE:%u, CAD:%u, CADD:%u, LOCK:%u, RSSI:%i, SNR:%i\r\n", RxDone_Count, RxTimeout_Count, RxError_Count, CADDone_Count, CADDetected_Count, PLLLock_Count, RssiValue, SnrValue);
            debug("Received %u bytes, payload = %s...\r\n", RxBufferSize, RxBuffer);
            State = GET_CMD;
            break;
        case RX_TIMEOUT:
            debug( "*RXT* RX:%u, RXT:%u, RXE:%u, CAD:%u, CADD:%u, LOCK:%u, RSSI:%i, SNR:%i\r\n", RxDone_Count, RxTimeout_Count, RxError_Count, CADDone_Count, CADDetected_Count, PLLLock_Count, RssiValue, SnrValue);
            debug("Receiving...\r\n");

            if (USE_CAD == 1) {
               Radio.StartCad();
            } else {
               Radio.Rx( RX_TIMEOUT_VALUE );
            }

            State = LOWPOWER;
            break;
        case CAD_DONE:
            if (channelActivityDetectedFlag == true) {
//                debug( "*CAD* RX:%u, RXT:%u, RXE:%u, CAD:%u, CADD:%u, LOCK:%u, RSSI:%i, SNR:%i\r\n", RxDone_Count, RxTimeout_Count, RxError_Count, CADDone_Count, CADDetected_Count, PLLLock_Count, RssiValue, SnrValue);
                Radio.Rx( RX_TIMEOUT_VALUE );
            } else {
                Radio.StartCad();
            }
            State = LOWPOWER;
            break;
        case LOWPOWER:
            break;
        default:
            State = GET_CMD;
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
    RxDone_Count++;
    RxBufferSize = size;
    memcpy( RxBuffer, payload, TxBufferSize );
    RssiValue = rssi;
    SnrValue = snr;
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
