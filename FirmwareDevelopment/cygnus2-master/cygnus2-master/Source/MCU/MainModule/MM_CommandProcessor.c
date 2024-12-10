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
*  File         : MM_CommandProcessor.c
*
*  Description  : Manage the command/response messages fromm the Mesh Protocol. 
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/
#include "stm32l4xx.h"
#include "cmsis_os.h"
#include "MM_Main.h"
#include "DM_NVM.h"
#include "MM_ATHandleTask.h"
#include "MM_MeshAPI.h"
#include "MC_MAC.h"
#include "SM_StateMachine.h"
#include "DM_svi.h"
#include "BLF_Boot.h"
#include "eeprom_emul.h"
#include "flash_interface.h"
#include "MM_CIEQueueManager.h"
#include "MM_ApplicationCommon.h"
#include "MM_NeighbourInfo.h"
#include "DM_RelayControl.h"
#include "DM_OutputManagement.h"
#include "MM_GpioTask.h"
#include "MC_MacConfiguration.h"
#include "CFG_Device_cfg.h"
#include "MM_TimedEventTask.h"
#include "MC_StateManagement.h"
#include "DM_BatteryMonitor.h"
#include "MM_PpuManagement.h"
#include "MM_CommandProcessor.h"


/* Definitions
*************************************************************************************/


/* Private Functions Prototypes
*************************************************************************************/
extern uint32_t MM_GetFaultStatusFlags(void);
extern uint8_t MM_RBU_GetOverallFaults(void);

