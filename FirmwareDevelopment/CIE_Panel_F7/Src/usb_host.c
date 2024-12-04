/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file            : usb_host.c
  * @version         : v1.0_Cube
  * @brief           : This file implements the USB Host
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "usb_host.h"
#include "usbh_core.h"
//#include "usbh_msc.h"
#include "usbh_ftdi.h"
#include "usbh_hid.h"
#include "usbh_hub.h"
#include "usbh_conf.h"
#include "usbh_dev.h"
#include "vgamode.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USB Host core handle declaration */
 
USBH_HandleTypeDef hUSBHost[ MAX_HUB_PORTS + 1];


/*
 * -- Insert your variables declaration here --
 */
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*
 * user callback declaration
 */
void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

/*
 * -- Insert your external function declaration here --
 */
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/**
  * Init USB host library, add supported class and start the library
  * @retval None
  */
  
void MX_USB_HOST_Init(void)
{
  /* USER CODE BEGIN USB_HOST_Init_PreTreatment */
   memset(&hUSBHost[0], 0, sizeof(USBH_HandleTypeDef));

	hUSBHost[0].valid   = 1;
	hUSBHost[0].address = USBH_DEVICE_ADDRESS;
	hUSBHost[0].Pipes   = USBH_malloc(sizeof(uint32_t) * USBH_MAX_PIPES_NBR);
  /* USER CODE END USB_HOST_Init_PreTreatment */
  
  /* Init host Library, add supported class and start the library. */
  USBH_Init(&hUSBHost[0], USBH_UserProcess, ID_USB_HOST_HS );
 
 
#if VGA_MODE==1  
  USBH_RegisterClass( &hUSBHost[0], USBH_HID_CLASS );	
  
  USBH_RegisterClass( &hUSBHost[0], USBH_HUB_CLASS);
#else
		USBH_RegisterClass( &hUSBHost[0], USBH_FTDI_CLASS );		// ftdi USB serial
#endif	
  
  if ( USBH_Start( &hUSBHost[0]) != USBH_OK )
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_HOST_Init_PostTreatment */
  
  /* USER CODE END USB_HOST_Init_PostTreatment */
}


 
  

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
