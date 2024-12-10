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
*  File         : DP_System.h
*
*  Description  :
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#ifndef	HDF_SYSTEM_H
#define	HDF_SYSTEM_H




#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>





// HDF_DEBUG_OUTPUT_UART1	// Enable RS232 debug serial output if defined (UART #1)
// HDF_DEBUG_OUTPUT_UART2	// Enable RS232 debug serial output if defined (UART #2)
// HDF_DEBUG_OUTPUT_UART3	// Enable RS232 debug serial output if defined (UART #3)
// HDF_DEBUG_OUTPUT_UART4	// Enable RS232 debug serial output if defined (UART #4)


#define	HDF_PPU_SYSTEM_DEBUG_MODE		true

#define	HDF_SYSTEM_PPU_ID					0x8002					// 0x8000 & above = PPU/Dongle ID

#define	HDF_SYSTEM_PPU_ID_MIN			0x8000					//
#define	HDF_SYSTEM_PPU_ID_MAX			0xa000					//

#define	HDF_SYSTEM_RBU_ID_MIN			0							//
#define	HDF_SYSTEM_RBU_ID_MAX			511						//

#define	HDF_SYSTEM_PPU_NAME				"CYGUS DONGLE PROXY"	//
#define	HDF_SYSTEM_PPU_VERSION			"X0.1"					// Version with an 'X' == Internal Development Build (doesn't go out).
#define	HDF_SYSTEM_PPU_SERIAL_NUMBER	0x1248ABCD				//



	// Extra debug functionality if defined.
	// Extra debug functionality if defined.
	// Extra debug functionality if defined.
	// Extra debug functionality if defined.


#define	HDF_DONGLE_DEBUG_EXTRA

	// AT_MODE_EXIT_TIMEOUT_US - Extended


	// Extra debug functionality if defined.
	// Extra debug functionality if defined.
	// Extra debug functionality if defined.
	// Extra debug functionality if defined.




#endif	// #ifndef	HDF_SYSTEM_H

/***************************************************************************************************
 ***************************************************************************************************
 **	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!	EOF!
 ***************************************************************************************************
 **************************************************************************************************/
