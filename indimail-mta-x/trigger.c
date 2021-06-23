/*
 * $Log: trigger.c,v $
 * Revision 1.2  2021-06-23 10:02:37+05:30  Cprogrammer
 * converted to ansic prototypes
 *
 */
#include <unistd.h>
#include "select.h"
#include "open.h"
#include "trigger.h"
#include "hasnpbg1.h"

static int      fd = -1;
#ifdef HASNAMEDPIPEBUG1
static int      fdw = -1;
#endif

void
trigger_set()
{
	if (fd != -1)
		close(fd);
#ifdef HASNAMEDPIPEBUG1
	if (fdw != -1)
		close(fdw);
#endif
	fd = open_read("lock/trigger");
#ifdef HASNAMEDPIPEBUG1
	fdw = open_write("lock/trigger");
#endif
}

void
trigger_selprep(int *nfds, fd_set *rfds)
{
	if (fd != -1) {
		FD_SET(fd, rfds);
		if (*nfds < fd + 1)
			*nfds = fd + 1;
	}
}

int
trigger_pulled(fd_set *rfds)
{
	if (fd != -1 && FD_ISSET(fd, rfds))
		return 1;
	return 0;
}

void
getversion_trigger_c()
{
	static char    *x = "$Id: trigger.c,v 1.2 2021-06-23 10:02:37+05:30 Cprogrammer Exp mbhangui $";

	x++;
}
