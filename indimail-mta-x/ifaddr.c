/*
 * $Log: ifaddr.c,v $
 * Revision 1.4  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.3  2020-11-24 13:45:27+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.2  2004-10-22 20:25:48+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-07-17 20:54:43+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <substdio.h>
#include <alloc.h>
#include <strerr.h>
#include <getln.h>
#include <str.h>
#include <mess822.h>
#include <case.h>
#include <env.h>
#include <noreturn.h>

#define FATAL "ifaddr: fatal: "

char           *recipient;
char          **addrs;
static stralloc addrlist = { 0 };
static mess822_action  a0[] = {
	{"to", 0, 0, 0, &addrlist, 0},
	{"cc", 0, 0, 0, &addrlist, 0},
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
		a1[i].flag = 0;
		a1[i].copy = 0;
		a1[i].value = 0;
		a1[i].addr = &addrlist;
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

void
check(char *addr)
{
	int             i;

	if (recipient) {
		if (recipient[0] == '@') {
			if (case_equals(addr + str_rchr(addr, '@'), recipient))
				_exit(0);
		} else
		if (case_equals(addr, recipient))
			_exit(0);
	} else {
		for (i = 0; addrs[i]; ++i) {
			if (addrs[i][0] == '@') {
				if (case_equals(addr + str_rchr(addr, '@'), addrs[i]))
					_exit(0);
			} else
			if (case_equals(addr, addrs[i]))
				_exit(0);
		}
	}
}

int
main(int argc, char **argv)
{
	int             i, j, match;
	stralloc        line = { 0 };
	char            ssinbuf[1024];
	substdio        ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);
	mess822_header  h = MESS822_HEADER;
	mess822_action *a;

	recipient = env_get("RECIPIENT");
	i = 0;
	addrs = ++argv;

	while (*addrs && str_diff(*addrs, ":")) {
		++i;
		++addrs;
	}

	if (*addrs) {
		a = init(i, argv);
		++addrs;
	} else {
		a = init(0, addrs);
		addrs = argv;
	}
	if (!mess822_begin(&h, a))
		nomem();
	if (*addrs)
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
getversion_ifaddr_c()
{
	static char    *x = "$Id: ifaddr.c,v 1.4 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
