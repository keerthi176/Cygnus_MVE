/***************************************************************************
* File name: MM_list.h
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
* list templates
*
**************************************************************************/
#ifndef _LIST_H_
#define _LIST_H_


/* System Include Files
**************************************************************************/
#include <stdlib.h>
//#include "MM_CUtils.h"

/* User Include Files
**************************************************************************/

#define DEFAULT_BUFFER_SIZE 20

extern "C" void mattcpy( void* dst, void* src, int bytes );

/*************************************************************************/
/**           class Circular<class Item>
***  \brief   circular buffer template
**************************************************************************/
	
template<class Item>
class Circular
{
	Item* base;
	Item* inpos;
	Item* outpos;
	Item* end;
	
	public:
	
	int items;
	int size;	
	
	
	Circular( Item* buffer, int size ) : size( size )
	{
		end = size + ( base = inpos = outpos = buffer );
		items = 0;
	}
	
	~Circular( )
	{

	}
	
	void Clear( )
	{
		items = 0;
		inpos = outpos = base;
	}
	
	bool Empty( )
	{
		return items == 0;
	}
	
	bool Full( )
	{
		return items >= size;
	}
	
	bool Push( Item& itm )
	{
		while ( items >= size )
		{
			Pop( );
		}
		
		if ( inpos != NULL )
		{
			mattcpy( inpos, &itm, sizeof( Item ) );
			inpos++;
			items++;

			// wrap if need
			if ( inpos == end ) inpos = base;
			
			return true;
		}
		
		return false;
	}
	
	bool PushFront( Item& itm )
	{
		while ( items >= size )
		{
			Pop( );
		}
		
		if ( outpos != NULL )
		{
			outpos--;
			// wrap if need
			if ( outpos < base ) outpos = end - 1;
			
			mattcpy( outpos, &itm, sizeof( Item ) );
			 
			items++;
	 
			return true;
		}
		
		return false;
	}
	
	Item& operator[](int i)
	{
		if ( outpos + i >= end ) 
		{
			return base[ (outpos + i) - end ];
		}
		return outpos[ i ];
	}
	
	bool Remove( Item* item )
	{
		bool found = false;
						
		for( int n = 1; n < items; n++ )
		{
			Item* i = outpos + items - n;
			
			// wrap if need
			if ( i >= end ) i = base + ( i - end );
						
			if ( i == item )
			{
				found = true;
			}
			if ( found )
			{
				if ( i == base )
				{
					*i = *(end - 1);
				}
				else
				{
					*i = *(i - 1 );
				}
			}
		}
		if ( found ) 
		{
			items--;
			outpos++;
			if ( outpos == end ) outpos = base;
		}
		return found;
	}
	
	Item* Pop( )
	{
		if ( items > 0 )
		{
		//	app.DebOut( "Popping from &%X\n", outpos );
			Item* output = outpos;
			items--;
			outpos++;
			if ( outpos == end ) outpos = base;
			return output;
		}
		else return NULL;
	}
	
	Item* Peek( )
	{
		if ( items > 0 )
		{
			return outpos;
		}
		else return NULL;
	}
};

#endif

