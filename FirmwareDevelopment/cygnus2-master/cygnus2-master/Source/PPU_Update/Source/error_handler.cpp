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
*  File         : error_handler.cpp
*
*  Description  : Implementation of 'reset on error'
*
*************************************************************************************/
#include "common.h"
#include "mbed.h"

extern char sw_reset_debug_message[126];
extern uint32_t sw_reset_msg_indication_flag;
extern uint32_t rbu_pp_mode_request;

/*******************************************************************************
Function name : void Error_Handler(void)
   returns    : nothing
Description   : This function is executed in case of error occurrence.
Notes         : restrictions, odd modes
*******************************************************************************/
void Error_Handler(char* error_msg)
{
   /* Capture debug information */
   if (strlen(error_msg) < (sizeof(sw_reset_debug_message) - 1u))
   {
      /* Copy the error message in the non initialised area */
      strcpy(sw_reset_debug_message, error_msg);
   }
   else
   {
      /* Copy the error message in the non initialised area 
       * Message is too long so truncate it
       */
      memcpy(sw_reset_debug_message, error_msg, sizeof(sw_reset_debug_message) - 1);
      sw_reset_debug_message[sizeof(sw_reset_debug_message) - 1] = '\0';
   }

   /* Set the Indication Flag */
   sw_reset_msg_indication_flag = SW_RESET_FLAG_KEY;
   
   /* This is an Error reset, skip the PP operation next start-up */
   rbu_pp_mode_request = RBU_SKIP_PP_MODE_REQ;
   
   /* Wait for the end of any pending EEPROM writing operation */
//   FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
   
   //a delay for all debug messages to be sent
   wait_ms(1000);

   /* Reset the system now */
   HAL_NVIC_SystemReset();
}
