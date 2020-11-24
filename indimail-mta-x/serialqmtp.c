/*
 * $Log: serialqmtp.c,v $
 * Revision 1.7  2020-11-24 13:48:07+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.6  2008-07-15 19:53:48+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.5  2005-08-23 17:35:49+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.4  2004-10-22 20:30:14+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-10-22 15:39:01+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.2  2004-07-15 23:33:04+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.1  2004-05-14 00:45:11+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <sys/stat.h>
#include "strerr.h"
#include "getln.h"
#include "substdio.h"
#include "stralloc.h"
#include "subfd.h"
#include "sig.h"
#include "timeoutread.h"
#include "timeoutwrite.h"
#include "fd.h"
#include "open.h"
#include "wait.h"
#include "str.h"
#include "fmt.h"
#include "env.h"
#include <unistd.h>

#define FATAL "serialqmtp: fatal: "

char           *remoteip;
char           *prefix;
unsigned int    prefixlen;
char            netbuf[2048];
substdio        ssnet;			/*- in child: write 7; in parent: read 6 */
stralloc        line = { 0 };
stralloc        fn = { 0 };
int             match;


/*
 * ------------------------------------------------------------------- CHILD 
 */

ssize_t
safewrite(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	int             w;
	if ((w = timeoutwrite(73, fd, buf, len)) <= 0)
		_exit(31);
	return w;
}

char            num[FMT_ULONG];
char            num2[FMT_ULONG];
stralloc        recipient = { 0 };
stralloc        sender = { 0 };

char            inbuf[2048];

void
doit(fd)
	int             fd;
{
	struct stat     st;
	unsigned long   len;
	int             len2;
	char           *x;
	int             n;
	substdio        ssin;

	if (fstat(fd, &st) == -1)
		_exit(35);
	len = 0;
	substdio_fdbuf(&ssin, read, fd, inbuf, sizeof inbuf);
	if (getln(&ssin, &line, &match, '\n') == -1)
		_exit(32);
	len += line.len;
	if (!match)
		return;
	if (!stralloc_starts(&line, "Return-Path: <"))
		return;
	if (line.s[line.len - 2] != '>')
		return;
	if (line.s[line.len - 1] != '\n')
		return;
	if (!stralloc_copyb(&sender, line.s + 14, line.len - 16))
		_exit(36);
	if (getln(&ssin, &line, &match, '\n') == -1)
		_exit(32);
	len += line.len;
	if (!match)
		return;
	if (!stralloc_starts(&line, "Delivered-To: "))
		return;
	if (line.s[line.len - 1] != '\n')
		return;
	if (!stralloc_copyb(&recipient, line.s + 14, line.len - 15))
		_exit(36);
	if (!stralloc_starts(&recipient, prefix))
		return;
	if (st.st_size < len)
		return;					/*- okay, who's the wise guy?  */
	len = st.st_size + 1 - len;
	if (substdio_putflush(subfdoutsmall, fn.s, fn.len) == -1)
		_exit(33);
	/*
	 * must occur before writes to net, to avoid deadlock 
	 */
	substdio_put(&ssnet, num, fmt_ulong(num, len));
	substdio_put(&ssnet, ":\n", 2);
	--len;						/*- for the \n */
	while (len > 0)
	{
		n = substdio_feed(&ssin);
		if (n <= 0)
			_exit(32);			/*- wise guy again */
		x = substdio_PEEK(&ssin);
		substdio_put(&ssnet, x, n);
		substdio_SEEK(&ssin, n);
		len -= n;
	}
	substdio_put(&ssnet, ",", 1);
	len = sender.len;
	substdio_put(&ssnet, num, fmt_ulong(num, len));
	substdio_put(&ssnet, ":", 1);
	substdio_put(&ssnet, sender.s, sender.len);
	substdio_put(&ssnet, ",", 1);
	len = recipient.len - prefixlen;
	len2 = fmt_ulong(num2, len);
	len += len2 + 2;
	substdio_put(&ssnet, num, fmt_ulong(num, len));
	substdio_put(&ssnet, ":", 1);
	substdio_put(&ssnet, num2, len2);
	substdio_put(&ssnet, ":", 1);
	substdio_put(&ssnet, recipient.s + prefixlen, recipient.len - prefixlen);
	substdio_put(&ssnet, ",,", 2);
	substdio_flush(&ssnet);
	return;
}

