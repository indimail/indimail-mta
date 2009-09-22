/*
 * $Log: fmt_strn.c,v $
 * Revision 1.3  2004-10-22 20:25:17+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:18:53+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "fmt.h"

unsigned int
fmt_strn(s, t, n)
	register char  *s;
	register char  *t;
	register unsigned int n;
{
	register unsigned int len;
	char            ch;
	len = 0;
	if (s)
	{
		while (n-- && (ch = t[len]))
			s[len++] = ch;
	} else
		while (n-- && t[len])
			len++;
	return len;
}

void
getversion_fmt_strn_c()
{
	static char    *x = "$Id: fmt_strn.c,v 1.3 2004-10-22 20:25:17+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
