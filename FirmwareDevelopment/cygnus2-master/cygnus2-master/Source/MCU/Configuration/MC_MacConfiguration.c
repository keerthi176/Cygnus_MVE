/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2019 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
*  File         : MC_MacConfiguration.c
*
*  Description  : Source for the MAC configuration functions
*
*************************************************************************************/

/* System Include Files
*************************************************************************************/

/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "main.h"
#include "DM_NVM.h"
#include "DM_LED.h"
#include "MC_MAC.h"
#include "radio.h"
#include "MC_SessionManagement.h"
#include "MC_MacConfiguration.h"

/* Private definitions
*************************************************************************************/
// Primary defintions of the MCU Counter (hardware) and Long Counter (software) sizes.
#define MCU_CTR_LSB_BITS           (16)    // Counter hardware MCU counter value, with range 0 to 65535
#define MCU_CTR_MSB_BITS           (6)     // Long counter is therefore 16 + 6 = 22 bits, allowing total long count duration 256 seconds.
#define MCU_CTR_CYCLE_RATE         (16384)
#define SLOTS_PER_SECOND           (32)

#define CYCLES_PER_SLOT            (MCU_CTR_CYCLE_RATE / SLOTS_PER_SECOND)    // 512

#define SHORT_FRAMES_FOR_STATE_CHANGE  16U


/* Global Varbles
*************************************************************************************/
extern MC_MAC_Mode_t gMode;

/* Private Variables
*************************************************************************************/
static uint16_t gNetworkAddress;
static uint32_t gSerialNumber;
static uint32_t ShortFramesPerLongFrame = MAX_SHORT_FRAMES_PER_LONG_FRAME;
static uint16_t MaxNumberOfNodes= 0;
static uint32_t SlotsPerLongFrame = 0;
static uint32_t SlotsPerSuperFrame = 0;
static int32_t LPTIMCyclesPerShortFrame = 0;
static int32_t LPTIMCyclesPerLongFrame = 0;
static int32_t Slot0EstimatedOffset = 0;
static int32_t LongFrameDurationThreshold = 0;
static uint16_t MaxSfiForStateChange = 0;
static uint16_t DchSlotsPerLongFrame = 0;
static uint16_t DlcchHoppingSequenceLength = 0;
static uint32_t ProgrammedShortFramesPerLongFrame = MAX_SHORT_FRAMES_PER_LONG_FRAME;
static uint32_t gTimeActive = 0; // the time in seconds since the transition to active mode.
static uint32_t MaxRank = MAX_RANK;
static uint32_t SystemID;
static uint32_t TxPowerHigh;
static uint32_t TxPowerLow;
static MC_MAC_TestMode_t gTestMode;
static uint32_t gBaseFrequencyChannel;
static FrequencyBand_t gFrequencyBand;
static PpuState_t gPpuState;


#define NUM_FREQ_CHANS_865 11
#define NUM_FREQ_CHANS_915 11
static uint32_t const frequencyChannels865[NUM_FREQ_CHANS_865] = {865150000, 865450000, 865750000, 866050000, 866350000,
                                                                 866650000, 866950000, 867250000, 867550000, 867850000, 868450000};

static uint32_t const frequencyChannels915[NUM_FREQ_CHANS_915] = {920150000, 920450000, 920750000, 921050000, 921350000,
                                                                 921650000, 921950000, 922250000, 922550000, 922850000, 923450000};

/* Private Functions Prototypes
*************************************************************************************/

/* Functions
*************************************************************************************/