void
child()	/*- reading from original stdin, writing to parent */
{
	int             fd;

	substdio_fdbuf(&ssnet, safewrite, 7, netbuf, sizeof netbuf);
	for (;;)
	{
		if (getln(subfdinsmall, &fn, &match, '\0') == -1)
			_exit(34);
		if (!match)
			return;
		if ((fd = open_read(fn.s)) == -1)
			_exit(35);
		doit(fd);
		close(fd);
	}
}


/*
 * ------------------------------------------------------------------ PARENT 
 */

void
die_proto()
{
	strerr_die2x(111, FATAL, "remote protocol violation");
}

void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

void
die_output()
{
	strerr_die2sys(111, FATAL, "unable to write output: ");
}

ssize_t
saferead(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	int             r;
	if (!(r = timeoutread(3600, fd, buf, len)))
		strerr_die2x(111, FATAL, "network read error: end of file");
	if (r < 0)
		strerr_die2sys(111, FATAL, "network read error: ");
	return r;
}

void
parent()	/*- reading from child, writing to original stdout */
{
	unsigned int    len;
	unsigned char   ch;

	substdio_fdbuf(&ssnet, saferead, 6, netbuf, sizeof netbuf);
	for (;;)
	{
		if (getln(subfdinsmall, &fn, &match, '\0') == -1)
			strerr_die2sys(111, FATAL, "unable to read from child: ");
		if (!match)
			return;
		len = 0;
		for (;;)
		{
			substdio_get(&ssnet, (char *) &ch, 1);
			if (ch == ':')
				break;
			if (len > 200000000)
				die_proto();
			if (ch - '0' > 9)
				die_proto();
			len = 10 * len + (ch - '0');
		}
		if (!len)
			die_proto();
		substdio_get(&ssnet, (char *) &ch, 1);
		--len;
		if ((ch != 'Z') && (ch != 'D') && (ch != 'K'))
			die_proto();
		if (!stralloc_copyb(&line, (char *) &ch, 1))
			die_nomem();
		if (remoteip)
		{
			if (!stralloc_cats(&line, remoteip))
				die_nomem();
			if (!stralloc_cats(&line, " said: "))
				die_nomem();
		}
		while (len > 0)
		{
			substdio_get(&ssnet, (char *) &ch, 1);
			if (line.len < 2000 && !stralloc_append(&line, (char *) &ch))
				die_nomem();
			--len;
		}
		for (len = 0; len < line.len; ++len)
		{
			ch = line.s[len];
			if ((ch < 32) || (ch > 126))
				line.s[len] = '?';
		}
		if (!stralloc_append(&line, "\n"))
			die_nomem();
		if (substdio_put(subfdoutsmall, fn.s, fn.len) == -1)
			die_output();
		if (substdio_put(subfdoutsmall, line.s, line.len) == -1)
			die_output();
		if (substdio_flush(subfdoutsmall) == -1)
			die_output();
		substdio_get(&ssnet, (char *) &ch, 1);
		if (ch != ',')
			die_proto();
	}
}


/*
 * -------------------------------------------------------------------- MAIN 
 */

int             pic2p[2];

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	int             wstat;
	int             pid;

	sig_pipeignore();

	remoteip = env_get("TCPREMOTEIP");

	prefix = argv[1];
	if (!prefix)
		strerr_die1x(100, "serialqmtp: usage: serialqmtp prefix");
	prefixlen = str_len(prefix);

	if (pipe(pic2p) == -1)
		strerr_die2sys(111, FATAL, "unable to create pipe: ");

	pid = fork();
	if (pid == -1)
		strerr_die2sys(111, FATAL, "unable to fork: ");

	if (!pid)
	{
		close(pic2p[0]);
		fd_move(1, pic2p[1]);
		child();
		_exit(0);
	}

	close(pic2p[1]);
	fd_move(0, pic2p[0]);
	parent();

	if (wait_pid(&wstat, pid) == -1)
		strerr_die2sys(111, FATAL, "unable to get child status: ");
	if (wait_crashed(wstat))
		strerr_die2x(111, FATAL, "child crashed");
	switch (wait_exitcode(wstat))
	{
	case 0:
		_exit(0);
	case 31:
		strerr_die2x(111, FATAL, "unable to write to network");
	case 32:
		strerr_die2x(111, FATAL, "unable to read file");
	case 34:
		strerr_die2x(111, FATAL, "unable to read input");
	case 35:
		strerr_die2x(111, FATAL, "unable to open file");
	case 36:
		die_nomem();
	}
	strerr_die2x(111, FATAL, "internal error");
	/*- Not reached */
	return(0);
}

void
getversion_serialqmtp_c()
{
	static char    *x = "$Id: serialqmtp.c,v 1.7 2020-11-24 13:48:07+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
