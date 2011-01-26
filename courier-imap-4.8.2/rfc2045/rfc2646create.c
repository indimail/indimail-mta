/*
** Copyright 2000-2011 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include "rfc2045_config.h"
#include "../unicode/unicode.h"
#include	"rfc2646.h"
#include	<stdlib.h>
#include	<string.h>

static const char rcsid[]="$Id: rfc2646create.c,v 1.7 2011/01/15 16:14:37 mrsam Exp $";

struct rfc2646create *rfc2646create_alloc( int (*f)(const char *,
						    size_t,
						    void *),
					   const char *charset,
					   void *vp)
{
	struct rfc2646create *p=calloc(1, sizeof(struct rfc2646create));

	if (!p)
		return (NULL);

	p->handler=f;
	p->voidarg=vp;

	p->charset=charset;
	p->linesize=76;
	p->sent_firsttime=1;
	return (p);
}

static int wordwrap_line(unicode_char *, size_t, size_t,
			 struct rfc2646create *);

static int ismatch(const unicode_char *uc,
		   const char *cp,
		   size_t cnt)
{
	while (cnt)
	{
		if (*uc != *cp)
			return -1;

		++uc;
		++cp;
		--cnt;
	}

	return 0;
}

static int wordwrap_sent(unicode_char *buf,
			 const char *cpbuf,
			 struct rfc2646create *rfcptr)
{
	size_t	i;
	int quote_depth=0;
	int rc;
	size_t cnt;

	for (cnt=0; buf[cnt]; ++cnt)
		;

	for (i=0; i<cnt; i++)
	{
		if (buf[i] != '>')
			break;
		++quote_depth;
	}

	if (i < cnt && buf[i] == ' ')
		++i;

	rc=0;

	/* A flowed line, followed by empty unflowed line, is a paragraph
	** break.
	*/
	if (rfcptr->has_sent_paragraph && i >= cnt &&
	    rfcptr->last_sent_quotelevel == quote_depth)
	{
		rc=(*rfcptr->handler)(" \n", 2, rfcptr->voidarg);
		rfcptr->has_sent_paragraph=0;
	}
	else
	{
		if (!rfcptr->sent_firsttime)
			rc=(*rfcptr->handler)("\n", 1, rfcptr->voidarg);
		rfcptr->has_sent_paragraph=1;
	}

	rfcptr->sent_firsttime=0;
	rfcptr->last_sent_quotelevel=quote_depth;

	if (rc)
		return (rc);

	if (quote_depth)	/* Already wrapped */
	{
		return ((*rfcptr->handler)(cpbuf, strlen(cpbuf),
					   rfcptr->voidarg));
	}

	while (cnt > i && buf[cnt-1] == ' ')
	{
		if (cnt - i == 3 && ismatch(buf+i, "-- ", 3) == 0)
			break;
		--cnt;
	}

	while (i < cnt)
	{
		size_t j;

		size_t k;
		size_t w=0;

		int found_spc=0;
		size_t spc_index=0;
		int no_spc=0;
		const char *spnl;

		for (k=i; ; k++)
		{
			if (k >= cnt)
			{
				rc=wordwrap_line(buf, cnt, i, rfcptr);
				return rc;
			}

			if (w >= rfcptr->linesize)
				break;

			if (buf[k] == ' ')
			{
				found_spc=1;
				spc_index=k;
			}
			w += unicode_wcwidth(buf[k]);
		}

		if (!found_spc)
		{
			spc_index=k;
			no_spc=1;
		}

		j=spc_index;

		rc=wordwrap_line(buf, j, i, rfcptr);

		if (j < cnt && buf[j] == ' ')
			++j;

		i=j;

		spnl=" \n";

		rc=(*rfcptr->handler)(spnl + no_spc, 2 - no_spc,
				      rfcptr->voidarg);
		if (rc)
			break;
	}
	return (rc);
}

static int wordwrap_line(unicode_char *buf,
			 size_t cnt, size_t i,
			 struct rfc2646create *rfcptr)
{
	int rc=0;

	if ((cnt - i >= 5 && ismatch(buf+i, "From ", 5) == 0) ||
	    (cnt > i && buf[i] == '-'
	     && (cnt - i != 3 || ismatch(buf+i, "-- ", 3))))
		rc=(*rfcptr->handler)(" ", 1, rfcptr->voidarg);

	if (rc == 0)
	{
		char *cp;
		size_t cplen;
		libmail_u_convert_handle_t h;

		buf += i;
		cnt -= i;

		h=libmail_u_convert_fromu_init(rfcptr->charset, &cp,
					       &cplen, 1);
		if (h)
		{
			libmail_u_convert_uc(h, buf, cnt);
			libmail_u_convert_deinit(h, NULL);
		}
		else
			cp=NULL;

		if (cp)
		{
			rc=(*rfcptr->handler)(cp, strlen(cp), rfcptr->voidarg);
			free(cp);
		}
	}

	return (rc);
}

int rfc2646create_parse(struct rfc2646create *rfcptr,
			const char *str, size_t strcnt)
{
	char *ptr, *q;
	size_t cnt;
	int rc;

	if (strcnt + rfcptr->buflen > rfcptr->bufsize)
	{
		size_t l=strcnt + rfcptr->buflen + 256;
		char *newbuf= rfcptr->buffer
			? (char *)realloc(rfcptr->buffer,
				  l * sizeof(*rfcptr->buffer))
			: (char *)malloc(l * sizeof(*rfcptr->buffer));

		if (!newbuf)
			return (-1);

		rfcptr->buffer=newbuf;
		rfcptr->bufsize=l;
	}

	if (strcnt)
		memcpy(rfcptr->buffer + rfcptr->buflen, str,
		       strcnt * sizeof(*str));

	rfcptr->buflen += strcnt;

	ptr=rfcptr->buffer;
	cnt=rfcptr->buflen;

	rc=0;
	for (;;)
	{
		unicode_char *uc;
		size_t uclen;
		size_t i;
		libmail_u_convert_handle_t h;

		for (i=0; i<cnt; i++)
			if (ptr[i] == '\n')
				break;
		if (i >= cnt)	break;

		h=libmail_u_convert_tou_init(rfcptr->charset, &uc, &uclen, 1);

		if (h)
		{
			libmail_u_convert(h, ptr, i);
			libmail_u_convert_deinit(h, NULL);
		}
		else
			uc=NULL;

		ptr[i]=0;
		if (uc)
		{
			rc=wordwrap_sent(uc, ptr, rfcptr);
			free(uc);
		}
		ptr[i]='\n';
		if (rc)
			break;
		++i;
		ptr += i;
		cnt -= i;
	}
	q=rfcptr->buffer;
	rfcptr->buflen=cnt;
	while (cnt)
	{
		*q++ = *ptr++;
		--cnt;
	}
	return (rc);
}

int rfc2646create_free(struct rfc2646create *rfcptr)
{
	int rc=0;

	if (rfcptr->buflen)
		rc=rfc2646create_parse(rfcptr, "\n", 1);

	if (rfcptr->buffer)
	{
		if (rc == 0)
			rc=(*rfcptr->handler)("\n", 1, rfcptr->voidarg);
		free(rfcptr->buffer);
	}
	free(rfcptr);
	return (rc);
}
