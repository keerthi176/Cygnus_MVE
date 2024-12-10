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
*  File         : DM_NVM.h
*
*  Description  : Header file for Non-Volatile Memory driver module
*
*************************************************************************************/

#ifndef DM_NVM_H
#define DM_NVM_H


/* System Include files
*******************************************************************************/
#include <stdint.h>


/* User Include files
*******************************************************************************/
#include "CO_ErrorCode.h"
#include "DM_NVM_cfg.h"

/* Public Structures
*******************************************************************************/


/* Public Enumerations
*******************************************************************************/ 


/* Public functions prototypes
*******************************************************************************/
ErrorCode_t DM_NVMInit(void);
ErrorCode_t DM_NVMRead(const DM_NVMParamId_t paramId, void *const pValue, const uint16_t size);
ErrorCode_t DM_NVMWrite(const DM_NVMParamId_t paramId, const void *const pValue, const uint16_t size);
DM_NVMParamId_t DM_NVM_ChannelIndexToNVMParameter(const uint32_t channel_index);

/* Public Constants
*******************************************************************************/
#define NVM_OUTPUT_PROFILE_MASK              0x0000007F
#define NVM_OUTPUT_OPTIONS_MASK              0x00000380
#define NVM_OUTPUT_LOCAL_DELAYS_MASK         0x00000400
#define NVM_OUTPUT_DELAY1_MASK               0x000FF800
#define NVM_OUTPUT_DELAY2_MASK               0x1FF00000
#define NVM_OUTPUT_DELAYS_MASK               0x1FFFF800
#define NVM_OUTPUT_SILENCEABLE_MASK          0x00000200
#define NVM_OUTPUT_IGNORE_NIGHT_DELAYS_MASK  0x00000100
#define NVM_OUTPUT_INVERTED_MASK             0x00000080
#define NVM_OUTPUT_FORMAT_MASK               0x00000380
#define NVM_OUTPUT_CHANNEL_ENABLED_MASK      0xC0000000

#define NVM_GLOBAL_OUTPUT_DELAY1_MASK        0x0000FFFF
#define NVM_GLOBAL_OUTPUT_DELAY2_MASK        0xFFFF0000

#define NVM_RESTART_BATTERY_TEST_MASK        0x00000003
#define NVM_RESTART_PRIMARY_BATTERY_MASK     0x00000001
#define NVM_RESTART_BACKUP_BATTERY_MASK      0x00000002

#define DM_CHANNEL_DAY_ENABLE_MASK 0x08
#define DM_CHANNEL_NIGHT_ENABLE_MASK 0x10
#define DM_ALARM_OPTION_FLAGS_MASK 0x07

/* Macros
*******************************************************************************/



#endif // DM_NVM_H
