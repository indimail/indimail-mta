/*
 * $Log: qmail-multi.c,v $
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
 * removed dependency of indimail on qmail-multi
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
#include "fmt.h"
#include "now.h"
#include "scan.h"
#include "datetime.h"
#include "env.h"
#include "substdio.h"
#include "stralloc.h"
#include "error.h"
#include "control.h"
#include "wait.h"
#include "sig.h"
#include "auto_qmail.h"
#include "variables.h"
#ifdef sun
#include <sys/types.h>
#include <sys/statvfs.h>
#elif defined(DARWIN)
#include <sys/param.h>
#include <sys/mount.h>
#elif defined(linux)
#include <sys/vfs.h>
#endif
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

static int      getfreespace(char *);
static int      qmail_multi(int, char **);
static int      run_mailfilter(int, char **);
int             mkTempFile(int);
int             rewrite_envelope(int);
int             discard_envelope();

#if !defined(QUEUE_COUNT)
#define QUEUE_COUNT 10
#endif
#define DEATH 86400	/*- 24 hours; _must_ be below q-s's OSSIFIED (36 hours) */

extern char   **MakeArgs(char *);

void
sigalrm()
{
	/*- thou shalt not clean up here */
	_exit(52);
}

void
sigbug()
{
	_exit(81);
}

int
main(int argc, char **argv)
{
	int             wstat, filt_exitcode, queueexitcode, n;
	int             pipefd[2], recpfd[2];
	pid_t           filt_pid, queuepid;
	struct substdio ssin, ssout;
	char            inbuf[2048], outbuf[2048];
	char           *ptr, *makeseekable;
	stralloc        spamfilterargs = { 0 };
	char           *spamf;
	char          **Argv;

	if (chdir(auto_qmail) == -1)
		_exit(61);
	sig_pipeignore();
	sig_miscignore();
	sig_alarmcatch(sigalrm);
	sig_bugcatch(sigbug);
	alarm(DEATH);
	if ((ptr = env_get("VIRUSCHECK")) && *ptr) {
		scan_int(ptr, &n);
		if (1 < n && 8 > n) {
			execv("sbin/qscanq", argv);
			_exit(75);
		}
	}
	if (!(spamf = env_get("SPAMFILTER")) || env_get("RELAYCLIENT"))
		return (run_mailfilter(argc, argv)); /*- Does not return */
	if (pipe(pipefd) == -1)
		_exit(60);
	switch ((filt_pid = fork())) /*- spam filter */
	{
	case -1:
		close(0);
		close(1);
		close(pipefd[0]);
		close(pipefd[1]);
		_exit(121);
	case 0:
		if (!stralloc_copys(&spamfilterargs, spamf))
			_exit(51);
		if (!stralloc_0(&spamfilterargs))
			_exit(51);
		if (!(Argv = MakeArgs(spamfilterargs.s)))
			_exit(51);
		/*- Mail content read from fd 0 */
		makeseekable = env_get("MAKE_SEEKABLE");
		if (makeseekable && mkTempFile(0))
			_exit(68);
		if (dup2(pipefd[1], 1) == -1 || close(pipefd[0]) == -1)
			_exit(60);
		if (pipefd[1] != 1)
			close(pipefd[1]);
		execv(*Argv, Argv);
		_exit(75);
	default:
		break;
	}
	if (pipe(recpfd) == -1) {
		close(0);
		close(1);
		close(pipefd[0]);
		close(pipefd[1]);
		close(recpfd[0]);
		close(recpfd[1]);
		_exit(60);
	}
	close(0);
	switch ((queuepid = fork()))
	{
	case -1:
		close(1);
		close(pipefd[0]);
		close(pipefd[1]);
		close(recpfd[0]);
		close(recpfd[1]);
		wait_pid(&wstat, filt_pid);
		_exit(120);
	case 0:
		/*- 
		 * Mail content read from pipfd[0]
		 * which has been filtered through SPAMFILTER
		 * Envelope information can be read through recpfd[0]
		 */
		if (dup2(pipefd[0], 0) == -1 || close(pipefd[1]) == -1)
			_exit(60);
		if (dup2(recpfd[0], 1) == -1 || close(recpfd[1]) == -1)
			_exit(60);
		if (pipefd[0] != 0)
			close(pipefd[0]);
		if (recpfd[0] != 1)
			close(recpfd[0]);
		return (run_mailfilter(argc, argv));
	default:
		close(pipefd[0]);
		close(pipefd[1]);
		close(recpfd[0]);
		break;
	}
	if (wait_pid(&wstat, filt_pid) != filt_pid) {
		close(1);
		close(recpfd[1]);
		wait_pid(&wstat, queuepid);
		_exit(122);
	}
	if (wait_crashed(wstat)) {
		close(1);
		close(recpfd[1]);
		wait_pid(&wstat, queuepid);
		_exit(123);
	}
	/*
	 * Drop message if filter returns 99
	 * Process message if exit code is 0, 1, 2
	 */
	switch (filt_exitcode = wait_exitcode(wstat))
	{
	case 0: /*- SPAM */
	case 1: /*- HAM */
	case 2: /*- Unsure */
		if ((ptr = env_get("SPAMEXITCODE"))) {
			scan_int(ptr, &n);
			if (n == filt_exitcode) { /*- Message is SPAM */
				if ((n = rewrite_envelope(recpfd[1])) > 1) { /*- Some error */
					close(1);
					close(recpfd[1]);
					wait_pid(&wstat, queuepid);
					_exit(n);
				}
				if (n == 1 || (ptr = env_get("REJECTSPAM"))) {
					/*- REJECTSPAM takes precedence over spam notifications */
					if (ptr && *ptr > '0') {
						(void) discard_envelope();
						close(1);
						close(recpfd[1]);
						wait_pid(&wstat, queuepid);
						if (*ptr == '1')
							_exit(32); /*- bounce */
						else
							_exit(0); /*- blackhole */
					} else /*- spam notification - envelope has been rewritten */ if (n == 1) {
						(void) discard_envelope();
						goto finish;
					}
				}
			}
		}
		break;
	default: /*- should not happen normally */
		close(1);
		close(recpfd[1]);
		wait_pid(&wstat, queuepid);
		_exit(76); /*- treat this as temp problem with spam filter */
	}
	/*- Write envelope to qmail-queue */
	substdio_fdbuf(&ssout, write, recpfd[1], outbuf, sizeof (outbuf));
	/*- Read envelope from qmail-smtpd */
	substdio_fdbuf(&ssin, read, 1, inbuf, sizeof (inbuf));
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2: /*- read error */
		close(1);
		close(recpfd[1]);
		_exit(54);
	case -3: /*- write error */
		close(1);
		close(recpfd[1]);
		_exit(53);
	}
	if (substdio_flush(&ssout) == -1)
		_exit(53);
  finish:
	close(1);
	close(recpfd[1]);
	if (wait_pid(&wstat, queuepid) != queuepid)
		_exit(122);
	if (wait_crashed(wstat))
		_exit(123);
	_exit(queueexitcode = wait_exitcode(wstat));
	/*- Not reached */
	return (0);
}

