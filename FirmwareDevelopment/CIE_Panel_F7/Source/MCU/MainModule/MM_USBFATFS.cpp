/***************************************************************************
* File name: USBFATFS.cpp
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
* USBFATFS control module
*
**************************************************************************/

/* Defines
**************************************************************************/
#define FILE_BLOCK_SIZE 256


/* System Include Files
**************************************************************************/
#ifdef STM32F767xx
#include "stm32f7xx_hal.h"
#else
#include "stm32f4xx_hal.h"
#endif


#include <assert.h>


/* User Include Files
**************************************************************************/
#include "MM_Application.h"
#include "MM_Command2.h"
#include "MM_USBFATFS.h"
#include "MM_fault.h"
#include "MM_log.h"
#include "MM_QuadSPI.h"


/* Globals
**************************************************************************/         
extern USBH_HandleTypeDef hUsbHostHS; /* USB Host handle */


/*************************************************************************/
/**  \fn      USBFATFS::USBFATFS( )
***  \brief   Constructor for class
**************************************************************************/

USBFATFS::USBFATFS( ) : Module( "USBFATFS", 0, EVENT_USBH_STORAGE_READY | EVENT_USBH_STORAGE_DISCONNECT )
{
   /* Init USBFATFS control */	
	FATFS_LinkDriver(&USBH_Driver, USBDISKPath );
	
	writing = false;
	reading = false;
	
	readpipe  = NULL;
	writepipe = NULL;
	
	mounted = false;
} 



/*************************************************************************/
/**  \fn      int USBFATFS::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int USBFATFS::Receive( Event e )
{
	switch ( e )
	{
		case EVENT_USBH_STORAGE_READY :
		{
		   /* Register the file system object to the FatFs module */
			if ( f_mount( &USBDISKFatFs, (TCHAR const*)USBDISKPath, 0 ) != FR_OK )
			{
				// not a fault really
			}
			else
			{
				mounted = true;
			}
			break;
		}
		
		case EVENT_USBH_STORAGE_DISCONNECT :
		{
			// dismount
			f_mount( NULL, (TCHAR const*) USBDISKPath, 0 );	
		}
		
		default:	
			return 0;
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int USBFATFS::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int USBFATFS::Receive( Message* )
{
	return 0;
}
	

/*************************************************************************/
/**  \fn      int USBFATFS::Receive( Command* )
***  \brief   Class function
**************************************************************************/

