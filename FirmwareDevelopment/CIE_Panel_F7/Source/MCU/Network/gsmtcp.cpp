

#include "lwip/opt.h"

#include "lwip/altcp.h"

#include "MM_GSM.h"

unsigned int HAL_GetTick( void );

const ip_addr_t ip_addr_any = IPADDR4_INIT(IPADDR_ANY);

struct tcp_pcb* tcp_active_pcbs = NULL;
struct tcp_pcb *tcp_tw_pcbs = NULL;

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
	
	MQTTSocket( const char* hub, char* device ) : Socket( hub, device,  8883 )
	{
		
	}

	virtual void Receive( char* data, int len, int packet_num, bool more )
	{
	}
	
	void Connect( )
	{
		static char pwbuff[ 600 ];
		
	   mqtt_client = mqtt_client_new( );

		mqtt_client_info.client_id = DEVICE;
		mqtt_client_info.client_user = HUB"/"DEVICE;
		

		
	  if ( MakeSASToken( HUB, DEVICE, 60*60*100, pwbuff ) )
	  {	
		  mqtt_client_info.client_pass = pwbuff;
		  mqtt_set_inpub_callback(mqtt_client,
					 mqtt_incoming_publish_cb,
					 mqtt_incoming_data_cb,
					 LWIP_CONST_CAST(void*, &mqtt_client_info));

		  mqtt_client_connect(mqtt_client,
					 &mqtt_ip, MQTT_PORT,
					 mqtt_connection_cb, LWIP_CONST_CAST(void*, &mqtt_client_info),
					 &mqtt_client_info);
		}
		tcp_connected_fn connected;
};



struct tcp_pcb gsm_tcp;

tcpwnd_size_t output_buff[ 512 ];

extern "C"
{


u32_t sys_now(void)
{
  return HAL_GetTick();
}


void tcp_tmr(void)
{
  
}

struct tcp_pcb* tcp_new_ip_type (u8_t type)
{
	gsm_tcp.snd_buf = 512;
	
	
	return &gsm_tcp;
}


err_t tcp_output( struct tcp_pcb *pcb)
{
	return ERR_OK;
}


err_t tcp_write( struct tcp_pcb *pcb, const void *dataptr, u16_t len, u8_t apiflags )
{
	static char no_header = 0;
	 
	GSM::Send( &gsm_socket, &no_header, (char*) dataptr, len );
	
	return ERR_OK;
}


void tcp_recved( struct tcp_pcb *pcb, u16_t len)
{
	
}


err_t tcp_bind( struct tcp_pcb *pcb, const ip_addr_t *ipaddr, u16_t port )
{	
	return ERR_OK;
}


void tcp_abort( struct tcp_pcb *pcb )
{
	
}


err_t tcp_connect( struct tcp_pcb *conn, const ip_addr_t *ipaddr, u16_t port, tcp_connected_fn connected )
{
//	*(int*) gsm_socket.dest_ip = *(int*)ipaddr;
	
//	gsm_socket.dest_port = port;
	
	gsm_socket.connected = connected;
	
	return ERR_OK;
}


err_t tcp_close(struct tcp_pcb *pcb)
{
	GSM::state = GSM_CLOSE_SOCKET;
	
	return ERR_OK;
}

void tcp_arg(struct tcp_pcb *pcb, void *arg)
{
	
}

void             tcp_poll    (struct tcp_pcb *pcb, tcp_poll_fn poll, u8_t interval)
{
	
}
	
void             tcp_recv    (struct tcp_pcb *pcb, tcp_recv_fn recv)
{
	
}

void             tcp_sent    (struct tcp_pcb *pcb, tcp_sent_fn sent)
{
}

void             tcp_err     (struct tcp_pcb *pcb, tcp_err_fn err)
{
}	

void             tcp_accept  (struct tcp_pcb *pcb, tcp_accept_fn accept)
{
}


}

