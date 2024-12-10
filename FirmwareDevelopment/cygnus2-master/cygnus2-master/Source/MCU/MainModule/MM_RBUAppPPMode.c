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
*  File         : MM_RBUAppPPModeTask.c
*
*  Description  : RBU Application code in the Peer to Peer Mode
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>


/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "CO_ErrorCode.h"
#include "lptim.h"
#include "DM_CRC.h"
#include "DM_LED.h"
#include "DM_SerialPort.h"
#include "DM_SVI.h"
#include "board.h"
#include "MC_MAC.h"
#include "MC_PUP.h"
#include "SM_StateMachine.h"
#include "MM_ATHandleTask.h"
#include "MM_CommandProcessor.h"
#include "MM_Interrupts.h"
#include "MM_Main.h"
#include "BLF_Boot.h"
#include "MM_RBUApplicationTask.h"
//#include "MM_PPUApplicationTask.h"

#include	"DP_Debug.h"

/* Private Functions Prototypes
*************************************************************************************/
static void OnTxDone_PPMode( void );
static void OnRxDone_PPMode( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, int32_t frequncy_deviation );
static void OnTxTimeout_PPMode( void );
static void OnRxTimeout_PPMode( void );
static void OnRxError_PPMode( void );
static void MM_PPModeRxDoneOperation(const uint8_t * const payload, const uint16_t size, const int16_t rssi, const int8_t snr);
static ErrorCode_t MM_RBUAppPPModeOutputSig(const uint8_t OutputProfile, const uint16_t OutputsActivated, const uint8_t OutputDuration);
static ErrorCode_t MM_RBUAppPPModeCommand(const PP_CommandMessage_t *const pCommandMsg);
static void send_radio_packet(uint8_t *const p_packet);
static void send_heartbeat(bool active);
static ErrorCode_t PP_ProcessHeadCommand (const PP_CommandMessage_t *const pCommandMsg);
static ErrorCode_t PP_GenerateHeadResponse (PP_CommandMessage_t *const pResponseMsg, const HeadMessage_t *const pHeadResponse);

/* Private definitions
*************************************************************************************/
static void MM_PPModeTimerISR(void);
static uint32_t gUnitSerialNo = 0u;

/* Global Varbles
*************************************************************************************/
extern osPoolId AppPool;
extern osMessageQId(AppQ);
extern osMessageQId(HeadQ);
extern int32_t gHeadQueueCount;
#ifndef USE_NEW_HEAD_INTERFACE
extern osSemaphoreId HeadInterfaceSemaphoreId;
#endif

/* Private Variables
*************************************************************************************/
#define PP_HEARTBEAT_TX_PERIOD_MS   (uint32_t)30e3
#define RBU_PP_MODE_MAX_TIME_MS     (uint32_t)180e3
#define PP_TIMER_TICK_INTERVAL_MS   500u
#define PP_VIS_INDIC_INTERVAL_MS    10u

static RadioEvents_t RadioEvents;
static int16_t gRxDoneRssi = 0; 
static int8_t  gRxDoneSnr = 0;
static uint8_t *gRxDonePayload = NULL;
static uint16_t gRxDoneSize = 0;
static bool gRxDoneFlag = false;
static uint32_t idle_time_cntr_ms = 0;
static uint32_t tx_heartbeat_cntr_ms = 0;

extern uint32_t GetNextHandle(void);
extern HAL_StatusTypeDef FLASH_WaitForLastOperation(uint32_t Timeout);

/*****************************************************************************
*   Function:         void OnTxDone_PPMode( void )
*   Returns:         nothing
*   Description:   TX done event
*   Notes:            none
*****************************************************************************/
static void OnTxDone_PPMode( void )
{
   /* put radio into standby for config */
   Radio.Standby( );
      
   /* Set the antenna switch to receive */
   GpioWrite(&AntRxNtx,1);
   GpioWrite(&AntTxNrx,0);
   
   // Start the radio reception.
   Radio.Rx();
	
	CO_PRINT_A_0(DBG_INFO, "\r\n ###------>>> TXDONE! - PPU MODE\r\n");	// !!!! DEBUG !!!!
}

/*****************************************************************************
*   Function:         void OnRxDone_PPMode( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
*   Returns:         nothing
* arguments:      uint8_t *payload - pointer to a data buffer
*                        uint16_t size - number of bytes in the buffer
*                        int16_t rssi - rssi value of rx data
*                        int8_t snr - snr value of rx data
*   Description:   RX done event
*   Notes:            none
*****************************************************************************/
static void OnRxDone_PPMode( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, int32_t frequncy_deviation )
{
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdReq = NULL;
   
	CO_PRINT_A_0(DBG_INFO, "\r\n ###------>>> RXDONE! - PPU MODE\r\n");	// !!!! DEBUG !!!!

   /* trigger operation in task */
   pCmdReq = osPoolAlloc(AppPool);
   if (pCmdReq)
   {
      /* store parameters */
      gRxDonePayload = payload;
      gRxDoneSize = size;
      gRxDoneRssi = rssi;
      gRxDoneSnr = snr;

      /* trigger operation in task */
      gRxDoneFlag = true;
      pCmdReq->Type = CO_MESSAGE_MAC_EVENT_E;
      osStat = osMessagePut(AppQ, (uint32_t)pCmdReq, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         osPoolFree(AppPool, pCmdReq);
      }
   }

   Radio.Standby();
}

/*************************************************************************************/
/**
* function name : void OnCadDone_PPMode( bool channelActivityDetected )
* description   : Call back function for the Cad Done interrupt.
*
* INPUTS
* @param - bool channelActivityDetected: true if activity detected, false if timeout.
*
* @return - void
*/
static void OnCadDone_PPMode( bool channelActivityDetected )
{
   /* put radio into standby for config */
   Radio.Standby( );
      
   /* Set the antenna switch to receive */
   GpioWrite(&AntRxNtx,1);
   GpioWrite(&AntTxNrx,0);
   
   // Start the radio reception.
   Radio.Rx();
}


