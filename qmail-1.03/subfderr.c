/*
 * $Log: subfderr.c,v $
 * Revision 1.4  2004-10-22 20:31:03+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-10-22 15:39:34+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.2  2004-07-17 21:24:26+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "subfd.h"

char            subfd_errbuf[256];
static substdio it = SUBSTDIO_FDBUF(write, 2, subfd_errbuf, 256);
substdio       *subfderr = &it;

void
getversion_subfderr_c()
{
	static char    *x = "$Id: subfderr.c,v 1.4 2004-10-22 20:31:03+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
