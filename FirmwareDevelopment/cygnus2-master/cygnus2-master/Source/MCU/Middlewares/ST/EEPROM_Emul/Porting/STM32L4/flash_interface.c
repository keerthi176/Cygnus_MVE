/**
  ******************************************************************************
  * @file    EEPROM_Emul/Porting/STM32L4/flash_interface.c
  * @author  MCD Application Team
  * @brief   This file provides all the EEPROM emulation flash interface functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics International N.V.
  * All rights reserved.</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice,
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other
  *    contributors to this software may be used to endorse or promote products
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under
  *    this license is void and will automatically terminate your rights under
  *    this license.
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "eeprom_emul.h"
#include "flash_interface.h"

/** @addtogroup EEPROM_Emulation
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint32_t GetBankNumber(uint32_t Address);

/* Exported functions --------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/** @addtogroup EEPROM_Private_Functions
  * @{
  */

/**
  * @brief  Erase a page in polling mode
  * @param  Page Page number
  * @param  NbPages Number of pages to erase
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE error code: if an error occurs
  */
EE_Status PageErase(uint32_t Page, uint16_t NbPages)
{
  FLASH_EraseInitTypeDef s_eraseinit;
  uint32_t bank = FLASH_BANK_1, page_error = 0U;

#if defined(FLASH_OPTR_BFB2)
  bank = GetBankNumber(PAGE_ADDRESS(Page));
#endif

  s_eraseinit.TypeErase   = FLASH_TYPEERASE_PAGES;
  s_eraseinit.NbPages     = NbPages;
  s_eraseinit.Page        = Page;
  s_eraseinit.Banks       = bank;

  /* Erase the Page: Set Page status to ERASED status */
  if (HAL_FLASHEx_Erase(&s_eraseinit, &page_error) != HAL_OK)
  {
    return EE_ERASE_ERROR;
  }
  return EE_OK;
}

/**
  * @brief  Erase a page with interrupt enabled
  * @param  Page Page number
  * @param  NbPages Number of pages to erase
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE error code: if an error occurs
  */
EE_Status PageErase_IT(uint32_t Page, uint16_t NbPages)
{
  FLASH_EraseInitTypeDef s_eraseinit;
  uint32_t bank = FLASH_BANK_1;

#if defined(FLASH_OPTR_BFB2)
  bank = GetBankNumber(PAGE_ADDRESS(Page));
#endif

  s_eraseinit.TypeErase   = FLASH_TYPEERASE_PAGES;
  s_eraseinit.NbPages     = NbPages;
  s_eraseinit.Page        = Page;
  s_eraseinit.Banks       = bank;

  /* Erase the Page: Set Page status to ERASED status */
  if (HAL_FLASHEx_Erase_IT(&s_eraseinit) != HAL_OK)
  {
    return EE_ERASE_ERROR;
  }
  return EE_OK;
}

/**
  * @brief  Gets the bank of a given address
  * @param  Address Address of the FLASH Memory
  * @retval Bank_Number The bank of a given address
  */
static uint32_t GetBankNumber(uint32_t Address)
{
  uint32_t bank = 0U;
   
   if ( FLASH_BANK1_END >= Address )
   {
      bank = FLASH_BANK_1;
   }
   else 
   {
     if (READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE) == 0U)
     {
       /* No Bank swap */
       if (Address < (FLASH_BASE + FLASH_BANK_SIZE))
       {
         bank = FLASH_BANK_1;
       }
       else
       {
         bank = FLASH_BANK_2;
       }
     }
     else
     {
       /* Bank swap */
       if (Address < (FLASH_BASE + FLASH_BANK_SIZE))
       {
         bank = FLASH_BANK_2;
       }
       else
       {
         bank = FLASH_BANK_1;
       }
     }
  }

  return bank;
}

/**
  * @brief  Delete corrupted Flash address, can be called from NMI. No Timeout.
  * @param  Address Address of the FLASH Memory to delete
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE error code: if an error occurs
  */
