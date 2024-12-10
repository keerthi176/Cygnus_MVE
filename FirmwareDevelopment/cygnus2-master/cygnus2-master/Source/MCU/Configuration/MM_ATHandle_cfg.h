/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2018 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : MM_ATHandle_cfg.h
*
*  Description  : Header file of the configuration of the AT Handle Module
*
*************************************************************************************/

#ifndef MM_AT_HANDLE_CFG_H
#define MM_AT_HANDLE_CFG_H


/* System Include files
*******************************************************************************/




/* User Include files
*******************************************************************************/


/* Public Structures
*******************************************************************************/
#ifdef IS_PPU
#define ATHANDLE_REPLY_WAIT_TIMEOUT_MS 2000u
#else
#define ATHANDLE_REPLY_WAIT_TIMEOUT_MS 10000u
#endif

/* Public Enumerations
*******************************************************************************/ 


/* Public functions prototypes
*******************************************************************************/


/* Public Constants
*******************************************************************************/


/* Macros
*******************************************************************************/


#endif
