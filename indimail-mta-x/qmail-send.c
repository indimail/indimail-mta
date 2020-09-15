/*
 * $Log: qmail-send.c,v $
 * Revision 1.70  2020-09-15 21:09:07+05:30  Cprogrammer
 * use control files conf-fsync, conf-syncdir to turn on fsync, bsd style syncdir semantics
 * set / unset USE_FSYNC, USE_SYNCDIR env variables
 *
 * Revision 1.69  2020-07-04 22:23:52+05:30  Cprogrammer
 * replaced utime() with utimes()
 *
 * Revision 1.68  2020-05-19 10:33:59+05:30  Cprogrammer
 * define use_fsync for non-external qmail-todo
 *
 * Revision 1.67  2020-05-16 09:57:09+05:30  Cprogrammer
 * avoid possible integer overflow in rewrite()
 *
 * Revision 1.66  2020-05-15 10:49:07+05:30  Cprogrammer
 * converted function prototypes to c89 standard
 * use unsigned int to store return value of str_len
 *
 * Revision 1.65  2019-06-26 18:45:51+05:30  Cprogrammer
 * insert X-Bounced-Address header for bounces
 *
 * Revision 1.64  2018-07-03 01:59:03+05:30  Cprogrammer
 * fixed indentation
 *
 * Revision 1.63  2018-07-03 01:52:37+05:30  Cprogrammer
 * reread envnoathost on HUP
 *
 * Revision 1.62  2018-01-09 11:53:03+05:30  Cprogrammer
 * removed non-indimail code
 *
 * Revision 1.61  2017-03-31 21:10:10+05:30  Cprogrammer
 * log null addresses in log_stat() as <>
 *
 * Revision 1.60  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.59  2016-03-31 17:38:54+05:30  Cprogrammer
 * added log lock code to ensure complete lines get written when running as a multi delivery process
 * flush logs only when line gets completed
 *
 * Revision 1.58  2016-01-29 18:31:00+05:30  Cprogrammer
 * include queue name in logs
 *
 * Revision 1.57  2014-03-07 19:46:01+05:30  Cprogrammer
 * fixed issue with bounce processor returning non-zero exit status
 *
 * Revision 1.56  2014-02-10 16:49:05+05:30  Cprogrammer
 * added discard bounce feature
 *
 * Revision 1.55  2014-01-22 20:38:42+05:30  Cprogrammer
 * added hassrs.h
 *
 * Revision 1.54  2013-09-23 22:13:56+05:30  Cprogrammer
 * added queue specific concurrency
 *
 * Revision 1.53  2013-08-25 18:38:05+05:30  Cprogrammer
 * added SRS
 *
 * Revision 1.52  2013-05-16 23:32:33+05:30  Cprogrammer
 * added log_stat as part of non-indimail code
 *
 * Revision 1.51  2011-07-29 09:29:48+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.50  2011-05-18 12:56:38+05:30  Cprogrammer
 * fix qmail-queue waiting endlessly if bounceprocessor returned non-zero status
 *
 * Revision 1.49  2011-04-16 11:29:28+05:30  Cprogrammer
 * check for write errors
 *
 * Revision 1.48  2010-07-23 13:59:20+05:30  Cprogrammer
 * fixed envrules() not working
 *
 * Revision 1.47  2010-07-21 21:20:51+05:30  Cprogrammer
 * added envheader code
 *
 * Revision 1.46  2010-07-20 18:39:03+05:30  Cprogrammer
 * changed order of arguments of original recipient and bounce sender when running
 * bounceprocessor script
 *
 * Revision 1.45  2010-07-18 19:20:18+05:30  Cprogrammer
 * report all recipients in log_stat()
 * added startup plugins functionality
 *
 * Revision 1.44  2009-09-01 22:03:10+05:30  Cprogrammer
 * added Bounce Address Tag Validation (BATV) code
 *
 * Revision 1.43  2009-05-05 03:19:17+05:30  Cprogrammer
 * added bounce filename, original recipient and bounce report as argument to bounce processor
 *
 * Revision 1.42  2009-05-04 10:32:08+05:30  Cprogrammer
 * fixed recipient get passed twice as an argument
 *
 * Revision 1.41  2009-05-03 22:45:59+05:30  Cprogrammer
 * added bounce_process() function
 *
 * Revision 1.40  2009-05-01 10:41:28+05:30  Cprogrammer
 * added errstr argument to envrules()
 *
 * Revision 1.39  2009-04-10 14:14:15+05:30  Cprogrammer
 * added restore_env() to reset environment
 *
 * Revision 1.38  2009-03-20 22:36:18+05:30  Cprogrammer
 * added BOUNCEQUEUE patch
 *
 * Revision 1.37  2009-02-01 00:07:32+05:30  Cprogrammer
 * display system error when unable to switch directory
 *
 * Revision 1.36  2008-06-12 08:40:08+05:30  Cprogrammer
 * added envrules() to modify behaviour of bounces
 *
 * Revision 1.35  2007-12-20 14:06:09+05:30  Cprogrammer
 * unset SPAMFILTER & FILTERARGS before calling qmail-queue
 *
 * Revision 1.34  2005-12-29 14:03:35+05:30  Cprogrammer
 * separate queuelifetime for bounce messages
 *
 * Revision 1.33  2005-08-23 17:35:29+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.32  2005-03-03 16:12:24+05:30  Cprogrammer
 * minimum interval in secs for todo run
 *
 * Revision 1.31  2004-12-20 22:57:23+05:30  Cprogrammer
 * change log2() to my_log2() to avoid conflicts with fedora3
 *
 * Revision 1.30  2004-10-22 20:29:34+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.29  2004-10-22 15:38:00+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.28  2004-10-22 14:44:58+05:30  Cprogrammer
 * added argument to control_readnativefile()
 *
 * Revision 1.27  2004-08-15 12:54:43+05:30  Cprogrammer
 * qqeh bug fix
 *
 * Revision 1.26  2004-07-17 21:21:40+05:30  Cprogrammer
 * added qqeh code
 * added RCS log
 *
 * Revision 1.25  2004-02-03 13:51:15+05:30  Cprogrammer
 * unset SPAMFILTER, FILTERARGS for bounces
 *
 * Revision 1.24  2004-01-14 23:41:46+05:30  Cprogrammer
 * delay fsync()
 *
 * Revision 1.23  2004-01-02 10:16:31+05:30  Cprogrammer
 * prevent segmentation fault in log_stat() when to or from is null
 * reset to and from
 *
 * Revision 1.22  2003-12-31 20:03:22+05:30  Cprogrammer
 * added use_fsync to turn on/off fsync
 *
 * Revision 1.21  2003-12-15 13:52:46+05:30  Cprogrammer
 * preserve mime when bouncing
 *
 * Revision 1.20  2003-12-09 21:26:42+05:30  Cprogrammer
 * corrected non-indimail compilation
 *
 * Revision 1.19  2003-11-29 23:44:33+05:30  Cprogrammer
 * documentation added for newlocals and newvdoms
 *
 * Revision 1.18  2003-10-23 01:26:16+05:30  Cprogrammer
 * used struct utimbuf for utime()
 * fixed compilation warnings
 *
 * Revision 1.17  2003-10-17 21:03:43+05:30  Cprogrammer
 * added log_stat() to log delivery messages
 *
 * Revision 1.16  2003-10-01 19:05:50+05:30  Cprogrammer
 * changed return type to int
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include "sig.h"
#include "direntry.h"
#include "control.h"
#include "select.h"
#include "open.h"
#include "seek.h"
#include "exit.h"
#include "lock.h"
#include "ndelay.h"
#include "now.h"
#include "getln.h"
#include "substdio.h"
#include "alloc.h"
#include "error.h"
#include "stralloc.h"
#include "str.h"
#include "byte.h"
#include "fmt.h"
#include "scan.h"
#include "case.h"
#include "auto_qmail.h"
#include "auto_control.h"
#include "trigger.h"
#include "newfield.h"
#include "quote.h"
#include "qmail.h"
#include "qsutil.h"
#include "prioq.h"
#include "constmap.h"
#include "fmtqfn.h"
#include "qmail-todo.h"
#include "env.h"
#include "envrules.h"
#include "variables.h"
#include "readsubdir.h"
#include "hassrs.h"
#ifdef HAVESRS
#include "srs.h"
#endif
#include "wait.h"

/*- critical timing feature #1: if not triggered, do not busy-loop */
/*- critical timing feature #2: if triggered, respond within fixed time */
/*- important timing feature: when triggered, respond instantly */
#define SLEEP_TODO 1500			/*- check todo/ every 25 minutes in any case */
#define ONCEEVERY 10			/*- Run todo maximal once every N seconds */
#define SLEEP_FUZZ 1			/*- slop a bit on sleeps to avoid zeno effect */
#define SLEEP_FOREVER 86400		/*- absolute maximum time spent in select() */
#define SLEEP_CLEANUP 76431		/*- time between cleanups */
#define SLEEP_SYSFAIL 123
#define OSSIFIED 129600			/*- 36 hours; _must_ exceed q-q's DEATH (24 hours) */
/*- #define MIME 1 -*/

int             lifetime = 604800;
#ifdef BOUNCELIFETIME
int             bouncelifetime = 604800;
#endif
int             bouncemaxbytes = 50000;
stralloc        percenthack = { 0 };

struct constmap mappercenthack;
stralloc        locals = { 0 };
stralloc        bouncemessage = { 0 };
stralloc        bouncesubject = { 0 };
stralloc        doublebouncemessage = { 0 };
stralloc        doublebouncesubject = { 0 };

#ifdef HAVESRS
stralloc        srs_domain = { 0 };
#endif
struct constmap maplocals;
stralloc        vdoms = { 0 };

struct constmap mapvdoms;
stralloc        envnoathost = { 0 };
stralloc        bouncefrom = { 0 };
stralloc        bouncehost = { 0 };
stralloc        doublebounceto = { 0 };
stralloc        doublebouncehost = { 0 };

#ifdef LOCK_LOGS
stralloc        lockfn = { 0 };

int             loglock_fd = -1;
#endif

char            strnum2[FMT_ULONG];
char            strnum3[FMT_ULONG];

#define CHANNELS 2
char           *chanaddr[CHANNELS] = { "local/", "remote/" };
char           *chanstatusmsg[CHANNELS] = { " local ", " remote " };
char           *chanjobsheldmsg[CHANNELS] = { /* NJL 1998/05/03 */
	"local deliveries temporarily held\n",
	"remote deliveries temporarily held\n"
};
char           *chanjobsunheldmsg[CHANNELS] = {	/* NJL 1998/05/03 */
	"local deliveries resumed\n",
	"remote deliveries resumed\n"
};
char           *tochan[CHANNELS] = { " to local ", " to remote " };
int             chanfdout[CHANNELS] = { 1, 3 };
int             chanfdin[CHANNELS] = { 2, 4 };
int             chanskip[CHANNELS] = { 10, 20 };

char           *queuedesc;

#ifdef EXTERNAL_TODO
void            reread(int);
#else
void            reread();
#endif

#ifdef LOCK_LOGS
void
lock_logs_open(int preopen)
{
	char           *ptr;
	int             lock_status;

	if (!(ptr = env_get("LOCK_LOGS")))
		lock_status = 0;
	else
		scan_int(ptr, &lock_status);
	if (!lock_status && preopen)
		lock_status = preopen;
	if (lock_status > 0) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
		if (!stralloc_copys(&lockfn, controldir)
				|| !stralloc_append(&lockfn, "/")
				|| !stralloc_catb(&lockfn, "/defaultdelivery", 16)
				|| !stralloc_0(&lockfn))
			nomem();
		if ((loglock_fd = open_read(lockfn.s)) == -1) {
			log3("alert: ", queuedesc, ": cannot start: unable to open defaultdelivery\n");
			lockerr();
		}
	}
	log3("loglock: ", queuedesc, loglock_fd == -1 ? ": disabled\n" : ": enabled\n");
}
#endif

