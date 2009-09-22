/*
** Copyright 2000 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include "rfc2045_config.h"
#include	"rfc2646.h"
#include	<stdlib.h>
#include	<string.h>

static const char rcsid[]="$Id: rfc2646.c,v 1.2 2003/03/07 00:47:31 mrsam Exp $";

struct rfc2646parser *rfc2646_alloc( int (*f)(struct rfc2646parser *, int,
					     void *),
				     void *a)
{
	struct rfc2646parser *p=(struct rfc2646parser *)
		calloc(1, sizeof(struct rfc2646parser));

	if (!p)
		return (0);

	p->handler=f;
	p->voidarg=a;
	return (p);
}

#define PARSER_QUOTECNT	1
#define PARSER_LINE	2

int rfc2646_parse_cb(const char *s, size_t c, void *vp)
{
	return (rfc2646_parse( (struct rfc2646parser *)vp, s, c));
}

int rfc2646_parse(struct rfc2646parser *p, const char *s, size_t c)
{
	for ( ; c; --c, ++s)
	{
		if (*s == '\r')
			continue;

		if (!p->parse_mode)
		{
			p->parse_mode=PARSER_QUOTECNT;
			p->quote_depth=0;
			p->linelen=0;
		}

		if (*s == '\n')
		{
			int rc;
			int isflowed=0;

			if (p->linelen && p->line[p->linelen-1] == ' ' &&
			    (p->linelen != 3 || strncmp(p->line, "-- ", 3)))
			{
				--p->linelen;
				isflowed=1;
			}

			p->line[p->linelen]=0;

			p->parse_mode=0;
			rc=(*p->handler)(p, isflowed, p->voidarg);
			if (rc)
				return (rc);
			continue;
		}

		if (p->parse_mode == PARSER_QUOTECNT)
		{
			if (*s == '>')
			{
				++p->quote_depth;
				continue;
			}

			p->parse_mode=PARSER_LINE;
			if (*s == ' ')
			{
				continue;
			}
		}
		if (p->linelen < sizeof(p->line)-1)
			p->line[p->linelen++]= *s;
	}
	return (0);
}

int rfc2646_free(struct rfc2646parser *p)
{
	int rc=0;

	if (p->parse_mode)
		rc=rfc2646_parse(p, "\n", 1);
	free(p);
	return (rc);
}
