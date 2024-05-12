/*
 * $Log: gfrom.c,v $
 * Revision 1.5  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.4  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
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
gfrom(char *s, int len)
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
	const char     *x = "$Id: gfrom.c,v 1.5 2024-05-12 00:20:03+05:30 mbhangui Exp mbhangui $";

	x++;
}
