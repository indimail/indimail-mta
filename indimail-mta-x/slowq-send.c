/*
 * $Id: slowq-send.c,v 1.36 2023-12-25 09:31:28+05:30 Cprogrammer Exp mbhangui $
 */
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
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
#include "getDomainToken.h"
#include "varargs.h"

/*- critical timing feature #1: if not triggered, do not busy-loop */
/*- critical timing feature #2: if triggered, respond within fixed time */
/*- important timing feature: when triggered, respond instantly */
#define SLEEP_FUZZ         1 /*- slop a bit on sleeps to avoid zeno effect */
#define SLEEP_FOREVER  86400 /*- absolute maximum time spent in select() */
#define SLEEP_CLEANUP  76431 /*- time between cleanups */
#define SLEEP_SYSFAIL    123
#ifndef TODO_INTERVAL
#define SLEEP_TODO      1500 /*- check todo/ every 25 minutes in any case */
#define ONCEEVERY         10 /*- Run todo maximal once every N seconds */
#endif
#define CHUNK_SIZE      1

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
static char    *argv0 = "slowq-send";

static int      flagexitsend; /*- slowq-send: exit when set */
static int      flagexittodo; /*- todo-processor: exit when set */
static int      flagrunasap; /*- immediaely schedule deliveries in queue */
static int      flagreadasap; /*- reread control files on sighup */
static int      flagdetached; /*- slowq-send detaches from todo processing */
static int      todoproc; /*- run a parallel independent todo processor */
static int      flagtodoalive, flagsendalive = 1; /*- flag to indicate if todo-processor and slowq-send are alive */
static int      todopid, bigtodo, todofdi, todofdo, sendfdi, sendfdo, todo_chunk_size;
static datetime_sec nexttodorun, lasttodorun;
static int      flagtododir = 0;	/*- if 0, have to readsubdir_init again */
static int      todo_interval = -1;
static readsubdir todosubdir;
static stralloc todoline = { 0 };
static char     todobuf[SUBSTDIO_INSIZE];
static char     todobufinfo[512];
static char     todobufchan[CHANNELS][1024];

extern dtype    delivery;
static int      do_ratelimit;
unsigned long   delayed_jobs;

static substdio sstoqc;
static substdio ssfromqc;
static char     sstoqcbuf[1024];
static char     ssfromqcbuf[1024];
static stralloc comm_buf_spawn[CHANNELS] = { {0}, {0} };
static int      comm_pos_spawn[CHANNELS];

static stralloc comm_buf_todo = { 0 };
static int      comm_pos_todo, comm_count_todo;

typedef enum    {qmail_spawn, qmail_todo} comm_type;

static void     reread();
static void     sigusr1();
static void     sigusr2();

void
#ifdef  HAVE_STDARG_H
todo_log(char *s1, ...)
#else
todo_log(va_alist)
va_dcl
#endif
{
	int             pos;
	va_list         ap;
	char           *str;
#ifndef HAVE_STDARG_H
	char           *s1;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, s1);
#else
	va_start(ap);
	s1 = va_arg(ap, char *);
#endif

	pos = comm_buf_todo.len;
	if (!stralloc_cats(&comm_buf_todo, "L") ||
			!stralloc_cats(&comm_buf_todo, s1))
		goto fail;

	while (1) {
		str = va_arg(ap, char *);
		if (!str)
			break;
		if (!stralloc_cats(&comm_buf_todo, str))
			goto fail;
	}
	if (!stralloc_0(&comm_buf_todo))
		goto fail;
	va_end(ap);
	return;
fail:
	va_end(ap);
	/*- either all or nothing */
	comm_buf_todo.len = pos;
}

static void
sigterm()
{
	flagexitsend = 1;
	if (todopid)
		kill(todopid, SIGTERM);
	slog(1, "alert: ", argv0, ": ", mypid, ": got TERM: ", queuedesc, "\n", NULL);
}

static void
sigterm_todo()
{
	sig_block(sig_term);
	todo_log("alert: ", argv0, ": pid ", mypid, " got TERM: ", queuedesc, "\n", NULL);
	if (!flagexittodo)
		todo_log("info: ", argv0, ": ", queuedesc, " stop todo processing asap\n", NULL);
	flagexittodo = 1;
}

static void
stop_todo()
{
	sig_block(sig_term);
	todo_log("alert: ", argv0, ": pid ", mypid, " ordered to quit by slowq-send: ", queuedesc, "\n", NULL);
	if (!flagexittodo)
		todo_log("info: ", argv0, ": ", queuedesc, " stop todo processing asap\n", NULL);
	flagexittodo = 1;
}

static void
sigalrm()
{
	flagrunasap = 1;
	slog(1, "alert: ", argv0, ": ", mypid, ": got ALRM: ", queuedesc, "\n", NULL);
}

static void
sighup()
{
	flagreadasap = 1;
	slog(1, "alert: ", argv0, ": ", mypid, ": got HUP: ", queuedesc, "\n", NULL);
}

