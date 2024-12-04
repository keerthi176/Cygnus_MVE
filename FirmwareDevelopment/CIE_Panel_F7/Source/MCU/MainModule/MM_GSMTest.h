

#include "MM_Module.h"
#include "DM_UartHandler.h"


#define BUFF_LEN 1024

class GSMTest : public Module, public UartHandler
{
	public:
	
	GSMTest( );
	 
	virtual void UartCommand( UART_HandleTypeDef* uart );
	
	char rxbuffer[ BUFF_LEN ];
};

