/*
 * $Log: pidopen.c,v $
 * Revision 1.4  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2022-04-03 18:14:07+05:30  Cprogrammer
 * use 70 as return code for pidopen failure
 *
 * Revision 1.2  2021-06-15 21:50:34+05:30  Cprogrammer
 * added tmpdir argument
 *
 * Revision 1.1  2021-06-12 18:18:03+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <fmt.h>
#include <alloc.h>
#include <fcntl.h>
#include <open.h>
#include <datetime.h>
#include "pidopen.h"

char           *pidfn;
int             messfd;

unsigned int
pidfmt(char *s, unsigned long seq, datetime_sec _starttime, const char *tmpdir)
{
	unsigned int    i, len;
	pid_t           mypid;

	mypid = getpid();
	len = 0;
	i = fmt_str(s, tmpdir);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, "/qmail-pid.");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, mypid);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, _starttime);
	len += i;
	if (s)
		s += i;
	i = fmt_str(s, ".");
	len += i;
	if (s)
		s += i;
	i = fmt_ulong(s, seq);
	len += i;
	if (s)
		s += i;
	++len;
	if (s)
		*s++ = 0;

	return len;
}

int
pidopen(datetime_sec _starttime, const char *tmpdir)
{
	unsigned int    len;
	unsigned long   seq;

	seq = 1;
	len = pidfmt((char *) 0, seq, _starttime, tmpdir);
	if (!(pidfn = alloc(len)))
		return (51);
	for (seq = 1; seq < 10; ++seq) {
		if (pidfmt((char *) 0, seq, _starttime, tmpdir) > len)
			return(81);
		pidfmt(pidfn, seq, _starttime, tmpdir);
		if ((messfd = open_exclr(pidfn)) != -1)
			return 0;
	}
	return(70);
}

#ifndef lint
void
getversion_pidopen_c()
{
	const char     *x = "$Id: pidopen.c,v 1.4 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
#endif
