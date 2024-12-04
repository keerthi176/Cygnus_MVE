





class SiteHandler : public XMLElementHandler
{
	public:
	
	SiteHandler( );
	
	virtual void Begin( );
	virtual void End( );
	
	virtual void Attribute( char* name );
};	
