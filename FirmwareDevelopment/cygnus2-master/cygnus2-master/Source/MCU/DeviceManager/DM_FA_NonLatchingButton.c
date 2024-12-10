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
*  File         : DM_FA_NonLatchingButton.h
*
*  Description  : Source file for manageing the behaviour of the First Aid non-latching button
*
*************************************************************************************/


/* System Include files
*******************************************************************************/


/* User Include files
*******************************************************************************/
#include "CO_Message.h"
#include "DM_SerialPort.h"
#include "MM_ApplicationCommon.h"
#include "MM_MeshAPI.h"
#include "DM_FA_NonLatchingButton.h"



/* Private Functions Prototypes
*************************************************************************************/
static void DM_FA_ProcessButtonRelease(void);

/* Global Variables
*************************************************************************************/

/* Extern declarations
*************************************************************************************/
extern void MM_RBUApplicationAlarmSignalReq(const CO_RBUSensorData_t* pSensorData);

/* Private Variables
*************************************************************************************/
static DM_FA_Button_t gFaButton;

/* Public Functions
*************************************************************************************/
/*****************************************************************************
*  Function:      DM_FA_Initialise
*  Description:   Initialise the FA button handler.
*
*  param          None
*  return         None
*
*  Notes:
*****************************************************************************/
void DM_FA_Initialise(void)
{
   gFaButton.State = DM_FA_RESET_E;
   gFaButton.AlarmActive = false;
}

/*****************************************************************************
*  Function:      DM_FA_Update
*  Description:   Called by the Application 1 second timer to manage the
*                 non-latching button
*
*  param          None
*  return         None
*
*  Notes:
*****************************************************************************/
void DM_FA_Update(void)
{
   //Increment the time since that last alarm was sent, but limit it to the timeout value
   //to stop it wrapping.
   gFaButton.MessageIntervalCount++;
   if ( DM_FA_MESSAGE_TIMEOUT < gFaButton.MessageIntervalCount )
   {
      gFaButton.MessageIntervalCount = DM_FA_MESSAGE_TIMEOUT;
   }
   
   //Check for button transient.  Set the button value for one cycle.
   if ( gFaButton.ButtonTransient )
   {
      //Pull the transition from the queue
      gFaButton.ButtonPressed = gFaButton.ButtonTransientValue[0];
      //Move the transition queue forward
      for ( uint32_t index = 0; index < gFaButton.ButtonTransient && (DM_FA_MAX_TRANSIENTS-1) > index; index++ )
      {
         gFaButton.ButtonTransientValue[index] = gFaButton.ButtonTransientValue[index + 1];
      }
      gFaButton.ButtonTransient--;
   }
   
   //handle each wait state for the button activity
   switch ( gFaButton.State )
   {
      case DM_FA_RESET_E:
//         CO_PRINT_B_0(DBG_INFO_E,"FA state DM_FA_RESET_E\r\n");
         gFaButton.PressedCount = 0;
         gFaButton.MessageIntervalCount = DM_FA_MESSAGE_TIMEOUT;
         gFaButton.ButtonPressed = false;
         gFaButton.State = DM_FA_WAITING_FOR_PRESSED_E;
         break;
      
      case DM_FA_WAITING_FOR_PRESSED_E:
//         CO_PRINT_B_0(DBG_INFO_E,"FA state DM_FA_WAITING_FOR_PRESSED_E\r\n");
         if ( gFaButton.ButtonPressed )
         {
            gFaButton.State = DM_FA_WAITING_FOR_RELEASE_E;
            gFaButton.PressedCount = 0;
         }
         break;
         
      case DM_FA_WAITING_FOR_RELEASE_E:
//         CO_PRINT_B_0(DBG_INFO_E,"FA state DM_FA_WAITING_FOR_RELEASE_E\r\n");
         gFaButton.PressedCount++;
         if ( false == gFaButton.ButtonPressed )
         {
            gFaButton.State = DM_FA_PROCESSING_E;
         }
         break;
         
      case DM_FA_PROCESSING_E:
//         CO_PRINT_B_0(DBG_INFO_E,"FA state DM_FA_PROCESSING_E\r\n");
         DM_FA_ProcessButtonRelease();
         break;
      
      default:
//         CO_PRINT_B_0(DBG_INFO_E,"FA state default\r\n");
         DM_FA_Initialise();
         break;
   }
}

/*****************************************************************************
*  Function:      DM_FA_SetbuttonState
*  Description:   Sets the state of the button to pressed or relesed
*
*  param    pressed  True if button is held down, false otherwise.
*  return   None
*
*  Notes:
*****************************************************************************/
void DM_FA_SetButtonState(const bool pressed)
{
   CO_PRINT_B_1(DBG_INFO_E,"DM_FA_SetButtonState %d\r\n", pressed);

   if ( gFaButton.ButtonTransientValue[gFaButton.ButtonTransient] == pressed )
   {
      //the queue should always be in the sequence on,off,on,off......
      //The processing code took a value inbetween an on/off toggle
      //advance the transient count to avoid consecutive values being the same.
      gFaButton.ButtonTransient++;
   }
   gFaButton.ButtonTransientValue[gFaButton.ButtonTransient] = pressed;
   gFaButton.ButtonTransient++;
}

