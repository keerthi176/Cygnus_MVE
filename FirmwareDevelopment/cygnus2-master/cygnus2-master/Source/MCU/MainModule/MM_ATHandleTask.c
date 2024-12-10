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
*  File         : MM_ATHandleTask.c
*
*  Description  : AT Commands handler task functions
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "CO_Message.h"
#include "cmsis_os.h"
#include "DM_SerialPort.h"
#include "MM_ATHandleTask.h"
#include "MM_ConfigSerialTask.h"
#include "MM_ATCommand.h"
#include "DM_NVM.h"


/* Private Functions Prototypes
*************************************************************************************/
void MM_ATHandleTaskMain(void const *argument);                   // thread function
ATCommandType_t MM_ATHandleParseCommand(uint8_t* source_buffer, 
                                       uint8_t message_length,
                                       uint8_t* command_buffer,
                                       uint8_t* parameter_buffer,
                                       uint8_t* parameter_length);


/* Global Variables
*************************************************************************************/
#define ATHANDLE_POOL_SIZE		10
#define ATHANDLE_Q_SIZE    	4

extern const at_command_def_t at_command_def[];
extern const uint16_t at_command_def_size;
extern osMessageQId(ConfigSerialQ);       // Queue definition from ConfigSerialTask
extern osPoolId ConfigSerialPool;         // Pool definition from ConfigSerialTask
extern osPoolId AppPool;
extern osMessageQId(AppQ);

const char CIE_ERROR_CODE_STR[AT_CIE_ERROR_MAX_E][3] = 
{
   [AT_CIE_ERROR_NONE_E]                     = {"00"},
   [AT_CIE_ERROR_UNKNOWN_COMMAND_E]          = {"01"},
   [AT_CIE_ERROR_VALUE_OUT_OF_RANGE_E]       = {"02"},
   [AT_CIE_ERROR_WRONG_VALUE_FORMAT_E]       = {"03"},
   [AT_CIE_ERROR_RU_DOES_NOT_EXIST_E]        = {"04"},
   [AT_CIE_ERROR_CHANNEL_DOES_NOT_EXIST_E]   = {"05"},
   [AT_CIE_ERROR_QUEUE_EMPTY_E]              = {"06"},
   [AT_CIE_ERROR_TX_BUFFER_FULL_E]           = {"07"},
   [AT_CIE_ERROR_NETWORK_TIMEOUT_E]          = {"08"}
};

ATHandleState_t AT_State = AT_STATE_STOPPED_E;

uint8_t at_data_buffer[ATHANDLE_Q_SIZE];

osThreadId tid_ATHandleTask;                                   // thread id
osThreadDef(MM_ATHandleTaskMain, osPriorityNormal, 1, 1024);   // thread object

// Pool Definition
osPoolDef(ATHandlePool, ATHANDLE_POOL_SIZE, Cmd_Reply_t);
osPoolId ATHandlePool;

#ifdef DEBUG_ATHANDLE_POOL
uint32_t gAtHandlePoolCount = 0;
#endif

// Queue Definition
osMessageQDef(ATHandleQ, ATHANDLE_Q_SIZE, &Cmd_Reply_t);
osMessageQId(ATHandleQ);

/* Private Variables
*************************************************************************************/


/*************************************************************************************/
/**
* function name   : MM_ATHandleTaskInit(const uint16_t address)
* description     : Initialisation of the ATHandle task
*
* @return - ErrorCode_t  // Returns an error code, else SUCCESS_E
*/
ErrorCode_t MM_ATHandleTaskInit(void)
{
   // create pool
   ATHandlePool = osPoolCreate(osPool(ATHandlePool));
   CO_ASSERT_RET_MSG(NULL != ATHandlePool, ERR_FAILED_TO_CREATE_MEMPOOL_E, "ERROR - Failed to create AT Pool");
   
   // create queues
   ATHandleQ = osMessageCreate(osMessageQ(ATHandleQ), NULL);
   CO_ASSERT_RET_MSG(NULL != ATHandleQ, ERR_FAILED_TO_CREATE_QUEUE_E, "ERROR - Failed to create AT Queue");

   // Create thread (return on error)
   tid_ATHandleTask = osThreadCreate(osThread(MM_ATHandleTaskMain), NULL);
   CO_ASSERT_RET_MSG(NULL != tid_ATHandleTask, ERR_FAILED_TO_CREATE_THREAD_E, "ERROR - Failed to create AT thread");
   
   CO_PRINT_B_0(DBG_INFO_E, "ATHandleTask created\r\n");

   return(SUCCESS_E);
}


