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
*  File         : BLF_AppUpdater.h
*
*  Description  : brief description
*
*************************************************************************************/

#ifndef BLF_APP_UPDATER_H
#define BLF_APP_UPDATER_H


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/



/* Public Structures
*************************************************************************************/ 


/* Public Enumerations
*************************************************************************************/
typedef enum
{
   REPROG_NO_REQ_E = 0,
   REPROG_RESCUE_MODE_E,
   REPROG_STANDARD_E
}reprogramming_mode_t;

typedef enum
{
   APP_UPDATE_SUCCESS_E = 0,
   APP_UPDATE_FAIL_E
}blf_app_update_result_t;

typedef enum
{
   VALID_APPLICATION_PRESENT_E = 0u,
   NO_VALID_APPLICATION_E
}blf_application_status_t;

/* Public Functions Prototypes
*************************************************************************************/
void BLF_LoaderUpdater(reprogramming_mode_t reprogramming_mode);
blf_application_status_t check_other_bank_app_validity(uint32_t program_size, uint32_t bank);

/* Public Constants
*************************************************************************************/


/* Macros
*************************************************************************************/



#endif
