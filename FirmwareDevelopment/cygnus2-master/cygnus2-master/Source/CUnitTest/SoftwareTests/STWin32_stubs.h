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
*  File         : STWin32_hal.h
*
*  Description  : Header file for the WIN32 stubs other than the HAL stubs
*
*************************************************************************************/

#ifndef ST_WIN32_STUBS_H
#define ST_WIN32_STUBS_H


/* System Include Files
*************************************************************************************/
#include<stdint.h>
#include<stdbool.h>


/* User Include Files
*************************************************************************************/
#include "eeprom_emul.h"

/* Public Enumerations
*************************************************************************************/
/**
* @brief  Comm status structures definition
*/
typedef enum
{
	COM_OK = 0x00,
	COM_ERROR = 0x01,
	COM_ABORT = 0x02,
	COM_TIMEOUT = 0x03,
	COM_DATA = 0x04,
	COM_LIMIT = 0x05
} COM_StatusTypeDef;

/* Error code */
enum
{
	FLASHIF_OK = 0,
	FLASHIF_ERASEKO,
	FLASHIF_WRITINGCTRL_ERROR,
	FLASHIF_WRITING_ERROR,
	FLASHIF_PROTECTION_ERRROR
};

/* Public Structures
*************************************************************************************/
typedef struct
{
   uint32_t top_of_main_stack_address;
   uint32_t reset_function_address;
   uint32_t prog_check_value;
}stwin32_stub_application_image_t;

/*!
* Operation Mode for the GPIO
*/
typedef enum
{
	PIN_INPUT = 0,
	PIN_OUTPUT,
	PIN_ALTERNATE_FCT,
	PIN_ANALOGIC
}PinModes;

/*!
* Add a pull-up, a pull-down or nothing on the GPIO line
*/
typedef enum
{
	PIN_NO_PULL = 0,
	PIN_PULL_UP,
	PIN_PULL_DOWN
}PinTypes;

/*!
* Define the GPIO as Push-pull type or Open Drain
*/
typedef enum
{
	PIN_PUSH_PULL = 0,
	PIN_OPEN_DRAIN
}PinConfigs;

/*!
* Define the GPIO IRQ on a rising, falling or both edges
*/
typedef enum
{
	NO_IRQ = 0,
	IRQ_RISING_EDGE,
	IRQ_FALLING_EDGE,
	IRQ_RISING_FALLING_EDGE
}IrqModes;

/*!
* Define the IRQ priority on the GPIO
*/
typedef enum
{
	IRQ_VERY_LOW_PRIORITY = 0,
	IRQ_LOW_PRIORITY,
	IRQ_MEDIUM_PRIORITY,
	IRQ_HIGH_PRIORITY,
	IRQ_VERY_HIGH_PRIORITY
}IrqPriorities;

/*
 * Structure for the GPIO
 */
typedef struct
{
	uint32_t  pin;
	uint16_t pinIndex;
	void *port;
	uint16_t portIndex;
}Gpio_t;

/*!
* GPIO IRQ handler function prototype
*/
typedef void(GpioIrqHandler)(void);

typedef enum
{
   AT_CIE_ERROR_NONE_E,
   AT_CIE_ERROR_UNKNOWN_COMMAND_E,
   AT_CIE_ERROR_VALUE_OUT_OF_RANGE_E,
   AT_CIE_ERROR_WRONG_VALUE_FORMAT_E,
   AT_CIE_ERROR_RU_DOES_NOT_EXIST_E,
   AT_CIE_ERROR_CHANNEL_DOES_NOT_EXIST_E,
   AT_CIE_ERROR_QUEUE_EMPTY_E,
   AT_CIE_ERROR_TX_BUFFER_FULL_E,
   AT_CIE_ERROR_NETWORK_TIMEOUT_E,
   AT_CIE_ERROR_MAX_E
} AT_CIE_ErrorCode_t;


/* Public Functions Prototypes
*************************************************************************************/
extern uint32_t Str2Int(uint8_t *inputstr, uint32_t *intnum);
extern COM_StatusTypeDef Ymodem_Receive(uint32_t *p_size, uint32_t bank);
extern uint32_t FLASH_If_ErasePages(uint32_t Start, uint32_t number_of_sectors);
extern uint32_t FLASH_If_GetWriteProtectionStatus(void);
extern uint32_t FLASH_If_Write(uint32_t destination, uint32_t *p_source, uint32_t length);
extern uint32_t FLASH_If_WriteProtectionConfig(uint32_t protectionstate);
extern void ST_BlfBoot_application_call(void);
extern void FLASH_If_Init(void);
void GpioSetInterrupt(Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority, GpioIrqHandler *irqHandler, uint32_t pull);
extern uint32_t GpioRead(Gpio_t *obj);
extern void SetEepromAddress(uint8_t* address);
uint32_t FLASH_If_GetActiveBank(void);
EE_Status EE_CopyDataToBank(uint32_t Bank);
HAL_StatusTypeDef FLASH_If_BankSwitch(void);

/* Public Constants
*************************************************************************************/
#define SW_RESET_DEBUG_MESSAGE_SIZE  (126u)
extern char sw_reset_debug_message[SW_RESET_DEBUG_MESSAGE_SIZE];
extern uint16_t sw_reset_msg_indication_flag;

extern stwin32_stub_application_image_t *stwin32_stub_application_image_p;
extern uint32_t regMainStackPointer;
extern uint32_t EEPROMbuffer[10];
extern uint32_t test_hal_tick_value;
extern uint32_t os_Kernel_Sys_Tick;
extern uint32_t TestFirstAidMCPState;
extern uint32_t TestFireMCPState;
extern uint32_t TestTamper1State;
extern uint32_t TestTamper2State;
extern uint32_t TestMainBatteryState;
extern bool jump_to_application_success;
extern COM_StatusTypeDef stub_ymodem_result;
extern Gpio_t FirstAidMCP;
extern Gpio_t FireMCP;
extern Gpio_t Tamper1;
extern Gpio_t Tamper2;
extern Gpio_t MainBattery;
extern uint32_t stub_crc_value;
extern EE_Status eeprom_return_value;
extern uint32_t os_Kernel_Sys_Tick;

/* Macros
*************************************************************************************/
#define CF_BL_ACTUAL_APP_ADDR  ((uint32_t)stwin32_stub_vector_table_p)
#define CF_BL_APP_IMAGE_ADDR   ((uint32_t)stwin32_stub_vector_table_p)
#define CF_BL_APP_RES_PROG_SIZE 12

#define CO_STATIC
#define FLASH_BASE 0
#define CO_ASSERT_RET_MSG(a,b,str)
#define CO_ASSERT_VOID_MSG(a,str)

#define FLASH_BANK_1              ((uint32_t)0x01)                          /*!< Bank 1   */

#endif /* ST_WIN32_HAL_H */
