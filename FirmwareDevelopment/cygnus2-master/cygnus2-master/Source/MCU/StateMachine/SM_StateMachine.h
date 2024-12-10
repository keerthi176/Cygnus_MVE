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
**************************************************************************************
*  File         : SM_StateMachine.h
*
*  Description  : State Machine header file
*
*************************************************************************************/

#ifndef SM_STATE_MACHINE_H
#define SM_STATE_MACHINE_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>



/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "CO_Message.h"
#include "DM_Led.h"


/* Public Structures
*************************************************************************************/ 


/* Public Enumerations
*************************************************************************************/
typedef enum
{
   EVENT_HEARTBEAT_E,
   EVENT_FIRE_SIGNAL_E,
   EVENT_ALARM_SIGNAL_E,
   EVENT_OUTPUT_SIGNAL_E,
   EVENT_ACK_SIGNAL_E,
   EVENT_GENERATE_HEARTBEAT_E,                  //5
   EVENT_GENERATE_FIRE_SIGNAL_E,
   EVENT_GENERATE_ALARM_SIGNAL_E,
   EVENT_GENERATE_OUTPUT_SIGNAL_E,
   EVENT_GENERATE_FAULT_SIGNAL_E,
   EVENT_MAC_EVENT_E,                           //10
   EVENT_ROUTE_ADD_E,
   EVENT_ROUTE_DROP_E,
   EVENT_FAULT_SIGNAL_E,
   EVENT_GENERATE_LOGON_REQ_E,
   EVENT_LOGON_REQ_E,                           //15
   EVENT_GENERATE_COMMAND_REQ_E,
   EVENT_COMMAND_E,
   EVENT_GENERATE_RESPONSE_REQ_E,
   EVENT_RESPONSE_E,
   EVENT_TEST_MESSAGE_E,                        //20
   EVENT_TEST_REPORT_E,
   EVENT_GENERATE_TEST_MESSAGE_REQ_E,
   EVENT_GENERATE_SET_STATE_REQ_E,
   EVENT_SET_STATE_SIGNAL_E,
   EVENT_GENERATE_ROUTE_ADD_RESPONSE_E,         //25
   EVENT_ROUTE_ADD_RESPONSE_E,
   EVENT_GENERATE_ROUTE_ADD_REQUEST_E,
   EVENT_GENERATE_RBU_DISABLE_MESSAGE_E,
   EVENT_RBU_DISABLE_E,
   EVENT_GENERATE_STATUS_INDICATION_E,          //30
   EVENT_STATUS_INDICATION_E,
   EVENT_GENERATE_ROUTE_DROP_REQUEST_E,
   EVENT_APPLICATION_REQUEST_E,
   EVENT_GENERATE_ALARM_OUTPUT_STATE_E,
   EVENT_ALARM_OUTPUT_STATE_E,                  //35
   EVENT_GENERATE_BATTERY_STATUS_E,
   EVENT_BATTERY_STATUS_MESSAGE_E,
   EVENT_GENERATE_PING_REQUEST_E,
   EVENT_PING_REQUEST_E,
   EVENT_GENERATE_ZONE_ENABLE_E,                //40
   EVENT_ZONE_ENABLE_E,
   EVENT_GENERATE_AT_COMMAND_E,
   EVENT_AT_COMMAND_E,
   EVENT_GENERATE_ENTER_AT_MODE_E,
   EVENT_PPU_MODE_E,                         //45
   EVENT_PPU_COMMAND_E,
   EVENT_PPU_RESPONSE_E,
   EVENT_GLOBAL_DELAYS_E,
	EVENT_EXIT_TEST_MODE_E,
   EVENT_MAX_E
} SM_Event_t;


/* Public Functions Prototypes
*************************************************************************************/
void SMInitialise(bool isNCU, uint32_t address, const uint32_t systemId);
void SMHandleEvent(SM_Event_t event, uint8_t *pData);

bool SMCheckDestinationAddress(const uint32_t destAddress, const uint32_t targetZone);

void SM_SendConfirmationToApplication( const uint32_t Handle, const uint32_t ReplacementHandle, const AppConfirmationType_t ConfirmationType, const uint32_t Error );
void SM_SendEventToApplication( const CO_MessageType_t MeshEventType, const ApplicationMessage_t* pEventMessage );
void SM_ActivateStateChange(const uint8_t LongFrameIndex);

/* Public Variables
*************************************************************************************/
extern const DM_LedPatterns_t SM_mesh_led_patterns[STATE_MAX_E];

/* Public Constants
*************************************************************************************/



/* Macros
*************************************************************************************/



#endif // SM_STATE_MACHINE_H