int             flagexitasap = 0;
void
sigterm()
{
	flagexitasap = 1;
}

int             flagrunasap = 0;
void
sigalrm()
{
	flagrunasap = 1;
}

int             flagreadasap = 0;
void
sighup()
{
	flagreadasap = 1;
}

void
chdir_toqueue()
{
	if (!queuedir) {
		if (!(queuedir = env_get("QUEUEDIR")))
			queuedir = "queue1";
	}
	while (chdir(queuedir) == -1) {
		log5("alert: ", queuedesc, ": unable to switch back to queue directory; HELP! sleeping...", error_str(errno), "\n");
		sleep(10);
	}
}

#ifdef LOCK_LOGS
void
sigint()
{
	if (loglock_fd == -1) {
		if (chdir(auto_qmail) == -1) {
			log5("alert: ", queuedesc, ": unable to reread controls: unable to switch to home directory", error_str(errno), "\n");
			return;
		}
		lock_logs_open(1);
		chdir_toqueue();
	} else {
		close(loglock_fd);
		loglock_fd = -1;
		log3("loglock: ", queuedesc, loglock_fd == -1 ? ": disabled\n" : ": enabled\n");
	}
}
#endif

void
cleandied()
{
	log3("alert: ", queuedesc, ": oh no! lost qmail-clean connection! dying...\n");
	flagexitasap = 1;
}

int             flagspawnalive[CHANNELS];
void
spawndied(int c)
{
	log3("alert: ", queuedesc, ": oh no! lost spawn connection! dying...\n");
	flagspawnalive[c] = 0;
	flagexitasap = 1;
}

#define REPORTMAX 10000

datetime_sec    recent;


/* this file is too long ----------------------------------------- FILENAMES */

stralloc        fn = { 0 };
stralloc        fn2 = { 0 };

char            fnmake_strnum[FMT_ULONG];

void
fnmake_init()
{
	while (!stralloc_ready(&fn, FMTQFN))
		nomem();
	while (!stralloc_ready(&fn2, FMTQFN))
		nomem();
}

void
fnmake_info(unsigned long id)
{
	fn.len = fmtqfn(fn.s, "info/", id, 1);
}

void
fnmake_todo(unsigned long id)
{
	fn.len = fmtqfn(fn.s, "todo/", id, 1);
}

void
fnmake_mess(unsigned long id)
{
	fn.len = fmtqfn(fn.s, "mess/", id, 1);
}

void
fnmake_foop(unsigned long id)
{
	fn.len = fmtqfn(fn.s, "foop/", id, 0);
}

void
fnmake_split(unsigned long id)
{
	fn.len = fmtqfn(fn.s, "", id, 1);
}

void
fnmake2_bounce(unsigned long id)
{
	fn2.len = fmtqfn(fn2.s, "bounce/", id, 0);
}

void
fnmake_chanaddr(unsigned long id, int c)
{
	fn.len = fmtqfn(fn.s, chanaddr[c], id, 1);
}


/* this file is too long ----------------------------------------- REWRITING */

stralloc        rwline = { 0 };

/* 1 if by land, 2 if by sea, 0 if out of memory. not allowed to barf. */
/* may trash recip. must set up rwline, between a T and a \0. */
int
rewrite(char *recip)
{
	unsigned int    i, j, at;
	char           *x;
	static stralloc addr = { 0 };

	if (!stralloc_copys(&rwline, "T")
			|| !stralloc_copys(&addr, recip))
		return 0;
	if ((i = byte_rchr(addr.s, addr.len, '@')) == addr.len) {
		if (!stralloc_cats(&addr, "@"))
			return 0;
		if (!stralloc_cat(&addr, &envnoathost))
			return 0;
	}
	while (constmap(&mappercenthack, addr.s + i + 1, addr.len - i - 1)) {
		j = byte_rchr(addr.s, i, '%');
		if (j == i)
			break;
		addr.len = i;
		i = j;
		addr.s[i] = '@';
	}
	at = byte_rchr(addr.s, addr.len, '@');
	if (constmap(&maplocals, addr.s + at + 1, addr.len - at - 1)) {
		if (!stralloc_cat(&rwline, &addr) || !stralloc_0(&rwline))
			return 0;
		return 1;
	}
	for (i = 0; i <= addr.len; ++i) {
		if (!i || (i == at + 1) || (i == addr.len) || ((i > at) && (addr.s[i] == '.'))) {
			if ((x = constmap(&mapvdoms, addr.s + i, addr.len - i))) {
				if (!*x)
					break;
				if (!stralloc_cats(&rwline, x) || !stralloc_cats(&rwline, "-")
						|| !stralloc_cat(&rwline, &addr) || !stralloc_0(&rwline))
					return 0;
				return 1;
			}
		}
	}
	if (!stralloc_cat(&rwline, &addr) || !stralloc_0(&rwline))
		return 0;
	return 2;
}

void
senderadd(stralloc *sa, char *sender, char *recip)
{
	unsigned int    i;

	if ((i = str_len(sender)) >= 4) {
		if (str_equal(sender + i - 4, "-@[]")) {
			unsigned int j = byte_rchr(sender, i - 4, '@');
			unsigned int k = str_rchr(recip, '@');
			if (recip[k] && (j + 5 <= i)) {
				/*- owner-@host-@[] -> owner-recipbox=reciphost@host */
				while (!stralloc_catb(sa, sender, j))
					nomem();
				while (!stralloc_catb(sa, recip, k))
					nomem();
				while (!stralloc_cats(sa, "="))
					nomem();
				while (!stralloc_cats(sa, recip + k + 1))
					nomem();
				while (!stralloc_cats(sa, "@"))
					nomem();
				while (!stralloc_catb(sa, sender + j + 1, i - 5 - j))
					nomem();
				return;
			}
		}
	}
	while (!stralloc_cats(sa, sender))
		nomem();
}


/* this file is too long ---------------------------------------------- INFO */

int
getinfo(stralloc *sa, stralloc *qh, stralloc *eh, datetime_sec *dt, unsigned long id)
{
	int             fdinfo, match;
	struct stat     st;
	static stralloc line = { 0 };
	substdio        ss;
	char            buf[128];

	fnmake_info(id);
	if ((fdinfo = open_read(fn.s)) == -1)
		return 0;
	if (fstat(fdinfo, &st) == -1) {
		close(fdinfo);
		return 0;
	}
	substdio_fdbuf(&ss, read, fdinfo, buf, sizeof (buf));
	sa->len = qh->len = eh->len = 0;
	for (;;) {
		if (getln(&ss, &line, &match, '\0') == -1) {
			close(fdinfo);
			return 0;
		}
		if (!match)
			break;
		if (line.s[0] == 'F')
			while (!stralloc_copys(sa, line.s + 1))
				nomem();
		if (line.s[0] == 'e')
			while (!stralloc_copys(qh, line.s + 1))
				nomem();
		if (line.s[0] == 'h')
			while (!stralloc_copys(eh, line.s + 1))
				nomem();
	}
	close(fdinfo);
	while (!stralloc_0(sa))
		nomem();
	while (!stralloc_0(qh))
		nomem();
	while (!stralloc_0(eh))
		nomem();
	*dt = st.st_mtime;
	return 1;
}


/* this file is too long ------------------------------------- COMMUNICATION */

substdio        sstoqc;
char            sstoqcbuf[1024];
substdio        ssfromqc;
char            ssfromqcbuf[1024];
stralloc        comm_buf[CHANNELS] = { {0}, {0} };
int             comm_pos[CHANNELS];

void
comm_init()
{
	int             c;

	substdio_fdbuf(&sstoqc, write, 5, sstoqcbuf, sizeof (sstoqcbuf));
	substdio_fdbuf(&ssfromqc, read, 6, ssfromqcbuf, sizeof (ssfromqcbuf));
	for (c = 0; c < CHANNELS; ++c) {
		/*- this is so stupid: NDELAY semantics should be default on write */
		if (ndelay_on(chanfdout[c]) == -1)
			spawndied(c); /*- drastic, but better than risking deadlock */
	}
}

int
comm_canwrite(int c)
{
	/*- XXX: could allow a bigger buffer; say 10 recipients */
	if (comm_buf[c].s && comm_buf[c].len)
		return 0;
	return 1;
}

void
comm_write(int c, int delnum, unsigned long id, char *sender, char *qqeh, char *envh, char *recip)
{
	char            ch;

	if (comm_buf[c].s && comm_buf[c].len)
		return;
	while (!stralloc_copys(&comm_buf[c], ""))
		nomem();
	ch = delnum;
	while (!stralloc_append(&comm_buf[c], &ch))
		nomem();
	ch = delnum >> 8;
	while (!stralloc_append(&comm_buf[c], &ch))
		nomem();
	fnmake_split(id);
	while (!stralloc_cats(&comm_buf[c], fn.s))
		nomem();
	while (!stralloc_0(&comm_buf[c]))
		nomem();
	senderadd(&comm_buf[c], sender, recip);
	while (!stralloc_0(&comm_buf[c]))
		nomem();
	while (!stralloc_cats(&comm_buf[c], qqeh))
		nomem();
	while (!stralloc_0(&comm_buf[c]))
		nomem();
	while (!stralloc_cats(&comm_buf[c], envh))
		nomem();
	while (!stralloc_0(&comm_buf[c]))
		nomem();
	while (!stralloc_cats(&comm_buf[c], recip))
		nomem();
	while (!stralloc_0(&comm_buf[c]))
		nomem();
	comm_pos[c] = 0;
}

void
comm_selprep(int *nfds, fd_set *wfds)
{
	int             c;

	for (c = 0; c < CHANNELS; ++c) {
		if (flagspawnalive[c] && comm_buf[c].s && comm_buf[c].len) {
			FD_SET(chanfdout[c], wfds);
			if (*nfds <= chanfdout[c])
				*nfds = chanfdout[c] + 1;
		}
	}
}

void
comm_do(fd_set *wfds)
{
	int             c;

	for (c = 0; c < CHANNELS; ++c) {
		if (flagspawnalive[c] && comm_buf[c].s && comm_buf[c].len && FD_ISSET(chanfdout[c], wfds)) {
			int             w;
			int             len;

			len = comm_buf[c].len;
			if ((w = write(chanfdout[c], comm_buf[c].s + comm_pos[c], len - comm_pos[c])) <= 0) {
				if ((w == -1) && (errno == error_pipe))
					spawndied(c);
				else
					continue;	/*- kernel select() bug; can't avoid busy-looping */
			} else {
				comm_pos[c] += w;
				if (comm_pos[c] == len)
					comm_buf[c].len = 0;
			}
		}
	}
}


/*- this file is too long ------------------------------------------ CLEANUPS */

int             flagcleanup;	/*- if 1, cleanupdir is initialized and ready */
readsubdir      cleanupdir;
datetime_sec    cleanuptime;

void
cleanup_init()
{
	flagcleanup = 0;
	cleanuptime = now();
}

void
cleanup_selprep(datetime_sec *wakeup)
{
	if (flagcleanup)
		*wakeup = 0;
	if (*wakeup > cleanuptime)
		*wakeup = cleanuptime;
}

