/*
 * $Log: qmail-daemon.c,v $
 * Revision 1.23  2021-07-19 12:54:19+05:30  Cprogrammer
 * exit 0 if flagexitasap is set
 *
 * Revision 1.22  2021-05-29 23:49:06+05:30  Cprogrammer
 * fixed qbase path
 *
 * Revision 1.21  2021-05-26 10:43:58+05:30  Cprogrammer
 * handle access() error other than ENOENT
 *
 * Revision 1.20  2020-05-11 11:04:08+05:30  Cprogrammer
 * fixed shadowing of global variables by local variables
 *
 * Revision 1.19  2016-06-03 13:34:48+05:30  Cprogrammer
 * moved qmail-start to sbin
 *
 * Revision 1.18  2016-03-31 17:02:13+05:30  Cprogrammer
 * added handler for SIGINT
 *
 * Revision 1.17  2009-11-09 20:33:24+05:30  Cprogrammer
 * Use control file queue_base to process multiple indimail queues
 *
 * Revision 1.16  2009-02-01 00:06:55+05:30  Cprogrammer
 * display system error when unable to switch directory
 *
 * Revision 1.15  2005-07-11 14:33:18+05:30  Cprogrammer
 * qmail-daemon does not start notification queue if queuedir is not in /var/qmail and
 * links are not created in /var/qmail
 *
 * Revision 1.14  2004-10-22 20:28:15+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.13  2004-10-09 00:57:40+05:30  Cprogrammer
 * removed param.h
 * made length of queuedir dynamic
 *
 * Revision 1.12  2004-10-09 00:27:11+05:30  Cprogrammer
 * use stralloc instead of fixed path for queuedir
 *
 * Revision 1.11  2004-09-22 23:12:07+05:30  Cprogrammer
 * replaced atoi with scan_int
 *
 * Revision 1.10  2004-07-15 23:32:06+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.9  2004-05-06 22:28:23+05:30  Cprogrammer
 * do not use nqueue if SPAMFILTER is not set
 *
 * Revision 1.8  2004-05-03 22:11:56+05:30  Cprogrammer
 * replace puts() with my_put()
 * use QUEUE_BASE to avoid using single queue prefix
 *
 * Revision 1.7  2003-12-07 13:03:01+05:30  Cprogrammer
 * puts check for return value of env_put()
 *
 * Revision 1.6  2003-12-05 13:34:29+05:30  Cprogrammer
 * added notification queue
 *
 * Revision 1.5  2003-10-23 01:23:29+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.4  2003-09-16 17:58:41+05:30  Cprogrammer
 * changed sys_errlist to strerror()
 *
 * Revision 1.3  2002-09-14 21:17:36+05:30  Cprogrammer
 * changed execvp to execv
 *
 * Revision 1.2  2002-09-14 20:38:07+05:30  Cprogrammer
 * program to start qmail-send for multiple queues
 *
 * Revision 1.1  2002-09-14 16:29:25+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdlib.h>
#include <string.h>
#include "substdio.h"
#include "error.h"
#include "control.h"
#include "fmt.h"
#include "env.h"
#include "scan.h"
#include "wait.h"
#include "sig.h"
#include "lock.h"
#include "open.h"
#include "stralloc.h"
#include "alloc.h"
#include "str.h"
#include "auto_qmail.h"
#include <signal.h>
#include <unistd.h>

#ifndef QUEUE_COUNT
#define QUEUE_COUNT 10
#endif

char            ssoutbuf[512];
substdio        ssout = SUBSTDIO_FDBUF(write, 1, ssoutbuf, sizeof(ssoutbuf));
char            sserrbuf[512];
substdio        sserr = SUBSTDIO_FDBUF(write, 2, sserrbuf, sizeof(sserrbuf));
int             flagexitasap = 0;
char            strnum[FMT_ULONG];
char           *(qlargs[]) = { "sbin/qmail-start", "./Mailbox", 0};
struct pidtab
{
	int pid;
	char *queuedir;
};
struct pidtab  *pid_table;

void
sigterm()
{
	int             i, qcount;
	char           *queue_count_ptr;

	sig_block(SIGTERM);
	if (!(queue_count_ptr = env_get("QUEUE_COUNT")))
		qcount = QUEUE_COUNT;
	else
		scan_int(queue_count_ptr, &qcount);
	for (i = 0;i <= qcount;i++) {
		if (pid_table[i].pid == -1)
			continue;
		kill(pid_table[i].pid, SIGTERM);
	}
	flagexitasap = 1;
}

void
sigalrm()
{
	int             i, qcount;
	char           *queue_count_ptr;

	if (!(queue_count_ptr = env_get("QUEUE_COUNT")))
		qcount = QUEUE_COUNT;
	else
		scan_int(queue_count_ptr, &qcount);
	for (i = 0;i <= qcount;i++) {
		if (pid_table[i].pid == -1)
			continue;
		kill(pid_table[i].pid, SIGALRM);
	}
}

void
sighup()
{
	int             i, qcount;
	char           *queue_count_ptr;

	if (!(queue_count_ptr = env_get("QUEUE_COUNT")))
		qcount = QUEUE_COUNT;
	else
		scan_int(queue_count_ptr, &qcount);
	for (i = 0;i <= qcount;i++) {
		if (pid_table[i].pid == -1)
			continue;
		kill(pid_table[i].pid, SIGHUP);
	}
}

void
sigint()
{
	int             i, qcount;
	char           *queue_count_ptr;

	if (!(queue_count_ptr = env_get("QUEUE_COUNT")))
		qcount = QUEUE_COUNT;
	else
		scan_int(queue_count_ptr, &qcount);
	for (i = 0;i <= qcount;i++) {
		if (pid_table[i].pid == -1)
			continue;
		kill(pid_table[i].pid, SIGINT);
	}
}

void
die()
{
	sleep(5);
	sigterm();
	_exit(111);
}

void
my_log(s)
	char           *s;
{
	if (substdio_puts(&ssout, s) == -1)
		_exit(1);
}

void
my_logf(s)
	char           *s;
{
	if (substdio_puts(&ssout, s) == -1)
		_exit(1);
	if (substdio_flush(&ssout) == -1)
		_exit(1);
}

void
logerr(s)
	char           *s;
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
}

void
logerrf(s)
	char           *s;
{
	if (substdio_puts(&sserr, s) == -1)
		_exit(1);
	if (substdio_flush(&sserr) == -1)
		_exit(1);
}

void
log_announce(int pid, char *queuedir)
{
	my_log("qmail-daemon: pid ");
	strnum[fmt_ulong(strnum, pid)] = 0;
	my_log(strnum);
	my_log(", queue ");
	my_log(queuedir);
	my_logf(" started\n");

}

int
check_send(char *queuedir)
{
	static stralloc lockfile = { 0 };
	int             fd;

	if (!stralloc_copys(&lockfile, queuedir)) {
		logerrf("alert: out of memory\n");
		return(1);
	}
	if (!stralloc_cats(&lockfile, "/lock/sendmutex")) {
		logerrf("alert: out of memory\n");
		return(1);
	}
	if (!stralloc_0(&lockfile)) {
		logerrf("alert: out of memory\n");
		return(1);
	}
	if ((fd = open_write(lockfile.s)) == -1) {
		logerr("alert: cannot start: unable to open ");
		logerr(lockfile.s);
		logerr(": ");
		logerr(error_str(errno));
		logerrf("\n");
		return(1);
	} else
	if (lock_exnb(fd) == -1) {
		close(fd); /*- send already running */
		logerr("alert: cannot start: qmail-send with queue ");
		logerr(queuedir);
		logerrf(" is already running\n");
		return(1);
	}
	close(fd);
	return(0);
}

