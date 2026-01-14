/*
 * $Id: svstatd.c,v 1.1 2026-01-14 18:51:19+05:30 Cprogrammer Exp mbhangui $
 *
 * © 2026 Manvendra Bhangui
 * All intellectual property developed during the term of this agreement, including but not limited
 * to svstatd.c, shall be and remain the sole and exclusive property of Manvendra Bhangui
 */

#include <unistd.h>
#include <ctype.h>
#include <stralloc.h>
#include <str.h>
#include <substdio.h>
#include <subfd.h>
#include <getln.h>
#include <strerr.h>

#define FATAL "svstatd: fatal: "

stralloc        line = { 0 };
int             i, match;
const char     *(svstat[]) = { "svstat", 0, (char *) NULL};

int
main(int argc, char **argv)
{
	if (getln(subfdinsmall, &line, &match, '\n') == -1)
		strerr_die2sys(111, FATAL, "unable to read input: ");
	if (!line.len || !match) {
		if (substdio_puts(subfdout, FATAL) == -1 ||
				substdio_put(subfdout, "invalid input\n", 14) == -1 ||
				substdio_flush(subfdout) == -1)
			strerr_die2x(111, FATAL, "Unable to write to descriptor 2");
		strerr_die2x(111, FATAL, "invalid input");
	}
	if (!stralloc_0(&line)) {
		if (substdio_puts(subfdout, FATAL) == -1 ||
				substdio_put(subfdout, "out of memory\n", 14) == -1 ||
				substdio_flush(subfdout) == -1)
			strerr_die2x(111, FATAL, "Unable to write to descriptor 2");
		strerr_die2x(111, FATAL, "out of memory");
	}
	line.s[line.len - 2] = 0;
	i = str_chr(line.s, ' ');
	if (line.s[i]) {
		line.s[i] = '/';
		if (access(line.s, X_OK)) {
			if (substdio_puts(subfdout, FATAL) == -1 ||
					substdio_put(subfdout, "unable to access ", 17) == -1 ||
					substdio_put(subfdout, line.s, line.len - 2) == -1 ||
					substdio_flush(subfdout) == -1)
				strerr_die2x(111, FATAL, "Unable to write to descriptor 2");
			strerr_die4sys(111, FATAL, "unable to access [", line.s, "]: ");
		}
		svstat[1] = line.s;
		execvp(*svstat, (char **) svstat);
		if (substdio_puts(subfdout, FATAL) == -1 ||
				substdio_put(subfdout, "unable to exec svstat ", 22) == -1 ||
				substdio_put(subfdout, line.s, line.len - 2) == -1 ||
				substdio_flush(subfdout) == -1)
			strerr_die2x(111, FATAL, "Unable to write to descriptor 2");
		strerr_die4sys(111, FATAL, "unable to exec svstat ", line.s, ": ");
	} else {
		if (substdio_puts(subfdout, FATAL) == -1 ||
				substdio_put(subfdout, "invalid input\n", 14) == -1 ||
				substdio_flush(subfdout) == -1)
			strerr_die2x(111, FATAL, "Unable to write to descriptor 2");
		strerr_die2x(111, FATAL, "invalid input");
	}
	/*- NOT REACHED */
}

/*
 * $Log: svstatd.c,v $
 * Revision 1.1  2026-01-14 18:51:19+05:30  Cprogrammer
 * Initial revision
 *
 */
