/*
** Copyright 2000 Double Precision, Inc.  See COPYING for
** distribution information.
*/

#include "rfc2045_config.h"
#include	"rfc2646.h"
#include	<stdlib.h>
#include	<string.h>

static const char rcsid[]="$Id: rfc2646reply.c,v 1.4 2003/03/07 00:47:31 mrsam Exp $";

struct rfc2646reply *rfc2646reply_alloc( int (*f)(const char *, size_t,
						  void *),
					 void *a)
{
	struct rfc2646reply *p=(struct rfc2646reply *)
		calloc(1, sizeof(struct rfc2646reply));

	if (!p)
		return (0);

	p->handler=f;
	p->voidarg=a;
	p->prev_was_flowed=1;
	p->first_line=1;
	return (p);
}

static int flushreply(struct rfc2646reply *p, int isflowed)
{
	int qcnt=p->current_quote_depth+1;
	int rc;

	static const char quotes[]=">>>>>";

	while (qcnt)
	{
		int l;

		l=sizeof(quotes)-1;
		if (l > qcnt)
			l=qcnt;

		rc= (*p->handler)(quotes, l, p->voidarg);
		if (rc)
			return (rc);
		qcnt -= l;
	}

	if (!isflowed)
		while (p->replylen && p->replybuffer[p->replylen-1] == ' ')
		{
			if (p->replylen == 3 && strncmp(p->replybuffer, "-- ",
							3) == 0)
				break;
			--p->replylen;
		}

	rc= (*p->handler)(" ", 1, p->voidarg);
	if (rc == 0)
		rc= (*p->handler)(p->replybuffer, p->replylen, p->voidarg);
	p->replylen=0;
	if (rc == 0 && isflowed)
		rc= (*p->handler)(" ", 1, p->voidarg);
	if (rc == 0)
		rc= (*p->handler)("\n", 1, p->voidarg);
	return (rc);
}

static int fillreply(struct rfc2646reply *p, const char *txt, int txtlen)
{
	int rc;
	int maxlen;

	maxlen= p->current_quote_depth + 20 > sizeof(p->replybuffer)
		? 15: sizeof(p->replybuffer)-p->current_quote_depth;

	while (txtlen)
	{
		int l=txtlen;
		int i;

		if (p->replylen + l + 1 <= maxlen)
		{
			if (p->replylen)
				p->replybuffer[p->replylen++]=' ';

			memcpy(p->replybuffer + p->replylen, txt, l);
			p->replylen += l;
			break;
		}

		for (i=0; i<l; i++)
			if (txt[i] == ' ')
				break;

		if (p->replylen + i + 1 <= maxlen)
		{
			p->replybuffer[p->replylen]=' ';
			if (p->replylen)
				++p->replylen;

			memcpy(p->replybuffer + p->replylen,
			       txt, i);
			p->replylen += i;
			txt += i;
			txtlen -= i;
			if (txtlen)
			{
				++txt;
				--txtlen;
			}
			continue;
		}
		if (p->replylen)
		{
			rc=flushreply(p, 1);
			if (rc)
				return (rc);
			continue;
		}
		if (l > maxlen)
			l=maxlen;
		memcpy(p->replybuffer, txt, l);
		p->replylen=l;
		txt += l;
		txtlen -= l;
	}
	return (0);
}

int rfc2646reply_handler(struct rfc2646parser *p, int isflowed, void *vp)
{
	struct rfc2646reply *r=(struct rfc2646reply *)vp;
	int rc=0;

	int first_line=r->first_line;

	r->first_line=0;
	if (p->quote_depth != r->current_quote_depth)
	{
		if (r->replylen || r->prev_was_flowed)
			rc=flushreply(r, 0);

		if (rc)
			return (rc);
		r->current_quote_depth=p->quote_depth;
		r->prev_was_flowed=0;

		if (p->linelen == 0 && !isflowed)
		{
			rc=flushreply(r, 0);
			return (rc);
		}
		/* Want to insert a blank line, in this case */
	}

	if (r->prev_was_flowed && !isflowed && p->linelen == 0 && !first_line)
	{
		/* Special case */

		rc=flushreply(r, 1);
		if (rc == 0)
			rc=flushreply(r, 0);

		r->prev_was_flowed=0;
		return (rc);
	}

	rc=fillreply(r, p->line, p->linelen);
	if (!isflowed)
		rc=flushreply(r, 0);
	r->prev_was_flowed=isflowed;
	return (rc);
}

int rfc2646reply_free(struct rfc2646reply *p)
{
	int rc=0;

	if (p->replylen)
		rc=flushreply(p, 0);
	free(p);
	return (rc);
}
