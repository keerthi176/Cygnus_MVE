/*************************************************************************************
*  CONFIDENTIAL between PA and Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2018 PA Consulting Group. All rights reserved.
*  Cambridge Technology Centre
*  Melbourn Royston
*  Herts SG8 6DP UK
*  Tel: <44> 1763 261222
*
**************************************************************************************
*  File         : DM_i2c.h
*
*  Description  : Header file for the Sound and Visual Indicator (SVI)
*                 interface
*
*************************************************************************************/

#ifndef DM_SVI_H
#define DM_SVI_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "CO_Message.h"
#include "CO_ErrorCode.h"

/* User Include Files
*************************************************************************************/



/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/
typedef enum
{
   DM_SVI_TYPE_E,                   // 0
   DM_SVI_ENABLE_E,                 // 1
   DM_SVI_TEST_MODE_E,              // 2
   DM_SVI_FLASH_RATE_E,             // 3
   DM_SVI_TONE_SELECT_E,            // 4
   DM_SVI_TONE_ATTEN_E,             // 5
   DM_SVI_FAULT_STATUS_E,           // 6
   DM_SVI_FW_MAJOR_LS_E,            // 7
   DM_SVI_FW_MAJOR_MS_E,            // 8
   DM_SVI_FW_MINOR_LS_E,            // 9
   DM_SVI_FW_MINOR_MS_E,            // 10
   DM_SVI_FW_MAINT_REV_LS_E,        // 11
   DM_SVI_FW_MAINT_REV_MS_E,        // 12
   DM_SVI_FW_DATE_DAY_LS_E,         // 13
   DM_SVI_FW_DATE_DAY_MS_E,         // 14
   DM_SVI_FW_DATE_MONTH_LS_E,       // 15
   DM_SVI_FW_DATE_MONTH_MS_E,       // 16
   DM_SVI_FW_DATE_YEAR_LS_E,        // 17
   DM_SVI_FW_DATE_YEAR_MS_E,        // 18
   DM_SVI_SERIAL_NUMBER_0_E,        // 19
   DM_SVI_SERIAL_NUMBER_1_E,        // 20
   DM_SVI_SERIAL_NUMBER_2_E,        // 21
   DM_SVI_SERIAL_NUMBER_3_E,        // 22
   DM_SVI_PARAMETER_TYPE_MAX_E,     // 23
   DM_SVI_POWER_LOCK_E              // 24
} DM_SviParameterType_t;



enum
{
   DM_SVI_OFF_E,
   DM_SVI_ON_E
};

typedef enum 
{
   DM_SVI_STATE_INITIALISING_E,
   DM_SVI_STATE_BIT_TEST_E,
   DM_SVI_STATE_BIT_FAIL_E,
   DM_SVI_STATE_IDLE_E,
   DM_SVI_STATE_ACTIVE_E
} SviState_t;

/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t DM_SVI_Initialise(void);
ErrorCode_t DM_SVI_Deinitialise(void);
void DM_SVI_CheckForTimeout(void);
ErrorCode_t DM_SVI_GetRegister( const uint8_t command, uint8_t* registerAddress );
ErrorCode_t DM_SVI_ReadRegister( const uint8_t registerAddress, uint8_t* pResponse );
ErrorCode_t DM_SVI_WriteRegister( const uint8_t registerAddress, const uint8_t value );
bool DM_SVI_IsEnabled(void);

/* Public utility Function Prototypes
*************************************************************************************/
ErrorCode_t DM_SVI_ReadFirmwareVersion( uint32_t* version );
ErrorCode_t DM_SVI_SviGetSerialNumber(uint32_t *SerialNumber);
ErrorCode_t DM_SVI_BitTest(void);
bool DM_SVI_SviIsSerialNumberValid(void);
void DM_SVI_PeriodicCheck(void);
bool DM_SVI_SetToneSelection(const DM_ToneSelectionType_t tone);
bool DM_SVI_SetFlashRate(const FlashRate_t flashRate);
bool DM_SVI_SetSounder(const uint8_t sounder_state);
bool DM_SVI_SetVisualIndicator(const uint8_t vi_state);
bool DM_SVI_SetToneAttenuation(const uint8_t soundLevel);

/* Public Constants
*************************************************************************************/
#define SOUND_LEVEL_MAX_E 3

#define I2C_SVI_SINGLE_BYTE 1U
#define I2C_SVI_FIRMWARE_VERSION_LENGTH 6U
#define I2C_SVI_FIRMWARE_DATE_LENGTH 6U
#define I2C_SVI_SERIAL_NUMBER_LENGTH 4U

#define I2C_SVI_ADDRESS 0x10

#define DM_I2C_SVI_REG_TYPE 0x00
#define DM_I2C_SVI_REG_ENABLE  0x10
#define DM_I2C_SVI_REG_TEST_MODE 0x20
#define DM_I2C_SVI_REG_FLASH_RATE 0x40
#define DM_I2C_SVI_REG_TONE_SELECT 0x80
#define DM_I2C_SVI_REG_TONE_ATTEN 0x90
#define DM_I2C_SVI_REG_FAULT_STATUS 0xC0
#define DM_I2C_SVI_REG_FW_MAJOR_LS   0xE0
#define DM_I2C_SVI_REG_FW_MAJOR_MS   0xE1
#define DM_I2C_SVI_REG_FW_MINOR_LS   0xE2
#define DM_I2C_SVI_REG_FW_MINOR_MS   0xE3
#define DM_I2C_SVI_REG_FW_MAINT_REV_LS 0xE4
#define DM_I2C_SVI_REG_FW_MAINT_REV_MS 0xE5
#define DM_I2C_SVI_REG_FW_DATE_DAY_LS   0xE6
#define DM_I2C_SVI_REG_FW_DATE_DAY_MS   0xE7
#define DM_I2C_SVI_REG_FW_DATE_MONTH_LS   0xE8
#define DM_I2C_SVI_REG_FW_DATE_MONTH_MS   0xE9
#define DM_I2C_SVI_REG_FW_DATE_YEAR_LS   0xEA
#define DM_I2C_SVI_REG_FW_DATE_YEAR_MS   0xEB
#define DM_I2C_SVI_REG_SERIAL_NUMBER_0  0xFC
#define DM_I2C_SVI_REG_SERIAL_NUMBER_1  0xFD
#define DM_I2C_SVI_REG_SERIAL_NUMBER_2  0xFE
#define DM_I2C_SVI_REG_SERIAL_NUMBER_3  0xFF



/* Macros
*************************************************************************************/



#endif // DM_SVI_H
