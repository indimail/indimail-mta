/*
 * $Log: loadbalance.c,v $
 * Revision 2.1  2003-11-18 18:59:16+05:30  Cprogrammer
 * load balance function
 *
 */

#ifndef	lint
static char     sccsid[] = "$Id: loadbalance.c,v 2.1 2003-11-18 18:59:16+05:30 Cprogrammer Stab mbhangui $";
#endif

#include "indimail.h"
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

int
loadbalance(int count)
{
#ifdef RANDOM_BALANCING
	static int      seeded;

	if (!seeded)
	{
		seeded = 1;
		srandom(time(0)^(getpid()<<15));
	}
#endif
#ifdef RANDOM_BALANCING
	return(1 + (int) ((float) count * rand()/(RAND_MAX + 1.0)));
#else
	return((time(0) % count) + 1);
#endif
}

void
getversion_loadbalance_c()
{
	printf("%s\n", sccsid);
	printf("%s\n", sccsidh);
}
