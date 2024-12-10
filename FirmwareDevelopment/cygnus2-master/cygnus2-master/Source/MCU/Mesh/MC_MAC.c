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
*  File         : MC_MAC.c
*
*  Description  : Implementation of the MAC layer of the mesh protocol
*
*                 HKD-16-0015-D_A37 Mesh Protocol Design
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "MC_MAC.h"
#include "lptim.h"
#include "CO_Message.h"
#include "CO_Defines.h"
#include "utilities.h"
#include "CFG_Device_cfg.h"
#include "DM_SerialPort.h"
#include "DM_LED.h"
#include "DM_CRC.h"
#include "board.h"
#include "MC_ChanHopSeqGenPublic.h"
#include "MC_SessionManagement.h"
#include "MC_MeshFormAndHeal.h"
#include "MC_TestMode.h"
#include "MC_Encryption.h"
#include "DM_NVM.h"
#include "MC_MacConfiguration.h"
#include "MC_TDM_SlotManagement.h"
#include "MC_TDM.h"
#include "MC_SyncAlgorithm.h"
#include "MM_Main.h"
#include "MC_StateManagement.h"
#include "MC_MacQueues.h"



/* Private Types
*************************************************************************************/



/* Private Functions Prototypes
*************************************************************************************/
static void MC_MAC_TimerOperationRxContinuous(void);
static void MC_MAC_PacketReceiveContinuous(void);

static void MC_MAC_IterateLowPowerCycle(void);

#ifdef SHOW_ALL_HOPPING_CHANNELS
void PrintArray(void);
#endif

bool gTxInProgress = false;
bool gCrcFailureDetected = false;

/* Global Variables
*************************************************************************************/
extern osPoolId MeshPool;
extern osMessageQId(MeshQ);
extern osMessageQId(DCHQ);
extern osMessageQId(RACHPQ);
extern osMessageQId(RACHSQ);
extern osMessageQId(ACKQ);
extern osMessageQId(DLCCHPQ);
extern osMessageQId(DLCCHSQ);
extern const uint32_t frequencyChannels865[];
extern uint16_t gRadioSettlingTime;
extern bool gAtModeOverTheAir;

#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
extern uint16_t lora_standby_time;
extern uint16_t lora_program_start_time;
extern uint16_t lora_program_end_time;
extern uint16_t lora_tx_time;
extern uint16_t lora_rx_time;
#endif

LoraParameters_t MC_MAC_LoraParameters;
static MC_MAC_TestMode_t gTestMode = MC_MAC_TEST_MODE_OFF_E;
static MC_MAC_ReceiveMode_t gReceiveMode = MC_MAC_NOT_RECEIVING_E;

#ifdef LOW_POWER_SLEEP_ENABLE
static uint32_t SleepPhaseCounter = 0;                 // Used to coordinate the on/off periods when an RBU is waiting for a mesh to come online.
#endif
bool gMC_MAC_MeshStateIsActive = false;
bool gChannelHoppingEnabled = false;
MC_MAC_Mode_t gMode = MC_MAC_MODE_RX_CONTINUOUS_E;
int16_t rach_tx_rx_offset;
uint16_t gTxTime;
char rx_string[256];
char bytes[128];

/* CHANNEL HOPPING SEQUENCE GENERATION
*************************************************************************************/

// IMPORTANT - IF DCH_SEQ_LEN IS NOT AN EXACT 2^N THEN PLEASE SEE ITS USE IN FUNCTION MC_MAC_CalcTDMIndices(), AS IT WILL NEED RECODING.
//#define DCH_SEQ_LEN            (16)  
#define BYTES_IN_SYSTEM_ID     (4)   // Number of bytes used for the system ID
#define MIN_CHAN_INTERVAL      (4)   // Minimum number of channels separation to give at least 1.0 MHz hops

static uint8_t    aSystemIDByteArray[BYTES_IN_SYSTEM_ID] = { 0 };
Channel_t  aRachDlcchChanHopSeq[MAX_RACH_DLCCH_SEQ_LEN] = { 0 };
Channel_t  aDchChanHopSeq[DCH_SEQ_LEN] = { 0 };
static uint8_t aSeedArray[NUM_HOPPING_CHANS][BYTES_IN_SYSTEM_ID] = { {12,34,56,78},{90,12,34,56},{78,90,12,34},{88,41,16,77},{34,56,78,90},
                                                                     {98,76,54,32},{91,98,76,54},{32,45,98,71},{87,59,63,23},{76,54,91,47}};

static uint32_t gAtMessageLength = MAX_OTA_AT_COMMAND_LENGTH;
                                                                        
#define TIMER_TICKS_IN_1_MILLISECOND 17u
// Rx Continuous sleep duration (1 second)
#define RX_CONTINUOUS_SLEEP_DURATION      16393   // 1s in LPTIM ticks

/*************************************************************************************/
/**
* MC_MAC_Init
* MAC initialisation function
*
* @param - const bool isMaster
* @param - const uint32_t systemId.  The System ID.
* @param - const uint32_t address.  The RBU's address.
*
* @return - ErrorCode_t ErrorCode

*/
ErrorCode_t MC_MAC_Init(const bool isMaster, const uint32_t systemId, const uint32_t address)
{
   ErrorCode_t ErrorCode = SUCCESS_E;
   uint16_t dlcchHoppingSeqLength = MC_DlcchHoppingSequenceLength();
   uint32_t baseFrequency = MC_GetBaseFrequencyChannel();
   
   ErrorCode = MC_MACQ_Initialise();
   CO_ASSERT_RET_MSG(SUCCESS_E == ErrorCode, ERR_INIT_FAIL_E, "ERROR - Initialise MAC Queues");
   
   ErrorCode = MC_TDM_Init();
   CO_ASSERT_RET_MSG(SUCCESS_E == ErrorCode, ERR_INIT_FAIL_E, "ERROR - Initialise TDM");
 
   //default to 865MHz
   MC_MAC_LoraParameters.LoraModulationBandwidth = LORA_BANDWIDTH_868;
   MC_MAC_LoraParameters.InitialFrequencyChannelIdx = baseFrequency;
   MC_MAC_LoraParameters.CurrentFrequencyChannelIdx = baseFrequency;
   MC_MAC_LoraParameters.LoraSpreadingFactor = LORA_SPREADING_FACTOR;
   MC_MAC_LoraParameters.LoraCodingRate = LORA_CODINGRATE;
   MC_MAC_LoraParameters.LoraLowTxPower  = MC_GetTxPowerLow();
   MC_MAC_LoraParameters.LoraHighTxPower = MC_GetTxPowerHigh();
   
   if (false == isMaster)
   {
      MC_MAC_StartRxContinuousMode();
   }
   
//   // Convert the System ID into a byte array
//   for (uint8_t i = 0; i < BYTES_IN_SYSTEM_ID; i++)
//   {
//      #define BYTE_MASK 0x000000FF  // 0xFF is the mask of the least significant byte
//      uint8_t nextByte = (systemId >> (BITS_PER_BYTE * i)) & BYTE_MASK;
//      aSystemIDByteArray[i] = nextByte;
//   }   
//   // Seed the random number generator (which is used by function MC_GenChanHopSeq)
//   PRBSGeneratorSeed_t PRBSGeneratorSeed = { BYTES_IN_SYSTEM_ID, aSystemIDByteArray };
   
#ifdef SHOW_ALL_HOPPING_CHANNELS
   PrintArray();
#endif
   
   //CYG2-1191 randomise hopping seq on base frequency only
   aSystemIDByteArray[0] = aSeedArray[baseFrequency][0];
   aSystemIDByteArray[1] = aSeedArray[baseFrequency][1];
   aSystemIDByteArray[2] = aSeedArray[baseFrequency][2];
   aSystemIDByteArray[3] = aSeedArray[baseFrequency][3];
   PRBSGeneratorSeed_t PRBSGeneratorSeed = { BYTES_IN_SYSTEM_ID, aSystemIDByteArray };
   
   
   ErrorCode = MC_GetRandNumber(NULL, &PRBSGeneratorSeed);
   CO_ERROR_CHECK_RET(SUCCESS_E == ErrorCode, ErrorCode);
   
   // Initialise the channel hopping sequence arrays: one for DCH, and one for RACH & DLCCH
   ErrorCode =  MC_GenChanHopSeq(aDchChanHopSeq, DCH_SEQ_LEN, NUM_HOPPING_CHANS, MIN_CHAN_INTERVAL);
   CO_ERROR_CHECK_RET(SUCCESS_E == ErrorCode, ErrorCode);
   
   ErrorCode = MC_GenChanHopSeq(aRachDlcchChanHopSeq, dlcchHoppingSeqLength, NUM_HOPPING_CHANS, MIN_CHAN_INTERVAL);
   CO_ERROR_CHECK_RET(SUCCESS_E == ErrorCode, ErrorCode);
   
   

   #ifdef ENABLE_CHANNEL_HOPPING
   uint16_t initialBestChannel = 0;
   ErrorCode =  MC_SelectInitialChannel(aDchChanHopSeq, DCH_SEQ_LEN, NUM_HOPPING_CHANS, &initialBestChannel);
   CO_ERROR_CHECK_RET(SUCCESS_E == ErrorCode, ErrorCode);
   ErrorCode =  MC_RotateHoppingSequence(aDchChanHopSeq, DCH_SEQ_LEN, NUM_HOPPING_CHANS, initialBestChannel, baseFrequency);
   CO_ERROR_CHECK_RET(SUCCESS_E == ErrorCode, ErrorCode);
   #endif
   
   #if 1
   CO_PRINT_B_0(DBG_INFO_E, "\r\nRACH:DLCCH Hopping Sequence:\r\n");
   for (uint8_t i = 0; i < dlcchHoppingSeqLength; i++)
   {
      CO_PRINT_B_1(DBG_NOPREFIX_E, "%d,", aRachDlcchChanHopSeq[i]);
   }   

   CO_PRINT_B_0(DBG_NOPREFIX_E, "\r\nDCH Hopping Sequence:\r\n");
   for (uint8_t i = 0; i < DCH_SEQ_LEN; i++)
   {
      CO_PRINT_B_1(DBG_INFO_E, "%d,", aDchChanHopSeq[i]);
   }
   CO_PRINT_B_0(DBG_NOPREFIX_E, "\r\n");
   #endif
   
   
   //Re-seed the random number generator to make it unique for each node
   uint32_t DeviceSeed = (address & 0xFFFF) + ((address & 0xFFFF) << 16);
   PRBSGeneratorSeed.NumBytes = sizeof(uint32_t);
   PRBSGeneratorSeed.pSeedByteArray = (uint8_t*)&DeviceSeed;
   ErrorCode = MC_GetRandNumber(NULL, &PRBSGeneratorSeed);
   CO_ERROR_CHECK_RET(SUCCESS_E == ErrorCode, ErrorCode);
   
   gCrcFailureDetected = false;
   
   return ErrorCode;
}

