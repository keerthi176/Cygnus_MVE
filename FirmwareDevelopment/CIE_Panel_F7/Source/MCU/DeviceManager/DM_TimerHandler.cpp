
/***************************************************************************
* File name: DM_TimerHandler.cpp
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



/* User Include Files
**************************************************************************/

#include "DM_Handlers.h"
#include "DM_TimerHandler.h"


/* Globals
**************************************************************************/

extern TIM_HandleTypeDef htim13;
extern TIM_HandleTypeDef htim14;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim8;


  

TimerHandler* tim_handler[ 15 ] ={0};


/*************************************************************************/
/**  \fn      PanelNetwork::PanelNetwork( )
***  \brief   Constructor for class
**************************************************************************/
 
TimerHandler::TimerHandler( TIM_HandleTypeDef* uh )
{
 	switch( (int)uh->Instance )
	{
		case TIM6_BASE : tim_handler[ 6] = this;	break;
		case TIM7_BASE : tim_handler[ 7] = this;	break;
		case TIM10_BASE : tim_handler[ 10] = this;	break;
		case TIM13_BASE : tim_handler[ 13] = this;	break;
		case TIM14_BASE : tim_handler[ 14] = this;	break;
		case TIM8_BASE : tim_handler[ 8] = this;	break;
	}	
 
};	
	


/*************************************************************************/
/**  \fn      TimerHandler::TimerPeriodElapsed( TIM_HandleTypeDef* uh )
***  \brief   Virtual class function
**************************************************************************/
 
void TimerHandler::TimerPeriodElapsed( TIM_HandleTypeDef* uh ) { }

 

/*************************************************************************/
/**  \fn      TimerHandler::PulseFinished( TIM_HandleTypeDef* uh )
***  \brief   irtual class function
**************************************************************************/

void TimerHandler::TimerPulseFinished( TIM_HandleTypeDef* uh ) { }


