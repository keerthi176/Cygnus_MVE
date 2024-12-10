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
*  File         : DP_Debug.cpp
*
*  Description  :
*
*************************************************************************************/

#include	"DP_Debug.h"




/**************************************************************************************************
 **************************************************************************************************
 ** @brief	:
 ** @param	:
 ** @return	:
 **************************************************************************************************
 **************************************************************************************************/
CDP_Debug::CDP_Debug()
{
	//ulMask = 0;			// No debug print on initialisation.

	ulMask = 0xffffffff;	// ALL debug output - TEMP!!!!!!
}




/**************************************************************************************************
**************************************************************************************************
** Member	:
** In			:
** Out		:
** About		:
**************************************************************************************************
*************************************************************************************************/
CDP_Debug::~CDP_Debug()
{
}




/**************************************************************************************************
 **************************************************************************************************
 ** @member	: m_Enable
 ** @brief	:
 ** @param	:
 ** @return	:
 **************************************************************************************************
 **************************************************************************************************/
void CDP_Debug::m_Enable		( ENUM_DEBUG_MASK enuMask )
{
	ulMask |= enuMask;
}





/**************************************************************************************************
 **************************************************************************************************
 ** @member	: m_Disable
 ** @brief	:
 ** @param	:
 ** @return	:
 **************************************************************************************************
 **************************************************************************************************/
void CDP_Debug::m_Disable		( ENUM_DEBUG_MASK enuMask )
{
	ulMask &= ~enuMask;
}




/**************************************************************************************************
 **************************************************************************************************
 ** @member	: m_Print
 ** @brief	: Send zero-terminated string through the UART ( No Carriage Return )
 ** @param	:
 ** @return	:
 **************************************************************************************************
 **************************************************************************************************/
void CDP_Debug::m_Print	( ENUM_DEBUG_MASK enuMask, int8_t *pszString )
{
#ifdef HDF_DEBUG_UART_OUTPUT
	//CSoxxUDP	*pobSoxxUDP	= (CSoxxUDP*) lpvSoxxUDP;

	// ?? Has the mask bit been set. ??
	if ( enuMask & ulMask )
	{
		Serial.print( pszString );

		//pobSoxxUDP->m_TX_RAW_Debug( pszString, strlen(pszString) );
	}
#endif
}




/**************************************************************************************************
 **************************************************************************************************
 ** @member	: m_PrintCR
 ** @brief	: Send zero-terminated string through the UART ( With Carriage Return )
 ** @param	:
 ** @return	:
 **************************************************************************************************
 **************************************************************************************************/
void CDP_Debug::m_PrintCR	( ENUM_DEBUG_MASK enuMask, int8_t *pszString )
{
#ifdef HDF_DEBUG_UART_OUTPUT
	//CSoxxUDP	*pobSoxxUDP	= (CSoxxUDP*) lpvSoxxUDP;

	// ?? Has the mask bit been set. ??
	if ( enuMask & ulMask )
	{
		Serial.println( pszString );

		//pobSoxxUDP->m_TX_RAW_Debug( pszString, strlen(pszString) );
	}
#endif
}





/**************************************************************************************************
 **************************************************************************************************
 ** @member	:
 ** @brief	:
 ** @param	:
 ** @return	:
 **************************************************************************************************
 **************************************************************************************************/
void CDP_Debug::m_Tester	( void )
{
int16_t	sTester;
	
	for ( sTester = 0; sTester < 10; sTester++ )
	{
		;
	}
}

/***************************************************************************************************
 ***************************************************************************************************
 **	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!
 ***************************************************************************************************
 **************************************************************************************************/