/*****************************************************************************
*  Function:      MC_InitialiseConfig
*  Description:   Initialisation function for the Battery monitor 
*
*  param    short_frames_per_long_frame : The programmed number of short frames to
*                                         make a long frame.
*  return   None.
*
*  Notes:
*****************************************************************************/
void MC_InitialiseConfig(const uint32_t short_frames_per_long_frame)
{
   ShortFramesPerLongFrame = short_frames_per_long_frame;
   MaxNumberOfNodes = ShortFramesPerLongFrame * DCH_SLOTS_PER_SHORT_FRAME;
   SlotsPerLongFrame = ShortFramesPerLongFrame * SLOTS_PER_SHORT_FRAME;
   SlotsPerSuperFrame = SlotsPerLongFrame * LONG_FRAMES_PER_SUPER_FRAME;
   LPTIMCyclesPerShortFrame = CYCLES_PER_SLOT * SLOTS_PER_SHORT_FRAME;
   LPTIMCyclesPerLongFrame = SlotsPerLongFrame * SLOT_DURATION;
   Slot0EstimatedOffset = (((LPTIMCyclesPerLongFrame >> MCU_CTR_LSB_BITS) + 1) << MCU_CTR_LSB_BITS); // 1441792 for 85s LF
   LongFrameDurationThreshold = (LPTIMCyclesPerLongFrame + LPTIMCyclesPerShortFrame) << BIT_SHIFT_FOR_QX_4;
// Set the last short frame index in which a state change can be scheduled for the beginning of the next long frame.
// The state change cannot be scheduled too soon, as the DLCCH must propogate the entire mesh before the state change is activated.
// We allow 10 seconds
   MaxSfiForStateChange = ShortFramesPerLongFrame - SHORT_FRAMES_FOR_STATE_CHANGE;

   DchSlotsPerLongFrame = DCH_SLOTS_PER_SHORT_FRAME * ShortFramesPerLongFrame;

   //Set the DLCCH hopping sequence length to short_frames_per_long_frame/2, but not less than 16.
   DlcchHoppingSequenceLength = short_frames_per_long_frame / 2;
   if ( 16 > DlcchHoppingSequenceLength )
   {
      DlcchHoppingSequenceLength = 16;
   }
   
   uint32_t nvm_value;
   if ( SUCCESS_E == DM_NVMRead(NV_MAXIMUM_RANK_E, (void*)&nvm_value, sizeof(nvm_value)) )
   {
      if ( 0 == nvm_value )
      {
         nvm_value = MAX_RANK;
         DM_NVMWrite(NV_MAXIMUM_RANK_E, (void*)&nvm_value, sizeof(nvm_value));
      }
      MaxRank = nvm_value;
   }
   
   //Read the frequency band
   uint32_t frequency_band;
   if ( SUCCESS_E != DM_NVMRead(NV_FREQUENCY_BAND_E, &frequency_band, sizeof(frequency_band)))
   {
      CO_PRINT_B_0(DBG_ERROR_E,"MAC CFG: Failed to read frequency setting\r\n");
      //default to 865MHz
      gFrequencyBand = FREQUENCY_BAND_865_E;
   }
   else 
   {
      if ( FREQUENCY_BAND_MAX_E > frequency_band )
      {
         gFrequencyBand = (FrequencyBand_t)frequency_band;
      }
      else 
      {
         CO_PRINT_B_0(DBG_ERROR_E,"MAC CFG: Frequency setting in NVM out of range.  Defaulting to 865MHz\r\n");
         gFrequencyBand = FREQUENCY_BAND_865_E;
      }
   }

   //Read the base frequency channel
   uint32_t frequency_channel;
   if ( SUCCESS_E != DM_NVMRead(NV_FREQUENCY_CHANNEL_E, &frequency_channel, sizeof(frequency_channel)))
   {
      CO_PRINT_B_0(DBG_ERROR_E,"MAC CFG: Failed to read the base frequency channel. Defaulting to channel 0\r\n");
      //default to 865MHz
      gBaseFrequencyChannel = 0;
   }
   else 
   {
      uint32_t number_of_channels = NUM_FREQ_CHANS_865;
      if ( FREQUENCY_BAND_915_E == gFrequencyBand )
      {
         number_of_channels = NUM_FREQ_CHANS_915;
      }
      
      if ( number_of_channels > frequency_channel )
      {
         gBaseFrequencyChannel = frequency_channel;
      }
      else 
      {
         CO_PRINT_B_0(DBG_ERROR_E,"MAC CFG: Base Frequency setting in NVM out of range. Defaulting to channel 0\r\n");
         gBaseFrequencyChannel = 0;
      }
   }

   //Read power settings from the config
   if ( SUCCESS_E != DM_NVMRead(NV_TX_POWER_LOW_E, &TxPowerLow, sizeof(uint32_t)) )
   {
      TxPowerLow = TX_LOW_OUTPUT_POWER;
   }
   if ( SUCCESS_E != DM_NVMRead(NV_TX_POWER_HIGH_E, &TxPowerHigh, sizeof(uint32_t)) )
   {
      TxPowerHigh = TX_HIGH_OUTPUT_POWER;
   }
   
   uint32_t serial_number;
   if ( SUCCESS_E == DM_NVMRead(NV_UNIT_SERIAL_NO_E, &serial_number, sizeof(uint32_t)) )
   {
      MC_SetSerialNumber(serial_number);
   }
   
//   CO_PRINT_B_1(DBG_INFO_E,"Frame Length = %d\r\n",ShortFramesPerLongFrame);
}
/*****************************************************************************
*  Function:      MC_SetShortFramesPerLongFrame
*  Description:   Set how many short frames to use in a long frame 
*
*  param          short_frames_per_long_frame   The value to use
*  return         ErrorCode_t                   SUCCESS_E if the new value was accepted
*                                               ERR_INVALID_PARAMETER_E otherwise
*
*  Notes:
*****************************************************************************/
ErrorCode_t MC_SetShortFramesPerLongFrame(const uint32_t short_frames_per_long_frame)
{
   uint32_t new_frame_length = short_frames_per_long_frame;
   // If short_frames_per_long_frame == 0, it's an uninitialised board.  Set to max value.
   if ( 0 == new_frame_length )
   {
      new_frame_length = MAX_SHORT_FRAMES_PER_LONG_FRAME;
   }
   // call init function with the new value
   MC_InitialiseConfig(new_frame_length);

   return SUCCESS_E;
}

