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
*  File         : DM_OutputManagement.c
*
*  Description  : Source for the management of the output silenceable/unsilenceable
*                 bit masks and output control in accordance with an Alarm Output
*                 State signal
*
*************************************************************************************/

/* System Include Files
*************************************************************************************/
#include "cmsis_os.h"
/* User Include Files
*************************************************************************************/
#include "board.h"
#include "DM_NVM.h"
#include "MM_ApplicationCommon.h"
#include "MM_MeshAPI.h"
#include "DM_svi.h"
#include "DM_RelayControl.h"
#include "DM_LED.h"
#include "MM_TimedEventTask.h"
#include "CFG_Device_cfg.h"
#include "DM_OutputManagement.h"
#include "MC_MacConfiguration.h"

/* Private definitions
*************************************************************************************/
#define DM_ONE_SHOT_DURATION 2
#define TEST_MODE_HOLD_OFF_COUNT 6 //seconds

typedef void (*SounderFunction_t)( const uint32_t count );

typedef enum
{
   DM_OP_DLY_IDLE_E,
   DM_OP_DLY_INITIAL_DELAY_E,
   DM_OP_DLY_ACTIVE_E
}DelayedOutputState_t;

/* Private Functions Prototypes
*************************************************************************************/

static bool DM_OP_ProcessRBUOutputSignal(const OutputSignalIndication_t* OutputData);
static bool DM_OP_ProcessIOUnitOutputSignal(const OutputSignalIndication_t* OutputData);
static bool DM_OP_ProcessMCPOutputSignal(const OutputSignalIndication_t* pOutputData);
static bool DM_OP_ProcessConstructionOutputSignal(const OutputSignalIndication_t* OutputData);
static bool DM_OP_SendOutputToHead(const OutputSignalIndication_t* pOutputData);
static ErrorCode_t DM_OP_SetSounderAlarmState(const bool activated, const CO_OutputProfile_t profile);
static ErrorCode_t DM_OP_SetBeaconAlarmState(const bool activated, const CO_OutputProfile_t profile);
static ErrorCode_t DM_OP_SetIndicatorLEDState(const bool activated, const CO_OutputProfile_t profile);
static ErrorCode_t DM_OP_SetVisualIndicator(const bool activated, const CO_OutputProfile_t profile);
static ErrorCode_t DM_OP_SetCombinedSounderVisualIndicator(const bool activated, const CO_OutputProfile_t profile);
static ErrorCode_t DM_OP_SetOutputChannelState(CO_ChannelIndex_t channel, const bool activated, const CO_OutputProfile_t profile);
static bool DM_OP_HasPriority(const DM_OP_OutputChannel_t channel, const CO_OutputProfile_t profile);
static void DM_OP_SiteNetSounderPattern1(const uint32_t count);
static void DM_OP_SiteNetSounderPattern2(const uint32_t count);
static void DM_OP_SiteNetSounderPattern3(const uint32_t count);
static void DM_OP_SiteNetSounderPattern4(const uint32_t count);
static void DM_OP_SiteNetSounderPattern5(const uint32_t count);
static void DM_OP_SiteNetSounderPattern6(const uint32_t count);

/* Global Varbles
*************************************************************************************/
extern DM_BaseType_t gBaseType;
extern uint16_t gDeviceCombination;
extern uint16_t gLastCommandedChannel;
extern uint8_t gLastCommandedOutputProfile;
extern uint8_t gLastCommandedOutputActive;
extern uint16_t gLastCommandedOutputDuration;
extern uint8_t gLastCommandedSilenceableMask;
extern uint8_t gLastCommandedUnsilenceableMask;
extern uint8_t gLastCommandedDelayMask;
extern uint32_t gZoneNumber;
extern uint16_t gNodeAddress;
extern Gpio_t SndrBeaconOn;
extern osPoolId AppPool;
extern osMessageQId(HeadQ);
extern int32_t gHeadQueueCount;
extern ApplicationChildArray_t childList;
#ifndef USE_NEW_HEAD_INTERFACE
extern osSemaphoreId HeadInterfaceSemaphoreId;
#endif

uint8_t gOutputProfile[CO_PROFILE_MAX_E][CO_CHANNEL_MAX_E];


SounderFunction_t SounderSequence[DM_OP_SITENET_SOUNDER_PATTERN_MAX_E] =
{
   DM_OP_SiteNetSounderPattern6,//   DM_OP_SITENET_SOUNDER_PATTERN_NONE_E        : Continuous OFF
   DM_OP_SiteNetSounderPattern1,//   DM_OP_SITENET_SOUNDER_PATTERN_ON_E          : Continuous ON
   DM_OP_SiteNetSounderPattern2,//   DM_OP_SITENET_SOUNDER_PATTERN_1000_7000_E   : 1s ON : 7s OFF
   DM_OP_SiteNetSounderPattern3,//   DM_OP_SITENET_SOUNDER_PATTERN_500_700_E     : 0.5s ON : 0.7s OFF
   DM_OP_SiteNetSounderPattern4,//   DM_OP_SITENET_SOUNDER_PATTERN_500_300_E     : (0.5s ON : 0.5 OFF)*3 : 1.5s OFF
   DM_OP_SiteNetSounderPattern5,//   DM_OP_SITENET_SOUNDER_PATTERN_100_2000_E    : 0.1s ON : 2s OFF
   DM_OP_SiteNetSounderPattern6 //   DM_OP_SITENET_SOUNDER_PATTERN_OFF_E         : Continuous OFF
};

/* Private Variables
*************************************************************************************/
static bool gFirstAidAlarmActive = false;
static uint32_t gFirstAidAlarmCount = 0;

static DM_Enable_t gChannelDisabled[CO_CHANNEL_MAX_E];

static DM_OP_AlarmState_t OutputState[DM_OP_CHANNEL_MAX_E];

static CO_AlarmOutputStateData_t previous_command;

static DelayedOutputState_t gDelayedOutputState;
static CO_ChannelIndex_t gDelayedOutputChannel;
static uint32_t gDelayedOutputInitialDelay;
static uint32_t gDelayedOutputDuration;
static uint32_t gTestModeResetCount;

/* Functions
*************************************************************************************/

/*****************************************************************************
*  Function:      DM_OP_Initialise
*  Description:   Initialisation function for output management
*
*  param          LoadProfiles - True if the profiles should be read from NVM
*  return         None
*
*  Notes:
*****************************************************************************/
void DM_OP_Initialise(const bool LoadProfiles)
{
   uint32_t profile;
   uint32_t channel;
   uint32_t op_channel;
   
   if ( LoadProfiles )
   {
      //Assign default output profiles
      for( profile = 0; profile < CO_PROFILE_MAX_E; profile++)
      {
         for( channel = 0; channel < CO_CHANNEL_MAX_E; channel++)
         {
            gOutputProfile[profile][channel] = 0;
         }
      }
      
      //Read the programmed profiles from NVM
      DM_OP_LoadProfiles();
   }
   
   //Set the initial states of the outputs
   DM_OP_InitialiseOutputs();
   
   //initialise the output channel activation state
   for( op_channel = 0; op_channel < DM_OP_CHANNEL_MAX_E; op_channel++ )
   {
      OutputState[op_channel].Activated = false;
      OutputState[op_channel].ActiveProfile = CO_PROFILE_MAX_E;		
   }
   
   //Repeat command filter.
   //Reset previous commands to an out-of-range value to ensure that the first command is actioned.
   previous_command.Silenceable = 0xFFFFFFFF;
   previous_command.Unsilenceable = 0xFFFFFFFF;
   previous_command.DelayMask = 0xFFFFFFFF;
   
   //Pull the global delays out of NVM
   uint32_t combined_delays;
   if ( SUCCESS_E == DM_OP_GetGlobalDelayValues(&combined_delays) )
   {
      //Store the separate delay values in the device config for dynamic access.
      uint16_t delay1 = (uint16_t)(combined_delays & 0xFFFF);
      uint16_t delay2 = (uint16_t)(combined_delays >> 16);
      CFG_SetGlobalDelay1(delay1);
      CFG_SetGlobalDelay2(delay2);
   }
   
   //Reset the last Rx'd command record to cancel duplicate checks
   gLastCommandedChannel = CO_CHANNEL_MAX_E;
   gLastCommandedOutputProfile = CO_PROFILE_MAX_E;
   gLastCommandedOutputActive = 0;
   gLastCommandedOutputDuration = 0;
   
   //Initialise the delayed output test function
   gDelayedOutputState = DM_OP_DLY_IDLE_E;
   gDelayedOutputChannel = CO_CHANNEL_MAX_E;
   gDelayedOutputInitialDelay = 0;
   gDelayedOutputDuration = 0;
   gTestModeResetCount = 0;
   
}


