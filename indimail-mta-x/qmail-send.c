/*
 * $Id: qmail-send.c,v 1.119 2025-05-06 22:39:16+05:30 Cprogrammer Exp mbhangui $
 */
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/time.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include "sig.h"
#include "direntry.h"
#include "select.h"
#include "open.h"
#include "seek.h"
#include "lock.h"
#include "ndelay.h"
#include "now.h"
#include "getln.h"
#include "getEnvConfig.h"
#include "wait.h"
#include "substdio.h"
#include "alloc.h"
#include "error.h"
#include "stralloc.h"
#include "str.h"
#include "byte.h"
#include "fmt.h"
#include "scan.h"
#include "case.h"
#include "constmap.h"
#include "env.h"
#include "haslibrt.h"
#ifdef HASLIBRT
#include <sys/mman.h>
#include <fcntl.h> /* For O_* constants */
#endif
#include "sgetopt.h"
#include "auto_qmail.h"
#include "auto_control.h"
#include "auto_split.h"
#include "control.h"
#include "trigger.h"
#include "newfield.h"
#include "quote.h"
#include "qmail.h"
#include "qsutil.h"
#include "prioq.h"
#include "fmtqfn.h"
#include "envrules.h"
#include "variables.h"
#include "readsubdir.h"
#include "hassrs.h"
#ifdef HAVESRS
#include "srs.h"
#endif
#ifdef USE_FSYNC
#include "syncdir.h"
#endif
#include "delivery_rate.h"
#include "getDomainToken.h"

/*- critical timing feature #1: if not triggered, do not busy-loop */
/*- critical timing feature #2: if triggered, respond within fixed time */
/*- important timing feature: when triggered, respond instantly */
#define SLEEP_FUZZ         1 /*- slop a bit on sleeps to avoid zeno effect */
#define SLEEP_FOREVER  86400 /*- absolute maximum time spent in select() */
#define SLEEP_CLEANUP  76431 /*- time between cleanups */
#define SLEEP_SYSFAIL    123

typedef const char c_char;
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

static char     mypid[FMT_ULONG];
static char     strnum1[FMT_ULONG];
static char     strnum2[FMT_ULONG];
static char    *qident;

#define CHANNELS 2
static c_char  *chanaddr[CHANNELS] = { "local/", "remote/" };
static c_char  *chanstatusmsg[CHANNELS] = { " local ", " remote " };
static c_char  *chanjobsheldmsg[CHANNELS] = { /* NJL 1998/05/03 */
	"local deliveries temporarily held\n",
	"remote deliveries temporarily held\n"
};
static c_char  *chanjobsunheldmsg[CHANNELS] = {	/* NJL 1998/05/03 */
	"local deliveries resumed\n",
	"remote deliveries resumed\n"
};
static c_char  *tochan[CHANNELS] = { " to local ", " to remote " };
static int      chanfdout[CHANNELS] = { 1, 3 };
static int      chanfdin[CHANNELS] = { 2, 4 };
static int      chanskip[CHANNELS] = { 10, 20 };

const char     *queuedesc;
static c_char  *argv0 = "qmail-send";

extern dtype    delivery;
static int      do_ratelimit;
unsigned long   delayed_jobs;

static int      flagexitsend = 0;
static int      flagrunasap = 0;
static int      flagreadasap = 0;
/*-
 * allow qmail-send to stop adding new jobs received from todo-proc
 * flagdetached values
 * 0 - attached (todo-proc sends delivery jobs to qmail-send)
 * 1 - half detached (qmail-send instructs todo-proc to resume
 *     sending jobs when there no jobs to process)
 * 2 - full detached (todo-proc stops sending jobs to qmail-send
 *     until qmail-send gets SIGUSR2
 */
static int      flagdetached = 0;
static int      dynamic_queue = 0;
#ifdef HASLIBRT
static int      shm_queue = -1;
#endif
static int      bigtodo;

static void     reread(int);
static void     sigusr1(int x);
static void     sigusr2(int x);
static void     exit_todo();

static void
sigterm(int x)
{
	flagexitsend = 1;
	slog(1, "alert: ", argv0, ": pid ", mypid, " got TERM: ", queuedesc, "\n", NULL);
	exit_todo();
}

static void
sigalrm(int x)
{
	flagrunasap = 1;
	slog(1, "alert: ", argv0, ": pid ", mypid, " got ALRM: ", queuedesc, "\n", NULL);
}

static void
sighup(int x)
{
	flagreadasap = 1;
	slog(1, "alert: ", argv0, ": pid ", mypid, " got HUP: ", queuedesc, "\n", NULL);
}

static void
chdir_toqueue()
{
	if (!queuedir && !(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue"; /*- single queue like qmail */
	while (chdir(queuedir) == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc,
				": unable to switch back to queue directory; HELP! sleeping...",
				error_str(errno), "\n", NULL);
		sleep(10);
	}
}

#ifdef LOGLOCK
static void
sigint(int x)
{
	if (loglock_fd == -1) {
		if (chdir(auto_qmail) == -1) {
			slog(1, "alert: ", argv0, ": ", queuedesc,
					": unable to reread controls: unable to switch to ",
					auto_qmail, ": ", error_str(errno), "\n", NULL);
			return;
		}
		loglock_open(argv0, 1);
		slog(1, "info: ", argv0, ": ", queuedesc, ": loglock enabled\n", NULL);
		chdir_toqueue();
	} else {
		if (loglock_fd != -1) {
			close(loglock_fd);
			loglock_fd = -1;
			slog(1, "info: ", argv0, ": ", queuedesc, ": loglock disabled\n", NULL);
		}
	}
}
#endif

static stralloc todoline = { 0 };
static char     todobuf[2048];
static int      todofdi = 8, todofdo = 7, flagtodoalive;
int             flagspawnalive[CHANNELS];

static void
exit_todo()
{
	int             r;

	if (!flagtodoalive)
		return;
	if (write(todofdo, "X", 1)) ; /*- keep compiler happy */
	r = read(todofdi, todobuf, sizeof (todobuf));
	if (r > 0 && todobuf[0] == 'L')
		slog(1, todobuf + 1, NULL);
	flagtodoalive = 0;
}

static void
cleandied()
{
	slog(1, "alert: ", argv0, ": ", queuedesc, ": oh no! lost qmail-clean connection! dying...\n", NULL);
	flagexitsend = 1;
	if (flagtodoalive)
		exit_todo();
}

static void
spawndied(int c)
{
	slog(1, "alert: ", argv0, ": ", queuedesc, ": oh no! lost spawn connection! dying...\n", NULL);
	flagspawnalive[c] = 0;
	flagexitsend = 1;
	if (flagtodoalive)
		exit_todo();
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
		nomem(argv0);
	while (!stralloc_ready(&fn2, FMTQFN))
		nomem(argv0);
}

static void
fnmake_info(unsigned long id)
{
	fn1.len = fmtqfn(fn1.s, "info/", id, 1);
}

static void
fnmake_todo(unsigned long id)
{
	fn1.len = fmtqfn(fn1.s, "todo/", id, bigtodo);
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
fnmake_bounce(unsigned long id)
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
					nomem(argv0);
				while (!stralloc_catb(sa, recip, k))
					nomem(argv0);
				while (!stralloc_cats(sa, "="))
					nomem(argv0);
				while (!stralloc_cats(sa, recip + k + 1))
					nomem(argv0);
				while (!stralloc_cats(sa, "@"))
					nomem(argv0);
				while (!stralloc_catb(sa, sender + j + 1, i - 5 - j))
					nomem(argv0);
				return;
			}
		}
	}
	while (!stralloc_cats(sa, sender))
		nomem(argv0);
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
	substdio_fdbuf(&ss, (ssize_t (*)(int,  char *, size_t)) read, fdinfo, buf, sizeof (buf));
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
				nomem(argv0);
		if (line.s[0] == 'e') /*- qqeh */
			while (!stralloc_copys(qh, line.s + 1))
				nomem(argv0);
		if (line.s[0] == 'h') /*- envheader */
			while (!stralloc_copys(eh, line.s + 1))
				nomem(argv0);
	}
	close(fdinfo);
	while (!stralloc_0(sa))
		nomem(argv0);
	while (!stralloc_0(qh))
		nomem(argv0);
	while (!stralloc_0(eh))
		nomem(argv0);
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
	substdio_fdbuf(&sstoqc, (ssize_t (*)(int,  char *, size_t)) write, 5, sstoqcbuf, sizeof (sstoqcbuf));
	substdio_fdbuf(&ssfromqc, (ssize_t (*)(int,  char *, size_t)) read, 6, ssfromqcbuf, sizeof (ssfromqcbuf));
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
		nomem(argv0);
	ch = delnum;
	while (!stralloc_append(&comm_buf[c], &ch))
		nomem(argv0);
	ch = delnum >> 8;
	while (!stralloc_append(&comm_buf[c], &ch))
		nomem(argv0);
	fnmake_split(id);
	while (!stralloc_cats(&comm_buf[c], fn1.s))
		nomem(argv0);
	while (!stralloc_0(&comm_buf[c]))
		nomem(argv0);
	senderadd(&comm_buf[c], sender, recip);
	while (!stralloc_0(&comm_buf[c]))
		nomem(argv0);
	while (!stralloc_cats(&comm_buf[c], qqeh))
		nomem(argv0);
	while (!stralloc_0(&comm_buf[c]))
		nomem(argv0);
	while (!stralloc_cats(&comm_buf[c], envh))
		nomem(argv0);
	while (!stralloc_0(&comm_buf[c]))
		nomem(argv0);
	while (!stralloc_cats(&comm_buf[c], recip))
		nomem(argv0);
	while (!stralloc_0(&comm_buf[c]))
		nomem(argv0);
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
	unsigned long   id, ossified;

	if (!flagcleanup) {
		if (recent < cleanuptime)
			return;
		readsubdir_init(&cleanupdir, "mess", 1, pausedir);
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
	getEnvConfiguLong(&ossified, "OSSIFIED", OSSIFIED);
	if (recent <= st.st_atime + ossified)
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
		slog(1, "warn: ", argv0, ": ", queuedesc, ": qmail-clean unable to clean up ", fn1.s, "\n", NULL);
}

