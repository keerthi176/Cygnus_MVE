/*************************************************************************************
*  CONFIDENTIAL between PA and Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2017 PA Consulting Group. All rights reserved.
*  Cambridge Technology Centre
*  Melbourn Royston
*  Herts SG8 6DP UK
*  Tel: <44> 1763 261222
*
**************************************************************************************
*  File         : MM_MACTask.c
*
*  Description  : MAC Task functions
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>



/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "MC_MAC.h"
#include "CO_Message.h"
#include "MC_TestMode.h"
#include "DM_SerialPort.h"
#include "DM_LED.h"
#include "board.h"
#include "radio.h"
#include "MM_ConfigSerialTask.h"
#include "MC_MacConfiguration.h"
#include "MM_MACTask.h"

#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
#include "lptim.h"
#endif

/* Private Functions Prototypes
*************************************************************************************/
static void MM_MACTaskMain (void const *argument);
static void OnTxDone( void );
static void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, int32_t frequency_deviation );
static void OnTxTimeout( void );
static void OnRxTimeout( void );
static void OnRxError( void );



/* Global Variables
*************************************************************************************/
osThreadId tid_MACTask;
osThreadDef (MM_MACTaskMain, osPriorityHigh, 1, 1300);

osSemaphoreDef (MACSem);
osSemaphoreId  (MACSemId);

extern osPoolId MeshPool;
extern osMessageQId(MeshQ);
extern uint16_t gIRQlptim1Value;
extern LoraParameters_t MC_MAC_LoraParameters;
extern uint16_t gRadioSettlingTime;
extern uint16_t wake_up_time;

int16_t gRxDoneRssi = 0; 
int8_t  gRxDoneSnr = 0;
uint8_t gRxDoneFreqChannelIdx = 0;
int32_t gRxFrequencyDeviation = 0;


#ifdef USE_PIN_11
extern Gpio_t Pin11;
#endif
#ifdef USE_PIN_12
extern Gpio_t Pin12;
#endif

bool gMacTimerFlag = false;
                                                 
/* Private Variables
*************************************************************************************/
static bool gTxDoneFlag = false;
static bool gRxDoneFlag = false;
static bool gCadDoneFlag = false;
static bool gTxTimeoutFlag = false;
static bool gRxTimeoutFlag = false;
static bool gRxErrorFlag = false;
static bool gBuiltInTestReqFlag = false;
static bool gTxAtCommand = false;

static uint8_t *gRxDonePayload = NULL;
static uint16_t gRxDoneSize = 0;
static uint16_t gRxDoneMcuRtcValue = 0;
static bool gChannelActivityDetected = false;

static RadioEvents_t RadioEvents;

/* False CAD Counting Variables
*************************************************************************************/
#ifdef COUNT_FALSE_CADS
static bool gTestForFalseCAD = false;  // This is set to true when a CAD suggests channel activity, so that false CADs can be reported.
uint16_t gCAD_TimeoutCount = 0;        // Incremented each time a CAD results in an RxSingle timeout: this often indicates a false CAD.
#endif

/*****************************************************************************
*   Function:         void MC_MAC_TxAtCommand( void )
*   Returns:          nothing
*   Description:      Instructs the MAC to transmit the message in the RACHS queue
*   Notes:            none
*****************************************************************************/
void MC_MAC_TxAtCommand( void )
{
   /* trigger operation in task */
   gTxAtCommand = true;
   osSemaphoreRelease(MACSemId);
}

   
/*****************************************************************************
*   Function:         void OnTxDone( void )
*   Returns:         nothing
*   Description:   TX done event
*   Notes:            none
*****************************************************************************/
static void OnTxDone( void )
{
   /* trigger operation in task */
   gTxDoneFlag = true;
   osSemaphoreRelease(MACSemId);
}


