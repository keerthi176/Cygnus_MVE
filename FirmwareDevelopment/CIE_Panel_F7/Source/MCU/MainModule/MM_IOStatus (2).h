/***************************************************************************
* File name: MM_IOStatus.cpp
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
* IO Status
*
**************************************************************************/

#ifndef _MM_IO_STATUS_H_
#define _MM_IO_STATUS_H_


/* System Include Files
**************************************************************************/
#include <list>
#include "rtc.h"


/* User Include Files
**************************************************************************/
#include "MM_Module.h"
#include "MM_Application.h"
#include "MM_Settings.h"
#include "MM_device.h"
#include "MM_IOStatus.h"

/* Defines
**************************************************************************/

extern Application* app;
/*************************************************************************/
/**  \fn      void EwPrint( const char* aFormat, ... )
***  \brief   Global helper function
**************************************************************************/

void EwPrint( const char* aFormat, ... );


/*************************************************************************/
/**  \fn      OutputRef::OutputRef( const Device* d )
***  \brief   Constructor for class
**************************************************************************/

OutputRef::OutputRef( const Device* d ) : devptr( d )
{
	
}


/*************************************************************************/
/**  \fn      void OutputRef::Activate( )
***  \brief   Class function
**************************************************************************/

void OutputRef::Activate( )
{
	
};



struct PanelOutputRef : public OutputRef
{
	PanelOutputRef( const Device* d ) : OutputRef( d ) {}
	
	virtual void Activate( )
	{
		app->DebOut( "Panel IO" );
	}
};


struct ExpansionOutputRef : public OutputRef
{
	ExpansionOutputRef( const Device* d ) : OutputRef( d ) {}
	
	virtual void Activate( )
	{
		app->DebOut( "Expansion IO" );
	}
};


struct MeshOutputRef : public OutputRef
{
	MeshOutputRef( Device* d ) : OutputRef( d ) { }
		
	virtual void Activate( )
	{
		app->DebOut( "Mesh IO" );
	}
};




/*************************************************************************/
/**  \fn      int IOStatus::Receive( Event event )
***  \brief   Class function
**************************************************************************/

int IOStatus::Receive( Event event )
{
	return 0;
}


const Device* InputRef::operator->( )
{
	return devptr;
}

const Device& InputRef::operator*( )
{
	return *devptr;
}

const Device* OutputRef::operator->( )
{
	return devptr;
}

const Device& OutputRef::operator*( )
{
	return *devptr;
}


/*************************************************************************/
/**  \fn      IOStatus::IOStatus( Application* app )
***  \brief   Constructor for class
**************************************************************************/

IOStatus::IOStatus( Application* app ) : Module( "IOStatus", 0, app )
{
	settings = (Settings*) app->FindModule( "Settings" );
	
	PanelOutputRef pan( NULL );
	ExpansionOutputRef exp( NULL );
	MeshOutputRef mesh( NULL );
	
	asserted_outputs = new std::list<OutputRef>;
	
	asserted_outputs->push_back( pan );
	asserted_outputs->push_back( exp );
	asserted_outputs->push_back( mesh );
	
	for ( std::list<OutputRef>::iterator it = asserted_outputs->begin(); it != asserted_outputs->end(); ++it )
	{
		it->Activate( );
	}
		
}


/*************************************************************************/
/**  \fn      int IOStatus::UpdateStatus( )
***  \brief   Class function
**************************************************************************/

int IOStatus::UpdateStatus( )
{
	unsigned int new_status = 0;
	RTC_TimeTypeDef now;
	
	HAL_RTC_GetTime( app->rtc, &now, RTC_FORMAT_BIN );
		
	for ( std::list<InputRef>::iterator it = asserted_inputs->begin(); it != asserted_inputs->end(); ++it )
	{
		new_status |= 1 << (int)(**it).inputAction; 
	}		
	
	int new_events = (status ^ new_status) & new_status;
	
	app->Send( (Event) new_events );		// Send them all at once - why not
	
	status = new_status;
	/*
	const Device* devptr = settings->rom_address->panel.devices;
	
	std::list<OutputRef>* new_outputs = new std::list<OutputRef>;
	
	for( int n = 0; n < settings->rom_address->panel.totalNumDevs; n++ )
	{
		if ( devptr->IsOutput( ) )
		{
			if ( (int) devptr->outputSetting & status )
			{
				PanelOutputRef output( devptr );
			//	new_outputs->push_back( 
			}
		}
	}*/
	return 0;	 
}






#endif

