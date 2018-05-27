/*
 * $Log: daneq.c,v $
 * Revision 1.2  2018-05-27 17:47:16+05:30  Cprogrammer
 * added option for qmail-remote to query/update records
 *
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

void
print_status(char status)
{
	switch (status)
	{
	case RECORD_NEW:
		out("RECORD NEW");
		break;
	case RECORD_WHITE:
		out("RECORD WHITE");
		break;
	case RECORD_NOVRFY:
		out("RECORD NOVERIFY");
		break;
	case RECORD_OK:
		out("RECORD OK");
		break;
	case RECORD_OLD:
		out("RECORD OLD");
		break;
	case RECORD_FAIL:
		out("RECORD FAIL");
		break;
	default:
		out("RECORD UNKNOWN");
		break;
	}
	return;
}

char           *usage =
				"usage: daneq [qsf] -d domain ipaddr\n"
				"        -q (query mode for record check)\n"
				"        -S (update mode success)\n"
				"        -F (update mode failure)";

int
main(int argc, char **argv)
{
	char           *domain = 0;
	int             opt, query_or_update = 0;
	char            rbuf[2];

	while ((opt = getopt(argc, argv, "qSFd:")) != opteof) {
		switch (opt)
		{
		case 'q':
			query_or_update = 1;
			break;
		case 'S':
			query_or_update = 2;
			break;
		case 'F':
			query_or_update = 3;
			break;
		case 'd':
			domain = optarg;
			break;
		}
	}
	if (optind + 1 != argc)
		strerr_die1x(100, usage);
	switch (tlsacheck(argv[optind], domain, query_or_update, rbuf, timeoutfn, err_tmpfail))
	{
		case 0:
			out(query_or_update ? "does not exist " : "tlsa verification failed ");
			print_status(rbuf[1]);
			out("\n");
			flush();
			return (1);
		case 1:/*- success */
			out(query_or_update ? "exists " : "success ");
			print_status(rbuf[1]);
			out("\n");
			flush();
			return (0);
		case 2: /*- update failed */
			out("update failed ");
			print_status(rbuf[1]);
			out("\n");
			flush();
			return (1);
		case 3: /*- update succeeded */
			out("update succeeded ");
			print_status(rbuf[1]);
			out("\n");
			flush();
			break;
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
	static char    *x = "$Id: daneq.c,v 1.2 2018-05-27 17:47:16+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
