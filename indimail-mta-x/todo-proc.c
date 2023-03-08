/*
 * $Id: todo-proc.c,v 1.66 2023-01-15 23:30:27+05:30 Cprogrammer Exp mbhangui $
 */
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <alloc.h>
#include <byte.h>
#include <sig.h>
#include <constmap.h>
#include <direntry.h>
#include <error.h>
#include <fmt.h>
#include <getln.h>
#include <open.h>
#include <ndelay.h>
#include <now.h>
#include <time.h>
#include <scan.h>
#include <select.h>
#include <str.h>
#include <stralloc.h>
#include <substdio.h>
#include <env.h>
#include <getEnvConfig.h>
#include <sgetopt.h>
#include "haslibrt.h"
#ifdef HASLIBRT
#include <mqueue.h>
#include "qscheduler.h"
#endif
#include "auto_qmail.h"
#include "auto_control.h"
#include "auto_split.h"
#include "control.h"
#include "fmtqfn.h"
#include "readsubdir.h"
#include "variables.h"
#include "trigger.h"
#ifdef USE_FSYNC
#include "syncdir.h"
#endif
#include "varargs.h"

/*- critical timing feature #1: if not triggered, do not busy-loop */
/*- critical timing feature #2: if triggered, respond within fixed time */
/*- important timing feature: when triggered, respond instantly */
#define SLEEP_TODO     1500  /*- check todo/ every 25 minutes in any case */
#define ONCEEVERY        10  /*- Run todo maximal once every N seconds */
#define CHUNK_WAIT        1
#define SLEEP_FOREVER 86400  /*- absolute maximum time spent in select() */
#define SLEEP_SYSFAIL   123
#define CHUNK_SIZE      1

static stralloc percenthack = { 0 };
typedef struct  constmap cmap;

static cmap     mappercenthack;
static cmap     maplocals;
static stralloc locals = { 0 };
static cmap     mapvdoms;
static stralloc vdoms = { 0 };
static stralloc envnoathost = { 0 };

static char     strnum1[FMT_ULONG];
static char     strnum2[FMT_ULONG];

/*- if qmail-send.c changes this has to be updated */
#define CHANNELS 2
static char    *chanaddr[CHANNELS] = { "local/", "remote/" };

static int      flagexittodo = 0;
static int      flagdetached = 0; /*- allow todo-proc to stop sending new jobs to qmail-send */
static char    *queuedesc;
static char    *argv0 = "todo-proc";
static datetime_sec recent, nexttodorun;

static stralloc fn = { 0 };
static stralloc rwline = { 0 };
static substdio sstoqc;
static substdio ssfromqc;
static char     sstoqcbuf[1024];
static char     ssfromqcbuf[1024];
static stralloc comm_buf = { 0 };
static int      comm_pos;
static int      comm_count;
static int      todo_chunk_size;
static int      sendfdo = -1; /*- write fd to   qmail-send */
static int      sendfdi = -1; /*- read  fd from qmail-send */

static int      flagsendalive = 1;
static int      flagtododir = 0; /*- if 0, have to readsubdir_init again */
static int      todo_interval = -1;
static int      bigtodo;
static readsubdir todosubdir;
static stralloc todoline = { 0 };
static char     todobuf[SUBSTDIO_INSIZE];
static char     todobufinfo[512];
static char     todobufchan[CHANNELS][1024];
static stralloc mailfrom = { 0 };
static stralloc mailto = { 0 };
static stralloc newlocals = { 0 };
static stralloc newvdoms = { 0 };
static int      dynamic_queue = 0;
static int      chunk_wait = CHUNK_WAIT;
#ifdef HASLIBRT
static int      do_readsubdir = 1;
static mqd_t    mq_queue = (mqd_t)-1;
static char    *msgbuf;
static int      msgbuflen;
static stralloc qfn = { 0 };
/*-
 * compat_mode_flag
 * 0 - indicator of being in mqueue mode
 * 1 - indicator of being in readsubdir mode
 * 2 - pure dynamic mode
 * default is to run dynamic mode without compat
 */
static int      compat_mode_flag = 2;
clockid_t       clock_id;
#endif

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

	pos = comm_buf.len;
	if (!stralloc_cats(&comm_buf, "L") ||
			!stralloc_cats(&comm_buf, s1))
		goto fail;

	while (1) {
		str = va_arg(ap, char *);
		if (!str)
			break;
		if (!stralloc_cats(&comm_buf, str))
			goto fail;
	}
	if (!stralloc_0(&comm_buf))
		goto fail;
	va_end(ap);
	return;
fail:
	va_end(ap);
	/*- either all or nothing */
	comm_buf.len = pos;
}

void
sigterm(void)
{
	sig_block(sig_term);
	strnum1[fmt_ulong(strnum1, getpid())] = 0;
	todo_log("alert: ", argv0, ": pid ", strnum1, " got TERM: ", queuedesc, "\n", 0);
	if (!flagexittodo)
		todo_log("info: ", argv0, ": ", queuedesc, " stop todo processing asap\n", 0);
	flagexittodo = 1;
}

int             flagreadasap = 0;
void
sighup(void)
{
	flagreadasap = 1;
	strnum1[fmt_ulong(strnum1, getpid())] = 0;
	todo_log("alert: ", argv0, ": pid ", strnum1, " got HUP: ", queuedesc, "\n", 0);
}

void
senddied(void)
{
	flagsendalive = 0;
}

/*
 * taken from qsutil.c
 */
void
nomem()
{
	todo_log("alert: ", argv0, ": ", queuedesc, ": out of memory, sleeping...\n", 0);
	sleep(10);
}

void
pausedir(char *dir)
{
	todo_log("alert: ", argv0, ": ", queuedesc, ": unable to opendir ", dir, ", sleeping...\n", 0);
	sleep(10);
}

void
cleandied()
{
	todo_log("alert: ", argv0, ": ", queuedesc, ": oh no! lost qmail-clean connection! dying...\n", 0);
	flagexittodo = 1;
}

/*
 * fnmake functions from qmail-send.c
 */