/*****************************************************************************
*   Function:         void OnTxTimeout_PPMode( void )
*   Returns:         nothing
*   Description:   TX timeout event
*   Notes:            none
*****************************************************************************/
static void OnTxTimeout_PPMode( void )
{
   /* put radio into standby for config */
   Radio.Standby( );
      
   /* Set the antenna switch to receive */
   GpioWrite(&AntRxNtx,1);
   GpioWrite(&AntTxNrx,0);
   
   // Start the radio reception.
   Radio.Rx();
}


/*****************************************************************************
*   Function:         void OnRxTimeout_PPMode( void )
*   Returns:         nothing
*   Description:   RX timeout event
*   Notes:            none
*****************************************************************************/
static void OnRxTimeout_PPMode( void )
{
   /* put radio into standby for config */
   Radio.Standby( );
      
   /* Set the antenna switch to receive */
   GpioWrite(&AntRxNtx,1);
   GpioWrite(&AntTxNrx,0);
   
   // Start the radio reception.
   Radio.Rx();
}

/*****************************************************************************
*   Function:         void OnRxError_PPMode( void )
*   Returns:         nothing
*   Description:   RX error event
*   Notes:            none
*****************************************************************************/
static void OnRxError_PPMode( void )
{
   /* put radio into standby for config */
   Radio.Standby( );
      
   /* Set the antenna switch to receive */
   GpioWrite(&AntRxNtx,1);
   GpioWrite(&AntTxNrx,0);
   
   // Start the radio reception.
   Radio.Rx();
}

