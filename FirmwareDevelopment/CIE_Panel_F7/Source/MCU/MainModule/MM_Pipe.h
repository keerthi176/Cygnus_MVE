/***************************************************************************
* File name: MM_Pipe.h
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
* Piping stream manager 
*
**************************************************************************/

#ifndef __PIPE_H__
#define __PIPE_H__


/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "MM_Module.h"

class EmbeddedWizard;


typedef enum
{
	Progress_TriggerEW,
	Progress_None,
	Progress_RedOverlay,
	Progress_GreenOverlay,
} PipeProgress;


class Pipe;

class PipeFrom
{
	public:
		
	virtual int StartTransmission( Pipe* pipe ) = 0;

	virtual void EndTransmission( Pipe* pipe, int success ) = 0;
};


class PipeTo
{
	public:
		
	virtual int StartReception( Pipe* pipe ) = 0;
	
	virtual void Receive( Pipe* pipe, char* data ) = 0;
	
	virtual void EndReception( Pipe* pipe, int success ) = 0;
};


class Pipe
{
	public:

	Pipe( PipeFrom* src, PipeTo* dst, int size, bool create_new_crc = false );
	Pipe( const char* src, const char* dest, int size, bool create_new_crc = false );
	
	int   Start( );
	void  End( );
	void  Abort( );
	
	int 	Send( char* buff );
	
	int 	Progress( );
	int 	Remaining( );
	int 	GetCRC( );

	int 	totalsize;
	int 	sent;	
	int	finished;	
	int   success;

	PipeFrom* 			src;
	PipeTo* 				dst;
	
	private:

	PipeProgress		feedback;	
	int					juststarted;
	unsigned int		lastword;
	EmbeddedWizard* 	emWiz;
	int					new_crc;
};

	

#endif
