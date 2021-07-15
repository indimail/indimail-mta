/*
 * $Log: slowq-send.c,v $
 * Revision 1.11  2021-07-15 22:37:20+05:30  Cprogrammer
 * corrected data type of comm_pos to int
 *
 * Revision 1.10  2021-07-15 13:23:32+05:30  Cprogrammer
 * organize bounce related control files together
 *
 * Revision 1.9  2021-06-27 10:45:38+05:30  Cprogrammer
 * moved conf_split variable to fmtqfn.c
 * fixed error handling in injectbounce
 *
 * Revision 1.8  2021-06-23 13:27:26+05:30  Cprogrammer
 * moved log_stat to qsutil.c
 *
 * Revision 1.7  2021-06-05 22:42:38+05:30  Cprogrammer
 * added code comments
 *
 * Revision 1.6  2021-06-05 18:02:19+05:30  Cprogrammer
 * corrected log message
 *
 * Revision 1.5  2021-06-05 17:46:32+05:30  Cprogrammer
 * reduce wakeup when time_needed is earlier than wakeup
 *
 * Revision 1.4  2021-06-05 12:58:45+05:30  Cprogrammer
 * refactored rate limit code to display delayed job count in logs
 *
 * Revision 1.3  2021-06-03 18:08:21+05:30  Cprogrammer
 * use new prioq functions
 *
 * Revision 1.2  2021-06-01 01:52:35+05:30  Cprogrammer
 * moved delivery_report to delivery_report.c
 *
 * Revision 1.1  2021-05-31 17:06:24+05:30  Cprogrammer
 * Initial revision
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
#include "env.h"
#include "envrules.h"
#include "variables.h"
#include "readsubdir.h"
#include "hassrs.h"
#include "getEnvConfig.h"
#include "auto_split.h"
#ifdef HAVESRS
#include "srs.h"
#endif
#include "wait.h"
#ifdef USE_FSYNC
#include "syncdir.h"
#endif
#include "delivery_rate.h"

/*- critical timing feature #1: if not triggered, do not busy-loop */
/*- critical timing feature #2: if triggered, respond within fixed time */
/*- important timing feature: when triggered, respond instantly */
#define SLEEP_FUZZ         1 /*- slop a bit on sleeps to avoid zeno effect */
#define SLEEP_FOREVER  86400 /*- absolute maximum time spent in select() */
#define SLEEP_CLEANUP  76431 /*- time between cleanups */
#define SLEEP_SYSFAIL    123
#define OSSIFIED      129600 /*- 36 hours; _must_ exceed q-q's DEATH (24 hours) */
#ifndef TODO_INTERVAL
#define SLEEP_TODO      1500 /*- check todo/ every 25 minutes in any case */
#define ONCEEVERY         10 /*- Run todo maximal once every N seconds */
#endif

static int      lifetime = 604800;
static int      bouncemaxbytes = 50000;
#ifdef BOUNCELIFETIME
static int      bouncelifetime = 604800;
#endif
static stralloc percenthack = { 0 };
static stralloc locals = { 0 };
static stralloc bouncemessage = { 0 };
static stralloc bouncesubject = { 0 };
static stralloc doublebouncemessage = { 0 };
static stralloc doublebouncesubject = { 0 };
#ifdef HAVESRS
static stralloc srs_domain = { 0 };
#endif
static stralloc vdoms = { 0 };
typedef struct constmap cmap;
static cmap     mapvdoms;
static stralloc envnoathost = { 0 };
static stralloc bouncefrom = { 0 };
static stralloc bouncehost = { 0 };
static stralloc doublebounceto = { 0 };
static stralloc doublebouncehost = { 0 };
static cmap     mappercenthack;
static cmap     maplocals;

static char     strnum1[FMT_ULONG];
static char     strnum2[FMT_ULONG];

#define CHANNELS 2
static char    *chanaddr[CHANNELS] = { "local/", "remote/" };
static char    *chanstatusmsg[CHANNELS] = { " local ", " remote " };
static char    *chanjobsheldmsg[CHANNELS] = { /* NJL 1998/05/03 */
	"local deliveries temporarily held\n",
	"remote deliveries temporarily held\n"
};
static char    *chanjobsunheldmsg[CHANNELS] = {	/* NJL 1998/05/03 */
	"local deliveries resumed\n",
	"remote deliveries resumed\n"
};
static char    *tochan[CHANNELS] = { " to local ", " to remote " };
static int      chanfdout[CHANNELS] = { 1, 3 };
static int      chanfdin[CHANNELS] = { 2, 4 };
static int      chanskip[CHANNELS] = { 10, 20 };

char           *queuedesc;

static int      flagexitasap = 0;
static int      flagrunasap = 0;
static int      flagreadasap = 0;

dtype           delivery;
int             do_ratelimit;
unsigned long   delayed_jobs;

static void     reread();

static void
sigterm()
{
	flagexitasap = 1;
}

static void
sigalrm()
{
	flagrunasap = 1;
}

static void
sighup()
{
	flagreadasap = 1;
}