/*************************************************************************************/
/**
* MM_RBUApplicationPPMode
* Main function for RBU in Peer to Peer mode
*
* @param - unitSerialNo: This unit serial number
*
* @return - void
*/
__NO_RETURN void MM_RBUApplicationPPMode(const uint32_t unitSerialNo)
{     
	
   /* Route the LPTIM interrupt to this module ISR */
   LPTIM_CompareMatchCallback = MM_PPModeTimerISR;
   
   /* Keep a copy of our serial number */
   gUnitSerialNo = unitSerialNo;
   
   // Init SPI
   SpiInit( &SX1272.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC_E );
        
   // Init IOs
   SX1272IoInit( );
     
   // Radio initialization
   RadioEvents.TxDone = OnTxDone_PPMode;
   RadioEvents.RxDone = OnRxDone_PPMode;
   RadioEvents.CadDone = OnCadDone_PPMode;
   RadioEvents.TxTimeout = OnTxTimeout_PPMode;
   RadioEvents.RxTimeout = OnRxTimeout_PPMode;
   RadioEvents.RxError = OnRxError_PPMode;

   Radio.Init( &RadioEvents );

   uint8_t silicon_rev = Radio.Read(REG_VERSION);

   if(SEMTECH_SILICON_DEFAULT_ID != silicon_rev)
   {
      DM_LedPatternRequest(LED_BUILT_IN_TEST_FAIL_E);
      CO_PRINT_A_0(DBG_BIT_E, "SPI Radio Fail\r\n");
   }

   Radio.SetChannel( LORA_PP_MODE_FREQUENCY );
   
   // Set register RegPll (addr. 0x5C) to 0x10 to set PLL loop filter bandwidth to 75 kHz (JIRA CYG2-615).
   // This improves phase noise of device and therefore performance in the presence of an in-band blocker.
   // Impact: locking time increased by factor of 4.
   // Note that this is not suitable for the transmitter, which works best with B/W of 300 kHz (JIRA CYG2-827).
   // Hence the PLL bandwidth is set during the pre-processing phase of each receive or transmit action.
   SX1272Write( REG_LR_PLL, REG_PLL_BW_75KHZ);
   uint32_t ppu_radio_power;
   
   if (SUCCESS_E != DM_NVMRead(NV_TX_POWER_LOW_E, &ppu_radio_power, sizeof(ppu_radio_power)))
   {
      /* No valid Transmission power is stored in the NVM, use default value */
      ppu_radio_power = LORA_PP_TX_OUTPUT_POWER;
   }

   /* Configure the LoRa Modem */
   Radio.SetTxConfig( MODEM_LORA, ppu_radio_power, 0, LORA_PP_BANDWIDTH,
                                  LORA_PP_SPREADING_FACTOR, LORA_PP_CODINGRATE,
                                  LORA_PP_PREAMBLE_LENGTH, LORA_PP_FIX_LENGTH_PAYLOAD_ON,
                                  LORA_PP_INITIALISE_CRC, 0, 0, LORA_PP_IQ_INVERSION_ON, 3000 );
    
   Radio.SetRxConfig( MODEM_LORA, LORA_PP_BANDWIDTH, LORA_PP_SPREADING_FACTOR,
                                   LORA_PP_CODINGRATE, 0, LORA_PP_PREAMBLE_LENGTH,
                                   LORA_PP_SYMBOL_TIMEOUT, LORA_PP_FIX_LENGTH_PAYLOAD_ON,
                                   LORA_PP_MODE_PACKET_LENGTH, LORA_PP_INITIALISE_CRC, 0, 0, LORA_PP_IQ_INVERSION_ON, LORA_PP_RX_CONTINUOUS_MODE );
      
   /* disable interrupt in case it is running already */
   HAL_LPTIM_Disable_CMPM_Interrupt(&hlptim1);

   /* wake from sleep mode in the middle of the IWDG timeout to kick the watchdog */
   uint16_t next_wakeup = (uint16_t)(LPTIM_ReadCounter(&hlptim1) + ((PP_TIMER_TICK_INTERVAL_MS*LPTIM_TICKS_PER_SEC)/1000u));
   HAL_LPTIM_Set_CMP(&hlptim1, next_wakeup);
   HAL_LPTIM_Enable_CMPM_Interrupt(&hlptim1);
   CO_PRINT_B_0(DBG_INFO_E, "RBU Application in PP Mode\r\n");

   /* Apply an offset between devices based on their SRN to reduce collisions */
   uint32_t device_time_offset = 0;
   DM_NVMRead(NV_UNIT_SERIAL_NO_E, &device_time_offset, sizeof(uint32_t));
   device_time_offset &= 0x1F;
   /* steps in 250ms */
   device_time_offset *=250u;
   idle_time_cntr_ms = device_time_offset;
   tx_heartbeat_cntr_ms = device_time_offset;
   send_heartbeat(true); // Send first heartbeat

   /* Set the Peer to Peer mode visual indication */   
   DM_LedPatternRequest(LED_PP_MODE_ACTIVE_E);
   uint32_t led_visual_indication_step = 0;

	uint32_t		uiCountRX = 0;
	int16_t		iCount;


   while( true )
   {
      osEvent event;
      CO_Message_t *pMsg = NULL;
        
      event = osMessageGet(AppQ, osWaitForever);

      if (osEventMessage == event.status)
      {
         pMsg = (CO_Message_t *)event.value.p;
         if (pMsg)
         {

#ifdef	HDF_DX_PPU_PROGRAMMER_DEBUG
				
				// !!!! DEBUG !!!!
				// !!!! DEBUG !!!!
				// !!!! DEBUG !!!!
				// !!!! DEBUG !!!!

				uiCountRX++;
	
				CO_PRINT_B_1( DBG_NOPREFIX, "RX %0.4d  :  ", uiCountRX );
				if ( pMsg->Payload.PhyDataInd.Data[iCount] <= 24 )
				{
					for ( iCount = 0; iCount < pMsg->Payload.PhyDataInd.Size; iCount++ )
					{
						//CO_PRINT_B_1( DBG_NOPREFIX, "%0.2X ", pMsg->Payload.PhyDataInd.Data[iCount] );
						CO_PRINT_B_1( DBG_NOPREFIX, "%0.2X ", pMsg-																					hyDataReq.Data[iCount] );
					}
				}
				else
				{
					CO_PRINT_B_1( DBG_NOPREFIX, "> 24  :: == %0.2X ", pMsg->Payload.PhyDataInd.Size );
				}
				CO_PRINT_B_1( DBG_NOPREFIX, "%0.2X ", pMsg->Payload.PhyDataReq.Data[iCount] );

				// !!!! DEBUG !!!!
				// !!!! DEBUG !!!!
				// !!!! DEBUG !!!!
				// !!!! DEBUG !!!!
#endif
				
				
            switch ( pMsg->Type )
            {
               case CO_MESSAGE_MAC_EVENT_E:
               {
                  if (false != gRxDoneFlag)
                  {
                     /* reset flag */
                     gRxDoneFlag = false;
                     
                     /* Process the received Packet */
                     MM_PPModeRxDoneOperation(gRxDonePayload, gRxDoneSize, gRxDoneRssi, gRxDoneSnr);
                  }
                  break;
               }
               
               case CO_MESSAGE_GENERATE_RESPONSE_SIGNAL_E:
               {
                  uint8_t buffer[LORA_PP_MODE_PACKET_LENGTH];

                  PP_ResponseMessage_t *pResponseMsg = (PP_ResponseMessage_t *)buffer;
                  pResponseMsg->frame_type = APP_MSG_TYPE_RESPONSE_E;

                  ErrorCode_t result = PP_GenerateHeadResponse(pResponseMsg, (HeadMessage_t *)pMsg->Payload.PhyDataInd.Data);
                  
                  if (SUCCESS_E == result)
                  {
                     send_radio_packet(buffer);
                     CO_PRINT_B_0(DBG_INFO_E, "PP: Tx CMD RESP\r\n");
                  }

                  break;
               }
               
               case CO_MESSAGE_CIE_COMMAND_E:
               {
                  uint8_t* pCmdType = (uint8_t*)pMsg->Payload.PhyDataReq.Data;
                  
                  if( pCmdType )
                  {
                     CIECommand_t* pCommand = (CIECommand_t*)pMsg->Payload.PhyDataReq.Data;
                     
                     ErrorCode_t result = ERR_INVALID_POINTER_E;
                     if ( pCommand )
                     {
                        result = MM_CMD_ProcessCIECommand ( pCommand );
                        
                        if ( SUCCESS_E != result )
                        {
                           CO_PRINT_B_3(DBG_ERROR_E, "FAILED to process CIE command.  cmdType=%d, Source=%d, Error=%d\r\n", (int32_t)pCommand->CommandType, (int32_t)pCommand->PortNumber, result);
                        }
                     }
                  }
               }
               break;

               case CO_MESSAGE_TIMER_EVENT_E:
               {                  
                  if ( led_visual_indication_step == 0)
                  {
                     /* wake from sleep mode to kick the watchdog and control toggle the LED */
                     HAL_LPTIM_Set_CMP(&hlptim1, (uint16_t)(LPTIM_ReadCounter(&hlptim1) + ((PP_VIS_INDIC_INTERVAL_MS*LPTIM_TICKS_PER_SEC)/1000u)));
                     idle_time_cntr_ms += PP_VIS_INDIC_INTERVAL_MS;
                     tx_heartbeat_cntr_ms += PP_VIS_INDIC_INTERVAL_MS;
                  }
                  else
                  {
                     /* wake from sleep mode to kick the watchdog and control toggle the LED */
                     HAL_LPTIM_Set_CMP(&hlptim1, (uint16_t)(LPTIM_ReadCounter(&hlptim1) + ((PP_TIMER_TICK_INTERVAL_MS*LPTIM_TICKS_PER_SEC)/1000u)));
                     idle_time_cntr_ms += PP_TIMER_TICK_INTERVAL_MS;
                     tx_heartbeat_cntr_ms += PP_TIMER_TICK_INTERVAL_MS;
                  }
                  /* Request the current step */
                  DM_LedExternalControl(LED_PP_MODE_ACTIVE_E, led_visual_indication_step);

                  /* Move to the next step for the next call */
                  led_visual_indication_step = (led_visual_indication_step+1u)%LED_SEQUENCE_LENGTH;
                  
                  if (idle_time_cntr_ms > RBU_PP_MODE_MAX_TIME_MS)
                  {
                     /* Indicate that no PP mode is requested at the next start-up */
                     rbu_pp_mode_request = RBU_SKIP_PP_MODE_REQ;                     
                     
                     /* Send heartbeat to notify the PPU that we are going to quit */
                     send_heartbeat(false);

                     /* Wait for the end of transmission */
                     do
                     {
                        osDelay(10);
                     } while (Radio.GetStatus() == RF_TX_RUNNING);
                     
                     // Put the radio to sleep
#ifdef ALLOW_RADIO_SLEEP
                     Radio.Sleep();
#else
                     Radio.Standby();
#endif                     
                     // Reset now
                     HAL_NVIC_SystemReset();
                  }
#ifndef	HDF_DX_PPU_PROGRAMMER
                  /* Send periodic heartbeats */
                  if (tx_heartbeat_cntr_ms >= PP_HEARTBEAT_TX_PERIOD_MS)
                  {
                     send_heartbeat(true);
                     tx_heartbeat_cntr_ms = 0;
                  }
#endif
                  break;
               }

               default:
               {
                  break;
               }
            }
            osPoolFree(AppPool, pMsg);
         }
      }
   }
}


