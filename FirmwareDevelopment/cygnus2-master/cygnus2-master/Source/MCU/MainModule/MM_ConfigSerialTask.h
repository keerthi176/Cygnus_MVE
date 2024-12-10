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
*  File         : MM_ConfigSerialTask.h
*
*  Description  : Config Serial Task header file
*
*************************************************************************************/

#ifndef MM_CONFIG_SERIAL_TASK_H
#define MM_CONFIG_SERIAL_TASK_H


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/
#include "DM_SerialPort.h"
#include "CO_ErrorCode.h"
#include "MM_ATHandleTask.h"
#include "MM_ConfigSerial_cfg.h"

/* Public Structures
*************************************************************************************/ 
typedef struct
{
   UartComm_t port_id;
   uint8_t data_buffer[AT_COMMAND_MAX_LENGTH+AT_PARAMETER_MAX_LENGTH+1u];/* +1 for the operation type */
   uint8_t length;
} Cmd_Message_t;


/* Public Enumerations
*************************************************************************************/

/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t MM_ConfigSerialTaskInit(CS_Configurations_t serial_config);
void MM_ConfigSerialTaskMain(void const *argument);
void MM_SetTransparentMode(UartComm_t uart_port);
bool MM_ConfigSerialEnterStopMode(void);
void MM_ConfigSerialLeaveStopMode(void);
bool MM_ConfigSerial_CanGoToSleep(void);
void MM_ConfigSerial_ErrorCallback(uint8_t channel_id);

/* Public Constants
*************************************************************************************/



/* Macros
*************************************************************************************/



#endif // MM_CONFIG_SERIAL_TASK_H
