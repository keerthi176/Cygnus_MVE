/***************************************************************************
* File name: USBFATFS.h
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
* USB Host FATFS module.
*
**************************************************************************/

#ifndef _USB_FATFS_H_
#define _USB_FATFS_H_


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
#include "usbh_diskio_dma.h"
#include "MM_Pipe.h"
#include "MM_EmbeddedWizard.h"


typedef enum 
{
	USBFATFS_READING,
	USBFATFS_IDLE,
} USBFATFSState;


class USBFATFS : public Module, public PipeTo, public PipeFrom
{
	public:
		
	USBFATFS( );
	
	virtual void Receive( Pipe*, char* );
	
	virtual int Receive( Event );
	virtual int Receive( Message* );
	virtual int Receive( Command* );
	
	int OpenNewFile( char* name );
	int OpenExistingFile( char* name );
	
	unsigned int Read( char* data, unsigned int len );
	unsigned int Write( char* data, unsigned int len );
	
	virtual int StartReception( Pipe* );
	virtual void EndReception( Pipe*, int success );
	
	virtual int StartTransmission( Pipe* p );
	virtual void EndTransmission( Pipe*, int success );

	virtual void Poll( );
	
	
	private:
		
	USBFATFSState	status;
	
	FATFS 	USBDISKFatFs;           /* File system object for USB disk logical drive */
	char 		USBDISKPath[ 4];
	FIL 		writefile;
	FIL 		readfile;
	int 		mounted;
	int 		writing;
	int 		reading;
	Pipe* 	readpipe;
	Pipe* 	writepipe;
	
	EmbeddedWizard* wizard;
};


#endif