/*************************************************************************************/
/**
* MM_PPModeTimerISR
*
* This function is called in the context of the interrupt from the LPTIM timer.
*
* @param - void
*
* @return - void

*/
static void MM_PPModeTimerISR(void)
{
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdReq = NULL;
   
   /* trigger operation in task */
   pCmdReq = osPoolAlloc(AppPool);
   if (pCmdReq)
   {
      pCmdReq->Type = CO_MESSAGE_TIMER_EVENT_E;
      osStat = osMessagePut(AppQ, (uint32_t)pCmdReq, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         osPoolFree(AppPool, pCmdReq);
      }
   }
}

/*************************************************************************************/
/**
* MM_PPModeRxDoneOperation
*
* Process any LoRa received packet
*
* @param - const uint8_t *payload - pointer to payload read from sx1272
* @param - const uint16_t size - length of payload
* @param - const int16_t rssi - received signal strength indication
* @param - const int8_t snr - received signal to noise ratio
*
* @return - void
*/
static void MM_PPModeRxDoneOperation(const uint8_t * const payload, const uint16_t size, const int16_t rssi, const int8_t snr)
{
   uint32_t lSize = 0;

	
	CO_PRINT_A_0(DBG_INFO, "\r\n ########------>>>>>> ENTER RX-DONE OP!\r\n");	// !!!! DEBUG !!!!
	
   /* Sanity check of the packet length */
   if (LORA_PP_MODE_PACKET_LENGTH == size)
   {
      /* Remove the CRC bytes */
      lSize = size - 2u;

      /* Calculate the CRC on the payload */
      uint16_t expected_crc;
      
      if (SUCCESS_E == DM_Crc16bCalculate8bDataWidth((uint8_t*)payload, lSize, &expected_crc, 1u)) 
      {
         /* Fetch the CRC value from the received Packet */
         uint16_t actual_crc = payload[size - 2] << 8;
         actual_crc += payload[size - 1];     

         /* Don't process messages with failed crc */
         if (actual_crc == expected_crc)
         {           
            switch(payload[0])
            {
               case APP_MSG_TYPE_OUTPUT_SIGNAL_E:
               {
                  PP_OutputSignalCmd_t *pOutputSignalCmd = (PP_OutputSignalCmd_t *)payload;
                  uint32_t destination_serno = pOutputSignalCmd->unit_serno;
                  
                  if ( gUnitSerialNo == destination_serno)
                  {
                     MM_RBUAppPPModeOutputSig(pOutputSignalCmd->OutputProfile, pOutputSignalCmd->OutputsActivated, pOutputSignalCmd->OutputDuration);
                  }
                  break;
               }
                case APP_MSG_TYPE_COMMAND_E:
               {
                  PP_CommandMessage_t *pCommandMsg = (PP_CommandMessage_t *)payload;
                  uint32_t destination_serno = pCommandMsg->unit_serno_lo | (pCommandMsg->unit_serno_hi<<24);
                  
                  if ( gUnitSerialNo == destination_serno)
                  {
                     CO_PRINT_B_0(DBG_INFO_E, "Rx CMD\r\n");

                     /* This command is for us, reset the idle timer */
                     idle_time_cntr_ms = 0;
                     
                     /* Execute the command */
                     MM_RBUAppPPModeCommand(pCommandMsg);
                  }
                  break;
               }

               case APP_MSG_TYPE_HEARTBEAT_E:
               {
                  /* Nothing to do about other device's hearbeats */
                  break;
               }

               default:
               {
                  break;
               }
            }
         }
      }
   }
   
   /* We just processed the last received packet, check is there is anything to
    *  send back otherwise Re-start listening
    */
   if ( RF_TX_RUNNING != Radio.GetStatus())
   {
      /* Set the antenna switch to receive */
      GpioWrite( &AntRxNtx,1 );
      GpioWrite( &AntTxNrx,0 );
   
      /* put radio into standby for config */
      Radio.Standby();
         
      // Start the radio reception.
      Radio.Rx();
   }
}

