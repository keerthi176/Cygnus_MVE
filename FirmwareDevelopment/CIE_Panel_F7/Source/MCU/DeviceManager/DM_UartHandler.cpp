
/***************************************************************************
* File name: DM_UartHandler.cpp
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
* Uart Handling
*
************************************************************************/


/* System Include Files
**************************************************************************/
#include "stm32f767xx.h"


/* User Include Files
**************************************************************************/

#include "DM_Handlers.h"
#include "DM_UartHandler.h"
 
 
/* Defines
**************************************************************************/

#define UART_BYTE_GAP 3


/* Globals
**************************************************************************/

extern "C"
{

static void UART_EndTransmit_IT(UART_HandleTypeDef *huart, UartHandler* uh )
{
  /* Disable the UART Transmit Complete Interrupt */
  CLEAR_BIT(huart->Instance->CR1, USART_CR1_TCIE);

  /* Tx process is ended, restore huart->gState to Ready */
  huart->gState = HAL_UART_STATE_READY;

  /* Cleat TxISR function pointer */
  huart->TxISR = NULL;

	uh->UartTxComplete( huart );
}


void UART_EndRxTransfer(UART_HandleTypeDef *huart)
{
  /* Disable RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts */
  CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));
  CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

  /* At end of Rx process, restore huart->RxState to Ready */
  huart->RxState = HAL_UART_STATE_READY;

  /* Reset RxIsr function pointer */
  huart->RxISR = NULL;
}
}

extern UART_HandleTypeDef huart6;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;
extern UART_HandleTypeDef huart5;



UartHandler* uart_handler[ 8 ] = { 0 };


/*************************************************************************/
/**  \fn      UartHandler::UartHandler( )
***  \brief   Constructor for class
**************************************************************************/
 
UartHandler::UartHandler( UART_HandleTypeDef* uh )
{
	manager.Add( uh, this );
	
	switch( (int)uh->Instance )
	{
		case USART1_BASE : uart_handler[ 1] = this;	break;
		case USART2_BASE : uart_handler[ 2] = this;	break;
		case USART3_BASE : uart_handler[ 3] = this;	break;
		case UART4_BASE  : uart_handler[ 4] = this;	break;
		case UART5_BASE  : uart_handler[ 5] = this;	break;
		case USART6_BASE : uart_handler[ 6] = this;	break;
		case UART7_BASE  : uart_handler[ 7] = this;	break;
	}	
	
	bytetime = UART_BYTE_GAP * 1000 * 10 / uh->Init.BaudRate;
	
	if ( bytetime == 0 ) bytetime = 999999;
		
	HAL_UART_RegisterCallback( uh, HAL_UART_TX_COMPLETE_CB_ID, StaticUartTxComplete );
	HAL_UART_RegisterCallback( uh, HAL_UART_RX_COMPLETE_CB_ID, StaticUartRxComplete );
	HAL_UART_RegisterCallback( uh, HAL_UART_ERROR_CB_ID, 		  StaticUartError );
};	
	

/*************************************************************************/
/***  \brief   Virtual functions  
**************************************************************************/

void UartHandler::UartTxComplete( UART_HandleTypeDef* uh )  {}
void UartHandler::UartRxComplete( UART_HandleTypeDef* uh )  {}
void UartHandler::UartError( UART_HandleTypeDef* uh )       {}
void UartHandler::UartCommand( UART_HandleTypeDef* uh )     {}
void UartHandler::UartStartPacket( UART_HandleTypeDef* uh ) {}


