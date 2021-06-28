/*
 * $Log: $
 */
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <open.h>
#include <stralloc.h>
#include <fmt.h>
#include <scan.h>
#include <env.h>
#include <error.h>
#include <substdio.h>
#include <getln.h>
#include <constmap.h>
#include <byte.h>
#include <getEnvConfig.h>
#include <select.h>
#include <now.h>
#include <datetime.h>
#include <seek.h>
#include <alloc.h>
#include <str.h>
#include <sig.h>
#include <fd.h>
#include <ndelay.h>
#include <lock.h>
#include <sgetopt.h>
#include <strerr.h>
#include <wait.h>
#include "newfield.h"
#include "quote.h"
#include "qmail.h"
#include "prioq.h"
#include "control.h"
#include "qsutil.h"
#include "fmtqfn.h"
#include "readsubdir.h"
#include "trigger.h"
#include "prot.h"
#ifdef HAVESRS
#include "srs.h"
#endif
#include "variables.h"
#include "auto_qmail.h"
#include "auto_split.h"
#include "auto_uids.h"

#define OSSIFIED      129600 /*- 36 hours; _must_ exceed q-q's DEATH (24 hours) */
#define SLEEP_CLEANUP 76431 /*- time between cleanups */
#define SLEEP_SYSFAIL 123
#define SLEEP_FUZZ    1 /*- slop a bit on sleeps to avoid zeno effect */
#define SLEEP_FOREVER 86400 /*- absolute maximum time spent in select() */
#ifndef TODO_INTERVAL
#define SLEEP_TODO    60 /*- check todo/ every 25 minutes in any case */
#define ONCEEVERY     10 /*- Run todo maximal once every N seconds */
#endif
#define REPORTMAX     10000
#define FATAL         "qmta-sent: fatal: "

int             qmail_lspawn(int, char **);
int             qmail_rspawn(int, char **);

char           *queuedesc;
static int      lifetime = 604800;
static int      flagexitasap = 0, flagrunasap = 1, flagreadasap = 0;
static int      qmail_clean, _qmail_lspawn, _qmail_rspawn;;
static readsubdir todosubdir;
static datetime_sec nexttodorun, lasttodorun;
static int      flagtododir = 0;	/*- if 0, have to readsubdir_init again */
static int      todo_interval = -1;
static stralloc fn1 = { 0 };
static stralloc fn2 = { 0 };
static stralloc envnoathost = { 0 };
static stralloc percenthack = { 0 };
static stralloc locals = { 0 };
static stralloc vdoms = { 0 };
typedef struct constmap cmap;
static cmap     mappercenthack;
static cmap     maplocals;
static cmap     mapvdoms;
static char     strnum1[FMT_ULONG];
static char     strnum2[FMT_ULONG];
datetime_sec    recent;

#define CHANNELS 2
static int      chanfdout[CHANNELS];
static int      chanfdin[CHANNELS];
static int      chanskip[CHANNELS] = { 10, 20 };
static char    *chanaddr[CHANNELS] = { "local/", "remote/" };
static char    *chanstatusmsg[CHANNELS] = { " local ", " remote " };
static char    *tochan[CHANNELS] = { " to local ", " to remote " };
static prioq    pqdone = { 0 };						/*- -todo +info; HOPEFULLY -local -remote */
static prioq    pqchan[CHANNELS] = { {0} , {0} };	/*- pqchan 0: -todo +info +local ?remote */
													/*- pqchan 1: -todo +info ?local +remote */
static prioq    pqfail = { 0 };						/*- stat() failure; has to be pqadded again */

static substdio sstoqc;
static substdio ssfromqc;
static char     sstoqcbuf[1024];
static char     ssfromqcbuf[1024];
static stralloc comm_buf[CHANNELS] = { {0}, {0} };
static char     comm_pos[CHANNELS];