void
cleanup_do()
{
	char            ch;
	struct stat     st;
	unsigned long   id;

	if (!flagcleanup) {
		if (recent < cleanuptime)
			return;
		readsubdir_init(&cleanupdir, "mess", pausedir);
		flagcleanup = 1;
	}
	switch (readsubdir_next(&cleanupdir, &id))
	{
	case 1:
		break;
	case 0:
		flagcleanup = 0;
		cleanuptime = recent + SLEEP_CLEANUP;
	default:
		return;
	}
	fnmake_mess(id);
	if (stat(fn.s, &st) == -1)
		return;	/*- probably qmail-queue deleted it */
	if (recent <= st.st_atime + OSSIFIED)
		return;
	fnmake_info(id);
	if (stat(fn.s, &st) == 0)
		return;
	if (errno != error_noent)
		return;
	fnmake_todo(id);
	if (stat(fn.s, &st) == 0)
		return;
	if (errno != error_noent)
		return;
	fnmake_foop(id);
	if (substdio_putflush(&sstoqc, fn.s, fn.len) == -1) {
		cleandied();
		return;
	}
	if (substdio_get(&ssfromqc, &ch, 1) != 1) {
		cleandied();
		return;
	}
	if (ch != '+')
		log5("warning: ", queuedesc, ": qmail-clean unable to clean up ", fn.s, "\n");
}


/*- this file is too long ----------------------------------- PRIORITY QUEUES */

prioq           pqdone = { 0 };						/*- -todo +info; HOPEFULLY -local -remote */
prioq           pqchan[CHANNELS] = { {0} , {0} };	/*- pqchan 0: -todo +info +local ?remote */
													/*- pqchan 1: -todo +info ?local +remote */
prioq           pqfail = { 0 };						/*- stat() failure; has to be pqadded again */

void
pqadd(unsigned long id)
{
	struct prioq_elt pe;
	struct prioq_elt pechan[CHANNELS];
	int             flagchan[CHANNELS];
	struct stat     st;
	int             c;

#define CHECKSTAT if (errno != error_noent) goto fail;

	fnmake_info(id);
	if (stat(fn.s, &st) == -1) {
		CHECKSTAT
		return; /*- someone yanking our chain */
	}
	fnmake_todo(id);
	if (stat(fn.s, &st) != -1)
		return;	/*- look, ma, dad crashed writing info!  */
	CHECKSTAT
	for (c = 0; c < CHANNELS; ++c) {
		fnmake_chanaddr(id, c);
		if (stat(fn.s, &st) == -1) {
			flagchan[c] = 0;
			CHECKSTAT
		} else {
			flagchan[c] = 1;
			pechan[c].id = id;
			pechan[c].dt = st.st_mtime;
		}
	}
	for (c = 0; c < CHANNELS; ++c)
		if (flagchan[c])
			while (!prioq_insert(&pqchan[c], &pechan[c]))
				nomem();
	for (c = 0; c < CHANNELS; ++c)
		if (flagchan[c])
			break;
	if (c == CHANNELS) {
		pe.id = id;
		pe.dt = now();
		while (!prioq_insert(&pqdone, &pe))
			nomem();
	}
	return;
fail:
	log5("warning: ", queuedesc, ": unable to stat ", fn.s, "; will try again later\n");
	pe.id = id;
	pe.dt = now() + SLEEP_SYSFAIL;
	while (!prioq_insert(&pqfail, &pe))
		nomem();
}

void
pqstart()
{
	readsubdir      rs;
	int             x;
	unsigned long   id;

	readsubdir_init(&rs, "info", pausedir);
	while ((x = readsubdir_next(&rs, &id))) {
		if (x > 0)
			pqadd(id);
	}
}

void
pqfinish()
{
	int             c;
	struct prioq_elt pe;
	struct timeval   ut[2] = {0};
	/*- XXX: more portable than utimbuf, but still worrisome */
	/*- time_t          ut[2]; -*/

	for (c = 0; c < CHANNELS; ++c) {
		while (prioq_min(&pqchan[c], &pe)) {
			prioq_delmin(&pqchan[c]);
			fnmake_chanaddr(pe.id, c);
			/*- ut[0] = ut[1] = pe.dt; -*/
			ut[0].tv_sec = ut[1].tv_sec = pe.dt;
			if (utimes(fn.s, ut) == -1)
				log5("warning: ", queuedesc, "unable to utime ", fn.s, "; message will be retried too soon\n");
		}
	}
}

void
pqrun()
{
	int             c, i;

	for (c = 0; c < CHANNELS; ++c) {
		if (pqchan[c].p && pqchan[c].len) {
			for (i = 0; i < pqchan[c].len; ++i)
				pqchan[c].p[i].dt = recent;
		}
	}
}


/*- this file is too long ---------------------------------------------- JOBS */

struct job {
	unsigned long   id;
	int             refs;		/*- if 0, this struct is unused */
	int             channel;
	int             numtodo;
	int             flaghiteof;
	int             flagdying;
	datetime_sec    retry;
	stralloc        sender, qqeh, envh;
};

int             numjobs;
struct job     *jo;

void
job_init()
{
	int             j;

	while (!(jo = (struct job *) alloc(numjobs * sizeof (struct job))))
		nomem();
	for (j = 0; j < numjobs; ++j) {
		jo[j].refs = 0;
		jo[j].sender.s = 0;
		jo[j].qqeh.s = 0;
		jo[j].envh.s = 0;
	}
}

int
job_avail()
{
	int             j;

	for (j = 0; j < numjobs; ++j) {
		if (!jo[j].refs)
			return 1;
	}
	return 0;
}

int
job_open(unsigned long id, int channel)
{
	int             j;

	for (j = 0; j < numjobs; ++j) {
		if (!jo[j].refs)
			break;
	}
	if (j == numjobs)
		return -1;
	jo[j].refs = 1;
	jo[j].id = id;
	jo[j].channel = channel;
	jo[j].numtodo = 0;
	jo[j].flaghiteof = 0;
	return j;
}

void
job_close(int j)
{
	struct prioq_elt pe;
	struct stat     st;

	if (0 < --jo[j].refs)
		return;
	pe.id = jo[j].id;
	pe.dt = jo[j].retry;
	if (jo[j].flaghiteof && !jo[j].numtodo) {
		fnmake_chanaddr(jo[j].id, jo[j].channel);
		if (unlink(fn.s) == -1) {
			log5("warning: ", queuedesc, ": unable to unlink ", fn.s, "; will try again later\n");
			pe.dt = now() + SLEEP_SYSFAIL;
		} else {
			int             c;
			for (c = 0; c < CHANNELS; ++c) {
				if (c != jo[j].channel) {
					fnmake_chanaddr(jo[j].id, c);
					if (stat(fn.s, &st) == 0)
						return;	/*- more channels going */
					if (errno != error_noent) {
						log5("warning: ", queuedesc, ": unable to stat ", fn.s, "\n");
						break;	/*- this is the only reason for HOPEFULLY */
					}
				}
			}
			pe.dt = now();
			while (!prioq_insert(&pqdone, &pe))
				nomem();
			return;
		}
	}
	while (!prioq_insert(&pqchan[jo[j].channel], &pe))
		nomem();
}


/*- this file is too long ------------------------------------------- BOUNCES */

/*- strip the virtual domain which is prepended to addresses e.g. xxx.com-user01@xxx.com */
char           *
stripvdomprepend(char *recip)
{
	unsigned int    i, domainlen;
	char           *domain, *prepend;

	i = str_rchr(recip, '@');
	if (!recip[i])
		return recip;
	domain = recip + i + 1;
	domainlen = str_len(domain);
	for (i = 0; i <= domainlen; ++i) {
		if ((i == 0) || (i == domainlen) || (domain[i] == '.')) {
			if ((prepend = constmap(&mapvdoms, domain + i, domainlen - i))) {
				if (!*prepend)
					break;
				i = str_len(prepend);
				if (str_diffn(recip, prepend, i))
					break;
				if (recip[i] != '-')
					break;
				return recip + i + 1;
			}
		}
	}
	return recip;
}

stralloc        bouncetext = { 0 };
stralloc        orig_recip = { 0 };

/*
 * prepare bounce txt with the following format
 * user@domain:\nbounce_report
 */
void
addbounce(unsigned long id, char *recip, char *report)
{
	int             fd, pos, w;

	while (!stralloc_copyb(&bouncetext, "<", 1))
		nomem();
	while (!stralloc_cats(&bouncetext, stripvdomprepend(recip)))
		nomem();
	for (pos = 0; pos < bouncetext.len; ++pos) {
		if (bouncetext.s[pos] == '\n')
			bouncetext.s[pos] = '_';
	}
	while (!stralloc_copy(&orig_recip, &bouncetext))
		nomem();
	while (!stralloc_catb(&orig_recip, ">\n", 2))
		nomem();
	while (!stralloc_catb(&bouncetext, ">:\n", 3))
		nomem();
	while (!stralloc_cats(&bouncetext, report))
		nomem();
	if (report[0] && report[str_len(report) - 1] != '\n') {
		while (!stralloc_append(&bouncetext, "\n"))
			nomem();
	}
	for (pos = bouncetext.len - 2; pos > 0; --pos) {
		if (bouncetext.s[pos] == '\n' && bouncetext.s[pos - 1] == '\n')
			bouncetext.s[pos] = '/';
	}
	while (!stralloc_append(&bouncetext, "\n"))
		nomem();
	fnmake2_bounce(id);
	for (;;) {
		if ((fd = open_append(fn2.s)) != -1)
			break;
		log3("alert: ", queuedesc, ": unable to append to bounce message; HELP! sleeping...\n");
		sleep(10);
	}
	pos = 0;
	while (pos < bouncetext.len) {
		if ((w = write(fd, bouncetext.s + pos, bouncetext.len - pos)) <= 0) {
			log3("alert: ", queuedesc, ": unable to append to bounce message; HELP! sleeping...\n");
			sleep(10);
		} else
			pos += w;
	}
	close(fd);
}

int
bounce_processor(struct qmail *qq, char *messfn, char *bouncefn, char *bounce_report, char *origrecip, char *sender,
				 char *recipient)
{
	char           *prog, *(args[8]);
	int             i, child, wstat;

	if (qq->flagerr)
		return (0);
	if (!(prog = env_get("BOUNCEPROCESSOR")))
		return (0);
	switch (child = fork())
	{
	case -1:
		log5("alert: ", queuedesc, ": Unable to fork: ", error_str(errno), "\n");
		return (111);
	case 0:
		args[0] = prog;
		args[1] = messfn; /*- message filename */
		args[2] = bouncefn;	/*- bounce message filename */
		args[3] = bounce_report; /*- bounce report */
		args[4] = sender; /*- bounce sender */
		args[5] = origrecip; /*- original recipient */
		args[6] = recipient; /*- original sender */
		args[7] = 0;
		execv(*args, args);
		log7("alert: ", queuedesc, ": Unable to run: ", prog, ": ", error_str(errno), "\n");
		_exit(111);
	}
	wait_pid(&wstat, child);
	if (wait_crashed(wstat)) {
		log7("alert: ", queuedesc, ": ", prog, " crashed: ", error_str(errno), "\n");
		return (111);
	}
	i = wait_exitcode(wstat);
	strnum2[fmt_ulong(strnum2, i)] = 0;
	log13("bounce processor sender <", sender, "> recipient <", recipient, "> messfn <", messfn, "> bouncefn <", bouncefn,
		  "> exit=", strnum2, " ", queuedesc, "\n");
	return (i);
}