static void
chdir_toqueue()
{
	if (!queuedir && !(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue/slowq";
	while (chdir(queuedir) == -1) {
		log5("alert: ", queuedesc, ": unable to switch back to queue directory; HELP! sleeping...", error_str(errno), "\n");
		sleep(10);
	}
}

static void
cleandied()
{
	log3("alert: ", queuedesc, ": oh no! lost qmail-clean connection! dying...\n");
	flagexitasap = 1;
}

int             flagspawnalive[CHANNELS];
static void
spawndied(int c)
{
	log3("alert: ", queuedesc, ": oh no! lost spawn connection! dying...\n");
	flagspawnalive[c] = 0;
	flagexitasap = 1;
}

#define REPORTMAX 10000

datetime_sec    recent, time_needed;


/* this file is too long ----------------------------------------- FILENAMES */
static stralloc fn1 = { 0 };
static stralloc fn2 = { 0 };

static void
fnmake_init()
{
	while (!stralloc_ready(&fn1, FMTQFN))
		nomem();
	while (!stralloc_ready(&fn2, FMTQFN))
		nomem();
}

static void
fnmake_info(unsigned long id)
{
	fn1.len = fmtqfn(fn1.s, "info/", id, 1);
}

static void
fnmake_todo(unsigned long id)
{
	fn1.len = fmtqfn(fn1.s, "todo/", id, 1);
}

static void
fnmake_mess(unsigned long id)
{
	fn1.len = fmtqfn(fn1.s, "mess/", id, 1);
}

static void
fnmake_foop(unsigned long id)
{
	fn1.len = fmtqfn(fn1.s, "foop/", id, 0);
}

static void
fnmake_split(unsigned long id)
{
	fn1.len = fmtqfn(fn1.s, "", id, 1);
}

static void
fnmake2_bounce(unsigned long id)
{
	fn2.len = fmtqfn(fn2.s, "bounce/", id, 0);
}

static void
fnmake_chanaddr(unsigned long id, int c)
{
	fn1.len = fmtqfn(fn1.s, chanaddr[c], id, 1);
}


/* this file is too long ----------------------------------------- REWRITING */

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

	fnmake_info(id); /* info/split/filename */
	if ((fdinfo = open_read(fn1.s)) == -1)
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
		if (line.s[0] == 'F') /*- from */
			while (!stralloc_copys(sa, line.s + 1))
				nomem();
		if (line.s[0] == 'e') /*- qqeh */
			while (!stralloc_copys(qh, line.s + 1))
				nomem();
		if (line.s[0] == 'h') /*- envheader */
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

static substdio sstoqc;
static substdio ssfromqc;
static char     sstoqcbuf[1024];
static char     ssfromqcbuf[1024];
static stralloc comm_buf[CHANNELS] = { {0}, {0} };
static int      comm_pos[CHANNELS];

static void
comm_init()
{
	int             c;

	/* fd 5 is pi5[1] - write, fd 6 is pi6[0] - read*/
	substdio_fdbuf(&sstoqc, write, 5, sstoqcbuf, sizeof (sstoqcbuf));
	substdio_fdbuf(&ssfromqc, read, 6, ssfromqcbuf, sizeof (ssfromqcbuf));
	for (c = 0; c < CHANNELS; ++c) {
		/*- this is so stupid: NDELAY semantics should be default on write */
		if (ndelay_on(chanfdout[c]) == -1)
			spawndied(c); /*- drastic, but better than risking deadlock */
	}
}

static int
comm_canwrite(int c)
{
	/*- XXX: could allow a bigger buffer; say 10 recipients */
	if (comm_buf[c].s && comm_buf[c].len) /*- data pending to be written to qmail-[l|r]spawn */
		return 0;
	return 1;
}

/*-
 * "\0\00015/id\0recipient@domain\0qqeh\0envh\0sender@domain\0"
 * NULL
 * delivery number
 * id
 * recipient
 * qq extra header
 * env header
 * recipient
 */
static void
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
	while (!stralloc_cats(&comm_buf[c], fn1.s))
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

static void
comm_selprep(int *nfds, fd_set *wfds)
{
	int             c;

	for (c = 0; c < CHANNELS; ++c) {
		if (flagspawnalive[c] && comm_buf[c].s && comm_buf[c].len) {
			FD_SET(chanfdout[c], wfds); /*- fd 1 - pi1[1] - write, and fd 3 - pi3[1] - write */
			if (*nfds <= chanfdout[c])
				*nfds = chanfdout[c] + 1;
		}
	}
}

/*-
 * write to qmail-lspawn, qmail-rspawn
 * set comm_buff[c].len = 0 when data is written
 */
static void
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

static int      flagcleanup;	/*- if 1, cleanupdir is initialized and ready */
static readsubdir cleanupdir;
static datetime_sec cleanuptime;

static void
cleanup_init()
{
	flagcleanup = 0;
	cleanuptime = now();
}

static void
cleanup_selprep(datetime_sec *wakeup)
{
	if (flagcleanup)
		*wakeup = 0;
	if (*wakeup > cleanuptime)
		*wakeup = cleanuptime;
}

static void
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
	case 0: /*- no files found */
		flagcleanup = 0;
		cleanuptime = recent + SLEEP_CLEANUP;
	default:
		return;
	}
	fnmake_mess(id);
	if (stat(fn1.s, &st) == -1)
		return;	/*- probably qmail-queue deleted it */
	if (recent <= st.st_atime + OSSIFIED)
		return;
	fnmake_info(id);
	if (stat(fn1.s, &st) == 0)
		return;
	if (errno != error_noent)
		return;
	fnmake_todo(id);
	if (stat(fn1.s, &st) == 0)
		return;
	if (errno != error_noent)
		return;
	fnmake_foop(id);
	if (substdio_putflush(&sstoqc, fn1.s, fn1.len) == -1) {
		cleandied();
		return;
	}
	if (substdio_get(&ssfromqc, &ch, 1) != 1) {
		cleandied();
		return;
	}
	if (ch != '+')
		log5("warning: ", queuedesc, ": qmail-clean unable to clean up ", fn1.s, "\n");
}


/*- this file is too long ----------------------------------- PRIORITY QUEUES */

static prioq    pqdone = { 0 };						/*- -todo +info; HOPEFULLY -local -remote */
static prioq    pqchan[CHANNELS] = { {0} , {0} };	/*- pqchan 0: -todo +info +local ?remote */
													/*- pqchan 1: -todo +info ?local +remote */
static prioq    pqfail = { 0 };						/*- stat() failure; has to be pqadded again */

static void
pqadd(unsigned long id, char delayedflag)
{
	struct prioq_elt pe;
	struct prioq_elt pechan[CHANNELS];
	int             flagchan[CHANNELS];
	struct stat     st;
	int             c;

#define CHECKSTAT if (errno != error_noent) goto fail;

	fnmake_info(id);
	if (stat(fn1.s, &st) == -1) {
		CHECKSTAT
		return; /*- someone yanking our chain */
	}
	fnmake_todo(id);
	if (stat(fn1.s, &st) != -1)
		return;	/*- look, ma, dad crashed writing info!  */
	CHECKSTAT
	for (c = 0; c < CHANNELS; ++c) {
		fnmake_chanaddr(id, c);
		if (stat(fn1.s, &st) == -1) {
			flagchan[c] = 0;
			CHECKSTAT
		} else {
			flagchan[c] = 1;
			pechan[c].id = id;
			if (!recent)
				recent = now();
			pechan[c].dt = delayedflag ? recent : st.st_mtime;
			pechan[c].delayed = delayedflag;
		}
	}
	for (c = 0; c < CHANNELS; ++c) {
		if (flagchan[c]) {
			while (!prioq_insert(min, &pqchan[c], &pechan[c]))
				nomem();
		}
	}
	for (c = 0; c < CHANNELS; ++c) {
		if (flagchan[c])
			break;
	}
	if (c == CHANNELS) {
		pe.id = id;
		pe.dt = now();
		while (!prioq_insert(min, &pqdone, &pe))
			nomem();
	}
	return;
fail:
	log5("warning: ", queuedesc, ": unable to stat ", fn1.s, "; will try again later\n");
	pe.id = id;
	pe.dt = now() + SLEEP_SYSFAIL;
	while (!prioq_insert(min, &pqfail, &pe))
		nomem();
}

