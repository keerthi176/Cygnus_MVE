/*
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
*  File         : DP_State.cpp
*
*  Description  :
*
*************************************************************************************/

#include	"DP_Protocol.h"
#include	"DP_State.h"
#include	"board.h"


	extern	void	DP_Protocol_BlipTest			( uint32_t uiBlip );
/*
	extern 	void	DP_Protocol_SendFlashOTA 	( uint16_t uiPageTotal, uint16_t uiPageNumber, uint16_t uiPageSize,
																uint8_t ucBlockTotal, uint8_t ucBlockCurrent, uint8_t ucBlockSize,
																uint8_t *paucPageData );
*/
	void	DP_TransferTest							( void );
	void	DP_SetLED									( ENUM_LED_PRIME_COL enuColour );
	


/**************************************************************************************************
**************************************************************************************************
** 			: DP_StatePPU
** 			:
** 			:
** 			:
**************************************************************************************************
*************************************************************************************************/
void	DP_StatePPU	( void )
{	
static	int16_t	iState = 0, iStateLast = -1;
static	int32_t	iCounter = 0;	// !!!! DEBUG !!!!
//`ENUM_LED_PRIME_COL	aEnuLedCol[] = { ENU_LED_RED, ENU_LED_WHITE, ENU_LED_BLUE, ENU_LED_OFF };

	
	switch ( iState )
	{
		case ENU_PPU_INIT:
			if ( iState != iStateLast )
			{
				iStateLast = iState;
				CO_PRINT_A_0( DBG_DEV, "- RBX STATEX INIT\r\n" );
			}
			xSystemPPU.iTicker = 0;
			xSystemPPU.uiFlashPage = 0;

			//xSystemPPU.enuMode = ENU_PPU_DEV_TEST_INIT;
			iState = ENU_PPU_DEV_TEST_INIT;
			break;

		
		
		case ENU_PPU_DEV_TEST_INIT:
			if ( iState != iStateLast )
			{
				iStateLast = iState;
				CO_PRINT_A_0( DBG_DEV, "- RBX PPU DEV TEST INIT\r\n" );
			}
			iState = ENU_PPU_DEV_TEST_RUN;
			break;

		
		case ENU_PPU_DEV_TEST_RUN:
			if ( iState != iStateLast )
			{
				iStateLast = iState;
				CO_PRINT_A_0( DBG_DEV, "- RBX PPU DEV TEST RUN\r\n" );
			}
			//CO_PRINT_A_0( DBG_DEV, "- STATEX: TEST RUNNING\r\n" );
			//DP_TransferTest();

			//DP_SetLED( aEnuLedCol[iCounter & 0x03] );

			switch ( iCounter & 0x03 )
			{
				case 0:
					DP_SetLED( ENU_LED_RED );
					break;
				case 1:
					DP_SetLED( ENU_LED_GREEN );
					break;
				case 2:
					DP_SetLED( ENU_LED_BLUE );
					break;
				case 3:
					DP_SetLED( ENU_LED_YELLOW );
					break;
			}

			DP_Protocol_BlipTest( iCounter );
			DP_SetLED( ENU_LED_OFF);

		
			break;


		case ENU_PPU_LISTENING_INIT:
			if ( iState != iStateLast )
			{
				iStateLast = iState;
				CO_PRINT_A_0( DBG_DEV, "- RBX PPU DEV LISTENING\r\n" );
			}
			break;

		case ENU_PPU_LISTENING_RUN:
			if ( iState != iStateLast )
			{
				iStateLast = iState;
				CO_PRINT_A_0( DBG_DEV, "- RBX PPU DEV TEST RUN\r\n" );
			}
			//DP_Protocol_KeywordRX( ENU_KW_LORA );
			break;
		
		
		case ENU_PPU_FIRMWARE_BROADCAST_UPDATE_INIT:
			if ( iState != iStateLast )
			{
				iStateLast = iState;
				CO_PRINT_A_0( DBG_DEV, "- FIRMWARE BROADCAST\r\n" );
			}
			break;

		case ENU_PPU_FIRMWARE_BROADCAST_UPDATE_RUN:
			if ( iState != iStateLast )
			{
				iStateLast = iState;
				CO_PRINT_A_0( DBG_DEV, "- RBX PPU DEV TEST INIT\r\n" );
			}
				//DP_Protocol_BradcastUpdateOTA_TEST();
			break;

		
		case ENU_PPU_FIRMWARE_BROADCAST_PAGES_INIT:
			if ( iState != iStateLast )
			{
				iStateLast = iState;
				CO_PRINT_A_0( DBG_DEV, "- RBX BROADCAST PAGES\r\n" );
			}
			break;

		
		case ENU_PPU_FIRMWARE_BROADCAST_PAGES_RUN:
			if ( iState != iStateLast )
			{
				iStateLast = iState;
				CO_PRINT_A_0( DBG_DEV, "- RBX PPU DEV TEST INIT\r\n" );
			}
			//DP_Protocol_SendPagesOTA();
			break;

		
		case ENU_PPU_FIRMWARE_SEND_MISSING_PAGES_INIT:
			break;
		
		case ENU_PPU_FIRMWARE_SEND_MISSING_PAGES_RUN:
				//DP_Protocol_ReportMissingPacketsOTA();
			break;

		case ENU_PPU_FIRMWARE_INCOMING_INIT:
			if ( iState != iStateLast )
			{
				iStateLast = iState;
				CO_PRINT_A_0( DBG_DEV, "- RBX FIRMWARE INCOMING INIT\r\n" );
			}
			//
			break;
		
		case ENU_PPU_FIRMWARE_INCOMING_RUN:
			if ( iState != iStateLast )
			{
				iStateLast = iState;
				CO_PRINT_A_0( DBG_DEV, "- RBX FIRMWARE INCOMING RUN\r\n" );
			}
			//
			break;
		

		case ENU_PPU_PING_INIT:
			if ( iState != iStateLast )
			{
				iStateLast = iState;
				CO_PRINT_A_0( DBG_DEV, "- RBX PING INIT\r\n" );
			}
			break;


		case ENU_PPU_PING_RUN:
			if ( iState != iStateLast )
			{
				iStateLast = iState;
				CO_PRINT_A_0( DBG_DEV, "- RBX PING RUN\r\n" );
			}
			//DP_Protocol_Ping();
			break;

			
		case ENU_PPU_DEBUG_TEST_INIT:
			iState = ENU_PPU_DEBUG_TEST_RUN;
			break;
		
		
		case ENU_PPU_DEBUG_TEST_RUN:
			if ( iState != iStateLast )
			{
				iStateLast = iState;
				CO_PRINT_A_0( DBG_DEV, "- RBX DEBUG TEST RUN\r\n" );
			}
			xSystemPPU.iTicker = 0;
			xSystemPPU.uiFlashPage = 0;

			//xSystemPPU.enuMode = ENU_PPU_DEV_TEST_INIT;
			iState = ENU_PPU_DEV_TEST_INIT;
			break;
		
		
		
			
		default:
			break;
	}

	iCounter++;
}