/*- this file is too long ----------------------------------- PRIORITY QUEUES */

static prioq    pqchan[CHANNELS] = { {0} , {0} };	/*- pqchan 0: -todo +info +local ?remote */
													/*- pqchan 1: -todo +info ?local +remote */
static prioq    pqdone = { 0 };						/*- -todo +info; HOPEFULLY -local -remote */
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
			/*-
			 * if rate limiting, add already existing IDs
			 * to priority queue with delayed flag
			 */
			pechan[c].delayed = delayedflag;
		}
	}
	for (c = 0; c < CHANNELS; ++c) {
		if (flagchan[c]) {
			while (!prioq_insert(min, &pqchan[c], &pechan[c]))
				nomem(argv0);
		}
	}
	for (c = 0; c < CHANNELS; ++c) {
		if (flagchan[c])
			break;
	}
	if (c == CHANNELS) {
		pe.id = id; /*- new addition to priority queue */
		pe.dt = now();
		pe.delayed = 0;
		while (!prioq_insert(min, &pqdone, &pe))
			nomem(argv0);
	}
	return;
fail:
	slog(1, "warn: ", argv0, ": ", queuedesc, ": unable to stat ", fn1.s, "; will try again later\n", NULL);
	pe.id = id;
	pe.dt = now() + SLEEP_SYSFAIL;
	while (!prioq_insert(min, &pqfail, &pe))
		nomem(argv0);
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

	readsubdir_init(&rs, "info", 1, pausedir); /*- pausedir is a function in qsutil */
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
				slog(1, "warn: ", argv0, ": ", queuedesc, "unable to utime ",
						fn1.s, "; message will be retried too soon\n", NULL);
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
		nomem(argv0);
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
	jo[j].numtodo = 0; /*- prevent closing job before delivery completes */
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
			slog(1, "warn: ", argv0, ": ", queuedesc, ": unable to unlink ", fn1.s, "; will try again later\n", NULL);
			pe.dt = now() + SLEEP_SYSFAIL;
		} else {
			int             c;
			for (c = 0; c < CHANNELS; ++c) {
				if (c != jo[j].channel) {
					fnmake_chanaddr(jo[j].id, c);
					if (stat(fn1.s, &st) == 0)
						return;	/*- more channels going */
					if (errno != error_noent) {
						slog(1, "warn: ", argv0, ": ", queuedesc, ": unable to stat ", fn1.s, "\n", NULL);
						break;	/*- this is the only reason for HOPEFULLY */
					}
				}
			}
			pe.dt = now();
			while (!prioq_insert(min, &pqdone, &pe))
				nomem(argv0);
			return;
		}
	}
	pe.delayed = do_ratelimit ? 1 : 0;
	while (!prioq_insert(min, &pqchan[jo[j].channel], &pe))
		nomem(argv0);
}

/*- this file is too long ------------------------------------------- BOUNCES */

/*- strip the virtual domain which is prepended to addresses e.g. xxx.com-user01@xxx.com */
static const char *
stripvdomprepend(char *recip)
{
	unsigned int    i, domainlen;
	const char     *domain, *prepend;

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
		nomem(argv0);
	while (!stralloc_cats(&bouncetext, stripvdomprepend(recip)))
		nomem(argv0);
	for (pos = 0; pos < bouncetext.len; ++pos) {
		if (bouncetext.s[pos] == '\n')
			bouncetext.s[pos] = '_';
	}
	while (!stralloc_copy(&orig_recip, &bouncetext))
		nomem(argv0);
	while (!stralloc_catb(&orig_recip, ">\n", 2))
		nomem(argv0);
	while (!stralloc_catb(&bouncetext, ">:\n", 3))
		nomem(argv0);
	while (!stralloc_cats(&bouncetext, report))
		nomem(argv0);
	if (report[0] && report[str_len(report) - 1] != '\n') {
		while (!stralloc_append(&bouncetext, "\n"))
			nomem(argv0);
	}
	for (pos = bouncetext.len - 2; pos > 0; --pos) {
		if (bouncetext.s[pos] == '\n' && bouncetext.s[pos - 1] == '\n')
			bouncetext.s[pos] = '/';
	}
	while (!stralloc_append(&bouncetext, "\n"))
		nomem(argv0);
	fnmake_bounce(id);
	for (;;) {
		if ((fd = open_append(fn2.s)) != -1)
			break;
		slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to append to bounce message; HELP! sleeping...\n", NULL);
		sleep(10);
	}
	pos = 0;
	while (pos < bouncetext.len) {
		if ((w = write(fd, bouncetext.s + pos, bouncetext.len - pos)) <= 0) {
			slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to append to bounce message; HELP! sleeping...\n", NULL);
			sleep(10);
		} else
			pos += w;
	}
	close(fd);
}

