/***************************************************************************
* File name: MM_XML.cpp
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
* xml encode/decode
*
**************************************************************************/




/* User Include Files
**************************************************************************/
#include "xml.h"



/* Defines
**************************************************************************/
 
#define ALPHA(c) (((c)>='a'&& (c)<='z') || (c)=='-' || (c)=='_' || ((c) >='A' && (c) <= 'Z' ) )
#define ALPHANUM(c) (ALPHA(c) || (c>='0' && c <='9') )
#define WHITE(c) ((c) == ' ' || (c) == 9 )

 
/*************************************************************************/
/**  \fn      int XMLElementHandler::Init( )
***  \brief   Class function
**************************************************************************/

void XMLElementHandler::Init( )
{
	index = 0;
}


/*************************************************************************/
/**  \fn      int ContactHandler::Begin( )
***  \brief   Class function
**************************************************************************/

int XMLElementHandler::Begin( )
{
	data = 0;
	return true;
}


/*************************************************************************/
/**  \fn      int ContactHandler::End( )
***  \brief   Class function
**************************************************************************/

int XMLElementHandler::End( )
{
	return XML_OK;
}


/*************************************************************************/
/**  \fn      int IOStatus::Receive( Command* cmd )
***  \brief   Class function
**************************************************************************/

int XMLElementHandler::Attribute( char*, char* )
{
	return XML_OK;
}


/*************************************************************************/
/**  \fn      int XMLElementHandler::XMLElementHandler( const char* name, XMLList* list )
***  \brief   Class constructor
**************************************************************************/

XMLElementHandler::XMLElementHandler( const char* name, XMLList* list ) : name( name )
{
	assert( list->pos < list->list + MAX_HANDLERS );
	
	*list->pos = this;
	
	list->pos++;
}


/*************************************************************************/
/**  \fn      XMLList::XMLList(  )
***  \brief   Class constructor
**************************************************************************/

XMLList::XMLList( ) : pos( list )
{
	
}


/*************************************************************************/
/**  \fn      XMLParser::XMLParser( const char* name, XMLList* list )
***  \brief   Class constructor
**************************************************************************/

XMLParser::XMLParser(  ) : elementpos( elementbuff ), attribpos( attribbuff ), state( UNKNOWN )
{
	
}


/*************************************************************************/
/**  \fn      XMLParser::Start(  )
***  \brief   Class constructor
**************************************************************************/

void XMLParser::Start( )
{
	*elementname = 0;
	
	for( XMLElementHandler** xmli = lib->list; xmli < lib->pos; xmli++ )
	{
		(*xmli)->Init();
	}
	stack_pos = 0;
	state = UNKNOWN;
}



#define TAB(s) for( int i = 0; i < 4*t; i++ ) app.DebOut( " " ); app.DebOut( s );

/*************************************************************************/
/**  \fn      XMLParser::Parser( char* data, int len  )
***  \brief   Class function
**************************************************************************/

