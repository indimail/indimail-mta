/*
 * $Id: qmail-qread.c,v 1.48 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stralloc.h>
#include <substdio.h>
#include <subfd.h>
#include <fmt.h>
#include <getln.h>
#include <sgetopt.h>
#include <open.h>
#include <datetime.h>
#include <date822fmt.h>
#include <strerr.h>
#include <error.h>
#include <getEnvConfig.h>
#include <qprintf.h>
#include <alloc.h>
#include <env.h>
#ifndef MULTI_QUEUE
#include <env.h>
#include "auto_qmail.h"
#endif
#include "haslibrt.h"
#include "queue_load.h"
#include "auto_split.h"
#include "readsubdir.h"
#include "fmtqfn.h"
#include "control.h"
#include "variables.h"
#include "process_queue.h"
#include "set_environment.h"

#define FATAL "qmail-qread: fatal: "
#define WARN  "qmail-qread: warn: "

#ifndef QUEUE_COUNT
#define QUEUE_COUNT 10
#endif

static readsubdir rs;
static char     fnmess[FMTQFN];
static char     fninfo[FMTQFN];
static char     fnlocal[FMTQFN];
static char     fnremote[FMTQFN];
static char     fnbounce[FMTQFN];
static char     inbuf[1024];
static stralloc sender = { 0 };
static datetime_sec curtime;
static int      flagbounce, doLocal = 0, doRemote = 0, doTodo = 0, doCount = 0, silent = 0;
#ifdef HASLIBRT
static int      doshm = 0, dynamic_queue = -1;
#endif
static unsigned long   id;
static unsigned long   size;
static stralloc stats = { 0 };

static void
die_opendir(const char *fn)
{
	strerr_die4sys(111, FATAL, "unable to opendir ", fn, ": ");
}

static void
err(unsigned long _id)
{
	char            foo[FMT_ULONG];

	foo[fmt_ulong(foo, _id)] = 0;
	strerr_warn4(WARN, "trouble with #", foo, ": ", &strerr_sys);
}

unsigned int
fmtstats(char *s)
{
	char           *ptr;
	struct datetime dt;
	unsigned int    len;
	unsigned int    i;

	len = 0;
	datetime_tai(&dt, curtime);
	i = date822fmt(s, &dt) - 7 /*XXX*/;
	len += i;
	if (s)
		s += i;

	i = fmt_str(s, " GMT  ");
	len += i;
	if (s)
		s += i;
	/*-------------*/
	for(i = 0, ptr = fninfo + 5;s && *ptr && *ptr != '/';*s++ = *ptr++, len++);
	len += i;
	i = fmt_str(s, "/#");
	len += i;
	if (s)
		s += i;
	/*-------------*/
	i = fmt_ulong(s, id);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, "  ");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, size);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, "  <");
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, sender.s + 1);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, "> ");
	len += i;
	if (s)
		s += i;
	if (flagbounce) {
		i = fmt_str(s, " bouncing");
		len += i;
		if (s)
			s += i;
	}
	return len;
}

void
out(char *s, unsigned int n)
{
	if (silent)
		return;
	while (n > 0) {
		substdio_put(subfdout, ((*s >= 32) && (*s <= 126)) ? s : "_", 1);
		--n;
		++s;
	}
}

void
putstats()
{
	if (silent)
		return;
	if (!stralloc_ready(&stats, fmtstats(FMT_LEN)))
		strerr_die2x(111, FATAL, "out of memory");
	stats.len = fmtstats(stats.s);
	out(stats.s, stats.len);
	substdio_puts(subfdout, "\n");
}

void
usage()
{
	substdio_puts(subfdout, "qmail-read [-calrt]\n");
	substdio_puts(subfdout, "\t-a - Display local, remote and todo Queues\n");
	substdio_puts(subfdout, "\t-c - Display Counts\n");
	substdio_puts(subfdout, "\t-l - Display local  Queue\n");
	substdio_puts(subfdout, "\t-r - Display remote Queue\n");
#ifdef HASLIBRT
	substdio_puts(subfdout, "\t-i - Display queue counts, concurrencies queue load\n");
#endif
	substdio_flush(subfdout);
}

