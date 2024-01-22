/*
 * $Log: 822bodyfilter.c,v $
 * Revision 1.3  2020-11-24 13:42:08+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.2  2004-10-22 20:13:44+05:30  Cprogrammer
 * removed readwrite.h
 * added RCS id
 *
 * Revision 1.1  2004-07-17 21:04:47+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "error.h"
#include "strerr.h"
#include "substdio.h"
#include "getln.h"
#include "mess822.h"
#include "fd.h"
#include "seek.h"
#include "pathexec.h"
#include "buffer_defs.h"

#define FATAL "822bodyfilter: fatal: "

stralloc        line = { 0 };
int             match;
static char     ssinbuf[BUFSIZE_IN];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);
static char     ssoutbuf[BUFSIZE_OUT];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);

int
main(int argc, char **argv, char **envp)
{
	seek_pos        pos;
	int             r;

	if (!argv[1])
		strerr_die1x(100, "822bodyfilter: usage: 822bodyfilter program [ arg ... ]");
	for (;;)
	{
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!mess822_ok(&line))
			break;
		if (substdio_put(&ssout, line.s, line.len) == -1)
			strerr_die2sys(111, FATAL, "unable to write output: ");
		if (!match)
		{
			line.len = 0;
			break;
		}
	}
	r = 0;
	if (match)
	{
		--line.len;
		++r;
		if (line.len && (line.s[line.len - 1] == '\r'))
		{
			--line.len;
			++r;
		}
		if (!line.len)
		{
			if (substdio_put(&ssout, line.s, r) == -1)
				strerr_die2sys(111, FATAL, "unable to write output: ");
			r = 0;
		}
	}
	if (substdio_flush(&ssout) == -1)
		strerr_die2x(111, FATAL, "unable to write output: ");
	pos = seek_cur(0);
	if (pos == -1)
		strerr_die2sys(111, FATAL, "unable to seek: ");
	if (seek_set(0, pos - ssout.p - line.len - r) == -1)
		strerr_die2sys(111, FATAL, "unable to seek: ");

	pathexec_run(argv[1], argv + 1, envp);
	if (error_temp(errno))
		strerr_die2sys(111, FATAL, "exec failed: ");
	strerr_die2sys(100, FATAL, "exec failed: ");
	/*- Not reached */
	return(0);
}

void
getversion_822bodyfilter_c()
{
	static char    *x = "$Id: 822bodyfilter.c,v 1.3 2020-11-24 13:42:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