/**************************************************************************************************
***************************************************************************************************
** Function	: DP_TransferTest
** In			:
** Out		:
** About		:
**************************************************************************************************
**************************************************************************************************/
void	DP_TransferTest	( void )
{
static	uint8_t	i = 0;

	switch( i++ & 0x07 )
	{
		case 0:
			DP_SetLED( ENU_LED_RED );
			break;
		case 1:
			DP_SetLED( ENU_LED_GREEN );
			break;
		case 2:
			DP_SetLED( ENU_LED_BLUE );
			break;		
		case 3:
			DP_SetLED( ENU_LED_YELLOW );
			break;
		case 4:
			DP_SetLED( ENU_LED_CYAN );
			break;
		case 5:
			DP_SetLED( ENU_LED_MAGENTA );
			break;
		case 6:
			DP_SetLED( ENU_LED_WHITE );
			break;

		default:
			DP_SetLED( ENU_LED_OFF );
			break;
	}
}




/**************************************************************************************************
***************************************************************************************************
** Function	: DP_SetLED
** In			:
** Out		:
** About		:
**************************************************************************************************
**************************************************************************************************/
void	DP_SetLED( ENUM_LED_PRIME_COL enuColour )
{
	switch ( enuColour )
	{
		case ENU_LED_OFF:
			GpioWrite( &StatusLedRed, 0 );
			GpioWrite( &StatusLedGreen, 0 );
			GpioWrite( &StatusLedBlue, 0 );
			break;

		case ENU_LED_RED:
			GpioWrite( &StatusLedRed, 1 );
			GpioWrite( &StatusLedGreen, 0 );
			GpioWrite( &StatusLedBlue, 0 );
			break;

		case ENU_LED_GREEN:
			GpioWrite( &StatusLedRed, 0 );
			GpioWrite( &StatusLedGreen, 1 );
			GpioWrite( &StatusLedBlue, 0 );
			break;

		case ENU_LED_BLUE:
			GpioWrite( &StatusLedRed, 0 );
			GpioWrite( &StatusLedGreen, 0 );
			GpioWrite( &StatusLedBlue, 1 );
			break;
	
		case ENU_LED_YELLOW:
			GpioWrite( &StatusLedRed, 1 );
			GpioWrite( &StatusLedGreen, 1 );
			GpioWrite( &StatusLedBlue, 0 );
			break;

		case ENU_LED_CYAN:
			GpioWrite( &StatusLedRed, 0 );
			GpioWrite( &StatusLedGreen, 1 );
			GpioWrite( &StatusLedBlue, 1 );
			break;

		case ENU_LED_MAGENTA:
			GpioWrite( &StatusLedRed, 1 );
			GpioWrite( &StatusLedGreen, 0 );
			GpioWrite( &StatusLedBlue, 1 );
			break;

		case ENU_LED_WHITE:
			GpioWrite( &StatusLedRed, 1 );
			GpioWrite( &StatusLedGreen, 1 );
			GpioWrite( &StatusLedBlue, 1 );
			break;
																	
		default:
			GpioWrite( &StatusLedRed, 0 );
			GpioWrite( &StatusLedGreen, 0 );
			GpioWrite( &StatusLedBlue, 0 );
			break;
	}
}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_BradcastUpdateOTA_TEST
 **
 ** @brief		: (Still in development rtesting mode)
 **
 ** @param		: VOID
 **
 ** @Out			: VOID
 **************************************************************************************************
 *************************************************************************************************/
