/*
 * $Log: rewind.c,v $
 * Revision 1.4  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2004-10-22 20:29:59+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-06-03 22:43:13+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.1  2004-05-14 00:45:05+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "seek.h"

int
main(void)
{
	return(seek_begin(0));
}

void
getversion_rewind_c()
{
	const char     *x = "$Id: rewind.c,v 1.4 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
