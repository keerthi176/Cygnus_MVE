
/***************************************************************************
* File name: DM_Handlers.h
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
* First written on 18/05/19 by Matt Ault
*
* File History:
* Change History:
* Rev Date Description Author
* 1.0 18/05/18 Initial File
*
* Description:
* Handler list management
*
*************************************************************************/



#ifndef _DM_XXX_
#define _DM_XXX_




template<class HALDevice, class HALDeviceHandler>
struct HandlerItem
{
	HALDevice* 				device;
	HALDeviceHandler*		handler;
};  


template<class HALDevice, class HALDeviceHandler, int maxdevices>
struct HandlerManager
{		
	HandlerItem<HALDevice, HALDeviceHandler> list[ maxdevices ];
	HandlerItem<HALDevice, HALDeviceHandler>* pos;

	HandlerManager( ) : pos( list ) { };
	
	void Add( HALDevice* device, HALDeviceHandler* handler )
	{
		pos->device  = device;
		pos->handler = handler;
		pos++;
	}
		
	HALDeviceHandler* Get( HALDevice* device )
	{
		for( HandlerItem<HALDevice, HALDeviceHandler>* i = list; i < pos; i++ )
		{
			if ( i->device == device ) return i->handler;
		}
		return (HALDeviceHandler*)0;
	}
};
	

#define HANDLER( Static, handler, func, device ) void handler::Static##func( device* d ) { manager.Get( d )->func( d ); }

#endif
