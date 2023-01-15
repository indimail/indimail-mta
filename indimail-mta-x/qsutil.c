/*
 * $Log: qsutil.c,v $
 * Revision 1.24  2023-01-15 23:22:49+05:30  Cprogrammer
 * use slog() function with varargs to replace all log functions
 *
 * Revision 1.23  2022-03-16 20:01:53+05:30  Cprogrammer
 * added log5_noflush() function
 *
 * Revision 1.22  2022-01-30 09:28:08+05:30  Cprogrammer
 * print program name in logs
 *
 * Revision 1.21  2021-10-22 14:00:10+05:30  Cprogrammer
 * added ident argument to loglock_open() for identification in logs
 *
 * Revision 1.20  2021-07-17 14:40:06+05:30  Cprogrammer
 * add fix_split function to generate file name for any split value
 *
 * Revision 1.19  2021-06-29 09:30:25+05:30  Cprogrammer
 * fixed handling of closed lock descriptor
 *
 * Revision 1.18  2021-06-27 11:33:38+05:30  Cprogrammer
 * added loglock_open function
 *
 * Revision 1.17  2021-06-23 10:03:55+05:30  Cprogrammer
 * added log_stat function
 *
 * Revision 1.16  2021-06-05 22:35:05+05:30  Cprogrammer
 * added log4_noflush() function
 *
 * Revision 1.15  2021-06-04 09:22:44+05:30  Cprogrammer
 * added log15() function
 *
 * Revision 1.14  2021-05-30 00:14:16+05:30  Cprogrammer
 * added log11() function
 *
 * Revision 1.13  2016-03-31 17:37:11+05:30  Cprogrammer
 * flush logs only when line gets completed
 * added log lock code to ensure log lines done get jumbled when running as a multi process delivery
 *
 * Revision 1.12  2016-01-29 18:27:48+05:30  Cprogrammer
 * removed log11() and added log13()
 *
 * Revision 1.11  2014-03-07 19:15:24+05:30  Cprogrammer
 * added log9(), log11()
 *
 * Revision 1.10  2013-09-23 22:14:42+05:30  Cprogrammer
 * added noflush log functions
 *
 * Revision 1.9  2010-06-27 09:04:38+05:30  Cprogrammer
 * added log7() function
 *
 * Revision 1.8  2009-05-03 22:46:46+05:30  Cprogrammer
 * added log5() function
 *
 * Revision 1.7  2004-12-20 22:57:46+05:30  Cprogrammer
 * changed log2() to my_log2() to avoid conflicts in fedora3
 *
 * Revision 1.6  2004-10-22 20:29:49+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.5  2004-10-22 15:38:32+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.4  2004-09-21 23:51:12+05:30  Cprogrammer
 * improved faster logic for issafe()
 *
 * Revision 1.3  2003-10-23 01:27:12+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.2  2003-10-17 21:05:42+05:30  Cprogrammer
 * added log4() function
 * optimized logging
 *
 */
#include <unistd.h>
#include <stralloc.h>
#include <fmt.h>
#include <str.h>
#include <substdio.h>
#include <lock.h>
#include <env.h>
#include <open.h>
#include <scan.h>
#include <error.h>
#include "auto_control.h"
#include "qsutil.h"
#include "varargs.h"
#include "variables.h"

static stralloc foo = { 0 };

static char     errbuf[1024];
static struct substdio sserr = SUBSTDIO_FDBUF(write, 0, errbuf, sizeof(errbuf));
extern char    *queuedesc; /*- defined in qmail-send.c */
#ifdef LOGLOCK
static stralloc lockfn = { 0 };
int             loglock_fd = -1;
#endif

#ifdef LOGLOCK
void
lockerr()
{
	if (queuedesc)
		slog(1, "alert: ", queuedesc, ": problem with lock/unlock, sleeping...\n", 0);
	else
		slog(1, "alert: problem with lock/unlock, sleeping...\n", 0);
	sleep(10);
}

void
loglock_open(char *ident, int preopen)
{
	char           *ptr;
	int             lock_status;

	if (!(ptr = env_get("LOGLOCK")))
		lock_status = 0;
	else
		scan_int(ptr, &lock_status);
	if (!lock_status && preopen)
		lock_status = preopen;
	if (lock_status > 0) {
		if (!(controldir = env_get("CONTROLDIR")))
			controldir = auto_control;
		if (!stralloc_copys(&lockfn, controldir)
				|| !stralloc_append(&lockfn, "/")
				|| !stralloc_catb(&lockfn, "/defaultdelivery", 16)
				|| !stralloc_0(&lockfn))
			nomem(ident);
		if ((loglock_fd = open_read(lockfn.s)) == -1) {
			if (queuedesc)
				slog(1, "alert: ", ident, ": ", queuedesc, ": cannot start: unable to open defaultdelivery\n", 0);
			else
				slog(1, "alert: ", ident, ": cannot start: unable to open defaultdelivery\n", 0);
			lockerr();
		}
	}
	/*- slog(1, "info: ", ident, ": ", queuedesc, loglock_fd == -1 ? ": loglock disabled\n" : ": loglock enabled\n", 0); -*/
}
#endif

