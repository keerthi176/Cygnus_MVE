/***************************************************************************
* File name: SDFATFS.h
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
* SD Host FATFS module.
*
**************************************************************************/

#ifndef _SD_FATFS_H_
#define _SD_FATFS_H_


/* System Include Files
**************************************************************************/
#ifdef STM32F767xx
#include "stm32f7xx_hal_crc.h"
#else
#include "stm32f4xx_hal_crc.h"
#endif
#include "ff_gen_drv.h"



/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_Pipe.h"
#include "MM_EmbeddedWizard.h"
#include "MM_GPIO.h"


typedef enum 
{
	SDFATFS_READING,
	SDFATFS_IDLE,
} SDFATFSState;


class SDFATFS : public Module 
{
	public:
		
	SDFATFS( );
	
	virtual int Receive( Event );
	virtual int Receive( Message* );
	virtual int Receive( Command* );
	
	virtual int Init( );
	
	int OpenFile( const char* name, BYTE flags );

	int CloseFile( );
		
	unsigned int Read( char* data, unsigned int len );
	unsigned int Write( char* data, unsigned int len );
	
	int GetSize( );
	int GetPos( );
	void Seek( uint64_t pos );
	void Sync( );
 
	int 		mounted;
	
	static void SDFATFS_Init();

	private:
		
	SDFATFSState	status;
	GPIOManager* 	gpioman;


	int 		open;
};


#endif