static int      bouncemaxbytes = 50000;
#ifdef BOUNCELIFETIME
static int      bouncelifetime = 604800;
#endif
static stralloc bouncemessage = { 0 };
static stralloc bouncesubject = { 0 };
static stralloc doublebouncemessage = { 0 };
static stralloc doublebouncesubject = { 0 };
static stralloc bouncefrom = { 0 };
static stralloc bouncehost = { 0 };
static stralloc doublebounceto = { 0 };
static stralloc doublebouncehost = { 0 };
char           *(qlargs[]) = { "qmail-lspawn", "./Maildir/", 0};
char           *(qrargs[]) = { "qmail-rspawn", 0};
char           *(qcargs[]) = { "qmail-clean", 0};
char           *(qfargs[]) = { "queue-fix", "-s", 0, 0, 0};
static int      flagspawnalive[CHANNELS];
static int      flagcleanup;	/*- if 1, cleanupdir is initialized and ready */
static readsubdir cleanupdir;
static datetime_sec cleanuptime;
static pid_t    cleanuppid;
#ifdef HAVESRS
static stralloc srs_domain = { 0 };
#endif

static void
cleandied()
{
	log1("alert: oh no! lost qmail-clean connection! dying...\n");
	flagexitasap = 1;
}

static void
spawndied(int c)
{
	log1("alert: oh no! lost spawn connection! dying...\n");
	flagspawnalive[c] = 0;
	flagexitasap = 1;
}

static void
chdir_toqueue()
{
	if (!queuedir && !(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue/qmta";
	while (chdir(queuedir) == -1) {
		log3("alert: unable to switch back to queue directory; HELP! sleeping...", error_str(errno), "\n");
		sleep(10);
	}
}

static void
sigterm()
{
	flagexitasap = 1;
}

void sigalrm()
{
	flagrunasap = 1;
}

void sighup()
{
	flagreadasap = 1;
}

#ifdef LOGLOCK
static void
sigint()
{
	if (loglock_fd == -1) {
		if (chdir(auto_qmail) == -1) {
			log5("alert: unable to reread controls: unable to switch to ", auto_qmail, ": ", error_str(errno), "\n");
			return;
		}
		loglock_open(1);
		chdir_toqueue();
	} else {
		sig_block(sig_int);
		sig_block(sig_child);
		sig_block(sig_hangup);
		close(loglock_fd);
		loglock_fd = -1;
		sig_unblock(sig_int);
		sig_unblock(sig_child);
		sig_unblock(sig_hangup);
		log1(loglock_fd == -1 ? "loglock: disabled\n" : "loglock: enabled\n");
	}
}
#endif

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
fnmake_intd(unsigned long id)
{
	fn1.len = fmtqfn(fn1.s, "intd/", id, 1);
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
cleanup_do(fd_set *wfds)
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
		/*- flow through */
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
	if (!qmail_clean) {
		fnmake_intd(id);
		if (unlink(fn1.s) == -1) {
			log5("warning: unable to unlink ", fn1.s, "; will try again later: ", error_str(errno), "\n");
			return;
		}
		fnmake_mess(id);
		if (unlink(fn1.s) == -1) {
			log5("warning: unable to unlink ", fn1.s, "; will try again later: ", error_str(errno), "\n");
			return;
		}
	} else {
		fnmake_foop(id); /*- remove intd and mess */
		if (substdio_putflush(&sstoqc, fn1.s, fn1.len) == -1) {
			cleandied();
			return;
		}
		if (substdio_get(&ssfromqc, &ch, 1) != 1) {
			cleandied();
			return;
		}
		if (ch != '+')
			log3("warning: qmail-clean unable to clean up ", fn1.s, "\n");
	}
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

static void
pqadd(unsigned long id)
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
			pechan[c].dt = st.st_mtime;
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
	log3("warning: unable to stat ", fn1.s, "; will try again later\n");
	pe.id = id;
	pe.dt = now() + SLEEP_SYSFAIL;
	while (!prioq_insert(min, &pqfail, &pe))
		nomem();
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
			pqadd(id);
	}
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
				log3("warning: unable to utime ", fn1.s, "; message will be retried too soon\n");
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

void
process_todo(unsigned long id)
{
	int             fd = -1, fdinfo = -1, match, c;
	int             fdchan[CHANNELS], flagchan[CHANNELS];
	uid_t           uid;
	pid_t           pid;
	substdio        ss, ssinfo;
	substdio        sschan[CHANNELS];
	char            todobuf[SUBSTDIO_INSIZE], todobufinfo[512], todobufchan[CHANNELS][1024];
	char            ch;
	static stralloc todoline = { 0 }, mailfrom = { 0 }, mailto = { 0 };
	struct stat     st;
	struct prioq_elt pe;

	for (c = 0; c < CHANNELS; ++c)
		fdchan[c] = -1;
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
			log5("warning: unable to unlink ", fn1.s, ": ", error_str(errno), "\n");
			goto fail;
		}
	}
	if ((fdinfo = open_excl(fn1.s)) == -1) {
		log3("warning: unable to create1 ", fn1.s, "\n");
		goto fail;
	}
	strnum1[fmt_ulong(strnum1, id)] = 0;
	log3("new msg ", strnum1, "\n");
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
			scan_uint(todoline.s + 1, &uid);
			break;
		case 'p':
			scan_int(todoline.s + 1, &pid);
			break;
		case 'h':
		case 'e':
			if (substdio_put(&ssinfo, todoline.s, todoline.len) == -1) {
				log3("warning: trouble writing to ", fn1.s, "\n");
				goto fail;
			}
			break;
		case 'F':
			if (substdio_put(&ssinfo, todoline.s, todoline.len) == -1) {
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
			log3_noflush(" uid ", strnum2, "\n");
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
					log3("warning: unable to create2 ", fn1.s, "\n");
					goto fail;
				}
				substdio_fdbuf(&sschan[c], write, fdchan[c], todobufchan[c], sizeof (todobufchan[c]));
				flagchan[c] = 1;
			}
			if (substdio_bput(&sschan[c], rwline.s, rwline.len) == -1) {
				fnmake_chanaddr(id, c);
				log3("warning: trouble writing to ", fn1.s, "\n");
				goto fail;
			}
			break;
		default:
			fnmake_todo(id);
			log3("warning: unknown record type in ", fn1.s, "\n");
			goto fail;
		}
	} /*- for (;;) */
	close(fd);
	fd = -1;
	if (substdio_flush(&ssinfo) == -1) {
		log3("warning: trouble writing to ", fn1.s, "\n");
		goto fail;
	}
	for (c = 0; c < CHANNELS; ++c) {
		if (fdchan[c] != -1) {
			if (substdio_flush(&sschan[c]) == -1) {
				fnmake_chanaddr(id, c);
				log3("warning: trouble writing to ", fn1.s, "\n");
				goto fail;
			}
		}
	}
