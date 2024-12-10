/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2020 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : MC_TDM.c
*
*  Description  : Implementation of the Time Division Multiplexing of the mesh protocol
*
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "lptim.h"
#include "CO_Defines.h"
#include "utilities.h"
#include "DM_SerialPort.h"
#include "DM_LED.h"
#include "board.h"
#include "SM_StateMachine.h"
#include "MC_SyncAlgorithm.h"
#include "MC_ChanHopSeqGenPublic.h"
#include "MC_SessionManagement.h"
#include "MC_MeshFormAndHeal.h"
#include "MC_TestMode.h"
#include "DM_NVM.h"
#include "MC_StateManagement.h"
#include "MC_MacConfiguration.h"
#include "MC_MAC.h"
#include "MC_MacQueues.h"
#ifdef USE_ENCPRYPTED_PAYLOAD
#include "MC_Encryption.h"
#endif
#include "MC_TDM.h"


/* Private macros
*************************************************************************************/


/* Private Function Prototypes
*************************************************************************************/


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

extern const uint32_t NUM_FREQ_CHANS;
extern bool gFreqLockAchieved;
extern Channel_t  aRachDlcchChanHopSeq[MAX_RACH_DLCCH_SEQ_LEN];
extern Channel_t  aDchChanHopSeq[DCH_SEQ_LEN];
extern MC_MAC_Mode_t gMode;
extern DM_BaseType_t gBaseType;
extern bool gMC_MAC_MeshStateIsActive;
extern bool gChannelHoppingEnabled;
extern bool gTxInProgress;
extern uint16_t gTxTime;
extern uint16_t gHeartbeatTxSlotNumber;
extern MC_TDM_Index_t gCurrentSlot;
extern MC_TDM_Index_t gNextSlot;
extern uint32_t gDLCCHReceiveSlot;
extern uint32_t gDLCCHTransmitSlot;
extern MC_TDM_DCHBehaviour_t gDchBehaviour[MAX_DEVICES_PER_SYSTEM];
extern MC_TDM_DLCCHBehaviour_t gDlcchBehaviour[TDM_NUMBER_OF_DLCCH_SLOTS];

#ifdef ENABLE_HEARTBEAT_TX_SKIP
extern uint32_t gNumberOfHeatbeatsToSkip;
#endif


/* Private Variables
*************************************************************************************/
static uint16_t gSyncReference;
static uint16_t gPreviousSyncReference;
static uint8_t gStateChangeLongFrameCount = 0;
static uint8_t gFreqHopLongFrameCount = 0;
static uint32_t gDLCCHReceiveFrequencyIndex = 0;
static uint32_t gDLCCHTransmitFrequencyIndex = 0;
//static uint16_t gNcuWakeupTime;
static bool gStopTDM = false;
static bool gTdmRunning = false;


/* Public Variables
*************************************************************************************/
bool gPrimaryAckExpected = false;          // Set to true if a primary ACK Rx is due pending an earlier RACH transmission.
bool gSecondaryAckExpected = false;        // Set to true if a secondary ACK Rx is due pending an earlier RACH transmission.
bool gDulchAckExpected = false;            // Set to true if a DULCH ACK Rx is due pending an earlier DULCH transmission.
uint8_t gNoOfPrimaryAcksInTxQueue = 0;     // The number of primary ACKs still pending transmission.
uint8_t gNoOfSecondaryAcksInTxQueue = 0; // The number of secondary ACKs still pending transmission.
uint8_t gPrimaryDLCCHMessageCount = 0;              //A count of how many messages are in the primary DLCCH Q
uint8_t gSecondaryDLCCHMessageCount = 0;              //A count of how many messages are in the secondary DLCCH Q
#ifdef ENABLE_TDM_CALIBRATION
int16_t gDchRxDoneLatency = (int16_t)TDM_DCH_RX_DONE_LATENCY;
int16_t gDchTxOffset = (int16_t)TDM_DCH_TX_OFFSET;
int16_t gDchRxOffset = (int16_t)TDM_DCH_RX_OFFSET;

int16_t gRachTxOffsetDownlink = (int16_t)TDM_RACH_TX_OFFSET_DOWNLINK;
int16_t gRachTxOffsetUplink = (int16_t)TDM_RACH_TX_OFFSET_UPLINK;
int16_t gRachCadOffset = (int16_t)TDM_RACH_CAD_OFFSET;

int16_t gDlcchTxOffset = (int16_t)TDM_DLCCH_TX_OFFSET;
int16_t gDlcchCadOffset = (int16_t)TDM_DLCCH_CAD_OFFSET;

int16_t gAckTxOffsetUplink = (int16_t)TDM_ACK_TX_OFFSET;
int16_t gAckCadOffset = (int16_t)TDM_ACK_RX_OFFSET;

int16_t gDchOffset = (int16_t)TDM_DCH_MESSAGE_RXDONE_OFFSET;

#endif


uint16_t gSyncCorrection = 0;
int16_t gFrameSyncOffset = 0;
bool gSyncUpdated = false;
bool gSyncPulse = false;
double gSlotLength = (double)SLOT_DURATION;

#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
uint16_t wake_up_time = 0;
uint16_t first_wake_up_time = 0;
uint16_t lora_standby_time = 0;
uint16_t lora_program_start_time = 0;
uint16_t lora_program_end_time = 0;
uint16_t lora_tx_time = 0;
uint16_t lora_rx_time = 0;
#endif

/* Private Functions
*************************************************************************************/
bool MC_TDM_ProcessDLCCHSlot(void);
bool MC_TDM_ProcessPRACHSlot(void);
bool MC_TDM_ProcessSRACHSlot(void);
bool MC_TDM_ProcessDULCHSlot(void);
bool MC_TDM_ProcessACKSlot(void);

bool MC_TDM_ProcessDCHSlot(void);
bool MC_TDM_ProcessUnusedSlot(void);
#ifdef TXRX_TIMING_PULSE
void MC_TDM_RadioSendReceive(const bool pulse);
#else
void MC_TDM_RadioSendReceive(void);
#endif
void MC_TDM_Advance(void);
void MC_TDM_GenerateHeartbeat(void);
void MC_TDM_SendNewLongFrameToMesh(void);
uint32_t MC_TDM_GetDlcchFrequencyForNode(const uint16_t node_id);
bool MC_TDM_ValidateWakeupTime(const uint16_t wake_up_time);
static void (* StateChangeNotifyCallbackFn)(const uint8_t) = NULL;
static void MC_TDM_RequestDulchMessage(void);
static uint32_t MC_TDM_GetNextHbSlotFrequency(void);

#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
static void MC_TDM_ShowFirstWakeupTimes(void);
static void MC_TDM_ShowSecondWakeupTimes(void);
#endif

