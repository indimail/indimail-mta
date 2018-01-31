/*
 * $Log: substdio.c,v $
 * Revision 1.4  2008-07-14 20:59:22+05:30  Cprogrammer
 * fixed compilation warning on 64 bit os
 *
 * Revision 1.3  2004-10-22 20:31:11+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:24:39+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "substdio.h"
#include <sys/types.h>

void
substdio_fdbuf(s, op, fd, buf, len)
	register substdio *s;
	register ssize_t (*op) ();
	register int    fd;
	register char  *buf;
	register int    len;
{
	s->x = buf;
	s->fd = fd;
	s->op = op;
	s->p = 0;
	s->n = len;
}

void
getversion_substdio_c()
{
	static char    *x = "$Id: substdio.c,v 1.4 2008-07-14 20:59:22+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