void
start_send(char **argv, int qstart, int qcount)
{
	char           *qbase;
	int             i, j, child, nqueue;
	static stralloc queuedir = {0}, QueueBase = {0};

	if (argv[1])
		qlargs[1] = argv[1];
	if (!pid_table) {
		if (!(pid_table = (struct pidtab *) alloc(sizeof(struct pidtab) * (qcount + 1)))) {
			logerrf("alert: out of memory\n");
			_exit(111);
		}
		for (i = 0;i <= qcount;i++)
			pid_table[i].pid = -1;
	}
	if (!(qbase = env_get("QUEUE_BASE"))) {
		switch (control_readfile(&QueueBase, "queue_base", 0))
		{
		case -1:
			logerrf("alert: unable to read control file qbase\n");
			_exit(111);
			break;
		case 0:
			if (!stralloc_copys(&QueueBase, auto_qmail) ||
					!stralloc_catb(&QueueBase, "/queue", 6) ||
					!stralloc_0(&QueueBase)) {
				logerrf("alert: out of memory\n");
				die();
			}
			qbase = QueueBase.s;
			break;
		case 1:
			qbase = QueueBase.s;
			break;
		}
	}
	my_log("qmail-daemon: qStart/qCount ");
	strnum[fmt_ulong(strnum, qstart)] = 0;
	my_log(strnum);
	my_log("/");
	strnum[fmt_ulong(strnum, qcount)] = 0;
	my_log(strnum);
	if (env_get("SPAMFILTER")) {
		if (!stralloc_copys(&queuedir, qbase)) {
			logerrf("alert: out of memory\n");
			die();
		}
		if (!stralloc_cats(&queuedir, "/nqueue")) {
			logerrf("alert: out of memory\n");
			die();
		}
		if (!stralloc_0(&queuedir)) {
			logerrf("alert: out of memory\n");
			die();
		}
		nqueue = !access(queuedir.s, F_OK);
		if (!nqueue && errno != error_noent) {
			logerr("alert: ");
			logerr(queuedir.s);
			logerr(": ");
			logerr(error_str(errno));
			logerrf("\n");
			die();
		}
		if (nqueue)
			my_log(" nqueue");
	} else
		nqueue = 0;
	my_logf("\n");
	for (i = qstart;i <= qstart + qcount;i++) {
		if (!stralloc_copys(&queuedir, "QUEUEDIR=")) {
			logerrf("alert: out of memory\n");
			die();
		}
		if (!stralloc_cats(&queuedir, qbase)) {
			logerrf("alert: out of memory\n");
			die();
		}
		if (i == qstart + qcount) {
			if (!nqueue)
				break;
			if (!stralloc_cats(&queuedir, "/nqueue")) {
				logerrf("alert: out of memory\n");
				die();
			}
		} else {
			if (!stralloc_cats(&queuedir, "/queue")) {
				logerrf("alert: out of memory\n");
				die();
			}
			if (!stralloc_catb(&queuedir, strnum, fmt_ulong(strnum, (unsigned long) i))) {
				logerrf("alert: out of memory\n");
				die();
			}
		}
		if (!stralloc_0(&queuedir)) {
			logerrf("alert: out of memory\n");
			die();
		}
		if (check_send(queuedir.s + 9))
			die();
		switch((child = fork()))
		{
			case -1:
				logerrf("alert: fork failed\n");
				die();
			case 0:
				sig_catch(SIGTERM, SIG_DFL);
				sig_catch(SIGALRM, SIG_DFL);
				sig_catch(SIGHUP, SIG_DFL);
				if (i == qstart + qcount) /*- don't set this for nqueue */ {
					if (!env_unset("QMAILLOCAL") || !env_unset("QMAILREMOTE")) {
						logerrf("alert: out of memory\n");
						die();
					}
				}
				if (!env_put(queuedir.s)) {
					logerrf("alert: out of memory\n");
					die();
				}
				if (argv[1])
					execv(*qlargs, argv);
				else
					execv(*qlargs, qlargs);
				logerrf("alert: execv failed\n");
				_exit(111);
			default:
				pid_table[i - qstart].pid = child;
				if (!(pid_table[i - qstart].queuedir = (char *) alloc(str_len(queuedir.s) + 1))) {
					logerrf("alert: out of memory\n");
					die();
				}
				j = fmt_str(pid_table[i - qstart].queuedir, queuedir.s);
				pid_table[i - qstart].queuedir[j] = 0;
				log_announce(child, queuedir.s + 9);
				break;
		}
	} /*- for (i = qstart;i < qstart + qcount;i++) */
}

