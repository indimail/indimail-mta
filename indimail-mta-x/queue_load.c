/*
 * $Id: queue_load.c,v 1.3 2022-04-23 00:14:18+05:30 Cprogrammer Exp mbhangui $
 */
#include "haslibrt.h"

#ifdef HASLIBRT
#include <substdio.h>
#include <subfd.h>
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
queue_load(char *argv0, int *qcount, int *qconf, double total_load[2], QDEF **queue)
{
	int             shm, i, j, queue_count, queue_conf, len, x, min = -1, lcur = 0, rcur = 0;
	double          load_l, load_r;
	int             q[5];
	QDEF           *qdef;
	char            shm_name[256];
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
	/*- initialize qcount QDEF structures */
	for (j = 0; j < queue_count; j++) {
		qdef[j].queue.a = 0;
		qdef[j].queue.len = 0;
		qdef[j].queue.s = 0;
	}
	if (queue)
		*queue = qdef;
	for (j = 0, load_l = load_r = 0.0, min = -1; j < queue_count; j++) {
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
			substdio_put(subfderr, ": warn: invalid concurrency = 0 ", 35);
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
		qdef[j].rcur = q[1];
		rcur += q[1];
		qdef[j].lmax = q[2];
		qdef[j].rmax = q[3];
		qdef[j].flag = q[4];
		x = q[0] + q[1];
		load_l += (double) q[0] / q[2];
		load_r += (double) q[1] / q[3];
		if (min == -1 || x < min)
			min = x; /*- minimum concurrency */
	}
	if (total_load) {
		total_load[0] = load_l * 100;
		total_load[1] = load_r * 100;
	}
	return;
}

void
getversion_queue_load_c()
{
	static char    *x = "$Id: queue_load.c,v 1.3 2022-04-23 00:14:18+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
#endif

/*-
 * $Log: queue_load.c,v $
 * Revision 1.3  2022-04-23 00:14:18+05:30  Cprogrammer
 * initialize QDEF
 *
 * Revision 1.2  2022-04-16 13:03:13+05:30  Cprogrammer
 * removed display of queue loads
 *
 * Revision 1.1  2022-04-16 01:32:47+05:30  Cprogrammer
 * Initial revision
 *
 */
