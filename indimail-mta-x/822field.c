/*
 * $Log: 822field.c,v $
 * Revision 1.5  2020-11-28 12:43:07+05:30  Cprogrammer
 * +HeaderName feature by Erwin Hoffman: display all headers which have HeaderName as the initial text
 *
 * Revision 1.4  2020-11-24 13:42:20+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.3  2004-10-22 20:13:52+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-06-17 23:26:57+05:30  Cprogrammer
 * error handling for substdio_puts(), substdio_flush()
 *
 * Revision 1.1  2004-06-16 01:19:46+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "strerr.h"
#include "subfd.h"
#include "getln.h"
#include "mess822.h"

#define FATAL "822field: fatal: "

void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

int             flag;
stralloc        value = { 0 };

mess822_header  h = MESS822_HEADER;
mess822_action  a[] = {
	{"subject", &flag, 0, &value, 0, 0}
	, {0, 0, 0, 0, 0, 0}
};

stralloc        line = { 0 };
int             match;

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             t = 0;

	if (argv[1]) {
		if (*argv[1] == '+') {
			t = 1;
			if (!*(argv[1] + 1))
				a[0].name = "subject";
			else {
				a[0].name = argv[1] + 1;
			}
		} else
			a[0].name = argv[1];
	}
	if (!mess822_begin(&h, a))
		nomem();
	if (t)
		*a->flag = 2; /*- do a match on the initial length */
	for (;;) {
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
	if (substdio_putflush(subfdoutsmall, value.s, value.len))
		strerr_die2sys(111, FATAL, "unable to write: ");
	_exit(flag ? 0 : 100);
}

void
getversion_822field_c()
{
	static char    *x = "$Id: 822field.c,v 1.5 2020-11-28 12:43:07+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