void
fnmake_init(void)
{
	while (!stralloc_ready(&fn, FMTQFN))
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
	fn.len = fmtqfn(fn.s, "todo/", id, bigtodo);
}

void
fnmake_mess(unsigned long id)
{
	fn.len = fmtqfn(fn.s, "mess/", id, 1);
}

void
fnmake_chanaddr(unsigned long id, int c)
{
	fn.len = fmtqfn(fn.s, chanaddr[c], id, 1);
}


/*-
 * 1 if by land, 2 if by sea, 0 if out of memory. not allowed to barf.
 * may trash recip. must set up rwline, between a T and a \0.
 * taken from qmail-send.c
 */
int
rewrite(char *recip)
{
	int             i;
	int             j;
	char           *x;
	static stralloc addr = { 0 };
	int             at;

	if (!stralloc_copys(&rwline, "T") ||
			!stralloc_copys(&addr, recip))
		return 0;
	i = byte_rchr(addr.s, addr.len, '@');
	if (i == addr.len) {
		if (!stralloc_cats(&addr, "@") ||
				!stralloc_cat(&addr, &envnoathost))
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
		if (!stralloc_cat(&rwline, &addr) ||
				!stralloc_0(&rwline))
			return 0;
		return 1;
	}

	for (i = 0; i <= addr.len; ++i) {
		if (!i || (i == at + 1) || (i == addr.len) || ((i > at) && (addr.s[i] == '.'))) {
			if ((x = constmap(&mapvdoms, addr.s + i, addr.len - i))) {
				if (!*x)
					break;
				if (!stralloc_cats(&rwline, x) ||
						!stralloc_cats(&rwline, "-") ||
						!stralloc_cat(&rwline, &addr) ||
						!stralloc_0(&rwline))
					return 0;
				return 1;
			}
		}
	}
	if (!stralloc_cat(&rwline, &addr) ||
			!stralloc_0(&rwline))
		return 0;
	return 2;
}

static int
issafe(char ch)
{
	/*- general principle: allman's code is crap */
	if (ch == '%' || ch < 33 || ch > 126)
		return 0;
	return 1;
}

/*
 * comm functions taken from qmail-send.c
 *
 * idea of using comm_buf to communicate with qmail-send
 * comes from ext-todo patch
 * Claudio Jeker <jeker@n-r-g.com and Andre Oppermann ext-todo.
 * This is exactly how qmail-send communicates with qmail-lspawn, qmail-rspawn
 */
void
comm_init(void)
{
	/* fd 2 is pi9[1] - write to qmail-clean, fd 3 is pi10[0] - read from qmail-clean */
	substdio_fdbuf(&sstoqc, write, 2, sstoqcbuf, sizeof (sstoqcbuf));
	substdio_fdbuf(&ssfromqc, read, 3, ssfromqcbuf, sizeof (ssfromqcbuf));

	sendfdo = 1;	/*- stdout pi8[1] to qmail-send */
	sendfdi = 0;	/*- stdin  pi7[0] from qmail-send */
	/*- this is so stupid: NDELAY semantics should be default on write */
	if (ndelay_on(sendfdo) == -1)
		senddied(); /*- set flagsendalive = 0. drastic but avoids risking deadlock */
	while (!stralloc_ready(&comm_buf, 1024))
		nomem();
}

/*- XXX: returns true if there is something in the buffer */
int
comm_canwrite(void)
{
	if (!flagsendalive)
		return 0;
	/*- XXX: could allow a bigger buffer; say 10 recipients */
	if (comm_buf.s && comm_buf.len)
		return 1;
	return 0;
}

void
comm_write(unsigned long id, int local, int remote)
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
	pos = comm_buf.len;
	strnum1[fmt_ulong(strnum1, id)] = 0;
	if (!stralloc_cats(&comm_buf, "D") ||
			!stralloc_cats(&comm_buf, s) ||
			!stralloc_cats(&comm_buf, strnum1) ||
			!stralloc_0(&comm_buf))
		goto fail;
	return;
fail:
	/*- either all or nothing */
	comm_buf.len = pos;
}

void
comm_info(unsigned long id, unsigned long size, char *from, unsigned long pid, unsigned long uid)
{
	int             pos;
	int             i;

	comm_count++;
	pos = comm_buf.len;
	if (!stralloc_cats(&comm_buf, "Linfo msg "))
		goto fail;
	strnum1[fmt_ulong(strnum1, id)] = 0;
	strnum2[fmt_ulong(strnum2, size)] = 0;
	if (!stralloc_cats(&comm_buf, strnum1) ||
			!stralloc_cats(&comm_buf, ": bytes ") ||
			!stralloc_cats(&comm_buf, strnum2) ||
			!stralloc_cats(&comm_buf, " from <"))
		goto fail;
	i = comm_buf.len;
	if (!stralloc_cats(&comm_buf, from))
		goto fail;
	for (; i < comm_buf.len; ++i) {
		if (comm_buf.s[i] == '\n')
			comm_buf.s[i] = '/';
		else
		if (!issafe(comm_buf.s[i]))
			comm_buf.s[i] = '_';
	}
	if (!stralloc_cats(&comm_buf, "> qp "))
		goto fail;
	strnum1[fmt_ulong(strnum1, pid)] = 0;
	strnum2[fmt_ulong(strnum2, uid)] = 0;
	if (!stralloc_cats(&comm_buf, strnum1) ||
			!stralloc_cats(&comm_buf, " uid ") ||
			!stralloc_cats(&comm_buf, strnum2) ||
			!stralloc_cats(&comm_buf, " ") ||
			!stralloc_cats(&comm_buf, queuedesc) ||
			!stralloc_cats(&comm_buf, "\n") ||
			!stralloc_0(&comm_buf))
		goto fail;
	return;

fail:
	/*- either all or nothing */
	comm_buf.len = pos;
}

void
comm_exit(void)
{
	if (!stralloc_cats(&comm_buf, "X") ||
			!stralloc_0(&comm_buf))
		_exit(1);
}

