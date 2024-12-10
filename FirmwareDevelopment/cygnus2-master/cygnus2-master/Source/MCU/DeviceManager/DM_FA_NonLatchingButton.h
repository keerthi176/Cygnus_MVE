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
*  Description  : Header file for manageing the behaviour of the First Aid non-latching button
*
*************************************************************************************/

#ifndef DM_NONLATCHINGFABUTTON_H
#define DM_NONLATCHINGFABUTTON_H


/* System Include files
*******************************************************************************/
#include <stdint.h>
#include <stdbool.h>


/* User Include files
*******************************************************************************/


/* Public Enumerations
*************************************************************************************/

typedef enum
{
   DM_FA_RESET_E,
   DM_FA_WAITING_FOR_PRESSED_E,
   DM_FA_WAITING_FOR_RELEASE_E,
   DM_FA_PROCESSING_E,
   DM_FA_STATE_MAX_E
} DM_FA_ButtonState_t;



/* Public Constants
*************************************************************************************/
#define DM_FA_SHORT_PRESS 5  //short press is up to 5 seconds
#define DM_FA_MESSAGE_TIMEOUT  10
#define DM_FA_MAX_TRANSIENTS 20

/* Public Structures
*************************************************************************************/ 
typedef struct
{
   DM_FA_ButtonState_t State;
   uint32_t PressedCount;
   uint32_t MessageIntervalCount;
   bool ButtonPressed;
   uint32_t ButtonTransient;
   bool ButtonTransientValue[DM_FA_MAX_TRANSIENTS];
   bool AlarmActive;
} DM_FA_Button_t;


/* Public Functions Prototypes
*************************************************************************************/
void DM_FA_Initialise(void);
void DM_FA_Update(void);
void DM_FA_SetButtonState(const bool pressed);
void DM_FA_SetAlarmState(const bool state);


/* Macros
*************************************************************************************/


#endif // DM_NONLATCHINGFABUTTON_H
