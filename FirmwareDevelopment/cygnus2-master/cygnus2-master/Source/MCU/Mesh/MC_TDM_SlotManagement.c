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
*  File         : MC_TDM_SlotManagement.c
*
*  Description  : Implementation the slot mapping for the TDM
*
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "board.h"
#include "DM_SerialPort.h"
#include "MC_MacConfiguration.h"
#include "MC_TDM_SlotManagement.h"


/* Private macros
*************************************************************************************/
#define CO_BAD_SLOT_INDEX 0xffff


/* Private Function Prototypes
*************************************************************************************/


/* Global Variables
*************************************************************************************/
extern DM_BaseType_t gBaseType;



/* Private Variables
*************************************************************************************/
MC_TDM_SlotType_t gSlotDefinition[SLOTS_PER_SHORT_FRAME];
MC_TDM_DCHBehaviour_t gDchBehaviour[MAX_DEVICES_PER_SYSTEM];
MC_TDM_DLCCHBehaviour_t gDlcchBehaviour[TDM_NUMBER_OF_DLCCH_SLOTS];
MC_TDM_Index_t gCurrentSlot;
MC_TDM_Index_t gNextSlot;
uint32_t gDLCCHReceiveSlot = 0;
uint32_t gDLCCHTransmitSlot = 0;

const uint16_t gDchSlots[TDM_NUMBER_OF_DCH_SLOTS] = {0,10,20,30};

/* Public Variables
*************************************************************************************/
uint16_t gHeartbeatTxSlotNumber = 0;



const MC_TDM_SlotType_t gDefaultSlotBehaviour[SLOTS_PER_SHORT_FRAME] = 
{
   MC_TDM_SLOT_TYPE_DCH_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_PRIMARY_RACH_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_PRIMARY_ACK_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_SECONDARY_RACH_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_SECONDARY_ACK_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_DCH_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_PRIMARY_RACH_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_PRIMARY_ACK_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_SECONDARY_RACH_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_SECONDARY_ACK_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_DCH_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_PRIMARY_RACH_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_PRIMARY_ACK_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_SECONDARY_RACH_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_SECONDARY_ACK_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_DCH_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_PRIMARY_RACH_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_PRIMARY_ACK_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_SECONDARY_RACH_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
   MC_TDM_SLOT_TYPE_SECONDARY_ACK_E,
   MC_TDM_SLOT_TYPE_DLCCH_E,
};

/* Private Functions
*************************************************************************************/