/*************************************************************************************/
/**
* MC_MAC_IsaTrackingNodeDchSlot
* Function to determine if the current slot is a DCH slot allocated to a tracking node
*
* @param - void
*
* @return - bool isTrackingNodeDchSlot
*/
bool MC_MAC_IsaTrackingNodeDchSlot(void)
{
   bool isTrackingNodeDchSlot = false;
   
   if ( MC_TDM_IsTrackingDCHSlot() ) 
   {
      isTrackingNodeDchSlot = true;
   }
   
   return isTrackingNodeDchSlot;
}



/*************************************************************************************/
/**
* MC_MAC_StartRxContinuousMode
* Function to request the MAC move to Rx Continuous mode
*
* The timer interrupt is disabled and the MC_MAC_Operation does not run
*
* @param - void
*
* @return - void

*/
void MC_MAC_StartRxContinuousMode(void)
{
   uint32_t currentTime = 0;
   
   gCrcFailureDetected = false;
   
   /* disable interrupt in case it is running already */
   HAL_LPTIM_Disable_CMPM_Interrupt(&hlptim1);

   gMode = MC_MAC_MODE_RX_CONTINUOUS_E;
   
   /* Get the current time */
   currentTime = LPTIM_ReadCounter(&hlptim1);

   MC_MAC_PacketReceiveContinuous();
   /* wake up at offset from current time */
   uint32_t NextActivateTime = SUM_WRAP(currentTime, RX_CONTINUOUS_SLEEP_DURATION, LPTIM_MAX);
   /* Set the low power timer comparator and enable the low power time interrupt */
   HAL_LPTIM_Set_CMP(&hlptim1, NextActivateTime);
   HAL_LPTIM_Enable_CMPM_Interrupt(&hlptim1);

   CO_PRINT_B_0(DBG_INFO_E, "Start Rx Continuous mode\r\n");
}


/*************************************************************************************/
/**
* MC_MAC_TimerOperation(void)
*
* This function runs in the context of the MAC task but it is triggered each time the
* timer interrupt occurs. It is triggered by a semaphore from the ISR.
*
* The function is run in TDM Sync Master or TDM Sync Slave modes or the Rx Continuous mode.
* It is not run in the Idle mode.
*
* The function calls handler functions depending on the current mode of the MAC
*
* @param - void
*
* @return - void
*/
void MC_MAC_TimerOperation(void)
{
   // check if MAC is in Test mode, RxContinuous mode or TDM mode.
   if (MC_MAC_MODE_TDM_SYNC_TEST_E == gMode)
   {
      DoTestModeNetworkMonitor();
   }
   else if (MC_MAC_MODE_RX_CONTINUOUS_E == gMode)
   {
      MC_MAC_TimerOperationRxContinuous();
   }
   else
   {
      MC_TDM_TimerOperationTdm();
   }
}

/*************************************************************************************/
/**
* MC_MAC_TimerOperationRxContinuous(void)
*
* This function runs in the context of the MAC task but it is triggered each time the
* timer interrupt occurs. It is triggered by a semaphore from the ISR.
*
* The function is run in Rx Continuous mode only.  It mostly puts the LoRa modem into sleep mode, 
* waking it periodically to enable RxContinuous mode.
*
* It sets the next wakeup time using a fixed offset from the current time.
*
* @param - void
*
* @return - void
*/
static void MC_MAC_TimerOperationRxContinuous(void)
{
   uint32_t currentTime = 0;

   /* Get the current time */
   currentTime = LPTIM_ReadCounter(&hlptim1);
   /* wake up at offset from current time */
   uint32_t NextActivateTime = SUM_WRAP(currentTime, RX_CONTINUOUS_SLEEP_DURATION, LPTIM_MAX);
   /* Set the low power timer comparator and enable the low power time interrupt */
   HAL_LPTIM_Set_CMP(&hlptim1, NextActivateTime);
   // Iterate the power-saving cycle of RxContinuous and Sleep, applicable prior to the first heartbeat being detected.
   MC_MAC_IterateLowPowerCycle();
}

/*************************************************************************************/
/**
* MC_MAC_TxDoneOperation
*
* This function runs in the context of the MAC task but it is triggered by the TxDone
* interrupt from the SX1272 chip.
*
*
* @param - void
*
* @return - void
*/
void MC_MAC_TxDoneOperation(void)
{
   gTxInProgress = false;
//   uint16_t timeNow = HAL_LPTIM_ReadCounter(&hlptim1);

//   uint16_t txTime = timeNow - gTxTime;
   
   if ( PPU_STATE_NOT_ACTIVE_E == MC_GetPpuMode() || (PPU_STATE_REQUESTED_E == MC_GetPpuMode()) )
   {
      //CO_PRINT_B_0(DBG_INFO_E,"Nppu\r\n");
   // put radio to sleep
#ifdef ALLOW_RADIO_SLEEP
   Radio.Sleep();
#else
   Radio.Standby();
#endif
}
   else 
   {
      //CO_PRINT_B_1(DBG_INFO_E,"Ppu t=%d\r\n", txTime);

      MC_MAC_ConfigureRxForPpu();
   }
}


/*************************************************************************************/
/**
* MC_MAC_RxDoneOperation
*
* This function runs in the context of the MAC task but it is triggered by the RxDone
* interrupt from the SX1272 chip.
*
*
* @param - const uint8_t *payload - pointer to payload read from sx1272
* @param - const uint16_t size - length of payload
* @param - const int16_t rssi - received signal strength indication
* @param - const int8_t snr - received signal to noise ratio
* @param - const uint16_t mcuRtcValue - timer value when rxDone interrupt was received
* @param - const uint8_t freqChanIdx - frequency channel index value when rxDone interrupt was received
* @param - const int32_t frequencyDeviation - frequency deviation reported by the radio hardware
*
* @return - void
*/
void MC_MAC_RxDoneOperation(const uint8_t * const payload, const uint16_t size, const int16_t rssi, const int8_t snr, const uint16_t mcuRtcValue, const uint8_t freqChanIdx, const int32_t frequencyDeviation)
{
   static uint8_t rxBuffer[RX_BUFFER_SIZE];
   // Test for NULL pointer
   CO_ASSERT_VOID(NULL != payload);
   
   osStatus osStat = osErrorOS;
   CO_Message_t *pPhyDataInd = NULL;
   uint32_t lSize = 0;
   
   memset(rxBuffer, 0, RX_BUFFER_SIZE);
   memcpy(rxBuffer, payload, size);
   
   // put radio to sleep if not in RxContinuous mode
   if (MC_MAC_RX_CONTINUOUS_E != gReceiveMode)
   {
#ifdef ALLOW_RADIO_SLEEP
      Radio.Sleep();
#else
      Radio.Standby();
#endif
   }
   
#ifdef USE_LORA_CHIP_CRC
   if ( false == gCrcFailureDetected )
   {
      /* create PhyDataInd message and put into mesh queue */
      pPhyDataInd = ALLOCMESHPOOL;
      if (pPhyDataInd)
      {
         pPhyDataInd->Type = CO_MESSAGE_PHY_DATA_IND_E;
         lSize = MIN(size, PHY_DATA_IND_LENGTH_MAX);
         memcpy(&pPhyDataInd->Payload.PhyDataInd.Data, payload, lSize);
         pPhyDataInd->Payload.PhyDataInd.Size = lSize;
         pPhyDataInd->Payload.PhyDataInd.RSSI = rssi;
         pPhyDataInd->Payload.PhyDataInd.SNR = snr;
         pPhyDataInd->Payload.PhyDataInd.timer = mcuRtcValue;
         pPhyDataInd->Payload.PhyDataInd.Expectedtimer = gCurrentSlotRefTime;
         pPhyDataInd->Payload.PhyDataInd.freqChanIdx = freqChanIdx;
         pPhyDataInd->Payload.PhyDataInd.slotIdxInShortframe = MC_MAC_GetCurrentSlotInShortframeIdx();
         pPhyDataInd->Payload.PhyDataInd.slotIdxInLongframe = MC_MAC_GetCurrentSlotInLongframeIdx();
         pPhyDataInd->Payload.PhyDataInd.slotIdxInSuperframe = MC_MAC_GetCurrentSlotInSuperframeIdx();
         pPhyDataInd->Payload.PhyDataInd.isInTDMSyncSlaveMode = (gMode == MC_MAC_MODE_TDM_SYNC_SLAVE_E);
         osStat = osMessagePut(MeshQ, (uint32_t)pPhyDataInd, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            FREEMESHPOOL(pPhyDataInd);
            CO_PRINT_A_0(DBG_ERROR_E, "No queue space for received message\r\n");
         }
      }
      else 
      {
         CO_PRINT_A_0(DBG_ERROR_E, "Failed to allocate pool space for received message\r\n");
      }
   }
#else /* USE_LORA_CHIP_CRC */
   ErrorCode_t status = ERR_DATA_INTEGRITY_CHECK_E;

   /* Sanity check of the packet length */
   if ((2u < size) )//&& (PHY_DATA_IND_LENGTH_MAX >= size))
   {
//      MC_TDM_Index_t current_slot;
//      MC_TDM_GetCurrentSlot( &current_slot );
//      CO_PRINT_B_3(DBG_ERROR_E, "Rd [%d], f=%d (%d)\r\n", current_slot.SlotInSuperframe, freqChanIdx, current_slot.HoppingIndexRACH);
//      CO_PRINT_B_2(DBG_INFO_E, "RxDone : size %d, channel %d\r\n", size, freqChanIdx);

      /* Calculate the CRC on the payload */
      uint16_t expected_crc;
      uint16_t actual_crc;
      
#ifdef PPU_EMBEDDED_CHECKSUM
      if ( (PPU_STATE_NOT_ACTIVE_E == MC_GetPpuMode()) || (PPU_STATE_REQUESTED_E == MC_GetPpuMode()) )
      {
         /* Remove the CRC bytes */
         lSize = size - 2u;
         /* Fetch the CRC value from the received Packet */
         actual_crc = rxBuffer[lSize] << 8;
         actual_crc += rxBuffer[lSize+1];
      }
      else
      {
         lSize = size;
         //PPU Mode.  The messages have the CRC embedded in the header.
         actual_crc = (uint16_t)rxBuffer[PPU_CHECKSUM_LOW] + ((uint16_t)rxBuffer[PPU_CHECKSUM_HIGH] << 8);
         //Zero the CRC bytes in the original message before calculating CRC
         rxBuffer[PPU_CHECKSUM_LOW] = 0;
         rxBuffer[PPU_CHECKSUM_HIGH] = 0;
      }
         
#else
      /* Remove the CRC bytes */
      lSize = size - 2u;
      /* Fetch the CRC value from the received Packet */
      actual_crc = payload[lSize];
      actual_crc += (payload[lSize+1] << 8);
#endif
      
      if (SUCCESS_E == DM_Crc16bCalculate8bDataWidth((uint8_t*)rxBuffer, lSize, &expected_crc, 1u)) 
      {
         
         /* Don't propagate messages with failed crc, unless we are in test mode for which
            we pass on everything that we receive */
         if ( (actual_crc == expected_crc) || (MC_MAC_TEST_MODE_OFF_E != MC_GetTestMode()) )
         {
            /* create PhyDataInd message and put into mesh queue */
            pPhyDataInd = ALLOCMESHPOOL;
            if (pPhyDataInd)
            {
               if ( PPU_STATE_NOT_ACTIVE_E == MC_GetPpuMode() || (PPU_STATE_REQUESTED_E == MC_GetPpuMode()) )
               {
               pPhyDataInd->Type = CO_MESSAGE_PHY_DATA_IND_E;
               }
               else 
               {
                  pPhyDataInd->Type = CO_MESSAGE_PPU_COMMAND_E;
               }
               memcpy(&pPhyDataInd->Payload.PhyDataInd.Data, payload, lSize);
               pPhyDataInd->Payload.PhyDataInd.Size = lSize;
               pPhyDataInd->Payload.PhyDataInd.RSSI = rssi;
               pPhyDataInd->Payload.PhyDataInd.SNR = snr;
               pPhyDataInd->Payload.PhyDataInd.timer = mcuRtcValue;
               pPhyDataInd->Payload.PhyDataInd.freqChanIdx = freqChanIdx;
               pPhyDataInd->Payload.PhyDataInd.freqDeviation = frequencyDeviation;
               MC_TDM_Index_t slot;
               MC_TDM_GetCurrentSlot( &slot );
               pPhyDataInd->Payload.PhyDataInd.Expectedtimer = slot.ExpectedRxDoneTime;
               pPhyDataInd->Payload.PhyDataInd.slotIdxInShortframe = slot.SlotIndex;
               pPhyDataInd->Payload.PhyDataInd.slotIdxInLongframe = slot.SlotIndexInLongFrame;
               pPhyDataInd->Payload.PhyDataInd.slotIdxInSuperframe = slot.SlotInSuperframe;
               pPhyDataInd->Payload.PhyDataInd.isInTDMSyncSlaveMode = (gMode == MC_MAC_MODE_TDM_SYNC_SLAVE_E);
               
               osStat = osMessagePut(MeshQ, (uint32_t)pPhyDataInd, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  FREEMESHPOOL(pPhyDataInd);
                  CO_PRINT_A_0(DBG_ERROR_E, "No queue space for received message\r\n");
               }
               else
               {
                  /* Message posted successfully */
                  status = SUCCESS_E;
                  //CO_PRINT_B_0(DBG_INFO_E, "Msg put on MeshQ\r\n");
               }
               
               
#ifdef REPORT_RXD_MESSAGES_RAW_BINARY
               
               strcpy(rx_string, "RXD ");
               uint32_t str_size = size;
               if ( 60 < str_size )
               {
                  str_size = 60;
            }
               for (uint8_t i = 0; i < str_size; i++)
               {
                  sprintf(bytes, "%02X", payload[i]);
                  strcat(rx_string, bytes);
               }
               strcat(rx_string, "\r\n");
               CO_PRINT_B_0(DBG_INFO_E, rx_string);
#endif

            }
            else
            {
               CO_PRINT_A_0(DBG_ERROR_E, "Failed to allocate pool space for received message\r\n");
            }
         }
#ifdef OUTPUT_CRC_FAIL_INFO
         else
         {
            strcpy(rx_string, "MCU CRC= ");
            uint32_t str_size = size;
            if ( 60 < str_size )
            {
               str_size = 60;
            }
            for (uint8_t i = 0; i < str_size; i++)
            {
               sprintf(bytes, "%02X", payload[i]);
               strcat(rx_string, bytes);
            }

            sprintf(bytes, "\n\r\t Exp CRC: 0x%04X ", expected_crc);
            strcat(rx_string, bytes);
            
            sprintf(bytes, "\n\r\t Rx CRC : 0x%04X", actual_crc);
            strcat(rx_string, bytes);
            
            sprintf(bytes, "\n\r\t RSSI = %d \n\t SNR = %d \r\n size = %d\n\r", rssi,snr, size);
            strcat(rx_string, bytes);
            
            CO_PRINT_B_0(DBG_ERROR_E, rx_string);
         }
#endif
      }
      else 
      {
         CO_PRINT_B_0(DBG_ERROR_E, "FAILED crc function\r\n");
      }
   }
   else 
   {
      CO_PRINT_B_1(DBG_ERROR_E, "FAILED size check.  size=%d\r\n", size);
   }

   if (SUCCESS_E != status)
   {
      /* MAC error triggered */
      MC_MAC_RxTimeoutErrorOperation(CO_RXTE_ERROR_E);
   }
#endif  /* USE_LORA_CHIP_CRC */
   
//   CO_PRINT_B_0(DBG_INFO_E,"Rd\r\n");
}


