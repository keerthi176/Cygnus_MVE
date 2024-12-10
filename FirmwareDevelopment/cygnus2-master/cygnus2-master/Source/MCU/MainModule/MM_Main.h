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
*  File         : MM_Main.h
*
*  Description  : MM_Main header file
*
*************************************************************************************/

#ifndef MM_MAIN_H
#define MM_MAIN_H


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/



/* Public Structures
*************************************************************************************/ 
extern const char sw_version_nbr[ ];
extern const char sw_version_date[ ];
extern uint32_t rbu_pp_mode_request;

/* Public Enumerations
*************************************************************************************/


/* Public Functions Prototypes
*************************************************************************************/
void app_main (void);


/* Public Constants
*************************************************************************************/
#define VERSION_NUMBER_STRING    "03.01.22"     // 03.01.22.01	Alpha 01 Release
#define VERSION_DATE_STRING      "12/10/24"

#define RBU_RADIO_PP_MODE_REQ    0x5A5A5A5Au
#define SW_RESET_FLAG_KEY        0xA5A5A5A5u

/* Macros
*************************************************************************************/



#endif // MM_MAIN_H
