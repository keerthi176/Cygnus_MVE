#include <stdio.h>
#include "mbed.h"
#include "main.h"
#include "sx1272-hal.h"
#include "mbed_debug.h"
#include "main_global.h"
#include "RadioRegisterDump.h"
#include "PacketIncrementingPayload.h"
#include "PacketMainPayload.h"
#include "PacketInterfererPayload.h"
#include "PacketSynchronised.h"
#include "PacketPPU.h"
#include "TxContinuous.h"
#include "TxBerTestSignal.h"
#include "Standby.h"
#include "TxPingTest.h"
#include "TxSurveyTool.h"

// To compile for mbed board, remove -DTARGET_RBU from C/C++ Misc Controls.


/*!
 * Definitions
 */

#define REPEAT_TEST_INDEFINITLEY                      0
#define DEFAULT_LSI_FREQUENCY (37000u)
#define IWDG_TIMEOUT_PERIOD_MS (3000u)
/*!
 * Private function declarations
 */
static void initialiseGPIO(void);
static void OnUsartReceive(void);
static void PrintHelp(void);
static bool ChangeTest(char* pCommand);
static bool ChangeGlobal(char* pCommand);
static bool DM_IndependentWdgInit(void);

/*!
 * Local variables declarations
 */
DigitalOut led_r(CN_STATUS_LED1);
DigitalOut led_g(CN_STATUS_LED2);
DigitalOut led_b(CN_STATUS_LED3);



DigitalOut ant_tx_nrx(ANT_TX_NRX);
DigitalOut ant_rx_ntx(ANT_RX_NTX);


/*!
 * Radio events function pointer
 */
static RadioEvents_t RadioEvents;
//Serial Port
#ifdef CONTROL_PANEL  //defined in project settings C++ tab
static Serial serialPort(CN_DEBUG_TX, CN_DEBUG_RX, 115200);
#else
Serial serialPort(CN_PPU_TX, CN_PPU_RX);
#endif
static char serial_buffer[256];

static bool test_running = true;
static bool PauseTest = false;
static TestBase* pTestModule = NULL;

//Test modules
static PacketPpuCommand PacketIncrementingPpuPayloadModule;
static PacketIncrementingPayload PacketIncrementingPayloadModule;
static PacketMainPayload PacketMainPayloadModule;
static PacketInterfererPayload PacketInterfererPayloadModule;
static TxContinuous TxContinuousModule;
static TxBerTestSignal TxBerTestSignalModule;
static Standby StandbyModule;
static TxPingTest TxPingTestModule;
static TxSurveyTool TxSurveyToolModule;
/*
 *  Global variables declarations
 */
SX1272MB2xAS Radio( NULL );

IWDG_HandleTypeDef   IwdgHandle;


