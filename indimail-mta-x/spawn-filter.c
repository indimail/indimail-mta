/*
 * $Log: spawn-filter.c,v $
 * Revision 1.68  2019-09-30 22:59:06+05:30  Cprogrammer
 * use sh as argv0 instead of IndiMailfilter
 *
 * Revision 1.67  2019-07-18 10:48:31+05:30  Cprogrammer
 * use strerr_die?x macro instead of strerr_die() function
 *
 * Revision 1.66  2018-01-31 12:08:27+05:30  Cprogrammer
 * moved qmail-local, qmail-remote to sbin
 *
 * Revision 1.65  2016-06-05 13:22:05+05:30  Cprogrammer
 * fixed stupid error message
 *
 * Revision 1.64  2014-03-26 15:32:26+05:30  Cprogrammer
 * report deliveries blackholed by filters in delivery log
 *
 * Revision 1.63  2014-03-12 15:36:37+05:30  Cprogrammer
 * define REG_NOERROR for OSX / Systems with REG_NOERROR undefined
 *
 * Revision 1.62  2014-03-07 02:07:42+05:30  Cprogrammer
 * do not abort if regcomp() fails
 *
 * Revision 1.61  2014-03-04 02:41:38+05:30  Cprogrammer
 * fix BUG by doing chdir back to auto_qmail
 * ability to have regular expressions on rate control
 * ability to have global definition for rate control
 *
 * Revision 1.60  2014-01-22 15:43:29+05:30  Cprogrammer
 * apply envrules for RATELIMIT_DIR
 *
 * Revision 1.59  2013-09-06 13:58:23+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.58  2013-09-05 09:20:05+05:30  Cprogrammer
 * changed variables to double
 *
 * Revision 1.57  2013-08-29 18:27:15+05:30  Cprogrammer
 * switched to switch statement
 *
 * Revision 1.56  2013-08-27 09:42:18+05:30  Cprogrammer
 * added rate limiting by domain
 *
 * Revision 1.55  2011-06-09 21:28:11+05:30  Cprogrammer
 * blackhole mails if filter program exits 2
 *
 * Revision 1.54  2011-02-08 22:17:37+05:30  Cprogrammer
 * added missing unset of QMAILLOCAL when executing qmail-remote
 *
 * Revision 1.53  2011-01-08 16:41:27+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.52  2010-07-10 10:43:09+05:30  Cprogrammer
 * fixed matching of local/remote directives in filterargs control file
 *
 * Revision 1.51  2010-07-10 09:36:11+05:30  Cprogrammer
 * standardized environment variables set for filters
 *
 * Revision 1.50  2010-07-09 08:22:42+05:30  Cprogrammer
 * implemented sender based envrules using control file fromd.envrules
 *
 * Revision 1.49  2009-11-09 20:32:52+05:30  Cprogrammer
 * Use control file queue_base to process multiple indimail queues
 *
 * Revision 1.48  2009-09-08 13:22:28+05:30  Cprogrammer
 * removed dependency of indimail on spam filtering
 *
 * Revision 1.47  2009-05-01 10:43:40+05:30  Cprogrammer
 * added errstr argument to envrules(), address_match()
 *
 * Revision 1.46  2009-04-29 21:03:40+05:30  Cprogrammer
 * check address_match() for failure
 *
 * Revision 1.45  2009-04-29 14:18:50+05:30  Cprogrammer
 * conditional declaration of spf_fn
 *
 * Revision 1.44  2009-04-29 09:01:03+05:30  Cprogrammer
 * spamignore can be a cdb file
 *
 * Revision 1.43  2009-04-29 08:24:37+05:30  Cprogrammer
 * change for address_match() function
 *
 * Revision 1.42  2009-04-19 13:40:07+05:30  Cprogrammer
 * set environment variable DOMAIN for use in programs called as FILTERS
 *
 * Revision 1.41  2009-04-03 11:42:48+05:30  Cprogrammer
 * create pipe for error messages
 *
 * Revision 1.40  2009-04-02 15:17:54+05:30  Cprogrammer
 * unset QMAILLOCAL in qmail-remote and unset QMAILREMOTE in qmail-local
 *
 * Revision 1.39  2008-06-12 08:40:55+05:30  Cprogrammer
 * added rulesfile argument
 *
 * Revision 1.38  2008-05-25 17:16:43+05:30  Cprogrammer
 * made message more readable by adding a blank space
 *
 * Revision 1.37  2007-12-20 13:51:54+05:30  Cprogrammer
 * avoid loops with FILTERARGS, SPAMFILTERARGS
 * removed compiler warning
 *
 * Revision 1.36  2006-06-07 14:11:28+05:30  Cprogrammer
 * added SPAMEXT, SPAMHOST, SPAMSENDER, QQEH environment variable
 * unset FILTERARGS before calling filters
 *
 * Revision 1.35  2006-01-22 10:14:45+05:30  Cprogrammer
 * BUG fix for spam mails wrongly getting blackholed
 *
 * Revision 1.34  2005-08-23 17:36:48+05:30  Cprogrammer
 * gcc 4 compliance
 * delete sender in spam notification
 *
 * Revision 1.33  2005-04-02 19:07:47+05:30  Cprogrammer
 * use internal wildmat version
 *
 * Revision 1.32  2004-11-22 19:50:53+05:30  Cprogrammer
 * include regex.h after sys/types.h to avoid compilation prob on RH 7.3
 *
 * Revision 1.31  2004-10-22 20:30:35+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.30  2004-10-21 21:56:21+05:30  Cprogrammer
 * change for two additional arguments to strerr_die()
 *
 * Revision 1.29  2004-10-11 14:06:14+05:30  Cprogrammer
 * use control_readulong instead of control_readint
 *
 * Revision 1.28  2004-09-22 23:14:20+05:30  Cprogrammer
 * replaced atoi() with scan_int()
 *
 * Revision 1.27  2004-09-08 10:54:49+05:30  Cprogrammer
 * incorrect exit code in report() function for remote
 * mails. Caused qmail-rspawn to report "Unable to run qmail-remote"
 *
 * Revision 1.26  2004-07-17 21:23:31+05:30  Cprogrammer
 * change qqeh code in qmail-remote
 *
 * Revision 1.25  2004-07-15 23:40:46+05:30  Cprogrammer
 * fixed compilation warning
 *
 * Revision 1.24  2004-07-02 16:15:25+05:30  Cprogrammer
 * override control files rejectspam, spamredirect by
 * environment variables REJECTSPAM and SPAMREDIRECT
 * allow patterns in domain specification in the control files
 * spamfilterargs, filterargs, rejectspam and spamredirect
 *
 * Revision 1.23  2004-06-03 22:58:34+05:30  Cprogrammer
 * fixed compilation problem without indimail
 *
 * Revision 1.22  2004-05-23 22:18:17+05:30  Cprogrammer
 * added envrules filename as argument
 *
 * Revision 1.21  2004-05-19 23:15:07+05:30  Cprogrammer
 * added comments
 *
 * Revision 1.20  2004-05-12 22:37:47+05:30  Cprogrammer
 * added check DATALIMIT check
 *
 * Revision 1.19  2004-05-03 22:17:36+05:30  Cprogrammer
 * use QUEUE_BASE instead of auto_qmail
 *
 * Revision 1.18  2004-02-13 14:51:24+05:30  Cprogrammer
 * added envrules
 *
 * Revision 1.17  2004-01-20 06:56:56+05:30  Cprogrammer
 * unset FILTERARGS for notifications
 *
 * Revision 1.16  2004-01-20 01:52:08+05:30  Cprogrammer
 * report string length corrected
 *
 * Revision 1.15  2004-01-10 09:44:36+05:30  Cprogrammer
 * added comment for exit codes of bogofilter
 *
 * Revision 1.14  2004-01-08 00:32:49+05:30  Cprogrammer
 * use TMPDIR environment variable for temporary directory
 * send spam reports to central spam logger
 *
 * Revision 1.13  2003-12-30 00:44:42+05:30  Cprogrammer
 * set argv[0] from spamfilterprog
 *
 * Revision 1.12  2003-12-22 18:34:25+05:30  Cprogrammer
 * replaced spfcheck() with address_match()
 *
 * Revision 1.11  2003-12-20 01:35:06+05:30  Cprogrammer
 * added wait_pid to prevent zombies
 *
 * Revision 1.10  2003-12-17 23:33:39+05:30  Cprogrammer
 * improved logic for getting remote/local tokens
 *
 * Revision 1.9  2003-12-16 10:38:24+05:30  Cprogrammer
 * fixed incorrect address being returned if filterargs contained local: or
 * remote: directives
 *
 * Revision 1.8  2003-12-15 20:46:19+05:30  Cprogrammer
 * added case 100 to bounce mail
 *
 * Revision 1.7  2003-12-15 13:51:44+05:30  Cprogrammer
 * code to run additional filters using /bin/sh
 *
 * Revision 1.6  2003-12-14 11:36:18+05:30  Cprogrammer
 * added option to blackhole spammers
 *
 * Revision 1.5  2003-12-13 21:08:46+05:30  Cprogrammer
 * extensive rewrite
 * common report() function for local/remote mails to report errors
 *
 * Revision 1.4  2003-12-12 20:20:55+05:30  Cprogrammer
 * use -a option to prevent using header addresses
 *
 * Revision 1.3  2003-12-09 23:37:16+05:30  Cprogrammer
 * change for spawn-filter to be called as qmail-local or qmail-remote
 *
 * Revision 1.2  2003-12-08 23:48:23+05:30  Cprogrammer
 * new function getDomainToken() to retrieve domain specific values
 * read rejectspam and spamredirect only if SPAMEXITCODE is set
 *
 * Revision 1.1  2003-12-07 13:02:00+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "fmt.h"
#include "str.h"
#include "getln.h"
#include "case.h"
#include "byte.h"
#include "constmap.h"
#include "strerr.h"
#include "env.h"
#include "substdio.h"
#include "subfd.h"
#include "stralloc.h"
#include "error.h"
#include "control.h"
#include "wait.h"
#include "qregex.h"
#include "auto_qmail.h"
#include "variables.h"
#include "envrules.h"
#include "scan.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <regex.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "lock.h"
#include "open.h"
#include "evaluate.h"
#include "now.h"
#include "scan.h"
#include "MakeArgs.h"

#define REGCOMP(X,Y)    regcomp(&X, Y, REG_EXTENDED|REG_ICASE)
#define REGEXEC(X,Y)    regexec(&X, Y, (size_t) 0, (regmatch_t *) 0, (int) 0)
#ifndef REG_NOERROR
#define REG_NOERROR 0
#endif

static int      mkTempFile(int);
static void     report(int, char *, char *, char *, char *, char *, char *);
char           *getDomainToken(char *, stralloc *);
static int      run_mailfilter(char *, char *, char *, char *, char **);
void            log_spam(char *, char *, char *, stralloc *);
int             wildmat_internal(char *, char *);
static int      redirect_mail(char *, char *, char *, char *);
static void     create_logfilter();
static int      check_size(char *);
void            set_environ(char *, char *, char *, char *, char *);

static int      spfok = 0;
static stralloc spf = { 0 };
static int      sppok = 0;
static stralloc spp = { 0 };
struct constmap mapspf;
struct constmap mapspp;
static int      remotE;
stralloc        sender = { 0 };
stralloc        recipient = { 0 };
stralloc        QueueBase = { 0 };

static void
report(int errCode, char *s1, char *s2, char *s3, char *s4, char *s5, char *s6)
{
	if (!remotE) /*- strerr_die does not return */
		strerr_die6x(errCode, s1, s2, s3, s4, s5, s6);
	if (!errCode) {
		if (substdio_put(subfdoutsmall, 
			"r\0Kfilter accepted message.\n"
			"filter said: 250 ok notification queued\n\0", 82) == -1)
			_exit(111);
	} else {
		/*- h - hard, s - soft */
		if (substdio_put(subfdoutsmall, errCode == 111 ? "s" : "h", 1) == -1)
			_exit(111);
		if (s1 && substdio_puts(subfdoutsmall, s1) == -1)
			_exit(111);
		if (s2 && substdio_puts(subfdoutsmall, s2) == -1)
			_exit(111);
		if (s3 && substdio_puts(subfdoutsmall, s3) == -1)
			_exit(111);
		if (s4 && substdio_puts(subfdoutsmall, s4) == -1)
			_exit(111);
		if (s5 && substdio_puts(subfdoutsmall, s5) == -1)
			_exit(111);
		if (s6 && substdio_puts(subfdoutsmall, s6) == -1)
			_exit(111);
		if (substdio_put(subfdoutsmall, "\0", 1) == -1)
			_exit(111);
		if (substdio_puts(subfdoutsmall, 
			errCode == 111 ?  "Zfilter said: Message deferred" : "DGiving up on filter\n") == -1)
			_exit(111);
		if (substdio_put(subfdoutsmall, "\0", 1) == -1)
			_exit(111);
	}
	substdio_flush(subfdoutsmall);
	/*- For qmail-rspawn to stop complaining unable to run qmail-remote */
	_exit(0);
}