/*************************************************************************************/
/**
* MC_MAC_TxTimeoutOperation(void)
*
* This function runs in the context of the MAC task but it is triggered by the TxTimeout
* interrupt from the SX1272 chip.
*
*
* @param - void
*
* @return - void
*/
void MC_MAC_TxTimeoutOperation(void)
{
   gTxInProgress = false;
   // put radio to sleep if not in RxContinuous mode
   if (MC_MAC_RX_CONTINUOUS_E != gReceiveMode)
   {
#ifdef ALLOW_RADIO_SLEEP
      Radio.Sleep();
#else
      Radio.Standby();
#endif
   }
}


/*************************************************************************************/
/**
* MC_MAC_RxTimeoutErrorOperation(const CO_RxTimeoutError_t rxTimeoutError)
*
* This function runs in the context of the MAC task but it is triggered by the RxTimeout
* interrupt or by the RxError interrupt from the SX1272 chip.
*
*
* @param - const CO_RxTimeoutError_t rxTimeoutError - tx timeout or rx error
*
* @return - void
*/
void MC_MAC_RxTimeoutErrorOperation(const CO_RxTimeoutError_t rxTimeoutError)
{
   bool ProcessSyncMessage = false;
   uint32_t time_delta;
   uint16_t slot_reference_time;
   slot_reference_time = MC_TDM_GetSlotReferenceTime();
   uint32_t time_now = HAL_LPTIM_ReadCounter(&hlptim1);
   
   // put radio to sleep if not in RxContinuous mode
   if (MC_MAC_RX_CONTINUOUS_E != gReceiveMode)
   {
#ifdef ALLOW_RADIO_SLEEP
      Radio.Sleep();
#else
      Radio.Standby();
#endif
   }
   
   if ( CO_RXTE_ERROR_E == rxTimeoutError )
   {
      gCrcFailureDetected = true;
   }
   
   //Determine whether the timeout was because nothing was received or because the message failed to decode
   if ( slot_reference_time > time_now )
   {
      time_delta = slot_reference_time - time_now;
   }
   else 
   {
      time_delta = time_now - slot_reference_time;
   }
   
   if ( TIMER_TICKS_IN_1_MILLISECOND > time_delta )
   {
      //The timeout occurred within 1ms of the expected Rx Done.  The message arrived but failed to decode.
      MC_SendFailedDecodeDataMessage();
   }
   else if ( (MC_MAC_TEST_MODE_OFF_E == gTestMode) || (MC_MAC_TEST_MODE_NETWORK_MONITOR_E == gTestMode) )
   {
      if ( MC_TDM_SLOT_TYPE_DCH_E == MC_TDM_GetCurrentSlotType() )
      {
         if ( MC_TDM_IsTrackingDCHSlot() || MC_TDM_IsReceivingDCHSlot() )
         {
            ProcessSyncMessage = true;
         }
         /* send a message to the Mesh Task so that it can handle the missing heartbeat */
         MC_SendMissedHeartbeat(rxTimeoutError, ProcessSyncMessage);
      }
   }
   else if ( MC_MAC_TEST_MODE_RECEIVE_E == gTestMode )
   {
      /* Cad timed out in test mode, listen again */
      Radio.StartCad();
   }
//   CO_PRINT_B_0(DBG_INFO_E,"Rt\r\n");
}


/*************************************************************************************/
/**
* MC_MAC_PacketRxPreProcessSingle
*
* Prepare the LoRa modem for RxSingle reception.*
* @param - const uint16_t symbTimeout - Number of symbols timeout for SX1272 receiver.
* @param - const uint16_t preambleLen - Premable length of the expected packet.
* @param - const uint8_t payloadLen - Length of the payload.
* @param - const bool useLoraCrc - Set to true if the packet should check the LoRa CRC
*
* @return - void
*/
void MC_MAC_PacketRxPreProcessSingle(const uint16_t symbTimeout, const uint16_t preambleLen, const uint8_t payloadLen, const bool useLoraCrc, const Channel_t frequencyChannelIdx)
{
#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
   lora_standby_time = HAL_LPTIM_ReadCounter(&hlptim1);
#endif
   
   // wake up radio
   Radio.Standby();
   
   // Set the frequency channel
   //CO_PRINT_B_1(DBG_INFO_E,"MC_MAC_PacketRxPreProcessSingle Frequency %d\r\n",frequencyChannelIdx);
   MC_MAC_HopRadioChannel( frequencyChannelIdx );
   
   // Set packet size
   Radio.Write( REG_LR_PAYLOADLENGTH, payloadLen );

   // Set PLL bandwidth to best value for reception
   //Radio.Write( REG_LR_PLL, REG_PLL_BW_75KHZ);  // In accordance with JIRA CYG2-827.
   //Radio.Write( REG_LR_PLL, REG_PLL_BW_150KHZ); 
   Radio.Write( REG_LR_PLL, REG_PLL_BW_300KHZ); 

#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
   lora_program_start_time = HAL_LPTIM_ReadCounter(&hlptim1);
#endif
   // Configure the LoRa modem for RxSingle mode operation
   Radio.SetRxConfig( MODEM_LORA, MC_MAC_LoraParameters.LoraModulationBandwidth, MC_MAC_LoraParameters.LoraSpreadingFactor,
                      MC_MAC_LoraParameters.LoraCodingRate, 0, preambleLen,
                      symbTimeout, LORA_FIX_LENGTH_PAYLOAD_ON,
                      payloadLen, useLoraCrc, 0, 0, LORA_IQ_INVERSION_ON, LORA_RX_SINGLE_MODE );

#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
   lora_program_end_time = HAL_LPTIM_ReadCounter(&hlptim1);
#endif

   //Reset previous CRC flag
   gCrcFailureDetected = false;

   // put radio to sleep
#ifdef ALLOW_RADIO_SLEEP
   Radio.Sleep();
#endif

   // Record that the LoRa modem is configured for RxSingle operation
   gReceiveMode = MC_MAC_RX_SINGLE_E;
}