#ifdef USE_FSYNC
	if (use_fsync > 0 && fsync(fdinfo) == -1) {
		log3("warning: trouble fsyncing ", fn1.s, "\n");
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
				log3("warning: trouble fsyncing ", fn1.s, "\n");
				goto fail;
			}
#endif
			close(fdchan[c]);
			fdchan[c] = -1;
		}
	}
	if (!qmail_clean) {
		fnmake_intd(id);
		if (unlink(fn1.s) == -1) {
			if (errno != error_noent) {
				log5("warning: unable to unlink ", fn1.s, ": ", error_str(errno), "\n");
				goto fail;
			}
		}
		fnmake_todo(id);
		if (unlink(fn1.s) == -1) {
			if (errno != error_noent) {
				log5("warning: unable to unlink ", fn1.s, ": ", error_str(errno), "\n");
				goto fail;
			}
		}
	} else {
		fnmake_todo(id); /*- remove intd and todo */
		if (substdio_putflush(&sstoqc, fn1.s, fn1.len) == -1) {
			cleandied();
			return;
		}
		if (substdio_get(&ssfromqc, &ch, 1) != 1) {
			cleandied();
			return;
		}
		if (ch != '+') {
			log3("warning: qmail-clean unable to clean up ", fn1.s, "\n");
			return;
		}
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
	return;
}

void
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

static void
todo_do(fd_set *rfds)
{
	unsigned long   id;

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
		trigger_set(); /*- open lock/trigger */
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
		/*- flow through */
	default:
		return;
	}
	process_todo(id);
}

