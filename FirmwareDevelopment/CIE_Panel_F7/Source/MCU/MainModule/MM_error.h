/***************************************************************************
* File name: MM_Error.h
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
* Error list
*
**************************************************************************/
#ifndef _MM_ERROR_H_
#define _MM_ERROR_H_

/* System Include Files
**************************************************************************/

/* User Include Files
**************************************************************************/
#include "MM_Application.h"


/* Defines
**************************************************************************/
//#define ERROR(e) app.Error( #e, e ); 

#define CHECK(h) if ( (h) != HAL_OK ) app.Error( #h, h )

typedef enum
{
	ERR_HAL_OK       = 0x00U,
   ERR_HAL_ERROR    = 0x01U,
   ERR_HAL_BUSY     = 0x02U,
   ERR_HAL_TIMEOUT  = 0x03U,
	
	ERR_UNKNOWN_CMD,
	ERR_TOO_MANY_TIMERS, 	
	ERR_MODULE_DEPENDANCY,
	
	ERR_LAST
} Error;

#endif
