/***************************************************************************
* File name: MM_Pipe.cpp
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
* Data piping utility
*
**************************************************************************/


/* System Include Files
**************************************************************************/
#include <string.h>
#include <stdlib.h>
#include <assert.h>

/* User Include Files
**************************************************************************/
#include "MM_Pipe.h"
#include "MM_Application.h"
#include "MM_EmbeddedWizard.h"
#include "CO_Site.h"
#include "MM_SplashLoader.h"
#include "MM_QuadSPI.h"



/*************************************************************************/
/**  \fn      Pipe::Pipe( Pipeable* src, Pipeable* dest, int size, char* buff, PipeCallback* done, Module* owner )
***  \brief   Constructor for class
**************************************************************************/

Pipe::Pipe( PipeFrom* src, PipeTo* dest, int size, bool create_new_crc ) : totalsize( size ), src( src ), dst( dest ), feedback( feedback ), new_crc( create_new_crc )
{
	assert( src != NULL ); 
	assert( dst != NULL );
	
#ifndef BD_TEST_ONLY
	emWiz = (EmbeddedWizard*) app.FindModule( "EmbeddedWizard" );
	assert( emWiz != NULL );
#endif
}


/*************************************************************************/
/**  \fn      Pipe::Pipe( const char* src, const char* dest, int size, char* buff, PipeCallback* done, Module* owner )
***  \brief   Constructor for class
**************************************************************************/

Pipe::Pipe( const char* src, const char* dest, int size, bool create_new_crc ) 
		: totalsize( size ), src( dynamic_cast<PipeFrom*>( app.FindModule( src ))), dst( dynamic_cast<PipeTo*>(app.FindModule( dest ) )), new_crc( create_new_crc )
{
	assert( src != NULL ); 
	assert( dst != NULL );
	
#ifndef BD_TEST_ONLY
	emWiz = (EmbeddedWizard*) app.FindModule( "EmbeddedWizard" );
	assert( emWiz != NULL );
#endif
}


/*************************************************************************/
/**  \fn      int Pipe::Progress( )
***  \brief   Class function
**************************************************************************/

int Pipe::Progress( )
{
	return sent * 100 / ( totalsize + !finished );
}


/*************************************************************************/
/**  \fn      int Pipe::Send( char* data, int size )
***  \brief   Class function
**************************************************************************/


int Pipe::Send( char* data )
{				
	if ( !juststarted )
	{
		// add last bit of previous packet

		HAL_CRC_Accumulate( app.crc, &lastword, 4 ); 
	}
	else	// if no size given we assume its a site file and extract the length
	{	  
		juststarted = false;
	}
			

	if ( sent < totalsize )
	{
		int size = QSPI_PAGE_SIZE < Remaining( ) ? QSPI_PAGE_SIZE : Remaining( );
			
		if ( size >= 4 )
		{
			HAL_CRC_Accumulate( app.crc, (uint32_t*) data, size - 4 ); 	 
			memcpy( &lastword, data + size - 4, 4 );  
		}

		if ( new_crc && ( Remaining( ) <= QSPI_PAGE_SIZE ) )
		{			 
			char buff[ QSPI_PAGE_SIZE ];
			
			int crc = GetCRC( );
			
			memcpy( buff, data, Remaining( ) - sizeof( int ) );
			memcpy( buff + ( Remaining( ) - sizeof( int ) ), &crc, sizeof( int ) );
						
			dst->Receive( this, buff );			
		}
		else
		{
			dst->Receive( this, data );
		}
		
		sent += size;
	}

	if ( sent >= totalsize ) 
	{
		End( );
	}
	 	 
	return 0;
}


/*************************************************************************/
/**  \fn      int Pipe::Start( )
***  \brief   Class function
**************************************************************************/

int Pipe::Start( )
{
	if ( src->StartTransmission( this ) )
	{
		if ( dst->StartReception( this ) )
		{
			sent     	= 0;
			finished 	= false;
			juststarted = true;
			
			 __HAL_CRC_DR_RESET( app.crc );
	
			return true;
		}
		else
		{
			src->EndTransmission( this, false );	// fail
		}
	}
	return false;
}


/*************************************************************************/
/**  \fn      void Pipe::Remaining( )
***  \brief   Class function
**************************************************************************/

int Pipe::Remaining( )
{
	if ( totalsize == -1 )
	{
		return 0x7FFFFFFF;
	}
	else
	{
		return totalsize - sent;
	}
}
	

/*************************************************************************/
/**  \fn      void Pipe::Abort( )
***  \brief   Class function
**************************************************************************/

void Pipe::Abort( )
{
#ifndef BD_TEST_ONLY
	Message m;
	m.type = EW_PROGRESS_FAILED_MSG;
	m.to   = emWiz;
	app.Send( &m );
#endif

	dst->EndReception( this, false );			
	src->EndTransmission( this, false );
	success = false;
	finished = true;
	
	app.DebOut( "Calced CRC = %X\n", GetCRC( ) );
	app.DebOut( "Stored CRC = %X\n", lastword );
}
	

/*************************************************************************/
/**  \fn      int Pipe::GetCRC( )
***  \brief   Class function
**************************************************************************/

int Pipe::GetCRC( )
{
	return ~app.crc->Instance->DR;
}
	
	
/*************************************************************************/
/**  \fn      void Pipe::End( )
***  \brief   Class function
**************************************************************************/

void Pipe::End( )
{
	finished = true;
	
	if ( ( GetCRC( ) == lastword ) || new_crc )
	{	
#ifndef BD_TEST_ONLY
		Message m;
		m.type = EW_PROGRESS_SUCCESS_MSG;
		m.to   = emWiz;
		app.Send( &m );
#endif
		
		dst->EndReception( this, true );			
		src->EndTransmission( this, true );
		success = true; 
	}
	else	
	{
		Abort( );
	} 				
}




		
