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
*  File         : BLF_AppUpdater.c
*
*  Description  : Updates the application image in the Flash through the serial link or 
*                 pre-loaded image through the wireless link
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
#include "eeprom_emul.h"

#include "cmsis_os.h"

#ifdef WIN32
#include "STWin32_stubs.h"
#else
#include "common.h"
#include "flash_interface.h"
//#include "simple.h"
#include "BLF_Configuration.h"
#endif
#include "CO_ErrorCode.h"
#include "DM_NVM.h"
#include "BLF_serial_if.h"
#include "BLF_Main.h"
#include "BLF_Boot.h"
#include "BLF_AppUpdater.h"
#include "simple.h"

/* Private Functions Prototypes
*************************************************************************************/
static blf_app_update_result_t BLF_SerialLoaderUpdater(void);

/* Global Variables
*************************************************************************************/
extern osMutexId(CRCMutexId);


/* Private Variables
*************************************************************************************/

extern HAL_StatusTypeDef FLASH_WaitForLastOperation(uint32_t Timeout);

/************************************************************************************/
/**
* function name : BLF_LoaderUpdater
* description   : Main function of the Application Loader Updater module
*
* @param - reprogramming_mode
*
* @return - none
*/
void BLF_LoaderUpdater(reprogramming_mode_t reprogramming_mode)
{
   blf_app_update_result_t app_update_result = APP_UPDATE_FAIL_E;

   /* Initialise Flash */
   FLASH_If_Init();

   /* Go to the serial reprogramming procedure */
   app_update_result =  BLF_SerialLoaderUpdater();

   /* Inform the user of the final status */
   if (APP_UPDATE_SUCCESS_E == app_update_result)
   {
      Serial_PutString((uint8_t *)"\r\n\nBOOTLOADER: Serial Firmware Update Successful\r\n\n");

//      if ( REPROG_RESCUE_MODE_E == reprogramming_mode)
      {
         /* We are in a rescue mission, auto switch to the other bank */
         BLF_set_boot_bank(BLF_get_boot_bank() == 1u ? 2u : 1u, true);
      }
   }
   else
   {
      Serial_PutString((uint8_t *)"\r\n\nBOOTLOADER: Serial Firmware Update Failed \r\n\n");
   }

   /* Wait for any pending EEPROM write operation */
   FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
   
   /* Reset the system now */
   HAL_NVIC_SystemReset();
}

/************************************************************************************/
/**
* function name : BLF_SerialLoaderUpdater
* description   : Perform the Application Download and the Flash update
*
* @param - none
*
* @return - Result of the operation
*/
static blf_app_update_result_t BLF_SerialLoaderUpdater(void)
{
   blf_app_update_result_t app_update_result = APP_UPDATE_FAIL_E;

   #ifdef BLF_ENABLE_DEBUG_MENU_DISPLAY

   /* Display main menu */
   Main_Menu();
   #else   
   uint32_t program_size = 0u;
   COM_StatusTypeDef result;

   /* The CRC-16-CCIT polynomial is used by the Ymodem */
   if (HAL_OK != HAL_CRC_Init(&Ymodem_CrcHandle))
   {
      /* Initialisation Error */
      HAL_NVIC_SystemReset();
   }

   /* Reset the Data Register */
   __HAL_CRC_DR_RESET(&Ymodem_CrcHandle);

   /* Test from which bank the program runs */
   uint32_t BankActive = READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE);

   /* Send message to the Portable Programmer to start transmitting */
   Serial_PutString((uint8_t *)"\r\nBOOTLOADER: Waiting for the Application Image\r\n");
   
   /* Download the Application */
	result = Simple_Receive( &program_size, BankActive);
   if (COM_OK_E == result)
   {
      /* Check the Validity of the new application */
//      if ( VALID_APPLICATION_PRESENT_E == check_other_bank_app_validity(program_size, BankActive))
//      {
         /* Update performed successfully */
         app_update_result = APP_UPDATE_SUCCESS_E;
//      }
   }   
   #endif 

   return app_update_result;
}

