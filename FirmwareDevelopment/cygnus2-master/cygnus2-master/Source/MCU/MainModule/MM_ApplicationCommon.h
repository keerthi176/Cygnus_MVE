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
*  File         : MM_ApplicationCommon.h
*
*  Description  : Functions that are common to NCU and RBU applications.
*
                  Includes functions for managing the child records at the application level.
*                 This deals with ensuring that each child's output state corresponds
*                 to the last output signal.
*                 Not to be confused with the child management in the session manager,
*                 which is for a different purpose (maintaining mesh links).
*
*************************************************************************************/

#ifndef MM_APP_COMMON_H
#define MM_APP_COMMON_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>

/* User Include Files
*************************************************************************************/
#include "CO_Message.h"

/* Public Constants
*************************************************************************************/
#define RBU_PERIODIC_TIMER_PERIOD_MS 1000U
#define IUO_PERIODIC_TIMER_PERIOD_MS 1000U

/* Public Enumerations
*************************************************************************************/
/* Public Structures
*************************************************************************************/ 
/* Child Output State Correction Types */

typedef struct
{
   uint16_t nodeID;                       /* The child RBU's network ID */
   uint8_t zone;                          /* The child RBU's zone*/
   uint8_t outputChannel;                 /* The last commanded channel */
   uint8_t outputProfile;                 /* The last commanded profile */
   uint8_t outputDuration;                /* The last commanded duration */
   uint8_t outputValue;                   /* The last commanded output value */
   uint8_t lastReportedChannel;           /* The last reported channel of the child RBU */
   uint8_t lastReportedProfile;           /* The last reported channel of the child RBU */
   uint8_t lastReportedDuration;          /* The last reported duration of the child RBU */
   uint8_t lastReportedValue;             /* The last reported channel of the child RBU */
   uint8_t lastReportedSilenceableMask;   /* The last reported channel of the child RBU */
   uint8_t lastReportedUnsilenceableMask; /* The last reported channel of the child RBU */
   uint8_t lastReportedDelayMask;         /* The last reported channel of the child RBU */
   uint32_t stateQueryHandle;             /* The message ID that matches the query that was sent to the child */
   bool isPrimaryChild;                   /* True if this node is the child's primary parent */
   bool isMuted;                          /* True if the child is exempted from output state checks */
} ApplicationChildRecord_t;

typedef struct
{
   uint8_t shortFrameCount;      /* Counts the number of shortframes for delays */
   uint8_t outputSignalCount;
   uint8_t numberOfCorrectionCycles;
   ApplicationChildRecord_t childRecord[MAX_CHILDREN_PER_PARENT];
} ApplicationChildArray_t;


/* Public Functions Prototypes
*************************************************************************************/
uint32_t GetNextHandle(void);
uint8_t GetNextTransactionID(void);
CO_State_t GetCurrentDeviceState(void);
void MM_ApplicationInitialiseChildArray(void);
void MM_ApplicationAddChild(const uint16_t nodeID, const bool isPrimary);
void MM_ApplicationDropChild(const uint16_t nodeID);
int16_t MM_ApplicationGetChildIndexFromHandle(const uint32_t handle);
bool MM_ApplicationChildIsAMemberOfZone(const uint16_t ChildIndex, const uint16_t Zone);
void MM_ApplicationProcessOutputMessage(const uint16_t destination, const uint16_t Zone, const uint8_t OutputChannel, const uint8_t OutputProfile, const uint16_t OutputsActivated, const uint16_t OutputDuration);
void MM_ApplicationUpdateDeviceState(const uint32_t newState);
void StartPeriodicTimer(void);
void MM_SendFireSignal( const uint8_t ruChannelIndex, const CO_RBUSensorType_t sensorType, const uint8_t analogueValue, const bool alarmActive );
void MM_SendAlarmSignal( const uint8_t ruChannelIndex, const CO_RBUSensorType_t sensorType, const uint8_t analogueValue, const bool alarmActive );
void PeriodicTimerCallback(void const *id);
void MM_ConfigureForTestModeSleep(void);
void MM_CheckForRequestedPpuMode(void);
/* Macros
*************************************************************************************/



#endif // MM_APP_COMMON_H