void
log_spam(char *arg1, char *arg2, char *size, stralloc *line)
{
	int             logfifo, match;
	char           *fifo_name;
	char            strnum[FMT_ULONG];
	struct stat     statbuf;
	static char     spambuf[256], inbuf[1024];
	static substdio spamin;
	static substdio spamout;

	fifo_name = env_get("LOGFILTER");
	if (!fifo_name || !*fifo_name)
		return;
	if (*fifo_name != '/')
		return;
	if ((logfifo = open(fifo_name, O_NDELAY|O_WRONLY)) == -1) {
		if (errno == ENXIO)
			return;
		report(111, "spawn-filter: open: ", fifo_name, ": ", error_str(errno), ". (#4.3.0)", 0);
	}
	substdio_fdbuf(&spamout, write, logfifo, spambuf, sizeof(spambuf));
	if (substdio_puts(&spamout, remotE ? "qmail-remote: ": "qmail-local: ") == -1)
		report(111, "spawn-filter: write: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (substdio_puts(&spamout, "pid ") == -1)
		report(111, "spawn-filter: write: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	strnum[fmt_ulong(strnum, getpid())] = 0;
	if (substdio_puts(&spamout, strnum) == -1)
		report(111, "spawn-filter: write: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (substdio_puts(&spamout, " MAIL from <") == -1)
		report(111, "spawn-filter: write: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (substdio_puts(&spamout, arg1) == -1)
		report(111, "spawn-filter: write: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (substdio_puts(&spamout, "> RCPT <") == -1)
		report(111, "spawn-filter: write: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (substdio_puts(&spamout, arg2) == -1)
		report(111, "spawn-filter: write: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (substdio_puts(&spamout, "> Size: ") == -1)
		report(111, "spawn-filter: write: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (substdio_puts(&spamout, size) == -1)
		report(111, "spawn-filter: write: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	/*
	 * Read X-Bogosity line from bogofilter
	 * on fd 255. Write it to LOGFILTER
	 * fifo for qmail-cat spamlogger
	 */
	if (!fstat(255, &statbuf) && statbuf.st_size > 0 && !lseek(255, 0, SEEK_SET)) {
		if (substdio_puts(&spamout, " ") == -1)
			report(111, "spawn-filter: write: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		substdio_fdbuf(&spamin, read, 255, inbuf, sizeof(inbuf));
		if (getln(&spamin, line, &match, '\n') == -1)
			report(111, "spawn-filter: read: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		close(255);
		if (!stralloc_0(line))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (line->len) {
			if (substdio_puts(&spamout, line->s) == -1)
				report(111, "spawn-filter: write: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		}
	}
	if (substdio_puts(&spamout, "\n") == -1)
		report(111, "spawn-filter: write: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (substdio_flush(&spamout) == -1)
		report(111, "spawn-filter: write: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	close(logfifo);
	return;
}

void
set_environ(char *host, char *ext, char *qqeh, char *sender, char *recipient)
{
	if (!env_put2("DOMAIN", host)) 
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (!env_put2("_EXT", ext))
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (!env_put2("_QQEH", qqeh))
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (!env_put2("_SENDER", sender))
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (!env_put2("_RECIPIENT", recipient))
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	return;
}

static int
run_mailfilter(char *domain, char *ext, char *qqeh, char *mailprog, char **argv)
{
	char            strnum[FMT_ULONG];
	pid_t           filt_pid;
	int             pipefd[2], pipefe[2];
	int             wstat, filt_exitcode, len = 0;
	char           *filterargs;
	static stralloc filterdefs = { 0 };
	static char     errstr[1024];
	char            inbuf[1024];
	char            ch;
	static substdio errbuf;

	if (!(filterargs = env_get("FILTERARGS"))) {
		if (control_readfile(&filterdefs, "filterargs", 0) == -1)
			report(111, "spawn-filter: Unable to read filterargs: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		filterargs = getDomainToken(domain, &filterdefs);
	}
	if (!filterargs) {
		execv(mailprog, argv); /*- do the delivery (qmail-local/qmail-remote) */
		report(111, "spawn-filter: could not exec ", mailprog, ": ", error_str(errno), ". (#4.3.0)", 0);
		_exit(111); /*- To make compiler happy */
	}
	if (pipe(pipefd) == -1)
		report(111, "spawn-filter: Trouble creating pipes: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (pipe(pipefe) == -1)
		report(111, "spawn-filter: Trouble creating pipes: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	switch ((filt_pid = fork()))
	{
	case -1:
		report(111, "spawn-filter: Trouble creating child filter: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	case 0: /*- Filter Program */
		set_environ(domain, ext, qqeh, sender.s, recipient.s);
		/*- Mail content read from fd 0 */
		if (mkTempFile(0))
			report(111, "spawn-filter: lseek error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		/*- stdout will go here */
		if (dup2(pipefd[1], 1) == -1 || close(pipefd[0]) == -1)
			report(111, "spawn-filter: dup2 error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (pipefd[1] != 1)
			close(pipefd[1]);
		/*- stderr will go here */
		if (dup2(pipefe[1], 2) == -1 || close(pipefe[0]) == -1)
			report(111, "spawn-filter: dup2 error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (pipefe[1] != 2)
			close(pipefe[1]);
		/*- Avoid loop if program(s) defined by FILTERARGS call qmail-inject, etc */
		if (!env_unset("FILTERARGS") || !env_unset("SPAMFILTER"))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		execl("/bin/sh", "sh", "-c", filterargs, (char *) 0);
		report(111, "spawn-filter: could not exec /bin/sh: ",  filterargs, ": ", error_str(errno), ". (#4.3.0)", 0);
	default:
		close(pipefe[1]);
		close(pipefd[1]);
		if (dup2(pipefd[0], 0)) {
			close(pipefd[0]);
			close(pipefe[0]);
			wait_pid(&wstat, filt_pid);
			report(111, "spawn-filter: dup2 error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		}
		if (pipefd[0] != 0)
			close(pipefd[0]);
		if (mkTempFile(0)) {
			close(0);
			close(pipefe[0]);
			wait_pid(&wstat, filt_pid);
			report(111, "spawn-filter: lseek error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		}
		break;
	}
	/*- Process message if exit code is 0, bounce if 100 */
	if (wait_pid(&wstat, filt_pid) != filt_pid) {
		close(0);
		close(pipefe[0]);
		report(111, "spawn-filter: waitpid surprise: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	}
	if (wait_crashed(wstat)) {
		close(0);
		close(pipefe[0]);
		report(111, "spawn-filter: filter crashed: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	}
	switch (filt_exitcode = wait_exitcode(wstat))
	{
	case 0:
		execv(mailprog, argv); /*- do the delivery (qmail-local/qmail-remote) */
		report(111, "spawn-filter: could not exec ", mailprog, ": ", error_str(errno), ". (#4.3.0)", 0);
	case 2:
		report(0, "blackholed: ", filterargs, 0, 0, 0, 0); /*- Blackhole */
	case 100:
		report(100, "Mail Rejected (#5.7.1)", 0, 0, 0, 0, 0);
	default:
		substdio_fdbuf(&errbuf, read, pipefe[0], inbuf, sizeof(inbuf));
		for (len = 0; substdio_bget(&errbuf, &ch, 1) && len < (sizeof(errstr) - 1); len++)
			errstr[len] = ch;
		errstr[len] = 0;
		strnum[fmt_ulong(strnum, filt_exitcode)] = 0;
		report(111, filterargs, ": (spawn-filter) exit code: ", strnum, *errstr ? ": " : 0, *errstr ? errstr : 0, ". (#4.3.0)");
	}
	/*- Not reached */
	return(111);
}

char           *
getDomainToken(char *domain, stralloc *sa)
{
	regex_t         qreg;
	int             len, n, retval;
	char           *ptr, *p;

	for (len = 0, ptr = sa->s;len < sa->len;) {
		len += ((n = str_len(ptr)) + 1);
		for (p = ptr;*p && *p != ':';p++);
		if (*p == ':') {
			*p = 0;
			/*- build the regex */
			if ((retval = str_diff(ptr, domain))) {
				if (env_get("QREGEX")) {
					if ((retval = REGCOMP(qreg, ptr)) == 0)
						retval = (REGEXEC(qreg, domain) == REG_NOMATCH ? 1 : REG_NOERROR);
					regfree(&qreg);
				} else
					retval = !wildmat_internal(domain, ptr);
			}
			*p = ':';
			if (!retval) { /*- match occurred for domain or wildcard */
				/* check for local/remote directives */
				if (remotE) { /*- remote delivery */
					if (!str_diffn(p + 1, "remote:", 7))
						return (p + 8);
					if (!str_diffn(p + 1, "local:", 6)) {
						ptr = sa->s + len;
						continue; /*- skip local directives for remote mails */
					}
				} else { /*- local delivery */
					if (!str_diffn(p + 1, "local:", 6))
						return (p + 7);
					if (!str_diffn(p + 1, "remote:", 7)) {
						ptr = sa->s + len;
						continue; /*- skip remote directives for local mails */
					}
				}
				return (p + 1);
			}
		}
		ptr = sa->s + len;
	} /*- for (len = 0, ptr = sa->s;len < sa->len;) */
	return ((char *) 0);
}

int
mkTempFile(int seekfd)
{
	char            inbuf[2048], outbuf[2048], strnum[FMT_ULONG];
	char           *tmpdir;
	static stralloc tmpFile = {0};
	struct substdio _ssin;
	struct substdio _ssout;
	int             fd;

	if (lseek(seekfd, 0, SEEK_SET) == 0)
		return (0);
	if (errno == EBADF) {
		strnum[fmt_ulong(strnum, seekfd)] = 0;
		report(111, "spawn-filter: fd ", strnum, ": ", error_str(errno), ". (#4.3.0)", 0);
	}
	if (!(tmpdir = env_get("TMPDIR")))
		tmpdir = "/tmp";
	if (!stralloc_copys(&tmpFile, tmpdir))
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (!stralloc_cats(&tmpFile, "/qmailFilterXXX"))
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (!stralloc_catb(&tmpFile, strnum, fmt_ulong(strnum, (unsigned long) getpid())))
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (!stralloc_0(&tmpFile))
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if ((fd = open(tmpFile.s, O_RDWR | O_EXCL | O_CREAT, 0600)) == -1)
		report(111, "spawn-filter: ", tmpFile.s, ": ", error_str(errno), ". (#4.3.0)", 0);
	unlink(tmpFile.s);
	substdio_fdbuf(&_ssout, write, fd, outbuf, sizeof(outbuf));
	substdio_fdbuf(&_ssin, read, seekfd, inbuf, sizeof(inbuf));
	switch (substdio_copy(&_ssout, &_ssin))
	{
	case -2: /*- read error */
		report(111, "spawn-filter: read error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	case -3: /*- write error */
		report(111, "spawn-filter: write error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	}
	if (substdio_flush(&_ssout) == -1)
		report(111, "spawn-filter: write error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (dup2(fd, seekfd) == -1)
		report(111, "spawn-filter: dup2 error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (lseek(seekfd, 0, SEEK_SET) != 0)
		report(111, "spawn-filter: lseek: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	return (0);
}

static void
create_logfilter()
{
	int             fd;
	char           *tmpdir;
	char            strnum[FMT_ULONG];
	static stralloc tmpFile = { 0 };

	if (env_get("LOGFILTER")) {
		if (!(tmpdir = env_get("TMPDIR")))
			tmpdir = "/tmp";
		if (!stralloc_copys(&tmpFile, tmpdir))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_cats(&tmpFile, "/smtpFilterXXX"))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_catb(&tmpFile, strnum, fmt_ulong(strnum, (unsigned long) getpid())))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_0(&tmpFile))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if ((fd = open(tmpFile.s, O_RDWR | O_EXCL | O_CREAT, 0600)) == -1)
			report(111, "spawn-filter: open: ", tmpFile.s, ": ", error_str(errno), ". (#4.3.0)", 0);
		if (unlink(tmpFile.s))
			report(111, "spawn-filter: unlink: ", tmpFile.s, ": ", error_str(errno), ". (#4.3.0)", 0);
		if (dup2(fd, 255) == -1)
			report(111, "spawn-filter: dup: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (fd != 255)
			close(fd);
	}
	return;
}

static int
redirect_mail(char *notifyaddress, char *domain, char *ext, char *qqeh)
{
	char           *(args[7]);
	char           *qbase;
	pid_t           pid;
	int             wstat;
	static stralloc Queuedir = {0};

	args[0] = "bin/qmail-inject";
	args[1] = "-a";
	args[2] = "-s";
	args[3] = "-f";
	args[4] = "\"\"";
	args[5] = notifyaddress;
	args[6] = 0;
	switch((pid = fork()))
	{
	case -1:
		report(111, "spawn-filter: Trouble creating child inject: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	case 0:
		if (!env_put2("SPAMREDIRECT", notifyaddress))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		set_environ(domain, ext, qqeh, sender.s, recipient.s);
		/*- 
		 * we do not want notifications to be
		 * caught by spamfilter :)
		 * unset SPAMFILTER
		 */
		if (!(qbase = env_get("QUEUE_BASE"))) {
			switch (control_readfile(&QueueBase, "queue_base", 0))
			{
			case -1:
				report(111, "spawn-filter: Unable to read qbase: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
				break;
			case 0:
				qbase = auto_qmail;
				break;
			case 1:
				qbase = QueueBase.s;
				break;
			}
		}
		if (!env_unset("SPAMFILTER") || !env_unset("QMAILQUEUE") || !env_unset("QUEUEDIR") || !env_unset("FILTERARGS"))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_copys(&Queuedir, "QUEUEDIR="))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_cats(&Queuedir, qbase))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_cats(&Queuedir, "/nqueue"))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_0(&Queuedir))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!env_put(Queuedir.s))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		execv(*args, args); /*- run qmail-inject */
		report(111, "spawn-filter: could not exec ", *args, ": ", error_str(errno), ". (#4.3.0)", 0);
	default:
		break;
	}
	if (wait_pid(&wstat, pid) != pid)
		report(111, "spawn-filter: qmail-inject waitpid surprise: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (wait_crashed(wstat))
		report(111, "spawn-filter: qmail-inject crashed: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	return(wait_exitcode(wstat));
}

static int
check_size(char *size)
{
	char           *x;
	unsigned long   databytes = -1, msgsize;

	if (!(x = env_get("DATABYTES"))) {
		if (control_readulong(&databytes, "databytes") == -1)
			report(111, "spawn-filter: Unable to read databytes: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	} else
		scan_ulong(x, &databytes);
	if (databytes == -1)
		return (0);
	scan_ulong(size, &msgsize);
	if (msgsize > databytes)
		return(1);
	else
		return(0);
}

int
get_rate(char *expression, double *rate)
{
	struct val      result;
	struct vartable *vt;

	/*
	 * replace all occurences of %p in expression
	 * with the value of data
	 */
	if (!(vt = create_vartable()))
		return (-1);
	switch (evaluate(expression, &result, vt))
	{
	case ERROR_SYNTAX:
		free_vartable(vt);
		report(111, "spawn-filter: syntax error: ", expression, ". (#4.3.0)", 0, 0, 0);
		return (-1);
	case ERROR_VARNOTFOUND:
		free_vartable(vt);
		report(111, "spawn-filter: variable not found: ", expression, ". (#4.3.0)", 0, 0, 0);
		return (-1);
	case ERROR_NOMEM:
		free_vartable(vt);
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		return (-1);
	case ERROR_DIV0:
		free_vartable(vt);
		report(111, "spawn-filter: division by zero: ", expression, ". (#4.3.0)", 0, 0, 0);
		return (-1);
	case RESULT_OK:
		*rate = (double) ((result.type == T_INT) ? result.ival : result.rval);
		free_vartable(vt);
		return (0);
	}
	free_vartable(vt);
	return (0);
}

stralloc        fline = { 0 }, rate_expr = { 0 };

int
is_rate_ok(char *rate_dir, char *file, char *rate_exp)
{
	int             wfd, rfd, match, line_no = -1, rate_int, access_flag = 0;
	unsigned long   email_count = 0;
	char            reset, stime[FMT_ULONG], etime[FMT_ULONG], ecount[FMT_ULONG];
	double          conf_rate, cur_rate = 0.0;
	char            inbuf[2048], outbuf[1024];
	char           *ptr;
	struct substdio ssin, ssout;
	datetime_sec    starttime, endtime;
	struct stat     statbuf;

	starttime = endtime = now();
	if (!(ptr = env_get("RATELIMIT_INTERVAL")))
		rate_int = 86400;
	else
		scan_int(ptr, &rate_int);
	reset = ((stat(file, &statbuf) ? starttime : starttime - statbuf.st_mtime) > rate_int);
	stime[fmt_ulong(stime, starttime)] = 0;
	etime[fmt_ulong(etime, endtime)] = 0;
	ecount[0] = '1'; /*- we are delivering the first email since rate control has been imposed */
	ecount[1] = 0;
	if (rate_exp) {
		access_flag = access(file, F_OK);
		if ((wfd = (access_flag ? open_excl : open_write) (file)) == -1)
			report(111, "spawn-filter: unable to write_excl: ", file, ": ", error_str(errno), ". (#4.3.0)", 0);
		if (lock_ex(wfd) == -1)
			report(111, "spawn-filter: unable to lock: ", file, ": ", error_str(errno), ". (#4.3.0)", 0);
		if (!stralloc_copys(&rate_expr, rate_exp))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		else
		if (!stralloc_catb(&rate_expr, "\n", 1))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		get_rate(rate_exp, &conf_rate);
	} else {
		if ((wfd = open_write(file)) == -1)
			report(111, "spawn-filter: unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)", 0);
		if (lock_ex(wfd) == -1)
			report(111, "spawn-filter: unable to lock: ", file, ": ", error_str(errno), ". (#4.3.0)", 0);
	}
	if (!access_flag) { /*- only if rate definition exists */
		if ((rfd = open_read(file)) == -1)
			report(111, "spawn-filter: unable to read: ", file, ": ", error_str(errno), ". (#4.3.0)", 0);
		substdio_fdbuf(&ssin, read, rfd, inbuf, sizeof(inbuf));
		for (line_no = 1;;line_no++) { /*- Line Processing */
			if (getln(&ssin, &fline, &match, '\n') == -1)
				report(111, "spawn-filter: unable to read: ", file, ": ", error_str(errno), ". (#4.3.0)", 0);
			if (!match && fline.len == 0)
				break;
			switch (line_no)
			{
			case 1: /*- rate expression */
				if (!stralloc_copy(&rate_expr, &fline))
					report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
				fline.len--;
				if (!stralloc_0(&fline))
					report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
				get_rate(fline.s, &conf_rate);
				break;
			case 2: /*- email count */
				if (reset)
					continue;
				scan_ulong(fline.s, (unsigned long *) &email_count);
				break;
			case 3: /*- start time */
				if (reset)
					continue;
				scan_ulong(fline.s, (unsigned long *) &starttime);
				stime[fmt_ulong(stime, starttime)] = 0;
				break;
			case 4: /*- current rate */
				if (reset)
					continue;
				/* do not divide by zero */
				cur_rate = (endtime == starttime) ? 0 : ((float) email_count / (float) (endtime - starttime));
				break;
			}
		}
		close(rfd);
	}
	/*-
	 * line_no   < 1        - no point in messing with invalid data
	 * conf_rate < 0        - update the email count, timestamps
	 * conf_rate = 0        - defer emails
	 * cur_rate > conf_rate - defer emails
	 */
	if (line_no < 1 || (conf_rate >= 0 && cur_rate > conf_rate)) {
		close(wfd);
		return (line_no < 1 ? 1 : 0);
	}
	ecount[fmt_ulong(ecount, ++email_count)] = 0;
	substdio_fdbuf(&ssout, write, wfd, outbuf, sizeof(outbuf));
	if (substdio_bput(&ssout, rate_expr.s, rate_expr.len) == -1)
		report(111, "spawn-filter: unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)", 0);
	if (substdio_bputs(&ssout, ecount) == -1)
		report(111, "spawn-filter: unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)", 0);
	if (substdio_bput(&ssout, "\n", 1) == -1)
		report(111, "spawn-filter: unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)", 0);
	if (substdio_bputs(&ssout, stime) == -1)
		report(111, "spawn-filter: unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)", 0);
	if (substdio_bput(&ssout, "\n", 1) == -1)
		report(111, "spawn-filter: unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)", 0);
	if (substdio_bputs(&ssout, etime) == -1)
		report(111, "spawn-filter: unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)", 0);
	if (substdio_bput(&ssout, "\n", 1) == -1)
		report(111, "spawn-filter: unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)", 0);
	if (substdio_flush(&ssout) == -1)
		report(111, "spawn-filter: unable to write: ", file, ": ", error_str(errno), ". (#4.3.0)", 0);
	close(wfd);
	return (1);
}

int
main(int argc, char **argv)
{
	char           *ptr, *mailprog, *domain, *errStr = 0, *size = "0", *qqeh, *ext;
	int             len;
	char            sizebuf[FMT_ULONG];
	struct stat     statbuf;
	int             wstat, filt_exitcode;
	pid_t           filt_pid;
	int             pipefd[2], ret;
	char           *spamfilterprog, *notifyaddress, *rejectspam, *spf_fn, *rate_dir, *rate_exp;
	char          **Argv;
	stralloc        spamfilterargs = { 0 };
	stralloc        spamfilterdefs = { 0 };
	stralloc        rejectspamlist = { 0 };
	stralloc        addresslist = { 0 };
	stralloc        ratedefs = { 0 };
	stralloc        line = { 0 };
	int             spamcode = 0, hamcode = 1, unsurecode = 2;

	len = str_len(argv[0]);
	for (ptr = argv[0] + len;*ptr != '/' && ptr != argv[0];ptr--);
	if (*ptr && *ptr == '/')
		ptr++;
	ptr += 6;
	if (*ptr == 'l') { /*- qmail-local Filter */
		mailprog = "sbin/qmail-local";
		domain = argv[7];
		ext = argv[6];
		qqeh = argv[10];
		remotE = 0;
		if (!env_unset("QMAILREMOTE"))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!fstat(0, &statbuf)) {
			sizebuf[fmt_ulong(sizebuf, statbuf.st_size)] = 0;
			size = sizebuf;
		} else
			size = "0";
		/*- sender */
		if (!stralloc_copys(&sender, argv[8]))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_0(&sender))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		/*- recipient */
		if (*ext) { /*- EXT */
			if (!stralloc_copys(&recipient, ext))
				report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		} else /*- user */
			if (!stralloc_copys(&recipient, argv[2]))
				report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_cats(&recipient, "@"))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_cats(&recipient, domain))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_0(&recipient))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	} else
	if (*ptr == 'r') { /*- qmail-remote Filter */
		mailprog = "sbin/qmail-remote";
		domain = argv[1];
		ext = argv[5];
		qqeh = argv[3];
		size = argv[4];
		remotE = 1;
		if (!env_unset("QMAILLOCAL"))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		/*- sender */
		if (!stralloc_copys(&sender, argv[2]))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_0(&sender))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		/*- recipient */
		if (!stralloc_copys(&recipient, argv[5]))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_0(&recipient))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	} else {
		report(111, "spawn-filter: Incorrect usage. ", argv[0], " (#4.3.0)", 0, 0, 0);
		_exit(111);
	}
	if (chdir(auto_qmail) == -1)
		report(111, "spawn-filter: Unable to switch to ", auto_qmail, ": ", error_str(errno), ". (#4.3.0)", 0);
	if ((ret = envrules(sender.s, "fromd.envrules", "FROMRULES", 0)) == -1)
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	else
	if (ret == -2)
		report(111, "spawn-filter: Unable to read from envrules: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	else
	if (ret == -4)
		report(111, "spawn-filter: regex compilation failed: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if ((ret = envrules(recipient.s, "rcpt.envrules", "RCPTRULES", 0)) == -1)
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	else
	if (ret == -2)
		report(111, "spawn-filter: Unable to read rcpt envrules: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	else
	if (ret == -4)
		report(111, "spawn-filter: regex compilation failed: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if ((rate_dir = env_get("RATELIMIT_DIR"))) {
		if (chdir(rate_dir))
			report(111, "spawn-filter: Unable to switch to ", rate_dir, ": ", error_str(errno), ". (#4.3.0)", 0);
		if (!access(domain, W_OK)) {
			if (!is_rate_ok(rate_dir, domain, 0))
				report(111, "spawn-filter: high email rate for ", domain, ". (#4.3.0)", 0, 0, 0);
		} else
		if (!access("ratecontrol", R_OK)) {
			if (control_readfile(&ratedefs, "./ratecontrol", 0) == -1)
				report(111, "spawn-filter: Unable to read ratecontrol: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
			rate_exp = getDomainToken(domain, &ratedefs);
			if (!is_rate_ok(rate_dir, domain, rate_exp))
				report(111, "spawn-filter: high email rate for ", domain, ". (#4.3.0)", 0, 0, 0);
		} else
		if (!access(".global", W_OK) && !is_rate_ok(rate_dir, ".global", 0))
			report(111, "spawn-filter: high email rate for ", domain, ". (#4.3.0)", 0, 0, 0);
		if (chdir(auto_qmail) == -1)
			report(111, "spawn-filter: Unable to switch to ", auto_qmail, ": ", error_str(errno), ". (#4.3.0)", 0);
	}
	/*- DATABYTES Check */
	if (check_size(size))
		report(100, "sorry, that message size exceeds my databytes limit (#5.3.4)", 0, 0, 0, 0, 0);
	if (!(spamfilterprog = env_get("SPAMFILTER"))) {
		if (control_readfile(&spamfilterdefs, "spamfilter", 0) == -1)
			report(111, "spawn-filter: Unable to read spamfilter: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!(spamfilterprog = getDomainToken(domain, &spamfilterdefs))) {
			run_mailfilter(domain, ext, qqeh, mailprog, argv);
			report(111, "spawn-filter: could not exec ", mailprog, ": ", error_str(errno), ". (#4.3.0)", 0);
		}
	}
	if (!(spf_fn = env_get("SPAMIGNORE")))
		spf_fn = "spamignore";
	if ((spfok = control_readfile(&spf, ptr, 0)) == -1)
		report(111, "spawn-filter: Unable to read ", ptr, ": ", error_str(errno), ". (#4.3.0)", 0);
	if (spfok && !constmap_init(&mapspf, spf.s, spf.len, 0))
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (!(ptr = env_get("SPAMIGNOREPATTERNS")))
		ptr = "spamignorepatterns";
	if ((sppok = control_readfile(&spp, ptr, 0)) == -1)
		report(111, "spawn-filter: Unable to read ", ptr, ": ", error_str(errno), ". (#4.3.0)", 0);
	if (sppok && !constmap_init(&mapspp, spp.s, spp.len, 0))
		report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	/*
	 * Check if addr is in spamignore or spamignorepatterns
	 */
	switch (address_match(spf_fn, &sender, spfok ? &spf : 0, spfok ? &mapspf : 0, sppok ? &spp : 0, &errStr))
	{
	case 1:
		run_mailfilter(domain, ext, qqeh, mailprog, argv);
		report(111, "spawn-filter: could not exec ", mailprog, ": ", error_str(errno), ". (#4.3.0)", 0);
	case 0:
		break;
	case -1:
		report(111, "spawn-filter: out of mem: ", errStr, ". (#4.3.0)", 0, 0, 0);
	case -2:
		report(111, "spawn-filter: address_match: Unable to read cdb ", spf_fn, ": ", errStr, ". (#4.3.0)", 0);
	case -3:
		report(111, "spawn-filter: address_match: lseek error: ", errStr, ". (#4.3.0)", 0, 0, 0);
	case -4:
		report(111, "spawn-filter: address_match: regex compilation failed: ", errStr, ". (#4.3.0)", 0, 0, 0);
	default:
		report(111, "spawn-filter: address_match: ", errStr, ". (#4.3.0)", 0, 0, 0);
		break;
	}
	notifyaddress = rejectspam = 0;
	if (pipe(pipefd) == -1)
		report(111, "spawn-filter: Trouble creating pipes: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	create_logfilter();
	switch ((filt_pid = fork()))
	{
	case -1:
		report(111, "spawn-filter: Trouble creating child spamfilter: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	case 0: /*- SPAM Filter Program */
		set_environ(domain, ext, qqeh, sender.s, recipient.s);
		if (!stralloc_copys(&spamfilterargs, spamfilterprog))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!stralloc_0(&spamfilterargs))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (!(Argv = MakeArgs(spamfilterargs.s)))
			report(111, "spawn-filter: out of mem: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		/*- Mail content read from fd 0 */
		if (mkTempFile(0))
			report(111, "spawn-filter: lseek error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (dup2(pipefd[1], 1) == -1 || close(pipefd[0]) == -1)
			report(111, "spawn-filter: dup2 error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		if (pipefd[1] != 1)
			close(pipefd[1]);
		execv(*Argv, Argv); /*- run the spam filter */
		report(111, "spawn-filter: could not exec ", *Argv, ": ", error_str(errno), ". (#4.3.0)", 0);
	default:
		close(pipefd[1]);
		if (dup2(pipefd[0], 0)) {
			close(pipefd[0]);
			wait_pid(&wstat, filt_pid);
			report(111, "spawn-filter: dup2 error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		}
		if (pipefd[0] != 0)
			close(pipefd[0]);
		if (mkTempFile(0)) {
			close(0);
			wait_pid(&wstat, filt_pid);
			report(111, "spawn-filter: lseek error: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
		}
		break;
	}
	/*- Process message if exit code is 0, 1, 2 */
	if (wait_pid(&wstat, filt_pid) != filt_pid)
		report(111, "spawn-filter: waitpid surprise: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	if (wait_crashed(wstat))
		report(111, "spawn-filter: filter crashed: ", error_str(errno), ". (#4.3.0)", 0, 0, 0);
	/* write data to spamlogger */
	log_spam(sender.s, recipient.s, size, &line);
	if ((ptr = env_get("SPAMEXITCODE"))) 
		scan_int(ptr, &spamcode);
	else
		report(111, "spawn-filter: SPAMEXITCODE undefined. (#4.3.0)", 0, 0, 0, 0, 0);
	if ((ptr = env_get("HAMEXITCODE")))
		scan_int(ptr, &hamcode);
	if ((ptr = env_get("UNSUREEXITCODE")))
		scan_int(ptr, &unsurecode);
	filt_exitcode = wait_exitcode(wstat);
	if (filt_exitcode == spamcode || (filt_exitcode == hamcode) || (filt_exitcode == unsurecode)) {
		if (spamcode == filt_exitcode) { /*- Message is SPAM */
			if (!(rejectspam = env_get("REJECTSPAM"))) {
				if (control_readfile(&rejectspamlist, "rejectspam", 0) == -1)
					report(111, "spawn-filter: Unable to read ", ptr, ": ", error_str(errno), ". (#4.3.0)", 0);
				rejectspam = getDomainToken(domain, &rejectspamlist);
			}
			if (!(notifyaddress = env_get("SPAMREDIRECT"))) {
				if (control_readfile(&addresslist, "spamredirect", 0) == -1)
					report(111, "spawn-filter: Unable to read ", ptr, ": ", error_str(errno), ". (#4.3.0)", 0);
				notifyaddress = getDomainToken(domain, &addresslist);
			}
			if (notifyaddress && *notifyaddress && redirect_mail(notifyaddress, domain, ext, qqeh))
				report(111, "spawn-filter: unable to send spam notification. (#4.3.0)", 0, 0, 0, 0, 0);
			if (rejectspam && *rejectspam > '0') {
				if (*rejectspam == '1')
					report(100, "SPAM or junk mail threshold exceeded (#5.7.1)", 0, 0, 0, 0, 0);
				else /*- BLACKHOLE sender */
					report(0, 0, 0, 0, 0, 0, 0);
			}
			if (notifyaddress && *notifyaddress)
				report(0, 0, 0, 0, 0, 0, 0);
		}
	} else
		report(111, "spawn-filter bug. (#4.3.0)", 0, 0, 0, 0, 0);
	run_mailfilter(domain, ext, qqeh, mailprog, argv);
	report(111, "spawn-filter: could not exec ", mailprog, ": ", error_str(errno), ". (#4.3.0)", 0);
	_exit(111);
	/*- Not reached */
	return(0);
}

void
getversion_qmail_spawn_filter_c()
{
	static char    *x = "$Id: spawn-filter.c,v 1.68 2019-09-30 22:59:06+05:30 Cprogrammer Exp mbhangui $";

	x++;
	if (x)
		x = sccsidevalh;
}
