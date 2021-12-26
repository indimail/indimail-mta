/*
 * $Id: $
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

/*- critical timing feature #1: if not triggered, do not busy-loop */
/*- critical timing feature #2: if triggered, respond within fixed time */
/*- important timing feature: when triggered, respond instantly */
#define SLEEP_TODO     1500  /*- check todo/ every 25 minutes in any case */
#define ONCEEVERY        10  /*- Run todo maximal once every N seconds */
#define SLEEP_FUZZ        1  /*- slop a bit on sleeps to avoid zeno effect */
#define SLEEP_FOREVER 86400  /*- absolute maximum time spent in select() */
#define SLEEP_SYSFAIL   123
#define CHUNK_SIZE      100

static stralloc percenthack = { 0 };
typedef struct  constmap cmap;

static cmap     mappercenthack;
static cmap     maplocals;
static stralloc locals = { 0 };
static cmap     mapvdoms;
static stralloc vdoms = { 0 };
static stralloc envnoathost = { 0 };

static char     strnum[FMT_ULONG];

/*- if qmail-send.c changes this has to be updated */
#define CHANNELS 2
static char    *chanaddr[CHANNELS] = { "local/", "remote/" };

static int      flagstopasap = 0;
static char    *queuedesc;
static char    *argv0 = "qmail-todo";
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
static int      fdout = -1;
static int      fdin = -1;

static int      flagsendalive = 1;
static int      flagtododir = 0; /*- if 0, have to readsubdir_init again */
static int      todo_interval = -1;
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
#ifdef HASLIBRT
static int      do_readsubdir = 1;
static mqd_t    mq_queue = (mqd_t)-1;
static char    *msgbuf;
static int      msgbuflen;
static stralloc qfn = { 0 };
static int      compat_mode = 0;
#endif

void            log1(char *w);
void            log3(char *w, char *x, char *y);
void            log4(char *w, char *x, char *y, char *z);
void            log5(char *u, char *w, char *x, char *y, char *z);
void            log7(char *s, char *t, char *u, char *w, char *x, char *y, char *z);
void            log9(char *r, char *s, char *t, char *u, char *v, char *w, char *x, char *y, char *z);

void
sigterm(void)
{
	sig_block(sig_term);
	log5("info: ", argv0, ": Got TERM: ", queuedesc, "\n");
	if (!flagstopasap)
		log5("status: ", argv0, ": ", queuedesc, " stop processing asap\n");
	flagstopasap = 1;
}

int             flagreadasap = 0;
void
sighup(void)
{
	flagreadasap = 1;
	log5("info: ", argv0, ": Got HUP: ", queuedesc, "\n");
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
	log5("alert: ", argv0, ": ", queuedesc, ": out of memory, sleeping...\n");
	sleep(10);
}
void
pausedir(char *dir)
{
	log7("alert: ", argv0, ": ", queuedesc, ": unable to opendir ", dir, ", sleeping...\n");
	sleep(10);
}

void
cleandied()
{
	log5("alert: ", argv0, ": ", queuedesc, ": oh no! lost qmail-clean connection! dying...\n");
	flagstopasap = 1;
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
	fn.len = fmtqfn(fn.s, "todo/", id, 1);
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

	if (!stralloc_copys(&rwline, "T"))
		return 0;
	if (!stralloc_copys(&addr, recip))
		return 0;
	i = byte_rchr(addr.s, addr.len, '@');
	if (i == addr.len) {
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
		if (!stralloc_cat(&rwline, &addr))
			return 0;
		if (!stralloc_0(&rwline))
			return 0;
		return 1;
	}

	for (i = 0; i <= addr.len; ++i) {
		if (!i || (i == at + 1) || (i == addr.len) || ((i > at) && (addr.s[i] == '.'))) {
			if ((x = constmap(&mapvdoms, addr.s + i, addr.len - i))) {
				if (!*x)
					break;
				if (!stralloc_cats(&rwline, x))
					return 0;
				if (!stralloc_cats(&rwline, "-"))
					return 0;
				if (!stralloc_cat(&rwline, &addr))
					return 0;
				if (!stralloc_0(&rwline))
					return 0;
				return 1;
			}
		}
	}
	if (!stralloc_cat(&rwline, &addr))
		return 0;
	if (!stralloc_0(&rwline))
		return 0;
	return 2;
}

/*
 * comm functions taken from qmail-send.c
 */
