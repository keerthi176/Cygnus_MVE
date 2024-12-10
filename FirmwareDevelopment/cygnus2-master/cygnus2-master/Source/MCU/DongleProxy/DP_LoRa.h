/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2018 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : DP_LoRa.h
*
*  Description  :
*
*************************************************************************************/

#ifndef	HDF_DP_LORA_H
#define	HDF_DP_LORA_H

#include	"DP_System.h"

//#include "CO_Defines.h"
//#include "CO_SerialDebug.h"

	 
	/*
	 * Cygnus LoRa Channels.
	 *
	 * 01 - 865150000
	 * 02 - 865450000
	 * 03 - 865750000
	 * 04 - 866050000
	 * 05 - 866350000
	 * 06 - 866650000
	 * 07 - 866950000
	 * 08 - 867250000
	 * 09 - 867550000
	 * 10 - 867850000
	 *
	 * 11 - 868150000 - PPU
	 */


	// 
	uint16_t		DP_LoRa_ParsePacket	( void );
	uint16_t		DP_LoRa_GetRSSI		( void );
	uint16_t		DP_LoRa_ReadBytes		( int8_t *paucBuffer, uint16_t iLength );
	uint16_t		DP_LoRa_Transmit		( int8_t *paucBuffer, uint16_t iLength );
	bool			DP_LoRa_Available		( void );


#endif	// #ifdef	HDF_DP_LORA_H


/***************************************************************************************************
 ***************************************************************************************************
 **	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!
 ***************************************************************************************************
 **************************************************************************************************/
