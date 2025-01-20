/*
 * $Log: filterto.c,v $
 * Revision 1.14  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.13  2024-01-23 01:21:12+05:30  Cprogrammer
 * convert mywrite to ansic prototype
 *
 * Revision 1.12  2021-07-05 21:10:21+05:30  Cprogrammer
 * skip $HOME/.defaultqueue for root
 *
 * Revision 1.11  2021-05-13 14:43:13+05:30  Cprogrammer
 * use set_environment() to set env from ~/.defaultqueue or control/defaultqueue
 *
 * Revision 1.10  2020-11-24 13:45:17+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.9  2020-04-04 11:45:05+05:30  Cprogrammer
 * use auto_sysconfdir instead of auto_qmail
 *
 * Revision 1.8  2020-04-04 11:19:55+05:30  Cprogrammer
 * use environment variables $HOME/.defaultqueue before /etc/indimail/control/defaultqueue
 *
 * Revision 1.7  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.6  2010-06-08 21:59:20+05:30  Cprogrammer
 * use envdir_set() on queuedefault to set default queue parameters
 *
 * Revision 1.5  2008-07-15 19:51:11+05:30  Cprogrammer
 * porting for Mac OS X
 *
 * Revision 1.4  2004-10-22 20:25:06+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-10-22 15:35:09+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.2  2004-10-09 19:20:38+05:30  Cprogrammer
 * moved sig_ignore() and sig_uncatch() to sig.h
 *
 * Revision 1.1  2004-07-17 20:52:28+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <signal.h>
#include "envdir.h"
#include "sig.h"
#include "env.h"
#include "error.h"
#include "wait.h"
#include "qmail.h"
#include "strerr.h"
#include "substdio.h"
#include "fmt.h"
#include "getln.h"
#include "mess822.h"
#include "fd.h"
#include "pathexec.h"
#include "set_environment.h"

#define FATAL "filterto: fatal: "
#define WARN  "filterto: warn: "

void        (*sig_defaulthandler)(int) = SIG_DFL;
void        (*sig_ignorehandler)(int) = SIG_IGN;

struct qmail    qqt;

ssize_t
mywrite(int fd, const char *buf, size_t len)
{
	qmail_put(&qqt, buf, len);
	return len;
}

char            inbuf[SUBSTDIO_INSIZE];
char            outbuf[1];
substdio        ssin;
static char     ssoutbuf[512];
static substdio ssout = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) mywrite, -1, ssoutbuf, sizeof ssoutbuf);

char            num[FMT_ULONG];

int
main(int argc, char **argv, char **envp)
{
	char           *sender, *dtline;
	int             pid, wstat;
	const char     *qqx;
	int             pf[2];

	if (!argv[1] || !argv[2])
		strerr_die1x(100, "filterto: usage: filterto newaddress program [ arg ... ]");
	sig_ignore(SIGPIPE);
	if (pipe(pf) == -1)
		strerr_die2sys(111, FATAL, "unable to create pipe: ");
	if ((pid = fork()) == -1)
		strerr_die2sys(111, FATAL, "unable to fork: ");
	if (pid == 0) {
		close(pf[0]);
		if (fd_move(1, pf[1]) == -1)
			_exit(111);
		pathexec_run(argv[2], argv + 2, envp);
		if (error_temp(errno))
			_exit(111);
		_exit(100);
	}
	close(pf[1]);
	if (!(sender = env_get("SENDER")))
		strerr_die2x(100, FATAL, "SENDER not set");
	if (!(dtline = env_get("DTLINE")))
		strerr_die2x(100, FATAL, "DTLINE not set");
	set_environment(WARN, FATAL, 0);
	if (qmail_open(&qqt) == -1)
		strerr_die2sys(111, FATAL, "unable to fork: ");
	qmail_puts(&qqt, dtline);
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, pf[0], inbuf, sizeof inbuf);
	if (substdio_copy(&ssout, &ssin) != 0)
		strerr_die2sys(111, FATAL, "unable to read message: ");
	if (substdio_flush(&ssout) == -1)
		strerr_die2sys(111, FATAL, "unable to write message: ");
	if (wait_pid(&wstat, pid) == -1)
		strerr_die2x(111, FATAL, "wait failed");
	if (wait_crashed(wstat))
		strerr_die2x(111, FATAL, "child crashed");
	switch (wait_exitcode(wstat))
	{
	case 0:
		break;
	case 111:
		strerr_die2x(111, FATAL, "temporary child error");
	default:
		_exit(0);
	}
	close(pf[0]);
	num[fmt_ulong(num, qmail_qp(&qqt))] = 0;
	qmail_from(&qqt, sender);
	qmail_to(&qqt, argv[1]);
	qqx = qmail_close(&qqt);
	if (*qqx)
		strerr_die2x(*qqx == 'D' ? 100 : 111, FATAL, qqx + 1);
	strerr_die2x(99, "filterto: qp ", num);
	/*- Not reached */
	return(0);
}

void
getversion_filterto_c()
{
	const char     *x = "$Id: filterto.c,v 1.14 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
