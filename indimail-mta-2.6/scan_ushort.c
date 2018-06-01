/*
 * $Log: scan_ushort.c,v $
 * Revision 1.2  2004-10-22 20:30:08+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-01-02 23:51:30+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "scan.h"

unsigned int
scan_ushort(s, u)
	register char  *s;
	register unsigned short *u;
{
	register unsigned int pos;
	unsigned long   result;
	pos = scan_ulong(s, &result);
	*u = result;
	return pos;
}

void
getversion_scan_ushort_c()
{
	static char    *x = "$Id: scan_ushort.c,v 1.2 2004-10-22 20:30:08+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