/*****************************************************************************
*   Function:         void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
*   Returns:         nothing
* arguments:      uint8_t *payload - pointer to a data buffer
*                        uint16_t size - number of bytes in the buffer
*                        int16_t rssi - rssi value of rx data
*                        int8_t snr - snr value of rx data
*                        int32_t frequency_deviation - frequency difference between Rx and received signal
*   Description:   RX done event
*   Notes:            none
*****************************************************************************/
static void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, int32_t frequency_deviation )
{
   // Copy the timer value that was read very soon after the interrupt was detected.   
   // Note, this is just copying from one global to another. It may be possible to lose one global variable. 
   gRxDoneMcuRtcValue = gIRQlptim1Value;
   
   /* store parameters */
   gRxDonePayload = payload;
   gRxDoneSize = size;
   gRxDoneRssi = rssi;
   gRxDoneSnr = snr;
   gRxDoneFreqChannelIdx = MC_MAC_LoraParameters.CurrentFrequencyChannelIdx;
   gRxFrequencyDeviation = frequency_deviation;

   /* trigger operation in task */
   gRxDoneFlag = true;
   osSemaphoreRelease(MACSemId);
   
   #ifdef COUNT_FALSE_CADS
   // Rx Success, so reset the flag that tests for false CADs.
   gTestForFalseCAD = false;
   #endif
}


/*************************************************************************************/
/**
* function name : void OnCadDone( bool channelActivityDetected )
* description   : Call back function for the Cad Done interrupt.
*
* INPUTS
* @param - bool channelActivityDetected: true if activity detected, false if timeout.
*
* @return - void
*/
static void OnCadDone( bool channelActivityDetected )
{
   gChannelActivityDetected = channelActivityDetected;
   
   /* trigger operation in task */
   gCadDoneFlag = true;
   osSemaphoreRelease(MACSemId);
   
   #ifdef COUNT_FALSE_CADS
   // Set flag so that we can monitor for false CADs.
   gTestForFalseCAD = channelActivityDetected;
   #endif
}


/*****************************************************************************
*   Function:         void OnTxTimeout( void )
*   Returns:         nothing
*   Description:   TX timeout event
*   Notes:            none
*****************************************************************************/
static void OnTxTimeout( void )
{
   /* trigger operation in task */
   gTxTimeoutFlag = true;
   osSemaphoreRelease(MACSemId);
}


/*****************************************************************************
*   Function:         void OnRxTimeout( void )
*   Returns:         nothing
*   Description:   RX timeout event
*   Notes:            none
*****************************************************************************/
static void OnRxTimeout( void )
{
   /* trigger operation in task */
   gRxTimeoutFlag = true;
   osSemaphoreRelease(MACSemId);
   
   #ifdef COUNT_FALSE_CADS
   // If Rx was in respose to CAD, increment counter and reset flag.
   if (true == gTestForFalseCAD)
   {
      gTestForFalseCAD = false;
      gCAD_TimeoutCount++;
   }
   #endif
}

/*****************************************************************************
*   Function:         void OnRxError( void )
*   Returns:         nothing
*   Description:   RX error event
*   Notes:            none
*****************************************************************************/
static void OnRxError( void )
{
   /* trigger operation in task */
   gRxErrorFlag = true;
   osSemaphoreRelease(MACSemId);
   
   #ifdef COUNT_FALSE_CADS
   // Rx Error (CAD detected a packet, but it didn't pass the CRC check), so reset the flag that tests for false CADs.
   gTestForFalseCAD = false;
   #endif
}


