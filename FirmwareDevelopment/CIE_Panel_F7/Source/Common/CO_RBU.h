/***************************************************************************
* File name: CO_RBU.h
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

#ifndef _CO_RBU_H_
#define _CO_RBU_H_


#include "CO_Site.h"
#include <time.h>


#pragma anon_unions


#define UNIT_DROPPED					10
#define UNIT_UNKNOWN					0
#define UNIT_MESHED					1
#define UNIT_ACTIVE					2
 
#define VALID_PRIMARY				1
#define VALID_MORE_FAULTS			2
#define VALID_NOTIFIED_LINKED		4
#define VALID_ANONYMOUS				8
#define VALID_MORE_WARNINGS		16
#define VALID_SELECTED				32


typedef struct RadioUnit
{
#ifdef __cplusplus 
	void SetStatus( unsigned int parents, unsigned int children,  signed short rssiprimary,
		signed short rssisecondary, unsigned short nodes, unsigned short childnodes, unsigned short rank );

	RadioUnit( ); 
 	RadioUnit( short zone, short unit, int yy, int mm, int bb, int ss, unsigned char comb );
	
 
//	void SetStatus( int parents, unsigned int children, int flags, signed int primary,
//		signed int secondary, unsigned int nodes, unsigned int childnodes, unsigned int rank );
#endif
	
	
	signed char 	state;	
	unsigned char 	combination;
	char				valid;
	signed char 	battery;
	int 				battprimary;
	int				battbackup;
	time_t			starttime;
	short 			unit;
	char  			serial[ 14];
	unsigned int	parents : 2;
	unsigned int	children : 6;
	short				anachannel;
	short				anavalue;
	short          zone;
	signed short 	primary;
	signed short 	secondary;
	signed short 	rssiprimary;
	signed short   rssisecondary;	
	unsigned short	nodes;
	unsigned short childnodes;
	unsigned short rank;

	unsigned short	sent;
	unsigned short	received;
	unsigned int droptime;
	time_t 		lastinfo;

} RadioUnit;

 
 

#endif

