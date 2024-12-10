#include <stdint.h>
#include "mbed_config.h"
#include "PinNames.h"
#include "PinNamesTypes.h"
#include "mbed.h"
#include "main.h"
#include "sx1272-hal.h"
#include "mbed_debug.h"
#include "common.h"

// To compile for mbed board, remove -DTARGET_RBU from C/C++ Misc Controls.

/* Checksum value added in the SW image during the build process */
static uint32_t blf_application_checksum __attribute__( ( section( "ApplicationChecksum"),used) );
static uint32_t blf_bootloader_area __attribute__( ( section( "bootloader"),used) );
char sw_reset_debug_message[126] __attribute__( ( section( "NoInit"),zero_init) ) ;
uint32_t sw_reset_msg_indication_flag __attribute__( ( section( "NoInit"),zero_init) ) ;
uint32_t rbu_pp_mode_request __attribute__( ( section( "NoInit"),zero_init) ) ;
uint32_t* app_code = (uint32_t*)APP_ADDRESS;
uint32_t app_sp = app_code[0];
uint32_t app_start = app_code[1];

#ifdef BOOTLOADER_BUILD
const char sw_version_nbr[12] __attribute__(( at(0x08004188))) = VERSION_NUMBER_STRING;
const char sw_version_date[12] __attribute__(( at(0x08004194))) = VERSION_DATE_STRING;
#else
const char sw_version_nbr[12] __attribute__(( at(0x08000188))) = VERSION_NUMBER_STRING;
const char sw_version_date[12] __attribute__(( at(0x08000194))) = VERSION_DATE_STRING;
#endif

/*!
 * Definitions
 */
#define REPEAT_TEST_INDEFINITLEY 0
#define DEFAULT_LSI_FREQUENCY (37000u)
#define IWDG_TIMEOUT_PERIOD_MS (3000u)


/*!
 * Private function declarations
 */
static void UPD_RadioInit(void);
static void OnUsartReceive(void);
static bool DM_IndependentWdgInit(void);
static void start_app(uint32_t pc, uint32_t sp);

/*!
 * Local variables declarations
 */
//static CRC_HandleTypeDef crc_handle_8b_data_16b_crc;// CRC handler declaration
static RadioEvents_t RadioEvents;// Radio events structure
Serial serialPort(CN_PPU_TX, CN_PPU_RX);
static char serial_buffer[256];

/*
 *  Global variables declarations
 */
extern uint16_t PLLLock_Count;
DigitalOut led_r(CN_STATUS_LED1);
DigitalOut led_g(CN_STATUS_LED2);
DigitalOut led_b(CN_STATUS_LED3);
DigitalOut ant_tx_nrx(ANT_TX_NRX);
DigitalOut ant_rx_ntx(ANT_RX_NTX);
SX1272MB2xAS Radio( NULL );

IWDG_HandleTypeDef   IwdgHandle;

/*************************************************************************************/
/**
* main
* main function
*
* @param - void
*
* @return - void
*/
int main()
{
   /* De-initialise the Peripherals that are not need 
    * for the application or used differently 
    */
   __disable_irq();
   HAL_RNG_DeInit(&RngHandle);
   serial_deinit();
   __enable_irq();

   DM_IndependentWdgInit();
   
   serialPort.attach(&OnUsartReceive);
   
   UPD_RadioInit();
   
   led_b = 1;
   
   //Report PPU Updater version
   serialPort.printf("\r\nPPU UPD Version: %s %s\r\n\n", VERSION_NUMBER_STRING, VERSION_DATE_STRING);
   //Report reason for soft restart
   if ( 0 < strlen(sw_reset_debug_message) )
   {
      serialPort.printf("\r\n%s\r\n",sw_reset_debug_message);
      sw_reset_debug_message[0] = 0;//discard message
   }
   
   wait_ms(10);

    while( 1 )
   {
//      //TBD  main loop
//      start_app(app_start, app_sp);
      debug("Tick\r\n");
      
      //kick the dog
      HAL_IWDG_Refresh(&IwdgHandle);
      
      wait_ms(1000);
   }

}

/*************************************************************************************/
/**
* OnTxDone
* Radio callback function on Tx Done
*
* @param - void
*
* @return - void
*/
void OnTxDone( void )
{
}

