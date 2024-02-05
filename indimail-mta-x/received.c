/*
 * $Id: received.c,v 1.9 2024-02-05 09:32:58+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <fmt.h>
#include <now.h>
#include <datetime.h>
#include <date822fmt.h>
#include "qmail.h"
#include "received.h"

static int
issafe(char ch)
{
	if (ch == '.' || ch == '@' || ch == '%' || ch == '+'
			|| ch == '/' || ch == '=' || ch == ':' || ch == '-'
			|| ch == '_' || ch == '[' || ch == ']' || ch == ' ')
		return 1;
	if ((ch >= 'a') && (ch <= 'z'))
		return 1;
	if ((ch >= 'A') && (ch <= 'Z'))
		return 1;
	if ((ch >= '0') && (ch <= '9'))
		return 1;
	return 0;
}

void
safeput(struct qmail *qqt, char *s)
{
	char            ch;

	while ((ch = *s++)) {
		if (!issafe(ch))
			ch = '?';
		qmail_put(qqt, &ch, 1);
	}
}

/*
 * "Received: from relay1.uu.net (HELO uunet.uu.net) (7@192.48.96.5)\n"
 * "  by silverton.berkeley.edu with SMTP; 26 Sep 1995 04:46:54 -0000\n"
 */

void
received(struct qmail *qqt, char *program, char *protocol, char *local, char *remoteip,
		char *remotehost, char *remoteinfo, char *helo, int hide)
{
	struct datetime dt;
	char            strnum[FMT_ULONG], buf[DATE822FMT];

	qmail_puts(qqt, "Received: indimail-mta ");
	qmail_puts(qqt, program);
	qmail_puts(qqt, " ");
	qmail_put(qqt, strnum, fmt_ulong(strnum, getpid()));
	if (remotehost && !hide) {
		qmail_puts(qqt, " from ");
		safeput(qqt, remotehost);
	}
	if (helo && !hide) {
		qmail_puts(qqt, " (HELO ");
		safeput(qqt, helo);
		qmail_puts(qqt, ")");
	}
	if (remoteinfo && remoteip && !hide) {
		qmail_puts(qqt, " (");
		if (remoteinfo) {
			safeput(qqt, remoteinfo);
			qmail_puts(qqt, "@");
		}
		if (remoteip)
			safeput(qqt, remoteip);
		qmail_puts(qqt, ")\n");
	}
	if (local && !hide) {
		qmail_puts(qqt, "  by ");
		safeput(qqt, local);
	}
	qmail_puts(qqt, " with ");
	qmail_puts(qqt, protocol);
	qmail_puts(qqt, "; ");
	datetime_tai(&dt, now());
	qmail_put(qqt, buf, date822fmt(buf, &dt));
}

void
getversion_received_c()
{
	static char    *x = "$Id: received.c,v 1.9 2024-02-05 09:32:58+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*
 * $Log: received.c,v $
 * Revision 1.9  2024-02-05 09:32:58+05:30  Cprogrammer
 * added parameter hide to hide IP, Host in received headers
 *
 * Revision 1.8  2022-10-22 13:08:35+05:30  Cprogrammer
 * added program identifier to Received header
 *
 * Revision 1.7  2021-09-11 19:02:01+05:30  Cprogrammer
 * skip remotehost in received headers when value is unknown
 *
 * Revision 1.6  2020-07-08 09:15:22+05:30  Cprogrammer
 * added square brackets in the list of safe characters
 *
 * Revision 1.5  2004-10-22 20:29:55+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-09-22 22:26:51+05:30  Cprogrammer
 * added underscore to list of safe characters
 *
 * Revision 1.3  2004-07-17 21:22:34+05:30  Cprogrammer
 * added RCS log
 *
 */