unsigned long
delayed_job_count()
{
	unsigned long   c, i, n;

	for (c = n = 0; c < CHANNELS; ++c) {
		for (i = 0; i < pqchan[c].len; i++) {
			if (pqchan[c].p[i].delayed)
				n++;
		}
	}
	return n;
}

static void
pqstart()
{
	readsubdir      rs;
	int             x;
	unsigned long   id;

	readsubdir_init(&rs, "info", pausedir); /*- pausedir is a function in qsutil */
	while ((x = readsubdir_next(&rs, &id))) { /*- here id is the filename too */
		if (x > 0)
			pqadd(id, do_ratelimit);
	}
	delayed_jobs = do_ratelimit ? delayed_job_count() : 0;
}

static void
pqfinish()
{
	int             c;
	struct prioq_elt pe;
	struct timeval   ut[2] = {{0}};

	for (c = 0; c < CHANNELS; ++c) {
		while (prioq_get(&pqchan[c], &pe)) {
			prioq_del(&pqchan[c]);
			fnmake_chanaddr(pe.id, c);
			ut[0].tv_sec = ut[1].tv_sec = pe.dt;
			if (utimes(fn1.s, ut) == -1)
				log5("warning: ", queuedesc, "unable to utime ", fn1.s, "; message will be retried too soon\n");
		}
	}
}

static void
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

typedef struct job {
	unsigned long   id;
	int             refs;		/*- if 0, this struct is unused */
	int             channel;
	int             numtodo;
	int             flaghiteof;
	int             flagdying;
	datetime_sec    retry;
	stralloc        sender, qqeh, envh;
} job;

static int      numjobs;
static job     *jo;

static void
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

static int
job_avail()
{
	int             j;

    /*- Rolf Eike Beer */
	for (j = 0; j < numjobs; ++j) {
		if (!jo[j].refs)
			return j;
	}
	return -1;
}

static void
job_open(unsigned long id, int channel, int j)
{
    /*- Rolf Eike Beer */
	jo[j].refs = 1;
	jo[j].id = id;
	jo[j].channel = channel;
	jo[j].numtodo = 0; /*- prevent closing job before delivery complets */
	jo[j].flaghiteof = 0; /*- prevent closing before message is read */
	return;
}

static void
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
		if (unlink(fn1.s) == -1) {
			log5("warning: ", queuedesc, ": unable to unlink ", fn1.s, "; will try again later\n");
			pe.dt = now() + SLEEP_SYSFAIL;
		} else {
			int             c;
			for (c = 0; c < CHANNELS; ++c) {
				if (c != jo[j].channel) {
					fnmake_chanaddr(jo[j].id, c);
					if (stat(fn1.s, &st) == 0)
						return;	/*- more channels going */
					if (errno != error_noent) {
						log5("warning: ", queuedesc, ": unable to stat ", fn1.s, "\n");
						break;	/*- this is the only reason for HOPEFULLY */
					}
				}
			}
			pe.dt = now();
			while (!prioq_insert(min, &pqdone, &pe))
				nomem();
			return;
		}
	}
	pe.delayed = do_ratelimit ? 1 : 0;
	while (!prioq_insert(min, &pqchan[jo[j].channel], &pe))
		nomem();
}

/*- this file is too long ------------------------------------------- BOUNCES */

/*- strip the virtual domain which is prepended to addresses e.g. xxx.com-user01@xxx.com */
static char    *
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

static stralloc bouncetext = { 0 };
static stralloc orig_recip = { 0 };

/*
 * prepare bounce txt with the following format
 * user@domain:\nbounce_report
 */
static void
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

static int
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
	strnum1[fmt_ulong(strnum1, i)] = 0;
	log13("bounce processor sender <", sender, "> recipient <", recipient, "> messfn <", messfn, "> bouncefn <", bouncefn,
		  "> exit=", strnum1, " ", queuedesc, "\n");
	return (i);
}

