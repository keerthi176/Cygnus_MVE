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
*  File         : MM_SviTask.h
*
*  Description  : Header for SVI (Sounder and Visual Indicator) task
*
*************************************************************************************/

#ifndef MM_SVI_TASK_H
#define MM_SVI_TASK_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>



/* User Include Files
*************************************************************************************/


/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t MM_SVI_Initialise(void);
ErrorCode_t MM_SVI_BuiltInTestReq(void);


/* Public Constants
*************************************************************************************/


/* Macros
*************************************************************************************/



#endif // MM_SVI_TASK_H
