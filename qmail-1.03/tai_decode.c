/*-
 * $Log: tai_decode.c,v $
 * Revision 1.1  2016-01-02 19:21:36+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "tai2.h"
#include "stralloc.h"
#include <ctype.h>

tai            *
tai_decode(stralloc *str, char **endptr)
{
	static tai      t;
	char           *ptr;

	t.seconds = 0;
	t.nanoseconds = 0;
	for (ptr = str->s; isdigit(*ptr);)
		t.seconds = (t.seconds * 10) + (*ptr++ - '0');
	if (*ptr == '.') {
		++ptr;
		while (isdigit(*ptr))
			t.nanoseconds = (t.nanoseconds * 10) + (*ptr++ - '0');
	}
	if (endptr)
		*endptr = ptr;
	return &t;
}

void
getversion_tai_decode_c()
{
	static char    *x = "$Id: tai_decode.c,v 1.1 2016-01-02 19:21:36+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