/*****************************************************************************/
/**
*  Function:      MC_TDM_Init
*  Description:   Initialise the TDM properties
*
*  @param   None
*  @return  SUCCESS_E or error code.
*            
*****************************************************************************/
ErrorCode_t MC_TDM_Init(void)
{
   //Set the slot assignments
   MC_TDM_SlotInit();
   
   gTdmRunning = false;
   
   return SUCCESS_E;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_StartTDMMasterMode
*  Description:   Start the TDM for the NCU
*
*  @param   None
*  @return  SUCCESS_E or error code.
*            
*****************************************************************************/
void MC_TDM_StartTDMMasterMode(void)
{
   /* Get the current slot time */
   uint16_t time_now = (uint16_t)LPTIM_ReadCounter(&hlptim1);
   
   //Set the timer to wake up after 1 slot duration to give the initial heartbeat time to be generated
   uint16_t next_wake_up = (uint16_t)SUM_WRAP(time_now, SLOT_DURATION, LPTIM_MAX);
   uint32_t ticks_per_long_frame = MC_CyclesPerLongFrame();
   gSyncReference = next_wake_up;
   //Set the next long frame sync
   int32_t mod_ticks_per_long_frame = ticks_per_long_frame % LPTIM_MAX;
   gSyncCorrection = (uint16_t)SUM_WRAP(gSyncReference, mod_ticks_per_long_frame, LPTIM_MAX);
   
   
   //Set up the slot record for the next wake up
   gCurrentSlot.Action = MC_TDM_ACTION_PREPROCESS_E;
   gCurrentSlot.HoppingIndexDCH = 0;
   gCurrentSlot.HoppingIndexRACH = 0;
   gCurrentSlot.LongFrameIndex = 0;
   gCurrentSlot.ShortFrameIndex = 0;
   gCurrentSlot.SlotIndex = FIRST_DCH_SLOT_IN_SHORT_FRAME;
   gCurrentSlot.SlotIndexInLongFrame = FIRST_DCH_SLOT_IN_SHORT_FRAME;
   gCurrentSlot.SlotInSuperframe = FIRST_DCH_SLOT_IN_SHORT_FRAME;
   gCurrentSlot.WakeupTime = next_wake_up;
   gCurrentSlot.ExpectedRxDoneTime = SUM_WRAP(next_wake_up, TDM_DCH_MESSAGE_RXDONE_OFFSET, LPTIM_MAX);
   gCurrentSlot.Action = MC_TDM_ACTION_PREPROCESS_E;
   gCurrentSlot.pMessage = NULL;
   
   gNextSlot = gCurrentSlot;
   
   MC_TDM_GenerateHeartbeat();
   
   gMode = MC_MAC_MODE_TDM_SYNC_MASTER_E;
   
   gStopTDM = false;
   gTdmRunning = true;
   
   HAL_LPTIM_Disable_CMPM_Interrupt(&hlptim1);
   HAL_LPTIM_Set_CMP(&hlptim1, next_wake_up);
   HAL_LPTIM_Enable_CMPM_Interrupt(&hlptim1);
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_StartTDMMasterMode
*  Description:   Start the TDM for the RBU
*
*  @param   None
*  @return  SUCCESS_E or error code.
*            
*****************************************************************************/
void MC_TDM_StartTDMSlaveMode(const uint32_t slotInSuperframeIdx, const uint16_t slotRefTime)
{
   uint32_t slots_per_super_frame = MC_SlotsPerSuperFrame();
   //Parameter range checks
   CO_ASSERT_VOID_MSG((slotInSuperframeIdx < slots_per_super_frame), "MC_TDM_StartTDMSlaveMode : Slots per superframe too high");
   
   uint32_t slots_per_long_frame = MC_SlotsPerLongFrame();
   uint32_t ticks_per_long_frame = MC_CyclesPerLongFrame();
   //Set the current long frame
   gCurrentSlot.SlotIndex = slotInSuperframeIdx % SLOTS_PER_SHORT_FRAME;
   gCurrentSlot.SlotIndexInLongFrame = slotInSuperframeIdx % slots_per_long_frame;

   //Calculate the synch reference time (start of long frame slot 0)
   int32_t slot_in_long_frame_offset_time = (gCurrentSlot.SlotIndexInLongFrame * SLOT_DURATION);
   int32_t sync_node_offset = slot_in_long_frame_offset_time + TDM_DCH_MESSAGE_RXDONE_OFFSET;
   gSyncReference = (uint16_t)SUBTRACT_WRAP((int32_t)slotRefTime, sync_node_offset, LPTIM_MAX);
   //Set the next long frame sync
   int32_t mod_ticks_per_long_frame = ticks_per_long_frame % LPTIM_MAX;
   gSyncCorrection = (uint16_t)SUM_WRAP(gSyncReference, mod_ticks_per_long_frame, LPTIM_MAX);

   CO_PRINT_B_4(DBG_INFO_E,"LFslot=%d, ref=%d, sync=%d, correct=%d\r\n", gCurrentSlot.SlotIndexInLongFrame, slotRefTime,gSyncReference,gSyncCorrection);
   
   //Set the next slot properties
   gNextSlot.Action = MC_TDM_ACTION_PREPROCESS_E;
   gNextSlot.SlotInSuperframe = (slotInSuperframeIdx + 1) % slots_per_super_frame;
   gNextSlot.LongFrameIndex = gNextSlot.SlotInSuperframe / slots_per_long_frame;
   gNextSlot.ShortFrameIndex = (gNextSlot.SlotInSuperframe - (gNextSlot.LongFrameIndex * slots_per_long_frame)) / SLOTS_PER_SHORT_FRAME; 
   gNextSlot.SlotIndex = gNextSlot.SlotInSuperframe % SLOTS_PER_SHORT_FRAME;
   gNextSlot.SlotIndexInLongFrame = gNextSlot.SlotInSuperframe % slots_per_long_frame;
   gNextSlot.pMessage = NULL;
   
   if ( gNextSlot.SlotIndexInLongFrame == gHeartbeatTxSlotNumber )
   {
      MC_TDM_GenerateHeartbeat();
   }
   
   int32_t slot_offset_in_long_frame = (gNextSlot.SlotIndexInLongFrame * SLOT_DURATION);
   gNextSlot.WakeupTime = (uint16_t)SUM_WRAP((int32_t)gSyncReference, (int32_t)slot_offset_in_long_frame, LPTIM_MAX);

   //Identify the current heartbeat slot and configure for the next wake up
   if ( MC_TDM_SLOT_TYPE_DCH_E == MC_TDM_GetSlotTypeDefault(gCurrentSlot.SlotIndex) )
   {
      //configure for DLCCH slot next
      gNextSlot.HoppingIndexRACH = (gNextSlot.ShortFrameIndex) % MC_DlcchHoppingSequenceLength();
      gNextSlot.ExpectedRxDoneTime = (uint16_t)SUBTRACT_WRAP((int32_t)gNextSlot.WakeupTime, (int32_t)TDM_DATA_MESSAGE_RXDONE_OFFSET, LPTIM_MAX);
   }
   else 
   {
      Error_Handler("Sync started on non-DCH slot");
   }

   gMode = MC_MAC_MODE_TDM_SYNC_SLAVE_E;
   
   gStopTDM = false;
   gTdmRunning = true;

   /* Set the low power timer comparator and enable the low power time interrupt */
   HAL_LPTIM_Disable_CMPM_Interrupt(&hlptim1);
   HAL_LPTIM_Set_CMP(&hlptim1, gNextSlot.WakeupTime);
   HAL_LPTIM_Enable_CMPM_Interrupt(&hlptim1);

//   CO_PRINT_B_4(DBG_INFO_E,"slot=%d, ref=%d, sync=%d, next=%d\r\n", slotInSuperframeIdx, slotRefTime, gSyncReference, gNextSlot.WakeupTime);

}

/*****************************************************************************/
/**
*  Function:      MC_TDM_StopTDM
*  Description:   Set a flag that will instruct the TDM to stop after the current slot
*
*  @param   None
*  @return  None.
*            
*****************************************************************************/
void MC_TDM_StopTDM(void)
{
   gStopTDM = true;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_Running
*  Description:   Return TRUE if the TDM is running
*
*  @param   None
*  @return  bool : True if the TDM is running.
*            
*****************************************************************************/
bool MC_TDM_Running(void)
{
   return gTdmRunning;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_EnableChannelHopping
*  Description:   Start channel hopping
*
*  @param   None
*  @return  None.
*            
*****************************************************************************/
void MC_TDM_EnableChannelHopping(void)
{
#ifdef ENABLE_CHANNEL_HOPPING
   CO_PRINT_B_0(DBG_INFO_E,"Channel Hopping Enabled\r\n");
   gChannelHoppingEnabled = true;
#endif
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_TimerOperationTdm
*  Description:   Run the TDM
*
*  @param   None
*  @return  None.
*            
*****************************************************************************/
void MC_TDM_TimerOperationTdm(void)
{
   static bool advance = true;
   
   if ( advance )
   {
      uint16_t time_now = HAL_LPTIM_ReadCounter(&hlptim1);
      gCurrentSlot = gNextSlot;
      //CO_PRINT_B_2(DBG_INFO_E,"s=%d,w=%d\r\n", gCurrentSlot.SlotIndex, time_now);
   }

#ifdef TXRX_TIMING_PULSE
   if ( gSyncPulse )
   {
#ifdef TXRX_TIMING_FRAME_SYNC
         GpioWrite(&StatusLedGreen,1);
         CO_InlineDelayTicks(2);
         GpioWrite(&StatusLedGreen,0);
#endif //TXRX_TIMING_FRAME_SYNC
   }
#ifdef SHOW_SLOT_WAKEUPS
   else
   {
      GpioWrite(&StatusLedGreen,1);
      CO_InlineDelayTicks(1);
      GpioWrite(&StatusLedGreen,0);
   }
#endif //SHOW_SLOT_WAKEUPS
#endif //TXRX_TIMING_PULSE
   
   HAL_LPTIM_Disable_CMPM_Interrupt(&hlptim1);
   
//   CO_PRINT_B_5(DBG_INFO_E,"s=%d sl=%d ss=%d, SF=%d LF=%d\r\n", gCurrentSlot.SlotIndex, gCurrentSlot.SlotIndexInLongFrame, gCurrentSlot.SlotInSuperframe, gCurrentSlot.ShortFrameIndex, gCurrentSlot.LongFrameIndex);
   
   MC_TDM_SlotType_t slot_type = MC_TDM_GetCurrentSlotType( );
   
   switch ( slot_type )
   {
      case MC_TDM_SLOT_TYPE_DLCCH_E:
      {
         uint32_t dlcch_index = gCurrentSlot.SlotIndex / 2;
         if ( MC_TDM_DLCCH_SLEEP_E != gDlcchBehaviour[dlcch_index] )
         {
            advance = MC_TDM_ProcessDLCCHSlot();
         }
         else 
         {
            advance = true;
         }
      }
         break;
      case MC_TDM_SLOT_TYPE_PRIMARY_RACH_E:
         advance = MC_TDM_ProcessPRACHSlot();
         break;
      case MC_TDM_SLOT_TYPE_SECONDARY_RACH_E:
         advance = MC_TDM_ProcessSRACHSlot();
         break;
      case MC_TDM_SLOT_TYPE_DULCH_E:
         advance = MC_TDM_ProcessDULCHSlot();
         break;
      case MC_TDM_SLOT_TYPE_PRIMARY_ACK_E://intentional drop-through
      case MC_TDM_SLOT_TYPE_SECONDARY_ACK_E:
      case MC_TDM_SLOT_TYPE_DULCH_ACK_E:
         advance = MC_TDM_ProcessACKSlot();
         break;
      case MC_TDM_SLOT_TYPE_DCH_E:
         advance = MC_TDM_ProcessDCHSlot();
         break;
      case MC_TDM_SLOT_TYPE_UNUSED_E:
         advance = MC_TDM_ProcessUnusedSlot();
         break;
      default:
         advance = MC_TDM_ProcessUnusedSlot();
         break;
   }
   
   if ( gStopTDM )
   {
      //Stopping the TDM.  Cancel any LED activity
      DM_LedPatterns_t current_led_pattern = DM_LedGetCurrentPattern();
      while ( LED_PATTERN_NOT_DEFINED_E != current_led_pattern )
      {
         DM_LedPatternRemove(current_led_pattern);
         current_led_pattern = DM_LedGetCurrentPattern();
      }
      
      //Reset the TDM
      MC_TDM_Init();
      
      //Set the device state and mesh state
      if ( BASE_NCU_E == gBaseType )
      {
         //NCU
         MC_STATE_SetDeviceState(STATE_IDLE_E);
         MC_STATE_SetMeshState(STATE_IDLE_E);
      }
      else
      {
         //RBUs
         MC_STATE_SetDeviceState(STATE_CONFIG_SYNC_E);
         MC_STATE_SetMeshState(STATE_IDLE_E);
         //Start continuous Rx
         MC_MAC_StartRxContinuousMode();
      }
      
   }
   else 
   {
      //advance to the next expected slot
      if ( advance )
      {
         MC_TDM_Advance();
      }
      
      HAL_LPTIM_Enable_CMPM_Interrupt(&hlptim1);

      
   #ifndef TXRX_TIMING_PULSE
      
      /* Generate the Led pattern for visual Indication
       * check for the first RACH1 slot - all RBUs will wake up for this 
       */
      if ( FIRST_PRACH_ACK_SLOT_IDX == gCurrentSlot.SlotIndex )
      {
         /* Request the pattern */
         CO_State_t current_device_state = MC_STATE_GetDeviceState();
         DM_LedPatterns_t led_req_pattern = LED_PATTERN_NOT_DEFINED_E;

         if (current_device_state < STATE_MAX_E)
         {
           if (STATE_CONFIG_FORM_E == current_device_state)
           {
              if (MC_SMGR_ParentSessionIsActive())
              {
                 led_req_pattern = LED_MESH_STATE_READY_FOR_ACTIVE_E;
              }
              else
              {
                 led_req_pattern = LED_MESH_STATE_CONFIG_FORM_E;
              }
           }
           else
           {
              if ( LED_MESH_CONNECTED_E == DM_LedGetCurrentPattern() )
              {
                 led_req_pattern = LED_MESH_CONNECTED_E;
              }
              else
              {
                 led_req_pattern = SM_mesh_led_patterns[current_device_state];
              }
           }
            /* call LED module tick */
            DM_LedExternalControl(led_req_pattern, gCurrentSlot.ShortFrameIndex & LED_SEQUENCE_BITMASK);
         }
      }
#endif
   }

}

/*****************************************************************************/
/**
*  Function:      MC_TDM_ProcessDLCCHSlot
*  Description:   Process a DLCCH slot
*
*  @param   None
*  @return  bool  True if the slot activity is complete.
*            
*****************************************************************************/
bool MC_TDM_ProcessDLCCHSlot(void)
{
   static bool is_primary_dlcch = false;
   static bool transmit_slot = false;
   bool complete = true;

   if ( MC_TDM_ACTION_PREPROCESS_E == gCurrentSlot.Action )
   {
      //Check that the current slot is a S-RACH
      if ( MC_TDM_SLOT_TYPE_DLCCH_E == MC_TDM_GetCurrentSlotType( ) )
      {
         // Is this our transmit slot?
         if ( gCurrentSlot.SlotIndex == gDLCCHTransmitSlot )
         {
            if ( MC_MACQ_MessageCount(MAC_DLCCHS_Q_E) || MC_MACQ_MessageCount(MAC_DLCCHP_Q_E) )
            {
               gCurrentSlot.pMessage = ALLOCMESHPOOL;
               if ( gCurrentSlot.pMessage )
               {
                  ErrorCode_t msg_found = ERR_NOT_FOUND_E;
                  if ( MC_MACQ_MessageCount(MAC_DLCCHP_Q_E) )
                  {
                     //Read from the DLCCH Queues, prioritising the DLCCH Primary
                     msg_found = MC_MACQ_Pop(MAC_DLCCHP_Q_E, gCurrentSlot.pMessage);
                     if ( SUCCESS_E == msg_found )
                     {
#ifdef USE_ENCPRYPTED_PAYLOAD               
                     /* encrypt the payload before sending */
                     CO_Message_t* pMsg = (CO_Message_t*)gCurrentSlot.pMessage;
                     MC_MessageEncrypt(pMsg);
#endif               
                        is_primary_dlcch = true;
                     }
                  }
                  else
                  {
                     msg_found = MC_MACQ_Pop(MAC_DLCCHS_Q_E, gCurrentSlot.pMessage);
                     if ( SUCCESS_E == msg_found )
                     {
#ifdef USE_ENCPRYPTED_PAYLOAD               
                     /* encrypt the payload before sending */
                     CO_Message_t* pMsg = (CO_Message_t*)gCurrentSlot.pMessage;
                     MC_MessageEncrypt(pMsg);
#endif               
                        is_primary_dlcch = false;
                     }
                  }
                  
                  if ( SUCCESS_E == msg_found )
                  {
                     /* load message */
                     MC_MAC_PacketTxPreProcess(gCurrentSlot.pMessage, DIV_ROUNDUP(DATA_MESSAGE_SIZE, BITS_PER_BYTE), LORA_DLCCH_PREAMBLE_LENGTH, LORA_DLCCH_CRC, gDLCCHTransmitFrequencyIndex);
                     
                     gCurrentSlot.Action = MC_TDM_ACTION_TX_E;
#ifdef ENABLE_TDM_CALIBRATION
                     uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, gDlcchTxOffset, LPTIM_MAX);
#else
                     uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, TDM_DLCCH_TX_OFFSET, LPTIM_MAX);
#endif

                     HAL_LPTIM_Set_CMP(&hlptim1, wake_up);
                     transmit_slot = true;
                     complete = false;
//                     CO_PRINT_B_2(DBG_INFO_E,"DT %d,%d\r\n", gCurrentSlot.SlotIndex, gDLCCHTransmitFrequencyIndex);
                  }
                  else 
                  {
                     FREEMESHPOOL(gCurrentSlot.pMessage);
//                     CO_PRINT_B_1(DBG_INFO_E,"DT %d,NM\r\n", gCurrentSlot.SlotIndex);
                  }
               }
            }
            else 
            {
               //There was no message waiting to transmit.  Change over to receive mode.
               MC_MAC_PacketRxPreProcessSingle(LORA_DLCCH_SYMBOL_TIMEOUT, LORA_DLCCH_PREAMBLE_LENGTH, PHY_DLCCH_PACKET_SIZE, LORA_DLCCH_CRC, gDLCCHReceiveFrequencyIndex);
               
               gCurrentSlot.Action = MC_TDM_ACTION_RX_E;
#ifdef ENABLE_TDM_CALIBRATION
               uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, gDlcchCadOffset, LPTIM_MAX);
#else
               uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, TDM_DLCCH_CAD_OFFSET, LPTIM_MAX);
#endif
               HAL_LPTIM_Set_CMP(&hlptim1, wake_up);
               complete = false;
//               CO_PRINT_B_2(DBG_INFO_E,"DR %d,%d\r\n", gCurrentSlot.SlotIndex, gDLCCHReceiveFrequencyIndex);
            }
         }
         else if ( gCurrentSlot.SlotIndex == gDLCCHReceiveSlot )
         {
            //Set to receive.
            MC_MAC_PacketRxPreProcessSingle(LORA_DLCCH_SYMBOL_TIMEOUT, LORA_DLCCH_PREAMBLE_LENGTH, PHY_DLCCH_PACKET_SIZE, LORA_DLCCH_CRC, gDLCCHReceiveFrequencyIndex);
            
            gCurrentSlot.Action = MC_TDM_ACTION_RX_E;
#ifdef ENABLE_TDM_CALIBRATION
            uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, gDlcchCadOffset, LPTIM_MAX);
#else
            uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, TDM_DLCCH_CAD_OFFSET, LPTIM_MAX);
#endif

            HAL_LPTIM_Set_CMP(&hlptim1, wake_up);
            complete = false;
//            CO_PRINT_B_2(DBG_INFO_E,"DR %d,%d\r\n", gCurrentSlot.SlotIndex, gDLCCHReceiveFrequencyIndex);
         }
      }
   }
   else 
   {
#ifdef TXRX_TIMING_PULSE
      MC_TDM_RadioSendReceive(true);
#else
      MC_TDM_RadioSendReceive();
#endif
      
      if ( transmit_slot )
      {
         if ( is_primary_dlcch )
         {
            MC_MACQ_Discard(MAC_DLCCHP_Q_E);
         }
         else 
         {
            MC_MACQ_Discard(MAC_DLCCHS_Q_E);
         }
         is_primary_dlcch = false;
         transmit_slot = false;
      }
      complete = true;
   }
   
   return complete;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_ProcessPRACHSlot
