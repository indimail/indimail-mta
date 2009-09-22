/*
 * $Log: envread.c,v $
 * Revision 1.5  2008-08-03 18:25:43+05:30  Cprogrammer
 * use proper proto
 *
 * Revision 1.4  2004-10-22 20:24:49+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.3  2004-07-17 21:18:32+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "env.h"
#include "str.h"

char       *
env_get(s)
	char           *s;
{
	int             i;
	unsigned int    slen;
	char           *envi;

	slen = str_len(s);
	for (i = 0; (envi = environ[i]); ++i)
	{
		if ((!str_diffn(s, envi, slen)) && (envi[slen] == '='))
			return envi + slen + 1;
	}
	return 0;
}

char    *
env_pick()
{
	return environ[0];
}

char    *
env_findeq(s)
	char           *s;
{
	for (; *s; ++s)
		if (*s == '=')
			return s;
	return 0;
}

void
getversion_envread_c()
{
	static char    *x = "$Id: envread.c,v 1.5 2008-08-03 18:25:43+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
