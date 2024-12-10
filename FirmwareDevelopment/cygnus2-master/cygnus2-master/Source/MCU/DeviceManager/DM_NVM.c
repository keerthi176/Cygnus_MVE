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
*  File         : DM_NVM.c
*
*  Description  :
*
*   This module provides functionality for:
*     - Reading parameter values from Flash Emulated EEPROM
*     - Writing parameter values from Flash Emulated EEPROM
*
*************************************************************************************/



/* System Include files
*******************************************************************************/
#include <string.h>

/* User Include files
*******************************************************************************/
#include "cmsis_os.h"
#include "stm32l4xx.h"
#include "eeprom_emul.h"
#include "DM_NVM_cfg.h"
#include "DM_NVM.h"
#include "CO_SerialDebug.h"
#include "CO_Message.h"

/* Global Variables
*******************************************************************************/
uint32_t gNVMFailPoint = 0;//!!DEBUG CYG2-1019

/* This is a place holder for the Flash emulated EEPROM which serves as check for
 * the allocated size at the build time
 */
#ifndef WIN32
static const uint8_t dm_nvm_emulated_e2p_place_holder[FLASH_PAGE_SIZE][PAGES_NUMBER] __attribute__( ( section( "FlashEmulatedE2P"),used) ) ;
#endif

/* Mutex to garantee exclusive access */
osMutexId(NVM_MutexId) = NULL;
osMutexDef(NVM_Mutex);
extern osPoolId AppPool;
extern osMessageQId(AppQ);

/*************************************************************************************/
/**
* function name: DM_NVMInit
* description  : Initialisation routine of this module
*
* @param - none
*
* @return - ErrorCode_t : 0 for success, else error value
*/
ErrorCode_t DM_NVMInit(void)
{
   uint16_t data_idx;
   ErrorCode_t ret = SUCCESS_E;
   EE_Status ee_status = EE_OK;

   if(NULL == NVM_MutexId)
   {
      /* Create the Mutex */
      NVM_MutexId = osMutexCreate(osMutex(NVM_Mutex));
      if(NULL == NVM_MutexId)
      {
         Error_Handler("Failed to create NVM mutex");
      }
   }
   
   /* Check that the data definition is valid */
   for (data_idx = 0u; data_idx < NV_MAX_PARAM_ID_E; data_idx++)
   {
      if ( NULL == DM_NVM_DefaultValues[data_idx])
      {
         /* No default value, return Initialization Error */
         ret = ERR_INIT_FAIL_E;
      }

      /* Check any overlapping */
      if (data_idx < NV_MAX_PARAM_ID_E - 1u)
      {
         for (uint16_t sub_idx = 1u; sub_idx < (NV_MAX_PARAM_ID_E-data_idx); sub_idx++)
         {
            if ((DM_NVM_ParameterVirtualAddr[data_idx] + DM_NVM_ParameterSize[data_idx]) > DM_NVM_ParameterVirtualAddr[data_idx+sub_idx])
            {
               /* Initialization Error */
               ret = ERR_INIT_FAIL_E;
            }
         }
      }
   }

   if (ret == SUCCESS_E)
   {
      /* Enable and set FLASH Interrupt priority */
      /* FLASH interrupt is used for the purpose of pages clean up under interrupt */
      HAL_NVIC_SetPriority(FLASH_IRQn, 0, 0);
      HAL_NVIC_EnableIRQ(FLASH_IRQn);

      /* Unlock the Flash Program Erase controller */
      HAL_FLASH_Unlock();

      /* Clear all FLASH flags */
      __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR | FLASH_FLAG_OPTVERR |
                            FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR | FLASH_FLAG_RDERR );
      
      /* Initialize EEPROM emulation driver (mandatory) */
      ee_status = EE_Init((uint16_t*)DM_NVM_ParameterVirtualAddr, EE_FORCED_ERASE);

      if(ee_status != EE_OK) 
      {
         /* The Flash Emulated EEPROM area is probably blank, doa formatting */
         
         /* Clear all FLASH flags First */
         __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR | FLASH_FLAG_OPTVERR |
                            FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR | FLASH_FLAG_RDERR );

         /* Format flash pages used for EEPROM */         
         ee_status = EE_Format(EE_FORCED_ERASE);

         if(ee_status != EE_OK) 
         {
            ret = ERR_INIT_FAIL_E;
         }
         else
         {
            /* Initialize EEPROM emulation driver (mandatory) */
            ee_status = EE_Init((uint16_t*)DM_NVM_ParameterVirtualAddr, EE_FORCED_ERASE);
            
            if(ee_status != EE_OK) 
            {
               ret = ERR_INIT_FAIL_E;
            }
         }         
      }
      HAL_FLASH_Lock();

      /* Check if a data is not valid or not present then write its default value */
      if(SUCCESS_E == ret) 
      {
         for(uint16_t idx=0u; idx<NV_MAX_PARAM_ID_E; idx++)
         {
            /* Try reading this index */
            ErrorCode_t read_res = DM_NVMRead((DM_NVMParamId_t)idx, NULL, DM_NVM_ParameterSize[idx]*4u);
            
            /* Check if it is valid */
            if (SUCCESS_E != read_res)
            {
               /* No valid entry, write the default value */
               DM_NVMWrite((DM_NVMParamId_t)idx, (void*)DM_NVM_DefaultValues[idx], DM_NVM_ParameterSize[idx]*4u);
            }
         }
      }
   }

   return ret;
}