int
injectbounce(unsigned long id)
{
	struct qmail    qqt;
	struct stat     st;
	static stralloc sender = { 0 };
	static stralloc qqeh = { 0 };
	static stralloc envh = { 0 };
	static stralloc quoted = { 0 };
	datetime_sec    birth;
	substdio        ssread;
	char            buf[128], inbuf[128];
	char           *bouncesender, *bouncerecip = "", *brep = "?", *p;
	int             r = -1, fd, ret;
	unsigned long   qp;
#ifdef MIME
	stralloc        boundary = { 0 };
#endif

	if (!getinfo(&sender, &qqeh, &envh, &birth, id))
		return 0; /*- XXX: print warning */
	/*- owner-@host-@[] -> owner-@host */
	if (sender.len >= 5 && str_equal(sender.s + sender.len - 5, "-@[]")) {
		sender.len -= 4;
		sender.s[sender.len - 1] = 0;
	}
#ifdef BATV
	/*- sb*-foo -> foo */
	if (sender.len >= 5 && !str_diffn(sender.s, "sb*-", 4)) {
		sender.len -= 4;
		byte_copy(sender.s, sender.len, sender.s + 4);
	}
#endif
	fnmake2_bounce(id);
	fnmake_mess(id);
	if (stat(fn2.s, &st) == -1) {
		if (errno == error_noent)
			return 1;
		log5("warning: ", queuedesc, ": unable to stat ", fn2.s, "\n");
		return 0;
	}
	if (str_equal(sender.s, "#@[]"))
		log5("triple bounce: discarding ", fn2.s, " ", queuedesc, "\n");
	else
	if (!*sender.s && *doublebounceto.s == '@')
		log5("double bounce: discarding ", fn2.s, " ", queuedesc, "\n");
	else {
		restore_env();
		if ((p = env_get("BOUNCEQUEUE")) && !env_put2("QMAILQUEUE", p)) {
			log3("alert: ", queuedesc, ": out of memory; will try again later\n");
			restore_env();
			return (0);
		}
		/*-
		 * Unset SPAMFILTER & FILTERARGS before calling qmail-queue
		 * and set it back after queue-queue returns
		 */
		if ((env_get("SPAMFILTER") && !env_unset("SPAMFILTER")) || (env_get("FILTERARGS") && !env_unset("FILTERARGS"))) {
			log3("alert: ", queuedesc, ": out of memory; will try again later\n");
			restore_env();
			return (0);
		}
		/*-
		 * Allow bounces to have different rules, queue, controls, etc
		 */
		if (chdir(auto_qmail) == -1) {
			log5("alert: ", queuedesc, ": unable to reread controls: unable to switch to home directory", error_str(errno), "\n");
			restore_env();
			return (0);
		}
		switch ((ret = envrules(sender.s, "bounce.envrules", "BOUNCERULES", 0)))
		{
		case AM_MEMORY_ERR:
			log3("alert: ", queuedesc, ": out of memory; will try again later\n");
			restore_env();
			chdir_toqueue();
			return (0);
			break;
		case AM_FILE_ERR:
			log3("alert: ", queuedesc, ": cannot start: unable to read bounce.envrules\n");
			restore_env();
			chdir_toqueue();
			return (0);
			break;
		case AM_REGEX_ERR:
			log3("alert: ", queuedesc, ": cannot start: regex compilation failed\n");
			restore_env();
			chdir_toqueue();
			return (0);
			break;
		case 0:
			chdir_toqueue();
			break;
		default:
			if (ret > 0)
#ifdef EXTERNAL_TODO
				reread(0); /*- this does chdir_toqueue() */
#else
				reread(); /*- this does chdir_toqueue() */
#endif
			else
			if (ret) {
				log3("alert: ", queuedesc, ": cannot start: envrules failed\n");
				restore_env();
				chdir_toqueue();
				return (0);
			} else
				chdir_toqueue();
			break;
		}
		if (qmail_open(&qqt) == -1) {
			log3("warning: ", queuedesc, ": unable to start qmail-queue, will try later\n");
			restore_env();
			return 0;
		}
		restore_env();
		qp = qmail_qp(&qqt);
#ifdef HAVESRS
		if (*sender.s) {
			if (srs_domain.len) {
				unsigned int    j = 0;

				j = byte_rchr(sender.s, sender.len, '@');
				if (j < sender.len) {
					if (srs_domain.len == sender.len - j - 1 && stralloc_starts(&srs_domain, sender.s + j + 1)) {
						switch (srsreverse(sender.s))
						{
						case -3:
							log5("srs: ", queuedesc, ": ", srs_error.s, "\n");
							_exit(111);
						case -2:
							nomem();
							break;
						case -1:
							log3("alert: ", queuedesc, ": unable to read controls\n");
							_exit(111);
						case 0:
							break;
						case 1:
							if (!stralloc_copy(&sender, &srs_result))
								nomem();
							break;
						}
						if (chdir(auto_qmail) == -1) {
							log3("alert: ", queuedesc, ": unable to switch to home directory\n");
							_exit(111);
						}
						chdir_toqueue();
					}
				}
			}
			bouncesender = "";
			bouncerecip = sender.s;
		} else
		if (*sender.s) {
			bouncesender = "";
			bouncerecip = sender.s;
		}
#else
		if (*sender.s) {
			bouncesender = "";
			bouncerecip = sender.s;
		} else {
			bouncesender = "#@[]";
			bouncerecip = doublebounceto.s;
		}
#endif
		while (!newfield_datemake(now()))
			nomem();
		qmail_put(&qqt, newfield_date.s, newfield_date.len);
		if (orig_recip.len) {
			qmail_put(&qqt, "X-Bounced-Address: ", 19);
			qmail_put(&qqt, orig_recip.s, orig_recip.len);
		}
		qmail_put(&qqt, "From: ", 6);
		while (!quote(&quoted, &bouncefrom))
			nomem();
		qmail_put(&qqt, quoted.s, quoted.len);
		qmail_puts(&qqt, "@");
		qmail_put(&qqt, bouncehost.s, bouncehost.len);
		qmail_puts(&qqt, "\nTo: ");
		while (!quote2(&quoted, bouncerecip))
			nomem();
		qmail_put(&qqt, quoted.s, quoted.len);
#ifdef MIME
		/*- MIME header with boundary */
		qmail_puts(&qqt, "\nMIME-Version: 1.0\n" "Content-Type: multipart/mixed; " "boundary=\"");
		if (!stralloc_copyb(&boundary, strnum2, fmt_ulong(strnum2, birth)))
			nomem();
		if (!stralloc_cat(&boundary, &bouncehost))
			nomem();
		if (!stralloc_catb(&boundary, strnum2, fmt_ulong(strnum2, id)))
			nomem();
		qmail_put(&qqt, boundary.s, boundary.len);
		qmail_puts(&qqt, "\"");
#endif
		qmail_puts(&qqt, "\nSubject: ");
		if (*sender.s)
			qmail_put(&qqt, bouncesubject.s, bouncesubject.len);
		else
			qmail_put(&qqt, doublebouncesubject.s, doublebouncesubject.len);
		qmail_puts(&qqt, "\n\n");
#ifdef MIME
		qmail_puts(&qqt, "--");
		qmail_put(&qqt, boundary.s, boundary.len); /*- def type is text/plain */
		qmail_puts(&qqt, "\n\n");
#endif
		if (*sender.s && bouncemessage.len) {
			qmail_put(&qqt, bouncemessage.s, bouncemessage.len);
			qmail_puts(&qqt, "\n");
		} else
		if (!*sender.s && doublebouncemessage.len) {
			qmail_put(&qqt, doublebouncemessage.s, doublebouncemessage.len);
			qmail_puts(&qqt, "\n");
		} else {
			qmail_puts(&qqt, "Hi. This is the qmail-send program at ");
			qmail_put(&qqt, bouncehost.s, bouncehost.len);
			qmail_puts(&qqt, *sender.s ? ".\n\
I'm afraid I wasn't able to deliver your message to the following addresses.\n\
This is a permanent error; I've given up. Sorry it didn't work out.\n\
\n\
" : ".\n\
I tried to deliver a bounce message to this address, but the bounce bounced!\n\
\n\
");
		}
		if ((fd = open_read(fn2.s)) == -1)
			qmail_fail(&qqt);
		else {
			while (!stralloc_copys(&orig_recip, ""))
				nomem();
			substdio_fdbuf(&ssread, read, fd, inbuf, sizeof (inbuf));
			while ((r = substdio_get(&ssread, buf, sizeof (buf))) > 0) {
				while (!stralloc_catb(&orig_recip, buf, r))
					nomem();
				qmail_put(&qqt, buf, r);
			}
			while (!stralloc_0(&orig_recip))
				nomem();
			/*- 
			 * orig_recip is of the form orig_recipient:\nbounce_recipient
			 * remove :\nbounce_report from orig_recip to get the original
			 * recipient
			 */
			for (brep = orig_recip.s; *brep != ':' && brep < orig_recip.s + orig_recip.len; brep++);
			if (*brep == ':') {
				*brep++ = 0;
				for (; *brep && isspace(*brep); brep++);
			}
			close(fd);
			if (r == -1)
				qmail_fail(&qqt);
		}
#ifdef MIME
		qmail_puts(&qqt,
			*sender.s ? "--- Enclosed is a copy of the message.\n\n--" : "--- Enclosed is the original bounce.\n\n--");
		qmail_put(&qqt, boundary.s, boundary.len);	/*- enclosure boundary */
		qmail_put(&qqt, "\nContent-Type: message/rfc822\n\n", 31);
#else
		qmail_puts(&qqt, 
			*sender.s ? "--- Below this line is a copy of the message.\n\n" : "--- Below this line is the original bounce.\n\n");
#endif
		qmail_put(&qqt, "Return-Path: <", 14);
		while (!quote2(&quoted, sender.s))
			nomem();
		qmail_put(&qqt, quoted.s, quoted.len);
		qmail_put(&qqt, ">\n", 2);
		if ((fd = open_read(fn.s)) == -1)
			qmail_fail(&qqt);
		else {
			int             bytestogo = bouncemaxbytes;
			int             bytestoget = (bytestogo < sizeof buf) ? bytestogo : sizeof buf;
			substdio_fdbuf(&ssread, read, fd, inbuf, sizeof (inbuf));
			while (bytestoget > 0 && (r = substdio_get(&ssread, buf, bytestoget)) > 0) {
				qmail_put(&qqt, buf, r);
				bytestogo -= bytestoget;
				bytestoget = (bytestogo < sizeof buf) ? bytestogo : sizeof buf;
			}
			if (r > 0)
				qmail_puts(&qqt, "\n\n--- End of message stripped.\n");
			close(fd);
			if (r == -1)
				qmail_fail(&qqt);
		}
#ifdef MIME
		qmail_puts(&qqt, "\n--");			/*- end boundary */
		qmail_put(&qqt, boundary.s, boundary.len);
		qmail_puts(&qqt, "--\n");
#endif
		/*- hook for external bounce processor */
		switch (bounce_processor(&qqt, fn.s, fn2.s, brep, orig_recip.s, bouncesender, bouncerecip))
		{
		case 0:
			break;
		case 1:/*- discard bounce */
			qmail_fail(&qqt);
			qmail_from(&qqt, bouncesender);
			qmail_to(&qqt, bouncerecip);
			qmail_close(&qqt);
			if (unlink(fn2.s) == -1) {
				log5("warning: ", queuedesc, ": unable to unlink ", fn2.s, ". Will try later\n");
				return 0;
			}
			log5("delete bounce: discarding ", fn2.s, " ", queuedesc, "\n");
			return 1;
		default:
			qmail_fail(&qqt);
			break;
		}
		qmail_from(&qqt, bouncesender);
		qmail_to(&qqt, bouncerecip);
		if (*qmail_close(&qqt)) {
			log3("warning: ", queuedesc, ": trouble injecting bounce message, will try later\n");
			return 0;
		}
		strnum2[fmt_ulong(strnum2, id)] = 0;
		log2_noflush("bounce msg ", strnum2);
		strnum2[fmt_ulong(strnum2, qp)] = 0;
		log5(" qp ", strnum2, " ", queuedesc, "\n");
	}
	if (unlink(fn2.s) == -1) {
		log5("warning: ", queuedesc, ": unable to unlink ", fn2.s, "\n");
		return 0;
	}
	return 1;
}