void
comm_init(void)
{
	/* fd 2 is pi9[1] - write to qmail-clean, fd 3 is pi10[0] - read from qmail-clean */
	substdio_fdbuf(&sstoqc, write, 2, sstoqcbuf, sizeof (sstoqcbuf));
	substdio_fdbuf(&ssfromqc, read, 3, ssfromqcbuf, sizeof (ssfromqcbuf));

	fdout = 1;	/*- stdout pi8[1] to qmail-send */
	fdin = 0;	/*- stdin  pi7[0] from qmail-send */
	/*- this is so stupid: NDELAY semantics should be default on write */
	if (ndelay_on(fdout) == -1)
		senddied(); /*- set flagsendalive = 0. drastic but avoids risking deadlock */
	while (!stralloc_ready(&comm_buf, 1024))
		nomem();
}

int
comm_canwrite(void)
{
	/*- XXX: could allow a bigger buffer; say 10 recipients */
	/*- XXX: returns true if there is something in the buffer */
	if (!flagsendalive)
		return 0;
	if (comm_buf.s && comm_buf.len)
		return 1;
	return 0;
}

/*
 * idea of using comm_buf to communicate with qmail-send
 * comes from ext-todo patch
 * Claudio Jeker <jeker@n-r-g.com and Andre Oppermann ext-todo
 */
void
log1(char *x)
{
	int             pos;

	pos = comm_buf.len;
	if (!stralloc_cats(&comm_buf, "L") ||
			!stralloc_cats(&comm_buf, x) ||
			!stralloc_0(&comm_buf))
		goto fail;
	return;
fail:
	/*- either all or nothing */
	comm_buf.len = pos;
}

void
log3(char *x, char *y, char *z)
{
	int             pos;

	pos = comm_buf.len;
	if (!stralloc_cats(&comm_buf, "L") ||
			!stralloc_cats(&comm_buf, x) ||
			!stralloc_cats(&comm_buf, y) ||
			!stralloc_cats(&comm_buf, z) ||
			!stralloc_0(&comm_buf))
		goto fail;
	return;
fail:
	/*- either all or nothing */
	comm_buf.len = pos;
}

void
log4(char *w, char *x, char *y, char *z)
{
	int             pos;

	pos = comm_buf.len;
	if (!stralloc_cats(&comm_buf, "L") ||
			!stralloc_cats(&comm_buf, w) ||
			!stralloc_cats(&comm_buf, x) ||
			!stralloc_cats(&comm_buf, y) ||
			!stralloc_cats(&comm_buf, z) ||
			!stralloc_0(&comm_buf))
		goto fail;
	return;
fail:
	/*- either all or nothing */
	comm_buf.len = pos;
}

void
log5(char *v, char *w, char *x, char *y, char *z)
{
	int             pos;

	pos = comm_buf.len;
	if (!stralloc_cats(&comm_buf, "L") ||
			!stralloc_cats(&comm_buf, v) ||
			!stralloc_cats(&comm_buf, w) ||
			!stralloc_cats(&comm_buf, x) ||
			!stralloc_cats(&comm_buf, y) ||
			!stralloc_cats(&comm_buf, z) ||
			!stralloc_0(&comm_buf))
		goto fail;
	return;
fail:
	/*- either all or nothing */
	comm_buf.len = pos;
}

void
log7(char *t, char *u, char *v, char *w, char *x, char *y, char *z)
{
	int             pos;

	pos = comm_buf.len;
	if (!stralloc_cats(&comm_buf, "L") ||
			!stralloc_cats(&comm_buf, t) ||
			!stralloc_cats(&comm_buf, u) ||
			!stralloc_cats(&comm_buf, v) ||
			!stralloc_cats(&comm_buf, w) ||
			!stralloc_cats(&comm_buf, x) ||
			!stralloc_cats(&comm_buf, y) ||
			!stralloc_cats(&comm_buf, z) ||
			!stralloc_0(&comm_buf))
		goto fail;
	return;
fail:
	/*- either all or nothing */
	comm_buf.len = pos;
}

