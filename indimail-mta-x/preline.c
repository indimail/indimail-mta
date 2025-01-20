/*
 * $Log: preline.c,v $
 * Revision 1.12  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.11  2023-09-15 21:15:16+05:30  Cprogrammer
 * moved check for env variables after getopt
 *
 * Revision 1.10  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.9  2020-11-24 13:46:31+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.8  2010-04-07 16:05:53+05:30  Cprogrammer
 * fixed SIGSEGV when QQEH env variable was absent
 *
 * Revision 1.7  2004-10-22 20:27:59+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.6  2004-10-22 15:36:23+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.5  2004-07-17 21:20:19+05:30  Cprogrammer
 * added qqeh code
 * added RCS log
 *
 */
#include <unistd.h>
#include <fd.h>
#include <sgetopt.h>
#include <strerr.h>
#include <substdio.h>
#include <wait.h>
#include <env.h>
#include <sig.h>
#include <error.h>
#include <noreturn.h>

#define FATAL "preline: fatal: "

no_return void
die_usage(const char *arg)
{
	if (arg)
		strerr_die4x(111, FATAL, "No ", arg, " environment variable");
	else
		strerr_die1x(100, "preline: usage: preline cmd [ arg ... ]");
}


int
main(int argc, char **argv)
{
	int             pi[2];
	int             flagufline = 1, flagrpline = 1, flagdtline = 1,
					flagqqeh = 1, opt, pid, wstat;
	char           *ufline, *rpline, *dtline, *qqeh;
	char            outbuf[SUBSTDIO_OUTSIZE], inbuf[SUBSTDIO_INSIZE];
	substdio        ssout = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) write, 1, outbuf, sizeof outbuf);
	substdio        ssin = SUBSTDIO_FDBUF((ssize_t (*)(int,  char *, size_t)) read, 0, inbuf, sizeof inbuf);

	sig_pipeignore();

	while ((opt = getopt(argc, argv, "frde")) != opteof) {
		switch (opt)
		{
		case 'f':
			flagufline = 0;
			break;
		case 'r':
			flagrpline = 0;
			break;
		case 'd':
			flagdtline = 0;
			break;
		case 'e':
			flagqqeh = 0;
			break;
		default:
			die_usage(0);
		}
	}
	if (flagufline && !(ufline = env_get("UFLINE")))
		die_usage("UFLINE");
	if (flagrpline && !(rpline = env_get("RPLINE")))
		die_usage("RPLINE");
	if (flagdtline && !(dtline = env_get("DTLINE")))
		die_usage("DTLINE");
	if (flagqqeh && !(qqeh = env_get("QQEH")))
		die_usage("QQEH");
	argc -= optind;
	argv += optind;
	if (!*argv)
		die_usage(0);
	if (pipe(pi) == -1)
		strerr_die2sys(111, FATAL, "unable to create pipe: ");
	pid = fork();
	if (pid == -1)
		strerr_die2sys(111, FATAL, "unable to fork: ");
	if (pid == 0) {
		close(pi[1]);
		if (fd_move(0, pi[0]) == -1)
			strerr_die2sys(111, FATAL, "unable to set up fds: ");
		sig_pipedefault();
		execvp(*argv, argv);
		strerr_die4sys(error_temp(errno) ? 111 : 100, FATAL, "unable to run ", *argv, ": ");
	}
	close(pi[0]);
	if (fd_move(1, pi[1]) == -1)
		strerr_die2sys(111, FATAL, "unable to set up fds: ");
	if (flagufline)
		substdio_bputs(&ssout, ufline);
	if (flagrpline)
		substdio_bputs(&ssout, rpline);
	if (flagdtline)
		substdio_bputs(&ssout, dtline);
	if (flagqqeh && !(qqeh = env_get("QQEH")))
		flagqqeh = 0;
	if (flagqqeh)
		substdio_bputs(&ssout, qqeh);
	if (substdio_copy(&ssout, &ssin) != 0)
		strerr_die2sys(111, FATAL, "unable to copy input: ");
	substdio_flush(&ssout);
	close(1);
	if (wait_pid(&wstat, pid) == -1)
		strerr_die2sys(111, FATAL, "wait failed: ");
	if (wait_crashed(wstat))
		strerr_die2x(111, FATAL, "child crashed");
	_exit(wait_exitcode(wstat));
	/*- Not reached */
	return(0);
}

void
getversion_preline_c()
{
	const char     *x = "$Id: preline.c,v 1.12 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