EE_Status DeleteCorruptedFlashAddress(uint32_t Address)
{
  uint32_t dcachetoreactivate = 0U;
  EE_Status status = EE_OK;

  /* Deactivate the data cache if they are activated to avoid data misbehavior */
  if(READ_BIT(FLASH->ACR, FLASH_ACR_DCEN) != RESET)
  {
    /* Disable data cache  */
    __HAL_FLASH_DATA_CACHE_DISABLE();
    dcachetoreactivate = 1U;
  }

  /* Set FLASH Programmation bit */
  SET_BIT(FLASH->CR, FLASH_CR_PG);

  /* Program double word of value 0 */
  *(__IO uint32_t*)(Address) = (uint32_t)0U;
  *(__IO uint32_t*)(Address+4U) = (uint32_t)0U;

  /* Wait programmation completion */
  while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY)) 
  {
  }

  /* Check if error occured */
  if((__HAL_FLASH_GET_FLAG(FLASH_FLAG_OPERR))  || (__HAL_FLASH_GET_FLAG(FLASH_FLAG_PROGERR)) || 
     (__HAL_FLASH_GET_FLAG(FLASH_FLAG_WRPERR)) || (__HAL_FLASH_GET_FLAG(FLASH_FLAG_PGAERR))  || 
     (__HAL_FLASH_GET_FLAG(FLASH_FLAG_SIZERR)) || (__HAL_FLASH_GET_FLAG(FLASH_FLAG_PGSERR)))
  {
    status = EE_DELETE_ERROR;
  }

  /* Check FLASH End of Operation flag  */
  if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP))
  {
    /* Clear FLASH End of Operation pending bit */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
  }

  /* Clear FLASH Programmation bit */
  CLEAR_BIT(FLASH->CR, FLASH_CR_PG);

  /* Flush the caches to be sure of the data consistency */
  if(dcachetoreactivate == 1U)
  {
    /* Reset data cache */
    __HAL_FLASH_DATA_CACHE_RESET();
    /* Enable data cache */
    __HAL_FLASH_DATA_CACHE_ENABLE();
  }

  /* Clear FLASH ECCD bit */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ECCD);

  return status;  
}

/**
  * @brief  Unlocks Flash for write access
  * @param  None
  * @retval None
  */
void FLASH_If_Init(void)
{
  /* Unlock the Program memory */
  HAL_FLASH_Unlock();

  /* Clear all FLASH flags */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGSERR | FLASH_FLAG_WRPERR | FLASH_FLAG_OPTVERR |
                         FLASH_FLAG_PGAERR | FLASH_FLAG_SIZERR | FLASH_FLAG_RDERR );
  /* Unlock the Program memory */
  HAL_FLASH_Lock();
}

/**
  * @brief  This function does an erase of all user flash area
  * @param  bank_active: start of user flash area
  * @retval FLASHIF_OK : user flash area successfully erased
  *         FLASHIF_ERASEKO : error occurred
  */
uint32_t FLASH_If_Erase(uint32_t bank_active)
{
  uint32_t bank_to_erase, error = 0;
  FLASH_EraseInitTypeDef pEraseInit;
  HAL_StatusTypeDef status = HAL_OK;

  if (bank_active == 0)
  {
    bank_to_erase = FLASH_BANK_2;
  }
  else
  {
    bank_to_erase = FLASH_BANK_1;
  }

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  pEraseInit.Banks = bank_to_erase;
  pEraseInit.NbPages = (FLASH_BANK_2-FLASH_BANK_1)/FLASH_PAGE_SIZE;
  pEraseInit.Page = 0;
  pEraseInit.TypeErase = FLASH_TYPEERASE_MASSERASE;

  status = HAL_FLASHEx_Erase(&pEraseInit, &error);

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  if (status != HAL_OK)
  {
    /* Error occurred while page erase */
    return FLASHIF_ERASEKO;
  }

  return FLASHIF_OK;
}

/**
  * @brief  This function does an CRC check of an application loaded in a memory bank.
  * @param  start: start of user flash area
  * @retval FLASHIF_OK: user flash area successfully erased
  *         other: error occurred
  */
uint32_t FLASH_If_Check(uint32_t start)
{
  /* checking if the data could be code (first word is stack location) */
  if ((*(uint32_t*)start >> 24) != 0x20 ) return FLASHIF_EMPTY;

  return FLASHIF_OK;
}

/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @param  destination: start address for target location
  * @param  p_source: pointer on buffer with data to write
  * @param  length: length of data buffer (unit is 32-bit word)
  * @retval uint32_t 0: Data successfully written to Flash memory
  *         1: Error occurred while writing data in Flash memory
  *         2: Written Data in flash memory is different from expected one
  */
uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length)
{
  uint32_t status = FLASHIF_OK;
  uint32_t i = 0;

  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  /* DataLength must be a multiple of 64 bit */
  for (i = 0; (i < length / 2) && (destination <= (USER_FLASH_END_ADDRESS - 8)); i++)
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */
    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, destination, *((uint64_t *)(p_source + 2*i))) == HAL_OK)
    {
      /* Check the written value */
      if (*(uint64_t*)destination != *(uint64_t *)(p_source + 2*i))
      {
        /* Flash content doesn't match SRAM content */
        status = FLASHIF_WRITINGCTRL_ERROR;
        break;
      }
      /* Increment FLASH destination address */
      destination += 8;
    }
    else
    {
      /* Error occurred while writing data in Flash memory */
      status = FLASHIF_WRITING_ERROR;
      break;
    }
  }

  /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
  HAL_FLASH_Lock();

  return status;
}

/**
  * @brief  Configure the write protection status of user flash area.
  * @retval uint32_t FLASHIF_OK if change is applied.
  */
uint32_t FLASH_If_WriteProtectionClear( void )
{
  FLASH_OBProgramInitTypeDef OptionsBytesStruct1;
  HAL_StatusTypeDef retr;

  /* Unlock the Flash to enable the flash control register access *************/
  retr = HAL_FLASH_Unlock();

  /* Unlock the Options Bytes *************************************************/
  retr |= HAL_FLASH_OB_Unlock();

  OptionsBytesStruct1.RDPLevel = OB_RDP_LEVEL_0;
  OptionsBytesStruct1.OptionType = OPTIONBYTE_WRP;
  OptionsBytesStruct1.WRPArea = OB_WRPAREA_BANK2_AREAA;
  OptionsBytesStruct1.WRPEndOffset = 0x00;
  OptionsBytesStruct1.WRPStartOffset = 0xFF;
  retr |= HAL_FLASHEx_OBProgram(&OptionsBytesStruct1);

  OptionsBytesStruct1.WRPArea = OB_WRPAREA_BANK2_AREAB;
  retr |= HAL_FLASHEx_OBProgram(&OptionsBytesStruct1);

  return (retr == HAL_OK ? FLASHIF_OK : FLASHIF_PROTECTION_ERRROR);
}

/**
  * @brief  Modify the BFB2 status of user flash area.
  * @param  none
  * @retval HAL_StatusTypeDef HAL_OK if change is applied.
  */
uint32_t FLASH_If_GetActiveBank(void)
{
  FLASH_OBProgramInitTypeDef ob_config;
  uint32_t active_page;

  /* Get the current configuration */
  HAL_FLASHEx_OBGetConfig( &ob_config );

  if ((ob_config.USERConfig) & (OB_BFB2_ENABLE)) /* BANK1 active for boot */
  {
    active_page = 2u;
  }
  else
  {
    active_page = 1u;
  }
  
  return active_page;
}

/**
  * @brief  Modify the BFB2 status of user flash area.
  * @param  none
  * @retval HAL_StatusTypeDef HAL_OK if change is applied.
  */
HAL_StatusTypeDef FLASH_If_BankSwitch(void)
{
  FLASH_OBProgramInitTypeDef ob_config;
  HAL_StatusTypeDef result;

  HAL_FLASH_Lock();

  /* Clear OPTVERR bit set on virgin samples */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

  /* Get the current configuration */
  HAL_FLASHEx_OBGetConfig( &ob_config );

  ob_config.OptionType = OPTIONBYTE_USER;
  ob_config.USERType = OB_USER_BFB2;
  if ((ob_config.USERConfig) & (OB_BFB2_ENABLE)) /* BANK1 active for boot */
  {
    ob_config.USERConfig = OB_BFB2_DISABLE;
  }
  else
  {
    ob_config.USERConfig = OB_BFB2_ENABLE;
  }

  /* Initiating the modifications */
  result = HAL_FLASH_Unlock();

  /* program if unlock is successful */
  if ( result == HAL_OK )
  {
    result = HAL_FLASH_OB_Unlock();

    /* program if unlock is successful*/
    if ((READ_BIT(FLASH->CR, FLASH_CR_OPTLOCK) == RESET))
    {
      result = HAL_FLASHEx_OBProgram(&ob_config);
    }
    if (result == HAL_OK)
    {
      HAL_FLASH_OB_Launch();
    }
  }
  return result;
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
