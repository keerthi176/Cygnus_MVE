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
*  File         : FBL_Boot.c
*
*  Description  : brief description
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* User Include Files
*************************************************************************************/
#include "stm32l4xx.h"
#include "DM_NVM.h"
#include "DM_RNG.h"
#include "eeprom_emul.h"
#ifdef WIN32
#include "STWin32_stubs.h"
#else
#include "common.h"
#include "flash_interface.h"
#include "BLF_Configuration.h"
#endif
#include "CO_ErrorCode.h"
#include "BLF_serial_if.h"
#include "MM_Main.h"
#include "BLF_Main.h"
#include "BLF_AppUpdater.h"
#include "BLF_Boot.h"


/* Private Functions Prototypes
*************************************************************************************/
typedef enum
{
   KEY_VERIFICATION_SUCCESS_E = 0,
   KEY_VERIFICATION_FAIL_E
}key_verification_status_t;

//CO_STATIC reprogramming_mode_t BLF_CheckReprogramRequest(void);
//#ifdef CF_BLF_ENABLE_BACK_DOOR_REPROGRAM
//CO_STATIC key_verification_status_t VerifyKeyfromSeed(uint32_t seed, uint32_t key);
//#endif


/* Global Variables
*************************************************************************************/
extern uint32_t reprogram_request_status;



/* Private Variables
*************************************************************************************/
#define REPROGRAM_REQUEST_CODE    0xA5A5A5A5

/*************************************************************************************/
/**
* BLF_BootManager
* Boot the Application or returns if reprogramming conditions are met
*
* @param - none
*
* @return - none

*/
void BLF_BootManager(void)
{  
//   reprogramming_mode_t reprogram_req_status;

//   reprogram_req_status = BLF_CheckReprogramRequest();

//   if (REPROG_NO_REQ_E == reprogram_req_status)
//   {
      /* De-initialise the Peripherals that are not need 
       * for the application or used differently 
       */
      __disable_irq();
      DM_RNG_Initialise();
      serial_deinit();
      __enable_irq();

      /* Now, start the application */
      app_main();
//   }
//   else
//   {
//      /* Call the LoaderUpdater Function to perform the reprogramming operation */
//      BLF_LoaderUpdater(reprogram_req_status);
//   }
}

/*************************************************************************************/
/**
* BLF_get_boot_bank
* Function to get the boot bank number
*
* @param - None
*
* @return - returns the Id of the active bank (1 or 2)
*/
uint32_t BLF_get_boot_bank(void)
{
   /* Check which bank are we running from */
   return READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE)?2u:1u;
}

/*************************************************************************************/
/**
* BLF_set_boot_bank
* Function to set the boot bank number
*
* @param - Bank          : Image Id (1 or 2)
* @param - update_eeprom : Set to true if the EEPROM data to be copied in the process
*
* @return - returns the status of the key check
*/
ErrorCode_t BLF_set_boot_bank(uint32_t Bank, bool update_eeprom)
{
   blf_application_status_t application_status;
   const uint32_t flash_app_size = (FLASH_START_BANK2-FLASH_START_BANK1) - (PAGES_NUMBER*FLASH_PAGE_SIZE);
   ErrorCode_t ErrorCode = SUCCESS_E;

   if((1u == Bank) || (2u == Bank))
   {
      if (Bank != FLASH_If_GetActiveBank())
      {
         /* Check the other Image Validity */
         application_status = check_other_bank_app_validity(flash_app_size, Bank );
         
         if (VALID_APPLICATION_PRESENT_E == application_status)
         {
            if (update_eeprom != false)
            {
               /* Copy the EEPROM Data used to by this bank to be used by the other one */
               if (EE_OK != EE_CopyDataToBank(Bank))
               {
                  /* Writing the data failed */
                  ErrorCode = ERR_INIT_FAIL_E;
               }
            }
            
            /* Check there were no errors */
            if (SUCCESS_E == ErrorCode)
            {
               /* Wait for the end of any pending EEPROM writing operation */
               FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);

               /* switch bank now */
               FLASH_If_BankSwitch();
                              
               /* The bank switching is supposed to trigger a reset to reload the 
                * option bytes but there are rare case where it doesn't
                * Force the micro to reset
                */
               HAL_NVIC_SystemReset();
            }
         }
         else
         {
            /* No valid application resides in the bank we are requested to switch to */
            ErrorCode = ERR_DATA_INTEGRITY_CHECK_E;
         }
      }              
   }
   
   return ErrorCode;
}

