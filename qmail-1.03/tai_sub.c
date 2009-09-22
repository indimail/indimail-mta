/*
 * $Log: tai_sub.c,v $
 * Revision 1.2  2004-10-22 20:31:35+05:30  Cprogrammer
 * added RCS id
 *
 * Revision 1.1  2003-12-31 19:32:25+05:30  Cprogrammer
 * Initial revision
 *
 *
 * Public domain. 
 */

#include "tai.h"

void
tai_sub(struct tai *t, struct tai *u, struct tai *v)
{
	t->x = u->x - v->x;
}

void
getversion_tai_sub_c()
{
	static char    *x = "$Id: tai_sub.c,v 1.2 2004-10-22 20:31:35+05:30 Cprogrammer Stab mbhangui $";

	x++;
}
