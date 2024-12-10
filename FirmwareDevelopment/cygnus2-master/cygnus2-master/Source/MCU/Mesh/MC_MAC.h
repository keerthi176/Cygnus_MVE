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
***************************************************************************************
*  File        : MC_MAC.h
*
*  Description : Header for MAC layer of mesh protocol
*
*************************************************************************************/

#ifndef MC_MAC_H
#define MC_MAC_H


/* System Include Files
*************************************************************************************/
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"
#include "CO_Message.h"

/* Public Enumerations
*************************************************************************************/
typedef enum
{
   MC_MAC_MODE_IDLE_E,              /* timer interrupt: disabled transceiver: off */
   MC_MAC_MODE_RX_CONTINUOUS_E,     /* timer interrupt: disabled transceiver: RX */
   MC_MAC_MODE_TDM_SYNC_MASTER_E,   /* timer interrupt: enabled free running TDM */
   MC_MAC_MODE_TDM_SYNC_SLAVE_E,    /* timer interrupt: enabled synchronised TDM */
   MC_MAC_MODE_TDM_SYNC_TEST_E      /* timer interrupt: enabled synchronised TDM processed by test mode */
} MC_MAC_Mode_t;
   

typedef enum
{
   MC_MAC_NOT_RECEIVING_E,
   MC_MAC_RX_CONTINUOUS_E,
   MC_MAC_RX_SINGLE_E,   
} MC_MAC_ReceiveMode_t;

// This enum is used for the power control for the "Shout on Panic" feature, described in CYG2-433.
typedef enum
{
   MC_MAC_NO_TX_E,
   MC_MAC_LOW_POWER_TX_E,
   MC_MAC_HIGH_POWER_TX_E,   
   MC_MAC_MAX_POWER_TX_E,   
   MC_MAC_DYNAMIC_POWER_TX_E,
} MsgToTransmit_t;


/* Public Structures
*************************************************************************************/
typedef uint8_t Channel_t;

typedef struct
{
   uint8_t  actionIdx;
   uint32_t slotInSuperframeIdx;         // In range 0 to (SLOTS_PER_SUPER_FRAME - 1)
   uint8_t  slotIndex;                   // Index of the slot within the short frame (0 to SLOTS_PER_SHORT_FRAME-1 (19) )
   uint8_t  shortFrameIndex;             // Index of the short frame within the long frame (0 to SHORT_FRAMES_PER_LONG_FRAME-1 (135, but 3 during early development) )
   uint8_t  longFrameIndex;              // Index of the long frame within the super frame (0 to LONG_FRAMES_PER_SUPER_FRAME-1 (63) )
   uint16_t slotIndexInLongFrame;        // Index of slot within long frame (0 to (SLOTS_PER_SHORT_FRAME * SHORT_FRAMES_PER_LONG_FRAME) - 1)
   uint8_t  DCHHoppingIndex;             // Index of DCH hopping pattern (0 to DCH_SEQ_LEN-1 (15) )
   uint8_t  DLCCHHoppingIndex;           // Index of DLCCH hopping pattern (0 to RACH_DLCCH_SEQ_LEN-1 (67) )
   uint8_t  PRACHHoppingIndex;           // Index of Primary RACH hopping pattern (0 to RACH_DLCCH_SEQ_LEN-1 (67) )
   uint8_t  SRACHHoppingIndex;           // Index of Secondary RACH hopping pattern (0 to RACH_DLCCH_SEQ_LEN-1 (67) )
} MC_MAC_TDMIndex_t;

typedef struct
{
   int16_t  activateOffset;         /* in clock ticks, relative to time of DCH rx done interrupt */
} MC_MAC_ActionDefinition_t;

typedef struct
{
   uint32_t LoraModulationBandwidth;
   uint8_t InitialFrequencyChannelIdx;
   uint8_t CurrentFrequencyChannelIdx;
   uint32_t LoraSpreadingFactor;
   uint8_t LoraCodingRate;
   uint8_t LoraLowTxPower;
   uint8_t LoraHighTxPower;
} LoraParameters_t;

typedef struct
{
   uint8_t Frametype;
   uint32_t Address;
   uint32_t SystemID;
} ATModeRequest_t;



/* Public Functions Prototypes
*************************************************************************************/
extern ErrorCode_t MC_MAC_Init(const bool isMaster, const uint32_t systemId, const uint32_t address);
uint16_t MC_MAC_GetAddress(void);
uint32_t MC_MAC_ConstructSlotInSuperframeIdx(const uint32_t LongFrameIndex, const uint32_t ShortFrameIndex, const uint32_t SlotIndex);
uint32_t MC_MAC_GetCurrentSlotInShortframeIdx(void);
uint32_t MC_MAC_GetCurrentSlotInLongframeIdx(void);
uint32_t MC_MAC_GetCurrentSlotInSuperframeIdx(void);
bool MC_MAC_IsaTrackingNodeDchSlot(void);
//MC_MAC_SlotType_t MC_MAC_GetSlotTypeFromSlotNumber( const uint16_t slotIndex );
//MC_MAC_SlotBehaviour_t MC_MAC_GetSlotBehaviourFromSlotNumber(const uint32_t slotIndex);
//void MC_MAC_SetDCHSlotBehaviour( const uint16_t address, const MC_MAC_SlotBehaviour_t behaviour );
//MC_MAC_SlotBehaviour_t MC_MAC_GetSlotBehaviour(const MC_MAC_TDMIndex_t * const pTDMIndex);

