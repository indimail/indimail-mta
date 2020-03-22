/*
 * $Log: qsutil.c,v $
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
#include "stralloc.h"
#include "substdio.h"
#include "qsutil.h"
#include "lock.h"

static stralloc foo = { 0 };

static char     errbuf[1024];
static struct substdio sserr = SUBSTDIO_FDBUF(write, 0, errbuf, sizeof(errbuf));
extern char    *queuedesc; /*- defined in qmail-send.c */
#ifdef LOCK_LOGS
extern int      loglock_fd;
void            lockerr();
#endif

void
flush(void)
{
#ifdef LOCK_LOGS
	if (loglock_fd != -1)
		lock_exnb(loglock_fd);
#endif
	substdio_flush(&sserr);
#ifdef LOCK_LOGS
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
logsa_noflush(sa)
	stralloc       *sa;
{
	substdio_put(&sserr, sa->s, sa->len);
}

void
logsa(sa)
	stralloc       *sa;
{
#ifdef LOCK_LOGS
	if (loglock_fd != -1)
		lock_exnb(loglock_fd);
#endif
	substdio_putflush(&sserr, sa->s, sa->len);
#ifdef LOCK_LOGS
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
log1_noflush(s1)
	char           *s1;
{
	substdio_puts(&sserr, s1);
}

void
log2_noflush(s1, s2)
	char           *s1;
	char           *s2;
{
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
}

void
log3_noflush(s1, s2, s3)
	char           *s1;
	char           *s2;
	char           *s3;
{
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
	substdio_puts(&sserr, s3);
}

void
log1(s1)
	char           *s1;
{
#ifdef LOCK_LOGS
	if (loglock_fd != -1)
		lock_exnb(loglock_fd);
#endif
	substdio_putsflush(&sserr, s1);
#ifdef LOCK_LOGS
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
log3(s1, s2, s3)
	char           *s1;
	char           *s2;
	char           *s3;
{
#ifdef LOCK_LOGS
	if (loglock_fd != -1)
		lock_exnb(loglock_fd);
#endif
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
	substdio_puts(&sserr, s3);
	substdio_flush(&sserr);
#ifdef LOCK_LOGS
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
log4(s1, s2, s3, s4)
	char           *s1;
	char           *s2;
	char           *s3;
	char           *s4;
{
#ifdef LOCK_LOGS
	if (loglock_fd != -1)
		lock_exnb(loglock_fd);
#endif
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
	substdio_puts(&sserr, s3);
	substdio_puts(&sserr, s4);
	substdio_flush(&sserr);
#ifdef LOCK_LOGS
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
log5(s1, s2, s3, s4, s5)
	char           *s1;
	char           *s2;
	char           *s3;
	char           *s4;
	char           *s5;
{
#ifdef LOCK_LOGS
	if (loglock_fd != -1)
		lock_exnb(loglock_fd);
#endif
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
	substdio_puts(&sserr, s3);
	substdio_puts(&sserr, s4);
	substdio_puts(&sserr, s5);
	substdio_flush(&sserr);
#ifdef LOCK_LOGS
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
log7(s1, s2, s3, s4, s5, s6, s7)
	char           *s1;
	char           *s2;
	char           *s3;
	char           *s4;
	char           *s5;
	char           *s6;
	char           *s7;
{
#ifdef LOCK_LOGS
	if (loglock_fd != -1)
		lock_exnb(loglock_fd);
#endif
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
	substdio_puts(&sserr, s3);
	substdio_puts(&sserr, s4);
	substdio_puts(&sserr, s5);
	substdio_puts(&sserr, s6);
	substdio_puts(&sserr, s7);
	substdio_flush(&sserr);
#ifdef LOCK_LOGS
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
log9(s1, s2, s3, s4, s5, s6, s7, s8, s9)
	char           *s1;
	char           *s2;
	char           *s3;
	char           *s4;
	char           *s5;
	char           *s6;
	char           *s7;
	char           *s8;
	char           *s9;
{
#ifdef LOCK_LOGS
	if (loglock_fd != -1)
		lock_exnb(loglock_fd);
#endif
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
	substdio_puts(&sserr, s3);
	substdio_puts(&sserr, s4);
	substdio_puts(&sserr, s5);
	substdio_puts(&sserr, s6);
	substdio_puts(&sserr, s7);
	substdio_puts(&sserr, s8);
	substdio_puts(&sserr, s9);
	substdio_flush(&sserr);
#ifdef LOCK_LOGS
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
log13(s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11, s12, s13)
	char           *s1;
	char           *s2;
	char           *s3;
	char           *s4;
	char           *s5;
	char           *s6;
	char           *s7;
	char           *s8;
	char           *s9;
	char           *s10;
	char           *s11;
	char           *s12;
	char           *s13;
{
#ifdef LOCK_LOGS
	if (loglock_fd != -1)
		lock_exnb(loglock_fd);
#endif
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
	substdio_puts(&sserr, s3);
	substdio_puts(&sserr, s4);
	substdio_puts(&sserr, s5);
	substdio_puts(&sserr, s6);
	substdio_puts(&sserr, s7);
	substdio_puts(&sserr, s8);
	substdio_puts(&sserr, s9);
	substdio_puts(&sserr, s10);
	substdio_puts(&sserr, s11);
	substdio_puts(&sserr, s12);
	substdio_puts(&sserr, s13);
	substdio_flush(&sserr);
#ifdef LOCK_LOGS
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
nomem()
{
	if (queuedesc)
		log3("alert: ", queuedesc, ": out of memory, sleeping...\n");
	else
		log1("alert: out of memory, sleeping...\n");
	sleep(10);
}

#ifdef LOCK_LOGS
void
lockerr()
{
	if (queuedesc)
		log3("alert: ", queuedesc, ": problem with lock/unlock, sleeping...\n");
	else
		log1("alert: problem with lock/unlock, sleeping...\n");
	sleep(10);
}
#endif

void
pausedir(dir)
	char           *dir;
{
	if (queuedesc)
		log5("alert: ", queuedesc, ": unable to opendir ", dir, ", sleeping...\n");
	else
		log3("alert: unable to opendir ", dir, ", sleeping...\n");
	sleep(10);
}

static int
issafe(ch)
	char            ch;
{
	return ( ((ch == '%') || (ch < '!') || (ch > '~')) ? 0 : 1 );
}

void
logsafe_noflush(s)
	char           *s;
{
	int             i;
	while (!stralloc_copys(&foo, s))
		nomem();
	for (i = 0; i < foo.len; ++i)
		if (foo.s[i] == '\n')
			foo.s[i] = '/';
		else
		if (!issafe(foo.s[i]))
			foo.s[i] = '_';
	logsa_noflush(&foo);
}

void
logsafe(s)
	char           *s;
{
	int             i;
	while (!stralloc_copys(&foo, s))
		nomem();
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
	static char    *x = "$Id: qsutil.c,v 1.13 2016-03-31 17:37:11+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