*  Description:   Process a PRACH slot
*
*  @param   None
*  @return  bool  True if the slot activity is complete.
*            
*****************************************************************************/
bool MC_TDM_ProcessPRACHSlot(void)
{
   static bool transmit_slot = false;
   bool complete = true;
   Channel_t frequencyChannelIdx;
   
   if ( MC_TDM_ACTION_PREPROCESS_E == gCurrentSlot.Action )
   {
      //Check that the current slot is a P-RACH
      if ( MC_TDM_SLOT_TYPE_PRIMARY_RACH_E == MC_TDM_GetCurrentSlotType( ) )
      {
         // Set the channel frequency by indexing the hopping sequence array.
         frequencyChannelIdx = aRachDlcchChanHopSeq[gCurrentSlot.HoppingIndexRACH];

         /* check for P-RACH message to send */
         if ( MC_MACQ_MessageCount(MAC_RACHP_Q_E) )
         {
            gCurrentSlot.pMessage = ALLOCMESHPOOL;
            if ( gCurrentSlot.pMessage )
            {
               if ( SUCCESS_E == MC_MACQ_Pop(MAC_RACHP_Q_E, gCurrentSlot.pMessage) )
               {
#ifdef USE_ENCPRYPTED_PAYLOAD               
                  /* encrypt the payload before sending */
                  CO_Message_t* pMsg = (CO_Message_t*)gCurrentSlot.pMessage;
                  MC_MessageEncrypt(pMsg);
#endif               
                  
                  /* send message */
                  MC_MAC_PacketTxPreProcess(gCurrentSlot.pMessage, DIV_ROUNDUP(DATA_MESSAGE_SIZE, BITS_PER_BYTE), LORA_RACH_PREAMBLE_LENGTH, LORA_RACH_CRC, frequencyChannelIdx);
                  
                  gCurrentSlot.Action = MC_TDM_ACTION_TX_E;
                  uint16_t wake_up;
                  if ( gCurrentSlot.pMessage->Payload.PhyDataReq.Downlink )
                  {
#ifdef ENABLE_TDM_CALIBRATION
                     wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, gRachTxOffsetDownlink, LPTIM_MAX);
#else
                     wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, TDM_RACH_TX_OFFSET_DOWNLINK, LPTIM_MAX);
#endif
                  }
                  else 
                  {
#ifdef ENABLE_TDM_CALIBRATION
                     wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, gRachTxOffsetUplink, LPTIM_MAX);
#else
                     wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, TDM_RACH_TX_OFFSET_UPLINK, LPTIM_MAX);
#endif
                  }
                  HAL_LPTIM_Set_CMP(&hlptim1, wake_up);
                  transmit_slot = true;
                  complete = false;
               }
            }
         }
         else 
         {
            //No message to send.  Set to receive.
            MC_MAC_PacketRxPreProcessSingle(LORA_RACH_SYMBOL_TIMEOUT, LORA_RACH_PREAMBLE_LENGTH, PHY_RACH_PACKET_SIZE, LORA_RACH_CRC, frequencyChannelIdx);

            gCurrentSlot.Action = MC_TDM_ACTION_RX_E;
#ifdef ENABLE_TDM_CALIBRATION
            uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, gRachCadOffset, LPTIM_MAX);
#else
            uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, TDM_RACH_CAD_OFFSET, LPTIM_MAX);
#endif

            HAL_LPTIM_Set_CMP(&hlptim1, wake_up);
            complete = false;

         }
      }
      /* Tell the AckManagement that we have started a new RACH slot.
      The AckManagement runs in the Mesh task and counts slots as part of
      its message back-off algorithm (determines when to resend messages).*/
      CO_Message_t* pUpdateMsg;
      pUpdateMsg = ALLOCMESHPOOL;
      if (pUpdateMsg)
      {
         pUpdateMsg->Type = CO_MESSAGE_MAC_EVENT_E;
         MACEventMessage_t rachSlotUpdate;
         rachSlotUpdate.EventType = CO_RACH_CHANNEL_UPDATE_E;
         rachSlotUpdate.SlotType = MC_TDM_SLOT_TYPE_PRIMARY_RACH_E;
         rachSlotUpdate.ProcessSyncMessage = false;
         rachSlotUpdate.SuperframeSlotIndex = gCurrentSlot.SlotInSuperframe;
         rachSlotUpdate.ShortFrameIndex = gCurrentSlot.ShortFrameIndex;
         rachSlotUpdate.LongFrameIndex = gCurrentSlot.LongFrameIndex;
         rachSlotUpdate.SlotIndex = gCurrentSlot.SlotIndex;
         rachSlotUpdate.TransmitSlot = transmit_slot;
         memcpy(pUpdateMsg->Payload.PhyDataInd.Data, &rachSlotUpdate, sizeof(MACEventMessage_t));

         osStatus osStat = osMessagePut(MeshQ, (uint32_t)pUpdateMsg, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            FREEMESHPOOL(pUpdateMsg);
         }
      }
   }
   else 
   {
#ifdef TXRX_TIMING_PULSE
      MC_TDM_RadioSendReceive(false);
#else
      MC_TDM_RadioSendReceive();
#endif
      if ( transmit_slot )
      {
         MC_MACQ_Discard(MAC_RACHP_Q_E);
         transmit_slot = false;
         
         //Open the following ack slot
         MC_TDM_OpenAckSlot(false);
      }
      complete = true;
   }

   return complete;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_ProcessSRACHSlot
