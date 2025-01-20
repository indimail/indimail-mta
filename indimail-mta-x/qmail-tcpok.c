#include <unistd.h>
#include <strerr.h>
#include <scan.h>
#include <substdio.h>
#include <subfd.h>
#include <lock.h>
#include <open.h>
#include <env.h>
#include <error.h>
#include <stralloc.h>
#include <fmt.h>
#include "auto_qmail.h"
#include "control.h"
#include "tcpto.h"
#include "variables.h"
#include "process_queue.h"

#ifndef QUEUE_COUNT
#define QUEUE_COUNT 10
#endif

#define FATAL "qmail-tcpok: fatal: "
#define WARN  "qmail-tcpok: warn: "

#ifdef MULTI_QUEUE
int
main_function(int *lcount, int *rcount, int *bcount, int *tcount)
#else
int
main()
#endif
{
	int             fd, i;
	char            tcpto_buf[TCPTO_BUFSIZ];	/*- XXX: must match size in tcpto_clean.c, tcpto.c */
	substdio        ss;
#ifndef MULTI_QUEUE
	char           *qbase;
	stralloc        QueueBase = { 0 };
#endif

#ifndef MULTI_QUEUE
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
	if (chdir("lock") == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", queuedir, "/lock: ");
	if ((fd = open_write("tcpto")) == -1)
		strerr_die4sys(111, FATAL, "unable to open ", queuedir, "/lock/tcpto: ");
	if (lock_ex(fd) == -1)
		strerr_die4sys(111, FATAL, "unable to lock ", queuedir, "/lock/tcpto: ");
	substdio_fdbuf(&ss, (ssize_t (*)(int,  char *, size_t)) write, fd, tcpto_buf, sizeof tcpto_buf);
	for (i = 0; i < sizeof(tcpto_buf); ++i)
		substdio_put(&ss, "", 1);
	if (substdio_flush(&ss) == -1)
		strerr_die4sys(111, FATAL, "unable to clear ", queuedir, "/lock/tcpto: ");
#ifdef MULTI_QUEUE
	return (0);
#else
	_exit (0);
#endif
}

#ifdef MULTI_QUEUE
int
main(int argc, char **argv)
{
	process_queue(WARN, FATAL, main_function, 0, 0, 0, 0);
	return 0;
}
#endif

/*
 * $Log: qmail-tcpok.c,v $
 * Revision 1.30  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.29  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.28  2021-06-28 17:07:29+05:30  Cprogrammer
 * use process_queue to process all queues
 *
 * Revision 1.27  2021-06-12 18:35:07+05:30  Cprogrammer
 * removed chdir(auto_qmail)
 *
 * Revision 1.26  2021-06-05 12:52:03+05:30  Cprogrammer
 * process special queue "slowq"
 *
 * Revision 1.25  2021-05-30 00:13:42+05:30  Cprogrammer
 * fixed qbase path
 *
 * Revision 1.24  2021-05-26 10:46:25+05:30  Cprogrammer
 * handle access() error other than ENOENT
 *
 * Revision 1.23  2020-11-24 13:47:35+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.22  2010-02-10 08:59:10+05:30  Cprogrammer
 * use -DMULTI_QUEUE for using multiple queues
 *
 * Revision 1.21  2009-11-09 19:55:18+05:30  Cprogrammer
 * Use control file queue_base to process multiple indimail queues
 *
 * Revision 1.20  2009-09-08 13:33:12+05:30  Cprogrammer
 * define default value for QUEUE_COUNT
 *
 * Revision 1.19  2007-12-20 13:50:53+05:30  Cprogrammer
 * removed compiler warning
 *
 * Revision 1.18  2005-06-29 20:54:23+05:30  Cprogrammer
 * size of buffer changed to TCPTO_BUFSIZ
 *
 * Revision 1.17  2005-06-17 21:50:08+05:30  Cprogrammer
 * increased size of tcpto buffer
 *
 * Revision 1.16  2005-04-02 09:27:39+05:30  Cprogrammer
 * BUG - for multiple queues only the first queue was being processed
 *
 * Revision 1.15  2005-03-03 14:37:38+05:30  Cprogrammer
 * assign queuedir after stralloc operation
 *
 * Revision 1.14  2004-10-22 20:29:40+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.13  2004-10-22 15:38:12+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.12  2004-10-09 00:27:32+05:30  Cprogrammer
 * use stralloc variable for queuedir
 *
 * Revision 1.11  2004-09-22 23:13:48+05:30  Cprogrammer
 * replaced atoi() with scan_int()
 *
 * Revision 1.10  2004-05-07 10:16:52+05:30  Cprogrammer
 * use QUEUE_BASE instead of auto_qmail
 *
 * Revision 1.9  2004-05-03 22:15:57+05:30  Cprogrammer
 * use QUEUE_BASE instead of auto_qmail
 *
 * Revision 1.8  2003-12-07 23:11:30+05:30  Cprogrammer
 * added notification queue
 *
 * Revision 1.7  2003-10-28 20:02:18+05:30  Cprogrammer
 * conditional compilation for indimail
 *
 * Revision 1.6  2003-10-23 01:26:41+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.5  2003-10-01 19:05:57+05:30  Cprogrammer
 * changed return type to int
 *
 * Revision 1.4  2002-12-05 14:12:47+05:30  Cprogrammer
 * added code to process all queues
 *
 */

void
getversion_qmail_tcpok_c()
{
	const char     *x = "$Id: qmail-tcpok.c,v 1.30 2024-05-12 00:20:03+05:30 mbhangui Exp mbhangui $";

	x++;
}