static int
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
			log7("alert: ", queuedesc, ": unable to reread controls: unable to switch to ", auto_qmail, ": ", error_str(errno), "\n");
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
				reread(); /*- this does chdir_toqueue() */
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
							qmail_fail(&qqt);
							break;
						case -2:
							nomem();
							qmail_fail(&qqt);
							break;
						case -1:
							log3("alert: ", queuedesc, ": unable to read controls\n");
							qmail_fail(&qqt);
							break;
						case 0:
							break;
						case 1:
							while (!stralloc_copy(&sender, &srs_result))
								nomem();
							break;
						}
						while (chdir(auto_qmail) == -1) {
							log7("alert: ", queuedesc, ": unable to switch to ", auto_qmail, ": ", error_str(errno), "\n");
							sleep(10);
						}
						chdir_toqueue();
					}
				}
			}
			bouncesender = "";
			bouncerecip = sender.s;
		} else
		if (*sender.s) {
			bouncesender = "#@[]";
			bouncerecip = doublebounceto.s;
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
		while (!stralloc_copyb(&boundary, strnum2, fmt_ulong(strnum2, birth)))
			nomem();
		while (!stralloc_cat(&boundary, &bouncehost))
			nomem();
		while (!stralloc_catb(&boundary, strnum2, fmt_ulong(strnum2, id)))
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
			qmail_puts(&qqt, "Hi. This is the slowq-send program at ");
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
		if ((fd = open_read(fn1.s)) == -1)
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
		switch (bounce_processor(&qqt, fn1.s, fn2.s, brep, orig_recip.s, bouncesender, bouncerecip))
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

typedef struct DEL {
	int             used;
	int             j;
	unsigned long   delid;
	seek_pos        mpos;
	stralloc        recip;
} DEL;

unsigned long   masterdelid = 1;
unsigned int    concurrency[CHANNELS] = { 10, 20 };
unsigned int    concurrencyused[CHANNELS] = { 0, 0 };
unsigned int    holdjobs[CHANNELS] = { 0, 0 };	/* Booleans: hold deliveries NJL 1998/05/03 */

static DEL     *del[CHANNELS];
static stralloc concurrencyf = { 0 };

static stralloc dline[CHANNELS];
static char     delbuf[2048];

static void
del_status()
{
	int             c;

	log1_noflush("status:");
	for (c = 0; c < CHANNELS; ++c) {
		strnum1[fmt_ulong(strnum1, (unsigned long) concurrencyused[c])] = 0;
		strnum2[fmt_ulong(strnum2, (unsigned long) concurrency[c])] = 0;
		log4_noflush(chanstatusmsg[c], strnum1, "/", strnum2);
		if (holdjobs[c]) /*NJL*/
			log1_noflush(" (held)"); /*NJL*/
	}
	if (delayed_jobs) {
		strnum1[fmt_ulong(strnum1, delayed_jobs)] = 0;
		log4_noflush(" delayed jobs=", strnum1, " ", queuedesc);
	} else
		log2_noflush(" ", queuedesc);
	if (flagexitasap)
		log1_noflush(" exitasap");
	log1_noflush("\n");
	flush();
}

static void
del_init()
{
	int             c, i;

	for (c = 0; c < CHANNELS; ++c) {
		flagspawnalive[c] = 1;
		while (!(del[c] = (struct DEL *) alloc(concurrency[c] * sizeof (struct DEL))))
			nomem();
		for (i = 0; i < concurrency[c]; ++i) {
			del[c][i].used = 0;
			del[c][i].recip.s = 0;
		}
		dline[c].s = 0;
		while (!stralloc_copys(&dline[c], ""))
			nomem();
	}
	del_status();
}

static int
del_canexit()
{
	int             c;

	for (c = 0; c < CHANNELS; ++c) {
		if (flagspawnalive[c] && !holdjobs[c]) { /* if dead or held /NJL/, nothing we can do about its jobs */
			if (concurrencyused[c]) /*- stay alive if delivery jobs are present */
				return 0;
		}
	}
	return 1;
}

static int
del_avail(int c)
{
	return flagspawnalive[c] && comm_canwrite(c) && !holdjobs[c] && (concurrencyused[c] < concurrency[c]);	/* NJL 1998/07/24 */
}

static void
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
		if (!del[c][i].used)
			break;
	if (i == concurrency[c])
		return;
	if (!stralloc_copys(&del[c][i].recip, recip) ||
			!stralloc_0(&del[c][i].recip)) {
		nomem();
		return;
	}
	del[c][i].j = j;
	++jo[j].refs;
	del[c][i].delid = masterdelid++;
	del[c][i].mpos = mpos;
	del[c][i].used = 1;
	++concurrencyused[c];
	comm_write(c, i, jo[j].id, jo[j].sender.s, jo[j].qqeh.s, jo[j].envh.s, recip);
	strnum1[fmt_ulong(strnum1, del[c][i].delid)] = 0;
	strnum2[fmt_ulong(strnum2, jo[j].id)] = 0;
	log2_noflush("starting delivery ", strnum1);
	log3_noflush(": msg ", strnum2, tochan[c]);
	logsafe_noflush(recip);
	log3(" ", queuedesc, "\n");
	del_status();
}

static void
markdone(int c, unsigned long id, seek_pos pos)
{
	struct stat     st;
	int             fd;
	fnmake_chanaddr(id, c);
	for (;;) {
		if ((fd = open_write(fn1.s)) == -1)
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
	log5("warning: ", queuedesc, ": trouble marking ", fn1.s, "; message will be delivered twice!\n");
}

/*
 * read delivery report
 * first 2 bytes = delivery number
 * byte 3 is
 * 'K' - success
 * 'Z' - deferral
 * 'D' - failure
 */
static void
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
			if ((delnum < 0) || (delnum >= concurrency[c]) || !del[c][delnum].used)
				log3("warning: ", queuedesc, ": internal error: delivery report out of range\n");
			else {
				strnum1[fmt_ulong(strnum1, del[c][delnum].delid)] = 0;
				if (dline[c].s[2] == 'Z') {
					if (jo[del[c][delnum].j].flagdying) {
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
					log3_noflush("delivery ", strnum1, ": success: ");
					logsafe_noflush(dline[c].s + 3);
					log3(" ", queuedesc, "\n");
					markdone(c, jo[del[c][delnum].j].id, del[c][delnum].mpos);
					--jo[del[c][delnum].j].numtodo;
					break;
				case 'Z':
					log3_noflush("delivery ", strnum1, ": deferral: ");
					logsafe_noflush(dline[c].s + 3);
					log3(" ", queuedesc, "\n");
					break;
				case 'D':
					log3_noflush("delivery ", strnum1, ": failure: ");
					logsafe_noflush(dline[c].s + 3);
					log3(" ", queuedesc, "\n");
					addbounce(jo[del[c][delnum].j].id, del[c][delnum].recip.s, dline[c].s + 3);
					markdone(c, jo[del[c][delnum].j].id, del[c][delnum].mpos);
					--jo[del[c][delnum].j].numtodo;
					break;
				default:
					log5("delivery ", strnum1, ": report mangled, will defer: ", queuedesc, "\n");
				}
				job_close(del[c][delnum].j);
				del[c][delnum].used = 0;
				--concurrencyused[c];
				del_status();
			}
			dline[c].len = 0;
		} /* if (!ch && (dline[c].len > 2)) */
	} /*- for (i = 0; i < r; ++i) */
}

static void
del_selprep(int *nfds, fd_set *rfds)
{
	int             c;

	for (c = 0; c < CHANNELS; ++c) {
		if (flagspawnalive[c]) {
			/* fd 2 - pi2[0] - read. fd 4 - pi4[0] - read */
			FD_SET(chanfdin[c], rfds);
			if (*nfds <= chanfdin[c])
				*nfds = chanfdin[c] + 1;
		}
	}
}

static void
del_do(fd_set *rfds)
{
	int             c;

	for (c = 0; c < CHANNELS; ++c) {
		if (flagspawnalive[c] && FD_ISSET(chanfdin[c], rfds))
			del_dochan(c); /*- read delivery report from qmail-lspawn, qmail-rspawn */
	}
}


