/*
 * $Log: b64encode.c,v $
 * Revision 1.5  2004-10-22 20:18:29+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.4  2004-07-17 21:16:25+05:30  Cprogrammer
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
	stralloc        user = { 0 };
	stralloc        userout = { 0 };

	for (i = 1; i < argc; i++)
	{
		stralloc_copyb(&user, argv[i], strlen(argv[i]));
		b64encode(&user, &userout);
		userout.s[userout.len] = 0;
		printf("%-30s %s\n", argv[i], userout.s);
	}
	return(0);
}

void
getversion_b64encode_c()
{
	static char    *x = "$Id: b64encode.c,v 1.5 2004-10-22 20:18:29+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