/*****************************************************************************
*  Function:      MC_ShortFramesPerLongFrame
*  Description:   Return the number of short frames per long frame 
*
*  param          None
*  return         The 'short frames per long frame' setting
*
*  Notes:
*****************************************************************************/
uint8_t MC_ShortFramesPerLongFrame(void)
{
   return ShortFramesPerLongFrame;
}

/*****************************************************************************
*  Function:      MC_MaxNumberOfNodes
*  Description:   Return the max number of nodes that can fit into the long frame. 
*
*  param          None
*  return         The max number of addressable nodes
*
*  Notes:
*****************************************************************************/
uint16_t MC_MaxNumberOfNodes(void)
{
   return MaxNumberOfNodes;
}

/*****************************************************************************
*  Function:      MC_MaxLfdError
*  Description:   Return the max long frame duration error 
*
*  param          None
*  return         The max long frame duration error 
*
*  Notes:   This is set to allow one LPTIM tick of error per short frame
*****************************************************************************/
int32_t MC_MaxLfdError(void)
{
   return ShortFramesPerLongFrame;
}

/*****************************************************************************
*  Function:      MC_SlotsPerLongFrame
*  Description:   Return the number of slots per long frame 
*
*  param          None
*  return         The 'slots per long frame' setting
*
*  Notes:
*****************************************************************************/
uint32_t MC_SlotsPerLongFrame(void)
{
   return SlotsPerLongFrame;
}

/*****************************************************************************
*  Function:      MC_SlotsPerSuperFrame
*  Description:   Return the number of slots per super frame 
*
*  param          None
*  return         The 'slots per super frame' setting
*
*  Notes:
*****************************************************************************/
uint32_t MC_SlotsPerSuperFrame(void)
{
   return SlotsPerSuperFrame;
}

/*****************************************************************************
*  Function:      MC_CyclesPerShortFrame
*  Description:   Return the number LPTIM ticks per short frame 
*
*  param          None
*  return         The number ticks per short frame
*
*  Notes:
*****************************************************************************/
int32_t MC_CyclesPerShortFrame(void)
{
   return LPTIMCyclesPerShortFrame;
}

/*****************************************************************************
*  Function:      MC_CyclesPerLongFrame
*  Description:   Return the number LPTIM ticks per long frame 
*
*  param          None
*  return         The number ticks per long frame
*
*  Notes:
*****************************************************************************/
int32_t MC_CyclesPerLongFrame(void)
{
   return LPTIMCyclesPerLongFrame;
}

/*****************************************************************************
*  Function:      MC_Slot0EstimatedOffset
*  Description:   Return the number LPTIM ticks per long frame 
*
*  param          None
*  return         The estimated offset
*
*  Notes:
*****************************************************************************/
int32_t MC_Slot0EstimatedOffset(void)
{
   return Slot0EstimatedOffset;
}

/*****************************************************************************
*  Function:      MC_LongFrameDurationThreshold
*  Description:   Return the long frame tolerance in LPTIM ticks
*
*  param          None
*  return         The max deviation (from traking nodes) for a long frame
*
*  Notes:
*****************************************************************************/
int32_t MC_LongFrameDurationThreshold(void)
{
   return LongFrameDurationThreshold;
}