/************************************************************************************/
/**
* function name : check_other_bank_app_validity
* description   : Check the validity of an application
*
* @param - program_size: Size of the Flash area to be verified
*
* @return - result of the check
*/
blf_application_status_t check_other_bank_app_validity(uint32_t program_size, uint32_t bank )
{
   uint32_t flashdestination;
   blf_application_status_t application_status = NO_VALID_APPLICATION_E;   

   /* Initialize flashdestination variable */
   if ( 1 == bank )
  {
    flashdestination = FLASH_START_BANK1;
  }
  else
  {
    flashdestination = FLASH_START_BANK2;
  }

   #if defined _DEBUG && !defined WIN32
   /* In Debug mode just check the Top of stack address in the Vector table */
   if (((*(__IO uint32_t*)flashdestination) & 0x2FFE0000 ) == 0x20000000)
   {
      application_status = VALID_APPLICATION_PRESENT_E;
   }
   
   uint32_t firmware_crc = *(uint32_t*)(flashdestination + program_size - 4u);
   DM_NVMWrite(NV_BANK2_CRC_E, &firmware_crc, sizeof(uint32_t));
   
   #else
   #ifdef BLF_USE_CRC
   uint32_t calculated_crc;
   uint32_t expected_crc;   
   
   /* Program Integrity CRC handle declaration */
   CRC_HandleTypeDef ProgramCrcHandle =
   {
      .Instance = CRC,

      /* The default polynomial is used */
      .Init.DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_ENABLE,

      /* The default init value is used */
      .Init.DefaultInitValueUse     = DEFAULT_INIT_VALUE_ENABLE,

      /* The input data are not inverted */
      .Init.InputDataInversionMode  = CRC_INPUTDATA_INVERSION_NONE,

      /* The output data are not inverted */
      .Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE,

      /* The input data are 32-bit long words */
      .InputDataFormat              = CRC_INPUTDATA_FORMAT_WORDS,
   };

      /* Load the NVM Data CRC Handler */
   if (HAL_OK != HAL_CRC_Init(&ProgramCrcHandle))
   {
      /* Initialisation Error */
      HAL_NVIC_SystemReset();
   }
   
   /* Read the CRC save within with the application */
   expected_crc = *(uint32_t*)(flashdestination + program_size - 4u); 
   
   if (osOK == osMutexWait(CRCMutexId, 1)) 
   {
      /* Calculate the CRC on the whole application area */
      calculated_crc = HAL_CRC_Calculate(&ProgramCrcHandle, (uint32_t*)flashdestination, (program_size - 4u)/4u);
      
      if (calculated_crc == expected_crc)
      {
         /* Test the Top of stack address in the Vector table */
         if (((*(__IO uint32_t*)flashdestination) & 0x2FFE0000 ) == 0x20000000)
         {
            application_status = VALID_APPLICATION_PRESENT_E;
         }
      }
      osMutexRelease(CRCMutexId);
   }
      #else
      const uint32_t expected_checksum = *(uint32_t*)(flashdestination + program_size - 4u);
      uint32_t calculated_checksum = 0u;
         
   if (osOK == osMutexWait(CRCMutexId, 1)) 
   {
      for (uint32_t idx = 0; idx < (program_size -4u)/4u; idx++)
      {
         uint32_t val = *(uint32_t*)(flashdestination + (idx * 4));
         calculated_checksum +=  val & 0xFF;
         calculated_checksum += (val >> 8 ) & 0xFF;
         calculated_checksum += (val >> 16) & 0xFF;
         calculated_checksum += (val >> 24) & 0xFF;
      }

      if (calculated_checksum == expected_checksum)
      {
         /* Test the Top of stack address in the Vector table */
         if (((*(__IO uint32_t*)flashdestination) & 0x2FFE0000 ) == 0x20000000)
         {
            application_status = VALID_APPLICATION_PRESENT_E;
         }
      }
      osMutexRelease(CRCMutexId);
   }

   #endif
   #endif /* _DEBUG */

   return application_status;
}
