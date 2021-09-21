/*
 * $Log: checkaddr.c,v $
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
#include <unistd.h>
#include <substdio.h>
#include <strerr.h>
#include <getln.h>
#include <mess822.h>
#include <case.h>
#include <env.h>
#include <noreturn.h>

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
	char            ssinbuf[1024];
	substdio        ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);

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
	static char    *x = "$Id: checkaddr.c,v 1.4 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