static int
run_mailfilter(int argc, char **argv)
{
	char            strnum[FMT_ULONG];
	pid_t           filt_pid;
	int             pipefd[2];
	int             wstat, filt_exitcode;
	char           *filterargs;

	if (!(filterargs = env_get("FILTERARGS")))
		return (qmail_multi(argc, argv));
	if (pipe(pipefd) == -1)
		_exit(60);
	switch ((filt_pid = fork()))
	{
	case -1:
		_exit(121);
	case 0: /*- Filter Program */
		/*- Mail content read from fd 0 */
		if (mkTempFile(0))
			_exit(68);
		if (dup2(pipefd[1], 1) == -1 || close(pipefd[0]) == -1)
			_exit(60);
		if (pipefd[1] != 1)
			close(pipefd[1]);
		/*- Avoid loop if program(s) defined by FILTERARGS call qmail-inject, etc */
		if (!env_unset("FILTERARGS") || !env_unset("SPAMFILTER"))
			_exit(51);
		execl("/bin/sh", "qmailfilter", "-c", filterargs, (char *) 0);
		_exit(75);
	default:
		close(pipefd[1]);
		if (dup2(pipefd[0], 0)) {
			close(pipefd[0]);
			wait_pid(&wstat, filt_pid);
			_exit(60);
		}
		if (pipefd[0] != 0)
			close(pipefd[0]);
		if (mkTempFile(0)) {
			close(0);
			wait_pid(&wstat, filt_pid);
			_exit(68);
		}
		break;
	}
	/*- Process message if exit code is 0, bounce if 100, else issue temp error */
	if (wait_pid(&wstat, filt_pid) != filt_pid)
		_exit(122);
	if (wait_crashed(wstat))
		_exit(123);
	switch (filt_exitcode = wait_exitcode(wstat))
	{
	case 0:
		return (qmail_multi(argc, argv));
	case 2:
		return (0); /*- Blackhole */
	case 88: /*- exit with custom error code with error code string from stderr */
		_exit(88);
	case 100:
		_exit(31);
	default:
		strnum[fmt_ulong(strnum, filt_exitcode)] = 0;
		_exit(71);
	}
	/*- Not reached */
	return (0);
}

