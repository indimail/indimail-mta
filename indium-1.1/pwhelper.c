/*
 * $Log: pwhelper.c,v $
 * Revision 2.3  2009-02-25 11:26:29+05:30  Cprogrammer
 * added proto for pipe_exec()
 *
 * Revision 2.2  2009-02-23 13:49:33+05:30  Cprogrammer
 * added sccsidh
 *
 * Revision 2.1  2003-02-11 23:13:00+05:30  Cprogrammer
 * authentication helper for indium
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef	lint
static char     sccsid[] = "$Id: pwhelper.c,v 2.3 2009-02-25 11:26:29+05:30 Cprogrammer Exp mbhangui $";
#endif

int             pipe_exec(char **, char *, int);

int
main(int argc, char **argv)
{
	char            tmpbuf[512];
	int             len;

	if (argc < 4)
		exit(1);
	memset(tmpbuf, 0, 512);
	snprintf(tmpbuf, sizeof(tmpbuf), "%s", argv[1]);
	len = strlen(tmpbuf);
	snprintf(tmpbuf + len + 1, sizeof(tmpbuf) - len, "%s", argv[2]);
	return (pipe_exec(argv + 2, tmpbuf, 512));
}

void
getversion_pwhelper_c()
{
	printf("%s\n", sccsid);
}
