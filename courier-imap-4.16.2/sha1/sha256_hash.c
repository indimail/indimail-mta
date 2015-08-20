/*
** Copyright 2005 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"sha1.h"
#include	<string.h>


static const char base64tab[]=
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char *sha256_hash(const char *passw)
{
SHA256_DIGEST sha256buf;
static char hash_buffer[1+(sizeof(sha256buf)+2)/3*4];
int	a=0,b=0,c=0;
int	i, j;
int	d, e, f, g;

	sha256_digest(passw, strlen(passw), sha256buf);

	j=0;

	for (i=0; i<sizeof(sha256buf); i += 3)
	{
		a=sha256buf[i];
		b= i+1 < sizeof(sha256buf) ? sha256buf[i+1]:0;
		c= i+2 < sizeof(sha256buf) ? sha256buf[i+2]:0;

		d=base64tab[ a >> 2 ];
		e=base64tab[ ((a & 3 ) << 4) | (b >> 4)];
		f=base64tab[ ((b & 15) << 2) | (c >> 6)];
		g=base64tab[ c & 63 ];
		if (i + 1 >= sizeof(sha256buf))	f='=';
		if (i + 2 >= sizeof(sha256buf)) g='=';
		hash_buffer[j++]=d;
		hash_buffer[j++]=e;
		hash_buffer[j++]=f;
		hash_buffer[j++]=g;
	}

	hash_buffer[j]=0;
	return (hash_buffer);
}
