/*
 * $Log: udpclient.c,v $
 * Revision 1.7  2022-05-10 20:56:17+05:30  Cprogrammer
 * use headers from standard include path
 *
 * Revision 1.6  2021-04-29 20:30:32+05:30  Cprogrammer
 * moved variable inside if block
 *
 * Revision 1.5  2018-06-01 15:17:29+05:30  Cprogrammer
 * use strerr_die2sys to display error message
 *
 * Revision 1.4  2016-04-18 16:44:33+05:30  Cprogrammer
 * added option to read from socket
 *
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
#include <sgetopt.h>
#include <udpopen.h>
#include <strerr.h>
#include <subfd.h>
#include <scan.h>
#include <timeoutread.h>

char           *usage = "usage: udpclient [-h host] [-p port] [-r responsesize] [-t timeout] message message ...";

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
	int             sfd, opt, timeout = 5, maxresponsesize = -1;
	char            buf[1024];
	char           *host = 0, *port = 0;

	while ((opt = getopt(argc, argv, "h:p:r:t:")) != opteof) {
		switch (opt) {
		case 'h':
			host = optarg;
			break;
		case 'p':
			port = optarg;
			break;
		case 't':
			scan_int(optarg, &timeout);
			break;
		case 'r':
			scan_int(optarg, &maxresponsesize);
			break;
		default:
			strerr_die1x(100, usage);
			break;
		}
	} /*- while ((opt = getopt(argc, argv, "dw:s:t:g:m:")) != opteof) */
	if (!host)
		strerr_die1x(100, usage);

	if ((sfd = udpopen(host, port)) == -1)
		strerr_die2sys(111, FATAL, "udpopen: ");
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
	if (maxresponsesize > 0) {
		int             r;
		if ((r = timeoutread(timeout, sfd, buf, maxresponsesize)) == -1)
			strerr_die2sys(111, FATAL, "read: ");
		if (r > 0) {
			if (substdio_bput(subfderr, buf, r) == -1)
				strerr_die2sys(111, FATAL, "write: ");
			if (substdio_flush(subfderr) == -1)
				strerr_die2sys(111, FATAL, "write: ");
		}
	}
	close(1);
	close(sfd);
	_exit(0);
}

void
getversion_udpclient_c()
{
	static char    *x = "$Id: udpclient.c,v 1.7 2022-05-10 20:56:17+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
