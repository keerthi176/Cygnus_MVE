/***************************************************************************
* File name: MM_BIST.cpp
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
* Built In Self Test
*
**************************************************************************/

/* System Include Files
**************************************************************************/
#include "stm32f7xx_hal.h"
 
/* Defines
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "MM_Application.h"



BIST::BIST( Application* app ) : Module( "BIST", 0, app )
{
	CheckI2C( app->
}


/*************************************************************************/
/**  \fn      void CheckI2C( int device, int addr, char val )
***  \brief   Global helper function
**************************************************************************/

int BIST::CheckI2C( I2C_HandleTypeDef* bus, int device, int addr, char val )
{
	int pass = 0;
	
	HAL_I2C_IsDeviceReady( bus, device, 3, 100 ); 
	
	for( int n = 0; n < 1000; n++ )
	{
		HAL_StatusTypeDef status;
		uint8_t value = 0;
  
		status = HAL_I2C_Mem_Read( i2c, device, addr, I2C_MEMADD_SIZE_8BIT, &value, 1, 5 );

		if ( status == HAL_OK && value == val )
		{
			pass++;
		}
		else
		{
			app->DebOut( "Fail on iteration %d.\n", n + 1 );
			
			return false;
		}
	}
	
	return true;
}	


int BIST::CheckEEP( I2C_HandleTypeDef* bus, int device )
{
	int pass = 0;
	
	for( int n = 512; n < 512 + 100; n++ )
	{
		HAL_StatusTypeDef s1, s2, s3, s4;
		int32_t value, value2;
				
		s1 = HAL_I2C_Mem_Read( bus, device, n*4, I2C_MEMADD_SIZE_16BIT, (uint8_t*) &value, 4, 100 );
		
		if ( HAL_I2C_IsDeviceReady( bus, device, 3, 100 ) )
		{
			s2 = HAL_I2C_Mem_Write( bus, device, n*4, I2C_MEMADD_SIZE_16BIT, (uint8_t*) &value, 4, 100 );
		}
		else
		{
			return false;
		}
		
		s3 = HAL_I2C_Mem_Read( bus, device, n*4, I2C_MEMADD_SIZE_16BIT, (uint8_t*) &value2, 4, 100 );
		 
		if ( s1== HAL_OK && s2 == HAL_OK && s3 == HAL_OK && value2 == value1 )
		{
			pass++;
		}
		else
		{
			app->DebOut( "Fail on iteration %d.\n", n - 511 );			
			return false;
		}
	}
	return true;
}

