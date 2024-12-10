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
*  File         : MM_CommandProcessor.h
*
*  Description  : Manage the command/response messages fromm the Mesh Protocol. 
*
*************************************************************************************/

#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include "CO_Message.h"
#include "DM_Device.h"
#include "MM_ATCommand.h"

#define MAX_RESERVED_EEPROM_ATTRIBUTES NV_MAX_PARAM_ID_E

typedef enum
{
   CMD_SOURCE_MESH_E,
   CMD_SOURCE_UART_DEBUG_E,
   CMD_SOURCE_UART_PPU_E,
   CMD_SOURCE_UART_HEAD_E,
   CMD_SOURCE_UART_CP_E,
   CMD_SOURCE_UART_SBC_E,
   CMD_SOURCE_APPLICATION_E,
   CMD_SOURCE_INVALID_E
} CommandSource_t;

typedef enum
{
   FI_RBU_MAIN_IMAGE_E,
   FI_RBU_BACKUP_IMAGE_E,
   FI_EXTERNAL_DEVICE_1_E,
   FI_EXTERNAL_DEVICE_2_E,
   FI_EXTERNAL_DEVICE_3_E,
   FI_INTERNAL_DEVICE_1_E,
   FI_INTERNAL_DEVICE_2_E,
   FI_INTERNAL_DEVICE_3_E
}FirmwareInfoSource_t;

void MM_CMD_Initialise(const uint16_t address, const uint16_t device_combination, const bool sync_master, const uint32_t unitSerialNo);
ErrorCode_t MM_CMD_ProcessCommand ( const CO_CommandData_t* pCommand, const CommandSource_t source );
ErrorCode_t MM_CMD_ProcessCIECommand ( const CIECommand_t* pCommand );
ErrorCode_t MM_CMD_ProcessHeadMessage ( const HeadMessage_t* pMessage );
ErrorCode_t MM_CMD_ProcessMeshResponse ( const uint8_t Handle, const uint16_t Destination, const uint8_t CommandType,
                           const uint8_t Parameter1, const uint8_t Parameter2, const uint32_t Value, const uint8_t ReadWrite );

bool MM_CMD_SendATMessage(char* pMessage, const CommandSource_t cmd_source);
CommandSource_t MM_CMD_ConvertFromATCommandSource( const uint16_t at_source );
UartComm_t MM_CMD_ConvertToATCommandSource( const CommandSource_t cmd_source );
bool MM_CMD_CommandExpectsResponse( const ParameterType_t command, const uint16_t destination );
bool MM_CMD_SendHeadMessage(const CO_MessageType_t messageType, HeadMessage_t* pMessage, const CommandSource_t source);
ErrorCode_t MM_CMD_ProcessDayNightSetting ( const CO_DayNight_t dayNight );

#endif /*COMMAND_PROCESSOR_H*/