static void
chdir_toqueue()
{
	if (!queuedir && !(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue/slowq";
	while (chdir(queuedir) == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc,
			": unable to switch back to queue directory; HELP! sleeping...",
			error_str(errno), "\n", NULL);
		sleep(10);
	}
}

void
exit_todo()
{
	int             r;

	if (write(todofdo, "X", 1)) ; /*- keep compiler happy */
	r = read(todofdi, todobuf, sizeof (todobuf));
	if (r > 0 && todobuf[0] == 'L')
		slog(1, todobuf + 1, NULL);
	flagtodoalive = 0;
}

static void
cleandied()
{
	slog(1, "alert: ", argv0, ": ", queuedesc,
		": oh no! lost qmail-clean connection! dying...\n", NULL);
	flagexitsend = 1;
	if (flagtodoalive)
		exit_todo();
}

int             flagspawnalive[CHANNELS];

static void
spawndied(int c)
{
	slog(1, "alert: ", argv0, ": ", queuedesc,
		": oh no! lost spawn connection! dying...\n", NULL);
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
static int
issafe(char ch)
{
	/*- general principle: allman's code is crap */
	if (ch == '%' || ch < 33 || ch > 126)
		return 0;
	return 1;
}

/*
 * slowq-send
 * slowq-send + todo-processor
 */
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

/*
 * used by
 * todo-processor
 */
void
comm_exit_send(void)
{
	if (!stralloc_cats(&comm_buf_todo, "X") ||
			!stralloc_0(&comm_buf_todo))
		_exit(1);
}

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
static int
comm_canwrite(comm_type comm, int c)
{
	/*- XXX: could allow a bigger buffer; say 10 recipients */
	switch(comm)
	{
	case qmail_spawn:
		if (comm_buf_spawn[c].s && comm_buf_spawn[c].len) /*- data pending to be written to qmail-[l|r]spawn */
			return 0;
		return 1;
		break;
	case qmail_todo:
		if (!flagsendalive)
			return 0;
		if (comm_buf_todo.s && comm_buf_todo.len)
			return 1;
		return 0;
		break;
	}
	return 0;
}

/*-
 * used by
 * slowq-send
 * slowq-send + todo-processor
 *
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
comm_write_spawn(int c, int delnum, unsigned long id, char *sender,
		char *qqeh, char *envh, char *recip)
{
	char            ch;

	if (comm_buf_spawn[c].s && comm_buf_spawn[c].len)
		return;
	while (!stralloc_copys(&comm_buf_spawn[c], ""))
		nomem(argv0);
	ch = delnum;
	while (!stralloc_append(&comm_buf_spawn[c], &ch))
		nomem(argv0);
	ch = delnum >> 8;
	while (!stralloc_append(&comm_buf_spawn[c], &ch))
		nomem(argv0);
	fnmake_split(id);
	while (!stralloc_cats(&comm_buf_spawn[c], fn1.s))
		nomem(argv0);
	while (!stralloc_0(&comm_buf_spawn[c]))
		nomem(argv0);
	senderadd(&comm_buf_spawn[c], sender, recip);
	while (!stralloc_0(&comm_buf_spawn[c]))
		nomem(argv0);
	while (!stralloc_cats(&comm_buf_spawn[c], qqeh))
		nomem(argv0);
	while (!stralloc_0(&comm_buf_spawn[c]))
		nomem(argv0);
	while (!stralloc_cats(&comm_buf_spawn[c], envh))
		nomem(argv0);
	while (!stralloc_0(&comm_buf_spawn[c]))
		nomem(argv0);
	while (!stralloc_cats(&comm_buf_spawn[c], recip))
		nomem(argv0);
	while (!stralloc_0(&comm_buf_spawn[c]))
		nomem(argv0);
	comm_pos_spawn[c] = 0;
}

/*
 * used by
 * todo-processor
 * prepare comm buf to be written to slowq-send
 */
void
comm_write_todo(unsigned long id, int local, int remote)
{
	int             pos;
	char           *s;

	if (flagdetached)
		return;
	if (local && remote)
		s = "B";
	else
	if (local)
		s = "L";
	else
	if (remote)
		s = "R";
	else
		s = "X";
	pos = comm_buf_todo.len;
	if (!stralloc_cats(&comm_buf_todo, "D") ||
			!stralloc_cats(&comm_buf_todo, s) ||
			!stralloc_catb(&comm_buf_todo, strnum1, fmt_ulong(strnum1, id)) ||
			!stralloc_0(&comm_buf_todo))
		goto fail;
	return;
fail:
	/*- reset length to start */
	comm_buf_todo.len = pos;
}

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
static void
comm_selprep_spawn(int *nfds, fd_set *wfds)
{
	int             c;

	for (c = 0; c < CHANNELS; ++c) {
		if (flagspawnalive[c] && comm_buf_spawn[c].s && comm_buf_spawn[c].len) {
			FD_SET(chanfdout[c], wfds); /*- fd 1 - pi1[1] - write, and fd 3 - pi3[1] - write */
			if (*nfds <= chanfdout[c])
				*nfds = chanfdout[c] + 1;
		}
	}
}

/*
 * used by
 * todo-processor
 */
void
comm_selprep_send(int *nfds, fd_set *wfds, fd_set *rfds)
{
	if (flagsendalive) {
		int             c;

		c = comm_canwrite(qmail_todo, 0);
		if (flagexittodo && c == 0)
			comm_exit_send();
		if (c) {
			FD_SET(sendfdo, wfds); /*- write fd to slowq-send */
			if (*nfds <= sendfdo)
				*nfds = sendfdo + 1;
		}
		FD_SET(sendfdi, rfds); /*- read fd from slowq-send */
		if (*nfds <= sendfdi)
			*nfds = sendfdi + 1;
	}
}

/*-
 * used by
 * slowq-send
 * todo-processor
 * write to qmail-lspawn, qmail-rspawn
 * set comm_buf_spawn[c].len = 0 when data is written
 */
static void
comm_do_spawn(fd_set *wfds)
{
	int             c;

	for (c = 0; c < CHANNELS; ++c) {
		if (flagspawnalive[c] && comm_buf_spawn[c].s && comm_buf_spawn[c].len && FD_ISSET(chanfdout[c], wfds)) {
			int             w;
			int             len;

			len = comm_buf_spawn[c].len;
			if ((w = write(chanfdout[c], comm_buf_spawn[c].s + comm_pos_spawn[c], len - comm_pos_spawn[c])) <= 0) {
				if ((w == -1) && (errno == error_pipe))
					spawndied(c);
				else
					continue;	/*- kernel select() bug; can't avoid busy-looping */
			} else {
				comm_pos_spawn[c] += w;
				if (comm_pos_spawn[c] == len)
					comm_buf_spawn[c].len = 0;
			}
		}
	}
}

/*
 * used by
 * todo-processor
 */
static void
senddied()
{
	if (!flagexittodo)
		slog(1, "alert: ", argv0, ": ", queuedesc,
			": oh no! lost slowq-send connection! dying...\n", NULL);
	flagsendalive = 0;
}

/*
 * used by
 * todo-processor
 */
void
comm_read_todo(fd_set *wfds, fd_set *rfds)
{
	if (flagsendalive && comm_canwrite(qmail_todo, 0)) {
		if (FD_ISSET(sendfdo, wfds)) {
			int             w;
			int             len;

			len = comm_buf_todo.len;
			if ((w = write(sendfdo, comm_buf_todo.s + comm_pos_todo, len - comm_pos_todo)) <= 0) {
				if ((w == -1) && (errno == error_pipe))
					senddied();
			} else {
				comm_pos_todo += w;
				if (comm_pos_todo == len) {
					comm_buf_todo.len = 0;
					comm_pos_todo = 0;
					comm_count_todo = 0;
				}
			}
		}
	}
	/*- next read from slowq-send */
	if (flagsendalive && FD_ISSET(sendfdi, rfds)) {
		/*- handle 'A', 'D', 'H' and 'X' */
		char            c;
		int             r;
		if ((r = read(sendfdi, &c, 1)) <= 0) {
			if (!r || (r == -1 && errno != error_intr))
				senddied();
		} else {
			switch (c)
			{
			case 'A': /*- attached mode */
				flagdetached = 0;
				todo_log("alert: ", argv0, ": pid ", mypid,
					" got 'A' command: todo-processor attached: ",
					queuedesc, "\n", NULL);
				break;
			case 'D': /*- detached mode */
				flagdetached = 1;
				todo_log("alert: ", argv0, ": pid ", mypid,
					" got 'D' command: todo-processor detached: ",
					queuedesc, "\n", NULL);
				break;
			case 'H':
				sighup(); /*- set flagreadasap = 1 */
				break;
			case 'X':
				stop_todo(); /*-set flagexittodo = 1 */
				break;
			default:
				todo_log("warn: ", argv0, ": ", queuedesc,
					": slowq-send speaks an obscure dialect\n", NULL);
				break;
			}
		}
	}
}

/*-
 * used by
 * todo-processor
 * shutdown communication channel with slowq-send
 * and exit
 */
void
comm_die_send(int i)
{
	fd_set          rfds, wfds;
	int             nfds;

	if (ndelay_on(sendfdo) == -1)
		senddied();
	while (!stralloc_ready(&comm_buf_todo, 1024))
		nomem(argv0);
	comm_exit_send();
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	nfds = 1;
	comm_selprep_send(&nfds, &wfds, &rfds);
	comm_read_todo(&wfds, &rfds);
	todo_log("info: ", argv0, ": ", queuedesc, " stop todo processing asap\n", NULL);
	_exit(i);
}

/*
 * used by
 * todo-processor
 */
void
comm_info_todo(unsigned long id, unsigned long size, char *from, unsigned long pid, unsigned long uid)
{
	int             pos;
	int             i;

	comm_count_todo++;
	pos = comm_buf_todo.len;
	if (!stralloc_cats(&comm_buf_todo, "Linfo msg "))
		goto fail;
	if (!stralloc_catb(&comm_buf_todo, strnum1, fmt_ulong(strnum1, id)) ||
			!stralloc_cats(&comm_buf_todo, ": bytes ") ||
			!stralloc_catb(&comm_buf_todo, strnum2, fmt_ulong(strnum2, size)) ||
			!stralloc_cats(&comm_buf_todo, " from <"))
		goto fail;
	i = comm_buf_todo.len;
	if (!stralloc_cats(&comm_buf_todo, from))
		goto fail;
	for (; i < comm_buf_todo.len; ++i) {
		if (comm_buf_todo.s[i] == '\n')
			comm_buf_todo.s[i] = '/';
		else
		if (!issafe(comm_buf_todo.s[i]))
			comm_buf_todo.s[i] = '_';
	}
	if (!stralloc_cats(&comm_buf_todo, "> qp "))
		goto fail;
	if (!stralloc_catb(&comm_buf_todo, strnum1, fmt_ulong(strnum1, pid)) ||
			!stralloc_cats(&comm_buf_todo, " uid ") ||
			!stralloc_catb(&comm_buf_todo, strnum2, fmt_ulong(strnum2, uid)) ||
			!stralloc_cats(&comm_buf_todo, " ") ||
			!stralloc_cats(&comm_buf_todo, queuedesc) ||
			!stralloc_cats(&comm_buf_todo, "\n") ||
			!stralloc_0(&comm_buf_todo))
		goto fail;
	return;

fail:
	/*- either all or nothing */
	comm_buf_todo.len = pos;
}

/*- this file is too long ------------------------------------------ CLEANUPS */

static int      flagcleanup;	/*- if 1, cleanupdir is initialized and ready */
static readsubdir cleanupdir;
static datetime_sec cleanuptime;

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
static void
cleanup_init()
{
	flagcleanup = 0;
	cleanuptime = now();
}

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
static void
cleanup_selprep(datetime_sec *wakeup)
{
	if (flagcleanup)
		*wakeup = 0;
	if (*wakeup > cleanuptime)
		*wakeup = cleanuptime;
}

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
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

static prioq    pqdone = { 0 };						/*- -todo +info; HOPEFULLY -local -remote */
static prioq    pqchan[CHANNELS] = { {0} , {0} };	/*- pqchan 0: -todo +info +local ?remote */
													/*- pqchan 1: -todo +info ?local +remote */
static prioq    pqfail = { 0 };						/*- stat() failure; has to be pqadded again */

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
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
	/*- add to delivery queue */
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

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
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

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
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

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
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
				slog(1, "warn: ", argv0, ": ", queuedesc, "unable to utime ", fn1.s, "; message will be retried too soon\n", NULL);
		}
	}
}

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
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

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
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

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
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

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
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

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
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
		slog(1, "alert: ", argv0, ": ", queuedesc, ": Unable to fork: ", error_str(errno), "\n", NULL);
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
		slog(1, "alert: ", argv0, ": ", queuedesc, ": Unable to run: ", prog, ": ",
				error_str(errno), "\n", NULL);
		_exit(111);
	}
	wait_pid(&wstat, child);
	if (wait_crashed(wstat)) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": ", prog, " crashed: ",
				error_str(errno), "\n", NULL);
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
			slog(1, "alert: ", argv0, ": ", queuedesc, ": out of memory; will try again later\n");
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
				reread(); /*- this does chdir_toqueue() */
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
							slog(1, "srs: ", queuedesc, ": ", srs_error.s, "\n", NULL);
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
							slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to switch to ",
									auto_qmail, ": ", error_str(errno), "\n", NULL);
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
		while (!stralloc_copyb(&boundary, strnum2, fmt_ulong(strnum2, birth)))
			nomem(argv0);
		while (!stralloc_cat(&boundary, &bouncehost))
			nomem(argv0);
		while (!stralloc_catb(&boundary, strnum2, fmt_ulong(strnum2, id)))
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
			substdio_fdbuf(&ssread, read, fd, inbuf, sizeof (inbuf));
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
				slog(1, "warn: ", argv0, ": ", queuedesc, ": unable to unlink ", fn2.s, ". Will try later\n", NULL);
				return 0;
			}
			slog(1, "delete bounce: discarding ", fn2.s, " ", queuedesc, "\n", NULL);
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
		strnum2[fmt_ulong(strnum2, id)] = 0;
		slog(0, "bounce msg ", strnum2, NULL);
		strnum2[fmt_ulong(strnum2, qp)] = 0;
		slog(1, " qp ", strnum2, " ", queuedesc, "\n", NULL);
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

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
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

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
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

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
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

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
static int
del_avail(int c)
{
	return flagspawnalive[c] && comm_canwrite(qmail_spawn, c) && !holdjobs[c] && (concurrencyused[c] < concurrency[c]);	/* NJL 1998/07/24 */
}

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
static void
del_start(int j, seek_pos mpos, char *recip)
{
	int             i, c;

	c = jo[j].channel;
	if (!flagspawnalive[c])
		return;
	if (holdjobs[c])
		return;	/* NJL 1998/05/03 */
	if (!comm_canwrite(qmail_spawn, c))
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
	comm_write_spawn(c, i, jo[j].id, jo[j].sender.s, jo[j].qqeh.s, jo[j].envh.s, recip);
	strnum1[fmt_ulong(strnum1, del[c][i].delid)] = 0;
	strnum2[fmt_ulong(strnum2, jo[j].id)] = 0;
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
 * used by
 * slowq-send
 * slowq-send + todo-processor
 *
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
					slog(0, "delivery ", strnum1, ": success: ", NULL);
					logsafe_noflush(dline[c].s + 3, argv0);
					slog(1, " ", queuedesc, "\n", NULL);
					markdone(c, jo[del[c][delnum].j].id, del[c][delnum].mpos);
					--jo[del[c][delnum].j].numtodo;
					break;
				case 'Z':
					slog(0, "delivery ", strnum1, ": deferral: ", NULL);
					logsafe_noflush(dline[c].s + 3, argv0);
					slog(1, " ", queuedesc, "\n", NULL);
					break;
				case 'D':
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
				del_status();
			}
			dline[c].len = 0;
		} /* if (!ch && (dline[c].len > 2)) */
	} /*- for (i = 0; i < r; ++i) */
}

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
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

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
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

