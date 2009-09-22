/*
 * $Log: makeheader.c,v $
 * Revision 1.3  2005-08-23 17:33:08+05:30  Cprogrammer
 * added stdlib.h
 *
 * Revision 1.2  2004-10-20 20:08:34+05:30  Cprogrammer
 * moved to libdomainkeys-0.62
 *
 * Revision 1.1  2004-09-22 23:31:47+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * This program creates headers by parsing a .c file looking for magic
 * comments.  It copies everything from STARTHEAD through STOPHEAD to
 * the header file.  Then it excludes everything between STARTPRIV and
 * STOPPRIV.  When it seems a HEADER comment, it copies everything from
 * there to the first line with a open brace, and appends a semicolon.
 *
 * There is no point in maintaining the same information in two files.
 * Better to generate the header from the source itself.
 */

int             printing = 0;
char            line[1024];	/*- domainkey.c doesn't have any lines longer than 1024, so enuf. */
int             linelen;

#define startswith(a,b) (!strncmp((a),(b),strlen(b)))

int
main()
{

	printf("/* This file is automatically created from the corresponding .c file */\n");
	printf("/* Do not change this file; change the .c file instead. */\n");
	while (fgets(line, sizeof(line), stdin))
	{
		if (line[strlen(line) - 1] != '\n')
		{
			fprintf(stderr, "oops, 'line' is too short\n");
			exit(1);
		}
		if (startswith(line, "/* STARTHEAD") || startswith(line, "/* STOPPRIV"))
			printing = 1;
		else
		if (startswith(line, "/* STOPHEAD") || startswith(line, "/* STARTPRIV"))
			printing = 0;
		else
		if (startswith(line, "/* HEADER"))
			printing = 2;
		else
		if (printing == 2 && startswith(line, "{"))
		{
			printf(";\n\n\n");
			printing = 0;
		} else
		if (printing)
			fputs(line, stdout);
	}
	return(0);
}
