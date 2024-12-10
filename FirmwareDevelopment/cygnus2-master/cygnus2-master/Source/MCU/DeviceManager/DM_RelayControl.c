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
*  File         : DM_RelayControl.c
*
*  Description  : Source for the IOU Relay control
*
*************************************************************************************/

/* System Include Files
*************************************************************************************/

/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "board.h"
#include "DM_NVM.h"
#include "DM_ADC.h"
#include "DM_RelayControl.h"

/* Private definitions
*************************************************************************************/
#define RC_DEFAULT_SHORT_THRESHOLD  434U
#define RC_DEFAULT_LOGIC_THRESHOLD  1737U
#define RC_DEFAULT_OPEN_THRESHOLD   3395U
#define RC_DEFAULT_GUARD_BAND       50U     //this is applied as +- to the threshold so the actual band is double this figure

/* Global Varbles
*************************************************************************************/
extern DM_BaseType_t gBaseType;
extern ADC_HandleTypeDef adc1Handle;
extern Gpio_t EnDet1;
extern Gpio_t FaultDet1;
extern Gpio_t FaultDet2;
extern Gpio_t IODRlyEn;
extern Gpio_t IODSet;
extern Gpio_t IODReset;
extern Gpio_t DetR1;
extern Gpio_t DetR2;

/* Private Variables
*************************************************************************************/
static uint32_t InputShortThreshold = RC_DEFAULT_SHORT_THRESHOLD;
static uint32_t InputLogicThreshold = RC_DEFAULT_LOGIC_THRESHOLD;
static uint32_t InputOpenThreshold = RC_DEFAULT_OPEN_THRESHOLD;
static InputState_t gLastReportedInputState[MAX_INPUT_CHANNELS];
static bool gReadingInProgress = false;

/* Private Functions Prototypes
*************************************************************************************/
static ErrorCode_t DM_RC_ReadAnalogueInput(const CO_ChannelIndex_t channel, uint32_t* pValue);
static InputState_t DM_RC_GetInputState(const uint32_t Value, const InputState_t PreviousState);

/* Functions
*************************************************************************************/

/*****************************************************************************
*  Function:      DM_RC_Initialise
*  Description:   Initialisation function for Relay Control
*
*  param          None
*  return         None
*
*  Notes:
*****************************************************************************/
void DM_RC_Initialise(void)
{
   //Initialise the last reported states
   for ( uint32_t channel = 0; channel < MAX_INPUT_CHANNELS; channel++)
   {
      gLastReportedInputState[channel] = INPUT_STATE_SET_E;
   }
   
   //Initialise the relays to open
   DM_RC_WriteOutput(CO_CHANNEL_OUTPUT_1_E, RELAY_STATE_OPEN_E);
   DM_RC_WriteOutput(CO_CHANNEL_OUTPUT_2_E, RELAY_STATE_OPEN_E);
   
   //Read the input thresholds from NVM
   if ( (SUCCESS_E != DM_NVMRead(NV_IOU_INPUT_SHORT_THRESHOLD_E, (void*)&InputShortThreshold, sizeof(InputShortThreshold))) )
   {
      InputShortThreshold = RC_DEFAULT_SHORT_THRESHOLD;
   }
   
   if ( (SUCCESS_E != DM_NVMRead(NV_IOU_INPUT_LOGIC_THRESHOLD_E, (void*)&InputLogicThreshold, sizeof(InputLogicThreshold))) )
   {
      InputLogicThreshold = RC_DEFAULT_LOGIC_THRESHOLD;
   }
   
   if ( (SUCCESS_E != DM_NVMRead(NV_IOU_INPUT_OPEN_THRESHOLD_E, (void*)&InputOpenThreshold, sizeof(InputOpenThreshold))) )
   {
      InputOpenThreshold = RC_DEFAULT_OPEN_THRESHOLD;
   }
   
   //Make sure the ADC was initialised
   if ( NULL == adc1Handle.Instance )
   {
      adc1Handle.Instance = ADC1;
   }
}

/*****************************************************************************
*  Function:      DM_RC_CanGoToSleep
*  Description:   Function to prevent sleep while a reading is in progress.
*
*  param          None.
*
*  return         True if the RBU can sleep (no reading in progress).
*  Notes:
*****************************************************************************/
bool DM_RC_CanGoToSleep(void)
{
   return !gReadingInProgress;
}

