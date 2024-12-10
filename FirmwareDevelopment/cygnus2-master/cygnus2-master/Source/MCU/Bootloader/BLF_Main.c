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
*  File         : MM_Main.c
*
*  Description  : main routines
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <string.h>

/* User Include Files
*************************************************************************************/
#include "stm32l4xx.h"
#include "stm32l4xx_hal_def.h"
#include "eeprom_emul.h"
#ifndef WIN32
#include "flash_interface.h"
#endif
#include "CO_ErrorCode.h"
#include "utilities.h"
#include "BLF_serial_if.h"
#include "DM_NVM.h"
#include "DM_Crc.h"
#include "DM_RNG.h"
#include "DM_SystemClock.h"
#include "BLF_Boot.h"
#include "BLF_AppUpdater.h"


/* External Functions Prototypes
*************************************************************************************/
extern ErrorCode_t DM_CrcInit(void);

/* Private Functions Prototypes
*************************************************************************************/
static void BLF_MainInitialise(void);

/* Global Variables
*************************************************************************************/

/* CRC handler declaration */
CRC_HandleTypeDef Ymodem_CrcHandle =
{
   .Instance = CRC,

   .Init =
   {
      .DefaultPolynomialUse    = DEFAULT_POLYNOMIAL_DISABLE,
      .GeneratingPolynomial    = 0x1021,
      .CRCLength               = CRC_POLYLENGTH_16B,
      
      /* The zero init value is used */
      .DefaultInitValueUse     = DEFAULT_INIT_VALUE_DISABLE,
      .InitValue               = 0,
      
      /* The input data are not inverted */
      .InputDataInversionMode  = CRC_INPUTDATA_INVERSION_NONE,
      
      /* The output data are not inverted */
      .OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE,
   },

   /* The input data are 32-bit long words */
   .InputDataFormat              = CRC_INPUTDATA_FORMAT_BYTES,
};



/* Private Variables
*************************************************************************************/

/* Checksum value added in the SW image during the build process */
const uint32_t blf_application_checksum __attribute__( ( section( "ApplicationChecksum"),used) );
const uint32_t blf_bootloader_area __attribute__( ( section( "bootloader"),used) );

/*************************************************************************************/
/**
* main
* main routine for MCU
*
* @param - none
*
* @return - returns zero

*/
int32_t main (void)
{
   /* Initialise the Micro and peripherals */
   BLF_MainInitialise();
   
   /* Call the Boot Manager, if a valid application Image is present then it 
    * will be executed.
    */
   BLF_BootManager();
}

/*************************************************************************************/
/**
* BLF_MainInitialise
* Initialise this application
*
* @param - none
*
* @return - none

*/
static void BLF_MainInitialise(void)
{    
   /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
   HAL_Init();

   /* Configure the system clock */
   DM_SystemClockResetConfig();

  /* Initializes the RNG peripheral */
//  if (  (SUCCESS_E != DM_RNG_Deinitialise())
//     || (SUCCESS_E != DM_RNG_Initialise()) )
//  {
//    /* Initialization Error */
//    HAL_NVIC_SystemReset();
//  }

   /* Initialise the CRC module */
   DM_CrcInit();
  
  /* Initialise the NVM Device Manager */
   DM_NVMInit();
  
   /*## Configure the USART peripheral #######################################*/
   serial_init();
}


