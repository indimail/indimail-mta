/*
 * $Id: qdane.c,v 1.5 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $
 */
#include "subfd.h"
#include "sgetopt.h"
#include "strerr.h"
#include "tlsacheck.h"

#define FATAL "qdane: fatal: "
#define WARN  "qdane: warn: "

#ifdef LIBC_HAS_IP6
int             noipv6 = 0;
#else
int             noipv6 = 1;
#endif

void
out(const char *str)
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
timeoutfn(void)
{
	out("timedout querying qmail-dane\n");
	flush();
	return;
}

void
err_tmpfail(const char *arg)
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

const char     *usage =
				"usage: qdane [qsf] -d mxhost ipaddr\n"
				"        -q (query mode  - DANE VERIFICATION)\n"
				"        -S (update mode - success)\n"
				"        -F (update mode - failure)";

int
main(int argc, char **argv)
{
	char           *domain = 0;
	int             opt, query_or_update = DEFAULT_MODE;
	char            rbuf[2];

	while ((opt = getopt(argc, argv, "qSFd:")) != opteof) {
		switch (opt)
		{
		case 'q':
			query_or_update = QUERY_MODE;
			break;
		case 'S':
			query_or_update = UPDATE_SUCCESS;
			break;
		case 'F':
			query_or_update = UPDATE_FAILURE;
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
getversion_qdane_c()
{
	const char     *x = "$Id: qdane.c,v 1.5 2025-01-22 00:30:36+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
/*
 * $Log: qdane.c,v $
 * Revision 1.5  2025-01-22 00:30:36+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.4  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.3  2018-05-27 22:15:49+05:30  mbhangui
 * added defintions for qmail-daned modes
 *
 * Revision 1.2  2018-05-27 17:47:16+05:30  Cprogrammer
 * added option for qmail-remote to query/update records
 *
 * Revision 1.1  2018-04-26 11:37:56+05:30  Cprogrammer
 * Initial revision
 *
 */
