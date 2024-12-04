/***************************************************************************
* File name: CO_RBU.cpp
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
* RBU related structures
*
**************************************************************************/

#include <stdio.h>
#include "CO_RBU.h"
#include "limits.h"

#include "MM_Application.h"
#include "MM_CUtils.h"


/* Defines
**************************************************************************/


/*************************************************************************/
/**  \fn      RadioUnit::RadioUnit( )
***  \brief   Constructor for class
**************************************************************************/

RadioUnit::RadioUnit( )
{
   lastinfo = now( );
}


/*************************************************************************/
/**  \fn      RadioUnit::RadioUnit( int zone, int unit, int yy, int mm, int bb, int ss, int comb, int flags )
***  \brief   Constructor for class
**************************************************************************/

RadioUnit::RadioUnit( short zone, short unit, int yy, int mm, int bb, int ss, unsigned char comb ) :
				state( UNIT_UNKNOWN ), combination( comb ), valid( 0 ), battery( -1 ), battprimary( 0 ), battbackup( 0 ), starttime( 0 ), unit( unit ),
				zone( zone ), primary( -1 ), secondary( -1 ), rssiprimary( 0 ), rssisecondary( 0 ), sent( 0 ), received( 0 ), droptime( UINT_MAX )
{
	sprintf( serial, "%02d%02d-%02d-%05d", yy, mm, bb, ss );	
	lastinfo = now( );
}


/*************************************************************************/
/**  \fn      void RadioUnit::SetStatus( int parents, unsigned int children, int flags, signed int primary,
		signed int secondary, unsigned int nodes, unsigned int childnodes, unsigned int rank )
***  \brief   Class function
**************************************************************************/

void RadioUnit::SetStatus( unsigned int _parents, unsigned int _children, signed short _rssiprimary,
		signed short _rssisecondary, unsigned short _nodes, unsigned short _childnodes, unsigned short _rank  )
{
	parents	 		= _parents;
	children 		= _children;
	rssiprimary		= _rssiprimary;
	rssisecondary	= _rssisecondary;
	nodes				= _nodes;
	childnodes		= _childnodes;
	rank				= _rank;
}




