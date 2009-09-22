/*
 * $Log: gfrom.c,v $
 * Revision 1.3  2004-10-22 20:25:40+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:19:04+05:30  Cprogrammer
 * added RCS log
 *
 */
#include "str.h"
#include "gfrom.h"

int
gfrom(s, len)
	char           *s;
	int             len;
{
	while ((len > 0) && (*s == '>'))
	{
		++s;
		--len;
	}
	return (len >= 5) && !str_diffn(s, "From ", 5);
}

void
getversion_gfrom_c()
{
	static char    *x = "$Id: gfrom.c,v 1.3 2004-10-22 20:25:40+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