/*************************************************************************************/
/**
* MC_MAC_PacketReceiveSingle
*
* Wrapper function for lora receive. This ensures that Radio.Rx is not called if 
* the lora modem is alread receiving.
*
* @param - void
*
* @return - void
*/
void MC_MAC_PacketReceiveSingle(void)
{
   if ( gTxInProgress )
   {
      uint16_t time_now = HAL_LPTIM_ReadCounter(&hlptim1);
      CO_PRINT_A_2(DBG_ERROR_E,"Detected Tx fail 2 tx_start=%d, now=%d\r\n", gTxTime, time_now);
      gTxInProgress = false;
   }
#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
   lora_rx_time = HAL_LPTIM_ReadCounter(&hlptim1);
#endif
   // Start the radio reception.
   Radio.Rx();  // This maps to the function SX1272SetRx in sx1272.c
#ifdef TXRX_TIMING_PULSE
      GpioWrite(&StatusLedBlue,1);
#endif
}

/*************************************************************************************/
/**
* MC_MAC_PacketReceiveContinuous(void)
*
* Prepare the LoRa modem for RxContinuous reception.
*
* @param - void
*
* @return - void
*/
static void MC_MAC_PacketReceiveContinuous(void)
{
   /* put radio into standby for config */
   Radio.Standby( );

   // Set PLL bandwidth to best value for reception
   //Radio.Write( REG_LR_PLL, REG_PLL_BW_75KHZ);  // In accordance with JIRA CYG2-827.
   //Radio.Write( REG_LR_PLL, REG_PLL_BW_150KHZ); 
   Radio.Write( REG_LR_PLL, REG_PLL_BW_300KHZ); 

   // Configure the LoRa modem for RxContinous mode operation
   // Note, LoRa CRC mode is the same as for DCH processing, as only the DCH is monitored during continouus mode.
   Radio.SetRxConfig( MODEM_LORA, MC_MAC_LoraParameters.LoraModulationBandwidth, MC_MAC_LoraParameters.LoraSpreadingFactor,
                      MC_MAC_LoraParameters.LoraCodingRate, 0, LORA_DCH_PREAMBLE_LENGTH,
                      LORA_DCH_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                      PHY_DCH_PACKET_SIZE, LORA_DCH_CRC, 0, 0, LORA_IQ_INVERSION_ON, LORA_RX_CONTINUOUS_MODE );
   
   // Record that the LoRa modem is configured for RxContinuous operation
   gReceiveMode = MC_MAC_RX_CONTINUOUS_E;
   
   gCrcFailureDetected = false;
   
   // Start the radio reception.
   Radio.Rx();  // This maps to the function SX1272SetRx in sx1272.c
}


/*************************************************************************************/
/**
* MC_MAC_PacketTxPreProcess()
*
* Prepare the LoRa modem for transmission.
*
* @param - uint8_t * const buffer - location of send buffer
* @param - const uint8_t size - size in bytes of packet to send
* @param - const uint16_t preambleLen - the length of the preamble in symbols
* @param - const bool useLoraCrc - Set to true if the packet should transmit the LoRa CRC
* @param - const Channel_t frequencyChannelIdx - the frequency channel index.
*
* @return - void
*/
void MC_MAC_PacketTxPreProcess(CO_Message_t * const pMessage, const uint8_t size, const uint16_t preambleLen, const bool useLoraCrc, const Channel_t frequencyChannelIdx)
{
   // Test for NULL pointer
   CO_ASSERT_VOID(NULL != pMessage);
   
   uint8_t* buffer = pMessage->Payload.PhyDataReq.Data;
   CO_ASSERT_VOID(NULL != buffer);

   if ( gTxInProgress )
   {
      uint16_t time_now = HAL_LPTIM_ReadCounter(&hlptim1);
      CO_PRINT_A_2(DBG_ERROR_E,"Detected Tx fail 3 tx_start=%d, now=%d\r\n", gTxTime, time_now);
      gTxInProgress = false;
   }
#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
   lora_standby_time = HAL_LPTIM_ReadCounter(&hlptim1);
#endif
   
   // wake up radio
   Radio.Standby();

//   CO_PRINT_B_1(DBG_INFO_E,"MC_MAC_PacketTxPreProcess Frequency %d\r\n",frequencyChannelIdx);
   //Set the frequency
   MC_MAC_HopRadioChannel(frequencyChannelIdx);
     
   // Set PLL bandwidth to best value for transmission.
   Radio.Write( REG_LR_PLL, REG_PLL_BW_300KHZ);  // In accordance with JIRA CYG2-827.
   //Radio.Write( REG_LR_PLL, REG_PLL_BW_150KHZ);
   
#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
   lora_program_start_time = HAL_LPTIM_ReadCounter(&hlptim1);
#endif

   // Configure modem for transmission
   Radio.SetTxConfig( MODEM_LORA, pMessage->Payload.PhyDataReq.txPower, 0, MC_MAC_LoraParameters.LoraModulationBandwidth,
                                   MC_MAC_LoraParameters.LoraSpreadingFactor, MC_MAC_LoraParameters.LoraCodingRate,
                                   preambleLen, LORA_FIX_LENGTH_PAYLOAD_ON,
                                   useLoraCrc, 0, 0, LORA_IQ_INVERSION_ON, LORA_TX_TIMEOUT );

#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
   lora_program_end_time = HAL_LPTIM_ReadCounter(&hlptim1);
#endif

#ifdef USE_LORA_CHIP_CRC
   // Note, the original Semtech code for this function ends with a call to SX1272SetTx(), but we have commented out
   // that function call and instead from call it MC_MAC_PacketTransmit().  This is because we want to program the LoRa modem
   // parameters in advance of sending the packet to minimise any timing jitter in actually sending the packet.
   // So, despite the function below being called Send, it doesn't in fact send the packet.
   Radio.Send(buffer, size);
#else
   
   //If we are in PPU mode, the message is embedded in the header, not tagged to the end
   if ( PPU_STATE_NOT_ACTIVE_E == MC_GetPpuMode() || (PPU_STATE_REQUESTED_E == MC_GetPpuMode()))
   {
   uint16_t calculated_crc;
   DM_Crc16bCalculate8bDataWidth((uint8_t*)buffer, size, &calculated_crc, 1u);
   buffer[size] = (calculated_crc&0xFF);
   buffer[size+1] = ((calculated_crc>>8)&0xFF);
   }
   else
   {
#ifdef PPU_EMBEDDED_CHECKSUM
      //make sure the checksum location is 0.
      buffer[PPU_CHECKSUM_LOW] = 0;
      buffer[PPU_CHECKSUM_HIGH] = 0;
      // calculate and store CRC
      uint16_t calculated_crc;
      DM_Crc16bCalculate8bDataWidth((uint8_t*)buffer, size, &calculated_crc, 1u);
      buffer[PPU_CHECKSUM_LOW] = (calculated_crc&0xFF);
      buffer[PPU_CHECKSUM_HIGH] = ((calculated_crc>>8)&0xFF);
#else
      uint16_t calculated_crc;
      DM_Crc16bCalculate8bDataWidth((uint8_t*)buffer, size, &calculated_crc, 1u);
      buffer[size] = (calculated_crc&0xFF);
      buffer[size+1] = ((calculated_crc>>8)&0xFF);
#endif
   }
   
////////////////////////////////////////////////////////////////
//               char wkup_string[128];
//               strcpy(wkup_string, "Tx sending = ");
//               char bytes[64];
//                              
//               for (uint8_t i = 0; i < (size+2); i++)
//               {
//                  sprintf(bytes, "%02X", buffer[i]);
//                  strcat(wkup_string, bytes);
//               }
//
//               sprintf(bytes, "\r\nsize=%d\r\n", sizeof(ATModeRequest_t));
//               strcat(wkup_string, bytes);
//               
//               
//               CO_PRINT_B_0(DBG_ERROR_E, wkup_string);
///////////////////////////////////////////////////////////////

   // Note, the original Semtech code for this function ends with a call to SX1272SetTx(), but we have commented out
   // that function call and instead from call it MC_MAC_PacketTransmit().  This is because we want to program the LoRa modem
   // parameters in advance of sending the packet to minimise any timing jitter in actually sending the packet.
   // So, despite the function below being called Send, it doesn't in fact send the packet.
   Radio.Send(buffer, size+2);
#endif
   // Put radio to sleep
#ifdef ALLOW_RADIO_SLEEP
   //Radio.Sleep();
#else
   Radio.Standby();
#endif
   // Record that the LoRa modem is not configured for reception
   gReceiveMode = MC_MAC_NOT_RECEIVING_E;
   
}

/*************************************************************************************/
/**
* MC_MAC_PacketTransmit(uint8_t *buffer, uint8_t size)
*
* Wrapper function for lora receive. This ensures that Radio.Rx is not called if 
* the lora modem is alread receiving.
*
* @param - void
*
* @return - void
*/
#ifdef TXRX_TIMING_PULSE
void MC_MAC_PacketTransmit(bool pulse)
#else
void MC_MAC_PacketTransmit(void)
#endif
{
#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
   lora_tx_time = HAL_LPTIM_ReadCounter(&hlptim1);
#endif

   // SX1272SetTx(timeout = 0) was formerly called at the end of function SX1272Send (called using 
   // Radio.Send), but we have commented that call out and instead call it from here.
   // This was done because we want to do the pre-processing afforded by Radio.Send at an earlier
   // time, but want to defer the actual transmission (triggered by SX1272SetTx) here instead.
   // We have also modified the function SX1272SetTx() to set ANT1_SEL and ANT2_SEL as output bits.
   SX1272SetTx(0);
   
   gTxTime = HAL_LPTIM_ReadCounter(&hlptim1);
   gTxInProgress = true;

#ifdef TXRX_TIMING_PULSE
//   uint32_t time_start = HAL_LPTIM_ReadCounter(&hlptim1);
   if ( pulse )
   {
      GpioWrite(&StatusLedRed,1);
   }
//   uint32_t time_stop = HAL_LPTIM_ReadCounter(&hlptim1);
//   uint32_t delta = SUBTRACT_WRAP(time_stop, time_start, LPTIM_MAX);
#endif
   
#ifdef TXRX_TIMING_PULSE
//   CO_PRINT_B_1(DBG_INFO_E,"t%d\r\n",delta);
#endif
}

