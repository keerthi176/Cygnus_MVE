/**
  ******************************************************************************
  * @file    usbh_hid_mouse.c 
  * @author  MCD Application Team
  * @version V3.2.2
  * @date    07-July-2015
  * @brief   This file is the application layer for USB Host HID Mouse Handling.                  
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

#include "usbh_hid_mouse.h"
#include "usbh_hid_parser.h"

HID_MOUSE_Info_TypeDef   mouse_info;
//static uint8_t                  mouse_report_data[8];
uint32_t                  mouse_report_data[1];

USBH_StatusTypeDef USBH_HID_MouseDecode(USBH_HandleTypeDef *phost);

/* Structures defining how to access items in a HID mouse report */
/* Access button 1 state. */
static const HID_Report_ItemTypedef prop_b1={
  (uint8_t *)mouse_report_data+0, /*data*/
  1,     /*size*/
  0,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min value device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};

/* Access button 2 state. */
static const HID_Report_ItemTypedef prop_b2={
  (uint8_t *)mouse_report_data+0, /*data*/
  1,     /*size*/
  1,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min value device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};

/* Access button 3 state. */
static const HID_Report_ItemTypedef prop_b3={
  (uint8_t *)mouse_report_data+0, /*data*/
  1,     /*size*/
  2,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};

/* Access x coordinate change. */
static const HID_Report_ItemTypedef prop_x={
  (uint8_t *)mouse_report_data+1, /*data*/
  8,     /*size*/
  0,     /*shift*/
  0,     /*count (only for array items)*/
  1,     /*signed?*/
  0,     /*min value read can return*/
  0xFFFF,/*max value read can return*/
  0,     /*min vale device can report*/
  0xFFFF,/*max value device can report*/
  1      /*resolution*/
};

/* Access y coordinate change. */
static const HID_Report_ItemTypedef prop_y={
  (uint8_t *)mouse_report_data+2, /*data*/
  8,     /*size*/
  0,     /*shift*/
  0,     /*count (only for array items)*/
  1,     /*signed?*/
  0,     /*min value read can return*/
  0xFFFF,/*max value read can return*/
  0,     /*min vale device can report*/
  0xFFFF,/*max value device can report*/
  1      /*resolution*/
};

/**
  * @brief  USBH_HID_MouseInit 
  *         The function init the HID mouse.
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_HID_MouseInit(USBH_HandleTypeDef *phost)
{
  uint8_t idx = phost->device.current_interface;
  HID_HandleTypeDef *HID_Handle = phost->USBH_ClassTypeDef_pData[idx];

  mouse_info.x=0;
  mouse_info.y=0;
  mouse_info.buttons[0]=0;
  mouse_info.buttons[1]=0;
  mouse_info.buttons[2]=0;
  
  mouse_report_data[0]=0;
  
  if(HID_Handle->length[0] > sizeof(mouse_report_data))
  {
    HID_Handle->length[0] = sizeof(mouse_report_data);
  }
  HID_Handle->pData = (uint8_t *)mouse_report_data;
  fifo_init(&HID_Handle->fifo, phost->device.Data, HID_QUEUE_SIZE * sizeof(mouse_report_data));

  return USBH_OK;
}

/**
  * @brief  USBH_HID_GetMouseInfo 
  *         The function return mouse information.
  * @param  phost: Host handle
  * @retval mouse information
  */
HID_MOUSE_Info_TypeDef *USBH_HID_GetMouseInfo(USBH_HandleTypeDef *phost)
{
	if(USBH_HID_GetDeviceType(phost) == HID_MOUSE)
	{
		if(USBH_HID_MouseDecode(phost)== USBH_OK)
			return &mouse_info;
	}

	return NULL;
}

/**
  * @brief  USBH_HID_MouseDecode 
  *         The function decode mouse data.
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_HID_MouseDecode(USBH_HandleTypeDef *phost)
{
//  HID_HandleTypeDef *HID_Handle = (HID_HandleTypeDef *) phost->pActiveClass->pData;
	uint8_t idx = phost->device.current_interface;
	HID_HandleTypeDef *HID_Handle = phost->USBH_ClassTypeDef_pData[idx];

  if(HID_Handle->length[0] == 0)
  {
    return USBH_FAIL;
  }
  //Fill report
  if(fifo_read(&HID_Handle->fifo, &mouse_report_data, HID_Handle->length[0]) ==  HID_Handle->length[0])
  {
    //Decode report
    mouse_info.x = (int16_t )HID_ReadItem((HID_Report_ItemTypedef *) &prop_x, 0);
    mouse_info.y = (int16_t )HID_ReadItem((HID_Report_ItemTypedef *) &prop_y, 0);
    
    mouse_info.buttons[0]=(uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &prop_b1, 0);
    mouse_info.buttons[1]=(uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &prop_b2, 0);
    mouse_info.buttons[2]=(uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &prop_b3, 0);

    return USBH_OK;  
  }

  return   USBH_FAIL;
}


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


/**
  * @}
  */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
