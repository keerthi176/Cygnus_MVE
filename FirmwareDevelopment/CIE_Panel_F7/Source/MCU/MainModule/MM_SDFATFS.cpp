/***************************************************************************
* File name: SDFATFS.cpp
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
* SDFATFS control module
*
**************************************************************************/

/* Defines
**************************************************************************/



/* System Include Files
**************************************************************************/
#ifdef STM32F767xx
#include "stm32f7xx_hal.h"
#else
#include "stm32f4xx_hal.h"
#endif
#include "sd_diskio.h"
#include "ff.h"
#include "fatfs.h"
#include <assert.h>


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_SDFATFS.h"
#include "MM_fault.h"
 
 


/* Globals
**************************************************************************/         
extern SD_HandleTypeDef hsd1;

/*************************************************************************/
/**  \fn      SDFATFS::SDFATFS( )
***  \brief   Constructor for class
**************************************************************************/

SDFATFS::SDFATFS( ) : Module( "SDFATFS", 0, EVENT_RESET ), mounted( false ), open ( false )
{	
	if ( GPIOManager::Get( GPIO_SDCardDetect ) == IO_PIN_RESET )
	{		
		if ( f_mount( &SDFatFS, (TCHAR const*)SDPath, 0 ) != FR_OK )
		{
			char wbuff[ 2048 ];
			if ( f_mkfs( (TCHAR const*)SDPath, FM_FAT32, 512, wbuff, 2048 ) == FR_OK )
			{		
				if ( f_mount( &SDFatFS, (TCHAR const*)SDPath, 0 ) != FR_OK )
				{				
					Fault::AddFault( FAULT_UNABLE_TO_MOUNT_SDCARD );
				}
			}
			else
			{
				Fault::AddFault( FAULT_UNABLE_TO_FORMAT_SD_CARD );
			}		
		}	
		else
		{
			mounted = true;
		}
	}
	else
	{
		Fault::AddFault( FAULT_SD_CARD_NOT_DETECTED );
	}		
}

void SDFATFS::SDFATFS_Init()
{
	HAL_SD_Init(&hsd1);

	if ( f_mount( 0," ", 0 ) == FR_OK )
	{
		if ( f_mount( &SDFatFS, (TCHAR const*)SDPath, 1 ) != FR_OK )
		{
			Fault::AddFault( FAULT_UNABLE_TO_MOUNT_SDCARD );
		}
	}
}


/*************************************************************************/
/**  \fn      int SDFATFS::Init( )
***  \brief   Class function
**************************************************************************/

int SDFATFS::Init( )
{	

	return true;
}


/*************************************************************************/
/**  \fn      int SDFATFS::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int SDFATFS::Receive( Event e )
{
	// if e = EVENT_RESET
	
	if ( !mounted )
	{
		Init( );
	}
	
	return 0;
}


/*************************************************************************/
/**  \fn      int SDFATFS::Sync( )
***  \brief   Class function
**************************************************************************/

void SDFATFS::Sync( )
{
	f_sync( &SDFile );
}


/*************************************************************************/
/**  \fn      int SDFATFS::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int SDFATFS::Receive( Message* )
{
	return 0;
}
	

/*************************************************************************/
/**  \fn      int SDFATFS::Receive( Command* )
***  \brief   Class function
**************************************************************************/

int SDFATFS::Receive( Command* cmd )
{
	switch ( cmd->type )
	{
	} 		
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int SDFATFS::OpenNewFile( char* name )
***  \brief   Class function
**************************************************************************/

int SDFATFS::OpenFile( const char* name, BYTE flags )
{
	if ( mounted )
	{
	//	app.DebOut( "Writing %s\n", name );
		
		if ( f_open( &SDFile, name, flags ) == FR_OK ) 
		{
			open = true;
			return true;
		}
	}
	return false;
}


/*************************************************************************/
/**  \fn      int SDFATFS::CloseFile( )
***  \brief   Class function
**************************************************************************/

int SDFATFS::CloseFile(  )
{
	if ( mounted )
	{
		app.DebOut( "closing file\n" );
		
		if ( open )
		if ( f_close( &SDFile ) == FR_OK ) 
		{
			open = false;
			return true;
		}
	}
	return false;
}



/*************************************************************************/
/**  \fn      int SDFATFS::GetSize( );
***  \brief   Class function
**************************************************************************/

int SDFATFS::GetSize(  )
{
	if ( mounted )
	{
		return f_size( &SDFile );
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int SDFATFS::GetPos( );
***  \brief   Class function
**************************************************************************/

int SDFATFS::GetPos(  )
{
	if ( mounted )
	{
		return f_tell( &SDFile );
	}
	return 0;
}


/*************************************************************************/
/**  \fn      void SDFATFS::Seek( );
***  \brief   Class function
**************************************************************************/

void SDFATFS::Seek( uint64_t pos )
{
	if ( mounted )
	{
		f_lseek( &SDFile, pos );
		app.DebOut( "SD seek %d", pos );
	}
}


/*************************************************************************/
/**  \fn      unsigned int Read( char* data, unsigned int len );
***  \brief   Class function
**************************************************************************/

unsigned int SDFATFS::Read( char* data, unsigned int len )
{
	if ( mounted && open )
	{
		UINT bytesread;
		int trys = 0;
		
		while ( trys++ < 3 ) 
		{		
			FRESULT r = f_read( &SDFile, (void*) data, len, &bytesread );
			
			if ( r == FR_OK )
			{
				return bytesread;
			}
			app.DebOut( "SD read fail( %d ) er:%d\n", len, r );
		}
	}
	return 0;
}

/*************************************************************************/
/**  \fn      unsigned int Write( char* data, unsigned int len );
***  \brief   Class function
**************************************************************************/
				
unsigned int SDFATFS::Write( char* data, unsigned int len )
{
	if ( mounted && open )
	{
		UINT byteswritten;
		
		if ( f_write( &SDFile, (const void*) data, (UINT) len, &byteswritten )
					== FR_OK )
		{
			return byteswritten;
			
		}
		app.DebOut( "SD write fail( %d )", len );
	}
	return 0;
}

 
extern "C" void BSP_SD_AbortCallback( )
{
	app.DebOut( "SD Abort\n" );
	HAL_SD_DeInit( &hsd1 );
	HAL_SD_Init( &hsd1 ); 
}


extern "C" void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
	app.DebOut( "SD Error\n" );	
	HAL_SD_DeInit( &hsd1 );
	HAL_SD_Init( &hsd1 ); 
}



/*************************************************************************/
/**  \fn      WCHAR ff_convert (WCHAR wch, UINT dir)
***  \brief   Class function
**************************************************************************/

extern "C" WCHAR ff_convert( WCHAR wch, UINT dir ) 
{ 
	if ( wch < 0x80 )
	{ 
		/* ASCII Char */ 
		return wch; 
	}  

	/* I don't support unicode it is too big! */ 
	return 0; 
}  


/*************************************************************************/
/**  \fn      WCHAR ff_convert( WCHAR wch, UINT dir )
***  \brief   Class function
**************************************************************************/

extern "C" WCHAR ff_wtoupper( WCHAR wch ) 
{ 
	if ( wch < 0x80 )
	{      
		/* ASCII Char */ 
		if ( wch >= 'a' && wch <= 'z' )
		{ 
			wch &= ~0x20; 
		} 
		return wch; 
	}  

	/* I don't support unicode it is too big! */ 
	return 0; 
} 

 
