/*
 * $Id: qmail-qread.c,v 1.44 2022-04-14 19:53:47+05:30 Cprogrammer Exp mbhangui $
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
#ifndef MULTI_QUEUE
#include <env.h>
#include "auto_qmail.h"
#endif
#include "haslibrt.h"
#ifdef HASLIBRT
#include <fcntl.h>
#include <sys/mman.h>
#endif
#include "auto_split.h"
#include "readsubdir.h"
#include "fmtqfn.h"
#include "control.h"
#include "variables.h"
#include "process_queue.h"

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
static int      doshm = 0;
#endif
static unsigned long   id;
static unsigned long   size;
static stralloc stats = { 0 };
typedef struct {
	stralloc queue;
	int lcur;
	int lmax;
	int rcur;
	int rmax;
	char flag;
} qdef;

static void
die_opendir(char *fn)
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

#ifdef HASLIBRT
void
read_shm(int flag)
{
	int             shm, i, j, qcount, qconf, len, x, min = -1;
	double          load;
	int             q[5];
	qdef           *queue;
	char            shm_name[256], strnum[FMT_DOUBLE];
	char           *s;

	/*- get queue count */
	if ((shm = shm_open("/qscheduler", O_RDONLY, 0644)) == -1) {
		if (errno == error_noent) {
			if (flag)
				strerr_die2sys(111, FATAL, "dynamic queue disabled / qscheduler not running\n");
			else {
				substdio_puts(subfdout, "no active dynamic queue\n");
				return;
			}
		}
		strerr_die2x(111, FATAL, "unable to open POSIX shared memory segment /qscheduler");
	}
	if (read(shm, (char *) q, sizeof(int) * 2) == -1)
		strerr_die2x(111, FATAL, "unable to read POSIX shared memory segment /qscheduler");
	close(shm);
	qcount = q[0];
	qconf = q[1];
	if (!(queue = (qdef *) alloc(qcount * sizeof(qdef))))
		strerr_die2x(111, FATAL, "out of memory");
	for (j = 0, load = 0.0, min = -1; j < qcount; j++) {
		len = 0;
		s = shm_name;
		s += (i = fmt_str(s, "/queue"));
		len += i;
		s += (i = fmt_uint(s, j + 1));
		len += i;
		*s++ = 0;
		if (!stralloc_copyb(&queue[j].queue, shm_name + 1, len - 1) ||
				!stralloc_0(&queue[j].queue))
			strerr_die2x(111, FATAL, "out of memory");
		if ((shm = shm_open(shm_name, O_RDONLY, 0600)) == -1)
			strerr_die4sys(111, FATAL, "failed to open POSIX shared memory segment ", shm_name, ": ");
		if (read(shm, (char *) q, sizeof(int) * 5) == -1)
			strerr_die4sys(111, FATAL, "failed to read POSIX shared memory segment ", shm_name, ": ");
		close(shm);
		if (!q[2] || !q[3]) {
			substdio_put(subfderr, "invalid concurrency = 0 ", 24);
			if (!queue[j].lmax)
				substdio_put(subfderr, "[local] ", 8);
			if (!queue[j].rmax)
				substdio_put(subfderr, "[remote] ", 9);
			substdio_put(subfderr, "for queue ", 10);
			substdio_put(subfderr, shm_name + 1, len - 1);
			substdio_put(subfderr, "\n", 1);
			substdio_flush(subfderr);
			continue;
		}
		queue[j].lcur = q[0];
		queue[j].lmax = q[2];
		queue[j].rcur = q[1];
		queue[j].rmax = q[3];
		queue[j].flag = q[4];
		x = q[0] + q[1];
		load += (double) q[0] / q[2] + (double) q[1]/ q[3];
		if (min == -1 || x < min)
			min = x; /*- minimum concurrency */
	}
	for (j = 0; j < qcount; j++) {
		if (!j)
			substdio_put(subfdout, "queue      local  Remote   +/- flag\n", 36);
		if (!queue[j].lmax || !queue[j].rmax) {
			substdio_put(subfderr, "invalid concurrency = 0 ", 24);
			if (!queue[j].lmax)
				substdio_put(subfderr, "[local] ", 8);
			if (!queue[j].rmax)
				substdio_put(subfderr, "[remote] ", 9);
			substdio_put(subfderr, "for queue ", 10);
			substdio_put(subfderr, shm_name + 1, len - 1);
			substdio_put(subfderr, "\n", 1);
			substdio_flush(subfderr);
			continue;
		}
		x = queue[j].lcur + queue[j].rcur;
		qprintf(subfdout, queue[j].queue.s, "%-08s");
		substdio_put(subfdout, " ", 1);
		strnum[i = fmt_ulong(strnum, queue[j].lcur)] = 0;
		qprintf(subfdout, strnum, "%+3s");
		substdio_put(subfdout, "/", 1);
		strnum[i = fmt_ulong(strnum, queue[j].lmax)] = 0;
		qprintf(subfdout, strnum, "%-4s");
		substdio_put(subfdout, " ", 1);
		strnum[i = fmt_ulong(strnum, queue[j].rcur)] = 0;
		qprintf(subfdout, strnum, "%+3s");
		substdio_put(subfdout, "/", 1);
		strnum[i = fmt_ulong(strnum, queue[j].rmax)] = 0;
		qprintf(subfdout, strnum, "%-4s");
		substdio_put(subfdout, x == min ? " - " : " + ", 3);
		substdio_put(subfdout, queue[j].flag ? " disabled\n" : "  enabled\n", 10);
	}
	substdio_put(subfdout, "queue count = ", 14);
	strnum[i = fmt_ulong(strnum, qcount)] = 0;
	substdio_put(subfdout, strnum, i);
	substdio_put(subfdout, ", queue configured = ", 21);
	strnum[i = fmt_ulong(strnum, qconf)] = 0;
	substdio_put(subfdout, strnum, i);
	substdio_put(subfdout, ", Total queue load = ", 19);
	strnum[i = fmt_double(strnum, 100 * load, 4)] = 0;
	substdio_put(subfdout, strnum, i);
	substdio_put(subfdout, "\n", 1);
	substdio_flush(subfdout);
	return;
}
#endif