static int
bounce_processor(struct qmail *qq, const char *messfn, const char *bouncefn,
		const char *bounce_report, const char *origrecip, const char *sender,
		const char *recipient)
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
		slog(1, "alert: ", argv0, ": ", queuedesc, ": Unable to fork: ", error_str(errno), "\n", NULL);
		return (111);
	case 0:
		args[0] = prog;
		args[1] = (char *) messfn; /*- message filename */
		args[2] = (char *) bouncefn;	/*- bounce message filename */
		args[3] = (char *) bounce_report; /*- bounce report */
		args[4] = (char *) sender; /*- bounce sender */
		args[5] = (char *) origrecip; /*- original recipient */
		args[6] = (char *) recipient; /*- original sender */
		args[7] = 0;
		execv(*args, args);
		slog(1, "alert: ", argv0, ": ", queuedesc, ": Unable to run: ", prog, ": ", error_str(errno), "\n", NULL);
		_exit(111);
	}
	wait_pid(&wstat, child);
	if (wait_crashed(wstat)) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": ", prog, " crashed: ", error_str(errno), "\n", NULL);
		return (111);
	}
	i = wait_exitcode(wstat);
	strnum1[fmt_ulong(strnum1, i)] = 0;
	slog(1, "bounce processor sender <", sender, "> recipient <", recipient,
		"> messfn <", messfn, "> bouncefn <", bouncefn,
		"> exit=", strnum1, " ", queuedesc, "\n", NULL);
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
	const char     *bouncesender, *bouncerecip = "";
	char           *brep = (char *) "?", *p;
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
	fnmake_bounce(id);
	fnmake_mess(id);
	if (stat(fn2.s, &st) == -1) {
		if (errno == error_noent)
			return 1;
		slog(1, "warn: ", argv0, ": ", queuedesc, ": unable to stat ", fn2.s, "\n", NULL);
		return 0;
	}
	if (str_equal(sender.s, "#@[]"))
		slog(1, "triple bounce: discarding ", fn2.s, " ", queuedesc, "\n", NULL);
	else
	if (!*sender.s && *doublebounceto.s == '@')
		slog(1, "double bounce: discarding ", fn2.s, " ", queuedesc, "\n", NULL);
	else {
		restore_env();
		if ((p = env_get("BOUNCEQUEUE")) && !env_put2("QMAILQUEUE", p)) {
			slog(1, "alert: ", argv0, ": ", queuedesc, ": out of memory; will try again later\n", NULL);
			restore_env();
			return (0);
		}
		/*-
		 * Unset SPAMFILTER & FILTERARGS before calling qmail-queue
		 * and set it back after queue-queue returns
		 */
		if ((env_get("SPAMFILTER") && !env_unset("SPAMFILTER")) || (env_get("FILTERARGS") && !env_unset("FILTERARGS"))) {
			slog(1, "alert: ", argv0, ": ", queuedesc, ": out of memory; will try again later\n", NULL);
			restore_env();
			return (0);
		}
		/*-
		 * Allow bounces to have different rules, queue, controls, etc
		 */
		if (chdir(auto_qmail) == -1) {
			slog(1, "alert: ", argv0, ": ", queuedesc,
					": unable to reread controls: unable to switch to ",
					auto_qmail, ": ", error_str(errno), "\n", NULL);
			restore_env();
			return (0);
		}
		switch ((ret = envrules(sender.s, "bounce.envrules", "BOUNCERULES", 0)))
		{
		case AM_MEMORY_ERR:
			slog(1, "alert: ", argv0, ": ", queuedesc, ": out of memory; will try again later\n", NULL);
			restore_env();
			chdir_toqueue();
			return (0);
			break;
		case AM_FILE_ERR:
			slog(1, "alert: ", argv0, ": ", queuedesc, ": cannot start: unable to read bounce.envrules\n", NULL);
			restore_env();
			chdir_toqueue();
			return (0);
			break;
		case AM_REGEX_ERR:
			slog(1, "alert: ", argv0, ": ", queuedesc, ": cannot start: regex compilation failed\n", NULL);
			restore_env();
			chdir_toqueue();
			return (0);
			break;
		case 0:
			chdir_toqueue();
			break;
		default:
			if (ret > 0)
				reread(0); /*- this does chdir_toqueue() */
			else
			if (ret) {
				slog(1, "alert: ", argv0, ": ", queuedesc, ": cannot start: envrules failed\n", NULL);
				restore_env();
				chdir_toqueue();
				return (0);
			} else
				chdir_toqueue();
			break;
		}
		if (qmail_open(&qqt) == -1) {
			slog(1, "warn: ", argv0, ": ", queuedesc, ": unable to start qmail-queue, will try later\n", NULL);
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
							slog(1, "alert: ", argv0, ": ", queuedesc, ": srs: ", srs_error.s, "\n", NULL);
							qmail_fail(&qqt);
							break;
						case -2:
							nomem(argv0);
							qmail_fail(&qqt);
							break;
						case -1:
							slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to read controls\n", NULL);
							qmail_fail(&qqt);
							break;
						case 0:
							break;
						case 1:
							while (!stralloc_copy(&sender, &srs_result))
								nomem(argv0);
							while (!stralloc_0(&sender))
								nomem(argv0);
							sender.len--;
							break;
						}
						while (chdir(auto_qmail) == -1) {
							slog(1, "alert: ", argv0, ": ", queuedesc,
									": unable to switch to ", auto_qmail,
									": ", error_str(errno), "\n", NULL);
							sleep(10);
						}
						chdir_toqueue();
					}
				}
			}
			bouncesender = "";
			bouncerecip = sender.s;
		} else {
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
			nomem(argv0);
		qmail_put(&qqt, newfield_date.s, newfield_date.len);
		if (orig_recip.len) {
			qmail_put(&qqt, "X-Bounced-Address: ", 19);
			qmail_put(&qqt, orig_recip.s, orig_recip.len);
		}
		qmail_put(&qqt, "From: ", 6);
		while (!quote(&quoted, &bouncefrom))
			nomem(argv0);
		qmail_put(&qqt, quoted.s, quoted.len);
		qmail_puts(&qqt, "@");
		qmail_put(&qqt, bouncehost.s, bouncehost.len);
		qmail_puts(&qqt, "\nTo: ");
		while (!quote2(&quoted, bouncerecip))
			nomem(argv0);
		qmail_put(&qqt, quoted.s, quoted.len);
#ifdef MIME
		/*- MIME header with boundary */
		qmail_puts(&qqt, "\nMIME-Version: 1.0\n" "Content-Type: multipart/mixed; " "boundary=\"");
		while (!stralloc_copyb(&boundary, strnum1, fmt_ulong(strnum1, birth)))
			nomem(argv0);
		while (!stralloc_cat(&boundary, &bouncehost))
			nomem(argv0);
		while (!stralloc_catb(&boundary, strnum1, fmt_ulong(strnum1, id)))
			nomem(argv0);
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
			qmail_puts(&qqt, "Hi. This is the ");
			qmail_puts(&qqt, argv0);
			qmail_puts(&qqt, " program at ");
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
				nomem(argv0);
			substdio_fdbuf(&ssread, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof (inbuf));
			while ((r = substdio_get(&ssread, buf, sizeof (buf))) > 0) {
				while (!stralloc_catb(&orig_recip, buf, r))
					nomem(argv0);
				qmail_put(&qqt, buf, r);
			}
			while (!stralloc_0(&orig_recip))
				nomem(argv0);
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
			nomem(argv0);
		qmail_put(&qqt, quoted.s, quoted.len);
		qmail_put(&qqt, ">\n", 2);
		if ((fd = open_read(fn1.s)) == -1)
			qmail_fail(&qqt);
		else {
			int             bytestogo = bouncemaxbytes;
			int             bytestoget = (bytestogo < sizeof buf) ? bytestogo : sizeof buf;
			substdio_fdbuf(&ssread, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof (inbuf));
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
				slog(1, "warn: ", argv0, ": ", queuedesc, ": unable to unlink ", fn2.s, ". Will try later\n", NULL);
				return 0;
			}
			slog(1, "info: ", argv0, ": delete bounce: discarding ", fn2.s, " ", queuedesc, "\n", NULL);
			return 1;
		default:
			qmail_fail(&qqt);
			break;
		}
		qmail_from(&qqt, bouncesender);
		qmail_to(&qqt, bouncerecip);
		if (*qmail_close(&qqt)) {
			slog(1, "warn: ", argv0, ": ", queuedesc, ": trouble injecting bounce message, will try later\n", NULL);
			return 0;
		}
		strnum1[fmt_ulong(strnum1, id)] = 0;
		slog(0, "bounce msg ", strnum1, NULL);
		strnum1[fmt_ulong(strnum1, qp)] = 0;
		slog(1, " qp ", strnum1, " ", queuedesc, "\n", NULL);
	}
	if (unlink(fn2.s) == -1) {
		slog(1, "warn: ", argv0, ": ", queuedesc, ": unable to unlink ", fn2.s, "\n", NULL);
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

	slog(0, "status:", NULL);
	for (c = 0; c < CHANNELS; ++c) {
		strnum1[fmt_ulong(strnum1, (unsigned long) concurrencyused[c])] = 0;
		strnum2[fmt_ulong(strnum2, (unsigned long) concurrency[c])] = 0;
		slog(0, chanstatusmsg[c], strnum1, "/", strnum2, NULL);
		if (holdjobs[c]) /*NJL*/
			slog(0, " (held)", NULL); /*NJL*/
	}
	if (delayed_jobs) {
		strnum1[fmt_ulong(strnum1, delayed_jobs)] = 0;
		slog(0, " delayed jobs=", strnum1, " ", queuedesc, NULL);
	} else
		slog(0, " ", queuedesc, NULL);
	if (flagexitsend)
		slog(0, " exitasap", NULL);
	slog(0, "\n", NULL);
	flush();
}

