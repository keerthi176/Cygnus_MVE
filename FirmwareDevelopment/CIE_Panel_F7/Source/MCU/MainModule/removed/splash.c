/***************************************************************************
* File name: splash.c
* Project name: CIE Panel
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
* 1.0 MM.YYYY Initial File XX
*
* Description:
* Splash loader
*
**************************************************************************/
/* System Include Files
**************************************************************************/

	
/* User Include Files
**************************************************************************/
#include "splash.h"

/* Defines
**************************************************************************/
#define I2Cx_TIMEOUT_MAX                    0x3000 /*<! The value of the maximal timeout for I2C waiting loops */


/* Global Variables
**************************************************************************/



static int nib; 

#define NEXT(p) ( (nib++ & 1) ? (*(p)++ >> 4) & 15 : *(p) & 15 )
#define LOOK(p) ( (nib & 1) ? (*(p) >> 4) & 15 : *(p) & 15 )

void line( int x, int y, int w, int c )
{
	int n;
	int* p = (int*)0xC0000000 + ( y * 400 ) + (x>>1);
	
	for( n = 0; n < w>>1; n++ )
	{
		*p++ = c;
	}
}

void rect( int x, int y, int w, int h, int c )
{
	int n;
	
	for( n = 0; n < h; n++ )
	{
		line( x, y + n, w, c );
	}
}

void bar( int x, int y, int w, int h, int p, int c1, int c2, int c3, int c4 )
{
	rect( x, y, w-p, 3, c1 );
	rect( x + (w-p), y, p, 3, c2 );

	rect( x, y + 3, w-p, h-3, c3 );
	rect( x + (w-p), y + 3, p, h-3, c4 );
}



void loader_blit( const unsigned char* p, int x, int y, int w, int h )
{
	short* d = (short*)0xC0000000 + (479-(y+h)) * 800 + x + w - 1;
	int xn, yn;	
	int count = 0;
	unsigned char type;
	
	unsigned char pix;
	
	nib = 0;
	
	for( yn = 0; yn < h; yn++ )
	{
		for( xn = 0; xn < w; xn++ )
		{
			char dat = LOOK(p);
			
			if ( count == 0 && ( dat == 0 || dat == 0xE ))
			{
				type = NEXT(p);
				count = NEXT(p);
				count += NEXT(p) << 4;
			}
			
			if ( count > 0 )
			{
				count--;
				pix = type;
			}
			else
			{
				pix = NEXT(p)  ;	
			}	
				
			*d-- =  pix << 12;
		}
		
	 	d += 800 + w ;
	}
}		
	
TIM_HandleTypeDef  	htim14; 


HAL_StatusTypeDef loader_start(uint32_t TickPriority)
{
  RCC_ClkInitTypeDef    clkconfig;
  uint32_t              uwTimclock = 0;
  uint32_t              uwPrescalerValue = 0;
  uint32_t              pFLatency;
  
  /*Configure the TIM14 IRQ priority */
  HAL_NVIC_SetPriority(TIM8_TRG_COM_TIM14_IRQn, 0 ,0); 
  
  /* Enable the TIM14 global Interrupt */
  HAL_NVIC_EnableIRQ(TIM8_TRG_COM_TIM14_IRQn); 
  
  /* Enable TIM14 clock */
  __HAL_RCC_TIM14_CLK_ENABLE();
  
  /* Get clock configuration */
  HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);
  
  /* Compute TIM14 clock */
  uwTimclock = HAL_RCC_GetPCLK1Freq();
   
  /* Compute the prescaler value to have TIM14 counter clock equal to 1MHz */
  uwPrescalerValue = (uint32_t) ((uwTimclock / 10000) - 1);
  
  /* Initialize TIM14 */
  htim14.Instance = TIM14;
  
  /* Initialize TIMx peripheral as follow:
  + Period = [(TIM14CLK/1000) - 1]. to have a (1/1000) s time base.
  + Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
  + ClockDivision = 0
  + Counter direction = Up
  */
  htim14.Init.Period = 65535;
  htim14.Init.Prescaler = uwPrescalerValue;
  htim14.Init.ClockDivision = 0;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  if(HAL_TIM_Base_Init(&htim14) == HAL_OK)
  {
    /* Start the TIM time Base generation in interrupt mode */
    return HAL_TIM_Base_Start_IT(&htim14);
  }
  
  /* Return function status */
  return HAL_ERROR;
}

/**
  * @brief  Suspend Tick increment.
  * @note   Disable the tick increment by disabling TIM14 update interrupt.
  * @param  None
  * @retval None
  */
void loader_suspend(void)
{
  /* Disable TIM14 update Interrupt */
  __HAL_TIM_DISABLE_IT(&htim14, TIM_IT_UPDATE);                                                  
}

/**
  * @brief  Resume Tick increment.
  * @note   Enable the tick increment by Enabling TIM14 update interrupt.
  * @param  None
  * @retval None
  */
void loader_continue(void)
{
  /* Enable TIM14 Update interrupt */
  __HAL_TIM_ENABLE_IT(&htim14, TIM_IT_UPDATE);
}

void loader_tick(void)
{
	static int left = 500;
	
	bar( 150, 40, 500, 9, 500 - left, 0x98768432, 0xAB7FCC1A, 0x87657321, 0x9A6EBB09 );
	
	left -= left >> 4;
}
