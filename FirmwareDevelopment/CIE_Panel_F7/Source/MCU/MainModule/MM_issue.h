/***************************************************************************
* File name: MM_issue.h
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
* ADC Watcher
*
**************************************************************************/

#ifndef MM_ISSUES_H_
#define MM_ISSUES_H_


typedef enum
{
	ISSUE_SITE_CHECKSUM_FAILURE,
	ISSUE_QSPI_SITE_WRITE_FAIL,
	ISSUE_QUADSPI_INIT_FAIL,
	ISSUE_QUADSPI_READ_FAIL,
	ISSUE_QSPI_THRESHOLD_REACHED,
	
	ISSUE_EEPROM_FAIL,
	ISSUE_EEPROM_READ_FAIL,
	ISSUE_EEPROM_WRITE_FAIL,
	ISSUE_EEPROM_NOT_READY,
	
	ISSUE_CAN_START_FAIL,
	ISSUE_CAN_ACTIVATION_FAIL,
	ISSUE_CAN_FILTER_CONFIG_FAIL,
	ISSUE_CAN_TX_FAIL,
	ISSUE_CAN_RX_FAIL,
	
	ISSUE_I2C_FAIL,
	
	ISSUE_FLASH_OPTION_BYTES_FAIL,
	ISSUE_NO_TOUCH_IC_FOUND,
	ISSUE_DATA_INTEGRITY_CHECK_FAIL,
	
	ISSUE_CRYSTAL_FAILURE,
	
	ISSUE_MAINS_FAILURE,
	ISSUE_BATTERY_LOW,
	ISSUE_BATTERY_CRITICAL,
	ISSUE_CHARGER_FAILED,
	ISSUE_PSU_CONFIG_ERROR,
	ISSUE_BATTERY_RESISTANCE_TOO_HIGH,
	ISSUE_PSU_OVER_VOLTAGE,
	
	ISSUE_HEAP_FULL,
	ISSUE_CREATE_FILE_FAIL,
	ISSUE_OPEN_FILE_FAIL,
	ISSUE_PIPE_ALREADY_EXISTS
} Issue;
	
#endif
