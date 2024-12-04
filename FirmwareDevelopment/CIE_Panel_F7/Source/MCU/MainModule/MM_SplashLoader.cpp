/***************************************************************************
* File name: MM_SplashLoader.cpp
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
#include <assert.h>
 
	
/* User Include Files
**************************************************************************/
#include "MM_SplashLoader.h"
#include "MM_Application.h"
#include "MM_error.h"
#include "vgamode.h"

/* Defines
**************************************************************************/
#define COL(b,g,r) (r|(g<<6)|(b<<11)|(r<<16)|(g<<22)|(b<<27))
#define ALPHABLEND 1

/* Global Variables
**************************************************************************/

static int nib; 

#define NEXT(p) ( (nib++ & 1) ? (*(p)++ >> 4) & 15 : *(p) & 15 )
#define LOOK(p) ( (nib & 1) ? (*(p) >> 4) & 15 : *(p) & 15 )

 
/*************************************************************************/
/**  \fn      static void loader_flood( )
***  \brief   Global helper function
**************************************************************************/

static void loader_flood( )
{
	int x,y;
	int* p = (int*) 0xC0000000;
	
	unsigned int g = 57*16;
	unsigned int b = 80*16;
	
	short* pix = (short*) p;
	
	for( y = 0; y < 480; y++ )
	{
		unsigned int gfrac = 0;
		unsigned int bfrac = 0;
		
		for( x = 0; x < 800; x++ )
		{
			unsigned int cgreen = g + gfrac;
			unsigned int cblue = b + bfrac;
			
			if ( cgreen > 57*16 ) cgreen = 57*16;
			if ( cblue  > 80*16 ) cblue  = 80*16;
 
			
			unsigned int hgreen = ( cgreen >> 6);
			unsigned int hblue  = ( cblue >> 7);
			
			gfrac  = cgreen & 0x3F;	
			bfrac  = cblue & 0x7F;
			
			*pix++ = hblue + (hgreen << 5);
			g++;
			b++;
	
		}
		g-=780;
		b-=780;
	}
}	
 		

/*************************************************************************/
/**  \fn      static void line( int x, int y, int w, int c )
***  \brief   Global helper function
**************************************************************************/

static void line( int x, int y, int w, int c )
{
	int n;
	int* p  = (int*)0xC0000000 + ( y * 400 ) + (x>>1);
	int* p2 = (int*)0xC0000000 + (800 * 480 / 2) + ( y * 400 ) + (x>>1);
	
	for( n = 0; n < w>>1; n++ )
	{
		*p++ = c;
		*p2++ =c;
	}
}

/*************************************************************************/
/**  \fn      static void rect( int x, int y, int w, int h, int c )
***  \brief   Global helper function
**************************************************************************/

void SplashLoader::DrawSolidRect( int x, int y, int w, int h, int c )
{
	int n;
	
	for( n = 0; n < h; n++ )
	{
		line( x, y + n, w, c );
	}
}

/*************************************************************************/
/**  \fn      static void bar( int x, int y, int w, int h, int p, int c1, int c2, int c3, int c4 )
***  \brief   Global helper function
**************************************************************************/

static void bar( int x, int y, int w, int h, int p, int c1, int c2, int c3, int c4 )
{
	SplashLoader::DrawSolidRect( x, y, w-p, 1, c1 );
	SplashLoader::DrawSolidRect( x + (w-p), y, p, 1, c2 );

	SplashLoader::DrawSolidRect( x, y + 1, w-p, h-1, c3 );
	SplashLoader::DrawSolidRect( x + (w-p), y + 1, p, h-1, c4 );
}


#if 1
/*************************************************************************/
/**  \fn      static void loader_blit_red( const unsigned char* p, int x, int y, int w, int h )
***  \brief   Global helper function
**************************************************************************/

static void loader_blit_red( const unsigned char* p, int x, int y, int w, int h )
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
#endif


#if 1
/*************************************************************************/
/**  \fn      static void loader_blit( const unsigned char* p, int x, int y, int w, int h )
***  \brief   Global helper function
**************************************************************************/

