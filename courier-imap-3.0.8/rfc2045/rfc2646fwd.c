/*
** Copyright 2000 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include "rfc2045_config.h"
#include	"rfc2646.h"
#include	<stdlib.h>
#include	<string.h>

static const char rcsid[]="$Id: rfc2646fwd.c,v 1.2 2003/03/07 00:47:31 mrsam Exp $";

struct rfc2646fwd *rfc2646fwd_alloc( int (*f)(const char *, size_t, void *),
					   void *a)
{
	struct rfc2646fwd *p=(struct rfc2646fwd *)
		calloc(1, sizeof(struct rfc2646fwd));

	if (!p)
		return (0);

	p->handler=f;
	p->voidarg=a;
	return (p);
}

int rfc2646fwd_free(struct rfc2646fwd *p)
{
	int rc=0;

	if (p->prev_was_0depth)
		rc= (*p->handler)("\n", 1, p->voidarg);

	free(p);
	return (rc);
}

int rfc2646fwd_handler(struct rfc2646parser *p, int isflowed, void *vp)
{
	struct rfc2646fwd *r=(struct rfc2646fwd *)vp;
	int rc;

	/* Quoted text is copied verbatim. */

	if (p->quote_depth > 0)
	{
		static const char quotes[]=">>>>>";
		int n=p->quote_depth;

		if (r->prev_was_0depth)
			rc= (*r->handler)("\n", 1, r->voidarg);
		r->prev_was_0depth=0;

		while (n)
		{
			int i=sizeof(quotes)-1;

			if (i > n)
				i=n;
			rc= (*r->handler)(quotes, i, r->voidarg);
			if (rc)
				return (rc);
			n -= i;
		}

		rc=(*r->handler)(" ", 1, r->voidarg);
		if (rc == 0 && p->linelen)
			rc=(*r->handler)(p->line, p->linelen, r->voidarg);
		if (rc == 0 && isflowed)
			rc=(*r->handler)(" ", 1, r->voidarg);
		if (rc == 0)
			rc=(*r->handler)("\n", 1, r->voidarg);
		return (rc);
	}

	if (p->linelen == 0)
	{
		/*
		** If the previous line was flowed, insert two newlines here,
		** else insert one.
		*/

		rc=(*r->handler)("\n\n", r->prev_was_0depth ? 2:1, r->voidarg);
		r->prev_was_0depth=0;
		return (rc);
	}

	if (!r->prev_was_0depth && p->linelen == 3 &&
	    strncmp(p->line, "-- ", 3) == 0 && !isflowed)
	{
		return ((*r->handler)(p->line, 3, r->voidarg));
	}

	if ((r->prev_was_0depth || p->line[0] == '>')
	    && (rc=(*r->handler)(" ", 1, r->voidarg)) != 0)
		return (rc);


	rc=(*r->handler)(p->line, p->linelen, r->voidarg);
	if (rc)
		return (rc);
	if (!isflowed)
	{
		rc=(*r->handler)("\n", 1, r->voidarg);
		if (rc)
			return (rc);
		r->prev_was_0depth=0;
		return (0);
	}
	r->prev_was_0depth=1;
	return (0);
}
