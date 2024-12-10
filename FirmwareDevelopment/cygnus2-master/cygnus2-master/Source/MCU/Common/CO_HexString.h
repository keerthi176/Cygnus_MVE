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
*  File         : CO_HexString.h
*
*  Description  : AT Command functions header file
*
*************************************************************************************/

#ifndef CO_HEXSTRING_H
#define CO_HEXSTRING_H


/* System Include Files
*************************************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


/* User Include Files
*************************************************************************************/
//#include "main.h"
//#include "DM_SerialPort.h"
//#include "MM_ATHandleTask.h"

/* Public Structures
*************************************************************************************/ 




/* Public Enumerations
*************************************************************************************/




/* Public Functions Prototypes
*************************************************************************************/
char * CO_HexString (uint32_t value, char *result, int32_t base);
//long hexdec(unsigned const char *hex);

/* Public Constants
*************************************************************************************/



/* Macros
*************************************************************************************/
#define BASE_10               (10)


#endif // CO_HEXSTRING_H
