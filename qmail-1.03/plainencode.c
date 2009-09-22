/*
 * $Log: plainencode.c,v $
 * Revision 1.3  2004-10-22 20:27:57+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.2  2004-07-17 21:20:08+05:30  Cprogrammer
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
	int             len, c;
	stralloc        user = { 0 };
	stralloc        userout = { 0 };
	char            tmpbuf[128];

	if(argc != 3)
	{
		fprintf(stderr, "USAGE: %s username password\n", argv[0]);
		return(1);
	}
	if((len = strlen(argv[1]) + strlen(argv[2])) > 127)
	{
		fprintf(stderr, "argument too long\n");
		return(1);
	}
	len += 2;
	snprintf(tmpbuf, 128, "^%s^%s", argv[1], argv[2]);
	for (c = len - 1; c >= 0; c--)
	{
		if (tmpbuf[c] == '^')
			tmpbuf[c] = '\0';
	}
	stralloc_copyb(&user, tmpbuf, len);
	b64encode(&user, &userout);
	userout.s[userout.len] = 0;
	printf("%-15s %-15s %s\n", argv[1], argv[2], userout.s);
	return(0);
}

void
getversion_plainencode_c()
{
	static char    *x = "$Id: plainencode.c,v 1.3 2004-10-22 20:27:57+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
