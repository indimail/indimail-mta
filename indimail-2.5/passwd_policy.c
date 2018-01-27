/*
 * $Log: passwd_policy.c,v $
 * Revision 2.3  2008-09-11 23:01:35+05:30  Cprogrammer
 * removed unused definition
 *
 * Revision 2.2  2008-07-13 19:45:48+05:30  Cprogrammer
 * compilation for Mac OS X
 *
 * Revision 2.1  2002-07-15 02:25:41+05:30  Cprogrammer
 * passwd policy routine
 *
 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#define _XOPEN_SOURCE
#include <unistd.h>

#ifndef lint
static char     sccsid[] = "$Id: passwd_policy.c,v 2.3 2008-09-11 23:01:35+05:30 Cprogrammer Stab mbhangui $";
#endif

static int      get_dict(char *);
static int      get_hist(char *);

int
passwd_policy(passwd)
	char           *passwd;
{
	char           *ptr;
	int             len, alpha, numeric;

	if ((len = strlen(passwd)) < 8)
	{
		(void) fprintf(stderr, "passwd must be of minimum 8 chars\n");
		return (1);
	}
	for (ptr = passwd, alpha = numeric = 0; *ptr; ptr++)
	{
		if (isspace((int) *ptr))
		{
			(void) fprintf(stderr, "whitespace not allowed\n");
			return (1);
		}
		if (*ptr && *(ptr + 1) && (*ptr == *(ptr + 1)))
		{
			(void) fprintf(stderr, "two consequtive chars cannot be same\n");
			return (1);
		}
		if (isdigit((int) *ptr))
			numeric = 1;
		if (isalpha((int) *ptr))
			alpha = 1;
	}
	if (!alpha || !numeric)
	{
		(void) fprintf(stderr, "passwd must be alpha-numeric\n");
		return (1);
	}
	if (get_dict(passwd))
	{
		(void) fprintf(stderr, "passwd is restricted\n");
		return (1);
	}
	if (get_hist(passwd))
	{
		(void) fprintf(stderr, "passwd cannot be reused\n");
		return (1);
	}
	return (0);
}

static int
get_dict(passwd)
	char           *passwd;
{
	return (0);
}

static int
get_hist(passwd)
	char           *passwd;
{
	return (0);
}

void
getversion_passwd_policy_c()
{
	printf("%s\n", sccsid);
}