/*************************************************************************************/
/**
* function name : DM_NVMRead
* description   : Routine to read a single parameter value from the on-chip EEPROM
*
* @param - DM_NVMParamId_t paramId : parameter identifier
* @param - void *pValue : location to put value
* @param - size: size of the data block
*
* @return - ErrorCode_t : 0 for success, else error value
*/
ErrorCode_t DM_NVMRead(const DM_NVMParamId_t paramId, void *const pValue, const uint16_t size)
{
   uint16_t index;
   uint32_t value;
   uint8_t *pBuffer = (uint8_t*)pValue;
   ErrorCode_t ret = ERR_INVALID_PARAMETER_E;
   EE_Status ee_status = EE_OK;
   
   if (osOK == osMutexWait(NVM_MutexId, 1)) 
   {
         ret = SUCCESS_E;
      /* check input parameters */
      if ( (NV_MAX_PARAM_ID_E <= paramId) || (size != (DM_NVM_ParameterSize[paramId]*4u)))
      {
         ret = ERR_OUT_OF_RANGE_E;
      }
      else
      {
         for (index = 0u; index < DM_NVM_ParameterSize[paramId]; index++)
         {
            /* Read the actual value */
            ee_status |= EE_ReadVariable32bits(DM_NVM_ParameterVirtualAddr[paramId] + index, &value);
            
            /* Populate the destination buffer*/
            /* Copy each word separately */
            if (pBuffer)
            {
               memcpy((void*)(pBuffer + (index*4u)), (void*)&value, 4u);
            }
         }

         if (EE_OK != ee_status)
         {
            ret = ERR_DATA_INTEGRITY_CHECK_E;
         }
      }
      osMutexRelease(NVM_MutexId);
   }

   return ret;
}


