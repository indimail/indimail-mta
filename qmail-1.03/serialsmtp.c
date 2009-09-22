/*
 * $Log: serialsmtp.c,v $
 * Revision 1.5  2008-07-15 19:53:54+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.4  2005-06-11 21:32:26+05:30  Cprogrammer
 * added ipv6 address support
 *
 * Revision 1.3  2004-10-22 20:30:14+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-10-22 15:39:05+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.1  2004-05-14 00:45:12+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "strerr.h"
#include "getln.h"
#include "subfd.h"
#include "substdio.h"
#include "open.h"
#include "timeoutread.h"
#include "timeoutwrite.h"
#include "exit.h"
#include "stralloc.h"
#include "sig.h"
#include "str.h"
#include "byte.h"
#include "case.h"
#include "quote.h"
#include "scan.h"
#include "env.h"
#include <unistd.h>

#define FATAL "serialsmtp: fatal: "

void
die_usage()
{
	strerr_die1x(100, "serialsmtp: usage: serialsmtp prefix helohost");
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

void
die_readmess()
{
	strerr_die2sys(111, FATAL, "unable to read file: ");
}

void
die_smtppathetic()
{
	strerr_die2x(111, FATAL, "SMTP cannot transfer messages with partial final lines");
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
	if(!(r = timeoutread(81, fd, buf, len)))
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

	if((r = timeoutwrite(73, fd, buf, len)) <= 0)
		die_netwrite();
	return r;
}

char            buf6[2048];
char            buf7[2048];
substdio        ss6 = SUBSTDIO_FDBUF(saferead,  6, buf6, sizeof buf6);
substdio        ss7 = SUBSTDIO_FDBUF(safewrite, 7, buf7, sizeof buf7);

stralloc        dataline = { 0 };

void
blast(ssfrom)
	substdio       *ssfrom;
{
	int             match;

	for (;;)
	{
		if (getln(ssfrom, &dataline, &match, '\n') == -1)
			die_readmess();
		if (!match && !dataline.len)
		{
			substdio_put(&ss7, ".\r\n", 3);
			substdio_flush(&ss7);
			return;
		}
		if (!match)
			die_smtppathetic();
		--dataline.len;
		if (dataline.len && (dataline.s[0] == '.'))
			substdio_put(&ss7, ".", 1);
		substdio_put(&ss7, dataline.s, dataline.len);
		substdio_put(&ss7, "\r\n", 2);
	}
}

stralloc        smtpline = { 0 };
int             flagpipelining = 0;

unsigned long
smtpcode(flagehlo)
	int             flagehlo;
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
		if (flagehlo && code == 250 && !flagfirst && smtpline.len == 14 && !case_diffb("PIPELINING", 10, smtpline.s + 4))
			flagpipelining = 1;
		if (smtpline.len == 3)
			return code;
		flagfirst = 0;
	} while (smtpline.s[3] == '-');
	return code;
}

void
quit()	/*- what a stupid protocol */
{
	substdio_puts(&ss7, "QUIT\r\n");
	substdio_flush(&ss7);
}


int             flagneedrset = 0;

char            messbuf[4096];
substdio        ssmess;
char           *prefix;
char           *remoteip;

stralloc        line = { 0 };
stralloc        recipient = { 0 };
stralloc        sender = { 0 };
stralloc        quosender = { 0 };
stralloc        quorecip = { 0 };

stralloc        fn = { 0 };

void
result(code)
	unsigned long   code;
{
	char            ch;
	int             i;

	if (code >= 500)
	{
		if (!stralloc_copyb(&line, "D", 1))
			die_nomem();
	} else
	if (code >= 400)
	{
		if (!stralloc_copyb(&line, "Z", 1))
			die_nomem();
	} else
	{
		if (!stralloc_copyb(&line, "K", 1))
			die_nomem();
	}
	if (remoteip)
	{
		if (!stralloc_cats(&line, remoteip))
			die_nomem();
		if (!stralloc_cats(&line, " said: "))
			die_nomem();
	}
	if (!stralloc_cat(&line, &smtpline))
		die_nomem();
	if (line.len > 2000)
		line.len = 2000;
	for (i = 0; i < line.len; ++i)
	{
		ch = line.s[i];
		if ((ch < 32) || (ch > 126))
			line.s[i] = '?';
	}
	if (!stralloc_catb(&line, "\n", 1))
		die_nomem();
	if (substdio_put(subfdoutsmall, fn.s, fn.len) == -1)
		die_output();
	if (substdio_put(subfdoutsmall, line.s, line.len) == -1)
		die_output();
	if (substdio_flush(subfdoutsmall) == -1)
		die_output();
}

