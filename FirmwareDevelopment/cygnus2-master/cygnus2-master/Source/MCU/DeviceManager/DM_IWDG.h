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
*  File         : DM_IWDG.h
*
*  Description  : Header for the Independent Watchdog
*
*************************************************************************************/

#ifndef DM_IWDG_H
#define DM_IWDG_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>


/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"


/* Public Structures
*************************************************************************************/ 
extern IWDG_HandleTypeDef   IwdgHandle;


/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
extern ErrorCode_t DM_IndependentWdgInit(void);


/* Public Constants
*************************************************************************************/



/* Macros
*************************************************************************************/



#endif // DM_IWDG_H
