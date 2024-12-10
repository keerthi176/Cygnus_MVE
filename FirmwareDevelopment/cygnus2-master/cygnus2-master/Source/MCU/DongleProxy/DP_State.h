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
*  File         : DP_State.h
*
*  Description  :
*
*************************************************************************************/

#ifndef	HDF_DP_STATE_H
#define	HDF_DP_STATE_H

#include	"DP_System.h"

	typedef	enum		// !!!! DEBUG !!!!
	{
		ENU_LED_OFF = 0,
		ENU_LED_RED,
		ENU_LED_GREEN,
		ENU_LED_BLUE,
		ENU_LED_YELLOW,
		ENU_LED_CYAN,
		ENU_LED_MAGENTA,
		ENU_LED_WHITE
		//
	} ENUM_LED_PRIME_COL;

	
	typedef	enum
	{
		ENU_PPU_INIT = 0,

		ENU_PPU_LISTENING_INIT,
		ENU_PPU_LISTENING_RUN,

		ENU_PPU_FIRMWARE_BROADCAST_UPDATE_INIT,
		ENU_PPU_FIRMWARE_BROADCAST_UPDATE_RUN,

		ENU_PPU_FIRMWARE_BROADCAST_PAGES_INIT,
		ENU_PPU_FIRMWARE_BROADCAST_PAGES_RUN,
														
		
		ENU_PPU_FIRMWARE_SEND_MISSING_PAGES_INIT,
		ENU_PPU_FIRMWARE_SEND_MISSING_PAGES_RUN,

		ENU_PPU_FIRMWARE_INCOMING_INIT,
		ENU_PPU_FIRMWARE_INCOMING_RUN,
		
		ENU_PPU_PING_INIT,
		ENU_PPU_PING_RUN,

		ENU_PPU_DEBUG_TEST_INIT,
		ENU_PPU_DEBUG_TEST_RUN,
	
		//
		ENU_PPU_DEV_TEST_INIT,
		ENU_PPU_DEV_TEST_RUN,
		
		ENU_PPU_IDLE
		//
	} ENUM_PPU_MODE;
	
	
	typedef	struct
	{
		ENUM_PPU_MODE	enuMode;
		uint32_t			iTicker;
		uint32_t			uiFlashPage;
		
	} _xSystemPPU;
	
	_xSystemPPU	xSystemPPU;

	
	void	DP_StatePPU									( void );
	void	DP_StateLED									( ENUM_LED_PRIME_COL enuColour );
	void	DP_Protocol_BradcastUpdateOTA_TEST	( void );


#endif	// #ifndef	HDF_DP_STATE_H


/***************************************************************************************************
 ***************************************************************************************************
 **	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!
 ***************************************************************************************************
 **************************************************************************************************/
