/*
 * $Id: qmulti.c,v 1.58 2022-03-05 13:07:14+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include "haslibrt.h"
#ifdef HASLIBRT
#include <sys/mman.h>
#include <sys/stat.h>  /* For mode constants */
#include <fcntl.h>     /* For O_* constants */
#endif
#include <env.h>
#include <stralloc.h>
#include <substdio.h>
#include <datetime.h>
#include <now.h>
#include <fmt.h>
#include <scan.h>
#include <error.h>
#include <noreturn.h>
#ifdef sun
#include <sys/types.h>
#include <sys/statvfs.h>
#elif defined(DARWIN) || defined(FREEBSD)
#include <sys/param.h>
#include <sys/mount.h>
#elif defined(linux)
#include <sys/vfs.h>
#endif
#include "auto_qmail.h"
#include "auto_prefix.h"
#include "control.h"
#include "qmulti.h"
#ifdef HASLIBRT
#include "qmail.h"
#include "qscheduler.h"
#endif

#ifdef HASLIBRT
static char     errbuf[256];
static struct substdio sserr;
#endif

int
getfreespace(char *filesystem)
{
	unsigned long   quota_size, u;
	char           *ptr;
#ifdef sun
	struct statvfs  statbuf;
#else
	struct statfs   statbuf;
#endif

	if (!(ptr = env_get("MIN_FREE")))
		return (0);
#ifdef sun
	if (statvfs(filesystem, &statbuf))
#else
	if (statfs(filesystem, &statbuf))
#endif
		return (-1);
	quota_size = ((statbuf.f_bavail / 1024) * (statbuf.f_bsize / 1024));
	scan_ulong(ptr, &u);
	if (quota_size < (u / (1024 * 1024)))
		return (1);
	return (0);
}

#ifdef HASLIBRT
static void
custom_error(char *flag, char *status, char *extra, char *code)
{
	char           *c;

	if (substdio_put(&sserr, flag, 1) == -1)
		_exit(53);
	if (substdio_put(&sserr, "qmail-multi: ", 13) == -1)
		_exit(53);
	if (substdio_puts(&sserr, status) == -1)
		_exit(53);
	if (extra && substdio_puts(&sserr, extra) == -1)
		_exit(53);
	if (code) {
		if (substdio_put(&sserr, " (#", 3) == -1)
			_exit(53);
		c = (*flag == 'Z') ? "4" : "5";
		if (substdio_put(&sserr, c, 1) == -1)
			_exit(53);
		if (substdio_put(&sserr, code + 1, 4) == -1)
			_exit(53);
		if (substdio_put(&sserr, ")", 1) == -1)
			_exit(53);
	}
	if (substdio_flush(&sserr) == -1)
		_exit(53);
	return;
}

int
queueNo_from_shm()
{
	int             shm, errfd, i, j, x, min, n = 1, qcount;
	int             q[4];
	char            shm_name[FMT_ULONG + 6];
	char           *s, *ptr;

	if (!(ptr = env_get("ERROR_FD")))
		errfd = CUSTOM_ERR_FD;
	else
		scan_int(ptr, &errfd);
	substdio_fdbuf(&sserr, write, errfd, errbuf, sizeof(errbuf));
	/*- get queue count */
	if ((shm = shm_open("/qscheduler", O_RDONLY, 0644)) == -1) {
		custom_error("Z", "unable to open POSIX shared memory segment /qscheduler", 0, "X.3.0");
		_exit(88);
	}
	if (read(shm, (char *) &qcount, sizeof(int)) == -1) {
		custom_error("Z", "unable to read POSIX shared memory segment /qscheduler", 0, "X.3.0");
		_exit(88);
	}
	close(shm);
	/*- get queue with lowest concurrency load  */
	for (j = 0; j < qcount; j++) {
		s = shm_name;
		i = fmt_str(s, "/queue");
		s += i;
		i = fmt_int(s, j + 1);
		s += i;
		*s++ = 0;
		i = 0;
		if ((shm = shm_open(shm_name, O_RDONLY, 0600)) == -1) {
			custom_error("Z", "failed to open POSIX shared memory segment ", shm_name, "X.3.0");
			_exit(88);
		}
		if (read(shm, (char *) q, sizeof(int) * 4) == -1) {
			custom_error("Z", "failed to read POSIX shared memory segment ", shm_name, "X.3.0");
			_exit(88);
		}
		close(shm);
		/*-
		 * q[0] - concurrencyusedlocal
		 * q[1] - concurrencyusedremote
		 * q[2] - concurrencylocal
		 * q[3] - concurrencyremote
		 */
		if (!q[2] || !q[3])
			continue;
		x = q[0] * 100 /q[2] > q[1] * 100 /q[3] ? q[0] * 100/q[2] : q[1] * 100/q[3];
		if (!j) {
			min = x;
			n = 1;
			continue;
		}
		if (x < min) {
			min = x;
			n = j + 1;
		}
	}
	return n;
}
#endif