/*
 * used by
 * slowq-send
 * slowq-send + todo-processor
 */
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
		substdio_fdbuf(&pass[c].ss, read, pass[c].fd, pass[c].buf, sizeof (pass[c].buf));
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
		else
		if (_do_ratelimit) /*- for del_status to display delayed jobs */
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

/*
 * used by
 * slowq-send
 * todo-processor
 */
static void
todo_init()
{
	flagtododir = 0;
	lasttodorun = nexttodorun = now();
	trigger_set();
}

/*
 * slowq-send
 */
static void
todo_init_send()
{
	/*-
	 * syncrhonize with todo-processor so that
	 * we are ready when the todo-processor starts
	 * sending us deliveryjobs
	 */
	if (write(todofdo, "C", 1) == 1)
		flagtodoalive = 1;
	else {
		slog(1, "alert: ", argv0, ": ", queuedesc,
				": unable to write a byte to external todo! dying...: ",
				error_str(errno), "\n", NULL);
		flagexitsend = 1;
		flagtodoalive = 0;
	}
	return;
}

/*
 * slowq-send without todo-processor
 */
static void
todo_selprep(int *nfds, fd_set *rfds, datetime_sec *wakeup)
{
	if (flagexitsend)
		return;
	trigger_selprep(nfds, rfds);
	if (flagtododir)
		*wakeup = 0;
	if (*wakeup > nexttodorun)
		*wakeup = nexttodorun;
}

/*
 * todo-processor
 */
void
todo_selprep_todo(int *nfds, fd_set *rfds, datetime_sec *wakeup)
{
	if (flagexittodo)
		return;
	trigger_selprep(nfds, rfds);
	if (flagtododir)
		*wakeup = 0;
	if (*wakeup > nexttodorun)
		*wakeup = nexttodorun;
}

/*
 * slowq-send
 */
static void
todo_selprep_send(int *nfds, fd_set *rfds, datetime_sec *wakeup)
{
	if (flagexitsend && flagtodoalive) {
		if (write(todofdo, "X", 1)) ; /* keep compiler happy */
		flagtodoalive = 0;
	}
	if (flagtodoalive) {
		FD_SET(todofdi, rfds); /*- read fd from todo-processor */
		if (*nfds <= todofdi)
			*nfds = todofdi + 1;
	}
}

static stralloc mailfrom = { 0 };
static stralloc mailto = { 0 };