static void
del_init()
{
	int             c, i;

	for (c = 0; c < CHANNELS; ++c) {
		flagspawnalive[c] = 1;
		while (!(del[c] = (struct DEL *) alloc(concurrency[c] * sizeof (struct DEL))))
			nomem(argv0);
		for (i = 0; i < concurrency[c]; ++i) {
			del[c][i].used = 0;
			del[c][i].recip.s = 0;
		}
		dline[c].s = 0;
		while (!stralloc_copys(&dline[c], ""))
			nomem(argv0);
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
#ifdef HASLIBRT
	int             q, offset;
#endif

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
		nomem(argv0);
		return;
	}
	del[c][i].j = j;
	++jo[j].refs;
	del[c][i].delid = masterdelid++;
	del[c][i].mpos = mpos;
	del[c][i].used = 1;
	++concurrencyused[c];
#ifdef HASLIBRT
	/*- write the concurrency used for local/remote at offset 0, 1 */
	offset = c * sizeof(int);
	q = concurrencyused[c];
	if (shm_queue != -1 && (lseek(shm_queue, offset, SEEK_SET) == -1 || write(shm_queue, (char *) &q, sizeof(int)) <= 0))
		slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to write to shared memory: ", error_str(errno), "\n", NULL);
#endif
	comm_write(c, i, jo[j].id, jo[j].sender.s, jo[j].qqeh.s, jo[j].envh.s, recip);
	strnum1[fmt_ulong(strnum1, del[c][i].delid)] = 0;
	strnum2[fmt_ulong(strnum2, jo[j].id)] = 0;
	if (qident)
		slog(0, "starting delivery ", strnum1, ".", qident, NULL);
	else
		slog(0, "starting delivery ", strnum1, NULL);
	slog(0, ": msg ", strnum2, tochan[c], NULL);
	logsafe_noflush(recip, argv0);
	slog(1, " ", queuedesc, "\n", NULL);
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
	slog(1, "warn: ", argv0, ": ", queuedesc, ": trouble marking ", fn1.s, "; message will be delivered twice!\n", NULL);
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
#ifdef HASLIBRT
	int             q, offset;
#endif

	if ((r = read(chanfdin[c], delbuf, sizeof (delbuf))) == -1)
		return;
	if (r == 0) {
		spawndied(c);
		return;
	}
	for (i = 0; i < r; ++i) {
		ch = delbuf[i];
		while (!stralloc_append(&dline[c], &ch))
			nomem(argv0);
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
				slog(1, "warn: ", argv0, ": ", queuedesc, ": internal error: delivery report out of range\n", NULL);
			else {
				if (dline[c].s[2] == 'Z') {
					if (jo[del[c][delnum].j].flagdying) {
						dline[c].s[2] = 'D';
						--dline[c].len;
						while (!stralloc_cats
							   (&dline[c], "I'm not going to try again; this message has been in the queue too long.\n"))
							nomem(argv0);
						while (!stralloc_0(&dline[c]))
							nomem(argv0);
					}
				}
				strnum1[fmt_ulong(strnum1, del[c][delnum].delid)] = 0;
				switch (dline[c].s[2])
				{
				case 'K':
					if (qident)
						slog(0, "delivery ", strnum1, ".", qident, ": success: ", NULL);
					else
						slog(0, "delivery ", strnum1, ": success: ", NULL);
					logsafe_noflush(dline[c].s + 3, argv0);
					slog(1, " ", queuedesc, "\n", NULL);
					markdone(c, jo[del[c][delnum].j].id, del[c][delnum].mpos);
					--jo[del[c][delnum].j].numtodo;
					break;
				case 'Z':
					if (qident)
						slog(0, "delivery ", strnum1, ".", qident, ": deferral: ", NULL);
					else
						slog(0, "delivery ", strnum1, ": deferral: ", NULL);
					logsafe_noflush(dline[c].s + 3, argv0);
					slog(1, " ", queuedesc, "\n", NULL);
					break;
				case 'D':
					if (qident)
						slog(0, "delivery ", strnum1, ".", qident, ": failure: ", NULL);
					else
						slog(0, "delivery ", strnum1, ": failure: ", NULL);
					logsafe_noflush(dline[c].s + 3, argv0);
					slog(1, " ", queuedesc, "\n", NULL);
					addbounce(jo[del[c][delnum].j].id, del[c][delnum].recip.s, dline[c].s + 3);
					markdone(c, jo[del[c][delnum].j].id, del[c][delnum].mpos);
					--jo[del[c][delnum].j].numtodo;
					break;
				default:
					slog(1, "delivery ", strnum1, ": report mangled, will defer: ", queuedesc, "\n", NULL);
				}
				job_close(del[c][delnum].j);
				del[c][delnum].used = 0;
				--concurrencyused[c];
#ifdef HASLIBRT
				/*- write the concurrency used for local/remote at offset 0, 1 */
				offset = c * sizeof(int);
				q = concurrencyused[c];
				if (shm_queue != -1 && (lseek(shm_queue, offset, SEEK_SET) == -1 || write(shm_queue, (char *) &q, sizeof(int)) <= 0))
					slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to write to shared memory: ", error_str(errno), "\n", NULL);
#endif
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

	if (flagexitsend)
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
	datetime_sec    n;

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

	if (flagexitsend)
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
		substdio_fdbuf(&pass[c].ss, (ssize_t (*)(int,  char *, size_t)) read, pass[c].fd, pass[c].buf, sizeof (pass[c].buf));
		job_open(pe.id, c, j);
		jo[j].retry = nextretry(birth, c);
		jo[j].flagdying = (recent > birth + lifetime);
#ifdef BOUNCELIFETIME
		if (!*line.s)
			jo[j].flagdying = (recent > birth + bouncelifetime);
#endif
		while (!stralloc_copy(&jo[j].sender, &line))
			nomem(argv0);
		while (!stralloc_copy(&jo[j].qqeh, &qqeh))
			nomem(argv0);
		while (!stralloc_copy(&jo[j].envh, &envh))
			nomem(argv0);
	}
	if (!del_avail(c))
		return;
	/*- read local/split/inode or remote/split/inode */
	if (getln(&pass[c].ss, &line, &match, '\0') == -1) {
		fnmake_chanaddr(pass[c].id, c);
		slog(1, "warn: ", argv0, ": ", queuedesc, ": trouble reading ", fn1.s, "; will try again later\n", NULL);
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
		delivery = (c == 0) ?  local_delivery : remote_delivery;
		/*-
		 * if the domain has delivery rate control, delivery_rate()
		 * will set _do_ratelimit
		 */
		if (!(i = delivery_rate(line.s + 1, pe.id, &t, &_do_ratelimit, argv0))) {
			/* we are rate controlled */
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
			slog(1, "warn: ", argv0, ": ", queuedesc, ": failed to get delivery rate for ", line.s + 1, "; proceeding to deliver\n", NULL);
		else /*- i == 1 */
		if (_do_ratelimit)
			delayed_jobs = delayed_job_count();
		++jo[pass[c].j].numtodo;
		del_start(pass[c].j, pass[c].mpos, line.s + 1); /*- line.s[1] = recipient */
		break;
	case 'D': /*- delivery done */
		break;
	default:
		fnmake_chanaddr(pass[c].id, c);
		slog(1, "warn: ", argv0, ": ", queuedesc, ": unknown record type in ", fn1.s, "!\n", NULL);
		close(pass[c].fd);
		job_close(pass[c].j);
		pass[c].id = 0;
		return;
	}
	pass[c].mpos += line.len;
	return;
trouble:
	slog(1, "warn: ", argv0, ": ", queuedesc, ": trouble opening ", fn1.s, "; will try again later\n", NULL);
	pe.dt = recent + SLEEP_SYSFAIL;
	while (!prioq_insert(min, &pqchan[c], &pe))
		nomem(argv0);
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
			slog(1, "warn: ", argv0, ": ", queuedesc, ": unable to stat ", fn1.s, "; will try again later\n", NULL);
			goto fail;
		}
	}
	fnmake_todo(id);
	if (stat(fn1.s, &st) == 0)
		return;
	if (errno != error_noent) {
		slog(1, "warn: ", argv0, ": ", queuedesc, ": unable to stat ", fn1.s, "; will try again later\n", NULL);
		goto fail;
	}
	fnmake_info(id);
	if (stat(fn1.s, &st) == -1) {
		if (errno == error_noent)
			return;
		slog(1, "warn: ", argv0, ": ", queuedesc, ": unable to stat ", fn1.s, "; will try again later\n", NULL);
		goto fail;
	}

	/*- -todo +info -local -remote ?bounce */
	if (!injectbounce(id))
		goto fail;	/*- injectbounce() produced error message */
	strnum1[fmt_ulong(strnum1, id)] = 0;
	slog(1, "end msg ", strnum1, " ", argv0, ": ", queuedesc, "\n", NULL);

	/*- -todo +info -local -remote -bounce */
	fnmake_info(id);
	if (unlink(fn1.s) == -1) {
		slog(1, "warn: ", argv0, ": ", queuedesc, ": unable to unlink ", fn1.s, "; will try again later\n", NULL);
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
		slog(1, "warn: ", argv0, ": ", queuedesc, ": qmail-clean unable to clean up ", fn1.s, "\n", NULL);
	return;
fail:
	pe.id = id;
	pe.dt = now() + SLEEP_SYSFAIL;
	while (!prioq_insert(min, &pqdone, &pe))
		nomem(argv0);
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

static void
sigusr1(int x)
{
	if (flagdetached)
		return;
	sig_block(sig_usr1);
	if (del_canexit()) {
		slog(1, "alert: ", argv0, ": ", queuedesc,
				": no pending deliveries. Instrucing todo-proc to detach...\n", NULL);
		/*- tell todo-proc to stop sending jobs */
		if (write(todofdo, "D", 1) != 1) {
			slog(1, "alert: ", argv0, ": ", queuedesc,
					": unable to write two bytes to todo-proc! dying...: ",
					error_str(errno), "\n", NULL);
			flagexitsend = 1;
			flagtodoalive = 0;
		} else
			flagdetached = 2;
		sig_unblock(sig_usr1);
		return;
	}
	flagdetached = 1;
	/*- tell todo-proc to stop sending jobs */
	if (write(todofdo, "D", 1) != 1) {
		slog(1, "alert: ", argv0, ": ", queuedesc,
				": unable to write two bytes to todo-proc! dying...: ",
				error_str(errno), "\n", NULL);
		flagexitsend = 1;
		flagtodoalive = 0;
	}
	sig_unblock(sig_usr1);
	if (!del_canexit())
		sig_block(sig_usr2);
}

static void
sigusr2(int x)
{
	if (flagdetached == 0)
		return;
	sig_block(sig_usr2);
	/*-
	 * we need to rescan todo if we were earlier in detached mode
	 * add jobs from todo
	 */
	pqstart();
	/*- tell todo-proc to start sending jobs */
	if (write(todofdo, "A", 1) != 1) {
		slog(1, "alert: ", argv0, ": ", queuedesc,
				": unable to write two bytes to todo-proc! dying...: ",
				error_str(errno), "\n", NULL);
		flagexitsend = 1;
		flagtodoalive = 0;
	} else
		flagdetached = 0;
	sig_unblock(sig_usr2);
	sig_unblock(sig_usr1);
}

static void
tododied()
{
	slog(1, "alert: ", argv0, ": ", queuedesc,
			": oh no! lost todo-proc connection! dying...\n", NULL);
	flagexitsend = 1;
	flagtodoalive = 0;
}

static void
todo_init()
{
	todofdo = 7; /*- write end pi7[1] */
	todofdi = 8;  /*- read end  pi8[0] */
	flagtodoalive = 1;
	/*- tell todo-proc to start */
	if (write(todofdo, "C", 1) != 1) {
		slog(1, "alert: ", argv0, ": ", queuedesc,
				": unable to write a byte to external todo! dying...: ",
				error_str(errno), "\n", NULL);
		flagexitsend = 1;
		flagtodoalive = 0;
	}
	return;
}

static void
todo_selprep(int *nfds, fd_set *rfds, datetime_sec *wakeup)
{
	if (flagexitsend && flagtodoalive) {
		if (write(todofdo, "X", 1)) ; /*- keep compiler happy */
		flagtodoalive = 0;
	}
	if (flagtodoalive) {
		FD_SET(todofdi, rfds); /*- read fd from todo-processor */
		if (*nfds <= todofdi)
			*nfds = todofdi + 1;
	}
}

/*
 * set flagchan for local or remote
 * set pqchan[c]
 * set pqdone
 */
static void
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
		slog(1, "warn: ", argv0, ": ", queuedesc, ": todo-proc speaks an obscure dialect\n", NULL);
		return;
	}
	len = scan_ulong(s, &id);
	if (!len || s[len]) {
		slog(1, "warn: ", argv0, ": ", queuedesc, ": todo-proc speaks an obscure dialect\n", NULL);
		return;
	}
	pe.id = id;
	pe.dt = now();
	/*- add to delivery queue */
	for (c = 0; c < CHANNELS; ++c) {
		if (flagchan[c])
			while (!prioq_insert(min, &pqchan[c], &pe))
				nomem(argv0);
	}
	/*- mark delivery as 'done' */
	for (c = 0; c < CHANNELS; ++c) {
		if (flagchan[c])
			break;
	}
	if (c == CHANNELS) {
		while (!prioq_insert(min, &pqdone, &pe))
			nomem(argv0);
	}
	return;
}

