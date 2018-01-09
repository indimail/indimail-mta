/*
 * $Log: subfdouts.c,v $
 * Revision 1.4  2004-10-22 20:31:06+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-10-22 15:39:46+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.2  2004-07-17 21:24:33+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "subfd.h"

char            subfd_outbufsmall[256];
static substdio it = SUBSTDIO_FDBUF(write, 1, subfd_outbufsmall, 256);
substdio       *subfdoutsmall = &it;

void
getversion_subfdouts_c()
{
	static char    *x = "$Id: subfdouts.c,v 1.4 2004-10-22 20:31:06+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
