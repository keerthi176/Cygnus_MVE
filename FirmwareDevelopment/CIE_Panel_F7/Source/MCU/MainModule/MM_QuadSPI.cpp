/***************************************************************************
* File name: MM_QuadSPI.cpp
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
* QuadSPI flash block file system
*
**************************************************************************/

/* System Include Files
**************************************************************************/
#include <assert.h>
#include <string.h>
#include "stm32f7xx_hal.h"

/* User Include Files
**************************************************************************/
#include "MM_QuadSPI.h"
#include "MM_issue.h"
#include "MM_MemCheck.h"
#include "MM_EmbeddedWizard.h"
#include "MM_Utilities.h"



/* Defines
**************************************************************************/
#define SUBSECTOR_SIZE 	(1024 * 4)
#define SECTOR_SIZE 		(1024 * 64)


static volatile int memReady, cmdComplete, txComplete, rxComplete;
 

int ___qspi_dummy_read;

/*************************************************************************/
/**  \fn      QuadSPI::QuadSPI( )
***  \brief   Constructor for class
**************************************************************************/

QuadSPI::QuadSPI( ) : Module( "QuadSPI", 0 )
{	
   txCommand.InstructionMode   		= QSPI_INSTRUCTION_1_LINE;
   txCommand.AddressSize       		= QSPI_ADDRESS_24_BITS;
   txCommand.AlternateByteMode 		= QSPI_ALTERNATE_BYTES_NONE;
   txCommand.DdrMode           		= QSPI_DDR_MODE_DISABLE;
   txCommand.DdrHoldHalfCycle  		= QSPI_DDR_HHC_ANALOG_DELAY;
   txCommand.SIOOMode          		= QSPI_SIOO_INST_EVERY_CMD;	
	txCommand.Instruction 		 		= QUAD_IN_FAST_PROG_CMD;
	txCommand.AddressMode 		 		= QSPI_ADDRESS_1_LINE;
	txCommand.DataMode    		 		= QSPI_DATA_4_LINES;	
	txCommand.DummyCycles			   = 0;

   eraseCommand.InstructionMode   	= QSPI_INSTRUCTION_1_LINE;
   eraseCommand.AddressSize       	= QSPI_ADDRESS_24_BITS;
   eraseCommand.AlternateByteMode 	= QSPI_ALTERNATE_BYTES_NONE;
   eraseCommand.DdrMode           	= QSPI_DDR_MODE_DISABLE;
   eraseCommand.DdrHoldHalfCycle  	= QSPI_DDR_HHC_ANALOG_DELAY;
   eraseCommand.SIOOMode          	= QSPI_SIOO_INST_EVERY_CMD;
	eraseCommand.Instruction 		 	= SECTOR_ERASE_CMD;
	eraseCommand.AddressMode 		 	= QSPI_ADDRESS_1_LINE;
	eraseCommand.DataMode    		 	= QSPI_DATA_NONE;
	eraseCommand.DummyCycles 		 	= 0;

   subEraseCommand.InstructionMode   		= QSPI_INSTRUCTION_1_LINE;
   subEraseCommand.AddressSize       		= QSPI_ADDRESS_24_BITS;
   subEraseCommand.AlternateByteMode 		= QSPI_ALTERNATE_BYTES_NONE;
   subEraseCommand.DdrMode           		= QSPI_DDR_MODE_DISABLE;
   subEraseCommand.DdrHoldHalfCycle  		= QSPI_DDR_HHC_ANALOG_DELAY;
   subEraseCommand.SIOOMode          		= QSPI_SIOO_INST_EVERY_CMD;	
	subEraseCommand.Instruction 		 		= SUBSECTOR_ERASE_CMD;
	subEraseCommand.AddressMode 		 		= QSPI_ADDRESS_1_LINE;
	subEraseCommand.DataMode    		 		= QSPI_DATA_NONE;	
	subEraseCommand.DummyCycles			   = 0;

	mapCommand.InstructionMode   		= QSPI_INSTRUCTION_1_LINE;
	mapCommand.AddressSize       		= QSPI_ADDRESS_24_BITS;
	mapCommand.AlternateByteMode 		= QSPI_ALTERNATE_BYTES_1_LINE;
	mapCommand.AlternateBytes			= 8; //4;
	mapCommand.DdrMode           		= QSPI_DDR_MODE_DISABLE;
	mapCommand.DdrHoldHalfCycle  		= QSPI_DDR_HHC_ANALOG_DELAY;
	mapCommand.SIOOMode          		= QSPI_SIOO_INST_EVERY_CMD;
	mapCommand.Instruction 				= QUAD_OUT_FAST_READ_CMD;
	mapCommand.DummyCycles 				= 0;//DUMMY_CLOCK_CYCLES_READ_QUAD;
   mapCommand.AddressMode 				= QSPI_ADDRESS_1_LINE;
   mapCommand.DataMode    				= QSPI_DATA_4_LINES;
			 
	sMemMappedCfg.TimeOutActivation  = QSPI_TIMEOUT_COUNTER_DISABLE;

	ResetMemory( app.qspi );
	HAL_QSPI_MemoryMapped( app.qspi, &mapCommand, &sMemMappedCfg );
	
	src_addr = 0;
	dst_addr = 0;
	
	qspi_addr = (char*) QSPI_BASE;
	
	___qspi_dummy_read = *(int*) QSPI_BASE;
}