/*
 * if data is available fd 8 for read
 * read data
 * 'D' - set up delivery by calling todo_del()
 * 'L' - write to log (fd 0) for logging
 * 'X' - set flagtodoalive = 0, flagexitsend = 1
 */
static void
todo_do(fd_set *rfds)
{
	int             r, i;
	char            ch;

	if (!flagtodoalive)
		return;
	if (!FD_ISSET(todofdi, rfds))
		return;

	if ((r = read(todofdi, todobuf, sizeof (todobuf))) == -1)
		return;
	if (r == 0) {
		if (flagexitsend)
			flagtodoalive = 0;
		else
			tododied(); /*- sets flagexitsend, flagtodoalive */
		return;
	}
	for (i = 0; i < r; ++i) {
		ch = todobuf[i];
		while (!stralloc_append(&todoline, &ch))
			nomem(argv0);
		if (todoline.len > REPORTMAX)
			todoline.len = REPORTMAX;
		/*-
		 * todo-proc is responsible for keeping it short
		 * e.g. todo-proc writes a line like this
		 * local  DL656826\0
		 * remote DR656826\0
		 * both   DB656826\0
		 */
		if (!ch && (todoline.len > 1)) {
			switch (todoline.s[0])
			{
			case 'D': /*- message to be delivered */
				if (flagexitsend)
					break;
				todo_del(todoline.s + 1);
				break;
			case 'L': /*- write to log */
				slog(1, todoline.s + 1, NULL);
				break;
			case 'X': /*- todo-proc is exiting */
				if (flagexitsend)
					flagtodoalive = 0;
				else
					tododied(); /*- sets flagexitsend, flagtodoalive */
				break;
			default:
				slog(1, "warn: ", argv0, ": ", queuedesc, ": todo-proc speaks an obscure dialect\n", NULL);
				break;
			}
			todoline.len = 0;
		}
	} /*- for (i = 0; i < r; ++i) */
}

