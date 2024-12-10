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
*  File         : MM_ATHandleTask.h
*
*  Description  : AT Handle Task header file
*
*************************************************************************************/

#ifndef MM_AT_HANDLE_TASK_H
#define MM_AT_HANDLE_TASK_H


/* System Include Files
*************************************************************************************/
#include <stdbool.h>
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "DM_SerialPort_cfg.h"
#include "CO_ErrorCode.h"
#include "MM_ATHandle_cfg.h"


/* Public Constants
*************************************************************************************/
#define AT_COMMAND_MAX_LENGTH			 8U
#define AT_PARAMETER_MAX_LENGTH		48U
#define AT_RESPONSE_MAX_LENGTH      80U


/* Public Structures
*************************************************************************************/ 
typedef struct
{
   UartComm_t port_id;
   uint8_t data_buffer[AT_RESPONSE_MAX_LENGTH];
   uint8_t length;
} Cmd_Reply_t;



/* Public Enumerations
*************************************************************************************/
typedef enum
{
   AT_STATE_STOPPED_E,
   AT_STATE_WAITING_QUEUE_E,
   AT_STATE_GET_TYPE_E,
   AT_STATE_REPLY_E
} ATHandleState_t;

typedef enum 
{
   AT_COMMAND_NO_TYPE_E  = 0x00,
   AT_COMMAND_READ_E     = 0x01,
   AT_COMMAND_WRITE_E    = 0x02,
   AT_COMMAND_SPECIAL_E  = 0x04,
   AT_COMMAND_RESPONSE_E = 0x08, /*assync response to an earlier command. To be sent over the serial link*/
   AT_COMMAND_NO_REPLY_E = 0x80
} ATCommandType_t;

typedef enum {
   AT_NO_ERROR_E,
   AT_INVALID_COMMAND_E,
   AT_INVALID_PARAMETER_E,
   AT_COMMAND_TOO_LONG_E,
   AT_COMMAND_UNKNOWN_ERROR_E,
   AT_DO_NOT_RESPOND_E,
   AT_QUEUE_ERROR_E,
   AT_NVM_ERROR_E,
   AT_NO_RESOURCE_E,
   AT_OPERATION_FAIL_E,
} ATMessageStatus_t;

typedef ATMessageStatus_t (*at_command_processing_function_p)(const ATCommandType_t command_type,
                                                              const uint8_t *source_buffer,
                                                              const uint8_t buffer_length,
                                                              const UartComm_t uart_port,
                                                              const uint32_t option);
                                                              
typedef struct
{
   const char *const at_command;
   at_command_processing_function_p func_callback;
   uint8_t supported_operations;
   uint32_t option_param;
}at_command_def_t;

/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t MM_ATHandleTaskInit(void);
void MM_ATHandleTaskMain(void const *argument);



/* Macros
*************************************************************************************/



#endif // MM_AT_HANDLE_TASK_H
