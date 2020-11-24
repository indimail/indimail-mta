/*
 * $Log: qmail-tcpto.c,v $
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
 *
 *
 * XXX: this program knows quite a bit about tcpto's internals 
 */
#include <stdlib.h>
#include <sys/socket.h>
#include "substdio.h"
#include "subfd.h"
#include "open.h"
#include "byte.h"
#include "scan.h"
#include "subfd.h"
#include "auto_qmail.h"
#include "control.h"
#include "fmt.h"
#include "ip.h"
#include "lock.h"
#include "error.h"
#include "datetime.h"
#include "env.h"
#include "now.h"
#include "stralloc.h"
#include "tcpto.h"
#include "variables.h"
#include <unistd.h>

#ifndef QUEUE_COUNT
#define QUEUE_COUNT 10
#endif

int
die(n)
	int             n;
{
	substdio_flush(subfderr);
	substdio_flush(subfdout);
#ifdef MULTI_QUEUE
	if (n)
		_exit(n);
	else
		return(0);
#else
	_exit(n);
#endif
}

void
warn(s1, s2)
	char           *s1, *s2;
{
	char           *x;

	x = error_str(errno);
	substdio_puts(subfderr, s1);
	if (s2)
		substdio_puts(subfderr, s2);
	substdio_puts(subfderr, ": ");
	substdio_puts(subfderr, x);
	substdio_puts(subfderr, "\n");
}

void
die_chdir(char *dir)
{
	warn("fatal: unable to chdir to ", dir);
	die(111);
}

void
die_open()
{
	warn("fatal: unable to open tcpto", 0);
	die(111);
}

void
die_lock()
{
	warn("fatal: unable to lock tcpto", 0);
	die(111);
}

void
die_read()
{
	warn("fatal: unable to read tcpto", 0);
	die(111);
}

void
die_home()
{
	substdio_puts(subfderr, "Unable to switch to home directory.\n");
	die(111);
}

void
die_control()
{
	substdio_puts(subfderr, "fatal: unable to read controls\n");
	die(111);
}

char            tcpto_buf[TCPTO_BUFSIZ];
char            tmp[FMT_ULONG + IPFMT];
char           *qbase;
stralloc        QueueBase = { 0 };

#ifdef MULTI_QUEUE
int
main_function()
#else
int
main()
#endif
{
	int             fdlock, fd, r, i, af;
	char           *record;
	union v46addr   ip;
	datetime_sec    when, start;

#ifndef MULTI_QUEUE
	if (chdir(auto_qmail))
		die_home();
	if (!(qbase = env_get("QUEUE_BASE")))
	{
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
	if (chdir("lock") == -1)
		die_chdir("lock");
	if ((fdlock = open_write("tcpto")) == -1)
		die_open();
	if ((fd = open_read("tcpto")) == -1)
		die_open();
	if (lock_ex(fdlock) == -1)
		die_lock();
	r = read(fd, tcpto_buf, sizeof(tcpto_buf));
	close(fd);
	close(fdlock);
	if (r == -1)
		die_read();
	r >>= 5;
	start = now();
	record = tcpto_buf;
	for (i = 0; i < r; ++i)
	{
		if (record[4] >= 1)
		{
			af = record[0];
			if (af == AF_INET)
				byte_copy((char *) &ip.ip, 4, record + 16);
#ifdef IPV6
			else
			if (af == AF_INET6)
				byte_copy((char *) &ip.ip6, 16, record + 16);
#endif
			else
			{
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
			else
			{
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
	die(0);
	/*- Not reached */
	return(0);
}

#ifdef MULTI_QUEUE
void
die_nomem()
{
	substdio_puts(subfderr, "fatal: out of memory\n");
	die(111);
}

void
outok(s)
	char           *s;
{
	substdio_puts(subfdout, s);
}

int
main(int argc, char **argv)
{
	char           *queue_count_ptr, *queue_start_ptr;
	char            strnum[FMT_ULONG];
	int             idx, count, qcount, qstart;
	static stralloc Queuedir = { 0 };

	if (chdir(auto_qmail))
		die_home();
	if (!(qbase = env_get("QUEUE_BASE")))
	{
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
		outok("processing queue ");
		outok(queuedir);
		outok("\n");
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
		outok("processing queue ");
		outok(queuedir);
		outok("\n");
		main_function();
	}
	die(0);
	/*- Not reached */
	return(0);
}
#endif

void
getversion_qmail_tcpto_c()
{
	static char    *x = "$Id: qmail-tcpto.c,v 1.22 2020-11-24 13:47:38+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
