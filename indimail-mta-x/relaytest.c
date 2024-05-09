/*
 * $Log: relaytest.c,v $
 * Revision 1.8  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2023-10-07 08:42:37+05:30  Cprogrammer
 * updated with original author
 *
 */
#include <unistd.h>
#include <substdio.h>
#include <stralloc.h>
#include <getln.h>
#include <strerr.h>
#include <byte.h>
#include <scan.h>
#include <timeoutread.h>
#include <timeoutwrite.h>
#include <noreturn.h>

#define FATAL "relaytest: fatal: "

ssize_t         saferead(int fd, char *buf, size_t len);
ssize_t         safewrite(int fd, char *buf, size_t len);

static char     buf6[2048], buf7[2048], ssoutbuf[512];
static substdio ss6 = SUBSTDIO_FDBUF(saferead, 6, buf6, sizeof buf6);
static substdio ss7 = SUBSTDIO_FDBUF(safewrite, 7, buf7, sizeof buf7);
static substdio ssout = SUBSTDIO_FDBUF(safewrite, 1, ssoutbuf, sizeof ssoutbuf);
static stralloc smtpline = { 0 };

no_return void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

no_return void
die_output()
{
	strerr_die2sys(111, FATAL, "unable to write output: ");
}

no_return void
die_readmess()
{
	strerr_die2sys(111, FATAL, "unable to read file: ");
}

no_return void
die_neteof()
{
	strerr_die2x(111, FATAL, "network read error: end of file");
}

no_return void
die_netread()
{
	strerr_die2sys(111, FATAL, "network read error: ");
}

no_return void
die_netwrite()
{
	strerr_die2sys(111, FATAL, "network write error: ");
}

no_return void
die_proto()
{
	strerr_die2x(111, FATAL, "protocol violation");
}

ssize_t
saferead(int fd, char *buf, size_t len)
{
	int             r;

	if (!(r = timeoutread(81, fd, buf, len)))
		die_neteof();
	if (r < 0)
		die_netread();
	return r;
}

ssize_t
safewrite(int fd, char *buf, size_t len)
{
	int             r;

	if ((r = timeoutwrite(73, fd, buf, len)) <= 0)
		die_netwrite();
	return r;
}

unsigned long
smtpcode()
{
	unsigned long   code;
	int             match;
	char            num[4];

	do {
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
	} while (smtpline.s[3] == '-');
	substdio_flush(&ssout);
	return code;
}

void
quit()	/*- what a stupid protocol */
{
	substdio_putsflush(&ssout, "client: ");
	substdio_putsflush(&ssout, "QUIT\n");
	if (substdio_putsflush(&ss7, "QUIT\r\n"))
		die_netwrite();
	smtpcode();
}

int
main(int argc, char **argv)
{
	if (smtpcode() != 220) {
		quit();
		strerr_die2x(111, FATAL, "connected but greeting failed");
	}
	substdio_puts(&ssout, "client: ");
	substdio_putsflush(&ssout, "EHLO indimail.org\n");
	if (substdio_putsflush(&ss7, "EHLO indimail.org\r\n"))
		die_netwrite();
	if (smtpcode() != 250) {
		substdio_puts(&ssout, "client: ");
		substdio_putsflush(&ssout, "HELO indimail.org\n");
		if (substdio_putsflush(&ss7, "HELO indimail.org\r\n"))
			die_netwrite();
		if (smtpcode() != 250) {
			quit();
			_exit(1);
		}
	}
	substdio_puts(&ssout, "client: ");
	substdio_putsflush(&ssout, "MAIL FROM: test@indimail.org\n");
	if (substdio_putsflush(&ss7, "MAIL FROM: test@indimail.org\r\n"))
		die_netwrite();
	if (smtpcode() != 250) {
		quit();
		_exit(1);
	}
	substdio_puts(&ssout, "client: ");
	substdio_putsflush(&ssout, "RCPT TO: test@relay-test-invalid.com\n");
	if (substdio_putsflush(&ss7, "RCPT TO: test@relay-test-invalid.com\r\n"))
		die_netwrite();
	if (smtpcode() != 250) {
			quit();
			_exit(1);
	}
	substdio_puts(&ssout, "client: ");
	substdio_putsflush(&ssout, "DATA\n");
	if (substdio_putsflush(&ss7, "DATA\r\n"))
		die_netwrite();
	if (smtpcode() != 354) {
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
	const char     *x = "$Id: relaytest.c,v 1.8 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}

/*
 * $Log: relaytest.c,v $
 * Revision 1.8  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.7  2023-10-07 08:42:37+05:30  Cprogrammer
 * updated with original author
 *
 * Revision 1.6  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.5  2016-01-02 19:23:25+05:30  Cprogrammer
 * use indimail.org
 *
 * Revision 1.4  2011-07-29 09:29:58+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.3  2008-07-15 19:53:12+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.2  2005-06-15 22:35:24+05:30  Cprogrammer
 * added rcs version information
 *
 * Revision 1.1  2005-06-15 22:11:43+05:30  Cprogrammer
 * Initial revision
 *
 * Original by Matthew Trout
 */
