/*
 * $Id: qmail-qmtpd.c,v 1.18 2023-10-07 01:25:42+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <stralloc.h>
#include <substdio.h>
#include <now.h>
#include <str.h>
#include <fmt.h>
#include <env.h>
#include <sig.h>
#include <scan.h>
#include <buffer_defs.h>
#include "qmail.h"
#include "rcpthosts.h"
#include "control.h"
#include "received.h"

void
badproto()
{
	_exit(100);
}

void
resources()
{
	_exit(111);
}

ssize_t
safewrite(int fd, char *buf, size_t len)
{
	int             r;

	if ((r = write(fd, buf, len)) <= 0)
		_exit(0);
	return r;
}

char            ssoutbuf[256];
substdio        ssout = SUBSTDIO_FDBUF(safewrite, 1, ssoutbuf, sizeof ssoutbuf);

ssize_t
saferead(int fd, char *buf, size_t len)
{
	int             r;

	substdio_flush(&ssout);
	if ((r = read(fd, buf, len)) <= 0)
		_exit(0);
	return r;
}

char            ssinbuf[BUFSIZE_OUT];
substdio        ssin = SUBSTDIO_FDBUF(saferead, 0, ssinbuf, sizeof ssinbuf);

unsigned long
getlen()
{
	unsigned long   len = 0;
	char            ch;

	for (;;) {
		substdio_get(&ssin, &ch, 1);
		if (ch == ':')
			return len;
		/* trap non-numeric input in netstring: */
		if ((ch < '0') || (ch > '9'))
			badproto();
		len = 10 * len + (ch - '0');
		if (len > 200000000 || ch < '0' || ch > '9')
			resources();
	}
}

void
getcomma()
{
	char            ch;

	substdio_get(&ssin, &ch, 1);
	if (ch != ',')
		badproto();
}

unsigned int    databytes = 0, bytestooverflow = 0;
struct qmail    qq;
char            buf[1000], buf2[100];
char           *remotehost, *remoteinfo, *remoteip, *local, *relayclient;
stralloc        failure = { 0 };
int             relayclientlen;

