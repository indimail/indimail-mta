/*
 * $Log: b64decode.c,v $
 * Revision 1.6  2005-08-23 17:14:35+05:30  Cprogrammer
 * gcc 4 compliance
 *
 * Revision 1.5  2004-10-22 20:18:12+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-07-17 21:16:04+05:30  Cprogrammer
 * added RCS log
 *
 */
#include <stdio.h>
#include <string.h>
#include "stralloc.h"
#include "base64.h"

int
main(int argc, char **argv)
{
	int             i;
	stralloc        userout = { 0 };

	for (i = 1; i < argc; i++)
	{
		b64decode((const unsigned char *) argv[i], strlen(argv[i]), &userout);
		userout.s[userout.len] = 0;
		printf("%-30s %s\n", argv[i], userout.s);
	}
	return(0);
}

void
getversion_b64decode_c()
{
	static char    *x = "$Id: b64decode.c,v 1.6 2005-08-23 17:14:35+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
