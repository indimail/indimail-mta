/*
 * $Log: daneq.c,v $
 * Revision 1.1  2018-04-26 11:37:56+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "subfd.h"
#include "sgetopt.h"
#include "strerr.h"
#include "tlsacheck.h"

#define FATAL "daneq: fatal: "
#define WARN  "daneq: warning: "

void
out(char *str)
{
	if (!str || !*str)
		return;
	if (substdio_puts(subfdout, str) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
flush()
{
	if (substdio_flush(subfdout) == -1)
		strerr_die2sys(111, FATAL, "write: ");
	return;
}

void
timeoutfn()
{
	out("timedout querying qmail-dane\n");
	flush();
	return;
}

void
err_tmpfail(char *arg)
{
	out("temporary failure: ");
	out(arg);
	out("\n");
	flush();
	return;
}

char           *usage = "usage: daneq -d domain ipaddr";

int
main(int argc, char **argv)
{
	char           *domain = 0;
	int             opt;

	while ((opt = getopt(argc, argv, "d:")) != opteof) {
		switch (opt)
		{
		case 'd':
			domain = optarg;
			break;
		}
	}
	if (optind + 1 != argc)
		strerr_die1x(100, usage);
	switch (tlsacheck(argv[optind], domain, timeoutfn, err_tmpfail))
	{
		case 1:/*- success */
			out("success\n");
			flush();
			return (0);
		case 0:
			out("tlsa verification failed\n");
			flush();
			return (1);
		case -1:
			out("system error\n");
			flush();
			return (-1);
			break;
		case -2:
			out("memory error\n");
			flush();
			return (-2);
	}
	/*- never reached */
	return (0);
}

void
getversion_daneq_c()
{
	static char    *x = "$Id: daneq.c,v 1.1 2018-04-26 11:37:56+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