/*************************************************************************/
/**  \fn      int QuadSPI::Init( )
***  \brief   Class function
**************************************************************************/

int QuadSPI::Init( )
{
	ew = (EmbeddedWizard*) app.FindModule( "EmbeddedWizard" );
	assert( ew != NULL );
	
	return true;
}


/*************************************************************************/
/**  \fn      int QuadSPI::ResetMemory( QSPI_HandleTypeDef* hqspi )
***  \brief   Class function
**************************************************************************/

HAL_StatusTypeDef QuadSPI::ResetMemory( QSPI_HandleTypeDef* hqspi )
{
	QSPI_CommandTypeDef s_command;

	/* Initialize the Mode Bit Reset command */
	s_command.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	s_command.Instruction       = RESET_ENABLE_CMD;
	s_command.AddressMode       = QSPI_ADDRESS_NONE;
	s_command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	s_command.DataMode          = QSPI_DATA_NONE;
	s_command.DummyCycles       = 0;
	s_command.DdrMode           = QSPI_DDR_MODE_DISABLE;
	s_command.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	s_command.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	/* Send the command */
	if ( HAL_QSPI_Command( hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
	{
	   return HAL_ERROR;
	}

	/* Send the SW reset command */
	s_command.Instruction       = RESET_MEMORY_CMD;
	if ( HAL_QSPI_Command( hqspi, &s_command, HAL_QPSI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK)
	{
	   return HAL_ERROR;
	}

	/* Configure automatic polling mode to wait the memory is ready */
	return AutoPollingMemReady( hqspi );
}


/*************************************************************************/
/**  \fn      bool QuadSPI::IsWriteProtected(  )
***  \brief   Class function
**************************************************************************/
 
bool QuadSPI::IsWriteProtected( )
{
	if ( app.panel != NULL )
	{
		if ( Utilities::AccessLevel > 2 )
		{
			return false;
		}
	}		
	GPIO_PinState state = HAL_GPIO_ReadPin( WR_EN__GPIO_Port, WR_EN__Pin ); 
	
	if ( state == GPIO_PIN_SET )
	{		
		app.DebOut( "Write protected!\n" );
		Send( EW_WRITE_PROTECTION_MSG, ew );
		return true;
	}	
	return false;
}	



/*************************************************************************/
/**  \fn      int QuadSPI::MoveFlash( int src, int dest, int size )
***  \brief   Class function
**************************************************************************/
 
QSPI_Result QuadSPI::MoveFlashLower( int src, int dest, int size )
{		
	char buff[ SUBSECTOR_SIZE ];
	
	// Rememeber site address
	Site*  site  = app.site;
	Panel* panel = app.panel;
	
	MemCheck::Restart( );
	
	app.site  = NULL;
	app.panel = NULL;
	
	int movement = src - dest;
	
	while ( size > 0 )
	{
		int subsector = dest & ~(SUBSECTOR_SIZE - 1);
		int offset    = dest &  (SUBSECTOR_SIZE - 1);
		
		int portion = SUBSECTOR_SIZE - offset;
		
		if ( portion > size ) 
		{
			portion = size;
		}
		
		// copy whole subsector
		memcpy( buff, (const void*) (qspi_addr + subsector ), SUBSECTOR_SIZE ); 
		
		// copy moved section
		memcpy( buff + offset, (const void*) (qspi_addr + subsector + offset + movement ), portion );
		
		// Erase 4K
		if ( !EraseSubSector( subsector ) )
		{
			app.Report( ISSUE_QUADSPI_ERASE_FAIL );
			
			app.site  = site;
			app.panel = panel;	
			
			return QSPI_RESULT_ERASE_FAIL;
		}
		
		char* p = buff;
		
		// Program buffer back again
		for( int addr = subsector; addr < subsector + SUBSECTOR_SIZE; addr += QSPI_PAGE_SIZE )
		{
			if ( !ProgramPage( addr, p, QSPI_PAGE_SIZE ) )
			{
				app.Report( ISSUE_QUADSPI_WRITE_FAIL );
				
				app.site  = site;
				app.panel = panel;	
				
				return QSPI_RESULT_WRITE_FAIL;
			}
			p += QSPI_PAGE_SIZE;
		}
		
		size -= portion;

		dest += portion;
 
	}
	
	app.site  = site;
	app.panel = panel;	
			
	return QSPI_RESULT_OK;
}
	

/*************************************************************************/
/**  \fn      int QuadSPI::WriteToFlash( char* data, int dest, int size )
***  \brief   Class function
**************************************************************************/
 
QSPI_Result QuadSPI::WriteToFlash( char* data, int dest_addr, int size )
{			
	QSPI_Result	result = QSPI_RESULT_OK;
 	
	if ( IsWriteProtected( ) )
	{
		return QSPI_RESULT_WRITE_PROTECTED;
	}			
	
	// If same dont bother writing...
	if ( !memcmp( data, (const void*)( qspi_addr + dest_addr ), size ) ) return result;

	// Rememeber site address
	Site*  site  = app.site;
	Panel* panel = app.panel;
	
	MemCheck::Restart( );
	
	app.site  = NULL;
	app.panel = NULL;
	
	while ( size > 0 )
	{
		int subsector = dest_addr & ~(SUBSECTOR_SIZE - 1);
		int offset    = dest_addr &  (SUBSECTOR_SIZE - 1);
		
		int portion = SUBSECTOR_SIZE - offset;
		
		if ( portion > size ) portion = size;
		
		char buff[ SUBSECTOR_SIZE ];	// = 4K

		int trys = 3;
		
		do
		{
			if ( trys-- == 0 )
			{
				app.Report( ISSUE_QUADSPI_READ_FAIL );

				app.site  = site;
				app.panel = panel;	
				
				return QSPI_RESULT_READ_FAIL;	;
			}
			memcpy( buff, (const void*) (qspi_addr + subsector), SUBSECTOR_SIZE );
			
		}
		while ( memcmp( buff, (const void*) (qspi_addr + subsector), SUBSECTOR_SIZE ) );
		
		
		// Copy the data in
		memcpy( buff + offset, data, portion );
		
		// Erase 4K
		if ( !EraseSubSector( subsector ) )
		{
			app.Report( ISSUE_QUADSPI_ERASE_FAIL );
			result = QSPI_RESULT_ERASE_FAIL;
			break;
		}
		
		char* p = buff;
		
		// Program buffer back again
		for( int addr = subsector; addr < subsector + SUBSECTOR_SIZE; addr += QSPI_PAGE_SIZE )
		{
			if ( !ProgramPage( addr, p, QSPI_PAGE_SIZE ) )
			{
				app.Report( ISSUE_QUADSPI_WRITE_FAIL );
				
				app.site  = site;
				app.panel = panel;	
				
				return QSPI_RESULT_WRITE_FAIL;
			}
			p += QSPI_PAGE_SIZE;
		}
		
		if ( memcmp( buff, (const void*) (qspi_addr + subsector), SUBSECTOR_SIZE ) )
		{
			app.Report( ISSUE_QUADSPI_WRITE_FAIL );
			result = QSPI_RESULT_WRITE_FAIL;
			break;
		}
		
		size -= portion;

		dest_addr += portion;
		data      += portion;
	}
	
	app.site  = site;
	app.panel = panel;	
			
	return result;
}
 

QSPI_Result QuadSPI::WriteDontEraseToFlash( char* data, int dest_addr, int size )
{			
	QSPI_Result	result = QSPI_RESULT_OK;
 	
	if ( IsWriteProtected( ) )
	{
		return QSPI_RESULT_WRITE_PROTECTED;
	}			
	
	// Rememeber site address
	Site*  site  = app.site;
	Panel* panel = app.panel;
	
	// prevent access to site
	app.site  = NULL;
	app.panel = NULL;
	
	char* p = data;
	
	while ( size > 0 )
	{
		char buff[ QSPI_PAGE_SIZE ];
		
		int page = dest_addr & ~(QSPI_PAGE_SIZE - 1);
		int offset    = dest_addr &  (QSPI_PAGE_SIZE - 1);
	
		int portion = QSPI_PAGE_SIZE - offset;
	
		if ( size < portion ) portion = size;
	
		if ( offset == 0 && size >= QSPI_PAGE_SIZE )
		{
			if ( !ProgramPage( page, buff, QSPI_PAGE_SIZE ) )
			{
				app.Report( ISSUE_QUADSPI_WRITE_FAIL );
	 
				result = QSPI_RESULT_WRITE_FAIL;
				break;
			}
		}
		else
		{			
			int trys = 3;
			
			do
			{
				if ( trys-- == 0 )
				{
					app.Report( ISSUE_QUADSPI_READ_FAIL );

					// Restore site
					app.site  = site;
					app.panel = panel;	
					
					return QSPI_RESULT_READ_FAIL;	;
				}
				memcpy( buff, (const void*) (qspi_addr + page), QSPI_PAGE_SIZE );
				
			}
			while ( memcmp( buff, (const void*) (qspi_addr + page), QSPI_PAGE_SIZE ) != 0 );
						
			// Copy the data in
			memcpy( buff + offset, data, portion );
			 
			if ( !ProgramPage( page, buff, QSPI_PAGE_SIZE ) )
			{
				app.Report( ISSUE_QUADSPI_WRITE_FAIL );
	 
				result = QSPI_RESULT_WRITE_FAIL;
				break;
			}
			 
		}
		
		if ( memcmp( buff, (const void*) (qspi_addr + page), QSPI_PAGE_SIZE ) )
		{
			app.Report( ISSUE_QUADSPI_WRITE_FAIL );
			result = QSPI_RESULT_WRITE_FAIL;
			break;
		}
		
		size -= portion;

		dest_addr += portion;
		data      += portion;
	}
		
	app.site  = site;
	app.panel = panel;	
			
	return result;
}


extern "C" int FastCompare( char* a1, char* a2, int size );




/*************************************************************************/
/**  \fn      int QuadSPI::WaitFor( volatile int& global )
***  \brief   Class function
**************************************************************************/

int QuadSPI::WaitFor( volatile int& global )
{
	int iter = 0;
	
	while ( !global )
	{
		if ( iter++ > 0xFFFFFFF )
		{
			return false;
		}
	}
	return true;
}


/*************************************************************************/
/**  \fn      int QuadSPI::EraseSubSector( int addr )
***  \brief   Class function
**************************************************************************/

int QuadSPI::EraseSubSector( int addr ) 
{
	HAL_QSPI_DeInit( app.qspi );
	if ( HAL_QSPI_Init( app.qspi ) != HAL_OK )
	{
		return false;
	}
		
	/* Enable write operations ------------------------------------------- */
	if ( WriteEnable( app.qspi ) ) do
	{
		subEraseCommand.Address = addr;

		app.DebOut( "Erasing  @ 0x90000000+%X\n", addr );
		
		if ( HAL_QSPI_Command( app.qspi, &subEraseCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK )
		{
			break;
		}
	
		/* Configure automatic polling mode to wait for end of erase ------- */  
		if ( AutoPollingMemReady( app.qspi ) != HAL_OK )
		{
			break;
		}
		
		if ( HAL_QSPI_MemoryMapped( app.qspi, &mapCommand, &sMemMappedCfg ) != HAL_OK )
		{
			break;
		}
		return true;
	} while ( 0 );
	
	HAL_QSPI_MemoryMapped( app.qspi, &mapCommand, &sMemMappedCfg );
	return false;
}
		

/*************************************************************************/
/**  \fn      int QuadSPI::EraseSector( int addr )
***  \brief   Class function
**************************************************************************/


int QuadSPI::EraseSector( int addr ) 
{
	MemCheck::Restart( );
	
	HAL_QSPI_DeInit( app.qspi );
	if ( HAL_QSPI_Init( app.qspi ) == HAL_OK )
	{		
		/* Enable write operations ------------------------------------------- */
		if ( WriteEnable( app.qspi ) ) do
		{
			eraseCommand.Address = addr;
			cmdComplete = 0;

			app.DebOut( "Erasing  @ 0x90000000+%X\n", addr );
			if ( HAL_QSPI_Command( app.qspi, &eraseCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK )
			{
				break;;
			}
		
			/* Configure automatic polling mode to wait for end of erase ------- */  
			if ( AutoPollingMemReady( app.qspi ) != HAL_OK )
			{
				break;
			}
			
			if ( HAL_QSPI_MemoryMapped( app.qspi, &mapCommand, &sMemMappedCfg ) != HAL_OK )
			{
				break;
			}
			return true;
		} while ( 0 );
	}
	HAL_QSPI_MemoryMapped( app.qspi, &mapCommand, &sMemMappedCfg );
	return false;
}


/*************************************************************************/
/**  \fn      void QuadSPI::ProgramPage( int addr, char* buff, int size )
***  \brief   Class function
**************************************************************************/

int QuadSPI::ProgramPage( int addr, char* buff, int size )
{
	// Get out of mem mapped mode
	HAL_QSPI_DeInit( app.qspi );
	if ( HAL_QSPI_Init( app.qspi ) != HAL_OK )
	{
		app.DebOut( "fail QSPI init\n");
		return false;
	}
	
	/* Enable write operations ----------------------------------------- */
	if ( WriteEnable( app.qspi ) )
	{
		/* Writing Sequence ------------------------------------------------ */
		txCommand.NbData  = size;
		txCommand.Address = addr;
		
	//	app.DebOut( "DMA %d bytes from %X to 0x90000000+%X\n", size, buff, addr );
	//	app.DebOut( "%10s..\n", buff );
		
		txComplete = 0;
		
		if ( HAL_QSPI_Command( app.qspi, &txCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK )
		{
			app.DebOut( "fail QSPI write cmd\n" );
			 HAL_QSPI_MemoryMapped( app.qspi, &mapCommand, &sMemMappedCfg ) ;
			return false;
		}
		
		if ( HAL_QSPI_Transmit_DMA( app.qspi, (uint8_t*) buff ) != HAL_OK )
		{
			app.DebOut( "fail QSPI dma write\n" );
			 HAL_QSPI_MemoryMapped( app.qspi, &mapCommand, &sMemMappedCfg ) ;
			return false;
		}
		
		if ( WaitFor( txComplete ) )
		{
			if ( AutoPollingMemReady( app.qspi ) == HAL_OK )
			{
				return HAL_QSPI_MemoryMapped( app.qspi, &mapCommand, &sMemMappedCfg ) == HAL_OK;
			}
			else
			{
				app.DebOut( "fail QSPI mem ready\n" );
			}
		}
		else
		{
			app.DebOut( "fail QSPI waitfor\n" );
		}
	}
	else
	{
		app.DebOut( "fail QSPI write enable\n" );
	}
	 HAL_QSPI_MemoryMapped( app.qspi, &mapCommand, &sMemMappedCfg ) ;
	return false;		
}	



/**
  * @brief  Command completed callbacks.
  * @param  hqspi: QSPI handle
  * @retval None
  */
/*************************************************************************/
/**  \fn      void HAL_QSPI_CmdCpltCallback( QSPI_HandleTypeDef *hqspi )
***  \brief   Global helper function
**************************************************************************/

void HAL_QSPI_CmdCpltCallback( QSPI_HandleTypeDef *hqspi )
{
   cmdComplete++;
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  hqspi: QSPI handle
  * @retval None
  */
/*************************************************************************/
/**  \fn      void HAL_QSPI_RxCpltCallback( QSPI_HandleTypeDef *hqspi )
***  \brief   Global helper function
**************************************************************************/

void HAL_QSPI_RxCpltCallback( QSPI_HandleTypeDef *hqspi )
{
   rxComplete++;
}

/**
  * @brief  Error callback
  * @param  hqspi: QSPI handle
  * @retval None
  */
/*************************************************************************/
/**  \fn      void HAL_QSPI_ErrorCallback( QSPI_HandleTypeDef *hqspi )
***  \brief   Global helper function
**************************************************************************/

void HAL_QSPI_ErrorCallback( QSPI_HandleTypeDef *hqspi )
{
	app.Report( ISSUE_QSPI_SITE_WRITE_FAIL );
}


/**
  * @brief  Tx Transfer completed callbacks.
  * @param  hqspi: QSPI handle
  * @retval None
  */
/*************************************************************************/
/**  \fn      void HAL_QSPI_TxCpltCallback( QSPI_HandleTypeDef *hqspi )
***  \brief   Global helper function
**************************************************************************/

void HAL_QSPI_TxCpltCallback( QSPI_HandleTypeDef *hqspi )
{
   txComplete++; 
}

/**
  * @brief  Status Match callbacks
  * @param  hqspi: QSPI handle
  * @retval None
  */
/*************************************************************************/
/**  \fn      void HAL_QSPI_StatusMatchCallback( QSPI_HandleTypeDef *hqspi )
***  \brief   Global helper function
**************************************************************************/

void HAL_QSPI_StatusMatchCallback( QSPI_HandleTypeDef *hqspi )
{
   memReady++;
}


/*************************************************************************/
/**  \fn      void HAL_QSPI_FifoThresholdCallback( QSPI_HandleTypeDef *hqspi )
***  \brief   Global helper function
**************************************************************************/

void HAL_QSPI_FifoThresholdCallback( QSPI_HandleTypeDef *hqspi )
{
	app.Report( ISSUE_QSPI_THRESHOLD_REACHED );
}




/**
  * @brief  This function send a Write Enable and wait it is effective.
  * @param  hqspi: QSPI handle
  * @retval None
  */
/*************************************************************************/
/**  \fn      int QuadSPI::WriteEnable( QSPI_HandleTypeDef* QSPIHandle )
***  \brief   Class function
**************************************************************************/

int QuadSPI::WriteEnable( QSPI_HandleTypeDef* QSPIHandle )
{
	assert( QSPIHandle != NULL );
		
	HAL_QSPI_DeInit( QSPIHandle );
	
   if (HAL_QSPI_Init( QSPIHandle ) != HAL_OK)
	{
		app.Report( ISSUE_QUADSPI_INIT_FAIL );
		return false;
	}
	
	QSPI_CommandTypeDef     sCommand;
	QSPI_AutoPollingTypeDef sConfig;

	/* Enable write operations ------------------------------------------ */
	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction       = WRITE_ENABLE_CMD;
	sCommand.AddressMode       = QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode          = QSPI_DATA_NONE;
	sCommand.DummyCycles       = 0;
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

	if ( HAL_QSPI_Command( QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) != HAL_OK )
	{
		return false;
	}


	/* Configure automatic polling mode to wait for write enabling ---- */  
	sConfig.Match           = 0x02;
	sConfig.Mask            = 0x02;
	sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
	sConfig.StatusBytesSize = 1;
	sConfig.Interval        = 0x10;
	sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

	sCommand.Instruction    = READ_STATUS_REG_CMD;
	sCommand.DataMode       = QSPI_DATA_1_LINE;

	if ( HAL_QSPI_AutoPolling( QSPIHandle, &sCommand, &sConfig, HAL_QPSI_TIMEOUT_DEFAULT_VALUE ) != HAL_OK )
	{
		return false;
	}
	
	return true;
}

/**
  * @brief  This function read the SR of the memory and wait the EOP.
  * @param  hqspi: QSPI handle
  * @retval None
  */
/*************************************************************************/
/**  \fn      void QuadSPI::AutoPollingMemReady( QSPI_HandleTypeDef* hqspi )
***  \brief   Class function
**************************************************************************/

HAL_StatusTypeDef QuadSPI::AutoPollingMemReady( QSPI_HandleTypeDef* hqspi )
{
	assert( hqspi != NULL );
	
  QSPI_CommandTypeDef     sCommand;
  QSPI_AutoPollingTypeDef sConfig;

  /* Configure automatic polling mode to wait for memory ready ------ */  
  sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
  sCommand.Instruction       = READ_STATUS_REG_CMD;
  sCommand.AddressMode       = QSPI_ADDRESS_NONE;
  sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
  sCommand.DataMode          = QSPI_DATA_1_LINE;
  sCommand.DummyCycles       = 0;
  sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
  sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
  sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

  sConfig.Match           = 0x00;
  sConfig.Mask            = 0x01;
  sConfig.MatchMode       = QSPI_MATCH_MODE_AND;
  sConfig.StatusBytesSize = 1;
  sConfig.Interval        = 0x10;
  sConfig.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

  return HAL_QSPI_AutoPolling( hqspi, &sCommand, &sConfig, HAL_QPSI_TIMEOUT_DEFAULT_VALUE );
}

/**
  * @brief  This function configure the dummy cycles on memory side.
  * @param  hqspi: QSPI handle
  * @retval None
  */
/*************************************************************************/
/**  \fn      void QuadSPI::DummyCyclesCfg( QSPI_HandleTypeDef *QSPIHandle )
***  \brief   Class function
**************************************************************************/

int QuadSPI::DummyCyclesCfg( QSPI_HandleTypeDef *QSPIHandle )
{
	assert( QSPIHandle != NULL );
	
	QSPI_CommandTypeDef sCommand;
	uint8_t reg;

	/* Read Volatile Configuration register --------------------------- */
	sCommand.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
	sCommand.Instruction       = READ_VOL_CFG_REG_CMD;
	sCommand.AddressMode       = QSPI_ADDRESS_NONE;
	sCommand.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	sCommand.DataMode          = QSPI_DATA_1_LINE;
	sCommand.DummyCycles       = 0;
	sCommand.DdrMode           = QSPI_DDR_MODE_DISABLE;
	sCommand.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
	sCommand.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;
	sCommand.NbData            = 1;

	if ( HAL_QSPI_Command( QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE ) )
	{
	   return false;
	}

	if ( HAL_QSPI_Receive( QSPIHandle, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE ) )
	{
	   return false;
	}	  

	/* Enable write operations ---------------------------------------- */
	if ( WriteEnable( QSPIHandle ) )
	{
		/* Write Volatile Configuration register (with new dummy cycles) -- */  

		sCommand.Instruction = WRITE_VOL_CFG_REG_CMD;
		MODIFY_REG(reg, 0xF0, (DUMMY_CLOCK_CYCLES_READ_QUAD << POSITION_VAL(0xF0)));
			
		if ( HAL_QSPI_Command( QSPIHandle, &sCommand, HAL_QPSI_TIMEOUT_DEFAULT_VALUE) == HAL_OK )
		{
			if ( HAL_QSPI_Transmit( QSPIHandle, &reg, HAL_QPSI_TIMEOUT_DEFAULT_VALUE ) == HAL_OK )
			{
			  return true;
			}
		}
	}
	
	app.DebOut( "Write enable fail\n");
	return false;
}


int QuadSPI::Receive( Event )			{ return 0; };
int QuadSPI::Receive( Command* )    { return CMD_ERR_UNKNOWN_CMD; };
int QuadSPI::Receive( Message* )    { return 0; };