///*************************************************************************************/
///**
//* void MC_MAC_GetSlotTypeFromSlotNumber( const uint16_t slotIndex )
//*
//* Returns the TDM slot type for a given slot index.
//*
//* @param - uint16_t slotIndex.  the short frame index of the slot. 
//*
//* @return - The type of slot, or MC_MAC_SLOT_TYPE_UNUSED_E if slotIndex is out of range.
//*/
//MC_MAC_SlotType_t MC_MAC_GetSlotTypeFromSlotNumber( const uint16_t slotIndex )
//{
//   MC_MAC_SlotType_t slotType = MC_MAC_SLOT_TYPE_UNUSED_E;

//   if ( SLOTS_PER_SHORT_FRAME > slotIndex )
//   {
//      MC_TDM_SlotType_t slot_type = MC_TDM_GetSlotTypeDefault(slotIndex);
//      switch ( slot_type )
//      {
//         case MC_TDM_SLOT_TYPE_DLCCH_E:
//            slotType = MC_MAC_SLOT_TYPE_DLCCH_E;
//            break;
//         case MC_TDM_SLOT_TYPE_PRIMARY_RACH_E:
//            slotType = MC_MAC_SLOT_TYPE_PRIMARY_RACH_E;
//            break;
//         case MC_TDM_SLOT_TYPE_SECONDARY_RACH_E:
//            slotType = MC_MAC_SLOT_TYPE_SECONDARY_RACH_E;
//            break;
//         case MC_TDM_SLOT_TYPE_PRIMARY_ACK_E:
//            slotType = MC_MAC_SLOT_TYPE_PRIMARY_ACK_E;
//            break;
//         case MC_TDM_SLOT_TYPE_SECONDARY_ACK_E:
//            slotType = MC_MAC_SLOT_TYPE_SECONDARY_ACK_E;
//            break;
//         case MC_TDM_SLOT_TYPE_DCH_E:
//            slotType = MC_MAC_SLOT_TYPE_DCH_E;
//            break;
//         case MC_TDM_SLOT_TYPE_UNUSED_E:
//            slotType = MC_MAC_SLOT_TYPE_UNUSED_E;
//            break;
//         default:
//            slotType = MC_MAC_SLOT_TYPE_UNUSED_E;
//            break;
//      }
//   }

//   return slotType;
//}



/*************************************************************************************/
/**
* void MC_MAC_HopRadioChannel
*
* Function to hop the radio channel.
* A macro ENABLE_CHANNEL_HOPPING is included so that we can disable channel hopping,
* e.g. for measuring transmit power on a spectrum analyser.
*
* @param - const Channel_t HoppingChannelIdx
*
* @return - void
*/
void MC_MAC_HopRadioChannel(const Channel_t HoppingChannelIdx)
{
#ifdef ENABLE_CHANNEL_HOPPING
   uint8_t freqChannelIdx = 0;
   uint32_t rfFrequencyHz = 0;

   if ( gChannelHoppingEnabled )
   {
      // select the hopping channel index
      freqChannelIdx = HoppingChannelIdx;
   }
   else
   {
      // select the default channel index
      freqChannelIdx = MC_MAC_LoraParameters.InitialFrequencyChannelIdx;
   }
   
   // Convert the frequency channel index into frequency in Hz.
   rfFrequencyHz = MC_GetFrequencyForChannel(freqChannelIdx);

   // Program the LoRa modem with the desired frequency.
   Radio.SetChannel(rfFrequencyHz);
   
   
   // store the current frequency channel index
   MC_MAC_LoraParameters.CurrentFrequencyChannelIdx = freqChannelIdx;
   
#endif

   return;
}

/*************************************************************************************/
/**
* void DoTestModeRECEIVE
*
* Function to put the LORA radio into 'listen' mode.
*
* @param - none.
*
* @return - void
*/
void DoTestModeRECEIVE(void)
{
   /*Do Nothing*/
}


/*************************************************************************************/
/**
* void DoTestModeNetworkMonitor
*
* Function to put the LORA radio into 'network monitor' mode.
* In this mode the sync module provides the
* TDM timing and frequency hopping to enable this device to
* synchronise to a mesh network and report all of the messages
* that are sent between the network nodes.
* One constraint is that the initial frequency setting that is
* programmed into flash must match the setting in the target
* system.  This is because it is used to set the hopping
* sequence, so a different NVM frequency will generate a
* different hopping sequence.
*
* @param - none.
*
* @return - void
*/
void DoTestModeNetworkMonitor(void)
{
   MC_TDM_SetSlotsToDefaultType();
}


/*************************************************************************************/
/**
* void DoTestModeTRANSMIT
*
* Function to send a test message and schedule the next one.
*
* @param - none.
*
* @return - void
*/
void DoTestModeTRANSMIT(void)
{
   /*Do Nothing*/
}

/*************************************************************************************/
/**
* void DoTestModeSleep
*
* Function to reset the LPTIM timer for watchdog hold-off.
*
* @param - none.
*
* @return - void
*/
void MC_MAC_DoTestModeSleep(void)
{
   SetTestModeWakeup(TEST_MODE_WATCHDOG_PERIOD);
}

/*************************************************************************************/
/**
* void SetTestModeWakeup
*
* Function to set the next low power timer wake-up in test mode.
*
* @param - none.
*
* @return - void
*/
void SetTestModeWakeup(const uint16_t delayTime)
{
   uint32_t currentTime = 0;
   /* Get the current time */
   currentTime = LPTIM_ReadCounter(&hlptim1);
   /* wake up at offset from current time */
   uint32_t NextActivateTime = SUM_WRAP(currentTime, delayTime, LPTIM_MAX);
//   CO_PRINT_B_2(DBG_INFO_E,"n=%d, w=%d\r\n", currentTime, NextActivateTime);
   /* Set the low power timer comparator and enable the low power time interrupt */
   HAL_LPTIM_Set_CMP(&hlptim1, NextActivateTime);
}


/*************************************************************************************/
/**
* void MC_MAC_IterateLowPowerCycle
*
* Iterates between the low power and RxContinuous states during the period before the
* first heartbeat being detected.
* 1) Awake for first SYNC_STATE_INITIAL_AWAKE_DURATION seconds.
* 2) Then cycle between these two states:
*    - Sleep for SYNC_STATE_SLEEP_DURATION seconds.
*    - RxContinuous for SYNC_STATE_AWAKE_DURATION seconds.
*
* @param - void
*
* @return - void
*/
static void MC_MAC_IterateLowPowerCycle(void)
{
#ifdef LOW_POWER_SLEEP_ENABLE
   static uint32_t sleep_phase = 0;
   static uint32_t count = 0;
   static bool asleep = true;
   uint32_t initial_listen_duration = CFG_GetInitialListenPeriod();
   uint32_t phase2_duration = CFG_GetPhase2Period();
   uint32_t phase2_sleep_duration = CFG_GetPhase2SleepPeriod();
   uint32_t phase3_sleep_duration = CFG_GetPhase3SleepPeriod();

   //stop the count after it passes the phase three threshold so that it doesn't wrap around
   if ( SleepPhaseCounter <= (initial_listen_duration + phase2_duration) )
   {
      SleepPhaseCounter++;
   }
   
   if ( SleepPhaseCounter > (initial_listen_duration + phase2_duration) )
   {
      //we are in phase 3
      if ( 2 == sleep_phase )
      {
         sleep_phase = 3;
         count = phase3_sleep_duration;
         asleep = true;
#ifdef ALLOW_RADIO_SLEEP
         Radio.Sleep();
#else
         Radio.Standby();
#endif
         MC_STATE_SetDeviceState(STATE_SLEEP_MODE_E);
         gReceiveMode = MC_MAC_NOT_RECEIVING_E;  // Function MC_MAC_PacketReceiveContinuous sets this to MC_MAC_RX_CONTINUOUS_E.
         DM_LedPatternRequest(LED_MESH_SLEEP2_E);
         DM_LedPatternRemove(LED_MESH_SLEEP1_E);
         DM_LedPatternRemove(LED_MESH_IDLE_E);
         CO_PRINT_B_1(DBG_INFO_E,"LOW POWER START-UP Phase 3 sleep period = %d hours\r\n", (phase3_sleep_duration / SECONDS_IN_ONE_HOUR));
      }
      
      count--;
      if ( 0 == count )
      {
         if ( asleep )
         {
            count = LOW_POWER_AWAKE_DURATION;
            asleep = false;
            MC_STATE_SetDeviceState(STATE_IDLE_E);
            MC_MAC_PacketReceiveContinuous();       // Sets gReceiveMode to MC_MAC_RX_CONTINUOUS_E.
            DM_LedPatternRequest(LED_MESH_IDLE_E);
            CO_PRINT_B_1(DBG_INFO_E,"LOW POWER START-UP Phase 3 listen period = %d seconds\r\n", LOW_POWER_AWAKE_DURATION);
         }
         else 
         {
            count = phase3_sleep_duration;
            asleep = true;
#ifdef ALLOW_RADIO_SLEEP
            Radio.Sleep();
#else
            Radio.Standby();
#endif
            MC_STATE_SetDeviceState(STATE_SLEEP_MODE_E);
            gReceiveMode = MC_MAC_NOT_RECEIVING_E;  // Function MC_MAC_PacketReceiveContinuous sets this to MC_MAC_RX_CONTINUOUS_E.
            DM_LedPatternRemove(LED_MESH_IDLE_E);
            CO_PRINT_B_1(DBG_INFO_E,"LOW POWER START-UP Phase 3 sleep period = %d hours\r\n", (phase3_sleep_duration / SECONDS_IN_ONE_HOUR));
         }
      }
   }
   else if ( SleepPhaseCounter > initial_listen_duration )
   {
      //we are in phase 2
      if ( 1 == sleep_phase )
      {
         sleep_phase = 2;
         count = phase2_sleep_duration;
         asleep = true;
#ifdef ALLOW_RADIO_SLEEP
         Radio.Sleep();
#else
         Radio.Standby();
#endif
         MC_STATE_SetDeviceState(STATE_SLEEP_MODE_E);
         gReceiveMode = MC_MAC_NOT_RECEIVING_E;  // Function MC_MAC_PacketReceiveContinuous sets this to MC_MAC_RX_CONTINUOUS_E.
         DM_LedPatternRequest(LED_MESH_SLEEP1_E);
         DM_LedPatternRemove(LED_MESH_IDLE_E);
         CO_PRINT_B_1(DBG_INFO_E,"LOW POWER START-UP Phase 2 sleep period = %d hours\r\n", (phase2_sleep_duration / SECONDS_IN_ONE_HOUR));
      }
      
      count--;
      if ( 0 == count )
      {
         if ( asleep )
         {
            count = LOW_POWER_AWAKE_DURATION;
            asleep = false;
            MC_STATE_SetDeviceState(STATE_IDLE_E);
            MC_MAC_PacketReceiveContinuous();       // Sets gReceiveMode to MC_MAC_RX_CONTINUOUS_E.
            DM_LedPatternRequest(LED_MESH_IDLE_E);
            CO_PRINT_B_1(DBG_INFO_E,"LOW POWER START-UP Phase 2 listen period = %d seconds\r\n", LOW_POWER_AWAKE_DURATION);
         }
         else 
         {
            count = phase2_sleep_duration;
            asleep = true;
#ifdef ALLOW_RADIO_SLEEP
            Radio.Sleep();
#else
            Radio.Standby();
#endif
            MC_STATE_SetDeviceState(STATE_SLEEP_MODE_E);
            gReceiveMode = MC_MAC_NOT_RECEIVING_E;  // Function MC_MAC_PacketReceiveContinuous sets this to MC_MAC_RX_CONTINUOUS_E.
            DM_LedPatternRemove(LED_MESH_IDLE_E);
            CO_PRINT_B_1(DBG_INFO_E,"LOW POWER START-UP Phase 2 sleep period = %d hours\r\n", (phase2_sleep_duration / SECONDS_IN_ONE_HOUR));
         }
      }
   }
   else 
   {
      //we are in phase 1 ( initial listening period)
      if ( 0 == sleep_phase )
      {
         asleep = false;
         MC_STATE_SetDeviceState(STATE_IDLE_E);
         MC_MAC_PacketReceiveContinuous();       // Sets gReceiveMode to MC_MAC_RX_CONTINUOUS_E.
         CO_PRINT_B_1(DBG_INFO_E,"LOW POWER START-UP initial listening period = %d hours\r\n", (initial_listen_duration / SECONDS_IN_ONE_HOUR));
         sleep_phase = 1;
         DM_LedPatternRequest(LED_MESH_IDLE_E);
      }
   }
   
#endif
   return;
}