static int
qmail_multi(int argc, char **argv)
{
	datetime_sec    queueNo;
	char            strnum[FMT_ULONG];
	char           *qqargs[2] = { 0, 0 };
	char           *ptr, *queue_count_ptr, *queue_start_ptr, *qbase;
	int             qcount, qstart;
	static stralloc Queuedir = { 0 }, QueueBase = { 0 };

	if (!(ptr = env_get("QUEUEDIR"))) {
		if (!(queue_count_ptr = env_get("QUEUE_COUNT")))
			qcount = QUEUE_COUNT;
		else
			scan_int(queue_count_ptr, &qcount);
		if (!(queue_start_ptr = env_get("QUEUE_START")))
			qstart = 1;
		else
			scan_int(queue_start_ptr, &qstart);
		if (!(qbase = env_get("QUEUE_BASE"))) {
			switch (control_readfile(&QueueBase, "queue_base", 0))
			{
			case -1:
				_exit(55);
				break;
			case 0:
				qbase = auto_qmail;
				break;
			case 1:
				qbase = QueueBase.s;
				break;
			}
		}
		queueNo = (now() % qcount) + qstart;
		if (!stralloc_copys(&Queuedir, "QUEUEDIR="))
			_exit(51);
		if (!stralloc_cats(&Queuedir, qbase))
			_exit(51);
		if (!stralloc_cats(&Queuedir, "/queue"))
			_exit(51);
		if (!stralloc_catb(&Queuedir, strnum, fmt_ulong(strnum, (unsigned long) queueNo)))
			_exit(51);
		if (!stralloc_0(&Queuedir))
			_exit(51);
		env_put(Queuedir.s);
		ptr = Queuedir.s + 9;
	}
	switch (getfreespace(ptr))
	{
	case -1:
		_exit(55);
	case 1: /*- Disk full */
		_exit(53);
	}
	if (!(qqargs[0] = env_get("QUEUEPROG")))
		qqargs[0] = "sbin/qmail-queue";
	execv(*qqargs, argv);
	_exit(120);
	/*- Not reached */
	return (0);
}

static int
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

int
mkTempFile(int seekfd)
{
	char            inbuf[2048], outbuf[2048], strnum[FMT_ULONG];
	char           *tmpdir;
	static stralloc tmpFile = { 0 };
	struct substdio ssin;
	struct substdio ssout;
	int             fd;

	if (lseek(seekfd, 0, SEEK_SET) == 0)
		return (0);
	if (errno == EBADF)
		_exit(54);
	if (!(tmpdir = env_get("TMPDIR")))
		tmpdir = "/tmp";
	if (!stralloc_copys(&tmpFile, tmpdir))
		_exit(51);
	if (!stralloc_cats(&tmpFile, "/qmailFilterXXX"))
		_exit(51);
	if (!stralloc_catb(&tmpFile, strnum, fmt_ulong(strnum, (unsigned long) getpid())))
		_exit(51);
	if (!stralloc_0(&tmpFile))
		_exit(51);
	if ((fd = open(tmpFile.s, O_RDWR | O_EXCL | O_CREAT, 0600)) == -1)
		return (-1);
	unlink(tmpFile.s);
	substdio_fdbuf(&ssout, write, fd, outbuf, sizeof (outbuf));
	substdio_fdbuf(&ssin, read, seekfd, inbuf, sizeof (inbuf));
	switch (substdio_copy(&ssout, &ssin))
	{
	case -2: /*- read error */
		close(fd);
		_exit(54);
	case -3: /*- write error */
		close(fd);
		_exit(53);
	}
	if (substdio_flush(&ssout) == -1) {
		close(fd);
		_exit(68);
	}
	if (fd != seekfd) {
		if (dup2(fd, seekfd) == -1) {
			close(fd);
			_exit(68);
		}
		close(fd);
	}
	if (lseek(seekfd, 0, SEEK_SET) != 0) {
		close(seekfd);
		_exit(54);
	}
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
	} else if (!stralloc_copys(&notifyaddress, ptr))
		_exit(51);
	if (!notifyaddress.len)
		return (0);
	if ((n = discard_envelope()))
		return (n);
	substdio_fdbuf(&ssout, write, outfd, buffer, sizeof (buffer));
	if (substdio_bput(&ssout, "F", 1) == -1)
		return (53);
	if (substdio_bput(&ssout, "\0", 1) == -1)
		return (53);
	if (substdio_bput(&ssout, "T", 1) == -1)
		return (53);
	if (substdio_bput(&ssout, notifyaddress.s, notifyaddress.len) == -1)
		return (53);
	if (substdio_bput(&ssout, "\0", 1) == -1)
		return (53);
	if (substdio_bput(&ssout, "\0", 1) == -1)
		return (53);
	if (substdio_flush(&ssout) == -1)
		return (53);
	return (1);
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

void
getversion_qmail_multi_c()
{
	static char    *x = "$Id: qmail-multi.c,v 1.50 2018-06-22 14:29:28+05:30 Cprogrammer Exp mbhangui $";
	x++;
}