void
log9(char *r, char *s, char *t, char *u, char *v, char *w, char *x, char *y, char *z)
{
	int             pos;

	pos = comm_buf.len;
	if (!stralloc_cats(&comm_buf, "L") ||
			!stralloc_cats(&comm_buf, r) ||
			!stralloc_cats(&comm_buf, s) ||
			!stralloc_cats(&comm_buf, t) ||
			!stralloc_cats(&comm_buf, u) ||
			!stralloc_cats(&comm_buf, v) ||
			!stralloc_cats(&comm_buf, w) ||
			!stralloc_cats(&comm_buf, x) ||
			!stralloc_cats(&comm_buf, y) ||
			!stralloc_cats(&comm_buf, z) ||
			!stralloc_0(&comm_buf))
		goto fail;
	return;
fail:
	/*- either all or nothing */
	comm_buf.len = pos;
}

void
comm_write(unsigned long id, int local, int remote)
{
	int             pos;
	char           *s;

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
	strnum[fmt_ulong(strnum, id)] = 0;
	if (!stralloc_cats(&comm_buf, "D"))
		goto fail;
	if (!stralloc_cats(&comm_buf, s))
		goto fail;
	if (!stralloc_cats(&comm_buf, strnum))
		goto fail;
	if (!stralloc_0(&comm_buf))
		goto fail;
	return;
fail:
	/*- either all or nothing */
	comm_buf.len = pos;
}

static int
issafe(char ch)
{
	if (ch == '%')
		return 0; /*- general principle: allman's code is crap */
	if (ch < 33)
		return 0;
	if (ch > 126)
		return 0;
	return 1;
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
	strnum[fmt_ulong(strnum, id)] = 0;
	if (!stralloc_cats(&comm_buf, strnum))
		goto fail;
	if (!stralloc_cats(&comm_buf, ": bytes "))
		goto fail;
	strnum[fmt_ulong(strnum, size)] = 0;
	if (!stralloc_cats(&comm_buf, strnum))
		goto fail;
	if (!stralloc_cats(&comm_buf, " from <"))
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
	strnum[fmt_ulong(strnum, pid)] = 0;
	if (!stralloc_cats(&comm_buf, strnum))
		goto fail;
	if (!stralloc_cats(&comm_buf, " uid "))
		goto fail;
	strnum[fmt_ulong(strnum, uid)] = 0;
	if (!stralloc_cats(&comm_buf, strnum))
		goto fail;
	if (!stralloc_cats(&comm_buf, " "))
		goto fail;
	if (!stralloc_cats(&comm_buf, queuedesc))
		goto fail;
	if (!stralloc_cats(&comm_buf, "\n"))
		goto fail;
	if (!stralloc_0(&comm_buf))
		goto fail;
	return;

fail:
	/*- either all or nothing */
	comm_buf.len = pos;
}

void
comm_exit(void)
{
	/*- if it fails exit, we have already stoped */
	if (!stralloc_cats(&comm_buf, "X"))
		_exit(1);
	if (!stralloc_0(&comm_buf))
		_exit(1);
}

void
comm_selprep(int *nfds, fd_set *wfds, fd_set *rfds)
{
	if (flagsendalive) {
		if (flagstopasap && comm_canwrite() == 0)
			comm_exit();
		if (comm_canwrite()) {
			FD_SET(fdout, wfds); /*- write fd to qmail-send */
			if (*nfds <= fdout)
				*nfds = fdout + 1;
		}
		FD_SET(fdin, rfds); /*- read fd from qmail-send */
		if (*nfds <= fdin)
			*nfds = fdin + 1;
	}
}

