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
*  File         : MC_STATE_StateManagement.c
*
*  Description  : Source file for managing the mesh state and device state
*
*
*************************************************************************************/

/* System Include Files
*************************************************************************************/
#include <inttypes.h>       // Required for C99 fixed size integers.

/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "board.h"
#include "DM_LED.h"
#include "MC_MacConfiguration.h"
#include "MC_TDM.h"
#include "MC_SyncAlgorithm.h"
#include "MC_MeshFormAndHeal.h"
#include "SM_StateMachine.h"
#include "MC_StateManagement.h"

/* Macros
*************************************************************************************/

/* Global Variables
*************************************************************************************/
extern DM_BaseType_t gBaseType;

/* Private Variables
*************************************************************************************/
static CO_State_t gCurrentMeshState;
static CO_State_t gCurrentDeviceState;
static CO_State_t gScheduledDeviceState;
static MeshAdvanceState_t gMeshAdvanceState;

/* Private Functions Prototypes
*************************************************************************************/
static void MC_STATE_AdvanceStateNCU(void);
static void MC_STATE_AdvanceStateRBU(void);

/* Public Functions Definitions
*************************************************************************************/

/*************************************************************************************/
/**
* MC_STATE_Initialise
* Initialise the Mesh and Device states.
*
* @param - None.
*
* @return - None.
*/
void MC_STATE_Initialise(void)
{
   gCurrentMeshState = STATE_IDLE_E;
   gCurrentDeviceState = STATE_IDLE_E;
   gScheduledDeviceState = STATE_MAX_E;
   
   gMeshAdvanceState = MC_STATE_WAITING_FOR_SYNC_E;
   
}

/*************************************************************************************/
/**
* MC_STATE_AdvanceState
* Advance the device state to the next level.
*
* @param - None
*
* @return - ErrorCode_t    SUCCESS_E or error code.
*/
void MC_STATE_AdvanceState(void)
{
   if ( BASE_NCU_E == gBaseType )
   {
      MC_STATE_AdvanceStateNCU();
   }
   else 
   {
      MC_STATE_AdvanceStateRBU();
   }
}

/*************************************************************************************/
/**
* MC_STATE_AdvanceStateRBU
* Manage the state machine for the NCU.
*
* @param - None.
*
* @return - None.
*/
void MC_STATE_AdvanceStateNCU(void)
{
   switch ( gMeshAdvanceState )
   {
      case MC_STATE_WAITING_FOR_SYNC_E:
         if ( STATE_CONFIG_SYNC_E == gCurrentMeshState )
         {
            MC_STATE_SetDeviceState(STATE_CONFIG_SYNC_E);
            MC_STATE_ScheduleStateChange(STATE_NOT_DEFINED_E, true);
            if ( false == MC_TDM_Running() )
            {
               //moving to sync mode.  Start the TDM
               MC_TDM_StartTDMMasterMode();
            }
            //The NCU advances as soon as the mesh state is set
            gMeshAdvanceState = MC_STATE_WAITING_FOR_FORMING_E;
         }
         break;
      case MC_STATE_WAITING_FOR_FORMING_E:
         if ( STATE_CONFIG_FORM_E == gCurrentMeshState )
         {
            //The NCU advances as soon as the mesh state is set
            MC_STATE_ScheduleStateChange(STATE_CONFIG_FORM_E, true);
            gMeshAdvanceState = MC_STATE_WAITING_FOR_ACTIVE_E;
         }
         break;
      case MC_STATE_WAITING_FOR_ACTIVE_E:
         if ( STATE_ACTIVE_E == gCurrentMeshState )
         {
            //The NCU advances as soon as the mesh state is set
            if ( STATE_ACTIVE_E != gCurrentDeviceState )
            {
               if ( (STATE_ACTIVE_E != gScheduledDeviceState) )
               {
                  MC_STATE_ScheduleStateChange(STATE_ACTIVE_E, true);
               }
            }
            else 
            {
               MC_TDM_EnableChannelHopping();
               gMeshAdvanceState = MC_STATE_ACTIVE_E;
            }
         }
         break;
      case MC_STATE_ACTIVE_E:
         if ( STATE_ACTIVE_E != gCurrentMeshState )
         {
            //The NCU reverts to the initial state if the mesh isn't active
            gMeshAdvanceState = MC_STATE_WAITING_FOR_SYNC_E;
         }
         break;
      default:
         break;
   }
}

