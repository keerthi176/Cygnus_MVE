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
**************************************************************************************
*  File         : MC_MacConfiguration.h
*
*  Description  : Header for the MAC configuration functions
*
*************************************************************************************/

#ifndef MM_MAC_CONFIG_H
#define MM_MAC_CONFIG_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "CO_ErrorCode.h"
#include "CO_Message.h"

/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/
typedef enum 
{
   FREQUENCY_BAND_865_E,
   FREQUENCY_BAND_915_E,
   FREQUENCY_BAND_MAX_E
} FrequencyBand_t;

/* Public Constants
*************************************************************************************/
#define BIT_SHIFT_FOR_QX_4         (4)       // Bitshift to provide 4 bits of fractional representation

#define ACTIVE_TRANSITION_SACH_DURATION (300) // the time period after going to active mode for fault signals to use the SACH

#define PPU_FREQUENCY_INDEX 10

/* Macros
*************************************************************************************/


/* Public Functions Prototypes
*************************************************************************************/
void MC_InitialiseConfig(const uint32_t short_frames_per_long_frame);
ErrorCode_t MC_SetShortFramesPerLongFrame(const uint32_t short_frames_per_long_frame);
uint8_t MC_ShortFramesPerLongFrame(void);
uint16_t MC_MaxNumberOfNodes(void);
int32_t MC_MaxLfdError(void);
uint32_t MC_SlotsPerLongFrame(void);
uint32_t MC_SlotsPerSuperFrame(void);
int32_t MC_CyclesPerShortFrame(void);
int32_t MC_CyclesPerLongFrame(void);
int32_t MC_Slot0EstimatedOffset(void);
int32_t MC_LongFrameDurationThreshold(void);
uint16_t MC_MaxSfiForStateChange(void);
uint16_t MC_DchSlotsPerLongFrame(void);
uint16_t MC_DlcchHoppingSequenceLength(void);
void MC_ResetTimeActive(void);
void MC_IncrementTimeActive(void);
uint32_t MC_GetTimeActive(void);
void MC_SetProgrammedShortFramesPerLongFrame(const uint32_t short_frames_per_long_frame);
uint32_t MC_GetProgrammedShortFramesPerLongFrame(void);
ErrorCode_t MC_SetMaxRank(const uint32_t maxRank);
uint32_t MC_GetMaxRank(void);
void MC_SetNetworkAddress(const uint16_t network_address);
uint16_t MC_GetNetworkAddress(void);
void MC_SetSystemID(const uint32_t systemID);
uint32_t MC_GetSystemID(void);
void MC_SetSerialNumber(const uint32_t serialNumber);
uint32_t MC_GetSerialNumber(void);
MC_MAC_TestMode_t MC_GetTestMode(void);
void MC_SetTestMode(const MC_MAC_TestMode_t testMode);
bool MC_SetBaseFrequencychannel(const uint32_t frequencyChannel);
uint32_t MC_GetBaseFrequencyChannel(void);
bool MC_SetFrequencyBand(const uint32_t frequencyBand);
FrequencyBand_t MC_GetFrequencyBand(void);
uint32_t MC_GetFrequencyForChannel(const uint32_t channel);
void MC_SetTxPowerHigh(const uint32_t tx_power);
uint32_t MC_GetTxPowerHigh(void);
void MC_SetTxPowerLow(const uint32_t tx_power);
uint32_t MC_GetTxPowerLow(void);
void MC_SetPpuMode(const PpuState_t ppuState);
PpuState_t MC_GetPpuMode(void);

#endif // MM_MAC_CONFIG_H
