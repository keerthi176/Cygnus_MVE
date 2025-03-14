/***************************************************************************
* File name: MM_Message.h
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
* Message definition
*
**************************************************************************/

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

class Module;


class Message
{
	public:
		
	Module*	to;
	Module* 	from;
	int 		type;
	int 		value;
};
	
#endif
