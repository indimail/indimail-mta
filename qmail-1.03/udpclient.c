/*
 * $Log: udpclient.c,v $
 * Revision 1.3  2015-12-31 08:33:52+05:30  Cprogrammer
 * copy data from stdin if message argument not specified on command line
 *
 * Revision 1.2  2015-08-19 16:25:39+05:30  Cprogrammer
 * fixed typo
 *
 * Revision 1.1  2015-04-10 19:37:18+05:30  Cprogrammer
 * Initial revision
 *
 *
 */
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "sgetopt.h"
#include "udpopen.h"
#include "strerr.h"
#include "subfd.h"

char           *usage = "usage: udpclient [-h host] [-p port] message message ...";

#define FATAL "udpclient: fatal: "

void
out(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfdout, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

int
main(int argc, char **argv)
{
	int             sfd, opt;
	char           *host = 0, *port = 0;

	while ((opt = getopt(argc, argv, "h:p:")) != opteof) {
		switch (opt) {
		case 'h':
			host = optarg;
			break;
		case 'p':
			port = optarg;
			break;
		default:
			strerr_die1x(100, usage);
			break;
		}
	} /*- while ((opt = getopt(argc, argv, "dw:s:t:g:m:")) != opteof) */
	if (!host)
		strerr_die1x(100, usage);

	if ((sfd = udpopen(host, port)) == -1)
	{
		if (substdio_puts(subfderr, FATAL) == -1)
			strerr_die2sys(111, FATAL, "write: ");
		if (substdio_puts(subfderr, "udpopen: ") == -1)
			strerr_die2sys(111, FATAL, "write: ");
		if (substdio_puts(subfderr, strerror(errno)) == -1)
			strerr_die2sys(111, FATAL, "write: ");
		if (substdio_put(subfderr, "\n", 1) == -1)
			strerr_die2sys(111, "udpopen", "write: ");
		if (substdio_flush(subfderr) == -1)
			strerr_die2sys(111, FATAL, "write: ");
		_exit(1);
	}
	if (dup2(sfd, 1) == -1)
		strerr_die2sys(111, FATAL, "dup2: ");
	if (optind == argc) {
		switch (substdio_copy(subfdout, subfdin))
		{
		case -2:
			strerr_die2sys(111, FATAL, "read: ");
		case -3:
			strerr_die2sys(111, FATAL, "write: ");
		}
	} else
	while (optind < argc) {
		out(argv[optind++]);
		if (optind < argc)
			out(" ");
	}
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	close(sfd);
	_exit(0);
}

void
getversion_udpclient_c()
{
	static char    *x = "$Id: udpclient.c,v 1.3 2015-12-31 08:33:52+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
