/*
 * $Log: fmtqfn.c,v $
 * Revision 1.4  2021-05-16 00:42:11+05:30  Cprogrammer
 * use configurable conf_split instead of auto_split variable
 *
 * Revision 1.3  2004-10-22 20:25:07+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:18:50+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "fmtqfn.h"
#include "fmt.h"

extern int      conf_split;

unsigned int
fmtqfn(char *s, char *dirslash, unsigned long id, int flagsplit)
{
	unsigned int    len;
	unsigned int    i;

	len = 0;
	i = fmt_str(s, dirslash);
	len += i;
	if (s)
		s += i;
	if (flagsplit) {
		i = fmt_ulong(s, id % conf_split);
		len += i;
		if (s)
			s += i;
		i = fmt_str(s, "/");
		len += i;
		if (s)
			s += i;
	}
	i = fmt_ulong(s, id);
	len += i;
	if (s)
		s += i;
	if (s)
		*s++ = 0;
	++len;
	return len;
}

void
getversion_fmtqfn_c()
{
	static char    *x = "$Id: fmtqfn.c,v 1.4 2021-05-16 00:42:11+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
