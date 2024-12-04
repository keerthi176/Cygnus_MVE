

#include "lwip/opt.h"
#include "lwip/altcp.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"
#include "MM_GSM.h"

 
enum IOTMsg
{
	IOT_PanelInit = 0,
	IOT_PanelEvent,
	IOT_PanelFault,
	IOT_DeviceDrop,
	IOT_DeviceUpdate,
	IOT_DeviceFault,
	IOT_ClearDeviceFaults,
	IOT_ClearDeviceWarnings,
	IOT_ClearDeviceFaultsAndWarnings,
};

	
 

extern "C" int MakeSASToken( const char* hub, const char* device, char* key, size_t expiry, char* result );


	
class MQTTSocket : public Socket
{
	public:
	
	MQTTSocket( const char* hub );
	 
	virtual void Receive( char* data, int len, int packet_num, bool more );
	virtual void Reset( );
	
	int Post( IOTMsg msg, void (*callback)( char* data, int length ), char* data );
 
	char* Init( time_t expiry, char* device, char* key, int& connection_len, void (*reset_cb)( void ) );
	int Connect( char* connection, int connection_len, void (*callback)( char* data, int length ) );
	void Ping( void (*callback)( char* data, int length ) );
	 
	void (*response)( char* data, int length ); 
	void (*reset)( ); 
};