/*- this file is too long ---------------------------------------- DELIVERIES */

struct del {
	int             used;
	int             j;
	unsigned long   delid;
	seek_pos        mpos;
	stralloc        recip;
};

unsigned long   masterdelid = 1;
unsigned int    concurrency[CHANNELS] = { 10, 20 };
unsigned int    concurrencyused[CHANNELS] = { 0, 0 };
unsigned int    holdjobs[CHANNELS] = { 0, 0 };	/* Booleans: hold deliveries NJL 1998/05/03 */

struct del     *d[CHANNELS];
stralloc        concurrencyf = { 0 };

stralloc        dline[CHANNELS];
char            delbuf[2048];

void
del_status()
{
	int             c;

	log1_noflush("status:");
	for (c = 0; c < CHANNELS; ++c) {
		strnum2[fmt_ulong(strnum2, (unsigned long) concurrencyused[c])] = 0;
		strnum3[fmt_ulong(strnum3, (unsigned long) concurrency[c])] = 0;
		log2_noflush(chanstatusmsg[c], strnum2);
		log2_noflush("/", strnum3);
		if (holdjobs[c]) /*NJL*/
			log1_noflush(" (held)"); /*NJL*/
	}
	log2_noflush(" ", queuedesc);
	if (flagexitasap)
		log1_noflush(" exitasap");
	log1_noflush("\n");
	flush();
}

void
del_init()
{
	int             c, i;

	for (c = 0; c < CHANNELS; ++c) {
		flagspawnalive[c] = 1;
		while (!(d[c] = (struct del *) alloc(concurrency[c] * sizeof (struct del))))
			nomem();
		for (i = 0; i < concurrency[c]; ++i) {
			d[c][i].used = 0;
			d[c][i].recip.s = 0;
		}
		dline[c].s = 0;
		while (!stralloc_copys(&dline[c], ""))
			nomem();
	}
	del_status();
}

int
del_canexit()
{
	int             c;

	for (c = 0; c < CHANNELS; ++c) {
		if (flagspawnalive[c] && !holdjobs[c]) { /* if dead or held /NJL/, nothing we can do about its jobs */
			if (concurrencyused[c])
				return 0;
		}
	}
	return 1;
}

int
del_avail(int c)
{
	return flagspawnalive[c] && comm_canwrite(c) && !holdjobs[c] && (concurrencyused[c] < concurrency[c]);	/* NJL 1998/07/24 */
}

void
del_start(int j, seek_pos mpos, char *recip)
{
	int             i, c;

	c = jo[j].channel;
	if (!flagspawnalive[c])
		return;
	if (holdjobs[c])
		return;	/* NJL 1998/05/03 */
	if (!comm_canwrite(c))
		return;
	for (i = 0; i < concurrency[c]; ++i)
		if (!d[c][i].used)
			break;
	if (i == concurrency[c])
		return;
	if (!stralloc_copys(&d[c][i].recip, recip)) {
		nomem();
		return;
	}
	if (!stralloc_0(&d[c][i].recip)) {
		nomem();
		return;
	}
	d[c][i].j = j;
	++jo[j].refs;
	d[c][i].delid = masterdelid++;
	d[c][i].mpos = mpos;
	d[c][i].used = 1;
	++concurrencyused[c];
	comm_write(c, i, jo[j].id, jo[j].sender.s, jo[j].qqeh.s, jo[j].envh.s, recip);
	strnum2[fmt_ulong(strnum2, d[c][i].delid)] = 0;
	strnum3[fmt_ulong(strnum3, jo[j].id)] = 0;
	log2_noflush("starting delivery ", strnum2);
	log3_noflush(": msg ", strnum3, tochan[c]);
	logsafe_noflush(recip);
	log3(" ", queuedesc, "\n");
	del_status();
}

void
markdone(int c, unsigned long id, seek_pos pos)
{
	struct stat     st;
	int             fd;
	fnmake_chanaddr(id, c);
	for (;;) {
		if ((fd = open_write(fn.s)) == -1)
			break;
		if (fstat(fd, &st) == -1) {
			close(fd);
			break;
		}
		if (seek_set(fd, pos) == -1) {
			close(fd);
			break;
		}
		if (write(fd, "D", 1) != 1) {
			close(fd);
			break;
		}
		/*- further errors -> double delivery without us knowing about it, oh well */
		close(fd);
		return;
	}
	log5("warning: ", queuedesc, ": trouble marking ", fn.s, "; message will be delivered twice!\n");
}

void
del_dochan(int c)
{
	char            ch;
	int             r, i, delnum;

	if ((r = read(chanfdin[c], delbuf, sizeof (delbuf))) == -1)
		return;
	if (r == 0) {
		spawndied(c);
		return;
	}
	for (i = 0; i < r; ++i) {
		ch = delbuf[i];
		while (!stralloc_append(&dline[c], &ch))
			nomem();
		if (dline[c].len > REPORTMAX)
			dline[c].len = REPORTMAX;
		/*-
		 * qmail-lspawn and qmail-rspawn are responsible for keeping it short 
		 * but from a security point of view, we don't trust rspawn 
		 */
		if (!ch && (dline[c].len > 2)) {
			delnum = (unsigned int) (unsigned char) dline[c].s[0];
			delnum += (unsigned int) ((unsigned int) dline[c].s[1]) << 8;
			if ((delnum < 0) || (delnum >= concurrency[c]) || !d[c][delnum].used)
				log3("warning: ", queuedesc, ": internal error: delivery report out of range\n");
			else {
				strnum3[fmt_ulong(strnum3, d[c][delnum].delid)] = 0;
				if (dline[c].s[2] == 'Z') {
					if (jo[d[c][delnum].j].flagdying) {
						dline[c].s[2] = 'D';
						--dline[c].len;
						while (!stralloc_cats
							   (&dline[c], "I'm not going to try again; this message has been in the queue too long.\n"))
							nomem();
						while (!stralloc_0(&dline[c]))
							nomem();
					}
				}
				switch (dline[c].s[2])
				{
				case 'K':
					log3_noflush("delivery ", strnum3, ": success: ");
					logsafe_noflush(dline[c].s + 3);
					log3(" ", queuedesc, "\n");
					markdone(c, jo[d[c][delnum].j].id, d[c][delnum].mpos);
					--jo[d[c][delnum].j].numtodo;
					break;
				case 'Z':
					log3_noflush("delivery ", strnum3, ": deferral: ");
					logsafe_noflush(dline[c].s + 3);
					log3(" ", queuedesc, "\n");
					break;
				case 'D':
					log3_noflush("delivery ", strnum3, ": failure: ");
					logsafe_noflush(dline[c].s + 3);
					log3(" ", queuedesc, "\n");
					addbounce(jo[d[c][delnum].j].id, d[c][delnum].recip.s, dline[c].s + 3);
					markdone(c, jo[d[c][delnum].j].id, d[c][delnum].mpos);
					--jo[d[c][delnum].j].numtodo;
					break;
				default:
					log5("delivery ", strnum3, ": report mangled, will defer: ", queuedesc, "\n");
				}
				job_close(d[c][delnum].j);
				d[c][delnum].used = 0;
				--concurrencyused[c];
				del_status();
			}
			dline[c].len = 0;
		}
	}
}

void
del_selprep(int *nfds, fd_set *rfds)
{
	int             c;

	for (c = 0; c < CHANNELS; ++c) {
		if (flagspawnalive[c]) {
			FD_SET(chanfdin[c], rfds);
			if (*nfds <= chanfdin[c])
				*nfds = chanfdin[c] + 1;
		}
	}
}

void
del_do(fd_set *rfds)
{
	int             c;

	for (c = 0; c < CHANNELS; ++c) {
		if (flagspawnalive[c] && FD_ISSET(chanfdin[c], rfds))
			del_dochan(c);
	}
}


/*- this file is too long -------------------------------------------- PASSES */

struct {
	unsigned long   id;			/*- if 0, need a new pass */
	int             j;			/*- defined if id; job number */
	int             fd;			/*- defined if id; reading from {local,remote} */
	seek_pos        mpos;		/*- defined if id; mark position */
	substdio        ss;
	char            buf[128];
} pass[CHANNELS];

void
pass_init()
{
	int             c;

	for (c = 0; c < CHANNELS; ++c)
		pass[c].id = 0;
}

void
pass_selprep(datetime_sec *wakeup)
{
	int             c;
	struct prioq_elt pe;

	if (flagexitasap)
		return;
	for (c = 0; c < CHANNELS; ++c) {
		if (pass[c].id && del_avail(c)) {
			*wakeup = 0;
			return;
		}
	}
	if (job_avail()) {
		for (c = 0; c < CHANNELS; ++c) {
			if (!pass[c].id && prioq_min(&pqchan[c], &pe) && *wakeup > pe.dt)
				*wakeup = pe.dt;
		}
	}
	if (prioq_min(&pqfail, &pe) && *wakeup > pe.dt)
		*wakeup = pe.dt;
	if (prioq_min(&pqdone, &pe) && *wakeup > pe.dt)
		*wakeup = pe.dt;
}

/*- result^2 <= x < (result + 1)^2 */
static datetime_sec
squareroot(datetime_sec x) /* assuming: >= 0 */
{
	datetime_sec    y, yy, y21;
	int             j;

	y = 0;
	yy = 0;
	for (j = 15; j >= 0; --j) {
		y21 = (y << (j + 1)) + (1 << (j + j));
		if (y21 <= x - yy) {
			y += (1 << j);
			yy += y21;
		}
	}
	return y;
}

datetime_sec
nextretry(datetime_sec birth, int c)
{
	int             n;

	if (birth > recent)
		n = 0;
	else
		n = squareroot(recent - birth);	/*- no need to add fuzz to recent */
	n += chanskip[c];
	return birth + n * n;
}

void
pass_dochan(int c)
{
	datetime_sec    birth;
	struct prioq_elt pe;
	static stralloc line = { 0 };
	static stralloc qqeh = { 0 };
	static stralloc envh = { 0 };
	int             match;

	if (flagexitasap)
		return;
	if (!pass[c].id) {
		if (!job_avail())
			return;
		if (!prioq_min(&pqchan[c], &pe))
			return;
		if (pe.dt > recent)
			return;
		fnmake_chanaddr(pe.id, c);
		prioq_delmin(&pqchan[c]);
		pass[c].mpos = 0;
		if ((pass[c].fd = open_read(fn.s)) == -1)
			goto trouble;
		if (!getinfo(&line, &qqeh, &envh, &birth, pe.id)) {
			close(pass[c].fd);
			goto trouble;
		}
		pass[c].id = pe.id;
		substdio_fdbuf(&pass[c].ss, read, pass[c].fd, pass[c].buf, sizeof (pass[c].buf));
		pass[c].j = job_open(pe.id, c);
		jo[pass[c].j].retry = nextretry(birth, c);
		jo[pass[c].j].flagdying = (recent > birth + lifetime);
#ifdef BOUNCELIFETIME
		if (!*line.s)
			jo[pass[c].j].flagdying = (recent > birth + bouncelifetime);
#endif
		while (!stralloc_copy(&jo[pass[c].j].sender, &line))
			nomem();
		while (!stralloc_copy(&jo[pass[c].j].qqeh, &qqeh))
			nomem();
		while (!stralloc_copy(&jo[pass[c].j].envh, &envh))
			nomem();
	}
	if (!del_avail(c))
		return;
	if (getln(&pass[c].ss, &line, &match, '\0') == -1) {
		fnmake_chanaddr(pass[c].id, c);
		log5("warning: ", queuedesc, ": trouble reading ", fn.s, "; will try again later\n");
		close(pass[c].fd);
		job_close(pass[c].j);
		pass[c].id = 0;
		return;
	}
	if (!match) {
		close(pass[c].fd);
		jo[pass[c].j].flaghiteof = 1;
		job_close(pass[c].j);
		pass[c].id = 0;
		return;
	}
	switch (line.s[0])
	{
	case 'T':
		++jo[pass[c].j].numtodo;
		del_start(pass[c].j, pass[c].mpos, line.s + 1);
		break;
	case 'D':
		break;
	default:
		fnmake_chanaddr(pass[c].id, c);
		log5("warning: ", queuedesc, ": unknown record type in ", fn.s, "!\n");
		close(pass[c].fd);
		job_close(pass[c].j);
		pass[c].id = 0;
		return;
	}
	pass[c].mpos += line.len;
	return;
trouble:
	log5("warning: ", queuedesc, ": trouble opening ", fn.s, "; will try again later\n");
	pe.dt = recent + SLEEP_SYSFAIL;
	while (!prioq_insert(&pqchan[c], &pe))
		nomem();
}

