/*
 * $Log: 822fields.c,v $
 * Revision 1.4  2020-11-28 12:43:14+05:30  Cprogrammer
 * +HeaderName feature by Erwin Hoffman: display all headers which have HeaderName as the initial text
 *
 * Revision 1.3  2020-11-24 13:42:24+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.2  2004-10-22 20:14:02+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-17 20:46:15+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "alloc.h"
#include "strerr.h"
#include "getln.h"
#include "mess822.h"
#include "case.h"
#include "stralloc.h"

#define FATAL "822fields: fatal: "

static int      flag, t = 2;
stralloc        value = { 0 };
static char     ssinbuf[1024];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);
static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);

mess822_header  h = MESS822_HEADER;
mess822_action *a;
mess822_action  a0[] = {
	{"subject", &flag, 0, &value, 0, 0}
	, {0, 0, 0, 0, 0, 0}
};

stralloc        line = { 0 };
int             match;

void
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
	a1 = (mess822_action *) alloc((n + 1) * sizeof(mess822_action));
	if (!a1)
		nomem();
	for (i = 0; i < n; i++) {
		a1[i].name = *s[0] == '+' ? *s + 1 : *s;
		if (!a1[i].name[1])
			a1[i].name = "subject";
		a1[i].copy = 0;
		a1[i].value = &value;
		a1[i].addr = 0;
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
	int             i;

	a = init(argc - 1, ++argv);
	if (!mess822_begin(&h, a))
		nomem();
	for (i = 0; i < argc - 1; i++)
		a[i].flag = *argv[i] == '+' ? &t : &flag;
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
	substdio_putflush(&ssout, value.s, value.len);
	_exit(flag ? 0 : 100);
}

void
getversion_822fields_c()
{
	static char    *x = "$Id: 822fields.c,v 1.4 2020-11-28 12:43:14+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
