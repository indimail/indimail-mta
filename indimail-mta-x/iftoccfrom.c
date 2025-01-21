/*
 * $Id: iftoccfrom.c,v 1.7 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $
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

#define FATAL "iftoccfrom: fatal: "

no_return void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}


char           *recipient;
char          **recips;

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
	int             i, j, match;
	stralloc        addrlist = { 0 }, line = { 0 };
	char            ssinbuf[BUFSIZE_IN];
	substdio        ssin = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) read, 0, ssinbuf, sizeof ssinbuf);
	mess822_header  h = MESS822_HEADER;
	mess822_action  a[] = {
		{ "to", 0, 0, 0, &addrlist, 0 },
		{ "cc", 0, 0, 0, &addrlist, 0 },
		{ "from", 0, 0, 0, &addrlist, 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};

	recipient = env_get("RECIPIENT");
	recips = argv + 1;
	if (*recips)
		recipient = 0;

	if (!mess822_begin(&h, a))
		nomem();

	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!mess822_ok(&line))
			break;
		if (!mess822_line(&h, &line))
			nomem();
		if (!match)
			break;
	}
	if (!mess822_end(&h))
		nomem();
	for (j = i = 0; j < addrlist.len; ++j) {
		if (!addrlist.s[j]) {
			if (addrlist.s[i] == '+')
				check(addrlist.s + i + 1);
			i = j + 1;
		}
	}
	_exit(100);
}

void
getversion_iftoccfrom_c()
{
	const char     *x = "$Id: iftoccfrom.c,v 1.7 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: iftoccfrom.c,v $
 * Revision 1.7  2025-01-22 00:30:35+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.6  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2024-01-23 01:24:13+05:30  Cprogrammer
 * include buffer_defs.h for buffer size definitions
 *
 * Revision 1.4  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.3  2020-11-24 13:45:33+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.2  2004-10-22 20:25:51+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-17 20:55:07+05:30  Cprogrammer
 * Initial revision
 *
 */
