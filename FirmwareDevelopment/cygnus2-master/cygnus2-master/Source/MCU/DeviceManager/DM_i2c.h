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
*  Description  : Header for the I2C interface
*
*************************************************************************************/

#ifndef DM_I2C_H
#define DM_I2C_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "CO_ErrorCode.h"

/* User Include Files
*************************************************************************************/



/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/
ErrorCode_t DM_I2C_Initialise(void);
ErrorCode_t DM_I2C_Startup(const uint32_t i2c_address);
ErrorCode_t DM_I2C_ShutDown(void);
ErrorCode_t DM_I2C_Read( const uint8_t device_addr, const uint8_t register_addr, const uint16_t data_length, const uint8_t* rx_buffer, const uint16_t rx_buffer_size );
ErrorCode_t DM_I2C_Write( const uint8_t device_addr, const uint8_t register_addr, const uint16_t data_length, const uint8_t* data_buffer );
void DM_I2C_HandleError(uint32_t hal_error);
bool DM_I2C_IsInitialised(void);

#ifdef WIN32
I2C_HandleTypeDef* GetI2CHandle(void);
#endif

/* Public Constants
*************************************************************************************/
#define I2C_CMD_WRITE 0x00010000U
#define I2C_TIMEOUT_MILLISECONDS 100

#define I2C_RBU_ADDRESS 0x12
#define I2C_SVI_MAX_REGISTER 256
//#define I2C_TIMING_REGISTER_VALUE 0x00303D5B
#define I2C_TIMING_REGISTER_VALUE 0x30425A5A
//#define I2C_TIMING_REGISTER_VALUE 0x00308BFF
#define I2C_WRITE_DIR 0
#define I2C_READ_DIR 1

/* Macros
*************************************************************************************/



#endif // DM_I2C_H
