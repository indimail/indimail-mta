/*
 * $Log: checkdomain.c,v $
 * Revision 1.6  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2024-01-23 01:20:40+05:30  Cprogrammer
 * include buffer_defs.h for buffer size definitions
 *
 * Revision 1.4  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.3  2020-11-24 13:44:25+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.2  2004-10-22 20:23:52+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-17 20:49:42+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <substdio.h>
#include <str.h>
#include <strerr.h>
#include <getln.h>
#include <mess822.h>
#include <case.h>
#include <env.h>
#include <noreturn.h>
#include "buffer_defs.h"

#define FATAL "checkdomain: fatal: "

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
	int             i, match;
	stralloc        addrlist = { 0 };
	char            ssinbuf[BUFSIZE_IN];
	substdio        ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);

	recipient = env_get("RECIPIENT");
	recips = argv + 1;
	if (*recips)
		recipient = 0;
	else {
		recipient += str_rchr(recipient, '@');
		if (recipient)
			++recipient;
	}
	for (;;) {
		if (getln(&ssin, &addrlist, &match, '\0') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!match)
			break;
		if (addrlist.s[0] == '+') {
			i = str_rchr(addrlist.s, '@');
			if (addrlist.s[i])
				check(addrlist.s + i + 1);
		}
	}
	_exit(100);
}

void
getversion_checkdomain_c()
{
	const char     *x = "$Id: checkdomain.c,v 1.6 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