/*****************************************************************************
*  Function:      DM_OP_LoadProfiles
*  Description:   Loads the output profiles from NVM
*
*  param          None
*  return         None
*
*  Notes:
*****************************************************************************/
void DM_OP_LoadProfiles(void)
{
   DM_NVMParamId_t nvm_param;
   uint32_t nvm_value;
   DM_Enable_t enable_disable;
   uint32_t channelIndex;
   
   uint32_t profile_fire;
   DM_NVMRead(NV_PROFILE_FIRE_E, &profile_fire, sizeof(profile_fire));
   uint32_t profile_evacuate;
   DM_NVMRead(NV_PROFILE_EVACUATE_E, &profile_evacuate, sizeof(profile_evacuate));
   uint32_t profile_first_aid;
   DM_NVMRead(NV_PROFILE_FIRST_AID_E, &profile_first_aid, sizeof(profile_first_aid));
   uint32_t profile_security;
   DM_NVMRead(NV_PROFILE_SECURITY_E, &profile_security, sizeof(profile_security));
   uint32_t profile_general;
   DM_NVMRead(NV_PROFILE_GENERAL_E, &profile_general, sizeof(profile_general));
   uint32_t profile_fault;
   DM_NVMRead(NV_PROFILE_FAULT_E, &profile_fault, sizeof(profile_fault));
   uint32_t profile_routing;
   DM_NVMRead(NV_PROFILE_ROUTING_ACK_E, &profile_routing, sizeof(profile_routing));
   uint32_t profile_test;
   DM_NVMRead(NV_PROFILE_TEST_E, &profile_test, sizeof(profile_test));
   uint32_t profile_silent_test;
   DM_NVMRead(NV_PROFILE_SILENT_TEST_E, &profile_silent_test, sizeof(profile_silent_test));
   
   //read the disablement for all channels
   for ( channelIndex = 0; channelIndex < CO_CHANNEL_MAX_E; channelIndex++)
   {
      //check that the channel has an associated NVM slot
      nvm_param = DM_NVM_ChannelIndexToNVMParameter(channelIndex); 
      
      if ( NV_MAX_PARAM_ID_E != nvm_param )
      {
         // Read the NVM
         if ( SUCCESS_E == DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
         {
            enable_disable = (DM_Enable_t)(nvm_value >> 30);
            
            gChannelDisabled[channelIndex] = enable_disable;
         }
      }
   }
   
   //always enable the Status LED channel
   gChannelDisabled[CO_CHANNEL_STATUS_INDICATOR_LED_E] = DM_ENABLE_DAY_AND_NIGHT_E;
   
   
   //Use the profiles to set up the o/p behaviour, except for CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E which is hard coded.
   //the sounder tones are '1' based in the profile, but the svi is '0' based, so subtract 1 from the setting.
   gOutputProfile[CO_PROFILE_FIRE_E][DM_OP_CHANNEL_SOUNDER_E] = (uint8_t)(profile_fire & 0xFF) - 1;
   gOutputProfile[CO_PROFILE_FIRST_AID_E][DM_OP_CHANNEL_SOUNDER_E] = (uint8_t)(profile_first_aid & 0xFF) - 1;
   gOutputProfile[CO_PROFILE_EVACUATE_E][DM_OP_CHANNEL_SOUNDER_E] = (uint8_t)(profile_evacuate & 0xFF) - 1;
   gOutputProfile[CO_PROFILE_SECURITY_E][DM_OP_CHANNEL_SOUNDER_E] = (uint8_t)(profile_security & 0xFF) - 1;
   gOutputProfile[CO_PROFILE_GENERAL_E][DM_OP_CHANNEL_SOUNDER_E] = (uint8_t)(profile_general & 0xFF) - 1;
   gOutputProfile[CO_PROFILE_FAULT_E][DM_OP_CHANNEL_SOUNDER_E] = (uint8_t)(profile_fault & 0xFF) - 1;
   gOutputProfile[CO_PROFILE_ROUTING_ACK_E][DM_OP_CHANNEL_SOUNDER_E] = (uint8_t)(profile_routing & 0xFF) - 1;
   gOutputProfile[CO_PROFILE_TEST_E][DM_OP_CHANNEL_SOUNDER_E] = (uint8_t)(profile_test & 0xFF) - 1;
   gOutputProfile[CO_PROFILE_SILENT_TEST_E][DM_OP_CHANNEL_SOUNDER_E] = (uint8_t)(profile_silent_test & 0xFF) - 1;
   
   gOutputProfile[CO_PROFILE_FIRE_E][DM_OP_CHANNEL_BEACON_E] = (uint8_t)((profile_fire >> 16) & 0xFF);
   gOutputProfile[CO_PROFILE_FIRST_AID_E][DM_OP_CHANNEL_BEACON_E] = (uint8_t)((profile_first_aid >> 16) & 0xFF);
   gOutputProfile[CO_PROFILE_EVACUATE_E][DM_OP_CHANNEL_BEACON_E] = (uint8_t)((profile_evacuate >> 16) & 0xFF);
   
   gOutputProfile[CO_PROFILE_FIRE_E][DM_OP_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E] = (uint8_t)(CONST_SOUNDER_CONTINUOUS_E);
   gOutputProfile[CO_PROFILE_FIRST_AID_E][DM_OP_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E] = (uint8_t)(CONST_SOUNDER_PULSE_E);
   gOutputProfile[CO_PROFILE_EVACUATE_E][DM_OP_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E] = (uint8_t)(CONST_SOUNDER_CONTINUOUS_E);
   
   gOutputProfile[CO_PROFILE_FIRE_E][DM_OP_CHANNEL_VISUAL_INDICATOR_E] = (uint8_t)((profile_fire >> 16) & 0xFF);
   gOutputProfile[CO_PROFILE_FIRST_AID_E][DM_OP_CHANNEL_VISUAL_INDICATOR_E] = (uint8_t)((profile_first_aid >> 16) & 0xFF);
   gOutputProfile[CO_PROFILE_EVACUATE_E][DM_OP_CHANNEL_VISUAL_INDICATOR_E] = (uint8_t)((profile_evacuate >> 16) & 0xFF);
   
   gOutputProfile[CO_PROFILE_FIRE_E][DM_OP_CHANNEL_STATUS_INDICATOR_LED_E] = (uint8_t)((profile_fire >> 16) & 0xFF);
   gOutputProfile[CO_PROFILE_FIRST_AID_E][DM_OP_CHANNEL_STATUS_INDICATOR_LED_E] = (uint8_t)((profile_first_aid >> 16) & 0xFF);
   gOutputProfile[CO_PROFILE_EVACUATE_E][DM_OP_CHANNEL_STATUS_INDICATOR_LED_E] = (uint8_t)((profile_evacuate >> 16) & 0xFF);
}

/*****************************************************************************
*  Function:      DM_OP_InitialiseOutputs
*  Description:   Sets all outputs to 'inactive' unless the channel configuration
*                 is set to inverted, in which case the output is set to 'active'
*
*  param          None
*  return         None
*
*  Notes:
*****************************************************************************/
void DM_OP_InitialiseOutputs(void)
{
   AlarmOutputStateConfig_t  alarmConfig;
   ErrorCode_t result = ERR_INVALID_PARAMETER_E;
   uint32_t outputChannelIndex;
   uint32_t channelIndex;
   uint32_t deviceCode;
   CO_OutputProfile_t profile;
   
   // Iterate through all output channel indexes and check to see if this unit has that channel
   for( outputChannelIndex = 0; outputChannelIndex < DM_OP_CHANNEL_MAX_E; outputChannelIndex++ )
   {
      //clear any delays
      OutputState[outputChannelIndex].Delay1State = DM_OP_TIMER_NOT_RUNNING_E;
      OutputState[outputChannelIndex].Delay1Count = 0;
      OutputState[outputChannelIndex].Delay2State = DM_OP_TIMER_NOT_RUNNING_E;
      OutputState[outputChannelIndex].Delay2Count = 0;
      
      //Don't initialise the SVI sounder, it does that automatically.
//      if ( (DM_OP_CHANNEL_SOUNDER_E != outputChannelIndex) &&
//           (DM_OP_CHANNEL_VISUAL_INDICATOR_E != outputChannelIndex) )
//      {
         // convert local output channel index (DM_OP_OutputChannel_t)to the global channel index (CO_ChannelIndex_t)
         channelIndex = (uint32_t)DM_OP_OutputChannelToSystemChannel((DM_OP_OutputChannel_t)outputChannelIndex);
         
         //Check that the channel is valid and that it isn't disabled
         if ( (CO_CHANNEL_NONE_E != channelIndex) )
         {
            //convert channel index to device
            result = DM_DeviceGetDeviceCode(gDeviceCombination, channelIndex, &deviceCode);
            if ( SUCCESS_E == result )
            {
               //Do we have this output channel enabled on this unit?
               if ( DM_DeviceIsEnabled(gDeviceCombination, deviceCode, DC_MATCH_ANY_E) )
               {
                  //Get the channel config from NVM
                  result = DM_OP_GetChannelOutputConfig((CO_ChannelIndex_t) channelIndex, &alarmConfig);
                  if ( SUCCESS_E == result )
                  {
                     profile = OutputState[outputChannelIndex].ActiveProfile;
                     //If no profiles are active, assume FIRE profile
                     if ( CO_PROFILE_MAX_E == profile )
                     {
                        profile = CO_PROFILE_FIRE_E;
                     }
                     if ( (DM_OP_CHANNEL_OUTPUT_1_E == outputChannelIndex || DM_OP_CHANNEL_OUTPUT_2_E == outputChannelIndex ) && alarmConfig.Inverted )
                     {
                        //activate channel
                        result = DM_OP_SetAlarmState((CO_ChannelIndex_t)channelIndex, profile, true, false);
                     }
                     else 
                     {
                        //deactivate channel
                        CO_PRINT_B_1(DBG_INFO_E,"Resetting channel %d\r\n", channelIndex);
                        osDelay(10);
                        result = DM_OP_SetAlarmState((CO_ChannelIndex_t)channelIndex, profile, false, false);
                     }
                  }
               }
            }
         }
//      }
   }
}

/*****************************************************************************
*  Function:      DM_OP_OutputChannelToSystemChannel
*  Description:   Convert the local oputput channel index (DM_OP_OutputChannel_t)
*                 to the common system channel index (CO_ChannelIndex_t)
*
*  param    outputChannelIndex   The output channel index to convert.
*
*  return   CO_ChannelIndex_t    The corresponding system channel index.
*
*  Notes:
*****************************************************************************/
CO_ChannelIndex_t DM_OP_OutputChannelToSystemChannel(const DM_OP_OutputChannel_t outputChannelIndex)
{
   CO_ChannelIndex_t system_channel;
   
   switch ( outputChannelIndex )
   {
      case  DM_OP_CHANNEL_SOUNDER_E:
         system_channel = CO_CHANNEL_SOUNDER_E;
      break;
      case DM_OP_CHANNEL_BEACON_E:
         system_channel = CO_CHANNEL_BEACON_E;
      break;
      case DM_OP_CHANNEL_STATUS_INDICATOR_LED_E:
         system_channel = CO_CHANNEL_STATUS_INDICATOR_LED_E;
      break;
      case DM_OP_CHANNEL_VISUAL_INDICATOR_E:
         system_channel = CO_CHANNEL_VISUAL_INDICATOR_E;
      break;
      case DM_OP_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E:
         system_channel = CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E;
      break;
      case DM_OP_CHANNEL_OUTPUT_ROUTING_E:
         system_channel = CO_CHANNEL_OUTPUT_ROUTING_E;
      break;
      case DM_OP_CHANNEL_OUTPUT_1_E:
         system_channel = CO_CHANNEL_OUTPUT_1_E;
      break;
      case DM_OP_CHANNEL_OUTPUT_2_E:
         system_channel = CO_CHANNEL_OUTPUT_2_E;
      break;
      default:
         system_channel = CO_CHANNEL_NONE_E;
      break;
   }
   
   return system_channel;
}

/*****************************************************************************
*  Function:      DM_OP_SystemChannelToOutputChannel
*  Description:   Convert the common system channel index (CO_ChannelIndex_t)
*                 to the local oputput channel index (DM_OP_OutputChannel_t)
*
*  param    outputChannelIndex   The output channel index to convert.
*
*  return   CO_ChannelIndex_t    The corresponding system channel index.
*
*  Notes:
*****************************************************************************/
DM_OP_OutputChannel_t DM_OP_SystemChannelToOutputChannel(const CO_ChannelIndex_t systemChannelIndex)
{
   DM_OP_OutputChannel_t output_channel;
   
   switch ( systemChannelIndex )
   {
      case CO_CHANNEL_SOUNDER_E:
         output_channel = DM_OP_CHANNEL_SOUNDER_E;
      break;
      case CO_CHANNEL_BEACON_E:
         output_channel = DM_OP_CHANNEL_BEACON_E;
      break;
      case CO_CHANNEL_STATUS_INDICATOR_LED_E:
         output_channel = DM_OP_CHANNEL_STATUS_INDICATOR_LED_E;
      break;
      case CO_CHANNEL_VISUAL_INDICATOR_E:
         output_channel = DM_OP_CHANNEL_VISUAL_INDICATOR_E;
      break;
      case CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E:
         output_channel = DM_OP_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E;
      break;
      case CO_CHANNEL_OUTPUT_ROUTING_E:
         output_channel = DM_OP_CHANNEL_OUTPUT_ROUTING_E;
      break;
      case CO_CHANNEL_OUTPUT_1_E:
         output_channel = DM_OP_CHANNEL_OUTPUT_1_E;
      break;
      case CO_CHANNEL_OUTPUT_2_E:
         output_channel = DM_OP_CHANNEL_OUTPUT_2_E;
      break;
      default:
         output_channel = DM_OP_CHANNEL_MAX_E;//invalid channel
      break;
   }
   
   return output_channel;
}


/*****************************************************************************
*  Function:      DM_OP_TypeIsActive
*  Description:   Check the supplied profile for a particular alarm type
*
*  param    profile        The profile to test.
*  param    alarmType      The alarm type to look for.
*
*  return   bool           True if the alarm type is present, false otherwise.
*
*  Notes:
*****************************************************************************/
bool DM_OP_TypeIsActive(const uint8_t profile, const DM_OP_AlarmEvent_t alarmType)
{
   bool isActive;
   
   switch ( alarmType )
   {
      case DM_OP_ALARM_EVENT_FIRE_E:
         isActive = ((profile & DM_OP_BIT_FIRE) == DM_OP_BIT_FIRE) ? true:false;
         break;
      case DM_OP_ALARM_EVENT_FIRST_AID_E:
         isActive = ((profile & DM_OP_BIT_FIRST_AID) == DM_OP_BIT_FIRST_AID) ? true:false;
         break;
      case DM_OP_ALARM_EVENT_EVACUATION_E:
         isActive = ((profile & DM_OP_BIT_EVACUATION) == DM_OP_BIT_EVACUATION) ? true:false;
         break;
      case DM_OP_ALARM_EVENT_SECURITY_E:
         isActive = ((profile & DM_OP_BIT_SECURITY) == DM_OP_BIT_SECURITY) ? true:false;
         break;
      case DM_OP_ALARM_EVENT_GENERAL_E:
         isActive = ((profile & DM_OP_BIT_GENERAL) == DM_OP_BIT_GENERAL) ? true:false;
         break;
      case DM_OP_ALARM_EVENT_FAULT_E:
         isActive = ((profile & DM_OP_BIT_FAULT) == DM_OP_BIT_FAULT) ? true:false;
         break;
      case DM_OP_ALARM_EVENT_ROUTING_ACK_E:
         isActive = ((profile & DM_OP_BIT_ROUTING_ACK) == DM_OP_BIT_ROUTING_ACK) ? true:false;
         break;
      default:
         isActive = false;
      break;
   }
   return isActive;
}

/*****************************************************************************
*  Function:      DM_OP_IsActivated
*  Description:   Report the 'activated' state of an output channel
*
*  param    channel        The output channel to check.
*
*  return   bool           True if the output is activated, false otherwise.
*
*  Notes:
*****************************************************************************/
bool DM_OP_IsActivated(DM_OP_OutputChannel_t channel)
{
   bool isActive = false;
   
   if ( DM_OP_CHANNEL_MAX_E > channel )
   {
      isActive = OutputState[channel].Activated;
   }
   
   return isActive;
}
/*****************************************************************************
*  Function:      DM_OP_HasPriority
*  Description:   Check the supplied channel and profile are a priority
*                 combination for the output.
*
*  param    channel        The output channel.
*  param    profile        The profile to apply.
*
*  return   bool           True if the channel has priority, false otherwise.
*
*  Notes:
*****************************************************************************/
bool DM_OP_HasPriority(const DM_OP_OutputChannel_t channel, const CO_OutputProfile_t profile)
{
   bool hasPriority = false;
   
   if ( CO_PROFILE_FIRE_E == profile )
   {
      hasPriority = true;
   }
   else if ( false == OutputState[channel].Activated )
   {
      hasPriority = true;
   }
   else 
   {
      if ( profile <= OutputState[channel].ActiveProfile )
      {
         hasPriority = true;
      }
   }
   return hasPriority;
}
/*****************************************************************************
*  Function:      DM_OP_GetChannelOutputConfig
*  Description:   Read the channel profile from NVM and decode it into the
*                 supplied AlarmOutputStateConfig_t structure
*
*  param    channel        The channel index to retrieve the profile for.
*  param    pAlarmConfig   [OUT] The structure to populate with the results.
*
*  return   bool           True if the profile was retrieved, false otherwise.
*
*  Notes:
*****************************************************************************/
ErrorCode_t DM_OP_GetChannelOutputConfig(const CO_ChannelIndex_t channel, AlarmOutputStateConfig_t *pAlarmConfig)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   DM_NVMParamId_t nvmIndex;
   uint32_t nvmConfigWord;
   
   if ( pAlarmConfig )
   {
      // Read the config from NVM.
      // First convert the supplied channel index into  an NVM index
      nvmIndex = DM_NVM_ChannelIndexToNVMParameter((uint32_t)channel);
      // If the NVM index was found
      if ( NV_MAX_PARAM_ID_E != nvmIndex )
      {
         //Read the config word from NVM
         result = DM_NVMRead(nvmIndex, &nvmConfigWord, sizeof(nvmConfigWord));
         if ( SUCCESS_E == result )
         {
            //Decode the nvmConfigWord into the output structure, pAlarmConfig.
            pAlarmConfig->ProfileMask = nvmConfigWord & NVM_OUTPUT_PROFILE_MASK;
            pAlarmConfig->Silenceable = 0 == (nvmConfigWord & NVM_OUTPUT_SILENCEABLE_MASK) ? false:true;
            pAlarmConfig->IgnoreNightDelays = 0 == (nvmConfigWord & NVM_OUTPUT_IGNORE_NIGHT_DELAYS_MASK) ? false:true;
            pAlarmConfig->Inverted = 0 == (nvmConfigWord & NVM_OUTPUT_INVERTED_MASK) ? false:true;
            pAlarmConfig->Enabled = 0 == (nvmConfigWord & NVM_OUTPUT_CHANNEL_ENABLED_MASK) ? false:true;
            //Override the local delays setting if global override is set
            if ( CFG_GetGlobalDelayOverride() )
            {
               pAlarmConfig->LocalDelays = false;
            }
            else
            {
               pAlarmConfig->LocalDelays = 0 == (nvmConfigWord & NVM_OUTPUT_LOCAL_DELAYS_MASK) ? false:true;
            }
            //If the local delays bit is set, use the values in nvmConfigWord. Otherwise get the Global delay values.
            if ( pAlarmConfig->LocalDelays )
            {
               pAlarmConfig->Delay1 = (nvmConfigWord & NVM_OUTPUT_DELAY1_MASK) >> 11;
               pAlarmConfig->Delay2 = (nvmConfigWord & NVM_OUTPUT_DELAY2_MASK) >> 20;
            }
            else 
            {
               pAlarmConfig->Delay1 = CFG_GetGlobalDelay1();
               pAlarmConfig->Delay2 = CFG_GetGlobalDelay2();
            }
         }
      }
   }
   
   return result;
}


/*************************************************************************************/
/**
* DM_OP_ProcessFirstAidAlarmCycle
* Function to cycle the first aid alarm on for 1 sec and off for 7 sec.
* milliseconds.
*
* @param    None.
*
* @return - None.
*/
void DM_OP_ProcessFirstAidAlarmCycle(void)
{
   if ( gFirstAidAlarmActive )
   {
      if ( 0 == gFirstAidAlarmCount )
      {
         if ( BASE_CONSTRUCTION_E == gBaseType )
         {
            /* sound the alarm */
            GpioWrite(&SndrBeaconOn, 1);
         }
      }
      else
      {
         if ( BASE_CONSTRUCTION_E == gBaseType )
         {
            /* silence the alarm */
            GpioWrite(&SndrBeaconOn, 0);
         }
      }
      gFirstAidAlarmCount++;
      gFirstAidAlarmCount %= 8;
   }
}

/*************************************************************************************/
/**
* DM_OP_SendOutputToHead
* Send an output signal to the Plugin
*
* @param    pOutputData.   Structure containing the output data.
*
* @return - true if the message was queued.
*/
bool DM_OP_SendOutputToHead(const OutputSignalIndication_t* pOutputData)
{
   bool result = false;
   MM_HeadDeviceType_t head_device;
   
   if ( pOutputData )
   {
      osStatus osStat = osErrorOS;
      // Send the output command to the sensor head
      CO_Message_t* pPhyDataReq = osPoolAlloc(AppPool);
      if (pPhyDataReq)
      {
         if ( SUCCESS_E == DM_MapRUChannelIndexToHeadDevice(gDeviceCombination, pOutputData->OutputChannel, &head_device) )
         {
            pPhyDataReq->Type = CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E;
            HeadMessage_t headMessage;
            headMessage.MessageType = HEAD_WRITE_OUTPUT_ACTIVATED_E;
            headMessage.ChannelNumber = (uint8_t)head_device;
            headMessage.ProfileIndex = pOutputData->OutputProfile;
            headMessage.FlashRate = pOutputData->FlashRate;
            headMessage.Value = pOutputData->OutputsActivated;
            headMessage.Duration = pOutputData->OutputDuration;
            uint8_t* pMsgData = pPhyDataReq->Payload.PhyDataReq.Data;
            memcpy(pMsgData, &headMessage, sizeof(HeadMessage_t));

            osStat = osMessagePut(HeadQ, (uint32_t)pPhyDataReq, 0);
            
            if (osOK != osStat)
            {
               /* failed to write */
               osPoolFree(AppPool, pPhyDataReq);
            }
            else
            {
               gHeadQueueCount++;
               
               /* Message was queued OK. Signal the head interface task that we have sent a message */
#ifndef USE_NEW_HEAD_INTERFACE
               osSemaphoreRelease(HeadInterfaceSemaphoreId);
#endif
               
               
               result = true;
            }
         }
      }
   }

   return result;
}




