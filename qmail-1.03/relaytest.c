/*
 * $Log: relaytest.c,v $
 * Revision 1.3  2008-07-15 19:53:12+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.2  2005-06-15 22:35:24+05:30  Cprogrammer
 * added rcs version information
 *
 * Revision 1.1  2005-06-15 22:11:43+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include "substdio.h"
#include "stralloc.h"
#include "getln.h"
#include "strerr.h"
#include "byte.h"
#include "scan.h"
#include "timeoutread.h"
#include "timeoutwrite.h"

#define FATAL "relaytest: fatal: "

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

void
die_readmess()
{
	strerr_die2sys(111, FATAL, "unable to read file: ");
}

void
die_neteof()
{
	strerr_die2x(111, FATAL, "network read error: end of file");
}

void
die_netread()
{
	strerr_die2sys(111, FATAL, "network read error: ");
}

void
die_netwrite()
{
	strerr_die2sys(111, FATAL, "network write error: ");
}

void
die_proto()
{
	strerr_die2x(111, FATAL, "protocol violation");
}

ssize_t
saferead(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	int             r;
	if (!(r = timeoutread(81, fd, buf, len)))
		die_neteof();
	if (r < 0)
		die_netread();
	return r;
}

ssize_t
safewrite(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
{
	int             r;

	if ((r = timeoutwrite(73, fd, buf, len)) <= 0)
		die_netwrite();
	return r;
}

char            buf6[2048];
substdio        ss6 = SUBSTDIO_FDBUF(saferead, 6, buf6, sizeof buf6);
char            buf7[2048];
substdio        ss7 = SUBSTDIO_FDBUF(safewrite, 7, buf7, sizeof buf7);
static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF(safewrite, 1, ssoutbuf, sizeof ssoutbuf);
stralloc        smtpline = { 0 };

unsigned long
smtpcode()
{
	unsigned long   code;
	int             flagfirst;
	int             match;
	char            num[4];

	flagfirst = 1;
	do
	{
		if (getln(&ss6, &smtpline, &match, '\n') != 0)
			die_proto();
		if (!match)
			die_proto();
		substdio_puts(&ssout, "server: ");
		substdio_put(&ssout, smtpline.s, smtpline.len);
		if ((smtpline.len >= 1) && (smtpline.s[smtpline.len - 1] == '\n'))
			--smtpline.len;
		if ((smtpline.len >= 1) && (smtpline.s[smtpline.len - 1] == '\r'))
			--smtpline.len;
		if (smtpline.len < 3)
			die_proto();
		byte_copy(num, 3, smtpline.s);
		num[3] = 0;
		if (scan_ulong(num, &code) != 3)
			die_proto();
		if (smtpline.len == 3)
			return code;
		flagfirst = 0;
	} while (smtpline.s[3] == '-');
	substdio_flush(&ssout);
	return code;
}

void
quit()	/*- what a stupid protocol */
{
	substdio_putsflush(&ssout, "client: ");
	substdio_putsflush(&ssout, "QUIT\r\n");
	if (substdio_putsflush(&ss7, "QUIT\r\n"))
		die_netwrite();
	smtpcode();
}

int
main(argc, argv)
	int             argc;
	char          **argv;
{

	if (smtpcode() != 220)
	{
		quit();
		strerr_die2x(111, FATAL, "connected but greeting failed");
	}
	substdio_puts(&ssout, "client: ");
	substdio_putsflush(&ssout, "EHLO testindi.com\r\n");
	if (substdio_putsflush(&ss7, "EHLO testindi.com\r\n"))
		die_netwrite();
	if (smtpcode() != 250)
	{
		substdio_puts(&ssout, "client: ");
		substdio_putsflush(&ssout, "HELO testindi.com\r\n");
		if (substdio_putsflush(&ss7, "HELO testindi.com\r\n"))
			die_netwrite();
		if (smtpcode() != 250)
		{
			quit();
			_exit(1);
		}
	}
	substdio_puts(&ssout, "client: ");
	substdio_putsflush(&ssout, "MAIL FROM: test@testindi.com\r\n");
	if (substdio_putsflush(&ss7, "MAIL FROM: test@testindi.com\r\n"))
		die_netwrite();
	if (smtpcode() != 250)
	{
		quit();
		_exit(1);
	}
	substdio_puts(&ssout, "client: ");
	substdio_putsflush(&ssout, "RCPT TO: test@relay-test-invalid.com\r\n");
	if (substdio_putsflush(&ss7, "RCPT TO: test@relay-test-invalid.com\r\n"))
		die_netwrite();
	if (smtpcode() != 250)
	{
			quit();
			_exit(1);
	}
	substdio_puts(&ssout, "client: ");
	substdio_putsflush(&ssout, "DATA\r\n");
	if (substdio_putsflush(&ss7, "DATA\r\n"))
		die_netwrite();
	if (smtpcode() != 354)
	{
		quit();
		_exit(1);
	}
	quit();
	_exit(0);
	/*- Not Reached */
	return(0);
}

void
getversion_relaytest_c()
{
	static char    *x = "$Id: relaytest.c,v 1.3 2008-07-15 19:53:12+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
