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
*  File         : file name
*
*  Description  : brief description
*
*************************************************************************************/

#ifndef CF_BOOT_LOADER_H
#define CF_BOOT_LOADER_H


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/



/* Public Structures
*************************************************************************************/ 


/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/



/* Public Constants
*************************************************************************************/
#define CF_BLF_ENABLE_BACK_DOOR_REPROGRAM

#define CF_BLF_BACK_DOOR_KEY_WAIT_MS (2000u)

/* This is to stub the seed and key process */
#define CF_BLF_BACKDOOR_STATIC_KEY   (0x11223344u)

/* Enable STMicroelectronics Interactive menu */
//#define BLF_ENABLE_DEBUG_MENU_DISPLAY

/* Macros
*************************************************************************************/



#endif
