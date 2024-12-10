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
*  File         : MM_PPUApplicationTask.c
*
*  Description  : PPU Application code
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
#include "board.h"
#include "CO_SerialDebug.h"
#include "MC_MAC.h"
#include "MC_PUP.h"
#include "SM_StateMachine.h"
#include "MM_ATHandleTask.h"
#include "MM_ATCommand.h"
#include "MM_CommandProcessor.h"
#include "MM_Interrupts.h"
#include "MM_Main.h"
#include "MM_PPUApplicationTask.h"

/* Private Functions Prototypes
*************************************************************************************/
static void OnTxDone_PPU( void );
static void OnRxDone_PPU( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, int32_t freq_dev );
static void OnTxTimeout_PPU( void );
static void OnRxTimeout_PPU( void );
static void OnRxError_PPU( void );
static void MM_PPURxDoneOperation(const uint8_t * const payload, const uint16_t size, const int16_t rssi, const int8_t snr);
static ErrorCode_t generateOutputSignal(const CO_OutputData_t *const pOutData);
static ErrorCode_t generateCommand(const CO_CommandData_t * const CommandData);
static void GenerateDetectedDeviceList(const uint16_t uart_source);
static void send_pp_mode_broadcast_req(void);
static void send_radio_packet(uint8_t *const pbuffer, const uint8_t size);

/* Private definitions
*************************************************************************************/
typedef enum
{
   RBU_NOT_VALID,
   RBU_DETECTED,
   RBU_CONNECTED
} MM_PPUAppRbuState_t;

/* Global Variables
*************************************************************************************/


extern RadioEvents_t RadioEvents;
extern osPoolId AppPool;
extern osMessageQId(AppQ);

extern osPoolId ATHandlePool;
extern osMutexId(DebugPortMutexId);

/* Private Variables
*************************************************************************************/
#define PPU_MAX_SUPPORTED_RBU      32
#define PPU_HEARTBEAT_RX_TIMEOUT_S 60
#define PPU_TIMER_TICK_INTERVAL_S  MAX(1u, IWDG_TIMEOUT_PERIOD_MS/(2*1000))

static uint32_t broadcast_radio_frequency = LORA_PP_MODE_FREQUENCY;
static uint32_t broadcast_system_id = 0;
static uint32_t ppu_radio_power = LORA_PP_TX_OUTPUT_POWER;
static uint32_t last_unit_serno = 0u;
static uint8_t last_transaction_id = 0u;
static uint8_t last_parameter_type = 0u;
static CommandSource_t last_cmd_source;

static struct
{
   uint32_t serno;
   MM_PPUAppRbuState_t state;
   uint32_t last_seen_time;
}rbu_devices_list[PPU_MAX_SUPPORTED_RBU];

static RadioEvents_t RadioEvents;
static int16_t gRxDoneRssi = 0; 
static int8_t  gRxDoneSnr = 0;
static uint8_t *gRxDonePayload = NULL;
static uint16_t gRxDoneSize = 0;
static bool gRxDoneFlag = false;
//static int32_t gRxFrequencyDeviation = 0;
static uint32_t time_counter_sec = 0;

extern uint32_t GetNextHandle(void);

/*****************************************************************************
*   Function:         void OnTxDone_PPU( void )
*   Returns:         nothing
*   Description:   TX done event
*   Notes:            none
*****************************************************************************/
static void OnTxDone_PPU( void )
{
   /* put radio into standby for config */
   Radio.Standby( );

   /* Switch to the Peer to Peer Frequency */
   Radio.SetChannel( LORA_PP_MODE_FREQUENCY );

   /* Reconfigure the reception attributes */
   Radio.SetRxConfig( MODEM_LORA, LORA_PP_BANDWIDTH, LORA_PP_SPREADING_FACTOR,
                      LORA_PP_CODINGRATE, 0, LORA_PP_PREAMBLE_LENGTH,
                      LORA_PP_SYMBOL_TIMEOUT, LORA_PP_FIX_LENGTH_PAYLOAD_ON,
                      LORA_PP_MODE_PACKET_LENGTH, LORA_PP_INITIALISE_CRC, 0, 0,
                      LORA_PP_IQ_INVERSION_ON, LORA_PP_RX_CONTINUOUS_MODE );

   /* Set the antenna switch to receive */
   GpioWrite(&AntRxNtx,1);
   GpioWrite(&AntTxNrx,0);
   
   /* Start the radio reception */
   Radio.Rx();
}

