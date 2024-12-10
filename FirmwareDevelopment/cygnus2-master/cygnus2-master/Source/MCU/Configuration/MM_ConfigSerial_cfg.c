/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2018 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : MM_ConfigSerial_cfg.c
*
*  Description  :configuration of the Config Serial Module
*
*************************************************************************************/



/* System Include files
*******************************************************************************/


/* User Include files
*******************************************************************************/
#include "stm32l4xx.h"
#include "MM_ConfigSerialTask.h"
#include "MM_ConfigSerial_cfg.h"

/* Private Functions Prototypes
*************************************************************************************/


/* Global Variables
*************************************************************************************/

const bool ConfigSerial_ChannelDef[CS_MAX_CONFIGURATIONS_E][MAX_SUPPORTED_SERIAL_PORT_E] =
{
   [CS_RBU_CONFIGURATION_E] = 
   {
      /* RBU config */
      [DEBUG_UART_E]   = true,
      [PPU_UART_E]     = true,
      [HEAD_UART_E]    = false,// This channel is directly managed by the Head Interface module
      [NCU_CP_UART_E]  = false,
      [NCU_SBC_UART_E] = false,
   },

   [CS_NCU_CONFIGURATION_E] =    
   {
      /* NCU config */
      [DEBUG_UART_E]   = true,
      [PPU_UART_E]     = true,
      [HEAD_UART_E]    = false,
      [NCU_CP_UART_E]  = false,
      [NCU_SBC_UART_E] = false,
   },
};

/* Private Variables
*************************************************************************************/