void
restart(char **argv, int pid, int qcount)
{
	int             i, child;

	for (i = 0;i <= qcount;i++) {
		if (pid_table[i].pid == pid)
			break;
	}
	if (pid_table[i].pid != pid) {
		logerr("qmail-daemon: could not locate pid "); 
		strnum[fmt_ulong(strnum, pid)] = 0;
		logerr(strnum);
		logerrf("\n");
		die();
	}
	logerr("alert: qmail-daemon: pid ");
	strnum[fmt_ulong(strnum, pid)] = 0;
	logerr(strnum);
	logerr(", queue ");
	logerr(pid_table[i].queuedir + 9);
	logerrf(" died\n");
	if (check_send(pid_table[i].queuedir + 9))
		die();
	switch((child = fork()))
	{
		case -1:
			logerrf("alert: fork failed\n");
			die();
		case 0:
			sig_catch(SIGTERM, SIG_DFL);
			sig_catch(SIGALRM, SIG_DFL);
			sig_catch(SIGHUP, SIG_DFL);
			if (!env_put(pid_table[i].queuedir)) {
				logerrf("alert: out of memory\n");
				die();
			}
			if (argv[1])
				execv(*qlargs, argv);
			else
				execv(*qlargs, qlargs);
			logerrf("alert: execv failed\n");
			_exit(111);
		default:
			pid_table[i].pid = child;
			log_announce(child, pid_table[i].queuedir + 9);
			break;
	}
	return;
}

