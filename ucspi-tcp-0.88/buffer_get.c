/*
 * $Log: buffer_get.c,v $
 * Revision 1.2  2008-07-25 16:48:38+05:30  Cprogrammer
 * fix for darwin
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "buffer.h"
#include "byte.h"
#include "error.h"

static int
oneread(ssize_t (*op) (), int fd, char *buf, unsigned int len)
{
	int             r;

	for (;;)
	{
		r = op(fd, buf, len);
		if (r == -1 && errno == error_intr)
			continue;
		return r;
	}
}

static int
getthis(buffer * s, char *buf, unsigned int len)
{
	if (len > s->p)
		len = s->p;
	s->p -= len;
	byte_copy(buf, len, s->x + s->n);
	s->n += len;
	return len;
}

int
buffer_feed(buffer * s)
{
	int             r;

	if (s->p)
		return s->p;
	if((r = oneread(s->op, s->fd, s->x, s->n)) <= 0)
		return r;
	s->p = r;
	s->n -= r;
	if (s->n > 0)
		byte_copyr(s->x + s->n, r, s->x);
	return r;
}

int
buffer_bget(buffer * s, char *buf, unsigned int len)
{
	int             r;

	if (s->p > 0)
		return getthis(s, buf, len);
	if (s->n <= len)
		return oneread(s->op, s->fd, buf, s->n);
	if((r = buffer_feed(s)) <= 0)
		return r;
	return getthis(s, buf, len);
}

int
buffer_get(buffer * s, char *buf, unsigned int len)
{
	int             r;

	if (s->p > 0)
		return getthis(s, buf, len);
	if (s->n <= len)
		return oneread(s->op, s->fd, buf, len);
	if((r = buffer_feed(s)) <= 0)
		return r;
	return getthis(s, buf, len);
}

char           *
buffer_peek(buffer * s)
{
	return s->x + s->n;
}

void
buffer_seek(buffer * s, unsigned int len)
{
	s->n += len;
	s->p -= len;
}
