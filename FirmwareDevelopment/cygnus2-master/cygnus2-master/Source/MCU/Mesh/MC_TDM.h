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
*  File        : MC_TDM.h
*
*  Description : Header for the Time Division Multiplexing of the mesh protocol
*
*************************************************************************************/

#ifndef MC_TDM_H
#define MC_TDM_H


/* System Include Files
*************************************************************************************/
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"
#include "CO_Message.h"
#include "radio.h"
#include "sx1272.h"
#include "MC_TDM_SlotManagement.h"

//ATTIM=0,88
//ATTIM=11,378


/* Public Constants
*************************************************************************************/
#define TDM_DCH_RX_DONE_LATENCY        34   //     0

#define TDM_DCH_TX_OFFSET              55   // 3.3ms  1
#define TDM_DCH_RX_OFFSET              91   // 5.5ms  2

#define TDM_RACH_TX_OFFSET_DOWNLINK    55  // 3.3ms   3
#define TDM_RACH_TX_OFFSET_UPLINK      55  // 3.3ms   4
#define TDM_RACH_CAD_OFFSET            71  // 4.3ms   5

#define TDM_DLCCH_TX_OFFSET            55  // 3.3ms   6
#define TDM_DLCCH_CAD_OFFSET           105  // 3.66ms  7

#define TDM_ACK_SLOT_OFFSET            100  // 6ms  The delay between the slot start and the first wake-up
#define TDM_ACK_TX_OFFSET               55  // 3.3ms  8
#define TDM_ACK_RX_OFFSET               91  // 3.3ms  9

#ifdef SHOW_SLOT_WAKEUPS
   #define TDM_DCH_MESSAGE_RXDONE_OFFSET  (TDM_DCH_TX_OFFSET + TDM_DCH_RX_DONE_LATENCY + 339U) //Tx offset + DCH msg length in LPTIM ticks (20.68ms) 11
#else
   #define TDM_DCH_MESSAGE_RXDONE_OFFSET (TDM_DCH_TX_OFFSET + TDM_DCH_RX_DONE_LATENCY + 339U) //Tx offset + DCH msg length in LPTIM ticks (20.68ms) 11
#endif

#define TDM_DATA_MESSAGE_RXDONE_OFFSET 711U //data msg length in LPTIM ticks (28.73ms) + TDM_RACH_TX_OFFSET
#define INITIAL_SYNC_CORRECTION 0

#define MC_TDM_WIDE_INITAL_DCH_OFFSET 34     //Wake up 2ms earlier for initial sync node aquisition

#define LONG_FRAME_TICK_TOLERANCE 317 //0.0001% of tick count in ideal long frame (assumes framelength=128)

/* Public Enumerations
*************************************************************************************/

/* Public Structures
*************************************************************************************/


/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t MC_TDM_Init(void);
void MC_TDM_StartTDMSlaveMode(const uint32_t slotInSuperframeIdx, const uint16_t slotRefTime);
void MC_TDM_StartTDMMasterMode(void);
void MC_TDM_StopTDM(void);
bool MC_TDM_Running(void);
void MC_TDM_TimerOperationTdm(void);
uint16_t MC_TDM_GetSlotReferenceTime(void);

void MC_TDM_SetSynchReferenceTime(const uint16_t nextFrameStart, const uint32_t aveFrameLength);
void MC_TDM_StateChangeAtNextLongFrame(void (* callbackFunc)(const uint8_t));
void MC_TDM_StartFrequencyHoppingAtNextLongFrame(void);
void MC_TDM_EnableChannelHopping(void);
void MC_TDM_OpenAckSlot(const bool sendingAck);
void MC_TDM_SetShortFrameActions(const uint32_t superFrameSlot);
void MC_TDM_MissedSlotTestAndCompensate(void);

/* Public Variables
*************************************************************************************/

/* Macros
*************************************************************************************/


#endif // MC_TDM_H