static void
comm_init()
{
	int             c;

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
			log5("warning: unable to unlink ", fn1.s, "; will try again later: ", error_str(errno), "\n");
			pe.dt = now() + SLEEP_SYSFAIL;
		} else {
			int             c;
			for (c = 0; c < CHANNELS; ++c) {
				if (c != jo[j].channel) {
					fnmake_chanaddr(jo[j].id, c);
					if (stat(fn1.s, &st) == 0)
						return;	/*- more channels going */
					if (errno != error_noent) {
						log3("warning: unable to stat ", fn1.s, "\n");
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
	while (!prioq_insert(min, &pqchan[jo[j].channel], &pe))
		nomem();
}

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
static DEL     *del[CHANNELS];
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
	}
	if (flagexitasap)
		log1(" exitasap");
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
		if (flagspawnalive[c]) { /* if dead, nothing we can do about its jobs */
			if (concurrencyused[c]) /*- stay alive if delivery jobs are present */
				return 0;
		}
	}
	return 1;
}

static int
del_avail(int c)
{
	return flagspawnalive[c] && comm_canwrite(c) && (concurrencyused[c] < concurrency[c]);
}

static void
del_start(int j, seek_pos mpos, char *recip)
{
	int             i, c;

	c = jo[j].channel;
	if (!flagspawnalive[c])
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
	log1("\n");
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
	log3("warning: trouble marking ", fn1.s, "; message will be delivered twice!\n");
}

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
		log1("alert: unable to append to bounce message; HELP! sleeping...\n");
		sleep(10);
	}
	pos = 0;
	while (pos < bouncetext.len) {
		if ((w = write(fd, bouncetext.s + pos, bouncetext.len - pos)) <= 0) {
			log1("alert: unable to append to bounce message; HELP! sleeping...\n");
			sleep(10);
		} else
			pos += w;
	}
	close(fd);
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
				log1("warning: internal error: delivery report out of range\n");
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
					log1("\n");
					markdone(c, jo[del[c][delnum].j].id, del[c][delnum].mpos);
					--jo[del[c][delnum].j].numtodo;
					break;
				case 'Z':
					log3_noflush("delivery ", strnum1, ": deferral: ");
					logsafe_noflush(dline[c].s + 3);
					log1("\n");
					break;
				case 'D':
					log3_noflush("delivery ", strnum1, ": failure: ");
					logsafe_noflush(dline[c].s + 3);
					log1("\n");
					addbounce(jo[del[c][delnum].j].id, del[c][delnum].recip.s, dline[c].s + 3);
					markdone(c, jo[del[c][delnum].j].id, del[c][delnum].mpos);
					--jo[del[c][delnum].j].numtodo;
					break;
				default:
					log3("delivery ", strnum1, ": report mangled, will defer\n");
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
			if (!pass[c].id && prioq_get(&pqchan[c], &pe) && *wakeup > pe.dt)
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

/*
 * pass job to qmail-lspawn, qmail-rpsawn
 */
static void
pass_dochan(int c)
{
	datetime_sec    birth;
	struct prioq_elt pe;
	static stralloc line = {0}, qqeh = {0}, envh = {0};
	int             match;

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
		log3("warning: trouble reading ", fn1.s, "; will try again later\n");
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
		++jo[pass[c].j].numtodo;
		del_start(pass[c].j, pass[c].mpos, line.s + 1); /*- line.s[1] = recipient */
		break;
	case 'D': /*- delivery done */
		break;
	default:
		fnmake_chanaddr(pass[c].id, c);
		log3("warning: unknown record type in ", fn1.s, "!\n");
		close(pass[c].fd);
		job_close(pass[c].j);
		pass[c].id = 0;
		return;
	}
	pass[c].mpos += line.len;
	return;

