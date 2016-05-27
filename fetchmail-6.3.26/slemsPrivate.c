/*
 * $Log: slemsPrivate.c,v $
 * Revision 1.3  2008-06-20 22:21:42+05:30  Cprogrammer
 * fixed compilation warnings
 *
 * Revision 1.2  2008-06-20 20:51:46+05:30  Cprogrammer
 * added stdlib.h to fix compiler warning
 * moved programs to INDIMAILDIR
 *
 * Revision 1.1  2003-12-13 18:45:45+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "config.h"

int
main(int argc, char **argv)
{
	int             i;
	static char    *priv_cmds[] = {
		BINPREFIX "/bin/qmail-tcpok",
		BINPREFIX "/bin/svc",
		BINPREFIX "/bin/svstat",
		BINPREFIX "/bin/ppp-off",
		BINPREFIX "/bin/fixwvdialconf",
		BINPREFIX "/bin/qmail-qstat",
		"/usr/bin/killall",
		"/usr/bin/wvdial",
		0
	};

	if (argc < 2)
		return (1);
	for (i = 0; priv_cmds[i]; i++)
	{
		if (!strcmp(priv_cmds[i], argv[1]))
			break;
	}
	if (!priv_cmds[i])
	{
		fprintf(stderr, "%s: Invalid command\n", argv[1]);
		return (1);
	}
	if (setuid(0))
	{
		perror("setuid");
		return (1);
	} else
	if (setgid(0))
	{
		perror("setgid");
		return (1);
	}
	execv(argv[1], argv + 1);
	perror(argv[1]);
	exit(1);
}
