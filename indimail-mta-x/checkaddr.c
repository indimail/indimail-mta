/*
 * $Id: checkaddr.c,v 1.7 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <substdio.h>
#include <strerr.h>
#include <getln.h>
#include <mess822.h>
#include <case.h>
#include <env.h>
#include <noreturn.h>
#include "buffer_defs.h"

#define FATAL "checkaddr: fatal: "

char           *recipient;
char          **recips;

no_return void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

void
check(char *addr)
{
	int             i;

	if (recipient)
		if (case_equals(addr, recipient))
			_exit(0);
	if (!recipient)
		for (i = 0; recips[i]; ++i)
			if (case_equals(addr, recips[i]))
				_exit(0);
}

int
main(int argc, char **argv)
{
	stralloc        addrlist = { 0 };
	int             match;
	char            ssinbuf[BUFSIZE_IN];
	substdio        ssin = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) read, 0, ssinbuf, sizeof ssinbuf);

	recipient = env_get("RECIPIENT");
	recips = argv + 1;
	if (*recips)
		recipient = 0;
	for (;;) {
		if (getln(&ssin, &addrlist, &match, '\0') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!match)
			break;
		if (addrlist.s[0] == '+')
			check(addrlist.s + 1);
	}

	_exit(100);
}

void
getversion_checkaddr_c()
{
	const char     *x = "$Id: checkaddr.c,v 1.7 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: checkaddr.c,v $
 * Revision 1.7  2025-01-22 00:30:35+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.6  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2024-01-23 01:24:00+05:30  Cprogrammer
 * include buffer_defs.h for buffer size definitions
 *
 * Revision 1.4  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.3  2020-11-24 13:44:22+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.2  2004-10-22 20:23:50+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-17 21:25:33+05:30  Cprogrammer
 * Initial revision
 *
 */
