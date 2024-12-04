
#if 0
class criteria;
	
class event
{
	//virtual bool matches( criteria* ) = 0;
	
	char location[ 40];
	short unit;
	short zone;
};


template<class member> 
class link
{
	member* base;
	
	link* next;
};	
	

class item
{
	void* next;
}
	

template<class member>
class list
{
	int pagesize;
	link<member> start;
	
	int pos;		// current page index
	link<member>* page;		// current page
	
	public:
	
	dblist( int pages, int pagesize ) : pagesize( pagesize )
	{
		int page;
		int m;
		
		link<member>* l = start;
		
		l->base = new member[ pagesize ];
		l->link = null;
		
		for( page = 1; page < pages; page++ )
		{
			l->next = new link<member>;
			l = l->next;
			l->base = new member[ pagesize ];
			l->next = null;
		}	
	}	
};


	
template<class member>
class dblist
{
	int pagesize;
	link<member> start;
	
	int pos;		// current page index
	link<member>* page;		// current page
	
	public:
	
	dblist( int pages, int pagesize ) : pagesize( pagesize )
	{
		int page;
		int m;
		
		link<member>* l = start;
		
		l->base = new member[ pagesize ];
		l->link = null;
		
		for( page = 1; page < pages; page++ )
		{
			l->next = new link<member>;
			l = l->next;
			l->base = new member[ pagesize ];
			l->next = null;
		}	
	}	
};


/*************************************************************************/
/**  \fn      int main( void )
***  \brief   Global helper function
**************************************************************************/

int main( void )
{
	dblist<event>* eventlist = new dblist<event>( 10, 100 );
}

#endif

	