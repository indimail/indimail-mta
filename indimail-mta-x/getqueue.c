/*
 * $Log: getqueue.c,v $
 * Revision 1.1  2022-03-26 10:23:34+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <datetime.h>
#include <fmt.h>
#include <scan.h>
#include <env.h>
#include <error.h>
#include "haslibrt.h"
#ifdef HASLIBRT
#include <sys/mman.h>
#include <sys/stat.h>  /* For mode constants */
#include <fcntl.h>     /* For O_* constants */
#include <str.h>
#include <alloc.h>
#include "custom_error.h"
#endif

#ifdef HASLIBRT
int
queueNo_from_shm(char *ident, datetime_sec t)
{
	int             shm, i, j, x, y, min, n = 1, qcount;
	int             q[4];
	int            *queue;
	char            shm_name[FMT_ULONG + 6];
	char           *s;

	/*- get queue count */
	if ((shm = shm_open("/qscheduler", O_RDONLY, 0644)) == -1) {
		if (errno == error_noent)
			return -1;
		custom_error(ident, "Z", "unable to open POSIX shared memory segment /qscheduler", 0, "X.3.0");
	}
	if (read(shm, (char *) &qcount, sizeof(int)) == -1)
		custom_error(ident, "Z", "unable to read POSIX shared memory segment /qscheduler", 0, "X.3.0");
	close(shm);
	if (!(queue = (int *) alloc(qcount * sizeof(int))))
		_exit(51);
	/*- get queue with lowest concurrency load  */
	for (j = min = n = 0; j < qcount; j++) {
		s = shm_name;
		i = fmt_str(s, "/queue");
		s += i;
		i = fmt_int(s, j + 1);
		s += i;
		*s++ = 0;
		i = 0;
		if ((shm = shm_open(shm_name, O_RDONLY, 0600)) == -1)
			custom_error(ident, "Z", "failed to open POSIX shared memory segment ", shm_name, "X.3.0");
		if (read(shm, (char *) q, sizeof(int) * 4) == -1)
			custom_error(ident, "Z", "failed to read POSIX shared memory segment ", shm_name, "X.3.0");
		close(shm);
		/*-
		 * q[0] - concurrencyusedlocal
		 * q[1] - concurrencyusedremote
		 * q[2] - concurrencylocal
		 * q[3] - concurrencyremote
		 */
		if (!q[2] || !q[3]) {
			alloc_free((char *) queue);
			return -1;
		}
		x = q[0] * 100 /q[2] > q[1] * 100 /q[3] ? q[0] * 100/q[2] : q[1] * 100/q[3];
		queue[j] = x;
		if (!j) {
			min = x;
			n = j;
			continue;
		}
		if (x < min) {
			min = x;
			n = j;
		}
	}
	/*
	 * at this stage n+1 is our queue
	 * with minimum load. But there may be
	 * other queues with the same minimum load.
	 * use time modulus to select one of them
	 */
	for (i = 0, x = 0; i < qcount; i++) {
		if (queue[i] == min)
			x++; /*- queue count with minimum loads */
	}
	if (x == 1) {
		alloc_free((char *) queue);
		return n + 1;
	}
	for (i = j = 0, y = t % x; i < qcount; i++) {
		if (queue[i] == min) {
			if (j == y) {
				alloc_free((char *) queue);
				return i + 1;
			}
			j++;
		}
	}
	alloc_free((char *) queue);
	return n;
}
#endif

int
queueNo_from_env(datetime_sec t)
{
	char           *ptr;
	int             qcount, qstart;

	if (!(ptr = env_get("QUEUE_COUNT")))
		qcount = QUEUE_COUNT;
	else
		scan_int(ptr, &qcount);
	if (!(ptr = env_get("QUEUE_START")))
		qstart = QUEUE_START;
	else
		scan_int(ptr, &qstart);
	return ((t % qcount) + qstart);
}

#ifndef	lint
void
getversion_getqueue_c()
{
	static char    *x = "$Id: getqueue.c,v 1.1 2022-03-26 10:23:34+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
#endif