*  Description:   Process a SRACH slot
*
*  @param   None
*  @return  bool  True if the slot activity is complete.
*            
*****************************************************************************/
bool MC_TDM_ProcessSRACHSlot(void)
{
   static bool transmit_slot = false;
   bool complete = true;
   Channel_t frequencyChannelIdx;
   
   if ( MC_TDM_ACTION_PREPROCESS_E == gCurrentSlot.Action )
   {
      //Check that the current slot is a S-RACH
      if ( MC_TDM_SLOT_TYPE_SECONDARY_RACH_E == MC_TDM_GetCurrentSlotType( ) )
      {
         // Set the channel frequency by indexing the hopping sequence array.
         frequencyChannelIdx = aRachDlcchChanHopSeq[gCurrentSlot.HoppingIndexRACH];
         
          /* check for S-RACH message to send */
         if ( MC_MACQ_MessageCount(MAC_RACHS_Q_E) )
         {
            gCurrentSlot.pMessage = ALLOCMESHPOOL;
            if ( gCurrentSlot.pMessage )
            {
               if ( SUCCESS_E == MC_MACQ_Pop(MAC_RACHS_Q_E, gCurrentSlot.pMessage) )
               {
#ifdef USE_ENCPRYPTED_PAYLOAD               
                  /* encrypt the payload before sending */
                  CO_Message_t* pMsg = (CO_Message_t*)gCurrentSlot.pMessage;
                  MC_MessageEncrypt(pMsg);
#endif               
                  /* Configure for transmit */
                  MC_MAC_PacketTxPreProcess(gCurrentSlot.pMessage, DIV_ROUNDUP(DATA_MESSAGE_SIZE, BITS_PER_BYTE), LORA_RACH_PREAMBLE_LENGTH, LORA_RACH_CRC, frequencyChannelIdx);
                  
                  gCurrentSlot.Action = MC_TDM_ACTION_TX_E;
                  uint16_t wake_up;
                  if ( gCurrentSlot.pMessage->Payload.PhyDataReq.Downlink )
                  {
#ifdef ENABLE_TDM_CALIBRATION
                     wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, gRachTxOffsetDownlink, LPTIM_MAX);
#else
                     wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, TDM_RACH_TX_OFFSET_DOWNLINK, LPTIM_MAX);
#endif
                  }
                  else 
                  {
#ifdef ENABLE_TDM_CALIBRATION
                     wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, gRachTxOffsetUplink, LPTIM_MAX);
#else
                     wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, TDM_RACH_TX_OFFSET_UPLINK, LPTIM_MAX);
#endif
                  }
                  HAL_LPTIM_Set_CMP(&hlptim1, wake_up);
                  transmit_slot = true;
                  complete = false;
               }
            }
         }
         else 
         {
            //No message to send.  Set to receive.
            MC_MAC_PacketRxPreProcessSingle(LORA_RACH_SYMBOL_TIMEOUT, LORA_RACH_PREAMBLE_LENGTH, PHY_RACH_PACKET_SIZE, LORA_RACH_CRC, frequencyChannelIdx);

            gCurrentSlot.Action = MC_TDM_ACTION_RX_E;
#ifdef ENABLE_TDM_CALIBRATION
            uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, gRachCadOffset, LPTIM_MAX);
#else
            uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, TDM_RACH_CAD_OFFSET, LPTIM_MAX);
#endif

            HAL_LPTIM_Set_CMP(&hlptim1, wake_up);
            complete = false;

         }
      }
      /* Tell the AckManagement that we have started a new RACH slot.
      The AckManagement runs in the Mesh task and counts slots as part of
      its message back-off algorithm (determines when to resend messages).*/
      CO_Message_t* pUpdateMsg;
      pUpdateMsg = ALLOCMESHPOOL;
      if (pUpdateMsg)
      {
         pUpdateMsg->Type = CO_MESSAGE_MAC_EVENT_E;
         MACEventMessage_t rachSlotUpdate;
         rachSlotUpdate.EventType = CO_RACH_CHANNEL_UPDATE_E;
         rachSlotUpdate.SlotType = MC_TDM_SLOT_TYPE_SECONDARY_RACH_E;
         rachSlotUpdate.ProcessSyncMessage = false;
         rachSlotUpdate.SuperframeSlotIndex = gCurrentSlot.SlotInSuperframe;
         rachSlotUpdate.ShortFrameIndex = gCurrentSlot.ShortFrameIndex;
         rachSlotUpdate.LongFrameIndex = gCurrentSlot.LongFrameIndex;
         rachSlotUpdate.SlotIndex = gCurrentSlot.SlotIndex;
         rachSlotUpdate.TransmitSlot = transmit_slot;
         memcpy(pUpdateMsg->Payload.PhyDataInd.Data, &rachSlotUpdate, sizeof(MACEventMessage_t));

         osStatus osStat = osMessagePut(MeshQ, (uint32_t)pUpdateMsg, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            FREEMESHPOOL(pUpdateMsg);
         }
      }
   }
   else 
   {
#ifdef TXRX_TIMING_PULSE
      MC_TDM_RadioSendReceive(false);
#else
      MC_TDM_RadioSendReceive();
#endif
      if ( transmit_slot )
      {
         MC_MACQ_Discard(MAC_RACHS_Q_E);
         transmit_slot = false;
         //Open the following ack slot
         MC_TDM_OpenAckSlot(false);
      }
      complete = true;
   }

   return complete;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_ProcessDULCHSlot
*  Description:   Process a DULCH slot
*
*  @param   None
*  @return  bool  True if the slot activity is complete.
*            
*****************************************************************************/
bool MC_TDM_ProcessDULCHSlot(void)
{
   static bool transmit_slot = false;
   bool complete = true;
   Channel_t frequencyChannelIdx;
   
   if ( MC_TDM_ACTION_PREPROCESS_E == gCurrentSlot.Action )
   {
      //Check that the current slot is a DULCH
      if ( MC_TDM_SLOT_TYPE_DULCH_E == MC_TDM_GetCurrentSlotType( ) )
      {
         // Set the channel frequency by indexing the hopping sequence array.
         frequencyChannelIdx = aRachDlcchChanHopSeq[gCurrentSlot.HoppingIndexRACH];

         // Is this our DULCH Tx slot?
         uint16_t node_id = MC_GetNetworkAddress();
         if ( CO_IsDulchSlot(node_id, gCurrentSlot.SlotInSuperframe) )
         {
            // check for DULCH message to send
            if ( MC_MACQ_MessageCount(MAC_RACHS_Q_E) )
            {
               gCurrentSlot.pMessage = ALLOCMESHPOOL;
               if ( gCurrentSlot.pMessage )
               {
                  if ( SUCCESS_E == MC_MACQ_Pop(MAC_RACHS_Q_E, gCurrentSlot.pMessage) )
                  {
#ifdef USE_ENCPRYPTED_PAYLOAD               
                  /* encrypt the payload before sending */
                  CO_Message_t* pMsg = (CO_Message_t*)gCurrentSlot.pMessage;
                  MC_MessageEncrypt(pMsg);
#endif               
                     /* Configure for transmit */
                     MC_MAC_PacketTxPreProcess(gCurrentSlot.pMessage, DIV_ROUNDUP(DATA_MESSAGE_SIZE, BITS_PER_BYTE), LORA_RACH_PREAMBLE_LENGTH, LORA_RACH_CRC, frequencyChannelIdx);
                     
                     gCurrentSlot.Action = MC_TDM_ACTION_TX_E;
                     uint16_t wake_up;
                     if ( gCurrentSlot.pMessage->Payload.PhyDataReq.Downlink )
                     {
#ifdef ENABLE_TDM_CALIBRATION
                        wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, gRachTxOffsetDownlink, LPTIM_MAX);
#else
                        wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, TDM_RACH_TX_OFFSET_DOWNLINK, LPTIM_MAX);
#endif
                     }
                     else 
                     {
#ifdef ENABLE_TDM_CALIBRATION
                        wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, gRachTxOffsetUplink, LPTIM_MAX);
#else
                        wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, TDM_RACH_TX_OFFSET_UPLINK, LPTIM_MAX);
#endif
                     }
                     HAL_LPTIM_Set_CMP(&hlptim1, wake_up);
                     transmit_slot = true;
                     complete = false;
                  }
               }
            }
         }
         else 
         {
            //No message to send.  Set to receive.
            MC_MAC_PacketRxPreProcessSingle(LORA_RACH_SYMBOL_TIMEOUT, LORA_RACH_PREAMBLE_LENGTH, PHY_RACH_PACKET_SIZE, LORA_RACH_CRC, frequencyChannelIdx);

            gCurrentSlot.Action = MC_TDM_ACTION_RX_E;
#ifdef ENABLE_TDM_CALIBRATION
            uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, gRachCadOffset, LPTIM_MAX);
#else
            uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, TDM_RACH_CAD_OFFSET, LPTIM_MAX);
#endif

            HAL_LPTIM_Set_CMP(&hlptim1, wake_up);
            complete = false;
         }
      }
   }
   else 
   {
#ifdef TXRX_TIMING_PULSE
      MC_TDM_RadioSendReceive(false);
#else
      MC_TDM_RadioSendReceive();
#endif
      if ( transmit_slot )
      {
         MC_MACQ_Discard(MAC_RACHS_Q_E);
         transmit_slot = false;
         //Open the following ack slot
         MC_TDM_OpenAckSlot(false);
      }
      complete = true;
   }

   return complete;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_ProcessACKSlot
*  Description:   Process an ACK slot
*
*  @param   None
*  @return  bool  True if the slot activity is complete.
*            
*****************************************************************************/
bool MC_TDM_ProcessACKSlot(void)
{
   static bool transmit_slot = false;
   bool complete = false;
   MC_TDM_SlotType_t slot_type = MC_TDM_GetCurrentSlotType( );
   Channel_t frequencyChannelIdx;
   
   if ( MC_TDM_ACTION_PREPROCESS_E == gCurrentSlot.Action )
   {
      //Check that the current slot is a S-RACH
      if ( (MC_TDM_SLOT_TYPE_PRIMARY_ACK_E == slot_type) ||
           (MC_TDM_SLOT_TYPE_SECONDARY_ACK_E == slot_type) ||
           (MC_TDM_SLOT_TYPE_DULCH_ACK_E == slot_type) )
      {
         // Set the channel frequency by indexing the hopping sequence array.
         frequencyChannelIdx = aRachDlcchChanHopSeq[gCurrentSlot.HoppingIndexRACH];
         
         /* check for ACK message to send */
         if ( MC_MACQ_MessageCount(MAC_ACK_Q_E) )
         {
            gCurrentSlot.pMessage = (CO_Message_t*)ALLOCMESHPOOL;
            if ( gCurrentSlot.pMessage )
            {
               if ( SUCCESS_E == MC_MACQ_Pop(MAC_ACK_Q_E, gCurrentSlot.pMessage) )
               {
                  /* send message */
                  MC_MAC_PacketTxPreProcess(gCurrentSlot.pMessage, DIV_ROUNDUP(ACKNOWLEDGEMENT_SIZE, BITS_PER_BYTE), LORA_ACK_PREAMBLE_LENGTH, LORA_ACK_CRC, frequencyChannelIdx);
                  
                  gCurrentSlot.Action = MC_TDM_ACTION_TX_E;
#ifdef ENABLE_TDM_CALIBRATION
                  uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, gAckTxOffsetUplink, LPTIM_MAX);
#else
                  uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, TDM_ACK_TX_OFFSET, LPTIM_MAX);
#endif

                  HAL_LPTIM_Set_CMP(&hlptim1, wake_up);
                     uint16_t time_now = HAL_LPTIM_ReadCounter(&hlptim1);
//                     CO_PRINT_B_2(DBG_INFO_E,"ak now=%d, wk=%d\r\n",time_now,wake_up);
                  transmit_slot = true;
                  complete = false;
               }
               else 
               {
                  FREEMESHPOOL(gCurrentSlot.pMessage);
               }
            }
         }
         else 
         {
            /* configure for radio receive */
            MC_MAC_PacketRxPreProcessSingle(LORA_ACK_SYMBOL_TIMEOUT, LORA_ACK_PREAMBLE_LENGTH, PHY_RACH_ACK_PACKET_SIZE, LORA_ACK_CRC, frequencyChannelIdx);

            gCurrentSlot.Action = MC_TDM_ACTION_RX_E;
#ifdef ENABLE_TDM_CALIBRATION
            uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, gAckCadOffset, LPTIM_MAX);
#else
            uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, TDM_ACK_RX_OFFSET, LPTIM_MAX);
#endif

            HAL_LPTIM_Set_CMP(&hlptim1, wake_up);
                     uint16_t time_now = HAL_LPTIM_ReadCounter(&hlptim1);
