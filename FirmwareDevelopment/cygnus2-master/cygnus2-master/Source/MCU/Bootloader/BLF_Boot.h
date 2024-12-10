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
*  File         : BLF_Boot.h
*
*  Description  : brief description
*
*************************************************************************************/

#ifndef BLF_BOOT_H
#define BLF_BOOT_H

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
void BLF_BootManager(void);
void BLF_request_serial_programming(void);
uint32_t BLF_get_boot_bank(void);
ErrorCode_t BLF_set_boot_bank(uint32_t Bank, bool update_eeprom);


/* Public Constants
*************************************************************************************/


/* Macros
*************************************************************************************/



#endif
