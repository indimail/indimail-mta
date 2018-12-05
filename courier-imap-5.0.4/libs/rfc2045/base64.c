/*
** Copyright 2002 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include "rfc2045_config.h"
#include "base64.h"

#include <string.h>
#include <stdio.h>

void base64_decode_init(struct base64decode *b,
			int (*f)(const char *, int, void *),
			void *a)
{
	b->workbuflen=0;
	b->decode_func=f;
	b->decode_func_arg=a;
}

static int doflush(struct base64decode *);

static const char base64tab[]=
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64_decode(struct base64decode *b, const char *p, int l)
{
	while (l)
	{
		char c;

		if (b->workbuflen >= sizeof(b->workbuf))
		{
			int rc=doflush(b);

			if (rc)
				return (rc);
		}


		if (*p == '=')
		{
			c=100;
		}
		else
		{
			char *s=strchr(base64tab, *p);

			if (s == NULL)
			{
				++p;
				--l;
				continue;
			}

			c= s-base64tab;
		}
		b->workbuf[b->workbuflen++]=c;
		++p;
		--l;
	}
	return (0);
}

int base64_decode_end(struct base64decode *b)
{
	return (doflush(b));
}

static int doflush(struct base64decode *p)
{
	int i=p->workbuflen / 4;
	int j;
	int k=0;

	i= i * 4;

	for (j=0; j<i; j += 4)
	{
		char	a,b,c;

		int	w=p->workbuf[j];
		int	x=p->workbuf[j+1];
		int	y=p->workbuf[j+2];
		int	z=p->workbuf[j+3];

		a= (w << 2) | (x >> 4);
		b= (x << 4) | (y >> 2);
		c= (y << 6) | z;
		p->workbuf[k++]=a;
		if ( y != 100)
			p->workbuf[k++]=b;
		if ( z != 100)
			p->workbuf[k++]=c;
	}

	j= (*p->decode_func)(p->workbuf, k, p->decode_func_arg);

	k=0;
	while (i < p->workbuflen)
	{
		p->workbuf[k]=p->workbuf[i];
		++k;
		++i;
	}
	p->workbuflen=k;
	return (j);
}

/* ---- */

static int save_str(const char *p, int l, void *vp)
{
	memcpy(*(char **)vp, p, l);

	*(char **)vp += l;
	return (0);
}

char *base64_decode_str(const char *s)
{
	struct base64decode b;

	char *p=strdup(s);
	char *pp;

	if (!p)
		return (NULL);

	pp= p;

	base64_decode_init(&b, save_str, &pp);
	base64_decode(&b, s, strlen(s));
	base64_decode_end(&b);
	*pp=0;
	return p;
}
