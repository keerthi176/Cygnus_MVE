


#include "MM_CommandTable.h"

typedef struct
{
	CmdNum num;
	const char* name;
	char flags;
	const char* valid;
} ATCmdDef;

 
CmdDef gbl_cmddef_list[ 7];

ATCmdDef gbl_atdef_list[ ] =
{
	{ CMD_BACKLIGHT_LEVEL, 			"BACKL", 	CMD_WRITE, 		"0-100" },																				
	{ CMD_TIME,							"TIME",		CMD_READWRITE, "0-23, 0-59, 0-59" },
	{ CMD_DATE,							"DATE", 		CMD_READWRITE,	"1-31, 1-12, 2018-3018" },
	{ CMD_DST_START,					"DSTSTART", CMD_READWRITE,	"0-23, 1-31, 1-12, 2018-3018" },
	{ CMD_DST_END,						"DSTEND",	CMD_READWRITE,	"0-23, 1-31, 1-12, 2018-3018" },
	{ CMD_CALIB_VALUES,				"CALIB",		CMD_READWRITE,	"0-65535, 0-65535, 0-65535, 0-65535, 0-65535, 0-65535, 0-65535, 0-65535" },
	{ CMD_PSU_STATUS,					"PSU",    	CMD_READ, 		"GOOD|LOW|CRITICAL|HIGH|FAIL" },
};



/*************************************************************************/
/**  \fn      void AT( char* cmd )
***  \brief   Global helper function
**************************************************************************/

void AT( char* cmd )
{
}	
	
/*************************************************************************/
/**  \fn      void FillTable( )
***  \brief   Global helper function
**************************************************************************/

void FillTable( )
{
	gbl_cmddef_list[ 0] = CmdDef( CMD_BACKLIGHT_LEVEL, 			"BACKL", 	CMD_WRITE, 		"0-100" );																				
	gbl_cmddef_list[ 1] = CmdDef( CMD_TIME,							"TIME",		CMD_READWRITE, "0-23, 0-59, 0-59" );
	gbl_cmddef_list[ 2] = CmdDef( CMD_DATE,							"DATE", 		CMD_READWRITE,	"1-31, 1-12, 2018-3018" );
	gbl_cmddef_list[ 3] = CmdDef( CMD_DST_START,					"DSTSTART", 	CMD_READWRITE,	"0-23, 1-31, 1-12, 2018-3018" );
	gbl_cmddef_list[ 4] = CmdDef( CMD_DST_END,						"DSTEND",	CMD_READWRITE,	"0-23, 1-31, 1-12, 2018-3018" );
	gbl_cmddef_list[ 5] = CmdDef( CMD_CALIB_VALUES,				"CALIB",			CMD_READWRITE,	"0-65535, 0-65535, 0-65535, 0-65535, 0-65535, 0-65535, 0-65535, 0-65535" );
	gbl_cmddef_list[ 6] = CmdDef( CMD_PSU_STATUS,					"PSU",    	CMD_READ, 		"GOOD|LOW|CRITICAL|HIGH|FAIL" );
}
	
/*************************************************************************/
/**  \fn      CmdDef::CmdDef()
***  \brief   Constructor for class
**************************************************************************/

CmdDef::CmdDef(){ };

/*************************************************************************/
/**  \fn      CmdDef::CmdDef( CmdNum num, const char* name, int flags, const char* valid )
***  \brief   Constructor for class
**************************************************************************/

CmdDef::CmdDef( CmdNum num, const char* name, int flags, const char* valid ) : num( num ), name( name ), flags( flags ), num_args( 0 ) 
{
	char* pch;
	char tmp[ 256];
	int i;
	
	// Calc number of arguments
	for ( i = 0; valid[i]; i++ )
		num_args += (valid[i] == '.') & 1;
	
	// Allocate number of arg defs
	CmdArg* def = argdef = new CmdArg[ num_args];
	
	// Copy ROM to RAM
	strcpy( tmp, valid );
	
	// Parse each arg
	pch = strtok( tmp, " ," );
	
	while ( pch != NULL )
	{
		AddArg( pch, def++ );
		pch = strtok( NULL, " ," );
	}
}


#define digit(x) ( (x) >= '0' && (x) <= '9' )
	

/*************************************************************************/
/**  \fn      int CmdDef::AddArg( char* valid, CmdArg* def )
***  \brief   Class function
**************************************************************************/

int CmdDef::AddArg( char* valid, CmdArg* def )
{
	char* p = valid;
	
	int i = 0;
	
	if ( digit( *p ) )
	{
		while( digit( *p ) )
		{
			i = i * 10;
			i += *p++ - '0';	
		}
		if ( *p++ == '-' )
		{
			def->min = i;
			i = 0;
			while( digit( *p ) )
			{
				i = i * 10;
				i += *p++ - '0';	
			}
			def->max = i;
			if ( def->min < def->max )
			{
				def->type = AT_INT;
				return 0;
			}
		}
	}
	else 
	{
		def->type = AT_STRING;
		def->txt = valid;
		
		return 0;
	}
	return -1;
}







	
	
//Cmd( CMD_TIME, day, month, year );






	
	
	