/*****************************************************************************
*  Function:      DM_RC_ReadInputs
*  Description:   Read the state of the analogue inputs, channel 1 & 2.
*                 Keep a copy of the previous input state so that the
*                 guard bands are properly observed depending on the direction
*                 of the state transition (done in call to DM_RC_GetInputState).
*
*  param          pState1      [OUT]The logic state or fault condition for i/p channel 1.
*  param          pState2      [OUT]The logic state or fault condition for i/p channel 2.
*
*  return         SUCCESS_E or error code
*  Notes:
*****************************************************************************/
ErrorCode_t DM_RC_ReadInputs(InputState_t* pState1, InputState_t* pState2)
{
   static InputState_t previous_ip_channel_1_state = INPUT_STATE_RESET_E;
   static InputState_t previous_ip_channel_2_state = INPUT_STATE_RESET_E;
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   ErrorCode_t result1 = ERR_OPERATION_FAIL_E;
   ErrorCode_t result2 = ERR_OPERATION_FAIL_E;
   InputState_t ip_channel_1_state;
   InputState_t ip_channel_2_state;

   
   uint32_t raw_value;
   
   // check that the output parameter isn't null
   if ( pState1 && pState2 )
   {
      gReadingInProgress = true;
      //Set the input enable line
      GpioWrite(&EnDet1, GPIO_PIN_SET);
      //Give the enable voltage time to settle
      osDelay(3);
      
      // read the raw analogue value from channel 1
      result1 = DM_RC_ReadAnalogueInput(CO_CHANNEL_INPUT_1_E, &raw_value);

      if ( SUCCESS_E == result1 )
      {
         ip_channel_1_state = DM_RC_GetInputState(raw_value, previous_ip_channel_1_state);
//         CO_PRINT_B_2(DBG_INFO_E,"Reading channel 1, ADC value=%d, state=%d\r\n", raw_value, ip_channel_1_state);
         previous_ip_channel_1_state = ip_channel_1_state;
         *pState1 = ip_channel_1_state;
      }
      
      // read the raw analogue value from channel 2
      result2 = DM_RC_ReadAnalogueInput(CO_CHANNEL_INPUT_2_E, &raw_value);

      if ( SUCCESS_E == result2 )
      {
         ip_channel_2_state = DM_RC_GetInputState(raw_value, previous_ip_channel_2_state);
//         CO_PRINT_B_2(DBG_INFO_E,"Reading channel 2, ADC value=%d, state=%d\r\n", raw_value, ip_channel_2_state);
         previous_ip_channel_2_state = ip_channel_2_state;
         *pState2 = ip_channel_2_state;
      }
      
      //Reset the enable line
      GpioWrite(&EnDet1, GPIO_PIN_RESET);
      gReadingInProgress = false;
      
      
      if ( SUCCESS_E != result1 )
      {
         result = result1;
      }
      else if ( SUCCESS_E != result2 )
      {
         result = result2;
      }
      else 
      {
         result = SUCCESS_E;
      }
   }

   return result;
}