/*************************************************************************************/
/**
* function name: DM_NVMWrite
* description  : Routine to write a single parameter value to the on-chip EEPROM
*
* @param - DM_NVMParamId_t paramId : parameter identifier
* @param - void *pvalue : pointer to the value to write
* @param - size: size of the data block
*
* @return - ErrorCode_t : 0 for success, else error value
*/
ErrorCode_t DM_NVMWrite(const DM_NVMParamId_t paramId, const void *const pValue, const uint16_t size)
{
   uint16_t index;
   uint32_t value;
   uint8_t *pBuffer = (uint8_t*)pValue;
   ErrorCode_t ret = ERR_NO_RESOURCE_E;
   EE_Status ee_status = EE_OK;
   
   /* check input parameters */
   if (NV_MAX_PARAM_ID_E <= paramId)
   {
      return ERR_OUT_OF_RANGE_E;
   }
 
   if ((NULL == pValue) || (size != (DM_NVM_ParameterSize[paramId]*4u)))
   {
      ret = ERR_INVALID_POINTER_E;
   }
   else
   {
      if (osOK == osMutexWait(NVM_MutexId, 1)) 
      {
         ret = SUCCESS_E;
         /* Unlock the Flash Program controller */
         HAL_FLASH_Unlock();

         /* Clear all FLASH flags First */
         __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR | FLASH_FLAG_OPTVERR |
                               FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR | FLASH_FLAG_RDERR );

         for (index = 0u; index < DM_NVM_ParameterSize[paramId]; index++)
         {
            /* Read the actual value */
            memcpy((void*)&value, (void*)(pBuffer + (index*4u)), 4u);
            ee_status = EE_WriteVariable32bits(DM_NVM_ParameterVirtualAddr[paramId] + index, value);
            
            if ( EE_CLEANUP_REQUIRED == ee_status)
            {
               ee_status = EE_CleanUp();
               CO_PRINT_B_1(DBG_INFO_E,"NVM Clean-up complete.  %s\r\n", EE_OK==ee_status ? "OK":"FAIL");
               //Tell the application we just cleaned-up so it can check for synch loss
               CO_Message_t* pMessage = osPoolAlloc(AppPool);
               if (pMessage)
               {
                  pMessage->Type = CO_MESSAGE_NVM_CLEANUP_E;
                  
                  osStatus osStat = osMessagePut(AppQ, (uint32_t)pMessage, 0);
                  
                  if (osOK != osStat)
                  {
                     /* failed to put message in the head queue */
                     osPoolFree(AppPool, pMessage);
                  }
               }
            }
            else if ( EE_OK != ee_status )
            {
               CO_PRINT_B_1(DBG_INFO_E,"NVM Write Failed.  Error=%d\r\n", ee_status);
            }
         }
         

         /* Lock the Flash Program controller */
         HAL_FLASH_Lock();

         if (EE_OK != ee_status)
         {
            ret = ERR_DATA_INTEGRITY_CHECK_E;
            char error_buffer[128];
            sprintf(error_buffer,"Flash Write FAIL. paramId %d, error code %d, gNVMFailPoint %d\r\n", (int)paramId, ee_status, gNVMFailPoint);
            Error_Handler(error_buffer);
         }
    
         osMutexRelease(NVM_MutexId);
      }
   }
   return ret;
}