/*****************************************************************************
*  Function:      DM_FA_SetAlarmState
*  Description:   Sets the state of the button to pressed or relesed
*
*  param    pressed  True if button is held down, false otherwise.
*  return   None
*
*  Notes:
*****************************************************************************/
void DM_FA_SetAlarmState(const bool state)
{
   CO_PRINT_B_1(DBG_INFO_E,"DM_FA_SetAlarmState %d\r\n", state);
   //Save the new state
   gFaButton.AlarmActive = state;
   //Cancel activity
   gFaButton.State = DM_FA_RESET_E;
}


/* Private Functions
*************************************************************************************/
/*****************************************************************************
*  Function:      DM_FA_ProcessButtonRelease
*  Description:   Called when the button is release to determine if any action
*                 is required
*
*  param          None
*  return         None
*
*  Notes:
*****************************************************************************/
void DM_FA_ProcessButtonRelease(void)
{
   bool send_alarm_message = false;
   
   CO_PRINT_B_0(DBG_INFO_E,"DM_FA_ProcessButtonRelease+\r\n");
   //Are we waiting for the alarm to be triggerred?
   if ( false == gFaButton.AlarmActive )
   {
      CO_PRINT_B_0(DBG_INFO_E,"gFaButton.AlarmActive FALSE\r\n");
      //if we have just had a short press of the button and
      //the time since the last message exceeds DM_FA_MESSAGE_TIMEOUT
      //send an alarm signal
      if ( (DM_FA_SHORT_PRESS >= gFaButton.PressedCount) && (DM_FA_MESSAGE_TIMEOUT <= gFaButton.MessageIntervalCount) )
      {
         CO_PRINT_B_0(DBG_INFO_E,"gFaButton Short Press detected\r\n");
         //Send the alarm active message
         send_alarm_message = true;
         
         //Start the interval count
         gFaButton.MessageIntervalCount = 0;
         
         //remember that the alarm is active
         gFaButton.AlarmActive = true;
      }
      else 
      {
         //No further action required.  Wait for the button to be pressed again.
         gFaButton.State = DM_FA_WAITING_FOR_PRESSED_E;
      }
   }
   else
   {
      CO_PRINT_B_0(DBG_INFO_E,"gFaButton.AlarmActive TRUE\r\n");
      //The button was pressed and release while the alarm was active.
      //If its a long press send the 'alarm off' message
      if ( DM_FA_SHORT_PRESS < gFaButton.PressedCount )
      {
         CO_PRINT_B_0(DBG_INFO_E,"gFaButton Long Press detected\r\n");
         //Send the 'alarm off' message
         send_alarm_message = true;
         
         //Start the interval count
         gFaButton.MessageIntervalCount = 0;
         
         //remember that the alarm is off
         gFaButton.AlarmActive = false;
      }
      else       //if we have just had a short press of the button and
      //the time since the last message exceeds DM_FA_MESSAGE_TIMEOUT
      //send an alarm signal
      if ( (DM_FA_SHORT_PRESS >= gFaButton.PressedCount) && (DM_FA_MESSAGE_TIMEOUT <= gFaButton.MessageIntervalCount) )
      {
         CO_PRINT_B_0(DBG_INFO_E,"gFaButton Short Press detected\r\n");
         //Send the alarm active message
         send_alarm_message = true;
         
         //Start the interval count
         gFaButton.MessageIntervalCount = 0;
         
         //remember that the alarm is active
         gFaButton.AlarmActive = true;
      }

      //Wait for the button to be pressed again.
      gFaButton.State = DM_FA_WAITING_FOR_PRESSED_E;
   }
   
   //if alarm message is to be sent
   if ( send_alarm_message )
   {
      CO_RBUSensorData_t first_aid_signal;
      first_aid_signal.Handle = GetNextHandle();
      first_aid_signal.HopCount = 0;
      first_aid_signal.RUChannelIndex = CO_CHANNEL_MEDICAL_CALLPOINT_E;
      first_aid_signal.SensorType = CO_FIRST_AID_CALLPOINT_E;
      if ( gFaButton.AlarmActive )
      {
         first_aid_signal.SensorValue = 99;
         first_aid_signal.AlarmState = 1;
      }
      else 
      {
         first_aid_signal.SensorValue = 0;
         first_aid_signal.AlarmState = 0;
      }
      
      MM_RBUApplicationAlarmSignalReq(&first_aid_signal);
   }
}

