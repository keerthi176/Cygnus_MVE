

void DrawLine( int x, int y, int w, int c )
{
	int n;
	short* p  = (short*)0xC0000000 + ( y * 800 ) + x;
	short* p2 = (short*)0xC0000000 + (800 * 480 ) + ( y * 800 ) + x;
	
	for( n = 0; n < w; n++ )
	{
		*p++ = c;
		*p2++ =c;
	}
}


/*************************************************************************/
/**  \fn      static void DrawFloodFill( )
***  \brief   Global helper function
**************************************************************************/

void DrawFloodFill( )
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
/**  \fn      static void rect( int x, int y, int w, int h, int c )
***  \brief   Global helper function
**************************************************************************/

void DrawRect( int x, int y, int w, int h, int c )
{
	int n;
	
	for( n = 0; n < h; n++ )
	{
		DrawLine( x, y + n, w, c );
	}
}

/*************************************************************************/
/**  \fn      static void rect( int x, int y, int w, int h, int c )
***  \brief   Global helper function
**************************************************************************/

char scans[] = { 2,4,5,5,6,6,7,7,7 };

void DrawSect( int x, int y, int start, short col )
{
	int n;
	int size = sizeof( scans );
	
	if ( start ) for( n = 0; n < size; n++ )
	{
		DrawLine( x - scans[ n], y + n, scans[ n], col );
		DrawLine( x - scans[ n], y + size*2 - n-1, scans[ n], col );
	}
	
	else for( n = 0; n < sizeof( scans ); n++ )
	{
		DrawLine( x, y + n, scans[ n], col );
		DrawLine( x, y + size*2 - n -1, scans[ n], col );
	}
}