void
doit(fd)
	int             fd;
{
	int             match;
	unsigned long   code;

	substdio_fdbuf(&ssmess, read, fd, messbuf, sizeof messbuf);
	if (getln(&ssmess, &line, &match, '\n') == -1)
		die_readmess();
	if (!match)
		return;
	if (!stralloc_starts(&line, "Return-Path: <"))
		return;
	if (line.s[line.len - 2] != '>')
		return;
	if (line.s[line.len - 1] != '\n')
		return;
	if (!stralloc_copyb(&sender, line.s + 14, line.len - 16))
		die_nomem();
	if (getln(&ssmess, &line, &match, '\n') == -1)
		die_readmess();
	if (!match)
		return;
	if (!stralloc_starts(&line, "Delivered-To: "))
		return;
	if (line.s[line.len - 1] != '\n')
		return;
	if (!stralloc_copyb(&recipient, line.s + 14, line.len - 15))
		die_nomem();
	if (!stralloc_starts(&recipient, prefix))
		return;
	if (!stralloc_0(&sender))
		die_nomem();
	if (!quote2(&quosender, sender.s))
		die_nomem();
	if (!stralloc_0(&recipient))
		die_nomem();
	if (!quote2(&quorecip, recipient.s + str_len(prefix)))
		die_nomem();
	if (flagneedrset)
	{
		substdio_puts(&ss7, "RSET\r\n");	/*- what a stupid protocol */
		if (!flagpipelining)
		{
			substdio_flush(&ss7);
			code = smtpcode(0);
			if (code >= 400)
				die_proto();	/*- rejected RSET? tsk */
		}
	}
	substdio_puts(&ss7, "MAIL FROM:<");
	substdio_put(&ss7, quosender.s, quosender.len);
	substdio_puts(&ss7, ">\r\n");
	if (!flagpipelining)
	{
		substdio_flush(&ss7);
		code = smtpcode(0);
		if (code >= 400)
		{
			result(code);
			return;
		}
	}
	substdio_puts(&ss7, "RCPT TO:<");
	substdio_put(&ss7, quorecip.s, quorecip.len);
	substdio_puts(&ss7, ">\r\n");
	if (!flagpipelining)
	{
		substdio_flush(&ss7);
		code = smtpcode(0);
		if (code >= 400)
		{
			result(code);
			return;
		}
	}
	substdio_puts(&ss7, "DATA\r\n");
	substdio_flush(&ss7);
	if (flagpipelining)
	{
		if (flagneedrset)
		{
			code = smtpcode(0);
			if (code >= 400)
				die_proto();
		}
		code = smtpcode(0);
		if (code >= 400)
		{
			result(code);
			if (smtpcode(0) < 400)
				die_proto();	/*- rejected MAIL, accepted RCPT */
			if (smtpcode(0) < 400)
				die_proto();	/*- why does the spec allow this?  */
			return;
		}
		code = smtpcode(0);
		if (code >= 400)
		{
			result(code);
			if (smtpcode(0) < 400)
				die_proto();
			return;
		}
	}
	code = smtpcode(0);
	if (code >= 400)
	{
		result(code);
		return;
	}
	blast(&ssmess);
	result(smtpcode(0));
}

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	char           *helohost;
	int             fd;
	int             match;

	sig_pipeignore();
	prefix = *++argv;
	if (!prefix)
		die_usage();
	helohost = *++argv;
	if (!helohost)
		die_usage();
#ifdef IPV6
	if (!(remoteip = env_get("TCP6REMOTEIP")))
		remoteip = env_get("TCPREMOTEIP");
#else
	remoteip = env_get("TCPREMOTEIP");
#endif
	if (smtpcode(0) != 220)
	{
		quit();
		strerr_die2x(111, FATAL, "connected but greeting failed");
	}
	substdio_puts(&ss7, "EHLO ");
	substdio_puts(&ss7, helohost);
	substdio_puts(&ss7, "\r\n");
	substdio_flush(&ss7);
	if (smtpcode(1) != 250)
	{
		substdio_puts(&ss7, "HELO ");
		substdio_puts(&ss7, helohost);
		substdio_puts(&ss7, "\r\n");
		substdio_flush(&ss7);
		if (smtpcode(0) != 250)
		{
			quit();
			strerr_die2x(111, FATAL, "connected but my name was rejected");
		}
	}
	for (;;)
	{
		if (getln(subfdinsmall, &fn, &match, '\0') == -1)
			strerr_die2sys(111, FATAL, "unable to read input: ");
		if (!match)
			break;
		if((fd = open_read(fn.s)) == -1)
			strerr_die4sys(111, FATAL, "unable to open ", fn.s, ": ");
		doit(fd);
		close(fd);
		flagneedrset = 1;
	}
	if (argc == 3) /*- for ATRN support - Do not quit */
		quit();
	_exit(0);
	/*- Not reached */
	return (0);
}

void
getversion_serialsmtp_c()
{
	static char    *x = "$Id: serialsmtp.c,v 1.5 2008-07-15 19:53:54+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