trouble:
	log3("warning: trouble opening ", fn1.s, "; will try again later\n");
	pe.dt = recent + SLEEP_SYSFAIL;
	while (!prioq_insert(min, &pqchan[c], &pe))
		nomem();
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
	int             r = -1, fd;
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
		log3("warning: unable to stat ", fn2.s, "\n");
		return 0;
	}
	if (str_equal(sender.s, "#@[]"))
		log3("triple bounce: discarding ", fn2.s, "\n");
	else
	if (!*sender.s && *doublebounceto.s == '@')
		log3("double bounce: discarding ", fn2.s, "\n");
	else {
		if ((p = env_get("BOUNCEQUEUE")) && !env_put2("QMAILQUEUE", p)) {
			log1("alert: out of memory; will try again later\n");
			return (0);
		}
		/*-
		 * Allow bounces to have different rules, queue, controls, etc
		 */
		if (chdir(auto_qmail) == -1) {
			log5("alert: unable to read controls: unable to switch to ", auto_qmail, ": ", error_str(errno), "\n");
			return (0);
		}
		if (qmail_open(&qqt) == -1) {
			log1("warning: unable to start qmail-queue, will try later\n");
			return 0;
		}
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
							log3("srs: ", srs_error.s, "\n");
							qmail_fail(&qqt);
							break;
						case -2:
							nomem();
							qmail_fail(&qqt);
							break;
						case -1:
							log1("alert: unable to read controls\n");
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
							log5("alert: unable to switch to ", auto_qmail, ": ", error_str(errno), "\n");
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
		qmail_from(&qqt, bouncesender);
		qmail_to(&qqt, bouncerecip);
		if (*qmail_close(&qqt)) {
			log1("warning: trouble injecting bounce message, will try later\n");
			return 0;
		}
		strnum2[fmt_ulong(strnum2, id)] = 0;
		log2_noflush("bounce msg ", strnum2);
		strnum2[fmt_ulong(strnum2, qp)] = 0;
		log3(" qp ", strnum2, "\n");
	}
	if (unlink(fn2.s) == -1) {
		log5("warning: unable to unlink ", fn2.s, ": ", error_str(errno), "\n");
		return 0;
	}
	return 1;
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
			log3("warning: unable to stat ", fn1.s, "; will try again later\n");
			goto fail;
		}
	}
	fnmake_todo(id);
	if (stat(fn1.s, &st) == 0)
		return;
	if (errno != error_noent) {
		log3("warning: unable to stat ", fn1.s, "; will try again later\n");
		goto fail;
	}
	fnmake_info(id);
	if (stat(fn1.s, &st) == -1) {
		if (errno == error_noent)
			return;
		log3("warning: unable to stat ", fn1.s, "; will try again later\n");
		goto fail;
	}

	/*- -todo +info -local -remote ?bounce */
	if (!injectbounce(id))
		goto fail;	/*- injectbounce() produced error message */
	strnum1[fmt_ulong(strnum1, id)] = 0;
	log3("end msg ", strnum1, "\n");

	/*- -todo +info -local -remote -bounce */
	fnmake_info(id);
	if (unlink(fn1.s) == -1) {
		log3("warning: unable to unlink ", fn1.s, "; will try again later\n");
		goto fail;
	}
	/*- -todo -info -local -remote -bounce; we can relax */
	if (!qmail_clean) {
		fnmake_intd(id);
		if (unlink(fn1.s) == -1) {
			if (errno != error_noent) {
				log3("warning: unable to unlink ", fn1.s, "; will try again later\n");
				goto fail;
			}
		}
		fnmake_mess(id);
		if (unlink(fn1.s) == -1) {
			if (errno != error_noent) {
				log3("warning: unable to unlink ", fn1.s, "; will try again later\n");
				goto fail;
			}
		}
	} else {
		fnmake_foop(id); /*- remove intd and mess */
		if (substdio_putflush(&sstoqc, fn1.s, fn1.len) == -1) {
			cleandied();
			return;
		}
		if (substdio_get(&ssfromqc, &ch, 1) != 1) {
			cleandied();
			return;
		}
		if (ch != '+')
			log3("warning: qmail-clean unable to clean up ", fn1.s, "\n");
	}
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
			pqadd(pe.id);
		}
	}
	if (prioq_get(&pqdone, &pe)) {
		if (pe.dt <= recent) {
			prioq_del(&pqdone);
			messdone(pe.id);
		}
	}
}

void
close23456()
{
	close(2);
	close(3);
	close(4);
	close(5);
	close(6); 
}

void
fix_queue()
{
	pid_t           pid;
	int             wstat, exitcode;

	sig_block(sig_int);
	switch ((pid = fork()))
	{
	case -1:
		_exit(111);
	case 0:
		strnum1[fmt_int(strnum1, conf_split)] = 0;
		qfargs[2] = strnum1;
		qfargs[3] = queuedir;
		execvp(*qfargs, qfargs); /*- qmail-lspawn */
		_exit(111);
	}
	sig_unblock(sig_int);
	if (wait_pid(&wstat, pid) != pid)
		strerr_die1sys(111, "alert: waitpid surprise: ");
	if (wait_crashed(wstat))
		strerr_die1sys(111, "alert: queue-fis crashed: ");
	exitcode = wait_exitcode(wstat);
	log1(exitcode ? "warning: trouble fixing queue directory\n" : "info: queue OK\n");
}

