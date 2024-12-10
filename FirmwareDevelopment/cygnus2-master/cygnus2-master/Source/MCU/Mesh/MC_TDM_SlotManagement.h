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
***************************************************************************************
*  File        : MC_TDM_SlotManagement.h
*
*  Description : Header for the slot mapping for the TDM
*
*************************************************************************************/

#ifndef MC_TDM_SLOTMANAGEMENT_H
#define MC_TDM_SLOTMANAGEMENT_H


/* System Include Files
*************************************************************************************/
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"
#include "CO_Message.h"

/* Public Constants
*************************************************************************************/
#define TDM_NUMBER_OF_DCH_SLOTS 4U
#define TDM_NUMBER_OF_DLCCH_SLOTS 20
#define TDM_FIRST_DLCCH_SLOT 12U
#define TDM_LAST_DLCCH_SLOT  (TDM_FIRST_DLCCH_SLOT + TDM_NUMBER_OF_DLCCH_SLOTS - 1)

#define TDM_MAX_SLOT_FREQUENCY_VARIATION (TDM_NUMBER_OF_DLCCH_SLOTS * NUM_HOPPING_CHANS)

/* Public Enumerations
*************************************************************************************/

typedef enum
{
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_PRIMARY_RACH_E,
   MC_TDM_SLOT_TYPE_SECONDARY_RACH_E,
   MC_TDM_SLOT_TYPE_DULCH_E,
   MC_TDM_SLOT_TYPE_PRIMARY_ACK_E,
   MC_TDM_SLOT_TYPE_SECONDARY_ACK_E,
   MC_TDM_SLOT_TYPE_DULCH_ACK_E,
   MC_TDM_SLOT_TYPE_DCH_E,
   MC_TDM_SLOT_TYPE_UNUSED_E,
} MC_TDM_SlotType_t;

typedef enum
{
   MC_TDM_DCH_SLEEP_E,
   MC_TDM_DCH_TX_E,
   MC_TDM_DCH_RX_E,
   MC_TDM_DCH_TRACKING_E
} MC_TDM_DCHBehaviour_t;

typedef enum
{
   MC_TDM_DLCCH_SLEEP_E,
   MC_TDM_DLCCH_TX_E,
   MC_TDM_DLCCH_RX_E
} MC_TDM_DLCCHBehaviour_t;

typedef enum 
{
   MC_TDM_ACTION_PREPROCESS_E,
   MC_TDM_ACTION_TX_E,
   MC_TDM_ACTION_RX_E,
   MC_TDM_ACTION_NONE_E
}SlotAction_t;

/* Public Structures
*************************************************************************************/


typedef struct
{
   SlotAction_t   Action;                      // What to do when woken up
   uint32_t       SlotInSuperframe;            // In range 0 to (SLOTS_PER_SUPER_FRAME - 1)
   uint8_t        SlotIndex;                   // Index of the slot within the short frame (0 to SLOTS_PER_SHORT_FRAME-1 (19) )
   uint8_t        ShortFrameIndex;             // Index of the short frame within the long frame (0 to SHORT_FRAMES_PER_LONG_FRAME-1 (135, but 3 during early development) )
   uint8_t        LongFrameIndex;              // Index of the long frame within the super frame (0 to LONG_FRAMES_PER_SUPER_FRAME-1 (63) )
   uint16_t       SlotIndexInLongFrame;        // Index of slot within long frame (0 to (SLOTS_PER_SHORT_FRAME * SHORT_FRAMES_PER_LONG_FRAME) - 1)
   uint8_t        HoppingIndexDCH;
   uint8_t        HoppingIndexRACH;
   uint16_t       WakeupTime;
   uint16_t       ExpectedRxDoneTime;
   CO_Message_t*  pMessage;
} MC_TDM_Index_t;



/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t MC_TDM_SlotInit(void);
bool MC_TDM_SetDCHSlotBehaviour( const uint16_t address, const MC_TDM_DCHBehaviour_t behaviour );
bool MC_TDM_IsReceivingDCHSlot(void); 
bool MC_TDM_IsTrackingDCHSlot(void); 
void MC_TDM_SetSlotType( const uint16_t shortFrameSlot, const MC_TDM_SlotType_t slotType );
uint32_t MC_TDM_GetDlcchSlotForNode(const uint16_t node_id);
uint32_t MC_TDM_GetSlotsToAdvance(void);
void MC_TDM_SetShortFrameActions(const uint32_t superFrameSlot);
void MC_TDM_SetSlotsToDefaultType(void);
void MC_TDM_GetCurrentSlot( MC_TDM_Index_t* const slot );
uint32_t MC_TDM_ConstructSlotInSuperframeIdx(const uint32_t LongFrameIndex, const uint32_t ShortFrameIndex, const uint32_t SlotIndex);
uint32_t MC_TDM_GetCurrentSlotInShortframeIdx(void);
uint32_t MC_TDM_GetCurrentSlotInLongframeIdx(void);
uint32_t MC_TDM_GetCurrentSlotInSuperframeIdx(void);
MC_TDM_SlotType_t MC_TDM_GetSlotTypeDefault( const uint16_t shortFrameSlot );
MC_TDM_SlotType_t MC_TDM_GetCurrentSlotType( void );
MC_TDM_SlotType_t MC_TDM_GetSlotType( const uint16_t shortFrameSlot );
bool MC_TDM_DchConfigurationValid(void);
uint16_t MC_TDM_ConvertHeartbeatValueToSlot(const uint16_t hb_value);
uint16_t MC_TDM_ConvertSlotToHeartbeatValue(const uint16_t slot);
/* Public Variables
*************************************************************************************/

/* Macros
*************************************************************************************/


#endif // MC_TDM_SLOTMANAGEMENT_H
