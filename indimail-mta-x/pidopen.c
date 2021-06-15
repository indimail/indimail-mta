/*
 * $Log: pidopen.c,v $
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
pidfmt(char *s, unsigned long seq, datetime_sec _starttime, char *tmpdir)
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
pidopen(datetime_sec _starttime, char *tmpdir)
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
	return(63);
}

#ifndef lint
void
getversion_pidopen_c()
{
	static char    *x = "$Id: pidopen.c,v 1.2 2021-06-15 21:50:34+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidpidopenh;
	x++;
}
#endif
