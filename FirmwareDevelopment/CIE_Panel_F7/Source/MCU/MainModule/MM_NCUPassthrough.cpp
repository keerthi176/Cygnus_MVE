/***************************************************************************
* File name: MM_NCUPassthrough.cpp
* Project name: CIE_Panel
*
* Copyright 2018 Bull Products Ltd as an unpublished work.
* All Rights Reserved.
*
* The information contained herein is confidential property of Bull
* Products Ltd. The use, copying, transfer or disclosure of such
* information is prohibited except by express written agreement with Bull
* Products Ltd.
*
* First written on 18/05/18 by Matt Ault
*
* File History:
* Change History:
* Rev Date Description Author
* 1.0 18/05/18 Initial File
*
* Description:
* Main application
*
**************************************************************************/


/*************************************************************************/
/*								    	NOTE                                       */
/*************************************************************************/

// Horrible hack to pass thtough USBSerial <-> UART
// Rather than re-write the UART layer to allow for byte re-direction to
// different modules, the direction split is done in the IRQ handler.
// Unfortunately this means 