static void
todo_do(fd_set *rfds)
{
	struct stat     st;
	struct prioq_elt pe;
	substdio        sstodo, ssinfo;
	substdio        sschan[CHANNELS];
	int             fdtodo, fdinfo, match, i, c, split;
	int             fdchan[CHANNELS], flagchan[CHANNELS];
	char            ch;
	char           *ptr;
	unsigned long   id, uid, pid;

	if (flagexitsend)
		return;
	fdtodo = -1;
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
		readsubdir_init(&todosubdir, "todo", bigtodo, pausedir);
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
	ptr = readsubdir_name(&todosubdir);
	if (ptr) {
		scan_int(ptr, &split);
		fnmake_todo(id);
		scan_int(fn1.s + 5, &i);
		slog(1, split != i ? "warn: " : "info: ", argv0, ": ", queuedesc,
				": subdir=todo/", ptr, " fn=", fn1.s,
				split != i ? " incorrect split\n" : "\n", NULL);
		if (split != i)
			return;
	} else
		fnmake_todo(id);
	if ((fdtodo = open_read(fn1.s)) == -1) {
		slog(1, "warn: ", argv0, ": unable to open ", fn1.s, "\n", NULL);
		return;
	}
	fnmake_mess(id);
	/*- just for the statistics */
	if (stat(fn1.s, &st) == -1) {
		slog(1, "warn: ", argv0, ": unable to stat ", fn1.s, "\n", NULL);
		goto fail;
	}
	for (c = 0; c < CHANNELS; ++c) {
		fnmake_chanaddr(id, c);
		if (unlink(fn1.s) == -1) {
			if (errno != error_noent) {
				slog(1, "warn: ", argv0, ": unable to unlink: ", fn1.s, ": ", error_str(errno), "\n", NULL);
				goto fail;
			}
		}
	}
	fnmake_info(id);
	if (unlink(fn1.s) == -1) {
		if (errno != error_noent) {
			slog(1, "warn: ", argv0, ": unable to unlink ", fn1.s, "\n", NULL);
			goto fail;
		}
	}
	if ((fdinfo = open_excl(fn1.s)) == -1) {
		slog(1, "warn: ", argv0, ": unable to create ", fn1.s, "\n", NULL);
		goto fail;
	}
	strnum1[fmt_ulong(strnum1, id)] = 0;
	slog(1, "new msg ", strnum1, "\n", NULL);
	for (c = 0; c < CHANNELS; ++c)
		flagchan[c] = 0;
	substdio_fdbuf(&sstodo, read, fdtodo, todobuf, sizeof (todobuf));
	substdio_fdbuf(&ssinfo, write, fdinfo, todobufinfo, sizeof (todobufinfo));
	uid = 0;
	pid = 0;
	for (;;) {
		if (getln(&sstodo, &todoline, &match, '\0') == -1) {
			/*- perhaps we're out of memory, perhaps an I/O error */
			fnmake_todo(id);
			slog(1, "warn: ", argv0, ": trouble reading ", fn1.s, "\n", NULL);
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
				slog(1, "warn: ", argv0, ": trouble writing to ", fn1.s, "\n", NULL);
				goto fail;
			}
			break;
		case 'F':
			if (substdio_put(&ssinfo, todoline.s, todoline.len) == -1) {
				fnmake_info(id);
				slog(1, "warn: ", argv0, ": trouble writing to ", fn1.s, "\n", NULL);
				goto fail;
			}
			slog(0, "info msg ", strnum1, NULL);
			strnum2[fmt_ulong(strnum2, (unsigned long) st.st_size)] = 0;
			slog(0, ": bytes ", strnum2, NULL);
			slog(0, " from <", NULL);
			logsafe_noflush(todoline.s + 1, argv0);
			strnum2[fmt_ulong(strnum2, pid)] = 0;
			slog(0, "> qp ", strnum2, NULL);
			strnum2[fmt_ulong(strnum2, uid)] = 0;
			slog(0, " uid ", strnum2, NULL);
			slog(0, " ", queuedesc, "\n", NULL);
			flush();
			if (!stralloc_copy(&mailfrom, &todoline) || !stralloc_0(&mailfrom)) {
				nomem(argv0);
				goto fail;
			}
			break;
		case 'T':
			switch (rewrite(todoline.s + 1))
			{
			case 0:
				nomem(argv0);
				goto fail;
			case 2: /*- Sea */
				if (!stralloc_cats(&mailto, "R") || !stralloc_cat(&mailto, &todoline)) {
					nomem(argv0);
					goto fail;
				}
				c = 1;
				break;
			default: /*- Land */
				if (!stralloc_cats(&mailto, "L") || !stralloc_cat(&mailto, &todoline)) {
					nomem(argv0);
					goto fail;
				}
				c = 0;
				break;
			}
			if (fdchan[c] == -1) {
				fnmake_chanaddr(id, c);
				fdchan[c] = open_excl(fn1.s);
				if (fdchan[c] == -1) {
					slog(1, "warn: ", argv0, ": ", queuedesc, ": unable to create ", fn1.s, "\n", NULL);
					goto fail;
				}
				substdio_fdbuf(&sschan[c], write, fdchan[c], todobufchan[c], sizeof (todobufchan[c]));
				flagchan[c] = 1;
			}
			if (substdio_bput(&sschan[c], rwline.s, rwline.len) == -1) {
				fnmake_chanaddr(id, c);
				slog(1, "warn: ", argv0, ": ", queuedesc, ": trouble writing to ", fn1.s, "\n", NULL);
				goto fail;
			}
			break;
		default:
			fnmake_todo(id);
			slog(1, "warn: ", argv0, ": ", queuedesc, ": unknown record type in ", fn1.s, "\n", NULL);
			goto fail;
		}
	} /*- for (;;) */
	close(fdtodo);
	fdtodo = -1;
	fnmake_info(id);
	if (substdio_flush(&ssinfo) == -1) {
		slog(1, "warn: ", argv0, ": ", queuedesc, ": trouble writing to ", fn1.s, "\n", NULL);
		goto fail;
	}
	for (c = 0; c < CHANNELS; ++c) {
		if (fdchan[c] != -1) {
			if (substdio_flush(&sschan[c]) == -1) {
				fnmake_chanaddr(id, c);
				slog(1, "warn: ", argv0, ": ", queuedesc, ": trouble writing to ", fn1.s, "\n", NULL);
				goto fail;
			}
		}
	}
#ifdef USE_FSYNC
	if ((use_fsync > 0 || use_fdatasync > 0) && (use_fdatasync ? fdatasync(fdinfo) : fsync(fdinfo)) == -1) {
		slog(1, "warn: ", argv0, ": ", queuedesc, ": trouble fsyncing ", fn1.s, "\n", NULL);
		goto fail;
	}
#else
	if (fsync(fdinfo) == -1) {
		slog(1, "warn: ", argv0, ": ", queuedesc, ": trouble fsyncing ", fn1.s, "\n", NULL);
		goto fail;
	}
