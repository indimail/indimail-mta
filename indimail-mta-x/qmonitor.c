/*
 * $Id: qmonitor.c,v 1.4 2022-08-14 21:57:53+05:30 Cprogrammer Exp mbhangui $
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
#include <getEnvConfig.h>
#include <qprintf.h>
#include "queue_load.h"
#include "set_environment.h"
#include "send_qload.h"

#define FATAL "qmonitor: fatal: "
#define WARN  "qmonitor: warn: "

char           *usage =
	"usage: qmonitor [-t threshold ] [-i interval] [-n]\n"
	"        -t threshold (load threshold)\n"
	"        -i interval";

int
main(int argc, char **argv)
{
	int             i, j, opt, qcount, qconf, interval = 60,
					verbose = 0;
	double          threshold, total_load[2], qload;
	char            strnum[FMT_DOUBLE];
	QDEF           *queue = (QDEF *) NULL;

	set_environment(WARN, FATAL, 0);
	getEnvConfigDouble(&threshold,   "QUEUE_LOAD",   QUEUE_LOAD);
	while ((opt = getopt(argc, argv, "i:t:v")) != opteof) {
		switch (opt)
		{
		case 't':
			scan_double(optarg, &threshold);
			break;
		case 'i':
			scan_int(optarg, &interval);
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			strerr_die1x(100, usage);
		}
	}
	for (;;) {
		queue_load("qmonitor", &qcount, &qconf, total_load, &queue);
		if (verbose || ((total_load[0] + total_load[1]) > 2 * qcount * threshold)) {
			substdio_put(subfdout, "queue load avg (local[", 22);
			strnum[i = fmt_double(strnum, total_load[0], 2)] = 0;
			qprintf(subfdout, strnum, "%+6");
			substdio_put(subfdout, "] + remote[", 11);
			strnum[i = fmt_double(strnum, total_load[1], 2)] = 0;
			qprintf(subfdout, strnum, "%+6s");
			substdio_put(subfdout, "])/2 = ", 7);
			strnum[i = fmt_double(strnum, (total_load[0] + total_load[1])/2, 2)] = 0;
			qprintf(subfdout, strnum, "%+6s");
			if ((total_load[0] + total_load[1]) > 2 * qcount * threshold)
				substdio_put(subfdout, " >  (", 5);
			else
				substdio_put(subfdout, " <= (", 5);
			strnum[i = fmt_int(strnum, threshold)] = 0;
			substdio_put(subfdout, strnum, i);
			substdio_put(subfdout, " * ", 3);
			strnum[i = fmt_int(strnum, qcount)] = 0;
			substdio_put(subfdout, strnum, i);
			substdio_put(subfdout, ") = ", 4);
			strnum[i = fmt_double(strnum, qcount * threshold, 2)] = 0;
			substdio_put(subfdout, strnum, i);
			substdio_put(subfdout, "\n", 1);
			substdio_flush(subfdout);
		}
		if ((total_load[0] + total_load[1]) > 2 * qcount * threshold) {
			for (j = 0; j < qcount; j++) {
				qload = (double) queue[j].lcur * 100/queue[j].lmax + (double) queue[j].rcur * 100/queue[j].rmax;
				substdio_put(subfdout, "queue[", 6);
				strnum[i = fmt_int(strnum, j)] = 0;
				substdio_put(subfdout, strnum, i);
				substdio_put(subfdout, "] load = ", 9);
				strnum[i = fmt_double(strnum, qload, 2)] = 0;
				substdio_put(subfdout, strnum, i);
				substdio_put(subfdout, "\n", 1);
				substdio_flush(subfdout);
				send_qload("/qscheduler", j + 1, qload, 10);
			}
		}
		sleep(interval);
	}
	return 0;
}
#else
#warning "not compiled with -DHASLIBRT"
#include <substdio.h>
#include <subfd.h>
#include <unistd.h>

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
	static char    *x = "$Id: qmonitor.c,v 1.4 2022-08-14 21:57:53+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*-
 * $Log: qmonitor.c,v $
 * Revision 1.4  2022-08-14 21:57:53+05:30  Cprogrammer
 * fix compilation warning if HASLIBRT is undefined
 *
 * Revision 1.3  2022-04-24 13:08:54+05:30  Cprogrammer
 * display avg load and calculation
 *
 * Revision 1.2  2022-04-24 10:06:37+05:30  Cprogrammer
 * removed display code
 *
 * Revision 1.1  2022-04-24 08:51:15+05:30  Cprogrammer
 * Initial revision
 *
 */