/*****************************************************************************
*  Function:      MC_MaxSfiForStateChange
*  Description:   Return the latest short frame that a state change can be
*                 scheduled
*
*  param          None
*  return         The latest short frame
*
*  Notes:
*****************************************************************************/
uint16_t MC_MaxSfiForStateChange(void)
{
   return MaxSfiForStateChange;
}

/*****************************************************************************
*  Function:      MC_DchSlotsPerLongFrame
*  Description:   Return the number of DCH slots in a long frame
*
*  param          None
*  return         The number of DCH slots
*
*  Notes:
*****************************************************************************/
uint16_t MC_DchSlotsPerLongFrame(void)
{
   return DchSlotsPerLongFrame;
}

/*****************************************************************************
*  Function:      DlcchHoppingSequenceLength
*  Description:   Return the length of the DLCCH frequency hopping sequence
*
*  param          None
*  return         The length of the hopping sequence
*
*  Notes:
*****************************************************************************/
uint16_t MC_DlcchHoppingSequenceLength(void)
{
   return DlcchHoppingSequenceLength;
}

/*****************************************************************************
*  Function:      MC_ResetTimeActive
*  Description:   Reset the time since the transition to active mode
*
*  param          None
*  return         None
*
*  Notes:
*****************************************************************************/
void MC_ResetTimeActive(void)
{
   gTimeActive = 0;
}

/*****************************************************************************
*  Function:      MC_IncrementTimeActive
*  Description:   Increment the time since the transition to active mode
*
*  param          None
*  return         None
*
*  Notes:
*****************************************************************************/
void MC_IncrementTimeActive(void)
{
   gTimeActive++;
}

/*****************************************************************************
*  Function:      MC_GetTimeActive
*  Description:   Return the time since the transition to active mode
*
*  param          None
*  return         The active time in seconds
*
*  Notes:
*****************************************************************************/
uint32_t MC_GetTimeActive(void)
{
   return gTimeActive;
}

/*****************************************************************************
*  Function:      MC_SetProgrammedShortFramesPerLongFrame
*  Description:   Sets the value of short frames per long frame on initial
*                 startup.  
*
*  param          short_frames_per_long_frame  The programmed number of short frames
*  return         None.
*
*  Notes:         Should be called only once after reading the NVM.
*****************************************************************************/
void MC_SetProgrammedShortFramesPerLongFrame(const uint32_t short_frames_per_long_frame)
{
   ProgrammedShortFramesPerLongFrame = short_frames_per_long_frame;
}

/*****************************************************************************
*  Function:      MC_GetProgrammedShortFramesPerLongFrame
*  Description:   Return the programmed short frames per long frame
*
*  param          None
*  return         The number of short frames.
*
*  Notes:
*****************************************************************************/
uint32_t MC_GetProgrammedShortFramesPerLongFrame(void)
{
   return ProgrammedShortFramesPerLongFrame;
}

/*************************************************************************************/
/**
* MC_SetMaxRank
* Set the maximum rank that the sytem can grow to
*
* @param  - maxRank       : the max rank
*
* @return - Error code.
*/
ErrorCode_t MC_SetMaxRank(const uint32_t maxRank)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;
   
   // Read the NVM
   uint32_t nvm_value = maxRank;
   if ( SUCCESS_E == DM_NVMWrite(NV_MAXIMUM_RANK_E, (void*)&nvm_value, sizeof(nvm_value)) )
   {
      MaxRank = maxRank;
      result = SUCCESS_E;
   }

   return result;
}

/*****************************************************************************
*  Function:      MC_GetMaxRank
*  Description:   Return the programmed maximum Rank
*
*  param          None
*  return         The max rank.
*
*  Notes:
*****************************************************************************/
uint32_t MC_GetMaxRank(void)
{
   return MaxRank;
}

/*****************************************************************************
*  Function:      MC_SetNetworkAddress
*  Description:   Set the network address of the device
*
*  param          network_address
*  return         None.
*
*  Notes:
*****************************************************************************/
void MC_SetNetworkAddress(const uint16_t network_address)
{
   gNetworkAddress = network_address;
}

/*****************************************************************************
*  Function:      MC_GetNetworkAddress
*  Description:   Return the newtork address of the device
*
*  param          None
*  return         The network address.
*
*  Notes:
*****************************************************************************/
inline uint16_t MC_GetNetworkAddress(void)
{
   return gNetworkAddress;
}