#endif
	close(fdinfo);
	fdinfo = -1;
	for (c = 0; c < CHANNELS; ++c) {
		if (fdchan[c] != -1) {
#ifdef USE_FSYNC
			if ((use_fsync > 0 || use_fdatasync > 0) && (use_fdatasync ? fdatasync(fdchan[c]) : fsync(fdchan[c])) == -1) {
				fnmake_chanaddr(id, c);
				slog(1, "warn: ", argv0, ": ", queuedesc, ": trouble fsyncing ", fn1.s, "\n", NULL);
				goto fail;
			}
#else
			if (fsync(fdchan[c]) == -1) {
				fnmake_chanaddr(id, c);
				slog(1, "warn: ", argv0, ": ", queuedesc, ": trouble fsyncing ", fn1.s, "\n", NULL);
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
		slog(1, "warn: ", argv0, ": ", queuedesc, ": qmail-clean unable to clean up ", fn1.s, "\n", NULL);
		return;
	}
	pe.id = id;
	pe.dt = now();
	for (c = 0; c < CHANNELS; ++c) {
		if (flagchan[c])
			while (!prioq_insert(min, &pqchan[c], &pe))
				nomem(argv0);
	}
	for (c = 0; c < CHANNELS; ++c) {
		if (flagchan[c])
			break;
	}
	if (c == CHANNELS) {
		while (!prioq_insert(min, &pqdone, &pe))
			nomem(argv0);
	}
	log_stat(&mailfrom, &mailto, id, st.st_size);
	return;

fail:
	if (fdtodo != -1)
		close(fdtodo);
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
	int             qregex;
	char           *x;

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
	if (control_readint(&use_fdatasync, "conf-fdatasync") == -1)
		return 0;
#endif
#ifdef HAVESRS
	if (control_readline(&srs_domain, "srs_domain") == -1)
		return 0;
	if (srs_domain.len && !stralloc_0(&srs_domain))
		return 0;
#endif
	if (control_readint((int *) &lifetime, "queuelifetime") == -1)
		return 0;
	if (!(x = env_get("QREGEX"))) {
		if (control_readint(&qregex, "qregex") == -1)
			return 0;
		if (qregex && !env_put("QREGEX=1"))
			return 0;
	}
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
	if (control_readint(&todo_interval, "todointerval") == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/todointerval\n", NULL);
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
reread()
{
	if (chdir(auto_qmail) == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc,
				": unable to reread controls: unable to switch to ", auto_qmail, ": ",
				error_str(errno), "\n", NULL);
		return;
	}
	regetcontrols();
	chdir_toqueue();
}

/*-
 * scan todo subdir using readsubdir_next
 * set flagtododir, nexttodorun
 *
 * return
 *  0 - Found file
 *  1 - skip todo run
 *  4 - no files found
 * -1 - error
 */
int
todo_scan(int *nfds, fd_set *rfds, unsigned long *id, int mq_flag)
{
	/*- run todo maximal once every N seconds */
	if (todo_interval > 0 && recent < nexttodorun)
		return 1;	/* skip todo run this time */
	if (!flagtododir) {
		/*- we come here at the beginning or after end of a todo scan */
		if (!trigger_pulled(rfds) && recent < nexttodorun)
			return 1;
		trigger_set(); /*- close & open lock/trigger fifo */
		/*- initialize todosubdir */
		readsubdir_init(&todosubdir, "todo", bigtodo, pausedir);
		flagtododir = 1;
		nexttodorun = recent + (todo_interval > 0 ? todo_interval : SLEEP_TODO);
	}
	switch (readsubdir_next(&todosubdir, id))
	{
	case 1: /*- found file with name=*id */
		break;
	case 0: /*- no files in todo/split/ */
		flagtododir = 0;
		/*return 4;*/
		/*- flow through */
	default: /*- error */
		return -1;
	}
	return 0;
}

void
log_stat_todo(unsigned long id, size_t bytes)
{
	char           *ptr;
	char           *mode;

	strnum1[fmt_ulong(strnum1 + 1, id) + 1] = 0;
	strnum2[fmt_ulong(strnum2 + 1, bytes) + 1] = 0;
	*strnum1 = ' ';
	*strnum2 = ' ';
	for (ptr = mailto.s; ptr < mailto.s + mailto.len;) {
		mode = " opendir mode\n";
		slog(1, *ptr == 'L' ? "local: " : "remote: ", mailfrom.len > 3 ? mailfrom.s + 1 : "<>",
				" ", *(ptr + 2) ? ptr + 2 : "<>", strnum1, strnum2,
				" bytes ", queuedesc, mode, NULL);
		ptr += str_len(ptr) + 1;
	}
	mailfrom.len = mailto.len = 0;
}

/*-
 * used by todo processor
 *
 * todo_do calls todo_scan for static queues which
 *
 * 1. scans todo dir recursively
 * 2. sets flagtododir if any file found in todo subdir
 * 3. sets nexttodorun
 *
 * todo_do calls mqueue_scan for dynamic queues which
 * 1. reads mq_queue message queue to get
 *    todo/split/inode injected by qmail-queue
 * 2. sets nexttodorun
 *
 * todo_do will optmimize by writing to comm_buf all files
 * in todo directory before doing comm_do. But can return 3
 * if files in todo is a multiple of todo_chunk_size. This is to
 * prevent comm_buf becoming very large.
 *
 * todo_do returns
 *
 *  0 - stop asap
 *  1 - skip todo run
 *  2 - files in todo > 0
 *  3 - files in todo is a multiple of todo_chunk_size
 *  4 - files in todo = 0
 * -1 - error
 */
int
todo_do_todo(int *nfds, fd_set *rfds)
{
	struct stat     st;
	substdio        ss, ssinfo, sschan[CHANNELS];
	int             fd, fdinfo, match, i, c, split;
	int             fdchan[CHANNELS], flagchan[CHANNELS];
	char            ch;
	char           *ptr;
	unsigned long   id, uid, pid;

	fd = -1;
	fdinfo = -1;
	for (c = 0; c < CHANNELS; ++c)
		fdchan[c] = -1;
	if ((i = todo_scan(nfds, rfds, &id, 0))) /*- skip todo run if this returns 1 */
		return i;
	ptr = readsubdir_name(&todosubdir); /*- split/id */
	if (ptr) {
		scan_int(ptr, &split); /*- actual split value from filename */
		fnmake_todo(id); /*- set fn as todo/split/id */
		scan_int(fn1.s + 5, &i); /*- split as per calculation by fnmake using auto_split */
		todo_log(split != i ? "warn: " : "info: ", argv0, ": ", queuedesc,
			": subdir=todo/", ptr, " fn=", fn1.s,
			split != i ? " incorrect split\n" : "\n", NULL);
		if (split != i) /*- split doesn't match with split calculation in fnmake_todo() */
			return -1;
	} else
		fnmake_todo(id); /*- set fn as todo/id */
	if ((fd = open_read(fn1.s)) == -1) { /*- envelope in todo/split/id */
		todo_log("warn: ", argv0, ": ", queuedesc, ": open ", fn1.s, ": ",
			error_str(errno), "\n", NULL);
		return -1;
	}
	fnmake_mess(id); /*- change fn to mess/split/id */
	/*- just for the statistics, stat on mess/split file */
	if (stat(fn1.s, &st) == -1) {
		todo_log("warn: ", argv0, ": ", queuedesc, ": unable to stat ", fn1.s, ": ",
			error_str(errno), "\n", NULL);
		goto fail;
	}
	for (c = 0; c < CHANNELS; ++c) {
		fnmake_chanaddr(id, c);
		if (unlink(fn1.s) == -1 && errno != error_noent) {
			todo_log("warn: ", argv0, ": ", queuedesc, ": unable to unlink ", fn1.s, ": ",
				error_str(errno), "\n", NULL);
			goto fail;
		}
	}
	fnmake_info(id); /*- now fn is info/split/id */
	if (unlink(fn1.s) == -1 && errno != error_noent) { /*- delete any existing info/split/id */
		todo_log("warn: ", argv0, ": ", queuedesc, ": unable to unlink ", fn1.s, ": ",
			error_str(errno), "\n", NULL);
		goto fail;
	}
	if ((fdinfo = open_excl(fn1.s)) == -1) { /*- create info/split/id */
		todo_log("warn: ", argv0, ": ", queuedesc, ": unable to create ", fn1.s, ": ",
			error_str(errno), "\n", NULL);
		goto fail;
	}
	strnum1[fmt_ulong(strnum1, id)] = 0;
	todo_log("new msg ", strnum1, " ", queuedesc, "\n", NULL);
	for (c = 0; c < CHANNELS; ++c)
		flagchan[c] = 0;
	substdio_fdbuf(&ss, read, fd, todobuf, sizeof (todobuf)); /*- read envelope */
	substdio_fdbuf(&ssinfo, write, fdinfo, todobufinfo, sizeof (todobufinfo)); /*- write to info/split/id */
	uid = 0;
	pid = 0;
	for (;;) {
		if (getln(&ss, &todoline, &match, '\0') == -1) {
			/*- perhaps we're out of memory, perhaps an I/O error */
			fnmake_todo(id); /* todo/split/id */
			todo_log("warn: ", argv0, ": ", queuedesc, ": trouble reading ", fn1.s, ": ",
				error_str(errno), "\n", NULL);
			goto fail;
		}
		if (!match)
			break;
		switch (todoline.s[0])
		{
		case 'u': /*- uid */
			scan_ulong(todoline.s + 1, &uid);
			break;
		case 'p': /*- process ID */
			scan_ulong(todoline.s + 1, &pid);
			break;
		case 'h': /*- envheader */
		case 'e': /*- qqeh */
			if (substdio_put(&ssinfo, todoline.s, todoline.len) == -1) {
				todo_log("warn: ", argv0, ": ", queuedesc, ": trouble writing to ", fn1.s, ": ",
					error_str(errno), "\n", NULL);
				goto fail;
			}
			break;
		case 'F': /*- from */
			if (substdio_put(&ssinfo, todoline.s, todoline.len) == -1) {
				todo_log("warn: ", argv0, ": ", queuedesc, ": trouble writing to ", fn1.s, ": ",
					error_str(errno), "\n", NULL);
				goto fail;
			}
			if (!stralloc_copy(&mailfrom, &todoline) || !stralloc_0(&mailfrom)) {
				nomem(argv0);
				goto fail;
			}
			/*- write data to comm_buf, which will be written to slowq-send later */
			comm_info_todo(id, (unsigned long) st.st_size, todoline.s + 1, pid, uid);
			break;
		case 'T':
			/*-
			 * 1. check address in control/locals, controls/virtualdomains
			 * 2. rewrite address in rwline
			 */
			switch (rewrite(todoline.s + 1))
			{
			case 0:
				nomem(argv0);
				goto fail;
			case 2: /* Sea */
				if (!stralloc_cats(&mailto, "R") || !stralloc_cat(&mailto, &todoline)) {
					nomem(argv0);
					goto fail;
				}
				c = 1;
				break;
			default: /* Land */
				if (!stralloc_cats(&mailto, "L") || !stralloc_cat(&mailto, &todoline)) {
					nomem(argv0);
					goto fail;
				}
				c = 0;
				break;
			}
			if (fdchan[c] == -1) {
				/*- create local/split/id or remote/split/id */
				fnmake_chanaddr(id, c);
				if ((fdchan[c] = open_excl(fn1.s)) == -1) {
					todo_log("warn: ", argv0, ": ", queuedesc, ": unable to create ", fn1.s, ": ",
						error_str(errno), "\n", NULL);
					goto fail;
				}
				substdio_fdbuf(&sschan[c], write, fdchan[c], todobufchan[c], sizeof (todobufchan[c]));
				flagchan[c] = 1;
			}
			if (substdio_bput(&sschan[c], rwline.s, rwline.len) == -1) {
				fnmake_chanaddr(id, c);
				todo_log("warn: ", argv0, ": ", queuedesc, ": trouble writing to ", fn1.s, ": ",
					error_str(errno), "\n", NULL);
				goto fail;
			}
			break;
		default:
			fnmake_todo(id); /* todo/split/id */
			todo_log("warn: ", argv0, ": ", queuedesc, ": unknown record type in ", fn1.s, ": ",
				error_str(errno), "\n", NULL);
			goto fail;
		}
	}
	close(fd);
	fd = -1;
	fnmake_info(id); /* info/split/id */
	if (substdio_flush(&ssinfo) == -1) {
		todo_log("warn: ", argv0, ": ", queuedesc, ": trouble writing to ", fn1.s, ": ",
			error_str(errno), "\n", NULL);
		goto fail;
	}
	for (c = 0; c < CHANNELS; ++c) {
		if (fdchan[c] != -1) {
			if (substdio_flush(&sschan[c]) == -1) {
				fnmake_chanaddr(id, c);
				todo_log("warn: ", argv0, ": ", queuedesc, ": trouble writing to ", fn1.s, ": ",
					error_str(errno), "\n", NULL);
				goto fail;
			}
		}
	}
#ifdef USE_FSYNC
	if ((use_fsync > 0 || use_fdatasync > 0) && (use_fdatasync ? fdatasync(fdinfo) : fsync(fdinfo)) == -1) {
		todo_log("warn: ", argv0, ": ", queuedesc, ": trouble fsyncing ", fn1.s, ": ",
			error_str(errno), "\n", NULL);
		goto fail;
	}
#else
	if (fsync(fdinfo) == -1) {
		todo_log("warn: ", argv0, ": ", queuedesc, ": trouble fsyncing ", fn1.s, ": ",
			error_str(errno), "\n", NULL);
		goto fail;
	}
#endif
	close(fdinfo);
	fdinfo = -1;
	for (c = 0; c < CHANNELS; ++c) {
		if (fdchan[c] != -1) {
#ifdef USE_FSYNC
			if ((use_fsync > 0 || use_fdatasync > 0) && (use_fdatasync ? fdatasync(fdchan[c]) : fsync(fdchan[c])) == -1) {
				fnmake_chanaddr(id, c);
				todo_log("warn: ", argv0, ": ", queuedesc, ": trouble fsyncing ", fn1.s, ": ",
					error_str(errno), "\n", NULL);
				goto fail;
			}
#else
			if (fdatasync(fdchan[c]) == -1) {
				fnmake_chanaddr(id, c);
				todo_log("warn: ", argv0, ": ", queuedesc, ": trouble fsyncing ", fn1.s, ": ",
					error_str(errno), "\n", NULL);
				goto fail;
			}
#endif
			close(fdchan[c]);
			fdchan[c] = -1;
		}
	}
	fnmake_todo(id); /* todo/split/id */
	if (substdio_putflush(&sstoqc, fn1.s, fn1.len) == -1) {
		cleandied();
		return -1;
	}
	if (substdio_get(&ssfromqc, &ch, 1) != 1) {
		cleandied();
		return -1;
	}
	if (ch != '+') {
		todo_log("warn: ", argv0, ": ", queuedesc, ": qmail-clean unable to clean up ", fn1.s, ": ",
			error_str(errno), "\n", NULL);
		return -1;
	}
	comm_write_todo(id, flagchan[0], flagchan[1]); /*- e.g. "DL656826\0" */
	/*- "Llocal: mbhangui@argos.indimail.org mbhangui@argos.indimail.org 798 queue1\n\0" */
	log_stat_todo(id, st.st_size);
	/*-
	 * return in chunks of todo_chunk_size
	 * so that todo-processor doesn't spend to much time in building
	 * comm_buf without sending a single email for delivery. This
	 * will avoid slow delivery when todo-processor/slowq-send wasn't running
	 * and large number of emails have been injected into the queue.
	 */
	return (flagtododir == 0 ? 4 : (comm_count_todo % todo_chunk_size) == 0 ? 3 : 2);

fail:
	if (fd != -1)
		close(fd);
	if (fdinfo != -1)
		close(fdinfo);
	for (c = 0; c < CHANNELS; ++c) {
		if (fdchan[c] != -1)
			close(fdchan[c]);
	}
	return -1;
}

void
todo_processor(int *pi1, int *pi2, int lockfd)
{
	fd_set          rfds, wfds;
	datetime_sec    wakeup;
	int             nfds, r;
	char            c;
	struct timeval  tv;

	switch((todopid = fork()))
	{
	case -1:
		slog(1, "alert: ", argv0, ": ", queuedesc, ": unable to fork\n", NULL);
		_exit(111);
	case 0: /*- ps will show this as slowq-todo */
		mypid[fmt_ulong(mypid, getpid())] = 0;
		str_copyb(argv0 + 6, "todo", 4);
		sendfdi = pi2[0];
		sendfdo = pi1[1];
		close(pi1[0]);
		close(pi2[1]);
		close(lockfd);
		sig_pipeignore();
		sig_childdefault();
		sig_alarmdefault();
		sig_catch(sig_usr1, SIG_DFL);
		sig_catch(sig_usr2, SIG_DFL);
		sig_termcatch(sigterm_todo);
		sig_hangupcatch(sighup);
		todo_init(); /*- initialize flagtododir, lasttodorun, nexttodorun, open lock/trigger */
		for (;;) {
			/*- read from fd 0 (slowq-send) */
			if ((r = read(sendfdi, &c, 1)) == -1) {
				if (errno == error_intr)
					continue;
				comm_die_send(100);	/*- read failed probably slowq-send died */
			}
			if (!r)
				comm_die_send(100);	/*- read failed probably slowq-send died */
			break;
		}
		if (c != 'C') {
			todo_log("warn: ", argv0, ": ", queuedesc, ": slowq-send speaks an obscure dialect\n", NULL);
			comm_die_send(100);
		}
		while (!flagexitsend) {
			recent = now();
			if (flagreadasap) {
				flagreadasap = 0;
				reread();
			}
			if (!flagsendalive) {
				/*- slowq-send finally exited, so do the same. */
				if (flagexittodo)
					_exit(0);
				/*- slowq-send died. We can not log and we can not work therefor _exit(1). */
				_exit(1);
			}
			wakeup = recent + SLEEP_FOREVER;
			FD_ZERO(&rfds);
			FD_ZERO(&wfds);
			nfds = 1;
			todo_selprep_todo(&nfds, &rfds, &wakeup);
			comm_selprep_send(&nfds, &wfds, &rfds);
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
				} else
					todo_log("warn: ", argv0, ": ", queuedesc, ": trouble in select\n", NULL);
			} else {
				recent = now();
				while (todo_do_todo(&nfds, &rfds) == 2);
				comm_read_todo(&wfds, &rfds); /*- communicate with slowq-send on fd 0, fd 1 */
			}
		} /*- while (!flagexitsend) */
		todo_log("info: ", argv0, ": pid ", mypid, " ", queuedesc, " exiting\n", NULL);
		_exit(0);
	default:
		close(pi1[1]);
		close(pi2[0]);
		break;
	}
	flagtodoalive = 1;
	return;
}

