/*
 * $Log: fmt_str.c,v $
 * Revision 1.5  2004-10-22 20:32:37+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-07-17 21:18:52+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "fmt.h"

unsigned int
fmt_str(s, t)
	register char  *s;
	register char  *t;
{
	register unsigned int len;
	char            ch;

	len = 0;
	if (s)
	{
		while ((ch = t[len]))
			s[len++] = ch;
	} else
	{
		while (t[len])
			len++;
	}
	return len;
}

void
getversion_fmt_str_c()
{
	static char    *x = "$Id: fmt_str.c,v 1.5 2004-10-22 20:32:37+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
