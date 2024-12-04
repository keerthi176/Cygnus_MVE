
#ifndef _TEST_GSM_
#define _TEST_GSM_

 

class TestGSM : public Module//, public UartHandler
{
	public:
	
	TestGSM( );
	
	virtual int Init( );
	virtual int Receive( Event event );
	virtual int Receive( Command* );
	virtual int Receive( Message* ); 
	
	virtual void Poll( ); 
	
	void Reset( );
	
	//MQTTSocket mqtt_socket;
};



#endif

