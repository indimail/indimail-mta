/*
** Copyright 2001-2011 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	"config.h"
#include	<stdio.h>
#include	<ctype.h>
#include	<stdlib.h>
#include	<string.h>
#include	"rfc822hdr.h"


/*
** Read the next mail header.
*/

int rfc822hdr_read(struct rfc822hdr *h, FILE *f, off_t *pos, off_t epos)
{
	size_t n=0;
	int c;

	for (;;)
	{
		if ( n >= h->hdrsize)
		{
			size_t hn=h->hdrsize + 1024;
			char *p= h->header ? realloc(h->header, hn):
				malloc(hn);

			if (!p)
				return (-1);

			h->header=p;
			h->hdrsize=hn;
		}

		if (pos && *pos >= epos)
		{
			h->header[n]=0;
			break;
		}

		c=getc(f);
		if (c == EOF)
		{
			if (pos)
				*pos=epos;
			h->header[n]=0;
			break;
		}
		if (pos)
			++*pos;

		h->header[n]=c;
		if (c == '\n')
		{
			if (n == 0)
			{
				if (pos)
					*pos=epos;
				h->header[n]=0;
				break;
			}

			c=getc(f);
			if (c != EOF)
				ungetc(c, f);
			if (c == '\n' || c == '\r' ||
			    !isspace((int)(unsigned char)c))
			{
				h->header[n]=0;
				break;
			}
		}
		n++;
		if (h->maxsize && n + 2 > h->maxsize)
			--n;
	}

	if (n == 0)
	{
		if (pos)
			*pos=epos;
		h->value=h->header;
		return (1);
	}

	for (h->value=h->header; *h->value; ++h->value)
	{
		if (*h->value == ':')
		{
			*h->value++=0;
			while (*h->value &&
			       isspace((int)(unsigned char)*h->value))
				++h->value;
			break;
		}
	}
	return (0);
}

void rfc822hdr_fixname(struct rfc822hdr *h)
{
	char *p;

	for (p=h->header; *p; p++)
	{
		*p=tolower((int)(unsigned char)*p);
	}
}

void rfc822hdr_collapse(struct rfc822hdr *h)
{
	char *p, *q;

	for (p=q=h->value; *p; )
	{
		if (*p == '\n')
		{
			while (*p && isspace((int)(unsigned char)*p))
				++p;
			*q++=' ';
			continue;
		}
		*q++ = *p++;
	}
	*q=0;
}

/* This is, basically, a case-insensitive US-ASCII comparison function */

#define lc(x) ((x) >= 'A' && (x) <= 'Z' ? (x) + ('a'-'A'):(x))

int rfc822hdr_namecmp(const char *a, const char *b)
{
	int rc;

	while ((rc=(int)(unsigned char)lc(*a) - (int)(unsigned char)lc(*b))==0)
	{
		if (!*a)
			return 0;
		++a;
		++b;
	}

	return rc;
}

int rfc822hdr_is_addr(const char *hdr)
{
	return rfc822hdr_namecmp(hdr, "from") == 0 ||
		rfc822hdr_namecmp(hdr, "to") == 0 ||
		rfc822hdr_namecmp(hdr, "cc") == 0 ||
		rfc822hdr_namecmp(hdr, "bcc") == 0 ||
		rfc822hdr_namecmp(hdr, "resent-from") == 0 ||
		rfc822hdr_namecmp(hdr, "resent-to") == 0 ||
		rfc822hdr_namecmp(hdr, "resent-cc") == 0 ||
		rfc822hdr_namecmp(hdr, "resent-bcc") == 0;
}