static int
get_arguments(int argc, char **argv)
{
	int             c;
	int             errflag;

	errflag = 0;
	doTodo = doCount = doLocal = doRemote = 0;
#ifdef HASLIBRT
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
	if (!doTodo && !doLocal && !doRemote && !doshm)
#else
	if (!doTodo && !doLocal && !doRemote)
#endif
		doLocal = doRemote = 1;
	return(0);
}

void
putcounts(char *pre_str, int lCount, int rCount, int bCount, int tCount)
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
			substdio_fdbuf(&ss, read, fd, inbuf, sizeof(inbuf));
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
			fmtqfn(fninfo, "todo/", id, 1);
			if (stat(fnmess, &st) == -1) {
				err(id);
				continue;
			}
			size = st.st_size;
			if ((fd = open_read(fninfo)) == -1) {
				err(id);
				continue;
			}
			substdio_fdbuf(&ss, read, fd, inbuf, sizeof(inbuf));
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
	if (doshm)
		read_shm(doshm == 1 ? 0 : 1);
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

	if (get_arguments(argc, argv)) {
		substdio_flush(subfdout);
		return(1);
	}
	if (doLocal || doRemote || doTodo || doCount) {
		process_queue(WARN, FATAL, main_function, &lcount, &rcount, &bcount, &tcount);
		if (doCount)
			putcounts("Total ", lcount, rcount, bcount, tcount);
		substdio_flush(subfdout);
	}
#ifdef HASLIBRT
	if (doshm)
		read_shm(doshm == 1 ? 0 : 1);
#endif
	return(0);
}
#endif

void
getversion_qmail_qread_c()
{
	static char    *x = "$Id: qmail-qread.c,v 1.44 2022-04-14 19:53:47+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}

/*
 * $Log: qmail-qread.c,v $
 * Revision 1.44  2022-04-14 19:53:47+05:30  Cprogrammer
 * added header for queue load display
 *
 * Revision 1.43  2022-04-14 13:50:32+05:30  Cprogrammer
 * display qload for use in qtop
 *
 * Revision 1.42  2022-04-13 16:59:25+05:30  Cprogrammer
 * display queues configured
 *
 * Revision 1.41  2022-04-13 08:29:19+05:30  Cprogrammer
 * added -i option to display dynamic queue information
 *
 * Revision 1.40  2022-01-30 08:42:40+05:30  Cprogrammer
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
