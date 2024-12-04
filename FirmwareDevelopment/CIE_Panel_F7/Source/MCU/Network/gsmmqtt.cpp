

#include "lwip/opt.h"

#include "lwip/altcp.h"

unsigned int HAL_GetTick( void );

const ip_addr_t ip_addr_any = IPADDR4_INIT(IPADDR_ANY);

struct tcp_pcb* tcp_active_pcbs = NULL;
struct tcp_pcb *tcp_tw_pcbs = NULL;


struct tcp_pcb gsm_tcp;

u32_t sys_now(void)
{
  return HAL_GetTick();
}


void tcp_tmr(void)
{
  
}

struct tcp_pcb* tcp_new_ip_type (u8_t type)
{
	return &gsm_tcp;
}


err_t tcp_output( struct tcp_pcb *pcb)
{
	
}

err_t tcp_write( struct tcp_pcb *pcb, const void *dataptr, u16_t len, u8_t apiflags)
{
	
}

void tcp_recved( struct tcp_pcb *pcb, u16_t len)
{
}

err_t tcp_bind( struct tcp_pcb *pcb, const ip_addr_t *ipaddr, u16_t port )
{
	
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

