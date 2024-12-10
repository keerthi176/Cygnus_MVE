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
*  File         : DP_Enum.h
*
*  Description  :
*
*************************************************************************************/
#ifndef	HDF_ENUM_H
#define	HDF_ENUM_H


// Type types of transmissions.
	enum	ENUM_KEYWORD_TYPE
	{
		ENU_KW_RS232 = 1,
		ENU_KW_LORA,
		ENU_KW_SOCKET,
		ENU_KW_BLUETOOTH,
		ENU_KW_CANBUS,
		ENU_KW_USB,
		ENU_KW_RS485,
		ENU_KW_NFC,
		ENU_KW_LIN
		//
	};

	

	enum	ENUM_DEBUG_UART
	{
		ENU_UART1 = 1,
		ENU_UART2,
		ENU_UART3,
		ENU_UART4
		//
	};

		// Mutually exclusive bit-patterns.
	enum	ENUM_DEBUG_MASK
	{
		ENU_DEBUG_ERROR	=	0x01,
		ENU_DEBUG_LORA		=	0x02,
		ENU_DEBUG_TCP		=	0x04,
		ENU_DEBUG_UDP		=	0x08,
		ENU_DEBUG_I2C		=	0x10,
		ENU_DEBUG_WIFII	=	0x20,
		ENU_DEBUG_SPI		=	0x40,
		ENU_DEBUG_TEST		=	0x80
	};

	
#endif // #ifndef	HDF_ENUM_H
	
/***************************************************************************************************
 ***************************************************************************************************
 **	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!
 ***************************************************************************************************
 **************************************************************************************************/
