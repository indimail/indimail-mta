/*
 * $Log: condredirect.c,v $
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
#include <unistd.h>
#include "sig.h"
#include "envdir.h"
#include "pathexec.h"
#include "auto_qmail.h"
#include "auto_control.h"
#include "exit.h"
#include "env.h"
#include "error.h"
#include "wait.h"
#include "seek.h"
#include "qmail.h"
#include "strerr.h"
#include "substdio.h"
#include "fmt.h"
#ifdef HAVESRS
#include "stralloc.h"
#include "srs.h"
#endif
#include "variables.h"

#define FATAL "condredirect: fatal: "

struct qmail    qqt;

ssize_t
mywrite(fd, buf, len)
	int             fd;
	char           *buf;
	int             len;
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
main(argc, argv)
	int             argc;
	char          **argv;
{
	char           *sender, *dtline, *qqeh, *qbase;
	char          **e;
	int             pid;
	int             wstat;
	char           *qqx;

	if (!argv[1] || !argv[2])
		strerr_die1x(100, "condredirect: usage: condredirect newaddress program [ arg ... ]");
	if ((pid = fork()) == -1)
		strerr_die2sys(111, FATAL, "unable to fork: ");
	if (pid == 0)
	{
		execvp(argv[2], argv + 2);
		if (error_temp(errno))
			_exit(111);
		_exit(100);
	}
	if (wait_pid(&wstat, pid) == -1)
		strerr_die2x(111, FATAL, "wait failed");
	if (wait_crashed(wstat))
		strerr_die2x(111, FATAL, "child crashed");
	switch (wait_exitcode(wstat))
	{
	case 0:
		break;
	case 100:
		strerr_die2x(100,FATAL,"permanent child error");
	case 111:
		strerr_die2x(111, FATAL, "temporary child error");
	default:
		_exit(0);
	}
	if (seek_begin(0) == -1)
		strerr_die2sys(111, FATAL, "unable to rewind: ");
	sig_pipeignore();
	if (!(sender = env_get("SENDER")))
		strerr_die2x(100, FATAL, "SENDER not set");
	if (!(dtline = env_get("DTLINE")))
		strerr_die2x(100, FATAL, "DTLINE not set");
	if (chdir(auto_qmail) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", auto_qmail, ": ");
	if (!(qbase = env_get("QUEUE_BASE")))
	{
		if (!controldir)
		{
			if (!(controldir = env_get("CONTROLDIR")))
				controldir = auto_control;
		}
		if (chdir(controldir) == -1)
			strerr_die4sys(111, FATAL, "unable to switch to ", controldir, ": ");
		if (!access("defaultqueue", X_OK))
		{
			envdir_set("defaultqueue");
			if ((e = pathexec(0)))
				environ = e;
		}
	}
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
	qmail_to(&qqt, argv[1]);
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
	static char    *x = "$Id: condredirect.c,v 1.12 2016-05-17 19:44:58+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
