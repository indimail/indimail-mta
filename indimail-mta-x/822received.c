/*
 * $Log: 822received.c,v $
 * Revision 1.10  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.9  2021-06-14 00:33:36+05:30  Cprogrammer
 * removed chdir(auto_sysconfdir)
 *
 * Revision 1.8  2020-11-24 13:43:43+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.7  2016-05-21 14:48:03+05:30  Cprogrammer
 * use auto_sysconfdir for leapsecs_init()
 *
 * Revision 1.6  2016-01-28 08:59:41+05:30  Cprogrammer
 * chdir qmail_home for opening etc/leapsecs.dat
 *
 * Revision 1.5  2005-08-23 17:14:28+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.4  2004-10-22 20:14:22+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-13 22:30:11+05:30  Cprogrammer
 * fixed compiler warning
 *
 * Revision 1.2  2004-06-17 23:27:20+05:30  Cprogrammer
 * error handling for substdio_puts(), substdio_flush()
 *
 * Revision 1.1  2004-06-16 01:19:49+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <substdio.h>
#include <case.h>
#include <strerr.h>
#include <subfd.h>
#include <getln.h>
#include <mess822.h>
#include <leapsecs.h>
#include <caltime.h>
#include <tai.h>
#include <noreturn.h>
#include "auto_sysconfdir.h"

#define FATAL "822received: fatal: "

no_return void
nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

mess822_time    t;
struct tai      sec;
unsigned char   secpack[TAI_PACK];
time_t          secunix;

stralloc        tokens = { 0 };

stralloc        line = { 0 };

void
doit()
{
	int             i;
	int             j;
	int             state;
	char            ch;
	char           *x;

	for (i = 0; i < line.len; ++i)
		if (line.s[i] == 0)
			line.s[i] = '\n';
	if (!stralloc_0(&line))
		nomem();

	t.known = 0;
	if (!mess822_token(&tokens, line.s))
		nomem();
	if (!mess822_when(&t, line.s))
		nomem();
	--line.len;

	if (!t.known)
		substdio_puts(subfdoutsmall, "\t\t\t");
	else {
		caltime_tai(&t.ct, &sec);
		tai_pack((char *) secpack, &sec);
		secunix = secpack[0] - 64;
		secunix = (secunix << 8) + secpack[1];
		secunix = (secunix << 8) + secpack[2];
		secunix = (secunix << 8) + secpack[3];
		secunix = (secunix << 8) + secpack[4];
		secunix = (secunix << 8) + secpack[5];
		secunix = (secunix << 8) + secpack[6];
		secunix = (secunix << 8) + secpack[7];
		secunix -= 10;
		substdio_put(subfdoutsmall, ctime(&secunix), 24);
	}
	substdio_puts(subfdoutsmall, " ");

	state = 1;
	/*
	 * 1: start; 2: middle; 3: immediately after space; 4: after semicolon 
	 */

	for (j = i = 0; j < tokens.len; ++j)
		if (!tokens.s[j]) {
			x = tokens.s + i;
			if (*x == '(') {
#ifdef notdef
				if (state == 3)
					substdio_puts(subfdoutsmall, "\n\t\t\t   ");
#endif
				substdio_puts(subfdoutsmall, "(");
				while ((ch = tokens.s[++i])) {
					if (ch == '\n')
						ch = 0;
					substdio_put(subfdoutsmall, &ch, 1);
				}
				substdio_puts(subfdoutsmall, ")");
				if (state & 1)
					state = 2;
			} else
			if (*x == '=') {
				if (state == 3)
					if (!case_diffs(x, "=from") || !case_diffs(x, "=by") || !case_diffs(x, "=for") || !case_diffs(x, "=id"))
						substdio_puts(subfdoutsmall, "\n\t\t\t ");
				while ((ch = tokens.s[++i])) {
					if (ch == '\n')
						ch = 0;
					substdio_put(subfdoutsmall, &ch, 1);
				}
				if (state & 1)
					state = 2;
			} else
			if (*x == ';') {
				if ((state == 2) || (state == 3))
					substdio_puts(subfdoutsmall, "\n\t\t\t ");
				state = 4;
				substdio_puts(subfdoutsmall, ";");
				if (state & 1)
					state = 2;
			} else
			if ((*x == ' ') || (*x == '\t')) {
				if ((state != 1) && (state != 3))
					substdio_puts(subfdoutsmall, " ");
				if (state == 2)
					state = 3;
			} else {
				substdio_put(subfdoutsmall, tokens.s + i, 1);
				if (state & 1)
					state = 2;
			}
			i = j + 1;
		}

	substdio_puts(subfdoutsmall, "\n");
}

stralloc        received = { 0 };

mess822_header  h = MESS822_HEADER;
mess822_action  a[] = {
	{"received", 0, 0, &received, 0, 0}
	, {0, 0, 0, 0, 0, 0}
};

int
main(int argc, char **argv)
{
	int             i;
	int             j;
	int             match;

	if (leapsecs_init() == -1)
		strerr_die2sys(111, FATAL, "unable to init leapsecs: ");
	if (!mess822_begin(&h, a))
		nomem();
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
	i = 0;
	j = received.len;
	for (;;) {
		if (!j || (received.s[j - 1] == '\n')) {
			if (i >= j) {
				if (!stralloc_copyb(&line, received.s + j, i - j))
					nomem();
				doit();
			}
			if (!j)
				break;
			i = j - 1;
		}
		--j;
	}
	if (substdio_flush(subfdoutsmall))
		strerr_die2sys(111, FATAL, "unable to write: ");
	_exit(0);
	/*- Not reached */
	return(0);
}

void
getversion_822received_c()
{
	static char    *x = "$Id: 822received.c,v 1.10 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
