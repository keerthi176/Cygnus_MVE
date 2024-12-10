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
*  File       	: DP_Debug.h
*
*  Description	: Takes care & processes communiucation protocol data for both RX & TX
*
*************************************************************************************/

#ifndef	HDF_DP_DEBUG_H
#define	HDF_DP_DEBUG_H

#include	"DP_System.h"






#define	HDF_DX_PPU_PROGRAMMER_DEBUG_MODE
#define	HDF_DX_PPU_PROGRAMMER






	// Mutually exclusive bit-patterns.
	typedef	enum
	{
		ENU_DEBUG_ERROR	=	0x01,
		ENU_DEBUG_LORA		=	0x02,
		ENU_DEBUG_TCP		=	0x04,
		ENU_DEBUG_UDP		=	0x08,
		ENU_DEBUG_I2C		=	0x10,
		ENU_DEBUG_WIFII	=	0x20,
		ENU_DEBUG_SPI		=	0x40,
		ENU_DEBUG_TEST		=	0x80

	} ENUM_DEBUG_MASK;

	typedef	enum
	{
		ENU_UART1 = 1,
		ENU_UART2,
		ENU_UART3,
		ENU_UART4
		//
	} ENUM_DEBUG_UART;

	


	
	void	m_Tester					( void );


	void	DP_Debug_Enable		( ENUM_DEBUG_MASK enuMask );
	void	DP_Debug_Disable		( ENUM_DEBUG_MASK enuMask );

	void	DP_Debug_Print			( ENUM_DEBUG_MASK enuMask, int8_t *pszString );
	void	DP_Debug_PrintCR		( ENUM_DEBUG_MASK enuMask, int8_t *pszString );


#endif	// #ifdef	HDF_DP_DEBUG_H

/***************************************************************************************************
 ***************************************************************************************************
 **	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!
 ***************************************************************************************************
 **************************************************************************************************/