/*****************************************************************************/
/**
*  Function:      MC_TDM_SlotInit
*  Description:   Initialise the TDM slot assignments
*
*  @param   None
*  @return  SUCCESS_E or error code.
*            
*****************************************************************************/
ErrorCode_t MC_TDM_SlotInit(void)
{
   //Default the heartbeat behaviour to RX
   MC_TDM_DCHBehaviour_t default_behaviour = MC_TDM_DCH_RX_E;
   if ( BASE_NCU_E == gBaseType )
   {
      default_behaviour = MC_TDM_DCH_SLEEP_E;
   }
   for ( uint32_t index = 0; index < MAX_DEVICES_PER_SYSTEM; index++)
   {
      gDchBehaviour[index] = default_behaviour;
   }
   //Set the local device behaviour to TX
   uint16_t network_address = MC_GetNetworkAddress();
   gDchBehaviour[network_address] = MC_TDM_DCH_TX_E;
   
   //Calculate the long frame slot for the heartbeat
   gHeartbeatTxSlotNumber = ( ((network_address / DCH_SLOTS_PER_SHORT_FRAME) * SLOTS_PER_SHORT_FRAME) + ((network_address % DCH_SLOTS_PER_SHORT_FRAME) * 10) );
   
   
   //Set the type of each slot in the short frame to default type
   MC_TDM_SetSlotsToDefaultType();
   
   return SUCCESS_E;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_GetSlotsToAdvance
*  Description:   Determine how many slots can be slept through before the next
*                 slot activity.
*
*  @param   None
*  @return  uint32_t    The number of slots to sleep through
*            
*****************************************************************************/
uint32_t MC_TDM_GetSlotsToAdvance(void)
{
   uint32_t slots_to_advance = 1;
   
#ifdef WAKE_FOR_ALL_SLOTS
   return slots_to_advance;
#else
   uint32_t slot_index = (gCurrentSlot.SlotIndex + 1) % SLOTS_PER_SHORT_FRAME;
   uint32_t slots_per_long_frame = MC_SlotsPerLongFrame();
   uint32_t slots_per_super_frame = MC_SlotsPerSuperFrame();
   uint32_t super_frame_slot = (gCurrentSlot.SlotInSuperframe + 1) % slots_per_super_frame;
   bool done = false;
   
   while ( !done )
   {
      if ( MC_TDM_SLOT_TYPE_DLCCH_E == gSlotDefinition[slot_index] )
      {
         uint32_t dlcch_index = slot_index / 2;
         if ( MC_TDM_DLCCH_SLEEP_E != gDlcchBehaviour[dlcch_index] )
         {
            //found a Tx or Rx DLCCH slot
            done = true;
         }
      }
      else if ( MC_TDM_SLOT_TYPE_DCH_E == gSlotDefinition[slot_index] )
      {
         uint32_t node_id = CO_CalculateNodeID(super_frame_slot);
         if ( (CO_BAD_NODE_ID != node_id) && (MC_TDM_DCH_SLEEP_E != gDchBehaviour[node_id]) )
         {
            //found a dch slot that's open
            done = true;
         }
      }
      else if ( MC_TDM_SLOT_TYPE_UNUSED_E != gSlotDefinition[slot_index] )
      {
         done = true;
      }
      
      
      if( !done )
      {
         slot_index = (slot_index + 1) % SLOTS_PER_SHORT_FRAME;
         super_frame_slot = (super_frame_slot + 1) % slots_per_super_frame;
         slots_to_advance++;
      }
   }
//   CO_PRINT_B_2(DBG_INFO_E,"sta cur=%d, adv=%d\r\n", gCurrentSlot.SlotIndex, slots_to_advance);
   return slots_to_advance;
#endif
}


/*************************************************************************************/
/**
* MC_TDM_GetDlcchSlotForNode
*
* Calculate which DLCCH slot the supplied node ID transmits on.
*
* @param - node_id      The node to calculate the DLCCH slot for.
*
* @return - uint32_t    The slot for the supplied node.
*/
uint32_t MC_TDM_GetDlcchSlotForNode(const uint16_t node_id)
{
   uint32_t slot_number = 0;
   
   //Parameter range check
   uint32_t max_devices = MC_MaxNumberOfNodes();
   CO_ASSERT_RET_MSG((max_devices > node_id), 0, "MC_TDM_GetDlcchSlotForNode : node_id");

   if ( TDM_MAX_SLOT_FREQUENCY_VARIATION > node_id )
   {
      slot_number = node_id % TDM_NUMBER_OF_DLCCH_SLOTS;
   }
   else if ( (TDM_MAX_SLOT_FREQUENCY_VARIATION * 2) > node_id )
   {
      slot_number = (node_id + gNextSlot.ShortFrameIndex) % TDM_NUMBER_OF_DLCCH_SLOTS;
   }
   else 
   {
      slot_number = (node_id + (gNextSlot.ShortFrameIndex * 2)) % TDM_NUMBER_OF_DLCCH_SLOTS;
   }
   
   slot_number = (slot_number * 2) + 1;
   
   return slot_number;
}


/*************************************************************************************/
/**
* MC_TDM_ConvertHeartbeatValueToSlot
*
* Convert the heartbeat id value to a short frame slot value.
* The heartbeat has only two nits, giving a range of 0-3.  This function maps
* the DCH slot values in gDchSlots[] to the hb value and returns the short frame slot.
*
* @param - hb_value      The value taken from the heartbeat message.
*
* @return - uint16_t    The short frame slot for the hb value.
*/
uint16_t MC_TDM_ConvertHeartbeatValueToSlot(const uint16_t hb_value)
{
   uint16_t slot = CO_BAD_SLOT_INDEX;
   
   if ( TDM_NUMBER_OF_DCH_SLOTS > hb_value )
   {
      slot = gDchSlots[hb_value];
   }
   
   return slot;
}

/*************************************************************************************/
/**
* MC_TDM_ConvertSlotToHeartbeatValue
*
* Convert the short frame slot value to a heartbeat id value.
* The heartbeat has only two bits, giving a range of 0-3.  This function maps
* the DCH slot values in gDchSlots[] to the slot number and returns the HB value.
*
* @param - slot      The value taken from the heartbeat message.
*
* @return - uint16_t    The short frame slot for the hb value.
*/
uint16_t MC_TDM_ConvertSlotToHeartbeatValue(const uint16_t slot)
{
   uint16_t hb_value = CO_BAD_SLOT_INDEX;
   uint16_t short_frame_slot = (slot % SLOTS_PER_SHORT_FRAME);
   
   for( uint32_t index = 0; index < TDM_NUMBER_OF_DCH_SLOTS; index++ )
   {
      if ( short_frame_slot == gDchSlots[index] )
      {
         hb_value = index;
         break;
      }
   }
   
   return hb_value;
}


/*****************************************************************************/
/**
*  Function:      MC_TDM_IsReceivingDCHSlot
*  Description:   Return true if the current slot is a dch slot
*
*  @param   None
*  @return  True if the slot is a DCH.
*            
*****************************************************************************/
bool MC_TDM_IsReceivingDCHSlot(void)
{
   bool is_rx_dch = false;
   uint16_t node_id = CO_CalculateNodeID(gCurrentSlot.SlotInSuperframe);
   if ( CO_BAD_NODE_ID != node_id )
   {
      if ( MC_TDM_DCH_RX_E == gDchBehaviour[node_id] )
      {
         is_rx_dch = true;
      }
   }
   return is_rx_dch;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_IsTrackingDCHSlot
*  Description:   Return true if the current slot is a tracking node dch slot
*
*  @param   None
*  @return  True if the slot is a tracking DCH.
*            
*****************************************************************************/
bool MC_TDM_IsTrackingDCHSlot(void)
{
   bool is_trk = false;

   if ( MC_TDM_SLOT_TYPE_DCH_E == gSlotDefinition[gCurrentSlot.SlotIndex] )
   {
      uint32_t node_id = CO_CalculateNodeID(gCurrentSlot.SlotInSuperframe);
      if ( (CO_BAD_NODE_ID != node_id) && (MC_TDM_DCH_TRACKING_E == gDchBehaviour[node_id]) )
      {
         is_trk = true;
      }
   }
   return is_trk;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_GetCurrentSlot
*  Description:   return the details of the current TDM slot
*
*  @param   slot  [OUT] Pointer to the structure to populate.
*  @return  None.
*            
*****************************************************************************/
void MC_TDM_GetCurrentSlot( MC_TDM_Index_t* const slot )
{
   if ( slot )
   {
      slot->Action = gCurrentSlot.Action;
      slot->HoppingIndexDCH = gCurrentSlot.HoppingIndexDCH;
      slot->HoppingIndexRACH = gCurrentSlot.HoppingIndexRACH;
      slot->LongFrameIndex = gCurrentSlot.LongFrameIndex;
      slot->ShortFrameIndex = gCurrentSlot.ShortFrameIndex;
      slot->SlotIndex = gCurrentSlot.SlotIndex;
      slot->SlotIndexInLongFrame = gCurrentSlot.SlotIndexInLongFrame;
      slot->SlotInSuperframe = gCurrentSlot.SlotInSuperframe;
      slot->ExpectedRxDoneTime = gCurrentSlot.ExpectedRxDoneTime;
      slot->pMessage = gCurrentSlot.pMessage;
   }
}


/*****************************************************************************/
/**
*  Function:      MC_TDM_GetCurrentSlotInShortframeIdx
*  Description:   Return the current short frame
*
*  @param   None.
*  @return  uint32_t    The short frame index.
*            
*****************************************************************************/
uint32_t MC_TDM_GetCurrentSlotInShortframeIdx(void)
{
   return gCurrentSlot.SlotIndex;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_GetCurrentSlotInLongframeIdx
*  Description:   Return the slot index in the long frame
*
*  @param   None.
*  @return  uint32_t    The slot index into the long frame frame.
*            
*****************************************************************************/
uint32_t MC_TDM_GetCurrentSlotInLongframeIdx(void)
{
   return gCurrentSlot.SlotIndexInLongFrame;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_GetCurrentSlotInSuperframeIdx
*  Description:   Return the slot index in the super frame
*
*  @param   None.
*  @return  uint32_t    The slot index into the super frame frame.
*            
*****************************************************************************/
uint32_t MC_TDM_GetCurrentSlotInSuperframeIdx(void)
{
   return gCurrentSlot.SlotInSuperframe;
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_GetCurrentSlotType
*  Description:   Return the current slot type
*
*  @param   None.
*  @return  MC_TDM_SlotType_tuint32_t    The slot type.
*            
*****************************************************************************/
MC_TDM_SlotType_t MC_TDM_GetCurrentSlotType( void )
{
   return gSlotDefinition[gCurrentSlot.SlotIndex];
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_GetSlotType
*  Description:   Return the slot type for a specifies slot number
*
*  @param   shortFrameSlot               The short frame slot index
*  @return  MC_TDM_SlotType_tuint32_t    The slot type.
*            
*****************************************************************************/
MC_TDM_SlotType_t MC_TDM_GetSlotType( const uint16_t shortFrameSlot )
{
   MC_TDM_SlotType_t slot_type = MC_TDM_SLOT_TYPE_UNUSED_E;
   //Parameter range check
   if ( SLOTS_PER_SHORT_FRAME > shortFrameSlot )
   {
      slot_type = gSlotDefinition[shortFrameSlot];
   }
   return slot_type; 
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_SetSlotType
*  Description:   Set the slot type for a specifies slot number
*
*  @param   shortFrameSlot               The short frame slot index
*  @param  MC_TDM_SlotType_tuint32_t    The slot type.
*
*  @return  None.
*            
*****************************************************************************/
void MC_TDM_SetSlotType( const uint16_t shortFrameSlot, const MC_TDM_SlotType_t slotType )
{
   //Parameter range check
   if ( SLOTS_PER_SHORT_FRAME > shortFrameSlot )
   {
      gSlotDefinition[shortFrameSlot] = slotType;
   } 
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_GetSlotTypeDefault
*  Description:   Return the default slot type (unchanged by dynamic disablement)
*                 for a specifies slot number
*
*  @param   shortFrameSlot               The short frame slot index
*  @return  MC_TDM_SlotType_tuint32_t    The slot type.
*            
*****************************************************************************/
MC_TDM_SlotType_t MC_TDM_GetSlotTypeDefault( const uint16_t shortFrameSlot )
{
   MC_TDM_SlotType_t slot_type = MC_TDM_SLOT_TYPE_UNUSED_E;
   //Parameter range check
   if ( SLOTS_PER_SHORT_FRAME > shortFrameSlot )
   {
      slot_type = gDefaultSlotBehaviour[shortFrameSlot];
   }
   return slot_type; 
}

/*****************************************************************************/
/**
*  Function:      MC_TDM_SetDCHSlotBehaviour
*  Description:   Set the behaviour of a DCH slot
*
*  @param   address     The node ID to map to the DCH slot.
*  @param   behaviour   The behaviour to set.
*  @return  None.
*            
*****************************************************************************/
bool MC_TDM_SetDCHSlotBehaviour( const uint16_t address, const MC_TDM_DCHBehaviour_t behaviour )
{
   bool updated = false;
   
   //Parameter range check
   if ( (MAX_DEVICES_PER_SYSTEM > address) && (MC_TDM_DCH_TRACKING_E  >= behaviour) )
   {
      gDchBehaviour[address] = behaviour;
      updated = true;
   }
   return updated;
}

/*************************************************************************************/
/**
* MC_TDM_DchConfigurationValid
*
* Called each long frame to ensure that we have at least one DCH slot configured to receive.
*
* @param - None.
*
* @return - True if at least one heartbeat slot is configured to receive.
*/
bool MC_TDM_DchConfigurationValid(void)
{
   bool valid = false;
   
   for ( uint32_t index = 0; index < MAX_DEVICES_PER_SYSTEM; index++ )
   {
      if ( (MC_TDM_DCH_RX_E == gDchBehaviour[index]) ||
           (MC_TDM_DCH_TRACKING_E == gDchBehaviour[index]) )
      {
         valid = true;
         break;//no need to search any further
      }
   }
   return valid;
}


/*************************************************************************************/
/**
* MC_TDM_SetSlotsToDefaultType
*
* Set the TDM slots for default behaviour.
*
* @param - None.
*
* @return - None..
*/
void MC_TDM_SetSlotsToDefaultType(void)
{
   gSlotDefinition[0] = gDefaultSlotBehaviour[0];
   gSlotDefinition[1] = gDefaultSlotBehaviour[1];
   gSlotDefinition[2] = gDefaultSlotBehaviour[2];
   gSlotDefinition[3] = gDefaultSlotBehaviour[3];
   gSlotDefinition[4] = gDefaultSlotBehaviour[4];
   gSlotDefinition[5] = gDefaultSlotBehaviour[5];
   gSlotDefinition[6] = gDefaultSlotBehaviour[6];
   gSlotDefinition[7] = gDefaultSlotBehaviour[7];
   gSlotDefinition[8] = gDefaultSlotBehaviour[8];
   gSlotDefinition[9] = gDefaultSlotBehaviour[9];
   gSlotDefinition[10] = gDefaultSlotBehaviour[10];
   gSlotDefinition[11] = gDefaultSlotBehaviour[11];
   gSlotDefinition[12] = gDefaultSlotBehaviour[12];
   gSlotDefinition[13] = gDefaultSlotBehaviour[13];
   gSlotDefinition[14] = gDefaultSlotBehaviour[14];
   gSlotDefinition[15] = gDefaultSlotBehaviour[15];
   gSlotDefinition[16] = gDefaultSlotBehaviour[16];
   gSlotDefinition[17] = gDefaultSlotBehaviour[17];
   gSlotDefinition[18] = gDefaultSlotBehaviour[18];
   gSlotDefinition[19] = gDefaultSlotBehaviour[19];
   gSlotDefinition[20] = gDefaultSlotBehaviour[20];
   gSlotDefinition[21] = gDefaultSlotBehaviour[21];
   gSlotDefinition[22] = gDefaultSlotBehaviour[22];
   gSlotDefinition[23] = gDefaultSlotBehaviour[23];
   gSlotDefinition[24] = gDefaultSlotBehaviour[24];
   gSlotDefinition[25] = gDefaultSlotBehaviour[25];
   gSlotDefinition[26] = gDefaultSlotBehaviour[26];
   gSlotDefinition[27] = gDefaultSlotBehaviour[27];
   gSlotDefinition[28] = gDefaultSlotBehaviour[28];
   gSlotDefinition[29] = gDefaultSlotBehaviour[29];
   gSlotDefinition[30] = gDefaultSlotBehaviour[30];
   gSlotDefinition[31] = gDefaultSlotBehaviour[31];
   gSlotDefinition[32] = gDefaultSlotBehaviour[32];
   gSlotDefinition[33] = gDefaultSlotBehaviour[33];
   gSlotDefinition[34] = gDefaultSlotBehaviour[34];
   gSlotDefinition[35] = gDefaultSlotBehaviour[35];
   gSlotDefinition[36] = gDefaultSlotBehaviour[36];
   gSlotDefinition[37] = gDefaultSlotBehaviour[37];
   gSlotDefinition[38] = gDefaultSlotBehaviour[38];
   gSlotDefinition[39] = gDefaultSlotBehaviour[39];
}