/*************************************************************************************/
/**
* MC_STATE_AdvanceStateRBU
* Manage the state machine for the RBU.
*
* @param - None.
*
* @return - None.
*/
void MC_STATE_AdvanceStateRBU(void)
{
   
   switch ( gMeshAdvanceState )
   {
      case MC_STATE_WAITING_FOR_SYNC_E:
         if ( STATE_CONFIG_SYNC_E <= gCurrentMeshState )
         {
            //RBUs wait until frequency lock is achieved and the mesh state is advanced
            if ( MC_SYNC_SyncLocked() )
            {
               //Wait for the mesh state to go to forming
               if ( STATE_CONFIG_FORM_E <= gCurrentMeshState )
               {
                  MC_STATE_ScheduleStateChange(STATE_CONFIG_FORM_E, true);
                  gMeshAdvanceState = MC_STATE_WAITING_FOR_FORMING_E;
               }
            }
         }
         break;
      case MC_STATE_WAITING_FOR_FORMING_E:
         if ( STATE_CONFIG_FORM_E <= gCurrentDeviceState )
         {
            //The RBU waits for parent selection before moving to waiting for active
            if ( MC_SMGR_GetNumberOfParents() )
            {
               //check that the mesh has gone active
               gMeshAdvanceState = MC_STATE_WAITING_FOR_ACTIVE_E;
            }
         }
         break;
      case MC_STATE_WAITING_FOR_ACTIVE_E:
         if ( STATE_ACTIVE_E == gCurrentMeshState )
         {
            //The NCU advances as soon as the mesh state is set
            gMeshAdvanceState = MC_STATE_ACTIVE_E;
            MC_TDM_EnableChannelHopping();
         }
         break;
      case MC_STATE_ACTIVE_E:
         if ( STATE_ACTIVE_E != gCurrentMeshState )
         {
            //The NCU reverts to the initial state if the mesh isn't active
            gMeshAdvanceState = MC_STATE_WAITING_FOR_SYNC_E;
         }
         break;
      default:
         break;
   }
}

/*************************************************************************************/
/**
* MC_STATE_ScheduleStateChange
* Schedules a change of device state at the start of the next long frame.
*
* @param - nextState       The state to schedule
* @param - synchronised    True if change should occur on next long frame. False for immediate.
*
* @return - ErrorCode_t    SUCCESS_E or error code.
*/
ErrorCode_t MC_STATE_ScheduleStateChange(const CO_State_t nextState, const bool synchronised)
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   
   if ( STATE_NOT_DEFINED_E >= nextState )
   {
      if ( gScheduledDeviceState != nextState )
      {
         gScheduledDeviceState = nextState;
         if ( synchronised )
         {
            CO_PRINT_A_1(DBG_INFO_E, "Scheduling state change to %d\r\n", nextState);
            MC_TDM_StateChangeAtNextLongFrame(MC_STATE_ActivateStateChange);
         }
      }
      result = SUCCESS_E;
   }
   
   return result;
}

/*************************************************************************************/
/**
* MC_STATE_GetScheduledState
* Gets the current scheduled state.
*
* @param - None.
*
* @return - the mesh state
*/
CO_State_t MC_STATE_GetScheduledState(void)
{
   return gScheduledDeviceState;
}

/*************************************************************************************/
/**
* MC_STATE_SetMeshState
* Sets the current mesh state.
*
* @param - state            The state to set.
*
* @return - ErrorCode_t    SUCCESS_E or ERR_OUT_OF_RANGE_E.
*/
ErrorCode_t MC_STATE_SetMeshState(const CO_State_t state)
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   
   if ( STATE_MAX_E > state )
   {
      gCurrentMeshState = state;
      if ( STATE_ACTIVE_E == state )
      {
         MC_TDM_EnableChannelHopping();
      }
      CO_PRINT_B_1(DBG_INFO_E, "Current Mesh State set to %d\r\n", state);
      result = SUCCESS_E;
   }
   
   return result;
}

/*************************************************************************************/
/**
* MC_STATE_GetMeshState
* Gets the current mesh state.
*
* @param - None.
*
* @return - the mesh state
*/
CO_State_t MC_STATE_GetMeshState(void)
{
   return gCurrentMeshState;
}

