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
*  File         : MM_MACTask.h
*
*  Description  : MAC Task header file
*
*************************************************************************************/

#ifndef MM_MAC_TASK_H
#define MM_MAC_TASK_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"



/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t MM_MACTaskInit(const uint32_t isSyncMaster, const uint32_t address,  const uint32_t systemId);
void MM_MAC_TimerISR(void);
ErrorCode_t MM_MAC_BuiltInTestReq(void);
void MC_MAC_TxAtCommand( void );

/* Public Constants
*************************************************************************************/



/* Macros
*************************************************************************************/



#endif // MM_MAC_TASK_H