void
messdone(unsigned long id)
{
	char            ch;
	int             c;
	struct prioq_elt pe;
	struct stat     st;

	for (c = 0; c < CHANNELS; ++c) {
		fnmake_chanaddr(id, c);
		if (stat(fn.s, &st) == 0)
			return;	/*- false alarm; consequence of HOPEFULLY */
		if (errno != error_noent) {
			log5("warning: ", queuedesc, ": unable to stat ", fn.s, "; will try again later\n");
			goto fail;
		}
	}
	fnmake_todo(id);
	if (stat(fn.s, &st) == 0)
		return;
	if (errno != error_noent) {
		log5("warning: ", queuedesc, ": unable to stat ", fn.s, "; will try again later\n");
		goto fail;
	}
	fnmake_info(id);
	if (stat(fn.s, &st) == -1) {
		if (errno == error_noent)
			return;
		log5("warning: ", queuedesc, ": unable to stat ", fn.s, "; will try again later\n");
		goto fail;
	}

	/*- -todo +info -local -remote ?bounce */
	if (!injectbounce(id))
		goto fail;	/*- injectbounce() produced error message */
	strnum3[fmt_ulong(strnum3, id)] = 0;
	log5("end msg ", strnum3, " ", queuedesc, "\n");

	/*- -todo +info -local -remote -bounce */
	fnmake_info(id);
	if (unlink(fn.s) == -1) {
		log5("warning: ", queuedesc, ": unable to unlink ", fn.s, "; will try again later\n");
		goto fail;
	}
	/*- -todo -info -local -remote -bounce; we can relax */
	fnmake_foop(id);
	if (substdio_putflush(&sstoqc, fn.s, fn.len) == -1) {
		cleandied();
		return;
	}
	if (substdio_get(&ssfromqc, &ch, 1) != 1) {
		cleandied();
		return;
	}
	if (ch != '+')
		log5("warning: ", queuedesc, ": qmail-clean unable to clean up ", fn.s, "\n");
	return;
fail:
	pe.id = id;
	pe.dt = now() + SLEEP_SYSFAIL;
	while (!prioq_insert(&pqdone, &pe))
		nomem();
}

void
pass_do()
{
	int             c;
	struct prioq_elt pe;

	for (c = 0; c < CHANNELS; ++c)
		pass_dochan(c);
	if (prioq_min(&pqfail, &pe)) {
		if (pe.dt <= recent) {
			prioq_delmin(&pqfail);
			pqadd(pe.id);
		}
	}
	if (prioq_min(&pqdone, &pe)) {
		if (pe.dt <= recent) {
			prioq_delmin(&pqdone);
			messdone(pe.id);
		}
	}
}


/*- this file is too long ---------------------------------------------- TODO */

#ifndef EXTERNAL_TODO
datetime_sec    nexttodorun, lasttodorun;
int             flagtododir = 0;	/*- if 0, have to readsubdir_init again */
int             todo_interval = -1;
readsubdir      todosubdir;
stralloc        todoline = { 0 };

char            todobuf[SUBSTDIO_INSIZE];
char            todobufinfo[512];
char            todobufchan[CHANNELS][1024];

void
todo_init()
{
	flagtododir = 0;
	lasttodorun = nexttodorun = now();
	trigger_set();
}

void
todo_selprep(int *nfds, fd_set *rfds, datetime_sec *wakeup)
{
	if (flagexitasap)
		return;
	trigger_selprep(nfds, rfds);
	if (flagtododir)
		*wakeup = 0;
	if (*wakeup > nexttodorun)
		*wakeup = nexttodorun;
}

unsigned long   Bytes;
stralloc        mailfrom = { 0 };
stralloc        mailto = { 0 };

char            strnum[FMT_ULONG];

void
log_stat(long bytes)
{
	char           *ptr;

	for (ptr = mailto.s; ptr < mailto.s + mailto.len;) {
		log9(*ptr == 'L' ? "local: " : "remote: ", mailfrom.len > 3 ? mailfrom.s + 1 : "<>", " ", *(ptr + 2) ? ptr + 2 : "<>", " ",
			 strnum, " ", queuedesc, "\n");
		ptr += str_len(ptr) + 1;
	}
	mailfrom.len = mailto.len = 0;
}

void
todo_do(fd_set *rfds)
{
	struct stat     st;
	struct prioq_elt pe;
	substdio        ss, ssinfo;
	substdio        sschan[CHANNELS];
	int             fd, fdinfo, match, c;
	int             fdchan[CHANNELS], flagchan[CHANNELS];
	char            ch;
	unsigned long   id, uid, pid;

	fd = -1;
	fdinfo = -1;
	for (c = 0; c < CHANNELS; ++c)
		fdchan[c] = -1;

	if (flagexitasap)
		return;
	/*- run todo maximal once every N seconds */
	if (todo_interval > 0 && recent < (lasttodorun + todo_interval)) {
		nexttodorun = lasttodorun + todo_interval;	/* do this to wake us up in N secs */
		return;	/* skip todo run this time */
	}
	if (!flagtododir) {
		if (!trigger_pulled(rfds) && recent < nexttodorun)
			return;
		trigger_set();
		readsubdir_init(&todosubdir, "todo", pausedir);
		flagtododir = 1;
		lasttodorun = recent;
		nexttodorun = recent + SLEEP_TODO;
	}
	switch (readsubdir_next(&todosubdir, &id))
	{
	case 1:
		break;
	case 0:
		flagtododir = 0;
	default:
		return;
	}
	fnmake_todo(id);
	if ((fd = open_read(fn.s)) == -1) {
		log3("warning: unable to open ", fn.s, "\n");
		return;
	}
	fnmake_mess(id);
	/*- just for the statistics */
	if (stat(fn.s, &st) == -1) {
		log3("warning: unable to stat ", fn.s, "\n");
		goto fail;
	}
	Bytes = st.st_size;
	for (c = 0; c < CHANNELS; ++c) {
		fnmake_chanaddr(id, c);
		if (unlink(fn.s) == -1) {
			if (errno != error_noent) {
				log3("warning: unable to unlink ", fn.s, "\n");
				goto fail;
			}
		}
	}
	fnmake_info(id);
	if (unlink(fn.s) == -1) {
		if (errno != error_noent) {
			log3("warning: unable to unlink ", fn.s, "\n");
			goto fail;
		}
	}
	if ((fdinfo = open_excl(fn.s)) == -1) {
		log3("warning: unable to create ", fn.s, "\n");
		goto fail;
	}
	strnum3[fmt_ulong(strnum3, id)] = 0;
	log3("new msg ", strnum3, "\n");
	for (c = 0; c < CHANNELS; ++c)
		flagchan[c] = 0;
	substdio_fdbuf(&ss, read, fd, todobuf, sizeof (todobuf));
	substdio_fdbuf(&ssinfo, write, fdinfo, todobufinfo, sizeof (todobufinfo));
	uid = 0;
	pid = 0;
	for (;;) {
		if (getln(&ss, &todoline, &match, '\0') == -1) {
			/*- perhaps we're out of memory, perhaps an I/O error */
			fnmake_todo(id);
			log3("warning: trouble reading ", fn.s, "\n");
			goto fail;
		}
		if (!match)
			break;
		switch (todoline.s[0])
		{
		case 'u':
			scan_ulong(todoline.s + 1, &uid);
			break;
		case 'p':
			scan_ulong(todoline.s + 1, &pid);
			break;
		case 'h':
		case 'e':
			if (substdio_put(&ssinfo, todoline.s, todoline.len) == -1) {
				fnmake_info(id);
				log3("warning: trouble writing to ", fn.s, "\n");
				goto fail;
			}
			break;
		case 'F':
			if (substdio_put(&ssinfo, todoline.s, todoline.len) == -1) {
				fnmake_info(id);
				log3("warning: trouble writing to ", fn.s, "\n");
				goto fail;
			}
			log2_noflush("info msg ", strnum3);
			strnum2[fmt_ulong(strnum2, (unsigned long) st.st_size)] = 0;
			log2_noflush(": bytes ", strnum2);
			log1_noflush(" from <");
			logsafe_noflush(todoline.s + 1);
			strnum2[fmt_ulong(strnum2, pid)] = 0;
			log2_noflush("> qp ", strnum2);
			strnum2[fmt_ulong(strnum2, uid)] = 0;
			log2_noflush(" uid ", strnum2);
			log3_noflush(" ", queuedesc, "\n");
			flush();
			if (!stralloc_copy(&mailfrom, &todoline) || !stralloc_0(&mailfrom)) {
				nomem();
				goto fail;
			}
			break;
		case 'T':
			switch (rewrite(todoline.s + 1))
			{
			case 0:
				nomem();
				goto fail;
			case 2: /*- Sea */
				if (!stralloc_cats(&mailto, "R") || !stralloc_cat(&mailto, &todoline)) {
					nomem();
					goto fail;
				}
				c = 1;
				break;
			default: /*- Land */
				if (!stralloc_cats(&mailto, "L") || !stralloc_cat(&mailto, &todoline)) {
					nomem();
					goto fail;
				}
				c = 0;
				break;
			}
			if (fdchan[c] == -1) {
				fnmake_chanaddr(id, c);
				fdchan[c] = open_excl(fn.s);
				if (fdchan[c] == -1) {
					log5("warning: ", queuedesc, ": unable to create ", fn.s, "\n");
					goto fail;
				}
				substdio_fdbuf(&sschan[c], write, fdchan[c], todobufchan[c], sizeof (todobufchan[c]));
				flagchan[c] = 1;
			}
			if (substdio_bput(&sschan[c], rwline.s, rwline.len) == -1) {
				fnmake_chanaddr(id, c);
				log5("warning: ", queuedesc, ": trouble writing to ", fn.s, "\n");
				goto fail;
			}
			break;
		default:
			fnmake_todo(id);
			log5("warning: ", queuedesc, ": unknown record type in ", fn.s, "\n");
			goto fail;
		}
	}
	close(fd);
	fd = -1;
	fnmake_info(id);
	if (substdio_flush(&ssinfo) == -1) {
		log5("warning: ", queuedesc, ": trouble writing to ", fn.s, "\n");
		goto fail;
	}
	for (c = 0; c < CHANNELS; ++c) {
		if (fdchan[c] != -1) {
			fnmake_chanaddr(id, c);
			if (substdio_flush(&sschan[c]) == -1) {
				log5("warning: ", queuedesc, ": trouble writing to ", fn.s, "\n");
				goto fail;
			}
		}
	}
#ifdef USE_FSYNC
	if (use_fsync > 0 && fsync(fdinfo) == -1) {
		log5("warning: ", queuedesc, ": trouble fsyncing ", fn.s, "\n");
		goto fail;
	}
#endif
	close(fdinfo);
	fdinfo = -1;
	for (c = 0; c < CHANNELS; ++c) {
		if (fdchan[c] != -1) {
#ifdef USE_FSYNC
			if (use_fsync > 0 && fsync(fdchan[c]) == -1) {
				log5("warning: ", queuedesc, ": trouble fsyncing ", fn.s, "\n");
				goto fail;
			}
#endif
			close(fdchan[c]);
			fdchan[c] = -1;
		}
	}
	fnmake_todo(id);
	if (substdio_putflush(&sstoqc, fn.s, fn.len) == -1) {
		cleandied();
		return;
	}
	if (substdio_get(&ssfromqc, &ch, 1) != 1) {
		cleandied();
		return;
	}
	if (ch != '+') {
		log5("warning: ", queuedesc, ": qmail-clean unable to clean up ", fn.s, "\n");
		return;
	}
	pe.id = id;
	pe.dt = now();
	for (c = 0; c < CHANNELS; ++c) {
		if (flagchan[c])
			while (!prioq_insert(&pqchan[c], &pe))
				nomem();
	}
	for (c = 0; c < CHANNELS; ++c) {
		if (flagchan[c])
			break;
	}
	if (c == CHANNELS) {
		while (!prioq_insert(&pqdone, &pe))
			nomem();
	}
	log_stat(Bytes);
	return;

fail:
	if (fd != -1)
		close(fd);
	if (fdinfo != -1)
		close(fdinfo);
	for (c = 0; c < CHANNELS; ++c) {
		if (fdchan[c] != -1)
			close(fdchan[c]);
	}
}
#endif /*- #ifndef EXTERNAL_TODO */

