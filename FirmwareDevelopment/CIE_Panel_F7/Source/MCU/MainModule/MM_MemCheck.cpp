/***************************************************************************
* File name: MM_MemCheck.cpp
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
* Memory check module.
*
**************************************************************************/


/* System Include Files
**************************************************************************/
#ifdef STM32F767xx
#include "stm32f7xx_hal_crc.h"
#else
#include "stm32f4xx_hal_crc.h"
#endif


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_MemCheck.h"
#include "MM_Module.h"
#include "MM_Message.h"
#include "MM_log.h"
#include "MM_fault.h"
#include "MM_Settings.h"



/* Defines
**************************************************************************/

#define CHECKSUM_VALID 0x1234



/* Globals
**************************************************************************/


unsigned int* 	MemCheck::mem_pos = NULL;
unsigned int* 	MemCheck::mem_start = NULL;
unsigned int* 	MemCheck::mem_end = NULL;

int  MemCheck::checksum_valid = false;
unsigned int MemCheck::checksum = 0;


/*************************************************************************/
/**  \fn      MemCheck::Check( )
***  \brief   Constructor for class
**************************************************************************/

MemCheck::MemCheck( ) : Module( "MemCheck", 50, EVENT_RESET )
{	

}


/*************************************************************************/
/**  \fn      MemCheck::Check( )
***  \brief   Constructor for class
**************************************************************************/

void MemCheck::CheckEntireSite( bool report )
{
	if ( app.panel != NULL )
	{
		uint32_t stored_sum;
		
		uint32_t sum = HAL_CRC_Calculate( app.crc, (uint32_t*) app.panel, sizeof(*app.panel) / sizeof(int) );
		
		if ( GetChecksum( stored_sum ) )
		{	
			if ( sum != stored_sum )
			{
				if ( report ) Fault::AddFault( FAULT_SITE_DATA_CORRUPTION );
				// what if stored was wrong - gonna have to set to new otherwise we're stuck..
				SetChecksum( sum );
			}
			else
			{
				Fault::RemoveFault( FAULT_SITE_DATA_CORRUPTION );
			}
		}		
		else SetChecksum( sum );	
	}
}


/*************************************************************************/
/**  \fn      MemCheck::Check( )
***  \brief   Constructor for class
**************************************************************************/

void MemCheck::Check( unsigned int block_size )
{	
	if ( app.site != NULL && app.panel != NULL )
	{
		if ( mem_pos != NULL )
		{
			CRC_TypeDef* crc = app.crc->Instance;
			
			unsigned int* block_end = mem_pos + block_size;
			
			// clip if need
			if ( block_end > mem_end ) block_end = mem_end; 
			
			while ( mem_pos <= block_end - 8 )
			{
				/* 32 byte block */
				crc->DR = *mem_pos++;
				crc->DR = *mem_pos++;
				crc->DR = *mem_pos++;
				crc->DR = *mem_pos++;
				crc->DR = *mem_pos++;
				crc->DR = *mem_pos++;
				crc->DR = *mem_pos++;
				crc->DR = *mem_pos++;
			}
		 
			// do last bit
			while ( mem_pos < block_end )
			{
				crc->DR = *mem_pos++;
			}
			
			// are we finished?
			if ( block_end == mem_end ) 
			{	
				// if current checksum not valid
				if ( !checksum_valid )
				{
					// Just update
					SetChecksum( crc->DR );
				}
				else
				{
					// if different result
					if ( crc->DR != checksum )
					{
						// Lets double check..
						uint32_t failsum = crc->DR;
						uint32_t sum;
						uint32_t count = 0;
						
						do
						{
							sum = HAL_CRC_Calculate( app.crc, (uint32_t*) app.panel, sizeof(*app.panel) / sizeof(int) );
							
							if ( sum == failsum )
							{
								break;
							}	
							if ( ++count > 4 )
							{
								break;
							}
							
							failsum = sum;
						} while( sum != checksum );
						
						if ( sum != checksum )
						{
							app.DebOut( "Data integrity fail. %0X != %0X\n", sum, checksum );
							Fault::AddFault( FAULT_SITE_DATA_CORRUPTION );
							
							// We have to assume new value is correct unfortunately...
							
							SetChecksum( sum );
						}
						else
						{
							Fault::RemoveFault( FAULT_SITE_DATA_CORRUPTION );
							app.DebOut( "Data integrity OK.\n" );
						}
					}
					else
					{
						Fault::RemoveFault( FAULT_SITE_DATA_CORRUPTION );
						app.DebOut( "Data integrity OK.\n" );
					}
				}
				
				// And round we go again..
				Restart( false );
			}
		}
		else
		{
			Restart( false );
		}
	}
}


