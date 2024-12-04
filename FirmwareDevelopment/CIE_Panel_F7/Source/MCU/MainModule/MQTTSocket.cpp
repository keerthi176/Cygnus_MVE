

#include "sha.h"
#include "base64.h"
#include "lwip/opt.h"
#include "lwip/altcp.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"
#include "MM_GSM.h"

#include <string.h>
#include "MQTTSocket.h"

const ip_addr_t ip_addr_any = IPADDR4_INIT(IPADDR_ANY);

struct tcp_pcb* tcp_active_pcbs = NULL;
struct tcp_pcb *tcp_tw_pcbs = NULL;


extern "C" int hmac(SHAversion whichSha, const unsigned char *text, int text_len,
    const unsigned char *key, int key_len,
    uint8_t digest[USHAMaxHashSize]);

int MakeSASToken( const char* hub, const char* device, char* key, time_t expiry, char* result )
{
	char tokenExpirationTime[32];

	char* decodedKey;

	if ( ( decodedKey = Base64Dec( key, 43 ) ) == NULL )
	{		
		return false;
	}
	else
	{			
		snprintf( tokenExpirationTime, sizeof(tokenExpirationTime), "%u", expiry );
		
		char toBeHashed[ 512 ];
		char scope[ 128 ];
		char hash[ 512 ];
		  
		snprintf( scope, sizeof( scope ), "%s%%2Fdevices%%2F%s", hub, device );
		
		/*Codes_SRS_SASTOKEN_06_009: [The scope is the basis for creating a char*.]*/
		/*Codes_SRS_SASTOKEN_06_010: [A "\n" is appended to that string.]*/
		/*Codes_SRS_SASTOKEN_06_011: [tokenExpirationTime is appended to that string.]*/
		
		strncpy( toBeHashed, scope, sizeof(toBeHashed) );
		strncat( toBeHashed, "\n", sizeof(toBeHashed) );
		strncat( toBeHashed, tokenExpirationTime, sizeof(toBeHashed) );

		char* base64Signature = NULL;
		char* urlEncodedSignature = NULL;
		size_t inLen = strlen(toBeHashed);
		const char* inBuf = (const char*) toBeHashed ;
		size_t outLen = 32;
		char* outBuf = decodedKey;
		
	 	int len = hmac( SHA256, (const unsigned char*)inBuf, inLen, (const unsigned char*)outBuf, outLen, (uint8_t*) hash ) ;
		
		base64Signature = Base64Enc( hash, 32 ) ;
		
		base64Signature[ 44 ] = 0;	// terminate
		
		urlEncodedSignature = URLEnc( base64Signature );

		strcpy( result, "SharedAccessSignature sr=" );
		strcat( result, scope);
		strcat( result, "&sig=");
		strcat( result, urlEncodedSignature );
		strcat( result, "&se=");
		strcat( result, tokenExpirationTime );
 
	}

	return true;
}


extern "C" void mqtt_output_append_fixed_header(struct mqtt_ringbuf_t *rb, u8_t msg_type, u8_t fdup,
                                u8_t fqos, u8_t fretain, u16_t r_length);

static struct mqtt_connect_client_info_t mqtt_client_info =
{
  NULL, /*client */
  NULL, /* user */
  NULL, /* pass */
  240,  /* keep alive */
  NULL, /* will_topic */
  NULL, /* will_msg */
  0,    /* will_qos */
  0     /* will_retain */
#if LWIP_ALTCP && LWIP_ALTCP_TLS
  , NULL
#endif
};

static mqtt_client_t mqtt_client = { 0 };
static Socket* mqtt_owner;
static ip_addr_t mqtt_ip;


MQTTSocket::MQTTSocket( const char* hub ) : Socket( hub, NULL, 8883, PROT_TCP, true )
{
	
}


char* MQTTSocket::Init( time_t expiry, char* device, char* key, int& connection_len, void (*reset_cb)( void ) )
{
	static char pwbuff[ 256 ];
	static char userbuff[ 128 ];
				
	connection_len = 0;
	reset = reset_cb;
	lastrequest = 0;	// force early ping
	
  if ( MakeSASToken( dest_domain, device, key, expiry, pwbuff ) )
  {	
	  mqtt_owner = this;
	  mqtt_client_info.client_pass = pwbuff;
	  sprintf( userbuff, "%s/%s", dest_domain, device );
	  mqtt_client_info.client_user = userbuff;
	  mqtt_client_info.client_id = device;
		
	  mqtt_client.output.get = 0;
	  mqtt_client.output.put = 0;  
	  
	  app.DebOut("MQTT pass: %s\n", mqtt_client_info.client_pass );
	  app.DebOut("MQTT user: %s\n", mqtt_client_info.client_user );
	  app.DebOut("MQTT  uid: %s\n", mqtt_client_info.client_id );
	  
	  ((mqtt_client_s*)&mqtt_client)->conn->owner = this; 

	  if ( mqtt_client_connect( &mqtt_client,
				 &mqtt_ip, MQTT_PORT,
				 NULL, LWIP_CONST_CAST(void*, &mqtt_client_info),
				 &mqtt_client_info) == ERR_OK )
		{
			connection_len = mqtt_client.output.put - mqtt_client.output.get;
			return (char*) mqtt_client.output.buf + mqtt_client.output.get;
		}					 
	}
	return NULL;
}


void MQTTSocket::Receive( char* data, int len, int packet_num, bool more )
{
	 response( data, len );
}
	

void MQTTSocket::Reset( )
{
	reset( );
}


int MQTTSocket::Post( IOTMsg msgtype, void (*callback)( char* data, int length ), char* data )
{	
	if ( GSM::state == GSM_IDLE )
	{		
		char topic[ 200];
		snprintf( topic, 200, "devices/%s/messages/events/type=%d", mqtt_client_info.client_id, msgtype );
		
		app.DebOut( "post topic: %s\n", topic );
		app.DebOut( "data: %s\n", data );
		
		mqtt_client.output.get = 0;
		mqtt_client.output.put = 0;
		mqtt_publish( &mqtt_client, topic, data, strlen( data ), 1, 0,
                                   NULL, NULL );
	
		response = callback;
		
		return true;
	}
	return 0;
}
	

int MQTTSocket::Connect( char* connection, int connection_len, void (*callback)( char* data, int length ) )
{
	 if ( connection_len > 0 )
	 {
		 response = callback;
		 GSM::Send( this, connection, connection_len );
		 return true;		 
	 }
   return 0;
}

  
void MQTTSocket::Ping( void (*callback)( char* data, int length ) )
{	
	 mqtt_client.output.get = 0;
	 mqtt_client.output.put = 0;
	 mqtt_output_append_fixed_header( &mqtt_client.output, 12, 0, 0, 0, 0 );
	
	 int ping_len = mqtt_client.output.put - mqtt_client.output.get;
	
	 GSM::Send( this, (char*) (mqtt_client.output.buf + mqtt_client.output.get), ping_len );
		
	 response = callback;
}


struct tcp_pcb gsm_tcp;

 

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
	GSM::Send( mqtt_owner, (char*)dataptr, len );

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
	return ERR_OK;
}


err_t tcp_close(struct tcp_pcb *pcb)
{
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

 
