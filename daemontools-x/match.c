/*
 * $Log: match.c,v $
 * Revision 1.3  2024-05-09 22:39:36+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2004-10-22 20:27:16+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 19:31:14+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "match.h"

int
match(char *pattern, char *buf, unsigned int len)
{
	char            ch;

	for (;;)
	{
		ch = *pattern++;
		if (!ch)
			return !len;
		if (ch == '*')
		{
			ch = *pattern;
			if (!ch)
				return 1;
			for (;;)
			{
				if (!len)
					return 0;
				if (*buf == ch)
					break;
				++buf;
				--len;
			}
			continue;
		}
		if (!len)
			return 0;
		if (*buf != ch)
			return 0;
		++buf;
		--len;
	}
}

void
getversion_match_c()
{
	const char     *x = "$Id: match.c,v 1.3 2024-05-09 22:39:36+05:30 mbhangui Exp mbhangui $";

	x++;
}