HANDLER( Static, UartHandler, UartTxComplete, UART_HandleTypeDef )
HANDLER( Static, UartHandler, UartRxComplete, UART_HandleTypeDef )
HANDLER( Static, UartHandler, UartError, 		 UART_HandleTypeDef )

	
void UartHandler::RxISR( UART_HandleTypeDef *huart )
{
  uint16_t uhMask = huart->Mask;
  uint8_t  uhdata;

  /* Check that a Rx process is ongoing */
  if (huart->RxState == HAL_UART_STATE_BUSY_RX)
  {
    *huart->pRxBuffPtr = READ_REG(huart->Instance->RDR);
    huart->pRxBuffPtr++;
    huart->RxXferCount--;

    if (huart->RxXferCount == 0U)
    {
      /* Disable the UART Parity Error Interrupt and RXNE interrupts */
      CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));

      /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
      CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

      /* Rx process is completed, restore huart->RxState to Ready */
      huart->RxState = HAL_UART_STATE_READY;

      /* Clear RxISR function pointer */
      huart->RxISR = NULL;

#if (USE_HAL_UART_REGISTER_CALLBACKS == 1)
      /*Call registered Rx complete callback*/
      huart->RxCpltCallback(huart);
#else
      /*Call legacy weak Rx complete callback*/
      HAL_UART_RxCpltCallback(huart);
#endif /* USE_HAL_UART_REGISTER_CALLBACKS */
    }
  }
  else
  {
    /* Clear RXNE interrupt flag */
    __HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);
  }
}	
	