int
main(int argc, char **argv)
{
	char           *queue_count_ptr, *queue_start_ptr;
	int             qcount, qstart, wstat, child;

	if (!(queue_count_ptr = env_get("QUEUE_COUNT")))
		qcount = QUEUE_COUNT;
	else
		scan_int(queue_count_ptr, &qcount);
	if (!(queue_start_ptr = env_get("QUEUE_START")))
		qstart = 1;
	else
		scan_int(queue_start_ptr, &qstart);
	sig_catch(SIGTERM, sigterm);
	sig_catch(SIGALRM, sigalrm);
	sig_catch(SIGHUP, sighup);
	sig_catch(SIGINT, sigint);
	sig_catch(SIGCHLD, SIG_DFL);

	if (chdir(auto_qmail) == -1) {
		logerr("alert: qmail-daemon: unable to switch to qmail directory: ");
		logerr(error_str(errno));
		logerrf("\n");
		die();
	}
	if (flagexitasap) {
		logerrf("qmail-daemon: exiting\n");
		return(0);
	}
	start_send(argv, qstart, qcount);
	for (;!flagexitasap;) {
		if ((child = wait_pid(&wstat, -1)) == -1)
			break;
		if (flagexitasap)
			break;
		restart(argv, child, qcount);
		sleep(1);
	} /*- for (child = 0;;) */
	for (;flagexitasap;) {
		if ((child = wait_pid(&wstat, -1)) == -1)
			break;
	}
	logerrf("qmail-daemon: exiting\n");
	_exit(flagexitasap ? 0 : 111);
	/*- Not reached */
	return (0);
}

void
getversion_qmail_daemon_c()
{
	static char    *x = "$Id: qmail-daemon.c,v 1.23 2021-07-19 12:54:19+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