void
comm_selprep(int *nfds, fd_set *wfds, fd_set *rfds)
{
	if (flagsendalive) {
		int             c;

		c = comm_canwrite();
		if (flagexittodo && c == 0)
			comm_exit();
		if (c) {
			FD_SET(sendfdo, wfds); /*- write fd to qmail-send */
			if (*nfds <= sendfdo)
				*nfds = sendfdo + 1;
		}
		FD_SET(sendfdi, rfds); /*- read fd from qmail-send */
		if (*nfds <= sendfdi)
			*nfds = sendfdi + 1;
	}
}

void
comm_do(fd_set *wfds, fd_set *rfds)
{
	if (flagsendalive && comm_canwrite()) {
		if (FD_ISSET(sendfdo, wfds)) {
			int             w;
			int             len;

			len = comm_buf.len;
			if ((w = write(sendfdo, comm_buf.s + comm_pos, len - comm_pos)) <= 0) {
				if ((w == -1) && (errno == error_pipe))
					senddied(); /*- set flagsendalive = 0 */
			} else {
				comm_pos += w;
				if (comm_pos == len) {
					comm_buf.len = 0;
					comm_pos = 0;
					comm_count = 0;
				}
			}
		}
	}
	/*- next read from qmail-send */
	if (flagsendalive && FD_ISSET(sendfdi, rfds)) {
		/*- handle 'A', 'D', 'H' and 'X' */
		char            c;
		int             r;
		if ((r = read(sendfdi, &c, 1)) <= 0) {
			if (!r || (r == -1 && errno != error_intr))
				senddied(); /*- set flagsendalive = 0 */
		} else {
			switch (c)
			{
			case 'A': /*- attached mode */
				flagdetached = 0;
				strnum1[fmt_ulong(strnum1, getpid())] = 0;
				todo_log("alert: ", argv0, ": pid ", strnum1,
					" got 'A' command: todo-proc attached: ",
					queuedesc, "\n", 0);
				break;
			case 'D': /*- detached mode */
				flagdetached = 1;
				strnum1[fmt_ulong(strnum1, getpid())] = 0;
				todo_log("alert: ", argv0, ": pid ", strnum1,
					" got 'D' command: todo-proc detached: ",
					queuedesc, "\n", 0);
				break;
			case 'H':
				sighup(); /*- set flagreadasap = 1 */
				break;
			case 'X':
				sigterm(); /*-set flagexittodo = 1 */
				break;
			default:
				todo_log("warn: ", argv0, ": ", queuedesc,
					": qmail-send speaks an obscure dialect\n", 0, 0);
				break;
			}
		}
	}
}

/*-
 * shutdown communication channel with qmail-send
 * and exit
 */
void
comm_die(int i)
{
	fd_set          rfds, wfds;
	int             nfds;

	sendfdo = 1;
	sendfdi = 0;
	if (ndelay_on(sendfdo) == -1)
		senddied(); /*- set flagsendalive = 0. drastic but avoids risking deadlock */
	while (!stralloc_ready(&comm_buf, 1024))
		nomem();
	comm_exit();
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	nfds = 1;
	comm_selprep(&nfds, &wfds, &rfds);
	comm_do(&wfds, &rfds);
	todo_log("info: ", argv0, ": ", queuedesc, " stop todo processing asap\n", 0);
	_exit(i);
}

void
todo_init(void)
{
	flagtododir = 0;
	nexttodorun = now();
	trigger_set(); /*- close & open lock/trigger fifo */
}

#ifdef HASLIBRT
void
mqueue_init(void)
{
	char            mq_name[FMT_ULONG + 6];
	char           *s;
	int             i;

	s = mq_name;
	i = fmt_str(s, "/");
	s += i;
	i = fmt_str(s, queuedesc);
	s += i;
	*s++ = 0;
	i = 0;
	/*- message queue /queueN for communication between todo-proc, qmail-queue */
	if ((mq_queue = mq_open(mq_name, O_RDONLY,  0600, NULL)) == (mqd_t)-1) {
		todo_log("alert: ", argv0, ": failed to open POSIX message queue ",
			mq_name, ": ", error_str(errno), "\n", 0);
		comm_die(111);
	}
	nexttodorun = now();
	if (clock_getcpuclockid(0, &clock_id)) {
		todo_log("alert: ", argv0, ": failed to get clockid: ", error_str(errno), "\n", 0);
		comm_die(111);
	}
}

/*-
 * mqueue_scan
 * set nexttodorun
 *
 * return
 *  0 - Found file
 *  4 - no files found
 * -1 - error
 */
int
mqueue_scan(fd_set *rfds, unsigned long *id)
{
	struct mq_attr  attr;
	unsigned int    priority;
	struct timespec tsp;
	int             i;

	if (mq_queue == (mqd_t)-1) {
		mqueue_init();
		return 4;
	}
#ifdef FREEBSD
	if (!FD_ISSET(mq_getfd_np(mq_queue), rfds))
#else
	if (!FD_ISSET(mq_queue, rfds))
#endif
		return 4;
	if (mq_getattr(mq_queue, &attr) == -1) {
		todo_log("warn: ", argv0, ": ", queuedesc,
			": unable to get message queue attributes:  ", error_str(errno), "\n", 0);
		return -1;
	}
	if (!msgbuflen) {
		if (!(msgbuf = (char *) alloc(attr.mq_msgsize))) {
			todo_log("warn: ", argv0, ": ", queuedesc, ": out of memory\n", 0);
			return -1;
		}
	} else
	if (msgbuflen < attr.mq_msgsize) {
		if (!alloc_re((char *) &msgbuf, msgbuflen, attr.mq_msgsize)) {
			todo_log("warn: ", argv0, ": ", queuedesc, ": out of memory\n", 0);
			return -1;
		}
	}
	msgbuflen = attr.mq_msgsize;
	priority = 0;
	for (;;) {
		if (clock_gettime(clock_id, &tsp) == -1) {
			todo_log("warn: ", argv0, ": ", queuedesc,
				": unable to get RTC time:  ", error_str(errno), "\n", 0);
			return -1;
		}
		tsp.tv_sec += chunk_wait;
		if (mq_timedreceive(mq_queue, msgbuf, msgbuflen, &priority, &tsp) == -1) {
			if (errno == error_timeout) {
				flagtododir = 0;
				return 4;
			}
			if (errno == error_intr)
				continue;
			todo_log("warn: ", argv0, ": ", queuedesc,
				": unable to read message queue: ", error_str(errno), "\n", 0);
			mq_close(mq_queue);
			mq_queue = (mqd_t)-1;
			do_readsubdir = 1;
			return -1;
		} else
			break;
	}
	flagtododir = 1;
	*id = ((q_msg *) msgbuf)->inum;
	if (((q_msg *) msgbuf)->split == conf_split)
		qfn.len = 0;
	else {
		strnum1[i = fmt_ulong(strnum1, ((q_msg *) msgbuf)->split)] = 0;
		if (!stralloc_copyb(&qfn, strnum1, i) || !stralloc_0(&qfn)) {
			if (!do_readsubdir) {
				todo_log("warn: ", argv0, ": ", queuedesc,
					": out of memory. Resetting to opendir mode\n", 0);
				do_readsubdir = 1;
				trigger_set();
			}
			return -1;
		}
	}
	nexttodorun = recent + (todo_interval > 0 ? todo_interval : SLEEP_TODO);
	return 0;
}

