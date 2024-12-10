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
*  File         : CO_SerialDebug.c
*
*  Description  : This module provides the interfaces for the debug over UART
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "cmsis_os.h"


/* User Include Files
*************************************************************************************/
#include "DM_SerialPort.h"
#include "CO_SerialDebug.h"


/* Private Functions Prototypes
*************************************************************************************/


/* Global Variables
*************************************************************************************/
#ifdef _DEBUG
#define SERIAL_DEBUG_UART_PORT DEBUG_UART_E
#else
#define SERIAL_DEBUG_UART_PORT DEBUG_UART_E
#endif

/* Private Variables
*************************************************************************************/
/* Mutex */
osMutexId(DebugPortMutexId);
osMutexDef(DebugPortMutex);

/*************************************************************************************/
/**
* function name   : CO_SerialDebug_Init
* description      : Initialise the DEBUG serial port
*
* @param - const char *fmt - pointer to string
* @param - ... - List of arguments
*
* @return - error code
*
* CAUTION:    The maximum number of bytes to TX'd is 128.
*/
ErrorCode_t CO_SerialDebug_Init(void)
{
   ErrorCode_t ErrorCode = ERR_INIT_FAIL_E;

   /* Initialise the Debug Port */
   if ( UART_SUCCESS_E == SerialPortInit(SERIAL_DEBUG_UART_PORT) )
   {
      /* The debug port access is protected by a Mutex to avoid concurrent 
       *  accesses to the Tx Buffer 
       */
      DebugPortMutexId = osMutexCreate(osMutex(DebugPortMutex));
      if(DebugPortMutexId)
      {
         ErrorCode = SUCCESS_E;
      }
      else
      {
         Error_Handler("Failed to create Debug mutex");
      }
   }
   
   return ErrorCode;
}

/*************************************************************************************/
/**
* function name   : SerialDebug_Print
* description      : Custom printf function to TX data over the debug uart
*
* @param - debug_category_t cat - category of the message to be printed
* @param - const char *fmt - pointer to string
* @param - ... - List of arguments
*
* @return - SerialPortOperationResult_t
*
* CAUTION:    The maximum number of bytes to TX'd is 128.
*/

ErrorCode_t SerialDebug_Print(const UartComm_t port, const debug_category_t cat, const char *fmt, ...)
{
   static char m_string[SERIAL_PORT_0_TX_BUFFER_SIZE];
   ErrorCode_t result = ERR_MESSAGE_FAIL_E ;
   va_list argp;
   
   if ( fmt )
   {
      if (osMutexWait(DebugPortMutexId, 10u) == osOK) 
      {
         switch(cat)
         {
            case DBG_SYSTEM_E:
               strcpy(m_string, "+SYS:");
               break;
            case DBG_INFO_E:
               strcpy(m_string, "+INF:");
               break;
            case DBG_BIT_E:
               strcpy(m_string, "+BIT:");
               break;
            case DBG_ERROR_E:
               strcpy(m_string, "+ERR:");
               break;
            case DBG_DATA_E:
               strcpy(m_string, "+DAT:");
               break;
            default:
               strcpy(m_string, ":");
               break;
         }
         __va_start(argp, fmt);
         
         if(0 < vsprintf(&m_string[strlen(m_string)], fmt, argp))
         {
               SerialPortOperationResult_t op_result = SerialPortWriteTxBuffer(port, (uint8_t*)m_string, strlen(m_string));

               if (op_result == UART_SUCCESS_E)
               {
                  /* Everything is ok */
                  result = SUCCESS_E;
               }
         }

         __va_end(argp);
         
         /* Release the Mutex */
         osMutexRelease(DebugPortMutexId);
      }
   }
   
   return result;
}

