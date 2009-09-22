/*
 * $Log: leapsecs_add.c,v $
 * Revision 1.2  2004-10-22 20:26:02+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2004-06-16 01:19:55+05:30  Cprogrammer
 * Initial revision
 *
 */
#include "leapsecs.h"
#include "tai.h"

/*
 * XXX: breaks tai encapsulation 
 */

extern struct tai *leapsecs;
extern int      leapsecs_num;

void
leapsecs_add(t, hit)
	struct tai     *t;
	int             hit;
{
	int             i;
	uint64          u;

	if (leapsecs_init() == -1)
		return;

	u = t->x;

	for (i = 0; i < leapsecs_num; ++i)
	{
		if (u < leapsecs[i].x)
			break;
		if (!hit || (u > leapsecs[i].x))
			++u;
	}

	t->x = u;
}

void
getversion_leapsecs_add_c()
{
	static char    *x = "$Id: leapsecs_add.c,v 1.2 2004-10-22 20:26:02+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