void
mqueue_selprep(int *nfds, fd_set *rfds)
{
#ifdef FREEBSD
	int             i;
#else
	mqd_t           i;
#endif

#ifdef FREEBSD
	i = mq_getfd_np(mq_queue);
#else
	i = mq_queue;
#endif

	if (i != -1) {
		FD_SET(i, rfds);
		if (*nfds < i + 1)
			*nfds = i + 1;
	}
}
#endif /*- #ifdef HASLIBRT */

void
todo_selprep(int *nfds, fd_set *rfds, datetime_sec *wakeup)
{
	if (flagexittodo)
		return;
#ifdef HASLIBRT
	if (dynamic_queue)
		mqueue_selprep(nfds, rfds);
	if (do_readsubdir || compat_mode_flag != 2 || !dynamic_queue)
		trigger_selprep(nfds, rfds);
#else
	trigger_selprep(nfds, rfds);
#endif
#ifdef HASLIBRT
	if (!do_readsubdir) {
		if (*wakeup < recent + SLEEP_FOREVER)
			*wakeup = recent + SLEEP_FOREVER;
		return;
	}
#endif
	if (flagtododir)
		*wakeup = 0;
	if (*wakeup > nexttodorun)
		*wakeup = nexttodorun;
}

void
log_stat(unsigned long id, size_t bytes)
{
	char           *ptr;
	char           *mode;

	strnum1[fmt_ulong(strnum1 + 1, id) + 1] = 0;
	strnum2[fmt_ulong(strnum2 + 1, bytes) + 1] = 0;
	*strnum1 = ' ';
	*strnum2 = ' ';
	for (ptr = mailto.s; ptr < mailto.s + mailto.len;) {
#ifdef HASLIBRT
		if (compat_mode_flag == 1)
			mode = " compat mode\n";
		else
			mode = do_readsubdir ? " opendir mode\n" : " mqueue mode\n";
#else
		mode = " opendir mode\n";
#endif
		todo_log(*ptr == 'L' ? "local: " : "remote: ", mailfrom.len > 3 ? mailfrom.s + 1 : "<>",
				" ", *(ptr + 2) ? ptr + 2 : "<>", strnum1, strnum2,
				" bytes ", queuedesc, mode, 0);
		ptr += str_len(ptr) + 1;
	}
	mailfrom.len = mailto.len = 0;
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
	if (!mq_flag && todo_interval > 0 && recent < nexttodorun)
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
#ifdef HASLIBRT
		if (dynamic_queue && do_readsubdir) {
			if (compat_mode_flag == 2) {
				todo_log("info: ", argv0, ": ", queuedesc, ": Resetting to mqueue mode\n", 0);
				trigger_clear(nfds, rfds);
			} else
				todo_log("info: ", argv0, ": ", queuedesc, ": Resetting to compat mode\n", 0);
			do_readsubdir = 0;
		}
#endif
		/*return 4;*/
		/*- flow through */
	default: /*- error */
		return -1;
	}
	return 0;
}

/*-
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
todo_do(int *nfds, fd_set *rfds)
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
#ifdef HASLIBRT
	if (flagexittodo)
		return 0;
	if (dynamic_queue && !do_readsubdir) {
		/*- if trigger is pulled, set flagtododir to 0 */
		if (compat_mode_flag < 2 && trigger_pulled(rfds) && !(flagtododir = 0) &&
				!todo_scan(nfds, rfds, &id, 1)) { /*- found message without using mq_queue as trigger */
			if (!compat_mode_flag)
				todo_log("info: ", argv0, ": ", queuedesc, ": Resetting to compat mode\n", 0);
			compat_mode_flag = 1;
			ptr = readsubdir_name(&todosubdir);
		} else {
			if (compat_mode_flag == 1) {
				compat_mode_flag = 0;
				todo_log("info: ", argv0, ": ", queuedesc, ": Resetting to mqueue mode\n", 0);
			}
			if ((i = mqueue_scan(rfds, &id))) /*- message pushed and intimated through mq_queue */
				return i;
			ptr = qfn.len ? qfn.s : NULL; /*- split name */
		}
	} else {
		if ((i = todo_scan(nfds, rfds, &id, 0))) /*- skip todo run if this returns 1 */
			return i;
		ptr = readsubdir_name(&todosubdir); /*- split name */
	}
#else
	if ((i = todo_scan(nfds, rfds, &id, 0))) /*- skip todo run if this returns 1 */
		return i;
	ptr = readsubdir_name(&todosubdir); /*- split/id */
