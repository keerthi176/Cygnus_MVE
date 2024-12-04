

class BitArray
{
	BitArray( int size );
	BitArray( );
	
	int operator [] ( int size );
	char& operator [] ( int size );
	
	operator = (int n )
	
	unsigned char* data;
};


BitArray::BitArray( int size )
{
	data = new unsigned char [ ( size + 7 ) / 8 ];
}



int BitArray::operator [] ( int size )
{
	return 
	