/*- this file is too long ------------------------------------- EXTERNAL TODO */

#ifdef EXTERNAL_TODO
stralloc        todoline = { 0 };

char            todobuf[2048];
int             todofdin, todofdout, flagtodoalive;

void
tododied()
{
	log3("alert: ", queuedesc, ": oh no! lost qmail-todo connection! dying...\n");
	flagexitasap = 1;
	flagtodoalive = 0;
}

void
todo_init()
{
	todofdout = 7;
	todofdin = 8;
	flagtodoalive = 1;
	/*- sync with external todo */
	if (write(todofdout, "S", 1) != 1) {
		log3("alert: unable to write a byte to external todo! dying...:", error_str(errno), "\n");
		flagexitasap = 1;
		flagtodoalive = 0;
	}
	return;
}

void
todo_selprep(int *nfds, fd_set *rfds, datetime_sec *wakeup)
{
	if (flagexitasap) {
		if (flagtodoalive && write(todofdout, "X", 1) != 1) {
			log5("alert: ", queuedesc, ": unable to write a byte to external todo! dying...:", error_str(errno), "\n");
			flagexitasap = 1;
			flagtodoalive = 0;
		}
	}
	if (flagtodoalive) {
		FD_SET(todofdin, rfds);
		if (*nfds <= todofdin)
			*nfds = todofdin + 1;
	}
}

void
todo_del(char *s)
{
	int             c;
	int             flagchan[CHANNELS];
	struct prioq_elt pe;
	unsigned long   id;
	unsigned int    len;

	for (c = 0; c < CHANNELS; ++c)
		flagchan[c] = 0;
	switch (*s++)
	{
	case 'L':
		flagchan[0] = 1;
		break;
	case 'R':
		flagchan[1] = 1;
		break;
	case 'B':
		flagchan[0] = 1;
		flagchan[1] = 1;
		break;
	case 'X':
		break;
	default:
		log3("warning: ", queuedesc, ": qmail-send unable to understand qmail-todo\n");
		return;
	}
	len = scan_ulong(s, &id);
	if (!len || s[len]) {
		log3("warning: ", queuedesc, ": qmail-send unable to understand qmail-todo\n");
		return;
	}
	pe.id = id;
	pe.dt = now();
	for (c = 0; c < CHANNELS; ++c) {
		if (flagchan[c])
			while (!prioq_insert(&pqchan[c], &pe))
				nomem();
	}
	for (c = 0; c < CHANNELS; ++c) {
		if (flagchan[c])
			break;
	}
	if (c == CHANNELS) {
		while (!prioq_insert(&pqdone, &pe))
			nomem();
	}
	return;
}

void
todo_do(fd_set *rfds)
{
	int             r, i;
	char            ch;

	if (!flagtodoalive)
		return;
	if (!FD_ISSET(todofdin, rfds))
		return;

	if ((r = read(todofdin, todobuf, sizeof (todobuf))) == -1)
		return;
	if (r == 0) {
		if (flagexitasap)
			flagtodoalive = 0;
		else
			tododied();
		return;
	}
	for (i = 0; i < r; ++i) {
		ch = todobuf[i];
		while (!stralloc_append(&todoline, &ch))
			nomem();
		if (todoline.len > REPORTMAX)
			todoline.len = REPORTMAX;
		/*- qmail-todo is responsible for keeping it short */
		if (!ch && (todoline.len > 1)) {
			switch (todoline.s[0])
			{
			case 'D':
				if (flagexitasap)
					break;
				todo_del(todoline.s + 1);
				break;
			case 'L':
				log1(todoline.s + 1);
				break;
			case 'X':
				if (flagexitasap)
					flagtodoalive = 0;
				else
					tododied();
				break;
			default:
				log3("warning: ", queuedesc, ": qmail-send unable to understand qmail-todo: report mangled\n");
				break;
			}
			todoline.len = 0;
		}
	}
}
#endif /*- #ifdef EXTERNAL_TODO */
/*- this file is too long ---------------------------------------------- MAIN */

int
getcontrols()
{
	char           *ptr;

	if (control_init() == -1)
		return 0;
	if (control_readint(&bouncemaxbytes, "bouncemaxbytes") == -1)
		return 0;
	if (control_readint((int *) &lifetime, "queuelifetime") == -1)
		return 0;
#ifdef BOUNCELIFETIME
	if (control_readint(&bouncelifetime, "bouncelifetime") == -1)
		return 0;
	if (bouncelifetime > lifetime)
		bouncelifetime = lifetime;
#endif
	/*- read concurrencylocal and concurrencylocal.queue[n] */
	if (control_readint((int *) &concurrency[0], "concurrencylocal") == -1)
		return 0;
	if (!stralloc_copys(&concurrencyf, "concurrencyl."))
		return 0;
	for (ptr = queuedir; *ptr; ptr++);
	for (; ptr != queuedir && *ptr != '/'; ptr--);
	if (!stralloc_cats(&concurrencyf, *ptr == '/' ? ptr + 1 : ptr))
		return 0;
	if (!stralloc_0(&concurrencyf))
		return 0;
	if (control_readint((int *) &concurrency[0], concurrencyf.s) == -1)
		return 0;

	/*- read concurrencyremote and concurrencyremote.queue[n] */
	if (control_readint((int *) &concurrency[1], "concurrencyremote") == -1)
		return 0;
	if (!stralloc_copys(&concurrencyf, "concurrencyr."))
		return 0;
	for (ptr = queuedir; *ptr; ptr++);
	for (; ptr != queuedir && *ptr != '/'; ptr--);
	if (!stralloc_cats(&concurrencyf, *ptr == '/' ? ptr + 1 : ptr))
		return 0;
	if (!stralloc_0(&concurrencyf))
		return 0;
	if (control_readint((int *) &concurrency[1], concurrencyf.s) == -1)
		return 0;

	if (control_readint((int *) &holdjobs[0], "holdlocal") == -1)
		return 0; /*NJL*/
	if (control_readint((int *) &holdjobs[1], "holdremote") == -1)
		return 0; /*NJL*/
	if (control_rldef(&envnoathost, "envnoathost", 1, "envnoathost") != 1)
		return 0;
#ifdef USE_FSYNC
	if (control_readint(&use_syncdir, "conf-syncdir") == -1)
		return 0;
	if (control_readint(&use_fsync, "conf-fsync") == -1)
		return 0;
	if (use_syncdir > 0) {
		if (!env_put2("USE_SYNCDIR", "1"))
			return 0;
	} else {
		if (!env_unset("USE_SYNCDIR"))
			return 0;
	}
	if (use_fsync > 0) {
		if (!env_put2("USE_FSYNC", "1"))
			return 0;
	} else {
		if (!env_unset("USE_FSYNC"))
			return 0;
	}
#endif
	if (control_rldef(&bouncefrom, "bouncefrom", 0, "MAILER-DAEMON") != 1)
		return 0;
	if (control_rldef(&bouncehost, "bouncehost", 1, "bouncehost") != 1)
		return 0;
	if (control_rldef(&doublebouncehost, "doublebouncehost", 1, "doublebouncehost") != 1)
		return 0;
#ifdef HAVESRS
	if (control_readline(&srs_domain, "srs_domain") == -1)
		return 0;
	if (srs_domain.len && !stralloc_0(&srs_domain))
		return 0;
#endif
	if (control_rldef(&doublebounceto, "doublebounceto", 0, "postmaster") != 1)
		return 0;
	if (!stralloc_cats(&doublebounceto, "@"))
		return 0;
	if (!stralloc_cat(&doublebounceto, &doublebouncehost))
		return 0;
	if (!stralloc_0(&doublebounceto))
		return 0;
	if (control_readfile(&locals, "locals", 1) != 1)
		return 0;
	if (control_readnativefile(&bouncemessage, "bouncemessage", 0) == -1)
		return 0;
	if (control_readnativefile(&doublebouncemessage, "doublebouncemessage", 0) == -1)
		return 0;
	if (control_rldef(&bouncesubject, "bouncesubject", 0, "failure notice") != 1)
		return 0;
	if (control_rldef(&doublebouncesubject, "doublebouncesubject", 0, "failure notice") != 1)
		return 0;
	if (!constmap_init(&maplocals, locals.s, locals.len, 0))
		return 0;
	switch (control_readfile(&percenthack, "percenthack", 0))
	{
	case -1:
		return 0;
	case 0:
		if (!constmap_init(&mappercenthack, "", 0, 0))
			return 0;
		break;
	case 1:
		if (!constmap_init(&mappercenthack, percenthack.s, percenthack.len, 0))
			return 0;
		break;
	}
	switch (control_readfile(&vdoms, "virtualdomains", 0))
	{
	case -1:
		return 0;
	case 0:
		if (!constmap_init(&mapvdoms, "", 0, 1))
			return 0;
		break;
	case 1:
		if (!constmap_init(&mapvdoms, vdoms.s, vdoms.len, 1))
			return 0;
		break;
	}
#ifndef EXTERNAL_TODO
	if (control_readint(&todo_interval, "todointerval") == -1)
		return 0;
#endif
	return 1;
}

/*
 * The reason for DJB using newlocals and newvdoms is so that
 * the original variables locals and vdoms do not get screwed
 * in regetcontrols. This will allow qmail-send to serve domains
 * already being served (inspite of memory problems during regetcontrols).
 * new domains added will not get served till another sighup causes
 * regetcontrols to get executed without problems. This is much
 * better than having qmail-send come to a grinding halt.
 * Another way could be to set flagexitasap to 1
 */
stralloc        newlocals = { 0 };
stralloc        newvdoms = { 0 };