/*************************************************************************************/
/**
* MM_RBUAppPPModeOutputSig
* Callback function for Output Signal message.
*
* @param - OutputProfile: Requested Output Profile
* @param - OutputsActivated: Requested Output Activation State
* @param - OutputDuration: Duration of Activation State
*
* @return - Error code
*/
static ErrorCode_t MM_RBUAppPPModeOutputSig(const uint8_t OutputProfile, const uint16_t OutputsActivated, const uint8_t OutputDuration)
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   osStatus osStat;
   CO_PRINT_B_2(DBG_INFO_E, "Output Signal - OutputProfile = %d OutputsActivated=0x%x\r\n", OutputProfile, OutputsActivated);   

   // Send the output command to the sensor head
   CO_Message_t* pPhyDataReq = osPoolAlloc(AppPool);
   if (pPhyDataReq)
   {
      pPhyDataReq->Type = CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E;
      HeadMessage_t headMessage;
      headMessage.MessageType = HEAD_WRITE_OUTPUT_ACTIVATED_E;
      headMessage.ProfileIndex = OutputProfile;
      headMessage.Value = OutputsActivated;
      headMessage.Duration = OutputDuration;
      pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
      uint8_t* pMsgData = pPhyDataReq->Payload.PhyDataReq.Data;
      memcpy(pMsgData, &headMessage, sizeof(HeadMessage_t));

      osStat = osMessagePut(HeadQ, (uint32_t)pPhyDataReq, 0);      
      
      if (osOK != osStat)
      {
         /* failed to write */
         osPoolFree(AppPool, pPhyDataReq);
      }
      else
      {
         gHeadQueueCount++;
         /* Message was queued OK. Signal the head interface task that we have sent a message */
#ifndef USE_NEW_HEAD_INTERFACE
            osSemaphoreRelease(HeadInterfaceSemaphoreId);
#endif
         result = SUCCESS_E;
      }
   }
   
   /* set / clear leds */
   if (CO_PROFILE_FIRE_E == OutputProfile) 
   {
      if (0 < OutputsActivated)
      {
         DM_LedPatternRequest(LED_FIRE_INDICATION_E);
      }
      else
      {
         DM_LedPatternRemove(LED_FIRE_INDICATION_E);
      }
   }
   else if (CO_PROFILE_FIRST_AID_E == OutputProfile) 
   {
      if (0 < OutputsActivated)
      {
         DM_LedPatternRequest(LED_FIRST_AID_INDICATION_E);
      }
      else
      {
         DM_LedPatternRemove(LED_FIRST_AID_INDICATION_E);
      }
   }
   
   return result;
}