static void
sigusr1()
{
	if (flagdetached == 1)
		return;
	sig_block(sig_usr1);

	if (del_canexit()) {
		slog(1, "alert: ", argv0, ": ", queuedesc,
				": no pending deliveries. stop scheduling new deliveries...\n", NULL);
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
	/*- tell todo-processor to stop sending jobs */
	if (write(todofdo, "D", 1) != 1) {
		slog(1, "alert: ", argv0, ": ", queuedesc,
				": unable to write two bytes to todo-processor! dying...: ",
				error_str(errno), "\n", NULL);
		flagexitsend = 1;
		flagtodoalive = 0;
	}
	sig_unblock(sig_usr1);
	if (!del_canexit())
		sig_block(sig_usr2);
}

static void
sigusr2()
{
	if (flagdetached == 0)
		return;
	sig_block(sig_usr2);
	flagdetached = 0;
	/*-
	 * we need to rescan todo if we were earlier in detached mode
	 * add jobs from todo
	 */
	pqstart();
	/*- tell todo-processor to start sending jobs */
	if (write(todofdo, "A", 1) != 1) {
		slog(1, "alert: ", argv0, ": ", queuedesc,
				": unable to write two bytes to todo-processor! dying...: ",
				error_str(errno), "\n", NULL);
		flagexitsend = 1;
		flagtodoalive = 0;
	}
	sig_unblock(sig_usr2);
	sig_unblock(sig_usr1);
}

static void
tododied()
{
	if (!flagexitsend)
		slog(1, "alert: ", argv0, ": ", queuedesc,
			": oh no! lost todo-processor connection! dying...\n", NULL);
	flagexitsend = 1;
	flagtodoalive = 0;
}

static void
sigchld()
{
	int             wstat;
	pid_t           pid;

	while ((pid = wait_nohang(&wstat)) > 0) {
		if (pid == todopid) {
			tododied();
			break;
		}
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
		slog(1, "warn: ", argv0, ": ", queuedesc, ": todo-processor speaks an obscure dialect\n", NULL);
		return;
	}
	len = scan_ulong(s, &id);
	if (!len || s[len]) {
		slog(1, "warn: ", argv0, ": ", queuedesc, ": todo-processor speaks an obscure dialect\n", NULL);
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
todo_do_send(fd_set *rfds)
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
		 * todo-processor is responsible for keeping it short
		 * e.g. todo-processor writes a line like this
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
			case 'X': /*- todo-processor is exiting */
				if (flagexitsend)
					flagtodoalive = 0;
				else
					tododied(); /*- sets flagexitsend, flagtodoalive */
				break;
			default:
				slog(1, "warn: ", argv0, ": ", queuedesc, ": todo-processor speaks an obscure dialect\n", NULL);
				break;
			}
			todoline.len = 0;
		}
	} /*- for (i = 0; i < r; ++i) */
}

