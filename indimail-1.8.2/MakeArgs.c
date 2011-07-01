/*
 * $Log: MakeArgs.c,v $
 * Revision 2.5  2007-12-22 00:20:05+05:30  Cprogrammer
 * allow and expand environment variables in the argument
 *
 * Revision 2.4  2005-03-30 22:52:47+05:30  Cprogrammer
 * BUG - Incorrect free
 *
 * Revision 2.3  2004-07-12 22:47:58+05:30  Cprogrammer
 * bug fix. Free all allocated members
 *
 * Revision 2.2  2002-12-21 18:21:09+05:30  Cprogrammer
 * added functionality of escaping text via quotes
 *
 * Revision 2.1  2002-08-13 20:35:44+05:30  Cprogrammer
 * addition spaces were not getting skipped
 *
 * Revision 1.2  2002-03-03 17:23:05+05:30  Cprogrammer
 * replaced strcpy with scopy
 *
 * Revision 1.1  2001-12-13 01:46:09+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "indimail.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef lint
static char     sccsid[] = "$Id: MakeArgs.c,v 2.5 2007-12-22 00:20:05+05:30 Cprogrammer Stab mbhangui $";
#endif

#define isEscape(ch) ((ch) == '"' || (ch) == '\'')

/*
 * function to expand a string into command line
 * arguments. To free memory allocated by this
 * function the following should be done
 *
 * free(argv); free(argv[0]);
 *
 */
char          **
MakeArgs(char *cmmd)
{
	char           *ptr, *sptr, *marker;
	char          **argv;
	int             argc, idx;

	for (ptr = cmmd;*ptr && isspace((int) *ptr);ptr++);
	idx = strlen(ptr);
	if (!(sptr = (char *) malloc((idx + 1) * sizeof(char))))
		return((char **) 0);
	strncpy(sptr, ptr, idx + 1);
	/*-
	 * Get the number of arguments by counting
	 * white spaces. Allow escape via the double
	 * quotes character at the first word
	 */
	for (argc = 0, ptr = sptr;*ptr;)
	{
		for (;*ptr && isspace((int) *ptr);ptr++);
		if (!*ptr)
			break;
		argc++;
		marker = ptr;
		for (;*ptr && !isspace((int) *ptr);ptr++)
		{
			if (ptr == marker && isEscape(*ptr))
			{
				for (ptr++;*ptr && !isEscape(*ptr);ptr++);
				if (!*ptr)
					ptr = marker;
			}
		}
	}
#ifdef DEBUG
	printf("argc = %d\n", argc);
#endif
	/*
	 * Allocate memory to store the arguments
	 * Do not bother extra bytes occupied by
	 * white space characters.
	 */
	if (!(argv = (char **) malloc((argc + 1) * sizeof(char *))))
		return ((char **) NULL);
	for (idx = 0, ptr = sptr;*ptr;)
	{
		for (;*ptr && isspace((int) *ptr);ptr++)
			*ptr = 0;
		if (!*ptr)
			break;
		if (*ptr == '$')
			argv[idx++] = getenv(ptr + 1);
		else
			argv[idx++] = ptr;
		marker = ptr;
		for (;*ptr && !isspace((int) *ptr);ptr++)
		{
			if (ptr == marker && isEscape(*ptr))
			{
				for (ptr++;*ptr && !isEscape(*ptr);ptr++);
				if (!*ptr)
					ptr = marker;
				else /*- Remove the quotes */
				{
					argv[idx - 1] += 1;
					*ptr = 0;
				}
			}
		}
	}
	argv[idx++] = (char *) 0;
#ifdef DEBUG
	for (idx = 0;idx <= argc;idx++)
		printf("argv[%d] = [%s]\n", idx, argv[idx]);
#endif
	return (argv);
}

void
FreeMakeArgs(char **argv)
{
	free(argv[0]);
	free(argv);
	return;
}

void
getversion_MakeArgs_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
	return;
}