/*****************************************************************************
*  Function:      MC_SetSystemID
*  Description:   Set the System ID of the device
*
*  param systemID    The system ID
*  return   None.
*
*  Notes:
*****************************************************************************/
void MC_SetSystemID(const uint32_t systemID)
{
   SystemID = systemID;
}

/*****************************************************************************
*  Function:      MC_GetSystemID
*  Description:   Return the System ID of the device
*
*  param    None.
*  return   uint32_t   The System ID.
*
*  Notes:
*****************************************************************************/
uint32_t MC_GetSystemID(void)
{
   return SystemID;
}

/*****************************************************************************
*  Function:      MC_SetSerialNumber
*  Description:   Set the Serial number of the device in 32-bit comapct format
*
*  param serialNumber    The serial number
*  return   None.
*
*  Notes:
*****************************************************************************/
void MC_SetSerialNumber(const uint32_t serialNumber)
{
   gSerialNumber = serialNumber;
}
/*****************************************************************************
*  Function:      MC_GetSerialNumber
*  Description:   Return the Serial number of the device in 32-bit comapct format
*
*  param    None.
*  return   uint32_t   The serial number.
*
*  Notes:
*****************************************************************************/
uint32_t MC_GetSerialNumber(void)
{
   return gSerialNumber;
}

/*************************************************************************************/
/**
* void MC_GetTestMode
*
* Function to return the current test mode.
*
* @param - none.
*
* @return - MC_MAC_TestMode_t - the current value of gTestMode.
*/
MC_MAC_TestMode_t MC_GetTestMode(void)
{
   return gTestMode;
}


/*************************************************************************************/
/**
* void MC_SetTestMode
*
* Function to change the current test mode of the unit.
*
* @param - const MC_MAC_TestMode_t - The test mode to switch to.
*
* @return - void
*/
void MC_SetTestMode(const MC_MAC_TestMode_t testMode)
{
   switch ( testMode )
   {
      case MC_MAC_TEST_MODE_RECEIVE_E:
         gTestMode = testMode;
         SetTestModeWakeup(TEST_MODE_WATCHDOG_PERIOD);
         DoTestModeRECEIVE();
         break;
      case MC_MAC_TEST_MODE_SLEEP_E:
         CO_PRINT_A_0(DBG_INFO_E,"Entering sleep mode\r\n");
         Radio.Sleep();
         gTestMode = testMode;
         SetTestModeWakeup(TEST_MODE_WATCHDOG_PERIOD);
         break;
      case MC_MAC_TEST_MODE_TRANSMIT_E:
         gTestMode = testMode;
         DoTestModeTRANSMIT();
         break;
      case MC_MAC_TEST_MODE_TRANSPARENT_E:/* intentional drop through */
      case MC_MAC_TEST_MODE_MONITORING_E:
         gTestMode = testMode;
         /* Kick start the polling for messages to send*/
         SetTestModeWakeup(TEST_MODE_MONITORING_PERIOD);
         break;
      case MC_MAC_TEST_MODE_NETWORK_MONITOR_E:
         gTestMode = testMode;
         gMode = MC_MAC_MODE_RX_CONTINUOUS_E;
         DoTestModeNetworkMonitor();
         break;
      case MC_MAC_TEST_MODE_OFF_E:/* intentional drop through */
      default:
         if ( MC_MAC_TEST_MODE_OFF_E != gTestMode )
         {
            /* leaving test mode.  Reset the device */
            Error_Handler("Reset on leaving test mode");
         }
         gTestMode = testMode;
         break;
   }
//   HAL_LPTIM_Enable_CMPM_Interrupt(&hlptim1);
}

/*************************************************************************************/
/**
* void MC_SetBaseFrequencychannel
*
* Function to set the base frequency channel that is used.
*
* @param - frequencyChannel      The frequency band.
*
* @return - bool                 TRUE if the setting is accepted
*/
bool MC_SetBaseFrequencychannel(const uint32_t frequencyChannel)
{
   bool result = false;
   uint32_t number_of_channels = NUM_FREQ_CHANS_865;
   
   if ( FREQUENCY_BAND_865_E == gFrequencyBand )
   {
      number_of_channels = NUM_FREQ_CHANS_915;
   }
   
   if ( number_of_channels > frequencyChannel )
   {
      gBaseFrequencyChannel = frequencyChannel;
      result = true;
   }
   
   return result;
}

