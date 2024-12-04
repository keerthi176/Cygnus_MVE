
#ifndef _MESSAGE_H_
#define _MESSAGE_H_

class Module;

class Message
{
	public:
		
	Module*	to;
	Module* 	from;
	Message* next;
	int 		type;
};
	
#endif
