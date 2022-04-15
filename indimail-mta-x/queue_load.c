/*
 * $Id: queue_load.c,v 1.1 2022-04-16 01:32:47+05:30 Cprogrammer Exp mbhangui $
 */
#include "haslibrt.h"

#ifdef HASLIBRT
#include <substdio.h>
#include <subfd.h>
#include <qprintf.h>
#include <fmt.h>
#include <alloc.h>
#include <fcntl.h>
#include <error.h>
#include <stralloc.h>
#include <strerr.h>
#include <unistd.h>
#include <sys/mman.h>
#include "queue_load.h"

void
queue_load(char *argv0, int display, int *qcount, int *qconf, double *qload, QDEF **queue)
{
	int             shm, i, j, queue_count, queue_conf, len, x, min = -1, lcur = 0, rcur = 0;
	double          load;
	int             q[5];
	QDEF           *qdef;
	char            shm_name[256], strnum[FMT_DOUBLE];
	char           *s;

	/*- get queue count */
	if ((shm = shm_open("/qscheduler", O_RDONLY, 0644)) == -1) {
		if (errno == error_noent)
			strerr_die2x(111, argv0, ": fatal: dynamic queue disabled / qscheduler not running\n");
		strerr_die2sys(111, argv0, ": fatal: shm_open: /qscheduler: ");
	}
	if (read(shm, (char *) q, sizeof(int) * 2) == -1)
		strerr_die2x(111, argv0, ": fatal: unable to read POSIX shared memory segment /qscheduler");
	close(shm);
	queue_count = q[0];
	if (qcount)
		*qcount = queue_count;
	queue_conf = q[1];
	if (qconf)
		*qconf = queue_conf;
	if (!(qdef = (QDEF *) alloc(queue_count * sizeof(QDEF))))
		strerr_die2x(111, argv0, ": fatal: out of memory");
	if (queue)
		*queue = qdef;
	for (j = 0, load = 0.0, min = -1; j < queue_count; j++) {
		len = 0;
		s = shm_name;
		s += (i = fmt_str(s, "/queue"));
		len += i;
		s += (i = fmt_uint(s, j + 1));
		len += i;
		*s++ = 0;
		if (!stralloc_copyb(&qdef[j].queue, shm_name + 1, len - 1) ||
				!stralloc_0(&qdef[j].queue))
			strerr_die2x(111, argv0, ": fatal: out of memory");
		qdef[j].queue.len--;
		if ((shm = shm_open(shm_name, O_RDONLY, 0600)) == -1)
			strerr_die4sys(111, argv0, ": fatal: failed to open POSIX shared memory segment ", shm_name, ": ");
		if (read(shm, (char *) q, sizeof(int) * 5) == -1)
			strerr_die4sys(111, argv0, ": fatal: failed to read POSIX shared memory segment ", shm_name, ": ");
		close(shm);
		if (!q[2] || !q[3]) {
			substdio_puts(subfderr, argv0);
			substdio_put(subfderr, ": warning: invalid concurrency = 0 ", 35);
			if (!qdef[j].lmax)
				substdio_put(subfderr, "[local] ", 8);
			if (!qdef[j].rmax)
				substdio_put(subfderr, "[remote] ", 9);
			substdio_put(subfderr, "for queue ", 10);
			substdio_put(subfderr, shm_name + 1, len - 1);
			substdio_put(subfderr, "\n", 1);
			substdio_flush(subfderr);
			continue;
		}
		qdef[j].lcur = q[0];
		lcur += q[0];
		qdef[j].lmax = q[2];
		qdef[j].rcur = q[1];
		rcur += q[1];
		qdef[j].rmax = q[3];
		qdef[j].flag = q[4];
		x = q[0] + q[1];
		load += (double) q[0] / q[2] + (double) q[1]/ q[3];
		if (min == -1 || x < min)
			min = x; /*- minimum concurrency */
	}
	if (qload)
		*qload = load * 100;
	if (!display)
		return;
	for (j = 0; j < queue_count; j++) {
		if (!j)
			substdio_put(subfdout, "queue      local  Remote   +/- flag\n", 36);
		if (!qdef[j].lmax || !qdef[j].rmax) {
			substdio_put(subfderr, "invalid concurrency = 0 ", 24);
			if (!qdef[j].lmax)
				substdio_put(subfderr, "[local] ", 8);
			if (!qdef[j].rmax)
				substdio_put(subfderr, "[remote] ", 9);
			substdio_put(subfderr, "for queue ", 10);
			substdio_put(subfderr, qdef[j].queue.s, qdef[j].queue.len);
			substdio_put(subfderr, "\n", 1);
			substdio_flush(subfderr);
			continue;
		}
		qprintf(subfdout, qdef[j].queue.s, "%-08s");
		substdio_put(subfdout, " ", 1);
		strnum[i = fmt_ulong(strnum, qdef[j].lcur)] = 0;
		qprintf(subfdout, strnum, "%+3s");
		substdio_put(subfdout, "/", 1);
		strnum[i = fmt_ulong(strnum, qdef[j].lmax)] = 0;
		qprintf(subfdout, strnum, "%-4s");
		substdio_put(subfdout, " ", 1);
		strnum[i = fmt_ulong(strnum, qdef[j].rcur)] = 0;
		qprintf(subfdout, strnum, "%+3s");
		substdio_put(subfdout, "/", 1);
		strnum[i = fmt_ulong(strnum, qdef[j].rmax)] = 0;
		qprintf(subfdout, strnum, "%-4s");

		x = qdef[j].lcur + qdef[j].rcur;
		substdio_put(subfdout, x == min ? " - " : " + ", 3);
		substdio_put(subfdout, qdef[j].flag ? " disabled\n" : "  enabled\n", 10);
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
	strnum[i = fmt_double(strnum, 100 * load, 4)] = 0;
	substdio_put(subfdout, strnum, i);
	substdio_put(subfdout, "\n", 1);
	substdio_flush(subfdout);
	return;
}

void
getversion_queue_load_c()
{
	static char    *x = "$Id: queue_load.c,v 1.1 2022-04-16 01:32:47+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
#endif

/*-
 * $Log: queue_load.c,v $
 * Revision 1.1  2022-04-16 01:32:47+05:30  Cprogrammer
 * Initial revision
 *
 */