static void loader_blit( const unsigned char* p, int x, int y, int w, int h )
{
#if VGA_MODE==0
	short* d = (short*)0xC0000000 + (479-(y+h)) * 800 + x + w - 1;
#else
	short* d = (short*)0xC0000000 + (y+h) * 800 + x;
#endif	
	int xn, yn;	
	int count = 0;
	unsigned char type;
#ifdef ALPHABLEND	
	short o = *d;
#endif
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
				
			if (pix == 0 )
			{
			
			}
			else
			{
#ifdef ALPHABLEND				
				short ord = o >> 11;
				short ogn = (o >> 5 ) & 63;
				short obl = o & 31;
				 
				short drd = 139/8;// 31;
				short dgn = 184/4;//63;
				short dbl = 232/8;//31;
				 
				short r = ( drd - ord ) * pix / 15;
				short g = ( dgn - ogn ) * pix / 15;
				short b = ( dbl - obl ) * pix / 15;
				
				r += ord;
				g += ogn;
				b += obl;
				
				*d =  b | (g << 5) | ( r << 11 );   					// alpha-blend
#else
#ifdef REDSCALE				
				*d =  (0x1F<<11) | ( pix << 7 ) | (pix<<1)  ;  	// redscale
#else				
				*d =  0x1F | (pix<<7) | ( pix << 12 );   			// bluescale
#endif	
#endif
			}
#if VGA_MODE==0
			d--;
#else
			d++;
#endif			
			
#ifdef ALPHABLEND				
			o = *d;
#endif			
		}
#if VGA_MODE==0	
	 	d += 800 + w ;
#else
		d += 800 - w;
		d -= 1600;
#endif
	}
}		

#endif


extern LTDC_HandleTypeDef hltdc;
extern LTDC_LayerCfgTypeDef pLayerCfg;
extern "C" void MX_LPTIM1_Init(void); 

/*************************************************************************/
/**  \fn      static void loader_tick( void* )
***  \brief   Global helper function
**************************************************************************/

static void loader_tick( )
{
	static int left = 500;
	
	//bar( 150, 40, 500, 9, 500 - left, COL(4,4,4), COL(31,1,1), COL(8,8,8),COL(31,2,2) );
	
	int col = 0x001F001F;
	 
#if VGA_MODE==0
	bar( 150, 40, 500, 9, 500 - left, COL(4,4,4), col, COL(8,8,8), col );

	left = left * 15 / 16;
	
	pLayerCfg.Alpha += 8;
#else

	pLayerCfg.Alpha += 4;
#endif	
	if ( pLayerCfg.Alpha > 255 ) pLayerCfg.Alpha = 255;
 
	HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0);
}


/*************************************************************************/
/**  \fn      SplashLoader::SplashLoader( )
***  \brief   Constructor for class
**************************************************************************/

SplashLoader::SplashLoader( ) : Module( "SplashLoader", 0, EVENT_GUI_STARTING | EVENT_LOADING_CENTISECOND ),
										TimerHandler( &htim14 )
{
	// Clear background
	memset( (char*)(0xC0000000), 0, 800*480*2 );

	// Enable display
 	HAL_GPIO_WritePin(GPIOA, DISPLAY_EN_Pin, GPIO_PIN_SET );
	
	// Switch on backlight
	MX_LPTIM1_Init( );
	 
	if ( 1 )
	{	
		loader_flood( );
		loader_blit( &cygnus_bitmap_i, cygnus_bitmap_x, cygnus_bitmap_y, cygnus_bitmap_w, cygnus_bitmap_h );
	//	loader_blit( &smartnet_bitmap_i, smartnet_bitmap_x, smartnet_bitmap_y, smartnet_bitmap_w, smartnet_bitmap_h );	
	}
	else
	{
		loader_blit_red( &cygnus_bitmap_i, cygnus_bitmap_x, cygnus_bitmap_y, cygnus_bitmap_w, cygnus_bitmap_h );
		loader_blit_red( &sitenet_bitmap_i, sitenet_bitmap_x, sitenet_bitmap_y, sitenet_bitmap_w, sitenet_bitmap_h );		
	}
		
	HAL_TIM_Base_Start_IT( &htim14 );	
}


/*************************************************************************/
/**  \fn      int SplashLoader::Receive( Event e )
***  \brief   Class function
**************************************************************************/

int SplashLoader::Receive( Event e )
{
	if ( e == EVENT_GUI_STARTING )
	{
		app.loading = false;
		HAL_TIM_Base_Stop_IT( &htim14 );
	}
	return 0;
}


/*************************************************************************/
/**  \fn      void SplashLoader::TimerPeriodElapsed( )
***  \brief   Class function
**************************************************************************/

void SplashLoader::TimerPeriodElapsed( TIM_HandleTypeDef* )
{
	loader_tick( );
}


/*************************************************************************/
/**  \fn      int SplashLoader::Receive( Command* )
***  \brief   Class function
**************************************************************************/

int SplashLoader::Receive( Command* )
{
	return CMD_ERR_UNKNOWN_CMD;
}


/*************************************************************************/
/**  \fn      int SplashLoader::Receive( Message* )
***  \brief   Class function
**************************************************************************/

int SplashLoader::Receive( Message* )
{
   return 0;
}
		

