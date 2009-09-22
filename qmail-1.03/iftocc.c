/*
 * $Log: iftocc.c,v $
 * Revision 1.2  2004-10-22 20:25:50+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-06-16 01:19:54+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "substdio.h"
#include "strerr.h"
#include "subfd.h"
#include "getln.h"
#include "mess822.h"
#include "case.h"
#include "env.h"
#include "exit.h"

#define FATAL "iftocc: fatal: "

void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

stralloc        addrlist = { 0 };

mess822_header  h = MESS822_HEADER;
mess822_action  a[] = {
	{"to", 0, 0, 0, &addrlist, 0}
	, {"cc", 0, 0, 0, &addrlist, 0}
	, {0, 0, 0, 0, 0, 0}
};

stralloc        line = { 0 };
int             match;

char           *recipient;
char          **recips;

void
check(addr)
	char           *addr;
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
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             i;
	int             j;

	recipient = env_get("RECIPIENT");
	recips = argv + 1;
	if (*recips)
		recipient = 0;

	if (!mess822_begin(&h, a))
		nomem();

	for (;;)
	{
		if (getln(subfdinsmall, &line, &match, '\n') == -1)
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

	for (j = i = 0; j < addrlist.len; ++j)
		if (!addrlist.s[j])
		{
			if (addrlist.s[i] == '+')
				check(addrlist.s + i + 1);
			i = j + 1;
		}

	_exit(100);
}

void
getversion_iftocc_c()
{
	static char    *x = "$Id: iftocc.c,v 1.2 2004-10-22 20:25:50+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
