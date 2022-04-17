/*
 * $Id: qmonitor.c,v 1.2 2022-04-17 08:31:39+05:30 Cprogrammer Exp mbhangui $
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
#include <qprintf.h>
#include <getEnvConfig.h>
#include "queue_load.h"
#include "send_qload.h"

char           *usage =
				"usage: qmonitor [-t threshold ] [-i interval] [-n]\n"
				"        -t threshold (load threshold)\n"
				"        -i interval\n"
				"        -n           (Report Mode)";

void
display(QDEF *queue, int queue_count, int queue_conf)
{
	char            strnum[FMT_DOUBLE];
	int             i, j, x, lcur = 0, rcur = 0, min = -1;
	double          load_l = 0.0, load_r = 0.0;

	for (j = 0; j < queue_count; j++) {
		x = queue[j].lcur + queue[j].rcur;
		if (min == -1 || x < min)
			min = x; /*- minimum concurrency */
	}
	for (j = 0; j < queue_count; j++) {
		if (!j)
			substdio_put(subfdout, "queue      local  Remote   +/- flag\n", 36);
		if (!queue[j].lmax || !queue[j].rmax) {
			substdio_put(subfderr, "invalid concurrency = 0 ", 24);
			if (!queue[j].lmax)
				substdio_put(subfderr, "[local] ", 8);
			if (!queue[j].rmax)
				substdio_put(subfderr, "[remote] ", 9);
			substdio_put(subfderr, "for queue ", 10);
			substdio_put(subfderr, queue[j].queue.s, queue[j].queue.len);
			substdio_put(subfderr, "\n", 1);
			substdio_flush(subfderr);
			continue;
		}
		lcur += queue[j].lcur;
		rcur += queue[j].rcur;
		load_l += (double) queue[j].lcur / queue[j].lmax;
		load_r += (double) queue[j].rcur / queue[j].rmax;
		qprintf(subfdout, queue[j].queue.s, "%-08s");
		substdio_put(subfdout, " ", 1);
		strnum[i = fmt_ulong(strnum, queue[j].lcur)] = 0;
		qprintf(subfdout, strnum, "%+3s");
		substdio_put(subfdout, "/", 1);
		strnum[i = fmt_ulong(strnum, queue[j].lmax)] = 0;
		qprintf(subfdout, strnum, "%-4s");
		substdio_put(subfdout, " ", 1);
		strnum[i = fmt_ulong(strnum, queue[j].rcur)] = 0;
		qprintf(subfdout, strnum, "%+3s");
		substdio_put(subfdout, "/", 1);
		strnum[i = fmt_ulong(strnum, queue[j].rmax)] = 0;
		qprintf(subfdout, strnum, "%-4s");
		x = queue[j].lcur + queue[j].rcur;
		substdio_put(subfdout, x == min ? " - " : " + ", 3);
		substdio_put(subfdout, queue[j].flag ? " disabled\n" : "  enabled\n", 10);
	}
	substdio_put(subfdout, "queue count = ", 14);
	strnum[i = fmt_ulong(strnum, queue_count)] = 0;
	substdio_put(subfdout, strnum, i);

	substdio_put(subfdout, ", queue configured = ", 21);
	strnum[i = fmt_ulong(strnum, queue_conf)] = 0;
	substdio_put(subfdout, strnum, i);

	substdio_put(subfdout, ", Total local = ", 16);
	strnum[i = fmt_ulong(strnum, lcur)] = 0;
	substdio_put(subfdout, strnum, i);

	substdio_put(subfdout, ", Total remote = ", 17);
	strnum[i = fmt_ulong(strnum, rcur)] = 0;
	substdio_put(subfdout, strnum, i);

	substdio_put(subfdout, ", Total queue load = ", 19);
	strnum[i = fmt_double(strnum, 100 * load_l, 2)] = 0;
	qprintf(subfdout, strnum, "%+6s");
	substdio_put(subfdout, " / ", 3);
	strnum[i = fmt_double(strnum, 100 * load_r, 2)] = 0;
	qprintf(subfdout, strnum, "%+6s");
	substdio_put(subfdout, "\n", 1);
	substdio_flush(subfdout);
}

int
main(int argc, char **argv)
{
	int             i, j, opt, qcount, qconf, interval = 60, test_mode = 0;
	double          threshold, total_load[2], qload;
	char            strnum[FMT_DOUBLE];
	QDEF           *queue = (QDEF *) NULL;

	getEnvConfigDouble(&threshold,   "QUEUE_LOAD",   QUEUE_LOAD);
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
		queue_load("qmonitor", &qcount, &qconf, total_load, &queue);
		if (test_mode) {
			display(queue, qcount, qconf);
			break;
		}
		substdio_put(subfdout, "queue loads local[", 18);
		strnum[i = fmt_double(strnum, total_load[0], 2)] = 0;
		substdio_put(subfdout, strnum, i);
		substdio_put(subfdout, "] + remote[", 11);
		strnum[i = fmt_double(strnum, total_load[1], 2)] = 0;
		substdio_put(subfdout, strnum, i);
		if ((total_load[0] + total_load[1]) > 2 * qcount * threshold)
			substdio_put(subfdout, "] >  ", 5);
		else
			substdio_put(subfdout, "] <= ", 5);
		strnum[i = fmt_double(strnum, 2 * qcount * threshold, 2)] = 0;
		substdio_put(subfdout, strnum, i);
		substdio_put(subfdout, "\n", 1);
		substdio_flush(subfdout);
		if ((total_load[0] + total_load[1]) > 2 * qcount * threshold) {
			for (j = 0; j < qcount; j++) {
				qload = (double) queue[j].lcur * 50/queue[j].lmax + (double) queue[j].rcur * 50/queue[j].rmax;
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
	static char    *x = "$Id: qmonitor.c,v 1.2 2022-04-17 08:31:39+05:30 Cprogrammer Exp mbhangui $";

	x++;
}

/*-
 * $Log: qmonitor.c,v $
 * Revision 1.2  2022-04-17 08:31:39+05:30  Cprogrammer
 * use getEnvConfigDouble() to set threshold from QUEUE_LOAD env variable
 *
 * Revision 1.1  2022-04-16 13:01:40+05:30  Cprogrammer
 * Initial revision
 *
 */