/*************************************************************************************/
/**
* DM_OP_ProcessOutputMessage
* Function for processing a received output command message.
*
* @param pOutputData       Structure containing the output requirements.
*
* @return - void
*/
void DM_OP_ProcessOutputMessage(const OutputSignalIndication_t* pOutputData)
{
   bool success = false;
   
   if ( pOutputData )
   {
      if ( (ZONE_GLOBAL == pOutputData->zone) || (pOutputData->zone == gZoneNumber) )
      {
         CO_PRINT_B_5(DBG_INFO_E, "App Rx'd o/p Signal Ind cb - Destination=%d Zone=%d OutputChannel=%d OutputProfile=%d OutputsActivated=0x%x\r\n", pOutputData->nodeID, pOutputData->zone, pOutputData->OutputChannel, pOutputData->OutputProfile, pOutputData->OutputsActivated);

         CO_ChannelIndex_t channelIndex = (CO_ChannelIndex_t)pOutputData->OutputChannel;
         //Check that it's an output channel
         DM_OP_OutputChannel_t output_channel = DM_OP_SystemChannelToOutputChannel(channelIndex);
         

         if ( DM_OP_CHANNEL_MAX_E > output_channel )
         {
            //filter out for duplicate commands
            if ( (pOutputData->OutputChannel != gLastCommandedChannel) ||
               (pOutputData->OutputProfile != gLastCommandedOutputProfile) ||
               (pOutputData->OutputsActivated != gLastCommandedOutputActive) )
            {
               // record that last output command for parent to query.
               gLastCommandedChannel = pOutputData->OutputChannel;
               gLastCommandedOutputProfile = pOutputData->OutputProfile;
               gLastCommandedOutputActive = pOutputData->OutputsActivated;
               gLastCommandedOutputDuration = pOutputData->OutputDuration;
               
					
					
               if ( CO_PROFILE_TEST_E == pOutputData->OutputProfile )
               {
                  DM_OP_TestOneShot(true);
               }
               else if ( CO_PROFILE_SILENT_TEST_E == pOutputData->OutputProfile )
               {
                  DM_OP_TestOneShot(false);
               }
               else 
               {
			
                  //Don't action alarms if they are disabled
        //          if ( (DM_RbuEnabled()) &&
          //             (CFG_ZoneEnabled()) &&
            //           (!DM_OP_ChannelDisabled(channelIndex)) )
                  {
                     switch ( gBaseType )
                     {
                        case BASE_RBU_E:
                           success = DM_OP_ProcessRBUOutputSignal(pOutputData);
                           break;
                        case BASE_IOU_E:
                           success = DM_OP_ProcessIOUnitOutputSignal(pOutputData);
                           break;
                        case BASE_MCP_E:
                           success = DM_OP_ProcessMCPOutputSignal(pOutputData);
                        break;
                        case BASE_CONSTRUCTION_E:
                           success = DM_OP_ProcessConstructionOutputSignal(pOutputData);
                           break;
                        case BASE_NCU_E:        /*intentional drop-through.  No outputs associated with these base types*/
                        case BASE_REPEATER_E:
                        default:
                           success = true;
                           break;
                     }
                  }
                  //If the command fails, don't filter out the next duplicate
                  if ( !success )
                  {
                     gLastCommandedChannel = 0;
                  }
                  
                  //STATUS INDICATOR ACTIVATION/DEACTIVATION
                  if ( CO_CHANNEL_STATUS_INDICATOR_LED_E == pOutputData->OutputChannel )
                  {
                     //Head LEDs are only activated for fire profile
                     if ( CO_PROFILE_FIRE_E == pOutputData->OutputProfile )
                     {
                        // If we have a head fitted, forward the output command to the head.
                        if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_INDICATOR_LED, DC_MATCH_ANY_E) )
                        {
                           DM_OP_SendOutputToHead(pOutputData);
                        }
                     }
                  }
               }
            }
            else
            {
               CO_PRINT_B_0(DBG_INFO_E, "Ignored duplicate output signal\r\n");
            }
         }
      }
   }
}


/*************************************************************************************/
/**
* DM_OP_ProcessConstructionOutputSignal
* Function for processing a received output command message in a construction base unit.
*
* @param pOutputData       Structure containing the output requirements.
*
* @return - bool           True on success
*/
bool DM_OP_ProcessConstructionOutputSignal(const OutputSignalIndication_t* pOutputData)
{
   bool result = false;
   uint8_t sound_level;
   
   if ( BASE_CONSTRUCTION_E == gBaseType )
   {
      if ( pOutputData )
      {
         if ( (ZONE_GLOBAL == pOutputData->zone) || (pOutputData->zone == gZoneNumber) )
         {
            if ( CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E == pOutputData->OutputChannel )
            {
               DM_OP_SetCombinedSounderVisualIndicator(pOutputData->OutputsActivated, (CO_OutputProfile_t)pOutputData->OutputProfile);
            }

            //  SVI SOUNDER ACTIVATION/DEACTIVATION
            if ( CO_CHANNEL_SOUNDER_E == pOutputData->OutputChannel )
            {
               // If we have a sounder fitted, forward the output command to the i2c.
               if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SOUNDER_85DBA, DC_MATCH_ANY_E) )
               {
                  if ( 0 != pOutputData->OutputsActivated )
                  {
                     //Convert the system channel to an output channel
                     DM_OP_OutputChannel_t op_channel = DM_OP_SystemChannelToOutputChannel((CO_ChannelIndex_t)pOutputData->OutputChannel);
                     if ( DM_OP_CHANNEL_MAX_E != op_channel )
                     {
                        //We have received a command to activate the SVI sounder                     
                        //Allow time for sounder to initialise
                        osDelay(10);
                        //Set the sound level
                        sound_level = CFG_GetSoundLevel();
                        bool atten = DM_SVI_SetToneAttenuation(sound_level);
                        //Set the tone selection
                        bool tone = DM_SVI_SetToneSelection((DM_ToneSelectionType_t)gOutputProfile[pOutputData->OutputProfile][op_channel]);
                        //Start the SVI sounder
                        bool activated = DM_SVI_SetSounder(DM_SVI_ON_E);
                        
                        if ( activated )
                        {
                           OutputState[DM_OP_CHANNEL_SOUNDER_E].Activated = true;
                           OutputState[DM_OP_CHANNEL_SOUNDER_E].ActiveProfile = (CO_OutputProfile_t)pOutputData->OutputProfile;
                        }
                        result = atten && tone && activated;
                     }
                  }
                  else 
                  {
                     //We have received a command to deactivate the SVI sounder
                     // Restore the default tone
                     bool tone = DM_SVI_SetToneSelection(DM_SVI_TONE_FIRE_E);
                     // Stop the SVI sounder
                     bool activated = DM_SVI_SetSounder(DM_SVI_OFF_E);
                     
                     if ( activated )
                     {
                        OutputState[DM_OP_CHANNEL_SOUNDER_E].Activated = false;
                        OutputState[DM_OP_CHANNEL_SOUNDER_E].ActiveProfile = CO_PROFILE_MAX_E;
                     }
                     
                     result = tone && activated;
                  }
               }
            }
            
            //BEACON ACTIVATION/DEACTIVATION
            if ( CO_CHANNEL_BEACON_E == pOutputData->OutputChannel )
            {
               // If we have a head fitted, forward the output command to the head.
               if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAD_BEACONS, DC_MATCH_ANY_E) )
               {
                  OutputSignalIndication_t output_signal;
                  output_signal.nodeID = pOutputData->nodeID;
                  output_signal.OutputChannel = CO_CHANNEL_BEACON_E;
                  output_signal.OutputProfile = pOutputData->OutputProfile;
                  output_signal.OutputsActivated = pOutputData->OutputsActivated;
                  output_signal.zone = pOutputData->zone;
                  output_signal.OutputDuration = pOutputData->OutputDuration;
                  output_signal.FlashRate = gOutputProfile[pOutputData->OutputProfile][DM_OP_CHANNEL_BEACON_E];
                  DM_OP_SendOutputToHead(&output_signal);
               }
            }

            //  Status indicator LED
            if ( CO_CHANNEL_STATUS_INDICATOR_LED_E == pOutputData->OutputChannel )
            {
               if ( SUCCESS_E == DM_OP_SetAlarmState(CO_CHANNEL_STATUS_INDICATOR_LED_E, (CO_OutputProfile_t)pOutputData->OutputProfile, pOutputData->OutputsActivated, false) )
               {
                  result = true;
               }
            }
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* DM_OP_ProcessIOUnitOutputAlarmSignal
* Function for processing a received output command message in an IO base unit.
*
* @param pOutputData       Structure containing the output requirements.
*
* @return - bool           True on success
*/
bool DM_OP_ProcessIOUnitOutputSignal(const OutputSignalIndication_t* pOutputData)
{
   bool result = false;
   if ( BASE_IOU_E == gBaseType )
   {
      if ( pOutputData )
      {
         if ( (CO_CHANNEL_OUTPUT_1_E == (CO_ChannelIndex_t)pOutputData->OutputChannel) || (CO_CHANNEL_OUTPUT_2_E == (CO_ChannelIndex_t)pOutputData->OutputChannel) )
         {
            ErrorCode_t state;
            if ( 0!= pOutputData->OutputsActivated )
            {
               state = DM_RC_WriteOutput((CO_ChannelIndex_t)pOutputData->OutputChannel, RELAY_STATE_CLOSED_E);
            }
            else
            {
               state = DM_RC_WriteOutput((CO_ChannelIndex_t)pOutputData->OutputChannel, RELAY_STATE_OPEN_E);
            }
            
            if ( SUCCESS_E == state )
            {
               result = true;
               CO_PRINT_B_2(DBG_INFO_E,"IOU set relay state to %d on channel %d\r\n", pOutputData->OutputsActivated, pOutputData->OutputChannel);
            }
            else
            {
               CO_PRINT_A_1(DBG_ERROR_E,"IOU failed to set relay state on channel %d\r\n", pOutputData->OutputChannel);
            }
         }
         //  Status indicator LED
         if ( CO_CHANNEL_STATUS_INDICATOR_LED_E == (CO_ChannelIndex_t)pOutputData->OutputChannel )
         {
            if ( SUCCESS_E == DM_OP_SetAlarmState(CO_CHANNEL_STATUS_INDICATOR_LED_E, (CO_OutputProfile_t)pOutputData->OutputProfile, pOutputData->OutputsActivated, false) )
            {
               result = true;
            }
         }
            
            /* update the child list in Application Common */
            MM_ApplicationProcessOutputMessage(pOutputData->nodeID, pOutputData->zone, pOutputData->OutputChannel, pOutputData->OutputProfile, pOutputData->OutputsActivated, pOutputData->OutputDuration);
      }
   }
   return result;
}

/*************************************************************************************/
/**
* DM_OP_ProcessMCPOutputSignal
* Function for processing a received output command message in an MCP base unit.
*
* @param pOutputData       Structure containing the output requirements.
*
* @return - bool           True on success
*/
bool DM_OP_ProcessMCPOutputSignal(const OutputSignalIndication_t* pOutputData)
{
   bool result = false;
   if ( BASE_MCP_E == gBaseType )
   {
      if ( pOutputData )
      {
         if ( (ZONE_GLOBAL == pOutputData->zone) || (pOutputData->zone == gZoneNumber) )
         {
            //  Status indicator LED
            if ( CO_CHANNEL_STATUS_INDICATOR_LED_E == pOutputData->OutputChannel )
            {
               if ( SUCCESS_E == DM_OP_SetAlarmState(CO_CHANNEL_STATUS_INDICATOR_LED_E, (CO_OutputProfile_t)pOutputData->OutputProfile, pOutputData->OutputsActivated, false) )
               {
                  result = true;
               }
            }
            
            /* update the child list in Application Common */
            MM_ApplicationProcessOutputMessage(pOutputData->nodeID, pOutputData->zone, pOutputData->OutputChannel, pOutputData->OutputProfile, pOutputData->OutputsActivated, pOutputData->OutputDuration);
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* DM_OP_ProcessRBUOutputSignal
* Function for processing a received output command message in a RBU base unit.
*
* @param pOutputData       Structure containing the output requirements.
*
* @return - bool           True on success
*/
bool DM_OP_ProcessRBUOutputSignal(const OutputSignalIndication_t* pOutputData)
{
   bool result = false;
   
   if ( BASE_RBU_E == gBaseType )
   {
      if ( pOutputData )
      {
         if ( (ZONE_GLOBAL == pOutputData->zone) || (pOutputData->zone == gZoneNumber) )
         {
            
            //  SVI SOUNDER ACTIVATION/DEACTIVATION
            if ( CO_CHANNEL_SOUNDER_E == pOutputData->OutputChannel  ||
                 CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E == pOutputData->OutputChannel)
            {
               // If we have a head fitted, forward the output command to the head.
               if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SVI_SOUNDERS, DC_MATCH_ANY_E) )
               {
                  if ( 0 != pOutputData->OutputsActivated )
                  {
                     //We have received a command to activate the SVI sounder
                     //Convert the system channel to an output channel
                     DM_OP_OutputChannel_t op_channel = DM_OP_SystemChannelToOutputChannel((CO_ChannelIndex_t)pOutputData->OutputChannel);
                     if ( DM_OP_CHANNEL_MAX_E != op_channel )
                     {
                        //Allow time for sounder to initialise
                        osDelay(10);
                        //Set the sound level
                        uint8_t sound_level = CFG_GetSoundLevel();
                        bool atten = DM_SVI_SetToneAttenuation(sound_level);
                        //Set the tone selection
                        bool tone = DM_SVI_SetToneSelection((DM_ToneSelectionType_t)gOutputProfile[pOutputData->OutputProfile][op_channel]);
                        //Start the SVI sounder
                        bool activated = DM_SVI_SetSounder(DM_SVI_ON_E);
                        if ( activated )
                        {
                           OutputState[DM_OP_CHANNEL_SOUNDER_E].Activated = true;
                           OutputState[DM_OP_CHANNEL_SOUNDER_E].ActiveProfile = (CO_OutputProfile_t)pOutputData->OutputProfile;
                        }

                        
                        result = atten && tone && activated;
                     }
                  }
                  else 
                  {
                     //We have received a command to deactivate the SVI sounder
                     // Stop the SVI sounder
                     result = DM_SVI_SetSounder(DM_SVI_OFF_E);
                     if ( result )
                     {
                        OutputState[DM_OP_CHANNEL_SOUNDER_E].Activated = false;
                        OutputState[DM_OP_CHANNEL_SOUNDER_E].ActiveProfile = CO_PROFILE_MAX_E;
                     }
                  }
               }
            }
            
            //BEACON ACTIVATION/DEACTIVATION
            if ( CO_CHANNEL_BEACON_E == pOutputData->OutputChannel )
            {
               // If we have a head fitted, forward the output command to the head.
               if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAD_BEACONS, DC_MATCH_ANY_E) )
               {
                  OutputSignalIndication_t output_signal;
                  output_signal.nodeID = pOutputData->nodeID;
                  output_signal.OutputChannel = CO_CHANNEL_BEACON_E;
                  output_signal.OutputProfile = pOutputData->OutputProfile;
                  output_signal.OutputsActivated = pOutputData->OutputsActivated;
                  output_signal.zone = pOutputData->zone;
                  output_signal.OutputDuration = pOutputData->OutputDuration;
                  output_signal.FlashRate = gOutputProfile[pOutputData->OutputProfile][DM_OP_CHANNEL_BEACON_E];
                  result = DM_OP_SendOutputToHead(&output_signal);
               }
            }

               //VISUAL INDICATOR ACTIVATION/DEACTIVATION
            if ( CO_CHANNEL_VISUAL_INDICATOR_E == pOutputData->OutputChannel )
            {
               if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_VISUAL_INDICATOR, DC_MATCH_ANY_E) )
               {
                  if ( 0 != pOutputData->OutputsActivated )
                  {
                     //We have received a command to activate the SVI visual indicator
                     //Set the tone selection
                     bool rate = DM_SVI_SetFlashRate((FlashRate_t)gOutputProfile[pOutputData->OutputProfile][DM_OP_CHANNEL_VISUAL_INDICATOR_E]);
                     //We have received a command to activate the visual indicator
                     // Switch on the Visual Indicator
                     bool indicator = DM_SVI_SetVisualIndicator(DM_SVI_ON_E);
                     
                     result = indicator && rate;
                  }
                  else 
                  {
                     //We have received a command to deactivate the visual indicator
                     // Switch off the Visual Indicator
                     result = DM_SVI_SetVisualIndicator(DM_SVI_OFF_E);
                  }
               }
            }
            
            //STATUS INDICATOR ACTIVATION/DEACTIVATION
            if ( CO_CHANNEL_STATUS_INDICATOR_LED_E == pOutputData->OutputChannel )
            {
               // Tricolour indicator LED
               DM_OP_SetAlarmState(CO_CHANNEL_STATUS_INDICATOR_LED_E, (CO_OutputProfile_t)pOutputData->OutputProfile, pOutputData->OutputsActivated, false);
            }
         }
      }
   }
   return result;
}


static CO_AlarmOutputStateData_t restoreAOS = {0};

void DM_OP_ResetAlarmOutputState( )
{
	restoreAOS.Silenceable = 0;
	restoreAOS.Unsilenceable = 0;
	restoreAOS.DelayMask = 0;
}

ErrorCode_t DM_OP_RestoreAlarmOutputState( )
{
	CO_PRINT_B_0(DBG_INFO_E,"RESTORING AOS..." );
	return DM_OP_ProcessAlarmOutputStateCommand( &restoreAOS, true ); 
}

