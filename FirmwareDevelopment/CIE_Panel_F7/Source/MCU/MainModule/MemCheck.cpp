/***************************************************************************
* File name: MemCheck.cpp
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
#include "stm32f4xx_hal_crc.h"

/* User Include Files
**************************************************************************/
#include "Application.h"
#include "MemCheck.h"
#include "Module.h"
#include "Message.h"
#include "priority.h"
#include "log.h"
#include "fault.h"


/* Defines
**************************************************************************/
#define DATA_CHECKSUM   0x12345678;	


MemCheck::MemCheck( Application* app ) : Module( "MemCheck", MEMCHECK_PRIORITY, app )
{
	checksum 	= DATA_CHECKSUM;	
	mem_start	= (int*) app->settings.GetROMAddress( );
	mem_end	 	= mem_start + app->settings.GetSize( ) / sizeof( int );
	mem_pos     = mem_start;
	
	crc = app->crc->Instance;
	
	__HAL_CRC_DR_RESET( app->crc );
}


void MemCheck::Poll( )
{	
	if ( mem_pos < mem_end )
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
	else
	{
		if ( crc->DR != checksum )
		{
			app->ReportFault( FAULT_DATA_INTEGRITY );
		}
		else
		{
			app->Log( LOG_DATA_INTEGRITY_CHECK_OK );
		}
		
		/* Restart */
		mem_pos = mem_start;
	}
}

		