int main()
{
   led_r = 0;
   led_g = 0;
   led_b = 0;
   
   DM_IndependentWdgInit();
   
   //initialise the gpio pins
   initialiseGPIO();
   
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
   
   // Set the antenna direction
   //Original direction switch
   //ant_tx_nrx = 1;
   //ant_rx_ntx = 0;
   //New direction switch with reversed logic
   ant_tx_nrx = 0;
   ant_rx_ntx = 1;


   // verify the connection with the board
   while( Radio.Read( REG_VERSION ) == 0x00  )
   {
     debug( "Radio could not be detected!\n\r", NULL );
     wait( 1 );
   }
   

   
   pTestModule = &PacketIncrementingPayloadModule;
   test_running = true;
   
   while( 1 )
   {
      
      if ( pTestModule->Initialise() )
      {
//         RadioRegisterDump radioDump;
//         radioDump.DumpLoraRegistersRaw(Radio, serialPort);
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

void OnFhssChangeChannel( uint8_t channelIndex )
{
   if ( pTestModule )
   {
      pTestModule->OnFhssChangeChannel(channelIndex);
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
         if ( false == ChangeGlobal(serial_buffer) )
         {
            if ( pTestModule )
            {
               if ( pTestModule->ProcessCommand(serial_buffer) )
               {
                  serialPort.printf("OK\r\n");
               }
               else
               {
                  serialPort.printf("ERROR\r\n");
               }
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
   serialPort.printf("Options:\r\n\n");
   serialPort.printf("Change test by entering the folowing:\r\n\n");
   serialPort.printf("TxSurvey   (Transmit module for survey tool)\r\n");
   serialPort.printf("packetloss (Packet with incrementing payload)\r\n");
   serialPort.printf("packets1   (Packet with payload='ONExxx' (Main))\r\n");
   serialPort.printf("packets2   (Packet with payload='TWOxxx' (Interferer))\r\n");
   serialPort.printf("carrier    (Unmodulated carier)\r\n");
   serialPort.printf("ber        (FSK toggled 1,0)\r\n");
   serialPort.printf("standby    (Puts the LoRa chip into standby)\r\n");
   serialPort.printf("pause      (Pause the test)\r\n");
   serialPort.printf("run        (Continue the test)\r\n");
   serialPort.printf("\r\n");
   if ( pTestModule )
   {
      pTestModule->PrintHelp(serialPort);
   }

}

//////////////////////////////////////////////////////////////////////////////
// ChangeTest
//
// Change which test is running
//
///////////////////////////////////////////////////////////////////////////////
bool ChangeTest(char* pCommand)
{
   bool result = false;
   
   if ( pCommand )
   {
      if ( 0 == strncmp(pCommand, "packetloss", 10 ) )
      {
         PauseTest = true;
         Radio.Sleep();
         pTestModule = &PacketIncrementingPayloadModule;
         pTestModule->Initialise();
         PauseTest = false;
         test_running = true;
         result = true;
      }
      else if ( 0 == strncmp(pCommand, "packets1", 8 ) )
      {
         PauseTest = true;
         Radio.Sleep();
         pTestModule = &PacketMainPayloadModule;
         pTestModule->Initialise();
         PauseTest = false;
         test_running = true;
         result = true;
      }
      else if ( 0 == strncmp(pCommand, "packets2", 8 ) )
      {
         PauseTest = true;
         Radio.Sleep();
         pTestModule = &PacketInterfererPayloadModule;
         pTestModule->Initialise();
         PauseTest = false;
         test_running = true;
         result = true;
      }
      else if ( 0 == strncmp(pCommand, "carrier", 7 ) )
      {
         PauseTest = true;
         Radio.Sleep();
         pTestModule = &TxContinuousModule;
         pTestModule->Initialise();
         PauseTest = false;
         test_running = true;
         result = true;
      }
      else if ( 0 == strncmp(pCommand, "ber", 3 ) )
      {
         PauseTest = true;
         Radio.Sleep();
         pTestModule = &TxBerTestSignalModule;
         pTestModule->Initialise();
         PauseTest = false;
         test_running = true;
         result = true;
      }
      else if ( 0 == strncmp(pCommand, "TxSurvey", 8 ) )
      {
         PauseTest = true;
         Radio.Sleep();
         pTestModule = &TxSurveyToolModule;
         pTestModule->Initialise();
         PauseTest = false;
         test_running = true;
         result = true;
      }
      else if ( 0 == strncmp(pCommand, "standby", 7 ) )
      {
         PauseTest = true;
         Radio.Sleep();
         pTestModule = &StandbyModule;
         pTestModule->Initialise();
         PauseTest = false;
         test_running = true;
         result = true;
      }
   }
   return result;
}

//////////////////////////////////////////////////////////////////////////////
// ChangeTest
//
// Change parameters that affect all tests
//
///////////////////////////////////////////////////////////////////////////////
bool ChangeGlobal(char* pCommand)
{
   bool result = false;
   
   if ( 0 == strncmp(pCommand, "pause", 5 ) )
   {
      if ( pTestModule == &TxBerTestSignalModule || pTestModule == &TxContinuousModule )
      {
         Radio.Sleep();
      }
      PauseTest = true;
      test_running = true;
      result = true;
   }
   else if ( 0 == strncmp(pCommand, "run", 3 ) )
   {
      if ( pTestModule == &TxBerTestSignalModule || pTestModule == &TxContinuousModule )
      {
         pTestModule->Initialise();
      }
      PauseTest = false;
      test_running = true;
      result = true;
   }
   else if ( 0 == strncmp(pCommand, "reset", 5) )
   {
      test_running = true;
      //Leave the result as false so that the test module also gets called
   }
   
   return result;
}

//////////////////////////////////////////////////////////////////////////////
// ChangeTest
//
// Change parameters that affect all tests
//
///////////////////////////////////////////////////////////////////////////////
void initialiseGPIO(void)
{

//   GpioInit( &StatusLedGreen, CN_STATUS_LED_GREEN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//   GpioInit( &StatusLedRed, CN_STATUS_LED_RED, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//   GpioInit( &StatusLedBlue, CN_STATUS_LED_BLUE, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//   GpioInit( &PA0, HEAD_TMP_FLT, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//   GpioInit( &SviPwrOn, SVI_PWR_ON, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//   GpioInit( &I2CPullupsOn, I2C_PULLUPS_ON, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//      /* Port A */
//      GpioInit( &HeadTmpFlt, HEAD_TMP_FLT, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//      GpioInit( &BaseTmpFlt, BASE_TMP_FLT, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//      /* Port B */
//      GpioInit( &PrimaryBatIrq, PRI_BAT_IRQ, PIN_INPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//      GpioInit( &PriBatteryEn, PRI_BAT_EN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//      GpioInit( &BatMonEn, BAT_MON_EN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//      GpioInit( &TamperEnable, TAMPER_EN, PIN_OUTPUT, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
//      /* Port C */
//      GpioInit( &Pir, CONSTR_PIR_IN, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
//      GpioInit( &FirstAidMCP, CN_FA_MCP_IN, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
//      /* Port D */
//      GpioInit( &FireMCP, CN_FIRE_MCP_IN, PIN_INPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0 );
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
