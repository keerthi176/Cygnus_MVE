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

/* System Include Files
**************************************************************************/
 
#include <stdio.h>
#include <stdarg.h>
#include <string.h>


/* User Include Files
**************************************************************************/
#include "JSON.h"


/* Defines
**************************************************************************/

#define ALPHA(c) (((c)>='a'&& (c)<='z') || (c)=='_' || ((c) >='A' && (c) <= 'Z' ) )
#define ALPHANUM(c) (ALPHA(c) || (c>='0' && c <='9') )

enum
{
	AWAITING_ID,
	READING_ID,
	READING_STRING,
	READING_NUMBER,
	AWAITING_COMMA_OR_CURLY,
	AWAITING_COLON,
	AWAITING_DATA,
};



char* JSON::Make( JSONType t1, ... )
{
	static char buff[ 1024 ];
	JSONType type[ 16];
	int count = 0;
	
	type[ count++ ] = t1;
	
	va_list args;
	
	va_start( args, t1 );
	
	int t;
	
	do
	{
		t = va_arg( args, int );
		
		if ( t == JSON_NUM || t == JSON_STR || t == JSON_BOOL || t == JSON_SHORT_ARRAY )
		{
			type[ count++ ] = (JSONType) t;
		}
		else		
		{
			break;
		}
	} while ( true );
	
	char* key = (char*) t;
	
	sprintf( buff, "{" );
	
	for( int n = 0; n < count; n++ )
	{
		char sbuff[ 128 ];
		
		if ( n != 0 ) strncat( buff, ", ", 1024 );
		
		if ( type[ n] == JSON_STR )
		{
			char* v = va_arg( args, char* );
			snprintf( sbuff, 128, "\"%s\": \"%s\"", key, v );
		}
		else if ( type[ n] == JSON_NUM )
		{
			int v = va_arg( args, int );
			snprintf( sbuff, 128, "\"%s\": %d", key, v );
		}
		else if ( type[ n] == JSON_BOOL )
		{
			int v = va_arg( args, int );
			snprintf( sbuff, 128, "\"%s\": %s", key, v ? "true" : "false" );
		}	
		else if ( type[ n] == JSON_SHORT_ARRAY )
		{
			short* array = va_arg( args, short* );
			int size = va_arg( args, int ); 
			 
			char* p = sbuff + sprintf( sbuff, "\"%s\": [", key );
			
			for( int  i = 0; i < size; i++ )
			{
				if ( i == 0 )
				{
					p += sprintf( p, "%d", array[ i ] );
				}
				else
				{
					p += sprintf( p, ",%d", array[ i ] );
				}
			}
			*p++ = ']';
			*p = 0;
		}
		strncat( buff, sbuff, 1024 );
		
		key = va_arg( args, char* );		
	}	 
	strncat( buff, "}", 1024 );	
	buff[ 1023 ] = 0;
	
	return buff;
}


JSONResult JSON::Decode( char* buffer, int length, JSONHandler* h )
{
	for( char* p = buffer; p < buffer + length; p++ )
	{
		char c = *p;
		
		switch ( c )
		{
			case '{' :
			{
				state = AWAITING_ID;
				break;
			}
			case '}' :
			{
				switch ( state )
				{
					case READING_NUMBER :
					{
						if ( data_buff != NULL ) *((uint64_t*) data_buff) = num;
						 
					}
					case AWAITING_ID : 
					case AWAITING_COMMA_OR_CURLY :
						return JSON_DONE;
					case READING_STRING: break;
					default : return JSON_UNEXPECTED_BRACE;
				}
			}
			case '"':
			{
				switch ( state )
				{
					case AWAITING_ID : 			state = READING_ID;
														id = id_buff;
														break;
					case READING_ID : 			*id = 0;
														state = AWAITING_COLON;
														break;
					case AWAITING_DATA:			state = READING_STRING;
														break;
					case READING_STRING:			if ( data != NULL ) *data = 0;
														state = AWAITING_COMMA_OR_CURLY;
														break;
					
					default:							return JSON_UNEXPECTED_QUOTE;
				}
				break;
			}
			case ':':
			{
				if ( state == AWAITING_COLON )
				{
					data_buff = data = h->ReadTo( id_buff );
					
			//		if ( data == NULL ) return JSON_NULL_BUFFER;
					state = AWAITING_DATA;
					
				}
				else
				{
					return JSON_UNEXPECTED_COLON;
				}
				break;
			}
			case ',':
			{
				switch ( state )
				{
					case READING_NUMBER :
					{
						if ( data_buff != NULL ) *((uint64_t*) data_buff) = num;
					}
					case AWAITING_COMMA_OR_CURLY : state = AWAITING_ID; break;
					default: return JSON_UNEXPECTED_COMMA;
				}
				break;
			}
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			{
				switch ( state )
				{
					case READING_STRING: if ( data != NULL ) *data++ = c;  break;
					case  READING_ID :
					{
						if ( id != NULL ) *id++ = c;	
						break;
					}
					case AWAITING_DATA:	num = 0; state = READING_NUMBER;
				
					case READING_NUMBER:
					{
						num *= 10;
						num += c - '0';
						break;
					}
					default: return JSON_UNEXPECTED_DIGIT;
				}
				break;
			}
			case ' ':
			case 9:
			{
				if ( state == READING_STRING ) 
				{
					if ( data != NULL ) *data++ = c;
				}
				break;
			}				
			default:
			{
				switch ( state )
				{
					case READING_ID :
					{
						if ( ALPHANUM(c) )
						{
							if ( id != NULL ) *id++ = c;	
						}
						else
						{
							return JSON_UNEXPECTED_CHAR;
						}
						break;
					}
					case READING_STRING : if ( data != NULL ) *data++ = c;	  break;
					default: return JSON_UNEXPECTED_CHAR;
				}
			}
		}
	}
	return JSON_EOF;
}

					
		
			