void
run_daemons(char **oargv, char **argv)
{
	int             i;
	int             pi0[2];
	int             pi1[2], pi2[2]; /*- qmail-lspawn */
	int             pi3[2], pi4[2]; /*- qmail-rspawn */
	int             pi5[2], pi6[2]; /*- qmail-clean */

	if (argv && argv[0]) {
		if (pipe(pi0) == -1)
			_exit(111);
		switch (fork())
		{
		case -1:
			_exit(111);
		case 0: /* execute logger */
			if (prot_gid(auto_gidn) == -1) /*- nofiles unix group */
				_exit(111);
			if (prot_uid(auto_uidl) == -1) /*- qmaill unix user */
				_exit(111);
			close(pi0[1]);
			if (fd_move(0, pi0[0]) == -1)
				_exit(111);
			close23456();
			execvp(argv[0], argv); /*- splogger, etc */
			_exit(111);
		}
		close(pi0[0]);
		if (fd_move(1, pi0[1]) == -1)
			_exit(111);
		close(pi0[1]);
	}

	if (pipe(pi1) == -1 || pipe(pi2) == -1) {
		log1("alert: trouble creating pipes\n");
		_exit(111);
	}
	chanfdout[0] = pi1[1];
	chanfdin[0] = pi2[0];
	switch (fork())
	{
	case -1:
		_exit(111);
	case 0:
		if (fd_copy(0, pi1[0]) == -1)
			_exit(111);
		if (fd_copy(1, pi2[1]) == -1)
			_exit(111);
		close23456();
		close(pi1[0]);
		close(pi1[1]);
		close(pi2[0]);
		close(pi2[1]);
		if (_qmail_lspawn) {
			sig_block(sig_int);
			i = str_rchr(oargv[0], '/');
			if (oargv[0][i])
				str_copy(oargv[0] + i + 1, "MTAlspawn");
			else
				str_copy(oargv[0], "MTAlspawn");
			qmail_lspawn(2, qlargs);
		} else
			execvp(*qlargs, qlargs); /*- qmail-lspawn */
		_exit(111);
	}
	close(pi1[0]);
	close(pi2[1]);

	if (pipe(pi3) == -1 || pipe(pi4) == -1) {
		log1("alert: trouble creating pipes\n");
		_exit(111);
	}
	chanfdout[1] = pi3[1];
	chanfdin[1] = pi4[0];
	switch (fork())
	{
	case -1:
		_exit(111);
	case 0:
		if (prot_uid(auto_uidq) == -1) /*- qmailq unix user */
			_exit(111);
		if (fd_copy(0, pi3[0]) == -1)
			_exit(111);
		if (fd_copy(1, pi4[1]) == -1)
			_exit(111);
		close23456();
		close(pi1[0]);
		close(pi1[1]);
		close(pi2[0]);
		close(pi2[1]);
		close(pi3[0]);
		close(pi3[1]);
		close(pi4[0]);
		close(pi4[1]);
		if (_qmail_rspawn) {
			sig_block(sig_int);
			i = str_rchr(oargv[0], '/');
			if (oargv[0][i])
				str_copy(oargv[0] + i + 1, "MTArspawn");
			else
				str_copy(oargv[0], "MTArspawn");
			qmail_rspawn(1, qrargs);
		} else
			execvp(*qrargs, qrargs); /*- qmail-rspawn */
		_exit(111);
	}
	close(pi3[0]);
	close(pi4[1]);

	if (!qmail_clean)
		return;
	if (pipe(pi5) == -1 || pipe(pi6) == -1) {
		log1("alert: trouble creating pipes\n");
		_exit(111);
	}
	switch ((cleanuppid = fork()))
	{
	case -1:
		_exit(111);
	case 0:
		if (prot_uid(auto_uidq) == -1) /*- qmailq unix user */
			_exit(111);
		if (fd_copy(0, pi5[0]) == -1)
			_exit(111);
		if (fd_copy(1, pi6[1]) == -1)
			_exit(111);
		close23456();
		close(pi1[0]);
		close(pi1[1]);
		close(pi2[0]);
		close(pi2[1]);
		close(pi3[0]);
		close(pi3[1]);
		close(pi4[0]);
		close(pi4[1]);
		close(pi5[0]);
		close(pi5[1]);
		close(pi6[0]);
		close(pi6[1]);
		execvp(*qcargs, qcargs); /*- qmail-clean */
		_exit(111);
	}
	close(pi5[0]);
	close(pi6[1]);
	substdio_fdbuf(&sstoqc, write, pi5[1], sstoqcbuf, sizeof (sstoqcbuf));
	substdio_fdbuf(&ssfromqc, read, pi6[0], ssfromqcbuf, sizeof (ssfromqcbuf));
	return;
}

