
/***************************************************************************
* File name: DM_TimerHandler.h
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
* First written on 18/05/19 by Matt Ault
*
* File History:
* Change History:
* Rev Date Description Author
* 1.0 18/05/18 Initial File
*
* Description:
* Timer Handling
*
************************************************************************/

#ifndef _DM_TIMER_HANDLER_
#define _DM_TIMER_HANDLER_


/* System Include Files
**************************************************************************/
#include <stm32f7xx_hal.h>
#include <stm32f7xx_hal_tim.h>


struct TimerHandler
{
	TimerHandler( TIM_HandleTypeDef* uh );
	
	virtual void TimerPeriodElapsed( TIM_HandleTypeDef* uh );
 
	virtual void TimerPulseFinished( TIM_HandleTypeDef* uh );
};

#endif
