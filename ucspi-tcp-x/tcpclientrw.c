/*
 * $Log: tcpclientrw.c,v $
 * Revision 1.3  2021-03-03 21:54:42+05:30  Cprogrammer
 * use saferead(), safewrite() instead of substdio
 *
 * Revision 1.2  2021-03-03 13:46:14+05:30  Cprogrammer
 * fixed compiler warning
 *
 * Revision 1.1  2021-03-03 13:42:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <getopt.h>
#ifdef DARWIN
#define opteof -1
#else
#include <sgetopt.h>
#endif
#include <stralloc.h>
#include <error.h>
#include <errno.h>
#include <strerr.h>
#include <scan.h>
#include <timeoutread.h>
#include <timeoutwrite.h>

#define FATAL "tcpclientrw: fatal: "

#ifndef	lint
static char     sccsid[] = "$Id: tcpclientrw.c,v 1.3 2021-03-03 21:54:42+05:30 Cprogrammer Exp mbhangui $";
#endif

unsigned long   timeout = 300;

ssize_t
safewrite(int fd, char *buf, int len)
{
	ssize_t         r;

	if ((r = timeoutwrite(timeout, fd, buf, len)) < 0) {
		if (errno == error_timeout)
			strerr_die2x(2, FATAL, "write timeout: ");
		else
			strerr_die2sys(1, FATAL, "write: ");
	}
	return r;
}

ssize_t
saferead(int fd, char *buf, int len)
{
	ssize_t         r;

	if ((r = timeoutread(timeout, fd, buf, len)) == -1) {
		if (errno == error_timeout)
			strerr_die2x(4, FATAL, "read timeout: ");
		else
			strerr_die2sys(3, FATAL, "read: ");
	}
	return r;
}

int
main(int argc, char **argv)
{
	char            buf[512];
	ssize_t         n;
	int             r, opt;
	fd_set          rfds;	/*- File descriptor mask for select -*/

	while ((opt = getopt(argc, argv, "t:")) != opteof) {
		switch (opt)
		{
		case 't':
			scan_ulong(optarg, &timeout);
			break;
		default:
			strerr_die1x(100, "usage: tcpclientrw [-t timeout]\n");
		}
	}
	for (;;) {
		FD_ZERO(&rfds);
		FD_SET(0, &rfds);
		FD_SET(6, &rfds);
		if ((r = select(7, &rfds, (fd_set *) NULL, (fd_set *) NULL, 0)) < 0) {
#ifdef ERESTART
			if (errno == EINTR || errno == ERESTART)
#else
			if (errno == EINTR)
#endif
				continue;
			strerr_die2sys(111, FATAL, "select: ");
		}
		if (FD_ISSET(0, &rfds)) {
			if ((n = read(0, buf, sizeof(buf))) == -1)
				strerr_die2sys(3, FATAL, "read-stdin: ");
			if (!n)
				break;
			if ((n = safewrite(7, buf, n)) == -1)
				strerr_die2sys(1, FATAL, "write-network: ");
		}
		if (FD_ISSET(6, &rfds)) {
			if ((n = saferead(6, buf, sizeof(buf))) == -1)
				strerr_die2sys(3, FATAL, "read-network: ");
			if (!n) {
				close(6);
				close(7);
				_exit(0);
			}
			if ((n = safewrite(1, buf, n)) == -1)
				strerr_die2sys(1, FATAL, "write-stdout: ");
		}
	}
	_exit(0);
}

void
getversion_tcpclientrw_c()
{
	if (write(1, sccsid, 0) == -1)
		;
}
