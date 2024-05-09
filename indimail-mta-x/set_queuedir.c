/*
 * $Log: set_queuedir.c,v $
 * Revision 1.2  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.1  2022-04-04 00:10:31+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <stralloc.h>
#include <stddef.h>
#include <env.h>
#include "auto_qmail.h"
#include "control.h"

const char     *
set_queuedir(const char *argv0, const char *queue)
{
	char           *qbase;
	static stralloc QueueDir = { 0 };

	if ((qbase = env_get("QUEUE_BASE"))) {
		if (!stralloc_copys(&QueueDir, qbase) ||
				!stralloc_append(&QueueDir, "/") ||
				!stralloc_cats(&QueueDir, queue) ||
				!stralloc_0(&QueueDir))
			return ((char *) NULL);
		return QueueDir.s;
	}
	switch (control_readfile(&QueueDir, "queue_base", 0))
	{
	case 0:
		if (!stralloc_copys(&QueueDir, auto_qmail) ||
				!stralloc_append(&QueueDir, "/") ||
				!stralloc_cats(&QueueDir, queue) ||
				!stralloc_0(&QueueDir))
			return ((char *) NULL);
		return QueueDir.s;
	case 1:
		while (!stralloc_append(&QueueDir, "/") ||
				!stralloc_cats(&QueueDir, queue) ||
				!stralloc_0(&QueueDir))
			return ((char *) NULL);
		return QueueDir.s;
	default:
		return ((char *) NULL);
	}
}

#ifndef	lint
void
getversion_set_queuedir_c()
{
	const char     *x = "$Id: set_queuedir.c,v 1.2 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
#endif
