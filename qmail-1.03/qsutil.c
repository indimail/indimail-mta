/*
 * $Log: qsutil.c,v $
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

static stralloc foo = { 0 };

static char     errbuf[1];
static struct substdio sserr = SUBSTDIO_FDBUF(write, 0, errbuf, 1);

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
	substdio_putflush(&sserr, sa->s, sa->len);
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
log1(s1)
	char           *s1;
{
	substdio_putsflush(&sserr, s1);
}

void
my_log2(s1, s2)
	char           *s1;
	char           *s2;
{
	substdio_puts(&sserr, s1);
	substdio_putsflush(&sserr, s2);
}

void
log3(s1, s2, s3)
	char           *s1;
	char           *s2;
	char           *s3;
{
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
	substdio_putsflush(&sserr, s3);
}

void
log4(s1, s2, s3, s4)
	char           *s1;
	char           *s2;
	char           *s3;
	char           *s4;
{
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
	substdio_puts(&sserr, s3);
	substdio_putsflush(&sserr, s4);
}

void
log5(s1, s2, s3, s4, s5)
	char           *s1;
	char           *s2;
	char           *s3;
	char           *s4;
	char           *s5;
{
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
	substdio_puts(&sserr, s3);
	substdio_puts(&sserr, s4);
	substdio_putsflush(&sserr, s5);
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
	substdio_puts(&sserr, s1);
	substdio_puts(&sserr, s2);
	substdio_puts(&sserr, s3);
	substdio_puts(&sserr, s4);
	substdio_puts(&sserr, s5);
	substdio_puts(&sserr, s6);
	substdio_putsflush(&sserr, s7);
}

void
nomem()
{
	log1("alert: out of memory, sleeping...\n");
	sleep(10);
}

void
pausedir(dir)
	char           *dir;
{
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
	static char    *x = "$Id: qsutil.c,v 1.10 2013-09-23 22:14:42+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