int
queueNo_from_env()
{
	char           *ptr;
	int             qcount, qstart;

	if (!(ptr = env_get("QUEUE_COUNT")))
		qcount = QUEUE_COUNT;
	else
		scan_int(ptr, &qcount);
	if (!(ptr = env_get("QUEUE_START")))
		qstart = 1;
	else
		scan_int(ptr, &qstart);
	return ((now() % qcount) + qstart);
}

no_return int
qmulti(char *queue_env, int argc, char **argv)
{
	char            strnum[FMT_ULONG];
	char           *ptr, *qbase;
	int             queueNo;
	static stralloc Queuedir = { 0 }, QueueBase = { 0 };
	char           *qqargs[3] = { 0, 0, NULL };
	char           *binqqargs[2] = { 0, NULL };
	stralloc        q = {0};

	if (chdir("/") == -1)
		_exit(63);
	if (queue_env && (ptr = env_get(queue_env)) && *ptr) {
		binqqargs[0] = ptr;
		execv(*binqqargs, binqqargs);
		_exit(120);
	}
	if (!(ptr = env_get("QUEUEDIR"))) {
		if (!(qbase = env_get("QUEUE_BASE"))) {
			switch (control_readfile(&QueueBase, "queue_base", 0))
			{
			case -1:
				_exit(55);
				break;
			case 0:
				if (!stralloc_copys(&QueueBase, auto_qmail) ||
						!stralloc_catb(&QueueBase, "/queue", 6) ||
						!stralloc_0(&QueueBase))
					_exit(51);
				qbase = QueueBase.s;
				break;
			case 1:
				qbase = QueueBase.s;
				break;
			}
		}
#ifdef HASLIBRT
		if (!(ptr = env_get("DYNAMIC_QUEUE")))
			queueNo = queueNo_from_env();
		else {
			if ((queueNo = queueNo_from_shm()) == -1)
				queueNo = queueNo_from_env();
		}
#else
		queueNo = queueNo_from_env();
#endif
		if (!stralloc_copys(&Queuedir, "QUEUEDIR=") ||
				!stralloc_cats(&Queuedir, qbase) ||
				!stralloc_cats(&Queuedir, "/queue") ||
				!stralloc_catb(&Queuedir, strnum, fmt_ulong(strnum, (unsigned long) queueNo)) ||
				!stralloc_0(&Queuedir))
			_exit(51);
		env_put(Queuedir.s);
		ptr = Queuedir.s + 9;
	}
	qqargs[1] = ptr;
	switch (getfreespace(ptr))
	{
	case -1:
		_exit(55);
	case 1: /*- Disk full */
		_exit(53);
	}
	if ((ptr = env_get("QUEUEPROG"))) {
		argv[0] = qqargs[0] = ptr;
		execv(*qqargs, argv);
	} else {
		if (!stralloc_copys(&q, auto_prefix) ||
				!stralloc_catb(&q, "/sbin/qmail-queue", 17) ||
				!stralloc_0(&q))
			_exit(51);
		qqargs[0] = q.s;
		execv(*qqargs, qqargs);
	}
	_exit(120);
}

int
discard_envelope()
{
	char            buffer[2048];
	int             n, total = 0;

	/*- discard envelope and empty smtp's pipe */
	for (;;) {
		if (!(n = read(1, buffer, sizeof (buffer))))
			break;
		if (n == -1) {
			if (errno == error_intr)
				continue;
			return (54);
		}
		total += sizeof (buffer);
	}
	if (!total)
		return (54);
	return (0);
}

/*
 * Returns
 * 1 if envelope was rewritten
 * 0 if no rewriting has happened
 * > 1 on error
 */
