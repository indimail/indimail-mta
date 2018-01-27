#include	<stdlib.h>

static int decode64tab_init=0;
static char decode64tab[256];

/*
** Copyright 1998 - 1999 Double Precision, Inc.
** See COPYING for distribution information.
*/

int authsasl_frombase64(char *base64buf)
{
int	i, j, k;

	if (!decode64tab_init)
	{
		for (i=0; i<256; i++)	decode64tab[i]=100;
		for (i=0; i<64; i++)
			decode64tab[ (int)
				("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i])				]=i;
		decode64tab_init=1;
	}

	for (j=0; base64buf[j]; j++)
		if (decode64tab[(unsigned char)base64buf[j]] >= 100)
			break;

	if (base64buf[j] && base64buf[j+1] && base64buf[j+2])
		return (-1);
	while (base64buf[j] == '=')	++j;
	if (j % 4)	return (-1);

	i=j;
	k=0;
	for (j=0; j<i; j += 4)
	{
	int	w=decode64tab[(int)(unsigned char)base64buf[j]];
	int	x=decode64tab[(int)(unsigned char)base64buf[j+1]];
	int	y=decode64tab[(int)(unsigned char)base64buf[j+2]];
	int	z=decode64tab[(int)(unsigned char)base64buf[j+3]];
	int	a,b,c;

		a= (w << 2) | (x >> 4);
		b= (x << 4) | (y >> 2);
		c= (y << 6) | z;
		base64buf[k++]=a;
		if ( base64buf[j+2] != '=')
			base64buf[k++]=b;
		if ( base64buf[j+3] != '=')
			base64buf[k++]=c;
	}
	return (k);
}