/*- this file is too long -------------------------------------------- PASSES */

static struct {
	unsigned long   id;			/*- if 0, need a new pass */
	int             j;			/*- defined if id; job number */
	int             fd;			/*- defined if id; reading from {local,remote} */
	seek_pos        mpos;		/*- defined if id; mark position */
	substdio        ss;
	char            buf[128];
} pass[CHANNELS];

static void
pass_init()
{
	int             c;

	for (c = 0; c < CHANNELS; ++c)
		pass[c].id = 0;
}

static void
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
	if (job_avail() != -1) {
		for (c = 0; c < CHANNELS; ++c) {
			if (!pass[c].id && prioq_get(&pqchan[c], &pe) && *wakeup > pe.dt && !pe.delayed)
				*wakeup = pe.dt;
		}
	}
	if (prioq_get(&pqfail, &pe) && *wakeup > pe.dt)
		*wakeup = pe.dt;
	if (prioq_get(&pqdone, &pe) && *wakeup > pe.dt)
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

/*- generate quadratic retry schedule
 * local  chanskip=10
 * remote chanskip=20
 */
static datetime_sec
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

/*
 * pass job to qmail-lspawn, qmail-rpsawn
 */
static void
pass_dochan(int c)
{
	datetime_sec    birth;
	struct prioq_elt pe;
	static stralloc line = {0}, qqeh = {0}, envh = {0};
	datetime_sec    t;
	int             i, match, _do_ratelimit;

	if (flagexitasap)
		return;
	if (!pass[c].id) { /*- new pass */
		int             j;
		
		if ((j = job_avail()) == -1)
			return;
		if (!prioq_get(&pqchan[c], &pe))
			return;
		if (pe.dt > recent)
			return;
		fnmake_chanaddr(pe.id, c);
		prioq_del(&pqchan[c]); /*- remove from pqchan */
		pass[c].mpos = 0;
		if ((pass[c].fd = open_read(fn1.s)) == -1) /*- open local/split/inode or remote/split/inode */
			goto trouble;
		if (!getinfo(&line, &qqeh, &envh, &birth, pe.id)) { /*- read info/split/inode to get sender */
			close(pass[c].fd);
			goto trouble;
		}
		pass[c].id = pe.id;
		pass[c].j = j;
		substdio_fdbuf(&pass[c].ss, read, pass[c].fd, pass[c].buf, sizeof (pass[c].buf));
		job_open(pe.id, c, j);
		jo[j].retry = nextretry(birth, c);
		jo[j].flagdying = (recent > birth + lifetime);
#ifdef BOUNCELIFETIME
		if (!*line.s)
			jo[j].flagdying = (recent > birth + bouncelifetime);
#endif
		while (!stralloc_copy(&jo[j].sender, &line))
			nomem();
		while (!stralloc_copy(&jo[j].qqeh, &qqeh))
			nomem();
		while (!stralloc_copy(&jo[j].envh, &envh))
			nomem();
	}
	if (!del_avail(c))
		return;
	/*- read local/split/inode or remote/split/inode */
	if (getln(&pass[c].ss, &line, &match, '\0') == -1) {
		fnmake_chanaddr(pass[c].id, c);
		log5("warning: ", queuedesc, ": trouble reading ", fn1.s, "; will try again later\n");
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
	case 'T': /*- send message to qmail-lspawn/qmail-rspawn to start delivery */
		delivery = c;
		if (!(i = delivery_rate(line.s + 1, pe.id, &t, &_do_ratelimit))) {
			if (t && (t < time_needed || !time_needed))
				time_needed = t + SLEEP_FUZZ; /*- earliest delayed job */
			else
				time_needed = 0;
			close(pass[c].fd);
			jo[pass[c].j].retry = recent + t;
			job_close(pass[c].j);
			pass[c].id = 0;
			delayed_jobs = delayed_job_count();
			del_status();
			return;
		} else
		if (i == -1)
			log5("warning: ", queuedesc, ": failed to get delivery rate for ", line.s + 1, "; proceeding to deliver\n");
		else
		if (_do_ratelimit && delayed_jobs)
			delayed_jobs = delayed_job_count();
		++jo[pass[c].j].numtodo;
		del_start(pass[c].j, pass[c].mpos, line.s + 1); /*- line.s[1] = recipient */
		break;
	case 'D': /*- delivery done */
		break;
	default:
		fnmake_chanaddr(pass[c].id, c);
		log5("warning: ", queuedesc, ": unknown record type in ", fn1.s, "!\n");
		close(pass[c].fd);
		job_close(pass[c].j);
		pass[c].id = 0;
		return;
	}
	pass[c].mpos += line.len;
	return;
trouble:
	log5("warning: ", queuedesc, ": trouble opening ", fn1.s, "; will try again later\n");
	pe.dt = recent + SLEEP_SYSFAIL;
	while (!prioq_insert(min, &pqchan[c], &pe))
		nomem();
}

static void
messdone(unsigned long id)
{
	char            ch;
	int             c;
	struct prioq_elt pe;
	struct stat     st;

	for (c = 0; c < CHANNELS; ++c) {
		fnmake_chanaddr(id, c);
		if (stat(fn1.s, &st) == 0)
			return;	/*- false alarm; consequence of HOPEFULLY */
		if (errno != error_noent) {
			log5("warning: ", queuedesc, ": unable to stat ", fn1.s, "; will try again later\n");
			goto fail;
		}
	}
	fnmake_todo(id);
	if (stat(fn1.s, &st) == 0)
		return;
	if (errno != error_noent) {
		log5("warning: ", queuedesc, ": unable to stat ", fn1.s, "; will try again later\n");
		goto fail;
	}
	fnmake_info(id);
	if (stat(fn1.s, &st) == -1) {
		if (errno == error_noent)
			return;
		log5("warning: ", queuedesc, ": unable to stat ", fn1.s, "; will try again later\n");
		goto fail;
	}

	/*- -todo +info -local -remote ?bounce */
	if (!injectbounce(id))
		goto fail;	/*- injectbounce() produced error message */
	strnum1[fmt_ulong(strnum1, id)] = 0;
	log5("end msg ", strnum1, " ", queuedesc, "\n");

	/*- -todo +info -local -remote -bounce */
	fnmake_info(id);
	if (unlink(fn1.s) == -1) {
		log5("warning: ", queuedesc, ": unable to unlink ", fn1.s, "; will try again later\n");
		goto fail;
	}
	/*- -todo -info -local -remote -bounce; we can relax */
	fnmake_foop(id);
	if (substdio_putflush(&sstoqc, fn1.s, fn1.len) == -1) {
		cleandied();
		return;
	}
	if (substdio_get(&ssfromqc, &ch, 1) != 1) {
		cleandied();
		return;
	}
	if (ch != '+')
		log5("warning: ", queuedesc, ": qmail-clean unable to clean up ", fn1.s, "\n");
	return;
fail:
	pe.id = id;
	pe.dt = now() + SLEEP_SYSFAIL;
	while (!prioq_insert(min, &pqdone, &pe))
		nomem();
}

