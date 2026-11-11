/*
 * $Log: fmt_xpid.c,v $
 * Revision 1.5  2026-11-11 19:26:59+05:30  Cprogrammer
 * Fix warnings from -Wextra
 *
 * Revision 1.4  2026-11-10 16:01:17+05:30  mbhangui
 * fix set but unused variable warnings
 *
 * Revision 1.3  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2004-10-22 20:25:26+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-09-19 18:54:30+05:30  Cprogrammer
 * Initial revision
 *
 */

/*- Public domain.  */
#include "fmt_xpid.h"

static char     hex[] = "0123456789abcdef";
unsigned int
fmt_xpid(register char *s, register pid_t u, const unsigned int n)
{
	register unsigned int len;
	register unsigned long q;

	len = 1;
	q = u;
	while (q > 15)
	{
		++len;
		q /= 16;
	}
	if (len > n)
		return 0;
	if (s)
	{
		s += len;
		do
		{
			*--s = hex[u % 16];
			u /= 16;
		}
		while (u);	/*- handles u == 0 */
	} else
		return 0;
	return len;
}

void
getversion_fmt_xpid_c()
{
	const char     *x = "$Id: fmt_xpid.c,v 1.5 2026-11-11 19:26:59+05:30 Cprogrammer Exp mbhangui $";

	(void) x;
}