//                     CO_PRINT_B_3(DBG_INFO_E,"dl wk=%d now=%d, nwk=%d\r\n",gCurrentSlot.WakeupTime,time_now,wake_up);
            complete = false;
         }
      }
   }
   else 
   {
#ifdef TXRX_TIMING_PULSE
      MC_TDM_RadioSendReceive(false);
#else
      MC_TDM_RadioSendReceive();
#endif
      
      if ( transmit_slot )
      {
         MC_MACQ_Discard(MAC_ACK_Q_E);
         //Detect build up of DCH messages in the queue
         uint32_t msg_count =  MC_MACQ_MessageCount(MAC_ACK_Q_E);
         if ( 0 != msg_count )
         {
            CO_PRINT_B_1(DBG_ERROR_E,"ACKQ OVR %d\r\n", msg_count);
            MC_MACQ_ResetMessageQueue(MAC_ACK_Q_E);
         }
         transmit_slot = false;
      }
#ifndef WAKE_FOR_ALL_SLOTS
      //Close the ACK slot
      if ( MC_MAC_TEST_MODE_OFF_E == MC_GetTestMode() )
      {
         uint32_t ack_slot = gCurrentSlot.SlotIndex;
         MC_TDM_SetSlotType( ack_slot, MC_TDM_SLOT_TYPE_UNUSED_E );
      }
#endif
      complete = true;
   }
   return complete;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_ProcessDCHSlot
*  Description:   Process a DCH slot
*
*  @param   None
*  @return  bool  True if the slot activity is complete.
*            
*****************************************************************************/
bool MC_TDM_ProcessDCHSlot(void)
{
   bool complete = true;
   Channel_t frequencyChannelIdx;
   uint16_t symbTimeout = 0;
   uint16_t rx_wakeup_offset;
   
   if ( MC_TDM_ACTION_PREPROCESS_E == gCurrentSlot.Action )
   {
      //If the slot is our Tx slot we need to send a heartbeat
      //Otherwise we listen for a neighbour's heartbeat
      uint16_t node_id = CO_CalculateNodeID(gCurrentSlot.SlotInSuperframe);
      if ( CO_BAD_NODE_ID != node_id )
      {
         //Advance the DCH hopping sequence
         gCurrentSlot.HoppingIndexDCH = MC_TDM_GetNextHbSlotFrequency();

         //Receive a heartbeat
         // Set the channel frequency by indexing the DCH hopping sequence array using the DCHHoppingIndex.
         CO_ASSERT_RET_MSG(INVALID_HOPPING_SEQ_INDEX != gCurrentSlot.HoppingIndexDCH, "Error calculating the DCH Rx hopping sequence index", true);
         frequencyChannelIdx = aDchChanHopSeq[gCurrentSlot.HoppingIndexDCH];

         //      CO_PRINT_B_1(DBG_INFO_E,"F=%d\r\n",frequencyChannelIdx);
         if ( MC_TDM_DCH_TX_E == gDchBehaviour[node_id] )
         {
            //Send a heartbeat
            /* check for DCH message to send */
            if ( MC_MACQ_MessageCount(MAC_DCH_Q_E) )
            {
               gCurrentSlot.pMessage = ALLOCMESHPOOL;
               if ( gCurrentSlot.pMessage )
               {
                  if ( SUCCESS_E == MC_MACQ_Pop(MAC_DCH_Q_E, gCurrentSlot.pMessage) )
                  {
                     // Configure the LoRa modem to transmit the DCH packet
                     MC_MAC_PacketTxPreProcess(gCurrentSlot.pMessage, DIV_ROUNDUP(HEARTBEAT_SIZE, BITS_PER_BYTE), LORA_DCH_PREAMBLE_LENGTH, LORA_DCH_CRC, frequencyChannelIdx);
                     
                     gCurrentSlot.Action = MC_TDM_ACTION_TX_E;
#ifdef ENABLE_TDM_CALIBRATION
                     uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, gDchTxOffset, LPTIM_MAX);
#else
                     uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, TDM_DCH_TX_OFFSET, LPTIM_MAX);
#endif

                     uint16_t time_now = HAL_LPTIM_ReadCounter(&hlptim1);
                     HAL_LPTIM_Set_CMP(&hlptim1, wake_up);
                     complete = false;
//                     CO_PRINT_B_3(DBG_INFO_E,"dl wk=%d now=%d, nwk=%d\r\n",gCurrentSlot.WakeupTime,time_now,wake_up);
                  }
                  else 
                  {
                     //failed to retrieve a message.  Delete the memory
                     FREEMESHPOOL(gCurrentSlot.pMessage);
                     gCurrentSlot.pMessage = NULL;
                     gCurrentSlot.Action = MC_TDM_ACTION_NONE_E;
                  }
                  //remove the message from the queue
                  MC_MACQ_Discard(MAC_DCH_Q_E);
                  //Detect build up of DCH messages in the queue
                  uint32_t msg_count =  MC_MACQ_MessageCount(MAC_DCH_Q_E);
                  if ( 0 != msg_count )
                  {
                     CO_PRINT_B_1(DBG_ERROR_E,"DCHQ OVR %d\r\n", msg_count);
                     MC_MACQ_ResetMessageQueue(MAC_DCH_Q_E);
                  }
               }
            }
         }
         else if ( (MC_TDM_DCH_RX_E == gDchBehaviour[node_id]) ||
                   (MC_TDM_DCH_TRACKING_E == gDchBehaviour[node_id]) )
         {
            // Before frequency lock has been achieved, we allow a longer timeout period for the DCH to accommodate crystal inaccuracy: see CYG2-786.
            if (false == gFreqLockAchieved )
            {
               symbTimeout = WIDE_DCH_SYMBOL_TIMEOUT;  // 8.192ms timeout
#ifdef ENABLE_TDM_CALIBRATION
               rx_wakeup_offset = gDchRxOffset - MC_TDM_WIDE_INITAL_DCH_OFFSET;  //wake up slightly earlier
#else
               rx_wakeup_offset = TDM_DCH_RX_OFFSET - MC_TDM_WIDE_INITAL_DCH_OFFSET;  //wake up slightly earlier
#endif
            }
            else
            {
               symbTimeout = LORA_DCH_SYMBOL_TIMEOUT;  // 4.096ms timeout
#ifdef ENABLE_TDM_CALIBRATION
               rx_wakeup_offset = gDchRxOffset;
#else
               rx_wakeup_offset = TDM_DCH_RX_OFFSET;
#endif
            }
         
            // Configure the LoRa modem for Rx Single mode, DCH packets
            MC_MAC_PacketRxPreProcessSingle(symbTimeout, LORA_DCH_PREAMBLE_LENGTH, PHY_DCH_PACKET_SIZE, LORA_DCH_CRC, frequencyChannelIdx);
            
            gCurrentSlot.Action = MC_TDM_ACTION_RX_E;
            uint16_t wake_up = SUM_WRAP(gCurrentSlot.WakeupTime, rx_wakeup_offset, LPTIM_MAX);

            HAL_LPTIM_Set_CMP(&hlptim1, wake_up);
//                   uint16_t time_now = HAL_LPTIM_ReadCounter(&hlptim1);
//                     CO_PRINT_B_3(DBG_INFO_E,"dl wk=%d now=%d, nwk=%d\r\n",gCurrentSlot.WakeupTime,time_now,wake_up);
            complete = false;
         }
      }
#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
      MC_TDM_ShowFirstWakeupTimes();
#endif
   }
   else 
   {
#ifdef TXRX_TIMING_PULSE
      MC_TDM_RadioSendReceive(false);
#else
      MC_TDM_RadioSendReceive();
#endif
      complete = true;
      
#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
      MC_TDM_ShowSecondWakeupTimes();
#endif
   }
   
   return complete;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_RadioSendReceive