/*************************************************************************************/
/**
* void TellMACWhetherMeshStateIsActive
*
* Extern function to tell the MAC whether the mesh state is Active.  This is needed because
* if the mesh state is Active, signals sent over the P-RACH, acknowledgements to P-RACH signals,
* and DLCCH messages that are from the primary DLCCH queue, must all be transmitted at higher power.
*
* @param - const bool MeshStateIsActive
*
* @return - void
*/
void TellMACWhetherMeshStateIsActive(bool MeshStateIsActive)
{
   gMC_MAC_MeshStateIsActive = MeshStateIsActive;
   
   return;
}

/*************************************************************************************/
/**
* MC_SendMissedHeartbeat
*
* Function to create a 'missed heartbeat' message and despatch it to the Mesh Queue.
*
* @param - rxTimeoutError     Indicates whether the missed heartbeat was due to Rx timeout or packet error.
* @param - ProcessSyncMessage Set to true if the heartbeat slot was for a sync tracking node.
*
* @return - None.
*/
void MC_SendMissedHeartbeat(const CO_RxTimeoutError_t rxTimeoutError, const bool ProcessSyncMessage)
{
   CO_Message_t* pMsg;
   pMsg = ALLOCMESHPOOL;
   if (pMsg)
   {
      pMsg->Type = CO_MESSAGE_MAC_EVENT_E;
      MACEventMessage_t macEvent;
      macEvent.EventType = CO_MISSING_HEARTBEAT_E;
      macEvent.RxTimeoutError = rxTimeoutError;

      macEvent.ProcessSyncMessage = ProcessSyncMessage;
      MC_TDM_Index_t slot;
      MC_TDM_GetCurrentSlot( &slot );
      macEvent.SuperframeSlotIndex = slot.SlotInSuperframe;
      macEvent.LongFrameIndex = slot.LongFrameIndex;
      macEvent.ShortFrameIndex = slot.ShortFrameIndex;
      macEvent.SlotIndex = slot.SlotIndex;
//      CO_PRINT_B_0(DBG_INFO_E,"mhb\r\n");
      macEvent.McuCtr = LPTIM_ReadCounter(&hlptim1);  // Not needed by the algorithm, but useful for debugging.
      macEvent.SyncTimestampValid = false;  // Indicates that LoRa RxSingle timed-out or had a CRC error.
      macEvent.Frequency = aDchChanHopSeq[slot.HoppingIndexDCH];

      memcpy(pMsg->Payload.PhyDataInd.Data, &macEvent, sizeof(MACEventMessage_t));

      osStatus osStat = osMessagePut(MeshQ, (uint32_t)pMsg, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         FREEMESHPOOL(pMsg);
      }
   }
}

/*************************************************************************************/
/**
* MC_SendFailedDecodeDataMessage
*
* Function to create a 'corrupt data' message and despatch it to the Mesh Queue.
*
* @param  - None
*
* @return - None.
*/
void MC_SendFailedDecodeDataMessage(void)
{
   CO_Message_t* pMsg;
   pMsg = ALLOCMESHPOOL;
   if (pMsg)
   {
      pMsg->Type = CO_MESSAGE_MAC_EVENT_E;
      MACEventMessage_t macEvent;
      macEvent.EventType = CO_CORRUPTED_RX_MESSAGE_E;
      MC_TDM_Index_t slot;
      MC_TDM_GetCurrentSlot( &slot );
      macEvent.SuperframeSlotIndex = slot.SlotInSuperframe;
      macEvent.LongFrameIndex = slot.LongFrameIndex;
      macEvent.ShortFrameIndex = slot.ShortFrameIndex;
      macEvent.SlotIndex = slot.SlotIndex;
      macEvent.RxTimeoutError = CO_RXTE_CORRUPT_E;
      MC_TDM_SlotType_t slot_type = MC_TDM_GetSlotType( slot.SlotIndex );
      if ( MC_TDM_SLOT_TYPE_DCH_E == slot_type )
      {
         macEvent.Frequency = aDchChanHopSeq[slot.HoppingIndexDCH];
      }
      else 
      {
         macEvent.Frequency = aDchChanHopSeq[slot.HoppingIndexRACH];
      }

      // If a sync message must be processed, assign values to the three parameters that will be used.
      macEvent.ProcessSyncMessage = false;
      macEvent.McuCtr = LPTIM_ReadCounter(&hlptim1);  // Not needed by the algorithm, but useful for debugging.
      macEvent.SyncTimestampValid = false;  // Indicates that LoRa RxSingle timed-out or had a CRC error.

      memcpy(pMsg->Payload.PhyDataInd.Data, &macEvent, sizeof(MACEventMessage_t));

      osStatus osStat = osMessagePut(MeshQ, (uint32_t)pMsg, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         FREEMESHPOOL(pMsg);
      }
   }
}

/*************************************************************************************/
/**
* MC_MAC_SendAtCommand
*
* Function to send a message from the RACHS queue over the air.
* Only send if the TDM isn't running.
*
* @param  - None
*
* @return - None.
*/
void MC_MAC_SendAtMode( const ATModeRequest_t * const pRequest )
{
   bool send_ota = true;
   if ( pRequest )
   {
      if ( pRequest->Address == MC_GetNetworkAddress() )
      {
         //The message is for this unit
         send_ota = false;
      }
      else
      {
         //check if it's a local command for this unit
         uint32_t serial_number;
         if ( SUCCESS_E == DM_NVMRead(NV_UNIT_SERIAL_NO_E, &serial_number, sizeof(uint32_t)) )
         {
            if ( serial_number == pRequest->Address )
            {
               //The message is for this unit
               send_ota = false;
            }
         }
      }
      
      if ( send_ota )
      {

         // Set the frequency.
         uint32_t frequency = MC_MAC_LoraParameters.InitialFrequencyChannelIdx;//MC_GetBaseFrequencyChannel();

          //Wait until not transmitting, or 50ms timeout
         uint32_t count = 0;
         while ( gTxInProgress && (50 > count))
         {
            osDelay(1);
            count++;
         }
         
         //The receiving RBU is expecting an 11 byte heartbeat message on its base frequency.
         //We need to send an advance message to tell it to use the PPU freuency and AT message length OTA_AT_MESSAGE_LENGTH.
         
         CO_Message_t ppuMsg;
         memset(ppuMsg.Payload.PhyDataReq.Data, 0 , PHY_DATA_REQ_LENGTH_MAX);
         memcpy(ppuMsg.Payload.PhyDataReq.Data, pRequest, sizeof(ATModeRequest_t));
         
   CO_PRINT_B_3(DBG_INFO_E,"MC_MAC_SendAtMode - wakeUpMsg Frame=%x, address=%d, system=%d\r\n", pRequest->Frametype, pRequest->Address, pRequest->SystemID);
         
         MC_MAC_PacketTxPreProcess(&ppuMsg, DIV_ROUNDUP(HEARTBEAT_SIZE, BITS_PER_BYTE), LORA_RACH_PREAMBLE_LENGTH, LORA_RACH_CRC, (uint8_t)frequency);
         //Wake up the radio
         Radio.Standby();
#ifdef TXRX_TIMING_PULSE
         MC_MAC_PacketTransmit(false);
#else
         MC_MAC_PacketTransmit();
#endif
         
         //Set the base frequency to PPU frequency for future Tx and Rx
         MC_MAC_LoraParameters.InitialFrequencyChannelIdx = PPU_FREQUENCY_INDEX;
         gAtMessageLength = OTA_AT_MESSAGE_LENGTH;
         gAtModeOverTheAir = true;
      }
      else 
      {
         // local command
         MC_MAC_ProcessATRequest((uint8_t*) pRequest);
      }
   }
}

