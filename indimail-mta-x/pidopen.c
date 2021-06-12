/*
 * $Log: pidopen.c,v $
 * Revision 1.1  2021-06-12 18:18:03+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <sys/types.h>
#include <env.h>
#include <fmt.h>
#include <alloc.h>
#include <open.h>
#include <datetime.h>
#include "pidopen.h"

char           *pidfn;
int             messfd;

unsigned int
pidfmt(char *s, unsigned long seq, datetime_sec _starttime)
{
	unsigned int    i, len;
	pid_t           mypid;
	char           *tmpdir;

	mypid = getpid();
	if (!(tmpdir = env_get("TMPDIR")))
		tmpdir = "/tmp";
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
pidopen(datetime_sec _starttime)
{
	unsigned int    len;
	unsigned long   seq;

	seq = 1;
	len = pidfmt((char *) 0, seq, _starttime);
	if (!(pidfn = alloc(len)))
		return (51);
	for (seq = 1; seq < 10; ++seq) {
		if (pidfmt((char *) 0, seq, _starttime) > len)
			return(81);
		pidfmt(pidfn, seq, _starttime);
		if ((messfd = open_excl(pidfn)) != -1)
			return 0;
	}
	return(63);
}

#ifndef lint
void
getversion_pidopen_c()
{
	static char    *x = "$Id: pidopen.c,v 1.1 2021-06-12 18:18:03+05:30 Cprogrammer Exp mbhangui $";

	x = sccsidpidopenh;
	x++;
}
#endif
