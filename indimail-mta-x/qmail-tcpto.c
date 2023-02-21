/*-
 * XXX: this program knows quite a bit about tcpto's internals
 */
#include <unistd.h>
#include <sys/socket.h>
#include <substdio.h>
#include <subfd.h>
#include <open.h>
#include <byte.h>
#include <scan.h>
#include <fmt.h>
#include <lock.h>
#include <error.h>
#include <strerr.h>
#include <datetime.h>
#include <now.h>
#ifdef MULTI_QUEUE
#include <stralloc.h>
#include <env.h>
#endif
#include "tcpto.h"
#include "variables.h"
#include "ip.h"
#include "auto_qmail.h"
#include "control.h"
#include "process_queue.h"

#define FATAL "qmail-tcpto: fatal: "
#define WARN  "qmail-tcpto: warn: "

#ifndef QUEUE_COUNT
#define QUEUE_COUNT 10
#endif


#ifdef MULTI_QUEUE
int
main_function(int *lcount, int *rcount, int *bcount, int *tcount)
#else
int
main()
#endif
{
	int             fdlock, fd, r, i, af;
	char           *record;
	char            tcpto_buf[TCPTO_BUFSIZ], tmp[FMT_ULONG + IPFMT];
	union v46addr   ip;
	datetime_sec    when, start;
#ifndef MULTI_QUEUE
	char           *qbase;
	static stralloc QueueBase = { 0 };
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
	if ((fdlock = open_write("tcpto")) == -1)
		strerr_die4sys(111, FATAL, "unable to open ", queuedir, "/lock/tcpto for write: ");
	if ((fd = open_read("tcpto")) == -1)
		strerr_die4sys(111, FATAL, "unable to open ", queuedir, "/lock/tcpto for read: ");
	if (lock_ex(fdlock) == -1)
		strerr_die4sys(111, FATAL, "unable to lock ", queuedir, "/lock/tcpto: ");
	if ((r = read(fd, tcpto_buf, sizeof(tcpto_buf))) == -1) {
		close(fd);
		close(fdlock);
		strerr_die4sys(111, FATAL, "unable to read ", queuedir, "/lock/tcpto: ");
	}
	close(fd);
	close(fdlock);
	r >>= 5;
	start = now();
	record = tcpto_buf;
	for (i = 0; i < r; ++i) {
		if (record[4] >= 1) {
			af = record[0];
			if (af == AF_INET)
				byte_copy((char *) &ip.ip, 4, record + 16);
#ifdef IPV6
			else
			if (af == AF_INET6)
				byte_copy((char *) &ip.ip6, 16, record + 16);
#endif
			else {
				record += 32;
				continue;
			}
			when = (unsigned long) (unsigned char ) record[11];
			when = (when << 8) + (unsigned long) (unsigned char ) record[10];
			when = (when << 8) + (unsigned long) (unsigned char ) record[9];
			when = (when << 8) + (unsigned long) (unsigned char ) record[8];
			if (af == AF_INET)
				substdio_put(subfderr, tmp, ip4_fmt(tmp, &ip.ip));
#ifdef IPV6
			else
			if (af == AF_INET6)
				substdio_put(subfderr, tmp, ip6_fmt(tmp, &ip.ip6));
#endif
			else {
				record += 32;
				continue;
			}
			if (af == AF_INET)
				substdio_puts(subfderr, " ipv4");
			else
				substdio_puts(subfderr, " ipv6");
			substdio_puts(subfderr, " timed out ");
			substdio_put(subfderr, tmp, fmt_ulong(tmp, (unsigned long) (start - when)));
			substdio_puts(subfderr, " seconds ago; # recent timeouts: ");
			substdio_put(subfderr, tmp, fmt_ulong(tmp, (unsigned long) (unsigned char ) record[4]));
			substdio_puts(subfderr, "\n");
		}
		record += 32;
	}
	substdio_flush(subfderr);
	substdio_flush(subfdout);
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
 * $Log: qmail-tcpto.c,v $
 * Revision 1.27  2021-06-28 17:07:25+05:30  Cprogrammer
 * use process_queue to process all queues
 *
 * Revision 1.26  2021-06-12 18:35:18+05:30  Cprogrammer
 * removed chdir(auto_qmail)
 *
 * Revision 1.25  2021-06-05 12:52:07+05:30  Cprogrammer
 * process special queue "slowq"
 *
 * Revision 1.24  2021-05-30 00:13:49+05:30  Cprogrammer
 * fixed qbase path
 *
 * Revision 1.23  2021-05-26 10:46:41+05:30  Cprogrammer
 * handle access() error other than ENOENT
 *
 * Revision 1.22  2020-11-24 13:47:38+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.21  2015-08-24 19:08:47+05:30  Cprogrammer
 * replace ip_fmt() with ip4_fmt()
 *
 * Revision 1.20  2010-02-10 08:59:23+05:30  Cprogrammer
 * use -DMULTI_QUEUE for using multiple queues
 *
 * Revision 1.19  2009-11-09 19:55:25+05:30  Cprogrammer
 * Use control file queue_base to process multiple indimail queues
 *
 * Revision 1.18  2009-09-08 13:33:28+05:30  Cprogrammer
 * define default value for QUEUE_COUNT
 *
 * Revision 1.17  2005-06-29 20:54:35+05:30  Cprogrammer
 * size of buffer changed to TCPTO_BUFSIZ
 *
 * Revision 1.16  2005-06-17 21:50:24+05:30  Cprogrammer
 * ipv6 support
 *
 * Revision 1.15  2005-04-02 12:02:20+05:30  Cprogrammer
 * minor change
 *
 * Revision 1.14  2005-03-03 14:37:41+05:30  Cprogrammer
 * assign queuedir after stralloc operation
 *
 * Revision 1.13  2004-10-22 20:29:41+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.12  2004-10-09 23:39:15+05:30  Cprogrammer
 * removed compiler warnings
 *
 * Revision 1.11  2004-10-09 00:27:59+05:30  Cprogrammer
 * use stralloc variable for queuedir
 *
 * Revision 1.10  2004-09-22 23:13:52+05:30  Cprogrammer
 * replaced atoi() with scan_int()
 *
 * Revision 1.9  2004-05-06 22:29:25+05:30  Cprogrammer
 * use QUEUE_BASE instead of auto_qmail
 *
 * Revision 1.8  2004-05-03 22:16:09+05:30  Cprogrammer
 * use QUEUE_BASE instead of auto_qmail
 *
 * Revision 1.7  2003-12-07 23:11:35+05:30  Cprogrammer
 * added notification queue
 *
 * Revision 1.6  2003-10-28 20:02:28+05:30  Cprogrammer
 * included unistd.h
 *
 * Revision 1.5  2003-10-23 01:26:47+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.4  2003-10-01 19:06:00+05:30  Cprogrammer
 * changed return type to int
 *
 * Revision 1.3  2002-12-05 14:12:57+05:30  Cprogrammer
 * added code to process all queues
 */

void
getversion_qmail_tcpto_c()
{
	static char    *x = "$Id: qmail-tcpto.c,v 1.27 2021-06-28 17:07:25+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
