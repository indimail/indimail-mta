/*
 * $Log: $
 */
#include <unistd.h>
#include <substdio.h>
#include <getopt.h>
#ifdef DARWIN
#define opteof -1
#else
#include <sgetopt.h>
#endif
#include <subfd.h>
#include <getln.h>
#include <stralloc.h>
#include <error.h>
#include <errno.h>
#include <strerr.h>
#include <scan.h>
#include <timeoutread.h>
#include <timeoutwrite.h>

#define FATAL "tcpclientrw: fatal: "

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
	struct substdio ssout;
	char            buf[512], outbuf[512];
	static stralloc line = {0};
	ssize_t         n;
	int             r, opt, match;
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
	if ((n = saferead(6, buf, sizeof(buf))) == -1)
		strerr_die2sys(111, FATAL, "read-net: ");
	if (!n) {
		close(7);
		close(6);
		_exit(0);
	}
	if (substdio_put(subfdout, buf, n) || substdio_flush(subfdout))
		strerr_die2sys(111, FATAL, "write-stdout: ");
	substdio_fdbuf(&ssout, safewrite, 7, outbuf, sizeof(outbuf));
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
			if (getln(subfdin, &line, &match, '\n') == -1)
				strerr_die2sys(111, FATAL, "getln: read-stdin: ");
			if (line.len == 0 || !match)
				break;
			if (substdio_put(&ssout, line.s, line.len) || substdio_flush(&ssout))
				strerr_die2sys(111, FATAL, "write-net: ");
		}
		if (FD_ISSET(6, &rfds)) {
			if ((n = saferead(6, buf, sizeof(buf))) == -1)
				strerr_die2sys(111, FATAL, "read-net: ");
			if (!n) {
				close(6);
				close(7);
				_exit(0);
			}
			if (substdio_put(subfdout, buf, n) || substdio_flush(subfdout))
				strerr_die2sys(111, FATAL, "write-stdout: ");
		}
	}
	_exit(0);
}