/*************************************************************************************/
/**
* MM_MACTaskInit
* Initialisation function for the MAC Task
*
* @param - const uint32_t isSyncMaster - flag true for TDM master and false for TDM slave
* @param - const uint32_t address - address of node in mesh
* @param - const uint32_t systemId - System ID
*
* @return - ErrorCode_t  // Returns an error code, else SUCCESS_E

*/
ErrorCode_t MM_MACTaskInit(const uint32_t isSyncMaster, const uint32_t address, const uint32_t systemId)
{
   ErrorCode_t ErrorCode;
   uint32_t rfFrequency;
   uint32_t frequency_channel = MC_GetBaseFrequencyChannel();
   
   /* create semaphore */
   MACSemId = osSemaphoreCreate(osSemaphore(MACSem), 1);
   CO_ASSERT_RET_MSG(NULL != MACSemId, ERR_INIT_FAIL_E, "ERROR - Failed to create MAC semaphore");
   
   // Init SPI
   SpiInit( &SX1272.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC_E );      // RM: init SPI
        
   // Init IOs
   SX1272IoInit( );
     
   // Radio initialization
   RadioEvents.TxDone = OnTxDone;
   RadioEvents.RxDone = OnRxDone;
   RadioEvents.CadDone = OnCadDone;
   RadioEvents.TxTimeout = OnTxTimeout;
   RadioEvents.RxTimeout = OnRxTimeout;
   RadioEvents.RxError = OnRxError;

   Radio.Init( &RadioEvents );

   uint8_t silicon_rev = Radio.Read(REG_VERSION);

   if(SEMTECH_SILICON_DEFAULT_ID != silicon_rev)
   {
      //Send BIT fault to application.
      CO_SendFaultSignal(BIT_SOURCE_RADIO_E, CO_CHANNEL_RADIO_E, FAULT_SIGNAL_INTERNAL_FAULT_E, 1, false, false);
      CO_PRINT_A_0(DBG_BIT_E, "SPI Radio Fail\r\n");
   }
   else
   {           
      // select the default channel index
      rfFrequency = MC_GetFrequencyForChannel(frequency_channel);

      Radio.SetChannel( rfFrequency );
      
      // Set register RegPll (addr. 0x5C) to 0x10 to set PLL loop filter bandwidth to 75 kHz (JIRA CYG2-615).
      // This improves phase noise of device and therefore performance in the presence of an in-band blocker.
      // Impact: locking time increased by factor of 4.
      // Note that this is not suitable for the transmitter, which works best with B/W of 300 kHz (JIRA CYG2-827).
      // Hence the PLL bandwidth is set during the pre-processing phase of each receive or transmit action.
      SX1272Write( REG_LR_PLL, REG_PLL_BW_75KHZ);

	
      Radio.SetTxConfig( MODEM_LORA, TX_LOW_OUTPUT_POWER, 0, LORA_BANDWIDTH_868,
                                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                                      LORA_DCH_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                                      LORA_INITIALISE_CRC, 0, 0, LORA_IQ_INVERSION_ON, LORA_TX_TIMEOUT );
												  
												   
      Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH_868, LORA_SPREADING_FACTOR,
                                      LORA_CODINGRATE, 0, LORA_DCH_PREAMBLE_LENGTH,
                                      LORA_DCH_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                      PHY_DCH_PACKET_SIZE, LORA_INITIALISE_CRC, 0, 0, LORA_IQ_INVERSION_ON, LORA_RX_CONTINUOUS_MODE );


      // Initialise the MAC (return on error)
      ErrorCode = MC_MAC_Init(isSyncMaster, systemId, address);		

      // Create thread
      tid_MACTask = osThreadCreate(osThread(MM_MACTaskMain), NULL);
      CO_ASSERT_RET_MSG(NULL != tid_MACTask, ERR_FAILED_TO_CREATE_THREAD_E, "ERROR - Failed to create MAC Task thread");
   }
   
   /* initialise the test module */
   TM_Initialise(systemId, frequency_channel);

   return ErrorCode;
}


