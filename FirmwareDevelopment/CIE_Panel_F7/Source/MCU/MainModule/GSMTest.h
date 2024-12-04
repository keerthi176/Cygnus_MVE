



class GSM : public Module public UartHandler
{
	public:
	
	GSM( );
	 
	void UartCommand( UART_HandleTypeDef* uart );
	
	char rxbuffer[ 1000 ];
};