/*************************************************************************************/
/**
* MC_MAC_SendAtCommand
*
* Function to send a message from the RACHS queue over the air.
* Only send if the TDM isn't running.
*
* @param  - None
*
* @return - None.
*/
void MC_MAC_SendAtCommand( void )
{
    // Set the frequency.
   uint32_t frequency = MC_MAC_LoraParameters.InitialFrequencyChannelIdx;

   /* check for P-RACH message to send */
   if ( MC_MACQ_MessageCount(MAC_RACHS_Q_E) )
   {
      CO_Message_t* pMessage = ALLOCMESHPOOL;
      if ( pMessage )
      {
         if ( SUCCESS_E == MC_MACQ_Pop(MAC_RACHS_Q_E, pMessage) )
         {
            if ( pMessage )
            {
               //Wait until not transmitting, or 30ms timeout
               uint32_t count = 0;
               while ( gTxInProgress && (50 > count))
               {
                  osDelay(1);
                  count++;
               }
               
               //CO_PRINT_B_2(DBG_INFO_E,"MC_MAC_SendAtCommand - freq=%d size=%d\r\n", frequency, gAtMessageLength);
               
               /* send message */
               MC_MAC_PacketTxPreProcess(pMessage, gAtMessageLength, LORA_RACH_PREAMBLE_LENGTH, LORA_RACH_CRC, (uint8_t)frequency);
               //Wake up the radio
               Radio.Standby();
#ifdef TXRX_TIMING_PULSE
               MC_MAC_PacketTransmit(false);
#else
               MC_MAC_PacketTransmit();
#endif
               //Remove the message from the queue
               MC_MACQ_Discard(MAC_RACHS_Q_E);
               
                //Wait until not transmitting, or 50ms timeout
               count = 0;
               while ( gTxInProgress && (50 > count))
               {
                  osDelay(1);
                  count++;
               }
               
               //Wake up the radio
               Radio.Standby();
               
               //Set radio to receive
               Radio.SetRxConfig( MODEM_LORA, MC_MAC_LoraParameters.LoraModulationBandwidth, MC_MAC_LoraParameters.LoraSpreadingFactor,
                                  MC_MAC_LoraParameters.LoraCodingRate, 0, LORA_DCH_PREAMBLE_LENGTH,
                                  LORA_DCH_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                                  (gAtMessageLength+2), LORA_DCH_CRC, 0, 0, LORA_IQ_INVERSION_ON, LORA_RX_CONTINUOUS_MODE );

               // Record that the LoRa modem is configured for RxContinuous operation
               gReceiveMode = MC_MAC_RX_CONTINUOUS_E;
               // Start the radio reception.
               Radio.Rx();  // This maps to the function SX1272SetRx in sx1272.c

            }
         }
         else
         {
            CO_PRINT_B_0(DBG_INFO_E,"MC_MAC_SendAtCommand - failed to get message from queue\r\n");
         }
         //Always free pMessage
         FREEMESHPOOL(pMessage);
      }
      else
      {
         CO_PRINT_B_0(DBG_INFO_E,"MC_MAC_SendAtCommand - message alloc failed\r\n");
      }
   }
   else
   {
      CO_PRINT_B_0(DBG_INFO_E,"MC_MAC_SendAtCommand - No message found\r\n");
   }
}

/*************************************************************************************/
/**
* MC_MAC_ProcessATRequest
*
* Function to process a request to go into AT mode
*
* @param  - pMsg     the received AT prelim message containing the unit address 
*                    and the length of message to expect.
*
* @return - bool     True if the Radio was reconfigured
*/
bool MC_MAC_ProcessATRequest(const uint8_t* const pMsg)
{
   bool result = false;
   ATModeRequest_t at_request;
   bool address_match = false;
   
   if ( pMsg )
   {
      memcpy(&at_request, pMsg, sizeof(ATModeRequest_t));
      CO_PRINT_B_1(DBG_INFO_E,"ProcessATRequest address=%d\r\n", at_request.Address);
      //Check the address
      if ( MAX_NUMBER_OF_SYSTEM_NODES <= at_request.Address )
      {
         //the address is a serial number
         uint32_t serial_number;
         if ( SUCCESS_E == DM_NVMRead(NV_UNIT_SERIAL_NO_E, &serial_number, sizeof(uint32_t)) )
         {
            CO_PRINT_B_1(DBG_INFO_E,"local serno=%d\r\n", serial_number);
            if ( at_request.Address == serial_number )
            {
               address_match = true;
            }
         }
      }
      else 
      {
         //the address is a node ID
         uint16_t network_address = MC_GetNetworkAddress();
         if ( at_request.Address == network_address )
         {
            address_match = true;
         }
      }
      
      if ( address_match )
      {
         if ( AT_FRAME_TYPE_ENTER_AT_MODE_E == at_request.Frametype )
         {
            CO_PRINT_B_0(DBG_INFO_E,"Address MATCH\r\n");
            
            CO_PRINT_B_0(DBG_INFO_E,"AT mode ON\r\n");
            gAtMessageLength = OTA_AT_MESSAGE_LENGTH;
            
            // Convert the PPU frequency channel index into frequency in Hz.
            uint32_t rfFrequencyHz = MC_GetFrequencyForChannel(PPU_FREQUENCY_INDEX);
            // Program the LoRa modem with the desired frequency.
            Radio.SetChannel(rfFrequencyHz);
            CO_PRINT_B_2(DBG_INFO_E,"Radio Frequency %d, pkt len=%d\r\n",rfFrequencyHz, gAtMessageLength);
            
            MC_MAC_LoraParameters.InitialFrequencyChannelIdx = PPU_FREQUENCY_INDEX;
            
            //Wake up the radio
            Radio.Standby();
            
            Radio.SetRxConfig( MODEM_LORA, MC_MAC_LoraParameters.LoraModulationBandwidth, MC_MAC_LoraParameters.LoraSpreadingFactor,
                               MC_MAC_LoraParameters.LoraCodingRate, 0, LORA_DCH_PREAMBLE_LENGTH,
                               LORA_DCH_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                               (gAtMessageLength+2), LORA_DCH_CRC, 0, 0, LORA_IQ_INVERSION_ON, LORA_RX_CONTINUOUS_MODE );

            // Record that the LoRa modem is configured for RxContinuous operation
            gReceiveMode = MC_MAC_RX_CONTINUOUS_E;
            // Start the radio reception.
            Radio.Rx();  // This maps to the function SX1272SetRx in sx1272.c
            
            result = true;
         }
         else if (AT_FRAME_TYPE_ENTER_PPU_MODE_E == at_request.Frametype )
         {
            CO_Message_t* pMsg;
            pMsg = ALLOCMESHPOOL;
            if (pMsg)
            {
               pMsg->Type = CO_MESSAGE_MAC_EVENT_E;
               MACEventMessage_t macEvent;
               macEvent.EventType = CO_ENTER_PPU_OTA_MODE_E;

               memcpy(pMsg->Payload.PhyDataInd.Data, &macEvent, sizeof(MACEventMessage_t));

               osStatus osStat = osMessagePut(MeshQ, (uint32_t)pMsg, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(MeshPool, pMsg);
               }
            }
         }
      }
      else 
      {
         CO_PRINT_B_0(DBG_INFO_E,"Address NO MATCH\r\n");
      }
   }
   else
   {
      CO_PRINT_B_0(DBG_INFO_E,"MC_MAC_SendAtCommand - No message found\r\n");
   }
   
   return result;
}


/*************************************************************************************/
/**
* MC_MAC_ProcessPpuModeRequest
*
* Function to perform a PPU MOde operation
*
* @param  - request     The operation to perform.
*
* @return - bool     True if the operation succeded
*/
bool MC_MAC_ProcessPpuModeRequest(const uint32_t request)
{
   static uint8_t buffer[32];
   
   bool result = false;
   
   CO_PRINT_B_1(DBG_INFO_E,"MC_MAC_ProcessPpuModeRequest req=%d\r\n", request);
   
   if ( PPU_START_DISCONNECTED_MODE == request )
   {
      CO_PRINT_B_0(DBG_INFO_E,"Starting PPU Disconnected mode\r\n");
      //stop the TDM if it's running
      if ( MC_TDM_Running() )
      {
         MC_TDM_StopTDM();
      }
      //Set the Rx config for PPU messages.
      MC_MAC_ConfigureRxForPpu();
      
      result = true;
   }
   else if ( PPU_ANNOUNCEMENT_SEND == request )
   {
      CO_PRINT_B_0(DBG_INFO_E,"Sending PPU Announcement\r\n");
      //build the announcement message
      PPU_Message_t Announcement;
      Announcement.PpuAddress = ADDRESS_GLOBAL;
      Announcement.RbuAddress = MC_GetNetworkAddress(); //node ID
      Announcement.SystemID = MC_GetSystemID();
      Announcement.Command = PPU_ANNOUNCEMENT_MSG;
      Announcement.PacketLength = 23;
      uint32_t serial_number = MC_GetSerialNumber();
      uint32_t device_Combination = CFG_GetDeviceCombination();
      uint32_t firmware_version = CFG_GetFirmwareVersion();
      
      memset(buffer, 0, 32);
      buffer[0] = Announcement.PpuAddress & 0xff;
      buffer[1] = ((Announcement.PpuAddress >> 8) & 0xff);
      buffer[2] = Announcement.RbuAddress & 0xff;
      buffer[3] = ((Announcement.RbuAddress >> 8) & 0xff);
      buffer[4] = Announcement.SystemID & 0xff;
      buffer[5] = ((Announcement.SystemID >> 8) & 0xff);
      buffer[6] = ((Announcement.SystemID >> 16) & 0xff);
      buffer[7] = ((Announcement.SystemID >> 24) & 0xff);
#ifdef PPU_EMBEDDED_CHECKSUM
      buffer[10] = Announcement.PacketLength & 0xff;
      buffer[11] = ((Announcement.PacketLength >> 8) & 0xff);
      buffer[12] = Announcement.Command & 0xff;
      buffer[13] = ((Announcement.Command >> 8) & 0xff);
      buffer[14] = serial_number & 0xff;
      buffer[15] = ((serial_number >> 8) & 0xff);
      buffer[16] = ((serial_number >> 16) & 0xff);
      buffer[17] = ((serial_number >> 24) & 0xff);
      buffer[18] = firmware_version & 0xff;
      buffer[19] = ((firmware_version >> 8) & 0xff);
      buffer[20] = ((firmware_version >> 16) & 0xff);
      buffer[21] = ((firmware_version >> 24) & 0xff);
      buffer[22] = device_Combination & 0xff;
      //Calculate CRC
      uint16_t calculated_crc;
      DM_Crc16bCalculate8bDataWidth((uint8_t*)buffer, Announcement.PacketLength, &calculated_crc, 1u);
      buffer[8] = (calculated_crc & 0xFF);
      buffer[9] = ((calculated_crc >> 8) & 0xFF);

#else
      buffer[8] = Announcement.PacketLength & 0xff;
      buffer[9] = ((Announcement.PacketLength >> 8) & 0xff);
      buffer[10] = Announcement.Command & 0xff;
      buffer[11] = ((Announcement.Command >> 8) & 0xff);
      buffer[12] = serial_number & 0xff;
      buffer[13] = ((serial_number >> 8) & 0xff);
      buffer[14] = ((serial_number >> 16) & 0xff);
      buffer[15] = ((serial_number >> 24) & 0xff);
      buffer[16] = firmware_version & 0xff;
      buffer[17] = ((firmware_version >> 8) & 0xff);
      buffer[18] = ((firmware_version >> 16) & 0xff);
      buffer[19] = ((firmware_version >> 24) & 0xff);
      buffer[20] = device_Combination & 0xff;
      uint16_t calculated_crc;
      DM_Crc16bCalculate8bDataWidth((uint8_t*)buffer, (Announcement.PacketLength - 2), &calculated_crc, 1u);
      buffer[21] = (calculated_crc & 0xFF);
      buffer[22] = ((calculated_crc >> 8) & 0xFF);
#endif
      MC_MAC_ConfigureTxForPpu();
      
      // Note, the original Semtech code for this function ends with a call to SX1272SetTx(), but we have commented out
      // that function call and instead from call it MC_MAC_PacketTransmit().  This is because we want to program the LoRa modem
      // parameters in advance of sending the packet to minimise any timing jitter in actually sending the packet.
      // So, despite the function below being called Send, it doesn't in fact send the packet.
      Radio.Send(buffer, Announcement.PacketLength);
#ifdef TXRX_TIMING_PULSE
      MC_MAC_PacketTransmit(false);
#else
      MC_MAC_PacketTransmit();
#endif
      // Record that the LoRa modem is not configured for reception
      gReceiveMode = MC_MAC_NOT_RECEIVING_E;
      result = true;
   }
   
   return result;
}

