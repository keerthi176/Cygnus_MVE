/***************************************************************************
* File name: MM_CAN.cpp
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
* CAN API
*
**************************************************************************/

/* System Include Files
**************************************************************************/

#include "stm32f7xx_hal.h"
#include <stdint.h>


/* User Include Files
**************************************************************************/

#include "MM_CAN.h"
#include "MM_Application.h"


static CAN_TxHeaderTypeDef   TxHeader;
static CAN_RxHeaderTypeDef   RxHeader;

static uint8_t               RxData[8];	
static uint8_t               TxData[8];

static uint32_t              TxMailbox;


/*************************************************************************/
/**  \fn      CAN::CAN( ) 
***  \brief   Constructor function
**************************************************************************/

CAN::CAN( ) : Module( "CAN", 0 )
{
   /* Configure the CAN peripheral */
   Config();
}


/*************************************************************************/
/**  \fn      void CAN::Poll( )
***  \brief   Module loop function
**************************************************************************/

void CAN::Poll( )
{

}


/*************************************************************************/
/**  \fn      void CAN::Config( void )
***  \brief   Module loop function
**************************************************************************/

void CAN::Config( void )
{
	CAN_FilterTypeDef  sFilterConfig;

	/* Configure the CAN Filter ###########################################*/
	sFilterConfig.FilterBank = 0;
	sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
	sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
	sFilterConfig.FilterIdHigh = 0x0000;
	sFilterConfig.FilterIdLow = 0x0000;
	sFilterConfig.FilterMaskIdHigh = 0x0000;
	sFilterConfig.FilterMaskIdLow = 0x0000;
	sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
	sFilterConfig.FilterActivation = ENABLE;
	sFilterConfig.SlaveStartFilterBank = 14;

	if ( HAL_CAN_ConfigFilter( app.can, &sFilterConfig ) != HAL_OK )
	{
		/* Filter configuration Error */
		app.Report( ISSUE_CAN_FILTER_CONFIG_FAIL );
	}

	/* Start the CAN peripheral */
	if ( HAL_CAN_Start( app.can ) != HAL_OK )
	{
		/* Start Error */
		app.Report( ISSUE_CAN_START_FAIL );
	}

	/* Activate CAN RX notification */
	if ( HAL_CAN_ActivateNotification( app.can, CAN_IT_RX_FIFO0_MSG_PENDING ) != HAL_OK )
	{
	   /* Notification Error */
		app.Report( ISSUE_CAN_ACTIVATION_FAIL );
	}

	/* Configure Transmission process  */
	TxHeader.StdId = 0x321;
	TxHeader.ExtId = 0x01;
	TxHeader.RTR = CAN_RTR_DATA;
	TxHeader.IDE = CAN_ID_STD;
	TxHeader.DLC = 2;
	TxHeader.TransmitGlobalTime = DISABLE;
}


/*************************************************************************/
/**  \fn      void HAL_CAN_RxFifo0MsgPendingCallback( CAN_HandleTypeDef *hcan )
***  \brief   Callback function
**************************************************************************/

void HAL_CAN_RxFifo0MsgPendingCallback( CAN_HandleTypeDef *hcan )
{
	/* Get RX message */
	if ( HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData ) != HAL_OK )
	{
		/* Reception Error */
		app.Report( ISSUE_CAN_RX_FAIL );
		return;
	}

	if (( RxHeader.StdId == 767 ) && ( RxHeader.IDE == CAN_ID_STD ) && ( RxHeader.DLC == 2 ) )
	{
		app.DebOut( "Received CAN msg from 767\n" );
		
		if ( HAL_CAN_AddTxMessage( app.can, &TxHeader, TxData, &TxMailbox ) != HAL_OK )
		{
				 /* Transmission request Error */
			 app.Report( ISSUE_CAN_TX_FAIL );
		}
		
		 while (HAL_CAN_GetTxMailboxesFreeLevel(app.can) != 3)
		 {
		 }
		 
		 app.DebOut( "Sent msg back OK!\n" );		 
	}
}