int
main()
{
	char            ch;
	unsigned long   biglen, len, qp, u;
	int             i, flagdos = 0, flagsenderok, flagbother, hide_host;
	char           *result, *x;

	sig_pipeignore();
	sig_alarmcatch(resources);
	alarm(3600);

	hide_host = env_get("HIDE_HOST") ? 1 : 0;
	if (control_init() == -1)
		resources();
	if (rcpthosts_init() == -1)
		resources();
	relayclient = env_get("RELAYCLIENT");
	relayclientlen = relayclient ? str_len(relayclient) : 0;

	if(!(x = env_get("DATABYTES"))) {
		if (control_readint((int *) &databytes, "databytes") == -1)
			resources();
	} else {
		scan_ulong(x, &u);
		databytes = u;
	}
	if (!(databytes + 1))
		--databytes;
	if (!(remotehost = env_get("TCPREMOTEHOST")))
		remotehost = "unknown";
	remoteinfo = env_get("TCPREMOTEINFO");
	if (!(remoteip = env_get("TCPREMOTEIP")))
		remoteip = "unknown";
	if (!(local = env_get("TCPLOCALHOST")))
		local = env_get("TCPLOCALIP");
	if (!local)
		local = "unknown";
	for (;;) {
		if (!stralloc_copys(&failure, ""))
			resources();
		flagsenderok = 1;
		if (!(len = getlen()))
			badproto();
		if (databytes)
			bytestooverflow = databytes + 1;
		if (qmail_open(&qq) == -1)
			resources();
		qp = qmail_qp(&qq);
		substdio_get(&ssin, &ch, 1);
		--len;
		if (ch == 10)
			flagdos = 0;
		else
		if (ch == 13)
			flagdos = 1;
		else
			badproto();
		received(&qq, "qmtpd", "QMTP", local, remoteip,
				str_diff(remotehost, "unknown") ? remotehost : 0, remoteinfo,
				(char *) 0, hide_host);
		/*
		 * XXX: check for loops? only if len is big?
		 */
		if (flagdos) {
			while (len > 0) {
				substdio_get(&ssin, &ch, 1);
				--len;
				while ((ch == 13) && len) {
					substdio_get(&ssin, &ch, 1);
					--len;
					if (ch == 10)
						break;
					if (bytestooverflow)
						if (!--bytestooverflow)
							qmail_fail(&qq);
					qmail_put(&qq, "\015", 1);
				}
				if (bytestooverflow)
					if (!--bytestooverflow)
						qmail_fail(&qq);
				qmail_put(&qq, &ch, 1);
			}
		} else {
			if (databytes) {
				if (len > databytes) {
					bytestooverflow = 0;
					qmail_fail(&qq);
				}
			}
			while (len > 0) { /*- XXX: could speed this up, obviously */
				substdio_get(&ssin, &ch, 1);
				--len;
				qmail_put(&qq, &ch, 1);
			}
		}
		getcomma();
		len = getlen();
		if (len >= 1000) {
			buf[0] = 0;
			flagsenderok = 0;
			for (i = 0; i < len; ++i)
				substdio_get(&ssin, &ch, 1);
		} else {
			for (i = 0; i < len; ++i) {
				substdio_get(&ssin, buf + i, 1);
				if (!buf[i])
					flagsenderok = 0;
			}
			buf[len] = 0;
		}
		getcomma();
		flagbother = 0;
		qmail_from(&qq, buf);
		if (!flagsenderok)
			qmail_fail(&qq);
		biglen = getlen();
		while (biglen > 0) {
			if (!stralloc_append(&failure, ""))
				resources();
			len = 0;
			for (;;) {
				if (!biglen)
					badproto();
				substdio_get(&ssin, &ch, 1);
				--biglen;
				if (ch == ':')
					break;
				/* trap non-numeric input in netstring: */
				if ((ch < '0') || (ch > '9'))
					badproto();
				len = 10 * len + (ch - '0');
				if (len > 200000000 || ch < '0' || ch > '9')
					resources();
			}
			if (len >= biglen)
				badproto();
			if (len + relayclientlen >= 1000) {
				failure.s[failure.len - 1] = 'L';
				for (i = 0; i < len; ++i)
					substdio_get(&ssin, &ch, 1);
			} else {
				for (i = 0; i < len; ++i) {
					substdio_get(&ssin, buf + i, 1);
					if (!buf[i])
						failure.s[failure.len - 1] = 'N';
				}
				buf[len] = 0;

				if (relayclient)
					str_copy(buf + len, relayclient);
				else {
					switch (rcpthosts(buf, len, 0))
					{
					case -1:
						resources();
					case 0:
						failure.s[failure.len - 1] = 'D';
					}
				}
				if (!failure.s[failure.len - 1]) {
					qmail_to(&qq, buf);
					flagbother = 1;
				}
			}
			getcomma();
			biglen -= (len + 1);
		}
		getcomma();
		if (!flagbother)
			qmail_fail(&qq);
		result = qmail_close(&qq);
		if (!flagsenderok)
			result = "Dunacceptable sender (#5.1.7)";
		if (databytes) {
			if (!bytestooverflow)
				result = "Dsorry, that message size exceeds my databytes limit (#5.3.4)";
		}
		if (*result)
			len = str_len(result);
		else {
			/*- success!  */
			len = 0;
			len += fmt_str(buf2 + len, "Kok ");
			len += fmt_ulong(buf2 + len, (unsigned long) now());
			len += fmt_str(buf2 + len, " qp ");
			len += fmt_ulong(buf2 + len, qp);
			buf2[len] = 0;
			result = buf2;
		}
		len = fmt_ulong(buf, len);
		buf[len++] = ':';
		len += fmt_str(buf + len, result);
		buf[len++] = ',';
		for (i = 0; i < failure.len; ++i) {
			switch (failure.s[i])
			{
			case 0:
				substdio_put(&ssout, buf, len);
				break;
			case 'D':
				substdio_puts(&ssout, "66:Dsorry, that domain isn't in my list of allowed rcpthosts (#5.7.1),");
				break;
			default:
				substdio_puts(&ssout, "46:Dsorry, I can't handle that recipient (#5.1.3),");
				break;
			}
		}
		/*- ssout will be flushed when we read from the network again */
	}
	/*- Not reached */
	return(0);
}

void
getversion_qmail_qmtpd_c()
{
	static char    *x = "$Id: qmail-qmtpd.c,v 1.18 2023-10-07 01:25:42+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: qmail-qmtpd.c,v $
 * Revision 1.18  2023-10-07 01:25:42+05:30  Cprogrammer
 * use env variable HIDE_HOST to hide IP, host in received headers
 *
 * Revision 1.17  2023-10-04 23:18:55+05:30  Cprogrammer
 * converted to ansic prototypes
 *
 * Revision 1.16  2022-10-22 13:07:46+05:30  Cprogrammer
 * added program identifier to Received header
 *
 * Revision 1.15  2021-09-11 19:01:33+05:30  Cprogrammer
 * pass null remotehost to received when remotehost is unknown
 *
 * Revision 1.14  2021-06-12 18:27:09+05:30  Cprogrammer
 * removed chdir(auto_qmail)
 *
 * Revision 1.13  2013-08-23 15:32:49+05:30  Cprogrammer
 * validity checks to ensure that input actually conforms to the netstring protocol.
 *
 * Revision 1.12  2008-07-15 19:52:51+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.11  2005-08-23 17:35:06+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.10  2005-06-03 09:06:19+05:30  Cprogrammer
 * code beautification
 *
 * Revision 1.9  2004-10-22 20:28:42+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.8  2004-10-22 15:37:37+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.7  2004-07-17 21:21:12+05:30  Cprogrammer
 * added RCS log
 *
 */
