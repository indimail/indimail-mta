/*
 * $Log: buffer.c,v $
 * Revision 1.2  2008-07-25 16:48:33+05:30  Cprogrammer
 * fix for darwin
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "buffer.h"

void
buffer_init(buffer * s, ssize_t (*op) (), int fd, char *buf, unsigned int len)
{
	s->x = buf;
	s->fd = fd;
	s->op = op;
	s->p = 0;
	s->n = len;
}