/*************************************************************************************/
/**
* MC_MAC_SendPpuMessage
*
* Function to send a message from the RACHS queue over the air on the PPU frequency.
* Only send if the TDM isn't running.
*
* @param  - None
*
* @return - None.
*/
void MC_MAC_SendPpuMessage( void )
{
   /* check for P-RACH message to send */
   if ( MC_MACQ_MessageCount(MAC_RACHS_Q_E) )
   {
      CO_Message_t* pMessage = osPoolAlloc(MeshPool);
      if ( pMessage )
      {
         if ( SUCCESS_E == MC_MACQ_Pop(MAC_RACHS_Q_E, pMessage) )
         {
            if ( pMessage )
            {
               //The PPU is a bit slow switching over to Receive
               HAL_Delay(20);
               //Calculate CRC
               uint8_t* buffer = pMessage->Payload.PhyDataReq.Data;
               uint16_t calculated_crc;
#ifdef PPU_EMBEDDED_CHECKSUM
               uint32_t packet_length = pMessage->Payload.PhyDataReq.Size;
               DM_Crc16bCalculate8bDataWidth(buffer, packet_length, &calculated_crc, 1u);
               buffer[PPU_CHECKSUM_LOW] = (calculated_crc & 0xFF);
               buffer[PPU_CHECKSUM_HIGH] = ((calculated_crc >> 8) & 0xFF);
#else
               uint32_t packet_length = pMessage->Payload.PhyDataReq.Size - 2;
               DM_Crc16bCalculate8bDataWidth(buffer, packet_length, &calculated_crc, 1u);
               buffer[packet_length] = (calculated_crc & 0xFF);
               buffer[packet_length+1] = ((calculated_crc >> 8) & 0xFF);
#endif
               
               char crc_string[128];
               strcpy(crc_string, "Sending - ");
               char bytes[64];
                              
               for (uint8_t i = 0; i < pMessage->Payload.PhyDataReq.Size; i++)
               {
                  sprintf(bytes, "%02X", buffer[i]);
                  strcat(crc_string, bytes);
               }
               strcat(crc_string, "\r\n");
               CO_PRINT_B_0(DBG_INFO_E, crc_string);


               //Wait until not transmitting, or 30ms timeout
               uint32_t count = 0;
               while ( gTxInProgress && (50 > count))
               {
                  osDelay(1);
                  count++;
               }
               
               /* send message */
               MC_MAC_ConfigureTxForPpu();
               
               Radio.Send(pMessage->Payload.PhyDataReq.Data, pMessage->Payload.PhyDataReq.Size);
#ifdef TXRX_TIMING_PULSE
               MC_MAC_PacketTransmit(false);
#else
               MC_MAC_PacketTransmit();
#endif
               // Record that the LoRa modem is not configured for reception
               gReceiveMode = MC_MAC_NOT_RECEIVING_E;
               //Remove the message from the queue
               MC_MACQ_Discard(MAC_RACHS_Q_E);
            }
         }
         else
         {
            CO_PRINT_B_0(DBG_INFO_E,"MC_MAC_SendAtCommand - failed to get message from queue\r\n");
         }
      }
      else
      {
         CO_PRINT_B_0(DBG_INFO_E,"MC_MAC_SendAtCommand - message alloc failed\r\n");
      }
   }
   else
   {
      CO_PRINT_B_0(DBG_INFO_E,"MC_MAC_SendAtCommand - No message found\r\n");
   }
}


/*************************************************************************************/
/**
* MC_MAC_ConfigureTxForPpu
*
* Function to set the LoRa Transmitter to send a PPU message
*
* @param  - None.
*
* @return - None.
*/
void MC_MAC_ConfigureTxForPpu( void )
{
   /* put radio into standby for config */
   Radio.Standby( );
   // Convert the PPU frequency channel index into frequency in Hz.
   uint32_t rfFrequencyHz = MC_GetFrequencyForChannel(PPU_FREQUENCY_INDEX);
   // Program the LoRa modem with the desired frequency.
   Radio.SetChannel(rfFrequencyHz);
   Radio.Write( REG_LR_PLL, REG_PLL_BW_300KHZ);
   /* Reconfigure the transmit attributes */
   Radio.SetTxConfig( MODEM_LORA, TX_HIGH_OUTPUT_POWER, 0, LORA_PPU_BANDWIDTH,
                      LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                      LORA_DCH_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_OFF,
                      LORA_DCH_CRC, 0, 0, LORA_IQ_INVERSION_ON, PPU_LORA_TX_TIMEOUT );
}

/*************************************************************************************/
/**
* MC_MAC_ConfigureRxForPpu
*
* Function to set the LoRa receiver to expect a PPU message
*
* @param  - None.
*
* @return - None.
*/
void MC_MAC_ConfigureRxForPpu( void )
{
      /* put radio into standby for config */
      Radio.Standby( );
      // Convert the PPU frequency channel index into frequency in Hz.
      uint32_t rfFrequencyHz = MC_GetFrequencyForChannel(PPU_FREQUENCY_INDEX);
      // Program the LoRa modem with the desired frequency.
      Radio.SetChannel(rfFrequencyHz);
      CO_PRINT_B_1(DBG_INFO_E,"PPU Rx - Radio Frequency %d\r\n",rfFrequencyHz);
      
      MC_MAC_LoraParameters.CurrentFrequencyChannelIdx = PPU_FREQUENCY_INDEX;
      
      Radio.SetRxConfig( MODEM_LORA, LORA_PPU_BANDWIDTH, LORA_SPREADING_FACTOR,
                         LORA_CODINGRATE, 0, LORA_DCH_PREAMBLE_LENGTH,
                         LORA_DCH_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_OFF,
                         0xff, LORA_DCH_CRC, 0, 0, LORA_IQ_INVERSION_ON, LORA_RX_CONTINUOUS_MODE );
   
      // Record that the LoRa modem is configured for RxContinuous operation
      gReceiveMode = MC_MAC_RX_CONTINUOUS_E;
      // Start the radio reception.
      Radio.Rx();  // This maps to the function SX1272SetRx in sx1272.c
}

/*************************************************************************************/
/**
* MC_MAC_ConfigureRxForHeartbeat
*
* Function to set the LoRa receiver to expect a heartbeat
*
* @param  - None.
*
* @return - None.
*/
void MC_MAC_ConfigureRxForHeartbeat( void )
{
//   // Convert the base frequency channel index into frequency in Hz.
//   uint32_t rfFrequencyHz = MC_GetFrequencyForChannel(MC_MAC_LoraParameters.InitialFrequencyChannelIdx);
//   // Program the LoRa modem with the desired frequency.
//   Radio.SetChannel(rfFrequencyHz);
   
   MC_MAC_PacketReceiveContinuous();
}


#ifdef SHOW_ALL_HOPPING_CHANNELS
/*************************************************************************************/
/**
* PrintArray
*
* Debug function.  Prints the dch hopping sequence for each base frequency.
* Enabled by SHOW_ALL_HOPPING_CHANNELS in CO_Defines.h
* 
*
* @param - None.
*
* @return - None.
*/
void PrintArray(void)
{
   ErrorCode_t ErrorCode;
   uint32_t baseFrequency;
   uint16_t dlcchHoppingSeqLength = MC_DlcchHoppingSequenceLength();
   
   CO_PRINT_B_0(DBG_NOPREFIX_E, "\r\nDCH Hopping Sequence:\r\n");
   
   for (baseFrequency = 0; baseFrequency < NUM_HOPPING_CHANS; baseFrequency++)
   {
      //CYG2-1191 randomise hopping seq on base frequency only
      aSystemIDByteArray[0] = aSeedArray[baseFrequency][0];
      aSystemIDByteArray[1] = aSeedArray[baseFrequency][1];
      aSystemIDByteArray[2] = aSeedArray[baseFrequency][2];
      aSystemIDByteArray[3] = aSeedArray[baseFrequency][3];
      PRBSGeneratorSeed_t PRBSGeneratorSeed = { BYTES_IN_SYSTEM_ID, aSystemIDByteArray };
      
      
      ErrorCode = MC_GetRandNumber(NULL, &PRBSGeneratorSeed);
      
      // Initialise the channel hopping sequence arrays: one for DCH, and one for RACH & DLCCH
      ErrorCode =  MC_GenChanHopSeq(aDchChanHopSeq, DCH_SEQ_LEN, NUM_HOPPING_CHANS, MIN_CHAN_INTERVAL);
      
      ErrorCode = MC_GenChanHopSeq(aRachDlcchChanHopSeq, dlcchHoppingSeqLength, NUM_HOPPING_CHANS, MIN_CHAN_INTERVAL);
      
      

      #ifdef ENABLE_CHANNEL_HOPPING
      uint16_t initialBestChannel = 0;
      ErrorCode =  MC_SelectInitialChannel(aDchChanHopSeq, DCH_SEQ_LEN, NUM_HOPPING_CHANS, &initialBestChannel);
      ErrorCode =  MC_RotateHoppingSequence(aDchChanHopSeq, DCH_SEQ_LEN, NUM_HOPPING_CHANS, initialBestChannel, baseFrequency);
      #endif
      

//      CO_PRINT_B_0(DBG_INFO_E, "\r\nRACH:DLCCH Hopping Sequence:\r\n");
//      for (uint8_t i = 0; i < dlcchHoppingSeqLength; i++)
//      {
//         CO_PRINT_B_1(DBG_NOPREFIX_E, "%d,", aRachDlcchChanHopSeq[i]);
//      }   

      for (uint8_t i = 0; i < DCH_SEQ_LEN; i++)
      {
         CO_PRINT_B_1(DBG_INFO_E, "%d,", aDchChanHopSeq[i]);
      }
      CO_PRINT_B_0(DBG_NOPREFIX_E, "\r\n");
   }
   
}
#endif
