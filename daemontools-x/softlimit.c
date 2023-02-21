/*
 * $Log: softlimit.c,v $
 * Revision 1.4  2022-02-25 09:51:51+05:30  Cprogrammer
 * added option to set message queue limit
 * use -1 to set resource limit as unlimited
 *
 * Revision 1.3  2021-08-30 12:04:53+05:30  Cprogrammer
 * define funtions as noreturn
 *
 * Revision 1.2  2004-10-22 20:30:29+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 18:37:39+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pathexec.h>
#include <sgetopt.h>
#include <strerr.h>
#include <scan.h>
#include <str.h>
#include <noreturn.h>

#define FATAL "softlimit: fatal: "

no_return void
die_usage(void)
{
	strerr_die1x(100,
				 "softlimit: usage: softlimit [-a allbytes] [-c corebytes] [-d databytes] [-f filebytes] [-l lockbytes] [-m membytes] [-o openfiles] [-p processes] [-r residentbytes] [-s stackbytes] [-t cpusecs] child");
}

void
doit(int resource, char *arg)
{
	long            u;
	struct rlimit   r;

	if (getrlimit(resource, &r) == -1)
		strerr_die2sys(111, FATAL, "getrlimit failed: ");

	if (str_equal(arg, "="))
		r.rlim_cur = r.rlim_max;
	else {
		if (arg[scan_long(arg, &u)])
			die_usage();
		if (u == -1) {
			r.rlim_cur = r.rlim_max = RLIM_INFINITY;
		} else {
			r.rlim_cur = u;
			if (r.rlim_cur > r.rlim_max)
				r.rlim_cur = r.rlim_max;
		}
	}

	if (setrlimit(resource, &r) == -1)
		strerr_die2sys(111, FATAL, "setrlimit failed: ");
}

int
main(int argc, char **argv, char **envp)
{
	int             opt;

	while ((opt = getopt(argc, argv, "a:c:d:f:l:m:o:p:q:r:s:t:")) != opteof)
		switch (opt)
		{
		case '?':
			die_usage();
		case 'a':
#ifdef RLIMIT_AS
			doit(RLIMIT_AS, optarg);
#endif
#ifdef RLIMIT_VMEM
			doit(RLIMIT_VMEM, optarg);
#endif
			break;
		case 'c':
#ifdef RLIMIT_CORE
			doit(RLIMIT_CORE, optarg);
#endif
			break;
		case 'd':
#ifdef RLIMIT_DATA
			doit(RLIMIT_DATA, optarg);
#endif
			break;
		case 'f':
#ifdef RLIMIT_FSIZE
			doit(RLIMIT_FSIZE, optarg);
#endif
			break;
		case 'l':
#ifdef RLIMIT_MEMLOCK
			doit(RLIMIT_MEMLOCK, optarg);
#endif
			break;
		case 'm':
#ifdef RLIMIT_DATA
			doit(RLIMIT_DATA, optarg);
#endif
#ifdef RLIMIT_STACK
			doit(RLIMIT_STACK, optarg);
#endif
#ifdef RLIMIT_MEMLOCK
			doit(RLIMIT_MEMLOCK, optarg);
#endif
#ifdef RLIMIT_VMEM
			doit(RLIMIT_VMEM, optarg);
#endif
#ifdef RLIMIT_AS
			doit(RLIMIT_AS, optarg);
#endif
			break;
		case 'o':
#ifdef RLIMIT_NOFILE
			doit(RLIMIT_NOFILE, optarg);
#endif
#ifdef RLIMIT_OFILE
			doit(RLIMIT_OFILE, optarg);
#endif
			break;
		case 'p':
#ifdef RLIMIT_NPROC
			doit(RLIMIT_NPROC, optarg);
#endif
			break;
		case 'q':
#ifdef RLIMIT_MSGQUEUE
			doit(RLIMIT_MSGQUEUE, optarg);
#endif
			break;
		case 'r':
#ifdef RLIMIT_RSS
			doit(RLIMIT_RSS, optarg);
#endif
			break;
		case 's':
#ifdef RLIMIT_STACK
			doit(RLIMIT_STACK, optarg);
#endif
			break;
		case 't':
#ifdef RLIMIT_CPU
			doit(RLIMIT_CPU, optarg);
#endif
			break;
		}

	argv += optind;
	if (!*argv)
		die_usage();

	pathexec_run(*argv, argv, envp);
	strerr_die4sys(111, FATAL, "unable to run ", *argv, ": ");
	/*- Not reached */
	return(1);
}

void
getversion_softlimit_c()
{
	static char    *x = "$Id: softlimit.c,v 1.4 2022-02-25 09:51:51+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