int
fix_split(char *s, char *path, char *client_split, unsigned long id)
{
	int             i, len;
	
	len = 0;
	s += (i = fmt_str(s, path));
	len += i;
	s += (i = fmt_str(s, client_split));
	len += i;
	s += (i = fmt_str(s, "/"));
	len += i;
	s += (i = fmt_ulong(s, id));
	len += (i + 1);
	*s = 0;
	return len;
}

void
flush(void)
{
#ifdef LOGLOCK
	if (loglock_fd != -1) {
		if (lock_exnb(loglock_fd) == -1) {
			if (errno == error_ebadf)
				loglock_fd = -1;
			lockerr();
		}
	}
#endif
	substdio_flush(&sserr);
#ifdef LOGLOCK
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
logsa_noflush(stralloc *sa)
{
	substdio_put(&sserr, sa->s, sa->len);
}

void
logsa(stralloc *sa)
{
#ifdef LOGLOCK
	if (loglock_fd != -1) {
		if (lock_exnb(loglock_fd) == -1) {
			if (errno == error_ebadf)
				loglock_fd = -1;
			lockerr();
		}
	}
#endif
	substdio_putflush(&sserr, sa->s, sa->len);
#ifdef LOGLOCK
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
#ifdef  HAVE_STDARG_H
slog(int do_flush, ...)
#else
slog(va_alist)
va_dcl
#endif
{
	va_list         ap;
	char           *str;
#ifndef HAVE_STDARG_H
	int             do_flush;
#endif

#ifdef HAVE_STDARG_H
	va_start(ap, do_flush);
#else
	va_start(ap);
	do_flush = va_arg(ap, int);
#endif

#ifdef LOGLOCK
	if (loglock_fd != -1)
		lock_exnb(loglock_fd);
#endif
	while (1) {
		str = va_arg(ap, char *);
		if (!str)
			break;
		substdio_puts(&sserr, str);
	}
	va_end(ap);
	if (do_flush)
		substdio_flush(&sserr);
#ifdef LOGLOCK
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
log_stat(stralloc *mailfrom, stralloc *mailto, unsigned long id, size_t bytes)
{
	char           *ptr;
	char            strnum1[FMT_ULONG + 1], strnum2[FMT_ULONG + 1];

	strnum1[fmt_ulong(strnum1 + 1, id) + 1] = 0;
	strnum2[fmt_ulong(strnum2 + 1, bytes) + 1] = 0;
	*strnum1 = ' ';
	*strnum2 = ' ';
	for (ptr = mailto->s; ptr < mailto->s + mailto->len;) {
		if (queuedesc)
			slog(1, *ptr == 'L' ? "local: " : "remote: ", mailfrom->len > 3 ? mailfrom->s + 1 : "<>",
				" ", *(ptr + 2) ? ptr + 2 : "<>", strnum1, strnum2, " bytes ", queuedesc, "\n", 0);
		else
			slog(1, *ptr == 'L' ? "local: " : "remote: ", mailfrom->len > 3 ? mailfrom->s + 1 : "<>",
				" ", *(ptr + 2) ? ptr + 2 : "<>", strnum1, strnum2, " bytes\n", 0);
		ptr += str_len(ptr) + 1;
	}
	mailfrom->len = mailto->len = 0;
}

void
nomem(char *argv0)
{
	if (queuedesc)
		slog(1, "alert: ", argv0, ": ", queuedesc, ": out of memory, sleeping...\n", 0);
	else
		slog(1, "alert: ", argv0, ": out of memory, sleeping...\n", 0);
	sleep(10);
}

void
pausedir(char *dir)
{
	if (queuedesc)
		slog(1, "alert: ", queuedesc, ": unable to opendir ", dir, ", sleeping...\n", 0);
	else
		slog(1, "alert: unable to opendir ", dir, ", sleeping...\n", 0);
	sleep(10);
}

static int
issafe(char ch)
{
	return ( ((ch == '%') || (ch < '!') || (ch > '~')) ? 0 : 1 );
}

void
logsafe_noflush(char *s, char *argv0)
{
	int             i;

	while (!stralloc_copys(&foo, s))
		nomem(argv0);
	for (i = 0; i < foo.len; ++i)
		if (foo.s[i] == '\n')
			foo.s[i] = '/';
		else
		if (!issafe(foo.s[i]))
			foo.s[i] = '_';
	logsa_noflush(&foo);
}

void
logsafe(char *s, char *argv0)
{
	int             i;

	while (!stralloc_copys(&foo, s))
		nomem(argv0);
	for (i = 0; i < foo.len; ++i)
		if (foo.s[i] == '\n')
			foo.s[i] = '/';
		else
		if (!issafe(foo.s[i]))
			foo.s[i] = '_';
	logsa(&foo);
}

void
getversion_qsutil_c()
{
	static char    *x = "$Id: qsutil.c,v 1.24 2023-01-15 23:22:49+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