#endif
	if (ptr) {
		scan_int(ptr, &split); /*- actual split value from filename */
		fnmake_todo(id); /*- set fn as todo/split/id */
		scan_int(fn.s + 5, &i); /*- split as per calculation by fnmake using auto_split */
		todo_log(split != i ? "warn: " : "info: ", argv0, ": ", queuedesc,
			": subdir=todo/", ptr, " fn=", fn.s,
			split != i ? " incorrect split\n" : "\n", 0);
		if (split != i) /*- split doesn't match with split calculation in fnmake_todo() */
			return -1;
	} else
		fnmake_todo(id); /*- set fn as todo/id */
	if ((fd = open_read(fn.s)) == -1) { /*- envelope in todo/split/id */
		todo_log("warn: ", argv0, ": ", queuedesc, ": open ", fn.s, ": ",
			error_str(errno), "\n", 0);
		return -1;
	}
	fnmake_mess(id); /*- change fn to mess/split/id */
	/*- just for the statistics, stat on mess/split file */
	if (stat(fn.s, &st) == -1) {
		todo_log("warn: ", argv0, ": ", queuedesc, ": unable to stat ", fn.s, ": ",
			error_str(errno), "\n", 0);
		goto fail;
	}
	for (c = 0; c < CHANNELS; ++c) {
		fnmake_chanaddr(id, c);
		if (unlink(fn.s) == -1 && errno != error_noent) {
			todo_log("warn: ", argv0, ": ", queuedesc, ": unable to unlink ", fn.s, ": ",
				error_str(errno), "\n", 0);
			goto fail;
		}
	}
	fnmake_info(id); /*- now fn is info/split/id */
	if (unlink(fn.s) == -1 && errno != error_noent) { /*- delete any existing info/split/id */
		todo_log("warn: ", argv0, ": ", queuedesc, ": unable to unlink ", fn.s, ": ",
			error_str(errno), "\n", 0);
		goto fail;
	}
	if ((fdinfo = open_excl(fn.s)) == -1) { /*- create info/split/id */
		todo_log("warn: ", argv0, ": ", queuedesc, ": unable to create ", fn.s, ": ",
			error_str(errno), "\n", 0);
		goto fail;
	}
	strnum1[fmt_ulong(strnum1, id)] = 0;
	todo_log("new msg ", strnum1, " ", queuedesc, "\n", 0);
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
			todo_log("warn: ", argv0, ": ", queuedesc, ": trouble reading ", fn.s, ": ",
				error_str(errno), "\n", 0);
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
				todo_log("warn: ", argv0, ": ", queuedesc, ": trouble writing to ", fn.s, ": ",
					error_str(errno), "\n", 0);
				goto fail;
			}
			break;
		case 'F': /*- from */
			if (substdio_put(&ssinfo, todoline.s, todoline.len) == -1) {
				todo_log("warn: ", argv0, ": ", queuedesc, ": trouble writing to ", fn.s, ": ",
					error_str(errno), "\n", 0);
				goto fail;
			}
			if (!stralloc_copy(&mailfrom, &todoline) || !stralloc_0(&mailfrom)) {
				nomem();
				goto fail;
			}
			/*- write data to comm_buf, which will be written to qmail-send later */
			comm_info(id, (unsigned long) st.st_size, todoline.s + 1, pid, uid);
			break;
		case 'T':
			/*-
			 * 1. check address in control/locals, controls/virtualdomains
			 * 2. rewrite address in rwline
			 */
			switch (rewrite(todoline.s + 1))
			{
			case 0:
				nomem();
				goto fail;
			case 2: /* Sea */
				if (!stralloc_cats(&mailto, "R") || !stralloc_cat(&mailto, &todoline)) {
					nomem();
					goto fail;
				}
				c = 1;
				break;
			default: /* Land */
				if (!stralloc_cats(&mailto, "L") || !stralloc_cat(&mailto, &todoline)) {
					nomem();
					goto fail;
				}
				c = 0;
				break;
			}
			if (fdchan[c] == -1) {
				/*- create local/split/id or remote/split/id */
				fnmake_chanaddr(id, c);
				if ((fdchan[c] = open_excl(fn.s)) == -1) {
					todo_log("warn: ", argv0, ": ", queuedesc, ": unable to create ", fn.s, ": ",
						error_str(errno), "\n", 0);
					goto fail;
				}
				substdio_fdbuf(&sschan[c], write, fdchan[c], todobufchan[c], sizeof (todobufchan[c]));
				flagchan[c] = 1;
			}
			if (substdio_bput(&sschan[c], rwline.s, rwline.len) == -1) {
				fnmake_chanaddr(id, c);
				todo_log("warn: ", argv0, ": ", queuedesc, ": trouble writing to ", fn.s, ": ",
					error_str(errno), "\n", 0);
				goto fail;
			}
			break;
		default:
			fnmake_todo(id); /* todo/split/id */
			todo_log("warn: ", argv0, ": ", queuedesc, ": unknown record type in ", fn.s, ": ",
				error_str(errno), "\n", 0);
			goto fail;
		}
	}
	close(fd);
	fd = -1;
	fnmake_info(id); /* info/split/id */
	if (substdio_flush(&ssinfo) == -1) {
		todo_log("warn: ", argv0, ": ", queuedesc, ": trouble writing to ", fn.s, ": ",
			error_str(errno), "\n", 0);
		goto fail;
	}
	for (c = 0; c < CHANNELS; ++c) {
		if (fdchan[c] != -1) {
			if (substdio_flush(&sschan[c]) == -1) {
				fnmake_chanaddr(id, c);
				todo_log("warn: ", argv0, ": ", queuedesc, ": trouble writing to ", fn.s, ": ",
					error_str(errno), "\n", 0);
				goto fail;
			}
		}
	}
#ifdef USE_FSYNC
	if ((use_fsync > 0 || use_fdatasync > 0) && (use_fdatasync ? fdatasync(fdinfo) : fsync(fdinfo)) == -1) {
		todo_log("warn: ", argv0, ": ", queuedesc, ": trouble fsyncing ", fn.s, ": ",
			error_str(errno), "\n", 0);
		goto fail;
	}
