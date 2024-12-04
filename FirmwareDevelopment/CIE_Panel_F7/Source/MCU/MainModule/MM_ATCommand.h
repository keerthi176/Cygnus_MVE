
/***************************************************************************
* File name: MM_ATCommand.h
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
* Alarm list management
*
**************************************************************************/
#ifndef _MM_AT_COMMANDS_H_
#define _MM_AT_COMMANDS_H_

/* System Include Files
**************************************************************************/



/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_EmbeddedWizard.h"
#include "MM_NCU.h"


/* Defines
**************************************************************************/
#define RX_BUFF_LEN ( 128 )

struct ATCommand
{
	const char*		atcmd;
	CommandType		cmd;
	int 				reply_args;
};


class Settings;
class ImportXML;
class ExportXML;
class EEPROM;
class QuadSPI;

class ATCommands : public Module 
{
	public:
	
	ATCommands( );
	
	virtual int Init( );
		
	virtual int Receive( Command* cmd );
	virtual int Receive( Event event );
	virtual int Receive( Message* );
	
   int DecodeCommand( char* rxbuffer, char* reply );
	void SetError( int err, int line );
 	
	private:
		
	int 					ncu_squirt;
	char					ncu_cmd[ 24 ];
		
	int					lastError;
	int					lastLine;
	
	Settings* 			settings;
	ImportXML* 			ixml;
	ExportXML* 			oxml;
	EmbeddedWizard* 	ew;
	EEPROM*        	eeprom;
	NCU*					ncu;
	GSM*					gsm;
	QuadSPI*				qspi;
};
	

#endif