int
main(int argc, char **argv)
{
	int             lockfd, nfds, c, can_exit;
	int             pi1[2], pi2[2];
	datetime_sec    wakeup;
	fd_set          rfds, wfds;
	struct timeval  tv;
	char           *ptr;

	mypid[fmt_ulong(mypid, getpid())] = 0;
	if (!(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue/slowq"; /*- single queue like qmail */
	ptr = env_get("RATELIMIT_DIR");
	do_ratelimit = (ptr && *ptr) ? 1 : 0;
	c = str_rchr(argv[0], '/');
	argv0 = (argv[0][c] && argv[0][c + 1]) ? argv[0] + c + 1 : argv[0];
	if (!(ptr = env_get("TODO_PROCESSOR")))
		todoproc = 0;
	else
		scan_int(ptr, &todoproc);
	/*- get basename of queue directory to define slowq-send instance */
	for (queuedesc = queuedir; *queuedesc; queuedesc++);
	for (; queuedesc != queuedir && *queuedesc != '/'; queuedesc--);
	if (*queuedesc == '/')
		queuedesc++;
	if (chdir(auto_qmail) == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc,
				": cannot start: unable to switch to ", auto_qmail, ": ",
				error_str(errno), "\n", NULL);
		_exit(111);
	}
	getEnvConfigInt(&bigtodo, "BIGTODO", 1);
	getEnvConfigInt(&conf_split, "CONFSPLIT", auto_split);
	if (conf_split > auto_split)
		conf_split = auto_split;
	strnum1[fmt_ulong(strnum1, conf_split)] = 0;
	getEnvConfigInt(&todo_chunk_size, "TODO_CHUNK_SIZE", CHUNK_SIZE);
	if (todo_chunk_size <= 0)
		todo_chunk_size = CHUNK_SIZE;
	slog(1, "info: ", argv0, ": pid ", mypid, ": ", queuedir,
			": todo processor: ", todoproc ? "YES" : "NO",
			", ratelimit=", do_ratelimit ? "ON" : "OFF",
			", conf split=", strnum1, bigtodo ? ", bigtodo=1\n" : ", bigtodo=0\n", NULL);
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
		slog(1, "alert: ", argv0, ": ", queuedesc, ": cannot start: unable to read controls\n", NULL);
		_exit(111);
	}
	if (chdir(queuedir) == -1) {
		slog(1, "alert: ", argv0, ": ", queuedesc,
				": cannot start: unable to switch to queue directory ", queuedir, ": ",
				error_str(errno), "\n", NULL);
		_exit(111);
	}
	sig_pipeignore();
	sig_termcatch(sigterm);
	sig_alarmcatch(sigalrm);
	sig_hangupcatch(sighup);
	sig_childcatch(sigchld);
	if (todoproc) {
		sig_block(sig_usr1);
		sig_block(sig_usr2);
		sig_catch(sig_usr1, sigusr1);
		sig_catch(sig_usr2, sigusr2);
	}
	umask(077);
	/*- prevent multiple copies of slowq-send to run */
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

	fnmake_init();  /*- initialize fn1, fn2 */
	comm_init();    /*- assign fd 5 to queue comm to, 6 to queue comm from */
	pqstart();      /*- add files from info/split for processing */
	job_init();     /*- initialize numjobs job structures */
	del_init();     /*- initialize concurrencylocal + concurrrencyremote delivery structure */
	pass_init();    /*- initialize pass structure */
	if (todoproc) {
		if (pipe(pi1) == -1) {
			slog(1, "alert: ", argv0, ": ", queuedesc, ": failed to create pipe1: ", error_str(errno), "\n", NULL);
			_exit(111);
		}
		if (pipe(pi2) == -1) {
			slog(1, "alert: ", argv0, ": ", queuedesc, ": failed to create pipe2: ", error_str(errno), "\n", NULL);
			_exit(111);
		}
		todofdi = pi1[0];
		todofdo = pi2[1];
		todo_processor(pi1, pi2, lockfd); /*- this will wait till todo_init_send sends the "C" character */
		todo_init_send();
	} else
		todo_init(); /*- initialize flagtododir, lasttodorun, nexttodorun, open lock/trigger */
	cleanup_init(); /*- initialize flagcleanup = 0, cleanuptime = now*/
	if (todoproc) {
		sig_unblock(sig_usr1);
		sig_unblock(sig_usr2);
	}
	can_exit = del_canexit();
	while (!flagexitsend || !can_exit || flagtodoalive) { /*- stay alive if delivery, todo jobs are present */
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
		comm_selprep_spawn(&nfds, &wfds);
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
		 */
		if (todoproc)
			todo_selprep_send(&nfds, &rfds, &wakeup);
		else
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
			} else
				slog(1, "warn: ", argv0, ": ", queuedesc, ": trouble in select\n", NULL);
		} else {
			time_needed = 0;
			recent = now();
			/*- communicate with qmail-lspawn, qmail-rspawn
			 * These were created as pi1, pi3 in qmail-start.c
			 * write on fd 1 read by fd 0 in qmail-lspawn,
			 * write on fd 3 read by fd 0 in qmail-rspawn
			 */
			comm_do_spawn(&wfds);
			/*-
			 * if data is available for read on fd 2 or 4
			 * then read from qmail-lspawn or qmail-rspawn
			 * fd 2 - pi2[0] - data from qmail-lspawn
			 * fd 4 - pi4[0] - data from qmail-rspawn
			 */
			del_do(&rfds);
			/*-
			 * scan todo directory
			 * classify and rewrite todoline
			 */
			if (todoproc)
				todo_do_send(&rfds);
			else
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
			slog(1, "info: ", argv0, ": ", queuedesc,
					": no pending jobs, attaching back to todo-proc\n", NULL);
		}
	} /*- while (!flagexitsend || !del_canexit()) */
	pqfinish();
	slog(1, "info: ", argv0, ": pid ", mypid, " ", queuedesc, " exiting\n", NULL);
	return (0);
}

