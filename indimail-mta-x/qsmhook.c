/*
 * $Id: qsmhook.c,v 1.12 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $
 */
#include <unistd.h>
#include <fd.h>
#include <stralloc.h>
#include <sgetopt.h>
#include <wait.h>
#include <env.h>
#include <byte.h>
#include <str.h>
#include <alloc.h>
#include <case.h>
#include <subfd.h>
#include <error.h>
#include <substdio.h>
#include <sig.h>
#include <noreturn.h>

no_return void
die(int e, const char *s)
{
	substdio_putsflush(subfderr, s);
	_exit(e);
}

no_return void
die_usage()
{
	die(100, "qsmhook: fatal: incorrect usage\n");
}

no_return void
die_temp()
{
	die(111, "qsmhook: fatal: temporary problem\n");
}

no_return void
die_read()
{
	die(111, "qsmhook: fatal: unable to read message\n");
}

no_return void
die_badcmd()
{
	die(100, "qsmhook: fatal: command not found\n");
}

int
main(int argc, char **argv)
{
	unsigned int    i;
	int             pid, wstat, opt, flagesc, flagrpline = 0, flagufline = 1, flagdtline = 0;
	int             pi[2];
	char          **arg;
	char           *x, *rpline, *ufline, *dtline, *host, *sender, *recip;
	char            inbuf[SUBSTDIO_INSIZE], outbuf[SUBSTDIO_OUTSIZE];
	stralloc        newarg = { 0 };
	substdio        ssin, ssout;

	sig_pipeignore();

	if (!(dtline = env_get("DTLINE")))
		die_usage();
	if (!(rpline = env_get("RPLINE")))
		die_usage();
	if (!(ufline = env_get("UFLINE")))
		die_usage();
	if (!(recip = env_get("LOCAL")))
		die_usage();
	if (!(host = env_get("HOST")))
		die_usage();
	if (!(sender = env_get("SENDER")))
		die_usage();
	while ((opt = getopt(argc, argv, "DFlMmnPsx:")) != opteof) {
		switch (opt)
		{
		case 'D':
		case 'F':
		case 'M':
			break;	/*- be serious */
		case 'l':
			flagdtline = 1;
			break;	/*- also return-receipt-to? blech */
		case 'm':
			break;	/*- we only handle one recipient anyway */
		case 'n':
			flagufline = 0;
			break;
		case 's':
			break;	/*- could call quote() otherwise, i suppose...  */
		case 'P':
			flagrpline = 1;
			break;
		case 'x':
			if (case_starts(recip, optarg))
				recip += str_len(optarg);
			break;
		default:
			_exit(100);
		}
	}
	argc -= optind;
	argv += optind;
	if (!*argv)
		die_usage();
	for (arg = argv; (x = *arg); ++arg) {
		if (!stralloc_copys(&newarg, ""))
			die_temp();
		flagesc = 0;
		for (i = 0; x[i]; ++i) {
			if (flagesc) {
				switch (x[i])
				{
				case '%':
					if (!stralloc_cats(&newarg, "%"))
						die_temp();
					break;
				case 'g':
					if (!stralloc_cats(&newarg, sender))
						die_temp();
					break;
				case 'h':
					if (!stralloc_cats(&newarg, host))
						die_temp();
					break;
				case 'u':
					if (!stralloc_cats(&newarg, recip))
						die_temp();
					break;
				}
				flagesc = 0;
			} else
			if (x[i] == '%')
				flagesc = 1;
			else
			if (!stralloc_append(&newarg, &x[i]))
				die_temp();
		}
		if (!stralloc_0(&newarg))
			die_temp();
		i = str_len(newarg.s) + 1;
		if (!(x = alloc(i)))
			die_temp();
		byte_copy(x, i, newarg.s);
		*arg = x;
	}
	if (pipe(pi) == -1)
		die_temp();
	switch (pid = fork())
	{
	case -1:
		die_temp();
	case 0:
		close(pi[1]);
		if (fd_move(0, pi[0]) == -1)
			die_temp();
		sig_pipedefault();
		execvp(*argv, argv);
		if (error_temp(errno))
			die_temp();
		die_badcmd();
	}
	close(pi[0]);
	substdio_fdbuf(&ssout, (ssize_t (*)(int,  char *, size_t)) write, pi[1], outbuf, sizeof(outbuf));
	substdio_fdbuf(&ssin, (ssize_t (*)(int,  char *, size_t)) read, 0, inbuf, sizeof(inbuf));
	if (flagufline)
		substdio_bputs(&ssout, ufline);
	if (flagrpline)
		substdio_bputs(&ssout, rpline);
	if (flagdtline)
		substdio_bputs(&ssout, dtline);
	if (substdio_copy(&ssout, &ssin) == -2)
		die_read();
	substdio_flush(&ssout);
	close(pi[1]);
	if (wait_pid(&wstat, pid) == -1)
		die_temp();
	if (wait_crashed(wstat))
		die_temp();
	_exit(wait_exitcode(wstat));
	/*- Not reached */
	return(0);
}

void
getversion_qsmhook_c()
{
	const char     *x = "$Id: qsmhook.c,v 1.12 2025-01-22 00:30:35+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
/*
 * $Log: qsmhook.c,v $
 * Revision 1.12  2025-01-22 00:30:35+05:30  Cprogrammer
 * Fixes for gcc14
 *
 * Revision 1.11  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.10  2021-08-29 23:27:08+05:30  Cprogrammer
 * define functions as noreturn
 *
 * Revision 1.9  2020-11-24 13:47:49+05:30  Cprogrammer
 * removed exit.h
 *
 * Revision 1.8  2020-05-15 10:58:21+05:30  Cprogrammer
 * use unsigned int to store return value of str_len
 *
 * Revision 1.7  2004-10-22 20:29:48+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.6  2004-10-22 15:38:25+05:30  Cprogrammer
 * removed readwrite.h
 *
 * Revision 1.5  2004-07-17 21:22:24+05:30  Cprogrammer
 * added RCS log
 *
 */