extern HAL_StatusTypeDef FLASH_WaitForLastOperation(uint32_t Timeout);
static ErrorCode_t MM_CMD_ProcessHeadCommand ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessLocalCommand ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static bool MM_CMD_SendATResponse(const CO_ResponseData_t* pResponse, const CommandSource_t cmd_source);
static ErrorCode_t MM_CMD_ProcessCommandDeviceCombination ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessCommandAnalogueValue ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessCommandNeighbourInformation ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessCommandStatusFlags ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessCommandRBUSerialNumber ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessCommandRBUEnable ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessModulationBandwidth ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessSpreadingFactor ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessCodingRate ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessFrequency ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessTxPower ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessRBUTestMode ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessFirmwareInformation ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessFirmwareActiveImage ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessReboot ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessSVIComand ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessCommandTxPowerLevel ( const CO_CommandData_t* pCommand, const CommandSource_t source );/* Set Tx power Hi or Lo in NVM */
static ErrorCode_t MM_CMD_ProcessMeshStatusRequest ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessCommandZone ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessProductCode ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessPPUModeEnable ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessReadCIEMessageQueueStatus ( const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessReadZonesAndDevices ( const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessReadCIEFireMessageQueue ( const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessReadCIEAlarmMessageQueue ( const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessReadCIEFaultMessageQueue ( const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessReadCIEMiscMessageQueue ( const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessDiscardFromCIEFireMessageQueue ( const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessDiscardFromCIEAlarmMessageQueue ( const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessDiscardFromCIEFaultMessageQueue ( const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessDiscardFromCIEMiscMessageQueue ( const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessResetSelectedQueues ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessTestModeCommand ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessReadAnalogueValueCommand ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessReadDeviceStatusFlags ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessBeaconFlashRate ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessDeviceEnable ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessSounderToneSelection ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessUpperThresholdSetting ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessLowerThresholdSetting ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessPluginTestMode ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessCRCInformation ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessEEPROMInformation ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessReadNeighbourInformation ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessCIEDayNightSetting ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessPluginLEDSetting ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessAlarmGlobalDelays ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessAlarmAcknowledge ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessAlarmEvacuate ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessAlarmEnable ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessAlarmReset ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessSoundLevel ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessTestOneShot ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessConfirm ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessAlarmConfiguration ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessAlarmOptionFlags ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessAlarmDelays ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessChannelFlags ( const CO_CommandData_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessCIEChannelFlagsCommand ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessMaxRankCommand ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_Process200HourTest ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessCheckFirmwareRequest ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessLocalOrGlobalDelaySetting ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessGlobalDelayValues ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessGlobalDelayOverride ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessCIERemoteCommand( const CIECommand_t* pCommand, const ParameterType_t ParameterType, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessReadStatusIndication ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessReadRBUSerialNumber ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessFirmwareInfoRequest ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessPluginFirmwareInfoRequest ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessPluginTypeAndClassRequest ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessPluginSerialNumberRequest ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessBatteryStatusRequest ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessGlobalDelayCombined ( const CIECommand_t* pCommand, const CommandSource_t source );
static void MM_CMD_SendATFailWithErrorCode(const char* prefix, const ErrorCode_t errorCode, const CommandSource_t source );
static ErrorCode_t MM_CMD_AlarmReset( int flags );
static ErrorCode_t DM_OP_SetEnableDisable(const CO_DisableRegion_t region, const uint16_t channel_or_zone, const uint32_t enable_disable);
static ErrorCode_t MM_CMD_CheckFirmware(const uint32_t firmware_version);
static ErrorCode_t MM_CMD_ProcessScanForDevices ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessRequestSensorvalues ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessDelayedOutput ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessBatteryTestCommand ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessCIEProductCode ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessPPUModeCommand ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessEnterPPUModeCommand ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessExitTestModeCommand ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessCIEDepassivationSettings ( const CIECommand_t* pCommand, const CommandSource_t source );
static ErrorCode_t MM_CMD_ProcessDepassivationSettings(const CO_CommandData_t* pCommand, const CommandSource_t source);
static ErrorCode_t MM_CMD_ProcessEnableFaultsClearMessage ( const CIECommand_t* pCommand, const CommandSource_t source );

/* Global Variables
*************************************************************************************/
extern osPoolId ATHandlePool;
extern osMessageQId(ATHandleQ);
extern osPoolId AppPool;
extern osMessageQId(AppQ);     // Queue to the application
extern osMessageQId(HeadQ);
extern int32_t gHeadQueueCount;
#ifndef USE_NEW_HEAD_INTERFACE
extern osSemaphoreId HeadInterfaceSemaphoreId;
#endif

extern LoraParameters_t loraParameters;
extern bool gLoraParametersChanged;
extern bool gTxPowerChanged;
extern MC_MAC_TestMode_t gTestMode;
extern const char sw_version_nbr[];
extern const char sw_version_date[];
extern uint32_t blf_application_checksum;
extern const char ParamCommandMap[PARAM_TYPE_MAX_E][5];
extern const char CIE_ERROR_CODE_STR[AT_CIE_ERROR_MAX_E][3];
extern uint16_t gLastCommandedChannel;
extern uint8_t gLastCommandedOutputProfile;
extern uint8_t gLastCommandedOutputActive;
extern uint16_t gLastCommandedOutputDuration;
extern CO_DayNight_t gDayNightSetting;
extern DM_BaseType_t gBaseType;
extern Gpio_t SndrBeaconOn;
extern uint8_t gOutputProfile[CO_PROFILE_MAX_E][DM_OP_CHANNEL_MAX_E];
extern NeighbourInformation_t neighbourArray[MAX_DEVICES_PER_SYSTEM];
extern uint32_t rbu_pp_mode_request;

/* Private Variables
*************************************************************************************/
static uint16_t gNetworkAddress;
static bool gIsNCU;
static CO_CommandData_t gCurrentCommand;
static bool gCommandInProgress;
CommandSource_t gCommandSource;
static uint32_t gSoftwareVersionNumber = 0;
static uint32_t gBackupSoftwareVersionNumber = 0;
uint16_t gDeviceCombination = 0;

uint32_t gBank1VersionStringLocation = 0x08040188;
uint32_t gBank2VersionStringLocation = 0x08040188;

uint32_t gResetToPpenMode = 0;

/* Function Definitions
*************************************************************************************/

/*****************************************************************************
*  Function:   MM_CMD_Initialise
*  Description:      Initialise the command processor data. 
*  Param - address:  The network address of the RBU/NCU.
*  Param - device_combination: The device configuration.
*  Param - unitSerialNo: the serial number of the RBU or NCU.
*  Returns:          None.
*  Notes:            
*****************************************************************************/
void MM_CMD_Initialise(const uint16_t address, const uint16_t device_combination, const bool sync_master, const uint32_t unitSerialNo)
{
   gNetworkAddress = address;
   gIsNCU = sync_master;
   gDeviceCombination = device_combination;
   memset(&gCurrentCommand, 0, sizeof(CO_CommandData_t));
   gCommandInProgress = false;
   
   /* pack the software version number into an integer */
   char* pVersion = (char*)sw_version_nbr;
   if ( pVersion )
   {
      /* read the base number */
      char* str_end;
      uint32_t base_number = (uint32_t)strtol(pVersion, &str_end, 10);
      
      /* read the revision number */
      pVersion = str_end;
      if ( pVersion )
      {
         /* step over the full stop */
         pVersion++;
         uint32_t revision = (uint32_t)strtol(pVersion, &str_end, 10);
         /* read the sub-revision number */
         pVersion = str_end;
         if ( pVersion )
         {
            /* step over the full stop */
            pVersion++;
            uint32_t sub_revision = (uint32_t)strtol(pVersion, &str_end, 10);
            
            /* pack into a uint32 */
            gSoftwareVersionNumber = (base_number << 14) + (revision << 7) + sub_revision;
         }
      }      
   }
   
   /* pack the backup software version number into an integer */
   uint32_t active_bank =  FLASH_If_GetActiveBank();
   if ( FLASH_BANK_1 == active_bank )
   {
      pVersion = (char*)gBank1VersionStringLocation;
   }
   else 
   {
      pVersion = (char*)gBank2VersionStringLocation;
   }
   
   if ( pVersion )
   {
      /* read the base number */
      char* str_end;
      uint32_t base_number = (uint32_t)strtol(pVersion, &str_end, 10);
      
      /* read the revision number */
      pVersion = str_end;
      if ( pVersion )
      {
         /* step over the full stop */
         pVersion++;
         uint32_t revision = (uint32_t)strtol(pVersion, &str_end, 10);
         /* read the sub-revision number */
         pVersion = str_end;
         if ( pVersion )
         {
            /* step over the full stop */
            pVersion++;
            uint32_t sub_revision = (uint32_t)strtol(pVersion, &str_end, 10);
            
            /* pack into a uint32 */
            gBackupSoftwareVersionNumber = (base_number << 14) + (revision << 7) + sub_revision;
         }
      }      
   }


}

/*****************************************************************************
*  Function:   MM_CMD_ConvertFromATCommandSource
*  Description:      Converts the USART ID from an AT message to a CommandSource_t
*                    for use in the comand processor. 
*  Param - at_source:  The ID number from the source USART.
*  Returns:          CommandSource_t. The type used by the command processor
*                    for routing responses back to the source of the command.
*  Notes:            
*****************************************************************************/
CommandSource_t MM_CMD_ConvertFromATCommandSource( const uint16_t at_source )
{
   CommandSource_t source;
   
   switch( at_source )
   {
      case DEBUG_UART_E:
         source = CMD_SOURCE_UART_DEBUG_E;
         break;
      case PPU_UART_E:
         source = CMD_SOURCE_UART_PPU_E;
         break;
      case HEAD_UART_E:
         source = CMD_SOURCE_UART_HEAD_E;
         break;
      case NCU_CP_UART_E:
         source = CMD_SOURCE_UART_CP_E;
         break;
      case NCU_SBC_UART_E:
         source = CMD_SOURCE_UART_SBC_E;
         break;
      case MESH_INTERFACE_E:
         source = CMD_SOURCE_MESH_E;
         break;
      default:
         source = CMD_SOURCE_INVALID_E;
         break;
   }
   
   return source;
}

/*****************************************************************************
*  Function:   MM_CMD_ConvertToATCommandSource
*  Description:      Converts a CommandSource_t to a USART ID.
*  Param - cmd_source:  The ID number from the source USART.
*  Returns: UartComm_t The ID of the USART that was the command source.
*  Notes:            
*****************************************************************************/
UartComm_t MM_CMD_ConvertToATCommandSource( const CommandSource_t cmd_source )
{
   UartComm_t source;
   
   switch( cmd_source )
   {
      case CMD_SOURCE_UART_DEBUG_E:
         source = DEBUG_UART_E;
         break;
      case CMD_SOURCE_UART_PPU_E:
         source = PPU_UART_E;
         break;
      case CMD_SOURCE_UART_HEAD_E:
         source = HEAD_UART_E;
         break;
      case CMD_SOURCE_UART_CP_E:
         source = NCU_CP_UART_E;
         break;
      case CMD_SOURCE_UART_SBC_E:
         source = NCU_SBC_UART_E;
         break;
      case CMD_SOURCE_MESH_E:
         source = MESH_INTERFACE_E;
         break;
      default:
         source = INVALID_SERIAL_PORT_E;/* invalid value */
         break;
   }
   
   return source;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessCommand
*  Description:      Process the supplied command message.  Provide an immediate 
*                    response if possible, otherwise send a message to the Head and
*                    store the command pending a response.
*  Param - pCommand: The command to be processed.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCommand ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   
   if ( CMD_SOURCE_INVALID_E != source )
   {
      if ( pCommand )
      {
         if ( pCommand->Destination == gNetworkAddress || pCommand->Destination == ADDRESS_GLOBAL )
         {
            // default response
            result = ERR_NOT_FOUND_E;
            /* create a response msg*/
            CO_ResponseData_t response = { 0 };
            response.CommandType = pCommand->CommandType;
            response.Source = pCommand->Source;
            response.Destination = pCommand->Destination;
            response.Parameter1 = pCommand->Parameter1;
            response.Parameter2 = pCommand->Parameter2;
            response.ReadWrite = pCommand->ReadWrite;
            response.TransactionID = pCommand->TransactionID;
            response.Value = pCommand->Value;
            
            switch( pCommand->CommandType )
            {
               case PARAM_TYPE_TONE_SELECTION_E:
                  {
                     // convert the system channel index to an output channel index
                     DM_OP_OutputChannel_t outputChannel = DM_OP_SystemChannelToOutputChannel((CO_ChannelIndex_t)response.Parameter1);
                     if ( DM_OP_CHANNEL_MAX_E > outputChannel )
                     {
                        if ( COMMAND_READ == pCommand->ReadWrite )
                        {
                              uint8_t profile = (uint8_t)(pCommand->Value >> 8 ) & 0xFF;
                              
                              response.Value &= 0xFF00;//Remove all but the profile
                              response.Value |= gOutputProfile[profile][outputChannel];
                        }
                        else 
                        {
                           uint8_t profile = (uint8_t)(pCommand->Value >> 8 ) & 0xFF;
                           uint8_t tone = (uint8_t)(pCommand->Value & 0xFF);
                           if ( CO_PROFILE_MAX_E > profile && DM_SVI_TONE_MAX_E > tone)
                           {
                              gOutputProfile[profile][outputChannel] = tone;
                           }
                           else 
                           {
                              result = ERR_OUT_OF_RANGE_E;
                              response.Value = 0xFFFFFFFF;//Fail indication
                           }
                        }
                     }
                     
                     if ( CMD_SOURCE_INVALID_E != source )
                     {
                        /* Send response back to the command's originator */;
                        if( CMD_SOURCE_MESH_E != source )
                        {
                           /* respond over AT USART link */
                           MM_CMD_SendATResponse( &response, source );
                           result = SUCCESS_E;
                        }
                        else 
                        {
                           /* respond over Mesh link */
                           bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                                      response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
                           if ( msg_sent )
                           {
                              result = SUCCESS_E;
                           }
                        }
                     }
                  }
                  break;
               case PARAM_TYPE_FLASH_RATE_E:
                  {
                     // convert the system channel index to an output channel index
                     DM_OP_OutputChannel_t outputChannel = DM_OP_SystemChannelToOutputChannel((CO_ChannelIndex_t)response.Parameter1);
                     if ( DM_OP_CHANNEL_MAX_E > outputChannel )
                     {
                        if ( COMMAND_READ == pCommand->ReadWrite )
                        {
                            uint8_t profile = (uint8_t)(pCommand->Value >> 8 ) & 0xFF;
                           
                           response.Value &= 0xFF00;//Remove all but the profile
                           response.Value |= gOutputProfile[profile][outputChannel];
                        }
                        else 
                        {
                           uint8_t profile = (uint8_t)(pCommand->Value >> 8 ) & 0xFF;
                           uint8_t flash = (uint8_t)(pCommand->Value & 0xFF);
                           if ( CO_PROFILE_MAX_E > profile && FLASH_RATE_MAX_E > flash)
                           {
                              gOutputProfile[profile][outputChannel] = flash;
                           }
                           else 
                           {
                              result = ERR_OUT_OF_RANGE_E;
                              response.Value = 0xFFFFFFFF;//Fail indication
                           }
                           
                        }
                     }
                     
                     if ( CMD_SOURCE_INVALID_E != source )
                     {
                        /* Send response back to the command's originator */;
                        if( CMD_SOURCE_MESH_E != source )
                        {
                           /* respond over AT USART link */
                           MM_CMD_SendATResponse( &response, source );
                           result = SUCCESS_E;
                        }
                        else 
                        {
                           /* respond over Mesh link */
                           bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                                      response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
                           if ( msg_sent )
                           {
                              result = SUCCESS_E;
                           }
                        }
                     }
                  }
                  break;
               case PARAM_TYPE_INDICATOR_LED_E:
                  {
                     bool send_response = false;
                     
                     if ( COMMAND_READ == pCommand->ReadWrite )
                     {
                        //If there is an indicator LED on the head, forward the message to it.
                        if ( DM_DeviceIsEnabled( gDeviceCombination, DEV_INDICATOR_LED, DC_MATCH_ANY_E ) )
                        {
                           /* Head beacon */
                           result = MM_CMD_ProcessHeadCommand( pCommand, source );
                        }
                        else 
                        {
                           DM_LedPatterns_t led_pattern = DM_LedGetCurrentPattern();
                           if ( (LED_FIRE_INDICATION_E == led_pattern) || (LED_FIRST_AID_INDICATION_E == led_pattern))
                           {
                              response.Value = 1;
                           }
                           else 
                           {
                              response.Value = 0;
                           }
                           send_response = true;
                        }
                     }
                     else 
                     {
                        //If there is an indicator LED on the head, forward the message to it.
                        if ( DM_DeviceIsEnabled( gDeviceCombination, DEV_INDICATOR_LED, DC_MATCH_ANY_E ) )
                        {
                           /* Head beacon */
                           gCurrentCommand.Value = pCommand->Value;
                           result = MM_CMD_ProcessHeadCommand( pCommand, source );
                           if ( SUCCESS == result )
                           {
                              // The head took the message.  Make sure the RGB Status LED is not on.
                              DM_LedPatternRemove(LED_FIRE_INDICATION_E);
                              DM_LedPatternRemove(LED_FIRST_AID_INDICATION_E);
                           }
                           else 
                           {
                              // No head so revert to the status LED
                              if ( CO_PROFILE_FIRST_AID_E == pCommand->Parameter1 )
                              {
                                 // GREEN for first aid
                                 if ( pCommand->Value )
                                 {
                                    DM_LedPatternRequest(LED_FIRST_AID_INDICATION_E);
                                 }
                                 else 
                                 {
                                    DM_LedPatternRemove(LED_FIRST_AID_INDICATION_E);
                                 }
                              }
                              else 
                              {
                                 //RED for everything except first aid
                                 if ( pCommand->Value )
                                 {
                                    DM_LedPatternRequest(LED_FIRE_INDICATION_E);
                                 }
                                 else 
                                 {
                                    DM_LedPatternRemove(LED_FIRE_INDICATION_E);
                                 }
                              }
                              //Send response back to the command source
                              send_response = true;
                           }
                        }
                        else 
                        {
                           // get the highest active profile
                           CO_OutputProfile_t profile = DM_OP_GetHighestPriorityActiveProfile();
                           // No head LEDs so revert to the status LED
                           if ( CO_PROFILE_MAX_E == profile )
                           {
                              // No active profiles.  The output may have been switched of before the LED.
                              //If it's an 'off' command, switch both red and green off.
                              if ( 0 == pCommand->Value )
                              {
                                 DM_LedPatternRemove(LED_CONSTANT_RED_E);
                                 DM_LedPatternRemove(LED_CONSTANT_GREEN_E);
                              }
                           }
                           else if ( CO_PROFILE_FIRST_AID_E == profile )
                           {
                              // GREEN for first aid
                              if ( pCommand->Value )
                              {
                                 DM_LedPatternRequest(LED_CONSTANT_GREEN_E);
                              }
                              else 
                              {
                                 DM_LedPatternRemove(LED_CONSTANT_GREEN_E);
                              }
                           }
                           else 
                           {
                              //RED for everything except first aid
                              if ( pCommand->Value )
                              {
                                 DM_LedPatternRequest(LED_CONSTANT_RED_E);
                              }
                              else 
                              {
                                 DM_LedPatternRemove(LED_CONSTANT_RED_E);
                              }
                           }
                           send_response = true;
                        }
                     }

                     if ( send_response )
                     {
                        /* Send response back to the command's originator */;
                        if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
                        {
                           /* respond over AT USART link */
                           MM_CMD_SendATResponse( &response, source );
                           result = SUCCESS_E;
                        }
                        else
                        {
                           /* respond over Mesh link */
                           bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                                      response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
                           if ( msg_sent )
                           {
                              result = SUCCESS_E;
                           }
                        }
                     }
                  }
                  break;
               case PARAM_TYPE_ANALOGUE_VALUE_E:
               {
                  if ( (BASE_RBU_E == gBaseType) || (BASE_CONSTRUCTION_E == gBaseType) )
                  {
                     //constuction PIR is on GPIO
                     if ( (BASE_CONSTRUCTION_E == gBaseType) && (CO_CHANNEL_PIR_E == pCommand->Parameter1) )
                     {
                        //Read the PIR current state
                        response.Value = 0;
                        if ( IN_MON_ACTIVE_E == DM_GetPirState() )
                        {
                           response.Value = 1;
                        }
                        
                        /* Send response back to the command's originator */;
                        if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
                        {
                           /* respond over AT USART link */
                           MM_CMD_SendATResponse( &response, source );
                           result = SUCCESS_E;
                        }
                        else
                        {
                           /* respond over Mesh link */
                           bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                                      response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
                           if ( msg_sent )
                           {
                              result = SUCCESS_E;
                           }
                        }
                     }
                     else 
                     {
                        /* Head command */
                        gCurrentCommand.Value = pCommand->Value;
                        result = MM_CMD_ProcessHeadCommand( pCommand, source );
                     }
                  }
                  else if ( BASE_IOU_E == gBaseType )
                  {
                     /* local device command */
                     result = MM_CMD_ProcessLocalCommand( pCommand, source );
                  }
               }
               break;
               case PARAM_TYPE_SOUND_LEVEL_E:
                  {
                     if ( COMMAND_READ == pCommand->ReadWrite )
                     {
                        response.Value = (uint32_t)CFG_GetSoundLevel();
                     }
                     else 
                     {
                        if ( SUCCESS_E != DM_OP_SetSoundLevel(pCommand->Value) )
                        {
                           response.Value = 0xffffffff; //Fail indication for response
                        }
                     }
                     
                     if ( CMD_SOURCE_INVALID_E != source )
                     {
                        /* Send response back to the command's originator */;
                        if( CMD_SOURCE_MESH_E != source )
                        {
                           /* respond over AT USART link */
                           MM_CMD_SendATResponse( &response, source );
                           result = SUCCESS_E;
                        }
                        else 
                        {
                           /* respond over Mesh link */
                           bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                                      response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
                           if ( msg_sent )
                           {
                              result = SUCCESS_E;
                           }
                        }
                     }
                  }
                  break;
               case PARAM_TYPE_PIR_RESET_E:
                  if ( BASE_RBU_E == gBaseType )
                  {
                     result = MM_CMD_ProcessHeadCommand( pCommand, source );
                  }
                  else 
                  {
                     //copy the current command and source.  The construction PIR will respond via MM_CMD_ProcessHeadMessage.
                     gCurrentCommand.CommandType = pCommand->CommandType;
                     gCurrentCommand.Source = pCommand->Source;
                     gCurrentCommand.Destination = pCommand->Destination;
                     gCurrentCommand.Parameter1 = pCommand->Parameter1;
                     gCurrentCommand.Parameter2 = pCommand->Parameter2;
                     gCurrentCommand.ReadWrite = pCommand->ReadWrite;
                     gCurrentCommand.TransactionID = pCommand->TransactionID;
                     gCurrentCommand.Value = pCommand->Value;
                     gCommandInProgress = true;
                     gCommandSource = source;
                     result = SUCCESS_E;
                  }
                  break;
               case PARAM_TYPE_ALARM_THRESHOLD_E: /* intentional drop through */
               case PARAM_TYPE_PRE_ALARM_THRESHOLD_E:
               case PARAM_TYPE_FAULT_THRESHOLD_E:
               case PARAM_TYPE_PLUGIN_SERIAL_NUMBER_E:
               case PARAM_TYPE_PLUGIN_ENABLE_E:
               case PARAM_TYPE_PLUGIN_TEST_MODE_E:
               case PARAM_TYPE_PLUGIN_ID_NUMBER_E:
               case PARAM_TYPE_FAULT_TYPE_E:
               case PARAM_TYPE_PLUGIN_DETECTOR_TYPE_E:
                  /* Head command */
                  gCurrentCommand.Value = pCommand->Value;
                  result = MM_CMD_ProcessHeadCommand( pCommand, source );
                  break;
               case PARAM_TYPE_DAY_NIGHT_SETTING_E:
                  if ( CO_NUM_DAYNIGHT_SETTINGS_E > pCommand->Value )
                  {
                     result = MM_CMD_ProcessDayNightSetting( (CO_DayNight_t)pCommand->Value );
                  }
                  break;
               case PARAM_TYPE_ALARM_CONFIG_E:
                  if ( COMMAND_WRITE == pCommand->ReadWrite )
                  {
                     result = DM_OP_SetAlarmConfiguration((CO_ChannelIndex_t)pCommand->Parameter1, pCommand->Value);
                  }
                  else
                  {
                     uint32_t config;
                     result = DM_OP_GetAlarmConfiguration((CO_ChannelIndex_t)pCommand->Parameter1, &config);
                     response.Value = config;
                  }
                  
                  if (SUCCESS_E != result)
                  {
                     response.Value = 0xFFFFFFFF; // fail value
                  }
                  
                  if ( CMD_SOURCE_INVALID_E != source )
                  {
                     /* Send response back to the command's originator */;
                     if( CMD_SOURCE_MESH_E != source )
                     {
                        /* respond over AT USART link */
                        MM_CMD_SendATResponse( &response, source );
                        result = SUCCESS_E;
                     }
                     else 
                     {
                        /* respond over Mesh link */
                        bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                                   response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
                        if ( msg_sent )
                        {
                           result = SUCCESS_E;
                        }
                     }
                  }
                  break;
               case PARAM_TYPE_ALARM_OPTION_FLAGS_E:
                  if ( COMMAND_WRITE == pCommand->ReadWrite )
                  {
                     result = DM_OP_SetAlarmOptionFlags((CO_ChannelIndex_t)pCommand->Parameter1, pCommand->Value);
                  }
                  else
                  {
                     uint32_t config;
                     result = DM_OP_GetAlarmOptionFlags((CO_ChannelIndex_t)pCommand->Parameter1, &config);
                     response.Value = config;
                  }
                  
                  if (SUCCESS_E != result)
                  {
                     response.Value = 0xFFFFFFFF; // fail value
                  }
                  
                  if ( CMD_SOURCE_INVALID_E != source )
                  {
                     /* Send response back to the command's originator */;
                     if( CMD_SOURCE_MESH_E != source )
                     {
                        /* respond over AT USART link */
                        MM_CMD_SendATResponse( &response, source );
                        result = SUCCESS_E;
                     }
                     else 
                     {
                        /* respond over Mesh link */
                        bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                                   response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
                        if ( msg_sent )
                        {
                           result = SUCCESS_E;
                        }
                     }
                  }
                  break;
               case PARAM_TYPE_ALARM_DELAYS_E:
                  if ( COMMAND_WRITE == pCommand->ReadWrite )
                  {
                     result = DM_OP_SetAlarmDelays((CO_ChannelIndex_t)pCommand->Parameter1, pCommand->Value);
                  }
                  else
                  {
                     uint32_t config;
                     result = DM_OP_GetAlarmDelays((CO_ChannelIndex_t)pCommand->Parameter1, &config);
                     response.Value = config;
                  }
                  
                  if (SUCCESS_E != result)
                  {
                     response.Value = 0xFFFFFFFF; // fail value
                  }
                  
                  if ( CMD_SOURCE_INVALID_E != source )
                  {
                     /* Send response back to the command's originator */;
                     if( CMD_SOURCE_MESH_E != source )
                     {
                        /* respond over AT USART link */
                        MM_CMD_SendATResponse( &response, source );
                        result = SUCCESS_E;
                     }
                     else 
                     {
                        /* respond over Mesh link */
                        bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                                   response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
                        if ( msg_sent )
                        {
                           result = SUCCESS_E;
                        }
                     }
                  }
                  break;
               case PARAM_TYPE_SET_LOCAL_GLOBAL_DELAYS_E:
                  {
                     if ( COMMAND_WRITE == pCommand->ReadWrite )
                     {
                        bool enabled = 0 == pCommand->Value ? false:true;
                        result = DM_OP_SetLocalOrGlobalDelayFlag((CO_ChannelIndex_t)pCommand->Parameter1, enabled);
                     }
                     else
                     {
                        response.Value = (uint32_t)DM_OP_GetLocalOrGlobalDelayFlag((CO_ChannelIndex_t)pCommand->Parameter1);
                        result = SUCCESS_E;
                     }
                     
                     if (SUCCESS_E != result)
                     {
                        response.Value = 0xFFFFFFFF; // fail value
                     }
                     
                     if ( CMD_SOURCE_INVALID_E != source )
                     {
                        /* Send response back to the command's originator */;
                        if( CMD_SOURCE_MESH_E != source )
                        {
                           /* respond over AT USART link */
                           MM_CMD_SendATResponse( &response, source );
                           result = SUCCESS_E;
                        }
                     }
                  }
                  break;
               case PARAM_TYPE_GLOBAL_ALARM_OVERRIDE_E:
                  {
                     if ( COMMAND_WRITE == pCommand->ReadWrite )
                     {
                        bool override = 0 == pCommand->Value ? false:true;
                        CFG_SetGlobalDelayOverride(override);
                     }
                     else
                     {
                        response.Value = (uint32_t)CFG_GetGlobalDelayOverride();
                        result = SUCCESS_E;
                     }
                     
                     if (SUCCESS_E != result)
                     {
                        response.Value = 0xFFFFFFFF; // fail value
                     }
                     
                     if ( CMD_SOURCE_INVALID_E != source )
                     {
                        /* Send response back to the command's originator */;
                        if( CMD_SOURCE_MESH_E != source )
                        {
                           /* respond over AT USART link */
                           MM_CMD_SendATResponse( &response, source );
                           result = SUCCESS_E;
                        }
                     }
                  }
                  break;
               case PARAM_TYPE_GLOBAL_ALARM_DELAY_VALUES_E:
                  {
                     if ( COMMAND_WRITE == pCommand->ReadWrite )
                     {
                        result = DM_OP_SetGlobalDelayValues(pCommand->Value);
                     }
                     else
                     {
                        uint32_t combined_delays;
                        result = DM_OP_GetGlobalDelayValues(&combined_delays);
                        if ( SUCCESS_E == result )
                        {
                           response.Value = combined_delays;
                        }
                     }
                     
                     if (SUCCESS_E != result)
                     {
                        response.Value = 0xFFFFFFFF; // fail value
                     }
                     
                     if ( CMD_SOURCE_INVALID_E != source )
                     {
                        /* Send response back to the command's originator */;
                        if( CMD_SOURCE_MESH_E != source )
                        {
                           /* respond over AT USART link */
                           MM_CMD_SendATResponse( &response, source );
                           result = SUCCESS_E;
                        }
                     }
                  }
                  break;
               case PARAM_TYPE_GLOBAL_DELAY_COMBINED_E:
                  {
                     if ( COMMAND_WRITE == pCommand->ReadWrite )
                     {
                        CO_PRINT_B_1(DBG_INFO_E,"Rx'd Global Delay Combined msg. value=%x\r\n", pCommand->Value);
                        //Extract the values an apply them locally
                        bool delays_enabled = (bool)(pCommand->Value & ATGSET_DELAYS_ENABLE_MASK);
                        bool global_override = (bool)(pCommand->Value & ATGSET_GLOBAL_OVERRIDE_MASK);
                        bool is_day = (bool)(pCommand->Value & ATGSET_IS_DAY_MASK);
                        CO_DayNight_t dayNight = is_day ? CO_DAY_E:CO_NIGHT_E;
                        bool ignore_security_in_day = (bool)(pCommand->Value & ATGSET_IGNORE_SECURITY_DAY_MASK);
                        bool ignore_security_at_night = (bool)(pCommand->Value & ATGSET_IGNORE_SECURITY_NIGHT_MASK);
                        uint32_t global_delay1 = (pCommand->Value & ATGSET_GLOBAL_DELAY1_MASK);
                        global_delay1 *= ATGSET_GLOBAL_DELAY_RESOLUTION;
                        uint32_t global_delay2 = ((pCommand->Value & ATGSET_GLOBAL_DELAY2_MASK) >> ATGSET_GLOBAL_DELAY_BIT_LENGTH);
                        global_delay2 *= ATGSET_GLOBAL_DELAY_RESOLUTION;
                        uint32_t combined_global_delays = global_delay1 + (global_delay2 << 16); //global delay 2 is stored in the upper 16 bits, global delay 1 in the lower 16 bits.
                        
                        CFG_SetGlobalDelayEnabled(delays_enabled);
                        CFG_SetGlobalDelayOverride( global_override );
                        MM_CMD_ProcessDayNightSetting( dayNight );
                        CFG_SetSecurityInhibitDay(ignore_security_in_day);
                        CFG_SetSecurityInhibitNight(ignore_security_at_night);
                        DM_OP_SetGlobalDelayValues(combined_global_delays);
                        
                        result = SUCCESS_E;
                     }
                     
                     if (SUCCESS_E != result)
                     {
                        response.Value = 0xFFFFFFFF; // fail value
                     }
                     
                     if ( CMD_SOURCE_INVALID_E != source )
                     {
                        /* Send response back to the command's originator */;
                        if( CMD_SOURCE_MESH_E != source )
                        {
                           /* respond over AT USART link */
                           MM_CMD_SendATResponse( &response, source );
                           result = SUCCESS_E;
                        }
                     }
                  }
                  break;
               case PARAM_TYPE_BATTERY_STATUS_REQUEST_E:
                  {
                     //Send the battery status message
                     result = DM_BAT_SendBatteryStatus(false);
                  }
                  break;
               case PARAM_TYPE_MAX_RANK_E:
                  if ( COMMAND_WRITE == pCommand->ReadWrite )
                  {
                     result = MC_SetMaxRank(pCommand->Value);
                  }
                  else
                  {
                     response.Value = MC_GetMaxRank();
                     result = SUCCESS_E;
                  }
                  
                  if (SUCCESS_E != result)
                  {
                     response.Value = 0xFFFFFFFF; // fail value
                  }
                  
                  if ( CMD_SOURCE_INVALID_E != source )
                  {
                     /* Send response back to the command's originator */;
                     if( CMD_SOURCE_MESH_E != source )
                     {
                        /* respond over AT USART link */
                        MM_CMD_SendATResponse( &response, source );
                        result = SUCCESS_E;
                     }
                     else 
                     {
                        /* respond over Mesh link */
                        bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                                   response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
                        if ( msg_sent )
                        {
                           result = SUCCESS_E;
                        }
                     }
                  }
                  break;
               case PARAM_TYPE_ENABLE_FAULTS_CLEAR_MSG_E:
                  if ( COMMAND_WRITE == pCommand->ReadWrite )
                  {
                     result = CFG_SetFaultsClearEnable(pCommand->Value);
                  }
                  else
                  {
                     response.Value = CFG_GetFaultsClearEnable();
                     result = SUCCESS_E;
                  }
                  
                  if (SUCCESS_E != result)
                  {
                     response.Value = 0xFFFFFFFF; // fail value
                  }
                  
                  if ( CMD_SOURCE_INVALID_E != source )
                  {
                     /* Send response back to the command's originator */;
                     if( CMD_SOURCE_MESH_E != source )
                     {
                        /* respond over AT USART link */
                        MM_CMD_SendATResponse( &response, source );
                        result = SUCCESS_E;
                     }
                     else 
                     {
                        /* respond over Mesh link */
                        bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                                   response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
                        if ( msg_sent )
                        {
                           result = SUCCESS_E;
                        }
                     }
                  }
                  break;
               case PARAM_TYPE_NEIGHBOUR_INFO_E: /* intentional drop through */
               case PARAM_TYPE_STATUS_FLAGS_E:
               case PARAM_TYPE_DEVICE_COMBINATION_E:
               case PARAM_TYPE_RBU_SERIAL_NUMBER_E:
               case PARAM_TYPE_RBU_ENABLE_E:
               case PARAM_TYPE_MODULATION_BANDWIDTH_E:
               case PARAM_TYPE_SPREADING_FACTOR_E:
               case PARAM_TYPE_FREQUENCY_E:
               case PARAM_TYPE_CODING_RATE_E:
               case PARAM_TYPE_TX_POWER_E:
               case PARAM_TYPE_TEST_MODE_E:
               case PARAM_TYPE_FIRMWARE_ACTIVE_IMAGE_E:
               case PARAM_TYPE_REBOOT_E:
               case PARAM_TYPE_SVI_E:
               case PARAM_TYPE_TX_POWER_LOW_E:
               case PARAM_TYPE_TX_POWER_HIGH_E:
               case PARAM_TYPE_ZONE_E:
               case PARAM_TYPE_MESH_STATUS_E:
               case PARAM_TYPE_CRC_INFORMATION_E:
               case PARAM_TYPE_EEPROM_INFORMATION_E:
               case PARAM_TYPE_ALARM_ACK_E:
               case PARAM_TYPE_EVACUATE_E:
               case PARAM_TYPE_GLOBAL_DELAY_E:
               case PARAM_TYPE_ENABLE_DISABLE_E:
               case PARAM_TYPE_ALARM_RESET_E:
               case PARAM_TYPE_TEST_ONE_SHOT_E:
               case PARAM_TYPE_CONFIRMED_E:
               case PARAM_TYPE_CHANNEL_FLAGS_E:
               case PARAM_TYPE_200_HOUR_TEST_E:
               case PARAM_TYPE_CHECK_FIRMWARE_E:
               case PARAM_TYPE_SENSOR_VALUES_REQUEST_E:
               case PARAM_TYPE_DELAYED_OUTPUT_E:
               case PARAM_TYPE_BATTERY_TEST_E:
               case PARAM_TYPE_PRODUCT_CODE_E:
               case PARAM_TYPE_PPU_MODE_ENABLE_E://Old PPEN
               case PARAM_TYPE_ENTER_PPU_MODE_E: //For entering PPU Disconnected mode
               case PARAM_TYPE_EXIT_TEST_MODE_E: //For Exiting Test mode LED OFF
               case PARAM_TYPE_DEPASSIVATION_SETTINGS_E:
                  /* local device command */
                  result = MM_CMD_ProcessLocalCommand( pCommand, source );
                  break;
               case PARAM_TYPE_FIRMWARE_INFO_E:
                  if ( (FINDX_RBU_MAIN_IMAGE_E == pCommand->Parameter1) || (FINDX_INTTERNAL_INTERFACE_1_I2C_E == pCommand->Parameter1) )
                  {
                     /* local device command */
                     result = MM_CMD_ProcessLocalCommand( pCommand, source );
                  }
                  else if ( FINDX_EXTERNAL_INTERFACE_1_HEAD_E == pCommand->Parameter1 )
                  {
                     /* Head command */
                     gCurrentCommand.Value = pCommand->Value;
                     result = MM_CMD_ProcessHeadCommand( pCommand, source );
                  }
                  break;
               case PARAM_TYPE_SCAN_FOR_DEVICES_E:
               {
                     uint32_t Handle = GetNextTransactionID();
                     uint32_t number_of_messages = CFG_GetNumberOfResends();
                     if ( MM_MeshAPICommandReq(Handle, gNetworkAddress, gNetworkAddress, PARAM_TYPE_SCAN_FOR_DEVICES_E,
                           pCommand->Parameter1, pCommand->Parameter2, pCommand->Value, pCommand->ReadWrite, number_of_messages) )
                     {
                        result = SUCCESS_E;
                     }
               }
                  break;
               case PARAM_TYPE_PLUGIN_FIRMWARE_INFO_E:
                     /* Head command */
                     gCurrentCommand.Value = pCommand->Value;
                     result = MM_CMD_ProcessHeadCommand( pCommand, source );
                  break;
               default:
                  result = ERR_MESSAGE_FAIL_E;
               break;
            }
         }
         
         if ( BASE_NCU_E == gBaseType )
         {
            if ( pCommand->Destination != gNetworkAddress || pCommand->Destination == ADDRESS_GLOBAL )
            {
               if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
               {
                  CO_CommandData_t commandData;
                  commandData.TransactionID = GetNextTransactionID();
                  commandData.Source = NCU_NETWORK_ADDRESS;
                  commandData.CommandType = pCommand->CommandType;
                  commandData.Destination = pCommand->Destination;
                  commandData.Parameter1 = pCommand->Parameter1;
                  commandData.Parameter2 = pCommand->Parameter2;
                  commandData.Value = pCommand->Value;
                  commandData.ReadWrite = pCommand->ReadWrite;
                  commandData.SourceUSART = (uint16_t)source;
                  
                  CieBuffer_t msgBuffer;
                  msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
                  memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
                  
                  result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
               }
               
               if ( SUCCESS_E == result )
               {
                  MM_CMD_SendATMessage(AT_RESPONSE_OK, source); 
               }
               else 
               {
                  MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
               }
            }
         }
      }
      else
      {
         result = ERR_INVALID_POINTER_E;
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessHeadCommand
*  Description:      Process the supplied message for a sensor head.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessHeadCommand ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   HeadMessage_t headMessage;
   MM_HeadDeviceType_t head_device;
   bool sendMessage = true;
   
   if ( gIsNCU )
   {
      result = ERR_MESSAGE_FAIL_E;
   }
   else
   {
      if ( pCommand )
      {
         /* We need to extract the RU Channel for all commands except 
            PARAM_TYPE_FIRMWARE_INFO_E  and PARAM_TYPE_INDICATOR_LED_E*/
         result = SUCCESS_E;
         if ( (PARAM_TYPE_FIRMWARE_INFO_E != pCommand->CommandType) &&
              (PARAM_TYPE_INDICATOR_LED_E != pCommand->CommandType) )
         {
            result = DM_MapRUChannelIndexToHeadDevice(gDeviceCombination, pCommand->Parameter1, &head_device);
         }
         
         if ( SUCCESS_E == result )
         {
            switch( pCommand->CommandType )
            {
               case PARAM_TYPE_ANALOGUE_VALUE_E:
                  if (COMMAND_READ == pCommand->ReadWrite)
                  {
                     headMessage.MessageType = HEAD_READ_ANALOGUE_VALUE_E;
                  }
                  else
                  {
                     /* write not supported */
                     result = ERR_MESSAGE_FAIL_E;
                  }
               break;
               case PARAM_TYPE_ALARM_THRESHOLD_E:
                  if ( COMMAND_READ == pCommand->ReadWrite )
                  {
                     headMessage.MessageType = HEAD_READ_ALARM_THRESHOLD_E;
                  }
                  else
                  {
                     headMessage.MessageType = HEAD_WRITE_ALARM_THRESHOLD_E;
                  }
                  break;
               case PARAM_TYPE_PRE_ALARM_THRESHOLD_E:
                  if ( COMMAND_READ == pCommand->ReadWrite )
                  {
                     headMessage.MessageType = HEAD_READ_PREALARM_THRESHOLD_E;
                  }
                  else
                  {
                     headMessage.MessageType = HEAD_WRITE_PREALARM_THRESHOLD_E;
                  }
                  break;
               case PARAM_TYPE_FAULT_THRESHOLD_E:
                  if ( COMMAND_READ == pCommand->ReadWrite )
                  {
                     headMessage.MessageType = HEAD_READ_FAULT_THRESHOLD_E;
                  }
                  else
                  {
                     headMessage.MessageType = HEAD_WRITE_FAULT_THRESHOLD_E;
                  }
                  break;
               case PARAM_TYPE_FLASH_RATE_E:
                  if ( COMMAND_READ == pCommand->ReadWrite )
                  {
                     headMessage.MessageType = HEAD_READ_FLASH_RATE_E;
                  }
                  else
                  {
                     headMessage.MessageType = HEAD_WRITE_FLASH_RATE_E;
                  }
                  break;
                  case PARAM_TYPE_INDICATOR_LED_E:
                     if ( COMMAND_READ == pCommand->ReadWrite )
                     {
                        headMessage.MessageType = HEAD_READ_INDICATOR_LED_E;
                     }
                     else
                     {
                        headMessage.MessageType = HEAD_WRITE_INDICATOR_LED_E;
                     }
                     break;   
               case PARAM_TYPE_TONE_SELECTION_E:
                  if ( COMMAND_READ == pCommand->ReadWrite )
                  {
                     headMessage.MessageType = HEAD_READ_TONE_SELECTION_E;
                  }
                  else
                  {
                     headMessage.MessageType = HEAD_WRITE_TONE_SELECTION_E;
                  }
                  break;
               case PARAM_TYPE_PLUGIN_SERIAL_NUMBER_E:               
                  if (COMMAND_READ == pCommand->ReadWrite)
                  {
                     headMessage.MessageType = HEAD_READ_SERIAL_NUMBER_E;
                  }
                  else
                  {
                     /* write not supported */
                     result = ERR_MESSAGE_FAIL_E;
                  }
                  break;
               case PARAM_TYPE_PLUGIN_ENABLE_E:
                  if ( COMMAND_READ == pCommand->ReadWrite )
                  {
                     headMessage.MessageType = HEAD_READ_ENABLED_E;
                  }
                  else
                  {
                     headMessage.MessageType = HEAD_WRITE_ENABLED_E;
                  }
                  break;
               case PARAM_TYPE_PLUGIN_TEST_MODE_E:
                  if ( COMMAND_READ == pCommand->ReadWrite )
                  {
                     headMessage.MessageType = HEAD_READ_TEST_MODE_E;
                  }
                  else
                  {
                     headMessage.MessageType = HEAD_WRITE_TEST_MODE_E;
                  }
                  break;
               case PARAM_TYPE_FIRMWARE_INFO_E:
                     if (COMMAND_READ == pCommand->ReadWrite)
                     {
                        headMessage.MessageType = HEAD_READ_FIRMWARE_VERSION_E;
                     }
                     else
                     {
                        /* write not supported */
                        result = ERR_MESSAGE_FAIL_E;
                     }
                     break;
               case PARAM_TYPE_PLUGIN_FIRMWARE_INFO_E:
                     if (COMMAND_READ == pCommand->ReadWrite)
                     {
                        headMessage.MessageType = HEAD_READ_FIRMWARE_VERSION_E;
                     }
                     else
                     {
                        /* write not supported */
                        result = ERR_MESSAGE_FAIL_E;
                     }
                  break;
                  case PARAM_TYPE_PIR_RESET_E:
                     if (COMMAND_READ == pCommand->ReadWrite)
                     {
                        /* read not supported */
                        result = ERR_MESSAGE_FAIL_E;
                     }
                     else
                     {
                        headMessage.MessageType = HEAD_RESET_PIR_E;
                     }
                     break;
                  case PARAM_TYPE_FAULT_TYPE_E:
                     if (COMMAND_READ == pCommand->ReadWrite)
                     {
                        headMessage.MessageType = HEAD_READ_FAULT_TYPE_E;
                     }
                     else
                     {
                        /* write not supported */
                        result = ERR_MESSAGE_FAIL_E;
                     }
                     break;
                  case PARAM_TYPE_CRC_INFORMATION_E:
                     /* not supported by Head interface */
                     result = ERR_MESSAGE_FAIL_E;
                     break;               
                  case PARAM_TYPE_PLUGIN_ID_NUMBER_E:
                     if ( COMMAND_READ == pCommand->ReadWrite )
                     {
                        headMessage.MessageType = HEAD_READ_IDENTIFICATION_NUMBER_E;
                     }
                     else
                     {
                        headMessage.MessageType = HEAD_WRITE_IDENTIFICATION_NUMBER_E;
                     }
                     break;   
                  case PARAM_TYPE_PLUGIN_DETECTOR_TYPE_E:
                     if ( COMMAND_READ == pCommand->ReadWrite )
                     {
                        headMessage.MessageType = HEAD_READ_DEVICE_TYPE_E;
                     }
                     break;
                  case PARAM_TYPE_PLUGIN_TYPE_AND_CLASS_E:
                     if ( COMMAND_READ == pCommand->ReadWrite )
                     {
                        headMessage.MessageType = HEAD_READ_DEVICE_TYPE_AND_CLASS_E;
                     }
                     break;
               default:
                  sendMessage = false;
                  result = ERR_MESSAGE_FAIL_E;
                  break;
            }
            
            if ( sendMessage && SUCCESS_E == result )
            {
               /* we recognised the message type.  now pack the head message */
               headMessage.Value = pCommand->Value; 
               headMessage.TransactionID = pCommand->TransactionID;
                  headMessage.ChannelNumber = head_device;
               headMessage.ProfileIndex = pCommand->Parameter2;
               /* send the message to the head */
               if ( MM_CMD_SendHeadMessage( CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E, &headMessage, source ) )
               {
                  /* keep a copy of the command so that we can match the response from the head */
                  gCurrentCommand.CommandType = pCommand->CommandType;
                  gCurrentCommand.TransactionID = pCommand->TransactionID;
                  gCurrentCommand.Destination = pCommand->Destination;
                  gCurrentCommand.Source = pCommand->Source;
                  gCurrentCommand.Parameter1 = pCommand->Parameter1;
                  gCurrentCommand.Parameter2 = pCommand->Parameter2;
                  gCurrentCommand.ReadWrite = pCommand->ReadWrite;
                  gCurrentCommand.Value = pCommand->Value;
                  gCommandInProgress = true;
                  gCommandSource = source;
                  
                  result = SUCCESS_E;
                  if ( COMMAND_WRITE == pCommand->ReadWrite )
                  {
                     MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
                  }
               }
            }
         }
      }
   }

   /* respond over the source link */
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
   }

   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessLocalCommand
*  Description:      Process messages addressed to this RBU/NCU 
*                    (the radio board, not the attached sensors).
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessLocalCommand ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
      switch( pCommand->CommandType )
      {
         case PARAM_TYPE_ANALOGUE_VALUE_E:
            result = MM_CMD_ProcessCommandAnalogueValue( pCommand, source );
            break;
         case PARAM_TYPE_NEIGHBOUR_INFO_E:
            result = MM_CMD_ProcessCommandNeighbourInformation( pCommand, source );
            break;
         case PARAM_TYPE_STATUS_FLAGS_E:
            result = MM_CMD_ProcessCommandStatusFlags( pCommand, source );
            break;
         case PARAM_TYPE_DEVICE_COMBINATION_E:
            result = MM_CMD_ProcessCommandDeviceCombination( pCommand, source );
            break;
         case PARAM_TYPE_RBU_SERIAL_NUMBER_E:
            result = MM_CMD_ProcessCommandRBUSerialNumber( pCommand, source );
            break;
         case PARAM_TYPE_RBU_ENABLE_E:
            result = MM_CMD_ProcessCommandRBUEnable( pCommand, source );
            break;
         case PARAM_TYPE_MODULATION_BANDWIDTH_E:
            result = MM_CMD_ProcessModulationBandwidth( pCommand, source );
            break;            
         case PARAM_TYPE_SPREADING_FACTOR_E:
            result = MM_CMD_ProcessSpreadingFactor( pCommand, source );
            break;            
         case PARAM_TYPE_FREQUENCY_E:
            result = MM_CMD_ProcessFrequency( pCommand, source );
            break;            
         case PARAM_TYPE_CODING_RATE_E:
            result = MM_CMD_ProcessCodingRate( pCommand, source );
            break;            
         case PARAM_TYPE_TX_POWER_E:
            result = MM_CMD_ProcessTxPower( pCommand, source );
            break;
         case PARAM_TYPE_TEST_MODE_E:
            result = MM_CMD_ProcessRBUTestMode( pCommand, source );
            break;            
         case PARAM_TYPE_FIRMWARE_INFO_E:
            result = MM_CMD_ProcessFirmwareInformation( pCommand, source );
            break;            
         case PARAM_TYPE_FIRMWARE_ACTIVE_IMAGE_E:
            result = MM_CMD_ProcessFirmwareActiveImage( pCommand, source );
            break;            
         case PARAM_TYPE_REBOOT_E:
            result = MM_CMD_ProcessReboot( pCommand, source );
            break;
         case PARAM_TYPE_SVI_E:
            result = MM_CMD_ProcessSVIComand(pCommand, source );
            break;
         case PARAM_TYPE_TX_POWER_LOW_E:  /* intentional drop through */
         case PARAM_TYPE_TX_POWER_HIGH_E:
            result = MM_CMD_ProcessCommandTxPowerLevel( pCommand, source );
            break;
         case PARAM_TYPE_ZONE_E:
            result = MM_CMD_ProcessCommandZone( pCommand, source );
            break;
         case PARAM_TYPE_MESH_STATUS_E:
            result = MM_CMD_ProcessMeshStatusRequest( pCommand, source );
            break;
         case PARAM_TYPE_CRC_INFORMATION_E:
            result = MM_CMD_ProcessCRCInformation( pCommand, source );
            break;
         case PARAM_TYPE_EEPROM_INFORMATION_E:
            result = MM_CMD_ProcessEEPROMInformation( pCommand, source );
            break;
         case PARAM_TYPE_ALARM_ACK_E:
            result = DM_OP_AlarmAcknowledge((CO_OutputProfile_t)pCommand->Value);
            break;
         case PARAM_TYPE_EVACUATE_E:
            result = DM_OP_AlarmEvacuate();
            break;
         case PARAM_TYPE_GLOBAL_DELAY_E:
            if ( COMMAND_WRITE == pCommand->ReadWrite )
            {
               result = CFG_SetGlobalDelayEnabled((bool)pCommand->Value);
            }
            else 
            {
               bool enabled = CFG_GetGlobalDelayEnabled();
               CO_PRINT_A_1(DBG_INFO_E,"AGD:%d\r\n", enabled);
               result = SUCCESS_E;
            }
            break;
         case PARAM_TYPE_ENABLE_DISABLE_E:
            if ( CO_DISABLE_MAX_E > pCommand->Parameter1 )
            {
               result = DM_OP_SetEnableDisable((CO_DisableRegion_t)pCommand->Parameter1, pCommand->Parameter2,  pCommand->Value);
            }
            else 
            {
               result = ERR_OUT_OF_RANGE_E;
            }
            break;
         case PARAM_TYPE_ALARM_RESET_E:
            result = MM_CMD_AlarmReset( pCommand->Parameter1 );
            break;
         case PARAM_TYPE_SOUND_LEVEL_E:
            result = ERR_OUT_OF_RANGE_E;
            break;
         case PARAM_TYPE_TEST_ONE_SHOT_E:
            result = DM_OP_TestOneShot(pCommand->Value);
            break;
         case PARAM_TYPE_CONFIRMED_E:
            result = DM_OP_Confirmed((CO_OutputProfile_t)pCommand->Value);
            break;
         case PARAM_TYPE_CHANNEL_FLAGS_E:
            result = MM_CMD_ProcessChannelFlags ( pCommand, source );
            break;
         case PARAM_TYPE_200_HOUR_TEST_E:
            TE_200HourTestEnable((bool)pCommand->Value, (CO_ChannelIndex_t)pCommand->Parameter1);
            result = SUCCESS_E;
            break;
         case PARAM_TYPE_CHECK_FIRMWARE_E:
            result = MM_CMD_CheckFirmware(pCommand->Value);
            break;
         case PARAM_TYPE_SENSOR_VALUES_REQUEST_E:
            result = CO_RequestSensorValues();
            break;
         case PARAM_TYPE_DELAYED_OUTPUT_E:
         {
            uint32_t initial_delay = (pCommand->Value >> 16);
            uint32_t output_duration = (pCommand->Value & 0xFFFF);
            result = DM_OP_StartDelayedOutput(initial_delay, output_duration, (CO_ChannelIndex_t)pCommand->Parameter1);
         }
            break;
         case PARAM_TYPE_BATTERY_TEST_E:
            //Reset the battery restart flag in NVM
            DM_BAT_SetBatteryRestartFlag(DM_BAT_NORMAL_RESTART_E);
            //Initiate immediate battery test
            DM_BAT_RequestBatteryCheck(false);
            result = SUCCESS_E;
            break;
         case PARAM_TYPE_PRODUCT_CODE_E:
            result = MM_CMD_ProcessProductCode(pCommand, source);
            break;
         case PARAM_TYPE_PPU_MODE_ENABLE_E:
            result = MM_CMD_ProcessPPUModeEnable(pCommand, source);
            break;
         case PARAM_TYPE_ENTER_PPU_MODE_E:
         {
            ApplicationMessage_t AppMessage;
            AppMessage.MessageType = APP_MSG_TYPE_PPU_MODE_E;
            result = MM_PPU_ApplicationProcessPpuModeMessage(&AppMessage);
            break;
         }
         case PARAM_TYPE_EXIT_TEST_MODE_E:
         {
            ApplicationMessage_t AppMessage;
            AppMessage.MessageType = APP_MSG_TYPE_EXIT_TEST_MODE_E;
            result = MM_ApplicationProcessExitTestMessage(&AppMessage);
            break;
         }				 
         case PARAM_TYPE_DEPASSIVATION_SETTINGS_E:
            result = MM_CMD_ProcessDepassivationSettings(pCommand, source);
            break;
         default:
            result = ERR_OUT_OF_RANGE_E;
         break;
      }
   }
   
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessCIECommand
*  Description:      Process the supplied command message.  Provide an immediate 
*                    response if possible, otherwise send a message to the Head and
*                    store the command pending a response.
*  Param - pCommand: The command to be processed.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCIECommand ( const CIECommand_t* pCommand )
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   
   if ( pCommand )
   {
      CommandSource_t cmd_source = MM_CMD_ConvertFromATCommandSource(pCommand->PortNumber);
      
      if ( CMD_SOURCE_INVALID_E != cmd_source && CIE_CMD_MAX_E > pCommand->CommandType)
      {
         switch( pCommand->CommandType )
         {
            case CIE_CMD_READ_MESSAGE_QUEUE_STATUS_E:
               result = MM_CMD_ProcessReadCIEMessageQueueStatus( cmd_source );            
               break;
            case CIE_CMD_READ_ZONES_AND_DEVICES_E:
               result = MM_CMD_ProcessReadZonesAndDevices( cmd_source );            
               break;
            case CIE_CMD_READ_FIRE_MESSAGE_E:
               result = MM_CMD_ProcessReadCIEFireMessageQueue( cmd_source );             
               break;
            case CIE_CMD_READ_ALARM_MESSAGE_E:
               result = MM_CMD_ProcessReadCIEAlarmMessageQueue( cmd_source );             
               break;
            case CIE_CMD_READ_FAULT_MESSAGE_E:
               result = MM_CMD_ProcessReadCIEFaultMessageQueue( cmd_source );             
               break;
            case CIE_CMD_READ_MISC_MESSAGE_E:
               result = MM_CMD_ProcessReadCIEMiscMessageQueue( cmd_source );             
               break;
            case CIE_CMD_DISCARD_FIRE_MESSAGE_E:
               result = MM_CMD_ProcessDiscardFromCIEFireMessageQueue( cmd_source );             
               break;
            case CIE_CMD_DISCARD_ALARM_MESSAGE_E:
               result = MM_CMD_ProcessDiscardFromCIEAlarmMessageQueue( cmd_source );             
               break;
            case CIE_CMD_DISCARD_FAULT_MESSAGE_E:
               result = MM_CMD_ProcessDiscardFromCIEFaultMessageQueue( cmd_source );             
               break;
            case CIE_CMD_DISCARD_MISC_MESSAGE_E:
               result = MM_CMD_ProcessDiscardFromCIEMiscMessageQueue( cmd_source );             
               break;
            case CIE_CMD_RESET_SELECTED_QUEUE_E:
               result = MM_CMD_ProcessResetSelectedQueues( pCommand, cmd_source );             
               break;
            case CIE_CMD_READ_DEVICE_COMBINATION_E:
            {
               CO_CommandData_t cmd;
               cmd.CommandType = pCommand->CommandType;
               cmd.Destination = pCommand->NodeID;
               cmd.Parameter1 = pCommand->Parameter1;
               cmd.Parameter2 = pCommand->Parameter2;
               cmd.Source = 0;
               cmd.SourceUSART = cmd_source;
               cmd.Value = pCommand->Value;
               cmd.ReadWrite = pCommand->ReadWrite;
               result = MM_CMD_ProcessCommandDeviceCombination( &cmd, cmd_source );
            }
               break;
            case CIE_CMD_READ_DEVICE_STATUS_FLAGS_E:
               result = MM_CMD_ProcessReadDeviceStatusFlags(pCommand, cmd_source);
               break;
            case CIE_CMD_READ_DEVICE_MESH_STATUS_E:
               result = MM_CMD_ProcessReadStatusIndication( pCommand, cmd_source );
               break;
            case CIE_CMD_READ_DEVICE_SERIAL_NUMBER_E:
               result = MM_CMD_ProcessReadRBUSerialNumber( pCommand, cmd_source );
               break;
            case CIE_CMD_REBOOT_UNIT_E:
            {
               CO_CommandData_t cmdMessage;
               cmdMessage.CommandType = PARAM_TYPE_REBOOT_E;
               cmdMessage.Destination = pCommand->NodeID;
               cmdMessage.Source = gNetworkAddress;
               cmdMessage.Parameter1 = pCommand->Parameter1;
               cmdMessage.Parameter2 = pCommand->Zone;
               cmdMessage.ReadWrite = COMMAND_WRITE;
               cmdMessage.Source = cmd_source;
               cmdMessage.NumberToSend = CFG_GetNumberOfResends();
               
               CO_PRINT_A_2(DBG_INFO_E,"CIE_CMD_REBOOT_UNIT_E addr=%d, rw=%d\r\n", cmdMessage.Source = gNetworkAddress, cmdMessage.ReadWrite);
               
               if ( gNetworkAddress == cmdMessage.Destination )
               {
                  result = MM_CMD_ProcessReboot( &cmdMessage, cmd_source );
               }
               else 
               {
                  CO_Message_t* pRebootCommand = osPoolAlloc(AppPool);
                  if ( pRebootCommand )
                  {
                     pRebootCommand->Type = CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E;
                     memcpy(pRebootCommand->Payload.PhyDataReq.Data, &cmdMessage,sizeof(CO_CommandData_t));

                     osStatus osStat = osMessagePut(AppQ, (uint32_t)pRebootCommand, 0);
                     if (osOK != osStat)
                     {
                        /* failed to write */
                        osPoolFree(AppPool, pRebootCommand);
                        result = ERR_QUEUE_OVERFLOW_E;
                     }
                     else
                     {
                        result = SUCCESS_E;
                     }
                  }
                  
               }
            }
               break;
            case CIE_CMD_SET_TEST_MODE_E:
               result = MM_CMD_ProcessTestModeCommand( pCommand, cmd_source );
               break;
            case CIE_CMD_READ_ANALOGUE_VALUE_E:
               result = MM_CMD_ProcessReadAnalogueValueCommand( pCommand, cmd_source );
               break;
            case CIE_CMD_BEACON_FLASH_RATE_E:
               result = MM_CMD_ProcessBeaconFlashRate( pCommand, cmd_source );
               break;
            case CIE_CMD_DEVICE_ENABLE_E:
               result = MM_CMD_ProcessDeviceEnable( pCommand, cmd_source );
               break;
            case CIE_CMD_READ_PLUGIN_SERIAL_NUMBER_E:
               result = MM_CMD_ProcessPluginSerialNumberRequest( pCommand, cmd_source );
               break;
            case CIE_CMD_ENABLE_FAULTS_CLEAR_MSG_E:
               result = MM_CMD_ProcessEnableFaultsClearMessage( pCommand, cmd_source );
            break;
            case CIE_CMD_SOUNDER_TONE_SELECTION_E:
               result = MM_CMD_ProcessSounderToneSelection( pCommand, cmd_source );
               break;
            case CIE_CMD_UPPER_THRESHOLD_SETTING_E:
               result = MM_CMD_ProcessUpperThresholdSetting( pCommand, cmd_source );
               break;
            case CIE_CMD_LOWER_THRESHOLD_SETTING_E:
               result = MM_CMD_ProcessLowerThresholdSetting( pCommand, cmd_source );
               break;
            case CIE_CMD_PLUGIN_TEST_MODE_E:
               result = MM_CMD_ProcessPluginTestMode( pCommand, cmd_source );
               break;
            case CIE_CMD_READ_DEVICE_FIRMWARE_VERSION_E:
               result = MM_CMD_ProcessFirmwareInfoRequest( pCommand, cmd_source );
               break;
            case CIE_CMD_PLUGIN_FIRMWARE_VERSION_E:
               result = MM_CMD_ProcessPluginFirmwareInfoRequest( pCommand, cmd_source );
               break;
            case CIE_CMD_READ_PLUGIN_TYPE_AND_CLASS_E:
               result = MM_CMD_ProcessPluginTypeAndClassRequest( pCommand, cmd_source );
               break;
            case CIE_CMD_READ_DEVICE_NEIGHBOUR_INFORMATION_E:
               result = MM_CMD_ProcessReadNeighbourInformation( pCommand, cmd_source );
               break;
            case CIE_CMD_SET_DAY_NIGHT_E:
               result = MM_CMD_ProcessCIEDayNightSetting( pCommand, cmd_source );
               break;
            case CIE_CMD_PLUGIN_LED_SETTING_E:
               result = MM_CMD_ProcessPluginLEDSetting( pCommand, cmd_source );
               break;
            case CIE_CMD_ACKNOWLEDGE_E:
               result = MM_CMD_ProcessAlarmAcknowledge( pCommand, cmd_source );
               break;
            case CIE_CMD_EVACUATE_E:
               result = MM_CMD_ProcessAlarmEvacuate( pCommand, cmd_source );
               break;
            case CIE_CMD_GLOBAL_DELAYS_E:
               result = MM_CMD_ProcessAlarmGlobalDelays( pCommand, cmd_source );
               break;
            case CIE_CMD_ENABLE_DISABLE_E:
               result = MM_CMD_ProcessAlarmEnable( pCommand, cmd_source );
               break;
            case CIE_CMD_RESET_ALARMS_E:
               result = MM_CMD_ProcessAlarmReset( pCommand, cmd_source );
               break;
            case CIE_CMD_SOUND_LEVEL_E:
               result = MM_CMD_ProcessSoundLevel( pCommand, cmd_source );
               break;
            case CIE_CMD_TEST_ONE_SHOT_E:
               result = MM_CMD_ProcessTestOneShot( pCommand, cmd_source );
               break;
            case CIE_CMD_CONFIRM_E:
               result = MM_CMD_ProcessConfirm ( pCommand, cmd_source );
               break;
            case CIE_CMD_ALARM_CONFIG_E:
               result = MM_CMD_ProcessAlarmConfiguration( pCommand, cmd_source );
               break;
            case CIE_CMD_ALARM_OPTION_FLAGS_E:
               result = MM_CMD_ProcessAlarmOptionFlags( pCommand, cmd_source );
               break;
            case CIE_CMD_ALARM_DELAYS_E:
               result = MM_CMD_ProcessAlarmDelays( pCommand, cmd_source );
               break;
            case CIE_CMD_CHANNEL_FLAGS_E:
               result = MM_CMD_ProcessCIEChannelFlagsCommand( pCommand, cmd_source );
               break;
            case CIE_CMD_PRODUCT_CODE_E:
               result = MM_CMD_ProcessCIEProductCode( pCommand, cmd_source );
               break;
            case CIE_CMD_MAX_RANK_E:
               result = MM_CMD_ProcessMaxRankCommand( pCommand, cmd_source );
               break;
            case CIE_CMD_ENABLE_200_HOUR_TEST_E:
               result = MM_CMD_Process200HourTest( pCommand, cmd_source );
               break;
            case CIE_CMD_CHECK_FIRMWARE_E:
               result = MM_CMD_ProcessCheckFirmwareRequest( pCommand, cmd_source );
               break;
            case CIE_CMD_SET_LOCAL_OR_GLOBAL_ALARM_DELAYS_E:
               result = MM_CMD_ProcessLocalOrGlobalDelaySetting( pCommand, cmd_source );
               break;
            case CIE_CMD_SET_GLOBAL_ALARM_DELAY_VALUES_E:
               result = MM_CMD_ProcessGlobalDelayValues( pCommand, cmd_source );
               break;
            case CIE_CMD_SET_GLOBAL_ALARM_DELAY_OVERRIDE_E:
               result = MM_CMD_ProcessGlobalDelayOverride( pCommand, cmd_source );
               break;
            case CIE_CMD_REQUEST_BATTERY_STATUS_E:
               result = MM_CMD_ProcessBatteryStatusRequest( pCommand, cmd_source );
               break;
            case CIE_CMD_SET_GLOBAL_DELAY_COMBINED_E:
               result = MM_CMD_ProcessGlobalDelayCombined( pCommand, cmd_source );
               break;
            case CIE_CMD_SCAN_FOR_DEVICES_E:
               result = MM_CMD_ProcessScanForDevices( pCommand, cmd_source );
               break;
            case CIE_CMD_REQUEST_SENSOR_VALUES_E:
               result = MM_CMD_ProcessRequestSensorvalues( pCommand, cmd_source );
               break;
            case CIE_CMD_DELAYED_OUTPUT_E:
               result = MM_CMD_ProcessDelayedOutput( pCommand, cmd_source );
               break;
            case CIE_CMD_BATTERY_TEST_E:
               result = MM_CMD_ProcessBatteryTestCommand( pCommand, cmd_source );
               break;
            case CIE_CMD_PPU_MODE_ENABLE_E:
               result = MM_CMD_ProcessPPUModeCommand( pCommand, cmd_source );
               break;
            case CIE_CMD_ENTER_PPU_MODE_E:
               result = MM_CMD_ProcessEnterPPUModeCommand( pCommand, cmd_source );
               break;
            case CIE_CMD_EXIT_TEST_MODE_E:
               result = MM_CMD_ProcessExitTestModeCommand( pCommand, cmd_source );
               break;						
            case CIE_CMD_DEPASSIVATION_SETTINGS_E:
               result = MM_CMD_ProcessCIEDepassivationSettings( pCommand, cmd_source );
               break;
            default:
               result = ERR_MESSAGE_TYPE_UNKNOWN_E;
               break;
         }
      }
   }
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessHeadMessage
*  Description:      Process the supplied message from the sensor head.
*  Param - pCommand: The command to be processed.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessHeadMessage ( const HeadMessage_t* pMessage )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   if ( pMessage )
   {
      CO_PRINT_B_3(DBG_INFO_E,"MM_CMD_ProcessHeadMessage 1 : cip=%d, p-Tid=%d, c-Tid=%d\r\n", gCommandInProgress, pMessage->TransactionID, gCurrentCommand.TransactionID);
      if ( (HEAD_WRITE_OUTPUT_ACTIVATED_E == pMessage->MessageType) || 
           (CMD_SOURCE_APPLICATION_E == gCommandSource) ||
           (HEAD_INTERNAL_MESSAGE_E == pMessage->MessageType) ||
           (HEAD_WRITE_ENABLED_E == pMessage->MessageType) )
      {
         // no response expected
         result = SUCCESS_E;
      }
      /* check that it's the response that we were expecting */
      else if ( gCommandInProgress &&
           pMessage->TransactionID == gCurrentCommand.TransactionID )
      {
         /* it is the expected reponse.  Copy the value and send the response to the originator of the command*/
         CO_ResponseData_t responseData;
         responseData.CommandType = gCurrentCommand.CommandType;
         responseData.Destination = gCurrentCommand.Destination;
         responseData.Source = gCurrentCommand.Source;
         responseData.Parameter1 = gCurrentCommand.Parameter1;
         responseData.Parameter2 = gCurrentCommand.Parameter2;
         responseData.ReadWrite = gCurrentCommand.ReadWrite;
         responseData.TransactionID = pMessage->TransactionID;
         responseData.Value = pMessage->Value;

         if( CMD_SOURCE_MESH_E != gCommandSource && CMD_SOURCE_INVALID_E != gCommandSource)
         {
            /* the command came from the USART AT link.  Send the response there */
            if ( MM_CMD_SendATResponse( &responseData, gCommandSource ) )
            {
               result = SUCCESS_E;
            }
            else
            {
               CO_PRINT_B_0(DBG_INFO_E,"MM_CMD_ProcessHeadMessage FAIL 1\r\n");
               result = ERR_MESSAGE_FAIL_E;
               MM_CMD_SendATMessage(AT_RESPONSE_FAIL, gCommandSource);
            }
         }
         else
         {
            /* send the response over the Mesh */
            bool msg_sent = MM_MeshAPIResponseReq( responseData.TransactionID, responseData.Source, responseData.Destination, responseData.CommandType,
                                       responseData.Parameter1, responseData.Parameter2, responseData.Value, responseData.ReadWrite);
            if ( msg_sent )
            {
               result = SUCCESS_E;
            }
         }
      }
      else
      {
         CO_PRINT_B_0(DBG_INFO_E,"MM_CMD_ProcessHeadMessage FAIL 2\r\n");
         result = ERR_MESSAGE_FAIL_E;
      }
      
      gCommandInProgress = false;
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessMeshResponse
*  Description:      Process the supplied response from the Mesh.
*  Param - Handle:      The transaction ID of the original command.
*  Param - Destination: The node ID of the node that the command was sent to.
*  Param - CommandType: The command type.  See ParameterType_t.
*  Param - Parameter1:  Command parameter 1.
*  Param - Parameter2:  Command parameter 2.
*  Param - Value:       The value returned by the command.
*  Param - ReadWrite:   Read or write command.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessMeshResponse ( const uint8_t Handle, const uint16_t Destination, const uint8_t CommandType,
                           const uint8_t Parameter1, const uint8_t Parameter2, const uint32_t Value, const uint8_t ReadWrite )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   

   /* check that it's the response that we were expecting */
   if ( gCommandInProgress && Handle == gCurrentCommand.TransactionID )
   {
      CO_ResponseData_t responseData;
      responseData.CommandType = CommandType;
      responseData.Source = gNetworkAddress;
      responseData.Destination = Destination;
      responseData.Parameter1 = Parameter1;
      responseData.Parameter2 = Parameter2;
      responseData.ReadWrite = ReadWrite;
      responseData.TransactionID = Handle;
      responseData.Value = Value;

      /* it is the expected reponse.  Copy the value and send the response to the originator of the command*/
      if( CMD_SOURCE_MESH_E != gCommandSource && CMD_SOURCE_INVALID_E != gCommandSource)
      {
         /* the command came from the USART AT link.  Send the response there */
         if ( MM_CMD_SendATResponse( &responseData, gCommandSource ) )
         {
            result = SUCCESS_E;
         }
         else
         {
            result = ERR_MESSAGE_FAIL_E;
         }
      }
      else
      {
         /* send the response over the Mesh */
         bool msg_sent = MM_MeshAPIResponseReq( responseData.TransactionID, responseData.Destination, responseData.Source, responseData.CommandType,
                                    responseData.Parameter1, responseData.Parameter2, responseData.Value, responseData.ReadWrite);
         if ( msg_sent )
         {
            result = SUCCESS_E;
         }
      }
   }
   else
   {
      result = ERR_MESSAGE_FAIL_E;
   }
   
   gCommandInProgress = false;

   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessCommandDeviceCombination
*  Description:      Process the supplied message for the RBU/NCU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCommandDeviceCombination ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
      CO_ResponseData_t response = {0};
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         /* respond over AT USART link */
         result = DM_NVMRead(NV_DEVICE_COMBINATION_E, &response.Value, sizeof(uint32_t));
         
         /* Read deviceCombination and send it back to the command's originator */;
         /* Read value and send it back to the command's originator */;
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */
            if ( MM_CMD_SendATResponse( &response, source ) )
            {
               result = SUCCESS_E;
            }
            else
            {
               result = ERR_MESSAGE_FAIL_E;
               MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
            }
         }
         else
         {
            /* respond over Mesh link */
            bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
            if ( msg_sent )
            {
               result = SUCCESS_E;
            }
         }
      }
      else
      {
         CO_ASSERT_RET(DC_NUMBER_OF_DEVICE_COMBINATIONS_E > pCommand->Value, ERR_OUT_OF_RANGE_E);
         uint32_t value = pCommand->Value;
         result = DM_NVMWrite(NV_DEVICE_COMBINATION_E, &value, sizeof(uint32_t));

         if ( SUCCESS_E == result )
         {
            response.Value = value;
            /* Send the value back to the command's originator */;
            if( CMD_SOURCE_MESH_E == source )
            {
               /* respond over Mesh link */
               bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                          response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
               if ( msg_sent )
               {
                  result = SUCCESS_E;
               }
            }
            else if ( CMD_SOURCE_INVALID_E != source )
            {
               /* respond over AT USART link */
               if (MM_CMD_SendATMessage( AT_RESPONSE_OK, source ) )
               {
                  result = SUCCESS_E;
               }
               else
               {
                  result = ERR_MESSAGE_FAIL_E;
                  MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
               }
            }
         }
      }
   }
   
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessCommandTxPowerLevel
*  Description:      Process the command to read/write the Tx Power Level Low/High in NVM.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCommandTxPowerLevel ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
      CO_ResponseData_t response;
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         /* Read the Tx Power from NVM */
         
         if ( PARAM_TYPE_TX_POWER_LOW_E == pCommand->CommandType)
         {
            result = DM_NVMRead(NV_TX_POWER_LOW_E, &response.Value, sizeof(uint32_t));
         }
         else if ( PARAM_TYPE_TX_POWER_HIGH_E == pCommand->CommandType)
         {
            result = DM_NVMRead(NV_TX_POWER_HIGH_E, &response.Value, sizeof(uint32_t));
         }
         else 
         {
            result = ERR_OUT_OF_RANGE_E;
         }

         if ( SUCCESS_E == result )
         {
            /* Send the value back to the command's originator */;
            if( CMD_SOURCE_MESH_E == source )
            {
               /* respond over Mesh link */
               bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                          response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
               if ( msg_sent )
               {
                  result = SUCCESS_E;
               }
            }
            else if ( CMD_SOURCE_INVALID_E != source )
            {
               /* respond over AT USART link */
               if ( MM_CMD_SendATResponse( &response, source ) )
               {
                  result = SUCCESS_E;
               }
               else
               {
                  result = ERR_MESSAGE_FAIL_E;
                  MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
               }
            }
         }
      }
      else
      {
         /* Write the Tx Power to NVM */
         uint32_t tx_power_value = pCommand->Value;
         if ( PARAM_TYPE_TX_POWER_LOW_E == pCommand->CommandType)
         {
            result = DM_NVMWrite(NV_TX_POWER_LOW_E, &tx_power_value, sizeof(uint32_t));
         }
         else if ( PARAM_TYPE_TX_POWER_HIGH_E == pCommand->CommandType)
         {
            result = DM_NVMWrite(NV_TX_POWER_HIGH_E, &tx_power_value, sizeof(uint32_t));
         }
         else 
         {
            response.Value = ERROR_VALUE; /* Bad Value to indicate failure when responding over the Mesh */
            result = ERR_OUT_OF_RANGE_E;
         }

         if ( SUCCESS_E == result )
         {
            /* Send the value back to the command's originator */;
            if( CMD_SOURCE_MESH_E == source )
            {
               /* respond over Mesh link */
               bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                          response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
               if ( msg_sent )
               {
                  result = SUCCESS_E;
               }
            }
            else if ( CMD_SOURCE_INVALID_E != source )
            {
               /* respond over AT USART link */
               if (MM_CMD_SendATMessage(AT_RESPONSE_OK, source ) )
               {
                  result = SUCCESS_E;
               }
               else
               {
                  result = ERR_MESSAGE_FAIL_E;
                  MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
               }
            }
         }
      }
   }
   
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessCommandAnalogueValue
*  Description:      Process the Analogue Value command for the RBU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCommandAnalogueValue ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   CO_PRINT_B_0(DBG_INFO_E,"MM_CMD_ProcessCommandAnalogueValue+\r\n");
   if ( pCommand )
   {
      /* create a response */
      CO_ResponseData_t response;
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         CO_PRINT_B_0(DBG_INFO_E,"11\r\n");
         InputState_t state_channel_1;
         InputState_t state_channel_2;
         result = DM_RC_ReadInputs(&state_channel_1, &state_channel_2);
         
         if ( CO_CHANNEL_MAX_E > pCommand->Parameter1 )
         {
            CO_PRINT_B_0(DBG_INFO_E,"11\r\n");
            /* Read the requested value and send it back to the command's originator */;
            if ( CO_CHANNEL_INPUT_1_E == (CO_ChannelIndex_t)pCommand->Parameter1 )
            {
               if ( SUCCESS_E == result )
               {
                  if ( INPUT_STATE_RESET_E == state_channel_1 )
                  {
                     response.Value = 0;
                  }
                  else if ( INPUT_STATE_SET_E == state_channel_1 )
                  {
                     response.Value = 1;
                  }
                  else 
                  {
                     response.Value = 0xFFFFFFFFU;
                  }
               }
               else 
               {
                  response.Value = 0xFFFFFFFFU;
               }
            }
            else if ( CO_CHANNEL_INPUT_2_E == (CO_ChannelIndex_t)pCommand->Parameter1 )
            {
               if ( SUCCESS_E == result )
               {
                  if ( INPUT_STATE_RESET_E == state_channel_2 )
                  {
                     response.Value = 0;
                  }
                  else if ( INPUT_STATE_SET_E == state_channel_2 )
                  {
                     response.Value = 1;
                  }
                  else 
                  {
                     response.Value = 0xFFFFFFFFU;
                  }
               }
               else 
               {
                  response.Value = 0xFFFFFFFFU;
               }
            }
            else if ( CO_CHANNEL_OUTPUT_1_E == (CO_ChannelIndex_t)pCommand->Parameter1 )
            {
               RelayState_t state = DM_RC_ReadOutputState(CO_CHANNEL_OUTPUT_1_E);
               if ( RELAY_STATE_OPEN_E == state )
               {
                  response.Value = 0;
               }
               else if ( RELAY_STATE_CLOSED_E == state )
               {
                  response.Value = 1;
               }
               else 
               {
                  response.Value = 0xFFFFFFFFU;
               }
            }
            else if ( CO_CHANNEL_OUTPUT_2_E == (CO_ChannelIndex_t)pCommand->Parameter1 )
            {
               RelayState_t state = DM_RC_ReadOutputState(CO_CHANNEL_OUTPUT_2_E);
               if ( RELAY_STATE_OPEN_E == state )
               {
                  response.Value = 0;
               }
               else if ( RELAY_STATE_CLOSED_E == state )
               {
                  response.Value = 1;
               }
               else 
               {
                  response.Value = 0xFFFFFFFFU;
               }
            }
         }
         
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */
            MM_CMD_SendATResponse( &response, source );
            result = SUCCESS_E;
         }
         else
         {
            /* respond over Mesh link */
            bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
            if ( msg_sent )
            {
               result = SUCCESS_E;
            }
         }
      }
      else
      {
         /* WRITE is not supported */
         result = ERR_MESSAGE_FAIL_E;
         
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */
            MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
         }
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessCommandNeighbourInformation
*  Description:      Process the neighbour information command for the RBU/NCU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCommandNeighbourInformation ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_MESSAGE_FAIL_E;

   if (pCommand)
   {
      if (COMMAND_READ == pCommand->ReadWrite)
      {
         /* The requested data is held by the session manager in the Mesh Task.  Pass it down the Mesh */
         gCurrentCommand.CommandType = pCommand->CommandType;
         gCurrentCommand.Source = pCommand->Source;
         gCurrentCommand.Destination = pCommand->Destination;
         gCurrentCommand.Parameter1 = pCommand->Parameter1;
         gCurrentCommand.Parameter2 = pCommand->Parameter2;
         gCurrentCommand.ReadWrite = pCommand->ReadWrite;
         gCurrentCommand.TransactionID = pCommand->TransactionID;
         gCurrentCommand.Value = pCommand->Value;
         gCommandInProgress = true;
         gCommandSource = source;
         bool msg_sent = MM_MeshAPICommandReq(gCurrentCommand.TransactionID, gCurrentCommand.Source, gCurrentCommand.Destination, gCurrentCommand.CommandType,
            gCurrentCommand.Parameter1, gCurrentCommand.Parameter2, gCurrentCommand.Value, gCurrentCommand.ReadWrite, 1);
         if (msg_sent)
         {
            result = SUCCESS_E;
         }
      }
   }

   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessCommandStatusFlags
*  Description:      Process the Status Flags command for the RBU/NCU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCommandStatusFlags ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
      /* create a response */
      CO_ResponseData_t response;
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         uint32_t fault_status_flags = MM_GetFaultStatusFlags();
         response.Value = fault_status_flags;
         
         /* Read battery level and send it back to the command's originator */;
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */
            MM_CMD_SendATResponse( &response, source );
            result = SUCCESS_E;
         }
         else
         {
            /* respond over Mesh link */
            bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
            if ( msg_sent )
            {
               result = SUCCESS_E;
            }
         }
      }
      else
      {
         /* WRITE is not supported */
         result = ERR_MESSAGE_FAIL_E;
         
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */
            MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
         }
      }
   }
   
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessCommandRBUSerialNumber
*  Description:      Process the serial number command for the RBU/NCU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCommandRBUSerialNumber ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
      /* create a response */
      CO_ResponseData_t response = {0};
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         result = DM_NVMRead(NV_UNIT_SERIAL_NO_E, &response.Value, sizeof(uint32_t));
         
         /* Read value and send it back to the command's originator */;
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */
            MM_CMD_SendATResponse( &response, source );
            result = SUCCESS_E;
         }
         else
         {
            /* respond over Mesh link */
            bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
            if ( msg_sent )
            {
               result = SUCCESS_E;
            }
         }
      }
      else
      {
         uint32_t value = pCommand->Value;
         result = DM_NVMWrite(NV_UNIT_SERIAL_NO_E, &value, sizeof(uint32_t));

         if ( SUCCESS_E == result )
         {
            response.Value = value;
            /* Send the value back to the command's originator */;
            if( CMD_SOURCE_MESH_E == source )
            {
               /* respond over Mesh link */
               bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                          response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
               if ( msg_sent )
               {
                  result = SUCCESS_E;
               }
            }
            else if ( CMD_SOURCE_INVALID_E != source )
            {
               /* respond over AT USART link */
               if (MM_CMD_SendATMessage(AT_RESPONSE_OK, source ) )
               {
                  result = SUCCESS_E;
               }
               else
               {
                  result = ERR_MESSAGE_FAIL_E;
                  MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
               }
            }
         }
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessCommandRBUEnable
*  Description:      Process the RBU Enable command for the RBU/NCU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCommandRBUEnable ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
      /* create a response */
      CO_ResponseData_t response;
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         response.Value = 0;
         /* if we were previously disabled, the unit will be in SLEEP test mode. */
         MC_MAC_TestMode_t testmode = MC_GetTestMode();
         if ( MC_MAC_TEST_MODE_SLEEP_E == testmode )
         {
            response.Value = 1;
         }
         /* Read value and send it back to the command's originator.
            Don't try to send a Mesh response if we are in TEST mode because the MAC isn't running */
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */            
            MM_CMD_SendATResponse( &response, source );
         }
         else if ( MC_MAC_TEST_MODE_OFF_E == testmode )
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
         result = SUCCESS_E;
      }
      else
      {
         /* Write command.  We disable the RBU by putting it into the SLEEP test mode. */
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
         }
         else
         {
            /* We don't send a remote acknowledgement because we will have dropped out of the 
             * mesh before it can be scheduled 
             */
         }
         
         /* put the unit to sleep */
         MC_STATE_SetDeviceState(STATE_TEST_MODE_E);
         MC_SetTestMode( MC_MAC_TEST_MODE_SLEEP_E );
         result = SUCCESS_E;
      }
   }
   
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessModulationBandwidth
*  Description:      Process the Modulation Bandwitch command for the RBU/NCU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessModulationBandwidth ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
      /* create a command for the MAC */
      CO_ResponseData_t response = { 0 };
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         response.Value = MC_MAC_LoraParameters.LoraModulationBandwidth;
         /* Send the response back to the command's originator */;
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */            
            MM_CMD_SendATResponse( &response, source );
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
         result = SUCCESS_E;
      }
      else
      {
         MC_MAC_LoraParameters.LoraModulationBandwidth = pCommand->Value;

         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
         result = SUCCESS_E;
      }
   }
   
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessSpreadingFactor
*  Description:      Process the Spreading Factor command for the RBU/NCU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessSpreadingFactor ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
      /* create a command for the MAC */
      CO_ResponseData_t response = { 0 };
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         response.Value = MC_MAC_LoraParameters.LoraSpreadingFactor;

         /* Send the response back to the command's originator */;
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */            
            MM_CMD_SendATResponse( &response, source );
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
         result = SUCCESS_E;
      }
      else
      {
         MC_MAC_LoraParameters.LoraSpreadingFactor = pCommand->Value;

         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
         result = SUCCESS_E;
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessCodingRate
*  Description:      Process the Coding Rate command for the RBU/NCU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCodingRate ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
      /* create a command for the MAC */
      CO_ResponseData_t response = { 0 };
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         response.Value = MC_MAC_LoraParameters.LoraCodingRate;

         /* Send the response back to the command's originator */;
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */            
            MM_CMD_SendATResponse( &response, source );
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
         result = SUCCESS_E;
      }
      else
      {
         MC_MAC_LoraParameters.LoraCodingRate = pCommand->Value;

         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
         result = SUCCESS_E;
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessFrequency
*  Description:      Process the Frequency command for the RBU/NCU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessFrequency ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
      /* create a command for the MAC */
      CO_ResponseData_t response = { 0 };
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         response.Value = MC_MAC_LoraParameters.CurrentFrequencyChannelIdx;

         /* Send the response back to the command's originator */;
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */            
            MM_CMD_SendATResponse( &response, source );
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
         result = SUCCESS_E;
      }
      else
      {
         MC_MAC_LoraParameters.CurrentFrequencyChannelIdx = pCommand->Value;

         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
         result = SUCCESS_E;
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessTxPower
*  Description:      Process the Tx Power command for the RBU/NCU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessTxPower ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
      /* create a command for the MAC */
      CO_ResponseData_t response = { 0 };
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         response.Value = MC_MAC_LoraParameters.LoraLowTxPower;

         /* Send the response back to the command's originator */;
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */            
            MM_CMD_SendATResponse( &response, source );
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
         result = SUCCESS_E;
      }
      else
      {
         MC_MAC_LoraParameters.LoraLowTxPower = pCommand->Value;

         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
         result = SUCCESS_E;
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessRBUTestMode
*  Description:      Process the RBU Test Mode command for the RBU/NCU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessRBUTestMode ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   MC_MAC_TestMode_t testMode;
   
   if ( pCommand )
   {
      /* create a command for the MAC */
      CO_ResponseData_t response = { 0 };
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         response.Value = (uint32_t)MC_GetTestMode();

         /* Send the response back to the command's originator */;
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */            
            MM_CMD_SendATResponse( &response, source );
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
         result = SUCCESS_E;
      }
      else
      {
         switch(pCommand->Value)
         {
            case MC_MAC_TEST_MODE_RECEIVE_E:
               testMode = MC_MAC_TEST_MODE_RECEIVE_E;
               MC_STATE_SetDeviceState(STATE_TEST_MODE_E);
            break;
            case MC_MAC_TEST_MODE_TRANSPARENT_E:
               testMode = MC_MAC_TEST_MODE_TRANSPARENT_E;
               MC_STATE_SetDeviceState(STATE_TEST_MODE_E);
            break;
            case MC_MAC_TEST_MODE_TRANSMIT_E:
               testMode = MC_MAC_TEST_MODE_TRANSMIT_E;
               MC_STATE_SetDeviceState(STATE_TEST_MODE_E);
            break;
            case MC_MAC_TEST_MODE_MONITORING_E:
               testMode = MC_MAC_TEST_MODE_MONITORING_E;
               MC_STATE_SetDeviceState(STATE_TEST_MODE_E);
            break;
            case MC_MAC_TEST_MODE_SLEEP_E:
               testMode = MC_MAC_TEST_MODE_SLEEP_E;
               MC_STATE_SetDeviceState(STATE_TEST_MODE_E);
               //MM_ConfigureForTestModeSleep();
            break;
            case MC_MAC_TEST_MODE_NETWORK_MONITOR_E:
               testMode = MC_MAC_TEST_MODE_NETWORK_MONITOR_E;
               MC_STATE_SetDeviceState(STATE_TEST_MODE_E);
            break;
            case MC_MAC_TEST_MODE_OFF_E: // intentional drop-through
            default:
               Error_Handler("Rx'd cmd to leave test mode");
            break;
         }

         MC_SetTestMode( testMode );
         //this couldn't be done before calling MC_SetTestMode or the gpio will be disabled for the radio SPI
         if ( MC_MAC_TEST_MODE_SLEEP_E == testMode)
         {
            MM_ConfigureForTestModeSleep();
         }

         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
         result = SUCCESS_E;
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessFirmwareInformation
*  Description:      Process the Firmware Information command for the RBU/NCU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessFirmwareInformation ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   char error_response[AT_RESPONSE_MAX_LENGTH];
   
   if ( pCommand )
   {
      /* create a command for the MAC */
      CO_ResponseData_t response;
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      
      /* we only support READ for this command */
      if ( COMMAND_READ == pCommand->ReadWrite )
      {         
         switch ( pCommand->Parameter1 )
      {
            case FI_RBU_MAIN_IMAGE_E:
         response.Value = gSoftwareVersionNumber;
               result = SUCCESS_E;
               break;
            case FI_RBU_BACKUP_IMAGE_E:
               response.Value = gBackupSoftwareVersionNumber;
               result = SUCCESS_E;
               break;
            case FI_EXTERNAL_DEVICE_1_E:/* Head */
            {
               CO_CommandData_t command;
               command.CommandType = PARAM_TYPE_FIRMWARE_INFO_E;
               command.Source = pCommand->Source;
               command.Destination = pCommand->Destination;
               command.Parameter1 = pCommand->Parameter1;
               command.Parameter2 = pCommand->Parameter2;
               command.ReadWrite = pCommand->ReadWrite;
               command.TransactionID = pCommand->TransactionID;
               command.Value = pCommand->Value;
               
               result = MM_CMD_ProcessHeadCommand(&command, source);
            }
               break;
            case FI_INTERNAL_DEVICE_1_E:/* I2C */
            {
               uint32_t version = 0;
               result = DM_SVI_ReadFirmwareVersion( &version );
               if ( SUCCESS_E == result )
               {
                  response.Value = version;
               }
               else 
               {
                  response.Value = 0;
               }
            }
               break;
            case FI_EXTERNAL_DEVICE_2_E:/* reserved */ /* intentional drop-through */
            case FI_EXTERNAL_DEVICE_3_E:
            case FI_INTERNAL_DEVICE_2_E:
            case FI_INTERNAL_DEVICE_3_E:
               response.Value = 0;
               result = SUCCESS_E;
               break;
            default:
               result = ERR_OUT_OF_RANGE_E;
               snprintf(error_response, AT_RESPONSE_MAX_LENGTH,"ERROR,BAD_PARAM");
               response.Value = 0;
               break;
         }

         /* Send the response back to the command's originator */;
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            if ( SUCCESS_E == result )
            {
            /* respond over AT USART link */            
            MM_CMD_SendATResponse( &response, source );
         }
            else 
            {
               MM_CMD_SendATMessage(error_response, source);
            }
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
         result = SUCCESS_E;
      }
      else
      {
         result = ERR_MESSAGE_FAIL_E;
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessFirmwareActiveImage
*  Description:      Process the Firmware Active Image command for the RBU/NCU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessFirmwareActiveImage ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
      /* create a command for the MAC */
      CO_ResponseData_t response = { 0 };
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         response.Value = 1;

         /* Send the response back to the command's originator */;
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */            
            MM_CMD_SendATResponse( &response, source );
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
         result = SUCCESS_E;
      }
//      else
//      {
//         BLF_set_boot_bank(pCommand->Value, true);

//         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
//         {
//            MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
//         }
//         else
//         {
//            /* respond over Mesh link */
//            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
//                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
//         }
//         result = SUCCESS_E;
//      }
   }
   
   return result;
}
/*****************************************************************************
*  Function:   MM_CMD_ProcessReboot
*  Description:      Process the Reboot command for the RBU/NCU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessReboot ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
   CO_PRINT_A_2(DBG_INFO_E,"Received REBOOT command addr=%d, rw=%d\r\n", pCommand->Destination, pCommand->ReadWrite);
      if ( COMMAND_WRITE == pCommand->ReadWrite )
      {
         if ( (gNetworkAddress == pCommand->Destination) ||
              ((ADDRESS_GLOBAL == pCommand->Destination) && (BASE_NCU_E != gBaseType)) )
         {
            CO_PRINT_A_0(DBG_INFO_E,"Received REBOOT command\r\n");
            //Give messages time to clear
            osDelay(10000);
            /* Wait for any pending EEPROM write operation */
            FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

            /* Request a micro reset */
            Error_Handler("Received REBOOT command");
            result = SUCCESS_E;
         }
      }
      else // Write
      {
         MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
         result = ERR_MESSAGE_FAIL_E;
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessSVIComand
*  Description:      Process a command for the SVI.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessSVIComand ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   uint8_t svi_command_type = pCommand->Parameter1;
   uint8_t svi_response = 0;
   
   if ( pCommand )
   {
      /* create a response */
      CO_ResponseData_t response = { 0 };
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
//      if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SOUNDER_85DBA, DC_MATCH_ANY_E) )
//      {
//         //Enable the I2C to the sounder board via the sounder/beacon power line
//         GpioWrite(&SndrBeaconOn, 1);
//         osDelay(10);
//      }
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         response.Value = 0;
         
         if ( DM_SVI_POWER_LOCK_E == svi_command_type )
         {
            if ( CFG_GetSviPowerLock() )
            {
               response.Value = 1;
            }
            /* Send the result it back to the command's originator. */
            if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source )
            {
               /* respond over AT USART link */            
               MM_CMD_SendATResponse( &response, source );
            }
            else
            {
               /* respond over Mesh link */
               MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                          response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
            }
            result = SUCCESS_E;
         }
         else 
         {
            uint8_t registerAddress = 0;
            result = DM_SVI_GetRegister( svi_command_type, &registerAddress );
            if ( SUCCESS_E == result )
            {
               result = DM_SVI_ReadRegister( registerAddress, &svi_response );
               if ( SUCCESS_E == result )
               {
                  response.Value = svi_response;
                  /* Send the result it back to the command's originator. */
                  if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source )
                  {
                     /* respond over AT USART link */            
                     MM_CMD_SendATResponse( &response, source );
                  }
                  else
                  {
                     /* respond over Mesh link */
                     MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                                response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
                  }
                  result = SUCCESS_E;
               }
            }
         }
      }
      else
      {
         /* Write command. */
         if ( DM_SVI_POWER_LOCK_E == pCommand->Parameter1 )
         {
            bool lock = false;
            if ( 0 != pCommand->Value )
            {
               lock = true;
            }
            CFG_SetSviPowerLock(lock);
            if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source )
            {
               /* respond over AT USART link */
               MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
            }
            else
            {
               /* respond over Mesh link */
               MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                          response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
            }
            result = SUCCESS_E;
         }
         else 
         {
            uint8_t svi_command_type = pCommand->Parameter1;
            uint8_t registerAddress;         
            result = DM_SVI_GetRegister( svi_command_type, &registerAddress );
            if ( SUCCESS_E == result )
            {
               result = DM_SVI_WriteRegister( registerAddress, pCommand->Value );
               /* Send the result it back to the command's originator. */
               if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source )
               {
                  /* respond over AT USART link */
                  if ( SUCCESS_E == result )
                  {
                     MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
                  }
                  else 
                  {
                     MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
                  }
               }
               else
               {
                  /* respond over Mesh link */
                  MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                             response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
               }
            }
            else
            {
               if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
               {
                  MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
               }
               else
               {
                  response.Value = ERROR_VALUE;
                  /* respond over Mesh link */
                  MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                             response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
               }
            }
         }
      }
      