void
qmta_initialize(int *nfds, fd_set *rfds)
{
	int             c;

	if (fd_copy(0, 1) == -1) {
		log1("alert: cannot dup stdout\n");
		_exit(111);
	}
	fnmake_init();  /*- initialize fn1, fn2 */
	comm_init();
	pqstart();      /*- add files earlier processed and now in info/split for processing */
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
			log1("alert: cannot start: hath the daemon spawn no fire?\n");
			_exit(111);
		}
		do {
			r = read(chanfdin[c], &ch2, 1);
		} while ((r == -1) && (errno == error_intr));
		if (r < 1) {
			log1("alert: cannot start: hath the daemon spawn no fire?\n");
			_exit(111);
		}
		u = (unsigned int) (unsigned char) ch1;
		u += (unsigned int) ((unsigned char) ch2) << 8;
		if (concurrency[c] > u)
			concurrency[c] = u;
		numjobs += concurrency[c];
	} /*- for (c = 0; c < CHANNELS; ++c) */
	job_init();     /*- initialize numjobs job structures */
	del_init();     /*- initialize concurrencylocal + concurrrencyremote delivery structure */
	pass_init();    /*- initialize pass structure */
	todo_init();
	cleanup_init(); /*- initialize flagcleanup = 0, cleanuptime = now */
	*nfds = 1;
}

