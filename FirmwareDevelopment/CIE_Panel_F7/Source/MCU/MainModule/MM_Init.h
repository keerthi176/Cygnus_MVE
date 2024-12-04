/***************************************************************************
* File name: MM_Init.h
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
* MCU Initialisation
*
**************************************************************************/


/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "can.h"
#include "crc.h"
#include "dac.h"
#include "dma.h"
#include "dma2d.h"
#include "fatfs.h"
#include "i2c.h"
#include "iwdg.h"
#include "jpeg.h"
#include "lptim.h"
#include "ltdc.h"
#include "lwip.h"
#include "quadspi.h"
#include "rtc.h"
#include "sdmmc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "usb_host.h"
#include "gpio.h"
#include "fmc.h"
#include "can.h"


/* USER CODE BEGIN Includes */
#include "sdram.h"
#include "stm32f7xx_ll_rcc.h"
#include "vgamode.h"
/* USER CODE END Includes */



class Init
{
	public:
	Init( );
};