static int
getcontrols()
{
	int             qregex;
	char           *x;

	if (control_init() == -1)
		return 0;
	if (control_readint((int *) &lifetime, "queuelifetime") == -1)
		return 0;
	if (!(x = env_get("QREGEX"))) {
		if (control_readint(&qregex, "qregex") == -1)
			return 0;
		if (qregex && !env_put("QREGEX=1"))
			return 0;
	}
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
	if (control_readint(&use_fdatasync, "conf-fdatasync") == -1)
		return 0;
#endif
#ifdef HAVESRS
	if (control_readline(&srs_domain, "srs_domain") == -1)
		return 0;
	if (srs_domain.len && !stralloc_0(&srs_domain))
		return 0;
#endif
	if (control_readint(&bouncemaxbytes, "bouncemaxbytes") == -1)
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
 * Another way could be to set flagexitsend to 1
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
		slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/locals\n", NULL);
		return;
	}
	if ((r = control_readfile(&newvdoms, "virtualdomains", 0)) == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/virtualdomains\n", NULL);
		return;
	}
	if (control_readint((int *) &concurrency[0], "concurrencylocal") == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/concurrencylocal\n", NULL);
		return;
	}
	if (control_readint((int *) &concurrency[1], "concurrencyremote") == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/concurrencyremote\n", NULL);
		return;
	}
	/*- Add "holdlocal/holdremote" flags - NJL 1998/05/03 */
	if (control_readint(&newholdjobs[0], "holdlocal") == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/holdlocal\n", NULL);
		return;
	}
	if (control_readint(&newholdjobs[1], "holdremote") == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/holdremote\n", NULL);
		return;
	}
	if (control_rldef(&envnoathost, "envnoathost", 1, "envnoathost") != 1) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/envnoathost\n", NULL);
		return;
	}
#ifdef USE_FSYNC
	if (control_readint(&use_syncdir, "conf-syncdir") == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/conf-syncdir\n", NULL);
		return;
	}
	if (control_readint(&use_fsync, "conf-fsync") == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/conf-fsync\n", NULL);
		return;
	}
	if (control_readint(&use_fdatasync, "conf-fdatasync") == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/conf-fdatasync\n", NULL);
		return;
	}
#endif
	for (c = 0; c < CHANNELS; c++) {
		if (holdjobs[c] != newholdjobs[c]) {
			holdjobs[c] = newholdjobs[c];
			if (holdjobs[c])
				slog(1, chanjobsheldmsg[c], " ", queuedesc, NULL);
			else {
				slog(1, chanjobsunheldmsg[c], " ", queuedesc, NULL);
				flagrunasap = 1; /*- run all jobs now */
			}
		}
	}
	constmap_free(&maplocals);
	while (!stralloc_copy(&locals, &newlocals))
		nomem(argv0);
	while (!constmap_init(&maplocals, locals.s, locals.len, 0))
		nomem(argv0);
	constmap_free(&mapvdoms);
	if (r) {
		while (!stralloc_copy(&vdoms, &newvdoms))
			nomem(argv0);
		while (!constmap_init(&mapvdoms, vdoms.s, vdoms.len, 1))
			nomem(argv0);
	} else
		while (!constmap_init(&mapvdoms, "", 0, 1))
			nomem(argv0);
}

static void
reread(int hupflag)
{
	if (chdir(auto_qmail) == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc,
				": unable to reread controls: unable to switch to ",
				auto_qmail, ": ", error_str(errno), "\n", NULL);
		return;
	}
	if (hupflag && write(todofdo, "H", 1) != 1) {
		slog(1, "alert: ", argv0, ": ", queuedesc,
				": unable to write a byte to external todo: ",
				error_str(errno), "\n", NULL);
		return;
	}
	regetcontrols();
	chdir_toqueue();
}

stralloc        plugin = { 0 }, splugin = {0};

void
run_plugin()
{
	/*- startup plugins */
	void           *handle;
	int             i, status = 0, len;
	int             (*func) (void);
	char           *ptr, *plugin_ptr;
	const char     *error, *start_plugin, *plugin_symb, *plugindir, *end;

	if (!(plugindir = env_get("PLUGINDIR")))
		plugindir = "plugins";
	if (plugindir[i = str_chr(plugindir, '/')]) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": plugindir cannot have an absolute path\n", NULL);
		_exit(111);
	}
	if (!(plugin_symb = env_get("START_PLUGIN_SYMB")))
		plugin_symb = "startup";
	if (!(start_plugin = env_get("START_PLUGIN")))
		start_plugin = "qmail-send.so";
	if (!stralloc_copyb(&splugin, start_plugin, (len = str_len(start_plugin))))
		nomem(argv0);
	if (!stralloc_0(&splugin))
		nomem(argv0);
	end = splugin.s + len;
	for (ptr = plugin_ptr = splugin.s;; ptr++) {
		if (*ptr != ' ' && ptr != end)
			continue;
		if (ptr != end)
			*ptr = 0;
		if (!stralloc_copys(&plugin, auto_qmail))
			nomem(argv0);
		if (!stralloc_append(&plugin, "/"))
			nomem(argv0);
		if (!stralloc_cats(&plugin, plugindir))
			nomem(argv0);
		if (!stralloc_append(&plugin, "/"))
			nomem(argv0);
		if (!stralloc_cats(&plugin, plugin_ptr))
			nomem(argv0);
		if (!stralloc_0(&plugin))
			nomem(argv0);
		if (ptr != end)
			plugin_ptr = ptr + 1;
		if (access(plugin.s, F_OK)) {
			if (errno != error_noent) {
				slog(1, "alert: ", argv0, ": ", queuedesc,
						": unable to access: ", plugin.s, ": ", error_str(errno), "\n", NULL);
				_exit(111);
			}
			if (ptr == end)
				break;
			else
				continue;
		}
		if (!(handle = dlopen(plugin.s, RTLD_LAZY | RTLD_GLOBAL))) {
			slog(1, "alert: ", argv0, ": ", queuedesc, ": dlopen failed for ", plugin.s, ": ", dlerror(), "\n", NULL);
			_exit(111);
		}
		dlerror(); /*- man page told me to do this */
		func = dlsym(handle, plugin_symb);
		if ((error = dlerror())) {
			slog(1, "alert: ", argv0, ": ", queuedesc, ": dlsym ", plugin_symb, " failed: ", error, "\n", NULL);
			_exit(111);
		}
		slog(1, "info: ", argv0, ": ", queuedesc, ": executing function ", plugin_symb, "\n", NULL);
		if ((status = (*func) ())) {
			strnum1[fmt_ulong(strnum1, status)] = 0;
			slog(1, "alert: ", argv0, ": ", queuedesc, ": function ", plugin_symb, " failed with status ", strnum1, "\n", NULL);
		}
		if (dlclose(handle)) {
			slog(1, "alert: ", argv0, ": ", queuedesc, ": dlclose for ", plugin.s, "failed: ", error, "\n", NULL);
			_exit(111);
		}
		if (ptr == end)
			break;
		if (status)
			break;
	}
	if (status)
		_exit(status);
}

#ifdef HASLIBRT
void
shm_init(const char *shm_name)
{
	int             q[5];

	/*- open shm /queueN for storing concurrency values */
#ifdef FREEBSD
	shm_queue = shm_open(shm_name, O_RDWR, 0600);
#else
	shm_queue = shm_open(shm_name, O_WRONLY, 0600);
#endif
	if (shm_queue == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": failed to open POSIX shared memory ", shm_name, ": ", error_str(errno), "\n", NULL);
		_exit(111);
	}
	/*-
	 * q[0] - concurrencyusedlocal
	 * q[1] - concurrencyusedremote
	 * q[2] - concurrencylocal
	 * q[3] - concurrencyremote
	 * q[4] - 0 enabled, 1 disabled
	 *        on startup, qmail-send will always set this to 0 (enabled)
	 *        future version will use the mode of the queue base directory
	 *        to set it to enabled/disabled state
	 */
	q[0] = q[1] = 0;
	q[2] = concurrency[0];
	q[3] = concurrency[1];
	q[4] = 0;
#ifdef FREEBSD /*- another FreeBSD idiosyncrasies */
	if (ftruncate(shm_queue, getpagesize()) < 0) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to truncate shared memory: ", error_str(errno), "\n", NULL);
		_exit(111);
	}
#endif
	if (write(shm_queue, (char *) q, 5 * sizeof(int)) == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to write to shared memory: ", error_str(errno), "\n", NULL);
		_exit(111);
	}
}
#endif

