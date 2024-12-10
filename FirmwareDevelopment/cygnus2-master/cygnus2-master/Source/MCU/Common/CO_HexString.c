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
*  File         : CO_HexString.c
*
*  Description  : This module provides hex to and from String utilities.
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* User Include Files
*************************************************************************************/
#include "CO_HexString.h"



/* Private Functions Prototypes
*************************************************************************************/


/* Global Variables
*************************************************************************************/


/* Private Variables
*************************************************************************************/





/*************************************************************************************/
/**
* function name	: MM_hexdec
* description		: Converts hex string to decimal number
*
* @param - unsigned const char *hex
*
* @return - long ret
*/
//long MM_hexdec(unsigned const char *hex)
//{
//   static const long hextable[] = {
//   [0 ... 255] = -1, 
//   ['0'] = 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
//   ['A'] = 10, 11, 12, 13, 14, 15, 
//   ['a'] = 10, 11, 12, 13, 14, 15 
//   };
   //bit aligned access into this table is considerably
	 // faster for for most modern processors
   // for the space conscious, reduce to
	 // signed char
//   long ret = 0;
//   while (*hex && ret >=0)
//   {
//      ret = (ret << 4) | hextable[*hex++];
//   }
//   return ret;
//}

/*************************************************************************************/
/**
* function name   : CO_HexString
* description     : Converts int to string
*
* @param - int32_t value
* @param - int32_t base
*
* @return - char *result
*/
 char * CO_HexString (uint32_t value, char *result, int32_t base)
{
   if ( result )
   {
       // check that the base if valid
       if (base < 2 || base > 36) { *result = '\0'; return result; }

       char* ptr = result, *ptr1 = result, tmp_char;
       int32_t tmp_value;

       do {
           tmp_value = value;
           value /= base;
           *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
       } while ( value );

       // Apply negative sign
       if (tmp_value < 0) *ptr++ = '-';
       *ptr-- = '\0';
       while (ptr1 < ptr) {
           tmp_char = *ptr;
           *ptr--= *ptr1;
           *ptr1++ = tmp_char;
       }
    }
    return result;
}