#else
	if (fsync(fdinfo) == -1) {
		todo_log("warn: ", argv0, ": ", queuedesc, ": trouble fsyncing ", fn.s, ": ",
			error_str(errno), "\n", 0);
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
				todo_log("warn: ", argv0, ": ", queuedesc, ": trouble fsyncing ", fn.s, ": ",
					error_str(errno), "\n", 0);
				goto fail;
			}
#else
			if (fdatasync(fdchan[c]) == -1) {
				fnmake_chanaddr(id, c);
				todo_log("warn: ", argv0, ": ", queuedesc, ": trouble fsyncing ", fn.s, ": ",
					error_str(errno), "\n", 0);
				goto fail;
			}
#endif
			close(fdchan[c]);
			fdchan[c] = -1;
		}
	}
	fnmake_todo(id); /* todo/split/id */
	if (substdio_putflush(&sstoqc, fn.s, fn.len) == -1) {
		cleandied();
		return -1;
	}
	if (substdio_get(&ssfromqc, &ch, 1) != 1) {
		cleandied();
		return -1;
	}
	if (ch != '+') {
		todo_log("warn: ", argv0, ": ", queuedesc, ": qmail-clean unable to clean up ", fn.s, ": ",
			error_str(errno), "\n", 0);
		return -1;
	}
	comm_write(id, flagchan[0], flagchan[1]); /*- e.g. "DL656826\0" */
	/*- "Llocal: mbhangui@argos.indimail.org mbhangui@argos.indimail.org 798 queue1\n\0" */
	log_stat(id, st.st_size);
	/*-
	 * return in chunks of todo_chunk_size
	 * so that todo-proc doesn't spend to much time in building
	 * comm_buf without sending a single email for delivery. This
	 * will avoid slow delivery when todo-proc/qmail-send wasn't running
	 * and large number of emails have been injected into the queue.
	 */
	return (flagtododir == 0 ? 4 : (comm_count % todo_chunk_size) == 0 ? 3 : 2);

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

/*- this file is too long ---------------------------------------------- MAIN */

int
getcontrols(void)
{
	if (control_init() == -1 ||
			control_rldef(&envnoathost, "envnoathost", 1, "envnoathost") != 1 ||
			control_readfile(&locals, "locals", 1) != 1 ||
			!constmap_init(&maplocals, locals.s, locals.len, 0))
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
#ifdef USE_FSYNC
	if (control_readint(&use_syncdir, "conf-syncdir") == -1 ||
			control_readint(&use_fsync, "conf-fsync") == -1 ||
			control_readint(&use_fdatasync, "conf-fdatasync") == -1)
		return 0;
#endif
	return 1;
}