int
main(int argc, char **argv)
{
	int             lockfd, nfds, c, opt, can_exit;
	datetime_sec    wakeup;
	fd_set          rfds, wfds;
	struct timeval  tv;
	char           *ptr;

	mypid[fmt_ulong(mypid, getpid())] = 0;
	c = str_rchr(argv[0], '/');
	argv0 = (argv[0][c] && argv[0][c + 1]) ? argv[0] + c + 1 : argv[0];
	if (!(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue"; /*- single queue like qmail */
	ptr = env_get("RATELIMIT_DIR");
	do_ratelimit = (ptr && *ptr) ? 1 : 0;
	/*- get basename of queue directory to define qmail-send instance */
	for (queuedesc = queuedir; *queuedesc; queuedesc++);
	for (; queuedesc != queuedir && *queuedesc != '/'; queuedesc--);
	if (*queuedesc == '/')
		queuedesc++;
	if (chdir(auto_qmail) == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": cannot start: unable to switch to ", auto_qmail, ": ", error_str(errno), "\n", NULL);
		_exit(111);
	}
#ifdef LOGLOCK
	loglock_open(argv0, 0);
#endif
	qident = env_get("QIDENT");
	getEnvConfigInt(&bigtodo, "BIGTODO", 1);
	getEnvConfigInt(&conf_split, "CONFSPLIT", auto_split);
	if (conf_split > auto_split)
		conf_split = auto_split;
	strnum1[fmt_ulong(strnum1, conf_split)] = 0;
	slog(1, "info: ", argv0, ": pid ", mypid, ": ", queuedir, ": ratelimit=",
			do_ratelimit ? "ON, loglock=" : "OFF, loglock=",
			loglock_fd == -1 ? "disabled, conf split=" : "enabled, conf split=",
			strnum1, "\n", NULL);
#ifdef USE_FSYNC
	ptr = env_get("USE_FSYNC");
	use_fsync = (ptr && *ptr) ? 1 : 0;
	ptr = env_get("USE_FDATASYNC");
	use_fdatasync = (ptr && *ptr) ? 1 : 0;
	ptr = env_get("USE_SYNCDIR");
	use_syncdir = (ptr && *ptr) ? 1 : 0;
	if (use_syncdir > 0) {
		while (!env_put2("USE_SYNCDIR", "1"))
			nomem(argv0);
	} else
	if (!use_syncdir) {
		while (!env_unset("USE_SYNCDIR"))
			nomem(argv0);
	}
	if (use_fsync > 0) {
		while (!env_put2("USE_FSYNC", "1"))
			nomem(argv0);
	} else
	if (!use_fsync) {
		while (!env_unset("USE_FSYNC"))
			nomem(argv0);
	}
	if (use_fdatasync > 0) {
		while (!env_put2("USE_FDATASYNC", "1"))
			nomem(argv0);
	} else
	if (!use_fdatasync) {
		while (!env_unset("USE_FDATASYNC"))
			nomem(argv0);
	}
#endif
	if (!getcontrols()) {
		slog(1, "alert: ", argv0, ": ", queuedesc,
				": cannot start: unable to read controls\n", NULL);
		_exit(111);
	}
	run_plugin();
	if (chdir(queuedir) == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc,
				": cannot start: unable to switch to queue directory ",
				queuedir, ": ", error_str(errno), "\n", NULL);
		_exit(111);
	}
	sig_pipeignore();
	sig_termcatch(sigterm);
	sig_alarmcatch(sigalrm);
	sig_hangupcatch(sighup);
	sig_childdefault();
#ifdef LOGLOCK
	sig_intcatch(sigint);
#endif
	sig_block(sig_usr1);
	sig_block(sig_usr2);
	sig_catch(sig_usr1, sigusr1);
	sig_catch(sig_usr2, sigusr2);
	umask(077);

	while ((opt = getopt(argc, argv, "cds")) != opteof) {
		switch (opt)
		{
			case 'd':
#ifndef HASLIBRT
				slog(1, "alert: ", argv0, ": ", queuedesc, ": dynamic queue not supported\n", NULL);
				_exit(100);
#endif
				dynamic_queue = 1;
				break;
			case 's':
				dynamic_queue = 0;
				break;
		}
	}
	argc -= optind;
	argv += optind;

	/*- prevent multiple copies of qmail-send to run */
	if ((lockfd = open_write("lock/sendmutex")) == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": cannot start: unable to open mutex\n", NULL);
		_exit(111);
	}
	if (lock_exnb(lockfd) == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": cannot start: instance already running\n", NULL);
		_exit(111);
	}

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
			slog(1, "alert: ", argv0, ": ", queuedesc, ": cannot start: hath the daemon spawn no fire?\n", NULL);
			_exit(111);
		}
		do {
			r = read(chanfdin[c], &ch2, 1);
		} while ((r == -1) && (errno == error_intr));
		if (r < 1) {
			slog(1, "alert: ", argv0, ": ", queuedesc, ": cannot start: hath the daemon spawn no fire?\n", NULL);
			_exit(111);
		}
		u = (unsigned int) (unsigned char) ch1;
		u += (unsigned int) ((unsigned char) ch2) << 8;
		if (concurrency[c] > u)
			concurrency[c] = u;
		numjobs += concurrency[c];
	} /*- for (c = 0; c < CHANNELS; ++c) */
#ifdef HASLIBRT
	if (dynamic_queue)
		shm_init(queuedesc - 1);
#endif
	fnmake_init();  /*- initialize fn1, fn2 */
	comm_init();    /*- assign fd 5 to queue comm to, 6 to queue comm from */
	pqstart();      /*- add files from info/split for processing */
	job_init();     /*- initialize numjobs job structures */
	del_init();     /*- initialize concurrencylocal + concurrrencyremote delivery structure */
	pass_init();    /*- initialize pass structure */
	todo_init();    /*- set fd 7 to write to todo-proc, set fd 8 to read from todo-proc, write 'S' to todo-proc */
	cleanup_init(); /*- initialize flagcleanup = 0, cleanuptime = now*/
	sig_unblock(sig_usr1);
	sig_unblock(sig_usr2);
	can_exit = del_canexit();
	while (!flagexitsend || !can_exit || flagtodoalive) { /*- stay alive if delivery jobs, todo jobs are present */
		recent = now();
		if (flagrunasap) {
			flagrunasap = 0;
			pqrun();
		}
		if (flagreadasap) {
			flagreadasap = 0;
			reread(1);
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
		 * set todofdi = 8 for select: read end from todo-proc pi8[0]
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
			if (errno == error_intr) {
				if (flagexitsend)
					break;
			} else {
				if (errno == error_invalid)
					tv.tv_sec = 0;
				slog(1, "warn: ", argv0, ": ", queuedesc, ": trouble in select\n", NULL);
			}
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
			/* read data from fd 8 from todo-proc
			 * 'D' - do delivery
			 * 'L' - write to log (fd 0) for logging
			 * 'X' - set flagtodoalive = 0, flagexitsend = 1
			 */
			todo_do(&rfds);
			/*-
			 * add/del priority queue
			 * mark delivered/bounced messages as done
			 */
			pass_do();
			/*-
			 * communicate with qmail-clean for cleanup
			 */
			cleanup_do();
		}
		can_exit = del_canexit();
		if (can_exit && flagdetached == 1) {
			slog(1, "info: ", argv0, ": ", queuedesc,
					": no pending jobs, attaching back to todo-proc\n", NULL);
			pqstart();
			/*- tell todo-proc to start sending jobs */
			if (write(todofdo, "A", 1) != 1) {
				slog(1, "alert: ", argv0, ": ", queuedesc,
						": unable to write two bytes to todo-proc! dying...: ",
						error_str(errno), "\n", NULL);
				flagexitsend = 1;
				flagtodoalive = 0;
			} else
				flagdetached = 0;
			sig_unblock(sig_usr2);
			sig_unblock(sig_usr1);
		}
	} /*- while (!flagexitsend || !can_exit || flagtodoalive) */
	pqfinish();
	slog(1, "info: ", argv0, ": pid ", mypid, " ", queuedesc, " exiting\n", NULL);
#ifdef HASLIBRT
	shm_unlink(queuedesc);
#endif
	return (0);
}

void
getversion_qmail_send_c()
{
	const char     *x = "$Id: qmail-send.c,v 1.119 2025-05-06 22:39:16+05:30 Cprogrammer Exp mbhangui $";

	x = sccsiddelivery_rateh;
	x = sccsidgetdomainth;
	if (x)
		x++;
}

