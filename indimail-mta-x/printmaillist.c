/*
 * $Log: printmaillist.c,v $
 * Revision 1.3  2021-08-29 23:27:08+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.2  2004-10-22 20:28:01+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-10-21 22:47:06+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <substdio.h>
#include <subfd.h>
#include <str.h>
#include <strerr.h>
#include <stralloc.h>
#include <getln.h>
#include <noreturn.h>

#define FATAL "printmaillist: fatal: "

no_return void
badformat()
{
	strerr_die2x(100, FATAL, "bad mailing list format");
}

int
main()
{
	stralloc        line = { 0 };
	int             match;
	for (;;) {
		if (getln(subfdinsmall, &line, &match, '\0') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!match) {
			if (line.len)
				badformat();
			if (substdio_flush(subfdoutsmall) == -1)
				strerr_die2sys(111, FATAL, "unable to write output: ");
			_exit(0);
		}
		if (line.s[str_chr(line.s, '\n')])
			badformat();
		if (line.s[line.len - 1] == ' ')
			badformat();
		if (line.s[line.len - 1] == '\t')
			badformat();
		if ((line.s[0] == '.') || (line.s[0] == '/')) {
			if (substdio_puts(subfdoutsmall, line.s) == -1)
				strerr_die2sys(111, FATAL, "unable to write output: ");
			if (substdio_puts(subfdoutsmall, "\n") == -1)
				strerr_die2sys(111, FATAL, "unable to write output: ");
			continue;
		}
		if (line.s[0] == '&') {
			if (line.len > 900)
				badformat();
			if (substdio_puts(subfdoutsmall, line.s) == -1)
				strerr_die2sys(111, FATAL, "unable to write output: ");
			if (substdio_puts(subfdoutsmall, "\n") == -1)
				strerr_die2sys(111, FATAL, "unable to write output: ");
			continue;
		}
		badformat();
	}
	return(0);
}

void
getversion_printmaillist_c()
{
	static char    *x = "$Id: printmaillist.c,v 1.3 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
