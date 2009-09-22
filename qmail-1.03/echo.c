/*
 *  $Log: echo.c,v $
 *  Revision 1.5  2004-10-22 20:24:44+05:30  Cprogrammer
 *  added RCS id
 *
 *  Revision 1.4  2004-07-13 23:10:19+05:30  Cprogrammer
 *  fixed compiler warnings
 *
 *  Revision 1.3  2003-11-05 14:28:47+05:30  Cprogrammer
 *  fixed segmentation fault
 *
 *  Revision 1.2  2003-10-23 01:19:34+05:30  Cprogrammer
 *  fixed compilation warnings
 *
 *  Revision 1.1  2003-10-23 00:10:11+05:30  Cprogrammer
 *  Initial revision
 *
 */
#include <stdlib.h>
#include <stdio.h>

int
main(argc, argv)
	int             argc;
	char           *argv[];
{
	FILE           *fp;

	++argv;
	if(argv[0] && *argv[0] == '1')
		fp = stdout;
	else
	if(argv[0] && *argv[0] == '2')
		fp = stderr;
	else
	{
		fprintf(stderr, "usage: echo [1|2] args\n");
		fprintf(stderr, "1 - stdout\n");
		fprintf(stderr, "2 - stderr\n");
		exit(1);
	}
	++argv;
	while (argv[0])
	{
		(void) fprintf(fp, "%s", argv[0]);
		if (*++argv)
			fputc(' ', fp);
	}
	fputc('\r', fp);
	fputc('\n', fp);
	exit(0);
}

void
getversion_echo_c()
{
	static char    *x = "$Id: echo.c,v 1.5 2004-10-22 20:24:44+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
