/*
 * $Log: qmail-qread.c,v $
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include "stralloc.h"
#include "substdio.h"
#include "subfd.h"
#include "scan.h"
#include "fmt.h"
#include "str.h"
#include "getln.h"
#include "sgetopt.h"
#include "fmtqfn.h"
#include "readsubdir.h"
#include "open.h"
#include "strerr.h"
#include "datetime.h"
#include "date822fmt.h"
#include "env.h"
#include "error.h"
#include "envdir.h"
#include "pathexec.h"
#include "getEnvConfig.h"
#include "auto_qmail.h"
#include "auto_split.h"
#include "auto_sysconfdir.h"
#include "control.h"
#include "variables.h"
#include "set_environment.h"

#define FATAL "qmail-qread: fatal: "
#define WARN  "qmail-qread: warn: "

#ifndef QUEUE_COUNT
#define QUEUE_COUNT 10
#endif

readsubdir      rs;
int             conf_split;

void
die(n)
	int             n;
{
	substdio_flush(subfderr);
	_exit(n);
}

void
warn(s1, s2)
	char           *s1;
	char           *s2;
{
	char           *x;
	x = error_str(errno);
	substdio_puts(subfderr, s1);
	substdio_puts(subfderr, s2);
	substdio_puts(subfderr, ": ");
	substdio_puts(subfderr, x);
	substdio_puts(subfderr, "\n");
}

void
die_nomem()
{
	substdio_puts(subfderr, "fatal: out of memory\n");
	die(111);
}

void
die_chdir(char *dir)
{
	warn("fatal: unable to chdir to ", dir);
	die(111);
}

void
die_opendir(fn)
	char           *fn;
{
	warn("fatal: unable to opendir ", fn);
	die(111);
}

void
die_control()
{
	substdio_puts(subfderr, "fatal: unable to read controls\n");
	die(111);
}

void
err(id)
	unsigned long   id;
{
	char            foo[FMT_ULONG];
	foo[fmt_ulong(foo, id)] = 0;
	warn("warning: trouble with #", foo);
}

char            fnmess[FMTQFN];
char            fninfo[FMTQFN];
char            fnlocal[FMTQFN];
char            fnremote[FMTQFN];
char            fnbounce[FMTQFN];

char            inbuf[1024];
stralloc        sender = { 0 };

unsigned long   id;
datetime_sec    qtime;
int             flagbounce;
unsigned long   size;

unsigned int
fmtstats(s)
	char           *s;
{
	char           *ptr;
	struct datetime dt;
	unsigned int    len;
	unsigned int    i;

	len = 0;
	datetime_tai(&dt, qtime);
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

stralloc        stats = { 0 };

void
out(s, n)
	char           *s;
	unsigned int    n;
{
	while (n > 0) {
		substdio_put(subfdout, ((*s >= 32) && (*s <= 126)) ? s : "_", 1);
		--n;
		++s;
	}
}
void
outs(s)
	char           *s;
{
	out(s, str_len(s));
}

void
outok(s)
	char           *s;
{
	substdio_puts(subfdout, s);
}

void
putstats(int doCount)
{
	if (!stralloc_ready(&stats, fmtstats(FMT_LEN)))
		die_nomem();
	stats.len = fmtstats(stats.s);
	out(stats.s, stats.len);
	outok("\n");
}

int             doLocal = 0, doRemote = 0, doTodo = 0, doCount = 0;

#ifdef MULTI_QUEUE

void
usage()
{
	outs("qmail-read [-calrt]");
	outok("\n\t");
	outs("-a - Display local, remote and todo Queues");
	outok("\n\t");
	outs("-c - Display Counts");
	outok("\n\t");
	outs("-l - Display local  Queue");
	outok("\n\t");
	outs("-r - Display remote Queue");
	outok("\n");
	substdio_flush(subfdout);
}

static int
get_arguments(int argc, char **argv)
{
	int             c;
	int             errflag;

	errflag = 0;
	doTodo = 0;
	doCount = 0;
	while (!errflag && (c = getopt(argc, argv, "calrt")) != opteof) {
		switch (c)
		{
		case 'c':
			doCount = 1;
			break;
		case 'a':
			doTodo = 1;
			doLocal = 1;
			doRemote = 1;
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
		case 'h':
		default:
			usage();
			return(1);
			break;
		}
	}
	if (!doTodo && !doLocal && !doRemote)
		doLocal = doRemote = 1;
	return(0);
}
#endif

void
putcounts(char *pre_str, int lCount, int rCount, int bCount, int tCount)
{
	char            foo[FMT_ULONG];

	if (doLocal) {
	if (pre_str)
		outs(pre_str);
	outs("Messages in local  queue: ");
	foo[fmt_ulong(foo, lCount)] = 0;
	outs(foo);
	outok("\n");
	}
	if (doRemote) {
	if (pre_str)
		outs(pre_str);
	outs("Messages in remote queue: ");
	foo[fmt_ulong(foo, rCount)] = 0;
	outs(foo);
	outok("\n");
	}
	if (pre_str)
		outs(pre_str);
	outs("Messages in bounce queue: ");
	foo[fmt_ulong(foo, bCount)] = 0;
	outs(foo);
	outok("\n");
	if (doTodo) {
	if (pre_str)
		outs(pre_str);
	outs("Messages in todo   queue: ");
	foo[fmt_ulong(foo, tCount)] = 0;
	outs(foo);
	outok("\n");
	}
}

stralloc        line = { 0 };
char           *qbase;
stralloc        QueueBase = { 0 };

#ifdef MULTI_QUEUE
int
main_function(int *lcount, int *rcount, int *bcount, int *tcount)
#else
int
main(int argc, char **argv)
#endif
{
	int             channel, match, fd, x, flag;
	int             bCount, lCount, rCount, tCount;
	struct stat     st;
	substdio        ss;

#ifndef MULTI_QUEUE
	if (chdir(auto_sysconfdir))
		strerr_die4sys(111, FATAL, "unable to switch to ", auto_sysconfdir, ": ");
	if (!(qbase = env_get("QUEUE_BASE"))) {
		switch (control_readfile(&QueueBase, "queue_base", 0))
		{
		case -1:
			die_control();
			break;
		case 0:
			qbase = auto_qmail;
			break;
		case 1:
			qbase = QueueBase.s;
			break;
		}
	}
	if (chdir(qbase) == -1)
		die_chdir(qbase);
	if (!queuedir && !(queuedir = env_get("QUEUEDIR")))
		queuedir = "queue";
#endif
	if (chdir(queuedir) == -1)
		die_chdir(queuedir);
	getEnvConfigInt(&conf_split, "CONFSPLIT", auto_split);
	if (conf_split > auto_split)
		conf_split = auto_split;
	readsubdir_init(&rs, "info", die_opendir);
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
				die_nomem();
			if (fstat(fd, &st) == -1) {
				close(fd);
				err(id);
				continue;
			}
			close(fd);
			qtime = st.st_mtime;
			for (flag = 0,channel = (doLocal ? 0 : 1); channel < (doRemote ? 2 : 1); ++channel) {
				if ((fd = open_read(channel ? fnremote : fnlocal)) == -1) {
					if (errno != error_noent)
						err(id);
				} else {
					for (;;) {
						if (getln(&ss, &line, &match, 0) == -1)
							die_nomem();
						if (!match)
							break;
						switch (line.s[0])
						{
						case 'D':
							if (!flag++)
								putstats(doCount);
							outok("  done");
						case 'T':
							if (!flag++)
								putstats(doCount);
							if (channel)
								rCount++;
							else
								lCount++;
							outok(channel ? "\tremote\t" : "\tlocal\t");
							outs(line.s + 1);
							outok("\n");
							break;
						}
					}
					close(fd);
				}
			}
		}
	} /*- while (x = readsubdir_next(&rs, &id)) */
	if (!doTodo && !doCount) {
		substdio_flush(subfdout);
		return(0);
	}
	flagbounce = 0;
	readsubdir_init(&rs, "todo", die_opendir);
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
					die_nomem();
				if (!match || sender.s[0] == 'F')
					break;
			}
			if (fstat(fd, &st) == -1) {
				close(fd);
				err(id);
				continue;
			}
			close(fd);
			qtime = st.st_mtime;
			for (flag = 0,channel = 0; channel < 2; ++channel) {
				if ((fd = open_read(fnmess)) == -1) {
					if (errno != error_noent)
						err(id);
				} else {
					for (;;) {
						if (getln(&ss, &line, &match, 0) == -1)
							die_nomem();
						if (!match)
							break;
						switch (line.s[0]) {
						case 'D':
							if (!flag++)
								putstats(doCount);
							outok("  done");
						case 'T':
							if (!flag++)
								putstats(doCount);
							tCount++;
							outok("\ttodo\t");
							outs(line.s + 1);
							outok("\n");
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
#endif
	substdio_flush(subfdout);
	return(0);
}

#ifdef MULTI_QUEUE
int
main(int argc, char **argv)
{
	char           *queue_count_ptr, *queue_start_ptr;
	char            strnum[FMT_ULONG];
	int             idx, count, qcount, qstart, lcount, rcount, bcount, tcount;
	static stralloc Queuedir = { 0 };

	if (get_arguments(argc, argv)) {
		substdio_flush(subfdout);
		return(1);
	}
	set_environment(WARN, FATAL);
	if (!(qbase = env_get("QUEUE_BASE"))) {
		switch (control_readfile(&QueueBase, "queue_base", 0))
		{
		case -1:
			die_control();
			break;
		case 0:
			qbase = auto_qmail;
			break;
		case 1:
			qbase = QueueBase.s;
			break;
		}
	}
	if (!(queue_count_ptr = env_get("QUEUE_COUNT")))
		qcount = QUEUE_COUNT;
	else
		scan_int(queue_count_ptr, &qcount);
	if (!(queue_start_ptr = env_get("QUEUE_START")))
		qstart = 1;
	else
		scan_int(queue_start_ptr, &qstart);
	lcount = rcount = bcount = tcount = 0;
	for (idx = qstart, count=1; count <= qcount; count++, idx++) {
		if (!stralloc_copys(&Queuedir, qbase))
			die_nomem();
		if (!stralloc_cats(&Queuedir, "/queue"))
			die_nomem();
		if (!stralloc_catb(&Queuedir, strnum, fmt_ulong(strnum, (unsigned long) idx)))
			die_nomem();
		if (!stralloc_0(&Queuedir))
			die_nomem();
		if (access(Queuedir.s, F_OK))
			break;
		queuedir = Queuedir.s;
		outok("processing queue ");
		outok(queuedir);
		outok("\n");
		main_function(&lcount, &rcount, &bcount, &tcount);
		outok("\n");
	}
	if (!stralloc_copys(&Queuedir, qbase))
		die_nomem();
	if (!stralloc_cats(&Queuedir, "/nqueue"))
		die_nomem();
	if (!stralloc_0(&Queuedir))
		die_nomem();
	if (!access(Queuedir.s, F_OK)) {
		queuedir = Queuedir.s;
		outok("processing queue ");
		outok(queuedir);
		outok("\n");
		main_function(&lcount, &rcount, &bcount, &tcount);
		outok("\n");
	}
	if (doCount)
		putcounts("Total ", lcount, rcount, bcount, tcount);
	substdio_flush(subfdout);
	return(0);
}
#endif

void
getversion_qmail_qread_c()
{
	static char    *x = "$Id: qmail-qread.c,v 1.33 2021-05-16 00:46:51+05:30 Cprogrammer Exp mbhangui $";

	if (x)
		x++;
}
