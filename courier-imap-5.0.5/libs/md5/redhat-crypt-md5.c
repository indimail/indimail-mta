/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

#define	MD5_INTERNAL
#include	"md5.h"
#include	<string.h>


static char base64[]=
        "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

char *md5_crypt_redhat(const char *pw, const char *salt)
{
struct MD5_CONTEXT outer_context, inner_context;
MD5_DIGEST digest;
unsigned pwl=strlen(pw);
unsigned l;
unsigned i, j;
char	*p;
static char buffer[100];

	if (*salt == '$' && salt[1] == '1' && salt[2] == '$')
		salt += 3;

	for (l=0; l<8 && salt[l] && salt[l] != '$'; l++)
		;

	md5_context_init(&inner_context);
	md5_context_hashstream(&inner_context, pw, pwl);
	md5_context_hashstream(&inner_context, salt, l);
	md5_context_hashstream(&inner_context, pw, pwl);
	md5_context_endstream(&inner_context, pwl*2+l);
	md5_context_digest(&inner_context, digest);

	md5_context_init(&outer_context);
	md5_context_hashstream(&outer_context, pw, pwl);
	md5_context_hashstream(&outer_context, "$1$", 3);
	md5_context_hashstream(&outer_context, salt, l);

	for (i=pwl; i; )
	{
		j=i;
		if (j > 16)	j=16;
		md5_context_hashstream(&outer_context, digest, j);
		i -= j;
	}

	j=pwl*2+l+3;

	for (i=pwl; i; i >>= 1)
	{
		md5_context_hashstream(&outer_context, (i & 1) ? "": pw, 1);
		++j;
	}


	md5_context_endstream(&outer_context, j);
	md5_context_digest(&outer_context, digest);

	for (i=0; i<1000; i++)
	{
		j=0;

		md5_context_init(&outer_context);
		if (i & 1)
		{
			md5_context_hashstream(&outer_context, pw, pwl);
			j += pwl;
		}
		else
		{
			md5_context_hashstream(&outer_context, digest, 16);
			j += 16;
		}
    
		if (i % 3)
		{
			md5_context_hashstream(&outer_context, salt, l);
			j += l;
		}
    
    		if (i % 7)
		{
			md5_context_hashstream(&outer_context, pw, pwl);
			j += pwl;
		}
    
		if (i & 1)
		{
			md5_context_hashstream(&outer_context, digest, 16);
			j += 16;
		}
		else
		{
			md5_context_hashstream(&outer_context, pw, pwl);
			j += pwl;
		}

		md5_context_endstream(&outer_context, j);
		md5_context_digest(&outer_context, digest);
	}

	strcpy(buffer, "$1$");
	strncat(buffer, salt, l);
	strcat(buffer, "$");

	p=buffer+strlen(buffer);
	for (i=0; i<5; i++)
	{
	unsigned char *d=digest;

		j= (d[i] << 16) | (d[i+6] << 8) | d[i == 4 ? 5:12+i];
		*p++= base64[j & 63] ; j=j >> 6;
		*p++= base64[j & 63] ; j=j >> 6;
		*p++= base64[j & 63] ; j=j >> 6;
		*p++= base64[j & 63];
	}
	j=digest[11];
	*p++ = base64[j & 63]; j=j >> 6;
	*p++ = base64[j & 63];
	*p=0;
	return (buffer);
}
