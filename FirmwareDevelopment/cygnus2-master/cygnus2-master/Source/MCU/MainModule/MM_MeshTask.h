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
*  File         : MM_MeshTask.h
*
*  Description  : Mesh Task header file
*
*************************************************************************************/

#ifndef MM_MESH_TASK_H
#define MM_MESH_TASK_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>



/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"
#include "cmsis_os.h"


/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t MM_MeshTaskInit(const uint32_t isSyncMaster, const uint32_t address, const uint32_t systemIdMM_MeshTaskInit);
void MM_MeshTaskMain(void const *argument);


/* Public Constants
*************************************************************************************/



/* Macros
*************************************************************************************/



#endif // MM_MESH_TASK_H
