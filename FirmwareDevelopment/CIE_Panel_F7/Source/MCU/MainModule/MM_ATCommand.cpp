/***************************************************************************
* File name: MM_ATCommands.cpp
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
/* System Include Files
**************************************************************************/
#include <stdio.h>
#include <assert.h>

#include "version.h"

/* Defines
**************************************************************************/
#define ITEMS(x) (sizeof(x)/sizeof(*x)) 
#define MAX_PARAMS ( 8 )
#define MSG_SEND_SITE_USB_SERIAL 1
#define MSG_SEND_CMD 				2
#define MSG_SET_URL					3
#define MSG_SET_APN					4
#define MSG_SET_PWD					5
#define MSG_SET_LEDS					6


/* User Include Files
**************************************************************************/
#include "MM_ATCommand.h"
#include "MM_Application.h"
#include "CO_Site.h"
#include "MM_Settings.h"
#include "MM_ImportXML.h"
#include "MM_ExportXML.h"
#include "MM_GPIO.h"
#include "MM_USBSerial.h"
#include "MM_QuadSPI.h"


static FactorySettings fact_sett; 

extern "C" void SystemClock_Config( int, int );
extern "C" void System_Low( void );
 
extern char server_url[ 40]; 
 
/*  
static ATCommand command_list[] = 	{
												{ "SENDREQ", 	CMD_LOAD_SETTINGS_USB_SERIAL_MSG, 0 },
												{ "RECVREQ", 	CMD_SAVE_SETTINGS_USB_SERIAL, 0},
												{ "TOUCH", 		CMD_TOUCH_EMULATION, 0 },
												{ "KILLDOG",   CMD_KILL_DOG, 0 },
												{ "GETNAME",   CMD_GET_NAME, 0 },
												{ "SETNAME",   CMD_SET_NAME, 0 },
											 };
*/

/*************************************************************************/
/**  \fn      static char* GetErrMsg( int err )
***  \brief   Helper
**************************************************************************/

static const char* GetErrMsg( int err )
{
	switch( err )
	{
		case XML_OK								: return "OK"							;
		case XML_MISMATCHED_ELEMENT 		: return "Mismatched element" 	;
		case XML_UNKNOWN_ATTRIBUTE			: return "Unknown attribute"		;
		case XML_MISSING_ATTRIBUTE			: return "Missing attribute"		;
		case XML_EMPTY_ELEMENT				: return "Empty element"			;
		case XML_UNEXPECTED_DATA			: return "Unexpected data"		;
		case XML_BAD_VALUE					: return "Bad value"				;
		case XML_BAD_NUMBER_VALUE			: return "Bad number value"		;
		case XML_MISSING_DATA				: return "Missing data"			;
		case XML_UNEXPECTED_SYMBOL			: return "Unexpected symbol"		;
		case XML_INIT_FAIL				   : return "Init fail"				;
		case XML_USER_CANCEL					: return "User cancel"				;
		case XML_WRITE_PROTECTED			: return "Panel is write protected";
		case XML_EMPTY_QUOTES				: return "Empty quotes - missing data";		
		case XML_ILLEGAL_SITE			   : return "Site has too many devs/zones"; 
	}
	return "";
}

	
/*************************************************************************/
/**  \fn      ATCommands::ATCommands( )
***  \brief   Constructor for class
**************************************************************************/

ATCommands::ATCommands( ) : Module( "ATCommands", 0, EVENT_NCU_REPLIED )
{

}

	
/*************************************************************************/
/**  \fn      ATCommands::Init( )
***  \brief   Constructor for class
**************************************************************************/

