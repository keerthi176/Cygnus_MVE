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
*  File         : serial_if.h
*
*  Description  : Header file for the serial comms of the bootloader
*
*************************************************************************************/

#ifndef SERIAL_IF_H
#define SERIAL_IF_H


/* System Include files
*******************************************************************************/
#include <stdint.h>


/* User Include files
*******************************************************************************/


/* Public Structures
*******************************************************************************/


/* Public Enumerations
*******************************************************************************/ 


/* Public functions prototypes
*******************************************************************************/
void serial_init(void);
void serial_deinit(void);
HAL_StatusTypeDef Serial_PutString(const uint8_t *const p_string);
HAL_StatusTypeDef Serial_PutByte(const uint8_t param);
HAL_StatusTypeDef serial_receive_data(const uint8_t *p_data, const uint16_t size, const uint32_t timeout_ms);
HAL_StatusTypeDef serial_send_data(const uint8_t *const p_data, const uint16_t size, const uint32_t timeout_ms);
HAL_StatusTypeDef serial_request_new_packet(const uint8_t request_key, const uint8_t *p_data, const uint16_t max_packet_size, uint16_t *const received_packet_size, const uint32_t timeout_ms);


HAL_StatusTypeDef debug_out( const char* format, ... );

/* Public Constants
*******************************************************************************/


/* Macros
*******************************************************************************/


#endif // SERIAL_IF_H
