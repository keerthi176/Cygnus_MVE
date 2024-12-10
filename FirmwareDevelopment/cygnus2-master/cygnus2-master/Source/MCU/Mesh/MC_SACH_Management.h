/*************************************************************************************
*  CONFIDENTIAL between PA and Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2018 PA Consulting Group. All rights reserved.
*  Cambridge Technology Centre
*  Melbourn Royston
*  Herts SG8 6DP UK
*  Tel: <44> 1763 261222
*
***************************************************************************************
*  File        : MC_SACH_Management.h
*
*  Description : Header for the management of SACH messages
*
*************************************************************************************/

#ifndef MC_SACH_MANAGEMENT_H_H
#define MC_SACH_MANAGEMENT_H_H


/* System Include Files
*************************************************************************************/
#include <stddef.h>
#include <stdbool.h>


/* User Include Files
*************************************************************************************/
#include "CO_Message.h"
#include "MC_MAC.h"
/* Public definitions
*************************************************************************************/
#define SACH_MAX_MESSAGES 10

typedef struct
{
   uint16_t Destination;
   ApplicationLayerMessageType_t MessageType;
   uint32_t SubType;
   SendBehaviour_t SendBehaviour;
   CO_Message_t Message;
} SASQueueEntry_t;

typedef struct
{
   uint32_t MessageCount;/* the number of messages queued */
   SASQueueEntry_t channelQueue[SACH_MAX_MESSAGES];
} SASMsgQueue_t;

#define SACH_FRAME_INTERVAL 2



/* Public functions
*************************************************************************************/
void MC_SACH_Initialise( const uint16_t address );
bool MC_SACH_ScheduleMessage( const uint16_t destination, const ApplicationLayerMessageType_t MsgType, const uint32_t SubType,
                              const CO_Message_t* pNewMessage, const SendBehaviour_t SendBehaviour);
bool MC_SACH_UpdateSACHManagement( const uint8_t SlotIndex, const uint8_t ShortFrameIndex, const uint8_t LongFrameIndex );
bool MC_SACH_IsSendSlot( const uint8_t SlotIndex, const uint8_t ShortFrameIndex, const uint8_t LongFrameIndex );
void MC_SACH_RedirectMessagesForNode(const uint16_t node_id);
bool MC_SACH_RerouteMessage( const uint8_t index );
uint32_t MC_SACH_MessageCount(void);

#endif /*MC_SACH_MANAGEMENT_H_H*/