static void
pass_do()
{
	int             c;
	struct prioq_elt pe;

	for (c = 0; c < CHANNELS; ++c)
		pass_dochan(c);
	if (prioq_get(&pqfail, &pe)) {
		if (pe.dt <= recent) {
			prioq_del(&pqfail);
			pqadd(pe.id, 0);
		}
	}
	if (prioq_get(&pqdone, &pe)) {
		if (pe.dt <= recent) {
			prioq_del(&pqdone);
			messdone(pe.id);
		}
	}
}


/*- this file is too long ---------------------------------------------- TODO */

static stralloc rwline = { 0 };

/*
 * 1 if by land
 * 2 if by sea
 * 0 if out of memory. not allowed to barf.
 * may trash recip. must set up rwline, between a T and a \0.
 */
static int
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

static datetime_sec nexttodorun, lasttodorun;
static int      flagtododir = 0;	/*- if 0, have to readsubdir_init again */
static int      todo_interval = -1;
static readsubdir todosubdir;
static stralloc todoline = { 0 };

static char     todobuf[SUBSTDIO_INSIZE];
static char     todobufinfo[512];
static char     todobufchan[CHANNELS][1024];

static void
todo_init()
{
	flagtododir = 0;
	lasttodorun = nexttodorun = now();
	trigger_set();
}

static void
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

static stralloc mailfrom = { 0 };
static stralloc mailto = { 0 };

static void
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

	if (flagexitasap)
		return;
	fd = -1;
	fdinfo = -1;
	for (c = 0; c < CHANNELS; ++c)
		fdchan[c] = -1;
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
	case 0: /*- no files found */
		flagtododir = 0;
	default:
		return;
	}
	fnmake_todo(id);
	if ((fd = open_read(fn1.s)) == -1) {
		log3("warning: unable to open ", fn1.s, "\n");
		return;
	}
	fnmake_mess(id);
	/*- just for the statistics */
	if (stat(fn1.s, &st) == -1) {
		log3("warning: unable to stat ", fn1.s, "\n");
		goto fail;
	}
	for (c = 0; c < CHANNELS; ++c) {
		fnmake_chanaddr(id, c);
		if (unlink(fn1.s) == -1) {
			if (errno != error_noent) {
				log5("warning: unable to unlink: ", fn1.s, ": ", error_str(errno), "\n");
				goto fail;
			}
		}
	}
	fnmake_info(id);
	if (unlink(fn1.s) == -1) {
		if (errno != error_noent) {
			log3("warning: unable to unlink ", fn1.s, "\n");
			goto fail;
		}
	}
	if ((fdinfo = open_excl(fn1.s)) == -1) {
		log3("warning: unable to create ", fn1.s, "\n");
		goto fail;
	}
	strnum1[fmt_ulong(strnum1, id)] = 0;
	log3("new msg ", strnum1, "\n");
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
			log3("warning: trouble reading ", fn1.s, "\n");
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
				log3("warning: trouble writing to ", fn1.s, "\n");
				goto fail;
			}
			break;
		case 'F':
			if (substdio_put(&ssinfo, todoline.s, todoline.len) == -1) {
				fnmake_info(id);
				log3("warning: trouble writing to ", fn1.s, "\n");
				goto fail;
			}
			log2_noflush("info msg ", strnum1);
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
				fdchan[c] = open_excl(fn1.s);
				if (fdchan[c] == -1) {
					log5("warning: ", queuedesc, ": unable to create ", fn1.s, "\n");
					goto fail;
				}
				substdio_fdbuf(&sschan[c], write, fdchan[c], todobufchan[c], sizeof (todobufchan[c]));
				flagchan[c] = 1;
			}
			if (substdio_bput(&sschan[c], rwline.s, rwline.len) == -1) {
				fnmake_chanaddr(id, c);
				log5("warning: ", queuedesc, ": trouble writing to ", fn1.s, "\n");
				goto fail;
			}
			break;
		default:
			fnmake_todo(id);
			log5("warning: ", queuedesc, ": unknown record type in ", fn1.s, "\n");
			goto fail;
		}
	} /*- for (;;) */
	close(fd);
	fd = -1;
	fnmake_info(id);
	if (substdio_flush(&ssinfo) == -1) {
		log5("warning: ", queuedesc, ": trouble writing to ", fn1.s, "\n");
		goto fail;
	}
	for (c = 0; c < CHANNELS; ++c) {
		if (fdchan[c] != -1) {
			if (substdio_flush(&sschan[c]) == -1) {
				fnmake_chanaddr(id, c);
				log5("warning: ", queuedesc, ": trouble writing to ", fn1.s, "\n");
				goto fail;
			}
		}
	}
#ifdef USE_FSYNC
	if (use_fsync > 0 && fsync(fdinfo) == -1) {
		log5("warning: ", queuedesc, ": trouble fsyncing ", fn1.s, "\n");
		goto fail;
	}
