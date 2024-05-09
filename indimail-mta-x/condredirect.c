/*
 * $Id: condredirect.c,v 1.19 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $
 */
#include <unistd.h>
#include "sig.h"
#include "env.h"
#include "error.h"
#include "wait.h"
#include "seek.h"
#include "qmail.h"
#include "strerr.h"
#include "substdio.h"
#include "fmt.h"
#ifdef HAVESRS
#include "srs.h"
#endif
#include "set_environment.h"

#define FATAL "condredirect: fatal: "
#define WARN  "condredirect: warn: "

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
	char           *sender, *dtline, *qqeh;
	int             pid, wstat, i, reverse = 0;
	const char     *qqx;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (argv[i][1] != 'r')
				strerr_die3x(100, "condredirect: Invalid option ", argv[i], "\nusage: condredirect [ -r ] newaddress program [ arg ... ]");
			if (argv[i][2] == 0)
				reverse = 1;
		}
	}
	if (!argv[1] || !argv[2] || (reverse && (!argv[2] || !argv[3])))
		strerr_die1x(100, "condredirect: usage: condredirect [ -r ] newaddress program [ arg ... ]");
	if ((pid = fork()) == -1)
		strerr_die2sys(111, FATAL, "unable to fork: ");
	if (pid == 0) {
		execvp(reverse ? argv[3] : argv[2], reverse ? argv + 3: argv + 2);
		if (error_temp(errno)) /*- ENOENT is not included in error_temp */
			_exit(111);
		else
		if (errno == error_noent)
			_exit(2);
		_exit(100);
	}
	if (wait_pid(&wstat, pid) == -1)
		strerr_die2x(111, FATAL, "wait failed");
	if (wait_crashed(wstat))
		strerr_die2x(111, FATAL, "child crashed");
	switch ((i = wait_exitcode(wstat)))
	{
	case 0:
		if (reverse)
			_exit(0);
		/*- forward mail to newaddress */
		break;
	case 100:
		strerr_die2x(100,FATAL,"permanent child error");
	case 111:
		strerr_die2x(111, FATAL, "temporary child error");
	default:
		if (!reverse)
			_exit(0);
		/*- forward mail to newaddress */
	}
	if (seek_begin(0) == -1)
		strerr_die2sys(111, FATAL, "unable to rewind: ");
	sig_pipeignore();
	if (!(sender = env_get("SENDER")))
		strerr_die2x(100, FATAL, "SENDER not set");
	if (!(dtline = env_get("DTLINE")))
		strerr_die2x(100, FATAL, "DTLINE not set");
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
	set_environment(WARN, FATAL, 0);
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
	qmail_to(&qqt, reverse ? argv[2] : argv[1]);
	qqx = qmail_close(&qqt);
	if (*qqx)
		strerr_die2x(*qqx == 'D' ? 100 : 111, FATAL, qqx + 1);
	strerr_die2x(99, "condredirect: qp ", num);
	/*- Not reached */
	return(0);
}

void
getversion_condredirect_c()
{
	const char     *x = "$Id: condredirect.c,v 1.19 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}

/*
 * $Log: condredirect.c,v $
 * Revision 1.19  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.18  2023-07-11 11:29:46+05:30  Cprogrammer
 * added -r reverse option to forward when program fails instead of when program succeeds.
 *
 * Revision 1.17  2021-07-05 21:09:19+05:30  Cprogrammer
 * skip $HOME/.defaultqueue for root
 *
 * Revision 1.16  2021-05-13 14:42:23+05:30  Cprogrammer
 * use set_environment() to set env from ~/.defaultqueue or control/defaultqueue
 *
 * Revision 1.15  2020-11-24 13:44:37+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.14  2020-04-04 11:36:55+05:30  Cprogrammer
 * use auto_sysconfdir instead of auto_qmail
 *
 * Revision 1.13  2020-04-04 11:10:14+05:30  Cprogrammer
 * use environment variables $HOME/.defaultqueue before /etc/indimail/control/defaultqueue
 *
 * Revision 1.12  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.11  2013-08-25 18:38:27+05:30  Cprogrammer
 * added SRS
 *
 * Revision 1.10  2010-06-08 21:56:48+05:30  Cprogrammer
 * use envdir_set() on queuedefault to set default queue parameters
 *
 * Revision 1.9  2008-07-15 19:49:51+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.8  2004-10-22 20:24:03+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.7  2004-10-22 15:34:35+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.6  2004-07-17 21:17:44+05:30  Cprogrammer
 * added qqeh code
 *
 */
