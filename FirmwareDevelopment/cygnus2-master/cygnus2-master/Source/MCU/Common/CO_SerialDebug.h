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
*  File         : CO_SerialDebug.h
*
*  Description  : Include file for the Serial Debug fature
*
*************************************************************************************/

#ifndef CO_SERIAL_DEBUG_H
#define CO_SERIAL_DEBUG_H


/* System Include Files
*************************************************************************************/
#include <string.h>

/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"
#include "DM_SerialPort_cfg.h"

/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/
typedef enum
{
   DBG_SYSTEM_E,
   DBG_INFO_E,
   DBG_BIT_E,
   DBG_ERROR_E,
   DBG_DATA_E,
   DBG_NOPREFIX_E
}debug_category_t;

/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t CO_SerialDebug_Init(void);
ErrorCode_t SerialDebug_Print(const UartComm_t port, const debug_category_t cat, const char *fmt, ...);

/* Public Constants
*************************************************************************************/


/* Macros
*************************************************************************************/

#define PRINT_CMD SerialDebug_Print

// 1st-priority non-maskable prints
   #define CO_PRINT_A_0(category, s) PRINT_CMD(DEBUG_UART_E, category, s)
   #define CO_PRINT_A_1(category, s, x1) PRINT_CMD(DEBUG_UART_E, category, s, x1)
   #define CO_PRINT_A_2(category, s, x1, x2) PRINT_CMD(DEBUG_UART_E, category, s, x1, x2)
   #define CO_PRINT_A_3(category, s, x1, x2, x3) PRINT_CMD(DEBUG_UART_E, category, s, x1, x2, x3)
   #define CO_PRINT_A_4(category, s, x1, x2, x3, x4) PRINT_CMD(DEBUG_UART_E, category, s, x1, x2, x3, x4)
   #define CO_PRINT_A_5(category, s, x1, x2, x3, x4, x5) PRINT_CMD(DEBUG_UART_E, category, s, x1, x2, x3, x4, x5)

   #define CO_PRINT_PORT_0(port, s) PRINT_CMD(port, DBG_INFO_E, s)
   #define CO_PRINT_PORT_1(port, s, x1) PRINT_CMD(port, DBG_INFO_E, s, x1)
   #define CO_PRINT_PORT_2(port, s, x1, x2) PRINT_CMD(port, DBG_INFO_E, s, x1, x2)
   #define CO_PRINT_PORT_3(port, s, x1, x2, x3) PRINT_CMD(port, DBG_INFO_E, s, x1, x2, x3)
   #define CO_PRINT_PORT_4(port, s, x1, x2, x3, x4) PRINT_CMD(port, DBG_INFO_E, s, x1, x2, x3, x4)
   #define CO_PRINT_PORT_5(port, s, x1, x2, x3, x4, x5) PRINT_CMD(port, DBG_INFO_E, s, x1, x2, x3, x4, x5)

// 2nd-priority maskable prints
#if defined _DEBUG
   // 2nd-level maskable prints
   #define CO_PRINT_B_0(category, s) PRINT_CMD(DEBUG_UART_E, category, s)
   #define CO_PRINT_B_1(category, s, x1) PRINT_CMD(DEBUG_UART_E, category, s, x1)
   #define CO_PRINT_B_2(category, s, x1, x2) PRINT_CMD(DEBUG_UART_E, category, s, x1, x2)
   #define CO_PRINT_B_3(category, s, x1, x2, x3) PRINT_CMD(DEBUG_UART_E, category, s, x1, x2, x3)
   #define CO_PRINT_B_4(category, s, x1, x2, x3, x4) PRINT_CMD(DEBUG_UART_E, category, s, x1, x2, x3, x4)
   #define CO_PRINT_B_5(category, s, x1, x2, x3, x4, x5) PRINT_CMD(DEBUG_UART_E, category, s, x1, x2, x3, x4, x5)
   #define CO_PRINT_B_6(category, s, x1, x2, x3, x4, x5, x6) PRINT_CMD(DEBUG_UART_E, category, s, x1, x2, x3, x4, x5, x6)

#else
   // 2nd-level maskable prints
   #define CO_PRINT_B_0(category, s) // Do nothing
   #define CO_PRINT_B_1(category, s, x1) // Do nothing
   #define CO_PRINT_B_2(category, s, x1, x2) // Do nothing
   #define CO_PRINT_B_3(category, s, x1, x2, x3) // Do nothing
   #define CO_PRINT_B_4(category, s, x1, x2, x3, x4) // Do nothing
   #define CO_PRINT_B_5(category, s, x1, x2, x3, x4, x5) // Do nothing
   #define CO_PRINT_B_6(category, s, x1, x2, x3, x4, x5, x6) // Do nothing

#endif


#endif // CO_SERIAL_DEBUG_H
