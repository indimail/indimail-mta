/*
 * $Log: subfdins.c,v $
 * Revision 1.5  2008-07-14 20:59:14+05:30  Cprogrammer
 * fixed compilation warning on 64 bit os
 *
 * Revision 1.4  2004-10-22 20:31:05+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-10-22 15:39:40+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.2  2004-07-17 21:24:30+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "subfd.h"

ssize_t
subfd_readsmall(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	if (substdio_flush(subfdoutsmall) == -1)
		return -1;
	return read(fd, buf, len);
}

char            subfd_inbufsmall[256];
static substdio it = SUBSTDIO_FDBUF(subfd_readsmall, 0, subfd_inbufsmall, 256);
substdio       *subfdinsmall = &it;

void
getversion_subfdins_c()
{
	static char    *x = "$Id: subfdins.c,v 1.5 2008-07-14 20:59:14+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