extern "C" 
{
	void DM_TIM_IRQHandler( TIM_HandleTypeDef *htim, TimerHandler* h )
	{
	  /* Capture compare 1 event */
	  if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_CC1) != RESET)
	  {
		 if (__HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_CC1) != RESET)
		 {
			{
			  __HAL_TIM_CLEAR_IT(htim, TIM_IT_CC1);
			  htim->Channel = HAL_TIM_ACTIVE_CHANNEL_1;

			  /* Input capture event */
			  if ((htim->Instance->CCMR1 & TIM_CCMR1_CC1S) != 0x00U)
			  {
	//#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
	//          htim->IC_CaptureCallback(htim);
	//#else
	//          HAL_TIM_IC_CaptureCallback(htim);
	//#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
			  }
			  /* Output compare event */
			  else
			  {
	//#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
	//          htim->OC_DelayElapsedCallback(htim);
				 h->TimerPulseFinished(htim);
	//#else
	//          HAL_TIM_OC_DelayElapsedCallback(htim);
	//          HAL_TIM_PWM_PulseFinishedCallback(htim);
	//#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
			  }
			  htim->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
			}
		 }
	  }
	  /* Capture compare 2 event */
	  if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_CC2) != RESET)
	  {
		 if (__HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_CC2) != RESET)
		 {
			__HAL_TIM_CLEAR_IT(htim, TIM_IT_CC2);
			htim->Channel = HAL_TIM_ACTIVE_CHANNEL_2;
			/* Input capture event */
			if ((htim->Instance->CCMR1 & TIM_CCMR1_CC2S) != 0x00U)
			{
	//#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
	//        htim->IC_CaptureCallback(htim);
	//#else
	//        HAL_TIM_IC_CaptureCallback(htim);
	//#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
			}
			/* Output compare event */
			else
			{
	//#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
	//        htim->OC_DelayElapsedCallback(htim);
			  h->TimerPulseFinished(htim);
	//#else
	//        HAL_TIM_OC_DelayElapsedCallback(htim);
	//        HAL_TIM_PWM_PulseFinishedCallback(htim);
	//#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
			}
			htim->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
		 }
	  }
	  /* Capture compare 3 event */
	  if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_CC3) != RESET)
	  {
		 if (__HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_CC3) != RESET)
		 {
			__HAL_TIM_CLEAR_IT(htim, TIM_IT_CC3);
			htim->Channel = HAL_TIM_ACTIVE_CHANNEL_3;
			/* Input capture event */
			if ((htim->Instance->CCMR2 & TIM_CCMR2_CC3S) != 0x00U)
			{
	//#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
	//        htim->IC_CaptureCallback(htim);
	//#else
	//        HAL_TIM_IC_CaptureCallback(htim);
	//#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
			}
			/* Output compare event */
			else
			{
	//#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
	//        htim->OC_DelayElapsedCallback(htim);
				 h->TimerPulseFinished(htim);
	//#else
	//        HAL_TIM_OC_DelayElapsedCallback(htim);
	//        HAL_TIM_PWM_PulseFinishedCallback(htim);
	//#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
			}
			htim->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
		 }
	  }
	  /* Capture compare 4 event */
	  if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_CC4) != RESET)
	  {
		 if (__HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_CC4) != RESET)
		 {
			__HAL_TIM_CLEAR_IT(htim, TIM_IT_CC4);
			htim->Channel = HAL_TIM_ACTIVE_CHANNEL_4;
			/* Input capture event */
			if ((htim->Instance->CCMR2 & TIM_CCMR2_CC4S) != 0x00U)
			{
	//#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
	//        htim->IC_CaptureCallback(htim);
	//#else
	//        HAL_TIM_IC_CaptureCallback(htim);
	//#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
			}
			/* Output compare event */
			else
			{
	//#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
	//        htim->OC_DelayElapsedCallback(htim);
			  h->TimerPulseFinished(htim);
	//#else
	//        HAL_TIM_OC_DelayElapsedCallback(htim);
	//        HAL_TIM_PWM_PulseFinishedCallback(htim);
	//#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
			}
			htim->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
		 }
	  }
	  /* TIM Update event */
	  if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_UPDATE) != RESET)
	  {
		 if (__HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_UPDATE) != RESET)
		 {
			__HAL_TIM_CLEAR_IT(htim, TIM_IT_UPDATE);
	//#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
			h->TimerPeriodElapsed(htim);
	//#else
	//      HAL_TIM_PeriodElapsedCallback(htim);
	//#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
		 }
	  }
	  /* TIM Break input event */
	  if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_BREAK) != RESET)
	  {
		 if (__HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_BREAK) != RESET)
		 {
			__HAL_TIM_CLEAR_IT(htim, TIM_IT_BREAK);
	//#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
	//      htim->BreakCallback(htim);
	//#else
	//      HAL_TIMEx_BreakCallback(htim);
	//#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
		 }
	  }
	  /* TIM Break2 input event */
	  if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_BREAK2) != RESET)
	  {
		 if (__HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_BREAK) != RESET)
		 {
			__HAL_TIM_CLEAR_FLAG(htim, TIM_FLAG_BREAK2);
	//#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
	//      htim->Break2Callback(htim);
	//#else
	//      HAL_TIMEx_Break2Callback(htim);
	//#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
		 }
	  }
	  /* TIM Trigger detection event */
	  if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_TRIGGER) != RESET)
	  {
		 if (__HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_TRIGGER) != RESET)
		 {
			__HAL_TIM_CLEAR_IT(htim, TIM_IT_TRIGGER);
	//#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
	//      htim->TriggerCallback(htim);
	//#else
	//      HAL_TIM_TriggerCallback(htim);
	//#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
		 }
	  }
	  /* TIM commutation event */
	  if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_COM) != RESET)
	  {
		 if (__HAL_TIM_GET_IT_SOURCE(htim, TIM_IT_COM) != RESET)
		 {
			__HAL_TIM_CLEAR_IT(htim, TIM_FLAG_COM);
	//#if (USE_HAL_TIM_REGISTER_CALLBACKS == 1)
	//      htim->CommutationCallback(htim);
	//#else
	//      HAL_TIMEx_CommutCallback(htim);
	//#endif /* USE_HAL_TIM_REGISTER_CALLBACKS */
		 }
	  }
	}


	void TIM8_UP_TIM13_IRQHandler(void)
	{
	   if ( tim_handler[ 8] != NULL ) DM_TIM_IRQHandler( &htim8, tim_handler[ 8] );	
		if ( tim_handler[ 13] != NULL ) DM_TIM_IRQHandler( &htim13, tim_handler[ 13] );	
	}

	void TIM8_CC_IRQHandler(void)
	{ 
		if ( tim_handler[ 8] != NULL ) DM_TIM_IRQHandler( &htim8, tim_handler[ 8] );
	}

	void  TIM6_DAC_IRQHandler( void )
	{
		if ( tim_handler[ 6] != NULL ) DM_TIM_IRQHandler( &htim6, tim_handler[ 6] );
	}

	void  TIM7_IRQHandler( void )     
	{
		if ( tim_handler[ 7] != NULL ) DM_TIM_IRQHandler( &htim7, tim_handler[ 7] );
	}

	void TIM1_UP_TIM10_IRQHandler(void)
	{
		if ( tim_handler[ 10] != NULL )  DM_TIM_IRQHandler( &htim10, tim_handler[ 10] );
	}

	void TIM8_TRG_COM_TIM14_IRQHandler(void)
	{
		if ( tim_handler[ 14] != NULL ) DM_TIM_IRQHandler( &htim14, tim_handler[ 14] );
	}

}
