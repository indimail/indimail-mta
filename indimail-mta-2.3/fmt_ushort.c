/*
 * $Log: fmt_ushort.c,v $
 * Revision 1.2  2004-10-22 20:25:24+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-01-02 23:51:30+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "fmt.h"

unsigned int
fmt_ushort(s, u)
	register char  *s;
	register unsigned short u;
{
	register unsigned long l;
	l = u;
	return fmt_ulong(s, l);
}

void
getversion_fmt_ushort_c()
{
	static char    *x = "$Id: fmt_ushort.c,v 1.2 2004-10-22 20:25:24+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