//      if ( 0 == pCommand->Value && DM_DeviceIsEnabled(gDeviceCombination, DEV_SOUNDER_85DBA, DC_MATCH_ANY_E) )
//      {
//         //Enable the I2C to the sounder board via the sounder/beacon power line
//         GpioWrite(&SndrBeaconOn, 0);
//      }

   }
   
   return result;
}


/***************************************************************************
*  Function:   MM_CMD_SendATResponse
*  Description:      Build and send an AT string in response to a Command message.
*  Param - pCommand: The command that we are responding to.
*  Param - cmd_source: The ID of the usart to send the message to.
*  Returns:          True if the response was set, false otherwise.
*  Notes:            
*****************************************************************************/
bool MM_CMD_SendATResponse(const CO_ResponseData_t* pResponse, const CommandSource_t cmd_source)
{
   bool result = false;
   
   if ( pResponse )
   {
      char response[AT_RESPONSE_MAX_LENGTH] = {0};
      if ( PARAM_TYPE_FIRMWARE_INFO_E == pResponse->CommandType )
      {
         if ( FINDX_EXTERNAL_INTERFACE_1_HEAD_E == pResponse->Parameter1 )
         {
            MM_HeadDecodeFirmwareVersion(pResponse->Value, (uint8_t*)&response, AT_RESPONSE_MAX_LENGTH);
         }
         else if ( FINDX_RBU_MAIN_IMAGE_E == pResponse->Parameter1 )
         {
            uint32_t base_version = pResponse->Value >> 14;
            uint32_t revision = (pResponse->Value >> 7) & 0x7f;
            uint32_t sub_revision = pResponse->Value & 0x7f;
            snprintf(response, CIE_QUEUE_DATA_SIZE, "FIR:Z%dU%d,0,0,1,%d.%d.%d\r\n",
                    neighbourArray[pResponse->Source].ZoneNumber, pResponse->Source,base_version,revision,sub_revision);

         }
      }
      else if ( PARAM_TYPE_PLUGIN_SERIAL_NUMBER_E == pResponse->CommandType )
      {
         char buffer[16];
         MM_ATDecodeSerialNumber(pResponse->Value, buffer);
         snprintf(response, CIE_QUEUE_DATA_SIZE, "%d,%s\r\n", pResponse->Destination, buffer);
         //CO_PRINT_B_1(DBG_NOPREFIX_E,"SNP:%s\r\n", response);
      }
      else if ( PARAM_TYPE_PLUGIN_TYPE_AND_CLASS_E == pResponse->CommandType )
      {
         uint32_t head_type = pResponse->Value >> 16;
         uint32_t head_class = pResponse->Value & 0xFFFF;
         snprintf(response, CIE_QUEUE_DATA_SIZE, "%d,%d\r\n", head_type, head_class);
         //CO_PRINT_B_1(DBG_NOPREFIX_E,"PTYPE:%s\r\n", response);
      }
      else
      {
         sprintf(response, "%s:Z%dU%d,%d,%d,%d,%d,%d", ParamCommandMap[pResponse->CommandType],neighbourArray[pResponse->Source].ZoneNumber, pResponse->Source,
               pResponse->TransactionID, pResponse->Parameter1, pResponse->Parameter2, pResponse->ReadWrite, pResponse->Value);
         CO_PRINT_B_1(DBG_NOPREFIX_E,"%s\r\n", response);
      }

      result = MM_CMD_SendATMessage(response, cmd_source);
   }
   
   return result;
}