static int
get_arguments(int argc, char **argv)
{
	int             c;
	int             errflag;

	errflag = 0;
	doTodo = doCount = doLocal = doRemote = 0;
#ifdef HASLIBRT
	if (access("/dev/shm/qscheduler", F_OK)) {
		if (errno != error_noent)
			strerr_die2sys(111, FATAL, "/dev/shm/qscheduler: ");
		doshm = -1;
	} else
		doshm = 0;
	while (!errflag && (c = getopt(argc, argv, "calrsti")) != opteof) {
#else
	while (!errflag && (c = getopt(argc, argv, "calrst")) != opteof) {
#endif
		switch (c)
		{
		case 's':
			silent = 1;
			break;
		case 'c':
			doCount = 1;
			break;
		case 'a':
			doTodo = 1;
			doLocal = 1;
			doRemote = 1;
#ifdef HASLIBRT
			if (!doshm)
				doshm = 2;
#endif
			break;
		case 'l':
			doLocal = 1;
			break;
		case 'r':
			doRemote = 1;
			break;
		case 't':
			doTodo = 1;
			break;
#ifdef HASLIBRT
		case 'i':
			if (doshm == -1)
				strerr_die2x(100, FATAL, "dynamic queue not configured");
			doshm = 1;
			break;
#endif
		case 'h':
		default:
			usage();
			return(1);
			break;
		}
	}
#ifdef HASLIBRT
	if (doshm == -1)
		doshm = 0;
	if (!doTodo && !doLocal && !doRemote && !doshm)
#else
	if (!doTodo && !doLocal && !doRemote)
#endif
		doLocal = doRemote = 1;
	return(0);
}

void
putcounts(const char *pre_str, int lCount, int rCount, int bCount, int tCount)
{
	char            foo[FMT_ULONG];

	if (doLocal) {
		if (pre_str)
			substdio_puts(subfdout, pre_str);
		substdio_puts(subfdout, "Messages in local  queue: ");
		foo[fmt_ulong(foo, lCount)] = 0;
		substdio_puts(subfdout, foo);
		substdio_puts(subfdout, "\n");
	}
	if (doRemote) {
		if (pre_str)
			substdio_puts(subfdout, pre_str);
		substdio_puts(subfdout, "Messages in remote queue: ");
		foo[fmt_ulong(foo, rCount)] = 0;
		substdio_puts(subfdout, foo);
		substdio_puts(subfdout, "\n");
	}
	if (pre_str)
		substdio_puts(subfdout, pre_str);
	substdio_puts(subfdout, "Messages in bounce queue: ");
	foo[fmt_ulong(foo, bCount)] = 0;
	substdio_puts(subfdout, foo);
	substdio_puts(subfdout, "\n");
	if (doTodo) {
		if (pre_str)
			substdio_puts(subfdout, pre_str);
		substdio_puts(subfdout, "Messages in todo   queue: ");
		foo[fmt_ulong(foo, tCount)] = 0;
		substdio_puts(subfdout, foo);
		substdio_puts(subfdout, "\n");
	}
}

#ifdef HASLIBRT
void
display(QDEF *queue, int queue_count, int queue_conf)
{
	int             i, x, lcur = 0, rcur = 0, min = -1;
	double          threshold;
	double          load_l = 0.0, load_r = 0.0;

	getEnvConfigDouble(&threshold,   "QUEUE_LOAD",   QUEUE_LOAD);
	for (i = 0; i < queue_count; i++) {
		x = queue[i].lcur + queue[i].rcur;
		if (min == -1 || x < min)
			min = x; /*- minimum concurrency */
	}
	for (i = 0; i < queue_count; i++) {
		if (!i)
			substdio_put(subfdout, "queue      local  Remote   +/- flag\n", 36);
		if (!queue[i].lmax || !queue[i].rmax) {
			substdio_put(subfderr, "invalid concurrency = 0 ", 24);
			if (!queue[i].lmax)
				substdio_put(subfderr, "[local] ", 8);
			if (!queue[i].rmax)
				substdio_put(subfderr, "[remote] ", 9);
			substdio_put(subfderr, "for queue ", 10);
			substdio_put(subfderr, queue[i].queue.s, queue[i].queue.len);
			substdio_put(subfderr, "\n", 1);
			substdio_flush(subfderr);
			continue;
		}
		lcur += queue[i].lcur;
		rcur += queue[i].rcur;
		load_l += (double) queue[i].lcur / queue[i].lmax;
		load_r += (double) queue[i].rcur / queue[i].rmax;
		x = queue[i].lcur + queue[i].rcur;
		subprintf(subfdout, "%-8s %3d/%-4d %3d/%-4d %c %s\n",
				queue[i].queue.s, queue[i].lcur, queue[i].lmax,
				queue[i].rcur, queue[i].rmax, x == min ? '-' : '+',
				queue[i].flag ? "disabled" : "enabled");
	}
	subprintf(subfdout, "threshold = %-6.2f, qcount = %d, qconf = %d, local = %d, remote = %d, qload_av = %6.2f\n",
			threshold, queue_count, queue_conf, lcur, rcur, 50 * (load_l + load_r)/queue_count);
	substdio_flush(subfdout);
}
#endif

#ifdef MULTI_QUEUE
int
main_function(int *lcount, int *rcount, int *bcount, int *tcount)
#else
int
main(int argc, char **argv)
#endif
{
	int             channel, match, fd, x, flag, bigtodo,
					bCount, lCount, rCount, tCount;
	struct stat     st;
	substdio        ss;
	static stralloc line = { 0 };
#ifndef MULTI_QUEUE
#ifdef HASLIBRT
	int             qcount, qconf;
	QDEF           *queue = (QDEF *) NULL;
#endif
	char           *qbase;
	static stralloc QueueBase = { 0 };
#endif

#ifndef MULTI_QUEUE
	if (get_arguments(argc, argv)) {
		substdio_flush(subfdout);
		return(1);
	}
	if (chdir(auto_qmail) == -1)
		strerr_die4sys(111, FATAL, "unable to switch to ", auto_qmail, ": ");
	if (!(qbase = env_get("QUEUE_BASE"))) {
		switch (control_readfile(&QueueBase, "queue_base", 0))
		{
		case -1:
			strerr_die2sys(111, FATAL, "unable to read controls: ");
			break;
		case 0:
			if (!stralloc_copys(&QueueBase, auto_qmail) ||
					!stralloc_catb(&QueueBase, "/queue", 6) ||
					!stralloc_0(&QueueBase))
				strerr_die2x(111, FATAL, "out of memory");
			qbase = QueueBase.s;
			break;
		case 1:
			qbase = QueueBase.s;
			break;
		}
	}
	if (chdir(qbase) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", qbase, ": ");
	if (!queuedir && !(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue";
#endif
	if (chdir(queuedir) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", queuedir, ": ");
	getEnvConfigInt(&bigtodo, "BIGTODO", 1);
	getEnvConfigInt(&conf_split, "CONFSPLIT", auto_split);
	if (conf_split > auto_split)
		conf_split = auto_split;
	readsubdir_init(&rs, "info", 1, die_opendir);
	bCount = lCount = rCount = tCount = 0;
	while ((x = readsubdir_next(&rs, &id))) {
		if (x > 0) {
			fmtqfn(fnmess, "mess/", id, 1);
			fmtqfn(fninfo, "info/", id, 1);
			fmtqfn(fnlocal, "local/", id, 1);
			fmtqfn(fnremote, "remote/", id, 1);
			fmtqfn(fnbounce, "bounce/", id, 0);
			if (stat(fnmess, &st) == -1) {
				err(id);
				continue;
			}
			size = st.st_size;
			if ((flagbounce = !stat(fnbounce, &st)))
				bCount++;
			if ((fd = open_read(fninfo)) == -1) {
				err(id);
				continue;
			}
			substdio_fdbuf(&ss, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
			if (getln(&ss, &sender, &match, 0) == -1)
				strerr_die2x(111, FATAL, "out of memory");
			if (fstat(fd, &st) == -1) {
				close(fd);
				err(id);
				continue;
			}
			close(fd);
			curtime = st.st_mtime;
			for (flag = 0,channel = (doLocal ? 0 : 1); channel < (doRemote ? 2 : 1); ++channel) {
				if ((fd = open_read(channel ? fnremote : fnlocal)) == -1) {
					if (errno != error_noent)
						err(id);
				} else {
					for (;;) {
						if (getln(&ss, &line, &match, 0) == -1)
							strerr_die2x(111, FATAL, "out of memory");
						if (!match)
							break;
						switch (line.s[0])
						{
						case 'D':
							if (!flag++)
								putstats();
							if (!silent)
								substdio_puts(subfdout, "  done");
						case 'T':
							if (!flag++)
								putstats();
							if (channel)
								rCount++;
							else
								lCount++;
							if (!silent) {
								substdio_puts(subfdout, channel ? "\tremote\t" : "\tlocal\t");
								substdio_puts(subfdout, line.s + 1);
								substdio_puts(subfdout, "\n");
							}
							break;
						}
					}
					close(fd);
				}
			}
		}
	} /*- while (x = readsubdir_next(&rs, &id)) */
	if (!doTodo && !doCount) {
		if (!silent)
			substdio_flush(subfdout);
		return(0);
	}
	flagbounce = 0;
	readsubdir_init(&rs, "todo", bigtodo, die_opendir);
	while ((x = readsubdir_next(&rs, &id))) {
		if (x > 0) {
			fmtqfn(fnmess, "mess/", id, 1);
			fmtqfn(fninfo, "todo/", id, bigtodo);
			if (stat(fnmess, &st) == -1) {
				err(id);
				continue;
			}
			size = st.st_size;
			if ((fd = open_read(fninfo)) == -1) {
				err(id);
				continue;
			}
			substdio_fdbuf(&ss, (ssize_t (*)(int,  char *, size_t)) read, fd, inbuf, sizeof(inbuf));
			for (;;) {
				if (getln(&ss, &sender, &match, 0) == -1)
					strerr_die2x(111, FATAL, "out of memory");
				if (!match || sender.s[0] == 'F')
					break;
			}
			if (fstat(fd, &st) == -1) {
				close(fd);
				err(id);
				continue;
			}
			close(fd);
			curtime = st.st_mtime;
			for (flag = 0,channel = 0; channel < 2; ++channel) {
				if ((fd = open_read(fnmess)) == -1) {
					if (errno != error_noent)
						err(id);
				} else {
					for (;;) {
						if (getln(&ss, &line, &match, 0) == -1)
							strerr_die2x(111, FATAL, "out of memory");
						if (!match)
							break;
						switch (line.s[0]) {
						case 'D':
							if (!flag++)
								putstats();
							if (!silent)
								substdio_puts(subfdout, "  done");
						case 'T':
							if (!flag++)
								putstats();
							tCount++;
							if (!silent) {
								substdio_puts(subfdout, "\ttodo\t");
								substdio_puts(subfdout, line.s + 1);
								substdio_puts(subfdout, "\n");
							}
							break;
						}
					}
					close(fd);
				}
			}
		}
	} /*- while (x = readsubdir_next(&rs, &id)) */
#ifdef MULTI_QUEUE
	if (lcount)
		*lcount += lCount;
	if (rcount)
		*rcount += rCount;
	if (bcount)
		*bcount += bCount;
	if (tcount)
		*tcount += tCount;
	if (doCount)
		putcounts(0, lCount, rCount, bCount, tCount);
#else
	if (doCount)
		putcounts("Total ", lCount, rCount, bCount, tCount);
#ifdef HASLIBRT
	if (doshm) {
		/*- if dynamic_queue is set, don't bother with set_environment */
		if (dynamic_queue == -1) {
			set_environment(WARN, FATAL, 0);
			dynamic_queue = env_get("DYNAMIC_QUEUE") ? 1 : 0;
		}
		if (!dynamic_queue)
			doshm = 0;
		if (doshm) {
			queue_load("qmail-qread", &qcount, &qconf, 0, &queue);
			display(queue, qcount, qconf);
		}
	}
#endif
#endif
	substdio_flush(subfdout);
	return(0);
}

#ifdef MULTI_QUEUE
int
main(int argc, char **argv)
{
	int             lcount = 0, rcount = 0, bcount = 0, tcount = 0;
#ifdef HASLIBRT
	int             qcount, qconf;
	QDEF           *queue = (QDEF *) NULL;
#endif

	if (get_arguments(argc, argv)) {
		substdio_flush(subfdout);
		return(1);
	}
	if (doLocal || doRemote || doTodo || doCount) {
		process_queue(WARN, FATAL, main_function, &lcount, &rcount, &bcount, &tcount);
#ifdef HASLIBRT
		/*-
		 * process queue calls set_environment
		 * so we set dynamic_queue here to prevent calling
		 * set_environment again later and avoid duplicating
		 * envdir
		 */
		dynamic_queue = env_get("DYNAMIC_QUEUE") ? 1 : 0;
#endif
		if (doCount)
			putcounts("Total ", lcount, rcount, bcount, tcount);
		substdio_flush(subfdout);
	}
#ifdef HASLIBRT
	if (doshm) {
		/*- if dynamic_queue is set, don't bother with set_environment */
		if (dynamic_queue == -1) {
			set_environment(WARN, FATAL, 1);
			dynamic_queue = env_get("DYNAMIC_QUEUE") ? 1 : 0;
		}
		if (!dynamic_queue)
			doshm = 0;
		if (doshm) {
			queue_load("qmail-qread", &qcount, &qconf, 0, &queue);
			display(queue, qcount, qconf);
		}
	}
#endif
	return(0);
}
#endif

void
getversion_qmail_qread_c()
{
	const char     *x = "$Id: qmail-qread.c,v 1.48 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}

/*
 * $Log: qmail-qread.c,v $
 * Revision 1.48  2025-01-22 00:30:35+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.47  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.46  2023-02-08 15:10:42+05:30  Cprogrammer
 * auto determine dynamic queue using /dev/shm/qscheduler
 *
 * Revision 1.45  2023-02-07 20:36:05+05:30  Cprogrammer
 * BUG: Fixed SIGSEGV
 * skip dynamic queue if DYNAMIC_QUEUE is not set
 *
 * Revision 1.44  2023-01-18 00:02:13+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
 * Revision 1.43  2022-04-23 09:44:19+05:30  Cprogrammer
 * display qload average instead of total load
 *
 * Revision 1.42  2022-04-23 00:13:49+05:30  Cprogrammer
 * set default env variables from $HOME/.defaultqueue
 *
 * Revision 1.41  2022-04-22 11:52:42+05:30  Cprogrammer
 * display threshold
 *
 * Revision 1.40  2022-04-19 08:20:23+05:30  Cprogrammer
 * display qload for use in qtop
 * display queues configured
 * added -i option to display dynamic queue information
 * added -s option to display only counts with -c option
 * allow configurable big/small todo/intd
 *
 * Revision 1.39  2021-10-21 14:42:22+05:30  Cprogrammer
 * chdir to auto_qmail instead of auto_sysconfdir
 *
 * Revision 1.38  2021-06-28 17:06:33+05:30  Cprogrammer
 * use process_queue to process all queues
 *
 * Revision 1.37  2021-06-27 10:45:09+05:30  Cprogrammer
 * moved conf_split variable to fmtqfn.c
 *
 * Revision 1.36  2021-06-05 12:51:37+05:30  Cprogrammer
 * process special queue "slowq"
 *
 * Revision 1.35  2021-05-29 23:50:11+05:30  Cprogrammer
 * fixed qbase path
 *
 * Revision 1.34  2021-05-26 10:45:51+05:30  Cprogrammer
 * handle access() error other than ENOENT
 *
 * Revision 1.33  2021-05-16 00:46:51+05:30  Cprogrammer
 * limit conf_split to compile time value in conf-split
 *
 * Revision 1.32  2021-05-13 14:44:12+05:30  Cprogrammer
 * use set_environment() to set env from ~/.defaultqueue or control/defaultqueue
 *
 * Revision 1.31  2021-05-12 15:49:49+05:30  Cprogrammer
 * set conf_split from CONFSPLIT env variable
 *
 * Revision 1.30  2020-11-24 13:47:14+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.29  2020-04-04 12:13:55+05:30  Cprogrammer
 * use environment variables $HOME/.defaultqueue before /etc/indimail/control/defaultqueue
 *
 * Revision 1.28  2019-06-07 11:26:32+05:30  Cprogrammer
 * replaced getopt() with subgetopt()
 *
 * Revision 1.27  2016-06-13 17:49:11+05:30  Cprogrammer
 * removed ifdef indimail
 *
 * Revision 1.26  2016-05-17 19:44:58+05:30  Cprogrammer
 * use auto_control, set by conf-control to set control directory
 *
 * Revision 1.25  2016-01-29 11:55:52+05:30  Cprogrammer
 * use defaultqueue to set env variables and set QUEUE variables
 *
 * Revision 1.24  2011-07-29 09:29:40+05:30  Cprogrammer
 * fixed gcc 4.6 warnings
 *
 * Revision 1.23  2011-02-12 15:35:36+05:30  Cprogrammer
 * added usage() function
 *
 * Revision 1.22  2010-07-15 08:24:44+05:30  Cprogrammer
 * ability to toggle local, remote queues when displaying counts
 *
 * Revision 1.21  2010-05-20 11:28:46+05:30  Cprogrammer
 * added queue count functionality of qmail-qstat
 *
 * Revision 1.20  2010-02-17 10:38:55+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.19  2010-02-10 08:58:43+05:30  Cprogrammer
 * use -DMULTI_QUEUE to use multiple queues
 *
 * Revision 1.18  2009-11-09 19:55:08+05:30  Cprogrammer
 * Use control file queue_base to process multiple indimail queues
 *
 * Revision 1.17  2009-09-08 13:32:36+05:30  Cprogrammer
 * define default value for QUEUE_COUNT
 *
 * Revision 1.16  2007-12-20 12:49:10+05:30  Cprogrammer
 * removed compiler warnings
 *
 * Revision 1.15  2005-03-03 14:37:04+05:30  Cprogrammer
 * assign queuedir after stralloc operation
 *
 * Revision 1.14  2004-10-22 20:33:14+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.13  2004-10-22 15:37:43+05:30  Cprogrammer
 * replaced readwrite.h with unistd.h
 *
 * Revision 1.12  2004-10-09 00:28:12+05:30  Cprogrammer
 * use stralloc variable for queuedir
 *
 * Revision 1.11  2004-09-22 23:13:00+05:30  Cprogrammer
 * replaced atoi() with scan_int()
 *
 * Revision 1.10  2004-07-17 21:21:14+05:30  Cprogrammer
 * added RCS log
 *
 */
