/***************************************************************************
* File name: MM_XML.h
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
* device structures
*
**************************************************************************/

#ifndef _XML_H_
#define _XML_H_


/* System Include Files
**************************************************************************/
#include <assert.h>
#include <string.h>


/* User Include Files
**************************************************************************/
#include "MM_Application.h"


/* Defines
**************************************************************************/
#define MAX_HANDLERS 24

#define XML_OK								0
#define XML_MISMATCHED_ELEMENT 		1
#define XML_UNKNOWN_ATTRIBUTE			2
#define XML_MISSING_ATTRIBUTE			3
#define XML_EMPTY_ELEMENT				4
#define XML_UNEXPECTED_DATA			5
#define XML_BAD_VALUE					6
#define XML_BAD_NUMBER_VALUE			7
#define XML_MISSING_DATA				8
#define XML_UNEXPECTED_SYMBOL			9
#define XML_INIT_FAIL				  10
#define XML_USER_CANCEL				  11
#define XML_PANEL_BUSY				  12
#define XML_WRITE_PROTECTED		  13
#define XML_EMPTY_QUOTES			  14
#define XML_ILLEGAL_SITE			  15

#define STACK_SIZE		12

#define STRCHK(x, y, z) if ( strlen(y) > ( z-1) ) return XML_BAD_VALUE; else strcpy( x, y );

class XMLHandlers;


enum XMLState
{
	UNKNOWN,
	ELEMENT,
	BEGIN_ELEMENT,
	END_ELEMENT,
	ATTRIB,
	ATTRIBVALUE,
	FAILED,
};


struct XMLList;


struct XMLElementHandler
{
	XMLElementHandler( const char* name, XMLList* );
	
	virtual void Init( );
	
	virtual int Begin( );
	virtual int End( );
	
	virtual int Attribute( char* name, char* value );
	
	int index;
	int data;
	const char* name;
};


struct XMLList
{
	XMLList( );
	XMLElementHandler* 				list[ MAX_HANDLERS ];
	XMLElementHandler**		 		pos;
};



class XMLParser
{
	public:
		
	XMLParser( );
	
	void Start( );
	int  Parse( char* data, int len );
	int 				line;
	XMLList* 		lib;	
	
	private:
	
	XMLElementHandler*	elementhandler[ STACK_SIZE ];
	
	int 				stack_pos;	

	int 				remainlen;
	char 				buff[ 64];
	char 				remainbuff;

	char*				elementpos;
	char 				elementbuff[ 32 ];
	char*				attribpos;
	char*				valuepos;
	XMLState			state;
	char 				attribbuff[ 40 ];
	char				valuebuff[ 512 ];
	char 				elementname[ 160 ];
	char 				startchr;
};


#endif

