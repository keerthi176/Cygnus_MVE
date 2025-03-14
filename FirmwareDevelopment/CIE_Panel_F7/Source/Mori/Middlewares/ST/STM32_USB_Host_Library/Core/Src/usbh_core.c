/**
  ******************************************************************************
  * @file    usbh_core.c 
  * @author  MCD Application Team
  * @version V3.2.2
  * @date    07-July-2015
  * @brief   This file implements the functions for the core state machine process
  *          the enumeration and the control transfer process
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 
/* Includes ------------------------------------------------------------------*/

#include "usbh_core.h"
#include "stm32f7xx_hal_hcd.h"


extern USBH_ClassTypeDef  HID_Class;
#define USBH_HID_CLASS    &HID_Class

extern USBH_HandleTypeDef hUSBHost[5];
extern HCD_HandleTypeDef _hHCD[2];
/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_LIB_CORE
  * @{
  */

/** @defgroup USBH_CORE 
  * @brief This file handles the basic enumeration when a device is connected 
  *          to the host.
  * @{
  */ 


/** @defgroup USBH_CORE_Private_Defines
  * @{
  */ 

/**
  * @}
  */ 

/** @defgroup USBH_CORE_Private_Macros
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBH_CORE_Private_Variables
  * @{
  */ 
/**
  * @}
  */ 
 

/** @defgroup USBH_CORE_Private_Functions
  * @{
  */ 
static USBH_StatusTypeDef  USBH_HandleEnum    (USBH_HandleTypeDef *phost);
static void                USBH_HandleSof     (USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef  DeInitStateMachine(USBH_HandleTypeDef *phost);

#if (USBH_USE_OS == 1)  
static void USBH_Process_OS(void const * argument);
#endif

/**
  * @brief  HCD_Init 
  *         Initialize the HOST Core.
  * @param  phost: Host Handle
  * @param  pUsrFunc: User Callback
  * @retval USBH Status
  */
USBH_StatusTypeDef  USBH_Init(USBH_HandleTypeDef *phost, void (*pUsrFunc)(USBH_HandleTypeDef *phost, uint8_t ), uint8_t id)
{
  /* Check whether the USB Host handle is valid */
  if(phost == NULL)
  {
    USBH_ErrLog("Invalid Host handle");
    return USBH_FAIL; 
  }
  
  /* Set DRiver ID */
  phost->id = id;
  
  /* Unlink class*/
  phost->pActiveClass = NULL;
  phost->ClassNumber = 0;
  
  /* Restore default states and prepare EP0 */ 
  DeInitStateMachine(phost);
  
  /* Assign User process */
  if(pUsrFunc != NULL)
  {
    phost->pUser = pUsrFunc;
  }
  
#if (USBH_USE_OS == 1) 
  
  /* Create USB Host Queue */
  osMessageQDef(USBH_Queue, 10, uint16_t);
  phost->os_event = osMessageCreate (osMessageQ(USBH_Queue), NULL); 
  
  /*Create USB Host Task */
#if defined (USBH_PROCESS_STACK_SIZE)
  osThreadDef(USBH_Thread, USBH_Process_OS, USBH_PROCESS_PRIO, 0, USBH_PROCESS_STACK_SIZE);
#else
  osThreadDef(USBH_Thread, USBH_Process_OS, USBH_PROCESS_PRIO, 0, 8 * configMINIMAL_STACK_SIZE);
#endif  
  phost->thread = osThreadCreate (osThread(USBH_Thread), phost);
#endif  
  
  /* Initialize low level driver */
  USBH_LL_Init(phost);
  return USBH_OK;
}

/**
  * @brief  HCD_Init 
  *         De-Initialize the Host portion of the driver.
  * @param  phost: Host Handle
  * @retval USBH Status
  */
USBH_StatusTypeDef  USBH_DeInit(USBH_HandleTypeDef *phost)
{
  DeInitStateMachine(phost);
  
  if(phost->pData != NULL)
  {
//    phost->pActiveClass->pData = NULL;
    USBH_LL_Stop(phost);
  }

  return USBH_OK;
}

/**
  * @brief  DeInitStateMachine 
  *         De-Initialize the Host state machine.
  * @param  phost: Host Handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef  DeInitStateMachine(USBH_HandleTypeDef *phost)
{
  uint32_t i = 0;

  /* Clear Pipes flags*/
  for ( ; i < USBH_MAX_PIPES_NBR; i++)
  {
    phost->Pipes[i] = 0;
  }
  
  for(i = 0; i< USBH_MAX_DATA_BUFFER; i++)
  {
    phost->device.Data[i] = 0;
  }
  
  phost->gState = HOST_IDLE;
  phost->EnumState = ENUM_IDLE;
  phost->RequestState = CMD_SEND;
  phost->Timer = 0;  
  
  phost->Control.state = CTRL_SETUP;
  phost->Control.pipe_size = USBH_MPS_DEFAULT;  
  phost->Control.errorcount = 0;
  
  phost->device.address = USBH_ADDRESS_DEFAULT;
  phost->device.speed   = USBH_SPEED_FULL;
  
  return USBH_OK;
}

/**
  * @brief  USBH_RegisterClass 
  *         Link class driver to Host Core.
  * @param  phost : Host Handle
  * @param  pclass: Class handle
  * @retval USBH Status
  */
USBH_StatusTypeDef  USBH_RegisterClass(USBH_HandleTypeDef *phost, USBH_ClassTypeDef *pclass)
{
  USBH_StatusTypeDef   status = USBH_OK;
  
  if(pclass != 0)
  {
    if(phost->ClassNumber < USBH_MAX_NUM_SUPPORTED_CLASS)
    {
      /* link the class to the USB Host handle */
      phost->pClass[phost->ClassNumber++] = pclass;
      status = USBH_OK;
    }
    else
    {
      USBH_ErrLog("Max Class Number reached");
      status = USBH_FAIL; 
    }
  }
  else
  {
    USBH_ErrLog("Invalid Class handle");
    status = USBH_FAIL; 
  }
  
  return status;
}

static int getInterfaceIdxFromNum(USBH_HandleTypeDef *phost, uint8_t num)
{
	int i = 0;
	for(;i < phost->device.CfgDesc.bNumInterfaces; ++i)
	{
		if(phost->device.CfgDesc.Itf_Desc[i].bInterfaceNumber == num)
			return i;
	}

	return -1;
}
/**
  * @brief  USBH_SelectInterface 
  *         Select current interface.
  * @param  phost: Host Handle
  * @param  interface: Interface number
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_SelectInterface(USBH_HandleTypeDef *phost, uint8_t interface)
{
  USBH_StatusTypeDef   status = USBH_OK;
  
USBH_UsrLog ("Switching to Interface (#%d)", interface);
  if(phost->device.current_interface != interface)
  {
	  int val = getInterfaceIdxFromNum(phost, interface);
	  if(val >= 0 && val < phost->device.CfgDesc.bNumInterfaces)
      {
		phost->device.current_interface = val; //interface;

		//USBH_UsrLog ("Class    : %xh", phost->device.CfgDesc.Itf_Desc[interface].bInterfaceClass );
		//USBH_UsrLog ("SubClass : %xh", phost->device.CfgDesc.Itf_Desc[interface].bInterfaceSubClass );
		//USBH_UsrLog ("Protocol : %xh", phost->device.CfgDesc.Itf_Desc[interface].bInterfaceProtocol );
      }
      else
      {
    USBH_ErrLog ("Cannot Select This Interface.");
    status = USBH_FAIL; 
	  }
  }
  return status;  
}

/**
  * @brief  USBH_GetActiveClass 
  *         Return Device Class.
  * @param  phost: Host Handle
  * @param  interface: Interface index
  * @retval Class Code
  */
uint8_t USBH_GetActiveClass(USBH_HandleTypeDef *phost)
{
   return (phost->device.CfgDesc.Itf_Desc[0].bInterfaceClass);            
}
/**
  * @brief  USBH_FindInterface 
  *         Find the interface index for a specific class.
  * @param  phost: Host Handle
  * @param  Class: Class code
  * @param  SubClass: SubClass code
  * @param  Protocol: Protocol code
  * @retval interface index in the configuration structure
  * @note : (1)interface index 0xFF means interface index not found
  */
uint8_t  USBH_FindInterface(USBH_HandleTypeDef *phost, uint8_t Class, uint8_t SubClass, uint8_t Protocol)
{
  USBH_InterfaceDescTypeDef    *pif ;
  USBH_CfgDescTypeDef          *pcfg ;
  int8_t                        if_ix = 0;
  
  pif = (USBH_InterfaceDescTypeDef *)0;
  pcfg = &phost->device.CfgDesc;  
  
  while (if_ix < USBH_MAX_NUM_INTERFACES)
  {
    pif = &pcfg->Itf_Desc[if_ix];
    if(((pif->bInterfaceClass == Class) || (Class == 0xFF))&&
       ((pif->bInterfaceSubClass == SubClass) || (SubClass == 0xFF))&&
         ((pif->bInterfaceProtocol == Protocol) || (Protocol == 0xFF)))
    {
      return  if_ix;
    }
//		USBH_UsrLog ("CLASS: 0x%02X, SUBCLASS: 0x%02X, PROTO: 0x%02X", pif->bInterfaceClass, pif->bInterfaceSubClass, pif->bInterfaceProtocol);
    if_ix++;
  }
  return 0xFF;
}

/**
  * @brief  USBH_FindInterfaceIndex 
  *         Find the interface index for a specific class interface and alternate setting number.
  * @param  phost: Host Handle
  * @param  interface_number: interface number
  * @param  alt_settings    : alternate setting number
  * @retval interface index in the configuration structure
  * @note : (1)interface index 0xFF means interface index not found
  */
uint8_t  USBH_FindInterfaceIndex(USBH_HandleTypeDef *phost, uint8_t interface_number, uint8_t alt_settings)
{
  USBH_InterfaceDescTypeDef    *pif ;
  USBH_CfgDescTypeDef          *pcfg ;
  int8_t                        if_ix = 0;
  
  pif = (USBH_InterfaceDescTypeDef *)0;
  pcfg = &phost->device.CfgDesc;  
  
  while (if_ix < USBH_MAX_NUM_INTERFACES)
  {
    pif = &pcfg->Itf_Desc[if_ix];
    if((pif->bInterfaceNumber == interface_number) && (pif->bAlternateSetting == alt_settings))
    {
      return  if_ix;
    }
    if_ix++;
  }
  return 0xFF;
}

/**
  * @brief  USBH_Start 
  *         Start the USB Host Core.
  * @param  phost: Host Handle
  * @retval USBH Status
  */
USBH_StatusTypeDef  USBH_Start  (USBH_HandleTypeDef *phost)
{
  /* Start the low level driver  */
  USBH_LL_Start(phost);
  
  /* Activate VBUS on the port */ 
  USBH_LL_DriverVBUS (phost, TRUE);
  
  return USBH_OK;  
}

/**
  * @brief  USBH_Stop 
  *         Stop the USB Host Core.
  * @param  phost: Host Handle
  * @retval USBH Status
  */
USBH_StatusTypeDef  USBH_Stop   (USBH_HandleTypeDef *phost)
{
  /* Stop and cleanup the low level driver  */
  USBH_LL_Stop(phost);  
  
  /* DeActivate VBUS on the port */ 
  USBH_LL_DriverVBUS (phost, FALSE);
  
  /* FRee Control Pipes */
  USBH_FreePipe  (phost, phost->Control.pipe_in);
  USBH_FreePipe  (phost, phost->Control.pipe_out);  
  
  return USBH_OK;  
}

/**
  * @brief  HCD_ReEnumerate 
  *         Perform a new Enumeration phase.
  * @param  phost: Host Handle
  * @retval USBH Status
  */
USBH_StatusTypeDef  USBH_ReEnumerate   (USBH_HandleTypeDef *phost)
{
  /*Stop Host */ 
  USBH_Stop(phost);

  /*Device has disconnected, so wait for 200 ms */  
  USBH_Delay(200);
  
  /* Set State machines in default state */
  DeInitStateMachine(phost);
   
  /* Start again the host */
  USBH_Start(phost);
      
#if (USBH_USE_OS == 1)
      osMessagePut ( phost->os_event, USBH_PORT_EVENT, 0);
#endif  
  return USBH_OK;  
}

/**
  * @brief  USBH_Process 
  *         Background process of the USB Core.
  * @param  phost: Host Handle
  * @retval USBH Status
  */
USBH_StatusTypeDef  USBH_Process(USBH_HandleTypeDef *phost)
{
  __IO USBH_StatusTypeDef status = USBH_FAIL;
  uint8_t idx = 0;
  
  switch (phost->gState)
  {
  case HOST_IDLE :
    
    if (phost->device.is_connected)  
    {
      /* Wait for 200 ms after connection */
      phost->gState = HOST_DEV_WAIT_FOR_ATTACHMENT; 
      USBH_Delay(200); 
      USBH_LL_ResetPort(phost);
		phost->Timeout = 0U;
#if (USBH_USE_OS == 1)
      osMessagePut ( phost->os_event, USBH_PORT_EVENT, 0);
#endif
    }
    break;
    
  case HOST_DEV_WAIT_FOR_ATTACHMENT:
		if ( phost->device.PortEnabled == 1U )
      {
        USBH_UsrLog("USB Device Reset Completed");
       
        phost->gState = HOST_DEV_ATTACHED;
      }
      else
      {
        if (phost->Timeout > USBH_DEV_RESET_TIMEOUT)
        {
          
            /* Buggy Device can't complete reset */
            USBH_UsrLog("USB Reset Failed, Please unplug the Device.");
            phost->gState = HOST_ABORT_STATE;
          
        }
        else
        {
          phost->Timeout += 10U;
          USBH_Delay(5U);
        }
      }
#if (USBH_USE_OS == 1U)
      phost->os_msg = (uint32_t)USBH_PORT_EVENT;
#if (osCMSIS < 0x20000U)
      (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
      (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, NULL);
#endif
#endif
    break;    
    
  case HOST_DEV_ATTACHED :
    
    USBH_UsrLog("USB Device Attached");  
      
    /* Wait for 100 ms after Reset */
    USBH_Delay(100); 
          
    if(!phost->device.is_connected)
    {
USBH_UsrLog ("DEVICE DISCONNECTED DURING CONNECTION DELAY");
    	phost->gState = HOST_DEV_DISCONNECTED;
    	return USBH_OK;
    }

    phost->device.speed = USBH_LL_GetSpeed(phost);
    
    phost->gState = HOST_ENUMERATION;
    
    phost->Control.pipe_out = USBH_AllocPipe (phost, 0x00);
    phost->Control.pipe_in  = USBH_AllocPipe (phost, 0x80);    
    

    if(phost->device.speed == USBH_SPEED_LOW)
    	phost->Control.pipe_size = USBH_MPS_LOWSPEED;
    
    /* Open Control pipes */
    USBH_OpenPipe (phost,
                   phost->Control.pipe_in,
                   0x80,
                   phost->device.address,
                   phost->device.speed,
                   USBH_EP_CONTROL,
                   phost->Control.pipe_size); 
    
    /* Open Control pipes */
    USBH_OpenPipe (phost,
                   phost->Control.pipe_out,
                   0x00,
                   phost->device.address,
                   phost->device.speed,
                   USBH_EP_CONTROL,
                   phost->Control.pipe_size);
    
#if (USBH_USE_OS == 1)
    osMessagePut ( phost->os_event, USBH_PORT_EVENT, 0);
#endif    
    
    break;
    
  case HOST_ENUMERATION:     
    /* Check for enumeration status */  
    if ( USBH_HandleEnum(phost) == USBH_OK)
    { 
      /* The function shall return USBH_OK when full enumeration is complete */
      USBH_UsrLog ("Enumeration done.");
      phost->device.current_interface = 0;
      if(phost->device.DevDesc.bNumConfigurations == 1)
      {
        USBH_UsrLog ("This device has only 1 configuration.");
        phost->gState  = HOST_SET_CONFIGURATION;        
        
      }
      else
      {
        phost->gState  = HOST_INPUT; 
      }
          
    }
    break;
    
  case HOST_INPUT:
    {
      /* user callback for end of device basic enumeration */
      if(phost->pUser != NULL)
      {
        phost->pUser(phost, HOST_USER_SELECT_CONFIGURATION);
        phost->gState = HOST_SET_CONFIGURATION;
        
#if (USBH_USE_OS == 1)
        osMessagePut ( phost->os_event, USBH_STATE_CHANGED_EVENT, 0);
#endif         
      }
    }
    break;
    
  case HOST_SET_CONFIGURATION:
    /* set configuration */
    if (USBH_SetCfg(phost, phost->device.CfgDesc.bConfigurationValue) == USBH_OK)
    {
      phost->gState  = HOST_CHECK_CLASS;
      USBH_UsrLog ("Default configuration set.");
      
    }      
    
    break;
    
  case HOST_CHECK_CLASS:
    
    if(hUSBHost[0].ClassNumber == 0)
    {
      USBH_UsrLog ("No Class has been registered.");
		phost->gState  = HOST_ABORT_STATE;
    }
    else
    {
      phost->pActiveClass = NULL;
      uint8_t itf = phost->device.current_interface;
		 
		for (idx = 0; idx < USBH_MAX_NUM_SUPPORTED_CLASS ; idx ++)
      {
    	  if(hUSBHost[0].pClass[idx] != NULL)
    	  {
    		  if ( hUSBHost[0].pClass[idx]->ClassCode == phost->device.CfgDesc.Itf_Desc[itf].bInterfaceClass)	 
			  {
				 phost->pActiveClass = hUSBHost[0].pClass[idx];
				  break;
			  }
    	  }
      } 
		
		// lets just hope its a hid..
      if ( phost->pActiveClass == NULL ) phost->pActiveClass = USBH_HID_CLASS;
      
      if(phost->pActiveClass != NULL)
      {
    	  status = phost->pActiveClass->Init(phost);
        if(status == USBH_OK)
        {
          phost->gState  = HOST_CLASS_REQUEST; 
          USBH_UsrLog ("%s class started.", phost->pActiveClass->Name);
          
          /* Inform user that a class has been activated */
          if(phost->pUser != NULL)
          phost->pUser(phost, HOST_USER_CLASS_SELECTED);   
        }
        else if(status != USBH_BUSY)
        {
          phost->gState  = HOST_ABORT_STATE;
          USBH_UsrLog ("Device not supporting %s class.", phost->pActiveClass->Name);
        }
      }
      else
      {
        phost->gState  = HOST_ABORT_STATE;
        USBH_UsrLog ("No registered class for this device.");
      }
    }
    
#if (USBH_USE_OS == 1)
    osMessagePut ( phost->os_event, USBH_STATE_CHANGED_EVENT, 0);
#endif 
    break;    
    
  case HOST_CLASS_REQUEST:  
    /* process class standard control requests state machine */
    if(phost->pActiveClass != NULL)
    {
      status = phost->pActiveClass->Requests(phost);
      
      if(status == USBH_OK)
      {
        phost->gState  = HOST_CLASS;        
      }  
    }
    else
    {
      phost->gState  = HOST_ABORT_STATE;
      USBH_ErrLog ("Invalid Class Driver.");
    
#if (USBH_USE_OS == 1)
    osMessagePut ( phost->os_event, USBH_STATE_CHANGED_EVENT, 0);
#endif       
    }
    
    break;    
  case HOST_CLASS:   
    /* process class state machine */
    if(phost->pActiveClass != NULL)
    { 
      phost->pActiveClass->BgndProcess(phost);
    }
    break;       

  case HOST_DEV_DISCONNECTED :
    
USBH_UsrLog("HOST_DEV_DISCONNECTED %d", phost->address);

	// MORI - Hub disconnecting, remove all plugged devices
	{
		int i;
		for(i = 1; i < 5; ++i)
		{
			if(hUSBHost[i].valid)
			{
				if(hUSBHost[i].pActiveClass != NULL)
				{
					hUSBHost[i].pActiveClass->DeInit(&hUSBHost[i]);
					hUSBHost[i].pActiveClass = NULL;
				}

				memset(&hUSBHost[i], 0, sizeof(USBH_HandleTypeDef));
			}
		}
	}
    DeInitStateMachine(phost);  
    
    /* Re-Initilaize Host for new Enumeration */
    if(phost->pActiveClass != NULL)
    {
      phost->pActiveClass->DeInit(phost); 
      phost->pActiveClass = NULL;
    }     
    break;
    
  case HOST_ABORT_STATE:
  default :
    break;
  }
 return USBH_OK;  
}


/**
  * @brief  USBH_HandleEnum 
  *         This function includes the complete enumeration process
  * @param  phost: Host Handle
  * @retval USBH_Status
  */
static USBH_StatusTypeDef USBH_HandleEnum (USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef Status = USBH_BUSY;  
  
  switch (phost->EnumState)
  {
  case ENUM_IDLE:  
    /* Get Device Desc for only 1st 8 bytes : To get EP0 MaxPacketSize */
    if ( USBH_Get_DevDesc(phost, 8) == USBH_OK)
    {
      phost->Control.pipe_size = phost->device.DevDesc.bMaxPacketSize;

      phost->EnumState = ENUM_GET_FULL_DEV_DESC;
      
      /* modify control channels configuration for MaxPacket size */
      USBH_OpenPipe (phost,
                           phost->Control.pipe_in,
                           0x80,
                           phost->device.address,
                           phost->device.speed,
                           USBH_EP_CONTROL,
                           phost->Control.pipe_size); 
      
      /* Open Control pipes */
      USBH_OpenPipe (phost,
                           phost->Control.pipe_out,
                           0x00,
                           phost->device.address,
                           phost->device.speed,
                           USBH_EP_CONTROL,
                           phost->Control.pipe_size);           
      
USBH_UsrLog("Enum mps: %d, addr: %d", phost->Control.pipe_size, phost->device.address);
    }
    break;
    
  case ENUM_GET_FULL_DEV_DESC:  
    /* Get FULL Device Desc  */
    if ( USBH_Get_DevDesc(phost, USB_DEVICE_DESC_SIZE)== USBH_OK)
    {
USBH_UsrLog("PID  : %xh", phost->device.DevDesc.idProduct );
USBH_UsrLog("VID  : %xh", phost->device.DevDesc.idVendor );
//USBH_UsrLog("PROTO: %xh", phost->device.DevDesc.bDeviceProtocol );
//USBH_UsrLog("CLASS: %xh", phost->device.DevDesc.bDeviceClass );
      
      phost->EnumState = ENUM_SET_ADDR;
       
    }
    break;
   
  case ENUM_SET_ADDR: 
    /* set address */
    if ( USBH_SetAddress(phost, phost->address) == USBH_OK)
    {
      USBH_Delay(2);
      phost->device.address = phost->address;
      
      /* user callback for device address assigned */
      USBH_UsrLog("Address (#%d) assigned.", phost->device.address);
      phost->EnumState = ENUM_GET_CFG_DESC;
      
      /* modify control channels to update device address */
      USBH_OpenPipe (phost,
                           phost->Control.pipe_in,
                           0x80,
                           phost->device.address,
                           phost->device.speed,
                           USBH_EP_CONTROL,
                           phost->Control.pipe_size); 
      
      /* Open Control pipes */
      USBH_OpenPipe (phost,
                           phost->Control.pipe_out,
                           0x00,
                           phost->device.address,
                           phost->device.speed,
                           USBH_EP_CONTROL,
                           phost->Control.pipe_size);        
    }
    break;
    
  case ENUM_GET_CFG_DESC:  
    /* get standard configuration descriptor */
    if ( USBH_Get_CfgDesc(phost, 
                          USB_CONFIGURATION_DESC_SIZE) == USBH_OK)
    {
      phost->EnumState = ENUM_GET_FULL_CFG_DESC;        
    }
    break;
    
  case ENUM_GET_FULL_CFG_DESC:  
    /* get FULL config descriptor (config, interface, endpoints) */
    if (USBH_Get_CfgDesc(phost, 
                         phost->device.CfgDesc.wTotalLength) == USBH_OK)
    {
      phost->EnumState = ENUM_GET_MFC_STRING_DESC;       
    }
    break;
    
  case ENUM_GET_MFC_STRING_DESC:  
    if (phost->device.DevDesc.iManufacturer != 0)
    { /* Check that Manufacturer String is available */
      
      if ( USBH_Get_StringDesc(phost,
                               phost->device.DevDesc.iManufacturer, 
                                phost->device.Data , 
                               0xff) == USBH_OK)
      {
        /* User callback for Manufacturing string */
        USBH_UsrLog("Manufacturer : %s",  (char *)phost->device.Data);
        phost->EnumState = ENUM_GET_PRODUCT_STRING_DESC;
        
#if (USBH_USE_OS == 1)
    osMessagePut ( phost->os_event, USBH_STATE_CHANGED_EVENT, 0);
#endif          
      }
    }
    else
    {
     USBH_UsrLog("Manufacturer : N/A");      
     phost->EnumState = ENUM_GET_PRODUCT_STRING_DESC; 
#if (USBH_USE_OS == 1)
    osMessagePut ( phost->os_event, USBH_STATE_CHANGED_EVENT, 0);
#endif       
    }
    break;
    
  case ENUM_GET_PRODUCT_STRING_DESC:   
    if (phost->device.DevDesc.iProduct != 0)
    { /* Check that Product string is available */
      if ( USBH_Get_StringDesc(phost,
                               phost->device.DevDesc.iProduct, 
                               phost->device.Data, 
                               0xff) == USBH_OK)
      {
        /* User callback for Product string */
        USBH_UsrLog("Product : %s",  (char *)phost->device.Data);
        phost->EnumState = ENUM_GET_SERIALNUM_STRING_DESC;        
      }
    }
    else
    {
      USBH_UsrLog("Product : N/A");
      phost->EnumState = ENUM_GET_SERIALNUM_STRING_DESC; 
#if (USBH_USE_OS == 1)
    osMessagePut ( phost->os_event, USBH_STATE_CHANGED_EVENT, 0);
#endif        
    } 
    break;
    
  case ENUM_GET_SERIALNUM_STRING_DESC:   
    if (phost->device.DevDesc.iSerialNumber != 0)
    { /* Check that Serial number string is available */    
      if ( USBH_Get_StringDesc(phost,
                               phost->device.DevDesc.iSerialNumber, 
                               phost->device.Data, 
                               0xff) == USBH_OK)
      {
        /* User callback for Serial number string */
         USBH_UsrLog("Serial Number : %s",  (char *)phost->device.Data);
        Status = USBH_OK;
      }
    }
    else
    {
      USBH_UsrLog("Serial Number : N/A"); 
      Status = USBH_OK;
#if (USBH_USE_OS == 1)
    osMessagePut ( phost->os_event, USBH_STATE_CHANGED_EVENT, 0);
#endif        
    }  
    break;
    
  default:
    break;
  }  
  return Status;
}

/**
  * @brief  USBH_LL_SetTimer 
  *         Set the initial Host Timer tick
  * @param  phost: Host Handle
  * @retval None
  */
void  USBH_LL_SetTimer  (USBH_HandleTypeDef *phost, uint32_t time)
{
  phost->Timer = time;
}
/**
  * @brief  USBH_LL_IncTimer 
  *         Increment Host Timer tick
  * @param  phost: Host Handle
  * @retval None
  */
void  USBH_LL_IncTimer  (USBH_HandleTypeDef *phost)
{
  phost->Timer ++;
  USBH_HandleSof(phost);
}

/**
  * @brief  USBH_HandleSof 
  *         Call SOF process
  * @param  phost: Host Handle
  * @retval None
  */
void  USBH_HandleSof  (USBH_HandleTypeDef *phost)
{
  if((phost->gState == HOST_CLASS)&&(phost->pActiveClass != NULL))
  {
    phost->pActiveClass->SOFProcess(phost);
  }
}
/**
  * @brief  USBH_LL_Connect 
  *         Handle USB Host connexion event
  * @param  phost: Host Handle
  * @retval USBH_Status
  */
USBH_StatusTypeDef  USBH_LL_Connect  (USBH_HandleTypeDef *phost)
{
  if(phost->gState == HOST_IDLE )
  {
    phost->device.is_connected = 1;
    phost->gState = HOST_IDLE ;
    
    if(phost->pUser != NULL)
    {    
      phost->pUser(phost, HOST_USER_CONNECTION);
    }
  } 
  else if(phost->gState == HOST_DEV_WAIT_FOR_ATTACHMENT )
  {
	USBH_Delay(100);
    phost->gState = HOST_DEV_ATTACHED ;
  }
#if (USBH_USE_OS == 1)
  osMessagePut ( phost->os_event, USBH_PORT_EVENT, 0);
#endif 
  
  return USBH_OK;
}

/**
  * @brief  USBH_LL_Disconnect 
  *         Handle USB Host disconnection event
  * @param  phost: Host Handle
  * @retval USBH_Status
  */
USBH_StatusTypeDef  USBH_LL_Disconnect  (USBH_HandleTypeDef *phost)
{
USBH_UsrLog("\rUSBH_LL_Disconnect [%d]", phost->address);

	// MORI - Always select the root device, mainly if its a hub
	USBH_HandleTypeDef *pphost = &hUSBHost[0];
	HCD_HandleTypeDef *phHCD   = &_hHCD[pphost->id];

	phHCD->pData = pphost;
	pphost->pData = phHCD;
	//////////////////////////////////////////////
  /*Stop Host */ 
  USBH_LL_Stop(pphost);  
  
  /* FRee Control Pipes */
  USBH_FreePipe  (pphost, pphost->Control.pipe_in);
  USBH_FreePipe  (pphost, pphost->Control.pipe_out);  
   
  pphost->device.is_connected = 0; 
   
  if(pphost->pUser != NULL)
  {    
    pphost->pUser(pphost, HOST_USER_DISCONNECTION);
  }
  
  /* Start the low level driver  */
  USBH_LL_Start(pphost);
  
  pphost->gState = HOST_DEV_DISCONNECTED;
  
#if (USBH_USE_OS == 1)
  osMessagePut ( phost->os_event, USBH_PORT_EVENT, 0);
#endif 

  USBH_UsrLog("USBH_LL_Disconnect OK");
  
  return USBH_OK;
}


/**
  * @brief  USBH_PortEnabled
  *         Port Enabled
  * @param  phost: Host Handle
  * @retval None
  */
void USBH_LL_PortEnabled(USBH_HandleTypeDef *phost)
{
  phost->device.PortEnabled = 1U;

#if (USBH_USE_OS == 1U)
  phost->os_msg = (uint32_t)USBH_PORT_EVENT;
#if (osCMSIS < 0x20000U)
  (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
  (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, NULL);
#endif
#endif

  return;
}


/**
  * @brief  USBH_LL_PortDisabled
  *         Port Disabled
  * @param  phost: Host Handle
  * @retval None
  */
void USBH_LL_PortDisabled(USBH_HandleTypeDef *phost)
{
  phost->device.PortEnabled = 0U;

  return;
}



#if (USBH_USE_OS == 1)  
/**
  * @brief  USB Host Thread task
  * @param  pvParameters not used
  * @retval None
  */
static void USBH_Process_OS(void const * argument)
{
  osEvent event;
  
  for(;;)
  {
    event = osMessageGet(((USBH_HandleTypeDef *)argument)->os_event, osWaitForever );
    
    if( event.status == osEventMessage )
    {
      USBH_Process((USBH_HandleTypeDef *)argument);
    }
   }
}

/**
* @brief  USBH_LL_NotifyURBChange 
*         Notify URB state Change
* @param  phost: Host handle
* @retval USBH Status
*/
USBH_StatusTypeDef  USBH_LL_NotifyURBChange (USBH_HandleTypeDef *phost)
{
  osMessagePut ( phost->os_event, USBH_URB_EVENT, 0);
  return USBH_OK;
}
#endif  
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
