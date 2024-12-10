//#include "mbed.h"
#include <stdint.h>
#include "main.h"
#include "sx1272-hal.h"
#include "mbed_debug.h"
#include "main_global.h"
#include "stm32l4xx_hal_crc.h"
#include "RxPacketLossTest.h"
#include "RxOriginalPacketTest.h"
#include "RxBerTest.h"
#include "RxContinuous.h"
//#include "SurveyToolLedTest.h"
#include "RxSurveyTool.h"
//#include "RxPingTest.h"

// To compile for mbed board, remove -DTARGET_RBU from C/C++ Misc Controls.

/*!
 * Definitions
 */
#define REPEAT_TEST_INDEFINITLEY 0
#define DEFAULT_LSI_FREQUENCY (37000u)
#define IWDG_TIMEOUT_PERIOD_MS (3000u)

/*!
 * Private function declarations
 */
//static void ReportRegisters(void);
static void OnUsartReceive(void);
static void PrintHelp(void);
static bool ChangeTest(char* pCommand);
static bool DM_IndependentWdgInit(void);

/*!
 * Local variables declarations
 */
static CRC_HandleTypeDef crc_handle_8b_data_16b_crc;// CRC handler declaration
static RadioEvents_t RadioEvents;// Radio events structure
static RxTestBase* pTestModule = NULL; // Pointer to the test module to be executed
//Serial Port
#ifdef CONTROL_PANEL  //defined in project settings C++ tab
static Serial serialPort(CN_DEBUG_TX, CN_DEBUG_RX);
#else
static Serial serialPort(CN_PPU_TX, CN_PPU_RX);
#endif
static bool PauseTest = false;
static char serial_buffer[256];
static bool test_running;
static PacketLossTest PacketLossTestTestModule;
static RxOriginalPacketTest RxOriginalPacketTestTestModule;
static RxBerTest RxBerTestTestModule;
static RxContinuousTest RxContinuousTestModule;
//static SurveyToolLedTest SurveyToolLedTestModule;
static SurveyToolRx SurveyToolRxModule;
//static RxPingTest RxPingTestModule;
/*
 *  Global variables declarations
 */
DigitalOut led_r(CN_STATUS_LED1);
DigitalOut led_g(CN_STATUS_LED2);
DigitalOut led_b(CN_STATUS_LED3);
DigitalOut ant_tx_nrx(ANT_TX_NRX);
DigitalOut ant_rx_ntx(ANT_RX_NTX);
SX1272MB2xAS Radio( NULL );

IWDG_HandleTypeDef   IwdgHandle;

int main()
{
   DM_IndependentWdgInit();
   
   // Initialize Radio driver
   RadioEvents.TxDone = OnTxDone;
   RadioEvents.TxTimeout = OnTxTimeout;
   RadioEvents.RxDone = OnRxDone;
   RadioEvents.CadDone = OnCadDone;
   RadioEvents.RxError = OnRxError;
   RadioEvents.RxTimeout = OnRxTimeout;
   RadioEvents.FhssChangeChannel = OnFhssChangeChannel;
   Radio.Init( &RadioEvents );
   
   serialPort.attach(&OnUsartReceive);

   // verify the connection with the board
   while( Radio.Read( REG_VERSION ) == 0x00  )
   {
     debug( "Radio could not be detected!\n\r", NULL );
     wait( 1 );
   }
   
   // Set the antenna direction
   //Original direction switch
   //ant_tx_nrx = 0;
   //ant_rx_ntx = 1;
   //New direction switch with reversed logic
   ant_tx_nrx = 1;
   ant_rx_ntx = 0;


   // verify the connection with the board
   while( Radio.Read( REG_VERSION ) == 0x00  )
   {
     debug( "Radio could not be detected!\n\r", NULL );
     wait( 1 );
   }
   
   led_b = 1;

   pTestModule = &PacketLossTestTestModule;
   
   test_running = true;
   
   while( 1 )
   {
      if ( pTestModule->Initialise() )
      {
         while ( test_running )
         {
            if ( !PauseTest )
            {
               test_running = pTestModule->RunProcess();
               

               if ( test_running == false )
               {
                  if ( REPEAT_TEST_INDEFINITLEY )
                  {
                     test_running = pTestModule->Initialise();
                  }
               }
            }
            //kick the dog
            HAL_IWDG_Refresh(&IwdgHandle);
         }
      }
      else
      {
         debug("\r\n\nTest module failed to initialise\r\n");
      }

      debug("\r\n\nTest Complete\r\n");
      
      while( test_running == false )
      {
         //kick the dog
         HAL_IWDG_Refresh(&IwdgHandle);
         
         wait_us(100);
      }
   }

}

void OnTxDone( void )
{
   if ( pTestModule )
   {
      pTestModule->OnTxDone();
   }
}

void OnTxTimeout( void )
{
   if ( pTestModule )
   {
      pTestModule->OnTxTimeout();
   }
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror)
{
   if ( pTestModule )
   {
      pTestModule->OnRxDone(payload, size, rssi, snr, FeiVal, ferror);
   }
}

void OnRxTimeout( void )
{
   if ( pTestModule )
   {
      pTestModule->OnRxTimeout();
   }
}

void OnRxError( void )
{
   if ( pTestModule )
   {
      pTestModule->OnRxError();
   }
}

void OnCadDone( bool channelActivityDetected )
{
   if ( pTestModule )
   {
      pTestModule->OnCadDone(channelActivityDetected);
   }
}