#endif
	close(fdinfo);
	fdinfo = -1;
	for (c = 0; c < CHANNELS; ++c) {
		if (fdchan[c] != -1) {
#ifdef USE_FSYNC
			if (use_fsync > 0 && fsync(fdchan[c]) == -1) {
				fnmake_chanaddr(id, c);
				log5("warning: ", queuedesc, ": trouble fsyncing ", fn1.s, "\n");
				goto fail;
			}
#endif
			close(fdchan[c]);
			fdchan[c] = -1;
		}
	}
	fnmake_todo(id);
	if (substdio_putflush(&sstoqc, fn1.s, fn1.len) == -1) {
		cleandied();
		return;
	}
	if (substdio_get(&ssfromqc, &ch, 1) != 1) {
		cleandied();
		return;
	}
	if (ch != '+') {
		log5("warning: ", queuedesc, ": qmail-clean unable to clean up ", fn1.s, "\n");
		return;
	}
	pe.id = id;
	pe.dt = now();
	for (c = 0; c < CHANNELS; ++c) {
		if (flagchan[c])
			while (!prioq_insert(min, &pqchan[c], &pe))
				nomem();
	}
	for (c = 0; c < CHANNELS; ++c) {
		if (flagchan[c])
			break;
	}
	if (c == CHANNELS) {
		while (!prioq_insert(min, &pqdone, &pe))
			nomem();
	}
	log_stat(&mailfrom, &mailto, id, st.st_size);
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
/*- this file is too long ---------------------------------------------- MAIN */

static int
getcontrols()
{
	if (control_init() == -1)
		return 0;
	if (control_readint(&bouncemaxbytes, "bouncemaxbytes") == -1)
		return 0;
	if (!queuedesc) {
		for (queuedesc = queuedir; *queuedesc; queuedesc++);
		for (; queuedesc != queuedir && *queuedesc != '/'; queuedesc--);
		if (*queuedesc == '/')
			queuedesc++;
	}
	/*- read concurrencylocal and concurrencylocal.queue[n] */
	if (control_readint((int *) &concurrency[0], "concurrencylocal") == -1)
		return 0;
	/*- per queue concurrency */
	if (!stralloc_copys(&concurrencyf, "concurrencyl.") ||
			!stralloc_cats(&concurrencyf, queuedesc) ||
			!stralloc_0(&concurrencyf))
		return 0;
	if (control_readint((int *) &concurrency[0], concurrencyf.s) == -1)
		return 0;
	/*- read concurrencyremote and concurrencyremote.queue[n] */
	if (control_readint((int *) &concurrency[1], "concurrencyremote") == -1)
		return 0;
	/*- per queue concurrency */
	if (!stralloc_copys(&concurrencyf, "concurrencyr.") ||
			!stralloc_cats(&concurrencyf, queuedesc) ||
			!stralloc_0(&concurrencyf))
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
	} else
	if (!use_syncdir) {
		if (!env_unset("USE_SYNCDIR"))
			return 0;
	}
	if (use_fsync > 0) {
		if (!env_put2("USE_FSYNC", "1"))
			return 0;
	} else
	if (!use_fsync) {
		if (!env_unset("USE_FSYNC"))
			return 0;
	}
#endif
#ifdef HAVESRS
	if (control_readline(&srs_domain, "srs_domain") == -1)
		return 0;
	if (srs_domain.len && !stralloc_0(&srs_domain))
		return 0;
#endif
	if (control_readint((int *) &lifetime, "queuelifetime") == -1)
		return 0;
#ifdef BOUNCELIFETIME
	if (control_readint(&bouncelifetime, "bouncelifetime") == -1)
		return 0;
	if (bouncelifetime > lifetime)
		bouncelifetime = lifetime;
#endif
	if (control_rldef(&bouncefrom, "bouncefrom", 0, "MAILER-DAEMON") != 1)
		return 0;
	if (control_rldef(&bouncehost, "bouncehost", 1, "bouncehost") != 1)
		return 0;
	if (control_rldef(&doublebouncehost, "doublebouncehost", 1, "doublebouncehost") != 1)
		return 0;
	if (control_rldef(&doublebounceto, "doublebounceto", 0, "postmaster") != 1)
		return 0;
	if (!stralloc_cats(&doublebounceto, "@"))
		return 0;
	if (!stralloc_cat(&doublebounceto, &doublebouncehost))
		return 0;
	if (!stralloc_0(&doublebounceto))
		return 0;
	if (control_readnativefile(&bouncemessage, "bouncemessage", 0) == -1)
		return 0;
	if (control_readnativefile(&doublebouncemessage, "doublebouncemessage", 0) == -1)
		return 0;
	if (control_rldef(&bouncesubject, "bouncesubject", 0, "failure notice") != 1)
		return 0;
	if (control_rldef(&doublebouncesubject, "doublebouncesubject", 0, "failure notice") != 1)
		return 0;
	if (control_readfile(&locals, "locals", 1) != 1)
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
	if (control_readint(&todo_interval, "todointerval") == -1)
		return 0;
	return 1;
}

/*
 * The reason for DJB using newlocals and newvdoms is so that
 * the original variables locals and vdoms do not get screwed
 * in regetcontrols. This will allow slowq-send to serve domains
 * already being served (inspite of memory problems during regetcontrols).
 * new domains added will not get served till another sighup causes
 * regetcontrols to get executed without problems. This is much
 * better than having slowq-send come to a grinding halt.
 * Another way could be to set flagexitasap to 1
 */
static stralloc newlocals = { 0 };
static stralloc newvdoms = { 0 };

static void
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
	if (control_readint(&todo_interval, "todointerval") == -1) {
		log5("alert: ", queuedesc, ": qmail-todo: unable to reread ", controldir, "/todointerval\n");
		return;
	}
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
	} else
	if (!use_syncdir) {
		while (!env_unset("USE_SYNCDIR"))
			nomem();
	}
	if (use_fsync > 0) {
		while (!env_put2("USE_FSYNC", "1"))
			nomem();
	} else
	if (!use_fsync) {
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
				flagrunasap = 1; /*- run all jobs now */
			}
		}
	}
	constmap_free(&maplocals);
	while (!stralloc_copy(&locals, &newlocals))
		nomem();
	while (!constmap_init(&maplocals, locals.s, locals.len, 0))
		nomem();
	constmap_free(&mapvdoms);
	if (r) {
		while (!stralloc_copy(&vdoms, &newvdoms))
			nomem();
		while (!constmap_init(&mapvdoms, vdoms.s, vdoms.len, 1))
			nomem();
	} else
		while (!constmap_init(&mapvdoms, "", 0, 1))
			nomem();
}