/*************************************************************************************/
/**
* function name	: MM_ATHandleTaskMain(void const *argument)
* description		: Main function for ATHandle task
*
* @param - 
*
* @return - nothing
*/
void MM_ATHandleTaskMain(void const *argument)
{
   Cmd_Reply_t *pCmdReply = NULL;
   static Cmd_Message_t *pCmdMessage = NULL;
   static uint8_t command_buffer[AT_COMMAND_MAX_LENGTH];
   static uint8_t parameter_buffer[AT_PARAMETER_MAX_LENGTH];
   uint8_t parameter_length;
   ATCommandType_t command_type;
   static ATMessageStatus_t error_code = AT_NO_ERROR_E;
   char cr_lf_string[] = "\r\n";
   bool no_reply_prefix = false;

   while (1)
   {
      osEvent event;
      //Cmd_Message_t *pCmdMessage = NULL;
      
      switch(AT_State)
      {
         case AT_STATE_STOPPED_E:
            // RM: do something here
            AT_State = AT_STATE_WAITING_QUEUE_E;
         break;
         
         case AT_STATE_WAITING_QUEUE_E:

            event = osMessageGet(ConfigSerialQ, osWaitForever);
            if(event.status == osEventMessage)
            {
               // Queue found, extract message
               pCmdMessage = (Cmd_Message_t*)event.value.p;
               if(pCmdMessage != NULL)
               {
                  // Message content found
                  AT_State = AT_STATE_GET_TYPE_E;
                  strcpy((char*)command_buffer, "");
               }
            }
         break;
         
         case AT_STATE_GET_TYPE_E:
            error_code = AT_NO_ERROR_E;
            AT_State = AT_STATE_REPLY_E;
            no_reply_prefix = false;

            /* Flush the Receive Queue, if any item is pending */
            do
            {
               event = osMessageGet(ATHandleQ, 0u);
               if (event.status == osEventMessage)
               {
                  /* Free the memory pool entry */
                  FREE_ATHANDLE_POOL(event.value.p);
               }
            } while (event.status == osEventMessage);

            if(pCmdMessage->length == 3u && !strncmp((const char*)"+++", (const char*)pCmdMessage->data_buffer, 3u))
            {
               strcpy((char*)command_buffer, "+++");
               error_code = MM_ATCommand_enter_at_mode(pCmdMessage->port_id);
            }
            else
            {
               bool command_found = false;
               bool wait_for_response = true;

               //Remove spaces from the command string
               uint8_t buffer[AT_COMMAND_MAX_LENGTH+AT_PARAMETER_MAX_LENGTH+1u];
               uint8_t bufChar;
               uint32_t bufIndex = 0;
               uint32_t cmdIndex = 0;
               memset(buffer, 0, (sizeof(buffer)));
               do
               {
                  if ( ' ' != pCmdMessage->data_buffer[cmdIndex] )
                  {
                     buffer[bufIndex] = pCmdMessage->data_buffer[cmdIndex];
                     bufIndex++;
                  }
                  cmdIndex++;
               } while ( (0 != bufChar) && (cmdIndex < sizeof(buffer)) );
                  
               /* Parse the received command */
               command_type = MM_ATHandleParseCommand(pCmdMessage->data_buffer, 
                                                      pCmdMessage->length,
                                                      command_buffer,
                                                      parameter_buffer,
                                                      &parameter_length);
               
               for(uint16_t index = 0u;
                   index < at_command_def_size && command_found == false; 
                   index++ )
               {
                  const at_command_def_t *const at_command_def_p = &at_command_def[index];
                  /* Compare the whole strings, we are sure to have the Null terminator in both */
                  if(!strcmp(at_command_def_p->at_command, (const char*)command_buffer))
                  {
                     /* Set the Flag */
                     command_found = true;
                     
                     CO_PRINT_B_1(DBG_INFO_E,">>%s\r\n",pCmdMessage->data_buffer);


                     if (command_type & (at_command_def_p->supported_operations & ~AT_COMMAND_NO_REPLY_E))
                     {
                        error_code = at_command_def_p->func_callback(command_type, parameter_buffer, parameter_length, pCmdMessage->port_id, at_command_def_p->option_param);

                        if (at_command_def_p->supported_operations & AT_COMMAND_NO_REPLY_E)
                        {
                           if (AT_NO_ERROR_E == error_code)
                           {
                              /* We do not need to wait for a reponse for this command */
                              wait_for_response = false;
                           }
                        }
                        else if ( at_command_def_p->supported_operations & AT_COMMAND_RESPONSE_E )
                        {
                           no_reply_prefix = true;
                        }
                     }
                     else
                     {
                        error_code = AT_INVALID_COMMAND_E;
                     }
                  }                        
               }

               //Clear stale data from the command buffer
               memset(pCmdMessage->data_buffer, 0, (AT_COMMAND_MAX_LENGTH+AT_PARAMETER_MAX_LENGTH+1u));
               
               if ( command_found == false || wait_for_response == false )
               {
                  /* command made no sense.  Send error code to CIE and go back to the queue for the next one */
                  char reply[AT_RESPONSE_MAX_LENGTH];
                  snprintf(reply, AT_RESPONSE_MAX_LENGTH, "%s: ERROR,%s\r\n", (char*)&command_buffer[2], CIE_ERROR_CODE_STR[AT_CIE_ERROR_UNKNOWN_COMMAND_E]);
                  SerialPortWriteTxBuffer(pCmdMessage->port_id, (uint8_t*)reply, strlen(reply));
                  AT_State = AT_STATE_WAITING_QUEUE_E;
                  osPoolFree(ConfigSerialPool, pCmdMessage);
               }
            }

         break;

         case AT_STATE_REPLY_E:
         {
            char reply[AT_RESPONSE_MAX_LENGTH];

         	if( no_reply_prefix || !strncmp((const char*)"+++", (const char*)command_buffer, 3) )
            {
               /* No Prefix for the +++ response or assync response types (CIE Queue queries)*/
               strcpy(reply, "");
            }
            else
            {
               /* Create the reply with the AT command name as prefix (if any)*/
               strcpy(reply, (char*)&command_buffer[2]);
               strcat(reply, ": ");
            }

            if(error_code == AT_NO_ERROR_E) 
            {
               event = osMessageGet(ATHandleQ, ATHANDLE_REPLY_WAIT_TIMEOUT_MS);
               if(event.status == osEventMessage)
               {
                  // Queue found, extract message
                  pCmdReply = (Cmd_Reply_t*)event.value.p;
                  if(pCmdReply != NULL)
                  {
                     /* Append the response string */
                     strcat(reply,(char*)pCmdReply->data_buffer);
                     
                     if ( MESH_INTERFACE_E == pCmdMessage->port_id )
                     {
                        //pack the response back to the application for sending over the mesh.
                        //send the command to the Application
                        CO_Message_t* pMessage = osPoolCAlloc(AppPool);
                        if ( pMessage )
                        {
                           pMessage->Type = CO_MESSAGE_GENERATE_ATTX_RESPONSE_E;
                           uint32_t reply_length = strlen(reply);
                           
                           char* pDataBuffer = (char*)pMessage->Payload.PhyDataReq.Data;
                           memcpy(pDataBuffer, &reply, reply_length);
                           
                           osStatus osStat = osMessagePut(AppQ, (uint32_t)pMessage, 0);
                           if (osOK != osStat)
                           {
                              /* failed to write */
                              osPoolFree(AppPool, pMessage);
                           }
                        }
                     }
                     else
                     {
                        //We want to filter-out "ATTX: OK" 
                        if ( strcmp(reply,"ATTX: OK") )
                        {
                           // Append <CR><LF>
                           strcat(reply, cr_lf_string);
                           /* Send the response */
                           SerialPortWriteTxBuffer(pCmdMessage->port_id, (uint8_t*)reply, strlen(reply));
                        }
                     }
                     // Message content found
                     FREE_ATHANDLE_POOL(pCmdReply);
                  }
               }
               else
               {
                  strcat(reply, AT_RESPONSE_FAIL);
                  // Append <CR><LF>
                  strcat(reply, cr_lf_string);

                  SerialPortWriteTxBuffer(pCmdMessage->port_id, (uint8_t*)reply, strlen(reply));
               }
            }
            else
            {
               strcat(reply, AT_RESPONSE_FAIL);
               // Append <CR><LF>
               strcat(reply, cr_lf_string);
               SerialPortWriteTxBuffer(pCmdMessage->port_id, (uint8_t*)reply, strlen(reply));
            }

            AT_State = AT_STATE_WAITING_QUEUE_E;
            // Release Pool
            osPoolFree(ConfigSerialPool, pCmdMessage);
         }
         break;
            
         default:
            ;
         break;
      }
      
      // suspend thread
      osThreadYield ();                                           
   }
}