void OnFhssChangeChannel( uint8_t channelIndex )
{
   if ( pTestModule )
   {
      pTestModule->OnFhssChangeChannel(channelIndex);
   }
}

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
      if ( serial_buffer[0] == '?' )
      {
         PrintHelp();
      }
      else if ( false == ChangeTest(serial_buffer) )
      {
         if ( pTestModule )
         {
            if ( pTestModule->ProcessCommand(serial_buffer) )
            {
               test_running = true;
               PauseTest = false;
               serialPort.printf("OK\r\n");
            }
            else
            {
               serialPort.printf("ERROR\r\n");
            }
         }
      }
      else
      {
         serialPort.printf("OK\r\n");
      }
   }
}

void PrintHelp(void)
{
   uint8_t reg_value = Radio.Read(REG_LR_LNA) & RF_LNA_BOOST_ON;
   uint32_t lna_boost = 1;
   if (reg_value == 0 )
   {
      lna_boost = 0;
   }
   serialPort.printf("Options:\r\n\n");
   serialPort.printf("Change test by entering the folowing:\r\n\n");
   serialPort.printf("surveytool    (Rx Survey tool)\r\n");
   serialPort.printf("surveytest    (Rotates through LEDs on Survey tool)\r\n");
   serialPort.printf("packetloss    (counts lost/error packets and displays packet loss %%)\r\n");
   serialPort.printf("packets       (Legacy packet test used with 'main' and 'interferer' Tx modes)\r\n");
   serialPort.printf("ber           (Bit Error Rate in FSK mode)\r\n");
   serialPort.printf("carrier       (Reports the 0 or 1 value of the modulated output in FSK mode)\r\n");
   serialPort.printf("lnaboost  (%d) (Set the LNA boost 0=off 1=on)\r\n", lna_boost);
   serialPort.printf("pause         (Pause the test)\r\n");
   serialPort.printf("run           (Continue the test)\r\n");
   serialPort.printf("\r\n");
   if ( pTestModule )
   {
      pTestModule->PrintHelp(serialPort);
   }
}

bool ChangeTest(char* pCommand)
{
   bool result = false;
   
   if ( pCommand )
   {
      if ( 0 == strncmp(pCommand, "packetloss", 10 ) )
      {
         PauseTest = true;
         Radio.Sleep();
         pTestModule = &PacketLossTestTestModule;
         pTestModule->Initialise();
         PauseTest = false;
         test_running = true;
         result = true;
      }
      else if ( 0 == strncmp(pCommand, "packets", 7 ) )
      {
         PauseTest = true;
         Radio.Sleep();
         pTestModule = &RxOriginalPacketTestTestModule;
         pTestModule->Initialise();
         PauseTest = false;
         test_running = true;
         result = true;
      }
      else if ( 0 == strncmp(pCommand, "ber", 3 ) )
      {
         PauseTest = true;
         Radio.Sleep();
         pTestModule = &RxBerTestTestModule;
         pTestModule->Initialise();
         PauseTest = false;
         test_running = true;
         result = true;
      }
      else if ( 0 == strncmp(pCommand, "carrier", 7 ) )
      {
         PauseTest = true;
         Radio.Sleep();
         pTestModule = &RxContinuousTestModule;
         pTestModule->Initialise();
         PauseTest = false;
         test_running = true;
         result = true;
      }
//      else if ( 0 == strncmp(pCommand, "surveytest", 10 ) )
//      {
//         PauseTest = true;
//         Radio.Sleep();
//         pTestModule = &SurveyToolLedTestModule;
//         pTestModule->Initialise();
//         PauseTest = false;
//         test_running = true;
//         result = true;
//      }
      else if ( 0 == strncmp(pCommand, "surveytool", 10 ) )
      {
         PauseTest = true;
         Radio.Sleep();
         pTestModule = &SurveyToolRxModule;
         pTestModule->Initialise();
         PauseTest = false;
         test_running = true;
         result = true;
      }
      else if ( 0 == strncmp(pCommand, "lnaboost", 8 ) )
      {
         PauseTest = true;
         Radio.Sleep();
         char* pValue = (char*)pCommand + 8;
         if( *pValue == '=' )
         {
            pValue++;
            int32_t value = atoi(pValue);
            uint8_t reg_value = Radio.Read(REG_LR_LNA) & RF_LNA_BOOST_MASK;
            if ( value > 0 )
            {
               reg_value |= RF_LNA_BOOST_ON;
               serialPort.printf("LNA boost ON\r\n");
            }
            else
            {
               serialPort.printf("LNA boost OFF\r\n");
            }
            Radio.Write(REG_LR_LNA, reg_value);
            
            result = true;
         }
      }
      else if ( 0 == strncmp(pCommand, "pause", 5 ) )
      {
         PauseTest = true;
         result = true;
      }
      else if ( 0 == strncmp(pCommand, "run", 3 ) )
      {
         PauseTest = false;
         test_running = true;
         result = true;
      }
   }
   return result;
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

//void ReportRegisters(void)
//{
//   debug( "\r\n");
//   debug( "REG_OPMODE=0x%0.2x\r\n", Radio.Read(REG_OPMODE));
//   debug( "REG_DIOMAPPING1=0x%0.2x\r\n", Radio.Read(REG_DIOMAPPING1));
//   debug( "REG_DIOMAPPING2=0x%0.2x\r\n", Radio.Read(REG_DIOMAPPING2));
//   debug( "REG_SEQCONFIG1=0x%0.2x\r\n", Radio.Read(REG_SEQCONFIG1));
//   debug( "REG_SEQCONFIG2=0x%0.2x\r\n", Radio.Read(REG_SEQCONFIG2));
//   debug( "REG_OOKPEAK=0x%0.2x\r\n", Radio.Read(REG_OOKPEAK));
//   debug( "REG_PACKETCONFIG1=0x%0.2x\r\n", Radio.Read(REG_PACKETCONFIG1));
//   debug( "REG_PACKETCONFIG2=0x%0.2x\r\n", Radio.Read(REG_PACKETCONFIG2));
//}

