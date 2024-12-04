/***************************************************************************
* File name: MM_RadioDevice.h
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
* Input and Output radio device wrapper classes
*
**************************************************************************/

#ifndef _MM_RADIO_DEVICE_H_
#define _MM_RADIO_DEVICE_H_


/* User Include Files
**************************************************************************/
#include "MM_Device.h"
#include "CO_RBU.h"
#include "MM_List.h"


/* Defines
**************************************************************************/




class NCU;



class RadioDevice : public Device
{
	public:
	
	RadioDevice( RadioUnit*, DeviceConfig* conf, Module* owner );

	virtual void OutputChange( int channel, int action, int value );
	
	virtual int Broadcasted( );	
	virtual int Broadcast( int silenceable_status, int unsilenceable_status, int skip_bits );	
	
	virtual void Indicate( int asserted, int prof );
 
	
	
	void SetBattery( int val );
	
};



/*

class RadioOutputDevice : public OutputDevice
{
	public:
	
	RadioOutputDevice( RadioUnit*, DeviceConfig* conf, NCU* );
	
//	virtual int Actions( int channel );
	
	virtual void Assert( int channel, int action );
	virtual void DeAssert( int channel );

	virtual unsigned int FirstDelay( int channel );
	virtual unsigned int SecondDelay( int channel );
	
	virtual int IsInverted( int channel );
	virtual int IsSilenceable( int channel );
};
*/

#endif