/*************************************************************************************/
/**
* DM_OP_ProcessAlarmOutputStateCommand
* Function for processing a received 'alarm output state' command message.
*
* @param pAlarmData       Structure containing the alarm bit-fields.
*
* @return - ErrorCode_t   SUCCESS_E or error code
*/
ErrorCode_t DM_OP_ProcessAlarmOutputStateCommand(const CO_AlarmOutputStateData_t* pAlarmData, bool forceIt )
{
   AlarmOutputStateConfig_t  alarmConfig;
   ErrorCode_t result = ERR_INVALID_PARAMETER_E;
   uint32_t outputChannelIndex;
   uint32_t channelIndex;
   uint32_t bitField;
   uint32_t deviceCode;
   uint32_t event = 0;
   uint32_t bitMask = 0;
   bool command_activate = false;
   bool priority_alarm_set = false; // this stops low priority events cancelling delay timers
   CO_OutputProfile_t profile;
   bool delayIsMasked = false;
   
   if ( pAlarmData )
   {
		// copy restoreState
		restoreAOS.Unsilenceable = pAlarmData->Unsilenceable;
		restoreAOS.Silenceable = pAlarmData->Silenceable;
		restoreAOS.DelayMask = pAlarmData->DelayMask;
		
      //ignore duplicate commands
      if ( (pAlarmData->Silenceable != previous_command.Silenceable) ||
           (pAlarmData->Unsilenceable != previous_command.Unsilenceable) ||
           (pAlarmData->DelayMask != previous_command.DelayMask) ||
				forceIt )
      {
         CO_PRINT_B_3(DBG_INFO_E,"AOS s=%d, u=%d, d=%d\r\n", pAlarmData->Silenceable, pAlarmData->Unsilenceable, pAlarmData->DelayMask);
         CO_PRINT_B_3(DBG_INFO_E,"Previous AOS s=%d, u=%d, d=%d\r\n", previous_command.Silenceable, previous_command.Unsilenceable, previous_command.DelayMask);
         osDelay(50);//helps debug log messages to clear
         //Store the command values for future duplicate checks
         previous_command.Silenceable = pAlarmData->Silenceable;
         previous_command.Unsilenceable = pAlarmData->Unsilenceable;
         previous_command.DelayMask = pAlarmData->DelayMask;
         
         
         //Check that the device is enabled
         result = ERR_CONFIG_DISABLED_E;
			
         if ( (CFG_ZoneEnabled() && DM_RbuEnabled()) )		// dont do this hear - we want the			
         {
				
            result = SUCCESS_E;
            // Iterate through all output channel indexes and check to see if this unit has that channel
            for( outputChannelIndex = 0; outputChannelIndex < DM_OP_CHANNEL_MAX_E; outputChannelIndex++ )
            {
               //Clear the priority override setting of the previous channel.
               priority_alarm_set = false;
               // convert local output channel index (DM_OP_OutputChannel_t)to the global channel index (CO_ChannelIndex_t)
               channelIndex = (uint32_t)DM_OP_OutputChannelToSystemChannel((DM_OP_OutputChannel_t)outputChannelIndex);
               
               //Check that the channel is valid and that it isn't disabled
               if ( CO_CHANNEL_MAX_E > channelIndex )
               {
                  //convert channel index to device
                  result = DM_DeviceGetDeviceCode(gDeviceCombination, channelIndex, &deviceCode);
                  if ( SUCCESS_E == result )
                  {
                     //Do we have this output channel enabled on this unit?
                     if ( DM_DeviceIsEnabled(gDeviceCombination, deviceCode, DC_MATCH_ANY_E) )
                     {
                        //Get the channel profile from NVM
                        result = DM_OP_GetChannelOutputConfig((CO_ChannelIndex_t) channelIndex, &alarmConfig);
                        if ( SUCCESS_E == result )
                        {
                           //Identify the appropriate bit field from the received message
                           if ( alarmConfig.Silenceable )
                           {
                              bitField = pAlarmData->Silenceable;
                           }
                           else 
                           {
                              bitField = pAlarmData->Unsilenceable;
                           }
									 
									 
									// priority for activation
									static char priority_event_map[] ={  DM_OP_ALARM_EVENT_FIRE_E,
																					 DM_OP_ALARM_EVENT_EVACUATION_E,
																					 DM_OP_ALARM_EVENT_FIRST_AID_E,	
																					 DM_OP_ALARM_EVENT_SECURITY_E,
																					 DM_OP_ALARM_EVENT_GENERAL_E,
																					 DM_OP_ALARM_EVENT_FAULT_E,
																					 DM_OP_ALARM_EVENT_ROUTING_ACK_E,
																					 DM_OP_ALARM_EVENT_TEST_E,
																					 DM_OP_ALARM_EVENT_SILENT_TEST_E };
																 
                           
                           // Iterate the bit field to identify active events
                           for( int event_index = 0; event_index < DM_OP_ALARM_EVENT_MAX_E; event_index++ )
                           {
										event = priority_event_map[ event_index ]; 
														 
                              //Identify the correct profile for the event
                              switch ( event )
                              {
                                case DM_OP_ALARM_EVENT_FIRE_E:
                                   profile = CO_PROFILE_FIRE_E;
                                break;
                                case DM_OP_ALARM_EVENT_EVACUATION_E:
                                   profile = CO_PROFILE_EVACUATE_E;
                                break;
                                case DM_OP_ALARM_EVENT_FIRST_AID_E:
                                   profile = CO_PROFILE_FIRST_AID_E;
                                break;
                                case DM_OP_ALARM_EVENT_SECURITY_E:
                                   profile = CO_PROFILE_SECURITY_E;
                                   break;
                                case DM_OP_ALARM_EVENT_GENERAL_E:
                                   profile = CO_PROFILE_GENERAL_E;
                                   break;
                                case DM_OP_ALARM_EVENT_FAULT_E:
                                   profile = CO_PROFILE_FAULT_E;
                                   break;
                                case DM_OP_ALARM_EVENT_ROUTING_ACK_E:
                                   profile = CO_PROFILE_ROUTING_ACK_E;
                                   break;
                                case DM_OP_ALARM_EVENT_TEST_E:
                                   profile = CO_PROFILE_TEST_E;
                                break;
                                case DM_OP_ALARM_EVENT_SILENT_TEST_E:
                                   profile = CO_PROFILE_SILENT_TEST_E;
                                break;
                                default:
                                   profile = CO_PROFILE_FIRE_E;
                                break;
                              }
                              
                              bitMask = (0x01 << event);
                              
                              if ( (bitField & bitMask) &&
                                 ((CO_PROFILE_TEST_E == profile) || (CO_PROFILE_SILENT_TEST_E == profile)))
                              {
											if ( !DM_OP_ChannelDisabled((CO_ChannelIndex_t)channelIndex ) )
											{
												if ( CO_PROFILE_TEST_E == profile )
												{
													DM_OP_TestOneShot(true);
												}
												else if ( CO_PROFILE_SILENT_TEST_E == profile )
												{
													DM_OP_TestOneShot(false);
												}
											}
                                 //test mode needs to be repeatable but there is no 'OFF' command to reset the duplicate-command filter
                                 //Start a countdown for auto reset
                                 gTestModeResetCount = TEST_MODE_HOLD_OFF_COUNT;
                              }
                              else 
                              {
                                 //Check if the received message requests that the delays are skipped
                                 delayIsMasked = false;
                                 if ( 0 != (bitMask & pAlarmData->DelayMask) )
                                 {
                                    delayIsMasked = true;
                                 }
                                 // Take no action if the event is masked out in the device config
                                 if ( 0 != (bitMask & alarmConfig.ProfileMask) )
                                 {
                                    command_activate = false;
                                    if ( 0 != (bitField & bitMask) )
                                    {
                                       command_activate = true;
                                    }
												if ( DM_OP_ChannelDisabled((CO_ChannelIndex_t)channelIndex ) )
												{
													command_activate = false;
												}
												
                                    CO_PRINT_B_5(DBG_INFO_E,"profile=%d, delayIsMasked=%d, GlobalDelayEnabled=%d, Delay1=%d, DayNight=%d\r\n", 
                                       profile, delayIsMasked, CFG_GetGlobalDelayEnabled(), alarmConfig.Delay1, CFG_GetDayNight());
                                    CO_PRINT_B_3(DBG_INFO_E,"IgnoreNightDelays=%d, event=%d, activate=%d\r\n", alarmConfig.IgnoreNightDelays, event, command_activate);
                                    osDelay(1);//busy debug time
												
                                    // Check for delays in the config and start them if required
                                    if ( !delayIsMasked &&
                                         CFG_GetGlobalDelayEnabled() &&
                                         0 < alarmConfig.Delay1 &&
                                         ( CO_DAY_E == CFG_GetDayNight() || false == alarmConfig.IgnoreNightDelays ) )
                                    {
                                       if ( command_activate )
                                       {
                                          //If the channel timer isn't already running, start it now
                                          if ( DM_OP_TIMER_NOT_RUNNING_E == OutputState[outputChannelIndex].Delay1State )
                                          {
                                             OutputState[outputChannelIndex].Delay1State = DM_OP_TIMER_RUNNING_E;
                                             OutputState[outputChannelIndex].Delay1Count = 0;
                                             OutputState[outputChannelIndex].ActiveProfile = profile;
                                             priority_alarm_set = true;
                                          }
                                       }
                                       else 
                                       {
                                          // The command was to cancel the alarm.  Stop the timers and deactivate the output if it's already active
                                          //Don't cancel alarms if a higher priority event just started them
                                          if ( false == priority_alarm_set )
                                          {
                                             OutputState[outputChannelIndex].Delay1State = DM_OP_TIMER_NOT_RUNNING_E;
                                             OutputState[outputChannelIndex].Delay1Count = 0;
                                             OutputState[outputChannelIndex].Delay2State = DM_OP_TIMER_NOT_RUNNING_E;
                                             OutputState[outputChannelIndex].Delay2Count = 0;
                                             
                                             if ( OutputState[outputChannelIndex].ActiveProfile == profile )
                                             {
                                                if ( OutputState[outputChannelIndex].Activated )
                                                {
                                                   //Ignore inverted flag for all but DM_OP_CHANNEL_OUTPUT_1_E and DM_OP_CHANNEL_OUTPUT_2_E
                                                   if ( (DM_OP_CHANNEL_OUTPUT_1_E == outputChannelIndex || DM_OP_CHANNEL_OUTPUT_2_E == outputChannelIndex ) && alarmConfig.Inverted )
                                                   {
                                                      CO_PRINT_B_2(DBG_INFO_E,"DM_OP_SetAlarmState1 %d, %d, ON\r\n", channelIndex, profile);
                                                      //activate channel
                                                      result = DM_OP_SetAlarmState((CO_ChannelIndex_t)channelIndex, profile, true, false);
                                                   }
                                                   else 
                                                   {
                                                      CO_PRINT_B_2(DBG_INFO_E,"DM_OP_SetAlarmState2 %d, %d OFF\r\n", channelIndex, profile);
                                                      //deactivate channel
                                                      result = DM_OP_SetAlarmState((CO_ChannelIndex_t)channelIndex, profile, false, false);
                                                   }
                                                }
                                             }
                                          }
                                       }
                                    }
                                    else if ( false == priority_alarm_set )
                                    {
                                       if ( command_activate )
                                       {
                                          priority_alarm_set = true;
                                          //Cancel any previous delays that may be in progress
                                          OutputState[outputChannelIndex].Delay1State = DM_OP_TIMER_NOT_RUNNING_E;
                                          OutputState[outputChannelIndex].Delay1Count = 0;
                                          OutputState[outputChannelIndex].Delay2State = DM_OP_TIMER_NOT_RUNNING_E;
                                          OutputState[outputChannelIndex].Delay2Count = 0;
                                       }
                                       //No delays are configured.  Apply the command to the output.
                                       //Ignore inverted flag for all but DM_OP_CHANNEL_OUTPUT_1_E and DM_OP_CHANNEL_OUTPUT_2_E
                                       if ( (DM_OP_CHANNEL_OUTPUT_1_E == outputChannelIndex || DM_OP_CHANNEL_OUTPUT_2_E == outputChannelIndex ) && alarmConfig.Inverted )
                                       {
                                          CO_PRINT_B_3(DBG_INFO_E,"DM_OP_SetAlarmState3 %d, %d %s\r\n", channelIndex, profile, alarmConfig.Inverted ? "OFF":"ON");
                                         //deactivate channel
                                         result = DM_OP_SetAlarmState((CO_ChannelIndex_t)channelIndex, profile, !command_activate, false);
                                       }
                                       else 
                                       {
                                         //activate channel
                                         result = DM_OP_SetAlarmState((CO_ChannelIndex_t)channelIndex, profile, command_activate, false);
                                          CO_PRINT_B_3(DBG_INFO_E,"DM_OP_SetAlarmState4 %d, %d %s\r\n", channelIndex, profile, alarmConfig.Inverted ? "ON":"OFF");
                                       }
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }

            gLastCommandedSilenceableMask = pAlarmData->Silenceable;
            gLastCommandedUnsilenceableMask = pAlarmData->Unsilenceable;
            gLastCommandedDelayMask = pAlarmData->DelayMask;
         }
      }
      else
      {
         CO_PRINT_B_0(DBG_INFO_E,"AOS ignored duplicate command\r\n");
         //Duplicate command.  Report success.
         result = SUCCESS_E;
      }
   }

   return result;
}

/*************************************************************************************/
/**
* DM_OP_SetAlarmState
* Activate or deactivate the specified output channel using the supplied profile.
*
* @param systemChannel     The channel to activate/deactivate.
* @param profile           The output profile to apply.
* @param activated         True to turn on output, false to turn off.
* @param ignoreDisablement True to ignore disablement in config.
*
* @return - ErrorCode_t    SUCCESS_E or error code
*/
ErrorCode_t DM_OP_SetAlarmState(const CO_ChannelIndex_t systemChannel, const CO_OutputProfile_t profile, bool activated, const bool ignoreDisablement)
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   DM_OP_OutputChannel_t outputChannel;
   
   //Convert the system channel index to an output channel
   outputChannel = DM_OP_SystemChannelToOutputChannel(systemChannel);

   if ( DM_OP_CHANNEL_MAX_E != outputChannel )
   {		 
		if ( DM_OP_ChannelDisabled(systemChannel) )
		{
			activated = false;
			CO_PRINT_B_0(DBG_INFO_E,"Setting alarm state to off.");
		}
		
      // Don't respond to active alarms if the channel is disabled, but allow outputs to be turned off.
  //    if ( ignoreDisablement || !DM_OP_ChannelDisabled(systemChannel) )
      {
         switch ( outputChannel )
         {
            case DM_OP_CHANNEL_SOUNDER_E:
               result = DM_OP_SetSounderAlarmState(activated, profile);
               break;
            case DM_OP_CHANNEL_BEACON_E:
               result = DM_OP_SetBeaconAlarmState(activated, profile);
               break;
            case DM_OP_CHANNEL_STATUS_INDICATOR_LED_E:
               result = DM_OP_SetIndicatorLEDState(activated, profile);
            break;
            case DM_OP_CHANNEL_VISUAL_INDICATOR_E:
               result = DM_OP_SetVisualIndicator(activated, profile);
               break;
            case DM_OP_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E:
               result = DM_OP_SetCombinedSounderVisualIndicator(activated, profile);
               break;
            case DM_OP_CHANNEL_OUTPUT_1_E:   // intentional drop-through
            case DM_OP_CHANNEL_OUTPUT_2_E:
               result = DM_OP_SetOutputChannelState(systemChannel, activated, profile);
               break;
            default:
               result = ERR_NOT_SUPPORTED_E;
            break;
         }
      }
      if ( !ignoreDisablement && DM_OP_ChannelDisabled(systemChannel) )
      {
         result = ERR_CONFIG_DISABLED_E;
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* DM_OP_SetSounderAlarmState
* Activate or deactivate the SOUNDER output channel.
*
* @param activated         True to activate alarm, false to deactivate.
* @param profile           The output profile to apply.
*
* @return - ErrorCode_t    SUCCESS_E or error code
*/
ErrorCode_t DM_OP_SetSounderAlarmState(const bool activated, const CO_OutputProfile_t profile)
{
   ErrorCode_t result = ERR_NOT_SUPPORTED_E;
   
   switch ( gBaseType )
   {
      case BASE_CONSTRUCTION_E: // intentional drop-through
         // If we have an 85dBa sounder board
         if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SOUNDER_85DBA, DC_MATCH_ANY_E) )
         {
            if ( activated )
            {
               //We have received a command to activate the construction combined sounder
               //Set the tone for the profile
               DM_SVI_SetToneSelection((DM_ToneSelectionType_t)gOutputProfile[profile][DM_OP_CHANNEL_SOUNDER_E]);
               //Set the volume
               uint8_t sound_level = CFG_GetSoundLevel();
               DM_SVI_SetToneAttenuation(sound_level);
               //Start the SVI sounder
               if ( DM_SVI_SetSounder(DM_SVI_ON_E) )
               {
                  OutputState[DM_OP_CHANNEL_SOUNDER_E].Activated = true;
                  OutputState[DM_OP_CHANNEL_SOUNDER_E].ActiveProfile = profile;
                  result = SUCCESS_E;
               }
            }
            else 
            {
               //don't send the off command unless the sounder is active, otherwise it gives an annoying chirp.
               if ( OutputState[DM_OP_CHANNEL_SOUNDER_E].Activated )
               {
                  //We have received a command to deactivate the SVI sounder
                  // Stop the SVI sounder
                  if ( DM_SVI_SetSounder(DM_SVI_OFF_E) )
                  {
                     OutputState[DM_OP_CHANNEL_SOUNDER_E].Activated = false;
                     OutputState[DM_OP_CHANNEL_SOUNDER_E].ActiveProfile = CO_PROFILE_MAX_E;
                     result = SUCCESS_E;
                  }
               }
            }
         }
         
         // If we have a combined sounder and visual indicator.
         if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SOUNDER_VI_CONSTR, DC_MATCH_ANY_E) )
         {
            DM_OP_SetCombinedSounderVisualIndicator(activated, profile);
         }
         break;
      case BASE_RBU_E:
         // If we have an SVI sounder fitted, apply the activation state.
         if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SVI_SOUNDERS, DC_MATCH_ANY_E) )
         {
            if ( activated )
            {
               //We have received a command to activate the construction combined sounder
               //Set the tone for the profile
               DM_SVI_SetToneSelection((DM_ToneSelectionType_t)gOutputProfile[profile][DM_OP_CHANNEL_SOUNDER_E]);
               //Set the volume
               uint8_t sound_level = CFG_GetSoundLevel();
               DM_SVI_SetToneAttenuation(sound_level);
               //Start the SVI sounder
               if ( DM_SVI_SetSounder(DM_SVI_ON_E) )
               {
                  OutputState[DM_OP_CHANNEL_SOUNDER_E].Activated = true;
                  OutputState[DM_OP_CHANNEL_SOUNDER_E].ActiveProfile = profile;
                  result = SUCCESS_E;
               }
            }
            else 
            {
               //We have received a command to deactivate the SVI sounder
               //don't send the off command unless the sounder is active, otherwise it gives an annoying chirp.
               if ( OutputState[DM_OP_CHANNEL_SOUNDER_E].Activated )
               {
                  // Stop the SVI sounder
                  if ( DM_SVI_SetSounder(DM_SVI_OFF_E) )
                  {
                     OutputState[DM_OP_CHANNEL_SOUNDER_E].Activated = false;
                     OutputState[DM_OP_CHANNEL_SOUNDER_E].ActiveProfile = CO_PROFILE_MAX_E;
                     result = SUCCESS_E;
                  }
               }
            }
         }
         break;
      default:
         result = ERR_NOT_SUPPORTED_E;
         break;
   }
   
   return result;
}

/*************************************************************************************/
/**
* DM_OP_SetBeaconAlarmState
* Activate or deactivate the BEACON output channel.
*
* @param activated         True to activate beacon, false to deactivate.
* @param profile           The output profile to apply.
*
* @return - ErrorCode_t    SUCCESS_E or error code
*/
ErrorCode_t DM_OP_SetBeaconAlarmState(const bool activated, const CO_OutputProfile_t profile)
{
   ErrorCode_t result = ERR_NOT_SUPPORTED_E;
   
   CO_PRINT_B_2(DBG_INFO_E,"DM_OP_SetBeaconAlarmState profile=%d, flash=%d\r\n",profile, gOutputProfile[profile][DM_OP_CHANNEL_BEACON_E]);
   
   switch ( gBaseType )
   {
      case BASE_RBU_E:
         result = ERR_NOT_FOUND_E;
         // If we have a beacon fitted, apply the activation state.
         if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAD_BEACONS, DC_MATCH_ANY_E) )
         {
            result = ERR_MESSAGE_FAIL_E;
            OutputSignalIndication_t outputData;
            outputData.OutputChannel = CO_CHANNEL_BEACON_E;
            outputData.OutputProfile = profile;
            outputData.OutputDuration = 0;//not used
            
            //Get the flash rate for the profile from the config
            outputData.FlashRate = gOutputProfile[profile][DM_OP_CHANNEL_BEACON_E];
            
            
            if ( activated )
            {
               outputData.OutputsActivated = 1;
            }
            else 
            {
               outputData.OutputsActivated = 0;
            }
            
            if( DM_OP_SendOutputToHead(&outputData) )
            {
               OutputState[DM_OP_CHANNEL_BEACON_E].Activated = activated;
               result = SUCCESS_E;
            }
         }
         break;
      default:
         result = ERR_NOT_SUPPORTED_E;
         break;
   }
   
   return result;
}

/*************************************************************************************/
/**
* DM_OP_SetIndicatorLEDState
* Activate or deactivate the LED status indicator output channel.
*
* @param activated         True to activate beacon, false to deactivate.
* @param profile           The output profile to apply.
*
* @return - ErrorCode_t    SUCCESS_E or error code
*/
ErrorCode_t DM_OP_SetIndicatorLEDState(const bool activated, const CO_OutputProfile_t profile)
{
   ErrorCode_t result = ERR_NOT_SUPPORTED_E;
   
   switch ( gBaseType )
   {
      case BASE_CONSTRUCTION_E: //intentional drop-through
      case BASE_RBU_E:
         result = ERR_NOT_FOUND_E;
         // If we have an indicator LED fitted, apply the activation state.
         if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_INDICATOR_LED, DC_MATCH_ANY_E) )
         {
            //Head LEDs are only activated for fire profile
            if ( CO_PROFILE_FIRE_E == profile )
            {
               result = ERR_MESSAGE_FAIL_E;
               OutputSignalIndication_t outputData;
               outputData.OutputChannel = (uint8_t)CO_CHANNEL_STATUS_INDICATOR_LED_E;
               outputData.OutputProfile = profile;
               outputData.OutputsActivated = activated;
               outputData.OutputDuration = 0;//not used
               if ( activated )
               {
                  outputData.OutputsActivated = 1;
               }
               else 
               {
                  outputData.OutputsActivated = 0;
               }
               
               if( DM_OP_SendOutputToHead(&outputData) )
               {
                  OutputState[DM_OP_CHANNEL_STATUS_INDICATOR_LED_E].Activated = activated;
                  result = SUCCESS_E;
               }
            }
            else 
            {
               result = SUCCESS_E;
            }
         }

         //Set the onboard LED.
         if ( CO_PROFILE_FIRE_E == profile )
         {
            //fire profile uses the red led
            if ( activated )
            {
               DM_LedPatternRequest(LED_FIRE_INDICATION_E);
            }
            else 
            {
               /* Do not turn OFF red LED for smoke and heat devices */
               /* DM_LedPatternRemove(LED_FIRE_INDICATION_E);        */
            }
         }
         else 
         {
            //non-fire profiles use the green led
            if ( activated )
            {
               DM_LedPatternRequest(LED_FIRST_AID_INDICATION_E);
            }
            else 
            {
               DM_LedPatternRemove(LED_FIRST_AID_INDICATION_E);
            }
         }
         break;
      case BASE_IOU_E:                    //Intentional drop-through
      case BASE_MCP_E:
         //No status indicator fitted.  Use the onboard LED.
         if ( CO_PROFILE_FIRE_E == profile )
         {
            //fire profile uses the red led
            if ( activated )
            {
               DM_LedPatternRequest(LED_FIRE_INDICATION_E);
            }
            else 
            {
               DM_LedPatternRemove(LED_FIRE_INDICATION_E);
            }
         }
         else 
         {
            //non-fire profiles use the green led
            if ( activated )
            {
               DM_LedPatternRequest(LED_FIRST_AID_INDICATION_E);
            }
            else 
            {
               DM_LedPatternRemove(LED_FIRST_AID_INDICATION_E);
            }
         }
         break;
      default:
         result = ERR_NOT_SUPPORTED_E;
         break;
   }
   
   return result;
}

