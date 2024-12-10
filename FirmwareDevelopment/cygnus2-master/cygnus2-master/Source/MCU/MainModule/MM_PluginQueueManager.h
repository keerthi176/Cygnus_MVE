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
*  File         : MM_PluginQueueManager.h
*
*  Description  : Manages the queue for Plugin mesages waiting to be sent.
*
*************************************************************************************/

#ifdef USE_NEW_HEAD_INTERFACE

#ifndef MM_PLUGIN_QUEUE_MGR_H
#define MM_PLUGIN_QUEUE_MGR_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>



/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"
#include "CO_Message.h"
#include "MM_PluginInterfaceTask.h"

/* Public Constants
*************************************************************************************/
#define PLUGIN_QUEUE_DATA_SIZE 24
#define PLUGIN_POOL_SIZE 15
#define PLUGIN_Q_SIZE    15

/* Public Structures
*************************************************************************************/ 

/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t MM_PLUGINQ_Initialise(void);
ErrorCode_t MM_PLUGINQ_Push(const PluginMessage_t* message);
ErrorCode_t MM_PLUGINQ_Pop(const PluginMessage_t* message);
ErrorCode_t MM_PLUGINQ_Discard(void);
uint32_t MM_PLUGINQ_MessageCount(void);
uint32_t MM_PLUGINQ_LostMessageCount(void);
void MM_PLUGINQ_ResetLostMessageCount(void);
void MM_PLUGINQ_ResetMessageQueue(void);

/* Macros
*************************************************************************************/



#endif // MM_CIE_QUEUE_MGR_H

#endif //USE_NEW_HEAD_INTERFACE
