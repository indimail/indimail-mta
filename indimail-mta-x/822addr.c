/*
 * $Log: 822addr.c,v $
 * Revision 1.6  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2024-01-23 01:19:16+05:30  Cprogrammer
 * include buffer_defs.h for buffer size definitions
 *
 * Revision 1.4  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.3  2020-11-24 13:41:57+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.2  2004-10-22 20:12:48+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-17 21:04:32+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <substdio.h>
#include <alloc.h>
#include <strerr.h>
#include <getln.h>
#include <mess822.h>
#include <case.h>
#include <stralloc.h>
#include <noreturn.h>
#include "buffer_defs.h"

#define FATAL "822addr: fatal: "

static char     ssinbuf[BUFSIZE_IN];
static char     ssoutbuf[BUFSIZE_OUT];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static int      flag;
static int      match;
static stralloc addr = { 0 };
static stralloc line = { 0 };

static mess822_header  h = MESS822_HEADER;
static mess822_action *a;
static mess822_action  a0[] = {
	{"to", &flag, 0, 0, &addr, 0},
	{"cc", &flag, 0, 0, &addr, 0},
	{0, 0, 0, 0, 0, 0}
};

no_return void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}


mess822_action *
init(int n, char **s)
{
	int             i;
	mess822_action *a1;

	if (!n)
		return a0;

	if (!(a1 = (mess822_action *) alloc((n + 1) * sizeof(mess822_action))))
		nomem();
	for (i = 0; i < n; i++) {
		a1[i].name = *s;
		a1[i].flag = &flag;
		a1[i].copy = 0;
		a1[i].value = 0;
		a1[i].addr = &addr;
		a1[i].when = 0;
		++s;
	}
	a1[n].name = 0;
	a1[n].flag = 0;
	a1[n].copy = 0;
	a1[n].value = 0;
	a1[n].addr = 0;
	a1[n].when = 0;
	return a1;
}

int
main(int argc, char **argv)
{
	a = init(argc - 1, ++argv);
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
	if (substdio_put(&ssout, addr.s, addr.len))
		strerr_die2sys(111, FATAL, "unable to write: ");
	if (substdio_flush(&ssout))
		strerr_die2sys(111, FATAL, "unable to write: ");
	_exit(flag ? 0 : 100);
}

void
getversion_822addr_c()
{
	const char     *x = "$Id: 822addr.c,v 1.6 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
