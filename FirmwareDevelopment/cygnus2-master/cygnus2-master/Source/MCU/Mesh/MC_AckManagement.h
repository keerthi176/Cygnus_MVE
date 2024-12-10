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
*  File        : MC_AckManagement.h
*
*  Description : Header for the management of message acks and message re-transmission
*
*************************************************************************************/

#ifndef MC_ACK_MANAGEMENT_H_H
#define MC_ACK_MANAGEMENT_H_H


/* System Include Files
*************************************************************************************/
#include <stddef.h>
#include <stdbool.h>


/* User Include Files
*************************************************************************************/
#include "CO_Message.h"


/* Public definitions
*************************************************************************************/
typedef enum
{
   ACK_RACHP_E,/* Always define RACHP with the lowest number. The function MC_ACK_GetMessage will prioritise lowest channel first */
   ACK_RACHS_E,
   ACK_DULCH_E,
   ACK_MAX_QUEUE_ID_E
} AcknowledgedQueueID_t;

#define ACK_MAX_UNACKED_MESSAGES MAX_CHILDREN_PER_PARENT
#define ACK_NUMBER_OF_ACKNOWLEDGEMENT_QUEUES ((int32_t)ACK_MAX_QUEUE_ID_E)
#define ACK_RACH_UNKNOWN 3
#define ACK_NUMBER_OF_ACK_CHANNELS 2
#define ACK_NEIGHBOUR_UNKNOWN 0xffffffff
#define ACK_MAX_BACKOFF_EXPONENT 12

/* Public Structures
*************************************************************************************/
typedef struct mbQueueEntry_type
{
   uint32_t MessageHandle;
   bool RouteDiversity;  /* When true, resends alternate between Primary and Secondary parent */
   ApplicationLayerMessageType_t MessageType;
   uint32_t SubType; /* for status and fault MessageType to identify if they are duplicates */
   SendBehaviour_t SendBehaviour;
   uint32_t NumberToSend; // how many times the message should be sent before discarding
   CO_Message_t Message;
} mbQueueEntry_t;

typedef struct
{
   bool ReadyToSend;/*set to true if the queue has a message that needs resending*/
   bool WaitingForACK;
   uint32_t BackoffExp;
   uint32_t BackoffCounter;
   uint32_t MessageCount;/* the number of messages queued */
   uint32_t WaterMark; /* records the highest message count that has been seen */
   mbQueueEntry_t channelQueue[ACK_MAX_UNACKED_MESSAGES];
} UnackedMsgQueue_t;


/* Public functions
*************************************************************************************/
void MC_ACK_SetDuplicateMessageCallback(void (*pfCallback)(const uint32_t DiscardedHandle, const uint32_t NewHandle, const ApplicationLayerMessageType_t MsgType));
void MC_ACK_Initialise( void );
bool MC_ACK_AddMessage( const uint32_t neighbourID, const AcknowledgedQueueID_t channel, const ApplicationLayerMessageType_t MsgType, const uint32_t SubType,
                        const bool MultiHop, const uint32_t MsgHandle, const CO_Message_t* pNewMessage, const bool SendImmediately, const SendBehaviour_t SendBehaviour );
bool MC_ACK_AcknowledgeMessage( const AcknowledgedQueueID_t channelID, const uint16_t ackSourceNode, const bool ackReceived, ApplicationLayerMessageType_t* pMsgType, uint32_t* pMsgHandle );
bool MC_ACK_DiscardMessage( const AcknowledgedQueueID_t channelID, ApplicationLayerMessageType_t* pMsgType, uint32_t* pMsgHandle );
bool MC_ACK_PeekMessage( const AcknowledgedQueueID_t channelID, CO_Message_t* pMessage);
bool MC_ACK_WaitingForACK( const AcknowledgedQueueID_t channelID );
bool MC_ACK_UpdateSlot( const AcknowledgedQueueID_t channelID );
uint32_t MC_ACK_GetRandomDelay( const uint32_t backOffExp );
bool MC_ACK_GetMessage( const AcknowledgedQueueID_t channel,  CO_Message_t* pMessage );
bool MC_ACK_MessageReadyToSend( const AcknowledgedQueueID_t channel );
bool MC_ACK_RescheduleMessage( const AcknowledgedQueueID_t channel, uint32_t* pMsghandle );
bool MC_ACK_MessageWasDiscarded(uint32_t* Messagehandle, ApplicationLayerMessageType_t* MessageType, uint16_t* pMacDestination);
void MC_ACK_UpdateAckExpectedFlags(void);
uint16_t MC_ACK_GetMacDestinationOfCurrentMessage( const AcknowledgedQueueID_t channelID );
ApplicationLayerMessageType_t MC_ACK_GetTypeOfCurrentMessage( const AcknowledgedQueueID_t channelID );
void MC_ACK_RedirectMessagesForNode(const uint16_t node_id);
bool MC_ACK_QueueFull(const AcknowledgedQueueID_t channel);
uint32_t MC_ACK_GetQueueCount(const AcknowledgedQueueID_t channel);
#endif /* MC_ACK_MANAGEMENT_H_H */
