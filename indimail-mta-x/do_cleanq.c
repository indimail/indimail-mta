/*
 * $Log: do_cleanq.c,v $
 * Revision 1.3  2024-05-09 22:03:17+05:30  mbhangui
 * fix discarded-qualifier compiler warnings
 *
 * Revision 1.2  2004-10-22 20:24:35+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-09-22 22:23:16+05:30  Cprogrammer
 * Initial revision
 *
 */
#include <sys/types.h>
#include <unistd.h>
#include "wait.h"
#include "auto_cleaner.h"

char           *cmd[] = {
	auto_runcleaner, /*- run-cleanq */
	0
};

int
do_cleanq()
{
	pid_t           pid = -1;

	/*- Launch run-cleanq */
	switch (pid = vfork())
	{
	case -1:
		return 0;
	case 0:
		close(0);				/*- Don't let it fiddle with the message */
		close(1);				/*- Don't let it fiddle with envelope */
		execve(*cmd, cmd, 0);
		_exit(1);				/*- hopefully not reached */
	}

	/*- Don't wait for the exit status */
	return 1;
}

void
getversion_do_cleanq_c()
{
	const char     *x = "$Id: do_cleanq.c,v 1.3 2024-05-09 22:03:17+05:30 mbhangui Exp mbhangui $";

	x++;
}