int ATCommands::Init( )
{
	settings =  (Settings*) 		app.FindModule( "Settings" );
	ixml     = 	(ImportXML*) 		app.FindModule( "ImportXML" );
	ew		   =	(EmbeddedWizard*) app.FindModule( "EmbeddedWizard" );
	eeprom   = 	(EEPROM*)	 		app.FindModule( "EEPROM" );
	ncu   	= 	(NCU*)	 			app.FindModule( "NCU" );
	gsm		=	(GSM*)				app.FindModule( "GSM" );
	qspi		=	(QuadSPI*)			app.FindModule( "QuadSPI" );
	
	assert( settings != NULL );
	assert( ixml != NULL );
	assert( ew != NULL );
	assert( eeprom != NULL );	
	assert( ncu != NULL );	
	assert( qspi != NULL );	
	
	return true;
}

 
/*************************************************************************/
/**  \fn      int ATCommands::Receive( Command* )
***  \brief   Class function
**************************************************************************/

int ATCommands::Receive( Command* )
{
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int ATCommands::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int ATCommands::Receive( Event event )
{
	return 0;
}


static void NCUResponse( char* rb )
{
	USBSerial::TxString( rb );
}


/*************************************************************************/
/**  \fn      int ATCommands::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int ATCommands::Receive( Message* msg )
{
	if ( msg->type == MSG_SEND_CMD )
	{
		ncu->QueueMsg( true, NCU_CUSTOM_MSG, ncu_cmd, this, NCUResponse );
	}
	else if ( msg->type == MSG_SET_URL )
	{
		if ( !EEPROM::SetVar( AppSupp_URL, *((char*)msg->value ) ) )
		{
			if ( !EEPROM::SetVar( AppSupp_URL, *((char*)msg->value ) ) )
			{
				Fault::AddFault( FAULT_AT_APP_URL_SET );
				//Log::Msg(LOG_SYS, "01_ATCommand.cpp -- ADD");
				return false;
			}
		}
	}
	else if ( msg->type == MSG_SET_APN )
	{
		if ( !EEPROM::SetVar( AppSupp_APN, *((char*)msg->value ) ) )
		{
			if ( !EEPROM::SetVar( AppSupp_APN, *((char*)msg->value ) ) )
			{
				Fault::AddFault( FAULT_AT_APP_APN_SET );
				//Log::Msg(LOG_SYS, "02_ATCommand.cpp -- ADD");
				return false;
			}
		}
	}
	else if ( msg->type == MSG_SET_PWD )
	{
		if ( !EEPROM::SetVar( AppSupp_Password, *((char*)msg->value ) ) )
		{
			if ( !EEPROM::SetVar( AppSupp_Password, *((char*)msg->value ) ) )
			{
				Fault::AddFault( FAULT_AT_APP_PASSWORD_SET );
				//Log::Msg(LOG_SYS, "03_ATCommand.cpp -- ADD");
				return false;
			}
		}
	}
	else if ( msg->type == MSG_SET_LEDS )
	{
		if ( qspi->WriteToFlash( (char*) &fact_sett, QSPI_SETTINGS_ADDRESS, sizeof( fact_sett ) ) != QSPI_RESULT_OK )
		{
			if ( qspi->WriteToFlash( (char*) &fact_sett, QSPI_SETTINGS_ADDRESS, sizeof( fact_sett ) ) != QSPI_RESULT_OK )
			{
				// Fault::AddFault( FAULT_NO_QSPI_COMMS );	
			}
		}
	}
	
   return 0;
}


/*************************************************************************/
/**  \fn      void ATCommands::SetError( int err )
***  \brief   Class function
**************************************************************************/

void ATCommands::SetError( int _err, int _line )
{
	lastError = _err;
	lastLine  = _line; 
}

#if 0
extern "C" void my_smtp_result_fn(void *arg, uint8_t smtp_result, uint16_t srv_err, err_t err)
 {
   app.DebOut("mail (%p) sent with results: 0x%02x, 0x%04x, 0x%08x\n", arg,
          smtp_result, srv_err, err);
 }
#endif
 
int ATCommands::DecodeCommand( char* rxbuffer, char* txbuffer )
{
	int num;
 	
	app.DebOut( "Cmd: %s\n", rxbuffer );
#if 0	
	if ( !strncmp( rxbuffer, "SMPT", 4 ) )
	{		 
		smtp_set_server_addr( "smtp.office365.com" );
		// set both username and password as NULL if no auth needed
		smtp_set_auth("matthew.ault@cygnus-systems.com", "Xscale600" );
		smtp_send_mail("panel@cygnus-systems.com", "matthew.ault@cygnus-systems.com", "subject", "body", my_smtp_result_fn,
							0 );
	}
#endif	
	if ( !strncmp( rxbuffer, "ATMCUFW?", 8 ) )
	{
		sprintf( txbuffer, "%.2f\r\n", FIRMWARE_VERSION );
	}
	
	else if ( !strncmp( rxbuffer, "ATLEVEL?", 8 ) )
	{
		sprintf( txbuffer, "LEVEL:%d\r\n", Settings::access_level );
	}
	
	
	else if ( !strncmp( rxbuffer, "ATURL=", 6 ) )
	{
		char* p = strchr( rxbuffer, '\r' );
		
		if ( p == NULL )
		{
			p = strchr( rxbuffer, '\n' );
			
			if ( p == NULL )
			{
				sprintf( txbuffer, "URL:??\r\n" ); 
				return false;
			}
		}
		
		*p = 0;
		
		static char url[ 61];
	
		STRNCPY( url, rxbuffer + 6, 61 );
		Send( MSG_SET_URL, this, (int) url ); 
		
		sprintf( txbuffer, "URL:OK\r\n" );
	}
	
	else if ( !strncmp( rxbuffer, "ATNCUFW?", 8 ) )
	{
		Command cmd;
		
		Command0( CMD_GET_NCU_FW_VERSION, &cmd );
		
		sprintf( txbuffer, "%d.%d.%d\r\n", cmd.char2, cmd.char1, cmd.char0 );
	}

	else if ( !strncmp( rxbuffer, "ATPING", 6 ) )	
	{				
		sprintf( txbuffer, "PING:OK\r\n" );
	}	
	else if ( !strncmp( rxbuffer, "ATNCU=\"", 7 ) )	
	{
		char* p = rxbuffer + 7;		
		
//		if ( *p == '+' && p[1] == '+' && p[2] == '+' ) 
//		{
//			sprintf( txbuffer, "OK\r\n" );
//			return true;
//		}
		char* end = (char*) memchr( p, '"', 57 );
				
		if ( end )
		{
			int len = end - p;
			STRNCPY( ncu_cmd, p, len );
			ncu_cmd[ len ] = 0;
			Send( MSG_SEND_CMD );
			*txbuffer = 0;
		}
		else
		{
			sprintf( txbuffer, "NCU:ERROR\r\n" );
		}
	}	
	else if ( sscanf( rxbuffer, "ATLEDON,%d", &num ) == 1 )
	{
		GPIOManager::Set( (GPIO) ( GPIO_SounderFault + num ), 1 );
	}
	else if ( sscanf( rxbuffer, "ATLEDOFF,%d", &num ) == 1 )
	{
		GPIOManager::Set( (GPIO) ( GPIO_SounderFault + num ), 0 );
	}	
	else if ( sscanf( rxbuffer, "ATDFUMODE,%d", &num ) == 1 )
	{
		Command1( CMD_SET_FW_LENGTH, num );
		Send( EW_DFU_MODE_MSG, ew );
		sprintf( txbuffer, "DFUMODE:OK\r\n" );
		
		return true;
	}			
	else if ( !strncmp( rxbuffer, "ATRESULT", 8 ) )
	{
		sprintf( txbuffer, "RESULT:%d,\"%s\"\r\n", lastLine, GetErrMsg( lastError ) );
		
		return true;
	}	
	else if ( !strncmp( rxbuffer, "ATUID?", 6 ) )
	{
		sprintf( txbuffer, "UID:%08X%08X%08X\r\n", HAL_GetUIDw0(), HAL_GetUIDw1(), HAL_GetUIDw2() );
	}	
	else if ( !strncmp( rxbuffer, "ATNAME=", 7 ) )
	{
		char* start = strchr( rxbuffer, '"' ) + 1;
		
		if ( start!= NULL )
		{
			char* end = strchr( start, '"' );
			
			if ( end != NULL )
			{
				*end = 0;
				
				// Bit of a hack - we set xml importer name here..
				STRNCPY( ixml->panel_name, start, SITE_MAX_STRING_LEN );
				
				sprintf( txbuffer, "NAME:OK\r\n" );
			}
		}		
	}
//	else if ( !strncmp( rxbuffer, "ATLOW+", 6 ) )
//	{
//		System_Low( );
//		sprintf( txbuffer, "OK\r\n" );
//	}
//	else if ( !strncmp( rxbuffer, "ATHIGH+", 6 ) )
//	{
//		SystemClock_Config( RCC_OSCILLATORTYPE_HSE, RCC_LSEDRIVE_HIGH );
//		sprintf( txbuffer, "OK\r\n" );
//	}
	else if ( !strncmp( rxbuffer, "ATDISPON+", 9 ) )
	{
		Command1( CMD_SET_BACKLIGHT_ON, 0 );
		sprintf( txbuffer, "OK\r\n" );
	}
	else if ( !strncmp( rxbuffer, "ATDISPOFF+", 10 ) )
	{
		Command1( CMD_SET_BACKLIGHT_OFF, 0 );
		sprintf( txbuffer, "OK\r\n" );
	}
	else if ( !strncmp( rxbuffer, "ATSET?", 6 ) )	// AT
	{
		sprintf( txbuffer, "ATSET:%d,%d,%d\r\n", FACTORY_SETTINGS->zones, FACTORY_SETTINGS->leds, FACTORY_SETTINGS->devices );
	}
	else if ( !strncmp( rxbuffer, "ATSET=", 6 ) )	// ATSET=<zones>,<leds>,<devs>
	{
		int zones, leds, devs;
		
		if ( sscanf( rxbuffer + 6, "%d,%d,%d", &zones, &leds, &devs ) == 3 )
		{
			fact_sett.leds = leds;
			fact_sett.zones = zones;
			fact_sett.devices = devs;
			
			if ( (fact_sett.zones & 15) || ( fact_sett.leds & 15 ) || fact_sett.devices == 0 )
			{
				sprintf( txbuffer, "Bad numbers\r\n" );
			}
			else
			{
				Send( MSG_SET_LEDS, this, 0 );
				sprintf( txbuffer, "OK\r\n" );
			}
		}
		else
		{
			sprintf( txbuffer, "ERROR\r\n" );
		}
	}
	else if ( !strncmp( rxbuffer, "ATNAME?", 7 ) )
	{
		if ( app.panel != NULL )
		{
			sprintf( txbuffer, "NAME:%s\r\n", app.panel->name );
		}
		else
		{
			sprintf( txbuffer, "NAME:<Unknown>\r\n" );
		}
	}
	else if ( !strncmp( rxbuffer, "ATV?", 4 ) )
	{
		char version[ 8 ];
		 
		sprintf( txbuffer, "V:%0.2f\r\n", (float)FIRMWARE_VERSION );
	}
	else if ( !strncmp( rxbuffer, "ATISSITE?", 9  ) )	
	{				
		if ( app.site != NULL )
		{
			sprintf( txbuffer, "SITE:OK\r\n" );
		}
		else
		{
			sprintf( txbuffer, "SITE:NONE\r\n" );
		}
	}				
	else if ( !strncmp( rxbuffer, "ATWDG+", 6 ) )	
	{
		while(1);
	}
	else 
	{
		gsm->SendReceive( rxbuffer );
		sprintf( txbuffer, "UKNOWN\r\n" );

		return false;
	}
	return true;	
} 


