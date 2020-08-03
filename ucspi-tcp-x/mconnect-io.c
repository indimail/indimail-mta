/*
 * $Log: mconnect-io.c,v $
 * Revision 1.4  2008-07-25 16:49:26+05:30  Cprogrammer
 * fix for darwin
 *
 * Revision 1.3  2008-07-17 23:03:52+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.2  2005-06-10 09:12:57+05:30  Cprogrammer
 * removed fork.h
 *
 * Revision 1.1  2003-12-31 19:46:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sig.h>
#include <wait.h>
#include <substdio.h>
#include <subfd.h>
#include <strerr.h>
#include <unistd.h>
#include <signal.h>

char            outbuf[512];
substdio        bout;

char            inbuf[512];
substdio        bin;

ssize_t
myread(int fd, char *buf, int len)
{
	substdio_flush(&bout);
	return read(fd, buf, len);
}

int
main()
{
	int             pid;
	int             wstat;
	char            ch;

	sig_ignore(sig_pipe);

	pid = fork();
	if (pid == -1)
		strerr_die2sys(111, "mconnect-io: fatal: ", "unable to fork: ");

	if (!pid) {
		substdio_fdbuf(&bin, myread, 0, inbuf, sizeof inbuf);
		substdio_fdbuf(&bout, write, 7, outbuf, sizeof outbuf);

		while (substdio_get(&bin, &ch, 1) == 1) {
			if (ch == '\n')
				substdio_put(&bout, "\r", 1);
			substdio_put(&bout, &ch, 1);
		}
		_exit(0);
	}

	substdio_fdbuf(&bin, myread, 6, inbuf, sizeof inbuf);
	substdio_fdbuf(&bout, write, 1, outbuf, sizeof outbuf);

	while (substdio_get(&bin, &ch, 1) == 1)
		substdio_put(&bout, &ch, 1);

	kill(pid, sig_term);
	wait_pid(&wstat, pid);

	_exit(0);
	/*- Not reached */
}
