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
*  File         : MM_NCUApplicationStub.h
*
*  Description  : Header for NCU Application Test Stub
*
*************************************************************************************/
#ifdef USE_NCU_STUB
#ifndef MM_NCU_APPLICATION_STUB_H
#define MM_NCU_APPLICATION_STUB_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>



/* User Include Files
*************************************************************************************/



/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
int MM_NCUApplicationStubTaskInit(const uint16_t address, const uint16_t device_configuration, const uint32_t unitSerialNo, const uint32_t defaultZoneNumber);



/* Public Constants
*************************************************************************************/



/* Macros
*************************************************************************************/



#endif // MM_NCU_APPLICATION_STUB_H

#endif // USE_NCU_STUB
