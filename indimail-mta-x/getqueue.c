/*
 * $Log: getqueue.c,v $
 * Revision 1.3  2022-04-14 08:17:55+05:30  Cprogrammer
 * added feature to disable a queue and skip disabled queues
 * refactored code and added comments
 *
 * Revision 1.2  2022-03-30 21:08:38+05:30  Cprogrammer
 * use arc4random() to randomly select queue
 *
 * Revision 1.1  2022-03-26 10:23:34+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <fmt.h>
#include <scan.h>
#include <env.h>
#include <error.h>
#include <arc4random.h>
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
queueNo_from_shm(char *ident)
{
	int             shm, i, j, x, y, min, n = 1, qcount;
	int             q[5];
	int            *queue;
	uint32_t        random;
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
	/*- get queue with lowest concurrency */
	for (j = n = 0, min = -1; j < qcount; j++) {
		s = shm_name;
		i = fmt_str(s, "/queue");
		s += i;
		i = fmt_int(s, j + 1);
		s += i;
		*s++ = 0;
		i = 0;
		if ((shm = shm_open(shm_name, O_RDONLY, 0600)) == -1)
			custom_error(ident, "Z", "failed to open POSIX shared memory segment ", shm_name, "X.3.0");
		if (read(shm, (char *) q, sizeof(int) * 5) == -1)
			custom_error(ident, "Z", "failed to read POSIX shared memory segment ", shm_name, "X.3.0");
		close(shm);
		/*-
		 * q[0] - concurrencyusedlocal
		 * q[1] - concurrencyusedremote
		 * q[2] - concurrencylocal
		 * q[3] - concurrencyremote
		 * q[4] - 0 enabled, 1 disabled
		 *        on startup, qmail-send will always set this to 0 (enabled)
		 *        future version will use the mode of the queue base directory
		 *        to set it to enabled/disabled state
		 */
		/*- skip disabled queue and queue with invalid concurrency */
		if (q[4] || !q[2] || !q[3])
			continue;
		x = q[0] > q[1] ? q[0] : q[1]; /*- max concurrency for queue */
		queue[j] = x;
		if (min == -1 || x < min) {
			min = x; /*- minimum concurrency */
			n = j; /*- queue with minimum concurrency */
		}
	}
	/*-
	 * at this stage n+1 is our queue with minimum concurrency. There
	 * can be other queues with the same minimum concurrency (x).
	 * we will use modulus * of arc4random() and x to select one of them
	 */
	for (i = 0, x = 0; i < qcount; i++) {
		if (queue[i] == min)
			x++; /*- count of queues with the same minimum concurrency */
	}
	if (x == 1) { /*- only one queue with minimum concurrency */
		alloc_free((char *) queue);
		return n + 1;
	}
	random = arc4random();
	for (i = j = 0, y = random % x; i < qcount; i++) {
		if (queue[i] == min) {
			if (j == y) {
				alloc_free((char *) queue);
				return i + 1;
			}
			j++;
		}
	}
	/*- never reached but make compiler happy */
	alloc_free((char *) queue);
	return n + 1;
}
#endif

int
queueNo_from_env()
{
	char           *ptr;
	int             qcount, qstart;
	uint32_t        random;

	if (!(ptr = env_get("QUEUE_COUNT")))
		qcount = QUEUE_COUNT;
	else
		scan_int(ptr, &qcount);
	if (!(ptr = env_get("QUEUE_START")))
		qstart = QUEUE_START;
	else
		scan_int(ptr, &qstart);
	random = arc4random();
	return ((random % qcount) + qstart);
}

#ifndef	lint
void
getversion_getqueue_c()
{
	static char    *x = "$Id: getqueue.c,v 1.3 2022-04-14 08:17:55+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
#endif