/*************************************************************************************/
/**
* OnTxTimeout
* Radio callback function for Tx timeout
*
* @param - void
*
* @return - void
*/
void OnTxTimeout( void )
{
}

/*************************************************************************************/
/**
* OnRxDone
* Radio callback function for Rx'd message
*
* @param - void
*
* @return - void
*/
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror)
{
}

/*************************************************************************************/
/**
* OnRxTimeout
* Radio callback function for Rx timeout
*
* @param - void
*
* @return - void
*/
void OnRxTimeout( void )
{
}

/*************************************************************************************/
/**
* OnRxError
* Radio callback function for Rx error
*
* @param - void
*
* @return - void
*/
void OnRxError( void )
{
}

/*************************************************************************************/
/**
* OnFhssChangeChannel
* Radio callback function
*
* @param - void
*
* @return - void
*/
void OnCadDone( bool channelActivityDetected )
{
}

/*************************************************************************************/
/**
* OnFhssChangeChannel
* Radio callback function
*
* @param - void
*
* @return - void
*/
void OnFhssChangeChannel( uint8_t channelIndex )
{
}

/*************************************************************************************/
/**
* OnUsartReceive
* ISR for USART receive
*
* @param - void
*
* @return - void
*/
void OnUsartReceive(void)
{
   static uint32_t buffer_index = 0;
   
   char chr = serialPort.getc();
   serial_buffer[buffer_index] = chr;
   buffer_index++;
   serialPort.printf("%c", chr);
   
   if ( chr == '\r' )
   {
      serial_buffer[buffer_index] = 0;
      serialPort.printf("\n");
      buffer_index = 0;
      //TBD  set a flag to process serial_buffer outside this ISR
   }
   
}



/*************************************************************************************/
/**
* DM_IndependentWdgInit
* Initialisation function of the Independent Watchdog
*
* @param - void
*
* @return - ErrorCode_t - status 0=success else error
*/
bool DM_IndependentWdgInit(void)
{
   bool status = true;

   /* Freeze the IWDG while debugging */
#ifdef IWDG_DEBUG_FREEZE
   #if defined (STM32L0)
   __HAL_RCC_DBGMCU_CLK_ENABLE();
   #endif
   __HAL_DBGMCU_FREEZE_IWDG() ;
#endif
   
   /* Set counter reload value to obtain 1 sec. IWDG TimeOut.
   IWDG counter clock Frequency = uwLsiFreq
   Set Prescaler to 256 (IWDG_PRESCALER_256)
   Timeout Period = (Reload Counter Value * 256) / uwLsiFreq
   So Set Reload Counter Value = (Timeout Period  * uwLsiFreq) / 256 */
   IwdgHandle.Instance = IWDG;
   IwdgHandle.Init.Prescaler = IWDG_PRESCALER_256;
   IwdgHandle.Init.Reload = ((DEFAULT_LSI_FREQUENCY * IWDG_TIMEOUT_PERIOD_MS )/ 256000u);
   IwdgHandle.Init.Window = IWDG_WINDOW_DISABLE;

   if(HAL_IWDG_Init(&IwdgHandle) != HAL_OK)
   {
       /* Initialization Error */
       status = false;
   }

   return status;
}

/*************************************************************************************/
/**
* UPD_RadioInit
* Initialisation function for the Lora radio
*
* @param - void
*
* @return - void
*/
void UPD_RadioInit(void)
{
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
     serialPort.printf( "Radio could not be detected!\n\r" );
     wait( 1 );
   }

   // Set the antenna direction to RX
   ant_tx_nrx = 1;
   ant_rx_ntx = 0;
}

/*************************************************************************************/
/**
* UPD_SerialReceive
* Action commands received over the debug usart
*
* @param - void
*
* @return - void
*/
void UPD_SerialReceive(void)
{
   if ( strcmp(serial_buffer, "ATR+") )
   {
      Error_Handler((char*)"USART Restart cmd");
   }
}

/*************************************************************************************/
/**
* function name   : start_app
* description     : Transfer control from the bootloader to the RBU application
*
* @param - pc        The address to load into the program counter
* @param - sp        The address to load into the stack pointer
*
* @return - void
*/
__asm void start_app(uint32_t pc, uint32_t sp)
{
   msr msp, r1;
   bx r0;
}

