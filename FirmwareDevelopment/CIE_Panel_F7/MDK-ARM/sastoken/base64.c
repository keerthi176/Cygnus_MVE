
#include <stdint.h>
#include <string.h>


char hex( char d )
{
	d = d & 15;
	if ( d > 9 ) return 55 + d;
	return d + '0';
}
	
char* URLEnc( char* in )
{
	static char out[ 512 ];
	char* o = out;
	
	while ( *in )
	{
		char i = *in | 32;
		if ( (i >= 'a' && i <= 'z') || (i >= '0' && i <= '9' ) )
		{
			*o++ = *in;
		}
		else
		{
			*o++ = '%';
			*o++ = hex( *in >> 4);
			*o++ = hex( *in );
		}
		in++;
	}
	return out;
}
			

static const char base64[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

typedef struct  {
	uint32_t bs_bits;
	int bs_offs;
} b64state;

	
void b64flush( b64state *bs, unsigned char *outp )
{
	 
	if (bs->bs_offs == 8) {
		*outp++ = base64[(bs->bs_bits >> 2) & 0x3F];
		*outp++ = base64[(bs->bs_bits << 4) & 0x3F];
		*outp++ = '=';
		*outp++ = '=';
	} else if (bs->bs_offs == 16) {
		*outp++ = base64[(bs->bs_bits >> 10) & 0x3F];
		*outp++ = base64[(bs->bs_bits >> 4) & 0x3F];
		*outp++ = base64[(bs->bs_bits << 2) & 0x3F];
		*outp++ = '=';
	}
	bs->bs_offs = 0;
	bs->bs_bits = 0;
	
}


int b64enc( b64state *bs, unsigned char * inp, int inlen, unsigned char * outp )
{
	int outlen = 0;

	while (inlen > 0) {
		bs->bs_bits = (bs->bs_bits << 8) | *inp++;
		inlen--;
		bs->bs_offs += 8;
		if (bs->bs_offs >= 24) {
			*outp++ = base64[(bs->bs_bits >> 18) & 0x3F];
			*outp++ = base64[(bs->bs_bits >> 12) & 0x3F];
			*outp++ = base64[(bs->bs_bits >> 6) & 0x3F];
			*outp++ = base64[bs->bs_bits & 0x3F];
			outlen += 4;
			bs->bs_offs = 0;
			bs->bs_bits = 0;
		}
	}
	 b64flush( bs, outp );
	return (outlen);
}



char* Base64Enc( char * inp, int inlen )
{
	b64state bs;
	static char outp[ 512];
	
	bs.bs_bits = 0;
	bs.bs_offs = 0;
	
	b64enc( &bs,(unsigned char*) inp, inlen, (unsigned char*) outp );
	
	return outp;
}
	



int b64dec( b64state * bs, unsigned char *inp, int inlen, unsigned char * outp )
{
	int outlen = 0;
	char *cp;

	while (inlen > 0)
	{
		if (( cp = strchr(base64, *inp++)) == NULL )
			break;
		bs->bs_bits = (bs->bs_bits << 6) | (cp - base64);
		inlen--;
		bs->bs_offs += 6;
		if (bs->bs_offs >= 8) {
			*outp++ = bs->bs_bits >> (bs->bs_offs - 8);
			outlen++;
			bs->bs_offs -= 8;
		}
	}
	*outp = 0;
	return (outlen);
}

char* Base64Dec( const char *inp, int inlen )
{
	static char out[ 512 ];
	b64state bs;
	bs.bs_bits = 0;
	bs.bs_offs = 0;
	
	b64dec( &bs, (unsigned char*) inp, inlen, (unsigned char*) out );
	
	
	return out;
}