*  Description:   Perform the send or receive for a DCH slot
*
*  @param   None
*  @return  None.
*            
*****************************************************************************/
#ifdef TXRX_TIMING_PULSE
void MC_TDM_RadioSendReceive(const bool pulse)
#else
void MC_TDM_RadioSendReceive(void)
#endif
{
#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
   lora_standby_time = HAL_LPTIM_ReadCounter(&hlptim1);
#endif
   
   //Wake up the radio
   Radio.Standby();

   if ( MC_TDM_ACTION_TX_E == gCurrentSlot.Action )
   {
#ifdef ENABLE_HEARTBEAT_TX_SKIP
      if ( 0 == gNumberOfHeatbeatsToSkip )
      {
         if ( gCurrentSlot.pMessage )
         {
#ifdef TXRX_TIMING_PULSE
            MC_MAC_PacketTransmit(pulse);
#else
            MC_MAC_PacketTransmit();
#endif
                  
            /* send message to mesh for logging */
            gCurrentSlot.pMessage->Payload.PhyDataReq.slotIdxInSuperframe = gCurrentSlot.SlotInSuperframe;
            gCurrentSlot.pMessage->Payload.PhyDataReq.freqChanIdx = MC_MAC_LoraParameters.CurrentFrequencyChannelIdx;
            osStatus osStat = osMessagePut(MeshQ, (uint32_t)gCurrentSlot.pMessage, 0);
            if (osOK != osStat)
            {
               FREEMESHPOOL(gCurrentSlot.pMessage);
            }
         }
      }
      else 
      {
         gNumberOfHeatbeatsToSkip--;
         CO_PRINT_B_0(DBG_INFO_E,"Hearbeat skipped\r\n");
      }
#else // ENABLE_HEARTBEAT_TX_SKIP
      if ( gCurrentSlot.pMessage )
      {
#ifdef TXRX_TIMING_PULSE
         MC_MAC_PacketTransmit(pulse);
#else
         MC_MAC_PacketTransmit();
#endif
                  
         /* send message to mesh for logging */
         gCurrentSlot.pMessage->Payload.PhyDataReq.slotIdxInSuperframe = gCurrentSlot.SlotInSuperframe;
         gCurrentSlot.pMessage->Payload.PhyDataReq.freqChanIdx = MC_MAC_LoraParameters.CurrentFrequencyChannelIdx;
         osStatus osStat = osMessagePut(MeshQ, (uint32_t)gCurrentSlot.pMessage, 0);
         if (osOK != osStat)
         {
            FREEMESHPOOL(gCurrentSlot.pMessage);
         }
      }
#endif // ENABLE_HEARTBEAT_TX_SKIP
   }
   else if ( MC_TDM_ACTION_RX_E == gCurrentSlot.Action )
   {

      // Start radio receiving
      MC_TDM_SlotType_t slot_type = MC_TDM_GetCurrentSlotType( );
      if ( (MC_TDM_SLOT_TYPE_DCH_E == slot_type) ||
           (MC_TDM_SLOT_TYPE_PRIMARY_ACK_E == slot_type) ||
           (MC_TDM_SLOT_TYPE_SECONDARY_ACK_E == slot_type) ||
           (MC_TDM_SLOT_TYPE_DULCH_ACK_E == slot_type) )
      {
         MC_MAC_PacketReceiveSingle();
      }
      else 
      {
//         CO_InlineDelayTicks(gTxSettlingTime);
#ifdef TXRX_TIMING_PULSE_INCLUDE_CAD
         GpioWrite(&StatusLedBlue,1);
#endif
         if ( gTxInProgress )
         {
            uint16_t time_now = HAL_LPTIM_ReadCounter(&hlptim1);
            CO_PRINT_A_2(DBG_ERROR_E,"Detected Tx fail 1 tx_start=%d, now=%d\r\n", gTxTime, time_now);
            gTxInProgress = false;
         }

         Radio.StartCad();
      }
   }
   gCurrentSlot.pMessage = NULL;
   gCurrentSlot.Action = MC_TDM_ACTION_PREPROCESS_E;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_ProcessUnusedSlot
*  Description:   Process an unused slot
*
*  @param   None
*  @return  bool  True if the slot activity is complete.
*            
*****************************************************************************/
bool MC_TDM_ProcessUnusedSlot(void)
{
   return true;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_Advance
*  Description:   Calculate the next required wakeup and configure for the slot
*
*  @param   None
*  @return  None.
*            
*****************************************************************************/
void MC_TDM_Advance(void)
{
   int32_t slot_offset;
   uint32_t slots_to_advance = MC_TDM_GetSlotsToAdvance();
   int32_t short_frames_per_long_frame = MC_ShortFramesPerLongFrame();
   int32_t slots_per_long_frame = MC_SlotsPerLongFrame();
   int32_t slots_per_super_frame = MC_SlotsPerSuperFrame();
   
#ifdef DEFER_SYNCH_TO_NEXT_LONG_FRAME
   if ( gSyncPulse )
   {
      gSyncPulse = false;
      uint16_t time_now = HAL_LPTIM_ReadCounter(&hlptim1);
      
      CO_PRINT_B_3(DBG_INFO_E,"now=%d,sync=%d cor=%d\r\n", time_now, gSyncReference, gSyncCorrection);
      
      if ( gSyncUpdated )
      {
         CO_PRINT_B_2(DBG_INFO_E,"sync %d updated to %d\r\n", gSyncReference, gSyncCorrection );
         gPreviousSyncReference = gSyncReference;
         gSyncReference = gSyncCorrection;
         gSyncUpdated = false;
      }
      else 
      {
         CO_PRINT_B_0(DBG_INFO_E,"No sync correction applied\r\n");
      }
   }
#else
   if ( gSyncPulse )
   {
      gSyncPulse = false;
   }
#endif
   
//   CO_PRINT_B_3(DBG_INFO_E,"s=%d a=%d t=%d\r\n", gNextSlot.SlotIndex, slots_to_advance, MC_TDM_GetSlotType(gNextSlot.SlotIndex));

   //Advance to the next slot
   gNextSlot.SlotIndex += slots_to_advance;
   gNextSlot.SlotIndexInLongFrame = SUM_WRAP(gNextSlot.SlotIndexInLongFrame, slots_to_advance, slots_per_long_frame);
   gNextSlot.SlotInSuperframe = SUM_WRAP(gNextSlot.SlotInSuperframe, slots_to_advance, slots_per_super_frame);
   
   if ( SLOTS_PER_SHORT_FRAME <= gNextSlot.SlotIndex )
   {
      gNextSlot.SlotIndex -= SLOTS_PER_SHORT_FRAME;
      gNextSlot.ShortFrameIndex++;
      //Advance the RACH/DLCCH hopping sequence
      gNextSlot.HoppingIndexRACH = gNextSlot.ShortFrameIndex % MC_DlcchHoppingSequenceLength();
      //set the DLCCH Tx and Rx slots
      MC_TDM_SetShortFrameActions(gNextSlot.SlotInSuperframe);
      
      if ( gNextSlot.ShortFrameIndex == short_frames_per_long_frame )
      {
         gSyncPulse = true;

         gNextSlot.ShortFrameIndex = 0;
         gNextSlot.SlotIndexInLongFrame = gNextSlot.SlotIndex;
         gNextSlot.LongFrameIndex++;
         if ( LONG_FRAMES_PER_SUPER_FRAME == gNextSlot.LongFrameIndex )
         {
            gNextSlot.LongFrameIndex = 0;
            gNextSlot.SlotInSuperframe = gNextSlot.SlotIndex;
         }
         //Tell the Mesh that we are advancing to a new long frame
         MC_TDM_SendNewLongFrameToMesh();
         //Update the synch reference time for the new long frame
         //RBUs will correct this using the tracking node heartbeat
//         if ( BASE_NCU_E != gBaseType )
//         {
            uint32_t mod_ref = MC_CyclesPerLongFrame();
            mod_ref = mod_ref & LPTIM_MASK;
            gPreviousSyncReference = gSyncReference;
            gSyncReference = (uint16_t)SUM_WRAP(gSyncReference, mod_ref, LPTIM_MAX);
//            CO_PRINT_B_2(DBG_INFO_E,"New LF sync %d updated to %d\r\n", gPreviousSyncReference, gSyncReference);
//         }
         
         //Check for scheduled state change
         if ( 0 < gStateChangeLongFrameCount )
         {
            gStateChangeLongFrameCount--;
            if ( 0 == gStateChangeLongFrameCount )
            {
               if ( StateChangeNotifyCallbackFn )
               {
                  StateChangeNotifyCallbackFn(gNextSlot.LongFrameIndex);
               }
            }
         }

         //Check for scheduled channel hopping
         if ( 0 < gFreqHopLongFrameCount )
         {
            gFreqHopLongFrameCount--;
            if ( 0 == gFreqHopLongFrameCount )
            {
               //Start frequency hopping
               MC_TDM_EnableChannelHopping();
            }
         }
      }
      else 
      {
         gSyncPulse = false;
      }
   }

   //Check if it's time to generate the next heartbeat
   MC_TDM_SlotType_t slot_type = MC_TDM_GetSlotType(gNextSlot.SlotIndex);
   if ( MC_TDM_SLOT_TYPE_DCH_E == slot_type )
   {
      if ( gNextSlot.SlotIndexInLongFrame == gHeartbeatTxSlotNumber )
      {
         MC_TDM_GenerateHeartbeat();
      }
   }

   //Check if it's time to generate the next DULCH message
   if ( MC_TDM_SLOT_TYPE_DULCH_E == slot_type )
   {
      MC_TDM_RequestDulchMessage();
   }

   //Set the next wakeup to the start of the next slot
   //The RBU is forward calculated from the start of the long frame (gSyncReference)
   slot_offset = (int32_t)((double)gNextSlot.SlotIndexInLongFrame * gSlotLength);
   
   if (  (MC_TDM_SLOT_TYPE_SECONDARY_ACK_E == slot_type) ||
         (MC_TDM_SLOT_TYPE_PRIMARY_ACK_E == slot_type) ||
         (MC_TDM_SLOT_TYPE_DULCH_ACK_E == slot_type) )
   {
      slot_offset += TDM_ACK_SLOT_OFFSET;//No need to SMU_WRAP
   }
   
   gNextSlot.WakeupTime = (uint16_t)SUM_WRAP(gSyncReference, slot_offset, LPTIM_MAX);


   HAL_LPTIM_Set_CMP(&hlptim1, gNextSlot.WakeupTime);
}


/*****************************************************************************/
/**
*  Function:      MC_TDM_GetSlotReferenceTime
*  Description:   Return the expected RxDone time for the current slot
*
*  @param   None
*  @return  uint16_t    The predicted RxDone time.
*            
*****************************************************************************/
uint16_t MC_TDM_GetSlotReferenceTime(void)
{
   uint16_t slot_reference_time;
   MC_TDM_SlotType_t slot_type = MC_TDM_GetCurrentSlotType( );
   
   switch ( slot_type )
   {
      case MC_TDM_SLOT_TYPE_DLCCH_E:
#ifdef ENABLE_TDM_CALIBRATION
         slot_reference_time = gDlcchCadOffset + TDM_DATA_MESSAGE_RXDONE_OFFSET;
#else
         slot_reference_time = TDM_DLCCH_CAD_OFFSET + TDM_DATA_MESSAGE_RXDONE_OFFSET;
#endif
      break;
      case MC_TDM_SLOT_TYPE_PRIMARY_RACH_E: //intentional drop through
      case MC_TDM_SLOT_TYPE_SECONDARY_RACH_E:
#ifdef ENABLE_TDM_CALIBRATION
         slot_reference_time = gRachCadOffset + TDM_DATA_MESSAGE_RXDONE_OFFSET;
#else
         slot_reference_time = TDM_RACH_CAD_OFFSET + TDM_DATA_MESSAGE_RXDONE_OFFSET;
#endif
      break;
      case MC_TDM_SLOT_TYPE_DCH_E:
#ifdef ENABLE_TDM_CALIBRATION
         slot_reference_time = gDchRxOffset + TDM_DCH_MESSAGE_RXDONE_OFFSET;
#else
         slot_reference_time = TDM_DCH_RX_OFFSET + TDM_DCH_MESSAGE_RXDONE_OFFSET;
#endif
      break;
      case MC_TDM_SLOT_TYPE_PRIMARY_ACK_E: //intentional drop through
      case MC_TDM_SLOT_TYPE_SECONDARY_ACK_E:
      case MC_TDM_SLOT_TYPE_DULCH_ACK_E:
#ifdef ENABLE_TDM_CALIBRATION
      slot_reference_time = gAckCadOffset + TDM_DCH_MESSAGE_RXDONE_OFFSET;//ACK is same size as DCH
#else
      slot_reference_time = TDM_ACK_SLOT_OFFSET + TDM_ACK_RX_OFFSET + TDM_DCH_MESSAGE_RXDONE_OFFSET;//ACK is same size as DCH
#endif
      break;
      default:
         slot_reference_time = 0;
         break;
   }
   
   //Offset this from the current slot start
   slot_reference_time = SUM_WRAP(gCurrentSlot.WakeupTime, slot_reference_time, LPTIM_MAX);
   
   return slot_reference_time;
}



/*****************************************************************************/
/**
*  Function:      MC_TDM_ConstructSlotInSuperframeIdx
*  Description:   Calculate the superframe slot from the frame indexes
*
*  @param   LongFrameIndex    The long frame number
*  @param   ShortFrameIndex   The shortframe number
*  @param   SlotIndex         The slot index into the short frame
*
*  @return  uint32_t          The super frame slot number.
*            
*****************************************************************************/
uint32_t MC_TDM_ConstructSlotInSuperframeIdx(const uint32_t LongFrameIndex, const uint32_t ShortFrameIndex, const uint32_t SlotIndex)
{
   uint32_t slots_per_super_frame = MC_SlotsPerSuperFrame();
   uint32_t short_frames_per_long_frame = MC_ShortFramesPerLongFrame();
   
   //Parameter range check
   CO_ASSERT_RET_MSG((LONG_FRAMES_PER_SUPER_FRAME > LongFrameIndex), ERR_INVALID_PARAMETER_E, "MC_TDM_ConstructSlotInSuperframeIdx : oversize LongFrameIndex");
   CO_ASSERT_RET_MSG((short_frames_per_long_frame > ShortFrameIndex), ERR_INVALID_PARAMETER_E, "MC_TDM_ConstructSlotInSuperframeIdx : oversize ShortFrameIndex");
   CO_ASSERT_RET_MSG((slots_per_super_frame > SlotIndex), ERR_INVALID_PARAMETER_E, "MC_TDM_ConstructSlotInSuperframeIdx : oversize SlotIndex");
   
   uint32_t slot = LongFrameIndex * MC_SlotsPerLongFrame();
   slot += (ShortFrameIndex * SLOTS_PER_SHORT_FRAME);
   slot += SlotIndex;
   return slot;
}



/*****************************************************************************/
/**
*  Function:      MC_TDM_StateChangeAtNextLongFrame
*  Description:   Schedule the change of state
*
*  @param   callbackFunc       Function pointer, to be called when the state changes.
*  @return  None.
*            
*****************************************************************************/
void MC_TDM_StateChangeAtNextLongFrame(void (* callbackFunc)(const uint8_t))
{
   // Test to see whether a new state notification has been requested, and whether there is sufficient time to schedule it. 
   uint16_t max_sfi_for_state_change = MC_MaxSfiForStateChange();
   if ( gCurrentSlot.ShortFrameIndex < max_sfi_for_state_change )
   {
      gStateChangeLongFrameCount = 1;
   }
   else
   {
      gStateChangeLongFrameCount = 2;
   }
   
   if ( callbackFunc )
   {
      StateChangeNotifyCallbackFn = callbackFunc;
   }
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_StartFrequencyHoppingAtNextLongFrame
*  Description:   Schedule the frequency hopping
*
*  @param   None.
*  @return  None.
*            
*****************************************************************************/
void MC_TDM_StartFrequencyHoppingAtNextLongFrame(void)
{
   // Test to see whether a new state notification has been requested, and whether there is sufficient time to schedule it. 
   uint16_t max_sfi_for_state_change = MC_MaxSfiForStateChange();
   if ( gCurrentSlot.ShortFrameIndex < max_sfi_for_state_change )
   {
      gFreqHopLongFrameCount = 1;
   }
   else
   {
      gFreqHopLongFrameCount = 2;
   }
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_GenerateHeartbeat
*  Description:   Send a message to the mesh task to generate a heartbeat
*
*  @param   None.
*  @return  None.
*            
*****************************************************************************/
void MC_TDM_GenerateHeartbeat(void)
{
   /* send message to mesh task to trigger the generation of the next heartbeat */
   CO_Message_t *pGenHeartbeat = NULL;
   pGenHeartbeat = ALLOCMESHPOOL;
   if (pGenHeartbeat)
   {
      osStatus osStat;
      
      pGenHeartbeat->Type = CO_MESSAGE_GENERATE_HEARTBEAT_E;
      pGenHeartbeat->Payload.GenerateHeartbeat.SlotIndex = MC_TDM_ConvertSlotToHeartbeatValue(gNextSlot.SlotIndex);
      pGenHeartbeat->Payload.GenerateHeartbeat.ShortFrameIndex = gNextSlot.ShortFrameIndex;
      pGenHeartbeat->Payload.GenerateHeartbeat.LongFrameIndex = gNextSlot.LongFrameIndex;
   
      osStat = osMessagePut(MeshQ, (uint32_t)pGenHeartbeat, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         FREEMESHPOOL(pGenHeartbeat);
      }
   }
}



/*****************************************************************************/
/**
*  Function:      MC_TDM_SetSynchReferenceTime
*  Description:   Set the synch reference counter to the start of a long frame
*
*  @param   nextFrameStart    The calculated LPTIM count at the start of the next long frame
*  @param   aveFrameLength    The long frame duration in clock ticks
*  @return  None.
*            
*****************************************************************************/
void MC_TDM_SetSynchReferenceTime(const uint16_t nextFrameStart, const uint32_t aveFrameLength)
{
   //Parameter range check
   uint32_t ticks_per_long_frame = MC_CyclesPerLongFrame();
   uint32_t delta = ABS_DIFFERENCE(ticks_per_long_frame, aveFrameLength);
   CO_ASSERT_VOID_MSG((delta < LONG_FRAME_TICK_TOLERANCE), "MC_TDM_SetSynchReferenceTime : aveFrameLength");
   
#ifdef USE_AVERAGE_SYNC_FRAMES
   uint16_t slots_per_long_frame = MC_SlotsPerLongFrame();
   gSlotLength = ((double)aveFrameLength / (double)slots_per_long_frame);
#endif
   
#ifdef DEFER_SYNCH_TO_NEXT_LONG_FRAME
   gSyncCorrection =  nextFrameStart;
   gSyncUpdated = true;
//   CO_PRINT_B_2(DBG_INFO_E,"Update: gSyncReference=%d gSyncCorrection=%d\r\n", gSyncReference, gSyncCorrection);
#else
   gSyncCorrection = nextFrameStart;
   gPreviousSyncReference = gSyncReference;
   gSyncReference = gSyncCorrection;
//   CO_PRINT_B_2(DBG_INFO_E,"Update: gSyncReference=%d gSyncCorrection=%d\r\n", gPreviousSyncReference, gSyncReference);
#endif
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_SendNewLongFrameToMesh
*  Description:   Send a message to the Mesh that we have started a new long frame
*
*  @param   None.
*  @return  None.
*            
*****************************************************************************/
void MC_TDM_SendNewLongFrameToMesh(void)
{
   // Send a message to the Mesh Task to signal to the SM that a new long frame has started.
   CO_Message_t* pMsg;
   pMsg = ALLOCMESHPOOL;
   if (pMsg)
   {
      pMsg->Type = CO_MESSAGE_MAC_EVENT_E;
      MACEventMessage_t macEvent;
      macEvent.EventType = CO_NEW_LONG_FRAME_E;
      macEvent.LongFrameIndex = gNextSlot.LongFrameIndex;
      uint16_t frame_length = MC_ShortFramesPerLongFrame();
      if ( MAX_SHORT_FRAMES_PER_LONG_FRAME == frame_length )
      {
         macEvent.Data[0] = 1;
      }
      else 
      {
         macEvent.Data[0] = 0;
      }
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
* MC_TDM_GetDlcchFrequencyForNode
*
* Calculate which DLCCH slot the supplied node ID transmits on.
*
* @param - node_id      The node to calculate the DLCCH frequency for.
*
* @return - uint32_t    The frequency for the supplied node.
*/
uint32_t MC_TDM_GetDlcchFrequencyForNode(const uint16_t node_id)
{
   //Parameter range check
   uint32_t max_devices = MC_MaxNumberOfNodes();
   CO_ASSERT_RET_MSG((max_devices > node_id), 0, "MC_TDM_GetDlcchSlotForNode : node_id");

   uint32_t frequency = ((gNextSlot.HoppingIndexRACH + ((node_id % 200) /20)) % 10);
   
   return frequency;
}

/*************************************************************************************/
/**
* MC_TDM_OpenAckSlot
*
* Called when we need to open an ACK slot to send or receive an ACK.
* If we are sending an ACK, the TDM will have already decided to sleep through the
* slot, so we need to revise the wake-up time.
* Received ACKs were known about in advance, so the wake-up time will be correct.
*
* @param -  sendingAck  True if we are opening a slot to send an ACK. False to receive.
*
* @return - None.
*/
void MC_TDM_OpenAckSlot(const bool sendingAck)
{
#ifndef WAKE_FOR_ALL_SLOTS
   bool update_wake_up = sendingAck;
   
   uint32_t ack_slot = gCurrentSlot.SlotIndex + 2;
   
   //check that we have been called during a RACH slot
   switch ( MC_TDM_GetSlotTypeDefault(gCurrentSlot.SlotIndex) )
   {
      case MC_TDM_SLOT_TYPE_PRIMARY_RACH_E:
         MC_TDM_SetSlotType( ack_slot, MC_TDM_SLOT_TYPE_PRIMARY_ACK_E );
//         CO_PRINT_B_1(DBG_INFO_E,"P-ACK %d\r\n", ack_slot);
         break;
      case MC_TDM_SLOT_TYPE_SECONDARY_RACH_E:
         MC_TDM_SetSlotType( ack_slot, MC_TDM_SLOT_TYPE_SECONDARY_ACK_E );
//         CO_PRINT_B_1(DBG_INFO_E,"S-ACK %d\r\n", ack_slot);
         break;
      case MC_TDM_SLOT_TYPE_DULCH_E:
         if ( update_wake_up )
         {
            MC_TDM_SetSlotType( ack_slot, MC_TDM_SLOT_TYPE_DULCH_ACK_E );
//            CO_PRINT_B_1(DBG_INFO_E,"D-ACK %d\r\n", ack_slot);
         }
         break;
      default:
         CO_PRINT_B_2(DBG_ERROR_E,"MC_TDM_OpenAckSlot called for non-RACH slot=%d, action=%d\r\n", gCurrentSlot.SlotIndex, MC_TDM_GetCurrentSlotType( ));
         update_wake_up = false;
         break;
   }
   
   // The TDM will have calculated a wake-up time that sleeps through the ACK slot.
   if ( update_wake_up )
   {
      //If the ack slot is that last slot in the long frame the frame sync will already 
      //have been updated for the following long frame.  It needs to be put back or the
      //next wakeup time will be calculated incorrectly
      if ( FIRST_PRACH_ACK_SLOT_IDX >= gNextSlot.SlotIndexInLongFrame )
      {
//         CO_PRINT_B_2(DBG_INFO_E,"sync %d restored to %d\r\n", gSyncReference, gPreviousSyncReference);
         //The synch reference update will already have been done.
         gSyncPulse = false;
         gSyncReference = gPreviousSyncReference;
      }
//      CO_PRINT_B_2(DBG_INFO_E,"S=%d A=%d\r\n", gCurrentSlot.SlotIndex, ack_slot);
      //Update the next slot data.  Theres no need to wrap on the short frame.
      gNextSlot.Action = MC_TDM_ACTION_PREPROCESS_E;
      gNextSlot.SlotIndex = gCurrentSlot.SlotIndex + 1;
      gNextSlot.SlotIndexInLongFrame = gCurrentSlot.SlotIndexInLongFrame + 1;
      gNextSlot.SlotInSuperframe = gCurrentSlot.SlotInSuperframe + 1;
      gNextSlot.ShortFrameIndex = gCurrentSlot.ShortFrameIndex;
      gNextSlot.LongFrameIndex = gCurrentSlot.LongFrameIndex;
      gNextSlot.HoppingIndexRACH = gCurrentSlot.HoppingIndexRACH;
      uint32_t wake_up_offset = SLOT_DURATION;
      //We will either wake up in the next slot (DLCCH) if there is something to do, or in the following ACK slot.
      if ( MC_TDM_SLOT_TYPE_DLCCH_E == MC_TDM_GetSlotType(gNextSlot.SlotIndex) )
      {
         // its a DLCCH slot.  Is it active?
         uint32_t dlcch_index = gNextSlot.SlotIndex / 2;
         if ( MC_TDM_DLCCH_SLEEP_E == gDlcchBehaviour[dlcch_index] )
         {
            //The DLCCH slot is not active.  Sleep until the folowing ACK slot.
            wake_up_offset += (SLOT_DURATION + TDM_ACK_SLOT_OFFSET);
            //Advance gNextSlot to the next slot.
            gNextSlot.SlotIndex++;
            gNextSlot.SlotIndexInLongFrame++;
            gNextSlot.SlotInSuperframe++;
         }
      }
      gNextSlot.WakeupTime = (uint16_t)SUM_WRAP(gCurrentSlot.WakeupTime, wake_up_offset, LPTIM_MAX);
      
#ifdef ENABLE_TDM_CALIBRATION
      int32_t rx_done_offset = TDM_DCH_MESSAGE_RXDONE_OFFSET + gAckCadOffset;
#else
//      int32_t rx_done_offset = TDM_DCH_MESSAGE_RXDONE_OFFSET + TDM_ACK_RX_OFFSET;
      int32_t rx_done_offset = TDM_DCH_MESSAGE_RXDONE_OFFSET;
#endif
      gNextSlot.ExpectedRxDoneTime = (uint16_t)SUM_WRAP((int32_t)gNextSlot.WakeupTime, rx_done_offset, LPTIM_MAX);

      // Update the wake-up time so that we wake up for the ACK.
      HAL_LPTIM_Disable_CMPM_Interrupt(&hlptim1);
      HAL_LPTIM_Set_CMP(&hlptim1, gNextSlot.WakeupTime);
      HAL_LPTIM_Enable_CMPM_Interrupt(&hlptim1);
//      CO_PRINT_B_1(DBG_INFO_E,"NS=%d\r\n", gNextSlot.SlotIndex);
//      CO_PRINT_B_4(DBG_INFO_E,"Curr wk=%d, Next wk=%d syncref=%d, LFslot=%d\r\n", gCurrentSlot.WakeupTime, gNextSlot.WakeupTime, gSyncReference, gNextSlot.SlotIndexInLongFrame);

   }
#endif
}

/*************************************************************************************/
/**
* MC_TDM_SetShortFrameActions
*
* Function to set up the slot action for DLCCH and DULCH slots.  The action is set to transmit if
* the current slot number matches the designated slot, which we calculate
* using our node id.
* We also want to set the slot to Rx if it is a slot that our parent node should transmit on.
* For this we calculate using the parent node id.
* Frequency diversity is also used by indexing into the frequency hopping sequence using
* the node ID.
*
* @param - superFrameSlot    The slot number to check against.
*
* @return - None.
*/
void MC_TDM_SetShortFrameActions(const uint32_t superFrameSlot)
{
   uint32_t dlcch_index;
   
   //Parameter range check
   uint32_t slots_per_super_frame = MC_SlotsPerSuperFrame();
   CO_ASSERT_VOID_MSG((slots_per_super_frame > superFrameSlot), "MC_TDM_SetShortFrameActions : superFrameSlot");

   // DLCCH SLOT MANAGEMENT
   /* Default to MC_TDM_DLCCH_SLEEP_E */
   for ( int16_t i = 0; i < TDM_NUMBER_OF_DLCCH_SLOTS; i++)
   {
      gDlcchBehaviour[i] = MC_TDM_DLCCH_SLEEP_E;
   }
   
   //The NCU doesn't listen on DLCCH.  Tx only.
   if ( BASE_NCU_E != gBaseType )
   {
      /*======= set the slot to listen on ============*/
      uint16_t primary_node_id = MC_SMGR_GetPrimaryParentID();
      
      if ( CO_BAD_NODE_ID == primary_node_id )
      {
         /* we have no primary parent, use the tracking node */
         primary_node_id = MC_SYNC_GetSyncNode();
      }
      if ( CO_BAD_NODE_ID != primary_node_id )
      {
         /* we have an uplink source to select the slot and frequency to listen on */
         gDLCCHReceiveSlot = MC_TDM_GetDlcchSlotForNode(primary_node_id);
         gDLCCHReceiveFrequencyIndex = MC_TDM_GetDlcchFrequencyForNode(primary_node_id);
         dlcch_index = gDLCCHReceiveSlot/2;
         gDlcchBehaviour[dlcch_index] = MC_TDM_DLCCH_RX_E;
         MC_TDM_SetSlotType(gDLCCHReceiveSlot, MC_TDM_SLOT_TYPE_DLCCH_E);
      }
      else 
      {
         //There is no parent or tracking node.  Default all DLCCH to listen.
         for ( int16_t i = 0; i < TDM_NUMBER_OF_DLCCH_SLOTS; i++)
         {
            gDlcchBehaviour[i] = MC_TDM_DLCCH_RX_E;
         }
      }
   }
   
   /*======= set the slot to transmit on ============*/
   uint16_t network_address = MC_GetNetworkAddress();
   gDLCCHTransmitSlot = MC_TDM_GetDlcchSlotForNode(network_address);
   gDLCCHTransmitFrequencyIndex = MC_TDM_GetDlcchFrequencyForNode(network_address);
   
   dlcch_index = gDLCCHTransmitSlot/2;

   gDlcchBehaviour[dlcch_index] = MC_TDM_DLCCH_TX_E;
   MC_TDM_SetSlotType(gDLCCHTransmitSlot, MC_TDM_SLOT_TYPE_DLCCH_E);
   
   // DULCH SLOT MANAGEMENT
//   if ( ADDRESS_NCU != network_address )
//   {
      if ( CO_IsDulchShortFrame(superFrameSlot) )
      {
         //Set the DULCH slot
         MC_TDM_SetSlotType( DULCH_SLOT_IDX, MC_TDM_SLOT_TYPE_DULCH_E );
         //Set the ack slot to unused.  It is only activated if there is DULCH activity
         MC_TDM_SetSlotType( (DULCH_SLOT_IDX+2), MC_TDM_SLOT_TYPE_UNUSED_E );
      }
      else 
      {
         MC_TDM_SetSlotType( DULCH_SLOT_IDX, MC_TDM_SLOT_TYPE_SECONDARY_RACH_E );
      }
//   }
//   CO_PRINT_B_2(DBG_INFO_E,"SF TxDlcch=%d RxDlcch=%d\r\n", gDLCCHTransmitSlot, gDLCCHReceiveSlot);
}



/*************************************************************************************/
/**
* MC_TDM_RequestDulchMessage
*
* Called if the next slot is a DULCH slot so that a DULCH message can be queued.
*
* @param - None.
*
* @return - None..
*/
void MC_TDM_RequestDulchMessage(void)
{
   CO_Message_t* pUpdateMsg;
   
   // Is this our DULCH Tx slot?
   uint16_t node_id = MC_GetNetworkAddress();
   if ( CO_IsDulchSlot(node_id, gNextSlot.SlotInSuperframe) )
   {
      pUpdateMsg = ALLOCMESHPOOL;
      if (pUpdateMsg)
      {
         pUpdateMsg->Type = CO_MESSAGE_MAC_EVENT_E;
         MACEventMessage_t rachSlotUpdate;
         rachSlotUpdate.EventType = CO_RACH_CHANNEL_UPDATE_E;
         rachSlotUpdate.SlotType = MC_TDM_SLOT_TYPE_DULCH_E;
         rachSlotUpdate.ProcessSyncMessage = false;
         rachSlotUpdate.SuperframeSlotIndex = gNextSlot.SlotInSuperframe;
         rachSlotUpdate.ShortFrameIndex = gNextSlot.ShortFrameIndex;
         rachSlotUpdate.LongFrameIndex = gNextSlot.LongFrameIndex;
         rachSlotUpdate.SlotIndex = gNextSlot.SlotIndex;
         rachSlotUpdate.TransmitSlot = true;
         memcpy(pUpdateMsg->Payload.PhyDataInd.Data, &rachSlotUpdate, sizeof(MACEventMessage_t));

         osStatus osStat = osMessagePut(MeshQ, (uint32_t)pUpdateMsg, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            FREEMESHPOOL(pUpdateMsg);
         }
      }
   }
}

/*************************************************************************************/
/**
* MC_TDM_GetNextHbSlotFrequency
*
* Return the hopping sequence index for the DCH slot in gNextSlot
*
* @param - None.
*
* @return - uint32_t    The frequency channel.
*/
uint32_t MC_TDM_GetNextHbSlotFrequency(void)
{
   uint32_t frequency_channel = 0;
#ifdef USE_DCH_HOPPING_FORMULA
   
#ifdef USE_DCH_HOPPING_FORMULA_1
   //Hop for each short frame
   frequency_channel = (gCurrentSlot.LongFrameIndex + gCurrentSlot.ShortFrameIndex) % DCH_SEQ_LEN;
#else
#ifdef USE_DCH_HOPPING_FORMULA_2
   //Hop for each Hb slot
   uint16_t node_id = CO_CalculateNodeID(gCurrentSlot.SlotIndexInLongFrame);
   frequency_channel = (gCurrentSlot.LongFrameIndex + node_id) % DCH_SEQ_LEN;
#endif
#endif
#else
   //Original hop for each long frame
   frequency_channel = (gCurrentSlot.LongFrameIndex) % DCH_SEQ_LEN;
#endif
   return frequency_channel;
}

/*************************************************************************************/
/**
* MC_TDM_MissedSlotTestAndCompensate
*
* Called after an NVM clean-up.  Sometimes the TDM slot time expires while the
* interrupts are disabled.  The TDM sleeps for a full cycle of the LPTIM (4s)
* puting the RBU out of sync with the mesh.
* This function checks that the next slot interrupt is not too far in the future
* (i.e. missed the current slot interrupt) and advances the TDM if it is.
*
* @param - None.
*
* @return - None.
*/
void MC_TDM_MissedSlotTestAndCompensate(void)
{
   uint16_t time_now = HAL_LPTIM_ReadCounter(&hlptim1);
   
   //if a wake-up has been missed the delta will be roughly 4 seconds.
   //It should never be more than 7 slots (when it sleeps through its DULCH) so test for 8 slots
   uint32_t max_sleep_time = SLOT_DURATION * 8;
   
   uint32_t time_until_wakeup;
   
   do
   {
      if ( gNextSlot.WakeupTime > time_now )
      {
         time_until_wakeup = gNextSlot.WakeupTime - time_now;
      }
      else 
      {
         time_until_wakeup = ((uint32_t)gNextSlot.WakeupTime + 65536) - time_now;
      }
      
      CO_PRINT_B_3(DBG_INFO_E,"TDM now=%d, wkup=%d wait=%d\r\n", time_now, gNextSlot.WakeupTime, time_until_wakeup );
      
      //check if we missed a wake-up
      if ( time_until_wakeup > max_sleep_time )
      {
         //we missed a wake-up.  Advance the TDM.
         MC_TDM_Advance();
         CO_PRINT_B_0(DBG_ERROR_E,"TDM missed slot. Advancing TDM\r\n");
      }
      
   }while ( time_until_wakeup > max_sleep_time );
}


#ifdef SHOW_HB_WAKEUP_TIME_STAMPS
/*************************************************************************************/
/**
* MC_TDM_ShowFirstWakeupTimes
*
* Show the lptim times recored during the first slot wake-up.
*
* @param - None.
*
* @return - None.
*/
void MC_TDM_ShowFirstWakeupTimes()
{
   uint16_t total_time = SUBTRACT_WRAP(lora_program_end_time, wake_up_time, LPTIM_MAX);
   CO_PRINT_B_5(DBG_INFO_E,"wk1=%d, stby=%d, prg_start=%d, prg_end=%d, total=%d\r\n", wake_up_time, lora_standby_time, lora_program_start_time, lora_program_end_time, total_time);
   first_wake_up_time = wake_up_time;
}

/*************************************************************************************/
/**
* MC_TDM_ShowSecondWakeupTimes
*
* Show the lptim times recored during the second slot wake-up.
*
* @param -  None.
*
* @return - None.
*/
void MC_TDM_ShowSecondWakeupTimes()
{
   uint16_t total_time;
   uint16_t slot_rxtx_offset = SUBTRACT_WRAP(wake_up_time, first_wake_up_time, LPTIM_MAX);
   if ( MC_TDM_IsReceivingDCHSlot() )
   {
      total_time = SUBTRACT_WRAP(lora_rx_time, wake_up_time, LPTIM_MAX);
      CO_PRINT_B_5(DBG_INFO_E,"wk2=%d, stby=%d, rx_start=%d, total=%d, slot_offset=%d\r\n", wake_up_time, lora_standby_time, lora_rx_time, total_time, slot_rxtx_offset);
   }
   else
   {
      total_time = SUBTRACT_WRAP(lora_tx_time, wake_up_time, LPTIM_MAX);
      CO_PRINT_B_5(DBG_INFO_E,"wk2=%d, stby=%d, tx_start=%d, total=%d, slot_offset=%d\r\n", wake_up_time, lora_standby_time, lora_tx_time, total_time, slot_rxtx_offset);
   }
}
#endif