/*************************************************************************************/
/**
* CheckKeyfromSeed
* Function to verify the Key generated by the Client from a seed
*
* @param - none
*
* @return - returns the status of the key check
*/
void BLF_request_serial_programming(void)
{
   reprogram_request_status = REPROGRAM_REQUEST_CODE;
   
   /* Wait for the end of any pending EEPROM writing operation */
   FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
  
   /* Request a micro reset */
   HAL_NVIC_SystemReset();
}

///*************************************************************************************/
///**
//* BLF_CheckReprogramRequest
//* Chek if there is a reprogramming request in the NVM or eventually from the serial port
//*
//* @param - none
//*
//* @return - returns the reprogramming request status
//*/
//CO_STATIC reprogramming_mode_t BLF_CheckReprogramRequest(void)
//{
//   reprogramming_mode_t reprog_req = REPROG_NO_REQ_E;

//#ifdef CF_BLF_ENABLE_BACK_DOOR_REPROGRAM
//   char str_seed[48];
//   char str_key_msg[48];
//   char str_seed_msg[48] = "BOOTLOADER SEED = ";
//#endif
//   
//   /* Check if we are coming from the application following a reprogramming request */
//   if (RESET != __HAL_RCC_GET_FLAG(RCC_FLAG_BORRST))
//   {     
//      /* Consider as there is no reprogramming request */
//      reprog_req = REPROG_NO_REQ_E;
//   }
//   else
//   {
//      /* load the the request status for this cycle */
//      if (REPROGRAM_REQUEST_CODE == reprogram_request_status)
//      {
//         /* The reprogramming code is set */
//         reprog_req = REPROG_STANDARD_E;
//      }
//      else
//      {
//         /* No reprogramming code is not set */
//         reprog_req = REPROG_NO_REQ_E;
//      }      
//   }

//   /* Clear any previous request */
//   reprogram_request_status = 0;
//   
//   #ifdef CF_BLF_ENABLE_BACK_DOOR_REPROGRAM   
//   if (REPROG_NO_REQ_E == reprog_req)
//   {
//      HAL_StatusTypeDef hal_status;
//      uint32_t seed = DM_RNG_GetRandomNumber(0xFFFFFFFF);
//      uint32_t key;

//      if ( 0 != seed )
//      {
//         /* Send the seed on the UART*/
//         sprintf(str_seed, "0x%08X\r\n", seed);
//         strcat(str_seed_msg, str_seed);         
//         hal_status = serial_send_data((uint8_t*)str_seed_msg, strlen(str_seed_msg), CF_BLF_BACK_DOOR_KEY_WAIT_MS );
//         
//         if (HAL_OK == hal_status)
//         {
//            /* Wait 1s to receive the key */
//            hal_status = serial_receive_data((uint8_t*)&str_key_msg, 11u, CF_BLF_BACK_DOOR_KEY_WAIT_MS);

//            if (HAL_OK == hal_status)
//            {
//               //Ensure the programmer's response is null terminated
//               str_key_msg[10] = 0;
//               //Convert the text response key to an integer format.
//               if(Str2Int((uint8_t*)str_key_msg, &key) == 1)
//               {
//                  /* Check if the key is valid */
//                  if(SUCCESS_E == VerifyKeyfromSeed(seed, key))
//                  {
//                     /* We have been required to enter the reprogramming mode */
//                     reprog_req = REPROG_RESCUE_MODE_E;
//                  }
//               }
//            }
//         }
//      }
//   }
//   #endif

//   return reprog_req;
//}

///*************************************************************************************/
///**
//* CheckKeyfromSeed
//* Function to verify the Key generated by the Client from a seed
//*
//* @param - none
//*
//* @return - returns the status of the key check
//*/
//#ifdef CF_BLF_ENABLE_BACK_DOOR_REPROGRAM
//CO_STATIC key_verification_status_t VerifyKeyfromSeed(uint32_t seed, uint32_t key)
//{
//   if (CF_BLF_BACKDOOR_STATIC_KEY == key)
//   {
//      return KEY_VERIFICATION_SUCCESS_E;
//   }
//   else
//   {
//      return KEY_VERIFICATION_FAIL_E;
//   }
//}
//#endif
