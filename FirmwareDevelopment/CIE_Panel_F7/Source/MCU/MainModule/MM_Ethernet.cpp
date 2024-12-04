/***************************************************************************
* File name: MM_Ethernet.cpp
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
* Remote networking
*
**************************************************************************/

#include "MM_Ethernet.h"
#include "MM_Application.h"

#include <stdarg.h>


extern "C"
{

#include "lwip/opt.h"
#include "lwip/init.h"
#include "netif/etharp.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "ethernetif.h"
#include "main.h"
#include "app_ethernet.h"
#include "http_cgi_ssi.h"
	
	/**
  * @brief  DHCP periodic check
  * @param  localtime the current LocalTime value
  * @retval None
  */
extern void DHCP_Periodic_Handle(struct netif *netif);

}

#define USE_DHCP

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
struct netif gnetif;


static void Netif_Config(void);
static void MPU_Config(void);

extern "C" void LCD_UsrLog( const char* fmt, ... )
{
	char buffer[ 256];
   va_list args;
	
   va_start( args, fmt );
   vsprintf( buffer, fmt, args );
   HAL_UART_Transmit( ::app.debug_uart, (uint8_t*) buffer, strlen( buffer ), 300 );
   va_end( args );
}	

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
Ethernet::Ethernet( ) : Module( "Ethernet", 10 )
{
  /* Configure the MPU attributes as Device memory for ETH DMA descriptors */
  MPU_Config();
  
  /* Initialize the LwIP stack */
  lwip_init();
  
  /* Configure the Network interface */
  Netif_Config();

  /* Http webserver Init */
  http_server_init();

  /* Notify user about the network interface config */
  User_notification(&gnetif);
}


void Ethernet::Poll( )
{
    /* Read a received packet from the Ethernet buffers and send it 
       to the lwIP for handling */
    ethernetif_input(&gnetif);

    /* Handle timeouts */
    sys_check_timeouts();

#ifdef USE_DHCP
    /* handle periodic timers for DHCP */
    DHCP_Periodic_Handle(&gnetif);
#endif 
  
}



/**
  * @brief  Setup the network interface
  * @param  None
  * @retval None
  */
static void Netif_Config(void)
{
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;
 
#ifdef USE_DHCP
  ip_addr_set_zero_ip4(&ipaddr);
  ip_addr_set_zero_ip4(&netmask);
  ip_addr_set_zero_ip4(&gw);
#else
  IP_ADDR4(&ipaddr,IP_ADDR0,IP_ADDR1,IP_ADDR2,IP_ADDR3);
  IP_ADDR4(&netmask,NETMASK_ADDR0,NETMASK_ADDR1,NETMASK_ADDR2,NETMASK_ADDR3);
  IP_ADDR4(&gw,GW_ADDR0,GW_ADDR1,GW_ADDR2,GW_ADDR3);
#endif /* USE_DHCP */
  
  /* add the network interface */    
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);
  
  /*  Registers the default network interface */
  netif_set_default(&gnetif);
  
  if (netif_is_link_up(&gnetif))
  {
    /* When the netif is fully configured this function must be called */
    netif_set_up(&gnetif);
  }
  else
  {
    /* When the netif link is down this function must be called */
    netif_set_down(&gnetif);
  }
  
  /* Set the link callback function, this function is called on change of link status*/
  netif_set_link_callback(&gnetif, ethernetif_update_config);
}
#if 0
/**
  * @brief  EXTI line detection callbacks
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  /* Get the IT status register value */
  if(BSP_IO_ITGetStatus(MII_INT_PIN))
  {
    ethernetif_set_link(&gnetif);
  }
  BSP_IO_ITClear();
}
#endif



/**
  * @brief  Configure the MPU attributes .
  * @param  None
  * @retval None
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;
  
  /* Disable the MPU */
  HAL_MPU_Disable();
  
  /* Configure the MPU attributes as WT for SRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x20020000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  
  /* Configure the MPU as Normal Non Cacheable for Ethernet Buffers in the SRAM2 */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x2007C000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_16KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER1;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  
  /* Configure the MPU as Device for Ethernet Descriptors in the SRAM2 */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x2007C000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256B;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER2;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  
  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}


/*************************************************************************/
/**  \fn      int Ethernet::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int Ethernet::Receive( Event event )
{
	return 0;
}


/*************************************************************************/
/**  \fn      int Ethernet::Receive( Command* cmd  )
***  \brief   Class function
**************************************************************************/

int Ethernet::Receive( Command* cmd )
{
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int Ethernet::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int Ethernet::Receive( Message* )
{
	return 0;
}
