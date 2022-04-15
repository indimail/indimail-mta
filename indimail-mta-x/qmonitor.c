/*
 * $Id: qmonitor.c,v 1.1 2022-04-16 01:32:45+05:30 Cprogrammer Exp mbhangui $
 */
#include "haslibrt.h"
#ifdef HASLIBRT
#include <unistd.h>
#include <substdio.h>
#include <subfd.h>
#include <fmt.h>
#include <sgetopt.h>
#include <strerr.h>
#include <scan.h>
#include "queue_load.h"
#include "send_qload.h"

char           *usage =
				"usage: qmonitor [-t threshold ] recipient\n"
				"        -t threshold (load threshold)\n"
				"        -n Report mode. Do not actually increase qcount";

void
display(int qno, double qload)
{
	char            strnum[FMT_ULONG];
	int             i;

	substdio_put(subfdout, "queue[", 6);
	strnum[i = fmt_ulong(strnum, qno)] = 0;
	substdio_put(subfdout, strnum, i);
	substdio_put(subfdout, "] load = ", 9);
	strnum[i = fmt_double(strnum, qload, 4)] = 0;
	substdio_put(subfdout, strnum, i);
	substdio_put(subfdout, "\n", 1);
	substdio_flush(subfdout);
}

int
main(int argc, char **argv)
{
	int             j, opt, qcount, qconf, interval = 60, test_mode = 0;
	double          threshold, total_load, qload;
	QDEF           *queue = (QDEF *) NULL;

	while ((opt = getopt(argc, argv, "i:t:n")) != opteof) {
		switch (opt)
		{
		case 't':
			scan_double(optarg, &threshold);
			break;
		case 'i':
			scan_int(optarg, &interval);
			break;
		case 'n':
			test_mode = 1;
			break;
		default:
			strerr_die1x(100, usage);
		}
	}
	for (;;) {
		queue_load("qmonitor", 1, &qcount, &qconf, &total_load, &queue);
		for (j = 0; j < qcount; j++) {
			qload = (double) queue[j].lcur * 100/queue[j].lmax + (double) queue[j].rcur * 100/queue[j].rmax;
			if (test_mode)
				display(j + 1, qload);
			else
			if (total_load > qcount * threshold)
				send_qload("/qscheduler", j + 1, qload, 10);
		}
		sleep(5);
	}
}
#else
#warning "not compiled with -DHASLIBRT"
#include <substdio.h>
#include <subfd.h>
#include <unistd.h>

static char     sserrbuf[512];
struct substdio sserr;

int
main(argc, argv)
	int             argc;
	char          **argv;
{
	substdio_puts(subfderr, "not compiled with -DLIBRT\n");
	substdio_flush(subfderr);
	_exit(111);
}
#endif

void
getversion_qmonitor_c()
{
	static char    *x = "$Id: qmonitor.c,v 1.1 2022-04-16 01:32:45+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*-
 * $Log: qmonitor.c,v $
 * Revision 1.1  2022-04-16 01:32:45+05:30  Cprogrammer
 * Initial revision
 *
 */