#/*************************************************************************/
/**  \fn      void USART_IRQHandler(void)
***  \brief   IRQ Handler
**************************************************************************/

	
void USART_IRQHandler( UART_HandleTypeDef* huart, UartHandler* uh )
{
	uint32_t isrflags   = READ_REG(huart->Instance->ISR);
	uint32_t cr1its     = READ_REG(huart->Instance->CR1);
	uint32_t cr3its     = READ_REG(huart->Instance->CR3);

	uint32_t errorflags;
	uint32_t errorcode;

	/* If no error occurs */
	errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE));
	
	if (errorflags == 0U )
	{
		/* UART in mode Receiver ---------------------------------------------------*/
		if (((isrflags & USART_ISR_RXNE) != 0U)
		  && ((cr1its & USART_CR1_RXNEIE) != 0U))
		{
			unsigned int now = HAL_GetTick( );
			
			if ( huart->RxXferSize != huart->RxXferCount )
			{
				int diff = now - uh->lastrx;
			
				if ( diff > uh->bytetime )
				{
					uh->UartStartPacket( huart ); 
				}
			 
				if ( huart->Instance->RDR == '\n' && huart->pRxBuffPtr[-1] == '\r' )
				{
					*huart->pRxBuffPtr++ = '\n';
					*huart->pRxBuffPtr = 0;
					huart->RxXferCount--;
					uh->UartCommand( huart );
					return;					
				}	
			}
			  
			  /* Check that a Rx process is ongoing */
			  if (huart->RxState == HAL_UART_STATE_BUSY_RX)
			  {
				 *huart->pRxBuffPtr = READ_REG(huart->Instance->RDR);
				 huart->pRxBuffPtr++;
				 huart->RxXferCount--;

				 if (huart->RxXferCount == 0U)
				 {
					/* Disable the UART Parity Error Interrupt and RXNE interrupts */
					CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));

					/* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
					CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

					/* Rx process is completed, restore huart->RxState to Ready */
					huart->RxState = HAL_UART_STATE_READY;

					/* Clear RxISR function pointer */
					huart->RxISR = NULL;
					 
					uh->UartRxComplete( huart );
 
				 }
			  }
			  else
			  {
				 /* Clear RXNE interrupt flag */
				 __HAL_UART_SEND_REQ(huart, UART_RXDATA_FLUSH_REQUEST);
			  }
						
			uh->lastrx = now;
			
			return;
		}
	}

  /* If some errors occur */
  if ((errorflags != 0U)
      && (((cr3its & USART_CR3_EIE) != 0U)
          || ((cr1its & (USART_CR1_RXNEIE | USART_CR1_PEIE)) != 0U)))
  {
    /* UART parity error interrupt occurred -------------------------------------*/
    if (((isrflags & USART_ISR_PE) != 0U) && ((cr1its & USART_CR1_PEIE) != 0U))
    {
      __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_PEF);

      huart->ErrorCode |= HAL_UART_ERROR_PE;
    }

    /* UART frame error interrupt occurred --------------------------------------*/
    if (((isrflags & USART_ISR_FE) != 0U) && ((cr3its & USART_CR3_EIE) != 0U))
    {
      __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_FEF);

      huart->ErrorCode |= HAL_UART_ERROR_FE;
    }

    /* UART noise error interrupt occurred --------------------------------------*/
    if (((isrflags & USART_ISR_NE) != 0U) && ((cr3its & USART_CR3_EIE) != 0U))
    {
      __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_NEF);

      huart->ErrorCode |= HAL_UART_ERROR_NE;
    }

    /* UART Over-Run interrupt occurred -----------------------------------------*/
 
    if (((isrflags & USART_ISR_ORE) != 0U)
        && (((cr1its & USART_CR1_RXNEIE) != 0U) ||
            ((cr3its & USART_CR3_EIE) != 0U)))
    {
      __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_OREF);

      huart->ErrorCode |= HAL_UART_ERROR_ORE;
    }

    /* Call UART Error Call back function if need be --------------------------*/
    if (huart->ErrorCode != HAL_UART_ERROR_NONE)
    {
      /* UART in mode Receiver ---------------------------------------------------*/
      if (((isrflags & USART_ISR_RXNE) != 0U)
          && ((cr1its & USART_CR1_RXNEIE) != 0U))
      {
			unsigned int now = HAL_GetTick( );
			
			if ( huart->RxXferSize != huart->RxXferCount )
			{
				int diff = now - uh->lastrx;
			
				if ( diff > uh->bytetime )
				{
					uh->UartStartPacket( huart ); 
				}
			 
				if ( huart->Instance->RDR == '\r' )
				{
					*huart->pRxBuffPtr++ = 0;
					uh->UartCommand( huart );				
				}	
			}
			
			if (huart->RxISR != NULL)
			{
				uh->RxISR(huart);
			}
			
			uh->lastrx = now;
      }

      /* If Overrun error occurs, or if any error occurs in DMA mode reception,
         consider error as blocking */
      errorcode = huart->ErrorCode;
      if ((HAL_IS_BIT_SET(huart->Instance->CR3, USART_CR3_DMAR)) ||
          ((errorcode & HAL_UART_ERROR_ORE) != 0U))
      {
        /* Blocking error : transfer is aborted
           Set the UART state ready to be able to start again the process,
           Disable Rx Interrupts, and disable Rx DMA request, if ongoing */
        UART_EndRxTransfer(huart);
 
        uh->UartError( huart );  
      }
      else
      {
        /* Non Blocking error : transfer could go on.
           Error is notified to user through user error callback */
		//	uh->UartError( huart );  
         huart->ErrorCode = HAL_UART_ERROR_NONE;
      }
    }
    return;

  } /* End if some error occurs */

  /* UART in mode Transmitter ------------------------------------------------*/
  if (((isrflags & USART_ISR_TXE) != 0U)
      && ((cr1its & USART_CR1_TXEIE) != 0U))
  {
    if (huart->TxISR != NULL)
    {
      huart->TxISR(huart);
    }
    return;
  }

  /* UART in mode Transmitter (transmission end) -----------------------------*/
  if (((isrflags & USART_ISR_TC) != 0U) && ((cr1its & USART_CR1_TCIE) != 0U))
  {
    UART_EndTransmit_IT(huart, uh);
    return;
  }
 	
 	
}
	
/*************************************************************************/
/**  \fn      void USART2_IRQHandler(void)
***  \brief   IRQ Handler
**************************************************************************/

extern "C" void USART2_IRQHandler( void )
{
	if ( uart_handler[ 2] ) USART_IRQHandler( &huart2, uart_handler[ 2] );
} 


 

/*************************************************************************/
/**  \fn      void USART1_IRQHandler(void)
***  \brief   IRQ Handler
**************************************************************************/
 
extern "C" void USART1_IRQHandler( void )
{
   if ( uart_handler[ 1] ) USART_IRQHandler( &huart1, uart_handler[ 1] );
}
 



/*************************************************************************/
/**  \fn      void USART5_IRQHandler(void)
***  \brief   IRQ Handler
**************************************************************************/
 
extern "C" void UART5_IRQHandler( void )
{
   if ( uart_handler[ 5] ) USART_IRQHandler( &huart5, uart_handler[ 5] );
}

/*************************************************************************/
/**  \fn      void USART5_IRQHandler(void)
***  \brief   IRQ Handler
**************************************************************************/
 
extern "C" void UART7_IRQHandler( void )
{
   if ( uart_handler[ 7] ) USART_IRQHandler( &huart7, uart_handler[ 7] );
}