/*****************************************************************************
* Function:   MM_CMD_SendATMessage
* Description:       Put a string in the ATHandleQ for transmission over the serial link.
*
* Param - message:   The NULL terminated string to send.
* Param - cmd_source: The ID of the usart to send the message to.
* Returns:           True if the message was queued, false otherwise.
* Notes:            
*****************************************************************************/
bool MM_CMD_SendATMessage(char* pMessage, const CommandSource_t cmd_source)
{
   bool result = false;
   osStatus status;
   
   if ( pMessage )
   {
      Cmd_Reply_t *pCmdReply = ALLOC_ATHANDLE_POOL

      if (pCmdReply)
      {
         strcpy((char*)pCmdReply->data_buffer, pMessage);
         pCmdReply->length = (uint8_t)strlen(pMessage);
         pCmdReply->port_id = MM_CMD_ConvertToATCommandSource(cmd_source);
         status = osMessagePut(ATHandleQ, (uint32_t)pCmdReply, 0);
         if (osOK == status)
         {
            result = true;
         }
         else
         {
            CO_PRINT_B_1(DBG_INFO_E,"MM_CMD_SendATMessage Error %d\r\n", status);
            FREE_ATHANDLE_POOL(pCmdReply);
         }
      }
      else 
      {
         CO_PRINT_B_0(DBG_INFO_E,"MM_CMD_SendATMessage Failed to allocate mem\r\n");
      }
   }
   else 
   {
      CO_PRINT_B_0(DBG_INFO_E,"MM_CMD_SendATMessage Rxd NULL message\r\n");
   }
   return result;
}