/*************************************************************************************/
/**
* void MC_GetBaseFrequencyChannel
*
* Function to set the frequency band that is used.
*
* @param - None.
*
* @return - uint32_t    The frequency channel
*/
uint32_t MC_GetBaseFrequencyChannel(void)
{
   return gBaseFrequencyChannel;
}


/*************************************************************************************/
/**
* void MC_SetFrequencyBand
*
* Function to set the frequency band that is used.
*
* @param - frequencyBand - The frequency band.
*
* @return - bool           TRUE if the setting is accepted
*/
bool MC_SetFrequencyBand(const uint32_t frequencyBand)
{
   bool result = false;
   
   if ( FREQUENCY_BAND_MAX_E > frequencyBand )
   {
      gFrequencyBand = (FrequencyBand_t)frequencyBand;
      result = true;
   }
   
   return result;
}

/*************************************************************************************/
/**
* void MC_GetFrequencyBand
*
* Function to get the frequency band that is used.
*
* @param - None.
*
* @return - FrequencyBand_t    The frequency band.
*/
FrequencyBand_t MC_GetFrequencyBand(void)
{
   return gFrequencyBand;
}
/*************************************************************************************/
/**
* void MC_GetFrequencyForChannel
*
* Function to set the frequency band that is used.
*
* @param - channel      The channel to fetch the frequency for.
*
* @return - uint32_t    The frequency of the channel
*/
uint32_t MC_GetFrequencyForChannel(const uint32_t channel)
{
   uint32_t frequency = frequencyChannels865[gBaseFrequencyChannel];
   
   if ( FREQUENCY_BAND_865_E == gFrequencyBand )
   {
      if ( NUM_FREQ_CHANS_865 > channel )
      {
         frequency = frequencyChannels865[channel];
      }
   }
   else if ( FREQUENCY_BAND_915_E == gFrequencyBand )
   {
      if ( NUM_FREQ_CHANS_915 > channel )
      {
         frequency = frequencyChannels915[channel];
      }
      else 
      {
         frequency = frequencyChannels915[gBaseFrequencyChannel];
      }
   }
   return frequency;
}

/*************************************************************************************/
/**
* void MC_SetTxPowerHigh
*
* Set the high power level for radio Tx.
*
* @param - tx_power      The power level to use.
*
* @return - None.
*/
void MC_SetTxPowerHigh(const uint32_t tx_power)
{
   TxPowerHigh = tx_power;
}

/*************************************************************************************/
/**
* void MC_GetTxPowerHigh
*
* Function to get the Tx High Power Level.
*
* @param - None.
*
* @return - uint32_t    The High Power setting
*/
uint32_t MC_GetTxPowerHigh(void)
{
   return TxPowerHigh;
}

/*************************************************************************************/
/**
* void MC_SetTxPowerLow
*
* Set the low power level for radio Tx.
*
* @param - tx_power      The power level to use.
*
* @return - None.
*/
void MC_SetTxPowerLow(const uint32_t tx_power)
{
   TxPowerLow = tx_power;
}

/*************************************************************************************/
/**
* void MC_GetTxPowerLow
*
* Function to get the Tx Low Power Level.
*
* @param - None.
*
* @return - uint32_t    The High Power setting
*/
uint32_t MC_GetTxPowerLow(void)
{
   return TxPowerLow;
}

/*************************************************************************************/
/**
* void MC_SetPpuMode
*
* Record the PPu state of the device.
*
* @param - ppuState      The PPU State.
*
* @return - None.
*/
void MC_SetPpuMode(const PpuState_t ppuState)
{
   CO_PRINT_B_1(DBG_INFO_E,"PPU State set to %d\r\n", ppuState);
   gPpuState = ppuState;
   
   switch ( ppuState )
   {
      case PPU_STATE_DISCONNECTED_E:
         DM_LedPatternRemoveAll();
         DM_LedPatternRequest(LED_PPU_DISCONNECTED_E);
         break;
      case PPU_STATE_CONNECTED_E:
         DM_LedPatternRemoveAll();
         DM_LedPatternRequest(LED_PPU_CONNECTED_E);
         break;
      default:
         DM_LedPatternRemoveAll();
         DM_LedPatternRemove(LED_PPU_CONNECTED_E);
         break;
   }
}

/*************************************************************************************/
/**
* void MC_GetPpuMode
*
* Function to return the current PPU State.
*
* @param - None.
*
* @return - PpuState_t    The current PPU State.
*/
PpuState_t MC_GetPpuMode(void)
{
   return gPpuState;
}
