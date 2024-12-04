/***************************************************************************
* File name: MM_flash.c
* Project name: CIE_Panel
*
* Copyright 2018 Bull Products Ltd as an unpublished work.
* All Rights Reserved.
*
* The information contained herein is confidential property of Bull
* Products Ltd. The use, copying, transfer or disclosure of such
* information is prohibited except by express written agreement with Bull
* Products Ltd.
*
* First written on 18/05/18 by Matt Ault
*
* File History:
* Change History:
* Rev Date Description Author
* 1.0 18/05/18 Initial File
*
* Description:
* Internal flash library
*
**************************************************************************/


/* System Include Files
**************************************************************************/
#include <stdint.h>


/* User Include Files
**************************************************************************/

#include "stm32f4xx_hal_flash_ex.h"
#include "stm32f4xx_hal_flash.h"



#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */
#define ADDR_FLASH_SECTOR_12     ((uint32_t)0x08100000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_13     ((uint32_t)0x08104000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_14     ((uint32_t)0x08108000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_15     ((uint32_t)0x0810C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_16     ((uint32_t)0x08110000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_17     ((uint32_t)0x08120000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_18     ((uint32_t)0x08140000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_19     ((uint32_t)0x08160000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_20     ((uint32_t)0x08180000) /* Base @ of Sector 8, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_21     ((uint32_t)0x081A0000) /* Base @ of Sector 9, 128 Kbytes  */
#define ADDR_FLASH_SECTOR_22     ((uint32_t)0x081C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_23     ((uint32_t)0x081E0000) /* Base @ of Sector 11, 128 Kbytes */


/*************************************************************************/
/**  \fn      int Flash_WriteEnable( int sector )
***  \brief   Global helper function
**************************************************************************/

int Flash_WriteEnable( int sector )
{
	FLASH_OBProgramInitTypeDef OBInit;
	
	HAL_FLASHEx_OBGetConfig(&OBInit);
	HAL_FLASH_OB_Unlock();

	/* Allow Access to Flash control registers and user Flash */
	HAL_FLASH_Unlock();

	/* Disable FLASH_WRP_SECTORS write protection */
	OBInit.OptionType = OPTIONBYTE_WRP;
	OBInit.WRPState   = OB_WRPSTATE_DISABLE;
	OBInit.WRPSector  = sector;
	HAL_FLASHEx_OBProgram(&OBInit);

	/* Start the Option Bytes programming process */
	if (HAL_FLASH_OB_Launch() != HAL_OK)
	{
		return 0;
	}

	/* Prevent Access to option bytes sector */
	HAL_FLASH_OB_Lock();

	/* Disable the Flash option control register access (recommended to protect
	the option Bytes against possible unwanted operations) */
	HAL_FLASH_Lock();
	
	return -1;
}


/*************************************************************************/
/**  \fn      int Flash_WriteProtect( int sector )
***  \brief   Global helper function
**************************************************************************/

int Flash_WriteProtect( int sector )
{
	FLASH_OBProgramInitTypeDef OBInit;
	HAL_FLASHEx_OBGetConfig(&OBInit);
	
	/* Allow Access to option bytes sector */
	HAL_FLASH_OB_Unlock();

	/* Allow Access to Flash control registers and user Flash */
	HAL_FLASH_Unlock();

	/* Enable FLASH_WRP_SECTORS write protection */
	OBInit.OptionType = OPTIONBYTE_WRP;
	OBInit.WRPState   = OB_WRPSTATE_ENABLE;
	OBInit.WRPSector  = OB_WRP_SECTOR_4;
	HAL_FLASHEx_OBProgram( &OBInit );

	/* Start the Option Bytes programming process */
	if (HAL_FLASH_OB_Launch() != HAL_OK)
	{
	   return 0;
	}

	/* Prevent Access to option bytes sector */
	HAL_FLASH_OB_Lock();

	/* Disable the Flash option control register access (recommended to protect
	the option Bytes against possible unwanted operations) */
	HAL_FLASH_Lock();
	
	return -1;
}



/*************************************************************************/
/**  \fn      int Flash_Prepare( int addr )
***  \brief   Global helper function
**************************************************************************/

int Flash_Prepare( int addr )
{
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t error = 0;
	
	int sector = GetSector( addr );
	
	if ( Flash_WriteEnable( 1 << sector ) )
	{
		HAL_FLASH_Unlock();
 
		/* Fill EraseInit structure*/
		EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
		EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
		EraseInitStruct.Sector        = sector;
		EraseInitStruct.NbSectors     = 1;

		if ( HAL_FLASHEx_Erase( &EraseInitStruct, &error ) == HAL_OK )
		{		
			return true;
		}
	}
			
	app->ReportFault( FAULT_DATA_INTEGRITY );
			
	return false;
}	
	

/*************************************************************************/
/**  \fn      void Flash_Finish( int addr )
***  \brief   Global helper function
**************************************************************************/

void Flash_Finish( int addr )
{
	int sector = GetSector( addr );
	
	Flash_WriteProtect( 1 << sector );
}	
	

/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
/*************************************************************************/
/**  \fn      uint32_t GetSector(uint32_t Address)
***  \brief   Global helper function
**************************************************************************/

uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_SECTOR_7;
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_SECTOR_8;
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_SECTOR_9;
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_SECTOR_10;
  }
  else if((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11))
  {
    sector = FLASH_SECTOR_11;
  }
  else if((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12))
  {
    sector = FLASH_SECTOR_12;
  }
  else if((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13))
  {
    sector = FLASH_SECTOR_13;
  }
  else if((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14))
  {
    sector = FLASH_SECTOR_14;
  }
  else if((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15))
  {
    sector = FLASH_SECTOR_15;
  }
  else if((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16))
  {
    sector = FLASH_SECTOR_16;
  }
  else if((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17))
  {
    sector = FLASH_SECTOR_17;
  }
  else if((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18))
  {
    sector = FLASH_SECTOR_18;
  }
  else if((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19))
  {
    sector = FLASH_SECTOR_19;
  }
  else if((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20))
  {
    sector = FLASH_SECTOR_20;
  }
  else if((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21))
  {
    sector = FLASH_SECTOR_21;
  }
  else if((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22))
  {
    sector = FLASH_SECTOR_22;
  }
  else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23) */
  {
    sector = FLASH_SECTOR_23;
  }  
  return sector;
}

/**
  * @brief  Gets sector Size
  * @param  None
  * @retval The size of a given sector
  */
/*************************************************************************/
/**  \fn      static uint32_t GetSectorSize(uint32_t Sector)
***  \brief   Local helper function
**************************************************************************/

static uint32_t GetSectorSize(uint32_t Sector)
{
  uint32_t sectorsize = 0x00;
  if((Sector == FLASH_SECTOR_0) || (Sector == FLASH_SECTOR_1) || (Sector == FLASH_SECTOR_2) ||\
     (Sector == FLASH_SECTOR_3) || (Sector == FLASH_SECTOR_12) || (Sector == FLASH_SECTOR_13) ||\
     (Sector == FLASH_SECTOR_14) || (Sector == FLASH_SECTOR_15))
  {
    sectorsize = 16 * 1024;
  }
  else if((Sector == FLASH_SECTOR_4) || (Sector == FLASH_SECTOR_16))
  {
    sectorsize = 64 * 1024;
  }
  else
  {
    sectorsize = 128 * 1024;
  }  
  return sectorsize;
}