int XMLParser::Parse( char* data, int len  )
{
	int t = 0;
	
	while ( len > 0 )
	{
		char c = *data++;
		len--;
	
		switch ( state )
		{
			case FAILED: break;
			case UNKNOWN:
			{
				if ( c == '<' )
				{
					state = ELEMENT;
					elementpos = elementbuff;
				}
				else if ( ALPHA( c ) )
				{
					app.DebOut( "In unknown: " );
					state = FAILED;
					return XML_UNEXPECTED_DATA;
				}
				else if ( c == '\n' ) line++;
				break;
			}
					
   		case ELEMENT:
			{
				if ( c == '/' )
				{
					if ( elementpos == elementbuff )
						state = END_ELEMENT;
				}
				else if ( ALPHA( c ) )
				{
					*elementpos++ = c;
					
					if ( elementpos >= elementbuff + sizeof(elementbuff) )
					{
						return XML_MISMATCHED_ELEMENT;
					}
					
					state = BEGIN_ELEMENT;
				}
				else if ( c == '\n' ) line++;
				else if ( WHITE( c ) )
				{
				}
				else
				{
					state = FAILED;
					return XML_UNEXPECTED_SYMBOL;
				}
				break;
			}	
			
			case END_ELEMENT:
			{
				if ( ALPHA( c ) )
				{
					*elementpos++ = c;
					if ( elementpos >= elementbuff + sizeof(elementbuff) )
					{
						return XML_MISMATCHED_ELEMENT;
					}
				}
				else if ( c == '>' )
				{
					if ( elementpos > elementbuff )
					{
						char* dot = strrchr( elementname, '.' );
						
						if ( dot != NULL )
						{
							*elementpos = 0;
							elementpos = elementbuff;
							
							if ( !strcmp( dot + 1, elementbuff ) )
							{
								if ( elementhandler[ stack_pos ] != NULL )
								{
									int r = elementhandler[ stack_pos ]->End( );
									
									if ( r != XML_OK ) 
									{
										app.DebOut( "End element '%s': ", elementbuff );
										state = FAILED;
										return r;
									}
								}
								t--;
								TAB( "</" );
								app.DebOut( elementname );
								app.DebOut( ">\n" );
								
								*dot = 0;
								
								state = UNKNOWN;
							}
							else
							{
								app.DebOut( "End Element '%': ", elementbuff );
								state = FAILED;
								return XML_MISMATCHED_ELEMENT;
							}
						}
						stack_pos--;
						
						if ( stack_pos < 0 ) return XML_MISMATCHED_ELEMENT;
					}
					else
					{
						app.DebOut( "End Element: " );
						state = FAILED;
						return XML_EMPTY_ELEMENT;
					}
				}
				break;
			}
							
			case BEGIN_ELEMENT :
			{
				if ( c == '>' || c == ' ' )
				{
					*elementpos = 0;
					
					elementpos = elementbuff;
					
					STRNCAT( elementname, ".", sizeof(elementname) ); 
					STRNCAT( elementname, elementbuff, sizeof(elementname) );
					
					TAB( "<" );
					app.DebOut( elementname );
					app.DebOut( ">\n" );
					t++;
					
					if ( stack_pos >= STACK_SIZE - 1 )
					{
						return XML_MISMATCHED_ELEMENT;
					}
					
					elementhandler[ ++stack_pos ] = NULL;
					
					for( XMLElementHandler** i = lib->list; i < lib->pos; i++ )
					{
						if ( !strcmp( (*i)->name, elementname ) ) 
						{
							elementhandler[ stack_pos ] = *i;
							
							if ( !elementhandler[ stack_pos ]->Begin( ) )
							{
								app.DebOut( "begin element '%' :", elementname );
								state = FAILED;
								return XML_INIT_FAIL;
							}
							
							break;
						}
					}
					if ( c == '>' ) 
					{
						state = UNKNOWN;
					}
					else // =space
					{
						state = ATTRIB;
						attribpos = attribbuff;
					}
				}
				else if ( ALPHANUM(c) )
				{
					*elementpos++ = c;
					
					if ( elementpos >= elementbuff + sizeof(elementbuff) )
					{
						return XML_MISMATCHED_ELEMENT;
					}
				}
				if ( c == '\n' ) line++;
				break;
			}
			
			case ATTRIBVALUE :
			{
				if ( c == '"' || c == '\'' )
				{
					if ( startchr == 0 )
					{
						startchr = c;
					}
					else if ( startchr == c )
					{
						if ( attribpos > attribbuff )
						{
							*valuepos = 0;
							
							TAB( attribbuff );
							app.DebOut( "=" );
							app.DebOut( valuebuff );
							app.DebOut( "\n" );
							
							if ( elementhandler[ stack_pos ] != NULL )
							{
								int r = elementhandler[ stack_pos ]->Attribute( attribbuff, valuebuff );
								
								if ( r != XML_OK ) 
								{
									state = FAILED;
									app.DebOut( "Attrib value '%'='%s': ", attribbuff, valuebuff );
									
									return r;
								}
							}
							
							attribpos = attribbuff;
							valuepos  = valuebuff;
						}
						state = ATTRIB;
					}
						
				}
				else if ( c == '>' )
				{
					state = UNKNOWN;
				}
				else if ( startchr != 0 )
				{
					*valuepos++ = c;
					if ( valuepos >= valuebuff + sizeof( valuebuff ) )
					{
						return XML_BAD_VALUE;
					}
				}
				break;
			}				
			
			case ATTRIB :
			{
				if ( c == '=' )
				{
					*attribpos = 0;
					state = ATTRIBVALUE;
					startchr = 0;
					valuepos = valuebuff;
				}
				
				else if ( c == '>' )
				{
					state = UNKNOWN;
				}
				else if ( ALPHANUM( c ) )
				{		
					*attribpos++ = c;
					if ( attribpos >= attribbuff + sizeof( attribbuff ) )
					{
						return XML_BAD_VALUE;
					}
				}
				if ( c == '\n' ) line++;
			}
		}
	}
	return 0;
}

	
