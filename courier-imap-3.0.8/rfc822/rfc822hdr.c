/*
** Copyright 2001 Double Precision, Inc.
** See COPYING for distribution information.
*/

#include	<stdio.h>
#include	<ctype.h>
#include	<stdlib.h>
#include	<string.h>
#include	"rfc822hdr.h"

static const char rcsid[]="$Id: rfc822hdr.c,v 1.2 2009/11/08 18:14:47 mrsam Exp $";

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

int rfc822hdr_is_addr(const char *hdr)
{
	return strcasecmp(hdr, "from") == 0 ||
		strcasecmp(hdr, "to") == 0 ||
		strcasecmp(hdr, "cc") == 0 ||
		strcasecmp(hdr, "bcc") == 0 ||
		strcasecmp(hdr, "resent-from") == 0 ||
		strcasecmp(hdr, "resent-to") == 0 ||
		strcasecmp(hdr, "resent-cc") == 0 ||
		strcasecmp(hdr, "resent-bcc") == 0;
}