/*************************************************************************************/
/**
* MM_RBUAppPPModeCommand
* Callback function for Command message.
*
* @param - pCommandMsg: Pointer to the command message
*
* @return - Error code
*/
static ErrorCode_t MM_RBUAppPPModeCommand(const PP_CommandMessage_t *const pCommandMsg)
{
   bool command_forwarded = false;
   static uint8_t last_command_handle = 9;
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   uint8_t buffer[LORA_PP_MODE_PACKET_LENGTH];
   PP_ResponseMessage_t *ResponseMsg = (PP_ResponseMessage_t *)buffer;

   if ( pCommandMsg )
   {
      /* populate response message */
      memset(ResponseMsg, 0u, LORA_PP_MODE_PACKET_LENGTH);
      memcpy(ResponseMsg, pCommandMsg, sizeof(PP_ResponseMessage_t));
      ResponseMsg->frame_type = APP_MSG_TYPE_RESPONSE_E;

      if ( pCommandMsg->TransactionID != last_command_handle )
      {
         last_command_handle = pCommandMsg->TransactionID;
         switch( pCommandMsg->ParameterType )
         {
            case PARAM_TYPE_DEVICE_COMBINATION_E:
            {
               if ( OP_READ == pCommandMsg->OperationType )
               {
                  /* Read from NVM */
                  result = DM_NVMRead(NV_DEVICE_COMBINATION_E, &ResponseMsg->Value, sizeof(uint32_t));
               }
               else
               {
                  result = DM_NVMWrite(NV_DEVICE_COMBINATION_E, &pCommandMsg->Value, sizeof(uint32_t));
               }
               break;
            }

            case PARAM_TYPE_RBU_SERIAL_NUMBER_E:
            {
               if ( OP_READ == pCommandMsg->OperationType )
               {
                  /* Read the Tx Power from NVM */
                  result = DM_NVMRead(NV_UNIT_SERIAL_NO_E, &ResponseMsg->Value, sizeof(uint32_t));
               }
               else
               {
                  result = DM_NVMWrite(NV_UNIT_SERIAL_NO_E, &pCommandMsg->Value, sizeof(uint32_t));
               }
               break;
            }
            
            case PARAM_TYPE_FREQUENCY_E:
            {
               if ( OP_READ == pCommandMsg->OperationType )
               {
                  /* Read the Tx Power from NVM */
                  result = DM_NVMRead(NV_FREQUENCY_CHANNEL_E, &ResponseMsg->Value, sizeof(uint32_t));
               }
               else
               {
                  result = DM_NVMWrite(NV_FREQUENCY_CHANNEL_E, &pCommandMsg->Value, sizeof(uint32_t));
               }
               break;
            }        

            case PARAM_TYPE_TX_POWER_E:
            {
               break;
            }
            
            case PARAM_TYPE_FIRMWARE_ACTIVE_IMAGE_E:
            {
               ResponseMsg->Value = 1;
               result = SUCCESS_E;
               break;            
            }

            case PARAM_TYPE_REBOOT_E:
            {
               /* send positive reponse before the reset */
               ResponseMsg->Value = 0;

               if (pCommandMsg->P1 != 0u)
               {
                  /* The user doesn't want the PP mode after the reset */
                  rbu_pp_mode_request = RBU_SKIP_PP_MODE_REQ;    
               }

               /* Send LoRa packet */
               send_radio_packet(buffer);
               
               /* Wait for the end of transmission */
               do
               {
                  osDelay(10);
               } while (Radio.GetStatus() == RF_TX_RUNNING);

               /* Send an inactive heartbeat to signal that we are leaving the PP mode */
               send_heartbeat(false);

               /* Wait for the end of transmission */
               do
               {
                  osDelay(10);
               } while (Radio.GetStatus() == RF_TX_RUNNING);

               /* Wait for any pending EEPROM write operation */
               FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
               
               /* Request a micro reset now */
               HAL_NVIC_SystemReset();
               break;
            }
            case PARAM_TYPE_SVI_E:
            {
               if ( OP_READ == pCommandMsg->OperationType )
               {
                  ResponseMsg->Value = 0;
                  uint8_t svi_command_type = ResponseMsg->P1;
                  uint8_t svi_response = 0;
                  
                  uint8_t registerAddress;         
                  result = DM_SVI_GetRegister( svi_command_type, &registerAddress );
                  if ( SUCCESS_E == result )
                  {
                     result = DM_SVI_ReadRegister( registerAddress, &svi_response );
                     if ( SUCCESS_E == result )
                     {
                        ResponseMsg->Value = svi_response;
                        result = SUCCESS_E;
                     }
                  }
               }
               else
               {
                  /* Write command. */
                  uint8_t svi_command_type = ResponseMsg->P1;
                  uint8_t registerAddress;         
                  result = DM_SVI_GetRegister( svi_command_type, &registerAddress );
                  if ( SUCCESS_E == result )
                  {
                     result = DM_SVI_WriteRegister( registerAddress, ResponseMsg->Value );
                     if ( SUCCESS_E == result )
                     {
                        result = SUCCESS_E;
                     }
                  }
               }
               break;
            }

            case PARAM_TYPE_TX_POWER_LOW_E:
            {  
               if ( OP_READ == pCommandMsg->OperationType )
               {
                  /* Read the Tx Power from NVM */
                  result = DM_NVMRead(NV_TX_POWER_LOW_E, &ResponseMsg->Value, sizeof(uint32_t));
               }
               else
               {
                  result = DM_NVMWrite(NV_TX_POWER_LOW_E, &pCommandMsg->Value, sizeof(uint32_t));
               }            
               break;
            }

            case PARAM_TYPE_TX_POWER_HIGH_E:
            {  
               if ( OP_READ == pCommandMsg->OperationType )
               {
                  /* Read the Tx Power from NVM */
                  result = DM_NVMRead(NV_TX_POWER_HIGH_E, &ResponseMsg->Value, sizeof(uint32_t));
               }
               else
               {
                  result = DM_NVMWrite(NV_TX_POWER_HIGH_E, &pCommandMsg->Value, sizeof(uint32_t));
               }            
               break;
            }

            case PARAM_TYPE_ANALOGUE_VALUE_E: /* intentional drop through */
            case PARAM_TYPE_ALARM_THRESHOLD_E:
            case PARAM_TYPE_PRE_ALARM_THRESHOLD_E:
            case PARAM_TYPE_FAULT_THRESHOLD_E:
            case PARAM_TYPE_PLUGIN_SERIAL_NUMBER_E:
            case PARAM_TYPE_PLUGIN_ENABLE_E:
            case PARAM_TYPE_PLUGIN_TEST_MODE_E:
            {
               result = PP_ProcessHeadCommand( pCommandMsg);
               if (SUCCESS_E == result)
               {
                  /* This command was forwarded to HeadInterface Task */
                  command_forwarded = true;
               }
               break;
            }

            default:
            {
               result = ERR_OUT_OF_RANGE_E;
               break;
            }
         }

         // wake up radio
         Radio.Standby();

         if ( SUCCESS_E == result )
         {
            if (!command_forwarded)
            {
               /* send positive reponse */
               send_radio_packet(buffer);
               CO_PRINT_B_0(DBG_INFO_E, "PP: Tx CMD RESP\r\n");
            }
         }
         else
         {
            /* send error reponse */
            ResponseMsg->OperationType |= OP_FAILED_FLAG;

            /* Send LoRa packet */
            send_radio_packet(buffer);

            CO_PRINT_B_1(DBG_ERROR_E, "CMD Fail. Error=%d\r\n", result);
         }
      }
      else
      {
         // wake up radio
         Radio.Standby();

         /* send error reponse */
         ResponseMsg->OperationType |= OP_FAILED_FLAG;

         /* Send LoRa packet */
         send_radio_packet(buffer);
         
         CO_PRINT_B_1(DBG_INFO_E, "Duplicated command. Handle=%d\r\n", pCommandMsg->TransactionID);

      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* send_heartbeat
*
* Send the heartbeat frame
*
* @param - active - true if active, false if about to leave th PP mode
*
* @return - void
*/
static void send_heartbeat(bool active)
{
   uint8_t buffer[LORA_PP_MODE_PACKET_LENGTH];
   memset(buffer, 0u, LORA_PP_MODE_PACKET_LENGTH);
   PP_DevicePresentMessage_t *device_present_msg = (PP_DevicePresentMessage_t *)buffer;

   /* populate heartbeat */
   device_present_msg->frame_type        = APP_MSG_TYPE_HEARTBEAT_E;
   device_present_msg->unit_serno_lo     = gUnitSerialNo & 0x00FFFFFF;
   device_present_msg->unit_serno_hi     = (gUnitSerialNo >>24) & 0x000000FF;
   device_present_msg->device_in_pp_mode = active;

   // wake up radio
   Radio.Standby();

   /* Send LoRa packet */
   send_radio_packet(buffer);

   CO_PRINT_B_0(DBG_INFO_E, "PP Heartbeat Tx\r\n");
}

/*************************************************************************************/
/**
* send_radio_packet
*
* Send a LoRa packet with a fixed length
*
* @param - active - true if active, false if about to leave th PP mode
*
* @return - void
*/
static void send_radio_packet(uint8_t *const pbuffer)
{
   uint16_t calculated_crc;

   if ( pbuffer )
   {
      /* Wait for the end of the last transmission */
      while (Radio.GetStatus() == RF_TX_RUNNING)
      {
         osDelay(10);
      };

      /* Calculate the CRC over the complete packet */
      DM_Crc16bCalculate8bDataWidth((uint8_t*)pbuffer, LORA_PP_MODE_PACKET_LENGTH - 2u, &calculated_crc, 1u);

      /* Insert the CRC value in the Tx buffer */
      pbuffer[LORA_PP_MODE_PACKET_LENGTH-2] = ((calculated_crc>>8)&0xFF);
      pbuffer[LORA_PP_MODE_PACKET_LENGTH-1] = (calculated_crc&0xFF);

      /* Set the antenna switch to transmit */
      GpioWrite(&AntTxNrx,1);
      GpioWrite(&AntRxNtx,0);
      
      /* Send the actual packet */
      Radio.Send((uint8_t*)pbuffer, LORA_PP_MODE_PACKET_LENGTH);
      SX1272SetTx(0);
   }
}

/*****************************************************************************
*  Function:   PP_ProcessHeadCommand
*  Description: Process the supplied message for a sensor head.
*  Param - pCommandMsg: The command to be processed.
*  Notes:            
*****************************************************************************/
static ErrorCode_t PP_ProcessHeadCommand (const PP_CommandMessage_t *const pCommandMsg)
{
   HeadMessage_t headMessage;
   ErrorCode_t result = SUCCESS_E;

   switch( pCommandMsg->ParameterType )
   {
      case PARAM_TYPE_ANALOGUE_VALUE_E:
         if (OP_READ == pCommandMsg->OperationType)
         {
            headMessage.MessageType = HEAD_READ_ANALOGUE_VALUE_E;
         }
         else
         {
            /* write not supported */
            result = ERR_MESSAGE_FAIL_E;
         }
      break;
      case PARAM_TYPE_ALARM_THRESHOLD_E:
         if (OP_READ == pCommandMsg->OperationType)
         {
            headMessage.MessageType = HEAD_READ_ALARM_THRESHOLD_E;
         }
         else
         {
            headMessage.MessageType = HEAD_WRITE_ALARM_THRESHOLD_E;
         }
         break;
      case PARAM_TYPE_PRE_ALARM_THRESHOLD_E:
         if (OP_READ == pCommandMsg->OperationType)
         {
            headMessage.MessageType = HEAD_READ_PREALARM_THRESHOLD_E;
         }
         else
         {
            headMessage.MessageType = HEAD_WRITE_PREALARM_THRESHOLD_E;
         }
         break;
      case PARAM_TYPE_FAULT_THRESHOLD_E:
         if (OP_READ == pCommandMsg->OperationType)
         {
            headMessage.MessageType = HEAD_READ_FAULT_THRESHOLD_E;
         }
         else
         {
            headMessage.MessageType = HEAD_WRITE_FAULT_THRESHOLD_E;
         }
         break;
      case PARAM_TYPE_FLASH_RATE_E:
         if (OP_READ == pCommandMsg->OperationType)
         {
            headMessage.MessageType = HEAD_READ_FLASH_RATE_E;
         }
         else
         {
            headMessage.MessageType = HEAD_WRITE_FLASH_RATE_E;
         }
         break;
      case PARAM_TYPE_TONE_SELECTION_E:
         if (OP_READ == pCommandMsg->OperationType)
         {
            headMessage.MessageType = HEAD_READ_TONE_SELECTION_E;
         }
         else
         {
            headMessage.MessageType = HEAD_WRITE_TONE_SELECTION_E;
         }
         break;
      case PARAM_TYPE_PLUGIN_SERIAL_NUMBER_E:               
         if (OP_READ == pCommandMsg->OperationType)
         {
            headMessage.MessageType = HEAD_READ_SERIAL_NUMBER_E;
         }
         else
         {
            /* write not supported */
            result = ERR_MESSAGE_FAIL_E;
         }
         break;
      case PARAM_TYPE_PLUGIN_ENABLE_E:
         if (OP_READ == pCommandMsg->OperationType)
         {
            headMessage.MessageType = HEAD_READ_ENABLED_E;
         }
         else
         {
            headMessage.MessageType = HEAD_WRITE_ENABLED_E;
         }
         break;
      case PARAM_TYPE_PLUGIN_TEST_MODE_E:
         if (OP_READ == pCommandMsg->OperationType)
         {
            headMessage.MessageType = HEAD_READ_TEST_MODE_E;
         }
         else
         {
            headMessage.MessageType = HEAD_WRITE_TEST_MODE_E;
         }
         break;
      default:
         result = ERR_MESSAGE_FAIL_E;
         break;
   }
   
   if ( result == SUCCESS_E)
   {
      /* we recognised the message type.  now pack the head message */
      headMessage.Value = pCommandMsg->Value; 
      headMessage.TransactionID = pCommandMsg->TransactionID;
      headMessage.ChannelNumber = pCommandMsg->P1;
      headMessage.ProfileIndex = pCommandMsg->P2;
      /* send the message to the head */
      CO_Message_t* pPhyDataReq = osPoolAlloc(AppPool);
      if (pPhyDataReq)
      {
         pPhyDataReq->Type = CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E;
         uint8_t* pMsgData = pPhyDataReq->Payload.PhyDataReq.Data;
         memcpy(pMsgData, &headMessage, sizeof(HeadMessage_t));
         pPhyDataReq->Payload.PhyDataReq.Size = sizeof(HeadMessage_t);

         osStatus osStat = osMessagePut(HeadQ, (uint32_t)pPhyDataReq, 0);      
         
         if (osOK != osStat)
         {
            /* failed to put message in the head queue */
            osPoolFree(AppPool, pPhyDataReq);
            result = ERR_QUEUE_OVERFLOW_E;
         }
         else
         {
            gHeadQueueCount++;
            /* Message was queued OK. Signal the head interface task that we have sent a message */
#ifndef USE_NEW_HEAD_INTERFACE
            osSemaphoreRelease(HeadInterfaceSemaphoreId);
#endif
            
            result = SUCCESS_E;
         }
      }
   }

   return result;
}

/*****************************************************************************
*  Function:    PP_GenerateHeadResponse
*  Description: Process the response message coming from a sensor head.
*  Param - pResponseMsg: Processed response for the radio comms
*  Param - pHeadResponse:   The actual response from the Head sensor
*  Returns:          SUCCESS_E if the response was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
static ErrorCode_t PP_GenerateHeadResponse (PP_CommandMessage_t *const pResponseMsg, const HeadMessage_t *const pHeadResponse)
{
   ErrorCode_t result = SUCCESS_E;

   /* populate response message */
   switch(pHeadResponse->MessageType)
   {
      case HEAD_WRITE_ENABLED_E:
      {
         pResponseMsg->ParameterType = PARAM_TYPE_PLUGIN_ENABLE_E;
         pResponseMsg->OperationType = OP_WRITE;
         break;
      }
      case HEAD_WRITE_PREALARM_THRESHOLD_E:
      {
         pResponseMsg->ParameterType = PARAM_TYPE_PRE_ALARM_THRESHOLD_E;
         pResponseMsg->OperationType = OP_WRITE;
         break;
      }
      case HEAD_WRITE_ALARM_THRESHOLD_E:
      {
         pResponseMsg->ParameterType = PARAM_TYPE_ALARM_THRESHOLD_E;
         pResponseMsg->OperationType = OP_WRITE;
         break;
      }
      case HEAD_WRITE_FAULT_THRESHOLD_E:
      {
         pResponseMsg->ParameterType = PARAM_TYPE_FAULT_THRESHOLD_E;
         pResponseMsg->OperationType = OP_WRITE;
         break;
      }
      case HEAD_WRITE_FLASH_RATE_E:
      {
         pResponseMsg->ParameterType = PARAM_TYPE_FLASH_RATE_E;
         pResponseMsg->OperationType = OP_WRITE;
         break;
      }
      case HEAD_WRITE_TONE_SELECTION_E:
      {
         pResponseMsg->ParameterType = PARAM_TYPE_TONE_SELECTION_E;
         pResponseMsg->OperationType = OP_WRITE;
         break;
      }
      case HEAD_WRITE_TEST_MODE_E:
      {
         pResponseMsg->ParameterType = PARAM_TYPE_PLUGIN_TEST_MODE_E;
         pResponseMsg->OperationType = OP_WRITE;
         break;
      }
      case HEAD_READ_ENABLED_E:
      {
         pResponseMsg->ParameterType = PARAM_TYPE_PLUGIN_ENABLE_E;
         pResponseMsg->OperationType = OP_READ;
         break;
      }
      case HEAD_READ_PREALARM_THRESHOLD_E:
      {
         pResponseMsg->ParameterType = PARAM_TYPE_PRE_ALARM_THRESHOLD_E;
         pResponseMsg->OperationType = OP_READ;
         break;
      }
      case HEAD_READ_ALARM_THRESHOLD_E:
      {
         pResponseMsg->ParameterType = PARAM_TYPE_ALARM_THRESHOLD_E;
         pResponseMsg->OperationType = OP_READ;
         break;
      }
      case HEAD_READ_FAULT_THRESHOLD_E:
      {
         pResponseMsg->ParameterType = PARAM_TYPE_FAULT_THRESHOLD_E;
         pResponseMsg->OperationType = OP_READ;
         break;
      }
      case HEAD_READ_FLASH_RATE_E:
      {
         pResponseMsg->ParameterType = PARAM_TYPE_FLASH_RATE_E;
         pResponseMsg->OperationType = OP_READ;
         break;
      }
      case HEAD_READ_TONE_SELECTION_E:
      {
         pResponseMsg->ParameterType = PARAM_TYPE_TONE_SELECTION_E;
         pResponseMsg->OperationType = OP_READ;
         break;
      }
      case HEAD_READ_TEST_MODE_E:
      {
         pResponseMsg->ParameterType = PARAM_TYPE_PLUGIN_TEST_MODE_E;
         pResponseMsg->OperationType = OP_READ;
         break;
      }
      case HEAD_READ_ANALOGUE_VALUE_E:
      {
         pResponseMsg->ParameterType = PARAM_TYPE_ANALOGUE_VALUE_E;
         pResponseMsg->OperationType = OP_READ;
         break;
      }
      case HEAD_READ_SERIAL_NUMBER_E:
      {
         pResponseMsg->ParameterType = PARAM_TYPE_PLUGIN_SERIAL_NUMBER_E;
         pResponseMsg->OperationType = OP_READ;
         break;
      }
      
      default:
      {
         /* Not supported */
         result = ERR_OUT_OF_RANGE_E;
         break;
      }
   }

   pResponseMsg->unit_serno_lo = gUnitSerialNo & 0x00FFFFFF;
   pResponseMsg->unit_serno_hi = (gUnitSerialNo >>24) & 0x000000FF;
   pResponseMsg->TransactionID = pHeadResponse->TransactionID;
   pResponseMsg->P1            = pHeadResponse->ChannelNumber;
   pResponseMsg->P2            = pHeadResponse->ProfileIndex;
   pResponseMsg->Value         = pHeadResponse->Value;

   return result;
}