/*************************************************************************/
/**  \fn      MemCheck::Receive( Command* );
***  \brief   virtual class function
**************************************************************************/

int MemCheck::Receive( Command* )
{	
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      MemCheck::Receive( Event e )
***  \brief   virtual class function
**************************************************************************/

int MemCheck::Receive( Event e )
{	
	if ( e == EVENT_RESET )
	{
		CheckEntireSite( false );
		CheckEntireSite( true );
	}
			
	return 0;
}

/*************************************************************************/
/**  \fn      MemCheck::Receive( Event e )
***  \brief   virtual class function
**************************************************************************/

int MemCheck::Receive( Message* )
{	
	return 0;
}


/*************************************************************************/
/**  \fn      int MemCheck::LoadChecksum( int& sum )
***  \brief   virtual class function
**************************************************************************/

int MemCheck::GetChecksum( uint32_t& sum )
{	
	short valid;
		
	if( EEPROM::GetVar( checksumValid, valid ) )
	{
		if ( valid == CHECKSUM_VALID )
		{
			uint32_t stored_sum;
			checksum_valid = true;
			
			if ( EEPROM::GetVar( checksumValue, stored_sum ) )
			{
				sum = stored_sum;
				checksum = sum;
				return true;
			}
		}
	}
//	Fault::AddFault( FAULT_MEM_CHECH_SUM_GET );
	//Log::Msg(LOG_SYS, "01_MemCheck.cpp -- ADD");
	return false;
}


/*************************************************************************/
/**  \fn      int MemCheck::StoreChecksum( int sum )
***  \brief   virtual class function
**************************************************************************/

int MemCheck::SetChecksum( uint32_t sum )
{	
	short valid = CHECKSUM_VALID;
						
	if ( EEPROM::SetVar( checksumValid, valid ) )
	{
		if ( EEPROM::SetVar( checksumValue, sum ) )
		{
			Fault::RemoveFault( FAULT_EEPROM_COMMS );
			//Log::Msg(LOG_SYS, "02_MemCheck.cpp -- REMOVE");
			
			checksum_valid = true;
			checksum = sum;
			app.DebOut( "Checksum = %04X\n", checksum );
			return true;
		}
	}
	{
		Fault::AddFault( FAULT_MEM_CHECH_SUM_SET );
		//Log::Msg(LOG_SYS, "03_MemCheck.cpp -- ADD");
	}
	return false;
}
								

/*************************************************************************/
/**  \fn      int MemCheck::Init( )
***  \brief   class function
**************************************************************************/

int MemCheck::Init( )
{
	settings = (Settings*) 		app.FindModule( "Settings" );
 
	assert( settings != NULL );	

	last_ticks = HAL_GetTick( );
	
	return true;
}


/*************************************************************************/
/**  \fn      void MemCheck::Poll( )
***  \brief   class function
**************************************************************************/

void MemCheck::Poll( )
{
	unsigned int now = HAL_GetTick( );
	
	if ( app.panel != NULL )
	{
		unsigned int diff = 32;	// rough guess
		
		if ( now > last_ticks ) // check for overflow
		{
			diff = now - last_ticks;
		}

		unsigned int hour_in_ms 		= 60 * 60 * 1000;

		unsigned int panelmem_in_wrds = sizeof(*app.panel) / sizeof( int );
		
		unsigned int block_size = diff * panelmem_in_wrds / hour_in_ms;

	   Check( block_size );
	}

	
	last_ticks = now;
}


/*************************************************************************/
/**  \fn      void MemCheck::Retart( )
***  \brief   class function
**************************************************************************/

void MemCheck::Restart( bool site_changed )
{	
	if ( app.panel != NULL )
	{
		__HAL_CRC_DR_RESET( app.crc );
		
		mem_pos = mem_start = (unsigned int*) app.panel;
		mem_end = (unsigned int*) ( (char*)app.panel + sizeof(*app.panel) );

		if ( site_changed )
		{
			checksum_valid = false;
			
			// Invalidate checksum
			short zero = 0;
			if ( !EEPROM::SetVar( checksumValid, zero ) )
			{
				Fault::AddFault( FAULT_MEM_CHECH_SUM_SET );
				//Log::Msg(LOG_SYS, "04_MemCheck.cpp -- ADD");
			}
			else
			{
				Fault::RemoveFault( FAULT_EEPROM_COMMS );
				//Log::Msg(LOG_SYS, "05_MemCheck.cpp -- REMOVE");
			}
		}
	}
	else
	{
		mem_pos = NULL;
	}
}


