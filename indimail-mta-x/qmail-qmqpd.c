/*
 * $Log: qmail-qmqpd.c,v $
 * Revision 1.10  2021-08-29 23:27:08+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.9  2021-06-12 18:26:58+05:30  Cprogrammer
 * removed chdir(auto_qmail)
 *
 * Revision 1.8  2020-11-24 13:47:12+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.7  2008-07-15 19:52:41+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.6  2004-10-22 20:28:42+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-10-22 15:37:31+05:30  Cprogrammer
 * removed readwrite.h.
 *
 * Revision 1.4  2004-07-17 21:21:10+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <unistd.h>
#include <sig.h>
#include <byte.h>
#include <str.h>
#include <substdio.h>
#include <now.h>
#include <fmt.h>
#include <env.h>
#include <noreturn.h>
#include "qmail.h"
#include "received.h"

ssize_t         saferead(int fd, char *_buf, size_t len);
ssize_t         safewrite(int fd, char *_buf, size_t len);

static char     ssinbuf[512];
static char     ssoutbuf[256];
static substdio ssin = SUBSTDIO_FDBUF(saferead, 0, ssinbuf, sizeof ssinbuf);
static substdio ssout = SUBSTDIO_FDBUF(safewrite, 1, ssoutbuf, sizeof ssoutbuf);
static unsigned long   bytesleft = 100;

no_return void
resources()
{
	_exit(111);
}

ssize_t
safewrite(int fd, char *buf, size_t len)
{
	int             r;

	if((r = write(fd, buf, len)) <= 0)
		_exit(0);
	return r;
}

ssize_t
saferead(int fd, char *buf, size_t len)
{
	int             r;
	if ((r = read(fd, buf, len)) <=0)
		_exit(0);
	return r;
}

void
getbyte(ch)
	char           *ch;
{
	if (!bytesleft--)
		_exit(100);
	substdio_get(&ssin, ch, 1);
}

unsigned long
getlen()
{
	unsigned long   len = 0;
	char            ch;

	for (;;)
	{
		getbyte(&ch);
		if (ch == ':')
			return len;
		if (len > 200000000)
			resources();
		len = 10 * len + (ch - '0');
	}
}

void
getcomma()
{
	char            ch;
	getbyte(&ch);
	if (ch != ',')
		_exit(100);
}

struct qmail    qq;

void
identify()
{
	char           *remotehost;
	char           *remoteinfo;
	char           *remoteip;
	char           *local;

	remotehost = env_get("TCPREMOTEHOST");
	if (!remotehost)
		remotehost = "unknown";
	remoteinfo = env_get("TCPREMOTEINFO");
	remoteip = env_get("TCPREMOTEIP");
	if (!remoteip)
		remoteip = "unknown";
	local = env_get("TCPLOCALHOST");
	if (!local)
		local = env_get("TCPLOCALIP");
	if (!local)
		local = "unknown";
	received(&qq, "QMQP", local, remoteip, remotehost, remoteinfo, (char *) 0);
}

char            buf[1000];
char            strnum[FMT_ULONG];

int
getbuf()
{
	unsigned long   len;
	int             i;

	len = getlen();
	if (len >= 1000)
	{
		for (i = 0; i < len; ++i)
			getbyte(buf);
		getcomma();
		buf[0] = 0;
		return 0;
	}

	for (i = 0; i < len; ++i)
		getbyte(buf + i);
	getcomma();
	buf[len] = 0;
	return byte_chr(buf, len, '\0') == len;
}

int             flagok = 1;

int
main()
{
	char           *result;
	unsigned long   qp;
	unsigned long   len;
	char            ch;

	sig_pipeignore();
	sig_alarmcatch(resources);
	alarm(3600);

	bytesleft = getlen();

	len = getlen();

	if (qmail_open(&qq) == -1)
		resources();
	qp = qmail_qp(&qq);
	identify();

	while (len > 0)
	{/*- XXX: could speed this up */
		getbyte(&ch);
		--len;
		qmail_put(&qq, &ch, 1);
	}
	getcomma();
	if (getbuf())
		qmail_from(&qq, buf);
	else
	{
		qmail_from(&qq, "");
		qmail_fail(&qq);
		flagok = 0;
	}
	while (bytesleft)
	{
		if (getbuf())
			qmail_to(&qq, buf);
		else
		{
			qmail_fail(&qq);
			flagok = 0;
		}
	}
	bytesleft = 1;
	getcomma();
	result = qmail_close(&qq);
	if (!*result)
	{
		len = fmt_str(buf, "Kok ");
		len += fmt_ulong(buf + len, (unsigned long) now());
		len += fmt_str(buf + len, " qp ");
		len += fmt_ulong(buf + len, qp);
		buf[len] = 0;
		result = buf;
	}
	if (!flagok)
		result = "Dsorry, I can't accept addresses like that (#5.1.3)";
	substdio_put(&ssout, strnum, fmt_ulong(strnum, (unsigned long) str_len(result)));
	substdio_puts(&ssout, ":");
	substdio_puts(&ssout, result);
	substdio_puts(&ssout, ",");
	substdio_flush(&ssout);
	return(0);
}

void
getversion_qmail_qmqpd_c()
{
	static char    *x = "$Id: qmail-qmqpd.c,v 1.10 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
