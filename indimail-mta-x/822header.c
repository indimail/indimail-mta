/*
 * $Log: 822header.c,v $
 * Revision 1.8  2022-10-30 17:54:57+05:30  Cprogrammer
 * converted to ansic prototype
 *
 * Revision 1.7  2020-11-24 13:42:27+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.6  2005-04-05 21:19:59+05:30  Cprogrammer
 * removed superfluous lines
 *
 * Revision 1.5  2005-02-14 23:04:46+05:30  Cprogrammer
 * added tab as LWSP char
 * added usage
 *
 * Revision 1.4  2005-01-22 00:37:56+05:30  Cprogrammer
 * added options to include/exclude specific header fields
 *
 * Revision 1.3  2004-10-22 20:14:05+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-06-17 23:27:08+05:30  Cprogrammer
 * error handling for substdio_puts(), substdio_flush()
 *
 * Revision 1.1  2003-12-07 12:59:02+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "substdio.h"
#include "sgetopt.h"
#include "str.h"
#include "case.h"
#include "strerr.h"
#include "subfd.h"
#include "getln.h"
#include "mess822.h"

#define FATAL "822header: fatal: "

stralloc        line = { 0 };
stralloc        incl = { 0 };
stralloc        excl = { 0 };
int             match;

int
main(int argc, char **argv)
{
	int             opt, len, token_len, include = 0, exclude = 0, keep_continue = 0;
	char           *ptr;

	while ((opt = getopt(argc, argv, "I:X:")) != opteof) {
		switch (opt)
		{
		case 'X':
			exclude = 1;
			if (!stralloc_cats(&excl, optarg))
				strerr_die2x(111, FATAL, "out of memory");
			if (!stralloc_append(&excl, ":"))
				strerr_die2x(111, FATAL, "out of memory");
			if (!stralloc_0(&excl))
				strerr_die2x(111, FATAL, "out of memory");
			break;
		case 'I':
			include = 1;
			if (!stralloc_cats(&incl, optarg))
				strerr_die2x(111, FATAL, "out of memory");
			if (!stralloc_append(&incl, ":"))
				strerr_die2x(111, FATAL, "out of memory");
			if (!stralloc_0(&incl))
				strerr_die2x(111, FATAL, "out of memory");
			break;
		default:
			strerr_die1x(100, "USAGE: 822header [-I include headers] [-X exclude headers]");
			break;
		}
	}
	if (include && exclude)
		strerr_die1x(100, "Only one of -I or -X can be specified");
	for (;;) {
		if (getln(subfdinsmall, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!mess822_ok(&line))
			break;
		if (include) {
			if (line.s[0] == ' ' || line.s[0] == '\t') /*- RFC 822 LWSP char */ {
				if (keep_continue) {
					if (substdio_put(subfdoutsmall, line.s, line.len))
						strerr_die2sys(111, FATAL, "unable to write: ");
				}
			} else {
				keep_continue = 0;
				for (len = 0, ptr = incl.s;len < incl.len;) {
					len += ((token_len = str_len(ptr)) + 1);
					if (!case_diffb(ptr, token_len, line.s)) {
						if (substdio_put(subfdoutsmall, line.s, line.len))
							strerr_die2sys(111, FATAL, "unable to write: ");
						keep_continue = 1;
						break;
					}
					ptr = incl.s + len;
				}
			}
		} else
		if (exclude) {
			exclude = 1;
			if (line.s[0] == ' ' || line.s[0] == '\t') /*- RFC 822 LWSP char */ {
				if (!keep_continue)
					exclude = 2;
			} else {
				keep_continue = 1;
				for (len = 0, ptr = excl.s;len < excl.len;) {
					len += ((token_len = str_len(ptr)) + 1);
					if (!case_diffb(ptr, token_len, line.s)) {
						exclude = 2;
						keep_continue = 0;
						break;
					}
					ptr = excl.s + len;
				}
			}
			if (exclude == 1 && substdio_put(subfdoutsmall, line.s, line.len))
				strerr_die2sys(111, FATAL, "unable to write: ");
		} else
		if (substdio_put(subfdoutsmall, line.s, line.len))
			strerr_die2sys(111, FATAL, "unable to write: ");
		if (!match)
			break;
	}
	if (substdio_flush(subfdoutsmall))
		strerr_die2sys(111, FATAL, "unable to write: ");
	return(0);
}

void
getversion_822header_c()
{
	static char    *x = "$Id: 822header.c,v 1.8 2022-10-30 17:54:57+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