void
regetcontrols(void)
{
	int             r;

	if (!(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	if (control_readfile(&newlocals, "locals", 1) != 1) {
		todo_log("alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/locals\n", 0);
		return;
	}
	if ((r = control_readfile(&newvdoms, "virtualdomains", 0)) == -1) {
		todo_log("alert: ", argv0, ": ", queuedesc, ": reread ", controldir, "/virtualdomains\n", 0);
		return;
	}
	if (control_readint(&todo_interval, "todointerval") == -1) {
		todo_log("alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/todointerval\n", 0);
		return;
	}
	if (control_rldef(&envnoathost, "envnoathost", 1, "envnoathost") != 1) {
		todo_log("alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/envnoathost\n", 0);
		return;
	}
#ifdef USE_FSYNC
	if (control_readint(&use_syncdir, "conf-syncdir") == -1) {
		todo_log("alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/conf-syncdir\n", 0);
		return;
	}
	if (control_readint(&use_fsync, "conf-fsync") == -1) {
		todo_log("alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/conf-fsync\n", 0);
		return;
	}
	if (control_readint(&use_fdatasync, "conf-fdatasync") == -1) {
		todo_log("alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/conf-fdatasync\n", 0);
		return;
	}
#endif
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

void
reread(void)
{
	if (chdir(auto_qmail) == -1) {
		todo_log("alert: ", argv0, ": ", queuedesc,
				": cannot start: unable to switch to ", auto_qmail, ": ",
				error_str(errno), "\n", 0);
		return;
	}
	regetcontrols();
	if (!queuedir && !(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue"; /*- single queue like qmail */
	while (chdir(queuedir) == -1) {
		todo_log("alert: ", argv0, ": ", queuedesc,
				": unable to switch back to queue directory ", queuedir,
				": ", error_str(errno), "HELP! sleeping...\n", 0);
		sleep(10);
	}
}

int
main(int argc, char **argv)
{
	int             nfds, r, opt;
	char            c;
	datetime_sec    wakeup;
	fd_set          rfds, wfds;
	char           *ptr;
	struct timeval  tv;

	r = str_rchr(argv[0], '/');
	argv0 = (argv[0][r] && argv[0][r + 1]) ? argv[0] + r + 1 : argv[0];
	if (!queuedir && !(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue"; /*- single queue like qmail */
	for (queuedesc = queuedir; *queuedesc; queuedesc++);
	for (; queuedesc != queuedir && *queuedesc != '/'; queuedesc--);
	if (*queuedesc == '/')
		queuedesc++;
	if (chdir(auto_qmail) == -1) {
		todo_log("alert: ", argv0, ": ", queuedesc,
				": cannot start: unable to switch to ", auto_qmail, ": ",
				error_str(errno), "\n", 0);
		comm_die(111);
	}
	getEnvConfigInt(&chunk_wait, "CHUNK_WAIT", CHUNK_WAIT);
	getEnvConfigInt(&bigtodo, "BIGTODO", 1);
	getEnvConfigInt(&conf_split, "CONFSPLIT", auto_split);
	if (conf_split > auto_split)
		conf_split = auto_split;
	getEnvConfigInt(&todo_chunk_size, "TODO_CHUNK_SIZE", CHUNK_SIZE);
	if (todo_chunk_size <= 0)
		todo_chunk_size = CHUNK_SIZE;
	strnum1[fmt_ulong(strnum1, conf_split)] = 0;
	strnum2[fmt_ulong(strnum2, todo_chunk_size)] = 0;
	todo_log("info: ", argv0, ": ", queuedir, ": conf split=", strnum1,
		", todo chunk size=", strnum2, bigtodo ? ", bigtodo=yes\n" : ", bigtodo=no\n", 0);
#ifdef USE_FSYNC
	ptr = env_get("USE_FSYNC");
	use_fsync = (ptr && *ptr) ? 1 : 0;
	ptr = env_get("USE_FDATASYNC");
	use_fdatasync = (ptr && *ptr) ? 1 : 0;
	ptr = env_get("USE_SYNCDIR");
	use_syncdir = (ptr && *ptr) ? 1 : 0;
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
	if (use_fdatasync > 0) {
		while (!env_put2("USE_FDATASYNC", "1"))
			nomem();
	} else
	if (!use_fdatasync) {
		while (!env_unset("USE_FDATASYNC"))
			nomem();
	}
#endif
	if (!getcontrols()) {
		todo_log("alert: ", argv0, ": ", queuedesc,
				": cannot start: unable to read controls or out of memory\n", 0);
		comm_die(111);
	}
	if (chdir(queuedir) == -1) {
		todo_log("alert: ", argv0, ": ", queuedesc,
				": cannot start: unable to switch to queue directory",
				queuedir, ": ", error_str(errno), "\n", 0);
		comm_die(111);
	}
	sig_pipeignore();
	umask(077);

	while ((opt = getopt(argc, argv, "cds")) != opteof) {
		switch (opt)
		{
			case 'c':
#ifdef HASLIBRT
				compat_mode_flag = 0; /*- run in compat mode */
#endif
				break;
			case 'd':
#ifndef HASLIBRT
				todo_log("alert: ", argv0, ": ", queuedesc, ": dynamic queue not supported\n", 0);
				comm_die(100);
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
	fnmake_init(); /*- initialize fn */
#ifdef HASLIBRT
	if (dynamic_queue)
		mqueue_init();
#endif
	todo_init();   /*- set nexttodorun, open lock/trigger */
	comm_init();   /*- assign fd 2 to queue comm to, 3 to queue comm from */
	for (;;) {
		/*- read from fd 0 (qmail-send) */
		if ((r = read(sendfdi, &c, 1)) == -1) {
			if (errno == error_intr)
				continue;
			_exit(100);	/*- read failed probably qmail-send died */
		}
		if (!r)
			_exit(100);	/*- read failed probably qmail-send died */
		break;
	}
	if (c != 'C') {
		todo_log("warn: ", argv0, ": ", queuedesc, ": qmail-send speaks an obscure dialect\n", 0);
		_exit(100);
	}
	for (;;) {
		recent = now();
		if (flagreadasap) {
			flagreadasap = 0;
			reread();
		}
		if (!flagsendalive) {
			/*- qmail-send finally exited, so do the same. */
			if (flagexittodo)
				_exit(0);
			/*- qmail-send died. We can not log and we can not work therefor _exit(1). */
			_exit(1);
		}
		wakeup = recent + SLEEP_FOREVER;
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		nfds = 1;
		/*-
		 * 1. set select on read events on lock/trigger
		 * 2. if flagtododir is set, reset wakup to 0
		 *    if any message is found
		 * This is important so that we set tv.tv_sec=0
		 * and make select return immediately so as
		 * to continue scanning remaining directories.
		 */
		todo_selprep(&nfds, &rfds, &wakeup);
		/*-
		 * set select on read/write events on pipe (fd 0, 1) to/from qmail-send
		 * fd 0 - pi7[0] - read
		 * fd 1 - pi8[1] - write
		 */
		comm_selprep(&nfds, &wfds, &rfds);
		/*-  set tv.tv_sec to 0 to make make select return immediately */
		if (wakeup <= recent)
			tv.tv_sec = 0;
		else
			tv.tv_sec = wakeup - recent + chunk_wait;
		tv.tv_usec = 0;
		if (select(nfds, &rfds, &wfds, (fd_set *) 0, &tv) == -1) {
			if (errno == error_intr);
			else
				todo_log("warn: ", argv0, ": ", queuedesc, ": trouble in select\n", 0);
		} else {
			/*-
			 * we come here on
			 * 1. select timeout out
			 * 2. when trigger is pulled
			 * 3. when message using mq_queue is received
			 */
			recent = now();
			/*-
			 * todo_do returns 2 until files are present in todo subdir
			 * todo_do keeps building comm_buf, which is used by
			 * comm_do to communicate the file list to qmail-send.
			 */
			while (todo_do(&nfds, &rfds) == 2);
			comm_do(&wfds, &rfds); /*- communicate with qmail-send on fd 0, fd 1 */
		}
	} /*- for (;;) */
	strnum1[fmt_ulong(strnum1, getpid())] = 0;
	todo_log("info: ", argv0, ": pid ", strnum1, " ", queuedesc, " exiting\n", 0);
	_exit(0);
}

void
getversion_qmail_todo_c()
{
	static char    *x = "$Id: todo-proc.c,v 1.66 2023-01-15 23:30:27+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}

/*
 * $Log: todo-proc.c,v $
 * Revision 1.66  2023-01-15 23:30:27+05:30  Cprogrammer
 * use todo_log() function with varargs to log messages to qmail-send
 *
 * Revision 1.65  2022-11-24 08:50:10+05:30  Cprogrammer
 * changed variable type to c when reading from qmail-send
 *
 * Revision 1.64  2022-09-27 12:49:46+05:30  Cprogrammer
 * updated log messages
 *
 * Revision 1.63  2022-09-26 09:28:00+05:30  Cprogrammer
 * added feature to disconnect from qmail-send
 *
 * Revision 1.62  2022-04-12 08:37:19+05:30  Cprogrammer
 * added pid in logs
 *
 * Revision 1.61  2022-04-04 14:30:07+05:30  Cprogrammer
 * added setting of fdatasync() instead of fsync()
 *
 * Revision 1.60  2022-04-04 00:51:42+05:30  Cprogrammer
 * display queuedir in logs
 *
 * Revision 1.59  2022-03-31 00:25:15+05:30  Cprogrammer
 * use chunk_wait seconds to wait for message chunks
 *
 * Revision 1.58  2022-03-20 00:35:06+05:30  Cprogrammer
 * use mq_timedreceive() for TODO_CHUNK_SIZE to work
 *
 * Revision 1.57  2022-03-13 19:55:26+05:30  Cprogrammer
 * display bigtodo value in logs on startup
 *
 * Revision 1.56  2022-03-01 23:05:34+05:30  Cprogrammer
 * renamed compat_mode variable to compat_mode_flag
 *
 * Revision 1.55  2022-01-30 09:17:33+05:30  Cprogrammer
 * fixes for FreeBSD
 * revert to trigger method if using message queue fails
 * fix for OSX
 * fixed select timeout for dynamic queue
 * added compat mode to wakeup on lock/trigger
 * make USE_FSYNC, USE_SYNCDIR consistent across programs
 * allow configurable big/small todo/intd
 * remove/added use of lock/trigger based on dynamic/static mode
 *
 * Revision 1.54  2021-12-12 08:47:50+05:30  Cprogrammer
 * use argv0 for program name
 * transmit messid to qmail-send in TODO_CHUNK_SIZE
 *
 * Revision 1.53  2021-10-22 14:00:03+05:30  Cprogrammer
 * fixed typo
 *
 * Revision 1.52  2021-10-20 22:47:41+05:30  Cprogrammer
 * display program 'todo-proc' in logs for identification
 *
 * Revision 1.51  2021-07-26 23:24:53+05:30  Cprogrammer
 * log when log sighup, sigalrm is caught
 *
 * Revision 1.50  2021-07-17 14:39:28+05:30  Cprogrammer
 * skip processing of for messages queued with wrong split dir
 *
 * Revision 1.49  2021-06-27 10:45:15+05:30  Cprogrammer
 * moved conf_split variable to fmtqfn.c
 *
 * Revision 1.48  2021-06-23 13:21:18+05:30  Cprogrammer
 * display bytes in log_stat function
 *
 * Revision 1.47  2021-06-05 12:53:13+05:30  Cprogrammer
 * display todo-proc prefix for startup message
 *
 * Revision 1.46  2021-05-16 01:45:13+05:30  Cprogrammer
 * limit conf_split to compile time value in conf-split
 * added code comments
 *
 * Revision 1.45  2021-05-12 17:51:49+05:30  Cprogrammer
 * display todo filename in logs
 *
 * Revision 1.44  2021-05-12 15:51:36+05:30  Cprogrammer
 * set conf_split from CONFSPLIT env variable
 * added code comments
 *
 * Revision 1.43  2021-05-08 12:26:31+05:30  Cprogrammer
 * added log7() function
 * use /var/indimail/queue if QUEUEDIR is not defined
 *
 * Revision 1.42  2021-04-05 07:19:58+05:30  Cprogrammer
 * added todo-proc.h
 *
 * Revision 1.41  2020-11-24 13:47:41+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.40  2020-09-30 20:39:44+05:30  Cprogrammer
 * Darwin port for syncdir
 *
 * Revision 1.39  2020-09-15 21:43:40+05:30  Cprogrammer
 * unset USE_SYNCDIR, USE_FSYNC only when use_syncdir, use_fsync is zero
 *
 * Revision 1.38  2020-09-15 21:10:08+05:30  Cprogrammer
 * use control files conf-fsync, conf-syncdir to turn on fsync, bsd style syncdir semantics
 * set / unset USE_FSYNC, USE_SYNCDIR env variables
 *
 * Revision 1.37  2018-07-03 01:59:20+05:30  Cprogrammer
 * reread envnoathost on HUP
 *
 * Revision 1.36  2018-01-09 11:55:07+05:30  Cprogrammer
 * removed non-indimail code
 *
 * Revision 1.35  2017-03-31 21:10:34+05:30  Cprogrammer
 * log null addresses in log_stat as <>
 *
 * Revision 1.34  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.33  2016-01-29 18:31:23+05:30  Cprogrammer
 * include queue name in logs
 *
 * Revision 1.32  2013-09-23 22:14:20+05:30  Cprogrammer
 * display queue directory for todo-proc process
 *
 * Revision 1.31  2013-05-16 23:32:53+05:30  Cprogrammer
 * added log_stat part of non-indimail code
 *
 * Revision 1.30  2011-07-29 09:29:54+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.29  2010-06-27 09:08:55+05:30  Cprogrammer
 * report all recipients in log_stat for single transaction multiple recipient emails
 *
 * Revision 1.28  2007-12-20 13:50:59+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.27  2005-12-29 23:02:51+05:30  Cprogrammer
 * option for passing headers through queue
 *
 * Revision 1.26  2005-03-03 16:11:48+05:30  Cprogrammer
 * minimum interval in secs for todo run
 *
 * Revision 1.25  2004-10-22 20:29:42+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.24  2004-10-22 15:38:20+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.23  2004-07-17 21:21:55+05:30  Cprogrammer
 * added qqeh code
 *
 * Revision 1.22  2004-01-14 23:40:50+05:30  Cprogrammer
 * delay fsync()
 *
 * Revision 1.21  2004-01-02 10:17:10+05:30  Cprogrammer
 * prevent segmentation fault in log_stat when to or from is null
 * reset to and from
 *
 * Revision 1.20  2003-12-31 20:03:33+05:30  Cprogrammer
 * added use_fsync to turn on/off use_fsync
 *
 * Revision 1.19  2003-12-09 21:27:24+05:30  Cprogrammer
 * corrected flagsendalive check
 *
 * Revision 1.18  2003-10-28 20:02:34+05:30  Cprogrammer
 * conditional compilation for INDIMAIL
 *
 * Revision 1.17  2003-10-23 01:26:54+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.16  2003-10-17 21:06:48+05:30  Cprogrammer
 * added log_stat function
 *
 * Revision 1.15  2003-10-01 19:06:12+05:30  Cprogrammer
 * changed return type to int
 * added code for future log_stat
 *
 */
