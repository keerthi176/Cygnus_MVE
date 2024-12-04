 
 
#ifndef _BASE_64_
#define _BASE_64_

#ifdef __cplusplus

extern "C"
{
#endif

	
char* URLEnc( char* in );
 
char* Base64Enc( char * inp, int inlen );
 
char* Base64Dec( const char *inp, int inlen );


#ifdef __cplusplus
}
#endif


#endif

