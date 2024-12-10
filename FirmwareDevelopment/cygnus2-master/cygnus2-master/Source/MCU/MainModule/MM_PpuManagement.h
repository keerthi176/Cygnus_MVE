/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2022 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : MM_PpuManagement.h
*
*  Description  : Header for PPU message handler
*
*************************************************************************************/

#ifndef MM_PPU_MANAGEMENT_H
#define MM_PPU_MANAGEMENT_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>



/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "CO_Message.h"

/* Public Constants
*************************************************************************************/


/* Macros
*************************************************************************************/


/* Public Enumerations
*************************************************************************************/


/* Public Structures
*************************************************************************************/ 


/* Public Functions Prototypes
*************************************************************************************/
void MM_PPU_Initialise(void);
ErrorCode_t MM_PPU_ApplicationProcessPpuModeMessage(const ApplicationMessage_t* pAppMessage);
void MM_PPU_ApplicationProcessPpuPeriodicAnnouncement(void);
bool MM_PPU_SendPpuPeriodicAnnouncement(void);
void MM_PPU_ApplicationProcessPpuCommand(const ApplicationMessage_t* pAppMessage);
void MM_PPU_ProcessAtResponseMessage(const AtMessageData_t* pAtMessage);
void MM_PPU_ProcessRouteDropConfirmation(const uint32_t handle);
void MM_PPU_PeriodicChecks(void);
void MM_PPU_SetPpuMasterAddress(const uint16_t address);
ErrorCode_t CO_LockRNG( void );
ErrorCode_t CO_UnlockRNG( void );

#endif // MM_PPU_MANAGEMENT_H
