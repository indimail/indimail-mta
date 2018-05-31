/*
 * $Log: subfdout.c,v $
 * Revision 1.4  2004-10-22 20:31:06+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-10-22 15:39:43+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.2  2004-07-17 21:24:32+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "subfd.h"

char            subfd_outbuf[SUBSTDIO_OUTSIZE];
static substdio it = SUBSTDIO_FDBUF(write, 1, subfd_outbuf, SUBSTDIO_OUTSIZE);
substdio       *subfdout = &it;

void
getversion_subfdout_c()
{
	static char    *x = "$Id: subfdout.c,v 1.4 2004-10-22 20:31:06+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