int USBFATFS::Receive( Command* cmd )
{
	switch ( cmd->type )
	{
		case CMD_GET_ITEMS_IN_FOLDER :
		{
			cmd->int1 = 0;
			
			if ( mounted )
			{
				FRESULT res;
				DIR dir;
				UINT i;
				static FILINFO fno;
			 

				res = f_opendir( &dir, (char*) cmd->int0 );                       /* Open the directory */
				
				if (res == FR_OK)
				{
					for ( i = 0;; i++ )
					{
						res = f_readdir(&dir, &fno);                   /* Read a directory item */
						
						if ( res != FR_OK || fno.fname[0] == 0 ) 
						{
							break;  /* Break on error or end of dir */
						}			
					}
					cmd->int1 = i;		
					f_closedir( &dir );
				}
			}
		}			
				
		case CMD_GET_FILE_AT_INDEX :
		{
			cmd->int2 = (int)"";
			cmd->int3 = -1;			
			
			if ( mounted )
			{
				FRESULT res;
				DIR dir;
				UINT i;
				static FILINFO fno;
				 

				res = f_opendir( &dir, (char*) cmd->int0 );                       /* Open the directory */
				
				if (res == FR_OK)
				{
					for ( i = 0;; i++ )
					{
						res = f_readdir(&dir, &fno);                   /* Read a directory item */
						
						if ( res != FR_OK || fno.fname[0] == 0 ) 
						{
							break;  /* Break on error or end of dir */
						}
						
						// if correct index
						if ( i == cmd->int1 )
						{
							cmd->int2 = (int) fno.fname;
							
							if ( fno.fattrib & AM_DIR )
							{                    /* It is a directory */
								 cmd->int3 = 2;
							}
							else
							{                                       /* It is a file. */
								if ( !strcmp( fno.fname + (strlen(fno.fname) - 3 ), "csn" ) )
								{
									cmd->int3 = 1;
								}
								else cmd->int3 = 0;							
							}
							break;
						}
					}
					f_closedir( &dir );
				}
			}
		}
		return CMD_OK;
	}		
		 		
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int USBFATFS::OpenNewFile( char* name )
***  \brief   Class function
**************************************************************************/

int USBFATFS::OpenNewFile( char* name )
{
	if ( mounted )
	{
		app.DebOut( "Writing %s\n", name );
		
		if ( f_open( &writefile, name, FA_CREATE_NEW | FA_CREATE_ALWAYS | FA_WRITE ) == FR_OK ) 
		{
			writing = true;
			return true;
		}
	}
	return false;
}


/*************************************************************************/
/**  \fn      unsigned int Read( char* data, unsigned int len );
***  \brief   Class function
**************************************************************************/

unsigned int USBFATFS::Read( char* data, unsigned int len )
{
	if ( mounted )
	{
		UINT bytesread;
		
		if ( f_read( &readfile, (void*) data, len, &bytesread ) == FR_OK )
		{
			return bytesread;
		}
	}
	return 0;
}

/*************************************************************************/
/**  \fn      unsigned int Write( char* data, unsigned int len );
***  \brief   Class function
**************************************************************************/
				
unsigned int USBFATFS::Write( char* data, unsigned int len )
{
	if ( mounted )
	{
		UINT byteswritten;
		
		if ( f_write( &writefile, (const void*) data, (UINT) len, &byteswritten )
					== FR_OK )
		{
			return byteswritten;
		}
	}
	return 0;
}


/*************************************************************************/
/**  \fn      int USBFATFS::OpenExistingFile( char* name )
***  \brief   Class function
**************************************************************************/

int USBFATFS::OpenExistingFile( char* name )
{
	if ( mounted )
	{
		if ( f_open( &readfile, name, FA_READ ) == FR_OK ) 
		{
			reading = true;
			return true;
		}
	}
	return false;
}


/*************************************************************************/
/**  \fn      int USBFATFS::StartReception( Pipe* p )
***  \brief   Class function
**************************************************************************/

int USBFATFS::StartReception( Pipe* p )
{
	if ( mounted && writing )
	{
		writepipe = p;
		
		wizard = (EmbeddedWizard*) app.FindModule( "EmbeddedWizard" );
	
		assert( wizard != NULL );

		return true;
	}
	return false;
}


/*************************************************************************/
/**  \fn      int USBFATFS::StartTransmission( Pipe* p )
***  \brief   Class function
**************************************************************************/
 
int USBFATFS::StartTransmission( Pipe* p )
{
	if ( mounted && reading )
	{
 		readpipe = p;
		priority = 10;
 		
		return true;
	}
	else
	{
		return false;
	}
} 


/*************************************************************************/
/**  \fn      int USBFATFS::EndReception( Pipe* p, int sucess )
***  \brief   Class function
**************************************************************************/

void USBFATFS::EndReception( Pipe* p, int success )
{
	if ( mounted && writing )
	{
		f_close( &writefile );
		writing = false;
	}
}


/*************************************************************************/
/**  \fn      int USBFATFS::EndTransmission( Pipe* p, int sucess )
***  \brief   Class function
**************************************************************************/
 
void USBFATFS::EndTransmission( Pipe* p, int sucess )
{
	if ( mounted && reading )
	{
		f_close( &readfile );
		reading = false;
	}
}
 

/*************************************************************************/
/**  \fn      void USBFATFS::Receive( Pipe* p, char* data, int len )
***  \brief   Virtual class function
**************************************************************************/

void USBFATFS::Receive( Pipe* p, char* data )
{
	UINT byteswritten;
	
	UINT len = p->Remaining( ) > QSPI_PAGE_SIZE ? QSPI_PAGE_SIZE : p->Remaining( );
	
	assert( p != NULL );
	assert( data != NULL );
	
	if ( writing ) while ( len > 0 )
	{
		f_write( &writefile, (const void*) data, (UINT) len, &byteswritten );
		
		len -= byteswritten;
		data += byteswritten;
	}	
}


/*************************************************************************/
/**  \fn      void USBFATFS::Poll( )
***  \brief   Class function
**************************************************************************/

void USBFATFS::Poll( )
{
	if ( reading )
	{
		if ( f_eof( &readfile ) )
		{
			if ( !readpipe->finished )
			{
				readpipe->End( );
			}
			reading = false;
			
			return;
		}
			
		UINT bytesread;
		char data[ QSPI_PAGE_SIZE ];
		UINT len = QSPI_PAGE_SIZE;
			
		if ( f_read( &readfile, (void*) data, len, &bytesread ) != FR_OK )
		{
			readpipe->Abort( );
			
			return;
		}
			
		if ( bytesread > 0 )
		{
			readpipe->Send( data );
		}
	}
}