void MC_MAC_StartTDMSlaveMode(const uint32_t slotInSuperframeIdx, const uint16_t slotRefTime);
void MC_MAC_StartTDMMasterMode(void);
void MC_MAC_StartRxContinuousMode(void);

void MC_MAC_TimerOperation(void);
void MC_MAC_TxDoneOperation(void);
void MC_MAC_RxDoneOperation(const uint8_t * const payload, const uint16_t size, const int16_t rssi, const int8_t snr, const uint16_t mcuRtcValue, const uint8_t freqChanIdx, const int32_t frequencyDeviation);
void MC_MAC_TxTimeoutOperation(void);
void MC_MAC_RxTimeoutErrorOperation(const CO_RxTimeoutError_t rxTimeoutError);

void MC_MAC_PacketReceiveSingle(void);

void MC_MAC_PrepareForNextAction(const bool sendAnAck);

void MC_MAC_HopRadioChannel(const Channel_t frequencyChannelIdx);
void MC_MAC_PacketTxPreProcess(CO_Message_t * const pMessage, const uint8_t size, const uint16_t preambleLen, const bool useLoraCrc, const Channel_t frequencyChannelIdx);
void MC_MAC_PacketRxPreProcessSingle(const uint16_t symbTimeout, const uint16_t preambleLen, const uint8_t payloadLen, const bool useLoraCrc, const Channel_t frequencyChannelIdx);


/* Test mode Functions */
void DoTestModeTRANSMIT(void);
void DoTestModeRECEIVE(void);
void DoTestModeNetworkMonitor(void);

void SetTestModeWakeup(const uint16_t delayTime);
void TellMACWhetherMeshStateIsActive(bool MeshStateIsActive);
void MC_SendMissedHeartbeat(const CO_RxTimeoutError_t rxTimeoutError, const bool ProcessSyncMessage);
void MC_SendFailedDecodeDataMessage(void);
void MC_MAC_DoTestModeSleep(void);
void MC_MAC_TxAtCommand( void );
void MC_MAC_SendAtMode( const ATModeRequest_t * const pRequest );
void MC_MAC_SendAtCommand( void );
bool MC_MAC_ProcessATRequest(const uint8_t* const pMsg);
bool MC_MAC_ProcessPpuModeRequest(const uint32_t request);
void MC_MAC_SendPpuMessage(void);
void MC_MAC_ConfigureRxForHeartbeat( void );
void MC_MAC_ConfigureTxForPpu( void );
void MC_MAC_ConfigureRxForPpu( void );
 
#ifdef TXRX_TIMING_PULSE
void MC_MAC_PacketTransmit(bool pulse);
#else
void MC_MAC_PacketTransmit(void);
#endif

/* Public Constants
*************************************************************************************/

#define BITSHIFT_FOR_DCH_INDEX_PER_SHORTFRAME (2)  // There are four DCH slots per shortframe, requiring two bits to represent the index
#define BITMASK_FOR_DCH_INDEX_WITHIN_SHORTFRAME  (~((~0U) << BITSHIFT_FOR_DCH_INDEX_PER_SHORTFRAME))  // Bitshift for DCH Index per shortframe
 
#define MAX_ACTIONS_PER_SLOT           2

#define MAX_NEXT_ACTION_LOOP           20U

#define TICKS_PER_SLOT                 512        // timer ticks
#define TICKS_PER_SLOT_SHIFT           9
#define SLOT_TICK_MASK                 0x1FF
//#define TICKS_PER_SUPER_FRAME          (TICKS_PER_SLOT * SLOTS_PER_SUPER_FRAME)

#define NCU_DLCCH_TX_SLOT_IDX          0U

#define INVALID_HOPPING_SEQ_INDEX      0xFF

#define MESH_SAS_INDEX                 0  // The Mesh  SAS slot is the 0th in each group of 4 (numbered 0 to 3).
#define ZONE_SAS_INDEX                 1  // The Zone  SAS slot is the 1st in each group of 4 (numbered 0 to 3).
#define CHILD_SAS_INDEX                2  // The Child SAS slot is the 2nd in each group of 4 (numbered 0 to 3).

#define REG_PLL_BW_75KHZ               0x10  // From SX1272 data sheet, register 0x5C, page 107.
#define REG_PLL_BW_150KHZ              0x50  // From SX1272 data sheet, register 0x5C, page 107.
#define REG_PLL_BW_225KHZ              0x90  // From SX1272 data sheet, register 0x5C, page 107.
#define REG_PLL_BW_300KHZ              0xD0  // From SX1272 data sheet, register 0x5C, page 107.


/* Public Variables
*************************************************************************************/
extern LoraParameters_t MC_MAC_LoraParameters;
extern uint8_t MC_MAC_gTestMode;


/* Macros
*************************************************************************************/


#endif // MC_MAC_H