void
regetcontrols()
{
	int             r;
	int             c; /*NJL*/
	int             newholdjobs[CHANNELS] = { 0, 0 }; /*NJL*/

	if (!controldir && !(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	if (control_readfile(&newlocals, "locals", 1) != 1) {
		log5("alert: ", queuedesc, ": unable to reread ", controldir, "/locals\n");
		return;
	}
	if ((r = control_readfile(&newvdoms, "virtualdomains", 0)) == -1) {
		log5("alert: ", queuedesc, ": unable to reread ", controldir, "/virtualdomains\n");
		return;
	}
#ifndef EXTERNAL_TODO
	if (control_readint(&todo_interval, "todointerval") == -1) {
		log5("alert: ", queuedesc, ": qmail-todo: unable to reread ", controldir, "/todointerval\n");
		return;
	}
#endif
	if (control_readint((int *) &concurrency[0], "concurrencylocal") == -1) {
		log5("alert: ", queuedesc, ": unable to reread ", controldir, "/concurrencylocal\n");
		return;
	}
	if (control_readint((int *) &concurrency[1], "concurrencyremote") == -1) {
		log5("alert: ", queuedesc, ": unable to reread ", controldir, "/concurrencyremote\n");
		return;
	}
	/*- Add "holdlocal/holdremote" flags - NJL 1998/05/03 */
	if (control_readint(&newholdjobs[0], "holdlocal") == -1) {
		log5("alert: ", queuedesc, ": unable to reread ", controldir, "/holdlocal\n");
		return;
	}
	if (control_readint(&newholdjobs[1], "holdremote") == -1) {
		log5("alert: ", queuedesc, ": unable to reread ", controldir, "/holdremote\n");
		return;
	}
	if (control_rldef(&envnoathost, "envnoathost", 1, "envnoathost") != 1) {
		log5("alert: ", queuedesc, ": unable to reread ", controldir, "/envnoathost\n");
		return;
	}
#ifdef USE_FSYNC
	if (control_readint(&use_syncdir, "conf-syncdir") == -1) {
		log5("alert: ", queuedesc, ": unable to reread ", controldir, "/conf-syncdir\n");
		return;
	}
	if (control_readint(&use_fsync, "conf-fsync") == -1) {
		log5("alert: ", queuedesc, ": unable to reread ", controldir, "/conf-fsync\n");
		return;
	}
	if (use_syncdir > 0) {
		while (!env_put2("USE_SYNCDIR", "1"))
			nomem();
	} else {
		while (!env_unset("USE_SYNCDIR"))
			nomem();
	}
	if (use_fsync > 0) {
		while (!env_put2("USE_FSYNC", "1"))
			nomem();
	} else {
		while (!env_unset("USE_FSYNC"))
			nomem();
	}
#endif
	for (c = 0; c < CHANNELS; c++) {
		if (holdjobs[c] != newholdjobs[c]) {
			holdjobs[c] = newholdjobs[c];
			if (holdjobs[c])
				log3(chanjobsheldmsg[c], " ", queuedesc);
			else {
				log3(chanjobsunheldmsg[c], " ", queuedesc);
				flagrunasap = 1;
			}
		}
	}
	constmap_free(&maplocals);
	constmap_free(&mapvdoms);
	while (!stralloc_copy(&locals, &newlocals))
		nomem();
	while (!constmap_init(&maplocals, locals.s, locals.len, 0))
		nomem();
	if (r) {
		while (!stralloc_copy(&vdoms, &newvdoms))
			nomem();
		while (!constmap_init(&mapvdoms, vdoms.s, vdoms.len, 1))
			nomem();
	} else
		while (!constmap_init(&mapvdoms, "", 0, 1))
			nomem();
}

#ifdef EXTERNAL_TODO
void
reread(int hupflag)
#else
void
reread()
#endif
{
	if (chdir(auto_qmail) == -1) {
		log5("alert: ", queuedesc, ": unable to reread controls: unable to switch to home directory", error_str(errno), "\n");
		return;
	}
#ifdef EXTERNAL_TODO
	if (hupflag && write(todofdout, "H", 1) != 1) {
		log5("alert: ", queuedesc, ": unable to write a byte to external todo:", error_str(errno), "\n");
		return;
	}
#endif
	regetcontrols();
	chdir_toqueue();
}

int
main()
{
	int             fd, nfds, c;
	datetime_sec    wakeup;
	fd_set          rfds, wfds;
	struct timeval  tv;
	/*- startup plugins */
	void           *handle;
	int             i, status = 0, len;
	int             (*func) (void);
	char            strnum[FMT_ULONG];
	char           *error, *start_plugin, *plugin_symb, *plugindir, *ptr, *plugin_ptr, *end;
	stralloc        plugin = { 0 }, splugin = {
	0};

	if (!(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue1";
	for (queuedesc = queuedir; *queuedesc; queuedesc++);
	for (; queuedesc != queuedir && *queuedesc != '/'; queuedesc--);
	if (*queuedesc == '/')
		queuedesc++;
	if (chdir(auto_qmail) == -1) {
		log5("alert: ", queuedesc, ": cannot start: unable to switch to home directory", error_str(errno), "\n");
		_exit(111);
	}
#ifdef LOCK_LOGS
	lock_logs_open(0);
#endif
#ifndef EXTERNAL_TODO
	if (!(ptr = env_get("TODO_INTERVAL")))
		todo_interval = -1;
	else
	if (!*ptr)
		todo_interval = ONCEEVERY;
	else {
		scan_int(ptr, &todo_interval);
		if (todo_interval <= 0)
			todo_interval = ONCEEVERY;
	}
#endif
	if (!(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue1";
	if (!getcontrols()) {
		log3("alert: ", queuedesc, ": cannot start: unable to read controls\n");
		_exit(111);
	}
	if (!(plugindir = env_get("PLUGINDIR")))
		plugindir = "plugins";
	if (plugindir[i = str_chr(plugindir, '/')]) {
		log3("alert: ", queuedesc, ": plugindir cannot have an absolute path\n");
		_exit(111);
	}
	if (!(plugin_symb = env_get("START_PLUGIN_SYMB")))
		plugin_symb = "startup";
	if (!(start_plugin = env_get("START_PLUGIN")))
		start_plugin = "qmail-send.so";
	if (!stralloc_copyb(&splugin, start_plugin, (len = str_len(start_plugin))))
		nomem();
	if (!stralloc_0(&splugin))
		nomem();
	end = splugin.s + len;
	for (ptr = plugin_ptr = splugin.s;; ptr++) {
		if (*ptr != ' ' && ptr != end)
			continue;
		if (ptr != end)
			*ptr = 0;
		if (!stralloc_copys(&plugin, auto_qmail))
			nomem();
		if (!stralloc_append(&plugin, "/"))
			nomem();
		if (!stralloc_cats(&plugin, plugindir))
			nomem();
		if (!stralloc_append(&plugin, "/"))
			nomem();
		if (!stralloc_cats(&plugin, plugin_ptr))
			nomem();
		if (!stralloc_0(&plugin))
			nomem();
		if (ptr != end)
			plugin_ptr = ptr + 1;
		if (access(plugin.s, F_OK)) {
			if (ptr == end)
				break;
			else
				continue;
		}
		if (!(handle = dlopen(plugin.s, RTLD_LAZY | RTLD_GLOBAL))) {
			log7("alert: ", queuedesc, ": dlopen failed for ", plugin.s, ": ", dlerror(), "\n");
			_exit(111);
		}
		dlerror(); /*- man page told me to do this */
		func = dlsym(handle, plugin_symb);
		if ((error = dlerror())) {
			log7("alert: ", queuedesc, ": dlsym ", plugin_symb, " failed: ", error, "\n");
			_exit(111);
		}
		log5("status: ", queuedesc, ": qmail-send executing function ", plugin_symb, "\n");
		if ((status = (*func) ())) {
			strnum[fmt_ulong(strnum, status)] = 0;
			log7("alert: ", queuedesc, ": function ", plugin_symb, " failed with status ", strnum, "\n");
		}
		if (dlclose(handle)) {
			log7("alert: ", queuedesc, ": dlclose for ", plugin.s, "failed: ", error, "\n");
			_exit(111);
		}
		if (ptr == end)
			break;
		if (status)
			break;
	}
	if (status)
		_exit(status);
	if (chdir(queuedir) == -1) {
		log5("alert: cannot start: unable to switch to queue directory: ", queuedesc, ":", error_str(errno), "\n");
		_exit(111);
	}
#if !defined(EXTERNAL_TOTO) && defined(USE_FSYNC)
	if (env_get("USE_FSYNC"))
		use_fsync = 1;
#endif
	sig_pipeignore();
	sig_termcatch(sigterm);
	sig_alarmcatch(sigalrm);
	sig_hangupcatch(sighup);
	sig_childdefault();
#ifdef LOCK_LOGS
	sig_intcatch(sigint);
#endif
	umask(077);
	if ((fd = open_write("lock/sendmutex")) == -1) {
		log3("alert: ", queuedesc, ": cannot start: unable to open mutex\n");
		_exit(111);
	}
	if (lock_exnb(fd) == -1) {
		log3("alert: ", queuedesc, ": cannot start: qmail-send is already running\n");
		_exit(111);
	}
	numjobs = 0;
	for (c = 0; c < CHANNELS; ++c) {
		char            ch1, ch2;
		int             u;
		int             r;

		do {
			r = read(chanfdin[c], &ch1, 1);
		} while ((r == -1) && (errno == error_intr));
		if (r < 1) {
			log3("alert: ", queuedesc, ": cannot start: hath the daemon spawn no fire?\n");
			_exit(111);
		}
		do {
			r = read(chanfdin[c], &ch2, 1);
		} while ((r == -1) && (errno == error_intr));
		if (r < 1) {
			log3("alert: ", queuedesc, ": cannot start: hath the daemon spawn no fire?\n");
			_exit(111);
		}
		u = (unsigned int) (unsigned char) ch1;
		u += (unsigned int) ((unsigned char) ch2) << 8;
		if (concurrency[c] > u)
			concurrency[c] = u;
		numjobs += concurrency[c];
	}
	fnmake_init();
	comm_init();
	pqstart();
	job_init();
	del_init();
	pass_init();
	todo_init();
	cleanup_init();
#ifdef EXTERNAL_TODO
	while (!flagexitasap || !del_canexit() || flagtodoalive)
#else
	while (!flagexitasap || !del_canexit())
#endif
	{
		recent = now();
		if (flagrunasap) {
			flagrunasap = 0;
			pqrun();
		}
		if (flagreadasap) {
			flagreadasap = 0;
#ifdef EXTERNAL_TODO
			reread(1);
#else
			reread();
#endif
		}
		wakeup = recent + SLEEP_FOREVER;
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		nfds = 1;
		comm_selprep(&nfds, &wfds);
		del_selprep(&nfds, &rfds);
		pass_selprep(&wakeup);
		todo_selprep(&nfds, &rfds, &wakeup);
		cleanup_selprep(&wakeup);
		if (wakeup <= recent)
			tv.tv_sec = 0;
		else
			tv.tv_sec = wakeup - recent + SLEEP_FUZZ;
		tv.tv_usec = 0;
		if (select(nfds, &rfds, &wfds, (fd_set *) 0, &tv) == -1) {
			if (errno == error_intr);
			else
				log3("warning: ", queuedesc, ": trouble in select\n");
		} else {
			recent = now();
			comm_do(&wfds);
			del_do(&rfds);
			todo_do(&rfds);
			pass_do();
			cleanup_do();
		}
	} /*- while (!flagexitasap || !del_canexit() || flagtodoalive) */
	pqfinish();
	log5("status: ", queuedesc, " ", queuedesc, " qmail-send exiting\n");
	return (0);
}

void
getversion_qmail_send_c()
{
	static char    *x = "$Id: qmail-send.c,v 1.70 2020-09-15 21:09:07+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}
