/*
 * $Log: process_queue.c,v $
 * Revision 1.1  2021-06-28 17:08:03+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <unistd.h>
#include <env.h>
#include <substdio.h>
#include <subfd.h>
#include <scan.h>
#include <fmt.h>
#include <stralloc.h>
#include <error.h>
#include <strerr.h>
#include "control.h"
#include "auto_qmail.h"
#include "set_environment.h"
#include "variables.h"
#include "process_queue.h"

#ifndef QUEUE_COUNT
#define QUEUE_COUNT 10
#endif

void
process_queue(char *warn, char *fatal, int (*func)(), int *w, int *x, int *y, int *z)
{
	char           *queue_count_ptr, *queue_start_ptr, *qbase;
	char           *extra_queue[] = {"slowq", "nqueue", 0};
	char            strnum[FMT_ULONG];
	int             idx, count, qcount, qstart;
	static stralloc Queuedir = { 0 }, QueueBase = { 0 };

	set_environment(warn, fatal);
	if (!(qbase = env_get("QUEUE_BASE"))) {
		switch (control_readfile(&QueueBase, "queue_base", 0))
		{
		case -1:
			strerr_die2sys(111, fatal, "unable to read controls: ");
			break;
		case 0:
			if (!stralloc_copys(&QueueBase, auto_qmail) ||
					!stralloc_catb(&QueueBase, "/queue", 6) ||
					!stralloc_0(&QueueBase))
				strerr_die2x(111, fatal, "out of memory");
			break;
		case 1:
			break;
		}
	}
	qbase = QueueBase.s;
	if (!(queue_count_ptr = env_get("QUEUE_COUNT")))
		qcount = QUEUE_COUNT;
	else
		scan_int(queue_count_ptr, &qcount);
	if (!(queue_start_ptr = env_get("QUEUE_START")))
		qstart = 1;
	else
		scan_int(queue_start_ptr, &qstart);
	for (idx = qstart, count=1; count <= qcount; count++, idx++) {
		if (!stralloc_copys(&Queuedir, qbase) ||
				!stralloc_cats(&Queuedir, "/queue") ||
				!stralloc_catb(&Queuedir, strnum, fmt_ulong(strnum, (unsigned long) idx)) ||
				!stralloc_0(&Queuedir))
			strerr_die2x(111, fatal, "out of memory");
		if (access(Queuedir.s, F_OK)) {
			if (errno != error_noent)
				strerr_die4sys(111, fatal, "unable to access ", Queuedir.s, ": ");
			break;
		}
		substdio_puts(subfdout, "processing queue ");
		substdio_puts(subfdout, Queuedir.s);
		substdio_puts(subfdout, "\n");
		queuedir = Queuedir.s;
		if (w && x && y && z)
			(*func) (w, x, y, z);
		else
			(*func) ();
		substdio_puts(subfdout, "\n");
	}

	for (idx = 0; extra_queue[idx]; idx++) {
		if (!stralloc_copys(&Queuedir, qbase) ||
				!stralloc_append(&Queuedir, "/") ||
				!stralloc_cats(&Queuedir, extra_queue[idx]) ||
				!stralloc_0(&Queuedir))
			strerr_die2x(111, fatal, "out of memory");
		if (!access(Queuedir.s, F_OK)) {
			substdio_puts(subfdout, "processing queue ");
			substdio_puts(subfdout, Queuedir.s);
			substdio_puts(subfdout, "\n");
			queuedir = Queuedir.s;
			if (w && x && y && z)
				(*func) (w, x, y, z);
			else
				(*func) ();
			substdio_puts(subfdout, "\n");
		} else
		if (errno != error_noent)
			strerr_die4sys(111, fatal, "unable to access ", Queuedir.s, ": ");
	}
	return;
}

void
getversion_process_queue_c()
{
	static char    *x = "$Id: process_queue.c,v 1.1 2021-06-28 17:08:03+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
