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
*  File         : MM_NCUApplicationTask.h
*
*  Description  : Header for NCU Application
*
*************************************************************************************/

#ifndef MM_NCU_APPLICATION_TASK_H
#define MM_NCU_APPLICATION_TASK_H


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
ErrorCode_t MM_NCUApplicationTaskInit(const uint16_t address, const uint16_t device_configuration, const uint32_t unitSerialNo, const uint32_t defaultZoneNumber);
void MM_NCUApplicationTaskMain (void);



/* Public Constants
*************************************************************************************/



/* Macros
*************************************************************************************/



#endif // MM_NCU_APPLICATION_TASK_H
