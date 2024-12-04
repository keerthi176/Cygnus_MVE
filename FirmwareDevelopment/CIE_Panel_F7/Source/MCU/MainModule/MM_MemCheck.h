/***************************************************************************
* File name: MM_MemCheck.h
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

#include "MM_Module.h"
#include "MM_Settings.h"
 



class Message;
 

class MemCheck : public Module
{
	public:
		
	MemCheck( );
	
	virtual void Poll( );
	virtual int Init( );
	
	virtual int Receive( Event );
	virtual int Receive( Command* );
	virtual int Receive( Message* );

	static void Restart( bool site_changed = true );
	
	private:
		
	int GetChecksum( uint32_t& sum );
	int SetChecksum( uint32_t sum );
	
	
	void CheckEntireSite( bool report );
		
	void Check( unsigned int block_size );
		
	static unsigned int* 	mem_pos;
	static unsigned int* 	mem_start;
	static unsigned int* 	mem_end;
	
	static int checksum_valid;
	static unsigned int checksum;
	
	unsigned int last_ticks;
		
	Settings*  		settings;		
};


