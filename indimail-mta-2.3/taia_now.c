/*
 * $Log: taia_now.c,v $
 * Revision 1.2  2004-10-22 20:31:30+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-09-19 18:53:46+05:30  Cprogrammer
 * Initial revision
 *
 */

/*- Public domain.  */

#include <sys/types.h>
#include <sys/time.h>
#include "taia.h"

void
taia_now(struct taia *t)
{
	struct timeval  now;
	gettimeofday(&now, (struct timezone *) 0);
	tai_unix(&t->sec, now.tv_sec);
	t->nano = 1000 * now.tv_usec + 500;
	t->atto = 0;
}

void
getversion_taia_now_c()
{
	static char    *x = "$Id: taia_now.c,v 1.2 2004-10-22 20:31:30+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