void	DP_Protocol_BradcastUpdateOTA_TEST	( void )
{
#if 0
	
uint8_t	aucTestData[256];
uint32_t	uiBinarySize = 398921;

uint16_t	uiPageTotal = 4096;	// !!!! DEBUG TEST VALUE !!!!
uint16_t	uiPageSize = 2048;
uint16_t	uiPageNumber = 0;
														 
uint8_t	ucBlockTotal;			// Calculated.
uint8_t	ucBlockSize = 64;
uint8_t	ucBlockCurrent = 0;

uint16_t	uiLoopTotal, uiLoopPage, uiLoopBlock;

	
	
	// Test values for dev & debug...
	// Test values for dev & debug...
	_xFlashMemory	xFlashMemory =
	{
		//.uiDataSizeTotal	= 398921, 		// !!!! DEBUG TEST VALUE !!!!
		.uiDataSizeTotal	= 8192, 		// !!!! DEBUG TEST VALUE !!!!

		.usPagesTotal 		= 4,
		.usPageNumber 		= 0,
		.usPageSize			= 2048,
		
		.ucBlockSize	= 16,
		.ucBlockNumber		= 0
	};
	// Test values for dev & debug...
	// Test values for dev & debug...
#endif


	

#if 0
	for ( uiLoopTotal = 0; uiLoopTotal < uiPageTotal; uiLoopTotal += ucBlockSize )
	{
		for( uiLoopPage = 0; uiLoopPage < uiPageSize; uiLoopPage++ )
		{
/*			
			DP_Protocol_SendFlashOTA(	uiPageTotal,
												uiPageNumber,
												uiPageSize,
												ucBlockTotal,
												ucBlockCurrent,
												ucBlockSize,
												aucTestData
											);
*/
			;
		} // for( uiLoopPage = 0; uiLoopPage < uiPageSize; uiLoopPage++ )
		
	} // for ( uiLoopTotal = 0; uiLoopTotal < uiPageTotal; uiLoopTotal += ucBlockSize )
#endif

}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_BroadcastUpdatePage
 **
 ** @brief		: (Still in development rtesting mode)
 **
 ** @param		:
 **
 ** @Out			: VOID
 **************************************************************************************************
 *************************************************************************************************/
void DP_Protocol_BroadcastUpdatePage ( uint16_t uiPage )
{

}




/*!
 *************************************************************************************************
 **************************************************************************************************
 ** @function	: DP_Protocol_BroadcastUpdateBlock
 **
 ** @brief		:
 **
 ** @param		:
 **
 ** @Out			: VOID
 **************************************************************************************************
 *************************************************************************************************/
void DP_Protocol_BroadcastUpdateBlock ( uint16_t uiBlock )
{

}


/***************************************************************************************************
 ***************************************************************************************************
 **	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!
 ***************************************************************************************************
 **************************************************************************************************/
