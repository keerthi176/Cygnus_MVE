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
*  File         : simple.h
*
*  Description  : Header for ymodem simple file transfer
*
*************************************************************************************/

#ifndef BOOTLOADER_SIMPLE_H
#define BOOTLOADER_SIMPLE_H
#include "stdint.h"
#include "ymodem.h"


/* Public functions ---------------------------------------------------------*/
COM_StatusTypeDef Simple_Receive( uint32_t *p_size, uint32_t bank );



#endif //BOOTLOADER_SIMPLE_H
