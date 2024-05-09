/*
 * $Log: hostname.c,v $
 * Revision 1.8  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2020-11-24 13:45:23+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.6  2004-10-22 20:25:46+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-10-22 15:35:17+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.4  2004-07-17 21:19:08+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "subfd.h"

char            host[256];

int
main()
{
	host[0] = 0;
	gethostname(host, sizeof(host));
	host[sizeof(host) - 1] = 0;
	substdio_puts(subfdoutsmall, host);
	substdio_puts(subfdoutsmall, "\n");
	substdio_flush(subfdoutsmall);
	return(0);
}

void
getversion_hostname_c()
{
	const char     *x = "$Id: hostname.c,v 1.8 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
