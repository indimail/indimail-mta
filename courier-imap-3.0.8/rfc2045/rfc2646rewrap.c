/*
** Copyright 2000 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include "rfc2045_config.h"
#include	"rfc2646.h"
#include	<stdlib.h>
#include	<string.h>

static const char rcsid[]="$Id: rfc2646rewrap.c,v 1.3 2003/03/07 00:47:31 mrsam Exp $";

struct rfc2646rewrap *rfc2646rewrap_alloc( size_t w,
					   int (*f)(struct rfc2646rewrap *,
						    void *),
					   void *a)
{
	struct rfc2646rewrap *p=(struct rfc2646rewrap *)
		calloc(1, sizeof(struct rfc2646rewrap));

	if (!p)
		return (0);

	if (w < 20)
		w=20;

	p->wrap_width=w;
	p->handler=f;
	p->voidarg=a;

	if ((p->wrap_buf=malloc(w+1)) == 0)
	{
		free(p);
		return (0);
	}
	return (p);
}

int rfc2646rewrap_free(struct rfc2646rewrap *p)
{
	int rc=0;

	if (p->has_prev)
	{
		p->wrap_buf[p->wrap_buflen]=0;
		rc= (*p->handler)(p, p->voidarg);
	}
	free(p->wrap_buf);
	free(p);
	return (rc);
}

int rfc2646rewrap_handler(struct rfc2646parser *p, int isflowed, void *vp)
{
	struct rfc2646rewrap *r=(struct rfc2646rewrap *)vp;
	int rc;
	const char *ptr;
	size_t len;
	int needspc;

	size_t w;
	
	if (r->has_prev)
	{
		if (r->quote_depth != p->quote_depth)
		{
			r->wrap_buf[r->wrap_buflen]=0;
			rc=(*r->handler)(r, r->voidarg);
			if (rc)
				return (rc);
			r->wrap_buflen=0;
			r->has_prev=0;
		}
		else if (!isflowed && p->linelen == 0)
		{
			strcpy(r->wrap_buf + r->wrap_buflen, " ");
			rc=(*r->handler)(r, r->voidarg);
			if (rc)
				return (rc);
			r->wrap_buflen=0;
			r->has_prev=0;
		}
	}
	else
	{
		r->wrap_buflen=0;
	}
	r->quote_depth=p->quote_depth;

	ptr=p->line;
	len=p->linelen;
	needspc=r->has_prev;

	w=r->wrap_width-2;
	if (r->quote_depth + 20 < r->wrap_width)
	{
		w -= r->quote_depth;
		--w;
	}

	if (!r->has_prev && p->linelen == 3 && strncmp(p->line, "-- ", 3) == 0)
	{
		strcpy(r->wrap_buf, "-- ");
		return ((*r->handler)(r, r->voidarg));
	}

	while (len)
	{
		size_t i;

		for (i=0; i<len; i++)
			if (ptr[i] == ' ')
			{
				while (i < len && ptr[i] == ' ')
					++i;
				--i;
				break;
			}

		if (r->wrap_buflen + needspc + i < w)
		{
			if (needspc)
				r->wrap_buf[r->wrap_buflen++]=' ';
			while (i)
			{
				r->wrap_buf[r->wrap_buflen++]= *ptr++;
				--len;
				--i;
			}
			if (len)
			{
				--len;
				++ptr;
			}
			needspc=1;
			continue;
		}

		if (r->wrap_buflen > 0)
		{
			while (r->wrap_buflen > 0 &&
			       r->wrap_buf[r->wrap_buflen-1] == ' ')
				--r->wrap_buflen;
			strcpy(r->wrap_buf + r->wrap_buflen, " ");
			rc=(*r->handler)(r, r->voidarg);
			if (rc)
				return (rc);
			r->wrap_buflen=0;
			needspc=0;
			continue;
		}
		i= w-1;
		memcpy(r->wrap_buf, ptr, i);
		ptr += i;
		len -= i;
		strcpy(r->wrap_buf+i, " ");
		rc=(*r->handler)(r, r->voidarg);
		if (rc)
			return (rc);
		r->wrap_buflen=0;
		needspc=0;
	}
	r->has_prev=isflowed;

	if (!isflowed)
	{
		r->wrap_buf[r->wrap_buflen]=0;
		rc=(*r->handler)(r, r->voidarg);
		if (rc)
			return (rc);
		r->wrap_buflen=0;
	}
	return (0);
}
