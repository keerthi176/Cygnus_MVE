/***************************************************************************
* File name: DM_BatteryRam.h
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
* Log define list.
*
**************************************************************************/

#ifndef _BATTERY_RAM_H_
#define _BATTERY_RAM_H_

enum
{
	BATT_RAM_DFU_IMAGE_SIZE = 0,
	BATT_RAM_CRYSTAL_FAULT,
	BATT_RAM_WATCHDOG_FAULT,
	BATT_RAM_WATCHDOG_REASON,
	BATT_RAM_NCU_FAULT
};

enum
{
	WD_REASON_UNKNOWN = 0,
	WD_REASON_HARD_FAULT,
	WD_REASON_BUS_FAULT,
	WD_REASON_USAGE_FAULT,
	WD_REASON_MEM_FAULT,
};

#endif