/*************************************************************************************/
/**
* DM_OP_SetVisualIndicator
* Activate or deactivate the SVI visual indicator output channel.
*
* @param activated         True to activate visual indicator, false to deactivate.
* @param profile           The output profile to apply.
*
* @return - ErrorCode_t    SUCCESS_E or error code
*/
ErrorCode_t DM_OP_SetVisualIndicator(const bool activated, const CO_OutputProfile_t profile)
{
   ErrorCode_t result = ERR_NOT_SUPPORTED_E;

   switch ( gBaseType )
   {
      case BASE_RBU_E:           // intentional drop-through
      case BASE_CONSTRUCTION_E:
         result = ERR_NOT_FOUND_E;
         // If we have an SVI visual indicator fitted, apply the activation state.
         if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_VISUAL_INDICATOR, DC_MATCH_ANY_E) )
         {
            FlashRate_t flashRate = (FlashRate_t)gOutputProfile[profile][DM_OP_CHANNEL_VISUAL_INDICATOR_E];
            if( DM_SVI_SetFlashRate(flashRate) )
            {
               uint8_t onOff = DM_SVI_OFF_E;
               if ( activated )
               {
                  onOff = DM_SVI_ON_E;
               }
               if ( DM_SVI_SetVisualIndicator(onOff) )
               {
                  result = SUCCESS_E;
                  OutputState[DM_OP_CHANNEL_VISUAL_INDICATOR_E].Activated = activated;
               }
            }
         }
         break;
      default:
         result = ERR_NOT_SUPPORTED_E;
         break;
   }
   
   return result;
}

/*************************************************************************************/
/**
* DM_OP_SetCombinedSounderVisualIndicator
* Activate or deactivate the combined sounder and visual indicator output channel.
*
* @param activated         True to activate sounder & visual indicator, false to deactivate.
* @param profile           The output profile to apply.
*
* @return - ErrorCode_t    SUCCESS_E or error code
*/
ErrorCode_t DM_OP_SetCombinedSounderVisualIndicator(const bool activated, const CO_OutputProfile_t profile)
{
   ErrorCode_t result = ERR_NOT_SUPPORTED_E;
   
   switch ( gBaseType )
   {
      case BASE_CONSTRUCTION_E:
         result = ERR_NOT_FOUND_E;
         // If we have a combined sounder and visual indicator fitted, apply the activation state.
         if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SOUNDER_VI_CONSTR, DC_MATCH_ANY_E) )
         {
            //Only respond if the channel profile has priority, matches the active profile or there is no active profile set.
            if ( DM_OP_HasPriority(DM_OP_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E, profile) ||
               (OutputState[DM_OP_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E].ActiveProfile == profile) ||
                 (OutputState[DM_OP_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E].ActiveProfile == CO_PROFILE_MAX_E) )
            {
               if ( activated )
               {
                  OutputState[DM_OP_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E].ActiveProfile = profile;
               }
               else if ( profile == OutputState[DM_OP_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E].ActiveProfile )
               {
                  OutputState[DM_OP_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E].ActiveProfile = CO_PROFILE_MAX_E;
               }
               OutputState[DM_OP_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E].Activated = activated;
               result = SUCCESS_E;
               
               //Don't interfere with the sounder if a higher priority is active
               if ( DM_OP_HasPriority(DM_OP_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E, profile) )
               {
                  //Start/stop the event timer that drives the SiteNet sounder sequence
                  TE_SetSitenetSounderOutput(activated);
                  GpioWrite(&SndrBeaconOn, activated);
               }
            }
         }
         break;
      default:
         result = ERR_NOT_SUPPORTED_E;
         break;
   }
   
   return result;
}