int
rewrite_envelope(int outfd)
{
	static stralloc notifyaddress = { 0 };
	struct substdio ssout;
	int             n;
	char            buffer[2048];
	char           *ptr;

	if (!(ptr = env_get("SPAMREDIRECT"))) {
		if (control_readline(&notifyaddress, "globalspamredirect") == -1)
			return (55);
	} else
	if (!stralloc_copys(&notifyaddress, ptr))
		_exit(51);
	if (!notifyaddress.len)
		return (0);
	if ((n = discard_envelope()))
		return (n);
	substdio_fdbuf(&ssout, write, outfd, buffer, sizeof (buffer));
	if (substdio_bput(&ssout, "F", 1) == -1 ||
			substdio_bput(&ssout, "\0", 1) == -1 ||
			substdio_bput(&ssout, "T", 1) == -1 ||
			substdio_bput(&ssout, notifyaddress.s, notifyaddress.len) == -1 ||
			substdio_bput(&ssout, "\0", 1) == -1 ||
			substdio_bput(&ssout, "\0", 1) == -1 ||
			substdio_flush(&ssout) == -1)
		return (53);
	return (1);
}

#ifndef	lint
void
getversion_qmulti_c()
{
	static char    *x = "$Id: qmulti.c,v 1.58 2022-03-05 13:07:14+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidqmultih;
	x++;
}
#endif

