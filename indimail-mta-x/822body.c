/*
 * $Log: 822body.c,v $
 * Revision 1.6  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2024-01-23 01:19:48+05:30  Cprogrammer
 * include buffer_defs.h for buffer size definitions
 *
 * Revision 1.4  2022-10-30 17:54:31+05:30  Cprogrammer
 * converted to ansic prototype
 *
 * Revision 1.3  2004-10-22 19:41:53+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-06-16 09:12:57+05:30  Cprogrammer
 * added error checks
 *
 * Revision 1.1  2004-06-09 23:14:12+05:30  Cprogrammer
 * Initial revision
 *
 * From qtools-0.56
 */
#include <unistd.h>
#include "substdio.h"
#include "strerr.h"
#include "error.h"
#include "getln.h"
#include "mess822.h"
#include "buffer_defs.h"

#define FATAL "822body: fatal: "

stralloc        line = { 0 };
int             match;
static char     ssinbuf[BUFSIZE_IN];
static substdio ssin = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) read, 0, ssinbuf, sizeof ssinbuf);
static char     ssoutbuf[BUFSIZE_OUT];
static substdio ssout = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 1, ssoutbuf, sizeof ssoutbuf);

int
main(int argc, char **argv)
{
	int             r;

	for (;;) {
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!mess822_ok(&line))
			break;
		if (!match)
		{
			line.len = 0;
			break;
		}
	}
	if (match) {
		r = 0;
		--line.len;
		++r;
		if (line.len && (line.s[line.len - 1] == '\r')) {
			--line.len;
			++r;
		}
		if (line.len)
			line.len += r;
	}
	if (substdio_put(&ssout, line.s, line.len) ||
			substdio_copy(&ssout, &ssin) ||
			substdio_flush(&ssout))
		strerr_die2sys(111, FATAL, "unable to write: ");
	_exit(0);
}

void
getversion_822body_c()
{
	const char     *x = "$Id: 822body.c,v 1.6 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
