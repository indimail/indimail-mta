/*
 * $Log: 822headerfilter.c,v $
 * Revision 1.6  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2024-01-23 01:20:09+05:30  Cprogrammer
 * include buffer_defs.h for buffer size definitions
 *
 * Revision 1.4  2020-11-24 13:42:56+05:30  Cprogrammer
 * /usr/local/src/projects/rpmbuild/RPMS/x86_64/libqmail-devel-1.0-1.1.fc33.x86_64.rpm
 *
 * Revision 1.3  2004-10-22 20:14:08+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-22 15:33:40+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.1  2004-07-17 20:46:36+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "error.h"
#include "wait.h"
#include "strerr.h"
#include "substdio.h"
#include "getln.h"
#include "mess822.h"
#include "fd.h"
#include "pathexec.h"
#include "buffer_defs.h"

#define FATAL "822headerfilter: fatal: "

stralloc        line = { 0 };
int             match;

char            pipbuf[BUFSIZE_IN];
static char     ssinbuf[BUFSIZE_IN];
static substdio ssin = SUBSTDIO_FDBUF(read, 0, ssinbuf, sizeof ssinbuf);
static char     ssoutbuf[BUFSIZE_OUT];
static substdio ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof ssoutbuf);
static substdio sspip;

int
main(int argc, char **argv, char **envp)
{
	int             pid;
	int             pfi[2];
	int             wstat;

	if (!argv[1])
		strerr_die1x(100, "822headerfilter: usage: 822headerfilter program [ arg ... ]");
	if (pipe(pfi) == -1)
		strerr_die2sys(111, FATAL, "unable to create pipe: ");

	substdio_fdbuf(&sspip, write, pfi[1], pipbuf, sizeof pipbuf);
	pid = fork();
	if (pid == -1)
		strerr_die2sys(111, FATAL, "unable to fork: ");
	if (pid == 0)
	{
		close(pfi[1]);
		if (fd_move(0, pfi[0]) == -1)
			strerr_die2sys(111, FATAL, "unable to arrange file descriptors: ");
		pathexec_run(argv[1], argv + 1, envp);
		if (error_temp(errno))
			strerr_die2sys(111, FATAL, "exec failed: ");
		strerr_die2sys(100, FATAL, "exec failed: ");
	}
	close(pfi[0]);

	for (;;)
	{
		if (getln(&ssin, &line, &match, '\n') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!mess822_ok(&line))
			break;
		if (substdio_put(&sspip, line.s, line.len) == -1)
			strerr_die2sys(111, FATAL, "unable to write output: ");
		if (!match)
		{
			line.len = 0;
			break;
		}
	}
	if (substdio_flush(&sspip) == -1)
		strerr_die2x(111, FATAL, "unable to write output: ");
	close(pfi[1]);
	if (wait_pid(&wstat, pid) == -1)
		strerr_die2x(111, FATAL, "wait failed");
	if (wait_crashed(wstat))
		strerr_die2x(111, FATAL, "child crashed");
	if (wait_exitcode(wstat))
		_exit(wait_exitcode(wstat));
	if (substdio_put(&ssout, line.s, line.len) == -1)
		strerr_die2sys(111, FATAL, "unable to write output: ");
	if (substdio_copy(&ssout, &ssin) != 0)
		strerr_die2sys(111, FATAL, "unable to read input: ");
	if (substdio_flush(&ssout) == -1)
		strerr_die2x(111, FATAL, "unable to write output: ");
	_exit(0);
	/*- Not reached */
	return(0);
}

void
getversion_822headerfilter_c()
{
	const char     *x = "$Id: 822headerfilter.c,v 1.6 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
