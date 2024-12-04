

#include "lwip/opt.h"
#include "lwip/altcp.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"
#include "MM_GSM.h"


const ip_addr_t ip_addr_any = IPADDR4_INIT(IPADDR_ANY);

struct tcp_pcb* tcp_active_pcbs = NULL;
struct tcp_pcb *tcp_tw_pcbs = NULL;

int MakeSASToken( const char* hub, const char* device, size_t expiry, char* result );



static struct mqtt_connect_client_info_t mqtt_client_info =
{
  NULL, /*client */
  NULL, /* user */
  NULL, /* pass */
  100,  /* keep alive */
  NULL, /* will_topic */
  NULL, /* will_msg */
  0,    /* will_qos */
  0     /* will_retain */
#if LWIP_ALTCP && LWIP_ALTCP_TLS
  , NULL
#endif
};
 
	
class MQTTSocket : public Socket
{
	public:
	
	MQTTSocket( const char* hub, char* device );
	 
	virtual void Receive( char* data, int len, int packet_num, bool more );
	
	int Post( const char* resource, void (*callback)( int code, char* data, int length ), char* data );
 
	void Connect( int timevalid );
	 
	void (*response)( char* data, int length ); 
	
	tcp_connected_fn connected;
};