int
do_controls()
{
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

	if (control_rldef(&envnoathost, "envnoathost", 1, "envnoathost") != 1)
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

	if (control_readfile(&newlocals, "locals", 1) != 1) {
		log1("alert: unable to reread locals\n");
		return;
	}
	if ((r = control_readfile(&newvdoms, "virtualdomains", 0)) == -1) {
		log1("alert: unable to reread virtualdomains\n");
		return;
	}
	if (control_readint(&todo_interval, "todointerval") == -1) {
		log1("alert: qmail-todo: unable to reread todointerval\n");
		return;
	}
	if (control_readint((int *) &concurrency[0], "concurrencylocal") == -1) {
		log1("alert: unable to reread concurrencylocal\n");
		return;
	}
	if (control_readint((int *) &concurrency[1], "concurrencyremote") == -1) {
		log1("alert: unable to reread concurrencyremote\n");
		return;
	}
	if (control_rldef(&envnoathost, "envnoathost", 1, "envnoathost") != 1) {
		log1("alert: unable to reread envnoathost\n");
		return;
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
#ifdef USE_FSYNC
	if (control_readint(&use_syncdir, "conf-syncdir") == -1) {
		log1("alert: unable to reread conf-syncdir\n");
		return;
	}
	if (control_readint(&use_fsync, "conf-fsync") == -1) {
		log1("alert: unable to reread conf-fsync\n");
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
}

static void
reread()
{
	if (chdir(auto_qmail) == -1) {
		log5("alert: unable to reread controls: unable to switch to ", auto_qmail, ": ", error_str(errno), "\n");
		return;
	}
	regetcontrols();
	chdir_toqueue();
}

static void
sigchld()
{
	int             wstat;
	pid_t           pid;

	while ((pid = wait_nohang(&wstat)) > 0) {
		if (pid == cleanuppid) {
			cleandied();
			break;
		}
	}
}

int
main(int argc, char **argv)
{
	int             nfds, fd, opt, daemon_mode = 0, flagqfix = 0;
	char           *ptr;
	datetime_sec    wakeup;
	fd_set          rfds, wfds;
	struct timeval  tv;
	char           *usage_str =
		"USAGE: qmta-send [options] [ defaultdelivery [logger arg...] ]\n"
		"OPTIONS\n"
		"       -d Run as a daemon\n"
		"       -f fix queue\n"
		"       -c Use qmail-clean  for cleanup\n"
		"       -l Use qmail-lspawn for spawning local  deliveries\n"
		"       -r Use qmail-rspawn for spawning remote deliveries\n"
		"       -s queue_split";

	while ((opt = getopt(argc, argv, "dfclrs:")) != opteof) {
		switch (opt)
		{
		case 'd':
			daemon_mode = 1;
			break;
		case 'f':
			flagqfix = 1;
			break;
		case 'c':
			qmail_clean = 1;
			break;
		case 'l':
			_qmail_lspawn = 1;
			break;
		case 'r':
			_qmail_rspawn = 1;
			break;
		case 's':
			scan_int(optarg, &conf_split);
			break;
		default:
			strerr_die2x(100, FATAL, usage_str);
			break;
		}
	}
	if (!conf_split)
		getEnvConfigInt(&conf_split, "CONFSPLIT", auto_split);
	if (conf_split > auto_split)
		conf_split = auto_split;
	strnum1[fmt_ulong(strnum1, conf_split)] = 0;
	log3("info: qmta-send: conf split=", strnum1, "\n");
	if (!(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue/qmta"; /*- single queue like qmail */
#ifdef LOGLOCK
	loglock_open(0);
#endif
	if (chdir(auto_qmail) == -1) {
		log5("alert: cannot start: unable to switch to ", auto_qmail, ": ", error_str(errno), "\n");
		_exit(111);
	}
	if (flagqfix)
		fix_queue();
	if (chdir(queuedir) == -1) {
		log5("alert: cannot start: unable to switch to ", queuedir, ": ", error_str(errno), "\n");
		_exit(111);
	}
	if ((fd = open_write("lock/sendmutex")) == -1) {
		log1("alert: cannot start: unable to open mutex\n");
		_exit(111);
	}
	if (lock_exnb(fd) == -1) {
		log1("alert: cannot start: qmail-send/mta-send is already running\n");
		_exit(111);
	}
	if (argc - optind > 1)
		qlargs[optind] = argv[optind];
	run_daemons(argv, argc - optind > 2 ? argv + optind + 1 : 0);
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
	sig_pipeignore();
	sig_termcatch(sigterm);
	sig_alarmcatch(sigalrm);
	sig_hangupcatch(sighup);
	if (qmail_clean)
		sig_childcatch(sigchld);
#ifdef LOGLOCK
	sig_intcatch(sigint);
#endif
	umask(077);
	if (!do_controls()) {
		log1("alert: cannot start: unable to read controls\n");
		_exit(111);
	}
	qmta_initialize(&nfds, &rfds);
	while (!flagexitasap || !del_canexit()) { /*- stay alive if delivery jobs are present */
		recent = now();
		if (flagrunasap) {
			flagrunasap = 0;
			pqrun(); /*- added files just added to run queue */
		}
		if (flagreadasap) {
			flagreadasap = 0;
			reread();
		}
		wakeup = recent + SLEEP_FOREVER;
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
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
		pass_selprep(&wakeup);
		todo_selprep(&nfds, &rfds, &wakeup);
		cleanup_selprep(&wakeup); /*- set wakeup */
		if (wakeup <= recent)
			tv.tv_sec = 0;
		else
			tv.tv_sec = wakeup - recent + SLEEP_FUZZ;
		tv.tv_usec = 0;
		if (select(nfds, &rfds, &wfds, NULL, &tv) == -1)
			if (errno == error_intr);
			else
				log1("warning: trouble in select\n");
		else {
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
			cleanup_do(&wfds);
		}
		if (!daemon_mode && del_canexit()) {
			log1("info: no pending deliveries. quitting...\n");
			break;
		}
	} /*- while (!flagexitasap || !del_canexit()) */
	pqfinish();
}
