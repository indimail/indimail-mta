/*
 * $Log: received.c,v $
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
#include "fmt.h"
#include "qmail.h"
#include "now.h"
#include "datetime.h"
#include "date822fmt.h"
#include "received.h"

static int
issafe(ch)
	char            ch;
{
	if (ch == '.')
		return 1;
	if (ch == '@')
		return 1;
	if (ch == '%')
		return 1;
	if (ch == '+')
		return 1;
	if (ch == '/')
		return 1;
	if (ch == '=')
		return 1;
	if (ch == ':')
		return 1;
	if (ch == '-')
		return 1;
	if (ch == '_')
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
safeput(qqt, s)
	struct qmail   *qqt;
	char           *s;
{
	char            ch;
	while ((ch = *s++))
	{
		if (!issafe(ch))
			ch = '?';
		qmail_put(qqt, &ch, 1);
	}
}

static char     buf[DATE822FMT];

/*
 * "Received: from relay1.uu.net (HELO uunet.uu.net) (7@192.48.96.5)\n" 
 * "  by silverton.berkeley.edu with SMTP; 26 Sep 1995 04:46:54 -0000\n" 
 */

void
received(qqt, protocol, local, remoteip, remotehost, remoteinfo, helo)
	struct qmail   *qqt;
	char           *protocol;
	char           *local;
	char           *remoteip;
	char           *remotehost;
	char           *remoteinfo;
	char           *helo;
{
	struct datetime dt;

	qmail_puts(qqt, "Received: from ");
	safeput(qqt, remotehost);
	if (helo)
	{
		qmail_puts(qqt, " (HELO ");
		safeput(qqt, helo);
		qmail_puts(qqt, ")");
	}
	qmail_puts(qqt, " (");
	if (remoteinfo)
	{
		safeput(qqt, remoteinfo);
		qmail_puts(qqt, "@");
	}
	safeput(qqt, remoteip);
	qmail_puts(qqt, ")\n  by ");
	safeput(qqt, local);
	qmail_puts(qqt, " with ");
	qmail_puts(qqt, protocol);
	qmail_puts(qqt, "; ");
	datetime_tai(&dt, now());
	qmail_put(qqt, buf, date822fmt(buf, &dt));
}

void
getversion_received_c()
{
	static char    *x = "$Id: received.c,v 1.5 2004-10-22 20:29:55+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
