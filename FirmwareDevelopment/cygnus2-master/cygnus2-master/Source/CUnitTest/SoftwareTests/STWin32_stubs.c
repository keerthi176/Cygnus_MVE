/*************************************************************************************
*  CONFIDENTIAL between PA and Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2017 PA Consulting Group. All rights reserved.
*  Cambridge Technology Centre
*  Melbourn Royston
*  Herts SG8 6DP UK
*  Tel: <44> 1763 261222
*
**************************************************************************************
*  File         : STWin32_hal.c
*
*  Description  : Implementation of Miscellaneous Stubs for WIN32 testing
*
*************************************************************************************/



/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* User Include Files
*************************************************************************************/
#include "stm32l4xx.h"
#include "STWin32_stubs.h"
#include "cmsis_os.h"
#include "eeprom_emul.h"
#include "MM_Main.h"
#include "DM_NVM.h"

/* Private Functions Prototypes
*************************************************************************************/


/* Global Variables
*************************************************************************************/
stwin32_stub_application_image_t *stwin32_stub_application_image_p;
bool jump_to_application_success = false;
COM_StatusTypeDef stub_ymodem_result = COM_OK;
char sw_reset_debug_message[SW_RESET_DEBUG_MESSAGE_SIZE];
uint16_t sw_reset_msg_indication_flag;
uint32_t rbu_ppu_mode_request;
bool  bErrorHandlerCalled = false;
const char sw_version_nbr[] = "1.1.1";
const char sw_version_date[] = "01/01/01";
const uint32_t blf_application_checksum = 0x12345678;
uint32_t rbu_pp_mode_request;
uint32_t active_flash_bank = 1;
uint32_t gHeadQueueCount = 0;
const char CIE_ERROR_CODE_STR[AT_CIE_ERROR_MAX_E][3] =
{
   [AT_CIE_ERROR_NONE_E] = { "00" },
   [AT_CIE_ERROR_UNKNOWN_COMMAND_E] = { "01" },
   [AT_CIE_ERROR_VALUE_OUT_OF_RANGE_E] = { "02" },
   [AT_CIE_ERROR_WRONG_VALUE_FORMAT_E] = { "03" },
   [AT_CIE_ERROR_RU_DOES_NOT_EXIST_E] = { "04" },
   [AT_CIE_ERROR_CHANNEL_DOES_NOT_EXIST_E] = { "05" },
   [AT_CIE_ERROR_QUEUE_EMPTY_E] = { "06" },
   [AT_CIE_ERROR_TX_BUFFER_FULL_E] = { "07" },
   [AT_CIE_ERROR_NETWORK_TIMEOUT_E] = { "08" }
};

/* Private Variables
*************************************************************************************/

/*************************************************************************************/
/**
* app_main
* Application stub
*
* no params
*
* @return - none
*/
void app_main(void)
{
   jump_to_application_success = true;
}

COM_StatusTypeDef Ymodem_Receive(uint32_t *p_size, uint32_t bank)
{
   *p_size = sizeof(stwin32_stub_application_image_t);
   return stub_ymodem_result;
}

void GpioSetInterrupt(Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority, GpioIrqHandler *irqHandler, uint32_t pull)
{
}

uint32_t GpioRead(Gpio_t *obj)
{
	if (obj == &FirstAidMCP)
	{
		return TestFirstAidMCPState;
	}
	else if (obj == &FireMCP)
	{
		return TestFireMCPState;
	}
	else if (obj == &Tamper1)
	{
		return TestTamper1State;
	}
	else if (obj == &Tamper2)
	{
		return TestTamper2State;
	}
	else if (obj == &MainBattery)
	{
		return TestMainBatteryState;
	}
	else
	{
		// This is not supposed to happen
		return GPIO_PIN_RESET;
	}
}

void HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority){}
void HAL_NVIC_EnableIRQ(IRQn_Type IRQn){}

uint32_t Str2Int(uint8_t *inputstr, uint32_t *intnum){return 0;}
uint32_t FLASH_If_ErasePages(uint32_t Start, uint32_t number_of_sectors){return 0;}
uint32_t FLASH_If_GetWriteProtectionStatus(void){  return 0;}
uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length){ return 0;}
uint32_t FLASH_If_WriteProtectionConfig(uint32_t protectionstate){return 0;}
HAL_StatusTypeDef Serial_PutString(const uint8_t *const p_string){   return HAL_OK;}

uint32_t os_Kernel_Sys_Tick = 0;
uint32_t osKernelSysTick(void)
{
	return os_Kernel_Sys_Tick;
}

void Error_Handler(char* error_msg)
{
   bErrorHandlerCalled = true;
}

uint8_t* eepromAddress;
EE_Status eeprom_return_value = EE_OK;

void SetEepromAddress(uint8_t *address)
{
	eepromAddress = address;
}

EE_Status EE_Init(uint16_t* VirtAddTab, EE_Erase_type EraseType)
{
	return eeprom_return_value;
}

EE_Status EE_Format(EE_Erase_type EraseType)
{
	return eeprom_return_value;
}

EE_Status EE_ReadVariable32bits(uint16_t VirtAddress, uint32_t* pData)
{
	memcpy(pData, eepromAddress + (VirtAddress*4), 4);
	return eeprom_return_value;
}

EE_Status EE_WriteVariable32bits(uint16_t VirtAddress, uint32_t Data)
{
	uint32_t new_value = Data;
	memcpy(eepromAddress + (VirtAddress * 4), &new_value, 4);

	return eeprom_return_value;
}

void ConfigureCrc(void)
{

}

osMutexId osMutexCreate(const osMutexDef_t *mutex_def)
{
	osMutexId sid = NULL;
	if (mutex_def)
	{
		sid = (osMutexId)mutex_def;
	}

	return sid;
}

osStatus osMutexWait(osMutexId mutex_id, uint32_t millisec)
{
	return osOK;
}

osStatus osMutexRelease(osMutexId mutex_id)
{
	return osOK;
}

void MM_ConfigSerial_ErrorCallback(uint8_t channel_id)
{

}

void serial_init(void)
{

}

void serial_deinit(void)
{

}

EE_Status EE_CopyDataToBank(uint32_t Bank)
{
	return EE_OK;
}

ErrorCode_t MM_MAC_BuiltInTestReq(void)
{
	return SUCCESS_E;
}

ErrorCode_t MM_HeadInterface_BuiltInTestReq(void)
{
	return SUCCESS_E;
}

ErrorCode_t MM_SVI_BuiltInTestReq(void)
{
	return SUCCESS_E;
}

void GpioWrite(Gpio_t *obj, uint32_t value)
{
	/* store state in pinIndex field */
	obj->pinIndex = value;
}

ErrorCode_t DM_Crc16bCalculate8bDataWidth(uint8_t *pBuffer, uint32_t length, uint16_t *crc_value, uint32_t timeout_ms)
{
	*crc_value = stub_crc_value;
	return SUCCESS_E;
}

uint32_t FLASH_If_GetActiveBank(void)
{
   return active_flash_bank;
}