/*************************************************************************************/
/**
* function name	: MM_ATHandleGetCommand
* description		: Get AT command string and type of command
*
* @param - uint8_t* source_buffer : buffer containing complete AT command string
* @param - uint8_t* destination_buffer : buffer where just the command part of the complete string will be place
* @param - uint8_t message_length : length of the complete AT command string
* @param - uint8_t destination_buffer_length : length of just the command part
*
* @return - ATCommandType_t - returns the type of the command being processed
*/
ATCommandType_t MM_ATHandleParseCommand(uint8_t* source_buffer, 
                                       uint8_t message_length,
                                       uint8_t* command_buffer,
                                       uint8_t* parameter_buffer,
                                       uint8_t* parameter_length)
{
   ATCommandType_t command_type = AT_COMMAND_NO_TYPE_E;
   uint8_t i = 0;
   uint8_t j = 0;
   
   if ( source_buffer && command_buffer && parameter_buffer && parameter_length )
   {
      command_buffer[0] = '\0';
      parameter_buffer[0] = '\0';

      while (   i < (AT_COMMAND_MAX_LENGTH - 1) 
             && i < message_length 
             && source_buffer[i] != '?' 
             && source_buffer[i] != '=' 
             && source_buffer[i] != '+')
      {
         command_buffer[i] = source_buffer[i];
         i++;
      }

      // Finish command string with null character
      command_buffer[i] = '\0';

      // Fixes problem when command with no-type is received
      if(i < message_length) 
      {
         // Identify command type
         if (source_buffer[i] == '?')
            command_type = AT_COMMAND_READ_E;
         else if (source_buffer[i] == '=')
            command_type = AT_COMMAND_WRITE_E;
         else if (source_buffer[i] == '+')
            command_type = AT_COMMAND_SPECIAL_E;
         else
            command_type = AT_COMMAND_NO_TYPE_E;
         
         /* Jump over the operation type operator */
         i++;
         
         for (j = 0u; (j + i) < message_length; j++) 
         {
            // check that the parameter fits in the buffer (leave one
            // space for the null character)
            if (j == (AT_PARAMETER_MAX_LENGTH - 1))
            {
               /* command type not valid */
               command_type = AT_COMMAND_NO_TYPE_E;
               *parameter_length = 0u;
               break;
            }
            parameter_buffer[j] = source_buffer[j + i];
         }
         parameter_buffer[j] = '\0';
         *parameter_length = j;
      }
      else
      {
         /* command type not valid */
         command_type = AT_COMMAND_NO_TYPE_E;
         parameter_buffer = NULL;
         *parameter_length = 0u;
      }
   }

   return command_type;
}