/*
 * $Log: qmail-send.c,v $
 * Revision 1.119  2025-05-06 22:39:16+05:30  Cprogrammer
 * reset tv.tv_sec on EINVAL
 *
 * Revision 1.118  2025-05-03 11:03:47+05:30  Cprogrammer
 * reorganized code
 *
 * Revision 1.117  2025-01-22 00:30:35+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.116  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.115  2024-02-08 20:48:32+05:30  Cprogrammer
 * fixed multiplication result converted to larger type (codeql)
 *
 * Revision 1.114  2023-12-30 09:21:30+05:30  Cprogrammer
 * return from exit_todo if flagtodoalive is 0
 *
 * Revision 1.113  2023-12-25 09:30:51+05:30  Cprogrammer
 * made OSSIFIED configurable
 *
 * Revision 1.112  2023-12-23 23:58:18+05:30  Cprogrammer
 * log pid during startup
 *
 * Revision 1.111  2023-12-23 20:19:34+05:30  Cprogrammer
 * terminate todo process when spawn/clean process terminates
 *
 * Revision 1.110  2023-12-21 19:49:08+05:30  Cprogrammer
 * added concept of half-detached/full-detached
 *
 * Revision 1.109  2023-10-30 10:28:44+05:30  Cprogrammer
 * use qregex control file to set QREGEX env variable
 *
 * Revision 1.108  2023-10-02 22:50:35+05:30  Cprogrammer
 * fix copy of srs_result
 *
 * Revision 1.107  2023-01-15 12:35:40+05:30  Cprogrammer
 * use slog() function with varargs to log error messages
 *
 * Revision 1.106  2022-09-27 12:48:44+05:30  Cprogrammer
 * auto attach to todo-processor when there are no pending delivery jobs
 *
 * Revision 1.105  2022-09-25 23:55:34+05:30  Cprogrammer
 * added feature to disconnect from todo-proc
 *
 * Revision 1.104  2022-04-21 21:34:31+05:30  Cprogrammer
 * fixed bigtodo setting for todofn
 *
 * Revision 1.103  2022-04-16 01:31:08+05:30  Cprogrammer
 * set delayed flag to 0 for new jobs
 *
 * Revision 1.102  2022-04-13 19:36:45+05:30  Cprogrammer
 * added feature to disable a queue and skip disabled queues.
 *
 * Revision 1.101  2022-04-12 08:49:49+05:30  Cprogrammer
 * display queuedir in logs on startup
 *
 * Revision 1.100  2022-04-12 08:37:11+05:30  Cprogrammer
 * added pid in logs
 *
 * Revision 1.99  2022-04-04 11:15:36+05:30  Cprogrammer
 * added setting of fdatasync() instead of fsync()
 *
 * Revision 1.98  2022-03-20 22:59:00+05:30  Cprogrammer
 * added queue ident in logs for matchup (qmailanalog to handle multi-queue)
 *
 * Revision 1.97  2022-03-06 11:05:09+05:30  Cprogrammer
 * open shm with O_RDWR for FreeBSD
 * use ftruncate on shm for FreeBSD
 *
 * Revision 1.96  2022-03-02 07:59:20+05:30  Cprogrammer
 * use ipc - added qscheduler, removed qmail-daemon
 * added haslibrt.h to configure dynamic queue
 * added compat mode option (support trigger mode in ipc mode)
 * call shm_init after setting concurrency
 * allow configurable big/small todo/intd
 * fixed signal sent to child
 * display pid when exiting
 * removed usless sleep
 *
 * Revision 1.96  2022-01-30 08:45:41+05:30  Cprogrammer
 * use ipc - added qscheduler, removed qmail-daemon
 * added haslibrt.h to configure dynamic queue
 * added compat mode option (support trigger mode in ipc mode)
 * call shm_init after setting concurrency
 * allow configurable big/small todo/intd
 * fixed signal sent to child
 * display pid when exiting
 * removed usless sleep
 *
 * Revision 1.95  2021-11-02 22:30:07+05:30  Cprogrammer
 * use argv0 for program name
 *
 * Revision 1.94  2021-10-22 13:58:12+05:30  Cprogrammer
 * change for ident argument to loglock_open()
 *
 * Revision 1.93  2021-10-20 22:36:22+05:30  Cprogrammer
 * display program 'qmail-send' in logs for identification
 *
 * Revision 1.92  2021-08-28 23:07:00+05:30  Cprogrammer
 * moved dtype enum delivery variable from variables.h to getDomainToken.h
 *
 * Revision 1.91  2021-08-13 18:25:59+05:30  Cprogrammer
 * turn off ratelimit if RATELIMIT_DIR is set but empty
 *
 * Revision 1.90  2021-07-26 23:24:22+05:30  Cprogrammer
 * log when log sighup, sigalrm is caught
 *
 * Revision 1.89  2021-07-17 14:37:07+05:30  Cprogrammer
 * skip processing of for messages queued with wrong split dir
 *
 * Revision 1.88  2021-07-15 22:37:28+05:30  Cprogrammer
 * corrected data type of comm_pos to int
 *
 * Revision 1.87  2021-07-15 13:23:04+05:30  Cprogrammer
 * organize bounce related control files together
 *
 * Revision 1.86  2021-06-27 10:48:06+05:30  Cprogrammer
 * moved conf_split variable to fmtqfn.c
 * fixed error handling in injectbounce
 *
 * Revision 1.85  2021-06-23 13:05:13+05:30  Cprogrammer
 * moved log_stat function to qsutil.c
 *
 * Revision 1.84  2021-06-16 20:02:05+05:30  Cprogrammer
 * added code comments
 *
 * Revision 1.84  2021-06-05 22:42:29+05:30  Cprogrammer
 * added code comments
 *
 * Revision 1.83  2021-06-05 18:02:11+05:30  Cprogrammer
 * corrected log message
 *
 * Revision 1.82  2021-06-05 17:47:11+05:30  Cprogrammer
 * reduce wakeup when time_needed is earlier than wakeup
 *
 * Revision 1.81  2021-06-05 12:57:11+05:30  Cprogrammer
 * refactored rate limit code to display delayed job count in logs
 *
 * Revision 1.80  2021-06-03 18:06:25+05:30  Cprogrammer
 * use new prioq functions
 *
 * Revision 1.79  2021-06-01 01:52:23+05:30  Cprogrammer
 * moved delivery_rate() to delivery_rate.c
 *
 * Revision 1.78  2021-05-30 00:12:42+05:30  Cprogrammer
 * added email rate limiting feature
 *
 * Revision 1.77  2021-05-16 12:16:14+05:30  Cprogrammer
 * limit conf_split to compile time value in conf-split
 * added code comments
 *
 * Revision 1.76  2021-05-12 15:40:54+05:30  Cprogrammer
 * added code comments
 *
 * Revision 1.75  2021-04-05 07:19:23+05:30  Cprogrammer
 * converted local variables/functions to static
 *
 * Revision 1.74  2020-11-24 13:47:27+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.73  2020-09-30 20:39:41+05:30  Cprogrammer
 * Darwin port for syncdir
 *
 * Revision 1.72  2020-09-16 19:05:34+05:30  Cprogrammer
 * fix compiler warning for FreeBSD
 *
 * Revision 1.71  2020-09-15 21:45:16+05:30  Cprogrammer
 * unset USE_SYNCDIR, USE_FSYNC only when use_syncdir, use_fsync is zero
 *
 * Revision 1.70  2020-09-15 21:09:07+05:30  Cprogrammer
 * use control files conf-fsync, conf-syncdir to turn on fsync, bsd style syncdir semantics
 * set / unset USE_FSYNC, USE_SYNCDIR env variables
 *
 * Revision 1.69  2020-07-04 22:23:52+05:30  Cprogrammer
 * replaced utime() with utimes()
 *
 * Revision 1.68  2020-05-19 10:33:59+05:30  Cprogrammer
 * define use_fsync for non-external todo-proc
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
 * This has the following patches
 * big concurrency by "Johannes Erdfelt" / Daemeon Reiydelle
 * big-todo by Russ Nelson
 * external-todo patch Andre Oppermann
 * ext-todo + big-todo Andreas Aardal Hanssen
 * Frank Denis aka Jedi/Sector One <j at 4u.net> - Patched qmail-send to
 *  - limit the size of bounces (bouncemaxbytes)
 *  - MAXRECIPIENT
 *  - maxhop control file
 */
