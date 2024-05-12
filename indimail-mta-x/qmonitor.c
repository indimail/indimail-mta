/*
 * $Id: qmonitor.c,v 1.7 2024-05-12 00:20:03+05:30 mbhangui Exp mbhangui $
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

const char     *usage =
	"usage: qmonitor [-t threshold ] [-i interval] [-n]\n"
	"        -t threshold (load threshold)\n"
	"        -i interval";

int
main(int argc, char **argv)
{
	int             i, opt, qcount, qconf, interval = 60,
					verbose = 0;
	double          threshold, total_load[2], qload;
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
			subprintf(subfdout, "queue load avg (local[%6.2f] + remote[%6.2f])/2 = %6.2f %2s (%.0f * %d) = %.2f\n",
					total_load[0], total_load[1], (total_load[0] + total_load[1])/2,
					((total_load[0] + total_load[1]) > 2 * qcount * threshold) ?  ">" : "<=",
					threshold, qcount, (qcount * threshold));
			substdio_flush(subfdout);
		}
		if ((total_load[0] + total_load[1]) > 2 * qcount * threshold) {
			for (i = 0; i < qcount; i++) {
				qload = (double) queue[i].lcur * 100/queue[i].lmax + (double) queue[i].rcur * 100/queue[i].rmax;
				subprintf(subfdout, "queue[%d] load = %.2f\n", i, qload);
				substdio_flush(subfdout);
				send_qload("/qscheduler", i + 1, qload, 10);
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
main(int argc, char **argv)
{
	substdio_puts(subfderr, "not compiled with -DLIBRT\n");
	substdio_flush(subfderr);
	_exit(111);
}
#endif

void
getversion_qmonitor_c()
{
	const char     *x = "$Id: qmonitor.c,v 1.7 2024-05-12 00:20:03+05:30 mbhangui Exp mbhangui $";

	x++;
}

/*-
 * $Log: qmonitor.c,v $
 * Revision 1.7  2024-05-12 00:20:03+05:30  mbhangui
 * fix function prototypes
 *
 * Revision 1.6  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.5  2023-01-18 00:02:13+05:30  Cprogrammer
 * replaced qprintf with subprintf
 *
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