/*
 * $Log: qmulti.c,v $
 * Revision 1.58  2022-03-05 13:07:14+05:30  Cprogrammer
 * use IPC as another method for queues in addition to lock/trigger
 * added haslibrt.h to configure dynamic queue
 * make queue directory as the second argument to qmail-queue
 * use auto_prefix/sbin for qmail-queue path
 *
 * Revision 1.57  2021-10-21 12:41:36+05:30  Cprogrammer
 * eliminated extra variables
 *
 * Revision 1.56  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.55  2021-06-12 18:52:06+05:30  Cprogrammer
 * added chdir(auto_qmail) for qmail-queue
 *
 * Revision 1.54  2021-06-09 19:33:32+05:30  Cprogrammer
 * moved qmail-multi code to qmulti.c
 *
 * Revision 1.53  2021-05-29 23:49:39+05:30  Cprogrammer
 * fixed qbase path
 *
 * Revision 1.52  2020-09-16 19:04:52+05:30  Cprogrammer
 * FreeBSD fix
 *
 * Revision 1.51  2020-04-01 16:14:39+05:30  Cprogrammer
 * added header for makeargs() function
 *
 * Revision 1.50  2018-06-22 14:29:28+05:30  Cprogrammer
 * code indented
 *
 * Revision 1.49  2018-05-05 18:23:23+05:30  Cprogrammer
 * fixed qscanq path
 *
 * Revision 1.48  2016-06-03 09:58:05+05:30  Cprogrammer
 * moved qmail-queue to sbin
 *
 * Revision 1.47  2012-08-16 11:36:21+05:30  Cprogrammer
 * added case 88 for surblfilter
 *
 * Revision 1.46  2011-06-09 21:27:40+05:30  Cprogrammer
 * blackhole mails if filter program exits 2
 *
 * Revision 1.45  2010-03-26 15:02:37+05:30  Cprogrammer
 * Use QUEUEPROG to call an alternate queue program like qmail-qmqpc
 *
 * Revision 1.44  2009-11-09 20:32:39+05:30  Cprogrammer
 * Use control file queue_base to process multiple indimail queues
 *
 * Revision 1.43  2009-10-31 14:32:58+05:30  Cprogrammer
 * skip spam filtering for authenticated users
 *
 * Revision 1.42  2009-09-08 12:34:07+05:30  Cprogrammer
 * removed dependency of indimail on qmulti
 *
 * Revision 1.41  2008-07-25 16:52:04+05:30  Cprogrammer
 * port for darwin
 *
 * Revision 1.40  2007-12-20 12:48:45+05:30  Cprogrammer
 * combined SPAMFILTER & SPAMFILTERARGS into SPAMFILTER
 * removed compiler warnings
 *
 * Revision 1.39  2006-01-22 11:13:28+05:30  Cprogrammer
 * Fixed bug when spam mails being quarantined also matched expression in bodycheck
 *
 * Revision 1.38  2005-08-23 17:34:55+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.37  2005-02-14 23:05:20+05:30  Cprogrammer
 * moved unset of VIRUSCHECK to qscanq-stdin.c
 *
 * Revision 1.36  2004-10-22 20:28:30+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.35  2004-09-27 15:30:57+05:30  Cprogrammer
 * unset VIRUSCHECK before exec of qscanq to
 * prevent recursive execs
 *
 * Revision 1.34  2004-09-22 23:12:43+05:30  Cprogrammer
 * call external virus scanner
 * replaced atoi with scan_int
 *
 * Revision 1.33  2004-09-03 23:50:03+05:30  Cprogrammer
 * added handling of default signals
 *
 * Revision 1.32  2004-07-15 23:36:44+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.31  2004-07-02 16:14:13+05:30  Cprogrammer
 * use globalspamredirect only if SPAMREDIRECT is not defined
 * Environment variable SPAMREDIRECT overrides control file
 *
 * Revision 1.30  2004-05-03 22:14:11+05:30  Cprogrammer
 * use QUEUE_BASE to avoid using single queue prefix
 *
 * Revision 1.29  2004-02-05 18:48:11+05:30  Cprogrammer
 * changed stralloc variables to static
 *
 * Revision 1.28  2004-01-10 09:43:54+05:30  Cprogrammer
 * changed exit code for filter failures to 76
 *
 * Revision 1.27  2004-01-08 00:31:31+05:30  Cprogrammer
 * use TMPDIR environment variable for temporary directory
 *
 * Revision 1.26  2004-01-05 14:02:42+05:30  Cprogrammer
 * envelope wrongly discarded for REJECTSPAM=0
 *
 * Revision 1.25  2003-12-30 00:44:09+05:30  Cprogrammer
 * set argv[0] from spamfilterprog
 *
 * Revision 1.24  2003-12-22 18:36:01+05:30  Cprogrammer
 * moved mkTempFile() outside #ifdef indimail
 *
 * Revision 1.23  2003-12-20 01:47:34+05:30  Cprogrammer
 * added filter capability
 *
 * Revision 1.22  2003-12-15 13:47:44+05:30  Cprogrammer
 * renamed QMAILFILTER to SPAMFILTER and FILTERARGS to SPAMFILTERARGS
 * to avoid conflicts with generic filters
 *
 * Revision 1.21  2003-12-14 11:35:51+05:30  Cprogrammer
 * added option to blackhole spammers
 *
 * Revision 1.20  2003-12-07 13:04:53+05:30  Cprogrammer
 * used stralloc instead of fixed strings
 * made REJECTSPAM and NOTIFYSPAM independent
 *
 * Revision 1.19  2003-11-22 11:39:21+05:30  Cprogrammer
 * added documentation
 *
 * Revision 1.18  2003-10-28 20:01:16+05:30  Cprogrammer
 * conditional compilation of variables defined for indimail
 *
 * Revision 1.17  2003-10-27 00:51:19+05:30  Cprogrammer
 * NOTIFYSPAM functionality
 *
 * Revision 1.16  2003-10-23 01:31:13+05:30  Cprogrammer
 * code rewritten to handle mail rejection by filter
 *
 * Revision 1.15  2003-10-16 01:20:22+05:30  Cprogrammer
 * added REJECTSPAM and SPAMEXITCODE for dropping mails with spam content
 * exit codes changes
 *
 * Revision 1.14  2003-10-13 18:12:37+05:30  Cprogrammer
 * added seekfd argument to mkTemFile() to makeseekable any file
 *
 * Revision 1.13  2003-10-03 11:59:12+05:30  Cprogrammer
 * bypass multiplexing if QUEUEDIR is set
 *
 * Revision 1.12  2003-09-21 22:48:30+05:30  Cprogrammer
 * added filtering ability
 *
 * Revision 1.11  2003-01-05 23:52:08+05:30  Cprogrammer
 * corrected compilation for non-indimail environment
 *
 * Revision 1.10  2002-12-05 14:12:27+05:30  Cprogrammer
 * renamed queuedir to Queuedir
 *
 * Revision 1.9  2002-12-04 20:47:24+05:30  Cprogrammer
 * convert to megabytes to avoid variable overflow for large filesystems
 *
 * Revision 1.8  2002-09-30 19:49:45+05:30  Cprogrammer
 * corrected typo with statvfs()
 *
 * Revision 1.7  2002-09-14 21:21:35+05:30  Cprogrammer
 * removed string functions
 *
 * Revision 1.6  2002/09/11 21:35:28  Cprogrammer
 * added code to calculate free space on filesystem before accepting mail
 *
 */
