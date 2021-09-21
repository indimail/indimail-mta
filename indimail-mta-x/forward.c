/*
 * $Log: forward.c,v $
 * Revision 1.15  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.14  2021-07-05 21:10:35+05:30  Cprogrammer
 * skip $HOME/.defaultqueue for root
 *
 * Revision 1.13  2021-05-13 14:43:20+05:30  Cprogrammer
 * use set_environment() to set env from ~/.defaultqueue or control/defaultqueue
 *
 * Revision 1.12  2020-11-24 13:45:20+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.11  2020-04-04 11:46:26+05:30  Cprogrammer
 * use auto_sysconfdir instead of auto_qmail
 *
 * Revision 1.10  2020-04-04 11:21:35+05:30  Cprogrammer
 * use environment variables $HOME/.defaultqueue before /etc/indimail/control/defaultqueue
 *
 * Revision 1.9  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.8  2013-08-25 18:38:31+05:30  Cprogrammer
 * added SRS
 *
 * Revision 1.7  2010-06-08 21:59:24+05:30  Cprogrammer
 * use envdir_set() on queuedefault to set default queue parameters
 *
 * Revision 1.6  2008-07-15 19:51:23+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.5  2004-10-22 20:25:27+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-10-22 15:35:14+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.3  2004-07-17 20:53:27+05:30  Cprogrammer
 * added qqeh code
 *
 */
#include <unistd.h>
#include <envdir.h>
#include <pathexec.h>
#include <sig.h>
#include <env.h>
#include <strerr.h>
#include <substdio.h>
#include <fmt.h>
#ifdef HAVESRS
#include <stralloc.h>
#include <srs.h>
#endif
#include <noreturn.h>
#include "qmail.h"
#include "set_environment.h"

#define FATAL "forward: fatal: "
#define WARN  "forward: warn: "

no_return void
die_nomem()
{
	strerr_die2x(111, FATAL, "out of memory");
}

struct qmail    qqt;

ssize_t
mywrite(int fd, char *buf, int len)
{
	qmail_put(&qqt, buf, len);
	return len;
}

char            inbuf[SUBSTDIO_INSIZE];
char            outbuf[1];
substdio        ssin = SUBSTDIO_FDBUF(read, 0, inbuf, sizeof inbuf);
substdio        ssout = SUBSTDIO_FDBUF(mywrite, -1, outbuf, sizeof outbuf);

char            num[FMT_ULONG];

int
main(int argc, char **argv)
{
	char           *sender, *dtline, *qqeh, *qqx;

	sig_pipeignore();
	if (!(sender = env_get("NEWSENDER")))
		strerr_die2x(100, FATAL, "NEWSENDER not set");
	if (!(dtline = env_get("DTLINE")))
		strerr_die2x(100, FATAL, "DTLINE not set");
	set_environment(WARN, FATAL, 0);
#ifdef HAVESRS
	if (*sender) {
		switch(srsforward(sender))
		{
		case -3:
			strerr_die2x(100, FATAL, srs_error.s);
			break;
		case -2:
			strerr_die2x(111, FATAL, "out of memory");
			break;
		case -1:
			strerr_die2x(111, FATAL, "unable to read controls");
			break;
		case 0:
			break; // nothing
		case 1:
			sender = srs_result.s;
			break;
		}
  }
#endif
	if (qmail_open(&qqt) == -1)
		strerr_die2sys(111, FATAL, "unable to fork: ");
	qmail_puts(&qqt, dtline);
	if ((qqeh = env_get("QQEH")))
		qmail_puts(&qqt, qqeh);
	if (substdio_copy(&ssout, &ssin) != 0)
		strerr_die2sys(111, FATAL, "unable to read message: ");
	substdio_flush(&ssout);
	num[fmt_ulong(num, qmail_qp(&qqt))] = 0;
	qmail_from(&qqt, sender);
	while (*++argv)
		qmail_to(&qqt, *argv);
	qqx = qmail_close(&qqt);
	if (*qqx)
		strerr_die2x(*qqx == 'D' ? 100 : 111, FATAL, qqx + 1);
	strerr_die2x(0, "forward: qp ", num);
	/*- Not reached */
	return(0);
}

void
getversion_forward_c()
{
	static char    *x = "$Id: forward.c,v 1.15 2021-08-29 23:27:08+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