/*************************************************************************************/
/**
* DM_OP_SetOutputChannelState
* Activate or deactivate the output relays.
*
* @param activated         True to activate the relay, false to deactivate.
* @param profile           The output profile to apply.
*
* @return - ErrorCode_t    SUCCESS_E or error code
*/
ErrorCode_t DM_OP_SetOutputChannelState(CO_ChannelIndex_t channel, const bool activated, const CO_OutputProfile_t profile)
{
   ErrorCode_t result = ERR_NOT_SUPPORTED_E;
   AlarmOutputStateConfig_t alarmConfig;
   uint32_t device_code;
   bool normalised_activated;//accounts for inverted output
   
   //Make sure that channel maps to an output
   DM_OP_OutputChannel_t output_channel = DM_OP_SystemChannelToOutputChannel(channel);
   if ( DM_OP_CHANNEL_OUTPUT_1_E == output_channel || DM_OP_CHANNEL_OUTPUT_2_E == output_channel )
   {
      //map the channel index to a device type
      result = DM_DeviceGetDeviceCode(gDeviceCombination, channel, &device_code);
      if ( SUCCESS_E == result )
      {
         switch ( gBaseType )
         {
            case BASE_IOU_E:
               result = ERR_NOT_FOUND_E;
               // If we have the output device, apply the activation state.
               if ( DM_DeviceIsEnabled(gDeviceCombination, device_code, DC_MATCH_ANY_E) )
               {
                  //Get the channel profile from NVM
                  result = DM_OP_GetChannelOutputConfig(channel, &alarmConfig);
                  if ( SUCCESS_E == result )
                  {

                     if ( activated )
                     {
                        result = DM_RC_WriteOutput(channel, RELAY_STATE_CLOSED_E);
                     }
                     else
                     {
                        result = DM_RC_WriteOutput(channel, RELAY_STATE_OPEN_E);
                     }
                     
                     if ( SUCCESS_E == result )
                     {
                        if ( alarmConfig.Inverted )
                        {
                           normalised_activated = !activated;
                     }
                     else
                     {
                           normalised_activated = activated;
                     }
                     
                        OutputState[output_channel].Activated = normalised_activated;
                     if ( activated )
                     {
                           OutputState[output_channel].ActiveProfile = profile;
                     }
                     else
                     {
                           OutputState[output_channel].ActiveProfile = CO_PROFILE_MAX_E;
                     }
                        CO_PRINT_B_3(DBG_INFO_E,"IOU set relay state to %d on channel %d, profile %d\r\n", activated, channel, profile);
                  }
                     else
                     {
                        CO_PRINT_A_2(DBG_ERROR_E,"IOU failed to set relay state on channel %d, profile %d\r\n", channel, profile);
               }
                  }
               }
               break;
            default:
               result = ERR_NOT_SUPPORTED_E;
               break;
         }
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* DM_OP_ManageDelays
* Check the alarm status for any running delays and perform actions when delays expire.
*
* @param  - None.
*
* @return - None.
*/
void DM_OP_ManageDelays(void)
{
   ErrorCode_t result;
   uint32_t channelIndex;
   uint32_t output_channel;
   AlarmOutputStateConfig_t  alarmConfig;
   uint32_t deviceCode;
   uint16_t delay1;
   uint16_t delay2;
   
   
   //Iterate through each output channel
   for( output_channel = 0; output_channel < DM_OP_CHANNEL_MAX_E; output_channel++)
   {
      // convert local output channel index (DM_OP_OutputChannel_t) to the global channel index (CO_ChannelIndex_t)
      channelIndex = (uint32_t)DM_OP_OutputChannelToSystemChannel((DM_OP_OutputChannel_t)output_channel);
      //convert channel index to device
      result = DM_DeviceGetDeviceCode(gDeviceCombination, channelIndex, &deviceCode);
      if ( SUCCESS_E == result )
      {
         //Do we have this output channel enabled on this unit?
         if ( DM_DeviceIsEnabled(gDeviceCombination, deviceCode, DC_MATCH_ANY_E) )
         {
            //Get the channel profile from NVM
            result = DM_OP_GetChannelOutputConfig((CO_ChannelIndex_t) channelIndex, &alarmConfig);
            if ( SUCCESS_E == result )
            {
               // Is delay1 running?
               if ( DM_OP_TIMER_RUNNING_E == OutputState[output_channel].Delay1State )
               {
                  //set the timeout values to global or local setting
                  if ( CFG_GetGlobalDelayOverride() )
                  {
                     //The global delay override is in force.  Use the global delays
                     delay1 = CFG_GetGlobalDelay1();
                  }
                  else 
                  {
                     //The global delay override is not in force.  Use the local delays.
                     delay1 = alarmConfig.Delay1;
                  }
                  // increment the delay1 count
                  OutputState[output_channel].Delay1Count++;
                  CO_PRINT_B_2(DBG_INFO_E,"OP Channel %d delay1 count = %d\r\n", output_channel, OutputState[output_channel].Delay1Count);
                  //has the timer expired
                  if ( OutputState[output_channel].Delay1Count >= delay1 )
                  {
                     OutputState[output_channel].Delay1State = DM_OP_TIMER_EXPIRED_E;
                     OutputState[output_channel].Delay1Count = 0;
                     // If delay 1 expires without receiving an ACK, we skip delay 2
                     OutputState[output_channel].Delay2State = DM_OP_TIMER_EXPIRED_E;
                     OutputState[output_channel].Delay2Count = 0;
                  }
               }
               
               // Is delay2 running?
               if ( DM_OP_TIMER_RUNNING_E == OutputState[output_channel].Delay2State )
               {
                  //set the timeout values to global or local setting
                  if ( CFG_GetGlobalDelayOverride() )
                  {
                     //The global delay override is in force.  Use the global delays
                     delay2 = CFG_GetGlobalDelay2();
                  }
                  else 
                  {
                     //The global delay override is not in force.  Use the local delays.
                     delay2 = alarmConfig.Delay2;
                  }
                  // increment the delay2 count
                  OutputState[output_channel].Delay2Count++;
                  CO_PRINT_B_2(DBG_INFO_E,"OP Channel %d delay2 count = %d\r\n", output_channel, OutputState[output_channel].Delay2Count);
                  //has the timer expired
                  if ( OutputState[output_channel].Delay2Count >= delay2 )
                  {
                     OutputState[output_channel].Delay2State = DM_OP_TIMER_EXPIRED_E;
                     OutputState[output_channel].Delay2Count = 0;
                  }
               }
               
               //Are both delay expired?
               if ( DM_OP_TIMER_EXPIRED_E == OutputState[output_channel].Delay1State &&
                    DM_OP_TIMER_EXPIRED_E == OutputState[output_channel].Delay2State )
               {
                  //both delays have expired.  Reset the delays.
                  OutputState[output_channel].Delay1State = DM_OP_TIMER_NOT_RUNNING_E;
                  OutputState[output_channel].Delay2State = DM_OP_TIMER_NOT_RUNNING_E;
                  
                  CO_PRINT_B_1(DBG_INFO_E,"OP Channel %d alarm ACTIVE\r\n", output_channel);
                  OutputState[output_channel].Activated = true;
                  
                  //Trigger the output.
                 if ( (DM_OP_CHANNEL_OUTPUT_1_E == output_channel || DM_OP_CHANNEL_OUTPUT_2_E == output_channel ) && alarmConfig.Inverted )
                 {
                    CO_PRINT_B_2(DBG_INFO_E,"DM_OP_SetAlarmState5 %d, %d OFF\r\n", channelIndex, OutputState[output_channel].ActiveProfile);
                    //deactivate channel
                    result = DM_OP_SetAlarmState((CO_ChannelIndex_t)channelIndex, OutputState[output_channel].ActiveProfile, false, false);
                 }
                 else 
                 {
                    CO_PRINT_B_2(DBG_INFO_E,"DM_OP_SetAlarmState6 %d, %d ON\r\n", channelIndex, OutputState[output_channel].ActiveProfile);
                    //activate channel
                    result = DM_OP_SetAlarmState((CO_ChannelIndex_t)channelIndex, OutputState[output_channel].ActiveProfile, true, false);
                 }
               }
               
               //It the timed output running
               if ( OutputState[output_channel].TimedOutputActive )
               {
                  //decrease the timer count
                  OutputState[output_channel].OutputTimeoutCount--;
                  //Has the timeout expired?
                  if ( 0 == OutputState[output_channel].OutputTimeoutCount )
                  {
                     CO_PRINT_B_2(DBG_INFO_E,"DM_OP_SetAlarmState7 %d, %d OFF\r\n", channelIndex, OutputState[output_channel].ActiveProfile);
                     //Switch off the output
                     result = DM_OP_SetAlarmState((CO_ChannelIndex_t)channelIndex, OutputState[output_channel].ActiveProfile, false, false);
                     //Reset the active output properties
                     OutputState[DM_OP_CHANNEL_SOUNDER_E].Activated = false;
                     OutputState[DM_OP_CHANNEL_SOUNDER_E].ActiveProfile = CO_PROFILE_MAX_E;
                     //Set the switch off duration
                     OutputState[DM_OP_CHANNEL_SOUNDER_E].OutputTimeoutCount = 0;
                     //Start timing
                     OutputState[DM_OP_CHANNEL_SOUNDER_E].TimedOutputActive = false;
                  }
               }
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* DM_OP_AlarmAcknowledge
* Process the alarm Acknowledgement from the control panel.
*
* @param  - None.
*
* @return - SUCCESS_E.
*/
ErrorCode_t DM_OP_AlarmAcknowledge(const CO_OutputProfile_t profile)
{
   uint32_t output_channel;
   uint32_t channel_index;
   ErrorCode_t result;
   AlarmOutputStateConfig_t alarmConfig;
   
   //Iterate through each output channel
   for( output_channel = 0; output_channel < DM_OP_CHANNEL_MAX_E; output_channel++)
   {
      channel_index = (uint32_t)DM_OP_OutputChannelToSystemChannel((DM_OP_OutputChannel_t)output_channel);
      if ( CO_CHANNEL_NONE_E != channel_index )
      {
         result = DM_OP_GetChannelOutputConfig((CO_ChannelIndex_t) channel_index, &alarmConfig);
         
         if ( SUCCESS_E == result )
         {
            if ( CO_PROFILE_FIRE_E == profile )
            {
               // If delay 1 is running, cancel it and start delay 2
               if ( (DM_OP_TIMER_RUNNING_E == OutputState[output_channel].Delay1State) &&
                    (CO_PROFILE_FIRE_E == OutputState[output_channel].ActiveProfile) &&
                     (0 < alarmConfig.Delay2) )
               {
                  OutputState[output_channel].Delay1State = DM_OP_TIMER_EXPIRED_E;
                  OutputState[output_channel].Delay1Count = 0;
                  OutputState[output_channel].Delay2State = DM_OP_TIMER_RUNNING_E;
                  OutputState[output_channel].Delay2Count = 0;
               }
            }
            else 
            {
               if ( (DM_OP_TIMER_RUNNING_E == OutputState[output_channel].Delay1State) &&
                    (CO_PROFILE_FIRE_E != OutputState[output_channel].ActiveProfile) &&
                    (0 < alarmConfig.Delay2) )
               {
                  OutputState[output_channel].Delay1State = DM_OP_TIMER_EXPIRED_E;
                  OutputState[output_channel].Delay1Count = 0;
                  OutputState[output_channel].Delay2State = DM_OP_TIMER_RUNNING_E;
                  OutputState[output_channel].Delay2Count = 0;
               }
            }
         }
      }
   }
   
   return SUCCESS_E;
}

/*************************************************************************************/
/**
* DM_OP_AlarmEvacuate
* Process the Evacuate command from the control panel.
*
* @param  - None.
*
* @return - Error code.
*/
ErrorCode_t DM_OP_AlarmEvacuate(void)
{
   ErrorCode_t result;
   uint32_t output_channel;
   //Iterate through each output channel cancelling delays
   for( output_channel = 0; output_channel < DM_OP_CHANNEL_MAX_E; output_channel++)
   {
      // If delay 1 is running, cancel it.
      if ( DM_OP_TIMER_NOT_RUNNING_E != OutputState[output_channel].Delay1State )
      {
         OutputState[output_channel].Delay1State = DM_OP_TIMER_NOT_RUNNING_E;
         OutputState[output_channel].Delay1Count = 0;
      }
      // If delay 2 is running, cancel it.
      if ( DM_OP_TIMER_NOT_RUNNING_E != OutputState[output_channel].Delay2State )
      {
         OutputState[output_channel].Delay2State = DM_OP_TIMER_NOT_RUNNING_E;
         OutputState[output_channel].Delay2Count = 0;
      }
   }
   
   //Set the alarms with the evacuate profile
   CO_AlarmOutputStateData_t alarmData;
   alarmData.Silenceable = DM_OP_BIT_EVACUATION;
   alarmData.Unsilenceable = DM_OP_BIT_EVACUATION;
   alarmData.DelayMask = 0;//no delay

   result = DM_OP_ProcessAlarmOutputStateCommand( &alarmData, false );
   
   if ( SUCCESS_E != result )
   {
      CO_PRINT_A_1( DBG_ERROR_E, "Failed to initiate alarm output.  Error=%d\r\n", result);
   }
   
   return result;
}

void MM_ApplicationSendAlarmSignals(void);
void MM_ApplicationSendFaultReports( const bool sendOnDULCH );

/*************************************************************************************/
/**
* DM_OP_SetDisablement
* Set the day/night flags for channel disablement.
*
* @param  - channel        The channel to modify.
* @param  - enableDisable  The settings to apply to the channel.
*
* @return - Error code.
*/
ErrorCode_t DM_OP_SetDisablement(const uint16_t channel, const DM_Enable_t enableDisable)
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   if ( CO_CHANNEL_MAX_E > channel )
   {
      //update the dynamic record
      gChannelDisabled[channel] = enableDisable;
      
      //record the setting in NVM
      uint32_t nvm_disable = ((uint32_t)enableDisable << 30);//NVM is shared with other config properties. Bit shift to enable bits 30 & 31
      
      //check that the channel has an associated NVM slot
      DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(channel); 
      
      if ( NV_MAX_PARAM_ID_E != nvm_param )
      {
         // Read the NVM
         uint32_t nvm_value;
         if ( SUCCESS_E == DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
         {
				// if different
				if ( ( nvm_value & NVM_OUTPUT_CHANNEL_ENABLED_MASK ) != ( nvm_disable & NVM_OUTPUT_CHANNEL_ENABLED_MASK ) )
				{
					//Mask out the output profile bits from the value read from NVM above
					nvm_value &= ~NVM_OUTPUT_CHANNEL_ENABLED_MASK;
					
					// check that the supplied profile doesn't extend beyond the bit field for the profile
					if ( 0 == ( nvm_disable & ~NVM_OUTPUT_CHANNEL_ENABLED_MASK ) )
					{
						//  Apply the bit field and write back to NVM
						nvm_value |= nvm_disable;
						if ( SUCCESS_E == DM_NVMWrite(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
						{
							DM_OP_RestoreAlarmOutputState( );
							
							if ( enableDisable != DM_ENABLE_NONE_E )
							{
								//Send any existing alarm states
								MM_ApplicationSendAlarmSignals();
								MM_ApplicationSendFaultReports( false );
							}	
							result = SUCCESS_E;
						}
					}
            }
				else result = SUCCESS_E;
         }
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* DM_OP_GetChannelFlags
* Get the channel flags from NVM.
*
* @param  - channel        The channel to query.
*
* @return - uint32_t       the flags from NVM
*/
uint32_t DM_OP_GetChannelFlags(const CO_ChannelIndex_t channel)
{
   uint32_t channel_flags = 0;
   
   if ( CO_CHANNEL_MAX_E > channel )
   {
      //check that the channel has an associated NVM slot
      DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(channel); 
      
      if ( NV_MAX_PARAM_ID_E != nvm_param )
      {
         // Read the NVM
         DM_NVMRead(nvm_param, (void*)&channel_flags, sizeof(channel_flags));
      }
   }
   
   return channel_flags;
}

/*************************************************************************************/
/**
* DM_OP_GetDisablement
* Get the channel day/night disablement.
*
* @param  - channel        The channel to query.
* @param  - enableDisable  [OUT] the settings for the channel.
*
* @return - Error code.
*/
ErrorCode_t DM_OP_GetDisablement(const uint16_t channel, DM_Enable_t *enableDisable)
{
   ErrorCode_t result = ERR_CONFIG_DISABLED_E;
   AlarmOutputStateConfig_t config;
   DM_Enable_t enabled = DM_ENABLE_NONE_E;
   CO_DayNight_t day_night = CFG_GetDayNight();

   if ( enableDisable && (CO_CHANNEL_MAX_E > channel) )
   {
      if ( CFG_ZoneEnabled() && DM_RbuEnabled() )
      {
         //read the dynamic record
         enabled = gChannelDisabled[channel];
         
         // If the channel is PIR and if the channel is enabled, check for security profile disablement
         if ( (CO_CHANNEL_PIR_E == channel) && (DM_ENABLE_NONE_E != enabled) )
         {
            //read the channel config
            if ( SUCCESS_E == DM_OP_GetChannelOutputConfig((CO_ChannelIndex_t)channel, &config) )
            {
               //test for security profile
               if ( config.ProfileMask & DM_OP_BIT_SECURITY )
               {
                  //check for daytime inhibit
                  if ( CO_DAY_E == day_night )
                  {
                     if ( CFG_GetSecurityInhibitDay() )
                     {
                        if ( DM_ENABLE_DAY_AND_NIGHT_E == enabled )
                        {
                           enabled = DM_ENABLE_NIGHT_E;
                        }
                        else if ( DM_ENABLE_DAY_E == enabled )
                        {
                           enabled = DM_ENABLE_NONE_E;
                        }
                     }
                  }
                  else
                  {
                     //check for night time inhibit
                     if ( CFG_GetSecurityInhibitNight() )
                     {
                        if ( DM_ENABLE_DAY_AND_NIGHT_E == enabled )
                        {
                           enabled = DM_ENABLE_DAY_E;
                        }
                        else if ( DM_ENABLE_NIGHT_E == enabled )
                        {
                           enabled = DM_ENABLE_NONE_E;
                        }
                     }
                  }
               }
            }
         }
      }
      else 
      {
         //Either the zone or device are disabled
         enabled = DM_ENABLE_NONE_E;
      }
      
      *enableDisable = enabled;
      result = SUCCESS_E;
   }
   
   return result;
}
/*************************************************************************************/
/**
* DM_OP_SetSoundLevel
* Set the tone attenuation for the sounder.
*
* @param  - soundLevel   The new attenuation value (0-99).
*
* @return - Error code.
*/
ErrorCode_t DM_OP_SetSoundLevel(const uint8_t soundLevel)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;
   uint32_t attenuation = (uint32_t)soundLevel;
   
   //Limit the max value
   if ( SOUND_LEVEL_MAX_E < attenuation )
   {
      attenuation = SOUND_LEVEL_MAX_E;
   }
   
   //Write the new value to FLASH config
   if ( SUCCESS_E == CFG_SetSoundLevel(attenuation) )
   {
      // If the sounder is already running.  Set the new attenuation level.
      if ( OutputState[DM_OP_CHANNEL_SOUNDER_E].Activated )
      {
         DM_SVI_SetToneAttenuation(attenuation);
      }
      /* All done successfully */
      result = SUCCESS_E;
   }
   
   return result;
}


/*************************************************************************************/
/**
* DM_OP_ChannelDisabled
* Returns true if the supplied output channel is disabled.
* Also accounts for whether the zone or device is disabled.
*
* @param  - outputChannelIndex   The channel to check.
*
* @return - True if the output channel is disabled, false if enabled.
*/
bool DM_OP_ChannelDisabled(const CO_ChannelIndex_t channelIndex)
{
   bool disabled = false;
   bool day_night_disabled = true;
   
   //is it day or night
   if ( CO_DAY_E == CFG_GetDayNight() )
   {
      //check for daytime enabled
      if ( (DM_ENABLE_DAY_E == gChannelDisabled[channelIndex]) ||
           (DM_ENABLE_DAY_AND_NIGHT_E == gChannelDisabled[channelIndex]) )
      {
         day_night_disabled = false;
      }
   }
   else 
   {
      //check for night time enabled
      if ( (DM_ENABLE_NIGHT_E == gChannelDisabled[channelIndex]) ||
           (DM_ENABLE_DAY_AND_NIGHT_E == gChannelDisabled[channelIndex]) )
      {
         day_night_disabled = false;
      }
   }
   
   //We always enable the status LED so that the onboard LED can be used
   if ( CO_CHANNEL_STATUS_INDICATOR_LED_E == channelIndex )
   {
      day_night_disabled = false;
   }
   
   if ( (false == DM_RbuEnabled()) ||
        (false == CFG_ZoneEnabled()) ||
         day_night_disabled )
   {
      disabled = true;
   }
   return disabled;
}


/*************************************************************************************/
/**
* DM_OP_OutputsActive
* Returns true if any output is active (except the status indicator LED).
* The option to ignore relay outputs allows the function to discount the outputs
* that place no drain on the battery (for battery management tests).
*
* @param  - ingnoreRelayOutputs   Don't include the relay outputs.
*
* @return - True if any output is active.
*/
bool DM_OP_OutputsActive(const bool ingnoreRelayOutputs)
{
   bool output_active = false;
   uint32_t output_channel;
   
   for ( output_channel = DM_OP_CHANNEL_SOUNDER_E; output_channel < DM_OP_CHANNEL_MAX_E; output_channel++ )
   {
      if ( (DM_OP_CHANNEL_OUTPUT_1_E > output_channel) || (false == ingnoreRelayOutputs) )
      {
         if ( OutputState[output_channel].Activated )
         {
            //Don't include the status indicator LED
            if ( DM_OP_CHANNEL_STATUS_INDICATOR_LED_E != output_channel)
            {
               output_active = true;
            }
         }
      }
   }
   
   return output_active;
}

/*************************************************************************************/
/**
* DM_OP_TestOneShot
* Initiates the one-shot test of alarm outputs.
*
* @param  - silentOrStandard   sets silent or standard test profile. True = standard, false = silent.
*
* @return - True if any output is active.
*/
ErrorCode_t DM_OP_TestOneShot(const bool silentOrStandard)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;
   CO_OutputProfile_t profile;
   CO_ChannelIndex_t channel = CO_CHANNEL_NONE_E;
   DM_OP_OutputChannel_t output_channel;

   CO_PRINT_B_1(DBG_INFO_E,"Rx'd one-shot test command %d\r\n", silentOrStandard);
   if ( false == DM_OP_OutputsActive(true) )
   {
      result = ERR_BUSY_E;
      
      //Find out if we have an SVI or a construction sounder
      uint32_t device = 0;
      //Does this RBU have the SVI channel
      if ( SUCCESS_E == DM_DeviceGetDeviceType(gDeviceCombination, CO_CHANNEL_SOUNDER_E, &device) )
      {
         channel = CO_CHANNEL_SOUNDER_E;
      }
      else if ( SUCCESS_E == DM_DeviceGetDeviceType(gDeviceCombination, CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E, &device) )
      {
         channel = CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E;
      }

      if ( CO_CHANNEL_NONE_E != channel )
      {
         output_channel = DM_OP_SystemChannelToOutputChannel(channel);
         if ( DM_OP_CHANNEL_MAX_E != output_channel)
         {
 
            if ( silentOrStandard )
            {
               if ( DM_OP_HasPriority(output_channel, CO_PROFILE_TEST_E) )
               {
                  profile = CO_PROFILE_TEST_E;
                  result = SUCCESS_E;
               }
            }
            else 
            {
               if ( DM_OP_HasPriority(output_channel, CO_PROFILE_SILENT_TEST_E) )
               {
                  profile = CO_PROFILE_SILENT_TEST_E;
                  result = SUCCESS_E;
               }
            }
            
            if ( SUCCESS_E == result )
            {
               //Switch on the output
               CO_PRINT_B_2(DBG_INFO_E,"DM_OP_SetAlarmState8 %d, %d ON\r\n", channel, profile);
               result = DM_OP_SetAlarmState(channel, profile, true, false);
               if ( SUCCESS_E == result )
               {
                  //Set the switch off duration
                  OutputState[output_channel].OutputTimeoutCount = DM_ONE_SHOT_DURATION;
                  //Start timing
                  OutputState[output_channel].TimedOutputActive = true;
               }
               else 
               {
                  CO_PRINT_B_0(DBG_INFO_E,"One-shot test abandoned - failed to activate sounder\r\n");
               }
            }
            else 
            {
               result = ERR_BUSY_E;
               CO_PRINT_B_0(DBG_INFO_E,"One-shot test abandoned - profile does not have priority\r\n");
            }
         }
         else 
         {
            result = ERR_NOT_FOUND_E;
            CO_PRINT_B_0(DBG_INFO_E,"One-shot test abandoned - failed to map channel to a sounder\r\n");
         }
      }
      else 
      {
         result = ERR_NOT_FOUND_E;
         CO_PRINT_B_0(DBG_INFO_E,"One-shot test abandoned - no sounder fitted\r\n");
      }
   }
   else 
   {
      result = ERR_BUSY_E;
      CO_PRINT_B_0(DBG_INFO_E,"One-shot test abandoned - alarms are active\r\n");
   }
   
   return result;
}

/*************************************************************************************/
/**
* DM_OP_Confirmed
* Handles a 'fire confirmed' message from the control panel.
* Any fire profile delays are cancelled and the alarm sounded immediately
*
* @param  - None.
*
* @return - ERR_NOT_FOUND_E if no fire profile output is active.  SUCCESS_E if one is active
*/
ErrorCode_t DM_OP_Confirmed(const CO_OutputProfile_t profile)
{
   ErrorCode_t result = ERR_NOT_FOUND_E;
   uint32_t output_channel;
   
   CO_PRINT_B_0(DBG_INFO_E,"Reveived fire confirmation signal from control panel\r\n");
   
   //Iterate through each output channel expiring delays if it is using the fire profile.
   //Function DM_OP_ManageDelays will sound the alarms.
   for( output_channel = 0; output_channel < DM_OP_CHANNEL_MAX_E; output_channel++)
   {
      if ( CO_PROFILE_FIRE_E == profile )
      {
         if ( CO_PROFILE_FIRE_E == OutputState[output_channel].ActiveProfile )
         {
            // If delay 1 or delay 2 is running, set the delays to expired.
            if ( (DM_OP_TIMER_NOT_RUNNING_E != OutputState[output_channel].Delay1State) ||
                  (DM_OP_TIMER_NOT_RUNNING_E != OutputState[output_channel].Delay2State) )
            {
               OutputState[output_channel].Delay1State = DM_OP_TIMER_EXPIRED_E;
               OutputState[output_channel].Delay1Count = 0;
               OutputState[output_channel].Delay2State = DM_OP_TIMER_EXPIRED_E;
               OutputState[output_channel].Delay2Count = 0;
               result = SUCCESS_E;
            }
         }
      }
      else 
      {
         if ( CO_PROFILE_FIRE_E != OutputState[output_channel].ActiveProfile )
         {
            // If delay 1 or delay 2 is running, set the delays to expired.
            if ( (DM_OP_TIMER_NOT_RUNNING_E != OutputState[output_channel].Delay1State) ||
                  (DM_OP_TIMER_NOT_RUNNING_E != OutputState[output_channel].Delay2State) )
            {
               OutputState[output_channel].Delay1State = DM_OP_TIMER_EXPIRED_E;
               OutputState[output_channel].Delay1Count = 0;
               OutputState[output_channel].Delay2State = DM_OP_TIMER_EXPIRED_E;
               OutputState[output_channel].Delay2Count = 0;
               result = SUCCESS_E;
            }
         }
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* DM_OP_SetAlarmConfiguration
* Set the 
*
* @param  - systemChannel  : the channel to apply the config to
* @param  - bitField       : the config bit field
*
* @return - Error code.
*/
ErrorCode_t DM_OP_SetAlarmConfiguration(const CO_ChannelIndex_t systemChannel, const uint32_t bitField)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;
   
   //check that the channel has an associated NVM slot
   DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(systemChannel); 
   
   if ( NV_MAX_PARAM_ID_E != nvm_param )
   {
      // Read the NVM
      uint32_t nvm_value;
      if ( SUCCESS_E == DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
      {
         //Mask out the output profile bits from the value read from NVM above
         nvm_value &= ~NVM_OUTPUT_PROFILE_MASK;
         
         // check that the supplied profile doesn't extend beyond the bit field for the profile
         if ( 0 == ( bitField & ~NVM_OUTPUT_PROFILE_MASK ) )
         {
            //  Apply the bit field and write back to NVM
            nvm_value |= bitField;
            if ( SUCCESS_E == DM_NVMWrite(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
            {
               result = SUCCESS_E;
               //Reload the profiles for the running software
               DM_OP_LoadProfiles();
            }
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* DM_OP_GetAlarmConfiguration
* Set the 
*
* @param  - systemChannel  : the channel to read the config for
* @param  - bitField       : [OUT] pointer to receive the config bit field
*
* @return - Error code.
*/
ErrorCode_t DM_OP_GetAlarmConfiguration(const CO_ChannelIndex_t systemChannel, uint32_t *bitField)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;
   
   if ( bitField )
   {
      //check that the channel has an associated NVM slot
      DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(systemChannel); 
      
      if ( NV_MAX_PARAM_ID_E != nvm_param )
      {
         // Read the NVM
         uint32_t nvm_value;
         result = DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value));
         if ( SUCCESS_E == result )
         {
            //Mask out the output profile bits from the value read from NVM above
            nvm_value &= NVM_OUTPUT_PROFILE_MASK;
            *bitField = nvm_value;
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* DM_OP_SetAlarmOptionFlags
* Set the format parameters for the specified channel
* Flags - silenceable, inverted, ignore night delays
*
* @param  - systemChannel  : the channel to apply the config to
* @param  - bitField       : the config bit field
*
* @return - Error code.
*/
ErrorCode_t DM_OP_SetAlarmOptionFlags(const CO_ChannelIndex_t systemChannel, const uint32_t bitField)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;
   uint32_t option_bits = bitField << 7;
   
   //check that the channel has an associated NVM slot
   DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(systemChannel); 
   
   if ( NV_MAX_PARAM_ID_E != nvm_param )
   {
      // Read the NVM
      uint32_t nvm_value;
      if ( SUCCESS_E == DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
      {
         //Mask out the output profile bits from the value read from NVM above
         nvm_value &= ~NVM_OUTPUT_FORMAT_MASK;
         
         // check that the supplied profile doesn't extend beyond the bit field for the profile
         if ( 0 == ( option_bits & ~NVM_OUTPUT_FORMAT_MASK ) )
         {
            //  Apply the bit field and write back to NVM
            nvm_value |= option_bits;
            if ( SUCCESS_E == DM_NVMWrite(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
            {
               result = SUCCESS_E;
               //Reload the profiles for the running software
               DM_OP_LoadProfiles();
            }
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* DM_OP_GetAlarmOptionFlags
* Return the format parameters for the specified channel
* Flags - silenceable, inverted, ignore night delays
*
* @param  - systemChannel  : the channel to apply the config to
* @param  - bitField       : [OUT] pointer to receive the option flags
*
* @return - Error code.
*/
ErrorCode_t DM_OP_GetAlarmOptionFlags(const CO_ChannelIndex_t systemChannel, uint32_t *bitField)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;
   
   if ( bitField )
   {
      //check that the channel has an associated NVM slot
      DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(systemChannel); 
      
      if ( NV_MAX_PARAM_ID_E != nvm_param )
      {
         // Read the NVM
         uint32_t nvm_value;
         result = DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value));
         if ( SUCCESS_E == result )
         {
            //Mask out the output profile bits from the value read from NVM above
            nvm_value &= NVM_OUTPUT_FORMAT_MASK;
            *bitField = (nvm_value >> 7);
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* DM_OP_SetAlarmDelays
* Set the delay 1 and delay 2 values for the specified channel
*
* @param  - systemChannel  : the channel to apply the config to
* @param  - bitField       : the config bit field
*
* @return - Error code.
*/
ErrorCode_t DM_OP_SetAlarmDelays(const CO_ChannelIndex_t systemChannel, const uint32_t bitField)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;
   uint32_t delay_bits = bitField << 11;
   
   //check that the channel has an associated NVM slot
   DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(systemChannel); 
   
   if ( NV_MAX_PARAM_ID_E != nvm_param )
   {
      // Read the NVM
      uint32_t nvm_value;
      if ( SUCCESS_E == DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
      {
         //Mask out the output profile bits from the value read from NVM above
         nvm_value &= ~NVM_OUTPUT_DELAYS_MASK;
         
         // check that the supplied profile doesn't extend beyond the bit field for the profile
         if ( 0 == ( delay_bits & ~NVM_OUTPUT_DELAYS_MASK ) )
         {
            //  Apply the bit field and write back to NVM
            nvm_value |= delay_bits;
            if ( SUCCESS_E == DM_NVMWrite(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
            {
               result = SUCCESS_E;
               //Reload the profiles for the running software
               DM_OP_LoadProfiles();
            }
         }
      }
   }
   return result;
}

/*************************************************************************************/
/**
* DM_OP_GetAlarmDelays
* Set the delay 1 and delay 2 values for the specified channel
*
* @param  - systemChannel  : the channel to apply the config to
* @param  - bitField       : [OUT] pointer to receive the delay settings
*
* @return - Error code.
*/
ErrorCode_t DM_OP_GetAlarmDelays(const CO_ChannelIndex_t systemChannel, uint32_t *bitField)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;
   
   //check that the channel has an associated NVM slot
   DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(systemChannel); 
   
   if ( NV_MAX_PARAM_ID_E != nvm_param )
   {
      // Read the NVM
      uint32_t nvm_value;
      result = DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value));
      if ( SUCCESS_E == result )
      {
         //Mask out the output profile bits from the value read from NVM above
         nvm_value &= NVM_OUTPUT_DELAYS_MASK;
         *bitField = (nvm_value >> 11);
      }
   }
   return result;
}

/*************************************************************************************/
/**
* DM_OP_ReportAlarmConfiguration
* Send the current alarm configuration to the debug output
*
* @param  - port     The usart to respond to.
*
* @return - None.
*/
void DM_OP_ReportAlarmConfiguration(const UartComm_t port)
{
   static const char op_channel_string[DM_OP_CHANNEL_MAX_E][60] = 
   {
      "SVI SOUNDER",
      "BEACON",
      "STATUS_INDICATOR_LED",
      "VISUAL_INDICATOR",
      "SOUNDER_VISUAL_INDICATOR_COMBINED",
      "OUTPUT_ROUTING",
      "OUTPUT_1",
      "OUTPUT_2"
   };
   AlarmOutputStateConfig_t alarmConfig;
   ErrorCode_t result;
   uint32_t outputChannelIndex;
   uint32_t channelIndex;
   uint32_t deviceCode;
   UartComm_t usart = port;
   
   osDelay(100);//delay to give existing usart messages time to clear the buffer
   
   if ( MESH_INTERFACE_E == usart )
   {
      usart = DEBUG_UART_E;
   }
   
   bool override = CFG_GetGlobalDelayOverride();
   bool global_delays_enabled = CFG_GetGlobalDelayEnabled();
   bool device_enabled = DM_RbuEnabled();
   bool zone_enabled = CFG_ZoneEnabled();
   ZoneDisabledMap_t zoneMap;
   CFG_GetZoneDisabledMap(&zoneMap);
   CO_PRINT_PORT_1(usart, "Global Delay Override %s\r\n", override ? "ENABLED" : "DISABLED");
   CO_PRINT_PORT_1(usart, "Global Delays %s\r\n", global_delays_enabled ? "ENABLED" : "DISABLED");
   CO_PRINT_PORT_1(usart,"Zone %s\r\n", zone_enabled ? "ENABLED" : "DISABLED");
   CO_PRINT_PORT_1(usart,"Device %s\r\n", device_enabled ? "ENABLED" : "DISABLED");
   CO_PRINT_PORT_1(usart,"Day-Night = %s\r\n", CFG_GetDayNight() ? "NIGHT" : "DAY");
   CO_PRINT_PORT_3(usart,"ZoneMap = %08x %08x %08x\r\n", zoneMap.UpperWord, zoneMap.MiddleWord, zoneMap.LowerWord);
   
   // Iterate through all output channel indexes and check to see if this unit has that channel
   for( outputChannelIndex = 0; outputChannelIndex < DM_OP_CHANNEL_MAX_E; outputChannelIndex++ )
   {
      //We need to pace the rate at which we feed the serial link or the buffer overflows
      osDelay(10);
      // convert local output channel index (DM_OP_OutputChannel_t) to the global channel index (CO_ChannelIndex_t)
      channelIndex = (uint32_t)DM_OP_OutputChannelToSystemChannel((DM_OP_OutputChannel_t)outputChannelIndex);
      
      //Check that the channel is valid and that it isn't disabled
      if ( CO_CHANNEL_MAX_E > channelIndex )
      {
         //convert channel index to device
         result = DM_DeviceGetDeviceCode(gDeviceCombination, channelIndex, &deviceCode);
         if ( SUCCESS_E == result )
         {
            //Do we have this output channel on this unit?
            if ( DM_DeviceIsEnabled(gDeviceCombination, deviceCode, DC_MATCH_ANY_E) )
            {
               if ( false == DM_OP_ChannelDisabled((CO_ChannelIndex_t)channelIndex) )
               {
                  //Get the channel profile from NVM
                  result = DM_OP_GetChannelOutputConfig((CO_ChannelIndex_t) channelIndex, &alarmConfig);
                  if ( SUCCESS_E == result )
                  {
                     osDelay(100);//delay to prevent USART buffer overrun
                     CO_PRINT_PORT_2(usart,"Channel : %s : %s\r\n", op_channel_string[outputChannelIndex], alarmConfig.Enabled ? "ENABLED" : "DISABLED");
                     CO_PRINT_PORT_3(usart,"               profile mask=0x%x, delay1=%d, delay2=%d\r\n",alarmConfig.ProfileMask,alarmConfig.Delay1, alarmConfig.Delay2);
                     CO_PRINT_PORT_3(usart,"               ignore night delays=%d, inverted=%d, silenceable=%d\r\n",alarmConfig.IgnoreNightDelays, alarmConfig.Inverted, alarmConfig.Silenceable);
                     CO_PRINT_PORT_1(usart,"               selected delays=%s\r\n",alarmConfig.LocalDelays ? "LOCAL" : "GLOBAL");
                  }
               }
               else 
               {
                  CO_PRINT_PORT_1(usart,"Channel : %s : DISABLED\r\n", op_channel_string[outputChannelIndex]);
               }
            }
            else
            {
               CO_PRINT_PORT_1(usart,"Channel : %s : Not fitted\r\n", op_channel_string[outputChannelIndex]);
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* DM_OP_GetHighestPriorityActiveProfile
* Return the profile setting highest priority active output.
*
* @param  - None.
*
* @return - SUCCESS_E.
*/
CO_OutputProfile_t DM_OP_GetHighestPriorityActiveProfile(void)
{
   CO_OutputProfile_t highest_profile = CO_PROFILE_MAX_E;
   uint32_t output_channel;
   uint32_t profile;
   
   //iterate through the profiles in priority order
   for ( profile = 0; (profile < CO_PROFILE_MAX_E) && (CO_PROFILE_MAX_E == highest_profile); profile++ )
   {
      //Iterate through each output channel
      for( output_channel = 0; output_channel < DM_OP_CHANNEL_MAX_E; output_channel++)
      {
         //does the channel's active profile match the search profile
         if ( profile == OutputState[output_channel].ActiveProfile )
         {
            //we have found the highest priority active profile
            highest_profile = (CO_OutputProfile_t)profile;
            //break out of the channel loop
            break;
         }
      }
   }
   
   return highest_profile;
}

/*************************************************************************************/
/**
* DM_OP_SiteNetCombinedSounderCycle
* Rrun the next step in the SiteNet sounder output sequence.
*
* @param  - count       The number of counts into the sounder output sequence
*
* @return - SUCCESS_E.
*/
void DM_OP_SiteNetCombinedSounderCycle(const uint32_t count)
{
   DM_NVMParamId_t nvm_profile = NV_MAX_PARAM_ID_E;
   CO_OutputProfile_t active_profile = OutputState[DM_OP_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E].ActiveProfile;

   switch ( active_profile )
   {
      case CO_PROFILE_FIRE_E:
         nvm_profile = NV_PROFILE_FIRE_E;
         break;
      case CO_PROFILE_FIRST_AID_E:
         nvm_profile = NV_PROFILE_FIRST_AID_E;
         break;
      case CO_PROFILE_EVACUATE_E:
         nvm_profile = NV_PROFILE_EVACUATE_E;
         break;
      case CO_PROFILE_SECURITY_E:
         nvm_profile = NV_PROFILE_SECURITY_E;
         break;
      case CO_PROFILE_GENERAL_E:
         nvm_profile = NV_PROFILE_GENERAL_E;
         break;
      case CO_PROFILE_FAULT_E:
         nvm_profile = NV_PROFILE_FAULT_E;
         break;
      case CO_PROFILE_ROUTING_ACK_E:
         nvm_profile = NV_PROFILE_ROUTING_ACK_E;
         break;
      case CO_PROFILE_SILENT_TEST_E:
         nvm_profile = NV_PROFILE_SILENT_TEST_E;
         break;
      case CO_PROFILE_TEST_E:
         nvm_profile = NV_PROFILE_TEST_E;
         break;
      default:
         nvm_profile = NV_MAX_PARAM_ID_E;//invalid value
      break;
   }
   
   if ( NV_MAX_PARAM_ID_E != nvm_profile )
   {
      uint32_t nvm_value;
      if (SUCCESS_E == DM_NVMRead(nvm_profile, (void*)&nvm_value, sizeof(nvm_value)))
      {
         uint32_t sitenet_pattern = (nvm_value >> 24);
         if ( DM_OP_SITENET_SOUNDER_PATTERN_MAX_E > sitenet_pattern )
         {
            SounderSequence[sitenet_pattern](count);
         }
      }
   }
   else 
   {
      CO_PRINT_A_1(DBG_ERROR_E,"Unknown SiteNet sounder profile %d\r\n", active_profile);
   }
}

/*************************************************************************************/
/**
* DM_OP_SiteNetSounderPattern1
* Run the next step of the Sitenet sounder sequence for pattern 1.
*
* @param  - count       The number of counts into the sounder output sequence
*
* @return - SUCCESS_E.
*/
void DM_OP_SiteNetSounderPattern1(const uint32_t count)
{
   //Continuous ON
   GpioWrite(&SndrBeaconOn, GPIO_PIN_SET);
}

/*************************************************************************************/
/**
* DM_OP_SiteNetSounderPattern2
* Run the next step of the Sitenet sounder sequence for pattern 2.
*
* @param  - count       The number of counts into the sounder output sequence
*
* @return - SUCCESS_E.
*/
void DM_OP_SiteNetSounderPattern2(const uint32_t count)
{
   //1s ON, 7s OFF
   const uint32_t period = 160; //8 seconds at 50ms per count
   const uint32_t sounder_stop = 20; // 1 second at 50ms per count
   
   uint32_t sequence_index = count % period;
   
   if ( sounder_stop >= sequence_index )
   {
      //Sounder ON
      GpioWrite(&SndrBeaconOn, GPIO_PIN_SET);
   }
   else 
   {
      //Sounder OFF
      GpioWrite(&SndrBeaconOn, GPIO_PIN_RESET);
   }
}

/*************************************************************************************/
/**
* DM_OP_SiteNetSounderPattern3
* Run the next step of the Sitenet sounder sequence for pattern 3.
*
* @param  - count       The number of counts into the sounder output sequence
*
* @return - SUCCESS_E.
*/
void DM_OP_SiteNetSounderPattern3(const uint32_t count)
{
   //0.5s ON, 0.5s OFF
   const uint32_t period = 20; //1 second at 50ms per count
   const uint32_t sounder_stop = 10; // 0.5 seconds at 50ms per count
   
   uint32_t sequence_index = count % period;
   
   if ( sounder_stop >= sequence_index )
   {
      //Sounder ON
      GpioWrite(&SndrBeaconOn, GPIO_PIN_SET);
   }
   else 
   {
      //Sounder OFF
      GpioWrite(&SndrBeaconOn, GPIO_PIN_RESET);
   }
}

/*************************************************************************************/
/**
* DM_OP_SiteNetSounderPattern4
* Run the next step of the Sitenet sounder sequence for pattern 4.
*
* @param  - count       The number of counts into the sounder output sequence
*
* @return - SUCCESS_E.
*/
void DM_OP_SiteNetSounderPattern4(const uint32_t count)
{
   //((0.5s ON, 0.5s OFF) x 3), 1.5s OFF
   const uint32_t period = 90; //4.5 seconds at 50ms per count
   const uint32_t sound_sequence_end = 60; // 3 seconds.  start of 1.5s OFF
   const uint32_t Cycle_length = 20; // 1 second.  Duration of one sound cycle.
   const uint32_t sounder_stop = 10; // 0.5 seconds at 50ms per count
   
   uint32_t sequence_index = count % period;
   
   if ( sound_sequence_end >= sequence_index )
   {
      //In the 0.5s ON, 0.5s OFF period
      if ( sounder_stop < (count % Cycle_length) )
      {
         //Sounder ON
         GpioWrite(&SndrBeaconOn, GPIO_PIN_SET);
      }
      else 
      {
         //Sounder OFF
         GpioWrite(&SndrBeaconOn, GPIO_PIN_RESET);
      }
   }
   else 
   {
      // 1.5s OFF period.  Sounder OFF
      GpioWrite(&SndrBeaconOn, GPIO_PIN_RESET);
   }
}

/*************************************************************************************/
/**
* DM_OP_SiteNetSounderPattern5
* Run the next step of the Sitenet sounder sequence for pattern 5.
*
* @param  - count       The number of counts into the sounder output sequence
*
* @return - SUCCESS_E.
*/
void DM_OP_SiteNetSounderPattern5(const uint32_t count)
{
   //0.1s ON, 2s OFF
   const uint32_t period = 42; //2.1 seconds at 50ms per count
   const uint32_t sounder_stop = 2; // 0.1 seconds at 50ms per count
   
   uint32_t sequence_index = count % period;
   
   if ( sounder_stop >= sequence_index )
   {
      //Sounder ON
      GpioWrite(&SndrBeaconOn, GPIO_PIN_SET);
   }
   else 
   {
      //Sounder OFF
      GpioWrite(&SndrBeaconOn, GPIO_PIN_RESET);
   }
}

/*************************************************************************************/
/**
* DM_OP_SiteNetSounderPattern6
* Run the next step of the Sitenet sounder sequence for pattern 6.
*
* @param  - count       The number of counts into the sounder output sequence
*
* @return - SUCCESS_E.
*/
void DM_OP_SiteNetSounderPattern6(const uint32_t count)
{
   //Continuous OFF
   GpioWrite(&SndrBeaconOn, GPIO_PIN_RESET);
}

/*************************************************************************************/
/**
* DM_OP_SetLocalOrGlobalDelayFlag
* Update the Global delays flag
*
* @param  - systemChannel  : the channel to apply the config to
* @param  - localDelay     : True to enable local delays
*
* @return - Error code.
*/
ErrorCode_t DM_OP_SetLocalOrGlobalDelayFlag(const CO_ChannelIndex_t systemChannel, const bool localDelay)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;
   
   //check that the channel has an associated NVM slot
   DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(systemChannel); 
   
   if ( NV_MAX_PARAM_ID_E != nvm_param )
   {
      // Read the NVM
      uint32_t nvm_value;
      if ( SUCCESS_E == DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
      {
         //Mask out the output profile bits from the value read from NVM above
         nvm_value &= ~NVM_OUTPUT_LOCAL_DELAYS_MASK;
         
         // check that the supplied profile doesn't extend beyond the bit field for the profile
         if ( localDelay )
         {
            //  Apply the bit field and write back to NVM
            nvm_value |= NVM_OUTPUT_LOCAL_DELAYS_MASK;
         }
         //Write to NVM
         result = DM_NVMWrite(nvm_param, (void*)&nvm_value, sizeof(nvm_value));
      }
   }
   return result;
}


/*************************************************************************************/
/**
* DM_OP_GetLocalOrGlobalDelayFlag
* Return the Global delays flag
*
* @param  - systemChannel  : the channel to apply the config to
*
* @return - bool           : True if local delay timers should be used.
*                          : False if global delay timers should be used.
*/
bool DM_OP_GetLocalOrGlobalDelayFlag(const CO_ChannelIndex_t systemChannel)
{
   bool localDelay = false;
   ErrorCode_t result;
   
      //check that the channel has an associated NVM slot
      DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(systemChannel); 
      
      if ( NV_MAX_PARAM_ID_E != nvm_param )
      {
         // Read the NVM
         uint32_t nvm_value;
         result = DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value));
         if ( SUCCESS_E == result )
         {
            //Mask out the Local Delay bit from the value read from NVM
            nvm_value &= NVM_OUTPUT_LOCAL_DELAYS_MASK;
            if ( nvm_value )
            {
               localDelay = true;
            }
         }
      }

   return localDelay;
}

/*************************************************************************************/
/**
* DM_OP_SetGlobalDelayValues
* Update the Global delays
*
* @param  - CombinedValues  : delay 1 in lower 16 bits, delay 2 in upper
*
* @return - Error code.
*/
ErrorCode_t DM_OP_SetGlobalDelayValues(const uint32_t CombinedValues)
{
   ErrorCode_t result = ERR_OPERATION_FAIL_E;
   
   // Write to the NVM
   uint32_t nvm_value = CombinedValues;
   result = DM_NVMWrite(NV_GLOBAL_DELAY_E, (void*)&nvm_value, sizeof(nvm_value));
   if ( SUCCESS_E == result )
   {
      //Store the separate delay values in the device config for dynamic access.
      uint16_t delay1 = (uint16_t)(CombinedValues & 0xFFFF);
      uint16_t delay2 = (uint16_t)(CombinedValues >> 16);
      CFG_SetGlobalDelay1(delay1);
      CFG_SetGlobalDelay2(delay2);
   }
   
   return result;
}


/*************************************************************************************/
/**
* DM_OP_GetLocalOrGlobalDelayFlag
* Return the Global delay values
*
* @param  - void
*
* @return - ErrorCode_t    SUCCESS_E or error code
*/
ErrorCode_t DM_OP_GetGlobalDelayValues(uint32_t* const pCombinedDelay)
{
   uint32_t combined_delays = 0;
   ErrorCode_t result = ERR_INVALID_PARAMETER_E;
   
   if ( pCombinedDelay )
   {
      // Read the NVM
      result = DM_NVMRead(NV_GLOBAL_DELAY_E, (void*)&combined_delays, sizeof(combined_delays));
      if ( SUCCESS_E == result )
      {
         *pCombinedDelay = combined_delays;
      }
   }

   return result;
}

/*************************************************************************************/
/**
* DM_OP_StartDelayedOutput
* Initiate the delayed output test function
*
* @param  - initialDelay   The time to wait before activating the output
* @param  - duration       The duration that the output should be active
* @param  - channel        The output channel
*
* @return - ErrorCode_t    SUCCESS_E or error code
*/
ErrorCode_t DM_OP_StartDelayedOutput(const uint32_t initialDelay, const uint32_t duration, const CO_ChannelIndex_t channel)
{
   ErrorCode_t result = ERR_INVALID_PARAMETER_E;
   
   if ( CO_CHANNEL_MAX_E > channel )
   {
      // check that it's an output channel
      if ( DM_OP_CHANNEL_MAX_E != DM_OP_SystemChannelToOutputChannel(channel) )
      {
         //Cancel any previous output
         if ( DM_OP_DLY_ACTIVE_E == gDelayedOutputState )
         {
            DM_OP_SetAlarmState( gDelayedOutputChannel, CO_PROFILE_FIRE_E, false, true);
         }
         gDelayedOutputState = DM_OP_DLY_INITIAL_DELAY_E;
         gDelayedOutputInitialDelay = initialDelay;
         gDelayedOutputDuration = duration;
         gDelayedOutputChannel = channel;
         result = SUCCESS_E;
      }
   }
   
   return result;
}


/*************************************************************************************/
/**
* DM_OP_RunDelayedOutput
* Called once per second by the Application to manage the delayed output test function
*
* @param  - None
*
* @return - None
*/
void DM_OP_RunDelayedOutput(void)
{
   switch ( gDelayedOutputState )
   {
      case DM_OP_DLY_IDLE_E:
         //Do nothing.
         break;
      case DM_OP_DLY_INITIAL_DELAY_E:
         gDelayedOutputInitialDelay--;
         CO_PRINT_B_2(DBG_INFO_E,"Channel %d Delayed Output Initial Delay %d\r\n", gDelayedOutputChannel, gDelayedOutputInitialDelay);
         if ( 0 == gDelayedOutputInitialDelay )
         {
            DM_OP_SetAlarmState( gDelayedOutputChannel, CO_PROFILE_FIRE_E, true, true);
            gDelayedOutputState = DM_OP_DLY_ACTIVE_E;
         }
         break;
      case DM_OP_DLY_ACTIVE_E:
         gDelayedOutputDuration--;
         CO_PRINT_B_2(DBG_INFO_E,"Channel %d Delayed Output Active %d\r\n", gDelayedOutputChannel, gDelayedOutputDuration);
         if ( 0 == gDelayedOutputDuration )
         {
            DM_OP_SetAlarmState( gDelayedOutputChannel, CO_PROFILE_FIRE_E, false, true);
            gDelayedOutputState = DM_OP_DLY_IDLE_E;
         }
         break;
      default:
         break;
   }
}

/*************************************************************************************/
/**
* DM_OP_ResetTestMode
* Called once per second by the Application to manage the reset of the duplicate command filter
* to allow consecutive test messages without an 'OFF' command inbetween.
* Also resets the SVI registers if an SVI is present.
*
* @param  - None
*
* @return - None
*/
void DM_OP_ResetTestMode(void)
{
   uint32_t bitMask = 0;
   
   if ( 0 < gTestModeResetCount )
   {
      gTestModeResetCount--;
      if ( 0 == gTestModeResetCount )
      {
         //Mask out the test bits from the previous command to prevent a new 
         //test signal form being filtered out as a duplicate.
         bitMask = DM_OP_BIT_TEST | DM_OP_BIT_SILENT_TEST;
         bitMask = ~bitMask;
         previous_command.Silenceable &= bitMask;
         previous_command.Unsilenceable &= bitMask;
         previous_command.DelayMask &= bitMask;
         
         //Make sure that no priority outputs are active before resetting the SVI
         if ( (0 == previous_command.Silenceable) && (0 == previous_command.Unsilenceable) )
         {
            if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SVI_SOUNDERS, DC_MATCH_ANY_E) )
            {
               // Stop the SVI sounder
               if ( DM_SVI_SetSounder(DM_SVI_OFF_E) )
               {
                  OutputState[DM_OP_CHANNEL_SOUNDER_E].Activated = false;
                  OutputState[DM_OP_CHANNEL_SOUNDER_E].ActiveProfile = CO_PROFILE_MAX_E;
                  CO_PRINT_B_0(DBG_INFO_E,"Sounder test mode reset\r\n");
               }
               else
               {
                  CO_PRINT_B_0(DBG_INFO_E,"Sounder test mode failed to reset.\r\n");
               }
            }
         }
      }
   }
}