/*************************************************************************************/
/**
* function name: DM_NVM_ChannelIndexToNVMParameter
* description  : Routine to translate a channel number to an NVM index
*
* @param - channel_index      The channel index to be translated into a NVM index
*
* @return - DM_NVMParamId_t : The NVM parameter index, or NV_MAX_PARAM_ID_E if there is no valid translation.
*/
DM_NVMParamId_t DM_NVM_ChannelIndexToNVMParameter(const uint32_t channel_index)
{
   DM_NVMParamId_t nvmParameter = NV_MAX_PARAM_ID_E; //invalid value
   
   if ( CO_CHANNEL_MAX_E > channel_index )
   {
      CO_ChannelIndex_t channel = (CO_ChannelIndex_t)channel_index;
      
      switch ( channel )
      {
         case CO_CHANNEL_SMOKE_E:
            nvmParameter = NV_CHANNEL_CONFIG_SMOKE_E;
            break;
         case CO_CHANNEL_HEAT_B_E:
            nvmParameter = NV_CHANNEL_CONFIG_HEAT_B_E;
            break;
         case CO_CHANNEL_CO_E:
            nvmParameter = NV_CHANNEL_CONFIG_CO_E;
            break;
         case CO_CHANNEL_PIR_E:
            nvmParameter = NV_CHANNEL_CONFIG_PIR_E;
            break;
         case CO_CHANNEL_SOUNDER_E:
            nvmParameter = NV_CHANNEL_CONFIG_SOUNDER_E;
            break;
         case CO_CHANNEL_BEACON_E:
            nvmParameter = NV_CHANNEL_CONFIG_BEACON_E;
            break;
         case CO_CHANNEL_FIRE_CALLPOINT_E:
            nvmParameter = NV_CHANNEL_CONFIG_FIRE_CALLPOINT_E;
            break;
         case CO_CHANNEL_STATUS_INDICATOR_LED_E:
            nvmParameter = NV_CHANNEL_CONFIG_STATUS_LED_E;
            break;
         case CO_CHANNEL_VISUAL_INDICATOR_E:
            nvmParameter = NV_CHANNEL_CONFIG_VISUAL_INDICATOR_E;
            break;
         case CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E:
            nvmParameter = NV_CHANNEL_CONFIG_VISUAL_INDICATOR_COMBINED_E;
            break;
         case CO_CHANNEL_MEDICAL_CALLPOINT_E:
            nvmParameter = NV_CHANNEL_CONFIG_MEDICAL_CALLPOINT_E;
            break;
         case CO_CHANNEL_EVAC_CALLPOINT_E:
            nvmParameter = NV_CHANNEL_CONFIG_EVAC_CALLPOINT_E;
            break;
         case CO_CHANNEL_OUTPUT_ROUTING_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_ROUTING_E;
            break;
         case CO_CHANNEL_INPUT_1_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_1_E;
            break;
         case CO_CHANNEL_INPUT_2_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_2_E;
            break;
         case CO_CHANNEL_INPUT_3_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_3_E;
            break;
         case CO_CHANNEL_INPUT_4_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_4_E;
            break;
         case CO_CHANNEL_INPUT_5_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_5_E;
            break;
         case CO_CHANNEL_INPUT_6_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_6_E;
            break;
         case CO_CHANNEL_INPUT_7_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_7_E;
            break;
         case CO_CHANNEL_INPUT_8_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_8_E;
            break;
         case CO_CHANNEL_INPUT_9_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_9_E;
            break;
         case CO_CHANNEL_INPUT_10_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_10_E;
            break;
         case CO_CHANNEL_INPUT_11_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_11_E;
            break;
         case CO_CHANNEL_INPUT_12_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_12_E;
            break;
         case CO_CHANNEL_INPUT_13_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_13_E;
            break;
         case CO_CHANNEL_INPUT_14_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_14_E;
            break;
         case CO_CHANNEL_INPUT_15_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_15_E;
            break;
         case CO_CHANNEL_INPUT_16_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_16_E;
            break;
         case CO_CHANNEL_INPUT_17_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_17_E;
            break;
         case CO_CHANNEL_INPUT_18_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_18_E;
            break;
         case CO_CHANNEL_INPUT_19_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_19_E;
            break;
         case CO_CHANNEL_INPUT_20_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_20_E;
            break;
         case CO_CHANNEL_INPUT_21_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_21_E;
            break;
         case CO_CHANNEL_INPUT_22_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_22_E;
            break;
         case CO_CHANNEL_INPUT_23_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_23_E;
            break;
         case CO_CHANNEL_INPUT_24_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_24_E;
            break;
         case CO_CHANNEL_INPUT_25_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_25_E;
            break;
         case CO_CHANNEL_INPUT_26_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_26_E;
            break;
         case CO_CHANNEL_INPUT_27_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_27_E;
            break;
         case CO_CHANNEL_INPUT_28_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_28_E;
            break;
         case CO_CHANNEL_INPUT_29_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_29_E;
            break;
         case CO_CHANNEL_INPUT_30_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_30_E;
            break;
         case CO_CHANNEL_INPUT_31_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_31_E;
            break;
         case CO_CHANNEL_INPUT_32_E:
            nvmParameter = NV_CHANNEL_CONFIG_INPUT_32_E;
            break;
         case CO_CHANNEL_OUTPUT_1_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_1_E;
            break;
         case CO_CHANNEL_OUTPUT_2_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_2_E;
            break;
         case CO_CHANNEL_OUTPUT_3_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_3_E;
            break;
         case CO_CHANNEL_OUTPUT_4_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_4_E;
            break;
         case CO_CHANNEL_OUTPUT_5_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_5_E;
            break;
         case CO_CHANNEL_OUTPUT_6_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_6_E;
            break;
         case CO_CHANNEL_OUTPUT_7_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_7_E;
            break;
         case CO_CHANNEL_OUTPUT_8_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_8_E;
            break;
         case CO_CHANNEL_OUTPUT_9_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_9_E;
            break;
         case CO_CHANNEL_OUTPUT_10_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_10_E;
            break;
         case CO_CHANNEL_OUTPUT_11_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_11_E;
            break;
         case CO_CHANNEL_OUTPUT_12_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_12_E;
            break;
         case CO_CHANNEL_OUTPUT_13_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_13_E;
            break;
         case CO_CHANNEL_OUTPUT_14_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_14_E;
            break;
         case CO_CHANNEL_OUTPUT_15_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_15_E;
            break;
         case CO_CHANNEL_OUTPUT_16_E:
            nvmParameter = NV_CHANNEL_CONFIG_OUTPUT_16_E;
            break;
         case CO_CHANNEL_HEAT_A1R_E:
            nvmParameter = NV_CHANNEL_CONFIG_HEAT_A1R_E;
            break;
         default:
         break;
      }
   }
      
   return nvmParameter;
}

