/*
 * $Log: qmail-tcpok.c,v $
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
 * conditional compilation for INDIMAIL
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
#include <stdlib.h>
#include <unistd.h>
#include "strerr.h"
#include "scan.h"
#include "substdio.h"
#include "subfd.h"
#include "lock.h"
#include "open.h"
#include "auto_qmail.h"
#include "env.h"
#include "variables.h"
#include "stralloc.h"
#include "fmt.h"
#include "exit.h"
#include "tcpto.h"
#include "hasindimail.h"

#ifndef QUEUE_COUNT
#define QUEUE_COUNT 10
#endif

#define FATAL "qmail-tcpok: fatal: "

char            tcpto_buf[TCPTO_BUFSIZ];	/*- XXX: must match size in tcpto_clean.c, tcpto.c */
substdio        ss;

#ifdef INDIMAIL
int
main_function()
#else
int
main()
#endif
{
	int             fd, i;
	char           *qbase;

	if (!(qbase = env_get("QUEUE_BASE")))
		qbase = auto_qmail;
	if (chdir(qbase) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", qbase, ": ");
	if (chdir(queuedir) == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", qbase, "/queue: ");
	if (chdir("lock") == -1)
		strerr_die4sys(111, FATAL, "unable to chdir to ", qbase, "/queue/lock: ");
	if ((fd = open_write("tcpto")) == -1)
		strerr_die4sys(111, FATAL, "unable to write ", auto_qmail, "/queue/lock/tcpto: ");
	if (lock_ex(fd) == -1)
		strerr_die4sys(111, FATAL, "unable to lock ", auto_qmail, "/queue/lock/tcpto: ");
	substdio_fdbuf(&ss, write, fd, tcpto_buf, sizeof tcpto_buf);
	for (i = 0; i < sizeof(tcpto_buf); ++i)
		substdio_put(&ss, "", 1);
	if (substdio_flush(&ss) == -1)
		strerr_die4sys(111, FATAL, "unable to clear ", auto_qmail, "/queue/lock/tcpto: ");
	return(0);
}

#ifdef INDIMAIL
void
die(n)
	int             n;
{
	substdio_flush(subfderr);
	_exit(n);
}

void
die_nomem()
{
	substdio_puts(subfderr, "fatal: out of memory\n");
	die(111);
}

int
main(int argc, char **argv)
{
	char           *queue_count_ptr, *queue_start_ptr, *qbase;
	char            strnum[FMT_ULONG];
	int             idx, count, qcount, qstart;
	static stralloc Queuedir = { 0 };

	if(!(queue_count_ptr = env_get("QUEUE_COUNT")))
		qcount = QUEUE_COUNT;
	else
		scan_int(queue_count_ptr, &qcount);
	if(!(queue_start_ptr = env_get("QUEUE_START")))
		qstart = 1;
	else
		scan_int(queue_start_ptr, &qstart);
	if (!(qbase = env_get("QUEUE_BASE")))
		qbase = auto_qmail;
	for (idx = qstart, count=1; count <= qcount; count++, idx++)
	{
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
		main_function();
	}
	if (!stralloc_copys(&Queuedir, qbase))
		die_nomem();
	if (!stralloc_cats(&Queuedir, "/nqueue"))
		die_nomem();
	if (!stralloc_0(&Queuedir))
		die_nomem();
	if (!access(Queuedir.s, F_OK))
	{
		queuedir = Queuedir.s;
		main_function();
	}
	return(0);
}
#endif

void
getversion_qmail_tcpok_c()
{
	static char    *x = "$Id: qmail-tcpok.c,v 1.20 2009-09-08 13:33:12+05:30 Cprogrammer Stab mbhangui $";

#ifdef INDIMAIL
	x = sccsidh;
#else
	x++;
#endif
}
