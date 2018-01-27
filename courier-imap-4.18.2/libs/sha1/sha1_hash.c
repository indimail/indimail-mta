/*
** Copyright 2001-2008 Double Precision, Inc.
** See COPYING for distribution information.
*/
#define SHA1_INTERNAL
#include	"sha1.h"
#include	<string.h>


static const char base64tab[]=
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char *sha1_hash(const char *passw)
{
SHA1_DIGEST sha1buf;
static char hash_buffer[1+(sizeof(sha1buf)+2)/3*4];
int	a=0,b=0,c=0;
int	i, j;
int	d, e, f, g;

	sha1_digest(passw, strlen(passw), sha1buf);

	j=0;

	for (i=0; i<sizeof(sha1buf); i += 3)
	{
		a=sha1buf[i];
		b= i+1 < sizeof(sha1buf) ? sha1buf[i+1]:0;
		c= i+2 < sizeof(sha1buf) ? sha1buf[i+2]:0;

		d=base64tab[ a >> 2 ];
		e=base64tab[ ((a & 3 ) << 4) | (b >> 4)];
		f=base64tab[ ((b & 15) << 2) | (c >> 6)];
		g=base64tab[ c & 63 ];
		if (i + 1 >= sizeof(sha1buf))	f='=';
		if (i + 2 >= sizeof(sha1buf)) g='=';
		hash_buffer[j++]=d;
		hash_buffer[j++]=e;
		hash_buffer[j++]=f;
		hash_buffer[j++]=g;
	}

	hash_buffer[j]=0;
	return (hash_buffer);
}

const char *ssha_hash(const char *passw, SSHA_RAND seed)
{
	unsigned char sha1buf[sizeof(SHA1_DIGEST)+sizeof(SSHA_RAND)];

	static char hash_buffer[1+(sizeof(sha1buf)+2)/3*4];
	int	a=0,b=0,c=0;
	int	i, j;
	int	d, e, f, g;
	struct SHA1_CONTEXT ctx;

	sha1_context_init( &ctx );
	sha1_context_hashstream(&ctx, passw, strlen(passw));
	sha1_context_hashstream(&ctx, seed, sizeof(SSHA_RAND));
	sha1_context_endstream(&ctx, strlen(passw)+sizeof(SSHA_RAND));
	sha1_context_digest( &ctx, sha1buf );

	for(i=0; i<sizeof(SSHA_RAND); i++)
	{
		sha1buf[sizeof(SHA1_DIGEST)+i] = seed[i];
	}

	j=0;

	for (i=0; i<sizeof(sha1buf); i += 3)
	{
		a=sha1buf[i];
		b= i+1 < sizeof(sha1buf) ? sha1buf[i+1]:0;
		c= i+2 < sizeof(sha1buf) ? sha1buf[i+2]:0;

		d=base64tab[ a >> 2 ];
		e=base64tab[ ((a & 3 ) << 4) | (b >> 4)];
		f=base64tab[ ((b & 15) << 2) | (c >> 6)];
		g=base64tab[ c & 63 ];
		if (i + 1 >= sizeof(sha1buf))	f='=';
		if (i + 2 >= sizeof(sha1buf)) g='=';
		hash_buffer[j++]=d;
		hash_buffer[j++]=e;
		hash_buffer[j++]=f;
		hash_buffer[j++]=g;
	}

	hash_buffer[j]=0;
	return (hash_buffer);
}