/*****************************************************************************
*   Function:         void OnRxDone_PPU( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, int32_t freq_dev )
*   Returns:         nothing
* arguments:      uint8_t *payload - pointer to a data buffer
*                        uint16_t size - number of bytes in the buffer
*                        int16_t rssi - rssi value of rx data
*                        int8_t snr - snr value of rx data
*                        int32_t freq_dev - frequency deviation between Rx and received signal
*   Description:   RX done event
*   Notes:            none
*****************************************************************************/
static void OnRxDone_PPU( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, int32_t freq_dev )
{
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdReq = NULL;
   
   /* trigger operation in task */
   pCmdReq = osPoolAlloc(AppPool);
   if (NULL != pCmdReq)
   {
      /* store parameters */
      gRxDonePayload = payload;
      gRxDoneSize = size;
      gRxDoneRssi = rssi;
      gRxDoneSnr = snr;
//      gRxFrequencyDeviation = freq_dev;

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
* function name : void OnCadDone_PPU( bool channelActivityDetected )
* description   : Call back function for the Cad Done interrupt.
*
* INPUTS
* @param - bool channelActivityDetected: true if activity detected, false if timeout.
*
* @return - void
*/
static void OnCadDone_PPU( bool channelActivityDetected )
{
   /* put radio into standby for config */
   Radio.Standby( );

   /* Switch to the Peer to Peer Frequency */
   Radio.SetChannel( LORA_PP_MODE_FREQUENCY );

   /* Reconfigure the reception attributes */
   Radio.SetRxConfig( MODEM_LORA, LORA_PP_BANDWIDTH, LORA_PP_SPREADING_FACTOR,
                      LORA_PP_CODINGRATE, 0, LORA_PP_PREAMBLE_LENGTH,
                      LORA_PP_SYMBOL_TIMEOUT, LORA_PP_FIX_LENGTH_PAYLOAD_ON,
                      LORA_PP_MODE_PACKET_LENGTH, LORA_PP_INITIALISE_CRC, 0, 0,
                      LORA_PP_IQ_INVERSION_ON, LORA_PP_RX_CONTINUOUS_MODE );

   /* Set the antenna switch to receive */
   GpioWrite(&AntRxNtx,1);
   GpioWrite(&AntTxNrx,0);
   
   /* Start the radio reception */
   Radio.Rx();
}


/*****************************************************************************
*   Function:         void OnTxTimeout_PPU( void )
*   Returns:         nothing
*   Description:   TX timeout event
*   Notes:            none
*****************************************************************************/
static void OnTxTimeout_PPU( void )
{
   /* put radio into standby for config */
   Radio.Standby( );

   /* Switch to the Peer to Peer Frequency */
   Radio.SetChannel( LORA_PP_MODE_FREQUENCY );

   /* Reconfigure the reception attributes */
   Radio.SetRxConfig( MODEM_LORA, LORA_PP_BANDWIDTH, LORA_PP_SPREADING_FACTOR,
                      LORA_PP_CODINGRATE, 0, LORA_PP_PREAMBLE_LENGTH,
                      LORA_PP_SYMBOL_TIMEOUT, LORA_PP_FIX_LENGTH_PAYLOAD_ON,
                      LORA_PP_MODE_PACKET_LENGTH, LORA_PP_INITIALISE_CRC, 0, 0,
                      LORA_PP_IQ_INVERSION_ON, LORA_PP_RX_CONTINUOUS_MODE );

   /* Set the antenna switch to receive */
   GpioWrite(&AntRxNtx,1);
   GpioWrite(&AntTxNrx,0);
   
   /* Start the radio reception */
   Radio.Rx();
}

/*****************************************************************************
*   Function:         void OnRxTimeout_PPU( void )
*   Returns:         nothing
*   Description:   RX timeout event
*   Notes:            none
*****************************************************************************/
static void OnRxTimeout_PPU( void )
{
   /* put radio into standby for config */
   Radio.Standby( );

   /* Switch to the Peer to Peer Frequency */
   Radio.SetChannel( LORA_PP_MODE_FREQUENCY );

   /* Reconfigure the reception attributes */
   Radio.SetRxConfig( MODEM_LORA, LORA_PP_BANDWIDTH, LORA_PP_SPREADING_FACTOR,
                      LORA_PP_CODINGRATE, 0, LORA_PP_PREAMBLE_LENGTH,
                      LORA_PP_SYMBOL_TIMEOUT, LORA_PP_FIX_LENGTH_PAYLOAD_ON,
                      LORA_PP_MODE_PACKET_LENGTH, LORA_PP_INITIALISE_CRC, 0, 0,
                      LORA_PP_IQ_INVERSION_ON, LORA_PP_RX_CONTINUOUS_MODE );

   /* Set the antenna switch to receive */
   GpioWrite(&AntRxNtx,1);
   GpioWrite(&AntTxNrx,0);
   
   /* Start the radio reception */
   Radio.Rx();
}

/*****************************************************************************
*   Function:         void OnRxError_PPU( void )
*   Returns:         nothing
*   Description:   RX error event
*   Notes:            none
*****************************************************************************/
static void OnRxError_PPU( void )
{
   /* put radio into standby for config */
   Radio.Standby( );

   /* Switch to the Peer to Peer Frequency */
   Radio.SetChannel( LORA_PP_MODE_FREQUENCY );

   /* Reconfigure the reception attributes */
   Radio.SetRxConfig( MODEM_LORA, LORA_PP_BANDWIDTH, LORA_PP_SPREADING_FACTOR,
                      LORA_PP_CODINGRATE, 0, LORA_PP_PREAMBLE_LENGTH,
                      LORA_PP_SYMBOL_TIMEOUT, LORA_PP_FIX_LENGTH_PAYLOAD_ON,
                      LORA_PP_MODE_PACKET_LENGTH, LORA_PP_INITIALISE_CRC, 0, 0,
                      LORA_PP_IQ_INVERSION_ON, LORA_PP_RX_CONTINUOUS_MODE );

   /* Set the antenna switch to receive */
   GpioWrite(&AntRxNtx,1);
   GpioWrite(&AntTxNrx,0);
   
   /* Start the radio reception */
   Radio.Rx();
}

/*************************************************************************************/
/**
* MM_PPUApplicationInit
* Initialisation function for the PPU Application
*
* @param - void
*
* @return - void
*/
void MM_PPUApplicationInit(void)
{   
   for (uint32_t idx =0; idx < PPU_MAX_SUPPORTED_RBU; idx++)
   {
      rbu_devices_list[idx].state = RBU_NOT_VALID;
   }

   /* Init SPI */
   SpiInit( &SX1272.Spi, RADIO_MOSI, RADIO_MISO, RADIO_SCLK, NC );

   /* Init IOs */
   SX1272IoInit( );

   /* Radio initialization */
   RadioEvents.TxDone = OnTxDone_PPU;
   RadioEvents.RxDone = OnRxDone_PPU;
   RadioEvents.CadDone = OnCadDone_PPU;
   RadioEvents.TxTimeout = OnTxTimeout_PPU;
   RadioEvents.RxTimeout = OnRxTimeout_PPU;
   RadioEvents.RxError = OnRxError_PPU;

   Radio.Init( &RadioEvents );

   uint8_t silicon_rev = Radio.Read(REG_VERSION);

   if(SEMTECH_SILICON_DEFAULT_ID != silicon_rev)
   {
      DM_LedPatternRequest(LED_BUILT_IN_TEST_FAIL_E);
      CO_PRINT_A_0(DBG_BIT, "SPI Radio Fail\r\n");
   }

   Radio.SetChannel( LORA_PP_MODE_FREQUENCY );

   SX1272Write( REG_LR_PLL, REG_PLL_BW_75KHZ);

   Radio.SetTxConfig( MODEM_LORA, ppu_radio_power, 0, LORA_PP_BANDWIDTH,
                      LORA_PP_SPREADING_FACTOR, LORA_PP_CODINGRATE,
                      LORA_PP_PREAMBLE_LENGTH, LORA_PP_FIX_LENGTH_PAYLOAD_ON,
                      LORA_PP_INITIALISE_CRC, 0, 0, LORA_PP_IQ_INVERSION_ON, 3000 );

   Radio.SetRxConfig( MODEM_LORA, LORA_PP_BANDWIDTH, LORA_PP_SPREADING_FACTOR,
                      LORA_PP_CODINGRATE, 0, LORA_PP_PREAMBLE_LENGTH,
                      LORA_PP_SYMBOL_TIMEOUT, LORA_PP_FIX_LENGTH_PAYLOAD_ON,
                      LORA_PP_MODE_PACKET_LENGTH, LORA_PP_INITIALISE_CRC, 0, 0,
                      LORA_PP_IQ_INVERSION_ON, LORA_PP_RX_CONTINUOUS_MODE );

   /* Set the Modem in sleep mode until required */
#ifdef ALLOW_RADIO_SLEEP
      Radio.Sleep();
#else
   Radio.Standby();
#endif
}

/*************************************************************************************/
/**
* MM_PPURxDoneOperation
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
static void MM_PPURxDoneOperation(const uint8_t * const payload, const uint16_t size, const int16_t rssi, const int8_t snr)
{
   uint32_t lSize = 0;

   /* Sanity check of the packet length */
   if ( LORA_PP_MODE_PACKET_LENGTH == size)
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
               case APP_MSG_TYPE_RESPONSE_E:
               {
                  PP_ResponseMessage_t *ResponseMsg = (PP_ResponseMessage_t *)payload;
                  uint32_t source_serno = ResponseMsg->unit_serno_lo | (ResponseMsg->unit_serno_hi<<24);

                  if ( ( source_serno               == last_unit_serno)
                     &&( ResponseMsg->TransactionID == last_transaction_id)
                     &&( ResponseMsg->ParameterType == last_parameter_type))
                  {
                     if (ResponseMsg->OperationType & OP_FAILED_FLAG )
                     {
                        /* Error Flag was set, send negative response response */
                        MM_CMD_SendATMessage(AT_RESPONSE_FAIL, last_cmd_source);
                     }
                     else
                     {
                        /* Positive response */
                        char response[AT_RESPONSE_MAX_LENGTH];

                        sprintf(response, "%d", ResponseMsg->Value );

                        MM_CMD_SendATMessage(response, last_cmd_source);
                     }
                  }
                  break;
               }

               case APP_MSG_TYPE_HEARTBEAT_E:
               {
                  PP_DevicePresentMessage_t *DevicePresentMessage = (PP_DevicePresentMessage_t *)payload;
                  uint32_t source_serno = DevicePresentMessage->unit_serno_lo | (DevicePresentMessage->unit_serno_hi<<24);
                  
                  bool is_known_device = false;
                  
                  for (uint32_t idx = 0; idx < PPU_MAX_SUPPORTED_RBU && !is_known_device; idx++)
                  {
                     if ( rbu_devices_list[idx].serno == source_serno && rbu_devices_list[idx].state != RBU_NOT_VALID)
                     {
                        is_known_device = true;
                        if ( DevicePresentMessage->device_in_pp_mode == false)
                        {
                           /* This RBU is moving to the Mesh mode */
                           rbu_devices_list[idx].state = RBU_NOT_VALID;
                           char serial_no_str[32];
                           MM_ATDecodeSerialNumber(rbu_devices_list[idx].serno, serial_no_str);
                           CO_PRINT_B_1(DBG_INFO, "PPU: Remove device %s\r\n", serial_no_str);
                        }      
                        else
                        {
                           rbu_devices_list[idx].last_seen_time = time_counter_sec;
                        }                  
                     }
                  }

                  if (false == is_known_device)
                  {
                     for (uint32_t idx = 0; idx < PPU_MAX_SUPPORTED_RBU; idx++)
                     {
                        if ( rbu_devices_list[idx].state == RBU_NOT_VALID && DevicePresentMessage->device_in_pp_mode != false)
                        {
                           rbu_devices_list[idx].state = RBU_DETECTED;
                           rbu_devices_list[idx].serno = source_serno;
                           rbu_devices_list[idx].last_seen_time = time_counter_sec;
                           char serial_no_str[32];
                           MM_ATDecodeSerialNumber(rbu_devices_list[idx].serno, serial_no_str);
                           CO_PRINT_B_1(DBG_INFO, "PPU: Add device %s\r\n", serial_no_str);
                           break;
                        }
                     }
                  }
                  break;
               }
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* generateOutputSignal
* Generate an Output Signal message and send it on the LoRa 
*
* @param - pointer to the Output Signal data
*
* @return - SUCCESS_E

*/
static ErrorCode_t generateOutputSignal(const CO_OutputData_t *const pOutData)
{
   uint8_t buffer[LORA_PP_MODE_PACKET_LENGTH];

   PP_OutputSignalCmd_t *command = (PP_OutputSignalCmd_t *)buffer;
            
   /* The command is addressed to a remote node.  Queue it for transmission */
   /* populate command */
   command->frame_type       = APP_MSG_TYPE_OUTPUT_SIGNAL_E;
   command->unit_serno       = pOutData->UnitSerno ;
   command->OutputProfile    = pOutData->OutputProfile;
   command->OutputsActivated = pOutData->OutputsActivated;
   command->OutputDuration   = pOutData->OutputDuration;
   
   /* Keep track of this command */
   last_unit_serno     = command->unit_serno;

   /* wake up radio */
   Radio.Standby();

   /* Switch to the Peer to Peer Frequency */
   Radio.SetChannel( LORA_PP_MODE_FREQUENCY );

   /* Reconfigure the Transmission settings */
   Radio.SetTxConfig( MODEM_LORA, ppu_radio_power, 0, LORA_PP_BANDWIDTH,
                      LORA_PP_SPREADING_FACTOR, LORA_PP_CODINGRATE,
                      LORA_PP_PREAMBLE_LENGTH, LORA_PP_FIX_LENGTH_PAYLOAD_ON,
                      LORA_PP_INITIALISE_CRC, 0, 0, LORA_PP_IQ_INVERSION_ON, 3000 );


   /* Send LoRa packet */
   send_radio_packet(buffer, LORA_PP_MODE_PACKET_LENGTH);
   
   return SUCCESS_E;
}

/*************************************************************************************/
/**
* GenerateDetectedDeviceList
* send the list of the known device
*
* @param - uart source port
*
* @return - void
*/
static void GenerateDetectedDeviceList(const uint16_t uart_source)
{
   char device_list[512] = "PPLST: ";
   char device_serno[16];

   for (uint32_t idx = 0; idx< PPU_MAX_SUPPORTED_RBU; idx++)
   {
      if (rbu_devices_list[idx].state != RBU_NOT_VALID)
      {
         sprintf(device_serno,"0x%08X,", rbu_devices_list[idx].serno);
         strcat(device_list, device_serno);            
      }
   }

   uint32_t msg_length = strlen(device_list);
   if (device_list[msg_length - 1u] == ',')
   {
      /*Delete last coma */
      device_list[msg_length - 1u] = 0;
   }

   /* Add end of line characters */
   strcat(device_list, "\r\n");
   msg_length += 2u;

   if (DEBUG_UART == uart_source )
   {
      /* Acquire the mutex before access to the debug port */
      if (osMutexWait(DebugPortMutexId, 10u) == osOK) 
      {
         SerialPortWriteTxBuffer(DEBUG_UART, (uint8_t*)device_list, msg_length);

         /* Release the Mutex */
         osMutexRelease(DebugPortMutexId);
      }
   }
   else
   {
      /* Send the response on the relevant serial port */
      SerialPortWriteTxBuffer(uart_source, (uint8_t*)device_list, msg_length);
   }
}

/*************************************************************************************/
/**
* send_pp_mode_broadcast_req
*
* Send the heartbeat frame
*
* @return - void
*/
static void send_pp_mode_broadcast_req(void)
{
   FrameHeartbeat_t heartbeat = { 0 };
   CO_Message_t message;
   ErrorCode_t status = SUCCESS_E;

   /* populate heartbeat */
   heartbeat.FrameType            = FRAME_TYPE_HEARTBEAT_E;
   heartbeat.SlotIndex            = MIN_DCH_INDEX_IN_SHORT_FRAME;
   heartbeat.ShortFrameIndex      = 0u;
   heartbeat.LongFrameIndex       = 0u;
   heartbeat.State                = STATE_TEST_MODE_E;
   heartbeat.Rank                 = UNASSIGNED_RANK;
   heartbeat.NoOfChildrenIdx      = 0u;
   heartbeat.NoOfChildrenOfPTNIdx = 0u;
   heartbeat.SystemId             = broadcast_system_id;

   message.Type = CO_MESSAGE_PHY_DATA_REQ_E;
   status = MC_PUP_PackHeartbeat(&heartbeat, &message.Payload.PhyDataReq);

   if (SUCCESS_E == status)
   {
      // wake up radio
      Radio.Standby();
      
      /* Set the selected Frequency */
      Radio.SetChannel(broadcast_radio_frequency);
      
      /* Configure the Tx Setting as for a DCH packet */
      Radio.SetTxConfig( MODEM_LORA, ppu_radio_power, 0, LORA_BANDWIDTH,
                         LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                         LORA_DCH_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                         LORA_INITIALISE_CRC, 0, 0, LORA_IQ_INVERSION_ON, 3000 );

      /* Send LoRa packet */
      send_radio_packet(message.Payload.PhyDataReq.Data, PHY_DCH_PACKET_SIZE);
      
      CO_PRINT_B_0(DBG_INFO, "Tx: PP Req Broadcast\r\n");
   }
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
static void send_radio_packet(uint8_t *const pbuffer, const uint8_t size)
{
   uint16_t calculated_crc;
   /* Calculate the CRC over the complete packet */
   DM_Crc16bCalculate8bDataWidth((uint8_t*)pbuffer, size - 2u, &calculated_crc, 1u);
   
   /* Insert the CRC value in the Tx buffer */
   pbuffer[size-2] = ((calculated_crc>>8)&0xFF);
   pbuffer[size-1] = (calculated_crc&0xFF);
   
   /* Set the antenna switch to transmit */
   GpioWrite(&AntTxNrx,1);
   GpioWrite(&AntRxNtx,0);
   
   /* Send the actual packet */
   Radio.Send((uint8_t*)pbuffer, size);                     
   SX1272SetTx(0);
}

/*************************************************************************************/
/**
* MM_PPUApplicationTaskMain
* Main function for RBU in Peer to Peer mode
*
* @param - void
*
* @return - void
*/
void MM_PPUApplicationTaskMain(void)
{
   CO_PRINT_B_0(DBG_INFO, "PPU Application Running\r\n");
   
   MM_PPUApplicationInit();

   /* disable interrupt in case it is running already */
   HAL_LPTIM_Disable_CMPM_Interrupt(&hlptim1);

   /* wake from sleep mode in the middle of the IWDG timeout to kick the watchdog */
   uint16_t next_wakeup = (uint16_t)(LPTIM_ReadCounter(&hlptim1) + (PPU_TIMER_TICK_INTERVAL_S*LPTIM_TICKS_PER_SEC));
   HAL_LPTIM_Set_CMP(&hlptim1, next_wakeup);
   HAL_LPTIM_Enable_CMPM_Interrupt(&hlptim1);
  
   while(true)
   {
      osEvent event;
      CO_Message_t *pMsg = NULL;
      event = osMessageGet(AppQ, osWaitForever);            
      
      if (osEventMessage == event.status)
      {
         pMsg = (CO_Message_t *)event.value.p;
         if (NULL != pMsg)
         {
            switch (pMsg->Type)
            {
               case CO_MESSAGE_GENERATE_SET_STATE_E:
               {
                  CO_CommandData_t *pCommand = (CO_CommandData_t*)pMsg->Payload.PhyDataReq.Data;

                  if( pCommand )
                  {
                     CommandSource_t cmd_source = MM_CMD_ConvertFromATCommandSource(pCommand->Source);

                     if( 0u == pCommand->Value )
                     {
#ifdef ALLOW_RADIO_SLEEP
                        Radio.Sleep();
#else
                        Radio.Standby();
#endif
                        /* Forget all the devices */
                        for (uint32_t idx = 0; idx < PPU_MAX_SUPPORTED_RBU; idx++)
                        {
                           rbu_devices_list[idx].state = RBU_NOT_VALID;
                        }
                        CO_PRINT_B_0(DBG_INFO, "PPU set to sleep\r\n");
                        MM_CMD_SendATMessage(AT_RESPONSE_OK, cmd_source);
                     }
                     else if( 1u == pCommand->Value )
                     {
                        /* put radio into standby for config */
                        Radio.Standby();

                        /* Switch to the Peer to Peer Frequency */
                        Radio.SetChannel( LORA_PP_MODE_FREQUENCY );
                        
                        /* Re-configure the Radio */
                        Radio.SetTxConfig( MODEM_LORA, ppu_radio_power, 0, LORA_PP_BANDWIDTH,
                                           LORA_PP_SPREADING_FACTOR, LORA_PP_CODINGRATE,
                                           LORA_PP_PREAMBLE_LENGTH, LORA_PP_FIX_LENGTH_PAYLOAD_ON,
                                           LORA_PP_INITIALISE_CRC, 0, 0, LORA_PP_IQ_INVERSION_ON, 3000 );
       
                        Radio.SetRxConfig( MODEM_LORA, LORA_PP_BANDWIDTH, LORA_PP_SPREADING_FACTOR,
                                           LORA_PP_CODINGRATE, 0, LORA_PP_PREAMBLE_LENGTH,
                                           LORA_PP_SYMBOL_TIMEOUT, LORA_PP_FIX_LENGTH_PAYLOAD_ON,
                                           LORA_PP_MODE_PACKET_LENGTH, LORA_PP_INITIALISE_CRC, 0, 0,
                                           LORA_PP_IQ_INVERSION_ON, LORA_PP_RX_CONTINUOUS_MODE );
                        
                        /* Set the antenna switch to receive */
                        GpioWrite(&AntRxNtx,1);
                        GpioWrite(&AntTxNrx,0);
   
                        /* Start Listening */
                        Radio.Rx();
                        CO_PRINT_B_0(DBG_INFO, "PPU started listening\r\n");
                        MM_CMD_SendATMessage(AT_RESPONSE_OK, cmd_source);
                     }
                     else
                     {
                        /* This mode not supported by the PPU*/
                        MM_CMD_SendATMessage(AT_RESPONSE_FAIL, cmd_source);
                     }
                  }
                  break;
               }

               case CO_MESSAGE_GENERATE_OUTPUT_SIGNAL_E:
               {
                  /* extract the output data from the received message */
                  CO_OutputData_t *pOutData = (CO_OutputData_t *)pMsg->Payload.PhyDataReq.Data;
                  
                  if( pOutData )
                  {
                     /* find out which usart to respond to */
                     last_cmd_source = MM_CMD_ConvertFromATCommandSource(pOutData->Source);
                     
                     ErrorCode_t result = generateOutputSignal ( pOutData);
                     if ( SUCCESS_E != result )
                     {
                        CO_PRINT_B_1(DBG_ERROR, "FAILED to process command.  Error=%d\r\n", result);
                        MM_CMD_SendATMessage(AT_RESPONSE_FAIL, last_cmd_source);
                     }
                     else
                     {
                        /* Send positive response */
                        MM_CMD_SendATMessage(AT_RESPONSE_OK, last_cmd_source);
                     }
                  }
               }
               break;

               case CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E:
               {
                  CO_CommandData_t *pCommand = (CO_CommandData_t*)pMsg->Payload.PhyDataReq.Data;
                  if( pCommand )
                  {
                     last_cmd_source = MM_CMD_ConvertFromATCommandSource(pCommand->Source);
                     ErrorCode_t result = generateCommand(pCommand);
                     if ( SUCCESS_E != result )
                     {
                        CO_PRINT_B_1(DBG_ERROR, "FAILED to process command.  Error=%d\r\n", result);
                        MM_CMD_SendATMessage(AT_RESPONSE_FAIL, last_cmd_source);
                     }
                  }
                  break;
               }
               
               case CO_MESSAGE_GENERATE_PPU_DEV_LIST_E:
               {
                  CO_CommandData_t *pCommand = (CO_CommandData_t*)pMsg->Payload.PhyDataReq.Data;
                  last_cmd_source = MM_CMD_ConvertFromATCommandSource(pCommand->Source);
                  
                  GenerateDetectedDeviceList(pCommand->Source);

                  break;
               }
               
               case CO_MESSAGE_BROADCAST_PP_MODE_REQ_E:
               {
                  CO_CommandData_t *pCommand = (CO_CommandData_t*)pMsg->Payload.PhyDataReq.Data;
                  last_cmd_source = MM_CMD_ConvertFromATCommandSource(pCommand->Source);
                  
                  send_pp_mode_broadcast_req();                  
                  
                  MM_CMD_SendATMessage(AT_RESPONSE_OK, last_cmd_source);
                  
                  break;
               }

               case CO_MESSAGE_BROADCAST_PP_FREQ_E:
               {
                  CO_CommandData_t *pCommandData = (CO_CommandData_t*)pMsg->Payload.PhyDataReq.Data;

                  if( pCommandData )
                  {                                          
                     broadcast_radio_frequency = pCommandData->Value;

                     last_cmd_source = MM_CMD_ConvertFromATCommandSource(pCommandData->Source);
                     
                     MM_CMD_SendATMessage(AT_RESPONSE_OK, last_cmd_source);
                  }                             
                  break;
               }

               
               case CO_MESSAGE_BROADCAST_PP_SYS_ID_E:
               {
                  CO_CommandData_t *pCommandData = (CO_CommandData_t*)pMsg->Payload.PhyDataReq.Data;

                  if( pCommandData )
                  {                                          
                     broadcast_system_id = pCommandData->Value;

                     last_cmd_source = MM_CMD_ConvertFromATCommandSource(pCommandData->Source);
                     
                     MM_CMD_SendATMessage(AT_RESPONSE_OK, last_cmd_source);
                  }                             
                  break;
               }
               
               case CO_MESSAGE_MAC_EVENT_E:
               {
                  if (false != gRxDoneFlag)
                  {
                     /* reset flag */
                     gRxDoneFlag = false;
                     
                     /* Process the received Packet */
                     MM_PPURxDoneOperation(gRxDonePayload, gRxDoneSize, gRxDoneRssi, gRxDoneSnr);

                     /* Set the antenna switch to receive */
                     GpioWrite(&AntRxNtx,1);
                     GpioWrite(&AntTxNrx,0);
   
                     /* Re-Start the radio reception */
                     Radio.Rx();
                  }
                  break;
               }

               case CO_MESSAGE_TIMER_EVENT_E:
               {
                  /* wake from sleep mode to kick the watchdog */
                  uint16_t next_wakeup = (uint16_t)(LPTIM_ReadCounter(&hlptim1) + (PPU_TIMER_TICK_INTERVAL_S*LPTIM_TICKS_PER_SEC));
                  HAL_LPTIM_Set_CMP(&hlptim1, next_wakeup);
                  time_counter_sec++; /* keep track of time we are in PP mode*/
                  
                  for (uint32_t idx = 0; idx < PPU_MAX_SUPPORTED_RBU; idx++)
                  {
                     if (rbu_devices_list[idx].state != RBU_NOT_VALID)
                     {
                        if ( (time_counter_sec - rbu_devices_list[idx].last_seen_time) > PPU_HEARTBEAT_RX_TIMEOUT_S)
                        {
                           rbu_devices_list[idx].state = RBU_NOT_VALID;
                           char serial_no_str[32];
                           MM_ATDecodeSerialNumber(rbu_devices_list[idx].serno, serial_no_str);
                           CO_PRINT_B_1(DBG_INFO, "PPU: Remove device %s\r\n", serial_no_str);
                        }
                        break;
                     }
                  }
                     
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
* MM_PPUAppTimerISR
*
* This function is called in the context of the interrupt from the LPTIM timer.
*
* @param - void
*
* @return - void

*/
void MM_PPUAppTimerISR(void)
{
   osStatus osStat = osErrorOS;
   CO_Message_t *pCmdReq = NULL;
   
   /* trigger operation in task */
   pCmdReq = osPoolAlloc(AppPool);
   if (NULL != pCmdReq)
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
* generateCommand
* Generate an AT command message and send it on the LoRa 
*
* @param - pointer to the comand data
*
* @return - SUCCESS_E

*/
ErrorCode_t generateCommand(const CO_CommandData_t * const CommandData)
{
   uint8_t buffer[LORA_PP_MODE_PACKET_LENGTH];

   PP_CommandMessage_t *command = (PP_CommandMessage_t *)buffer;
            
   /* The command is addressed to a remote node.  Queue it for transmission */
   /* populate command */
   command->frame_type    = APP_MSG_TYPE_COMMAND_E;
   command->unit_serno_lo = CommandData->UnitSerno & 0x00FFFFFF;
   command->unit_serno_hi = (CommandData->UnitSerno >>24) & 0x000000FF;
   command->TransactionID = CommandData->TransactionID;
   command->ParameterType = CommandData->CommandType;
   command->P1            = CommandData->Parameter1;
   command->P2            = CommandData->Parameter2;
   command->Value         = CommandData->Value;
   command->OperationType = CommandData->ReadWrite;
   
   /* Keep track of this command */
   last_unit_serno     = CommandData->UnitSerno;
   last_transaction_id = CommandData->TransactionID;
   last_parameter_type = CommandData->CommandType;
   
   /* wake up radio */
   Radio.Standby();
   
   /* Switch to the Peer to Peer Frequency */
   Radio.SetChannel( LORA_PP_MODE_FREQUENCY );
   
   /* Reconfigure the Transmission settings */
   Radio.SetTxConfig( MODEM_LORA, ppu_radio_power, 0, LORA_PP_BANDWIDTH,
                      LORA_PP_SPREADING_FACTOR, LORA_PP_CODINGRATE,
                      LORA_PP_PREAMBLE_LENGTH, LORA_PP_FIX_LENGTH_PAYLOAD_ON,
                      LORA_PP_INITIALISE_CRC, 0, 0, LORA_PP_IQ_INVERSION_ON, 3000 );

   /* Send LoRa packet */
   send_radio_packet(buffer, LORA_PP_MODE_PACKET_LENGTH);

   return SUCCESS_E;
}

/*************************************************************************************/
/**
* PPU_IsDeviceActive
* state handler to generate a command message
*
* @param - pointer to event data
*
* @return - void
*/
bool PPU_IsDeviceActive(void)
{
   return !(Radio.GetStatus() == RF_IDLE);
}

/*************************************************************************************/
/**
* PPU_GetFrequency
*  Read the broadcast Radio Frequency
*
* @param - None
*
* @return - Frequency
*/
uint32_t PPU_GetFrequency(void)
{
   return broadcast_radio_frequency;
}

/*************************************************************************************/
/**
* PPU_GetSystemId
*  Read the broadcast Radio System Identifier
*
* @param - None
*
* @return - System Identifier
*/
uint32_t PPU_GetSystemId(void)
{
   return broadcast_system_id;
}
