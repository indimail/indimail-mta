/*
 * $Log: qsutil.c,v $
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
		log3("alert: ", queuedesc, ": problem with lock/unlock, sleeping...\n");
	else
		log1("alert: problem with lock/unlock, sleeping...\n");
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
			nomem();
		if ((loglock_fd = open_read(lockfn.s)) == -1) {
			if (queuedesc)
				log5("alert: ", ident, ": ", queuedesc, ": cannot start: unable to open defaultdelivery\n");
			else
				log3("alert: ", ident, ": cannot start: unable to open defaultdelivery\n");
			lockerr();
		}
	}
	/*- log5("info: ", ident, ": ", queuedesc, loglock_fd == -1 ? ": loglock disabled\n" : ": loglock enabled\n"); -*/
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
log1_noflush(char *s1)
{
	substdio_puts(&sserr, s1);
}

void
log2_noflush(char *s1, char *s2)
{
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
}

void
log3_noflush(char *s1, char *s2, char *s3)
{
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
	substdio_puts(&sserr, s3);
}

void
log4_noflush(char *s1, char *s2, char *s3, char *s4)
{
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
	substdio_puts(&sserr, s3);
	substdio_puts(&sserr, s4);
}

void
log1(char *s1)
{
#ifdef LOGLOCK
	if (loglock_fd != -1)
		lock_exnb(loglock_fd);
#endif
	substdio_putsflush(&sserr, s1);
#ifdef LOGLOCK
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
log3(char *s1, char *s2, char *s3)
{
#ifdef LOGLOCK
	if (loglock_fd != -1)
		lock_exnb(loglock_fd);
#endif
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
	substdio_puts(&sserr, s3);
	substdio_flush(&sserr);
#ifdef LOGLOCK
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
log4(char *s1, char *s2, char *s3, char *s4)
{
#ifdef LOGLOCK
	if (loglock_fd != -1)
		lock_exnb(loglock_fd);
#endif
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
	substdio_puts(&sserr, s3);
	substdio_puts(&sserr, s4);
	substdio_flush(&sserr);
#ifdef LOGLOCK
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
log5(char *s1, char *s2, char *s3, char *s4, char *s5)
{
#ifdef LOGLOCK
	if (loglock_fd != -1)
		lock_exnb(loglock_fd);
#endif
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
	substdio_puts(&sserr, s3);
	substdio_puts(&sserr, s4);
	substdio_puts(&sserr, s5);
	substdio_flush(&sserr);
#ifdef LOGLOCK
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
log7(char *s1, char *s2, char *s3, char *s4, char *s5, char *s6, char *s7)
{
#ifdef LOGLOCK
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
#ifdef LOGLOCK
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
log9(char *s1, char *s2, char *s3, char *s4, char *s5, char *s6, char *s7,
		char *s8, char *s9)
{
#ifdef LOGLOCK
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
#ifdef LOGLOCK
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
log11(char *s1, char *s2, char *s3, char *s4, char *s5, char *s6, char *s7,
		char *s8, char *s9, char *s10, char *s11)
{
#ifdef LOGLOCK
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
	substdio_flush(&sserr);
#ifdef LOGLOCK
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
log13(char *s1, char *s2, char *s3, char *s4, char *s5, char *s6, char *s7,
		char *s8, char *s9, char *s10, char *s11, char *s12, char *s13)
{
#ifdef LOGLOCK
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
#ifdef LOGLOCK
	if (loglock_fd != -1 && lock_un(loglock_fd) == -1)
		lockerr();
#endif
}

void
log15(char *s1, char *s2, char *s3, char *s4, char *s5, char *s6, char *s7,
		char *s8, char *s9, char *s10, char *s11, char *s12, char *s13,
		char *s14, char *s15)
{
#ifdef LOGLOCK
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
	substdio_puts(&sserr, s14);
	substdio_puts(&sserr, s15);
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
			log9(*ptr == 'L' ? "local: " : "remote: ", mailfrom->len > 3 ? mailfrom->s + 1 : "<>",
				" ", *(ptr + 2) ? ptr + 2 : "<>", strnum1, strnum2, " bytes ", queuedesc, "\n");
		else
			log7(*ptr == 'L' ? "local: " : "remote: ", mailfrom->len > 3 ? mailfrom->s + 1 : "<>",
				" ", *(ptr + 2) ? ptr + 2 : "<>", strnum1, strnum2, " bytes\n");
		ptr += str_len(ptr) + 1;
	}
	mailfrom->len = mailto->len = 0;
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

void
pausedir(char *dir)
{
	if (queuedesc)
		log5("alert: ", queuedesc, ": unable to opendir ", dir, ", sleeping...\n");
	else
		log3("alert: unable to opendir ", dir, ", sleeping...\n");
	sleep(10);
}

static int
issafe(char ch)
{
	return ( ((ch == '%') || (ch < '!') || (ch > '~')) ? 0 : 1 );
}

void
logsafe_noflush(char *s)
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
logsafe(char *s)
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
	static char    *x = "$Id: qsutil.c,v 1.21 2021-10-22 14:00:10+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