/*****************************************************************************
* Function:   MM_CMD_SendHeadMessage
* Description:       Put a message in the HeadQ for the Head task.
*
* Param - messageType:  The message type.
* Param - pMmessage:    The message content.
* Param - source:       The source of the message, where the response should be sent.
*
* Returns:           True if the message was queued, false otherwise.
* Notes:            
*****************************************************************************/
bool MM_CMD_SendHeadMessage(const CO_MessageType_t messageType, HeadMessage_t* pMessage, const CommandSource_t source)
{
   bool result = false;
   if ( pMessage )
   {
      CO_Message_t* pPhyDataReq = osPoolAlloc(AppPool);
      if (pPhyDataReq)
      {
         pPhyDataReq->Type = messageType;
         uint8_t* pMsgData = pPhyDataReq->Payload.PhyDataReq.Data;
         memcpy(pMsgData, pMessage, sizeof(HeadMessage_t));
         pPhyDataReq->Payload.PhyDataReq.Size = sizeof(HeadMessage_t);

         osStatus osStat = osMessagePut(HeadQ, (uint32_t)pPhyDataReq, 0);      
         
         if (osOK != osStat)
         {
            /* failed to put message in the head queue */
            osPoolFree(AppPool, pPhyDataReq);
            result = false;
            CO_PRINT_A_1(DBG_ERROR_E,"FAILED to add message to Head Queue. Error=%d\r\n", osStat);
            if ( osStat == osErrorResource )
            {
               CO_PRINT_A_1(DBG_INFO_E,"Head Queue msg count=%d\r\n", gHeadQueueCount);
#ifndef USE_NEW_HEAD_INTERFACE
               osSemaphoreRelease(HeadInterfaceSemaphoreId);
#endif
            }
         }
         else
         {
            gHeadQueueCount++;
            /* Message was queued OK. Signal the head interface task that we have sent a message */
#ifndef USE_NEW_HEAD_INTERFACE
            osSemaphoreRelease(HeadInterfaceSemaphoreId);
#endif
            gCommandInProgress = true;
            gCommandSource = source;
            result = true;
         }
      }
   }
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessCommandZone
*  Description:      Process the Zone read/write command for the RBU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCommandZone ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
      /* create a response */
      CO_ResponseData_t response = { 0 };
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         response.Value = CFG_GetZoneNumber();
         
         /* Read value and send it back to the command's originator */;
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */
            MM_CMD_SendATResponse( &response, source );
            result = SUCCESS_E;
         }
         else
         {
            /* respond over Mesh link */
            bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
            if ( msg_sent )
            {
               result = SUCCESS_E;
            }
         }
      }
      else
      {
         result = CFG_SetZoneNumber(response.Value);
         
         if ( SUCCESS_E == result )
         {
            //gZoneNumber = response.Value;
            CFG_SetZoneNumber(response.Value);
         }

         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
            
            if ( SUCCESS_E == result )
            {
               CO_PRINT_B_1(DBG_INFO_E, "Local Command : Zone number changed to %d\r\n", response.Value);
            }
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
            
            if ( SUCCESS_E == result )
            {
               CO_PRINT_B_1(DBG_INFO_E, "Remote Command : Zone number changed to %d\r\n", response.Value);
            }
         }
         
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessMeshStatusRequest
*  Description:      Process request for a Mesh Status Report from the NCU.
*  
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessMeshStatusRequest( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   bool delay_sending = false;
   uint16_t overall_fault = MM_RBU_GetOverallFaults();
   
   if ( pCommand && CMD_SOURCE_MESH_E == source  )
   {
      //If the request was global we need to respond on the DULCH, if directly addressed to this node send on RACH-S
      if ( ADDRESS_GLOBAL == pCommand->Destination )
      {
         delay_sending = true;
      }
      
      if ( MM_MeshAPIGenerateStatusSignalReq( pCommand->TransactionID, CO_MESH_EVENT_NONE_E, gNetworkAddress, 0, overall_fault, delay_sending ) )
      {
         result = SUCCESS_E;
      }
      else 
      {
         result = ERR_MESSAGE_FAIL_E;
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessReadCIEMessageQueueStatus
*  Description:      Read the number of messages in the CIE queues and send a
*                    response message to the CIE.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessReadCIEMessageQueueStatus ( const CommandSource_t source )
{
   ErrorCode_t result = ERR_MESSAGE_FAIL_E;
   char response[64];
//   CO_PRINT_B_0(DBG_INFO_E,"NCU Application received CIE_CMD_READ_MESSAGE_QUEUE_STATUS_E\r\n");
   
   uint32_t fire_count = MM_CIEQ_MessageCount(CIE_Q_FIRE_E);
   uint32_t fire_lost_count = MM_CIEQ_LostMessageCount(CIE_Q_FIRE_E);
   uint32_t alarm_count = MM_CIEQ_MessageCount(CIE_Q_ALARM_E);
   uint32_t alarm_lost_count = MM_CIEQ_LostMessageCount(CIE_Q_ALARM_E);
   uint32_t fault_count = MM_CIEQ_MessageCount(CIE_Q_FAULT_E);
   uint32_t fault_lost_count = MM_CIEQ_LostMessageCount(CIE_Q_FAULT_E);
   uint32_t misc_count = MM_CIEQ_MessageCount(CIE_Q_MISC_E);
   uint32_t misc_lost_count = MM_CIEQ_LostMessageCount(CIE_Q_MISC_E);
   uint32_t tx_count = MM_CIEQ_MessageCount(CIE_Q_TX_BUFFER_E);
   
   snprintf(response,64,"%d,%d\n%d,%d\n%d,%d\n%d,%d\n%d", fire_count, fire_lost_count,
                  alarm_count, alarm_lost_count, fault_count, fault_lost_count, misc_count, misc_lost_count, tx_count);
  
   /* respond over AT USART link */
   if( MM_CMD_SendATMessage(response, source) )
   {
      result = SUCCESS_E;
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessReadZonesAndDevices
*  Description:      Read the number of zones and number of devices that have logged on.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessReadZonesAndDevices ( const CommandSource_t source )
{
   ErrorCode_t result = ERR_MESSAGE_FAIL_E;
   char response[CIE_QUEUE_DATA_SIZE];
   
   uint32_t rbu_count = 0;
   uint32_t zone_count = 0;
   
   for ( uint32_t index = 1; index < MAX_DEVICES_PER_SYSTEM; index++)
   {
      /* count the rbus */
      if ( CO_BAD_NODE_ID != neighbourArray[index].NodeID )
      {
         rbu_count++;
      }
      
      /* count the number of zones */
      if ( 0 != neighbourArray[index].ZoneNumber )
      {
         /* iterate the previous nodes looking for duplicate */
         bool duplicate = false;
         for ( uint32_t dup_index = 1; dup_index < index && !duplicate; dup_index++)
         {
            if ( neighbourArray[index].ZoneNumber == neighbourArray[dup_index].ZoneNumber )
            {
               duplicate = true;
            }
         }
         if ( !duplicate )
         {
            zone_count++;
         }
      }
   }
   
   snprintf(response,CIE_QUEUE_DATA_SIZE,"%d,%d", zone_count, rbu_count);
  
   /* respond over AT USART link */
   if( MM_CMD_SendATMessage(response, source) )
   {
      result = SUCCESS_E;
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessReadCIEFireMessageQueue
*  Description:      Read a message from the CIE Fire queue and send it to the CIE.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessReadCIEFireMessageQueue( const CommandSource_t source )
{
   ErrorCode_t result;
   CieBuffer_t buffer;
   char response[CIE_QUEUE_DATA_SIZE];
   
   result = MM_CIEQ_Pop(CIE_Q_FIRE_E, &buffer);
   
   if ( SUCCESS_E == result )
   {  
      snprintf(response,CIE_QUEUE_DATA_SIZE,"%s", (char*)&buffer);
     
      /* respond over AT USART link */
      if( MM_CMD_SendATMessage(response, source) )
      {
         result = SUCCESS_E;
         CO_PRINT_B_1(DBG_INFO_E,"<<QFE:%s\r\n", response);
      
#ifdef AUTO_DISCARD_CIE_QUEUE_MSG_AFTER_READ
         MM_CIEQ_Discard(CIE_Q_FIRE_E);
#endif
      }
   }
   else 
   {
      CO_PRINT_B_1(DBG_INFO_E,"<<QFE:ERROR %d\r\n", result);
      MM_CMD_SendATFailWithErrorCode(NULL, result, source);
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessReadCIEAlarmMessageQueue
*  Description:      Read a message from the CIE Alarm queue and send it to the CIE.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessReadCIEAlarmMessageQueue( const CommandSource_t source )
{
   ErrorCode_t result;
   CieBuffer_t buffer;
   char response[CIE_QUEUE_DATA_SIZE];
   
   result = MM_CIEQ_Pop(CIE_Q_ALARM_E, &buffer);
   
   if ( SUCCESS_E == result )
   {  
      snprintf(response,CIE_QUEUE_DATA_SIZE,"%s", (char*)&buffer);
     
      /* respond over AT USART link */
      if( MM_CMD_SendATMessage(response, source) )
      {
         result = SUCCESS_E;
         CO_PRINT_B_1(DBG_INFO_E,"<<QAM:%s\r\n", response);
      
#ifdef AUTO_DISCARD_CIE_QUEUE_MSG_AFTER_READ
         MM_CIEQ_Discard(CIE_Q_ALARM_E);
#endif
      }
   }
   else 
   {
      CO_PRINT_B_1(DBG_INFO_E,"<<QAM:ERROR %d\r\n", result);
      MM_CMD_SendATFailWithErrorCode(NULL, result, source);
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessReadCIEFaultMessageQueue
*  Description:      Read a message from the CIE Fault queue and send it to the CIE.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessReadCIEFaultMessageQueue( const CommandSource_t source )
{
   ErrorCode_t result;
   CieBuffer_t buffer;
   char response[CIE_QUEUE_DATA_SIZE];
   
   result = MM_CIEQ_Pop(CIE_Q_FAULT_E, &buffer);
   
   if ( SUCCESS_E == result )
   {  
      snprintf(response,CIE_QUEUE_DATA_SIZE,"%s", (char*)&buffer);
     
      /* respond over AT USART link */
      if( MM_CMD_SendATMessage(response, source) )
      {
         result = SUCCESS_E;
         CO_PRINT_B_1(DBG_INFO_E,"<<QFT:%s\r\n", response);
         
#ifdef AUTO_DISCARD_CIE_QUEUE_MSG_AFTER_READ
         MM_CIEQ_Discard(CIE_Q_FAULT_E);
#endif
      }
   }
   else 
   {
      CO_PRINT_B_1(DBG_INFO_E,"<<QFT:ERROR %d\r\n", result);
      MM_CMD_SendATFailWithErrorCode(NULL, result, source);
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessReadCIEMiscMessageQueue
*  Description:      Read a message from the CIE Misc queue and send it to the CIE.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessReadCIEMiscMessageQueue( const CommandSource_t source )
{
   ErrorCode_t result;
   CieBuffer_t buffer;
   char response[CIE_QUEUE_DATA_SIZE];
   
   result = MM_CIEQ_Pop(CIE_Q_MISC_E, &buffer);
   
   if ( SUCCESS_E == result )
   {  
      snprintf(response,CIE_QUEUE_DATA_SIZE,"%s", buffer.Buffer);
     
      /* respond over AT USART link */
      if( MM_CMD_SendATMessage(response, source) )
      {
         result = SUCCESS_E;
         CO_PRINT_B_1(DBG_INFO_E,"<<%s\r\n", response);
         
#ifdef AUTO_DISCARD_CIE_QUEUE_MSG_AFTER_READ
         MM_CIEQ_Discard(CIE_Q_MISC_E);
#endif
      }
   }
   else 
   {
      CO_PRINT_B_1(DBG_INFO_E,"<<QMC:ERROR %d\r\n", result);
      MM_CMD_SendATFailWithErrorCode("QMC: ", result, source);
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessDiscardFromCIEFireMessageQueue
*  Description:      Remove a message from the CIE Fire queue.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessDiscardFromCIEFireMessageQueue( const CommandSource_t source )
{
   ErrorCode_t result;
   
   result = MM_CIEQ_Discard(CIE_Q_FIRE_E);
   
   if ( SUCCESS_E == result )
   {    
      /* respond over AT USART link */
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);         
   }
   else 
   {
      MM_CMD_SendATFailWithErrorCode(NULL, result, source);
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessDiscardFromCIEAlarmMessageQueue
*  Description:      Remove a message from the CIE Alarm queue.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessDiscardFromCIEAlarmMessageQueue( const CommandSource_t source )
{
   ErrorCode_t result;
   
   result = MM_CIEQ_Discard(CIE_Q_ALARM_E);
   
   if ( SUCCESS_E == result )
   {    
      /* respond over AT USART link */
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);         
   }
   else 
   {
      MM_CMD_SendATFailWithErrorCode(NULL, result, source);
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessDiscardFromCIEFaultMessageQueue
*  Description:      Remove a message from the CIE Fault queue.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessDiscardFromCIEFaultMessageQueue( const CommandSource_t source )
{
   ErrorCode_t result;
   
   result = MM_CIEQ_Discard(CIE_Q_FAULT_E);
   
   if ( SUCCESS_E == result )
   {    
      /* respond over AT USART link */
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);         
   }
   else
   {
      MM_CMD_SendATFailWithErrorCode(NULL, result, source);
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessDiscardFromCIEMiscMessageQueue
*  Description:      Remove a message from the CIE Misc queue.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessDiscardFromCIEMiscMessageQueue( const CommandSource_t source )
{
   ErrorCode_t result;
   
   result = MM_CIEQ_Discard(CIE_Q_MISC_E);
   
   if ( SUCCESS_E == result )
   {    
      /* respond over AT USART link */
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);         
   }
   else
   {
      MM_CMD_SendATFailWithErrorCode(NULL, result, source);
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessResetSelectedQueues
*  Description:      Check the array pCommand->QueueProperty for each queue/counter
*                    and reset the queue/counter if the array element = true.
*
*  Param - pCommand  The command structure contaning the reset falgs.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessResetSelectedQueues ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
      result = SUCCESS_E;
      
      if ( pCommand->QueueProperty[CIE_Q_FIRE_E] )
      {
         if ( SUCCESS_E != MM_CIEQ_ResetMessageQueue(CIE_Q_FIRE_E) )
         {
            result = ERR_MESSAGE_FAIL_E;
         }
      }
      
      if ( pCommand->QueueProperty[CIE_Q_FIRE_LOST_COUNT_E] )
      {
         if ( SUCCESS_E != MM_CIEQ_ResetLostMessageCount(CIE_Q_FIRE_E) )
         {
            result = ERR_MESSAGE_FAIL_E;
         }
      }
      
      if ( pCommand->QueueProperty[CIE_Q_ALARM_E] )
      {
         if ( SUCCESS_E != MM_CIEQ_ResetMessageQueue(CIE_Q_ALARM_E) )
         {
            result = ERR_MESSAGE_FAIL_E;
         }
      }
      
      if ( pCommand->QueueProperty[CIE_Q_ALARM_LOST_COUNT_E] )
      {
         if ( SUCCESS_E != MM_CIEQ_ResetLostMessageCount(CIE_Q_ALARM_E) )
         {
            result = ERR_MESSAGE_FAIL_E;
         }
      }
      
      if ( pCommand->QueueProperty[CIE_Q_FAULT_E] )
      {
         if ( SUCCESS_E != MM_CIEQ_ResetMessageQueue(CIE_Q_FAULT_E) )
         {
            result = ERR_MESSAGE_FAIL_E;
         }
      }
      
      if ( pCommand->QueueProperty[CIE_Q_FAULT_LOST_COUNT_E] )
      {
         if ( SUCCESS_E != MM_CIEQ_ResetLostMessageCount(CIE_Q_FAULT_E) )
         {
            result = ERR_MESSAGE_FAIL_E;
         }
      }
      
      if ( pCommand->QueueProperty[CIE_Q_MISC_E] )
      {
         if ( SUCCESS_E != MM_CIEQ_ResetMessageQueue(CIE_Q_MISC_E) )
         {
            result = ERR_MESSAGE_FAIL_E;
         }
      }
      
      if ( pCommand->QueueProperty[CIE_Q_MISC_LOST_COUNT_E] )
      {
         if ( SUCCESS_E != MM_CIEQ_ResetLostMessageCount(CIE_Q_MISC_E) )
         {
            result = ERR_MESSAGE_FAIL_E;
         }
      }
      
      if ( pCommand->QueueProperty[CIE_Q_TX_BUFFER_E] )
      {
         if ( SUCCESS_E != MM_CIEQ_ResetMessageQueue(CIE_Q_TX_BUFFER_E) )
         {
            result = ERR_MESSAGE_FAIL_E;
         }
      }      
   }
   
   if ( SUCCESS_E == result )
   {    
      /* respond over AT USART link */
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);         
   }
   else 
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessCIERemoteCommand
*  Description:         Send a command to a remote RBU i.e. read cmd or reboot
*  Param - pCommand     The command structure contaning the RBU node ID.
*  Param - CommandType  The Parameter being requested.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCIERemoteCommand( const CIECommand_t* pCommand, const ParameterType_t ParameterType, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
      {
         CO_CommandData_t commandData;
         commandData.TransactionID = GetNextTransactionID();
         commandData.Source = NCU_NETWORK_ADDRESS;
         commandData.CommandType = ParameterType;
         commandData.Destination = pCommand->NodeID;
         commandData.Parameter1 = pCommand->Parameter1;
         commandData.Parameter2 = pCommand->Parameter2;
         commandData.Value = pCommand->Value;
         commandData.ReadWrite = pCommand->ReadWrite;
         commandData.SourceUSART = (uint16_t)source;
         
         CieBuffer_t msgBuffer;
         msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
         memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
         
         result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
      }
      else 
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
      }
   }
   
   if ( SUCCESS_E == result )
   {    
      /* respond over AT USART link */
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
   }
   else 
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessTestModeCommand
*  Description:         Send a test mode change to an RBU
*  Param - pCommand     The command structure contaning the RBU node ID.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessTestModeCommand( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
      {
         CO_CommandData_t commandData;
         commandData.TransactionID = GetNextTransactionID();
         commandData.Source = NCU_NETWORK_ADDRESS;
         commandData.CommandType = PARAM_TYPE_TEST_MODE_E;
         commandData.Destination = pCommand->NodeID;
         commandData.Parameter1 = 0;
         commandData.Parameter2 = 0;
         commandData.Value = pCommand->Value;
         commandData.ReadWrite = 1;
         commandData.SourceUSART = (uint16_t)source;
         
         CieBuffer_t msgBuffer;
         msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
         memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
         
         result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
      }
      else 
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
      }
   }
   
   if ( SUCCESS_E == result )
   {    
      /* respond over AT USART link */
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
   }
   else 
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessReadAnalogueValueCommand
*  Description:         Read the analogue value from an RBU channel.
*  Param - pCommand     The command structure contaning the RBU node ID.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessReadAnalogueValueCommand( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   CO_CommandData_t commandData;
   CO_PRINT_B_0(DBG_INFO_E,"MM_CMD_ProcessReadAnalogueValueCommand+\r\n");

   if ( pCommand )
   {
      commandData.TransactionID = GetNextTransactionID();
      commandData.Source = NCU_NETWORK_ADDRESS;
      commandData.CommandType = PARAM_TYPE_ANALOGUE_VALUE_E;
      commandData.Destination = pCommand->NodeID;
      commandData.Parameter1 = pCommand->Value;
      commandData.Parameter2 = 0;
      commandData.Value = 0;
      commandData.ReadWrite = 0;
      commandData.SourceUSART = (uint16_t)source;
      if ( NCU_NETWORK_ADDRESS == gNetworkAddress )
      {
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
         }
         else 
         {
            result = ERR_SYSTEM_NOT_ACTIVE_E;
         }
      }
      else if ( pCommand->NodeID == gNetworkAddress )
      {
         //Local node.
         result = MM_CMD_ProcessCommand ( &commandData, source );
         
      }
      else 
      {
         result = ERR_INVALID_PARAMETER_E;
      }
   }
   
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessBeaconFlashRate
*  Description:         Read/write the beacon flash rate of an RBU.
*  Param - pCommand     The command structure contaning the RBU node ID.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessBeaconFlashRate( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
      {
         /* The Command/Response command was updated to always carry the zone number
            in parameter P2.  In the case of the FlashRate command, this displaced the
            OutputProfile, which we must now encode into the 'Value' field.
            The 'Value' field now consists of OutputsEnabled in byte0 and the 
            OutputProfile in byte1.
            Parameter2 contains the zone number
         */
         uint32_t value = ((uint32_t)pCommand->Parameter2 << 8);
         value |= (pCommand->Value & 0xff);
         
         CO_CommandData_t commandData;
         commandData.TransactionID = GetNextTransactionID();
         commandData.Source = NCU_NETWORK_ADDRESS;
         commandData.CommandType = PARAM_TYPE_FLASH_RATE_E;
         commandData.Destination = pCommand->NodeID;
         commandData.Parameter1 = pCommand->Parameter1;
         commandData.Parameter2 = (uint8_t)pCommand->Zone;
         commandData.Value = value;
         commandData.ReadWrite = pCommand->ReadWrite;
         commandData.SourceUSART = (uint16_t)source;
         
         CieBuffer_t msgBuffer;
         msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
         memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
         
         result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
      }
      else 
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
      }
   }
   
   if ( SUCCESS_E == result )
   {    
      /* respond over AT USART link */
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
   }
   else 
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessDeviceEnable
*  Description:         Enable/disable a device on an RBU.
*  Param - pCommand     The command structure contaning the RBU node ID.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessDeviceEnable( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
      {
         CO_CommandData_t commandData;
         commandData.TransactionID = GetNextTransactionID();
         commandData.Source = NCU_NETWORK_ADDRESS;
         commandData.CommandType = PARAM_TYPE_PLUGIN_ENABLE_E;
         commandData.Destination = pCommand->NodeID;
         commandData.Parameter1 = pCommand->Parameter1;
         commandData.Parameter2 = pCommand->Parameter2;
         commandData.Value = pCommand->Value;
         commandData.ReadWrite = pCommand->ReadWrite;
         commandData.SourceUSART = (uint16_t)source;
         
         CieBuffer_t msgBuffer;
         msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
         memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
         
         result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
      }
      else 
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
      }
   }
   
   if ( SUCCESS_E == result )
   {    
      /* respond over AT USART link */
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
   }
   else 
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessDeviceEnable
*  Description:         Enable/disable a device on an RBU.
*  Param - pCommand     The command structure contaning the RBU node ID.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessEnableFaultsClearMessage ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      CO_CommandData_t commandData;
      commandData.TransactionID = GetNextTransactionID();
      commandData.Source = NCU_NETWORK_ADDRESS;
      commandData.CommandType = PARAM_TYPE_ENABLE_FAULTS_CLEAR_MSG_E;
      commandData.Destination = pCommand->NodeID;
      commandData.Parameter1 = pCommand->Parameter1;
      commandData.Parameter2 = pCommand->Parameter2;
      commandData.Value = pCommand->Value;
      commandData.ReadWrite = pCommand->ReadWrite;
      commandData.SourceUSART = (uint16_t)source;
      
      if ( NCU_NETWORK_ADDRESS == gNetworkAddress )
      {
         commandData.Source = NCU_NETWORK_ADDRESS;
         
         CieBuffer_t msgBuffer;
         msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
         memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
         
         result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
      }
      else if ( (gNetworkAddress == pCommand->NodeID) || (ADDRESS_GLOBAL == pCommand->NodeID) )
      {
         commandData.Source = gNetworkAddress;
         
         CO_Message_t* pCmd = osPoolAlloc(AppPool);
         if ( pCmd )
         {
            pCmd->Type = CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E;
            memcpy(pCmd->Payload.PhyDataReq.Data, &commandData, sizeof(CO_CommandData_t));
            
            osStatus osStat = osMessagePut(AppQ, (uint32_t)pCmd, 0);
            if (osOK != osStat)
            {
               /* failed to write */
               osPoolFree(AppPool, pCmd);
               result = ERR_NO_RESOURCE_E;
            }
            else
            {
               result = SUCCESS_E;
            }
         }
      }
      else 
      {
         result = ERR_MESSAGE_FAIL_E;
      }
   }
   
   if ( SUCCESS_E == result )
   {    
      /* respond over AT USART link */
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
   }
   else 
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessSounderToneSelection
*  Description:         Read/write the sounder tone selection of an RBU plugin.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessSounderToneSelection( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
      {
         /* The Command/Response command was updated to always carry the zone number
            in parameter P2.  In the case of the Tone Selection command, this displaced the
            OutputProfile, which we must now encode into the 'Value' field.
            The 'Value' field now consists of the tone selection in byte0 and the 
            OutputProfile in byte1.
            Parameter2 contains the zone number
         */
         uint32_t value = ((uint32_t)pCommand->Parameter2 << 8);
         value |= (pCommand->Value & 0xff);
         
         CO_CommandData_t commandData;
         commandData.TransactionID = GetNextTransactionID();
         commandData.Source = NCU_NETWORK_ADDRESS;
         commandData.CommandType = PARAM_TYPE_TONE_SELECTION_E;
         commandData.Destination = pCommand->NodeID;
         commandData.Parameter1 = pCommand->Parameter1;
         commandData.Parameter2 = (uint8_t)pCommand->Zone;
         commandData.Value = value;
         commandData.ReadWrite = pCommand->ReadWrite;
         commandData.SourceUSART = (uint16_t)source;
         
         CieBuffer_t msgBuffer;
         msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
         memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
         
         result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
      }
      else 
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
      }
   }
   
   if ( SUCCESS_E == result )
   {    
      /* respond over AT USART link */
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
   }
   else 
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessUpperThresholdSetting
*  Description:         Read/write the alarm threshold setting of an RBU plugin.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessUpperThresholdSetting( const CIECommand_t* pCommand, const CommandSource_t source )
{
//   ErrorCode_t result = ERR_INVALID_POINTER_E;

//   if ( pCommand )
//   {
//      if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
//      {
//         CO_CommandData_t commandData;
//         commandData.TransactionID = GetNextTransactionID();
//         commandData.Source = NCU_NETWORK_ADDRESS;
//         commandData.CommandType = PARAM_TYPE_ALARM_THRESHOLD_E;
//         commandData.Destination = pCommand->NodeID;
//         commandData.Parameter1 = pCommand->Parameter1;
//         commandData.Parameter2 = pCommand->Parameter2;
//         commandData.Value = pCommand->Value;
//         commandData.ReadWrite = pCommand->ReadWrite;
//         commandData.SourceUSART = (uint16_t)source;
//         
//         CieBuffer_t msgBuffer;
//         msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
//         memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
//         
//         result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
//      }
//      else 
//      {
//         result = ERR_SYSTEM_NOT_ACTIVE_E;
//      }
//   }
//   
//   if ( SUCCESS_E == result )
//   {    
//      /* respond over AT USART link */
//      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
//   }
//   else 
//   {
//      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
//   }
//  
//   return result;
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   CO_CommandData_t commandData;
   CO_PRINT_B_0(DBG_INFO_E,"MM_CMD_ProcessUpperThresholdSetting+\r\n");

   if ( pCommand )
   {
      commandData.TransactionID = GetNextTransactionID();
      commandData.Source = NCU_NETWORK_ADDRESS;
      commandData.CommandType = PARAM_TYPE_ALARM_THRESHOLD_E;
      commandData.Destination = pCommand->NodeID;
      commandData.Parameter1 = pCommand->Parameter1;
      commandData.Parameter2 = pCommand->Parameter2;
      commandData.Value = pCommand->Value;
      commandData.ReadWrite = pCommand->ReadWrite;
      commandData.SourceUSART = (uint16_t)source;
      if ( NCU_NETWORK_ADDRESS == gNetworkAddress )
      {
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
         }
         else 
         {
            result = ERR_SYSTEM_NOT_ACTIVE_E;
         }
      }
      else if ( pCommand->NodeID == gNetworkAddress )
      {
         //Local node.
         result = MM_CMD_ProcessCommand ( &commandData, source );
         
      }
      else 
      {
         result = ERR_INVALID_PARAMETER_E;
      }
   }
   
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessLowerThresholdSetting
*  Description:         Read/write the pre-alarm threshold setting of an RBU plugin.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessLowerThresholdSetting( const CIECommand_t* pCommand, const CommandSource_t source )
{
//   ErrorCode_t result = ERR_INVALID_POINTER_E;

//   if ( pCommand )
//   {
//      if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
//      {
//         CO_CommandData_t commandData;
//         commandData.TransactionID = GetNextTransactionID();
//         commandData.Source = NCU_NETWORK_ADDRESS;
//         commandData.CommandType = PARAM_TYPE_PRE_ALARM_THRESHOLD_E;
//         commandData.Destination = pCommand->NodeID;
//         commandData.Parameter1 = pCommand->Parameter1;
//         commandData.Parameter2 = pCommand->Parameter2;
//         commandData.Value = pCommand->Value;
//         commandData.ReadWrite = pCommand->ReadWrite;
//         commandData.SourceUSART = (uint16_t)source;
//         
//         CieBuffer_t msgBuffer;
//         msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
//         memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
//         
//         result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
//      }
//      else 
//      {
//         result = ERR_SYSTEM_NOT_ACTIVE_E;
//      }
//   }
//   
//   if ( SUCCESS_E == result )
//   {    
//      /* respond over AT USART link */
//      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
//   }
//   else 
//   {
//      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
//   }
//  
//   return result;
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   CO_CommandData_t commandData;
   CO_PRINT_B_0(DBG_INFO_E,"MM_CMD_ProcessLowerThresholdSetting+\r\n");

   if ( pCommand )
   {
      commandData.TransactionID = GetNextTransactionID();
      commandData.Source = NCU_NETWORK_ADDRESS;
      commandData.CommandType = PARAM_TYPE_PRE_ALARM_THRESHOLD_E;
      commandData.Destination = pCommand->NodeID;
      commandData.Parameter1 = pCommand->Parameter1;
      commandData.Parameter2 = pCommand->Parameter2;
      commandData.Value = pCommand->Value;
      commandData.ReadWrite = pCommand->ReadWrite;
      commandData.SourceUSART = (uint16_t)source;
      if ( NCU_NETWORK_ADDRESS == gNetworkAddress )
      {
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
         }
         else 
         {
            result = ERR_SYSTEM_NOT_ACTIVE_E;
         }
      }
      else if ( pCommand->NodeID == gNetworkAddress )
      {
         //Local node.
         //result = MM_CMD_ProcessCommandAnalogueValue( &commandData, source );
         result = MM_CMD_ProcessCommand ( &commandData, source );
         
      }
      else 
      {
         result = ERR_INVALID_PARAMETER_E;
      }
   }
   
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessPluginTestMode
*  Description:         Read/write the Test Mode setting of an RBU plugin.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessPluginTestMode( const CIECommand_t* pCommand, const CommandSource_t source )
{
//   ErrorCode_t result = ERR_INVALID_POINTER_E;

//   if ( pCommand )
//   {
//      if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
//      {
//         CO_CommandData_t commandData;
//         commandData.TransactionID = GetNextTransactionID();
//         commandData.Source = NCU_NETWORK_ADDRESS;
//         commandData.CommandType = PARAM_TYPE_PLUGIN_TEST_MODE_E;
//         commandData.Destination = pCommand->NodeID;
//         commandData.Parameter1 = pCommand->Parameter1;
//         commandData.Parameter2 = pCommand->Parameter2;
//         commandData.Value = pCommand->Value;
//         commandData.ReadWrite = pCommand->ReadWrite;
//         commandData.SourceUSART = (uint16_t)source;
//         
//         CieBuffer_t msgBuffer;
//         msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
//         memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
//         
//         result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
//      }
//      else 
//      {
//         result = ERR_SYSTEM_NOT_ACTIVE_E;
//      }
//   }
//   
//   if ( SUCCESS_E == result )
//   {    
//      /* respond over AT USART link */
//      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
//   }
//   else 
//   {
//      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
//   }
//  
//   return result;
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   CO_CommandData_t commandData;
   CO_PRINT_B_0(DBG_INFO_E,"MM_CMD_ProcessPluginTestMode+\r\n");

   if ( pCommand )
   {
      commandData.TransactionID = GetNextTransactionID();
      commandData.Source = NCU_NETWORK_ADDRESS;
      commandData.CommandType = PARAM_TYPE_PLUGIN_TEST_MODE_E;
      commandData.Destination = pCommand->NodeID;
      commandData.Parameter1 = pCommand->Parameter1;
      commandData.Parameter2 = pCommand->Parameter2;
      commandData.Value = pCommand->Value;
      commandData.ReadWrite = pCommand->ReadWrite;
      commandData.SourceUSART = (uint16_t)source;
      if ( NCU_NETWORK_ADDRESS == gNetworkAddress )
      {
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
         }
         else 
         {
            result = ERR_SYSTEM_NOT_ACTIVE_E;
         }
      }
      else if ( pCommand->NodeID == gNetworkAddress )
      {
         //Local node.
         //result = MM_CMD_ProcessCommandAnalogueValue( &commandData, source );
         result = MM_CMD_ProcessCommand ( &commandData, source );
         
      }
      else 
      {
         result = ERR_INVALID_PARAMETER_E;
      }
   }
   
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessCRCInformation
*  Description:      Process the CRC Information command for the RBU/NCU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCRCInformation ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   char error_response[AT_RESPONSE_MAX_LENGTH];
   
   if ( pCommand )
   {
      /* create a command for the MAC */
      CO_ResponseData_t response;
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = 0;
      
      
      /* we only support READ for this command */
      if ( COMMAND_READ == pCommand->ReadWrite )
      {         
         switch ( pCommand->Parameter1 )
         {
            case FI_RBU_MAIN_IMAGE_E:
               response.Value = blf_application_checksum;
               result = SUCCESS_E;
               break;
            case FI_RBU_BACKUP_IMAGE_E:
            {
               uint32_t firmware_crc;
               /* find out which bank is currently running */
               uint32_t active_bank =  FLASH_If_GetActiveBank();
               
               if ( FLASH_BANK_1 == active_bank )
               {
                  /* Bank 1 is active.  Read bank 2 CRC */
                  result = DM_NVMRead(NV_BANK2_CRC_E, &firmware_crc, sizeof(uint32_t));
               }
               else 
               {
                  /* Bank 2 is active.  Read bank 1 CRC */
                  result = DM_NVMRead(NV_BANK1_CRC_E, &firmware_crc, sizeof(uint32_t));
               }

               if ( SUCCESS_E == result )
               {
                  response.Value = firmware_crc;
               }
            }
               break;
            case FI_INTERNAL_DEVICE_1_E:/* I2C. The SVI does not support CRC information*//* intentional drop-through */
            case FI_INTERNAL_DEVICE_2_E:
            case FI_INTERNAL_DEVICE_3_E:
            case FI_EXTERNAL_DEVICE_1_E:/* Head.  The Head interface does not support CRC information */
            case FI_EXTERNAL_DEVICE_2_E: 
            case FI_EXTERNAL_DEVICE_3_E:
               response.Value = 0;
               result = SUCCESS_E;
               break;
            default:
               result = ERR_OUT_OF_RANGE_E;
               snprintf(error_response, AT_RESPONSE_MAX_LENGTH,"ERROR,BAD_PARAM");
               response.Value = 0;
               break;
         }

         /* Send the response back to the command's originator */;
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            if ( SUCCESS_E == result )
            {
               /* respond over AT USART link */            
               MM_CMD_SendATResponse( &response, source );
            }
            else 
            {
               MM_CMD_SendATMessage(error_response, source);
            }
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
         result = SUCCESS_E;
      }
      else
      {
         result = ERR_MESSAGE_FAIL_E;
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessEEPROMInformation
*  Description:      Process the EEPROM Information command for the RBU/NCU.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessEEPROMInformation ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {      
      uint32_t eeprom_index = (uint32_t)NV_RESERVED_0_E + pCommand->Parameter1;
      uint32_t eeprom_value = pCommand->Value;
      
      if ( NV_RESERVED_9_E >= eeprom_index )
      {
         /* create a response */
         CO_ResponseData_t response = { 0 };
         response.CommandType = pCommand->CommandType;
         response.Source = pCommand->Source;
         response.Destination = pCommand->Destination;
         response.Parameter1 = pCommand->Parameter1;
         response.Parameter2 = pCommand->Parameter2;
         response.ReadWrite = pCommand->ReadWrite;
         response.TransactionID = pCommand->TransactionID;
         response.Value = 0;
         
         
         if ( COMMAND_READ == pCommand->ReadWrite )
         {
            result = DM_NVMRead((DM_NVMParamId_t)eeprom_index, &eeprom_value, sizeof(uint32_t));
            
            if ( SUCCESS_E == result )
            {
               response.Value = eeprom_value;
            }
            
            /* send value back to the command's originator */;
            if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
            {
               /* respond over AT USART link */
               if ( SUCCESS_E == result )
               {
                  /* respond over AT USART link */            
                  MM_CMD_SendATResponse( &response, source );
               }
               else 
               {
                  MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
               }
            }
            else
            {
               /* respond over Mesh link */
               bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                          response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
               if ( msg_sent )
               {
                  result = SUCCESS_E;
               }
            }
         }
         else
         {
            result = DM_NVMWrite((DM_NVMParamId_t)eeprom_index, &eeprom_value, sizeof(uint32_t));

            if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
            {
               if ( SUCCESS_E == result )
               {
                  /* respond over AT USART link */            
                  MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
               }
               else 
               {
                  MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
               }            
            }
            else
            {
               /* respond over Mesh link */
               MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                          response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
            }            
         }
      }
      else 
      {
         result = ERR_OUT_OF_RANGE_E;
         MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* MM_CMD_ProcessReadNeighbourInformation
* Process a request from the CIE for neighbour information on a specified node.
*
* @param - pCommand     The command from the CIE
* @param - source       The UART that the command arrived on
*
* @return - void
*/
ErrorCode_t MM_CMD_ProcessReadNeighbourInformation ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_MESSAGE_FAIL_E;
   
   /* This command is NCU only */
   if ( gIsNCU )
   {
      if ( pCommand )
      {
         NeighbourInformation_t neighbourInfo;
         
         if ( MM_NI_GetNeighbourInformation( pCommand->NodeID, &neighbourInfo ) )
         {
            /* Build a response and put it in the MISC queue for the CIE */
            CieBuffer_t cieMsg;
            snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "DNI:Z%dU%d,%d,%d,%d,%d\r\n", pCommand->Zone, pCommand->NodeID,
                     neighbourInfo.PrimaryParentID, neighbourInfo.PrimaryParentRssi, neighbourInfo.SecondaryParentID, neighbourInfo.SecondaryParentRssi );
            /* Push the response into the MISC queue */
            result = MM_CIEQ_Push(CIE_Q_MISC_E, &cieMsg);
         }
      }
   }
   
   /* Report success or failure on the source uart */
   if ( SUCCESS_E == result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
   }
   else 
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessReadDeviceStatusFlags
*  Description:         Read the fault status flags from the target device.
*  Param - pCommand     The command structure contaning the RBU node ID.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessReadDeviceStatusFlags ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   char AtBuffer[AT_RESPONSE_MAX_LENGTH] = "OK";
   uint32_t zone_number = CFG_GetZoneNumber();

   if ( pCommand )
   {
      if ( gNetworkAddress == pCommand->NodeID )
      {
         uint32_t fault_status_flags = MM_GetFaultStatusFlags();
         /* if we are the NCU we put the response into the CIE misc queue*/;
         if ( gIsNCU )
         {
            /* create the fault msg for the CIE */
            CieBuffer_t deviceStatus;
            snprintf((char*)deviceStatus.Buffer, CIE_QUEUE_DATA_SIZE, "%s:Z%dU%d,0,0,1,%X", ParamCommandMap[PARAM_TYPE_STATUS_FLAGS_E], zone_number, gNetworkAddress, fault_status_flags);
            /*push the message into the CIE misc queue */
            result = MM_CIEQ_Push(CIE_Q_MISC_E, &deviceStatus);
         }
         else 
         {
            /* this is an RBU and the local response should be sent straight to the requesting UART */
            snprintf((char*)AtBuffer, AT_RESPONSE_MAX_LENGTH, "Z%dU%d,0,0,1,%X", zone_number, gNetworkAddress, fault_status_flags);
            result = SUCCESS_E;
         }
      }
      else if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
      {
         /* The message is addressed to a remote device */
         result = MM_CMD_ProcessCIERemoteCommand( pCommand, PARAM_TYPE_STATUS_FLAGS_E, source );
      }
      else 
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
      }
   }
   
   if ( SUCCESS_E == result )
   {    
      /* respond over AT USART link */
      MM_CMD_SendATMessage(AtBuffer, source);
   }
   else 
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessReadStatusIndication
*  Description:         Read the status indication from the target device.
*  Param - pCommand     The command structure contaning the RBU node ID.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessReadStatusIndication ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   char AtBuffer[AT_RESPONSE_MAX_LENGTH] = "OK";

   if ( pCommand )
   {
      if ( gNetworkAddress == pCommand->NodeID )
      {
         /* The mesh status is held in the session layer.  Send a request for information to the Mesh code */
         if ( MM_MeshAPIApplicationCommand(APP_MSG_TYPE_STATUS_INDICATION_E, 0, 0, (uint8_t)source, 0) )
         {
            result = SUCCESS_E;
         }
         else
         {
            result = ERR_MESSAGE_FAIL_E;
         }
      }
      else if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
      {
         if ( 0 != pCommand->Parameter1 )
         {
            /* The message is addressed to a remote device */
            result = MM_CMD_ProcessCIERemoteCommand( pCommand, PARAM_TYPE_MESH_STATUS_E, source );
         }
      }
      else 
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
      }
   }
   
   if ( SUCCESS_E == result )
   {    
      /* respond over AT USART link */
      MM_CMD_SendATMessage(AtBuffer, source);
   }
   else 
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessReadRBUSerialNumber
*  Description:         Read the RBU serial number from the target device.
*  Param - pCommand     The command structure contaning the RBU node ID.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessReadRBUSerialNumber ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   char AtBuffer[AT_RESPONSE_MAX_LENGTH] = "OK";
   uint32_t zone_number = CFG_GetZoneNumber();

   if ( pCommand )
   {
      if ( gNetworkAddress == pCommand->NodeID )
      {
         /* The message is addressed to this device */
         /* Read the serial number from NVM */
         char snr_str[16];
         uint32_t snr;
         result = DM_NVMRead(NV_UNIT_SERIAL_NO_E, &snr, sizeof(uint32_t));
         /* Decode the raw number into a string */
         if ( MM_ATDecodeSerialNumber(snr, snr_str) )
         {
            /* if we are the NCU we put the response into the CIE misc queue*/;
            if ( gIsNCU )
            {
               /* create the fault msg for the CIE */
               CieBuffer_t deviceStatus;
               snprintf((char*)deviceStatus.Buffer, CIE_QUEUE_DATA_SIZE, "%s:Z%dU%d,0,0,1,%s", ParamCommandMap[PARAM_TYPE_RBU_SERIAL_NUMBER_E], zone_number, gNetworkAddress, snr_str);
               /*push the message into the CIE misc queue */
               result = MM_CIEQ_Push(CIE_Q_MISC_E, &deviceStatus);
            }
            else 
            {
               /* This is an RBU and the local response should be sent straight to the requesting UART */
               snprintf((char*)AtBuffer, AT_RESPONSE_MAX_LENGTH, "Z%dU%d,0,0,1,%s", zone_number, gNetworkAddress, snr_str);
               result = SUCCESS_E;
            }
         }
         else
         {
            result = ERR_MESSAGE_FAIL_E;
         }
      }
      else if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
      {
         /* The message is addressed to a remote device */
         result = MM_CMD_ProcessCIERemoteCommand( pCommand, PARAM_TYPE_RBU_SERIAL_NUMBER_E, source );
      }
      else 
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
      }
   }
   
   if ( SUCCESS_E == result )
   {    
      /* respond over AT USART link */
      MM_CMD_SendATMessage(AtBuffer, source);
   }
   else 
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessFirmwareInfoRequest
*  Description:         Read the from the target device.
*  Param - pCommand     The command structure contaning the RBU node ID, and output channel.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessFirmwareInfoRequest ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   char AtBuffer[AT_RESPONSE_MAX_LENGTH] = "OK";
   bool respond_to_usart = true;
   uint32_t zone_number = CFG_GetZoneNumber();
   
   if ( pCommand )
   {
      if ( gNetworkAddress == pCommand->NodeID )
      {
         uint32_t major = (gSoftwareVersionNumber >> 14) & 0x7f;
         uint32_t minor = (gSoftwareVersionNumber >> 7) & 0x7f;
         uint32_t increment = gSoftwareVersionNumber & 0x7f;
         
         if ( NCU_NETWORK_ADDRESS == gNetworkAddress)
         {
            /* NCU Local node - Put result in CIE misc queue*/
            CieBuffer_t cieMsg;
            snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "FIR:Z%dU%d,0,0,1,%02d.%02d.%02d", zone_number, gNetworkAddress, major, minor, increment);
            /*push the message into the CIE misc queue */
            result = MM_CIEQ_Push(CIE_Q_MISC_E, &cieMsg);
         }
         else 
         {
            /* RBU Local node - Send result to UART*/
            snprintf((char*)AtBuffer, AT_RESPONSE_MAX_LENGTH, "Z%dU%d,0,0,1,%02d.%02d.%02d", zone_number, gNetworkAddress, major, minor, increment);
            result = SUCCESS_E;
         }
      }
      else if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
      {
         /* The message is addressed to a remote device */
         result = MM_CMD_ProcessCIERemoteCommand( pCommand, PARAM_TYPE_FIRMWARE_INFO_E, source );
         respond_to_usart = false; /* MM_CMD_ProcessCIERemoteCommand takes care of the response */
      }
      else 
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
      }
   }
   
   if ( respond_to_usart )
   {
      if ( SUCCESS_E == result )
      {    
         /* respond over AT USART link */
         MM_CMD_SendATMessage(AtBuffer, source);
      }
      else 
      {
         MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
      }
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessPluginFirmwareInfoRequest
*  Description:         Read the firmware info from the head.
*  Param - pCommand     The command structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessPluginFirmwareInfoRequest ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   char AtBuffer[AT_RESPONSE_MAX_LENGTH] = "OK";
   bool respond_to_usart = true;
   
   if ( pCommand )
   {
      if ( gNetworkAddress == pCommand->NodeID )
      {
         if ( NCU_NETWORK_ADDRESS == gNetworkAddress)
         {
            /* NCU does not have a plugin*/
            result = ERR_MESSAGE_FAIL_E;
         }
         else 
         {
            /* RBU Local node - Send result to UART*/
            CO_CommandData_t cmdData;
            cmdData.CommandType = PARAM_TYPE_FIRMWARE_INFO_E;
            cmdData.Destination = pCommand->NodeID;
            cmdData.Parameter1 = pCommand->Parameter1;
            cmdData.Parameter2 = pCommand->Parameter2;
            cmdData.ReadWrite = COMMAND_READ;
            cmdData.Source = source;
            cmdData.TransactionID = GetNextTransactionID();
            result = MM_CMD_ProcessHeadCommand( &cmdData, source );
            respond_to_usart = false;
         }
      }
      else if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
      {
         /* The message is addressed to a remote device */
         result = MM_CMD_ProcessCIERemoteCommand( pCommand, PARAM_TYPE_FIRMWARE_INFO_E, source );
         respond_to_usart = false; /* MM_CMD_ProcessCIERemoteCommand takes care of the response */
      }
      else 
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
      }
   }
   
   if ( respond_to_usart )
   {
      if ( SUCCESS_E == result )
      {    
         /* respond over AT USART link */
         MM_CMD_SendATMessage(AtBuffer, source);
      }
      else 
      {
         MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
      }
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessPluginTypeAndClassRequest
*  Description:         Read the Type and CLass from the head.
*  Param - pCommand     The command structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessPluginTypeAndClassRequest ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   char AtBuffer[AT_RESPONSE_MAX_LENGTH] = "OK";
   bool respond_to_usart = true;
   
   if ( pCommand )
   {
      if ( gNetworkAddress == pCommand->NodeID )
      {
         if ( NCU_NETWORK_ADDRESS == gNetworkAddress)
         {
            /* NCU does not have a plugin*/
            result = ERR_MESSAGE_FAIL_E;
         }
         else 
         {
            /* RBU Local node - Send result to UART*/
            CO_CommandData_t cmdData;
            cmdData.CommandType = PARAM_TYPE_PLUGIN_TYPE_AND_CLASS_E;
            cmdData.Destination = pCommand->NodeID;
            cmdData.Parameter1 = pCommand->Parameter1;
            cmdData.Parameter2 = pCommand->Parameter2;
            cmdData.ReadWrite = COMMAND_READ;
            cmdData.Source = source;
            cmdData.TransactionID = GetNextTransactionID();
            result = MM_CMD_ProcessHeadCommand( &cmdData, source );
            respond_to_usart = false;
         }
      }
      else if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
      {
         /* The message is addressed to a remote device */
         result = MM_CMD_ProcessCIERemoteCommand( pCommand, PARAM_TYPE_PLUGIN_TYPE_AND_CLASS_E, source );
         respond_to_usart = false; /* MM_CMD_ProcessCIERemoteCommand takes care of the response */
      }
      else 
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
      }
   }
   
   if ( respond_to_usart )
   {
      if ( SUCCESS_E == result )
      {    
         /* respond over AT USART link */
         MM_CMD_SendATMessage(AtBuffer, source);
      }
      else 
      {
         MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
      }
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessPluginSerialNumberRequest
*  Description:         Read the serial number from the head.
*  Param - pCommand     The command structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessPluginSerialNumberRequest ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
//   char AtBuffer[AT_RESPONSE_MAX_LENGTH] = "OK";
   bool respond_to_usart = true;
   
   if ( pCommand )
   {
      if ( gNetworkAddress == pCommand->NodeID )
      {
         if ( NCU_NETWORK_ADDRESS == gNetworkAddress)
         {
            /* NCU does not have a plugin*/
            result = ERR_MESSAGE_FAIL_E;
         }
         else 
         {
            /* RBU Local node - Send result to UART*/
            CO_CommandData_t cmdData;
            cmdData.CommandType = PARAM_TYPE_PLUGIN_SERIAL_NUMBER_E;
            cmdData.Destination = pCommand->NodeID;
            cmdData.Parameter1 = pCommand->Parameter1;
            cmdData.Parameter2 = pCommand->Parameter2;
            cmdData.ReadWrite = COMMAND_READ;
            cmdData.Source = source;
            cmdData.TransactionID = GetNextTransactionID();
            result = MM_CMD_ProcessHeadCommand( &cmdData, source );
            respond_to_usart = false;
         }
      }
      else if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
      {
         /* The message is addressed to a remote device */
         result = MM_CMD_ProcessCIERemoteCommand( pCommand, PARAM_TYPE_PLUGIN_SERIAL_NUMBER_E, source );
         respond_to_usart = false; /* MM_CMD_ProcessCIERemoteCommand takes care of the response */
      }
      else 
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
      }
   }
   
   if ( respond_to_usart )
   {
      if ( SUCCESS_E != result )
      {
         MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
      }
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessCIEDayNightSetting
*  Description:         Handle CIE command to Read/write the Day/Night setting of the system.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCIEDayNightSetting( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         CO_DayNight_t dayNight = CFG_GetDayNight();
         char buffer[AT_RESPONSE_MAX_LENGTH];
         snprintf(buffer, AT_RESPONSE_MAX_LENGTH, "%d",(uint32_t)dayNight);
         if ( MM_CMD_SendATMessage( buffer, source ) )
         {
            result = SUCCESS_E;
         }
      }
      else
      {
         if ( (uint32_t)CO_DAY_E == pCommand->Value || (uint32_t)CO_NIGHT_E == pCommand->Value )
         {
            result = MM_CMD_ProcessDayNightSetting((CO_DayNight_t)pCommand->Value);
            
            if ( ADDRESS_NCU == gNetworkAddress )
            {
               if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
               {
                  CO_CommandData_t commandData;
                  commandData.TransactionID = GetNextTransactionID();
                  commandData.Source = NCU_NETWORK_ADDRESS;
                  commandData.CommandType = PARAM_TYPE_DAY_NIGHT_SETTING_E;
                  commandData.Destination = ADDRESS_GLOBAL;
                  commandData.Parameter1 = 0;
                  commandData.Parameter2 = (uint8_t)ZONE_GLOBAL;
                  commandData.Value = pCommand->Value;
                  commandData.ReadWrite = 1;/*WRITE*/
                  commandData.SourceUSART = (uint16_t)source;
                  
                  CieBuffer_t msgBuffer;
                  msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
                  memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
                  
                  result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
               }
            }
            else 
            {
               result = SUCCESS_E;
            }
            
         }
         else 
         {
            result = ERR_OUT_OF_RANGE_E;
         }
      }
   }
   
   if ( SUCCESS_E == result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
   }
   else
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessPluginLEDSetting
*  Description:         Read/write the LED setting of an RBU plugin.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessPluginLEDSetting( const CIECommand_t* pCommand, const CommandSource_t source )
{
//   ErrorCode_t result = ERR_INVALID_POINTER_E;

//   if ( pCommand )
//   {
////      if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
////      {
//         CO_CommandData_t commandData;
//         commandData.TransactionID = GetNextTransactionID();
//         commandData.Source = NCU_NETWORK_ADDRESS;
//         commandData.CommandType = PARAM_TYPE_INDICATOR_LED_E;
//         commandData.Destination = pCommand->NodeID;
//         commandData.Parameter1 = pCommand->Parameter1;
//         commandData.Parameter2 = pCommand->Parameter2;
//         commandData.Value = pCommand->Value;
//         commandData.ReadWrite = pCommand->ReadWrite;
//         commandData.SourceUSART = (uint16_t)source;
//         
//         CieBuffer_t msgBuffer;
//         msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
//         memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
//         
//         result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
////      }
////      else 
////      {
////         result = ERR_SYSTEM_NOT_ACTIVE_E;
////      }
//   }
//   
//   if ( SUCCESS_E == result )
//   {    
//      /* respond over AT USART link */
//      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
//   }
//   else 
//   {
//      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
//   }
//  
//   return result;
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   CO_CommandData_t commandData;
   CO_PRINT_B_0(DBG_INFO_E,"MM_CMD_ProcessPluginLEDSetting+\r\n");

   if ( pCommand )
   {
      commandData.TransactionID = GetNextTransactionID();
      commandData.Source = NCU_NETWORK_ADDRESS;
      commandData.CommandType = PARAM_TYPE_INDICATOR_LED_E;
      commandData.Destination = pCommand->NodeID;
      commandData.Parameter1 = pCommand->Parameter1;
      commandData.Parameter2 = pCommand->Parameter2;
      commandData.Value = pCommand->Value;
      commandData.ReadWrite = pCommand->ReadWrite;
      commandData.SourceUSART = (uint16_t)source;
      if ( NCU_NETWORK_ADDRESS == gNetworkAddress )
      {
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
         }
         else 
         {
            result = ERR_SYSTEM_NOT_ACTIVE_E;
         }
      }
      else if ( pCommand->NodeID == gNetworkAddress )
      {
         //Local node.
         result = MM_CMD_ProcessCommand ( &commandData, source );
         
      }
      else 
      {
         result = ERR_INVALID_PARAMETER_E;
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessAlarmGlobalDelays
*  Description:         Read/write the delays on/off setting of the system.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessAlarmGlobalDelays( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   bool delays_enabled;

   if ( pCommand )
   {
      if ( gIsNCU )
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
         
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            if ( 0 == pCommand->Value || 1 == pCommand->Value )
            {
               delays_enabled = (0 == pCommand->Value ? false:true);
               CFG_SetGlobalDelayEnabled(delays_enabled);
               CO_CommandData_t commandData;
               commandData.TransactionID = GetNextTransactionID();
               commandData.Source = NCU_NETWORK_ADDRESS;
               commandData.CommandType = PARAM_TYPE_GLOBAL_DELAY_E;
               commandData.Destination = ADDRESS_GLOBAL;
               commandData.Parameter1 = 0;
               commandData.Parameter2 = (uint8_t)ZONE_GLOBAL;
               commandData.Value = pCommand->Value;
               commandData.ReadWrite = 1;/*WRITE*/
               commandData.SourceUSART = (uint16_t)source;
               
               CieBuffer_t msgBuffer;
               msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
               memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
               
               result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
            }
            else 
            {
               result = ERR_OUT_OF_RANGE_E;
            }
         }
         else 
         {
            result = ERR_MESSAGE_FAIL_E;
         }
      }
      else 
      {
         // The command came from the debug port on a RBU.  Apply the change locally
         if ( 0 == pCommand->Value || 1 == pCommand->Value )
         {
            CFG_SetGlobalDelayEnabled((bool)pCommand->Value);
            result = SUCCESS_E;
         }
      }
   }
   
   if ( SUCCESS_E == result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source); 
   }
   else
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessAlarmAcknowledge
*  Description:         Send the Alarm ACK to the system.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessAlarmAcknowledge( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if (gIsNCU )
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
         
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_ALARM_ACK_E;
            commandData.Destination = ADDRESS_GLOBAL;
            commandData.Parameter1 = 0;
            commandData.Parameter2 = (uint8_t)ZONE_GLOBAL;
            commandData.Value = pCommand->Value;
            commandData.ReadWrite = 1;/*WRITE*/
            commandData.SourceUSART = (uint16_t)source;
            
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
         }
      }
      else
      {
         result = DM_OP_AlarmAcknowledge((CO_OutputProfile_t)pCommand->Value);
//         result = ERR_OPERATION_FAIL_E;
//         //This is an RBU, action this locally for test purposes
//         ApplicationMessage_t appMessage;
//         appMessage.MessageType = APP_MSG_TYPE_COMMAND_E;
//         CO_CommandData_t command;
//         command.CommandType = PARAM_TYPE_ALARM_ACK_E;
//         command.TransactionID = GetNextTransactionID();
//         command.Parameter1 = 0;
//         command.Parameter2 = (uint8_t)ZONE_GLOBAL;
//         command.ReadWrite = 1;/*WRITE*/
//         command.Source = NCU_NETWORK_ADDRESS;
//         command.Value = 0;
//         memcpy(appMessage.MessageBuffer, &command, sizeof(CO_CommandData_t));
//         SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );

//         CO_Message_t *pCmdSignalInd = osPoolAlloc(AppPool);
//         if (pCmdSignalInd)
//         {
//            pCmdSignalInd->Type = CO_MESSAGE_PHY_DATA_IND_E;
//            uint8_t* pCmdInd = (uint8_t*)pCmdSignalInd->Payload.PhyDataInd.Data;
//            memcpy(pCmdInd, (uint8_t*)&appMessage, sizeof(ApplicationMessage_t));
//            pCmdSignalInd->Payload.PhyDataReq.Size = sizeof(ApplicationMessage_t);
//            
//            osStatus osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalInd, 0);
//            if (osOK != osStat)
//            {
//               /* failed to write */
//               osPoolFree(AppPool, pCmdSignalInd);
//            }
//            else 
//            {
//               result = SUCCESS_E;
//            }
//         }
      }
   }
   
   if ( SUCCESS_E == result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
   }
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessAlarmEvacuate
*  Description:         Send the Alarm Evacuate to the system.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessAlarmEvacuate( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( gIsNCU )
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
         // don't progress the command unless the NCU mesh state is 'active'
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_EVACUATE_E;
            commandData.Destination = ADDRESS_GLOBAL;
            commandData.Parameter1 = 0;
            commandData.Parameter2 = (uint8_t)ZONE_GLOBAL;
            commandData.Value = 0;
            commandData.ReadWrite = 1;/*WRITE*/
            commandData.SourceUSART = (uint16_t)source;
            
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
         }
      }
      else
      {
         //This is an RBU, action this locally for test purposes
         result = DM_OP_AlarmEvacuate();
         
//         CO_AlarmOutputStateData_t alarmOutput;
//         alarmOutput.Silenceable = DM_OP_BIT_EVACUATION;
//         alarmOutput.Unsilenceable = DM_OP_BIT_EVACUATION;
//         alarmOutput.Source = source;
//         /* create FireSignalReq message and put into mesh queue */
//         CO_Message_t *pCmdSignalReq = osPoolAlloc(AppPool);
//         if (pCmdSignalReq)
//         {
//            pCmdSignalReq->Type = CO_MESSAGE_GENERATE_ALARM_OUTPUT_STATE_E;
//            uint8_t* pCmdReq = (uint8_t*)pCmdSignalReq->Payload.PhyDataReq.Data;
//            memcpy(pCmdReq, (uint8_t*)&alarmOutput, sizeof(CO_AlarmOutputStateData_t));
//            pCmdSignalReq->Payload.PhyDataReq.Size = sizeof(CO_AlarmOutputStateData_t);
//            
//            osStatus osStat = osMessagePut(AppQ, (uint32_t)pCmdSignalReq, 0);
//            if (osOK != osStat)
//            {
//               /* failed to write */
//               osPoolFree(AppPool, pCmdSignalReq);
//            }
//            else 
//            {
//               MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
//               result = SUCCESS_E;
//            }
//         }
      }
   }
   
   if ( SUCCESS_E == result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
   }
   else
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessAlarmEnable
*  Description:         Send the enable/disable outputs command to the system.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessAlarmEnable( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      result = ERR_SYSTEM_NOT_ACTIVE_E;
      
      CO_CommandData_t commandData;
      commandData.TransactionID = GetNextTransactionID();
      commandData.Source = NCU_NETWORK_ADDRESS;
      commandData.CommandType = PARAM_TYPE_ENABLE_DISABLE_E;
      commandData.Destination = pCommand->NodeID;
      commandData.Parameter1 = pCommand->Parameter1;
      commandData.Parameter2 = pCommand->Parameter2;
      commandData.Value = pCommand->Value;
      commandData.ReadWrite = pCommand->ReadWrite;
      commandData.SourceUSART = (uint16_t)source;
      if ( CO_DISABLE_ZONE_E == commandData.Parameter1 )
      {
         commandData.Parameter2 = pCommand->Zone;
      }
      
      result = MM_CMD_ProcessLocalCommand( &commandData, source );
      if ( gIsNCU && (SUCCESS_E == result) )
      {
         if (STATE_ACTIVE_E == GetCurrentDeviceState())
         {
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
         }
      }
      
      if ( SUCCESS_E == result )
      {
         MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessAlarmReset
*  Description:         Send the Alarm Reset to the system.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessAlarmReset( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      result = ERR_SYSTEM_NOT_ACTIVE_E;
      
      //On the NCU we want to purge the downlink queues so that they don't 
      //interleave with the reset command and send stale signals.
      CO_QueueInitialise();
      
      if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
      {
         CO_CommandData_t commandData;
         commandData.TransactionID = GetNextTransactionID();
         commandData.Source = NCU_NETWORK_ADDRESS;
         commandData.CommandType = PARAM_TYPE_ALARM_RESET_E;
         commandData.Destination = pCommand->NodeID;
         commandData.Parameter1 = pCommand->Parameter1;
         commandData.Parameter2 = (uint8_t)ZONE_GLOBAL;
         commandData.Value = 0;
         commandData.ReadWrite = 1;/*WRITE*/
         commandData.SourceUSART = (uint16_t)source;
         
         CieBuffer_t msgBuffer;
         msgBuffer.MessageType = APP_MSG_TYPE_RESET_E;
         memcpy(&msgBuffer.Buffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
         
         result = MM_CIEQ_Push(CIE_Q_OUTPUT_SIGNAL_PRIORITY_E, &msgBuffer);
         
         if ( SUCCESS_E == result )
         {
            MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
         }
      }
      else if ( false == gIsNCU )      //If this is an RBU, action this locally for test purposes
      {

         ApplicationMessage_t appMessage;
         appMessage.MessageType = APP_MSG_TYPE_COMMAND_E;
         CO_CommandData_t command;
         command.CommandType = PARAM_TYPE_ALARM_RESET_E;
         command.TransactionID = GetNextTransactionID();
         command.Destination = gNetworkAddress;
         command.Parameter1 = pCommand->Parameter1;
         command.Parameter2 = (uint8_t)ZONE_GLOBAL;
         command.ReadWrite = 1;/*WRITE*/
         command.Source = NCU_NETWORK_ADDRESS;
         command.Value = 0;
         memcpy(appMessage.MessageBuffer, &command, sizeof(CO_CommandData_t));
         SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
         
         MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
         result = SUCCESS_E;
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessSoundLevel
*  Description:         Send the Sound Level to a node on the system.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessSoundLevel ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      result = ERR_SYSTEM_NOT_ACTIVE_E;
      
      if ( gIsNCU )
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_SOUND_LEVEL_E;
            commandData.Destination = pCommand->NodeID;
            commandData.Parameter1 = 0;
            commandData.Parameter2 = (uint8_t)ZONE_GLOBAL;
            commandData.Value = pCommand->Value;
            commandData.ReadWrite = pCommand->ReadWrite;
            commandData.SourceUSART = (uint16_t)source;
            
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
            
            if ( SUCCESS_E == result )
            {
               MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
            }
         }
      }
      else if ( gNetworkAddress == pCommand->NodeID )
      {
         //This is an RBU, action this locally for test purposes
         
         result = CFG_SetSoundLevel((uint8_t) pCommand->Value);
         if ( SUCCESS_E == result )
         {
            MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
         }
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}
/*****************************************************************************
*  Function:   MM_CMD_SendATFailWithErrorCode
*  Description:      Sends an error code back to the USART for failed AT command.
*                    The 'prefix' parameter is usually passed as NULL, but is useful for
*                    AT commands that are declared with the type AT_COMMAND_RESPONSE_E because
*                    the AT handler does not retain the command name for the response that
*                    is sent to the control panel.  The main case would be the ATQMC command
*                    which reads the MISC queue and uses the command name read from the
*                    message.  When the ATQMC command fails (e.g. the queue is empty) the
*                    "QMC: " prefix is supplied to this function so that the control panel
*                    knows which command failed.
*  Param - prefix:   Char string that you want to print before the error message.  Can be NULL.
*  Param - errorCode:The error that occurred.
*  Param - source:   The AT USART source of the command.  Responses should be
*                    sent back via the same link.
*  Returns:          None.
*  Notes:            
*****************************************************************************/
void MM_CMD_SendATFailWithErrorCode(const char* prefix, const ErrorCode_t errorCode, const CommandSource_t source )
{
   char response[CIE_QUEUE_DATA_SIZE];   
   char* msg_prefix = "";
   
   if ( prefix )
   {
      msg_prefix = (char*)prefix;
   }
   

   if ( ERR_NOT_FOUND_E == errorCode )
   {
      snprintf(response,CIE_QUEUE_DATA_SIZE,"%s%s,%s", msg_prefix, AT_RESPONSE_FAIL,  CIE_ERROR_CODE_STR[AT_CIE_ERROR_QUEUE_EMPTY_E]);
   }
   else if ( ERR_OUT_OF_RANGE_E == errorCode )
   {
      snprintf(response,CIE_QUEUE_DATA_SIZE,"%s%s,%s", msg_prefix, AT_RESPONSE_FAIL,  CIE_ERROR_CODE_STR[AT_CIE_ERROR_VALUE_OUT_OF_RANGE_E]);
   }
   else 
   {
      snprintf(response,CIE_QUEUE_DATA_SIZE,"%s%s,%s", msg_prefix, AT_RESPONSE_FAIL,  CIE_ERROR_CODE_STR[AT_CIE_ERROR_UNKNOWN_COMMAND_E]);
   }
   
   MM_CMD_SendATMessage(response, source);
}


/*****************************************************************************
*  Function:   MM_CMD_CommandExpectsResponse
*  Description:         Determines whether the supplied command should wait for a response.
*
*  Param - command      The command to be checked.
*  Param - destination  The node that the command is being sent to.
*  
*  Returns:             True if a response is expected.  False otherwise.
*
*  Notes:            
*****************************************************************************/
bool MM_CMD_CommandExpectsResponse( const ParameterType_t command, const uint16_t destination )
{
   bool response_expected = true; //default for most commands
   
   switch( command )
   {
      case PARAM_TYPE_DAY_NIGHT_SETTING_E: //intentional drop-through
      case PARAM_TYPE_ALARM_ACK_E:
      case PARAM_TYPE_EVACUATE_E:
      case PARAM_TYPE_GLOBAL_DELAY_E:
      case PARAM_TYPE_ENABLE_DISABLE_E:
      case PARAM_TYPE_ALARM_RESET_E:
      case PARAM_TYPE_TEST_ONE_SHOT_E:
      case PARAM_TYPE_CONFIRMED_E:
      case PARAM_TYPE_200_HOUR_TEST_E:
      case PARAM_TYPE_CHECK_FIRMWARE_E:
      case PARAM_TYPE_SET_LOCAL_GLOBAL_DELAYS_E:
      case PARAM_TYPE_GLOBAL_ALARM_DELAY_VALUES_E:
      case PARAM_TYPE_GLOBAL_ALARM_OVERRIDE_E:
      case PARAM_TYPE_BATTERY_STATUS_REQUEST_E:
      case PARAM_TYPE_GLOBAL_DELAY_COMBINED_E:
      case PARAM_TYPE_SCAN_FOR_DEVICES_E:
      case PARAM_TYPE_REBOOT_E:
      case PARAM_TYPE_SENSOR_VALUES_REQUEST_E:
      case PARAM_TYPE_DELAYED_OUTPUT_E:
      case PARAM_TYPE_BATTERY_TEST_E:
      case PARAM_TYPE_ENTER_PPU_MODE_E:
      case PARAM_TYPE_EXIT_TEST_MODE_E: 
         response_expected = false;
         break;
      case PARAM_TYPE_PPU_MODE_ENABLE_E:
      case PARAM_TYPE_DEPASSIVATION_SETTINGS_E:
         if ( ADDRESS_GLOBAL == destination )
         {
            response_expected = false;
         }
      default:
         break;
   }
   
   return response_expected;
}

/*****************************************************************************
*  Function:   MM_CMD_AlarmReset
*  Description:         Carry out a commanded reset from the control panel.
*                       Outputs are set back to their start-up state.
*                       Inputs are checked for alarm conditions and Alarm
*                       Signals are sent if an input is in an alrm state.
*
*  Param - None
*  
*  Returns:             SUCCESS_E or error code
*
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_AlarmReset( int flags )
{
   ErrorCode_t result = ERR_NO_MEMORY_E;
   
   CO_PRINT_B_0(DBG_INFO_E, "Received Panel Reset event\r\n");
   
   /* Send a 'Panel Reset' event to the application */
   CO_Message_t *pEventMsg = NULL;

   /* create message and put into mesh queue */
   pEventMsg = osPoolAlloc(AppPool);
   if (pEventMsg)
   {
      pEventMsg->Type = CO_MESSAGE_RESET_EVENT_E;
		*(int*)pEventMsg->Payload.PhyDataReq.Data = flags;
		
      if ( osOK == osMessagePut(AppQ, (uint32_t)pEventMsg, 0) )
      {
         result = SUCCESS_E;
      }
      else 
      {
         /* failed to send */
         osPoolFree(AppPool, pEventMsg);
         result = ERR_QUEUE_OVERFLOW_E;
      }
   }
   
   return result;
}


void MM_ApplicationSendAlarmSignals( void );
void DM_OP_RefreshOutputStatus( void );
void MM_ApplicationSendFaultReports( const bool sendOnDULCH );

/*****************************************************************************
*  Function:   DM_OP_SetEnableDisable
*  Description:         Handles the enable and disable command for channels,
*                       zones or devices..
*
*  Param - region             Device, Zone or channel
*  Param - channel_or_zone    The channel or zone number
*  Param - enable_disable     day enable=0x01, night=0x02, both=0x03
*  
*  Returns:             SUCCESS_E or error code
*
*  Notes:            
*****************************************************************************/
ErrorCode_t DM_OP_SetEnableDisable(const CO_DisableRegion_t region, const uint16_t channel_or_zone, const uint32_t enable_disable)
{
   ErrorCode_t result = ERR_OUT_OF_RANGE_E;
   DM_Enable_t channel_enable;
   bool zone_or_device_enabled = ((uint32_t)DM_ENABLE_NONE_E != enable_disable);
	
	CO_PRINT_B_0(DBG_INFO_E, "Received enable/disable.. \r\n");
   
   if ( DM_ENABLE_MAX_E > enable_disable )
   {
      channel_enable = (DM_Enable_t)enable_disable;
      
      switch ( region )
      {
         case CO_DISABLE_CHANNEL_E:
            if ( CO_IsOutputChannel(channel_or_zone) )
            {
               result = DM_OP_SetDisablement(channel_or_zone, channel_enable);
               DM_SetChannelEnabled((CO_ChannelIndex_t)channel_or_zone, channel_enable);
               result = SUCCESS_E;
            }
            else if ( CO_IsInputChannel(channel_or_zone) )
            {
               DM_SetChannelEnabled((CO_ChannelIndex_t)channel_or_zone, channel_enable);
               result = SUCCESS_E;
            }
				// if possible channel enablement
				if ( enable_disable != DM_ENABLE_NONE_E )
				{
					MM_ApplicationSendFaultReports( false );
				}
            break;
         
         case CO_DISABLE_DEVICE_E://Intentional drop-through
         case CO_DISABLE_ALL_E:
            DM_SetRbuEnabled((bool)zone_or_device_enabled);
			
				// if device enablement
				if ( enable_disable != DM_ENABLE_NONE_E )
				{
					MM_ApplicationSendFaultReports( false );
				}
			
            result = SUCCESS_E;
            break;
         
         case CO_DISABLE_ZONE_E:
            if ( channel_or_zone == CFG_GetZoneNumber() )
            {
               CFG_SetZoneEnabled(zone_or_device_enabled);
					
					// if zone enablement
					if ( enable_disable != DM_ENABLE_NONE_E )
					{
						// dont re-send faults on DULCH
						MM_ApplicationSendFaultReports( false );
					}
            }
            CFG_UpdateZoneMap(channel_or_zone, (bool)zone_or_device_enabled);
            result = SUCCESS_E;
            break;
         default:
         break;
      }
   }

	// Refresh Input/Output status..
	
	// if somethings may have been enabled
	if ( enable_disable != DM_ENABLE_NONE_E )
	{		
		//Send any existing alarm states
		MM_ApplicationSendAlarmSignals();
	}
	
	// if disabled
	if ( !CFG_ZoneEnabled() || !DM_RbuEnabled() )		 		         
	{
		DM_OP_Initialise(false);
		//Remove the LED patterns for FIRE and FIRST AID
		DM_LedPatternRemove(LED_FIRE_INDICATION_E);
		DM_LedPatternRemove(LED_FIRST_AID_INDICATION_E);
	}
	else
	{
		DM_OP_RestoreAlarmOutputState( );
	}
		
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessTestOneShot
*  Description:         Command a one-shot alarm test.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessTestOneShot( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
      {
         if ( COMMAND_WRITE == pCommand->ReadWrite )
         {
            if ( (0 == pCommand->Value) || (1 == pCommand->Value) )
            {
               CO_CommandData_t commandData;
               commandData.TransactionID = GetNextTransactionID();
               commandData.Source = NCU_NETWORK_ADDRESS;
               commandData.CommandType = PARAM_TYPE_TEST_ONE_SHOT_E;
               commandData.Destination = ADDRESS_GLOBAL;
               commandData.Parameter1 = 0;
               commandData.Parameter2 = (uint8_t)ZONE_GLOBAL;
               commandData.Value = pCommand->Value;
               commandData.ReadWrite = 1;/*WRITE*/
               commandData.SourceUSART = (uint16_t)source;
               
               CieBuffer_t msgBuffer;
               msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
               memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
               
               result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
               
               if ( SUCCESS_E == result )
               {
                  MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
               }
            }
            else 
            {
               result = ERR_OUT_OF_RANGE_E;
            }
         }
      }
      else 
      {
         result = ERR_MESSAGE_FAIL_E;
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessTestOneShot
*  Description:         Send a 'fire confirmed' signal.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessConfirm ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( (0 == pCommand->Value) || (1 == pCommand->Value) )
      {
         if ( gIsNCU )
         {
            if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
            {
               CO_CommandData_t commandData;
               commandData.TransactionID = GetNextTransactionID();
               commandData.Source = NCU_NETWORK_ADDRESS;
               commandData.CommandType = PARAM_TYPE_CONFIRMED_E;
               commandData.Destination = ADDRESS_GLOBAL;
               commandData.Parameter1 = 0;
               commandData.Parameter2 = (uint8_t)ZONE_GLOBAL;
               commandData.Value = pCommand->Value;
               commandData.ReadWrite = 1;/*WRITE*/
               commandData.SourceUSART = (uint16_t)source;
               
               CieBuffer_t msgBuffer;
               msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
               memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
               
               result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
               
               if ( SUCCESS_E == result )
               {
                  MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
               }
            }
            else 
            {
               result = ERR_SYSTEM_NOT_ACTIVE_E;
            }
         }
         else 
         {
            //RBU.  Apply the command locally.
            result = DM_OP_Confirmed((CO_OutputProfile_t)pCommand->Value);
         }
      }
      else 
      {
         result = ERR_OUT_OF_RANGE_E;
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessAlarmConfiguration
*  Description:         Read/write the alarm configuration bitfield for a remote RBU.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessAlarmConfiguration( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( gIsNCU )
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
         
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_ALARM_CONFIG_E;
            commandData.Destination = pCommand->NodeID;
            commandData.Parameter1 = pCommand->Parameter1;//channel
            commandData.Parameter2 = pCommand->Parameter2;
            commandData.Value = pCommand->Value;//bit field
            commandData.ReadWrite = pCommand->ReadWrite;
            commandData.SourceUSART = (uint16_t)source;
            
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
            
            if ( SUCCESS_E == result )
            {
               MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
            }
         }
         else 
         {
            result = ERR_OUT_OF_RANGE_E;
         }
      }
      else 
      {
         result = ERR_MESSAGE_FAIL_E;
      }
   }
   else 
   {
      // The command came from the debug port on a RBU.  Apply the change locally
      if ( (0 == pCommand->Value) || (1 == pCommand->Value) )
      {
         DM_OP_SetAlarmConfiguration((CO_ChannelIndex_t)pCommand->Parameter1, pCommand->Value);
         result = SUCCESS_E;
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessAlarmOptionFlags
*  Description:         Read/write the alarm options bit filed for a remote RBU.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessAlarmOptionFlags( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( gIsNCU )
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
         
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_ALARM_OPTION_FLAGS_E;
            commandData.Destination = pCommand->NodeID;
            commandData.Parameter1 = pCommand->Parameter1;//channel
            commandData.Parameter2 = pCommand->Parameter2;
            commandData.Value = pCommand->Value;//bit field
            commandData.ReadWrite = pCommand->ReadWrite;
            commandData.SourceUSART = (uint16_t)source;
            
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
            
            if ( SUCCESS_E == result )
            {
               MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
            }
         }
         else 
         {
            result = ERR_OUT_OF_RANGE_E;
         }
      }
      else 
      {
         result = ERR_MESSAGE_FAIL_E;
      }
   }
   else 
   {
      // The command came from the debug port on a RBU.  Apply the change locally
      if ( (0 == pCommand->Value) || (1 == pCommand->Value) )
      {
         DM_OP_SetAlarmConfiguration((CO_ChannelIndex_t)pCommand->Parameter1, pCommand->Value);
         result = SUCCESS_E;
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessAlarmDelays
*  Description:         Read/write the alarm delays 1 and 2 for a remote RBU.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessAlarmDelays( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( gIsNCU )
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
         
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_ALARM_DELAYS_E;
            commandData.Destination = pCommand->NodeID;
            commandData.Parameter1 = pCommand->Parameter1;//channel
            commandData.Parameter2 = pCommand->Parameter2;
            commandData.Value = pCommand->Value;//bit field
            commandData.ReadWrite = pCommand->ReadWrite;
            commandData.SourceUSART = (uint16_t)source;
            
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
            
            if ( SUCCESS_E == result )
            {
               MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
            }
         }
         else 
         {
            result = ERR_OUT_OF_RANGE_E;
         }
      }
      else 
      {
         result = ERR_MESSAGE_FAIL_E;
      }
   }
   else 
   {
      // The command came from the debug port on a RBU.  Apply the change locally
      if ( (0 == pCommand->Value) || (1 == pCommand->Value) )
      {
         DM_OP_SetAlarmDelays((CO_ChannelIndex_t)pCommand->Parameter1, pCommand->Value);
         result = SUCCESS_E;
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessCIEChannelFlagsCommand
*  Description:         Send the channel flags command to the system.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCIEChannelFlagsCommand( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( CO_CHANNEL_MAX_E > pCommand->Parameter1 )
      {
         CO_ChannelIndex_t channel = (CO_ChannelIndex_t)pCommand->Parameter1;
         if ( gNetworkAddress == pCommand->NodeID )
         {
            if ( COMMAND_READ == pCommand->ReadWrite )
            {
               //Read the option flags
               uint32_t option_flags = DM_OP_GetChannelFlags(channel);
               //Extract the channel flags from bits 7,8,9,30,31.
               uint32_t channel_flags = (option_flags & 0x380) >> 7;
               channel_flags |= (( option_flags & 0xC0000000) >> 27);

               if ( NCU_NETWORK_ADDRESS == gNetworkAddress)
               {
                  /* NCU Local node - Put result in CIE misc queue*/
                  CieBuffer_t cieMsg;
                  snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "%d,%d", pCommand->Parameter1, channel_flags);
                  /*push the message into the CIE misc queue */
                  result = MM_CIEQ_Push(CIE_Q_MISC_E, &cieMsg);
                  if ( SUCCESS_E == result )
                  {
                     MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
                  }
               }
               else 
               {
                  /* RBU Local node - Send result to UART*/
                  char response[AT_RESPONSE_MAX_LENGTH];
                  snprintf((char*)response, AT_RESPONSE_MAX_LENGTH, "%d,%d", pCommand->Parameter1, channel_flags);
                  result = SUCCESS_E;
                  MM_CMD_SendATMessage(response, source);
               }
            }
            else //COMMAND_WRITE
            {
               //Write the day/night enable flags
               DM_Enable_t enableDisable = DM_ENABLE_NONE_E;
               uint32_t option_flags = pCommand->Value;
               if ( (DM_CHANNEL_DAY_ENABLE_MASK == (DM_CHANNEL_DAY_ENABLE_MASK & option_flags)) &&
                    (DM_CHANNEL_NIGHT_ENABLE_MASK == (DM_CHANNEL_NIGHT_ENABLE_MASK & option_flags)) )
               {
                  enableDisable = DM_ENABLE_DAY_AND_NIGHT_E;
               }
               else if ( DM_CHANNEL_DAY_ENABLE_MASK == (DM_CHANNEL_DAY_ENABLE_MASK & option_flags) )
               {
                  enableDisable = DM_ENABLE_DAY_E;
               }
               else if ( DM_CHANNEL_NIGHT_ENABLE_MASK == (DM_CHANNEL_NIGHT_ENABLE_MASK & option_flags) )
               {
                  enableDisable = DM_ENABLE_NIGHT_E;
               }
               
               result = DM_OP_SetDisablement(channel, enableDisable);
               
               //Write the alarm option flags
               if ( SUCCESS_E == result )
               {
                  uint32_t alarm_option_flags = option_flags & DM_ALARM_OPTION_FLAGS_MASK;
                  result = DM_OP_SetAlarmOptionFlags(channel, alarm_option_flags);
               }
               
               if ( SUCCESS_E == result )
               {
                  MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
               }
            }
         }
         else 
         {
            // The command is for a remote device.  Send it over the mesh.
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_CHANNEL_FLAGS_E;
            commandData.Destination = pCommand->NodeID;
            commandData.Parameter1 = pCommand->Parameter1;
            commandData.Parameter2 = pCommand->Parameter2;
            commandData.Value = pCommand->Value;
            commandData.ReadWrite = pCommand->ReadWrite;
            commandData.SourceUSART = (uint16_t)source;
            result = ERR_SYSTEM_NOT_ACTIVE_E;
            
            if (STATE_ACTIVE_E == GetCurrentDeviceState())
            {
               CieBuffer_t msgBuffer;
               msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
               memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
               
               result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
               
               if ( SUCCESS_E == result )
               {
                  MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
               }
            }
         }
      }
      else 
      {
         result = ERR_OUT_OF_RANGE_E;
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessChannelFlags
*  Description:      Process the channel config flags command.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessChannelFlags ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   char error_response[AT_RESPONSE_MAX_LENGTH];
   
   if ( pCommand )
   {
      /* create a command for the MAC */
      CO_ResponseData_t response;
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      if ( CO_CHANNEL_MAX_E > pCommand->Parameter1 )
      {
         CO_ChannelIndex_t channel = (CO_ChannelIndex_t)pCommand->Parameter1;
         if ( COMMAND_READ == pCommand->ReadWrite )
         {
            uint32_t option_flags;
            result = DM_OP_GetAlarmOptionFlags(channel, &option_flags);
            if ( SUCCESS_E == result )
            {
               DM_Enable_t enableDisable;
               result = DM_OP_GetDisablement(channel, &enableDisable);
               if ( SUCCESS_E == result )
               {
                  switch ( enableDisable )
                  {
                     case DM_ENABLE_DAY_E:
                        option_flags |= DM_CHANNEL_DAY_ENABLE_MASK;
                        break;
                     case DM_ENABLE_NIGHT_E:
                        option_flags |= DM_CHANNEL_NIGHT_ENABLE_MASK;
                        break;
                     case DM_ENABLE_DAY_AND_NIGHT_E:
                        option_flags |= DM_CHANNEL_DAY_ENABLE_MASK | DM_CHANNEL_NIGHT_ENABLE_MASK;
                        break;
                     default:
                        break;
                  }
                  
                  response.Value = option_flags;
               }
               else 
               {
                  snprintf(error_response, AT_RESPONSE_MAX_LENGTH,"ERROR,READ DISABLEMENT FAILED");
               }
            }
            else 
            {
               snprintf(error_response, AT_RESPONSE_MAX_LENGTH,"ERROR,READ AOF FAILED");
            }
         }
         else //COMMAND_WRITE
         {
            //Write the day/night enable flags
            DM_Enable_t enableDisable = DM_ENABLE_NONE_E;
            uint32_t option_flags = pCommand->Value;
            if ( (DM_CHANNEL_DAY_ENABLE_MASK == (DM_CHANNEL_DAY_ENABLE_MASK & option_flags)) &&
                 (DM_CHANNEL_NIGHT_ENABLE_MASK == (DM_CHANNEL_NIGHT_ENABLE_MASK & option_flags)) )
            {
               enableDisable = DM_ENABLE_DAY_AND_NIGHT_E;
            }
            else if ( DM_CHANNEL_DAY_ENABLE_MASK == (DM_CHANNEL_DAY_ENABLE_MASK & option_flags) )
            {
               enableDisable = DM_ENABLE_DAY_E;
            }
            else if ( DM_CHANNEL_NIGHT_ENABLE_MASK == (DM_CHANNEL_NIGHT_ENABLE_MASK & option_flags) )
            {
               enableDisable = DM_ENABLE_NIGHT_E;
            }
            
            result = DM_OP_SetDisablement(channel, enableDisable);
            
            //Write the alarm option flags
            if ( SUCCESS_E == result )
            {
               uint32_t alarm_option_flags = option_flags & DM_ALARM_OPTION_FLAGS_MASK;
               result = DM_OP_SetAlarmOptionFlags(channel, alarm_option_flags);
               if ( SUCCESS_E != result )
               {
                  snprintf(error_response, AT_RESPONSE_MAX_LENGTH,"ERROR,AOF OPERATION FAILED");
               }
            }
            else 
            {
               snprintf(error_response, AT_RESPONSE_MAX_LENGTH,"ERROR,DISC OPERATION FAILED");
            }
         }
      }
      else 
      {
         result = ERR_OUT_OF_RANGE_E;
         snprintf(error_response, AT_RESPONSE_MAX_LENGTH,"ERROR,BAD CHANNEL");
      }
      
      if ( SUCCESS_E == result )
      {
         /* Send the response back to the command's originator */
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            if ( SUCCESS_E == result )
            {
               /* respond over AT USART link */ 
               MM_CMD_SendATResponse( &response, source );
            }
            else 
            {
               MM_CMD_SendATMessage(error_response, source);
            }
         }
         else
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
      }
   }
   else
   {
      result = ERR_MESSAGE_FAIL_E;
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessMaxRankCommand
*  Description:      Process the Max Rank command.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessMaxRankCommand( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( gIsNCU )
      {
         result = ERR_SYSTEM_NOT_ACTIVE_E;
         
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_MAX_RANK_E;
            commandData.Destination = pCommand->NodeID;
            commandData.Parameter1 = pCommand->Parameter1;
            commandData.Parameter2 = pCommand->Parameter2;
            commandData.Value = pCommand->Value;//rank
            commandData.ReadWrite = pCommand->ReadWrite;
            commandData.SourceUSART = (uint16_t)source;
            
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
            result = SUCCESS_E;
         }
         else 
         {
            result = ERR_OUT_OF_RANGE_E;
         }
      }
      
      
      // Apply the change locally
      if ( (pCommand->NodeID == gNetworkAddress) || (ADDRESS_GLOBAL == pCommand->NodeID) )
      {
         result = ERR_OPERATION_FAIL_E;
         if ( COMMAND_READ == pCommand->ReadWrite )
         {
            uint32_t rank = MC_GetMaxRank();
            char response[AT_RESPONSE_MAX_LENGTH];
            snprintf(response, AT_RESPONSE_MAX_LENGTH,"%d", rank);
            MM_CMD_SendATMessage(response, source);
            result = SUCCESS_E;
         }
         else 
         {
            if( SUCCESS_E == MC_SetMaxRank(pCommand->Value) )
            {
               result = SUCCESS_E;
               MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
            }
         }
      }
      else if ( SUCCESS_E == result )
      {
         MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessSoundLevel
*  Description:         Send the Sound Level to a node on the system.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_Process200HourTest ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      result = ERR_SYSTEM_NOT_ACTIVE_E;
      
      if ( gIsNCU && STATE_ACTIVE_E == GetCurrentDeviceState() )
      {
         CO_CommandData_t commandData;
         commandData.TransactionID = GetNextTransactionID();
         commandData.Source = NCU_NETWORK_ADDRESS;
         commandData.CommandType = PARAM_TYPE_200_HOUR_TEST_E;
         commandData.Destination = pCommand->NodeID;
         commandData.Parameter1 = pCommand->Parameter1;
         commandData.Parameter2 = pCommand->Parameter2;
         commandData.Value = pCommand->Value;
         commandData.ReadWrite = pCommand->ReadWrite;
         commandData.SourceUSART = (uint16_t)source;
         
         CieBuffer_t msgBuffer;
         msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
         memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
         
         result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
         
         if ( SUCCESS_E == result )
         {
            MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
         }
      }
      else if ( false == gIsNCU )      //If this is an RBU, action this locally
      {
         CO_ChannelIndex_t channel = CO_CHANNEL_BEACON_E;//default test device
         if ( CO_CHANNEL_MAX_E > pCommand->Parameter1 )
         {
            channel = (CO_ChannelIndex_t)pCommand->Parameter1;
         }
         TE_200HourTestEnable( (bool)pCommand->Value, channel );
         
         MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
         result = SUCCESS_E;
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessCheckFirmwareRequest
*  Description:         Broadcast a command for all radio boards to check their firmware.
*                       version against the payload and issue a fault if different.
*  Param - pCommand     The command to check the firmware.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCheckFirmwareRequest ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      result = ERR_NOT_SUPPORTED_E;
      
      if ( ADDRESS_NCU == gNetworkAddress )
      {
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_CHECK_FIRMWARE_E;
            commandData.Destination = ADDRESS_GLOBAL;
            commandData.Parameter1 = 0;
            commandData.Parameter2 = (uint8_t)ZONE_GLOBAL;
            commandData.Value = gSoftwareVersionNumber;
            commandData.ReadWrite = 1;/*WRITE*/
            commandData.SourceUSART = (uint16_t)source;
            
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
         }
         else 
         {
            result = ERR_SYSTEM_NOT_ACTIVE_E;
         }
      }
      
      if ( SUCCESS_E == result )
      {
         MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_CheckFirmware
*  Description:         Compare the current version of firmware to the supplied
*                       version and send a fault if different.
*
*  Param - firmware_version  The version to compare against
*  
*  Returns:             SUCCESS_E or error code
*
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_CheckFirmware(const uint32_t firmware_version)
{
   ErrorCode_t result = SUCCESS_E;
   
   CO_PRINT_B_0(DBG_INFO_E, "Firmware check requested\r\n");
   
   //if the current firmware version doesn't match the parameter, send a fault to the NCU
   if ( gSoftwareVersionNumber != firmware_version )
   {
      //Send fault signal via DULCH.  There is no fault status flag for firmware mismatch so we must set 'force send'
      CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_FIRMWARE_VERSION_E, 1, true, true);
      CO_PRINT_B_2(DBG_INFO_E,"Firmware mismatch: installed=%d, verification=%d\r\n", gSoftwareVersionNumber, firmware_version);
   }
   else
   {
      CO_PRINT_B_2(DBG_INFO_E,"Firmware OK: installed=%d, verification=%d\r\n", gSoftwareVersionNumber, firmware_version);
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessLocalOrGlobalDelaySetting
*  Description:         Read/write the Local/Global delays setting of the system.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessLocalOrGlobalDelaySetting( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         bool globalLocal = DM_OP_GetLocalOrGlobalDelayFlag((CO_ChannelIndex_t)pCommand->Parameter1);
         char buffer[AT_RESPONSE_MAX_LENGTH];
         snprintf(buffer, AT_RESPONSE_MAX_LENGTH, "%d",(uint32_t)globalLocal);
         if ( MM_CMD_SendATMessage( buffer, source ) )
         {
            result = SUCCESS_E;
         }
      }
      else
      {
         if ( (uint32_t)CO_DAY_E == pCommand->Value || (uint32_t)CO_NIGHT_E == pCommand->Value )
         {
            DM_OP_SetLocalOrGlobalDelayFlag((CO_ChannelIndex_t)pCommand->Parameter1, pCommand->Value);
            
            if ( ADDRESS_NCU == gNetworkAddress )
            {
               if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
               {
                  CO_CommandData_t commandData;
                  commandData.TransactionID = GetNextTransactionID();
                  commandData.Source = NCU_NETWORK_ADDRESS;
                  commandData.CommandType = PARAM_TYPE_SET_LOCAL_GLOBAL_DELAYS_E;
                  commandData.Destination = pCommand->NodeID;
                  commandData.Parameter1 = pCommand->Parameter1;
                  commandData.Parameter2 = pCommand->Parameter2;
                  commandData.Value = pCommand->Value;
                  commandData.ReadWrite = pCommand->ReadWrite;
                  commandData.SourceUSART = (uint16_t)source;
                  
                  CieBuffer_t msgBuffer;
                  msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
                  memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
                  
                  result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
               }
               else 
               {
                  result = ERR_SYSTEM_NOT_ACTIVE_E;
               }
            }
            else 
            {
               result = SUCCESS_E;
            }
            
            if ( SUCCESS_E == result )
            {
               MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
            }
         }
         else 
         {
            result = ERR_OUT_OF_RANGE_E;
         }
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessGlobalDelayValues
*  Description:         Read/write the Local/Global delays setting of the system.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessGlobalDelayValues( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         char buffer[AT_RESPONSE_MAX_LENGTH];
         uint16_t delay1 = CFG_GetGlobalDelay1();
         uint16_t delay2 = CFG_GetGlobalDelay2();
         snprintf(buffer, AT_RESPONSE_MAX_LENGTH, "%d,%d",delay1, delay2);
         
         if ( MM_CMD_SendATMessage( buffer, source ) )
         {
            result = SUCCESS_E;
         }
      }
      else
      {
         result = DM_OP_SetGlobalDelayValues(pCommand->Value);
         
         if ( ADDRESS_NCU == gNetworkAddress )
         {
            if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
            {
               CO_CommandData_t commandData;
               commandData.TransactionID = GetNextTransactionID();
               commandData.Source = NCU_NETWORK_ADDRESS;
               commandData.CommandType = PARAM_TYPE_GLOBAL_ALARM_DELAY_VALUES_E;
               commandData.Destination = pCommand->NodeID;
               commandData.Parameter1 = pCommand->Parameter1;
               commandData.Parameter2 = pCommand->Parameter2;
               commandData.Value = pCommand->Value;
               commandData.ReadWrite = pCommand->ReadWrite;
               commandData.SourceUSART = (uint16_t)source;
               
               CieBuffer_t msgBuffer;
               msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
               memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
               
               result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
            }
            else 
            {
               result = ERR_SYSTEM_NOT_ACTIVE_E;
            }
         }
         else 
         {
            result = SUCCESS_E;
         }
         
         if ( SUCCESS_E == result )
         {
            MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
         }
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessGlobalDelayOverride
*  Description:         Read/write the Global Delay Override setting of the system.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessGlobalDelayOverride( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   uint32_t override;

   if ( pCommand )
   {
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         override = CFG_GetGlobalDelayOverride();
         char buffer[AT_RESPONSE_MAX_LENGTH];
         snprintf(buffer, AT_RESPONSE_MAX_LENGTH, "%d", override);
         if ( MM_CMD_SendATMessage( buffer, source ) )
         {
            result = SUCCESS_E;
         }
      }
      else
      {
         override = pCommand->Value;
         //If the value is non-zero, trim it to '1'.
         if ( 1 < override )
         {
            override = 1;
         }
         
         CFG_SetGlobalDelayOverride( ( bool)override );
         
         if ( ADDRESS_NCU == gNetworkAddress )
         {
            if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
            {
               CO_CommandData_t commandData;
               commandData.TransactionID = GetNextTransactionID();
               commandData.Source = NCU_NETWORK_ADDRESS;
               commandData.CommandType = PARAM_TYPE_GLOBAL_ALARM_OVERRIDE_E;
               commandData.Destination = pCommand->NodeID;
               commandData.Parameter1 = pCommand->Parameter1;
               commandData.Parameter2 = pCommand->Parameter2;
               commandData.Value = override;
               commandData.ReadWrite = pCommand->ReadWrite;
               commandData.SourceUSART = (uint16_t)source;
               
               CieBuffer_t msgBuffer;
               msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
               memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
               
               result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
            }
            else 
            {
               result = ERR_SYSTEM_NOT_ACTIVE_E;
            }
         }
         else 
         {
            result = SUCCESS_E;
         }
         
         if ( SUCCESS_E == result )
         {
            MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
         }
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessDayNightSetting
*  Description:         Process mesh command to Read/write the Day/Night setting of the system.
*                       A side effect of day/night is the enablement of channels.  The channel
*                       enablement is managed in the Application code, but the devices are
*                       managed by either the Application code, the GPIO code or the Head code.
*                       On change of day/night the channel enablement needs to be updated.
*                       For GPIO inputs, changes to the inputs are always fed to the Application
*                       and are filtered out if the channel is disabled.  Channels that are on
*                       a plug-in must be sent enable/disable commands.
*
*  Param - dayNight     The new Day/Night setting.
*
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessDayNightSetting( const CO_DayNight_t dayNight )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   bool setting_updated = false;
   
   //Update the day/night setting i the device config
   if ( CO_NUM_DAYNIGHT_SETTINGS_E > dayNight )
   {
      CO_DayNight_t current_setting = CFG_GetDayNight();
      if ( current_setting != dayNight )
      {
         CFG_SetDayNight(dayNight);
         setting_updated = true;
      }
      result = SUCCESS_E;
   }
   else 
   {
      result = ERR_OUT_OF_RANGE_E;
   }

   //If the command was valid and the setting changed, update the channel enablement
   if ( (SUCCESS_E == result) && setting_updated )
   {
      //Create a head command for plug-in channel enablements
      HeadMessage_t head_message;
      head_message.MessageType = HEAD_WRITE_ENABLED_E;
      //Itterate through the channels and send command if they are enabled
      for ( uint32_t channel = 0; channel < CO_CHANNEL_MAX_E; channel++ )
      {
         //Check if the channel config is set to disabled
         if ( DM_OP_ChannelDisabled((CO_ChannelIndex_t)channel) )
         {
            //channel disabled
            head_message.Value = 0;
         }
         else 
         {
            //channel enabled
            head_message.Value = 1;
         }
         
         //check and update the channel
         switch ( channel )
         {
            case CO_CHANNEL_SMOKE_E:
               if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SMOKE_DETECTOR, DC_MATCH_ANY_E) )
               {
                  head_message.ChannelNumber = DEV_SMOKE_DETECTOR_HEAD_E;
                  MM_CMD_SendHeadMessage( CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E, &head_message, CMD_SOURCE_APPLICATION_E );
               }
               break;
            case CO_CHANNEL_HEAT_B_E:
//               if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAT_DETECTOR_TYPE_B, DC_MATCH_ANY_E) )
//               {
//                  head_message.ChannelNumber = DEV_HEAT_DETECTOR_HEAD_E;
//                  MM_CMD_SendHeadMessage( CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E, &head_message, CMD_SOURCE_APPLICATION_E );
//               }
               result = SUCCESS_E;
               break;
            case CO_CHANNEL_HEAT_A1R_E:
//               if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAT_DETECTOR_TYPE_A1R, DC_MATCH_ANY_E) )
//               {
//                  head_message.ChannelNumber = DEV_HEAT_DETECTOR_HEAD_E;
//                  MM_CMD_SendHeadMessage( CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E, &head_message, CMD_SOURCE_APPLICATION_E );
//               }
               result = SUCCESS_E;
               break;
            case CO_CHANNEL_PIR_E:
               //only the RBU has a PIR on a plug-in
               if ( BASE_RBU_E == gBaseType )
               {
                  if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_PIR_DETECTOR, DC_MATCH_ANY_E) )
                  {
                     head_message.ChannelNumber = DEV_PIR_DETECTOR_HEAD_E;
                     if ( MM_CMD_SendHeadMessage( CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E, &head_message, CMD_SOURCE_APPLICATION_E ) )
                     {
                        result = SUCCESS_E;
                     }
                     else 
                     {
                        result = ERR_OPERATION_FAIL_E;
                     }
                  }
               }
               break;
            default:
               //do nothing
               break;
         }
      }
      //Send the setting to the head if one is fitted.
      uint32_t deviceCombination = CFG_GetDeviceCombination();
      if (true == DM_DeviceIsEnabled(deviceCombination, DEV_HEAD_DEVICES, DC_MATCH_ANY_E ))
      {
         CO_DayNight_t dayNight = CFG_GetDayNight();
         HeadMessage_t headMessage;
         headMessage.MessageType = HEAD_DAY_NIGHT_SETTING_E;
         headMessage.Value = dayNight;
         MM_CMD_SendHeadMessage( CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E, &headMessage, CMD_SOURCE_APPLICATION_E );
      }
   }
   
   
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessBatteryStatusRequest
*  Description:         Request a battery status message from an RBU.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessBatteryStatusRequest ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      //If the command was a local request to send the battery voltages to the debug log the command Value parameter will be '1'
      if ( 0 == pCommand->Value )
      {
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_BATTERY_STATUS_REQUEST_E;
            commandData.Destination = pCommand->NodeID;
            commandData.Parameter1 = 0;
            commandData.Parameter2 = 0;
            commandData.Value = pCommand->Value;
            commandData.ReadWrite = 0;
            commandData.SourceUSART = (uint16_t)source;
            
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
         }
         else 
         {
            result = ERR_SYSTEM_NOT_ACTIVE_E;
         }
      }
      else 
      {
         uint32_t primary_voltage_mv = DM_BAT_GetPrimaryBatteryVoltage();
         uint32_t backup_voltage_mv = DM_BAT_GetBackupBatteryVoltage();
         char buffer[AT_RESPONSE_MAX_LENGTH];
         snprintf(buffer, AT_RESPONSE_MAX_LENGTH, "Prim=%dmV Bkp=%dmV", primary_voltage_mv, backup_voltage_mv);
         if ( MM_CMD_SendATMessage( buffer, source ) )
         {
            result = SUCCESS_E;
         }

         result = SUCCESS_E;
      }
   }
   
   if ( SUCCESS_E == result )
   {    
      /* respond over AT USART link */
      MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
   }
   else 
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessGlobalDelayCombined
*  Description:         Read/write the Global Delay Override setting of the system.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessGlobalDelayCombined( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   bool delays_enabled = false;
   bool global_override = false;
   uint32_t global_delay1 = 0;
   uint32_t global_delay2 = 0;
   uint32_t combined_global_delays = 0;
   bool is_day = false;
   bool ignore_security_in_day = false;
   bool ignore_security_at_night = false;

   if ( pCommand )
   {
      if ( COMMAND_WRITE == pCommand->ReadWrite )
      {
         //Extract the values an apply them locally
         delays_enabled = (bool)(pCommand->Value & ATGSET_DELAYS_ENABLE_MASK);
         global_override = (bool)(pCommand->Value & ATGSET_GLOBAL_OVERRIDE_MASK);
         is_day = (bool)(pCommand->Value & ATGSET_IS_DAY_MASK);
         CO_DayNight_t dayNight = is_day ? CO_DAY_E:CO_NIGHT_E;
         ignore_security_in_day = (bool)(pCommand->Value & ATGSET_IGNORE_SECURITY_DAY_MASK);
         ignore_security_at_night = (bool)(pCommand->Value & ATGSET_IGNORE_SECURITY_NIGHT_MASK);
         global_delay1 = (pCommand->Value & ATGSET_GLOBAL_DELAY1_MASK);
         global_delay1 *= ATGSET_GLOBAL_DELAY_RESOLUTION;
         global_delay2 = ((pCommand->Value & ATGSET_GLOBAL_DELAY2_MASK) >> ATGSET_GLOBAL_DELAY_BIT_LENGTH);
         global_delay2 *= ATGSET_GLOBAL_DELAY_RESOLUTION;
         combined_global_delays = global_delay1 + (global_delay2 << 16); //global delay 2 is stored in the upper 16 bits, global delay 1 in the lower 16 bits.
         
         CFG_SetGlobalDelayEnabled(delays_enabled);
         CFG_SetGlobalDelayOverride( global_override );
         MM_CMD_ProcessDayNightSetting( dayNight );
         CFG_SetSecurityInhibitDay(ignore_security_in_day);
         CFG_SetSecurityInhibitNight(ignore_security_at_night);
         DM_OP_SetGlobalDelayValues(combined_global_delays);
         
         
         //For the NCU, distribute the command over the mesh
         if ( ADDRESS_NCU == gNetworkAddress )
         {
            if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
            {
               CO_CommandData_t commandData;
               commandData.TransactionID = GetNextTransactionID();
               commandData.Source = NCU_NETWORK_ADDRESS;
               commandData.CommandType = PARAM_TYPE_GLOBAL_DELAY_COMBINED_E;
               commandData.Destination = pCommand->NodeID;
               commandData.Parameter1 = pCommand->Parameter1;
               commandData.Parameter2 = pCommand->Parameter2;
               commandData.Value = pCommand->Value;
               commandData.ReadWrite = pCommand->ReadWrite;
               commandData.SourceUSART = (uint16_t)source;
               
               CieBuffer_t msgBuffer;
               msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
               memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
               
               result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
            }
            else 
            {
               result = ERR_SYSTEM_NOT_ACTIVE_E;
            }
         }
         else 
         {
            result = SUCCESS_E;
         }
         
         if ( SUCCESS_E == result )
         {
            MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
         }
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessScanForDevices
*  Description:         Send a command to scan for devices.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessScanForDevices( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( COMMAND_WRITE == pCommand->ReadWrite )
      {
         //build the command
         
         //For the NCU, distribute the command over the mesh
         if ( ADDRESS_NCU == gNetworkAddress )
         {
            if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
            {
               
               CO_CommandData_t commandData;
               commandData.TransactionID = GetNextTransactionID();
               commandData.Source = NCU_NETWORK_ADDRESS;
               commandData.CommandType = PARAM_TYPE_SCAN_FOR_DEVICES_E;
               commandData.Destination = pCommand->NodeID;
               commandData.Parameter1 = pCommand->Parameter1;
               commandData.Parameter2 = pCommand->Parameter2;
               commandData.Value = pCommand->Value;
               commandData.ReadWrite = pCommand->ReadWrite;
               commandData.SourceUSART = (uint16_t)source;
               
               CieBuffer_t msgBuffer;
               msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
               memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
               
               result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
            }
            else 
            {
               result = ERR_SYSTEM_NOT_ACTIVE_E;
            }
         }
         else if ( pCommand->NodeID == gNetworkAddress )
         {
            uint32_t Handle = GetNextTransactionID();
            uint32_t number_of_messages = CFG_GetNumberOfResends();
            MM_MeshAPICommandReq(Handle, gNetworkAddress, gNetworkAddress, PARAM_TYPE_SCAN_FOR_DEVICES_E,
                           pCommand->Parameter1, pCommand->Parameter2, pCommand->Value, pCommand->ReadWrite, number_of_messages);
            result = SUCCESS_E;
         }
         
         if ( SUCCESS_E == result )
         {
            MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
         }
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessRequestSensorvalues
*  Description:         Broadcast a command for all radio boards to return their
*                       sensor values.
*  Param - pCommand     The command to check the firmware.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessRequestSensorvalues ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      result = ERR_NOT_SUPPORTED_E;
      
      if ( ADDRESS_NCU == gNetworkAddress )
      {
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_SENSOR_VALUES_REQUEST_E;
            commandData.Destination = ADDRESS_GLOBAL;
            commandData.Parameter1 = 0;
            commandData.Parameter2 = (uint8_t)ZONE_GLOBAL;
            commandData.Value = gSoftwareVersionNumber;
            commandData.ReadWrite = 1;
            commandData.SourceUSART = (uint16_t)source;
            
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
         }
         else 
         {
            result = ERR_SYSTEM_NOT_ACTIVE_E;
         }
      }
      
      if ( SUCCESS_E == result )
      {
         MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessDelayedOutput
*  Description:         Activate an output channel after a specified delay for
*                       a specified period.
*  Param - pCommand     The command to check the firmware.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessDelayedOutput ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   uint32_t initial_delay;
   uint32_t output_duration;

   if ( pCommand )
   {
      result = ERR_NOT_SUPPORTED_E;
      
      if ( pCommand->NodeID == gNetworkAddress )
      {
         initial_delay = (pCommand->Value >> 16);
         output_duration = (pCommand->Value & 0xFFFF);
         CO_PRINT_B_3(DBG_INFO_E,"MM_CMD_ProcessDelayedOutput channel=%d, iniDly=%d, duration=%d\r\n",pCommand->Parameter1, initial_delay, output_duration);
         result = DM_OP_StartDelayedOutput(initial_delay, output_duration, (CO_ChannelIndex_t)pCommand->Parameter1);
      }
      else
      {
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_DELAYED_OUTPUT_E;
            commandData.Destination = pCommand->NodeID;
            commandData.Parameter1 = pCommand->Parameter1;
            commandData.Parameter2 = pCommand->Parameter2;
            commandData.Value = pCommand->Value;
            commandData.ReadWrite = pCommand->ReadWrite;
            commandData.SourceUSART = (uint16_t)source;
            
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
         }
         else 
         {
            result = ERR_SYSTEM_NOT_ACTIVE_E;
         }
      }
      
      if ( SUCCESS_E == result )
      {
         MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessBatteryTestCommand
*  Description:         Activate a full battery test
*  Param - pCommand     The command to force a battery test.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessBatteryTestCommand ( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      result = ERR_NOT_SUPPORTED_E;
      
      if ( pCommand->NodeID == gNetworkAddress )
      {
         CO_PRINT_B_0(DBG_INFO_E,"Command Rx'd to initiate battery test\r\n");
         //Reset the battery restart flag in NVM
         DM_BAT_SetBatteryRestartFlag(DM_BAT_NORMAL_RESTART_E);
         //Initiate immediate battery test
         DM_BAT_RequestBatteryCheck(false);
         result = SUCCESS_E;
      }
      else
      {
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_BATTERY_TEST_E;
            commandData.Destination = pCommand->NodeID;
            commandData.Parameter1 = pCommand->Parameter1;
            commandData.Parameter2 = pCommand->Parameter2;
            commandData.Value = pCommand->Value;
            commandData.ReadWrite = pCommand->ReadWrite;
            commandData.SourceUSART = (uint16_t)source;
            
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
         }
         else 
         {
            result = ERR_SYSTEM_NOT_ACTIVE_E;
         }
      }
      
      if ( SUCCESS_E == result )
      {
         MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessCIEProductCode
*  Description:         Send the product code command to the system.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCIEProductCode( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   uint32_t product_code = 0;
   char str_product_code[5];

   if ( pCommand )
   {
         if ( gNetworkAddress == pCommand->NodeID )
         {
            if ( COMMAND_READ == pCommand->ReadWrite )
            {
               //Read the product code from NVM
               result = DM_NVMRead(NV_PRODUCT_CODE_E, &product_code, sizeof(uint32_t));
               if ( SUCCESS_E == result )
               {
                  result = ERR_OPERATION_FAIL_E;
                  if ( MM_ATDecodeProductCode(product_code, str_product_code) )
                  {
                     result = SUCCESS_E;
                     if ( NCU_NETWORK_ADDRESS == gNetworkAddress)
                     {
                        /* NCU Local node - Put result in CIE misc queue*/
                        CieBuffer_t cieMsg;
                        snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "%s", str_product_code);
                        /*push the message into the CIE misc queue */
                        result = MM_CIEQ_Push(CIE_Q_MISC_E, &cieMsg);
                     }
                     
                     if ( SUCCESS_E == result )
                     {
                        /* RBU Local node - Send result to UART*/
                        char response[AT_RESPONSE_MAX_LENGTH];
                        snprintf((char*)response, AT_RESPONSE_MAX_LENGTH, "%s", str_product_code);
                        MM_CMD_SendATMessage(response, source);
                     }
                  }
               }
            }
            else //COMMAND_WRITE
            {
               //Write the product code to NVM
               product_code = pCommand->Value;
               result = DM_NVMWrite(NV_PRODUCT_CODE_E, &product_code, sizeof(uint32_t));
               if ( SUCCESS_E == result )
               {
                  MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
               }
            }
         }
         else 
         {
            // The command is for a remote device.  Send it over the mesh.
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_PRODUCT_CODE_E;
            commandData.Destination = pCommand->NodeID;
            commandData.Parameter1 = pCommand->Parameter1;
            commandData.Parameter2 = pCommand->Parameter2;
            commandData.Value = pCommand->Value;
            commandData.ReadWrite = pCommand->ReadWrite;
            commandData.SourceUSART = (uint16_t)source;
            result = ERR_SYSTEM_NOT_ACTIVE_E;
            
            if (STATE_ACTIVE_E == GetCurrentDeviceState())
            {
               CieBuffer_t msgBuffer;
               msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
               memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
               
               result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
               
               if ( SUCCESS_E == result )
               {
                  MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
               }
            }
         }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessProductCode
*  Description:         Read/Write the Product code to NVM.
*
*  Param - firmware_version  The version to compare against
*  
*  Returns:             SUCCESS_E or error code
*
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessProductCode(const CO_CommandData_t* pCommand, const CommandSource_t source)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
      /* create a response */
      CO_ResponseData_t response = {0};
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         result = DM_NVMRead(NV_PRODUCT_CODE_E, &response.Value, sizeof(uint32_t));
         
         /* Read value and send it back to the command's originator */;
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */
            MM_CMD_SendATResponse( &response, source );
            result = SUCCESS_E;
         }
         else
         {
            /* respond over Mesh link */
            bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
            if ( msg_sent )
            {
               result = SUCCESS_E;
            }
         }
      }
      else
      {
         uint32_t value = pCommand->Value;
         result = DM_NVMWrite(NV_PRODUCT_CODE_E, &value, sizeof(uint32_t));

         if ( SUCCESS_E == result )
         {
            response.Value = value;
            /* Send the value back to the command's originator */;
            if( CMD_SOURCE_MESH_E == source )
            {
               /* respond over Mesh link */
               bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                          response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
               if ( msg_sent )
               {
                  result = SUCCESS_E;
               }
            }
            else if ( CMD_SOURCE_INVALID_E != source )
            {
               /* respond over AT USART link */
               if (MM_CMD_SendATMessage(AT_RESPONSE_OK, source ) )
               {
                  result = SUCCESS_E;
               }
               else
               {
                  result = ERR_MESSAGE_FAIL_E;
                  MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
               }
            }
         }
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessPPUModeCommand
*  Description:         Set the PPU Mode Enable flag.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessPPUModeCommand( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
         if ( (gNetworkAddress == pCommand->NodeID) || (ADDRESS_GLOBAL == pCommand->NodeID) )
         {
            if ( COMMAND_READ == pCommand->ReadWrite )
            {
               //Read the NVM setting
               uint32_t ppu_enable;
               DM_NVMRead(NV_PP_MODE_ENABLE_E, &ppu_enable, sizeof(uint32_t));

               if ( NCU_NETWORK_ADDRESS == gNetworkAddress)
               {
                  /* NCU Local node - Put result in CIE misc queue*/
                  CieBuffer_t cieMsg;
                  snprintf((char*)cieMsg.Buffer, CIE_QUEUE_DATA_SIZE, "%d", ppu_enable);
                  /*push the message into the CIE misc queue */
                  result = MM_CIEQ_Push(CIE_Q_MISC_E, &cieMsg);
                  if ( SUCCESS_E == result )
                  {
                     MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
                  }
               }
               else 
               {
                  /* RBU Local node - Send result to UART*/
                  char response[AT_RESPONSE_MAX_LENGTH];
                  snprintf((char*)response, AT_RESPONSE_MAX_LENGTH, "%d", ppu_enable);
                  result = SUCCESS_E;
                  MM_CMD_SendATMessage(response, source);
               }
            }
            else //COMMAND_WRITE
            {
               uint32_t ppu_enable = pCommand->Value;
               //Write the NVM setting
               result = DM_NVMWrite(NV_PP_MODE_ENABLE_E, &ppu_enable, sizeof(uint32_t));
               if ( SUCCESS_E == result )
               {
                  rbu_pp_mode_request = ppu_enable;
                  MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
               }
            }
         }
         
         if ( (gNetworkAddress != pCommand->NodeID) || (ADDRESS_GLOBAL == pCommand->NodeID) )
                  {
            // The command is for a remote device.  Send it over the mesh.
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_PPU_MODE_ENABLE_E;
            commandData.Destination = pCommand->NodeID;
            commandData.Parameter1 = pCommand->Parameter1;
            commandData.Parameter2 = pCommand->Parameter2;
            commandData.Value = pCommand->Value;
            commandData.ReadWrite = pCommand->ReadWrite;
            commandData.SourceUSART = (uint16_t)source;
            result = ERR_SYSTEM_NOT_ACTIVE_E;
            
            if (STATE_ACTIVE_E == GetCurrentDeviceState())
                     {
               CieBuffer_t msgBuffer;
               msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
               memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
               
               result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
               
               if ( SUCCESS_E == result )
                     {
                  MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
                     }
                  }
               }
            }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
         }
         
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessEnterPPUModeCommand
*  Description:         Send a message to put a device into PPU Disconnected mode.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessEnterPPUModeCommand( const CIECommand_t* pCommand, const CommandSource_t source )
         {
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( COMMAND_WRITE == pCommand->ReadWrite )
      {
         if ( gNetworkAddress == pCommand->NodeID )
         {
            //It's for the local device
            ApplicationMessage_t AppMessage;
            AppMessage.MessageType = APP_MSG_TYPE_PPU_MODE_E;
            result = MM_PPU_ApplicationProcessPpuModeMessage(&AppMessage);

            if ( SUCCESS_E == result )
            {
               MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
            }
         }
         else
         {
            // The command is for a remote device.  Send it over the mesh.
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_ENTER_PPU_MODE_E;
            commandData.Destination = pCommand->NodeID;
            commandData.Parameter1 = pCommand->Parameter1;
            commandData.Parameter2 = pCommand->Parameter2;
            commandData.Value = pCommand->Value;
            commandData.ReadWrite = pCommand->ReadWrite;
            commandData.SourceUSART = (uint16_t)source;
            result = ERR_SYSTEM_NOT_ACTIVE_E;
            if (STATE_ACTIVE_E == GetCurrentDeviceState())
            {
               CieBuffer_t msgBuffer;
               msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
               memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
               
               result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
               
               if ( SUCCESS_E == result )
               {
                  MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
               }
            }
         }
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessExitTestModeCommand
*  Description:         Send a message to exit the device from the test mode.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessExitTestModeCommand( const CIECommand_t* pCommand, const CommandSource_t source )
         {
   ErrorCode_t result = ERR_INVALID_POINTER_E;

   if ( pCommand )
   {
      if ( COMMAND_WRITE == pCommand->ReadWrite )
      {
         if ( gNetworkAddress == pCommand->NodeID )
         {
					 //It's for the local device
            ApplicationMessage_t AppMessage;
            AppMessage.MessageType = APP_MSG_TYPE_EXIT_TEST_MODE_E;
            result = MM_ApplicationProcessExitTestMessage(&AppMessage);
            if ( SUCCESS_E == result )
            {
               MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
            }
         }
         else
         {
            // The command is for a remote device.  Send it over the mesh.
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_EXIT_TEST_MODE_E;
            commandData.Destination = pCommand->NodeID;
            commandData.Parameter1 = pCommand->Parameter1;
            commandData.Parameter2 = pCommand->Parameter2;
            commandData.Value = pCommand->Value;
            commandData.ReadWrite = pCommand->ReadWrite;
            commandData.SourceUSART = (uint16_t)source;
            result = ERR_SYSTEM_NOT_ACTIVE_E;
            if (STATE_ACTIVE_E == GetCurrentDeviceState())
            {
               CieBuffer_t msgBuffer;
               msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
               memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
               
               result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
               
               if ( SUCCESS_E == result )
               {
                  MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
               }
            }
         }
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessPPUModeEnable
*  Description:      Process the PPU Mode Enable command.
*  Param - pCommand: The command to be processed.
*  Param - source:   The source of the command, AT or Mesh.  Responses should be
*                    sent back via the same link.
*  Returns:          SUCCESS_E if the command was processed or an error code if
*                    there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessPPUModeEnable ( const CO_CommandData_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   char error_response[AT_RESPONSE_MAX_LENGTH];
   
   if ( pCommand )
   {
      /* create a response */
      CO_ResponseData_t response;
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         uint32_t enable_disable;
         result = DM_NVMRead(NV_PP_MODE_ENABLE_E, &enable_disable, sizeof(uint32_t));
         if ( SUCCESS_E == result )
         {
            response.Value = enable_disable;
         }
         else 
         {
            snprintf(error_response, AT_RESPONSE_MAX_LENGTH,"ERROR,NVM READ PPU MODE ENABLE FAILED");
         }
      }
      else //COMMAND_WRITE
      {
         uint32_t enable_disable = pCommand->Value;
         result = DM_NVMWrite(NV_PP_MODE_ENABLE_E, &enable_disable, sizeof(uint32_t));
         if ( SUCCESS_E == result )
         {
            //For global PPEN command, automatically reset the device
            if ( ADDRESS_GLOBAL == pCommand->Destination )
            {
               gResetToPpenMode = enable_disable;
            }
         }
         else
         {
            snprintf(error_response, AT_RESPONSE_MAX_LENGTH,"ERROR,NVM OPERATION FAILED");
         }
      }
      
      if ( SUCCESS_E == result )
      {
         /* Send the response back to the command's originator */
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            if ( SUCCESS_E == result )
            {
               /* respond over AT USART link */ 
               MM_CMD_SendATResponse( &response, source );
            }
            else 
            {
               MM_CMD_SendATMessage(error_response, source);
            }
         }
         //If the command came over the mesh, send the response back to the mesh
         //except for globally addressed commands, which could swamp the mesh with responses.
         if( (CMD_SOURCE_MESH_E == source) && (ADDRESS_GLOBAL != pCommand->Destination) )
         {
            /* respond over Mesh link */
            MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
         }
      }
   }
   else
   {
      result = ERR_MESSAGE_FAIL_E;
   }
   
   return result;
}


/*****************************************************************************
*  Function:   MM_CMD_ProcessCIEDepassivationSettings
*  Description:         Read/write the depassivation settings for battery management.
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessCIEDepassivationSettings( const CIECommand_t* pCommand, const CommandSource_t source )
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   DePassivationSettings_t settings = {0};
   

   if ( pCommand )
   {
      //For the NCU, distribute the command over the mesh
      if ( (ADDRESS_NCU == gNetworkAddress) && (ADDRESS_NCU != pCommand->NodeID) )
      {
         if ( STATE_ACTIVE_E == GetCurrentDeviceState() )
         {
            CO_CommandData_t commandData;
            commandData.TransactionID = GetNextTransactionID();
            commandData.Source = NCU_NETWORK_ADDRESS;
            commandData.CommandType = PARAM_TYPE_DEPASSIVATION_SETTINGS_E;
            commandData.Destination = pCommand->NodeID;
            commandData.Parameter1 = pCommand->Parameter1;
            commandData.Parameter2 = pCommand->Parameter2;
            commandData.Value = pCommand->Value;
            commandData.ReadWrite = pCommand->ReadWrite;
            commandData.SourceUSART = (uint16_t)source;
            
            CieBuffer_t msgBuffer;
            msgBuffer.MessageType = APP_MSG_TYPE_COMMAND_E;
            memcpy(&msgBuffer, (uint8_t*)&commandData, sizeof(CO_CommandData_t));
            
            result = MM_CIEQ_Push(CIE_Q_TX_BUFFER_E, &msgBuffer);
            
            if ( (SUCCESS_E == result) && (ADDRESS_GLOBAL != pCommand->NodeID) )
            {
               MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
            }
         }
         else 
         {
            result = ERR_SYSTEM_NOT_ACTIVE_E;
         }
      }
      
      if ( pCommand->NodeID == gNetworkAddress )
      {
         if ( COMMAND_WRITE == pCommand->ReadWrite )
         {
            //Apply the settings locally
            result = CFG_SetBatteryPassivationValuesCompound(pCommand->Value);
            
            if ( SUCCESS_E == result )
            {
               MM_CMD_SendATMessage(AT_RESPONSE_OK, source);
            }
         }
         else 
         {
            //Read command
            //Get the depassivation settings
            result = CFG_GetBatteryPassivationValues(&settings);
            if ( SUCCESS_E == result )
            {
               uint8_t response[AT_RESPONSE_MAX_LENGTH];
               snprintf((char*)response, AT_RESPONSE_MAX_LENGTH, "Z%dU%d,%d,%d,%d,%d", settings.Zone, gNetworkAddress, settings.Voltage, settings.Timeout, settings.OnTime, settings.OffTime);
               CO_PRINT_B_5(DBG_INFO_E,"ZxU%d,%d,%d,%d,%d\r\n", gNetworkAddress, settings.Voltage, settings.Timeout, settings.OnTime, settings.OffTime);
               MM_CMD_SendATMessage((char*)response, source);
            }
         }
      }
   }
   
   if ( SUCCESS_E != result )
   {
      MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source); 
   }
  
   return result;
}

/*****************************************************************************
*  Function:   MM_CMD_ProcessDepassivationSettings
*  Description:         Read/Write the battery management depassivation settings.
*
*  Param - pCommand     The command data structure.
*  Param - source:      The source of the command, AT or Mesh.  Responses should be
*                       sent back via the same link.
*  Returns:             SUCCESS_E if the command was processed or an error code if
*                       there was a problem.
*  Notes:            
*****************************************************************************/
ErrorCode_t MM_CMD_ProcessDepassivationSettings(const CO_CommandData_t* pCommand, const CommandSource_t source)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pCommand )
   {
      /* create a response */
      CO_ResponseData_t response = {0};
      response.CommandType = pCommand->CommandType;
      response.Source = pCommand->Source;
      response.Destination = pCommand->Destination;
      response.Parameter1 = pCommand->Parameter1;
      response.Parameter2 = pCommand->Parameter2;
      response.ReadWrite = pCommand->ReadWrite;
      response.TransactionID = pCommand->TransactionID;
      response.Value = pCommand->Value;
      
      
      if ( COMMAND_READ == pCommand->ReadWrite )
      {
         result = DM_NVMRead(NV_BAT_PASSIVATION_SETTINGS_E, &response.Value, sizeof(uint32_t));
         
         /* Read value and send it back to the command's originator */;
         if( CMD_SOURCE_MESH_E != source && CMD_SOURCE_INVALID_E != source)
         {
            /* respond over AT USART link */
            MM_CMD_SendATResponse( &response, source );
            result = SUCCESS_E;
         }
         else
         {
            /* respond over Mesh link, except for globally addressed commands*/
            if ( ADDRESS_GLOBAL != pCommand->Destination )
            {
               bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                       response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
               if ( msg_sent )
               {
                  result = SUCCESS_E;
               }
            }
            else 
            {
               result = SUCCESS_E;
            }
         }
      }
      else
      {
         uint32_t value = pCommand->Value;
         result = CFG_SetBatteryPassivationValuesCompound(value);

         if ( SUCCESS_E == result )
         {
            response.Value = value;
            /* Send the value back to the command's originator */;
            if( CMD_SOURCE_MESH_E == source )
            {
               /* respond over Mesh link, except for globally addressed commands*/
               if ( ADDRESS_GLOBAL != pCommand->Destination )
               {
                  bool msg_sent = MM_MeshAPIResponseReq(response.TransactionID, response.Source, response.Destination, response.CommandType,
                                          response.Parameter1, response.Parameter2, response.Value, response.ReadWrite);
                  if ( msg_sent )
                  {
                     result = SUCCESS_E;
                  }
               }
               else 
               {
                  result = SUCCESS_E;
               }
            }
            else if ( CMD_SOURCE_INVALID_E != source )
            {
               /* respond over AT USART link */
               if (MM_CMD_SendATMessage(AT_RESPONSE_OK, source ) )
               {
                  result = SUCCESS_E;
               }
               else
               {
                  result = ERR_MESSAGE_FAIL_E;
                  MM_CMD_SendATMessage(AT_RESPONSE_FAIL, source);
               }
            }
         }
      }
   }
   
   return result;
}