void
comm_do(fd_set *wfds, fd_set *rfds)
{
	/*- first write to qmail-send */
	if (flagsendalive && comm_canwrite()) {
		if (FD_ISSET(fdout, wfds)) {
			int             w;
			int             len;
			len = comm_buf.len;
			if ((w = write(fdout, comm_buf.s + comm_pos, len - comm_pos)) <= 0) {
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
	if (flagsendalive && FD_ISSET(fdin, rfds)) {
		/*- there are only two messages 'H' and 'X' */
		char            c;
		int             r;
		if ((r = read(fdin, &c, 1)) <= 0) {
			if (!r || (r == -1 && errno != error_intr))
				senddied(); /*- set flagsendalive = 0 */
		} else {
			switch (c)
			{
			case 'H':
				sighup(); /*- set flagreadasap = 1 */
				break;
			case 'X':
				sigterm(); /*-set flagstopasap = 1 */
				break;
			default:
				log5("warning: ", argv0, ": ", queuedesc, ": qmail-send speaks an obscure dialect\n");
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

	fdout = 1;
	fdin = 0;
	if (ndelay_on(fdout) == -1)
		senddied(); /*- set flagsendalive = 0. drastic but avoids risking deadlock */
	while (!stralloc_ready(&comm_buf, 1024))
		nomem();
	comm_exit();
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	nfds = 1;
	comm_selprep(&nfds, &wfds, &rfds);
	comm_do(&wfds, &rfds);
	log5("status: ", argv0, ": ", queuedesc, " stop processing asap\n");
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
	/*- message queue /queueN for communication between qmail-todo, qmail-queue */
	if ((mq_queue = mq_open(mq_name, O_RDONLY,  0600, NULL)) == (mqd_t)-1) {
		log7("alert: ", argv0, ": failed to open POSIX message queue ",
			mq_name, ": ", error_str(errno), "\n");
		comm_die(111);
	} 
	nexttodorun = now();
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
		log7("warning: ", argv0, ": ", queuedesc,
			": unable to get message queue attributes:  ", error_str(errno), "\n");
		return -1;
	}
	if (!msgbuflen) {
		if (!(msgbuf = (char *) alloc(attr.mq_msgsize))) {
			log5("warning: ", argv0, ": ", queuedesc, ": out of memory\n");
			return -1;
		}
	} else
	if (msgbuflen < attr.mq_msgsize) {
		if (!alloc_re((char *) &msgbuf, msgbuflen, attr.mq_msgsize)) {
			log5("warning: ", argv0, ": ", queuedesc, ": out of memory\n");
			return -1;
		}
	}
	msgbuflen = attr.mq_msgsize;
	priority = 0;
	for (;;) {
		if (mq_receive(mq_queue, msgbuf, msgbuflen, &priority) == -1) {
			if (errno == error_intr)
				continue;
			log7("warning: ", argv0, ": ", queuedesc,
				": unable to read message queue: ", error_str(errno), "\n");
			mq_close(mq_queue);
			mq_queue = (mqd_t)-1;
			do_readsubdir = 1;
			return -1;
		} else
			break;
	}
	*id = ((q_msg *) msgbuf)->inum;
	strnum[i = fmt_ulong(strnum, ((q_msg *) msgbuf)->split)] = 0;
	if (!stralloc_copyb(&qfn, strnum, i) || !stralloc_0(&qfn)) {
		if (!do_readsubdir)
			log5("warning: ", argv0, ": ", queuedesc,
				": out of memory. Resetting to opendir mode\n");
		do_readsubdir = 1;
		return -1;
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
	if (flagstopasap)
		return;
#ifdef HASLIBRT
	if (dynamic_queue)
		mqueue_selprep(nfds, rfds);
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
	char            strnum1[FMT_ULONG + 1], strnum2[FMT_ULONG + 1];

	strnum1[fmt_ulong(strnum1 + 1, id) + 1] = 0;
	strnum2[fmt_ulong(strnum2 + 1, bytes) + 1] = 0;
	*strnum1 = ' ';
	*strnum2 = ' ';
	for (ptr = mailto.s; ptr < mailto.s + mailto.len;) {
#ifdef HASLIBRT
		if (compat_mode)
			mode = " compat mode\n";
		else
			mode = do_readsubdir ? " opendir mode\n" : " mqueue mode\n";
#else
		mode = " opendir mode\n";
#endif
		log9(*ptr == 'L' ? "local: " : "remote: ", mailfrom.len > 3 ? mailfrom.s + 1 : "<>",
				" ", *(ptr + 2) ? ptr + 2 : "<>", strnum1, strnum2,
				" bytes ", queuedesc, mode);
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
todo_scan(fd_set *rfds, unsigned long *id, int mq_flag)
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
		readsubdir_init(&todosubdir, "todo", pausedir);
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
		if (dynamic_queue) {
			if (do_readsubdir)
				log5("info: ", argv0, ": ", queuedesc, ": Resetting to mqueue mode\n");
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
todo_do(fd_set *rfds)
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
	if (flagstopasap)
		return 0;
	if (dynamic_queue && !do_readsubdir) {
		/*- if trigger is pulled, set flagtododir to 0 */
		if (trigger_pulled(rfds) && !(flagtododir = 0) &&
				!todo_scan(rfds, &id, 1)) { /*- found message without using mq_queue as trigger */
			if (!compat_mode)
				log5("info: ", argv0, ": ", queuedesc, ": Resetting to compat mode\n");
			compat_mode = 1;
			ptr = readsubdir_name(&todosubdir);
		} else {
			if (compat_mode)
				log5("info: ", argv0, ": ", queuedesc, ": Resetting to mqueue mode\n");
			compat_mode = 0;
			if ((i = mqueue_scan(rfds, &id))) /*- message pushed and intimated through mq_queue */
				return i;
			ptr = qfn.s; /*- split name */
		}
	} else {
		if ((i = todo_scan(rfds, &id, 0))) /*- skip todo run if this returns 1 */
			return i;
		ptr = readsubdir_name(&todosubdir); /*- split name */
	}
#else
	if ((i = todo_scan(rfds, &id, 0))) /*- skip todo run if this returns 1 */
		return i;
	ptr = readsubdir_name(&todosubdir); /*- split/id */
#endif
	scan_int(ptr, &split); /*- actual split value from filename */
	fnmake_todo(id); /*- set fn as todo/split/id */
	scan_int(fn.s + 5, &i); /*- split as per calculation by fnmake using auto_split */
	log9(split != i ? "warning: " : "info: ", argv0, ": ", queuedesc, 
			": subdir=todo/", ptr, " fn=", fn.s,
			split != i ? " incorrect split\n" : "\n");
	if (split != i) /*- split doesn't match with split calculation in fnmake_todo() */
		return -1;
	if ((fd = open_read(fn.s)) == -1) { /*- envelope in todo/split/id */
		log9("warning: ", argv0, ": ", queuedesc, ": open ", fn.s, ": ",
			error_str(errno), "\n");
		return -1;
	}
	fnmake_mess(id); /*- change fn to mess/split/id */
	/*- just for the statistics, stat on mess/split file */
	if (stat(fn.s, &st) == -1) {
		log9("warning: ", argv0, ": ", queuedesc, ": unable to stat ", fn.s, ": ",
			error_str(errno), "\n");
		goto fail;
	}
	for (c = 0; c < CHANNELS; ++c) {
		fnmake_chanaddr(id, c);
		if (unlink(fn.s) == -1 && errno != error_noent) {
			log9("warning: ", argv0, ": ", queuedesc, ": unable to unlink ", fn.s, ": ",
				error_str(errno), "\n");
			goto fail;
		}
	}
	fnmake_info(id); /*- now fn is info/split/id */
	if (unlink(fn.s) == -1 && errno != error_noent) { /*- delete any existing info/split/id */
		log9("warning: ", argv0, ": ", queuedesc, ": unable to unlink ", fn.s, ": ",
			error_str(errno), "\n");
		goto fail;
	}
	if ((fdinfo = open_excl(fn.s)) == -1) { /*- create info/split/id */
		log9("warning: ", argv0, ": ", queuedesc, ": unable to create ", fn.s, ": ",
			error_str(errno), "\n");
		goto fail;
	}
	strnum[fmt_ulong(strnum, id)] = 0;
	log5("new msg ", strnum, " ", queuedesc, "\n");
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
			log9("warning: ", argv0, ": ", queuedesc, ": trouble reading ", fn.s, ": ",
				error_str(errno), "\n");
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
				log9("warning: ", argv0, ": ", queuedesc, ": trouble writing to ", fn.s, ": ",
					error_str(errno), "\n");
				goto fail;
			}
			break;
		case 'F': /*- from */
			if (substdio_put(&ssinfo, todoline.s, todoline.len) == -1) {
				log9("warning: ", argv0, ": ", queuedesc, ": trouble writing to ", fn.s, ": ",
					error_str(errno), "\n");
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
					log9("warning: ", argv0, ": ", queuedesc, ": unable to create ", fn.s, ": ",
						error_str(errno), "\n");
					goto fail;
				}
				substdio_fdbuf(&sschan[c], write, fdchan[c], todobufchan[c], sizeof (todobufchan[c]));
				flagchan[c] = 1;
			}
			if (substdio_bput(&sschan[c], rwline.s, rwline.len) == -1) {
				fnmake_chanaddr(id, c);
				log9("warning: ", argv0, ": ", queuedesc, ": trouble writing to ", fn.s, ": ",
					error_str(errno), "\n");
				goto fail;
			}
			break;
		default:
			fnmake_todo(id); /* todo/split/id */
			log9("warning: ", argv0, ": ", queuedesc, ": unknown record type in ", fn.s, ": ",
				error_str(errno), "\n");
			goto fail;
		}
	}
	close(fd);
	fd = -1;
	fnmake_info(id); /* info/split/id */
	if (substdio_flush(&ssinfo) == -1) {
		log9("warning: ", argv0, ": ", queuedesc, ": trouble writing to ", fn.s, ": ",
			error_str(errno), "\n");
		goto fail;
	}
	for (c = 0; c < CHANNELS; ++c) {
		if (fdchan[c] != -1) {
			if (substdio_flush(&sschan[c]) == -1) {
				fnmake_chanaddr(id, c);
				log9("warning: ", argv0, ": ", queuedesc, ": trouble writing to ", fn.s, ": ",
					error_str(errno), "\n");
				goto fail;
			}
		}
	}
#ifdef USE_FSYNC
	if (use_fsync && fsync(fdinfo) == -1) {
		log9("warning: ", argv0, ": ", queuedesc, ": trouble fsyncing ", fn.s, ": ",
			error_str(errno), "\n");
		goto fail;
	}
#endif
	close(fdinfo);
	fdinfo = -1;
	for (c = 0; c < CHANNELS; ++c) {
		if (fdchan[c] != -1) {
#ifdef USE_FSYNC
			if (use_fsync && fsync(fdchan[c]) == -1) {
				fnmake_chanaddr(id, c);
				log9("warning: ", argv0, ": ", queuedesc, ": trouble fsyncing ", fn.s, ": ",
					error_str(errno), "\n");
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
		log9("warning: ", argv0, ": ", queuedesc, ": qmail-clean unable to clean up ", fn.s, ": ",
			error_str(errno), "\n");
		return -1;
	}
	comm_write(id, flagchan[0], flagchan[1]); /*- e.g. "DL656826\0" */
	/*- "Llocal: mbhangui@argos.indimail.org mbhangui@argos.indimail.org 798 queue1\n\0" */
	log_stat(id, st.st_size);
	/*-
	 * return in chunks of todo_chunk_size
	 * so that qmail-todo doesn't spend to much time in building
	 * comm_buf without sending a single email for delivery. This
	 * will avoid slow delivery when qmail-todo/qmail-send wasn't running
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
	if (control_init() == -1)
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
	if (control_readint(&todo_interval, "todointerval") == -1)
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
	return 1;
}

void
regetcontrols(void)
{
	int             r;

	if (!(controldir = env_get("CONTROLDIR")))
		controldir = auto_control;
	if (control_readfile(&newlocals, "locals", 1) != 1) {
		log7("alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/locals\n");
		return;
	}
	if ((r = control_readfile(&newvdoms, "virtualdomains", 0)) == -1) {
		log7("alert: ", argv0, ": ", queuedesc, ": reread ", controldir, "/virtualdomains\n");
		return;
	}
	if (control_readint(&todo_interval, "todointerval") == -1) {
		log7("alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/todointerval\n");
		return;
	}
	if (control_rldef(&envnoathost, "envnoathost", 1, "envnoathost") != 1) {
		log7("alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/envnoathost\n");
		return;
	}
#ifdef USE_FSYNC
	if (control_readint(&use_syncdir, "conf-syncdir") == -1) {
		log7("alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/conf-syncdir\n");
		return;
	}
	if (control_readint(&use_fsync, "conf-fsync") == -1) {
		log7("alert: ", argv0, ": ", queuedesc, ": unable to reread ", controldir, "/conf-fsync\n");
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
		log9("alert: ", argv0, ": ", queuedesc,
				": cannot start: unable to switch to ", auto_qmail, ": ",
				error_str(errno), "\n");
		return;
	}
	regetcontrols();
	if (!queuedir && !(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue"; /*- single queue like qmail */
	while (chdir(queuedir) == -1) {
		log9("alert: ", argv0, ": ", queuedesc,
				": unable to switch back to queue directory ", queuedir,
				": ", error_str(errno), "HELP! sleeping...\n");
		sleep(10);
	}
}

int
main(int argc, char **argv)
{
	int             nfds, r, c, opt;
	datetime_sec    wakeup;
	fd_set          rfds, wfds;
	char           *ptr;
	struct timeval  tv;

	c = str_rchr(argv[0], '/');
	argv0 = (argv[0][c] && argv[0][c + 1]) ? argv[0] + c + 1 : argv[0];
	if (!queuedir && !(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue"; /*- single queue like qmail */
	for (queuedesc = queuedir; *queuedesc; queuedesc++);
	for (; queuedesc != queuedir && *queuedesc != '/'; queuedesc--);
	if (*queuedesc == '/')
		queuedesc++;
	if (chdir(auto_qmail) == -1) {
		log9("alert: ", argv0, ": ", queuedesc,
				": cannot start: unable to switch to ", auto_qmail, ": ",
				error_str(errno), "\n");
		comm_die(111);
	}
	getEnvConfigInt(&conf_split, "CONFSPLIT", auto_split);
	if (conf_split > auto_split)
		conf_split = auto_split;
	strnum[fmt_ulong(strnum, conf_split)] = 0;
	log7("info: ", argv0, ": ", queuedesc, ": conf split=", strnum, "\n");
	if (!getcontrols()) {
		log5("alert: ", argv0, ": ", queuedesc,
				": cannot start: unable to read controls or out of memory\n");
		comm_die(111);
	}
	if (chdir(queuedir) == -1) {
		log9("alert: ", argv0, ": ", queuedesc,
				": cannot start: unable to switch to queue directory",
				queuedir, ": ", error_str(errno), "\n");
		comm_die(111);
	}
#ifdef USE_FSYNC
	if (env_get("USE_FSYNC"))
		use_fsync = 1;
#endif
	sig_pipeignore();
	umask(077);

	while ((opt = getopt(argc, argv, "sd")) != opteof) {
		switch (opt)
		{
			case 'd':
#ifndef HASLIBRT
				log5("alert: ", argv0, ": ", queuedesc, ": dynamic queue not supported\n");
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
	if (!(ptr = env_get("TODO_CHUNK_SIZE")))
		todo_chunk_size = CHUNK_SIZE;
	else {
		scan_int(ptr, &todo_chunk_size);
		if (todo_chunk_size <= 0)
			todo_chunk_size = CHUNK_SIZE;
	}
	fnmake_init(); /*- initialize fn */
#ifdef HASLIBRT
	if (dynamic_queue)
		mqueue_init();
	todo_init();   /*- set nexttodorun, open lock/trigger */
#else
	todo_init();   /*- set nexttodorun, open lock/trigger */
#endif
	comm_init();   /*- assign fd 2 to queue comm to, 3 to queue comm from */
	for (;;) {
		/*- read from fd 0 (qmail-send) */
		if ((r = read(fdin, &c, 1)) == -1) {
			if (errno == error_intr)
				continue;
			_exit(100);	/*- read failed probably qmail-send died */
		}
		if (!r)
			_exit(100);	/*- read failed probably qmail-send died */
		break;
	} /*- we assume it is a 'S' */
	for (;;) {
		recent = now();
		if (flagreadasap) {
			flagreadasap = 0;
			reread();
		}
		if (!flagsendalive) {
			/*- qmail-send finally exited, so do the same. */
			if (flagstopasap)
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
			tv.tv_sec = wakeup - recent + SLEEP_FUZZ;
		tv.tv_usec = 0;
		if (select(nfds, &rfds, &wfds, (fd_set *) 0, &tv) == -1) {
			if (errno == error_intr);
			else
				log5("warning: ", argv0, ": ", queuedesc, ": trouble in select\n");
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
			while (todo_do(&rfds) == 2);
			comm_do(&wfds, &rfds); /*- communicate with qmail-send on fd 0, fd 1 */
		}
	} /*- for (;;) */
	log5("status: ", argv0, ": ", queuedesc, " exiting\n");
	_exit(0);
}

void
getversion_qmail_todo_c()
{
	static char    *x = "$Id: qmail-todo.c,v 1.54 2021-11-27 22:26:31+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}

/*
 * $Log: qmail-todo.c,v $
 * Revision 1.54  2021-12-12 08:47:50+05:30  Cprogrammer
 * use argv0 for program name
 * transmit messid to qmail-send in TODO_CHUNK_SIZE
 *
 * Revision 1.53  2021-10-22 14:00:03+05:30  Cprogrammer
 * fixed typo
 *
 * Revision 1.52  2021-10-20 22:47:41+05:30  Cprogrammer
 * display program 'qmail-todo' in logs for identification
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
 * display qmail-todo prefix for startup message
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
 * added qmail-todo.h
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
 * display queue directory for qmail-todo process
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