void
getversion_slowq_send_c()
{
	static char    *x = "$Id: slowq-send.c,v 1.36 2023-12-25 09:31:28+05:30 Cprogrammer Exp mbhangui $";

	x = sccsiddelivery_rateh;
	x = sccsidgetdomainth;
	if (x)
		x++;
}

/*
 * $Log: slowq-send.c,v $
 * Revision 1.36  2023-12-25 09:31:28+05:30  Cprogrammer
 * made OSSIFIED configurable
 *
 * Revision 1.35  2023-12-23 23:25:37+05:30  Cprogrammer
 * log pid during startup
 *
 * Revision 1.34  2023-12-23 20:20:09+05:30  Cprogrammer
 * terminate todo process when spawn/clean process terminates
 *
 * Revision 1.33  2023-12-21 19:53:01+05:30  Cprogrammer
 * added concept of half-detached/full-detached
 * se value of TODO_PROCESSOR env variable to run todo function
 *
 * Revision 1.32  2023-10-30 10:30:18+05:30  Cprogrammer
 * use qregex control file to set QREGEX env variable
 *
 * Revision 1.31  2023-10-02 22:50:46+05:30  Cprogrammer
 * fix copy of srs_result
 *
 * Revision 1.30  2023-03-08 20:04:03+05:30  Cprogrammer
 * fixed but with handling SRS address
 *
 * Revision 1.29  2023-01-15 23:26:06+05:30  Cprogrammer
 * use slog() function with varargs to log error messages
 * use todo_log() function with varargs to communicate with todo process
 *
 * Revision 1.28  2022-11-24 08:49:14+05:30  Cprogrammer
 * changed variable type to c when reading from slowq-send process
 *
 * Revision 1.27  2022-11-22 19:06:39+05:30  Cprogrammer
 * corrected name (qmail-send -> slowq-send)
 *
 * Revision 1.26  2022-09-27 12:49:40+05:30  Cprogrammer
 * auto attach to todo-processor when there are no pending delivery jobs
 *
 * Revision 1.25  2022-09-26 09:17:34+05:30  Cprogrammer
 * added dedicated todo processor
 *
 * Revision 1.25  2022-09-25 23:59:13+05:30  Cprogrammer
 * added dedicated todo processor
 *
 * Revision 1.24  2022-04-23 22:52:18+05:30  Cprogrammer
 * added pid in log when quitting
 *
 * Revision 1.23  2022-04-13 08:01:13+05:30  Cprogrammer
 * set delayed flag to 0 for new jobs
 *
 * Revision 1.22  2022-04-04 14:33:47+05:30  Cprogrammer
 * added setting of fdatasync() instead of fsync()
 *
 * Revision 1.21  2022-04-04 00:51:51+05:30  Cprogrammer
 * display queuedir in logs
 *
 * Revision 1.20  2022-03-31 00:08:53+05:30  Cprogrammer
 * replaced fsync() with fdatasync()
 *
 * Revision 1.19  2022-03-13 19:54:51+05:30  Cprogrammer
 * display bigtodo value in logs on startup
 *
 * Revision 1.18  2022-01-30 09:40:18+05:30  Cprogrammer
 * make USE_FSYNC, USE_SYNCDIR consistent across programs
 * allow configurable big/small todo/intd
 * fixed signal sent to child
 *
 * Revision 1.17  2021-11-02 17:57:22+05:30  Cprogrammer
 * use argv0 for program name
 *
 * Revision 1.16  2021-10-20 22:49:43+05:30  Cprogrammer
 * add program 'slowq-send' in logs for identifcation.
 *
 * Revision 1.15  2021-08-28 23:08:17+05:30  Cprogrammer
 * moved dtype enum delivery variable from variables.h to getDomainToken.h
 *
 * Revision 1.14  2021-08-13 18:26:02+05:30  Cprogrammer
 * turn off ratelimit if RATELIMIT_DIR is set but empty
 *
 * Revision 1.13  2021-07-26 23:25:36+05:30  Cprogrammer
 * log when log sighup, sigalrm is caught
 *
 * Revision 1.12  2021-07-17 14:38:09+05:30  Cprogrammer
 * skip processing of for messages queued with wrong split dir
 *
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
 * moved delivery_rate to delivery_rate.c
 *
 * Revision 1.1  2021-05-31 17:06:24+05:30  Cprogrammer
 * Initial revision
 *
 */