static void
reread()
{
	if (chdir(auto_qmail) == -1) {
		log7("alert: ", queuedesc, ": unable to reread controls: unable to switch to ", auto_qmail, ": ", error_str(errno), "\n");
		return;
	}
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
	char           *ptr;

	if (!(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue/slowq"; /*- single queue like qmail */
	do_ratelimit = env_get("RATELIMIT_DIR") ? 1 : 0;
	/*- get basename of queue directory to define slowq-send instance */
	for (queuedesc = queuedir; *queuedesc; queuedesc++);
	for (; queuedesc != queuedir && *queuedesc != '/'; queuedesc--);
	if (*queuedesc == '/')
		queuedesc++;
	if (chdir(auto_qmail) == -1) {
		log7("alert: ", queuedesc, ": cannot start: unable to switch to ", auto_qmail, ": ", error_str(errno), "\n");
		_exit(111);
	}
	getEnvConfigInt(&conf_split, "CONFSPLIT", auto_split);
	if (conf_split > auto_split)
		conf_split = auto_split;
	strnum1[fmt_ulong(strnum1, conf_split)] = 0;
	log7("info: slowq-send: ", queuedesc, ": ratelimit=", do_ratelimit ? "ON" : "OFF", ", conf split=", strnum1, "\n");
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
	if (!getcontrols()) {
		log3("alert: ", queuedesc, ": cannot start: unable to read controls\n");
		_exit(111);
	}
	if (chdir(queuedir) == -1) {
		log7("alert: ", queuedesc, ": cannot start: unable to switch to queue directory ", queuedir, ": ", error_str(errno), "\n");
		_exit(111);
	}
	sig_pipeignore();
	sig_termcatch(sigterm);
	sig_alarmcatch(sigalrm);
	sig_hangupcatch(sighup);
	sig_childdefault();
	umask(077);
	/*- prevent multiple copies of slowq-send to run */
	if ((fd = open_write("lock/sendmutex")) == -1) {
		log3("alert: ", queuedesc, ": cannot start: unable to open mutex\n");
		_exit(111);
	}
	if (lock_exnb(fd) == -1) {
		log3("alert: ", queuedesc, ": cannot start: slowq-send is already running\n");
		_exit(111);
	}
#ifdef USE_FSYNC
	if (env_get("USE_FSYNC"))
		use_fsync = 1;
#endif

	numjobs = 0;
	/*- read 2 bytes from qmail-lspawn, qmail-rspawn to get concurrency */
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
	} /*- for (c = 0; c < CHANNELS; ++c) */

	fnmake_init();  /*- initialize fn1, fn2 */
	comm_init();    /*- assign fd 5 to queue comm to, 6 to queue comm from */
	pqstart();      /*- add files from info/split for processing */
	job_init();     /*- initialize numjobs job structures */
	del_init();     /*- initialize concurrencylocal + concurrrencyremote delivery structure */
	pass_init();    /*- initialize pass structure */
	todo_init();    /*- set fd 7 to write to qmail-todo, set fd 8 to read from qmail-todo, write 'S' to qmail-todo */
	cleanup_init(); /*- initialize flagcleanup = 0, cleanuptime = now*/
	while (!flagexitasap || !del_canexit()) { /*- stay alive if delivery jobs are present */
		recent = now();
		if (flagrunasap) {
			flagrunasap = 0;
			pqrun();
		}
		if (flagreadasap) {
			flagreadasap = 0;
			reread();
		}
		wakeup = time_needed ? recent + time_needed : recent + SLEEP_FOREVER;
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		nfds = 1;
		/*-
		 * prepare select for write
		 * fd 1 - pi1[1] - write to qmail-lspawn
		 * fd 3 - pi3[1] - write to qmail-rspawn
		 */
		comm_selprep(&nfds, &wfds);
		/*-
		 * prepare readfd for select
		 * fd 2 - pi2[0] - read from qmail-lspawn
		 * fd 4 - pi4[0] - read from qmail-rspawn
		 */
		del_selprep(&nfds, &rfds);
		/*- set wakeup
		 * pqchan, pqfail, pqdone
		 * 1. set if new delivery jobs available
		 * 2. set if the earliest mail delivery message added from todo is now or earlier
		 * 3. set if the earliest mail delivery for local/remote is now or earlier
		 * 4. set if the earliest mail delivery for messages found in todo but stat had failed
		 */
		pass_selprep(&wakeup);
		/*-
		 * prepare readfd for select
		 * set todofdin = 8 for select: read end from qmail-todo pi8[0]
		 */
		todo_selprep(&nfds, &rfds, &wakeup);
		/*- set wakeup */
		cleanup_selprep(&wakeup);
		if (wakeup <= recent)
			tv.tv_sec = time_needed ? time_needed : 0;
		else
		if (time_needed && time_needed < (wakeup - recent))
			tv.tv_sec = time_needed + SLEEP_FUZZ;
		else
			tv.tv_sec = wakeup - recent + SLEEP_FUZZ;
		tv.tv_usec = 0;
		if (select(nfds, &rfds, &wfds, (fd_set *) 0, &tv) == -1) {
			if (errno == error_intr);
			else
				log3("warning: ", queuedesc, ": trouble in select\n");
		} else {
			time_needed = 0;
			recent = now();
			/*- communicate with qmail-lspawn, qmail-rspawn
			 * These were created as pi1, pi3 in qmail-start.c
			 * write on fd 1 read by fd 0 in qmail-lspawn,
			 * write on fd 3 read by fd 0 in qmail-rspawn
			 */
			comm_do(&wfds);
			/*-
			 * if data is available for read on fd 2 or 4
			 * then read from qmail-lspawn or qmail-rspawn
			 * fd 2 - pi2[0] - data from qmail-lspawn
			 * fd 4 - pi4[0] - data from qmail-rspawn
			 */
			del_do(&rfds);
			/* read data from fd 8 from qmail-todo
			 * 'D' - Do deliver
			 * 'L' - Write to log (fd 0) for logging
			 * 'X' - set flagtodoalive = 0, flagexitasap = 1
			 */
			todo_do(&rfds);
			pass_do();
			cleanup_do();
		}
	} /*- while (!flagexitasap || !del_canexit()) */
	pqfinish();
	log5("status: ", queuedesc, " ", queuedesc, " slowq-send exiting\n");
	return (0);
}

void
getversion_slowq_send_c()
{
	static char    *x = "$Id: slowq-send.c,v 1.11 2021-07-15 22:37:20+05:30 Cprogrammer Exp mbhangui $";

	x = sccsiddelivery_rateh;
	if (x)
		x++;
}
