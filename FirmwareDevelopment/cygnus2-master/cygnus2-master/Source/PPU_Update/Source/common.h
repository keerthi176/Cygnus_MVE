/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2019 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : common.h
*
*  Description  : Header for common definitions
*
*************************************************************************************/

#ifndef UPD_COMMON_H
#define UPD_COMMON_H
#include <stdint.h>
#include <stdbool.h>

#define VERSION_NUMBER_STRING    "00.00.01"
#define VERSION_DATE_STRING      "22/11/21"
#define RBU_SKIP_PP_MODE_REQ     0xAA55AA55u
#define RBU_FORCE_PP_MODE_REQ    0x55AA55AAu
#define SW_RESET_FLAG_KEY        0xA5A5A5A5u
#define APP_ADDRESS ( 0x08010000 )

void Error_Handler(char* error_msg);

#endif //UPD_COMMON_H
