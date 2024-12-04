/***************************************************************************
* File name: MM_SplashLoader.h
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
* Splash loader.
*
**************************************************************************/
/* System Include Files
**************************************************************************/


/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "DM_TimerHandler.h"

									
extern int cygnus_bitmap_x;		
extern int cygnus_bitmap_y;		
extern int cygnus_bitmap_w;		
extern int cygnus_bitmap_h;			
	
extern const unsigned char cygnus_bitmap_i; 	       	
	
extern int smartnet_bitmap_x;		
extern int smartnet_bitmap_y;		
extern int smartnet_bitmap_w;		
extern int smartnet_bitmap_h;		
	
extern const unsigned char smartnet_bitmap_i;	

extern int sitenet_bitmap_x;		
extern int sitenet_bitmap_y;		
extern int sitenet_bitmap_w;		
extern int sitenet_bitmap_h;		
	
extern const unsigned char sitenet_bitmap_i;


class SplashLoader : public Module, public TimerHandler
{
	public:
		
	SplashLoader( );
	
	virtual int Receive( Event );
	virtual int Receive( Command* );
	virtual int Receive( Message* );
	
	static void DrawSolidRect( int x, int y, int w, int h, int c );
	
	virtual void TimerPeriodElapsed( TIM_HandleTypeDef* );
};