/*****************************************************************************
*  Function:      DM_RC_GetInputState
*  Description:   Compare the latest reading against the programmed thresholds
*                 and determine the current state of the input.
*
*  param          Value          The latest reading from the input.
*  param          PreviousState  The previous logic state or fault condition for the input.
*
*  return         InputState_t   The determined state after comparison with thresholds
*  Notes:
*****************************************************************************/
InputState_t DM_RC_GetInputState(const uint32_t Value, const InputState_t PreviousState)
{
   InputState_t input_state = INPUT_STATE_FAULT_OPEN_E;
   
   // check against the thresholds for the input state
   switch ( PreviousState )
   {
      case INPUT_STATE_RESET_E:
         if ( (InputShortThreshold - RC_DEFAULT_GUARD_BAND) > Value )
         {
            //Fault condition - SHORT CCT
            input_state = INPUT_STATE_FAULT_SHORT_E;
         }
         else if ( (InputOpenThreshold + RC_DEFAULT_GUARD_BAND) < Value )
         {
            //The raw_value is above the OPEN threshold
            input_state = INPUT_STATE_FAULT_OPEN_E;
         }
         else if ( (InputLogicThreshold + RC_DEFAULT_GUARD_BAND) < Value )
         {
            //The raw_value is inbetween the OPEN threshold and the LOGIC threshold so it's SET
            input_state = INPUT_STATE_SET_E;
         }
         else 
         {
            //Still within the RESET region
            input_state = INPUT_STATE_RESET_E;
         }
         break;
      case INPUT_STATE_SET_E:
         if ( (InputShortThreshold - RC_DEFAULT_GUARD_BAND) > Value )
         {
            //Fault condition - SHORT CCT
            input_state = INPUT_STATE_FAULT_SHORT_E;
         }
         else if ( (InputLogicThreshold - RC_DEFAULT_GUARD_BAND) > Value )
         {
            //The raw_value is between the SHORT threshold and the LOGIC threshold, so its RESET
            input_state = INPUT_STATE_RESET_E;
         }
         else if ( (InputOpenThreshold + RC_DEFAULT_GUARD_BAND) < Value )
         {
            //The raw_value is above the OPEN threshold
            input_state = INPUT_STATE_FAULT_OPEN_E;
         }
         else 
         {
            //Still within the SET region
            input_state = INPUT_STATE_SET_E;
         }
         break;
      case INPUT_STATE_FAULT_SHORT_E:
         if ( (InputOpenThreshold + RC_DEFAULT_GUARD_BAND) < Value )
         {
            //The raw_value is above the OPEN threshold
            input_state = INPUT_STATE_FAULT_OPEN_E;
         }
         else if ( (InputLogicThreshold + RC_DEFAULT_GUARD_BAND) < Value )
         {
            //The raw_value is inbetween the LOGIC threshold and the OPEN threshold so it's SET
            input_state = INPUT_STATE_SET_E;
         }
         else if ( (InputShortThreshold + RC_DEFAULT_GUARD_BAND) < Value )
         {
            //The raw_value is inbetween the SHORT threshold and the LOGIC threshold so it's RESET
            input_state = INPUT_STATE_RESET_E;
         }
         else 
         {
            //Still within the SHORT region
            input_state = INPUT_STATE_FAULT_SHORT_E;
         }
         break;
      case INPUT_STATE_FAULT_OPEN_E:
         if ( (InputShortThreshold - RC_DEFAULT_GUARD_BAND) > Value )
         {
            //The raw_value is below the SHORT threshold
            input_state = INPUT_STATE_FAULT_SHORT_E;
         }
         else if ( (InputLogicThreshold - RC_DEFAULT_GUARD_BAND) > Value )
         {
            //The raw_value is inbetween the LOGIC threshold and the SHORT threshold so it's RESET
            input_state = INPUT_STATE_RESET_E;
         }
         else if ( (InputOpenThreshold - RC_DEFAULT_GUARD_BAND) > Value )
         {
            //The raw_value is inbetween the OPEN threshold and the LOGIC threshold so it's SET
            input_state = INPUT_STATE_SET_E;
         }
         else 
         {
            //Still within the OPEN region
            input_state = INPUT_STATE_FAULT_OPEN_E;
         }
         break;
      default:
         //Shouldn't get here.
         CO_PRINT_B_1(DBG_ERROR_E,"DM_RC_GetInputState received invalid PreviousState = %d\r\n", (uint32_t)PreviousState);
      break;
   }
   
   return input_state;
}
/*****************************************************************************
*  Function:      DM_RC_ReadAnalogueInput
*  Description:   Read the raw ADC value of the specified input.
*
*  param          channel     CO_CHANNEL_INPUT_1_E for input 1,
*                             CO_CHANNEL_INPUT_2_E for input 2
*  param          pValue      [OUT]the raw ADC reading.
*
*  return         SUCCESS_E or error code
*  Notes:
*****************************************************************************/
ErrorCode_t DM_RC_ReadAnalogueInput(const CO_ChannelIndex_t channel, uint32_t* pValue)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   
   uint32_t adc_channel = 0;
   // check that the output parameter isn't null
   if ( NULL != pValue )
   {
      //check that the channel is in range
      result = ERR_OUT_OF_RANGE_E;
      if (CO_CHANNEL_INPUT_1_E == channel || CO_CHANNEL_INPUT_2_E == channel )
      {
         adc_channel = ADC_CHANNEL_1;
         if ( CO_CHANNEL_INPUT_2_E == channel )
         {
            adc_channel = ADC_CHANNEL_2;
         }
         result = DM_ADC_Read(&adc1Handle, adc_channel, 0, pValue);
         
         if ( SUCCESS_E != result )
         {
            CO_PRINT_B_2(DBG_ERROR_E,"DM_RC_ReadAnalogueInput : FAILED channel = %d, error = %d\r\n", channel, result);
         }
//         else 
//         {
//            CO_PRINT_B_2(DBG_INFO_E,"DM_RC_ReadAnalogueInput : channel = %d, value = %d\r\n", channel, *pValue);
//         }
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:      DM_RC_WriteOutput
*  Description:   Write a new state to the output relays.
*
*  param          channel     CO_CHANNEL_OUTPUT_1_E for relay 1,
*                             CO_CHANNEL_OUTPUT_2_E for relay 2
*  param          state       RELAY_STATE_CLOSED_E to activate relay, RELAY_STATE_OPEN_E to de-activate
*
*  return         SUCCESS_E or error code
*  Notes:
*****************************************************************************/
ErrorCode_t DM_RC_WriteOutput(const CO_ChannelIndex_t channel, const RelayState_t state)
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   //check that the channel is in range
   if (CO_CHANNEL_OUTPUT_1_E == channel || CO_CHANNEL_OUTPUT_2_E == channel )
   {
      //Set the relay enable line
      if ( CO_CHANNEL_OUTPUT_1_E == channel )
      {
         GpioWrite(&IODRlyEn, GPIO_PIN_SET);
      }
      else 
      {
         GpioWrite(&IODRlyEn, GPIO_PIN_RESET);
      }
      
      //Give the enable voltage time to settle
      CO_InlineDelay(10);
      //Pulse the SET or RESET line 
      if ( RELAY_STATE_OPEN_E == state )
      {
         GpioWrite(&IODReset, GPIO_PIN_SET);
         CO_InlineDelay(10);
         GpioWrite(&IODReset, GPIO_PIN_RESET);
      }
      else 
      {
         GpioWrite(&IODSet, GPIO_PIN_SET);
         CO_InlineDelay(10);
         GpioWrite(&IODSet, GPIO_PIN_RESET);
      }
      //leave the enable line low.
      GpioWrite(&IODRlyEn, GPIO_PIN_RESET);
      result = SUCCESS_E;
   }

   return result;
}

/*****************************************************************************
*  Function:      DM_RC_ReadOutputState
*  Description:   Read the state of the requested output.
*
*  param          channel        CO_CHANNEL_OUTPUT_1_E for relay 1,
*                                CO_CHANNEL_OUTPUT_2_E for relay 2
*
*  return         RelayState_t   The current relay condition
*  Notes:
*****************************************************************************/
RelayState_t DM_RC_ReadOutputState(const CO_ChannelIndex_t channel)
{
   RelayState_t state = RELAY_STATE_UNKNOWN_E;

   switch ( channel )
   {
      case CO_CHANNEL_OUTPUT_1_E:
         if ( 0 == GpioRead(&DetR1) )
         {
            state = RELAY_STATE_OPEN_E;
         }
         else 
         {
            state = RELAY_STATE_CLOSED_E;
         }
         break;
      case CO_CHANNEL_OUTPUT_2_E:
         if ( 0 == GpioRead(&DetR2) )
         {
            state = RELAY_STATE_OPEN_E;
         }
         else 
         {
            state = RELAY_STATE_CLOSED_E;
         }
         break;
      default:
         state = RELAY_STATE_UNKNOWN_E;
      break;
   }

   return state;
}

/*****************************************************************************
*  Function:      DM_RC_SetLastReportedState
*  Description:   Record that last state that was sent to the control panel
*                 for a specified input channel.
*
*  param          channel        The channel to update.
*  param          state          The state to record.
*
*  return         ErrorCode_t    SUCCESS_E or error code.
*  Notes:
*****************************************************************************/
ErrorCode_t DM_RC_SetLastReportedState(const CO_ChannelIndex_t channel, const InputState_t state)
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   int32_t channel_index = channel - CO_CHANNEL_INPUT_1_E;
   
   if ( (0 <= channel_index) && (MAX_INPUT_CHANNELS > channel_index) )
   {
      gLastReportedInputState[channel_index] = state;
      result = SUCCESS_E;
   }
   return result;
}

/*****************************************************************************
*  Function:      DM_RC_GetLastReportedState
*  Description:   Return that last state that was sent to the control panel
*                 for a specified input channel.
*
*  param          channel        The channel to query.
*  param          pState         [OUT] The recorded state.
*
*  return         ErrorCode_t    SUCCESS_E or error code.
*  Notes:
*****************************************************************************/
ErrorCode_t DM_RC_GetLastReportedState(const CO_ChannelIndex_t channel, InputState_t* const pState)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   int32_t channel_index = channel - CO_CHANNEL_INPUT_1_E;
   
   if ( pState )
   {
      result = ERR_OUT_OF_RANGE_E;
      if ( (0 <= channel_index) && (MAX_INPUT_CHANNELS > channel_index) )
      {
         *pState = gLastReportedInputState[channel_index];
         result = SUCCESS_E;
      }
   }
   return result;
}

