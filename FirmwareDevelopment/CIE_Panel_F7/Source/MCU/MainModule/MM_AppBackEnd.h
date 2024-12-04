

#include <stdarg.h>

#include "MM_list.h"





#define APP_SUPP_MAX_ARGS	3

enum AppSupportInfo
{
	APP_INFO_PANEL_ADDED,
	APP_INFO_PANEL_DROPPED,
	APP_INFO_DEVICE_ADDED,
	APP_INFO_DEVICE_DROPPED,
	APP_INFO_ENTER_EVENT,
	APP_INFO_LEAVE_EVENT,
	APP_INFO_FAULT_ADDED,
	APP_INFO_FAULT_REMOVED,
};

enum AppSupportRequest
{
	APP_REQ_SILENCE,
	APP_REQ_RE_SOUND,
	APP_REQ_CONFIRM,
	APP_REQ_ACKNOWLEDGE,
	APP_REQ_RESET,
	APP_REQ_DEVICE_STATUS,
	APP_REQ_FAULT_STATUS,
	APP_REQ_EVENT_STATUS,
	APP_REQ_ALL_DEVICES,
	APP_REQ_ALL_FAULTS,
	APP_REQ_ALL_EVENTS,
};


struct AppSupportArgs
{
	AppSupportArgs( AppSupportInfo& _info )
	{
		a_arg args;
		va_start( args, _info );
		
		arg[ 0] = va_arg( args, int );
		arg[ 1] = va_arg( args, int );
		arg[ 2] = va_arg( args, int );
		
		info = _info;
	}	
		
	AppSupportInfo info;	
	int arg[ APP_SUPP_MAX_ARGS ];
};


char* infoformat[] =
{
	"DEV+%d,%d,\"%s\"",		// DEV+<unit_num>,<zone_num>,"<location>"
	"DEV-%d",					// DEV-<unit_num>
	"EVT+%d",					// EVT+<event_num>
	"EVT-%d"	,					// EVT-<event_num>
	"FLT+%d",					// FLT+<fault_num>
	"EVT-%d",					// FLT-<fault_num>
};


char* reqformat[] =
{
	"SIL",					 
	"RES",					
	"CNF",					
	"ACK"	,					
	"RST",					
	"DEV?%d",				// DEV?<unit_num>
	"FLT?%d",				// FLT?<unit_num>
	"EVT?%d",				// EVT?<unit_num>
	"?DEV",
	"?FLT",
	"?EVT"
};
	
	
class AppSupport : public Module
{
	public:
	
	AppSupport( );
	int Send( AppSupportInfo info, ... );
	
	private:
		
	HTTPSocket http;
	
	Circular<AppSupportArgs> outqueue;
	 
}	
	

	
int AppSupport::Send( AppSupportInfo info, ... )
{	 
	if ( ! outqueue.Full( ) )
	{
		AppSupportArgs asa( info );
	
		outqueue.push( asa );
	}
}
	
	


