/***************************************************************************
* File name: JSON.h
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
* JSON serializer
*
**************************************************************************/

#ifndef _JSON_
#define _JSON_

#include <stdint.h>

/* Defines
**************************************************************************/

enum JSONType
{
	JSON_NUM, JSON_STR, JSON_BOOL, JSON_SHORT_ARRAY
};
	

struct JSONHandler
{
	virtual char* ReadTo( char* id ) = 0;
};


enum JSONResult
{
	JSON_DONE,
	JSON_EOF,
	JSON_UNEXPECTED_BRACE,
	JSON_UNEXPECTED_QUOTE,
	JSON_UNEXPECTED_COLON,
	JSON_UNEXPECTED_COMMA,
	JSON_UNEXPECTED_DIGIT,
	JSON_UNEXPECTED_CHAR,
	JSON_NULL_BUFFER
};


class JSON
{
	public:
		
	static char* Make( JSONType, ... );
	
	JSONResult Decode( char* data, int length, JSONHandler* h );	
	
	char* id;
	char  id_buff[ 44];
	char* data;
	char* data_buff;
	
	int 			state;
	uint64_t		num;
};


#endif