/*************************************************************************************/
/**
* MC_STATE_SetDeviceState
* Sets the current device state.
*
* @param - state            The state to set.
*
* @return - ErrorCode_t    SUCCESS_E or error code.
*/
ErrorCode_t MC_STATE_SetDeviceState(const CO_State_t state)
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   
   if ( STATE_TEST_MODE_E != gCurrentDeviceState )
   {
      if ( STATE_MAX_E > state )
      {
         if ( (STATE_SLEEP_MODE_E != gCurrentDeviceState) && (gCurrentDeviceState > state) )
         {
            //MC_TDM_StopTDM();
            //TODO: purge session mgr and sync module
            
            CO_PRINT_B_0(DBG_INFO_E,"Received command to set device state backwards.\r\n");
            MC_SMGR_DelayedReset("Rx'd cmd to set STATE back");
         }

         gCurrentDeviceState = state;
         CO_PRINT_B_1(DBG_INFO_E,"Device State set to %d.\r\n", gCurrentDeviceState);

         /* inform the application that we have changed state */
         ApplicationMessage_t appMessage;
         appMessage.MessageType = APP_MSG_TYPE_STATUS_SIGNAL_E;
         CO_StatusIndicationData_t statusEvent;
         statusEvent.Event = CO_MESH_EVENT_STATE_CHANGE_E;
         statusEvent.EventNodeId = MC_GetNetworkAddress();
         statusEvent.EventData = gCurrentDeviceState;
         statusEvent.DelaySending = true;
         memcpy(appMessage.MessageBuffer, &statusEvent, sizeof(CO_StatusIndicationData_t));
         SM_SendEventToApplication( CO_MESSAGE_MESH_STATUS_EVENT_E, &appMessage );

         //Set the LED pattern
         switch ( gCurrentDeviceState )
         {
            case STATE_CONFIG_SYNC_E:
               DM_LedPatternRemove(LED_MESH_SLEEP1_E);
               DM_LedPatternRemove(LED_MESH_SLEEP2_E);
               DM_LedPatternRemove(LED_MESH_IDLE_E);
               DM_LedPatternRequest(LED_MESH_STATE_CONFIG_SYNC_E);
               break;
            case STATE_CONFIG_FORM_E:
               DM_LedPatternRemove(LED_MESH_STATE_CONFIG_SYNC_E);
               DM_LedPatternRequest(LED_MESH_STATE_CONFIG_FORM_E);
               break;
            case STATE_ACTIVE_E:
               MC_TDM_EnableChannelHopping();
               DM_LedPatternRemove(LED_MESH_STATE_CONFIG_FORM_E);
               DM_LedPatternRequest(LED_MESH_STATE_ACTIVE_E); //this will self cancel
               if (BASE_NCU_E != gBaseType)
               {
                  DM_LedPatternRequest(LED_MESH_CONNECTED_E);    //This will then come forward and be cancelled when the last DULCH has gone.
               }
               break;
            default:
               break;
         }
         result = SUCCESS_E;
      }
   }
   else 
   {
      result = SUCCESS_E;
   }
   
   
   return result;
}

/*************************************************************************************/
/**
* MC_STATE_GetDeviceState
* Gets the current device state.
*
* @param - None.
*
* @return - the mesh state
*/
CO_State_t MC_STATE_GetDeviceState(void)
{
   return gCurrentDeviceState;
}


/*************************************************************************************/
/**
* MC_STATE_ActivateStateChange
* Callback function, called by the TDM when a new longframe begins.
*
* @param - LongFrameIndex   The long frame number.
*
* @return - None.
*/
void MC_STATE_ActivateStateChange(const uint8_t LongFrameIndex)
{
   if ( BASE_NCU_E == gBaseType )
   {
      MC_STATE_SetMeshState(gScheduledDeviceState);
   }
   MC_STATE_SetDeviceState(gScheduledDeviceState);
   MC_STATE_AdvanceState();
   MC_STATE_ScheduleStateChange( STATE_NOT_DEFINED_E, false);
}

/*************************************************************************************/
/**
* MC_STATE_ReadyForStateChange
* Checks whether the device is ready to move to the requested state.
*
* @param - requestedState     The state to check for
*
* @return - bool              True if the device is ready.
*/
bool MC_STATE_ReadyForStateChange(const CO_State_t requestedState)
{
   bool ready_state = false;
   
   switch ( requestedState )
   {
      case STATE_IDLE_E:
         ready_state = true; //This will force a device restart.
         break;
      case STATE_CONFIG_SYNC_E:
         if ( STATE_IDLE_E == MC_STATE_GetDeviceState() )
         {
            ready_state = true;
         }
         break;
      case STATE_CONFIG_FORM_E:
         if ( STATE_CONFIG_SYNC_E == MC_STATE_GetDeviceState() &&
              MC_MFH_InitialTrackingNodeActive() )
         {
            ready_state = true;
         }
         break;
      case STATE_ACTIVE_E:
         if ( STATE_CONFIG_FORM_E == MC_STATE_GetDeviceState() &&
              MC_SMGR_ParentSessionIsActive() )
         {
            ready_state = true;
         }
         break;
      case STATE_TEST_MODE_E:
         ready_state = true;
         break;
      default:
         ready_state = false;
      break;
   }
   
   return ready_state;
}