/*************************************************************************************/
/**
* MM_MACTaskMain
* Main function for MAC Task
*
* @param - arguments
*
* @return - void

*/
static void MM_MACTaskMain(void const *argument)
{
   MC_MAC_TestMode_t test_mode = MC_MAC_TEST_MODE_OFF_E;
   
   while (true)
   {
      //GpioWrite(&Pin12, 0);
      osSemaphoreWait(MACSemId, osWaitForever);
      
      test_mode = MC_GetTestMode();

      //GpioWrite(&Pin11, 0);

      if (true == gMacTimerFlag)
      {
//         CO_PRINT_B_0(DBG_INFO_E,"Tm\r\n");

#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
         wake_up_time = HAL_LPTIM_ReadCounter(&hlptim1);
#endif
         /* reset flag */
         gMacTimerFlag = false;
         
         if ( (MC_MAC_TEST_MODE_OFF_E == test_mode)  )
         {
            /* MAC timer triggered */
            MC_MAC_TimerOperation();
         }
         else if ((MC_MAC_TEST_MODE_TRANSMIT_E == test_mode) ||
                  (MC_MAC_TEST_MODE_TRANSPARENT_E == test_mode) ||
                  (MC_MAC_TEST_MODE_MONITORING_E == test_mode) )
         {
            DoTestModeTRANSMIT();
         }
         else if ( MC_MAC_TEST_MODE_SLEEP_E == test_mode )
         {
            /* wake from sleep mode periodically to kick the watchdog */
            MC_MAC_DoTestModeSleep();
         }
         else if ( MC_MAC_TEST_MODE_RECEIVE_E == test_mode )
         {
            /* wake from sleep mode periodically to kick the watchdog */
            SetTestModeWakeup(TEST_MODE_WATCHDOG_PERIOD);
         }
         else
         {
            /* default action */
            MC_MAC_TimerOperation();
         }
         //GpioWrite(&Pin11, 0);
      }

      if (true == gTxDoneFlag)
      {
         
#ifdef TXRX_TIMING_PULSE
   GpioWrite(&StatusLedRed,0);
#endif
         
         /* reset flag */
         gTxDoneFlag = false;
         
         /* MAC Tx Done triggered */
         MC_MAC_TxDoneOperation();
         CO_PRINT_B_0(DBG_INFO_E,"Td\r\n");
     }
      
      if (true == gRxDoneFlag)
      {
#ifdef TXRX_TIMING_PULSE
   GpioWrite(&StatusLedBlue,0);
#endif
//         CO_PRINT_B_0(DBG_INFO_E,"Rd\r\n");
         /* reset flag */
         gRxDoneFlag = false;
         
         if (MC_MAC_TEST_MODE_TRANSMIT_E != test_mode)
         {
            MC_MAC_RxDoneOperation(gRxDonePayload, gRxDoneSize, gRxDoneRssi, gRxDoneSnr, gRxDoneMcuRtcValue, gRxDoneFreqChannelIdx, gRxFrequencyDeviation);
            if ( (MC_MAC_TEST_MODE_OFF_E != test_mode) && (MC_MAC_TEST_MODE_SLEEP_E != test_mode) && (MC_MAC_TEST_MODE_NETWORK_MONITOR_E != test_mode) )
            {
               /* Restart the receiver in test mode */
               Radio.Standby();

               Radio.Rx();
            }
         }
         
         //GpioWrite(&Pin11, 0);
         //GpioToggle(&Pin12);     
      }

      if (true == gCadDoneFlag)
      {
//         CO_PRINT_B_0(DBG_INFO_E,"Cd\r\n");
         /* reset flag */
         gCadDoneFlag = false;
         
#ifdef TXRX_TIMING_PULSE
   GpioWrite(&StatusLedBlue,0);
#endif
         
         
         if ((MC_MAC_TEST_MODE_OFF_E == test_mode) || 
            (MC_MAC_TEST_MODE_RECEIVE_E == test_mode) ||
            (MC_MAC_TEST_MODE_NETWORK_MONITOR_E == test_mode) )
         {

            if (true == gChannelActivityDetected)
            {
               MC_MAC_PacketReceiveSingle();
            }
            else
            {
               if ( MC_MAC_TEST_MODE_RECEIVE_E == test_mode )
               {
                  /* cad timed out in test mode.  start again */
                  SX1272StartCad();
               }
               else
               {
                  // put radio to sleep
#ifdef ALLOW_RADIO_SLEEP
                  Radio.Sleep();
#else
                  Radio.Standby();
#endif
               }
//               CO_PRINT_A_1(DBG_DATA_E,"%d\r\n", GUI_EVENT_CAD_TIMEOUT_E);
            }
         }
      }

      if (true == gTxTimeoutFlag)
      {
//         CO_PRINT_B_0(DBG_INFO_E,"Txto\r\n");
#ifdef TXRX_TIMING_PULSE
   GpioWrite(&StatusLedRed,0);
#endif
         /* reset flag */
         gTxTimeoutFlag = false;
         
         /* MAC Tx Timeout triggered */
         MC_MAC_TxTimeoutOperation();
         //CO_PRINT_B_0(DBG_INFO_E,"To\r\n");
      }

      if (true == gRxTimeoutFlag)
      {
//         CO_PRINT_B_0(DBG_INFO_E,"Rxto\r\n");
#ifdef TXRX_TIMING_PULSE
   GpioWrite(&StatusLedBlue,0);
#endif
         /* reset flag */
         gRxTimeoutFlag = false;
         //CO_PRINT_B_0(DBG_INFO_E,"Ro\r\n");
         
         if ((MC_MAC_TEST_MODE_OFF_E == test_mode) || 
            (MC_MAC_TEST_MODE_RECEIVE_E == test_mode) ||
            (MC_MAC_TEST_MODE_NETWORK_MONITOR_E == test_mode) )
         {

            /* MAC timer triggered */
            MC_MAC_RxTimeoutErrorOperation(CO_RXTE_TIMEOUT_E);
//            CO_PRINT_A_1(DBG_DATA_E,"%d\r\n", GUI_EVENT_RX_TIMEOUT_E);
         }
      }

      if (true == gRxErrorFlag)
      {
//         CO_PRINT_B_0(DBG_INFO_E,"Rxe\r\n");
#ifdef TXRX_TIMING_PULSE
   GpioWrite(&StatusLedBlue,0);
#endif
         /* reset flag */
         gRxErrorFlag = false;
         
         /* MAC error triggered */
         MC_MAC_RxTimeoutErrorOperation(CO_RXTE_ERROR_E);
         //CO_PRINT_A_1(DBG_DATA_E,"%d\r\n", GUI_EVENT_RX_ERROR_E);
      }

      if (true == gBuiltInTestReqFlag)
      {
//         CO_PRINT_B_0(DBG_INFO_E,"BIT\r\n");
         /* reset flag */
         gBuiltInTestReqFlag = false;

         uint8_t silicon_rev = Radio.Read(REG_VERSION);
         
         CO_PRINT_B_1(DBG_INFO_E, "Semtech ID 0x%02X\r\n", silicon_rev);

         /* TODO: Store the revision value in the EEPROM */
         if(SEMTECH_SILICON_DEFAULT_ID != silicon_rev)
         {
            //Send BIT fault to application.
            CO_SendFaultSignal(BIT_SOURCE_RADIO_E, CO_CHANNEL_RADIO_E, FAULT_SIGNAL_INTERNAL_FAULT_E, 1, false, false);
            CO_PRINT_A_0(DBG_BIT_E, "SPI Radio Fail\r\n");
         }
         else
         {
            CO_PRINT_B_0(DBG_BIT_E, "SPI Radio Success\r\n");
         }
      }
      
      if ( true == gTxAtCommand )
      {
         gTxAtCommand = false;
         MC_MAC_SendAtCommand();
      }
   }
}


/*************************************************************************************/
/**
* MM_MAC_TimerISR
*
* This function is called in the context of the interrupt from the LPTIM timer.
*
* it triggers the MAC task which performs the main processing
*
* @param - void
*
* @return - void

*/
void MM_MAC_TimerISR(void)
{
	//GpioWrite(&Pin11, 1);
   /* trigger operation in task */
   gMacTimerFlag = true;
   osSemaphoreRelease(MACSemId);
}

/*************************************************************************************/
/**
* MM_MAC_BuiltInTestReq
*
* Requests the built-in test of the Radio module
*
* @param - void
*
* @return - Error code

*/
ErrorCode_t MM_MAC_BuiltInTestReq(void)
{
   ErrorCode_t result = SUCCESS_E;

   /* Set the Flag to request a Built In Test of the Radio */
   gBuiltInTestReqFlag = true;

   if ( osOK != osSemaphoreRelease(MACSemId))
   {
      /* We couldn't relesae the semaphore 
       * Reset the Request Flag and return Failure 
       */
      gBuiltInTestReqFlag = false;
      result = ERR_BUILT_IN_TEST_FAIL_E;
   }

   return result;
}
